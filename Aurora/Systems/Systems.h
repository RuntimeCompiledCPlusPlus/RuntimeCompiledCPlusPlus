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

#ifndef SYSTEMS_INCLUDED
#define SYSTEMS_INCLUDED

#include "SystemTable.h"


// Actually creating a system table and initialising the systems can have many
// configurations, so we leave this to client code external to this static lib
// However some of the systems do need to refer to each other - the primary
// example being for logging. Hence, the client code should set this pointer
// as soon as it has created a SystemsTable instance and before initialising
// the subsystems.

// If you want to pass around a SystemsTable without having this variable
// directly accessible, prefer to include just SystemsTable.h

extern SystemTable * gSys;

#endif // SYSTEMS_INCLUDED