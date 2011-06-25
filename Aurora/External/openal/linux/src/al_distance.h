/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_distance.h
 *
 * Prototypes, macros and definitions related to the distance model.
 *
 */
#ifndef AL_DISTANCE_H_
#define AL_DISTANCE_H_

#include <AL/altypes.h>

/*
 * _alDistanceModel( ALenum distanceModel )
 *
 * Non-locking version of alDistanceModel.
 */
void _alDistanceModel( ALenum distanceModel );

/*
 * Distance model functions
 *
 * Distance model functions are used to compute a clamped multiplier for the
 * purposes of gain manipulation.  In this manner, they do distance
 * attenuation.
 *
 * The specific mapping of parameters to return value is dependant on the
 * model chosen.  The default Distance model is _alDistanceInverse, which
 * losely models the real world sound propagation of 1/r^2.
 */

/*
 * _alDistanceNone( ALfloat gain, ALfloat rolloff,
 *                  ALfloat dist, ALfloat ref, ALfloat max )
 *
 * _alDistanceNone performs no distance attenuation.  The return value is
 * always gain.
 */
ALfloat _alDistanceNone( ALfloat gain, ALfloat rolloff,
			 ALfloat dist, ALfloat ref, ALfloat max );

/*
 * _alDistanceInverse( ALfloat gain, ALfloat rolloff,
 *                     ALfloat dist, ALfloat ref, ALfloat max )
 *
 * _alDistanceInverse is the default distance model.  The return value is
 * given by the formula:
 *
 * retval = gain * ref / ( ref + rolloff * ( dist - ref ) )
 */
ALfloat _alDistanceInverse( ALfloat gain, ALfloat rolloff,
			    ALfloat dist, ALfloat ref, ALfloat max );

/*
 * _alDistanceInverseClamped( ALfloat gain, ALfloat rolloff,
 *                            ALfloat dist, ALfloat ref, ALfloat max )
 *
 * _alDistanceInverseClamped is more D3D distance model.  The return value is
 * given by the formula:
 *
 * dist = MAX( dist, ref )
 * dist = MIN( dist, max )
 *
 * retval = gain * ref / ( ref + rolloff * ( dist - ref ) )
 */
ALfloat _alDistanceInverseClamped( ALfloat gain, ALfloat rolloff,
				   ALfloat dist, ALfloat ref, ALfloat max );

#endif /* AL_DISTANCE_H_ */
