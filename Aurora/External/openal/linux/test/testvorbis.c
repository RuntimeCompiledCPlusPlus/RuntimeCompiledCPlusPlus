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


#define DATABUFSIZE 4096
#define VORBIS_FILE    "boom.ogg"
#define VORBIS_FUNC    "alutLoadVorbis_LOKI"
#define NUMSOURCES  1

static void init(void);
static void cleanup(void);

static ALuint vorbbuf; /* our buffer */
static ALuint vorbsource = (ALuint ) -1;

static time_t start;

static void *context_id;

extern int errno;

/* our vorbis extension */
typedef ALboolean (vorbisLoader)(ALuint, ALvoid *, ALint);
vorbisLoader *alutLoadVorbisp = NULL;

static void init( void ) {
	start = time(NULL);

	alGenBuffers( 1, &vorbbuf);
	alGenSources( 1, &vorbsource);

	alSourcei(  vorbsource, AL_BUFFER, vorbbuf );
	alSourcei(  vorbsource, AL_LOOPING, AL_TRUE );

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
	FILE *fh;
	struct stat sbuf;
	void *data;
	char *fname;
	int size;
	int i = 0;

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	/* Initialize ALUT. */
	context_id = alcCreateContext( dev, NULL );
	if(context_id == NULL) {
		alcCloseDevice( dev );

		return 1;
	}

	alcMakeContextCurrent( context_id );

	fixup_function_pointers();

	init( );

	if(argc == 1) {
		fname = VORBIS_FILE;
	} else {
		fname = argv[1];
	}

	if(stat(fname, &sbuf) == -1) {
		perror(fname);
		return errno;
	}

	size = sbuf.st_size;
	data = malloc(size);
	if(data == NULL) {
		exit(1);
	}

	fh = fopen(fname, "rb");
	if(fh == NULL) {
		fprintf(stderr, "Could not open %s\n", fname);

		free(data);

		exit(1);
	}

	fread(data, size, 1, fh);

	alutLoadVorbisp = (vorbisLoader *) alGetProcAddress((ALubyte *) VORBIS_FUNC);
	if(alutLoadVorbisp == NULL) {
		free(data);

		fprintf(stderr, "Could not GetProc %s\n",
			(ALubyte *) VORBIS_FUNC);
		exit(-4);
	}

	if(alutLoadVorbisp(vorbbuf, data, size) != AL_TRUE) {
		fprintf(stderr, "alutLoadVorbis failed\n");
		exit(-2);
	}

	free(data);

	alSourcePlay( vorbsource );

	while(SourceIsPlaying(vorbsource) == AL_TRUE) {
		sleep(1);
	}

	cleanup();

	alcCloseDevice( dev );

	return 0;
}
