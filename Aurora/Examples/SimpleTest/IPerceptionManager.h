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

#ifndef IPERCEPTIONMANAGER_INCLUDED
#define IPERCEPTIONMANAGER_INCLUDED

#include "../../RuntimeObjectSystem/IObject.h"
#include "IGameManager.h"
#include "InterfaceIds.h"
#include "../../Common/AUVec3f.inl"
#include "../../RuntimeCompiler/AUArray.h"
#include "../../Systems/IUpdateable.h" 

struct IGameObject;


struct IPerceptionManager : public  TInterface<IID_IPERCEPTIONMANAGER,IObject>, public IGameEventListener
{
	virtual int GetNumberPerceived( const IGameObject* pPerceiver, EGameObject perceivedType ) const = 0;
	virtual int GetNumberPerceived( const IGameObject* pPerceiver ) const = 0;  // Perceive all object types

	// Add as well as Get to easily accumulate different types 
	virtual void GetPerceived( const IGameObject* pPerceiver, EGameObject perceivedType, IAUDynArray<ObjectId>& objects ) const = 0;
	virtual void AddPerceived( const IGameObject* pPerceiver, EGameObject perceivedType, IAUDynArray<ObjectId>& objects ) const = 0;
	virtual void GetPerceived( const IGameObject* pPerceiver, IAUDynArray<ObjectId>& objects ) const = 0;   // Perceive all object types

	virtual AUVec3f GetGlobalAveragePos( EGameObject perceivedType ) const = 0;  
	virtual AUVec3f GetGlobalAveragePos() const = 0;  // Perceive all object types
	virtual AUVec3f GetPerceivedAveragePos( const IGameObject* pPerceiver, EGameObject perceivedType ) const = 0;  
	virtual AUVec3f GetPerceivedAveragePos( const IGameObject* pPerceiver ) const = 0;  // Perceive all object types
};


#endif // IPERCEPTIONMANAGER_INCLUDED