/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_filter.c
 *
 * Contains filters.
 *
 *
 * Short guide to filters:
 *
 * Filters all have the prefix alf_<something>.  Each filter
 * defined in software_time_filters or software_frequency_filters
 * is applied to each source that finds its way into the mixer.
 *
 * ApplyFilters takes a chunk of data from the original buffer 
 * associated with the passed source.
 *
 * This chunk is understood to be that block of data samp->_orig_buffer
 * offset src->soundpos to src->soundpos + bufsiz, where src is the
 * passed AL_source, samp is the AL_buffer associated with src, and
 * bufsiz is the length of the chunk of data that we want.  It is usually
 * set to _AL_DEF_BUFSIZE, unless specified by ALC_BUFFERSIZE in the
 * application.
 *
 * Applying filters to a source does not (should not) change the original
 * pcm data.  ApplyFilters will split the original pcm data prior to
 * calling each filter, and filters should restrict themselves to 
 * manipulating the passed data.
 *
 * time domain filters (those defines in software_time_filters) are
 * passed:
 *	ALuint cid
 *		identifier for the context that this source belongs to
 *	AL_source *src
 *		source that the filter should be applied to
 *	AL_buffer *samp
 *		buffer that the source is associated with
 *	ALshort **buffers
 *		arrays of points to PCM data, one element per 
 *		channel(left/right/etc)
 *	ALuint nc
 *		number of elements in buffers
 *	ALuint len
 *		byte length of each element in buffers
 *
 * Filters are expected to alter buffers[0..nc-1] in place.  After 
 * the ApplyFilter iteration is over, the resulting data is mixed into
 * the main mix buffer and forgotten.  The data altered is cumulative,
 * that is to say, if two filters alf_f and alf_g occur in sequential
 * order, alf_g will see the pcm data after alf_f has altered it.
 *
 * FINER POINTS:
 *
 * A lot of the filters make effects by modulating amplitude and delay.
 * Because these changes are cumulative, we can reduce the application
 * of amplitude and delay changes to one operation.  This is the point
 * of SourceParamApply --- filters can make changes to srcParams.gain
 * and srcParams.delay in a source and have those changes applied at
 * the end of the ApplyFilters call for the source.  These values are
 * reset to their defaults at the top of the ApplyFilters call.
 *
 */
#include "al_siteconfig.h"

#include "al_able.h"
#include "al_attenuation.h"
#include "al_buffer.h"
#include "al_debug.h"
#include "al_error.h"
#include "al_filter.h"
#include "al_listen.h"
#include "al_main.h"
#include "al_mixer.h"
#include "al_source.h"
#include "al_queue.h"
#include "al_vector.h"

#include "alc/alc_context.h"
#include "alc/alc_speaker.h"

#define __USE_ISOC99 1
#include <math.h>
#undef  __USE_ISOC99

#include <stdlib.h>
#include <string.h>
#include <float.h>

/* 
 * TPITCH_MAX sets the number of discrete values for AL_PITCH we can have.
 * You can set AL_PITCH to anything, but integer rounding will ensure that
 * it will fall beween MIN_SCALE and 2.0.
 *
 * 2.0 is an arbitrary constant, and likely to be changed.
 */
#define TPITCH_MAX       128

/*
 * The default software time domain filters.
 *
 * I wish I could say that the order of these does not matter,
 * but it does.  Namely, tdoppler and tpitch must occur in that
 * order, and they must occur before any other filter.  listenergain must
 * occur last.
 */
static time_filter_set software_time_filters[] = {
	{ "tdoppler",      alf_tdoppler }, /* time-domain doppler filter */
	{ "tpitch",        alf_tpitch   }, /* time-domain pitch filter */
	{ "da",            alf_da },
	{ "reverb",        alf_reverb },
	{ "coning",        alf_coning },
	{ "minmax",        alf_minmax },
	{ "listenergain",  alf_listenergain },
	{ { 0 },     NULL }
};

/*
 * compute_sa( ALfloat *source_pos, ALfloat source_max,
 *             ALfloat source_ref, ALfloat source_gain,
 *             ALfloat source_rolloff,
 *             ALfloat *speaker_pos,
 *             ALfloat (*df)( ALfloat dist, ALfloat rolloff,
 *                            ALfloat gain,
 *                            ALfloat ref, ALfloat max))
 *
 * computes distance attenuation with respect to a speaker position.
 *
 * This is some normalized value which gets expotenially closer to 1.0
 * as the source approaches the listener.  The minimum attenuation is
 * AL_CUTTOFF_ATTENUATION, which approached when the source approaches
 * the max distance.
 *
 * source_pos = source position   [x/y/z]
 * source_max = source specific max distance
 * speaker_pos = speaker position [x/y/z]
 * ref        = source's reference distance
 * df         = distance model function
 * max        = maximum distance, beyond which everything is clamped at
 *              some small value near, but not equal to, zero.
 */
static ALfloat compute_sa( ALfloat *source_pos, ALfloat source_max,
			   ALfloat source_ref, ALfloat source_gain,
			   ALfloat source_rolloff,
			   ALfloat *speaker_pos,
			   ALfloat df( ALfloat dist, ALfloat rolloff, ALfloat gain,
			   	       ALfloat ref, ALfloat max ));

/*
 * our quick lookup table for our time domain pitch filter.
 *
 *
 * We initialize each element in offsets to be a set of offsets,
 * such that
 *
 * 	offset[x][y]     = int portion of y * pitch
 *      fractinoal[x][y] = fractional portion of y * pitch
 *
 * 	Where x is the discrete integer value of pitch, and y is
 * 	any value between 0 and the length of a buffer.
 *
 * What's the point?  To save the pain of float->int conversion at
 * runtime, which is needed to map the original PCM data to a
 * "pitch modified" mapping of the same data.
 *
 */
static struct {
	int *offsets[TPITCH_MAX]; /* use int instead of ALint because
				   * these are array indexes
				   */
	float *fractionals[TPITCH_MAX];
	ALuint max;
	ALuint middle; /* the index which pitch == 1.0 corresponds to */
	ALuint len; /* length of offsets[0...TPITCH_MAX] in samples */

	int *rawoffsets;	/*
				 * raw data.  We actually alloc/dealloc this
				 * chunk and assign ->offsets in order to reduce
				 * fragmentation.
				 */
	float *rawfractionals;
} tpitch_lookup = { { NULL }, { NULL }, 0, 0, 0, NULL, NULL };

/* func associated with our tpitch lookup */
static void init_tpitch_lookup(ALuint len);

static ALfloat compute_doppler_pitch(ALfloat *object1, ALfloat *o1_vel,
			       ALfloat *object2, ALfloat *o2_vel,
			       ALfloat factor, ALfloat speed);

/*
 * _alInitTimeFilters( time_filter_set *tf_ptr_ref )
 *
 * Initializes tf_ptr_ref to the current set of time filters, and initialize
 * tpitch_lookup_max if it hasn't been initialized before.
 */
void _alInitTimeFilters( time_filter_set *tf_ptr_ref ) {
	ALuint i = 0;

	do {
		tf_ptr_ref[i] = software_time_filters[i];
	} while(software_time_filters[i++].filter != NULL);

	/*
	 * init tpitch_loopup only if it hasn't been initialized
	 * yet.
	 */
	if(tpitch_lookup.max != TPITCH_MAX) {
		tpitch_lookup.max    = TPITCH_MAX;
		tpitch_lookup.middle = TPITCH_MAX / 2;

		tpitch_lookup.rawoffsets = NULL;
		tpitch_lookup.rawfractionals = NULL;

		for(i = 0; i < tpitch_lookup.max; i++) {
			tpitch_lookup.offsets[i] = NULL;
		}
	}

	return;
}

/*
 * _alDestroyFilters( void )
 *
 * Deallocates data structures used by the filters and helper functions.
 */
void _alDestroyFilters( void ) {
	ALuint i;

	free( tpitch_lookup.rawoffsets );
	free( tpitch_lookup.rawfractionals );

	tpitch_lookup.rawoffsets = NULL;
	tpitch_lookup.rawfractionals = NULL;

	for(i = 0; i < TPITCH_MAX; i++) {
		tpitch_lookup.offsets[i] = NULL;
	}

	return;
}

/* 
 * _alApplyFilters( ALuint cid, ALuint sid )
 *
 * _alApplyFilters is called from the mixing function, and is passed
 * a source id and the context where this sourceid has meaning.
 *
 * The filters that are applied to the source are determined by the
 * context.  Each context is initialized such that it contains a table
 * of the software filters.  Extensions and plugins can be later loaded
 * to override the default functionality.  The point being, each context's
 * filter "signature" may be different.
 *
 * assumes locked source sid
 */
void _alApplyFilters( ALuint cid, ALuint sid ) {
	AL_source *src;
	AL_buffer *samp;
	time_filter_set *cc_tfilters;
	time_filter *tf;
	ALuint mixbuflen; /* byte size of total data to compose (all channels) */
	ALint len;        /* byte size of one channel's worth of data to compose */
	ALint filterlen;  /* filterlen is adjusted below to take into account looping, etc */
	int ic;           /* internal (canon) chans   */
	int mc;           /* mixer chans (==speakers) */
	ALboolean *boolp;	/* for determining bool flags */
	int i;

	/* initialize */
	ic = _al_ALCHANNELS( _ALC_CANON_FMT );

	_alcLockContext( cid );

	mc = _alcGetNumSpeakers( cid );
	mixbuflen = _alcGetWriteBufsiz( cid );

	samp = _alGetBufferFromSid( cid, sid );
	if(samp == NULL) {
		_alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
			"_alFilter: null samp, sid == %d", sid);

		_alcUnlockContext( cid );
		return;
	}

	_alcUnlockContext( cid );

	len = mixbuflen * ((float) ic / mc);
	filterlen = len;

	/*
	 * Allocate scratch space to hold enough data for the source
	 * about to be split.  We allocate more space in case of a
	 * multichannel source.
	 */
	if(f_buffers.len < len / sizeof (ALshort)) {
		void *temp;
		ALuint newlen = len * _al_ALCHANNELS(samp->format);

		for(i = 0; i < mc; i++) {
			temp = realloc(f_buffers.data[i], newlen);
			if(temp == NULL) {
				/* FIXME: do something */
			}

			f_buffers.data[i] = temp;
		}

		f_buffers.len   = newlen;
	}

	if(tpitch_lookup.len < len / sizeof (ALshort)) {
		init_tpitch_lookup(len / sizeof (ALshort));
	}

	src  = _alGetSource(cid, sid);
	if(src == NULL) {
		_alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
			"_alFilter: null src, sid == %d", sid);
		return;
	}

	/* streaming buffer?  set soundpos */
	if(samp->flags & ALB_STREAMING) {
		src->srcParams.soundpos = samp->streampos;

		if(samp->streampos > samp->size) {
			memset(src->srcParams.outbuf, 0, len);

#ifdef DEBUG_MAXIMUS
			fprintf(stderr, "underflow!!!!!!!!!!!!!!!!\n");
#endif
			return; /* underflow */
		}
	}

	_alSourceParamReset(src); /* reset srcParam settings */

	_alSplitSources(cid, sid, mc, len, samp, (ALshort **) f_buffers.data);

	/*
	 * translate head relative sources
	 */
	boolp = _alGetSourceParam(src, AL_SOURCE_RELATIVE);

	if(boolp != NULL) {
		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		"_alApplyFilters: sid %d relative boolp = %d", sid, *boolp );
		if(*boolp == AL_TRUE) {
			/* This is a RELATIVE source, which means we must
			 * translate it before applying any sort of positional
			 * filter to it.
			 */
			AL_context *cc;

			_alcLockContext( cid );

			cc = _alcGetContext(cid);
			if(cc != NULL) {
				_alSourceTranslate(src, cc->listener.Position );
			}

			_alcUnlockContext( cid );
		}
	}

	/* 
	 * adjust len to account for end of sample, looping, etc
	 */
	if(filterlen > _alSourceBytesLeft(src, samp)) {
		if(_alSourceIsLooping( src ) == AL_FALSE ) {
			/* Non looping source */
			filterlen = _alSourceBytesLeft(src, samp);
		}
	}

	if(filterlen > 0) {
		cc_tfilters = _alcGetTimeFilters(cid);

		/* apply time domain filters */
		for(i = 0; cc_tfilters[i].filter != NULL; i++) {
			tf = cc_tfilters[i].filter;

			tf(cid, src, samp, (ALshort **) f_buffers.data, mc, filterlen);
		}

		/*
		 *  Apply gain and delay for filters that don't actually touch
		 *  the data ( alf_da).
		 */
		_alSourceParamApply(src, mc, filterlen, (ALshort **) f_buffers.data);
	}

	/*
	 * Take the resulting pcm data in f_buffers, and mix these into
	 * the source's temporary output buffer.
	 */
	_alCollapseSource(cid, sid, mc, mixbuflen, (ALshort **) f_buffers.data);

	/*
	 * head RELATIVE sources need to be untranslated, lest their
	 * position become weird.
	 */
	if((boolp != NULL) && (*boolp == AL_TRUE)) {
		AL_context *cc;
		ALfloat ipos[3]; /* inverse listener position */

		_alcLockContext( cid );

		cc = _alcGetContext(cid);

		if(cc != NULL) {
			_alVectorInverse(ipos, cc->listener.Position);
			_alSourceTranslate(src, ipos);
		}

		_alcUnlockContext( cid );
	}

	return;
}

/*
 * alf_coning
 *
 * Implements the coning filter, which is used when CONE_INNER_ANGLE
 * or CONE_OUTER_ANGLE is set.  This is used for directional sounds.
 *
 * The spec is vague as to the actual requirements of directional sounds,
 * and Carlo has suggested that we maintain the DirectSound meaning for
 * directional sounds, namely (in my interpretation):
 *
 *    The inner, outer cone define three zones: inside inner cone
 *    (INSIDE), between inner and outer cone (BETWEEN, outside outer cone,
 *    (OUTSIDE).
 *
 *    In INSIDE, the gain of the sound is attenuated as a normal
 *    positional source.
 *
 *    In OUTSIDE, the gain is set to some value specified by the user.
 *
 *    In BETWEEN, the gain is transitionally set to some value between
 *    what it would be in INSIDE and OUTSIDE.
 *
 * This requires an additional source paramter, like CONE_OUTSIDE_ATTENUATION,
 * and quite frankly seems goofy.  This implementation implements the
 * following convention:
 *
 *    In INSIDE, the gain of the sound is attenuated as a normal
 *    positional source.
 *
 *    In OUTSIDE, the gain is set to _AL_CUTTOFF_ATTENUATION
 *
 *    In BETWEEN, the gain is transitionally set to some value between
 *    what it would be in INSIDE and OUTSIDE.
 *
 * Well, okay that's still pretty goofy.  Folks who want to set a 
 * minimum attenuation can stil do so using AL_SOURCE_ATTENUATION_MIN.
 *
 * IMPLEMENTATION: 
 * okay, we check the angle between the speaker position and
 * the source's direction vector, using the source's position
 * as the origin.  This angle we call theta.
 *
 * Then, we compare theta with the outer cone angle.  If it's greater,
 * we use the min attenuation.  If it's less, we compare theta with
 * the inner cone angle.  If it's greater, we attenuate as normal.
 * Otherwise, we don't attenuate at all (full volume, pitch etc).
 *
 * assumes locked source
 *
 * FIXME: please check my math.
 *	  - with an AL_NONE distance model, should this do anything
 *	    at all?
 */
void alf_coning( ALuint cid,
		 AL_source *src,
		 UNUSED(AL_buffer *samp),
		 UNUSED(ALshort **buffers),
		 ALuint nc,
		 UNUSED(ALuint len)) {
	AL_context *cc;
	ALfloat sa;  /* speaker attenuation */
	ALfloat *sp; /* source position  */
	ALfloat *sd; /* source direction */
	ALfloat *speaker_pos; /* speaker position */
	ALfloat theta; /* angle between listener and source's direction
			* vector, with the source's position as origin.
			*/
	ALfloat srcDir[3];
	ALfloat icone;      /* inner cone angle. */
	ALfloat ocone;      /* outer cone angle.  */
	ALfloat (*df)( ALfloat gain, ALfloat rolloff, ALfloat dist,
		       ALfloat ref, ALfloat max ); /* distance model func */
	ALfloat smax;       /* source specific max distance */
	ALfloat ref;        /* source specific reference distance */
	ALfloat gain;       /* source specific gain */
	ALfloat outergain;  /* source specific outer gain */
	ALfloat rolloff;    /* source specific rolloff factor */
	void *temp;
	ALuint i;

	_alcLockContext( cid );
	cc = _alcGetContext( cid );
	if(cc == NULL) {
		/* ugh.  bad context id */

		_alcUnlockContext( cid );
		return;
	}

	/*
	 * The source specific max is set to max at this point, but may be
	 * altered below of the application has set it.
	 */
	smax = FLT_MAX;
	df = cc->distance_func;

	_alcUnlockContext( cid );

	/* If no direction set, return */
	sd = _alGetSourceParam( src, AL_DIRECTION );
	if(sd == NULL) {
		/*
		 * source has no direction (normal).  leave it for alf_da
		 */
		return;
	}

	sp = _alGetSourceParam( src, AL_POSITION );
	if(sp == NULL) {
		/* If no position set, return */

		return;
	}

	/* set source specific ref distance */
	temp = _alGetSourceParam( src, AL_REFERENCE_DISTANCE );
	if( temp != NULL ) {
		ref = * (ALfloat *) temp;
	} else {
		_alSourceGetParamDefault( AL_REFERENCE_DISTANCE, &ref );
	}

	/* set source specific gain */
	temp = _alGetSourceParam( src, AL_GAIN_LINEAR_LOKI );
	if( temp != NULL ) {
		gain =  * (ALfloat *) temp;
	} else {
		_alSourceGetParamDefault( AL_GAIN_LINEAR_LOKI, &gain );
	}

	/* get source specific max distance */
	temp = _alGetSourceParam( src, AL_MAX_DISTANCE );
	if( temp != NULL ) {
		smax =  * (ALfloat *) temp;
	} else {
		_alSourceGetParamDefault( AL_MAX_DISTANCE, &smax );
	}

	/* get source specific rolloff factor */
	temp = _alGetSourceParam( src, AL_ROLLOFF_FACTOR );
	if( temp != NULL ) {
		rolloff =  * (ALfloat *) temp;

#if 0  /* rcg02012001 Don't need this. Handled in compute_sa(), now. */
		if( rolloff == 0.0f ) {
			/* FIXME: use epsilon */
			return;
		}
#endif
	} else {
		_alSourceGetParamDefault( AL_ROLLOFF_FACTOR, &rolloff );
	}

	srcDir[0] = sp[0] + sd[0];
	srcDir[1] = sp[1] + sd[1];
	srcDir[2] = sp[2] + sd[2];

	/*
	 * Get CONE settings.
	 *
	 * If unset, use 360.0 degrees
	 */
	temp = _alGetSourceParam( src, AL_CONE_INNER_ANGLE );
	if(temp != NULL) {
		icone = _alDegreeToRadian(* (ALfloat *) temp);
	} else {
		_alSourceGetParamDefault( AL_CONE_INNER_ANGLE, &icone );
	}

	temp = _alGetSourceParam( src, AL_CONE_OUTER_ANGLE );
	if(temp != NULL) {
		ocone = _alDegreeToRadian(* (ALfloat *) temp);
	} else {
		_alSourceGetParamDefault( AL_CONE_OUTER_ANGLE, &ocone );
	}

	temp = _alGetSourceParam( src, AL_CONE_OUTER_GAIN );
	if(temp != NULL) {
		outergain = * (ALfloat *) temp;
	} else {
		_alSourceGetParamDefault( AL_CONE_OUTER_GAIN, &outergain );
	}

	_alDebug(ALD_SOURCE, __FILE__, __LINE__,
		"alf_coning: sid %d icone %f ocone %f", src->sid, icone, ocone );

	for(i = 0; i < nc; i++) {
		speaker_pos = _alcGetSpeakerPosition(cid, i);
		theta = fabs( _alVectorAngleBeween(sp, speaker_pos, srcDir) );

		if( theta <= (icone / 2.0f) ) {
			/*
			 * INSIDE:
			 *
			 * attenuate normally
			 */
			_alDebug(ALD_SOURCE, __FILE__, __LINE__,
				"alf_coning: sid %d speaker %d theta %f INSIDE",
				src->sid, i, theta );

			/*
			 * speaker[i] is in inner cone, don't do
			 * anything.
			 */
			sa = compute_sa( sp, smax, ref, gain, rolloff,
					 speaker_pos, df );
		} else if( theta <= ( ocone / 2.0f) ) {
			/*
			 * BETWEEN:
			 *
			 * kind of cheesy, but we average the INSIDE
			 * and OUTSIDE attenuation.
			 */
			_alDebug(ALD_SOURCE, __FILE__, __LINE__,
				"alf_coning: sid %d speaker %d theta %f BETWEEN",
				src->sid, i, theta);

			sa = compute_sa( sp, smax, ref, gain, rolloff,
					 speaker_pos, df );

			sa += _AL_CUTTOFF_ATTENUATION;
			sa /= 2;
		} else { 
			/*
			 * OUTSIDE:
			 *
			 * Set to attenuation_min
			 */
			_alDebug(ALD_SOURCE, __FILE__, __LINE__,
				"alf_coning: sid %d speaker %d theta %f OUTSIDE",
				src->sid, i, theta );

			if( outergain < _AL_CUTTOFF_ATTENUATION ) {
				sa = _AL_CUTTOFF_ATTENUATION;
			} else {
				sa = outergain;
			}
		}

		/* set gain, to be applied in SourceParamApply */
		src->srcParams.gain[i] *= sa;
	}

	return;
}

/*
 * alf_reverb
 *
 * As far as reverb implementations go, this sucks.  Should be
 * frequency based?
 *
 * Should be able to be applied in sequence for second order 
 * approximations.
 *
 * FIXME: this is so ugly!  And consumes a ton of memory.
 */
void alf_reverb( UNUSED(ALuint cid),
		 AL_source *src,
		 AL_buffer *samp,
		 ALshort **buffers,
		 ALuint nc,
		 ALuint len ) {
	ALshort *bpt; /* pointer to passed buffers */
	ALshort *rpt; /* pointer to reverb buffers */
	ALuint i;
	ALfloat scale = src->reverb_scale;
	ALuint delay  = src->reverb_delay;
	ALuint k;
	int sample;

	/* with a delay of 0.0, no reverb possible or needed */
	if(!(src->flags & ALS_REVERB)) {
		return;
	}

	/*
	 * initialize persistent reverb buffers if they haven't been
	 * done before
	 */
	for(i = 0; i < nc; i++) {
		if(src->reverb_buf[i] == NULL) {
			src->reverb_buf[i] = malloc(samp->size);
			memset(src->reverb_buf[i], 0, samp->size);
		}
	}

	if(src->srcParams.soundpos > delay) {
		int revoffset = ((src->srcParams.soundpos - delay) / sizeof(ALshort));

		for(i = 0; i < nc; i++) {
			bpt  = buffers[i];
			rpt  = src->reverb_buf[i];
			rpt += revoffset;

			for(k = 0; k < len / sizeof(ALshort); k++) {
				sample = bpt[k] + rpt[k] * scale;

				if(sample > canon_max) {
					sample = canon_max;
				} else if (sample < canon_min) {
					sample = canon_min;
				}

				bpt[k] = sample;
			}
		}
	}

	_alBuffersAppend(src->reverb_buf,
			(void **) buffers, len, src->reverbpos, nc);

	src->reverbpos += len;

	return;
}

/* 
 * alf_da
 *
 * alf_da implements distance attenuation.  We call compute_sa to get the
 * per-speaker attenuation for each channel, and manipulate the srcParam gain
 * settings to effect that computation.
 *
 * alf_da returns early if we discover that the source has
 * either CONE_INNER_ANGLE or CONE_OUTER_ANGLE set (ie, is a
 * directional source).  In those cases, alf_coning should do
 * the distance attenuation.
 *
 * assumes locked source
 *
 * FIXME:
 *    Remind me to clean this up.
 */
void alf_da( ALuint cid,
	     AL_source *src,
	     UNUSED(AL_buffer *samp),
	     UNUSED(ALshort **buffers),
	     ALuint nc,
	     UNUSED(ALuint len)) {
	AL_context *cc;
	ALfloat *sp; /* source position */
	ALfloat *sd; /* speaker position */
	ALfloat sa;  /* speaker attenuation */
	ALfloat *listener_position;
	ALfloat *temp;
	ALuint i;
	ALfloat (*df)( ALfloat gain, ALfloat rolloff, ALfloat dist,
		       ALfloat ref, ALfloat max ); /* distance model func */
	ALfloat gain; /* source specific gain */
	ALfloat ref;  /* source specific ref distance */
	ALfloat smax;        /* source specific max distance */
	ALfloat rolloff; /* source specific rolloff factor */

	/* get distance scale */
	_alcLockContext( cid );
	cc = _alcGetContext(cid);
	if(cc == NULL) {
		_alcUnlockContext( cid );

		/* ugh.  bad context id */
		return;
	}

	df = cc->distance_func;

	_alcUnlockContext( cid );

	/*
	 * 
	 * The source specific max is set to max at this point, but may be
	 * altered below of the application has set it.
	 */
	smax = FLT_MAX;

	/*
	 * if coning is enabled for this source, then we want to
	 * let the coning filter take care of attenuating since
	 * it has more information then we do.
	 *
	 * We check the direction flag because coning may not
	 * be set (ie, they use defaults)
	 */
	temp = _alGetSourceParam(src, AL_DIRECTION);
	if(temp != NULL) {
		/*
		 * This sound has it's direction set, so leave it
		 * to the coning filter.
		 */
		_alDebug( ALD_SOURCE, __FILE__, __LINE__,
			"Directional sound, probably not right" );

		return;
	}

	/* ambient near listener */
	listener_position = _alGetListenerParam( cid, AL_POSITION );
	if(listener_position == NULL) {
		/*
		 * The listener position is unset.  This shouldn't
		 * happen.
		 */
		return;
	}

	sp = _alGetSourceParam( src, AL_POSITION );
	if(sp == NULL) {
		/*
		 * As an optimization, don't do any attenuation if
		 * the source is relative and there's no position.
		 */
		ALboolean *isrel;

		isrel = _alGetSourceParam( src, AL_SOURCE_RELATIVE );
		if ( isrel && *isrel ) {
			return;
		}

		/*
		 * no position set, so set it to the listener
		 * postition.  We should probably set it to
		 * 0.0, 0.0, 0.0 instead.
		 *
		 * We fall through to get the MIN/MAX
		 */
		sp = listener_position;

		_alDebug(ALD_SOURCE, __FILE__, __LINE__,
			"alf_da: setting to listener pos, probably not right");
	}

	/* set reference distance */
	temp = _alGetSourceParam( src, AL_REFERENCE_DISTANCE );
	if( temp != NULL ) {
		ref = * (ALfloat *) temp;
	} else {
		_alSourceGetParamDefault( AL_REFERENCE_DISTANCE, &ref );
	}

	/* set source specific gain */
	temp = _alGetSourceParam( src, AL_GAIN_LINEAR_LOKI );
	if( temp != NULL ) {
		gain = * (ALfloat *) temp;
	} else {
		_alSourceGetParamDefault( AL_GAIN_LINEAR_LOKI, &gain );
	}

	/* set source specific max distance */
	temp = _alGetSourceParam( src, AL_MAX_DISTANCE );
	if( temp != NULL ) {
		smax =  * (ALfloat *) temp;
	} else {
		_alSourceGetParamDefault( AL_MAX_DISTANCE, &smax );
	}

	/* get source specific rolloff factor */
	temp = _alGetSourceParam( src, AL_ROLLOFF_FACTOR );
	if( temp != NULL ) {
		rolloff =  * (ALfloat *) temp;

#if 0  /* rcg02012001 Don't need this. Handled in compute_sa(), now. */
		if( rolloff == 0.0f ) {
			/* FIXME: use epsilon */
			return;
		}
#endif
	} else {
		_alSourceGetParamDefault( AL_ROLLOFF_FACTOR, &rolloff );
	}

	for(i = 0; i < nc; i++) {
		sd = _alcGetSpeakerPosition( cid, i );
		sa = compute_sa( sp, smax, ref, gain, rolloff,
				 sd, df );

		src->srcParams.gain[i] *= sa;
	}

	return;
}

/*
 * init_tpitch_lookup( ALuint len )
 *
 * Initializes the tpitch lookup table.  See declaration of tpitch_lookup for
 * information on the layout and meaning of tpitch_lookup_init.
 */
static void init_tpitch_lookup( ALuint len ) {
	ALfloat step;
	ALfloat scale;
	void *temp;
	ALuint i;
	ALint j;

	if(tpitch_lookup.len >= len) {
		/* We only go through the main loop if we
		 * haven't been initialized, or have been
		 * initialized with less memory than needed.
		 */
		return;
	}
	tpitch_lookup.len = len;

	/*
	 * initialize time domain pitch filter lookup table
	 */

	/* get big chunk of data here, parcel it out in loop.
	 * resize offsets to fit at least len elements.
	 *
	 * len is the number of samples that we process for
	 * each source in our trip through ApplyFilters.
	 */
	temp = realloc(tpitch_lookup.rawoffsets,
				     len *
				     tpitch_lookup.max *
				     sizeof **tpitch_lookup.offsets);
	if(temp == NULL) {
		perror("malloc");
		_alDCSetError(AL_OUT_OF_MEMORY);

		return;
	}

	tpitch_lookup.rawoffsets = temp;

	/* same for fractional portions */
	temp = realloc( tpitch_lookup.rawfractionals,
				len *
				tpitch_lookup.max *
				sizeof **tpitch_lookup.fractionals );
	tpitch_lookup.rawfractionals = temp;

	/*
	 * For pitch < 1.0, we lower the frequency such that a pitch of
	 * 0.5 corresponds to 1 octave drop.  Is this just a linear
	 * application of the step?
	 */
	for(i = 0; i < tpitch_lookup.max; i++) {
		/* set offset part */
		temp = tpitch_lookup.rawoffsets + i * len;
		tpitch_lookup.offsets[i] = temp;

		/* set fractional part */
		temp = tpitch_lookup.rawfractionals + i * len;
		tpitch_lookup.fractionals[i] = temp;

		/* set iterate step */
		step = (2.0 * i) / tpitch_lookup.max;
		if(step == 0.0) {
			/* step = TPITCH_MIN_STEP; FIXME */
		}

		/* initialize offset table */
		scale = 0.0f;

		for(j = 0; j < (ALint) len; j++, scale += step) {
			tpitch_lookup.offsets[i][j]     = scale;
			tpitch_lookup.fractionals[i][j] = scale - (int) scale;
		}
	}

	return;
}

/*
 * alf_tdoppler
 *
 * This filter acts out the doppler effects, in the time domain as
 * opposed to frequency domain.  This filter works by computing the pitch
 * required to represent the doppler shift, and setting the AL_PITCH attribute
 * of the source directly.
 *
 * FIXME:
 *    It's not a good idea to mess with src's pitch.  Some method of
 *    expressing this computation without changing the source's attributes
 *    should be used.
 * 
 */
void alf_tdoppler( ALuint cid,
		   AL_source *src,
		   UNUSED(AL_buffer *samp),
		   UNUSED(ALshort **buffers),
		   UNUSED(ALuint nc),
		   UNUSED(ALuint len) ) {
	AL_context *cc;
	ALfloat *sv; /* source velocity */
	ALfloat *sp; /* source position */
	ALfloat *lv; /* listener velocity */
	ALfloat *lp; /* listener position */
	ALfloat relative_velocity;  /* speed of source wrt listener */
	ALfloat zeros[] = { 0.0, 0.0, 0.0 };
	AL_sourcestate *srcstate;
	ALfloat doppler_factor;
	ALfloat doppler_velocity;

	/* lock context, get context specific stuff */
	_alcLockContext( cid );

	cc = _alcGetContext(cid);
	if( cc == NULL ) {
		/* cid is an invalid context id. */
		_alcUnlockContext( cid );

		return;
	}

	lv = _alGetListenerParam( cid, AL_VELOCITY );
	lp = _alGetListenerParam( cid, AL_POSITION );

	doppler_factor   = cc->doppler_factor;
	doppler_velocity = cc->doppler_velocity;

	_alcUnlockContext( cid );

	sp = _alGetSourceParam(src, AL_POSITION );
	sv = _alGetSourceParam(src, AL_VELOCITY );

	if((sp == NULL) || (lp == NULL)) {
		return;
	}

	if((sv == NULL) && (lv == NULL)) {
		/* no velocity set, no doppler effect */
		return;
	}

	if(sv == NULL) {
		/*
		 * if unset, set to the velocity to the
		 * zero vector.
		 */
		sv = zeros;
	}

	if(lv == NULL) {
		/*
		 * if unset, set to the velocity to the
		 * zero vector.
		 */
		lv = zeros;
	}

	relative_velocity = _alVectorMagnitude(sv, lv);
	if(relative_velocity == 0.0) {
		/*
		 * no relative velocity, no doppler
		 *
		 * FIXME: use epsilon
		 */
		src->pitch.data = 1.0;
		
		return;
	}


	srcstate = _alSourceQueueGetCurrentState(src);
	if(srcstate == NULL) {
		fprintf(stderr, "weird\n");
	}

	src->pitch.data = compute_doppler_pitch(lp, lv, sp, sv,
				doppler_factor, doppler_velocity);

	return;
}

/*
 * alf_minmax
 *
 * Implements min/max gain.  First min is applied, then max.
 */
void alf_minmax( UNUSED(ALuint cid),
		 AL_source *src,
		 UNUSED(AL_buffer *samp),
		 UNUSED(ALshort **buffers),
		 ALuint nc,
		 UNUSED(ALuint len) ) {
	ALfloat *amaxp = _alGetSourceParam( src, AL_MAX_GAIN );
	ALfloat *aminp = _alGetSourceParam( src, AL_MIN_GAIN );
	ALfloat attenuation_min;
	ALfloat attenuation_max;
	ALuint i;

	/*
	 * if min or max are set, use them.  Otherwise, keep defaults
	 */
	if(aminp != NULL) {
		attenuation_min = _alDBToLinear(*aminp);
	} else {
		_alSourceGetParamDefault( AL_MIN_GAIN, &attenuation_min );
	}

	if(amaxp != NULL) {
		attenuation_max = _alDBToLinear(*amaxp);
	} else {
		_alSourceGetParamDefault( AL_MAX_GAIN, &attenuation_max );
	}

	for(i = 0; i < nc; i++) {
		if( src->srcParams.gain[i] > attenuation_max ) {
			src->srcParams.gain[i] = attenuation_max;
		} else if( src->srcParams.gain[i] < attenuation_min ) {
			src->srcParams.gain[i] = attenuation_min;
		}
	}

	return;
}

/*
 * alf_listenergain
 *
 * Implements listener gain.
 */
void alf_listenergain( ALuint cid,
		       AL_source *src,
		       UNUSED(AL_buffer *samp),
		       UNUSED(ALshort **buffers),
		       ALuint nc,
		       UNUSED(ALuint len) ) {
	ALfloat gain = 1.0f;
	void *temp;
	ALuint i;

	temp = _alGetListenerParam( cid, AL_GAIN_LINEAR_LOKI );

	if(temp == NULL) {
		_alDebug( ALD_SOURCE, __FILE__, __LINE__,
		       "listenergain: got NULL param" );
		return;
	}
	
	gain = * (ALfloat *) temp;

	for(i = 0; i < nc; i++) {
		src->srcParams.gain[i] *= gain;
	}

	return;
}

/*
 * compute_doppler_pitch( ALfloat *object1, ALfloat *o1_vel,
 *                        ALfloat *object2, ALfloat *o2_vel,
 *                        ALfloat factor,
 *                        ALfloat speed )
 *
 * compute_doppler_pitch is meant to return a value spanning 0.5 to 1.5,
 * which is meant to simulate the frequency shift undergone by sources
 * in relative movement wrt the listener.
 */
static ALfloat compute_doppler_pitch( ALfloat *object1, ALfloat *o1_vel,
				      ALfloat *object2, ALfloat *o2_vel,
				      ALfloat factor,  /* doppler_factor */
				      ALfloat speed ) { /* propagation_speed */
        ALfloat between[3];       /* Unit vector pointing in the direction
                                   * from one object to the other
                                   */
        ALfloat obj1V, obj2V;     /* Relative scalar velocity components */
        ALfloat ratio;            /* Ratio of relative velocities */
	ALfloat retval;           /* final doppler shift */

        /* 
         * Set up the "between" vector which points from one object to the
         * other
         */
        between[0] = object2[0] - object1[0];
        between[1] = object2[1] - object1[1];
        between[2] = object2[2] - object1[2];

        _alVectorNormalize( between, between );

        /*
         * Compute the dot product of the velocity vector and the "between"
         * vector.
         *
         * The _alVectorDotp function is not set up for computing dot products
         * for actual vectors (it works for three points that define two 
         * vectors from a common origin), so I'll do it here.
         */
        obj1V  = o1_vel[0] * between[0];
        obj1V += o1_vel[1] * between[1];
        obj1V += o1_vel[2] * between[2];

        /* Now compute the dot product for the second object */
        obj2V  = o2_vel[0] * between[0];
        obj2V += o2_vel[1] * between[1];
        obj2V += o2_vel[2] * between[2];

        /* 
         * Now compute the obj1/obj2 velocity ratio, taking into account
         * the propagation speed.  This formula is straight from the spec.
         */
        obj1V = speed + obj1V;
        obj2V = speed - obj2V;
        ratio = obj1V/obj2V;

        /* Finally, scale by the doppler factor */
	retval = factor * ratio;

	return retval;
}

/*
 * alf_tpitch
 *
 * this filter acts out AL_PITCH.
 *
 * This filter is implements AL_PITCH, but - oh-ho! - in the 
 * time domain.  All that good fft mojo going to waste.
 */
void alf_tpitch( UNUSED(ALuint cid),
		 AL_source *src,
		 AL_buffer *samp,
		 ALshort **buffers,
		 ALuint nc,
		 ALuint len ) {
	ALshort *obufptr = NULL; /* pointer to unmolested buffer data */
	ALshort *bufptr  = NULL;  /* pointer to buffers[0..nc-1] */
	ALuint l_index;   /* index into lookup table */
	ALint ipos = 0;   /* used to store offsets temporarily */
	ALint clen = 0;   /* len, adjusted to avoid overrun due to resampling */
	ALuint i;
	int *offsets;        /* pointer to set of offsets in lookup table */
	float *fractionals;  /* pointer to set of fractionals in lookup table */
	int bufchans;
	ALfloat *pitch;

	pitch = _alGetSourceParam( src, AL_PITCH );
	if( pitch == NULL ) {
		/*
		 * if pitch is unset, default or invalid, return.
		 */
		return;
	}

	bufchans = _al_ALCHANNELS(samp->format); /* we need bufchans to
						      * scale our increment
						      * of the soundpos,
						      * because of
						      * multichannel format
						      * buffers.
						      */
	/*
	 * if pitch is out of range, return.
	 */
	if((*pitch <= 0.0) || (*pitch > 2.0)) {
		_alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
 			"pitch out of range: %f", *pitch);
		return;
	}

	if(_alBufferIsCallback(samp) == AL_TRUE) {
		/* just debugging here, remove this block */

		_alDebug(ALD_BUFFER, __FILE__, __LINE__,
		      "No tpitch support for callbacks yet");

		_alSetError(cid, AL_ILLEGAL_COMMAND);
		return;
	}

	/*
	 *  We need len in samples, not bytes.
	 */
	len /= sizeof(ALshort);

	/* convert pitch into index in our lookup table */
	l_index = (*pitch / 2.0) * tpitch_lookup.max;

	/*
	 * sanity check.
	 */
	if(l_index >= tpitch_lookup.max) {
		l_index = tpitch_lookup.max - 1;
	}

	/*
	 * offsets is our set of pitch-scaled offsets, 0...pitch * len.
	 *
	 * Well, sort of.  0...pitch * len, but with len scaled such
	 * that we don't suffer a overrun if the buffer's original
	 * data is too short.
	 */
	offsets = tpitch_lookup.offsets[ l_index ];

	/*
	 *  adjust clen until we can safely avoid an overrun.
	 */
	clen = len;

	while(offsets[clen - 1] * sizeof(ALshort)
		+ src->srcParams.soundpos >= samp->size) {
		/* decrement clen until we won't suffer a buffer
		 * overrun.
		 */
		clen--;
	}

	/*
	 * Iterate over each buffers[0..nc-1]
	 */
	for(i = 0; i < nc; i++) {
		int j;

		/*
		 * Kind of breaking convention here and actually using
		 * the original buffer data instead of just resampling
		 * inside the passed buffer data.  This is because we
		 * won't have enough data to resample pitch > 1.0.
		 *
		 * We offset our original buffer pointer by the source's
		 * current position, but in samples, not in bytes
		 * (which is what src->srcParams.soundpos is in).
		 */
		obufptr  = samp->orig_buffers[i];
		obufptr += src->srcParams.soundpos / sizeof *obufptr;

		if(l_index == tpitch_lookup.middle ) {
			/* when this predicate is true, the pitch is
			 * equal to 1, which means there is no change.
			 * Therefore, we short circuit.
			 *
			 * Because we're incrementing the soundpos here,
			 * we can't just return.
			 */

			continue;
		}

		/*
		 * set bufptr to the pcm channel that we
		 * are about to change in-place.
		 */
		bufptr = buffers[i];

		/*
		 * We mess with offsets in the loop below, so reset it
		 * after each iteration.
		 */
		offsets = tpitch_lookup.offsets[ l_index ];
		fractionals = tpitch_lookup.fractionals[ l_index ];

		/*
		 * this is where the "resampling" takes place.  We do a
		 * very little bit on unrolling here, and it shouldn't
		 * be necessary, but seems to improve performance quite
		 * a bit.
		 */
		for(j = 0; j < clen; j++) {
			/* do a little interpolation */
			bufptr[j] = obufptr[ offsets[j] ] +
				fractionals[j] *
				( obufptr[ offsets[j] + 1] - obufptr[ offsets[j] ] );
		}
	}

	/*
	 *  Set offsets to a known good state.
	 */
	offsets = tpitch_lookup.offsets[l_index];

	/*
	 *  AL_PITCH (well, alf_tpitch actually) require that the
	 *  main mixer func does not increment the source's soundpos,
	 *  so we must increment it here.  If we detect an overrun, we
	 *  must reset the src's soundpos to something reasonable.
	 */
	ipos = offsets[clen - 1] + offsets[1];
	src->srcParams.soundpos += bufchans * ipos * sizeof(ALshort);
	
	if(clen < (ALint) len) {
		/*
		 * we've reached the end of this sample.
		 *
		 * Since we're handling the soundpos incrementing for
		 * this source (usually done in _alMixSources), we have
		 * to handle all the special cases here instead of 
		 * delegating them.
		 *
		 * These include callback, looping, and streaming 
		 * sources.  For now, we just handle looping and
		 * normal sources, as callback sources will probably 
		 * require added some special case logic to _alSplitSources 
		 * to give up a little more breathing room.
		 */
		if( _alSourceIsLooping( src ) == AL_TRUE ) {
			/*
			 * looping source
			 *
			 * FIXME:
			 * 	This isn't right.  soundpos should be set to
			 * 	something different, and we may need to carry
			 * 	over info so that the sound loops properly.
			 */

			/* FIXME: kind of kludgy */
			src->srcParams.soundpos = 0;
		} else {
			/*
			 * let _alMixSources know it's time for this source
			 * to die.
			 */
			src->srcParams.soundpos = samp->size;
		}
	}

	return;
}

/*
 * compute_sa( ALfloat *source_pos, ALfloat source_max,
 *             ALfloat source_ref, ALfloat source_gain,
 *             ALfloat source_rolloff,
 *             ALfloat *speaker_pos,
 *             ALfloat (*df)( ALfloat dist, ALfloat rolloff,
 *                            ALfloat gain,
 *                            ALfloat ref, ALfloat max))
 *
 * computes distance attenuation with respect to a speaker position.
 *
 * This is some normalized value which gets expotenially closer to 1.0
 * as the source approaches the listener.  The minimum attenuation is
 * AL_CUTTOFF_ATTENUATION, which approached when the source approaches
 * the max distance.
 *
 * source_pos = source position   [x/y/z]
 * source_max = source specific max distance
 * speaker_pos = speaker position [x/y/z]
 * ref        = source's reference distance
 * df         = distance model function
 * max        = maximum distance, beyond which everything is clamped at
 *              some small value near, but not equal to, zero.
 */
static ALfloat compute_sa( ALfloat *source_pos, ALfloat source_max,
			   ALfloat source_ref, ALfloat source_gain,
			   ALfloat source_rolloff,
			   ALfloat *speaker_pos,
			   ALfloat (*df)( ALfloat dist, ALfloat rolloff,
			   		  ALfloat gain,
			   		  ALfloat ref, ALfloat max)) {
	ALfloat distance;
	ALfloat retval;

#if 1
	/* rcg02012001 "Optimize" for rolloff == 0.0 */
	if (source_rolloff > 0.0) {
		distance = _alVectorMagnitude( source_pos, speaker_pos );

		retval = df( distance, source_rolloff, source_gain, source_ref,
		             source_max );
	}
	else {
		retval = source_gain;
	}
#else
	distance = _alVectorMagnitude( source_pos, speaker_pos );

	retval = df( distance, source_rolloff, source_gain, source_ref,
		     source_max );
#endif

	if( retval > 1.0 ) {
		return 1.0;
	} 

	if(retval < _AL_CUTTOFF_ATTENUATION) {
		return _AL_CUTTOFF_ATTENUATION;
	}
	
	return retval;
}
