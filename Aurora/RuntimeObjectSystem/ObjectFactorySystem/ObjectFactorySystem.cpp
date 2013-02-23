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

#include "../ObjectInterface.h"
#include "../ObjectInterfacePerModule.h"
#include "../IObject.h"
#include "../IRuntimeObjectSystem.h"


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

void ObjectFactorySystem::ProtectedFunc()
{
	m_ProtectedPhase = PHASE_SERIALIZEOUT;
	IAUDynArray<IObjectConstructor*>& constructors = *m_pNewConstructors;

	// serialize all out
	if( m_pLogger ) m_pLogger->LogInfo( "Serializing out from %d old constructors...\n", (int)m_Constructors.size());

	// use a temporary serializer in case there is an exception, so preserving any old state (if there is any)
	SimpleSerializer* pSerializer = new SimpleSerializer;
	// currently we don't protect the serialize out... should perhaps do so.
	pSerializer->SetIsLoading( false );
	for( size_t i = 0; i < m_Constructors.size(); ++i )
	{
		IObjectConstructor* pOldConstructor = m_Constructors[i];
		size_t numObjects = pOldConstructor->GetNumberConstructedObjects();
		for( size_t j = 0; j < numObjects; ++j )
		{
			IObject* pOldObject = pOldConstructor->GetConstructedObject( j );
			if (pOldObject)
			{
				pSerializer->Serialize( pOldObject );
			}		
		}
	}
	// swap serializer
	delete m_pSerializer;
	m_pSerializer = pSerializer;
	if( m_pLogger ) m_pLogger->LogInfo( "Swapping in and creating objects for %d new constructors...\n", (int)constructors.Size());

	m_ProtectedPhase = PHASE_CONSTRUCTNEW;
	m_PrevConstructors = m_Constructors;

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
					pConstructor->Construct();
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

	if( m_pLogger ) m_pLogger->LogInfo( "Serialising in...\n");

	//serialize back
	m_ProtectedPhase = PHASE_SERIALIZEIN;
	m_pSerializer->SetIsLoading( true );
	for( size_t i = 0; i < m_Constructors.size(); ++i )
	{
		IObjectConstructor* pConstructor = m_Constructors[i];
		for( PerTypeObjectId objId = 0; objId < pConstructor->GetNumberConstructedObjects(); ++ objId )
		{
			// Serialize new object
			IObject* pObject = pConstructor->GetConstructedObject( objId );
			if (pObject)
			{
				m_pSerializer->Serialize( pObject );
			}
		}
	}

	// Do a second pass, initializing objects now that they've all been serialized
	m_ProtectedPhase = PHASE_SERIALIZEOUTTEST;
	if( m_pLogger ) m_pLogger->LogInfo( "Initialising and testing new serialisation...\n");

	for( size_t i = 0; i < m_Constructors.size(); ++i )
	{
		IObjectConstructor* pConstructor = m_Constructors[i];
		for( PerTypeObjectId objId = 0; objId < pConstructor->GetNumberConstructedObjects(); ++ objId )
		{
			IObject* pObject = pConstructor->GetConstructedObject( objId );
			if (pObject)
			{
				pObject->Init(false);

				if( m_PrevConstructors.size() <= i || m_PrevConstructors[ i ] != m_Constructors[ i ] )
				{
					//test serialize out for all new objects, we assume old objects are OK.
					SimpleSerializer tempSerializer;
					tempSerializer.SetIsLoading( false );
					tempSerializer.Serialize( pObject );
				}
			}
		}
	}

	m_ProtectedPhase = PHASE_DELETEOLD;
	//delete old objects which have been replaced
	for( size_t i = 0; i < m_PrevConstructors.size(); ++i )
	{
		if( m_PrevConstructors[i] != m_Constructors[i] )
		{
			//TODO: could put a constructor around this.
			//constructor has been replaced
			IObjectConstructor* pOldConstructor = m_PrevConstructors[i];
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

void ObjectFactorySystem::AddConstructors( IAUDynArray<IObjectConstructor*> &constructors )
{
	m_ProtectedPhase = PHASE_NONE;
	m_pNewConstructors = &constructors;
	// we use the protected function to do all serialization
    m_pRuntimeObjectSystem->TryProtectedFunction( this );

	if( HasHadException() && PHASE_DELETEOLD != m_ProtectedPhase )
	{
		if( m_pLogger )
		{
			m_pLogger->LogError( "Exception during object swapping, switching back to previous objects.\n" );
			switch( m_ProtectedPhase  )
			{
            case PHASE_NONE:
                AU_ASSERT( false );
                break;
			case PHASE_SERIALIZEOUT:
				m_pLogger->LogError( "\tError occured during serialize out old objects phase.\n" );
				break;
			case PHASE_CONSTRUCTNEW:
				m_pLogger->LogError( "\tError occured during constructing new objects phase.\n" );
				break;
			case PHASE_SERIALIZEIN:
				m_pLogger->LogError( "\tError occured during serialize into the new objects phase.\n" );
				break;
			case PHASE_SERIALIZEOUTTEST:
				m_pLogger->LogError( "\tError occured during serialize test of new objects phase.\n" );
                break;
           case PHASE_DELETEOLD:
                break;
 			}
		}

		//swap back to new constructors before everything is serialized back in
		m_Constructors = m_PrevConstructors;

		if( m_pSerializer && PHASE_SERIALIZEOUT != m_ProtectedPhase )
		{
			//serialize back with old objects - could cause exception which isn't handled, but hopefully not.
			m_pSerializer->SetIsLoading( true );
			for( size_t i = 0; i < m_Constructors.size(); ++i )
			{
				IObjectConstructor* pConstructor = m_Constructors[i];
				for( PerTypeObjectId objId = 0; objId < pConstructor->GetNumberConstructedObjects(); ++ objId )
				{
					// Iserialize new object
					IObject* pObject = pConstructor->GetConstructedObject( objId );
					if (pObject)
					{
						m_pSerializer->Serialize( pObject );
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
	}
	else
	{
		if( m_pLogger ) m_pLogger->LogInfo( "Object swap completed\n");
		if( HasHadException() && PHASE_DELETEOLD == m_ProtectedPhase )
		{
			if( m_pLogger ) m_pLogger->LogError( "Exception during object destruction of old objects, leaking.\n" );
		}
	}
	m_ProtectedPhase = PHASE_NONE;
	ClearExceptions();
	delete m_pSerializer;
	m_pSerializer = 0;

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
