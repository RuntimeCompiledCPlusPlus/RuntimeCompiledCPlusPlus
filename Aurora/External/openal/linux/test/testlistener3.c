#include "testlib.h"
#include "../src/al_vector.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI 3.14
#endif /* M_PI */

#define WAVEFILE "sample.wav"

#define TORAD(d) ((d / 180.0) * M_PI)

static void iterate( void );
static void init( const char *fname );
static void cleanup(void);

static ALuint rightSid;

static void *context_id;
static void *wave = NULL;

static void iterate( void ) {
	ALfloat orientation[]  = { 0.0f, 0.0f, -1.0f,
				   0.0f, 1.0f, 0.0f };
	static ALint angle = 0;

	/*
	 * rotate at vector about up vector by angle degrees.
	 */
	_alRotatePointAboutAxis(TORAD(angle), orientation, &orientation[3]);

	angle += 15; /* increment fifeteen degrees degree */

	fprintf(stderr, "orientation: \n\tAT(%f %f %f)\n\tUP(%f %f %f)\n",
		orientation[0], orientation[1], orientation[2],
		orientation[3], orientation[4], orientation[5]);

	alListenerfv(AL_ORIENTATION, orientation);

	micro_sleep(1500000);
}

static void init( const char *fname ) {
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat position[] = { 50.0f, 0.0f, 0.0f };
	ALuint boom;
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	ALboolean err;
	int i = 0;

	alListenerfv(AL_POSITION, zeroes );

	alGenBuffers( 1, &boom );

	err = alutLoadWAV( fname, &wave, &format, &size, &bits, &freq);
	if(err == AL_FALSE) {
		fprintf(stderr, "Could not load %s\n", fname);
		exit(1);
	}

	alBufferData( boom, format, wave, size, freq );
	free(wave); /* openal makes a local copy of wave data */

	alGenSources(1, &rightSid);

	alSourcefv( rightSid, AL_POSITION, position );
	alSourcei( rightSid, AL_BUFFER, boom );
	alSourcei( rightSid, AL_LOOPING, AL_TRUE);

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
	time_t start;
	time_t shouldend;

	start = time(NULL);
	shouldend = time(NULL);

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	/* Initialize ALUT. */
	context_id = alcCreateContext( dev, NULL);
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

	alSourcePlay(rightSid);

	while(shouldend - start < 20) {
		shouldend = time(NULL);

		iterate();
	}

	cleanup();

	alcCloseDevice( dev );

	return 0;
}
