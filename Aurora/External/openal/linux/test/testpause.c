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


#define WAVEFILE "sample.wav"

static void iterate(void);
static void init(const char *fname);
static void cleanup(void);

static ALuint moving_source = 0;

static void *wave = NULL;
static time_t start;
static void *context_id = NULL;

static void iterate( void ) {
	sleep(1);

	return;
}

static void init(const char *fname) {
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat position[] = { 0.0f, 0.0f, -4.0f };
	ALuint sample;
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	ALboolean err;

	start = time(NULL);

	alListenerfv(AL_POSITION, zeroes );
	alListenerfv(AL_VELOCITY, zeroes );
	alListenerfv(AL_ORIENTATION, front );

	alGenBuffers( 1, &sample );

	err = alutLoadWAV( fname, &wave, &format, &size, &bits, &freq);
	if(err == AL_FALSE) {
		fprintf(stderr, "Could not load %s\n", fname);
		exit(1);
	}

	alBufferData( sample, format, wave, size, freq );
	free(wave); /* openal makes a local copy of wave data */

	alGenSources( 1, &moving_source);

	alSourcefv( moving_source, AL_POSITION, position );
	alSourcefv( moving_source, AL_VELOCITY, zeroes );
	alSourcefv( moving_source, AL_ORIENTATION, back );
	alSourcei(  moving_source, AL_BUFFER, sample );
	alSourcei(  moving_source, AL_LOOPING, AL_FALSE);

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
	static ALboolean paused = AL_FALSE;
	time_t shouldend;

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

	if(argc == 1) {
		init(WAVEFILE);
	} else {
		init(argv[1]);
	}

	alSourcePlay( moving_source );
	while(1) {
	    shouldend = time(NULL);

	    if((shouldend - start) == 3) {
		    if(paused == AL_TRUE) {
			    continue;
		    }
		    paused = AL_TRUE;

		    fprintf(stderr, "Pause\n");
		    alcSuspendContext( context_id );

		    continue;
	    }

	    if((shouldend - start) == 5) {
		    if( paused == AL_FALSE ) {
			    continue;
		    }

		    paused = AL_FALSE;
		    fprintf(stderr, "Unpause\n");
		    alcProcessContext( context_id );

		    continue;
	    }

	    if((shouldend - start) > 10) {
		    break;
	    }


	    iterate();
	}

	cleanup();

	alcCloseDevice( dev );

	return 0;
}
