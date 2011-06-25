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

void BuildTool::BuildModule( const std::vector<boost::filesystem::path>& buildFileList,
							 const std::vector<boost::filesystem::path>& includeDirList,
							 const boost::filesystem::path& moduleName, bool bForce )
{
	//initial version is very basic, simply compiles them.
	path objectFileExtension = m_Compiler.GetObjectFileExtension();
	vector<path> compileFileList;

	path current = boost::filesystem::current_path();

	//m_pLogger->LogInfo("[RuntimeCompiler] Considering source files:\n");
	
	for( size_t i = 0; i < buildFileList.size(); ++i )
	{
		//TODO: should check if we have a pre-compiled object version of this file, and if so use that.
		path buildFile = buildFileList[i];
		path runtimeFolder = L"Runtime";
		path objectFileName = current/runtimeFolder/buildFile.leaf();
		objectFileName.replace_extension(objectFileExtension);

		if( !bForce && boost::filesystem::exists( objectFileName ) && boost::filesystem::exists( buildFile )
			&& boost::filesystem::last_write_time( objectFileName ) > boost::filesystem::last_write_time( buildFile ) )
		{
			buildFile = objectFileName;
		}

		compileFileList.push_back(buildFile);
		//m_pLogger->LogInfo("  %ls\n",buildFile.c_str());
	}

	//m_pLogger->LogInfo("[RuntimeCompiler] Using include paths:\n");
	//for( size_t i = 0; i < includeDirList.size(); ++i )
	//{
	//	m_pLogger->LogInfo("  %ls\n",includeDirList[i].wstring().c_str());
	//}

	m_Compiler.RunCompile( compileFileList, includeDirList, moduleName );
}
