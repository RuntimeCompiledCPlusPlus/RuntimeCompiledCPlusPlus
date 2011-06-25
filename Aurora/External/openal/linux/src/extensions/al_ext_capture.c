/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_ext_capture.c
 *
 * audio recording extension
 *
 */
#include "al_siteconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AL/al.h"
#include "AL/alc.h"

#include "al_ext_needed.h"
#include "al_ext_capture.h"

#include "al_buffer.h"
#include "al_error.h"
#include "al_debug.h"
#include "al_mixer.h"
#include "al_source.h"
#include "alc/alc_device.h"
#include "alc/alc_context.h"
#include "al_types.h"

#include <audioconvert.h>

#include "arch/interface/interface_sound.h"
#include "mutex/mutexlib.h"

#ifdef OPENAL_EXTENSION

/*
 * we are not being build into the library, therefore define the
 * table that informs openal how to register the extensions upon
 * dlopen.
 */
struct { ALubyte *name; void *addr; } alExtension_03222001 [] = {
	AL_EXT_PAIR(alCaptureInit_EXT),
	AL_EXT_PAIR(alCaptureStart_EXT),
	AL_EXT_PAIR(alCaptureStop_EXT),
	AL_EXT_PAIR(alCaptureGetData_EXT),
	AL_EXT_PAIR(alCaptureDestroy_EXT),
	{ NULL, NULL }
};

/*
 *  We don't need init or fini functions, but we might as well
 *  keep them in place if, in some distant future, they turn out
 *  to be useful.
 */
void alExtInit_03282000(void) {
	fprintf(stderr, "alExtInit_03282000 STUB\n");
	return;
}

void alExtFini_03282000(void) {
	fprintf(stderr, "alExtFini_03282000 STUB\n");
	return;
}
#endif /* OPENAL_EXTENSION */

void alInitCapture(void) {
	return;
}

void alFiniCapture(void) {
	return;
}

ALboolean alCaptureInit_EXT( UNUSED(ALenum format),
                             UNUSED(ALuint rate),
                             UNUSED(ALsizei bufferSize) )
{
	ALuint cid;
	AL_context *cc;
	AL_device *capture_device;

	/* get the current context */
	capture_device = NULL;
	cid = _alcCCId;
	_alcLockContext( cid );
	cc = _alcGetContext(cid);
	if ( cc != NULL ) {
		capture_device = cc->read_device;
		if ( capture_device == NULL ) {
			char spec[1024];
			char *fmt="'( (direction \"read\") (sampling-rate %d))";

			sprintf(spec, fmt, rate);
			capture_device = alcOpenDevice((ALubyte *)spec);
			if ( capture_device ) {
				_alcSetContext(NULL, cid, capture_device);
				_alcDeviceSet(capture_device);
			}
		}
	}
	_alcUnlockContext( cid );

	return (capture_device != NULL);
}

ALboolean alCaptureDestroy_EXT( ALvoid )
{
	ALuint cid;
	AL_context *cc;

	/* get the current context */
	cid = _alcCCId;
	_alcLockContext( cid );
	cc = _alcGetContext(cid);
	if ( cc == NULL ) {
		_alcUnlockContext( cid );
		return AL_FALSE;
	}

	if ( cc->read_device ) {
		/* Only close it if we opened it originally */
		if (cc->write_device && (cc->read_device != cc->write_device)) {
			alcCloseDevice(cc->read_device);
			cc->read_device = NULL;
		}
	}
	_alcUnlockContext( cid );

	return AL_TRUE;
}

ALboolean alCaptureStart_EXT( ALvoid )
{
	return AL_FALSE;
}

ALboolean alCaptureStop_EXT( ALvoid )
{
	return AL_FALSE;
}

ALsizei alCaptureGetData_EXT( UNUSED(ALvoid* data),
                              UNUSED(ALsizei n),
                              UNUSED(ALenum format),
                              UNUSED(ALuint rate) )
{
	AL_device *dev;
	ALuint size;
	ALuint cid;
	AL_context *cc;

	/* get the read device */
	cid = _alcCCId;
	cc = _alcGetContext(cid);
	if ( cc == NULL ) {
		return 0;
	}
	dev = cc->read_device;

	if ( (dev->format == format) && (dev->speed == rate) ) {
		size = _alcDeviceRead(cid, data, n);
	} else {
		ALuint samples;
		void *temp;

		samples = n / (_al_formatbits(format) / 8);

		/* Set size to the bytes of raw audio data we need */
		size = _al_PCMRatioify(rate, dev->speed,
		                       format, dev->format, samples);
		size *= (_al_formatbits(dev->format) / 8);

        	if ( n > (ALsizei)size )
			temp = malloc( n );
		else
			temp = malloc( size );

		if ( size > 0 ) {
			size = _alcDeviceRead(cid, temp, size);

			temp = _alBufferCanonizeData(dev->format,
						     temp,
						     size,
						     dev->speed,
						     format,
						     rate,
						     &size,
						     AL_TRUE);
		} else {
			/* Hmm, zero size in record.. */
			memset(temp, 0, n);
			size = n;
		}
		if(temp == NULL) {
			fprintf(stderr, "could not canonize data\n");
			return 0;
		}

		memcpy(data, temp, size);

		free( temp );
	}
	return size;
}
