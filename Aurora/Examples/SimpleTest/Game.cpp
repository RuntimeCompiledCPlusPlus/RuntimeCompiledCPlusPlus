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
#include "../../Common/AUArray.inl"

#include "../../Renderer/AURenMesh.h"
#include "../../Renderer/AURenderContext.h"
#include "../../Renderer/RenderWindow.h"
#include "../../RunTimeCompiler/BuildTool.h"
#include "../../RuntimeCompiler/ICompilerLogger.h"
#include "../../Systems/ILogSystem.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/ITimeSystem.h"
#include "../../Systems/IUpdateable.h"
#include "../../Systems/IObjectFactorySystem.h"
#include "../../Systems/IGUISystem.h"
#include "../../Systems/SystemTable.h"
#include "../../Audio/alManager.h"
#include "../../Audio/alSound.h"

#include "../../Systems/LogSystem/RocketLogSystem/RocketLogSystem.h"


#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <strstream>
#include <vector>
#include <algorithm>

#include <Rocket/Core.h>
#include <Rocket/Controls.h>
#include <Rocket/Debugger.h>
#include "../../Systems/RocketLibSystem/RocketLibSystem.h"
#include "../../Systems/RocketLibSystem/Input.h"

using boost::filesystem::path;


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
	, m_pCompilerLogger(0)
	, m_pBuildTool(0)
	, m_pRocketContext(0)
	, m_pOpenGLRenderer(0)
	, m_pSystemInterface(0)
	, m_pRenderContext(0)
	, m_bRenderError(false)
	, m_pCameraControl(0)
	, m_pLightingControl(0)
	, m_bHaveProgramError(false)
	, m_fLastUpdateSessionTime(-1)
	, m_bCompiling( false )
	, m_pLoopingBackgroundSound(0)
	, m_pLoopingBackgroundSoundBuffer(0)
	, m_GameSpeed(1.0f)
	, m_bAutoCompile( true )
{
	AU_ASSERT(g_pGame == NULL);
	g_pGame = this;
}

Game::~Game()
{
	m_pEnv->sys->pFileChangeNotifier->RemoveListener(this);

	delete m_pRenderContext;
	delete m_pSystemInterface;
	delete m_pOpenGLRenderer;
	delete m_pBuildTool;
	delete m_pCompilerLogger;
	delete m_pConsole;
	delete m_pEnv;
}


bool Game::Init()
{
	// We need the current directory to be the process dir
	DWORD size = MAX_PATH;
	wchar_t filename[MAX_PATH];
	GetModuleFileName( NULL, filename, size );
	std::wstring strTempFileName( filename );
	path launchPath( strTempFileName );
	launchPath = launchPath.parent_path();
	SetCurrentDirectory( launchPath.wstring().c_str() );

	m_pEnv = new Environment( this );
	m_pCompilerLogger = new CompilerLogger(m_pEnv);
	m_pBuildTool = new BuildTool();
	m_pBuildTool->Initialise(m_pCompilerLogger);
	m_pSystemInterface = new RocketLibSystemSystemInterface();
	m_pOpenGLRenderer = new RocketLibSystemRenderInterfaceOpenGL();

	// Should be nearly zero, but cleaner to explicitly fetch it
	m_fLastUpdateSessionTime = m_pEnv->sys->pTimeSystem->GetSessionTimeNow();

	RocketLibInit();

	//AURenderContext must be initialized after RocketLibInit() due to OGL init in RocketLibInit;
	m_pRenderContext = new AURenderContext();

	// We start by using the code in the current module
	HMODULE module = GetModuleHandle(NULL);

	GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd = NULL;
	pPerModuleInterfaceProcAdd = (GETPerModuleInterface_PROC) GetProcAddress(module, "GetPerModuleInterface");
	if (!pPerModuleInterfaceProcAdd)
	{
		m_pEnv->sys->pLogSystem->Log(eLV_ERRORS,"Failed GetProcAddress\n");
		return false;
	}

	// Tell it the system table to pass to objects we construct
	pPerModuleInterfaceProcAdd()->SetSystemTable(m_pEnv->sys);

	SetupObjectConstructors(pPerModuleInterfaceProcAdd);

	m_pEnv->sys->pObjectFactorySystem->AddListener(this);

	m_pConsole = new Console(this, m_pEnv, m_pRocketContext);

	InitSound();

	return true;
}


void Game::Run()
{
	InitObjects();

	RocketLibSystem::EventLoop(MainLoop_Wrapper);
}


void Game::Shutdown()
{
	DeleteObjects();
	RocketLibShutdown();
	ShutdownSound();
}


void Game::OnFileChange(const IAUDynArray<const char*>& filelist)
{

	if( !m_bAutoCompile )
	{
		return;
	}

	m_pEnv->sys->pLogSystem->Log(eLV_COMMENTS, "FileChangeNotifier triggered recompile of files:\n");
	bool bForce = false;
	TFileList pathlist;
	for( size_t i = 0; i < filelist.Size(); ++ i )
	{
		//m_pEnv->sys->pLogSystem->Log(eLV_COMMENTS, "  %s\n", filelist[i]);
		path filePath = filelist[i];
		if( filePath.extension() != ".h") //TODO: change to check for .cpp and .c as could have .inc files etc.?
		{
			pathlist.push_back( filePath );
		}
		else
		{
			TFileToFileEqualRange range = m_RuntimeIncludeMap.equal_range( filePath );
			for(TFileToFileIterator it=range.first; it!=range.second; ++it)
			{
				pathlist.push_back( (*it).second );
				bForce = true;
			}
		}
	}

	StartRecompile(pathlist, bForce);
}

void Game::OnConstructorsAdded()
{
	InitStoredObjectPointers();
}


void Game::CompileAll( bool bForceRecompile )
{
	StartRecompile(m_RuntimeFileList, bForceRecompile);
}

void Game::Reset()
{
	ResetGame();
}

void Game::SetAutoCompile( bool autoCompile )
{
	m_bAutoCompile = autoCompile;
}

void Game::AddToRuntimeFileList( const char* filename )
{
	path filePath = filename;
	TFileList::iterator it = std::find( m_RuntimeFileList.begin(), m_RuntimeFileList.end(), filePath );
	if ( it == m_RuntimeFileList.end() )
	{
		m_RuntimeFileList.push_back( filePath );
		m_pEnv->sys->pFileChangeNotifier->Watch( filename, this );
	}
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

void Game::RemoveFromRuntimeFileList( const char* filename )
{
	TFileList::iterator it = std::find( m_RuntimeFileList.begin(), m_RuntimeFileList.end(), filename );
	if ( it != m_RuntimeFileList.end() )
	{
		m_RuntimeFileList.erase( it );
	}
}

void Game::SetVolume( float volume )
{
	CalManager::GetInstance().SetVolume( volume );
}

void Game::SetSpeed( float speed )
{
	m_GameSpeed = speed;
	float pitch = 1.0f + 0.1f*(speed-1.0f);//fake, but works.
	CalManager::GetInstance().SetGlobalPitch( pitch );
}

void Game::GetWindowSize( float& width, float& height ) const
{
	int WindowSize[4];
	RocketLibSystem::GetViewport( WindowSize );
	width = (float)WindowSize[2];
	height = (float)WindowSize[3];
}

bool Game::ProtectedUpdate(AUDynArray<AUEntityId> &entities, float fDeltaTime)
{
	bool bSuccess = true;
	assert(!m_bHaveProgramError);

	__try {

		for (size_t i=0; i<entities.Size(); ++i)
		{
			IAUEntity* pEnt = m_pEnv->sys->pEntitySystem->Get(entities[i]);
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
	__except( RuntimeExceptionFilter() )
	{
		bSuccess = false;
	}
	return bSuccess;
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

	float fFrameTimeDelta = (float)pTimeSystem->GetSmoothFrameDuration();

	m_pEnv->sys->pFileChangeNotifier->Update(fSessionTimeDelta);

	//check status of any compile
	bool bLoadModule = false;
	if( m_bCompiling && m_pBuildTool->GetIsComplete() )
	{
		bLoadModule = true; //we load module after update/display, to get notification on screen correct
		m_bCompiling = false;
	}

	pTimeSystem->StartFrame();

	if( !m_bHaveProgramError )
	{
		AUDynArray<AUEntityId> entities;
		IEntitySystem* pEntitySystem = m_pEnv->sys->pEntitySystem;
		pEntitySystem->GetAll(entities);
		
		if (!ProtectedUpdate(entities, fClampedDelta))
		{
			m_bHaveProgramError = true;
			m_pEnv->sys->pLogSystem->Log(eLV_ERRORS, "Have caught an exception in main entity Update loop, code will not be run until new compile - please fix.\n");
		}
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
		bool bSuccess = LoadCompiledModule();
		m_pConsole->OnCompileDone(bSuccess);
	}

	// Limit frame rate
	double dTimeTaken = pTimeSystem->GetFrameTimeNow();
	const double dIdealTime = 1.0 / 60.0; 
	if ( dTimeTaken < dIdealTime)
	{
		Sleep( (DWORD) ((dIdealTime - dTimeTaken) * 1000.0) );
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

	//set sound position
	AUVec3f velocity(0.0f, 0.0f, 0.0f);
	CalManager::GetInstance().SetListener(	viewPos, velocity, viewOrientation );

	//play audio
	CalManager::GetInstance().PlayRequestedSounds();

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

void Game::StartRecompile(const TFileList& filelist, bool bForce)
{
	m_bCompiling = true;
	m_pEnv->sys->pLogSystem->Log(eLV_COMMENTS, "Compiling...\n");

	//Use a temporary filename for the dll
	wchar_t tempPath[MAX_PATH];
	GetTempPath( MAX_PATH, tempPath );
	wchar_t tempFileName[MAX_PATH]; 
	GetTempFileName( tempPath, L"", 0, tempFileName );
	std::wstring strTempFileName( tempFileName );

	//Need currentmodule path
	wchar_t CurrentModuleFileName[MAX_PATH];
	GetModuleFileNameW( NULL, CurrentModuleFileName, MAX_PATH ); //get filename of current module (full path?)
	path currModuleFileName(CurrentModuleFileName);
	path currModuleFullPath = currModuleFileName.parent_path();


	std::vector<BuildTool::FileToBuild> buildFileList;
	std::vector<path> includeDirList; //currently no include paths required
	m_CurrentlyCompilingModuleName= strTempFileName;

	for( size_t i = 0; i < filelist.size(); ++ i )
	{
		//don't compile .h files:
		if( filelist[i].extension() != ".h" ) //TODO: change to check for .cpp and .c as could have .inc files etc.?
		{
			buildFileList.push_back( BuildTool::FileToBuild( filelist[i], bForce ) );
		}
	}
	buildFileList.push_back( currModuleFullPath / path(L"/../RunTimeCompiler/ObjectInterfacePerModuleSource.cpp") );
	buildFileList.push_back( currModuleFullPath / path(L"/../RunTimeCompiler/ObjectInterfacePerModuleSource_PlatformWindows.cpp") );

	m_pBuildTool->BuildModule( buildFileList, includeDirList, m_CurrentlyCompilingModuleName );
}

bool Game::LoadCompiledModule()
{
	// Since the temporary file is created with 0 bytes, loadlibrary can fail with a dialogue we want to prevent. So check size
	// We pass in the ec value so the function won't throw an exception on error, but the value itself sometimes seems to
	// be set even without an error, so not sure if it should be relied on.
	boost::system::error_code ec;
	uintmax_t sizeOfModule = file_size( m_CurrentlyCompilingModuleName, ec );

	HMODULE module = 0;
	if( sizeOfModule )
	{
		module = LoadLibraryW( m_CurrentlyCompilingModuleName.c_str() );
	}

	if (!module)
	{
		m_pEnv->sys->pLogSystem->Log(eLV_ERRORS,"Failed to load module %ls\n",m_CurrentlyCompilingModuleName.c_str());
		return false;
	}

	GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd = (GETPerModuleInterface_PROC) GetProcAddress(module, "GetPerModuleInterface");
	if (!pPerModuleInterfaceProcAdd)
	{
		m_pEnv->sys->pLogSystem->Log(eLV_ERRORS,"Failed GetProcAddress\n");
		return false;
	}

	pPerModuleInterfaceProcAdd()->SetSystemTable( m_pEnv->sys );
	m_Modules.push_back( module );

	m_pEnv->sys->pLogSystem->Log(eLV_COMMENTS, "Compilation Succeeded\n");

	SetupObjectConstructors(pPerModuleInterfaceProcAdd);

	m_bHaveProgramError = false; //reset

	return true;
}


void Game::RocketLibInit()
{
	// Generic OS initialisation, creates a window and attaches OpenGL.
	if (!RocketLibSystem::Initialise("../") ||
	    !RocketLibSystem::OpenWindow(L"Pulse", true))
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

	RocketLibSystem::LoadFonts("/Assets/GUI/");

	// Set the Rocketlib logger font size
	IGUIElement* pElement = m_pEnv->sys->pGUISystem->GetLogElement();
	if (pElement)
	{
		pElement->SetProperty("font-size", "18pt");
		pElement->RemoveReference();
	}

	Rocket::Core::GetSystemInterface()->LogMessage(Rocket::Core::Log::LT_INFO, "Hello");
	
}


void Game::RocketLibShutdown()
{
	m_pRocketContext->RemoveReference();
	Rocket::Core::Shutdown();

	RocketLibSystem::CloseWindow();
	RocketLibSystem::Shutdown();
}

void Game::InitObjects()
{
	IObjectUtils::CreateUniqueObject( "MainObject" );
	
	InitStoredObjectPointers();
}

void Game::SetupObjectConstructors(GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd)
{
	// get hold of the constructors
	const std::vector<IObjectConstructor*> &objectConstructors = pPerModuleInterfaceProcAdd()->GetConstructors();
	AUDynArray<IObjectConstructor*> constructors( objectConstructors.size() );
	for (size_t i=0, iMax=objectConstructors.size(); i<iMax; ++i)
	{
		constructors[i] = objectConstructors[i];
		AddToRuntimeFileList( objectConstructors[i]->GetFileName() );

		//add include file mappings
		unsigned int includeNum = 0;
		while( objectConstructors[i]->GetIncludeFile( includeNum ) )
		{
			TFileToFilePair includePathPair;
			includePathPair.first = objectConstructors[i]->GetIncludeFile( includeNum );
			includePathPair.second = objectConstructors[i]->GetFileName();
			AddToRuntimeFileList( objectConstructors[i]->GetIncludeFile( includeNum ) );
			m_RuntimeIncludeMap.insert( includePathPair );
			++includeNum;
		}
	}
	m_pEnv->sys->pObjectFactorySystem->AddConstructors( constructors );
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

void Game::InitSound()
{
	AUVec3f pos, vel;
	AUOrientation3D orientation;
	CalManager::GetInstance().SetListener(	pos, vel, orientation );

	m_pLoopingBackgroundSoundBuffer = new CalBuffer("./../Assets/Sounds/62912_Benboncan_Heartbeat_Mono_shortloop.wav");
	m_pLoopingBackgroundSound = new CalSound(*m_pLoopingBackgroundSoundBuffer, true );
	m_pLoopingBackgroundSound->SetReferenceDistance( 1000.0f );	//since this is ambient it doesn't fade
	m_pLoopingBackgroundSound->Play( pos );
}

void Game::ShutdownSound()
{
	delete m_pLoopingBackgroundSound;
	delete m_pLoopingBackgroundSoundBuffer;
	CalManager::CleanUp();
	CalManager::GetInstance().SetIsEnabled( false );
}