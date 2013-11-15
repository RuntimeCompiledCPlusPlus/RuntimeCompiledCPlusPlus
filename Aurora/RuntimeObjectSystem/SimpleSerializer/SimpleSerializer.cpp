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

#include "SimpleSerializer.h"
#include <assert.h>
#include "../../RuntimeObjectSystem/IObject.h"

SimpleSerializer::SimpleSerializer()
	: m_numProperties(0)
	, m_bLoading( false )
	, m_pCurrentObject( 0 )
{
}

SimpleSerializer::~SimpleSerializer() 
{
	Clear();
}

void SimpleSerializer::Clear()
{
	TSerializationMap::iterator it = m_map.begin();
	while (it != m_map.end())
	{
		TValueGroup::iterator it2 = it->second.begin();
		while (it2 != it->second.end())
		{
			delete it2->second;
			++it2;
		}

		++it;
	}
}

void SimpleSerializer::Clear(ObjectId ownerId)
{
	TSerializationMap::iterator found = m_map.find(ownerId);
	if (found != m_map.end())
	{
		TValueGroup::iterator it = found->second.begin();
		while (it != found->second.end())
		{
			delete it->second;
			++it;
		}
	}
}

void SimpleSerializer::Clear(ObjectId ownerId, const char* propertyName)
{
	TSerializationMap::iterator found = m_map.find(ownerId);
	if (found != m_map.end())
	{
		TValueGroup::iterator propertyFound = found->second.find(propertyName);
		if (propertyFound != found->second.end())
		{
			delete propertyFound->second;
			found->second.erase(propertyFound);
		}
	}
}

void SimpleSerializer::Serialize( IObject* pObject )
{
	assert( pObject );
	assert( 0 == m_pCurrentObject );	//should not serialize an object from within another

	m_pCurrentObject = pObject;
	ObjectId ownerId;
	m_pCurrentObject->GetObjectId(ownerId);

//	gSys->pLogSystem->Log( eLV_COMMENTS, "SimpleSerialiser: Serialising %s object: objectIdtype \"%s\", perTypeID:%d, address:0x%p\n"
//			, m_bLoading ? "in" : "out", pObject->GetTypeName(), ownerId.m_PerTypeId, pObject );

	m_CurrentSerialization = m_map.find(ownerId);

	m_pCurrentObject->Serialize( this );

	//reset m_pCurrentObject
	m_pCurrentObject = 0;
}

void SimpleSerializer::SetISerializedValue(const char* propertyName, const ISerializedValue* pValue)
{
	assert( m_pCurrentObject );
	assert(pValue);
	if( m_CurrentSerialization == m_map.end() )
	{
		ObjectId ownerId;
		m_pCurrentObject->GetObjectId(ownerId);
		m_map[ownerId][propertyName] = pValue;
		m_CurrentSerialization = m_map.find(ownerId);
	}
	m_CurrentSerialization->second[propertyName] = pValue;
}

const ISerializedValue* SimpleSerializer::GetISerializedValue(const char* propertyName) const
{
	assert( m_pCurrentObject );
	assert( propertyName );
	assert( m_bLoading );
	const ISerializedValue* pRet = NULL;

	//TSerializationMap::const_iterator found = m_map.find(ownerId);
	if (m_CurrentSerialization != m_map.end())
	{
		TValueGroup::const_iterator propertyFound = m_CurrentSerialization->second.find(propertyName);
		if (propertyFound != m_CurrentSerialization->second.end())
		{
			pRet = propertyFound->second;
		}
	}

	return pRet;
}
