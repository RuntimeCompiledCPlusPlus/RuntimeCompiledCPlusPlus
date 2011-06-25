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

#ifndef __AL_SOURCE_H
#define __AL_SOURCE_H

#include "Types.h"

ALsource *alimCreateSource(ALvoid);

ALvoid alimDeleteSource(ALsource *source);

ALvoid alimSourcef(ALcontext *context, ALuint source, ALenum pname, ALfloat value);

ALvoid alimSourcefv(ALcontext *context, ALuint source, ALenum pname, const ALfloat *values);

ALvoid alimSource3f(ALcontext *context, ALuint source, ALenum pname, ALfloat value1, ALfloat value2, ALfloat value3);

ALvoid alimSourcei(ALcontext *context, ALuint source, ALenum pname, ALint value);

ALvoid alimGetSourcef(ALcontext *context, ALuint source, ALenum pname, ALfloat *value);

ALvoid alimGetSourcefv(ALcontext *context, ALuint source, ALenum pname, ALfloat *values);

ALvoid alimGetSourcei(ALcontext *context, ALuint source, ALenum pname, ALint *value);

ALvoid alimSourcePlay(ALcontext *context, ALuint source);

ALvoid alimSourcePause(ALcontext *context, ALuint source);

ALvoid alimSourceStop(ALcontext *context, ALuint source);

#endif
