/* testdoppler.c - test doppler pitch shift effect */
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


	 <---------- 100 m ---------->

		     20 m/s
	(A)   -- source -->         (B)
				^
				|
				|
			   5 m
				|
				|
				v
			listener
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

static const ALfloat kVelocity = 20.0f; 	/* 20 m/s (72 Kmh) */
static const ALfloat kSeconds = 5.0f;		/* 5 secs (100 m) */
static const ALfloat kLatency = 0.01f;		/* 10 ms */
static const ALfloat kDistance = 5.0f;		/* 5 m */
static const ALfloat kFrontAngle = 0.0f;	/* front angle (0-360) */




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
	ALfloat sourceGain = 5.0f;



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


	/* start the sources, move left/right, right/left and stop all */
	printf("Play\n");
	for (i = 0; i < kNumSources; i++) {
		alSourcePlay(source[i]);
	}

	printf("Left to Right ");
	for (t = 0.0f; t < kSeconds; t += kLatency) {
		listenerOrientation[0] = cos(2.0 * M_PI * kFrontAngle / 360.0f);
		listenerOrientation[2] = sin(2.0 * M_PI * kFrontAngle / 360.0f);

		sourceVelocity[0] = kVelocity * cos(2.0 * M_PI * (kFrontAngle + 90.0f) / 360.0f);
		sourceVelocity[2] = kVelocity * sin(2.0 * M_PI * (kFrontAngle + 90.0f) / 360.0f);

		sourcePosition[0] = listenerPosition[0] + kDistance * listenerOrientation[0] + (t - kSeconds/2) * sourceVelocity[0];
		sourcePosition[2] = listenerPosition[2] + kDistance * listenerOrientation[2] + (t - kSeconds/2) * sourceVelocity[2];

		alListenerfv(AL_ORIENTATION, listenerOrientation);

		for (i = 0; i < kNumSources; i++) {
			alSourcefv(source[i], AL_POSITION, sourcePosition);
			alSourcefv(source[i], AL_VELOCITY, sourceVelocity);
		}

		printf(".");
		fflush(stdout);

		delay((int) (1000 * kLatency));
	}
	delay(1000);
	printf("\n");


	printf("Right to Left ");
	for (t = 0.0f; t < kSeconds; t += kLatency) {
		listenerOrientation[0] = cos(2.0 * M_PI * kFrontAngle / 360.0f);
		listenerOrientation[2] = sin(2.0 * M_PI * kFrontAngle / 360.0f);

		sourceVelocity[0] = kVelocity * cos(2.0 * M_PI * (kFrontAngle - 90.0f) / 360.0f);
		sourceVelocity[2] = kVelocity * sin(2.0 * M_PI * (kFrontAngle - 90.0f) / 360.0f);

		sourcePosition[0] = listenerPosition[0] + kDistance * listenerOrientation[0] + (t - kSeconds/2) * sourceVelocity[0];
		sourcePosition[2] = listenerPosition[2] + kDistance * listenerOrientation[2] + (t - kSeconds/2) * sourceVelocity[2];

		alListenerfv(AL_ORIENTATION, listenerOrientation);

		for (i = 0; i < kNumSources; i++) {
			alSourcefv(source[i], AL_POSITION, sourcePosition);
			alSourcefv(source[i], AL_VELOCITY, sourceVelocity);
		}

		printf(".");
		fflush(stdout);

		delay((int) (1000 * kLatency));
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
