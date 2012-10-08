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

#include "Behavior_Virus_Base.h"
#include <float.h>

class Behavior_Virus_HuntRBC : public Behavior_Virus_Base
{
public:

	Behavior_Virus_HuntRBC() 
		: m_pApproachTarget(0)
		, m_pAvoidTarget(0)
		, m_TimeToNextDirChange(0.0f)
	{}

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		BehaviorCommon::Serialize(pSerializer);

		SERIALIZE(m_TimeToNextDirChange);
		SERIALIZE(m_PatrolDir);
		SERIALIZEIOBJPTR(m_pApproachTarget);
		SERIALIZEIOBJPTR(m_pAvoidTarget);
	}

	virtual void UpdateBehavior( float timeDelta )
	{
		// Always look for the closest possible target
		m_pApproachTarget = 0;
		if ( m_pBBIndividual->visible_rbc.Size() > 0 )
		{
			GetClosestApproachTarget();
		}

		m_pAvoidTarget = 0;
		if ( m_pBBIndividual->visible_wbc.Size() > 0 )
		{
			GetClosestAvoidTarget();
		}

		if (!m_pApproachTarget && !m_pAvoidTarget)
		{
			bool bBadHeading = m_pPhysics->IsHeadingToGameBounds( m_pBBCommon->current_position, m_pBBCommon->current_velocity );
			m_TimeToNextDirChange -= timeDelta;
			if ( bBadHeading || m_TimeToNextDirChange <= 0.0f )
			{
				m_TimeToNextDirChange = m_pBBIndividual->patrol_change_frequency;
				m_PatrolDir = AUVec3f((float)rand()/RAND_MAX - 0.5f, 0.0f, (float)rand()/RAND_MAX - 0.5f).GetNormalised();
			}
		}
	}

	virtual AUVec3f CalculateDesiredPosition( float timeDelta )
	{
		AUVec3f pos = m_pOwner->GetEntity()->GetPosition();
		AUVec3f targetVec;
		if (m_pAvoidTarget)
		{
			targetVec = pos -m_pAvoidTarget->GetEntity()->GetPosition();
		}
		else if (m_pApproachTarget)
		{
			targetVec = m_pApproachTarget->GetEntity()->GetPosition() - pos;
		}
		else
		{
			targetVec = m_PatrolDir * m_pOwner->GetMaxSpeed();
		}

		AUVec3f targetDir = targetVec.GetNormalised();
		float dist = std::min( targetVec.Magnitude(), m_pOwner->GetMaxSpeed() * timeDelta );
		pos += targetDir * dist;

		return pos;
	}

	void GetClosestApproachTarget()
	{
		float dist = FLT_MAX;
		const AUVec3f& refPos = m_pOwner->GetEntity()->GetPosition();

		GetClosestTarget( m_pBBIndividual->visible_rbc, refPos, dist, &m_pApproachTarget );
	}

	void GetClosestAvoidTarget()
	{
		float dist = FLT_MAX;
		const AUVec3f& refPos = m_pOwner->GetEntity()->GetPosition();

		GetClosestTarget( m_pBBIndividual->visible_wbc, refPos, dist, &m_pAvoidTarget );

		if ( m_pAvoidTarget && m_pAvoidTarget->GetThreatRating() < m_pOwner->GetThreatRating() )
		{
			m_pAvoidTarget = 0;
		}
	}


private:
	float m_TimeToNextDirChange;
	AUVec3f m_PatrolDir;
	IGameObject* m_pApproachTarget;
	IGameObject* m_pAvoidTarget;
};

REGISTERCLASS(Behavior_Virus_HuntRBC);
