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

#include "RocketLogSystem.h"

#include <vector>
#include <Rocket/Core/Core.h>
#include <stdarg.h>
#include <assert.h>



#pragma warning( disable : 4996 4800 )


struct RocketLogSystem::RLSPlatformImpl
{
	std::vector<Rocket::Core::Log::Type> messageTypes;
};

RocketLogSystem::RocketLogSystem(void)
{
	m_pImpl = new RLSPlatformImpl();

	m_buffIndex = 0;  // Start at 0 messages, start of buffer
	m_buff[0] = '\0';

	m_eVerbosity = eLV_COMMENTS;   // By default, defer any filtering
}

RocketLogSystem::~RocketLogSystem(void)
{
	delete m_pImpl;
}

void RocketLogSystem::Push(void)
{

	int count = (int)m_pImpl->messageTypes.size();

	// Make sure there's a limit to the amount of rubbish we can output
	m_buff[BUFF_SIZE-1] = '\0';

	Rocket::Core::SystemInterface *pRocketSystemInterface = Rocket::Core::GetSystemInterface();
	if (!pRocketSystemInterface)
		goto Cleanup;
	
    {
        int msgStart=0;
        int msgEnd=0;
        for (int i=0; i<count; ++i)
        {
            // Find the first logging substring
            while (m_buff[msgEnd] && msgEnd<BUFF_SIZE)
            {
                ++msgEnd;
            }

            if (m_buff[msgEnd])
            {
                // Corrupted buffer
                goto Cleanup;
            }

            pRocketSystemInterface->LogMessage(m_pImpl->messageTypes[i], &m_buff[msgStart]);
            msgEnd++;
            msgStart = msgEnd;
        }
    }

Cleanup:
	m_buffIndex = 0;
	m_buff[0] = '\0';
	m_pImpl->messageTypes.clear();
}


ELogVerbosity RocketLogSystem::GetVerbosity() const
{
	return m_eVerbosity;
}

void RocketLogSystem::SetVerbosity(ELogVerbosity eVerbosity)
{
	m_eVerbosity = eVerbosity;
}

RocketLogSystem::TVerbosityPeeker RocketLogSystem::GetVerbosityPeeker() const
{
	return (&m_eVerbosity);
}

void RocketLogSystem::Log(ELogVerbosity eVerbosity, const char * format, ...)
{
	va_list args;
	va_start(args, format);
	LogInternal(eVerbosity, format, args);
}

void RocketLogSystem::LogVa(va_list args, ELogVerbosity eVerbosity, const char * format)
{
	LogInternal(eVerbosity, format, args);
}

void RocketLogSystem::LogInternal(ELogVerbosity eVerbosity, const char * format, va_list args)
{
	if (eVerbosity > m_eVerbosity || eVerbosity == eLV_NEVER) return;
	// Possible bug here in all loggers - should also check against "compile out" value

	// If there may not be space - throw away this message
	if (BUFF_SIZE - m_buffIndex < LOGSYSTEM_MAX_BUFFER)
    {
        return;
    }

	Rocket::Core::Log::Type rocketVerbosity;
	switch (eVerbosity)
	{
	case eLV_ERRORS:    rocketVerbosity = Rocket::Core::Log::LT_ERROR; break;
	case eLV_WARNINGS:  rocketVerbosity = Rocket::Core::Log::LT_WARNING; break;
	case eLV_EVENTS:    rocketVerbosity = Rocket::Core::Log::LT_INFO; break;
	case eLV_COMMENTS:  rocketVerbosity = Rocket::Core::Log::LT_DEBUG; break;
	default:
		assert(false);
	}

	int result = vsnprintf(&m_buff[m_buffIndex], LOGSYSTEM_MAX_BUFFER-1, format, args);
	// Make sure there's a limit to the amount of rubbish we can output
	m_buff[m_buffIndex+LOGSYSTEM_MAX_BUFFER-1] = 0;

	m_buffIndex += result + 1;
	m_pImpl->messageTypes.push_back(rocketVerbosity);
}