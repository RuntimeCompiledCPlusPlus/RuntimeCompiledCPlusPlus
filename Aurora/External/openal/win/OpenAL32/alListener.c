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

#include "include\alMain.h"
#include "include\alError.h"
#include "include\alListener.h"

ALAPI ALvoid ALAPIENTRY alListenerf(ALenum pname,ALfloat value)
{
	ALCcontext *Context;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch(pname) 
	{
		case AL_GAIN:
			Context->Listener.Gain=value;
			Context->Listener.update1 |= LVOLUME;
			alcUpdateContext(Context, ALLISTENER, 0);
			break;
		default:
			alSetError(AL_INVALID_OPERATION);
			break;
	}
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alListener3f(ALenum pname,ALfloat v1,ALfloat v2,ALfloat v3)
{
	ALCcontext *Context;
	
	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch(pname) 
	{
		case AL_POSITION:
			Context->Listener.Position[0]=v1;
			Context->Listener.Position[1]=v2;
			Context->Listener.Position[2]=v3;
			Context->Listener.update1 |= LPOSITION;
			alcUpdateContext(Context, ALLISTENER, 0);
			break;
		case AL_VELOCITY:
			Context->Listener.Velocity[0]=v1;
			Context->Listener.Velocity[1]=v2;
			Context->Listener.Velocity[2]=v3;
			Context->Listener.update1 |= LVELOCITY;
			alcUpdateContext(Context, ALLISTENER, 0);
			break;
		default:
			alSetError(AL_INVALID_OPERATION);
			break;
	}

	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alListenerfv(ALenum pname,ALfloat *values)
{
	ALCcontext *Context;
	
	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch(pname) 
	{
		case AL_POSITION:
			Context->Listener.Position[0]=values[0];
			Context->Listener.Position[1]=values[1];
			Context->Listener.Position[2]=values[2];
			Context->Listener.update1 |= LPOSITION;
			alcUpdateContext(Context, ALLISTENER, 0);
			break;
		case AL_VELOCITY:
			Context->Listener.Velocity[0]=values[0];
			Context->Listener.Velocity[1]=values[1];
			Context->Listener.Velocity[2]=values[2];
			Context->Listener.update1 |= LVELOCITY;
			alcUpdateContext(Context, ALLISTENER, 0);
			break;
		case AL_ORIENTATION:
/*			// UP then AT
			Context->Listener.Forward[0]=values[3];
			Context->Listener.Forward[1]=values[4];
			Context->Listener.Forward[2]=values[5];
			Context->Listener.Up[0]=values[0]; 
			Context->Listener.Up[1]=values[1];
			Context->Listener.Up[2]=values[2];
*/
			// AT then UP
			Context->Listener.Forward[0]=values[0];
			Context->Listener.Forward[1]=values[1];
			Context->Listener.Forward[2]=values[2];
			Context->Listener.Up[0]=values[3]; 
			Context->Listener.Up[1]=values[4];
			Context->Listener.Up[2]=values[5];

			Context->Listener.update1 |= LORIENTATION;
			alcUpdateContext(Context, ALLISTENER, 0);
			break;
		default:
			alSetError(AL_INVALID_OPERATION);
			break;
	}

    alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alListeneri(ALenum pname,ALint value)
{
	ALCcontext *Context;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch(pname) 
	{
		default:
			alSetError(AL_INVALID_OPERATION);
			break;
	}
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alGetListenerf(ALenum pname,ALfloat *value)
{
	ALCcontext *Context;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch(pname)
	{
		case AL_GAIN:
			*value=Context->Listener.Gain;
			break;
		default:
			alSetError(AL_INVALID_OPERATION);
			break;
	}
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alGetListener3f(ALenum pname,ALfloat *v1,ALfloat *v2,ALfloat *v3)
{
	ALCcontext *Context;
	
	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch(pname) 
	{
		case AL_POSITION:
			*v1=Context->Listener.Position[0];
			*v2=Context->Listener.Position[1];
			*v3=Context->Listener.Position[2];
			break;
		case AL_VELOCITY:
			*v1=Context->Listener.Velocity[0];
			*v2=Context->Listener.Velocity[1];
			*v3=Context->Listener.Velocity[2];
			break;
		default:
			alSetError(AL_INVALID_OPERATION);
			break;
	}
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alGetListenerfv(ALenum pname,ALfloat *values)
{
	ALCcontext *Context;
	
	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch(pname) 
	{
		case AL_POSITION:
			values[0]=Context->Listener.Position[0];
			values[1]=Context->Listener.Position[1];
			values[2]=Context->Listener.Position[2];
			break;
		case AL_VELOCITY:
			values[0]=Context->Listener.Velocity[0];
			values[1]=Context->Listener.Velocity[1];
			values[2]=Context->Listener.Velocity[2];
			break;
		case AL_ORIENTATION:
/*			// UP then AT
			values[0]=Context->Listener.Up[0]; 
			values[1]=Context->Listener.Up[1];
			values[2]=Context->Listener.Up[2];
			values[3]=Context->Listener.Forward[0];
			values[4]=Context->Listener.Forward[1];
			values[5]=Context->Listener.Forward[2];
*/
			// AT then UP
			values[3]=Context->Listener.Up[0]; 
			values[4]=Context->Listener.Up[1];
			values[5]=Context->Listener.Up[2];
			values[0]=Context->Listener.Forward[0];
			values[1]=Context->Listener.Forward[1];
			values[2]=Context->Listener.Forward[2];

			break;
		default:
			alSetError(AL_INVALID_OPERATION);
			break;
	}
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alGetListeneri(ALenum pname,ALint *value)
{
	ALCcontext *Context;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch(pname) 
	{
		default:
			alSetError(AL_INVALID_OPERATION);
			break;
	}
	alcProcessContext(Context);
}
