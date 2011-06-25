#ifndef _AL_STATE_H_
#define _AL_STATE_H_

#define ALAPI __declspec(dllexport)
#define ALAPIENTRY __cdecl

#include <al\altypes.h>

#ifdef __cplusplus
extern "C" {
#endif

ALAPI ALvoid	ALAPIENTRY alEnable(ALenum capability);
ALAPI ALvoid	ALAPIENTRY alDisable(ALenum capability);
ALAPI ALboolean	ALAPIENTRY alIsEnabled(ALenum capability);
ALAPI ALboolean	ALAPIENTRY alGetBoolean(ALenum pname);
ALAPI ALdouble	ALAPIENTRY alGetDouble(ALenum pname);
ALAPI ALfloat	ALAPIENTRY alGetFloat(ALenum pname);
ALAPI ALint		ALAPIENTRY alGetInteger(ALenum pname);
ALAPI ALvoid	ALAPIENTRY alGetBooleanv(ALenum pname,ALboolean *data);
ALAPI ALvoid	ALAPIENTRY alGetDoublev(ALenum pname,ALdouble *data);
ALAPI ALvoid	ALAPIENTRY alGetFloatv(ALenum pname,ALfloat *data);
ALAPI ALvoid	ALAPIENTRY alGetIntegerv(ALenum pname,ALint *data);
ALAPI ALubyte * ALAPIENTRY alGetString(ALenum pname);

ALAPI ALvoid	ALAPIENTRY alDistanceModel( ALenum value );
ALAPI ALvoid	ALAPIENTRY alDopplerScale( ALfloat value );
ALAPI ALvoid	ALAPIENTRY alDopplerVelocity( ALfloat value );

#ifdef __cplusplus
}
#endif

#endif