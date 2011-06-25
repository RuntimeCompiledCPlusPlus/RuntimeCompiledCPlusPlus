/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_state.c
 *
 * State management.  Mainly stubbed.
 *
 */
#include "al_siteconfig.h"

#include <stdio.h>

#include <AL/al.h>
#include <AL/alc.h>

#include "al_types.h"
#include "al_error.h"
#include "al_main.h"
#include "al_state.h"
#include "al_ext.h"

#include "alc/alc_error.h"

/** State retrieval. */

/*
 * alGetFloatv( ALenum param, ALfloat *fv )
 *
 * Populated fv with the ALfloat representation for param.
 */
void alGetFloatv( ALenum param, ALfloat *fv ) {
	_alcDCLockContext();

	_alGetFloatv( param, fv );

	_alcDCUnlockContext();

	return;
}

/*
 * alGetBooleanv( ALenum param, ALboolean *bv )
 *
 * Populated bv with the ALboolean representation for param.
 */
void alGetBooleanv( ALenum param, ALboolean *bv ) {
	_alcDCLockContext();

	_alGetBooleanv( param, bv );

	_alcDCUnlockContext();

	return;
}

/*
 * alGetIntegerv( ALenum param, ALint *iv )
 *
 * Populated iv with the ALint representation for param.
 */
void alGetIntegerv( ALenum param, ALint *iv ) {
	_alcDCLockContext();

	_alGetIntegerv( param, iv );

	_alcDCUnlockContext();

	return;
}

/*
 * alGetDoublev(ALenum param, ALdouble *dv )
 *
 * Populated dv with the double representation for param.
 */
void alGetDoublev(ALenum param, ALdouble *dv ) {
	_alcDCLockContext();

	_alGetDoublev( param, dv );

	_alcDCUnlockContext();

	return;
}

/*
 * _alGetFloatv( ALenum param, ALfloat *fv )
 *
 * Non locking version of alGetFloatv.
 */
void _alGetFloatv( ALenum param, ALfloat *fv ) {
	AL_context *cc;

	cc = _alcDCGetContext();
	if( cc == NULL ) {
		/* no current context, set error and return */
		_alcSetError( ALC_INVALID_CONTEXT );

		return;
	}

	switch( param ) {
		case AL_DOPPLER_FACTOR:
			*fv = cc->doppler_factor;
			break;
		case AL_DOPPLER_VELOCITY:
			*fv = cc->doppler_velocity;
			break;
		default:
			_alDCSetError( AL_ILLEGAL_ENUM );
			break;
	}

	return;
}

/*
 * _alGetBooleanv( ALenum param, ALboolean *bv)
 *
 * Non locking version of alGetBooleanv.
 */
void _alGetBooleanv(UNUSED(ALenum param), UNUSED(ALboolean *bv)) {
	_alStub("alGetBooleanv");

	/* FIXME: don't set error if no current context */
	_alDCSetError( AL_ILLEGAL_ENUM );

	return;
}

/*
 * _alGetIntegerv( ALenum param, ALint *iv )
 *
 * Non locking version of alGetIntegerv
 */
void _alGetIntegerv(UNUSED(ALenum param), UNUSED(ALint *iv)) {
	AL_context *cc;

	cc = _alcDCGetContext();
	if( cc == NULL ) {
		/* no current context, set error and return */
		_alcSetError( ALC_INVALID_CONTEXT );

		return;
	}

	switch( param ) {
		case AL_DISTANCE_MODEL:
			*iv = cc->distance_model;
			break;
		default:
			_alDCSetError( AL_ILLEGAL_ENUM );
			break;
	}

	return;
}

/*
 * _alGetDoublev( ALenum param, ALdouble *dv )
 *
 * Non locking version of alGetDoublev
 *
 */
void _alGetDoublev(UNUSED(ALenum param), UNUSED(ALdouble *dv)) {
	_alStub("alGetDoublev");

	/* FIXME: don't set error if no current context */
	_alDCSetError( AL_ILLEGAL_ENUM );

	return;
}

/*
 * alGetString( ALenum param )
 *
 * Returns readable string representation of param, or NULL if no string
 * representation is available.
 */
const ALubyte *alGetString( ALenum param ) {
	static ALubyte extensions[4096];

	/*
	 * First, we check to see if the param corresponds to an
	 * error, in which case we return the value from _alGetErrorString.
	 */
	if(_alIsError(param) == AL_TRUE) {
		return _alGetErrorString(param);
	}

	/*
	 * Next, we check to see if the param corresponds to an alc
	 * error, in which case we return the value from _alcGetErrorString.
	 */
	if( alcIsError( param ) == AL_TRUE ) {
		return _alcGetErrorString( param );
	}

	switch(param) {
		case AL_VENDOR:
			return (const ALubyte *) "Loki(R) Software";
			break;
		case AL_VERSION:
			return (const ALubyte *) LAL_VERSION;
			break;
		case AL_RENDERER:
			return (const ALubyte *) "Software";
			break;
		case AL_EXTENSIONS:
			_alGetExtensionStrings( extensions, sizeof( extensions ) );
			return extensions;
			break;
		case 0xfeedabee:
			return (const ALubyte *) "Mark 12:31";
			break;
		default:
		  break;
	}

	return NULL;
}

/*
 * _alHint( ALenum target, ALenum mode )
 *
 * Provides hint for the implementation.
 */
void alHint( ALenum target, ALenum mode ) {
	_alHint( target, mode );

	return;
}

/*
 * _alHint( ALenum target, ALenum mode )
 *
 * Non-locking version of _alHint.
 */
void _alHint( UNUSED(ALenum target), UNUSED(ALenum mode) ) {
	return;
}
