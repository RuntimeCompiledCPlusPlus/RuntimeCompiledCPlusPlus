/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_mspool.c
 *
 * Prototypes, macros and definitions related to the management of mspool
 * objects.  mspool objects are objects which ease the slab allocation of
 * _alMixSource objects.
 */
#include "al_debug.h"
#include "al_mspool.h"

#include <stdlib.h>

/*
 * _alMixPoolResize( _alMixPool *mspool, size_t newsize )
 *
 * Initializes an already allocated _alMixPool object.  Returns AL_TRUE,
 * inless initialization failed for some reason, in which case AL_FALSE is
 * returned.
 *
 * After a successful initialization, mspool will have the capacity for at
 * least newsize _alMixPoolNodes.
 */
ALboolean _alMixPoolResize(_alMixPool *spool, size_t newsize) {
	_alMixPoolNode *temp;
	unsigned int i;

	if(newsize < 1) {
		newsize = 1;
	}

	if(spool->size >= newsize) {
		return AL_TRUE; /* no resize needed */
	}

	if(spool->pool == NULL) {
		spool->pool = malloc(newsize * sizeof *spool->pool);
	} else  {
		temp = realloc(spool->pool, newsize * sizeof(_alMixPoolNode));
		if(temp == NULL) {
			return AL_FALSE; /* could not realloc */
		}

		spool->pool = temp;
	}

	for(i = spool->size; i < newsize; i++) {
		spool->pool[i].inuse = AL_FALSE;
	}

	spool->size = newsize;

	return AL_TRUE;
}

/*
 * _alMixPoolAlloc( _alMixPool *mspool )
 *
 * Initializes an already allocated _alMixPool object.  Returns index suitable
 * for calls which expect an index, or -1 on error.
 */
int _alMixPoolAlloc( _alMixPool *spool ) {
	int msindex;

	msindex = _alMixPoolFirstFreeIndex(spool);
	if(msindex == -1) {
		if(_alMixPoolResize(spool, spool->size * 2) == AL_FALSE) {
			return -1;
		}
		
		msindex = _alMixPoolFirstFreeIndex(spool);
	}

	spool->pool[msindex].inuse = AL_TRUE;

	return msindex;
}

/*
 * _alMixPoolIndex( _alMixPool *mspool, int msindex )
 *
 * Return _alMixSource from mspool using simple index, or NULL if msindex is
 * not a valid index or has not been flagged for use.
 */
_alMixSource *_alMixPoolIndex(_alMixPool *spool, int msindex) {
	if(spool->pool[msindex].inuse == AL_FALSE) {
		return NULL;
	}

	return &spool->pool[msindex].data;
}

/*
 * _alMixPoolFirstFreeIndex( _alMixPool *mspool )
 *
 * Returns first available index in mspool, or -1 if nothing is available.
 */
int _alMixPoolFirstFreeIndex(_alMixPool *spool) {
	ALuint i;

	for(i = 0; i < spool->size; i++) {
		if(spool->pool[i].inuse == AL_FALSE) {
			return i;
		}
	}

	return -1;
}

/*
 * _alMixPoolDealloc( _alMixPool *mspool, int msindex,
 *                   void (*freer_func)(void *))
 *
 * Finalize a _alMixSource, indexed by msindex, using freer_func,
 * from mspool, marking is as not in use.
 */
ALboolean _alMixPoolDealloc( _alMixPool *spool, int msindex,
			     void (*freer_func)(void *) ) {
	_alMixSource *src;

	if( msindex < 0 ) {
		return AL_FALSE;
	}

	src = _alMixPoolIndex( spool, msindex );
	if(src == NULL) {
		_alDebug(ALD_MIXER, __FILE__, __LINE__,
			"%d is a bad index", msindex);

		return AL_FALSE;
	}

	if(spool->pool[msindex].inuse == AL_FALSE) {
		_alDebug(ALD_MIXER, __FILE__, __LINE__,
			"index %d is not in use", msindex);

		/* already deleted */
		return AL_FALSE;
	}

	spool->pool[msindex].inuse = AL_FALSE;

	freer_func(src);

	return AL_TRUE;
}

/*
 * _alMixPoolFree( _alMixPool *mspool, void (*freer_func)(void *) )
 *
 * Finalizes each _alMixSource in the _alMixPool object, using freer_func.
 */
void _alMixPoolFree(_alMixPool *spool, void (*freer_func)(void *)) {
	unsigned int i;

	for(i = 0; i < spool->size; i++) {
		if(spool->pool[i].inuse == AL_TRUE) {
			_alMixPoolDealloc( spool, i, freer_func );
		}
	}

	free( spool->pool );
	spool->pool = NULL;

	spool->size = 0;

	/* let the caller free spool itself */

	return;
}
