#include "Compiler_PlatformWindows.h"
#include "ICompilerLogger.h"

#include <set>
#include <assert.h>

struct VSVersionInfo
{
    int				Version;
    std::string		Path;
};

void GetPathsOfVisualStudioInstalls( std::vector<VSVersionInfo>* pVersions );

class PlatformCompilerImplDataVS : public PlatformCompilerImplData
{
public:
    PlatformCompilerImplDataVS() : PlatformCompilerImplData(){}

    std::string			m_VSPath;
};

std::string Compiler::GetObjectFileExtension() const
{
    return ".obj";
}

void Compiler::Initialise( ICompilerLogger * pLogger )
{
    m_pImplData = new PlatformCompilerImplDataVS;
    m_pImplData->m_pLogger = pLogger;
    // get VS compiler path
    std::vector<VSVersionInfo> Versions;
    GetPathsOfVisualStudioInstalls( &Versions );

    if( !Versions.empty() )
    {
        ((PlatformCompilerImplDataVS*)m_pImplData)->m_VSPath = Versions[0].Path;
    }
    else
    {
        ((PlatformCompilerImplDataVS*)m_pImplData)->m_VSPath = "";
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
    if( ((PlatformCompilerImplDataVS*)m_pImplData)->m_VSPath.empty() )
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

        std::string cmdSetParams = "@PROMPT $ \n\"" + ((PlatformCompilerImplDataVS*)m_pImplData)->m_VSPath +
#ifndef _WIN64
        "Vcvarsall.bat\" x86\n";
#else
        "Vcvarsall.bat\" x86_amd64\n";
#endif

        //send initial set up command
        WriteInput(m_pImplData->m_CmdProcessInputWrite, cmdSetParams );
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
