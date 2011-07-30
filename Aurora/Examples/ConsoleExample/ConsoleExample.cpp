// ConsoleExample.cpp : simple example using console command line
//
// Runs a 'game loop' every time the user presses a key, and does stuff...


#include <conio.h>
#include <iostream>
#include <Windows.h>

#include "../../RuntimeCompiler/ObjectInterface.h"
#include "../../RuntimeCompiler/ICompilerLogger.h"
#include "../../RunTimeCompiler/BuildTool.h"

#define BOOST_FILESYSTEM_VERSION 3
#include "boost/filesystem.hpp" 


using boost::filesystem::path;


class CompilerLogger : public ICompilerLogger
{
public:
	virtual void LogError(const char * format, ...)
	{
		va_list args;
		va_start( args, format );
		printf( format, args);
	}

	virtual void LogWarning(const char * format, ...)
	{
		va_list args;
		va_start( args, format );
		printf( format, args);
	}

    virtual void LogInfo(const char * format, ...)
	{
		va_list args;
		va_start( args, format );
		printf( format, args);
	}
};

// Global Variables
CompilerLogger*	g_pCompilerLogger	= 0;
BuildTool*		g_pBuildTool		= 0;


bool MainLoop();
bool Init();

int main(int argc, char* argv[])
{
	if( Init() )
	{
		while( MainLoop() )
		{
		}
	}
	else
	{
		std::cout << "\nFailed Initialisation, press a key to exit.\n";
		int ret = _getche();
	}

	return 0;
}


bool MainLoop()
{
	std::cout << "\nHello\n";
	int ret = _getche();
	if( 'q' == ret )
	{
		return false;
	}

	return true;
}



bool Init()
{
	// We need the current directory to be the process dir
	DWORD size = MAX_PATH;
	wchar_t filename[MAX_PATH];
	GetModuleFileName( NULL, filename, size );
	std::wstring strTempFileName( filename );
	path launchPath( strTempFileName );
	launchPath = launchPath.parent_path();
	SetCurrentDirectory( launchPath.wstring().c_str() );

	g_pCompilerLogger = new CompilerLogger();
	g_pBuildTool = new BuildTool();
	g_pBuildTool->Initialise(g_pCompilerLogger);

	// We start by using the code in the current module
	HMODULE module = GetModuleHandle(NULL);

	GETPerModuleInterface_PROC pPerModuleInterfaceProcAdd = NULL;
	pPerModuleInterfaceProcAdd = (GETPerModuleInterface_PROC) GetProcAddress(module, "GetPerModuleInterface");
	if (!pPerModuleInterfaceProcAdd)
	{
		std::cout << "Failed GetProcAddress for GetPerModuleInterface in current module\n";
		return false;
	}

/*

	// Tell it the system table to pass to objects we construct
	pPerModuleInterfaceProcAdd()->SetSystemTable(m_pEnv->sys);

	SetupObjectConstructors(pPerModuleInterfaceProcAdd);

	m_pEnv->sys->pObjectFactorySystem->AddListener(this);

	m_pConsole = new Console(this, m_pEnv, m_pRocketContext);

	InitSound();
*/
	return true;

}

