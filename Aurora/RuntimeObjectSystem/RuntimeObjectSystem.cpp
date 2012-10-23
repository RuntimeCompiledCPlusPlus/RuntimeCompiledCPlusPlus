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
#include "../Common/AUArray.inl"
#include "../RuntimeCompiler/ICompilerLogger.h"
#include "../RuntimeCompiler/FileChangeNotifier.h"
#include "IObjectFactorySystem.h"
#include "ObjectFactorySystem/ObjectFactorySystem.h"
#include "ObjectInterfacePerModule.h"

#include "IObject.h"

#ifndef _WIN32
//TODO: fix below in a better generic fashion.
#define MAX_PATH 256
#include <dlfcn.h>
#endif

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

	m_pBuildTool = new BuildTool();
	m_pBuildTool->Initialise(m_pCompilerLogger);

	// We start by using the code in the current module
	GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd = NULL;
#ifdef _WIN32
	HMODULE module = GetModuleHandle(NULL);

	pPerModuleInterfaceProcAdd = (GETPerModuleInterface_PROC) GetProcAddress(module, "GetPerModuleInterface");
#else
    void* this_process = dlopen(NULL,0);
    pPerModuleInterfaceProcAdd = (GETPerModuleInterface_PROC) dlsym(this_process,"GetPerModuleInterface");
    
#endif

	if (!pPerModuleInterfaceProcAdd)
	{
		m_pCompilerLogger->LogError( "Failed GetProcAddress for GetPerModuleInterface in current module\n" );
		return false;
	}
    pPerModuleInterfaceProcAdd()->SetModuleFileName( "Main Exe" );
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
	std::vector<BuildTool::FileToBuild> buildFileList;

	m_pCompilerLogger->LogInfo("FileChangeNotifier triggered recompile of files:\n");
	for( size_t i = 0; i < filelist.Size(); ++ i )
	{
		BuildTool::FileToBuild fileToBuild(filelist[i]);
		if( fileToBuild.filePath.extension() != ".h") //TODO: change to check for .cpp and .c as could have .inc files etc.?
		{
			buildFileList.push_back( fileToBuild );
		}
		else
		{
			TFileToFileEqualRange range = m_RuntimeIncludeMap.equal_range( fileToBuild.filePath );
			for(TFileToFileIterator it=range.first; it!=range.second; ++it)
			{
				BuildTool::FileToBuild fileToBuildFromIncludes( (*it).second, true );
				buildFileList.push_back( fileToBuildFromIncludes );
			}
		}
	}

	StartRecompile( buildFileList );
}

bool RuntimeObjectSystem::GetIsCompiledComplete()
{
	return m_bCompiling && m_pBuildTool->GetIsComplete();
}

void RuntimeObjectSystem::CompileAll( bool bForceRecompile )
{
	std::vector<BuildTool::FileToBuild> buildFileList;
	for( size_t i = 0; i < m_RuntimeFileList.size(); ++ i )
	{
		BuildTool::FileToBuild fileToBuild(m_RuntimeFileList[i], true ); //force re-compile on compile all
		if( fileToBuild.filePath.extension() != ".h") //TODO: change to check for .cpp and .c as could have .inc files etc.?
		{
			buildFileList.push_back( fileToBuild );
		}
	}

	StartRecompile(buildFileList);
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
		m_pFileChangeNotifier->Watch( filename, this );
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

void RuntimeObjectSystem::StartRecompile( const std::vector<BuildTool::FileToBuild>& buildFileList )
{
	m_bCompiling = true;
	m_pCompilerLogger->LogInfo( "Compiling...\n");

	//Use a temporary filename for the dll
#ifdef _WIN32
	wchar_t tempPath[MAX_PATH];
	GetTempPath( MAX_PATH, tempPath );
	wchar_t tempFileName[MAX_PATH]; 
	GetTempFileName( tempPath, L"", 0, tempFileName );
	std::wstring strTempFileName( tempFileName );
	m_CurrentlyCompilingModuleName= strTempFileName;
#else
    char tempPath[] = "/tmp/RCCppTempDylibXXXXXX";
    int fileDesc = mkstemp(tempPath);
    assert( fileDesc != -1 ); //TODO: should really handle the error
    close( fileDesc ); //we don't actually want to make the file as yet
    m_CurrentlyCompilingModuleName = tempPath;
    
#endif


	std::vector<BuildTool::FileToBuild> ourBuildFileList( buildFileList );


	//Add required source files
	const std::vector<const char*> vecRequiredFiles = PerModuleInterface::GetInstance()->GetRequiredSourceFiles();
	for( size_t i = 0; i < vecRequiredFiles.size(); ++i )
	{
		BuildTool::FileToBuild reqFile( vecRequiredFiles[i], false );	//don't force compile of these
		ourBuildFileList.push_back( reqFile );
	}

	m_pBuildTool->BuildModule(	ourBuildFileList,
								m_IncludeDirList,
								m_LibraryDirList,
								m_CompileOptions.c_str(),
								m_LinkOptions.c_str(),
								m_CurrentlyCompilingModuleName );
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
#ifdef _WIN32
		module = LoadLibraryW( m_CurrentlyCompilingModuleName.c_str() );
#else
        module = dlopen( m_CurrentlyCompilingModuleName.c_str(), RTLD_NOW );
#endif
	}

	if (!module)
	{
		m_pCompilerLogger->LogError( "Failed to load module %ls\n",m_CurrentlyCompilingModuleName.wstring().c_str());
		return false;
	}

    GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd = 0;
#ifdef _WIN32
    pPerModuleInterfaceProcAdd = (GETPerModuleInterface_PROC) GetProcAddress(module, "GetPerModuleInterface");
#else
    pPerModuleInterfaceProcAdd = (GETPerModuleInterface_PROC) dlsym(module,"GetPerModuleInterface");
    
#endif
	if (!pPerModuleInterfaceProcAdd)
	{
		m_pCompilerLogger->LogError( "Failed GetProcAddress\n");
		return false;
	}

    pPerModuleInterfaceProcAdd()->SetModuleFileName( m_CurrentlyCompilingModuleName.string().c_str() );
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
	m_pObjectFactorySystem->AddConstructors( constructors );
}

void RuntimeObjectSystem::AddIncludeDir( const char *path_ )
{
	m_IncludeDirList.push_back(path(path_));
}


void RuntimeObjectSystem::AddLibraryDir( const char *path_ )
{
	m_LibraryDirList.push_back(path(path_));
}
