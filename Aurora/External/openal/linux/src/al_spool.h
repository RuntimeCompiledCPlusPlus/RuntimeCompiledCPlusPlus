/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_spool.h
 *
 * Prototypes, macros and definitions related to the management of spool
 * objects.  spool objects are objects which ease the slab allocation of
 * AL_source objects.
 *
 */
#ifndef _AL_SPOOL_H_
#define _AL_SPOOL_H_

#include "al_types.h"

#include <sys/types.h>

/*
 * source pool stuff
 *
 * overcomplicated method of pooling sources to avoid memory
 * fragmentation.
 *
 */

/*
 * spool_init( spool_t *spool )
 *
 * Creates, initializes and returns source pool object.
 */
void spool_init( spool_t *spool );

/*
 * spool_alloc( spool_t *spool )
 *
 * Allocates a source pool node and returns and index that can be used in
 * other spool_ calls.
 */
int spool_alloc( spool_t *spool );

/* 
 * spool_free( spool_t *spool, void (*freer_func)(void *) );
 *
 * Deallocates all source pool nodes in a source pool object.
 */
void spool_free( spool_t *spool, void (*freer_func)(void *) );

/*
 * spool_first_free_index( spool_t *spool )
 *
 * Returns the index of first unused source pool node.
 */
int spool_first_free_index( spool_t *spool );

/*
 * spool_index( spool_t *spool, ALuint sindex )
 *
 * retrieve the source pool node associated with sindex from the source pool
 * object spool.
 */
AL_source *spool_index( spool_t *spool, ALuint sindex );

/*
 * spool_sid_to_index( spool_t *spool, ALuint sid )
 *
 * Converts an AL_source name (sid) to the source pool node index suitable
 * for passing to spool_index.
 */
int spool_sid_to_index( spool_t *spool, ALuint sid );

/*
 * spool_next_id( void )
 *
 * Returns next suitable AL_source name (id) suitable for using in
 * al calls ( alSourcei( sid, ... ), etc )
 */
ALuint spool_next_id( void );

/*
 *  spool_dealloc( spool_t *spool, ALuint sid,
 *                 void (*freer_func)(void *) );
 *
 * Finalizes a source pool node, keyed off of sid, in spool, using freer_func.
 */
ALboolean spool_dealloc( spool_t *spool, ALuint sid,
				void (*freer_func)(void *) );

/*
 * spool_resize( spool_t *spool, size_t size )
 *
 * Increases the size of source pool object spool to accomodate at least size
 * source pool nodes.
 */
ALboolean spool_resize( spool_t *spool, size_t size );

#endif /* AL_SPOOL_H_ */
