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

#include "ConsoleGame.h"

// Remove windows.h define of GetObject which conflicts with EntitySystem GetObject
#if defined _WINDOWS_ && defined GetObject
#undef GetObject
#endif
#include "../../Common/AUArray.inl"
#include "../../RunTimeCompiler/BuildTool.h"
#include "../../RuntimeCompiler/ICompilerLogger.h"
#include "../../RuntimeCompiler/FileChangeNotifier.h"
#include "../../Systems/ILogSystem.h"
#include "../../Systems/IObjectFactorySystem.h"
#include "../../Systems/ObjectFactorySystem/ObjectFactorySystem.h"

#include "../../Systems/SystemTable.h"
#include "../../Systems/Systems.h"


#include "StdioLogSystem.h"
#include "../../Systems/LogSystem/FileLogSystem/FileLogSystem.h"
#include "../../Systems/LogSystem/MultiLogSystem/MultiLogSystem.h"
#include "../../Systems/LogSystem/ThreadsafeLogSystem/ThreadsafeLogSystem.h"

#include "../SimpleTest/IObject.h"
#include "../../Systems/IUpdateable.h"

#include <iostream>
#include <tchar.h>
#include <conio.h>
#include <strstream>
#include <vector>
#include <algorithm>
#include <string>
#include <stdarg.h>

using boost::filesystem::path;

SystemTable* gSys = 0;

class CompilerLogger : public ICompilerLogger
{
public:
	virtual void LogError(const char * format, ...)
	{
		va_list args;
		va_start(args, format);
		gSys->pLogSystem->LogVa(args, eLV_ERRORS, format);
	}

	virtual void LogWarning(const char * format, ...)
	{
		va_list args;
		va_start( args, format );
		gSys->pLogSystem->LogVa(args, eLV_WARNINGS, format);
	}

    virtual void LogInfo(const char * format, ...)
	{
		va_list args;
		va_start( args, format );
		gSys->pLogSystem->LogVa(args, eLV_COMMENTS, format);
	}
};


ConsoleGame::ConsoleGame()
	: m_pCompilerLogger(0)
	, m_pBuildTool(0)
	, m_bHaveProgramError(false)
	, m_bCompiling( false )
	, m_bAutoCompile( true )
	, m_pUpdateable(0)
{
}

ConsoleGame::~ConsoleGame()
{
	gSys->pFileChangeNotifier->RemoveListener(this);

	delete m_pUpdateable;

	//should clean up loggers.
	delete gSys->pFileChangeNotifier;
	delete gSys->pObjectFactorySystem;
	delete m_pBuildTool;
	delete m_pCompilerLogger;
}


bool ConsoleGame::Init()
{
	// We need the current directory to be the process dir
	DWORD size = MAX_PATH;
	wchar_t filename[MAX_PATH];
	GetModuleFileName( NULL, filename, size );
	std::wstring strTempFileName( filename );
	path launchPath( strTempFileName );
	launchPath = launchPath.parent_path();
	SetCurrentDirectory( launchPath.wstring().c_str() );

	m_pCompilerLogger = new CompilerLogger();
	m_pBuildTool = new BuildTool();
	m_pBuildTool->Initialise(m_pCompilerLogger);

	// We start by using the code in the current module
	HMODULE module = GetModuleHandle(NULL);

	GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd = NULL;
	pPerModuleInterfaceProcAdd = (GETPerModuleInterface_PROC) GetProcAddress(module, "GetPerModuleInterface");
	if (!pPerModuleInterfaceProcAdd)
	{
		std::cout << "Failed GetProcAddress for GetPerModuleInterface in current module\n";
		return false;
	}


	gSys = new SystemTable;
	StdioLogSystem* pStdioLog = new StdioLogSystem();
	FileLogSystem *pFileLog = new FileLogSystem();
	pFileLog->SetLogPath("Log.txt");
	pFileLog->SetVerbosity(eLV_COMMENTS);
	pFileLog->Log(eLV_EVENTS, "Started file logger\n");


	MultiLogSystem *pMultiLog = new MultiLogSystem();
	pMultiLog->AddLogSystem(pFileLog);
	pMultiLog->AddLogSystem(pStdioLog);

	ThreadsafeLogSystem *pThreadsafeLog = new ThreadsafeLogSystem();
	pThreadsafeLog->SetProtectedLogSystem(pMultiLog);

	gSys->pLogSystem = pThreadsafeLog;

	gSys->pLogSystem->Log(eLV_EVENTS, "All logs initialised\n");

	pPerModuleInterfaceProcAdd()->SetSystemTable(gSys);

	gSys->pObjectFactorySystem = new ObjectFactorySystem();

	gSys->pFileChangeNotifier = new FileChangeNotifier();


	SetupObjectConstructors(pPerModuleInterfaceProcAdd);

	gSys->pObjectFactorySystem->AddListener(this);


	// construct first object
	IObjectConstructor* pCtor = gSys->pObjectFactorySystem->GetConstructor( "RuntimeObject01" );
	if( pCtor )
	{
		IObject* pObj = pCtor->Construct();
		pObj->GetInterface( IID_IUPDATEABLE, (void**)&m_pUpdateable );
		if( 0 == m_pUpdateable )
		{
			delete pObj;
			gSys->pLogSystem->Log(eLV_ERRORS, "Error - no updateable interface found\n");
			return false;
		}

	}


	return true;
}




void ConsoleGame::Shutdown()
{
}


void ConsoleGame::OnFileChange(const IAUDynArray<const char*>& filelist)
{

	if( !m_bAutoCompile )
	{
		return;
	}

	gSys->pLogSystem->Log(eLV_COMMENTS, "FileChangeNotifier triggered recompile of files:\n");

	TFileList pathlist;
	pathlist.resize(filelist.Size());
	for( size_t i = 0; i < filelist.Size(); ++ i )
	{
		pathlist[i] = path(filelist[i]);
	}

	StartRecompile(pathlist, false);
}

void ConsoleGame::OnConstructorsAdded()
{
}


void ConsoleGame::CompileAll( bool bForceRecompile )
{
	StartRecompile(m_RuntimeFileList, bForceRecompile);
}

void ConsoleGame::SetAutoCompile( bool autoCompile )
{
	m_bAutoCompile = autoCompile;
}

void ConsoleGame::AddToRuntimeFileList( const char* filename )
{
	TFileList::iterator it = std::find( m_RuntimeFileList.begin(), m_RuntimeFileList.end(), filename );
	if ( it == m_RuntimeFileList.end() )
	{
		m_RuntimeFileList.push_back( filename );
	}
}

void ConsoleGame::RemoveFromRuntimeFileList( const char* filename )
{
	TFileList::iterator it = std::find( m_RuntimeFileList.begin(), m_RuntimeFileList.end(), filename );
	if ( it != m_RuntimeFileList.end() )
	{
		m_RuntimeFileList.erase( it );
	}
}



bool ConsoleGame::MainLoop()
{
	const float deltaTime = 0.1f;
	gSys->pFileChangeNotifier->Update( deltaTime );

	//check status of any compile
	if( m_bCompiling && m_pBuildTool->GetIsComplete() )
	{
		// load module when compile complete, and notify console - TODO replace with event system 
		bool bSuccess = LoadCompiledModule();
		m_bCompiling = false;

	}

	std::cout << "\nMain Loop - press q to quit, enter to run object loop and check for changed files\n";
	int ret = _getche();
	if( 'q' == ret )
	{
		return false;
	}
	m_pUpdateable->Update( deltaTime );

	return true;
}

void ConsoleGame::StartRecompile(const TFileList& filelist, bool bForce)
{
	m_bCompiling = true;
	gSys->pLogSystem->Log(eLV_COMMENTS, "Compiling...\n");

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


	std::vector<path> buildFileList;
	std::vector<path> includeDirList;
	m_CurrentlyCompilingModuleName= strTempFileName;

	for( size_t i = 0; i < filelist.size(); ++ i )
	{
		buildFileList.push_back( filelist[i] );
	}
	buildFileList.push_back( currModuleFullPath / path(L"/../RunTimeCompiler/ObjectInterfacePerModuleSource.cpp") );
	buildFileList.push_back( currModuleFullPath / path(L"/../RunTimeCompiler/ObjectInterfacePerModuleSource_PlatformWindows.cpp") );

	includeDirList.push_back( currModuleFullPath / path(L"/../RunTimeCompiler/") );
	includeDirList.push_back( currModuleFullPath / path(L"/../Systems/") );
	includeDirList.push_back( currModuleFullPath / path(L"/../Common/") );
	includeDirList.push_back( currModuleFullPath / path(L"/../Renderer/") );

	m_pBuildTool->BuildModule( buildFileList, includeDirList, m_CurrentlyCompilingModuleName, bForce );
}

bool ConsoleGame::LoadCompiledModule()
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
		gSys->pLogSystem->Log(eLV_ERRORS,"Failed to load module %ls\n",m_CurrentlyCompilingModuleName.c_str());
		return false;
	}

	GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd = (GETPerModuleInterface_PROC) GetProcAddress(module, "GetPerModuleInterface");
	if (!pPerModuleInterfaceProcAdd)
	{
		gSys->pLogSystem->Log(eLV_ERRORS,"Failed GetProcAddress\n");
		return false;
	}

	pPerModuleInterfaceProcAdd()->SetSystemTable( gSys );
	m_Modules.push_back( module );

	gSys->pLogSystem->Log(eLV_COMMENTS, "Compilation Succeeded\n");

	SetupObjectConstructors(pPerModuleInterfaceProcAdd);

	m_bHaveProgramError = false; //reset

	return true;
}

void ConsoleGame::SetupObjectConstructors(GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd)
{
	// get hold of the constructors
	const std::vector<IObjectConstructor*> &objectConstructors = pPerModuleInterfaceProcAdd()->GetConstructors();
	AUDynArray<IObjectConstructor*> constructors( objectConstructors.size() );
	for (size_t i=0, iMax=objectConstructors.size(); i<iMax; ++i)
	{
		constructors[i] = objectConstructors[i];
		// Add to runtime file list if it's not already there (and not a ".h" file - temp solution until proper dependency system is in place)
		path filename = objectConstructors[i]->GetFileName();
		if ( m_RuntimeFileList.end() == std::find(m_RuntimeFileList.begin(), m_RuntimeFileList.end(), filename) )
		{
			// Start watching all the files involved, to trigger recompile
			gSys->pFileChangeNotifier->Watch( objectConstructors[i]->GetFileName(), this );

			if (filename.extension() != ".h")
			{
				m_RuntimeFileList.push_back( filename );
			}		
		}		
	}
	gSys->pObjectFactorySystem->AddConstructors( constructors );
}
