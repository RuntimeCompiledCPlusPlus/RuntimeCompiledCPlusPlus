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

#include "Behavior_RBC_Base.h"


class Behavior_RBC_Combat : public Behavior_RBC_Base
{
public:

	virtual void StartBehavior()
	{
		m_pEntitySystem = PerModuleInterface::g_pSystemTable->pEntitySystem;

		m_pBBCommon->time_to_next_attack = m_pGameObjectParams->attack_speed;
	}

	virtual void EndBehavior()
	{
		m_pBBCommon->enemy_collision_objectid.SetInvalid();
		m_pBBCommon->time_to_next_attack = 0.0f;
	}

	virtual void UpdateBehavior( float timeDelta )
	{
		IGameObject* pGameObject = 0;
		IObjectUtils::GetObject( &pGameObject, m_pBBCommon->enemy_collision_objectid );

		m_pBBCommon->time_to_next_attack -= timeDelta;
		if ( m_pBBCommon->time_to_next_attack <= 0.0f )
		{
			m_pBBCommon->time_to_next_attack += m_pGameObjectParams->attack_speed;

			if (pGameObject && pGameObject->GetBehavior())
			{
				float enemyHealth = pGameObject->GetBehavior()->ReceiveDamage( m_pGameObjectParams->attack_damage );
				if (enemyHealth <= 0.0f)
				{
					pGameObject = 0;
				}
			}
		}

		if (!pGameObject)
		{
			m_pBBCommon->enemy_collision_objectid.SetInvalid();
		}
	}

	virtual AUVec3f CalculateDesiredPosition( float timeDelta )
	{
		AUVec3f pos = m_pOwner->GetEntity()->GetPosition();

		IGameObject* pGameObject = 0;
		IObjectUtils::GetObject( &pGameObject, m_pBBCommon->enemy_collision_objectid );
		if (pGameObject)
		{
			AUVec3f targetVec = pGameObject->GetEntity()->GetPosition() - m_pBBCommon->current_position;
			AUVec3f targetDir = targetVec.GetNormalised();
			float dist = std::min( targetVec.Magnitude(), m_pOwner->GetMaxSpeed() * timeDelta );
			pos += targetDir * dist;
		}

		return pos;
	}

private:
	IEntitySystem* m_pEntitySystem;
};

REGISTERCLASS(Behavior_RBC_Combat);
