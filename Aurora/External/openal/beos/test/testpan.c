/* testpan.c - test 1-D panning in the XZ plane (assumes right-hand system) */
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
	kNumSources = 1,

	kSpeakerAngle = 45
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
	ALfloat t, pan;


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


	/* play the sources, pan left, right, center, left/right, right/left and stop all */
	printf("Play\n");
	for (i = 0; i < kNumSources; i++)
		alSourcePlay(source[i]);

	printf("Left");
	for (t = 0.0f; t < 1.0f; t += 0.01f) {

		listenerOrientation[0] = cos(2.0 * M_PI * t);
		listenerOrientation[2] = sin(2.0 * M_PI * t);

		sourcePosition[0] = listenerPosition[0] + cos(2.0 * M_PI * (t - kSpeakerAngle / 360.0));
		sourcePosition[2] = listenerPosition[2] + sin(2.0 * M_PI * (t - kSpeakerAngle / 360.0));

		alListenerfv(AL_ORIENTATION, listenerOrientation);

		for (i = 0; i < kNumSources; i++)
			alSourcefv(source[i], AL_POSITION, sourcePosition);

		printf(".");
		fflush(stdout);

		delay(20);
	}
	delay(1000);
	printf("\n");


	printf("Right");
	for (t = 0.0f; t < 1.0f; t += 0.01f) {

		listenerOrientation[0] = cos(2.0 * M_PI * t);
		listenerOrientation[2] = sin(2.0 * M_PI * t);

		sourcePosition[0] = listenerPosition[0] + cos(2.0 * M_PI * (t + kSpeakerAngle / 360.0));
		sourcePosition[2] = listenerPosition[2] + sin(2.0 * M_PI * (t + kSpeakerAngle / 360.0));

		alListenerfv(AL_ORIENTATION, listenerOrientation);

		for (i = 0; i < kNumSources; i++)
			alSourcefv(source[i], AL_POSITION, sourcePosition);

		printf(".");
		fflush(stdout);

		delay(20);
	}
	delay(1000);
	printf("\n");


	printf("Center");
	for (t = 0.0f; t < 1.0f; t += 0.01f) {

		listenerOrientation[0] = cos(2.0 * M_PI * t);
		listenerOrientation[2] = sin(2.0 * M_PI * t);

		sourcePosition[0] = listenerPosition[0] + cos(2.0 * M_PI * (t + 0.0 / 360.0));
		sourcePosition[2] = listenerPosition[2] + sin(2.0 * M_PI * (t + 0.0 / 360.0));

		alListenerfv(AL_ORIENTATION, listenerOrientation);

		for (i = 0; i < kNumSources; i++)
			alSourcefv(source[i], AL_POSITION, sourcePosition);

		printf(".");
		fflush(stdout);

		delay(20);
	}
	delay(1000);
	printf("\n");


	printf("Left to Right");
	for (pan = -1.0f; pan < 1.0f; pan += 0.2f) {
		for (t = 0.0f; t < 1.0f; t += 0.1f) {

			listenerOrientation[0] = cos(2.0 * M_PI * t);
			listenerOrientation[2] = sin(2.0 * M_PI * t);

			sourcePosition[0] = listenerPosition[0] + cos(2.0 * M_PI * (t + kSpeakerAngle * pan / 360.0));
			sourcePosition[2] = listenerPosition[2] + sin(2.0 * M_PI * (t + kSpeakerAngle * pan / 360.0));

			alListenerfv(AL_ORIENTATION, listenerOrientation);

			for (i = 0; i < kNumSources; i++)
				alSourcefv(source[i], AL_POSITION, sourcePosition);

			printf(".");
			fflush(stdout);

			delay(20);
		}
	}
	delay(1000);
	printf("\n");


	printf("Right to Left");
	for (pan = -1.0f; pan < 1.0f; pan += 0.2f) {
		for (t = 0.0f; t < 1.0f; t += 0.1f) {

			listenerOrientation[0] = cos(2.0 * M_PI * t);
			listenerOrientation[2] = sin(2.0 * M_PI * t);

			sourcePosition[0] = listenerPosition[0] + cos(2.0 * M_PI * (t - kSpeakerAngle * pan / 360.0));
			sourcePosition[2] = listenerPosition[2] + sin(2.0 * M_PI * (t - kSpeakerAngle * pan / 360.0));

			alListenerfv(AL_ORIENTATION, listenerOrientation);

			for (i = 0; i < kNumSources; i++)
				alSourcefv(source[i], AL_POSITION, sourcePosition);

			printf(".");
			fflush(stdout);

			delay(20);
		}
	}
	delay(1000);
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
