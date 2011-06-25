#include "al_siteconfig.h"
#include "al_bpool.h"
#include "al_debug.h"

#include "alc/alc_context.h"
#include "alc/alc_speaker.h"

#include <stdlib.h>

#define AL_FIRST_BUFFER_ID  0x8000
#define MINBUFFERS          20

/*
 * _alBufferInit( AL_buffer *buf, ALuint bid )
 *
 * Initializes AL_buffer *buf to defaults, set its id to bid.
 */
static void _alBufferInit( AL_buffer *buf, ALuint bid );

/*
 * bpool_resize( bpool_t *bpool, size_t newsize )
 *
 * increase size of buffer pool object (bpool) until it can contain up to
 * newsize objects.  Return AL_TRUE if resize operation was successful,
 * AL_FALSE otherwise. 
 */
ALboolean bpool_resize( bpool_t *bpool, size_t newsize ) {
	void *temp;
	unsigned int i;

	if(newsize < 1) {
		newsize = 1;
	}

	if(bpool->size >= newsize) {
		return AL_TRUE; /* no resize needed */
	}

	/*
	 * Resize buffer pool
	 */
	temp = realloc(bpool->pool, newsize * sizeof *bpool->pool);
	if(temp == NULL) {
		return AL_FALSE; /* could not realloc */
	}

	bpool->pool = temp;

	for(i = bpool->size; i < newsize; i++) {
		bpool->pool[i].inuse = AL_FALSE;
	}

	/*
	 * resize the sid <-> index map.
	 */
	temp = realloc(bpool->map, newsize * sizeof *bpool->map);
	if(temp == NULL) {
		return AL_FALSE;
	}
	bpool->map = temp;

	for(i = bpool->size; i < newsize; i++) {
		bpool->map[i] = 0;
	}

	bpool->size = newsize;

	return AL_TRUE;
}

/* 
 * bpool_alloc( bpool_t *bpool )
 *
 * allocates a buffer pool node from bpool, returns index or -1
 * on error.
 */
int bpool_alloc( bpool_t *bpool ) {
	ALuint size = 0;
	int bindex = 0;

	bindex = bpool_first_free_index( bpool );
	if(bindex == -1)  {
		size = bpool->size + bpool->size / 2;

		if(size < MINBUFFERS) {
			size = MINBUFFERS;
		}

		if(bpool_resize(bpool, size) == AL_FALSE) {
			return -1;
		}
		
		bindex = bpool_first_free_index(bpool);
	}

	bpool->pool[bindex].inuse = AL_TRUE;
	bpool->map[bindex]        = bpool_next_bid();

	_alBufferInit(&bpool->pool[bindex].data, bpool->map[bindex]);

	return bpool->map[bindex];
}

/*
 * bpool_index( bpool_t *bpool, ALuint bindex )
 *
 * retrieve a buffer pool node from a buffer pool ( bpool ), with index
 * bindex.  Returns NULL if bindex is not a valid index into the bpool.
 *
 * NOTE: bindex is *not* a buffer name (bid).  It is a value returned from
 * a successful call to bpool_alloc.
 */
AL_buffer *bpool_index( bpool_t *bpool, ALuint bid ) {
	int bindex;

	bindex = bpool_bid_to_index( bpool, bid );
	if(bindex < 0) {
		return NULL;
	}

	if(bindex >= (int) bpool->size) {
		return NULL;
	}

	return &bpool->pool[bindex].data;
}

/*
 * bpool_first_free_index( bpool_t *bpool )
 *
 * returns index of first unused buffer pool node in bpool.
 */
int bpool_first_free_index( bpool_t *bpool ) {
	ALuint i;

	for( i = 0; i < bpool->size; i++ ) {
		if(bpool->pool[i].inuse == AL_FALSE) {
			return i;
		}
	}

	return -1;
}

/*
 * bpool_dealloc( bpool_t *bpool, ALuint bid, void (*freer_func)(void *) )
 *
 * finalizes a single buffer pool node, named by bid.  Returns AL_TRUE if the
 * buffer pool node was valid, in use, and was finalized.  Returns AL_FALSE if
 * the buffer pool node was invalid, not in use, or any other error occured.
 */
ALboolean bpool_dealloc( bpool_t *bpool, ALuint bid,
				void (*freer_func)(void *) ) {
	AL_buffer *src;
	int bindex;

	bindex = bpool_bid_to_index( bpool, bid );
	if(bindex < 0) {
		/* invalid bid */
		_alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
			"bid %d is a bad index", bid);

		return AL_FALSE;
	}

	if(bindex >= (int) bpool->size) {
		return AL_FALSE;
	}

	src = bpool_index(bpool, bid);
	if(src == NULL) {
		_alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
			"bid %d is a bad index", bid);
		return AL_FALSE;
	}

	if(bpool->pool[bindex].inuse == AL_FALSE) {
		/* already deleted */
		return AL_FALSE;
	}

	_alDebug(ALD_MEM, __FILE__, __LINE__, "freer_func'ing %d", bid);

	freer_func(src);

	bpool->pool[bindex].inuse = AL_FALSE;
	bpool->map[bindex] = 0;

	return AL_TRUE;
}

/*
 * bpool_free( bpool_t *bpool, void (*freer_func)(void *) )
 *
 * dealloc all buffer pool nodes in a buffer pool object, using freer_func to
 * finalize each node.
 */
void bpool_free( bpool_t *bpool, void (*freer_func)(void *) ) {
	ALuint bid;
	unsigned int i;

	for(i = 0; i < bpool->size; i++) {
		if(bpool->pool[i].inuse == AL_TRUE) {
			bid = bpool->map[i];

			bpool_dealloc(bpool, bid, freer_func);
		}
	}

	free( bpool->pool );
	bpool->pool = NULL;

	free( bpool->map );
	bpool->map = NULL;

	bpool->size = 0;

	/* let the caller free bpool itself */
	return;
}

/*
 * bpool_init( bpool_t *bpool )
 *
 * initializes buffer pool object (bpool).
 */
void bpool_init( bpool_t *bpool ) {
	bpool->size = 0;
	bpool->pool = NULL;
	bpool->map  = NULL;

	return;
}

/*
 * bpool_bid_to_index( bpool_t *bpool, ALuint bid )
 *
 * converts an AL_buffer name (bid) to buffer pool node index suitable
 * for passing to bpool_index.  Returns -1 if bid does not name a buffer id
 * within bpool.
 *
 * FIXME: use binary search.
 */
int bpool_bid_to_index( bpool_t *bpool, ALuint bid ) {
	unsigned int i;

	if ( bid > 0 ) {
		for(i = 0; i < bpool->size; i++) {
			if(bpool->map[i] == bid) {
				return i;
			}
		}
	}

	return -1;
}

/*
 * bpool_next_bid( void )
 *
 * return next suitable AL_buffer name (bid) suitable for using in
 * al calls ( alBufferData( bid, ... ), etc ).  These should be sequential
 * and unique.
 */
ALuint bpool_next_bid( void ) {
	static ALuint id = AL_FIRST_BUFFER_ID;

	return ++id;
}

/*
 * _alBufferInit( AL_buffer *buf, ALuint bid )
 *
 * Allocate and initialize a new buffer.  Probably shouldn't
 * be called directly but from add_bufnod (which subsequently
 * adds it to the pool data structure, which maintains these
 * things)
 *
 * assumes locked buffers
 */
static void _alBufferInit( AL_buffer *buf, ALuint bid ) {
	ALuint i;

	if( buf == NULL ) {
		return;
	}

	buf->num_buffers = _alcDCGetNumSpeakers();

	for(i = 0; i < buf->num_buffers; i++) {
		buf->orig_buffers[i] = NULL;
	}

	buf->bid            = bid;
	buf->flags          = ALB_NONE;
	buf->streampos      = 0;
	buf->appendpos	    = 0;

	buf->format         = canon_format;
	buf->freq           = canon_speed;
	buf->size           = 0;

	buf->callback                = NULL;
	buf->destroy_source_callback = NULL;
	buf->destroy_buffer_callback = NULL;

	buf->queue_list.sids    = NULL;
	buf->queue_list.size    = 0;
	buf->queue_list.items   = 0;

	buf->current_list.sids    = NULL;
	buf->current_list.size    = 0;
	buf->current_list.items   = 0;

	return;
}
