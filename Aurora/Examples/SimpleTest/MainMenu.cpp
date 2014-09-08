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

// OnCheckbox Handy checkbox class
struct OnCheckbox : public IGUISingleEventListener
{
	virtual void OnEvent( const IGUIEvent& event_info )
	{
        char value[100];
		event_info.GetParameter( "value", value, sizeof( value ) );
        bool bCheck =  strlen( value ) > 0;
        OnCheckChanged(bCheck);
	}

    virtual void OnAdd()
    {
        if( GetElement() )
        {
            char value[100];
            GetElement()->GetAttribute( "checked", value, sizeof( value ) );
            bool bCheck = strlen( value ) > 0;
            OnCheckChanged(bCheck);
        }
    }

    virtual void OnCheckChanged( bool bCheck ) = 0;
};

class OnClickConsole : public IGUISingleEventListener
{
public:
	virtual void OnEvent( const IGUIEvent& event_info )
	{
		PerModuleInterface::g_pSystemTable->pGame->ToggleConsoleGUI();
	}
};

class OnClickNewButton : public IGUISingleEventListener
{
public:
	virtual void OnEvent( const IGUIEvent& event_info )
	{
		PerModuleInterface::g_pSystemTable->pGame->Reset();
	}
};


class OnClickRestartButton : public IGUISingleEventListener
{
public:
	virtual void OnEvent( const IGUIEvent& event_info )
	{
		PerModuleInterface::g_pSystemTable->pGame->Restart();
	}
};

bool g_bTestFileTracking = true;

class OnClickTestRCCppButton : public IGUISingleEventListener
{
public:
	virtual void OnEvent( const IGUIEvent& event_info )
	{
        PerModuleInterface::g_pSystemTable->pGame->RunRCCppTests(g_bTestFileTracking);
	}
};

class OnClickUndoRCCppButton : public IGUISingleEventListener
{
public:
	virtual void OnEvent( const IGUIEvent& event_info )
	{
        PerModuleInterface::g_pSystemTable->pObjectFactorySystem->UndoObjectConstructorChange();
	}
};

class OnClickRedoRCCppButton : public IGUISingleEventListener
{
public:
	virtual void OnEvent( const IGUIEvent& event_info )
	{
        PerModuleInterface::g_pSystemTable->pObjectFactorySystem->RedoObjectConstructorChange();
	}
};

class OnTestFileTracking : public OnCheckbox
{
public:
    virtual void OnCheckChanged( bool bCheck )
    {
        g_bTestFileTracking = bCheck;
    }
};


class OnClickVisibleButton : public IGUISingleEventListener
{
public:
	OnClickVisibleButton() 
		: m_bVisible( false )
		, m_pTargetElement(0)
		, m_bInline(true)
		, m_pChildClose(0)
        , m_bChildWasVisible(false)
	{
	}
	~OnClickVisibleButton()
	{
		SetTargetElement( 0 );
	}
	void SetTargetElement( IGUIElement* pElement )
	{
		if( m_pTargetElement )
		{
			m_pTargetElement->RemoveReference();
		}
		m_pTargetElement = pElement;
	}

	void SetChildClose( OnClickVisibleButton* pChild )
	{
		m_pChildClose = pChild;
	}

	virtual void OnEvent( const IGUIEvent& event_info )
	{
		m_bVisible = !m_bVisible; // Toggle or force close
		SetVisibility();

        if ( m_pChildClose && ( ( !m_bVisible && m_pChildClose->m_bVisible ) ||  ( m_bChildWasVisible && m_bVisible ) ) )
		{
            m_bChildWasVisible = m_pChildClose->m_bVisible;
			m_pChildClose->OnEvent( event_info );
		}
	}

	void SetVisibility()
	{
		if(m_bInline)
		{
			m_pTargetElement->SetProperty( "display", m_bVisible ? "inline" : "none" );
		}
		else
		{
			m_pTargetElement->SetProperty( "display", m_bVisible ? "block" : "none" );
		}
	}

	bool m_bVisible;
	bool m_bChildWasVisible;
	bool m_bInline;
private:
	IGUIElement*        m_pTargetElement;
	OnClickVisibleButton*  m_pChildClose;
};

class OnAutoCompile : public OnCheckbox
{
public:
    virtual void OnCheckChanged( bool bCheck )
    {
		PerModuleInterface::g_pSystemTable->pRuntimeObjectSystem->SetAutoCompile( bCheck );
    }
};

class OnFastCompile : public OnCheckbox
{
public:
    virtual void OnCheckChanged( bool bCheck )
    {
        PerModuleInterface::g_pSystemTable->pRuntimeObjectSystem->SetFastCompileMode( bCheck );
    }
};

class OnOptimizeDebug : public OnCheckbox
{
public:
    virtual void OnCheckChanged( bool bCheck )
    {
		RCppOptimizationLevel optlevel = RCCPPOPTIMIZATIONLEVEL_DEFAULT;
		if( bCheck )
		{
			optlevel = RCCPPOPTIMIZATIONLEVEL_DEBUG;
		}
		PerModuleInterface::g_pSystemTable->pRuntimeObjectSystem->SetOptimizationLevel( optlevel );
		
    }
};

float g_Speed = 1.0f;
bool g_Paused = true;

class OnChangeSpeed : public IGUISingleEventListener
{
public:
	float m_MaxSpeed;
	virtual void OnEvent( const IGUIEvent& event_info )
	{
		char Value[100];
		event_info.GetParameter( "value", Value, sizeof( Value ) );
		g_Speed = (float)atof( Value )*m_MaxSpeed;
		if( !g_Paused )
		{
			PerModuleInterface::g_pSystemTable->pGame->SetSpeed ( g_Speed );
		}
	}

};

class OnPauseGame : public OnCheckbox
{
public:
    virtual void OnCheckChanged( bool bCheck )
    {
		g_Paused = bCheck;
        if( g_Paused )
        {
			PerModuleInterface::g_pSystemTable->pGame->SetSpeed( 0.0f );
        }
        else
        {
    		PerModuleInterface::g_pSystemTable->pGame->SetSpeed( g_Speed );
        }
    }
};


class MainMenu : public IObject, public IFileChangeListener
{
public:
	
	MainMenu()
	{
	}

	~MainMenu()
	{
	}



	virtual void Init( bool isFirstInit )
	{
		InitWatch();
		InitDocument(false);
	}

	
	virtual void Serialize(ISimpleSerializer *pSerializer) 
	{
		SERIALIZE( m_MenuEvent.m_bVisible);
		SERIALIZE( m_OptionsEvent.m_bVisible);
		SERIALIZE( g_Speed );
		SERIALIZE( g_Paused );
        SERIALIZE( g_bTestFileTracking );
	}

	virtual void OnFileChange(const IAUDynArray<const char*>& filelist) 
	{
		// Reload RML document and clear RCSS cache
		InitDocument(true);
	}

	void InitWatch()
	{
		IFileChangeNotifier* pFileChangeNotifier = PerModuleInterface::g_pSystemTable->pFileChangeNotifier;

		// Set watches on the data files we rely on for drawing GUI
		std::string path = PerModuleInterface::g_pSystemTable->pAssetSystem->GetAssetDirectory();
		path += "/GUI/menu.rml";
		pFileChangeNotifier->Watch(path.c_str(), this);
		path = PerModuleInterface::g_pSystemTable->pAssetSystem->GetAssetDirectory();
		path += "/GUI/menu.rcss";
		pFileChangeNotifier->Watch(path.c_str(), this);
	}


	void InitDocument(bool forceLoad)
	{
		// Load and show the menu
		IGUISystem* pGUI = PerModuleInterface::g_pSystemTable->pGUISystem;

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
            m_ConsoleEvent.AddEventToElementInDoc( "click", "ConsoleButton", pDocument );

            m_NewEvent.AddEventToElementInDoc( "click", "NewButton", pDocument );

            m_RestartEvent.AddEventToElementInDoc( "click", "RestartButton", pDocument );

            m_MenuEvent.AddEventToElementInDoc( "click", "ToggleButton", pDocument );
			m_MenuEvent.SetChildClose( &m_OptionsEvent );
			m_MenuEvent.SetTargetElement( pDocument->Element()->GetElementById("menu-group") );
			m_MenuEvent.SetVisibility();	//force toggle menu to set to default state

            m_OptionsEvent.AddEventToElementInDoc( "click", "ToggleOptions", pDocument );
			m_OptionsEvent.SetTargetElement( pDocument->Element()->GetElementById("Options") );
			m_OptionsEvent.m_bInline = false;
			m_OptionsEvent.SetVisibility();	//force toggle menu to set to default state

            m_AutoCompileCheckBoxEvent.AddEventToElementInDoc(   "change", "autocompilecheckbox", pDocument );
            m_FastCompileCheckBoxEvent.AddEventToElementInDoc(   "change", "fastcompilecheckbox", pDocument );
			m_OptimizeDebugCheckBoxEvent.AddEventToElementInDoc( "change", "optimizefordebug", pDocument );


			char Value[80];
            m_SpeedEvent.AddEventToElementInDoc( "change", "speedslider", pDocument );
            m_SpeedEvent.GetElement()->GetAttribute( "value", Value, sizeof( Value ) );
			float val = (float)atof( Value );
			char Max[100];
			m_SpeedEvent.GetElement()->GetAttribute( "max", Max, sizeof( Max ) );
			float max = (float)atof( Max );
			m_SpeedEvent.m_MaxSpeed = max;

            m_PauseCheckBoxEvent.AddEventToElementInDoc( "change", "pausecheckbox", pDocument );

            m_TestRCCpp.AddEventToElementInDoc( "click", "TestRCCpp", pDocument );
            m_testFileTracking.AddEventToElementInDoc( "change", "TestFileTracking", pDocument );
            m_UndoRCCpp.AddEventToElementInDoc( "click", "UndoRCCpp", pDocument );
            m_RedoRCCpp.AddEventToElementInDoc( "click", "RedoRCCpp", pDocument );

            if( bHaveLoadedDoc )
            {
                m_PauseCheckBoxEvent.OnAdd();
                m_FastCompileCheckBoxEvent.OnAdd();
                m_AutoCompileCheckBoxEvent.OnAdd();
				m_OptimizeDebugCheckBoxEvent.OnAdd();
                m_testFileTracking.OnAdd();
            }

			pDocument->Show();
			pDocument->RemoveReference();
		}

	}


	OnClickConsole			m_ConsoleEvent;
	OnClickNewButton		m_NewEvent;
	OnClickRestartButton	m_RestartEvent;
	OnClickVisibleButton	m_MenuEvent;
	OnClickVisibleButton	m_OptionsEvent;
	OnAutoCompile			m_AutoCompileCheckBoxEvent;
	OnFastCompile			m_FastCompileCheckBoxEvent;
	OnOptimizeDebug			m_OptimizeDebugCheckBoxEvent;
	OnChangeSpeed			m_SpeedEvent;
	OnPauseGame				m_PauseCheckBoxEvent;
    OnClickTestRCCppButton  m_TestRCCpp;
    OnTestFileTracking      m_testFileTracking;
	OnClickUndoRCCppButton	m_UndoRCCpp;
	OnClickRedoRCCppButton	m_RedoRCCpp;
};

REGISTERSINGLETON(MainMenu, false);