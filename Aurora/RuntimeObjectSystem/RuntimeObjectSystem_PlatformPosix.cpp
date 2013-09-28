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

#ifdef __APPLE__
	// Mach ports requirements
	#include <mach/mach.h>
#endif

#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <string.h>

static __thread RuntimeProtector*   m_pCurrProtector    = 0; // for nested threaded handling, one per thread.

#ifdef __APPLE__
	static bool                         ms_bMachPortSet     = false;

	const size_t NUM_OLD_EXCEPTION_HANDLERS = 16;
	static mach_msg_type_number_t  old_count;
	static exception_mask_t        old_masks[NUM_OLD_EXCEPTION_HANDLERS];
	static mach_port_t             old_ports[NUM_OLD_EXCEPTION_HANDLERS];
	static exception_behavior_t    old_behaviors[NUM_OLD_EXCEPTION_HANDLERS];
	static thread_state_flavor_t   old_flavors[NUM_OLD_EXCEPTION_HANDLERS];
#endif

void RuntimeObjectSystem::CreatePlatformImpl()
{
#ifdef __APPLE__
    if( !ms_bMachPortSet )
    {
        // prevent OS X debugger from catching signals in a none re-catchable way
        task_get_exception_ports(mach_task_self(), EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION, old_masks, &old_count, old_ports, old_behaviors, old_flavors);
        task_set_exception_ports(mach_task_self(), EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION, MACH_PORT_NULL, EXCEPTION_DEFAULT, MACHINE_THREAD_STATE);
        ms_bMachPortSet = true;
    }
#endif
}

void RuntimeObjectSystem::DeletePlatformImpl()
{
}

void RuntimeObjectSystem::SetProtectionEnabled( bool bProtectionEnabled_ )
{
    m_bProtectionEnabled = bProtectionEnabled_;
#ifdef __APPLE__
    if( !m_bProtectionEnabled )
    {
        if( ms_bMachPortSet )
        {
            for( int i = 0; i < old_count; ++i )
            {
                task_set_exception_ports( mach_task_self(), old_masks[i], old_ports[i], old_behaviors[i], old_flavors[i]);
            }
        }
    }
    else
    {
        CreatePlatformImpl();
    }
#endif
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


bool RuntimeObjectSystem::TestBuildWaitAndUpdate()
{
    usleep( 100 * 1000 );
    return true;
}
