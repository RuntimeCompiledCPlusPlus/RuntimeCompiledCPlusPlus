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

#ifndef RUNTIMELINKLIBRARY_INCLUDED
#define RUNTIMELINKLIBRARY_INCLUDED


#ifndef RCCPPOFF
//NOTE: the file macro will only emit the full path if /FC option is used in visual studio or /ZI (Which forces /FC)
//Following creates a list of files which are runtime modifiable, to be used in headers
//requires use of __COUNTER__ predefined macro, which is in gcc 4.3+, clang/llvm and MSVC

struct IRuntimeLinkLibraryList
{
	IRuntimeLinkLibraryList( size_t max ) : MaxNum( max )
	{
	}

	// GetIncludeFile may return 0, so you should iterate through to GetMaxNum() ignoring 0 returns
	virtual const char* GetLinkLibrary( size_t Num_ ) const
	{
		return 0;
	}
	size_t MaxNum; // initialized in constructor below
};


namespace
{

template< size_t COUNT > struct RuntimeLinkLibrary : public RuntimeLinkLibrary<COUNT-1>
{
	RuntimeLinkLibrary( size_t max ) : RuntimeLinkLibrary<COUNT-1>( max )
	{
	}
	RuntimeLinkLibrary() : RuntimeLinkLibrary<COUNT-1>( COUNT )
	{
	}

	virtual const char* GetLinkLibrary( size_t Num_ ) const
	{
		if( Num_ < COUNT )
		{
			return this->RuntimeLinkLibrary< COUNT-1 >::GetLinkLibrary( Num_ );
		}
		else return 0;
	}
};

template<> struct RuntimeLinkLibrary<0> : public IRuntimeLinkLibraryList
{
	RuntimeLinkLibrary( size_t max ) : IRuntimeLinkLibraryList( max )
	{
	}
	RuntimeLinkLibrary() : IRuntimeLinkLibraryList( 0 )
	{
	}

	virtual const char* GetLinkLibrary( size_t Num_ ) const
	{
		return 0;
	} 
};



#define RUNTIME_COMPILER_LINKLIBRARY_BASE( LIBRARY, N ) \
	template<> struct RuntimeLinkLibrary< N + 1 >  : public RuntimeLinkLibrary< N >\
	{ \
		RuntimeLinkLibrary( size_t max ) : RuntimeLinkLibrary<N>( max ) {} \
		RuntimeLinkLibrary< N + 1 >() : RuntimeLinkLibrary<N>( N + 1 ) {} \
		virtual const char* GetLinkLibrary( size_t Num_ ) const \
		{ \
			if( Num_ <= N ) \
			{ \
				if( Num_ == N ) \
				{ \
					return LIBRARY; \
				} \
				else return this->RuntimeLinkLibrary< N >::GetLinkLibrary( Num_ ); \
			} \
			else return 0; \
		} \
	}; \


#define RUNTIME_COMPILER_LINKLIBRARY( LIBRARY ) namespace { RUNTIME_COMPILER_LINKLIBRARY_BASE( LIBRARY, __COUNTER__ ) }

}
#else
#define RUNTIME_COMPILER_LINKLIBRARY( LIBRARY )
#endif //RCCPPOFF


#endif //RUNTIMELINKLIBRARY_INCLUDED
