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

#ifndef FILECHANGENOTIFIER_INCLUDED
#define FILECHANGENOTIFIER_INCLUDED

#include "IFileChangeNotifier.h"
#include "SimpleFileWatcher/FileWatcher.h"
#include <vector>
#include <map>
#include <set>

#include "FileSystemUtils.h"


// Manages the registering of files with the file monitor and triggering
// Of compilation when a registered file changes
class FileChangeNotifier : public IFileChangeNotifier, public FW::FileWatchListener
{
public:
	FileChangeNotifier();
	virtual ~FileChangeNotifier();

	// IFileChangeNotifier
		
	virtual bool IsMonitoringActive() const
	{
		return m_bActive;
	}

	virtual void SetMonitoringActive( bool bActive )
	{
		m_bActive = bActive;
	}

	virtual float GetMinTimeBetweenNotifications() const
	{
		return m_fMinTimeBetweenNotifications;
	}

	virtual void SetMinTimeBetweenNotifications( float fMinTime );

	virtual float GetChangeNotifyDelay() const
	{
		return m_fChangeNotifyDelay;
	}

	virtual void SetChangeNotifyDelay( float fDelay );

	virtual void Update( float fDeltaTime );

	// Add file to trigger compilation when it changes
	virtual void Watch( const FileSystemUtils::Path& filename, IFileChangeListener *pListener );
	virtual void Watch( const char *filename, IFileChangeListener *pListener );

	virtual void RemoveListener( IFileChangeListener *pListener );

	// ~IFileChangeNotifier


    // FW::FileWatchListener

    void handleFileAction(FW::WatchID watchid, const FW::String& dir, const FW::String& filename,
        FW::Action action);

    // ~FW::FileWatchListener


private:
	
	void TriggerNotificationIfPossible();
	void NotifyListeners();

	typedef std::set<IFileChangeListener*> TFileChangeListeners;
	typedef std::map<FileSystemUtils::Path, TFileChangeListeners> TFileListenerMap;
	typedef std::set<std::string> TPathNameList;

	// Private members
    FW::FileWatcher* m_pFileWatcher;
    TPathNameList    m_WatchedDirs;

	TFileListenerMap m_fileListenerMap;
	TPathNameList    m_changedFileList;
	
	bool m_bActive;
	bool m_bRecompilePending;

	float m_fMinTimeBetweenNotifications;
	float m_fChangeNotifyDelay;
	float m_fTimeUntilNextAllowedRecompile;
	float m_fFileChangeSpamTimeRemaining;
	FileSystemUtils::Path m_LastFileChanged;	
};

#endif //FILECHANGENOTIFIER_INCLUDED