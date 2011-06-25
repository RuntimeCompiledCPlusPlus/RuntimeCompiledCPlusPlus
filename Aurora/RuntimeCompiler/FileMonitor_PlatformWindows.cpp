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


#include "FileMonitor_PlatformWindows.h"
#include "assert.h"
#include <algorithm>
#include <process.h>

using namespace std;
using namespace boost::filesystem;


#define CHANGE_QUEUE_SIZE 1000
#define DIRECTORY_CHANGE_BUFFER_SIZE 1024


class DirWatchRequest;
class DirChangeMonitor;

typedef pair<DirChangeMonitor*, boost::filesystem::path> TAddDirRequestArgs;


// Used by DirChangeMonitor thread to store individual watch requests
// All methods run in the context of the worker thread
class DirWatchRequest
{
	typedef std::vector<BYTE> TBuffer;

public:
	DirWatchRequest( FileMonitor::TNotifications *pNotifications, const boost::filesystem::path& dirName )
		: m_pNotifications(pNotifications)
		, m_hDir(NULL)
		, m_DirName(dirName)
	{
		assert(m_pNotifications != NULL);

		ZeroMemory(&m_Overlapped, sizeof(OVERLAPPED));
		m_Overlapped.hEvent = this;
		m_NotificationBuffer.resize(DIRECTORY_CHANGE_BUFFER_SIZE);
		m_ProcessBuffer.resize(DIRECTORY_CHANGE_BUFFER_SIZE);

		if (OpenDir())
		{
			StartRead();
		}
	}


	~DirWatchRequest()
	{
		::CancelIo(m_hDir);
		::CloseHandle(m_hDir);
		m_hDir = INVALID_HANDLE_VALUE;
	}


private:

	static VOID CALLBACK NotificationCompletion(
		DWORD dwErrorCode,							// completion code
		DWORD dwNumberOfBytesTransfered,			// number of bytes transferred
		LPOVERLAPPED lpOverlapped)
	{
		DirWatchRequest *request = (DirWatchRequest *)lpOverlapped->hEvent;

		if (dwErrorCode == ERROR_OPERATION_ABORTED || !dwNumberOfBytesTransfered)
		{
			return;
		}

		// Copy notification buffer to process buffer so we can restart read operation immediately
		memcpy(&(request->m_ProcessBuffer)[0], &(request->m_NotificationBuffer)[0], dwNumberOfBytesTransfered);
				
		request->StartRead();

		request->ProcessChangeNotification();
	}


	bool OpenDir()
	{
#ifdef UNICODE
		wstring dirName = m_DirName.wstring();
#else
		string dirName = m_DirName.string();
#endif

		m_hDir = CreateFile(
			dirName.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);

		assert (m_hDir != INVALID_HANDLE_VALUE);
		return (m_hDir != INVALID_HANDLE_VALUE);
	}


	void StartRead()
	{
		DWORD dwBytes = 0;
		
		BOOL bSuccess = ReadDirectoryChangesW(
			m_hDir,
			&m_NotificationBuffer[0],
			m_NotificationBuffer.size(),
			false,  // Watch subtree
			FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_CREATION|FILE_NOTIFY_CHANGE_FILE_NAME,
			&dwBytes,
			&m_Overlapped,
			NotificationCompletion);

		assert(bSuccess);
	}

	void ProcessChangeNotification() const
	{
		char* pBase = (char*)&m_ProcessBuffer[0];

		for (;;)
		{
			FILE_NOTIFY_INFORMATION& fni = (FILE_NOTIFY_INFORMATION&)*pBase;

			wstring stringFilename = wstring(fni.FileName, fni.FileNameLength/sizeof(wchar_t));
			boost::filesystem::path filePath(m_DirName);
			filePath /= stringFilename;

			m_pNotifications->push(filePath.make_preferred());

			if (!fni.NextEntryOffset)
				break;
			pBase += fni.NextEntryOffset;
		};
	}


	FileMonitor::TNotifications* m_pNotifications;
	HANDLE m_hDir;
	OVERLAPPED m_Overlapped;
	boost::filesystem::path m_DirName;
	TBuffer m_NotificationBuffer; // used to store directory read results
	TBuffer m_ProcessBuffer;      // copy notification results in here to process so we can begin a new read operation immediately
};



// Run on a separate worker thread to initiate read requests and wait for notifications
// All methods run in the context of the worker thread
class DirChangeMonitor
{
public:
	DirChangeMonitor(FileMonitor::TNotifications* pNotifications) 
		: m_bTerminate(false)
		, m_pNotifications(pNotifications)
	{
		assert(m_pNotifications != NULL);
	}


	static unsigned int WINAPI RunProc( LPVOID arg )
	{
		DirChangeMonitor* pMonitor = (DirChangeMonitor*)arg;
		pMonitor->Run();
		return 0;
	}

	// Called by QueueUserAPC to start orderly shutdown.
	static void CALLBACK TerminateProc( __in  ULONG_PTR arg )
	{
		DirChangeMonitor* pMonitor = (DirChangeMonitor*)arg;
		pMonitor->Terminate();
	}

	// Called by QueueUserAPC to add another directory.
	static void CALLBACK AddDirectoryProc( __in  ULONG_PTR arg )
	{
		TAddDirRequestArgs* pRequestArgs = (TAddDirRequestArgs*)arg;
		pRequestArgs->first->AddDirectory(pRequestArgs->second);
		delete pRequestArgs;
	}


private:

	void Run()
	{
		while (!m_bTerminate)
		{
			DWORD rc = ::SleepEx(100 /*INFINITE*/, true);
		}
	}

	void AddDirectory( const boost::filesystem::path& dir )
	{
		m_pRequests.push_back(new DirWatchRequest(m_pNotifications, dir));		
	}

	void Terminate()
	{
		m_bTerminate = true;

		for (unsigned int i=0; i<m_pRequests.size(); ++i)
		{
			delete m_pRequests[i];
		}

		m_pRequests.clear();
	}

	vector<DirWatchRequest*> m_pRequests;
	FileMonitor::TNotifications* m_pNotifications;
	bool m_bTerminate;
};



FileMonitor::FileMonitor()
	: m_changeNotifications(CHANGE_QUEUE_SIZE)
	, m_bChangeFlag(false)
	, m_hThread(NULL)
	, m_threadId(0)
{
	m_pDirChangeMonitor = new DirChangeMonitor(&m_changeNotifications);

	InitMonitorThread();
}


FileMonitor::~FileMonitor()
{
	TerminateMonitorThread();

	delete m_pDirChangeMonitor;
}


void FileMonitor::InitMonitorThread()
{
	m_hThread = (HANDLE)_beginthreadex(
		NULL,
		0,
		DirChangeMonitor::RunProc,
		m_pDirChangeMonitor,
		0,
		&m_threadId);
}


void FileMonitor::TerminateMonitorThread()
{
	if (m_hThread)
	{
		::QueueUserAPC(DirChangeMonitor::TerminateProc, m_hThread, (ULONG_PTR)m_pDirChangeMonitor);
		::WaitForSingleObjectEx(m_hThread, 10000, true);
		::CloseHandle(m_hThread);

		m_hThread = NULL;
		m_threadId = 0;
	}
}


void FileMonitor::Update(	float fDeltaTime )
{
	while (!m_changeNotifications.empty())
	{
		boost::filesystem::path file;
		if (m_changeNotifications.pop(file))
		{
			ProcessChangeNotification(file);
		}
	}
}

void FileMonitor::Watch( const char* filename, IFileMonitorListener *pListener /*= NULL*/ )
{
	Watch(boost::filesystem::path(filename), pListener);
}

void FileMonitor::Watch( const boost::filesystem::path& filename, IFileMonitorListener *pListener /*= NULL*/ )
{
	boost::filesystem::path filepath = boost::filesystem::path(filename).make_preferred();

	// Is this a directory path or a file path?
	// Actually, we can't tell from a path, in general
	// foo/bar could be a filename with no extension
	// It seems reasonable to inspect the path using boost, check it actually exists
	// (it should, surely? otherwise error?) and determine whether it is a file or a folder
	// but for now - we cheat by assuming files have extensions
	bool bPathIsDir = !filepath.has_extension();

	boost::filesystem::path pathDir = bPathIsDir ? filepath : filepath.parent_path();
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


FileMonitor::TDirList::iterator FileMonitor::GetWatchedDirEntry( const boost::filesystem::path& dir )
{
	TDirList::iterator dirIt = m_DirWatchList.begin();
	TDirList::iterator dirItEnd = m_DirWatchList.end();
	while (dirIt != dirItEnd && !ArePathsEqual(dirIt->dir, dir))
	{
		dirIt++;
	}

	return dirIt;
}


FileMonitor::TFileList::iterator FileMonitor::GetWatchedFileEntry( const boost::filesystem::path& file, TFileList& fileList )
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
	TAddDirRequestArgs* pArgs = new TAddDirRequestArgs(m_pDirChangeMonitor, dirEntry.dir);

	QueueUserAPC(DirChangeMonitor::AddDirectoryProc, m_hThread, (ULONG_PTR)pArgs);
}


void FileMonitor::ProcessChangeNotification( const boost::filesystem::path& file )
{
	// Notify any listeners and add to change list if this is a watched file/dir
	
	// Again - this isn't correct, just a hack
	bool bPathIsDir = !file.has_extension();
	boost::filesystem::path pathDir = bPathIsDir ? file : file.parent_path();

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




