/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * alc_speaker.h
 *
 * Prototypes, defines etc for speaker management.
 */
#ifndef ALC_SPEAKER_H_
#define ALC_SPEAKER_H_

#include "alc_context.h"
#include "AL/altypes.h"

/*
 * _ALC_SPEAKER_DISTANCE is the default distance of each speaker from the
 * origin.
 */
#define _ALC_SPEAKER_DISTANCE 5.0f

typedef enum {
	ALS_LEFT,
	ALS_RIGHT,
	ALS_LEFTS,
	ALS_RIGHTS
} _alcSpeakerEnum;

/*
 * _alcSpeakerInit( ALuint cid )
 *
 * Initializes the speaker setup for the context named by cid.
 */
void _alcSpeakerInit( ALuint cid );

/*
 *  _alcSpeakerMove( ALuint cid )
 *
 *  Updates the speaker setup for the context named by cid for changes in
 *  listener position and orientation.
 */
void _alcSpeakerMove( ALuint cid );

/*
 * _alcGetSpeakerPosition( ALuint cid, ALuint speaker_num )
 *
 * Returns the 3-tuple position (x/y/z) of the speaker enumerated by
 * speaker_num for the context named by cid, or NULL if cid does not name a
 * context.
 */
ALfloat *_alcGetSpeakerPosition( ALuint cid, ALuint speaker_num );

/*
 * _alcGetNumSpeakers( ALuint cid )
 *
 * Returns the number of speakers associated with the context named by cid.
 */
ALuint _alcGetNumSpeakers( ALuint cid );

/* default context macros */
#define _alcDCSpeakerMove()     _alcSpeakerMove(_alcCCId)
#define _alcDCSpeakerInit()     _alcSpeakerInit(_alcCCId)
#define _alcDCGetNumSpeakers()  _alcGetNumSpeakers(_alcCCId)

#endif /* _ALC_SPEAKERS_H_ */
