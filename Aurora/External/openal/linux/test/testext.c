#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <stdio.h>


static void cleanup(void);

static ALuint moving_source = 0;

static void *context_id;

static void cleanup(void) {
#ifdef JLIB
	jv_check_mem();
#endif
}

#define BADPROC           "lokitest"
#define GOODPROC          "alLokiTest"

typedef void blah_type( void * );

int main( int argc, char* argv[] ) {
	ALCdevice *dev;
	blah_type *blah;
	int i = 0;

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	context_id = alcCreateContext( dev, NULL);
	if(context_id == NULL) {
		alcCloseDevice( dev );

		return 1;
	}

	alcMakeContextCurrent( context_id );

	fixup_function_pointers();

	blah = (blah_type *) alGetProcAddress((ALubyte *) BADPROC);
	if(blah != NULL) {
		fprintf(stderr, "weird, it seems %s is defined\n", BADPROC);
	}

	blah = (blah_type *) alGetProcAddress((ALubyte *) GOODPROC);
	if(blah == NULL) {
		fprintf(stderr, "weird, it seems %s is not defined\n",GOODPROC);
	} else {
		fprintf(stderr, "good, %s is %p\n", GOODPROC, (void *) blah);
	}

	blah(NULL);

	alcDestroyContext(context_id);

	alcCloseDevice( dev );

	return 0;
}
