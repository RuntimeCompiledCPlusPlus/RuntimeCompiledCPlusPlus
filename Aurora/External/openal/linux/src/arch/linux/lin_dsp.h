/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * lin_dsp.h
 *
 * Native linux backend implementation.
 */
#ifndef LIN_DSP_H_
#define LIN_DSP_H_

#include "AL/alc.h"

/*
 * grab_write_native( void )
 *
 * Returns handle to an audio device suitable for writing data to, or NULL if
 * no such device is available.
 */
void *grab_write_native( void );

/*
 * grab_read_native( void )
 *
 * Returns handle to an audio device suitable for reading data from, or NULL if
 * no such device is available.
 */
void *grab_read_native( void );

/*
 * release_native( void *handle )
 *
 * Releases an audio device aquired using grab_foo_native.  Returns
 * AL_TRUE if release was successful, AL_FALSE if handle was invalid or the
 * device could not be released for some reason.
 */
void release_native( void *handle );

/*
 * set_read_native ( void *handle, ALuint *bufsiz,
 *                       ALenum *fmt, ALuint *speed )
 *
 * Sets the parameters associated with the device specified by handle.
 * Because we follow a meet-or-exceed policty, *bufsiz, *fmt, and *speed might be
 * different from the parameters initially passed, so the caller should check
 * these after a succesfull call.
 *
 * Returns AL_FALSE if the parameters could not be matched or exceeded.
 */
ALboolean set_read_native( void *handle,
		     ALuint *bufsiz,
		     ALenum *fmt,
		     ALuint *speed );

/*
 * set_write_native( void *handle, ALuint *bufsiz,
 *                        ALenum *fmt, ALuint *speed )
 *
 * Sets the parameters associated with the device specified by handle.
 * Because we follow a meet-or-exceed policty, *bufsiz, *fmt, and *speed might be
 * different from the parameters initially passed, so the caller should check
 * these after a succesfull call.
 *
 * Returns AL_FALSE if the parameters could not be matched or exceeded.
 */
ALboolean set_write_native( void *handle,
			    ALuint *bufsiz,
			    ALenum *fmt,
			    ALuint *speed );

/*
 * native_blitbuffer( void *handle, void *data, int bytes )
 *
 * Writes bytes worth of data from data to the device specified by handle.
 * dataptr is an interleaved array.
 */
void native_blitbuffer( void *handle, void *data, int bytes );

/*
 * get_nativechannel( void *handle, ALuint channel )
 *
 * Returns normalized audio setting for handle at channel.
 */
float get_nativechannel( void *handle, ALCenum channel );

/*
 * set_nativechannel( void *handle, ALuint channel, float volume )
 *
 * Sets normalized audio setting for handle at channel.
 */
int set_nativechannel( void *handle, ALCenum channel, float volume );

/*
 * pause_nativedevice( void *handle )
 *
 * Informs device specified by handle that it's about to get paused.
 */
void pause_nativedevice( void *handle );

/*
 * resume_nativedevice( void *handle )
 *
 * Informs device specified by handle that it's about to get unpaused.
 */
void resume_nativedevice( void *handle );

/*
 * capture_nativedevice( void *handle, void *capture_buffer, int bufsiz )
 *
 * capture data from the audio device specified by handle, into
 * capture_buffer, which is bufsiz long.
 */
ALsizei capture_nativedevice( void *handle, void *capture_buffer, int bufsiz );

#endif /* LIN_DSP_H_ */
