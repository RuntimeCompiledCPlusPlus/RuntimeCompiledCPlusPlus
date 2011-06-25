/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * alc_speaker.c
 *
 * functions related to the position of the speakers in the alc world.
 *
 * This speaker handling stuff is a little goofy.  Rotation code should be
 * rewritten.  Also, the assumption is that speaker positions correspond to an
 * interleaved layout of LRLRLR, or LF RF LB RB.  This might not always be the
 * case.
 */
#include <AL/alctypes.h>

#include "AL/altypes.h"

#include "al_siteconfig.h"
#include "al_main.h"
#include "al_types.h"
#include "al_debug.h"
#include "al_state.h"
#include "al_vector.h"

#include "alc_context.h"
#include "alc_error.h"
#include "alc_speaker.h"

#include <stdio.h>

/*
 * _alcSpeakerMove( ALuint cid )
 *
 * Moves speakers for context named by cid, using listener orientation and
 * position as modifiers.
 *
 * If cid is invalid, set ALC_INVALID_CONTEXT.
 *
 * assumes context cid is locked
 *
 * FIXME:
 *    please check my math
 */
void _alcSpeakerMove( ALuint cid ) {
	AL_context *cc;
	ALfloat zeros[] = { 0.0, 0.0,  0.0 };
	ALfloat oldat[] = { 0.0, 0.0, -1.0 };
	ALfloat oldup[] = { 0.0, 1.0, 0.0 };
	ALfloat newat[3];
	ALfloat newup[3];
	ALfloat angle;   /* angle to rotate at vector by */
	ALfloat upangle; /* angle to rotate up vector by */
	ALfloat *pos;    /* listener position */
	ALfloat ipos[3]; /* inverse listener position */
	ALuint i;

	cc = _alcGetContext( cid );
	if(cc == NULL) {
		_alDebug(ALD_CONTEXT, __FILE__, __LINE__,
		      "_alcSpeakerMove: invalid context id %d", cid);

		_alcSetError( ALC_INVALID_CONTEXT );

		return;
	}

	pos = cc->listener.Position;

	_alVectorNormalize(newat, cc->listener.Orientation );
	_alVectorNormalize(newup, &cc->listener.Orientation[3] );

	angle   = _alVectorAngleBeween( zeros, newat, oldat );
	upangle = _alVectorAngleBeween( zeros, newup, oldup );

	_alDebug(ALD_MATH, __FILE__, __LINE__, "( oldat %f %f %f) ( newat %f %f %f )",
		oldat[0], oldat[1], oldat[2], newat[0], newat[1], newat[2] );
	_alDebug(ALD_MATH, __FILE__, __LINE__, "( oldup %f %f %f) ( newup %f %f %f )",
		oldup[0], oldup[1], oldup[2], newup[0], newup[1], newup[2] );

	_alDebug(ALD_MATH, __FILE__, __LINE__, "( angle %f )", angle );
	_alDebug(ALD_MATH, __FILE__, __LINE__, "( upangle %f) ", upangle );

	/* reset speaker position */
	_alcSpeakerInit(cid);

	_alVectorInverse( ipos, cc->listener.Position );

	/* rotate about at and up vectors */
	for(i = 0; i < _alcDCGetNumSpeakers(); i++) {
		_alVectorTranslate(cc->_speaker_pos[i].pos,
				 cc->_speaker_pos[i].pos, ipos);

		_alRotatePointAboutAxis( angle, cc->_speaker_pos[i].pos, oldup );
		_alRotatePointAboutAxis( upangle, cc->_speaker_pos[i].pos, oldat );

		_alVectorTranslate(cc->_speaker_pos[i].pos,
				 cc->_speaker_pos[i].pos, pos);
	}

	_alDebug(ALD_MATH, __FILE__, __LINE__,
		"SpAdj: l/r [%f|%f|%f] [%f|%f|%f]",
		cc->_speaker_pos[0].pos[0],
		cc->_speaker_pos[0].pos[1],
		cc->_speaker_pos[0].pos[2],

		cc->_speaker_pos[1].pos[0],
		cc->_speaker_pos[1].pos[1],
		cc->_speaker_pos[1].pos[2]);

	return;
}

/*
 * _alcSpeakerInit( ALuint cid )
 *
 * Initialize position wrt listener, w/o rt orientation.
 *
 * assumes locked context
 */
void _alcSpeakerInit( ALuint cid ) {
	AL_context  *cc;
	AL_listener *lis;
	ALfloat *lpos;
	ALfloat sdis; /* scaled distance */

	cc  = _alcGetContext( cid );
	lis = _alcGetListener( cid );
	if(cc == NULL) {
		/* invalid cid */
		_alDebug(ALD_CONTEXT, __FILE__, __LINE__,
			"_alcSpeakerInit: invalid cid 0x%x", cid );

		return;
	}

	if(lis == NULL) {
		/* weird */
		return;
	}

	lpos = lis->Position;

	/*
	 * _alGetFloatv( AL_DISTANCE_SCALE, &sdis );
	 *
	 * Don't use AL_DISTANCE_SCALE because this might not be 
	 * the current context.
	 */
	sdis = 1.0f;
	sdis *= _ALC_SPEAKER_DISTANCE;

	_alDebug(ALD_CONTEXT, __FILE__, __LINE__,
		"_alcSpeakerInit: ( sdis %f )", sdis );

	/* left */
	cc->_speaker_pos[ALS_LEFT].pos[0]   = lpos[0] - sdis;
	cc->_speaker_pos[ALS_LEFT].pos[1]   = lpos[1];
	cc->_speaker_pos[ALS_LEFT].pos[2]   = lpos[2];

	/* right */
	cc->_speaker_pos[ALS_RIGHT].pos[0]  =  lpos[0] + sdis;
	cc->_speaker_pos[ALS_RIGHT].pos[1]  =  lpos[1];
	cc->_speaker_pos[ALS_RIGHT].pos[2]  =  lpos[2];

	/* left rear */
	cc->_speaker_pos[ALS_LEFTS].pos[0]  =  lpos[0] - sdis;
	cc->_speaker_pos[ALS_LEFTS].pos[1]  =  lpos[1] + sdis;
	cc->_speaker_pos[ALS_LEFTS].pos[2]  =  lpos[2] - sdis;

	/* right rear */
	cc->_speaker_pos[ALS_RIGHTS].pos[0] =  lpos[0] + sdis;
	cc->_speaker_pos[ALS_RIGHTS].pos[1] =  lpos[1] + sdis;
	cc->_speaker_pos[ALS_RIGHTS].pos[2] =  lpos[2] + sdis;

	return;
}

/*
 * _alcGetNumSpeakers( ALuint cid )
 *
 * Returns number of speakers available for context named cid.
 *
 * assumes locked context
 */
ALuint _alcGetNumSpeakers( ALuint cid ) {
	return _al_ALCHANNELS( _alcGetWriteFormat( cid ) );
}

/*
 * _alcGetSpeakerPosition( ALuint cid, ALuint speaker_num )
 *
 * Returns 3-float tuple giving the speaker position for speaker with offset
 * speaker_num for the context named cid, or NULL on error.
 *
 * assumes locked context
 */
ALfloat *_alcGetSpeakerPosition( ALuint cid, ALuint speaker_num ) {
	AL_context *cc;
	ALuint nc;

	cc = _alcGetContext( cid );
	if(cc == NULL) {
		return NULL;
	}

	nc = _alcGetNumSpeakers( cid );

	if( speaker_num >= nc ) {
		return NULL;
	}

	return cc->_speaker_pos[speaker_num].pos;
}
