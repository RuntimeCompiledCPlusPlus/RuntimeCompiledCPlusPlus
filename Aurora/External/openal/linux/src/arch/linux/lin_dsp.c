/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * lin_dsp.c
 *
 * functions related to the aquisition and management of /dev/dsp under
 * linux.
 *
 */
#include "al_siteconfig.h"
#include "al_config.h"
#include "al_main.h"
#include "al_debug.h"
#include "alc/alc_context.h"
#include "arch/interface/interface_sound.h"
#include "arch/linux/lin_dsp.h"

#include <AL/altypes.h>

#include <errno.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define DONTCARE ( 1 << 16)

/* convert an alc channel to a linux dsp channel */
static int alcChannel_to_dsp_channel(ALuint alcc);

/* /dev/dsp variables */
static fd_set dsp_fd_set;
static int mixer_fd    = -1; /* /dev/mixer file read/write descriptor */
static ALboolean use_select = AL_TRUE;

/* gets user prefered path */
static const char *lin_getwritepath(void);
static const char *lin_getreadpath(void);
static int aquire_read(void);
static int grab_mixerfd(void);

/* set the params associated with a file descriptor */
static int set_fd(int dsp_fd, ALboolean readable,
		     ALuint *bufsiz,
		     ALuint *fmt,
		     ALuint *speed,
		     ALuint *channels);


extern const char *sys_errlist[];

/* convert the format channel from /dev/dsp to openal format */
static int LIN2ALFMT(int fmt, int channels) {
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
static int AL2LINFMT(int fmt) {
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
void *grab_write_native(void) {
	static int write_fd;
	Rcvar rc_use_select;
	const char *writepath = lin_getwritepath();
	int divisor = DONTCARE | _alSpot( _ALC_DEF_BUFSIZ );

	write_fd = open(writepath, O_WRONLY | O_NONBLOCK);
	if(write_fd < 0) {
		perror("open /dev/dsp");
		return NULL;
	}

	if(ioctl(write_fd, SNDCTL_DSP_SETFRAGMENT, &divisor) < 0) {
		perror("ioctl SETFRAGMENT grab");
	}

	if(fcntl(write_fd, F_SETFL, ~O_NONBLOCK) == -1) {
		perror("fcntl");
	}

	_alBlitBuffer = native_blitbuffer;

	/* now get mixer_fd */
	mixer_fd = grab_mixerfd();

	/*
	 * Some drivers, ie aurreal ones, don't implemented select.
	 * I like to use select, as it ensures that we don't hang on
	 * a bum write forever, but if you're in the unadmirable position
	 * of having one of these cards I'm sure it's a small comfort.
	 *
	 * So, we have a special alrc var: native-use-select, that, if
	 * set to #f, causes us to not do a select on the fd before
	 * writing to it.
	 */
	use_select = AL_TRUE;

	rc_use_select = rc_lookup("native-use-select");
	if(rc_use_select != NULL) {
		if(rc_type(rc_use_select) == ALRC_BOOL) {
			use_select = rc_tobool(rc_use_select);
		}
	}

#ifdef DEBUG
	fprintf(stderr, "grab_native: (path %s fd %d)\n", writepath, write_fd);
#endif

	return &write_fd;
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
void *grab_read_native(void) {
	static int read_fd;

#ifndef CAPTURE_SUPPORT
	return NULL;
#endif

	read_fd = aquire_read();
	if( read_fd < 0) {
		return NULL;
	}

	return &read_fd;
}

static int grab_mixerfd(void) {
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

void release_native(void *handle) {
	int handle_fd;

	if(handle == NULL) {
		return;
	}

	handle_fd = *(int *) handle;

	if(ioctl(handle_fd, SNDCTL_DSP_RESET) < 0) {
#ifdef DEBUG
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

float get_nativechannel(UNUSED(void *handle), ALCenum channel) {
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
int set_nativechannel(UNUSED(void *handle), ALCenum channel, float volume) {
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

	if(fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		perror("fcntl");
	}

#if 0
	if(ioctl(fd, SNDCTL_DSP_POST, 0) == -1) {
		perror("ioctl");
	}
#endif

	return;
}

void resume_nativedevice(void *handle) {
	int fd;

	if(handle == NULL) {
		return;
	}

	fd = *(int *) handle;

	if(fcntl(fd, F_SETFL, ~O_NONBLOCK) == -1) {
		perror("fcntl");
	}
/*
	if(ioctl(fd, SNDCTL_DSP_SYNC, 0) == -1) {
		perror("ioctl");
	}
 */

	return;
}

static const char *lin_getwritepath(void) {
	Rcvar devdsp_path = rc_lookup("lin-dsp-path");
	static char retval[65]; /* FIXME */

	if(devdsp_path == NULL) {
		return "/dev/dsp";
	}

	switch(rc_type(devdsp_path)) {
		case ALRC_STRING:
			rc_tostr0(devdsp_path, retval, 64);
			break;
		default:
			return "/dev/dsp";
	}

	return retval;
}

static const char *lin_getreadpath(void) {
	Rcvar devdsp_path = rc_lookup("lin-dsp-read-path");
	static char retval[65]; /* FIXME */

	if(devdsp_path == NULL) {
		/*
		 * no explicit read path?  try the default
		 * path.
		 */
		devdsp_path = rc_lookup("lin-dsp-path");
	}

	if(devdsp_path == NULL) {
		return "/dev/dsp";
	}

	switch(rc_type(devdsp_path)) {
		case ALRC_STRING:
			rc_tostr0(devdsp_path, retval, 64);
			break;
		default:
			return "/dev/dsp";
	}


	/*
	 * stupid.  /dev/dsp1 cannot be used for reading,
	 * only /dev/dsp
	 */
	if(retval[strlen(retval) - 1] == '1') {
		retval[strlen(retval) - 1] = '\0';
	}

	return retval;
}

/* capture data from the audio device */
ALsizei capture_nativedevice(UNUSED(void *handle),
			  UNUSED(void *capture_buffer),
			  UNUSED(int bufsiz)) {
	int read_fd = *(int *)handle;
	int retval;

	retval = read(read_fd, capture_buffer, bufsiz);
	return retval > 0 ? retval : 0;
}

static int aquire_read(void) {
	int read_fd;
	const char *readpath  = lin_getreadpath();
	int divisor = _alSpot(_ALC_DEF_BUFSIZ) | (1<<16);

	read_fd = open( readpath, O_RDONLY | O_NONBLOCK );
	if(read_fd >= 0) {
#if 0 /* Reads should be non-blocking */
		if(fcntl(read_fd, F_SETFL, ~O_NONBLOCK) == -1) {
			perror("fcntl");
		}
#endif
		if(ioctl(read_fd, SNDCTL_DSP_SETFRAGMENT, &divisor) < 0) {
			perror("ioctl SETFRAGMENT");
		}
	}

	return read_fd;
}

ALboolean set_write_native(UNUSED(void *handle),
			   ALuint *bufsiz,
			   ALenum *fmt,
			   ALuint *speed) {
	int write_fd = *(int *)handle;
	ALuint linformat;
	ALuint channels = _al_ALCHANNELS(*fmt);
	int err;

	if(write_fd < 0) {
		return AL_FALSE;
	}

	linformat = AL2LINFMT(*fmt);

	err = set_fd(write_fd, AL_FALSE, bufsiz, &linformat, speed, &channels);
	if(err < 0) {
#ifdef DEBUG
		fprintf(stderr, "Could not do write_fd\n");
#endif
		return AL_FALSE;
	}

	/* set format for caller */
	*fmt = LIN2ALFMT(linformat, channels);

	return AL_TRUE;
}

ALboolean set_read_native(UNUSED(void *handle),
			  ALuint *bufsiz,
			  ALenum *fmt,
			  ALuint *speed) {
	int read_fd = *(int *)handle;
	ALuint linformat;
	ALuint channels = 1;

	linformat = AL2LINFMT(*fmt);

	if(set_fd(read_fd, AL_TRUE, bufsiz, &linformat, speed, &channels) >= 0) {
		/* set format for caller */
		*fmt = LIN2ALFMT(linformat, channels);

		return AL_TRUE;
	}

	return AL_FALSE;
}

void native_blitbuffer(void *handle, void *dataptr, int bytes_to_write) {
	struct timeval tv = { 0, 800000 }; /* at most .8 secs */
	int iterator = 0;
	int err;
	int fd;
	
	if(handle == NULL) {
		return;
	}

	fd = *(int *) handle;

	ASSERT( fd >= 0 );

	for(iterator = bytes_to_write; iterator > 0; ) {
		FD_ZERO(&dsp_fd_set);
		FD_SET(fd, &dsp_fd_set);
	
		if(use_select == AL_TRUE) {
			err = select(fd + 1, NULL, &dsp_fd_set, NULL, &tv);
			if(FD_ISSET(fd, &dsp_fd_set) == 0) {
				/*
				 * error or timeout occured, don't try
				 * and write.
				 */
				fprintf(stderr,
				"native_blitbuffer: select error occured\n");
				return;
			}
		}

		ASSERT(iterator > 0);
		ASSERT(iterator <= bytes_to_write);

		err = write(fd,
			    (char *) dataptr + bytes_to_write - iterator,
			    iterator);

		if(err < 0) {
#ifdef DEBUG_MAXIMUS
			fprintf( stderr, "write error: ( fd %d error %s )\n",
				fd, sys_errlist[ errno ] );
#endif
			ASSERT( 0 );
			return;
		}

		iterator -= err;
	};

	return;
}

static int set_fd(int dsp_fd, ALboolean readable,
		     ALuint *bufsiz,
		     ALuint *fmt,
		     ALuint *speed,
		     ALuint *channels) {
	struct audio_buf_info info;
	int divisor = DONTCARE | _alSpot( *bufsiz );

	if(dsp_fd < 0) {
		return -1;
	}

#ifdef DEBUG
	fprintf( stderr, "set_fd in: bufsiz %d fmt 0x%x speed %d channels %d\n",
		 *bufsiz, *fmt, *speed, *channels );
#endif

	if( ioctl(dsp_fd, SNDCTL_DSP_SETFRAGMENT, &divisor ) < 0) {
#ifdef DEBUG
		perror("ioctl SETFRAGMENT");
#endif
	}

	/* reset card defaults */
	if(ioctl(dsp_fd, SNDCTL_DSP_RESET, NULL) < 0) {
#ifdef DEBUG
		perror("set_devsp reset ioctl");
#endif
		return AL_FALSE;
	}

	if(ioctl(dsp_fd, SNDCTL_DSP_SETFMT, fmt) < 0) {
#ifdef DEBUG
		fprintf(stderr, "fmt %d\n", *fmt);
		perror("set_devsp format ioctl");
#endif
		return AL_FALSE;
	}

	if(ioctl(dsp_fd, SNDCTL_DSP_CHANNELS, channels)) {
#ifdef DEBUG
		fprintf(stderr, "channels %d\n", *channels);
		perror("set_devsp channels ioctl");
#endif
		return AL_FALSE;
	}

	if( readable == AL_TRUE ) {
		/*
		 * This is for reading.  Don't really use
		 * the speed argument.
		 */
		*speed = 8000;

		/* Try to set the speed (ignore value), then read it back */
                ioctl(dsp_fd, SNDCTL_DSP_SPEED, speed);
                if (ioctl(dsp_fd, SOUND_PCM_READ_RATE, speed) < 0) {
#ifdef DEBUG
			char errbuf[256];

			sprintf(errbuf, "(fd %d speed %d)", dsp_fd, *speed);

			perror(errbuf);
			return AL_FALSE;
#endif
                }
/*printf("Set recording rate: %d\n", *speed);*/

	} else {
		/* writable, set speed, otherwise no */
		if(ioctl(dsp_fd, SNDCTL_DSP_SPEED, speed) < 0) {
#ifdef DEBUG
			char errbuf[256];

			sprintf(errbuf, "(fd %d speed %d)", dsp_fd, *speed);

			perror(errbuf);
			return AL_FALSE;
#endif
		}

		if(ioctl(dsp_fd, SNDCTL_DSP_GETOSPACE, &info) < 0) {
#ifdef DEBUG
			perror("ioctl SNDCTL_DSP_GETOSPACE");
#endif
		}

		/* set bufsiz correctly */

		*bufsiz = info.fragsize;

#ifdef DEBUG
		if( *bufsiz & (*bufsiz - 1) ) {
			fprintf( stderr, "Non power-of-two bufsiz %d\n",
				*bufsiz );
		}
#endif
	}

#ifdef DEBUG
	fprintf( stderr, "set_fd out: bufsiz %d fmt 0x%x speed %d channels %d\n",
		 *bufsiz, *fmt, *speed, *channels );
#endif

	return 0;
}
