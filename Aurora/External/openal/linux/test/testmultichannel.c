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

static void init(const char *fname);

static ALuint moving_source = 0;

static void *wave = NULL;
static time_t start;

static void init( const char *fname ) {
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat position[] = { 0.0f, 0.0f, 0.0f };
	ALuint boom;
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	ALint err;

	start = time(NULL);

	alListenerfv(AL_POSITION, zeroes );
	/*
	alListenerfv(AL_VELOCITY, zeroes );
	alListenerfv(AL_ORIENTATION, front );
	*/

	alGenBuffers( 1, &boom );

	err = alutLoadWAV( fname, &wave, &format, &size, &bits, &freq);
	if(err == AL_FALSE) {
		fprintf(stderr, "Could not open %s\n", fname);
		exit(1);
	}

	fprintf(stderr, "(format 0x%x size %d freq %d\n",
		format, size, freq);

	switch(format) {
	    case AL_FORMAT_MONO8:
	    case AL_FORMAT_MONO16:
	      fprintf(stderr, "Not using MULTICHANNEL, format = 0x%x\n",
	      	format);
	      break;
	    default:
	 	fprintf(stderr, "Using MULTICHANNEL\n");
	      	talBufferi(boom, AL_CHANNELS, 2);
	      break;
	}
	
	alBufferData( boom, format, wave, size, freq );
	free(wave); /* openal makes a local copy of wave data */

	alGenSources( 1, &moving_source);

	alSourcefv( moving_source, AL_POSITION, position );
	alSourcef(  moving_source, AL_PITCH, 1.0 );
	/*
	alSourcefv( moving_source, AL_VELOCITY, zeroes );
	alSourcefv( moving_source, AL_ORIENTATION, back );
	*/
	alSourcei(  moving_source, AL_BUFFER, boom );
	alSourcei(  moving_source, AL_LOOPING, AL_FALSE);

	return;
}

int main( int argc, char* argv[] ) {
	ALCdevice *dev;
	void *ci;
	int attrlist[] = { ALC_FREQUENCY, 22050, 0 };

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	/* Initialize ALUT. */
	ci = alcCreateContext( dev, attrlist);
	if(ci == NULL) {
		alcCloseDevice( dev );

		exit(1);
	}

	alcMakeContextCurrent( ci );

	fixup_function_pointers();

	if(argc == 1) {
		init(WAVEFILE);
	} else {
		init(argv[1]);
	}

	alSourcePlay( moving_source );
	while(SourceIsPlaying(moving_source)) {
		micro_sleep(100000);
	}

	alcDestroyContext(ci);


	alcCloseDevice( dev );

	return 0;
}
