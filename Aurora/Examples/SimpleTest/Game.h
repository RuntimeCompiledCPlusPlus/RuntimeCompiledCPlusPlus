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
#include "../../RuntimeCompiler/ObjectInterface.h"
#include "../../Common/AUArray.inl"
#include "../../Systems/IObjectFactorySystem.h"
#include "../../Systems/IGame.h"
#include <Rocket/Core/EventListener.h>
#include <Rocket/Core/Context.h>
#include <Windows.h>
#include <vector>
#include <map>

#define BOOST_FILESYSTEM_VERSION 3
#include "boost/filesystem.hpp" 


class Console;
class Environment;
class CompilerLogger;
class BuildTool;
class RocketLibSystemRenderInterfaceOpenGL;
class RocketLibSystemSystemInterface;
class AURenderContext;
struct ICameraControl;
struct ILightingControl;
typedef int AUEntityId;
class CalSound;
class CalBuffer;

class Game : public IGame, public IFileChangeListener, public IObjectFactoryListener
{
public:
	Game();
	virtual ~Game();

	bool Init();
	void Run();
	void Shutdown();
	void MainLoop(); // Called by libRocket and CompilerLogger


	// IFileChangeListener

	virtual void OnFileChange(const IAUDynArray<const char*>& filelist);

	// ~IFileChangeListener

	// IObjectFactoryListener

	virtual void OnConstructorsAdded();

	// ~IObjectFactoryListener

	// IGame 

	virtual void CompileAll( bool bForceRecompile );
	virtual void Reset();
	virtual void Restart();
	virtual void ToggleConsoleGUI();
	virtual void Exit();
	virtual void GetWindowSize( float& width, float& height ) const;
	virtual void AddToRuntimeFileList( const char* filename );
	virtual void RemoveFromRuntimeFileList( const char* filename );
	virtual void SetVolume( float volume );
	virtual void SetSpeed( float speed );
	virtual void SetAutoCompile( bool autoCompile );
	virtual bool GetIsCompiling() const
	{
		return m_bCompiling;
	}

	// ~IGame

private:
	typedef std::vector<boost::filesystem::path> TFileList;
	typedef std::multimap<boost::filesystem::path,boost::filesystem::path> TFileToFileMap;
	typedef TFileToFileMap::iterator TFileToFileIterator;
	typedef std::pair<boost::filesystem::path,boost::filesystem::path> TFileToFilePair;
	typedef std::pair<TFileToFileMap::iterator,TFileToFileMap::iterator> TFileToFileEqualRange;

	void StartRecompile(const TFileList& filelist, bool bForce);
	bool LoadCompiledModule();

	void RocketLibInit();
	void RocketLibUpdate();
	void RocketLibShutdown();
	void RenderWorld();

	void InitSound();
	void ShutdownSound();

	void InitObjects();
	void SetupObjectConstructors(GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd);
	void DeleteObjects();
	void ResetGame();
	void InitStoredObjectPointers(); // Get or refresh our stored pointers to runtime objects
	bool ProtectedUpdate(AUDynArray<AUEntityId> &entities, float fDeltaTime);


	// Private Members

	Console* m_pConsole;
	Environment* m_pEnv;
	CompilerLogger* m_pCompilerLogger;
	BuildTool* m_pBuildTool;
	
	Rocket::Core::Context* m_pRocketContext;
	RocketLibSystemRenderInterfaceOpenGL* m_pOpenGLRenderer;
	RocketLibSystemSystemInterface* m_pSystemInterface;
	AURenderContext* m_pRenderContext;
	bool m_bRenderError;

	ICameraControl* m_pCameraControl;
	ILightingControl* m_pLightingControl;

	std::vector<HMODULE> m_Modules;	// Stores runtime created modules, but not the exe module.
	TFileList m_RuntimeFileList;
	TFileToFileMap m_RuntimeIncludeMap;
	bool m_bHaveProgramError;
	double m_fLastUpdateSessionTime;
	bool m_bCompiling;
	bool m_bAutoCompile;
	boost::filesystem::path m_CurrentlyCompilingModuleName;

	CalSound*	m_pLoopingBackgroundSound;
	CalBuffer*	m_pLoopingBackgroundSoundBuffer;

	float m_GameSpeed;

};

#endif // GAME_INCLUDED