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


#ifndef ISIMPLESERIALIZER_INCLUDED
#define ISIMPLESERIALIZER_INCLUDED

// Currently this system is more of a 'property store' rather than a serializer,
// but could be modified in future in that direction if needed, though more likely
// will be scrapped for something more sophisticated


#include "../RuntimeObjectSystem/ObjectInterface.h"


#define SERIALIZE(prop) pSerializer->SerializeProperty(#prop, prop);




struct ISerializedValue {
	virtual ~ISerializedValue()
	{
	}
};

template <typename T>
struct SerializedValue : ISerializedValue
{
	// NOTE: this requires value being serialized to have a correct copy constructor that
	// will copy all values that would otherwise be deleted when object being serialized
	// is deleted	
	SerializedValue(const T& value) : value(value)
	{
	}

	const T value;
};


struct ISimpleSerializer
{
	virtual void Clear() = 0;
	virtual void Clear(ObjectId ownerId) = 0;
	virtual void Clear(ObjectId ownerId, const char* propertyName) = 0;

	virtual bool IsLoading() const = 0;	
	
	// Stores a copy of the value when loading is false
	// Returns true on successful property load, or always when saving a value
	template <typename T> bool SerializeProperty(const char* propertyName, T& value);

	// Array of T version of SerializeProperty
	// Stores a copy of the value when loading is false
	// Returns true on successful property load, or always when saving a value
	template <typename T, size_t N> bool SerializeProperty(const char* propertyName, T (&arrayIn)[N] );
 
    virtual ~ISimpleSerializer( ) {}
private:
	// Implementation requires backing the following functions with keyed storage
    // pValue should be deleted by implementation in destructor.
	virtual void SetISerializedValue(const char* propertyName, const ISerializedValue* pValue) = 0;
	virtual const ISerializedValue* GetISerializedValue(const char* propertyName) const = 0;

};


// NOTE: this is less efficient than having separate functions for setting and getting properties,
// but allows for user code to generally have much simpler serialization methods without needing to 
// handle save and load separately (in most cases)
template <typename T>
inline bool ISimpleSerializer::SerializeProperty(const char* propertyName, T& value)
{
	if (IsLoading())
	{
		const SerializedValue<T>* pSV = static_cast<const SerializedValue<T>*>(GetISerializedValue(propertyName));
		if (!pSV)
		{
			return false;
		}

		value = pSV->value;
	}
	else
	{
		const SerializedValue<T>* pSv = new SerializedValue<T>(value);
		SetISerializedValue(propertyName, pSv);
	}	

	return true;
}

template <typename T, size_t N>
struct SerializedValueArray : ISerializedValue
{
	// NOTE: this requires value being serialized to have a correct copy constructor that
	// will copy all values that would otherwise be deleted when object being serialized
	// is deleted	
	SerializedValueArray(const T (&arrayIn)[N] )
	{
		memcpy( valueArray, arrayIn, sizeof( valueArray) );
	}

	T valueArray[N];
};

// NOTE: this is less efficient than having separate functions for setting and getting properties,
// but allows for user code to generally have much simpler serialization methods without needing to 
// handle save and load separately (in most cases)
template <typename T, size_t N>
inline bool ISimpleSerializer::SerializeProperty(const char* propertyName, T (&arrayIn)[N])
{
	if (IsLoading())
	{
		const SerializedValueArray<T,N>* pSV = static_cast<const SerializedValueArray<T,N>*>(GetISerializedValue(propertyName));
		if (!pSV)
		{
			return false;
		}

		memcpy( arrayIn, pSV->valueArray, sizeof( arrayIn ) );
	}
	else
	{
		const SerializedValueArray<T,N>* pSv = new SerializedValueArray<T,N>(arrayIn);
		SetISerializedValue(propertyName, pSv);
	}	

	return true;
}

#endif //ISIMPLESERIALIZER_INCLUDED
