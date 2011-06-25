#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#define WAVEFILE      "boom.wav"
#define NUMSOURCES    15

static void iterate( void );
static void init(const char *fname);
static void cleanup(void);

static ALuint multis[NUMSOURCES] = { 0 };

static void *context_id;
static void *wave = NULL;

static void iterate( void ) {
	int i;

	for(i = 0; i < NUMSOURCES; i++) {
		if(SourceIsPlaying(multis[i]) != AL_TRUE) {
			alSourcePlay( multis[i]);
		}
		micro_sleep(20000);
	}

}

static void init(const char *fname) {
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]  = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat position[] = { 2.0f, 0.0f, 4.0f };
	ALuint boom;
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	ALboolean err;
	int i = 0;

	alListenerfv(AL_POSITION, zeroes );

	alGenBuffers( 1, &boom );

	err = alutLoadWAV(fname, &wave, &format, &size, &bits, &freq);
	if(err == AL_FALSE) {
		fprintf(stderr, "Could not include %s\n", fname);
		exit(1);
	}


	alBufferData( boom, format, wave, size, freq );
	free(wave); /* openal makes a local copy of wave data */
	memset(wave, size, 0);

	alGenSources( NUMSOURCES ,multis);

	alSourcefv( multis[0], AL_POSITION, position );
	alSourcei(  multis[0], AL_BUFFER, boom );
	alSourcef(  multis[i], AL_MAX_GAIN, 1.0);
	alSourcef(  multis[i], AL_REFERENCE_DISTANCE, 1.0 );

	for(i = 1; i < NUMSOURCES ; i++) {
		position[0] = -2.0f * i;
		position[1] =  0.0;
		position[2] = -4.0f * i;

		alSourcefv( multis[i], AL_POSITION, position );
		alSourcei(  multis[i], AL_BUFFER, boom );
		alSourcef(  multis[i], AL_MAX_GAIN, 0);
		alSourcef(  multis[i], AL_REFERENCE_DISTANCE, 0.1 );
	}

	return;
}

void cleanup(void) {
	alcDestroyContext(context_id);

#ifdef JLIB
	jv_check_mem();
#endif
}

int main( int argc, char* argv[] ) {
	ALCdevice *dev;
	int attrlist[] = { ALC_FREQUENCY, 22050, 0 };
	time_t start;
	time_t shouldend;

	start = time(NULL);
	shouldend = time(NULL);

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	/* Initialize ALUT. */
	context_id = alcCreateContext( dev, attrlist );
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

	while(shouldend - start < 30) {
		shouldend = time(NULL);

		iterate();
	}

	cleanup();

	alcCloseDevice( dev );

	return 0;
}
