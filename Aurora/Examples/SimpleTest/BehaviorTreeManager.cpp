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

#include "IBehaviorTreeManager.h"
#include "IBehaviorTree.h"
#include "IObjectUtils.h"

#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/IAssetSystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"

#include <assert.h>
#include <map>
#include <string>

class BehaviorTreeManager : public IBehaviorTreeManager
{
public:
	BehaviorTreeManager() 
	{
		
	}

	virtual ~BehaviorTreeManager()
	{
		if( m_pEntity )
		{
			m_pEntity->SetUpdateable(NULL);
		}

		if (!IsRuntimeDelete())
		{
			DestroyAllTrees();
		}
	}


	// IEntityObject

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		AU_ASSERT(pSerializer);
		IEntityObject::Serialize(pSerializer);
		SERIALIZE(m_trees);
	}

	virtual void Init( bool isFirstInit )
	{
		m_pEntity->SetUpdateable( this );
	}

	// ~IEntityObject

	// IAUUpdateable

	virtual void Update( float deltaTime )
	{
		// Do nothing for now
	}

	// ~IAUUpdateable

	// IBehaviorTreeManager

	virtual IBehaviorTree* GetTree( const char* name )
	{
		IBehaviorTree* pTree = NULL;

		TTreeMap::iterator it = m_trees.find(name);
		if (it != m_trees.end())
		{
			IObjectUtils::GetObject( &pTree, it->second );
		}
		else
		{
			IObject* pObj = IObjectUtils::CreateUniqueObject( name );
			if (pObj)
			{
				IObjectUtils::GetObject( &pTree, pObj->GetObjectId() );
			}
			
			if (pTree)
			{
				m_trees[name] = pTree->GetObjectId();
			}		
		}

		return pTree;
	}

	// ~IBehaviorTreeManager


private:

	void DestroyAllTrees()
	{
		IObjectFactorySystem* pFactory = PerModuleInterface::g_pSystemTable->pObjectFactorySystem;
		TTreeMap::iterator it = m_trees.begin();
		TTreeMap::iterator itEnd = m_trees.end();
		while (it != itEnd)
		{
			IObject* pObj = pFactory->GetObject(it->second);
			delete pObj;
			++it;
		}
	}

	
	// Private Members

	typedef std::map<std::string, ObjectId> TTreeMap;

	TTreeMap m_trees;
};

REGISTERCLASS(BehaviorTreeManager);




