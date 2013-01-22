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

#ifndef CONSOLECONTEXT_INCLUDED
#define CONSOLECONTEXT_INCLUDED

#include "../../RuntimeObjectSystem/IObject.h"
#include "IObjectUtils.h"
#include "IConsoleContext.h"
#include "IGameObject.h"
#include "ILightingControl.h"
#include "ICameraControl.h"
#include "IGameManager.h"
#include "GlobalParameters.h"

#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/ILogSystem.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/IGUISystem.h"

#include <stdarg.h>


class ConsoleContext: public IConsoleContext
{
public:

	ConsoleContext()
	{
	
	}

	virtual void Serialize(ISimpleSerializer *pSerializer) 
	{
		// Do Nothing
	}

	virtual void Execute(SystemTable* sys);

private:

	// Helper functions for use in Execution context

	virtual void Log( const char* format, ... )
	{
		va_list args;
		va_start(args, format);
		PerModuleInterface::GetInstance()->GetSystemTable()->pLogSystem->LogVa(args, eLV_COMMENTS, format);
	}

	virtual IAUEntity* GetEntity( const char* name )
	{
		return PerModuleInterface::g_pSystemTable->pEntitySystem->Get( name );
	}

	virtual IGameObject* GetGameObject( const char* name )
	{
		IGameObject* pGameObject = 0;
		IObject* pObj = PerModuleInterface::g_pSystemTable->pEntitySystem->Get( name )->GetObject();
		IObjectUtils::GetObject( &pGameObject, pObj->GetObjectId() );
		return pGameObject;
	}

	virtual ILightingControl* LightingControl()
	{
		return (ILightingControl*)IObjectUtils::GetUniqueInterface( "LightingControl", IID_ILIGHTINGCONTROL );
	}

	virtual ICameraControl* CameraControl()
	{
		return (ICameraControl*)IObjectUtils::GetUniqueInterface( "CameraControl", IID_ICAMERACONTROL );
	}

	virtual IGameManager* GameManager()
	{
		return (IGameManager*)IObjectUtils::GetUniqueInterface( "GameManager", IID_IGAMEMANAGER );
	}

	virtual GlobalParameters* GlobalParameters()
	{
		return GameManager()->GetGlobalParameters();
	}

	virtual void ListEntities()
	{
		IEntitySystem* pEntitySystem = PerModuleInterface::g_pSystemTable->pEntitySystem;
		AUDynArray<AUEntityId> entities;
		pEntitySystem->GetAll(entities);
		Log( "Listing %d entities...\n", entities.Size() );
		for (size_t i=0; i<entities.Size(); ++i)
		{
			IAUEntity* pEnt = pEntitySystem->Get(entities[i]);
			Log( "%d: %s\n", pEnt->GetId(), pEnt->GetName() );
		}
	}

	virtual void ListGameObjects()
	{
		IEntitySystem* pEntitySystem = PerModuleInterface::g_pSystemTable->pEntitySystem;
		IObjectFactorySystem* pFactory = PerModuleInterface::g_pSystemTable->pObjectFactorySystem;

		// Get list of all Game Objects
		IObjectConstructor* pConstructor = pFactory->GetConstructor( "GameObject" );
		size_t count = pConstructor->GetNumberConstructedObjects();
		std::vector<ObjectId> objects; 
		objects.reserve(count); // actual amount might sometimes be less if some freed ids have not been reused
		for (size_t i=0; i<count; ++i)
		{
			IObject* pObj = pConstructor->GetConstructedObject(i);
			if (pObj)
			{
				objects.push_back(pObj->GetObjectId());
			}
		}

		// List all valid GameObjects now
		Log( "Listing %d GameObjects...\n", objects.size() );
		for (size_t i=0; i<objects.size(); ++i)
		{
			IEntityObject* pBase = 0;
			IObjectUtils::GetObject( &pBase, objects[i] );
			IAUEntity* pEnt = pEntitySystem->Get( pBase->GetEntityId() );
			Log( "%d: %s\n", pEnt->GetId(), pEnt->GetName() );
		}
	}

	virtual void SetLogProperty( const char* name, const char* value )
	{
		SystemTable* pSys = PerModuleInterface::g_pSystemTable;
		IGUIElement* pElement = pSys->pGUISystem->GetLogElement();
		if (pElement)
		{
			pElement->SetProperty( name, value );
			pElement->RemoveReference();
		}
	}

	virtual void Help()
	{
		Log( "Quick Command List:\n"
			"\t Log( format, ... ) \n"
			"\t ListEntities() \n"
			"\t ListGameObjects() \n"
			"\t GetEntity( name ) - returns IAUEntity* \n"
			"\t GetGameObject( name ) - returns IGameObject* \n"
			"\t GlobalParameters() - returns GlobalParameters* \n"
			"\t GameManager() - returns IGameManager* \n"
			"\t CameraControl() - returns ICameraControl* \n"
			"\t LightingControl() - returns ILightingControl* \n"
			"\t SetLogProperty( name, value ) \n"
		);
	}

	virtual void help()
	{
		Help();
	}
};


#endif // CONSOLECONTEXT_INCLUDED