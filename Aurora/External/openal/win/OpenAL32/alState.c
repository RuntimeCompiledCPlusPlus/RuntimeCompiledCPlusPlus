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

#include <stdlib.h>
#include "include\alMain.h"
#include "include\alError.h"
#include "include\alState.h"

ALubyte alVendor[]="Creative Labs Inc.";
ALubyte alVersion[]="OpenAL 1.0";
ALubyte alRenderer[]="Software";
ALubyte alExtensions[]="IASIG";

ALAPI ALvoid ALAPIENTRY alEnable(ALenum capability)
{
	ALCcontext *Context;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch (capability)
	{
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alDisable(ALenum capability)
{
	ALCcontext *Context;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch (capability)
	{
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
	alcProcessContext(Context);
}

ALAPI ALboolean ALAPIENTRY alIsEnabled(ALenum capability)
{
	ALCcontext *Context;
	ALboolean value=AL_FALSE;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch (capability)
	{
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
	alcProcessContext(Context);
	return value;
}

ALAPI ALboolean ALAPIENTRY alGetBoolean(ALenum pname)
{
	ALCcontext *Context;
	ALboolean value=AL_FALSE;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch (pname)
	{
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
	alcProcessContext(Context);
	return value;
}

ALAPI ALdouble ALAPIENTRY alGetDouble(ALenum pname)
{
	ALCcontext *Context;
	ALdouble value=0.0;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch (pname)
	{
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
	alcProcessContext(Context);
	return value;
}

ALAPI ALfloat ALAPIENTRY alGetFloat(ALenum pname)
{
	ALCcontext *Context;
	ALfloat value=0.0f;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch (pname)
	{
		case AL_DOPPLER_FACTOR:
			value=Context->DopplerFactor;
			break;
		case AL_DOPPLER_VELOCITY:
			value=Context->DopplerVelocity;
			break;
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
	alcProcessContext(Context);
	return value;
}

ALAPI ALint ALAPIENTRY alGetInteger(ALenum pname)
{
	ALCcontext *Context;
	ALint value=0;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch (pname)
	{
		case AL_DISTANCE_MODEL:
			value=Context->DistanceModel;
			break;
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
	alcProcessContext(Context);
	return value;
}

ALAPI ALvoid ALAPIENTRY alGetBooleanv(ALenum pname,ALboolean *data)
{
	ALCcontext *Context;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch (pname)
	{
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alGetDoublev(ALenum pname,ALdouble *data)
{
	ALCcontext *Context;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch (pname)
	{
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alGetFloatv(ALenum pname,ALfloat *data)
{
	ALCcontext *Context;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch (pname)
	{
		case AL_DOPPLER_FACTOR:
			*data=Context->DopplerFactor;
			break;
		case AL_DOPPLER_VELOCITY:
			*data=Context->DopplerVelocity;
			break;
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alGetIntegerv(ALenum pname,ALint *data)
{
	ALCcontext *Context;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch (pname)
	{
		case AL_DISTANCE_MODEL:
			*data=Context->DistanceModel;
			break;
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
	alcProcessContext(Context);
}

ALAPI ALubyte * ALAPIENTRY alGetString(ALenum pname)
{
	ALCcontext *Context;
	ALubyte *value;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch(pname)
	{
		case AL_VENDOR:
			value=alVendor;
			break;
		case AL_VERSION:
			value=alVersion;
			break;
		case AL_RENDERER:
			value=alRenderer;
			break;
		case AL_EXTENSIONS:
			value=alExtensions;
			break;
		default:
			alSetError(AL_INVALID_VALUE);
			break;
	}
	alcProcessContext(Context);
	return value;
}

ALAPI ALvoid ALAPIENTRY alDopplerFactor(ALfloat value)
{
	ALCcontext *Context;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	if (value>=0.0f)
	{
		Context->DopplerFactor=value;
		Context->Listener.update1 = LDOPPLERFACTOR;
		alcUpdateContext(Context, ALLISTENER, 0);
	}
	else
		alSetError(AL_INVALID_VALUE);
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alDopplerVelocity(ALfloat value)
{
	ALCcontext *Context;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	if (value>0.0f)
	{
		Context->DopplerVelocity=value;
		Context->Listener.update1 = LDOPPLERVELOCITY;
		alcUpdateContext(Context, 0, 0);
	}
	else
		alSetError(AL_INVALID_VALUE);
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alDistanceModel(ALenum value)
{
	ALCcontext *Context;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	switch (value)
	{
		case AL_NONE:
		case AL_INVERSE_DISTANCE:
		case AL_INVERSE_DISTANCE_CLAMPED:
			Context->DistanceModel=value;
			Context->Listener.update1 = LROLLOFFFACTOR;
			alcUpdateContext(Context, ALLISTENER, 0);
			break;
		default:
			alSetError(AL_INVALID_VALUE);
			break;
	}
	alcProcessContext(Context);
}
