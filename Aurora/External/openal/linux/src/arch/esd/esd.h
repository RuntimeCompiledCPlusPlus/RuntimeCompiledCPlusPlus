/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * esd.h
 *
 * backend prototypes for esound daemon
 *
 */
#ifndef ESD_H_
#define ESD_H_

#include <AL/altypes.h>

void *grab_read_esd(void);
void *grab_write_esd(void);

ALboolean set_write_esd(void *handle,
		  ALuint *bufsiz,
		  ALenum *fmt,
		  ALuint *speed);
ALboolean set_read_esd(void *handle,
		  ALuint *bufsiz,
		  ALenum *fmt,
		  ALuint *speed);
void release_esd(void *handle);

void esd_blitbuffer(void *handle, void *data, int bytes);
void pause_esd(void *handle);
void resume_esd(void *handle);

#endif /* ESD_H_ */
