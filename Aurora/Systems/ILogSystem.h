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

#ifndef ILOGSYSTEM_INCLUDED
#define ILOGSYSTEM_INCLUDED

// I'm not sure if testing the log system makes much sense
// Might be room for a super-verbose debugging log level
// This might genuinely be worth multithreaded
// Differing file/console later. Console buffer later.
// note that eLV_ is probably the thing to grep for, if we insist it must be specified.

// Add a way to set a callback function that can be used to generate prefixes - in this case frame and time
// a flush might be a useful addition, or two modes, one of which will always flush, and some catch on exit() that will ensure it gets output?

// The log system is an exception to orthogonality in that it can be passed into UnitTest methods that exist in all other systems.
// Without it, we could not report where our systems were going wrong in a consistent manner. This is part of the reason why ILogSystem
// does _not_ have a proper UnitTest method and instead relies upon external examination.

//#include "Settings.h"
#include "ISystem.h"
#include <stdarg.h> // Only simple definitions

static const int LOGSYSTEM_MAX_BUFFER = 4096;

enum ELogVerbosity
{
	eLV_NEVER,       // Never output, always ignore
	eLV_ERRORS,      // Only output when broken
	eLV_WARNINGS,    // ... and output when suspicious
	eLV_EVENTS,      // ... and output events considered "highest-level" for this system
	eLV_COMMENTS,     // ... and output everything else that we're doing
};

struct ILogSystem : public ISystem
{
public:
	//// ISystem methods
	//virtual SErrorDescriptor UnitTest(ILogSystem *pLog) = 0; // Unit testing a logger is awkward - we delegate it

	//// New methods
	virtual ~ILogSystem() {};

	typedef const ELogVerbosity * const TVerbosityPeeker;

	virtual ELogVerbosity GetVerbosity() const = 0;                // Get currently set verbosity level 
	virtual void SetVerbosity(ELogVerbosity eVerbosity) = 0;       // Set verbosity level. Defaults to eLV_EVENTS
	virtual TVerbosityPeeker GetVerbosityPeeker() const = 0;       // Get a pointer to the current verbosity level to store and use for fast "peek" with no function call
	                                                               // Remains valid for the lifetime of the log instance
	
	// Log a message with given minimum verbosity level and printf syntax
	virtual void Log(ELogVerbosity eVerbosity, const char * format, ...) = 0;
	// Log a message with given minimum verbosity level taking a va_list - used when wrapping this logging and re-exposing as printf syntax
	// Argument order is chosen to make it harder to accidentally call the variadic version
	virtual void LogVa(va_list args, ELogVerbosity eVerbosity, const char * format) = 0;
};



// Allow avoiding debug code, allow compiling it out, etc.
// Note there are apparently some variations for varaiadic macros - GCC etc.

#ifndef PN_LOG_LEVEL
	#define PN_LOG_LEVEL eLV_COMMENTS
#endif

#define PN_LOG(env,level,format,...) \
	if ( (level) <= PN_LOG_LEVEL ) {(env)->sys->pLogSystem->Log((level),(format),__VA_ARGS__);}

#define PN_LOG_EXT(env,level,code_block,format,...) \
	if ( (level) <= PN_LOG_LEVEL ) \
	{ \
		code_block ; \
		(env)->sys->pLogSystem->Log((level),(format),__VA_ARGS__); \
	} 


#endif //ILOGSYSTEM_INCLUDED