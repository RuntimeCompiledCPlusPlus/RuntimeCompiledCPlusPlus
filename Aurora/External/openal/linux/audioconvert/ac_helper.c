/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * ac_helper.c
 *
 * audioconvert helper funcs.
 *
 * This file and the funcs within taken almost whole from:
 *     SDL - Simple DirectMedia Layer
 *     Copyright (C) 1997, 1998, 1999  Sam Lantinga
 */

/* Functions for audio drivers to perform runtime conversion of audio format */

#include "audioconvert.h"
#include "AL/altypes.h"
#include "al_siteconfig.h"

#include <stdio.h>
#include <stdlib.h>

void acConvertSign(acAudioCVT *cvt, ALushort format);
void acConvertEndian(acAudioCVT *cvt, ALushort format);

/* Toggle signed/unsigned */
void acConvertSign(acAudioCVT *cvt, ALushort format) {
	int i;
	ALubyte *data;

	data = cvt->buf;
	if((format & 0xFF) == 16) {
		if((format & 0x1000) != 0x1000) { /* Little endian */
			++data;
		}

		for(i = cvt->len_cvt/2; i; i-- ) {
			*data ^= 0x80;
			data += 2;
		}
	} else {
		for(i = cvt->len_cvt; i; i-- ) {
			*data++ ^= 0x80;
		}
	}

	format = (format ^ 0x8000);

	return;
}

/* Toggle endianness */
void acConvertEndian(acAudioCVT *cvt, ALushort format) {
	int i;
	ALubyte *data, tmp;

	data = cvt->buf;
	for(i = cvt->len_cvt/2; i; i--) {
		tmp     = data[0];
		data[0] = data[1];
		data[1] = tmp;
		data   += 2;
	}

	format = (format ^ 0x1000);

}


/* Creates a set of audio filters to convert from one format to another. 
   Returns -1 if the format conversion is not supported, or 1 if the
   audio filter is set up.
*/
  
int acBuildAudioCVT(acAudioCVT *cvt,
	ALushort src_format, ALubyte src_channels, ALuint src_rate,
	ALushort dst_format, ALubyte dst_channels, ALuint dst_rate) {

	/* Start off with no conversion necessary */
	cvt->needed       = 0;
	cvt->filter_index = 0;
	cvt->filters[0]   = NULL;
	cvt->len_mult     = 1;
	cvt->len_ratio    = 1.0;

	/* First filter:  Endian conversion from src to dst */
	if((( src_format & 0x1000) != (dst_format & 0x1000)) &&
	    (( src_format & 0xff) != 8))
	{
		cvt->filters[cvt->filter_index++] = acConvertEndian;
	}
	
	/* Third filter: Sign conversion -- signed/unsigned */
	if((src_format & 0x8000) != (dst_format & 0x8000)) {
		cvt->filters[cvt->filter_index++] = acConvertSign;
	}

	/* Second filter:  Convert 16 bit <--> 8 bit PCM */
	if((src_format & 0xFF) != (dst_format & 0xFF)) {
		switch(dst_format & 0x10FF) {
			case AUDIO_U8:
				cvt->filters[cvt->filter_index++] =
							 acConvert8;
				cvt->len_ratio /= 2;
				break;
			case AUDIO_U16LSB:
				cvt->filters[cvt->filter_index++] =
							acConvert16LSB;
				cvt->len_mult *= 2;
				cvt->len_ratio *= 2;
				break;
			case AUDIO_U16MSB:
				cvt->filters[cvt->filter_index++] =
							acConvert16MSB;
				cvt->len_mult *= 2;
				cvt->len_ratio *= 2;
				break;
			default:
				break;
		}
	}

	/* Last filter:  Mono/Stereo conversion */
	if(src_channels != dst_channels) {
		while((src_channels * 2) <= dst_channels) {
			cvt->filters[cvt->filter_index++] = 
						acConvertStereo;
			src_channels   *= 2;
			cvt->len_mult  *= 2;
			cvt->len_ratio *= 2;
		}

		/*
		 * This assumes that 4 channel audio is in the format:
		 * Left {front/back} + Right {front/back}
		 * so converting to L/R stereo works properly.
		 */
		while(((src_channels%2) == 0) &&
			((src_channels/2) >= dst_channels)) {
			cvt->filters[cvt->filter_index++] =
						 acConvertMono;
			src_channels   /= 2;
			cvt->len_ratio /= 2;
		}
		if ( src_channels != dst_channels ) {
			/* Uh oh.. */;
		}
	}

	/* Do rate conversion */
	cvt->rate_incr = 0.0;
	if((src_rate / 100) != (dst_rate / 100)) {
		ALuint hi_rate, lo_rate;
		int len_mult;
		double len_ratio;
		void (*rate_cvt)(acAudioCVT *, ALushort );

		if ( src_rate > dst_rate ) {
			hi_rate   = src_rate;
			lo_rate   = dst_rate;
			rate_cvt  = acFreqDIV2;
			len_mult  = 1;
			len_ratio = 0.5;
		} else {
			hi_rate   = dst_rate;
			lo_rate   = src_rate;
			rate_cvt  = acFreqMUL2;
			len_mult  = 2;
			len_ratio = 2.0;
		}
		/* If hi_rate = lo_rate*2^x then conversion is easy */
		while(((lo_rate * 2)/100) <= (hi_rate/100)) {
			cvt->filters[cvt->filter_index++] = rate_cvt;
			lo_rate        *= 2;
			cvt->len_mult  *= len_mult;
			cvt->len_ratio *= len_ratio;
		}

		/* We may need a slow conversion here to finish up */
		if((lo_rate/100) != (hi_rate/100)) {
#if 0
			/* The problem with this is that if the input buffer is
			   say 1K, and the conversion rate is say 1.1, then the
			   output buffer is 1.1K, which may not be an acceptable
			   buffer size for the audio driver (not a power of 2)
			*/
			/* For now, punt and hope the rate distortion isn't great.
			*/
#else
			if ( src_rate < dst_rate ) {
				cvt->rate_incr = (double)lo_rate/hi_rate;
				cvt->len_mult *= 2;
				cvt->len_ratio /= cvt->rate_incr;
			} else {
				cvt->rate_incr = (double)hi_rate/lo_rate;
				cvt->len_ratio *= cvt->rate_incr;
			}
			cvt->filters[cvt->filter_index++] = acFreqSLOW;
#endif
		}
	}

	/* Set up the filter information */
	if(cvt->filter_index != 0) {
		cvt->needed     = 1;
		cvt->len        = 0;
		cvt->buf        = NULL;
		cvt->src_format = src_format;
		cvt->dst_format = dst_format;
		cvt->filters[cvt->filter_index] = NULL;
	}

	return cvt->needed;
}

int acConvertAudio(acAudioCVT *cvt) {
	int i;

	/* Make sure there's data to convert */
	if(cvt->buf == NULL) {
#ifdef DEBUG_MAXIMUS
		fprintf(stderr,
			"ALD_MAXIMUS\t[%s:%d]\t %s\n",
			__FILE__, __LINE__,
			"No buffer allocated for conversion");
#endif
		return -1;
	}

	/* Return okay if no conversion is necessary */
	cvt->len_cvt = cvt->len;
	if(cvt->filters[0] == NULL) {
		return 0;
	}

	/* Set up the conversion and go! */
	cvt->filter_index = 0;
	for(i = 0; cvt->filters[i]; i++) {
		cvt->filters[i](cvt, cvt->src_format);
	}

	return 0;
}
