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

#include "IInputManager.h"
#include "IGameManager.h"
#include "IGameObject.h"
#include "IObjectUtils.h"
#include "ICameraControl.h"
#include "GlobalParameters.h"

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

#include <float.h>
#include <assert.h>
#include <limits>
#include <algorithm>
#include <stdio.h>

class InputManager: public IInputManager, public IFileChangeListener, public IGUIEventListener, public IGameEventListener
{
	// We have two sets of typedefs here, one for fast access during runtime, and another
	// that is used for safe storage during serialization
	typedef std::vector<IGameObject*> TGameObjects;
	typedef std::vector<ObjectId> TGameObjectIds;

public:
	InputManager() 
		: m_pInputElement(0)
		, m_pInfoElement(0)
		, m_pSelectedObject(0)
		, m_pCameraControl(0)
		, m_pGlobalParameters(0)
		, m_bShowingObjectInfo(false)
	{
	}

	virtual ~InputManager()
	{
		if( m_pEntity )
		{
			m_pEntity->SetUpdateable(NULL);
		}

		if ( m_pInputElement )
		{
			m_pInputElement->RemoveEventListener( "click", this );
			m_pInputElement->RemoveReference();
		}
		if ( m_pInfoElement )
		{
			m_pInfoElement->RemoveReference();
		}

		((IGameManager*)IObjectUtils::GetUniqueInterface( "GameManager", IID_IGAMEMANAGER ))->RemoveListener(this);
	}


	// IObject

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		AU_ASSERT(pSerializer);
		IEntityObject::Serialize(pSerializer);
		SERIALIZEIOBJPTR(m_pSelectedObject);
		
		SerializeObjectsList( pSerializer);
	}

	virtual void Init( bool isFirstInit )
	{
		m_pEntity->SetUpdateable( this );

		m_pCameraControl = (ICameraControl*)IObjectUtils::GetUniqueInterface( "CameraControl", IID_ICAMERACONTROL );
		IGameManager* pGameManager = (IGameManager*)IObjectUtils::GetUniqueInterface( "GameManager", IID_IGAMEMANAGER );

		pGameManager->AddListener(this);
		m_pGlobalParameters = pGameManager->GetGlobalParameters();

		InitWatch();
		InitDocument(false);
	}

	// ~IObject
	
	// IAUUpdateable
	
	virtual void Update( float deltaTime )
	{
		if( !m_pInfoElement )
		{
			// No available element to alter - probably a document error
			return;
		}

		if (m_pSelectedObject)
		{
			if (!m_bShowingObjectInfo)
			{
				m_pInfoElement->SetProperty( "display", "inline" );
				m_bShowingObjectInfo = true;
			}

			char info[1024];
			m_pSelectedObject->GetDebugInfo(info, sizeof(info));
			m_pInfoElement->SetAttribute( "value", info );
		}
		else if (m_bShowingObjectInfo)
		{
			m_pInfoElement->SetProperty( "display", "none" );
			m_bShowingObjectInfo = false;
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
		AUVec3f selectPos;
		char buff[16];
		event_info.GetParameter( "mouse_x", buff, sizeof(buff) );
		selectPos.SetX( (float)atof( buff ) );
		event_info.GetParameter( "mouse_y", buff, sizeof(buff) );
		selectPos.SetY( (float)atof( buff ) );
		event_info.GetParameter( "button", buff, sizeof(buff) );
		int button = atoi( buff );

		AUVec3f worldPos = m_pCameraControl->Unproject( selectPos );

		IGameObject* pGameObject = GetSelectedObject( worldPos );
		if ( button == 0 ) // left button
		{
			if ( m_pSelectedObject && pGameObject != m_pSelectedObject )
			{
				// Deselect currently selected object
				m_pSelectedObject->OnDeselect();
				m_pSelectedObject = 0;
			}

			if ( pGameObject )
			{
				// Select object
				pGameObject->OnSelect();
				m_pSelectedObject = pGameObject;
			}
		}
		else // right button
		{
			if ( m_pSelectedObject && pGameObject != m_pSelectedObject )
			{
				// Set target
				m_pSelectedObject->OnPositionRequest( worldPos );
			}
		}
	}
	
	// ~IGUIEventListener 

	// IGameEventListener

	virtual void OnGameReset() 
	{
		m_pSelectedObject = 0;
		m_Objects.clear();
	}

	virtual void OnGameObjectCreated( IGameObject* pGameObject )
	{
		m_Objects.push_back( pGameObject );
	}

	virtual void OnGameObjectAboutToDestroy( IGameObject* pGameObject )
	{
		TGameObjects::iterator it = std::find(m_Objects.begin(), m_Objects.end(), pGameObject);
		if (it != m_Objects.end())
		{
			m_Objects.erase(it);
		}

		if (m_pSelectedObject == pGameObject)
		{
			m_pSelectedObject = 0;
		}
	}

	// IInputManager

	virtual IGameObject* GetCurrentlySelectedObject()
	{
		return m_pSelectedObject;
	}

	virtual const IGameObject* GetCurrentlySelectedObject() const
	{
		return m_pSelectedObject;
	}

	// ~IInputManager


private:

	IGameObject* GetSelectedObject( const AUVec3f& selectPos )
	{
		IGameObject* pGameObject = 0;
		float dist = FLT_MAX;
		TGameObjects::iterator it = m_Objects.begin();
		TGameObjects::iterator itEnd = m_Objects.end();
		while (it != itEnd)
		{
			IGameObject* pObj = *it;
			const AUVec3f& objPos = pObj->GetEntity()->GetPosition();
			float testDist = AUVec3f(objPos.x - selectPos.x, 0.0f, objPos.z - selectPos.z).Magnitude();
			if ( testDist < pObj->GetCollisionRadius() && testDist < dist )
			{
				pGameObject = pObj;
				dist = testDist;
			}

			++it;
		}

		return pGameObject;
	}

	void InitWatch()
	{
		IFileChangeNotifier* pFileChangeNotifier = PerModuleInterface::g_pSystemTable->pFileChangeNotifier;

		// Set watches on the data files we rely on for drawing GUI
		std::string path = PerModuleInterface::g_pSystemTable->pAssetSystem->GetAssetDirectory();
		path += "/GUI/input.rml";
		pFileChangeNotifier->Watch(path.c_str(), this);
		path = PerModuleInterface::g_pSystemTable->pAssetSystem->GetAssetDirectory();
		path += "/GUI/input.rcss";
		pFileChangeNotifier->Watch(path.c_str(), this);
	}

	void InitDocument(bool forceLoad)
	{
		// may be serializing an already initialized object, ensure we handle reference
		// counting correctly.
		if (m_pInputElement)
		{
			m_pInputElement->RemoveEventListener( "click", this );
			m_pInputElement->RemoveReference();
			m_pInputElement = 0;
		}
		if ( m_pInfoElement )
		{
			m_pInfoElement->RemoveReference();
			m_pInfoElement = 0;
		}

		// Load and show the input element
		IGUISystem* pGUI = PerModuleInterface::g_pSystemTable->pGUISystem;

		if (forceLoad)
		{
			// Clear style sheet cache so any changes to RCSS files will be applied
			pGUI->ClearStyleSheetCache();
		}

		IGUIDocument* pDocument = forceLoad ? NULL : pGUI->GetDocument("Input");
		if (pDocument == NULL)
		{
			pDocument = pGUI->LoadDocument("/GUI/input.rml", "Input");
		}

		if (pDocument != NULL)
		{
			pDocument->Show();
			m_pInputElement = pDocument->Element()->GetElementById("input");
			m_pInputElement->AddEventListener( "click", this );

			// Make input element same size as window
			char buff[16];
			float windowWidth, windowHeight;
			PerModuleInterface::g_pSystemTable->pGame->GetWindowSize( windowWidth, windowHeight );
            _snprintf_s(buff, sizeof(buff), _TRUNCATE, "%d",(int)windowWidth);
			m_pInputElement->SetProperty( "width", buff );
            _snprintf_s(buff, sizeof(buff), _TRUNCATE, "%d",(int)windowHeight);
			m_pInputElement->SetProperty( "height", buff );
			
			// Set up info element in the bottom right corner
			m_pInfoElement = pDocument->Element()->GetElementById("info");

			if( m_pInfoElement )
			{
				if ( m_pSelectedObject )
				{
					m_pInfoElement->SetProperty( "display", "inline" );
					m_bShowingObjectInfo = true;
				}
				else
				{
					m_pInfoElement->SetProperty( "display", "none" );
					m_bShowingObjectInfo = false;
				}

			}
			pDocument->RemoveReference();
		}
	}

	void SerializeObjectsList( ISimpleSerializer *pSerializer )
	{
		TGameObjectIds m_ObjectIds;

		if ( !pSerializer->IsLoading() )
		{
			// Create a collection of ObjectIds that matches m_Objects pointer collection 
		
			size_t count = m_Objects.size();
			m_ObjectIds.resize( count );

			for (size_t i=0; i<count; ++i)
			{
				m_ObjectIds[i] = m_Objects[i]->GetObjectId();
			}
		}

		SERIALIZE(m_ObjectIds);

		if ( pSerializer->IsLoading() )
		{
			// Rebuild m_objects pointer collection
			
			size_t count = m_ObjectIds.size();
			m_Objects.clear();
			m_Objects.resize( count );

			for (size_t i=0; i<count; ++i)
			{
				IGameObject* pGameObject = 0;
				IObjectUtils::GetObject( &pGameObject, m_ObjectIds[i] );

				m_Objects[i] = pGameObject;
			}		
		}	
	}


	// Private Members

	ICameraControl* m_pCameraControl;
	GlobalParameters* m_pGlobalParameters;

	TGameObjects m_Objects;
	
	IGUIElement* m_pInputElement;
	IGUIElement* m_pInfoElement;
	IGameObject* m_pSelectedObject;
	bool m_bShowingObjectInfo;
};

REGISTERCLASS(InputManager);




