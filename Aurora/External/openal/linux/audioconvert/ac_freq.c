/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * ac_freq.c
 *
 * audioconvert functions related to changing the sampling
 * rate of a buffer.
 *
 */
#include "audioconvert.h"
#include "AL/altypes.h"
#include "al_siteconfig.h"

#include <stdio.h>

/* Convert rate up by multiple of 2 */
void acFreqMUL2(acAudioCVT *cvt, ALushort format) {
	int i;
	ALubyte *src, *dst;

	src = (ALubyte *) cvt->buf + cvt->len_cvt;
	dst = (ALubyte *) cvt->buf + cvt->len_cvt * 2;

	switch(format & 0xFF) {
		case 8:
			for ( i=cvt->len_cvt; i; --i ) {
				src -= 1;
				dst -= 2;
				dst[0] = src[0];
				dst[1] = src[0];
			}
			break;
		case 16:
			for ( i=cvt->len_cvt/2; i; --i ) {
				src -= 2;
				dst -= 4;
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[0];
				dst[3] = src[1];
			}
			break;
	}
	cvt->len_cvt *= 2;
}

/* Convert rate down by multiple of 2 */
void acFreqDIV2(acAudioCVT *cvt, ALushort format) {
	int i;
	ALubyte *src, *dst;

	src = cvt->buf;
	dst = cvt->buf;
	switch(format & 0xFF) {
		case 8:
			for ( i=cvt->len_cvt/2; i; --i ) {
				dst[0] = src[0];
				src += 2;
				dst += 1;
			}
			break;
		case 16:
			for ( i=cvt->len_cvt/4; i; --i ) {
				dst[0] = src[0];
				dst[1] = src[1];
				src += 4;
				dst += 2;
			}
			break;
	}

	cvt->len_cvt /= 2;
}

/* Very slow rate conversion routine */
void acFreqSLOW(acAudioCVT *cvt, ALushort format) {
	double ipos;
	int i, clen;

	clen = (int) ((double)cvt->len_cvt / cvt->rate_incr);
	if(cvt->rate_incr > 1.0) {
		switch(format & 0xFF) {
			case 8: {
				ALubyte *output;

				output = cvt->buf;
				ipos = 0.0;
				for ( i=clen; i; --i ) {
					*output = *((ALubyte *) cvt->buf + (int)ipos);
					ipos += cvt->rate_incr;
					output += 1;
				}
			}
			break;

			case 16: {
				ALushort *output;

				clen &= ~1;
				output = (ALushort *) cvt->buf;
				ipos = 0.0;
				for ( i=clen/2; i; --i ) {
					*output=((ALushort *)cvt->buf)[(int)ipos];
					ipos += cvt->rate_incr;
					output += 1;
				}
			}
			break;
		}
	} else {
		switch (format & 0xFF) {
			case 8: {
				ALubyte *output;

				output = (ALubyte *) cvt->buf + clen;
				ipos = (double)cvt->len_cvt;
				for ( i=clen; i; --i ) {
					ipos -= cvt->rate_incr;
					output -= 1;
					*output = *((ALubyte *) cvt->buf + (int)ipos);
				}
			}
			break;

			case 16: {
				ALushort *output;

				clen &= ~1;
				output = (ALushort *) cvt->buf;
				output += clen / sizeof *output;

				ipos = (double)cvt->len_cvt/2;
				for ( i=clen/2; i; --i ) {
					ipos -= cvt->rate_incr;
					output -= 1;
					*output=((ALushort *)cvt->buf)[(int)ipos];
				}
			}
			break;
		}
	}
	cvt->len_cvt = clen;
}
