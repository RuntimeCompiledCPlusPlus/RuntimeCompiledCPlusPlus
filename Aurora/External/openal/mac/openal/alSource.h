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
 
#pragma export on

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
ALAPI ALvoid ALAPIENTRY alSourceRewind(ALuint source);
ALAPI ALvoid ALAPIENTRY alSourcePlayv(ALsizei n, ALuint *ID);
ALAPI ALvoid ALAPIENTRY alSourcePausev(ALsizei n, ALuint *ID);
ALAPI ALvoid ALAPIENTRY alSourceStopv(ALsizei n, ALuint *ID);
ALAPI ALvoid ALAPIENTRY alSourceRewindv (ALsizei n, ALuint *ID);
ALAPI ALvoid ALAPIENTRY alQueuei (ALuint source, ALenum param, ALint value);
ALAPI ALvoid ALAPIENTRY alQueuef (ALuint source, ALenum param, ALfloat value);
ALAPI ALvoid ALAPIENTRY alUnqueue (ALuint source, ALsizei n, ALuint *buffers);

#pragma export off
