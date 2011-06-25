/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * alc_context.c
 *
 * Context management and application level calls.
 */
#include "al_siteconfig.h"

#include <AL/al.h>
#include <AL/alc.h>

#include "al_buffer.h"
#include "al_config.h"
#include "al_debug.h"
#include "al_distance.h"
#include "al_listen.h"
#include "al_main.h"
#include "al_mixer.h"
#include "al_spool.h"
#include "al_source.h"
#include "al_types.h"
#include "al_filter.h"

#include "alc_context.h"
#include "alc_device.h"
#include "alc_speaker.h"
#include "alc_error.h"

#include "mutex/mutexlib.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string.h>

#include "arch/interface/interface_sound.h"

/*
 * CONTEXT_BASE is the number which we start at for context ids.
 */
#define CONTEXT_BASE   0x9000

/*
 * canon_max and canon_min are the max/min values for PCM data in our
 * canonical format, respectively.
 */
const int canon_max = ((1<<(16-1))-1),
	  canon_min = -(1<<(16-1));

/*
 * canon_format is the canonical format that we represent data internally as.
 */
ALenum canon_format = _ALC_CANON_FMT;

/*
 * canon_speed is the sampling rate at which we internally represent data.
 */
ALuint canon_speed = _ALC_CANON_SPEED;

/*
 * _alcCCId holds the context id of the current context.
 */
ALuint _alcCCId = (ALuint) -1;

/*
 * al_contexts is our context pool.
 */
static struct {
	ALuint size;
	ALuint items;

	ALuint *map;
	ALboolean *inuse;
	AL_context *pool;
} al_contexts = { 0, 0, NULL, NULL, NULL };

/*
 * all_context_mutex is the mutex guarding operations which require all the
 * contexts to be locked.
 */
static MutexID all_context_mutex = NULL;

/*
 * contex_mutexen is a set of mutexes, one for each context, guarding each
 * contex.
 */
static MutexID *context_mutexen  = NULL;

/*
 * _alcDestroyContext( AL_context *cc )
 *
 * Non locking version of alcDestroyContext.
 */
static ALCenum _alcDestroyContext( AL_context *cc );

/*
 * _alcReallocContexts( ALuint newsize )
 *
 * Increases data structures to accomodate at least newsize contexts.
 */
static void _alcReallocContexts( ALuint newsize );

/*
 * _alcGenerateNewCid( void )
 *
 * Returns a new unique ALuint suitable for use as a cid.
 */
static ALuint _alcGenerateNewCid( void );

/*
 * _alcCidToIndex( ALuint cid )
 *
 * Converts cid into a simple index, returning that.
 */
static ALuint _alcCidToIndex( ALuint cid );

/*
 * _alcIndexToCid( int index )
 *
 * Converts index to a cid, returning that.
 */
static ALuint _alcIndexToCid( int cindex );

/*
 * _alcDeviceReadSet( ALuint cid )
 *
 * Apply parameters for the read device associated with the context named by
 * cid.
 */
static void _alcDeviceReadSet( ALuint cid );

/*
 * _alcDeviceWriteSet( ALuint cid )
 *
 * Apply parameters for the write device associated with the context named by
 * cid.
 */
static void _alcDeviceWriteSet( ALuint cid );

#ifdef JLIB
unsigned int jlib_debug = 0;
#endif

/*
 * VOIDP_TO_ALUINT and ALUINT_TO_VOIDP are macros to ease the conversion of
 * void * to ALuint and visa versa.
 */
#if SIZEOF_VOID_P == 8
#define VOIDP_TO_ALUINT(vp) ((ALuint) (int64_t) (vp))
#define ALUINT_TO_VOIDP(al) ((void *) (int64_t) (al))
#else
#define VOIDP_TO_ALUINT(vp) ((ALuint) (vp))
#define ALUINT_TO_VOIDP(al) ((void *) (al))
#endif

/*
 * alcMakeContextCurrent( ALvoid *handle )
 *
 * Makes the context refered to by handle the current context.  If handle does
 * not refer to a context, ALC_INVALID_CONTEXT is set and returned.
 * Otherwise, the operation sucedes and ALC_NO_ERROR is returned.
 */
ALCenum alcMakeContextCurrent( ALvoid *handle ) {
	AL_context *cc;
	int cid;
	static ALboolean ispaused = AL_FALSE;
	ALboolean should_init_mixer = AL_FALSE;

	if(handle == NULL) {
		/* NULL handle means pause */
		if(ispaused == AL_FALSE) {
			if(al_contexts.items != 0) {
				/* only lock if a context has been
				 * created.  Otherwise, don't.
				 */

				/* Give mixer thread chance to catch up */

				_alLockMixerPause();

				_alcLockAllContexts();

				cc = _alcDCGetContext();
				if( cc == NULL ) {
					/* I don't even want to think about it */

					/* FIXME: wrong error */
					_alcSetError( ALC_INVALID_CONTEXT );

					_alcUnlockAllContexts();
					
					return ALC_INVALID_CONTEXT;
				}
				
				/*
				 * inform current audio device about
				 * impending stall.
				 */
				_alcDevicePause( cc->write_device );
				_alcDevicePause( cc->read_device );
				
				_alcCCId = (ALuint) -1;
				_alcUnlockAllContexts();
			}

			ispaused = AL_TRUE;
		}

		return ALC_NO_ERROR;
	}

	cid = VOIDP_TO_ALUINT( handle );

	_alcLockAllContexts();

	if( _alcIsContext( _alcCCId ) == AL_FALSE ) {
		should_init_mixer = AL_TRUE;
	}

	_alcCCId = cid;

	cc = _alcGetContext( cid );
	if( cc == NULL ) {
		/* I don't even want to think about it */

		/* FIXME: wrong error */
		_alcSetError( ALC_INVALID_CONTEXT );
		_alcUnlockAllContexts( );
					
		return ALC_INVALID_CONTEXT;
	}
	
	if( should_init_mixer == AL_TRUE ) {
		/* Set up mixer thread */
		if(_alInitMixer() == AL_FALSE) {
			/* do something */
		}
	}

	/* set mixer */
	_alcDeviceWriteSet( cid );
	_alcDeviceReadSet( cid );

	if(ispaused == AL_TRUE) {
		/* someone unpaused us */
		ispaused = AL_FALSE;

		_alcDeviceResume( cc->write_device );
		_alcDeviceResume( cc->read_device );

		_alcUnlockAllContexts();
		_alUnlockMixerPause();
	} else {
		/* just unlock contexts */
		_alcUnlockAllContexts();
	}

	return ALC_NO_ERROR;
}

/*
 * alcDestroyContext( ALvoid *handle )
 *
 * Destroys the context referred to by handle.
 */
ALCenum alcDestroyContext( ALvoid *handle ) {
	AL_context *cc;
	ALCenum retval = ALC_NO_ERROR;
	int cid;

	if( handle == NULL ) {
		return ALC_INVALID_CONTEXT;
	}

	cid = VOIDP_TO_ALUINT( handle );

	_alcLockContext( cid );
	cc = _alcGetContext( cid );
	if(cc == NULL) {
		_alcUnlockContext( cid );
		return ALC_INVALID_CONTEXT;
	}

	/*
	 * If this is the last context, run _alExit()
	 * to clean up the cruft
	 */
	if( al_contexts.items == 1 ) {
		/* unlock context for final time */
		_alcUnlockContext( cid );

		/* cleanup */
		_alExit();

		/*
		 * Set NumContexts to 0
		 */
		al_contexts.items = 0;

		/*
		 * Destroy the all-locking-contexts
		 */
		mlDestroyMutex( all_context_mutex );
		all_context_mutex = NULL;

		return retval;
	}

	/* call internal destroyer */
	retval = _alcDestroyContext( cc );

	/*
	 * Decrement the number of contexts in use.
	 */
	al_contexts.items--;

	_alcUnlockContext( cid );

	return retval;
}

/**
 * alcProcessContext( ALvoid *alcHandle )
 *
 * Performs processing on a synced context, nop on a asynchronous
 * context.
 *
 * If alcHandle is not valid, ALC_INVALID_CONTEXT is returned.
 */
void *alcProcessContext( ALvoid *alcHandle ) {
	AL_context *cc;
	ALboolean should_sync;
	int cid;

	if( alcHandle == NULL ) {
		/*
		 * invalid name?
		 */
		_alDebug(ALD_CONTEXT, __FILE__, __LINE__,
		      "alcUpdateContext: alcHandle == NULL");

		_alcSetError( ALC_INVALID_CONTEXT );
		return NULL;
	}

	cid = VOIDP_TO_ALUINT( alcHandle );

	/* determine whether we need to sync or not */
	_alcLockAllContexts();

	cc = _alcGetContext( cid );
	if(cc == NULL) {
		_alDebug( ALD_CONTEXT, __FILE__, __LINE__,
			  "alcUpdateContext: invalid context id %d",
			  cid );

		_alcSetError( ALC_INVALID_CONTEXT );

		_alcUnlockAllContexts();

		return NULL;
	}

	should_sync = cc->should_sync;
	_alcUnlockAllContexts();

	if( should_sync == AL_TRUE ) {
		mixer_iterate( NULL );
	} else {
		/* unsuspend async contexts */
		cc->issuspended = AL_FALSE;
	}

	return alcHandle;
}

/**
 *
 * alcSuspendContext( ALvoid *alcHandle )
 *
 * Suspends processing on an asynchronous context.  This is a legal nop on a
 * synced context.
 *
 * If alcHandle is not valid, ALC_INVALID_CONTEXT is returned.
 */
void alcSuspendContext( ALvoid *alcHandle ) {
	AL_context *cc;
	ALuint cid;

	if( alcHandle == NULL ) {
		/*
		 * invalid name?
		 */
		_alDebug(ALD_CONTEXT, __FILE__, __LINE__,
		      "alcUpdateContext: alcHandle == NULL");

		_alcSetError( ALC_INVALID_CONTEXT );

		return;
	}

	cid = VOIDP_TO_ALUINT( alcHandle );

	/* determine whether we need to sync or not */
	_alcLockAllContexts();

	cc = _alcGetContext( cid );
	if( cc == NULL ) {
		_alDebug( ALD_CONTEXT, __FILE__, __LINE__,
			  "alcUpdateContext: invalid context id %d",
			  cid );

		_alcSetError( ALC_INVALID_CONTEXT );

		_alcUnlockAllContexts();

		return;
	}

	if( cc->should_sync == AL_FALSE) {
		/*
		 * only asynchronous contexts can be 
		 * suspended.
		 */
		cc->issuspended = AL_TRUE;
	}

	_alcUnlockAllContexts();

	return;
}

/*
 * alcCreateContext( struct _AL_device *dev, int *attrlist )
 *
 * Allocates, initialiaes, and returns an AL_context handle, suitable for
 * passing to other alc functions.  Uses dev as the write device for the
 * context.  attrlist is an int array, ALC_INVALID terminated, that contains
 * attribute/value pairs used to initialize the context.
 *
 * We use a meet-or-exceed system here.  If any attribute in attrlist cannot
 * have the required value met or exceeded, NULL is returned.  If dev is not
 * valid, ALC_INVALID_DEVICE is set and NULL is returned.
 *
 * FIXME: not as well tested as I'd like.
 */
void *alcCreateContext( struct _AL_device *dev, int *attrlist ) {
	ALint cid;

	if( dev == NULL ) {
		_alcSetError( ALC_INVALID_DEVICE );
		
		return NULL;
	}

	if( al_contexts.items == 0 ) {
		/*
		 * This is the first context to be created.  Initialize the
		 * library's data structures.
		 */
#ifdef JLIB
		if(getenv("JLIB_DEBUG")) {
			jlib_debug = atoi(getenv("JLIB_DEBUG"));
		}
#endif

		/* get a context name for the new context */
		cid = _alcGetNewContextId();

		/* misc library initialization */
		_alInit();

		/* set the context attributes */
		_alcLockContext( cid );
		_alcSetContext( attrlist, cid, dev );
		_alcUnlockContext( cid );

		return ALUINT_TO_VOIDP( cid );
	}

	_alcLockAllContexts();
	cid = _alcGetNewContextId();
	if(cid == -1) {
		_alDebug( ALD_CONTEXT, __FILE__, __LINE__,
			  "alcCreateContext failed." );

		_alcSetError( ALC_INVALID_DEVICE );
		_alcUnlockAllContexts();

		return NULL;
	}

	_alcUnlockAllContexts();

	_alcLockContext( cid );
	_alcSetUse( cid, AL_TRUE );
	_alcSetContext( attrlist, cid, dev );
	_alcUnlockContext( cid );

	return ALUINT_TO_VOIDP( cid );
}

/*
 * _alcDestroyContext( AL_context *cc )
 *
 * Non locking version of alcDestroyContext.
 *
 * FIXME: should assume that *all contexts* are locked?
 */
ALCenum _alcDestroyContext( AL_context *cc ) {

	_alDestroyListener( &cc->listener );
	_alDestroySources( &cc->source_pool );

  	return ALC_NO_ERROR;
}


/*
 * FL_alcLockContext( ALuint cid, const char *fn, int ln )
 *
 * Locks the mutex associated with the context named by cid, passing fn and ln
 * to _alLockPrintf for debugging purposes.
 */
void FL_alcLockContext(ALuint cid, UNUSED(const char *fn), UNUSED(int ln)) {
	int cindex;

	_alLockPrintf("_alcLockContext", fn, ln);

	cindex = _alcCidToIndex(cid);
	if( cindex < 0 ) {
		_alDebug(ALD_CONTEXT, __FILE__, __LINE__,
				"FL_alcLockContext: invalid context.");
		return;
	}

	_alcLockAllContexts();

	mlLockMutex(context_mutexen[cindex]);

	_alcUnlockAllContexts();

	return;
}

/*
 * FL_alcUnlockContext( ALuint cid, const char *fn, int ln )
 *
 * Unlocks the mutex associated with the context named by cid, passing fn and ln
 * to _alLockPrintf for debugging purposes.
 */
void FL_alcUnlockContext(ALuint cid, UNUSED(const char *fn), UNUSED(int ln)) {
	int cindex;

	_alLockPrintf("_alcUnlockContext", fn, ln);

	cindex = _alcCidToIndex( cid );
	if( cindex < 0 ) {
		_alDebug(ALD_CONTEXT, __FILE__, __LINE__,
				"FL_alcUnlockContext: invalid context.");
		return;
	}

	mlUnlockMutex(context_mutexen[cindex]);

	return;
}

/*
 * _alcGetContext( ALuint cid )
 *
 * Returns pointer to the AL_context named by cid, or NULL if cid is not a
 * valid context name.
 */
AL_context *_alcGetContext( ALuint cid ) {
	ALuint cindex;

	cindex = _alcCidToIndex(cid);

	if(cindex >= al_contexts.size) {
		return NULL;
	}

	if(al_contexts.inuse[cindex] == AL_FALSE) {
		return NULL;
	}

	return &al_contexts.pool[cindex];
}


/*
 * _alcSetContext( int *attrlist, ALuint cid, AL_device *dev )
 *
 * Sets context id paramaters according to an attribute list and device.
 *
 */
void _alcSetContext(int *attrlist, ALuint cid, AL_device *dev ) {
	AL_context *cc;
	ALboolean reading_keys = AL_TRUE;
	struct { int key; int val; } rdr;
	ALuint refresh_rate = 15;
        ALuint bufsiz;

	cc = _alcGetContext( cid );
	if(cc == NULL) {
		return;
	}

	/* Set our preferred mixer stats */
        if (dev->flags & ALCD_WRITE)
		cc->write_device = dev;
        if (dev->flags & ALCD_READ)
		cc->read_device = dev;

	while(attrlist && (reading_keys == AL_TRUE)) {
		rdr.key = *attrlist++;

		switch(rdr.key) {
			case ALC_FREQUENCY:
				rdr.val = *attrlist++;
				canon_speed = rdr.val;

				_alDebug( ALD_CONTEXT, __FILE__, __LINE__,
					"cc->external_speed = %d", rdr.val );
				break;
			case ALC_REFRESH:
				refresh_rate = *attrlist++;
				break;
			case ALC_SOURCES_LOKI:
				rdr.val = *attrlist++;
				spool_resize(&cc->source_pool, rdr.val);
								
				_alDebug(ALD_CONTEXT,
					__FILE__, __LINE__,
					"ALC_SOURCES (%d)", rdr.val);
				break;
			case ALC_BUFFERS_LOKI:
				rdr.val = *attrlist++;

				_alNumBufferHint( rdr.val );
				break;
			case ALC_SYNC:
				rdr.val = *attrlist++;

				if(rdr.val == AL_TRUE) {
					cc->should_sync = AL_TRUE;
				} else {
					cc->should_sync = AL_FALSE;
				}
				break;
			case ALC_INVALID:
				reading_keys = AL_FALSE;
				break;
			default:
				reading_keys = AL_FALSE;
				break;
				_alDebug(ALD_CONTEXT,
					__FILE__, __LINE__,
					"unsupported context attr %d",
					rdr.key );
				break;
		}
	}

	/* okay, now post process */

	if( refresh_rate > canon_speed ) {
		/*
		 * We don't accept refresh rates greater than the
		 * sampling rate.
		 */
		refresh_rate = canon_speed;
	}

	bufsiz = _alSmallestPowerOfTwo(
				(ALuint) ((float) canon_speed / refresh_rate));
        if (dev->flags & ALCD_WRITE)
		cc->write_device->bufsiz = bufsiz;
        if (dev->flags & ALCD_READ)
		cc->read_device->bufsiz = bufsiz;

	_alDebug( ALD_CONTEXT, __FILE__, __LINE__,
		"new bufsiz = %d", rdr.val);

	return;
}

/*
 * _alcInitContext( ALuint cid )
 *
 * Initialize the context named by cid, and returns the AL_context associated
 * with that id.
 *
 * assumes locked context
 */
AL_context *_alcInitContext( ALuint cid ) {
	AL_context *cc;

	cc = _alcGetContext(cid);
	if(cc == NULL) {
		/* invalid context */
		return NULL;
	}

	_alInitTimeFilters(cc->time_filters);

	cc->alErrorIndex   = AL_NO_ERROR;

	cc->doppler_factor    = 1.0;
	cc->doppler_velocity  = 1.0;

	_alInitListener(&cc->listener);

        _alcSpeakerInit( cid );

	/* nothing is enabled by default */
	cc->enable_flags = 0;

	/* Source initializations */
	spool_init(&cc->source_pool);

	cc->read_device = NULL;
	cc->write_device = NULL;

	/*
	 * should_sync:
	 * 	AL_FALSE:
	 * 		we use async_mixer_iterate, and don't need
	 * 		to have alcUpdateContext called to actually
	 * 		mix the audio.
	 * 	AL_TRUE:
	 * 		we use sync_mixer_iterate, and need to have
	 * 		alcUpdateContext called to actually mix the
	 * 		audio.
	 */
	cc->should_sync = AL_FALSE;
	cc->issuspended = AL_FALSE; /* deviates */

	/*
	 * set distance model stuff
	 */
	cc->distance_model = AL_INVERSE_DISTANCE;
	cc->distance_func  = _alDistanceInverse;

	return cc;
}

/*
 * _alcSetUse( ALuint cid, ALboolean value )
 *
 * Sets the use flag of context with id cid to value.
 *
 * Assumes context is locked
 *
 * NOTES:
 *    Can't use alcGetContext, because that checks the use flag,
 *    which maybe set to false, which is what this function seeks
 *    to correct.
 *
 */
ALboolean _alcSetUse(ALuint cid, ALboolean val) {
	ALuint cindex;

	cindex = _alcCidToIndex(cid);

	if(cindex >= al_contexts.size) {
		return !val;
	}

	return al_contexts.inuse[cindex] = val;
}

/*
 * _alcInUse( ALuint cid )
 *
 * Returns AL_TRUE if the context named by cid is in use, AL_FALSE otherwise.
 */
ALboolean _alcInUse(ALuint cid) {
	ALuint cindex;

	cindex = _alcCidToIndex(cid);

	if(cindex >= al_contexts.size) {
		return AL_FALSE;
	}

	return al_contexts.inuse[cindex];
}

/*
 * FL_alcLockAllContexts( const char *fn, int ln )
 *
 * Locks the mutex associated guarding all contexts, passing fn and ln to 
 * _alLockPrintf for debugging purposes.
 */
void FL_alcLockAllContexts(UNUSED(const char *fn), UNUSED(int ln)) {
	if( all_context_mutex == NULL ) {
		return;
	}

	_alLockPrintf("_alcLockAllContexts", fn, ln);
	mlLockMutex(all_context_mutex);
}

/*
 * FL_alcUnlockAllContexts( const char *fn, int ln )
 *
 * Unlocks the mutex associated guarding all contexts, passing fn and ln to 
 * _alLockPrintf for debugging purposes.
 */
void FL_alcUnlockAllContexts(UNUSED(const char *fn), UNUSED(int ln)) {
	if( all_context_mutex == NULL ) {
		return;
	}

	_alLockPrintf("_alcUnlockAllContexts", fn, ln);
	mlUnlockMutex(all_context_mutex);
}

/*
 * _alcGetListener( ALuint cid )
 *
 * Returns a pointer to the listener associated with context named by cid, or
 * NULL if cid does not name a valid context.
 *
 * assumes locked context
 */
AL_listener *_alcGetListener( ALuint cid ) {
	AL_context *cc;

	cc = _alcGetContext(cid);
	if(cc == NULL) {
		return NULL;
	}

	return &cc->listener;
}

/*
 * _alcDestroyAll( void )
 *
 * Deallocates the data structures for all contexts.
 */
void _alcDestroyAll( void ) {
	AL_context *freer;
	ALuint i;
	ALuint cid;

	for(i = 0; i < al_contexts.items; i++) {
		cid = _alcIndexToCid( i );

		if(context_mutexen[i] != NULL) {
			mlDestroyMutex( context_mutexen[i] );
			context_mutexen[i] = NULL;
		}

		if(_alcInUse(cid) == AL_TRUE) {
			freer = _alcGetContext( cid );

			if(freer != NULL) {
				_alcDestroyContext( freer );
			}
		}
	}

	free( context_mutexen );
	context_mutexen = NULL;

	free( al_contexts.map );
	free( al_contexts.pool );
	free( al_contexts.inuse );

	al_contexts.map   = NULL;
	al_contexts.pool  = NULL;
	al_contexts.inuse = NULL;
	al_contexts.items = 0;
	al_contexts.size  = 0;

	return;
}

/*
 * _alcGetNewContextId( void )
 *
 * Returns a new id for use as a context name, setting its use flag to
 * AL_TRUE, and returns the id.
 *
 * If there are no unused contexts, at least one more is created,
 * and it is modified and returned in the manner described above.
 *
 * assumes locked contexts
 */
ALint _alcGetNewContextId(void) {
	ALuint i;
	ALuint cid;
	ALuint cindex;

	for(i = 0; i < al_contexts.size; i++) {
		if(al_contexts.inuse[i] == AL_TRUE) {
			continue;
		}

		al_contexts.items++;
		al_contexts.inuse[i] = AL_TRUE;
		return al_contexts.map[i] = _alcGenerateNewCid();
	}

	_alcReallocContexts(al_contexts.size + 1);

	cindex = al_contexts.size - 1;
	cid = _alcGenerateNewCid();

	ASSERT(al_contexts.inuse[cindex] == AL_FALSE);

	al_contexts.inuse[cindex] = AL_TRUE;
	al_contexts.map[cindex]   = cid;

	if(_alcInitContext(cid) == NULL) {
		ASSERT(0);
		return -1;
	}

	al_contexts.items++;

	/*
	 *  We create contexts at the end, so the context id
	 *  will be the last valid element index (al_contexts.items - 1)
	 */
	return cid;
}

/*
 * _alcReallocContexts( ALuint newsize )
 *
 * _alcReallocContexts resizes the context pool to at least
 * newsize contexts, and creates mutex such that the new
 * contexts can be locked.
 *
 * assumes locked contexts
 */
static void _alcReallocContexts(ALuint newsize) {
	void *temp;
	ALuint i;

	if(al_contexts.size >= newsize) {
		return;
	}

	/* resize context pool */	
	temp = realloc(al_contexts.pool, sizeof *al_contexts.pool * newsize);
	if(temp == NULL) {
		perror("_alcReallocContexts malloc");
		exit(4);
	}
	al_contexts.pool = temp;

	/* resize inuse flags */	
	temp = realloc(al_contexts.inuse, sizeof *al_contexts.inuse * newsize);
	if(temp == NULL) {
		perror("_alcReallocContexts malloc");
		exit(4);
	}
	al_contexts.inuse = temp;

	/* resize context map */	
	temp = realloc(al_contexts.map, sizeof *al_contexts.map * newsize);
	if(temp == NULL) {
		perror("_alcReallocContexts malloc");
		exit(4);
	}
	al_contexts.map = temp;

	temp = realloc(context_mutexen, sizeof *context_mutexen * newsize);
	if(temp == NULL) {
		perror("_alcReallocContexts malloc");
		exit(4);
	}
	context_mutexen = temp;

	/* initialize new data */
	for(i = al_contexts.items; i < newsize; i++) {
		al_contexts.inuse[i] = AL_FALSE;
		al_contexts.map[i] = 0;
		context_mutexen[i] = mlCreateMutex();
	}

	if(al_contexts.items == 0) {
		/* If al_contexts.items is <= 0, then were are creating
		 * the contexts for the first time, and must create the
		 * "lock all contexts" mutex as well.
		 */

		all_context_mutex = mlCreateMutex();
		if(all_context_mutex == NULL) {
			perror("CreateMutex");
			exit(2);
		}
	}

	al_contexts.size = newsize;

	return;
}

/*
 * _alcGetTimeFilters( ALuint cid )
 *
 * Returns a pointer to the time_filter_set associated with the context named
 * by cid, or NULL if cid does not name a context.
 *
 * assumes locked context cid
 */
time_filter_set *_alcGetTimeFilters( ALuint cid ) {
	AL_context *cc;

	cc = _alcGetContext( cid );
	if(cc == NULL) {
		return NULL;
	}

	return cc->time_filters;
}

/*
 * _alcIndexToCid( int index )
 *
 * Converts index to a cid, returning that.
 */
static ALuint _alcCidToIndex( ALuint cid ) {
	ALuint i;

	for(i = 0; i < al_contexts.size; i++) {
		if( al_contexts.map[i] == cid ) {
			return i;
		}
	}

	return -1;
}

/*
 * _alcIndexToCid( int index )
 *
 * Converts index to a cid, returning that.
 */
static ALuint _alcIndexToCid(int index) {
	ASSERT(index < (int) al_contexts.size);

	return al_contexts.map[index];
}

/*
 * _alcGenerateNewCid( void )
 *
 * Returns a new unique ALuint suitable for use as a cid.
 */
static ALuint _alcGenerateNewCid(void) {
	static ALuint base = CONTEXT_BASE;

	return base++;
}

/*
 * alcGetCurrentContext( void )
 *
 * Returns context handle suitable associated with current context,
 * suitable for use with every function that takes a context handle,
 * or NULL if there is no current context.
 */
void *alcGetCurrentContext( void ) {
	if(al_contexts.items == 0) {
		return NULL;
	}

	if( _alcCCId == (ALuint) -1 ) {
		/* We are paused */
		return NULL;
	}

	return ALUINT_TO_VOIDP( _alcCCId );
}

/*
 * _alcDeviceReadSet( ALuint cid )
 *
 * Apply parameters for the read device associated with the context named by
 * cid.
 *
 * assumes locked context
 * FIXME: handle read?
 */
static void _alcDeviceReadSet( ALuint cid ) {
	AL_context *cc;
	ALboolean err;

	cc = _alcGetContext( cid );
	if( cc == NULL ) {
		_alcSetError( ALC_INVALID_CONTEXT );
		return;
	}

	if ( cc->read_device != NULL ) {
		err = _alcDeviceSet( cc->read_device );
	
		if( err != AL_TRUE) {
			_alDebug(ALD_CONTEXT, __FILE__, __LINE__, "set_audiodevice failed.");

			_alcSetError( ALC_INVALID_DEVICE );
		}
	}

	return;
}

/*
 * _alcDeviceWriteSet( ALuint cid )
 *
 * Apply parameters for the write device associated with the context named by
 * cid.
 *
 * assumes locked context
 * FIXME: handle read?
 */
static void _alcDeviceWriteSet( ALuint cid ) {
	AL_context *cc;
	ALboolean err;

	cc = _alcGetContext( cid );
	if( cc == NULL ) {
		_alcSetError( ALC_INVALID_CONTEXT );
		return;
	}

	if ( cc->write_device != NULL ) {
		err = _alcDeviceSet( cc->write_device );
	
		if( err != AL_TRUE) {
			_alDebug(ALD_CONTEXT, __FILE__, __LINE__, "set_audiodevice failed.");

			_alcSetError( ALC_INVALID_DEVICE );
		}
	}

	_alSetMixer( cc->should_sync ); /* set mixing stats */

	return;
}

/*
 * _alcGetReadBufsiz( ALuint cid )
 *
 * Returns the preferred read buffer size of the context named by cid,
 * in bytes.
 */
ALuint _alcGetReadBufsiz( ALuint cid ) {
	AL_context *cc = _alcGetContext( cid );

	if(cc == NULL) {
		return 0;
	}

	if( cc->read_device == NULL) {
		return 0;
	}

	return cc->read_device->bufsiz;
}

/*
 * _alcGetWriteBufsiz( ALuint cid )
 *
 * Returns the preferred write buffer size of the context named by cid,
 * in bytes.
 *
 * assumes locked context
 */
ALuint _alcGetWriteBufsiz( ALuint cid ) {
	AL_context *cc = _alcGetContext( cid );

	if(cc == NULL) {
		return 0;
	}

	if( cc->write_device == NULL) {
		return 0;
	}

	return cc->write_device->bufsiz;
}

/*
 * _alcGetReadFormat( ALuint cid )
 *
 * Returns the preferred read openal format of the context named by cid.
 *
 * assumes locked context
 */
ALenum _alcGetReadFormat( ALuint cid ) {
	AL_context *cc;

	cc = _alcGetContext( cid );
	if(cc == NULL) {
		return 0;
	}

	if( cc->read_device == NULL) {
		return 0;
	}

	return cc->read_device->format;
}

/*
 * _alcGetWriteFormat( ALuint cid )
 *
 * Returns the preferred write openal format of the context named by cid.
 */
ALenum _alcGetWriteFormat( ALuint cid ) {
	AL_context *cc;

	cc = _alcGetContext( cid );
	if(cc == NULL) {
		return 0;
	}

	if( cc->write_device == NULL) {
		return 0;
	}

	return cc->write_device->format;
}

/*
 * _alcGetReadSpeed( ALuint cid )
 *
 * Returns the preferred sampling rate of the read device associated with the
 * context named by cid.
 *
 * assumed locked context
 */
ALuint _alcGetReadSpeed(ALuint cid) {
	AL_context *cc;

	cc = _alcGetContext( cid );
	if(cc == NULL) {
		return 0;
	}

	if( cc->read_device == NULL) {
		return 0;
	}

	return cc->read_device->speed;
}

/*
 * _alcGetWriteSpeed( ALuint cid )
 *
 * Returns the preferred sampling rate of the write device associated with the
 * context named by cid.
 *
 * assumes locked context
 */
ALuint _alcGetWriteSpeed( ALuint cid ) {
	AL_context *cc;

	cc = _alcGetContext( cid );
	if(cc == NULL) {
		return 0;
	}

	if( cc->write_device == NULL) {
		return 0;
	}

	return cc->write_device->speed;
}

/*
 * _alcDeviceRead( ALuint cid, ALvoid *dataptr, ALuint bytes_to_read )
 *
 * Reads bytes_to_read worth of data from the read device
 * associated with the context named cid.
 *
 * assumes locked context
 */
ALsizei _alcDeviceRead( ALuint cid, ALvoid *dataptr, ALuint bytes_to_read ) {
	AL_context *cc;

	cc = _alcGetContext( cid );
	if( cc == NULL ) {
		return 0;
	}

	if( cc->read_device == NULL ) {
		return 0;
	}

	return capture_audiodevice(cc->read_device->handle, dataptr, bytes_to_read);	
}

/*
 * _alcDeviceWrite( ALuint cid, ALvoid *dataptr, ALuint bytes_to_write )
 *
 * Writes bytes_to_write worth of data from dataptr to the write device
 * associated with the context named cid.
 *
 * assumes locked context
 */
void _alcDeviceWrite( ALuint cid, ALvoid *dataptr, ALuint bytes_to_write ) {
	AL_context *cc;

	cc = _alcGetContext( cid );
	if( cc == NULL ) {
		return;
	}

	if( cc->write_device == NULL ) {
		return;
	}

	_alBlitBuffer( cc->write_device->handle, dataptr, bytes_to_write );

	return;
}

/*
 * _alcIsContext( ALuint cid )
 *
 * Returns AL_TRUE if cid names a valid context, AL_FALSE otherwise.
 *
 * assumes locked context
 */
ALboolean _alcIsContext( ALuint cid ) {
	AL_context *cc;

	cc = _alcGetContext( cid );
	if( cc == NULL ) {
		return AL_FALSE;
	}

	return AL_TRUE;
}

/*
 * _alcIsContextSuspended( ALuint cid )
 *
 * Returns AL_TRUE if this context is suspended, AL_FALSE otherwise.
 * Suspended contexts do not have their sources updated, or mixed.
 *
 * assumes locked context
 */
ALboolean _alcIsContextSuspended( ALuint cid ) {
	AL_context *cc;

	cc = _alcGetContext( cid );
	if( cc == NULL ) {
		return AL_TRUE;
	}

	return cc->issuspended;
}

/*
 * alcIsExtensionPresent( UNUSED(ALCdevice *device), ALubyte *extName )
 *
 * Returns AL_TRUE if the alc extension extName is present, AL_FALSE
 * otherwise.
 */
ALboolean alcIsExtensionPresent( UNUSED(ALCdevice *device), ALubyte *extName ) {
	return alIsExtensionPresent( extName );
}

/*
 * alcGetProcAddress( UNUSED(ALCdevice *device), ALubyte *funcName ).
 *
 * Returns the alc extension function named funcName, or NULL if it doesn't
 * exist.
 */
ALvoid *alcGetProcAddress( UNUSED(ALCdevice *device), ALubyte *funcName ) {
	return alGetProcAddress( funcName );
}

/*
 * alcGetEnumValue( ALCdevice *device, ALubyte *enumName )
 *
 * Returns enum value for enumName.
 */
ALenum alcGetEnumValue( UNUSED(ALCdevice *device), ALubyte *enumName ) {
	return alGetEnumValue( enumName );
}
