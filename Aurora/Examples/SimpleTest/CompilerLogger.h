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

#ifndef COMPILERLOGGER_INCLUDED
#define COMPILERLOGGER_INCLUDED

#include "../../RuntimeCompiler/ICompilerLogger.h"
#include <assert.h>

class Environment;


class CompilerLogger : public ICompilerLogger
{
	void LogError(const char * format, ...);
	void LogWarning(const char * format, ...);
	void LogInfo(const char * format, ...);
public:
	CompilerLogger(Environment* pEnv) : m_pEnv(pEnv)
	{
		assert(m_pEnv);
	}
private:
	Environment* m_pEnv;
};

#endif // COMPILERLOGGER_INCLUDED