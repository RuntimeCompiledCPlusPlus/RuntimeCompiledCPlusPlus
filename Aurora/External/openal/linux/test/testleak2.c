#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#define WAVEFILE   "adpcm.adp"
#define NUMSOURCES 8000

static void init(const char *fname);
static void cleanup(void);

static ALuint moving_sources[NUMSOURCES];
static ALuint buffer_id;

static void init( const char *fname ) {
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	ALboolean err;
	FILE *fh;
	ALvoid *data;
	struct stat buf;
	int size;
	int speed;

	alListenerfv(AL_POSITION, zeroes );
	alListenerfv(AL_VELOCITY, zeroes );
	alListenerfv(AL_ORIENTATION, front );

	alGenBuffers( 1, &buffer_id );

	if(stat(fname, &buf) < 0) {
		fprintf(stderr, "Could not stat %s\n", fname);
		exit(1);
	}

	fh = fopen(fname, "rb");
	if(fh == NULL) {
		fprintf(stderr, "Could not fopen %s\n", fname);
		exit(1);
	}

	size = buf.st_size;
	data = malloc(size);

	fread(data, 1, size, fh);

	speed = *(int *) data;

	if(talutLoadRAW_ADPCMData(buffer_id,
			(char*)data+4, size-4,
			speed, AL_FORMAT_MONO16) == AL_FALSE) {
		fprintf(stderr, "Could not alutLoadADPCMData_LOKI\n");
		exit(-2);
	}

	free(data);

	return;
}

static void cleanup(void) {
	alutExit();
	
#ifdef DMALLOC
	dmalloc_verify(0);
	dmalloc_log_unfreed();

#endif
#ifdef JLIB
	jv_check_mem();
#endif
}

int main( int argc, char* argv[] ) {
	time_t start;
	time_t shouldend;
	int i = 0;
	int j = 0;

	/* Initialize ALUT. */
	alutInit(&argc, argv);

	fixup_function_pointers();

	if(argc == 1) {
		init(WAVEFILE);
	} else {
		init(argv[1]);
	}

	for(i = 0; i < 8; i++) {
		fprintf(stderr, "i = %d\n", i);

		for(j = 0; j < NUMSOURCES; j++) {
			alGenSources(1, &moving_sources[j] );
			alIsSource(moving_sources[j]);
			alDeleteSources(1, &moving_sources[j] );
		}

	}

	cleanup();

	return 0;
}
