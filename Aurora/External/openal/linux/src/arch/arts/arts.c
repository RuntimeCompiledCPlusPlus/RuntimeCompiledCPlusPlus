/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * arts.c
 *
 * arts backend.
 */
#include "al_siteconfig.h"
#include <AL/altypes.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "al_main.h"
#include "al_debug.h"

#include "arch/arts/arts.h"
#include "arch/interface/interface_sound.h"

#include <artsc.h>

#define DEF_SPEED	_ALC_CANON_SPEED
#define DEF_SIZE	_AL_DEF_BUFSIZ
#define DEF_SAMPLES     (DEF_SIZE / 2)
#define DEF_CHANNELS	2

static struct {
	arts_stream_t stream;
} arts_info;

static const char *genartskey(void);

void *grab_read_arts(void) {
	return NULL;
}

void *grab_write_arts(void) {
	int err = arts_init();

	if(err < 0) {
		fprintf(stderr, "aRTs init failed: %s\n",
			arts_error_text(err));
		return NULL;
	}

	_alBlitBuffer = arts_blitbuffer;

	fprintf(stderr, "arts grab audio ok\n");
		
	_alDebug(ALD_CONTEXT, __FILE__, __LINE__,
		"arts grab audio ok");

        return &arts_info.stream;
}

void arts_blitbuffer(void *handle, void *data, int bytes)  {
	arts_stream_t *ap = (arts_stream_t *) handle;

	if(handle == NULL) {
		return;
	}

	arts_write(*ap, data, bytes);

        return;
}

void release_arts(void *handle) {
	arts_stream_t *ap = (arts_stream_t *) handle;

	if(handle == NULL) {
		return;
	}

	arts_close_stream(*ap);
	arts_free();

	return;
}

static const char *genartskey(void) {
	static char retval[1024];

	sprintf(retval, "openal%d", getpid());

	return retval;
}

ALboolean set_write_arts(UNUSED(void *handle),
		   ALuint *bufsiz,
		   ALenum *fmt,
		   ALuint *speed) {
	ALuint chans = _al_ALCHANNELS(*fmt);

	arts_stream_set(arts_info.stream, ARTS_P_BUFFER_SIZE, *bufsiz);

	fprintf(stderr, "set_arts forcing speed from %d to 44100\n", *speed);

	*speed = 44100;

	/* FIXME: how do we know if this failed? */
	arts_info.stream = arts_play_stream(*speed,
					    chans, 
					    _al_formatbits(*fmt),
					    genartskey());

	*bufsiz = arts_stream_get(arts_info.stream, ARTS_P_BUFFER_SIZE);

        return AL_TRUE;
}

ALboolean set_read_arts(UNUSED(void *handle),
		   UNUSED(ALuint *bufsiz),
		   UNUSED(ALenum *fmt),
		   UNUSED(ALuint *speed)) {
	return AL_FALSE;
}
