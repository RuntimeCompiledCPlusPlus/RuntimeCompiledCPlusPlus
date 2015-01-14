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

#ifndef IENTITYSYSTEM_INCLUDED
#define IENTITYSYSTEM_INCLUDED

// What is an entity?
// I define it to be the glue that holds all the useful components that might be present together
// Position seems somehow special. Lots of things care about positions and positions without physics can make sense.
// Entities include static geometry, Pneumata, some interface elements? 


#include "../Common/AUVec3f.inl"
#include "../RuntimeCompiler/AUArray.h"
#include "ISystem.h"
#include "IEntity.h"

#include <vector>


struct IEntitySystem : public ISystem
{
	/// ISystem interface
	//SErrorDescriptor UnitTest(ILogSystem *pLog);         // Giving a logger is optional

	/// New methods
	virtual ~IEntitySystem() {};

	virtual void Reset() = 0;

	// Consider renaming with Entity in there?
	virtual AUEntityId Create(const char * sName = NULL) = 0;  // Create a new, empty entity of this type
	virtual bool Destroy(AUEntityId nId) = 0;              // Cleanup and destroy an existing entity. Returns false iff no such id.
	virtual IAUEntity * Get(AUEntityId id) = 0;
	virtual IAUEntity * Get(const char * sName) = 0;  // Get first entity that matches this name (not sure if we want to enforce name uniqueness?)
	virtual void GetAll(IAUDynArray<AUEntityId> &entities) const = 0;
};

#endif // IENTITYSYSTEM_INCLUDED