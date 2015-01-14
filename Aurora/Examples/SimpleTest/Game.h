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

#ifndef GAME_INCLUDED
#define GAME_INCLUDED

#include "../../RuntimeCompiler/IFileChangeNotifier.h"
#include "../../RuntimeObjectSystem/ObjectInterface.h"
#include "../../RuntimeObjectSystem/IObjectFactorySystem.h"
#include "../../RuntimeObjectSystem/IRuntimeObjectSystem.h"
#include "../../RuntimeObjectSystem/RuntimeProtector.h"
#include "../../RuntimeCompiler/AUArray.h"
#include "../../Systems/IGame.h"
#include <Rocket/Core/EventListener.h>
#include <Rocket/Core/Context.h>
#include <vector>
#include <map>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif


class Console;
class Environment;
class RocketLibSystemRenderInterfaceOpenGL;
class RocketLibSystemSystemInterface;
class AURenderContext;
struct ICameraControl;
struct ILightingControl;
typedef int AUEntityId;
struct IEntitySystem;


class Game : public IGame, public IObjectFactoryListener, public ITestBuildNotifier
{
public:
	Game();
	virtual ~Game();

	bool Init();
	void Run();
	void Shutdown();
	void MainLoop(); // Called by libRocket and CompilerLogger



	// IObjectFactoryListener

	virtual void OnConstructorsAdded();

	// ~IObjectFactoryListener

	// IGame 

	virtual void Reset();
	virtual void Restart();
	virtual void ToggleConsoleGUI();
	virtual void Exit();
	virtual void GetWindowSize( float& width, float& height ) const;
	virtual void SetSpeed( float speed );
	virtual void RunRCCppTests( bool bTestFileTracking );

	// ~IGame
    virtual bool TestBuildCallback(const char* file, TestBuildResult type);
    virtual bool TestBuildWaitAndUpdate();

private:

	void RocketLibInit();
	void RocketLibUpdate();
	void RocketLibShutdown();
	void RenderWorld();

	void InitObjects();
	void DeleteObjects();
	void ResetGame();
	void InitStoredObjectPointers(); // Get or refresh our stored pointers to runtime objects
	bool ProtectedUpdate(AUDynArray<AUEntityId> &entities, float fDeltaTime);


	// Private Members

	Console*				m_pConsole;
	Environment*			m_pEnv;
	
	Rocket::Core::Context*					m_pRocketContext;
	RocketLibSystemRenderInterfaceOpenGL*	m_pOpenGLRenderer;
	RocketLibSystemSystemInterface*			m_pSystemInterface;
	AURenderContext*						m_pRenderContext;
	bool									m_bRenderError;

	ICameraControl*		m_pCameraControl;
	ILightingControl*	m_pLightingControl;

	double				m_fLastUpdateSessionTime;
	double				m_CompileStartedTime;


	float				m_GameSpeed;

    // Local class definition for handling protected update
    class EntityUpdateProtector : public RuntimeProtector
    {
    public:
        AUDynArray<AUEntityId>  entities;
        float                   fDeltaTime;
        IEntitySystem*          pEntitySystem;
    
    private:
        virtual void ProtectedFunc();

    };
    EntityUpdateProtector m_EntityUpdateProtector;

};

#endif // GAME_INCLUDED