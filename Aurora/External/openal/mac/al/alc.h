/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2000 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */

#ifndef ALC_CONTEXT_H_
#define ALC_CONTEXT_H_

#include "altypes.h"
#include "alctypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALC_VERSION_0_1         1

#define ALAPI
#define ALAPIENTRY

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
} ALCdevice;

typedef struct ALCcontext_struct 
{
	ALenum		LastError;
	ALboolean	InUse;
	ALboolean	Valid;
} ALCcontext;

// MACOS export on
#pragma export on


#ifndef AL_NO_PROTOTYPES

ALAPI void * ALAPIENTRY alcCreateContext( ALCdevice *dev,  ALint* attrlist );

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
ALAPI void ALAPIENTRY alcProcessContext( ALvoid *alcHandle );

/**
 * Suspend processing on an asynchronous context, non-op on a
 * synced context.
 */
ALAPI void ALAPIENTRY alcSuspendContext( ALvoid *alcHandle );

ALAPI ALCenum ALAPIENTRY alcDestroyContext( ALvoid *alcHandle );

ALAPI ALCenum ALAPIENTRY alcGetError( ALvoid );

ALAPI void * ALAPIENTRY alcGetCurrentContext( ALvoid );

ALAPI ALCdevice* ALAPIENTRY alcGetContextsDevice(ALCcontext *context);

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

#pragma export off

#ifdef __cplusplus
}
#endif

#endif /* ALC_CONTEXT_H_ */