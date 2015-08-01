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

#include "ISplashScreen.h"

#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../RuntimeCompiler/IFileChangeNotifier.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/IAssetSystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"
#include "../../Systems/IGUISystem.h"
#include "../../Systems/IGame.h"
#include "../../Systems/IAssetSystem.h"

#include <assert.h>
#include <stdio.h>
#include <algorithm>

class SplashScreen: public ISplashScreen, public IFileChangeListener, public IGUIEventListener
{
public:
	SplashScreen() 
		: m_pSplashElement(0)
		, m_pDocument(0)
		, m_fMinViewTime(0.0f)
		, m_fFadeInTime(0.0f)
		, m_fFadeOutTime(0.0f)
		, m_fTimeDisplayed(0.0f)
		, m_fFadeOutStartTime(0.0f)
		, m_bAutoClose(false)
		, m_bCloseRequested(false)
		, m_bReadyToClose(false)
	{
	}

	virtual ~SplashScreen()
	{
		if( m_pEntity )
		{
			m_pEntity->SetUpdateable(NULL);
		}

		if ( m_pSplashElement )
		{
			m_pSplashElement->RemoveEventListener( "click", this );
			m_pSplashElement->RemoveReference();
		}

		if ( m_pDocument )
		{
			if (!IsRuntimeDelete())
			{
				m_pDocument->Hide();
			}
			
			m_pDocument->RemoveReference();
		}
	}


	// IEntityObject

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		AU_ASSERT(pSerializer);
		IEntityObject::Serialize(pSerializer);
		SERIALIZE(m_ImageFile);
		SERIALIZE(m_fMinViewTime);
		SERIALIZE(m_fFadeInTime);
		SERIALIZE(m_fFadeOutTime);
		SERIALIZE(m_fTimeDisplayed);
		SERIALIZE(m_fFadeOutStartTime);
		SERIALIZE(m_bAutoClose);
		SERIALIZE(m_bCloseRequested);
		SERIALIZE(m_bReadyToClose);
	}

	virtual void Init( bool isFirstInit )
	{
		m_pEntity->SetUpdateable( this );

		InitWatch();
		InitDocument(false);

		if (!m_ImageFile.empty())
		{
			SetImage(m_ImageFile.c_str());
		}
	}

	// ~IEntityObject

	// IAUUpdateable

	virtual void Update( float deltaTime )
	{
		if( !m_pSplashElement || !m_pDocument )
		{
			return;
		}

		if (!m_bReadyToClose)
		{
			m_pDocument->Show(); // HACK - recompiling with visible splashscreen seems to make it vanish for some reason


			char buff[32];
			
			// Check if window has re-szie and move splash screen
			float currWindowSize[2];
			PerModuleInterface::g_pSystemTable->pGame->GetWindowSize( currWindowSize[0], currWindowSize[1] );
			if( currWindowSize[0] != m_WindowSize[0] || currWindowSize[1] != m_WindowSize[1] )
			{
				m_WindowSize[0] = currWindowSize[0];
				m_WindowSize[1] = currWindowSize[1];

				int left;
				left = (int)( (m_WindowSize[0] - m_pSplashElement->GetClientWidth()) * 0.5f );
                _snprintf_s(buff, sizeof(buff), _TRUNCATE, "%d",left);
				m_pSplashElement->SetProperty("left", buff);

				int top;
				top = (int)( (m_WindowSize[1] - m_pSplashElement->GetClientHeight()) * 0.5f );
                _snprintf_s(buff, sizeof(buff), _TRUNCATE, "%d",top);
				m_pSplashElement->SetProperty("top", buff);
			}


			m_fTimeDisplayed += deltaTime;
			if (m_fTimeDisplayed < m_fFadeInTime)
			{
				// Fade In
				float t = m_fTimeDisplayed / m_fFadeInTime;
				_snprintf_s( buff, sizeof(buff), _TRUNCATE, "rgba(255,255,255,%d)", (int)(255 * t) );
				m_pSplashElement->SetProperty( "color", buff );
			}
			else if (m_fTimeDisplayed < (m_fFadeInTime + m_fMinViewTime) || 
			         (!m_bAutoClose && !m_bCloseRequested))
			{
				// Regular View
				m_fFadeOutStartTime = m_fTimeDisplayed;
				m_pSplashElement->SetProperty( "color", "rgba(255,255,255,255)" );
			}
			else if (m_fTimeDisplayed < (m_fFadeOutStartTime + m_fFadeOutTime))
			{
				// Fade Out
				float t = (m_fFadeOutStartTime + m_fFadeOutTime - m_fTimeDisplayed) / m_fFadeOutTime;
				_snprintf_s( buff, sizeof(buff),_TRUNCATE , "rgba(255,255,255,%d)", (int)(255 * t) );
				m_pSplashElement->SetProperty( "color", buff );
			}
			else 
			{
				m_bReadyToClose = true;
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

	// IGUIEventListener 

	virtual void OnEvent( const IGUIEvent& event_info )
	{
		m_bCloseRequested = true;
	}

	// ~IGUIEventListener 


	// ISplashScreen

	virtual void SetImage( const char* imageFile )
	{
		AU_ASSERT(imageFile);
		m_ImageFile = imageFile;
		if (m_pSplashElement)
		{
			m_pSplashElement->SetAttribute("src", imageFile);

			// Position element correctly so that it is centered
			PerModuleInterface::g_pSystemTable->pGame->GetWindowSize( m_WindowSize[0], m_WindowSize[1] );

			char buff[16];
			int left;
			left = (int)( (m_WindowSize[0] - m_pSplashElement->GetClientWidth()) * 0.5f );
            _snprintf_s(buff, sizeof(buff), _TRUNCATE, "%d",left);
			m_pSplashElement->SetProperty("left", buff);

			int top;
			top = (int)( (m_WindowSize[1] - m_pSplashElement->GetClientHeight()) * 0.5f );
            _snprintf_s(buff, sizeof(buff), _TRUNCATE, "%d",top);
			m_pSplashElement->SetProperty("top", buff);
		}
	}

	virtual void SetMinViewTime( float fSeconds )
	{
		m_fMinViewTime = std::max(0.0f, fSeconds);
	}

	virtual void SetFadeInTime( float fSeconds )
	{
		m_fFadeInTime = std::max(0.0f, fSeconds);
	}

	virtual void SetFadeOutTime( float fSeconds )
	{
		m_fFadeOutTime = std::max(0.0f, fSeconds);
	}

	virtual void SetAutoClose( bool bAutoClose )
	{
		m_bAutoClose = bAutoClose;
	}

	virtual bool ReadyToClose() const
	{
		return m_bReadyToClose;
	}

	// ~ISplashScreen


private:

	void InitWatch()
	{
		IFileChangeNotifier* pFileChangeNotifier = PerModuleInterface::g_pSystemTable->pFileChangeNotifier;

		// Set watches on the data files we rely on for drawing GUI
		std::string path = PerModuleInterface::g_pSystemTable->pAssetSystem->GetAssetDirectory();
		path += "/GUI/splashscreen.rml";
		pFileChangeNotifier->Watch(path.c_str(), this);
		path = PerModuleInterface::g_pSystemTable->pAssetSystem->GetAssetDirectory();
		path += "/GUI/splashscreen.rcss";
		pFileChangeNotifier->Watch(path.c_str(), this);
	}

	void InitDocument(bool forceLoad)
	{
		// may be serializing an already initialized object, ensure we handle reference
		// counting correctly.
		if (m_pSplashElement)
		{
			m_pSplashElement->RemoveEventListener( "click", this );
			m_pSplashElement->RemoveReference();
			m_pSplashElement = 0;
		}
		if (m_pDocument)
		{
			m_pDocument->RemoveReference();
			m_pDocument = 0;
		}

		// Load and show the splashscreen
		IGUISystem* pGUI = PerModuleInterface::g_pSystemTable->pGUISystem;

		if (forceLoad)
		{
			// Clear style sheet cache so any changes to RCSS files will be applied
			pGUI->ClearStyleSheetCache();
		}

		//Always load document in order to reset its state correctly
		m_pDocument = pGUI->LoadDocument("/GUI/splashscreen.rml", "Splashscreen");


		if (m_pDocument != NULL)
		{
			m_pDocument->Show();
			m_pSplashElement = m_pDocument->Element()->GetElementById("splash");
			m_pSplashElement->AddEventListener( "click", this );
		}
	}


	// Private Members

	IGUIElement* m_pSplashElement;
	IGUIDocument* m_pDocument;
	std::string m_ImageFile;
	float m_fMinViewTime;
	float m_fFadeInTime;
	float m_fFadeOutTime;
	float m_fTimeDisplayed;
	float m_fFadeOutStartTime;
	bool m_bAutoClose;
	bool m_bCloseRequested;
	bool m_bReadyToClose;
	float m_WindowSize[2];
};

REGISTERCLASS(SplashScreen);




