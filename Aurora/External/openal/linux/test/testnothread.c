#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define WAVEFILE   "sample.wav"
#define NUMSOURCES 7

static void init(const char *fname);
static void cleanup(void);

static ALuint moving_sources[NUMSOURCES] = { 0 };
static void *context_id;
static void *wave = NULL;
static time_t start;

static void init(const char *fname) {
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat position[] = { 0.0f, 0.0f, -4.0f };
	ALuint boom;
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	ALboolean err;
	int i;

	start = time(NULL);

	alListenerfv(AL_POSITION, zeroes );
	alListenerfv(AL_ORIENTATION, front );

	alGenBuffers( 1, &boom );

	err = alutLoadWAV(fname, &wave, &format, &size, &bits, &freq);
	if(err == AL_FALSE) {
		fprintf(stderr, "Could not load %s\n", fname);
		exit(1);
	}

	alBufferData( boom, format, wave, size, freq );
	free(wave); /* openal makes a local copy of wave data */

	alGenSources( NUMSOURCES, moving_sources );

	for( i = 0; i < NUMSOURCES; i++) {
		alSourcefv( moving_sources[i], AL_POSITION, position );
		alSourcefv( moving_sources[i], AL_ORIENTATION, back );
		alSourcei(  moving_sources[i], AL_BUFFER, boom );
		alSourcei(  moving_sources[i], AL_LOOPING, AL_TRUE);
	}


	return;
}

static void cleanup(void) {
#ifdef DMALLOC
	dmalloc_verify(0);
	dmalloc_log_unfreed();

#endif
#ifdef JLIB
	jv_check_mem();
#endif
}

int main( int argc, char* argv[] ) {
	ALCdevice *dev;
	int attrlist[] = { ALC_SYNC, AL_TRUE, 0 };
	time_t shouldend;
	int i, j;

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	/* Initialize ALUT. */
	context_id = alcCreateContext( dev, attrlist);
	if(context_id == NULL) {
		alcCloseDevice( dev );

		return 1;
	}

	alcMakeContextCurrent( context_id );

	fixup_function_pointers();

	if(argc == 1) {
		init(WAVEFILE);
	} else {
		init(argv[1]);
	}

	/*
	 * First, play one sources,
	 * then, play two sources and so on until NUMSOURCES are playing.
	 */
	for(i = 1; i <= NUMSOURCES; i++ ) {
		fprintf(stderr, "Playing %d source(s)\n", i );

		alSourceStopv( i, moving_sources );

		for( j = 0 ; j < i; j++)
		{
			alSourcePlay( moving_sources[j] );
			alcProcessContext( context_id );
			micro_sleep( 40000 );
		}

		while( 1 ) {
			alcProcessContext( context_id );

			shouldend = time(NULL);
			if((shouldend - start) > 40)
			{
				break;
			}
		}

		start = time( NULL );
	}

	alcDestroyContext( context_id );
	alcCloseDevice( dev );

	cleanup();

	return 0;
}
