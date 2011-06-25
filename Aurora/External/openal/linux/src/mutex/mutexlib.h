/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * mutexlib.h
 *
 * Include that sorts out which mutex library we're using.
 */
#ifndef MUTEXLIB_H_
#define MUTEXLIB_H_

#ifdef USE_EMPTY_LOCKS

#define mlCreateMutex()
#define mlDestroyMutex(m)
#define mlLockMutex(m)
#define mlTryLockMutex(m)
#define mlUnlockMutex(m)

#else

#ifdef USE_POSIXMUTEX
#include "posixmutex.h"

#define mlCreateMutex()      Posix_CreateMutex()
#define mlDestroyMutex(m)    Posix_DestroyMutex(m)
#define mlLockMutex(m)       Posix_LockMutex(m)
#define mlTryLockMutex(m)    Posix_TryLockMutex(m)
#define mlUnlockMutex(m)     Posix_UnlockMutex(m)

#elif defined USE_WINDOWSMUTEX
#include "windowsmutex.h"

#define mlCreateMutex()      Windows_CreateMutex()
#define mlDestroyMutex(m)    Windows_DestroyMutex(m)
#define mlLockMutex(m)       Windows_LockMutex(m)
#define mlTryLockMutex(m)    Windows_TryLockMutex(m)
#define mlUnlockMutex(m)     Windows_UnlockMutex(m)

#else /* USE_WINDOWSMUTEX */

/* #warning "No mutex package?" */

#endif /* USE_POSIXMUTEX */

#endif /* USE_EMPTYLOCKS */

#endif /* MUTEXLIB_H_ */
