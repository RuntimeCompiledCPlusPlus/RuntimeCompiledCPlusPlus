#include "Compiler_PlatformWindows.h"
#include <assert.h>
#include "ICompilerLogger.h"

std::string Compiler::GetObjectFileExtension() const
{
    return ".o";
}

void Compiler::Initialise( ICompilerLogger * pLogger )
{
    m_pImplData = new PlatformCompilerImplData;
    m_pImplData->m_pLogger = pLogger;
}


void Compiler::RunCompile( const std::vector<FileSystemUtils::Path>&	filesToCompile_,
               const CompilerOptions&			compilerOptions_,
               std::vector<FileSystemUtils::Path>		linkLibraryList_,
                const FileSystemUtils::Path&		moduleName_ )

{
    const std::vector<FileSystemUtils::Path>& includeDirList = compilerOptions_.includeDirList;
    const std::vector<FileSystemUtils::Path>& libraryDirList = compilerOptions_.libraryDirList;
    const char* pCompileOptions =  compilerOptions_.compileOptions.c_str();
    const char* pLinkOptions = compilerOptions_.linkOptions.c_str();

    std::string compilerLocation = compilerOptions_.compilerLocation.m_string;
    if (compilerLocation.size()==0)
    {
        compilerLocation = "g++ ";
    }

    //NOTE: Currently doesn't check if a prior compile is ongoing or not, which could lead to memory leaks
    m_pImplData->m_bCompileIsComplete = false;

    //create pipes
    if( NULL == m_pImplData->m_CmdProcessInfo.hProcess )
    {
        m_pImplData->InitialiseProcess();
    }

    std::string compileString = compilerLocation + " " + "-g -fvisibility=hidden -shared ";

    RCppOptimizationLevel optimizationLevel = GetActualOptimizationLevel( compilerOptions_.optimizationLevel );
    switch( optimizationLevel )
    {
    case RCCPPOPTIMIZATIONLEVEL_DEFAULT:
        assert(false);
    case RCCPPOPTIMIZATIONLEVEL_DEBUG:
        compileString += "-O0 ";
        break;
    case RCCPPOPTIMIZATIONLEVEL_PERF:
        compileString += "-Os ";
        break;
    case RCCPPOPTIMIZATIONLEVEL_NOT_SET:;
    }

    // Check for intermediate directory, create it if required
    // There are a lot more checks and robustness that could be added here
    if( !compilerOptions_.intermediatePath.Exists() )
    {
        bool success = compilerOptions_.intermediatePath.CreateDir();
        if( success && m_pImplData->m_pLogger ) { m_pImplData->m_pLogger->LogInfo("Created intermediate folder \"%s\"\n", compilerOptions_.intermediatePath.c_str()); }
        else if( m_pImplData->m_pLogger ) { m_pImplData->m_pLogger->LogError("Error creating intermediate folder \"%s\"\n", compilerOptions_.intermediatePath.c_str()); }
    }

    if( compilerOptions_.intermediatePath.Exists() )
    {
        // add save object files
        compileString = "cd \"" + compilerOptions_.intermediatePath.m_string + "\"\n" + compileString + " --save-temps ";
    }


    // include directories
    for( size_t i = 0; i < includeDirList.size(); ++i )
    {
        compileString += "-I\"" + includeDirList[i].m_string + "\" ";
    }

    // library and framework directories
    for( size_t i = 0; i < libraryDirList.size(); ++i )
    {
        compileString += "-L\"" + libraryDirList[i].m_string + "\" ";
        compileString += "-F\"" + libraryDirList[i].m_string + "\" ";
    }

    compileString += "-o \"" + moduleName_.m_string + "\" ";


    if( pCompileOptions )
    {
        compileString += pCompileOptions;
        compileString += " ";
    }
    if( pLinkOptions && strlen(pLinkOptions) )
    {
        compileString += "-Wl,";
        compileString += pLinkOptions;
        compileString += " ";
    }

    // files to compile
    for( size_t i = 0; i < filesToCompile_.size(); ++i )
    {
        compileString += "\"" + filesToCompile_[i].m_string + "\" ";
    }

    // libraries to link
    for( size_t i = 0; i < linkLibraryList_.size(); ++i )
    {
        compileString += " " + linkLibraryList_[i].m_string + " ";
    }


    if( m_pImplData->m_pLogger ) m_pImplData->m_pLogger->LogInfo( "%s", compileString.c_str() ); // use %s to prevent any tokens in compile string being interpreted as formating

    compileString += "\necho ";
    compileString += c_CompletionToken + "\n";
    WriteInput( m_pImplData->m_CmdProcessInputWrite, compileString );
}
