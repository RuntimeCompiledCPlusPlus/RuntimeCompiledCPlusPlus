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

#define DATABUFFERSIZE (10 * (512 * 3) * 1024)
#define NUMCONTEXTS    2

static void iterate(void);
static void init(const char *fname);
static void cleanup(void);

static void *data;

static void *context_ids[NUMCONTEXTS];
static ALuint moving_sources[NUMCONTEXTS];
static ALuint bid;

static void iterate( void ) {
	static ALfloat position[] = { 10.0f, 0.0f, -4.0f };
	static ALfloat movefactor = 4.5;
	static time_t then = 0;
	time_t now;
	int i;

	now = time( NULL );

	/* Switch between left and right bid sample every two seconds. */
	if( now - then > 2 ) {
		then = now;

		movefactor *= -1.0;
	}

	position[0] += movefactor;

	for(i = 0; i < NUMCONTEXTS; i++) {
		alcMakeContextCurrent( context_ids[i] );
		alSourcefv( moving_sources[i], AL_POSITION, position );
	}

	micro_sleep(500000);

	return;
}

static void init( const char *fname ) {
	FILE *fh;
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat position[] = { 0.0f, 0.0f, -4.0f };
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	int filelen;
	ALint err;
	int i;

	data = malloc(DATABUFFERSIZE);

	alListenerfv(AL_POSITION, zeroes );
	/* alListenerfv(AL_VELOCITY, zeroes ); */
	alListenerfv(AL_ORIENTATION, front );

	alGenBuffers( 1, &bid);

	fh = fopen(fname, "rb");
	if(fh == NULL) {
		fprintf(stderr, "Couldn't open fname\n");
		exit(1);
	}

	filelen = fread(data, 1, DATABUFFERSIZE, fh);
	fclose(fh);

	alGetError();

	alBufferData( bid, AL_FORMAT_WAVE_EXT, data, filelen, 0 );
	if( alGetError() != AL_NO_ERROR ) {
		fprintf(stderr, "Could not BufferData\n");
		exit(1);
	}

	free(data);

	for(i = 0; i < NUMCONTEXTS; i++) {
		alcMakeContextCurrent(context_ids[i]);

		alGenSources( 1, &moving_sources[i]);

		alSourcefv( moving_sources[i], AL_POSITION, position );
		/* alSourcefv( moving_sources[i], AL_VELOCITY, zeroes ); */
		alSourcei(  moving_sources[i], AL_BUFFER, bid );
		alSourcei(  moving_sources[i], AL_LOOPING, AL_TRUE);
	}

	return;
}

static void cleanup(void) {
	int i;

	for(i = 0; i < NUMCONTEXTS; i++) {
		alcDestroyContext( context_ids[i] );
	}

#ifdef JLIB
	jv_check_mem();
#endif

	return;
}

int main( int argc, char* argv[] ) {
	ALCdevice *dev;
	int attrlist[] = { ALC_FREQUENCY, 44100,
			   ALC_INVALID };
	time_t shouldend;
	time_t start;
	int i;
	ALboolean done = AL_FALSE;

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	/* Initialize ALUT. */
	for(i = 0; i < NUMCONTEXTS; i++) {
		context_ids[i] = alcCreateContext( dev, attrlist );
		if(context_ids[i] == NULL) {
			return 1;
		}
	}

	fixup_function_pointers();

	talBombOnError();

	if(argc == 1) {
		init("sample.wav");
	} else {
		init(argv[1]);
	}

	start = time(NULL);

	for(i = 0; i < NUMCONTEXTS; i++) {
		alcMakeContextCurrent(context_ids[i]);
		alSourcePlay( moving_sources[i] );
		sleep(1);
	}

#if 0
	while(done == AL_FALSE) {
		iterate();

		shouldend = time(NULL);
		if((shouldend - start) > 10) {
			for(i = 0; i < NUMCONTEXTS; i++) {
				alcMakeContextCurrent(context_ids[i]);
				alSourceStop(moving_sources[i]);
			}
		}

		done = AL_TRUE;
		for(i = 0; i < NUMCONTEXTS; i++) {
			alcMakeContextCurrent(context_ids[i]);
			done = done && !SourceIsPlaying( moving_sources[i] );
		}
	}
#else
	for(i = 0; i < 10; i++) {
		fprintf(stderr, "i = %d\n", i);
		sleep(1);
	}
#endif

	cleanup();

	return 0;
}
