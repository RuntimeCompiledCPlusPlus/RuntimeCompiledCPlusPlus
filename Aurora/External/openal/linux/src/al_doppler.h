/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_doppler.h
 *
 * Prototypes, macros and definitions related to the setting of doppler
 * parameters.
 *
 */
#ifndef AL_DOPPLER_H_
#define AL_DOPPLER_H_

#include <AL/altypes.h>

/*
 * _alDopplerFactor( ALfloat value )
 *
 * Non locking version of alDopplerFactor.
 */
void _alDopplerFactor( ALfloat value );

/*
 * _alDopplerVelocity( ALfloat value )
 *
 * Non locking version of alDopplerVelocity.
 */
void _alDopplerVelocity( ALfloat value );

#endif /* AL_DOPPLER_H_ */
