#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
	ALCdevice *dev;
	void *context_id;
	ALuint bid;
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

	fprintf(stderr, "alGenBuffers(0, &bid): should be a NOP\n");
	/* Should be a NOP */
	alGenBuffers(0, &bid);
	fprintf(stderr, "              result : %s\n",
		alGetString(alGetError()));

	fprintf(stderr, "alGenBuffers(-1, &bid): should error\n");
	/* Should be an error */
	alGenBuffers(-1, &bid);
	fprintf(stderr, "              result : %s\n",
		alGetString(alGetError()));


	fprintf(stderr, "alDeleteBuffers(0, &bid): should be a NOP\n");
	/* Should be a NOP */
	alDeleteBuffers(0, &bid);
	fprintf(stderr, "              result : %s\n",
		alGetString(alGetError()));

	fprintf(stderr, "alDeleteBuffers(-1, &bid): should error\n");
	/* Should be an error */
	alDeleteBuffers(-1, &bid);
	fprintf(stderr, "              result : %s\n",
		alGetString(alGetError()));

	alcDestroyContext(context_id);

	alcCloseDevice( dev );

	return 0;
}
