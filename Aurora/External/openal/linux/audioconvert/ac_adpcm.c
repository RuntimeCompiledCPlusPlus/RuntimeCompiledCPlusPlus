/***********************************************************
 -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:

Copyright 1992 by Stichting Mathematisch Centrum, Amsterdam, The
Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.

STICHTING MATHEMATISCH CENTRUM DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH CENTRUM BE LIABLE
FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/*
** Intel/DVI ADPCM coder/decoder.
**
** The algorithm for this coder was taken from the IMA Compatability Project
** proceedings, Vol 2, Number 2; May 1992.
**
** Version 1.2, 18-Dec-92.
**
** Change log:
** - Fixed a stupid bug, where the delta was computed as
**   stepsize*code/4 in stead of stepsize*(code+0.5)/4.
** - There was an off-by-one error causing it to pick
**   an incorrect delta once in a blue moon.
** - The NODIVMUL define has been removed. Computations are now always done
**   using shifts, adds and subtracts. It turned out that, because the standard
**   is defined using shift/add/subtract, you needed bits of fixup code
**   (because the div/mul simulation using shift/add/sub made some rounding
**   errors that real div/mul don't make) and all together the resultant code
**   ran slower than just using the shifts all the time.
** - Changed some of the variable names to be more meaningful.
*/

#include "al_siteconfig.h"
#include "ac_adpcm.h"
#include "ac_endian.h"
#include "ac_wave.h"

#include <stdio.h>       /*DBG*/
#include <stdlib.h>

#include "AL/altypes.h"

#define MS_ADPCM_max ((1<<(16-1))-1)
#define MS_ADPCM_min -(1<<(16-1))

#define NELEMS(x) ((sizeof x) / (sizeof *x))

/* IMA in-wave-file */
static ALint IMA_ADPCM_nibble(alIMAADPCM_decodestate_LOKI *state, ALubyte nybble);
static void Fill_IMA_ADPCM_block(ALubyte *decoded, ALubyte *encoded,
	int channel, int numchannels, alIMAADPCM_decodestate_LOKI *state);

/* Intel ADPCM step variation table */
static int indexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

static int stepsizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};
    
void
ac_adpcm_coder(short indata[], char outdata[], int len,adpcm_state_t *state) {

    short *inp;			/* Input buffer pointer */
    signed char *outp;		/* output buffer pointer */
    int val;			/* Current input sample value */
    int sign;			/* Current adpcm sign bit */
    int delta;			/* Current adpcm output value */
    int diff;			/* Difference between val and valprev */
    int step;			/* Stepsize */
    int valpred;		/* Predicted output value */
    int vpdiff;			/* Current change to valpred */
    int index;			/* Current step change index */
    int outputbuffer = 0;	/* place to keep previous 4-bit value */
    int bufferstep;		/* toggle between outputbuffer/output */

    outp = (signed char *)outdata;
    inp = indata;

    valpred = state->valprev;
    index = state->index;
    step = stepsizeTable[index];

    bufferstep = 1;

    for ( ; len > 0 ; len-- ) {
	val = *inp++;

	/* Step 1 - compute difference with previous value */
	diff = val - valpred;
	sign = (diff < 0) ? 8 : 0;
	if ( sign ) diff = (-diff);

	/* Step 2 - Divide and clamp */
	/* Note:
	** This code *approximately* computes:
	**    delta = diff*4/step;
	**    vpdiff = (delta+0.5)*step/4;
	** but in shift step bits are dropped. The net result of this is
	** that even if you have fast mul/div hardware you cannot put it to
	** good use since the fixup would be too expensive.
	*/
	delta = 0;
	vpdiff = (step >> 3);
	
	if ( diff >= step ) {
	    delta = 4;
	    diff -= step;
	    vpdiff += step;
	}
	step >>= 1;
	if ( diff >= step  ) {
	    delta |= 2;
	    diff -= step;
	    vpdiff += step;
	}
	step >>= 1;
	if ( diff >= step ) {
	    delta |= 1;
	    vpdiff += step;
	}

	/* Step 3 - Update previous value */
	if ( sign )
	  valpred -= vpdiff;
	else
	  valpred += vpdiff;

	/* Step 4 - Clamp previous value to 16 bits */
	if ( valpred > 32767 )
	  valpred = 32767;
	else if ( valpred < -32768 )
	  valpred = -32768;

	/* Step 5 - Assemble value, update index and step values */
	delta |= sign;
	
	index += indexTable[delta];
	if ( index < 0 ) index = 0;
	if ( index > 88 ) index = 88;
	step = stepsizeTable[index];

	/* Step 6 - Output value */
	if ( bufferstep ) {
	    outputbuffer = (delta << 4) & 0xf0;
	} else {
	    *outp++ = (delta & 0x0f) | outputbuffer;
	}
	bufferstep = !bufferstep;
    }

    /* Output last step, if needed */
    if ( !bufferstep )
      *outp++ = outputbuffer;
    
    state->valprev = valpred;
    state->index = index;
}

void
ac_adpcm_decoder(char indata[], short outdata[], int len, adpcm_state_t *state,
int position)
{
    signed char *inp;		/* Input buffer pointer */
    short *outp;		/* output buffer pointer */
    int sign;			/* Current adpcm sign bit */
    int delta;			/* Current adpcm output value */
    int step;			/* Stepsize */
    int valpred;		/* Predicted value */
    int vpdiff;			/* Current change to valpred */
    int index;			/* Current step change index */
    int inputbuffer = 0;	/* place to keep next 4-bit value */
    int bufferstep;		/* toggle between inputbuffer/input */

    outp = outdata;
    inp = (signed char *)indata;

    valpred = state->valprev;
    index = state->index;
    step = stepsizeTable[index];

    inp += position>>1;
    bufferstep = position&1;
    if(bufferstep) {
	    inputbuffer = *inp++;
    }
    
    for ( ; len > 0 ; len-- ) {
	
	/* Step 1 - get the delta value */
	if ( bufferstep ) {
	    delta = inputbuffer & 0xf;
	} else {
	    inputbuffer = *inp++;
	    delta = (inputbuffer >> 4) & 0xf;
	}
	bufferstep = !bufferstep;

	/* Step 2 - Find new index value (for later) */
	index += indexTable[delta];
	if ( index < 0 ) index = 0;
	if ( index > 88 ) index = 88;

	/* Step 3 - Separate sign and magnitude */
	sign = delta & 8;
	delta = delta & 7;

	/* Step 4 - Compute difference and new predicted value */
	/*
	** Computes 'vpdiff = (delta+0.5)*step/4', but see comment
	** in adpcm_coder.
	*/
	vpdiff = step >> 3;
	if ( delta & 4 ) vpdiff += step;
	if ( delta & 2 ) vpdiff += step>>1;
	if ( delta & 1 ) vpdiff += step>>2;

	if ( sign )
	  valpred -= vpdiff;
	else
	  valpred += vpdiff;

	/* Step 5 - clamp output value */
	if ( valpred > 32767 )
	  valpred = 32767;
	else if ( valpred < -32768 )
	  valpred = -32768;

	/* Step 6 - Update step value */
	step = stepsizeTable[index];

	/* Step 7 - Output value */
	*outp++ = valpred;
    }

    state->valprev = valpred;
    state->index = index;
}

int msadpcm_decode(ALubyte *encoded, ALubyte *decoded, ALuint audio_len,
			   alMSADPCM_state_LOKI *dstate, int offset) {
	alMSADPCM_decodestate_LOKI *state[2];
	ALint encoded_len, samplesleft;
	ALbyte nybble, stereo;
	ALshort *coeff[2];
	ALint new_sample;

	/* scale encoded */
	encoded += (offset/4);

	/* Allocate the proper sized output buffer */
	encoded_len = audio_len /* 4:1 with 16bit samples */;

	/* Get ready... Go! */
	stereo = (dstate->wavefmt.channels == 2);
	state[0] = &dstate->state[0];
	state[1] = &dstate->state[(int) stereo];

	if(encoded_len < dstate->wavefmt.blockalign ) {
		fprintf(stderr, "too short\n");
	}

	while ( encoded_len >= dstate->wavefmt.blockalign ) {
		/* Grab the initial information for this block */
		state[0]->hPredictor = *encoded++;
		if ( stereo ) {
			state[1]->hPredictor = *encoded++;
		}
		state[0]->iDelta = ((encoded[1]<<8)|encoded[0]);
		encoded += sizeof(ALshort);
		if ( stereo ) {
			state[1]->iDelta = ((encoded[1]<<8)|encoded[0]);
			encoded += sizeof(ALshort);
		}
		state[0]->iSamp1 = ((encoded[1]<<8)|encoded[0]);
		encoded += sizeof(ALshort);
		if ( stereo ) {
			state[1]->iSamp1 = ((encoded[1]<<8)|encoded[0]);
			encoded += sizeof(ALshort);
		}
		state[0]->iSamp2 = ((encoded[1]<<8)|encoded[0]);
		encoded += sizeof(ALshort);
		if ( stereo ) {
			state[1]->iSamp2 = ((encoded[1]<<8)|encoded[0]);
			encoded += sizeof(ALshort);
		}
		coeff[0] = dstate->aCoeff[(int) state[0]->hPredictor];
		coeff[1] = dstate->aCoeff[(int) state[1]->hPredictor];

		/* Store the two initial samples we start with */
		decoded[0] = state[0]->iSamp2&0xFF;
		decoded[1] = state[0]->iSamp2>>8;
		decoded += 2;
		if ( stereo ) {
			decoded[0] = state[1]->iSamp2&0xFF;
			decoded[1] = state[1]->iSamp2>>8;
			decoded += 2;
		}
		decoded[0] = state[0]->iSamp1&0xFF;
		decoded[1] = state[0]->iSamp1>>8;
		decoded += 2;
		if ( stereo ) {
			decoded[0] = state[1]->iSamp1&0xFF;
			decoded[1] = state[1]->iSamp1>>8;
			decoded += 2;
		}

		/* Decode and store the other samples in this block */
		samplesleft = (dstate->wSamplesPerBlock-2)*
					dstate->wavefmt.channels;

		while ( samplesleft > 0 ) {
			nybble = (*encoded)>>4;
			new_sample = MS_ADPCM_nibble(state[0], nybble, coeff[0]);
			decoded[0] = new_sample & 0xFF;
			new_sample >>= 8;
			decoded[1] = new_sample & 0xFF;
			decoded   += 2;

			nybble = (*encoded) & 0x0F;
			new_sample = MS_ADPCM_nibble(state[1],nybble,coeff[1]);
			decoded[0] = new_sample & 0xFF;
			new_sample >>= 8;
			decoded[1] = new_sample & 0xFF;
			decoded   += 2;

			++encoded;
			samplesleft -= 2;
		}
		encoded_len -= dstate->wavefmt.blockalign;
	}

	/* free(freeable); */
	return 0;
}

ALint MS_ADPCM_nibble(alMSADPCM_decodestate_LOKI *state,
					ALubyte nybble, ALshort *coeff) {
	const ALint adaptive[] = {
		230, 230, 230, 230, 307, 409, 512, 614,
		768, 614, 512, 409, 307, 230, 230, 230
	};
	ALint new_sample, delta;

	new_sample = ((state->iSamp1 * coeff[0]) +
		      (state->iSamp2 * coeff[1]))/256;

	if(nybble & 0x08) {
		new_sample += state->iDelta * (nybble-0x10);
	} else {
		new_sample += state->iDelta * nybble;
	}

	if(new_sample < MS_ADPCM_min) {
		new_sample = MS_ADPCM_min;
	} else if(new_sample > MS_ADPCM_max) {
		new_sample = MS_ADPCM_max;
	}

	delta = ((ALint) state->iDelta * adaptive[nybble]);
	if(delta < 4096) {
		delta = 16;
	} else {
		delta /= 256;
	}

	state->iDelta = delta;
	state->iSamp2 = state->iSamp1;
	state->iSamp1 = new_sample;

	return new_sample;
}

int InitIMA_ADPCM(alIMAADPCM_state_LOKI *state, alWaveFMT_LOKI *format)
{
	ALubyte  *rogue_feel;
	ALushort extra_info;

	/* Set the rogue pointer to the IMA_ADPCM specific data */
	state->wavefmt.encoding     = swap16le(format->encoding);
	state->wavefmt.channels     = swap16le(format->channels);
	state->wavefmt.frequency    = swap32le(format->frequency);
	state->wavefmt.byterate     = swap32le(format->byterate);
	state->wavefmt.blockalign   = swap16le(format->blockalign);
	state->wavefmt.bitspersample =
					 swap16le(format->bitspersample);
	rogue_feel = (ALubyte *)format + sizeof(*format);
	if ( sizeof(*format) == 16 ) {
		extra_info = ((rogue_feel[1]<<8)|rogue_feel[0]);
		rogue_feel += sizeof(ALushort);
	}
	state->wSamplesPerBlock = ((rogue_feel[1]<<8)|rogue_feel[0]);

	state->state[0].valprev = 0;
	state->state[0].index   = 0;

	state->state[1].valprev = 0;
	state->state[1].index   = 0;

	return(0);
}

static ALint IMA_ADPCM_nibble(alIMAADPCM_decodestate_LOKI *state, ALubyte nybble)
{
	const ALint max_audioval = ((1<<(16-1))-1);
	const ALint min_audioval = -(1<<(16-1));
	const int index_table[16] = {
		-1, -1, -1, -1,
		 2,  4,  6,  8,
		-1, -1, -1, -1,
		 2,  4,  6,  8
	};
	const ALint step_table[89] = {
		7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31,
		34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
		143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408,
		449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282,
		1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
		3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
		9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350,
		22385, 24623, 27086, 29794, 32767
	};
	ALint delta, step;

	/* Compute difference and new sample value */
	step = step_table[(int) state->index];
	delta = step >> 3;
	if ( nybble & 0x04 ) delta += step;
	if ( nybble & 0x02 ) delta += (step >> 1);
	if ( nybble & 0x01 ) delta += (step >> 2);
	if ( nybble & 0x08 ) delta = -delta;
	state->valprev += delta;

	/* Update index value */
	state->index += index_table[nybble];
	if ( state->index > 88 ) {
		state->index = 88;
	} else
	if ( state->index < 0 ) {
		state->index = 0;
	}

	/* Clamp output sample */
	if ( state->valprev > max_audioval ) {
		state->valprev = max_audioval;
	} else
	if ( state->valprev < min_audioval ) {
		state->valprev = min_audioval;
	}
	return(state->valprev);
}

/* Fill the decode buffer with a channel block of data (8 samples) */
static void Fill_IMA_ADPCM_block(ALubyte *decoded, ALubyte *encoded,
	int channel, int numchannels, alIMAADPCM_decodestate_LOKI *state)
{
	int i;
	ALbyte  nybble;
	ALint new_sample;

	decoded += (channel * 2);
	for ( i=0; i<4; ++i ) {
		nybble = (*encoded)&0x0F;
		new_sample = IMA_ADPCM_nibble(state, nybble);
		decoded[0] = new_sample&0xFF;
		new_sample >>= 8;
		decoded[1] = new_sample&0xFF;
		decoded += 2 * numchannels;

		nybble = (*encoded)>>4;
		new_sample = IMA_ADPCM_nibble(state, nybble);
		decoded[0] = new_sample&0xFF;
		new_sample >>= 8;
		decoded[1] = new_sample&0xFF;
		decoded += 2 * numchannels;

		++encoded;
	}
}

int IMA_ADPCM_decode(ALubyte *indata, ALubyte *outdata,
		ALuint len, alIMAADPCM_state_LOKI *istate, int offset) {
	alIMAADPCM_decodestate_LOKI *state;
	int indata_len;
	int c, channels = istate->wavefmt.channels;
	int samplesleft;

	indata += offset;
	state = istate->state;

	if(len < istate->wavefmt.blockalign) {
#ifdef DEBUG_CONVERT
		fprintf(stderr, "IMA_ADPCM_decode len too small!\n");
#endif
		return -1;
	}

	indata_len = len;
	/* Get ready... Go! */
	while ( indata_len >= istate->wavefmt.blockalign ) {
		/* Grab the initial information for this block */
		for ( c=0; c<channels; ++c ) {
			/* Fill the state information for this block */
			state[c].valprev = ((indata[1]<<8)|indata[0]);
			indata += 2;
			if ( state[c].valprev & 0x8000 ) {
				state[c].valprev -= 0x10000;
			}
			state[c].index = *indata++;
			/* Reserved byte in buffer header, should be 0 */
			if ( *indata++ != 0 ) {
				/* Uh oh, corrupt data?  Buggy code? */;
			}

			/* Store the initial valprev we start with */
			outdata[0] = state[c].valprev&0xFF;
			outdata[1] = state[c].valprev>>8;
			outdata += 2;
		}

		/* Decode and store the other samples in this block */
		samplesleft = (istate->wSamplesPerBlock-1)*channels;
		while ( samplesleft > 0 ) {
			for ( c=0; c<channels; ++c ) {
				Fill_IMA_ADPCM_block(outdata, indata,
						c, channels, &state[c]);
				indata += 4;
				samplesleft -= 8;
			}
			outdata += (channels * 8 * 2);
		}
		indata_len -= istate->wavefmt.blockalign;
	}

	return 0;
}

int IMA_ADPCM_decode_FULL(alIMAADPCM_state_LOKI *istate,
			ALubyte **audio_buf, ALuint *audio_len) {
	ALubyte *freeable, *encoded;
	ALint encoded_len;
	unsigned int channels;


	
	/* Check to make sure we have enough variables in the state array */
	channels = istate->wavefmt.channels;
	if ( channels > NELEMS(istate->state) ) {
#ifdef DEBUG_CONVERT
		fprintf(stderr, "IMA ADPCM decoder can only handle %d channels",
						NELEMS(istate->state));
#endif

		return(-1);
	}

	/* Allocate the proper sized output buffer */
	encoded_len = *audio_len;
	encoded = *audio_buf;
	freeable = *audio_buf;
	*audio_len = (encoded_len/istate->wavefmt.blockalign) * 
				istate->wSamplesPerBlock*
				istate->wavefmt.channels*sizeof(ALshort);

	*audio_buf = malloc(*audio_len);
	if ( *audio_buf == NULL ) {
#ifdef DEBUG_CONVERT
		fprintf(stderr, "No mem\n");
#endif
		return(-1);
	}

	return IMA_ADPCM_decode(encoded, *audio_buf,
			encoded_len, istate, 0);
}
