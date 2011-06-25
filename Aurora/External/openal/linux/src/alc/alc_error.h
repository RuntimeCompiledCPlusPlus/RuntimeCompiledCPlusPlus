/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * alc_error.h
 *
 * openal alc error reporting.
 */
#ifndef _LAL_ALC_ERROR_H_
#define _LAL_ALC_ERROR_H_

#include "AL/altypes.h"
#include "AL/alctypes.h"

/*
 * alcIsError( ALCenum param )
 *
 * Returns AL_TRUE if param specifies a valid alc error, AL_FALSE otherwise.
 */
ALboolean alcIsError( ALCenum param );

/*
 * _alcSetError( ALCenum param )
 *
 * Sets the context-independant error to param, if param is a valid context
 * error.
 */
void _alcSetError( ALCenum param );

/*
 * _alcGetErrorString( ALCenum param )
 *
 * Returns a const ALubyte * string giving a readable representation of the
 * error param, or NULL if param is not an alc error.
 */
const ALubyte *_alcGetErrorString( ALCenum param );

#endif /* _LAL_ALC_ERROR_H_ */
