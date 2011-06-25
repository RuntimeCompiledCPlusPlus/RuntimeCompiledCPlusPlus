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

//	This code is based on blog entry titled, "Understanding ReadDirectoryChangesW"
//	http://qualapps.blogspot.com/2010/05/understanding-readdirectorychangesw.html


#pragma once

#ifndef FILEMONITOR_DEF
#define FILEMONITOR_DEF

#include "IFileMonitor.h"
#include "ThreadSafeQueue.h"

#include "windows.h"
#include <vector>

#define BOOST_FILESYSTEM_VERSION 3
#include "boost/filesystem.hpp"   // includes all needed Boost.Filesystem declarations


class DirChangeMonitor;


class FileMonitor : public IFileMonitor
{
public:
	typedef ThreadSafeQueue<boost::filesystem::path> TNotifications;

	FileMonitor();
	virtual ~FileMonitor();


	// IFileMonitor

	virtual void Update( float fDeltaTime );

	// Watch file or directory for changes
	// Optional callbackFunc will be notified on change occurring
	// If callback is specified, file will not be added to change list when it changes
	virtual void Watch( const boost::filesystem::path& filename, IFileMonitorListener *pListener /*=0*/ );
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

	const std::vector<boost::filesystem::path>& GetChanges() const
	{
		return m_FileChangedList;
	}


private:
	
	struct WatchedFile
	{
		boost::filesystem::path file;
		IFileMonitorListener *pListener;

		WatchedFile( const boost::filesystem::path& file_, IFileMonitorListener *pListener_ )
			: file(file_), pListener(pListener_)
		{}
	};
	typedef std::vector<WatchedFile> TFileList;

	struct WatchedDir
	{
		boost::filesystem::path dir;
		IFileMonitorListener *pListener; // used when the directory itself is explicitly being watched
		TFileList fileWatchList;
		bool bWatchDirItself;

		WatchedDir( const boost::filesystem::path& dir_ )
			: dir(dir_)
			, bWatchDirItself(false)
		{}
	};
	typedef std::vector<WatchedDir> TDirList;
	

	void InitMonitorThread();
	void TerminateMonitorThread();
	void StartWatchingDir( WatchedDir& dir );
	TDirList::iterator GetWatchedDirEntry( const boost::filesystem::path& dir );
	TFileList::iterator GetWatchedFileEntry( const boost::filesystem::path& file, TFileList& fileList );
	void ProcessChangeNotification( const boost::filesystem::path& file );
	bool ArePathsEqual( const boost::filesystem::path& file1, const boost::filesystem::path& file2 ) const;

	DirChangeMonitor *m_pDirChangeMonitor;
	TDirList m_DirWatchList;
	std::vector<boost::filesystem::path> m_FileChangedList;
	TNotifications m_changeNotifications;

	bool m_bChangeFlag;
	HANDLE m_hThread;
	unsigned int m_threadId;
};


inline bool FileMonitor::ArePathsEqual( const boost::filesystem::path& file1, const boost::filesystem::path& file2 ) const
{
#ifdef _WINDOWS_
	// Do case insensitive comparison on Windows
	// Not as inefficient as it looks, since boost paths are natively wstrings on Windows
	// so no conversions or allocations are actually happening here
	return _wcsicmp(file1.wstring().c_str(), file2.wstring().c_str()) == 0;
#else
	return file1 == file2;
#endif
}

#endif //FILEMONITOR_DEF