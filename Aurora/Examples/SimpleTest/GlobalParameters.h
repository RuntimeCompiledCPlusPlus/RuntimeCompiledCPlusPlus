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

#ifndef GLOBALPARAMETERS_INCLUDED
#define GLOBALPARAMETERS_INCLUDED

//////////////////////////////////////////////////////////////////////////
// GlobalParameters contains configuration parameters for the game
// We can roll this into a CVar system when we have something like that
// Can be obtained via GameManager object
//
// NOTE: Currently doesn't support add/remove of members at runtime
//       Not sure exactly what would happen, but it's probably bad!
//////////////////////////////////////////////////////////////////////////

#include "../../RuntimeObjectSystem/RuntimeInclude.h"
RUNTIME_MODIFIABLE_INCLUDE; //adds this include to runtime tracking

#include "IGameManager.h"
#include "../../Common/AUColor.inl"
#include <string>

struct GameObjectParams
{
	EGameTeam team;
	int initial_count;
	int max_count; // Will stop spawning new ones if we reach this amount
	float spawn_rate;     // Number of seconds before a new one spawns
	
	float initial_health;
	float max_speed;
	float max_size;
	float min_size;
	float base_size_modifier; // used to handle model scale issues
	float collision_radius;

	float attack_damage;
	float attack_speed; // time between attacks

	AUColor color_normal;
	AUColor color_highlight;

	std::string behavior_tree;
	std::string model;
	std::string base_name;

	// Maximum distance in world units that object can perceive 
	// objects of the given type
	float perceptionDist[EGO_COUNT];
};


struct GlobalParameters
{
	// per object type parameters
	GameObjectParams go[EGO_COUNT];

	float infected_health_gain_from_eating;
	float infected_max_health; // will explode if goes above this
	int infected_num_viruses_on_explode;
	
	GlobalParameters()
	{
		go[EGO_WBC].team = EGT_IMMUNE;
		go[EGO_WBC].initial_count = 3;
		go[EGO_WBC].max_count = 10;
		go[EGO_WBC].spawn_rate = 13.0f;
		go[EGO_WBC].initial_health = 100;
		go[EGO_WBC].max_speed = 35.0f;
		go[EGO_WBC].max_size = 1.0f;
		go[EGO_WBC].min_size = 0.5f;
		go[EGO_WBC].base_size_modifier = 3.0f;
		go[EGO_WBC].collision_radius = 28.0f;
		go[EGO_WBC].attack_damage = 10.0f;
		go[EGO_WBC].attack_speed = 0.25f;
		go[EGO_WBC].color_normal = AUColor(1.0f, 1.0f, 1.0f, 1.0f);
		go[EGO_WBC].color_highlight = AUColor(0.6f, 0.6f, 0.6f, 1.0f);
		go[EGO_WBC].behavior_tree = "BehaviorTree_WBC";
		go[EGO_WBC].model = "white_blood_cell.aml";
		go[EGO_WBC].base_name = "wbc";

		go[EGO_RBC].team = EGT_IMMUNE;
		go[EGO_RBC].initial_count = 7;
		go[EGO_RBC].max_count = 20;
		go[EGO_RBC].spawn_rate = 4.5f;
		go[EGO_RBC].initial_health = 50;
		go[EGO_RBC].max_speed = 30.0f;        // A bit high, for presentation purposes
		go[EGO_RBC].max_size = 1.0f;
		go[EGO_RBC].min_size = 0.5f;
		go[EGO_RBC].base_size_modifier = 2.5f;
		go[EGO_RBC].collision_radius = 25.0f;
		go[EGO_RBC].attack_damage = 1.0f;
		go[EGO_RBC].attack_speed = 0.25f;
		go[EGO_RBC].color_normal = AUColor(1.0f, 0.0f, 0.0f, 1.0f);
		go[EGO_RBC].color_highlight = AUColor(1.0f, 0.5f, 0.5f, 1.0f);
		go[EGO_RBC].behavior_tree = "BehaviorTree_RBC";
		go[EGO_RBC].model = "red_blood_cell.aml";
		go[EGO_RBC].base_name = "rbc";

		go[EGO_VIRUS].team = EGT_INFECTION;
		go[EGO_VIRUS].initial_count = 3;
		go[EGO_VIRUS].max_count = 10;
		go[EGO_VIRUS].spawn_rate = 0.0f;
		go[EGO_VIRUS].initial_health = 40;
		go[EGO_VIRUS].max_speed = 50.0f;
		go[EGO_VIRUS].max_size = 1.0f;
		go[EGO_VIRUS].min_size = 0.5f;
		go[EGO_VIRUS].base_size_modifier = 4.5f;
		go[EGO_VIRUS].collision_radius = 15.0f;
		go[EGO_VIRUS].attack_damage = 5.0f;
		go[EGO_VIRUS].attack_speed = 0.25f;
		go[EGO_VIRUS].color_normal = AUColor(0.8f, 0.8f, 0.0f, 1.0f);
		go[EGO_VIRUS].color_highlight = AUColor(1.0f, 1.0f, 0.2f, 1.0f);
		go[EGO_VIRUS].behavior_tree = "BehaviorTree_Virus";
		go[EGO_VIRUS].model = "virus.aml";
		go[EGO_VIRUS].base_name = "virus";

		go[EGO_INFECTED].team = EGT_INFECTION;
		go[EGO_INFECTED].initial_count = 0;
		go[EGO_INFECTED].max_count = 10;
		go[EGO_INFECTED].spawn_rate = 0.0f;
		go[EGO_INFECTED].initial_health = 100;
		go[EGO_INFECTED].max_speed = 35.0f;
		go[EGO_INFECTED].max_size = 1.0f;
		go[EGO_INFECTED].min_size = 0.5f;
		go[EGO_INFECTED].base_size_modifier = 4.0f;
		go[EGO_INFECTED].collision_radius = 30.0f;
		go[EGO_INFECTED].attack_damage = 12.0f;
		go[EGO_INFECTED].attack_speed = 0.25f;
		go[EGO_INFECTED].color_normal = AUColor(0.4f, 0.7f, 0.3f, 1.0f);
		go[EGO_INFECTED].color_highlight = AUColor(0.6f, 0.9f, 0.5f, 1.0f);
		go[EGO_INFECTED].behavior_tree = "BehaviorTree_Infected";
		go[EGO_INFECTED].model = "infected_cell.aml";
		go[EGO_INFECTED].base_name = "infected";

		// all values for perception distances initialized together for easier comparison
		go[EGO_WBC].perceptionDist[EGO_WBC] = 1000.0f;
		go[EGO_WBC].perceptionDist[EGO_RBC] = 0.0f;
		go[EGO_WBC].perceptionDist[EGO_VIRUS] = 250.0f;
		go[EGO_WBC].perceptionDist[EGO_INFECTED] = 250.0f;
		go[EGO_RBC].perceptionDist[EGO_WBC] = 0.0f;
		go[EGO_RBC].perceptionDist[EGO_RBC] = 0.0f;
		go[EGO_RBC].perceptionDist[EGO_VIRUS] = 150.0f;         // Set for presentation purposes
		go[EGO_RBC].perceptionDist[EGO_INFECTED] = 250.0f;      // Set for presentation purposes
		go[EGO_VIRUS].perceptionDist[EGO_WBC] = 150.0f;
		go[EGO_VIRUS].perceptionDist[EGO_RBC] = 300.0f;
		go[EGO_VIRUS].perceptionDist[EGO_VIRUS] = 1000.0f;
		go[EGO_VIRUS].perceptionDist[EGO_INFECTED] = 1000.0f;
		go[EGO_INFECTED].perceptionDist[EGO_WBC] = 150.0f;
		go[EGO_INFECTED].perceptionDist[EGO_RBC] = 300.0f;
		go[EGO_INFECTED].perceptionDist[EGO_VIRUS] = 1000.0f;
		go[EGO_INFECTED].perceptionDist[EGO_INFECTED] = 1000.0f;

		infected_health_gain_from_eating = 60.0f;
		infected_max_health = 250.0f;
		infected_num_viruses_on_explode = 3;
	}
};


#endif // GLOBALPARAMETERS_INCLUDED