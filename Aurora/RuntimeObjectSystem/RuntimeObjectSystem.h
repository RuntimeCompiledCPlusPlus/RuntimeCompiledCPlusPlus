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
#include "../RuntimeCompiler/BuildTool.h"
#include "../RuntimeCompiler/AUArray.h"
#include "ObjectInterface.h"
#include "IRuntimeObjectSystem.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
	#undef GetObject
#else
    typedef void* HMODULE;
#endif
#include <vector>
#include <map>

#include "../RuntimeCompiler/FileSystemUtils.h"

struct ICompilerLogger;
struct IObjectFactorySystem;

class RuntimeObjectSystem : public IRuntimeObjectSystem, IFileChangeListener
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

    virtual void CompileAllInProject(           bool bForcerecompile_,  unsigned short projectId_ = 0 );
    virtual void AddToRuntimeFileList(          const char* filename,   unsigned short projectId_ = 0 );
    virtual void RemoveFromRuntimeFileList(     const char* filename,   unsigned short projectId_ = 0 );
    virtual void AddIncludeDir(                 const char* path_,      unsigned short projectId_ = 0 );
    virtual void AddLibraryDir(                 const char* path_,      unsigned short projectId_ = 0 );
    virtual void SetAdditionalCompileOptions(   const char* options,    unsigned short projectId_ = 0 );
    virtual void SetAdditionalLinkOptions(      const char* options,    unsigned short projectId_ = 0 );
    virtual void SetCompilerLocation        (   const char* path,       unsigned short projectId_ = 0 );
    virtual void SetOptimizationLevel( RCppOptimizationLevel optimizationLevel_,	unsigned short projectId_ = 0 );
    virtual RCppOptimizationLevel GetOptimizationLevel(					unsigned short projectId_ = 0 );
    virtual void SetIntermediateDir(            const char* path_,      unsigned short projectId_ = 0 );

	virtual void SetAutoCompile( bool autoCompile );
	virtual bool GetAutoCompile() const
	{
		return m_bAutoCompile;
	}

    virtual void SetFastCompileMode( bool bFast )
    {
        if( m_pBuildTool )
        {
            m_pBuildTool->SetFastCompileMode( bFast );
        }
    }

    virtual void CleanObjectFiles() const;

	virtual bool GetLastLoadModuleSuccess() const
	{
		return m_bLastLoadModuleSuccess;
	}
     virtual unsigned int GetNumberLoadedModules() const
     {
         return m_TotalLoadedModulesEver;
     }
 
	virtual void SetupObjectConstructors(IPerModuleInterface* pPerModuleInterface);

     // exception handling to catch and protect main app from crashing when using runtime compiling
    virtual void SetProtectionEnabled( bool bProtectionEnabled_ );
	
    virtual bool IsProtectionEnabled() const
    {
        return m_bProtectionEnabled;
    }
    virtual bool TryProtectedFunction( RuntimeProtector* pProtectedObject_ );

    
    // tests one by one touching each runtime modifiable source file
    // returns the number of errors - 0 if all passed.
   virtual int TestBuildAllRuntimeSourceFiles(  ITestBuildNotifier* callback, bool bTestFileTracking );

    // tests touching each header which has RUNTIME_MODIFIABLE_INCLUDE.
    // returns the number of errors - 0 if all passed.
    virtual int TestBuildAllRuntimeHeaders(     ITestBuildNotifier* callback, bool bTestFileTracking );


    virtual bool TestBuildCallback(const char* file, TestBuildResult type);
    virtual bool TestBuildWaitAndUpdate();

    // FindFile - attempts to find the file in a source directory
    virtual FileSystemUtils::Path   FindFile( const FileSystemUtils::Path& input );

    // AddPathToSourceSearch - adds a path to help source search. Can be called multiple times to add paths.
    virtual void AddPathToSourceSearch( const char* path );

	// IFileChangeListener

	virtual void OnFileChange(const IAUDynArray<const char*>& filelist);

	// ~IFileChangeListener

    std::vector<FileSystemUtils::Path> linkLibraryList;

private:
    typedef std::vector<FileSystemUtils::Path>                              TFileList;
	typedef std::map<FileSystemUtils::Path,FileSystemUtils::Path>           TFileMap;
	typedef TFileMap::iterator                                              TFileMapIterator;
	typedef std::multimap<FileSystemUtils::Path,FileSystemUtils::Path>      TFileToFilesMap;
	typedef TFileToFilesMap::iterator                                       TFileToFilesIterator;
	typedef std::pair<FileSystemUtils::Path,FileSystemUtils::Path>          TFileToFilePair;
	typedef std::pair<TFileToFilesMap::iterator,TFileToFilesMap::iterator>  TFileToFilesEqualRange;

	void StartRecompile();
	void SetupRuntimeFileTracking( const IAUDynArray<IObjectConstructor*>& constructors_ );

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

	bool					m_bAutoCompile;
	FileSystemUtils::Path   m_CurrentlyCompilingModuleName;

    // per project information
    struct ProjectSettings
    {
		ProjectSettings()
		{
			m_CompilerOptions.optimizationLevel = RCCPPOPTIMIZATIONLEVEL_DEFAULT;
			m_CompilerOptions.baseIntermediatePath = ms_DefaultIntermediatePath;
		}

		CompilerOptions						m_CompilerOptions;

		TFileList                           m_RuntimeFileList;
        TFileToFilesMap                     m_RuntimeIncludeMap;
        TFileToFilesMap			            m_RuntimeLinkLibraryMap;
        TFileToFilesMap                     m_RuntimeSourceDependencyMap;

        std::vector<BuildTool::FileToBuild> m_BuildFileList;
        std::vector<BuildTool::FileToBuild> m_PendingBuildFileList; // if a compile is already underway, store files here.

		static FileSystemUtils::Path		ms_DefaultIntermediatePath;
    };
    std::vector<ProjectSettings>            m_Projects;
    ProjectSettings&                        GetProject( unsigned short projectId_ );
    unsigned short                          m_CurrentlyBuildingProject;

    unsigned int            m_TotalLoadedModulesEver;
    bool                    m_bProtectionEnabled;


    // File mappings - we need to map from compiled path to a potentially different path
    // on the system the code is running on
    TFileMap                m_FoundSourceDirectoryMappings; // mappings between directories found and requested
    unsigned int            m_NumNotFoundSourceFiles;       // count of source directories not found

    // platform implementation in RuntimeObjectSystem_Plaform*.cpp
public:
    struct PlatformImpl;
private:
    PlatformImpl*           m_pImpl;
    void                    CreatePlatformImpl();
    void                    DeletePlatformImpl();

};

#endif // RUNTIMEOBJECTSYSTEM_INCLUDED
