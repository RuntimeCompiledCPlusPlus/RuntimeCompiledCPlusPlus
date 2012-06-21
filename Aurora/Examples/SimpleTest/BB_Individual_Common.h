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

#ifndef BB_INDIVIDUAL_COMMON_INCLUDED
#define BB_INDIVIDUAL_COMMON_INCLUDED

#include "../../RuntimeObjectSystem/RuntimeInclude.h"
RUNTIME_MODIFIABLE_INCLUDE; //adds this include to runtime tracking

#include "IBlackboard.h"
#include "../../Common/AUVec3f.inl"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"
#include <assert.h>


struct BB_Individual_Common : public IBlackboard
{
	BB_Individual_Common()
	{
		current_health = 0;
		max_speed = 0;
		target_position.SetInfinite();
		time_to_next_attack = 0.0f;
	}

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		AU_ASSERT(pSerializer);

		SERIALIZE(current_health);
		SERIALIZE(max_speed);
		SERIALIZE(current_velocity);
		SERIALIZE(current_position);
		SERIALIZE(target_position);
		SERIALIZE(enemy_collision_objectid);
		SERIALIZE(time_to_next_attack);
	}

	// Members

	float current_health;
	float max_speed;
	AUVec3f current_velocity;
	AUVec3f current_position;
	AUVec3f target_position;         // Infinity if none
	ObjectId enemy_collision_objectid;   // 0 if none
	float time_to_next_attack;
};

// Registered inside BlackboardManager.cpp
// REGISTERCLASS(BB_Individual_Common);

#endif // BB_INDIVIDUAL_COMMON_INCLUDED