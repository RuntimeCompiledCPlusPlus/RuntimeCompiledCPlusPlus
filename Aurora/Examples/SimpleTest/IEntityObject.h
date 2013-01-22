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
// IEntityObject header file.
//
// All objects that will be linked to an entity should be based off this class, which implements just a 
// couple of features from IObject to make sure object linking with entity ID is done correctly, which is important
// for correct serialization, and saves having to reimplement the same boilerplate in every object class
//
////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef IENTITYOBJECT_INCLUDED
#define IENTITYOBJECT_INCLUDED

#include "InterfaceIds.h"
#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"
#include "../../RuntimeObjectSystem/IObject.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/IEntity.h"
#include "../../Systems/IEntitySystem.h"
#include <assert.h>


class IEntityObject : public  TInterface<IID_IENTITYOBJECT,IObject>
{
public:
	IEntityObject() : m_pEntity(NULL) 
	{
	}

	virtual ~IEntityObject()
	{
		if (m_pEntity)
		{
			m_pEntity->SetObject(NULL);
		}
	}

	virtual void SetEntity( IAUEntity* pEntity )
	{
		if( pEntity != m_pEntity )
		{
			if( m_pEntity )
			{
				m_pEntity->SetObject( NULL );
			}
			if( pEntity )
			{
				IEntityObject* pEntityObject = pEntity->GetObject();
				if( pEntityObject )
				{
					pEntityObject->SetEntity( NULL );
				}
				pEntity->SetObject( this );
			}
			m_pEntity = pEntity;
		}
	}

	virtual AUEntityId GetEntityId() const { AU_ASSERT(m_pEntity); return m_pEntity->GetId(); }

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		AU_ASSERT( pSerializer );
		AUEntityId entityId = -1;
		if( pSerializer->IsLoading() )
		{
			pSerializer->SerializeProperty("m_Entity", entityId);
			IAUEntity* pEntity = PerModuleInterface::g_pSystemTable->pEntitySystem->Get(entityId);
			AU_ASSERT(pEntity);
			SetEntity( pEntity );	//must do this through set entity
		}
		else
		{
			entityId = m_pEntity ? m_pEntity->GetId() : 0;
			pSerializer->SerializeProperty("m_Entity", entityId);
		}
	}

protected:
	IAUEntity* m_pEntity;
};


#endif // IENTITYOBJECT_INCLUDED