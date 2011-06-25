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

#ifndef ROCKETLOGSYSTEM_INCLUDED
#define ROCKETLOGSYSTEM_INCLUDED

#include "../../ILogSystem.h"
//#include <vector>
//#include "../libRocket/Include/Rocket/Core.h"

class RocketLogSystem : public ILogSystem
{
public:
	//// ILogSystem interface
	//SErrorDescriptor UnitTest(ILogSystem *) 
	//	{ return SErrorDescriptor(); }                 
	
	//// New to this implementation
	
	RocketLogSystem(void);
	~RocketLogSystem(void);

	// RocketLib redraws the logger as part of redrawing the GUI, so only needs to fetch
	// state when this occurs, not on every logging call. Hence a polling interface would
	// be ideal.
	// For now, a push interface gets the same results. Call Push() before RocketLib update.
	void Push();

	struct RLSPlatformImpl;

	//// ILogSystem interface

	ELogVerbosity GetVerbosity() const;
	void SetVerbosity(ELogVerbosity eVerbosity); 
	TVerbosityPeeker GetVerbosityPeeker() const;

	void Log(ELogVerbosity eVerbosity, const char * format, ...);
	void LogVa(va_list args, ELogVerbosity eVerbosity, const char * format);
                                                       
protected:
	void LogInternal(ELogVerbosity eVerbosity, const char * format, va_list args);

	RLSPlatformImpl * m_pImpl;

	ELogVerbosity m_eVerbosity;

	// Give a generous allocation of full-length log messages between updates
	// or a great many smaller messages
	const static int BUFF_SIZE = LOGSYSTEM_MAX_BUFFER * 20;
	char m_buff[BUFF_SIZE];
	int m_buffIndex;
};


#endif //ROCKETLOGSYSTEM_INCLUDED