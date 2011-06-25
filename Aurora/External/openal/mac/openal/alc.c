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
 
#include <Sound.h>
#include "alc.h"

static ALubyte *alcErrorStr[]=
{
	(unsigned char *)"There is no accessible sound device/driver/server.",
	(unsigned char *)"The Device argument does not name a valid device.",
	(unsigned char *)"The Context argument does not name a valid context.",
	(unsigned char *)"No error.",
};

// ALC-global variables
ALCenum LastError=ALC_NO_ERROR;
ALCdevice *pOpenDevice = 0;
void *pContext = 0;

ALAPI void * ALAPIENTRY alcCreateContext( ALCdevice *dev,  ALint* attrlist )
{
	// if have device pointer and there isn't a context yet, then allow creation of new one
	if ((dev != 0) && (pContext == 0))
	{
		pContext = (void *) dev;
		return pContext;
	} else
	{
		return 0 ;
	}
}

ALAPI ALCenum ALAPIENTRY alcMakeContextCurrent( ALvoid *alcHandle )
{
	return ALC_NO_ERROR;
}

ALAPI void ALAPIENTRY alcProcessContext( ALvoid *alcHandle )
{
}

ALAPI void ALAPIENTRY alcSuspendContext( ALvoid *alcHandle )
{
}

ALAPI ALCenum ALAPIENTRY alcDestroyContext( ALvoid *alcHandle )
{
	if (alcHandle == pContext)
	{
		pContext = 0;
		return ALC_NO_ERROR;
	} else
	{
		LastError = ALC_INVALID_CONTEXT;
		return ALC_INVALID_CONTEXT;
	}
}

ALAPI ALCenum ALAPIENTRY alcGetError( ALvoid )
{
	ALCenum errorCode;

	errorCode=LastError;
	LastError=AL_NO_ERROR;
	return errorCode;
}

ALAPI void * ALAPIENTRY alcGetCurrentContext( ALvoid )
{
	return pContext;
}

ALAPI ALCdevice* ALAPIENTRY alcGetContextsDevice(ALCcontext *context)
{
	if ((context == pContext) && (context != 0))
	{
		return pOpenDevice;
	} else
	{
		LastError = ALC_INVALID_CONTEXT;
		return 0;
	}
}

ALAPI ALCdevice *alcOpenDevice( const ALubyte *tokstr )
{
	// allow opening of one device
	if (pOpenDevice != 0)
	{
		pOpenDevice = (ALCdevice *) NewPtrClear(sizeof(ALCdevice));
		return pOpenDevice;
	} else
	{
		return 0;
	}
}

ALAPI void alcCloseDevice( ALCdevice *dev )
{
	if (dev != 0)
	{
		DisposePtr((char *)pOpenDevice);
	}
}

ALAPI ALboolean ALAPIENTRY alcIsExtensionPresent(ALCdevice *device, ALubyte *extName)
{
	return AL_FALSE;
}

ALAPI ALvoid  * ALAPIENTRY alcGetProcAddress(ALCdevice *device, ALubyte *funcName)
{
	return 0;
}

ALAPI ALenum ALAPIENTRY alcGetEnumValue(ALCdevice *device, ALubyte *enumName)
{
	return AL_NO_ERROR;
}