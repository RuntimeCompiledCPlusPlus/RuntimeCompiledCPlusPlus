/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * alut_wav.c
 *
 * Loki's wave file loader.
 *
 * FIXME: error handling?
 */
#include "al_siteconfig.h"

#include <AL/al.h>
#include <AL/alut.h>

#include "al_types.h"
#include "al_main.h"
#include "al_buffer.h"
#include "al_debug.h"
#include "alc/alc_context.h"

#include "audioconvert.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string.h>

static ALboolean ReadWAVFile(const char *fname, void **pcmdata,
			ALushort *rfmt, ALushort *rchan,
			ALushort *rfreq, ALuint *rsize);

ALboolean alutLoadWAV( const char *fname,
                        void **wave,
			ALsizei *format,
			ALsizei *size,
			ALsizei *bits,
			ALsizei *freq ) {
	ALushort alFmt  = 0;
	ALushort acChan = 0;
	ALushort acFreq = 0;
	ALuint   acSize = 0;

	if(ReadWAVFile(fname, wave,
			&alFmt, &acChan, &acFreq, &acSize) == AL_FALSE) {
		_alDebug(ALD_CONVERT, __FILE__, __LINE__,
			"ReadWAVFile failed for %s", fname);
		return AL_FALSE;
	}

	/* set output params */
	*format = (ALsizei) alFmt;
	*freq   = (ALsizei) acFreq;
	*size   = (ALsizei) acSize;
	*bits   = (ALsizei) _al_formatbits(alFmt);
	
	_alDebug(ALD_CONVERT, __FILE__, __LINE__,
		"alutLoadWAV %s with [alformat/size/bits/freq] = [0x%x/%d/%d]",
		fname,
		*format, *size, *freq);

	return AL_TRUE;
}

static ALboolean ReadWAVFile(const char *fname, void **pcmdata,
			ALushort *rfmt, ALushort *rchan, ALushort *rfreq,
			ALuint *rsize) {
	void *data;
	ALint len;  /* length of pcm portion */
	void *udata;

	if((rfmt == NULL) || (rchan == NULL) || (rfreq == NULL)) {
		/* bad mojo */
		return AL_FALSE;
	}

	len = _alSlurp(fname, &data);
	if(len < 0) {
		/* couldn't slurp audio file */
		_alDebug(ALD_CONVERT, __FILE__, __LINE__,
			"Could not slurp %s", fname);

		return AL_FALSE;
	}

	if(acLoadWAV(data, (ALuint *) &len, &udata,
		rfmt, rchan, rfreq) == NULL) {
		_alDebug(ALD_CONVERT, __FILE__, __LINE__,
			"Could not buffer and convert data");

		free(data);
		return AL_FALSE;
	}

	free(data);

	*rfmt    = _al_AC2ALFMT(*rfmt, *rchan);
	*rsize   = len;
	*pcmdata = udata;

	_alDebug(ALD_CONVERT, __FILE__, __LINE__,
		"ReadWAVFile [freq/size/acformat] = [%d/%d/0x%x]",
		*rfreq, *rsize, *rfmt);

	return AL_TRUE;
}
