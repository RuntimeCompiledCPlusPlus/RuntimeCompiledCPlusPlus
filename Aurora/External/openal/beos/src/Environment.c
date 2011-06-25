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
#include "Environment.h"

ALenvironment *alimCreateEnvironmentIASIG(ALvoid)
{
	ALenvironment *environment = (ALenvironment *) alimMemAlloc(sizeof(ALenvironment));

	if (environment != NULL) {
		environment->room = 0.0f;
		environment->roomHighFrequency = 1.0f;
		environment->decayTime = 1.0f;
		environment->decayHighFrequencyRatio = 0.5f;
		environment->reflections = 0.0f;
		environment->reflectionsDelay = 0.02f;
		environment->reverb = 0.0f;
		environment->reverbDelay = 0.04f;
		environment->diffusion = 100.0f;
		environment->density = 100.0f;
		environment->highFrequencyReference = 5000.0f;
	}
	return environment;
}

ALvoid alimDeleteEnvironmentIASIG(ALenvironment *environment)
{
	if (environment != NULL) {
		alimMemFree(environment);
	}
}

ALvoid alimEnvironmentfIASIG(ALcontext *context, ALuint handle, ALenum pname, ALfloat value)
{
	ALenvironment *environment = alimContextEnvironmentIASIG(context, handle);

	if (environment == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alEnvironmentfIASIG");
		return;
	}

	switch (pname) {
	case AL_ENV_ROOM_IASIG:
		if (value >= 0.0f && value <= 1.0f)
			environment->room = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alEnvironmentfIASIG(AL_ENV_ROOM_IASIG)");
		break;

	case AL_ENV_ROOM_HIGH_FREQUENCY_IASIG:
		if (value >= 0.0 && value <= 1.0f)
			environment->roomHighFrequency = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alEnvironmentfIASIG(AL_ENV_ROOM_HIGH_FREQUENCY_IASIG)");
		break;

	case AL_ENV_DECAY_TIME_IASIG:
		if (value >= 0.1f && value <= 20.0f)
			environment->decayTime = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alEnvironmentfIASIG(AL_ENV_DECAY_TIME_IASIG)");
		break;

	case AL_ENV_DECAY_HIGH_FREQUENCY_RATIO_IASIG:
		if (value >= 0.1f && value <= 2.0f)
			environment->decayHighFrequencyRatio = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alEnvironmentfIASIG(AL_ENV_DECAY_HIGH_FREQUENCY_RATIO)");
		break;

	case AL_ENV_REFLECTIONS_IASIG:
		if (value >= 0.0f && value <= 3.0f)
			environment->reflections = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alEnvironmentfIASIG(AL_ENV_REFLECTIONS_IASIG)");
		break;

	case AL_ENV_REFLECTIONS_DELAY_IASIG:
		if (value >= 0.0f && value <= 0.3f)
			environment->reflectionsDelay = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alEnvironmentfIASIG(AL_ENV_REFLECTIONS_DELAY_IASIG)");
		break;

	case AL_ENV_REVERB_IASIG:
		if (value >= 0.0f && value <= 10.0f)
			environment->reverb = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alEnvironmentfIASIG(AL_ENV_REVERB_IASIG)");
		break;

	case AL_ENV_REVERB_DELAY_IASIG:
		if (value >= 0.0f && value <= 0.1f)
			environment->reverbDelay = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alEnvironmentfIASIG(AL_ENV_REVERB_DELAY_IASIG)");
		break;

	case AL_ENV_DIFFUSION_IASIG:
		if (value >= 0.0f && value <= 100.0f)
			environment->diffusion = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alEnvironmentfIASIG(AL_ENV_DIFFUSION_IASIG)");
		break;

	case AL_ENV_DENSITY_IASIG:
		if (value >= 0.0f && value <= 100.0f)
			environment->density = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alEnvironmentfIASIG(AL_ENV_DENSITY_IASIG)");
		break;

	case AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG:
		if (value >= 20.0f && value <= 20000.0f)
			environment->highFrequencyReference = value;
		else
			alimSetError(context, AL_ILLEGAL_VALUE, "alEnvironmentfIASIG(AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG)");
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alEnvironmentfIASIG");
		break;
	}
}

ALvoid alimEnvironmentiIASIG(ALcontext *context, ALuint handle, ALenum pname, ALint value)
{
	ALenvironment *environment = alimContextEnvironmentIASIG(context, handle);

	if (environment == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alEnvironmentiIASIG");
		return;
	}

	switch (pname) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alEnvironmentiIASIG");
		break;
	}
}

ALvoid alimGetEnvironmentfIASIG(ALcontext *context, ALuint handle, ALenum pname, ALfloat *value)
{
	ALenvironment *environment = alimContextEnvironmentIASIG(context, handle);

	if (environment == NULL || value == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetEnvironmentfIASIG");
		return;
	}

	switch (pname) {
	case AL_ENV_ROOM_IASIG:
		*value = environment->room;
		break;

	case AL_ENV_ROOM_HIGH_FREQUENCY_IASIG:
		*value = environment->roomHighFrequency;
		break;

	case AL_ENV_DECAY_TIME_IASIG:
		*value = environment->decayTime;
		break;

	case AL_ENV_DECAY_HIGH_FREQUENCY_RATIO_IASIG:
		*value = environment->decayHighFrequencyRatio;
		break;

	case AL_ENV_REFLECTIONS_IASIG:
		*value = environment->reflections;
		break;

	case AL_ENV_REFLECTIONS_DELAY_IASIG:
		*value = environment->reflectionsDelay;
		break;

	case AL_ENV_REVERB_IASIG:
		*value = environment->reverb;
		break;

	case AL_ENV_REVERB_DELAY_IASIG:
		*value = environment->reverbDelay;
		break;

	case AL_ENV_DIFFUSION_IASIG:
		*value = environment->diffusion;
		break;

	case AL_ENV_DENSITY_IASIG:
		*value = environment->density;
		break;

	case AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG:
		*value = environment->highFrequencyReference;
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetEnvironmentfIASIG");
		break;
	}
}

ALvoid alimGetEnvironmentiIASIG(ALcontext *context, ALuint handle, ALenum pname, ALint *value)
{
	ALenvironment *environment = alimContextEnvironmentIASIG(context, handle);

	if (environment == NULL || value == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetEnvironmentiIASIG");
		return;
	}

	switch (pname) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetEnvironmentiIASIG");
		break;
	}
}
