/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * alut_msadpcm.c
 *
 * Loki's alut MS-ADPCM loader.
 *
 * FIXME: this is such a mess.
 *
 */
#include "al_siteconfig.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include "audioconvert.h"

#include "al_siteconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "al_main.h"
#include "al_debug.h"
#include "al_buffer.h"

#define MAX_ADPCM     1024
#define MS_ADPCM_MULT 1

/* data structure for wrapping buffers */
typedef struct _ringbuffer {
	ALubyte *data;
	ALuint bytesLeft;
	ALint  readByteOffset;
	ALuint blockSampSize;
} ringbuffer;

/* maximum MAX_ADPCM simultaneous sid/offset */
static struct {
	ALuint bid;
	ALint size;
	alMSADPCM_state_LOKI mss;
	void *data;
} mbidmap[MAX_ADPCM];

static struct {
	ALint sid;
	ALint offset;
	alMSADPCM_state_LOKI state;
	ringbuffer rb;
	ALint flen;
} msmap[MAX_ADPCM];

static ALboolean MS_first_time  = AL_TRUE;

static int MS_ADPCM_Callback(ALuint sid, ALuint bid,
			     ALshort *outdata,
			     ALenum format, ALint freq, ALint size);

static int  mbidmap_get(ALuint bid,
		void **data, ALint *size, alMSADPCM_state_LOKI *spec);
static int  mbidmap_insert(ALuint bid,
		void *data, int size, alMSADPCM_state_LOKI *spec);
static void mbidmap_remove(ALuint bid);

static void msmap_init(void);
static int  msmap_get(ALuint sid, ALint *offset,
			alMSADPCM_state_LOKI *state, ringbuffer *rb,
			ALuint *bytesleft);
static int  msmap_insert(ALuint sid, ringbuffer *rb, ALint bytesleft);
static void msmap_update(int i, ALint offset,
		alMSADPCM_state_LOKI state, ringbuffer *rb, ALint bytesleft);
static void msmap_remove(ALuint sid);


static int mbidmap_get(ALuint bid,
		ALvoid **data, ALint *size,
		alMSADPCM_state_LOKI *spec) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(mbidmap[i].bid == bid) {
			*size = mbidmap[i].size;
			*data = mbidmap[i].data;
			*spec = mbidmap[i].mss;

			return i;
		}
	}

	return -1;
}

static int mbidmap_insert(ALuint bid,
		ALvoid *data, ALint size, alMSADPCM_state_LOKI *spec) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(mbidmap[i].bid == bid) {
			/* old copy */
			if(mbidmap[i].data != NULL) {
				free(mbidmap[i].data);
			}
			mbidmap[i].bid = -1; /* flag for next */
		}

		if(mbidmap[i].bid == (ALuint) -1) {
			mbidmap[i].bid  = bid;
			mbidmap[i].size = size;
			mbidmap[i].data = data;

			mbidmap[i].mss = *spec;

			return i;
		}
	}

	return -1;
}

static void mbidmap_remove(ALuint bid) {
	int i = 0;

	_alDebug(ALD_CONVERT, __FILE__, __LINE__,
		"mbidmap_remove(%d)", bid);

	for(i = 0; i < MAX_ADPCM; i++) {
		if(mbidmap[i].bid == bid) {
			mbidmap[i].bid = -1;

			if(mbidmap[i].data != NULL) {

				free(mbidmap[i].data);
				mbidmap[i].data = NULL;
			}
			return;
		}
	}

	return;
}

/* ms adpcm stuff */
ALboolean alutLoadMS_ADPCMData_LOKI(ALuint bid, void *data, int size,
			alMSADPCM_state_LOKI *mss) {
	ALvoid *newdata;

	if(MS_first_time == AL_TRUE) {
		/* so kludgey */
		msmap_init();
		MS_first_time = AL_FALSE;
	}

	newdata = malloc(size);
	if(newdata == NULL) {
		return AL_FALSE;
	}
	memcpy(newdata, data, size);

	mbidmap_insert(bid, newdata, 2 * size, mss);

	_alBufferDataWithCallback_LOKI(bid,
				MS_ADPCM_Callback,
				msmap_remove,
				mbidmap_remove);

	return AL_TRUE;
}


static void msmap_init(void) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		mbidmap[i].bid   = (ALuint) -1;

		msmap[i].sid   = -1;
		msmap[i].offset = 0;
	}

	return;
}

/* FIXME: make binary search */
static int msmap_get(ALuint sid, ALint *offset,
			alMSADPCM_state_LOKI *state, ringbuffer *rb,
			ALuint *flen) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(msmap[i].sid == (ALint) sid) {
			*offset = msmap[i].offset;
			*state  = msmap[i].state;
			*rb     = msmap[i].rb;
			*flen   = msmap[i].flen;

			return i;
		}
	}

	return -1;
}

/* FIXME: sorted insert for binary search */
static int msmap_insert(ALuint sid, ringbuffer *rb, ALint flen) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if((msmap[i].sid == -1) || (msmap[i].sid == (ALint) sid)) {
			msmap[i].sid    = sid;
			msmap[i].offset = 0;
			msmap[i].rb     = *rb;
			msmap[i].flen   = flen;
			return i;
		}
	}

	return -1;
}

static void msmap_update(int i, ALint offset,
	alMSADPCM_state_LOKI state, ringbuffer *rb,
	ALint flen) {

	if(i >= MAX_ADPCM) {
		return;
	}

	msmap[i].offset        = offset;
	msmap[i].state         = state;
	msmap[i].rb            = *rb;
	msmap[i].flen          = flen;

	return;
}

static void msmap_remove(ALuint sid) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(msmap[i].sid == (ALint) sid) {
			msmap[i].sid           = -1;
			msmap[i].offset        = 0;

			if(msmap[i].rb.data != NULL) {
				free(msmap[i].rb.data);
				msmap[i].rb.data = NULL;
			}
			return;
		}
	}

	return;
}

static int MS_ADPCM_Callback(ALuint sid, ALuint bid,
			ALshort *outdata,
			ALenum format, ALint freq, ALint size) {
	ALboolean needToConvert = AL_FALSE;
	alMSADPCM_state_LOKI state, origstate;
	int offset;
	int i;
	ALboolean should_remove = AL_FALSE;
	ALvoid *indata;
	ringbuffer rb;
	ALuint sampsToRead = 0;
	ALuint bytesToRead = 0;
	ALuint filebytesleft = 0; /* from freq, from len */
	int origflen;
	ALfloat ratio = 1.0;

	/* get buffer specific information */
	i = mbidmap_get(bid, &indata, &origflen, &origstate);
	if(i == -1) {
		return -1; /* weird */
	}

	/* get source specific information */
	i = msmap_get(sid, &offset, &state, &rb, &filebytesleft);
	if(i == -1) {
		state = origstate;

		rb.blockSampSize = state.wavefmt.blockalign
				 * state.wSamplesPerBlock;
		rb.data = malloc(rb.blockSampSize);
		rb.bytesLeft      = 0;
		rb.readByteOffset = 0;

		i = msmap_insert(sid, &rb, origflen);

		filebytesleft = origflen;
		offset = 0;
	}

	/*
	 * Here, we check the disparity between frequency, channels
	 * and bit size between what's being requested and what's
	 * being doled out.
	 *
	 * Any difference, and we resample in place.
	 */
	if((ALuint) freq != state.wavefmt.frequency) {
		ratio *= (ALfloat) state.wavefmt.frequency / freq;
		needToConvert = AL_TRUE;
	}
	if((ALuint) _al_ALCHANNELS(format) != state.wavefmt.channels) {
		ratio *= (ALfloat) state.wavefmt.channels / _al_ALCHANNELS(format);
		needToConvert = AL_TRUE;
	}

	/* scale size by sample different */
	sampsToRead = size * ratio;
	bytesToRead = sizeof(ALshort) * sampsToRead;

	/* clamp length to end */
	if(filebytesleft <= sampsToRead) {
		should_remove = AL_TRUE;

		/* do something */
		bytesToRead = filebytesleft;
		sampsToRead = bytesToRead/sizeof(ALshort);
	} 

	bytesToRead = sampsToRead * sizeof(ALshort);

	if((rb.bytesLeft < bytesToRead) && (rb.bytesLeft != 0)) {
		memcpy(outdata, rb.data + rb.readByteOffset, rb.bytesLeft);

		bytesToRead -= rb.bytesLeft;
		sampsToRead  = bytesToRead  / sizeof(ALshort);
		outdata     += rb.bytesLeft / sizeof(ALshort);

		rb.bytesLeft  = 0;
	}

	if(rb.bytesLeft <= 0) {
		int copylen;
		
		if(filebytesleft >= sizeof(ALshort) * rb.blockSampSize) {
			copylen = rb.blockSampSize;
		} else {
			copylen = filebytesleft / sizeof(ALshort);
		}

		msadpcm_decode(indata,
				rb.data,
				copylen,
				&state,
				offset);

		offset            += copylen;
		rb.bytesLeft       = copylen * 2 * sizeof(ALshort);
		rb.readByteOffset  = 0;
	}

	memcpy(outdata, rb.data + rb.readByteOffset, bytesToRead);

	rb.bytesLeft      -= bytesToRead;
	rb.readByteOffset += bytesToRead;
	filebytesleft     -= sampsToRead;

	if(should_remove == AL_TRUE) {
		rb.readByteOffset = 0;
		rb.bytesLeft   = rb.blockSampSize;

		/* 
		 * looping happens at offset 0.
		 *
		 * Sorry, that's the way it has to be until I 
		 * rewrite msadpcm_decode
		 */
		msmap_update(i, 0, state, &rb, origflen);
	} else {
		msmap_update(i, offset, state, &rb, filebytesleft);
	}

	/*
	 * If we have a freq / chan / bitsize mismatch, convert
	 */
	if(needToConvert == AL_TRUE) {
		acAudioCVT cvt;

		if(acBuildAudioCVT(&cvt,
				/* from */
				/* FIXME: always little endian ? */
				AUDIO_S16LSB,
				state.wavefmt.channels,
				state.wavefmt.frequency,

				/* to */
				_al_AL2ACFMT(format),
				_al_ALCHANNELS(format),
				freq) < 0) {
			/* could not convert */
			return 0;
		}

		cvt.len = bytesToRead;
		cvt.buf = outdata;

		acConvertAudio(&cvt);

		sampsToRead = cvt.len_cvt / sizeof *outdata;
	}

	return sampsToRead;
}
