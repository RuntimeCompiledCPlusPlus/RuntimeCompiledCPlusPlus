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
#include <algorithm>
#include "IObject.h"

#ifndef _WIN32
//TODO: fix below in a better generic fashion.
#define MAX_PATH 256
#include <dlfcn.h>
#endif

using FileSystemUtils::Path;

RuntimeObjectSystem::RuntimeObjectSystem()
	: m_pCompilerLogger(0)
	, m_pSystemTable(0)
	, m_pObjectFactorySystem(0)
	, m_pFileChangeNotifier(0)
	, m_pBuildTool(0)
	, m_bCompiling( false )
	, m_bLastLoadModuleSuccess( false )
	, m_bAutoCompile( true )
    , m_TotalLoadedModulesEver(1) // starts at one for current exe
    , m_bProtectionEnabled( true )
    , m_pImpl( 0 )
{
    CreatePlatformImpl();
}

RuntimeObjectSystem::~RuntimeObjectSystem()
{
	m_pFileChangeNotifier->RemoveListener(this);
    DeletePlatformImpl();
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
    m_pObjectFactorySystem->SetRuntimeObjectSystem( this );

	m_pFileChangeNotifier = new FileChangeNotifier();


	SetupObjectConstructors(pPerModuleInterfaceProcAdd);

	//add this dir to list of include dirs
	FileSystemUtils::Path includeDir( __FILE__ );
	includeDir = includeDir.ParentPath();
	AddIncludeDir(includeDir.c_str());

	//also add the runtime compiler dir to list of dirs
	includeDir = includeDir.ParentPath() / Path("RuntimeCompiler");
	AddIncludeDir(includeDir.c_str());



	return true;
}


void RuntimeObjectSystem::OnFileChange(const IAUDynArray<const char*>& filelist)
{
	if( !m_bAutoCompile )
	{
		return;
	}

	std::vector<BuildTool::FileToBuild>* pBuildFileList = &m_BuildFileList;
	if( m_bCompiling )
	{
		pBuildFileList = &m_PendingBuildFileList;
	}


	m_pCompilerLogger->LogInfo("FileChangeNotifier triggered recompile of files:\n");
	for( size_t i = 0; i < filelist.Size(); ++ i )
	{
		BuildTool::FileToBuild fileToBuild(filelist[i]);
		if( fileToBuild.filePath.Extension() != ".h") //TODO: change to check for .cpp and .c as could have .inc files etc.?
		{
			pBuildFileList->push_back( fileToBuild );
		}
		else
		{
			TFileToFileEqualRange range = m_RuntimeIncludeMap.equal_range( fileToBuild.filePath );
			for(TFileToFileIterator it=range.first; it!=range.second; ++it)
			{
				BuildTool::FileToBuild fileToBuildFromIncludes( (*it).second, true );
				pBuildFileList->push_back( fileToBuildFromIncludes );
			}
		}
	}

	if( !m_bCompiling )
	{
		StartRecompile();
	}
}

bool RuntimeObjectSystem::GetIsCompiledComplete()
{
	return m_bCompiling && m_pBuildTool->GetIsComplete();
}

void RuntimeObjectSystem::CompileAll( bool bForceRecompile )
{
	// since this is a compile all we can clear any pending compiles
	m_BuildFileList.clear();

	// add all files except headers
	for( size_t i = 0; i < m_RuntimeFileList.size(); ++ i )
	{
		BuildTool::FileToBuild fileToBuild(m_RuntimeFileList[i], true ); //force re-compile on compile all
		if( fileToBuild.filePath.Extension() != ".h") //TODO: change to check for .cpp and .c as could have .inc files etc.?
		{
			m_BuildFileList.push_back( fileToBuild );
		}
	}

	StartRecompile();
}

void RuntimeObjectSystem::SetAutoCompile( bool autoCompile )
{
	m_bAutoCompile = autoCompile;
}

void RuntimeObjectSystem::AddToRuntimeFileList( const char* filename )
{
	FileSystemUtils::Path path = filename;
	path = path.GetCleanPath();
	TFileList::iterator it = std::find( m_RuntimeFileList.begin(), m_RuntimeFileList.end(), path );
	if ( it == m_RuntimeFileList.end() )
	{
		m_RuntimeFileList.push_back( path );
        m_pFileChangeNotifier->Watch( path.c_str(), this );
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

void RuntimeObjectSystem::StartRecompile()
{
	m_bCompiling = true;
	m_pCompilerLogger->LogInfo( "Compiling...\n");

	//Use a temporary filename for the dll
#ifdef _WIN32
	char tempPath[MAX_PATH];
	GetTempPathA( MAX_PATH, tempPath );
	char tempFileName[MAX_PATH]; 
	GetTempFileNameA( tempPath, "", 0, tempFileName );
	std::string strTempFileName( tempFileName );
	m_CurrentlyCompilingModuleName= strTempFileName;
#else
    char tempPath[] = "/tmp/RCCppTempDylibXXXXXX";
    int fileDesc = mkstemp(tempPath);
    assert( fileDesc != -1 ); //TODO: should really handle the error
    close( fileDesc ); //we don't actually want to make the file as yet
    m_CurrentlyCompilingModuleName = tempPath;
    
#endif


	m_BuildFileList.insert( m_BuildFileList.end(), m_PendingBuildFileList.begin(), m_PendingBuildFileList.end() );
	m_PendingBuildFileList.clear();
	std::vector<BuildTool::FileToBuild> ourBuildFileList( m_BuildFileList );


	//Add libraries which need linking
	std::vector<FileSystemUtils::Path> linkLibraryList;
	for( size_t i = 0; i < ourBuildFileList.size(); ++ i )
	{

		TFileToFileEqualRange range = m_RuntimeLinkLibraryMap.equal_range( ourBuildFileList[i].filePath );
		for(TFileToFileIterator it=range.first; it!=range.second; ++it)
		{
			linkLibraryList.push_back( it->second );
		}
	}


	//Add required source files
	const std::vector<const char*> vecRequiredFiles = PerModuleInterface::GetInstance()->GetRequiredSourceFiles();
	FileSystemUtils::Path compileDir = PerModuleInterface::GetInstance()->GetCompiledPath();
	for( size_t i = 0; i < vecRequiredFiles.size(); ++i )
	{
		FileSystemUtils::Path fullpath = compileDir / vecRequiredFiles[i];
		BuildTool::FileToBuild reqFile( fullpath, false );	//don't force compile of these
		ourBuildFileList.push_back( reqFile );
	}

    //Add dependency source files
    size_t buildListSize = ourBuildFileList.size(); // we will add to the build list, so get the size before the loop
	for( size_t i = 0; i < buildListSize; ++ i )
	{

		TFileToFileEqualRange range = m_RuntimeSourceDependencyMap.equal_range( ourBuildFileList[i].filePath );
		for(TFileToFileIterator it=range.first; it!=range.second; ++it)
		{
		    BuildTool::FileToBuild reqFile( it->second, false );	//don't force compile of these
			ourBuildFileList.push_back( reqFile );
		}
	}



	m_pBuildTool->BuildModule(	ourBuildFileList,
								m_IncludeDirList,
								m_LibraryDirList,
								linkLibraryList,
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
	uint64_t sizeOfModule = m_CurrentlyCompilingModuleName.GetFileSize();

	HMODULE module = 0;
	if( sizeOfModule )
	{
#ifdef _WIN32
		module = LoadLibraryA( m_CurrentlyCompilingModuleName.c_str() );
#else
        module = dlopen( m_CurrentlyCompilingModuleName.c_str(), RTLD_NOW );
#endif
	}

	if (!module)
	{
		m_pCompilerLogger->LogError( "Failed to load module %s\n",m_CurrentlyCompilingModuleName.c_str());
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

    pPerModuleInterfaceProcAdd()->SetModuleFileName( m_CurrentlyCompilingModuleName.c_str() );
	pPerModuleInterfaceProcAdd()->SetSystemTable( m_pSystemTable );
	m_Modules.push_back( module );

	m_pCompilerLogger->LogInfo( "Compilation Succeeded\n");
    ++m_TotalLoadedModulesEver;

	SetupObjectConstructors(pPerModuleInterfaceProcAdd);
	m_BuildFileList.clear();	// clear the files from our compile list
	m_bLastLoadModuleSuccess = true;
	if( !m_PendingBuildFileList.empty() )
	{
		// we have pending files to compile, go ahead and compile them
		StartRecompile();
	}
	return true;
}

void RuntimeObjectSystem::SetupObjectConstructors(GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd)
{
	// for optimization purposes we skip some actions when running for the first time (i.e. no previous constructors)
	bool bFirstTime = m_RuntimeFileList.empty();
	

	// get hold of the constructors
	const std::vector<IObjectConstructor*> &objectConstructors = pPerModuleInterfaceProcAdd()->GetConstructors();
	AUDynArray<IObjectConstructor*> constructors( objectConstructors.size() );
	for (size_t i=0, iMax=objectConstructors.size(); i<iMax; ++i)
	{
		constructors[i] = objectConstructors[i];
		AddToRuntimeFileList( objectConstructors[i]->GetFileName() );

		Path filePath = objectConstructors[i]->GetFileName();
		if( !bFirstTime )
		{
 			//remove old include file mappings for this file
			TFileToFileIterator itrCurr = m_RuntimeIncludeMap.begin();
			while( itrCurr != m_RuntimeIncludeMap.end() )
			{
				if( itrCurr->second == filePath )
				{
                    TFileToFileIterator itrErase = itrCurr;
                    ++itrCurr;
					m_RuntimeIncludeMap.erase( itrErase );
				}
				else
				{
					++itrCurr;
				}
			}

            //remove previous link libraries for this file
            m_RuntimeLinkLibraryMap.erase( filePath );

            //remove previous source dependencies
            m_RuntimeSourceDependencyMap.erase( filePath );
		}

        //we need the compile path for some platforms where the __FILE__ path is relative to the compile path
        FileSystemUtils::Path compileDir = PerModuleInterface::GetInstance()->GetCompiledPath();

		//add include file mappings
		for( size_t includeNum = 0; includeNum <= objectConstructors[i]->GetMaxNumIncludeFiles(); ++includeNum )
		{
			const char* pIncludeFile = objectConstructors[i]->GetIncludeFile( includeNum );
			if( pIncludeFile )
			{
                FileSystemUtils::Path fullpath = compileDir / pIncludeFile;
				TFileToFilePair includePathPair;
				includePathPair.first = fullpath;
				includePathPair.second = filePath;
                AddToRuntimeFileList( fullpath.c_str() );
				m_RuntimeIncludeMap.insert( includePathPair );
			}
		}
            

 		//add link library file mappings
		for( size_t linklibraryNum = 0; linklibraryNum <= objectConstructors[i]->GetMaxNumLinkLibraries(); ++linklibraryNum )
		{
			const char* pLinkLibrary = objectConstructors[i]->GetLinkLibrary( linklibraryNum );
			if( pLinkLibrary )
			{
				TFileToFilePair linklibraryPathPair;
				linklibraryPathPair.first = filePath;
				linklibraryPathPair.second = pLinkLibrary;
				m_RuntimeLinkLibraryMap.insert( linklibraryPathPair );
			}
		}

        //add source dependency file mappings
        for( size_t num = 0; num <= objectConstructors[i]->GetMaxNumSourceDependencies(); ++num )
		{
			const char* pSourceDependency = objectConstructors[i]->GetSourceDependency( num );
			if( pSourceDependency )
			{
                FileSystemUtils::Path path = compileDir / pSourceDependency;
                path.ReplaceExtension( ".cpp" );
				TFileToFilePair sourcePathPair;
				sourcePathPair.first = filePath;
				sourcePathPair.second = path;
				m_RuntimeSourceDependencyMap.insert( sourcePathPair );
			}
		}

	}
	m_pObjectFactorySystem->AddConstructors( constructors );
}

void RuntimeObjectSystem::AddIncludeDir( const char *path_ )
{
	m_IncludeDirList.push_back(Path(path_));
}


void RuntimeObjectSystem::AddLibraryDir( const char *path_ )
{
	m_LibraryDirList.push_back(Path(path_));
}
