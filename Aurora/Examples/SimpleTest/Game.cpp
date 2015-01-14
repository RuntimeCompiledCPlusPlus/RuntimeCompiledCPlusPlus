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

#include "Game.h"

// Remove windows.h define of GetObject which conflicts with EntitySystem GetObject
#if defined _WINDOWS_ && defined GetObject
#undef GetObject
#endif

#include "CompilerLogger.h"
#include  "InterfaceIds.h"
#include "IObjectUtils.h"
#include "Console.h"
#include "Environment.h"
#include "ICameraControl.h"
#include "ILightingControl.h"
#include "IGameManager.h"

#include "../../Common/AUOrientation3D.inl"
#include "../../RuntimeCompiler/AUArray.h"

#include "../../Renderer/AURenMesh.h"
#include "../../Renderer/AURenderContext.h"
#include "../../RuntimeCompiler/BuildTool.h"
#include "../../RuntimeCompiler/ICompilerLogger.h"
#include "../../Systems/ILogSystem.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/ITimeSystem.h"
#include "../../Systems/IUpdateable.h"
#include "../../RuntimeObjectSystem/IObjectFactorySystem.h"
#include "../../RuntimeObjectSystem/RuntimeObjectSystem.h"
#include "../../Systems/IGUISystem.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/IAssetSystem.h"

#include "../../Systems/LogSystem/RocketLogSystem/RocketLogSystem.h"


#include <stdio.h>
#ifdef _WIN32
    #include <tchar.h>
    #include <conio.h>
#endif
#include <strstream>
#include <vector>
#include <algorithm>

#include <Rocket/Core.h>
#include <Rocket/Controls.h>
#include <Rocket/Debugger.h>
#include "../../Systems/RocketLibSystem/RocketLibSystem.h"
#include "../../Systems/RocketLibSystem/Input.h"

#include <GL/glfw.h>

using FileSystemUtils::Path;


void Game::EntityUpdateProtector::ProtectedFunc()
{
	for (size_t i=0; i<entities.Size(); ++i)
	{
		IAUEntity* pEnt = pEntitySystem->Get(entities[i]);
		if (pEnt) // Safety check in case entity was deleted during this update by another object
		{
			IAUUpdateable* pUpdateable = pEnt->GetUpdateable();
			if (pUpdateable)
			{
				// If dropped here after a runtime failure, your crash was likely
				// somewhere directly in the pUpdatable object's Update method
				pUpdateable->Update(fDeltaTime);
			}
		}
	}
}


// Global pointer to Game object necessary so we can do callback to Game::MainLoop method
// Could be dangerous if we're instantiating multiple Game objects for some reason
static Game* g_pGame = NULL;
void MainLoop_Wrapper()
{
	g_pGame->MainLoop();
}


Game::Game()
	: m_pEnv(0)
	, m_pConsole(0)
	, m_pRocketContext(0)
	, m_pOpenGLRenderer(0)
	, m_pSystemInterface(0)
	, m_pRenderContext(0)
	, m_bRenderError(false)
	, m_pCameraControl(0)
	, m_pLightingControl(0)
	, m_fLastUpdateSessionTime(-1)
	, m_GameSpeed(1.0f)
	, m_CompileStartedTime(0.0)
{
	AU_ASSERT(g_pGame == NULL);
	g_pGame = this;
}

Game::~Game()
{
	delete m_pRenderContext;
	delete m_pSystemInterface;
	delete m_pOpenGLRenderer;
	delete m_pConsole;
	delete m_pEnv;
}


bool Game::Init()
{
#ifdef _WIN32
	// We Set Dir here so logs go to bin directory, useful for debugging as dir can be set anywhere.
	DWORD size = MAX_PATH;
	char filename[MAX_PATH];
	GetModuleFileNameA( NULL, filename, size );
	std::string strTempFileName( filename );
	Path launchPath( strTempFileName );
	launchPath = launchPath.ParentPath();
	SetCurrentDirectoryA( launchPath.m_string.c_str() );
#endif
    

	m_pEnv = new Environment( this );
	m_pSystemInterface = new RocketLibSystemSystemInterface();
	m_pOpenGLRenderer = new RocketLibSystemRenderInterfaceOpenGL();

	// Should be nearly zero, but cleaner to explicitly fetch it
	m_fLastUpdateSessionTime = m_pEnv->sys->pTimeSystem->GetSessionTimeNow();

	RocketLibInit();

	//AURenderContext must be initialized after RocketLibInit() due to OGL init in RocketLibInit;
	m_pRenderContext = new AURenderContext();

	m_pEnv->sys->pObjectFactorySystem->AddListener(this);

	m_EntityUpdateProtector.pEntitySystem = m_pEnv->sys->pEntitySystem;

    m_pEnv->Init();
	m_pConsole = new Console(m_pEnv, m_pRocketContext);

	return true;
}


void Game::Run()
{
	InitObjects();

	RocketLibSystem::EventLoop(MainLoop_Wrapper);
}


void Game::Shutdown()
{
    // clean up any temp object files
    if( m_pEnv->sys->pRuntimeObjectSystem )
    {
        m_pEnv->sys->pRuntimeObjectSystem->CleanObjectFiles();
    }

    DeleteObjects();
	RocketLibShutdown();
}

void Game::OnConstructorsAdded()
{
	InitStoredObjectPointers();
}

void Game::Reset()
{
	ResetGame();
}

void Game::Restart()
{
	delete IObjectUtils::GetUniqueObject( "MainObject" );
	IObjectUtils::CreateUniqueObject( "MainObject" );
	InitStoredObjectPointers();
}

void Game::ToggleConsoleGUI()
{
	m_pConsole->ToggleGUI();
}

void Game::Exit()
{
	RocketLibSystem::RequestExit();
}

void Game::SetSpeed( float speed )
{
	m_GameSpeed = speed;
}

void Game::GetWindowSize( float& width, float& height ) const
{
	int WindowSize[4];
	RocketLibSystem::GetViewport( WindowSize );
	width = (float)WindowSize[2];
	height = (float)WindowSize[3];
}



void Game::MainLoop()
{
	ITimeSystem *pTimeSystem = m_pEnv->sys->pTimeSystem;

	// Time in userspace, ignoring frametime and whether we are paused, compiling, etc.
	// That seems most appropriate to the filechangenotifier
	double fSessionTimeNow = pTimeSystem->GetSessionTimeNow();
	float fSessionTimeDelta = (float)(fSessionTimeNow - m_fLastUpdateSessionTime);
	float fClampedDelta = (std::min)( fSessionTimeDelta*m_GameSpeed, 0.1f ); // used for IObject updates
	m_fLastUpdateSessionTime = fSessionTimeNow;

	m_pEnv->sys->pFileChangeNotifier->Update(fSessionTimeDelta);

	if( m_pEnv->sys->pRuntimeObjectSystem->GetIsCompiling() && m_CompileStartedTime == 0.0 )
	{
		m_CompileStartedTime = pTimeSystem->GetSessionTimeNow();
	}

	//check status of any compile
	bool bLoadModule = false;
	if( m_pEnv->sys->pRuntimeObjectSystem->GetIsCompiledComplete() )
	{
		bLoadModule = true; //we load module after update/display, to get notification on screen correct
	}

	pTimeSystem->StartFrame();

	AUDynArray<AUEntityId> entities;
	m_EntityUpdateProtector.pEntitySystem->GetAll(m_EntityUpdateProtector.entities);
    m_EntityUpdateProtector.fDeltaTime = fClampedDelta;
		
    if (!m_pEnv->sys->pRuntimeObjectSystem->TryProtectedFunction( &m_EntityUpdateProtector ) )
	{
		m_pEnv->sys->pLogSystem->Log(eLV_ERRORS, "Have caught an exception in main entity Update loop, code will not be run until new compile - please fix.\n");
	}


	if (!m_pCameraControl || !m_pLightingControl)
	{
		if (!m_bRenderError)
		{
			m_bRenderError = true;
			m_pEnv->sys->pLogSystem->Log(eLV_ERRORS, "Missing Camera and/or Lighting control. Can't render world.\n");
		}	
	}
	else
	{
		m_bRenderError = false;
		RenderWorld();
	}

	RocketLibUpdate();

	if( bLoadModule )
	{
		// load module when compile complete, and notify console - TODO replace with event system 
		bool bSuccess = m_pEnv->sys->pRuntimeObjectSystem->LoadCompiledModule();
		m_pConsole->OnCompileDone(bSuccess);
		if( bSuccess )
		{
			float compileAndLoadTime = (float)( pTimeSystem->GetSessionTimeNow() - m_CompileStartedTime );
			m_pEnv->sys->pLogSystem->Log(eLV_COMMENTS, "Compile and Module Reload Time: %.1f s\n", compileAndLoadTime);

		}
		m_CompileStartedTime = 0.0;
	}

	// Limit frame rate
	double dTimeTaken = pTimeSystem->GetFrameTimeNow();
	const double dIdealTime = 1.0 / 70.0; //ideal time is actually 1/60, but we want some leeway 
	if ( dTimeTaken < dIdealTime)
	{
        glfwSleep( dIdealTime - dTimeTaken );
	}

	pTimeSystem->EndFrame();
}

void Game::RenderWorld()
{
	float params[4];

	m_pLightingControl->GetBackColor(params);
	glClearColor(params[0], params[1], params[2], params[3]);
	glClearDepth( 1.0 );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Hacked render of mesh
	glDisable(GL_FOG);

	int WindowSize[4];
	RocketLibSystem::GetViewport( WindowSize );
	//may need to re-size context, check
	Rocket::Core::Vector2i contextSize = m_pRocketContext->GetDimensions();
	if( contextSize.x != WindowSize[2] || contextSize.y != WindowSize[3] )
	{
		contextSize.x = WindowSize[2];
		contextSize.y = WindowSize[3];
		m_pRocketContext->SetDimensions( contextSize );
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(60.0f,(GLdouble)WindowSize[2]/(GLdouble)WindowSize[3],
	//	1.0f, 100000.0f);
	glOrtho( (GLdouble)contextSize.x * -0.5, (GLdouble)contextSize.x * 0.5, 
		       (GLdouble)contextSize.y * -0.5, (GLdouble)contextSize.y * 0.5,
					 1.0f, 100000.0f );
	glMatrixMode(GL_MODELVIEW); // Always go back to modelview matrix
	glLoadIdentity();
	float fglMatrix[16];
	AUOrientation3D viewOrientation;
	viewOrientation.Set( AUVec3f( 0.0f, 1.0f, 0.0f ), AUVec3f( 0.0f, 0.0f, 1.0f ) );
	viewOrientation.LoadglViewMatrix(fglMatrix);
	glMultMatrixf(fglMatrix);
	AUVec3f viewPos = m_pCameraControl->GetCurrentPos();
	glTranslatef(	-viewPos.x,
		-viewPos.y,
		-viewPos.z );

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_POLYGON_SMOOTH);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	m_pLightingControl->GetGlobalAmbient(params);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, params);
	glEnable(GL_LIGHTING);
	m_pLightingControl->GetLightAmbient(params);
	glLightfv(GL_LIGHT0, GL_AMBIENT, params);
	m_pLightingControl->GetLightDiffuse(params);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, params);
	m_pLightingControl->GetLightSpecular(params);
	glLightfv(GL_LIGHT0, GL_SPECULAR, params);
	glEnable(GL_LIGHT0);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable( GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisable( GL_ALPHA_TEST );
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc( GL_LESS );

	m_pRenderContext->Render( m_pEnv->sys->pEntitySystem );
	// End mesh draw

}

void Game::RocketLibUpdate()
{
	// Push through any log messages before rendering
	m_pEnv->sys->pRocketLogSystem->Push();

	RocketLibSystem::PreRenderRocketLib();
	m_pRocketContext->Update();
	m_pRocketContext->Render();

	RocketLibSystem::FlipBuffers();
}

void Game::RocketLibInit()
{
	// Generic OS initialisation, creates a window and attaches OpenGL.
	if (!RocketLibSystem::Initialise() ||
	    !RocketLibSystem::OpenWindow("Pulse", true))
	{
		RocketLibSystem::Shutdown();
		return;
	}

	// Rocket initialisation.
	Rocket::Core::SetRenderInterface(m_pOpenGLRenderer);
	Rocket::Core::SetSystemInterface(m_pSystemInterface);
	Rocket::Core::Initialise();
	Rocket::Controls::Initialise();

	// Create the main Rocket context and set it on the RocketLibSystem's input layer.
	Rocket::Core::Vector2i contextSize;
	int WindowSize[4];
	RocketLibSystem::GetViewport( WindowSize );
	contextSize.x = WindowSize[2];
	contextSize.y = WindowSize[3];
	m_pRocketContext = Rocket::Core::CreateContext("main", contextSize);
	if (m_pRocketContext == NULL)
	{
		Rocket::Core::Shutdown();
		RocketLibSystem::Shutdown();
		return;
	}

	m_pEnv->sys->pGUISystem->SetContext(m_pRocketContext);

	Rocket::Debugger::Initialise(m_pRocketContext);
	Input::SetContext(m_pRocketContext);

	RocketLibSystem::LoadFonts("/GUI/");

	// Set the Rocketlib logger font size
	IGUIElement* pElement = m_pEnv->sys->pGUISystem->GetLogElement();
	if (pElement)
	{
		pElement->SetProperty("font-size", "18pt");
		pElement->RemoveReference();
	}
	
}


void Game::RocketLibShutdown()
{
    // ensure any log messages prior to shutdown are output
    if( m_pEnv->sys->pRocketLogSystem )
    {
        m_pEnv->sys->pRocketLogSystem->Push();
    }
    
	m_pRocketContext->RemoveReference();
	Rocket::Core::Shutdown();

	RocketLibSystem::CloseWindow();
	RocketLibSystem::Shutdown();
}

void Game::InitObjects()
{	
	InitStoredObjectPointers();
}

void Game::DeleteObjects()
{
	m_pConsole->DestroyContext();

	delete IObjectUtils::GetUniqueObject( "MainObject" );


#ifdef _DEBUG
	// Do a check to verify that all objects have been destroyed at this point
	int totalObjectCount = 0;
	AUDynArray<IObjectConstructor*> constructors;
	m_pEnv->sys->pObjectFactorySystem->GetAll(constructors);
	for (size_t i=0; i<constructors.Size(); ++i)
	{
		IObjectConstructor* pConstructor = constructors[i];
	
		size_t count = pConstructor->GetNumberConstructedObjects();

		// Need to iterate through all objects and check if they're valid
		// since GetNumConstructedObjects isn't accurate, can return some null pointers
		for (size_t j=0; j<count; ++j)
		{
			if (pConstructor->GetConstructedObject(j) != NULL)
			{
				totalObjectCount++;
			}
		}

		// Do an assert check here so it's easy to figure out which object type wasn't deleted
		AU_ASSERT( totalObjectCount == 0 );
	}

#endif
}

void Game::ResetGame()
{
	IGameManager* pGameManager = (IGameManager*)IObjectUtils::GetUniqueInterface( "GameManager", IID_IGAMEMANAGER );
	pGameManager->ResetGame();
}

void Game::InitStoredObjectPointers()
{
	m_pCameraControl = (ICameraControl*)IObjectUtils::GetUniqueInterface( "CameraControl", IID_ICAMERACONTROL );
	AU_ASSERT(m_pCameraControl);

	m_pLightingControl = (ILightingControl*)IObjectUtils::GetUniqueInterface( "LightingControl", IID_ILIGHTINGCONTROL );
	AU_ASSERT(m_pLightingControl);
}

void Game::RunRCCppTests( bool bTestFileTracking )
{
    m_pEnv->sys->pRuntimeObjectSystem->CleanObjectFiles();
    m_pEnv->sys->pRuntimeObjectSystem->TestBuildAllRuntimeSourceFiles( this, bTestFileTracking );
    m_pEnv->sys->pRuntimeObjectSystem->TestBuildAllRuntimeHeaders( this, bTestFileTracking );
}


bool Game::TestBuildCallback(const char* file, TestBuildResult type)
{
    switch( type )
    {
    case TESTBUILDRRESULT_SUCCESS:            // SUCCESS, yay!
        m_pEnv->sys->pLogSystem->Log(eLV_EVENTS, "TESTBUILDRRESULT_SUCCESS: %s\n", file);
        break;
    case TESTBUILDRRESULT_NO_FILES_TO_BUILD:  // file registration error or no runtime files of this type
        m_pEnv->sys->pLogSystem->Log(eLV_WARNINGS, "TESTBUILDRRESULT_NO_FILES_TO_BUILD\n");
        break;
    case TESTBUILDRRESULT_BUILD_FILE_GONE:    // the file is no longer present
        m_pEnv->sys->pLogSystem->Log(eLV_ERRORS, "TESTBUILDRRESULT_BUILD_FILE_GONE: %s\n", file);
        break;
    case TESTBUILDRRESULT_BUILD_NOT_STARTED:  // file change detection could be broken, or if an include may not be included anywhere
        m_pEnv->sys->pLogSystem->Log(eLV_ERRORS, "TESTBUILDRRESULT_BUILD_NOT_STARTED: %s\n", file);
        break;
    case TESTBUILDRRESULT_BUILD_FAILED:       // a build was started, but it failed or module failed to load. See log.
        m_pEnv->sys->pLogSystem->Log(eLV_ERRORS, "TESTBUILDRRESULT_BUILD_FAILED: %s\n", file);
        break;
    case TESTBUILDRRESULT_OBJECT_SWAP_FAIL:   // build succeeded, module loaded but errors on swapping
        m_pEnv->sys->pLogSystem->Log(eLV_ERRORS, "TESTBUILDRRESULT_OBJECT_SWAP_FAIL: %s\n", file);
        break;
    default:
        assert(false);
        break;
    }
    return true;
}

bool Game::TestBuildWaitAndUpdate()
{
	double dTimeStart = glfwGetTime();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RocketLibUpdate();
    double dTimeEnd = glfwGetTime();

    const double dGoodWaitTime = 0.05;
    double dTimeLeft = dGoodWaitTime - (dTimeEnd - dTimeStart);
	if( dTimeLeft > 0.0 )
	{
        glfwSleep( dTimeLeft );
	}

    if( !glfwGetWindowParam( GLFW_OPENED ) )
    {
        // closed window so stop
        return false;
    }
    return true;
}
