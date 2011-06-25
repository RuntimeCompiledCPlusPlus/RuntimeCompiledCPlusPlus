/* darwin_native.c
 *
 * Mac OS X backend for OpenAL
 * only PowerPC target on MacOS X is supported (Darwin is not supported alone)
 *
 * file originally created on jan 2000 by
 * guillaume borios (gborios@free.fr) and florent boudet (flobo@iname.com)
 * to help the PineApple project (http://ios.free.fr) run on MOSX
 *
 * Version : Alpha 2
 */
 
#include "al_siteconfig.h"

#include <CoreAudio/AudioHardware.h>

#include "arch/interface/interface_sound.h"
#include "arch/darwin/darwin_native.h"

#include <fcntl.h>
#include <stdlib.h>

#include "al_main.h"
#include "al_debug.h"
#include "alc/alc_context.h"

#define maxBuffer 20 /*number of buffers to use (1 buffer = 23.22 ms) */
/* Adding buffers improves response to temporary heavy CPU loads but increases latency... */

typedef struct {
    ALboolean	bufferIsEmpty;
    Ptr		startOfDataPtr;
} archBuffer;

typedef struct {
    AudioDeviceID	deviceW;			/* the default device */
    UInt32		deviceWBufferSize;	/* Buffer size of the audio device */
    AudioStreamBasicDescription	deviceFormat;	/* format of the default device */
    short		bufferToFill;		/* Id of the next buffer to fill */
    short		bufferToRead;		/* Id of the next buffer to read */
    archBuffer		buffer[maxBuffer];	/* Buffers array */
} globalVars, *globalPtr;

/************************************** GLOBALS *********************************/

globalVars	libGlobals; /* my globals */

unsigned int	alWriteFormat;	/* format of data from AL*/
unsigned int	alWriteSpeed;	/* speed of data from AL*/

/************************************* PROTOTYPES *********************************/

void implement_me(const char *fn);
OSStatus deviceFillingProc (AudioDeviceID  inDevice, const AudioTimeStamp*  inNow, const void*  inInputData, const AudioTimeStamp*  inInputTime, void*  outOutputData, const AudioTimeStamp* inOutputTime, void* globals);

/************************************** UTILITIES *********************************/

void implement_me(const char *fn)
{
    fprintf(stderr,"[darwin_native.c] : %s is not implemented.\nPlease contact gborios@freesbee.fr for information or help.\n", fn);
}

/*********************************** OS callback proc *****************************/

OSStatus deviceFillingProc (AudioDeviceID  inDevice, const AudioTimeStamp*  inNow, const void*  inInputData, const AudioTimeStamp*  inInputTime, void*  outOutputData, const AudioTimeStamp* inOutputTime, void* globals)
{    
    globalPtr		theGlobals;
    
   theGlobals = globals;
    
    if (theGlobals->buffer[theGlobals->bufferToRead].bufferIsEmpty == AL_FALSE)
    {
	BlockMoveData(theGlobals->buffer[theGlobals->bufferToRead].startOfDataPtr, outOutputData, theGlobals->deviceWBufferSize);
	theGlobals->buffer[theGlobals->bufferToRead].bufferIsEmpty = AL_TRUE;
	theGlobals->bufferToRead++;
	if (theGlobals->bufferToRead  == maxBuffer) theGlobals->bufferToRead = 0;
    }
    
    return (noErr);     
}


/************************************** INTERFACE *********************************/

void *grab_read_native(void)
{
    implement_me("void *grab_read_native()");
    return NULL;
}

void *grab_write_native(void)
{
    OSStatus	error = noErr;
    UInt32	count;
   
    libGlobals.deviceW = kAudioDeviceUnknown;

    count = sizeof(libGlobals.deviceW);  /* Look for audio device */
    error = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &count, (void *) &libGlobals.deviceW);
    if (error != noErr) goto Crash;

    count = sizeof(libGlobals.deviceWBufferSize); /* Determines buffer size */
    error = AudioDeviceGetProperty(libGlobals.deviceW, 0, false, kAudioDevicePropertyBufferSize, &count, &libGlobals.deviceWBufferSize);
    if (error != noErr) goto Crash;


    count = sizeof(libGlobals.deviceFormat); /* Determines the device format */
    error = AudioDeviceGetProperty(libGlobals.deviceW, 0, false, kAudioDevicePropertyStreamFormat, &count, &libGlobals.deviceFormat);
    if (error != noErr) goto Crash;

    blitbuffer = native_blitbuffer; /* Defines the blitbuffer function */
    
    error = AudioDeviceAddIOProc(libGlobals.deviceW, deviceFillingProc, (void *) &libGlobals);	/* Creates the callback proc */
    if (error != noErr) goto Crash;
    
    return &libGlobals.deviceW;

Crash :
    printf("An error occured during void *grab_write_native()\n");
    libGlobals.deviceW = NULL;
    return NULL;
}


ALboolean set_write_native(UNUSED(void *handle), unsigned int *bufsiz, unsigned int *fmt, unsigned int *speed)
{
    OSStatus		error = noErr;
    unsigned short 	i;
    
    alWriteFormat = *fmt;
    alWriteSpeed = *speed;
    
    /* Set the buffers states to empty */
    for(i=0; i<maxBuffer; i++)
    {
	if ( (libGlobals.buffer[i].startOfDataPtr = malloc(libGlobals.deviceWBufferSize)) == NULL) return AL_FALSE;
	libGlobals.buffer[i].bufferIsEmpty = AL_TRUE;
    }
    libGlobals.bufferToFill = 0;
    libGlobals.bufferToRead = 0;
    
    /* start playing sound through the device */
    error = AudioDeviceStart(libGlobals.deviceW, deviceFillingProc);
    if (error != noErr) return NULL;
    
    /* defines what the AL buffer size should be */
    switch(alWriteFormat)
    {
	case AL_FORMAT_STEREO8:
	case AL_FORMAT_MONO16: *bufsiz = libGlobals.deviceWBufferSize/4;
	break;
	
	case AL_FORMAT_STEREO16: *bufsiz = libGlobals.deviceWBufferSize/2;
	break;
	
	case AL_FORMAT_MONO8: *bufsiz = libGlobals.deviceWBufferSize/8;
	break;

	default: break;
    }

    
    return AL_TRUE;
}


ALboolean set_read_native(void *handle, unsigned int *bufsiz, unsigned int *fmt, unsigned int *speed)
{
    implement_me("ALboolean set_read_native()");
    return AL_FALSE;
}


int   grab_mixerfd(void)
{
    implement_me("int grab_mixerfd()");
    return NULL;
}


void  native_blitbuffer(void *handle, void *data, int bytes)
{
    UInt32	samples, count;   
    SInt16	*inDataPtr16 = (SInt16 *) data;
    SInt8	*inDataPtr8 = (SInt8 *) data;
    Float32	*outDataPtr;
    
    if (handle == NULL) return;

    outDataPtr = (Float32 *) (libGlobals.buffer[libGlobals.bufferToFill].startOfDataPtr);


    switch(alWriteFormat)
    {
	case AL_FORMAT_MONO16:
	{
	    samples = bytes / 2;
	    for (count = 0; count < samples; count++)
	    {
		*outDataPtr = ((Float32) *inDataPtr16)/32767.0;
		outDataPtr++;
		*outDataPtr = ((Float32) *inDataPtr16)/32767.0;
		outDataPtr++;
		inDataPtr16++;
	    }
	}
	break;
	
	case AL_FORMAT_STEREO16:
	{
	    samples = bytes / 2;
	    for (count = 0; count < samples; count++)
	    {
		*outDataPtr = ((Float32) *inDataPtr16)/32767.0;
		outDataPtr++;
		inDataPtr16++;
	    }
	}
	break;
	
	case AL_FORMAT_MONO8:
	{
	    for (count = 0; count < (UInt32)bytes; count++)
	    {
		*outDataPtr = ((Float32) *inDataPtr8)/255.0;
		outDataPtr++;
		*outDataPtr = ((Float32) *inDataPtr8)/255.0;
		outDataPtr++;
		inDataPtr8++;
	    }
	}
	break;

	case AL_FORMAT_STEREO8:
	{
	    for (count = 0; count < (UInt32)bytes; count++)
	    {
		*outDataPtr = ((Float32) *inDataPtr8)/255.0;
		outDataPtr++;
		inDataPtr8++;
	    }
	}
	break;
	
	default: break;
    }


    libGlobals.buffer[libGlobals.bufferToFill].bufferIsEmpty = AL_FALSE;
    libGlobals.bufferToFill++;
    if (libGlobals.bufferToFill == maxBuffer) libGlobals.bufferToFill = 0;
    while (libGlobals.buffer[libGlobals.bufferToFill].bufferIsEmpty != AL_TRUE) _alMicroSleep(1000);
}


void  release_native(void *handle)
{
    if (libGlobals.deviceW == *(AudioDeviceID*)handle)
    {
        AudioDeviceStop(libGlobals.deviceW, deviceFillingProc);
	AudioDeviceRemoveIOProc(libGlobals.deviceW, deviceFillingProc);
    }
}

float get_nativechannel(void *handle, ALCenum channel)
{
    implement_me("float get_nativechannel()");
    return NULL;
}

int   set_nativechannel(void *handle, ALCenum channel, float volume)
{
    implement_me("int set_nativechannel()");
    return NULL;
}

void pause_nativedevice(void *handle) /* Not tested :-( */
{
    if (libGlobals.deviceW == *(AudioDeviceID*)handle)
	AudioDeviceStop(libGlobals.deviceW, deviceFillingProc);
}

void resume_nativedevice(void *handle) /* Not tested :-( */
{
    if (libGlobals.deviceW == *(AudioDeviceID*)handle)
	AudioDeviceStart(libGlobals.deviceW, deviceFillingProc);
}

ALsizei capture_nativedevice(void *handle, void *capture_buffer, int bufsiz)
{
    implement_me("void capture_nativedevice()");
    return 0;
}
