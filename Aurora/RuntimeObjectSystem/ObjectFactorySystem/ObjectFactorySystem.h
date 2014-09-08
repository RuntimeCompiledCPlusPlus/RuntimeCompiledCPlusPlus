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
class ObjectFactorySystem : public IObjectFactorySystem
{
public:
	ObjectFactorySystem()
		: m_pLogger( 0 )
		, m_pRuntimeObjectSystem( 0 )
        , m_bTestSerialization(true)
		, m_HistoryMaxSize( 0 )
		, m_HistoryCurrentLocation( 0 )
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
    virtual void SetTestSerialization( bool bTest )
    {
        m_bTestSerialization = bTest;
    }
    virtual bool GetTestSerialization() const
    {
        return m_bTestSerialization;
    }

	virtual void				SetObjectConstructorHistorySize( int num_ );
	virtual int					GetObjectConstructorHistorySize();
	virtual bool				UndoObjectConstructorChange();
	virtual bool				RedoObjectConstructorChange();
	virtual int					GetObjectContstructorHistoryLocation();


private:
	typedef std::map<std::string,ConstructorId> CONSTRUCTORMAP;
	typedef std::set<IObjectFactoryListener*>	TObjectFactoryListeners;
	typedef std::vector<IObjectConstructor*>	TConstructors;

	CONSTRUCTORMAP 						m_ConstructorIds;
	TConstructors 						m_Constructors;
	TObjectFactoryListeners 			m_Listeners;
	ICompilerLogger* 					m_pLogger;
    IRuntimeObjectSystem* 				m_pRuntimeObjectSystem;
	bool                                m_bTestSerialization;

	// History
	int									m_HistoryMaxSize;
	int									m_HistoryCurrentLocation;	// positive non-zero number means previous
	struct HistoryPoint
	{
		TConstructors before;
		TConstructors after;
	};
	std::vector<HistoryPoint>			m_HistoryConstructors;

	bool HandleRedoUndo( const TConstructors& constructors );

	enum ProtectedPhase
	{
		PHASE_NONE,
		PHASE_SERIALIZEOUT,
		PHASE_CONSTRUCTNEW,
		PHASE_SERIALIZEIN,
		PHASE_AUTOCONSTRUCTSINGLETONS,
		PHASE_INITANDSERIALIZEOUTTEST,
		PHASE_DELETEOLD,
	};

	// temp data needed during object swap
	struct ProtectedObjectSwapper:  public RuntimeProtector
	{
		TConstructors						m_ConstructorsToAdd;
		TConstructors						m_ConstructorsOld;
		TConstructors						m_ConstructorsReplaced;
		SimpleSerializer					m_Serializer;
		ICompilerLogger*					m_pLogger;
		ObjectFactorySystem*				m_pObjectFactorySystem;
		bool								m_bTestSerialization;

		ProtectedPhase						m_ProtectedPhase;

		// RuntimeProtector implementation
		virtual void ProtectedFunc();
	};
	friend struct ProtectedObjectSwapper;

	void CompleteConstructorSwap( ProtectedObjectSwapper& swapper );
};


#endif //OBJECTFACTORYSYSTEM_INCLUDED
