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

#ifndef NDEBUG
#include <stdio.h>
#endif
#include "Memory.h"
#include "Context.h"
#include "State.h"

ALstate *alimCreateState(ALvoid)
{
	ALstate *state = (ALstate *) alimMemAlloc(sizeof(ALstate));

	if (state != NULL) {
		state->error = AL_NO_ERROR;
	}
	return state;
}

ALvoid alimDeleteState(ALstate *state)
{
	if (state != NULL) {
		alimMemFree(state);
	}
}

ALvoid alimEnable(ALcontext *context, ALenum capability)
{
	ALstate *state = alimContextState(context);

	if (state != NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alEnable");
		return;
	}

	switch (capability) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alEnable");
		break;
	}
}

ALvoid alimDisable(ALcontext *context, ALenum capability)
{
	ALstate *state = alimContextState(context);

	if (state != NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alDisable");
		return;
	}

	switch (capability) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alDisable");
		break;
	}
}

ALboolean alimIsEnabled(ALcontext *context, ALenum capability)
{
	ALstate *state = alimContextState(context);

	if (state != NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alIsEnabled");
		return AL_FALSE;
	}

	switch (capability) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alIsEnabled");
		return AL_FALSE;
	}
}

ALboolean alimGetBoolean(ALcontext *context, ALenum pname)
{
	ALstate *state = alimContextState(context);

	if (state != NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetBoolean");
		return AL_FALSE;
	}

	switch (pname) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetBoolean");
		return AL_FALSE;
	}
}

ALdouble alimGetDouble(ALcontext *context, ALenum pname)
{
	ALstate *state = alimContextState(context);

	if (state != NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetDouble");
		return 0.0;
	}

	switch (pname) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetDouble");
		return 0.0;
	}
}

ALfloat alimGetFloat(ALcontext *context, ALenum pname)
{
	ALstate *state = alimContextState(context);

	if (state != NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetFloat");
		return 0.0f;
	}

	switch (pname) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetFloat");
		return 0.0f;
	}
}

ALint alimGetInteger(ALcontext *context, ALenum pname)
{
	ALstate *state = alimContextState(context);

	if (state != NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetInteger");
		return 0;
	}

	switch (pname) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetInteger");
		return 0;
	}
}

ALvoid alimGetBooleanv(ALcontext *context, ALenum pname, ALboolean *data)
{
	ALstate *state = alimContextState(context);

	if (state != NULL || data == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetBooleanv");
		return;
	}

	switch (pname) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetBooleanv");
		break;
	}
}

ALvoid alimGetDoublev(ALcontext *context, ALenum pname, ALdouble *data)
{
	ALstate *state = alimContextState(context);

	if (state != NULL || data == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetDoublev");
		return;
	}

	switch (pname) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetDoublev");
		break;
	}
}

ALvoid alimGetFloatv(ALcontext *context, ALenum pname, ALfloat *data)
{
	ALstate *state = alimContextState(context);

	if (state != NULL || data == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetFloatv");
		return;
	}

	switch (pname) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetFloatv");
		break;
	}
}

ALvoid alimGetIntegerv(ALcontext *context, ALenum pname, ALint *data)
{
	ALstate *state = alimContextState(context);

	if (state != NULL || data == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetIntegerv");
		return;
	}

	switch (pname) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetIntegerv");
		break;
	}
}

const ALubyte *alimGetString(ALcontext *context, ALenum pname)
{
	ALstate *state = alimContextState(context);

	if (state != NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetString");
		return NULL;
	}

	switch (pname) {
	case AL_VENDOR:
		return "Carlos Hasan";

	case AL_VERSION:
		return "0.1";

	case AL_RENDERER:
		return "Software";

	case AL_EXTENSIONS:
		return "IASIG";

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetString");
		return NULL;
	}
}

ALenum alimGetError(ALcontext *context)
{
	ALstate *state = alimContextState(context);
	ALenum error;

	if (state == NULL)
		return AL_INVALID;

	error = state->error;
	state->error = AL_NO_ERROR;

	return error;
}

ALvoid alimSetError(ALcontext *context, ALenum error, const ALubyte *where)
{
	ALstate *state = alimContextState(context);

	if (state != NULL) {
		if (state->error == AL_NO_ERROR)
			state->error = error;
	}

#ifndef NDEBUG
	fprintf(stderr, "OpenAL: %s in %s\n",
		alimGetErrorString(context, error), where);
#endif

}

const ALubyte *alimGetErrorString(ALcontext *context, ALenum error)
{
	switch (error) {
	case AL_ILLEGAL_VALUE:
		return "Illegal value passed as argument";

	case AL_ILLEGAL_OPERATION:
		return "Illegal operation";

	case AL_OUT_OF_MEMORY:
		return "Out of memory";

	case AL_NO_ERROR:
		return "No error";

	default:
		return "Unknown";
	}
}
