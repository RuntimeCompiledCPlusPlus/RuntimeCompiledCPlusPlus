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

#ifndef __AL_LISTENER_H
#define __AL_LISTENER_H

#include "Types.h"

ALlistener *alimCreateListener(ALvoid);

ALvoid alimDeleteListener(ALlistener *listener);

ALvoid alimListenerf(ALcontext *context, ALenum pname, ALfloat value);

ALvoid alimListener3f(ALcontext *context, ALenum pname, ALfloat value1, ALfloat value2, ALfloat value3);

ALvoid alimListenerfv(ALcontext *context, ALenum pname, const ALfloat *values);

ALvoid alimListeneri(ALcontext *context, ALenum pname, ALint value);

ALvoid alimGetListenerf(ALcontext *context, ALenum pname, ALfloat *value);

ALvoid alimGetListener3f(ALcontext *context, ALenum pname, ALfloat *value1, ALfloat *value2, ALfloat *value3);

ALvoid alimGetListenerfv(ALcontext *context, ALenum pname, ALfloat *values);

ALvoid alimGetListeneri(ALcontext *context, ALenum pname, ALint *value);

#endif
