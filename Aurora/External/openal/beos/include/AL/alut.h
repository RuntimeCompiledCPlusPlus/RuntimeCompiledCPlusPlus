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

#ifndef __alut_h_
#define __alut_h

#include <AL/altypes.h>

#define ALUTAPI
#define ALUTAPIENTRY

#ifdef __cplusplus
extern "C" {
#endif

ALUTAPI ALvoid ALUTAPIENTRY alutInit(int *argc, char **argv);

ALUTAPI ALvoid ALUTAPIENTRY alutExit(void);

ALUTAPI ALvoid ALUTAPIENTRY alutLoadWAV(const ALubyte *name, ALenum *format, ALvoid **data, ALsizei *size, ALsizei *freq);

#ifdef __cplusplus
};
#endif

#endif
