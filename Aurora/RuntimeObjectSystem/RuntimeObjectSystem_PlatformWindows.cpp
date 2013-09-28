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

#include "RuntimeProtector.h"
#include "RuntimeObjectSystem.h"

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "WinBase.h"
#include "excpt.h"
#include <assert.h>

// windows includes can cause GetObject to be defined, we undefine it here.
#ifdef GetObject
    #undef GetObject
#endif

struct RuntimeObjectSystem::PlatformImpl
{

	enum ExceptionState
	{
		ES_PASS,
		ES_CATCH,
	};

	ExceptionState s_exceptionState;

	PlatformImpl()
		: s_exceptionState( ES_PASS )
	{
	}

	int RuntimeExceptionFilter()
	{
		if( !AmBeingDebugged() )
		{
			// if there's no debugger, we simply continue operating.
			// TODO: Should implement a method to ensure this can be
			// disabled so process crashes on end user machines
			return EXCEPTION_EXECUTE_HANDLER;
		}

		int result;
		switch (s_exceptionState)
		{
		case ES_PASS:
			// Let pass to debugger once, then catch it
			result = EXCEPTION_CONTINUE_SEARCH;
			s_exceptionState = ES_CATCH;
			break;
		case ES_CATCH:
			// Catch it now. Reset to catch in debugger again next time.
			result = EXCEPTION_EXECUTE_HANDLER;
			s_exceptionState = ES_PASS;
			break;
		default:;
			assert(false);
		}

		return result;
	}

	bool AmBeingDebugged()
	{
		if( IsDebuggerPresent() )
		{
			return true;
		}
		BOOL bRDebugPresent = FALSE;
		CheckRemoteDebuggerPresent( GetModuleHandle(NULL), &bRDebugPresent );
		if( FALSE == bRDebugPresent )
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	int SimpleExceptionFilter( void * nativeExceptionInfo, RuntimeProtector* pRuntimeProtector )
	{	
		EXCEPTION_RECORD *pRecord = ((LPEXCEPTION_POINTERS) nativeExceptionInfo)->ExceptionRecord;
		int nCode = pRecord->ExceptionCode;
		pRuntimeProtector->ExceptionInfo.Type = RuntimeProtector::ESE_Unknown;
		pRuntimeProtector->ExceptionInfo.Addr = 0;
	

		if (nCode == EXCEPTION_ACCESS_VIOLATION)
		{
			ULONG_PTR flavour = pRecord->ExceptionInformation[0];
			switch( flavour )
			{
			case 0: 
				pRuntimeProtector->ExceptionInfo.Type = RuntimeProtector::ESE_AccessViolationRead;
				pRuntimeProtector->ExceptionInfo.Addr = (void*)pRecord->ExceptionInformation[1];
				break;
			case 1:
				pRuntimeProtector->ExceptionInfo.Type = RuntimeProtector::ESE_AccessViolationWrite;
				pRuntimeProtector->ExceptionInfo.Addr = (void*)pRecord->ExceptionInformation[1];
				break;
			default:
				break;
			}
		}
		else if( nCode == EXCEPTION_ILLEGAL_INSTRUCTION )
		{
			pRuntimeProtector->ExceptionInfo.Type = RuntimeProtector::ESE_InvalidInstruction;
			pRuntimeProtector->ExceptionInfo.Addr = pRecord->ExceptionAddress;
		}

		if( !pRuntimeProtector->m_bHintAllowDebug )
		{
			// We don't want debugging to catch this
			return EXCEPTION_EXECUTE_HANDLER;
		}

		// Otherwise fall back
		return RuntimeExceptionFilter();
	}
};

void RuntimeObjectSystem::CreatePlatformImpl()
{
    m_pImpl = new PlatformImpl();
}
void RuntimeObjectSystem::DeletePlatformImpl()
{
    delete m_pImpl;
}

void RuntimeObjectSystem::SetProtectionEnabled( bool bProtectionEnabled_ )
{
    m_bProtectionEnabled = bProtectionEnabled_;
}


bool RuntimeObjectSystem::TryProtectedFunction( RuntimeProtector* pProtectedObject_ )
{
    bool bJustCaughtException = false;
    if( m_TotalLoadedModulesEver != pProtectedObject_->m_ModulesLoadedCount )
    {
        // clear exceptions if we've just loaded a new module
        pProtectedObject_->m_ModulesLoadedCount = m_TotalLoadedModulesEver;
    	pProtectedObject_->m_bHashadException = false;
    }
	if( m_bProtectionEnabled )
	{
        if( !pProtectedObject_->m_bHashadException )
        {
	        __try
            {
		        pProtectedObject_->ProtectedFunc();
	        }
            __except( m_pImpl->SimpleExceptionFilter( GetExceptionInformation(), pProtectedObject_ ) )
	        {
		        // If we hit any structured exception, exceptionInfo will be initialized
		        // If it's one we recognise and we hinted for no debugging, we'll go straight here, with info filled out
		        // If not we'll go to debugger first, then here
		        pProtectedObject_->m_bHashadException = true;
                bJustCaughtException = true;
	        }
         }
	}
	else
	{
    	pProtectedObject_->m_bHashadException = false;
		pProtectedObject_->ProtectedFunc();
	}
	return !bJustCaughtException;
}

bool RuntimeObjectSystem::TestBuildWaitAndUpdate()
{
    Sleep( 100 );
    MSG msg;
    while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
    {
        DispatchMessage( &msg );
    }
    return true;
}
