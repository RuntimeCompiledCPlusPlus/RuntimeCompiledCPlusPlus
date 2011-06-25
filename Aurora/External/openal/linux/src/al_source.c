/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_source.c
 *
 * Contains the implementation for source related functions like
 * GenSource, DeleteSource, SourcePlay, etc, as well as internal
 * use functions intended to manage the source structure.
 *
 */
#include "al_siteconfig.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include "al_buffer.h"
#include "al_config.h"
#include "al_debug.h"
#include "al_error.h"
#include "al_main.h"
#include "al_mixer.h"
#include "al_spool.h"
#include "al_source.h"
#include "al_queue.h"
#include "al_types.h"
#include "al_vector.h"
#include "alc/alc_context.h"
#include "alc/alc_speaker.h"

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

/*
 * _alInitSource( ALuint sid )
 *
 * Initialize an already allocated source.
 */
static void _alInitSource( ALuint sid );

/*
 * _alSource2D( AL_source *src )
 *
 * Turns off 3D attributes of a source
 */
static void _alSource2D( AL_source *src );

/*
 * Monoify functions copy an interleaved array of PCM data, usually in LRLR
 * format into seperate dst buffers.
 */
static void _alMonoifyOffset1to2(ALshort **dstref, ALuint offset, ALvoid *src, ALuint ssize);
static void _alMonoifyOffset2to2(ALshort **dstref, ALuint offset, ALvoid *src, ALuint ssize);

/*
 * Channelify functions copy the PCM data from srcs[0..nc-1] into an
 * interleaved destination.  nc ( number of channels) is specified in the
 * function name, ie _alChannelify2 specifies that srcs[0] and srcs[1] contain
 * left and right channel data, respectively.
 */
static void _alChannelify2Offset(ALshort *dst, ALuint offset, ALshort **srcs, ALuint size);

/* static data */
static ALshort *stereoptr = NULL; /*
				   * scratch space for splitting multichannels
				   * sources.
				   */
				   
/*
 * special split source to handle callbacks
 */
static void _alSplitSourceCallback(ALuint cid,
		     ALuint sourceid,
		     ALint nc, ALuint len,
		     AL_buffer *samp,
		     ALshort **buffers);
/*
 * special split source to handle looping end case (wrap-around).
 */
static void _alSplitSourceLooping(ALuint cid,
		     ALuint sourceid,
		     ALint nc, ALuint len,
		     AL_buffer *samp,
		     ALshort **buffers);

/*
 * special split source to handle buffer queue transitions
 * (wrap-around).
 */
static void _alSplitSourceQueue(ALuint cid,
		     ALuint sourceid,
		     ALint nc, ALuint len,
		     AL_buffer *samp,
		     ALshort **buffers);

/*
 * get a channel pointer into the buffer's data, scaled by the source's
 * position into the PCM data.
 */
void *_alSourceGetBufptr(AL_source *src, AL_buffer *buf, ALuint index);

/*
 * alIsSource( ALuint sid )
 *
 * Returns AL_TRUE if sid is a currently valid source id, 
 * AL_FALSE otherwise.
 */
ALboolean alIsSource( ALuint sid ) {
	ALboolean retval = AL_FALSE;

	_alDCLockSource( sid );

	retval = _alIsSource( sid );

	_alDCUnlockSource( sid );

	return retval;
}

/*
 * _alIsSource( ALuint sid )
 *
 * Returns AL_TRUE if sid is a currently valid source id, 
 * AL_FALSE otherwise.
 *
 * Assumes locked source
 */
ALboolean _alIsSource( ALuint sid ) {
	AL_source *src;
	ALboolean retval = AL_TRUE;

	src = _alDCGetSource( sid );
	if(src == NULL) {
		retval = AL_FALSE;
	}

	return retval;
}

/*
 * alGenSources( ALsizei n, ALuint *buffer )
 *
 * Generate n sources, with sids in buffer[0..(n-1)].  Only
 * full allocation is performed, with error being set otherwise.
 *
 * If n is 0, this is a legal nop.  If n < 0, INVALID_VALUE is
 * set and this is a nop.
 */
void alGenSources( ALsizei n, ALuint *buffer ) {
	AL_context *cc;
	ALint sindex;
	ALuint *temp;
	int i;

	if( n == 0 ) {
		/* with n == 0, we NOP */
		return;
	}

	if( n < 0 ) {
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "alGenSources: illegal n value %d\n", n );

		_alDCSetError( AL_INVALID_VALUE );

		return;
	}

	temp = malloc( n * sizeof *temp );
	if(temp == NULL) {
		/*
		 * Could not reserve memory for temporary
		 * ALuint *buffer.
		 */
		_alDCSetError( AL_OUT_OF_MEMORY );

		return;
	}

	_alcDCLockContext();
 	cc = _alcDCGetContext();

	ASSERT( cc );

	for(i = 0; i < n; i++) {
		sindex = spool_alloc( &cc->source_pool );
		if( sindex == -1 ) {
			/* We ran into a problem somewhere in
			 * allocing a set of source ids.  Run
			 * through the ones we did alloc, 
			 * realloc them, set error and return.
			 *
			 * FIXME: Should unlock, or have a non-locking
			 * version of alDelSources called inside
			 * the current lock?
			 */
			_alcDCUnlockContext();

			if(i > 0) {
				/*
				 *  We delete i sources, and not i + 1, because
				 *  the last alloc did not happen.  And only
				 *  if i > 0.
				 */
				alDeleteSources( i, temp );
			}

			free( temp );

			_alDCSetError( AL_OUT_OF_MEMORY );

			return;
		}

		temp[i] = sindex;
		_alInitSource( temp[i] );
	}

	_alcDCUnlockContext();

	/*
	 * temp[0...n-1] now contains the generated buffers.  Copy them
	 * to the user data.
	 */
	memcpy( buffer, temp, n * sizeof *buffer );

	free( temp );

	return;
}

/*
 * _alSource2D( AL_source *src )
 *
 * Turns off 3D attributes of a source
 *
 * Assumes locked source
 */
void _alSource2D( AL_source *src ) {
	_alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
			"_alSource2D: source turned 2D");
		
	src->position.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_POSITION, src->position.data );

	src->direction.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_DIRECTION, src->direction.data );
	
	src->coneinnerangle.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_CONE_INNER_ANGLE, &src->coneinnerangle.data );

	src->coneouterangle.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_CONE_OUTER_ANGLE, &src->coneouterangle.data );

	src->coneoutergain.isset = AL_FALSE;
}

/*
 * alSourcei( ALuint sid, ALenum param, ALint i1 )
 *
 * Sets an attribute for a source.
 *
 * If sid does not name a valid source, AL_INVALID_NAME.
 * If param does not specify a source attribute, AL_ILLEGAL_ENUM.
 * If i1 is out of range for the attribute, AL_INVALID_VALUE.
 *
 * If param is AL_BUFFER, and the source in question is not in the state
 * AL_INITIAL or AL_STOPPED, AL_ILLEGAL_COMMAND.
 *
 */
void alSourcei( ALuint sid, ALenum param, ALint i1 ) {
	AL_source *src;
	AL_sourcestate *srcstate;
	ALboolean inrange = AL_TRUE;
	ALfloat temp;

	/*
	 * If param refers to a integer attribute, accept it.
	 * If it refers to a float attribute, delegate it to alSourcefv.  If
	 * it is an invalid attribute, set ILLEGAL_ENUM and bail out.
	 *
	 * Float vectors are obviously not possible, so we set
	 * ILLEGAL_COMMAND.  This is probably a spec violation.
	 */
	switch( param ) {
		case AL_BUFFER:
		case AL_LOOPING:
		case AL_SOURCE_RELATIVE:
		case AL_STREAMING:
			/*
			 * We handle these natively.
			 */
			break;
		case AL_PITCH:
		case AL_MIN_GAIN:
		case AL_MAX_GAIN:
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		case AL_CONE_OUTER_GAIN:
		case AL_GAIN:
		case AL_GAIN_LINEAR_LOKI:
		case AL_REFERENCE_DISTANCE:
		case AL_ROLLOFF_FACTOR:
		case AL_MAX_DISTANCE:
			temp = i1;

			alSourcef( sid, param, temp );

			return;
			break;
		case AL_POSITION:
		case AL_VELOCITY:
		case AL_DIRECTION:
		default:
			_alcDCLockContext();
			_alDCSetError( AL_ILLEGAL_ENUM );
			_alcDCUnlockContext();

			return;
			break;
	}

	SOURCELOCK();

	src = _alDCGetSource( sid );
	if(src == NULL) {
		/*
		 * sid is invalid.
		 */
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
			"alSourcei: source id %d is not valid", sid);
		
		_alDCSetError( AL_INVALID_NAME );
		
		SOURCEUNLOCK();
		return;
	}

	/*
	 * all calls to alSourcei specify ALboolean parameters,
	 * which means that i1 is either AL_TRUE, AL_FALSE, or
	 * not valid.  So check for validity of i1 first, and
	 * set error if that's the case.
	 */
	switch( param ) {
		case AL_LOOPING:
		case AL_STREAMING:
		case AL_SOURCE_RELATIVE:
			inrange = _alCheckRangeb( i1 );
			break;
		case AL_BUFFER:
			inrange = alIsBuffer( i1 );

			if( i1 == 0 ) {
				/*
				 * bid of 0 has a special meaning: unset the
				 * parameter.	So it's in range.
				 */
				inrange = AL_TRUE;
			}
			break;
		default:
			/* invalid param, error below. */
			break;
	}

	if( inrange == AL_FALSE ) {
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "alSourcei(%d, 0x%x, ...) called with invalid value %d",
		      sid, param, i1);

		_alDCSetError( AL_INVALID_VALUE );
		
		SOURCEUNLOCK();
		return;
	}

	srcstate = _alSourceQueueGetCurrentState(src);
	ASSERT( srcstate );

	switch(param) {
		case AL_BUFFER:
			switch( src->state ) {
				case AL_PLAYING:
				case AL_PAUSED:
					/*
					 * Invalid state to set buffer in.
					 */
					_alDebug( ALD_SOURCE, __FILE__, __LINE__,
						"alSourcei(%d): source is playing, AL_BUFFER invalid",
						sid );
					_alDCSetError( AL_ILLEGAL_COMMAND );
					break;
				default:
					_alSourceQueueHead( src, i1 );
					break;
			}

			break;
		case AL_LOOPING:
			srcstate->islooping.isset = AL_TRUE;
			srcstate->islooping.data = i1;
			break;
		case AL_SOURCE_RELATIVE:
			src->isrelative.isset = AL_TRUE;
			src->isrelative.data = i1;
			/* If this is a relative source with zero position,
			   perform the 2D sound hack...
			   (Tribes 2 reuses 3D sources as 2D sources)
			*/
		  	if ( i1 && src->position.isset &&
			     ! src->position.data[0] &&
			     ! src->position.data[1] &&
			     ! src->position.data[2] ) {
				_alSource2D(src);
			}
			break;
		case AL_STREAMING:
			src->isstreaming.isset = AL_TRUE;
			src->isstreaming.data = i1;
			break;
		default:
			_alDebug(ALD_SOURCE, __FILE__, __LINE__,
				"alSourcei: invalid or stubbed source param 0x%x",
				param );
			
			_alDCSetError( AL_ILLEGAL_ENUM );
			break;
	}

	SOURCEUNLOCK();

	return;
}

/*
 * alSourcef( ALuint sid, ALenum param, ALfloat f1 )
 *
 * Sets an attribute for a source.
 *
 * If sid does not name a valid source, AL_INVALID_NAME.
 * If param does not specify a source attribute, AL_ILLEGAL_ENUM.
 * If f1 is out of range for the attribute, AL_INVALID_VALUE.
 *
 */
void alSourcef( ALuint sid, ALenum param, ALfloat f1 ) {
	/*
	 * If param refers to a integer attribute, delegate it to alSourcei.
	 * If it refers to a float attribute, delegate it to alSourcefv.  If
	 * it is an invalid attribute, set ILLEGAL_ENUM and bail out.
	 *
	 * Float vectors are obviously not possible, so we set
	 * ILLEGAL_COMMAND.  This is probably a spec violation.
	 */
	switch( param ) {
		case AL_BUFFER:
		case AL_LOOPING:
		case AL_SOURCE_RELATIVE:
		case AL_STREAMING:
			alSourcei( sid, param, (ALint) f1 );

			return;
			break;
		case AL_PITCH:
		case AL_MIN_GAIN:
		case AL_MAX_GAIN:
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		case AL_CONE_OUTER_GAIN:
		case AL_GAIN:
		case AL_GAIN_LINEAR_LOKI:
		case AL_REFERENCE_DISTANCE:
		case AL_ROLLOFF_FACTOR:
		case AL_MAX_DISTANCE:
			alSourcefv( sid, param, &f1 );

			return;
			break;
		case AL_POSITION:
		case AL_VELOCITY:
		case AL_DIRECTION:
		default:
			_alcDCLockContext();
			_alDCSetError( AL_ILLEGAL_ENUM );
			_alcDCUnlockContext();

			return;
			break;
	}

	return;
}

/*
 * alSource3f( ALuint sid, ALenum param, ALfloat f1, ALfloat f2, ALfloat f3 )
 *
 * Sets an 3 float parameter for a source.
 *
 * If sid does not name a valid source, AL_INVALID_NAME.
 * If param does not specify a source attribute, AL_ILLEGAL_ENUM.
 * If f1, f2, or f3 is out of range for the attribute, AL_INVALID_VALUE.
 *
 */
void alSource3f( ALuint sid, ALenum param,
		 ALfloat f1, ALfloat f2, ALfloat f3 ) {
	ALfloat fv[3];

	fv[0] = f1;
	fv[1] = f2;
	fv[2] = f3;

	alSourcefv( sid, param, fv );

	return;
}

/*
 * alSourcefv( ALuint sid, ALenum param, ALfloat fv )
 *
 * Sets an float vector parameter for a source.
 *
 * If sid does not name a valid source, AL_INVALID_NAME.
 * If param does not specify a source attribute, AL_ILLEGAL_ENUM.
 * If any member of fv is out of range for the attribute, AL_INVALID_VALUE.
 *
 */
void alSourcefv( ALuint sid, ALenum param, ALfloat *fv1 ) {
	AL_source *source;
	AL_sourcestate *srcstate;
	ALboolean inrange = AL_TRUE;

	/*
	 * If param refers to a integer attribute, delegate it to alSourcei.
	 * If it refers to a float attribute, we accept it.
	 */
	switch( param ) {
		case AL_BUFFER:
		case AL_LOOPING:
		case AL_SOURCE_RELATIVE:
		case AL_STREAMING:
			alSourcei( sid, param, (ALint) fv1[0] );

			return;
			break;
		case AL_PITCH:
		case AL_MIN_GAIN:
		case AL_MAX_GAIN:
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		case AL_CONE_OUTER_GAIN:
		case AL_GAIN:
		case AL_GAIN_LINEAR_LOKI:
		case AL_REFERENCE_DISTANCE:
		case AL_ROLLOFF_FACTOR:
		case AL_MAX_DISTANCE:
		case AL_POSITION:
		case AL_VELOCITY:
		case AL_DIRECTION:
			break;
		default:
			_alcDCLockContext();
			_alDCSetError( AL_ILLEGAL_ENUM );
			_alcDCUnlockContext();
			return;
			break;
	}
	
	
	SOURCELOCK();
	source = _alDCGetSource( sid );

	if( source == NULL ) {
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
			"alSourcefv: %d is an invalid source id", sid );

		_alDCSetError( AL_INVALID_NAME );

		SOURCEUNLOCK();

		return;
	}

	if( fv1 == NULL ) {
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
			"alSourcefv: passed fv is NULL");

		_alDCSetError( AL_INVALID_VALUE );

		SOURCEUNLOCK();

		return;
	}

	/* check to see if we are in range, first */
	switch( param ) {
		case AL_MIN_GAIN:
		case AL_MAX_GAIN:
		case AL_CONE_OUTER_GAIN:
		case AL_GAIN:
		case AL_GAIN_LINEAR_LOKI:
		  inrange = _alCheckRangef( fv1[0], 0.0, 1.0 );
		  break;
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		  inrange = _alCheckRangef( fv1[0], 0.0, 360.0 );
		  break;
		case AL_PITCH:
		  /* FIXME: deviates from spec */
		  inrange = _alCheckRangef( fv1[0], 0.0, 2.0 );
		  break;
		case AL_REFERENCE_DISTANCE:
		case AL_MAX_DISTANCE:
		case AL_ROLLOFF_FACTOR:
		  inrange = _alCheckRangef( fv1[0], 0.0, FLT_MAX );
		  break;
		case AL_POSITION:
		case AL_DIRECTION:
		  inrange = inrange && ( _alIsFinite( fv1[0] ) == AL_TRUE );
		  inrange = inrange && ( _alIsFinite( fv1[1] ) == AL_TRUE );
		  inrange = inrange && ( _alIsFinite( fv1[2] ) == AL_TRUE );

		  ASSERT( inrange );
		  break;
		default:
		  /* invalid param. error below */
		  break;
	}

	if( inrange == AL_FALSE ) {
		/*
		 *  We have a range error, exit early.
		 */
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "alSourcef(%d, 0x%x, ...): %f out of range",
		      sid, param, fv1[0] );

		_alDCSetError( AL_INVALID_VALUE );

		SOURCEUNLOCK();
		
		return;
	}

	srcstate = _alSourceQueueGetCurrentState( source );
	ASSERT( srcstate );
	
	switch( param ) {
		case AL_POSITION:
		  source->position.isset = AL_TRUE;
		  memcpy( &source->position.data, fv1, SIZEOFVECTOR );
		  /* If this is a relative source with zero position,
		     perform the 2D sound hack...
		     (Tribes 2 reuses 3D sources as 2D sources)
		   */
		  if ( !fv1[0] && !fv1[1] && !fv1[2] &&
		       source->isrelative.isset &&
		       source->isrelative.data ) {
			_alSource2D(source);
		  }
		  break;
		case AL_DIRECTION:
		  /*
		   * The zero vector will make a directional sound
		   * non-directional.
		   */
		  if(_alIsZeroVector(fv1) == AL_TRUE) {
			  /* clear DIRECTION flag */
			  source->direction.isset = AL_FALSE;
		  } else {
			  source->direction.isset = AL_TRUE;
			  memcpy( &source->direction.data, fv1, SIZEOFVECTOR );
		  }
		  break;
		case AL_VELOCITY:
		  source->velocity.isset = AL_TRUE;
		  memcpy( &source->velocity.data, fv1, SIZEOFVECTOR );

		  /*
		   * velocity means doppler, which means that we
		   * need pitch, which means alf_tpitch takes care of
		   * incrementing soundpos.  Very kludgey, I'll admit.
		   *
		   */
		  source->flags      |= ALS_NEEDPITCH;
		  source->pitch.isset = AL_TRUE;
		  break;
		case AL_MIN_GAIN:
		  source->attenuationmin.isset = AL_TRUE;
		  source->attenuationmin.data  = fv1[0];
		  break;
		case AL_MAX_GAIN:
		  source->attenuationmax.isset = AL_TRUE;
		  source->attenuationmax.data  = fv1[0];
		  break;
		case AL_CONE_INNER_ANGLE:
		  source->coneinnerangle.isset = AL_TRUE;
		  source->coneinnerangle.data  = fv1[0];
		  break;
		case AL_CONE_OUTER_ANGLE:
		  source->coneouterangle.isset = AL_TRUE;
		  source->coneouterangle.data  = fv1[0];
		  break;
		case AL_CONE_OUTER_GAIN:
		  source->coneoutergain.isset = AL_TRUE;
		  source->coneoutergain.data  = fv1[0];
		  break;
		case AL_PITCH:
		  /* only set pitch if it differs from 1.0 */
		  if(fv1[0] == 1.0) {
			source->pitch.isset = AL_FALSE;
			source->pitch.data  = 1.0;

			source->flags &= ~ALS_NEEDPITCH;
		  } else {
		  	source->pitch.isset = AL_TRUE;
		  	source->pitch.data  = fv1[0];

			/* tpitch messes with soundpos */
			source->flags |= ALS_NEEDPITCH;
		  }
		  break;
		case AL_GAIN:
		  source->gain.isset = AL_TRUE;
		  source->gain.data = _alDBToLinear( fv1[0] );
		  break;
		case AL_GAIN_LINEAR_LOKI:
		  source->gain.isset = AL_TRUE;
		  source->gain.data = fv1[0];
		  break;
		case AL_REFERENCE_DISTANCE:
		  source->ref_distance.isset = AL_TRUE;
		  source->ref_distance.data = fv1[0];
		  break;
		case AL_MAX_DISTANCE:
		  source->max_distance.isset = AL_TRUE;
		  source->max_distance.data = fv1[0];
		  break;
		case AL_ROLLOFF_FACTOR:
		  source->rolloff_factor.isset = AL_TRUE;
		  source->rolloff_factor.data = fv1[0];
		  break;
		default:
		  _alDebug(ALD_SOURCE, __FILE__, __LINE__,
			  "alSourcefv(%d): param 0x%x not valid", sid, param );
		  
		  _alDCSetError( AL_ILLEGAL_ENUM );
		  break;
	}

	SOURCEUNLOCK();

	return;
}

/*
 * alGetSourceiv( ALuint sid, ALenum pname, ALint *retref )
 *
 * Retrieve the value of a source attribute.
 *
 * If sid does not name a valid source, AL_INVALID_NAME.
 * If param does not specify a source attribute, AL_ILLEGAL_ENUM.
 *
 */
void alGetSourceiv( ALuint sid, ALenum param, ALint *retref ) {
	AL_source *src;
	AL_sourcestate *srcstate;
	ALint *temp;

	/*
	 * Check for invalid type request: eg AL_GAIN.  We need
	 * to convert these.
	 */
	switch( param ) {
		case AL_GAIN:
		case AL_GAIN_LINEAR_LOKI:
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		case AL_CONE_OUTER_GAIN:
		case AL_PITCH:
		case AL_REFERENCE_DISTANCE:
		case AL_ROLLOFF_FACTOR:
		case AL_MAX_DISTANCE:
			/* float conversion */
			do {
				ALfloat ftemp = 0.0f;

				alGetSourcefv( sid, param, &ftemp );

				/* how do we know if we've failed? */
				*retref = ftemp;

				return;
			} while(0);
			break;
		case AL_VELOCITY:
		case AL_POSITION:
		case AL_DIRECTION:
			/* float conversion */
			do {
				ALfloat ftemp[3];

				alGetSourcefv( sid, param, ftemp );

				/* how do we know if we've failed? */
				retref[0] = ftemp[0];
				retref[1] = ftemp[1];
				retref[2] = ftemp[2];

				return;
			} while(0);
			break;
		default:
			/*
			 * either integer, boolean, or invalid
			 * param.
			 */
			break;
	}

	SOURCELOCK();
	src = _alDCGetSource( sid );

	if( src == NULL ) {
		/*
		 * Invalid source id
		 */
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "alGetSourcei: invalid source id %d",
		      sid);

		_alDCSetError( AL_INVALID_NAME );

		SOURCEUNLOCK();
		return;
	}

	if(retref == NULL) {
		/* silently ignore */

		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "alGetSourcei(%d): NULL value", sid);

		SOURCEUNLOCK();
		return;
	}


	/*
	 * get param value, and store it in temp.  We need it
	 * more most, but not all, of the following params enums.
	 */
	temp = _alGetSourceParam( src, param );
	if(temp != NULL) {
		/* If temp is not NULL, then there is a value set,
		 * and we don't have to slog through the defaults
		 * below.
		 */

		switch( param ) {
			case AL_LOOPING:
			case AL_STREAMING:
			case AL_SOURCE_RELATIVE:
				*retref = * (ALboolean *) temp;
				break;
			default:
				*retref = *temp;
				break;
		}

		SOURCEUNLOCK();

		return;
	}

	srcstate = _alSourceQueueGetCurrentState(src);
	ASSERT(srcstate);

	switch(param) {
		case AL_BUFFERS_QUEUED:
		  /* AL_BUFFERS_QUEUED is not setable, and has no default. */
		  *retref = src->bid_queue.size - src->bid_queue.read_index;
		  break;
		case AL_BUFFERS_PROCESSED:
		  /* AL_BUFFERS_PROCESSED is not setable, and has no default. */
		  *retref = src->bid_queue.read_index - 1;
		  break;
		case AL_BYTE_LOKI:
		  /* AL_BYTE_LOKI is not setable, and has no default. */
		  switch(src->state) {
			  case AL_PLAYING:
			  case AL_PAUSED:
			    *retref = src->srcParams.soundpos;
			    break;
			  default:
			    *retref = -1;
			    break;
		  }
		  break;
		case AL_SOURCE_STATE:
		  *retref = src->state;
		  break;
		case AL_LOOPING:
		case AL_STREAMING:
		case AL_SOURCE_RELATIVE:
		case AL_BUFFER:
			/* These all have default states */
			_alSourceGetParamDefault( param, retref );
			break;
		default:
		  _alDebug(ALD_SOURCE, __FILE__, __LINE__,
		        "alGetSourcei: invalid or unsupported param 0x%x",
			param);

		  _alDCSetError( AL_ILLEGAL_ENUM );

		  break;
	}

	SOURCEUNLOCK();

	return;
}

/*
 * alGetSourcefv( ALuint sid, ALenum param, ALfloat *values )
 *
 * Retrieve the value of a source attribute.
 *
 * If sid does not name a valid source, AL_INVALID_NAME.
 * If param does not specify a source attribute, AL_ILLEGAL_ENUM.
 */
void alGetSourcefv( ALuint sid, ALenum param, ALfloat *values ) {
	AL_source *src;
	ALfloat *srcvals;
	ALsizei numvalues; /* number of values to copy */

	/*
	 * Check for invalid type request: eg integer, boolean
	 * or single float types.
	 *
	 * We need to convert these.
	 */
	switch( param ) {
		case AL_GAIN:
		case AL_GAIN_LINEAR_LOKI:
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		case AL_CONE_OUTER_GAIN:
		case AL_PITCH:
		case AL_REFERENCE_DISTANCE:
		case AL_MAX_DISTANCE:
		case AL_ROLLOFF_FACTOR:
			/* scalar param, only copy 1 value */
			numvalues = 1;
			break;
		case AL_LOOPING:
		case AL_STREAMING:
		case AL_SOURCE_RELATIVE:
		case AL_BUFFERS_QUEUED:
		case AL_BUFFERS_PROCESSED:
		case AL_BYTE_LOKI:
		case AL_SOURCE_STATE:
		case AL_BUFFER:
			/* conversion to integer */
			do {
				ALint temp = 0;

				alGetSourceiv( sid, param, &temp );

				/*
				 * how do we know if we've failed?
				 *
				 * only populate first entry
				 */
				*values = temp;

				return;
			} while(0);
			break;
		default:
			/*
			 * either float vector or invalid
			 * param.
			 */
			numvalues = 3;
			break;
	}

	SOURCELOCK();
	src = _alDCGetSource( sid );

	if(src == NULL) {
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
			"alGetSourcefv: source id %d is invalid", sid);
		
		_alDCSetError( AL_INVALID_NAME );

		SOURCEUNLOCK();
		return;
	}

	if( values == NULL ) {
		/* silently ignore */

		_alDebug( ALD_SOURCE, __FILE__, __LINE__,
			"alGetSourcefv: values passed is NULL", values );
		
		SOURCEUNLOCK();

		return;
	}

	srcvals = _alGetSourceParam( src, param );
	if(srcvals != NULL) {
		/*
		 * If srcvals is not NULL, then the attribute has been set by
		 * the application, and we don't need to set defaults.
		 */
		memcpy( values, srcvals, numvalues * sizeof *values );

		SOURCEUNLOCK();

		return;
	}

	/* If we are at this point, srcvals is NULL, which means
	 * that either param is an invalid param, or that the value
	 * is not set.	Check for a valid param, in which case we
	 * set the default, or set error.
	 */
	switch( param ) {
		/* scalars */
		case AL_GAIN:
			/*
			 * AL_GAIN reflects the dB of AL_GAIN_LINEAR_LOKI, so
			 * we compute this from AL_GAIN_LINEAR_LOKI.
			 */
			srcvals = _alGetSourceParam( src, AL_GAIN_LINEAR_LOKI );
			if( srcvals != NULL) {
			/*
			 * If srcval is not NULL, the param in question has
			 * been explicitly set, which means that we don't
			 * need to set it to defaults below.
			 */
			values[0] = _alLinearToDB( *srcvals );

			SOURCEUNLOCK();
			return;
			}

			/* AL_GAIN_LINEAR_LOKI was not set, so we set default */
			_alSourceGetParamDefault( AL_GAIN_LINEAR_LOKI, values );
			values[0] = _alLinearToDB( values[0] );
			break;
		case AL_GAIN_LINEAR_LOKI:
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		case AL_CONE_OUTER_GAIN:
		case AL_PITCH:
		case AL_ROLLOFF_FACTOR:
		case AL_REFERENCE_DISTANCE:
		case AL_MAX_DISTANCE:
		case AL_VELOCITY:
		case AL_POSITION:
		case AL_DIRECTION:
			_alSourceGetParamDefault( param, values );
			break;
		default:
			_alDebug( ALD_SOURCE, __FILE__, __LINE__,
			       "alGetSourcefv: param 0x%x either invalid or unset",
			       param);
			break;
	}

	_alDCSetError( AL_ILLEGAL_ENUM );

	SOURCEUNLOCK();

	return;
}

/*
 * _alGetSource( ALuint cid, ALuint sid )
 *
 * Returns the address of the source sid from the 
 * context cid, or NULL if the cid or sid is 
 * invalid.
 *
 * Assumes locked context
 */
AL_source *_alGetSource( ALuint cid, ALuint sid ) {
	AL_context *cc;
	
	cc  = _alcGetContext( cid );
	if(cc == NULL) {
		/*
		 * FIXME: Where, if at all, should be set the error?
		 */
		return NULL;
	}

	return spool_index( &cc->source_pool, sid );
}

/*
 * _alSplitSources( ALuint cid, ALuint sid,
 *                  ALint nc, ALuint len,
 *                  AL_buffer *buf, ALshort **buffs );
 *
 * Populate buffs[0..nc-1][0..len/2-1] with data from buf, with offset into
 * buf given by the position associated with the source named by sid in the
 * context named by cid.
 *
 * This function delegates to static functions in the case of looping,
 * callback or queue sounds.
 *
 * assumes locked context
 *
 * FIXME: what an ugly mess
 */
void _alSplitSources( ALuint cid,
		      ALuint sourceid,
		      ALint nc, ALuint len,
		      AL_buffer *samp,
		      ALshort **buffers ) {
	AL_source *src;
	AL_sourcestate *srcstate;
	ALuint i;
	char *bufptr = NULL;
	static ALuint buflen = 0;

	src = _alGetSource( cid, sourceid );
	if(src == NULL) {
		/* bad mojo */
		return;
	}

	/*
	 * if buflen is less that the passed len, this is probably
	 * the first initialization and we need to allocate the
	 * monoptr (monoptr being that buffer where we mix this
	 * sort of thing.
	 */
	if(buflen < len) {
		buflen = len;

		stereoptr = realloc(stereoptr, buflen * 2);

		memset( stereoptr, 0, buflen * 2 );
	}

	if(stereoptr == NULL) {
		/* at this point, we're dead and don't know it. */
		return;
	}

	/* Shouldn't happen. */
	if(len == 0) {
		_alDebug(ALD_SOURCE,
			__FILE__, __LINE__,
			"wtf? size = 0!!!!!!");
		_alDebug(ALD_SOURCE,
			__FILE__, __LINE__,
			"Expect SIGSEGV soon");
		return;
	}


	srcstate = _alSourceQueueGetCurrentState( src );

	/*
	 *  If we have a callback function, read from it.
	 */
	if(samp->flags & ALB_CALLBACK) {
		srcstate->flags |= ALQ_CALLBACKBUFFER;

		_alSplitSourceCallback(cid, sourceid, nc, len, samp, buffers);
		return;
	} else {
		/* make sure we mark this as a normal buffer */
		srcstate->flags &= ~ALQ_CALLBACKBUFFER;
	}

	if(_alSourceBytesLeftByChannel(src, samp) < (ALint) len) {
		if(_alSourceIsLooping(src) == AL_TRUE ) {
			/*
			 * looping sources, when they need to wrap,
			 * are handled via SplitSourceLooping.
			 */
			_alSplitSourceLooping(cid, sourceid,
		     			nc, len,
		     			samp,
		     			buffers);

			return;
		}

		if(_alSourceGetPendingBids(src) > 0) {
			/*
			 * There are more buffers in the queue, so
			 * do the wrapping.
			 */
			_alSplitSourceQueue(cid, sourceid, nc, len, samp, buffers);

			return;
		}

		len = _alSourceBytesLeftByChannel(src, samp);

		if((len <= 0) || (len > samp->size)) {
			/* really short sound */
			len = samp->size;

			return;
		}
	}
	
	for(i = 0; i < _alcGetNumSpeakers(cid); i++) {
		bufptr = _alSourceGetBufptr(src, samp, i);

		memcpy(buffers[i], bufptr, len);
	}

	return;
}

/*
 * _alSplitSourceCallback( ALuint cid,
 *                         ALuint sourceid,
 *                         ALint nc, ALuint len,
 *                         AL_buffer *samp,
 *                         ALshort **buffers )
 *
 *  Special SplitSource for callback sources, which need to have their
 *  data populated not from the buffer's original data, but from the
 *  callback.
 */
static void _alSplitSourceCallback( ALuint cid,
				    ALuint sourceid,
				    ALint nc, ALuint len,
				    AL_buffer *samp,
				    ALshort **buffers ) {
	AL_source *src;
	int *bid            = NULL;
	ALuint nsamps = 0;
	int resultsamps     = -1;
	int bufchannels     = _al_ALCHANNELS( samp->format );

	src = _alGetSource( cid, sourceid );
	if(src == NULL) {
		/*
		 * Should we really be setting the error here?
		 */
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "_alSplitSourceCallback: invalid source id %d",
		      sourceid);

		_alSetError(cid, AL_INVALID_NAME);
		return;
	}

 	bid = (int *) _alGetSourceParam(src, AL_BUFFER);
	if(bid == NULL) {
		return;
	}

	nsamps = bufchannels * len / sizeof **buffers;

	resultsamps = samp->callback(sourceid, *bid,
				     stereoptr,
				     samp->format,
				     samp->freq,
				     nsamps);

	if(resultsamps < 0) {
		/* callback problem */
		_alDebug(ALD_STREAMING, __FILE__, __LINE__,
			"%d callback returned -1", sourceid);

		memset(stereoptr, 0, len);

		_alRemoveSourceFromMixer(sourceid);

		return;
	}

	if(resultsamps < (ALint) nsamps) {
		/* source is over */
		_alDebug(ALD_STREAMING, __FILE__, __LINE__,
			"time for %d to die", sourceid);

		/* FIXME:
		 *
		 * offset memset at resultsamps / width to end
		 
		 memset(stereoptr, 0, len);
		 */

		/*
		 * we want this to end.  please.
		 * What a cheat.
		 */

		src->srcParams.soundpos = samp->size + nc * resultsamps * sizeof **buffers;
	}

	/* set len to the number of bytes we actually got back
	 * from the callback.
	 */
	len = resultsamps * sizeof **buffers / bufchannels;

	/*
	 * since we're decoding it, copy it to the orig_buffers so we only
	 * have to do it once.
	 */
	_alMonoify(buffers, stereoptr, len, samp->num_buffers, bufchannels);

	samp->size += nc * resultsamps * sizeof **buffers;

	return;
}

/*
 * _alSplitSourceLooping( ALuint cid,
 *                        ALuint sourceid,
 *                        ALint nc, ALuint len,
 *                        AL_buffer *samp,
 *                        ALshort **buffers )
 *
 * _alSplitSourceLooping is called to split a looping source that
 * has reached the loop point (ie, point where it needs to wrap around.
 *
 * This is very ugly, and needs to be cleaned up.  I'd prefer not to
 * have a special case so if you're looking to contribute, please
 * consider redoing this.
 *
 * assumes locked context
 *
 * FIXME: this is so ugly.
 */
static void _alSplitSourceLooping( ALuint cid,
				   ALuint sourceid,
				   ALint nc, ALuint len,
				   AL_buffer *samp,
				   ALshort **buffers ) {
	AL_source *src;
	long mixable;
	long remaining;
	char *bufptr;
	int i;
	int bi;
	int bufchannels = _al_ALCHANNELS(samp->format);
	char *mdp;

	src = _alGetSource(cid, sourceid);
	if(src == NULL) {
		/*
		 * Should we really be setting the error here?
		 */
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "_alSplitSourceLooping: invalid source id %d",
		      sourceid);

		_alSetError(cid, AL_INVALID_NAME);
		return;
	}

	mixable    = _alSourceBytesLeftByChannel(src, samp);
	remaining  = 0;

	ASSERT(mixable >= 0);

	/* in case samp->size < len, we don't want
	 * to overwrite with the memcpy
	 */
	if(len * bufchannels <= samp->size) {
		/* normal case */
		remaining = (len * bufchannels) - mixable;

		for(i = 0; i < nc; i++) {
			bufptr = _alSourceGetBufptr(src, samp, i);

			memcpy(buffers[i], bufptr, mixable);
			memcpy(buffers[i] + mixable/2, samp->orig_buffers[i],
				remaining);
		}

		return;
	} else {
		/* really small looping sample */
		if(mixable < 0) {
			/* we loop sound in monoptr */
			mixable = src->srcParams.soundpos % len;
		}

		for(bi = 0; bi < nc; bi++) {
			mdp = (char *) buffers[bi];

			/* copy samp again and again */
			for(i = mixable; i < (signed int) len; i += samp->size) {
				int copylen;
				
				if(i + samp->size < len) {
					copylen = samp->size;
				} else {
					copylen = len - i;
				}

				memcpy(&mdp[i],	samp->orig_buffers[bi], copylen);
			}

			for(i = 0; i < mixable; i += samp->size) {
				int copylen;
				
				if(i + samp->size < (unsigned int) mixable) {
					copylen = samp->size;
				} else {
					copylen = mixable - i;
				}

				memcpy(&mdp[i],
					samp->orig_buffers[bi],
					copylen);
			}
		}

		return;

		_alDebug(ALD_LOOP, __FILE__, __LINE__, "handle size");
	}

	for(i = 0; i < nc; i++) {
		bufptr = _alSourceGetBufptr(src, samp, i);

		memcpy(buffers[i], bufptr, len);
	}

	return;
}

/*
 * _alSplitSourceQueue( ALuint cid,
 *                      ALuint sourceid,
 *                      ALint nc, ALuint len,
 *                      AL_buffer *samp,
 *                      ALshort **buffers )
 *
 * _alSplitSourceQueue is called to ease the transition between
 * buffers in a source's queue.
 *
 * This is very ugly, and needs to be cleaned up.  I'd prefer not to
 * have a special case so if you're looking to contribute, please
 * consider redoing this.
 *
 * assumes locked context
 *
 * FIXME: this is so ugly.
 */
static void _alSplitSourceQueue( ALuint cid,
				 ALuint sourceid,
				 ALint nc, ALuint len,
				 AL_buffer *samp,
				 ALshort **buffers ) {
	AL_source *src;
	long mixable;
	long remaining;
	char *bufptr;
	int i;
	int bufchannels = _al_ALCHANNELS(samp->format);
	ALuint nextbid;
	AL_buffer *nextsamp;
	void *nextpcm;

	_alDebug(ALD_QUEUE, __FILE__, __LINE__, "_alSplitSourceQueue: foo");
		     
	src = _alGetSource(cid, sourceid);
	if(src == NULL) {
		/*
		 * Should we really be setting the error here?
		 */
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "_alSplitSourceQueue: invalid source id %d",
		      sourceid);

		_alSetError(cid, AL_INVALID_NAME);
		return;
	}

	nextbid = src->bid_queue.queue[src->bid_queue.read_index + 1];

	nextsamp = _alGetBuffer(nextbid);
	if(nextsamp == NULL) {
		/*
		 * Should we really be setting the error here?
		 */
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "_alSplitSourceQueue: shouldn't happen");
		return;
	}

	mixable    = _alSourceBytesLeftByChannel(src, samp);
	remaining  = 0;

	/* in case samp->size < len, we don't want
	 * to overwrite with the memcpy
	 */
	if(len * bufchannels <= samp->size) {
		/* normal case */
		remaining = (len * bufchannels) - mixable;

		for(i = 0; i < nc; i++) {
			bufptr = _alSourceGetBufptr(src, samp, i);
			nextpcm = nextsamp->orig_buffers[i];

			memcpy(buffers[i], bufptr, mixable);
			memcpy(buffers[i] + mixable/2, nextpcm, remaining);
		}

		return;
	}

	for(i = 0; i < nc; i++) {
		bufptr = _alSourceGetBufptr(src, samp, i);

		memcpy(buffers[i], bufptr, len);
	}

	return;
}

/*
 * _alMonoifyOffset( ALshort **dstref, ALuint offset,
 *                   ALvoid *src, ALuint src_size,
 *                   ALuint dest_channels, ALuint src_channels )
 *
 * Copies the interleaved data from src[0..(src_size/2)-1] to
 * dstret[0..dest_channels-1], splitting it into seperate channels.
 * src_channels describes the period of the channel repitition in src,
 * dest_channels describes the number of independant buffers in dstref.
 * src_size is the size of src in bytes, and offset is the offset into each
 * seperate channel in dstref where the copying begins.
 *
 * assumes locked context
 */
void _alMonoifyOffset(ALshort **dstref, ALuint offset,
		      ALvoid *srcp, ALuint size, ALuint dc, ALuint sc) {
	switch( dc ) {
		case 2:
		  switch(sc) {
			  case 1:
				_alMonoifyOffset1to2(dstref, offset, srcp, size);
				break;
			  case 2:
				_alMonoifyOffset2to2(dstref, offset, srcp, size);
				break;
			  default:
			  	fprintf(stderr, "unhandled Monoify (dc %d sc %d)\n",
					dc, sc);
				break;
		  }
		  break;
		case 1:
		  switch(sc) {
			  case 1:
		    		memcpy((char *) *dstref + offset, srcp, size);
				break;
			  default:
			  	fprintf(stderr, "unhandled Monoify (dc %d sc %d)\n",
					dc, sc);
			  	break;
		  }
		  break;
		default:
		  _alDebug(ALD_SOURCE, __FILE__, __LINE__,
			"Unhandled dc %d", dc);
		  break;
	}

	return;
}

/*
 * _alMonoifyOffset1to2( ALshort **dsts, ALuint offset,
 * 			 ALvoid *srcp, ALuint size)
 *
 * Helper function for _alMonoifyOffset.  Converts from stereo to 2 channels
 * of buffer data.
 */
static void _alMonoifyOffset1to2( ALshort **dsts, ALuint offset,
				  ALvoid *srcp, ALuint size) {
	ALshort *src = (ALshort *) srcp;
	ALshort *dst0 = dsts[0];
	ALshort *dst1 = dsts[1];
	int len      = size / sizeof *src;
        int i;

	offset /= sizeof **dsts;
	dst0 += offset;
	dst1 += offset;

	for(i = 0; i < len; i++) {
		dst0[i] = src[0];
		dst1[i] = src[0];

		src++;
	}

        return;
}

/*
 * _alMonoifyOffset2to2( ALshort **dsts, ALuint offset,
 *                       ALvoid *srcp, ALuint size )
 *
 * Helper function for _alMonoifyOffset.  Converts from stereo to 2 channels
 * of buffer data.
 */
static void _alMonoifyOffset2to2( ALshort **dsts, ALuint offset,
				  ALvoid *srcp, ALuint size) {
	ALshort *src = (ALshort *) srcp;
	ALshort *dst0 = dsts[0];
	ALshort *dst1 = dsts[1];
	int len      = size / sizeof *src;
        int i;

	offset /= sizeof **dsts;

	dst0 += offset;
	dst1 += offset;

	for(i = 0; i < len; i++) {
		dst0[i] = src[0];
		dst1[i] = src[1];

		src += 2;
	}

        return;
}

/*
 * _alChannelifyOffset( ALshort *dst, ALuint offset,
 *                      ALshort **srcs, ALuint size, ALuint nc )
 *
 * This function is sort of the complement of _alMonoifyOffset.  Data is
 * copied from srcs[0..nc-1][offset/2..(offset + size)/2-1] into an
 * interleaved array dst.
 *
 * assumes locked context
 *
 *  FIXME: handle cases with > 2 channels
 */
void _alChannelifyOffset( ALshort *dst, ALuint offset,
			  ALshort **srcs, ALuint size, ALuint nc ) {
	switch( nc ) {
		case 2:
			_alChannelify2Offset(dst, offset, srcs, size);
			break;
		case 1:
			memcpy( dst, srcs[0] + offset/sizeof *srcs, size );
			break;
		default:
			break;
	}

	return;
}

/*
 *  _alChannelify2Offset( ALshort *dst, ALuint offset,
 *                        ALshort **srcs, ALuint size )
 *
 * This function is like ChannelifyOffset, but specificly for those cases
 * where the number of channels in srcs is 2.
 *
 * assumes locked context
 */
void _alChannelify2Offset( ALshort *dst, ALuint offset,
			   ALshort **srcs, ALuint size ) {
	ALshort *src0 = &srcs[0][offset / sizeof *srcs];
	ALshort *src1 = &srcs[1][offset / sizeof *srcs];
	ALuint k;

	size /= sizeof *dst; /* we need sample offsets */

	for( k = 0; k < size; k++ ) {
		dst[0] = src0[k];
		dst[1] = src1[k];

		dst += 2;
	}

	return;
}

/*
 * alDeleteSources( ALsizei n, ALuint *sources )
 *
 * Delete n sources, with sids located in sources[0..n-1].  Only full
 * deallocations are possible, and if one of sources[0..n-1] is not a
 * valid source id (or is currently in an active state), and error is
 * set and no deallocation occurs.
 *
 * If n is 0, this is a legal nop.  If n < 0, INVALID_VALUE is set
 * and this is a nop.
 */
void alDeleteSources( ALsizei n, ALuint *sources ) {
	AL_source *src;
	AL_context *cc;
	int i;

	if(n == 0) {
		/* silently return */
		return;
	}

	if(n < 0) {
		_alDebug(ALD_BUFFER, __FILE__, __LINE__,
		      "alDeleteSources: invalid n %d\n", n);

		_alDCSetError(AL_INVALID_VALUE);
		return;
	}

	_alcDCLockContext();

	cc = _alcDCGetContext();
        if(cc == NULL) {
		/*
		 * No current context with which to evaluate the 
		 * validity of the sources
		 */
		_alcDCUnlockContext();
		return;
	}

	if(n < 0) {
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "alDeleteSources: illegal n value %d", n);

		_alDCSetError(AL_INVALID_VALUE);
		_alcDCUnlockContext();
		return;
	}

	for(i = 0; i < n; i++) {
		src = _alDCGetSource( sources[i] );
		if(src == NULL) {
			/* invalid source id, return. */
			_alDebug(ALD_SOURCE, __FILE__, __LINE__,
			      "alDeleteSources: invalid source %d",
			      sources[i]);

			_alDCSetError( AL_INVALID_NAME );
			_alcDCUnlockContext();

			return;
		}

		if((src->state == AL_PLAYING) || (src->state == AL_PAUSED)) {
			/* 
			 * FIXME: illegal to delete playing source?
			 */
			_alDebug(ALD_SOURCE, __FILE__, __LINE__,
			      "alDeleteSources: tried to delete playing/paused source %d",
			      sources[i]);

			_alDCSetError(AL_ILLEGAL_COMMAND);
			_alcDCUnlockContext();
			return;
		}

	}

	for(i = 0; i < n; i++) {
		src = _alDCGetSource(sources[i]);
		if(src == NULL) {
			_alDebug(ALD_SOURCE, __FILE__, __LINE__,
			      "alDeleteSources: invalid source %d",
			      sources[i]);

			_alDCSetError(AL_INVALID_NAME);
			continue;
		}

		if(src->state == AL_PLAYING) {
			/* 
			 * FIXME: illegal to delete playing source?
			 */
			_alDebug(ALD_SOURCE, __FILE__, __LINE__,
			      "alDeleteSources: tried to del playing source %d",
			      sources[i]);

			_alDCSetError(AL_ILLEGAL_COMMAND);
			continue;
		}

		spool_dealloc(&cc->source_pool, sources[i],
						_alDestroySource);
	}

	_alcDCUnlockContext();
	return;
}

/*
 * _alDestroySources
 *
 * This destructor is responsible for deallocating source data structures
 * after openal has finished.
 */
void _alDestroySources(spool_t *spool) {
	ALuint i;

	for( i = 0; i < spool->size; i++ ) {
		mlDestroyMutex( spool->smutexen[i] );
	}


	spool_free( spool, _alDestroySource );

	free( spool->smutexen );
	free( stereoptr );

	spool->smutexen = NULL;
	stereoptr = NULL;

	return;
}

/*
 * _alDestroySource
 *
 * Deallocates the memory associated with an AL_source, passed as a void *
 * to this function.
 *
 * assumes locked context
 */
void _alDestroySource( void *srcp ) {
	AL_source *src = (AL_source *) srcp;
	ALuint *bidp;
	int i;

	/*
	 * if we have a callback buffer, call the
	 * destructor with the "free one source" args
	 */
	bidp = _alGetSourceParam(src, AL_BUFFER);
	if(bidp != NULL) {
		if(_alBidIsCallback(*bidp) == AL_TRUE) {
			_alBidCallDestroyCallbackSource(src->sid);
		}
	}

	/* deallocation per source scratch space */
	free(src->srcParams.outbuf);
	src->srcParams.outbuf = NULL;

	for(i = _alcDCGetNumSpeakers() - 1; i >= 0; i--) {
		if(src->reverb_buf[i] != NULL) {
			/* deallocation reverb scratch space */
			free( src->reverb_buf[i] );
			src->reverb_buf[i] = NULL;
		}
	}

	free( src->bid_queue.queuestate );
	free( src->bid_queue.queue );

	src->bid_queue.queue = NULL;
	src->bid_queue.queuestate = NULL;
	src->bid_queue.size = 0;

	return;
}

/*
 * alSourcePause( ALuint sid )
 *
 * If sid is a valid source name, then if the source associated with it has
 * the state AL_PLAYING, it will be taken to the state AL_PAUSED.
 *
 * If sid is not a valid source name, set AL_INVALID_NAME.
 */
void alSourcePause( ALuint sid ) {
	alSourcePausev( 1, &sid );

	return;
}

/*
 * alSourcePlay( ALuint sid )
 *
 * If sid is a valid source name, then if the source associated with it has
 * the state AL_INITAL, AL_PAUSED, or AL_STOPPED, it will be taken to the
 * state AL_PLAYING.
 *
 * If sid is not a valid source name, set AL_INVALID_NAME.
 */
void alSourcePlay( ALuint sid ) {
	alSourcePlayv( 1, &sid );

	return;
}

/*
 * alSourceStop( ALuint sid )
 *
 * If sid is a valid source name, then if the source associated with it has
 * the state AL_PLAYING or AL_PAUSED it will be taken to the state AL_STOPPED.
 *
 * If sid is not a valid source name, set AL_INVALID_NAME.
 */
void alSourceStop( ALuint sid ) {
	alSourceStopv( 1, &sid );

	return;
}

/*
 * alSourceRewind( ALuint sid )
 *
 * If sid is a valid source name, then if the source associated with it has
 * the state AL_PLAYING, AL_PAUSED or AL_STOPPED, it will be taken to the
 * state AL_INITAL.
 *
 * If the source is playing or paused, it will first be stopped.
 *
 * If sid is not a valid source name, set AL_INVALID_NAME.
 */
void alSourceRewind( ALuint sid ) {
	alSourceRewindv( 1, &sid );

	return;
}

/*
 * alSourceRewindv( ALsizei ns, ALuint *sids )
 *
 * If ns == 0, legal NOP.
 * If ns < 0, sets AL_INVALID_VALUE.
 *
 * If sids[0..ns-1] are all valid source names, then for each source with
 * state AL_PLAYING, AL_PAUSED or AL_STOPPER change the state to AL_INITAL.
 * If sources are playing or paused, they will first be stopped.
 *
 * If any member of sids[0..ns-1] is not a valid source name, set
 * AL_INVALID_NAME.
 */
void alSourceRewindv( ALsizei ns, ALuint *sids ) {
	AL_source *src;
	ALsizei i;

	if( ns == 0 ) {
		/* legal NOP */
		return;
	}

	if( ns < 0 ) {
		/* set error */
		_alcDCLockContext();
		_alDCSetError( AL_INVALID_VALUE );
		_alcDCUnlockContext();

		return;
	}

	SOURCELOCK();

	/* validate */
	for( i = 0; i < ns; i++ ) {
		if( _alIsSource( sids[i] ) == AL_FALSE ) {
			_alDCSetError( AL_INVALID_NAME );

			SOURCEUNLOCK();

			return;
		}
	}

	_alLockMixBuf();

	for( i = 0; i < ns; i++ ) {
		src = _alDCGetSource( sids[i] );
		if(src == NULL) {
			/* shouldn't happen */
			_alDebug(ALD_SOURCE, __FILE__, __LINE__,
			      "alSourceRewindv: source id %d is invalid",
			      sids[i] );

			_alDCSetError( AL_INVALID_NAME );

			SOURCEUNLOCK();
			return;
		}


		switch( src->state ) {
			case AL_INITIAL:
				/* legal NOP */
				break;
			case AL_PAUSED:
			case AL_PLAYING:
				_alRemoveSourceFromMixer( sids[i] );
			case AL_STOPPED:
				src->state = AL_INITIAL;
				src->srcParams.soundpos = 0;
				break;
		}
	}

	_alUnlockMixBuf();

	SOURCEUNLOCK();

	return;
}

/*
 * alSourceStopv( ALsizei ns, ALuint *sids )
 *
 * If ns == 0, legal NOP.
 * If ns < 0, sets AL_INVALID_VALUE.
 *
 * If sids[0..ns-1] are all valid source names, then for each source with
 * state AL_PLAYING or AL_PAUSED change the state to AL_STOPPED.  Otherwise, 
 * set AL_INVALID_NAME.
 */
void alSourceStopv( ALsizei ns, ALuint *sids ) {
	ALsizei i;

	if( ns == 0 ) {
		/* legal NOP */
		return;
	}

	if( ns < 0 ) {
		/* set error */
		_alcDCLockContext();
		_alDCSetError( AL_INVALID_VALUE );
		_alcDCUnlockContext();

		return;
	}

	SOURCELOCK();

	/* validate */
	for( i = 0; i < ns; i++ ) {
		if( _alIsSource( sids[i] ) == AL_FALSE ) {
			_alDCSetError( AL_INVALID_NAME );

			SOURCEUNLOCK();

			return;
		}
	}

	_alLockMixBuf();

	for( i = 0; i < ns; i++ ) {
		_alRemoveSourceFromMixer( sids[i] );
	}

	_alUnlockMixBuf();

	SOURCEUNLOCK();

	return;
}

/*
 * alSourcePlayv( ALsizei ns, ALuint *sids )
 *
 * If ns == 0, legal NOP.
 * If ns < 0, sets AL_INVALID_VALUE.
 *
 * If sids[0..ns-1] are all valid source names, then for each source with
 * state AL_INITIAL, AL_STOPPED or AL_PAUSED, change the state to AL_PLAYING.
 * Otherwise, set AL_INVALID_NAME.
 */
void alSourcePlayv( ALsizei ns, ALuint *sids ) {
	ALsizei i;

	if( ns == 0 ) {
		/* legal NOP */
		return;
	}

	if( ns < 0 ) {
		/* set error */
		_alcDCLockContext();
		_alDCSetError( AL_INVALID_VALUE );
		_alcDCUnlockContext();

		return;
	}

	SOURCELOCK();

	/* validate */
	for( i = 0; i < ns; i++ ) {
		if( _alIsSource( sids[i] ) == AL_FALSE ) {
			_alDCSetError( AL_INVALID_NAME );

			SOURCEUNLOCK();

			return;
		}
	}

	_alLockMixBuf();

	for(i = 0; i < ns; i++) {
		_alAddSourceToMixer( sids[i] );
	}

	_alUnlockMixBuf();

	SOURCEUNLOCK();

	return;
}

/*
 * alSourcePausev( ALsizei ns, ALuint *sids )
 *
 * If ns == 0, legal NOP.
 * If ns < 0, sets AL_INVALID_VALUE.
 *
 * If sids[0..ns-1] are all valid source names, then for each source with
 * state AL_PLAYING, change the state to AL_PAUSED.  Otherwise, set
 * AL_INVALID_NAME.
 *
 */
void alSourcePausev( ALsizei ns, ALuint *sids ) {
	AL_source *src;
	ALsizei i;

	if( ns == 0 ) {
		 /* legal NOP */
		return;
	}

	if( ns < 0 ) {
		/* set error */
		_alcDCLockContext();
		_alDCSetError( AL_INVALID_VALUE );
		_alcDCUnlockContext();

		return;
	}

	SOURCELOCK();

	/* validate */
	for( i = 0; i < ns; i++ ) {
		if( _alIsSource( sids[i] ) == AL_FALSE ) {
			_alDCSetError( AL_INVALID_NAME );

			SOURCEUNLOCK();

			return;
		}
	}

	/* set */

	_alLockMixBuf();

	for( i = 0; i < ns; i++ ) {
		src = _alDCGetSource( sids[i] );

		/*
		 * If source is active, set it to be paused.  Otherwise,
		 * it's a legal NOP.
		 */

		if( src->state == AL_PLAYING ) {
			src->state = AL_PAUSED;
		}
	}

	_alUnlockMixBuf();

	SOURCEUNLOCK();

	return;
}

/*
 * _alCollapseSource( ALuint cid, ALuint sid,
 *                    ALuint nc, ALuint mixbuflen,
 *                    ALshort **buffers );
 *
 * Populates the scratch space associated with the source named by sid in the
 * context named by cid with interleaved data.  The data is interleaved using
 * alternating channels from buffers[0..nc-1].  Each member of the set
 * buffers[0..nc-1] is an indepedent channel's worth of data, 0..mixbuflen/2
 * long bytes long.
 *
 * assumes locked context
 */
void _alCollapseSource( ALuint cid, ALuint sid,
			ALuint nc, ALuint mixbuflen,
			ALshort **buffers ) {
	AL_source *src;
	AL_buffer *smp;
	ALuint len;
	ALuint bufchannels;

	len = mixbuflen / sizeof **buffers;

	src = _alGetSource( cid, sid );
	if(src == NULL) {
		_alSetError( cid, AL_INVALID_NAME );
		return;
	}

	smp = _alGetBufferFromSid( cid, sid );
	if(smp == NULL) {
		_alSetError( cid, AL_INVALID_NAME );
		return;
	}

	bufchannels = _al_ALCHANNELS( smp->format );

	if(src->srcParams.outbuf == NULL) {
		src->srcParams.outbuf = malloc( mixbuflen );

		if(src->srcParams.outbuf == NULL) {
			_alSetError( cid, AL_OUT_OF_MEMORY );
			return;
		}
	}

	memset(src->srcParams.outbuf, 0, mixbuflen);

	if(len > bufchannels * (smp->size - src->srcParams.soundpos)) {
		/* kludge.  dc->silence? */
		memset(src->srcParams.outbuf, 0, mixbuflen);

		if( _alSourceIsLooping(src) == AL_FALSE ) {
			/*
			 * Non looping source get f_buffer truncated because
			 * they don't (potentially) posses enough data.
			 */
			len = bufchannels * (smp->size - src->srcParams.soundpos);
		}
	}

	_alChannelify(src->srcParams.outbuf, buffers, len, nc);

	return;
}

/*
 * _alInitSource( ALuint sid )
 *
 * Initialize an already allocated source.
 */
static void _alInitSource( ALuint sid ) {
	AL_source *src;
	AL_sourcestate *srcstate;
	int i;
	
	src = _alDCGetSource( sid );
	if(src == NULL) {
		/* sid is invalid */
		return;
	}

	/* set state */
	src->state = AL_INITIAL;

	/* set identifier */
	src->sid = sid;

	/* set data values */
	src->srcParams.outbuf   = NULL;
	src->srcParams.soundpos = 0;
	src->flags              = ALS_NONE;
	src->reverbpos          = 0;

	for(i = 0; i < _ALC_MAX_CHANNELS; i++) {
		src->reverb_buf[i] = NULL;
	}

	/* Initialize the buffer queue */
	_alSourceQueueInit( src );

	srcstate = _alSourceQueueGetCurrentState( src );
	ASSERT( srcstate );

	_alSourceStateInit( srcstate );

	src->position.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_POSITION, src->position.data );

	src->direction.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_DIRECTION, src->direction.data );
	
	src->velocity.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_VELOCITY, src->velocity.data );

	src->reverb_scale = 0.25;
	src->reverb_delay = 0.00;

	src->gain.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_GAIN_LINEAR_LOKI, &src->gain.data );
	
	src->coneinnerangle.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_CONE_INNER_ANGLE, &src->coneinnerangle.data );

	src->coneouterangle.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_CONE_OUTER_ANGLE, &src->coneouterangle.data );

	src->isstreaming.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_STREAMING, &src->isstreaming.data );
 
	src->isrelative.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_SOURCE_RELATIVE, &src->isrelative.data );

	src->attenuationmin.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_MIN_GAIN, &src->attenuationmin.data );

	src->attenuationmax.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_MAX_GAIN, &src->attenuationmax.data );

	src->pitch.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_PITCH, &src->pitch.data );

	src->ref_distance.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_REFERENCE_DISTANCE, &src->ref_distance.data );

	src->max_distance.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_MAX_DISTANCE, &src->max_distance.data );

	src->rolloff_factor.isset = AL_FALSE;
	_alSourceGetParamDefault( AL_ROLLOFF_FACTOR, &src->rolloff_factor.data );

	return;
}

/*
 * _alSourceTranslate( AL_source *src, ALfloat *delta )
 *
 * Translates the source (src) position attribute by delta.  Delta is a three
 * tuple x/y/z.
 *
 * assumes locked context
 */
void _alSourceTranslate( AL_source *src, ALfloat *delta ) {
	ALfloat *opos; /* original source position */

	opos = _alGetSourceParam( src, AL_POSITION );
	if( opos == NULL ) {
		/* no translation possible or needed */
		return;
	}

	_alVectorTranslate( opos, opos, delta );

	return;
}

/*
 * srcParam functions
 *
 * The use of srcParam settings is this: most filter operations break down
 * into applying a multiplier or delay to the raw PCM data copied in
 * SplitSources.  Since there operations are cumulative, this can be simplified
 * by simply collecting the coefficients of the operations and applying them
 * all at one.  So instead of actually performing a gain or delay operation on
 * the PCM data, the filter can change the srcParam setting, which is applied
 * in _alSourceParamApply.
 *
 */

/*
 * _alSourceParamReset( AL_source *src )
 *
 * Resets the srcParam settings gain and delay.  Does not affect srcParam
 * settings associated with sound position or the temporary scratch space.
 *
 * assumes locked context
 */
void _alSourceParamReset( AL_source *src ) {
	AL_listener *lis;
	int i;

	lis = _alcDCGetListener();

	if(src == NULL) {
		return;
	}

	for(i = 0; i < _ALC_MAX_CHANNELS; i++) {
		src->srcParams.delay[i] = 0;
		src->srcParams.gain[i]  = 1.0;
	}

	return;
}

/*
 * _alSourceParamApply( AL_source *src,
 *                      ALuint nc, ALuint len, ALshort **buffers)
 *
 * Applies the srcParam settings of src to buffers[0..nc-1][0..(len/2)-1].
 *
 * This function should be called before a source is collapsed, in
 * ApplyFilters.
 *
 * assumes locked context
 *
 * FIXME: ignores delay
 */
void _alSourceParamApply( AL_source *src,
			  ALuint nc, ALuint len, ALshort **buffers ) {
	ALuint sampLen;
	ALuint i;
	ALfloat gain;
 
	sampLen = len / sizeof(ALshort); /* we pass sample length */

	for(i = 0; i < nc; i++) {
		gain = src->srcParams.gain[i];

		if(gain == 1.0) {
			/* don't floatmul when gain is 1.0 */
			continue;
		}

		_alFloatMul( buffers[i], gain, sampLen );
	}

	return;
}

/*
 * _alSourceGetPendingBids( AL_source *src )
 *
 * Returns the number of buffers queued in the source, not including the
 * current one ( if any ).
 */
ALint _alSourceGetPendingBids(AL_source *src) {
	ALint retval =  (src->bid_queue.size - 1) - src->bid_queue.read_index;

	return retval;
}

/*
 * _alSourceShouldIncrement( AL_source *src )
 *
 * Usually, the top-level mixing function _alMixSources handles updating each
 * source's sound position ( a pointer into its associate buffer's raw PCM
 * data. )  Some attributes, like pitch, demand a more sophisticated approach
 * that can only be done by the filter which handles this attribute.
 *
 * _alSourceShouldIncrement returns AL_TRUE if the top-level mixing function
 * should update this sort of state information, and AL_FALSE if it should
 * leave it to another portion of the library.
 *
 * assumes locked context
 */
ALboolean _alSourceShouldIncrement(AL_source *src) {
	AL_sourcestate *srcstate;

	srcstate =_alSourceQueueGetCurrentState(src);
	ASSERT(srcstate);

	if(src->flags & ALS_NEEDPITCH) {
		return AL_FALSE;
	}

	return AL_TRUE;
}

/*
 * _alSourceIncrement( AL_source *src, ALuint bytes )
 *
 * Increments the source's (src) offset into its current buffer's PCM data.
 */
void _alSourceIncrement(AL_source *src, ALuint bytes) {
	src->srcParams.soundpos += bytes;

	return;
}

/*
 * _alSourceGetBufptr( AL_source *src, AL_buffer *buf, ALuint index )
 *
 * Returns pointer to pcm data for buf, offset by src's soundpos.
 *
 * assumes locked context
 */
void *_alSourceGetBufptr( AL_source *src, AL_buffer *buf, ALuint index ) {
	ALbyte *retval;
	ALuint pos = src->srcParams.soundpos;

	retval = buf->orig_buffers[index];

	return retval + pos;
}

/*
 * _alSourceBytesLeftByChannel( AL_source *src, AL_buffer *samp )
 *
 * Returns the byte length of the amount of data left before this source/samp
 * pair will either run out of data or be required to wrap.  This returns the
 * byte length of a simple array, with 1 channel's worth of data.
 */
ALint _alSourceBytesLeft(AL_source *src, AL_buffer *samp) {
	ALuint nc = samp->num_buffers;

	return nc * _alSourceBytesLeftByChannel(src, samp);
}

/*
 * _alSourceBytesLeft(AL_source *src, AL_buffer *samp)
 *
 * Returns the byte length of the amount of data left before this source/samp
 * pair will either run out of data or be required to wrap.  This returns the
 * byte length of an interleaved array, with periodic repetition equal to the
 * number of channels in the canonical format.
 */
ALint _alSourceBytesLeftByChannel(AL_source *src, AL_buffer *samp) {
	return samp->size - src->srcParams.soundpos;
}

/*
 * _alSourceIsLooping( AL_source *src )
 *
 * Returns AL_TRUE if the source (src) has its AL_LOOPING attribute set to
 * AL_TRUE, AL_FALSE otherwise.
 */
ALboolean _alSourceIsLooping( AL_source *src ) {
	ALboolean *boo = _alGetSourceParam( src, AL_LOOPING );

	if( boo == NULL ) {
		return AL_FALSE;
	}

	return *boo;
}

/*
 * FL_alLockSource( UNUSED(const char *fn),
 *                  UNUSED(int ln),
 *                  ALuint cid,
 *                  ALuint sid )
 *
 * Locks source sid at cid.
 */
ALboolean FL_alLockSource( UNUSED(const char *fn),
			   UNUSED(int ln),
			   ALuint cid,
			   ALuint sid ) {
	ALint sindex;
	AL_context *cc;

	cc = _alcGetContext( cid );
	if( cc == NULL ) {
		return AL_FALSE;
	}

	_alLockPrintf("_alLockSource", fn, ln);
	
	sindex = spool_sid_to_index( &cc->source_pool, sid );
	if( sindex < 0 ) {
		/* invalid sid */
		return AL_FALSE;
	}

	if( cc->source_pool.smutexen[sindex] == NULL ) {
		return AL_FALSE;
	}

	mlLockMutex( cc->source_pool.smutexen[sindex] );

	return AL_TRUE;
}

/*
 * FL_alUnlockSource( UNUSED(const char *fn),
 *                    UNUSED(int ln),
 *                    ALuint cid,
 *                    ALuint sid )
 *
 * Unlocks source mutex for sid at cid.
 */
ALboolean FL_alUnlockSource( UNUSED(const char *fn),
			     UNUSED(int ln),
			     ALuint cid,
			     ALuint sid ) {
	ALint sindex;
	AL_context *cc;

	cc = _alcGetContext( cid );
	if( cc == NULL ) {
		return AL_FALSE;
	}

	_alLockPrintf("_alUnlockSource", fn, ln);
	
	sindex = spool_sid_to_index( &cc->source_pool, sid );
	if( sindex < 0 ) {
		/* invalid sid */
		return AL_FALSE;
	}

	if( cc->source_pool.smutexen[sindex] == NULL ) {
		return AL_FALSE;
	}

	mlUnlockMutex( cc->source_pool.smutexen[sindex] );

	return AL_TRUE;
}

/*
 * _alGetSourceParam( AL_source *source, ALenum param )
 *
 * Returns a pointer to the source attribute specified by param.  If the
 * attribute has not been set by the application, NULL is returned.
 *
 * NOTES:
 *  This returns NULL if the paramater specified by param hasn't
 *  been set, so that the calling function can determine what it
 *  thinks a sensible default should be.  Perhaps it would be
 *  best to have a set of default variables here and return their
 *  value when need be?
 */
void *_alGetSourceParam(AL_source *source, ALenum param ) {
	AL_sourcestate *srcstate;

	if( _alSourceIsParamSet( source, param ) == AL_FALSE ) {
		return NULL;
	}

	srcstate = _alSourceQueueGetCurrentState( source );
	ASSERT(srcstate);

	switch( param ) {
		case AL_BUFFER:
			if( source->bid_queue.size > 0 ) {
				int index = source->bid_queue.read_index;

				return &source->bid_queue.queue[index];
			} else {
				_alDebug(ALD_SOURCE, __FILE__, __LINE__,
					"_alGetSourceState: bid_queue.size == %d",
				source->bid_queue.size);
			}
			break;
		case AL_CONE_INNER_ANGLE:
			return &source->coneinnerangle.data;
			break;
		case AL_CONE_OUTER_ANGLE:
			return &source->coneouterangle.data;
			break;
		case AL_CONE_OUTER_GAIN:
			return &source->coneoutergain.data;
			break;
		case AL_DIRECTION:
			return &source->direction.data;
			break;
		case AL_GAIN_LINEAR_LOKI:
			return &source->gain.data;
			break;
		case AL_LOOPING:
			return &srcstate->islooping.data;
			break;
		case AL_PITCH:
			return &source->pitch.data;
			break;
		case AL_POSITION:
			return &source->position.data;
			break;
		case AL_SOURCE_RELATIVE:
			return &source->isrelative.data;
			break;
		case AL_STREAMING:
			return &source->isstreaming.data;
			break;
		case AL_VELOCITY:
			return &source->velocity.data;
			break;
		case AL_MIN_GAIN:
		 	return &source->attenuationmin.data;
			break;
		case AL_MAX_GAIN:
			return &source->attenuationmax.data;
			break;
		case AL_SOURCE_STATE:
			return &source->state;
			break;
		case AL_REFERENCE_DISTANCE:
			return &source->ref_distance.data;
			break;
		case AL_MAX_DISTANCE:
			return &source->max_distance.data;
			break;
		case AL_ROLLOFF_FACTOR:
			return &source->rolloff_factor.data;
			break;
		default:
			_alDebug(ALD_SOURCE, __FILE__, __LINE__,
				"unknown source param 0x%x", param);

			ASSERT( 0 );

			break;
	}

	return NULL;
}

/*
 * _alSourceIsParamSet( AL_source *source, ALenum param )
 *
 * Returns AL_TRUE if param is set for source, AL_FALSE otherwise.
 */
ALboolean _alSourceIsParamSet( AL_source *source, ALenum param ) {
	AL_sourcestate *srcstate;

	srcstate = _alSourceQueueGetCurrentState( source );
	ASSERT(srcstate);

	switch( param ) {
		case AL_BUFFER:
		case AL_SOURCE_STATE:
			return AL_TRUE;
			break;
		case AL_CONE_INNER_ANGLE:
			return source->coneinnerangle.isset;
			break;
		case AL_CONE_OUTER_ANGLE:
			return source->coneouterangle.isset;
			break;
		case AL_CONE_OUTER_GAIN:
			return source->coneoutergain.isset;
			break;
		case AL_DIRECTION:
			return source->direction.isset;
			break;
		case AL_GAIN_LINEAR_LOKI:
			return source->gain.isset;
			break;
		case AL_LOOPING:
			return srcstate->islooping.isset;
			break;
		case AL_PITCH:
			return source->pitch.isset;
			break;
		case AL_POSITION:
			return source->position.isset;
			break;
		case AL_SOURCE_RELATIVE:
			return source->isrelative.isset;
			break;
		case AL_STREAMING:
			return source->isstreaming.isset;
			break;
		case AL_VELOCITY:
			return source->velocity.isset;
			break;
		case AL_MIN_GAIN:
			return source->attenuationmin.isset;
			break;
		case AL_MAX_GAIN:
			return source->attenuationmax.isset;
			break;
		case AL_REFERENCE_DISTANCE:
			return source->ref_distance.isset;
			break;
		case AL_MAX_DISTANCE:
			return source->max_distance.isset;
			break;
		case AL_ROLLOFF_FACTOR:
			return source->rolloff_factor.isset;
			break;
		default:
			_alDebug(ALD_SOURCE, __FILE__, __LINE__,
				"unknown source param 0x%x", param);

			ASSERT( 0 );
			break;
	}

	return AL_FALSE;
}

/*
 * _alSourceGetParamDefault( ALenum param, ALvoid *retref )
 *
 * Sets retref to the default value for param.
 */
void _alSourceGetParamDefault( ALenum param, ALvoid *retref ) {
	ALint *ip     = retref;
	ALboolean *bp = retref;
	ALfloat *fp   = retref;
	Rcvar pref    = NULL;

	switch( param ) {
		case AL_BUFFER:
			*ip = 0;
			break;
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
			*fp = 360.0f;
			break;
		case AL_MIN_GAIN:
			*fp = 0.0f;
			break;
		case AL_CONE_OUTER_GAIN:
		case AL_GAIN_LINEAR_LOKI:
		case AL_MAX_GAIN:
		case AL_PITCH:
		case AL_REFERENCE_DISTANCE:
			*fp = 1.0f;
			break;
		case AL_ROLLOFF_FACTOR:
			/*
			 * check user preference.  If set, use that.
			 * Otherwise, use the default of 1.0f
			 */
			pref = rc_lookup( "source-rolloff-factor" );
			if( pref != NULL ) {
				*fp = rc_tofloat( pref );
			}  else {
				*fp = 1.0f;
			}
			break;
		case AL_DIRECTION:
		case AL_POSITION:
		case AL_VELOCITY:
			fp[0] = 0.0f;
			fp[1] = 0.0f;
			fp[2] = 0.0f;
			break;
		case AL_LOOPING:
		case AL_SOURCE_RELATIVE:
		case AL_STREAMING:
			*bp = AL_FALSE;
			break;
		case AL_MAX_DISTANCE:
			*fp = FLT_MAX;
			break;
		case AL_SOURCE_STATE:
		default:
			ASSERT( 0 );
			break;
	}

	return;
}
