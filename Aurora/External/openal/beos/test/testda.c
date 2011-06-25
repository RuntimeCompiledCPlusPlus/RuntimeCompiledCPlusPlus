/* testda.c - test distance attenuation */

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

	/* start the sources, fade in, fade out and stop all */
	printf("Play\n");
	for (i = 0; i < kNumSources; i++) {
		alSourcePlay(source[i]);
	}

	printf("Distance .");
	for (t = 0.0f; t < 20.0f; t += 0.1f) {
		sourcePosition[0] = listenerPosition[0] + t * listenerOrientation[0];
		sourcePosition[2] = listenerPosition[2] + t * listenerOrientation[2];

		for (i = 0; i < kNumSources; i++)
			alSourcefv(source[i], AL_POSITION, sourcePosition);

		printf(".");
		fflush(stdout);

		delay(30);
	}
	delay(2000);
	printf("\n");

	printf("Constant Distance .");
	for (t = 0.0f; t < 40.0f; t += 0.2f) {
	
		listenerPosition[0] = 0.0f;
		listenerPosition[1] = 0.0f;
		listenerPosition[2] = t;
		
		sourcePosition[0] = listenerPosition[0];
		sourcePosition[1] = listenerPosition[1];
		sourcePosition[2] = listenerPosition[2] + 5.0f;
		
		alListenerfv(AL_POSITION, listenerPosition);
		for (i = 0; i < kNumSources; i++)
			alSourcefv(source[i], AL_POSITION, sourcePosition);
		
		printf(".");
		fflush(stdout);
		
		delay(30);
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
