/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * alsa.c
 *
 * alsa backend
 */

#ifndef _SVID_SOURCE
#define _SVID_SOURCE
#endif /* _SVID_SOURCE */

#include "arch/interface/interface_sound.h"
#include "arch/alsa/alsa.h"

#include <sys/asoundlib.h>

#ifdef UNUSED
#undef UNUSED
#endif /* UNUSED */

#include "al_config.h"
#include "al_main.h"
#include "al_siteconfig.h"

#include <stdio.h>
#include <stdlib.h>

/* alsa stuff */

/* convert from AL to ALSA format */
static int AL2ALSAFMT(ALenum format);
static ALenum ALSA2ALFMT(int fmt, int chans);

static struct {
	snd_pcm_t *handle;
	fd_set fd_set;
} alsa_info;

void *release_alsa(void *handle) {
	snd_pcm_t *phandle;

	if(handle == NULL) {
		return NULL;
	}

	phandle = handle;

	snd_pcm_close(phandle);

	return NULL;
}

void *grab_read_alsa( void ) {
	/* no capture support */
	return NULL;
}

void *grab_write_alsa( void ) {
	snd_pcm_t *handle;
	int err;
	Rcvar rcv;
	int alsa_card = 0;
	char *name = NULL;
	char temp[256];

	rcv = rc_lookup("alsa-device");
	if(rcv != NULL) {
		if(rc_type(rcv) == ALRC_STRING) {
			rc_tostr0(rcv, temp, 256);

			alsa_card = snd_card_name(temp);

			if(alsa_card < 0) {
				fprintf(stderr,
					"grab_alsa: invalid card specified %s\n",
					temp);

				alsa_card = 0;
			}
		}
	}

	err = snd_pcm_open(&handle, alsa_card, 0, SND_PCM_OPEN_PLAYBACK);
	if(err < 0) {
		fprintf(stderr, "alsa init failed %s\n", snd_strerror(err));
		return NULL;
	}

	alsa_info.handle = handle;
	_alBlitBuffer = alsa_blitbuffer;

	snd_card_get_name(alsa_card, &name);

	fprintf(stderr, "alsa init ok, using %s\n", name);

	free(name);

	return handle;
}

ALboolean set_read_alsa( UNUSED(void *handle),
			 UNUSED(ALuint *bufsiz),
			 UNUSED(ALenum *fmt),
			 UNUSED(ALuint *speed))  {
	/* no capture support */
	return AL_FALSE;
}

ALboolean set_write_alsa( void *handle,
		    ALuint *bufsiz,
		    ALenum *fmt,
		    ALuint *speed ) {
	snd_pcm_format_t *format;
	snd_pcm_channel_params_t setup;
	int err;
	ALuint channels = _al_ALCHANNELS(*fmt);

	memset(&setup, 0, sizeof setup);

	format = &setup.format;
	
	format->interleave = 1;
	format->format = AL2ALSAFMT(*fmt);
	format->voices = channels;
	format->rate   = *speed;

	setup.channel = SND_PCM_CHANNEL_PLAYBACK;
	setup.mode    = SND_PCM_MODE_BLOCK;

	setup.buf.block.frag_size = *bufsiz;
	setup.buf.block.frags_min  = 1;
	setup.buf.block.frags_max  = 2;

	err = snd_pcm_channel_params(handle, &setup);
	if(err < 0) {
		fprintf(stderr, "set_alsa %s\n", snd_strerror(err));
		return AL_FALSE;
	}

	*speed    = format->rate;
	*bufsiz   = setup.buf.block.frag_size;

	*fmt = ALSA2ALFMT(format->format, format->voices);

	err = snd_pcm_playback_prepare(handle);
	if(err < 0) {
		fprintf(stderr, "set_alsa %s\n", snd_strerror(err));
		return AL_FALSE;
	}

	return AL_TRUE;
}

void alsa_blitbuffer(void *handle, void *data, int bytes) {
	snd_pcm_t *phandle = handle;
	int err;

	if(handle == NULL) {
		return;
	}

	err = snd_pcm_write(phandle, data, bytes);
	if(err < 0) {
		fprintf(stderr,
		"alsa_blitbuffer %s\n",	snd_strerror(err));
	}

	return;
}
	
static int AL2ALSAFMT(ALenum format) {
	switch(format) {
		case AL_FORMAT_STEREO8:  return SND_PCM_SFMT_U8;
		case AL_FORMAT_MONO8:    return SND_PCM_SFMT_U8;
		case AL_FORMAT_STEREO16: return SND_PCM_SFMT_S16_LE;
		case AL_FORMAT_MONO16:   return SND_PCM_SFMT_S16_LE;
		default: break;
	}

	return -1;
}

static ALenum ALSA2ALFMT(int fmt, int chans) {
	switch(fmt) {
		case SND_PCM_SFMT_U8:
			if(chans == 1) {
				return AL_FORMAT_MONO8;
			} else if (chans == 2) {
				return AL_FORMAT_STEREO8;
			}
			break;
		case SND_PCM_SFMT_S16_LE:
			if(chans == 1) {
				return AL_FORMAT_MONO16;
			} else if (chans == 2) {
				return AL_FORMAT_STEREO16;
			}
			break;
		}

	return -1;
}

