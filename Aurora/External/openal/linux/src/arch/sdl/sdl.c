/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * sdl.c
 *
 * SDL backend.
 */
#include "al_siteconfig.h"

#include <AL/altypes.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "al_main.h"
#include "al_debug.h"

#include "arch/sdl/sdl.h"
#include "arch/interface/interface_sound.h"
#include "alc/alc_context.h"

#include <SDL.h>
#include <SDL_audio.h>

#define DEF_SPEED	_ALC_CANON_SPEED
#define DEF_SIZE	_ALC_DEF_BUFSIZ
#define DEF_SAMPLES     (DEF_SIZE / 2)
#define DEF_CHANNELS	2
#define SDL_DEF_FMT	AUDIO_S16

struct {
	SDL_AudioSpec spec;
	Uint8   *sound;			/* Pointer to wave data */
	Uint32   soundlen;		/* Length of wave data */
	int      soundpos;		/* Current play position */
} sdl_info;

static void *ringbuffer;
static int  ringbuffersize;
static int  readOffset;
static int  writeOffset;

static void dummy(void *unused, Uint8 *stream, int len);

/* sdl stuff */
static void dummy(UNUSED(void *unused), Uint8 *stream, int len) {
	memcpy_offset(stream, ringbuffer, readOffset, len);
	readOffset += len;

	if(readOffset >= ringbuffersize) {
		readOffset  = 0;
		writeOffset = 0;
	}

	return;
}

void *grab_write_sdl(void) {
        sdl_info.spec.freq     = DEF_SPEED;
        sdl_info.spec.channels = DEF_CHANNELS;
        sdl_info.spec.samples  = DEF_SAMPLES;
        sdl_info.spec.size     = DEF_SIZE;
        sdl_info.spec.format   = SDL_DEF_FMT;
        sdl_info.spec.callback = dummy;

        if(SDL_OpenAudio(&sdl_info.spec, NULL) < 0) {
		/* maybe we need SDL_Init? */
		SDL_Init(SDL_INIT_AUDIO);

		if(SDL_OpenAudio(&sdl_info.spec, NULL) < 0) {
			_alDebug(ALD_CONTEXT, __FILE__, __LINE__,
				"No SDL: %s", SDL_GetError());
			return NULL;
		}
        }

	if(ringbuffer != NULL) {
		free(ringbuffer);
	}

	ringbuffersize = 2 * sdl_info.spec.size;
	ringbuffer     = malloc(ringbuffersize);
	readOffset      = 0;
	writeOffset     = 0;

	_alBlitBuffer = firsttime_sdl_blitbuffer;

	_alDebug(ALD_CONTEXT, __FILE__, __LINE__,
		"SDL grab audio ok");

        return &sdl_info.spec;
}

void *grab_read_sdl(void) {
	return NULL;
}

void firsttime_sdl_blitbuffer(UNUSED(void *handle), void *data, int bytes)  {
	offset_memcpy(ringbuffer, writeOffset, data, bytes);
	writeOffset = bytes;

	/* next time, skip pause audio */
	_alBlitBuffer = sdl_blitbuffer;

	/* start SDL callback mojo */
        SDL_PauseAudio(0);

        return;
}

void sdl_blitbuffer(UNUSED(void *handle), void *data, int bytes)  {
	SDL_LockAudio();
	while(writeOffset >= ringbuffersize) {
		SDL_UnlockAudio();
		SDL_Delay(0);
		SDL_LockAudio();
	}

	offset_memcpy(ringbuffer, writeOffset, data, bytes);
	writeOffset += bytes;

	SDL_UnlockAudio();

        return;
}

void release_sdl(UNUSED(void *handle)) {
	SDL_CloseAudio();

	return;
}

ALboolean set_write_sdl(UNUSED(void *handle),
		ALuint *bufsiz,
		ALenum *fmt,
		ALuint *speed) {
	ALuint bytesPerSample   = _al_formatbits(*fmt) >> 3;
	ALuint bufsizPot        = _alSpot( *bufsiz ); /* smallest power of two that
						 * meets or exceeds bufsiz */
	ALuint channels = _al_ALCHANNELS(*fmt);

        sdl_info.spec.freq     = *speed;
        sdl_info.spec.channels = channels;
        sdl_info.spec.samples  = bufsizPot / bytesPerSample;
        sdl_info.spec.size     = bufsizPot;
        sdl_info.spec.format   = _al_AL2ACFMT(*fmt);
        sdl_info.spec.callback = dummy; 

        SDL_CloseAudio();

        if(SDL_OpenAudio(&sdl_info.spec, NULL) < 0) {
		fprintf(stderr,
			"No SDL: %s\n", SDL_GetError());

                return AL_FALSE;
        }

	*bufsiz = sdl_info.spec.size;

	if(ringbuffer != NULL) {
		free(ringbuffer);
	}

	ringbuffersize = 2 * sdl_info.spec.size;
	ringbuffer     = malloc(ringbuffersize);
	readOffset      = 0;
	writeOffset     = 0;

	memset(ringbuffer, 0, ringbuffersize);

	_alBlitBuffer = firsttime_sdl_blitbuffer;

#ifdef DEBUG
	fprintf(stderr, "[%s:%d] SDL set audio ok\n",
		__FILE__, __LINE__);
#endif

	/*

	FIXME: should remove extraneous *channels and rely only on format
         */
	*fmt      = _al_AC2ALFMT(sdl_info.spec.format, sdl_info.spec.channels);
	*speed    = sdl_info.spec.freq;

        return AL_TRUE;
}

ALboolean set_read_sdl(UNUSED(void *handle),
		UNUSED(ALuint *bufsiz),
		UNUSED(ALenum *fmt),
		UNUSED(ALuint *speed)) {
	return AL_FALSE;
}
