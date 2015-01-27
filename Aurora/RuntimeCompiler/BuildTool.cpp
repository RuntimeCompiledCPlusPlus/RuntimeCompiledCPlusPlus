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
#include <algorithm>
#include "ICompilerLogger.h"

using namespace std;
using namespace FileSystemUtils;

BuildTool::BuildTool()
{
}


BuildTool::~BuildTool()
{
}

void BuildTool::Clean( const FileSystemUtils::Path& temporaryPath_ ) const
{
	// Remove any existing intermediate directory
	FileSystemUtils::PathIterator pathIter( temporaryPath_ );
	std::string obj_extension = m_Compiler.GetObjectFileExtension();
	while( ++pathIter )
	{
		if( pathIter.GetPath().Extension() == obj_extension )
		{
			if( m_pLogger )
			{
				m_pLogger->LogInfo( "Deleting temp RCC++ obj file: %s\n", pathIter.GetPath().c_str() );
			}
			pathIter.GetPath().Remove();
		}
	}
}


void BuildTool::Initialise( ICompilerLogger * pLogger )
{
	m_pLogger = pLogger;
	m_Compiler.Initialise(pLogger);
}

void BuildTool::BuildModule( const std::vector<FileToBuild>&		buildFileList_, 
							 const CompilerOptions&					compilerOptions_,
							 std::vector<FileSystemUtils::Path>		linkLibraryList_,
							 const FileSystemUtils::Path&			moduleName_  )
{
	// Initial version is very basic, simply compiles them.
	Path objectFileExtension = m_Compiler.GetObjectFileExtension();
	vector<Path> compileFileList;			// List of files we pass to the compiler
	compileFileList.reserve( buildFileList_.size() );
	vector<Path> forcedCompileFileList;		// List of files which must be compiled even if object file exists
	vector<Path> nonForcedCompileFileList;	// List of files which can be linked if already compiled

	// Seperate into seperate file lists of force and non-forced,
	// so we can ensure we don't have the same file in both
	for( size_t i = 0; i < buildFileList_.size(); ++i )
	{
		Path buildFile = buildFileList_[i].filePath;
		if( buildFileList_[i].forceCompile )
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

	// runtime folder needs to be aware of compilation level and debug/

	// Add non forced files, but only if they don't exist in forced compile list
	for( size_t i = 0; i < nonForcedCompileFileList.size(); ++i )
	{
		Path buildFile = nonForcedCompileFileList[i];
		if( find( forcedCompileFileList.begin(), forcedCompileFileList.end(), buildFile ) == forcedCompileFileList.end() )
		{
			// Check if we have a pre-compiled object version of this file, and if so use that.
			Path objectFileName = compilerOptions_.intermediatePath / buildFile.Filename();
			objectFileName.ReplaceExtension(objectFileExtension.c_str());

			if( objectFileName.Exists() && buildFile.Exists() )
            {
                FileSystemUtils::filetime_t objTime = objectFileName.GetLastWriteTime();
                if( objTime > buildFile.GetLastWriteTime() )
 			    {
                    // we only want to use the object file if it's newer than the source file
				    buildFile = objectFileName;
			    }
            }
			compileFileList.push_back(buildFile);
		}
	}

	m_Compiler.RunCompile( compileFileList, compilerOptions_, linkLibraryList_, moduleName_ );
}
