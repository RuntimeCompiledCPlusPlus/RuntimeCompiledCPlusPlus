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

// Implementation 

// Apply salts and buffer reuse later

#include "EntitySystem.h"
#include "../../Renderer/IAURenderable.h"
#include "../IUpdateable.h"

#include <assert.h>

//#include physics etc proxies for delete

void EntitySystem::Entity::SetName(const char * sName) 
{
	if (sName && sName[0])
	{
#ifdef _WIN32
		strncpy_s(m_sName, sName, AU_ENTITY_NAME_LENGTH);
#else
        strncpy(m_sName,sName,AU_ENTITY_NAME_LENGTH);
#endif
		m_sName[AU_ENTITY_NAME_LENGTH-1] = '\0';
		// Could re length warn here
	}
	else
	{
		m_sName[0] = '\0';
		// And here
	}
}

void EntitySystem::Entity::SetObject(IEntityObject *pObject) { m_pObject = pObject; } // Not safe to delete, created in DLLs...

void EntitySystem::Entity::SetRenderable(IAURenderable *pRenderable) { m_pRenderable = pRenderable; } // Not safe to delete, created in DLLs...

void EntitySystem::Entity::SetUpdateable(IAUUpdateable *pUpdateable) { m_pUpdateable = pUpdateable; } // Not safe to delete, created in DLLs...

AUEntityId EntitySystem::Create(const char * sName)
{
	Entity *pEntity = new Entity(m_nextId++);
	pEntity->SetName(sName);
	m_Entities[pEntity->GetId()] = pEntity;
	return pEntity->GetId();
}

bool EntitySystem::Destroy(AUEntityId id)
{
	TCESEntities::iterator it = m_Entities.find(id);
	if (it != m_Entities.end())
	{
		IAUEntity *pEntity = it->second;
		delete pEntity;
		m_Entities.erase(it);
		return true;
	}

	return false;
}

IAUEntity * EntitySystem::Get(AUEntityId id)
{
	TCESEntities::iterator it = m_Entities.find(id);
	if (it != m_Entities.end())
	{
		return it->second;
	}
	
	return NULL;
}

IAUEntity * EntitySystem::Get(const char * sName)
{
	if (sName != NULL && *sName != '\0')
	{
		TCESEntities::iterator it = m_Entities.begin();
		TCESEntities::iterator itEnd = m_Entities.end();
		for (; it != itEnd; ++it)
		{
			if (!strcmp(it->second->GetName(), sName))
			{
				return it->second;
			}
		}
	}
	
	return NULL;
}

void EntitySystem::GetAll(IAUDynArray<AUEntityId> &entities) const
{
	entities.Resize(m_Entities.size());
	TCESEntities::const_iterator it = m_Entities.begin();
	TCESEntities::const_iterator itEnd = m_Entities.end();
	for(int i = 0; it != itEnd; ++it, ++i)
	{
		entities[i] = it->second->m_id;
	}
}

void EntitySystem::Reset()
{
	AUDynArray<AUEntityId> entities;
	GetAll(entities);
	for (size_t i = 0; i < entities.Size(); ++i)
	{
		Destroy(entities[i]);
	}

	assert(m_Entities.size() == 0);
}


EntitySystem::EntitySystem(void) : m_nextId(1)
{
}

EntitySystem::~EntitySystem(void)
{
}


