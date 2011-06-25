#include "testlib.h"

#include <AL/al.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define GP(x)          alGetProcAddress((const ALubyte *) x)

#ifdef _WIN32
#include <windows.h>

void micro_sleep(unsigned int n) {
	Sleep(n / 1000);

	return;
}

#else

void micro_sleep(unsigned int n) {
	struct timeval tv = { 0, 0 };

	tv.tv_usec = n;

	select(0, NULL, NULL, NULL, &tv);

	return;
}


#endif /* _WIN32 */


void fixup_function_pointers(void) {
	talcGetAudioChannel = (ALfloat (*)(ALuint channel))
				GP("alcGetAudioChannel_LOKI");
	talcSetAudioChannel = (void (*)(ALuint channel, ALfloat volume))
				GP("alcSetAudioChannel_LOKI");

	talMute   = (void (*)(ALvoid)) GP("alMute_LOKI");
	talUnMute = (void (*)(ALvoid)) GP("alUnMute_LOKI");

	talReverbScale = (void (*)(ALuint sid, ALfloat param))
		GP("alReverbScale_LOKI");
	talReverbDelay = (void (*)(ALuint sid, ALfloat param))
		GP("alReverbDelay_LOKI");

	talBombOnError = (void (*)(void))
		GP("alBombOnError_LOKI");

	if(talBombOnError == NULL) {
		fprintf(stderr,
			"Could not GetProcAddress alBombOnError_LOKI\n");
		exit(1);
	}
	
	talBufferi = (void (*)(ALuint, ALenum, ALint ))
		GP("alBufferi_LOKI");

	if(talBufferi == NULL) {
		fprintf(stderr,
			"Could not GetProcAddress alBufferi_LOKI\n");
		exit(1);
	}

	alCaptureInit    = (ALboolean (*)( ALenum, ALuint, ALsizei )) GP("alCaptureInit_EXT");
	alCaptureDestroy = (ALboolean (*)( ALvoid )) GP("alCaptureDestroy_EXT");
	alCaptureStart   = (ALboolean (*)( ALvoid )) GP("alCaptureStart_EXT");
	alCaptureStop    = (ALboolean (*)( ALvoid )) GP("alCaptureStop_EXT");
	alCaptureGetData = (ALsizei (*)( ALvoid*, ALsizei, ALenum, ALuint )) GP("alCaptureGetData_EXT");

	talBufferAppendData = (ALuint (*)(ALuint, ALenum, ALvoid *, ALint, ALint)) GP("alBufferAppendData_LOKI");
	talBufferAppendWriteData = (ALuint (*)(ALuint, ALenum, ALvoid *, ALint, ALint, ALenum)) GP("alBufferAppendWriteData_LOKI");

	talGenStreamingBuffers = (void (*)(ALsizei n, ALuint *bids )) GP("alGenStreamingBuffers_LOKI");
	if( talGenStreamingBuffers == NULL ) {
		fprintf( stderr, "Could not GP alGenStreamingBuffers_LOKI\n");
		exit(1);
	}
	
	talutLoadRAW_ADPCMData = (ALboolean (*)(ALuint bid,ALvoid *data, ALuint size, ALuint freq,ALenum format)) GP("alutLoadRAW_ADPCMData_LOKI");
	if( talutLoadRAW_ADPCMData == NULL ) {
		fprintf( stderr, "Could not GP alutLoadRAW_ADPCMData_LOKI\n");
		exit(1);
	}

	talutLoadIMA_ADPCMData = (ALboolean (*)(ALuint bid,ALvoid *data, ALuint size, ALuint freq,ALenum format)) GP("alutLoadIMA_ADPCMData_LOKI");
	if( talutLoadIMA_ADPCMData == NULL ) {
		fprintf( stderr, "Could not GP alutLoadIMA_ADPCMData_LOKI\n");
		exit(1);
	}

	talutLoadMS_ADPCMData = (ALboolean (*)(ALuint bid,ALvoid *data, ALuint size, ALuint freq,ALenum format)) GP("alutLoadMS_ADPCMData_LOKI");
	if( talutLoadMS_ADPCMData == NULL ) {
		fprintf( stderr, "Could not GP alutLoadMS_ADPCMData_LOKI\n");
		exit(1);
	}


	return;
}

ALboolean SourceIsPlaying(ALuint sid) {
	ALint state;

	if(alIsSource(sid) == AL_FALSE) {
		return AL_FALSE;
	}

	state = AL_INITIAL;
	alGetSourceiv(sid, AL_SOURCE_STATE, &state);
	switch(state) {
		case AL_PLAYING:
		case AL_PAUSED:
		  return AL_TRUE;
		default:
		  break;
	}

	return AL_FALSE;
}
