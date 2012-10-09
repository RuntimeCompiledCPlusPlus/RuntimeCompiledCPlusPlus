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

// Notes:
//   - Might actually make sense to perform the printf once for performance reasons.
//     This would probably end up in having a char* form. But wait until its needed.


#include "MultiLogSystem.h"

#include <stdarg.h>
#include <assert.h>

#pragma warning( disable : 4996 4800 )


MultiLogSystem::MultiLogSystem(void)
{
	m_eVerbosity = eLV_COMMENTS;   // By default, defer any filtering
}

MultiLogSystem::~MultiLogSystem(void)
{
	for (unsigned int i=0; i<m_logSystems.size(); ++i)
		delete m_logSystems[i];
}

bool MultiLogSystem::AddLogSystem(ILogSystem *pLogSystem)
{
	for (unsigned int i=0; i<m_logSystems.size(); ++i)
		if (m_logSystems[i] == pLogSystem)
			return false;
	m_logSystems.push_back(pLogSystem);
	return true;
}

bool MultiLogSystem::RemoveLogSystem(ILogSystem *pLogSystem)
{
	for (unsigned int i=0; i<m_logSystems.size(); ++i)
		if (m_logSystems[i] == pLogSystem)
		{
			m_logSystems[i] = m_logSystems[m_logSystems.size()-1];
			m_logSystems.pop_back();
			return true;
		}
	return false;
}

ELogVerbosity MultiLogSystem::GetVerbosity() const
{
	return m_eVerbosity;
}

void MultiLogSystem::SetVerbosity(ELogVerbosity eVerbosity)
{
	m_eVerbosity = eVerbosity;
}

MultiLogSystem::TVerbosityPeeker MultiLogSystem::GetVerbosityPeeker() const
{
	return (&m_eVerbosity);
}

void MultiLogSystem::Log(ELogVerbosity eVerbosity, const char * format, ...)
{
	va_list args;
	va_start(args, format);
	LogInternal(eVerbosity, format, args);
}

void MultiLogSystem::LogVa(va_list args, ELogVerbosity eVerbosity, const char * format)
{
	LogInternal(eVerbosity, format, args);
}

void MultiLogSystem::LogInternal(ELogVerbosity eVerbosity, const char * format, va_list args)
{
	if (eVerbosity > m_eVerbosity || eVerbosity == eLV_NEVER) return;
	// Possible bug here in all loggers - should also check against "compile out" value

	for (unsigned int i=0; i<m_logSystems.size(); ++i)
    {
#ifdef _WIN32
		m_logSystems[i]->LogVa(args, eVerbosity, format);
#else
        // May need to use a copy
        va_list copyArgs;
        va_copy(copyArgs, args);
 		m_logSystems[i]->LogVa(copyArgs, eVerbosity, format);
#endif
    }
}
