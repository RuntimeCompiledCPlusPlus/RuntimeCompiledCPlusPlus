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

#include "globals.h" 
#include "alError.h"
#include "alListener.h"
 
// AL_LISTENER functions
ALAPI ALvoid ALAPIENTRY alListenerf (ALenum pname, ALfloat value)
{
	switch(pname) 
	{
		case AL_GAIN:
			gListener.Gain=value;
			break;
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
}

ALAPI ALvoid ALAPIENTRY alListener3f (ALenum pname, ALfloat v1, ALfloat v2, ALfloat v3)
{
	switch(pname) 
	{
		case AL_POSITION:
			gListener.Position[0]=v1;
			gListener.Position[1]=v2;
			gListener.Position[2]=v3;
			break;
		case AL_VELOCITY:
			gListener.Velocity[0]=v1;
			gListener.Velocity[1]=v2;
			gListener.Velocity[2]=v3;
			break;
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
}

ALAPI ALvoid ALAPIENTRY alListenerfv (ALenum pname, ALfloat *values)
{
	switch(pname) 
	{
		case AL_POSITION:
			gListener.Position[0]=values[0];
			gListener.Position[1]=values[1];
			gListener.Position[2]=values[2];
			break;
		case AL_VELOCITY:
			gListener.Velocity[0]=values[0];
			gListener.Velocity[1]=values[1];
			gListener.Velocity[2]=values[2];
			break;
		case AL_ORIENTATION:
			gListener.Forward[0]=values[0];
			gListener.Forward[1]=values[1];
			gListener.Forward[2]=values[2];
			gListener.Up[0]=values[3]; 
			gListener.Up[1]=values[4];
			gListener.Up[2]=values[5];
			break;
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
}

ALAPI ALvoid ALAPIENTRY alListeneri (ALenum pname, ALint value)
{		
	alSetError(AL_INVALID_ENUM);
}
