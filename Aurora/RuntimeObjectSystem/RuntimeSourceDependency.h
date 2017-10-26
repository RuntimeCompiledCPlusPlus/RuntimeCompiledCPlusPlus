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

#include "RuntimeTracking.h"

#ifndef RCCPPOFF


#define RUNTIME_COMPILER_SOURCEDEPENDENCY_BASE( SOURCEFILE, SOURCEEXT, RELATIVEPATHTO, N ) \
RCCPP_OPTMIZE_OFF \
template<> struct RuntimeTracking< N + 1 >  : RuntimeTracking< N >\
{ \
	RuntimeTracking( size_t max ) : RuntimeTracking<N>( max ) {} \
	RuntimeTracking< N + 1 >() : RuntimeTracking<N>( N + 1 ) {} \
	virtual RuntimeTackingInfo GetTrackingInfo( size_t Num_ ) const \
	{ \
		if( Num_ <= N ) \
		{ \
			if( Num_ == N ) \
			{ \
				RuntimeTackingInfo info = RuntimeTackingInfo::GetNULL(); \
				info.sourceDependencyInfo = { SOURCEFILE, SOURCEEXT, RELATIVEPATHTO }; \
				return info; \
			} \
			else return this->RuntimeTracking< N >::GetTrackingInfo( Num_ ); \
		} \
		else return RuntimeTackingInfo::GetNULL(); \
	} \
}; \
RCCPP_OPTMIZE_ON

// The RUNTIME_COMPILER_SOURCEDEPENDENCY macro will return the name of the current file, which should be a header file.
// The runtime system will strip off the extension and add .cpp
#define RUNTIME_COMPILER_SOURCEDEPENDENCY namespace { RUNTIME_COMPILER_SOURCEDEPENDENCY_BASE( __FILE__, ".cpp", 0, __COUNTER__ - COUNTER_OFFSET ) }

// if you want to specify another extension use this version:
#define RUNTIME_COMPILER_SOURCEDEPENDENCY_EXT( EXT_ )  namespace { RUNTIME_COMPILER_SOURCEDEPENDENCY_BASE( __FILE__, EXT_, 0, __COUNTER__ - COUNTER_OFFSET ) }

// for complete freedom of which file to specify, use this version (FILE_ is relative to current file path):
#define RUNTIME_COMPILER_SOURCEDEPENDENCY_FILE( FILE_, EXT_ )  namespace { RUNTIME_COMPILER_SOURCEDEPENDENCY_BASE( FILE_, EXT_, __FILE__, __COUNTER__ - COUNTER_OFFSET ) }

#else
#define RUNTIME_COMPILER_SOURCEDEPENDENCY
#define RUNTIME_COMPILER_SOURCEDEPENDENCY_EXT( EXT_ )
#define RUNTIME_COMPILER_SOURCEDEPENDENCY_FILE( FILE_, EXT_ )
#endif //RCCPPOFF

#endif //RUNTIMESOURCEDEPENDENCY_INCLUDED
