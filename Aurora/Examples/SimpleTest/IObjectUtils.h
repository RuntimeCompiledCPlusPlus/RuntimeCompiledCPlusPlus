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
// IObjectUtils header file.
//
// Utility functions and classes for use with IObject
//
////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef IOBJECTUTILS_INCLUDED
#define IOBJECTUTILS_INCLUDED

#include "../../RuntimeObjectSystem/IObject.h"
#include "InterfaceIds.h"
#include "IEntityObject.h"
#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../Systems/SystemTable.h"
#include "../../RuntimeObjectSystem/IObjectFactorySystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/ILogSystem.h"


#define SERIALIZEIOBJPTR(objPtr) SerializeIObjectPtr(pSerializer, #objPtr, &objPtr);

template<typename T> void SerializeIObjectPtr( ISimpleSerializer* pSerializer, const char* propertyName, T** ppObj, InterfaceID iid)
{
	if( pSerializer->IsLoading() )
	{
		ObjectId id;
		pSerializer->SerializeProperty( propertyName, id);
		IObject* pObj = PerModuleInterface::g_pSystemTable->pObjectFactorySystem->GetObject( id );
		if( pObj ) 
		{
			pObj->GetInterface( iid, (void**)ppObj );
		}
	}
	else
	{
		ObjectId id = *ppObj ? (*ppObj)->GetObjectId() : ObjectId();
		pSerializer->SerializeProperty( propertyName, id);
	}
}

template<typename T> void SerializeIObjectPtr( ISimpleSerializer* pSerializer, const char* propertyName, T** ppObj )
{
	SerializeIObjectPtr(pSerializer, propertyName, ppObj, T::s_interfaceID);
}

struct IObjectUtils
{

	// Standard setup of a new entity and linking it to an IObject
	// Should be called between construction of object, and call to object Init() method
	static IAUEntity* CreateEntityForObject(IObject *pObj, const char* entityName, bool bEntityNameIsUnique=true)
	{
		IEntitySystem* pEntitySystem = PerModuleInterface::g_pSystemTable->pEntitySystem;

		AUEntityId id = 0;
		if (bEntityNameIsUnique)
		{
			IAUEntity* pExisting = pEntitySystem->Get(entityName);
			id = (pExisting && !pExisting->GetObject()) ? pExisting->GetId() : 0;
		}
		if (!id)
		{
			id = pEntitySystem->Create( entityName );
		}

		IAUEntity* pEntity = pEntitySystem->Get( id );
		IEntityObject* pBase( 0 );
		pObj->GetInterface( IID_IENTITYOBJECT, (void**)&pBase );
		if( pBase )
		{
			pBase->SetEntity( pEntity );
		}

		return pEntity;
	}

	static IObjectConstructor* GetConstructor( const char* objectType )
	{
		return  PerModuleInterface::g_pSystemTable->pObjectFactorySystem->GetConstructor( objectType );
	}

	static ConstructorId GetConstructorId( const char* objectType )
	{
		  IObjectConstructor* pConstructor = GetConstructor( objectType );
		  if( pConstructor )
		  {
			  return pConstructor->GetConstructorId();
		  }
		  else
		  {
			  return InvalidId;
		  }
	}


	static IObject* CreateObjectAndEntity( const char* objectType, const char* entityName, bool bEntityNameIsUnique=true )
	{
		IObject* pObj = 0;
		IObjectConstructor* pConstructor = GetConstructor( objectType );
		if( pConstructor )
		{
			pObj = pConstructor->Construct();
			IObjectUtils::CreateEntityForObject( pObj, entityName, bEntityNameIsUnique );
			pObj->Init(true);
		}
		else
		{
			PerModuleInterface::g_pSystemTable->pLogSystem->Log( eLV_ERRORS, "CreateObjectAndEntity: Could not find constructor: %s\n", objectType  );
			pObj = 0;
		}
		return pObj;
	}

	static IObject* CreateObject( const char* objectType )
	{
		IObject* pObj = 0;
		IObjectConstructor* pConstructor = GetConstructor( objectType );
		if( pConstructor )
		{
			pObj = pConstructor->Construct();
			pObj->Init(true);
		}
		else
		{
			PerModuleInterface::g_pSystemTable->pLogSystem->Log( eLV_ERRORS, "CreateObject: Could not find constructor: %s\n", objectType  );
			pObj = 0;
		}
		return pObj;
	}
	template <class T> static void CreateObject( T **pInterface, const char * objectType )
	{
		IObject* pObj = CreateObject( objectType );
		if( pObj )
		{
			pObj->GetInterface( pInterface );
			if( 0 == *pInterface )
			{
				//mismatched type
				PerModuleInterface::g_pSystemTable->pLogSystem->Log( eLV_ERRORS, "CreateObject: requested objectType does not support interface %s\n", objectType  );
				delete pObj;
			}
		}
		else
		{
			*pInterface = 0;
		}
	}

	
	static IObjectConstructor* GetConstructor( ConstructorId constructor )
	{
		return  PerModuleInterface::g_pSystemTable->pObjectFactorySystem->GetConstructor( constructor );
	}


	static IObject* CreateObject( ConstructorId constructor )
	{
		IObject* pObj = 0;
		IObjectConstructor* pConstructor = GetConstructor( constructor );
		if( pConstructor )
		{
			pObj = pConstructor->Construct();
			pObj->Init(true);
		}
		else
		{
			PerModuleInterface::g_pSystemTable->pLogSystem->Log( eLV_ERRORS, "CreateObject: Could not find constructor\n"  );
		}
		return pObj;
	}

	template <class T> static void CreateObject( T **pInterface, ConstructorId constructor )
	{
		IObject* pObj = CreateObject( constructor );
		if( pObj )
		{
			pObj->GetInterface( pInterface );
			if( 0 == *pInterface )
			{
				//mismatched type
				PerModuleInterface::g_pSystemTable->pLogSystem->Log( eLV_ERRORS, "CreateObject: requested objectType does not support interface\n"  );
				delete pObj;
			}
		}
		else
		{
			*pInterface = 0;
		}

	}

	template <class T> static void GetObject( T **pInterface, ObjectId id )
	{
		IObjectFactorySystem* pFactorySystem = PerModuleInterface::g_pSystemTable->pObjectFactorySystem;
		IObject* pObj = pFactorySystem->GetObject(id);
		if (pObj)
			pObj->GetInterface( T::s_interfaceID, (void**)pInterface );
		else
			*pInterface = NULL;
	}



	static bool UniqueObjectExists( const char* objectType )
	{
		IObjectConstructor* pConstructor = PerModuleInterface::g_pSystemTable->pObjectFactorySystem->GetConstructor( objectType );
		return ( pConstructor ? pConstructor->GetNumberConstructedObjects() == 1 : false );
	}

	static IObject* CreateUniqueObjectAndEntity( const char* objectType, const char* entityName )
	{
		IObject* pObj = 0;
		if (!UniqueObjectExists(objectType))
		{
			pObj = CreateObjectAndEntity(objectType, entityName);
		}
		return pObj;
	}

	static IObject* CreateUniqueObject( const char* objectType )
	{
		IObject* pObj = 0;
		if (!UniqueObjectExists(objectType))
		{
			IObjectConstructor* pConstructor = PerModuleInterface::g_pSystemTable->pObjectFactorySystem->GetConstructor( objectType );
			if (pConstructor)
			{
				pObj = pConstructor->Construct();
				pObj->Init(true);
			}
			else
			{
				PerModuleInterface::g_pSystemTable->pLogSystem->Log( eLV_ERRORS, "CreateUniqueObject: Could not find constructor: %s\n", objectType  );
			}
		}
		return pObj;
	}

	static IObject* GetUniqueObject( const char* objectType )
	{
		IObject* pObj = 0;
		if (UniqueObjectExists(objectType))
		{
			IObjectConstructor* pConstructor = PerModuleInterface::g_pSystemTable->pObjectFactorySystem->GetConstructor( objectType );
			pObj = pConstructor->GetConstructedObject(0);
		}
		return pObj;
	}

	static void* GetUniqueInterface( const char* objectType, InterfaceID iid )
	{
		void* pVoid = 0;
		IObject* pObj = GetUniqueObject( objectType );
		if (pObj)
		{
			pObj->GetInterface( iid, (void**)&pVoid );
		}

		return pVoid;
	}

	static void DestroyObjectAndEntity( AUEntityId entityId )
	{
		IEntitySystem* pEntitySystem = PerModuleInterface::g_pSystemTable->pEntitySystem;
		IAUEntity* pEntity = pEntitySystem->Get(entityId);
		if (pEntity)
		{
			delete pEntity->GetObject();
			pEntitySystem->Destroy( pEntity->GetId() );
		}		
	}
};

#endif //IOBJECTUTILS_INCLUDED