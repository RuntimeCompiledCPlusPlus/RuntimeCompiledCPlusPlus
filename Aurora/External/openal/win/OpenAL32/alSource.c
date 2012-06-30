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

#include <stdlib.h>
#include <math.h>
#include "include\alMain.h"
#include "include\alError.h"
#include "include\alSource.h"


ALAPI ALvoid ALAPIENTRY alGenSources(ALsizei n,ALuint *sources)
{
	ALCcontext *Context;
	ALsource *Source;
	ALsizei i=0;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);

	// Check that the requested number of sources can be generated
	if ((Context->SourceCount + n) > Context->Device->MaxNoOfSources)
	{
		alSetError(AL_INVALID_VALUE);
		alcProcessContext(Context);
		return;
	}

	if (!Context->Source)
	{
		// First Source to be created !
		Context->Source=malloc(sizeof(ALsource));
		if (Context->Source)
		{
			memset(Context->Source,0,sizeof(ALsource));
			sources[i]=(ALuint)Context->Source;
			Context->Source->valid=AL_TRUE;
			Context->Source->update1 |= SGENERATESOURCE;
	
			Context->Source->param[AL_CONE_INNER_ANGLE-AL_CONE_INNER_ANGLE].data.f=360.0;
			Context->Source->param[AL_CONE_INNER_ANGLE-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->param[AL_CONE_OUTER_ANGLE-AL_CONE_INNER_ANGLE].data.f=360.0;
			Context->Source->param[AL_CONE_OUTER_ANGLE-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->param[AL_PITCH-AL_CONE_INNER_ANGLE].data.f= 1.0;
			Context->Source->param[AL_PITCH-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->param[AL_POSITION-AL_CONE_INNER_ANGLE].data.fv3[0]=0.0;
			Context->Source->param[AL_POSITION-AL_CONE_INNER_ANGLE].data.fv3[1]=0.0;
			Context->Source->param[AL_POSITION-AL_CONE_INNER_ANGLE].data.fv3[2]=0.0;
			Context->Source->param[AL_POSITION-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->param[AL_DIRECTION-AL_CONE_INNER_ANGLE].data.fv3[0]=0.0;
			Context->Source->param[AL_DIRECTION-AL_CONE_INNER_ANGLE].data.fv3[1]=0.0;
			Context->Source->param[AL_DIRECTION-AL_CONE_INNER_ANGLE].data.fv3[2]=0.0;
			Context->Source->param[AL_DIRECTION-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->param[AL_VELOCITY-AL_CONE_INNER_ANGLE].data.fv3[0]=0.0;
			Context->Source->param[AL_VELOCITY-AL_CONE_INNER_ANGLE].data.fv3[1]=0.0;
			Context->Source->param[AL_VELOCITY-AL_CONE_INNER_ANGLE].data.fv3[2]=0.0;
			Context->Source->param[AL_VELOCITY-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->param[AL_REFERENCE_DISTANCE-AL_CONE_INNER_ANGLE].data.f= 1.0;
			Context->Source->param[AL_REFERENCE_DISTANCE-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->param[AL_MAX_DISTANCE-AL_CONE_INNER_ANGLE].data.f= 1000000.0;
			Context->Source->param[AL_MAX_DISTANCE-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->param[AL_ROLLOFF_FACTOR-AL_CONE_INNER_ANGLE].data.f= 1.0;
			Context->Source->param[AL_ROLLOFF_FACTOR-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->param[AL_LOOPING-AL_CONE_INNER_ANGLE].data.i= AL_FALSE;
			Context->Source->param[AL_LOOPING-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->param[AL_GAIN-AL_CONE_INNER_ANGLE].data.f= 1.0f;
			Context->Source->param[AL_GAIN-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->param[AL_MIN_GAIN-AL_CONE_INNER_ANGLE].data.f= 0.0f;
			Context->Source->param[AL_MIN_GAIN-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->param[AL_MAX_GAIN-AL_CONE_INNER_ANGLE].data.f= 1.0f;
			Context->Source->param[AL_MAX_GAIN-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->param[AL_CONE_OUTER_GAIN-AL_CONE_INNER_ANGLE].data.f= 1.0f;
			Context->Source->param[AL_CONE_OUTER_GAIN-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->state = AL_INITIAL;

			Context->Source->param[AL_BUFFER-AL_CONE_INNER_ANGLE].data.i= 0;
			Context->Source->param[AL_BUFFER-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Context->Source->update1 |= CONEANGLES | FREQUENCY | POSITION | VELOCITY | ORIENTATION |
				MINDIST | MAXDIST | LOOPED | VOLUME | CONEOUTSIDEVOLUME | STATE;
/*
            Context->Source->eaxBP.lDirect = EAXBUFFER_DEFAULTDIRECT;
            Context->Source->eaxBP.lDirectHF = EAXBUFFER_DEFAULTDIRECTHF;
            Context->Source->eaxBP.lRoom = EAXBUFFER_DEFAULTROOM;
            Context->Source->eaxBP.lRoomHF = EAXBUFFER_DEFAULTROOMHF;
            Context->Source->eaxBP.flRoomRolloffFactor = EAXBUFFER_DEFAULTROOMROLLOFFFACTOR;
            Context->Source->eaxBP.lObstruction = EAXBUFFER_DEFAULTOBSTRUCTION;
            Context->Source->eaxBP.flObstructionLFRatio = EAXBUFFER_DEFAULTOBSTRUCTIONLFRATIO;
            Context->Source->eaxBP.lOcclusion = EAXBUFFER_DEFAULTOCCLUSION;
            Context->Source->eaxBP.flOcclusionLFRatio = EAXBUFFER_DEFAULTOCCLUSIONLFRATIO;
            Context->Source->eaxBP.flOcclusionRoomRatio = EAXBUFFER_DEFAULTOCCLUSIONROOMRATIO;
            Context->Source->eaxBP.lOutsideVolumeHF = EAXBUFFER_DEFAULTOUTSIDEVOLUMEHF;
            Context->Source->eaxBP.flAirAbsorptionFactor = EAXBUFFER_DEFAULTAIRABSORPTIONFACTOR;
            Context->Source->eaxBP.dwFlags = EAXBUFFER_DEFAULTFLAGS;
*/			
			Context->Source->update2 |= SALLPARAMS;
            Context->SourceCount++;
			i++;
		}
		alcUpdateContext(Context, ALSOURCE, (ALuint)Context->Source);
		// Some Listener Settings (e.g EAX 2.0 Reverb) can only be initialized once a Source has been
		// created, so call alcUpdateContext again to update Listener parameters
		alcUpdateContext(Context, ALLISTENER, 0);
		Source=Context->Source;
	}
	else
	{
		// Some number of sources have already been created - move to the end of the list
		Source=Context->Source;
		while (Source->next)
			Source=Source->next;
	}

	// Add additional sources to the list (Source->next points to the location for the next Source structure)
	while ((Source)&&(i<n))
	{
		Source->next=malloc(sizeof(ALsource));
		if (Source->next)
		{
			memset(Source->next,0,sizeof(ALsource));
			sources[i]=(ALuint)Source->next;
			Source->next->previous=Source;
			Source->next->valid=AL_TRUE;
			Source->next->update1 |= SGENERATESOURCE;


			Source->next->param[AL_CONE_INNER_ANGLE-AL_CONE_INNER_ANGLE].data.f=360.0;
			Source->next->param[AL_CONE_INNER_ANGLE-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->param[AL_CONE_OUTER_ANGLE-AL_CONE_INNER_ANGLE].data.f=360.0;
			Source->next->param[AL_CONE_OUTER_ANGLE-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->param[AL_PITCH-AL_CONE_INNER_ANGLE].data.f= 1.0;
			Source->next->param[AL_PITCH-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->param[AL_POSITION-AL_CONE_INNER_ANGLE].data.fv3[0]=0.0;
			Source->next->param[AL_POSITION-AL_CONE_INNER_ANGLE].data.fv3[1]=0.0;
			Source->next->param[AL_POSITION-AL_CONE_INNER_ANGLE].data.fv3[2]=0.0;
			Source->next->param[AL_POSITION-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->param[AL_DIRECTION-AL_CONE_INNER_ANGLE].data.fv3[0]=0.0;
			Source->next->param[AL_DIRECTION-AL_CONE_INNER_ANGLE].data.fv3[1]=0.0;
			Source->next->param[AL_DIRECTION-AL_CONE_INNER_ANGLE].data.fv3[2]=0.0;
			Source->next->param[AL_DIRECTION-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->param[AL_VELOCITY-AL_CONE_INNER_ANGLE].data.fv3[0]=0.0;
			Source->next->param[AL_VELOCITY-AL_CONE_INNER_ANGLE].data.fv3[1]=0.0;
			Source->next->param[AL_VELOCITY-AL_CONE_INNER_ANGLE].data.fv3[2]=0.0;
			Source->next->param[AL_VELOCITY-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->param[AL_REFERENCE_DISTANCE-AL_CONE_INNER_ANGLE].data.f= 1.0;
			Source->next->param[AL_REFERENCE_DISTANCE-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->param[AL_MAX_DISTANCE-AL_CONE_INNER_ANGLE].data.f= 1000000.0;
			Source->next->param[AL_MAX_DISTANCE-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->param[AL_ROLLOFF_FACTOR-AL_CONE_INNER_ANGLE].data.f= 1.0;
			Source->next->param[AL_ROLLOFF_FACTOR-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->param[AL_LOOPING-AL_CONE_INNER_ANGLE].data.i= AL_FALSE;
			Source->next->param[AL_LOOPING-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->param[AL_GAIN-AL_CONE_INNER_ANGLE].data.f= 1.0f;
			Source->next->param[AL_GAIN-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->param[AL_MIN_GAIN-AL_CONE_INNER_ANGLE].data.f= 0.0f;
			Source->next->param[AL_MIN_GAIN-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->param[AL_MAX_GAIN-AL_CONE_INNER_ANGLE].data.f= 1.0f;
			Source->next->param[AL_MAX_GAIN-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->param[AL_CONE_OUTER_GAIN-AL_CONE_INNER_ANGLE].data.f= 1.0f;
			Source->next->param[AL_CONE_OUTER_GAIN-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->state = AL_INITIAL;

			Source->next->param[AL_BUFFER-AL_CONE_INNER_ANGLE].data.i= 0;
			Source->next->param[AL_BUFFER-AL_CONE_INNER_ANGLE].valid = AL_TRUE;

			Source->next->update1 |= CONEANGLES | FREQUENCY | POSITION | VELOCITY | ORIENTATION |
				MINDIST | MAXDIST | LOOPED | VOLUME | CONEOUTSIDEVOLUME | STATE;
/*
            Source->next->eaxBP.lDirect = EAXBUFFER_DEFAULTDIRECT;
            Source->next->eaxBP.lDirectHF = EAXBUFFER_DEFAULTDIRECTHF;
            Source->next->eaxBP.lRoom = EAXBUFFER_DEFAULTROOM;
            Source->next->eaxBP.lRoomHF = EAXBUFFER_DEFAULTROOMHF;
            Source->next->eaxBP.flRoomRolloffFactor = EAXBUFFER_DEFAULTROOMROLLOFFFACTOR;
            Source->next->eaxBP.lObstruction = EAXBUFFER_DEFAULTOBSTRUCTION;
            Source->next->eaxBP.flObstructionLFRatio = EAXBUFFER_DEFAULTOBSTRUCTIONLFRATIO;
            Source->next->eaxBP.lOcclusion = EAXBUFFER_DEFAULTOCCLUSION;
            Source->next->eaxBP.flOcclusionLFRatio = EAXBUFFER_DEFAULTOCCLUSIONLFRATIO;
            Source->next->eaxBP.flOcclusionRoomRatio = EAXBUFFER_DEFAULTOCCLUSIONROOMRATIO;
            Source->next->eaxBP.lOutsideVolumeHF = EAXBUFFER_DEFAULTOUTSIDEVOLUMEHF;
            Source->next->eaxBP.flAirAbsorptionFactor = EAXBUFFER_DEFAULTAIRABSORPTIONFACTOR;
            Source->next->eaxBP.dwFlags = EAXBUFFER_DEFAULTFLAGS;
*/			
			Source->next->update2 |= SALLPARAMS;

			Context->SourceCount++;
			i++;
		}
		alcUpdateContext(Context, ALSOURCE, (ALuint)Source->next);
		Source=Source->next;
	}
	alcProcessContext(Context);
	if (i!=n)
		alSetError(AL_OUT_OF_MEMORY);
}

ALAPI ALvoid ALAPIENTRY alDeleteSources(ALsizei n,ALuint *sources)
{
	ALCcontext *Context;
	ALsource *ALSource;
	ALsizei i;
	ALbufferlistitem *ALBufferList;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	for (i=0;i<n;i++)
	{
		if (alIsSource(sources[i]))
		{
			ALSource=((ALsource *)sources[i]);
            alSourceStop((ALuint)ALSource);

			// For each buffer in the source's queue, decrement its reference counter and remove it
			while (ALSource->queue != NULL)
			{
				ALBufferList = ALSource->queue;
				// Decrement buffer's reference counter
				if (ALBufferList->buffer != 0)
					((ALbuffer*)(ALBufferList->buffer))->refcount--;
				// Update queue to point to next element in list
				ALSource->queue = ALBufferList->next;
				// Release memory allocated for buffer list item
				free(ALBufferList);
			}

			// Call alcUpdateContext with SDELETE flag to perform context specific deletion of source
			ALSource->update1 = SDELETE;
			alcUpdateContext(Context, ALSOURCE, (ALuint)ALSource);

			// Remove Source from list of Sources
			if (ALSource->previous)
				ALSource->previous->next=ALSource->next;
			else
				Context->Source=ALSource->next;
			if (ALSource->next)
				ALSource->next->previous=ALSource->previous;
			
			memset(ALSource,0,sizeof(ALsource));
			Context->SourceCount--;
			free(ALSource);
		}
		else alSetError(AL_INVALID_NAME);
	}
	alcProcessContext(Context);
}

ALAPI ALboolean ALAPIENTRY alIsSource(ALuint source)
{
	ALboolean result=AL_FALSE;
	ALCcontext *Context;
	ALsource *Source;
	
	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	Source=((ALsource *)source);
	if (Source)
	{
		if ((Source->previous==NULL)||(Source->previous->next==Source))
		{
			if ((Source->next==NULL)||(Source->next->previous==Source))
				result=Source->valid;
		}
	}
	alcProcessContext(Context);
	return result;
}

ALAPI ALvoid ALAPIENTRY alSourcef(ALuint source,ALenum pname,ALfloat value)
{
	ALCcontext *Context;
	ALsource *Source;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	if (alIsSource(source))
	{
		Source=((ALsource *)source);
		switch(pname)
		{
			case AL_PITCH:
				if ((value>=0.0f)&&(value<=2.0f))
				{	
					Source->param[pname-AL_CONE_INNER_ANGLE].data.f=value;
					Source->param[pname-AL_CONE_INNER_ANGLE].valid=AL_TRUE;
					Source->update1 |= FREQUENCY;	// Property to update
					alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
				}
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_GAIN:
			case AL_MAX_DISTANCE:
			case AL_ROLLOFF_FACTOR:
			case AL_REFERENCE_DISTANCE:
				if (value>=0.0f)
				{
					Source->param[pname-AL_CONE_INNER_ANGLE].data.f=value;
					Source->param[pname-AL_CONE_INNER_ANGLE].valid=AL_TRUE;

					if (pname == AL_GAIN)
						Source->update1 |= VOLUME;
					else if (pname == AL_MAX_DISTANCE)
						Source->update1 |= MAXDIST;
					else if (pname == AL_REFERENCE_DISTANCE)
						Source->update1 |= MINDIST;

					// ROLLOFF_FACTOR ignored at this time !

					alcUpdateContext(Context,ALSOURCE, (ALuint)Source);
				}
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_MIN_GAIN:
			case AL_MAX_GAIN:
			case AL_CONE_OUTER_GAIN:
				if ((value>=0.0f)&&(value<=1.0f))
				{	
					Source->param[pname-AL_CONE_INNER_ANGLE].data.f=value;
					Source->param[pname-AL_CONE_INNER_ANGLE].valid=AL_TRUE;

					Source->update1 |= CONEOUTSIDEVOLUME;	// Property to update
					alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
				}
				else
					alSetError(AL_INVALID_VALUE);
				break;
			default:
				alSetError(AL_INVALID_ENUM);
				break;
		}
	} 
	else alSetError(AL_INVALID_NAME);
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alSourcefv(ALuint source,ALenum pname,ALfloat *values)
{
	ALCcontext *Context;
	ALsource *Source;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	if (alIsSource(source))
	{
		Source=((ALsource *)source);
		switch(pname) 
		{
			case AL_POSITION:
			case AL_VELOCITY:
			case AL_DIRECTION:
				Source->param[pname-AL_CONE_INNER_ANGLE].data.fv3[0]=values[0];
				Source->param[pname-AL_CONE_INNER_ANGLE].data.fv3[1]=values[1];
				Source->param[pname-AL_CONE_INNER_ANGLE].data.fv3[2]=values[2];
				Source->param[pname-AL_CONE_INNER_ANGLE].valid=AL_TRUE;

				if (pname == AL_POSITION)
					Source->update1 |= POSITION;
				else if (pname == AL_VELOCITY)
					Source->update1 |= VELOCITY;
				else if (pname == AL_DIRECTION)
					Source->update1 |= ORIENTATION;

				alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
				break;
			default:
				alSetError(AL_INVALID_ENUM);
				break;
		}
	} 
	else alSetError(AL_INVALID_NAME);
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alSource3f(ALuint source,ALenum pname,ALfloat v1,ALfloat v2,ALfloat v3)
{
	ALCcontext *Context;
	ALsource *Source;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	if (alIsSource(source))
	{
		Source=((ALsource *)source);
		switch(pname) 
		{
			case AL_POSITION:
			case AL_VELOCITY:
			case AL_DIRECTION:
				Source->param[pname-AL_CONE_INNER_ANGLE].data.fv3[0]=v1;
				Source->param[pname-AL_CONE_INNER_ANGLE].data.fv3[1]=v2;
				Source->param[pname-AL_CONE_INNER_ANGLE].data.fv3[2]=v3;
				Source->param[pname-AL_CONE_INNER_ANGLE].valid=AL_TRUE;

				if (pname == AL_POSITION)
					Source->update1 |= POSITION;
				else if (pname == AL_VELOCITY)
					Source->update1 |= VELOCITY;
				else if (pname == AL_DIRECTION)
					Source->update1 |= ORIENTATION;

				alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
				break;
			default:
				alSetError(AL_INVALID_ENUM);
				break;
		}
	} 
	else alSetError(AL_INVALID_NAME);
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alSourcei(ALuint source,ALenum pname,ALint value)
{
	ALCcontext *Context;
	ALsource *Source;
	ALbufferlistitem *ALBufferListItem;
	ALint	Counter = 0;
	ALint	DataSize = 0;
	ALint	BufferSize;
	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	if (alIsSource(source))
	{
		Source=((ALsource *)source);
		switch(pname) 
		{
			case AL_SOURCE_RELATIVE:
				if ((value==AL_FALSE)||(value==AL_TRUE))
				{
					Source->relative=value;
					Source->update1 |= MODE;
					
					alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
				}
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_CONE_INNER_ANGLE:
			case AL_CONE_OUTER_ANGLE:
				if ((value>=0)&&(value<=360))
				{
					Source->param[pname-AL_CONE_INNER_ANGLE].data.f=(float)value;
					Source->param[pname-AL_CONE_INNER_ANGLE].valid=AL_TRUE;

					Source->update1 |= CONEANGLES;
					alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
				}
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_LOOPING:
				if ((value==AL_FALSE)||(value==AL_TRUE))
				{
					Source->param[pname-AL_CONE_INNER_ANGLE].data.i=value;
					Source->param[pname-AL_CONE_INNER_ANGLE].valid=AL_TRUE;
					Source->update1 |= LOOPED;
					alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
				}
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_BUFFER:
				if ((Source->state == AL_STOPPED) || (Source->state == AL_INITIAL))
				{
					if (alIsBuffer(value) || (value == 0))
					{
						// Remove all elements in the queue
						while (Source->queue != NULL)
						{
							ALBufferListItem = Source->queue;
							Source->queue = ALBufferListItem->next;
							// Decrement reference counter for buffer
							if (ALBufferListItem->buffer)
								((ALbuffer*)(ALBufferListItem->buffer))->refcount--;
							// Record size of buffer
							alGetBufferi(ALBufferListItem->buffer, AL_SIZE, &BufferSize);
							DataSize += BufferSize;
							// Increment the number of buffers removed from queue
							Counter++;
							// Release memory for buffer list item
							free(ALBufferListItem);
							// Decrement the number of buffers in the queue
							Source->BuffersInQueue--;
						}

						// Update variables required by the SUNQUEUE routine in UpdateContext
						Source->NumBuffersRemovedFromQueue = Counter;
						Source->SizeOfBufferDataRemovedFromQueue = DataSize;
						Source->update1 |= SUNQUEUE;
						alcUpdateContext(Context, ALSOURCE, source);

						// Add the buffer to the queue (as long as it is NOT the NULL buffer)
						if (value != 0)
						{
							// Add the selected buffer to the queue
							ALBufferListItem = malloc(sizeof(ALbufferlistitem));
							ALBufferListItem->buffer = value;
							ALBufferListItem->bufferstate = PENDING;
							ALBufferListItem->flag = 0;
							ALBufferListItem->next = NULL;

							Source->queue = ALBufferListItem;
							Source->BuffersInQueue = 1;
							alGetBufferi(value, AL_SIZE, &DataSize);

							// Increment reference counter for buffer
							((ALbuffer*)(value))->refcount++;

							Source->SizeOfBufferDataAddedToQueue = DataSize;
							Source->NumBuffersAddedToQueue = 1;
							Source->update1 |= SQUEUE;
							alcUpdateContext(Context, ALSOURCE, source);
						}

						// Set Buffers Processed
						Source->BuffersProcessed = 0;

						// Update AL_BUFFER parameter
						Source->param[pname-AL_CONE_INNER_ANGLE].data.i=value;
						Source->param[pname-AL_CONE_INNER_ANGLE].valid=AL_TRUE;
					}
					else
						alSetError(AL_INVALID_VALUE);
				}
				else
					alSetError(AL_INVALID_OPERATION);
				break;
			case AL_SOURCE_STATE:
				Source->state=value;
				Source->update1 |= STATE;
				alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
				break;
			default:
				alSetError(AL_INVALID_ENUM);
				break;
	
		}
	} 
	else alSetError(AL_INVALID_NAME);
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alGetSourcef(ALuint source,ALenum pname,ALfloat *value)
{
	ALCcontext *Context;
	ALsource *Source;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	if (alIsSource(source))
	{
		Source=((ALsource *)source);
		switch(pname) 
		{
			case AL_PITCH:
			case AL_GAIN:
			case AL_MIN_GAIN:
			case AL_MAX_GAIN:
			case AL_MAX_DISTANCE:
			case AL_ROLLOFF_FACTOR:
			case AL_CONE_OUTER_GAIN:
			case AL_CONE_INNER_ANGLE:
			case AL_CONE_OUTER_ANGLE:
			case AL_REFERENCE_DISTANCE:
				if (Source->param[pname-AL_CONE_INNER_ANGLE].valid)
					*value=Source->param[pname-AL_CONE_INNER_ANGLE].data.f;
				break;
			default:
				alSetError(AL_INVALID_ENUM);
				break;
		}
	} 
	else alSetError(AL_INVALID_NAME);
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alGetSourcefv(ALuint source,ALenum pname,ALfloat *values)
{
	ALCcontext *Context;
	ALsource *Source;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	if (alIsSource(source))
	{
		Source=((ALsource *)source);
		switch(pname) 
		{
			case AL_POSITION:
			case AL_VELOCITY:
			case AL_DIRECTION:
				if (Source->param[pname-AL_CONE_INNER_ANGLE].valid)
				{
					values[0]=Source->param[pname-AL_CONE_INNER_ANGLE].data.fv3[0];
					values[1]=Source->param[pname-AL_CONE_INNER_ANGLE].data.fv3[1];
					values[2]=Source->param[pname-AL_CONE_INNER_ANGLE].data.fv3[2];
				}
				break;
			default:
				alSetError(AL_INVALID_ENUM);
				break;
		}
	} 
	else alSetError(AL_INVALID_NAME);
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alGetSourcei(ALuint source,ALenum pname,ALint *value)
{
	ALCcontext *Context;
	ALsource *Source;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	if (alIsSource(source))
	{
		Source=((ALsource *)source);
		switch(pname) 
		{
			case AL_SOURCE_RELATIVE:
				*value=Source->relative;
				break;
			case AL_CONE_INNER_ANGLE:
			case AL_CONE_OUTER_ANGLE:
				if (Source->param[pname-AL_CONE_INNER_ANGLE].valid)
					*value=(ALint)Source->param[pname-AL_CONE_INNER_ANGLE].data.f;
				break;
			case AL_LOOPING:
				if (Source->param[pname-AL_CONE_INNER_ANGLE].valid)
					*value = Source->param[pname-AL_CONE_INNER_ANGLE].data.i;
				else
					*value = 0;
				break;
			case AL_BUFFER:
				// Call UpdateContext to retrieve up-to-date information about the current buffer
				Source->update1 |= SUPDATEBUFFERS;
				alcUpdateContext(Context, ALSOURCE, source);

				if (Source->param[pname-AL_CONE_INNER_ANGLE].valid)
					*value=Source->param[pname-AL_CONE_INNER_ANGLE].data.i;
				else
					*value=0;
				break;
			case AL_SOURCE_STATE:
				*value=Source->state;
				break;
			case AL_BUFFERS_QUEUED:
				*value=Source->BuffersInQueue;
				break;
			case AL_BUFFERS_PROCESSED:
				*value=Source->BuffersProcessed;
				break;
			default:
				alSetError(AL_INVALID_ENUM);
				break;
		}
	} 
	else alSetError(AL_INVALID_NAME);
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alSourcePlay(ALuint source)
{
	ALCcontext *Context;
	ALsource *Source;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	if (alIsSource(source))
	{
		Source=((ALsource *)source);
		if (Source->state!=AL_PAUSED)
		{
			Source->state=AL_PLAYING;
			Source->inuse=AL_TRUE;
			Source->play=AL_TRUE;
			Source->position=0;
			Source->position_fraction=0;
			Source->BuffersProcessed = 0;
			Source->BuffersAddedToDSBuffer = 0;
		}
		else
		{
			Source->state=AL_PLAYING;
			Source->inuse=AL_TRUE;
			Source->play=AL_TRUE;
		}
		Source->update1 |= STATE;
		alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
	} 
	else alSetError(AL_INVALID_OPERATION);
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alSourcePlayv(ALsizei n,ALuint *sources)
{
	ALCcontext *Context;
	ALsource *Source;
	ALsizei i;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	for (i=0;i<n;i++)
	{
		if (alIsSource(sources[i]))
		{
			Source=((ALsource *)sources[i]);
			if (Source->state!=AL_PAUSED)
			{
				Source->state=AL_PLAYING;
				Source->inuse=AL_TRUE;
				Source->play=AL_TRUE;
				Source->position=0;
				Source->position_fraction=0;
				Source->BuffersProcessed = 0;
				Source->BuffersAddedToDSBuffer = 0;
			}
			else
			{
				Source->state=AL_PLAYING;
				Source->inuse=AL_TRUE;
				Source->play=AL_TRUE;
			}
			Source->update1 |= STATE;
			alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
		} 
		else alSetError(AL_INVALID_OPERATION);
	}
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alSourcePause(ALuint source)
{
	ALCcontext *Context;
	ALsource *Source;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	if (alIsSource(source))
	{
		Source=((ALsource *)source);
		if (Source->state==AL_PLAYING)
		{
			Source->state=AL_PAUSED;
			Source->inuse=AL_FALSE;
		}
		Source->update1 = STATE;
		alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
	} 
	else alSetError(AL_INVALID_OPERATION);
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alSourcePausev(ALsizei n,ALuint *sources)
{
	ALCcontext *Context;
	ALsource *Source;
	ALsizei i;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	for (i=0;i<n;i++)
	{
		if (alIsSource(sources[i]))
		{
			Source=((ALsource *)sources[i]);
			if (Source->state==AL_PLAYING)
			{
				Source->state=AL_PAUSED;
				Source->inuse=AL_FALSE;
			}
			Source->update1 |= STATE;
			alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
		} 
		else alSetError(AL_INVALID_OPERATION);
	} 
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alSourceStop(ALuint source)
{
	ALCcontext *Context;
	ALsource *Source;
	ALbufferlistitem *ALBufferListItem;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	if (alIsSource(source))
	{
		Source=((ALsource *)source);
		if (Source->state!=AL_INITIAL)
		{
			Source->state=AL_STOPPED;
			Source->inuse=AL_FALSE;
			Source->BuffersProcessed = Source->BuffersInQueue;
			ALBufferListItem= Source->queue;
			while (ALBufferListItem != NULL)
			{
				ALBufferListItem->bufferstate = PROCESSED;
				ALBufferListItem = ALBufferListItem->next;
			}
		}
		Source->update1 |= STATE;
		alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
	} 
	else alSetError(AL_INVALID_OPERATION);
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alSourceStopv(ALsizei n,ALuint *sources)
{
	ALCcontext *Context;
	ALsource *Source;
	ALsizei i;
	ALbufferlistitem *ALBufferListItem;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	for (i=0;i<n;i++)
	{
		if (alIsSource(sources[i]))
		{
			Source=((ALsource *)sources[i]);
			if (Source->state!=AL_INITIAL)
			{
				Source->state=AL_STOPPED;
				Source->inuse=AL_FALSE;
				Source->BuffersProcessed = Source->BuffersInQueue;
				ALBufferListItem= Source->queue;
				while (ALBufferListItem != NULL)
				{
					ALBufferListItem->bufferstate = PROCESSED;
					ALBufferListItem = ALBufferListItem->next;
				}
			}
			Source->update1 |= STATE;
			alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
		} 
		else alSetError(AL_INVALID_OPERATION);
	}
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alSourceRewind(ALuint source)
{
	ALCcontext *Context;
	ALsource *Source;
	ALbufferlistitem *ALBufferListItem;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	if (alIsSource(source))
	{
		Source=((ALsource *)source);
		if (Source->state!=AL_INITIAL)
		{
			Source->state=AL_INITIAL;
			Source->inuse=AL_FALSE;
			Source->position=0;
			Source->position_fraction=0;
			Source->BuffersProcessed = 0;
			ALBufferListItem= Source->queue;
			while (ALBufferListItem != NULL)
			{
				ALBufferListItem->bufferstate = PROCESSED;
				ALBufferListItem = ALBufferListItem->next;
			}
		}
		Source->update1 |= STATE;
		alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
	} 
	else alSetError(AL_INVALID_OPERATION);
	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alSourceRewindv(ALsizei n,ALuint *sources)
{
	ALCcontext *Context;
	ALsource *Source;
	ALsizei i;
	ALbufferlistitem *ALBufferListItem;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);
	for (i=0;i<n;i++)
	{
		if (alIsSource(sources[i]))
		{
			Source=((ALsource *)sources[i]);
			if (Source->state!=AL_INITIAL)
			{
				Source->state=AL_INITIAL;
				Source->inuse=AL_FALSE;
				Source->position=0;
				Source->position_fraction=0;
				Source->BuffersProcessed = 0;
				ALBufferListItem= Source->queue;
				while (ALBufferListItem != NULL)
				{
					ALBufferListItem->bufferstate = PROCESSED;
					ALBufferListItem = ALBufferListItem->next;
				}
			}
			Source->update1 |= STATE;
			alcUpdateContext(Context, ALSOURCE, (ALuint)Source);
		} 
		else alSetError(AL_INVALID_OPERATION);
	}
	alcProcessContext(Context);
}


ALAPI ALvoid ALAPIENTRY alSourceQueueBuffers( ALuint source, ALsizei n, ALuint* buffers )
{
	ALCcontext *Context;
	ALsource *ALSource;
	ALsizei i;
	ALbufferlistitem *ALBufferList;
	ALbufferlistitem *ALBufferListStart;
	ALuint DataSize;
	ALuint BufferSize;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);

	ALSource = (ALsource*)source;

	DataSize = 0;
	BufferSize = 0;

	// Check that all buffers are valid or zero and that the source is valid
	
	// Check that this is a valid source
	if (!alIsSource(source))
	{
		alSetError(AL_INVALID_NAME);
		alcProcessContext(Context);
		return;
	}

	for (i = 0; i < n; i++)
	{
		if ((!alIsBuffer(buffers[i])) && (buffers[i] != 0))
		{
			alSetError(AL_INVALID_NAME);
			alcProcessContext(Context);
			return;
		}
	}		
	
	// All buffers are valid - so add them to the list
	ALBufferListStart = malloc(sizeof(ALbufferlistitem));
	ALBufferListStart->buffer = buffers[0];
	ALBufferListStart->bufferstate = PENDING;
	ALBufferListStart->flag = 0;
	ALBufferListStart->next = NULL;

	alGetBufferi(buffers[0], AL_SIZE, &BufferSize);
	DataSize += BufferSize;

	// Increment reference counter for buffer
	if (buffers[0] != 0)
		((ALbuffer*)(buffers[0]))->refcount++;

	ALBufferList = ALBufferListStart;

	for (i = 1; i < n; i++)
	{
		ALBufferList->next = malloc(sizeof(ALbufferlistitem));
		ALBufferList->next->buffer = buffers[i];
		ALBufferList->next->bufferstate = PENDING;
		ALBufferList->next->flag = 0;
		ALBufferList->next->next = NULL;

		alGetBufferi(buffers[i], AL_SIZE, &BufferSize);
		DataSize += BufferSize;

		// Increment reference counter for buffer
		if (buffers[i] != 0)
			((ALbuffer*)(buffers[i]))->refcount++;
	
		ALBufferList = ALBufferList->next;
	}

	if (ALSource->queue == NULL)
	{
		ALSource->queue = ALBufferListStart;		
		// Update Current Buffer
		ALSource->param[AL_BUFFER-AL_CONE_INNER_ANGLE].data.i = ALBufferListStart->buffer;
	}
	else
	{
		// Find end of queue
		ALBufferList = ALSource->queue;
		while (ALBufferList->next != NULL)
		{
			ALBufferList = ALBufferList->next;
		}

		ALBufferList->next = ALBufferListStart;
	}

	// Update number of buffers in queue
	ALSource->BuffersInQueue += n;

	// Record the amount of data added to the queue
	ALSource->SizeOfBufferDataAddedToQueue = DataSize;
	ALSource->NumBuffersAddedToQueue = n;
	ALSource->update1 |= SQUEUE;
	alcUpdateContext(Context, ALSOURCE, source);

	alcProcessContext(Context);
}

ALAPI ALvoid ALAPIENTRY alSourceUnqueueBuffers( ALuint source, ALsizei n, ALuint* buffers )
{
	ALCcontext *Context;
	ALsource *ALSource;
	ALsizei i;
	ALbufferlistitem *ALBufferList;
	ALbufferlistitem *ALBufferListTemp;
	ALuint DataSize;
	ALuint BufferSize;
	ALuint BufferID;

	DataSize = 0;
	BufferSize = 0;

	Context=alcGetCurrentContext();
	alcSuspendContext(Context);

	if (alIsSource(source))
	{
		ALSource = (ALsource*)source;

		// Check that each of the requested buffers to be unqueued have been processed
		for (i = 0; i < n; i++)
		{
			ALBufferList = ALSource->queue;

			// Find buffer[i] in the queue and check that it has been processed
			while (ALBufferList != NULL)
			{
				// Check if this is the buffer to be removed.  If it is, mark it so that we won't try
				// and remove it multiple times (this can happen if the application queues the same buffer
				// multiple times, and then removes the incorrect amount)
				if ((ALBufferList->buffer == buffers[i]) && (ALBufferList->flag != READYTOREMOVE))
				{
					// Set flag to indicate that this buffer is marked for removal
					ALBufferList->flag = READYTOREMOVE;
					break;
				}
				else
					ALBufferList = ALBufferList->next;
			}

			if ((ALBufferList == NULL) || (ALBufferList->bufferstate != PROCESSED))
			{
				// Buffer not found - or not ready for unqueuing
				alSetError(AL_INVALID_VALUE);
				alcProcessContext(Context);
				return;
			}
		}

		// OK to proceed with removal of buffers from queue
			
		for (i = 0; i < n; i++)
		{
			// Find each buffer to unqueue and remove it from the list
			ALBufferList = ALSource->queue;

			if (ALBufferList->buffer == buffers[i])
			{
				ALSource->queue = ALBufferList->next;
				// Decrement buffer reference counter
				if (ALBufferList->buffer)
					((ALbuffer*)(ALBufferList->buffer))->refcount--;
				// Record size of buffer
				alGetBufferi(ALBufferList->buffer, AL_SIZE, &BufferSize);
				DataSize += BufferSize;
				// Release memory for buffer list item
				free(ALBufferList);
				ALSource->BuffersInQueue--;
				ALSource->BuffersProcessed--;
			}
			else
			{
				while (ALBufferList->next != NULL)
				{
					if (ALBufferList->next->buffer == buffers[i])
					{
						ALBufferListTemp = ALBufferList->next;
						ALBufferList->next = ALBufferList->next->next;
						// Decrement buffer reference counter
						if (ALBufferListTemp->buffer)
							((ALbuffer*)(ALBufferListTemp->buffer))->refcount--;
						// Record size of buffer
						alGetBufferi(ALBufferListTemp->buffer, AL_SIZE, &BufferSize);
						DataSize += BufferSize;
						// Release memory for buffer list item
						free(ALBufferListTemp);
						ALSource->BuffersInQueue--;
						ALSource->BuffersProcessed--;
						break;
					}
					ALBufferList = ALBufferList->next;
				}
			}
		}

		if (ALSource->state != AL_PLAYING)
		{
			if (ALSource->queue)
				BufferID = ALSource->queue->buffer;
			else
				BufferID = 0;

			ALSource->param[AL_BUFFER-AL_CONE_INNER_ANGLE].data.i = BufferID;
		}

		ALSource->NumBuffersRemovedFromQueue = n;
		ALSource->SizeOfBufferDataRemovedFromQueue = DataSize;
		ALSource->BuffersAddedToDSBuffer -= ALSource->NumBuffersRemovedFromQueue;

		ALSource->update1 |= SUNQUEUE;
		alcUpdateContext(Context, ALSOURCE, source);
	}
	else
	{
		// Invalid source name
		alSetError(AL_INVALID_NAME);
	}

	alcProcessContext(Context);
}