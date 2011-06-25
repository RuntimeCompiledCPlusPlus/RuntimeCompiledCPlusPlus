/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_mixer.h
 *
 * Prototypes, macros and definitions related to the control and
 * execution of the mixing "thread".
 *
 * The mixing "thread" is responsible for managing playing sources,
 * applying the requisite filters, mixing in audio data from said sources,
 * etc.
 *
 */
#ifndef _AL_MIXER_H_
#define _AL_MIXER_H_

#include <AL/altypes.h>

/*
 * Number of sources for which optimized mixing functions exist.
 */
#define MAXMIXSOURCES 64

/*
 * int (*mixer_iterate)( void *dummy )
 *
 * our main mixing function.
 */
extern int (*mixer_iterate)( void *dummy );

/*
 * The mixing function checks this variable for equality with AL_TRUE.  When
 * this is the case, it exits.  The default is AL_FALSE.
 */
extern volatile ALboolean time_for_mixer_to_die;

/*
 * _alInitMixer( void )
 *
 * Create and initialize data structures needed by the mixing function.
 */
ALboolean _alInitMixer( void ); 

/*
 * _alSetMixer( ALboolean synchronous )
 *
 * Inform the mixer that settings may have changed.  Data structures can
 * be/are updated to reflect new settings in the current context.
 *
 * Synchronous, if AL_FALSE, causes a new thread to be launched.
 */
void _alSetMixer( ALboolean synchronous );

/*
 * void _alDestroyMixer( void )
 *
 * Deallocate data allocated in _alInitMixer.
 */
void _alDestroyMixer( void );

/*
 * _alAddSourceToMixer( ALuint sid )
 *
 * "play" the source named by sid.  If sid does not refer to a valid source,
 * AL_INVALID_NAME is set.
 */
void _alAddSourceToMixer( ALuint sid );

/*
 * _alRemoveSourceFromMixer( ALuint sid )
 *
 * "stop" the source named by sid.  If sid does not refer to a valid source,
 * AL_INVALID_NAME is set.
 */
ALboolean _alRemoveSourceFromMixer( ALuint sid );

/*
 * _alAddSourceToMixer( ALuint cpid )
 *
 * "start" the capture named by cpid.  If cpid does not refer to a valid
 * capture, AL_INVALID_NAME is set.
 */
void _alAddCaptureToMixer( ALuint cpid );

/*
 * _alRemoveSourceFromMixer( ALuint cpid )
 *
 * "stop" the capture named by cpid.  If cpid does not refer to a valid
 * capture, AL_INVALID_NAME is set.
 */
void _alRemoveCaptureFromMixer( ALuint cpid );

/*
 * FL_alLockMixBuf( const char *fn, int ln )
 *
 * Lock the mixer mutex, handing fn and ln to _alLockPrintf
 */
void FL_alLockMixBuf( const char *fn, int ln );

/*
 * FL_alUnlockMixBuf( const char *fn, int ln )
 *
 * Unlock the mixer mutex, handing fn and ln to _alLockPrintf
 */
void FL_alUnlockMixBuf( const char *fn, int ln );

/*
 * functions to pause async mixer.  Oy Vey
 */

/*
 * _alLockMixerPause( void )
 *
 * Lock the MixerPause mutex, which is use to "pause" the mixer.
 */
void _alLockMixerPause( void );

/*
 * _alUnlockMixerPause( void )
 *
 * Unlock the MixerPause mutex, which is use to "resume" the mixer.
 */
void _alUnlockMixerPause( void );

/* macro madness */
#define _alLockMixBuf()   FL_alLockMixBuf(__FILE__, __LINE__)
#define _alUnlockMixBuf() FL_alUnlockMixBuf(__FILE__, __LINE__)

#endif
