/*
 * -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * ac_misc.c
 *
 * misc. audioconvert funcs. 
 *
 * ripped straight from the SDL
 *
 * FIXME: consolidate ac_adpcm for MS_ADPCM stuff (ie, remove
 *        _FULL sillyness
 *        attach SDL and Sam copyright.
 */

#include "audioconvert.h"
#include "ac_endian.h"
#include "ac_wave.h"
#include "al_siteconfig.h"

#include "AL/altypes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUC__
    #ifndef DARWIN_TARGET /* darwin os uses a cc based on gcc and have __GNUC__ defined */
    #define UNUSED(x) x __attribute((unused))
    #else 
    #define UNUSED(x) x 
    #endif /* DARWIN_TARGET */
#else
#define UNUSED(x) x 
#endif /* GNU_C_ */

#define PCM_CODE        0x0001
#define MS_ADPCM_CODE   0x0002
#define IMA_ADPCM_CODE  0x0011

#define MS_ADPCM_max ((1<<(16-1))-1)
#define MS_ADPCM_min -(1<<(16-1))

struct MS_ADPCM_decodestate_FULL {
	ALubyte hPredictor;
	ALushort iDelta;
	ALshort iSamp1;
	ALshort iSamp2;
} __attribute__ ((packed));

static struct MS_ADPCM_decoder_FULL {
	alWaveFMT_LOKI wavefmt;
	ALushort wSamplesPerBlock;
	ALushort wNumCoef;
	ALshort aCoeff[7][2];
	/* * * */
	struct MS_ADPCM_decodestate_FULL state[2];
} MS_ADPCM_state_FULL;

typedef struct Chunk {
	ALuint magic;
	ALuint length;
	void *data;
} Chunk;


/* decode of MS stuff in full (as opposed to ac_adpcm) */
static ALint MS_ADPCM_nibble_FULL(struct MS_ADPCM_decodestate_FULL *state,
					ALubyte nybble, ALshort *coeff);
static int MS_ADPCM_decode_FULL(ALubyte **audio_buf, ALuint *audio_len);
static int InitMS_ADPCM(alWaveFMT_LOKI *format);

static int ReadChunk(void *src, int offset, Chunk *chunk);
static int ac_isWAVEadpcm(void *data, ALuint size, int encoding);

/* read riff chunk */
static int ReadChunk(void *srcp, int offset, Chunk *chunk) {
	ALuint reader;
	ALubyte *src = srcp;

	src += offset;

	memcpy((void *) &reader, (void *) src, 4);
	chunk->magic    = swap32le(reader);

	memcpy((void *) &reader, (void *) (src+4), 4);
	chunk->length   = swap32le(reader);

	chunk->data     = (void *) (src+8);

	return chunk->length;
}

void *ac_guess_info(void *data, ALuint *size,
		ALushort *fmt, ALushort *chan, ALushort *freq){
	if(ac_is_wave(data)) {
		return ac_guess_wave_info(data, size, fmt, chan, freq);
	}
#ifdef DEBUG_CONVERT
	fprintf(stderr,
		"ac_guess_info: can only guess WAVE files right now.\n");
#endif

	return NULL;
}

/* 0 if not wave
 * 1 if is wave
 *
 * FIXME:
 *   potential endian issues;
 */
int ac_is_wave(void *data) {
	ALuint comparator;

	memcpy((void *) &comparator, data, 4);
	if(swap32le(comparator) != RIFF) {
#ifdef DEBUG_CONVERT
		fprintf(stderr, "Not a RIFF file: magic = 0x%x\n",
			comparator);
#endif
		return 0;
	}

	memcpy((void *) &comparator, (void *) ((char *)data+8), 4);
	if(swap32le(comparator) != WAVE) {
#ifdef DEBUG_CONVERT
		fprintf(stderr, "Not a WAVE file: magic = 0x%x\n",
			comparator);
#endif
		return 0;
	}

	return 1;
}

void *acLoadWAV(void *data, ALuint *size, void **udata, 
		ALushort *fmt, ALushort *chan, ALushort *freq) {
	acAudioCVT endianConverter;

	if((data == NULL) || (udata == NULL) || (size == NULL) ||
	   (fmt  == NULL) || (chan  == NULL) || (freq == NULL)) {
		return NULL;
	}

	*udata = ac_wave_to_pcm(data, size, fmt, chan, freq);
	if(*udata == NULL) {
#ifdef DEBUG_CONVERT
		/* bad mojo */
		fprintf(stderr,
		"[%s:%d] acLoadWAV: Could not load WAV\n", __FILE__, __LINE__);
#endif

		return NULL;
	}

	if((*fmt == AUDIO_S8) || (*fmt == AUDIO_U8) || (*fmt == AUDIO_S16)) {
		return *udata;
	}

	if(acBuildAudioCVT(&endianConverter,
		/* from */
		*fmt,
		*chan,
		*freq,

		/* to */
		AUDIO_S16,
		*chan,
		*freq) < 0) {
		fprintf(stderr,
			"[%s:%d] Couldn't build audio convertion data structure.",
			__FILE__, __LINE__);

		free(udata);

		return NULL;
	}

	endianConverter.buf = *udata;
	endianConverter.len = *size;

	acConvertAudio(&endianConverter);

	return endianConverter.buf;
}

/*
 * convert wave to pcm data, setting size, fmt, chan, and freq.
 *
 * usually just a memcpy, but for MS_ADPCM does that too.
 */
void *ac_wave_to_pcm(void *data, ALuint *size,
		ALushort *fmt, ALushort *chan, ALushort *freq) {
	void *retval;
	alWaveFMT_LOKI *format;
	Chunk riffchunk = { 0, 0, 0 };
	int offset = 12;
	long length;
	alIMAADPCM_state_LOKI ima_decoder;
	ALuint tempfreq;

	do {
		offset += (length=ReadChunk(data, offset, &riffchunk)) + 8;
		if(length < 0) {
			fprintf(stderr,
				"ouch length|offset [%ld|%d]\n",
				length, offset);
			return NULL;
		}
	} while ((riffchunk.magic == WAVE) ||
		 (riffchunk.magic == RIFF));
		   
	if(riffchunk.magic != FMT) {
		fprintf(stderr,
			"ouch II magic|FMT [0x%x|0x%x]\n",
				riffchunk.magic, FMT);
		return NULL;
	}

	format = (alWaveFMT_LOKI *) riffchunk.data;

	/* channels */
	*chan = swap16le(format->channels);

	/* freq */
	tempfreq = swap32le(format->frequency);
	*freq = (ALushort) tempfreq;

	switch(swap16le(format->encoding)) {
		case PCM_CODE:
		  switch(swap16le(format->bitspersample)) {
			  case 8:
			    *fmt = AUDIO_U8;
			    break;
			  case 16:
			    *fmt = AUDIO_S16LSB;
			    break;
			  default:
			    fprintf(stderr,
			    	"Unknown bits %d\n",
			        swap16le(format->bitspersample));
			    break;
		  }

		  do {
			length = ReadChunk(data, offset, &riffchunk);
			offset += (length + 8);
							
			if(length < 0) {
				fprintf(stderr,
				"ouch III length|offset|magic\t[%ld|%d|0x%x]\n",
					length, offset, riffchunk.magic);
				return NULL;
			}
		  } while(riffchunk.magic != DATA);

		  retval = malloc(length);
		  if(retval == NULL) {
			  return NULL;
		  }

		  /* FIXME: Need to convert to native format? */
#ifdef WORDS_BIGENDIAN
		  fprintf(stderr,
		  	"[%s:%d] do we need to convert to host native format here?\n",
			__FILE__, __LINE__);
#endif
		  memcpy(retval, riffchunk.data, length);

		  *size = length;
		  return retval;
		  break;
		case MS_ADPCM_CODE:
		  /* not possible to do an inplace conversion */
		  *fmt = AUDIO_S16LSB;

		  if(InitMS_ADPCM(format) < 0) {
#ifdef DEBUG
			  fprintf(stderr, "Couldn't init MSADPCM\n");
#endif
			  return NULL;
		  }

		  do {
			length = ReadChunk(data, offset, &riffchunk);
			offset += length + 8;

			retval = riffchunk.data;
		  } while (riffchunk.magic != DATA);

		  if(MS_ADPCM_decode_FULL((ALubyte **) &retval,
			  	     (ALuint *) &length) < 0) {
#ifdef DEBUG
			  fprintf(stderr, "Couldn't decode MS_ADPCM\n");
#endif
			  return NULL;
		  } 

		  *size = length;
		  return retval;
		  break;
		case IMA_ADPCM_CODE:
		  /* not possible to do an inplace conversion */
		  *fmt = AUDIO_S16LSB;

		  if(InitIMA_ADPCM(&ima_decoder, format) < 0) {
#ifdef DEBUG_CONVERT
			  fprintf(stderr, "Couldn't init IMA ADPCM\n");
#endif
			  return NULL;
		  }

		  do {
			length = ReadChunk(data, offset, &riffchunk);
			offset += length + 8;

			retval = riffchunk.data;
		  } while (riffchunk.magic != DATA);

		  if(IMA_ADPCM_decode_FULL(&ima_decoder,
			  		   (ALubyte **) &retval,
			  	           (ALuint *) &length) < 0) {
#ifdef DEBUG_CONVERT
			  fprintf(stderr, "Couldn't decode IMA_ADPCM\n");
#endif
			  return NULL;
		  } 

		  *size = length;
		  return retval;
		  break;
		default:
#ifdef DEBUG_CONVERT
		  fprintf(stderr,
		  	"[%s:%d] unknown WAVE format 0x%x\n",
			__FILE__, __LINE__, *fmt);
#endif
		  break;
	}

	return NULL;
}

void *ac_guess_wave_info(void *data, ALuint *size,
		ALushort *fmt, ALushort *chan, ALushort *freq) {
	alWaveFMT_LOKI *format;
	Chunk riffchunk = { 0, 0, 0 };
	int offset = 12;
	long length;

	do {
		length=ReadChunk(data, offset, &riffchunk);
		offset += (length + 8);

		if(length < 0) {
			fprintf(stderr, "ouch length|offset"
				"[%ld|%d]\n",
				length, offset);
			return NULL;
		}
	} while ((riffchunk.magic == WAVE) ||
		 (riffchunk.magic == RIFF));
		   

	if(riffchunk.magic != FMT) {
		fprintf(stderr, "ouch II magic|FMT"
				"[0x%x|0x%x]\n",
				riffchunk.magic, FMT);
		return NULL;
	}

	format = (alWaveFMT_LOKI *) riffchunk.data;

	/* channels */
	*chan = swap16le(format->channels);

	/* freq */
	*freq = swap16le(format->frequency);

	switch(swap16le(format->encoding)) {
		case PCM_CODE:
		  switch(swap16le(format->bitspersample)) {
			  case 8:
			    *fmt = AUDIO_U8;
			    break;
			  case 16:
			    *fmt = AUDIO_S16LSB;
			    break;
			  default:
			    fprintf(stderr, "Unknown bits %d\n",
			    	    swap16le(format->bitspersample));
			    break;
		  }

		  do {
			length = ReadChunk(data, offset, &riffchunk);
			offset += (length + 8);
			if(length < 0) {
				fprintf(stderr,
				"ouch III length|offset|magic\t[%ld|%d|0x%x]\n",
					length, offset, riffchunk.magic);
				return NULL;
			}
		  } while ((riffchunk.magic == FACT) ||
			   (riffchunk.magic == FMT)  ||
			   (riffchunk.magic == LIST) ||
			   (riffchunk.magic == WAVE) ||
			   (riffchunk.magic == RIFF));

		  *size = length;

		  return riffchunk.data;
		  break;
		case MS_ADPCM_CODE:
		  break;
		default:
#ifdef DEBUG
		  fprintf(stderr,
		  	"[%s:%d] unknown WAVE format 0x%x\n",
			__FILE__, __LINE__, *fmt);
#endif
		  break;
	}

	return NULL;
}

static int InitMS_ADPCM(alWaveFMT_LOKI *format) {
	ALubyte *rogue_feel;
	ALushort extra_info;
	int i;

	/* Set the rogue pointer to the MS_ADPCM specific data */
	MS_ADPCM_state_FULL.wavefmt.encoding = swap16le(format->encoding);
	MS_ADPCM_state_FULL.wavefmt.channels = swap16le(format->channels);
	MS_ADPCM_state_FULL.wavefmt.frequency = swap32le(format->frequency);
	MS_ADPCM_state_FULL.wavefmt.byterate = swap32le(format->byterate);
	MS_ADPCM_state_FULL.wavefmt.blockalign = swap16le(format->blockalign);
	MS_ADPCM_state_FULL.wavefmt.bitspersample =
					 swap16le(format->bitspersample);
	rogue_feel = (ALubyte *)format+sizeof(*format);
	if(sizeof(*format) == 16) {
		extra_info = ((rogue_feel[1]<<8)|rogue_feel[0]);
		rogue_feel += sizeof(ALushort);
	}

	MS_ADPCM_state_FULL.wSamplesPerBlock = ((rogue_feel[1]<<8)|rogue_feel[0]);
	rogue_feel += sizeof(ALushort);
	MS_ADPCM_state_FULL.wNumCoef = ((rogue_feel[1]<<8)|rogue_feel[0]);
	rogue_feel += sizeof(ALushort);
	if ( MS_ADPCM_state_FULL.wNumCoef != 7 ) {
		fprintf(stderr, "Unknown set of MS_ADPCM coefficients\n");
		return -1;
	}

	for(i = 0; i < MS_ADPCM_state_FULL.wNumCoef; i++) {
		MS_ADPCM_state_FULL.aCoeff[i][0] =((rogue_feel[1]<<8)|rogue_feel[0]);
		rogue_feel += sizeof(ALushort);
		MS_ADPCM_state_FULL.aCoeff[i][1] =((rogue_feel[1]<<8)|rogue_feel[0]);
		rogue_feel += sizeof(ALushort);
	}

	return 0;
}

static int MS_ADPCM_decode_FULL(ALubyte **audio_buf, ALuint *audio_len) {
	struct MS_ADPCM_decodestate_FULL *state[2];
	ALubyte *freeable, *encoded, *decoded;
	ALint encoded_len, samplesleft;
	ALbyte nybble, stereo;
	ALshort *coeff[2];
	ALint new_sample;

	/* Allocate the proper sized output buffer */
	encoded_len = *audio_len;
	encoded = *audio_buf;
	freeable = *audio_buf;
	*audio_len = (encoded_len/MS_ADPCM_state_FULL.wavefmt.blockalign) * 
				MS_ADPCM_state_FULL.wSamplesPerBlock*
				MS_ADPCM_state_FULL.wavefmt.channels*sizeof(ALshort);
	*audio_buf = malloc(*audio_len);
	if(*audio_buf == NULL) {
		perror("malloc");
		return -1;
	}
	decoded = *audio_buf;

	/* Get ready... Go! */
	stereo = (MS_ADPCM_state_FULL.wavefmt.channels == 2);
	state[0] = &MS_ADPCM_state_FULL.state[0];
	state[1] = &MS_ADPCM_state_FULL.state[(int) stereo];
	while ( encoded_len >= MS_ADPCM_state_FULL.wavefmt.blockalign ) {
		/* Grab the initial information for this block */
		state[0]->hPredictor = *encoded++;
		if ( stereo ) {
			state[1]->hPredictor = *encoded++;
		}
		state[0]->iDelta = ((encoded[1]<<8)|encoded[0]);
		encoded += sizeof(ALshort);
		if ( stereo ) {
			state[1]->iDelta = ((encoded[1]<<8)|encoded[0]);
			encoded += sizeof(ALshort);
		}
		state[0]->iSamp1 = ((encoded[1]<<8)|encoded[0]);
		encoded += sizeof(ALshort);
		if ( stereo ) {
			state[1]->iSamp1 = ((encoded[1]<<8)|encoded[0]);
			encoded += sizeof(ALshort);
		}
		state[0]->iSamp2 = ((encoded[1]<<8)|encoded[0]);
		encoded += sizeof(ALshort);
		if ( stereo ) {
			state[1]->iSamp2 = ((encoded[1]<<8)|encoded[0]);
			encoded += sizeof(ALshort);
		}
		coeff[0] = MS_ADPCM_state_FULL.aCoeff[state[0]->hPredictor];
		coeff[1] = MS_ADPCM_state_FULL.aCoeff[state[1]->hPredictor];

		/* Store the two initial samples we start with */
		decoded[0] = state[0]->iSamp2&0xFF;
		decoded[1] = state[0]->iSamp2>>8;
		decoded += 2;
		if ( stereo ) {
			decoded[0] = state[1]->iSamp2&0xFF;
			decoded[1] = state[1]->iSamp2>>8;
			decoded += 2;
		}
		decoded[0] = state[0]->iSamp1&0xFF;
		decoded[1] = state[0]->iSamp1>>8;
		decoded += 2;
		if ( stereo ) {
			decoded[0] = state[1]->iSamp1&0xFF;
			decoded[1] = state[1]->iSamp1>>8;
			decoded += 2;
		}

		/* Decode and store the other samples in this block */
		samplesleft = (MS_ADPCM_state_FULL.wSamplesPerBlock-2)*
					MS_ADPCM_state_FULL.wavefmt.channels;
		while ( samplesleft > 0 ) {
			nybble = (*encoded)>>4;
			new_sample = MS_ADPCM_nibble_FULL(state[0],nybble,coeff[0]);
			decoded[0] = new_sample&0xFF;
			new_sample >>= 8;
			decoded[1] = new_sample&0xFF;
			decoded += 2;

			nybble = (*encoded)&0x0F;
			new_sample = MS_ADPCM_nibble_FULL(state[1],nybble,coeff[1]);
			decoded[0] = new_sample&0xFF;
			new_sample >>= 8;
			decoded[1] = new_sample&0xFF;
			decoded += 2;

			++encoded;
			samplesleft -= 2;
		}
		encoded_len -= MS_ADPCM_state_FULL.wavefmt.blockalign;
	}

	/* free(freeable); */
	return 0;
}

static ALint MS_ADPCM_nibble_FULL(struct MS_ADPCM_decodestate_FULL *state,
					ALubyte nybble, ALshort *coeff) {
	const ALint adaptive[] = {
		230, 230, 230, 230, 307, 409, 512, 614,
		768, 614, 512, 409, 307, 230, 230, 230
	};
	ALint new_sample, delta;

	new_sample = ((state->iSamp1 * coeff[0]) +
		      (state->iSamp2 * coeff[1]))/256;

	if(nybble & 0x08) {
		new_sample += state->iDelta * (nybble-0x10);
	} else {
		new_sample += state->iDelta * nybble;
	}

	if(new_sample < MS_ADPCM_min) {
		new_sample = MS_ADPCM_min;
	} else if(new_sample > MS_ADPCM_max) {
		new_sample = MS_ADPCM_max;
	}

	delta = ((ALint) state->iDelta * adaptive[nybble]);
	if(delta < 4096) {
		delta = 16;
	} else {
		delta /= 256;
	}

	state->iDelta = delta;
	state->iSamp2 = state->iSamp1;
	state->iSamp1 = new_sample;

	return new_sample;
}

/* 0 if adpcm, -1 otherwise */
static int ac_isWAVEadpcm(void *data, UNUSED(ALuint size), int encoding) {
	alWaveFMT_LOKI *format;
	Chunk riffchunk = { 0, 0, 0 };
	int offset = 12;
	long length;

	do {
		length = ReadChunk(data, offset, &riffchunk);
		offset += (length + 8);

		if(length < 0) {
			return -1;
		}
	} while ((riffchunk.magic == WAVE) ||
		 (riffchunk.magic == RIFF));

	if(riffchunk.magic != FMT) {
		return -1;
	}

	format = (alWaveFMT_LOKI *) riffchunk.data;

	if(swap16le(format->encoding) == encoding) {
		  return 0;
	}

#ifdef DEBUG_CONVERT
	fprintf(stderr, "encoding | format->enc %d|%d\n",
		format->encoding, encoding);
#endif

	return -1;
}

void *ac_getWAVEadpcm_info(void *data, ALuint *size, void *spec) {
	void *retval;
	alWaveFMT_LOKI        *format;
	alMSADPCM_state_LOKI  *mss;
	alIMAADPCM_state_LOKI *ias;
	Chunk riffchunk = { 0, 0, 0 };
	int offset = 12;
	long length;
	ALubyte *ext_format_reader;
	ALushort reader16;
	int i;

	do {
		offset += (length=ReadChunk(data, offset, &riffchunk)) + 8;
		if(length < 0) {
			return NULL;
		}
	} while ((riffchunk.magic == WAVE) ||
		 (riffchunk.magic == RIFF));
		   
	if(riffchunk.magic != FMT) {
		fprintf(stderr, "returning NULL\n");
		return NULL;
	}

	format = (alWaveFMT_LOKI *) riffchunk.data;

	do {
		length = ReadChunk(data, offset, &riffchunk);
		offset += length + 8;

		retval = riffchunk.data;
	} while (riffchunk.magic != DATA);

	*size = length;

	switch(swap16le(format->encoding)) {
		case MS_ADPCM_CODE:
		  ext_format_reader = (ALubyte *) format + 18;
		  mss = (alMSADPCM_state_LOKI *) spec;

		  mss->wavefmt.encoding      = swap16le(format->encoding);
		  mss->wavefmt.channels      = swap16le(format->channels);
		  mss->wavefmt.frequency     = swap32le(format->frequency);
		  mss->wavefmt.byterate      = swap32le(format->byterate);
		  mss->wavefmt.blockalign    = swap16le(format->blockalign);
		  mss->wavefmt.bitspersample = swap16le(format->bitspersample);

		  ext_format_reader = cp16le(ext_format_reader, &reader16);
		  mss->wSamplesPerBlock = reader16;

		  ext_format_reader = cp16le(ext_format_reader, &reader16);
		  mss->wNumCoef = reader16;

		  if(mss->wNumCoef != 7) {
			  fprintf(stderr, "wNumCoeff != 7\n");
		  }

		  for(i = 0; i < mss->wNumCoef; i++) {
			ext_format_reader = cp16le(ext_format_reader,&reader16);
			mss->aCoeff[i][0] = reader16;

			ext_format_reader = cp16le(ext_format_reader,&reader16);
			mss->aCoeff[i][1] = reader16;
		  }
		  return retval;
		case IMA_ADPCM_CODE:
		  ias = (alIMAADPCM_state_LOKI *) spec;
		  InitIMA_ADPCM(ias, format);

		  return retval;
		  break;
		case PCM_CODE:
		default:
		  fprintf(stderr, "returning NULL\n");
		  return NULL;
		  break;
	}

	return NULL;
}

int ac_isWAVE_IMA_adpcm(void *data, ALuint size) {
	return ac_isWAVEadpcm(data, size, IMA_ADPCM_CODE);
}

int ac_isWAVE_MS_adpcm(void *data, ALuint size) {
	return ac_isWAVEadpcm(data, size, MS_ADPCM_CODE);
}

int ac_isWAVE_ANY_adpcm(void *data, ALuint size) {
	int retval;

	retval = ac_isWAVEadpcm(data, size, MS_ADPCM_CODE);
	if(retval) {
		retval = ac_isWAVEadpcm(data, size, IMA_ADPCM_CODE);
	}

	return retval;
}


/* read riff chunk */
int RiffOffset(ALubyte *rawdata, ALint magic) {
	unsigned char *rawp;
	struct {
		ALint magic;
		ALint len;
	} chunk;

	rawp = rawdata + 12;

	do {
		memcpy(&chunk.magic, rawp, sizeof chunk.magic);
		rawp += sizeof chunk.magic;

		memcpy(&chunk.len,   rawp, sizeof chunk.len);
		rawp += sizeof chunk.magic;

		chunk.magic = swap32le(chunk.magic);
		chunk.len   = swap32le(chunk.len);

#ifdef DEBUG_CONVERT
		fprintf(stderr,
			"chunk.magic = %c%c%c%c\n",
			((char *) &chunk.magic)[0],
			((char *) &chunk.magic)[1],
			((char *) &chunk.magic)[2],
			((char *) &chunk.magic)[3]);
			
		fprintf(stderr,
			"chunk.len   = %d\n", chunk.len);
#endif

		if(chunk.magic == magic) {
			return rawp - rawdata;
		}

		rawp += chunk.len;
	} while(1);

	return -1;
}
