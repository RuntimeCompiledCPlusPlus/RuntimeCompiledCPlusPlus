/*
 * OpenAL cross platform audio library
 *
 * Copyright (C) 1999-2000 by Authors.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include "Thread.h"
#include "State.h"
#include "Listener.h"
#include "Buffer.h"
#include "Source.h"
#include "Environment.h"
#include "Extension.h"
#include "Context.h"
#include "Main.h"


/* Main */

ALAPI ALvoid ALAPIENTRY alInit(ALint *argc, ALubyte **argv)
{
	alimInit();
}

ALAPI ALvoid ALAPIENTRY alExit(ALvoid)
{
	alimExit();
}


/* Context */

ALAPI ALvoid * ALAPIENTRY alcCreateContext(ALuint frequency, ALenum format, ALsizei size)
{
	return (ALvoid *) alimCreateContext(frequency, format, size);
}

ALAPI ALvoid ALAPIENTRY alcDeleteContext(ALvoid *context)
{
	alimDeleteContext((ALcontext *) context);
}

ALAPI ALvoid ALAPIENTRY alcMakeCurrent(ALvoid *context)
{
	alimMakeCurrent((ALcontext *) context);
}

ALAPI ALvoid ALAPIENTRY alcUpdateContext(ALvoid *context)
{
	alimUpdateContext((ALcontext *) context);
}


/* Listener */

ALAPI ALvoid ALAPIENTRY alListenerf(ALenum pname, ALfloat value)
{
	alimListenerf(alimGetCurrent(), pname, value);
}

ALAPI ALvoid ALAPIENTRY alListener3f(ALenum pname, ALfloat value1, ALfloat value2, ALfloat value3)
{
	alimListener3f(alimGetCurrent(), pname, value1, value2, value3);
}

ALAPI ALvoid ALAPIENTRY alListenerfv(ALenum pname, ALfloat *values)
{
	alimListenerfv(alimGetCurrent(), pname, values);
}

ALAPI ALvoid ALAPIENTRY alListeneri(ALenum pname, ALint value)
{
	alimListeneri(alimGetCurrent(), pname, value);
}

ALAPI ALvoid ALAPIENTRY alGetListenerf(ALenum pname, ALfloat *value)
{
	alimGetListenerf(alimGetCurrent(), pname, value);
}

ALAPI ALvoid ALAPIENTRY alGetListener3f(ALenum pname, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
	alimGetListener3f(alimGetCurrent(), pname, value1, value2, value3);
}

ALAPI ALvoid ALAPIENTRY alGetListenerfv(ALenum pname, ALfloat *values)
{
	alimGetListenerfv(alimGetCurrent(), pname, values);
}

ALAPI ALvoid ALAPIENTRY alGetListeneri(ALenum pname, ALint *value)
{
	alimGetListeneri(alimGetCurrent(), pname, value);
}


/* Buffers */

ALAPI ALsizei ALAPIENTRY alGenBuffers(ALsizei n, ALuint *buffers)
{
	return alimGenBuffers(alimGetCurrent(), n, buffers);
}

ALAPI ALvoid ALAPIENTRY alDeleteBuffers(ALsizei n, ALuint *buffers)
{
	alimDeleteBuffers(alimGetCurrent(), n, buffers);
}

ALAPI ALboolean ALAPIENTRY alIsBuffer(ALuint buffer)
{
	return alimIsBuffer(alimGetCurrent(), buffer);
}

ALAPI ALvoid ALAPIENTRY alBufferData(ALuint buffer, ALenum format, ALvoid *data, ALsizei size, ALsizei frequency)
{
	alimBufferData(alimGetCurrent(), buffer, format, data, size, frequency);
}

ALAPI ALvoid ALAPIENTRY alGetBufferf(ALuint buffer, ALenum pname, ALfloat *value)
{
	alimGetBufferf(alimGetCurrent(), buffer, pname, value);
}

ALAPI ALvoid ALAPIENTRY alGetBufferi(ALuint buffer, ALenum pname, ALint *value)
{
	alimGetBufferi(alimGetCurrent(), buffer, pname, value);
}


/* Sources */

ALAPI ALsizei ALAPIENTRY alGenSources(ALsizei n, ALuint *sources)
{
	return alimGenSources(alimGetCurrent(), n, sources);
}

ALAPI ALvoid ALAPIENTRY alDeleteSources(ALsizei n, ALuint *sources)
{
	alimDeleteSources(alimGetCurrent(), n, sources);
}

ALAPI ALboolean ALAPIENTRY alIsSource(ALuint source)
{
	return alimIsSource(alimGetCurrent(), source);
}

ALAPI ALvoid ALAPIENTRY alSourcef(ALuint source, ALenum pname, ALfloat value)
{
	alimSourcef(alimGetCurrent(), source, pname, value);
}

ALAPI ALvoid ALAPIENTRY alSourcefv(ALuint source, ALenum pname, ALfloat *values)
{
	alimSourcefv(alimGetCurrent(), source, pname, values);
}

ALAPI ALvoid ALAPIENTRY alSource3f(ALuint source, ALenum pname, ALfloat value1, ALfloat value2, ALfloat value3)
{
	alimSource3f(alimGetCurrent(), source, pname, value1, value2, value3);
}

ALAPI ALvoid ALAPIENTRY alSourcei(ALuint source, ALenum pname, ALint value)
{
	alimSourcei(alimGetCurrent(), source, pname, value);
}

ALAPI ALvoid ALAPIENTRY alGetSourcef(ALuint source, ALenum pname, ALfloat *value)
{
	alimGetSourcef(alimGetCurrent(), source, pname, value);
}

ALAPI ALvoid ALAPIENTRY alGetSourcefv(ALuint source, ALenum pname, ALfloat *values)
{
	alimGetSourcefv(alimGetCurrent(), source, pname, values);
}

ALAPI ALvoid ALAPIENTRY alGetSourcei(ALuint source, ALenum pname, ALint *value)
{
	alimGetSourcei(alimGetCurrent(), source, pname, value);
}

ALAPI ALvoid ALAPIENTRY alSourcePlay(ALuint source)
{
	alimSourcePlay(alimGetCurrent(), source);
}

ALAPI ALvoid ALAPIENTRY alSourcePause(ALuint source)
{
	alimSourcePause(alimGetCurrent(), source);
}

ALAPI ALvoid ALAPIENTRY alSourceStop(ALuint source)
{
	alimSourceStop(alimGetCurrent(), source);
}


/* Environment */

ALAPI ALsizei ALAPIENTRY alGenEnvironmentIASIG(ALsizei n, ALuint *environments)
{
	return alimGenEnvironmentsIASIG(alimGetCurrent(), n, environments);
}

ALAPI ALvoid ALAPIENTRY alDeleteEnvironmentIASIG(ALsizei n, ALuint *environments)
{
	alimDeleteEnvironmentsIASIG(alimGetCurrent(), n, environments);
}

ALAPI ALboolean ALAPIENTRY alIsEnvironmentIASIG(ALuint environment)
{
	return alimIsEnvironmentIASIG(alimGetCurrent(), environment);
}

ALAPI ALvoid ALAPIENTRY alEnvironmentiIASIG(ALuint environment, ALenum pname, ALint value)
{
	alimEnvironmentiIASIG(alimGetCurrent(), environment, pname, value);
}

ALAPI ALvoid ALAPIENTRY alEnvironmentfIASIG(ALuint environment, ALenum pname, ALfloat value)
{
	alimEnvironmentfIASIG(alimGetCurrent(), environment, pname, value);
}

ALAPI ALvoid ALAPIENTRY alGetEnvironmentiIASIG(ALuint environment, ALenum pname, ALint *value)
{
	alimGetEnvironmentiIASIG(alimGetCurrent(), environment, pname, value);
}


ALAPI ALvoid ALAPIENTRY alGetEnvironmentfIASIG(ALuint environment, ALenum pname, ALfloat *value)
{
	alimGetEnvironmentfIASIG(alimGetCurrent(), environment, pname, value);
}


/* State */

ALAPI ALvoid ALAPIENTRY alEnable(ALenum capability)
{
	alimEnable(alimGetCurrent(), capability);
}

ALAPI ALvoid ALAPIENTRY alDisable(ALenum capability)
{
	alimDisable(alimGetCurrent(), capability);
}

ALAPI ALboolean	ALAPIENTRY alIsEnabled(ALenum capability)
{
	return alimIsEnabled(alimGetCurrent(), capability);
}

ALAPI ALboolean	ALAPIENTRY alGetBoolean(ALenum pname)
{
	return alimGetBoolean(alimGetCurrent(), pname);
}

ALAPI ALdouble ALAPIENTRY alGetDouble(ALenum pname)
{
	return alimGetDouble(alimGetCurrent(), pname);
}

ALAPI ALfloat ALAPIENTRY alGetFloat(ALenum pname)
{
	return alimGetFloat(alimGetCurrent(), pname);
}

ALAPI ALint	ALAPIENTRY alGetInteger(ALenum pname)
{
	return alimGetInteger(alimGetCurrent(), pname);
}

ALAPI ALvoid ALAPIENTRY alGetBooleanv(ALenum pname, ALboolean *data)
{
	alimGetBooleanv(alimGetCurrent(), pname, data);
}

ALAPI ALvoid ALAPIENTRY alGetDoublev(ALenum pname, ALdouble *data)
{
	alimGetDoublev(alimGetCurrent(), pname, data);
}

ALAPI ALvoid ALAPIENTRY alGetFloatv(ALenum pname, ALfloat *data)
{
	alimGetFloatv(alimGetCurrent(), pname, data);
}

ALAPI ALvoid ALAPIENTRY alGetIntegerv(ALenum pname, ALint *data)
{
	alimGetIntegerv(alimGetCurrent(), pname, data);
}

ALAPI const ALubyte * ALAPIENTRY alGetString(ALenum pname)
{
	return alimGetString(alimGetCurrent(), pname);
}

ALAPI ALenum ALAPIENTRY alGetError(ALvoid)
{
	return alimGetError(alimGetCurrent());
}

ALAPI const ALubyte * ALAPIENTRY alGetErrorString(ALenum error)
{
	return alimGetErrorString(alimGetCurrent(), error);
}


/* Extensions */

ALAPI const ALvoid * ALAPIENTRY alGetProcAddress(const ALubyte *name)
{
	return alimGetProcAddress(alimGetCurrent(), name);
}

ALAPI ALenum ALAPIENTRY alGetEnumValue(const ALubyte *ename)
{
	return alimGetEnumValue(alimGetCurrent(), ename);
}
