/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_propagation.c
 *
 * Propagation tweakage
 */
#include <AL/al.h>

#include "al_propagation.h"
#include "al_error.h"
#include "al_main.h"
#include "alc/alc_context.h"
#include "al_siteconfig.h"

#include <stdlib.h>

#define MIN_PROPAGATION_SPEED 0.001
#define MAX_PROPAGATION_SPEED 40000

void alPropagationSpeed(ALfloat value) {
	_alcDCLockContext();

	_alPropagationSpeed(value);

	_alcDCUnlockContext();

	return;
}

/*
 * Assumes locked context.
 */
void _alPropagationSpeed(ALfloat value) {
	AL_context *cc;
	ALboolean inrange;

	inrange = _alCheckRangef(value,
				 MIN_PROPAGATION_SPEED,
				 MAX_PROPAGATION_SPEED);
	if(inrange == AL_FALSE) {
		_alDCSetError(AL_INVALID_VALUE);
		return;
	}

	cc = _alcDCGetContext();
	if(cc == NULL) {
		return;
	}

	cc->propagation_speed = value;

	return;
}
