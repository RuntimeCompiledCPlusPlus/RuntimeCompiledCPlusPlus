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

#include "ObjectFactorySystem.h"

#include "../../Systems/Systems.h"
#include "../../RuntimeCompiler/ObjectInterface.h"
#include "../../RuntimeCompiler/ObjectInterfacePerModule.h"
#include "../../Systems/SimpleSerializer/SimpleSerializer.h"
#include "../../Systems/IlogSystem.h"
#include "../../Systems/ISystem.h"
#include "../../Examples/SimpleTest/Environment.h"	//TODO: Move exception handling to systems
#include "../../Examples/SimpleTest/IObject.h"		//TODO: Move to systems

IObjectConstructor* ObjectFactorySystem::GetConstructor( const char* type ) const
{
	CONSTRUCTORMAP::const_iterator found =  m_ConstructorIds.find( type );
	if( found != m_ConstructorIds.end() )
	{
		return m_Constructors[ found->second ];
	}
	return 0;
}

ConstructorId ObjectFactorySystem::GetConstructorId( const char* type ) const
{
	CONSTRUCTORMAP::const_iterator found =  m_ConstructorIds.find( type );
	if( found != m_ConstructorIds.end() )
	{
		return found->second;
	}
	return InvalidId;
}

IObjectConstructor* ObjectFactorySystem::GetConstructor( ConstructorId id ) const
{
	if( id < m_Constructors.size() )
	{
		return m_Constructors[ id ];
	}
	return 0;
}

bool ProtectedConstruct( IObjectConstructor* pConstructor )
{
	__try
	{
		pConstructor->Construct();
	}
	__except( RuntimeExceptionFilter() )
	{
		return false;
	}
	return true;
}

bool ProtectedSerialize( IObject* pObject, SimpleSerializer& serializer )
{
	__try
	{
		serializer.Serialize( pObject );
	}
	__except( RuntimeExceptionFilter() )
	{
		return false;
	}
	return true;
}

bool ProtectedInit( IObject* pObject )
{
	__try
	{
		pObject->Init(false);
	}
	__except( RuntimeExceptionFilter() )
	{
		return false;
	}
	return true;
}

void ObjectFactorySystem::AddConstructors( IAUDynArray<IObjectConstructor*> &constructors )
{
	ILogSystem *pLogSystem = gSys->pLogSystem;

	//serialize all out
	SimpleSerializer serializer;

	pLogSystem->Log( eLV_EVENTS, "Serializing out from %d old constructors\n", m_Constructors.size());

	//currently we don't protect the serialize out... should perhaps do so.
	serializer.SetIsLoading( false );
	for( size_t i = 0; i < m_Constructors.size(); ++i )
	{
		IObjectConstructor* pOldConstructor = m_Constructors[i];
		size_t numObjects = pOldConstructor->GetNumberConstructedObjects();
		for( size_t j = 0; j < numObjects; ++j )
		{
			IObject* pOldObject = pOldConstructor->GetConstructedObject( j );
			if (pOldObject)
			{
				serializer.Serialize( pOldObject );
			}		
		}
	}

	pLogSystem->Log( eLV_EVENTS, "Swapping in and creating objects for %d new constructors\n", constructors.Size());

	std::vector<IObjectConstructor*> oldConstructors( m_Constructors );

	bool bConstructionOK = true;
	//swap old constructors with new ones and create new objects
	for( size_t i = 0; i < constructors.Size(); ++i )
	{
		IObjectConstructor* pConstructor = constructors[i];
		//replace constructor, but if one exists then replace objects
		IObjectConstructor* pOldConstructor = GetConstructor( pConstructor->GetName() );
		// Reconstruct objects, starting at end to reduce overhead in factory container
		if( pOldConstructor )
		{
			// replace and construct
			pConstructor->SetConstructorId( pOldConstructor->GetConstructorId() );
			m_Constructors[ pConstructor->GetConstructorId() ] = pConstructor;
			for( PerTypeObjectId objId = 0; objId < pOldConstructor->GetNumberConstructedObjects(); ++ objId )
			{
				// create new object
				if( pOldConstructor->GetConstructedObject( objId ) )
				{
					if( !ProtectedConstruct( pConstructor ) )
					{
						bConstructionOK = false;
						break;
					}
				}
				else
				{
					pConstructor->ConstructNull();
				}
			}

		}
		else
		{
			ConstructorId id = m_Constructors.size();
			m_ConstructorIds[ pConstructor->GetName() ] = id;
			m_Constructors.push_back( pConstructor );
			pConstructor->SetConstructorId( id );
		}
	}

	bool bSerializeOK = true;
	bool bInitOk = true;
	if( bConstructionOK )
	{
		pLogSystem->Log( eLV_EVENTS, "Serialising in...\n");

		//serialize back
		serializer.SetIsLoading( true );
		for( size_t i = 0; i < m_Constructors.size() && bSerializeOK == true; ++i )
		{
			IObjectConstructor* pConstructor = m_Constructors[i];
			for( PerTypeObjectId objId = 0; objId < pConstructor->GetNumberConstructedObjects(); ++ objId )
			{
				// Serialize new object
				IObject* pObject = pConstructor->GetConstructedObject( objId );
				if (pObject)
				{
					if( !ProtectedSerialize( pObject, serializer ) )
					{
						serializer.SetIsLoading( true );//reset object pointer to prevent assert (ugly, TODO remove)
						bSerializeOK = false;
						break;
					}
				}
			}
		}

		// Do a second pass, initializing objects now that they've all been serialized
		if ( bSerializeOK )
		{
			pLogSystem->Log( eLV_EVENTS, "Initialising and testing new serialisation...\n");

			for( size_t i = 0; i < m_Constructors.size() && bSerializeOK == true; ++i )
			{
				IObjectConstructor* pConstructor = m_Constructors[i];
				for( PerTypeObjectId objId = 0; objId < pConstructor->GetNumberConstructedObjects(); ++ objId )
				{
					IObject* pObject = pConstructor->GetConstructedObject( objId );
					if (pObject)
					{
						if( !ProtectedInit( pObject ) )
						{
							bInitOk = false;
							break;
						}

						if( oldConstructors.size() <= i || oldConstructors[ i ] != m_Constructors[ i ] )
						{
							//test serialize out for all new objects, we assume old objects are OK.
							SimpleSerializer tempSerializer;
							tempSerializer.SetIsLoading( false );
							if( !ProtectedSerialize( pObject, tempSerializer ) )
							{
								bSerializeOK = false;
								break;
							}
						}
					}
				}
			}
		}
	}
	
	if( !bSerializeOK || !bConstructionOK || !bInitOk )
	{
		if( !bSerializeOK )
		{
			pLogSystem->Log( eLV_ERRORS, "Exception during object serialization, switching back to previous objects" );
		}
		else
		{
			pLogSystem->Log( eLV_ERRORS, "Exception during object construction, switching back to previous objects" );
		}

		//swap back to new constructors before everything is serialized back in
		m_Constructors = oldConstructors;

		//serialize back with old objects - could cause exception which isn't handled, but hopefully not.
		serializer.SetIsLoading( true );
		for( size_t i = 0; i < m_Constructors.size(); ++i )
		{
			IObjectConstructor* pConstructor = m_Constructors[i];
			for( PerTypeObjectId objId = 0; objId < pConstructor->GetNumberConstructedObjects(); ++ objId )
			{
				// Iserialize new object
				IObject* pObject = pConstructor->GetConstructedObject( objId );
				if (pObject)
				{
					serializer.Serialize( pObject );
				}			
			}
		}

		// Do a second pass, initializing objects now that they've all been serialized
		for( size_t i = 0; i < m_Constructors.size(); ++i )
		{
			IObjectConstructor* pConstructor = m_Constructors[i];
			for( PerTypeObjectId objId = 0; objId < pConstructor->GetNumberConstructedObjects(); ++ objId )
			{
				IObject* pObject = pConstructor->GetConstructedObject( objId );
				if (pObject)
				{
					pObject->Init(false);
				}			
			}
		}
	}
	else
	{
		pLogSystem->Log( eLV_EVENTS, "Object swap completed\n");
	}

	try
	{
		//delete old objects which have been replaced
		for( size_t i = 0; i < oldConstructors.size(); ++i )
		{
			if( oldConstructors[i] != m_Constructors[i] )
			{
				//TODO: could put a constructor around this.
				//constructor has been replaced
				IObjectConstructor* pOldConstructor = oldConstructors[i];
				size_t numObjects = pOldConstructor->GetNumberConstructedObjects();
				for( size_t j = 0; j < numObjects; ++j )
				{
					IObject* pOldObject = pOldConstructor->GetConstructedObject( j );
					if( pOldObject )
					{
						pOldObject->_isRuntimeDelete = true;
						delete pOldObject;
					}
				}
			}
		}
	}
	catch(...)
	{
		//do nothing.
		pLogSystem->Log( eLV_ERRORS, "Exception during object destruction of old objects, leaking." );
	}

	// Notify any listeners that constructors have changed
	TObjectFactoryListeners::iterator it = m_Listeners.begin();
	TObjectFactoryListeners::iterator itEnd = m_Listeners.end();
	while (it != itEnd)
	{
		(*it)->OnConstructorsAdded();
		++it;
	}
}

void ObjectFactorySystem::GetAll(IAUDynArray<IObjectConstructor*> &constructors) const
{
	constructors.Resize(m_Constructors.size());
	std::vector<IObjectConstructor*>::const_iterator it = m_Constructors.begin();
	std::vector<IObjectConstructor*>::const_iterator itEnd = m_Constructors.end();
	for(int i = 0; it != itEnd; ++it, ++i)
	{
		constructors[i] = *it;
	}
}

IObject* ObjectFactorySystem::GetObject( ObjectId id ) const
{
	IObjectConstructor* pConstructor = ObjectFactorySystem::GetConstructor( id.m_ConstructorId );
	if( pConstructor )
	{
		return pConstructor->GetConstructedObject( id.m_PerTypeId );
	}
	return 0;
}

void ObjectFactorySystem::AddListener(IObjectFactoryListener* pListener)
{
	m_Listeners.insert(pListener);
}

void ObjectFactorySystem::RemoveListener(IObjectFactoryListener* pListener)
{
	m_Listeners.erase(pListener);
}
