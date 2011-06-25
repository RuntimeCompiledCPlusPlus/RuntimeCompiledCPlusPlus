/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * interface_sound.c
 *
 * This file defines the high-level interface to the sound device.
 * It actually dispatches requests based on the architecture that
 * we're targetting.  See platform.h for more of a clue.
 */
#include "al_siteconfig.h"

#include "AL/altypes.h"
#include "al_config.h"
#include "al_main.h"

#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "arch/interface/interface_sound.h"
#include "arch/interface/platform.h"

typedef enum {
	LA_NONE,
	LA_NATIVE,  /* native audio for platform */
	LA_SDL,     /* SDL backend */
	LA_ALSA,    /* ALSA backend */
	LA_ARTS,    /* aRTs backend */
	LA_ESD,     /* esd backend */
	LA_WAVEOUT, /* wavefile output backend */
	LA_NULL,    /* no output backend */
	LA_EMU10K1
} lin_audio;

/* represents which backend we are using */
static lin_audio hardware_type = LA_NONE;

void *grab_write_audiodevice(void) {
	Rcvar device_params;
	Rcvar device_list;
	Rcvar device;
	void *retval = NULL;
	char adevname[64]; /* FIXME: magic number */

	device_list = rc_lookup("devices");
	while(device_list != NULL) {
		device      = rc_car( device_list );
		device_list = rc_cdr( device_list );

		switch(rc_type(device)) {
			case ALRC_STRING:
				rc_tostr0(device, adevname, 64);
				break;
			case ALRC_SYMBOL:
				rc_symtostr0(device, adevname, 64);
				break;
			case ALRC_CONSCELL:
				device_params = rc_cdr( device );
				if(device_params == NULL) {
					continue;
				}

				rc_define("device-params", device_params);
				rc_symtostr0(rc_car(device), adevname, 64);
				break;
			default:
				fprintf(stderr, "bad type %s for device\n",
					rc_typestr( rc_type( device ) ));
				continue;
		}

		if(strcmp(adevname, "dsp") == 0) {
			fprintf(stderr,
				"dsp is a deprecated device name.  Use native instead.\n");
			retval = grab_write_native();
			if(retval != NULL) {
				hardware_type = LA_NATIVE;
				return retval;
			}
		}
		if(strcmp(adevname, "native") == 0) {
			retval = grab_write_native();
			if(retval != NULL) {
				hardware_type = LA_NATIVE;
				return retval;
			}
		}

		if(strcmp(adevname, "arts") == 0) {
			retval = grab_write_arts();
			if(retval != NULL) {
				hardware_type = LA_ARTS;
				return retval;
			}
		}

		if(strcmp(adevname, "sdl") == 0) {
			retval = grab_write_sdl();
			if(retval != NULL) {
				hardware_type = LA_SDL;
				return retval;
			}
		}

		if(strcmp(adevname, "alsa") == 0) {
			retval = grab_write_alsa();
			if(retval != NULL) {
				hardware_type = LA_ALSA;
				return retval;
			}
		}

		if(strcmp(adevname, "esd") == 0) {
			retval = grab_write_esd();
			if(retval != NULL) {
				hardware_type = LA_ESD;
				return retval;
			}
		}

		if(strcmp(adevname, "waveout") == 0) {
			retval = grab_write_waveout();
			if(retval != NULL) {
				hardware_type = LA_WAVEOUT;
				return retval;
			}
		}

		if(strcmp(adevname, "null") == 0) {
			retval = grab_write_null();
			if(retval != NULL) {
				hardware_type = LA_NULL;
				return retval;
			}
		}

		if(strcmp(adevname, "emu10k1") == 0) {
			retval = grab_write_emu10k1();
			if(retval != NULL) {
				hardware_type = LA_EMU10K1;
				return retval;
			}
		}
	}

	/* no device list specified, try native or fail */
	retval = grab_write_native();
	if(retval != NULL) {
		hardware_type = LA_NATIVE;
		return retval;
	}

	return NULL;
}

void *grab_read_audiodevice(void) {
	Rcvar device_params;
	Rcvar device_list;
	Rcvar device;
	void *retval = NULL;
	char adevname[64]; /* FIXME: magic number */

	device_list = rc_lookup("devices");
	while(device_list != NULL) {
		device      = rc_car(device_list);
		device_list = rc_cdr(device_list);

		switch(rc_type(device)) {
			case ALRC_STRING:
				rc_tostr0(device, adevname, 64);
				break;
			case ALRC_SYMBOL:
				rc_symtostr0(device, adevname, 64);
				break;
			case ALRC_CONSCELL:
				device_params = rc_cdr(device);
				if(device_params == NULL) {
					continue;
				}

				rc_define("device-params", device_params);
				rc_symtostr0(rc_car(device), adevname, 64);
				break;
			default:
				fprintf(stderr, "bad type %s for device\n",
					rc_typestr( rc_type( device ) ));
				continue;
		}

		if(strcmp(adevname, "dsp") == 0) {
			fprintf(stderr,
				"dsp is a deprecated device name.  Use native instead.\n");
			retval = grab_read_native();
			if(retval != NULL) {
				hardware_type = LA_NATIVE;
				return retval;
			}
		}
		if(strcmp(adevname, "native") == 0) {
			retval = grab_read_native();
			if(retval != NULL) {
				hardware_type = LA_NATIVE;
				return retval;
			}
		}

		if(strcmp(adevname, "arts") == 0) {
			retval = grab_read_arts();
			if(retval != NULL) {
				hardware_type = LA_ARTS;
				return retval;
			}
		}

		if(strcmp(adevname, "sdl") == 0) {
			retval = grab_read_sdl();
			if(retval != NULL) {
				hardware_type = LA_SDL;
				return retval;
			}
		}

		if(strcmp(adevname, "alsa") == 0) {
			retval = grab_read_alsa();
			if(retval != NULL) {
				hardware_type = LA_ALSA;
				return retval;
			}
		}

		if(strcmp(adevname, "esd") == 0) {
			retval = grab_read_esd();
			if(retval != NULL) {
				hardware_type = LA_ESD;
				return retval;
			}
		}

		if(strcmp(adevname, "waveout") == 0) {
			retval = grab_read_waveout();
			if(retval != NULL) {
				hardware_type = LA_WAVEOUT;
				return retval;
			}
		}

		if(strcmp(adevname, "null") == 0) {
			retval = grab_read_null();
			if(retval != NULL) {
				hardware_type = LA_NULL;
				return retval;
			}
		}

		if(strcmp(adevname, "emu10k1") == 0) {
			retval = grab_write_emu10k1();
			if(retval != NULL) {
				hardware_type = LA_EMU10K1;
				return retval;
			}
		}
	}

	/* no device list specified, try native or fail */
	retval = grab_read_native();
	if(retval != NULL) {
		hardware_type = LA_NATIVE;
		return retval;
	}

	return NULL;
}

ALboolean release_audiodevice(void *handle) {
	ALboolean retval = AL_TRUE;
	int handle_fd;

	if(handle == NULL) {
		return AL_FALSE;
	} else {
		handle_fd = *((int *) handle);
	}

	switch(hardware_type) {
		case LA_NATIVE:
		  release_native(handle);
		  break;
		case LA_ALSA:
		  release_alsa(handle);
		  break;
		case LA_ARTS:
		  release_arts(handle);
		  break;
		case LA_ESD:
		  release_esd(handle);
		  break;
		case LA_WAVEOUT:
		  release_waveout(handle);
		  break;
		case LA_NULL:
		  release_null(handle);
		  break;
		case LA_SDL:
		  release_sdl(handle);
		  break;
		case LA_EMU10K1:
		  release_emu10k1(handle);
		  break;
		default:
		  fprintf(stderr,
		  	"release_audiodevices stubbed for 0x%x\n",
			hardware_type);
		  break;
	}

	return retval;
}

float get_audiochannel(void *handle, ALuint channel) {
	float lr_vol = 0;

	switch(hardware_type) {
	    case LA_NATIVE:
	    	lr_vol = get_nativechannel(handle, channel);
		break;
	    default:
	        fprintf(stderr,
			"get_audiochannel not implemented for type 0x%x\n",
			hardware_type);
		break;
	}

	return lr_vol;
}

/* this is a kludge and should be removed or changed */
void set_audiochannel(void *handle, ALuint channel, float volume) {
	switch(hardware_type) {
	    case LA_NATIVE:
	        set_nativechannel(handle, channel, volume);
		break;
	    default:
	        fprintf(stderr,
			"set_audiochannel not implemented for type 0x%x\n",
			hardware_type);
		break;
	}

	return;
}

/* inform device specified by handle that it's about to get paused */
void pause_audiodevice(void *handle) {
	switch(hardware_type) {
	    case LA_NATIVE:
	        pause_nativedevice(handle);
		break;
	    case LA_ESD:
	        pause_esd(handle);
	  	break;
	    default:
	    	fprintf(stderr,
		"pause_audiodevice stubbed for 0x%x\n",
		hardware_type);
		break;
	}

	return;
}

/* inform device specified by handle that it's about to get paused */
void resume_audiodevice(void *handle) {
	switch(hardware_type) {
	    case LA_NATIVE:
	        resume_nativedevice(handle);
		break;
	    case LA_ESD:
	        resume_esd(handle);
	  	break;
	    default:
	    	fprintf(stderr,
		"resume_audiodevice stubbed for 0x%x\n",
		hardware_type);
		break;
	}

	return;
}

/* capture data from the audio device */
ALsizei capture_audiodevice(void *handle, void *capture_buffer, int bufsiz) {
	ALsizei bytes = 0;
	switch(hardware_type) {
		case LA_NATIVE:
		  bytes = capture_nativedevice(handle, capture_buffer, bufsiz);
		  break;
		case LA_EMU10K1:
		  bytes = capture_emu10k1(handle, capture_buffer, bufsiz);
		  break;
		case LA_ALSA:
		case LA_SDL:
		case LA_ARTS:
		case LA_ESD:
		case LA_WAVEOUT:
		case LA_NULL:
		case LA_NONE:
		default:
		  memset(capture_buffer, 0, bufsiz);

		  fprintf(stderr,
			"openal: capture_audiodevice unimplemented for 0x%x\n",
			hardware_type);
		  break;
	}

	return bytes;
}

/*
 *  Returns smallest power of two that will meet or excees spotee.
 */
ALuint _alSpot( ALuint spotee ) {
	unsigned int retval = 0;

	spotee >>= 1;

	while(spotee) {
		retval++;

		spotee >>= 1;
	}

	return retval;
}

ALboolean set_write_audiodevice(void *handle,
			  ALuint *bufsiz,
			  ALenum *fmt,
			  ALuint *speed) {
	switch(hardware_type) {
		case LA_NATIVE:
		  return set_write_native(handle, bufsiz, fmt, speed);
		case LA_ALSA:
		  return set_write_alsa(handle, bufsiz, fmt, speed);
		case LA_SDL:
		  return set_write_sdl(handle, bufsiz, fmt, speed);
		case LA_ARTS:
		  return set_write_arts(handle, bufsiz, fmt, speed);
		case LA_ESD:
		  return set_write_esd(handle, bufsiz, fmt, speed);
		case LA_WAVEOUT:
		  return set_write_waveout(handle, bufsiz, fmt, speed);
		case LA_NULL:
		  return set_write_null(handle, bufsiz, fmt, speed);
		case LA_EMU10K1:
		  return set_write_emu10k1(handle, bufsiz, fmt, speed);
		case LA_NONE:
		default:
		  fprintf(stderr,
			"openal: set_audiodevice failed "
			"because no audio device has been opened.\n");
		  return AL_FALSE;
		  break;
	}

	return AL_FALSE;
}

ALboolean set_read_audiodevice(void *handle,
			  ALuint *bufsiz,
			  ALenum *fmt,
			  ALuint *speed) {
	switch(hardware_type) {
		case LA_NATIVE:
		  return set_read_native(handle, bufsiz, fmt, speed);
		case LA_ALSA:
		  return set_read_alsa(handle, bufsiz, fmt, speed);
		case LA_SDL:
		  return set_read_sdl(handle, bufsiz, fmt, speed);
		case LA_ARTS:
		  return set_read_arts(handle, bufsiz, fmt, speed);
		case LA_ESD:
		  return set_read_esd(handle, bufsiz, fmt, speed);
		case LA_WAVEOUT:
		  return set_read_waveout(handle, bufsiz, fmt, speed);
		case LA_NULL:
		  return set_read_null(handle, bufsiz, fmt, speed);
		case LA_EMU10K1:
		  return set_read_emu10k1(handle, bufsiz, fmt, speed);
		case LA_NONE:
		default:
		  fprintf(stderr,
			"openal: set_audiodevice failed "
			"because no audio device has been opened.\n");
		  return AL_FALSE;
		  break;
	}

	return AL_FALSE;
}
