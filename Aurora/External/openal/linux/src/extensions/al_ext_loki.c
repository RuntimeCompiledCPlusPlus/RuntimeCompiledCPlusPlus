/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_ext_loki.c
 *
 * (semi) standard Loki extensions.
 *
 * For more information about extensions, please check the documentation,
 * and visit al_ext.c.
 */
#include "al_siteconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <AL/al.h>

#include "al_ext_needed.h"
#include "al_ext_loki.h"

#include "al_buffer.h"
#include "al_error.h"
#include "al_debug.h"
#include "al_source.h"
#include "alc/alc_context.h"
#include "alc/alc_speaker.h"

#include <audioconvert.h>

#include "arch/interface/interface_sound.h"

static ALfloat _alcGetAudioChannel_LOKI(ALuint channel);
static void _alcSetAudioChannel_LOKI(ALuint channel, ALfloat volume);

/* BufferAppendData defines */
#define MINSTREAMCHUNKSIZE  32768 /* minimum reasonable size */
#define EXPANDSTREAMBUFSIZE 262144 /* expand buffer with reckless
				    * abandon until this point, then
				    * avoid it if possible
				    */

static struct {
	void *data;
	ALuint size;
} scratch = { NULL, 0 };

#if 0
static void *bufferAppendScratch = NULL; /* scratch space for BufferAppendData */
#endif

#ifdef OPENAL_EXTENSION

/*
 * we are not being build into the library, therefore define the
 * table that informs openal how to register the extensions upon
 * dlopen.
 */
struct { ALubyte *name; void *addr; } alExtension_03282000 [] = {
	AL_EXT_PAIR(alutLoadMS_ADPCMData_LOKI),
	AL_EXT_PAIR(alutLoadRAW_ADPCMData_LOKI),
	AL_EXT_PAIR(alutLoadIMA_ADPCMData_LOKI),
	AL_EXT_PAIR(alGenStreamingBuffers_LOKI),
	AL_EXT_PAIR(alcGetAudioChannel_LOKI),
	AL_EXT_PAIR(alcSetAudioChannel_LOKI),
	AL_EXT_PAIR(alBombOnError_LOKI),	
	AL_EXT_PAIR(alReverbScale_LOKI),
	AL_EXT_PAIR(alReverbDelay_LOKI),
	AL_EXT_PAIR(alBufferi_LOKI),
	AL_EXT_PAIR(alBufferDataWithCallback_LOKI),
	AL_EXT_PAIR(alBufferAppendData_LOKI),
	AL_EXT_PAIR(alBufferWriteData_LOKI),
	AL_EXT_PAIR(alBufferAppendWriteData_LOKI),
	AL_EXT_PAIR(alutLoadRAW_ADPCMData_LOKI),
	{ NULL, NULL }
};


/*
 *  We don't need init or fini functions, but we might as well
 *  keep them in place if, in some distant future, they turn out
 *  to be useful.
 */
void alExtInit_03282000(void) {
	fprintf(stderr, "alExtInit_03282000 STUB\n");
	return;
}

void alExtFini_03282000(void) {
	if(bufferAppendScratch != NULL) {
		free(bufferAppendScratch);
	}

	bufferAppendScratch = NULL;

	return;
}
#endif /* OPENAL_EXTENSION */

ALfloat alcGetAudioChannel_LOKI(ALuint channel) {
	ALfloat retval;

	_alcDCLockContext();

	retval = _alcGetAudioChannel_LOKI(channel);

	_alcDCUnlockContext();

	return retval;
}

void alcSetAudioChannel_LOKI(ALuint channel, ALfloat volume) {
	_alcDCLockContext();

	_alcSetAudioChannel_LOKI(channel, volume);

	_alcDCUnlockContext();
}

static void _alcSetAudioChannel_LOKI(ALuint channel, ALfloat volume) {
	AL_context *cc;
	
	cc = _alcDCGetContext();
	if(cc == NULL) {
		return;
	}

	set_audiochannel(cc->write_device->handle, channel, volume);

	return;
}

static ALfloat _alcGetAudioChannel_LOKI(ALuint channel) {
	AL_context *cc = _alcDCGetContext();

	if(cc == NULL) {
		return 0;
	}

	return get_audiochannel( cc->write_device->handle, channel);
}

/* 0.0 - 1.0 */
void alReverbScale_LOKI(ALuint sid, ALfloat param) {
	AL_source *src;

	if((param < 0.0) || (param > 1.0)) {
		/*
		 * For this kludge, the scale is normalized.
		 */
		_alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
		      "alReverbScale: invalid value %f",
		      param);

		_alDCSetError(AL_INVALID_VALUE);
		return;
	}

	_alcDCLockContext();

	src = _alDCGetSource(sid);
	if(src == NULL) {
		_alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
		      "alReverbScale: invalid source id %d",
		      sid);

		_alDCSetError(AL_INVALID_NAME);
		return;
	}

	src->reverb_scale = param;
	src->flags       |= ALS_REVERB;

	_alcDCUnlockContext();

	return;
}

/* 0.0 - 2.0 in seconds */
void alReverbDelay_LOKI(ALuint sid, ALfloat param) {
	AL_source *src;

	if((param < 0.0) || (param > 2.0)) {
		/*
		 * For this kludge, the scale is 2 * normalized.
		 */
		_alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
		      "alReverbDelay: invalid value %f",
		      param);

		_alDCSetError(AL_INVALID_VALUE);
		return;
	}

	_alcDCLockContext();

	src = _alDCGetSource(sid);
	if(src == NULL) {
		_alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
		      "alReverbScale: invalid source id %d",
		      sid);

		_alDCSetError(AL_INVALID_NAME);
		return;
	}

	src->reverb_delay = param * canon_speed * _al_ALCHANNELS(canon_format);

	src->flags |= ALS_REVERB;

	_alcDCUnlockContext();

	return;
}

void alBombOnError_LOKI(void) {
	_alShouldBombOnError_LOKI = AL_TRUE;

	return;
}

void alBufferi_LOKI(ALuint buffer, ALenum param, ALint value) {
	AL_buffer *buf = NULL;

	_alLockBuffer();
	buf = _alGetBuffer(buffer);

	if(buf == NULL) {
		_alUnlockBuffer();

		_alDebug(ALD_BUFFER, __FILE__, __LINE__,
			"buffer id %d is a bad index", buffer);

		_alDCSetError(AL_INVALID_NAME);
		return;
	}

	switch(param) {
		case AL_FREQUENCY:
		  buf->freq = value;
		  break;
		case AL_BITS:
		  switch(value) {
		      case 8:
			switch(_al_ALCHANNELS(buf->format)) {
			    case 1:
			      buf->format = AL_FORMAT_MONO8;
			      break;
			    case 2:
			      buf->format = AL_FORMAT_STEREO8;
			      break;
			    default:
			      break;
			}
			break;
		      case 16:
			switch(_al_ALCHANNELS(buf->format)) {
			    case 1:
			      buf->format = AL_FORMAT_MONO16;
			      break;
			    case 2:
			      buf->format = AL_FORMAT_STEREO16;
			      break;
			    default:
			      break;
			}
			break;
		  }
		  break;
		case AL_CHANNELS:
		  switch(value) {
		      case 1:
			switch(_al_formatbits(buf->format)) {
			    case 8:
			      buf->format = AL_FORMAT_MONO8;
			      break;
			    case 16:
			      buf->format = AL_FORMAT_MONO16;
			      break;
			    default: break;
			}
			break;
		      case 2:
			switch(_al_formatbits(buf->format)) {
			    case 8:
			      buf->format = AL_FORMAT_STEREO8;
			      break;
			    case 16:
			      buf->format = AL_FORMAT_STEREO16;
			      break;
			}
			break;
		  }
		  break;
		case AL_SIZE:
		  buf->size = value;
		  break;
		default:
		  _alDebug(ALD_BUFFER, __FILE__, __LINE__,
			"alBufferi bad param 0x%x", param);

		  _alDCSetError(AL_ILLEGAL_ENUM);
		  break;
	}

	_alUnlockBuffer();

	return;
}

/*
 *  Just a thin wrapper around our internal use callback interface.
 *  Applications that rely on the callback know when to delete sources
 *  and buffers, so the additional callback are not needed.
 */
void alBufferDataWithCallback_LOKI(ALuint bid,
		int (*Callback)(ALuint, ALuint, ALshort *, ALenum, ALint, ALint)) {

	_alBufferDataWithCallback_LOKI(bid, Callback, NULL, NULL);

	return;
}

void alInitLoki(void) {
	return;
}

void alFiniLoki(void) {
	if(scratch.size != 0) {
		free(scratch.data);
	}

	scratch.size = 0;
	scratch.data = NULL;

	return;
}

/**
 * Like buffer data but supports format hint
 */
void alBufferWriteData_LOKI( ALuint  bid,
		   ALenum  format,
                   void   *data,
		   ALsizei size,
		   ALsizei freq,
		   ALenum internalFormat ) {
	AL_buffer *buf;
	unsigned int retsize;
	void *cdata;

	_alLockBuffer();

	buf = _alGetBuffer(bid);
	if(buf == NULL) {
		_alDebug(ALD_BUFFER, __FILE__, __LINE__,
		      "alBufferData: buffer id %d not valid",
		      bid);

		_alDCSetError(AL_INVALID_NAME);

		_alUnlockBuffer();
		return;
	}

	cdata = _alBufferCanonizeData(format,
				      data,
				      size,
				      freq,
				      internalFormat,
				      buf->freq,
				      &retsize,
				      AL_FALSE);

	if(cdata == NULL) {
		/*  _alBufferCanonize Data should set error */
		_alUnlockBuffer();
		return;
	}

	if(buf->flags & ALB_STREAMING) {
		/* Streaming buffers cannot use alBufferData */
		_alDCSetError(AL_ILLEGAL_COMMAND);

		free(cdata);

		_alUnlockBuffer();

		return;
	}

	_alMonoify((ALshort **) buf->orig_buffers,
		   cdata,
		   retsize / _al_ALCHANNELS(buf->format),
		   buf->num_buffers, _al_ALCHANNELS(buf->format));

	free(cdata);

	buf->size          = retsize;

	_alUnlockBuffer();

	return;
}

/**
 * Specify data to be filled into a looping buffer.
 * This takes the current position at the time of the
 *  call, and returns the number of samples written.
 *
 *  FIXME: this version is not as stingy about memory as
 *         it could be.
 *
 *  FIXME: move convert to the bottom of the testing, so
 *   	   that only as much data as is being used will be
 *   	   converted.
 *
 *  FIXME: mostly untested
 *
 *  FIXME: this is the most horrible function ever.  I can only
 *  	   claim responsibility for 50% of the hideous horror that
 *  	   is this function.  Before you smite me to eternal damnation 
 *  	   for the monstrosity that follows, please keep in mind that
 *  	   the remaining half of the horror belongs squarely on those
 *  	   who defined the semantics and side effects of this disgusting
 *  	   monster.
 *
 */
ALsizei alBufferAppendWriteData_LOKI( ALuint   buffer,
                            ALenum   format,
                            ALvoid*    data,
		            ALsizei  osamps,
                            ALsizei  freq,
			    ALenum internalFormat) {
	AL_buffer *buf;
	ALuint osize;  /* old size */
	ALuint csize;  /* converted size */
	ALuint retval; /* our return value: number of samples we can use */
	ALuint csamps; /* number of samples to convert */
	ALuint psize;  /* predicted size of passed data after conversion */
	ALuint orig_csamps; /* original number of samples to convert, pre truncation */
	unsigned int remainingspace = 0;
	unsigned int copyoffset = 0;
	unsigned int copysize   = 0;
	ALenum tformat = 0; /* buffer's target format */
	ALuint tfreq = 0;   /* buffer's target frequency */
	void *temp = NULL;
	ALuint i;
	ALuint bufchan = _al_ALCHANNELS(internalFormat); /* channels in internal format */
	int formatWidth = _al_formatbits(format) / 8;

	_alLockBuffer();

	buf = _alGetBuffer(buffer);
	if(buf == NULL) {
		/* invalid buffers go */
		_alUnlockBuffer();

		_alDebug(ALD_BUFFER, __FILE__, __LINE__,
			"buffer id %d is invalid",
			buffer);

		_alDCSetError( AL_INVALID_NAME );

		return 0;
	}

	/*
	 * Non streaming buffers go bye bye
	 */
	if(!(buf->flags & ALB_STREAMING)) {
		_alUnlockBuffer();

		_alDebug(ALD_STREAMING, __FILE__, __LINE__,
			"buffer id %d not created with alGenStreamingBuffer",
			buffer);

		_alDCSetError( AL_ILLEGAL_COMMAND );

		return 0;
	}

	buf->format = internalFormat;

	/* initialize stuff */
	osize = buf->size;

	/*
	 * Set csamps to the size of osamps in bytes.
	 *
	 * make sure that csamps contains an even multiple of
	 * the number of channels
	 */
	csamps  = osamps;
	csamps -= (csamps % _al_ALCHANNELS(format));
	csamps *= formatWidth;

	orig_csamps = csamps;

	/* Set psize to the value that csamps would be in converted format */
	psize = _al_PCMRatioify( freq, buf->freq, format, buf->format, csamps );

	/* set retval.  We'll scale it later */
	retval = osamps;

	if(buf->streampos > buf->size) {
		/* underflow! */
		_alDebug(ALD_STREAMING,
			__FILE__, __LINE__, "underflow! sp|size %d|%d",
			buf->streampos, buf->size);

		buf->streampos = buf->appendpos = 0;
		remainingspace = bufchan * buf->size;
	} else if(buf->appendpos > buf->streampos) {
		remainingspace = bufchan * (buf->size - buf->appendpos);
	} else if(buf->size != 0) {
		remainingspace = bufchan * (buf->streampos - buf->appendpos);
	} else {
		remainingspace = 0;
	}

	if((remainingspace >= MINSTREAMCHUNKSIZE) ||
	   (psize <= remainingspace)) {
		/* only take enough space to fill buffer. */
		_alDebug(ALD_STREAMING, __FILE__, __LINE__,
		"fill data to end: rs|sp|ap. %d|%d|%d",
			remainingspace, buf->streampos, buf->appendpos);

		if(remainingspace < psize) {
			copysize = remainingspace;
		} else {
			copysize = psize;
		}

		/* scale samples */
		retval *= (float) copysize / (float) psize;

		/* remember to set copyoffset for offset_memcpy below */
		copyoffset = buf->appendpos;

		buf->appendpos += copysize / bufchan;
	} else if((osize > EXPANDSTREAMBUFSIZE)                   &&
		  (buf->streampos > MINSTREAMCHUNKSIZE / bufchan) &&
	          (buf->appendpos > buf->streampos)) {
		/*
		 *  Okay:
		 *
		 *  Since streampos is after append pos, and
		 *  streampos is less than the min stream chunk
		 *  size, we can safely wrap around and put the
		 *  data at the beginning (the WRAP flag gets
		 *  removed elsewhere.
		 *
		 *  The only thing is, if the buffer is still
		 *  fairly small, it would be nice to expand it
		 *  so that it can accomodate more data (within
		 *  reason).  So we check to see if the size
		 *  greater than a certain threshold (expandbuffersize),
		 *  below which we defer to allow BufferAppendData to
		 *  expand the buffer.
		 */
		_alDebug(ALD_STREAMING, __FILE__, __LINE__,
			"reset offset 0 osize|psize|sp|ap|rs %d|%d|%d|%d|%d",
			osize, psize, buf->streampos, buf->appendpos,
			remainingspace);

		if(buf->streampos < psize / bufchan) {
			copysize = bufchan * buf->streampos;
		} else {
			copysize = psize;
		}

		/* scale samples */
		retval *= (float) copysize / (float) psize;

		copyoffset = 0;

		buf->appendpos = copysize / bufchan;

		/* we can wrap. */
		buf->flags |= ALB_STREAMING_WRAP;
	} else if((osize < EXPANDSTREAMBUFSIZE)  && (buf->streampos < buf->appendpos)) {
		unsigned int newsize;

		_alDebug(ALD_STREAMING, __FILE__, __LINE__,
			"eb time: size|rs|ap|sp      %d|%d|%d|%d",
			osize, remainingspace, buf->appendpos, buf->streampos);

		/* we must expand the size of our buffer */
		newsize = buf->appendpos + psize / bufchan;

		ASSERT(newsize >= osize);

		for(i = 0; i < buf->num_buffers; i++) {
			temp = realloc(buf->orig_buffers[i], newsize);
			if(temp == NULL) {
				_alUnlockBuffer();

				return 0;
			}

			buf->orig_buffers[i] = temp;
		}

		/* set copy params for offset_memcpy below */
		copyoffset = buf->appendpos;
		copysize   = psize;

		/* reset size */
		buf->size = newsize;

		/* 
		 * set append
		 */
		buf->appendpos += psize / bufchan;
	}  else if(buf->size > 0) {
		/* not ready to accept data */
		_alDebug(ALD_STREAMING, __FILE__, __LINE__,
			"no room: (osize %d sp %d ap %d rs %d)",
			osize,
			buf->streampos,
			buf->appendpos,
			remainingspace);

		_alUnlockBuffer();
		return 0;
	}

	/*
	 * unlock buffer for time intensive stuff, but
	 * get buffer params first
	 */
	tformat = buf->format;
	tfreq = buf->freq;

	_alUnlockBuffer();

	/*
	 *  Recompute csamps to reflect a decrease of the amount of
	 *  data that we will use.
	 */
	csamps  = retval;
	csamps -= (csamps % _al_ALCHANNELS(format));
	csamps *= formatWidth;

	/*
	 * We should decide how much data to use such that csize is
	 * a reasonable number.
	 *
	 * bufferAppendScratch = converted data scratch space
	 * csize = converted data's size
	 *
	 * retval = number of samples that csize represents
	 */
	if(scratch.size < (ALuint) osamps * formatWidth) {
		temp = realloc(scratch.data, osamps * formatWidth);
		if(temp == NULL) {
			_alUnlockBuffer();
			return 0;
		}
	
		scratch.data = temp;
		scratch.size = osamps * formatWidth;
	}

	memcpy(scratch.data, data, osamps * formatWidth);

	temp = _alBufferCanonizeData(format,
				     scratch.data,
				     csamps,
				     freq,
				     tformat,
				     tfreq,
				     &csize,
				     AL_TRUE);
	if(temp == NULL) {
		/* conversion problem */
		_alDCSetError(AL_OUT_OF_MEMORY);

		_alDebug(ALD_CONVERT, __FILE__, __LINE__,
			"streaming buffer id %d: could not convert",
			buffer);

		return 0;
	}


	/* lock buffer again, as we are about to make changes */
	_alLockBuffer();

	if(buf->size == 0) {
		/* first time: copy data */
		_alDebug(ALD_STREAMING, __FILE__, __LINE__, "first time!");

		/* first time */
		buf->size = csize / bufchan;

		for(i = 0; i < buf->num_buffers; i++) {
			temp = realloc(buf->orig_buffers[i], csize / bufchan);

			if(temp == NULL) {
				/* FIXME: do something */
				_alUnlockBuffer();

				return 0;
			}

			buf->orig_buffers[i] = temp;
		}

		_alMonoify((ALshort **) buf->orig_buffers,
			   scratch.data,
			   csize / bufchan,
			   buf->num_buffers, bufchan);

		buf->appendpos  = csize / bufchan;

		_alUnlockBuffer();

		return osamps;
	}

	_alMonoifyOffset((ALshort **) buf->orig_buffers,
			 copyoffset,
			 scratch.data,
			 csize / bufchan,
			 buf->num_buffers, bufchan);

	_alUnlockBuffer();

	return retval;
}

/**
 * Specify data to be filled into a looping buffer.
 * This takes the current position at the time of the
 *  call, and returns the number of samples written.
 *
 *  FIXME: this version is not as stingy about memory as
 *         it could be.
 *
 *  FIXME: move convert to the bottom of the testing, so
 *   	   that only as much data as is being used will be
 *   	   converted.
 *
 *  FIXME: mostly untested
 *
 *  FIXME: this is the most horrible function ever.  I can only
 *  	   claim responsibility for 50% of the hideous horror that
 *  	   is this function.  Before you smite me to eternal damnation 
 *  	   for the monstrosity that follows, please keep in mind that
 *  	   the remaining half of the horror belongs squarely on those
 *  	   who defined the semantics and side effects of this disgusting
 *  	   monster.
 *
 */
ALsizei alBufferAppendData_LOKI( ALuint   buffer,
                            ALenum   format,
                            void*    data,
		            ALsizei  osamps,
                            ALsizei  freq ) {
	AL_buffer *buf;
	ALuint osize;  /* old size */
	ALuint csize;  /* converted size */
	ALuint nsamps; /* number of samples that csize represents */
	ALuint csamps; /* number of samples to convert */
	ALuint psize;  /* predicted size of passed data after conversion */
	ALuint orig_csamps; /* original number of samples to convert, pre truncation */
	unsigned int remainingspace = 0;
	unsigned int copyoffset = 0;
	unsigned int copysize   = 0;
	ALenum tformat = 0; /* buffer's target format */
	ALuint tfreq = 0;   /* buffer's target frequency */
	void *temp = NULL;
	ALuint i;

	_alLockBuffer();

	buf = _alGetBuffer(buffer);
	if(buf == NULL) {
		/* invalid buffers go */
		_alUnlockBuffer();

		_alDebug(ALD_BUFFER, __FILE__, __LINE__,
			"buffer id %d is invalid",
			buffer);

		_alDCSetError(AL_INVALID_NAME);

		return 0;
	}

	/*
	 * Non streaming buffers go bye bye
	 */
	if(!(buf->flags & ALB_STREAMING)) {
		_alUnlockBuffer();

		_alDebug(ALD_STREAMING, __FILE__, __LINE__,
			"buffer id %d not created with alGenStreamingBuffer",
			buffer);

		_alDCSetError(AL_ILLEGAL_COMMAND);

		return 0;
	}

	/* initialize stuff */

	osize = buf->size;

	/*
	 * Set csamps to the size of osamps in bytes.
	 *
	 * make sure that csamps contains an even multiple of
	 * the number of channels
	 */
	csamps  = osamps;
	csamps -= (csamps % _al_ALCHANNELS(format));
	csamps *= (_al_formatbits(format) / 8);

	orig_csamps = csamps;

	/* Set psize to the value that csamps would be in converted format */
	psize = _al_PCMRatioify(freq, buf->freq, format, buf->format, csamps);

	/* set nsamps */
	nsamps = osamps;

	if(buf->streampos > buf->size) {
		/* underflow! */
		_alDebug(ALD_STREAMING,
			__FILE__, __LINE__, "underflow! sp|size %d|%d",
			buf->streampos, buf->size);

		buf->streampos = buf->appendpos = 0;
		remainingspace = buf->size;
	} else if(buf->appendpos > buf->streampos) {
		remainingspace = buf->size - buf->appendpos;
	} else if(buf->size != 0) {
		remainingspace = buf->streampos - buf->appendpos;
	} else {
		remainingspace = 0;
	}

	if((remainingspace >= MINSTREAMCHUNKSIZE) ||
	   (psize <= remainingspace)) {
		/* only take enough space to fill buffer. */
		_alDebug(ALD_STREAMING, __FILE__, __LINE__,
		"fill data to end: rs|sp|ap. %d|%d|%d",
			remainingspace, buf->streampos, buf->appendpos);

		if(remainingspace < psize) {
			copysize = remainingspace;
		} else {
			copysize = psize;
		}

		/* scale samples */
		nsamps *= copysize;
		nsamps /= psize;

		/* remember to set copyoffset for offset_memcpy below */
		copyoffset = buf->appendpos;

		buf->appendpos += copysize;
	} else if((osize > EXPANDSTREAMBUFSIZE)         &&
		  (buf->streampos > MINSTREAMCHUNKSIZE) &&
	          (buf->appendpos > buf->streampos)) {
		/*
		 *  Okay:
		 *
		 *  Since streampos is after append pos, and
		 *  streampos is less than the min stream chunk
		 *  size, we can safely wrap around and put the
		 *  data at the beginning (the WRAP flag gets
		 *  removed elsewhere.
		 *
		 *  The only thing is, if the buffer is still
		 *  fairly small, it would be nice to expand it
		 *  so that it can accomodate more data (within
		 *  reason).  So we check to see if the size
		 *  greater than a certain threshold (expandbuffersize),
		 *  below which we defer to allow BufferAppendData to
		 *  expand the buffer.
		 */
		_alDebug(ALD_STREAMING, __FILE__, __LINE__,
			"reset offset 0 osize|psize|sp|ap|rs %d|%d|%d|%d|%d",
			osize, psize, buf->streampos, buf->appendpos,
			remainingspace);

		if(buf->streampos < psize) {
			copysize = buf->streampos;
		} else {
			copysize = psize;
		}

		/* scale samples */
		nsamps *= copysize;
		nsamps /= psize;

		copyoffset = 0;

		buf->appendpos = copysize;

		/* we can wrap. */
		buf->flags |= ALB_STREAMING_WRAP;
	} else if(buf->streampos < buf->appendpos) {
		unsigned int newsize;

		_alDebug(ALD_STREAMING, __FILE__, __LINE__,
			"eb time: size|rs|ap|sp      %d|%d|%d|%d",
			osize, remainingspace, buf->appendpos, buf->streampos);

		/* we must expand the size of our buffer */
		newsize = buf->appendpos + psize; 

		ASSERT(newsize >= osize);

		for(i = 0; i < buf->num_buffers; i++) {
			temp = realloc(buf->orig_buffers[i], newsize);
			if(temp == NULL) {
				_alUnlockBuffer();

				return 0;
			}

			buf->orig_buffers[i] = temp;
		}

		/* set copy params for offset_memcpy below */
		copyoffset = buf->appendpos;
		copysize   = psize;

		/* reset size */
		buf->size = newsize;

		/* 
		 * set append
		 */
		buf->appendpos += psize;
	}  else if(buf->size > 0) {
		/* not ready to accept data */
		_alDebug(ALD_STREAMING, __FILE__, __LINE__,
			"osize|sp|ap|rs %d|%d|%d|%d",
			osize,
			buf->streampos,
			buf->appendpos,
			remainingspace);

		_alUnlockBuffer();
		return 0;
	}

	/*
	 * unlock buffer for time intensive stuff, but
	 * get buffer params first
	 */
	tformat = buf->format;
	tfreq   = buf->freq;

	_alUnlockBuffer();

	/*
	 *  Recompute csamps to reflect a decrease of the amount of
	 *  data that we will use.
	 */
	csamps  = nsamps;
	csamps -= (csamps % _al_ALCHANNELS(format));
	csamps *= (_al_formatbits(format) / 8);

	/*
	 * We should decide how much data to use such that csize is
	 * a reasonable number.
	 *
	 * bufferAppendScratch = converted data scratch space
	 * csize = converted data's size
	 *
	 * nsamps = number of samples that csize represents
	 */
	if(scratch.size < csamps * (_al_formatbits(format)/8)) {
		temp = realloc(scratch.data, csamps * (_al_formatbits(format)/8));
		if(temp == NULL) {
			/* oops */
			return 0;
		}

		scratch.data = temp;
		scratch.size = csamps * (_al_formatbits(format)/8);
	}

	memcpy(scratch.data, data, csamps * (_al_formatbits(format)>>3));

	temp = _alBufferCanonizeData(format,
				     scratch.data,
				     csamps,
				     freq,
				     tformat,
				     tfreq,
				     &csize,
				     AL_TRUE);
	if(temp == NULL) {
		/* conversion problem */
		_alDCSetError(AL_OUT_OF_MEMORY);

		_alDebug(ALD_CONVERT, __FILE__, __LINE__,
			"streaming buffer id %d: could not convert",
			buffer);

		return 0;
	}

	/* lock buffer again, as we are about to make changes */
	_alLockBuffer();

	if(buf->size == 0) {
		/* first time: copy data */
		_alDebug(ALD_STREAMING, __FILE__, __LINE__,
			"first time!");

		/* first time */
		buf->size = csize / _al_ALCHANNELS(buf->format);

		for(i = 0; i < buf->num_buffers; i++) {
			temp = realloc(buf->orig_buffers[i], csize / _al_ALCHANNELS(buf->format));
			if(temp == NULL) {
				_alUnlockBuffer();

				return 0;
			}

			buf->orig_buffers[i] = temp;
		}

		_alMonoify((ALshort **) buf->orig_buffers,
			   scratch.data,
			   csize / _al_ALCHANNELS(buf->format),
			   buf->num_buffers, _al_ALCHANNELS(buf->format));

		buf->appendpos    = csize;

		_alUnlockBuffer();

		return osamps;
	}

	_alMonoifyOffset((ALshort **) buf->orig_buffers,
			 copyoffset,
			 scratch.data,
			 csize / _al_ALCHANNELS(buf->format),
			 buf->num_buffers, _al_ALCHANNELS(buf->format));

/*
	offset_memcpy(buf->_orig_buffer, copyoffset, bufferAppendScratch, copysize);
 */

	_alUnlockBuffer();

	return nsamps;
}

/*
 * alGenStreamingBuffers_LOKI
 *
 * Perform full allocation in buffers[0..n-1].  If full allocation
 * is not possible, the appropriate error is set and the function
 * returns.  Each buffer id in a sucessful call can be used with
 * BufferAppendData.
 *
 * If n is 0, legal nop.  If n < 0, set INVALID_VALUE and nop.
 *
 */
void alGenStreamingBuffers_LOKI( ALsizei n, ALuint *buffer) {
	AL_buffer *buf;
	int i;

	if(n == 0) {
		/* silently return */
		return;
	}

	if(n < 0) {
		_alDebug(ALD_BUFFER, __FILE__, __LINE__,
		      "alGenStreamingBuffers_LOKI: invalid n %d\n", n);

		_alcDCLockContext();
		_alDCSetError( AL_INVALID_VALUE );
		_alcDCUnlockContext();

		return;
	}

	/* generate normal buffers */
	alGenBuffers( n, buffer );

	_alLockBuffer();

	for( i = 0; i < n; i++ ) {
		buf = _alGetBuffer( buffer[i] );
		if( buf == NULL ) {
			/* allocation must have failed */
			_alUnlockBuffer();

			return;
		}

		/* make sure to set the streaming flag */
		buf->flags |= ALB_STREAMING;
	}

	_alUnlockBuffer();

	return;
}
