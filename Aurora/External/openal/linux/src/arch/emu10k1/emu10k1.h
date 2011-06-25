/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * emu10k1.h
 *
 * emu10k1 card implementation
 */
#ifndef EMU10K1_H_
#define EMU10K1_H_

#include "AL/alc.h"

void *grab_read_emu10k1(void);
void *grab_write_emu10k1(void);

ALboolean set_read_emu10k1(void *handle,
		     unsigned int *bufsiz,
		     unsigned int *fmt,
		     unsigned int *speed);
ALboolean set_write_emu10k1(void *handle,
		     unsigned int *bufsiz,
		     unsigned int *fmt,
		     unsigned int *speed);

void emu10k1_blitbuffer(void *handle, void *data, int bytes);
void release_emu10k1(void *handle);

float get_emu10k1channel(void *handle, ALCenum channel);
int set_emu10k1channel(void *handle, ALCenum channel, float volume);

void pause_emu10k1(void *handle);
void resume_emu10k1(void *handle);

/* capture data from the audio device */
ALsizei capture_emu10k1(void *handle, void *capture_buffer, int bufsiz);

#endif /* EMU10K1 */
