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

////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// InterfaceId header file.
//
// Specifys interface ids for getting hold of interfaces
//
////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef INTERFACEIDS_INCLUDED
#define INTERFACEIDS_INCLUDED

#include "../../RuntimeObjectSystem/IObject.h"

enum InterfaceIDEnumSimpleTest
{
	IID_IENTITYOBJECT = IID_ENDInterfaceID,
	IID_ICONSOLECONTEXT,
	IID_ICAMERACONTROL,
	IID_ILIGHTINGCONTROL,
	IID_IGAMEMANAGER,
	IID_IGAMEOBJECT,
	IID_IBEHAVIORTREEMANAGER,
	IID_IBEHAVIORTREE,
	IID_IBEHAVIOR,
	IID_IBLACKBOARDMANAGER,
	IID_IBLACKBOARD,
	IID_IPERCEPTIONMANAGER,
	IID_IINPUTMANAGER,
	IID_ISPLASHSCREEN,
	IID_IPHYSICSMANAGER,
	IID_IUPDATEABLE,

	IID_ENDInterfaceIDEnumSimpleTest
};


#endif //INTERFACEIDS_INCLUDED