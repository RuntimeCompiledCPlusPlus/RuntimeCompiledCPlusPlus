/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * windowsmutex.h
 *
 * Header of windows mutex implementation.
 */
#ifndef WINDOWS_MUTEXS_H_
#define WINDOWS_MUTEXS_H_

#include <windows.h>

/*
 * typedef out MutexID type.
 */
typedef CRITICAL_SECTION *MutexID;

/*
 * Windows_CreateMutex( void )
 *
 * Creates and returns a Windows mutex.
 */
MutexID Windows_CreateMutex( void );

/*
 * Windows_DestroyMutex( MutexID mutex )
 *
 * Destroys a Windows mutex.
 */
void Windows_DestroyMutex( MutexID mutex );

/*
 * Windows_LockMutex( MutexID mutex )
 *
 * Locks the Windows mutex mutex.
 */
void Windows_LockMutex( MutexID mutex );

/*
 * Windows_TryLockMutex( MutexID mutex )
 *
 * Returns 1 and locks mutex if possible, 0 and no change if already locked.
 */
int Windows_TryLockMutex( MutexID mutex );

/*
 * Windows_UnlockMutex( MutexID mutex )
 *
 * Unlocks the Windows mutex mutex.
 */
void Windows_UnlockMutex( MutexID mutex );

#endif /* WINDOWS_MUTEXS_H */
