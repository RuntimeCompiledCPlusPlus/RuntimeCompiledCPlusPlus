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

#include <string>
#include <vector>

#include "FileSystemUtils.h"
#include "CompileOptions.h"

class PlatformCompilerImplData;
struct ICompilerLogger;

struct CompilerOptions
{
	std::vector<FileSystemUtils::Path>	includeDirList;
	std::vector<FileSystemUtils::Path>	libraryDirList;
	std::string							compileOptions;
	std::string							linkOptions;
  	RCppOptimizationLevel				optimizationLevel;
	FileSystemUtils::Path				baseIntermediatePath;
	FileSystemUtils::Path				intermediatePath;
	FileSystemUtils::Path				compilerLocation;
};

class Compiler
{
public:
	Compiler();
	~Compiler();
	void Initialise( ICompilerLogger * pLogger );

    // On Win32 the compile command line process can be preserved in between compiles for improved performance,
    // however this can result in Zombie processes and also prevent handles such as sockets from being closed.
    // This function is safe to call at any time, but will only have an effect on Win32 compiles from the second
    // compile on after the call (as the first must launch the process and set the VS environment).
    //
    // Defaults to m_bFastCompileMode = false
    void SetFastCompileMode( bool bFast )
    {
        m_bFastCompileMode = bFast;

        // call GetIsComplete() to ensure this stops process
        GetIsComplete();
    }

    std::string GetObjectFileExtension() const;
	void RunCompile( const std::vector<FileSystemUtils::Path>&	filesToCompile_,
                     const CompilerOptions&						compilerOptions_,
					 std::vector<FileSystemUtils::Path>			linkLibraryList_,
					 const FileSystemUtils::Path&				moduleName_  );


	bool GetIsComplete() const;
private:
	PlatformCompilerImplData* m_pImplData;
    bool                      m_bFastCompileMode;
};
