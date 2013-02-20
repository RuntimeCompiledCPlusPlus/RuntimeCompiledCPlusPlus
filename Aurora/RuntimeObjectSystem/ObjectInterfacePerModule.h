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

#ifndef OBJECTINTERFACEPERMODULE_INCLUDED
#define OBJECTINTERFACEPERMODULE_INCLUDED

#include "ObjectInterface.h"
#include "RuntimeInclude.h"
#include "RuntimeLinkLibrary.h"
#include <string>
#include <vector>
#include <assert.h>

#define AU_ASSERT( statement )  do { if (!(statement)) { volatile int* p = 0; int a = *p; if(a) {} } } while(0)

class PerModuleInterface : public IPerModuleInterface
{
public:
	static PerModuleInterface*  GetInstance();
	static SystemTable*			g_pSystemTable;

	void AddConstructor( IObjectConstructor* pConstructor );

	virtual std::vector<IObjectConstructor*>& GetConstructors();
	virtual void SetSystemTable( SystemTable* pSystemTable );

	SystemTable* GetSystemTable()
	{
		return g_pSystemTable;
	}

	virtual const std::vector<const char*>& GetRequiredSourceFiles() const;
	virtual void AddRequiredSourceFiles( const char* file_ );
    virtual void SetModuleFileName( const char* name )
    {
        m_ModuleFilename = name;
    }

private:
	PerModuleInterface();

	~PerModuleInterface()
	{
	}


	static PerModuleInterface*			ms_pObjectManager;
	std::vector<IObjectConstructor*>	m_ObjectConstructors;
	std::vector<const char*>			m_RequiredSourceFiles;
    std::string                         m_ModuleFilename;
};





template<typename T> class TObjectConstructorConcrete: public IObjectConstructor
{
public:
	TObjectConstructorConcrete(
		const char* Filename,
		IRuntimeIncludeFileList* pIncludeFileList_,
		IRuntimeLinkLibraryList* pLinkLibraryList )
		: m_FileName( Filename )
		, m_pIncludeFileList( pIncludeFileList_ )
		, m_pLinkLibraryList( pLinkLibraryList )
        , m_pModuleInterface(0)
	{
		PerModuleInterface::GetInstance()->AddConstructor( this );
        m_pModuleInterface = PerModuleInterface::GetInstance();
		m_Id = InvalidId;
	}

	virtual IObject* Construct()
	{
		T* pT = 0;
		if( m_FreeIds.empty() )
		{
			PerTypeObjectId id = m_ConstructedObjects.size();

			pT = new T();
			pT->SetPerTypeId( id );
			m_ConstructedObjects.push_back( pT );
		}
		else
		{
			PerTypeObjectId id = m_FreeIds.back();
			m_FreeIds.pop_back();
			pT = new T();
			pT->SetPerTypeId( id );
			AU_ASSERT( 0 == m_ConstructedObjects[ id ] );
			m_ConstructedObjects[ id ] = pT;

		}
		return pT;
	}

	virtual void ConstructNull()
	{
		m_ConstructedObjects.push_back( NULL );
	}

	virtual const char* GetName()
	{
		return T::GetTypeNameStatic();
	}

	virtual const char* GetFileName()
	{
		return m_FileName.c_str();
	}
	virtual const char* GetIncludeFile( size_t Num_ ) const
	{
		if( m_pIncludeFileList )
		{
			return m_pIncludeFileList->GetIncludeFile( Num_ );
		}
		return 0;
	}

	virtual size_t GetMaxNumIncludeFiles() const
	{
		if( m_pIncludeFileList )
		{
			return m_pIncludeFileList->MaxNum;
		}
		return 0;
	}
	virtual const char* GetLinkLibrary( size_t Num_ ) const
	{
		if( m_pLinkLibraryList )
		{
			return m_pLinkLibraryList->GetLinkLibrary( Num_ );
		}
		return 0;
	}

	virtual size_t GetMaxNumLinkLibraries() const
	{
		if( m_pLinkLibraryList )
		{
			return m_pLinkLibraryList->MaxNum;
		}
		return 0;
	}

	virtual IObject* GetConstructedObject( PerTypeObjectId id ) const
	{
		if( m_ConstructedObjects.size() > id )
		{
			return m_ConstructedObjects[id];
		}
		return 0;
	}
	virtual size_t	 GetNumberConstructedObjects() const
	{
		return m_ConstructedObjects.size();
	}
	virtual ConstructorId GetConstructorId() const
	{
		return m_Id;
	}
	virtual void SetConstructorId( ConstructorId id )
	{
		if( InvalidId == m_Id )
		{
			m_Id = id;
		}
	}

	void DeRegister( PerTypeObjectId id )
	{
		//remove from constructed objects.
		//use swap with last one
		if( m_ConstructedObjects.size() - 1 == id )
		{
			//it's the last one, just remove it.
			m_ConstructedObjects.pop_back();
		}
		else
		{
			m_FreeIds.push_back( id );
			m_ConstructedObjects[ id ] = 0;
		}
	}
private:
	std::string				m_FileName;
	std::vector<T*>			m_ConstructedObjects;
	std::vector<PerTypeObjectId>	m_FreeIds;
	ConstructorId			m_Id;
	IRuntimeIncludeFileList* m_pIncludeFileList;
	IRuntimeLinkLibraryList* m_pLinkLibraryList;
    PerModuleInterface*      m_pModuleInterface;
};


template<typename T> class TActual: public T
{
public:
	// overload new/delete to get alignment correct
#ifdef _WIN32
	void* operator new(size_t size)
	{
		size_t align = __alignof( TActual );
		return _aligned_malloc( size, align );
	}
	void operator delete(void* p)
	{
		_aligned_free( p );
	}
#else
#ifdef __APPLE_CC__
	void* operator new(size_t size)
	{
		size_t align = __alignof__( TActual<T> );
		void* pRet;
		posix_memalign( &pRet, align, size );
		return pRet;
	}
	void operator delete(void* p)
	{
		free( p );
	}
#else
	void* operator new(size_t size)
	{
		size_t align = __alignof__( TActual );
		return memalign( size, align );	}
	}
    void operator delete(void* p)
    {
        free( p );
    }
#endif //__APPLE_CC__
#endif //_WIN32
	friend class TObjectConstructorConcrete<TActual>;
	virtual ~TActual() { m_Constructor.DeRegister( m_Id ); }
	virtual PerTypeObjectId GetPerTypeId() const { return m_Id; }
	virtual IObjectConstructor* GetConstructor() const { return &m_Constructor; }
	static const char* GetTypeNameStatic();
	virtual const char* GetTypeName() const
	{
		return GetTypeNameStatic();
	}
private:
	void SetPerTypeId( PerTypeObjectId id ) { m_Id = id; }
	PerTypeObjectId m_Id;
	static TObjectConstructorConcrete<TActual> m_Constructor;
};

//NOTE: the file macro will only emit the full path if /FC option is used in visual studio or /ZI (Which forces /FC)
#define REGISTERCLASS( T )	\
	static RuntimeIncludeFiles< __COUNTER__ > g_includeFileList_##T; \
	static RuntimeLinkLibrary< __COUNTER__ > g_linkLibraryList_##T; \
template<> TObjectConstructorConcrete< TActual< T > > TActual< T >::m_Constructor( __FILE__, &g_includeFileList_##T, &g_linkLibraryList_##T );\
template<> const char* TActual< T >::GetTypeNameStatic() { return #T; } \
template class TActual< T >; \

#endif // OBJECTINTERFACEPERMODULE_INCLUDED