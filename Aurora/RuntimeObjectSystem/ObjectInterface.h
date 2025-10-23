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

#ifndef OBJECTINTERFACE_INCLUDED
#define OBJECTINTERFACE_INCLUDED

#ifndef RCCPP_ALLOCATOR_INTERFACE
#define RCCPP_ALLOCATOR_INTERFACE 0
#endif

#include <vector>
#include <stdlib.h>

struct SystemTable; //This is the interface to your own engine code, which you need to define yourself if required.
struct IObject;
struct IRuntimeObjectSystem;


const size_t InvalidId = (size_t)-1;
typedef size_t PerTypeObjectId;
typedef size_t ConstructorId;

struct ObjectId
{
	ObjectId() : m_PerTypeId(InvalidId), m_ConstructorId(InvalidId) {}

	PerTypeObjectId m_PerTypeId;
	ConstructorId	m_ConstructorId;
	bool operator<( ObjectId lhs ) const
	{
		if( m_ConstructorId < lhs.m_ConstructorId )
		{
			return true;
		}
		if( m_ConstructorId == lhs.m_ConstructorId )
		{
			return m_PerTypeId < lhs.m_PerTypeId;
		}
		return false;
	}
	bool operator==( const ObjectId& rhs) const
	{
		return (m_ConstructorId == rhs.m_ConstructorId && m_PerTypeId == rhs.m_PerTypeId);
	}
	bool IsValid() const
	{
		return (m_ConstructorId != InvalidId && m_PerTypeId != InvalidId);
	}
	void SetInvalid() 
	{
		m_ConstructorId = InvalidId;
		m_PerTypeId = InvalidId;
	}
};

#if RCCPP_ALLOCATOR_INTERFACE
struct IObjectAllocator
{
	virtual ~IObjectAllocator() = default;
	virtual void* Allocate( size_t size, size_t alignment ) = 0;
	virtual void Free( void* p ) = 0;
};
#endif

struct RuntimeTackingInfo;

struct IObjectConstructor
{
#if RCCPP_ALLOCATOR_INTERFACE
	// Memory management
	virtual IObjectAllocator* GetAllocator() const = 0;
	virtual void SetAllocator( IObjectAllocator* pAllocator ) = 0;
#endif

	virtual IObject* Construct() = 0;
	virtual void ConstructNull() = 0;	//for use in object replacement, ensures a deleted object can be replaced
	virtual void Destroy( IObject* object ) = 0;
	virtual const char* GetName() = 0;
	virtual const char* GetFileName() = 0;
	virtual const char* GetCompiledPath() = 0;
	virtual size_t GetMaxNumTrackingInfo() const = 0;
	virtual RuntimeTackingInfo GetTrackingInfo( size_t Num_ ) const = 0;
    virtual void SetProjectId( unsigned short projectId_ ) = 0;
    virtual unsigned short GetProjectId() const = 0;

    // Singleton functions
    virtual bool        GetIsSingleton() const = 0;
    virtual bool        GetIsAutoConstructSingleton() const = 0;
    IObject*            GetSingleton()
    {
        return Construct();
    }

	virtual IObject* GetConstructedObject( PerTypeObjectId num ) const = 0;	//should return 0 for last or deleted object
	virtual size_t	 GetNumberConstructedObjects() const = 0;
	virtual ConstructorId GetConstructorId() const = 0;
	virtual void SetConstructorId( ConstructorId id ) = 0;					//take care how you use this - should only be used by id service
	virtual void ClearIfAllDeleted() = 0;									//if there are no objects left then clear internal memory (does not reduce memory consumption)
	virtual ~IObjectConstructor() {}
};

struct IPerModuleInterface
{
	virtual std::vector<IObjectConstructor*>& GetConstructors() = 0;
    virtual void SetProjectIdForAllConstructors( unsigned short projectId_ ) = 0;
	virtual void SetSystemTable( SystemTable* pSystemTable ) = 0;
	virtual void SetRuntimeObjectSystem( IRuntimeObjectSystem* pIRuntimeObjectSystem_ ) = 0;
	virtual const std::vector<const char*>& GetRequiredSourceFiles() const = 0;
	virtual void AddRequiredSourceFiles( const char* file_ ) = 0;
    virtual void SetModuleFileName( const char* name ) = 0;
	virtual ~IPerModuleInterface() {}
};

#ifdef _WIN32
typedef IPerModuleInterface* (__cdecl *GETPerModuleInterface_PROC)(void);
#else
typedef IPerModuleInterface* ( *GETPerModuleInterface_PROC)(void);
#endif


#endif //OBJECTINTERFACE_INCLUDED
