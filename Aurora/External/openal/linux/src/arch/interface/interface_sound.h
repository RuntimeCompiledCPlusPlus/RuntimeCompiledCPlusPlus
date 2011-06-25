/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * interface_sound.h
 *
 * High level prototypes for sound device aquisition and management.
 *
 */
#ifndef INTERFACE_SOUND_H_
#define INTERFACE_SOUND_H_

#include <AL/altypes.h>

/*
 * _alSpot( ALuint num )
 *
 * Returns smallest power of two that meets or exceeds num.
 */
ALuint _alSpot( ALuint num );

/*
 * grab_write_audiodevice( void )
 *
 * Returns handle to an audio device suitable for writing data to, or NULL if
 * no such device is available.
 */
void *grab_write_audiodevice( void );

/*
 * grab_read_audiodevice( void )
 *
 * Returns handle to an audio device suitable for reading data from, or NULL if
 * no such device is available.
 */
void *grab_read_audiodevice( void );

/*
 * release_audiodevice( void *handle )
 *
 * Releases an audio device aquired using grab_foo_audiodevice.  Returns
 * AL_TRUE if release was successful, AL_FALSE if handle was invalid or the
 * device could not be released for some reason.
 */
ALboolean release_audiodevice( void *handle );

/*
 * set_read_audiodevice( void *handle, ALuint *bufsiz,
 *                       ALenum *fmt, ALuint *speed )
 *
 * Sets the parameters associated with the device specified by handle.
 * Because we follow a meet-or-exceed policty, *bufsiz, *fmt, and *speed might be
 * different from the parameters initially passed, so the caller should check
 * these after a succesfull call.
 *
 * Returns AL_FALSE if the parameters could not be matched or exceeded.
 */
ALboolean set_read_audiodevice( void *handle, ALuint *bufsiz,
				ALenum *fmt, ALuint *speed );

/*
 * set_write_audiodevice( void *handle, ALuint *bufsiz,
 *                        ALenum *fmt, ALuint *speed )
 *
 * Sets the parameters associated with the device specified by handle.
 * Because we follow a meet-or-exceed policty, *bufsiz, *fmt, and *speed might be
 * different from the parameters initially passed, so the caller should check
 * these after a succesfull call.
 *
 * Returns AL_FALSE if the parameters could not be matched or exceeded.
 */
ALboolean set_write_audiodevice( void *handle, ALuint *bufsiz,
				 ALenum *fmt, ALuint *speed );

/*
 * Function pointer to a function that writes bytes_to_write worth of data
 * from dataptr to the device specified by handle.  dataptr is an interleaved
 * array.
 */
void (*_alBlitBuffer)( void *handle, void *dataptr, int bytes_to_write );

/*
 * get_audiochannel( void *handle, ALuint channel )
 *
 * Returns normalized audio setting for handle at channel.
 */
float get_audiochannel( void *handle, ALuint channel );

/*
 * set_audiochannel( void *handle, ALuint channel, float volume )
 *
 * Sets normalized audio setting for handle at channel.
 */
void set_audiochannel( void *handle, ALuint channel, float volume );

/*
 * pause_audiodevice( void *handle )
 *
 * Informs device specified by handle that it's about to get paused.
 */
void pause_audiodevice( void *handle );

/*
 * resume_audiodevice( void *handle )
 *
 * Informs device specified by handle that it's about to get unpaused.
 */
void resume_audiodevice( void *handle );

/*
 * capture_audiodevice( void *handle, void *capture_buffer, int bufsiz )
 *
 * capture data from the audio device specified by handle, into
 * capture_buffer, which is bufsiz long.
 */
ALsizei capture_audiodevice( void *handle, void *capture_buffer, int bufsiz );

#endif /* INTERFACE_SOUND_H_ */
