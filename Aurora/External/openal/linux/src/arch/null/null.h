/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * null.h
 *
 * backend prototypes for no output (but blocking!).
 *
 * Useful chiefly as a debugging tool.
 *
 */
#ifndef NULL_H_
#define NULL_H_

#include <AL/altypes.h>

void *grab_read_null(void);
void *grab_write_null(void);

ALboolean set_write_null(void *handle,
		  ALuint *bufsiz,
		  ALenum *fmt,
		  ALuint *speed);

ALboolean set_read_null(void *handle,
		  ALuint *bufsiz,
		  ALenum *fmt,
		  ALuint *speed);

void release_null(void *handle);

void null_blitbuffer(void *handle, void *data, int bytes);

#endif /* NULL_H_ */
