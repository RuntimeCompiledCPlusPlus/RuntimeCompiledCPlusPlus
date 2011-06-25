/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * windowsthreads.h
 *
 * Windows thread backend prototypes.
 */
#ifndef WINDOWS_THREADS_H_
#define WINDOWS_THREADS_H_

#include <windows.h>

/*
 * typedef our ThreadID type.
 */
typedef HANDLE ThreadID;

/*
 * Windows_CreateThread( int (*fn)(void *), void *data )
 */
extern ThreadID Windows_CreateThread( int (*fn)(void *), void *data );

/*
 * Windows_WaitThread( ThreadID waitfor )
 *
 * Wait for waitfor to terminate of natural causes.
 */
extern int Windows_WaitThread( ThreadID waitfor );

/*
 * Windows_KillThread( ThreadID killit )
 *
 * Kill killit and return when it's dead.
 */
extern int Windows_KillThread( ThreadID killit );

/*
 * Windows_SelfThread( void )
 *
 * Returns the identifier for the callee's thread.
 */
extern unsigned int Windows_SelfThread( void );

/*
 * Windows_ExitThread( int retval )
 *
 * Terminate the callee's thread.
 */
extern void Windows_ExitThread( int retval );

#endif /* WINDOWS_THREADS_H_ */
