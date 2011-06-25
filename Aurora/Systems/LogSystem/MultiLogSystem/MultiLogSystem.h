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

#ifndef MULTILOGSYSTEM_INCLUDED
#define MULTILOGSYSTEM_INCLUDED

#include "../../ILogSystem.h"
#include <vector>

class MultiLogSystem : public ILogSystem
{
public:
	//// ILogSystem interface
	//SErrorDescriptor UnitTest(ILogSystem *) 
	//	{ return SErrorDescriptor(); }                 
	
	//// New class methods
	
	MultiLogSystem(void);

	// Destroy this instance and all currently added log instances
	~MultiLogSystem(void);

	// Add a log system instance to the list, if not already present
	// Returns true if instance was added
	bool AddLogSystem(ILogSystem *pLogSystem);
	// Remove a log system instance from the list
	// Returns true if found. Does not destroy the instance
	bool RemoveLogSystem(ILogSystem *pLogSystem);
	
	//// ILogSystem interface

	ELogVerbosity GetVerbosity() const;
	void SetVerbosity(ELogVerbosity eVerbosity); 
	TVerbosityPeeker GetVerbosityPeeker() const;

	void Log(ELogVerbosity eVerbosity, const char * format, ...);
	void LogVa(va_list args, ELogVerbosity eVerbosity, const char * format);
                                                       
protected:
	void LogInternal(ELogVerbosity eVerbosity, const char * format, va_list args);

	ELogVerbosity m_eVerbosity;
	std::vector<ILogSystem*> m_logSystems;
};


#endif //MULTILOGSYSTEM_INCLUDED