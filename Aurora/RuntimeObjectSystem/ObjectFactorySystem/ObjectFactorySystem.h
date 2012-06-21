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
#include <map>
#include <string>
#include <set>


class ObjectFactorySystem : public IObjectFactorySystem
{
public:
	ObjectFactorySystem()
		: m_pLogger( 0 )
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


private:
	typedef std::map<std::string,ConstructorId> CONSTRUCTORMAP;
	typedef std::set<IObjectFactoryListener*> TObjectFactoryListeners;

	CONSTRUCTORMAP m_ConstructorIds;
	std::vector<IObjectConstructor*> m_Constructors;
	TObjectFactoryListeners m_Listeners;
	ICompilerLogger* m_pLogger;
};


#endif //OBJECTFACTORYSYSTEM_INCLUDED