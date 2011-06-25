/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_mixmanager.h
 *
 * Interface to the mixmanager.  The mix manager is an abstraction such
 * that repeated calls to mix audio data can be collected into one set,
 * so that optimizations can be performed based on the number of data
 * streams that need to be mixed simultaneously.
 *
 */
#ifndef _AL_MIXMANAGER_H_
#define _AL_MIXMANAGER_H_

#include "al_types.h"
#include "al_mixfunc.h"

/*
 * Our mix manager.
 *
 * By using a mix manager, I hope to pool together the calls to
 * mix audio, in essence loop unrolling on the fly.  Instead of
 * mixing each source's outdata directly onto the mix buffer, we
 * add an entry to the MixManager, which after it has collected
 * all sources for the mixer_iteration interval uses a MixAudio
 * call optimized for the number of sources present.
 *
 * Used in conjunction with MixFuncs
 */
typedef struct _AL_MixManager {
	alMixEntry *pool;
	ALuint size;
	ALuint index;
} ALMixManager;

/* Mix manager funcs */

/*
 * _alMixManagerInit( ALMixManager *mixman, ALuint size )
 *
 * Initializes the already allocated ALMixManager object *mixman, to
 * accomodate at least size entries.
 */
ALboolean _alMixManagerInit( ALMixManager *mixman, ALuint size );

/*
 * _alMixManagerDestroy( ALMixManager *mixman )
 *
 * Performs finalization on the ALMixManager object *mixman.
 */
void _alMixManagerDestroy( ALMixManager *mixman );

/*
 * _alMixManagerAdd( ALMixManager *mixman, ALvoid *dataptr, int bytes_to_write )
 *
 * Adds an entry to the ALMixManager object *mixman, with data dataptr of
 * length bytes_to_write in bytes.
 */
void _alMixManagerAdd( ALMixManager *mixman, ALvoid *dataptr, int bytes_to_write );

/*
 * _alMixManagerMix( ALMixManager *mixman, ALMixFunc *mf, ALvoid *dataptr )
 *
 * Mixes each entry in ALMixManager *mixman, using mixing functions from
 * ALMixFunc, populating dataptr.
 */
void _alMixManagerMix( ALMixManager *mixman, ALMixFunc *mf, ALvoid *dataptr );

#endif /* _AL_MIXMANAGER_H_ */
