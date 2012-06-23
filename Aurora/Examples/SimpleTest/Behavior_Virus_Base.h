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

#ifndef BEHAVIOR_VIRUS_BASE_INCLUDED
#define BEHAVIOR_VIRUS_BASE_INCLUDED

#include "../../RuntimeObjectSystem/RuntimeInclude.h"
RUNTIME_MODIFIABLE_INCLUDE; //adds this include to runtime tracking

#include "BehaviorCommon.h"

#include "BB_Team_Infection.h"
#include "BB_Group_Virus.h"
#include "BB_Individual_Virus.h"


class Behavior_Virus_Base : public BehaviorCommon
{
public:
	Behavior_Virus_Base() 
		: m_pBBTeam(0)
		, m_pBBGroup(0)
		, m_pBBIndividual(0)
	{
	}

	virtual void UpdatePerception( float timeDelta )
	{
		m_pPerception->GetPerceived( m_pOwner, EGO_WBC, m_pBBIndividual->visible_wbc );
		m_pPerception->GetPerceived( m_pOwner, EGO_RBC, m_pBBIndividual->visible_rbc );
	}

	virtual void UpdateBlackboards( float timeDelta )
	{
		// TODO
	}


protected:

	virtual void InitGameObjectSpecificPointers()
	{
		BehaviorCommon::InitGameObjectSpecificPointers();

		IBlackboardManager* m_pBBManager = (IBlackboardManager*)IObjectUtils::GetUniqueInterface( "BlackboardManager", IID_IBLACKBOARDMANAGER );
		m_pBBTeam = (BB_Team_Infection*)m_pBBManager->GetBlackboardTeam( m_pOwner->GetGameTeam() );
		m_pBBGroup = (BB_Group_Virus*)m_pBBManager->GetBlackboardGroup( m_pOwner->GetGameObjectType() );
		m_pBBIndividual = (BB_Individual_Virus*)m_pBBManager->GetBlackboardIndividual( m_pOwner );
	}

	// Private Members

	BB_Team_Infection* m_pBBTeam;
	BB_Group_Virus* m_pBBGroup;
	BB_Individual_Virus* m_pBBIndividual;
};

#endif // BEHAVIOR_VIRUS_BASE_INCLUDED


