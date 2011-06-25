#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <stdio.h>


#define FLOAT_VAL        0.65

int main(void) {
	ALCdevice *dev;
	void *context_id = NULL;
	ALfloat pregain  = FLOAT_VAL;
	ALfloat postgain = 0.0;
	ALuint sid;

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	context_id = alcCreateContext( dev, NULL );
	if( context_id == NULL ) {
		alcCloseDevice( dev );

		return 1;
	}

	alcMakeContextCurrent( context_id );

	alListenerf(AL_GAIN, pregain);
	alGetListenerfv(AL_GAIN, &postgain);

	if(postgain != pregain) {
		fprintf(stderr, "Listener GAIN f***ed up: %f vs %f\n",
			pregain, postgain);
	}

	alGenSources(1, &sid);

	alSourcef(sid, AL_GAIN, pregain);
	alGetSourcefv(sid, AL_GAIN, &postgain);

	if(postgain != pregain) {
		fprintf(stderr, "Source GAIN f***ed up: %f vs %f\n",
			pregain, postgain);
	}

	alListenerf(AL_GAIN_LINEAR_LOKI, pregain);
	alGetListenerfv(AL_GAIN_LINEAR_LOKI, &postgain);

	if(postgain != pregain) {
		fprintf(stderr, "Listener GAIN_LINEAR f***ed up: %f vs %f\n",
			pregain, postgain);
	}

	alSourcef(sid, AL_GAIN_LINEAR_LOKI, pregain);
	alGetSourcefv(sid, AL_GAIN_LINEAR_LOKI, &postgain);

	if(postgain != pregain) {
		fprintf(stderr, "Source GAIN f***ed up: %f vs %f\n",
			pregain, postgain);
	}

	fprintf(stderr, "All tests okay\n" );

	fflush( stderr );

	alcMakeContextCurrent( NULL );

	alcCloseDevice( dev );

	return 0;
}
