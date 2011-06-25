/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_rctree.h
 *
 * Stuff related to the rctree data structure.
 */
#include "al_debug.h"
#include "al_config.h"
#include "al_rctree.h"
#include "al_siteconfig.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * AL_RcTreeNodes are used to help manage AL_rctrees.
 */
typedef struct _AL_RcTreeNode {
	ALuint size;
	ALuint freeslots;
	AL_rctree **data;
} AL_RcTreeNode;

/*
 * rlist is the data structure we use to manage AL_rcTreeNodes.
 */
static AL_RcTreeNode rlist = { 0, 0, NULL };

/*
 * rlist_add_rctree( AL_rctree *node )
 *
 * Adds a reference to node to the rlist.
 */
static void rlist_add_rctree(AL_rctree *node);

/*
 * rlist_delete_rctree( AL_rctree *node )
 *
 * Removes a reference to node from the rlist.
 */
static void rlist_delete_rctree( AL_rctree *node );

/*
 * rlist_realloc( void )
 *
 * Increases the size of rlist.
 */
static void rlist_realloc( void );

/*
 * _alRcTreeAlloc( void )
 *
 * Creates, initializes and returns an AL_rctree.
 */
AL_rctree *_alRcTreeAlloc( void ) {
	AL_rctree *retval;

	retval = malloc( sizeof *retval );
	if(retval == NULL) {
		/* _alDCSetError(AL_NO_MEMORY); */
		return NULL;
	}

	rlist_add_rctree( retval );

	retval->type      = ALRC_INVALID;
	retval->data.proc = 0;
	retval->data.ccell.car = NULL;
	retval->data.ccell.cdr = NULL;

	return retval;
}

/*
 * _alRcTreeFree( AL_rctree *node )
 *
 * Finalizes and deallocates an AL_rctree node, removing the rlist reference
 * at the same time.
 */
void _alRcTreeFree( AL_rctree *node ) {
	if(node == NULL) {
		return;
	}

	if( node->type == ALRC_CONSCELL ) {
		_alRcTreeFree( node->data.ccell.car );
		_alRcTreeFree( node->data.ccell.cdr );
	}
	
	/* remove reference from our list */
	rlist_delete_rctree( node );

	free( node );

	return;
}

/*
 * _alRcTreeDestroyAll( void )
 *
 * Deallocates any and all AL_rctree objects creates thus far.
 */
void _alRcTreeDestroyAll( void ) {
	ALuint i;

	for(i = 0; i < rlist.size; i++) {
		if(rlist.data[i] == NULL) {
			continue;
		}

		free(rlist.data[i]);
	}

	free(rlist.data);

	rlist.data      = NULL;
	rlist.size      = 0;
	rlist.freeslots = 0;

	return;
}

/*
 * rlist_add_rctree( AL_rctree *node )
 *
 * Adds a reference to node to the rlist.
 */
static void rlist_add_rctree( AL_rctree *node ) {
	ALuint i;

	if(rlist.freeslots <= 0) {
		rlist_realloc();
	}

	for(i = 0; i < rlist.size; i++) {
		if(rlist.data[i] == NULL) {
			rlist.freeslots--;
			rlist.data[i] = node;

			return;
		}
	}

	/* weird.  Do something here. */
	ASSERT(0);

	return;
}

/*
 * rlist_add_rctree( AL_rctree *node )
 *
 * Adds a reference to node to the rlist.
 */
static void rlist_delete_rctree( AL_rctree *node ) {
	ALuint i;

	for(i = 0; i < rlist.size; i++) {
		if(rlist.data[i] == node) {
			rlist.freeslots++;
			rlist.data[i] = NULL;

			return;
		}
	}

	/* not found */

	return;
}

/*
 * rlist_realloc( void )
 *
 * Increases the size of rlist.
 */
static void rlist_realloc( void ) {
	ALuint newsize = rlist.size * 2 + 1;
	ALuint i;
	void *temp;

	temp = realloc( rlist.data, newsize * sizeof *rlist.data );
	if(temp == NULL) {
		ASSERT(0);
		return;
	}

	rlist.data = temp;

	for(i = rlist.size; i < newsize; i++) {
		rlist.data[i] = NULL;
	}

	rlist.freeslots += newsize - rlist.size;

	rlist.size = newsize;

	return;
}
