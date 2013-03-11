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

#include "IGameObject.h"
#include "IObjectUtils.h"
#include "IBehaviorTreeManager.h"
#include "IBehaviorTree.h"
#include "IBehavior.h"
#include "IBlackboardManager.h"
#include "IGameManager.h"
#include "GlobalParameters.h"

#include "BB_Individual_Common.h"

#include "../../Common/Math.inl"
#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/IAssetSystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"
#include "../../Renderer/IAURenderable.h"

#include <assert.h>
#include <vector>
#include <stdio.h>

const AUVec3f ZERO(0,0,0);

class GameObject: public IGameObject
{
public:
	GameObject()
		: m_gameObjectType(EGO_COUNT)
		, m_pBehaviorTree(0)
		, m_pBehavior(0)
		, m_pGameManager(0)
		, m_pBlackboardManager(0)
		, m_pGlobalParameters(0)
		, m_pGameObjectParams(0)
		, m_pRenMesh(0)
		, m_rotationAxis(ZERO)	  // Demo
		, m_collisionRadius(0.0f)
		, m_scaleModulationTime(0.0f)
		, m_bIsSelected(false)
	{
	}

	virtual ~GameObject()
	{		
		if (m_pRenMesh)
		{
			PerModuleInterface::g_pSystemTable->pAssetSystem->DestroyRenderableMesh(m_pRenMesh);
		}

		if( m_pEntity )
		{
			m_pEntity->SetRenderable(NULL);
			m_pEntity->SetUpdateable(NULL);
		}

		if (!IsRuntimeDelete())
		{
			delete m_pBehavior;
		}
	}


	// IEntityObject

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		IEntityObject::Serialize(pSerializer);

		SERIALIZE(m_gameObjectType);
		SERIALIZE(m_gameTeam);
		SERIALIZE(m_color);
		SERIALIZE(m_rotationAxis); // Demo
		SERIALIZE(m_collisionRadius);
		SERIALIZE(m_scaleModulationTime);
		SERIALIZE(m_bIsSelected);
		SERIALIZEIOBJPTR(m_pBehaviorTree);
		SERIALIZEIOBJPTR(m_pBehavior);
	}

	virtual void Init( bool isFirstInit )
	{
		// Do nothing here on first init, because we need to use the more complicated version below
		// to properly init the object

		if (!isFirstInit)
		{
			DoInit(isFirstInit);
		}
	}

	virtual void Init( EGameObject type, const AUVec3f& spawnPosition )
	{
		assert (type < EGO_COUNT);
		if (type < EGO_COUNT)
		{
			// Initial load, set up some values for the first time
			m_gameObjectType = type;

			m_pEntity->SetPosition(spawnPosition);

			// Now do regular init stuff
			DoInit(true);
		}
	}

	// ~IEntityObject

	// IAUUpdateable

	virtual void Update( float deltaTime )
	{	
		AU_ASSERT(m_pBehaviorTree);
		m_pBehaviorTree->Execute(this);
		
		AU_ASSERT(m_pBehavior);
		IBehavior* pBehavior = m_pBehavior; // Demo [Tutorial02] >>> NULL;//
		
		pBehavior->Update(deltaTime);

		UpdateScale();
		m_scaleModulationTime += deltaTime;
		
		//m_pRenMesh->SetColor( AUColor(1,1,1) ); // Demo // [Tutorial01]

		SetRotation((float)M_PI_4 * 0.5f * deltaTime); // Demo
	}

	// ~IAUUpdateable

	// IGameObject

	virtual EGameObject GetGameObjectType() const
	{
		return m_gameObjectType;
	}

	virtual EGameTeam GetGameTeam() const
	{
		return m_gameTeam;
	}

	virtual const AUColor& GetColor() const
	{
		return m_color;
	}

	virtual IBehaviorTree* GetBehaviorTree()
	{
		return m_pBehaviorTree;
	}

	virtual IBehavior* GetBehavior()
	{
		return m_pBehavior;
	}

	virtual IAUEntity* GetEntity()
	{
		return m_pEntity;
	}

	virtual const IAUEntity* GetEntity() const
	{
		return m_pEntity;
	}

	virtual void SetModel( const char* file )
	{
		std::string path = "/Models/"; //directories relative to asset dir
		path += file;
		m_pRenMesh = PerModuleInterface::g_pSystemTable->pAssetSystem->CreateRenderableMeshFromFile( path.c_str() );
		m_pEntity->SetRenderable( m_pRenMesh );
	}

	virtual void SetColor( const AUColor& color )
	{
		m_color = color;
		if (m_pRenMesh)
		{
			m_pRenMesh->SetColor( m_color );
		}
	}

	virtual void SetColor( float r, float g, float b, float a )
	{
		SetColor( AUColor(r, g, b, a) );
	}

	virtual void SetBehavior( ConstructorId constructor )
	{
		if (!m_pBehavior || m_pBehavior->GetObjectId().m_ConstructorId != constructor )
		{
			IBehavior* pBehavior = 0;
			IObjectUtils::CreateObject(  &pBehavior, constructor );
			
			// Only destroy and replace existing behavior if we successfully created new one
			if (pBehavior)
			{
				if (m_pBehavior)
				{
					m_pBehavior->EndBehavior();
					delete m_pBehavior;
				}
				
				m_pBehavior = pBehavior;
				m_pBehavior->SetGameObject(this);
				m_pBehavior->StartBehavior();
			}
		}
	}


	virtual void OnSelect()
	{
		SetColor( m_pGameObjectParams->color_highlight );
		m_bIsSelected = true;
	}

	virtual void OnDeselect()
	{
		SetColor( m_pGameObjectParams->color_normal );
		m_bIsSelected = false;
	}

	virtual void OnPositionRequest( const AUVec3f& position )
	{
		BB_Individual_Common* pBB = (BB_Individual_Common*)m_pBlackboardManager->GetBlackboardIndividualCommon( this );
		pBB->target_position = position;
	}

	virtual void OnCollision( IGameObject* pCollider )
	{
		if (m_pBehavior)
		{
			m_pBehavior->OnCollision( pCollider );
		}
	}

	virtual float GetCollisionRadius() const
	{
		return m_collisionRadius;
	}

	virtual float GetMaxSpeed() const
	{
		return m_pGameObjectParams->max_speed;
	}

	virtual float GetHealth() const
	{
		BB_Individual_Common* pBB = (BB_Individual_Common*)m_pBlackboardManager->GetBlackboardIndividualCommon( this );
		return pBB->current_health;
	}

	virtual float GetThreatRating() const
	{
		BB_Individual_Common* pBB = (BB_Individual_Common*)m_pBlackboardManager->GetBlackboardIndividualCommon( this );
		return pBB->current_health * m_pGameObjectParams->attack_damage * ( 1.0f / m_pGameObjectParams->attack_speed );
	}

	virtual void GetDebugInfo( char* outputBuffer, size_t bufferLen )
	{
		BB_Individual_Common* pBB = (BB_Individual_Common*)m_pBlackboardManager->GetBlackboardIndividualCommon( this );
		const AUVec3f& pos = pBB->current_position;
		const AUVec3f& vel = pBB->current_velocity;

		_snprintf_s( outputBuffer, bufferLen, _TRUNCATE,
			"ID: %d\n"
			"Name: %s\n"
			"Position: (%0.2f, %0.2f, %0.2f)\n"
			"Velocity: (%0.2f, %0.2f, %0.2f)\n"
			"Health: %0.0f\n"
			"Behavior: %s\n",
			m_pEntity->GetId(), m_pEntity->GetName(), pos.x, pos.y, pos.z,
			vel.x, vel.y, vel.z, pBB->current_health,
			m_pBehavior ? m_pBehavior->GetTypeName() : "none"
			);
	}

	// ~IGameObject


private:

	void DoInit(bool bFirstTime)
	{
		m_pGameManager = (IGameManager*)IObjectUtils::GetUniqueInterface( "GameManager", IID_IGAMEMANAGER );
		m_pBlackboardManager= (IBlackboardManager*)IObjectUtils::GetUniqueInterface( "BlackboardManager", IID_IBLACKBOARDMANAGER );
		m_pGlobalParameters = m_pGameManager->GetGlobalParameters();
		m_pGameObjectParams = &(m_pGlobalParameters->go[m_gameObjectType]);

		m_gameTeam = m_pGameObjectParams->team;
		SetModel( m_pGameObjectParams->model.c_str() );
		m_collisionRadius = m_pGameObjectParams->collision_radius;

		if (bFirstTime)
		{
			SetColor( m_bIsSelected ? m_pGameObjectParams->color_highlight : m_pGameObjectParams->color_normal);
			m_scaleModulationTime = (float)(rand()*2*M_PI/RAND_MAX);
		}
		else
		{
			SetColor( m_color ); // Set render mesh to serialised color
		}

		if (!m_pBehaviorTree)
		{
			IBehaviorTreeManager* pBTManager = (IBehaviorTreeManager*)IObjectUtils::GetUniqueInterface( "BehaviorTreeManager", IID_IBEHAVIORTREEMANAGER );

			m_pBehaviorTree = pBTManager->GetTree( m_pGameObjectParams->behavior_tree.c_str() );
			AU_ASSERT(m_pBehaviorTree);
		}

		//* Demo
		if (m_rotationAxis.IsZero())
		{
			m_rotationAxis = AUVec3f((float)rand()/RAND_MAX - 0.5f, (float)rand()/RAND_MAX - 0.5f, (float)rand()/RAND_MAX - 0.5f);
			m_rotationAxis.Normalise();
			SetRotation((float)(rand()*2*M_PI/RAND_MAX - M_PI));
		}
		//*/

		m_pEntity->SetUpdateable( this );
	}

	//* Demo
	void SetRotation(float fRotation)
	{
		AUOrientation3D orientation = m_pEntity->GetOrientation();
		orientation.Rotate(m_rotationAxis, fRotation);
		m_pEntity->SetOrientation(orientation);
	}
	//*/

	void UpdateScale()
	{
		BB_Individual_Common* pBB = (BB_Individual_Common*)m_pBlackboardManager->GetBlackboardIndividualCommon( this );
		const float sizeDiff = m_pGameObjectParams->max_size - m_pGameObjectParams->min_size;
		const float healthProportion = pBB->current_health / m_pGameObjectParams->initial_health;
		const float modulation = 1.0f + sin(m_scaleModulationTime) * 0.3f;
		const float scale = m_pGameObjectParams->min_size + sizeDiff * healthProportion;// * modulation; // Demo

		m_pEntity->SetScale( m_pGameObjectParams->base_size_modifier * scale );

		m_collisionRadius = scale * m_pGameObjectParams->collision_radius;
	}


	// Private Members

	EGameObject m_gameObjectType;
	EGameTeam m_gameTeam;
	IBehaviorTree* m_pBehaviorTree;
	IBehavior* m_pBehavior;
	
	IGameManager* m_pGameManager;
	IBlackboardManager* m_pBlackboardManager;
	GlobalParameters* m_pGlobalParameters;
	GameObjectParams* m_pGameObjectParams;

	IAURenderableMesh* m_pRenMesh;
	AUColor m_color;
	AUVec3f m_rotationAxis; // Demo
	float m_collisionRadius;
	float m_scaleModulationTime;
	bool m_bIsSelected;
};

REGISTERCLASS(GameObject);




