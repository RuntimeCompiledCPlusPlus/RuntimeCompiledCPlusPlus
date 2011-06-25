/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_state.h
 *
 * Prototypes, macros and definitions related to state management.
 */
#ifndef AL_STATE_H_

#include <AL/altypes.h>

/*
 * _alHint( ALenum target, ALenum mode )
 *
 * Non locking version of alHint.
 */
void _alHint( ALenum target, ALenum mode );

/*
 * _alGetBooleanv( ALenum param, ALboolean *bv )
 *
 * Non locking version of alGetBooleanv.
 */
void _alGetBooleanv( ALenum param, ALboolean *bv );

/*
 * _alGetIntegerv( ALenum param, ALint *iv )
 *
 * Non locking version of alGetIntegerv.
 */
void _alGetIntegerv( ALenum param, ALint *iv );

/*
 * _alGetDoublev( ALenum param, ALdouble *dv )
 *
 * Non locking version of alGetDoublev.
 */
void _alGetDoublev( ALenum param, ALdouble *dv );

/*
 * _alGetFloatv( ALenum param, ALfloat *fv )
 *
 * Non locking version of alGetFloatv.
 */
void _alGetFloatv( ALenum param, ALfloat *fv );

#endif /* AL_STATE_H_ */
