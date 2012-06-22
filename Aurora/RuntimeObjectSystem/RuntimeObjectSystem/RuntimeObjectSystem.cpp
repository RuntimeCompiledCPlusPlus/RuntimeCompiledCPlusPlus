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

#include "RuntimeObjectSystem.h"

// Remove windows.h define of GetObject which conflicts with EntitySystem GetObject
#if defined _WINDOWS_ && defined GetObject
#undef GetObject
#endif
#include "../../Common/AUArray.inl"
#include "../../RunTimeCompiler/BuildTool.h"
#include "../../RuntimeCompiler/ICompilerLogger.h"
#include "../../RuntimeCompiler/FileChangeNotifier.h"
#include "../IObjectFactorySystem.h"
#include "../ObjectFactorySystem/ObjectFactorySystem.h"


#include "../IObject.h"

using boost::filesystem::path;

RuntimeObjectSystem::RuntimeObjectSystem()
	: m_pCompilerLogger(0)
	, m_pBuildTool(0)
	, m_bCompiling( false )
	, m_bLastLoadModuleSuccess( false )
	, m_bAutoCompile( true )
	, m_pObjectFactorySystem(0)
	, m_pFileChangeNotifier(0)
{
}

RuntimeObjectSystem::~RuntimeObjectSystem()
{
	m_pFileChangeNotifier->RemoveListener(this);

	delete m_pObjectFactorySystem;
	delete m_pFileChangeNotifier;
	delete m_pBuildTool;

	// Note we do not delete compiler logger, creator should do this
}


bool RuntimeObjectSystem::Initialise( ICompilerLogger * pLogger, SystemTable* pSystemTable  )
{
	m_pCompilerLogger = pLogger;
	m_pSystemTable = pSystemTable;

	// We need the current directory to be the process dir
	DWORD size = MAX_PATH;
	wchar_t filename[MAX_PATH];
	GetModuleFileName( NULL, filename, size );
	std::wstring strTempFileName( filename );
	path launchPath( strTempFileName );
	launchPath = launchPath.parent_path();
	SetCurrentDirectory( launchPath.wstring().c_str() );

	m_pBuildTool = new BuildTool();
	m_pBuildTool->Initialise(m_pCompilerLogger);

	// We start by using the code in the current module
	HMODULE module = GetModuleHandle(NULL);

	GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd = NULL;
	pPerModuleInterfaceProcAdd = (GETPerModuleInterface_PROC) GetProcAddress(module, "GetPerModuleInterface");
	if (!pPerModuleInterfaceProcAdd)
	{
		m_pCompilerLogger->LogError( "Failed GetProcAddress for GetPerModuleInterface in current module\n" );
		return false;
	}
	pPerModuleInterfaceProcAdd()->SetSystemTable( m_pSystemTable );

	m_pObjectFactorySystem = new ObjectFactorySystem();
	m_pObjectFactorySystem->SetLogger( m_pCompilerLogger );

	m_pFileChangeNotifier = new FileChangeNotifier();


	SetupObjectConstructors(pPerModuleInterfaceProcAdd);

	return true;
}


void RuntimeObjectSystem::OnFileChange(const IAUDynArray<const char*>& filelist)
{

	if( !m_bAutoCompile )
	{
		return;
	}

	m_pCompilerLogger->LogInfo("FileChangeNotifier triggered recompile of files:\n");

	TFileList pathlist;
	pathlist.resize(filelist.Size());
	for( size_t i = 0; i < filelist.Size(); ++ i )
	{
		pathlist[i] = path(filelist[i]);
	}

	StartRecompile(pathlist, false);
}

bool RuntimeObjectSystem::GetIsCompiledComplete()
{
	return m_bCompiling && m_pBuildTool->GetIsComplete();
}

void RuntimeObjectSystem::CompileAll( bool bForceRecompile )
{
	StartRecompile(m_RuntimeFileList, bForceRecompile);
}

void RuntimeObjectSystem::SetAutoCompile( bool autoCompile )
{
	m_bAutoCompile = autoCompile;
}

void RuntimeObjectSystem::AddToRuntimeFileList( const char* filename )
{
	TFileList::iterator it = std::find( m_RuntimeFileList.begin(), m_RuntimeFileList.end(), filename );
	if ( it == m_RuntimeFileList.end() )
	{
		m_RuntimeFileList.push_back( filename );
	}
}

void RuntimeObjectSystem::RemoveFromRuntimeFileList( const char* filename )
{
	TFileList::iterator it = std::find( m_RuntimeFileList.begin(), m_RuntimeFileList.end(), filename );
	if ( it != m_RuntimeFileList.end() )
	{
		m_RuntimeFileList.erase( it );
	}
}

void RuntimeObjectSystem::StartRecompile(const TFileList& filelist, bool bForce)
{
	m_bCompiling = true;
	m_pCompilerLogger->LogInfo( "Compiling...\n");

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
	std::vector<path> includeDirList; //we don't need any include paths for this example
	m_CurrentlyCompilingModuleName= strTempFileName;

	for( size_t i = 0; i < filelist.size(); ++ i )
	{
		buildFileList.push_back( BuildTool::FileToBuild( filelist[i], bForce ) );
	}

	buildFileList.push_back( currModuleFullPath / path(L"/../RuntimeObjectSystem/ObjectInterfacePerModuleSource.cpp") );
	buildFileList.push_back( currModuleFullPath / path(L"/../RuntimeObjectSystem/ObjectInterfacePerModuleSource_PlatformWindows.cpp") );

	m_pBuildTool->BuildModule( buildFileList, includeDirList, m_CurrentlyCompilingModuleName );
}

bool RuntimeObjectSystem::LoadCompiledModule()
{
	m_bLastLoadModuleSuccess = false;
	m_bCompiling = false;

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
		m_pCompilerLogger->LogError( "Failed to load module %ls\n",m_CurrentlyCompilingModuleName.c_str());
		return false;
	}

	GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd = (GETPerModuleInterface_PROC) GetProcAddress(module, "GetPerModuleInterface");
	if (!pPerModuleInterfaceProcAdd)
	{
		m_pCompilerLogger->LogError( "Failed GetProcAddress\n");
		return false;
	}

	pPerModuleInterfaceProcAdd()->SetSystemTable( m_pSystemTable );
	m_Modules.push_back( module );

	m_pCompilerLogger->LogInfo( "Compilation Succeeded\n");

	SetupObjectConstructors(pPerModuleInterfaceProcAdd);

	m_bLastLoadModuleSuccess = true;
	return true;
}

void RuntimeObjectSystem::SetupObjectConstructors(GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd)
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
			m_pFileChangeNotifier->Watch( objectConstructors[i]->GetFileName(), this );

			if (filename.extension() != ".h")
			{
				m_RuntimeFileList.push_back( filename );
			}		
		}		
	}
	m_pObjectFactorySystem->AddConstructors( constructors );
}
