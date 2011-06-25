/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * posixmutex.h
 *
 * Header of posix mutex implementation.
 */
#ifndef POSIX_MUTEXS_H_
#define POSIX_MUTEXS_H_

#include <pthread.h>

/*
 * typedef the MutexID type to pthread_mutex_t *.
 */
typedef pthread_mutex_t *MutexID;

/*
 * Posix_CreateMutex( void )
 *
 * Creates a POSIX mutex, returning it, or NULL on error.
 */
MutexID Posix_CreateMutex( void );

/*
 * Posix_DestroyMutex( MutexID mutex )
 *
 * Destroys the POSIX mutex mutex.
 */
void Posix_DestroyMutex( MutexID mutex );

/*
 * Posix_LockMutex( MutexID mutex )
 *
 * Locks the POSIX mutex mutex.
 */
void Posix_LockMutex( MutexID mutex );

/*
 * Posix_TryLockMutex( MutexID mutex )
 *
 * Returns 1 and locks this mutex if possible, 0 ( with no lock change) if
 * not.
 */
int Posix_TryLockMutex( MutexID mutex );

/*
 * Posix_UnlockMutex( MutexID mutex )
 *
 * Unlocks POSIX mutex.
 */
void Posix_UnlockMutex( MutexID mutex );

#endif /* POSIX_MUTEXS_H_ */
