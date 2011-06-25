/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_ext_iasig.c
 *
 * Implementation of the IASIG extension.  This file will be moved soon.
 *
 */
#include "al_siteconfig.h"
#include "al_types.h"
#include "al_main.h"
#include "al_debug.h"
#include "al_error.h"

#include <AL/al.h>

#include <stdio.h>

/* env property set */
typedef struct _EnvironmentIASIG {
	ALint   room;
	ALint   room_hf;
	ALfloat room_rolloff_factor;

	ALfloat decay_time;
	ALfloat decay_hf_ratio;

	ALint   reflections;
	ALfloat reflections_delay;
	
	ALint   reverb;
	ALfloat reverb_delay;

	ALfloat diffusion;
	ALfloat density;

	ALfloat hf_reference;
} EnvironmentIASIG;

typedef struct _ei_node {
	struct _ei_node *next;
	EnvironmentIASIG *env;

	ALuint id;
} ei_node;

/* static function prototype */
static ei_node *_alGetEI(ALuint eid);
static void _alEnvironmentIASIGInit(EnvironmentIASIG *env);

/* static data */
static UNUSED(ei_node *ei_head) = NULL;

/**
 * Allocate n environment ids and store them in the array environs.
 * Returns the number of environments actually allocated.
 *
 *
 * FIXME: implement
 */
ALsizei alGenEnvironmentIASIG(UNUSED(ALsizei n), UNUSED(ALuint* environs) ) {
	_alStub("alGenEnvironmentIASIG");

	_alEnvironmentIASIGInit(NULL);
	return -1;
}

void alDeleteEnvironmentIASIG(UNUSED(ALsizei n), UNUSED(ALuint* environs) ) {
	_alStub("alDeleteEnvironmentIASIG");

	return;

}

ALboolean alIsEnvironmentIASIG( ALuint eid ) {
	ei_node *itr = _alGetEI(eid);

	if(itr == NULL) {
		return AL_FALSE;
	}
	

	return AL_TRUE;
}

/*
 Int:
	AL_ENV_ROOM_IASIG;
	AL_ENV_ROOM_HIGH_FREQUENCY_IASIG;
	AL_ENV_REFLECTIONS_IASIG;
	AL_ENV_REVERB_IASIG;

 Float
	AL_ENV_ROOM_HIGH_FACTOR_IASIG;
	AL_ENV_DECAY_TIME_IASIG;
	AL_ENV_DECAY_HIGH_FREQUENCY_IASIG;
	AL_ENV_REFLECTIONS_DELAY_IASIG;
	AL_ENV_REVERB_DELAY_IASIG;
	AL_ENV_DIFFUSION_IASIG;
	AL_ENV_DENSITY_IASIG;
	AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG;
 */

void alEnvironmentiIASIG( ALuint id, ALenum param, ALint value ) {
	ei_node *itr = _alGetEI(id);
	EnvironmentIASIG *env = itr->env;

	if(itr == NULL) {
		_alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
		      "alEnvironmentIASIG: invalid id %d",
		      id);

		_alDCSetError(AL_INVALID_NAME);
		return;
	}

	switch(param) {
		case AL_ENV_ROOM_IASIG:
		  env->room        = value;	
		  break;
		case AL_ENV_ROOM_HIGH_FREQUENCY_IASIG:
		  env->room_hf     = value;
		  break;
		case AL_ENV_REFLECTIONS_IASIG:
		  env->reflections = value;
		  break;
		default:
		  _alDebug(ALD_MAXIMUS, __FILE__, __LINE__,
			"alEnvironmentIASIG: invalid param 0x%x", param);

		  _alDCSetError(AL_ILLEGAL_ENUM);
		  break;
	}

	return;
}

void alEnvironmentfIASIG(UNUSED(ALuint id), UNUSED(ALenum param), UNUSED(ALuint value )) {
	_alStub("alEnvironmentfIASIG");

	return;
}

static void _alEnvironmentIASIGInit(EnvironmentIASIG *env) {
	if(env == NULL) {
		return;
	}

	env->room                 = -10000;
	env->room_hf              = 0;
	env->room_rolloff_factor  = 0.0;

	env->decay_time           = 1.0;
	env->decay_hf_ratio       = 0.5;

	env->reflections          = -10000;
	env->reflections_delay    = 0.02;
	
	env->reverb               = -10000;
	env->reverb_delay         = 0.04;

	env->diffusion            = 100;
	env->density              = 100;

	env->hf_reference         = 5000;

	return;
}

ei_node *_alGetEI(UNUSED(ALuint eid)) {

	return NULL;
}
