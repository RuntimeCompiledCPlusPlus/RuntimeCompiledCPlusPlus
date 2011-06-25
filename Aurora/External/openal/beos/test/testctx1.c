/* testctx1.c - share a single context between two threads */

#include <stdio.h>
#include <stdlib.h>
#include <OS.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

enum {
	kNumBuffers = 1,
	kNumSources = 1
};

static const char *kWaveFileName = "test.wav";

static ALvoid *context = NULL;
static ALuint buffer[kNumBuffers], source[kNumSources];


static void quit(const char *message)
{
	printf("ERROR: %s\n", message);
	exit(1);
}

static void print(const char *message)
{
	puts(message);
	fflush(stdout);
}

static int32 threadFunc1(void *data)
{
	ALsizei i, j;

	for (j = 0; j < 20; j++) {
		print("\tA: make current");
		alcMakeCurrent(context);
		
		print("\tA: play");
		for (i = 0; i < kNumSources; i++) {
			alSourceStop(source[i]);
			alSourcePlay(source[i]);
			alSourcef(source[i], AL_PITCH, 1.0f);
		}

		print("\tA: release current");
		alcMakeCurrent(NULL);
	
		print("\tA: snooze");
		snooze(100000 * ((rand() % 10) + 1));
	}
	
	print("\tA: exit");
	return 0;
}

static int32 threadFunc2(void *data)
{
	ALsizei i, j;
	
	for (j = 0; j < 20; j++) {
		print("\t\tB: make current");
		alcMakeCurrent(context);
		
		print("\t\tB: play");
		for (i = 0; i < kNumSources; i++) {
			alSourceStop(source[i]);
			alSourcePlay(source[i]);
			alSourcef(source[i], AL_PITCH, 0.5f);
		}

		print("\t\tB: release current");
		alcMakeCurrent(NULL);
		
		print("\t\tB: snooze");
		snooze(100000 * ((rand() % 10) + 1));
	}
	
	print("\t\tB: exit");
	return 0;
}


int main(int argc, char *argv[])
{
	ALuint freq;
	ALenum format;
	ALvoid *data;
	ALsizei i, size;
	thread_id thread1, thread2;
	status_t status;
	
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
	print("Main: initialize");
	alInit((ALint *) &argc, (ALubyte **) argv);

	/* create context */
	print("Main: create context");
	context = alcCreateContext(22050, AL_FORMAT_STEREO16, 2048);

	/* lock the context */
	print("Main: make current");
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
		alSourcei(source[i], AL_LOOPING, AL_TRUE);
	}

	/* start the sources */
	print("Main: play");
	for (i = 0; i < kNumSources; i++)
		alSourcePlay(source[i]);
	
	/* release the context */
	print("Main: release current");
	alcMakeCurrent(NULL);
	

	/* spawn two threads */
	print("Main: spawn thread 1");
	thread1 = spawn_thread(threadFunc1, "thread 1", B_NORMAL_PRIORITY, NULL);
	print("Main: spawn thread 2");
	thread2 = spawn_thread(threadFunc2, "thread 2", B_NORMAL_PRIORITY, NULL);

	/* resume the threads */	
	print("Main: resume thread 1");
	resume_thread(thread1);
	print("Main: resume thread 2");
	resume_thread(thread2);

	/* acquire context, snooze and release context */
	print("Main: make current");
	alcMakeCurrent(context);
	
	print("Main: snooze...");
	snooze(500000);

	print("Main: release current");
	alcMakeCurrent(NULL);
	

	/* wait until the threads end */
	print("Main: wait thread 1");
	wait_for_thread(thread1, &status);
	if (status != 0)
		print("Main: thread 1 failed?");
	print("Main: wait thread 2");
	wait_for_thread(thread2, &status);
	if (status != 0)
		print("Main: thread 2 failed?");


	/* acquire the context */
	print("Main: make current");
	alcMakeCurrent(context);

	/* delete buffers and sources */
	print("Main: delete sources");
	alDeleteSources(kNumSources, source);
	print("Main: delete buffers");
	alDeleteBuffers(kNumBuffers, buffer);

	/* release the context */
	print("Main: release current");
	alcMakeCurrent(NULL);
	
	
	/* shutdown */
	print("Main: delete context");
	alcDeleteContext(context);

	/* bye */
	print("Main: bye");
	alExit();
	
	return 0;
}
