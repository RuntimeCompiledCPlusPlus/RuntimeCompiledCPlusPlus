//
//	The MIT License
//
//	Copyright (c) 2010 James E Beveridge
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

//	This sample code is for my blog entry titled, "Understanding ReadDirectoryChangesW"
//	http://qualapps.blogspot.com/2010/05/understanding-readdirectorychangesw.html
//	See ReadMe.txt for overview information.

#pragma once

#ifndef THREADSAFEQUEUE_DEF
#define THREADSAFEQUEUE_DEF

#include <Windows.h>
#include <concrt.h>
#include <list>

template <typename T> class ThreadSafeQueue : protected std::list<T>
{
public:
	ThreadSafeQueue(int nMaxCount)
	{
		m_bOverflow = false;

		m_hSemaphore = ::CreateSemaphore(
			NULL,		// no security attributes
			0,			// initial count
			nMaxCount,	// max count
			NULL);		// anonymous

		InitializeCriticalSection(&m_Crit);
	}
	
	~ThreadSafeQueue()
	{
		::CloseHandle(m_hSemaphore);
		m_hSemaphore = NULL;
		DeleteCriticalSection(&m_Crit);
	}

	void push(T& item)
	{
		EnterCriticalSection(&m_Crit);
		push_back( item );
		LeaveCriticalSection(&m_Crit);

		if (!::ReleaseSemaphore(m_hSemaphore, 1, NULL))
		{
			// If the semaphore is full, then take back the entry.
			pop_back();
			if (GetLastError() == ERROR_TOO_MANY_POSTS)
			{
				m_bOverflow = true;
			}
		}
	}

	bool pop(T& item)
	{
		EnterCriticalSection(&m_Crit);

		// If the user calls pop() more than once after the
		// semaphore is signaled, then the semaphore count will
		// get out of sync.  We fix that when the queue empties.
		if (__super::empty())
		{
			while (::WaitForSingleObject(m_hSemaphore, 0) != WAIT_TIMEOUT)
				1;
			return false;
		}

		item = front();
		pop_front();

		LeaveCriticalSection(&m_Crit);

		return true;
	}

	// If overflow, use this to clear the queue.
	void clear()
	{
		EnterCriticalSection(&m_Crit);

		for (DWORD i=0; i<size(); i++)
			WaitForSingleObject(m_hSemaphore, 0);

		__super::clear();

		m_bOverflow = false;

		LeaveCriticalSection(&m_Crit);
	}

	bool overflow()
	{
		return m_bOverflow;
	}

	bool empty()
	{
		EnterCriticalSection(&m_Crit);
		bool bEmpty = __super::empty();
		LeaveCriticalSection(&m_Crit);

		return bEmpty;
	}

	HANDLE GetWaitHandle() { return m_hSemaphore; }

protected:
	HANDLE m_hSemaphore;

	CRITICAL_SECTION m_Crit;

	bool m_bOverflow;
};


#endif // THREADSAFEQUEUE_DEF
