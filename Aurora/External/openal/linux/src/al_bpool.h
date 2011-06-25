/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_bpool.h
 *
 * Prototypes, macros and definitions related to the management of bpool
 * objects.  bpool objects are objects which ease the slab allocation of
 * AL_buffer objects.
 *
 */
#ifndef _AL_BPOOL_H_
#define _AL_BPOOL_H_

#include "al_types.h"

/*
 * pool structures: bpool_node and bpool_t
 *
 * These structures are used to group buffers in a growable array, so that
 * the relatively small allocations of AL_buffer objects can be combined into
 * a larger structure, in the hopes of reducing the effects of fragmentation.
 *
 * Each AL_buffer manipulated is actually a pointer to a bpool_node->data 
 * object.  The bpool->node inuse flag marks whether the AL_buffer in a bpool_node
 * object is currently "alloced" (in the sense that the application/library is
 * using it) or "dealloced".
 *
 * The bpool_t object is used to collect those variables needed to manage the
 * bpool_node pool, in point of fact the size and the data itself.  Also, a
 * mapping of buffer ids (used internally and by the application) is present
 * to facilitate the easy conversion from buffer ids (which are in essence
 * arbitrary other than their requirement to be unique) and indexes into 
 * the bpool_node pool.
 */
typedef struct {
	AL_buffer data;
	ALboolean inuse;
} bpool_node;

typedef struct {
	bpool_node *pool;
	ALuint size;
	ALuint *map; /* map[index] = bid */
} bpool_t;

/*
 * buffer pool stuff
 *
 * overcomplicated method of pooling buffers to avoid memory
 * fragmentation.
 */
 
/*
 * bpool_init( bpool_t *bpool )
 *
 * initializes buffer pool object (bpool).
 */
void bpool_init( bpool_t *bpool );

/* 
 * bpool_alloc( bpool_t *bpool )
 *
 * allocates a buffer pool node from bpool, returns index or -1
 * on error.
 */
int bpool_alloc( bpool_t *bpool );

/*
 * bpool_free( bpool_t *bpool, void (*freer_func)(void *) )
 *
 * dealloc all buffer pool nodes in a buffer pool object, using freer_func to
 * finalize each node.
 */
void bpool_free( bpool_t *bpool, void (*freer_func)(void *) );

/*
 * bpool_first_free_index( bpool_t *bpool )
 *
 * returns index of first unused buffer pool node in bpool.
 */
int bpool_first_free_index( bpool_t *bpool );

/*
 * bpool_dealloc( bpool_t *bpool, ALuint bid, void (*freer_func)(void *) )
 *
 * finalizes a single buffer pool node, named by bid.  Returns AL_TRUE if the
 * buffer pool node was valid, in use, and was finalized.  Returns AL_FALSE if
 * the buffer pool node was invalid, not in use, or any other error occured.
 */
ALboolean bpool_dealloc( bpool_t *bpool, ALuint bid,
				void (*freer_func)(void *) );

/*
 * bpool_resize( bpool_t *bpool, size_t newsize )
 *
 * increase size of buffer pool object (bpool) until it can contain up to
 * newsize objects.  Return AL_TRUE if resize operation was successful,
 * AL_FALSE otherwise. 
 */
ALboolean bpool_resize( bpool_t *bpool, size_t newsize );

/*
 * bpool_index( bpool_t *bpool, ALuint bindex )
 *
 * retrieve a buffer pool node from a buffer pool ( bpool ), with index
 * bindex.  Returns NULL if bindex is not a valid index into the bpool.
 *
 * NOTE: bindex is *not* a buffer name (bid).  It is a value returned from
 * a successful call to bpool_alloc.
 */
AL_buffer *bpool_index( bpool_t *bpool, ALuint bindex );

/*
 * bpool_bid_to_index( bpool_t *bpool, ALuint bid )
 *
 * converts an AL_buffer name (bid) to buffer pool node index suitable
 * for passing to bpool_index.  Returns -1 if bid does not name a buffer id
 * within bpool.
 */
int bpool_bid_to_index( bpool_t *bpool, ALuint bid );

/*
 * bpool_next_bid( void )
 *
 * return next suitable AL_buffer name (bid) suitable for using in
 * al calls ( alBufferData( bid, ... ), etc ).  These should be sequential
 * and unique.
 */
ALuint bpool_next_bid( void );

#endif /* AL_BPOOL_H_ */
