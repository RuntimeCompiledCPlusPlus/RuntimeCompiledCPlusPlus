/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_queue.c
 *
 * Stuff related to the management and use of buffer queue.
 *
 * FIXME: clean this mess up
 */

#include "al_siteconfig.h"
#include "al_config.h"
#include "al_buffer.h"
#include "al_debug.h"
#include "al_error.h"
#include "al_main.h"
#include "al_source.h"
#include "al_queue.h"
#include "al_types.h"

#include "alc/alc_context.h"

#include <AL/al.h>
#include <stdlib.h>

/*
 * the dual array is a data structure useful for sorting an array of
 * items, but maintaining the index information so that the array can
 * be unsorted later.
 */
struct dualarray {
	ALuint id;
	int index;
};

/* sort a dualarray by the id */
static int dualarray_id_cmp( const void *a, const void *b );

/* sort a dualarray by the index (ie, undo the by-id sort */
static int dualarray_index_cmp( const void *a, const void *b );

/*
 * alSourceQueueBuffers( ALuint sid, ALsizei numBuffers, ALuint *bids )
 *
 * Queue bids[0..numBuffers-1] to the source named sid, setting
 * AL_INVALID_VALUE if numBuffers < 0, or AL_INVALID_NAME if either sid or and
 * bid in bids[0..numBuffers-1] is not a valid buffer.
 */
void alSourceQueueBuffers( ALuint sid, ALsizei numBuffers, ALuint *bids ) {
	AL_source *src;
	ALsizei i;

	if( numBuffers == 0) {
		/* with n == 0, we NOP */
		return;
	}

	if( numBuffers < 0) {
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "alSourceQueueBuffers: illegal n value %d\n", numBuffers );

		_alcDCLockContext();
		_alDCSetError( AL_INVALID_VALUE );
		_alcDCUnlockContext();

		return;
	}

	SOURCELOCK();

	src = _alDCGetSource( sid );
	if( src == NULL ) {
		_alDCSetError( AL_INVALID_NAME );

		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "alSourceQueueBuffers: invalid sid %d\n", sid );

		SOURCEUNLOCK();

		return;
	}

	_alLockBuffer();

	/* make sure that bids are valid */
	for( i = 0; i < numBuffers; i++ ) {
		if( _alIsBuffer( bids[i] ) == AL_FALSE ) {
			/*
			 * we have an invalid bid.  bid 0 is okay but the
			 * rest are not.
			 */
			if( bids[i] != 0 ) {
				_alUnlockBuffer();

				_alDCSetError( AL_INVALID_NAME );

				SOURCEUNLOCK();

				return;
			}
		}
	}

	/* now, append the bids */
	for( i = 0; i < numBuffers; i++ ) {
		if( bids[i] != 0 ) {
			/*
			 * only append non-0 bids, because we don't
			 * need them anyway.
			 */
		  	_alSourceQueueAppend( src, bids[i] );
		}
	}

	/* we're done */

	_alUnlockBuffer();
	SOURCEUNLOCK();

	return;
}

/*
 * alQueuei( ALuint sid, ALenum param, ALint i1 )
 *
 * Obsolete function.  Remove this.
 */
void alQueuei( ALuint sid, ALenum param, ALint i1 ) {
	AL_source *src;
	ALboolean inrange = AL_FALSE;

	SOURCELOCK();
	src = _alDCGetSource(sid);
	if(src == NULL) {
		_alDCSetError(AL_INVALID_NAME);
		SOURCEUNLOCK();

		return;
	}

	/*
	 * all calls to alQueuei specify either ALboolean parameters,
	 * which means that i1 is either AL_TRUE, AL_FALSE, or
	 * a buffer id.  So check for validity of i1 first, and
	 * set error if that's the case.
	 */
	switch(param) {
		case AL_LOOPING:
		case AL_SOURCE_RELATIVE:
		  inrange = _alCheckRangeb(i1);
		  break;
		case AL_BUFFER:
		  inrange = alIsBuffer(i1);
		  break;
		default:
		  /* invalid param,  error below. */
		  break;
	}

	if(inrange == AL_FALSE) {
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "alQueuei(%d, 0x%x, ...) called with invalid value %d",
		      sid, param, i1);

		_alDCSetError(AL_INVALID_VALUE);
		
		SOURCEUNLOCK();
		return;
	}

	switch(param) {
		/* FIXME: add loop count */
		case AL_BUFFER:
		  /* Append bid to end */
		  _alSourceQueueAppend(src, i1);
		  break;
		default:
		  _alDebug(ALD_SOURCE, __FILE__, __LINE__,
			"alQueuei: invalid or stubbed source param 0x%x",
			param);
			
		  _alDCSetError(AL_ILLEGAL_ENUM);
		  break;
	}

	SOURCEUNLOCK();

	return;
}

/*
 * _alSourceQueueGetCurrentState( AL_source *src )
 *
 * Returns the current AL_sourcestate of the current queue entry for AL_source
 * *src.
 *
 * assumes locked sources
 */
AL_sourcestate *_alSourceQueueGetCurrentState( AL_source *src ) {
	int index = src->bid_queue.read_index;

	return &src->bid_queue.queuestate[index];
}

/*
 * _alSourceQueueInit( AL_source *src )
 *
 * Initialize an AL_source queue.
 *
 * assumes locked context
 */
void _alSourceQueueInit( AL_source *src ) {
	src->bid_queue.queue       = NULL;
	src->bid_queue.queuestate  = NULL;
	src->bid_queue.size        = 0;

	_alSourceQueueClear( src );

	return;
}

/*
 * _alSourceQueueClear( AL_source *src )
 *
 * Clears a source's queue, removing all entries.
 *
 * assumes locked context
 */
void _alSourceQueueClear( AL_source *src ) {
	ALuint bid;
	int i;

	for(i = 0; i < src->bid_queue.size; i++) {
		bid = src->bid_queue.queue[i];

		if(bid != 0) {
			/* bid 0 is a special name */
			_alBidRemoveQueueRef(bid, src->sid);
		}
	}

	src->bid_queue.read_index  = 0;
	src->bid_queue.write_index = 0;
	src->bid_queue.size        = 0;

	_alSourceQueueAppend(src, 0);

	return;
}

/*
 * _alSourceQueueAppend( AL_source *src, ALuint bid )
 *
 * Append bid to source's queue.
 *
 * assumes locked context
 */
void _alSourceQueueAppend( AL_source *src, ALuint bid ) {
	int size    = src->bid_queue.size;
	int newsize = size + 1;
	int windex  = src->bid_queue.write_index;
	void *temp;

	if(src->bid_queue.size > 0) {
		if(src->bid_queue.queue[windex] == 0) {
			/*
			 * special case.  bid == 0 is the "no bid"
			 * bid, which means that it's just a place
			 * holder to allow buffer specific source
			 * parameters to be set.
			 *
			 * Don't bother to resize, just overwrite.
			 */
			src->bid_queue.queue[windex] = bid;

			return;
		}
	}

	temp = realloc(src->bid_queue.queue,
		       newsize * sizeof *src->bid_queue.queue);
	if(temp == NULL) {
		return;
	}
	src->bid_queue.queue = temp;
	src->bid_queue.queue[size] = 0;

	temp = realloc(src->bid_queue.queuestate,
		       newsize * sizeof *src->bid_queue.queuestate);
	if(temp == NULL) {
		return;
	}
	src->bid_queue.queuestate = temp;

	/*
	 * If this is a "real" append operation, ie not bid == NONE,
	 * then increment the write index so that buffer specific
	 * source setting operations take place.
	 */
	if(bid != 0) {
		windex++;
		src->bid_queue.write_index++;
	}

	/*
	 * Initialize sourcestate flags.
	 */
	_alSourceStateInit(&src->bid_queue.queuestate[windex]);

	/*
	 * Set bid and new size.
	 */
	src->bid_queue.queue[windex] = bid;
	src->bid_queue.size = newsize;


	return;
}


/*
 * _alSourceQueueHead( AL_source *src, ALuint bid )
 *
 * Truncates a source's queue with a single entry of bid.
 *
 * assumes locked context
 */
void _alSourceQueueHead( AL_source *src, ALuint bid ) {
	_alSourceQueueClear( src );

	src->bid_queue.queue[0]    = bid;
	src->bid_queue.write_index = 0;
	src->bid_queue.read_index  = 0;
	src->bid_queue.size        = 1;

	return;
}

/*
 * alSourceUnqueueBuffers( ALuint sid, ALsizei n, ALuint *bids )
 *
 * Unqueues first occurance of each bid in bids[0..n-1] from source named sid.
 */
void alSourceUnqueueBuffers( ALuint sid, ALsizei n, ALuint *bids ) {
	_alcDCLockContext();

	_alSourceUnqueueBuffers( sid, n, bids );

	_alcDCUnlockContext();

	return;
}

/*
 * unqueueing demands that the buffer ids be convulted, and to 
 * that effect initially sorted.  However, because their initial
 * order is significant we need to unsort the buffer once we're
 * done messing with it.  So we use the dualarray.
 *
 */
static int dualarray_id_cmp( const void *a, const void *b ) {
	ALuint ai = ((const struct dualarray *) a)->id;
	ALuint bi = ((const struct dualarray *) b)->id;

	return ai - bi;
}
	
static int dualarray_index_cmp( const void *a, const void *b ) {
	int ai = ((const struct dualarray *) a)->index;
	int bi = ((const struct dualarray *) b)->index;

	return ai - bi;
}

/*
 * _alSourceUnqueueBuffers( ALuint sid, ALsizei n, ALuint *bids )
 *
 * Non locking version of alSourceUnqueueBuffers.
 *
 * assumes locked context
 */
void _alSourceUnqueueBuffers(ALuint sid, ALsizei n, ALuint *bids ) {
	AL_source *src;
	ALuint *tempqueue;
	AL_sourcestate *tempstate;
	struct dualarray *dualbuffers; /* the source's bid queue */
	struct dualarray *dualbids;   /* the passer's bids */
	int newsize;
	int i;
	int j;

	if(n == 0) {
		/* legal nop */
		return;
	}

	if(n < 0) {
		/* bad n */
		_alDCSetError( AL_INVALID_VALUE );

		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
			"alSourceUnqueueBuffers: invalid numBuffers %d", n );

		return;
	}

	src = _alDCGetSource( sid );
	if(src == NULL) {
		_alDCSetError(AL_INVALID_NAME);
		return;
	}

	if(n >= src->bid_queue.read_index) {
		/* User has requested too many buffers unqueued */
		_alDCSetError( AL_INVALID_VALUE );

		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
			"alSourceUnqueueBuffers: invalid numBuffers %d", n );

		return;
	}

	dualbuffers = malloc( src->bid_queue.size * sizeof *dualbuffers );
	if( dualbuffers == NULL ) {
		_alDCSetError( AL_OUT_OF_MEMORY );

		return;
	}

	dualbids =  malloc( n * sizeof *dualbuffers );
	if( dualbids == NULL ) {
		free( dualbuffers );
		_alDCSetError( AL_OUT_OF_MEMORY );

		return;
	}

	/* mark */
	for( i = 0; i < src->bid_queue.size; i++ ) {
		dualbuffers[i].id = src->bid_queue.queue[i];
		dualbuffers[i].index = i;
	}

	for( i = 0; i < n; i++ ) {
		dualbids[i].id = bids[i];
		dualbids[i].index = i;
	}

	/*
	 * sort both the source's bids and the passer's deletion
	 * request.
	 */
	qsort( dualbuffers, src->bid_queue.size,
		sizeof *dualbuffers, dualarray_id_cmp );
	qsort( dualbids, n, sizeof *dualbids, dualarray_id_cmp );

	/* sweep */
	newsize = src->bid_queue.size;
	for( i = 0, j = 0; i < n; i++ ) {
		if( dualbids[j].id == dualbuffers[i].id ) {
			dualbuffers[i].id    = 0;
			dualbuffers[i].index = -1;

			_alBidRemoveQueueRef( dualbids[j].id, src->sid );

			j++;
			newsize--;
		}
	}
			
	tempqueue = malloc(newsize * sizeof *tempqueue);
	if(tempqueue == NULL) {
		/*
		 * ouch
		 */
		_alDCSetError( AL_INVALID_VALUE );

		free( dualbids );
		free( dualbuffers );

		return;

	}

	tempstate = malloc(newsize * sizeof *tempstate);
	if(tempstate == NULL) {
		/* too late
		 *
		 * FIXME: re-add queuerefs removed above */
		_alDCSetError(AL_INVALID_VALUE);

		free( tempqueue );
		free( dualbids );
		free( dualbuffers );

		return;

	}

	/* unsort */
	qsort( dualbuffers, src->bid_queue.size,
		sizeof *dualbuffers, dualarray_index_cmp );

	/* rewrite */

	/* first, skip the 0'd out stuff */
	for( i = 0; dualbuffers[i].id == 0; i++ ) {
		/*
		 * ouch.  not sure about read_index or write_index
		 * moving.
		 */
		if(src->bid_queue.read_index >= i) {
			src->bid_queue.read_index--;
		}

		if(src->bid_queue.write_index >= i) {
			src->bid_queue.write_index--;
		}
	}

	for(j = 0; j < newsize; j++, i++) {
		tempqueue[j] = src->bid_queue.queue[i];
		tempstate[j] = src->bid_queue.queuestate[i];
	}

	/* free and assign */
	free( dualbuffers );
	free( dualbids );

	free( src->bid_queue.queue );
	free( src->bid_queue.queuestate );

	src->bid_queue.queue      = tempqueue;
	src->bid_queue.queuestate = tempstate;
	src->bid_queue.size       = newsize;

	return;
}

/*
 * _alSourceStateInit( AL_sourcestate *srcstate )
 *
 * Initialize an AL_sourcestate object to the default settings.
 */
void _alSourceStateInit( AL_sourcestate *srcstate ) {
	ALboolean tempbool;
	ALboolean err;

	srcstate->flags = ALQ_NONE;

	/*
	 * if there is a setting for islooping, use it.
	 * Otherwise, set to defaults.
	 */
	err = _alGetGlobalScalar("source-looping", ALRC_BOOL, &tempbool);
	if(err == AL_FALSE) {
		/* no preset looping */
		srcstate->islooping.data   = AL_FALSE;
		srcstate->islooping.isset  = AL_FALSE;
	} else {
		srcstate->islooping.data  = tempbool;
		srcstate->islooping.isset = AL_TRUE;
	}

	return;
}

