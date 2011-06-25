#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <stdlib.h>
#include <stdio.h>

int main( int argc, char* argv[] )
{
	void *context_id = NULL;
	ALCdevice *dev = NULL;

	int attrlist[] = { ALC_FREQUENCY, 44100,
			   0 };

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		exit( 1 );
	}

	context_id = alcCreateContext( dev, attrlist );
	if( context_id == NULL ) {
		alcCloseDevice( dev );

		exit( 1 );
	}

	printf( "AL_VENDOR: %s\n", alGetString( AL_VENDOR ) );
	printf( "AL_VERSION: %s\n", alGetString( AL_VERSION ) );
	printf( "AL_RENDERER: %s\n", alGetString( AL_RENDERER ) );
	printf( "AL_EXTENSIONS: %s\n", alGetString( AL_EXTENSIONS ) );

	if( alIsExtensionPresent( (const ALubyte*) "AL_LOKI_attenuation_scale" ) ) {
		printf( "Found AL_LOKI_attenuation_scale\n" );
	}

	alcDestroyContext( context_id );

	return 0;
}
