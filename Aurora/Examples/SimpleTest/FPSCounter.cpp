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
#include "../../RuntimeCompiler/IFileChangeNotifier.h"
#include "../../Systems/SystemTable.h"
#include "../../RuntimeObjectSystem/IObjectFactorySystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../Systems/IGUISystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"
#include "../../Systems/IUpdateable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/ITimeSystem.h"
#include "../../Systems/IAssetSystem.h"

#include "IEntityObject.h"
#include <stdio.h>

#define UPDATE_INTERVAL 1.0/25.0 // Update display this often (in seconds)


class FPSCounter: public IEntityObject, public IAUUpdateable, public IFileChangeListener
{
public:
	FPSCounter()
	{
		m_pCounterElement = NULL;
		m_fTimeToNextUpdate = UPDATE_INTERVAL;
	}

	virtual ~FPSCounter()
	{
		if( m_pEntity )
		{
			m_pEntity->SetUpdateable(NULL);
		}

		if (m_pCounterElement)
		{
			m_pCounterElement->RemoveReference();
		}
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
		if (m_pCounterElement)
		{
			// Refreshing the content of the counter is not itself free, so we impose some limit
			// Since deltaTime is game time, which can be paused or slowed down, we update with frame time
			double fSmoothFrameTime = PerModuleInterface::g_pSystemTable->pTimeSystem->GetSmoothFrameDuration();
			m_fTimeToNextUpdate -= fSmoothFrameTime;
			if (m_fTimeToNextUpdate <= 0.0f)
			{
				m_fTimeToNextUpdate += UPDATE_INTERVAL;

				if (fSmoothFrameTime < 0.0001)
					fSmoothFrameTime = 0.0001;
				int nFPS = (int)(1.0 / fSmoothFrameTime);
				char text[16];
				_snprintf_s(text, sizeof(text), _TRUNCATE, "%d", nFPS);
				m_pCounterElement->SetInnerRML(text);
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
		path += "/GUI/fps-counter.rml";
		pFileChangeNotifier->Watch(path.c_str(), this);
		path = PerModuleInterface::g_pSystemTable->pAssetSystem->GetAssetDirectory();
		path += "/GUI/fps-counter.rcss";
		pFileChangeNotifier->Watch(path.c_str(), this);
	}

	void InitDocument(bool forceLoad)
	{
		// may be serliazing an already initialized object, ensure we handle reference
		// counting correctly.
		if (m_pCounterElement)
		{
			m_pCounterElement->RemoveReference();
			m_pCounterElement = 0;
		}

		// Load and show the fps counter
		SystemTable* pSystemTable = PerModuleInterface::g_pSystemTable;
		IGUISystem* pGUI = pSystemTable->pGUISystem;

		if (forceLoad)
		{
			// Clear style sheet cache so any changes to RCSS files will be applied
			pGUI->ClearStyleSheetCache();
		}

		IGUIDocument* pDocument = forceLoad ? NULL : pGUI->GetDocument("FPSCounter");
		if (pDocument == NULL)
		{
			pDocument = pGUI->LoadDocument("/GUI/fps-counter.rml", "FPSCounter");
		}

		if (pDocument != NULL)
		{
			pDocument->Show();
			m_pCounterElement = pDocument->Element()->GetElementById("fps");
			
			pDocument->RemoveReference();
		}
	}


	// Private Members

	IGUIElement* m_pCounterElement;
	double m_fTimeToNextUpdate;
};

REGISTERCLASS(FPSCounter);


