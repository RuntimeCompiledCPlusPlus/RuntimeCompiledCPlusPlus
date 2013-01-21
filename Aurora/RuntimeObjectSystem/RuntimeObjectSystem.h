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

#ifndef RUNTIMEOBJECTSYSTEM_INCLUDED
#define RUNTIMEOBJECTSYSTEM_INCLUDED

#include "../RuntimeCompiler/IFileChangeNotifier.h"
#include "../RunTimeCompiler/BuildTool.h"
#include "../Common/AUArray.inl"
#include "ObjectInterface.h"
#include "IRuntimeObjectSystem.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#else
    typedef void* HMODULE;
#endif
#include <vector>
#include <map>

#include "../RuntimeCompiler/FileSystemUtils.h"

struct ICompilerLogger;
struct IObjectFactorySystem;

class RuntimeObjectSystem : public IRuntimeObjectSystem, public IFileChangeListener
{
public:
	RuntimeObjectSystem();
	virtual ~RuntimeObjectSystem();

	// Initialise RuntimeObjectSystem. pLogger should be deleted by creator
	virtual bool Initialise( ICompilerLogger * pLogger, SystemTable* pSystemTable );

	virtual bool GetIsCompiling()
	{
		return m_bCompiling;
	}

	virtual bool GetIsCompiledComplete();

	virtual bool LoadCompiledModule();

	virtual IObjectFactorySystem* GetObjectFactorySystem() const
	{
		return m_pObjectFactorySystem;
	}
	virtual IFileChangeNotifier* GetFileChangeNotifier() const
	{
		return m_pFileChangeNotifier;
	}

	virtual void CompileAll( bool bForceRecompile );
	virtual void AddToRuntimeFileList( const char* filename );
	virtual void RemoveFromRuntimeFileList( const char* filename );
	virtual void AddIncludeDir( const char *path_ );
	virtual void AddLibraryDir( const char *path_ );
	virtual void SetAdditionalCompileOptions( const char *options )
	{
		m_CompileOptions = options;
	}
	virtual void SetAdditionalLinkOptions( const char *options )
	{
		m_LinkOptions = options;
	}

	virtual void SetAutoCompile( bool autoCompile );
	virtual bool GetAutoCompile() const
	{
		return m_bAutoCompile;
	}
	virtual bool GetLastLoadModuleSuccess() const
	{
		return m_bLastLoadModuleSuccess;
	}



	// IFileChangeListener

	virtual void OnFileChange(const IAUDynArray<const char*>& filelist);

	// ~IFileChangeListener

private:
	typedef std::vector<FileSystemUtils::Path> TFileList;
	typedef std::multimap<FileSystemUtils::Path,FileSystemUtils::Path> TFileToFileMap;
	typedef TFileToFileMap::iterator TFileToFileIterator;
	typedef std::pair<FileSystemUtils::Path,FileSystemUtils::Path> TFileToFilePair;
	typedef std::pair<TFileToFileMap::iterator,TFileToFileMap::iterator> TFileToFileEqualRange;

	void StartRecompile();

	void InitObjects();
	void SetupObjectConstructors(GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd);
	void DeleteObjects();
	void ResetGame();


	// Members set in initialise
	ICompilerLogger*		m_pCompilerLogger;
	SystemTable*			m_pSystemTable;

	// Members created by this system
	IObjectFactorySystem*	m_pObjectFactorySystem;
	IFileChangeNotifier*	m_pFileChangeNotifier;
	BuildTool*				m_pBuildTool;

	bool					m_bCompiling;
	bool					m_bLastLoadModuleSuccess;
	std::vector<HMODULE>	m_Modules;	// Stores runtime created modules, but not the exe module.
	TFileList				m_RuntimeFileList;
	TFileToFileMap			m_RuntimeIncludeMap;
	TFileToFileMap			m_RuntimeLinkLibraryMap;
	bool					m_bAutoCompile;
	FileSystemUtils::Path m_CurrentlyCompilingModuleName;
	std::vector<BuildTool::FileToBuild> m_BuildFileList;
	std::vector<BuildTool::FileToBuild> m_PendingBuildFileList; // if a compile is already underway, store files here.
	TFileList				m_IncludeDirList;
	TFileList				m_LibraryDirList;
	std::string				m_CompileOptions;
	std::string				m_LinkOptions;

};

#endif // RUNTIMEOBJECTSYSTEM_INCLUDED