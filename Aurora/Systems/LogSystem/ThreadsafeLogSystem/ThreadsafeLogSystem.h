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

#ifndef THREADSAFELOGSYSTEM_INCLUDED
#define THREADSAFELOGSYSTEM_INCLUDED

#include "../../ILogSystem.h"

class ThreadsafeLogSystem : public ILogSystem
{
public:
	//// ILogSystem interface
	//SErrorDescriptor UnitTest(ILogSystem *) 
	//	{ return SErrorDescriptor(); }                 
	
	//// New to this class

	// There should only be one place that creates the logs, and one place that destroys them
	// All theaded logging sources should be shut down before we do this! 
	ThreadsafeLogSystem(void);
	~ThreadsafeLogSystem(void);

	// Critical section - but should set before any multithreaded logging started
	void SetProtectedLogSystem(ILogSystem *pLogSystem);

	// WaitForThreads() functionality here seems useful
	// There are various threads that could still be logging and we need to wait until they all exit

	// Pimpl used for OS-specific multithreading primitives
	struct TLSPlatformImpl;
	
	//// ILogSystem interface

	// No critical section - rarely changes, should appear atomic
	ELogVerbosity GetVerbosity() const;

	// Critical section
	void SetVerbosity(ELogVerbosity eVerbosity); 

	// No critical section returned pointer should remain valid
	TVerbosityPeeker GetVerbosityPeeker() const;
	
	// Critical section
	void Log(ELogVerbosity eVerbosity, const char * format, ...);

	// Critical section
	void LogVa(va_list args, ELogVerbosity eVerbosity, const char * format);
                                                       
protected:
	void LogInternal(ELogVerbosity eVerbosity, const char * format, va_list args);

	TLSPlatformImpl * m_pImpl;
	ELogVerbosity m_eVerbosity;
	ILogSystem* m_protectedLogger;
};


#endif //THREADSAFELOGSYSTEM_INCLUDED