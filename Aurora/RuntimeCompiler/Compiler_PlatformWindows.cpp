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

//
// Notes:
//   - We use a single intermediate directory for compiled .obj files, which means
//     we don't support compiling multiple files with the same name. Could fix this
//     with either mangling names to include paths,  or recreating folder structure
//
//

#ifdef _WIN32


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
	std::string		Path;
};

const std::string	c_CompletionToken( "_COMPLETION_TOKEN_" );

void GetPathsOfVisualStudioInstalls( std::vector<VSVersionInfo>* pVersions, ICompilerLogger * pLogger );

void ReadAndHandleOutputThread( LPVOID arg );

struct CmdProcess
{
	CmdProcess();
	~CmdProcess();

	void InitialiseProcess();
	void WriteInput(std::string& input);
	void CleanupProcessAndPipes();


	PROCESS_INFORMATION m_CmdProcessInfo;
	HANDLE				m_CmdProcessOutputRead;
	HANDLE				m_CmdProcessInputWrite;
	volatile bool		m_bIsComplete;
	ICompilerLogger*    m_pLogger;
	bool				m_bStoreCmdOutput;
	std::string         m_CmdOutput;
    FileSystemUtils::Path m_PathTempCLCommandFile;
};

class PlatformCompilerImplData
{
public:
	PlatformCompilerImplData();
	~PlatformCompilerImplData();

	std::string			m_VSPath;
	bool				m_bFindVS;
	CmdProcess          m_CmdProcess;
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
    bool bComplete = m_pImplData->m_CmdProcess.m_bIsComplete;
    if( bComplete & !m_bFastCompileMode )
    {
        m_pImplData->m_CmdProcess.CleanupProcessAndPipes();
    }
	return bComplete;
}

void Compiler::Initialise( ICompilerLogger * pLogger )
{
	m_pImplData = new PlatformCompilerImplData;
	m_pImplData->m_pLogger = pLogger;
	m_pImplData->m_CmdProcess.m_pLogger = pLogger;
}

static inline std::string ExpandEnvVars( const std::string& string_ )
{
    std::wstring temp = FileSystemUtils::_Win32Utf8ToUtf16( string_ );
    uint32_t numChars = ExpandEnvironmentStringsW( temp.c_str(), nullptr, 0 );
    std::wstring tempExpanded;
    tempExpanded.resize( ++numChars ); // documentation is a little unclear if null character is included
    uint32_t numCharsExpanded = ExpandEnvironmentStringsW( temp.c_str(), &tempExpanded[0], numChars );
    return FileSystemUtils::_Win32Utf16ToUtf8( tempExpanded );
}

void Compiler::RunCompile(	const std::vector<FileSystemUtils::Path>&	filesToCompile_,
							const CompilerOptions&						compilerOptions_,
							const std::vector<FileSystemUtils::Path>&	linkLibraryList_,
							const FileSystemUtils::Path&				moduleName_ )
{
	if( m_pImplData->m_bFindVS )
	{
		// get VS compiler path
		m_pImplData->m_bFindVS = false; // only run once
		std::vector<VSVersionInfo> Versions;
		GetPathsOfVisualStudioInstalls(&Versions, m_pImplData->m_pLogger);

		if (!Versions.empty())
		{
			m_pImplData->m_VSPath = Versions[0].Path;
		}
		else
		{
			m_pImplData->m_VSPath = "";
			if (m_pImplData->m_pLogger)
			{
				m_pImplData->m_pLogger->LogError("No Supported Compiler for RCC++ found.\n");
			}
		}
	}

    if( m_pImplData->m_VSPath.empty() )
    {
        if (m_pImplData->m_pLogger) { m_pImplData->m_pLogger->LogError("No Supported Compiler for RCC++ found, cannot compile changes.\n"); }
    	m_pImplData->m_CmdProcess.m_bIsComplete = true;
        return;
    }
	m_pImplData->m_CmdProcess.m_bIsComplete = false;
	//optimization and c runtime
#ifdef _DEBUG
	std::string flags = "/nologo /Z7 /FC /utf-8 /MDd /LDd ";
#else
	std::string flags = "/nologo /Z7 /FC /utf-8 /MD /LD ";	//also need debug information in release
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
		flags += "/O2 ";

// Add improved debugging options if available: http://randomascii.wordpress.com/2013/09/11/debugging-optimized-codenew-in-visual-studio-2012/
#if   (_MSC_VER >= 1700)
		flags += "/d2Zi+ ";
#endif
		break;
	case RCCPPOPTIMIZATIONLEVEL_NOT_SET:;
	case RCCPPOPTIMIZATIONLEVEL_SIZE:;
	}

	if( NULL == m_pImplData->m_CmdProcess.m_CmdProcessInfo.hProcess )
	{
		m_pImplData->m_CmdProcess.InitialiseProcess();
#ifndef _WIN64
		std::string cmdSetParams = "\"" + m_pImplData->m_VSPath + "Vcvarsall.bat\" x86\n";
#else
		std::string cmdSetParams = "\"" + m_pImplData->m_VSPath + "Vcvarsall.bat\" x86_amd64\n";
#endif
		//send initial set up command
		m_pImplData->m_CmdProcess.WriteInput(cmdSetParams);
        m_pImplData->m_CmdProcess.WriteInput( std::string("chcp 65001\n") ); // set utf-8 console locale
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
			linkOptions += " /LIBPATH:\"" + ExpandEnvVars( compilerOptions_.libraryDirList[i].m_string ) + "\"";
		}

		if( bHaveLinkOptions )
		{
			linkOptions += compilerOptions_.linkOptions;
            linkOptions += " ";
		}
	}

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
		strIncludeFiles += " /I \"" + ExpandEnvVars( compilerOptions_.includeDirList[i].m_string ) + "\"";
	}


	// When using multithreaded compilation, listing a file for compilation twice can cause errors, hence 
	// we do a final filtering of input here.
	// See http://msdn.microsoft.com/en-us/library/bb385193.aspx - "Source Files and Build Order"

	// Create compile path search string
	std::string strFilesToCompile;
	std::set<std::string> filteredPaths;
	for( size_t i = 0; i < filesToCompile_.size(); ++i )
	{
		std::string strPath = ExpandEnvVars( filesToCompile_[i].m_string );
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
		strLinkLibraries += " \"" + ExpandEnvVars( linkLibraryList_[i].m_string ) + "\" ";
	}
	



#ifdef UNICODE
	const char* pCharTypeFlags = "/D UNICODE /D _UNICODE ";
#else
	const char* pCharTypeFlags = "";
#endif

	std::string compilerLocation = compilerOptions_.compilerLocation.m_string;
    if( compilerLocation.size() == 0 ) {
#if defined __clang__
	#ifndef _WIN64
	std::string arch = "-m32 ";
	#else
	std::string arch = "-m64 ";
	#endif
	compilerLocation = "\"%VCINSTALLDIR%Tools\\Llvm\\bin\\clang-cl\" ";
	compilerLocation += arch;
#else
	// full path and arch is not required as cl compiler already initialized by Vcvarsall.bat
	compilerLocation = "cl ";
#endif
	}
    else if( compilerLocation.back() != ' ' ) {
        // we need a space at the end
        compilerLocation += " ";
    }

	// /MP - use multiple processes to compile if possible. Only speeds up compile for multiple files and not link
	std::string clCommandOptions = flags + pCharTypeFlags
		+ " /MP /Fo\"" + compilerOptions_.intermediatePath.m_string + "\\\\\" "
		+ "/D WIN32 /EHa /Fe" + moduleName_.m_string;
	clCommandOptions += " " + strIncludeFiles + " " + strFilesToCompile + strLinkLibraries + linkOptions;
	if( m_pImplData->m_pLogger ) m_pImplData->m_pLogger->LogInfo( "%s", clCommandOptions.c_str() ); // use %s to prevent any tokens in compile string being interpreted as formating

    // Write the compile line out to a seperate Command File to allow large line length and also utf-8
    FileSystemUtils::Path pathTempCLCommandFile = compilerOptions_.intermediatePath / "ClCommandFile.temp";
    FILE* file = FileSystemUtils::fopen( pathTempCLCommandFile, "wb" );
    if( file )
    {
        m_pImplData->m_CmdProcess.m_PathTempCLCommandFile = pathTempCLCommandFile;
        uint8_t utf_8_BOM[] = {0xEF,0xBB,0xBF};
        fwrite( utf_8_BOM, 1, sizeof( utf_8_BOM ), file );
        fwrite( clCommandOptions.c_str(), 1, clCommandOptions.size(), file );
        fclose( file );
	    std::string cmdToSend = compilerLocation + " @" + pathTempCLCommandFile.GetOSShortForm().m_string;
        cmdToSend += "\necho ";
	    cmdToSend += c_CompletionToken + "\n";
	    m_pImplData->m_CmdProcess.WriteInput( cmdToSend );
    }
    else
    {
    	if( m_pImplData->m_pLogger ) m_pImplData->m_pLogger->LogInfo( "Could not create CL Command File %s\n  - Falling back to command line options\n", pathTempCLCommandFile.c_str() ); // use %s to prevent any tokens in compile string being interpreted as formating
        // cannot create the file - fallback to command line
        m_pImplData->m_CmdProcess.m_PathTempCLCommandFile.m_string.clear();
        std::string cmdToSend = compilerLocation + clCommandOptions;
        cmdToSend += "\necho ";
	    cmdToSend += c_CompletionToken + "\n";
	    m_pImplData->m_CmdProcess.WriteInput( cmdToSend );
    }
}

struct VSKey
{
	const char* keyName;
	const char* pathToAdd;
	HKEY        key;
};

struct VSVersionDiscoveryInfo
{
	const char* versionName;
	const char* versionNextName; // vswhere query requires a range, so we need the 'next' version name - store rather than use +1 in array as may not exist or be correct
	int         versionKey; // index into an array of VSKey values for the key, -1 for don't look
	bool        tryVSWhere; // can use VSWhere for versioning
};

void GetPathsOfVisualStudioInstalls( std::vector<VSVersionInfo>* pVersions, ICompilerLogger* pLogger )
{
	//e.g.: HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\<version>\Setup\VS\<edition>
	// to view 32bit keys on Windows use start->run and enter: %systemroot%\syswow64\regedit
	// as for 32bit keys need to run 32bit regedit.
	VSKey VS_KEYS[] = { {"SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VC7", "", NULL},
					    {"SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VS7", "VC\\Auxiliary\\Build\\", NULL} };
	int NUMVSKEYS = sizeof( VS_KEYS ) / sizeof( VSKey );

    // supporting: VS2005, VS2008, VS2010, VS2011, VS2013, VS2015, VS2017, VS2019
	// See https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B#Internal_version_numbering for version info
	VSVersionDiscoveryInfo VS_DISCOVERY_INFO[] = { {"8.0","9.0",0,false}, {"9.0","10.0",0,false}, {"10.0","11.0",0,false}, {"11.0","12.0",0,false}, {"12.0","13.0",0,false}, {"14.0","15.0",0,false}, {"15.0","16.0",1,true}, {"16.0","17.0",1,true}, {"17.0","18.0",1,true} };


	int NUMNAMESTOCHECK = sizeof( VS_DISCOVERY_INFO ) / sizeof( VSVersionDiscoveryInfo );
    // we start searching for a compatible compiler from the current version backwards
    int startVersion = NUMNAMESTOCHECK - 1;

#if !defined __clang__ // do not check _MSC_VER for clang as this reports version 1800 by default
	//switch around prefered compiler to the one we've used to compile this file
	const unsigned int MSCVERSION = _MSC_VER;
	bool bMSCVersionFound = true; // default to true as only one false case
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
	case 1910:	//VS 2017
	case 1911:	//VS 2017
	case 1912:	//VS 2017
	case 1913:	//VS 2017
	case 1914:	//VS 2017
    case 1915:  //VS 2017
    case 1916:  //VS 2017
		startVersion = 6;
		break;
	case 1920: // VS 2019
	case 1921: // VS 2019
	case 1922: // VS 2019
	case 1923: // VS 2019
	case 1924: // VS 2019
	case 1925: // VS 2019
	case 1926: // VS 2019
	case 1927: // VS 2019
	case 1928: // VS 2019
	case 1929: // VS 2019
		startVersion = 7;
		break;
	case 1930: // VS 2022
	case 1931: // VS 2022
	case 1932: // VS 2022
	case 1933: // VS 2022
	case 1934: // VS 2022
	case 1935: // VS 2022
	case 1936: // VS 2022
	case 1937: // VS 2022
	case 1938: // VS 2022
	case 1939: // VS 2022
	case 1940: // VS 2022
		startVersion = 8;
		break;
	default:
		bMSCVersionFound = false;
		if( pLogger )
		{
			pLogger->LogWarning("WARNING: VS Compiler with _MSC_VER %d potentially not supported. Defaulting to version %s.\n",MSCVERSION, VS_DISCOVERY_INFO[startVersion].versionName);
		}
	}
#endif


	char value[MAX_PATH];
	DWORD size = MAX_PATH;

	for( int i =0; i < NUMVSKEYS; ++i )
	{
		LONG retKeyVal = RegOpenKeyExA(
					  HKEY_LOCAL_MACHINE,	//__in        HKEY hKey,
					  VS_KEYS[i].keyName,			//__in_opt    LPCTSTR lpSubKey,
					  0,					//__reserved  DWORD ulOptions,
					  KEY_READ | KEY_WOW64_32KEY,		//__in        REGSAM samDesired,
					  &VS_KEYS[i].key					//__out       PHKEY phkResult
					);
	}

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
			VSVersionDiscoveryInfo vsinfo = VS_DISCOVERY_INFO[i];
			VSKey                  vskey  = VS_KEYS[ vsinfo.versionKey ];

			if( vsinfo.tryVSWhere )
			{
				CmdProcess cmdProc;
				cmdProc.m_pLogger = pLogger;
				cmdProc.InitialiseProcess();
				cmdProc.m_bStoreCmdOutput = true;
				cmdProc.m_CmdOutput = "";
				std::string maxVersion;
				if( !bMSCVersionFound && i == startVersion )
				{
					// open ended max version so we find the latest
					maxVersion =  "";
				}
				else
				{
					maxVersion =  vsinfo.versionNextName;
				}
				std::string vsWhereQuery = "\"%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere\""
					                         " -version [" + std::string( vsinfo.versionName ) + "," + maxVersion + ") "          // [min,max) format for version names
					                         " -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
					                         " -property installationPath"
							                 "\nexit\n";
				cmdProc.WriteInput( vsWhereQuery );
				WaitForSingleObject( cmdProc.m_CmdProcessInfo.hProcess, 2000 ); // max 2 secs
				// get the first non-empty substring
				size_t start = cmdProc.m_CmdOutput.find_first_not_of("\r\n", 0);
				if( start != std::string::npos )
				{
					size_t end = cmdProc.m_CmdOutput.find_first_of("\r\n", start);
					if( end == std::string::npos )
					{
						end = cmdProc.m_CmdOutput.length();
					}
					FileSystemUtils::Path path = cmdProc.m_CmdOutput.substr( start, end-start );
					if( path.m_string.length() && path.Exists() )
					{
						VSVersionInfo vInfo;
						vInfo.Path = path.m_string;
						vInfo.Path += "\\";
						vInfo.Path += vskey.pathToAdd;
						pVersions->push_back( vInfo );
						continue;
					}
				}
			}

		    LONG retVal = RegQueryValueExA(
				          vskey.key,					//__in         HKEY hKey,
					      vsinfo.versionName,	//__in_opt     LPCTSTR lpValueName,
					      NULL,					//__reserved   LPDWORD lpReserved,
					      NULL ,				//__out_opt    LPDWORD lpType,
					      (LPBYTE)value,			//__out_opt    LPBYTE lpData,
					      &size					//__inout_opt  LPDWORD lpcbData
					    );
		    if( ERROR_SUCCESS == retVal )
		    {
			    VSVersionInfo vInfo;
			    vInfo.Path = value;
				vInfo.Path += vskey.pathToAdd;
			    pVersions->push_back( vInfo );
		    }
	    }
        startVersion =  NUMNAMESTOCHECK - 1; // if we loop around again make sure it's from the top
    }

	for( int i =0; i < NUMVSKEYS; ++i )
	{
		RegCloseKey( VS_KEYS[i].key	 );
	}
	return;
}


void ReadAndHandleOutputThread( LPVOID arg )
{
	CmdProcess* pCmdProc = (CmdProcess*)arg;

	CHAR lpBuffer[1024];
	DWORD nBytesRead;
	bool bReadActive = true;
	while( bReadActive )
	{
		if( !ReadFile( pCmdProc->m_CmdProcessOutputRead,lpBuffer,sizeof(lpBuffer)-1,
										&nBytesRead,NULL) || !nBytesRead)
		{
			bReadActive = false;
			if( GetLastError() != ERROR_BROKEN_PIPE)	//broken pipe is OK
			{
				if(pCmdProc->m_pLogger ) pCmdProc->m_pLogger->LogError( "[RuntimeCompiler] Redirect of compile output failed on read\n" );
			}
		}
		else
		{
			// Add null termination
			lpBuffer[nBytesRead]=0;

			//fist check for completion token...
			std::string buffer( lpBuffer );
			size_t found = buffer.find( c_CompletionToken );
			if( found != std::string::npos )
			{
				//we've found the completion token, which means we quit
				buffer = buffer.substr( 0, found );
				if( !pCmdProc->m_bStoreCmdOutput && pCmdProc->m_pLogger ) pCmdProc->m_pLogger->LogInfo("[RuntimeCompiler] Complete\n");
				pCmdProc->m_bIsComplete = true;
			}
			if( bReadActive || buffer.length() ) //don't output blank last line
			{
				if( pCmdProc->m_bStoreCmdOutput )
				{
					pCmdProc->m_CmdOutput += buffer;
				}
				else
				{
					//check if this is an error
					size_t errorFound = buffer.find( " : error " );
					size_t fatalErrorFound = buffer.find( " : fatal error " );
					if( ( errorFound != std::string::npos ) || ( fatalErrorFound != std::string::npos ) )
					{
						if(pCmdProc->m_pLogger ) pCmdProc->m_pLogger->LogError( "%s", buffer.c_str() );
					}
					else
					{
						if(pCmdProc->m_pLogger ) pCmdProc->m_pLogger->LogInfo( "%s", buffer.c_str() );
					}
				}
			}
		}
	}
}

PlatformCompilerImplData::PlatformCompilerImplData()
	: m_bFindVS(true)
	, m_pLogger(NULL)
{
}

PlatformCompilerImplData::~PlatformCompilerImplData()
{
}

CmdProcess::CmdProcess()
	: m_CmdProcessOutputRead(NULL)
	, m_CmdProcessInputWrite(NULL)
	, m_bIsComplete(false)
	, m_pLogger(NULL)
	, m_bStoreCmdOutput(false)
{
	ZeroMemory(&m_CmdProcessInfo, sizeof(m_CmdProcessInfo));
}

void CmdProcess::InitialiseProcess()
{
	//init compile process
	STARTUPINFOW				si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	// Set up the security attributes struct.
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;


	// Create the child output pipe.
	//redirection of output
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	HANDLE hOutputReadTmp = NULL, hOutputWrite = NULL, hErrorWrite = NULL;
	if (!CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 20 * 1024))
	{
		if (m_pLogger) m_pLogger->LogError("[RuntimeCompiler] Failed to create output redirection pipe\n");
		goto ERROR_EXIT;
	}
	si.hStdOutput = hOutputWrite;

	// Create a duplicate of the output write handle for the std error
	// write handle. This is necessary in case the child application
	// closes one of its std output handles.
	if (!DuplicateHandle(GetCurrentProcess(), hOutputWrite,
		GetCurrentProcess(), &hErrorWrite, 0,
		TRUE, DUPLICATE_SAME_ACCESS))
	{
		if (m_pLogger) m_pLogger->LogError("[RuntimeCompiler] Failed to duplicate error output redirection pipe\n");
		goto ERROR_EXIT;
	}
	si.hStdError = hErrorWrite;


	// Create new output read handle and the input write handles. Set
	// the Properties to FALSE. Otherwise, the child inherits the
	// properties and, as a result, non-closeable handles to the pipes
	// are created.
	if (si.hStdOutput)
	{
		if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp,
			GetCurrentProcess(),
			&m_CmdProcessOutputRead, // Address of new handle.
			0, FALSE, // Make it uninheritable.
			DUPLICATE_SAME_ACCESS))
		{
			if (m_pLogger) m_pLogger->LogError("[RuntimeCompiler] Failed to duplicate output read pipe\n");
			goto ERROR_EXIT;
		}
		CloseHandle(hOutputReadTmp);
		hOutputReadTmp = NULL;
	}


	HANDLE hInputRead, hInputWriteTmp;
	// Create a pipe for the child process's STDIN. 
	if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 4096))
	{
		if (m_pLogger) m_pLogger->LogError("[RuntimeCompiler] Failed to create input pipes\n");
		goto ERROR_EXIT;
	}
	si.hStdInput = hInputRead;

	// Create new output read handle and the input write handles. Set
	// the Properties to FALSE. Otherwise, the child inherits the
	// properties and, as a result, non-closeable handles to the pipes
	// are created.
	if (si.hStdOutput)
	{
		if (!DuplicateHandle(GetCurrentProcess(), hInputWriteTmp,
			GetCurrentProcess(),
			&m_CmdProcessInputWrite, // Address of new handle.
			0, FALSE, // Make it uninheritable.
			DUPLICATE_SAME_ACCESS))
		{
			if (m_pLogger) m_pLogger->LogError("[RuntimeCompiler] Failed to duplicate input write pipe\n");
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

	const wchar_t* pCommandLine = L"cmd /q /K @PROMPT $";
	//CreateProcessW won't accept a const pointer, so copy to an array 
	wchar_t pCmdLineNonConst[1024];
	wcscpy_s(pCmdLineNonConst, pCommandLine);
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

	//launch threaded read.
	_beginthread(ReadAndHandleOutputThread, 0, this); //this will exit when process for compile is closed


ERROR_EXIT:
	if( hOutputReadTmp )
	{
		CloseHandle( hOutputReadTmp );
	}
	if( hOutputWrite )
	{
		CloseHandle(hOutputWrite);
	}
	if( hErrorWrite )
	{
		CloseHandle( hErrorWrite );
	}
}


void CmdProcess::WriteInput( std::string& input )
{
	DWORD nBytesWritten;
	DWORD length = (DWORD)input.length();
	WriteFile( m_CmdProcessInputWrite , input.c_str(), length, &nBytesWritten, NULL);
}

void CmdProcess::CleanupProcessAndPipes()
{
	// do not reset m_bIsComplete and other members here, just process and pipes and 
    if( !m_PathTempCLCommandFile.m_string.empty() && m_PathTempCLCommandFile.Exists() )
    {
        m_PathTempCLCommandFile.Remove();
        m_PathTempCLCommandFile.m_string.clear();
    }

	if( m_CmdProcessInfo.hProcess )
	{
		TerminateProcess(m_CmdProcessInfo.hProcess, 0);
		TerminateThread(m_CmdProcessInfo.hThread, 0);
		CloseHandle(m_CmdProcessInfo.hThread);
		ZeroMemory(&m_CmdProcessInfo, sizeof(m_CmdProcessInfo));
		CloseHandle(m_CmdProcessInputWrite);
		m_CmdProcessInputWrite = 0;
		CloseHandle(m_CmdProcessOutputRead);
		m_CmdProcessOutputRead = 0;
	}
}

CmdProcess::~CmdProcess()
{
	CleanupProcessAndPipes();
}

#endif // #ifdef _WIN32
