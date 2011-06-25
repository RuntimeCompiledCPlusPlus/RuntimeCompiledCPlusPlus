
/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * mixaudio16.c
 *
 * Where the mixing gets done.  Don't enable MMX, it needs work.
 */

#include "al_siteconfig.h"
#include "al_debug.h"
#include "al_main.h"
#include "al_types.h"
#include "mixaudio16.h"

#include <AL/altypes.h>

#ifdef MMX_SUPPORT
extern void mix16_1( ALshort *dst, ALshort *src,  int len );
extern void mix16_2( ALshort *dst, ALshort **src, int len );
extern void mix16_3( ALshort *dst, ALshort **src, int len );
extern void mix16_4( ALshort *dst, ALshort **src, int len );
extern void mix16_5( ALshort *dst, ALshort **src, int len );
extern void mix16_6( ALshort *dst, ALshort **src, int len );
extern void mix16_7( ALshort *dst, ALshort **src, int len );
#endif

/*
 * MixAudio16( ALshort *dst, ALshort *src, int len )
 *
 * Mix src[0..len/2-1] into dst[0..len/2-1], clamping above by max_audioval
 * and below by min_audioval to prevent overflow.
 */
void MixAudio16(ALshort *dst, ALshort *src, int len) {
#ifdef MMX_SUPPORT
	mix16_1( dst, src, len / 2 );
#else
	int sample;

	len /= sizeof(ALshort); /* len is in bytes */

	while(len--) {
		sample = *dst + *src;

		if( sample < min_audioval ) {
			*dst = min_audioval;
		} else if( sample > max_audioval ) {
			*dst = max_audioval;
		} else {
			*dst = sample;
		}

		src++;
		dst++;
	}
#endif

	return;
}

/*
 * MixAudio16_n( ALshort *dst, alMixEntry *entries, ALuint numents );
 *
 * Mix a numents number of streams into dst, clamping above by max_audioval
 * and below by min_audioval to prevent overflow.
 *
 * FIXME: could be optimized to bisect the data until an "optimized"
 * MixAudio func could be used on it.
 */
void MixAudio16_n( ALshort *dst, alMixEntry *entries, ALuint numents ) {
	int sample;
	ALuint i;
	int si; /* source iterator */
	int len;

	len = entries[0].bytes; /* sure hope all the same */
	len /= sizeof(ALshort);     /* len is in bytes */

	si = 0;
	while(len--) {
		sample = *dst;

		for(i = 0; i < numents; i++) {
			sample += ((ALshort *) entries[i].data)[si];
		}

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		dst++;
		si++;
	}
	return;
}

/*
 * Mixing functions.
 *
 * These are specialized functions which mix each entry in entries to 
 * destination.  They are called using a dispatch table generating by the
 * ALMixManager and ALMixFuncs.
 *
 * The number in the function name refers to the number of entries which the
 * function can handle.  This number is exact, not an upperbound, so the
 * number of streams in entries must match the function exactly.
 */
void MixAudio16_0(UNUSED(ALshort *dst), UNUSED(alMixEntry *entries)) {
	return;
}

void MixAudio16_1(ALshort *dst, alMixEntry *entries) {
	MixAudio16(dst, entries->data, entries->bytes);

	return;
}

void MixAudio16_2(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );

#endif
	return;
}

void MixAudio16_3(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1    = entries[2].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_1( dst, srcs1, len );

#endif
	return;
}

void MixAudio16_4(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );

#endif
	return;
}

void MixAudio16_5(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2    = entries[4].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_1( dst, srcs2, len );

#endif
	return;
}

void MixAudio16_6(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );

#endif
	return;
}

void MixAudio16_7(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3    = entries[6].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_1( dst, srcs3, len );

#endif
	return;
}

void MixAudio16_8(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );

#endif
	return;
}

void MixAudio16_9(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4    = entries[8].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_1( dst, srcs4, len );

#endif
	return;
}

void MixAudio16_10(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );

#endif
	return;
}

void MixAudio16_11(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5    = entries[10].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_1( dst, srcs5, len );

#endif
	return;
}

void MixAudio16_12(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );

#endif
	return;
}

void MixAudio16_13(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6    = entries[12].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_1( dst, srcs6, len );

#endif
	return;
}

void MixAudio16_14(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );

#endif
	return;
}

void MixAudio16_15(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7    = entries[14].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_1( dst, srcs7, len );

#endif
	return;
}

void MixAudio16_16(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );

#endif
	return;
}

void MixAudio16_17(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8    = entries[16].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_1( dst, srcs8, len );

#endif
	return;
}

void MixAudio16_18(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );

#endif
	return;
}

void MixAudio16_19(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9    = entries[18].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_1( dst, srcs9, len );

#endif
	return;
}

void MixAudio16_20(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );

#endif
	return;
}

void MixAudio16_21(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10    = entries[20].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_1( dst, srcs10, len );

#endif
	return;
}

void MixAudio16_22(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );

#endif
	return;
}

void MixAudio16_23(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11    = entries[22].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_1( dst, srcs11, len );

#endif
	return;
}

void MixAudio16_24(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );

#endif
	return;
}

void MixAudio16_25(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12    = entries[24].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_1( dst, srcs12, len );

#endif
	return;
}

void MixAudio16_26(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );

#endif
	return;
}

void MixAudio16_27(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13    = entries[26].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_1( dst, srcs13, len );

#endif
	return;
}

void MixAudio16_28(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );

#endif
	return;
}

void MixAudio16_29(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14    = entries[28].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_1( dst, srcs14, len );

#endif
	return;
}

void MixAudio16_30(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );

#endif
	return;
}

void MixAudio16_31(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15    = entries[30].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_1( dst, srcs15, len );

#endif
	return;
}

void MixAudio16_32(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );

#endif
	return;
}

void MixAudio16_33(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16    = entries[32].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_1( dst, srcs16, len );

#endif
	return;
}

void MixAudio16_34(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );

#endif
	return;
}

void MixAudio16_35(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17    = entries[34].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_1( dst, srcs17, len );

#endif
	return;
}

void MixAudio16_36(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );

#endif
	return;
}

void MixAudio16_37(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18    = entries[36].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_1( dst, srcs18, len );

#endif
	return;
}

void MixAudio16_38(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );

#endif
	return;
}

void MixAudio16_39(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19    = entries[38].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_1( dst, srcs19, len );

#endif
	return;
}

void MixAudio16_40(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );

#endif
	return;
}

void MixAudio16_41(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20    = entries[40].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_1( dst, srcs20, len );

#endif
	return;
}

void MixAudio16_42(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );

#endif
	return;
}

void MixAudio16_43(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21    = entries[42].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_1( dst, srcs21, len );

#endif
	return;
}

void MixAudio16_44(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );

#endif
	return;
}

void MixAudio16_45(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22    = entries[44].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_1( dst, srcs22, len );

#endif
	return;
}

void MixAudio16_46(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );

#endif
	return;
}

void MixAudio16_47(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23    = entries[46].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_1( dst, srcs23, len );

#endif
	return;
}

void MixAudio16_48(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );

#endif
	return;
}

void MixAudio16_49(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24    = entries[48].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_1( dst, srcs24, len );

#endif
	return;
}

void MixAudio16_50(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );

#endif
	return;
}

void MixAudio16_51(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25    = entries[50].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_1( dst, srcs25, len );

#endif
	return;
}

void MixAudio16_52(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25[0] = entries[50].data;
	srcs25[1] = entries[51].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25[0];
		sample += *srcs25[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25[0]++;
		srcs25[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_2( dst, srcs25, len );

#endif
	return;
}

void MixAudio16_53(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25[2];
	ALshort *srcs26;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25[0] = entries[50].data;
	srcs25[1] = entries[51].data;
	srcs26    = entries[52].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25[0];
		sample += *srcs25[1];
		sample += *srcs26;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25[0]++;
		srcs25[1]++;
		srcs26++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_2( dst, srcs25, len );
	mix16_1( dst, srcs26, len );

#endif
	return;
}

void MixAudio16_54(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25[2];
	ALshort *srcs26[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25[0] = entries[50].data;
	srcs25[1] = entries[51].data;
	srcs26[0] = entries[52].data;
	srcs26[1] = entries[53].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25[0];
		sample += *srcs25[1];
		sample += *srcs26[0];
		sample += *srcs26[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25[0]++;
		srcs25[1]++;
		srcs26[0]++;
		srcs26[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_2( dst, srcs25, len );
	mix16_2( dst, srcs26, len );

#endif
	return;
}

void MixAudio16_55(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25[2];
	ALshort *srcs26[2];
	ALshort *srcs27;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25[0] = entries[50].data;
	srcs25[1] = entries[51].data;
	srcs26[0] = entries[52].data;
	srcs26[1] = entries[53].data;
	srcs27    = entries[54].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25[0];
		sample += *srcs25[1];
		sample += *srcs26[0];
		sample += *srcs26[1];
		sample += *srcs27;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25[0]++;
		srcs25[1]++;
		srcs26[0]++;
		srcs26[1]++;
		srcs27++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_2( dst, srcs25, len );
	mix16_2( dst, srcs26, len );
	mix16_1( dst, srcs27, len );

#endif
	return;
}

void MixAudio16_56(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25[2];
	ALshort *srcs26[2];
	ALshort *srcs27[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25[0] = entries[50].data;
	srcs25[1] = entries[51].data;
	srcs26[0] = entries[52].data;
	srcs26[1] = entries[53].data;
	srcs27[0] = entries[54].data;
	srcs27[1] = entries[55].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25[0];
		sample += *srcs25[1];
		sample += *srcs26[0];
		sample += *srcs26[1];
		sample += *srcs27[0];
		sample += *srcs27[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25[0]++;
		srcs25[1]++;
		srcs26[0]++;
		srcs26[1]++;
		srcs27[0]++;
		srcs27[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_2( dst, srcs25, len );
	mix16_2( dst, srcs26, len );
	mix16_2( dst, srcs27, len );

#endif
	return;
}

void MixAudio16_57(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25[2];
	ALshort *srcs26[2];
	ALshort *srcs27[2];
	ALshort *srcs28;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25[0] = entries[50].data;
	srcs25[1] = entries[51].data;
	srcs26[0] = entries[52].data;
	srcs26[1] = entries[53].data;
	srcs27[0] = entries[54].data;
	srcs27[1] = entries[55].data;
	srcs28    = entries[56].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25[0];
		sample += *srcs25[1];
		sample += *srcs26[0];
		sample += *srcs26[1];
		sample += *srcs27[0];
		sample += *srcs27[1];
		sample += *srcs28;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25[0]++;
		srcs25[1]++;
		srcs26[0]++;
		srcs26[1]++;
		srcs27[0]++;
		srcs27[1]++;
		srcs28++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_2( dst, srcs25, len );
	mix16_2( dst, srcs26, len );
	mix16_2( dst, srcs27, len );
	mix16_1( dst, srcs28, len );

#endif
	return;
}

void MixAudio16_58(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25[2];
	ALshort *srcs26[2];
	ALshort *srcs27[2];
	ALshort *srcs28[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25[0] = entries[50].data;
	srcs25[1] = entries[51].data;
	srcs26[0] = entries[52].data;
	srcs26[1] = entries[53].data;
	srcs27[0] = entries[54].data;
	srcs27[1] = entries[55].data;
	srcs28[0] = entries[56].data;
	srcs28[1] = entries[57].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25[0];
		sample += *srcs25[1];
		sample += *srcs26[0];
		sample += *srcs26[1];
		sample += *srcs27[0];
		sample += *srcs27[1];
		sample += *srcs28[0];
		sample += *srcs28[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25[0]++;
		srcs25[1]++;
		srcs26[0]++;
		srcs26[1]++;
		srcs27[0]++;
		srcs27[1]++;
		srcs28[0]++;
		srcs28[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_2( dst, srcs25, len );
	mix16_2( dst, srcs26, len );
	mix16_2( dst, srcs27, len );
	mix16_2( dst, srcs28, len );

#endif
	return;
}

void MixAudio16_59(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25[2];
	ALshort *srcs26[2];
	ALshort *srcs27[2];
	ALshort *srcs28[2];
	ALshort *srcs29;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25[0] = entries[50].data;
	srcs25[1] = entries[51].data;
	srcs26[0] = entries[52].data;
	srcs26[1] = entries[53].data;
	srcs27[0] = entries[54].data;
	srcs27[1] = entries[55].data;
	srcs28[0] = entries[56].data;
	srcs28[1] = entries[57].data;
	srcs29    = entries[58].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25[0];
		sample += *srcs25[1];
		sample += *srcs26[0];
		sample += *srcs26[1];
		sample += *srcs27[0];
		sample += *srcs27[1];
		sample += *srcs28[0];
		sample += *srcs28[1];
		sample += *srcs29;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25[0]++;
		srcs25[1]++;
		srcs26[0]++;
		srcs26[1]++;
		srcs27[0]++;
		srcs27[1]++;
		srcs28[0]++;
		srcs28[1]++;
		srcs29++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_2( dst, srcs25, len );
	mix16_2( dst, srcs26, len );
	mix16_2( dst, srcs27, len );
	mix16_2( dst, srcs28, len );
	mix16_1( dst, srcs29, len );

#endif
	return;
}

void MixAudio16_60(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25[2];
	ALshort *srcs26[2];
	ALshort *srcs27[2];
	ALshort *srcs28[2];
	ALshort *srcs29[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25[0] = entries[50].data;
	srcs25[1] = entries[51].data;
	srcs26[0] = entries[52].data;
	srcs26[1] = entries[53].data;
	srcs27[0] = entries[54].data;
	srcs27[1] = entries[55].data;
	srcs28[0] = entries[56].data;
	srcs28[1] = entries[57].data;
	srcs29[0] = entries[58].data;
	srcs29[1] = entries[59].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25[0];
		sample += *srcs25[1];
		sample += *srcs26[0];
		sample += *srcs26[1];
		sample += *srcs27[0];
		sample += *srcs27[1];
		sample += *srcs28[0];
		sample += *srcs28[1];
		sample += *srcs29[0];
		sample += *srcs29[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25[0]++;
		srcs25[1]++;
		srcs26[0]++;
		srcs26[1]++;
		srcs27[0]++;
		srcs27[1]++;
		srcs28[0]++;
		srcs28[1]++;
		srcs29[0]++;
		srcs29[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_2( dst, srcs25, len );
	mix16_2( dst, srcs26, len );
	mix16_2( dst, srcs27, len );
	mix16_2( dst, srcs28, len );
	mix16_2( dst, srcs29, len );

#endif
	return;
}

void MixAudio16_61(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25[2];
	ALshort *srcs26[2];
	ALshort *srcs27[2];
	ALshort *srcs28[2];
	ALshort *srcs29[2];
	ALshort *srcs30;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25[0] = entries[50].data;
	srcs25[1] = entries[51].data;
	srcs26[0] = entries[52].data;
	srcs26[1] = entries[53].data;
	srcs27[0] = entries[54].data;
	srcs27[1] = entries[55].data;
	srcs28[0] = entries[56].data;
	srcs28[1] = entries[57].data;
	srcs29[0] = entries[58].data;
	srcs29[1] = entries[59].data;
	srcs30    = entries[60].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25[0];
		sample += *srcs25[1];
		sample += *srcs26[0];
		sample += *srcs26[1];
		sample += *srcs27[0];
		sample += *srcs27[1];
		sample += *srcs28[0];
		sample += *srcs28[1];
		sample += *srcs29[0];
		sample += *srcs29[1];
		sample += *srcs30;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25[0]++;
		srcs25[1]++;
		srcs26[0]++;
		srcs26[1]++;
		srcs27[0]++;
		srcs27[1]++;
		srcs28[0]++;
		srcs28[1]++;
		srcs29[0]++;
		srcs29[1]++;
		srcs30++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_2( dst, srcs25, len );
	mix16_2( dst, srcs26, len );
	mix16_2( dst, srcs27, len );
	mix16_2( dst, srcs28, len );
	mix16_2( dst, srcs29, len );
	mix16_1( dst, srcs30, len );

#endif
	return;
}

void MixAudio16_62(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25[2];
	ALshort *srcs26[2];
	ALshort *srcs27[2];
	ALshort *srcs28[2];
	ALshort *srcs29[2];
	ALshort *srcs30[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25[0] = entries[50].data;
	srcs25[1] = entries[51].data;
	srcs26[0] = entries[52].data;
	srcs26[1] = entries[53].data;
	srcs27[0] = entries[54].data;
	srcs27[1] = entries[55].data;
	srcs28[0] = entries[56].data;
	srcs28[1] = entries[57].data;
	srcs29[0] = entries[58].data;
	srcs29[1] = entries[59].data;
	srcs30[0] = entries[60].data;
	srcs30[1] = entries[61].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25[0];
		sample += *srcs25[1];
		sample += *srcs26[0];
		sample += *srcs26[1];
		sample += *srcs27[0];
		sample += *srcs27[1];
		sample += *srcs28[0];
		sample += *srcs28[1];
		sample += *srcs29[0];
		sample += *srcs29[1];
		sample += *srcs30[0];
		sample += *srcs30[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25[0]++;
		srcs25[1]++;
		srcs26[0]++;
		srcs26[1]++;
		srcs27[0]++;
		srcs27[1]++;
		srcs28[0]++;
		srcs28[1]++;
		srcs29[0]++;
		srcs29[1]++;
		srcs30[0]++;
		srcs30[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_2( dst, srcs25, len );
	mix16_2( dst, srcs26, len );
	mix16_2( dst, srcs27, len );
	mix16_2( dst, srcs28, len );
	mix16_2( dst, srcs29, len );
	mix16_2( dst, srcs30, len );

#endif
	return;
}

void MixAudio16_63(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25[2];
	ALshort *srcs26[2];
	ALshort *srcs27[2];
	ALshort *srcs28[2];
	ALshort *srcs29[2];
	ALshort *srcs30[2];
	ALshort *srcs31;

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25[0] = entries[50].data;
	srcs25[1] = entries[51].data;
	srcs26[0] = entries[52].data;
	srcs26[1] = entries[53].data;
	srcs27[0] = entries[54].data;
	srcs27[1] = entries[55].data;
	srcs28[0] = entries[56].data;
	srcs28[1] = entries[57].data;
	srcs29[0] = entries[58].data;
	srcs29[1] = entries[59].data;
	srcs30[0] = entries[60].data;
	srcs30[1] = entries[61].data;
	srcs31    = entries[62].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25[0];
		sample += *srcs25[1];
		sample += *srcs26[0];
		sample += *srcs26[1];
		sample += *srcs27[0];
		sample += *srcs27[1];
		sample += *srcs28[0];
		sample += *srcs28[1];
		sample += *srcs29[0];
		sample += *srcs29[1];
		sample += *srcs30[0];
		sample += *srcs30[1];
		sample += *srcs31;

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25[0]++;
		srcs25[1]++;
		srcs26[0]++;
		srcs26[1]++;
		srcs27[0]++;
		srcs27[1]++;
		srcs28[0]++;
		srcs28[1]++;
		srcs29[0]++;
		srcs29[1]++;
		srcs30[0]++;
		srcs30[1]++;
		srcs31++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_2( dst, srcs25, len );
	mix16_2( dst, srcs26, len );
	mix16_2( dst, srcs27, len );
	mix16_2( dst, srcs28, len );
	mix16_2( dst, srcs29, len );
	mix16_2( dst, srcs30, len );
	mix16_1( dst, srcs31, len );

#endif
	return;
}

void MixAudio16_64(ALshort *dst, alMixEntry *entries) {
	ALshort *srcs0[2];
	ALshort *srcs1[2];
	ALshort *srcs2[2];
	ALshort *srcs3[2];
	ALshort *srcs4[2];
	ALshort *srcs5[2];
	ALshort *srcs6[2];
	ALshort *srcs7[2];
	ALshort *srcs8[2];
	ALshort *srcs9[2];
	ALshort *srcs10[2];
	ALshort *srcs11[2];
	ALshort *srcs12[2];
	ALshort *srcs13[2];
	ALshort *srcs14[2];
	ALshort *srcs15[2];
	ALshort *srcs16[2];
	ALshort *srcs17[2];
	ALshort *srcs18[2];
	ALshort *srcs19[2];
	ALshort *srcs20[2];
	ALshort *srcs21[2];
	ALshort *srcs22[2];
	ALshort *srcs23[2];
	ALshort *srcs24[2];
	ALshort *srcs25[2];
	ALshort *srcs26[2];
	ALshort *srcs27[2];
	ALshort *srcs28[2];
	ALshort *srcs29[2];
	ALshort *srcs30[2];
	ALshort *srcs31[2];

	int len;
#ifndef MMX_SUPPORT
	int sample;
#endif

	len = entries[0].bytes;
	len /= sizeof(ALshort);

	srcs0[0] = entries[0].data;
	srcs0[1] = entries[1].data;
	srcs1[0] = entries[2].data;
	srcs1[1] = entries[3].data;
	srcs2[0] = entries[4].data;
	srcs2[1] = entries[5].data;
	srcs3[0] = entries[6].data;
	srcs3[1] = entries[7].data;
	srcs4[0] = entries[8].data;
	srcs4[1] = entries[9].data;
	srcs5[0] = entries[10].data;
	srcs5[1] = entries[11].data;
	srcs6[0] = entries[12].data;
	srcs6[1] = entries[13].data;
	srcs7[0] = entries[14].data;
	srcs7[1] = entries[15].data;
	srcs8[0] = entries[16].data;
	srcs8[1] = entries[17].data;
	srcs9[0] = entries[18].data;
	srcs9[1] = entries[19].data;
	srcs10[0] = entries[20].data;
	srcs10[1] = entries[21].data;
	srcs11[0] = entries[22].data;
	srcs11[1] = entries[23].data;
	srcs12[0] = entries[24].data;
	srcs12[1] = entries[25].data;
	srcs13[0] = entries[26].data;
	srcs13[1] = entries[27].data;
	srcs14[0] = entries[28].data;
	srcs14[1] = entries[29].data;
	srcs15[0] = entries[30].data;
	srcs15[1] = entries[31].data;
	srcs16[0] = entries[32].data;
	srcs16[1] = entries[33].data;
	srcs17[0] = entries[34].data;
	srcs17[1] = entries[35].data;
	srcs18[0] = entries[36].data;
	srcs18[1] = entries[37].data;
	srcs19[0] = entries[38].data;
	srcs19[1] = entries[39].data;
	srcs20[0] = entries[40].data;
	srcs20[1] = entries[41].data;
	srcs21[0] = entries[42].data;
	srcs21[1] = entries[43].data;
	srcs22[0] = entries[44].data;
	srcs22[1] = entries[45].data;
	srcs23[0] = entries[46].data;
	srcs23[1] = entries[47].data;
	srcs24[0] = entries[48].data;
	srcs24[1] = entries[49].data;
	srcs25[0] = entries[50].data;
	srcs25[1] = entries[51].data;
	srcs26[0] = entries[52].data;
	srcs26[1] = entries[53].data;
	srcs27[0] = entries[54].data;
	srcs27[1] = entries[55].data;
	srcs28[0] = entries[56].data;
	srcs28[1] = entries[57].data;
	srcs29[0] = entries[58].data;
	srcs29[1] = entries[59].data;
	srcs30[0] = entries[60].data;
	srcs30[1] = entries[61].data;
	srcs31[0] = entries[62].data;
	srcs31[1] = entries[63].data;


#ifndef MMX_SUPPORT
	while(len--) {
		sample = *dst;

		sample = *srcs0[0];
		sample += *srcs0[1];
		sample += *srcs1[0];
		sample += *srcs1[1];
		sample += *srcs2[0];
		sample += *srcs2[1];
		sample += *srcs3[0];
		sample += *srcs3[1];
		sample += *srcs4[0];
		sample += *srcs4[1];
		sample += *srcs5[0];
		sample += *srcs5[1];
		sample += *srcs6[0];
		sample += *srcs6[1];
		sample += *srcs7[0];
		sample += *srcs7[1];
		sample += *srcs8[0];
		sample += *srcs8[1];
		sample += *srcs9[0];
		sample += *srcs9[1];
		sample += *srcs10[0];
		sample += *srcs10[1];
		sample += *srcs11[0];
		sample += *srcs11[1];
		sample += *srcs12[0];
		sample += *srcs12[1];
		sample += *srcs13[0];
		sample += *srcs13[1];
		sample += *srcs14[0];
		sample += *srcs14[1];
		sample += *srcs15[0];
		sample += *srcs15[1];
		sample += *srcs16[0];
		sample += *srcs16[1];
		sample += *srcs17[0];
		sample += *srcs17[1];
		sample += *srcs18[0];
		sample += *srcs18[1];
		sample += *srcs19[0];
		sample += *srcs19[1];
		sample += *srcs20[0];
		sample += *srcs20[1];
		sample += *srcs21[0];
		sample += *srcs21[1];
		sample += *srcs22[0];
		sample += *srcs22[1];
		sample += *srcs23[0];
		sample += *srcs23[1];
		sample += *srcs24[0];
		sample += *srcs24[1];
		sample += *srcs25[0];
		sample += *srcs25[1];
		sample += *srcs26[0];
		sample += *srcs26[1];
		sample += *srcs27[0];
		sample += *srcs27[1];
		sample += *srcs28[0];
		sample += *srcs28[1];
		sample += *srcs29[0];
		sample += *srcs29[1];
		sample += *srcs30[0];
		sample += *srcs30[1];
		sample += *srcs31[0];
		sample += *srcs31[1];

		if(sample > max_audioval ) {
			*dst = max_audioval;
		} else if(sample < min_audioval ) {
			*dst = min_audioval;
		} else {
			*dst = sample;
		}

		srcs0[0]++;
		srcs0[1]++;
		srcs1[0]++;
		srcs1[1]++;
		srcs2[0]++;
		srcs2[1]++;
		srcs3[0]++;
		srcs3[1]++;
		srcs4[0]++;
		srcs4[1]++;
		srcs5[0]++;
		srcs5[1]++;
		srcs6[0]++;
		srcs6[1]++;
		srcs7[0]++;
		srcs7[1]++;
		srcs8[0]++;
		srcs8[1]++;
		srcs9[0]++;
		srcs9[1]++;
		srcs10[0]++;
		srcs10[1]++;
		srcs11[0]++;
		srcs11[1]++;
		srcs12[0]++;
		srcs12[1]++;
		srcs13[0]++;
		srcs13[1]++;
		srcs14[0]++;
		srcs14[1]++;
		srcs15[0]++;
		srcs15[1]++;
		srcs16[0]++;
		srcs16[1]++;
		srcs17[0]++;
		srcs17[1]++;
		srcs18[0]++;
		srcs18[1]++;
		srcs19[0]++;
		srcs19[1]++;
		srcs20[0]++;
		srcs20[1]++;
		srcs21[0]++;
		srcs21[1]++;
		srcs22[0]++;
		srcs22[1]++;
		srcs23[0]++;
		srcs23[1]++;
		srcs24[0]++;
		srcs24[1]++;
		srcs25[0]++;
		srcs25[1]++;
		srcs26[0]++;
		srcs26[1]++;
		srcs27[0]++;
		srcs27[1]++;
		srcs28[0]++;
		srcs28[1]++;
		srcs29[0]++;
		srcs29[1]++;
		srcs30[0]++;
		srcs30[1]++;
		srcs31[0]++;
		srcs31[1]++;


		dst++;
	}
#else
	mix16_2( dst, srcs0, len );
	mix16_2( dst, srcs1, len );
	mix16_2( dst, srcs2, len );
	mix16_2( dst, srcs3, len );
	mix16_2( dst, srcs4, len );
	mix16_2( dst, srcs5, len );
	mix16_2( dst, srcs6, len );
	mix16_2( dst, srcs7, len );
	mix16_2( dst, srcs8, len );
	mix16_2( dst, srcs9, len );
	mix16_2( dst, srcs10, len );
	mix16_2( dst, srcs11, len );
	mix16_2( dst, srcs12, len );
	mix16_2( dst, srcs13, len );
	mix16_2( dst, srcs14, len );
	mix16_2( dst, srcs15, len );
	mix16_2( dst, srcs16, len );
	mix16_2( dst, srcs17, len );
	mix16_2( dst, srcs18, len );
	mix16_2( dst, srcs19, len );
	mix16_2( dst, srcs20, len );
	mix16_2( dst, srcs21, len );
	mix16_2( dst, srcs22, len );
	mix16_2( dst, srcs23, len );
	mix16_2( dst, srcs24, len );
	mix16_2( dst, srcs25, len );
	mix16_2( dst, srcs26, len );
	mix16_2( dst, srcs27, len );
	mix16_2( dst, srcs28, len );
	mix16_2( dst, srcs29, len );
	mix16_2( dst, srcs30, len );
	mix16_2( dst, srcs31, len );

#endif
	return;
}
