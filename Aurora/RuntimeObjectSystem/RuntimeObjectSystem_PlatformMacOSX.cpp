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

#include "RuntimeObjectSystem.h"
#include "RuntimeProtector.h"

// Mach ports requirements
#include <pthread.h>
#include <mach/mach.h>
#include <assert.h>
#include <signal.h>


static bool                         ms_bMachPortSet     = false;
static __thread RuntimeProtector*   m_pCurrProtector    = 0; // for nested threaded handling, one per thread.
    


void RuntimeObjectSystem::CreatePlatformImpl()
{
    if( !ms_bMachPortSet )
    {
        // prevent OS X debugger from catching signals in a none re-catchable way
        task_set_exception_ports(mach_task_self(), EXC_MASK_BAD_ACCESS, MACH_PORT_NULL, EXCEPTION_DEFAULT, MACHINE_THREAD_STATE);
        task_set_exception_ports(mach_task_self(), EXC_MASK_BAD_INSTRUCTION, MACH_PORT_NULL, EXCEPTION_DEFAULT, MACHINE_THREAD_STATE);
        ms_bMachPortSet = true;
    }
}

void RuntimeObjectSystem::DeletePlatformImpl()
{
}

void RuntimeObjectSystem::SetProtectionEnabled( bool bProtectionEnabled_ )
{
    m_bProtectionEnabled = bProtectionEnabled_;
}



void signalHandler(int sig, siginfo_t *info, void *context)
{
    // we only handle synchronous signals with this handler, so they come to the correct thread.
    assert( m_pCurrProtector );
    
    // store exception information
    switch( sig )
    {
        case SIGILL:
            m_pCurrProtector->ExceptionInfo.Type = RuntimeProtector::ESE_InvalidInstruction;
            break;
        case SIGBUS:
            m_pCurrProtector->ExceptionInfo.Type = RuntimeProtector::ESE_AccessViolation;
            break;
        case SIGSEGV:
            m_pCurrProtector->ExceptionInfo.Type = RuntimeProtector::ESE_AccessViolation;
            break;
        default: assert(false); //should not get here
    }
    m_pCurrProtector->ExceptionInfo.Addr = info->si_addr;
    longjmp(m_pCurrProtector->m_env, sig );
}

bool RuntimeObjectSystem::TryProtectedFunction( RuntimeProtector* pProtectedObject_ )
{
   if( !m_bProtectionEnabled )
   {
       pProtectedObject_->ProtectedFunc();
       return true;
   }
    
    // allow cascading by storing prev and current impl
    pProtectedObject_->m_pPrevious         = m_pCurrProtector;
    m_pCurrProtector                       = pProtectedObject_;
 
    struct sigaction oldAction[3]; // we need to store old actions, could remove for optimization

    bool bHasJustHadException = false;
    if( m_TotalLoadedModulesEver != pProtectedObject_->m_ModulesLoadedCount )
    {
        // clear exceptions if we've just loaded a new module
        pProtectedObject_->m_ModulesLoadedCount = m_TotalLoadedModulesEver;
    	pProtectedObject_->m_bHashadException = false;
    }

    if( !pProtectedObject_->m_bHashadException )
    {
        if( setjmp(m_pCurrProtector->m_env) )
        {
            pProtectedObject_->m_bHashadException = true;
            bHasJustHadException = true;
        }
        else
        {
            struct sigaction newAction;
            memset( &newAction, 0, sizeof( newAction ));
            newAction.sa_sigaction = signalHandler;
            newAction.sa_flags = SA_SIGINFO; //use complex signal hander function sa_sigaction not sa_handler
            sigaction(SIGILL, &newAction, &oldAction[0] );
            sigaction(SIGBUS, &newAction, &oldAction[1] );
            sigaction(SIGSEGV, &newAction, &oldAction[2] );
                
            pProtectedObject_->ProtectedFunc();
        }
        
        //reset
        sigaction(SIGILL, &oldAction[0], NULL );
        sigaction(SIGBUS, &oldAction[1], NULL );
        sigaction(SIGSEGV, &oldAction[2], NULL );
    }
    m_pCurrProtector = pProtectedObject_->m_pPrevious;
    return !bHasJustHadException;
}
