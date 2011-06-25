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


#define WAVEFILE "fire2.wav"

static void init(const char *fname);
static void cleanup(void);

static void *wave = NULL;
static void *context_id = NULL;
static ALuint moving_source = 0;


static void init(const char *fname) {
	ALfloat weirdpos[] = { 300.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat position[] = { 0.0f, 0.0f, 4.0f };
	ALuint boom;
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	ALboolean err;
	FILE* file = NULL;
	char* buffer = NULL;

	alListenerfv( AL_POSITION, weirdpos );

	alGenBuffers( 1, &boom );

	err = alutLoadWAV(fname, &wave, &format, &size, &bits, &freq);
	if(err == AL_FALSE) {
		fprintf(stderr, "Could not include %s\n", fname);
		exit(1);
	}

	alBufferData( boom, format, wave, size, freq );
	free(wave); /* openal makes a local copy of wave data */

	alGenSources( 1, &moving_source);

	alSourcei(  moving_source, AL_BUFFER, boom );
	alSourcei(  moving_source, AL_LOOPING, AL_TRUE );
	alSourcei(  moving_source, AL_STREAMING, AL_TRUE );
	alSourcei(  moving_source, AL_SOURCE_RELATIVE, AL_TRUE );
	alSourcefv( moving_source, AL_POSITION, position );
	alSourcef(  moving_source, AL_PITCH, 1.00 );

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
	int attrlist[] = { ALC_FREQUENCY, 22050, 0 };
			   
	time_t shouldend, start;
	ALint test = AL_FALSE;

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	/* Initialize ALUT. */
	context_id = alcCreateContext( dev, attrlist);
	if(context_id == NULL) {
		alcCloseDevice( dev );
		
		exit(1);
	}

	alcMakeContextCurrent( context_id );

	fixup_function_pointers();

	talBombOnError();

	if(argc == 1) {
		init(WAVEFILE);
	} else {
		init(argv[1]);
	}

	fprintf( stderr, "Loop for 4 seconds\n");
	
	alSourcePlay( moving_source );

	shouldend = start = time( NULL );
	
	while( shouldend - start <= 4 ) {
		shouldend = time(NULL);

		micro_sleep( 1000000 );
	}
	alSourceStop( moving_source );

	test = -1;

	alGetSourceiv( moving_source, AL_LOOPING, &test );
	fprintf(stderr, "is looping?  getsi says %s\n",
		(test == AL_TRUE)?"AL_TRUE":"AL_FALSE");

	/* part the second */
	fprintf( stderr, "Play once\n");
	micro_sleep( 1000000 );


	alSourcei( moving_source, AL_LOOPING, AL_FALSE );
	alSourcePlay( moving_source );

	do {
		micro_sleep( 1000000 );
	} while( SourceIsPlaying( moving_source ) );


	alcDestroyContext(context_id);
	alcCloseDevice( dev );	

	cleanup();

	return 0;
}
