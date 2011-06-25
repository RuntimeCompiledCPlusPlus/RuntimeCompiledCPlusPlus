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

#include <Sound.h>
#include "sm.h"
#include "globals.h" 
#include "alError.h"
#include "alSoftware.h"
#include "alSource.h"
 
ALAPI ALvoid ALAPIENTRY alGenSources(ALsizei n, ALuint *sources)
{
	int i=0;
	int iCount=0;
	
	// make sure invalid source ID passed back if not allocatable
	for (i = 0; i < n; i++)
	{
		sources[i] = 0;
	}
	
	// allocate sources where possible...
	for (i = 1; i <= AL_MAXSOURCES; i++)
	{
		if (gSource[i].srcBufferNum == AL_MAXBUFFERS + 1) // found un-used source
		{
		    smSourceInit(i);
			gSource[i].srcBufferNum = 0;  // allocate the source
			gSource[i].state = AL_INITIAL;
			sources[iCount] = i;
			iCount++;
		}
		if (iCount >= n) break;
	}
}

ALAPI ALvoid ALAPIENTRY alDeleteSources (ALsizei n, ALuint *sources)
{
	int i;
	QueueEntry *tempQPtr;
	
	// clear source/channel information
	for (i= 0; i < n; i++)
	{
		if (alIsSource(sources[i]) == true)
		{
            smSourceKill(sources[i]);
			gSource[sources[i]].srcBufferNum = AL_MAXBUFFERS + 1; // value > AL_MAXBUFFERS used as signal source is not being used
			gSource[sources[i]].readOffset = 0;
			gSource[sources[i]].writeOffset = 0;
			gSource[sources[i]].state = AL_INITIAL;
			
			if (gSource[sources[i]].ptrQueue != NULL)
			{
				tempQPtr = gSource[sources[i]].ptrQueue;
				gSource[sources[i]].ptrQueue = tempQPtr->pNext;
				DisposePtr((char *)tempQPtr);
			}
			
			sources[i] = 0;
		}
	}
}

ALAPI ALboolean ALAPIENTRY alIsSource(ALuint source)
{
	if (source > AL_MAXSOURCES) return AL_FALSE; // can't be a source in this case...
	
	if (gSource[source].srcBufferNum <= AL_MAXBUFFERS)
	{
	 	return AL_TRUE;
	}
	
	return AL_FALSE;
}

ALAPI ALvoid ALAPIENTRY alSourcef (ALuint source, ALenum pname, ALfloat value)
{
	if (alIsSource(source))
	{
		switch(pname) 
		{
			case AL_PITCH:
				if ((value>=0.5f)&&(value<=2.0f))
				{	
					gSource[source].pitch = value;
				}
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_GAIN:
				gSource[source].gain = value;	
				break;
			case AL_MAX_DISTANCE:
				gSource[source].maxDistance = value;
				break;
			case AL_MIN_GAIN:
				if ((value > 0.0f) && (value <= 1.0f))
				{
					gSource[source].minGain = value;
				} else
				{
					alSetError(AL_INVALID_VALUE);
				}
				break;
			case AL_MAX_GAIN:
				if ((value > 0.0f) && (value <= 1.0f))
				{
					gSource[source].maxGain = value;
				} else
				{
					alSetError(AL_INVALID_VALUE);
				}
				break;
			case AL_ROLLOFF_FACTOR:
				if (value > 0.0f)
				{
					if (value <= 1.0f) // clamp to 1.0f because implementation breaks above 1.0f
					{
						gSource[source].rolloffFactor = value;
					}
				} else
				{
					alSetError(AL_INVALID_VALUE);
				}
				break;
			case AL_REFERENCE_DISTANCE:
				if (value > 0.0f)
				{
					gSource[source].referenceDistance = value;
				} else
				{
					alSetError(AL_INVALID_VALUE);
				}
				break;
			default:
				alSetError(AL_INVALID_OPERATION);
				break;
		}
	}
}

ALAPI ALvoid ALAPIENTRY alSourcefv (ALuint source, ALenum pname, ALfloat *values)
{
	switch(pname) 
	{
		case AL_POSITION:
			gSource[source].Position[0]=values[0];
			gSource[source].Position[1]=values[1];
			gSource[source].Position[2]=values[2];
			break;
		case AL_DIRECTION:
		    alSetError(AL_INVALID_ENUM); // cone functions not implemented yet
		    break;
		case AL_VELOCITY:
			gSource[source].Velocity[0]=values[0];
			gSource[source].Velocity[1]=values[1];
			gSource[source].Velocity[2]=values[2];
			break;
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
}

ALAPI ALvoid ALAPIENTRY alSource3f (ALuint source, ALenum pname, ALfloat v1, ALfloat v2, ALfloat v3)
{
	switch(pname) 
	{
		case AL_POSITION:
			gSource[source].Position[0]=v1;
			gSource[source].Position[1]=v2;
			gSource[source].Position[2]=v3;
			break;
		case AL_VELOCITY:
			gSource[source].Velocity[0]=v1;
			gSource[source].Velocity[1]=v2;
			gSource[source].Velocity[2]=v3;
			break;
		default:
			alSetError(AL_INVALID_ENUM);
			break;
	}
}

ALAPI ALvoid ALAPIENTRY alSourcei (ALuint source, ALenum pname, ALint value)
{
	if (alIsSource(source))
	{
		switch(pname) 
		{
			case AL_LOOPING:
				gSource[source].looping = value;
				break;
			case AL_BUFFER:
				gSource[source].srcBufferNum = value;
				break;
			default:
				alSetError(AL_INVALID_ENUM);
				break;
	
		}
	}
}

ALAPI ALvoid ALAPIENTRY alGetSourcef (ALuint source, ALenum pname, ALfloat *value)
{
	if (alIsSource(source))
	{
		switch(pname) 
		{
			case AL_PITCH:
				*value = gSource[source].pitch;
				break;
			case AL_GAIN:
				*value = gSource[source].gain;
				break;
			case AL_MAX_DISTANCE:
				*value = gSource[source].maxDistance;
				break;
			case AL_MIN_GAIN:
				*value = gSource[source].minGain;
				break;
			case AL_MAX_GAIN:
				*value = gSource[source].maxGain;
				break;
			case AL_ROLLOFF_FACTOR:
				*value = gSource[source].rolloffFactor;
				break;
			case AL_REFERENCE_DISTANCE:
				*value = gSource[source].referenceDistance;
				break;
			default:
				alSetError(AL_INVALID_ENUM);
				break;
		}
	}	
}

ALAPI ALvoid ALAPIENTRY alGetSourcefv (ALuint source, ALenum pname, ALfloat *values)
{
	if (alIsSource(source))
	{
		switch(pname) 
		{
			case AL_POSITION:
				values[0] = gSource[source].Position[0];
				values[1] = gSource[source].Position[1];
				values[2] = gSource[source].Position[2];
				break;
			case AL_VELOCITY:
				values[0] = gSource[source].Velocity[0];
				values[1] = gSource[source].Velocity[1];
				values[2] = gSource[source].Velocity[2];
				break;
			case AL_DIRECTION:
				alSetError(AL_INVALID_ENUM);
				break;
			default:
				alSetError(AL_INVALID_ENUM);
				break;
		}
	}
}

ALAPI ALvoid ALAPIENTRY alGetSourcei (ALuint source, ALenum pname, ALint *value)
{
    QueueEntry *pQE;
    ALint i;
    
	if (alIsSource(source))
	{
		switch(pname) 
		{
			case AL_CONE_INNER_ANGLE:
			case AL_CONE_OUTER_ANGLE:
				alSetError(AL_INVALID_ENUM); // not implemented yet
				break;
			case AL_LOOPING:
				*value=gSource[source].looping;
				break;
			case AL_BUFFER:
				*value=gSource[source].srcBufferNum;
				break;
			case AL_SOURCE_STATE:
			    *value=gSource[source].state;
			    break;
			case AL_BUFFERS_QUEUED:
			    pQE = gSource[source].ptrQueue;
			    i = 0;
			    while (pQE != NULL)
			    {
			    	i++;
			    	pQE = (QueueEntry *)pQE->pNext;
			    }
			    *value = i;
			    break;
			case AL_BUFFERS_PROCESSED:
				pQE = gSource[source].ptrQueue;
			    i = 0;
			    while (pQE != NULL)
			    {
			    	if (pQE->processed == AL_TRUE)
			    	{
			    		i++;
			    	}
			    	pQE = (QueueEntry *)pQE->pNext;
			    }
			    *value = i;
			    break;
			default:
				alSetError(AL_INVALID_ENUM);
				break;
		}
	}
}

ALAPI ALvoid ALAPIENTRY alSourcePlay(ALuint source)
{
    if (gSource[source].state != AL_PAUSED)
    {
        gSource[source].readOffset = 0;
    }	
    
	gSource[source].state = AL_PLAYING;
	smPlaySegment(source);
}

ALAPI ALvoid ALAPIENTRY alSourcePause (ALuint source)
{
	gSource[source].state = AL_PAUSED;
}

ALAPI ALvoid ALAPIENTRY alSourceStop (ALuint source)
{
	gSource[source].state = AL_STOPPED;
	gSource[source].readOffset = gBuffer[gSource[source].srcBufferNum].size;
	gSource[source].looping = 0;
}

ALAPI ALvoid ALAPIENTRY alSourceRewind (ALuint source)
{
    alSourceStop(source);
    gSource[source].state = AL_INITIAL;
    gSource[source].readOffset = 0; 
}

ALAPI ALvoid ALAPIENTRY alSourcePlayv(ALsizei n, ALuint *ID)
{
	if (n > 0)
	{
		while (n--)
		{
			alSourcePlay(ID[n]);
		}
	}
}

ALAPI ALvoid ALAPIENTRY alSourcePausev(ALsizei n, ALuint *ID)
{
	if (n > 0)
	{
		while (n--)
		{
			alSourcePause(ID[n]);
		}
	}
}

ALAPI ALvoid ALAPIENTRY alSourceStopv(ALsizei n, ALuint *ID)
{
	if (n > 0)
	{
		while (n--)
		{
			alSourceStop(ID[n]);
		}
	}
}

ALAPI ALvoid ALAPIENTRY alSourceRewindv (ALsizei n, ALuint *ID)
{
    if (n > 0)
    {
        while (n--)
        {
            alSourceRewind(ID[n]);
        }
    }
}

ALAPI ALvoid ALAPIENTRY alQueuei (ALuint source, ALenum param, ALint value)
{
	QueueEntry *ptrQE, *tempPtr;
	
	if (alIsSource(source))
	{
		switch(param) 
		{
			case AL_BUFFER:
				ptrQE = (void *)NewPtrClear(sizeof(QueueEntry));
				ptrQE->bufferNum = value;
				ptrQE->processed = AL_FALSE;
				ptrQE->pitch = gSource[source].pitch;
				ptrQE->gain = gSource[source].gain;
				ptrQE->loopDirection = AL_FALSE; // ***** need to implement real loop directions
				
				tempPtr = gSource[source].ptrQueue;
				if (tempPtr != NULL)
				{
					while (tempPtr->pNext != NULL)
					{
						tempPtr = tempPtr->pNext;
					}
					tempPtr->pNext = ptrQE;
				} else
				{
					gSource[source].ptrQueue = ptrQE;
				}
				break;
			default:
				alSetError(AL_INVALID_ENUM);
				break;
	
		}
	} else
	{
		alSetError(AL_INVALID_NAME);
	}
}

ALAPI ALvoid ALAPIENTRY alQueuef (ALuint source, ALenum param, ALfloat value)
{
	if (alIsSource(source))
	{
		switch(param) 
		{
			case AL_PITCH:
				if ((value>=0.5f)&&(value<=2.0f))
				{	
					// ***** gSource[source].pitch = value;
				}
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_GAIN:
				if (value <= 1.0f)
				{
					// ***** smSetSourceVolume (source, (value * kFullVolume), (value * kFullVolume));
				}	
				break;
			default:
				alSetError(AL_INVALID_OPERATION);
				break;
		}
	} else
	{
		alSetError(AL_INVALID_NAME);
	}
}

ALAPI ALvoid ALAPIENTRY alUnqueue (ALuint source, ALsizei n, ALuint *buffers)
{
 	ALsizei i, count;
 	ALint srcState;
	QueueEntry *tempQEPtr, *lastQEPtr;
	ALboolean exit;
	
	if (alIsSource(source))
	{
		alGetSourcei(source, AL_SOURCE_STATE, &srcState);
		
		count = 0;
		
		for (i = 0; i < n; i++)
		{
			// test to see if all queue entries can be deleted
			exit = AL_FALSE;
			tempQEPtr = gSource[source].ptrQueue;		
			while ((tempQEPtr != NULL) && (exit == AL_FALSE))
			{
				if (tempQEPtr->bufferNum == buffers[i])
				{
					if ((tempQEPtr->processed == AL_TRUE) || (srcState != AL_PLAYING))
					{
					    count++;
					    exit = AL_TRUE;	
					}
				}
				tempQEPtr = tempQEPtr->pNext;
			}
		}
		
		if (count == n) // array of queue entries is valid, so delete them
		{
			for (i = 0; i < n; i++)
			{
				exit = AL_FALSE;
				lastQEPtr = NULL;
				tempQEPtr = gSource[source].ptrQueue;		
				while ((tempQEPtr != NULL) && (exit == AL_FALSE))
				{
					if (tempQEPtr->bufferNum == buffers[i])
					{
						if ((tempQEPtr->processed == AL_TRUE) || (srcState != AL_PLAYING))
						{
					    	if (lastQEPtr == NULL)
					    	{
					    		gSource[source].ptrQueue = tempQEPtr->pNext;
					    	} else
					    	{
					    		lastQEPtr->pNext = tempQEPtr->pNext;
					    	}
					    	DisposePtr((char *)tempQEPtr);
					   	 	exit = AL_TRUE;	
						}
					}
					lastQEPtr = tempQEPtr;
					tempQEPtr = tempQEPtr->pNext;
				}
			}
		} else
		{
			alSetError(AL_INVALID_VALUE);
		}
	} else
	{
		alSetError(AL_INVALID_NAME);
	}			
}


