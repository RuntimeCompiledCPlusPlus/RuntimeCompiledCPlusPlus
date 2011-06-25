#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#define DATABUFSIZE 4098

#define ADPCM_FILE "adpcm.adp"

static void init(void);
static void cleanup(void);

static ALuint moving_source[1];

static time_t start;

static void *context_id;
ALuint stereo; /* our buffer */

extern int errno;

static void init( void ) {
	ALfloat zeroes[]   = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]     = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]    = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat position[] = { -10.0f, 0.0f, 4.0f };
	ALsizei size;
	ALsizei bits;
	ALsizei freq;
	ALsizei format;
	int filelen;

	start = time(NULL);

	alListenerfv(AL_POSITION, zeroes );
	alListenerfv(AL_ORIENTATION, front );

	alGenBuffers( 1, &stereo);
	alGenSources( 1, moving_source);

	alSourcefv( moving_source[0], AL_POSITION, position );
	alSourcefv( moving_source[0], AL_ORIENTATION, back );
	alSourcei(  moving_source[0], AL_BUFFER,      stereo );
	alSourcei(  moving_source[0], AL_LOOPING,     AL_FALSE);

	return;
}

static void cleanup(void) {
#ifdef JLIB
	jv_check_mem();
#endif
}

int main( int argc, char* argv[] ) {
	ALCdevice *dev;
	int attrlist[] = { ALC_FREQUENCY, 22050 ,
			   ALC_INVALID, 0 };
	time_t shouldend;
	float sinsamp;
	ALshort buf[DATABUFSIZE];
	int blah = 0;
	void *data = NULL;
	struct stat sbuf;
	FILE *fh;
	int speed;
	int size;
	int i = 0;
	const int microsecs = 50000;
	char *fname;

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	/* Initialize ALUT. */
	context_id = alcCreateContext( dev, attrlist);
	if(context_id == NULL) {
		return 1;
	}

	alcMakeContextCurrent( context_id );

	fixup_function_pointers();

	if(argc == 1) {
		fname = ADPCM_FILE;
	} else {
		fname = argv[1];
	}

	init();

	talBombOnError();

	if(stat(fname, &sbuf) == -1) {
		perror("stat");
		return errno;
	}

	size = sbuf.st_size;

	data = malloc(size);
	if(data == NULL) {
		perror("malloc");
		return errno;
	}

	fh = fopen(fname, "rb");
	if(fh == NULL) {
		fprintf(stderr, "Could not open %s\n", fname);

		exit(1);
	}

	fread(data, 1, size, fh);

	speed = *(int *) data;

	if(talutLoadRAW_ADPCMData(stereo,
			(char*)data+4, size-4,
			speed, AL_FORMAT_MONO16) == AL_FALSE) {
		fprintf(stderr, "Could not alutLoadADPCMData_LOKI\n");
		exit(-2);
	}
	free(data);

	alSourcePlay( moving_source[0] );

	while(SourceIsPlaying(moving_source[0]) == AL_TRUE) {
		sleep(1);
	}

	cleanup();

	alcDestroyContext( context_id );
	alcCloseDevice(  dev  );

	return 0;
}
