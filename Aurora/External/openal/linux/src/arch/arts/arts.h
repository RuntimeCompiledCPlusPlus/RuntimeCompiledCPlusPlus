/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * arts.h
 *
 * backend prototypes for KDE's aRTs.
 *
 */
#ifndef ARTS_H_
#define ARTS_H_

#include <AL/altypes.h>

void *grab_read_arts(void);
void *grab_write_arts(void);

ALboolean set_write_arts(void *handle,
		   ALuint *bufsiz,
		   ALenum *fmt,
		   ALuint *speed);
ALboolean set_read_arts(void *handle,
		   ALuint *bufsiz,
		   ALenum *fmt,
		   ALuint *speed);
void release_arts(void *handle);

void arts_blitbuffer(void *handle, void *data, int bytes);

#endif /* ARTS_H_ */
