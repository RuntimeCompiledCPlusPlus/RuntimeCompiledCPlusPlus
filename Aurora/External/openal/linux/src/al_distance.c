/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_distance.c
 *
 * Distance tweakage.
 */
#include "al_siteconfig.h"
#include "al_distance.h"
#include "al_error.h"
#include "al_main.h"
#include "alc/alc_context.h"
#include "alc/alc_error.h"

#include <AL/al.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN_DISTANCE 0.001
#define MAX_DISTANCE 40000

/*
 * alDistanceModel( ALenum distanceModel )
 *
 * Sets the distance model for the current context.  If distanceModel is not
 * one of AL_NONE, AL_INVERSE_DISTANCE, or AL_INVERSE_DISTANCE_CLAMPED,
 * AL_ILLEGAL_ENUM is set.
 */
void alDistanceModel( ALenum distanceModel ) {
	_alcDCLockContext();

	_alDistanceModel( distanceModel );

	_alcDCUnlockContext();

	return;
}

/*
 *  ALfloat foo( double value, double max )
 *
 *  these functions normalize the value value to somewhere
 *  within [0.0-1.0].  This is, effectively, the distance
 *  model.
 */


/*
 * _alDistanceNone
 *
 * No distance attenuation.  Which means we return 1.0 for
 * all values.
 */
ALfloat _alDistanceNone( UNUSED(ALfloat dis),
			 UNUSED(ALfloat gain),
			 UNUSED(ALfloat rolloff),
			 UNUSED(ALfloat ref),
			 UNUSED(ALfloat max)) {
	return gain;
}

/*
 * We are using the linear adaptation of the spec's
 * formula, because we are dealing with all linear
 * gains at this point:
 *
 * G = GAIN * REF / ( REF + ROLLOFF * ( dist - REF ) );
 */
ALfloat _alDistanceInverse( ALfloat dist,
			    ALfloat rolloff,
			    ALfloat gain,
			    ALfloat ref,
			    UNUSED(ALfloat max)) {
	ALfloat retval;

	/* dist = MAX( dist, ref ) */
	if( dist < ref ) {
		dist = ref;
	}

	/* formula, expressed in linear terms */
	retval = gain * ref / ( ref + rolloff * ( dist - ref ) );

	if( retval < 0.0 ) {
		return 0.0;
	} else if( retval > 1.0 ) {
		return 1.0;
	}

	return retval;
}

ALfloat _alDistanceInverseClamped( ALfloat dist,
				   ALfloat rolloff,
				   ALfloat gain,
				   ALfloat ref,
				   ALfloat max ) {
	ALfloat retval;

	/* dist = MAX( dist, ref ) */
	if( dist < ref ) {
		dist = ref;
	}

	/* dist = MIN( dist, max ) */
	if( dist > max ) {
		dist = max;
	}

	/* linear gain formula */
	retval = gain * ref / ( ref + rolloff * ( dist - ref ) );

	return retval;
}

/*
 * _alDistanceModel( ALenum distanceModel )
 *
 * Non locking version of alDistanceModel.
 */
void _alDistanceModel( ALenum distanceModel ) {
	AL_context *cc;

	cc = _alcDCGetContext();
	if( cc == NULL ) {
		/* no current context, set error and return */
		_alcSetError( ALC_INVALID_CONTEXT );

		return;
	}

	switch( distanceModel ) {
		case AL_NONE:
			cc->distance_model = AL_NONE;
			cc->distance_func = _alDistanceNone;
			break;
		case AL_INVERSE_DISTANCE:
			cc->distance_model = AL_INVERSE_DISTANCE;
			cc->distance_func = _alDistanceInverse;
			break;
		case AL_INVERSE_DISTANCE_CLAMPED:
			cc->distance_model = AL_INVERSE_DISTANCE_CLAMPED;
			cc->distance_func = _alDistanceInverseClamped;
			break;
		default:
			_alDCSetError( AL_ILLEGAL_ENUM );

			break;
		}

	return;
}
