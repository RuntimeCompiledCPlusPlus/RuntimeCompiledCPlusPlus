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
#include "alState.h"

ALAPI ALvoid ALAPIENTRY alEnable (ALenum capability)
{
}

ALAPI ALvoid ALAPIENTRY alDisable (ALenum capability)
{
}

ALAPI ALboolean ALAPIENTRY alIsEnabled(ALenum capability)
{
	return AL_FALSE;
}

ALAPI ALboolean ALAPIENTRY alGetBoolean (ALenum pname)
{
	return AL_FALSE;
}

ALAPI ALdouble ALAPIENTRY alGetDouble (ALenum pname)
{
	return 0;
}

ALAPI ALfloat ALAPIENTRY alGetFloat (ALenum pname)
{
	return 0.0f;
}

ALAPI ALint ALAPIENTRY alGetInteger (ALenum pname)
{
	return 0;
}

ALAPI ALvoid ALAPIENTRY alGetBooleanv (ALenum pname, ALboolean *data)
{
}

ALAPI ALvoid ALAPIENTRY alGetDoublev (ALenum pname, ALdouble *data)
{
}

ALAPI ALvoid ALAPIENTRY alGetFloatv (ALenum pname, ALfloat *data)
{
}

ALAPI ALvoid ALAPIENTRY alGetIntegerv (ALenum pname, ALint *data)
{
}

ALAPI const ALubyte * ALAPIENTRY alGetString (ALenum pname)
{
	switch(pname)
	{
		case AL_VENDOR:
			return (const ALubyte *)alVendor;
		case AL_VERSION:
			return (const ALubyte *)alVersion;
		case AL_RENDERER:
			return (const ALubyte *)alRenderer;
		case AL_EXTENSIONS:
			return (const ALubyte *)alExtensions;
		case AL_NO_ERROR:
			return (const ALubyte *)alNoError;
		case AL_INVALID_NAME:
			return (const ALubyte *)alErrInvalidName;
		case AL_INVALID_ENUM:
			return (const ALubyte *)alErrInvalidEnum;
		case AL_INVALID_VALUE:
			return (const ALubyte *)alErrInvalidValue;
		case AL_INVALID_OPERATION:
			return (const ALubyte *)alErrInvalidOp;
		case AL_OUT_OF_MEMORY:
			return (const ALubyte *)alErrOutOfMemory;
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
	return NULL;
}

ALAPI ALvoid ALAPIENTRY alDopplerFactor (ALfloat value)
{
	if (value >= 0.0f)
	{
		gDopplerFactor = value;
	} else
	{
		alSetError(AL_INVALID_VALUE);
	}
}

ALAPI ALvoid ALAPIENTRY alDopplerVelocity (ALfloat value)
{
	if (value > 0.0f)
	{
		gDopplerVelocity = value;
	} else
	{
		alSetError(AL_INVALID_VALUE);
	}
}

ALAPI ALvoid ALAPIENTRY alDistanceScale (ALfloat value)
{
	if (value > 0.0f)
	{
		gDistanceScale = value;
	} else
	{
		alSetError(AL_INVALID_VALUE);
	}
}

ALAPI ALvoid ALAPIENTRY alPropagationSpeed (ALfloat value)
{
	if (value > 0.0f)
	{
		gPropagationSpeed = value;
	} else
	{
		alSetError(AL_INVALID_VALUE);
	}
}

ALAPI ALvoid ALAPIENTRY	alDistanceModel (ALenum value)
{
	if ((value == AL_NONE) || (value == AL_INVERSE_DISTANCE) || (value == AL_INVERSE_DISTANCE_CLAMPED))
	{
		gDistanceModel = value;
	} else
	{
		alSetError(AL_INVALID_VALUE);
	}
}

