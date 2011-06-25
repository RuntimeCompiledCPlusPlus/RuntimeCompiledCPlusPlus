/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_queue.h
 *
 * Stuff related to the alQueue.
 */
#ifndef AL_QUEUE_H_

#include "al_types.h"

/*
 * _alSourceStateInit( AL_sourcestate *srcstate )
 *
 * Initialize an AL_sourcestate object to the default settings.
 */
void _alSourceStateInit( AL_sourcestate *srcstate );

/*
 * _alSourceQueueGetCurrentState( AL_source *src )
 *
 * Returns the current AL_sourcestate of the source src, or NULL on
 * error.
 */
AL_sourcestate *_alSourceQueueGetCurrentState( AL_source *src );

/*
 * _alSourceQueueHead( AL_source *src, ALuint bid )
 *
 * Truncates a source's queue with a single entry of bid.
 */
void _alSourceQueueHead( AL_source *src, ALuint bid );

/*
 * _alSourceQueueInit( AL_source *src )
 *
 * Initializes a source's queue.
 */
void _alSourceQueueInit( AL_source *src );

/*
 * _alSourceQueueClear( AL_source *src )
 *
 * Clears a source's queue, removing all entries.
 */
void _alSourceQueueClear( AL_source *src );

/*
 * _alSourceQueueAppend( AL_source *src, ALuint bid )
 *
 * Append bid to source's queue.
 */
void _alSourceQueueAppend( AL_source *src, ALuint bid );

/*
 * _alSourceUnqueueBuffers( ALuint sid, ALsizei n, ALuint *bids )
 *
 * Non locking version of alSourceUnqueueBuffers.
 */
void _alSourceUnqueueBuffers( ALuint sid, ALsizei n, ALuint *bids );

#endif /* AL_QUEUE_H_ */
