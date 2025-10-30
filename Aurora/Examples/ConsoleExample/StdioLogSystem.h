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

#ifndef STDIOLOGSYSTEM_INCLUDED
#define STDIOLOGSYSTEM_INCLUDED

#include "../../RuntimeCompiler/ICompilerLogger.h"

#include <string>
#include <stdio.h>

// StdioLogSystem for compiler

const size_t LOGSYSTEM_MAX_BUFFER = 4096;

class StdioLogSystem : public ICompilerLogger
{
public:	
	virtual void LogError(const char * format, ...);
	virtual void LogWarning(const char * format, ...);
    virtual void LogInfo(const char * format, ...);

protected:
	void LogInternal(const char * format, va_list args);
	char m_buff[LOGSYSTEM_MAX_BUFFER];
};


#endif //STDIOLOGSYSTEM_INCLUDED