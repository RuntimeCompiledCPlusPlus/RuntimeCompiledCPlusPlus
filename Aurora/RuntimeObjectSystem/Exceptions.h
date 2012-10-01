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

#ifdef WIN32
#define AUTRY_RETURN( X )                 \
__try                              \
{                                   \
    X;                              \
}                                   \
__except( RuntimeExceptionFilter() ) \
{                                   \
    return false;                   \
}                                   \
return true;
#else
#define AUTRY_RETURN( X )  X; return true;
#endif


// We might handle div by 0, stack overflow, etc
enum ESimpleExceptions
{
	ESE_Unknown,
	ESE_AccessViolationRead,
	ESE_AccessViolationWrite,
};

struct AuroraExceptionInfo
{
	ESimpleExceptions exceptionType;
	unsigned int xAddress;
};


// For handling failures in runtime exceptions
// Might move into a DebugSystem for more better API with more control
int RuntimeExceptionFilter(void);

// For catching simple errors that we understand and falling back to
// RuntimeExceptionFilter behaviour for others. Intended for situations
// where we expect simple errors like null pointers in simple code
// and so try to provide simple feedback rather than going to the debugger
int SimpleExceptionFilter( void * nativeExceptionInfo, AuroraExceptionInfo *auroraExceptionInfo );