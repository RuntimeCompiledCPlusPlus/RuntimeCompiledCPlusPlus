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

#include "FileChangeNotifier.h"
#include "boost/algorithm/string.hpp"

#include "FileMonitor.h"
#include <algorithm>
using namespace std;

#define DEFAULT_MIN_TIME_BETWEEN_RECOMPILES 1.0f
#define DEFAULT_NOTIFY_DELAY 0.1f
#define FILE_CHANGE_SPAM_TIME 1.0f


FileChangeNotifier::FileChangeNotifier()
	: m_bActive(true)
	, m_bRecompilePending(false)
	, m_fMinTimeBetweenNotifications(DEFAULT_MIN_TIME_BETWEEN_RECOMPILES)
	, m_fChangeNotifyDelay(DEFAULT_NOTIFY_DELAY)
	, m_fTimeUntilNextAllowedRecompile(0.0f)
	, m_fFileChangeSpamTimeRemaining(0.0f)
{
	m_pFileMonitor = new FileMonitor();
	m_LastFileChanged = "";
}


FileChangeNotifier::~FileChangeNotifier()
{
	delete m_pFileMonitor;
}


void FileChangeNotifier::SetMinTimeBetweenNotifications( float fMinTime )
{
	m_fMinTimeBetweenNotifications = max(0.0f, fMinTime);
}

void FileChangeNotifier::SetChangeNotifyDelay( float fDelay)
{
	m_fChangeNotifyDelay = max(0.0f, fDelay);
}

void FileChangeNotifier::Update( float fDeltaTime )
{
	if (m_bActive)
	{
		m_pFileMonitor->Update(fDeltaTime);
		m_fTimeUntilNextAllowedRecompile = max(0.0f, m_fTimeUntilNextAllowedRecompile - fDeltaTime);
		m_fFileChangeSpamTimeRemaining = max(0.0f, m_fFileChangeSpamTimeRemaining - fDeltaTime);

		if (m_bRecompilePending)
		{
			TriggerNotificationIfPossible();
		}
	}
}


void FileChangeNotifier::Watch( const boost::filesystem::path& filename, IFileChangeListener *pListener )
{
	boost::filesystem::path fixedFilename = filename;
	fixedFilename.normalize().make_preferred(); // this is technically not safe on filesystems with symlinks, but practically is fine for our purposes
	
#ifdef _WINDOWS_
	// make filename lowercase to avoid case sensitivity issues with __FILE__ and ReadDirectoryChangesW output
	fixedFilename = boost::filesystem::path(boost::to_lower_copy(fixedFilename.wstring()));
#endif

	m_pFileMonitor->Watch(fixedFilename, this);
	m_fileListenerMap[fixedFilename].insert(pListener);
	pListener->OnRegisteredWithNotifier(this);
}


void FileChangeNotifier::Watch( const char *filename, IFileChangeListener *pListener )
{
	Watch(boost::filesystem::path(filename), pListener);
}

void FileChangeNotifier::RemoveListener( IFileChangeListener *pListener )
{
	TFileListenerMap::iterator it = m_fileListenerMap.begin();
	TFileListenerMap::iterator itEnd = m_fileListenerMap.end();
	while (it != itEnd)
	{
		it->second.erase(pListener);
		++it;
	}

	pListener->OnRegisteredWithNotifier(NULL);
}

void FileChangeNotifier::OnFileChange( const boost::filesystem::path& filename )
{
	if (m_bActive)
	{
		// Check for multiple hits on the same file in close succession 
		// (Can be caused by NTFS system making multiple changes even though only
		//  one actual change occurred)
		bool bIgnoreFileChange = (filename == m_LastFileChanged) && 
			m_fFileChangeSpamTimeRemaining > 0.0f;
		m_LastFileChanged = filename;

		if (!bIgnoreFileChange)
		{
			const boost::filesystem::path* pFilename = &filename;

#ifdef _WIN32
			// make filename lowercase to avoid case sensitivity issues with __FILE__ and ReadDirectoryChangesW output
			boost::filesystem::path lowerFilename = boost::filesystem::path(boost::to_lower_copy(filename.wstring()));
			pFilename = &lowerFilename;
#endif

			m_changedFileList.push_back(pFilename->string());

			if (!m_bRecompilePending)
			{
				m_bRecompilePending = true;
				m_fTimeUntilNextAllowedRecompile = max(m_fTimeUntilNextAllowedRecompile, m_fChangeNotifyDelay);
			}

			m_fFileChangeSpamTimeRemaining = FILE_CHANGE_SPAM_TIME;
			TriggerNotificationIfPossible();	
		}	
	}
}

void FileChangeNotifier::TriggerNotificationIfPossible()
{
	if (m_fTimeUntilNextAllowedRecompile <= 0.0f)
	{
		m_fTimeUntilNextAllowedRecompile = m_fMinTimeBetweenNotifications;
		m_bRecompilePending = false;

		NotifyListeners();
					
		m_changedFileList.clear();
	}
	else
	{
		m_bRecompilePending = true;
	}	
}

void FileChangeNotifier::NotifyListeners()
{
	std::map<IFileChangeListener*, AUDynArray<const char*> > interestedListenersMap;

	// Determine which listeners are interested in which changed files
	TPathNameList::const_iterator fileIt = m_changedFileList.begin();
	TPathNameList::const_iterator fileItEnd = m_changedFileList.end();
	while (fileIt != fileItEnd)
	{
		TFileChangeListeners& listeners = m_fileListenerMap[*fileIt];
		TFileChangeListeners::iterator listenerIt = listeners.begin();
		TFileChangeListeners::iterator listenerItEnd = listeners.end();
		while (listenerIt != listenerItEnd)
		{
			interestedListenersMap[*listenerIt].Add(fileIt->c_str());
			++listenerIt;
		}
		
		++fileIt;
	}

	// Notify each listener with an appropriate file list
	std::map<IFileChangeListener*, AUDynArray<const char*> >::iterator finalIt = interestedListenersMap.begin();
	std::map<IFileChangeListener*, AUDynArray<const char*> >::iterator finalItEnd = interestedListenersMap.end();
	while (finalIt != finalItEnd)
	{
		finalIt->first->OnFileChange(finalIt->second);	
		++finalIt;
	}
}

