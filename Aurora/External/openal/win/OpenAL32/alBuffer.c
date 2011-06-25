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
#include "include\alMain.h"
#include "include\alError.h"
#include "include\alBuffer.h"

static ALbuffer *Buffer=NULL;
static ALuint BufferCount=0;

ALAPI ALvoid ALAPIENTRY alGenBuffers(ALsizei n,ALuint *buffers)
{
	ALbuffer *ALBuf;
	ALsizei i=0;

	if (!Buffer)
	{
		Buffer=malloc(sizeof(ALbuffer));
		if (Buffer)
		{
			memset(Buffer,0,sizeof(ALbuffer));
			buffers[i]=(ALuint)Buffer;
			Buffer->state=UNUSED;
			BufferCount++;
			i++;
		}
		ALBuf=Buffer;
	}
	else
	{
		ALBuf=Buffer;
		while (ALBuf->next)
			ALBuf=ALBuf->next;
	}
	
	while ((ALBuf)&&(i<n))
	{
		ALBuf->next=malloc(sizeof(ALbuffer));
		if (ALBuf->next)
		{
			memset(ALBuf->next,0,sizeof(ALbuffer));
			buffers[i]=(ALuint)ALBuf->next;
			ALBuf->next->previous=ALBuf;
			ALBuf->next->state=UNUSED;
			BufferCount++;
			i++;
		}
		ALBuf=ALBuf->next;
	}
	if (i!=n) alSetError(AL_OUT_OF_MEMORY);
}

ALAPI ALvoid ALAPIENTRY alDeleteBuffers(ALsizei n,ALuint *buffers)
{
	ALbuffer *ALBuf;
	ALsizei i;


	for (i=0;i<n;i++)
	{
		if (alIsBuffer(buffers[i]))
		{
			ALBuf=((ALbuffer *)buffers[i]);

			// Check the reference counter for the buffer 
			if (ALBuf->refcount == 0)
			{
				if (ALBuf->previous)
					ALBuf->previous->next=ALBuf->next;
				else
					Buffer=ALBuf->next;
				if (ALBuf->next)
					ALBuf->next->previous=ALBuf->previous;

				// Release the memory used to store audio data
				if (ALBuf->data)
					free(ALBuf->data);

				// Release buffer structure
				memset(ALBuf,0,sizeof(ALbuffer));
				BufferCount--;
				free(ALBuf);
			}
			else alSetError(AL_INVALID_OPERATION);
		} 
		else
		{
			if (buffers[i] != 0)
				alSetError(AL_INVALID_NAME);
		}
	}
}

ALAPI ALboolean ALAPIENTRY alIsBuffer(ALuint buffer)
{
	ALboolean result=AL_FALSE;
	ALbuffer *ALBuf;
	
	ALBuf=((ALbuffer *)buffer);
	if (ALBuf)
	{
		if ((ALBuf->previous==NULL)||(ALBuf->previous->next==ALBuf))
		{
			if ((ALBuf->next==NULL)||(ALBuf->next->previous==ALBuf))
				result=AL_TRUE;
		}
	}
	return result;
}

ALAPI ALvoid ALAPIENTRY alBufferData(ALuint buffer,ALenum format,ALvoid *data,ALsizei size,ALsizei freq)
{
	ALbuffer *ALBuf;
	ALsizei i;
	
	if (alIsBuffer(buffer))
	{
		ALBuf=((ALbuffer *)buffer);
		if ((ALBuf->state==UNUSED)&&(data))
		{
			switch(format)
			{
				case AL_FORMAT_MONO8:
					if (ALBuf->data)
						ALBuf->data=realloc(ALBuf->data,(size+1)/sizeof(ALubyte)*sizeof(ALshort));
					else
						ALBuf->data=malloc((size+1)/sizeof(ALubyte)*sizeof(ALshort));
					if (ALBuf->data)
					{
						ALBuf->format=AL_FORMAT_MONO16;
						for (i=0;i<size/sizeof(ALubyte);i++) 
							ALBuf->data[i]=(ALshort)((((ALubyte *)data)[i]-128)<<8);
						ALBuf->size=size/sizeof(ALubyte)*sizeof(ALshort);
						ALBuf->frequency=freq;
					}
					else
						alSetError(AL_OUT_OF_MEMORY);
					break;
				case AL_FORMAT_MONO16:
					if (ALBuf->data)
						ALBuf->data=realloc(ALBuf->data,(size+1)/sizeof(ALshort)*sizeof(ALshort));
					else
						ALBuf->data=malloc((size+1)/sizeof(ALshort)*sizeof(ALshort));
					if (ALBuf->data)
					{
						ALBuf->format=AL_FORMAT_MONO16;
						memcpy(ALBuf->data,data,size/sizeof(ALshort)*sizeof(ALshort));
						ALBuf->size=size/sizeof(ALshort)*sizeof(ALshort);
						ALBuf->frequency=freq;
					}
					else
						alSetError(AL_OUT_OF_MEMORY);
					break;
				case AL_FORMAT_STEREO8:
					if (ALBuf->data)
						ALBuf->data=realloc(ALBuf->data,(size+2)/sizeof(ALubyte)*sizeof(ALshort));
					else
						ALBuf->data=malloc((size+2)/sizeof(ALubyte)*sizeof(ALshort));
					if (ALBuf->data)
					{
						ALBuf->format=AL_FORMAT_STEREO16;
						for (i=0;i<size/sizeof(ALubyte);i++) 
							ALBuf->data[i]=(ALshort)((((ALubyte *)data)[i]-128)<<8);
						ALBuf->size=size/sizeof(ALubyte)*sizeof(ALshort);
						ALBuf->frequency=freq;
					}
					else
						alSetError(AL_OUT_OF_MEMORY);
					break;
				case AL_FORMAT_STEREO16:
					if (ALBuf->data)
						ALBuf->data=realloc(ALBuf->data,(size+2)/sizeof(ALshort)*sizeof(ALshort));
					else
						ALBuf->data=malloc((size+2)/sizeof(ALshort)*sizeof(ALshort));
					if (ALBuf->data)
					{
						ALBuf->format=AL_FORMAT_STEREO16;
						memcpy(ALBuf->data,data,size/sizeof(ALshort)*sizeof(ALshort));
						ALBuf->size=size/sizeof(ALshort)*sizeof(ALshort);
						ALBuf->frequency=freq;
					}
					else
						alSetError(AL_OUT_OF_MEMORY);
					break;
				default:
					alSetError(AL_INVALID_VALUE);
					break;
			}
		}
		else alSetError(AL_INVALID_VALUE);
	} 
	else alSetError(AL_INVALID_OPERATION);
}

ALAPI ALvoid ALAPIENTRY alGetBufferf(ALuint buffer,ALenum pname,ALfloat *value)
{
	ALbuffer *ALBuf;

	if (alIsBuffer(buffer))
	{
		ALBuf=((ALbuffer *)buffer);
		switch(pname)
		{
			default:
				alSetError(AL_INVALID_OPERATION);
				break;
		}
	} 
	else alSetError(AL_INVALID_OPERATION);
}

ALAPI ALvoid ALAPIENTRY alGetBufferi(ALuint buffer,ALenum pname,ALint *value)
{
	ALbuffer *ALBuf;

	if (buffer == 0)
		*value = 0;
	else if (alIsBuffer(buffer))
	{
		ALBuf=((ALbuffer *)buffer);
		switch(pname)
		{
			case AL_FREQUENCY:
				*value=ALBuf->frequency;
				break;
			case AL_BITS:
				*value=(((ALBuf->format==AL_FORMAT_MONO8)||(ALBuf->format==AL_FORMAT_STEREO8))?8:16);
				break;
			case AL_CHANNELS:
				*value=(((ALBuf->format==AL_FORMAT_MONO8)||(ALBuf->format==AL_FORMAT_MONO16))?1:2);
				break;
			case AL_SIZE:
				*value=ALBuf->size;
				break;
			case AL_DATA:
				*value=(ALint)ALBuf->data;
				break;
			default:
				alSetError(AL_INVALID_OPERATION);
				break;
		}
	} 
	else alSetError(AL_INVALID_OPERATION);
}
