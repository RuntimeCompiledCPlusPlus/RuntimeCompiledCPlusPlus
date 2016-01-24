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

class ICompilerLogger;

const std::string	c_CompletionToken( "_COMPLETION_TOKEN_" );

void ReadAndHandleOutputThread( LPVOID arg );
void WriteInput( HANDLE hPipeWrite, std::string& input  );

class PlatformCompilerImplData
{
public:
    PlatformCompilerImplData();

    void InitialiseProcess();

    void CleanupProcessAndPipes();

    ~PlatformCompilerImplData();

    PROCESS_INFORMATION m_CmdProcessInfo;
    HANDLE				m_CmdProcessOutputRead;
    HANDLE				m_CmdProcessInputWrite;
    volatile bool		m_bCompileIsComplete;
    ICompilerLogger*	m_pLogger;
};
