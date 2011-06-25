/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * windowsmutex.c
 *
 * windows mutex backend for out mutex lib interface.
 */
#include "al_siteconfig.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "windowsmutex.h"

#ifndef ASSERT
#ifdef DEBUG
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif /* DEBUG */
#endif /* ASSERT */

MutexID Windows_CreateMutex(void) {
	MutexID retval;

	retval = malloc(sizeof *retval);

	InitializeCriticalSection(retval);

	return retval;
}

void Windows_DestroyMutex(MutexID mutex) {
	DeleteCriticalSection(mutex);

	return;
}


int Windows_TryLockMutex(MutexID mutex) {
	BOOL err;
	
	err = TryEnterCriticalSection(mutex);

	if(err == 0) {
		return 0;
	}

	/* FIXME: we're reversing the meaning here.  We should actually
	 * return -1 when err is 0 and 0 otherwise. */
	
	/* lock failed */	
	return -1;
}

void Windows_LockMutex(MutexID mutex) {
	EnterCriticalSection(mutex);
	
	return;

}
void Windows_UnlockMutex(MutexID mutex) {
	LeaveCriticalSection(mutex);

	return;
}

