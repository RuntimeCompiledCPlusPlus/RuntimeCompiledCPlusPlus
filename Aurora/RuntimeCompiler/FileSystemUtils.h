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
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#ifdef _WIN32
	#include <direct.h>
    #include <sys/utime.h>
	#define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
	#undef GetObject
    #undef GetCurrentTime

	#define FILESYSTEMUTILS_SEPERATORS "/\\"
#else
    #include <utime.h>
    #include <string.h>
    #include <unistd.h>
    #include <dirent.h>
	#define FILESYSTEMUTILS_SEPERATORS "/"
#endif


namespace FileSystemUtils
{
#ifdef _WIN32
    typedef __time64_t filetime_t;
#else
    typedef time_t filetime_t;
#endif

	class Path
	{
	public:
		std::string m_string;

		Path()
		{
		}

		Path( const std::string& rhs_ )
			: m_string( rhs_ )
		{
		}

		Path( const char* rhs_ )
			: m_string( rhs_ )
		{
		}
        virtual ~Path() {}  // for RCC++

		const char* c_str()             const;

		Path& operator=( const std::string& rhs_ );
		Path& operator=( const char* rhs_ );

		bool		Exists()			const;
		bool		CreateDir()			const;
		bool		Remove()			const;
		filetime_t	GetLastWriteTime()	const;
        void        SetLastWriteTime( filetime_t time_ ) const;
		uint64_t	GetFileSize()		const;
		bool		HasExtension()		const;
		bool		HasParentPath()		const;
		std::string Extension()			const;

		// returns filename - i.e. part after final seperator: ./dirone/hello.txt > hello.txt ; ./dirone/hello > hello ; ./dirone/ > [empty string]
		Path Filename()					const;
		Path ParentPath()				const;
		Path DelimitersToOSDefault()	const;

		// returns a path cleaned of /../ by removing prior dir
		Path GetCleanPath()				const;
        void ToOSCanonicalCase();  // lower case on Windows, preserve case on Linux

		// replaces extension if one exists, or adds it if not
		void ReplaceExtension( const std::string& ext );

#ifdef _WIN32
		static const char seperator		= '\\';
		static const char altseperator	= '/';
#else
		static const char seperator		= '/';
		static const char altseperator	= '\\';
#endif
	};

	
	inline void ToLowerInPlace( std::string& inout_str )
	{
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

	inline const char* Path::c_str() const
	{
		return m_string.c_str();
	}

	inline Path& Path::operator=( const std::string& rhs_ )
	{
		m_string = rhs_;
		return *this;
	}

	inline Path& Path::operator=( const char* rhs_ )
	{
		m_string = rhs_;
		return *this;
	}

	inline bool Path::Exists() const
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

	inline bool Path::CreateDir() const
	{
        if( m_string.length() == 0 )
        {
            return false;
        }
        if( Exists() )
        {
            return false;
        }

        // we may need to create the parent path recursively
        Path parentpath = ParentPath();
        if( !parentpath.Exists() )
        {
            parentpath.CreateDir();
        }

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

	inline filetime_t Path::GetLastWriteTime()	const
	{
		filetime_t lastwritetime = 0;
		int error = -1;
#ifdef _WIN32
		struct _stat64 buffer;
		error = _stat64( c_str(), &buffer );
#else
		struct stat buffer;
		error = stat( c_str(), &buffer );
#endif
		if( 0 == error )
		{
			lastwritetime = buffer.st_mtime;
		}
		return lastwritetime;
	}

    inline void Path::SetLastWriteTime( filetime_t time_ ) const
    {
#ifdef _WIN32
        __utimbuf64 modtime = { time_, time_ };
        _utime64( c_str(), &modtime );
#else
        utimbuf modtime = { time_, time_ };
        utime( c_str(), &modtime );
#endif
    }

    inline filetime_t GetCurrentTime()
    {
        filetime_t timer;
#ifdef _WIN32
        _time64(&timer);
#else
        time(&timer);
#endif
        return timer;
    }

    inline tm GetTimeStruct( filetime_t time )
    {
        tm ret;
#ifdef _WIN32
        _gmtime64_s(&ret, &time);
#else
        gmtime_r(&time, &ret);
#endif
        return ret;
    }

	inline bool		Path::Remove()			const
	{
		int error = remove( c_str() );
		if( !error )
		{
			return true;
		}
		return false;
	}

	inline uint64_t	Path::GetFileSize()	const
	{
		uint64_t filesize = 0;
		int error = -1;
#ifdef _WIN32
		struct _stat64 buffer;
		error = _stat64( c_str(), &buffer );
#else
		struct stat buffer;
		error = stat( c_str(), &buffer );
#endif
		if( 0 == error )
		{
			filesize = buffer.st_size;
		}
		return filesize;
	}

	inline bool Path::HasExtension() const
	{
		size_t posdot = m_string.find_last_of( '.' );
		if( posdot != std::string::npos )
		{
			size_t posseperator = m_string.find_last_of( seperator );
			if( posseperator != std::string::npos && posseperator > posdot )
			{
				return false; // the dot is for a directory such as ./ or ../ or the path is malformed
			}
			size_t posaltseperator = m_string.find_last_of( altseperator );
			if( posaltseperator != std::string::npos && posaltseperator > posdot )
			{
				return false; // the dot is for a directory such as ./ or ../ or the path is malformed
			}
			return true;
		}
		return false;
	}

	inline bool Path::HasParentPath() const
	{
		size_t posseperator = m_string.find_last_of( seperator );
		if( posseperator != std::string::npos && posseperator > 0 )
		{
			return true;
		}
		size_t posaltseperator = m_string.find_last_of( altseperator );
		if( posaltseperator != std::string::npos && posaltseperator > 0 )
		{
			return true;
		}
		return false;
	}

	inline std::string Path::Extension()			const
	{
		std::string ext;
		if( HasExtension() )
		{
			size_t pos = m_string.find_last_of( '.' );
			if( pos < m_string.length() )
			{
				ext = m_string.substr( pos );
			}
		}
		return ext;
	}



	inline Path Path::Filename() const
	{
		Path filename;
		size_t pos = m_string.find_last_of( seperator ) + 1;
		if( pos <= m_string.length() )
		{
			filename = m_string.substr(pos);
		}

		return filename;
	}

	inline Path Path::ParentPath() const
	{
		Path parentpath = m_string;

        if( parentpath.m_string.length() == 0 )
        {
            return parentpath;
        }
		//remove any trailing seperators
		while( parentpath.m_string.find_last_of( FILESYSTEMUTILS_SEPERATORS ) == parentpath.m_string.length()-1 )
		{
			parentpath.m_string.erase(parentpath.m_string.length()-1, 1);
		}

		size_t pos = parentpath.m_string.find_last_of( FILESYSTEMUTILS_SEPERATORS );
		if( pos < parentpath.m_string.length() )
		{
			parentpath = parentpath.m_string.substr(0, pos);

			//remove any trailing seperators
			while( parentpath.m_string.find_last_of( FILESYSTEMUTILS_SEPERATORS ) == parentpath.m_string.length()-1)
			{
                parentpath.m_string.erase(parentpath.m_string.length()-1, 1);
			}
		}

		return parentpath;
	}
	
	inline Path Path::DelimitersToOSDefault()	const
	{
		Path path = m_string;
		for( size_t i = 0; i < path.m_string.size(); ++i )
		{
			if( path.m_string[i] == altseperator )
			{
				path.m_string[i] = seperator;
			}
		}
		return path;
	}

	inline void Path::ReplaceExtension( const std::string& ext )
	{
		if( HasExtension() )
		{
			size_t pos = m_string.find_last_of( '.' );
			if( pos < m_string.length() )
			{
				m_string.erase( m_string.begin() + pos, m_string.end() );
			}
		}
		m_string += ext;
	}


    inline Path operator/( const Path& lhs_, const Path& rhs_ )
    {
        if( 0 == lhs_.m_string.length() )
        {
            return rhs_;
        }
        if( 0 == rhs_.m_string.length() )
        {
            return lhs_;
        }
        std::string strlhs = lhs_.m_string;
        while( strlhs.length() && strlhs.find_last_of( FILESYSTEMUTILS_SEPERATORS ) == strlhs.length()-1 )
        {
        	strlhs.erase(strlhs.length()-1, 1);
        }
        
        //note: should probably remove preceding seperators to rhs_, but this has not as yet occured
        Path join = strlhs + Path::seperator + rhs_.m_string;
        return join;
    }

	inline bool operator==(  const Path& lhs_, const Path& rhs_ )
	{
		return lhs_.m_string == rhs_.m_string;
	}

	inline bool operator<(  const Path& lhs_, const Path& rhs_ )
	{
		return lhs_.m_string < rhs_.m_string;
	}

	inline Path GetCurrentPath()
	{
		Path currPath;
#ifdef _WIN32
		char currdir[MAX_PATH];
		GetCurrentDirectoryA( sizeof( currdir ), currdir );
		currPath = currdir;
#else
		char* currdir = getcwd(0,0);
		currPath = currdir;
		free( currdir );
#endif
		
		return currPath;
	}

	inline Path Path::GetCleanPath() const
	{
		Path path = m_string;
		bool bFound = false;
		do
		{
			bFound = false;
			size_t pos = path.m_string.find( ".." );
			if( pos != std::string::npos && pos+3 < path.m_string.length() && pos > 0 )
			{
				Path a = path.m_string.substr(0,pos-1); 					 // pos-1 as we don't want delimiter
				Path b = path.m_string.substr(pos+3,path.m_string.length()); // pos+3 as we don't want delimiter
				a = a.ParentPath();
				path = a / b;
				bFound = true;
			}
		} while( bFound );

		return path;
	}

    inline void Path::ToOSCanonicalCase()
    {
#ifdef _WIN32
        ToLowerInPlace( m_string );
#endif
    }


    class PathIterator
    {
    private:
        Path m_dir;
        Path m_path;
        bool m_bIsValid;
#ifdef _WIN32
        void ImpCtor()
        {
            Path test = m_dir / "*";
            m_path = m_dir;
            m_hFind = INVALID_HANDLE_VALUE;
            m_hFind = FindFirstFileA(test.c_str(), &m_ffd);
            m_bIsValid = INVALID_HANDLE_VALUE != m_hFind;
        }
        bool ImpNext()
        {
            if( m_bIsValid )
            {
                m_bIsValid = 0 != FindNextFileA( m_hFind, &m_ffd );
                if( m_bIsValid )
                {
                    m_path = m_dir / m_ffd.cFileName;
                    if( m_path.Filename() == ".." )
                    {
                        return ImpNext();
                    }
                }
            }
            return m_bIsValid;
        }
        void ImpDtor()
        {
            FindClose( m_hFind );
        }

        HANDLE           m_hFind;
        WIN32_FIND_DATAA m_ffd;
#else
        void ImpCtor()
        {
            Path test = m_dir / "*";
            m_path = m_dir;
            m_numFilesInList = scandir( m_path.c_str(), &m_paDirFileList, 0, alphasort);
            m_bIsValid = m_numFilesInList > 0;
            m_currFile = 0;
            if( !m_bIsValid )
            {
                m_paDirFileList = 0;
            }
        }
        bool ImpNext()
        {
            if( m_bIsValid )
            {
                ++m_currFile;
                m_bIsValid = m_currFile < m_numFilesInList;
                if( m_bIsValid )
                {
                    m_path = m_dir / m_paDirFileList[ m_currFile ]->d_name;
                    if( strcmp( m_paDirFileList[ m_currFile ]->d_name, "." )  == 0 ||
                        strcmp( m_paDirFileList[ m_currFile ]->d_name, ".." ) == 0 )
                    {
                        return ImpNext();
                    }
                }
            }
            return m_bIsValid;
        }
        void ImpDtor()
        {
            free(m_paDirFileList);
        }
        struct dirent **m_paDirFileList;
        int           m_numFilesInList;
        int           m_currFile;

#endif
    public:
        PathIterator( const Path& path_ )
        : m_dir( path_ )
        {
            ImpCtor();
        }
        ~PathIterator()
        {
            ImpDtor();
        }
        
        bool operator++()
        {
            return ImpNext();
        }
        
        bool IsValid() const
        {
            return m_bIsValid;
        }
        const Path& GetPath() const
        {
            return m_path;
        }
        
    };


}


inline FileSystemUtils::Path operator/( const std::string& lhs_, const std::string& rhs_ )
{
	//remove any trailing seperators
	FileSystemUtils::Path pathlhs = lhs_;
	FileSystemUtils::Path pathrhs = rhs_;
	return pathlhs / pathrhs;
}
