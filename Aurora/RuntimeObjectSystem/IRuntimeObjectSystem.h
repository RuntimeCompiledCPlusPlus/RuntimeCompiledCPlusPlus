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

#ifndef IRUNTIMEOBJECTSYSTEM_INCLUDED
#define IRUNTIMEOBJECTSYSTEM_INCLUDED

#include "../RuntimeCompiler/CompileOptions.h"

struct ICompilerLogger;
struct IObjectFactorySystem;
struct IFileChangeNotifier;
class  BuildTool;
struct RuntimeProtector;
struct SystemTable;
struct IPerModuleInterface;

enum TestBuildResult
{
    TESTBUILDRRESULT_SUCCESS,            // SUCCESS, yay!
    TESTBUILDRRESULT_NO_FILES_TO_BUILD,  // file registration error or no runtime files of this type
    TESTBUILDRRESULT_BUILD_FILE_GONE,    // the file is no longer present
    TESTBUILDRRESULT_BUILD_NOT_STARTED,  // file change detection could be broken, or if an include may not be included anywhere
    TESTBUILDRRESULT_BUILD_FAILED,       // a build was started, but it failed or module failed to load. See log.
    TESTBUILDRRESULT_OBJECT_SWAP_FAIL,   // build succeeded, module loaded but errors on swapping
};


struct ITestBuildNotifier
{
    // Notifier gets name of file which and result type.
    // Errors will also be output to log in 'standard' RCC++ way.
    // file may be NULL if type TESTBUILDFAILTYPE_NO_FILES_TO_BUILD.
    // The default callback outputs result and file to log, and returns true.
    //
    // Return true to continue with testing more files or false to end test.
    virtual bool TestBuildCallback(const char* file, TestBuildResult type) = 0;

    // Notifier should implement sleep function for a small interval - say 10-100ms.
    // Additionally, any message queues / view updates should be handled here, especially
    // on Win32 where the file change notifiers need the message queue to be processed.
    // Default uses usleep or Sleep, dispatches messages on Win32 and returns true.
    //
    // Return true to continue with testing or false to end test.
    virtual bool TestBuildWaitAndUpdate() = 0;
};

namespace FileSystemUtils
{
    class Path;
}

struct IRuntimeObjectSystem : public ITestBuildNotifier
{
	// Initialise RuntimeObjectSystem. pLogger and pSystemTable should be deleted by creator. 
	// Both pLogger and pSystemTable can be 0
	virtual bool Initialise( ICompilerLogger * pLogger, SystemTable* pSystemTable  ) = 0;

	virtual bool GetIsCompiling() = 0;
	virtual bool GetIsCompiledComplete() = 0;
	virtual bool LoadCompiledModule() = 0;
	virtual bool GetLastLoadModuleSuccess() const = 0;

    // GetNumberLoadedModules() returns total number successfully loaded, not current number loaded
    // Mainly useful for detected wether a new module has been loaded by checking for change
    virtual unsigned int GetNumberLoadedModules() const = 0;

	virtual IObjectFactorySystem* GetObjectFactorySystem() const = 0;
	virtual IFileChangeNotifier* GetFileChangeNotifier() const = 0;

	virtual void CompileAll( bool bForceRecompile ) = 0;

    // Compile & Link settings can be associated with a project identifier.
    // This identifier should be defined by the application using RCC++,
    // for example using enums or an identifier service.
    // Identifier 0 is the default for all code not using the project identifiers.
    // The backing storage will use the an array lookup, so use compact identifiers
    // such as (0, 1, 2, 3) and not (20,39,42,250).
    virtual void CompileAllInProject(           bool bForcerecompile_,  unsigned short projectId_ = 0 ) = 0;
    virtual void AddToRuntimeFileList(          const char* filename,   unsigned short projectId_ = 0 ) = 0;
    virtual void RemoveFromRuntimeFileList(     const char* filename,   unsigned short projectId_ = 0 ) = 0;
    virtual void AddIncludeDir(                 const char *path_,      unsigned short projectId_ = 0 ) = 0;
    virtual void AddLibraryDir(                 const char *path_,      unsigned short projectId_ = 0 ) = 0;
    virtual void SetAdditionalCompileOptions(   const char *options,    unsigned short projectId_ = 0 ) = 0;
    virtual void SetAdditionalLinkOptions(      const char *options,    unsigned short projectId_ = 0 ) = 0;
    virtual void SetCompilerLocation        (   const char* path,       unsigned short projectId_ = 0 ) = 0;
    virtual void SetOptimizationLevel( RCppOptimizationLevel optimizationLevel_,	unsigned short projectId_ = 0 ) = 0;
    virtual RCppOptimizationLevel GetOptimizationLevel(					unsigned short projectId_ = 0 ) = 0;

	// Intermediate Dir has DEBUG in debug or RELEASE plus project optimization level appended to it.
	// defaults to current directory plus /Runtime
    virtual void SetIntermediateDir(            const char* path_,      unsigned short projectId_ = 0 ) = 0;

	virtual void SetAutoCompile( bool autoCompile ) = 0;
	virtual bool GetAutoCompile() const = 0;

    // see Compiler::SetFastCompileMode
    virtual void SetFastCompileMode( bool bFast ) = 0;

    // clean up temporary object files
    virtual void CleanObjectFiles() const = 0;

	virtual void SetupObjectConstructors(IPerModuleInterface* pPerModuleInterface) = 0;

	//ensure subclasses are deleted correctly
	virtual ~IRuntimeObjectSystem(){};

    // exception handling to catch and protect main app from crashing when using runtime compiling
    virtual void SetProtectionEnabled( bool bProtectionEnabled_ ) = 0;
	virtual bool IsProtectionEnabled() const = 0;
    virtual bool TryProtectedFunction( RuntimeProtector* pProtectedObject_ ) = 0;

    // tests one by one touching each runtime modifiable source file
    // returns the number of errors - 0 if all passed.
   virtual int TestBuildAllRuntimeSourceFiles(  ITestBuildNotifier* callback, bool bTestFileTracking ) = 0;

    // tests touching each header which has RUNTIME_MODIFIABLE_INCLUDE.
    // returns the number of errors - 0 if all passed.
    virtual int TestBuildAllRuntimeHeaders(     ITestBuildNotifier* callback, bool bTestFileTracking ) = 0;

    // FindFile - attempts to find the file in a source directory
    virtual FileSystemUtils::Path   FindFile( const FileSystemUtils::Path& input ) = 0;

    // AddPathToSourceSearch - adds a path to help source search. Can be called multiple times to add paths.
    virtual void AddPathToSourceSearch( const char* path ) = 0;

};

#endif // IRUNTIMEOBJECTSYSTEM_INCLUDED