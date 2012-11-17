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

#include "IBehaviorTree.h"
#include "IGameObject.h"
#include "IBlackboard.h"
#include "IBlackboardManager.h"
#include "IObjectUtils.h"

#include "BB_Global.h"
#include "BB_Team_Immune.h"
#include "BB_Group_RBC.h"
#include "BB_Individual_RBC.h"
#include "BB_Individual_Common.h"

#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/IAssetSystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"

#include <assert.h>


class BehaviorTree_RBC : public IBehaviorTree
{
public:

	virtual void Execute( IGameObject* pGameObject )
	{
		BB_Individual_RBC* pBBIndividual = (BB_Individual_RBC*)m_pBBManager->GetBlackboardIndividual( pGameObject );
		BB_Individual_Common* pBBCommon = (BB_Individual_Common*)m_pBBManager->GetBlackboardIndividualCommon( pGameObject );

		if ( pBBCommon->enemy_collision_objectid.IsValid() )
		{
			pGameObject->SetBehavior( m_Behavior_RBC_Combat );
		}
		else if ( !pBBCommon->target_position.IsInfinite() )
		{
			pGameObject->SetBehavior( m_Behavior_RBC_Approach );
		}
		/* Demo [Tutorial03]
		else if ( pBBIndividual->visible_dangerous.Size() > 0 )
		{
			pGameObject->SetBehavior( m_Behavior_RBC_Evade );
		} 
		//*/
		else
		{
			pGameObject->SetBehavior( m_Behavior_RBC_Idle );
		}	
	}

	virtual void Init( bool isFirstInit )
	{
		m_pBBManager = (IBlackboardManager*)IObjectUtils::GetUniqueInterface( "BlackboardManager", IID_IBLACKBOARDMANAGER );
		m_Behavior_RBC_Combat	= IObjectUtils::GetConstructorId( "Behavior_RBC_Combat" );
		m_Behavior_RBC_Approach	= IObjectUtils::GetConstructorId( "Behavior_RBC_Approach" );
		// m_Behavior_RBC_Evade		= IObjectUtils::GetConstructorId( "Behavior_RBC_Evade" ); /// Demo [Tutorial03]
		m_Behavior_RBC_Idle		= IObjectUtils::GetConstructorId( "Behavior_RBC_Idle" );
	}

private:

	IBlackboardManager* m_pBBManager;
	ConstructorId		m_Behavior_RBC_Combat;
	ConstructorId		m_Behavior_RBC_Approach;
	// ConstructorId		m_Behavior_RBC_Evade; /// Demo [Tutorial03] 
	ConstructorId		m_Behavior_RBC_Idle;
};

REGISTERCLASS(BehaviorTree_RBC);