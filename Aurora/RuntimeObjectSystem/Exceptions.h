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

// class RuntimeProtector
// overload void ProtectedFunc() to use, put function context (io) in new members
// do not create threads within protected function
// for threaded usuage use a protector per thread
// amortize virtual function call and exception handling by processing many things in one call
// note this isn't a functor as we prefer the explicit function name, and not using lambda's due to Cx11
// not being supported sufficiently as yet
class RuntimeProtector
{
public:
	// consctructor, hint allow debug may be ignored when true if on an OS which has not had this implemented
    RuntimeProtector();
    virtual ~RuntimeProtector();

    // TryProtectedFunc() calls ProtectedFunc() and if it gets an exception sets m_bHashadException
    // and returns !m_bHashadException
    bool TryProtectedFunc();
    
    bool HasHadException() const
    {
        return m_bHashadException;
    }
    void ClearExceptions()
    {
        m_bHashadException = false;
    }

	// To allow the debugger to fully catch an exception without it being handled by the RuntimeProtector
	// call SetProtection( false ). This is mostly of use on Mac OS X where the debugger does not capture all
	// exception information if protection is enabled.
	void SetProtection( bool bProtected_ )
	{
		m_bProtectionEnabled = bProtected_;
		if( !m_bProtectionEnabled )
		{
			m_bHashadException = false;
		}
	}
	bool IsProtectionEnabled() const
	{
		return m_bProtectionEnabled;
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
    
    struct Impl;
    Impl*                   m_pImpl;
    bool                    m_bHintAllowDebug;

protected:
    // don't call this directly, derive a class and implement it for your protected func
    virtual void ProtectedFunc() = 0;
private:
    bool                    m_bHashadException;
    bool                    m_bProtectionEnabled;
};

#define AUTRY_RETURN( X )  X; return true;

// now include inline implementations, which are inline so we can use this in a runtime compiled module.
#ifdef _WIN32
    #include "Exceptions_PlatformWindows.inl"
#else
#endif
