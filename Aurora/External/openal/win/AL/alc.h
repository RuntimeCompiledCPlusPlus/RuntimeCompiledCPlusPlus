#ifndef _ALC_H_
#define _ALC_H_

#include "altypes.h"
#include "alctypes.h"

#ifdef _WIN32
 #define ALCAPIENTRY __cdecl
 #pragma comment(lib, "winmm.lib")
 #pragma comment(lib, "dxguid.lib")
 #include <windows.h>
 #include <dsound.h>
 #ifdef _LIB
  #include "..\OpenAL32\include\alListener.h"
  #include "..\OpenAL32\include\alSource.h"
  #define ALCAPI __declspec(dllexport)
  typedef struct ALCdevice_struct 
  {
 	ALenum		LastError;
 	ALboolean	InUse;
 	ALboolean	Valid;

	ALuint		Frequency;
	ALuint		Channels;
	ALenum		Format;

	ALint		MajorVersion;
	ALint		MinorVersion;

	//Platform specific variables
	CRITICAL_SECTION mutex;

	// Maximum number of sources that can be created
	ALuint		MaxNoOfSources;

	//DirectSound
	LPDIRECTSOUND DShandle;
	LPDIRECTSOUNDBUFFER DSpbuffer;
	LPDIRECTSOUNDBUFFER DSsbuffer;	// Only used by 'DirectSound' device
	LPDIRECTSOUND3DLISTENER DS3dlistener;

	//waveOut
	HWAVEOUT	handle;
	WAVEHDR		buffer[3];

	//mmTimer
	MMRESULT	timer;
  } ALCdevice;
  typedef struct ALCcontext_struct 
  {
	ALlistener	Listener;
	
	ALsource *	Source;
	ALuint		SourceCount;

	ALenum		LastError;
	ALboolean	InUse;
	ALboolean	Valid;

	ALuint		Frequency;
	ALuint		Channels;
	ALenum		Format;

	ALenum		DistanceModel;

	ALfloat		DopplerFactor;
	ALfloat		DopplerVelocity;

	ALCdevice * Device;

	struct ALCcontext_struct *previous;
	struct ALCcontext_struct *next;
  }  ALCcontext;
 #else
  #define ALCAPI __declspec(dllimport)
 typedef ALvoid * ALCdevice;
 typedef ALvoid * ALCcontext;  
#endif
#else
 #define ALCAPI
 #define ALCAPIENTRY __cdecl
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AL_NO_PROTOTYPES

ALCAPI ALCubyte*  ALCAPIENTRY alcGetString(ALCdevice *device,ALCenum param);
ALCAPI ALCvoid    ALCAPIENTRY alcGetIntegerv(ALCdevice *device,ALCenum param,ALCsizei size,ALCint *data);

ALCAPI ALCdevice* ALCAPIENTRY alcOpenDevice(ALCubyte *deviceName);
ALCAPI ALCvoid    ALCAPIENTRY alcCloseDevice(ALCdevice *device);

ALCAPI ALCcontext*ALCAPIENTRY alcCreateContext(ALCdevice *device,ALCint *attrList);
ALCAPI ALCboolean ALCAPIENTRY alcMakeContextCurrent(ALCcontext *context);
ALCAPI ALCvoid	  ALCAPIENTRY alcProcessContext(ALCcontext *context);
ALCAPI ALCcontext*ALCAPIENTRY alcGetCurrentContext(ALCvoid);
ALCAPI ALCdevice* ALCAPIENTRY alcGetContextsDevice(ALCcontext *context);
ALCAPI ALCvoid	  ALCAPIENTRY alcSuspendContext(ALCcontext *context);
ALCAPI ALCvoid    ALCAPIENTRY alcUpdateContext(ALCcontext *context,ALuint type,ALuint name);
ALCAPI ALCvoid    ALCAPIENTRY alcDestroyContext(ALCcontext *context);

ALCAPI ALCenum	  ALCAPIENTRY alcGetError(ALCdevice *device);

ALCAPI ALCboolean ALCAPIENTRY alcIsExtensionPresent(ALCdevice *device,ALCubyte *extName);
ALCAPI ALCvoid *  ALCAPIENTRY alcGetProcAddress(ALCdevice *device,ALCubyte *funcName);
ALCAPI ALCenum	  ALCAPIENTRY alcGetEnumValue(ALCdevice *device,ALCubyte *enumName);
				
#else /* AL_NO_PROTOTYPES */

ALCAPI ALCubyte*  ALCAPIENTRY (*alcGetString)(ALCdevice *device,ALCenum param);
ALCAPI ALCvoid    ALCAPIENTRY (*alcGetIntegerv)(ALCdevice * device,ALCenum param,ALCsizei size,ALCint *data);

ALCAPI ALCdevice* ALCAPIENTRY (*alcOpenDevice)(ALubyte *deviceName);
ALCAPI ALCvoid    ALCAPIENTRY (*alcCloseDevice)(ALCdevice *device);

ALCAPI ALCcontext*ALCAPIENTRY (*alcCreateContext)(ALCdevice *device,ALCint *attrList);
ALCAPI ALCboolean ALCAPIENTRY (*alcMakeContextCurrent)(ALCcontext *context);
ALCAPI ALCvoid	  ALCAPIENTRY (*alcProcessContext)(ALCcontext *context);
ALCAPI ALCcontext*ALCAPIENTRY (*alcGetCurrentContext)(ALCvoid);
ALCAPI ALCdevice* ALCAPIENTRY (*alcGetContextsDevice)(ALCcontext *context);
ALCAPI ALCvoid	  ALCAPIENTRY (*alcSuspendContext)(ALCcontext *context);
ALCAPI ALCvoid    ALCAPIENTRY (*alcUpdateContext)(ALCcontext *context,ALuint type,ALuint name);
ALCAPI ALCvoid    ALCAPIENTRY (*alcDestroyContext)(ALCcontext *context);

ALCAPI ALCenum	  ALCAPIENTRY (*alcGetError)(ALCdevice *device);

ALCAPI ALCboolean ALCAPIENTRY (*alcIsExtensionPresent)(ALCdevice *device,ALCubyte *extName);
ALCAPI ALCvoid *  ALCAPIENTRY (*alcGetProcAddress)(ALCdevice *device,ALCubyte *funcName);
ALCAPI ALCenum	  ALCAPIENTRY (*alcGetEnumValue)(ALCdevice *device,ALCubyte *enumName);

#endif /* AL_NO_PROTOTYPES */

#ifdef __cplusplus
};
#endif

#endif
