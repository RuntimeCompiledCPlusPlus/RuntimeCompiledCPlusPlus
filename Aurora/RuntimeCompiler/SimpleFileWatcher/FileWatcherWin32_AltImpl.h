//
// Copyright (c) 2013 Márton Tamás
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

// Minor modifications by Doug Binks
// This implementation for FileWatcherWin32 uses FindFirstChangeNotification rather
// than ReadDirectoryChanges

#ifndef _FileWatcherWin32_AltImpl_h_
#define _FileWatcherWin32_AltImpl_h_
#pragma once


#if FILEWATCHER_PLATFORM == FILEWATCHER_PLATFORM_WIN32

#include <iostream>
#include <vector>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace FW
{
    const static int fw_types[] = { FILE_NOTIFY_CHANGE_LAST_WRITE, FILE_NOTIFY_CHANGE_CREATION, FILE_NOTIFY_CHANGE_SIZE, FILE_NOTIFY_CHANGE_FILE_NAME };
    const static size_t NUMTYPES = sizeof(fw_types)/sizeof(int);

    class FileWatcherWin32_AltImpl
    {
      struct filedata
      {
        std::string fname;
        LARGE_INTEGER fsize;
        FILETIME ftime;
        FILETIME fwritetime;
        filedata( std::string n = std::string(), LARGE_INTEGER s = LARGE_INTEGER(), FILETIME t = FILETIME(),  FILETIME tw = FILETIME())
        {
          fname = n;
          fsize = s;
          ftime = t;
          fwritetime = tw;
        }
      };

      std::vector< HANDLE > handles[NUMTYPES];
      std::vector< std::vector< filedata > > dir_contents;

      void get_dir_contents( const std::string& path, std::vector< filedata >& contents )
      {
        WIN32_FIND_DATAA fd;
        HANDLE dir_lister = FindFirstFileA( (path + "\\*").c_str(), &fd );

        if( dir_lister == INVALID_HANDLE_VALUE )
        {
          std::cout << "Couldn't list files." << std::endl;
          return;
        }

        do
        {
          if( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
          {
            LARGE_INTEGER fsize;
            fsize.LowPart = fd.nFileSizeLow;
            fsize.HighPart = fd.nFileSizeHigh;
            FILETIME ftime;
            ftime = fd.ftCreationTime;
            FILETIME fwritetime;
            fwritetime = fd.ftLastWriteTime;
            contents.push_back(filedata( fd.cFileName, fsize, ftime, fwritetime ));
          }
        }
        while(FindNextFileA(dir_lister, &fd) != 0);

        FindClose(dir_lister);
      }
    public:

      ~FileWatcherWin32_AltImpl()
      {
        for( int c = 0; c < NUMTYPES; ++c )
        {
          for( size_t d = 0; d != handles[c].size(); ++d )
          {
            FindCloseChangeNotification(handles[c][d]);
          }
        }
      }

      enum type
      {
        CHANGE_LAST_WRITE = 0, CHANGE_CREATION, CHANGE_SIZE, CHANGE_FILE_NAME, CHANGE_NONE
      };

      struct fw_event
      {
        size_t id;
        type ty;
        fw_event( size_t i = -1, type t = CHANGE_NONE )
        {
          id = i;
          ty = t;
        }
      };

      size_t add( const std::string& path )
      {
        size_t id = handles[0].size();

        for( int c = 0; c < NUMTYPES; ++c )
        {
          handles[c].push_back( FindFirstChangeNotificationA( path.c_str(), false, fw_types[c] ) );
        }

        for( int c = 0; c < NUMTYPES; ++c )
        {
          if( handles[c][id] == INVALID_HANDLE_VALUE )
          {
            std::cerr << "Couldn't create handle." << std::endl;
            return -1;
          }
        }

        dir_contents.resize(id + 1);
        get_dir_contents( path, dir_contents[id] );

        return id;
      }

      bool watch(std::vector<fw_event>& ids)
      {
        for( int c = 0; c < NUMTYPES; ++c )
        {
          DWORD status = WaitForMultipleObjects(handles[c].size(), &handles[c][0], false, 0);

          for( size_t d = 0; d < handles[c].size(); ++d )
          {
            if( status == WAIT_OBJECT_0 + d )
            {
              ids.push_back( fw_event( d, (type)c ) );
              if( FindNextChangeNotification( handles[c][d] ) == false )
              {
                return false;
              }
            }
          }
        }

        return true;
      }

      std::string get_event_filename( const std::string& path, size_t id, type ty, DWORD& fni, std::string& old_name )
      {
        static std::vector< filedata > contents;
        contents.clear();

        get_dir_contents( path, contents );

        switch(ty)
        {
        //change in file write time
        //find the not matching write time
        case CHANGE_LAST_WRITE:
          {
            for( auto c = contents.begin(); c != contents.end(); ++c )
            {
              for( auto d = dir_contents[id].begin(); d != dir_contents[id].end(); ++d )
              {
                if( c->fname == d->fname &&
                    ( c->fwritetime.dwLowDateTime != d->fwritetime.dwLowDateTime ||
                      c->fwritetime.dwHighDateTime != d->fwritetime.dwHighDateTime )
                  )
                {
                  //make sure we 'neutralize' the event
                  d->fwritetime.dwLowDateTime = c->fwritetime.dwLowDateTime;
                  d->fwritetime.dwHighDateTime = c->fwritetime.dwHighDateTime;
                  fni = FILE_ACTION_MODIFIED;
                  return d->fname;
                }
              }
            }
            break;
          }
         //change in file creation time
        //find the not matching creation time
        case CHANGE_CREATION:
          {
            for( auto c = contents.begin(); c != contents.end(); ++c )
            {
              for( auto d = dir_contents[id].begin(); d != dir_contents[id].end(); ++d )
              {
                if( c->fname == d->fname &&
                    ( c->ftime.dwLowDateTime != d->ftime.dwLowDateTime ||
                      c->ftime.dwHighDateTime != d->ftime.dwHighDateTime )
                  )
                {
                  //make sure we 'neutralize' the event
                  d->ftime.dwLowDateTime = c->ftime.dwLowDateTime;
                  d->ftime.dwHighDateTime = c->ftime.dwHighDateTime;
                  fni = FILE_ACTION_ADDED;
                  return d->fname;
                }
              }
            }
            break;
          }
        //change in file name
        //find the new filename, and add it
        //remove the old filename
        case CHANGE_FILE_NAME:
          {
            std::string filename;

            bool file_deleted = contents.size() < dir_contents[id].size();

            for( auto c = contents.begin(); c != contents.end(); ++c )
            {
              bool found = false;
              for( auto d = dir_contents[id].begin(); d != dir_contents[id].end(); ++d )
              {
                if( c->fname == d->fname )
                {
                  found = true;
                  break;
                }
              }

              if( !found )
              {
                //this is what we are looking for, the new filename
                filename = c->fname;
                dir_contents[id].push_back(*c); //add the new file
                fni = FILE_ACTION_ADDED;
                break;
              }
            }

            for( auto c = dir_contents[id].begin(); c != dir_contents[id].end(); ++c )
            {
              bool found = false;

              for( auto d = contents.begin(); d != contents.end(); ++d )
              {
                if( c->fname == d->fname )
                {
                  found = true;
                  break;
                }
              }

              if( !found )
              {
                //this is the old filename

                if( file_deleted ) //file removed
                {
                  filename = c->fname;
                  fni = FILE_ACTION_REMOVED;
                }
                else
                {
                  old_name = c->fname;
                  fni = FILE_ACTION_MODIFIED;
                }

                dir_contents[id].erase(c);
                return filename;
              }
            }

            //in case a new file was created
            if( !filename.empty() )
            {
              fni = FILE_ACTION_ADDED;
              return filename;
            }

            break;
          }
        //change in file size
        //find the not matching file size
        case CHANGE_SIZE:
          {
            for( auto c = contents.begin(); c != contents.end(); ++c )
            {
              for( auto d = dir_contents[id].begin(); d != dir_contents[id].end(); ++d )
              {
                if( c->fname == d->fname &&
                    ( c->fsize.LowPart != d->fsize.LowPart ||
                      c->fsize.HighPart != d->fsize.HighPart )
                  )
                {
                  //make sure we 'neutralize' the event
                  d->fsize.LowPart = c->fsize.LowPart;
                  d->fsize.HighPart = c->fsize.HighPart;
                  fni = FILE_ACTION_MODIFIED;
                  return d->fname;
                }
              }
            }

            //new file
            for( auto c = contents.begin(); c != contents.end(); ++c )
            {
              bool found = false;
              for( auto d = dir_contents[id].begin(); d != dir_contents[id].end(); ++d )
              {
                if( c->fname == d->fname )
                {
                  found = true;
                  break;
                }
              }

              if( !found )
              {
                //this is what we are looking for, the new filename
                fni = FILE_ACTION_ADDED;
                return c->fname;
              }
            }

            break;
          }
        default:
          break;
        }

        return "";
      }

      void remove( size_t id )
      {
        for( int c = 0; c < NUMTYPES; ++c )
        {
          auto it = handles[c].begin();
          auto it2 = dir_contents.begin();
          for( int d = 0; d < id; ++d )
          {
            ++it;
            ++it2;
          };
          FindCloseChangeNotification( handles[c][id] );
          handles[c].erase(it);
          dir_contents.erase(it2);
        }
      }
    };

}

#endif

#endif