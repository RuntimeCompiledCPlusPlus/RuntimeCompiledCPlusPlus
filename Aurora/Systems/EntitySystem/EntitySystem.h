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

#include "../IEntitySystem.h"

#include <map>

class EntitySystem : public IEntitySystem
{
public:
	/// IEntitySystem methods
	AUEntityId Create(const char * sName);
	bool Destroy(AUEntityId id);
	IAUEntity * Get(AUEntityId id);
	IAUEntity * Get(const char * sName);
	void GetAll(IAUDynArray<AUEntityId> &entities) const;
	void Reset();

	/// New methods
	EntitySystem(void);
	~EntitySystem(void);

protected:
	class Entity : public IAUEntity
	{
	public:
		AUEntityId m_id;
		IEntityObject* m_pObject;
		IAURenderable* m_pRenderable;
		//IAUTransform * m_pTransform
		IAUUpdateable* m_pUpdateable; 
		AUVec3f m_vPos;
		AUVec3f m_vScale;
		AUOrientation3D m_vOrientation;
		
		char m_sName[AU_ENTITY_NAME_LENGTH];

		/// IAUEntity methods

		AUEntityId GetId() const { return m_id; }

		const char * GetName() const { return m_sName; }
		void SetName(const char * _sName);

		const IEntityObject * GetObject() const { return m_pObject; }
		IEntityObject * GetObject() { return m_pObject; }
		void SetObject(IEntityObject *pObject);

		const IAURenderable * GetRenderable() const { return m_pRenderable; }
		IAURenderable * GetRenderable() { return m_pRenderable; }
		void SetRenderable(IAURenderable *pRender);

		const IAUUpdateable * GetUpdateable() const { return m_pUpdateable; }
		IAUUpdateable * GetUpdateable() { return m_pUpdateable; }
		void SetUpdateable(IAUUpdateable *pUpdateable);

		const AUVec3f& GetPosition() const { return m_vPos; }
		AUVec3f& GetPosition() { return m_vPos; }
		void SetPosition(const AUVec3f& vPos) { m_vPos = vPos; }
		void SetPosition(float x, float y, float z) { m_vPos = AUVec3f(x, y, z); }

		const AUVec3f& GetScale() const { return m_vScale; }
		AUVec3f& GetScale() { return m_vScale; }
		void SetScale(const AUVec3f& vScale) { m_vScale = vScale; }
		void SetScale(float fScale) { m_vScale = AUVec3f(fScale, fScale, fScale); }

		const AUOrientation3D& GetOrientation() const { return m_vOrientation; }
		AUOrientation3D& GetOrientation() { return m_vOrientation; }
		void SetOrientation(const AUOrientation3D& vOrientation) { m_vOrientation = vOrientation; }
		void SetOrientation(float xForward, float yForward, float zForward, float xUp, float yUp, float zUp)
		{
			m_vOrientation = AUOrientation3D( AUVec3f(xForward, yForward, zForward), AUVec3f(xUp, yUp, zUp) );
		}

		/// New methods

		Entity(AUEntityId id)
			: m_id(id)
			, m_pRenderable(0)
			, m_pUpdateable(0)
			, m_vScale(1.0f, 1.0f, 1.0f)
			, m_pObject( NULL )
 			{ m_sName[0] = '\n'; }
	};

	typedef std::map<AUEntityId, Entity*> TCESEntities;
	TCESEntities m_Entities;
	AUEntityId m_nextId;
};