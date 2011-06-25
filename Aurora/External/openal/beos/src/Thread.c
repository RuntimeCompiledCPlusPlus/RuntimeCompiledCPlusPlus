/*
 * OpenAL cross platform audio library
 *
 * Copyright (C) 1999-2000 by Authors.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include <OS.h>
#include <TLS.h>
#include <assert.h>
#include "Memory.h"
#include "Thread.h"

struct ALmutex {
	vint32		atom;
	sem_id		sem;
#ifndef NDEBUG
	thread_id	owner;
#endif
};

ALuint alimCreateTLS(ALvoid)
{
	return tls_allocate();
}

ALvoid alimDeleteTLS(ALuint handle)
{
}

ALvoid alimSetTLS(ALuint handle, ALvoid *data)
{
	tls_set(handle, data);
}

ALvoid *alimGetTLS(ALuint handle)
{
	return tls_get(handle);
}

ALmutex *alimCreateMutex(ALvoid)
{
	ALmutex *mutex = (ALmutex *) alimMemAlloc(sizeof(ALmutex));

	if (mutex != NULL) {
		mutex->atom = 0;
		mutex->sem = create_sem(0, "mutex");
#ifndef NDEBUG
		mutex->owner = 0;
#endif
	}
	return mutex;
}

ALvoid alimDeleteMutex(ALmutex *mutex)
{
	if (mutex != NULL) {
		delete_sem(mutex->sem);
		alimMemFree(mutex);
	}
}

ALvoid alimLockMutex(ALmutex *mutex)
{
	assert(mutex != NULL);

#ifndef NDEBUG
	if (mutex->owner == find_thread(NULL))
		alimDebugger("alimLockMutex: dead lock detected!");
#endif

	if (atomic_add(&mutex->atom, 1) != 0)
		acquire_sem(mutex->sem);

#ifndef NDEBUG
	mutex->owner = find_thread(NULL);
#endif

}

ALvoid alimUnlockMutex(ALmutex *mutex)
{
	assert(mutex != NULL);

#ifndef NDEBUG
	if (mutex->owner != find_thread(NULL))
		alimDebugger("alimUnlockMutex: only the owner can unlock the mutex!");
	mutex->owner = 0;
#endif

	if (atomic_add(&mutex->atom, -1) != 1)
		release_sem_etc(mutex->sem, 1, B_DO_NOT_RESCHEDULE);
}

ALvoid alimDebugger(const ALubyte *message)
{
	debugger(message);
}
