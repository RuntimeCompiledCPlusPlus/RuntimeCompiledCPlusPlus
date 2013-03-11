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

#pragma once

#ifndef FILEMONITOR_DEF
#define FILEMONITOR_DEF

#include "IFileMonitor.h"
#include <vector>

#include "FileSystemUtils.h"

#include "SimpleFileWatcher/FileWatcher.h"




class FileMonitor : public IFileMonitor, public FW::FileWatchListener
{
public:
	//typedef ThreadSafeQueue<FileSystemUtils::Path> TNotifications;
	typedef std::vector<FileSystemUtils::Path> TNotifications;

	FileMonitor();
	virtual ~FileMonitor();

	// FW::FileWatchListener

	void handleFileAction(FW::WatchID watchid, const FW::String& dir, const FW::String& filename,
                   FW::Action action);

	// ~FW::FileWatchListener

	// IFileMonitor

	virtual void Update( float fDeltaTime );

	// Watch file or directory for changes
	// Optional callbackFunc will be notified on change occurring
	// If callback is specified, file will not be added to change list when it changes
	virtual void Watch( const FileSystemUtils::Path& filename, IFileMonitorListener *pListener /*=0*/ );
	virtual void Watch( const char* filename, IFileMonitorListener *pListener /*=0*/ );

	// ~IFileMonitor


	// Returns if there are any changes detected - does not poll files
	// but simply returns change flag status which is updated asynchronously
	bool GetHasChanges() const
	{
		return m_bChangeFlag;
	}

	// Clears the list of changes, resets changed flag
	void ClearChanges();

	const std::vector<FileSystemUtils::Path>& GetChanges() const
	{
		return m_FileChangedList;
	}


private:
	
	struct WatchedFile
	{
		FileSystemUtils::Path file;
		IFileMonitorListener *pListener;

		WatchedFile( const FileSystemUtils::Path& file_, IFileMonitorListener *pListener_ )
			: file(file_), pListener(pListener_)
		{}
	};
	typedef std::vector<WatchedFile> TFileList;

	struct WatchedDir
	{
		FileSystemUtils::Path dir;
		IFileMonitorListener *pListener; // used when the directory itself is explicitly being watched
		TFileList fileWatchList;
		bool bWatchDirItself;

		WatchedDir( const FileSystemUtils::Path& dir_ )
			: dir(dir_)
			, pListener(0)
			, bWatchDirItself(false)
		{}
	};
	typedef std::vector<WatchedDir> TDirList;
	

	void StartWatchingDir( WatchedDir& dir );
	TDirList::iterator GetWatchedDirEntry( const FileSystemUtils::Path& dir );
	TFileList::iterator GetWatchedFileEntry( const FileSystemUtils::Path& file, TFileList& fileList );
	void ProcessChangeNotification( const FileSystemUtils::Path& file );
	bool ArePathsEqual( const FileSystemUtils::Path& file1, const FileSystemUtils::Path& file2 ) const;

	TDirList 							m_DirWatchList;
	std::vector<FileSystemUtils::Path>	m_FileChangedList;
	TNotifications						m_changeNotifications;
	FW::FileWatcher* 					m_pFileWatcher;
	bool 								m_bChangeFlag;
};


inline bool FileMonitor::ArePathsEqual( const FileSystemUtils::Path& file1, const FileSystemUtils::Path& file2 ) const
{
#ifdef _WIN32
	// Do case insensitive comparison on Windows
	return _stricmp(file1.c_str(), file2.c_str()) == 0;
#else
	return file1 == file2;
#endif
}

#endif //FILEMONITOR_DEF
