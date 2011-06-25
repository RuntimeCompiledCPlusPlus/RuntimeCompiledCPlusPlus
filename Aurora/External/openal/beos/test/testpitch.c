/* testpitch.c - test the source pitch shifting */

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
	kNumSources = 2
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


	/* test pitch range [0.5, 2.0] */
	printf("Test range\n");
	for (sourcePitch = -1.0f; sourcePitch < 3.0f; sourcePitch += 0.1f) {
		for (i = 0; i < kNumSources; i++) {
			alSourcef(source[i], AL_PITCH, sourcePitch);
			if (alGetError() != AL_NO_ERROR) {
				if (sourcePitch >= 0.5f && sourcePitch <= 2.0f)
					quit("Pitch change failed");
			}
			else {
				if (!(sourcePitch >= 0.5f && sourcePitch <= 2.0f))
					quit("Pitch out of range accepted");
			}
		}
	}

	/* start the sources, pitch shift up, down and stop all */
	printf("Play\n");
	for (i = 0; i < kNumSources; i++) {
		alSourcePlay(source[i]);
	}

	printf("Pitch Up ");
	for (sourcePitch = 0.5f; sourcePitch < 2.0f; sourcePitch += 0.005f) {

		for (i = 0; i < kNumSources; i++)
			alSourcef(source[i], AL_PITCH, sourcePitch);

		printf(".");
		fflush(stdout);

		delay(20);
	}
	delay(2000);
	printf("\n");

	printf("Pitch Down ");
	for (sourcePitch = 2.0f; sourcePitch > 0.5f; sourcePitch -= 0.005f) {

		for (i = 0; i < kNumSources; i++)
			alSourcef(source[i], AL_PITCH, sourcePitch);

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
