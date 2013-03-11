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

#include "IGameManager.h"
#include "IGameObject.h"
#include "GlobalParameters.h"
#include "IObjectUtils.h"
#include "ISplashScreen.h"
#include "IBlackboardManager.h"
#include "IBlackboard.h"
#include "BB_Global.h"

#include "../../Common/AUVec3f.inl"
#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../RuntimeCompiler/IFileChangeNotifier.h"
#include "../../Systems/SystemTable.h"
#include "../../RuntimeObjectSystem/IObjectFactorySystem.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/IAssetSystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"
#include "../../Systems/IGame.h"

#include <assert.h>
#include <vector>
#include <set>
#include <algorithm>
#include <stdio.h>

class GameManager: public IGameManager
{
	// We have two sets of typedefs here, one for fast access during runtime, and another
	// that is used for safe storage during serialization
	typedef std::set<IGameObject*> TGameObjects;
	typedef std::vector<ObjectId> TGameObjectIds;

	struct GameObjectSpawnParams
	{
		EGameObject type;
		AUVec3f spawnPosition; // If infinite, will be positioned randomly

		GameObjectSpawnParams( EGameObject type ) : type(type)
		{
			spawnPosition.SetInfinite();
		}

		GameObjectSpawnParams( EGameObject type, const AUVec3f& spawnPosition ) : type(type), spawnPosition(spawnPosition) {}
	};

public:
	GameManager() 
		: m_CurrentState(EGS_STARTUP)
		, m_pSplashScreen(0)
		, m_pBBGlobal(0)
	{
		m_GameObjects.resize(EGO_COUNT);
		m_NextSpawnTimes.resize(EGO_COUNT);
		m_NextGameObjectNumber.resize(EGO_COUNT);
	}

	virtual ~GameManager()
	{
		if( m_pEntity )
		{
			m_pEntity->SetUpdateable(NULL);
		}

		if (!IsRuntimeDelete())
		{
			DestroyGameObjects();
			DestroySplashScreen();
		}
	}


	// IEntityObject

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		IEntityObject::Serialize(pSerializer);
		SERIALIZE(m_GlobalParameters);
		SERIALIZE(m_NextSpawnTimes);
		SERIALIZE(m_NextGameObjectNumber);
		SERIALIZE(m_CurrentState);
		SERIALIZE(m_GameObjectsToDestroy);
		SERIALIZE(m_GameObjectsToSpawn);
		SERIALIZEIOBJPTR(m_pSplashScreen);
		SERIALIZEIOBJPTR(m_pBBGlobal);

		SerializeObjectsList( pSerializer );
	}

	virtual void Init( bool isFirstInit )
	{
		if (isFirstInit)
		{
			PreloadModels();

			CreateSplashScreen( "//GUI/title.tga", 1.0f, 0.0f, 0.5f, false );
		}
		
		m_pEntity->SetUpdateable( this );
	}

	// ~IEntityObject

	// IAUUpdateable

	virtual void Update( float deltaTime )
	{
		UpdateSplashScreen();

		UpdateGameState();
		
		if (EGS_PLAYING == m_CurrentState)
		{
			UpdateSpawning( deltaTime );

			UpdateGlobalBlackboard( deltaTime );
		}

		DestroyPendingObjects();
		SpawnPendingObjects();
	}

	// ~IAUUpdateable

	// IGameManager

	virtual void ResetGame()
	{
		DestroyGameObjects();
		
		m_GameObjectsToDestroy.clear();
		m_GameObjectsToSpawn.clear();

		m_GlobalParameters = GlobalParameters();

		IBlackboardManager* pBBManager = (IBlackboardManager*)IObjectUtils::GetUniqueInterface( "BlackboardManager", IID_IBLACKBOARDMANAGER );
		pBBManager->ResetBlackboards();
		m_pBBGlobal = (BB_Global*)pBBManager->GetBlackboardGlobal();

		if (m_CurrentState != EGS_STARTUP && m_pSplashScreen)
		{
			DestroySplashScreen();
		}

		// Set Initial spawn times
		for (int i=0; i<EGO_COUNT; ++i)
		{
			m_NextSpawnTimes[i] = m_GlobalParameters.go[i].spawn_rate;
		}

		// Notify any listeners that game was reset
		for (size_t i=0; i<m_Listeners.size(); ++i)
		{
			m_Listeners[i]->OnGameReset();
		}

		SetGameState(EGS_NEWGAME);
	}

	virtual void SpawnGameObject( EGameObject type )
	{
		m_GameObjectsToSpawn.push_back( GameObjectSpawnParams(type) );
	}

	virtual void SpawnGameObject( EGameObject type, const AUVec3f& spawnPosition )
	{
		m_GameObjectsToSpawn.push_back( GameObjectSpawnParams(type, spawnPosition) );
	}

	virtual void DestroyGameObject( ObjectId id )
	{
		m_GameObjectsToDestroy.push_back( id );
	}

	virtual void DestroyGameObject( const char* name )
	{
		IEntitySystem* pEntitySystem = PerModuleInterface::g_pSystemTable->pEntitySystem;
		IAUEntity* pEnt = pEntitySystem->Get(name);
		if (pEnt)
		{
			DestroyGameObject( pEnt->GetObject()->GetObjectId() );
		}
	}

	virtual GlobalParameters* GetGlobalParameters()
	{
		return &m_GlobalParameters;
	}

	virtual EGameState GetGameState() const
	{
		return m_CurrentState;
	}

	virtual void GetAll( EGameObject type, IAUDynArray<ObjectId> &objects ) const
	{
		AU_ASSERT(type < EGO_COUNT);
		if (type < EGO_COUNT)
		{
			size_t count = m_GameObjects[type].size();

			objects.Resize(count);
			TGameObjects::const_iterator it = m_GameObjects[type].begin();
			TGameObjects::const_iterator itEnd = m_GameObjects[type].end();
			for(int i = 0; it != itEnd; ++it, ++i)
			{
				objects[i] = (*it)->GetObjectId();
			}
		}	
	}

	void AddListener(IGameEventListener* pListener)
	{
		if ( std::find(m_Listeners.begin(), m_Listeners.end(), pListener) == m_Listeners.end() )
		{
			m_Listeners.push_back(pListener);
		}	
	}

	void RemoveListener(IGameEventListener* pListener)
	{
		TGameEventListeners::iterator it = std::find(m_Listeners.begin(), m_Listeners.end(), pListener);
		if (it != m_Listeners.end())
		{
			m_Listeners.erase(it);
		}
	}

	// ~IGameManager


private:

	void CreateInitialGameObjects()
	{
		for (int i=0; i<(int)EGO_COUNT; ++i)
		{
			int count = m_GlobalParameters.go[(EGameObject)i].initial_count;
			for (int j=0; j<count; ++j)
			{
				DoSpawnGameObject( GameObjectSpawnParams( (EGameObject)i ) );
			}
		}
	}

	AUVec3f GetSpawnPosition( EGameObject type )
	{
		float width, height;
		PerModuleInterface::g_pSystemTable->pGame->GetWindowSize( width, height );
		float edgeMargin = 0.05f;

		// Spawn WBC in bottom quarter of screen, RBC in middle quarter, and Virus and Infected Cell
		// in the top quarter
		float min = height * edgeMargin;       // from bottom of screen
		float max = height * (1 - edgeMargin);  // from bottom of screen
		if (type == EGO_WBC)
		{
			max = height * 0.25f;
		}
		else if (type == EGO_RBC)
		{
			min = height * 0.375f;
			max = height * 0.625f;
		}
		else
		{
			min = height * 0.75f;
		}
		
		AUVec3f position;
		position.SetX( rand() * 1.0f / RAND_MAX * width * (1.0f - edgeMargin * 2) - width * (0.5f - edgeMargin) );
		position.SetY( 0.0f );
		position.SetZ( rand() * 1.0f / RAND_MAX * (max - min) + min - height * 0.5f);

		return position;
	}

	void UpdateSpawning( float deltaTime )
	{
		for (int i=0; i<EGO_COUNT; ++i)
		{
			m_NextSpawnTimes[i] -= deltaTime;
			if (m_GlobalParameters.go[i].spawn_rate > 0.0f && m_NextSpawnTimes[i] < 0.0f)
			{
				m_NextSpawnTimes[i] += m_GlobalParameters.go[i].spawn_rate;
				if (m_GameObjects[i].size() < (size_t)m_GlobalParameters.go[i].max_count)
				{
					DoSpawnGameObject( GameObjectSpawnParams( (EGameObject)i ) );
				}
			}			
		}
	}

	void SpawnPendingObjects()
	{
		TSpawnParams::iterator it = m_GameObjectsToSpawn.begin();
		TSpawnParams::iterator itEnd = m_GameObjectsToSpawn.end();
		while (it != itEnd)
		{
			DoSpawnGameObject( *it );
			++it;
		}
		m_GameObjectsToSpawn.clear();
	}

	void DestroyPendingObjects()
	{
		TGameObjectIds::iterator it = m_GameObjectsToDestroy.begin();
		TGameObjectIds::iterator itEnd = m_GameObjectsToDestroy.end();
		while (it != itEnd)
		{
			DoDestroyGameObject( *it );
			++it;
		}
		m_GameObjectsToDestroy.clear();
	}

	IGameObject* DoSpawnGameObject( const GameObjectSpawnParams& params )
	{
		IGameObject* pGameObject = 0;
		EGameObject type = params.type;

		if (type < EGO_COUNT)
		{
			std::string name = m_GlobalParameters.go[type].base_name;
			char buff[16];
            _snprintf_s(buff, sizeof(buff), _TRUNCATE, "%d",++(m_NextGameObjectNumber[type]));
			name += buff;

			IObject* pObj = IObjectUtils::CreateObjectAndEntity( "GameObject", name.c_str() );
			IObjectUtils::GetObject( &pGameObject, pObj->GetObjectId() );

			AUVec3f pos = (params.spawnPosition.IsInfinite()) ? GetSpawnPosition(type) : params.spawnPosition;
			pGameObject->Init(type, pos);
			m_GameObjects[type].insert(pGameObject);
		}

		// Notify any listeners that object was created
		for (size_t i=0; i<m_Listeners.size(); ++i)
		{
			m_Listeners[i]->OnGameObjectCreated(pGameObject);
		}

		return pGameObject;
	}

	void DoDestroyGameObject( ObjectId id )
	{
		IObjectFactorySystem* pFactory = PerModuleInterface::g_pSystemTable->pObjectFactorySystem;
		IObject* pObj = pFactory->GetObject(id);
		if (pObj)
		{
			IGameObject* pGameObject = 0;
			IObjectUtils::GetObject( &pGameObject, pObj->GetObjectId() );

			// Notify any listeners that object is about to be destroyed
			for (size_t i=0; i<m_Listeners.size(); ++i)
			{
				m_Listeners[i]->OnGameObjectAboutToDestroy(pGameObject);
			}

			m_GameObjects[pGameObject->GetGameObjectType()].erase(pGameObject);
			IObjectUtils::DestroyObjectAndEntity( pGameObject->GetEntityId() );
		}
	}

	void UpdateGameState()
	{
		switch (m_CurrentState)
		{
		case EGS_STARTUP:
			// Created a few dummy RBCs on initial startup to set the scene until a new game is started
			if (!m_pSplashScreen && m_GameObjects[EGO_RBC].size() == 0)
			{
				for (int j=0; j<8; ++j)
				{
					DoSpawnGameObject( GameObjectSpawnParams( EGO_RBC ) );
				}
			}

			break;
		case EGS_NEWGAME:
			if (!m_pSplashScreen)
			{
				CreateInitialGameObjects();

				SetGameState(EGS_PLAYING);
			}

			break;
		case EGS_PLAYING:
			if (m_GameObjects[EGO_WBC].size() == 0)
			{
				SetGameState(EGS_INFECTIONWON);
				CreateSplashScreen( "//GUI/infectionwin.tga", 1.0f, 0.5f, 0.0f, false );
			}
			else if (m_GameObjects[EGO_VIRUS].size() == 0 && m_GameObjects[EGO_INFECTED].size() == 0)
			{
				SetGameState(EGS_IMMUNEWON);
				CreateSplashScreen( "//GUI/immunewin.tga", 1.0f, 0.5f, 0.0f, false );
			}

			break;
		case EGS_IMMUNEWON:
		case EGS_INFECTIONWON:
			if (!m_pSplashScreen)
			{
				ResetGame();
			}

			break;
		}
	}

	void CreateSplashScreen( const char* file, float fMinTime, float fFadeInTime, float fFadeOutTime, bool bAutoClose )
	{
		DestroySplashScreen();

		IObject* pObj = IObjectUtils::CreateObjectAndEntity( "SplashScreen", "SplashScreen" );
		IObjectUtils::GetObject( &m_pSplashScreen, pObj->GetObjectId() );
		m_pSplashScreen->SetImage(file);
		m_pSplashScreen->SetMinViewTime(fMinTime);
		m_pSplashScreen->SetFadeInTime(fFadeInTime);
		m_pSplashScreen->SetFadeOutTime(fFadeOutTime);
		m_pSplashScreen->SetAutoClose(bAutoClose);
	}

	void UpdateSplashScreen()
	{
		if (m_pSplashScreen && m_pSplashScreen->ReadyToClose())
		{
			DestroySplashScreen();
		}
	}

	void DestroyGameObjects()
	{
		for (size_t i=0; i<m_GameObjects.size(); ++i)
		{
			TGameObjects& objects = m_GameObjects[i];
			TGameObjects::iterator it = objects.begin();
			TGameObjects::iterator itEnd = objects.end();
			while (it != itEnd)
			{
				IGameObject* pGameObject = *it;
				if (pGameObject)
				{
					IObjectUtils::DestroyObjectAndEntity( pGameObject->GetEntityId() );
				}

				++it;
			}
			objects.clear();
		}
	}
	
	void DestroySplashScreen()
	{
		if (m_pSplashScreen)
		{
			IObjectUtils::DestroyObjectAndEntity( m_pSplashScreen->GetEntityId() );
			m_pSplashScreen = 0;
		}
	}

	void PreloadModels()
	{
		IAssetSystem* pAssetSystem = PerModuleInterface::g_pSystemTable->pAssetSystem;

		for (int i=0; i<EGO_COUNT; ++i)
		{
			std::string path = "/Models/"; //directories relative to asset dir
			path += m_GlobalParameters.go[i].model;
			IAURenderableMesh* pMesh = pAssetSystem->CreateRenderableMeshFromFile( path.c_str() );
			if (pMesh)
			{
				pAssetSystem->DestroyRenderableMesh(pMesh);
			}
		}
	}

	void SetGameState( EGameState state )
	{
		if (state != m_CurrentState)
		{
			m_CurrentState = state;

			// Notify any listeners that state has changed
			for (size_t i=0; i<m_Listeners.size(); ++i)
			{
				m_Listeners[i]->OnStateChange(state);
			}
		}
	}

	void UpdateGlobalBlackboard( float deltaTime )
	{
		m_pBBGlobal->gameTimeElapsed += deltaTime;

		// Calculate Immune team strength (we don't count RBCs here, since they don't actively fight)
		m_pBBGlobal->immune_team_strength = GetStrengthSum( m_GameObjects[EGO_WBC] );

		// Calculate Infection team strength
		m_pBBGlobal->infection_team_strength = GetStrengthSum( m_GameObjects[EGO_VIRUS] );
		m_pBBGlobal->infection_team_strength += GetStrengthSum( m_GameObjects[EGO_INFECTED] );
	}

	float GetStrengthSum( const TGameObjects& objects ) const
	{
		float sum = 0;
		TGameObjects::const_iterator it = objects.begin();
		TGameObjects::const_iterator itEnd = objects.end();
		while (it != itEnd)
		{
			IGameObject* pGameObject = *it;
			sum += pGameObject->GetThreatRating();

			++it;
		}

		return sum;
	}

	void SerializeObjectsList( ISimpleSerializer *pSerializer )
	{
		std::vector<TGameObjectIds> m_ObjectIds; 

		if ( !pSerializer->IsLoading() )
		{
			// Create a collection of ObjectIds that matches m_Objects pointer collection 

			m_ObjectIds.resize(EGO_COUNT);
			for (int i=0; i<EGO_COUNT; ++i)
			{
				size_t count = m_GameObjects[i].size();
				m_ObjectIds[i].reserve( count );
				m_ObjectIds[i].clear();

				TGameObjects::iterator it = m_GameObjects[i].begin();
				TGameObjects::iterator itEnd = m_GameObjects[i].end();
				while (it != itEnd)
				{
					m_ObjectIds[i].push_back( (*it)->GetObjectId() );
					++it;
				}
			}
		}

		SERIALIZE(m_ObjectIds);

		if ( pSerializer->IsLoading() )
		{
			// Rebuild m_objects pointer collection

			for (int i=0; i<EGO_COUNT; ++i)
			{
				m_GameObjects[i].clear();
				size_t count = m_ObjectIds[i].size();

				for (size_t j=0; j<count; ++j)
				{
					IGameObject* pGameObject = 0;
					IObjectUtils::GetObject( &pGameObject, m_ObjectIds[i][j] );

					m_GameObjects[i].insert( pGameObject );
				}
			}
		}	
	}


	// Private Members

	typedef std::vector<unsigned int> TGameObjectNumbers;
	typedef std::vector<IGameEventListener*> TGameEventListeners;
	typedef std::vector<GameObjectSpawnParams> TSpawnParams;

	TGameEventListeners m_Listeners;
	GlobalParameters m_GlobalParameters;
	TGameObjectNumbers m_NextGameObjectNumber;
	EGameState m_CurrentState;

	std::vector<TGameObjects> m_GameObjects;
	std::vector<float> m_NextSpawnTimes;

	TGameObjectIds m_GameObjectsToDestroy;
	TSpawnParams m_GameObjectsToSpawn;

	ISplashScreen* m_pSplashScreen;
	BB_Global* m_pBBGlobal;
};

REGISTERCLASS(GameManager);




