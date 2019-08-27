/**
	Copyright (c) 2009 James Wynn (james@jameswynn.com)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.

	James Wynn james@jameswynn.com
*/

/**
    Major changes Doug Binks
 **/

#include "FileWatcherOSX.h"

#if FILEWATCHER_PLATFORM == FILEWATCHER_PLATFORM_KQUEUE

#include <sys/event.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>

namespace FW
{
	
#define MAX_FILELIST_SIZE 2048
	
	typedef struct kevent KEvent;
	
	struct FileInfo
	{
		FileInfo()
		: mFilename(0), mModifiedTime(0)
		{
		}
		const char* mFilename;
		time_t mModifiedTime;
	};
    
    int comparator(const void* f1, const void* f2)
    {
        FileInfo* finfo1 = (FileInfo*)f1;
        FileInfo* finfo2 = (FileInfo*)f2;
        
        int retVal = strcmp(finfo1->mFilename, finfo2->mFilename);
        
        return retVal;
    }
    
    
	struct WatchStruct
	{
		WatchID             mWatchID;
		String              mDirName;
		FileWatchListener*  mListener;
        KEvent              mDirKevent;
		
		FileInfo            mFileList[MAX_FILELIST_SIZE];
		size_t              mFileListCount;
		
		WatchStruct(WatchID watchid, const String& dirname, FileWatchListener* listener)
		: mWatchID(watchid), mDirName(dirname), mListener(listener), mFileListCount( 0 )
		{
			addAll(true);
		}
        
        ~WatchStruct()
        {
            removeAll();
        }
		
		void addFile(const std::string& name)
		{
			// create entry in file list
			struct stat attrib;
			stat(name.c_str(), &attrib);

			char* namecopy = new char[name.length() + 1];
			strncpy(namecopy, name.c_str(), name.length());
			namecopy[name.length()] = 0;
            mFileList[mFileListCount].mFilename = namecopy;
            mFileList[mFileListCount].mModifiedTime = attrib.st_mtime;
			++mFileListCount;
			
		}
		void removeFile(const std::string& name)
		{
			// bsearch
			FileInfo key;
			key.mFilename =  name.c_str();
			FileInfo* found = (FileInfo*)bsearch(&key, &mFileList, mFileListCount, sizeof(FileInfo), comparator);
			if(!found)
            {
				return;
            }
            
			key.mFilename = 0; // prevent deletion of key string
			delete found->mFilename;
            found->mFilename = 0;
            
            assert( mFileListCount > 1 );
			// move end to current
			memcpy(found, &mFileList[mFileListCount-1], sizeof(FileInfo));
			memset(&mFileList[mFileListCount-1], 0, sizeof(FileInfo));
			--mFileListCount;
        }
		
		// called when the directory is actually changed
		// means a file has been added or removed
		// rescans the watched directory adding/removing files and sending notices
		void rescan()
		{
			// if new file, call addFile
			// if missing file, call removeFile
			// if timestamp modified, call handleAction(filename, ACTION_MODIFIED);
			DIR* dir = opendir(mDirName.c_str());
			if(!dir)
				return;
			
			struct dirent* dentry;
			size_t fileIndex = 0;
			struct stat attrib;			
			bool bRescanRequired = false; //if files are added or deleted we need a rescan.
			while((dentry = readdir(dir)) != NULL)
			{
                std::string fname = mDirName.m_string + "/" + dentry->d_name;
				stat(fname.c_str(), &attrib);
				if(!S_ISREG(attrib.st_mode))
					continue;
				
				if( fileIndex < mFileListCount )
				{
					FileInfo& entry = mFileList[fileIndex];
					int result = strcmp(entry.mFilename, fname.c_str());
					if(result == 0)
					{
						stat(entry.mFilename, &attrib);
						time_t timestamp = attrib.st_mtime;
						
						if(entry.mModifiedTime != timestamp)
						{
							entry.mModifiedTime = timestamp;
							handleAction(entry.mFilename, Actions::Modified);
						}
						++fileIndex;
					}
                    else
                    {
                        // file might have been added or deleted
                        // if we find the file in our list, then we have some deletions up to that point
                        // otherwise we have an add
                        bRescanRequired = true;
                        size_t currFile = fileIndex+1;
                        while( currFile < mFileListCount )
                        {
                            FileInfo& entry2 = mFileList[currFile];
                            int res = strcmp(entry2.mFilename, fname.c_str());
                            if(res == 0)
                            {
                                //have found the file in our list
                                break;
                            }
                            ++currFile;
                        }
                        
                        //process events but don't add/remove to list.
                        if( currFile < mFileListCount )
                        {
                           //have some deletions.
                           while( fileIndex < currFile )
                           {
                               FileInfo& entry2 = mFileList[currFile];
                               handleAction(entry2.mFilename, Actions::Delete);
                               ++fileIndex;
                           }
                            ++fileIndex;
                        }
                        else
                        {
                            //we don't increment fileIndex here as it's an add in the middle.
                            handleAction(fname.c_str(), Actions::Add);
                        }
                    }
 				}
				else
				{
					// just add
 					addFile(fname);
                    handleAction(fname.c_str(), Actions::Add);
					++fileIndex;
				}
			}//end while
			
			closedir(dir);
            
            while( fileIndex < mFileListCount )
            {
                // the last files have been deleted...
                bRescanRequired = true;
                FileInfo& entry = mFileList[fileIndex];
                handleAction(entry.mFilename, Actions::Delete);
                ++fileIndex;
            }
            
            if( bRescanRequired )
            {
                removeAll();
                addAll(false);
            }
		};
		
		void handleAction(const String& filename, FW::Action action)
		{
			mListener->handleFileAction(mWatchID, mDirName, filename, action);
		}
		
		void addAll( bool bCreatedirevent )
		{
            if( bCreatedirevent )
            {
                // add base dir
                int fd = open(mDirName.c_str(), O_RDONLY);
                EV_SET(&mDirKevent, fd, EVFILT_VNODE,
                       EV_ADD | EV_ENABLE | EV_CLEAR,
                       NOTE_DELETE | NOTE_EXTEND | NOTE_WRITE | NOTE_ATTRIB | NOTE_RENAME | NOTE_REVOKE,
                       0, 0);
            }
			
			//fprintf(stderr, "ADDED: %s\n", mDirName.c_str());			
			
			// scan directory and call addFile(name, false) on each file
			DIR* dir = opendir(mDirName.c_str());
			if(!dir)
				return;
			
			struct dirent* entry;
			struct stat attrib;
			while((entry = readdir(dir)) != NULL)
			{
                std::string fname = (mDirName.m_string + "/" + std::string(entry->d_name));
				stat(fname.c_str(), &attrib);
				if(S_ISREG(attrib.st_mode))
					addFile(fname);
				//else
				//	fprintf(stderr, "NOT ADDED: %s (%d)\n", fname.c_str(), attrib.st_mode);

			}//end while
			
			closedir(dir);
		}
		
		void removeAll()
		{
			// go through list removing each file but not the directory
			for(size_t i = 0; i < mFileListCount; ++i)
			{
				FileInfo& entry = mFileList[i];
				// delete
				delete[] entry.mFilename;
                entry.mModifiedTime = 0;
			}
            mFileListCount = 0;
		}
	};
	
	void FileWatcherOSX::update()
	{
		int nev = 0;
		struct kevent event;
        
        // DJB updated code to handle multiple directories correctly
        // first look for events which have occurred in our queue
		while((nev = kevent(mDescriptor, 0, 0, &event, 1, &mTimeOut)) != 0)
        {
            if(nev == -1)
                perror("kevent");
            else
            {
                // have an event, need to find the watch which has this event
                WatchMap::iterator iter = mWatches.begin();
                WatchMap::iterator end = mWatches.end();
                for(; iter != end; ++iter)
                {
                    WatchStruct* watch = iter->second;
                    if( event.ident ==  watch->mDirKevent.ident )
                    {
                        watch->rescan();
                        break;
                    }
                }
           }
        }
    }
	
	//--------
	FileWatcherOSX::FileWatcherOSX()
	{
		mDescriptor = kqueue();
		mTimeOut.tv_sec = 0;
		mTimeOut.tv_nsec = 0;
	}

	//--------
	FileWatcherOSX::~FileWatcherOSX()
	{
		WatchMap::iterator iter = mWatches.begin();
		WatchMap::iterator end = mWatches.end();
		for(; iter != end; ++iter)
		{
			delete iter->second;
		}
		mWatches.clear();
		
		close(mDescriptor);
	}

	//--------
	WatchID FileWatcherOSX::addWatch(const String& directory, FileWatchListener* watcher, bool recursive)
	{
		(void) recursive;
/*		int fd = open(directory.c_str(), O_RDONLY);
		if(fd == -1)
			perror("open");
				
		EV_SET(&change, fd, EVFILT_VNODE,
			   EV_ADD | EV_ENABLE | EV_ONESHOT,
			   NOTE_DELETE | NOTE_EXTEND | NOTE_WRITE | NOTE_ATTRIB,
			   0, (void*)"testing");
*/
		
		WatchStruct* watch = new WatchStruct(++mLastWatchID, directory, watcher);
		mWatches.insert(std::make_pair(mLastWatchID, watch));
        
        // DJB we add the event to our kqueue (but don't request any return events, these are looked for in update loop
        kevent(mDescriptor, (KEvent*)&(watch->mDirKevent), 1, 0, 0, 0);
        
		return mLastWatchID;
	}

	//--------
	void FileWatcherOSX::removeWatch(const String& directory)
	{
		WatchMap::iterator iter = mWatches.begin();
		WatchMap::iterator end = mWatches.end();
		for(; iter != end; ++iter)
		{
			if(directory == iter->second->mDirName)
			{
				removeWatch(iter->first);
				return;
			}
		}
	}

	//--------
	void FileWatcherOSX::removeWatch(WatchID watchid)
	{
		WatchMap::iterator iter = mWatches.find(watchid);

		if(iter == mWatches.end())
			return;

		WatchStruct* watch = iter->second;
		mWatches.erase(iter);
	
		//inotify_rm_watch(mFD, watchid);
		
		delete watch; // Note: this also removes the event for the watch from the queue
		watch = 0;
	}
	
	//--------
	void FileWatcherOSX::handleAction(WatchStruct* watch, const String& filename, unsigned long action)
	{
				(void)watch;
				(void)filename;
				(void)action;
        assert(false);//should not get here for OSX impl
	}

};//namespace FW

#endif//FILEWATCHER_PLATFORM_KQUEUE
