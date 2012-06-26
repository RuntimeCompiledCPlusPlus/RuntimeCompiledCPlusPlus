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


#include "../../Common/AUArray.inl"
#include "../../RunTimeCompiler/BuildTool.h"
#include "../../RuntimeCompiler/ICompilerLogger.h"
#include "../../RuntimeCompiler/FileChangeNotifier.h"
#include "../../RuntimeObjectSystem/IObjectFactorySystem.h"
#include "../../RuntimeObjectSystem/ObjectFactorySystem/ObjectFactorySystem.h"
#include "../../RuntimeObjectSystem/RuntimeObjectSystem/RuntimeObjectSystem.h"

#include "StdioLogSystem.h"

#include "../../RuntimeObjectSystem/IObject.h"
#include "IUpdateable.h"
#include "InterfaceIds.h"

#include <iostream>
#include <tchar.h>
#include <conio.h>
#include <strstream>
#include <vector>
#include <algorithm>
#include <string>
#include <stdarg.h>

// Remove windows.h define of GetObject which conflicts with EntitySystem GetObject
#if defined _WINDOWS_ && defined GetObject
#undef GetObject
#endif
using boost::filesystem::path;

ConsoleGame::ConsoleGame()
	: m_pCompilerLogger(0)
	, m_pUpdateable(0)
	, m_pRuntimeObjectSystem(0)
{
}

ConsoleGame::~ConsoleGame()
{
	m_pRuntimeObjectSystem->GetObjectFactorySystem()->RemoveListener(this);

	// delete object via correct interface
	IObject* pObj = m_pRuntimeObjectSystem->GetObjectFactorySystem()->GetObject( m_ObjectId );
	delete pObj;

	delete m_pRuntimeObjectSystem;
	delete m_pCompilerLogger;
}


bool ConsoleGame::Init()
{
	//Initialise the RuntimeObjectSystem
	m_pRuntimeObjectSystem = new RuntimeObjectSystem;
	m_pCompilerLogger = new StdioLogSystem();
	m_pRuntimeObjectSystem->Initialise(m_pCompilerLogger, 0);
	m_pRuntimeObjectSystem->GetObjectFactorySystem()->AddListener(this);


	// construct first object
	IObjectConstructor* pCtor = m_pRuntimeObjectSystem->GetObjectFactorySystem()->GetConstructor( "RuntimeObject01" );
	if( pCtor )
	{
		IObject* pObj = pCtor->Construct();
		pObj->GetInterface( IID_IUPDATEABLE, (void**)&m_pUpdateable );
		if( 0 == m_pUpdateable )
		{
			delete pObj;
			m_pCompilerLogger->LogError("Error - no updateable interface found\n");
			return false;
		}
		m_ObjectId = pObj->GetObjectId();

	}

	return true;
}

void ConsoleGame::OnConstructorsAdded()
{
	// This could have resulted in a change of object pointer, so release old and get new one.
	if( m_pUpdateable )
	{
		IObject* pObj = m_pRuntimeObjectSystem->GetObjectFactorySystem()->GetObject( m_ObjectId );
		pObj->GetInterface( IID_IUPDATEABLE, (void**)&m_pUpdateable );
		if( 0 == m_pUpdateable )
		{
			delete pObj;
			m_pCompilerLogger->LogError( "Error - no updateable interface found\n");
		}
	}
}



bool ConsoleGame::MainLoop()
{
	//check status of any compile
	if( m_pRuntimeObjectSystem->GetIsCompiledComplete() )
	{
		// load module when compile complete, and notify console - TODO replace with event system 
		m_pRuntimeObjectSystem->LoadCompiledModule();
	}

	if( !m_pRuntimeObjectSystem->GetIsCompiling() )
	{

		std::cout << "\nMain Loop - press q to quit. Updates every second.\n";
		if( _kbhit() )
		{
			int ret = _getche();
			if( 'q' == ret )
			{
				return false;
			}
		}
		const float deltaTime = 1.0f;
		m_pRuntimeObjectSystem->GetFileChangeNotifier()->Update( deltaTime );
		m_pUpdateable->Update( deltaTime );
		Sleep(1000);
	}

	return true;
}