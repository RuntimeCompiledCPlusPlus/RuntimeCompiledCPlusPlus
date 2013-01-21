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

// we use std::string as there are many string types in the community
// and this can be easily swapped for your own implementation if desired
#include <string>
#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#endif

namespace FileSystemUtils
{

	class Path
	{
	public:
		std::string m_string;

		Path( const std::string& rhs_ )
			: m_string( rhs_ )
		{
		}

		const char* c_str() const;

		Path& operator=( const std::string& rhs_ );
		Path& operator=( const char* rhs_ );

		bool Exists()		const;
		bool CreateDir()	const;

#ifdef _WIN32
		static const char seperator = '\\';
#else
		static const char seperator = '/';
#endif
	};

	
	inline void ToLowerInPlace( std::string& inout_str )
	{
		if( 0 == inout_str.size() )
		{
			return;
		}
		for( size_t i = 0; i < inout_str.size(); ++i )
		{
			if( inout_str[i] >= 'A' && inout_str[i] <= 'Z' )
			{
				//tolower
				inout_str[i] -= 'A'-'a';
			}
		}
	}

	///////////////////////////////////////////////////////////////////
	// Path function definitions

	const char* Path::c_str() const
	{
		return m_string.c_str();
	}

	Path& Path::operator=( const std::string& rhs_ )
	{
		m_string = rhs_;
		return *this;
	}

	Path& Path::operator=( const char* rhs_ )
	{
		m_string = rhs_;
		return *this;
	}

	bool Path::Exists() const
	{

		int error = -1;
#ifdef _WIN32
		struct _stat buffer;
		error = _stat( m_string.c_str(), &buffer );
#else
		struct stat buffer;
		error = stat( m_string.c_str(), &buffer );
#endif
		if( 0 == error )
		{
			return true;
		}
		return false;
	}

	bool Path::CreateDir() const
	{
		int error = -1;
#ifdef _WIN32
		error = _mkdir( m_string.c_str() );
#else
		error = mkdir( m_string.c_str(), 0777 );
#endif
		if( 0 == error )
		{
			return true;
		}
		return false;
	}

	Path operator/( const Path& lhs_, const Path& rhs_ )
	{
		Path join = lhs_.m_string + Path::seperator + rhs_.m_string;
		return join;
	}
}


FileSystemUtils::Path operator/( const std::string& lhs_, const std::string& rhs_ )
{
	FileSystemUtils::Path join = lhs_ + FileSystemUtils::Path::seperator + rhs_;
	return join;
}