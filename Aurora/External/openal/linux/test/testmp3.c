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


#define DATABUFSIZE 4098
#define MP3_FILE    "boom.mp3"
#define MP3_FUNC    "alutLoadMP3_LOKI"
#define NUMSOURCES  1

static void init(void);
static void cleanup(void);

static ALuint mp3buf; /* our buffer */
static ALuint mp3source = (ALuint ) -1;

static time_t start;

static void *context_id;

extern int errno;

/* our mp3 extension */
typedef ALboolean (mp3Loader)(ALuint, ALvoid *, ALint);
mp3Loader *alutLoadMP3p = NULL;

static void init( void ) {
	start = time(NULL);

	alGenBuffers( 1, &mp3buf);
	alGenSources( 1, &mp3source);

	alSourcei(  mp3source, AL_BUFFER, mp3buf );
	alSourcei(  mp3source, AL_LOOPING, AL_FALSE );

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
	int i = 0;
	int size;

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
		fname = MP3_FILE;
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

	fread(data, 1, size, fh);

	alutLoadMP3p = (mp3Loader *) alGetProcAddress((ALubyte *) MP3_FUNC);
	if(alutLoadMP3p == NULL) {
		free(data);

		fprintf(stderr, "Could not GetProc %s\n",
			(ALubyte *) MP3_FUNC);
		exit(-4);
	}

	if(alutLoadMP3p(mp3buf, data, size) != AL_TRUE) {
		fprintf(stderr, "alutLoadMP3p failed\n");
		exit(-2);
	}

	free(data);

	alSourcePlay( mp3source );

	while(SourceIsPlaying(mp3source) == AL_TRUE) {
		sleep(1);
	}

	cleanup();

	alcCloseDevice( dev );

	return 0;
}
