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

#include "IPhysicsManager.h"
#include "IGameManager.h"
#include "IGameObject.h"
#include "IObjectUtils.h"
#include "ICameraControl.h"
#include "GlobalParameters.h"

#include "../../Common/Math.inl"
#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../RuntimeCompiler/IFileChangeNotifier.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/IAssetSystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"
#include "../../Systems/IGUISystem.h"
#include "../../Systems/IGame.h"

#include <assert.h>
#include <limits>
#include <algorithm>


class PhysicsManager: public IPhysicsManager, public IGameEventListener
{
	// We have two sets of typedefs here, one for fast access during runtime, and another
	// that is used for safe storage during serialization
	typedef std::vector<IGameObject*> TGameObjects;
	typedef std::vector<ObjectId> TGameObjectIds;

public:
	PhysicsManager() 
	{
		m_Objects.resize(EGT_COUNT);
	}

	virtual ~PhysicsManager()
	{
		((IGameManager*)IObjectUtils::GetUniqueInterface( "GameManager", IID_IGAMEMANAGER ))->RemoveListener(this);
	}


	// IObject

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		AU_ASSERT(pSerializer);
		SERIALIZE(m_fWorldCenteringDist);
		SerializeObjectsList( pSerializer );
	}

	virtual void Init( bool isFirstInit )
	{
		IGameManager* pGameManager = (IGameManager*)IObjectUtils::GetUniqueInterface( "GameManager", IID_IGAMEMANAGER );
		pGameManager->AddListener(this);

		float height, width;
		PerModuleInterface::g_pSystemTable->pGame->GetWindowSize( width, height );

		// Slightly reduce dimensions so object never cross window edges (since their position is in the center of object
		width -= 50;
		height -= 50;

		m_fWorldCenteringDist.SetX( width * width * 0.25f );
		m_fWorldCenteringDist.SetZ( height * height * 0.25f );
	}

	// ~IObject

	// IGameEventListener

	virtual void OnGameReset() 
	{
		for (int i=0; i<EGT_COUNT; ++i)
		{
			m_Objects[i].clear();
		}
	}

	virtual void OnGameObjectCreated( IGameObject* pGameObject )
	{
		m_Objects[pGameObject->GetGameTeam()].push_back( pGameObject );
	}

	virtual void OnGameObjectAboutToDestroy( IGameObject* pGameObject )
	{
		TGameObjects& data = m_Objects[pGameObject->GetGameTeam()];
		TGameObjects::iterator it = std::find(data.begin(), data.end(), pGameObject);
		if (it != data.end())
		{
			data.erase(it);
		}
	}

	// ~IGameEventListener

	// IPhysicsManager

	virtual void RequestPositionUpdate( IGameObject* pGameObject, const AUVec3f& desiredPosition, float frameDelta )
	{
		AUVec3f pos = desiredPosition;

		ApplyGameAreaRepulsionField( pGameObject, pos, frameDelta );
		ApplyTeamRepulsionFields( pGameObject, pos, frameDelta );
		
		CheckForCollisions( pGameObject, pos, frameDelta );

		pGameObject->GetEntity()->SetPosition(pos);
	}

	virtual bool IsHeadingToGameBounds( const AUVec3f& position, const AUVec3f& velocity ) const
	{
		float edgeProportion = (position.x * position.x) / m_fWorldCenteringDist.x +
			(position.z * position.z) / m_fWorldCenteringDist.z;
		return ( edgeProportion > 0.8f && position.Dot(velocity) > cos( M_PI_4 ) );
	}

	// ~IPhysicsManager


private:

	void ApplyGameAreaRepulsionField( IGameObject* pGameObject, AUVec3f& desiredPosition, float frameDelta )
	{
		float edgeProportion = (desiredPosition.x * desiredPosition.x) / m_fWorldCenteringDist.x +
			(desiredPosition.z * desiredPosition.z) / m_fWorldCenteringDist.z;
		float forceStart = 0.9f;
		if ( edgeProportion > forceStart )
		{
			float centeringMagnitude = (edgeProportion - forceStart) / (1.0f - forceStart);
			AUVec3f dir = (-desiredPosition).GetNormalised(); // Towards center
			desiredPosition += dir * pGameObject->GetMaxSpeed() * frameDelta * centeringMagnitude;
		}
	}

	void ApplyTeamRepulsionFields( IGameObject* pGameObject, AUVec3f& desiredPosition, float frameDelta )
	{
		const AUVec3f& refPos = pGameObject->GetEntity()->GetPosition();
		const float refDist = pGameObject->GetCollisionRadius();
		const float forceStartMultiplier = 1.5f;

		TGameObjects& data = m_Objects[pGameObject->GetGameTeam()];
		TGameObjects::iterator it = data.begin();
		TGameObjects::iterator itEnd = data.end();
		while (it != itEnd)
		{
			IGameObject* pTeamObject = *it;
			if (pTeamObject != pGameObject)
			{
				const AUVec3f& teamObjectPos = pTeamObject->GetEntity()->GetPosition();
				const float minAllowedDist = refDist + pTeamObject->GetCollisionRadius();
				const float distSqr = (refPos - teamObjectPos).MagnitudeSqr();
				const float forceStart = minAllowedDist * forceStartMultiplier;
				if ( distSqr < forceStart * forceStart )
				{
					float repulsionMagnitude = (forceStart - sqrt(distSqr)) / ( forceStart * ( 1.0f - 1.0f / forceStartMultiplier ) );
					AUVec3f dir = (refPos - teamObjectPos).GetNormalised();
					desiredPosition += dir * pGameObject->GetMaxSpeed() * frameDelta * repulsionMagnitude;
				}
			}
			
			++it;
		}
	}

	void CheckForCollisions( IGameObject* pGameObject, AUVec3f& desiredPosition, float frameDelta )
	{
		const AUVec3f& refPos = pGameObject->GetEntity()->GetPosition();
		const float refDist = pGameObject->GetCollisionRadius();

		for (int i=0; i<EGT_COUNT; ++i)
		{
			TGameObjects& data = m_Objects[i];
			TGameObjects::iterator it = data.begin();
			TGameObjects::iterator itEnd = data.end();
			while (it != itEnd)
			{
				IGameObject* pTestObject = *it;
				if (pTestObject != pGameObject)
				{
					const AUVec3f& testObjectPos = pTestObject->GetEntity()->GetPosition();
					float distSqr = (refPos - testObjectPos).MagnitudeSqr();
					float minAllowedDist = refDist + pTestObject->GetCollisionRadius();
					if ( (distSqr <= minAllowedDist * minAllowedDist) && (pGameObject->GetGameTeam() != pTestObject->GetGameTeam()) )
					{
						// Set desired position to edge of collision radii		
						AUVec3f dir = (refPos - testObjectPos).GetNormalised();
						desiredPosition = testObjectPos + dir * minAllowedDist;

						pTestObject->OnCollision( pGameObject );
						pGameObject->OnCollision( pTestObject );
					}
				}

				++it;
			}
		}
	}

	void SerializeObjectsList( ISimpleSerializer *pSerializer )
	{
		std::vector<TGameObjectIds> m_ObjectIds; 

		if ( !pSerializer->IsLoading() )
		{
			// Create a collection of ObjectIds that matches m_Objects pointer collection 

			m_ObjectIds.resize(EGT_COUNT);
			for (int i=0; i<EGT_COUNT; ++i)
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

			for (int i=0; i<EGT_COUNT; ++i)
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

	std::vector<TGameObjects> m_Objects; // set of gameobjects separated by team (this is handy for potential field calculations)
	AUVec3f m_fWorldCenteringDist;
};

REGISTERCLASS(PhysicsManager);




