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


#define WAVEFILE "sample.wav"

static void iterate(void);
static void init(const char *fname);
static void cleanup(void);

static ALuint moving_source = 0;

static void *wave = NULL;
static time_t start;
static void *cc; /* al context */

static void iterate( void ) {
	static float pitch = 1.0;

/*
	pitch -= .0021;

	alSourcef( moving_source, AL_PITCH, pitch );
*/

	micro_sleep(80000);
}

static void init(const char *fname) {
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	ALuint boom;
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	ALboolean err;

	start = time(NULL);

	alListenerfv(AL_POSITION, zeroes );
	/* alListenerfv(AL_VELOCITY, zeroes ); */
	alListenerfv(AL_ORIENTATION, front );

	alGenBuffers( 1, &boom );

	err = alutLoadWAV( fname, &wave, &format, &size, &bits, &freq);
	if(err == AL_FALSE) {
		fprintf(stderr, "Could not open %s\n", fname);
		exit(1);
	}

	alBufferData( boom, format, wave, size, freq );
	free(wave); /* openal makes a local copy of wave data */

	alGenSources( 1, &moving_source);

	alSourcef(  moving_source, AL_GAIN_LINEAR_LOKI,   0.40 );
	alSourcei(  moving_source, AL_BUFFER, boom );
	alSourcei(  moving_source, AL_LOOPING, AL_TRUE);

	return;
}

static void cleanup(void) {
	alcDestroyContext(cc);
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
	time_t shouldend;
	int attrlist[] = { ALC_FREQUENCY, 22050,
			   ALC_INVALID };

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	cc = alcCreateContext( dev, attrlist);
	if(cc == NULL) {
		alcCloseDevice( dev );

		return 1;
	}

	alcMakeContextCurrent( cc );

	fixup_function_pointers();

	if(argc == 1) {
		init(WAVEFILE);
	} else {
		init(argv[1]);
	}


	alSourcef(moving_source, AL_PITCH, 1.15);

	alSourcePlay( moving_source );

	shouldend = time(NULL);
	while((shouldend - start) <= 20) {
	    iterate();

	    shouldend = time(NULL);
	    if((shouldend - start) > 20) {
		    alSourceStop(moving_source);
	    }
	    sleep(1);
	}

	cleanup();

	alcCloseDevice( dev );

	return 0;
}
