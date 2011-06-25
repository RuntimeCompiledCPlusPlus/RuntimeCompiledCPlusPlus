#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#define WAVEFILE1 "boom.wav"
#define WAVEFILE2 "sample.wav"

#define	NUMBUFFERS 2
#define	NUMSOURCES 2

static void iterate(void);
static void init(void);
static void cleanup(void);

static ALuint moving_sources[NUMSOURCES];

static time_t start;
static void *data  = (void *) 0xDEADBEEF;
static void *data2 = (void *) 0xDEADBEEF;

static void *context_id;

static void iterate( void ) {
	static ALfloat position[] = { 10.0f, 0.0f, 4.0f };
	static ALfloat movefactor = 2.0;
	static time_t then        = 0;
	time_t now;

	now = time( NULL );

	/* Switch between left and right every two seconds. */
	if( now - then > 2 ) {
		then = now;

		movefactor *= -1.0;
	}

	position[0] += movefactor;
	alSourcefv( moving_sources[1], AL_POSITION, position );

	position[0] *= -1.0;
	alSourcefv( moving_sources[0], AL_POSITION, position );
	position[0] *= -1.0;

	micro_sleep(500000);

	return;
}

static void init( void ) {
	FILE *fh;
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat position[] = { 0.0f, 0.0f, -4.0f };
	ALuint  boomers[NUMBUFFERS];
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	int filelen;

	int i;

	data = malloc(5 * (512 * 3) * 1024);
	data2 = malloc(5 * (512 * 3) * 1024);

	start = time(NULL);

	alListenerfv(AL_POSITION, zeroes );
	alListenerfv(AL_VELOCITY, zeroes );
	alListenerfv(AL_ORIENTATION, front );

	alGenBuffers(NUMBUFFERS, boomers);

	fh = fopen(WAVEFILE1, "rb");
	if(fh == NULL) {
		fprintf(stderr, "Couldn't open %s\n", WAVEFILE1);
		exit(1);

	}
	filelen = fread(data, 1, 1024 * 1024, fh);
	fclose(fh);

	alGetError();

	alBufferData( boomers[0], AL_FORMAT_WAVE_EXT, data, filelen, 0 );
	if( alGetError() != AL_NO_ERROR ) {
		fprintf(stderr, "Could not BufferData\n");
		exit(1);
	}

	fh = fopen(WAVEFILE2, "rb");
	if(fh == NULL) {
		fprintf(stderr, "Couldn't open %s\n", WAVEFILE2);
		exit(1);
	}

	filelen = fread(data2, 1, 1024 * 1024, fh);
	fclose(fh);

	alBufferData( boomers[1], AL_FORMAT_WAVE_EXT, data, filelen, 0 );
	alGenSources( 2, moving_sources);

	alSourcefv( moving_sources[0], AL_POSITION, position );
	alSourcefv( moving_sources[0], AL_VELOCITY, zeroes );
	alSourcefv( moving_sources[0], AL_ORIENTATION, back );
	alSourcei(  moving_sources[0], AL_BUFFER, boomers[1] );
	alSourcei(  moving_sources[0], AL_LOOPING, AL_TRUE );

	alSourcefv( moving_sources[1], AL_POSITION, position );
	alSourcefv( moving_sources[1], AL_VELOCITY, zeroes );
	alSourcefv( moving_sources[1], AL_ORIENTATION, back );
	alSourcei(  moving_sources[1], AL_BUFFER, boomers[0] );
	alSourcei(  moving_sources[1], AL_LOOPING, AL_TRUE );

	return;
}

static void cleanup(void) {
	free(data);
	free(data2);

	alcDestroyContext(context_id);
#ifdef JLIB
	jv_check_mem();
#endif

	return;
}

int main( int argc, char* argv[] ) {
	ALCdevice *dev;
	time_t shouldend;
	int attrlist[] = { ALC_FREQUENCY, 22050, ALC_SOURCES_LOKI, 3000, 0 };

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	context_id = alcCreateContext( dev, attrlist);
	if(context_id == NULL) {
		alcCloseDevice( dev );
		return 1;
	}

	alcMakeContextCurrent( context_id );

	fixup_function_pointers();

	init( );

	alSourcePlay( moving_sources[0] );
	sleep(1);
	alSourcePlay( moving_sources[1] );

	while((SourceIsPlaying( moving_sources[0] ) == AL_TRUE) ||
	      (SourceIsPlaying( moving_sources[1] ) == AL_TRUE)) {
	    iterate();

	    shouldend = time(NULL);

	    if((shouldend - start) > 10) {
		    alSourceStop(moving_sources[0]);
		    alSourceStop(moving_sources[1]);
	    }
	}

	cleanup();

	alcCloseDevice( dev );

	return 0;
}
