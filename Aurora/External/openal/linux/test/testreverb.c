#include "testlib.h"


#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WAVEFILE "sample.wav"

static void init( const char *fname );
static void cleanup(void);

static ALuint reverb_sid = 0;

static void *context_id;
static void *wave = NULL;

static void init( const char *fname ) {
	ALfloat zeroes[] = { 0.0f, 0.0f, 0.0f };
	ALfloat back[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat sourcepos[] = { 2.0f, 0.0f, 4.0f };
	ALuint locutus;
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	ALboolean err;

	alListenerfv(AL_POSITION, zeroes );
	alListenerfv(AL_VELOCITY, zeroes );
	alListenerfv(AL_ORIENTATION, front );

	alGenBuffers( 1, &locutus);

	err = alutLoadWAV(fname, &wave, &format, &size, &bits, &freq);
	if(err == AL_FALSE) {
		fprintf(stderr, "Could not include %s\n", fname);
		exit(1);
	}

	alBufferData( locutus, format, wave, size, freq );
	free(wave); /* openal makes a local copy of wave data */

	alGenSources( 1, &reverb_sid);

	alSourcefv(reverb_sid, AL_POSITION, sourcepos );
	alSourcefv(reverb_sid, AL_VELOCITY, zeroes );
	alSourcefv(reverb_sid, AL_ORIENTATION, back );
	alSourcei (reverb_sid, AL_BUFFER, locutus );

	talReverbScale(reverb_sid, 0.35);
	talReverbDelay(reverb_sid, 1);

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
	int i;

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

	alSourcePlay(reverb_sid);

	for(i = 0; i < 10; i++) {
		micro_sleep(1000000);
	}

	cleanup();

	alcCloseDevice( dev );

	return 0;
}
