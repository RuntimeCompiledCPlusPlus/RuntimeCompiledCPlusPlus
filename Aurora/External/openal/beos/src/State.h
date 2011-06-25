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

#ifndef __AL_STATE_H
#define __AL_STATE_H

#include "Types.h"

ALstate *alimCreateState(ALvoid);

ALvoid alimDeleteState(ALstate *state);

ALvoid alimEnable(ALcontext *context, ALenum capability);

ALvoid alimDisable(ALcontext *context, ALenum capability);

ALboolean alimIsEnabled(ALcontext *context, ALenum capability);

ALboolean alimGetBoolean(ALcontext *context, ALenum pname);

ALdouble alimGetDouble(ALcontext *context, ALenum pname);

ALfloat alimGetFloat(ALcontext *context, ALenum pname);

ALint alimGetInteger(ALcontext *context, ALenum pname);

ALvoid alimGetBooleanv(ALcontext *context, ALenum pname, ALboolean *data);

ALvoid alimGetDoublev(ALcontext *context, ALenum pname, ALdouble *data);

ALvoid alimGetFloatv(ALcontext *context, ALenum pname, ALfloat *data);

ALvoid alimGetIntegerv(ALcontext *context, ALenum pname, ALint *data);

const ALubyte *alimGetString(ALcontext *context, ALenum pname);

ALenum alimGetError(ALcontext *context);

ALvoid alimSetError(ALcontext *context, ALenum error, const ALubyte *where);

const ALubyte *alimGetErrorString(ALcontext *context, ALenum error);

#endif
