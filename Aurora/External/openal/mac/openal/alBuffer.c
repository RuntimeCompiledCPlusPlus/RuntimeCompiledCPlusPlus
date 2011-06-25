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

#include "globals.h"
#include "alError.h"
#include "alBuffer.h"
 
// AL_BUFFER functions
ALAPI ALvoid ALAPIENTRY alGenBuffers(ALsizei n, ALuint *buffers)
{
	int i=0;
	int iCount=0;
	
	for (i = 1; i <= AL_MAXBUFFERS; i++)
	{
		if (gBuffer[i].data == NULL) // found un-used internal buffer to use
		{
			gBuffer[i].data = (void *) NewPtrClear(1024);  // allocate the buffer
			gBuffer[i].size = 1024;
			gBuffer[i].bits = 8;
			gBuffer[i].channels = 1;
			buffers[iCount] = i;
			iCount++;
		}
		if (iCount >= n) break;
	}
}

ALAPI ALvoid ALAPIENTRY alDeleteBuffers(ALsizei n, ALuint *buffers)
{
	int i=0;
	
	for (i = 0; i < n; i++)
	{
		if (alIsBuffer(buffers[i]) == true)
		{
			DisposePtr((char *)gBuffer[buffers[i]].data); // get rid of memory used by buffer
	 		gBuffer[buffers[i]].data = NULL;
	 		gBuffer[buffers[i]].size = 0;
	 		gBuffer[buffers[i]].bits = 8;
	 		gBuffer[buffers[i]].channels = 1;
	 		buffers[i] = 0;
	 	}
	}
}

ALAPI ALboolean ALAPIENTRY alIsBuffer(ALuint buffer)
{
	if (buffer > AL_MAXBUFFERS) return false; // can't be a buffer in this case...
	if (gBuffer[buffer].size == 0) return false;  // otherwise should have some memory allocated to it...
	
	return AL_TRUE;
}

ALAPI ALvoid ALAPIENTRY alBufferData(ALuint buffer,ALenum format,ALvoid *data,ALsizei size,ALsizei freq)
{
	if (alIsBuffer(buffer) == true)
	{
		DisposePtr((char *) gBuffer[buffer].data);
	
		gBuffer[buffer].data = (void *) NewPtrClear(size); // size is bytes for this example
		if (gBuffer[buffer].data != NULL)
		{
			BlockMove((char *) data, gBuffer[buffer].data, size);
			gBuffer[buffer].size = size;
			gBuffer[buffer].frequency = freq;
			switch (format)
			{
		    	case AL_FORMAT_STEREO8:
		    		gBuffer[buffer].channels = 2;
		    		gBuffer[buffer].bits = 8;
		    		break;
				case AL_FORMAT_MONO8 : 
					gBuffer[buffer].channels = 1;
		    		gBuffer[buffer].bits = 8;
		    		break;
		    	case AL_FORMAT_STEREO16:
		    		gBuffer[buffer].channels = 2;
		    		gBuffer[buffer].bits = 16;
		    		break;
		   	 	case AL_FORMAT_MONO16: 
		    		gBuffer[buffer].channels = 1;
		    		gBuffer[buffer].bits = 16;
		    		break;
		    	default: 
		    		gBuffer[buffer].channels = 1;
		    		gBuffer[buffer].bits = 8;
			}
		}
	}
}

ALAPI ALvoid ALAPIENTRY alGetBufferf (ALuint buffer, ALenum pname, ALfloat *value)
{
	if (alIsBuffer(buffer))
	{
		switch(pname)
		{
			case AL_FREQUENCY:
				*value = (float) gBuffer[buffer].frequency;
				break;
			default:
				alSetError(AL_INVALID_ENUM);
		}
	}
}

ALAPI ALvoid ALAPIENTRY alGetBufferi(ALuint buffer, ALenum pname, ALint *value)
{
	if (alIsBuffer(buffer))
	{
		switch(pname)
		{
			case AL_FREQUENCY:
				*value=gBuffer[buffer].frequency;
				break;
			case AL_BITS:
				*value=gBuffer[buffer].bits;
				break;
			case AL_CHANNELS:
				*value=gBuffer[buffer].channels;
				break;
			case AL_SIZE:
				*value=gBuffer[buffer].size;
				break;
			default:
				alSetError(AL_INVALID_ENUM);
		}
	}
}