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

#ifndef BB_GLOBAL_INCLUDED
#define BB_GLOBAL_INCLUDED

#include "../../RuntimeObjectSystem/RuntimeInclude.h"
RUNTIME_MODIFIABLE_INCLUDE; //adds this include to runtime tracking

#include "IBlackboard.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"
#include <assert.h>

struct BB_Global : public IBlackboard
{
	BB_Global()
	{
		gameTimeElapsed = 0.0f;
		immune_count = 0;
		infection_count = 0;
		immune_team_strength = 0;
		infection_team_strength = 0;
	}

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		AU_ASSERT(pSerializer);
		SERIALIZE(gameTimeElapsed);
		SERIALIZE(immune_count);
		SERIALIZE(infection_count);
		SERIALIZE(immune_team_strength);
		SERIALIZE(infection_team_strength);
	}

	// Members

	float gameTimeElapsed;
	int immune_count;
	int infection_count;
	float immune_team_strength;
	float infection_team_strength;
};

// Registered inside BlackboardManager.cpp
// REGISTERCLASS(BB_Global);

#endif // BB_GLOBAL_INCLUDED