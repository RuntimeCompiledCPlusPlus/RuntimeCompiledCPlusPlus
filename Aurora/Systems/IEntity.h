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

#ifndef IAUENTITY_INCLUDED
#define IAUENTITY_INCLUDED

#include "Definitions.inl"
#include "../Common/AUOrientation3D.inl"
#include "../Common/AUVec3f.inl"

#define AU_ENTITY_NAME_LENGTH 32

class IEntityObject;

struct IAUEntity
{
		virtual AUEntityId GetId() const = 0;

		virtual const char * GetName() const = 0;                      // May return empty string, never NULL
		virtual void SetName(const char * sName) = 0;                  // Up to AU_ENTITY_NAME_LENGTH - 1 bytes plus null terminator, or NULL

		//virtual const IPhysics * GetPhysics() const = 0;
		//virtual IPhysics * GetPhysics() = 0;
		//virtual void SetPhysics(IPhysics *pPhysics) = 0;

		virtual const IEntityObject * GetObject() const = 0;
		virtual IEntityObject * GetObject() = 0;
		virtual void SetObject(IEntityObject *pObject) = 0;

		virtual const IAURenderable * GetRenderable() const = 0;
		virtual IAURenderable * GetRenderable() = 0;
		virtual void SetRenderable(IAURenderable *pRenderable) = 0;

		virtual const IAUUpdateable * GetUpdateable() const = 0;
		virtual IAUUpdateable * GetUpdateable() = 0;
		virtual void SetUpdateable(IAUUpdateable *pUpdateable) = 0;

		virtual const AUVec3f& GetPosition() const = 0;
		virtual AUVec3f& GetPosition() = 0;
		virtual void SetPosition(const AUVec3f& vPos) = 0;
		virtual void SetPosition(float x, float y, float z) = 0;

		virtual const AUVec3f& GetScale() const = 0;
		virtual AUVec3f& GetScale() = 0;
		virtual void SetScale(const AUVec3f& vScale) = 0;
		virtual void SetScale(float fScale) = 0;

		virtual const AUOrientation3D& GetOrientation() const = 0;
		virtual AUOrientation3D& GetOrientation() = 0;
		virtual void SetOrientation(const AUOrientation3D& vOrientation) = 0;
		virtual void SetOrientation(float xForward, float yForward, float zForward, float xUp, float yUp, float zUp) = 0;
    
        virtual ~IAUEntity() {}
};

#endif // IAUENTITY_INCLUDED