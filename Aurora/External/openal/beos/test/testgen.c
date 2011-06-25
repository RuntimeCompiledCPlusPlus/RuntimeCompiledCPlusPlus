/* testgen.c - test the generation of buffers and sources */

#include <stdio.h>
#include <stdlib.h>
#include <AL/al.h>
#include <AL/alut.h>

enum {
	kNumBuffers = 50,
	kNumSources = 32
};

static const char *kWaveFileName = "test.wav";


static void quit(const char *message)
{
	printf("ERROR: %s\n", message);
	exit(1);
}

int main(int argc, char *argv[])
{
	ALuint buffer[kNumBuffers], source[kNumSources];
	ALuint freq;
	ALenum format;
	ALvoid *data;
	ALsizei i, size;


	/* initialize */
	printf("Init\n");
	alutInit(&argc, argv);

	/* create buffers and sources */
	printf("Creating buffers\n");
	if (alGenBuffers(kNumBuffers, buffer) != kNumBuffers)
		quit("Can't create buffers");

	printf("Creating sources\n");
	if (alGenSources(kNumSources, source) != kNumSources)
		quit("Can't create sources");


	/* test the buffer and source IDs */
	printf("Test buffer identifiers\n");
	for (i = 0; i < kNumBuffers; i++) {
		if (!alIsBuffer(buffer[i]))
			quit("Bad buffer identifier");
		if (alIsSource(buffer[i]))
			quit("Bad buffer identifier (is source)\n");
	}

	printf("Test source identifiers\n");
	for (i = 0; i < kNumSources; i++) {
		if (!alIsSource(source[i]))
			quit("Bad source identifier");
		if (alIsBuffer(source[i]))
			quit("Bad source identifier (is buffer)");
	}


	/* load buffers with data */
	printf("Loading buffer data: ");
	alutLoadWAV(kWaveFileName, &format, &data, &size, &freq);
	for (i = 0; i < kNumBuffers; i++) {
		alBufferData(buffer[i], format, data, size, freq);
	}
	printf("%s, %d bytes, %d Hz\n",
		format == AL_FORMAT_MONO8 ? "8 bit Mono" :
		format == AL_FORMAT_MONO16 ? "16 bit Mono" :
		format == AL_FORMAT_STEREO8 ? "8 bit Stereo" :
		format == AL_FORMAT_STEREO16 ? "16 bit Stereo" : "Unknown format",
		size, freq);
	free(data);


	/* test the buffer parameters */
	printf("Test buffer parameters\n");
	for (i = 0; i < kNumBuffers; i++) {
		ALint parms[4];

		alGetBufferi(buffer[i], AL_BITS, &parms[0]);
		alGetBufferi(buffer[i], AL_CHANNELS, &parms[1]);
		alGetBufferi(buffer[i], AL_SIZE, &parms[2]);
		alGetBufferi(buffer[i], AL_FREQUENCY, &parms[3]);

		if ((parms[0] != 8 && (format == AL_FORMAT_MONO8 || format == AL_FORMAT_STEREO8)) ||
			(parms[0] != 16 && (format == AL_FORMAT_MONO16 || format == AL_FORMAT_STEREO16)))
			quit("Bad buffer parameter (AL_BITS)");

		if ((parms[1] != 1 && (format == AL_FORMAT_MONO8 || format == AL_FORMAT_MONO16)) ||
		    (parms[1] != 2 && (format == AL_FORMAT_STEREO8 || format == AL_FORMAT_STEREO16)))
			quit("Bad buffer parameter (AL_CHANNEL)");

		if (parms[2] != size)
			quit("Bad buffer parameter (AL_SIZE)");

		if (parms[3] != freq)
			quit("Bad buffer parameter (AL_FREQUENCY)");
	}

	/* delete buffers and sources */
	printf("Delete buffers\n");
	for (i = 0; i < kNumBuffers; i++) {
		alDeleteBuffers(1, &buffer[i]);
		if (alIsBuffer(buffer[i]))
			quit("Deleted buffer still valid?");
		if (alIsSource(buffer[i]))
			quit("Deleted buffer is a source?");
	}

	printf("Delete sources\n");
	for (i = 0; i < kNumSources; i++) {
		alDeleteSources(1, &source[i]);
		if (alIsSource(source[i]))
			quit("Deleted source still valid?");
		if (alIsBuffer(source[i]))
			quit("Deleted source is a buffer?");
	}

	/* test the "is a" predicates */
	printf("Testing IsA predicates\n");
	for (i = 0; i < 1000; i++) {
		if (alIsSource(rand()))
			quit("Random number is a source?");
			
		if (alIsBuffer(rand()))
			quit("Random number is a buffer?");
		
		if (alGetError() != AL_NO_ERROR)
			quit("predicate raised an error?");
	}

	/* check the error code */
	if (alGetError() != AL_NO_ERROR)
		quit("Error raised in the test?");

	/* shutdown */
	printf("Exit\n");
	alutExit();

	return 0;
}
