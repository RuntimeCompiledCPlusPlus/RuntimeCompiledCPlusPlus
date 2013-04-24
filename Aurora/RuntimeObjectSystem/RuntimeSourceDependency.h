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

#ifndef RUNTIMESOURCEDEPENDENCY_INCLUDED
#define RUNTIMESOURCEDEPENDENCY_INCLUDED

//NOTE: the file macro will only emit the full path if /FC option is used in visual studio or /ZI (Which forces /FC)
//Following creates a list of files which are runtime modifiable, to be used in headers
//requires use of __COUNTER__ predefined macro, which is in gcc 4.3+, clang/llvm and MSVC
#include "../RuntimeCompiler/FileSystemUtils.h"

struct IRuntimeSourceDependancyList
{
	IRuntimeSourceDependancyList( size_t max ) : MaxNum( max )
	{
	}

	// GetIncludeFile may return 0, so you should iterate through to GetMaxNum() ignoring 0 returns
	virtual const char* GetSourceDependency( size_t Num_ ) const
	{
		return 0;
	}
	size_t MaxNum; // initialized in constructor below
};


namespace
{

template< size_t COUNT > struct RuntimeSourceDependancy : public RuntimeSourceDependancy<COUNT-1>
{
	RuntimeSourceDependancy( size_t max ) : RuntimeSourceDependancy<COUNT-1>( max )
	{
	}
	RuntimeSourceDependancy() : RuntimeSourceDependancy<COUNT-1>( COUNT )
	{
	}

	virtual const char* GetSourceDependency( size_t Num_ ) const
	{
		if( Num_ < COUNT )
		{
			return this->RuntimeSourceDependancy< COUNT-1 >::GetSourceDependency( Num_ );
		}
		else return 0;
	}
};

template<> struct RuntimeSourceDependancy<0> : public IRuntimeSourceDependancyList
{
	RuntimeSourceDependancy( size_t max ) : IRuntimeSourceDependancyList( max )
	{
	}
	RuntimeSourceDependancy() : IRuntimeSourceDependancyList( 0 )
	{
	}

	virtual const char* GetSourceDependency( size_t Num_ ) const
	{
		return 0;
	} 
};



#define RUNTIME_COMPILER_SOURCEDEPENENCY_BASE( SOURCEFILE, N ) \
	template<> struct RuntimeSourceDependancy< N + 1 >  : public RuntimeSourceDependancy< N >\
	{ \
		RuntimeSourceDependancy( size_t max ) : RuntimeSourceDependancy<N>( max ) {} \
		RuntimeSourceDependancy< N + 1 >() : RuntimeSourceDependancy<N>( N + 1 ) {} \
		virtual const char* GetSourceDependency( size_t Num_ ) const \
		{ \
			if( Num_ <= N ) \
			{ \
				if( Num_ == N ) \
				{ \
					return SOURCEFILE; \
				} \
				else return this->RuntimeSourceDependancy< N >::GetSourceDependency( Num_ ); \
			} \
			else return 0; \
		} \
	}; \

// The RUNTIME_COMPILER_SOURCEDEPENENCY macro will return the name of the current file, which should be a header file.
// The runtime system will strip off the extension and add .cpp
#define RUNTIME_COMPILER_SOURCEDEPENENCY namespace { RUNTIME_COMPILER_SOURCEDEPENENCY_BASE( __FILE__, __COUNTER__ ) }

}

#endif //RUNTIMESOURCEDEPENDENCY_INCLUDED
