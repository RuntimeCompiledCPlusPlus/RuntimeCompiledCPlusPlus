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

#pragma once

#include <vector>
#include <string>
#include "Compiler.h"

#include "FileSystemUtils.h"

class BuildTool
{
public:
	BuildTool();
	~BuildTool();
	void Initialise( ICompilerLogger * pLogger );

    // Clean - cleans up the intermediate files
    void Clean( const FileSystemUtils::Path& temporaryPath_ ) const;

	struct FileToBuild
	{
		FileToBuild( const FileSystemUtils::Path& filePath_ )
			: filePath( filePath_ )
			, forceCompile( false )
		{
		}
		FileToBuild( const FileSystemUtils::Path& filePath_, bool forceCompile_ )
			: filePath( filePath_ )
			, forceCompile( forceCompile_ )
		{
		}
		FileSystemUtils::Path	filePath;
		bool					forceCompile; //if true the file is compiled even if object file is present
	};

	void BuildModule( const std::vector<FileToBuild>&		buildFileList_, 
					  const CompilerOptions&				compilerOptions_,
					  std::vector<FileSystemUtils::Path>	linkLibraryList_,
                      const FileSystemUtils::Path&			moduleName_ );

	bool GetIsComplete()
	{
		return m_Compiler.GetIsComplete();
	}

    void SetFastCompileMode( bool bFast )
    {
        m_Compiler.SetFastCompileMode( bFast );
    }
    

private:
	Compiler                    m_Compiler;
	ICompilerLogger*            m_pLogger;
};

