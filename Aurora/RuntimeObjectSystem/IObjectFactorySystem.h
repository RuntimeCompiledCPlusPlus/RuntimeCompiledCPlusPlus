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

#ifndef IOBJECTFACTORYSYSTEM_INCLUDED
#define IOBJECTFACTORYSYSTEM_INCLUDED


#include "../Common/AUArray.inl"
#include "../RuntimeObjectSystem/ObjectInterface.h"
#include "../RuntimeCompiler/ICompilerLogger.h"

struct IObjectFactoryListener
{
	// Called after a full serialization of objects is done when a new
	// object constructor is added, so listeners can update any object
	// pointers they're holding
	virtual void OnConstructorsAdded() = 0; 
};

struct IObjectFactorySystem
{
	virtual IObjectConstructor* GetConstructor( const char* type ) const = 0;
	virtual ConstructorId GetConstructorId( const char* type ) const = 0;
	virtual IObjectConstructor* GetConstructor( ConstructorId id ) const = 0;
	virtual void AddConstructors(IAUDynArray<IObjectConstructor*> &constructors) = 0;
	virtual void GetAll(IAUDynArray<IObjectConstructor*> &constructors) const = 0;
	virtual IObject* GetObject( ObjectId id ) const = 0;

	virtual void AddListener(IObjectFactoryListener* pListener) = 0;
	virtual void RemoveListener(IObjectFactoryListener* pListener) = 0;
	virtual void SetLogger( ICompilerLogger * pLogger ) = 0;

};


#endif //IOBJECTFACTORYSYSTEM_INCLUDED