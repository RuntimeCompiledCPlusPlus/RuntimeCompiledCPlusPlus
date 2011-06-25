#ifndef __alc_h_
#define __alc_h_

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

#include <AL/altypes.h>

#ifdef __cplusplus
extern "C" {
#endif

ALAPI ALvoid * ALAPIENTRY alcCreateContext(ALuint frequency, ALenum format, ALsizei size);

ALAPI ALvoid ALAPIENTRY alcDeleteContext(ALvoid *context);

ALAPI ALvoid ALAPIENTRY alcUpdateContext(ALvoid *context);

ALAPI ALvoid ALAPIENTRY alcMakeCurrent(ALvoid *context);

#ifdef __cplusplus
}
#endif

#endif
