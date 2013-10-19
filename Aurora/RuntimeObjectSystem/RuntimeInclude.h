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

#ifndef RUNTIMEINCLUDE_INCLUDED
#define RUNTIMEINCLUDE_INCLUDED

#include <stddef.h>

#ifndef RCCPPOFF

//NOTE: the file macro will only emit the full path if /FC option is used in visual studio or /ZI (Which forces /FC)
//Following creates a list of files which are runtime modifiable, to be used in headers
//requires use of __COUNTER__ predefined macro, which is in gcc 4.3+, clang/llvm and MSVC

struct IRuntimeIncludeFileList
{
	IRuntimeIncludeFileList( size_t max ) : MaxNum( max )
	{
	}

	// GetIncludeFile may return 0, so you should iterate through to GetMaxNum() ignoring 0 returns
	virtual const char* GetIncludeFile( size_t Num_ ) const
	{
		return 0;
	}
	size_t MaxNum; // initialized in constructor below
};


namespace
{

template< size_t COUNT > struct RuntimeIncludeFiles : public RuntimeIncludeFiles<COUNT-1>
{
	RuntimeIncludeFiles( size_t max ) : RuntimeIncludeFiles<COUNT-1>( max )
	{
	}
	RuntimeIncludeFiles() : RuntimeIncludeFiles<COUNT-1>( COUNT )
	{
	}

	virtual const char* GetIncludeDir( size_t Num_ ) const
	{
		if( Num_ < COUNT )
		{
			return this->RuntimeIncludeFiles< COUNT-1 >::GetIncludeDir( Num_ );
		}
		else return 0;
	}
};

template<> struct RuntimeIncludeFiles<0> : public IRuntimeIncludeFileList
{
	RuntimeIncludeFiles( size_t max ) : IRuntimeIncludeFileList( max )
	{
	}
	RuntimeIncludeFiles() : IRuntimeIncludeFileList( 0 )
	{
	}

	virtual const char* GetIncludeDir( size_t Num_ ) const
	{
		return 0;
	} 
};



#define RUNTIME_MODIFIABLE_INCLUDE_BASE( N ) \
	template<> struct RuntimeIncludeFiles< N + 1 >  : public RuntimeIncludeFiles< N >\
	{ \
		RuntimeIncludeFiles( size_t max ) : RuntimeIncludeFiles<N>( max ) {} \
		RuntimeIncludeFiles< N + 1 >() : RuntimeIncludeFiles<N>( N + 1 ) {} \
		virtual const char* GetIncludeFile( size_t Num_ ) const \
		{ \
			if( Num_ <= N ) \
			{ \
				if( Num_ == N ) \
				{ \
					return __FILE__; \
				} \
				else return this->RuntimeIncludeFiles< N >::GetIncludeFile( Num_ ); \
			} \
			else return 0; \
		} \
	}; \


#define RUNTIME_MODIFIABLE_INCLUDE namespace { RUNTIME_MODIFIABLE_INCLUDE_BASE( __COUNTER__ ) }

}
#else
#define RUNTIME_MODIFIABLE_INCLUDE
#endif //RCCPPOFF

#endif //RUNTIMEINCLUDE_INCLUDED
