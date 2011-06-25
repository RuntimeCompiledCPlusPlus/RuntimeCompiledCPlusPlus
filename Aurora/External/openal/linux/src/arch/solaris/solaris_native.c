/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * solaris_native.c
 *
 * functions related to the aquisition and management of the native 
 * audio on Solaris.
 *
 * This is a crude first-cut implementation, which doesn't do any
 * sophisticated error handling at all yet.  
 *   John E. Stone, September 13, 2000 
 *   <johns@megapixel.com>
 *   <j.stone@acm.org>
 */
#include <AL/altypes.h>

#include <stdio.h>

#include "arch/interface/interface_sound.h"
#include "arch/solaris/solaris_native.h"

#include "al_main.h"
#include "al_debug.h"
#include "alc/alc_context.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/audioio.h>  /* Sun's audio system headers */

typedef struct {
  int fd;                /* file descriptor of audio device          */
  audio_info_t ainfo;    /* audio info structure used in ioctl calls */
} solaris_audio;

solaris_audio gaudio;

/* XXX Gross global variables */

#define EMAIL_ADDRESS "j.stone@acm.org"

static const char *implement_me(const char *fn) {
	static char retval[2048];

	sprintf(retval,
	"%s is not implemented under Solaris.  Please contact %s for\n"
	"information on how you can help get %s implemented on Solaris.\n",
	fn, EMAIL_ADDRESS, fn);

	return retval;
}

void *grab_native(void) {
  fprintf(stderr, "solaris_native: opening /dev/audio\n");
  gaudio.fd = open("/dev/audio", O_WRONLY | O_NONBLOCK);
  if (gaudio.fd < 0) {
    perror("open /dev/audio");
    return NULL;
  }
  
  if(fcntl(gaudio.fd, F_SETFL, ~O_NONBLOCK) == -1) {
    perror("fcntl");
  }

  fprintf(stderr, "Opened /dev/audio successfully\n");

  _alBlitBuffer = native_blitbuffer;

  return &gaudio;
}

void native_blitbuffer(UNUSED(void *handle),
		    void *dataptr,
		    int bytes_to_write) {
  fprintf(stderr, "Writing to audio device...\n");
  write(gaudio.fd, (char *) dataptr, bytes_to_write);
  return;
}

void release_native(UNUSED(void *handle)) {
  fprintf(stderr, "Closing audio device...\n");
  close(gaudio.fd);
  return;
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

ALsizei capture_nativedevice(UNUSED(void *handle),
			  UNUSED(void *capture_buffer),
			  UNUSED(int bufsiz)) {
	return 0; /* unimplemented */
}

ALboolean set_write_native(UNUSED(void *handle),
		     unsigned int *bufsiz,
		     ALenum *fmt,
		     unsigned int *speed) {
  ALuint channels = _al_ALCHANNELS(*fmt);

  fprintf(stderr, "Setting audio device...\n");
  gaudio.ainfo.play.sample_rate = *speed;
  gaudio.ainfo.play.channels = channels;
  switch (*fmt) {
    case AL_FORMAT_MONO8: 
    case AL_FORMAT_STEREO8: 
      gaudio.ainfo.play.precision = 8;
      gaudio.ainfo.play.encoding = AUDIO_ENCODING_ULINEAR;
      break;
    case AL_FORMAT_MONO16:
    case AL_FORMAT_STEREO16:
      gaudio.ainfo.play.precision = 16;
#ifdef WORDS_BIGENDIAN
      gaudio.ainfo.play.encoding = AUDIO_ENCODING_SLINEAR_BE;
#else
      gaudio.ainfo.play.encoding = AUDIO_ENCODING_SLINEAR_LE;
#endif /* WORDS_BIGENDIAN */
      break;

    default:
      fprintf(stderr, "Unsuported audio format:%d\n", *fmt);
      return AL_FALSE;
  }

  gaudio.ainfo.play.buffer_size = *bufsiz;
  if (ioctl(gaudio.fd, AUDIO_SETINFO, &gaudio.ainfo) < 0) 
    return AL_FALSE;
  else  
    return AL_TRUE;
}

ALboolean set_read_native(UNUSED(void *handle),
		     unsigned int *bufsiz,
		     ALenum *fmt,
		     unsigned int *speed) {
	return AL_FALSE;
}
