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

void ObjectFactorySystem::ProtectedObjectSwapper::ProtectedFunc()
{
	m_ProtectedPhase = PHASE_SERIALIZEOUT;

	// serialize all out
	if( m_pLogger ) m_pLogger->LogInfo( "Serializing out from %d old constructors...\n", (int)m_ConstructorsOld.size());

	// use a temporary serializer in case there is an exception, so preserving any old state (if there is any)
	m_Serializer.SetIsLoading( false );
	for( size_t i = 0; i < m_ConstructorsOld.size(); ++i )
	{
		IObjectConstructor* pOldConstructor = m_ConstructorsOld[i];
		size_t numObjects = pOldConstructor->GetNumberConstructedObjects();
		for( size_t j = 0; j < numObjects; ++j )
		{
			IObject* pOldObject = pOldConstructor->GetConstructedObject( j );
			if (pOldObject)
			{
				m_Serializer.Serialize( pOldObject );
			}		
		}
	}
	// swap serializer
	if( m_pLogger ) m_pLogger->LogInfo( "Swapping in and creating objects for %d new constructors...\n", (int)m_ConstructorsToAdd.size());

	m_ProtectedPhase = PHASE_CONSTRUCTNEW;
	TConstructors& constructorsNew = m_pObjectFactorySystem->m_Constructors;

	//swap old constructors with new ones and create new objects
	for( size_t i = 0; i < m_ConstructorsToAdd.size(); ++i )
	{
		IObjectConstructor* pConstructor = m_ConstructorsToAdd[i];
		//replace constructor, but if one exists then replace objects
		IObjectConstructor* pOldConstructor = m_pObjectFactorySystem->GetConstructor( pConstructor->GetName() );

        if( pOldConstructor == pConstructor )
        {
            // don't add constructor if it's already in existance
            continue;
        }

		// Reconstruct objects, starting at end to reduce overhead in factory container
		if( pOldConstructor )
		{
			// replace and construct
			pConstructor->SetConstructorId( pOldConstructor->GetConstructorId() );
			constructorsNew[ pConstructor->GetConstructorId() ] = pConstructor;
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
			m_ConstructorsReplaced.push_back( pOldConstructor );
		}
		else
		{
            ConstructorId id = constructorsNew.size();
			m_pObjectFactorySystem->m_ConstructorIds[ pConstructor->GetName() ] = id;
			constructorsNew.push_back( pConstructor );
			pConstructor->SetConstructorId( id );
		}
	}

	if( m_pLogger ) m_pLogger->LogInfo( "Serialising in...\n");

	//serialize back
	m_ProtectedPhase = PHASE_SERIALIZEIN;
	m_Serializer.SetIsLoading( true );
	for( size_t i = 0; i < constructorsNew.size(); ++i )
	{
		IObjectConstructor* pConstructor = constructorsNew[i];
		for( PerTypeObjectId objId = 0; objId < pConstructor->GetNumberConstructedObjects(); ++ objId )
		{
			// Serialize new object
			IObject* pObject = pConstructor->GetConstructedObject( objId );
			if (pObject)
			{
				m_Serializer.Serialize( pObject );
			}
		}
	}

    // auto construct singletons
    // now in 2 phases - construct then init
    m_ProtectedPhase = PHASE_AUTOCONSTRUCTSINGLETONS;
    std::vector<bool> bSingletonConstructed( constructorsNew.size(), false );
	if( m_pLogger ) m_pLogger->LogInfo( "Auto Constructing Singletons...\n");
	for( size_t i = 0; i < constructorsNew.size(); ++i )
	{
		IObjectConstructor* pConstructor = constructorsNew[i];
        if( pConstructor->GetIsAutoConstructSingleton() )
        {
            if( 0 == pConstructor->GetNumberConstructedObjects() )
            {
                pConstructor->GetSingleton();
                bSingletonConstructed[i] = true;
            }
        }
	}


	// Do a second pass, initializing objects now that they've all been serialized
    // and testing serialization if required
	m_ProtectedPhase = PHASE_INITANDSERIALIZEOUTTEST;
    if( m_bTestSerialization )
    {
	    if( m_pLogger ) m_pLogger->LogInfo( "Initialising and testing new serialisation...\n");
    }
    else
    {
	    if( m_pLogger ) m_pLogger->LogInfo( "Initialising...\n");
    }

	for( size_t i = 0; i < constructorsNew.size(); ++i )
	{
		IObjectConstructor* pConstructor = constructorsNew[i];
		for( PerTypeObjectId objId = 0; objId < pConstructor->GetNumberConstructedObjects(); ++ objId )
		{
			IObject* pObject = pConstructor->GetConstructedObject( objId );
			if (pObject)
			{
                // if a singleton was newly constructed in earlier phase, pass true to init.
				pObject->Init( bSingletonConstructed[i] );

				if( m_bTestSerialization && ( m_ConstructorsOld.size() <= i || m_ConstructorsOld[ i ] != constructorsNew[ i ] ) )
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
	for( size_t i = 0; i < m_ConstructorsOld.size(); ++i )
	{
		if( m_ConstructorsOld[i] != constructorsNew[i] )
		{
			//TODO: could put a constructor around this.
			//constructor has been replaced
			IObjectConstructor* pOldConstructor = m_ConstructorsOld[i];
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
			pOldConstructor->ClearIfAllDeleted();
			assert( 0 == pOldConstructor->GetNumberConstructedObjects() );
		}
	}
}

bool ObjectFactorySystem::HandleRedoUndo( const TConstructors& constructors )
{
	if( constructors.size() == 0 )
	{
		m_pLogger->LogInfo( "ObjectFactorySystem::HandleRedoUndo() called with no constructors.\n" );
		return true;
	}

	ProtectedObjectSwapper swapper;
	swapper.m_ConstructorsToAdd = constructors;
	swapper.m_ConstructorsOld = m_Constructors;
	swapper.m_pLogger = m_pLogger;
	swapper.m_pObjectFactorySystem = this;
	swapper.m_bTestSerialization = false; // we don't need to test as this should alraedy have been done

	swapper.m_ProtectedPhase = PHASE_NONE;
	// we use the protected function to do all serialization
    m_pRuntimeObjectSystem->TryProtectedFunction( &swapper );

	CompleteConstructorSwap( swapper );

	return !swapper.HasHadException() || ( PHASE_DELETEOLD == swapper.m_ProtectedPhase );
}

void ObjectFactorySystem::AddConstructors( IAUDynArray<IObjectConstructor*> &constructors )
{
	if( constructors.Size() == 0 )
	{
		m_pLogger->LogInfo( "ObjectFactorySystem::AddConstructors() called with no constructors.\n" );
		return;
	}

	if( m_HistoryCurrentLocation )
	{
		m_pLogger->LogInfo( "Need to fast forward undo system to current state of source code.\n" );
		while( RedoObjectConstructorChange() ) {}
	}

	ProtectedObjectSwapper swapper;
	swapper.m_ConstructorsToAdd.assign( &constructors[0], &constructors[constructors.Size() - 1] + 1 );
	swapper.m_ConstructorsOld = m_Constructors;
	swapper.m_pLogger = m_pLogger;
	swapper.m_pObjectFactorySystem = this;
	swapper.m_bTestSerialization = m_bTestSerialization;

	swapper.m_ProtectedPhase = PHASE_NONE;
	// we use the protected function to do all serialization
    m_pRuntimeObjectSystem->TryProtectedFunction( &swapper );

	CompleteConstructorSwap( swapper );

	if( m_HistoryMaxSize )
	{
		HistoryPoint historyPoint = { swapper.m_ConstructorsReplaced, swapper.m_ConstructorsToAdd };
		m_HistoryConstructors.push_back( historyPoint );
		if( (int)m_HistoryConstructors.size() > m_HistoryMaxSize )
		{
			m_HistoryConstructors.erase( m_HistoryConstructors.begin() );
		}
	}
}

void ObjectFactorySystem::CompleteConstructorSwap( ProtectedObjectSwapper& swapper )
{
	if( swapper.HasHadException() && PHASE_DELETEOLD != swapper.m_ProtectedPhase )
	{
		if( m_pLogger )
		{
			m_pLogger->LogError( "Exception during object swapping, switching back to previous objects.\n" );
			switch( swapper.m_ProtectedPhase  )
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
			case PHASE_AUTOCONSTRUCTSINGLETONS:
				m_pLogger->LogError( "\tError occured during auto construct singletons phase.\n" );
                break;
			case PHASE_INITANDSERIALIZEOUTTEST:
                if( m_bTestSerialization )
                {
				    m_pLogger->LogError( "\tError occured during Initialization and serialize test of new objects phase.\n" );
                }
                else
                {
				    m_pLogger->LogError( "\tError occured during Initialization phase.\n" );
                }
                break;
           case PHASE_DELETEOLD:
                break;
 			}
		}

		//swap back to new constructors before everything is serialized back in
		m_Constructors = swapper.m_ConstructorsOld;
		if( PHASE_SERIALIZEOUT != swapper.m_ProtectedPhase )
		{
			//serialize back with old objects - could cause exception which isn't handled, but hopefully not.
			swapper.m_Serializer.SetIsLoading( true );
			for( size_t i = 0; i < m_Constructors.size(); ++i )
			{
				IObjectConstructor* pConstructor = m_Constructors[i];
				for( PerTypeObjectId objId = 0; objId < pConstructor->GetNumberConstructedObjects(); ++ objId )
				{
					// Iserialize new object
					IObject* pObject = pConstructor->GetConstructedObject( objId );
					if (pObject)
					{
						swapper.m_Serializer.Serialize( pObject );
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
		if( swapper.HasHadException() && PHASE_DELETEOLD == swapper.m_ProtectedPhase )
		{
			if( m_pLogger ) m_pLogger->LogError( "Exception during object destruction of old objects, leaking.\n" );
		}
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


void ObjectFactorySystem::SetObjectConstructorHistorySize( int num_ )
{
	if( num_ >= m_HistoryCurrentLocation )
	{
		m_HistoryMaxSize = num_;
	}

	while( m_HistoryMaxSize < (int)m_HistoryConstructors.size() )
	{
		m_HistoryConstructors.erase( m_HistoryConstructors.begin() );
	}
}

int ObjectFactorySystem::GetObjectConstructorHistorySize()
{
	return m_HistoryMaxSize;
}

bool ObjectFactorySystem::UndoObjectConstructorChange()
{
	if( m_HistoryCurrentLocation < (int)m_HistoryConstructors.size() )
	{
		++m_HistoryCurrentLocation;
		size_t loc = m_HistoryConstructors.size() - m_HistoryCurrentLocation;
		return HandleRedoUndo( m_HistoryConstructors[ loc ].before );
	}

	return false;
}

bool ObjectFactorySystem::RedoObjectConstructorChange()
{
	if( m_HistoryCurrentLocation > 0 )
	{
		size_t loc = m_HistoryConstructors.size() - m_HistoryCurrentLocation;
		--m_HistoryCurrentLocation;
		return HandleRedoUndo( m_HistoryConstructors[ loc ].after );
	}

	return false;
}

int ObjectFactorySystem::GetObjectContstructorHistoryLocation()
{
	return m_HistoryCurrentLocation;
}
