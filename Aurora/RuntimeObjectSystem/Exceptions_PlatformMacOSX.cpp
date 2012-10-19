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

#include "Exceptions.h"

// Mach ports requirements
#include <pthread.h>
#include <mach/mach.h>
#include <assert.h>
#include <setjmp.h>
#include <signal.h>


struct RuntimeProtector::Impl
{
    Impl( RuntimeProtector* pProtector )
        : m_pPrevImpl( 0 )
        , m_pProtector( pProtector )
    {
    }
    
    static bool             ms_bMachPortSet;
    static __thread Impl*   m_pCurrImpl; // for nested threaded handling, one per thread.
    
    jmp_buf                 m_env;
    Impl*                   m_pPrevImpl; // for nested threaded handling (thread not required)
    RuntimeProtector*       m_pProtector;
};

bool RuntimeProtector::Impl::ms_bMachPortSet = false;
__thread RuntimeProtector::Impl* RuntimeProtector::Impl::m_pCurrImpl = 0;

RuntimeProtector::RuntimeProtector()
    : m_pImpl( new Impl( this ) )
    , m_bHashadException( false )
{
    if( !Impl::ms_bMachPortSet )
    {
        // prevent OS X debugger from catching signals in a none re-catchable way
        task_set_exception_ports(mach_task_self(), EXC_MASK_BAD_ACCESS, MACH_PORT_NULL, EXCEPTION_DEFAULT, MACHINE_THREAD_STATE);
        task_set_exception_ports(mach_task_self(), EXC_MASK_BAD_INSTRUCTION, MACH_PORT_NULL, EXCEPTION_DEFAULT, MACHINE_THREAD_STATE);
    }
}


RuntimeProtector::~RuntimeProtector()
{
    delete m_pImpl;
}

void signalHandler(int sig, siginfo_t *info, void *context)
{
    // we only handle synchronous signals with this handler, so they come to the correct thread.
    assert( RuntimeProtector::Impl::m_pCurrImpl );
    
    // store exception information
    switch( sig )
    {
        case SIGILL:
            RuntimeProtector::Impl::m_pCurrImpl->m_pProtector->ExceptionInfo.Type = RuntimeProtector::ESE_InvalidInstruction;
            break;
        case SIGBUS:
            RuntimeProtector::Impl::m_pCurrImpl->m_pProtector->ExceptionInfo.Type = RuntimeProtector::ESE_AccessViolation;
            break;
        case SIGSEGV:
            RuntimeProtector::Impl::m_pCurrImpl->m_pProtector->ExceptionInfo.Type = RuntimeProtector::ESE_AccessViolation;
            break;
        default: assert(false); //should not get here
    }
    RuntimeProtector::Impl::m_pCurrImpl->m_pProtector->ExceptionInfo.Addr = info->si_addr;
    longjmp(RuntimeProtector::Impl::m_pCurrImpl->m_env, sig );
}

bool RuntimeProtector::TryProtectedFunc()
{
   
    // allow cascading by storing prev and current impl
    m_pImpl->m_pPrevImpl         = Impl::m_pCurrImpl;
    Impl::m_pCurrImpl            = m_pImpl;
 
    struct sigaction oldAction[3]; // we need to store old actions, could remove for optimization

 	m_bHashadException = false;
	if( setjmp(m_pImpl->m_env) )
    {
        m_bHashadException = true;
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
        
        ProtectedFunc();
    }
    
    //reset
    sigaction(SIGILL, &oldAction[0], NULL );
    sigaction(SIGBUS, &oldAction[1], NULL );
    sigaction(SIGSEGV, &oldAction[2], NULL );
    
    return !m_bHashadException;
};
