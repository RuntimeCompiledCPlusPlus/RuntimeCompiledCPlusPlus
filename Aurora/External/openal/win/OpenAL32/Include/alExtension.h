#ifndef _AL_EXTENSION_H_
#define _AL_EXTENSION_H_

#define ALAPI __declspec(dllexport)
#define ALAPIENTRY __cdecl

#include <al\altypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ALextension_struct
{
	ALubyte		*extName;
	ALvoid		*address;
} ALextension;

typedef struct ALfunction_struct
{
	ALubyte		*funcName;
	ALvoid		*address;
} ALfunction;

typedef struct ALenum_struct
{
	ALubyte		*enumName;
	ALenum		value;
} ALenums;

ALAPI ALboolean ALAPIENTRY alIsExtensionPresent(ALubyte *extName);
ALAPI ALvoid *	ALAPIENTRY alGetProcAddress(ALubyte *funcName);
ALAPI ALenum	ALAPIENTRY alGetEnumValue(ALubyte *enumName);

#ifdef __cplusplus
}
#endif

#endif