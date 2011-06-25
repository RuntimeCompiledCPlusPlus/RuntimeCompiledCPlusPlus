/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_rctree.h
 *
 * Stuff related to the rctree data structure
 *
 */
#ifndef AL_RCTREE_H_
#define AL_RCTREE_H_

#define ALRC_MAXSTRLEN 90

#include "al_rcvar.h"

/*
 * The AL_rctree is the base type for the alrc language.
 */
typedef struct _AL_rctree {
	ALRcEnum type;

	union {
		ALboolean b;
		ALint   i;
		ALfloat f;
		ALuint ui;
		ALvoid *p;

		struct {
			char c_str[ALRC_MAXSTRLEN];
			int len;
		} str;
		struct _AL_rctree *(*proc)(struct _AL_rctree *env, struct _AL_rctree *args);
		struct {
			struct _AL_rctree *car;
			struct _AL_rctree *cdr;
		} ccell;
	} data;
} AL_rctree;

/*
 * alrc_prim is a typedef that aids in the handling of alrc primitives, which
 * provide a sort of foreign function interface.
 */
typedef	AL_rctree *(*alrc_prim)( AL_rctree *env, AL_rctree *args );

/*
 * _alRcTreeAlloc( void )
 *
 * Allocate, initialize, and return an AL_rctree object.
 */
AL_rctree *_alRcTreeAlloc( void );

/*
 * _alRcTreeFree( AL_rctree *node )
 *
 * Finalize and deallocate an AL_rctree object.
 */
void _alRcTreeFree( AL_rctree *node );

/*
 * _alRcTreeDestroyAll( void )
 *
 * Deallocates any and all AL_rctree objects creates thus far.
 */
void _alRcTreeDestroyAll( void );

#endif /* AL_RCTREE_H_ */
