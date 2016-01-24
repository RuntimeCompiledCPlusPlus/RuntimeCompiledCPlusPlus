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

#include "Compiler_PlatformWindows.h"
#include <process.h>

#include "ICompilerLogger.h"


#define ERROR_EXIT \
if( hOutputReadTmp )\
{\
    CloseHandle( hOutputReadTmp );\
}\
if( hOutputWrite )\
{\
    CloseHandle( hOutputWrite );\
}\
if( hErrorWrite )\
{\
    CloseHandle( hErrorWrite );\
}\
return


PlatformCompilerImplData::PlatformCompilerImplData()
    : m_bCompileIsComplete( false )
    , m_CmdProcessOutputRead( NULL )
    , m_CmdProcessInputWrite( NULL )
{
    ZeroMemory( &m_CmdProcessInfo, sizeof(m_CmdProcessInfo) );
}

void PlatformCompilerImplData::InitialiseProcess()
{
    //init compile process
    STARTUPINFOW				si;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);

    // Set up the security attributes struct.
    SECURITY_ATTRIBUTES sa;
    sa.nLength= sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;


    // Create the child output pipe.
    //redirection of output
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    HANDLE hOutputReadTmp = NULL, hOutputWrite  = NULL, hErrorWrite = NULL;
    if (!CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,20*1024))
    {
        if( m_pLogger ) m_pLogger->LogError("[RuntimeCompiler] Failed to create output redirection pipe\n");
        ERROR_EXIT;
    }
    si.hStdOutput = hOutputWrite;

    // Create a duplicate of the output write handle for the std error
    // write handle. This is necessary in case the child application
    // closes one of its std output handles.
    if (!DuplicateHandle(GetCurrentProcess(),hOutputWrite,
                           GetCurrentProcess(),&hErrorWrite,0,
                           TRUE,DUPLICATE_SAME_ACCESS))
    {
        if( m_pLogger ) m_pLogger->LogError("[RuntimeCompiler] Failed to duplicate error output redirection pipe\n");
        ERROR_EXIT;
    }
    si.hStdError = hErrorWrite;


    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    if( si.hStdOutput )
    {
         if (!DuplicateHandle(GetCurrentProcess(),hOutputReadTmp,
                               GetCurrentProcess(),
                               &m_CmdProcessOutputRead, // Address of new handle.
                               0,FALSE, // Make it uninheritable.
                               DUPLICATE_SAME_ACCESS))
         {
               if( m_pLogger ) m_pLogger->LogError("[RuntimeCompiler] Failed to duplicate output read pipe\n");
               ERROR_EXIT;
         }
        CloseHandle( hOutputReadTmp );
        hOutputReadTmp = NULL;
    }


    HANDLE hInputRead,hInputWriteTmp;
    // Create a pipe for the child process's STDIN.
    if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 4096))
    {
        if( m_pLogger ) m_pLogger->LogError("[RuntimeCompiler] Failed to create input pipes\n");
        ERROR_EXIT;
    }
    si.hStdInput = hInputRead;

    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    if( si.hStdOutput )
    {
         if (!DuplicateHandle(GetCurrentProcess(),hInputWriteTmp,
                               GetCurrentProcess(),
                               &m_CmdProcessInputWrite, // Address of new handle.
                               0,FALSE, // Make it uninheritable.
                               DUPLICATE_SAME_ACCESS))
         {
               if( m_pLogger ) m_pLogger->LogError("[RuntimeCompiler] Failed to duplicate input write pipe\n");
               ERROR_EXIT;
         }
    }
    /*
    // Ensure the write handle to the pipe for STDIN is not inherited.
    if ( !SetHandleInformation(hInputWrite, HANDLE_FLAG_INHERIT, 0) )
    {
        m_pLogger->LogError("[RuntimeCompiler] Failed to make input write pipe non inheritable\n");
        ERROR_EXIT;
    }
    */

    wchar_t* pCommandLine = L"cmd /q";
    //CreateProcessW won't accept a const pointer, so copy to an array
    wchar_t pCmdLineNonConst[1024];
    wcscpy_s( pCmdLineNonConst, pCommandLine );
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
    _beginthread( ReadAndHandleOutputThread, 0, this ); //this will exit when process for compile is closed


    ERROR_EXIT;
}

void PlatformCompilerImplData::CleanupProcessAndPipes()
{
    // do not reset m_bCompileIsComplete and other members here, just process and pipes
    if(  m_CmdProcessInfo.hProcess )
    {
        TerminateProcess( m_CmdProcessInfo.hProcess, 0 );
        TerminateThread( m_CmdProcessInfo.hThread, 0 );
        CloseHandle( m_CmdProcessInfo.hThread );
        ZeroMemory( &m_CmdProcessInfo, sizeof(m_CmdProcessInfo) );
        CloseHandle( m_CmdProcessInputWrite );
        m_CmdProcessInputWrite = 0;
        CloseHandle( m_CmdProcessOutputRead );
        m_CmdProcessOutputRead = 0;
    }

}

PlatformCompilerImplData::~PlatformCompilerImplData()
{
    CleanupProcessAndPipes();
}

Compiler::Compiler() 
	: m_pImplData( 0 )
    , m_bFastCompileMode( false )
{
}

Compiler::~Compiler()
{
	delete m_pImplData;
}

bool Compiler::GetIsComplete() const
{
    bool bComplete = m_pImplData->m_bCompileIsComplete;
    if( bComplete & !m_bFastCompileMode )
    {
        m_pImplData->CleanupProcessAndPipes();
    }
	return bComplete;
}

void ReadAndHandleOutputThread( LPVOID arg )
{
	PlatformCompilerImplData* pImpl = (PlatformCompilerImplData*)arg;

    CHAR lpBuffer[1024];
    DWORD nBytesRead;
 	bool bReadActive = true;
	bool bReadOneMore = false;
    while( bReadActive )
    {
		if (!ReadFile(pImpl->m_CmdProcessOutputRead,lpBuffer,sizeof(lpBuffer)-1,
										&nBytesRead,NULL) || !nBytesRead)
		{
			bReadActive = false;
			if (GetLastError() != ERROR_BROKEN_PIPE)	//broken pipe is OK
			{
				if( pImpl->m_pLogger ) pImpl->m_pLogger->LogError( "[RuntimeCompiler] Redirect of compile output failed on read\n" );
			}
		}
		else
		{
			// Display the characters read in logger.
			lpBuffer[nBytesRead]=0;

			//fist check for completion token...
			std::string buffer( lpBuffer );
			size_t found = buffer.find( c_CompletionToken );
			if( found != std::string::npos )
			{
				//we've found the completion token, which means we quit
				buffer = buffer.substr( 0, found );
				if( pImpl->m_pLogger ) pImpl->m_pLogger->LogInfo("[RuntimeCompiler] Complete\n");
				pImpl->m_bCompileIsComplete = true;
			}
			if( bReadActive || buffer.length() ) //don't output blank last line
			{
				//check if this is an error
				size_t errorFound = buffer.find( " : error " );
				size_t fatalErrorFound = buffer.find( " : fatal error " );
				if( ( errorFound != std::string::npos ) || ( fatalErrorFound != std::string::npos ) )
				{
					if( pImpl->m_pLogger ) pImpl->m_pLogger->LogError( "%s", buffer.c_str() );
				}
				else
				{
					if( pImpl->m_pLogger ) pImpl->m_pLogger->LogInfo( "%s", buffer.c_str() );
				}
			}
		}
     }

}

void WriteInput( HANDLE hPipeWrite, std::string& input  )
{
    DWORD nBytesWritten;
	DWORD length = (DWORD)input.length();
	WriteFile( hPipeWrite, input.c_str() , length, &nBytesWritten, NULL );
}
