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

#include "IPerceptionManager.h"
#include "IObjectUtils.h"
#include "IGameObject.h"
#include "GlobalParameters.h"

#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"

#include <assert.h>
#include <set>
#include <vector>
#include <algorithm>


const AUVec3f ZERO(0,0,0);


class PerceptionManager: public IPerceptionManager
{ 
	// We have two sets of typedefs here, one for fast access during runtime, and another
	// that is used for safe storage during serialization
	typedef std::vector<IGameObject*> TGameObjects;
	typedef std::vector<ObjectId> TGameObjectIds;

public:
	PerceptionManager() 
		: m_bDoUpdate(false)
		, m_pGlobalParameters(0)
	{
		m_Objects.resize(EGO_COUNT);
	}

	virtual ~PerceptionManager()
	{
		((IGameManager*)IObjectUtils::GetUniqueInterface( "GameManager", IID_IGAMEMANAGER ))->RemoveListener(this);
	}


	// IObject

	virtual void Serialize( ISimpleSerializer *pSerializer )
	{
		AU_ASSERT(pSerializer);

		SERIALIZE(m_bDoUpdate);
		SerializeObjectsList( pSerializer );
	}

	virtual void Init( bool isFirstInit )
	{
		IGameManager* pGameManager = (IGameManager*)IObjectUtils::GetUniqueInterface( "GameManager", IID_IGAMEMANAGER );

		pGameManager->AddListener(this);
		m_pGlobalParameters = pGameManager->GetGlobalParameters();
	}

	// ~IObject

	// IGameEventListener

	virtual void OnGameReset() 
	{
		for (int i=0; i<EGO_COUNT; ++i)
		{
			m_Objects[i].clear();
		}
	}

	virtual void OnStateChange( EGameState newState )
	{
		m_bDoUpdate = (newState == EGS_PLAYING);
	}

	virtual void OnGameObjectCreated( IGameObject* pGameObject )
	{
		m_Objects[pGameObject->GetGameObjectType()].push_back( pGameObject );
	}

	virtual void OnGameObjectAboutToDestroy( IGameObject* pGameObject )
	{
		TGameObjects& data = m_Objects[pGameObject->GetGameObjectType()];
		TGameObjects::iterator it = std::find(data.begin(), data.end(), pGameObject);
		if (it != data.end())
		{
			data.erase(it);
		}
	}

	// ~IGameEventListener

	// IPerceptionManager

	virtual int GetNumberPerceived( const IGameObject* pPerceiver, EGameObject perceivedType ) const
	{
		AUVec3f center = pPerceiver->GetEntity()->GetPosition();
		float radius = m_pGlobalParameters->go[pPerceiver->GetGameObjectType()].perceptionDist[perceivedType];
		m_workingData.clear();

		// Fill working array with results
		DoGetPerceived( m_Objects[perceivedType], pPerceiver, center, radius );

		return (int)m_workingData.size();
	}

	virtual int GetNumberPerceived( const IGameObject* pPerceiver ) const
	{
		int num = 0;

		for (int i=0; i<EGO_COUNT; ++i)
		{
			num += GetNumberPerceived( pPerceiver, (EGameObject)i );
		}

		return num;
	}

	virtual void GetPerceived( const IGameObject* pPerceiver, EGameObject perceivedType, IAUDynArray<ObjectId>& objects ) const
	{
		objects.Clear();
		DoAddPerceived( pPerceiver, perceivedType, objects );
	}

	virtual void AddPerceived( const IGameObject* pPerceiver, EGameObject perceivedType, IAUDynArray<ObjectId>& objects ) const
	{
		DoAddPerceived( pPerceiver, perceivedType, objects );
	}

	virtual void GetPerceived( const IGameObject* pPerceiver, IAUDynArray<ObjectId>& objects ) const
	{
		objects.Clear();
		AUVec3f center = pPerceiver->GetEntity()->GetPosition();
		m_workingData.clear();

		// Fill working array with results
		for (int i=0; i<EGO_COUNT; ++i)
		{
			float radius = m_pGlobalParameters->go[pPerceiver->GetGameObjectType()].perceptionDist[i];

			DoGetPerceived( m_Objects[i], pPerceiver, center, radius );
		}
		
		DoAddToArray(objects);
	}
	
	virtual AUVec3f GetGlobalAveragePos( EGameObject perceivedType ) const
	{
		AUVec3f avg = ZERO;
		const TGameObjects& data = m_Objects[perceivedType];
		size_t count = data.size();
		if (count > 0)
		{
			for (size_t i=0; i<count; ++i)
			{
				avg += data[i]->GetEntity()->GetPosition();
			}

			avg /= (float)count;
		}
		
		return avg;		
	}

	virtual AUVec3f GetGlobalAveragePos() const
	{
		AUVec3f avg = ZERO;
		
		for (int i=0; i<EGO_COUNT; ++i)
		{
			avg += GetGlobalAveragePos( (EGameObject)i );
		}

		avg /= EGO_COUNT;
		return avg;		
	}

	virtual AUVec3f GetPerceivedAveragePos( const IGameObject* pPerceiver, EGameObject perceivedType ) const
	{
		AUVec3f center = pPerceiver->GetEntity()->GetPosition();
		float radius = m_pGlobalParameters->go[pPerceiver->GetGameObjectType()].perceptionDist[perceivedType];
		m_workingData.clear();

		// Fill working array with results
		DoGetPerceived( m_Objects[perceivedType], pPerceiver, center, radius );

		// Calculate average position
		AUVec3f avg = ZERO;
		size_t count = m_workingData.size();
		if (count > 0)
		{
			for (size_t i=0; i<count; ++i)
			{
				avg += m_workingData[i]->GetEntity()->GetPosition();
			}

			avg /= (float)count;
		}

		return avg;		
	}

	virtual AUVec3f GetPerceivedAveragePos( const IGameObject* pPerceiver ) const
	{
		AUVec3f avg = ZERO;

		for (int i=0; i<EGO_COUNT; ++i)
		{
			avg += GetPerceivedAveragePos( pPerceiver, (EGameObject)i );
		}

		avg /= EGO_COUNT;
		return avg;		
	}


	// ~IPerceptionManager


private:

	void DoAddPerceived( const IGameObject* pPerceiver, EGameObject perceivedType, IAUDynArray<ObjectId>& objects ) const
	{
		AUVec3f center = pPerceiver->GetEntity()->GetPosition();
		float radius = m_pGlobalParameters->go[pPerceiver->GetGameObjectType()].perceptionDist[perceivedType];
		m_workingData.clear();

		// Fill working array with results
		DoGetPerceived( m_Objects[perceivedType], pPerceiver, center, radius );

		DoAddToArray(objects);
	}


	// Pushes results into m_workingData
	void DoGetPerceived( const TGameObjects& data, const IGameObject* pPerceiver, const AUVec3f& center, float radius ) const
	{
		size_t count = data.size();
		for (size_t j=0; j<count; ++j)
		{
			IGameObject* pObj = data[j];
			float dist = (pObj->GetEntity()->GetPosition() - center).Magnitude();
			if ( pObj->GetEntityId() != pPerceiver->GetEntityId() && dist < radius )
			{
				m_workingData.push_back( pObj );
			}
		}
	}

	// Fills with current contents of m_workingData
	void DoAddToArray( IAUDynArray<ObjectId>& objects ) const
	{
		size_t count = m_workingData.size();
		size_t origCount = objects.Size();
		objects.Resize(origCount + count);
		for (size_t i=0; i < count; ++i)
		{
			objects[i+origCount] = m_workingData[i]->GetObjectId();
		}
	}


	void SerializeObjectsList( ISimpleSerializer *pSerializer )
	{
		std::vector<TGameObjectIds> m_ObjectIds; 

		if ( !pSerializer->IsLoading() )
		{
			// Create a collection of ObjectIds that matches m_Objects pointer collection 

			m_ObjectIds.resize(EGO_COUNT);
			for (int i=0; i<EGO_COUNT; ++i)
			{
				size_t count = m_Objects[i].size();
				m_ObjectIds[i].resize( count );
				
				for (size_t j=0; j<count; ++j)
				{
					m_ObjectIds[i][j] = m_Objects[i][j]->GetObjectId();
				}
			}
		}

		SERIALIZE(m_ObjectIds);
		
		if ( pSerializer->IsLoading() )
		{
			// Rebuild m_objects pointer collection

			for (int i=0; i<EGO_COUNT; ++i)
			{
				size_t count = m_ObjectIds[i].size();
				m_Objects[i].clear();
				m_Objects[i].resize( count );

				for (size_t j=0; j<count; ++j)
				{
					IGameObject* pGameObject = 0;
					IObjectUtils::GetObject( &pGameObject, m_ObjectIds[i][j] );

					m_Objects[i][j] = pGameObject;
				}
			}
		}	
	}

	// Private Members

	std::vector<TGameObjects> m_Objects; // Keeps its own set of valid GameObjects for more efficient calculations
	bool m_bDoUpdate;
	GlobalParameters* m_pGlobalParameters;

	mutable TGameObjects m_workingData; // Vector to be used when making calculations to avoid constant allocations
};

REGISTERCLASS(PerceptionManager);




