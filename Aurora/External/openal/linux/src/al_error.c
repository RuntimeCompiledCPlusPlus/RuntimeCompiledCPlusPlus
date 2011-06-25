/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_error.c
 *
 * openal error reporting.
 *
 */
#include <AL/al.h>

#include "al_debug.h"
#include "al_types.h"
#include "al_error.h"
#include "alc/alc_context.h"
#include "al_siteconfig.h"

#include <signal.h>

#include <stdio.h>

/*
 * _alShouldBombOnError_LOKI controls whether or not _alSetError should
 * abort when setting an error.  This allows applications to get immediate
 * error reporting.
 */
ALboolean _alShouldBombOnError_LOKI = AL_FALSE;

/*
 * _alErrorStr holds string representations of al errors.
 *
 * 0 -> AL_NO_ERROR
 * 1 -> AL_INVALID_NAME
 * 2 -> AL_ILLEGAL_ENUM
 * 3 -> AL_INVALID_VALUE
 * 4 -> AL_ILLEGAL_COMMAND
 * 5 -> AL_OUT_OF_MEMORY
 */
static const char *_alErrorStr[] = {
	"No error.",
	"Invalid Name parameter",
	"Illegal paramater",
	"Invalid enum parameter value",
	"Illegal call",
	"Unable to allocate memory"
};

/*
 * index2ErrorNo( int index )
 *
 * Returns an al error from a simple index.
 */
static ALenum index2ErrorNo( int index );

/*
 * ErrorNo2index( ALenum error_number )
 *
 * Returns a simple index from an al error.
 */
static int ErrorNo2index( ALenum error_number );

/**
 * Error support.
 */

/*
 * alGetError( void )
 *
 * Returns the most recent error generated in the AL state machine.
 */
ALenum alGetError( void ) {
	AL_context *cc;
	int index;

	_alcDCLockContext();

	cc = _alcDCGetContext();

	if(cc == NULL)
		return 0;

	index = index2ErrorNo(cc->alErrorIndex);

	/*
	 * In deference to the new spec, GetError clears the error
	 * after reading it.
	 */
	cc->alErrorIndex = 0;
	
	_alcDCUnlockContext();

	return index;
}

/*
 * ErrorNo2index( ALenum error_number )
 *
 * Returns a simple index from an al error.
 */
static int ErrorNo2index( ALenum error_number ) {
	switch( error_number ) {
		case AL_NO_ERROR:
			return 0;
			break;
		case AL_INVALID_NAME:
			return 1;
			break;
		case AL_ILLEGAL_ENUM:
			return 2;
			break;
		case AL_INVALID_VALUE:
			return 3;
			break;
		case AL_ILLEGAL_COMMAND:
			return 4;
			break;
		case AL_OUT_OF_MEMORY:
			return 5;
			break;
		default:
			_alDebug( ALD_ERROR, __FILE__, __LINE__,
				"Unknown error condition: 0x%x", error_number );
			return -1;
			break;
	}

	return -1;
}

/*
 * index2ErrorNo( int index )
 *
 * Returns an al error from a simple index.
 */
static ALenum index2ErrorNo( int index ) {
	switch( index ) {
 		case 0:
			return AL_NO_ERROR;
			break;
		case 1:
			return AL_INVALID_NAME;
			break;
		case 2:
			return AL_ILLEGAL_ENUM;
			break;
		case 3:
			return AL_INVALID_VALUE;
			break;
		case 4:
			return AL_ILLEGAL_COMMAND;
			break;
		case 5:
			return AL_OUT_OF_MEMORY;
			break;
		default:
			_alDebug( ALD_ERROR, __FILE__, __LINE__,
				"Unknown index : %d", index );
			break;
	}

	return -1;
}

/*
 * _alSetError( ALuint cid, ALenum param )
 *
 * Sets the error for the context with name cid to param.
 *
 * assumes locked context
 */
void _alSetError( ALuint cid, ALenum param ) {
	AL_context *cc;

	cc = _alcGetContext( cid );
	if(cc == NULL) {
		/* No default context, no error set. */
		return;
	}

	if( cc->alErrorIndex == 0 ) {
		/*
		 * Only set error if no previous error has been recorded.
		 */
		
		cc->alErrorIndex = ErrorNo2index(param);
	}
		
	if(_alShouldBombOnError_LOKI == AL_TRUE) {
		raise(SIGABRT);
	}

	return;
}

/*
 * _alIsError( ALenum param )
 *
 * _alIsError returns AL_TRUE if the passed param is an error message,
 * AL_FALSE otherwise.
 * 
 */
ALboolean _alIsError( ALenum param ) {
	switch( param ) {
		case AL_NO_ERROR:
		case AL_INVALID_NAME:
		case AL_ILLEGAL_ENUM:
		case AL_INVALID_VALUE:
		case AL_ILLEGAL_COMMAND:
		case AL_OUT_OF_MEMORY:
			return AL_TRUE;
		default:
			return AL_FALSE;
	}

	return AL_FALSE;
}

/*
 * _alGetErrorString( ALenum param )
 *
 * This function returns the string corresponding to the
 * error in question.  It doesn't validate that the passed
 * param, so calling functions should ensure that _alIsError(param)
 * return AL_TRUE before passing it to this function.
 */
const ALubyte *_alGetErrorString( ALenum param ) {
	int offset;

	offset = ErrorNo2index( param );

	return (const ALubyte *) _alErrorStr[offset];
}
