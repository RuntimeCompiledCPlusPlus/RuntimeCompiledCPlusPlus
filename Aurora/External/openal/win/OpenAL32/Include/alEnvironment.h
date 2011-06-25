#ifndef _AL_ENVIRONMENT_H_
#define _AL_ENVIRONMENT_H_

#define ALAPI __declspec(dllexport)
#define ALAPIENTRY __cdecl

#include <al\altypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ALenvironment_struct 
{
	ALfloat		room;
	ALfloat		room_hf;
	ALfloat		decay_time;
	ALfloat		decay_hf_ratio;
	ALfloat		reflections;
	ALfloat		reflections_delay;
	ALfloat		reverb;
	ALfloat		reverb_delay;
	ALfloat		diffusion;
	ALfloat		density;
	ALfloat		hf_reference;
	ALboolean	inuse;
	ALboolean	valid;
	struct ALenvironment_struct *previous;
	struct ALenvironment_struct *next;
} ALenvironment;

ALAPI ALvoid	ALAPIENTRY alGenEnvironmentIASIG(ALsizei n,ALuint *environments);
ALAPI ALvoid	ALAPIENTRY alDeleteEnvironmentIASIG(ALsizei n,ALuint *environments);
ALAPI ALboolean	ALAPIENTRY alIsEnvironmentIASIG(ALuint environment);
ALAPI ALvoid	ALAPIENTRY alEnvironmentfIASIG(ALuint environment,ALenum pname,ALfloat value);
ALAPI ALvoid	ALAPIENTRY alEnvironmentiIASIG(ALuint environment,ALenum pname,ALint value);
ALAPI ALvoid	ALAPIENTRY alGetEnvironmentfIASIG(ALuint environment,ALenum pname,ALfloat *value);
ALAPI ALvoid	ALAPIENTRY alGetEnvironmentiIASIG(ALuint environment,ALenum pname,ALint *value);

#ifdef __cplusplus
};
#endif

#endif