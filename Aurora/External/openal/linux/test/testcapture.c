#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define NUMCAPTURES   3
#define WAVEFILE      "boom.wav"
#define FREQ          22050
#define SAMPLES       (5 * FREQ)

static void cleanup(void);

static ALuint caps[NUMCAPTURES] = { 0 };

static void *context_id;

void cleanup(void) {
	alcDestroyContext(context_id);

#ifdef JLIB
	jv_check_mem();
#endif
}

int main( int argc, char* argv[] ) {
	ALCdevice *dev;
	ALuint cpid;
	ALuint sid = 0;
	ALuint cbid;
	ALuint sbid;
	ALuint retval;
	ALvoid *buffer = NULL;
	FILE *fh;

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	/* Initialize openal. */
	context_id = alcCreateContext( dev, NULL);
	if(context_id == NULL) {
		alcCloseDevice( dev );

		return 1;
	}

	alcMakeContextCurrent( context_id );

	fixup_function_pointers();

	if ( ! alCaptureInit(AL_FORMAT_MONO16, FREQ, 1024) ) {
		alcCloseDevice( dev );

		printf("Unable to initialize capture\n");
		return 1;
	}

	buffer = malloc(SAMPLES * 2);

	/*                    test 1              */
	fprintf(stderr, "test1\n");

	fprintf(stderr, "recording...");
	alCaptureStart();
	retval = 0;
	while ( retval < (SAMPLES*2) ) {
		retval += alCaptureGetData(&((char *)buffer)[retval],
		                           (SAMPLES*2)-retval,
		                           AL_FORMAT_MONO16, FREQ);
	}
	alCaptureStop();
	fprintf(stderr, "\n");

	fh = fopen("outpcm.pcm", "wb");
	if(fh != NULL) {
		fwrite(buffer, retval, 1, fh);
		fclose(fh);
	}

	fprintf(stderr, "Sleeping for 5 seconds\n");
	sleep(5);

	/*                    test 2              */
	fprintf(stderr, "test2\n");

	fprintf(stderr, "recording...");
	alCaptureStart();
	retval = 0;
	while ( retval < (SAMPLES*2) ) {
		retval += alCaptureGetData(&((char *)buffer)[retval],
		                           (SAMPLES*2)-retval,
		                           AL_FORMAT_MONO16, FREQ);
	}
	alCaptureStop();
	fprintf(stderr, "\n");

	fprintf(stderr, "playback...");
	alGenSources(1, &sid);
	alGenBuffers(1, &sbid);
	alSourcei(sid, AL_BUFFER, sbid);

	alBufferData(sbid, AL_FORMAT_MONO16, buffer, retval, FREQ);

	alSourcePlay(sid);

	while(SourceIsPlaying(sid) == AL_TRUE) {
		sleep(1);
	}

	fprintf(stderr, "\n");

	free(buffer);

	cleanup();

	alcCloseDevice( dev );

	return 0;
}
