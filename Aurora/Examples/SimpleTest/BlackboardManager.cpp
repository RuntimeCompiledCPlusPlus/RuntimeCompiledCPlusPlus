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

#include "IBlackboardManager.h"
#include "IGameObject.h"
#include "IGameManager.h"
#include "IObjectUtils.h"
#include "GlobalParameters.h"
#include "IObjectUtils.h"

#include "BB_Individual_WBC.h"
#include "BB_Individual_RBC.h"
#include "BB_Individual_Virus.h"
#include "BB_Individual_Infected.h"
#include "BB_Individual_Common.h"
#include "BB_Team_Immune.h"
#include "BB_Team_Infection.h"
#include "BB_Group_WBC.h"
#include "BB_Group_RBC.h"
#include "BB_Group_Virus.h"
#include "BB_Group_Infected.h"
#include "BB_Global.h"

#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../Systems/SystemTable.h"
#include "../../RuntimeObjectSystem/IObjectFactorySystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"

#include <assert.h>
#include <map>
#include <vector>


class BlackboardManager: public IBlackboardManager, public IGameEventListener
{
public:
	BlackboardManager() 
	{
		m_pGameManager = 0;
		m_pGlobalParameters = 0;
		m_pBlackboardGlobal = 0;
		m_BlackboardTeam.resize(EGT_COUNT);
		m_BlackboardGroup.resize(EGO_COUNT);
	}

	virtual ~BlackboardManager()
	{
		((IGameManager*)IObjectUtils::GetUniqueInterface( "GameManager", IID_IGAMEMANAGER ))->RemoveListener(this);

		if (!IsRuntimeDelete())
		{
			DestroyAllBlackboards();
		}
	}

	// IObject

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		SERIALIZEIOBJPTR(m_pBlackboardGlobal);
		SERIALIZE(m_BlackboardTeam);
		SERIALIZE(m_BlackboardGroup);
		SERIALIZE(m_BlackboardIndividual);
		SERIALIZE(m_BlackboardIndividualCommon);
	}

	virtual void Init( bool isFirstInit )
	{
		IGameManager* pGameManager = (IGameManager*)IObjectUtils::GetUniqueInterface( "GameManager", IID_IGAMEMANAGER );
		pGameManager->AddListener(this);
		m_pGlobalParameters = pGameManager->GetGlobalParameters();

		if (isFirstInit)
		{
			CreateSharedBlackboards();
		}
	}

	// ~IObject

	// IGameEventListener

	virtual void OnGameObjectCreated( IGameObject* pGameObject )
	{
		EGameTeam team = pGameObject->GetGameTeam();
		EGameObject type = pGameObject->GetGameObjectType();

		CreateIndividualBlackboards(pGameObject);

		// Set/Modify appropriate blackboard values
		SetInitialCommonValues( pGameObject );
		ModifyGlobalTeamCount( team, 1 );
		ModifyTeamCount( team, 1 );
		ModifyGroupCount( type, 1 );
	}

	virtual void OnGameObjectAboutToDestroy( IGameObject* pGameObject )
	{
		EGameTeam team = pGameObject->GetGameTeam();
		EGameObject type = pGameObject->GetGameObjectType();

		ModifyGlobalTeamCount( team, -1 );
		ModifyTeamCount( team, -1 );	
		ModifyGroupCount( type, -1 );

		DestroyBlackboardIndividual(pGameObject->GetObjectId());
		DestroyBlackboardIndividualCommon(pGameObject->GetObjectId());
	}

	// ~IGameEventListener

	// IBlackboardManager

	virtual IBlackboard* GetBlackboardGlobal() const
	{
		return m_pBlackboardGlobal;
	}

	virtual IBlackboard* GetBlackboardTeam( EGameTeam team ) const
	{
		IBlackboard* pBlackboard = 0;
		
		if (m_BlackboardTeam[team].IsValid())
		{
			IObjectUtils::GetObject( &pBlackboard, m_BlackboardTeam[team] );
		}

		return pBlackboard;
	}

	virtual IBlackboard* GetBlackboardGroup( EGameObject type ) const
	{
		IBlackboard* pBlackboard = 0;
		
		if (m_BlackboardGroup[type].IsValid())
		{
			IObjectUtils::GetObject( &pBlackboard, m_BlackboardGroup[type] );
		}
		
		return pBlackboard;
	}

	virtual IBlackboard* GetBlackboardIndividual( const IGameObject* pGameObject ) const
	{
		IBlackboard* pBlackboard = 0;
		
		TIndividualMap::const_iterator it = m_BlackboardIndividual.find( pGameObject->GetObjectId() );
		if (it != m_BlackboardIndividual.end())
		{
			IObjectUtils::GetObject( &pBlackboard, it->second );
		}

		return pBlackboard;
	}

	virtual IBlackboard* GetBlackboardIndividualCommon( const IGameObject* pGameObject ) const
	{
		IBlackboard* pBlackboard = 0;

		TIndividualMap::const_iterator it = m_BlackboardIndividualCommon.find( pGameObject->GetObjectId() );
		if (it != m_BlackboardIndividualCommon.end())
		{
			IObjectUtils::GetObject( &pBlackboard, it->second );
		}

		return pBlackboard;
	}

	virtual void ResetBlackboards()
	{
		DestroyAllBlackboards();
		CreateSharedBlackboards();
	}

	// ~IBlackboardManager


private:

	void CreateSharedBlackboards()
	{
		// Global
		AU_ASSERT(!m_pBlackboardGlobal);
		IObjectUtils::CreateObject( &m_pBlackboardGlobal, "BB_Global" );

		// Team
		m_BlackboardTeam[EGT_IMMUNE] = IObjectUtils::CreateObject( "BB_Team_Immune" )->GetObjectId();
		m_BlackboardTeam[EGT_INFECTION] = IObjectUtils::CreateObject( "BB_Team_Infection" )->GetObjectId();
		
		// Group
		m_BlackboardGroup[EGO_WBC] = IObjectUtils::CreateObject( "BB_Group_WBC" )->GetObjectId();
		m_BlackboardGroup[EGO_RBC] = IObjectUtils::CreateObject( "BB_Group_RBC" )->GetObjectId();
		m_BlackboardGroup[EGO_VIRUS] = IObjectUtils::CreateObject( "BB_Group_Virus" )->GetObjectId();
		m_BlackboardGroup[EGO_INFECTED] = IObjectUtils::CreateObject( "BB_Group_Infected" )->GetObjectId();
	}

	void CreateIndividualBlackboards( IGameObject* pGameObject )
	{
		ObjectId id = pGameObject->GetObjectId();

		// Individual
		TIndividualMap::iterator it = m_BlackboardIndividual.find(id);
		AU_ASSERT(it == m_BlackboardIndividual.end());
		if (it == m_BlackboardIndividual.end())
		{
			const char* text = 0;
			switch (pGameObject->GetGameObjectType())
			{
			case EGO_WBC: text = "BB_Individual_WBC"; break;
			case EGO_RBC: text = "BB_Individual_RBC"; break;
			case EGO_VIRUS: text = "BB_Individual_Virus"; break;
			case EGO_INFECTED: text = "BB_Individual_Infected"; break;
            default: AU_ASSERT(false);
			}

			m_BlackboardIndividual[id] = IObjectUtils::CreateObject( text )->GetObjectId();
		}

		// IndividualCommon
		it = m_BlackboardIndividualCommon.find(id);
		AU_ASSERT(it == m_BlackboardIndividualCommon.end());
		if (it == m_BlackboardIndividualCommon.end())
		{
			m_BlackboardIndividualCommon[id] = IObjectUtils::CreateObject( "BB_Individual_Common" )->GetObjectId();
		}
	}

	void DestroyBlackboardIndividual( ObjectId id )
	{
		TIndividualMap::iterator it = m_BlackboardIndividual.find(id);
		if (it != m_BlackboardIndividual.end())
		{
			IObjectFactorySystem* pFactory = PerModuleInterface::g_pSystemTable->pObjectFactorySystem;
			IObject* pObj = pFactory->GetObject(it->second);
			delete pObj;

			m_BlackboardIndividual.erase(it);
		}
	}

	void DestroyBlackboardIndividualCommon( ObjectId id )
	{
		TIndividualMap::iterator it = m_BlackboardIndividualCommon.find(id);
		if (it != m_BlackboardIndividualCommon.end())
		{
			IObjectFactorySystem* pFactory = PerModuleInterface::g_pSystemTable->pObjectFactorySystem;
			IObject* pObj = pFactory->GetObject(it->second);
			delete pObj;

			m_BlackboardIndividualCommon.erase(it);
		}
	}

	void DestroyAllBlackboards()
	{
		IObjectFactorySystem* pFactory = PerModuleInterface::g_pSystemTable->pObjectFactorySystem;
		IObject* pObj = 0;

		delete m_pBlackboardGlobal;
		m_pBlackboardGlobal = 0;

		for (int i=0; i<EGT_COUNT; ++i)
		{
			pObj = pFactory->GetObject(m_BlackboardTeam[i]);
			delete pObj;
			m_BlackboardTeam[i] = ObjectId();
		}

		for (int i=0; i<EGO_COUNT; ++i)
		{	
			pObj = pFactory->GetObject(m_BlackboardGroup[i]);
			delete pObj;
			m_BlackboardGroup[i] = ObjectId();
		}

		TIndividualMap::iterator it = m_BlackboardIndividual.begin();
		TIndividualMap::iterator itEnd = m_BlackboardIndividual.end();
		while (it != itEnd)
		{
			pObj = pFactory->GetObject(it->second);
			delete pObj;
			++it;
		}
		m_BlackboardIndividual.clear();

		it = m_BlackboardIndividualCommon.begin();
		itEnd = m_BlackboardIndividualCommon.end();
		while (it != itEnd)
		{
			pObj = pFactory->GetObject(it->second);
			delete pObj;
			++it;
		}
		m_BlackboardIndividualCommon.clear();
	}

	void ModifyTeamCount( EGameTeam team, int delta )
	{
		IBlackboard* pTeam = GetBlackboardTeam(team);
		
		switch (team)
		{
		case EGT_IMMUNE: ((BB_Team_Immune*)pTeam)->team_size += delta; break;
		case EGT_INFECTION: ((BB_Team_Infection*)pTeam)->team_size += delta; break;
        default: AU_ASSERT(false);
		}
	}

	void ModifyGroupCount( EGameObject type, int delta )
	{
		IBlackboard* pGroup = GetBlackboardGroup(type);

		switch (type)
		{
		case EGO_WBC: ((BB_Group_WBC*)pGroup)->group_size += delta; break;
		case EGO_RBC: ((BB_Group_RBC*)pGroup)->group_size += delta; break;
		case EGO_VIRUS: ((BB_Group_Virus*)pGroup)->group_size += delta; break;
		case EGO_INFECTED: ((BB_Group_Infected*)pGroup)->group_size += delta; break;
        default: AU_ASSERT(false);
		}
	}

	void ModifyGlobalTeamCount( EGameTeam team, int delta )
	{
		BB_Global* pBBGlobal = (BB_Global*)GetBlackboardGlobal();
		if (team == EGT_IMMUNE)
		{
			pBBGlobal->immune_count += delta;
		}
		else
		{
			pBBGlobal->infection_count += delta;
		}
	}

	void SetInitialCommonValues( IGameObject* pGameObject )
	{
		BB_Individual_Common* pCommon = (BB_Individual_Common*)GetBlackboardIndividualCommon( pGameObject );
		GameObjectParams& params = m_pGlobalParameters->go[pGameObject->GetGameObjectType()];
		pCommon->current_health = params.initial_health;
		pCommon->max_speed = params.max_speed;
		pCommon->current_position = pGameObject->GetEntity()->GetPosition();
	}

	// Private Members

	typedef std::map<ObjectId, ObjectId> TIndividualMap;
	typedef std::vector<ObjectId> TObjectList;
	
	IGameManager* m_pGameManager;
	GlobalParameters* m_pGlobalParameters;
	IBlackboard* m_pBlackboardGlobal;
	TObjectList m_BlackboardTeam;
	TObjectList m_BlackboardGroup;
	TIndividualMap m_BlackboardIndividual;
	TIndividualMap m_BlackboardIndividualCommon;
};

REGISTERCLASS(BlackboardManager);

// Register all blackboard classes here, since they're defined in headers
// And we get a link error if they get registered in more than one place

REGISTERCLASS(BB_Global);
REGISTERCLASS(BB_Team_Infection);
REGISTERCLASS(BB_Team_Immune);
REGISTERCLASS(BB_Group_WBC);
REGISTERCLASS(BB_Group_Virus);
REGISTERCLASS(BB_Group_RBC);
REGISTERCLASS(BB_Group_Infected);
REGISTERCLASS(BB_Individual_Common);
REGISTERCLASS(BB_Individual_WBC);
REGISTERCLASS(BB_Individual_RBC);
REGISTERCLASS(BB_Individual_Virus);
REGISTERCLASS(BB_Individual_Infected);

