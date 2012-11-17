//
// Copyright (c) 2010-2011 Matthew Jack and Doug Binks
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

// RuntimeDLLTest01.cpp : Defines the entry point for the console application.
//
// Notes:
//   - We use a single intermediate directory for compiled .obj files, which means
//     we don't support compiling multiple files with the same name. Could fix this
//     with either mangling names to include paths,  or recreating folder structure
//
//

#include "Compiler.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include "boost/algorithm/string.hpp"

#include "assert.h"
#include <process.h>

#include "ICompilerLogger.h"

using namespace std;


struct VSVersionInfo
{
	int				Version;
	std::wstring	Path;
};

const std::string	c_CompletionToken( "_COMPLETION_TOKEN_" );

void GetPathsOfVisualStudioInstalls( std::vector<VSVersionInfo>* pVersions );

void ReadAndHandleOutputThread( LPVOID arg );
void WriteInput( HANDLE hPipeWrite, std::string& input  );

class PlatformCompilerImplData
{
public:
	PlatformCompilerImplData()
		: m_bCompileIsComplete( false )
		, m_CmdProcessOutputRead( NULL )
		, m_CmdProcessInputWrite( NULL )
	{
		ZeroMemory( &m_CmdProcessInfo, sizeof(m_CmdProcessInfo) );
	}

	void InitialiseProcess()
	{
		//init compile process
		STARTUPINFOW				si;
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);

		boost::filesystem::path VSPath(  m_VSPath );

#ifndef _WIN64
		std::string cmdSetParams = "@PROMPT $ \n\"" + VSPath.string() + "Vcvars32.bat\"\n";
#else
		std::string cmdSetParams = "@PROMPT $ \n\"" + VSPath.string() + "/../Vcvarsall.bat\" amd64\n";
#endif
		// Set up the security attributes struct.
		SECURITY_ATTRIBUTES sa;
		sa.nLength= sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;


		// Create the child output pipe.
		//redirection of output
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		HANDLE hOutputReadTmp,hOutputWrite;
		if (!CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,20*1024))
		{
			if( m_pLogger ) m_pLogger->LogError("[RuntimeCompiler] Failed to create output redirection pipe\n");
			goto ERROR_EXIT;
		}
		si.hStdOutput = hOutputWrite;

		// Create a duplicate of the output write handle for the std error
		// write handle. This is necessary in case the child application
		// closes one of its std output handles.
		HANDLE hErrorWrite;
		if (!DuplicateHandle(GetCurrentProcess(),hOutputWrite,
							   GetCurrentProcess(),&hErrorWrite,0,
							   TRUE,DUPLICATE_SAME_ACCESS))
		{
			if( m_pLogger ) m_pLogger->LogError("[RuntimeCompiler] Failed to duplicate error output redirection pipe\n");
			goto ERROR_EXIT;
		}
		si.hStdError = hErrorWrite;


		// Create new output read handle and the input write handles. Set
		// the Properties to FALSE. Otherwise, the child inherits the
		// properties and, as a result, non-closeable handles to the pipes
		// are created.
 		HANDLE hOutputRead;
 		if( si.hStdOutput )
		{
			 if (!DuplicateHandle(GetCurrentProcess(),hOutputReadTmp,
								   GetCurrentProcess(),
								   &hOutputRead, // Address of new handle.
								   0,FALSE, // Make it uninheritable.
								   DUPLICATE_SAME_ACCESS))
			 {
				   if( m_pLogger ) m_pLogger->LogError("[RuntimeCompiler] Failed to duplicate output read pipe\n");
				   goto ERROR_EXIT;
			 }
		}


		HANDLE hInputRead,hInputWriteTmp,hInputWrite;
		// Create a pipe for the child process's STDIN. 
		if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 4096))
		{
			if( m_pLogger ) m_pLogger->LogError("[RuntimeCompiler] Failed to create input pipes\n");
			goto ERROR_EXIT;
		}
		si.hStdInput = hInputRead;

		// Create new output read handle and the input write handles. Set
		// the Properties to FALSE. Otherwise, the child inherits the
		// properties and, as a result, non-closeable handles to the pipes
		// are created.
 		if( si.hStdOutput )
		{
			 if (!DuplicateHandle(GetCurrentProcess(),hInputWriteTmp,
								   GetCurrentProcess(),
								   &hInputWrite, // Address of new handle.
								   0,FALSE, // Make it uninheritable.
								   DUPLICATE_SAME_ACCESS))
			 {
				   if( m_pLogger ) m_pLogger->LogError("[RuntimeCompiler] Failed to duplicate input write pipe\n");
				   goto ERROR_EXIT;
			 }
		}
		/*
		// Ensure the write handle to the pipe for STDIN is not inherited. 
		if ( !SetHandleInformation(hInputWrite, HANDLE_FLAG_INHERIT, 0) )
		{
			m_pLogger->LogError("[RuntimeCompiler] Failed to make input write pipe non inheritable\n");
			goto ERROR_EXIT;
		}
		*/

		wchar_t* pCommandLine = L"cmd /q";
		//CreateProcessW won't accept a const pointer, so copy to an array 
		wchar_t pCmdLineNonConst[1024];
		wcscpy_s( pCmdLineNonConst, pCommandLine );
		CreateProcessW(
			  NULL,				//__in_opt     LPCTSTR lpApplicationName,
			  pCmdLineNonConst,			//__inout_opt  LPTSTR lpCommandLine,
			  NULL,				//__in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes,
			  NULL,				//__in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes,
			  TRUE,				//__in         BOOL bInheritHandles,
			  0,				//__in         DWORD dwCreationFlags,
			  NULL,				//__in_opt     LPVOID lpEnvironment,
			  NULL,				//__in_opt     LPCTSTR lpCurrentDirectory,
			  &si,				//__in         LPSTARTUPINFO lpStartupInfo,
			  &m_CmdProcessInfo				//__out        LPPROCESS_INFORMATION lpProcessInformation
			);



		m_CmdProcessInputWrite = hInputWrite;
		m_CmdProcessOutputRead = hOutputRead;

		//send initial set up command
		WriteInput( m_CmdProcessInputWrite, cmdSetParams );

		//launch threaded read.
		_beginthread( ReadAndHandleOutputThread, 0, this ); //this will exit when process for compile is closed


	ERROR_EXIT:
		CloseHandle( hOutputReadTmp );
		hOutputReadTmp = NULL;
		CloseHandle( hOutputWrite );
		hOutputWrite = NULL;
		CloseHandle( hErrorWrite );
		hErrorWrite = NULL;
	}
	std::wstring		m_VSPath;
	std::string			m_intermediatePath;
	PROCESS_INFORMATION m_CmdProcessInfo;
	HANDLE				m_CmdProcessOutputRead;
	HANDLE				m_CmdProcessInputWrite;
	volatile bool		m_bCompileIsComplete;
	ICompilerLogger*	m_pLogger;
};

Compiler::Compiler() 
	: m_pImplData( 0 )
{
}

Compiler::~Compiler()
{
	BOOL retval = TerminateProcess( m_pImplData->m_CmdProcessInfo.hProcess, 0 );
	//CloseHandle( m_pImplData->m_CmdProcessInfo.hProcess );
    CloseHandle( m_pImplData->m_CmdProcessInfo.hThread );
	CloseHandle( m_pImplData->m_CmdProcessInputWrite );
	CloseHandle( m_pImplData->m_CmdProcessOutputRead );
}

const std::wstring Compiler::GetObjectFileExtension() const
{
	return L".obj";
}

bool Compiler::GetIsComplete() const
{
	return m_pImplData->m_bCompileIsComplete;
}

void Compiler::Initialise( ICompilerLogger * pLogger )
{
	m_pImplData = new PlatformCompilerImplData;
	m_pImplData->m_pLogger = pLogger;
	// get VS compiler path
	std::vector<VSVersionInfo> Versions;
	GetPathsOfVisualStudioInstalls( &Versions );
	m_pImplData->m_VSPath = Versions[0].Path;

	m_pImplData->m_intermediatePath = "Runtime";

	// Remove any existing intermediate directory
	boost::system::error_code ec;
	boost::filesystem::path path(m_pImplData->m_intermediatePath);
	if (boost::filesystem::is_directory(path))
	{
		// In theory remove_all should do the job here, but it doesn't seem to
		boost::filesystem::directory_iterator dir_iter(path), dir_end;
		int removed = 0, failed = 0;
		for(;dir_iter != dir_end; ++dir_iter)
		{
			boost::filesystem::remove(*dir_iter, ec);
			if (ec) failed++;
			else removed++;
		}
		boost::filesystem::remove(path,ec);
	}

}

void Compiler::RunCompile( const std::vector<boost::filesystem::path>& filesToCompile,
					 const std::vector<boost::filesystem::path>& includeDirList,
					 const std::vector<boost::filesystem::path>& libraryDirList,
					 const std::vector<boost::filesystem::path>& linkLibraryList,
					 const char* pCompileOptions,
					 const char* pLinkOptions,
					 const boost::filesystem::path& outputFile )
{
	m_pImplData->m_bCompileIsComplete = false;
	//optimization and c runtime
#ifdef _DEBUG
	std::string flags = "/nologo /Od /Zi /FC /LDd ";
#else
	std::string flags = "/nologo /O2 /Zi /FC /LD ";	//also need debug information in release
#endif
	if( NULL == m_pImplData->m_CmdProcessInfo.hProcess )
	{
		m_pImplData->InitialiseProcess();
	}

	if( pCompileOptions )
	{
		flags += pCompileOptions;
	}

	std::string linkOptions;
	bool bHaveLinkOptions = pLinkOptions && strlen( pLinkOptions );
	if( libraryDirList.size() ||  bHaveLinkOptions )
	{
		linkOptions = " /link ";
		for( size_t i = 0; i < libraryDirList.size(); ++i )
		{
			linkOptions += " /LIBPATH:\"" + libraryDirList[i].string() + "\"";
		}

		if( bHaveLinkOptions )
		{
			linkOptions += pLinkOptions;
		}
	}

	// Check for intermediate directory, create it if required
	// There are a lot more checks and robustness that could be added here
	std::string intermediate = m_pImplData->m_intermediatePath;
	if (!boost::filesystem::exists(intermediate))
	{
		boost::system::error_code ec;
		boost::filesystem::create_directory(intermediate,ec);
		if( m_pImplData->m_pLogger ) m_pImplData->m_pLogger->LogInfo("Created intermediate folder \"%s\"\n",intermediate.c_str());
	}


	//create include path search string
	std::string strIncludeFiles;
	for( size_t i = 0; i < includeDirList.size(); ++i )
	{
		strIncludeFiles += " /I \"" + includeDirList[i].string() + "\"";
	}


	// When using multithreaded compilation, listing a file for compilation twice can cause errors, hence 
	// we do a final filtering of input here.
	// See http://msdn.microsoft.com/en-us/library/bb385193.aspx - "Source Files and Build Order"

	// Create compile path search string
	std::string strFilesToCompile;
	std::set<std::string> filteredPaths;
	for( size_t i = 0; i < filesToCompile.size(); ++i )
	{
		std::string strPath = filesToCompile[i].string();
#ifdef _WIN32
		// In Win32, make filename lowercase so paths can be compared. Could alternatively use boost equality() operation.
		strPath = boost::to_lower_copy(strPath);
#endif
		std::set<std::string>::const_iterator it = filteredPaths.find(strPath);
		if (it == filteredPaths.end())
		{
			strFilesToCompile += " \"" + strPath + "\"";
			filteredPaths.insert(strPath);
		}
	}

	std::string strLinkLibraries;
	for( size_t i = 0; i < linkLibraryList.size(); ++i )
	{
		strLinkLibraries += " \"" + linkLibraryList[i].string() + "\" ";
	}
	



char* pCharTypeFlags = "";
#ifdef UNICODE
	pCharTypeFlags = "/D UNICODE /D _UNICODE ";
#endif

	// /MP - use multiple processes to compile if possible. Only speeds up compile for multiple files and not link
	std::string cmdToSend = "cl " + flags + pCharTypeFlags
		+ " /MP /Fo\"" + intermediate + "\\\\\" "
		+ "/D WIN32 /EHa /Fe" + outputFile.string();
	cmdToSend += " " + strIncludeFiles + " " + strFilesToCompile + strLinkLibraries + linkOptions
		+ "\necho " + c_CompletionToken + "\n";
	OutputDebugStringA( cmdToSend.c_str() );
	WriteInput( m_pImplData->m_CmdProcessInputWrite, cmdToSend );
}


void GetPathsOfVisualStudioInstalls( std::vector<VSVersionInfo>* pVersions )
{
	//HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\<version>\Setup\VS\<edition>
	std::wstring keyName = L"SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VC7";

	const size_t NUMNAMESTOCHECK = 4;
	std::wstring valueName[NUMNAMESTOCHECK];

	//switch around prefered compiler to the one we've used to compile this file
	const unsigned int MSCVERSION = _MSC_VER;
	switch( MSCVERSION )
	{
	case 1400:	//VS 2005
		valueName[3] = L"8.0";	//VS 2005
		valueName[2] = L"9.0";	//VS 2008
		valueName[1] = L"10.0";	//VS 2010
		valueName[0] = L"11.0";	//VS 2011
		break;
	case 1500:	//VS 2008
		valueName[2] = L"8.0";	//VS 2005
		valueName[3] = L"9.0";	//VS 2008
		valueName[1] = L"10.0";	//VS 2010
		valueName[0] = L"11.0";	//VS 2011
		break;
	case 1600:	//VS 2010
		valueName[1] = L"8.0";	//VS 2005
		valueName[2] = L"9.0";	//VS 2008
		valueName[3] = L"10.0";	//VS 2010
		valueName[0] = L"11.0";	//VS 2011
		break;
	case 1700:	//VS 2011
		valueName[0] = L"8.0";	//VS 2005
		valueName[1] = L"9.0";	//VS 2008
		valueName[2] = L"10.0";	//VS 2010
		valueName[3] = L"11.0";	//VS 2011
		break;
	default:
		assert( false ); //shouldn't happen.
	}



	wchar_t value[MAX_PATH];
	DWORD size = MAX_PATH;

	HKEY key;
	LONG retKeyVal = RegOpenKeyExW(
				  HKEY_LOCAL_MACHINE,	//__in        HKEY hKey,
				  keyName.c_str(),			//__in_opt    LPCTSTR lpSubKey,
				  0,					//__reserved  DWORD ulOptions,
				  KEY_READ | KEY_WOW64_32KEY,		//__in        REGSAM samDesired,
				  &key					//__out       PHKEY phkResult
				);

	for( int i = NUMNAMESTOCHECK-1; i >= 0; --i )
	{

		LONG retVal = RegQueryValueExW(
					  key,					//__in         HKEY hKey,
					  valueName[i].c_str(),	//__in_opt     LPCTSTR lpValueName,
					  NULL,					//__reserved   LPDWORD lpReserved,
					  NULL ,				//__out_opt    LPDWORD lpType,
					  (LPBYTE)value,			//__out_opt    LPBYTE lpData,
					  &size					//__inout_opt  LPDWORD lpcbData
					);
		if( ERROR_SUCCESS == retVal )
		{
			VSVersionInfo vInfo;
			vInfo.Version = i + 8;
			vInfo.Path = value;
			vInfo.Path += L"bin\\";
			pVersions->push_back( vInfo );
		}
	}

	RegCloseKey( key );

	return;
}


void ReadAndHandleOutputThread( LPVOID arg )
{
	PlatformCompilerImplData* pImpl = (PlatformCompilerImplData*)arg;

    CHAR lpBuffer[1024];
    DWORD nBytesRead;
 	bool bReadActive = true;
	bool bReadOneMore = false;
    while( bReadActive )
    {
		if (!ReadFile(pImpl->m_CmdProcessOutputRead,lpBuffer,sizeof(lpBuffer)-1,
										&nBytesRead,NULL) || !nBytesRead)
		{
			bReadActive = false;
			if (GetLastError() != ERROR_BROKEN_PIPE)	//broken pipe is OK
			{
				if( pImpl->m_pLogger ) pImpl->m_pLogger->LogError( "[RuntimeCompiler] Redirect of compile output failed on read\n" );
			}
		}
		else
		{
			// Display the characters read in logger.
			lpBuffer[nBytesRead]=0;

			//fist check for completion token...
			std::string buffer( lpBuffer );
			size_t found = buffer.find( c_CompletionToken );
			if( found != std::string::npos )
			{
				//we've found the completion token, which means we quit
				buffer = buffer.substr( 0, found );
				if( pImpl->m_pLogger ) pImpl->m_pLogger->LogInfo("[RuntimeCompiler] Complete\n");
				pImpl->m_bCompileIsComplete = true;
			}
			if( bReadActive || buffer.length() ) //don't output blank last line
			{
				//check if this is an error
				size_t errorFound = buffer.find( " : error " );
				size_t fatalErrorFound = buffer.find( " : fatal error " );
				if( ( errorFound != std::string::npos ) || ( fatalErrorFound != std::string::npos ) )
				{
					if( pImpl->m_pLogger ) pImpl->m_pLogger->LogError( "%s", buffer.c_str() );
				}
				else
				{
					if( pImpl->m_pLogger ) pImpl->m_pLogger->LogInfo( "%s", buffer.c_str() );
				}
			}
		}
     }

}

void WriteInput( HANDLE hPipeWrite, std::string& input  )
{
    DWORD nBytesWritten;
	DWORD length = (DWORD)input.length();
	WriteFile( hPipeWrite, input.c_str() , length, &nBytesWritten, NULL );
}
