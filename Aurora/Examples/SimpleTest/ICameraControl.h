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

#ifndef ICAMERACONTROL_INCLUDED
#define ICAMERACONTROL_INCLUDED

#include "../../Common/AUVec3f.inl"
#include "IEntityObject.h"
#include "InterfaceIds.h"
#include "../../Systems/IUpdateable.h" 

struct ICameraControl : public TInterface<IID_ICAMERACONTROL,IEntityObject>, public IAUUpdateable
{
	virtual AUVec3f GetCurrentPos() const = 0;
	virtual AUVec3f GetTargetPos() const = 0;
	virtual void SetTargetPos(const AUVec3f& pos) = 0;
	virtual float GetMoveSpeed() const = 0;
	virtual void SetMoveSpeed(float fSpeed) = 0;

	virtual AUVec3f Project( const AUVec3f& worldPos ) const = 0;
	virtual AUVec3f Unproject( const AUVec3f& screenPos ) const = 0;
};


#endif // ICAMERACONTROL_INCLUDED