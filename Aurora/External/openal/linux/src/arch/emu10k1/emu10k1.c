/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * emu10k1.c
 *
 * functions related to the aquisition and management of /dev/dsp under
 * linux, with an emu10k1 card.
 *
 */
#include "al_siteconfig.h"

#include <AL/altypes.h>
#include <AL/alkludge.h>

#include <fcntl.h>
#include <sys/soundcard.h>
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
#include "arch/emu10k1/emu10k1.h"

#include "al_config.h"
#include "al_main.h"
#include "al_debug.h"
#include "alc/alc_context.h"

/*

mgr_text -a fxc:out4l -a fxd:out4r -rfxc:out0l -rfxd:out0r -afx0:out0l
-a fx1:out0r -rfx0:out4l -rfx1:out4r

the spaces between -r and the route name are important. The command line
is longer because there are no stereo control. You can control left and
right independently.

If you want a route with volume control use:

mgr_text -v fxc:out4l


(instead of the -a option)
To change the volume:

mgr_text -c"Vol fxc:out4l" -s 0 (mute)
mgr_text -c"Vol fxc:out4l" -s 7fffffff (full volume)

and last RTFM ;)

man ./mgr_text.1

Rui Sousa

*/

/* convert an alc channel to a linux dsp channel */
static int alcChannel_to_dsp_channel(ALuint alcc);

/* /dev/dsp variables */
static fd_set dsp_fd_set;
static int write_fd[2] = { -1, -1 }; /* /dev/dsp file write descriptor */
static int read_fd     = -1; /* /dev/dsp file read descriptor */
static int mixer_fd    = -1; /* /dev/mixer file read/write descriptor */
static ALboolean use_select = AL_TRUE;
static ALboolean can_read = AL_FALSE;

/* gets user prefered path */
static void lin_getwritepair(char [2][65]);
static const char *lin_getreadpath(void);
static int aquire_read(void);
static int aquire_write(const char *path);
static int grab_mixerfd(void);

/* set the params associated with a file descriptor */
static int set_fd(int dsp_fd, ALboolean readable,
		     unsigned int *bufsiz,
		     unsigned int *fmt,
		     unsigned int *speed,
		     unsigned int *channels);

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
void *grab_write_emu10k1(void) {
	Rcvar rc_use_select;
	char writepair[2][65];

	lin_getwritepair(writepair);

	write_fd[0] = aquire_write(writepair[0]);
	if(write_fd[0] < 0) {
		return NULL;
	}

	write_fd[1] = aquire_write(writepair[1]);
	if(write_fd[1] < 0) {
		return NULL;
	}

	_alBlitBuffer = emu10k1_blitbuffer;

	/* now get mixer_fd */
	mixer_fd = grab_mixerfd();

	/*
	 * Some drivers, ie aurreal ones, don't implemented select.
	 * I like to use select, as it ensures that we don't hang on
	 * a bum write forever, but if you're in the unadmirable position
	 * of having one of these cards I'm sure it's a small comfort.
	 *
	 * So, we have a special alrc var: emu10k1-use-select, that, if
	 * set to #f, causes us to not do a select on the fd before
	 * writing to it.
	 */
	use_select = AL_TRUE;

	rc_use_select = rc_lookup("emu10k1-use-select");
	if(rc_use_select != NULL) {
		if(rc_type(rc_use_select) == ALRC_BOOL) {
			use_select = rc_tobool(rc_use_select);
		}
	}

#ifdef DEBUG
	fprintf(stderr, "grab_emu10k1: (path %s fd %d)\n", writepath, write_fd);
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
void *grab_read_emu10k1(void) {
#ifndef CAPTURE_SUPPORT
	return NULL;
#endif

	if(aquire_read() < 0) {
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

void emu10k1_blitbuffer(void *handle, void *dataptr, int bytes_to_write) {
	struct timeval tv = { 0, 800000 }; /* at most .8 secs */
	int iterator = 0;
	int err;
	int fds;
	int i;
	
	if(handle == NULL) {
		return;
	}
	
	fd = *(int *) handle;

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
				"emu10k1_blitbuffer: select error occured\n");
				return;
			}
		}

		ASSERT(iterator > 0);
		ASSERT(iterator <= bytes_to_write);

		err = write(fd,
			    (char *) dataptr + bytes_to_write - iterator,
			    iterator);
		ASSERT(err > 0);

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

void release_emu10k1(void *handle) {
	int fds;
	int i;

	if(handle == NULL) {
		return;
	}

	fds = *(int *) handle;

	for(i = 0; i < 2; i++) {
		if(ioctl(fds[i], SNDCTL_DSP_RESET) < 0) {
#ifdef DEBUG
			fprintf(stderr, "Couldn't reset dsp\n");
#endif
		}

		ioctl(fds[i], SNDCTL_DSP_SYNC, NULL);
		if(close(fds[i]) < 0) {
			return;
		}

		fds[i] = -1;
	}

	if(close(mixer_fd) < 0) {
		return;
	}

	mixer_fd        = -1;

	return;
}

float get_emu10k1channel(UNUSED(void *handle), ALuint channel) {
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
int set_emu10k1channel(UNUSED(void *handle), ALuint channel, float volume) {
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

void pause_emu10k1(void *handle) {
	int fds[2];
	int i;

	if(handle == NULL) {
		return;
	}

	fds = * (int *) handle;

	for(i = 0; i < 2; i++) {
		if(fcntl(fds[i], F_SETFL, O_NONBLOCK) == -1) {
			perror("fcntl");
		}
	}

#if 0
	if(ioctl(fd, SNDCTL_DSP_POST, 0) == -1) {
		perror("ioctl");
	}
#endif

	return;
}

void resume_emu10k1(void *handle) {
	int fds[2];
	int i;

	if(handle == NULL) {
		return;
	}

	fds = *(int *) handle;

	for(i = 0; i < 2; i++) {
		if(fcntl(fds[i], F_SETFL, ~O_NONBLOCK) == -1) {
			perror("fcntl");
		}
	}
/*
	if(ioctl(fd, SNDCTL_DSP_SYNC, 0) == -1) {
		perror("ioctl");
	}
 */

	return;
}

static int set_fd(int dsp_fd, ALboolean readable,
		     unsigned int *bufsiz,
		     unsigned int *fmt,
		     unsigned int *speed,
		     unsigned int *channels) {
	struct audio_buf_info info;

	int divisor = _alSpot(*bufsiz) | (1<<16);

	if(dsp_fd < 0) {
		return -1;
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

	if(ioctl(dsp_fd, SNDCTL_DSP_SETFRAGMENT, &divisor) < 0) {
#ifdef DEBUG
		fprintf(stderr, "bufsiz %d\n", *bufsiz);
		perror("ioctl SETFRAGMENT");
#endif
	}

	if(readable == AL_TRUE) {
		/* writable, set speed, otherwise no */
		*speed = 8000;

		/* set bufsiz correctly */
		if(ioctl(dsp_fd, SNDCTL_DSP_GETISPACE, &info) < 0) {
#ifdef DEBUG
			perror("ioctl SNDCTL_DSP_GETOSPACE");
#endif
		}
	} else {
		/* writable, set speed, otherwise no */
		if(ioctl(dsp_fd, SNDCTL_DSP_SPEED, speed) < 0) {
#ifdef DEBUG
			char errbuf[256];

			sprintf(errbuf, "(fd %d speed %d)", dsp_fd, *speed);

			perror(errbuf);
#endif
			return AL_FALSE;
		}

		/* set bufsiz correctly */
		if(ioctl(dsp_fd, SNDCTL_DSP_GETOSPACE, &info) < 0) {
#ifdef DEBUG
			perror("ioctl SNDCTL_DSP_GETOSPACE");
#endif
		}

		*bufsiz = info.fragsize;
	}


	return 0;
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
ALsizei capture_emu10k1(UNUSED(void *handle),
			  UNUSED(void *capture_buffer),
			  UNUSED(int bufsiz)) {
	int retval;

	if(can_read == AL_FALSE) {
		fprintf(stderr, "can't read\n");

		return;
	}

	retval = read(read_fd, capture_buffer, bufsiz);
        return retval > 0 ? retval : 0;
}

static int aquire_read(void) {
	const char *readpath  = lin_getreadpath();
	int divisor = _alSpot(_ALC_DEF_BUFSIZ) | (1<<16);

	read_fd = open(readpath, O_RDONLY | O_NONBLOCK );
	if(read_fd >= 0) {
		can_read = AL_TRUE;

		if(fcntl(read_fd, F_SETFL, ~O_NONBLOCK) == -1) {
			perror("fcntl");
		}

		if(ioctl(read_fd, SNDCTL_DSP_SETFRAGMENT, &divisor) < 0) {
			perror("ioctl SETFRAGMENT");
		}
	}

	fprintf(stderr, "aquire_read: (path %s fd %d)\n", readpath, read_fd);

	return read_fd;
}

ALboolean set_write_emu10k1(UNUSED(void *handle),
		     unsigned int *bufsiz,
		     unsigned int *fmt,
		     unsigned int *speed) {
	ALuint linformat;
	ALuint channels = _al_ALCHANNELS(*fmt);
	int *fds;
	int err;
	int i;

	fds = (int *) handle;

	linformat = AL2LINFMT(*fmt);

	for(i = 0; i < 2; i++) {
		if(fds[i] < 0) {
			return AL_FALSE;
		}

		err = set_fd(fds[i], AL_FALSE, bufsiz, &linformat, speed, &channels);
		if(err < 0) {
#ifdef DEBUG
			fprintf(stderr, "Could not do write_fd\n");
#endif
			return AL_FALSE;
		}
	}

	/* set format for caller */
	*fmt = LIN2ALFMT(linformat, channels);

	return AL_TRUE;
}

ALboolean set_read_emu10k1(UNUSED(void *handle),
		     unsigned int *bufsiz,
		     unsigned int *fmt,
		     unsigned int *speed) {
	ALuint linformat;
	ALuint channels = _al_ALCHANNELS(*fmt);

	linformat = AL2LINFMT(*fmt);

	if(can_read == AL_TRUE) {
		if(set_fd(read_fd, AL_TRUE, bufsiz, &linformat, speed, &channels) >= 0) {
			/* set format for caller */
			*fmt = LIN2ALFMT(linformat, channels);

			return AL_TRUE;
		}
	}

	return AL_FALSE;
}

static int aquire_write(const char *writepath) {
	char errmsg[256];
	int divisor = _alSpot(_ALC_DEF_BUFSIZ) | (1<<16);
	int retval;

	retval = open(writepath, O_WRONLY | O_NONBLOCK);
	if(retval < 0) {
		sprintf(errmsg, "aquire_write: open %s", writepath);
		perror(errmsg);
		return -1;
	}

	if(fcntl(retval, F_SETFL, ~O_NONBLOCK) == -1) {
		perror("fcntl");
	}

	if(ioctl(retval, SNDCTL_DSP_SETFRAGMENT, &divisor) < 0) {
		perror("ioctl SETFRAGMENT");
	}

	return retval;
}

void lin_getwritepair(char retref[2][65]) {
	Rcvar device_params = rc_lookup("device-params");
	Rcvar first_device, second_device;

	strncpy(retref[0], "/dev/dsp", 64);
	strncpy(retref[1], "/dev/dsp1", 64);

	if(device_params == NULL) {
		return;
	}

	first_device  = rc_car(device_params);
	second_device = rc_car(rc_cdr(device_params));

	rc_tostr0(first_device, retref[0], 64);
	rc_tostr0(second_device, retref[1], 64);

	return;
}
