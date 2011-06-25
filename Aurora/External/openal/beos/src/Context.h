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

#ifndef __AL_CONTEXT_H
#define __AL_CONTEXT_H

#include "Types.h"


ALvoid alimInit(ALvoid);

ALvoid alimExit(ALvoid);

ALcontext *alimGetCurrent(ALvoid);

ALvoid alimMakeCurrent(ALcontext *context);


ALcontext *alimCreateContext(ALuint frequency, ALenum format, ALsizei size);

ALvoid alimDeleteContext(ALcontext *context);

ALvoid alimUpdateContext(ALcontext *context);


ALplayer *alimContextPlayer(ALcontext *context);

ALmixer *alimContextMixer(ALcontext *context);

ALstate *alimContextState(ALcontext *context);

ALlistener *alimContextListener(ALcontext *context);

ALbuffer *alimContextBuffer(ALcontext *context, ALuint buffer);

ALsource *alimContextSource(ALcontext *context, ALuint source);

ALenvironment *alimContextEnvironmentIASIG(ALcontext *context, ALuint environment);


ALsizei alimGenBuffers(ALcontext *context, ALsizei n, ALuint *buffers);

ALvoid alimDeleteBuffers(ALcontext *context, ALsizei n, ALuint *buffers);

ALboolean alimIsBuffer(ALcontext *context, ALuint buffer);


ALsizei alimGenSources(ALcontext *context, ALsizei n, ALuint *sources);

ALvoid alimDeleteSources(ALcontext *context, ALsizei n, ALuint *sources);

ALboolean alimIsSource(ALcontext *context, ALuint source);


ALsizei alimGenEnvironmentsIASIG(ALcontext *context, ALsizei n, ALuint *environments);

ALvoid alimDeleteEnvironmentsIASIG(ALcontext *context, ALsizei n, ALuint *environments);

ALboolean alimIsEnvironmentIASIG(ALcontext *context, ALuint environment);


#endif
