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

// FPSCounter.cpp : Defines an IObject that displays framerate on screen
//
#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../RuntimeObjectSystem/IObjectFactorySystem.h"
#include "../../RuntimeObjectSystem/IRuntimeObjectSystem.h"
#include "../../RuntimeObjectSystem/IObject.h"
#include "../../RuntimeCompiler/IFileChangeNotifier.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/ILogSystem.h"
#include "../../Systems/IGUISystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"
#include "../../Systems/IUpdateable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/IGame.h"
#include "../../Systems/IAssetSystem.h"

class OnClickCompile : public IGUIEventListener
{
public:
	virtual void OnEvent( int event_id, const IGUIEvent& event_info )
	{
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		pSystemTable->pRuntimeObjectSystem->CompileAll( true );
	}
};

class OnClickConsole : public IGUIEventListener
{
public:
	virtual void OnEvent( int event_id, const IGUIEvent& event_info )
	{
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		pSystemTable->pGame->ToggleConsoleGUI();
	}
};

class OnClickNewButton : public IGUIEventListener
{
public:
	virtual void OnEvent( int event_id, const IGUIEvent& event_info )
	{
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		pSystemTable->pGame->Reset();
	}
};


class OnClickRestartButton : public IGUIEventListener
{
public:
	virtual void OnEvent( int event_id, const IGUIEvent& event_info )
	{
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		pSystemTable->pGame->Restart();
	}
};



class OnClickVisibleButton : public IGUIEventListener
{
public:
	OnClickVisibleButton() 
		: m_bVisible( false )
		, m_pElement(0)
		, m_bInline(true)
		, m_pChildClose(0)
	{
	}
	~OnClickVisibleButton()
	{
		SetElement( 0 );
	}
	void SetElement( IGUIElement* pElement )
	{
		if( m_pElement )
		{
			m_pElement->RemoveReference();
		}
		m_pElement = pElement;
	}

	void SetChildClose( IGUIEventListener* pChild )
	{
		m_pChildClose = pChild;
	}

	virtual void OnEvent( int event_id, const IGUIEvent& event_info )
	{
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		m_bVisible = (event_id == 0) ? !m_bVisible : false; // Toggle or force close	
		SetVisibility();

		if ( !m_bVisible && m_pChildClose )
		{
			m_pChildClose->OnEvent( 1, event_info );
		}
	}

	void SetVisibility()
	{
		if(m_bInline)
		{
			m_pElement->SetProperty( "display", m_bVisible ? "inline" : "none" );
		}
		else
		{
			m_pElement->SetProperty( "display", m_bVisible ? "block" : "none" );
		}
	}

	bool m_bVisible;
	bool		m_bInline;
private:
	IGUIElement* m_pElement;
	IGUIEventListener* m_pChildClose;
};

bool g_bAutoCompile = true;


class OnAutoCompile : public IGUIEventListener
{
public:
	virtual void OnEvent( int event_id, const IGUIEvent& event_info )
	{
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		char AutoCompile[100];
		event_info.GetParameter( "value", AutoCompile, sizeof( AutoCompile ) );
		int length = strlen( AutoCompile );
		if ( 0 == length )
		{
			g_bAutoCompile = false;
		}
		else
		{
			g_bAutoCompile = true;
		}
		pSystemTable->pRuntimeObjectSystem->SetAutoCompile( g_bAutoCompile );
	}
};


float g_Volume = 1.0f;
bool g_Muted = false;

class OnChangeVolume : public IGUIEventListener
{
public:
	virtual void OnEvent( int event_id, const IGUIEvent& event_info )
	{
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		char Value[100];
		event_info.GetParameter( "value", Value, sizeof( Value ) );
		g_Volume = (float)atof( Value );
		if( !g_Muted )
		{
			pSystemTable->pGame->SetVolume( g_Volume );
		}
	}

};

class OnMuteVolume : public IGUIEventListener
{
public:
	virtual void OnEvent( int event_id, const IGUIEvent& event_info )
	{
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		char Mute[100];
		event_info.GetParameter( "value", Mute, sizeof( Mute ) );
		int length = strlen( Mute );
		if ( length == 0 )
		{
			g_Muted = false;
			pSystemTable->pGame->SetVolume( g_Volume );
		}
		else
		{
			g_Muted = true;
			pSystemTable->pGame->SetVolume( 0.0f );
		}
	}

};


float g_Speed = 1.0f;
bool g_Paused = true;

class OnChangeSpeed : public IGUIEventListener
{
public:
	float m_MaxSpeed;
	virtual void OnEvent( int event_id, const IGUIEvent& event_info )
	{
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		char Value[100];
		event_info.GetParameter( "value", Value, sizeof( Value ) );
		g_Speed = (float)atof( Value )*m_MaxSpeed;
		if( !g_Paused )
		{
			pSystemTable->pGame->SetSpeed ( g_Speed );
		}
	}

};


class OnPauseGame : public IGUIEventListener
{
public:
	virtual void OnEvent( int event_id, const IGUIEvent& event_info )
	{
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		char Pause[100];
		event_info.GetParameter( "value", Pause, sizeof( Pause ) );
		int length = strlen( Pause );
		if ( length == 0 )
		{
			g_Paused = false;
			pSystemTable->pGame->SetSpeed( g_Speed );
		}
		else
		{
			g_Paused = true;
			pSystemTable->pGame->SetSpeed( 0.0f );
		}
	}
};

class MainMenu : public IObject, public IFileChangeListener
{
public:
	
	MainMenu()
		: m_pReCompileButton(0)
		, m_pConsoleButton(0)
		, m_pNewButton(0)
		, m_pRestartButton(0)
		, m_pMenuButton(0)
		, m_pOptionsButton(0)
		, m_pAutoCompileCheckBox(0)
		, m_pVolumeSlider(0)
		, m_pMuteCheckBox(0)
		, m_pSpeedSlider(0)
		, m_pPauseCheckBox(0)

	{
	}

	~MainMenu()
	{
		RemoveReferences();
	}

	void RemoveReferences()
	{
		if( m_pReCompileButton )
		{
			m_pReCompileButton->RemoveEventListener( "click", &m_CompileEvent, 0 );
			m_pReCompileButton->RemoveReference();
			m_pReCompileButton = 0;
		}
		if( m_pConsoleButton )
		{
			m_pConsoleButton->RemoveEventListener( "click", &m_ConsoleEvent, 0 );
			m_pConsoleButton->RemoveReference();
			m_pConsoleButton = 0;
		}
		if( m_pNewButton )
		{
			m_pNewButton->RemoveEventListener( "click", &m_NewEvent, 0 );
			m_pNewButton->RemoveReference();
			m_pNewButton = 0;
		}
		if( m_pRestartButton )
		{
			m_pRestartButton->RemoveEventListener( "click", &m_RestartEvent, 0 );
			m_pRestartButton->RemoveReference();
			m_pRestartButton = 0;
		}
		if( m_pMenuButton )
		{
			m_pMenuButton->RemoveEventListener( "click", &m_MenuEvent, 0 );
			m_pMenuButton->RemoveReference();
			m_pMenuButton = 0;
		}
		if( m_pOptionsButton )
		{
			m_pOptionsButton->RemoveEventListener( "click", &m_OptionsEvent, 0 );
			m_pOptionsButton->RemoveReference();
			m_pOptionsButton = 0;
		}
		if( m_pAutoCompileCheckBox )
		{
			m_pAutoCompileCheckBox->RemoveEventListener( "change", &m_AutoCompileCheckBoxEvent, 0 );
			m_pAutoCompileCheckBox->RemoveReference();
			m_pAutoCompileCheckBox = 0;
		}
		if( m_pVolumeSlider )
		{
			m_pVolumeSlider->RemoveEventListener( "change", &m_VolumeEvent, 0 );
			m_pVolumeSlider->RemoveReference();
			m_pVolumeSlider = 0;
		}
		if( m_pMuteCheckBox )
		{
			m_pMuteCheckBox->RemoveEventListener( "change", &m_MuteCheckBoxEvent, 0 );
			m_pMuteCheckBox->RemoveReference();
			m_pMuteCheckBox = 0;
		}
		if( m_pSpeedSlider )
		{
			m_pSpeedSlider->RemoveEventListener( "change", &m_SpeedEvent, 0 );
			m_pSpeedSlider->RemoveReference();
			m_pSpeedSlider = 0;
		}
		if( m_pPauseCheckBox )
		{
			m_pPauseCheckBox->RemoveEventListener( "change", &m_PauseCheckBoxEvent, 0 );
			m_pPauseCheckBox->RemoveReference();
			m_pPauseCheckBox = 0;
		}
	}


	virtual void Init( bool isFirstInit )
	{
		InitWatch();
		InitDocument(false);
	}

	
	virtual void Serialize(ISimpleSerializer *pSerializer) 
	{
		SERIALIZE(m_MenuEvent.m_bVisible);
		SERIALIZE(m_OptionsEvent.m_bVisible);
		SERIALIZE( g_bAutoCompile );
		SERIALIZE( g_Volume );
		SERIALIZE( g_Muted );
		SERIALIZE( g_Speed );
		SERIALIZE( g_Paused );
	}

	virtual void OnFileChange(const IAUDynArray<const char*>& filelist) 
	{
		// Reload RML document and clear RCSS cache
		InitDocument(true);
	}

	void InitWatch()
	{
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		IFileChangeNotifier* pFileChangeNotifier = pSystemTable->pFileChangeNotifier;

		// Set watches on the data files we rely on for drawing GUI
		std::string path = pSystemTable->pAssetSystem->GetAssetDirectory();
		path += "/GUI/menu.rml";
		pFileChangeNotifier->Watch(path.c_str(), this);
		path = pSystemTable->pAssetSystem->GetAssetDirectory();
		path += "/GUI/menu.rcss";
		pFileChangeNotifier->Watch(path.c_str(), this);
	}


	void InitDocument(bool forceLoad)
	{
		RemoveReferences();

		// Load and show the menu
		SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
		IGUISystem* pGUI = pSystemTable->pGUISystem;

		if (forceLoad)
		{
			// Clear style sheet cache so any changes to RCSS files will be applied
			pGUI->ClearStyleSheetCache();
		}

		bool bHaveLoadedDoc = false;
		IGUIDocument* pDocument = forceLoad ? NULL : pGUI->GetDocument("MainDocument");
		if (pDocument == NULL)
		{
			pDocument = pGUI->LoadDocument( "/GUI/menu.rml", "MainDocument");
			bHaveLoadedDoc = true;
		}

		if( pDocument )
		{
			m_pReCompileButton = pDocument->Element()->GetElementById( "ReCompileButton");
			m_pReCompileButton->AddEventListener( "click", &m_CompileEvent, 0 );

			m_pConsoleButton = pDocument->Element()->GetElementById( "ConsoleButton");
			m_pConsoleButton->AddEventListener( "click", &m_ConsoleEvent, 0 );

			m_pNewButton = pDocument->Element()->GetElementById( "NewButton");
			m_pNewButton->AddEventListener( "click", &m_NewEvent, 0 );

			m_pRestartButton = pDocument->Element()->GetElementById( "RestartButton");
			m_pRestartButton->AddEventListener( "click", &m_RestartEvent, 0 );

			m_pMenuButton = pDocument->Element()->GetElementById( "ToggleButton");
			m_pMenuButton->AddEventListener( "click", &m_MenuEvent, 0 );
			m_MenuEvent.SetChildClose( &m_OptionsEvent );
			m_MenuEvent.SetElement( pDocument->Element()->GetElementById("menu-group") );
			m_MenuEvent.SetVisibility();	//force toggle menu to set to default state

			m_pOptionsButton = pDocument->Element()->GetElementById( "ToggleOptions");
			m_pOptionsButton->AddEventListener( "click", &m_OptionsEvent, 0 );
			m_OptionsEvent.SetElement( pDocument->Element()->GetElementById("Options") );
			m_OptionsEvent.m_bInline = false;
			m_OptionsEvent.SetVisibility();	//force toggle menu to set to default state

			SystemTable* pSystemTable = PerModuleInterface::GetInstance()->GetSystemTable();
			m_pAutoCompileCheckBox = pDocument->Element()->GetElementById( "autocompilecheckbox");
			m_pAutoCompileCheckBox->AddEventListener( "change", &m_AutoCompileCheckBoxEvent, 0 );
			//TODO: fix below. See other code below for pause and call the game->setAutoCompile with the value

			if( bHaveLoadedDoc )
			{
				char AutoCompile[100];
				m_pAutoCompileCheckBox->GetAttribute( "checked", AutoCompile, sizeof( AutoCompile ) );
				g_bAutoCompile = strlen( AutoCompile ) > 0;
			}
			pSystemTable->pRuntimeObjectSystem->SetAutoCompile( g_bAutoCompile );


			m_pVolumeSlider = pDocument->Element()->GetElementById( "volumeslider");
			m_pVolumeSlider->AddEventListener( "change", &m_VolumeEvent, 0 );
			char Value[100];
			m_pVolumeSlider->GetAttribute( "value", Value, sizeof( Value ) );
			g_Volume = (float)atof( Value );

			m_pMuteCheckBox = pDocument->Element()->GetElementById( "mutecheckbox");
			m_pMuteCheckBox->AddEventListener( "change", &m_MuteCheckBoxEvent, 0 );

			if( bHaveLoadedDoc )
			{
				char Mute[100];
				m_pMuteCheckBox->GetAttribute( "checked", Mute, sizeof( Mute ) );
				g_Muted = strlen( Mute ) > 0;
			}
			
			if( g_Muted )
			{
				pSystemTable->pGame->SetVolume( 0.0f );
			}
			else
			{
				pSystemTable->pGame->SetVolume( g_Volume );
			}

			m_pSpeedSlider = pDocument->Element()->GetElementById( "speedslider");
			m_pSpeedSlider->AddEventListener( "change", &m_SpeedEvent, 0 );
			m_pSpeedSlider->GetAttribute( "value", Value, sizeof( Value ) );
			float val = (float)atof( Value );
			pSystemTable->pGame->SetSpeed( val );
			char Max[100];
			m_pSpeedSlider->GetAttribute( "max", Max, sizeof( Max ) );
			float max = (float)atof( Max );
			m_SpeedEvent.m_MaxSpeed = max;

			m_pPauseCheckBox = pDocument->Element()->GetElementById( "pausecheckbox");
			m_pPauseCheckBox->AddEventListener( "change", &m_PauseCheckBoxEvent, 0 );
			
			if( bHaveLoadedDoc )
			{
				char Pause[100];
				m_pPauseCheckBox->GetAttribute( "checked", Pause, sizeof( Pause ) );
				g_Paused = strlen( Pause ) > 0;
			}
			
			if( g_Paused )
			{
				pSystemTable->pGame->SetSpeed( 0.0f );
			}
			else
			{
				pSystemTable->pGame->SetSpeed( g_Speed );					
			}

			pDocument->Show();
			pDocument->RemoveReference();
		}

	}


	IGUIElement* m_pReCompileButton;
	IGUIElement* m_pConsoleButton;
	IGUIElement* m_pNewButton;
	IGUIElement* m_pRestartButton;
	IGUIElement* m_pMenuButton;
	IGUIElement* m_pOptionsButton;
	IGUIElement* m_pAutoCompileCheckBox;
	IGUIElement* m_pVolumeSlider;
	IGUIElement* m_pMuteCheckBox;
	IGUIElement* m_pSpeedSlider;
	IGUIElement* m_pPauseCheckBox;

	OnClickCompile			m_CompileEvent;
	OnClickConsole			m_ConsoleEvent;
	OnClickNewButton		m_NewEvent;
	OnClickRestartButton	m_RestartEvent;
	OnClickVisibleButton	m_MenuEvent;
	OnClickVisibleButton	m_OptionsEvent;
	OnAutoCompile			m_AutoCompileCheckBoxEvent;
	OnChangeVolume			m_VolumeEvent;
	OnMuteVolume			m_MuteCheckBoxEvent;
	OnChangeSpeed			m_SpeedEvent;
	OnPauseGame				m_PauseCheckBoxEvent;
};

REGISTERCLASS(MainMenu);
