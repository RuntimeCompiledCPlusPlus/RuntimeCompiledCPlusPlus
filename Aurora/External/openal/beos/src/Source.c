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
#include "Buffer.h"
#include "Environment.h"
#include "Source.h"
#include "Mixer.h"


ALsource *alimCreateSource(ALvoid)
{
	ALsource *source = (ALsource *) alimMemAlloc(sizeof(ALsource));

	if (source != NULL) {
		source->channel = NULL;
		source->coneInnerAngle = 360;
		source->coneOuterAngle = 360;
		source->coneOuterGain = 0.0f;
		source->pitch = 1.0f;
		source->gain = 1.0f;
		source->position[0] = source->position[1] = source->position[2] = 0.0f;
		source->direction[0] = source->direction[1] = source->direction[2] = 0.0f;
		source->velocity[0] = source->velocity[1] = source->velocity[2] = 0.0f;
		source->looping = AL_FALSE;
		source->streaming = AL_FALSE;
		source->buffer = 0;
		source->environment = 0;
	}
	return source;
}

ALvoid alimDeleteSource(ALsource *source)
{
	if (source != NULL) {
		alimMemFree(source);
	}
}

ALvoid alimSourcef(ALcontext *context, ALuint handle, ALenum pname, ALfloat value)
{
	ALsource *source = alimContextSource(context, handle);

	if (source == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alSourcef");
		return;
	}

	switch (pname) {
	case AL_GAIN:
		if (value >= 0.0f)
			source->gain = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alSourcef(AL_GAIN)");
		break;

	case AL_PITCH:
		if (value >= 0.5f && value <= 2.0f)
			source->pitch = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alSourcef(AL_PITCH)");
		break;

	case AL_CONE_OUTER_GAIN:
		if (value >= 0.0f)
			source->coneOuterGain = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alSourcef(AL_CONE_OUTER_GAIN)");
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alSourcef");
		break;
	}

	alimMixerUpdateSource(context, source);
}

ALvoid alimSourcefv(ALcontext *context, ALuint handle, ALenum pname, const ALfloat *values)
{
	ALsource *source = alimContextSource(context, handle);

	if (source == NULL || values == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alSourcefv");
		return;
	}

	switch (pname) {
	case AL_POSITION:
		source->position[0] = values[0];
		source->position[1] = values[1];
		source->position[2] = values[2];
		break;

	case AL_DIRECTION:
		source->direction[0] = values[0];
		source->direction[1] = values[1];
		source->direction[2] = values[2];
		break;

	case AL_VELOCITY:
		source->velocity[0] = values[0];
		source->velocity[1] = values[1];
		source->velocity[2] = values[2];
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alSourcefv");
		break;
	}

	alimMixerUpdateSource(context, source);
}

ALvoid alimSource3f(ALcontext *context, ALuint handle, ALenum pname, ALfloat value1, ALfloat value2, ALfloat value3)
{
	ALsource *source = alimContextSource(context, handle);

	if (source == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alSource3f");
		return;
	}

	switch (pname) {
	case AL_POSITION:
		source->position[0] = value1;
		source->position[1] = value2;
		source->position[2] = value3;
		break;

	case AL_DIRECTION:
		source->direction[0] = value1;
		source->direction[1] = value2;
		source->direction[2] = value3;
		break;

	case AL_VELOCITY:
		source->velocity[0] = value1;
		source->velocity[1] = value2;
		source->velocity[2] = value3;
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alSource3f");
		break;
	}

	alimMixerUpdateSource(context, source);
}

ALvoid alimSourcei(ALcontext *context, ALuint handle, ALenum pname, ALint value)
{
	ALsource *source = alimContextSource(context, handle);

	if (source == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alSourcei");
		return;
	}

	switch (pname) {
	case AL_CONE_INNER_ANGLE:
		if (value >= 0 && value <= 360)
			source->coneInnerAngle = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alSourcei(AL_CONE_INNER_ANGLE)");
		break;

	case AL_CONE_OUTER_ANGLE:
		if (value >= 0 && value <= 360)
			source->coneOuterAngle = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alSourcei(AL_CONE_OUTER_ANGLE)");
		break;

	case AL_LOOPING:
		if (value >= AL_FALSE && value <= AL_TRUE)
			source->looping = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alSourcei(AL_LOOPING)");
		break;

	case AL_STREAMING:
		if (value >= AL_FALSE && value <= AL_TRUE)
			source->streaming = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alSourcei(AL_STREAMING)");
		break;

	case AL_BUFFER:
		if (alimIsBuffer(context, value))
			source->buffer = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alSourcei(AL_BUFFER)");
		break;

	case AL_ENVIRONMENT_IASIG:
		if (alimIsEnvironmentIASIG(context, value))
			source->environment = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alSourcei(AL_ENVIRONMENT_IASIG)");
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alSourcei");
		break;
	}

	alimMixerUpdateSource(context, source);
}

ALvoid alimGetSourcef(ALcontext *context, ALuint handle, ALenum pname, ALfloat *value)
{
	ALsource *source = alimContextSource(context, handle);

	if (source == NULL || value == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetSourcef");
		return;
	}

	switch (pname) {
	case AL_GAIN:
		*value = source->gain;
		break;

	case AL_PITCH:
		*value = source->pitch;
		break;

	case AL_CONE_OUTER_GAIN:
		*value = source->coneOuterGain;
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetSourcef");
		break;
	}
}

ALvoid alimGetSourcefv(ALcontext *context, ALuint handle, ALenum pname, ALfloat *values)
{
	ALsource *source = alimContextSource(context, handle);

	if (source == NULL || values == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetSourcefv");
		return;
	}

	switch (pname) {
	case AL_POSITION:
		values[0] = source->position[0];
		values[1] = source->position[1];
		values[2] = source->position[2];
		break;

	case AL_DIRECTION:
		values[0] = source->direction[0];
		values[1] = source->direction[1];
		values[2] = source->direction[2];
		break;

	case AL_VELOCITY:
		values[0] = source->velocity[0];
		values[1] = source->velocity[1];
		values[2] = source->velocity[2];
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetSourcefv");
		break;
	}
}

ALvoid alimGetSourcei(ALcontext *context, ALuint handle, ALenum pname, ALint *value)
{
	ALsource *source = alimContextSource(context, handle);

	if (source == NULL || value == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetSourcei");
		return;
	}

	switch (pname) {
	case AL_CONE_INNER_ANGLE:
		*value = source->coneInnerAngle;
		break;

	case AL_CONE_OUTER_ANGLE:
		*value = source->coneOuterAngle;
		break;

	case AL_LOOPING:
		*value = source->looping;
		break;

	case AL_STREAMING:
		*value = source->streaming;
		break;

	case AL_BUFFER:
		*value = source->buffer;
		break;

	case AL_ENVIRONMENT_IASIG:
		*value = source->environment;
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetSourcei");
		break;
	}
}

ALvoid alimSourcePlay(ALcontext *context, ALuint handle)
{
	ALsource *source = alimContextSource(context, handle);

	if (source == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alSourcePlay");
		return;
	}

	alimMixerStartSource(context, source);
}

ALvoid alimSourcePause(ALcontext *context, ALuint handle)
{
	ALsource *source = alimContextSource(context, handle);

	if (source == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alSourcePause");
		return;
	}

	alimMixerPauseSource(context, source);
}

ALvoid alimSourceStop(ALcontext *context, ALuint handle)
{
	ALsource *source = alimContextSource(context, handle);

	if (source == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alSourceStop");
		return;
	}

	alimMixerStopSource(context, source);
}
