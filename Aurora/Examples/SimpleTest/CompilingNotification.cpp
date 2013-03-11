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
#include "../../RuntimeCompiler/IFileChangeNotifier.h"

#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../RuntimeObjectSystem/IObjectFactorySystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"
#include "../../RuntimeObjectSystem/IRuntimeObjectSystem.h"

#include "../../Systems/SystemTable.h"
#include "../../Systems/ILogSystem.h"
#include "../../Systems/IGUISystem.h"
#include "../../Systems/IUpdateable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/ITimeSystem.h"
#include "../../Systems/IAssetSystem.h"

#include "IEntityObject.h"
#include <stdio.h>

const float UPDATE_INTERVAL = 0.1f; // Update display this often (in seconds)
const float SHOW_COMPLETE_INTERVAL = 1.0f;
const float SHOW_FLASH_INTERVAL = 0.2f;


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

	// copies string which sets a flash with strength denoted by
	// flashAmount (a 0.0f - 1.0f value);
	// flashcolour (bool false = red, true = green);
	void StrCpyFlashDivStartRML( char* text, float flashAmount, bool flashColour )
	{
		
		if( flashAmount < 0.0f )
		{
			flashAmount = 0.0f;
		}
		
		int red = 0;
		int green = 0;
		int blue = 0;
		if ( !flashColour ) //red
		{
			red = 20 + (int)( 235.0f*flashAmount );
			green = 20 + (int)( 40.0f*flashAmount );
			blue = 20 + (int)( 40.0f*flashAmount );
		}
		else //green
		{
			red = 20 + (int)( 40.0f*flashAmount );
			green = 20 + (int)( 235.0f*flashAmount );
			blue = 20 + (int)( 40.0f*flashAmount );
		}

		char flashdivstart[80];
		flashdivstart[0] = 0;
		sprintf( flashdivstart, "<div style='background-color: rgb(%d,%d,%d);'>", red, green, blue);
		strcpy( text, flashdivstart );
	}



	void StrCatCompilingRML( char* text )
	{
		static char phrase[] = "&nbsp;&nbsp;&nbsp;&nbsp;Compiling C++ Code&nbsp;&nbsp;";
		static char dots[6][20] = {	"",
									".",
									"..",
									"...",
									"&nbsp;..",
									"&nbsp;&nbsp;." };
		static unsigned int count = 0;
		count = (count+1) % 6; //TODO: this will now change every frame, not every 0.2s (grumble)

		double time = PerModuleInterface::g_pSystemTable->pTimeSystem->GetFrameSessionTime();
		unsigned int newCount = (int)( time/UPDATE_INTERVAL ) % 6;
		strcat( text, phrase );
		strcat( text, dots[newCount] );
	}




	virtual void Update( float deltaTime )
	{
		if (m_pCompilingNotification)
		{
			// Refreshing the content of the counter is not itself free, so we impose some limit
			// Since deltaTime is game time, which can be paused or slowed down, we update with frame time
			double fSmoothFrameTime = PerModuleInterface::g_pSystemTable->pTimeSystem->GetSmoothFrameDuration();
			m_fTimeToNextUpdate -= fSmoothFrameTime;

			bool bCompiling = PerModuleInterface::g_pSystemTable->pRuntimeObjectSystem->GetIsCompiling();
			bool bLoadedModule = PerModuleInterface::g_pSystemTable->pRuntimeObjectSystem->GetLastLoadModuleSuccess();
			char text[200];

			switch( m_CompilationStatus )
			{
				case NOT_COMPILING:
					if( bCompiling )
					{
						m_CompilationStatus = COMPILING_INPROGRESS_FLASH;
						m_fTimeToNextUpdate = SHOW_FLASH_INTERVAL;
					}
					else
					{
						break;
					}
				case COMPILING_INPROGRESS_FLASH:
					m_pCompilingNotification->SetProperty( "display", "block" );
					StrCpyFlashDivStartRML( text, m_fTimeToNextUpdate/SHOW_FLASH_INTERVAL, true );
					StrCatCompilingRML( text );
					strcat( text, "</div>" );
					m_pCompilingNotification->SetInnerRML( text );
					if( m_fTimeToNextUpdate <= 0 )
					{
						m_CompilationStatus = COMPILING_INPROGRESS;
						m_fTimeToNextUpdate = UPDATE_INTERVAL;
					}
					else
					{
						break;
					}
				case COMPILING_INPROGRESS:
					text[0] = 0;	//sets string to be ""
					StrCatCompilingRML( text );
					m_pCompilingNotification->SetInnerRML(text);
					if( !bCompiling )
					{
						m_CompilationStatus = COMPILING_COMPLETE_FLASH;
						m_fTimeToNextUpdate = SHOW_FLASH_INTERVAL;
					}
					else
					{
						break;
					}
				case COMPILING_COMPLETE_FLASH:
					StrCpyFlashDivStartRML( text, m_fTimeToNextUpdate/SHOW_FLASH_INTERVAL, true );
					strcat( text, "&nbsp;&nbsp;&nbsp;&nbsp;Compile complete" );
					strcat( text, "</div>" );
					m_pCompilingNotification->SetInnerRML( text );
					if( m_fTimeToNextUpdate <= 0 )
					{
						m_CompilationStatus = COMPILING_COMPLETE;
						m_fTimeToNextUpdate = SHOW_COMPLETE_INTERVAL;
						m_pCompilingNotification->SetInnerRML( "&nbsp;&nbsp;&nbsp;&nbsp;Compile complete" );
					}
					else
					{
						break;
					}
				case COMPILING_COMPLETE:
					if( m_fTimeToNextUpdate <= 0 )
					{
						m_CompilationStatus = LOAD_MODULE_STATUS_FLASH;
						m_fTimeToNextUpdate = SHOW_FLASH_INTERVAL;
					}
					else
					{
						break;
					}
				case LOAD_MODULE_STATUS_FLASH:
					if ( !bLoadedModule )
					{
						StrCpyFlashDivStartRML( text, m_fTimeToNextUpdate/SHOW_FLASH_INTERVAL, false );
						strcat( text, "&nbsp;&nbsp;&nbsp;&nbsp;Module Load Fail" );
						strcat( text, "</div>" );
						m_pCompilingNotification->SetInnerRML( text );
						if( m_fTimeToNextUpdate <= 0 )
						{
							m_CompilationStatus = LOAD_MODULE_STATUS;
							m_fTimeToNextUpdate = SHOW_COMPLETE_INTERVAL;
							m_pCompilingNotification->SetInnerRML( "&nbsp;&nbsp;&nbsp;&nbsp;Module Load Fail" );
						}
						else
						{
							break;
						}
					}
					else
					{
						m_CompilationStatus = NOT_COMPILING;
					    m_pCompilingNotification->SetProperty( "display", "none" );
					}
				case LOAD_MODULE_STATUS:
					if( m_fTimeToNextUpdate <= 0 )
					{
						m_CompilationStatus = NOT_COMPILING;
						m_pCompilingNotification->SetProperty( "display", "none" );
					}
					else
					{
						break;
					} 
				default:;
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
		IFileChangeNotifier* pFileChangeNotifier = PerModuleInterface::g_pSystemTable->pFileChangeNotifier;

		// Set watches on the data files we rely on for drawing GUI
		std::string path = PerModuleInterface::g_pSystemTable->pAssetSystem->GetAssetDirectory();
		path += "/GUI/compiling-notification.rml";
		pFileChangeNotifier->Watch(path.c_str(), this);
		path = PerModuleInterface::g_pSystemTable->pAssetSystem->GetAssetDirectory();
		path += "/GUI/compiling-notification.rcss";
		pFileChangeNotifier->Watch(path.c_str(), this);
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

		// Load the compiling notification rml
		IGUISystem* pGUI = PerModuleInterface::g_pSystemTable->pGUISystem;

		if (forceLoad)
		{
			// Clear style sheet cache so any changes to RCSS files will be applied
			pGUI->ClearStyleSheetCache();
		}

		IGUIDocument* pDocument = forceLoad ? NULL : pGUI->GetDocument("CompilingNotification");
		if (pDocument == NULL)
		{
			pDocument = pGUI->LoadDocument("/GUI/compiling-notification.rml", "CompilingNotification");
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
		COMPILING_INPROGRESS_FLASH,
		COMPILING_INPROGRESS,
		COMPILING_COMPLETE_FLASH,
		COMPILING_COMPLETE,
		LOAD_MODULE_STATUS_FLASH,
		LOAD_MODULE_STATUS
	} m_CompilationStatus;
};

REGISTERCLASS(CompilingNotification);


