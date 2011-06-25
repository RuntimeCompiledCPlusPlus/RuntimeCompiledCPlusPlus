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

#include "State.h"
#include "Memory.h"
#include "Context.h"
#include "Listener.h"
#include "Mixer.h"

ALlistener *alimCreateListener(ALvoid)
{
	ALlistener *listener = (ALlistener *) alimMemAlloc(sizeof(ALlistener));

	if (listener != NULL) {
		listener->position[0] = 0.0f;
		listener->position[1] = 0.0f;
		listener->position[2] = 0.0f;

		listener->velocity[0] = 0.0f;
		listener->velocity[1] = 0.0f;
		listener->velocity[2] = 0.0f;

		listener->orientation[0][0] = -1.0f;
		listener->orientation[0][1] = 0.0f;
		listener->orientation[0][2] = 0.0f;

		listener->orientation[1][0] = 0.0f;
		listener->orientation[1][1] = 1.0f;
		listener->orientation[1][2] = 0.0f;

		listener->orientation[2][0] = 0.0f;
		listener->orientation[2][1] = 0.0f;
		listener->orientation[2][2] = 1.0f;

		listener->gain = 1.0f;

		listener->environment = 0;
	}
	return listener;
}

ALvoid alimDeleteListener(ALlistener *listener)
{
	if (listener != NULL) {
		alimMemFree(listener);
	}
}

ALvoid alimListenerf(ALcontext *context, ALenum pname, ALfloat value)
{
	ALlistener *listener = alimContextListener(context);

	if (listener == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alListenerf");
		return;
	}

	switch (pname) {
	case AL_GAIN:
		if (value >= 0.0f)
			listener->gain = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alListenerf(AL_GAIN)");
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alListenerf");
		break;
	}

	alimMixerUpdateListener(context);
}

ALvoid alimListener3f(ALcontext *context, ALenum pname, ALfloat value1, ALfloat value2, ALfloat value3)
{
	ALlistener *listener = alimContextListener(context);

	if (listener == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alListener3f");
		return;
	}

	switch (pname) {
	case AL_POSITION:
		listener->position[0] = value1;
		listener->position[1] = value2;
		listener->position[2] = value3;
		break;

	case AL_VELOCITY:
		listener->velocity[0] = value1;
		listener->velocity[1] = value2;
		listener->velocity[2] = value3;
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alListener3f");
		break;
	}

	alimMixerUpdateListener(context);
}

ALvoid alimListenerfv(ALcontext *context, ALenum pname, const ALfloat *values)
{
	ALlistener *listener = alimContextListener(context);

	if (listener == NULL || values == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alListenerfv");
		return;
	}

	switch (pname) {
	case AL_POSITION:
		listener->position[0] = values[0];
		listener->position[1] = values[1];
		listener->position[2] = values[2];
		break;

	case AL_VELOCITY:
		listener->velocity[0] = values[0];
		listener->velocity[1] = values[1];
		listener->velocity[2] = values[2];
		break;

	case AL_ORIENTATION:
		listener->orientation[2][0] = values[0];
		listener->orientation[2][1] = values[1];
		listener->orientation[2][2] = values[2];

		listener->orientation[1][0] = values[3];
		listener->orientation[1][1] = values[4];
		listener->orientation[1][2] = values[5];

		listener->orientation[0][0] = values[1] * values[5] - values[2] * values[4];
		listener->orientation[0][1] = values[2] * values[3] - values[0] * values[5];
		listener->orientation[0][2] = values[0] * values[4] - values[1] * values[3];
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alListenerfv");
		break;
	}

	alimMixerUpdateListener(context);
}

ALvoid alimListeneri(ALcontext *context, ALenum pname, ALint value)
{
	ALlistener *listener = alimContextListener(context);

	if (listener == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alListeneri");
		return;
	}

	switch (pname) {
	case AL_ENVIRONMENT_IASIG:
		if (alimIsEnvironmentIASIG(context, value))
			listener->environment = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alListeneri(AL_ENVIRONMENT_IASIG)");
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alListeneri");
		break;
	}

	alimMixerUpdateListener(context);
}

ALvoid alimGetListenerf(ALcontext *context, ALenum pname, ALfloat *value)
{
	ALlistener *listener = alimContextListener(context);

	if (listener == NULL || value == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetListenerf");
		return;
	}

	switch (pname) {
	case AL_GAIN:
		*value = listener->gain;
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetListenerf");
		break;
	}
}

ALvoid alimGetListener3f(ALcontext *context, ALenum pname, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
	ALlistener *listener = alimContextListener(context);

	if (listener == NULL || value1 == NULL || value2 == NULL || value3 == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetListener3f");
		return;
	}

	switch (pname) {
	case AL_POSITION:
		*value1 = listener->position[0];
		*value2 = listener->position[1];
		*value3 = listener->position[2];
		break;

	case AL_VELOCITY:
		*value1 = listener->velocity[0];
		*value2 = listener->velocity[1];
		*value3 = listener->velocity[2];
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetListener3f");
		break;
	}
}

ALvoid alimGetListenerfv(ALcontext *context, ALenum pname, ALfloat *values)
{
	ALlistener *listener = alimContextListener(context);

	if (listener == NULL || values == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetListenerfv");
		return;
	}

	switch (pname) {
	case AL_POSITION:
		values[0] = listener->position[0];
		values[1] = listener->position[1];
		values[2] = listener->position[2];
		break;

	case AL_VELOCITY:
		values[0] = listener->velocity[0];
		values[1] = listener->velocity[1];
		values[2] = listener->velocity[2];
		break;

	case AL_ORIENTATION:
		values[0] = listener->orientation[2][0];
		values[1] = listener->orientation[2][1];
		values[2] = listener->orientation[2][2];

		values[3] = listener->orientation[1][0];
		values[4] = listener->orientation[1][1];
		values[5] = listener->orientation[1][2];
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetListenerfv");
		break;
	}
}

ALvoid alimGetListeneri(ALcontext *context, ALenum pname, ALint *value)
{
	ALlistener *listener = alimContextListener(context);

	if (listener == NULL || value == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetListeneri");
		return;
	}

	switch (pname) {
	case AL_ENVIRONMENT_IASIG:
		*value = listener->environment;
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetListeneri");
		break;
	}
}
