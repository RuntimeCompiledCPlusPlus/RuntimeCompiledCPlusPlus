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

#include "CompilerLogger.h"
#include "Environment.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/ILogSystem.h"

#include <string>
#include <stdio.h>

#include <stdarg.h>

void CompilerLogger::LogError(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	
	m_pEnv->sys->pLogSystem->LogVa(args, eLV_ERRORS, format);
}

void CompilerLogger::LogWarning(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	m_pEnv->sys->pLogSystem->LogVa(args, eLV_WARNINGS, format);

}

void CompilerLogger::LogInfo(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	m_pEnv->sys->pLogSystem->LogVa(args, eLV_COMMENTS, format);

}
