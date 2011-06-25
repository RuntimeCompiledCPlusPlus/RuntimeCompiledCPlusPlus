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

#ifdef __cplusplus
extern "C" {
#endif

#include "altypes.h"

#define ALAPI 
#define ALAPIENTRY 

#pragma export on
// AL_MAIN functions
ALAPI ALvoid ALAPIENTRY alInit(ALint *argc, ALubyte **argv);
ALAPI ALvoid ALAPIENTRY alExit(ALvoid);

// AL_BUFFER functions
ALAPI ALvoid ALAPIENTRY alGenBuffers(ALsizei n, ALuint *buffers);
ALAPI ALvoid ALAPIENTRY alDeleteBuffers(ALsizei n, ALuint *buffers);
ALAPI ALboolean ALAPIENTRY alIsBuffer(ALuint buffer);
ALAPI ALvoid ALAPIENTRY alBufferData(ALuint buffer,ALenum format,ALvoid *data,ALsizei size,ALsizei freq);
ALAPI ALvoid ALAPIENTRY alGetBufferf (ALuint buffer, ALenum pname, ALfloat *value);
ALAPI ALvoid ALAPIENTRY alGetBufferi(ALuint buffer, ALenum pname, ALint *value);

// AL_SOURCE functions
ALAPI ALvoid ALAPIENTRY alGenSources(ALsizei n, ALuint *sources);
ALAPI ALvoid ALAPIENTRY alDeleteSources (ALsizei n, ALuint *sources);
ALAPI ALboolean ALAPIENTRY alIsSource(ALuint source);
ALAPI ALvoid ALAPIENTRY alSourcef (ALuint source, ALenum paname, ALfloat value);
ALAPI ALvoid ALAPIENTRY alSourcefv (ALuint source, ALenum pname, ALfloat *values);
ALAPI ALvoid ALAPIENTRY alSource3f (ALuint source, ALenum pname, ALfloat v1, ALfloat v2, ALfloat v3);
ALAPI ALvoid ALAPIENTRY alSourcei (ALuint id, ALenum param, ALint value);
ALAPI ALvoid ALAPIENTRY alGetSourcef (ALuint soruce, ALenum pname, ALfloat *value);
ALAPI ALvoid ALAPIENTRY alGetSourcefv (ALuint source, ALenum pname, ALfloat *values);
ALAPI ALvoid ALAPIENTRY alGetSourcei (ALuint source, ALenum pname, ALint *value);
ALAPI ALvoid ALAPIENTRY alSourcePlay(ALuint source);
ALAPI ALvoid ALAPIENTRY alSourcePause (ALuint source);
ALAPI ALvoid ALAPIENTRY alSourceStop (ALuint source);
ALAPI ALvoid ALAPIENTRY alSourceRewind (ALuint source);
ALAPI ALvoid ALAPIENTRY alSourcePlayv(ALsizei n, ALuint *ID);
ALAPI ALvoid ALAPIENTRY alSourcePausev(ALsizei n, ALuint *ID);
ALAPI ALvoid ALAPIENTRY alSourceStopv(ALsizei n, ALuint *ID);
ALAPI ALvoid ALAPIENTRY alSourceRewindv (ALsizei n, ALuint *ID);
ALAPI ALvoid ALAPIENTRY alQueuei (ALuint source, ALenum param, ALint value);
ALAPI ALvoid ALAPIENTRY alQueuef (ALuint source, ALenum param, ALfloat value);
ALAPI ALvoid ALAPIENTRY alUnqueue (ALuint source, ALsizei n, ALuint *buffers);

// AL_STATE functions
ALAPI ALvoid ALAPIENTRY alEnable (ALenum capability);
ALAPI ALvoid ALAPIENTRY alDisable (ALenum capability);
ALAPI ALboolean ALAPIENTRY alIsEnabled(ALenum capability);
ALAPI ALboolean ALAPIENTRY alGetBoolean (ALenum pname);
ALAPI ALdouble ALAPIENTRY alGetDouble (ALenum pname);
ALAPI ALfloat ALAPIENTRY alGetFloat (ALenum pname);
ALAPI ALint ALAPIENTRY alGetInteger (ALenum pname);
ALAPI ALvoid ALAPIENTRY alGetBooleanv (ALenum pname, ALboolean *data);
ALAPI ALvoid ALAPIENTRY alGetDoublev (ALenum pname, ALdouble *data);
ALAPI ALvoid ALAPIENTRY alGetFloatv (ALenum pname, ALfloat *data);
ALAPI ALvoid ALAPIENTRY alGetIntegerv (ALenum pname, ALint *data);
ALAPI const ALubyte * ALAPIENTRY alGetString (ALenum pname);
ALAPI ALvoid ALAPIENTRY alDopplerScale (ALfloat value);
ALAPI ALvoid ALAPIENTRY alDistanceScale (ALfloat value);
ALAPI ALvoid ALAPIENTRY alPropagationSpeed (ALfloat value);

// AL_ERROR functions
ALAPI ALenum ALAPIENTRY alGetError (ALvoid);

// AL_EXTENSION functions
ALAPI ALboolean ALAPIENTRY alIsExtensionPresent(ALubyte *ename);
ALAPI ALvoid * ALAPIENTRY alGetProcAddress(ALubyte *fname);
ALAPI ALenum ALAPIENTRY alGetEnumValue (ALubyte *ename);

// AL_LISTENER functions
ALAPI ALvoid ALAPIENTRY alListenerf (ALenum pname, ALfloat value);
ALAPI ALvoid ALAPIENTRY alListener3f (ALenum pname, ALfloat v1, ALfloat v2, ALfloat v3);
ALAPI ALvoid ALAPIENTRY alListenerfv (ALenum pname, ALfloat *values);
ALAPI ALvoid ALAPIENTRY alListeneri (ALenum pname, ALint value);

#pragma export off

#ifdef __cplusplus
}  /* extern "C" */
#endif
