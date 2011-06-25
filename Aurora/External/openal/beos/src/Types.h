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

#ifndef __AL_TYPES_H
#define __AL_TYPES_H

#include <AL/altypes.h>
#include "Thread.h"
#include "Table.h"


typedef struct ALplayer ALplayer;

typedef struct ALmixer ALmixer;

typedef struct ALchannel ALchannel;

typedef struct ALstate ALstate;

typedef struct ALlistener ALlistener;

typedef struct ALbuffer ALbuffer;

typedef struct ALsource ALsource;

typedef struct ALenvironment ALenvironment;

typedef struct ALcontext ALcontext;


struct ALplayer {
	ALvoid 		*device;
	ALuint 		frequency;
	ALenum 		format;
	ALsizei 	size;
	ALvoid		(*hook)(ALcontext *, ALvoid *, ALsizei);
};

struct ALmixer {
	ALmutex		*mutex;
	ALchannel	*channels;
	ALuint		frequency;
	ALenum		format;
	ALsizei		size;
	ALfloat		*buffer;
};

struct ALchannel {
	ALsource	*source;
	ALbuffer	*buffer;
	ALboolean	active;
	ALboolean	dirty;
	ALuint		pitch;
	ALuint		position;
	ALuint		fraction;
	ALfloat		volume[2];
	struct {
		ALfloat	position[3];
		ALfloat	velocity[3];
		ALfloat	distance;
	} dr;
	struct {
		ALfloat pitch;
	} doppler;
	struct {
		ALfloat gain;
	} rolloff;
	struct {
		ALfloat	gain;
	} cone;
	struct {
		ALfloat position[3];
	} pan;
	struct {
		ALfloat	decay;
		ALfloat	sample;
	} muffle;
};

struct ALstate {
	ALenum		error;
};

struct ALlistener {
	ALfloat 	position[3];
	ALfloat		velocity[3];
	ALfloat		orientation[3][3];
	ALfloat		gain;
	ALuint		environment;
};

struct ALsource {
	ALchannel	*channel;
	ALint		coneInnerAngle;
	ALint		coneOuterAngle;
	ALfloat 	coneOuterGain;
	ALfloat 	pitch;
	ALfloat 	gain;
	ALfloat 	position[3];
	ALfloat 	direction[3];
	ALfloat 	velocity[3];
	ALboolean	looping;
	ALboolean	streaming;
	ALint		buffer;
	ALuint		environment;
};

struct ALbuffer {
	ALenum		format;
	ALuint		frequency;
	ALuint		bits;
	ALuint		channels;
	ALsizei 	size;
	ALvoid 		*data;
};

struct ALenvironment {
	ALfloat 	room;
	ALfloat 	roomHighFrequency;
	ALfloat		decayTime;
	ALfloat		decayHighFrequencyRatio;
	ALfloat		reflections;
	ALfloat		reflectionsDelay;
	ALfloat		reverb;
	ALfloat		reverbDelay;
	ALfloat		diffusion;
	ALfloat		density;
	ALfloat		highFrequencyReference;
};

struct ALcontext {
	ALmutex		*mutex;
	ALplayer	*player;
	ALmixer		*mixer;
	ALstate		*state;
	ALlistener	*listener;
	ALtable		*sources;
	ALtable		*buffers;
	ALtable		*environments;
};

#endif
