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

#ifndef CONSOLEGAME_INCLUDED
#define CONSOLEGAME_INCLUDED

#include "../../RuntimeCompiler/IFileChangeNotifier.h"
#include "../../RuntimeObjectSystem/IObjectFactorySystem.h"
#include "../../RuntimeObjectSystem/ObjectInterface.h"
#include "../../Common/AUArray.inl"
#include <Windows.h>
#include <vector>

#define BOOST_FILESYSTEM_VERSION 3
#include "boost/filesystem.hpp" 


class CompilerLogger;
class BuildTool;
struct IAUUpdateable;

class ConsoleGame : public IFileChangeListener, public IObjectFactoryListener
{
public:
	ConsoleGame();
	virtual ~ConsoleGame();

	bool Init();
	void Shutdown();
	bool MainLoop();


	// IFileChangeListener

	virtual void OnFileChange(const IAUDynArray<const char*>& filelist);

	// ~IFileChangeListener

	// IObjectFactoryListener

	virtual void OnConstructorsAdded();

	// ~IObjectFactoryListener


	void CompileAll( bool bForceRecompile );
	void AddToRuntimeFileList( const char* filename );
	void RemoveFromRuntimeFileList( const char* filename );
	void SetAutoCompile( bool autoCompile );

private:
	typedef std::vector<boost::filesystem::path> TFileList;

	void StartRecompile(const TFileList& filelist, bool bForce);
	bool LoadCompiledModule();

	void InitObjects();
	void SetupObjectConstructors(GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd);
	void DeleteObjects();
	void ResetGame();


	// Private Members
	CompilerLogger* m_pCompilerLogger;
	BuildTool* m_pBuildTool;

	// Runtime object
	IAUUpdateable* m_pUpdateable;
	ObjectId	   m_ObjectId;

	std::vector<HMODULE> m_Modules;	// Stores runtime created modules, but not the exe module.
	TFileList m_RuntimeFileList;
	bool m_bHaveProgramError;
	bool m_bCompiling;
	bool m_bAutoCompile;
	boost::filesystem::path m_CurrentlyCompilingModuleName;

};

#endif // CONSOLEGAME_INCLUDED