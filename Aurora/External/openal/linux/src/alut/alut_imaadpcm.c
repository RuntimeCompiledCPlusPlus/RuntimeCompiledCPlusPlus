/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * alut_imaadpcm.c
 *
 * Loki's alut IMA-ADPCM loader.
 *
 * FIXME: this is such a mess.
 */

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include "al_siteconfig.h"
#include "al_main.h"
#include "al_debug.h"
#include "al_buffer.h"
#include "audioconvert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ADPCM      1024
#define IMA_ADPCM_MULT 8

/* data structure for wrapping buffers */
typedef struct _ringbuffer {
	ALubyte *data;
	ALuint bytesLeft;
	ALint  readByteOffset;
	ALuint blockSampSize;
} ringbuffer;

/* maximum MAX_ADPCM simultaneous sid/offset */
static struct {
	ALint bid;
	ALint size;
	alIMAADPCM_state_LOKI ias;
	void *data;
} ibidmap[MAX_ADPCM];

/* maximum MAX_ADPCM simultaneous sid/offset */
static struct {
	ALint sid;
	ALint offset;
	alIMAADPCM_state_LOKI state;
	ringbuffer rb;
	ALuint flen;
} ismap[MAX_ADPCM];

/*
 * IMA_first_time is set to AL_TRUE initialy, and after the first invocation
 * should be set to AL_FALSE.  It is used to key initialization stuff.
 *
 * FIXME:
 *    should convert this to an extension and use the extension
 *    initialization routines instead.
 */
static ALboolean IMA_first_time  = AL_TRUE;

/*
 * IMA_ADPCM_Callback( ALuint sid, ALuint bid,
 *                     ALshort *outdata,
 *                     ALenum format, ALint freq, ALint size );
 *
 * Callback to decode IMA_ADPCM data.
 */
static int IMA_ADPCM_Callback( ALuint sid, ALuint bid,
			       ALshort *outdata,
			       ALenum format, ALint freq, ALint size );

/*
 * ibidmap_get( ALuint bid, void **data, ALuint *size,
 *              alIMAADPCM_state_LOKI *spec )
 */
static int ibidmap_get( ALuint bid, void **data, ALuint *size,
			alIMAADPCM_state_LOKI *spec );
static int ibidmap_insert(ALuint bid, void *data, int size,
			alIMAADPCM_state_LOKI *spec);
static void ibidmap_remove(ALuint bid);

static void ismap_init(void);
static int  ismap_insert(ALuint sid, ringbuffer *rb, ALuint *bytesleft);
static int  ismap_get(ALuint sid, ALint *offset,
			alIMAADPCM_state_LOKI *state, ringbuffer *rb,
			ALuint *bitestoread);
static void ismap_update(int i, ALint offset,
			alIMAADPCM_state_LOKI state, ringbuffer *rb,
			ALuint *flen);
static void ismap_remove(ALuint sid);

static int ibidmap_get(ALuint bid, ALvoid **data,
			ALuint *size, alIMAADPCM_state_LOKI *spec) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(ibidmap[i].bid == (ALint) bid) {
			*size = ibidmap[i].size;
			*data = ibidmap[i].data;
			*spec = ibidmap[i].ias;

			return i;
		}
	}

	return -1;
}

static int ibidmap_insert(ALuint bid, ALvoid *data,
			ALint size, alIMAADPCM_state_LOKI *spec) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(ibidmap[i].bid == (ALint) bid) {
			/* old copy */
			free( ibidmap[i].data );
			ibidmap[i].bid = -1; /* flag for next */
		}

		if(ibidmap[i].bid == -1) {
			ibidmap[i].bid  = bid;
			ibidmap[i].size = size;
			ibidmap[i].data = data;

			ibidmap[i].ias = *spec;

			return i;
		}
	}

	return -1;
}

static void ibidmap_remove(ALuint bid) {
	int i = 0;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(ibidmap[i].bid == (ALint) bid) {
			ibidmap[i].bid = -1;
			if(ibidmap[i].data != NULL) {
				free(ibidmap[i].data);
				ibidmap[i].data = NULL;
			}
			return;
		}
	}

	return;
}

ALboolean alutLoadIMA_ADPCMData_LOKI(ALuint bid,
				ALvoid *data, ALuint size,
				alIMAADPCM_state_LOKI *ias) {
	ALvoid *newdata;

	if(IMA_first_time == AL_TRUE) {
		/* so kludgey */
		ismap_init();
		IMA_first_time = AL_FALSE;
	}

	newdata = malloc(size);
	if(newdata == NULL) {
		_alDebug(ALD_CONVERT,
			__FILE__, __LINE__,
			"MALLOC ERROR\n");
		return AL_FALSE;
	}
	memcpy(newdata, data, size);

	ibidmap_insert(bid, newdata, 2 * size, ias);

	_alBufferDataWithCallback_LOKI(bid,
				IMA_ADPCM_Callback,
				ismap_remove,
				ibidmap_remove);

	return AL_TRUE;
}

static void ismap_init(void) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		ibidmap[i].bid = -1;
		ismap[i].sid   = -1;
		ismap[i].offset = 0;
	}

	return;
}

/* FIXME: make binary search */
static int ismap_get(ALuint sid, ALint *offset,
			alIMAADPCM_state_LOKI *state, ringbuffer *rb,
			ALuint *bytestoread) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(ismap[i].sid == (ALint) sid) {
			*offset      = ismap[i].offset;
			*state       = ismap[i].state;
			*rb          = ismap[i].rb;
			*bytestoread = ismap[i].flen;

			return i;
		}
	}

	return -1;
}

/* FIXME: sorted insert for binary search */
static int ismap_insert(ALuint sid, ringbuffer *rb, ALuint *bytesleft) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if((ismap[i].sid == -1) || (ismap[i].sid == (ALint) sid)) {
			ismap[i].sid    = sid;
			ismap[i].offset = 0;
			ismap[i].rb     = *rb;
			ismap[i].flen   = *bytesleft;
			return i;
		}
	}

	return -1;
}

static void ismap_update(int i, ALint offset,
	alIMAADPCM_state_LOKI state, ringbuffer *rb,
	ALuint *flen) {

	if(i >= MAX_ADPCM) {
		return;
	}

	ismap[i].offset        = offset;
	ismap[i].state         = state;
	ismap[i].rb            = *rb;
	ismap[i].flen          = *flen;

	return;
}

static void ismap_remove(ALuint sid) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(ismap[i].sid == (ALint) sid) {
			ismap[i].sid           = -1;
			ismap[i].offset        = 0;

			if(ismap[i].rb.data) {
				free(ismap[i].rb.data);

				ismap[i].rb.data = NULL;
			}
			return;
		}
	}

	return;
}

static int IMA_ADPCM_Callback(ALuint sid, ALuint bid,
			ALshort *outdata,
			ALenum format, ALint freq, ALint size) {
	alIMAADPCM_state_LOKI original_state;
	alIMAADPCM_state_LOKI state;
	int offset;
	int i;
	ALuint flen; /* from freq, from len */
	ALboolean should_remove = AL_FALSE;
	ALvoid *indata;
	ringbuffer rb;
	unsigned int sampsToRead;
	unsigned int bytesToRead;
	ALuint filebytesleft;

	/* get buffer specific information */
	i = ibidmap_get(bid, &indata, &flen, &original_state);
	if(i == -1) {
		_alDebug(ALD_CONVERT, __FILE__, __LINE__,
			"Weird return with bid == %d\n", bid);
		return -1; /* weird */
	}

	/* get source specific information */
	i = ismap_get(sid, &offset, &state, &rb, &filebytesleft);
	if(i == -1) {
		state = original_state;

		rb.blockSampSize = IMA_ADPCM_MULT *
			state.wavefmt.blockalign *
			state.wSamplesPerBlock;

		rb.data          = malloc(rb.blockSampSize * sizeof(ALshort));
		rb.bytesLeft      = 0;
		rb.readByteOffset = 0;
		filebytesleft     = flen;

		i = ismap_insert(sid, &rb, &filebytesleft);
		offset = 0;
	}

	/* scale size by sample different */
	sampsToRead = size;
	bytesToRead = 2 * size;

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

		IMA_ADPCM_decode(indata,
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

	/* in place re-frequencyification */
	if((ALuint) freq != state.wavefmt.frequency) {
		_alDebug(ALD_CONVERT, __FILE__, __LINE__,
			"freq weird %d vs %d",
			freq,
			state.wavefmt.frequency);
	}
	if((ALuint) _al_ALCHANNELS(format) != state.wavefmt.channels) {
		_alDebug(ALD_CONVERT, __FILE__, __LINE__,
			"chan weird %d vs %d",
			_al_ALCHANNELS(format),
			state.wavefmt.channels);
	}


	if(should_remove == AL_TRUE) {
		rb.readByteOffset = 0;
		rb.bytesLeft   = rb.blockSampSize;

		/* 
		 * looping happens at offset 0.
		 *
		 * Sorry, that's the way it has to be until I 
		 * rewrite msadpcm_decode
		 */
		ismap_update(i, 0, state, &rb, &flen);
	} else {
		ismap_update(i, offset, state, &rb, &filebytesleft);
	}

	return sampsToRead;
}
