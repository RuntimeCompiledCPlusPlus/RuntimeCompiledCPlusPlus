/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_ext_mp3.c
 *
 * Temporary hack.
 */
#include "al_siteconfig.h"
#include <stdio.h>

#include "al_ext_needed.h"
#include "al_ext_mp3.h"

#include "al_buffer.h"
#include "al_ext.h"
#include "alc/alc_context.h"

#define MAX_MP3 64
#define MAX_MPEG_READ 512

#ifdef SMPEG_SUPPORT

#include <SDL/SDL.h>
#include <smpeg.h>

/* maximum MAX_MP3 simultaneous sid/offset */
static struct {
	ALuint bid;
	SMPEG *mpeg;
} mp3bid[MAX_MP3];

static struct {
	ALuint sid;
	ALuint offset;
} mp3map[MAX_MP3];

#ifdef OPENAL_EXTENSION

/*
 * we are not being build into the library, therefore define the
 * table that informs openal how to register the extensions upon
 * dlopen.
 */
struct { ALubyte *name; void *addr; } alExtension_03282000 [] = {
	AL_EXT_PAIR(alutLoadMP3_LOKI),
	{ NULL, NULL }
};

void alExtInit_03282000(void) {
	int i;

	for(i = 0; i < MAX_MP3; i++) {
		mp3map[i].sid    = 0;
		mp3map[i].offset = 0;

		mp3bid[i].bid  = 0;
		mp3bid[i].mpeg = NULL;
	}

	return;
}

void alExtFini_03282000(void) {
	int i;

	fprintf(stderr, "alExtFini_03282000 STUB\n");

	for(i = 0; i < MAX_MP3; i++) {
		if(mp3bid[i].mpeg != NULL) {
			/* Do something */
		}
	}

	return;
}

#endif 

void MP3_DestroyCallback_Sid(ALuint sid);
void MP3_DestroyCallback_Bid(ALuint bid);

static int  mp3bid_get(ALuint bid, SMPEG **mpegp);
static int  mp3bid_insert(ALuint bid, SMPEG *mpeg);
static void mp3bid_remove(ALuint bid);

static int  mp3map_get(ALuint sid, ALuint *offset);
static int  mp3map_insert(ALuint sid);
static void mp3map_update(int i, ALuint offset);
static void mp3map_remove(ALuint sid);

ALboolean alutLoadMP3_LOKI(ALuint bid, ALvoid *data, ALint size) {
	static void (*alBufferi)(ALuint, ALenum, ALint) = NULL;
	SMPEG *newMpeg;
	SDL_AudioSpec spec;

	if(alBufferi == NULL) {
		alBufferi = (void (*)(ALuint, ALenum, ALint))
			    alGetProcAddress((ALubyte *) "alBufferi_LOKI");

		if(alBufferi == NULL) {
			fprintf(stderr, "Need alBufferi_LOKI\n");
			return AL_FALSE;
		}
	}

	newMpeg = SMPEG_new_data( data, size, NULL, 0 );

	SMPEG_wantedSpec( newMpeg, &spec );

	_alcDCLockContext();

	spec.freq     = canon_speed;
	spec.format   = AUDIO_S16; /* FIXME */

	_alcDCUnlockContext();

	/* implicitly multichannel */
	alBufferi( bid, AL_CHANNELS, spec.channels );

	SMPEG_actualSpec( newMpeg, &spec );

	/* insert new bid */
	mp3bid_insert( bid, newMpeg );

	_alBufferDataWithCallback_LOKI(bid,
				MP3_Callback,
				mp3map_remove,
				mp3bid_remove);

	return AL_TRUE;
}


ALint MP3_Callback(ALuint sid,
		ALuint bid,
		ALshort *outdata,
		ALenum format,
		UNUSED(ALint freq),
		ALint samples) {
	int first;
	int second;
	SMPEG *mpeg;
	ALuint offset;
	int bytesRequested = samples * sizeof( ALshort );
	int bytesPlayed;
	int bps; /* bytes per sample */
	int i;

	if(samples > MAX_MPEG_READ) {
		first  = MP3_Callback(sid, bid, outdata, format, freq, MAX_MPEG_READ);
		second = MP3_Callback(sid, bid, outdata + MAX_MPEG_READ, format, freq, samples - MAX_MPEG_READ);
		return first + second;
		       
	}

	bps = _al_formatbits( format );

	/* get buffer specific information */
	i = mp3bid_get( bid, &mpeg );
	if(i == -1) {
		fprintf(stderr, "No buffer id %d in data structures\n", bid);

		return -1; /* weird */
	}

	/* get source specific information */
	i = mp3map_get( sid, &offset );
	if(i == -1) {
		i = mp3map_insert( sid );

		offset = AL_FALSE;

		SMPEG_enableaudio( mpeg, 1 );
		SMPEG_enablevideo( mpeg, 0 ); /* sanity check */
	}

	if( SMPEG_status(mpeg) != SMPEG_PLAYING ) {
		SMPEG_play( mpeg );
	}

	memset( outdata, 0, bytesRequested );

	bytesPlayed = SMPEG_playAudio( mpeg, (ALubyte *) outdata, bytesRequested );
	bytesPlayed /= 2;

	if(bytesPlayed < samples) {
		SMPEG_stop( mpeg );
		SMPEG_rewind( mpeg );

		return bytesPlayed;
	}

	mp3map_update(i, offset + samples);

	return samples;
}

static int mp3bid_get(ALuint bid, SMPEG **mpegp) {
	int i;

	for(i = 0; i < MAX_MP3; i++) {
		if(mp3bid[i].bid == bid) {
			*mpegp = mp3bid[i].mpeg;

			return i;
		}
	}

	return -1;
}

static int mp3bid_insert(ALuint bid, SMPEG *mpeg) {
	int i;

	for(i = 0; i < MAX_MP3; i++) {
		if(mp3bid[i].bid == bid) {
			if(mp3bid[i].mpeg != NULL) {
				SMPEG_stop( mp3bid[i].mpeg );
				SMPEG_delete( mp3bid[i].mpeg );
				mp3bid[i].mpeg = NULL;
			}

			mp3bid[i].bid = 0; /* flag for next */
		}

		if(mp3bid[i].bid == 0) {
			mp3bid[i].bid  = bid;
			mp3bid[i].mpeg = mpeg;

			return i;
		}
	}

	return 0;
}

static void mp3bid_remove( ALuint bid ) {
	int i = 0;

	for(i = 0; i < MAX_MP3; i++) {
		if(mp3bid[i].bid == (ALuint) bid) {
			if(mp3bid[i].mpeg != NULL) {
				SMPEG_stop( mp3bid[i].mpeg );
				SMPEG_delete(mp3bid[i].mpeg);
				mp3bid[i].mpeg = NULL;
			}

			mp3bid[i].bid = 0;
			return;
		}
	}

	return;
}

/* FIXME: make binary search */
static int mp3map_get(ALuint sid, ALuint *offset) {
	int i;

	for(i = 0; i < MAX_MP3; i++) {
		if(mp3map[i].sid == sid) {
			*offset = mp3map[i].offset;

			return i;
		}
	}

	return -1;
}

/* FIXME: sorted insert for binary search */
static int mp3map_insert(ALuint sid) {
	int i;

	for(i = 0; i < MAX_MP3; i++) {
		if((mp3map[i].sid == 0) ||
		   (mp3map[i].sid == sid)) {
			mp3map[i].sid    = sid;
			mp3map[i].offset  = AL_FALSE;
			
			return i;
		}
	}

	return 0;
}

static void mp3map_update(int i, ALuint offset) {
	if(i < 0) {
		return;
	}

	if(i >= MAX_MP3) {
		return;
	}

	mp3map[i].offset = offset;

	return;
}

static void mp3map_remove(ALuint sid) {
	int i;

	for(i = 0; i < MAX_MP3; i++) {
		if(mp3map[i].sid == sid) {
			mp3map[i].sid   = 0;
			mp3map[i].offset = AL_FALSE;
			
			return;
		}
	}

	return;
}

#else

/* without smpeg support, we don't do jack */

#endif /* SMPEG_SUPPORT */
