/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * posixthreads.h
 *
 * Posix thread backend prototypes.
 */
#ifndef POSIX_THREADS_H_
#define POSIX_THREADS_H_

#include <pthread.h>

/*
 * typedef our ThreadID type.
 */
typedef pthread_t *ThreadID;

/*
 * Posix_CreateThread( int (*fn )(void *), void *data )
 *
 * Creates a thread, which starts by running fn and passing data to it.
 */
extern pthread_t *Posix_CreateThread( int (*fn )(void *), void *data );

/*
 * Posix_WaitThread( pthread_t *waitfor )
 *
 * Waits for waitfor to terminate before returning.
 */
extern int Posix_WaitThread( pthread_t *waitfor );

/*
 * Posix_KillThread( pthread_t *killit )
 *
 * Kills the thread specified by killit.
 */
extern int Posix_KillThread( pthread_t *killit );

/*
 * Posix_SelfThread( void )
 *
 * Returns the identifier for the callee's thread.
 */
extern unsigned int Posix_SelfThread( void );

/*
 * Posix_ExitThread( int retval )
 *
 * Forces the callee to terminate.
 */
extern void Posix_ExitThread( int retval );

/*
 * Posix_AtForkThread( void (*prepare)( void ),
 *                     void (*parent)( void ),
 *                     void (*child)( void ));
 *
 * Sets up handlers to be called when a process forks.
 */
extern int Posix_AtForkThread( void (*prepare)( void ),
			       void (*parent)( void ),
			       void (*child)( void ));

#endif /* POSIX_THREADS_H_ */
