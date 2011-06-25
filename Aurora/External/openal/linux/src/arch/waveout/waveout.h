/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * waveout.h
 *
 * backend prototypes for waveout target.
 *
 * Kind of silly, but we just pumpout data to a file,
 *
 */
#ifndef WAVEOUT_H_
#define WAVEOUT_H_

#include <AL/altypes.h>

void *grab_read_waveout(void);
void *grab_write_waveout(void);

ALboolean set_write_waveout(void *handle,
		  ALuint *bufsiz,
		  ALenum *fmt,
		  ALuint *speed);
ALboolean set_read_waveout(void *handle,
		  ALuint *bufsiz,
		  ALenum *fmt,
		  ALuint *speed);
void release_waveout(void *handle);

void waveout_blitbuffer(void *handle, void *data, int bytes);

#endif /* WAVEOUT_H_ */
