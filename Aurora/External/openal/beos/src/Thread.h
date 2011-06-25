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

#ifndef __AL_THREAD_H
#define __AL_THREAD_H

#include <AL/altypes.h>

typedef struct ALmutex ALmutex;

ALuint alimCreateTLS(ALvoid);

ALvoid alimDeleteTLS(ALuint handle);

ALvoid alimSetTLS(ALuint handle, ALvoid *data);

ALvoid *alimGetTLS(ALuint handle);

ALmutex *alimCreateMutex(ALvoid);

ALvoid alimDeleteMutex(ALmutex *mutex);

ALvoid alimLockMutex(ALmutex *mutex);

ALvoid alimUnlockMutex(ALmutex *mutex);

ALvoid alimDebugger(const ALubyte *message);

#endif
