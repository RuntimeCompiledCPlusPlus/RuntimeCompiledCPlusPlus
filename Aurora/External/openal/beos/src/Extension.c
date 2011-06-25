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

#include <AL/al.h>
#include <string.h>
#include "State.h"
#include "Extension.h"

const ALvoid * alimGetProcAddress(ALcontext *context, const ALubyte *name)
{
	typedef ALvoid *ALfunction;

	static const struct {
		const ALubyte *name;
		ALfunction address;
	} extensions[] = {
		{ "alGenEnvironmentIASIG", (ALfunction) alGenEnvironmentIASIG },
		{ "alDeleteEnvironmentIASIG", (ALfunction) alDeleteEnvironmentIASIG },
		{ "alIsEnvironmentIASIG", (ALfunction) alIsEnvironmentIASIG },
		{ "alEnvironmentiIASIG", (ALfunction) alEnvironmentiIASIG },
		{ "alEnvironmentfIASIG", (ALfunction) alEnvironmentfIASIG },
		{ "alGetEnvironmentiIASIG", (ALfunction) alGetEnvironmentiIASIG },
		{ "alGetEnvironmentfIASIG", (ALfunction) alGetEnvironmentfIASIG },
		{ NULL, NULL }
	};

	ALsizei i;

	if (name == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetProcAddress");
		return NULL;
	}

	for (i = 0; extensions[i].name != NULL; i++) {
		if (strcmp(name, extensions[i].name) == 0)
			break;
	}
	return extensions[i].address;
}

ALenum alimGetEnumValue(ALcontext *context, const ALubyte *name)
{
	if (name == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetEnumValue");
		return AL_INVALID;
	}

	return AL_NO_ERROR;
}
