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



// TODO: Implement OSX Version...

enum ExceptionState
{
	ES_PASS,
	ES_CATCH,
	ES_CONTINUE,
};

static ExceptionState s_exceptionState = ES_PASS;

int RuntimeExceptionFilter()
{
	int result=0;
	switch (s_exceptionState)
	{
	case ES_PASS:
		// Let pass to debugger once, then catch it
//		result = EXCEPTION_CONTINUE_SEARCH;
		s_exceptionState = ES_CATCH;
		break;
	case ES_CATCH:
		// Catch it now. Reset to catch in debugger again next time.
//		result = EXCEPTION_EXECUTE_HANDLER;
		s_exceptionState = ES_PASS;
		break;
	case ES_CONTINUE:
		// Expand this to cope with edit-and-continue case
		break;
	}

	return result;
}


int SimpleExceptionFilter( void * nativeExceptionInfo, AuroraExceptionInfo *auroraExceptionInfo )
{	
	// Otherwise fall back
	return RuntimeExceptionFilter();
}