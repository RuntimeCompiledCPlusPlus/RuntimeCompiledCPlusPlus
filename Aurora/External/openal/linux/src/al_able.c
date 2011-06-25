/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_able.c
 *
 * Handles Enable / Disable stuff.
 */
#include "al_siteconfig.h"

#include <stdio.h>

#include <AL/al.h>
#include <AL/alexttypes.h>

#include "al_able.h"
#include "al_types.h"
#include "al_error.h"
#include "al_main.h"

#include "alc/alc_context.h"

/** Enable/Disable mojo. */

/*
 * alEnable( ALenum param )
 *
 * Enables param if possible for the current context.  If param does not
 * specify a valid enabling token, AL_INVALID_ENUM is set.
 */
void alEnable( ALenum param ) {
	_alcDCLockContext();

	_alEnable( param );

	_alcDCUnlockContext();

	return;
}

/*
 * alDisable( ALenum param )
 *
 * Disables param if possible for the current context.  If param does not
 * specify a valid enabling token, AL_INVALID_ENUM is set.
 */
void alDisable( ALenum param ) {
	_alcDCLockContext();

	_alDisable( param );

	_alcDCUnlockContext();

	return;
}

/*
 * alIsEnabled( ALenum param )
 *
 * returns AL_TRUE if the attribute specified by param is enabled, AL_FALSE
 * otherwise.
 *
 * if param is not a valid enable/disable token, AL_ILLEGAL_ENUM is set.
 */
ALboolean alIsEnabled(ALenum param) {
	ALboolean retval;

	_alcDCLockContext();

	retval = _alIsEnabled( param );

	_alcDCUnlockContext();

	return retval;
}

/*
 * _alIsEnabled( ALenum param )
 *
 * Non locking version of alIsEnabled.
 *
 * assumes locked context
 */
ALboolean _alIsEnabled( ALenum param ) {
	AL_context *cc;

	cc = _alcDCGetContext();
	if( cc == NULL ) {
		return AL_FALSE;
	}

	switch( param ) {
		default:
			_alDCSetError( AL_ILLEGAL_ENUM );
			break;
	}

	return AL_FALSE;
}

/*
 * _alEnable( ALenum param )
 *
 * Enables the attribute specified by param.
 *
 * If param is not a valid attribute, AL_ILLEGAL_ENUM is set.
 *
 * assumes locked context
 */
void _alEnable( ALenum param ) {
	AL_context *cc;

	cc = _alcDCGetContext();
	if(cc == NULL) {
		return;
	}

	switch( param ) {
		default:
			_alDCSetError( AL_ILLEGAL_ENUM );
			break;
	}

	return;
}

/*
 * _alDisable( ALenum param )
 *
 * Disables the attribute specified by param.
 *
 * If param is not a valid attribute, AL_ILLEGAL_ENUM is set.
 *
 * assumes locked context
 */
void _alDisable( ALenum param ) {
	AL_context *cc;

	cc = _alcDCGetContext();
	if(cc == NULL) {
		return;
	}

	switch( param ) {
		default:
			_alDCSetError( AL_ILLEGAL_ENUM );
			break;
	}

	return;
}
