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


#define NUMCONTEXTS 400

static void iterate(void);
static void cleanup(void);

static ALuint moving_source = 0;

static void *context_id;
static ALCdevice *dev;

static void iterate( void ) {
	context_id = alcCreateContext( dev, NULL);

	/*
	 * if we don't pause, we'll in all likelyhood hose
	 * the soundcard
	 */
	micro_sleep(800000);
	
	alcDestroyContext( context_id );
}

static void cleanup(void) {
#ifdef JLIB
	jv_check_mem();
#endif
}

int main( int argc, char* argv[] ) {
	int i = 0;

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	for( i = 0; i < 200; i++) {
		fprintf(stderr, "iteration %d\n", i);
		iterate();
	}

	cleanup();

	alcCloseDevice(  dev  );

	return 0;
}
