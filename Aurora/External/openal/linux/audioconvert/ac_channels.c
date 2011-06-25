/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * ac_channels.c
 *
 * audioconvert function related to number of audio channels
 */

#include "audioconvert.h"
#include "AL/altypes.h"
#include "al_siteconfig.h"

#include <stdio.h>

/* Duplicate a mono channel to both stereo channels */
void acConvertStereo(acAudioCVT *cvt, ALushort format) {
	int i;

	if((format & 0xFF) == 16) {
		ALushort *src, *dst;

		src  = (ALushort *) cvt->buf;
		dst  = (ALushort *) cvt->buf;
		src += cvt->len_cvt / sizeof *src;
		dst += cvt->len_cvt * 2 / sizeof *dst;

		for ( i=cvt->len_cvt/2; i; --i ) {
			dst -= 2;
			src -= 1;
			dst[0] = src[0];
			dst[1] = src[0];
		}
	} else {
		ALubyte *src, *dst;

		src = (ALubyte *) cvt->buf + cvt->len_cvt;
		dst = (ALubyte *) cvt->buf + cvt->len_cvt * 2;
		for(i = cvt->len_cvt; i; i-- ) {
			dst -= 2;
			src -= 1;
			dst[0] = src[0];
			dst[1] = src[0];
		}
	}

	cvt->len_cvt *= 2;
}

/* Effectively mix right and left channels into a single channel */
void acConvertMono(acAudioCVT *cvt, ALushort format) {
	int i;
	ALint sample;

	switch(format & 0x8018) {
		case AUDIO_U8: {
			ALubyte *src, *dst;

			src = cvt->buf;
			dst = cvt->buf;
			for ( i=cvt->len_cvt/2; i; --i ) {
				sample = src[0] + src[1];
				sample /= 2;

				if ( sample > 255 ) {
					*dst = 255;
				} else {
					*dst = sample;
				}
				src += 2;
				dst += 1;
			}
		}
		break;

		case AUDIO_S8: {
			ALbyte *src, *dst;

			src = (ALbyte *)cvt->buf;
			dst = (ALbyte *)cvt->buf;
			for ( i=cvt->len_cvt/2; i; --i ) {
				sample = src[0] + src[1];
				sample /= 2;

				if ( sample > 127 ) {
					*dst = 127;
				} else
				if ( sample < -128 ) {
					*dst = -128;
				} else {
					*dst = sample;
				}
				src += 2;
				dst += 1;
			}
		}
		break;

		case AUDIO_U16: {
			ALubyte *src, *dst;

			src = cvt->buf;
			dst = cvt->buf;
			if ( (format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len_cvt/4; i; --i ) {
					sample = (ALushort)((src[0]<<8)|src[1])+
					         (ALushort)((src[2]<<8)|src[3]);
					sample /= 2;

					if ( sample > 65535 ) {
						dst[0] = 0xFF;
						dst[1] = 0xFF;
					} else {
						dst[1] = (sample&0xFF);
						sample >>= 8;
						dst[0] = (sample&0xFF);
					}
					src += 4;
					dst += 2;
				}
			} else {
				for ( i=cvt->len_cvt/4; i; --i ) {
					sample = (ALushort)((src[1]<<8)|src[0])+
					         (ALushort)((src[3]<<8)|src[2]);
					sample /= 2;

					if ( sample > 65535 ) {
						dst[0] = 0xFF;
						dst[1] = 0xFF;
					} else {
						dst[0] = (sample&0xFF);
						sample >>= 8;
						dst[1] = (sample&0xFF);
					}
					src += 4;
					dst += 2;
				}
			}
		}
		break;

		case AUDIO_S16: {
			ALubyte *src, *dst;

			src = cvt->buf;
			dst = cvt->buf;
			if ( (format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len_cvt/4; i; --i ) {
					sample = (ALshort)((src[0]<<8)|src[1])+
					         (ALshort)((src[2]<<8)|src[3]);
					sample /= 2;

					if ( sample > 32767 ) {
						dst[0] = 0x7F;
						dst[1] = 0xFF;
					} else
					if ( sample < -32768 ) {
						dst[0] = 0x80;
						dst[1] = 0x00;
					} else {
						dst[1] = (sample&0xFF);
						sample >>= 8;
						dst[0] = (sample&0xFF);
					}
					src += 4;
					dst += 2;
				}
			} else {
				for ( i=cvt->len_cvt/4; i; --i ) {
					sample = (ALshort)((src[1]<<8)|src[0])+
					         (ALshort)((src[3]<<8)|src[2]);
					sample /= 2;

					if ( sample > 32767 ) {
						dst[1] = 0x7F;
						dst[0] = 0xFF;
					} else
					if ( sample < -32768 ) {
						dst[1] = 0x80;
						dst[0] = 0x00;
					} else {
						dst[0] = (sample&0xFF);
						sample >>= 8;
						dst[1] = (sample&0xFF);
					}
					src += 4;
					dst += 2;
				}
			}
		}
		break;
	}
	cvt->len_cvt /= 2;
}
