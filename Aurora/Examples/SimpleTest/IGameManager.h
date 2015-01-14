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

#ifndef IGAMEMANAGER_INCLUDED
#define IGAMEMANAGER_INCLUDED

#include "IEntityObject.h"
#include "InterfaceIds.h"
#include "../../Systems/IUpdateable.h" 
#include "../../RuntimeObjectSystem/ObjectInterface.h"
#include "../../RuntimeCompiler/AUArray.h"

enum EGameState
{
	EGS_STARTUP,
	EGS_NEWGAME,
	EGS_PLAYING,
	EGS_IMMUNEWON,
	EGS_INFECTIONWON
};

enum EGameTeam
{
	EGT_IMMUNE = 0,
	EGT_INFECTION,

	EGT_COUNT
};

enum EGameObject
{
	EGO_WBC = 0,  // White Blood Cell
	EGO_RBC,      // Red Blood Cell
	EGO_VIRUS,
	EGO_INFECTED,

	EGO_COUNT
};


struct GlobalParameters;
struct IGameObject;


struct IGameEventListener
{
	// All events have empty implementations so listeners 
	// only have to handle events that they care about

	virtual void OnGameReset() {}
	virtual void OnStateChange( EGameState newState ) {}
	virtual void OnGameObjectCreated( IGameObject* pGameObject ) {}
	virtual void OnGameObjectAboutToDestroy( IGameObject* pGameObject ) {}
};


struct IGameManager : public  TInterface<IID_IGAMEMANAGER,IEntityObject>, public IAUUpdateable
{
	virtual void ResetGame() = 0;
	virtual void SpawnGameObject( EGameObject type ) = 0;
	virtual void SpawnGameObject( EGameObject type, const AUVec3f& spawnPosition ) = 0;
	virtual void DestroyGameObject( ObjectId id ) = 0;
	virtual void DestroyGameObject( const char* name ) = 0;
	virtual GlobalParameters* GetGlobalParameters() = 0;
	virtual EGameState GetGameState() const = 0;

	virtual void GetAll( EGameObject type, IAUDynArray<ObjectId> &objects ) const = 0;

	virtual void AddListener(IGameEventListener* pListener) = 0;
	virtual void RemoveListener(IGameEventListener* pListener) = 0;
};


#endif // IGAMEMANAGER_INCLUDED