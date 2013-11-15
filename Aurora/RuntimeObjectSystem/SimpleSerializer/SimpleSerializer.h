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

#ifndef SIMPLESERIALIZER_INCLUDED
#define SIMPLESERIALIZER_INCLUDED

#include "../ISimpleSerializer.h"

#include <map>
#include <string>

struct IObject;

class SimpleSerializer : public ISimpleSerializer
{
public:

	SimpleSerializer();
	virtual ~SimpleSerializer();

	// ISimpleSerializer

	void Clear();
	void Clear(ObjectId ownerId);
	void Clear(ObjectId ownerId, const char* propertyName);

	void Serialize( IObject* Object );

	void SetISerializedValue(const char* propertyName, const ISerializedValue* pValue);
	const ISerializedValue *GetISerializedValue(const char* propertyName) const;
	virtual bool IsLoading() const
	{
		return m_bLoading;
	}
	void SetIsLoading( bool loading )
	{
		m_bLoading = loading;
		m_pCurrentObject = 0;
	}


	// ~ISimpleSerializer

private:

	typedef std::map<std::string, const ISerializedValue*> TValueGroup;
	typedef std::map<ObjectId, TValueGroup> TSerializationMap;

	TSerializationMap			m_map;
	int							m_numProperties;
	bool						m_bLoading;
	IObject*					m_pCurrentObject;
	TSerializationMap::iterator m_CurrentSerialization;
};


#endif // SIMPLESERIALIZER_INCLUDED