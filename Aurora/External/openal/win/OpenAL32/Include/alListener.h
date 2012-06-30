#ifndef _AL_LISTENER_H_
#define _AL_LISTENER_H_

#define ALAPI __declspec(dllexport)
#define ALAPIENTRY __cdecl

#include <al\altypes.h>
//NO EAX SUPPORT// #include "..\include\eax.h"

#ifdef __cplusplus
extern "C" {
#endif

// Flags indicating what Direct Sound parameters need to be updated in the UpdateContext call
#define LVOLUME				1
#define LPOSITION			2
#define LVELOCITY			4
#define LORIENTATION		8
#define LDOPPLERFACTOR		16
#define LDOPPLERVELOCITY	32
#define LROLLOFFFACTOR		64

// Flags indicated what EAX 2 Listener parameters need to be updated in the UpdateContext call
#define LALLPARAMS			1	
#define LROOM				2
#define LROOMHF				4
#define LROOMROLLOFFFACTOR	8
#define LDECAYTIME			16
#define LDECAYHFRATIO		32
#define LREFLECTIONS		64
#define LREFLECTIONSDELAY	128
#define LREVERB				256
#define LREVERBDELAY		512
#define LENVIRONMENT		1024
#define LENVIRONMENTSIZE	2048
#define LENVIRONMENTDIFFUSION	4096
#define LAIRABSORPTIONHF	8192
#define LFLAGS				16384
#define LDEFERRED			32768
#define LCOMMITSETTINGS		65536

#pragma pack(push, 4)

typedef struct ALlistener_struct
{
	ALfloat		Position[3];
	ALfloat		Velocity[3];
	ALfloat		Forward[3];
	ALfloat		Up[3];
	ALfloat		Gain;
	ALint		Environment;
	ALuint		update1;			// Store changes that need to be made in UpdateContext
	ALuint		update2;			// Store changes that need to be made in UpdateContext (EAX Related)

//	EAXLISTENERPROPERTIES eaxLP;	// EAX Listener Parameters

} ALlistener;

#pragma pack (pop)

ALAPI ALvoid	ALAPIENTRY alListenerf(ALenum pname,ALfloat value);
ALAPI ALvoid	ALAPIENTRY alListener3f(ALenum pname,ALfloat v1,ALfloat v2,ALfloat v3); 
ALAPI ALvoid	ALAPIENTRY alListenerfv(ALenum pname,ALfloat *values); 
ALAPI ALvoid	ALAPIENTRY alListeneri(ALenum pname,ALint value);
ALAPI ALvoid	ALAPIENTRY alGetListenerf(ALenum pname,ALfloat *value);
ALAPI ALvoid	ALAPIENTRY alGetListener3f(ALenum pname,ALfloat *v1,ALfloat *v2,ALfloat *v3); 
ALAPI ALvoid	ALAPIENTRY alGetListenerfv(ALenum pname,ALfloat *values); 
ALAPI ALvoid	ALAPIENTRY alGetListeneri(ALenum pname,ALint *value);

#ifdef __cplusplus
}
#endif

#endif