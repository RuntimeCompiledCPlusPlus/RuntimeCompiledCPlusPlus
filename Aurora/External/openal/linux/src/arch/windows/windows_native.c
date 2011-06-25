/* vi:set ts=8:
 *
 * windows_native.c
 *
 * functions related to the aquisition and management of the native 
 * audio on Windows.
 *
 */
#include <AL/altypes.h>

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <mmsystem.h>

#include "arch/interface/interface_sound.h"
#include "arch/windows/windows_native.h"

#include "al_main.h"
#include "al_debug.h"
#include "alc/alc_context.h"
#include "al_siteconfig.h"

#include "mutex/mutexlib.h"

#define DEF_BITSPERSAMPLE 16
#define DEF_SPEED         44100
#define DEF_FORMAT        WAVE_FORMAT_PCM
#define DEF_CHANNELS      2
#define DEF_CBSIZE        0

#define MAX_AUDIOBUFS     8
#define MAX_PCMDATA       (DEF_SPEED * DEF_CHANNELS)
#define EMAIL_ADDRESS     "tsaotsao@lokigames.com"

static MutexID mutex;

static struct {
	WAVEHDR whdrs[MAX_AUDIOBUFS];
	int index;
} audiobufs;

static struct {
        HWAVEOUT hwo;
	WAVEFORMATEX pwfx;
} WinAudioHandle;

static char pcmdata[MAX_PCMDATA];
static int scalesize = 0;


static void CALLBACK WinFillAudio(UNUSED(HWAVEOUT hwo),
	UNUSED(UINT uMsg),
	UNUSED(DWORD dwInstance),
	UNUSED(DWORD dwParam1),
	UNUSED(DWORD dwParam2)) {
	
        /* Only service "buffer done playing" messages */
        if( uMsg == WOM_DONE ) {
		mlUnlockMutex(mutex);
	}

	return;
}


static const char *implement_me(const char *fn) {
	static char retval[2048];

	sprintf(retval,
	"%s is not implemented under Windows.  Please contact %s for\n"
	"information on how you can help get %s implemented on Windows.\n",
	fn, EMAIL_ADDRESS, fn);

	return retval;
}

void *grab_read_native(void) {
	return NULL;
}

void *grab_write_native(void) {
	MMRESULT err;
	LPWAVEFORMATEX pwfx = &WinAudioHandle.pwfx;
	int i;

	audiobufs.index = 0;
	mutex = mlCreateMutex();
	
	for(i = 0; i < MAX_AUDIOBUFS; i++) {
		audiobufs.whdrs[i].lpData  = NULL;
		audiobufs.whdrs[i].dwBufferLength = 0;
		audiobufs.whdrs[i].dwFlags = WHDR_DONE;
	}

	_alBlitBuffer = native_blitbuffer;
	
	memset(pwfx, 0, sizeof *pwfx);

	pwfx->wFormatTag      = DEF_FORMAT;
	pwfx->nChannels       = DEF_CHANNELS;
	pwfx->wBitsPerSample  = DEF_BITSPERSAMPLE;
	pwfx->nBlockAlign     = DEF_CHANNELS * (pwfx->wBitsPerSample / 8);
	pwfx->nSamplesPerSec  = DEF_SPEED;
	pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;
 	pwfx->cbSize          = DEF_CBSIZE;

	err = waveOutOpen(&WinAudioHandle.hwo, WAVE_MAPPER, pwfx,
		(DWORD) WinFillAudio,
		0,
		CALLBACK_FUNCTION);

	if(err == MMSYSERR_NOERROR) {
		fprintf(stderr, "got Windows native audio\n");
		
		return &WinAudioHandle.hwo;
	}

	fprintf(stderr, "Could not open Windows native audio: %d\n",
		err);

	return NULL;
}

void native_blitbuffer(void *handle, void *dataptr, int bytes_to_write) {
	WAVEHDR *whdr;
	MMRESULT err;
        HWAVEOUT hwo;
	char *bufptr;
	char errmsg[256];

	if(handle == NULL) {
		return;
	}

	hwo = WinAudioHandle.hwo;

	mlLockMutex(mutex);

	whdr   = &audiobufs.whdrs[audiobufs.index];

	do {
		/* busy wait, ouch */
		_alMicroSleep(0);
	} while((whdr->dwFlags & WHDR_DONE) == 0);

	bufptr = pcmdata + audiobufs.index * scalesize;
	memcpy(bufptr, dataptr, bytes_to_write);

	audiobufs.index = (audiobufs.index + 1) % MAX_AUDIOBUFS;

	err = waveOutWrite(hwo, whdr, sizeof *whdr);
	if(err != MMSYSERR_NOERROR) {
		waveOutGetErrorText(err, errmsg, 256);
		fprintf(stderr, "native_blitbuffer: write error %s\n",
			errmsg);
	}

	return;
}

/*
 * FIXME: unprepare whdrs
 */
void release_native(void *handle) {
	WAVEHDR *whdr;
	HWAVEOUT hwo;
	int i;

	mlDestroyMutex(mutex);
	mutex = NULL;

	if(handle == NULL) {
		return;
	}

	hwo = WinAudioHandle.hwo;
	whdr = &audiobufs.whdrs[audiobufs.index];

	for(i = 0; i < MAX_AUDIOBUFS; i++) {
		/* not first time */
		waveOutUnprepareHeader(hwo, whdr, sizeof *whdr);
	}

	waveOutClose(hwo);

	return;
}

int set_nativechannel(UNUSED(void *handle),
		   UNUSED(ALCenum channel),
		   UNUSED(float volume)) {
	fprintf(stderr,  implement_me("set_nativechannel"));

	return 0;
}

void pause_nativedevice(UNUSED(void *handle)) {
	fprintf(stderr,  implement_me("pause_nativedevice"));

	return;
}

void resume_nativedevice(UNUSED(void *handle)) {
	fprintf(stderr,  implement_me("resume_nativedevice"));

	return;
}

float get_nativechannel(UNUSED(void *handle), UNUSED(ALCenum channel)) {
	fprintf(stderr,  implement_me("get_nativechannel"));

	return 0.0;
}

ALsizei capture_nativedevice(UNUSED(void *handle),
			  UNUSED(void *capture_buffer),
			  UNUSED(int bufsiz)) {
	return 0; /* unimplemented */
}


ALboolean set_write_native(UNUSED(void *handle),
		     UNUSED(unsigned int *bufsiz),
		     UNUSED(ALenum *fmt),
		     UNUSED(unsigned int *speed)) {
	WAVEHDR *whdr;
	MMRESULT err;
	LPWAVEFORMATEX pwfx = &WinAudioHandle.pwfx;
	int i;
	char *bufptr;
        HWAVEOUT hwo;
	ALuint channels = _al_ALCHANNELS(*fmt);

	/* close previous instance */
	waveOutClose(WinAudioHandle.hwo);
	
	memset(pwfx, 0, sizeof *pwfx);

	pwfx->wFormatTag      = DEF_FORMAT;
	pwfx->nChannels       = channels;
	pwfx->wBitsPerSample  = _al_formatbits(*fmt);
	pwfx->nBlockAlign     = pwfx->nChannels * (pwfx->wBitsPerSample / 8);
	pwfx->nSamplesPerSec  = *speed;
	pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;
 	pwfx->cbSize          = DEF_CBSIZE;

	fprintf(stderr, "chans %d bps %d bal %d sps %d abs %d cbs %d\n",
		pwfx->nChannels,
		pwfx->wBitsPerSample,
		pwfx->nBlockAlign,
		(int) pwfx->nSamplesPerSec,
		(int) pwfx->nAvgBytesPerSec,
		(int) pwfx->cbSize);

	err = waveOutOpen(&WinAudioHandle.hwo, WAVE_MAPPER, pwfx,
		(DWORD) WinFillAudio,
		0,
		CALLBACK_FUNCTION);

	if(err != MMSYSERR_NOERROR) {
		fprintf(stderr, "Could not setfmt: %d\n", err);
		return AL_FALSE;
	}

	fprintf(stderr, "setfmt okay\n");

	hwo = WinAudioHandle.hwo;

	scalesize = *bufsiz;

	for(i = 0; i < MAX_AUDIOBUFS; i++) {
		whdr   = &audiobufs.whdrs[i];
		bufptr = pcmdata + i * scalesize;

		whdr->lpData  = (LPSTR) bufptr;
		whdr->dwBufferLength = *bufsiz;
		whdr->dwFlags = WHDR_DONE;

		err = waveOutPrepareHeader(hwo, whdr, sizeof *whdr);

		if(err != MMSYSERR_NOERROR) {
			fprintf(stderr, "Could not prepare header: %d\n", err);
			
			return AL_FALSE;
		}
	}

	return AL_TRUE;
}

ALboolean set_read_native(UNUSED(void *handle),
		     UNUSED(unsigned int *bufsiz),
		     UNUSED(ALenum *fmt),
		     UNUSED(unsigned int *speed)) {
	return AL_FALSE;
}
