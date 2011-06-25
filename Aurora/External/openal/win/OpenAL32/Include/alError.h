#ifndef _AL_ERROR_H_
#define _AL_ERROR_H_

#define ALAPI __declspec(dllexport)
#define ALAPIENTRY __cdecl

#include <al\altypes.h>

#ifdef __cplusplus
extern "C" {
#endif

ALAPI ALenum	ALAPIENTRY alGetError(ALvoid);
ALAPI ALvoid	ALAPIENTRY alSetError(ALenum errorCode);

#ifdef __cplusplus
}
#endif

#endif