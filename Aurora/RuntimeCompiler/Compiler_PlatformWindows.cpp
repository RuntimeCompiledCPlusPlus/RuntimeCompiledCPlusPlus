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
#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include "FileSystemUtils.h"

#include "assert.h"
#include <process.h>

#include "ICompilerLogger.h"

using namespace std;
using namespace FileSystemUtils;

struct VSVersionInfo
{
	int				Version;
	std::string		Path;
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

#ifndef _WIN64
		std::string cmdSetParams = "@PROMPT $ \n\"" + m_VSPath + "Vcvarsall.bat\" x86\n";
#else
		std::string cmdSetParams = "@PROMPT $ \n\"" + m_VSPath + "Vcvarsall.bat\" x86_amd64\n";
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
		HANDLE hOutputReadTmp = NULL, hOutputWrite  = NULL, hErrorWrite = NULL;
		if (!CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,20*1024))
		{
			if( m_pLogger ) m_pLogger->LogError("[RuntimeCompiler] Failed to create output redirection pipe\n");
			goto ERROR_EXIT;
		}
		si.hStdOutput = hOutputWrite;

		// Create a duplicate of the output write handle for the std error
		// write handle. This is necessary in case the child application
		// closes one of its std output handles.
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
 		if( si.hStdOutput )
		{
			 if (!DuplicateHandle(GetCurrentProcess(),hOutputReadTmp,
								   GetCurrentProcess(),
								   &m_CmdProcessOutputRead, // Address of new handle.
								   0,FALSE, // Make it uninheritable.
								   DUPLICATE_SAME_ACCESS))
			 {
				   if( m_pLogger ) m_pLogger->LogError("[RuntimeCompiler] Failed to duplicate output read pipe\n");
				   goto ERROR_EXIT;
			 }
			CloseHandle( hOutputReadTmp );
			hOutputReadTmp = NULL;
		}


		HANDLE hInputRead,hInputWriteTmp;
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
								   &m_CmdProcessInputWrite, // Address of new handle.
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

		//send initial set up command
		WriteInput( m_CmdProcessInputWrite, cmdSetParams );

		//launch threaded read.
		_beginthread( ReadAndHandleOutputThread, 0, this ); //this will exit when process for compile is closed


	ERROR_EXIT:
		if( hOutputReadTmp ) 
		{
			CloseHandle( hOutputReadTmp );
		}
		if( hOutputWrite ) 
		{
			CloseHandle( hOutputWrite );
		}
		if( hErrorWrite )
		{
			CloseHandle( hErrorWrite );
		}
	}

    void CleanupProcessAndPipes()
    {
        // do not reset m_bCompileIsComplete and other members here, just process and pipes
        if(  m_CmdProcessInfo.hProcess )
        {
            TerminateProcess( m_CmdProcessInfo.hProcess, 0 );
			TerminateThread( m_CmdProcessInfo.hThread, 0 );
            CloseHandle( m_CmdProcessInfo.hThread );
		    ZeroMemory( &m_CmdProcessInfo, sizeof(m_CmdProcessInfo) );
	        CloseHandle( m_CmdProcessInputWrite );
            m_CmdProcessInputWrite = 0;
	        CloseHandle( m_CmdProcessOutputRead );
            m_CmdProcessOutputRead = 0;
        }

    }
    

    ~PlatformCompilerImplData()
    {
        CleanupProcessAndPipes();
    }

	std::string			m_VSPath;
	PROCESS_INFORMATION m_CmdProcessInfo;
	HANDLE				m_CmdProcessOutputRead;
	HANDLE				m_CmdProcessInputWrite;
	volatile bool		m_bCompileIsComplete;
	ICompilerLogger*	m_pLogger;
};

Compiler::Compiler() 
	: m_pImplData( 0 )
    , m_bFastCompileMode( false )
{
}

Compiler::~Compiler()
{
	delete m_pImplData;
}

std::string Compiler::GetObjectFileExtension() const
{
	return ".obj";
}

bool Compiler::GetIsComplete() const
{
    bool bComplete = m_pImplData->m_bCompileIsComplete;
    if( bComplete & !m_bFastCompileMode )
    {
        m_pImplData->CleanupProcessAndPipes();
    }
	return bComplete;
}

void Compiler::Initialise( ICompilerLogger * pLogger )
{
	m_pImplData = new PlatformCompilerImplData;
	m_pImplData->m_pLogger = pLogger;
	// get VS compiler path
	std::vector<VSVersionInfo> Versions;
	GetPathsOfVisualStudioInstalls( &Versions );

    if( !Versions.empty() )
    {
	    m_pImplData->m_VSPath = Versions[0].Path;
    }
    else
    {
        m_pImplData->m_VSPath = "";
        if( m_pImplData->m_pLogger )
        {
            m_pImplData->m_pLogger->LogError("No Supported Compiler for RCC++ found.\n");
        }
    }
}


void Compiler::RunCompile(	const std::vector<FileSystemUtils::Path>&	filesToCompile_,
							const CompilerOptions&						compilerOptions_,
							std::vector<FileSystemUtils::Path>			linkLibraryList_,
							const FileSystemUtils::Path&				moduleName_ )
{
    if( m_pImplData->m_VSPath.empty() )
    {
        if (m_pImplData->m_pLogger) { m_pImplData->m_pLogger->LogError("No Supported Compiler for RCC++ found, cannot compile changes.\n"); }
    	m_pImplData->m_bCompileIsComplete = true;
        return;
    }
	m_pImplData->m_bCompileIsComplete = false;
	//optimization and c runtime
#ifdef _DEBUG
	std::string flags = "/nologo /Zi /FC /MDd /LDd ";
#else
	std::string flags = "/nologo /Zi /FC /MD /LD ";	//also need debug information in release
#endif

	RCppOptimizationLevel optimizationLevel = GetActualOptimizationLevel( compilerOptions_.optimizationLevel );
	switch( optimizationLevel )
	{
	case RCCPPOPTIMIZATIONLEVEL_DEFAULT:
		assert(false);
	case RCCPPOPTIMIZATIONLEVEL_DEBUG:
		flags += "/Od ";
		break;
	case RCCPPOPTIMIZATIONLEVEL_PERF:
		flags += "/O2 /Oi ";

// Add improved debugging options if available: http://randomascii.wordpress.com/2013/09/11/debugging-optimized-codenew-in-visual-studio-2012/
#if   (_MSC_VER >= 1700)
		flags += "/d2Zi+ ";
#endif
		break;
	case RCCPPOPTIMIZATIONLEVEL_NOT_SET:;
	}

	if( NULL == m_pImplData->m_CmdProcessInfo.hProcess )
	{
		m_pImplData->InitialiseProcess();
	}

	flags += compilerOptions_.compileOptions;
    flags += " ";

	std::string linkOptions;
	bool bHaveLinkOptions = ( 0 != compilerOptions_.linkOptions.length() );
	if( compilerOptions_.libraryDirList.size() ||  bHaveLinkOptions )
	{
		linkOptions = " /link ";
		for( size_t i = 0; i < compilerOptions_.libraryDirList.size(); ++i )
		{
			linkOptions += " /LIBPATH:\"" + compilerOptions_.libraryDirList[i].m_string + "\"";
		}

		if( bHaveLinkOptions )
		{
			linkOptions += compilerOptions_.linkOptions;
            linkOptions += " ";
		}
	}
    // faster linking if available: https://randomascii.wordpress.com/2015/07/27/programming-is-puzzles/
    #if   (_MSC_VER >= 1900)
        if( linkOptions.empty() )
        {
            linkOptions = " /link ";
        }
        linkOptions += "/DEBUG:FASTLINK ";
    #endif

	// Check for intermediate directory, create it if required
	// There are a lot more checks and robustness that could be added here
	if ( !compilerOptions_.intermediatePath.Exists() )
	{
		bool success = compilerOptions_.intermediatePath.CreateDir();
		if( success && m_pImplData->m_pLogger ) { m_pImplData->m_pLogger->LogInfo("Created intermediate folder \"%s\"\n", compilerOptions_.intermediatePath.c_str()); }
		else if( m_pImplData->m_pLogger ) { m_pImplData->m_pLogger->LogError("Error creating intermediate folder \"%s\"\n", compilerOptions_.intermediatePath.c_str()); }
	}


	//create include path search string
	std::string strIncludeFiles;
	for( size_t i = 0; i < compilerOptions_.includeDirList.size(); ++i )
	{
		strIncludeFiles += " /I \"" + compilerOptions_.includeDirList[i].m_string + "\"";
	}


	// When using multithreaded compilation, listing a file for compilation twice can cause errors, hence 
	// we do a final filtering of input here.
	// See http://msdn.microsoft.com/en-us/library/bb385193.aspx - "Source Files and Build Order"

	// Create compile path search string
	std::string strFilesToCompile;
	std::set<std::string> filteredPaths;
	for( size_t i = 0; i < filesToCompile_.size(); ++i )
	{
		std::string strPath = filesToCompile_[i].m_string;
		FileSystemUtils::ToLowerInPlace(strPath);

		std::set<std::string>::const_iterator it = filteredPaths.find(strPath);
		if (it == filteredPaths.end())
		{
			strFilesToCompile += " \"" + strPath + "\"";
			filteredPaths.insert(strPath);
		}
	}

	std::string strLinkLibraries;
	for( size_t i = 0; i < linkLibraryList_.size(); ++i )
	{
		strLinkLibraries += " \"" + linkLibraryList_[i].m_string + "\" ";
	}
	



char* pCharTypeFlags = "";
#ifdef UNICODE
	pCharTypeFlags = "/D UNICODE /D _UNICODE ";
#endif

	// /MP - use multiple processes to compile if possible. Only speeds up compile for multiple files and not link
	std::string cmdToSend = "cl " + flags + pCharTypeFlags
		+ " /MP /Fo\"" + compilerOptions_.intermediatePath.m_string + "\\\\\" "
		+ "/D WIN32 /EHa /Fe" + moduleName_.m_string;
	cmdToSend += " " + strIncludeFiles + " " + strFilesToCompile + strLinkLibraries + linkOptions
		+ "\necho ";
	if( m_pImplData->m_pLogger ) m_pImplData->m_pLogger->LogInfo( "%s", cmdToSend.c_str() ); // use %s to prevent any tokens in compile string being interpreted as formating
	cmdToSend += c_CompletionToken + "\n";
	WriteInput( m_pImplData->m_CmdProcessInputWrite, cmdToSend );
}


void GetPathsOfVisualStudioInstalls( std::vector<VSVersionInfo>* pVersions )
{
	//HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\<version>\Setup\VS\<edition>
	std::string keyName = "SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VC7";

	const size_t    NUMNAMESTOCHECK = 6;

    // supporting: VS2005, VS2008, VS2010, VS2011, VS2013, VS2015
    std::string     valueName[NUMNAMESTOCHECK] = {"8.0","9.0","10.0","11.0","12.0","14.0"};

    // we start searching for a compatible compiler from the current version backwards
    int startVersion = NUMNAMESTOCHECK - 1;
	//switch around prefered compiler to the one we've used to compile this file
	const unsigned int MSCVERSION = _MSC_VER;
	switch( MSCVERSION )
	{
	case 1400:	//VS 2005
		startVersion = 0;
		break;
	case 1500:	//VS 2008
		startVersion = 1;
		break;
	case 1600:	//VS 2010
		startVersion = 2;
		break;
	case 1700:	//VS 2012
		startVersion = 3;
		break;
	case 1800:	//VS 2013
		startVersion = 4;
		break;
	case 1900:	//VS 2015
		startVersion = 5;
		break;
	default:
		assert( false ); //unsupported compiler, find MSCVERSION to add case, increase NUMNAMESTOCHECK and add valueName.
	}



	char value[MAX_PATH];
	DWORD size = MAX_PATH;

	HKEY key;
	LONG retKeyVal = RegOpenKeyExA(
				  HKEY_LOCAL_MACHINE,	//__in        HKEY hKey,
				  keyName.c_str(),			//__in_opt    LPCTSTR lpSubKey,
				  0,					//__reserved  DWORD ulOptions,
				  KEY_READ | KEY_WOW64_32KEY,		//__in        REGSAM samDesired,
				  &key					//__out       PHKEY phkResult
				);

    int loopCount = 1;
    if( startVersion != NUMNAMESTOCHECK - 1 )
    {
        // we potentially need to restart search from top
        loopCount = 2;
    }
    for( int loop = 0; loop < loopCount; ++loop )
    {
	    for( int i = startVersion; i >= 0; --i )
	    {

		    LONG retVal = RegQueryValueExA(
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
			    pVersions->push_back( vInfo );
		    }
	    }
        startVersion =  NUMNAMESTOCHECK - 1; // if we loop around again make sure it's from the top
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
