#ifndef ALC_CONTEXT_H_
#define ALC_CONTEXT_H_

#include <AL/altypes.h>
#include <AL/alctypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ALC_VERSION_0_1         1

#ifdef _WIN32
#define ALAPI      __declspec(dllexport)
#define ALAPIENTRY __cdecl
#else  /* _WIN32 */

#ifdef TARGET_OS_MAC
#if TARGET_OS_MAC
#pragma export on
#endif /* TARGET_OS_MAC */
#endif /* TARGET_OS_MAC */

#define ALAPI
#define ALAPIENTRY
#define AL_CALLBACK
#endif /* _WIN32 */

#ifndef AL_NO_PROTOTYPES

ALAPI void * ALAPIENTRY alcCreateContext( struct _AL_device *dev,  ALint* attrlist );

/**
 * There is no current context, as we can mix
 *  several active contexts. But al* calls
 *  only affect the current context.
 */
ALAPI ALCenum ALAPIENTRY alcMakeContextCurrent( ALvoid *alcHandle );

/**
 * Perform processing on a synced context, non-op on a asynchronous
 * context.
 */
ALAPI void *  ALAPIENTRY alcProcessContext( ALvoid *alcHandle );

/**
 * Suspend processing on an asynchronous context, non-op on a
 * synced context.
 */
ALAPI void ALAPIENTRY alcSuspendContext( ALvoid *alcHandle );

ALAPI ALCenum ALAPIENTRY alcDestroyContext( ALvoid *alcHandle );

ALAPI ALCenum ALAPIENTRY alcGetError( ALvoid );

ALAPI void * ALAPIENTRY alcGetCurrentContext( ALvoid );

ALAPI ALCdevice *alcOpenDevice( const ALubyte *tokstr );
ALAPI void alcCloseDevice( ALCdevice *dev );

ALAPI ALboolean ALAPIENTRY alcIsExtensionPresent(ALCdevice *device, ALubyte *extName);
ALAPI ALvoid  * ALAPIENTRY alcGetProcAddress(ALCdevice *device, ALubyte *funcName);
ALAPI ALenum    ALAPIENTRY alcGetEnumValue(ALCdevice *device, ALubyte *enumName);

#else
      void *	     (*alcCreateContext)( ALint* attrlist );
      ALCenum	     (*alcMakeContextCurrent)( ALvoid *alcHandle );
      void *	     (*alcUpdateContext)( ALvoid *alcHandle );
      ALCenum	     (*alcDestroyContext)( ALvoid *alcHandle );
      ALCenum	     (*alcGetError)( ALvoid );
      void *         (*alcGetCurrentContext)( ALvoid );
      ALboolean      (*alcIsExtensionPresent)( ALCdevice *device, ALubyte *extName );
      ALvoid  *      (*alcGetProcAddress)( ALCdevice *device, ALubyte *funcName );
      ALenum         (*alcGetEnumValue)( ALCdevice *device, ALubyte *enumName );
#endif /* AL_NO_PROTOTYPES */

#ifdef TARGET_OS_MAC
#if TARGET_OS_MAC
#pragma export off
#endif /* TARGET_OS_MAC */
#endif /* TARGET_OS_MAC */

#ifdef __cplusplus
}
#endif

#endif /* ALC_CONTEXT_H_ */
