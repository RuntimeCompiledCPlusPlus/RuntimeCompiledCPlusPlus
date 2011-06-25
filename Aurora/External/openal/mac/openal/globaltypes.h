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
 
#include "altypes.h"
 
typedef struct ALbuffer_struct 
{
	ALenum		format;
	ALshort *	data;
	ALsizei		size;
	ALsizei		frequency;
	ALsizei     bits;
	ALsizei     channels;
} ALbuffer;

typedef struct QueueEntry_struct
{
	ALint		bufferNum;
	ALboolean   processed;
	ALenum      loopDirection;
	ALfloat		pitch;
	ALfloat     gain;
	void  *pNext;
} QueueEntry;

typedef struct ALsource_struct
{
	SndChannelPtr channelPtr;
    ALint         srcBufferNum;
    ALuint 		  readOffset; // playback position (read position)
    ALuint        writeOffset; // write position
    ALuint        state;
    ALuint        looping;
    ALfloat       pitch;
    ALfloat 	  gain;
    ALfloat       maxDistance;
    ALfloat       minGain;
    ALfloat       maxGain;
    ALfloat       rolloffFactor;
    ALfloat       referenceDistance;
    ALfloat       Position[3];
    ALfloat       Velocity[3];
    QueueEntry    *ptrQueue;
} ALsource;

typedef struct ALlistener_struct
{
	ALfloat		Position[3];
	ALfloat		Velocity[3];
	ALfloat		Forward[3];
	ALfloat		Up[3];
	ALfloat		Gain;
	ALint		Environment;
} ALlistener;

#define AL_MAXBUFFERS 1024
#define AL_MAXSOURCES 256 
#define AL_INTERNAL_BUFFERS_SIZE 8000

#define ALAPI
#define ALAPIENTRY
