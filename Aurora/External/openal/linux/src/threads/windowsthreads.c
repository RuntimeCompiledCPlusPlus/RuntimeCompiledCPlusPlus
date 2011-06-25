/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * windowsthreads.c
 *
 * windows thread implementation.
 */
#include "al_siteconfig.h"
#include <stdio.h>
#include <stdlib.h>

#include "windowsthreads.h"

#include "al_main.h"

typedef int (*ptfunc)(void *);

HANDLE Windows_CreateThread(int (*fn)(void *), UNUSED(void *data)) {
	HANDLE retval;
	DWORD dummy;

	retval = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) fn, data, 0, &dummy);

	return retval;
}

extern int Windows_WaitThread(HANDLE waitfor) {
	BOOL stillrunning;
	DWORD ExitCode; // termination status
	int retval;
	int tries = 20; /* gets tries iterations before we nuke it */
	const int interval = 40000;

	if(waitfor == NULL) {
		return -1;
	}

	do {
		stillrunning = GetExitCodeThread(waitfor, &ExitCode);
		if(stillrunning == 0) {
			break;
		}

		/* thread is still running, be nice and wait a bit */
		_alMicroSleep(interval);
	} while(tries--);
	
	retval = TerminateThread(waitfor, 0);

	if(retval == 0) {
		return -1;
	}

	return 0;
}

extern int Windows_KillThread(HANDLE killit) {
	int retval;

	if(killit == NULL) {
		return -1;
	}

	retval = TerminateThread(killit, 0);

	if(retval == 0) {
		return -1;
	}

	return 0;
}

extern void Windows_ExitThread(int retval) {
	ExitThread(retval);

	return;
}


extern unsigned int Windows_SelfThread(void) {
	return 	(unsigned int) GetCurrentThreadId();
}

	
