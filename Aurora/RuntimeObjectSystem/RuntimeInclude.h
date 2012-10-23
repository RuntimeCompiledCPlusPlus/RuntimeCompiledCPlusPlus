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

//NOTE: the file macro will only emit the full path if /FC option is used in visual studio or /ZI (Which forces /FC)
//Following creates a list of files which are runtime modifiable, to be used in headers
//requires use of __COUNTER__ predefined macro, which is in gcc 4.3+, clang/llvm and MSVC

struct IRuntimeIncludeFileList
{
	virtual const char* GetIncludeFile( unsigned int Num_ ) const
	{
		return 0;
	}
};


namespace
{

template< unsigned int COUNT > struct RuntimeIncludeFiles : public IRuntimeIncludeFileList
{
};


#define RUNTIME_MODIFIABLE_INCLUDE_BASE( N ) \
	template<> struct RuntimeIncludeFiles< N + 1 >  : public RuntimeIncludeFiles< N >\
	{ \
		virtual const char* GetIncludeFile( unsigned int Num_ ) const \
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

#endif //RUNTIMEINCLUDE_INCLUDED
