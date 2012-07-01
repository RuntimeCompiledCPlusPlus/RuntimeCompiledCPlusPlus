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

#include "BuildTool.h"
#include "Compiler.h"
#include <fstream>

#include "ICompilerLogger.h"

using namespace std;
using namespace boost::filesystem;

BuildTool::BuildTool()
{
}


BuildTool::~BuildTool()
{
}

void BuildTool::Initialise( ICompilerLogger * pLogger )
{
	m_pLogger = pLogger;
	m_Compiler.Initialise(pLogger);
}

void BuildTool::BuildModule( const std::vector<FileToBuild>& buildFileList,
							 const std::vector<boost::filesystem::path>& includeDirList,
							 const std::vector<boost::filesystem::path>& libraryDirList,
							 const char* pCompileOptions,
							 const char* pLinkOptions,
							 const boost::filesystem::path& moduleName )
{
	// Initial version is very basic, simply compiles them.
	path objectFileExtension = m_Compiler.GetObjectFileExtension();
	vector<path> compileFileList;			// List of files we pass to the compiler
	compileFileList.reserve( buildFileList.size() );
	vector<path> forcedCompileFileList;		// List of files which must be compiled even if object file exists
	vector<path> nonForcedCompileFileList;	// List of files which can be linked if already compiled

	path current = boost::filesystem::current_path();

	// Seperate into seperate file lists of force and non-forced,
	// so we can ensure we don't have the same file in both
	for( size_t i = 0; i < buildFileList.size(); ++i )
	{
		path buildFile = buildFileList[i].filePath;
		if( buildFileList[i].forceCompile )
		{
			if( find( forcedCompileFileList.begin(), forcedCompileFileList.end(), buildFile ) == forcedCompileFileList.end() )
			{
				forcedCompileFileList.push_back( buildFile );
			}
		}
		else
		{
			if( find( nonForcedCompileFileList.begin(), nonForcedCompileFileList.end(), buildFile ) == nonForcedCompileFileList.end() )
			{
				nonForcedCompileFileList.push_back( buildFile );
			}
		}
	}
	
	// Add all forced compile files to build list
	for( size_t i = 0; i < forcedCompileFileList.size(); ++i )
	{
		compileFileList.push_back(  forcedCompileFileList[i] );
	}

	// Add non forced files, but only if they don't exist in forced compile list
	for( size_t i = 0; i < nonForcedCompileFileList.size(); ++i )
	{
		path buildFile = nonForcedCompileFileList[i];
		if( find( forcedCompileFileList.begin(), forcedCompileFileList.end(), buildFile ) == forcedCompileFileList.end() )
		{
			// Check if we have a pre-compiled object version of this file, and if so use that.
			path runtimeFolder = L"Runtime";
			path objectFileName = current/runtimeFolder/buildFile.leaf();
			objectFileName.replace_extension(objectFileExtension);

			if( boost::filesystem::exists( objectFileName ) && boost::filesystem::exists( buildFile )
				&& boost::filesystem::last_write_time( objectFileName ) > boost::filesystem::last_write_time( buildFile ) )
			{
				buildFile = objectFileName;
			}

			compileFileList.push_back(buildFile);
		}
	}

	m_Compiler.RunCompile( compileFileList, includeDirList, libraryDirList, pCompileOptions, pLinkOptions, moduleName );
}
