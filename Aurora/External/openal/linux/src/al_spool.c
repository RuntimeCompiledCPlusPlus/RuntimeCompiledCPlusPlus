#include "al_siteconfig.h"
#include "al_debug.h"
#include "al_source.h"
#include "al_spool.h"
#include <stdlib.h>
#include <sys/types.h>

/*
 *  Resize spool to at least newsize units
 */
ALboolean spool_resize(spool_t *spool, size_t newsize) {
	void *temp;
	unsigned int i;

	if(newsize < 1) {
		newsize = 1;
	}

	if(spool->size >= newsize) {
		return AL_TRUE; /* no resize needed */
	}

	/*
	 * Resize the AL_source pool.
	 */
	temp = realloc(spool->pool, newsize * sizeof *spool->pool);
	if(temp == NULL) {
		return AL_FALSE; /* could not realloc */
	}
	spool->pool = temp;

	/*
	 * Resize the sid -> index map.
	 */
	temp = realloc(spool->map, newsize * sizeof *spool->map);
	if(temp == NULL) {
		return AL_FALSE; /* could not resize map */
	}
	spool->map = temp;

	/*
	 * Resize the mutex locks
	 */
	temp = realloc( spool->smutexen, newsize * sizeof *spool->smutexen );
	if( temp == NULL ) {
		return AL_FALSE;
	}
	spool->smutexen = temp;

	/*
	 * set defaults for newly allocated stuff
	 */
	for(i = spool->size; i < newsize; i++) {
		spool->pool[i].inuse = AL_FALSE;       /* use flag  */
		spool->map[i] = 0;                     /* unset sid */
		spool->smutexen[i] = mlCreateMutex();  /* new mutex */
	}

	spool->size = newsize;

	return AL_TRUE;
}

/*
 *  Reserve an AL_source in spool, returning the index of the AL_source,
 *  and mark it in use.  Return -1 if no reservation is possible.
 */
int spool_alloc(spool_t *spool) {
	int sindex;

	sindex = spool_first_free_index(spool);
	if(sindex == -1) {
		if(spool_resize(spool, spool->size * 2) == AL_FALSE) {
			return -1;
		}
		
		sindex = spool_first_free_index(spool);
	}

	spool->pool[sindex].inuse = AL_TRUE;
	spool->map[sindex] = spool_next_id();

	return spool->map[sindex];
}

/*
 *  Get the AL_source from sourcepool spool at index index, if
 *  it's in use.  Otherwise, return NULL.
 */
AL_source *spool_index(spool_t *spool, ALuint sid) {
	int sindex;

	sindex = spool_sid_to_index(spool, sid);

	if(sindex < 0) {
		return NULL;
	}

	if(sindex >= (int) spool->size) {
		return NULL;
	}

	if(spool->pool[sindex].inuse == AL_FALSE) {
		return NULL;
	}

	return &spool->pool[sindex].data;
}

int spool_first_free_index(spool_t *spool) {
	unsigned int i;

	for(i = 0; i < spool->size; i++) {
		if(spool->pool[i].inuse == AL_FALSE) {
			return i;
		}
	}

	return -1;
}

ALboolean spool_dealloc(spool_t *spool, ALuint sid,
				void (*freer_func)(void *)) {
	AL_source *src;
	int sindex;

	sindex = spool_sid_to_index(spool, sid);
	if(sindex < 0) {
		/* sid is invalid */
		return AL_FALSE;
	}

	if(sindex >= (int) spool->size) {
		return AL_FALSE;
	}

	src = spool_index(spool, sid);
	if(src == NULL) {
		_alDebug(ALD_SOURCE,
			__FILE__, __LINE__,
			"sid %d is a bad index", sid);
		return AL_FALSE;
	}

	if(spool->pool[sindex].inuse == AL_FALSE) {
		/* already deleted */
		return AL_FALSE;
	}

	freer_func(src);

	spool->pool[sindex].inuse = AL_FALSE;

	return AL_TRUE;
}

void spool_free(spool_t *spool, void (*freer_func)(void *)) {
	unsigned int i;
	ALuint sid;

	for(i = 0; i < spool->size; i++) {
		if(spool->pool[i].inuse == AL_TRUE) {
			sid = spool->pool[i].data.sid;

			spool_dealloc(spool, sid, freer_func);
		}
	}

	if(spool->pool != NULL) {
		free(spool->pool);
		spool->pool = NULL;
	}

	if(spool->map != NULL) {
		free(spool->map);
		spool->map = NULL;
	}

	spool->size = 0;

	/* let the caller free spool itself */
	return;
}

void spool_init(spool_t *spool) {
	if(spool == NULL) {
		return;
	}

	spool->size     = 0;
	spool->pool     = NULL;
	spool->map      = NULL;
	spool->smutexen = NULL;

	return;
}

/*
 * Get unique id.
 */
ALuint spool_next_id(void) {
	static ALuint id = AL_FIRST_SOURCE_ID;

	return ++id;
}

/*
 * Convert the sid to an index in spool's pool.  This is
 * to enable unique sids but reuse the indexes.
 *
 * FIXME: use binary search.
 */
int spool_sid_to_index(spool_t *spool, ALuint sid) {
	unsigned int i;

	for(i = 0; i < spool->size; i++) {
		if(spool->map[i] == sid) {
			return i;
		}
	}

	return -1;
}
