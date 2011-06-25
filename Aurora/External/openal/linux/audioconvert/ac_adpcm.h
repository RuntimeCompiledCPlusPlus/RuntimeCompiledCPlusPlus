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
 *
 * ac_adpcm.h
 *
 * This file contains prototypes and definitions for the ADPCM
 * decoders in audioconvert.
 *
 * For more information, please see ac_adpcm.c
 */
#ifndef _AC_ADPCM_H_
#define _AC_ADPCM_H_

#include "AL/altypes.h"
#include "AL/alexttypes.h"

typedef struct adpcm_state_s {
    ALshort	valprev;	/* Previous output value */
    ALbyte	index;		/* Index into stepsize table */
} adpcm_state_t;

void ac_adpcm_coder   (short [], char [], int, adpcm_state_t *);
void ac_adpcm_decoder (char [], short [], int, adpcm_state_t *, int pos);

int msadpcm_decode(ALubyte *encoded, ALubyte *decoded, ALuint audio_len,
			   alMSADPCM_state_LOKI *dstate, int offset);

ALint MS_ADPCM_nibble(alMSADPCM_decodestate_LOKI *state,
					ALubyte nybble, ALshort *coeff);

int InitIMA_ADPCM(alIMAADPCM_state_LOKI *state, alWaveFMT_LOKI *format);
int IMA_ADPCM_decode_FULL(alIMAADPCM_state_LOKI *state,
			ALubyte **audio_buf, ALuint *audio_len);
int IMA_ADPCM_decode(ALubyte *indata, ALubyte *outdata,
		ALuint len, alIMAADPCM_state_LOKI *istate, int offset);

#endif /* _AC_ADPCM_H_ */
