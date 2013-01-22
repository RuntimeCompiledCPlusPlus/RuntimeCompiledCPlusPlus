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
#include "BB_Team_Infection.h"
#include "BB_Group_Virus.h"
#include "BB_Individual_Virus.h"
#include "BB_Individual_Common.h"

#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/IAssetSystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"

#include <assert.h>


class BehaviorTree_Virus : public IBehaviorTree
{
public:

	virtual void Execute( IGameObject* pGameObject )
	{
		BB_Global* pBBGlobal = (BB_Global*)m_pBBManager->GetBlackboardGlobal();
		BB_Team_Infection* pBBTeam = (BB_Team_Infection*)m_pBBManager->GetBlackboardTeam( pGameObject->GetGameTeam() );
		BB_Group_Virus* pBBGroup = (BB_Group_Virus*)m_pBBManager->GetBlackboardGroup( pGameObject->GetGameObjectType() );
		BB_Individual_Virus* pBBIndividual = (BB_Individual_Virus*)m_pBBManager->GetBlackboardIndividual( pGameObject );
		BB_Individual_Common* pBBCommon = (BB_Individual_Common*)m_pBBManager->GetBlackboardIndividualCommon( pGameObject );

		if ( pBBCommon->enemy_collision_objectid.IsValid() )
		{
			pGameObject->SetBehavior( m_Behavior_Virus_Combat );
		}
		else if ( !pBBCommon->target_position.IsInfinite() )
		{
			pGameObject->SetBehavior( m_Behavior_Virus_Approach );
		}
		else if ( pBBGlobal->infection_team_strength > pBBGlobal->immune_team_strength * 1.2f && pBBGlobal->infection_count > 3 )
		{
			pGameObject->SetBehavior( m_Behavior_Virus_HuntWBC );
		}
		else if ( pBBGlobal->immune_count > 0 )
		{
			pGameObject->SetBehavior( m_Behavior_Virus_HuntRBC );
		}
		else
		{
			pGameObject->SetBehavior( m_Behavior_Virus_Idle );
		}	
	}

	virtual void Init( bool isFirstInit )
	{
		m_pBBManager = (IBlackboardManager*)IObjectUtils::GetUniqueInterface( "BlackboardManager", IID_IBLACKBOARDMANAGER );
		m_Behavior_Virus_Combat		= IObjectUtils::GetConstructorId( "Behavior_Virus_Combat" );
		m_Behavior_Virus_Approach	= IObjectUtils::GetConstructorId( "Behavior_Virus_Approach" );
		m_Behavior_Virus_HuntWBC	= IObjectUtils::GetConstructorId( "Behavior_Virus_HuntWBC" );
		m_Behavior_Virus_HuntRBC	= IObjectUtils::GetConstructorId( "Behavior_Virus_HuntRBC" );
		m_Behavior_Virus_Idle		= IObjectUtils::GetConstructorId( "Behavior_Virus_Idle" );
	}

private:

	IBlackboardManager* m_pBBManager;
	ConstructorId		m_Behavior_Virus_Combat;
	ConstructorId		m_Behavior_Virus_Approach;
	ConstructorId		m_Behavior_Virus_HuntWBC;
	ConstructorId		m_Behavior_Virus_HuntRBC;
	ConstructorId		m_Behavior_Virus_Idle;
};

REGISTERCLASS(BehaviorTree_Virus);
