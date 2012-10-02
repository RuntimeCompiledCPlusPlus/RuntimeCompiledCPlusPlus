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

// Compiler Logger - implement this interface to recieve logging from the RuntimeCompiler module
// In Aurora, this interface exists to avoid dependency on the Systems project

#ifndef ICOMPILERLOGGER_INCLUDED
#define ICOMPILERLOGGER_INCLUDED

struct ICompilerLogger
{
	virtual void LogError(const char * format, ...) = 0;
	virtual void LogWarning(const char * format, ...) = 0;
    virtual void LogInfo(const char * format, ...) = 0;
    virtual ~ICompilerLogger() {}
};

#endif // ICOMPILERLOGGER_INCLUDED