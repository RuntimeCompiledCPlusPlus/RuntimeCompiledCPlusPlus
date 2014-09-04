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

#include "Compiler.h"

#include <string>
#include <vector>
#include <iostream>
#include "assert.h"
#include <sys/wait.h>

#include "ICompilerLogger.h"

using namespace std;
const char	c_CompletionToken[] = "_COMPLETION_TOKEN_" ;

class PlatformCompilerImplData
{
public:
	PlatformCompilerImplData()
		: m_bCompileIsComplete( false )
        , m_pLogger( 0 )
        , m_ChildForCompilationPID( 0 )
	{
        m_PipeStdOut[0] = 0;
        m_PipeStdOut[1] = 1;
        m_PipeStdErr[0] = 0;
        m_PipeStdErr[1] = 1;
	}

	std::string			m_intermediatePath;
	volatile bool		m_bCompileIsComplete;
	ICompilerLogger*	m_pLogger;
    pid_t               m_ChildForCompilationPID;
    int                 m_PipeStdOut[2];
    int                 m_PipeStdErr[2];
};

Compiler::Compiler() 
	: m_pImplData( 0 )
{
}

Compiler::~Compiler()
{
}

std::string Compiler::GetObjectFileExtension() const
{
	return ".o";
}

bool Compiler::GetIsComplete() const
{
    if( !m_pImplData->m_bCompileIsComplete && m_pImplData->m_ChildForCompilationPID )
    {
        
        // check for whether process is closed
        int procStatus;
        pid_t ret = waitpid( m_pImplData->m_ChildForCompilationPID, &procStatus, WNOHANG);
        if( ret && ( WIFEXITED(procStatus) || WIFSIGNALED(procStatus) ) )
        {
            m_pImplData->m_bCompileIsComplete = true;
            m_pImplData->m_ChildForCompilationPID = 0;
 
            // get output and log
            if( m_pImplData->m_pLogger )
            {
                const size_t buffSize = 256 * 80; //should allow for a few lines...
                char buffer[buffSize];
                ssize_t numread = 0;
                while( ( numread = read( m_pImplData->m_PipeStdOut[0], buffer, buffSize-1 ) ) > 0 )
                {
                    buffer[numread] = 0;
                    m_pImplData->m_pLogger->LogInfo( buffer );
                }
                
                while( ( numread = read( m_pImplData->m_PipeStdErr[0], buffer, buffSize-1 ) )> 0 )
                {
                    buffer[numread] = 0;
                    m_pImplData->m_pLogger->LogError( buffer );    //TODO: seperate warnings from errors.
                }
            }

            // close the pipes as this process no longer needs them.
            close( m_pImplData->m_PipeStdOut[0] );
            m_pImplData->m_PipeStdOut[0] = 0;
            close( m_pImplData->m_PipeStdErr[0] );
            m_pImplData->m_PipeStdErr[0] = 0;
        }
    }
	return m_pImplData->m_bCompileIsComplete;
}

void Compiler::Initialise( ICompilerLogger * pLogger )
{

    m_pImplData = new PlatformCompilerImplData;
    m_pImplData->m_pLogger = pLogger;
	m_pImplData->m_intermediatePath = "./Runtime";

	// Remove any existing intermediate directory
    /*
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
     */

}

FileSystemUtils::Path Compiler::GetRuntimeIntermediatePath() const
{
    return m_pImplData->m_intermediatePath;
}

void Compiler::RunCompile( const std::vector<FileSystemUtils::Path>& filesToCompile,
					 const std::vector<FileSystemUtils::Path>& includeDirList,
					 const std::vector<FileSystemUtils::Path>& libraryDirList,
                     const std::vector<FileSystemUtils::Path>& linkLibraryList,
                     RCppOptimizationLevel optimizationLevel_,
					 const char* pCompileOptions,
					 const char* pLinkOptions,
					 const FileSystemUtils::Path& outputFile )
{
    //NOTE: Currently doesn't check if a prior compile is ongoing or not, which could lead to memory leaks
 	m_pImplData->m_bCompileIsComplete = false;
    
    //create pipes
    if ( pipe( m_pImplData->m_PipeStdOut ) != 0 )
    {
        if( m_pImplData->m_pLogger )
        {
            m_pImplData->m_pLogger->LogError( "Error in Compiler::RunCompile, cannot create pipe - perhaps insufficient memory?\n");
        }
        return;
    }
    //create pipes
    if ( pipe( m_pImplData->m_PipeStdErr ) != 0 )
    {
        if( m_pImplData->m_pLogger )
        {
            m_pImplData->m_pLogger->LogError( "Error in Compiler::RunCompile, cannot create pipe - perhaps insufficient memory?\n");
        }
        return;
    }
    
    pid_t retPID;
    switch( retPID = fork() )
    {
        case -1: // error, no fork
            if( m_pImplData->m_pLogger )
            {
                m_pImplData->m_pLogger->LogError( "Error in Compiler::RunCompile, cannot fork() process - perhaps insufficient memory?\n");
            }
            return;
        case 0: // child process - carries on below.
            break;
        default: // current process - returns to allow application to run whilst compiling
            close( m_pImplData->m_PipeStdOut[1] );
            m_pImplData->m_PipeStdOut[1] = 0;
            close( m_pImplData->m_PipeStdErr[1] );
            m_pImplData->m_PipeStdErr[1] = 0;
            m_pImplData->m_ChildForCompilationPID = retPID;
           return;
    }
    
    //duplicate the pipe to stdout, so output goes to pipe
    dup2( m_pImplData->m_PipeStdErr[1], STDERR_FILENO );
    dup2( m_pImplData->m_PipeStdOut[1], STDOUT_FILENO );
    close( m_pImplData->m_PipeStdOut[0] );
    m_pImplData->m_PipeStdOut[0] = 0;
    close( m_pImplData->m_PipeStdErr[0] );
    m_pImplData->m_PipeStdErr[0] = 0;

#ifdef __APPLE__
	#ifdef DEBUG
        #ifndef __LP64__
            std::string compileString = "clang++ -g -O0 -m32 -fvisibility=hidden -Xlinker -dylib ";
        #else
            std::string compileString = "clang++ -g -O0 -fvisibility=hidden -Xlinker -dylib ";
        #endif
    #else
        #ifndef __LP64__
            std::string compileString = "clang++ -g -Os -m32 -fvisibility=hidden -Xlinker -dylib ";
        #else
            std::string compileString = "clang++ -g -Os -fvisibility=hidden -Xlinker -dylib ";
        #endif
	#endif
#else
	// NOTE: we do not need the COMPILE_PATH variable to be set here, as the filenames passed in are all full paths.
	
    #ifdef DEBUG
        #ifndef __LP64__
            std::string compileString = "g++ -g -m32 -fPIC -fvisibility=hidden -shared ";
       #else
            std::string compileString = "g++ -g -fPIC -fvisibility=hidden -shared ";
        #endif
    #else
        #ifndef __LP64__
            std::string compileString = "g++ -g -m32 -fPIC -fvisibility=hidden -shared ";
        #else
            std::string compileString = "g++ -g -fPIC -fvisibility=hidden -shared ";
        #endif
    #endif
#endif //__APPLE__

#ifdef DEBUG
	optimizationLevel_ = RCCPPOPTIMIZATIONLEVEL_DEBUG;
#else
	optimizationLevel_ = RCCPPOPTIMIZATIONLEVEL_PERF;
#endif
	switch( optimizationLevel_ )
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
    
    // output file
    compileString += "-o " + outputFile.m_string + " ";


	if( pCompileOptions )
	{
		compileString += pCompileOptions;
	}
	if( pLinkOptions )
	{
		compileString += pLinkOptions;
	}
	
    // files to compile
    for( size_t i = 0; i < filesToCompile.size(); ++i )
	{
        compileString += "\"" + filesToCompile[i].m_string + "\" ";
    }
    
    // libraries to link
    for( size_t i = 0; i < linkLibraryList.size(); ++i )
	{
        compileString += " " + linkLibraryList[i].m_string + " ";
    }
    
    
    std::cout << compileString << std::endl << std::endl;

    execl("/bin/sh", "sh", "-c", compileString.c_str(), (const char*)NULL);
}
