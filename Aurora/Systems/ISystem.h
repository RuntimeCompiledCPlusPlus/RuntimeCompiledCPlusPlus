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

#ifndef ISYSTEM_INCLUDED
#define ISYSTEM_INCLUDED

// Skeleton of common interface across all system interfaces
// Should eventually encompass memory statistics etc

// Some kind of virtual constructor... is perhaps the tricky bit. Best left out actually, to allow for multiple implementations
// Update?

// Constructors of systems can't fail, but init can fail, returning an error code
//	virtual SErrorDescriptor Init() {};  Should this be compulsory? Or does it always need different parameters?

//#include "Definitions.h"

struct ISystem
{
	//virtual SErrorDescriptor UnitTest(ILogSystem *pLog = NULL) = 0;         // Giving a logger is optional
	//virtual SErrorDescriptor Shutdown() { return PN_NO_ERROR; };            // Shutdown to clean state that could be reinitialised
	virtual ~ISystem() {};                                                  // Destroy the system
};

#endif //ISYSTEM_INCLUDED