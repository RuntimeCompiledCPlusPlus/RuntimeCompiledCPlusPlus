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

#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../Systems/SystemTable.h"
#include "../../RuntimeObjectSystem/IObjectFactorySystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"

#include "../../RuntimeObjectSystem/IObject.h"
#include "IObjectUtils.h"
#include "IGameManager.h"


class MainObject: public IObject
{
public:
	
	MainObject()
	{
	}

	~MainObject()
	{
		if (!IsRuntimeDelete())
		{
			// Delete all created objects
			delete IObjectUtils::GetUniqueObject( "BlackboardManager" );
			delete IObjectUtils::GetUniqueObject( "BehaviorTreeManager" );
			delete IObjectUtils::GetUniqueObject( "PerceptionManager" );
			delete IObjectUtils::GetUniqueObject( "PhysicsManager" );
			delete IObjectUtils::GetUniqueObject( "InputManager" );
			delete IObjectUtils::GetUniqueObject( "LightingControl" );
			delete IObjectUtils::GetUniqueObject( "CameraControl" );
			delete IObjectUtils::GetUniqueObject( "FPSCounter" );
			delete IObjectUtils::GetUniqueObject( "CompilingNotification" );
			delete IObjectUtils::GetUniqueObject( "GameManager" );
			delete IObjectUtils::GetUniqueObject( "MainMenu" );
		}
	}
	
	virtual void Serialize(ISimpleSerializer *pSerializer) 
	{
	}

	virtual void Init( bool isFirstInit )
	{
		if (isFirstInit)
		{
			CreateObjects();
		}
	}


private:

	void CreateObjects()
	{
		// Construct unique objects (managers, util objects, etc)
		IObjectUtils::CreateUniqueObject( "MainMenu" );
		IObjectUtils::CreateUniqueObjectAndEntity( "CompilingNotification", "CompilingNotification" );

		
		IObjectUtils::CreateUniqueObjectAndEntity( "GameManager", "GameManager" );
		IObjectUtils::CreateUniqueObjectAndEntity( "FPSCounter", "FPSCounter" );
		IObjectUtils::CreateUniqueObjectAndEntity( "CameraControl", "CameraControl" );
		IObjectUtils::CreateUniqueObjectAndEntity( "LightingControl", "LightingControl" );
		IObjectUtils::CreateUniqueObjectAndEntity( "InputManager", "InputManager" );
		IObjectUtils::CreateUniqueObjectAndEntity( "PhysicsManager", "PhysicsManager" );
		IObjectUtils::CreateUniqueObjectAndEntity( "PerceptionManager", "PerceptionManager" );
		IObjectUtils::CreateUniqueObjectAndEntity( "BehaviorTreeManager", "BehaviorTreeManager" );
		IObjectUtils::CreateUniqueObjectAndEntity( "BlackboardManager", "BlackboardManager" );

		PerModuleInterface::g_pSystemTable->pLogSystem->Log(eLV_COMMENTS, "Created Objects\n");
	}
};

REGISTERSINGLETON(MainObject, true);