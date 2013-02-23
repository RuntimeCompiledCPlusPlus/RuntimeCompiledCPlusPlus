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
#include <float.h>

/* Demo [Tutorial03]
class Behavior_RBC_Evade : public Behavior_RBC_Base
{
public:

	virtual void StartBehavior()
	{
	}

	virtual void EndBehavior()
	{
	}

	virtual void UpdateBehavior( float timeDelta )
	{
	}

	virtual AUVec3f CalculateDesiredPosition( float timeDelta )
	{
		AUVec3f pos = m_pOwner->GetEntity()->GetPosition();
		float dist = FLT_MAX;		
		IGameObject* pGameObject = 0;

		GetClosestTarget( m_pBBIndividual->visible_dangerous, pos, dist, &pGameObject );

		if (pGameObject)
		{
			AUVec3f targetVec = pGameObject->GetEntity()->GetPosition() - m_pBBCommon->current_position;
			AUVec3f targetDir = targetVec.GetNormalised() * -1.0f;
			float dist = std::min( targetVec.Magnitude(), m_pOwner->GetMaxSpeed() * timeDelta );
			pos += targetDir * dist;
		}

		return pos;
	}
};

REGISTERCLASS(Behavior_RBC_Evade);

//*/











class CDummy : public IObject
{
	virtual void Serialize(ISimpleSerializer *pSerializer) {}
};

REGISTERCLASS(CDummy);

