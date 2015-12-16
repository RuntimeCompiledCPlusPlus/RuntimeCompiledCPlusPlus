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

// Source Dependencies are constructed from a macro template from sources which may include
// the __FILE__ macro, so to reduce inter-dependencies we return three values which are combined
// by the higher level code. The full source dependency filename is then pseudo-code:
// RemoveAnyFileName( relativeToPath ) + ReplaceExtension( filename, extension  )
struct SourceDependencyInfo
{
    static SourceDependencyInfo GetNULL() { SourceDependencyInfo ret = {0,0,0}; return ret; }
	const char* filename;			// If NULL then no SourceDependencyInfo
	const char* extension;			// If NULL then use extension in filename
	const char* relativeToPath;		// If NULL filename is either full or relative to known path
};

#ifndef RCCPPOFF

struct IRuntimeSourceDependencyList
{
	IRuntimeSourceDependencyList( size_t max ) : MaxNum( max )
	{
	}

	// GetIncludeFile may return 0, so you should iterate through to GetMaxNum() ignoring 0 returns
	virtual SourceDependencyInfo GetSourceDependency( size_t Num_ ) const
	{
		return SourceDependencyInfo::GetNULL();
	}

	size_t MaxNum; // initialized in constructor below
};


namespace
{

template< size_t COUNT > struct RuntimeSourceDependency : RuntimeSourceDependency<COUNT-1>
{
	RuntimeSourceDependency( size_t max ) : RuntimeSourceDependency<COUNT-1>( max )
	{
	}
	RuntimeSourceDependency() : RuntimeSourceDependency<COUNT-1>( COUNT )
	{
	}

	virtual SourceDependencyInfo GetSourceDependency( size_t Num_ ) const
	{
		if( Num_ < COUNT )
		{
			return this->RuntimeSourceDependency< COUNT-1 >::GetSourceDependency( Num_ );
		}
		else return SourceDependencyInfo::GetNULL();
	}
};

template<> struct RuntimeSourceDependency<0> : IRuntimeSourceDependencyList
{
	RuntimeSourceDependency( size_t max ) : IRuntimeSourceDependencyList( max )
	{
	}
	RuntimeSourceDependency() : IRuntimeSourceDependencyList( 0 )
	{
	}

	virtual SourceDependencyInfo GetSourceDependency( size_t Num_ ) const
	{
		return SourceDependencyInfo::GetNULL();
	} 

};



#define RUNTIME_COMPILER_SOURCEDEPENDENCY_BASE( SOURCEFILE, SOURCEEXT, RELATIVEPATHTO, N ) \
	template<> struct RuntimeSourceDependency< N + 1 >  : RuntimeSourceDependency< N >\
	{ \
		RuntimeSourceDependency( size_t max ) : RuntimeSourceDependency<N>( max ) {} \
		RuntimeSourceDependency< N + 1 >() : RuntimeSourceDependency<N>( N + 1 ) {} \
		virtual SourceDependencyInfo GetSourceDependency( size_t Num_ ) const \
		{ \
			if( Num_ <= N ) \
			{ \
				if( Num_ == N ) \
				{ \
					return { SOURCEFILE, SOURCEEXT, RELATIVEPATHTO }; \
				} \
				else return this->RuntimeSourceDependency< N >::GetSourceDependency( Num_ ); \
			} \
			else return SourceDependencyInfo::GetNULL(); \
		} \
	}; \

// The RUNTIME_COMPILER_SOURCEDEPENDENCY macro will return the name of the current file, which should be a header file.
// The runtime system will strip off the extension and add .cpp
#define RUNTIME_COMPILER_SOURCEDEPENDENCY namespace { RUNTIME_COMPILER_SOURCEDEPENDENCY_BASE( __FILE__, ".cpp", 0, __COUNTER__ ) }

// if you want to specify another extension use this version:
#define RUNTIME_COMPILER_SOURCEDEPENDENCY_EXT( EXT_ )  namespace { RUNTIME_COMPILER_SOURCEDEPENDENCY_BASE( __FILE__, EXT_, 0, __COUNTER__ ) }

// for complete freedom of which file to specify, use this version (FILE_ is relative to current file path):
#define RUNTIME_COMPILER_SOURCEDEPENDENCY_FILE( FILE_, EXT_ )  namespace { RUNTIME_COMPILER_SOURCEDEPENDENCY_BASE( FILE_, EXT_, __FILE__, __COUNTER__ ) }

}
#else
#define RUNTIME_COMPILER_SOURCEDEPENDENCY
#endif //RCCPPOFF

#endif //RUNTIMESOURCEDEPENDENCY_INCLUDED
