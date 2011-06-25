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


#define WAVEFILE "boom.wav"

static void iterate(void);
static void init(const char *fname);
static void cleanup(void);

static ALuint moving_source = 0;

static void *wave = NULL;
static time_t start;
static void *cc; /* al context */

static void iterate( void ) {
	ALfloat position[] = { 2.0f, 0.0f, -4.0f };
	static ALfloat movefactor = 10.0;
	static time_t then = 0;
	time_t now;

	now = time( NULL );

	/* Switch between left and right boom every five seconds. */
	if( now - then > 5 ) {
		then = now;

		movefactor *= -1.0;
	}

	position[0] += movefactor;
	alListenerfv( AL_POSITION, position );

	micro_sleep(500000);

	return;
}

static void init( const char *fname ) {
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat side[]   = { 0.0f, 1.0f,  0.0f, 0.0f, 1.0f, 0.0f };
	ALfloat position[] = { 0.0f, 0.0f, -4.0f };
	ALuint boom;
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	ALboolean err;

	start = time(NULL);

	alListenerfv(AL_POSITION, zeroes );
	alListenerfv(AL_VELOCITY, zeroes );
	alListenerfv(AL_ORIENTATION, side );

	alGenBuffers( 1, &boom );

	err = alutLoadWAV(fname, &wave, &format, &size, &bits, &freq);
	if(err == AL_FALSE) {
		fprintf(stderr, "Could not include %s\n", fname);
		exit(1);
	}

	alBufferData( boom, format, wave, size, freq );
	free(wave); /* openal makes a local copy of wave data */

	alGenSources( 1, &moving_source);

	alSourcefv( moving_source, AL_POSITION, position );
	alSourcefv( moving_source, AL_VELOCITY, zeroes );
	alSourcei(  moving_source, AL_BUFFER, boom );
	alSourcei(  moving_source, AL_LOOPING, AL_TRUE);
	alSourcei(  moving_source, AL_SOURCE_RELATIVE, AL_TRUE);

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
	int attrlist[3];
	
	attrlist[0] = ALC_FREQUENCY;
	attrlist[1] = 22050;
	attrlist[2] = 0;

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

	alSourcePlay( moving_source );

	shouldend = time(NULL);
	while((shouldend - start) <= 10) {
	    iterate();

	    shouldend = time(NULL);
	}

	cleanup();

	alcCloseDevice( dev );

	return 0;
}
