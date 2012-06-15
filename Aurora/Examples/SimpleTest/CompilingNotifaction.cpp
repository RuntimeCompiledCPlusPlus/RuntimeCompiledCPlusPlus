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

// CompilingNotification.cpp : Defines an IObject that displays when we're compiling
//
#include "../../RunTimeCompiler/ObjectInterfacePerModule.h"
#include "../../RuntimeCompiler/IFileChangeNotifier.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/IObjectFactorySystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../Systems/IGUISystem.h"
#include "../../Systems/ISimpleSerializer.h"
#include "../../Systems/IUpdateable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/ITimeSystem.h"
#include "../../Systems/IGame.h"

#include "IEntityObject.h"

const float UPDATE_INTERVAL = 2.0f/10.0f; // Update display this often (in seconds)
const float SHOW_COMPLETE_INTERVAL = 1.5f; // Update display this often (in seconds)


class CompilingNotification: public IEntityObject, public IAUUpdateable, public IFileChangeListener
{
public:
	CompilingNotification()
	{
		m_pCompilingNotification = NULL;
		m_fTimeToNextUpdate = UPDATE_INTERVAL;
		m_CompilationStatus = NOT_COMPILING;
	}

	virtual ~CompilingNotification()
	{
		if( m_pEntity )
		{
			m_pEntity->SetUpdateable(NULL);
		}

		if (m_pCompilingNotification)
		{
			m_pCompilingNotification->RemoveReference();
		}
	}

	virtual void Serialize(ISimpleSerializer *pSerializer) 
	{
		IEntityObject::Serialize(pSerializer);
		SERIALIZE(m_fTimeToNextUpdate);
		SERIALIZE(m_CompilationStatus);
	}

	
	// IEntityObject

	virtual void Init( bool isFirstInit )
	{
		m_pEntity->SetUpdateable( this );

		InitWatch();
		InitDocument(false);
	}

	// ~IEntityObject

	// IAUUpdateable

	virtual void Update( float deltaTime )
	{
		if (m_pCompilingNotification)
		{
			// Refreshing the content of the counter is not itself free, so we impose some limit
			m_fTimeToNextUpdate -= deltaTime;
			if (m_fTimeToNextUpdate <= 0.0f)
			{
				
				SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
				bool bCompiling = pSystemTable->pGame->GetIsCompiling();
				if( bCompiling )
				{
					m_CompilationStatus = COMPILING_INPROGRESS;
					m_fTimeToNextUpdate = UPDATE_INTERVAL;
					m_pCompilingNotification->SetProperty( "display", "block" );
					static char text[80];
					static char phrase[] = "&nbsp;&nbsp;&nbsp;&nbsp;Compiling C++ Code&nbsp;&nbsp;";
					static char dots[6][20] = {	"",
												".",
												"..",
												"...",
												"&nbsp;..",
												"&nbsp;&nbsp;." };
					static unsigned int count = 0;
					count = (count+1) % 6;
					strcpy( text, phrase );
					strcat( text, dots[count] );
					m_pCompilingNotification->SetInnerRML(text);
				}
				else
				{
					switch( m_CompilationStatus )
					{
					case NOT_COMPILING:
						break; //do nothing
					case COMPILING_INPROGRESS:
						m_CompilationStatus = COMPILING_COMPLETE;
						m_pCompilingNotification->SetInnerRML("&nbsp;&nbsp;&nbsp;&nbsp;Compile complete");
						m_fTimeToNextUpdate = SHOW_COMPLETE_INTERVAL; //use longer interval to show completion
						break;
					case COMPILING_COMPLETE:
						m_CompilationStatus = NOT_COMPILING;
						m_pCompilingNotification->SetProperty( "display", "none" );
						m_fTimeToNextUpdate = UPDATE_INTERVAL;
					default:;
					};

				}
			}		
		}
	}

	// ~IAUUpdateable

	// IFileChangeListener

	virtual void OnFileChange(const IAUDynArray<const char*>& filelist) 
	{
		// Reload RML document and clear RCSS cache
		InitDocument(true);
	}

	// ~IFileChangeListener


private:

	void InitWatch()
	{
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		IFileChangeNotifier* pFileChangeNotifier = pSystemTable->pFileChangeNotifier;

		// Set watches on the data files we rely on for drawing GUI
		// Note that the path will get correctly normalized by FileChangeNotifier
		// An extra level of /.. has been added so that the filename in __FILE__ will get removed on normalizing
		char path[256]; 
		_snprintf_s(path, sizeof(path), "%s/../../../Assets/GUI/compiling-notification.rml", __FILE__);
		pFileChangeNotifier->Watch(path, this);
		_snprintf_s(path, sizeof(path), "%s/../../../Assets/GUI/compiling-notification.rcss", __FILE__);
		pFileChangeNotifier->Watch(path, this);
	}

	void InitDocument(bool forceLoad)
	{
		// may be serliazing an already initialized object, ensure we handle reference
		// counting correctly.
		if (m_pCompilingNotification)
		{
			m_pCompilingNotification->RemoveReference();
			m_pCompilingNotification = 0;
		}

		// Load and show the fps counter
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		IGUISystem* pGUI = pSystemTable->pGUISystem;

		if (forceLoad)
		{
			// Clear style sheet cache so any changes to RCSS files will be applied
			pGUI->ClearStyleSheetCache();
		}

		IGUIDocument* pDocument = forceLoad ? NULL : pGUI->GetDocument("CompilingNotification");
		if (pDocument == NULL)
		{
			pDocument = pGUI->LoadDocument("/Assets/GUI/compiling-notification.rml", "CompilingNotification");
		}

		if (pDocument != NULL)
		{
			pDocument->Show();
			m_pCompilingNotification = pDocument->Element()->GetElementById("compiling");
			
			pDocument->RemoveReference();
		}
	}


	// Private Members

	IGUIElement* m_pCompilingNotification;
	float m_fTimeToNextUpdate;

	enum CompilationStatus
	{
		NOT_COMPILING,
		COMPILING_INPROGRESS,
		COMPILING_COMPLETE
	} m_CompilationStatus;
};

REGISTERCLASS(CompilingNotification);


