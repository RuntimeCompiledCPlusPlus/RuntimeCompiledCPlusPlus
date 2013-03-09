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

//	New code based on using the cross platform file watcher "SimpleFileWatcher" by James Wynn
// https://code.google.com/p/simplefilewatcher/


#include "FileMonitor.h"
#include "assert.h"
#include <algorithm>

using namespace std;


#define CHANGE_QUEUE_SIZE 1000
#define DIRECTORY_CHANGE_BUFFER_SIZE 1024

FileMonitor::FileMonitor()
	: m_changeNotifications(CHANGE_QUEUE_SIZE)
	, m_pFileWatcher( new FW::FileWatcher() ) // Create the file watch object
	, m_bChangeFlag(false)
{
}


FileMonitor::~FileMonitor()
{
	delete m_pFileWatcher;
}

void FileMonitor::Update(	float fDeltaTime )
{
	m_pFileWatcher->update();

	while (!m_changeNotifications.empty())
	{
		ProcessChangeNotification(m_changeNotifications.back());
		m_changeNotifications.pop_back();
	}
}

void FileMonitor::Watch( const char* filename, IFileMonitorListener *pListener /*= NULL*/ )
{
	Watch(FileSystemUtils::Path(filename), pListener);
}

void FileMonitor::Watch( const FileSystemUtils::Path& filename, IFileMonitorListener *pListener /*= NULL*/ )
{
	FileSystemUtils::Path filepath = filename.DelimitersToOSDefault();


	// Is this a directory path or a file path?
	// Actually, we can't tell from a path, in general
	// foo/bar could be a filename with no extension
	// It seems reasonable to inspect the path using FileSystemUtils, check it actually exists
	// (it should, surely? otherwise error?) and determine whether it is a file or a folder
	// but for now - we cheat by assuming files have extensions
	bool bPathIsDir = !filepath.HasExtension();

	FileSystemUtils::Path pathDir = bPathIsDir ? filepath : filepath.ParentPath();
	TDirList::iterator dirIt = GetWatchedDirEntry(pathDir);	
	if (dirIt == m_DirWatchList.end())
	{
		// New directory entry
		m_DirWatchList.push_back( WatchedDir(pathDir) );
		dirIt = --(m_DirWatchList.end());
		StartWatchingDir(*dirIt);
	}
	assert(dirIt != m_DirWatchList.end());

	if (bPathIsDir)
	{
		if (!dirIt->bWatchDirItself)
		{
			assert(!dirIt->pListener);
			dirIt->pListener = pListener;
			dirIt->bWatchDirItself = true;
		}
	}
	else
	{
		// Add file to directory's watch list (if it's not already there)
		TFileList::iterator fileIt = GetWatchedFileEntry(filepath, dirIt->fileWatchList);
		if (fileIt == dirIt->fileWatchList.end())
		{
			dirIt->fileWatchList.push_back(WatchedFile(filepath, pListener));
		}
	}
}


FileMonitor::TDirList::iterator FileMonitor::GetWatchedDirEntry( const FileSystemUtils::Path& dir )
{
	TDirList::iterator dirIt = m_DirWatchList.begin();
	TDirList::iterator dirItEnd = m_DirWatchList.end();
	while (dirIt != dirItEnd && !ArePathsEqual(dirIt->dir, dir))
	{
		dirIt++;
	}

	return dirIt;
}


FileMonitor::TFileList::iterator FileMonitor::GetWatchedFileEntry( const FileSystemUtils::Path& file, TFileList& fileList )
{
	TFileList::iterator fileIt = fileList.begin();
	TFileList::iterator fileItEnd = fileList.end();
	while (fileIt != fileItEnd && !ArePathsEqual(fileIt->file, file))
	{
		fileIt++;
	}

	return fileIt;
}


void FileMonitor::ClearChanges()
{
	m_bChangeFlag = false;
	m_FileChangedList.clear();
}


void FileMonitor::StartWatchingDir( WatchedDir& dirEntry )
{
	m_pFileWatcher->addWatch( dirEntry.dir, this );
}


void FileMonitor::ProcessChangeNotification( const FileSystemUtils::Path& file )
{
	// Notify any listeners and add to change list if this is a watched file/dir
	
	// Again - this isn't correct, just a hack
	bool bPathIsDir = !file.HasExtension();
	FileSystemUtils::Path pathDir = bPathIsDir ? file : file.ParentPath();

	TDirList::iterator dirIt = GetWatchedDirEntry(pathDir);	
	if (dirIt != m_DirWatchList.end())
	{
		// Is directory itself being watched?
		// Unsure here - no need to notify as folder will get it's own notification come through?
		// Bit below feels redundant
		if (dirIt->bWatchDirItself)
		{
			if (dirIt->pListener != NULL)
			{
				dirIt->pListener->OnFileChange(pathDir);
			}
			else
			{
				m_FileChangedList.push_back(pathDir);
				m_bChangeFlag = true;
			}
		}
		
		// Is this one of the files being watched in the directory?
		TFileList::iterator fileIt = GetWatchedFileEntry(file, dirIt->fileWatchList);
		if (fileIt != dirIt->fileWatchList.end())
		{
			if (fileIt->pListener != NULL)
			{
				fileIt->pListener->OnFileChange(file);
			}
			else
			{
				m_FileChangedList.push_back(file);
				m_bChangeFlag = true;
			}
		}
	}
}




void FileMonitor::handleFileAction(FW::WatchID watchid, const FW::String& dir, const FW::String& filename,
                   FW::Action action)
{
	switch(action)
	{
	case FW::Actions::Add:
		// Currently do nothing
		break;
	case FW::Actions::Delete:
		// Currently do nothing
		break;
	case FW::Actions::Modified:
		{
  			FileSystemUtils::Path filePath(filename);
 			if( !filename.HasParentPath() )
  			{
  				filePath = dir / filePath;
  			}

			m_changeNotifications.push_back(filePath.DelimitersToOSDefault());
		}
		break;
	default:
		assert( false ); //should not happen
	}

}
