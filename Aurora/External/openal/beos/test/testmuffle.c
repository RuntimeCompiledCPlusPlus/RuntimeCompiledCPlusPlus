/* testmuffle.c - test front/back lowpass filtering in the XZ plane */
/*

    (Y)
      \
       \
		+----------> (X)
		| \ a |
		|   \/
		|     \ (cos(A), y, sin(A))
		|
		|
		(Z)

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <socket.h>
#include <math.h>
#include <AL/al.h>
#include <AL/alut.h>

enum {
	kNumBuffers = 1,
	kNumSources = 1
};

static const char *kWaveFileName = "test.wav";

static const ALfloat kFrontAngle = 0.0f;


static void quit(const char *message)
{
	printf("ERROR: %s\n", message);
	exit(1);
}

static void delay(int msecs)
{
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = msecs * 1000;

	select(0, NULL, NULL, NULL, &tv);
}


int main(int argc, char *argv[])
{
	ALuint buffer[kNumBuffers], source[kNumSources];
	ALuint freq;
	ALenum format;
	ALvoid *data;
	ALsizei i, j, size;
	ALfloat t;
	
	/* listener parameters */
	ALfloat listenerOrientation[] = { 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f };
	ALfloat listenerPosition[] = { 0.0f, 0.0f, 0.0f };
	ALfloat listenerVelocity[] = { 0.0f, 0.0f, 0.0f };

	/* source parameters */
	ALfloat sourcePosition[] = { 0.0f, 0.0f, 1.0f };
	ALfloat sourceVelocity[] = { 0.0f, 0.0f, 0.0f };
	ALfloat sourcePitch = 1.0f;
	ALfloat sourceGain = 1.0f;



	/* initialize */
	alutInit(&argc, argv);

	/* create buffers and sources */
	if (alGenBuffers(kNumBuffers, buffer) != kNumBuffers)
		quit("Can't create buffers");

	if (alGenSources(kNumSources, source) != kNumSources)
		quit("Can't create sources");

	/* load buffers with data */
	alutLoadWAV(kWaveFileName, &format, &data, &size, &freq);
	for (i = 0; i < kNumBuffers; i++) {
		alBufferData(buffer[i], format, data, size, freq);
	}
	free(data);


	/* initialize listener */
	alListenerfv(AL_POSITION, listenerPosition);
	alListenerfv(AL_VELOCITY, listenerVelocity);
	alListenerfv(AL_ORIENTATION, listenerOrientation);

	/* initialize sources */
	for (i = 0; i < kNumSources; i++) {
		alSourcefv(source[i], AL_POSITION, sourcePosition);
		alSourcefv(source[i], AL_VELOCITY, sourceVelocity);

		alSourcef(source[i], AL_PITCH, sourcePitch);
		alSourcef(source[i], AL_GAIN, sourceGain);

		alSourcei(source[i], AL_BUFFER, buffer[i % kNumBuffers]);
		alSourcei(source[i], AL_LOOPING, AL_TRUE);
	}


	/* play the sources, rotate around the listener, stop all */
	printf("Play\n");
	for (i = 0; i < kNumSources; i++)
		alSourcePlay(source[i]);

	for (j = 0; j <= 8*4; j++) {
		printf("%3d azim ", (45 * j) % 360);
		
		listenerOrientation[0] = cos(2.0 * M_PI * kFrontAngle);
		listenerOrientation[2] = sin(2.0 * M_PI * kFrontAngle);
	
		for (t = 0.0f; t <= 1.0f; t += 0.02f) {
		
			sourcePosition[0] = listenerPosition[0] + cos(2.0 * M_PI * (kFrontAngle + 45 * (j + t)) / 360.0f);
			sourcePosition[2] = listenerPosition[2] + sin(2.0 * M_PI * (kFrontAngle + 45 * (j + t)) / 360.0f);
		
			alListenerfv(AL_ORIENTATION, listenerOrientation);
		
			for (i = 0; i < kNumSources; i++)
				alSourcefv(source[i], AL_POSITION, sourcePosition);

			printf(".");
			fflush(stdout);

			delay(20);
		}	
		printf("\n");
	}


	printf("Stop\n");
	for (i = 0; i < kNumSources; i++)
		alSourceStop(source[i]);


	/* delete buffers and sources */
	alDeleteSources(kNumSources, source);
	alDeleteBuffers(kNumBuffers, buffer);

	/* shutdown */
	alutExit();

	return 0;
}
