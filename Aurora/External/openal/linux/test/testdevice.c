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

int main( int argc, char* argv[] ) {
	ALCdevice *dev;
	const ALubyte *initstr = (const ALubyte *) "'( ( devices '( native null ) ) )";
	
	dev = alcOpenDevice( initstr );

	sleep( 1 );

	alcCloseDevice(  dev  );

	return 0;
}
