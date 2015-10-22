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
#include "../RuntimeCompiler/AUArray.h"
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

FileSystemUtils::Path RuntimeObjectSystem::ProjectSettings::ms_DefaultIntermediatePath;

static Path GetIntermediateFolder( Path basePath_, RCppOptimizationLevel optimizationLevel_ )
{
	std::string folder;
#ifdef _DEBUG
	folder = "DEBUG_";
#else
	folder = "RELEASE_";
#endif
	folder +=  RCppOptimizationLevelStrings[ GetActualOptimizationLevel( optimizationLevel_ ) ];
	Path runtimeFolder = basePath_ / folder;
	return runtimeFolder;
}


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
    , m_CurrentlyBuildingProject( 0 )
{
    ProjectSettings::ms_DefaultIntermediatePath = FileSystemUtils::GetCurrentPath() / "Runtime";
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

    for( unsigned short proj = 0; proj < m_Projects.size(); ++proj )
    {

        std::vector<BuildTool::FileToBuild>* pBuildFileList = &m_Projects[ proj ].m_BuildFileList;
        if( m_bCompiling )
        {
            pBuildFileList = &m_Projects[ proj ].m_PendingBuildFileList;
        }


        if (m_pCompilerLogger) { m_pCompilerLogger->LogInfo( "FileChangeNotifier triggered recompile with changes to:\n" ); }
        for( size_t i = 0; i < filelist.Size(); ++i )
        {
            // check this file is in our project list
            TFileList::iterator it = std::find( m_Projects[ proj ].m_RuntimeFileList.begin( ), m_Projects[ proj ].m_RuntimeFileList.end( ), filelist[i] );
            if( it == m_Projects[ proj ].m_RuntimeFileList.end() )
            {
                continue;
            }

            if (m_pCompilerLogger) { m_pCompilerLogger->LogInfo( "    File %s\n", filelist[ i ] ); }
            BuildTool::FileToBuild fileToBuild( filelist[ i ] );

            bool bFindIncludeDependencies = true;  // if this is a header or a source dependency need to find include dependencies
            bool bForceIncludeDependencies = true;
            if( fileToBuild.filePath.Extension() != ".h" ) //TODO: change to check for .cpp and .c as could have .inc files etc.?
            {
                bFindIncludeDependencies = false;
                pBuildFileList->push_back( fileToBuild );

                // file may be a source dependency, check
                TFileToFilesIterator itrCurr = m_Projects[ proj ].m_RuntimeSourceDependencyMap.begin( );
                while( itrCurr != m_Projects[ proj ].m_RuntimeSourceDependencyMap.end( ) )
                {
                    if( itrCurr->second == fileToBuild.filePath )
                    {
                        BuildTool::FileToBuild fileToBuild( itrCurr->first );
                        pBuildFileList->push_back( fileToBuild );
                    }
                    ++itrCurr;
                }
           }

            if( bFindIncludeDependencies )
            {
                TFileToFilesEqualRange range = m_Projects[ proj ].m_RuntimeIncludeMap.equal_range( fileToBuild.filePath );
                for( TFileToFilesIterator it = range.first; it != range.second; ++it )
                {
                    BuildTool::FileToBuild fileToBuildFromIncludes( ( *it ).second, bForceIncludeDependencies );
                    pBuildFileList->push_back( fileToBuildFromIncludes );
                }
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

void RuntimeObjectSystem::CompileAllInProject( bool bForceRecompile, unsigned short projectId_ )
{
    ProjectSettings& project = GetProject( projectId_ );
    // since this is a compile all we can clear any pending compiles
    project.m_BuildFileList.clear( );

	// ensure we have an up to date list of files to commpile if autocompile is off
	if( !m_bAutoCompile )
	{
		AUDynArray<IObjectConstructor*> constructors;
		m_pObjectFactorySystem->GetAll(constructors);
		SetupRuntimeFileTracking(constructors);
	}

	// add all files except headers
    for( size_t i = 0; i < m_Projects[ projectId_ ].m_RuntimeFileList.size( ); ++i )
	{
        BuildTool::FileToBuild fileToBuild( project.m_RuntimeFileList[ i ], true ); //force re-compile on compile all
		if( fileToBuild.filePath.Extension() != ".h") //TODO: change to check for .cpp and .c as could have .inc files etc.?
		{
            project.m_BuildFileList.push_back( fileToBuild );
		}
	}

	StartRecompile();
}

void RuntimeObjectSystem::CompileAll( bool bForceRecompile )
{
    for( unsigned short proj = 0; proj < m_Projects.size(); ++proj )
    {
        CompileAllInProject( bForceRecompile, proj );
    }
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
void RuntimeObjectSystem::AddToRuntimeFileList( const char* filename, unsigned short projectId_ )
{
    ProjectSettings& project = GetProject( projectId_ );
    TFileList::iterator it = std::find( project.m_RuntimeFileList.begin( ), project.m_RuntimeFileList.end( ), filename );
    if( it == project.m_RuntimeFileList.end( ) )
	{
        project.m_RuntimeFileList.push_back( filename );
        m_pFileChangeNotifier->Watch( filename, this );
	}
}

void RuntimeObjectSystem::RemoveFromRuntimeFileList( const char* filename, unsigned short projectId_ )
{
    ProjectSettings& project = GetProject( projectId_ );
    TFileList::iterator it = std::find( project.m_RuntimeFileList.begin( ), project.m_RuntimeFileList.end( ), filename );
    if( it != project.m_RuntimeFileList.end( ) )
	{
        project.m_RuntimeFileList.erase( it );
	}
}

void RuntimeObjectSystem::StartRecompile()
{
    m_bCompiling = true;
    if( m_pCompilerLogger ) { m_pCompilerLogger->LogInfo( "Compiling...\n" ); }

    //Use a temporary filename for the dll
#ifdef _WIN32
    char tempPath[ MAX_PATH ];
    GetTempPathA( MAX_PATH, tempPath );
    char tempFileName[ MAX_PATH ];
    GetTempFileNameA( tempPath, "", 0, tempFileName );
    std::string strTempFileName( tempFileName );
    m_CurrentlyCompilingModuleName = strTempFileName;
#else
    char tempPath[] = "/tmp/RCCppTempDylibXXXXXX";
    int fileDesc = mkstemp(tempPath);
    assert( fileDesc != -1 ); //TODO: should really handle the error
    close( fileDesc ); //we don't actually want to make the file as yet
    m_CurrentlyCompilingModuleName = tempPath;

#endif

    // we step through and build each project in turn if there is anything to build, starting from m_PreviousBuildProject
    bool bHaveProjectToBuild = false;
    unsigned short project = m_CurrentlyBuildingProject;
    if( m_Projects.size( ) )
    {
        do
        {
            ++project;
            if( project >= m_Projects.size( ) )
            {
                project = 0;
            }
            if( m_Projects[ project ].m_BuildFileList.size() || m_Projects[ project ].m_PendingBuildFileList.size() )
            {
                bHaveProjectToBuild = true;
            }
        } while( !bHaveProjectToBuild && m_CurrentlyBuildingProject != project );
        m_CurrentlyBuildingProject = project;
    }

    if( !bHaveProjectToBuild )
    {
        if( m_pCompilerLogger ) { m_pCompilerLogger->LogError( "Error - could not find any files to build in any projects\n" ); }
        m_bCompiling = false;
        return;
    }

    m_Projects[ project ].m_BuildFileList.insert( m_Projects[ project ].m_BuildFileList.end( ), m_Projects[ project ].m_PendingBuildFileList.begin( ), m_Projects[ project ].m_PendingBuildFileList.end( ) );
    m_Projects[ project ].m_PendingBuildFileList.clear( );
    std::vector<BuildTool::FileToBuild> ourBuildFileList( m_Projects[ project ].m_BuildFileList );


	//Add libraries which need linking
	std::vector<FileSystemUtils::Path> linkLibraryList;
	for( size_t i = 0; i < ourBuildFileList.size(); ++ i )
	{

        TFileToFilesEqualRange range = m_Projects[ project ].m_RuntimeLinkLibraryMap.equal_range( ourBuildFileList[ i ].filePath );
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

        TFileToFilesEqualRange range = m_Projects[ project ].m_RuntimeSourceDependencyMap.equal_range( ourBuildFileList[ i ].filePath );
		for(TFileToFilesIterator it=range.first; it!=range.second; ++it)
		{
		    BuildTool::FileToBuild reqFile( it->second, false );	//don't force compile of these
			ourBuildFileList.push_back( reqFile );
		}
	}

	m_Projects[ project ].m_CompilerOptions.intermediatePath = GetIntermediateFolder(	m_Projects[ project ].m_CompilerOptions.baseIntermediatePath,
																						m_Projects[ project ].m_CompilerOptions.optimizationLevel );


    m_pBuildTool->BuildModule(  ourBuildFileList,
                                m_Projects[ project ].m_CompilerOptions,
								linkLibraryList, m_CurrentlyCompilingModuleName );
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
		if (m_pCompilerLogger) { m_pCompilerLogger->LogError( "Failed to load module %s\n",m_CurrentlyCompilingModuleName.c_str()); }
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
		if (m_pCompilerLogger) { m_pCompilerLogger->LogError( "Failed GetProcAddress\n"); }
		return false;
	}

    pPerModuleInterfaceProcAdd()->SetModuleFileName( m_CurrentlyCompilingModuleName.c_str() );
    pPerModuleInterfaceProcAdd( )->SetProjectIdForAllConstructors( m_CurrentlyBuildingProject );
    m_Modules.push_back( module );

	if (m_pCompilerLogger) { m_pCompilerLogger->LogInfo( "Compilation Succeeded\n"); }
    ++m_TotalLoadedModulesEver;

	SetupObjectConstructors(pPerModuleInterfaceProcAdd());
    m_Projects[ m_CurrentlyBuildingProject ].m_BuildFileList.clear( );	// clear the files from our compile list
	m_bLastLoadModuleSuccess = true;

    // check if there is another project to build
    bool bNeedAnotherCompile = false;
    for( unsigned short proj = 0; proj < m_Projects.size( ); ++proj )
    {
        if( m_Projects[ proj ].m_BuildFileList.size( ) || m_Projects[ proj ].m_PendingBuildFileList.size( ) )
        {
            bNeedAnotherCompile = true;
        }
    }

    if( bNeedAnotherCompile )//
	{
		// we have pending files to compile, go ahead and compile them
		StartRecompile();
	}
	return true;
}

void RuntimeObjectSystem::SetupObjectConstructors(IPerModuleInterface* pPerModuleInterface)
{
    // Set system Table
    pPerModuleInterface->SetSystemTable( m_pSystemTable );

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
#ifndef RCCPPOFF
	// for optimization purposes we skip some actions when running for the first time (i.e. no previous constructors)
	static bool bFirstTime = true;

	for (size_t i = 0, iMax = constructors_.Size(); i < iMax; ++i)
	{
		const char* pFilename = constructors_[i]->GetFileName(); // GetFileName returns full path including GetCompiledPath()
		if( !pFilename )
		{
			continue;
		}
		Path filePath = pFilename;
        filePath = filePath.GetCleanPath();
        filePath = FindFile( filePath );

        unsigned short projectId = constructors_[ i ]->GetProjectId();
        ProjectSettings& project = GetProject( projectId );
        AddToRuntimeFileList( filePath.c_str( ), projectId );

		if( !bFirstTime )
		{
 			//remove old include file mappings for this file
            TFileToFilesIterator itrCurr = project.m_RuntimeIncludeMap.begin( );
            while( itrCurr != project.m_RuntimeIncludeMap.end( ) )
			{
				if( itrCurr->second == filePath )
				{
                    TFileToFilesIterator itrErase = itrCurr;
                    ++itrCurr;
                    project.m_RuntimeIncludeMap.erase( itrErase );
				}
				else
				{
					++itrCurr;
				}
			}

            //remove previous link libraries for this file
            project.m_RuntimeLinkLibraryMap.erase( filePath );

            //remove previous source dependencies
            project.m_RuntimeSourceDependencyMap.erase( filePath );
		}

        //we need the compile path for some platforms where the __FILE__ path is relative to the compile path
		FileSystemUtils::Path compileDir = constructors_[i]->GetCompiledPath();

		//add include file mappings
		for (size_t includeNum = 0; includeNum <= constructors_[i]->GetMaxNumIncludeFiles(); ++includeNum)
		{
			const char* pIncludeFile = constructors_[i]->GetIncludeFile(includeNum);
			if( pIncludeFile )
			{
                FileSystemUtils::Path pathInc = compileDir / pIncludeFile;
                pathInc = FindFile( pathInc.GetCleanPath() );
				TFileToFilePair includePathPair;
				includePathPair.first = pathInc;
				includePathPair.second = filePath;
                AddToRuntimeFileList( pathInc.c_str(), projectId );
                project.m_RuntimeIncludeMap.insert( includePathPair );
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
                project.m_RuntimeLinkLibraryMap.insert( linklibraryPathPair );
			}
		}

        //add source dependency file mappings
		for (size_t num = 0; num <= constructors_[i]->GetMaxNumSourceDependencies(); ++num)
		{
			SourceDependencyInfo sourceDependency = constructors_[i]->GetSourceDependency(num);
			FileSystemUtils::Path pathInc[2];	// array of potential include files for later checks
			if( sourceDependency.filename )
			{
				FileSystemUtils::Path pathSrc;
				if( sourceDependency.relativeToPath )
				{
					pathSrc = sourceDependency.relativeToPath;
					if( pathSrc.HasExtension() )
					{
						pathInc[1] = compileDir / pathSrc;
						pathSrc =  compileDir / pathSrc.ParentPath() / sourceDependency.filename;
					}
					else
					{
						pathSrc =  compileDir / pathSrc / sourceDependency.filename;
					}
				}
				else
				{
					pathSrc = compileDir / sourceDependency.filename;
				}
                pathSrc.ToOSCanonicalCase();
                pathSrc = pathSrc.DelimitersToOSDefault();
                pathSrc = pathSrc.GetCleanPath();
				pathInc[0] = pathSrc;
				if( sourceDependency.extension )
				{
					pathSrc.ReplaceExtension( sourceDependency.extension );
				}
				pathSrc = FindFile( pathSrc.GetCleanPath() );
				TFileToFilePair sourcePathPair;
				sourcePathPair.first = filePath;
				sourcePathPair.second = pathSrc;
                project.m_RuntimeSourceDependencyMap.insert( sourcePathPair );
                
                // if the include file with a source dependancy is logged as an runtime include, then we mark this .cpp as compile dependencies on change
				for( int inc=0; inc<2; ++inc )
				{
					TFileToFilesEqualRange range = project.m_RuntimeIncludeMap.equal_range( pathInc[inc] );
					if( range.first != range.second )
					{
						// add source file to runtime file list
						AddToRuntimeFileList( pathSrc.c_str(), projectId );

						// also add this as a source dependency, so it gets force compiled on change of header (and not just compiled)
						TFileToFilePair includePathPair;
						includePathPair.first = pathInc[inc];
						includePathPair.second = pathSrc;
						project.m_RuntimeIncludeMap.insert( includePathPair );
					}
				}
			}
		}
	}

    bFirstTime = false;
#endif
}

RuntimeObjectSystem::ProjectSettings& RuntimeObjectSystem::GetProject( unsigned short projectId_ )
{
    if( projectId_ >= m_Projects.size() )
    {
        m_Projects.resize(projectId_ + 1);
    }
    return m_Projects[ projectId_ ];
}


void RuntimeObjectSystem::AddIncludeDir( const char *path_, unsigned short projectId_ )
{
    GetProject( projectId_).m_CompilerOptions.includeDirList.push_back( path_ );
}


void RuntimeObjectSystem::AddLibraryDir( const char *path_, unsigned short projectId_ )
{
    GetProject( projectId_ ).m_CompilerOptions.libraryDirList.push_back( path_ );
}

void RuntimeObjectSystem::SetAdditionalCompileOptions( const char *options, unsigned short projectId_ )
{
    GetProject( projectId_ ).m_CompilerOptions.compileOptions = options;
}

void RuntimeObjectSystem::SetCompilerLocation( const char *path, unsigned short projectId_ )
{
    GetProject( projectId_ ).m_CompilerOptions.compilerLocation = path;
}

void RuntimeObjectSystem::SetAdditionalLinkOptions( const char *options, unsigned short projectId_ )
{
    GetProject( projectId_ ).m_CompilerOptions.linkOptions = options;
}

void RuntimeObjectSystem::SetOptimizationLevel( RCppOptimizationLevel optimizationLevel_,	unsigned short projectId_ )
{
    GetProject( projectId_ ).m_CompilerOptions.optimizationLevel = optimizationLevel_;
}

RCppOptimizationLevel RuntimeObjectSystem::GetOptimizationLevel(					unsigned short projectId_ )
{
	return GetProject( projectId_ ).m_CompilerOptions.optimizationLevel;
}

void RuntimeObjectSystem::SetIntermediateDir(            const char* path_,      unsigned short projectId_ )
{
	GetProject( projectId_ ).m_CompilerOptions.baseIntermediatePath = path_;
}

void RuntimeObjectSystem::CleanObjectFiles() const
{
    if( m_pBuildTool )
    {
		for( unsigned short proj = 0; proj < m_Projects.size(); ++proj )
		{
			for( int optimizationLevel = 0;
					optimizationLevel < RCCPPOPTIMIZATIONLEVEL_SIZE;
					++optimizationLevel )
			{
				Path intermediateFolder = GetIntermediateFolder( m_Projects[ proj ].m_CompilerOptions.baseIntermediatePath, RCppOptimizationLevel( optimizationLevel ) );
				m_pBuildTool->Clean( intermediateFolder );
			}
		}
    }
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

    size_t numFilesToBuild = 0;
    for( unsigned short proj = 0; proj < m_Projects.size( ); ++proj )
    {
        numFilesToBuild += m_Projects[ proj ].m_RuntimeFileList.size( );
    }

    if( 0 == numFilesToBuild )
    {
        failCallbackLocal->TestBuildCallback( NULL, TESTBUILDRRESULT_NO_FILES_TO_BUILD );
    }
    
    for( unsigned short proj = 0; proj < m_Projects.size(); ++proj )
    {
        TFileList filesToTest = m_Projects[ proj ].m_RuntimeFileList; // m_RuntimeFileList could change if file content changes (new includes or source dependencies) so make copy to ensure iterators valid.
        for( TFileList::iterator it = filesToTest.begin(); it != filesToTest.end(); ++it )
        {
            const Path& file = *it;
            if( file.Extension() != ".h" ) // exclude headers, use TestBuildAllRuntimeHeaders
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

    size_t numFilesToBuild = 0;
    for( unsigned short proj = 0; proj < m_Projects.size( ); ++proj )
    {
        numFilesToBuild += m_Projects[ proj ].m_RuntimeFileList.size( );
    }

    if( 0 == numFilesToBuild )
    {
        failCallbackLocal->TestBuildCallback( NULL, TESTBUILDRRESULT_NO_FILES_TO_BUILD );
    }

    for( unsigned short proj = 0; proj < m_Projects.size(); ++proj )
    {
        TFileList filesToTest = m_Projects[ proj ].m_RuntimeFileList; // m_RuntimeFileList could change if file content changes (new includes or source dependencies) so make copy to ensure iterators valid.
        for( TFileList::iterator it = filesToTest.begin( ); it != filesToTest.end( ); ++it )
        {
            const Path& file = *it;
            if( file.Extension() == ".h" ) // exclude headers, use TestBuildAllRuntimeHeaders
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
