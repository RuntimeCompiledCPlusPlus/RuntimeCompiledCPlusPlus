/* testctx1.c - test multi-threading, one context per thread */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <OS.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

enum {
	kNumBuffers = 1,
	kNumSources = 1,
	kNumThreads = 4
};

static const char *kWaveFileName = "test.wav";



static void quit(const char *message)
{
	printf("ERROR: %s\n", message);
	exit(1);
}

static void print(const char *name, const char *message)
{
	char buffer[1024];
	
	strcpy(buffer, name);
	strcat(buffer, message);
	puts(buffer);
	fflush(stdout);
}


static int32 threadFunc(void *args)
{
	ALuint buffer[kNumBuffers], source[kNumSources];
	ALuint freq;
	ALenum format;
	ALvoid *data;
	ALsizei i, j, size;
	ALvoid *context;
	const char *name;

	/* listener parameters */
	ALfloat listenerOrientation[] = { 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f };
	ALfloat listenerPosition[] = { 0.0f, 0.0f, 0.0f };
	ALfloat listenerVelocity[] = { 0.0f, 0.0f, 0.0f };

	/* source parameters */
	ALfloat sourcePosition[] = { 0.0f, 0.0f, 1.0f };
	ALfloat sourceVelocity[] = { 0.0f, 0.0f, 0.0f };
	ALfloat sourcePitch = 1.0f;
	ALfloat sourceGain = 1.0f;

	/* grab the name of this thread */
	name = (const char *) args;

	print(name, "---- start ----");

	/* create context */
	print(name, "create context");
	context = alcCreateContext(22050, AL_FORMAT_STEREO16, 2048);

	/* acquire the context */
	print(name, "make current");
	alcMakeCurrent(context);

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
		alSourcei(source[i], AL_LOOPING, AL_FALSE);
	}

	/* random play */	
	for (j = 0; j < 20; j++) {
		print(name, "play");
		for (i = 0; i < kNumSources; i++) {
			alSourceStop(source[i]);
			alSourcePlay(source[i]);
		}
	
		print(name, "snooze");
		snooze(500000 + 150000 * (rand() % 5));
	}
		

	/* delete buffers and sources */
	print(name, "delete sources");
	alDeleteSources(kNumSources, source);
	print(name, "delete buffers");
	alDeleteBuffers(kNumBuffers, buffer);

	/* release the context */
	print(name, "release current");
	alcMakeCurrent(NULL);
	
	/* shutdown */
	print(name, "delete context");
	alcDeleteContext(context);

	print(name, "---- exit ----");
	return 0;
}



int main(int argc, char *argv[])
{
	thread_id thread[kNumThreads];
	status_t status;
	ALsizei i, j;
	char names[kNumThreads][kNumThreads + 5];

	print("Main: ", "---- start ----");
	
	/* initialize */
	print("Main: ", "initialize");
	alInit((ALint *) &argc, (ALubyte **) argv);
	
	
	/* spawn threads */
	for (i = 0; i < kNumThreads; i++) {
		print("Main: ", "spawn thread");
		for (j = 0; j <= i; j++)
			names[i][j] = '\t';
		names[i][i + 1] = 'A' + i;
		names[i][i + 2] = ':';
		names[i][i + 3] = ' ';
		names[i][i + 4] = 0;
		thread[i] = spawn_thread(threadFunc, names[i], B_NORMAL_PRIORITY, names[i]);
	}

	/* resume the threads */
	for (i = 0; i < kNumThreads; i++) {
		print("Main: ", "resume thread");
		resume_thread(thread[i]);
	}

	/* wait until the threads end */
	for (i = 0; i < kNumThreads; i++) {
		print("Main: ", "wait thread");
		wait_for_thread(thread[i], &status);
		if (status != 0)
			print("Main: ", "thread failed?");
	}


	/* shutdown */
	print("Main: ", "shutdown");
	alExit();

	print("Main: ", "---- exit ----");
	return 0;
}
