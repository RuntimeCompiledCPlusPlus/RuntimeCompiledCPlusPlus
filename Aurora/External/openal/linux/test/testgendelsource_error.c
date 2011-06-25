#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
	ALCdevice *dev;
	void *context_id;
	ALuint sid;
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

	fprintf(stderr, "alGenSources(0, &sid): should be a NOP\n");
	/* Should be a NOP */
	alGenSources(0, &sid);
	fprintf(stderr, "              result : %s\n",
		alGetString(alGetError()));

	fprintf(stderr, "alGenSources(-1, &sid): should error\n");
	/* Should be an error */
	alGenSources(-1, &sid);
	fprintf(stderr, "              result : %s\n",
		alGetString(alGetError()));

	fprintf(stderr, "alDeleteSources(0, &sid): should be a NOP\n");
	/* Should be a NOP */
	alDeleteSources(0, &sid);
	fprintf(stderr, "              result : %s\n",
		alGetString(alGetError()));

	fprintf(stderr, "alDeleteSources(-1, &sid): should error\n");
	/* Should be an error */
	alDeleteSources(-1, &sid);
	fprintf(stderr, "              result : %s\n",
		alGetString(alGetError()));

	alcDestroyContext(context_id);

	alcCloseDevice( dev );

	return 0;
}
