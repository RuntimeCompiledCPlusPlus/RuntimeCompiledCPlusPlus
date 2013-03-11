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

#ifndef _WIN32
    #include <setjmp.h> // used by posix type systems to chain handling
#endif

// class RuntimeProtector
// overload void ProtectedFunc() to use, put function context (io) in new members
// do not create threads within protected function
// for threaded usuage use a protector per thread
// amortize virtual function call and exception handling by processing many things in one call
// note this isn't a functor as we prefer the explicit function name, and not using lambda's due to Cx11
// not being supported sufficiently as yet
struct RuntimeProtector
{
	// consctructor, hint allow debug may be ignored when true if on an OS which has not had this implemented
    RuntimeProtector()
	    : m_bHashadException( false )
	    , m_bHintAllowDebug( true )
        , m_ModulesLoadedCount( 0 )
#ifndef _WIN32
    	, m_pPrevious( 0 )
#endif
    {
    }

    virtual ~RuntimeProtector() {}

     // don't call this directly, derive a class and implement it for your protected func
    virtual void ProtectedFunc() = 0;
    
    bool HasHadException() const
    {
        return m_bHashadException;
    }
    void ClearExceptions()
    {
        m_bHashadException = false;
    }
   
    //exception information (exposed rather than get/set for simplicity)
    enum ExceptionType
    {
        ESE_Unknown,
        ESE_AccessViolation,
        ESE_AccessViolationRead,    //may just get ESE_AccessViolation
        ESE_AccessViolationWrite,   //may just get ESE_AccessViolation
        ESE_InvalidInstruction
        
    };
    struct ExceptionInfo_t
    {
        ExceptionType       Type;
        void*               Addr; //address of data for access voilation, or instruction for invalid instruction
    };
    ExceptionInfo_t         ExceptionInfo;
    
    bool                    m_bHashadException;
    bool                    m_bHintAllowDebug;    // some RuntimeProtectors may not want to allow debug

    // internal 
    unsigned int            m_ModulesLoadedCount; // used internally to reset exceptions when a new module is loaded
#ifndef _WIN32
    jmp_buf                 m_env;
    RuntimeProtector*       m_pPrevious;          // used by posix type systems to chain handling
#endif
};

