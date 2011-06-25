/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * posixthreads.c
 *
 * posix thread implementation.
 */
#include "al_siteconfig.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "posixthreads.h"

#include "al_main.h"

#ifdef IRIX_TARGET
extern int pthread_atfork(void (*)(void), void (*)(void), void (*)(void));
#endif

typedef int (*ptfunc)(void *);

static void *RunThread(void *data) {
	ptfunc fn;

	fn = (ptfunc) data;

	fn(NULL);
	
	pthread_exit(NULL);
	return NULL;		/* Prevent compiler warning */
}


extern pthread_t *Posix_CreateThread(int (*fn)(void *), UNUSED(void *data)) {
	pthread_attr_t type;
	pthread_t *retval;
	
	retval = malloc(sizeof *retval);
	if(retval == NULL) {
		return NULL;
	}

	if(pthread_attr_init(&type) != 0) {
		free(retval);
		fprintf(stderr, "Couldn't pthread_attr_init\n");
		return NULL;
	}
	
	pthread_attr_setdetachstate(&type, PTHREAD_CREATE_JOINABLE);

	if(pthread_create(retval, &type, RunThread, (void *) fn) != 0) {
		fprintf(stderr, "No CreateThread\n");
		free(retval);
		return NULL;
	}

	return retval;
}

extern int Posix_WaitThread(pthread_t *waitfor) {
	int retval = -1;

	if(waitfor == NULL) {
		return -1;
	}

	retval = pthread_join(*waitfor, NULL);

	free(waitfor);

	return retval;
}

extern int Posix_KillThread(pthread_t *killit) {
	int retval;

	if(killit == NULL) {
		return -1;
	}

	retval = pthread_cancel(*killit);

	free(killit);

	return retval;
}

extern unsigned int Posix_SelfThread(void) {
	return (unsigned int) pthread_self();
}


extern void Posix_ExitThread(int retval) {
#if SIZEOF_VOID_P == 8
	pthread_exit( ( void *) ( int64_t ) retval );
#else
	pthread_exit( ( void *) retval );
#endif

	return;
}

#ifndef DARWIN_TARGET
extern int Posix_AtForkThread(void (*prepare)(void),
			  void (*parent)(void),
			  void (*child)(void)) {
	return pthread_atfork(prepare, parent, child);
}
#endif
