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

#pragma once

#ifndef OBJECTFACTORYSYSTEM_INCLUDED
#define OBJECTFACTORYSYSTEM_INCLUDED

#include "../IObjectFactorySystem.h"
#include "../SimpleSerializer/SimpleSerializer.h"
#include "../RuntimeProtector.h"
#include <map>
#include <string>
#include <set>

// class  ObjectFactorySystem
// implements interface IObjectFactorySystem
// also implements RuntimeProtector so that when new constructors are added and used,
// exceptions can be caught by the runtime system to allow fixing on the fly.
class ObjectFactorySystem : public IObjectFactorySystem , public RuntimeProtector
{
public:
	ObjectFactorySystem()
		: m_pLogger( 0 )
		, m_pRuntimeObjectSystem( 0 )
		, m_pNewConstructors( 0 )
		, m_pSerializer( 0 )
		, m_ProtectedPhase(PHASE_NONE)
 	{
	}

	virtual IObjectConstructor* GetConstructor( const char* type ) const;
	virtual ConstructorId GetConstructorId( const char* type ) const;
	virtual IObjectConstructor* GetConstructor( ConstructorId id ) const;
	virtual void AddConstructors(IAUDynArray<IObjectConstructor*> &constructors);
	virtual void GetAll(IAUDynArray<IObjectConstructor*> &constructors) const;
	virtual IObject* GetObject( ObjectId id ) const;

	virtual void AddListener(IObjectFactoryListener* pListener);
	virtual void RemoveListener(IObjectFactoryListener* pListener);
	virtual void SetLogger( ICompilerLogger * pLogger )
	{
		m_pLogger = pLogger;
	}
    virtual void SetRuntimeObjectSystem( IRuntimeObjectSystem* pRuntimeObjectSystem )
    {
        m_pRuntimeObjectSystem = pRuntimeObjectSystem;
    }


	// RuntimeProtector implementation
	virtual void ProtectedFunc();

private:
	typedef std::map<std::string,ConstructorId> CONSTRUCTORMAP;
	typedef std::set<IObjectFactoryListener*> TObjectFactoryListeners;

	CONSTRUCTORMAP 						m_ConstructorIds;
	std::vector<IObjectConstructor*> 	m_Constructors;
	TObjectFactoryListeners 			m_Listeners;
	ICompilerLogger* 					m_pLogger;
    IRuntimeObjectSystem* 				m_pRuntimeObjectSystem;

	// temp data needed during object swap
	IAUDynArray<IObjectConstructor*>*	m_pNewConstructors;
	std::vector<IObjectConstructor*>	m_PrevConstructors;
	SimpleSerializer*					m_pSerializer;
	enum ProtectedPhase
	{
		PHASE_NONE,
		PHASE_SERIALIZEOUT,
		PHASE_CONSTRUCTNEW,
		PHASE_SERIALIZEIN,
		PHASE_SERIALIZEOUTTEST,
		PHASE_DELETEOLD,
	};
	ProtectedPhase						m_ProtectedPhase;

};


#endif //OBJECTFACTORYSYSTEM_INCLUDED
