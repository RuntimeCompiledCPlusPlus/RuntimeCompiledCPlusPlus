/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * alut_rawadpcm.c
 *
 * Loki's alut raw adpcm loader.
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

/* maximum MAX_ADPCM simultaneous sid/offset */
static struct {
	ALint bid;
	ALint size;
	alWaveFMT_LOKI wfx;
	void *data;
} bidmap[MAX_ADPCM];

/* maximum MAX_ADPCM simultaneous sid/offset */
static struct {
	ALint sid;
	ALint offset;
	adpcm_state_t state;
} admap[MAX_ADPCM];

static ALboolean RAW_first_time = AL_TRUE;

static int RAW_ADPCM_Callback(ALuint sid, ALuint bid,
			     ALshort *outdata,
			     ALenum format, ALint freq, ALint size);
static void RAW_ADPCM_DestroyCallback_Sid(ALuint sid);
static void RAW_ADPCM_DestroyCallback_Bid(ALuint bid);

static int  bidmap_get(ALuint bid, void **data, ALint *size, void *spec);
static int  bidmap_insert(ALuint bid, void *data, int size, void *spec);
static void bidmap_remove(ALuint bid);

static int  admap_get(ALuint sid, ALint *offset, adpcm_state_t *state);
static int  admap_insert(ALuint sid);
static void admap_update(int i, ALint offset, adpcm_state_t state);
static void admap_remove(ALuint sid);

ALboolean alutLoadRAW_ADPCMData_LOKI(ALuint bid,
				ALvoid *data, ALuint size, ALuint freq,
				ALenum format) {
	alWaveFMT_LOKI wfx;
	int i;
	ALvoid *persistent_data;

	if(RAW_first_time == AL_TRUE) {
		/* so kludgey */
		for(i = 0; i < MAX_ADPCM; i++) {
			bidmap[i].bid  = -1;
			bidmap[i].data = NULL;
			admap[i].sid   = -1;
		}
		RAW_first_time = AL_FALSE;
	}
	
	persistent_data = malloc(size);
	if(persistent_data == NULL) {
		return AL_FALSE;
	}

	memcpy(persistent_data, data, size);

	wfx.frequency = freq;
	wfx.channels = _al_ALCHANNELS(format);
	wfx.bitspersample = _al_formatbits(format);

	/* insert new bid, ignore blockalign */
	bidmap_insert(bid, persistent_data, size, &wfx);

	_alBufferDataWithCallback_LOKI(bid, RAW_ADPCM_Callback,
				RAW_ADPCM_DestroyCallback_Sid,
				RAW_ADPCM_DestroyCallback_Bid);

	return AL_TRUE;
}

static int RAW_ADPCM_Callback(ALuint sid, ALuint bid, ALshort *outdata,
			UNUSED(ALenum format), UNUSED(ALint freq), ALint size) {
	adpcm_state_t state;
	int offset;
	int i;
	int ffreq, flen; /* from freq, from len */
	ALboolean should_remove = AL_FALSE;
	ALvoid *indata;
	alWaveFMT_LOKI wfx;

	/* get buffer specific information */
	if((i = bidmap_get(bid, &indata, &flen, &wfx) == -1)) {
		return -1; /* weird */
	}

	ffreq = wfx.frequency;

	/* get source specific information */
	i = admap_get(sid, &offset, &state);
	if(i == -1) {
		i = admap_insert(sid);

		offset = 0;
		state.valprev = 0;
		state.index = 0;
	}

	if(2 * flen - offset < size) {
		should_remove = AL_TRUE;
		size = 2 * flen - offset;
	}

	/* main mojo */
	ac_adpcm_decoder(indata, outdata, size, &state, offset);

	if(should_remove == AL_TRUE) {
		state.valprev = 0;
		state.index   = 0;

		/* place at correct interval in case we're looping */
		admap_update(i, offset + size - 2 * flen, state);

		return size / sizeof *outdata;
	} else {
		/* i is set to our index into the adpcm map */
		admap_update(i, offset + size, state);
	}

	return size;
}

static void RAW_ADPCM_DestroyCallback_Sid(ALuint sid) {
	admap_remove(sid);
}

static void RAW_ADPCM_DestroyCallback_Bid(ALuint bid) {
	bidmap_remove(bid);
}

/* FIXME: make binary search */
static int admap_get(ALuint sid, ALint *offset, adpcm_state_t *state) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(admap[i].sid == (ALint) sid) {
			*offset = admap[i].offset;
			*state  = admap[i].state;

			return i;
		}
	}

	return -1;
}

/* FIXME: sorted insert for binary search */
static int admap_insert(ALuint sid) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if((admap[i].sid == -1) || (admap[i].sid == (ALint) sid)) {
			admap[i].sid = sid;
			admap[i].offset = 0;
			admap[i].state.valprev = 0;
			admap[i].state.index   = 0;
			
			return i;
		}
	}

	return -1;
}

static void admap_update(int i, ALint offset, adpcm_state_t state) {
	if(i >= MAX_ADPCM) {
		return;
	}

	admap[i].offset        = offset;
	admap[i].state.valprev = state.valprev;
	admap[i].state.index   = state.index;

	return;
}

static int bidmap_get(ALuint bid, ALvoid **data,
			ALint *size, void *spec) {
	alWaveFMT_LOKI *wfx;
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(bidmap[i].bid == (ALint) bid) {
			*size = bidmap[i].size;
			*data = bidmap[i].data;

			wfx = (alWaveFMT_LOKI *) spec;
			*wfx  = bidmap[i].wfx;

			return i;
		}
	}

	return -1;
}

static int bidmap_insert(ALuint bid, ALvoid *data, ALint size, void *spec) {
	alWaveFMT_LOKI *wfx;
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(bidmap[i].bid == (ALint ) bid) {
			/* old copy */
			if(bidmap[i].data != NULL) {
				free(bidmap[i].data);
			}
			bidmap[i].bid = -1; /* flag for next */
		}

		if(bidmap[i].bid == -1) {
			bidmap[i].bid  = bid;
			bidmap[i].size = size;
			bidmap[i].data = data;

			wfx = (alWaveFMT_LOKI *) spec;
			bidmap[i].wfx  = *wfx;

			return i;
		}
	}

	return -1;
}

static void bidmap_remove(ALuint bid) {
	int i = 0;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(bidmap[i].bid == (ALint) bid) {
			bidmap[i].bid = -1;
			if(bidmap[i].data != NULL) {
				free(bidmap[i].data);
				bidmap[i].data = NULL;
			}
			return;
		}
	}

	return;
}

static void admap_remove(ALuint sid) {
	int i;

	for(i = 0; i < MAX_ADPCM; i++) {
		if(admap[i].sid == (ALint) sid) {
			admap[i].sid           = -1;
			admap[i].offset        = 0;
			admap[i].state.valprev = 0;
			admap[i].state.index   = 0;
			
			return;
		}
	}

	return;
}
