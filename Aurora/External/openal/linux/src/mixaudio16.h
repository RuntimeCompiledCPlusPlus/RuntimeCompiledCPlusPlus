/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * mixaudio16.h
 *
 * Mixing functions for signed, 16-bit PCM streams.
 */
#ifndef _MIX_AUDIO16_H_
#define _MIX_AUDIO16_H_

#include <AL/altypes.h>
#include "al_types.h"

/*
 * max_audioval is the maximum value of a signed short.
 */
#define max_audioval ((1<<(16-1))-1)

/*
 * min_audioval is the minimum value of a signed short.
 */
#define min_audioval -(1<<(16-1))

/*
 * MixAudio16( ALshort *dst, ALshort *src, int len )
 *
 * Mix src[0..len/2-1] into dst[0..len/2-1], clamping above by max_audioval
 * and below by min_audioval to prevent overflow.
 */
void MixAudio16( ALshort *dst, ALshort *src, int len );

/*
 * MixAudio16_n( ALshort *dst, alMixEntry *entries, ALuint numents );
 *
 * Mix a numents number of streams into dst, clamping above by max_audioval
 * and below by min_audioval to prevent overflow.
 */
void MixAudio16_n( ALshort *dst, alMixEntry *entries, ALuint numents );

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
void MixAudio16_0( ALshort *dst, alMixEntry *entries );
void MixAudio16_1( ALshort *dst, alMixEntry *entries );
void MixAudio16_2( ALshort *dst, alMixEntry *entries );
void MixAudio16_3( ALshort *dst, alMixEntry *entries );
void MixAudio16_4( ALshort *dst, alMixEntry *entries );
void MixAudio16_5( ALshort *dst, alMixEntry *entries );
void MixAudio16_6( ALshort *dst, alMixEntry *entries );
void MixAudio16_7( ALshort *dst, alMixEntry *entries );
void MixAudio16_8( ALshort *dst, alMixEntry *entries );
void MixAudio16_9( ALshort *dst, alMixEntry *entries );
void MixAudio16_10( ALshort *dst, alMixEntry *entries );
void MixAudio16_11( ALshort *dst, alMixEntry *entries );
void MixAudio16_12( ALshort *dst, alMixEntry *entries );
void MixAudio16_13( ALshort *dst, alMixEntry *entries );
void MixAudio16_14( ALshort *dst, alMixEntry *entries );
void MixAudio16_15( ALshort *dst, alMixEntry *entries );
void MixAudio16_16( ALshort *dst, alMixEntry *entries );
void MixAudio16_17( ALshort *dst, alMixEntry *entries );
void MixAudio16_18( ALshort *dst, alMixEntry *entries );
void MixAudio16_19( ALshort *dst, alMixEntry *entries );
void MixAudio16_20( ALshort *dst, alMixEntry *entries );
void MixAudio16_21( ALshort *dst, alMixEntry *entries );
void MixAudio16_22( ALshort *dst, alMixEntry *entries );
void MixAudio16_23( ALshort *dst, alMixEntry *entries );
void MixAudio16_24( ALshort *dst, alMixEntry *entries );
void MixAudio16_25( ALshort *dst, alMixEntry *entries );
void MixAudio16_26( ALshort *dst, alMixEntry *entries );
void MixAudio16_27( ALshort *dst, alMixEntry *entries );
void MixAudio16_28( ALshort *dst, alMixEntry *entries );
void MixAudio16_29( ALshort *dst, alMixEntry *entries );
void MixAudio16_30( ALshort *dst, alMixEntry *entries );
void MixAudio16_31( ALshort *dst, alMixEntry *entries );
void MixAudio16_32( ALshort *dst, alMixEntry *entries );
void MixAudio16_33( ALshort *dst, alMixEntry *entries );
void MixAudio16_34( ALshort *dst, alMixEntry *entries );
void MixAudio16_35( ALshort *dst, alMixEntry *entries );
void MixAudio16_36( ALshort *dst, alMixEntry *entries );
void MixAudio16_37( ALshort *dst, alMixEntry *entries );
void MixAudio16_38( ALshort *dst, alMixEntry *entries );
void MixAudio16_39( ALshort *dst, alMixEntry *entries );
void MixAudio16_40( ALshort *dst, alMixEntry *entries );
void MixAudio16_41( ALshort *dst, alMixEntry *entries );
void MixAudio16_42( ALshort *dst, alMixEntry *entries );
void MixAudio16_43( ALshort *dst, alMixEntry *entries );
void MixAudio16_44( ALshort *dst, alMixEntry *entries );
void MixAudio16_45( ALshort *dst, alMixEntry *entries );
void MixAudio16_46( ALshort *dst, alMixEntry *entries );
void MixAudio16_47( ALshort *dst, alMixEntry *entries );
void MixAudio16_48( ALshort *dst, alMixEntry *entries );
void MixAudio16_49( ALshort *dst, alMixEntry *entries );
void MixAudio16_50( ALshort *dst, alMixEntry *entries );
void MixAudio16_51( ALshort *dst, alMixEntry *entries );
void MixAudio16_52( ALshort *dst, alMixEntry *entries );
void MixAudio16_53( ALshort *dst, alMixEntry *entries );
void MixAudio16_54( ALshort *dst, alMixEntry *entries );
void MixAudio16_55( ALshort *dst, alMixEntry *entries );
void MixAudio16_56( ALshort *dst, alMixEntry *entries );
void MixAudio16_57( ALshort *dst, alMixEntry *entries );
void MixAudio16_58( ALshort *dst, alMixEntry *entries );
void MixAudio16_59( ALshort *dst, alMixEntry *entries );
void MixAudio16_60( ALshort *dst, alMixEntry *entries );
void MixAudio16_61( ALshort *dst, alMixEntry *entries );
void MixAudio16_62( ALshort *dst, alMixEntry *entries );
void MixAudio16_63( ALshort *dst, alMixEntry *entries );
void MixAudio16_64( ALshort *dst, alMixEntry *entries );

#endif /* _MIX_AUDIO16_H_ */
