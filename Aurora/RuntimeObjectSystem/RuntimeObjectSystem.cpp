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
	, m_pObjectFactorySystem(new ObjectFactorySystem())
	, m_pFileChangeNotifier(new FileChangeNotifier())
	, m_pBuildTool(new BuildTool())
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

	m_pBuildTool->Initialise(m_pCompilerLogger);

	// We start by using the code in the current module
	IPerModuleInterface* pPerModuleInterface = PerModuleInterface::GetInstance();
    pPerModuleInterface->SetModuleFileName( "Main Exe" );
    pPerModuleInterface->SetSystemTable( m_pSystemTable );

	m_pObjectFactorySystem->SetLogger( m_pCompilerLogger );
    m_pObjectFactorySystem->SetRuntimeObjectSystem( this );

    FileSystemUtils::Path initialDir = FileSystemUtils::GetCurrentPath();
    m_FoundSourceDirectoryMappings[initialDir] = initialDir;

	SetupObjectConstructors(pPerModuleInterface);

	//add this dir to list of include dirs
	FileSystemUtils::Path includeDir = FindFile(__FILE__);
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


	m_pCompilerLogger->LogInfo("FileChangeNotifier triggered recompile with changes to:\n");
	for( size_t i = 0; i < filelist.Size(); ++ i )
	{
	    m_pCompilerLogger->LogInfo("    File %s\n",filelist[i]);
		BuildTool::FileToBuild fileToBuild(filelist[i]);

        bool bFindIncludeDependencies = true;  // if this is a header or a source dependency need to find include dependencies
        bool bForceIncludeDependencies = true;
		if( fileToBuild.filePath.Extension() != ".h") //TODO: change to check for .cpp and .c as could have .inc files etc.?
		{
            bFindIncludeDependencies = false;
            // file may be a source dependency, check
			TFileToFilesIterator itrCurr = m_RuntimeSourceDependencyMap.begin();
			while( itrCurr != m_RuntimeSourceDependencyMap.end() )
			{
				if( itrCurr->second == fileToBuild.filePath )
				{
                    fileToBuild.filePath.ReplaceExtension(".h");
                    bFindIncludeDependencies = true;
                    bForceIncludeDependencies = false; // a src change, not a header change - so no need to force compile (can just link object file if exists)
                    break;
				}
				else
				{
					++itrCurr;
				}
			}

            if( !bFindIncludeDependencies )
            {
    			pBuildFileList->push_back( fileToBuild );
            }
		}

		if( bFindIncludeDependencies )
		{
			TFileToFilesEqualRange range = m_RuntimeIncludeMap.equal_range( fileToBuild.filePath );
			for(TFileToFilesIterator it=range.first; it!=range.second; ++it)
			{
				BuildTool::FileToBuild fileToBuildFromIncludes( (*it).second, bForceIncludeDependencies );
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

	if (m_bAutoCompile)
	{
		AUDynArray<IObjectConstructor*> constructors;
		m_pObjectFactorySystem->GetAll(constructors);
		SetupRuntimeFileTracking(constructors);
	}
}

// RuntimeObjectSystem::AddToRuntimeFileList - filename should be cleaned of "/../" etc, see FileSystemUtils::Path::GetCleanPath()
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

		TFileToFilesEqualRange range = m_RuntimeLinkLibraryMap.equal_range( ourBuildFileList[i].filePath );
		for(TFileToFilesIterator it=range.first; it!=range.second; ++it)
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
        fullpath = FindFile( fullpath );
		BuildTool::FileToBuild reqFile( fullpath, false );	//don't force compile of these
		ourBuildFileList.push_back( reqFile );
	}

    //Add dependency source files
    size_t buildListSize = ourBuildFileList.size(); // we will add to the build list, so get the size before the loop
	for( size_t i = 0; i < buildListSize; ++ i )
	{

		TFileToFilesEqualRange range = m_RuntimeSourceDependencyMap.equal_range( ourBuildFileList[i].filePath );
		for(TFileToFilesIterator it=range.first; it!=range.second; ++it)
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

	SetupObjectConstructors(pPerModuleInterfaceProcAdd());
	m_BuildFileList.clear();	// clear the files from our compile list
	m_bLastLoadModuleSuccess = true;
	if( !m_PendingBuildFileList.empty() )
	{
		// we have pending files to compile, go ahead and compile them
		StartRecompile();
	}
	return true;
}

void RuntimeObjectSystem::SetupObjectConstructors(IPerModuleInterface* pPerModuleInterface)
{

	// get hold of the constructors
	const std::vector<IObjectConstructor*> &objectConstructors = pPerModuleInterface->GetConstructors();
	AUDynArray<IObjectConstructor*> constructors(objectConstructors.size());
	for (size_t i = 0, iMax = objectConstructors.size(); i < iMax; ++i)
	{
		constructors[i] = objectConstructors[i];
	}

	if (m_bAutoCompile)
	{
		SetupRuntimeFileTracking(constructors);
	}

	m_pObjectFactorySystem->AddConstructors(constructors);

}

void RuntimeObjectSystem::SetupRuntimeFileTracking(const IAUDynArray<IObjectConstructor*>& constructors_)
{
	// for optimization purposes we skip some actions when running for the first time (i.e. no previous constructors)
	bool bFirstTime = m_RuntimeFileList.empty();

	for (size_t i = 0, iMax = constructors_.Size(); i < iMax; ++i)
	{

		Path filePath = constructors_[i]->GetFileName(); // GetFileName returns full path including GetCompiledPath()
        filePath = filePath.GetCleanPath();
        filePath = FindFile( filePath );
        AddToRuntimeFileList( filePath.c_str() );


		if( !bFirstTime )
		{
 			//remove old include file mappings for this file
			TFileToFilesIterator itrCurr = m_RuntimeIncludeMap.begin();
			while( itrCurr != m_RuntimeIncludeMap.end() )
			{
				if( itrCurr->second == filePath )
				{
                    TFileToFilesIterator itrErase = itrCurr;
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
		FileSystemUtils::Path compileDir = constructors_[i]->GetCompiledPath();

		//add include file mappings
		for (size_t includeNum = 0; includeNum <= constructors_[i]->GetMaxNumIncludeFiles(); ++includeNum)
		{
			const char* pIncludeFile = constructors_[i]->GetIncludeFile(includeNum);
			if( pIncludeFile )
			{
                FileSystemUtils::Path fullpath = compileDir / pIncludeFile;
                fullpath = FindFile( fullpath.GetCleanPath() );
				TFileToFilePair includePathPair;
				includePathPair.first = fullpath;
				includePathPair.second = filePath;
                AddToRuntimeFileList( fullpath.c_str() );
				m_RuntimeIncludeMap.insert( includePathPair );
			}
		}
            

 		//add link library file mappings
		for (size_t linklibraryNum = 0; linklibraryNum <= constructors_[i]->GetMaxNumLinkLibraries(); ++linklibraryNum)
		{
			const char* pLinkLibrary = constructors_[i]->GetLinkLibrary(linklibraryNum);
			if( pLinkLibrary )
			{
                // We do not use FindFiles for Linked Libraries as these are searched for on
                // the library paths, which are themselves searched for.
				TFileToFilePair linklibraryPathPair;
				linklibraryPathPair.first = filePath;
				linklibraryPathPair.second = pLinkLibrary;
				m_RuntimeLinkLibraryMap.insert( linklibraryPathPair );
			}
		}

        //add source dependency file mappings
		for (size_t num = 0; num <= constructors_[i]->GetMaxNumSourceDependencies(); ++num)
		{
			const char* pSourceDependency = constructors_[i]->GetSourceDependency(num);
			if( pSourceDependency )
			{
                FileSystemUtils::Path pathInc = compileDir / pSourceDependency;
                pathInc = FindFile( pathInc.GetCleanPath() );
                FileSystemUtils::Path pathSrc = pathInc;
                pathSrc.ReplaceExtension( ".cpp" );
				TFileToFilePair sourcePathPair;
				sourcePathPair.first = filePath;
				sourcePathPair.second = pathSrc;
				m_RuntimeSourceDependencyMap.insert( sourcePathPair );
                
                // if the include file with a source dependancy is logged as an runtime include, then we mark this .cpp as compile dependencies on change
                TFileToFilesEqualRange range = m_RuntimeIncludeMap.equal_range( pathInc );
                if( range.first != range.second )
                {
                    // add source file to runtime file list
                    AddToRuntimeFileList( pathSrc.c_str() );
                }
			}
		}

	}
}

void RuntimeObjectSystem::AddIncludeDir( const char *path_ )
{
	m_IncludeDirList.push_back(path_);
}


void RuntimeObjectSystem::AddLibraryDir( const char *path_ )
{
	m_LibraryDirList.push_back(path_);
}

FileSystemUtils::Path RuntimeObjectSystem::FindFile( const FileSystemUtils::Path& input )
{
    FileSystemUtils::Path requestedDirectory = input;
    FileSystemUtils::Path filename;
    FileSystemUtils::Path foundFile = input;
    bool bIsFile = input.HasExtension();
    if( bIsFile )
    {
        requestedDirectory = requestedDirectory.ParentPath();
        filename = input.Filename();
    }
    requestedDirectory.ToOSCanonicalCase();
    filename.ToOSCanonicalCase();
    foundFile.ToOSCanonicalCase();

    // Step 1: Try input directory
    if( requestedDirectory.Exists() )
    {
        m_FoundSourceDirectoryMappings[ requestedDirectory ] = requestedDirectory;
    }
    else
    {
        // Step 2: Attempt to find a pre-existing mapping
        bool bFoundMapping = false;
        if( m_FoundSourceDirectoryMappings.size() )
        {
            FileSystemUtils::Path testDir = requestedDirectory;
            FileSystemUtils::Path foundDir;
            unsigned int depth = 0;
            bool bFound = false;
            while( testDir.HasParentPath() )
            {
                TFileMapIterator itrFind = m_FoundSourceDirectoryMappings.find( testDir );
                if( itrFind != m_FoundSourceDirectoryMappings.end() )
                {
                    foundDir = itrFind->second;
                    bFound = true;
                    break;
                }

                testDir = testDir.ParentPath();
                ++depth;
            }

            if( bFound )
            {
                if( depth )
                {
                    // not an exact match
                    FileSystemUtils::Path directory = requestedDirectory;
                    directory.m_string.replace( 0, testDir.m_string.length(), foundDir.m_string );
                    if( directory.Exists() )
                    {
                        foundFile = directory / filename;
                        if( foundFile.Exists() )
                        {
                            m_FoundSourceDirectoryMappings[ requestedDirectory ] = directory;
                            if( m_pCompilerLogger ) {  m_pCompilerLogger->LogInfo( "Found Directory Mapping: %s to %s\n", requestedDirectory.c_str(), directory.c_str() ); }
                            bFoundMapping = true;
                        }
                    }

                }
                else
                {
                    // exact match
                    foundFile = foundDir / filename;
                    bFoundMapping = true;
                }
            }
            
            if( !bFoundMapping )
            {
                // Step 3: Attempt to find a mapping from a known path
                TFileList requestedSubPaths;
                FileSystemUtils::Path requestedSubPath = requestedDirectory;
                while( requestedSubPath.HasParentPath() )
                {
                    requestedSubPaths.push_back( requestedSubPath );
                    requestedSubPath = requestedSubPath.ParentPath();
                }

                TFileMapIterator itr = m_FoundSourceDirectoryMappings.begin();
                while( ( itr != m_FoundSourceDirectoryMappings.end() ) && !bFoundMapping )
                {
                    FileSystemUtils::Path existingPath = itr->second;
                    while( ( existingPath.HasParentPath() ) && !bFoundMapping )
                    {
                        // check all potentials
                        for( size_t i=0; i<requestedSubPaths.size(); ++i )
                        {
                            FileSystemUtils::Path toCheck = existingPath / requestedSubPaths[i].Filename();
                            if( toCheck.Exists() )
                            {
                                // potential mapping
                                FileSystemUtils::Path directory = requestedDirectory;
                                directory.m_string.replace( 0, requestedSubPaths[i].m_string.length(), toCheck.m_string );
                                if( directory.Exists() )
                                {
                                    foundFile = directory / filename;
                                    if( foundFile.Exists() )
                                    {
                                        m_FoundSourceDirectoryMappings[ requestedDirectory ] = directory;
                                        if( m_pCompilerLogger ) {  m_pCompilerLogger->LogInfo( "Found Directory Mapping: %s to %s\n", requestedDirectory.c_str(), directory.c_str() ); }
                                        bFoundMapping = true;
                                        break;
                                    }
                                }
                            }
                        }
                        existingPath = existingPath.ParentPath();
                    }
                    ++itr;
                }
            }
        }
    }

    if( !foundFile.Exists() )
    {
        if( m_pCompilerLogger ) {  m_pCompilerLogger->LogWarning( "Could not find Directory Mapping for: %s\n", input.c_str() ); }
        ++m_NumNotFoundSourceFiles;
    }
    return foundFile;
}


void RuntimeObjectSystem::AddPathToSourceSearch( const char* path )
{
    m_FoundSourceDirectoryMappings[ path ] = path;
}


bool RuntimeObjectSystem::TestBuildCallback(const char* file, TestBuildResult type)
{
    switch( type )
    {
    case TESTBUILDRRESULT_SUCCESS:            // SUCCESS, yay!
        if( m_pCompilerLogger ) { m_pCompilerLogger->LogInfo("TESTBUILDRRESULT_SUCCESS: %s\n", file); }
        break;
    case TESTBUILDRRESULT_NO_FILES_TO_BUILD:  // file registration error or no runtime files of this type
        if( m_pCompilerLogger ) { m_pCompilerLogger->LogWarning("TESTBUILDRRESULT_NO_FILES_TO_BUILD\n"); }
        break;
    case TESTBUILDRRESULT_BUILD_FILE_GONE:    // the file is no longer present
        if( m_pCompilerLogger ) { m_pCompilerLogger->LogError("TESTBUILDRRESULT_BUILD_FILE_GONE: %s\n", file); }
        break;
    case TESTBUILDRRESULT_BUILD_NOT_STARTED:  // file change detection could be broken, or if an include may not be included anywhere
        if( m_pCompilerLogger ) { m_pCompilerLogger->LogError("TESTBUILDRRESULT_BUILD_NOT_STARTED: %s\n", file); }
        break;
    case TESTBUILDRRESULT_BUILD_FAILED:       // a build was started, but it failed or module failed to load. See log.
        if( m_pCompilerLogger ) { m_pCompilerLogger->LogError("TESTBUILDRRESULT_BUILD_FAILED: %s\n", file); }
        break;
    case TESTBUILDRRESULT_OBJECT_SWAP_FAIL:   // build succeeded, module loaded but errors on swapping
        if( m_pCompilerLogger ) { m_pCompilerLogger->LogError("TESTBUILDRRESULT_OBJECT_SWAP_FAIL: %s\n", file); }
        break;
    default:
        assert(false);
        break;
    }
    return true;
}

// returns 0 on success, -ve number of errors if there is an error and we should quit,
// positive number of errors if there is an error but we should continue
static int TestBuildFile( ICompilerLogger* pLog, RuntimeObjectSystem* pRTObjSys, const Path& file,
                          ITestBuildNotifier* callback, bool bTestFileTracking )
{
    assert( callback );

    if( pLog ) { pLog->LogInfo("Testing change to file: %s\n", file.c_str()); }

    int numErrors = 0;
    if( file.Exists() )
    {
        if( bTestFileTracking )
        {
            FileSystemUtils::filetime_t currTime = FileSystemUtils::GetCurrentTime();
            FileSystemUtils::filetime_t oldModTime = file.GetLastWriteTime();
            if( currTime == oldModTime )
            {
                // some files may be auto-generated by the program, so may have just been created so won't
                // get a time change unless we force it.
                currTime += 1;
            }
            file.SetLastWriteTime( currTime );
            // we must also change the directories time, as some of our watchers watch the dir
            Path directory = file.ParentPath();
            directory.SetLastWriteTime( currTime );
            for( int i=0; i<50; ++i )
            {
                // wait up to 100 seconds (make configurable?)
                pRTObjSys->GetFileChangeNotifier()->Update( 1.0f ); // force update by using very large time delta
                if( pRTObjSys->GetIsCompiling() ) { break; }
                if( !callback->TestBuildWaitAndUpdate() )
                {
                    return -0xD1E;
                }
            }
        }
        else
        {
            AUDynArray<const char*> filelist;
            filelist.Add( file.c_str() );
            pRTObjSys->OnFileChange( filelist );
        }
        if( pRTObjSys->GetIsCompiling() )
        {
            while( !pRTObjSys->GetIsCompiledComplete() )
            {
                if( !callback->TestBuildWaitAndUpdate() )
                {
                    return -0xD1E;
                }
            }
            int numCurrLoadedModules = pRTObjSys->GetNumberLoadedModules();
            if( pRTObjSys->LoadCompiledModule() )
            {
                if( !callback->TestBuildCallback( file.c_str(), TESTBUILDRRESULT_SUCCESS ) ) { return -0xD1E; }
                return 0;
            }
            else
            {
                ++numErrors;
                if( pRTObjSys->GetNumberLoadedModules() == numCurrLoadedModules )
                {
                    if( !callback->TestBuildCallback( file.c_str(), TESTBUILDRRESULT_BUILD_FAILED ) ) { return -numErrors; }
                }
                else
                {
                    // loaded the module but some other issue
                    if( !callback->TestBuildCallback( file.c_str(), TESTBUILDRRESULT_OBJECT_SWAP_FAIL ) ) { return -numErrors; }
                }
            }
        }
        else
        {
            ++numErrors;
           if( !callback->TestBuildCallback( file.c_str(), TESTBUILDRRESULT_BUILD_NOT_STARTED ) ) { return -numErrors; }
        }
    }
    else
    {
        ++numErrors;
        if( !callback->TestBuildCallback( file.c_str(), TESTBUILDRRESULT_BUILD_FILE_GONE ) ) { return -numErrors; }
    }
    return numErrors;
}

// tests one by one touching each runtime modifiable source file
// returns the number of errors - 0 if all passed.
int RuntimeObjectSystem::TestBuildAllRuntimeSourceFiles(  ITestBuildNotifier* callback, bool bTestFileTracking )
{
    if( m_pCompilerLogger ) { m_pCompilerLogger->LogInfo("TestBuildAllRuntimeSourceFiles Starting\n"); }
   
    ITestBuildNotifier* failCallbackLocal = callback;
    if( !failCallbackLocal )
    {
        failCallbackLocal = this;
    }

    int numErrors = 0;
    if( m_RuntimeFileList.empty() )
    {
        failCallbackLocal->TestBuildCallback( NULL, TESTBUILDRRESULT_NO_FILES_TO_BUILD );
    }
    
    TFileList filesToTest = m_RuntimeFileList; // m_RuntimeFileList could change if file content changes (new includes or source dependencies) so make copy to ensure iterators valid.
	for( TFileList::iterator it = filesToTest.begin(); it != filesToTest.end(); ++it )
    {
        const Path& file = *it;
        if( file.Extension() != ".h") // exclude headers, use TestBuildAllRuntimeHeaders
        {
            int fileErrors = TestBuildFile( m_pCompilerLogger, this, file, failCallbackLocal, bTestFileTracking );
            if( fileErrors < 0 )
            {
                // this means exit, and the number of errors is -ve so remove, unless -0xD1E is the response (for no error die)
                if( fileErrors != -0xD1E )
                {
                    numErrors -= fileErrors;
                }
                return numErrors;
            }
            numErrors += fileErrors;
        }
    }

    if( 0 == numErrors )
    {
        if( m_pCompilerLogger ) { m_pCompilerLogger->LogInfo("All Tests Passed\n"); }
    }
    else
    {
        if( m_pCompilerLogger ) { m_pCompilerLogger->LogError("Tests Failed: %d\n", numErrors); }
    }
    return numErrors;
}

// tests touching each header which has RUNTIME_MODIFIABLE_INCLUDE.
// returns the number of errors - 0 if all passed.
int RuntimeObjectSystem::TestBuildAllRuntimeHeaders(      ITestBuildNotifier* callback, bool bTestFileTracking )
{
    ITestBuildNotifier* failCallbackLocal = callback;
    if( !failCallbackLocal )
    {
        failCallbackLocal = this;
    }

    int numErrors = 0;
    if( m_RuntimeFileList.empty() )
    {
        failCallbackLocal->TestBuildCallback( NULL, TESTBUILDRRESULT_NO_FILES_TO_BUILD );
    }

	for( TFileList::iterator it = m_RuntimeFileList.begin(); it != m_RuntimeFileList.end(); ++it )
    {
        const Path& file = *it;
        if( file.Extension() == ".h") // exclude headers, use TestBuildAllRuntimeHeaders
        {
            int fileErrors = TestBuildFile( m_pCompilerLogger, this, file, failCallbackLocal, bTestFileTracking );
            if( fileErrors < 0 )
            {
                // this means exit, and the number of errors is -ve so remove
                return numErrors - fileErrors;
            }
            numErrors += fileErrors;
        }
    }

    
    if( 0 == numErrors )
    {
        if( m_pCompilerLogger ) { m_pCompilerLogger->LogInfo("All Tests Passed\n"); }
    }
    else
    {
        if( m_pCompilerLogger ) { m_pCompilerLogger->LogError("Tests Failed: %d\n", numErrors); }
    }
    return numErrors;
}
