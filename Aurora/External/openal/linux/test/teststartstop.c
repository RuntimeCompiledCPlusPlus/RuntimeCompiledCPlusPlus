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
/*#define SYNCHRONIZED*/

static void iterate(void);
static void init(const char *fname);
static void cleanup(void);

static ALuint moving_source = 0;

static time_t start;
static void *data = (void *) 0xDEADBEEF;

static void *context_id;

static void iterate( void ) {
	alSourcePlay( moving_source );
#ifdef SYNCHRONIZED
	alcUpdateContext( context_id );
#endif
	alSourceStop( moving_source );
}

static void init( const char *fname ) {
	FILE *fh;
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat position[] = { 0.0f, 0.0f, -4.0f };
	ALuint stereo;
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	int filelen;
	ALint err;

	data = malloc(DATABUFFERSIZE);

	start = time(NULL);

	alListenerfv(AL_POSITION, zeroes );
	/* alListenerfv(AL_VELOCITY, zeroes ); */
	alListenerfv(AL_ORIENTATION, front );

	alGenBuffers( 1, &stereo);

	fh = fopen(fname, "rb");
	if(fh == NULL) {
		fprintf(stderr, "Couldn't open fname\n");
		exit(1);
	}

	filelen = fread(data, 1, DATABUFFERSIZE, fh);
	fclose(fh);

	alGetError();
	alBufferData( stereo, AL_FORMAT_WAVE_EXT, data, filelen, 0 );
	if( alGetError() != AL_NO_ERROR ) {
		fprintf(stderr, "Could not BufferData\n");
		exit(1);
	}

	free( data );

	alGenSources( 1, &moving_source);

	alSourcefv( moving_source, AL_POSITION, position );
	/* alSourcefv( moving_source, AL_VELOCITY, zeroes ); */
	alSourcei(  moving_source, AL_BUFFER, stereo );
	alSourcei(  moving_source, AL_LOOPING, AL_TRUE);

	return;
}

static void cleanup(void) {
	alcDestroyContext(context_id);
#ifdef JLIB
	jv_check_mem();
#endif
}

int main( int argc, char* argv[] ) {
	ALCdevice *dev;
	int attrlist[] = { ALC_FREQUENCY, 44100,
#ifdef SYNCHRONIZED
			   ALC_SYNC, AL_TRUE,
#endif
			   ALC_INVALID };
	time_t shouldend;

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	/* Initialize ALUT. */
	context_id = alcCreateContext( dev, attrlist);
	if(context_id == NULL) {
		alcCloseDevice( dev );
		return 1;
	}

	alcMakeContextCurrent( context_id );

	fixup_function_pointers();

	talBombOnError();

	if(argc == 1) {
		init("sample.wav");
	} else {
		init(argv[1]);
	}

	while( 1 ) {
		iterate();

		shouldend = time(NULL);
		if((shouldend - start) > 10) {
			break;
		}
	}

	cleanup();

	alcCloseDevice( dev );

	return 0;
}
