/* testgain.c - test the source gain */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <socket.h>
#include <AL/al.h>
#include <AL/alut.h>

enum {
	kNumBuffers = 1,
	kNumSources = 1
};

static const char *kWaveFileName = "test.wav";


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
	ALsizei i, size;
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

	/* test gain range [0, inf[ */
	printf("Test range\n");
	for (sourceGain = -1.0f; sourceGain < 2.0f; sourceGain += 0.1f) {
		for (i = 0; i < kNumSources; i++) {
			alSourcef(source[i], AL_GAIN, sourceGain);
			if (alGetError() != AL_NO_ERROR) {
				if (sourceGain >= 0.0f)
					quit("Gain change failed");
			}
			else {
				if (!(sourceGain >= 0.0f))
					quit("Gain out of range accepted");
			}
		}
	}


	/* start the sources, fade in, fade out and stop all */
	printf("Play\n");
	for (i = 0; i < kNumSources; i++) {
		alSourcePlay(source[i]);
	}

	printf("Fade In ");
	for (t = 0.0f; t < 1.0f; t += 0.02) {
		sourceGain = t;

		for (i = 0; i < kNumSources; i++)
			alSourcef(source[i], AL_GAIN, sourceGain);

		printf(".");
		fflush(stdout);

		delay(20);
	}
	delay(2000);
	printf("\n");


	printf("Fade Out ");
	for (t = 0.0f; t < 1.0f; t += 0.02) {
		sourceGain = 1 - t;

		for (i = 0; i < kNumSources; i++)
			alSourcef(source[i], AL_GAIN, sourceGain);

		printf(".");
		fflush(stdout);

		delay(20);
	}
	delay(2000);
	printf("\n");


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
