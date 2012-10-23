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

#ifndef IGAMEOBJECT_INCLUDED
#define IGAMEOBJECT_INCLUDED

#include "IEntityObject.h"
#include "IGameManager.h"
#include "../../Common/AUColor.inl"
#include "../../Systems/IUpdateable.h" 
#include "InterfaceIds.h"

struct IBehavior;
struct IBehaviorTree;


struct IGameObject : public  TInterface<IID_IGAMEOBJECT,IEntityObject>, public IAUUpdateable
{
	virtual ~IGameObject() {}

	// This version of Init must be called to properly initialize the object
	virtual void Init( EGameObject type, const AUVec3f& spawnPosition ) = 0;

	virtual EGameObject GetGameObjectType() const = 0;
	virtual EGameTeam GetGameTeam() const = 0;
	virtual const AUColor& GetColor() const = 0;
	virtual IBehaviorTree* GetBehaviorTree() = 0;
	virtual IBehavior* GetBehavior() = 0;
	virtual IAUEntity* GetEntity() = 0; // Helper since we need to get this a lot
	virtual const IAUEntity* GetEntity() const = 0; // Helper since we need to get this a lot

	virtual void SetColor( const AUColor& color ) = 0;
	virtual void SetColor( float r, float g, float b, float a = 0.0f ) = 0;
	virtual void SetModel( const char* file ) = 0;
	virtual void SetBehavior( ConstructorId constructor ) = 0;

	virtual void OnSelect() = 0;
	virtual void OnDeselect() = 0;
	virtual void OnPositionRequest( const AUVec3f& position ) = 0;

	virtual void OnCollision( IGameObject* pCollider ) = 0;
	virtual float GetCollisionRadius() const = 0;
	virtual float GetMaxSpeed() const = 0;
	virtual float GetHealth() const = 0;
	virtual float GetThreatRating() const = 0;

	virtual void GetDebugInfo( char* outputBuffer, size_t bufferLen ) = 0;
};


#endif // IGAMEOBJECT_INCLUDED