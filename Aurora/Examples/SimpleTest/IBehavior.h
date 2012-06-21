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

#ifndef IBEHAVIOR_INCLUDED
#define IBEHAVIOR_INCLUDED

#include "../../RuntimeObjectSystem/IObject.h"
#include "InterfaceIds.h"
#include "../../Common/AUVec3f.inl"

struct IGameObject;

struct IBehavior : public TInterface<IID_IBEHAVIOR,IObject>
{
	virtual ~IBehavior() {}

	virtual void Update( float timeDelta ) = 0;

	virtual void StartBehavior() = 0; // for initializing behavior properties
	virtual void EndBehavior() = 0; // for behavior cleanup

	virtual void UpdatePerception( float timeDelta ) = 0;
	virtual void UpdateBlackboards( float timeDelta ) = 0;
	virtual void UpdateBehavior( float timeDelta ) = 0;
	virtual AUVec3f CalculateDesiredPosition( float timeDelta ) = 0;
	
	virtual void OnCollision( IGameObject* pCollider ) = 0;
	virtual float ReceiveDamage( float fAmount ) = 0; // Returns new health value

	virtual void SetGameObject( IGameObject* pOwner ) = 0;
};


#endif // IBEHAVIOR_INCLUDED