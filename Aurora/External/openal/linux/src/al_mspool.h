/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_mspool.h
 *
 * Prototypes, macros and definitions related to the management of mspool
 * objects.  mspool objects are objects which ease the slab allocation of
 * _alMixSource objects.
 */
#ifndef AL_MSPOOL_H_
#define AL_MSPOOL_H_

#include <AL/altypes.h>
#include <stdlib.h>

/*
 *  MixSource state info.
 */
typedef enum _alMixSourceEnum {
	ALM_PLAY_ME        = (1<<0),
	ALM_DESTROY_ME     = (1<<1),
	ALM_STREAMING	   = (1<<2)
} _alMixSourceEnum;

/*
 * _alMixSource is the data structure that contains an entry in the mixing
 * queue.  Entries have a context id and source id, as well as some processing
 * flags.
 */
typedef struct _alMixSource {
	ALuint context_id;
	ALuint sid;
	_alMixSourceEnum flags;
} _alMixSource;

/*
 * The _alMixPool is a type used to facilitate slab allocation of _alMixSource
 * entries.
 */
typedef struct _alMixPoolNode {
	_alMixSource data;
	ALboolean inuse;
} _alMixPoolNode;

/*
 * The _alMixPool is a type which contains _alMixPoolNode, in order to
 * facilitate slab allocation.
 */
typedef struct _alMixPool {
	_alMixPoolNode *pool;
	ALuint size;
} _alMixPool;

/*
 * _alMixPoolAlloc( _alMixPool *mspool )
 *
 * Initializes an already allocated _alMixPool object.
 */
int _alMixPoolAlloc( _alMixPool *mspool );

/*
 * _alMixPoolResize( _alMixPool *mspool, size_t newsize )
 *
 * Initializes an already allocated _alMixPool object.  Returns AL_TRUE,
 * inless initialization failed for some reason, in which case AL_FALSE is
 * returned.
 */
ALboolean _alMixPoolResize( _alMixPool *mspool, size_t newsize );

/*
 * _alMixPoolDealloc( _alMixPool *mspool, int msindex,
 *                   void (*freer_func)(void *))
 *
 * Finalize a _alMixSource, indexed by msindex, using freer_func,
 * from mspool.
 */
ALboolean _alMixPoolDealloc( _alMixPool *mspool, int msindex,
			     void (*freer_func)(void *));

/*
 * _alMixPoolIndex( _alMixPool *mspool, int msindex )
 *
 * Return _alMixSource from mspool using simple index, or NULL if msindex is
 * not a valid index.
 */
_alMixSource *_alMixPoolIndex( _alMixPool *mspool, int msindex );

/*
 * _alMixPoolFirstFreeIndex( _alMixPool *mspool )
 *
 * Returns first available index in mspool, or -1 if nothing is available.
 */
int _alMixPoolFirstFreeIndex( _alMixPool *mspool );

/*
 * _alMixPoolFree( _alMixPool *mspool, void (*freer_func)(void *) )
 *
 * Finalizes each _alMixSource in the _alMixPool object, using freer_func.
 */
void _alMixPoolFree( _alMixPool *mspool, void (*freer_func)(void *) );

#endif /* AL_MSPOOL_H_ */
