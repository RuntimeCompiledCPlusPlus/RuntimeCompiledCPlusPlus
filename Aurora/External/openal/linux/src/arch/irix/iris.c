/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * iris.c
 *
 * functions related to the aquisition and management of the native 
 * audio on IRIX.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <dmedia/audio.h>
#include <errno.h>

/* dmedia complains */
#undef AL_CHANNELS
#undef AL_GAIN
#undef AL_VERSION
#undef AL_INVALID_VALUE

#include <AL/altypes.h>
#include <AL/alkludge.h>

#include "arch/interface/interface_sound.h"
#include "arch/irix/iris.h"

#include "al_main.h"
#include "al_debug.h"
#include "alc/alc_context.h"
#include "al_siteconfig.h"

#define EMAIL_ADDRESS "erik@ehofman.com"

typedef struct {
	ALport port;
	ALconfig config;
} ALhandle;

ALhandle alh_buff = {NULL, NULL};
ALhandle *alh = &alh_buff;

static int sampleWidth = 16;
static int numChannels = 2;

static const char *implement_me(const char *fn) {
	static char retval[2048];

	sprintf(retval,
	"%s is not implemented under IRIX.  Please contact %s for\n"
	"information on how you can help get %s implemented on IRIX.\n",
	fn, EMAIL_ADDRESS, fn);

	return retval;
}

void *grab_read_native(void) {
	fprintf(stderr,  implement_me("grab_read_native"));

	return NULL;
}

void *grab_write_native(void) {
	_alBlitBuffer = native_blitbuffer;

	if (alh->config != NULL) {	/* Port already configured? */
		/*
		 * Now attempt to open an audio output port using this config
		 */
		alh->port = alOpenPort("OpenAL native port", "w", alh->config);
		alFreeConfig(alh->config);
		if (!alh->port) {
#ifdef DEBUG
			fprintf(stderr, "Can't open an audio port:%s\n",
				alGetErrorString(oserror()));
#endif
			return NULL;
		}

		return alh->port;
	} else {
		/*
		 *
		 * JV:
		 *
		 * okay, this is really cheesy.  Handle is unused, but the
		 * idea is that we return non-null here without getting a port
		 * or making a new config so that set_format can do it's
		 * stuff.
		 */
		return (void *) 1;
	}

	return NULL;
}

void native_blitbuffer(UNUSED(void *handle),
		    void *dataptr,
		    int bytes_to_write) {
	int frames_to_write;

	frames_to_write = bytes_to_write / (sampleWidth/8);
	frames_to_write /= numChannels;

	alWriteFrames(alh->port, dataptr, frames_to_write);

	return;
}

void release_native(UNUSED(void *handle)) {
	if (alh->port != NULL) {
		/*
		 * Close an audio input port
		 */
		alClosePort(alh->port);
	}
}

int set_nativechannel(UNUSED(void *handle),
		   UNUSED(ALCenum channel),
		   UNUSED(float volume)) {
	fprintf(stderr,  implement_me("set_nativechannel"));

	return 0;
}

void pause_nativedevice(UNUSED(void *handle)) {
	fprintf(stderr,  implement_me("pause_nativedevice"));

	return;
}

void resume_nativedevice(UNUSED(void *handle)) {
	fprintf(stderr,  implement_me("resume_nativedevice"));

	return;
}

float get_nativechannel(UNUSED(void *handle), UNUSED(ALCenum channel)) {
	fprintf(stderr,  implement_me("get_nativechannel"));

	return 0.0;
}

/* capture data from the audio device */
ALsizei capture_nativedevice(UNUSED(void *handle),
			  UNUSED(void *capture_buffer),
			  UNUSED(int bufsiz)) {
	/* unimplemented */

	return 0;
}

ALboolean set_write_native(UNUSED(void *handle),
		     unsigned int *bufsiz,
		     ALenum *fmt,
		     unsigned int *speed) {
	ALuint channels = _al_ALCHANNELS(*fmt);

	int result;
        unsigned int data_format;
	ALpv params[2];

	if (alh->port != NULL) {
		/*
		 * boo
		 *
		 */
		alClosePort(alh->port);

		alh->port = NULL;
	}

	if (alh->config == NULL) {
		/*
		 * Create a config.
		 * This defaults to stereo, 16-bit integer data
		 */
		alh->config = alNewConfig();
		if (!alh->config) {
#ifdef DEBUG
			fprintf(stderr, "Can't create ALconfig:%s\n",
				alGetErrorString(oserror()));
#endif
			return NULL;
		}
	}

	if (alh->port == NULL) {	/* If port not initialized */
		/*
		 * Change the size of the audio queue
		 * and the number of audio channels.
		 */
		alSetChannels(alh->config, channels);
		alSetQueueSize(alh->config, *bufsiz);
	}

	/*
	 * Change the audio format
	 */
	alSetSampFmt(alh->config, AL_SAMPFMT_TWOSCOMP);
	switch(*fmt) {
	case AL_FORMAT_MONO8:
	case AL_FORMAT_STEREO8:
		data_format = AL_SAMPLE_8;
		sampleWidth = 8;
		break;
	case AL_FORMAT_MONO16:
	case AL_FORMAT_STEREO16:
		data_format = AL_SAMPLE_16;
		sampleWidth = 16;
		break;
	default:
		fprintf(stderr, "Unsuported audio format:%d\n", *fmt);
		return AL_FALSE;
	}
	alSetWidth(alh->config, data_format);

	numChannels = channels;

	/*
	 * Change the playback sample rate
	 */
	params[0].param = AL_MASTER_CLOCK;
	params[0].value.i = AL_CRYSTAL_MCLK_TYPE;
	params[1].param = AL_OUTPUT_RATE;
	params[1].value.i = *speed;
	alSetParams(AL_DEFAULT_DEVICE, params, 2);

	/*
	 * Alter configuration parameters, if possible
	 */
        if (alh->port == NULL)
        {
                /*
                 * Now attempt to open an audio output port using this config
                 */
                alh->port = alOpenPort("OpenAL native port", "w", alh->config);
                alFreeConfig(alh->config);
                alh->config = NULL;
                if (!alh->port) {
#ifdef DEBUG
                        fprintf(stderr, "Can't open an audio port:%s\n",
                                alGetErrorString(oserror()));
#endif
                        return AL_FALSE;
                }

                return AL_TRUE;
        }
        else
        {
                result = alSetConfig(alh->port, alh->config);
        }

	if (!result) {
#ifdef DEBUG
		fprintf(stderr, "Can't change configuration:%s\n",
			alGetErrorString(oserror()));
#endif
		return AL_FALSE;
	}

	return AL_TRUE;
}

ALboolean set_read_native(UNUSED(void *handle),
		     UNUSED(unsigned int *bufsiz),
		     UNUSED(ALenum *fmt),
		     UNUSED(unsigned int *speed)) {
	return AL_FALSE;
}
