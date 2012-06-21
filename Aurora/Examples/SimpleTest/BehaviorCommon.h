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

#ifndef BEHAVIORCOMMON_INCLUDED
#define BEHAVIORCOMMON_INCLUDED

#include "../../RuntimeObjectSystem/RuntimeInclude.h"
RUNTIME_MODIFIABLE_INCLUDE; //adds this include to runtime tracking

#include "IBehavior.h"
#include "IObjectUtils.h"
#include "IGameObject.h"
#include "IGameManager.h"
#include "IPerceptionManager.h"
#include "IBlackboardManager.h"
#include "IPhysicsManager.h"
#include "GlobalParameters.h"
#include "IObjectUtils.h"

#include "BB_Global.h"
#include "BB_Individual_Common.h"

#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/IAssetSystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"

#include <assert.h>
#include <algorithm>


class BehaviorCommon : public IBehavior
{
public:
	BehaviorCommon() 
		: m_pOwner(0)
		, m_pGameManager(0)
		, m_pPerception(0)
		, m_pPhysics(0)
		, m_pGlobalParameters(0)
		, m_pGameObjectParams(0)
		, m_pBBGlobal(0)
		, m_pBBCommon(0)
	{
	}

	// IObject

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		SERIALIZEIOBJPTR(m_pOwner);
	}

	virtual void Init( bool isFirstInit )
	{
		m_pGameManager = (IGameManager*)IObjectUtils::GetUniqueInterface( "GameManager", IID_IGAMEMANAGER );
		m_pPerception = (IPerceptionManager*)IObjectUtils::GetUniqueInterface( "PerceptionManager", IID_IPERCEPTIONMANAGER );
		m_pPhysics = (IPhysicsManager*)IObjectUtils::GetUniqueInterface( "PhysicsManager", IID_IPHYSICSMANAGER );
		m_pGlobalParameters = m_pGameManager->GetGlobalParameters();
		
		if (m_pOwner)
		{
			if (m_pOwner->GetBehavior() != this)
			{
				// We've become orphaned, so delete ourselves
				delete this;
			}
			else
			{
				InitGameObjectSpecificPointers();
			}
		}
	}

	// ~IObject

	// IBehavior

	virtual void StartBehavior() {}
	virtual void EndBehavior() {}

	virtual void Update( float timeDelta )
	{
		UpdatePerception(timeDelta);
		UpdateBlackboards(timeDelta);
		UpdateBehavior(timeDelta);

		AUVec3f desiredPosition = CalculateDesiredPosition(timeDelta);
		UpdatePosition(desiredPosition, timeDelta);
	}

	virtual AUVec3f CalculateDesiredPosition( float timeDelta )
	{
		return m_pOwner->GetEntity()->GetPosition();
	}

	virtual void OnCollision( IGameObject* pCollider )
	{
		if (pCollider->GetGameTeam() != m_pOwner->GetGameTeam())
		{
			m_pBBCommon->enemy_collision_objectid = pCollider->GetObjectId();
		}
	}

	virtual float ReceiveDamage( float fAmount )
	{
		m_pBBCommon->current_health -= fAmount;
		if ( m_pBBCommon->current_health <= 0.0f )
		{
			m_pGameManager->DestroyGameObject( m_pOwner->GetObjectId() );
			return 0.0f;
		}

		return m_pBBCommon->current_health;
	}

	virtual void SetGameObject( IGameObject* pOwner )
	{
		m_pOwner = pOwner;

		if (m_pOwner)
		{
			InitGameObjectSpecificPointers();
		}
	}

	// ~IBehavior


protected:

	virtual void InitGameObjectSpecificPointers()
	{
		IBlackboardManager* m_pBBManager = (IBlackboardManager*)IObjectUtils::GetUniqueInterface( "BlackboardManager", IID_IBLACKBOARDMANAGER );
		m_pBBGlobal = (BB_Global*)m_pBBManager->GetBlackboardGlobal();
		m_pBBCommon = (BB_Individual_Common*)m_pBBManager->GetBlackboardIndividualCommon( m_pOwner );

		m_pGameObjectParams = &(m_pGlobalParameters->go[m_pOwner->GetGameObjectType()]);
	}

	void UpdatePosition( const AUVec3f& desiredPosition, float timeDelta )
	{
		AUVec3f currentPosition = m_pOwner->GetEntity()->GetPosition();
		AUVec3f dir = desiredPosition - currentPosition;
		float dist = dir.Magnitude();
		if (dist > 0.0f)
		{
			dir /= dist;
		}
		
		dist = std::min( m_pBBCommon->max_speed * timeDelta, dist );

		AUVec3f requestPos = currentPosition + dir * dist;
		m_pPhysics->RequestPositionUpdate( m_pOwner, requestPos, timeDelta );

		const AUVec3f& newPosition = m_pOwner->GetEntity()->GetPosition();
		m_pBBCommon->current_position = newPosition;
		m_pBBCommon->current_velocity = (newPosition - currentPosition) / timeDelta;
	}

	void GetClosestTarget( const AUDynArray<ObjectId>& list, const AUVec3f& refPos, float& dist, IGameObject** pTarget )
	{
		for (size_t i=0; i<list.Size(); ++i)
		{
			IGameObject* pTest = 0;
			IObjectUtils::GetObject( &pTest, list[i] );
			if (pTest)
			{
				float testDist = (refPos - pTest->GetEntity()->GetPosition()).MagnitudeSqr();
				if (testDist < dist)
				{
					dist = testDist;
					*pTarget = pTest;
				}
			}
		}
	}


	// Protected Members

	IGameObject* m_pOwner;
	IGameManager* m_pGameManager;
	IPerceptionManager* m_pPerception;
	IPhysicsManager* m_pPhysics;
	GlobalParameters* m_pGlobalParameters;
	GameObjectParams* m_pGameObjectParams;

	BB_Global* m_pBBGlobal;
	BB_Individual_Common* m_pBBCommon;
};

#endif // BEHAVIORCOMMON_INCLUDED


