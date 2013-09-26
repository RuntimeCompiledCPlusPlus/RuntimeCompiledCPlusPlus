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

struct ICompilerLogger;
struct IObjectFactorySystem;
struct IFileChangeNotifier;
class  BuildTool;
struct RuntimeProtector;
struct SystemTable;

enum TestBuildFailType
{
    TESTBUILDFAILTYPE_NONE,               // should not see this
    TESTBUILDFAILTYPE_NO_FILES_TO_BUILD,  // file registration error or no runtime files of this type
    TESTBUILDFAILTYPE_BUILD_FILE_GONE,    // the file is no longer present
    TESTBUILDFAILTYPE_BUILD_NOT_STARTED,  // file change detection could be broken, or if an include may not be included anywhere
    TESTBUILDFAILTYPE_BUILD_FAILED,       // a build was started, but it failed or module failed to load. See log.
    TESTBUILDFAILTYPE_OBJECT_SWAP_FAIL,   // build succeeded, module loaded but errors on swapping
};

// callback gets name of file which when touched causes a failed
// build. Return true to continue with testing more files or false to end
// test. Errors will also be output to log in 'standard' rcc++ way.
// file may be NULL if type TESTBUILDFAILTYPE_NO_FILES_TO_BUILD 
typedef bool (*RCCppTestBuildFailCallback)(const char* file, TestBuildFailType type);

struct IRuntimeObjectSystem
{
public:
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
	virtual void AddToRuntimeFileList( const char* filename ) = 0;
	virtual void RemoveFromRuntimeFileList( const char* filename ) = 0;
	virtual void AddIncludeDir( const char *path_ ) = 0;
	virtual void AddLibraryDir( const char *path_ ) = 0;
	virtual void SetAdditionalCompileOptions( const char *options ) = 0;
	virtual void SetAdditionalLinkOptions( const char *options ) = 0;

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
   virtual int TestBuildAllRuntimeSourceFiles(  RCCppTestBuildFailCallback failCallback ) = 0;

    // tests touching each header which has RUNTIME_MODIFIABLE_INCLUDE.
    // returns the number of errors - 0 if all passed.
    virtual int TestBuildAllRuntimeHeaders(      RCCppTestBuildFailCallback failCallback ) = 0;
};

#endif // IRUNTIMEOBJECTSYSTEM_INCLUDED