#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <AL/alext.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <math.h>


#define NUMSOURCES 10
#define WAVEFILE   "boom.wav"

static void iterate(void);
static void init(const char *fname);
static void cleanup(void);

static ALuint moving_sources[NUMSOURCES];

static void *context_id;
static void *wave = NULL;
static time_t start;

static void sel_sleep(int usec);

extern int mixer_iterate(void *dummy);

static void iterate( void ) {
	static float f = 0;
	float g;

	f += .001;

	g = (sin(f) + 1.0) / 2.0;

/*
	fprintf(stderr, "AL_PITCH = %f\n", g);
	alSourcef(moving_source, AL_PITCH, g);
	*/
	alcProcessContext(context_id);
}

static void init( const char *fname ) {
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	ALuint boom;
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	ALboolean err;
	int i;

	start = time(NULL);

	alListenerfv(AL_POSITION, zeroes );
	alListenerfv(AL_VELOCITY, zeroes );
	alListenerfv(AL_ORIENTATION, front );

	alGenBuffers( 1, &boom );

	err = alutLoadWAV(fname, &wave, &format, &size, &bits, &freq);
	if(err == AL_FALSE) {
		fprintf(stderr, "Could not include %s\n", fname);
		exit(1);
	}


	alBufferData( boom, format, wave, size, freq );
	free(wave); /* openal makes a local copy of wave data */

	alGenSources( NUMSOURCES, moving_sources);

	for(i = 0; i < NUMSOURCES; i++) {
		alSourcef(  moving_sources[i], AL_GAIN_LINEAR_LOKI, 0.25 );
		alSourcei(  moving_sources[i], AL_BUFFER, boom );
		alSourcei(  moving_sources[i], AL_LOOPING, AL_TRUE);
		alSourcef(  moving_sources[i], AL_PITCH, 1.00);
	}

	return;
}

static void cleanup(void) {
	alcDestroyContext(context_id);
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
	int attrlist[] = {
		ALC_SYNC, AL_TRUE,
		0 };
	time_t shouldend;
	int i;

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

	for(i = 0; i < NUMSOURCES; i++) {
		alSourcef(moving_sources[i], AL_PITCH, 0.45);
	}

	alSourcePlayv( NUMSOURCES, moving_sources );

	shouldend = time(NULL);

	while((shouldend - start) < 300) {
		shouldend = time(NULL);

	    	iterate();
	}

	cleanup();

	alcCloseDevice( dev );

	return 0;
}
