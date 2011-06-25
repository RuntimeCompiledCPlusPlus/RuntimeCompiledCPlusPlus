/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_mixer.c
 *
 * Place where most stuff happens.  The main mixer iteration function
 * is defined here.
 *
 */
#include "al_siteconfig.h"

#include <AL/al.h>

#include "al_debug.h"
#include "al_error.h"
#include "al_types.h"
#include "al_main.h"
#include "al_buffer.h"
#include "al_filter.h"
#include "al_mixer.h"
#include "al_mixmanager.h"
#include "al_source.h"
#include "al_mspool.h"
#include "mixaudio16.h"

#include "alc/alc_context.h"
#include "alc/alc_device.h"
#include "alc/alc_speaker.h"

#include "audioconvert.h"

#include "threads/threadlib.h"
#include "mutex/mutexlib.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "arch/interface/interface_sound.h"

/*
 * The mixing function checks this variable for equality with AL_TRUE.  When
 * this is the case, it exits.  The default is AL_FALSE.
 */
ALboolean volatile time_for_mixer_to_die = AL_FALSE;

/* our thread ID, if async */
ThreadID mixthread = NULL; 

/*
 * MixManager is the ALMixManager we use, in conjunction with MixFunc, to
 * dispatch mixing functions for combining streams of PCM data.
 */
static ALMixManager MixManager;

/*
 * MixFunc is the ALMixFunc we use, in conjunction with MixManager, to
 * dispatch mixing functions for combining streams of PCM data.
 */
static ALMixFunc MixFunc;

/*
 * mspool is our pool of mspool nodes.
 */
static _alMixPool mspool;

/*
 * s16le is the data structure we use to convert data from the canonical
 * format into the external format.
 */
static acAudioCVT s16le;

/*
 * mix_mutex is the guard around mspool.
 */
static MutexID mix_mutex   = NULL;

/*
 * pause_mutex is an aid to mixer pausing.
 */
static MutexID pause_mutex = NULL;

/*
 * Size of how much data we should mix at a time for each source.
 */
static ALuint bufsiz = 0;

static struct {
	void *data;
	ALuint length;
} mixbuf = { NULL, 0 };

/* streaming buffer array */
static struct {
	ALuint *streaming_buffers;
	ALuint size;
	ALuint items;
} sbufs = { NULL, 0, 0 };

/*
 * sync_mixer_iterate( void *dummy )
 *
 * Non-threaded top level mixing logic.
 */
int sync_mixer_iterate( void *dummy );

/*
 * async_mixer_iterate( void *dummy )
 *
 * Threaded top level mixing logic.
 */
int async_mixer_iterate( void *dummy );

/*
 * Pointer to either sync or async_mixer_iterate.
 */
int (*mixer_iterate)( void *dummy ) = NULL;

/*
 * _alAddDataToMixer( void *dataptr, ALuint bytes_to_write )
 *
 * Mixes bytes_to_write data from dataptr to the system buffer.  If
 * bytes_to_write exceeds the system buffer size, it is truncated and returns
 * the maximum it could write.
 */
static ALuint _alAddDataToMixer( void *dataptr, ALuint bytes_to_write );

/*
 * _alProcessFlags( void )
 *
 * Processes flags set by the top-level mixing function.  These flags control
 * things like deletion.
 */
static void _alProcessFlags( void );

/*
 * _alDestroyMixSource( void *ms )
 *
 * Finalize a _alMixSource object.
 */
static void _alDestroyMixSource( void *ms );

/*
 * _alAddBufferToStreamingList( ALuint bid )
 *
 * AL_buffers generated using alGenStreamingBuffers_LOKI need to be handled in
 * a special way.  This function does that special handling.
 */
static void _alAddBufferToStreamingList( ALuint bid );

/*
 * _alTryLockMixerPause( void )
 *
 * Try to lock the mix_pause mutex.  Return AL_TRUE if lock suceeded, AL_FALSE
 * otherwise.
 */
static ALboolean _alTryLockMixerPause( void );

/*
 * _alMixSources
 *
 * This is the where most of the action is directed.
 *
 * assumes locked mixbuf
 */
static void _alMixSources( void ) {
	AL_buffer *samp;
	AL_source *src;
	int *sampid;
	int written;
	ALuint bytes_to_write;
	_alMixSource *itr = NULL;
	ALboolean islooping   = AL_FALSE;
	ALboolean isstreaming = AL_FALSE;
	ALboolean iscallback  = AL_FALSE;
	ALuint pitr; /* pool iterator */
	ALuint nc = 2;
	
	for(pitr = 0; pitr < mspool.size; pitr++) {
		if(mspool.pool[pitr].inuse == AL_FALSE) {
			/* if not in use, can't be played anyway */
			continue;
		}

		itr = _alMixPoolIndex( &mspool, pitr );

		if(!(itr->flags & ALM_PLAY_ME)) {
			/*
			 * current mix source on the way out, so
			 * ignore it
			 */
			_alDebug(ALD_MIXER, __FILE__, __LINE__,
				"_alMixSources: %d is on the out",
				itr->sid);
			continue;
		}

		_alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
			"_alMixSources: currently on source id %d",
			itr->sid);

		/* check for paused context. */
		if( _alcIsContextSuspended( itr->context_id ) == AL_TRUE ) {
			continue;
		}

		_alLockSource( itr->context_id, itr->sid );

		src = _alGetSource(itr->context_id, itr->sid);
		if(src == NULL) {
			/* not a valid src */
			itr->flags = ALM_DESTROY_ME;

			_alUnlockSource( itr->context_id, itr->sid );

			continue;
		}

		if(src->state == AL_PAUSED) {
			/* Paused sources don't get mixed */
			_alUnlockSource( itr->context_id, itr->sid );
			continue;
		}

		sampid = _alGetSourceParam(src, AL_BUFFER);
		if(sampid == NULL) {
			/* source added with no buffer associated */
			itr->flags = ALM_DESTROY_ME;

			_alDebug(ALD_MIXER, __FILE__, __LINE__,
				"No bid associated with sid %d", itr->sid);

			_alUnlockSource( itr->context_id, itr->sid );
			continue;
		}

 		samp = _alGetBuffer(*sampid);
		if(samp == NULL) {
			/* source added with no valid buffer associated */
			_alDebug(ALD_MIXER, __FILE__, __LINE__,
				"no such bid [sid|bid] [%d|%d]",
				itr->sid, *sampid);

			itr->flags = ALM_DESTROY_ME;

			_alUnlockSource( itr->context_id, itr->sid );

			continue;
		}

		/* get special needs */
		islooping   = _alSourceIsLooping( src );
		isstreaming = _alBidIsStreaming( *sampid );
		iscallback  = _alBidIsCallback( *sampid );

		/* apply each filter to sourceid sid */
		_alApplyFilters( itr->context_id, itr->sid );

		/*
		 * calculate how many bytes left.  For looping sounds,
		 * we can ignore qualifications of samp->size because
		 * we loop (ie, have infinite length).
		 */
		if( islooping == AL_FALSE ) {
			/* Non looping source */
			bytes_to_write = _alSourceBytesLeft(src, samp);
		} else {
			bytes_to_write = bufsiz;
		}

		/*
		 * set written to either bufsiz or the number of bytes
		 * left in the source.
		 */
		if(bytes_to_write > bufsiz) {
			written = bufsiz;
		} else {
			written = bytes_to_write;
		}

		_alAddDataToMixer( src->srcParams.outbuf, written );

		if(_alSourceShouldIncrement(src) == AL_TRUE) {
			/*
			 * soundpos is an offset into the original buffer
			 * data, which is most likely mono.  We use nc (the
			 * number of channels in mixer's format) to scale
			 * soundpos
			 */
			_alSourceIncrement(src, written / nc);
		}

		if(_alSourceBytesLeft(src, samp) <= 0) {
			/*
			 * end of sound.  streaming & looping are special
			 * cases.
			 */
			if(islooping == AL_TRUE ) {
				if(iscallback == AL_TRUE) {
					_alDebug(ALD_LOOP, __FILE__, __LINE__,
					"%d callback loop reset ", itr->sid);

					src->srcParams.soundpos = 0;

					/*
					 * we've actually been fudging the
					 * size
					 */
					samp->size /= nc;
				} else {
					_alDebug(ALD_LOOP, __FILE__, __LINE__,
					"%d loop reset", itr->sid);

					/*
					 * Looping buffers are prefed via
					 * SplitSources, so soundpos is
					 * actually bigger than the buffer
					 * size (because they wrap around).
					 */
					src->srcParams.soundpos %= samp->size;
				}
			} else if(isstreaming == AL_FALSE) {
				/*
				 * The source's current buffer is fini.  Do we 
				 * go to the next buffer in the queue?
				 */
				if(src->bid_queue.read_index < src->bid_queue.size - 1) {
					ALuint rindex = src->bid_queue.read_index;
					ALuint bid    = src->bid_queue.queue[rindex];

					/* There *is* another buffer.  We
					 * count on SplitSources to wrap
					 * around buffer queue entries so that
					 * we get to artifact at crossings.
					 */

					/*
					 * Change current state to queue for
					 * this bid/sid pair
					 */
					_alBidRemoveCurrentRef(bid, itr->sid);
					_alBidAddQueueRef(bid, itr->sid);

					src->bid_queue.read_index++;

					src->srcParams.soundpos = 0;
					/* src->srcParams.soundpos -= samp->size; */


				} else {
					/* This buffer is solo */
					itr->flags = ALM_DESTROY_ME;
				}
			}
		}

		_alUnlockSource( itr->context_id, itr->sid );
	}

	return;
}

/*
 * _alAddDataToMixer( void *dataptr, ALuint bytes_to_write )
 *
 *
 * Mixes bytes_to_write data from dataptr to the system buffer.  If
 * bytes_to_write exceeds the system buffer size, it is truncated and returns
 * the maximum it could write.
 *
 * assumes that mixer and default context are locked
 */
static ALuint _alAddDataToMixer( void *dataptr, ALuint bytes_to_write ) {
	if(dataptr == NULL) {
		/* Most likely, thread is waiting to die */
		return 0;
	}

	if(bytes_to_write > bufsiz) {
		bytes_to_write = bufsiz;
	}

	/* add entry to mix manager */
	_alMixManagerAdd( &MixManager, dataptr, bytes_to_write );

	return bytes_to_write;
}

/*
 * void _alDestroyMixer( void )
 *
 * Deallocate data allocated in _alInitMixer.
 */
void _alDestroyMixer( void ) {
	if( mix_mutex != NULL ) {
		mlDestroyMutex( mix_mutex );
	}

	if( pause_mutex != NULL ) {
		/* we may we destroyed while paused, which is bad, but
		 * not horrible.  Try to lock it.  If sucessful, then
		 * we weren't locked before, so we unlock and Destroy.  
		 * Otherwise, just unlock and destroy, since we aren't
		 * going to be need async_mixer_iterate services anytime
		 * soon anyway.
		 */
		_alTryLockMixerPause();
		_alUnlockMixerPause(); /* at this point, we are either locked by
					  the context or by our own doing, so it's
					  okay to unlock.
					*/

		mlDestroyMutex( pause_mutex );
	}

	_alMixPoolFree( &mspool, _alDestroyMixSource );
	mspool.size = 0;

	mixthread   = NULL;
	pause_mutex = NULL;
	mix_mutex   = NULL;

	_alMixFuncDestroy(&MixFunc);
	_alMixManagerDestroy(&MixManager);

	free( mixbuf.data );

	mixbuf.data   = NULL;
	mixbuf.length = 0;

	return;
}

/*
 * _alDestroyMixSource( void *ms )
 *
 * Finalize a _alMixSource object.
 */
static void _alDestroyMixSource( void *ms ) {
	_alMixSource *msrc = (_alMixSource *) ms;
	AL_source *src;
	ALuint *bid;
	ALuint i;
	ALuint sid = msrc->sid;
	ALuint cid = msrc->context_id;

	_alLockSource( cid, sid );

	src = _alGetSource( cid, sid );
	if(src == NULL) {
		/*
		 * source got nuked somewhere, or context was
		 * destroyed while paused.
		 */

		_alDebug(ALD_MIXER, __FILE__, __LINE__,
			"_alDestroyMixSource: source id %d is not valid",
			msrc->sid);

		_alUnlockSource( cid, sid );

		return;
	}

	/*
	 * state should always be set to stopped by this point.
	 */
	src->state = AL_STOPPED;

	/*
	 * reset read index
	 */
	src->bid_queue.read_index = 0;

	/* clear sid */
	msrc->sid = 0;

	/*
	 * Update buffer state
	 */
	bid = _alGetSourceParam(src, AL_BUFFER);
	if(bid == NULL) {
		/* This shouldn't happend:  The buffer param of this
		 * source is now invalid, but we're stopping it.  This
		 * really is an ugly error: it most likely means that
		 * there's a bug in the refcounting stuff somewhere.
		 */
		_alDebug(ALD_MIXER, __FILE__, __LINE__,
		      "_alDestroyMixSource: no bid for source id %d",
		      src->sid);

		_alUnlockSource( cid, sid );

		_alDCSetError( AL_ILLEGAL_COMMAND );

		return;
	}

	_alBidRemoveCurrentRef(*bid, src->sid);

	if(src->bid_queue.size != 1) {
		/* This is the last entry in the queue (or the source
		 * was stopped) so we want to change the current state
		 * for this bid/sid to queue
		 */
		_alBidAddQueueRef(*bid, src->sid);
	}

	/*
	 * if we have a callback buffer, call the
	 * destructor on the source (because the source
	 * is over.
	 */
	if(_alBidIsCallback(*bid) == AL_TRUE) {
		_alBidCallDestroyCallbackSource(src->sid);
	}

	/* streaming sources */
	if(_alBidIsStreaming(*bid) == AL_TRUE) {

		for(i = 0; i < sbufs.size; i++) {
			if(sbufs.streaming_buffers[i] == *bid) {
				sbufs.streaming_buffers[i] = 0;
				sbufs.items--;
			}
		}
	}

	_alUnlockSource( cid, sid );

	return;
}

/*
 * _alInitMixer( void )
 *
 * Create and initialize data structures needed by the mixing function.
 */
ALboolean _alInitMixer( void ) {
	bufsiz = _alcDCGetWriteBufsiz();

	mix_mutex = mlCreateMutex();
	if(mix_mutex == NULL) {
		return AL_FALSE;
	}

	pause_mutex = mlCreateMutex();
	if(pause_mutex == NULL) {
		mlDestroyMutex( mix_mutex );
		mix_mutex = NULL;

		return AL_FALSE;
	}

	/* init Mixer funcs */
	if( _alMixFuncInit(&MixFunc, MAXMIXSOURCES ) == AL_FALSE) {
		mlDestroyMutex( mix_mutex );
		mlDestroyMutex( pause_mutex );
		mix_mutex = NULL;
		pause_mutex = NULL;

		return AL_FALSE;
	}

	/* init MixManager */
	if(_alMixManagerInit(&MixManager, MAXMIXSOURCES) == AL_FALSE) {
		mlDestroyMutex(mix_mutex);
		mlDestroyMutex(pause_mutex);
		mix_mutex = NULL;
		pause_mutex = NULL;

		_alMixFuncDestroy( &MixFunc );

		return AL_FALSE;
	}

	mspool.size = 0;

	return AL_TRUE;
}

/*
 * _alSetMixer( ALboolean synchronous )
 *
 * Sets mixer to match the current context.  If synchronous is AL_FALSE, a
 * seperate thread is launched.
 *
 * assumes locked context
 */
void _alSetMixer( ALboolean synchronous ) {
	AL_context *dc;
	ALuint ex_format;
	ALuint ex_speed;

	dc =  _alcDCGetContext();
	if(dc == NULL) {
		_alDebug(ALD_MIXER, __FILE__, __LINE__,
			"_alSetMixer with no default context?  weird");
		return;
	}

	if ( dc->write_device ) {
		ex_format   = _alcDCGetWriteFormat();
		ex_speed    = _alcDCGetWriteSpeed();
		bufsiz      = _alcDCGetWriteBufsiz();
	} else {
		ex_format   = _alcDCGetReadFormat();
		ex_speed    = _alcDCGetReadSpeed();
		bufsiz      = _alcDCGetReadBufsiz();
	}

	_alDebug(ALD_CONVERT, __FILE__, __LINE__,
		"_alSetMixer f|c|s [0x%x|%d|%d] -> [0x%x|%d|%d]",
		/* from */
		canon_format,
		_al_ALCHANNELS( ex_format ), /* ignore channel settings.  We handle this */
		canon_speed,
		/* to */
		ex_format,
		_al_ALCHANNELS( ex_format ),
		ex_speed);

	if(acBuildAudioCVT(&s16le,
		/* from */
		_al_AL2ACFMT( canon_format ),
		_al_ALCHANNELS( ex_format ), /* ignore channel settings.  We handle this */
		canon_speed,

		/* to */
		_al_AL2ACFMT(ex_format),
		_al_ALCHANNELS(ex_format),/* ignore channel settings.  We handle this */
		ex_speed) < 0) {
		_alDebug(ALD_CONVERT, __FILE__, __LINE__,
			"Couldn't build audio convertion data structure.");
	}

	if( s16le.len_ratio > 1.0 ) {
		/*
		 * we always alloc the larger value, because
		 * we need the extra space
		 */
		mixbuf.length = bufsiz * s16le.len_ratio;
	} else {
		mixbuf.length = bufsiz;
	}

	mixbuf.data   = realloc( mixbuf.data, mixbuf.length );
	s16le.buf     = mixbuf.data;
	s16le.len     = bufsiz;

	if(synchronous == AL_TRUE) {
		mixer_iterate = sync_mixer_iterate;
	} else {
		mixer_iterate = async_mixer_iterate;

		if(mixthread == NULL) {
			mixthread = tlCreateThread(mixer_iterate, NULL);
		}
	}

	return;
}

/*
 * _alAllocMixSource( ALuint sid )
 *
 * Reserve an _alMixSource, associate it with the source named sid.  Returns
 * AL_TRUE if reservation was true, AL_FALSE if sid is invalid or allocation
 * was not possible.
 *
 * If sid is not a valid source, AL_INVALID_NAME is set.
 *
 * Assumes locked context
 */
static ALboolean _alAllocMixSource( ALuint sid ) {
	AL_source *src;
	ALuint *bid;
	_alMixSource *msrc;
	ALuint context_id = _alcCCId; /* current context id */
	int msindex;

	src = _alGetSource( context_id, sid );
	if( src == NULL ) {
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		      "_alAllocMixSource: source id %d is not valid", sid);

		_alSetError( context_id, AL_INVALID_NAME );

		return AL_FALSE;
	}

	/*
	 *  Make sure that the source isn't already playing.
	 */
	if( src->state == AL_PLAYING ) {
		/*
		 * The source in question is already playing.
		 *
		 * Legal NOP
		 */
		_alDebug(ALD_MIXER, __FILE__, __LINE__,
			"_alAllocMixSource: source id %d already playing", sid);

		return AL_FALSE;
	}

	/*
	 *  Add reference for buffer
	 */
	_alLockBuffer();

	bid = _alGetSourceParam( src, AL_BUFFER );
	if(bid == NULL) {
		_alUnlockBuffer();

		/*
		 * The source in question does not have the BUFFER
		 * attribute set.
		 */
		_alDebug(ALD_MIXER, __FILE__, __LINE__,
			"_alAllocMixSource: source id %d has BUFFER unset",sid);

		_alSetError(context_id, AL_ILLEGAL_COMMAND);

		return AL_FALSE;
	}

	if( _alIsBuffer(*bid) == AL_FALSE ) {
		/*
		 * The source in question has a buffer id, but it is not
		 * valid.
		 */
		_alUnlockBuffer();

		_alDebug(ALD_MIXER, __FILE__, __LINE__,
			"_alAllocMixSource: source %d has invalid BUFFER %d:%d",
			sid, src->bid_queue.read_index, bid);

		_alSetError(context_id, AL_INVALID_NAME);

		return AL_FALSE;
	}

	_alUnlockBuffer();

	/* streaming buffers added to increment list */
	if( _alBidIsStreaming(*bid) == AL_TRUE ) {
		_alAddBufferToStreamingList(*bid);
	}

	if( src->bid_queue.read_index < src->bid_queue.size - 1 ) {
		_alBidRemoveQueueRef(*bid, sid);
	}

	_alBidAddCurrentRef(*bid,  sid);

	/*
	 *  Allocate space.
	 */
	msindex = _alMixPoolAlloc( &mspool );
	if(msindex == -1) {
		return AL_FALSE;
	}

	/*
	 *  Initialization mojo.
	 *
	 *  set sid to source's id, flags to ALM_PLAY_ME, and
	 *  set the source's flags so that we know it's playing,
	 *  and reset soundpos.
	 */
	msrc = _alMixPoolIndex( &mspool, msindex );

	/* set mixsource information */
	msrc->context_id    = context_id;
	msrc->sid           = sid;
	msrc->flags         = ALM_PLAY_ME;

	/* set source information */
	src->state		      = AL_PLAYING;
	src->srcParams.soundpos       = 0;
	src->bid_queue.read_index     = 0;

	return AL_TRUE;
}

/*
 * _alRemoveSourceFromMixer( ALuint sid )
 *
 * Removes the source named sid from the mixer queue.
 */
ALboolean _alRemoveSourceFromMixer( ALuint sid ) {
	AL_source *src;
	ALuint i;

	src = _alDCGetSource( sid );
	if(src == NULL) {
		_alDebug( ALD_MIXER, __FILE__, __LINE__,
			  "_alRemoveSourceFromMixer: %d is an invalid source id",
			  sid );

		_alDCSetError( AL_INVALID_NAME );

		return AL_FALSE;
	}

	/*
	 *	We are stopping now.	Which means we set the state:
	 *
	 *	active	-> stopped
	 *	paused	-> stopped
	 *	initial -> NOP
	 *	stopped -> NOP
	 *
	 */
	switch(src->state) {
		case AL_INITIAL:
		case AL_STOPPED:
			/* Stop on a non active source is a legal NOP */
			_alDebug( ALD_MIXER, __FILE__, __LINE__,
				  "_alRemoveSourceFromMixer(%d): source is not playing",
				  sid );

			return AL_FALSE;
			break;
		default:
			/* We're okay, otherwise */
			break;
	}

	for(i = 0; i < mspool.size; i++) {
		if((mspool.pool[i].data.sid == sid) &&
			 (mspool.pool[i].inuse == AL_TRUE)) {
			_alMixPoolDealloc( &mspool, i, _alDestroyMixSource );

			_alDebug( ALD_MIXER, __FILE__, __LINE__,
					"_alRemoveSourceFromMixer: removed sid %d",
					sid );

			return AL_TRUE;
		}
	}

	/*
	 * We really shouldn't end up here.  It means that the ->flags
	 * attribute got weird somewhere.
	 */
	_alDebug(ALD_MIXER, __FILE__, __LINE__,
		"_alRemoveSourceFromMixer(%d): Could not remove source",
		sid);

	return AL_FALSE;
}

/*
 * _alAddSourceToMixer( ALuint sid )
 *
 * Adds the source named sid to the mixing queue.  Sets AL_INVALID_NAME if sid
 * is invalid.
 *
 * assumes that context is locked
 * assumes that mixbuf is locked
 */
void _alAddSourceToMixer( ALuint sid ) {
	AL_source *src;

	src = _alDCGetSource( sid );
	if(src == NULL) {
		/* invalid name */
		_alDebug( ALD_MIXER, __FILE__, __LINE__,
			  "_alAddSourceToMixer: source id %d is not valid",
			  sid );

		_alDCSetError( AL_INVALID_NAME );
		return;
	}

	/*
	 * Now, we are going to set the state:
	 *
	 * initial -> active
	 * paused  -> active
	 * stopped -> active
	 * active  -> nop
	 *
	 * Paused sources are already in the mixer, but being
	 * ignored, so we just "turn them on." initial and stopped
	 * sources need to have new mixsources alloced for them.
	 */
	switch(src->state) {
		case AL_PAUSED:
			/* Paused sources, when played again, resume
			 * at their old location.  We don't need to
			 * alloc a new mixsource.
			 */
			src->state = AL_PLAYING;
			return;
		case AL_PLAYING:
			/* This source is already playing.
			 *
			 * Legal NOP
			 */
			_alDebug( ALD_MIXER, __FILE__, __LINE__,
				  "_alAddSourceToMixer: source %d already active",
				  sid );
			
			return;
		default:
			/* alloc a mix source */
			break;
	}

	if(_alAllocMixSource( sid ) == AL_FALSE) {
		/* most likely, the buffer associated with the
		 * source in question has been deleted.  Return
		 * asap.
		 *
		 * We shouldn't set the error because _alAllocMixSource
		 * will actually be better aware of what the problem is,
		 * and so will set the error accordingly.
		 */
		_alDebug( ALD_MIXER, __FILE__, __LINE__,
			   "_alAddSourceToMixer: Could not add source sid %d",
			  sid );

		return;
	}

	_alDebug(ALD_MIXER, __FILE__, __LINE__,
		"_alAddSourceToMixer: added sid %d", sid);

	return;
}

/*
 * FL_alLockMixBuf(UNUSED(const char *fn), UNUSED(int ln))
 *
 * Locks the mutex guarding mixer data structures.
 */
void FL_alLockMixBuf( UNUSED(const char *fn), UNUSED(int ln) ) {
	_alLockPrintf("_alLockMixbuf", fn, ln);

	mlLockMutex( mix_mutex );

	return;
}

/*
 * FL_alUnlockMixBuf(UNUSED(const char *fn), UNUSED(int ln))
 *
 * Unlocks the mutex guarding mixer data structures.
 */
void FL_alUnlockMixBuf(UNUSED(const char *fn), UNUSED(int ln)) {
	_alLockPrintf("_alUnlockMixbuf", fn, ln);

	mlUnlockMutex( mix_mutex );

	return;
}

/*
 * sync_mixer_iterate(UNUSED(void *dummy))
 *
 * non-threaded version of top level mixing function.  Called many times.
 */
int sync_mixer_iterate(UNUSED(void *dummy)) {
	ALshort *dataptr = mixbuf.data;
	int bytes_to_write = 0;

	/* clear buffer */
	memset( dataptr, 0, bufsiz );

	_alLockMixBuf();

	_alMixSources();

	_alProcessFlags();

	_alUnlockMixBuf();

	/* we've accumulated sources, now mix */
	_alMixManagerMix(&MixManager, &MixFunc, mixbuf.data);

	if(acConvertAudio(&s16le) < 0) {
		_alDebug(ALD_CONVERT, __FILE__, __LINE__,
			"Couldn't execute conversion from canon.");
		return -1;
	}

	bytes_to_write = s16le.len_cvt;

	_alcDCDeviceWrite( dataptr, bytes_to_write );
	
	return 0;
}

/*
 * async_mixer_iterate(UNUSED(void *dummy))
 *
 * threaded version of top level mixing function.  Doesn't end until the last
 * context is deleted.
 */
int async_mixer_iterate(UNUSED(void *dummy)) {
	ALuint bytes_to_write = 0;

	/* clear buffer */
	memset(mixbuf.data, 0, mixbuf.length);

	do {
		if(_alTryLockMixerPause() == AL_TRUE) {
			_alLockMixBuf();

			_alMixSources();

			_alProcessFlags();

			_alUnlockMixBuf();

			/* we've accumulated sources, now mix */
			_alMixManagerMix( &MixManager, &MixFunc, mixbuf.data );

			if(acConvertAudio(&s16le) < 0) {
				_alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
				"Couldn't execute conversion from canon.");
				/*
				 * most likely we're just early.
				 * Don't sweat it.
				 */
				continue;
			}

			bytes_to_write = s16le.len_cvt;

			_alcDCDeviceWrite( mixbuf.data, bytes_to_write );

			/* clear buffer */
			memset(mixbuf.data, 0, mixbuf.length);

			_alUnlockMixerPause();
		}
	} while(time_for_mixer_to_die == AL_FALSE);

	time_for_mixer_to_die = AL_FALSE;

	tlExitThread(0);
	
	return 0;
}

/*
 * _alAddBufferToStreamingList( ALuint bid )
 *
 * Adds a buffer name to the streaming list, for special handling.
 *
 *  FIXME: need to free sbufs.stremaing_buffers on exit.
 */
static void _alAddBufferToStreamingList( ALuint bid ) {
	void *temp;
	int offset;
	ALuint newsize;
	ALuint i;

	if(sbufs.items >= sbufs.size) {
		/* realloc */
		newsize = sbufs.size + 1;

		temp = realloc(sbufs.streaming_buffers,
				newsize * sizeof *sbufs.streaming_buffers);
		if(temp == NULL) {
			/* what a bummer */
			return;
		}
		sbufs.streaming_buffers = temp;

		for(i = sbufs.size; i < newsize; i++) {
			sbufs.streaming_buffers[i] = 0;
		}

		sbufs.size = newsize;
	}

	/* add bid to streaming list */
	for(i = 0, offset = sbufs.items; i < sbufs.size; i++) {
		offset = (offset + 1) % sbufs.size;

		if(sbufs.streaming_buffers[offset] == 0) {
			sbufs.streaming_buffers[offset] = bid;

			sbufs.items++;

			return;
		}
	}
	
	return;
}

/*
 * _alTryLockMixerPause( void )
 *
 * The function that alcMakeContextCurrent calls to pause
 * asynchronous mixers.  Return AL_TRUE if the pause mutex was locked,
 * AL_FALSE otherwise.
 */
static ALboolean _alTryLockMixerPause( void ) {
	if(mlTryLockMutex( pause_mutex ) == 0) {
		return AL_TRUE;
	}
	
	return AL_FALSE;
}

/*
 * _alLockMixerPause( void )
 *
 * The function that alcMakeContextCurrent calls to pause
 * asynchronous mixers
 */
void _alLockMixerPause( void ) {
	mlLockMutex( pause_mutex );

	return;
}

/*
 * _alUnlockMixerPause( void )
 *
 * The function that alcMakeContextCurrent calls to resume
 * asynchronous mixers
 */
void _alUnlockMixerPause( void ) {
	mlUnlockMutex( pause_mutex );

	return;
}


/*
 * _alProcessFlags( void )
 *
 * The mixing function (_alMixSources), in the course of it's job marks the
 * mixsource nodes with commands that need to be executed after the completion
 * of the mixsource's iteration through the loop.  This is the function where
 * such things are done.
 *
 * Also, we process streaming buffers here because they need to be visited
 * once per call to _alMixSources, but the presence of multiple sources
 * refering to the same buffer precludes us from doing the processing in
 * _alMixSources.
 *
 * assumes locked mixbuf
 */
void _alProcessFlags( void ) {
	_alMixSource *itr = NULL;
	AL_buffer *bitr; /* buffer iterator */
	ALuint i;
	ALuint k;

	for(i = 0; i < mspool.size; i++) {
		if(mspool.pool[i].inuse == AL_FALSE) {
			/* skip mixsources not in use */
			continue;
		}

		itr = _alMixPoolIndex(&mspool, i);
		if(itr == NULL) {
			/* shouldn't happen */
			continue;
		}

		if(itr->flags & ALM_DESTROY_ME) {
			/* this source associated with this mixsource
			 * has expired (either because it has been stopped
			 * or just run out.  Remove it.
			 */
			if( alIsSource(itr->sid) == AL_FALSE ) {
				/* sanity check */
				continue;
			}

			/* deallocated mixsource */
			_alMixPoolDealloc( &mspool, i, _alDestroyMixSource );
		}
	}

	_alLockBuffer();

	/* process streaming buffers */
	i = sbufs.items;
	k = sbufs.size - 1;

	while(i--) {
		int nc;

		while(sbufs.streaming_buffers[k] == 0) {
			/*
			 * We don't worry about underflow because
			 * we are keying off of the number of sbuf
			 * items.
			 */
			k--;
		}

		bitr = _alGetBuffer(sbufs.streaming_buffers[k]);
		if(bitr == NULL) {
			_alDebug(ALD_STREAMING, __FILE__, __LINE__,
				"invalid buffer id %d",
				sbufs.streaming_buffers[k]);

			/* invalid buffer
			 *
			 * Most likely, the buffer got deleted at some
			 * point.  We remove here, decrement the items
			 * count and get on with life.
			 */
			sbufs.streaming_buffers[k] = 0;
			sbufs.items--;

			continue;
		}

		/* get the buffer's number of channels, so multichannel
		 * streaming sounds work properly.
		 */
		nc = _alcDCGetNumSpeakers();

		if(nc <= 0) {
			nc = 1;
		}

		if(_alGetBufferState(bitr) == AL_UNUSED) {
			/* refcount 0?  Don't bother. */
			sbufs.streaming_buffers[k] = 0;
			sbufs.items--;

			continue;
		}

		bitr->streampos += bufsiz/nc;

		if(bitr->streampos >= bitr->size) {
			if(bitr->flags & ALB_STREAMING_WRAP) {
				/* If we have the wrap flag, wrap.
				 * Otherwise, what?  End the source?
				 * Loop forever?
				 */

				_alDebug(ALD_STREAMING, __FILE__, __LINE__,
					"Wrapping\n");

				bitr->streampos = 0;
				bitr->flags &= ~ALB_STREAMING_WRAP;
			}
		}
	}

	_alUnlockBuffer();

	return;
}

