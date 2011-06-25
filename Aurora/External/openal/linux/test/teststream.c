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


#define RAWPCM      "rawpcm.pcm"
#define DATABUFSIZE 32768

static void iterate(void);
static void init( const char *fname );
static void cleanup(void);

static ALuint moving_source = 0;

static void *data = (void *) 0xDEADBEEF;

static void *context_id;
ALuint stereo; /* our buffer */
static ALshort buf[DATABUFSIZE];
static FILE *fh;
static int bytesread;

static void iterate( void ) {
	alcProcessContext( context_id );

	return;
}

static void init( const char *fname ) {
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	int filelen;

	alListenerfv(AL_POSITION, zeroes );
	alListenerfv(AL_ORIENTATION, front );

	talGenStreamingBuffers( 1, &stereo);
	alGenSources( 1, &moving_source);

	alSourcei( moving_source, AL_SOURCE_RELATIVE, AL_TRUE );
	alSourcei( moving_source, AL_BUFFER, stereo );

	fh = fopen( fname, "rb" );
	if(fh == NULL) {
		fprintf(stderr,
			"Could not open %s\n", RAWPCM);
		exit(2);
	}

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
	time_t start;
	time_t now;
	float sinsamp;
	int i;
	int blah   = 0;
	int rsamps = 0;
	int nsamps = 0;
	unsigned int waitfor = 0;
	int attrlist[] = { ALC_FREQUENCY, 44100, ALC_SYNC, AL_TRUE, 0 };
	int delay = 0;

	dev = alcOpenDevice( (const ALubyte *) "'((sampling-rate 44100))" );
	if( dev == NULL ) {
		return 1;
	}

	/* Initialize ALUT. */
	context_id = alcCreateContext( dev, NULL);
	if(context_id == NULL) {
		alcCloseDevice( dev );

		return 1;
	}

	alcMakeContextCurrent( context_id );

	fixup_function_pointers();

	if( argc == 2 ) {
		init( argv[1] );
	} else {
		init( RAWPCM );
	}

	alSourcePlay( moving_source );

	do {
		nsamps = fread(buf, 1, DATABUFSIZE, fh) / 2;

		rsamps = 0;

		while(rsamps < nsamps) {
			micro_sleep( delay ); 

			waitfor = talBufferAppendWriteData(stereo,
					AL_FORMAT_STEREO16,
					&buf[rsamps], nsamps - rsamps, 44100,
					AL_FORMAT_STEREO16 );
			rsamps += waitfor;

			if( waitfor == 0 ) {
				delay += 500; /* add 500 millisecs */
			} else {
				/* decrease wait time */
				delay -= 500;
			}
		}
	} while(feof(fh) == 0);

	fclose(fh);

	fprintf(stderr, "rsamps = %d\n", nsamps);

	/* loop for 20 seconds */
	start = time(NULL);
	now   = time(NULL);

	for( ; now <= start + 20; now = time(NULL)) {
		fprintf(stderr, "now - start = %ld\n",
			(long int) (now - start));
		sleep(1);
	}

	alSourceStop(moving_source);

	cleanup();

	alcCloseDevice( dev );

	return 0;
}
