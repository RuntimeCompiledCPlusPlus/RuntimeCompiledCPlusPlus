/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * bsd_dsp.c
 *
 * functions related to the aquisition and management of /dev/dsp under
 * bsd.
 *
 */
#include "al_siteconfig.h"

#include <AL/altypes.h>
#include <AL/alkludge.h>

#include "al_siteconfig.h"

#include <fcntl.h>
#include <machine/soundcard.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "arch/interface/interface_sound.h"
#include "arch/bsd/bsd_dsp.h"

#include "al_main.h"
#include "al_debug.h"
#include "alc/alc_context.h"

static int alcChannel_to_dsp_channel(ALuint alcc);

/* /dev/dsp variables */
static fd_set dsp_fd_set;
static int dsp_fd      = -1; /* /dev/dsp file descriptor */
static int mixer_fd    = -1; /* /dev/mixer file descriptor */

/* convert the format channel from /dev/dsp to openal format */
static int BSD2ALFMT(int fmt, int channels) {
	switch(fmt) {
		case AFMT_U8:
			switch(channels) {
				case 1: return AL_FORMAT_MONO8;
				case 2: return AL_FORMAT_STEREO8;
				default: return -1;
			}
			break;
		case AFMT_S16_LE:
			switch(channels) {
				case 1: return AL_FORMAT_MONO16;
				case 2: return AL_FORMAT_STEREO16;
				default: return -1;
			}
			break;
		default:
#ifdef DEBUG_MAXIMUS
			fprintf(stderr, "unsupported dsp format\n");
#endif
			return -1;
			break;
	}

	return -1;
}

/* convert the format channel from openal to /dev/dsp format */
static int AL2BSDFMT(int fmt) {
	switch(fmt) {
		case AL_FORMAT_MONO16:
		case AL_FORMAT_STEREO16:
		  return AFMT_S16_LE;
		  break;
		case AL_FORMAT_MONO8:
		case AL_FORMAT_STEREO8:
		  return AFMT_U8;
		  break;
		default:
#ifdef DEBUG_MAXIMUS
		  fprintf(stderr, "unknown format 0x%x\n", fmt);
#endif
		  break;
	}

	return -1;
}

/*
 *
 *  Format of divisor is bit field where:
 *
 *
 *         MSHW          LSHW
 *  [ some big value |     x  ]
 *
 *  where x is translated into 2^x, and used as the
 *  dma buffer size.
 *
 */
void *grab_native(void) {
	const char *dsppath = "/dev/dsp";
	int divisor = _alSpot(_AL_DEF_BUFSIZ) | (2<<16);

	dsp_fd = open(dsppath, O_WRONLY | O_NONBLOCK);

	if(dsp_fd < 0) {
		perror("open /dev/dsp");
		return NULL;
	}

	if(fcntl(dsp_fd, F_SETFL, ~O_NONBLOCK) == -1) {
		perror("fcntl");
	}

	if(ioctl(dsp_fd, SNDCTL_DSP_SETFRAGMENT, &divisor) < 0) {
		perror("ioctl SETFRAGMENT");
	}

	_alBlitBuffer = native_blitbuffer;

	FD_ZERO(&dsp_fd_set);
	FD_SET(dsp_fd, &dsp_fd_set);

	/* now get mixer_fd */
	mixer_fd = grab_mixerfd();

#ifdef DEBUG
	fprintf(stderr, "Got /dev/dsp\n");
#endif
	return &dsp_fd;
}

int grab_mixerfd(void) {
	mixer_fd = open("/dev/mixer", O_WRONLY | O_NONBLOCK);

	if(mixer_fd > 0) {
		if(fcntl(mixer_fd, F_SETFL, ~O_NONBLOCK) == -1) {
			perror("fcntl");
		}

		return mixer_fd;
	} else {
		perror("open /dev/mixer");
	}

	return -1;
}

void native_blitbuffer(void *handle, void *dataptr, int bytes_to_write) {
	struct timeval tv = { 1, 0 }; /* wait 1 sec max */
	int iterator = 0;
	int err;
	int fd;
	
	if(handle == NULL) {
		return;
	}
	
	fd = *(int *) handle;

	for(iterator = bytes_to_write; iterator > 0; ) {
		if(select(fd + 1, NULL, &dsp_fd_set, NULL, &tv) == 0) {
			/* timeout occured, don't try and write */
#ifdef DEBUG_MAXIMUS
			fprintf(stderr, "native_blitbuffer: timeout occured\n");
#endif
			return;
		}

		FD_ZERO(&dsp_fd_set);
		FD_SET(fd, &dsp_fd_set);

		ASSERT(iterator > 0);
		ASSERT(iterator <= bytes_to_write);

		err = write(fd,
			    (char *) dataptr + bytes_to_write - iterator,
			    iterator);
		if(err < 0) {
#ifdef DEBUG_MAXIMUS
			perror("write");
#endif
			return;
		}

		iterator -= err;
	};

	return;
}

void release_native(void *handle) {
	int handle_fd;

	if(handle == NULL) {
		return;
	}

	handle_fd = *(int *) handle;

	if(ioctl(handle_fd, SNDCTL_DSP_RESET) < 0) {
#ifdef DEBUG_MAXIMUS
		fprintf(stderr, "Couldn't reset dsp\n");
#endif
	}

	ioctl(handle_fd, SNDCTL_DSP_SYNC, NULL);
	if((close(handle_fd) < 0) || (close(mixer_fd) < 0)) {
		return;
	}

	*(int *) handle = -1;
	mixer_fd        = -1;

	return;
}

float get_nativechannel(UNUSED(void *handle), ALuint channel) {
	int retval = 0;

	channel = alcChannel_to_dsp_channel(channel);

	if(ioctl(mixer_fd, MIXER_READ(channel), &retval) < 0) {
		return -1;
	}

	return (retval >> 8) / 100.0;
}


/*
 * Okay:
 *
 * Set audio channel expects an integer, in the range of
 * 0 - 100.  But wait!  It expects the integer to be 
 * partitioned into a 16bit empty, L/R channel pair (high bits left,
 * low bits right), each 8 bit pair in the range 0 - 100.
 *
 * Kludgey, and obviously not the right way to do this
 */
int set_nativechannel(UNUSED(void *handle), ALuint channel, float volume) {
	int unnormalizedvolume;

	unnormalizedvolume = volume * 100;
	unnormalizedvolume <<= 8;
	unnormalizedvolume += (volume * 100);
	
	channel = alcChannel_to_dsp_channel(channel);

	if(ioctl(mixer_fd, MIXER_WRITE(channel), &unnormalizedvolume) < 0) {
		return -1;
	}

	return 0;
}

/* convert the mixer channel from ALC to /dev/mixer format */
static int alcChannel_to_dsp_channel(ALuint alcc) {
	switch(alcc) {
		case ALC_CHAN_MAIN_LOKI: return SOUND_MIXER_VOLUME;
		case ALC_CHAN_CD_LOKI:   return SOUND_MIXER_CD;
		case ALC_CHAN_PCM_LOKI:  return SOUND_MIXER_PCM;
		default: return -1;
	}

	return -1;
}

void pause_nativedevice(void *handle) {
	int fd;

	if(handle == NULL) {
		return;
	}

	fd = *(int *) handle;

	if(ioctl(fd, SNDCTL_DSP_POST, 0) == -1) {
		perror("ioctl");
	}

	return;
}

void resume_nativedevice(void *handle) {
	int fd;

	if(handle == NULL) {
		return;
	}

	fd = *(int *) handle;

	if(ioctl(fd, SNDCTL_DSP_POST, 0) == -1) {
		perror("ioctl");
	}

	return;
}

ALsizei capture_nativedevice(UNUSED(void *handle),
			  UNUSED(void *capture_buffer),
			  UNUSED(int bufsiz)) {
	/* unimplemented */
	return 0;
}

ALboolean set_write_native(UNUSED(void *handle),
		     UNUSED(unsigned int *bufsiz),
		     ALenum *fmt,
		     unsigned int *speed) {
	ALuint channels = _al_ALFORMAT(*fmt);

	if(dsp_fd < 0) {
		return AL_FALSE;
	}

	*fmt  = AL2BSDFMT(*fmt);

	/* reset card defaults */
	if(ioctl(dsp_fd, SNDCTL_DSP_RESET, NULL) < 0) {
#ifdef DEBUG_MAXIMUS
		perror("set_devsp reset ioctl");
#endif
		return AL_FALSE;
	}

	if(ioctl(dsp_fd, SNDCTL_DSP_SPEED, speed) < 0) {
#ifdef DEBUG_MAXIMUS
		fprintf(stderr, "speed %d\n", *speed);
		perror("set_devsp speed ioctl");
#endif
		return AL_FALSE;
	}

	if(ioctl(dsp_fd, SNDCTL_DSP_SETFMT, fmt) < 0) {
#ifdef DEBUG_MAXIMUS
		fprintf(stderr, "fmt %d\n", *fmt);
		perror("set_devsp format ioctl");
#endif
		return AL_FALSE;
	}

	if(ioctl(dsp_fd, SOUND_PCM_WRITE_CHANNELS, &channels)) {
#ifdef DEBUG_MAXIMUS
		fprintf(stderr, "channels %d\n", channels);
		perror("set_devsp channels ioctl");
#endif
		return AL_FALSE;
	}


	*fmt = BSD2ALFMT(*fmt, channels);

	return AL_TRUE;
}

ALboolean set_read_native(UNUSED(void *handle),
		     UNUSED(unsigned int *bufsiz),
		     UNUSED(ALenum *fmt),
		     UNUSED(unsigned int *speed)) {
	return AL_FALSE;
}
