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

#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <al\alc.h>
#include <al\alu.h>
#include <OpenAL32\include\alBuffer.h>
#include "..\OpenAL32\Include\Eax.h"

static ALubyte *alcErrorStr[]=
{
	"There is no accessible sound device/driver/server.",
	"The Device argument does not name a valid device.",
	"The Context argument does not name a valid context.",
	"No error.",
};

static ALCcontext *Context=NULL;
static ALCuint ContextCount=0;
static ALCenum LastError=ALC_NO_ERROR;

// Multimedia Timer Callback function prototype
void CALLBACK TimerCallback(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

// Update Context functions
void UpdateSource(ALCcontext *ALContext, ALsource *ALSource);
void UpdateSourceEAXProperties(ALsource *ALSource);
void UpdateListener(ALCcontext *ALContext);
void UpdateListenerEAXProperties(ALCcontext *ALContext);

ALuint		g_nTimerInterval;
ALuint		g_nTimerID;

ALCAPI ALCvoid ALCAPIENTRY alcSuspendContext(ALCcontext *context)
{
	EnterCriticalSection(&context->Device->mutex);
}

ALCAPI ALCvoid ALCAPIENTRY alcProcessContext(ALCcontext *context)
{
	LeaveCriticalSection(&context->Device->mutex);
}

ALCAPI ALCenum ALCAPIENTRY alcGetError(ALCdevice *device)
{
	ALCenum errorCode;

	errorCode=LastError;
	LastError=AL_NO_ERROR;
	return errorCode;
}

ALCAPI ALCvoid ALCAPIENTRY alcSetError(ALenum errorCode)
{
	LastError=errorCode;
}

static void CALLBACK alcWaveOutProc(HWAVEOUT hDevice,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
	ALCcontext *ALContext;
	LPWAVEHDR WaveHdr;

	if (uMsg==WOM_DONE)
	{
		ALContext=alcGetCurrentContext();
		WaveHdr=((LPWAVEHDR)dwParam1);
		if (ALContext)
		{
			alcSuspendContext(ALContext);
			aluMixData(ALContext,WaveHdr->lpData,WaveHdr->dwBufferLength,ALContext->Format);
			alcProcessContext(ALContext);
		}
		else memset(WaveHdr->lpData,0,WaveHdr->dwBufferLength);
		waveOutWrite(hDevice,WaveHdr,sizeof(WAVEHDR));
	}
}

static void CALLBACK alcDirectSoundProc(UINT uID,UINT uReserved,DWORD dwUser,DWORD dwReserved1,DWORD dwReserved2)
{
	static DWORD OldWriteCursor=0;
	DWORD PlayCursor,WriteCursor;
	BYTE *WritePtr1,*WritePtr2;
	DWORD WriteCnt1,WriteCnt2;
	ALCcontext *ALContext;
	ALCdevice *ALDevice;
	DWORD BytesPlayed;
	HRESULT DSRes;

	ALDevice=(ALCdevice *)dwUser;
	IDirectSoundBuffer_GetCurrentPosition(ALDevice->DSsbuffer,&PlayCursor,&WriteCursor);
	if (!OldWriteCursor) OldWriteCursor=WriteCursor-PlayCursor;
	BytesPlayed=((((long)WriteCursor-(long)OldWriteCursor)<0)?((long)32768+(long)WriteCursor-(long)OldWriteCursor):((DWORD)WriteCursor-(DWORD)OldWriteCursor));
	DSRes=IDirectSoundBuffer_Lock(ALDevice->DSsbuffer,(OldWriteCursor+3528)&32767,BytesPlayed,&WritePtr1,&WriteCnt1,&WritePtr2,&WriteCnt2,0);
	if (DSRes==DSERR_BUFFERLOST)
	{
		IDirectSoundBuffer_Restore(ALDevice->DSsbuffer);
		IDirectSoundBuffer_Play(ALDevice->DSsbuffer,0,0,DSBPLAY_LOOPING);
		DSRes=IDirectSoundBuffer_Lock(ALDevice->DSsbuffer,(OldWriteCursor+3528)&32767,BytesPlayed,&WritePtr1,&WriteCnt1,&WritePtr2,&WriteCnt2,0);
	}
	if (DSRes==DS_OK)
	{
		ALContext=alcGetCurrentContext();
		if (ALContext)
		{
			alcSuspendContext(ALContext);
			if (WritePtr1)
				aluMixData(ALContext,WritePtr1,WriteCnt1,ALContext->Format);
			if (WritePtr2)
				aluMixData(ALContext,WritePtr2,WriteCnt2,ALContext->Format);
			alcProcessContext(ALContext);
		}
		else 
		{
			if (WritePtr1)
				memset(WritePtr1,0,WriteCnt1);
			if (WritePtr2)
				memset(WritePtr2,0,WriteCnt2);
		}
		IDirectSoundBuffer_Unlock(ALDevice->DSsbuffer,WritePtr1,WriteCnt1,WritePtr2,WriteCnt2);
	}
	OldWriteCursor=WriteCursor;
}

ALCAPI ALvoid ALCAPIENTRY alcInitContext(ALCcontext *context)
{
	TIMECAPS timeCaps;

	if (context)
	{
		//Lock context
		alcSuspendContext(context);
		//Initialise listener
		context->Listener.Gain=1.0f;
		context->Listener.Position[0]=0.0f;
		context->Listener.Position[1]=0.0f;
		context->Listener.Position[2]=0.0f;
		context->Listener.Velocity[0]=0.0f;
		context->Listener.Velocity[1]=0.0f;
		context->Listener.Velocity[2]=0.0f;
		context->Listener.Forward[0]=0.0f;
		context->Listener.Forward[1]=0.0f;
		context->Listener.Forward[2]=-1.0f;
		context->Listener.Up[0]=0.0f; 
		context->Listener.Up[1]=1.0f;
		context->Listener.Up[2]=0.0f;
		context->Listener.Environment=0;

		// Initialize update to set all the Listener parameters
		context->Listener.update1 = LPOSITION | LVELOCITY | LORIENTATION | LDOPPLERFACTOR | LROLLOFFFACTOR;

		// Default Reverb settings, but set to -100db
		context->Listener.eaxLP.lRoom = -10000;
		context->Listener.eaxLP.lRoomHF = EAXLISTENER_DEFAULTROOMHF;
		context->Listener.eaxLP.flRoomRolloffFactor = EAXLISTENER_DEFAULTROOMROLLOFFFACTOR;
		context->Listener.eaxLP.flDecayTime = EAXLISTENER_DEFAULTDECAYTIME;
		context->Listener.eaxLP.flDecayHFRatio = EAXLISTENER_DEFAULTDECAYHFRATIO;
		context->Listener.eaxLP.lReflections = EAXLISTENER_DEFAULTREFLECTIONS;
		context->Listener.eaxLP.flReflectionsDelay = EAXLISTENER_DEFAULTREFLECTIONSDELAY;
		context->Listener.eaxLP.lReverb = EAXLISTENER_DEFAULTREVERB;
		context->Listener.eaxLP.flReverbDelay = EAXLISTENER_DEFAULTREVERBDELAY;
		context->Listener.eaxLP.dwEnvironment = EAXLISTENER_DEFAULTENVIRONMENT;
		context->Listener.eaxLP.flEnvironmentSize = EAXLISTENER_DEFAULTENVIRONMENTSIZE;
		context->Listener.eaxLP.flEnvironmentDiffusion = EAXLISTENER_DEFAULTENVIRONMENTDIFFUSION;
		context->Listener.eaxLP.flAirAbsorptionHF = EAXLISTENER_DEFAULTAIRABSORPTIONHF;
		context->Listener.eaxLP.dwFlags = EAXLISTENER_DEFAULTFLAGS;

		context->Listener.update2 = LALLPARAMS;

		//Validate context
		context->LastError=AL_NO_ERROR;
		context->InUse=AL_FALSE;
		context->Valid=AL_TRUE;
		//Set output format
		context->Frequency=context->Device->Frequency;
		context->Channels=context->Device->Channels;
		context->Format=context->Device->Format;
		//Set globals
		context->DistanceModel=AL_INVERSE_DISTANCE_CLAMPED;
		context->DopplerFactor=1.0f;
		context->DopplerVelocity=1.0f;

		alcUpdateContext(context, ALLISTENER, 0);

		g_nTimerInterval = 100;
		g_nTimerID = 0;

		// Get the Timer Capabilities of the system
		timeGetDevCaps(&timeCaps, sizeof(TIMECAPS));

		// We want 100ms accuracy, if this is not possible, then just go with the best that we have
		if (timeCaps.wPeriodMin > g_nTimerInterval)
			g_nTimerInterval = timeCaps.wPeriodMin;

		// Begin Time Period
		timeBeginPeriod(g_nTimerInterval);
			
		//Unlock context
		alcProcessContext(context);
	}
}

ALCAPI ALCvoid ALCAPIENTRY alcExitContext(ALCcontext *context)
{
	if (context)
	{
		//Lock context
		alcSuspendContext(context);
		//Invalidate context
		context->LastError=AL_NO_ERROR;
		context->InUse=AL_FALSE;
		context->Valid=AL_TRUE;

		// Stop the multimedia timer
		if (g_nTimerID != 0)
			timeKillEvent(g_nTimerID);

		// End Timer Period
		timeEndPeriod(g_nTimerInterval);

		//Unlock context
		alcProcessContext(context);
	}
}

ALCAPI ALCcontext*ALCAPIENTRY alcCreateContext(ALCdevice *device,ALCint *attrList)
{
	ALCcontext *ALContext;

	if (!Context)
    {
		Context=malloc(sizeof(ALCcontext));
		if (Context)
		{
			memset(Context,0,sizeof(ALCcontext));
			Context->Device=device;
			Context->Valid=AL_TRUE;
			alcInitContext(Context);
			ContextCount++;
		}
		ALContext=Context;
	}
	else
	{
		ALContext=Context;
		while (ALContext->next)
			ALContext=ALContext->next;
		if (ALContext)
		{
			ALContext->next=malloc(sizeof(ALCcontext));
			if (ALContext->next)
			{
				memset(ALContext->next,0,sizeof(ALCcontext));
				ALContext->next->previous=ALContext;
				ALContext->next->Device=device;
				ALContext->next->Valid=AL_TRUE;
				alcInitContext(ALContext);
				ContextCount++;
			}
			ALContext=ALContext->next;
		}
	}
	return ALContext;
}

ALCAPI ALCvoid ALCAPIENTRY alcDestroyContext(ALCcontext *context)
{
	ALCcontext *ALContext;

	if (context)
	{
		ALContext=((ALCcontext *)context);
		alcExitContext(ALContext);
		if (ALContext->previous)
			ALContext->previous->next=ALContext->next;
		else
			Context=ALContext->next;
		if (ALContext->next)
			ALContext->next->previous=ALContext->previous;
		memset(ALContext,0,sizeof(ALCcontext));
		ContextCount--;
		free(ALContext);
	}
}

ALCAPI ALCcontext * ALCAPIENTRY alcGetCurrentContext(ALCvoid)
{
	ALCcontext *ALContext;

	ALContext=Context;
	while ((ALContext)&&(!ALContext->InUse))
		ALContext=ALContext->next;
	return ALContext;
}

ALCAPI ALCdevice* ALCAPIENTRY alcGetContextsDevice(ALCcontext *context)
{
	ALCdevice *ALDevice=NULL;
	ALCcontext *ALContext;

	ALContext=context;
	if (ALContext)
	{
		alcSuspendContext(ALContext);
		ALDevice=ALContext->Device;
		alcProcessContext(ALContext);
	}
	return ALDevice;
}

ALCAPI ALCboolean ALCAPIENTRY alcMakeContextCurrent(ALCcontext *context)
{
	ALCcontext *ALContext;
	
	if (ALContext=alcGetCurrentContext())
	{
		alcSuspendContext(ALContext);
		ALContext->InUse=AL_FALSE;
		alcProcessContext(ALContext);
	}
	if (ALContext=context)
	{
		alcSuspendContext(ALContext);
		ALContext->InUse=AL_TRUE;
		alcProcessContext(ALContext);
	}
	return AL_TRUE;
}


void CALLBACK TimerCallback(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	ALuint i, loop;
	ALuint PlayCursor, WriteCursor, DataToLock;
	ALuint BufferID, DataSize;
	ALuint Part1Size, Part2Size;
	ALuint Data, BytesWritten, DataLeft;
	ALuint DataPlayed, DataCount;
	ALuint BytesPlayed, BufferSize;
	ALvoid *lpPart1, *lpPart2;
	ALsource *ALSource;
	ALuint BuffersToSkip;
	ALCcontext *ALContext;
	ALbufferlistitem *ALBufferListItem;

	ALContext = (ALCcontext*)dwUser;

	alcSuspendContext(ALContext);

	ALSource = ALContext->Source;

	// Process each playing source
	for (loop=0;loop < ALContext->SourceCount;loop++)
	{
		if (alIsSource((ALuint)ALSource) && (ALSource->state == AL_PLAYING))
		{
			// Valid playing source

			// Get position in DS Buffer
			IDirectSoundBuffer_GetCurrentPosition((LPDIRECTSOUNDBUFFER)ALSource->uservalue1, &PlayCursor, &WriteCursor);

			// Lock buffer from Old Write cursor to current Play cursor
			if (ALSource->OldWriteCursor > PlayCursor)
				DataToLock = (88200 - ALSource->OldWriteCursor) + PlayCursor;
			else
				DataToLock = PlayCursor - ALSource->OldWriteCursor;
			
			// Calculate amount of data played since last Timer event
			if (ALSource->OldPlayCursor > PlayCursor)
				ALSource->BytesPlayed += ((88200 - ALSource->OldPlayCursor) + PlayCursor);
			else
				ALSource->BytesPlayed += (PlayCursor - ALSource->OldPlayCursor);

			// Update current buffer variable

			// Find position in queue
			BytesPlayed = ALSource->BytesPlayed;
			if (BytesPlayed >= ALSource->TotalBufferDataSize)
				BytesPlayed = (BytesPlayed % ALSource->TotalBufferDataSize);

			ALBufferListItem = ALSource->queue;
			DataSize = 0;
			while (ALBufferListItem != NULL)
			{
				alGetBufferi(ALBufferListItem->buffer, AL_SIZE, &BufferSize);
				DataSize += BufferSize;
				if (DataSize > BytesPlayed)
					break;
				else
					ALBufferListItem = ALBufferListItem->next;
			}

			BufferID = ALBufferListItem->buffer;
			ALSource->param[AL_BUFFER-AL_CONE_INNER_ANGLE].data.i = BufferID;

			// If we are not looping, decrement DataStillToPlay by the amount played since the last
			// Timer event, and check if any buffers in the queue have finished playing
			if (ALSource->param[AL_LOOPING-AL_CONE_INNER_ANGLE].data.i == AL_FALSE)
			{
				if (PlayCursor > ALSource->OldPlayCursor)
					ALSource->DataStillToPlay -= (PlayCursor - ALSource->OldPlayCursor);
				else
					ALSource->DataStillToPlay -= ((88200 - ALSource->OldPlayCursor) + PlayCursor);

				if (ALSource->DataStillToPlay < 0)
					ALSource->DataStillToPlay = 0;

				// Check if any buffers in the queue have finished playing - if they have adjust
				// their state to PROCESSED

				DataPlayed = ALSource->TotalBufferDataSize - ALSource->DataStillToPlay;

				DataCount = 0;
				ALSource->BuffersProcessed = 0;
				ALBufferListItem = ALSource->queue;
				while (ALBufferListItem != NULL)
				{
					alGetBufferi(ALBufferListItem->buffer, AL_SIZE, &DataSize);
					DataCount += DataSize;
					if (DataCount <= DataPlayed)
					{
						// Buffer has been played
						ALBufferListItem->bufferstate = PROCESSED;
						ALSource->BuffersProcessed++;
						ALBufferListItem = ALBufferListItem->next;
					}
					else
						ALBufferListItem = NULL;
				}
			}
			
			if (ALSource->FinishedQueue)
			{
				// Check if finished - if so stop source !
				if (ALSource->DataStillToPlay <= 0)
				{
					IDirectSoundBuffer_Stop((LPDIRECTSOUNDBUFFER)ALSource->uservalue1);
					IDirectSoundBuffer_SetCurrentPosition((LPDIRECTSOUNDBUFFER)ALSource->uservalue1, 0);
					ALSource->state = AL_STOPPED;

					// Reset variables
//					ALSource->BuffersAddedToDSBuffer = 0;
					ALSource->BufferPosition = 0;
					ALSource->BytesPlayed = 0;
					ALSource->DataStillToPlay = 0;
					ALSource->FinishedQueue = AL_FALSE;
					ALSource->OldPlayCursor = 0;
					ALSource->OldWriteCursor = 0;
					ALSource->SilenceAdded = 0;

					// Set current buffer to first buffer in queue
					if (ALSource->queue)
						BufferID = ALSource->queue->buffer;
					else
						BufferID = 0;
					ALSource->param[AL_BUFFER-AL_CONE_INNER_ANGLE].data.i = BufferID;
				}
				else
				{
					// Finished copying audio data into source, but data hasn't finished playing yet
					// Therefore copy silence into DS Buffer
					IDirectSoundBuffer_Lock((LPDIRECTSOUNDBUFFER)ALSource->uservalue1, ALSource->OldWriteCursor, DataToLock, &lpPart1, &Part1Size, &lpPart2, &Part2Size, 0);

					if (lpPart1)
					{
						memset(lpPart1, 0, Part1Size);
						ALSource->SilenceAdded += Part1Size;
						ALSource->OldWriteCursor += Part1Size;
							if (ALSource->OldWriteCursor >= 88200)
								ALSource->OldWriteCursor -= 88200;
					}
					if (lpPart2)
					{
						memset(lpPart2, 0, Part2Size);
						ALSource->SilenceAdded += Part2Size;
						ALSource->OldWriteCursor += Part2Size;
							if (ALSource->OldWriteCursor >= 88200)
								ALSource->OldWriteCursor -= 88200;
					}

					IDirectSoundBuffer_Unlock((LPDIRECTSOUNDBUFFER)ALSource->uservalue1, lpPart1, Part1Size, lpPart2, Part2Size);
				}
				
				// Update Old Play Cursor
				ALSource->OldPlayCursor = PlayCursor;
			}
			else
			{
				IDirectSoundBuffer_Lock((LPDIRECTSOUNDBUFFER)ALSource->uservalue1, ALSource->OldWriteCursor, DataToLock, &lpPart1, &Part1Size, &lpPart2, &Part2Size, 0);

				if (lpPart1 != NULL)
				{
					// Find position in buffer queue
					BuffersToSkip = ALSource->BuffersAddedToDSBuffer;
					if (BuffersToSkip >= ALSource->BuffersInQueue)
						BuffersToSkip = BuffersToSkip % ALSource->BuffersInQueue;

					ALBufferListItem = ALSource->queue;
					for (i = 0; i < BuffersToSkip; i++)
					{
						ALBufferListItem = ALBufferListItem->next;
					}

					BytesWritten = 0;
					BufferID = ALBufferListItem->buffer;

					while (AL_TRUE)
					{
						// Copy audio data from Open AL Buffer(s) into DS buffer
									
						// Find out how much data is left in current Open AL Buffer
						alGetBufferi(BufferID, AL_DATA, &Data);
						alGetBufferi(BufferID, AL_SIZE, &DataSize);
									
						if (DataSize == 0)
							DataLeft = 0;
						else
							DataLeft = DataSize - ALSource->BufferPosition;

						if (DataLeft > (Part1Size - BytesWritten))
						{
							// Copy (Part1Size - BytesWritten) bytes to Direct Sound buffer
							memcpy((ALubyte*)lpPart1 + BytesWritten, (ALubyte*)Data + ALSource->BufferPosition, Part1Size - BytesWritten);
							ALSource->FinishedQueue = AL_FALSE;	// More data to follow ...
							ALSource->BufferPosition += (Part1Size - BytesWritten);		// Record position in buffer data
							BytesWritten += (Part1Size - BytesWritten);
							break;
						}
						else
						{
							// Not enough data in buffer to fill DS buffer so just copy as much data as possible
							memcpy((ALubyte*)lpPart1 + BytesWritten, (ALubyte*)Data + ALSource->BufferPosition, DataLeft);

							BytesWritten += DataLeft;

							ALSource->BuffersAddedToDSBuffer++;

							ALSource->BufferPosition = 0;

							// Get next valid buffer ID
							ALBufferListItem = ALBufferListItem->next;
									
							if (ALBufferListItem == NULL)
							{
								// No more buffers - check for looping flag
								if (ALSource->param[AL_LOOPING-AL_CONE_INNER_ANGLE].data.i == AL_TRUE)
								{
									// Looping
									ALBufferListItem = ALSource->queue;
								}
								else
								{
									// Not looping and no more buffers
									ALSource->FinishedQueue = AL_TRUE;
									break;
								}
							}

							BufferID = ALBufferListItem->buffer;
						}
					}

					if (BytesWritten < Part1Size)
					{
						// Fill the rest of the buffer with silence
						memset((ALubyte*)lpPart1 + BytesWritten, 0, Part1Size - BytesWritten);
						ALSource->SilenceAdded += (Part1Size - BytesWritten);
					}

					ALSource->OldWriteCursor += Part1Size;
					if (ALSource->OldWriteCursor >= 88200)
						ALSource->OldWriteCursor -= 88200;
				}


				if (lpPart2 != NULL)
				{
					if (ALSource->FinishedQueue)
					{
						// Fill Part 2 with silence
						memset(lpPart2, 0, Part2Size);
					}
					else
					{
						// Find position in buffer queue
						BuffersToSkip = ALSource->BuffersAddedToDSBuffer;
						if (BuffersToSkip >= ALSource->BuffersInQueue)
							BuffersToSkip = BuffersToSkip % ALSource->BuffersInQueue;

						ALBufferListItem = ALSource->queue;
						for (i = 0; i < BuffersToSkip; i++)
						{
							ALBufferListItem = ALBufferListItem->next;
						}

						BytesWritten = 0;
						BufferID = ALBufferListItem->buffer;

						while (AL_TRUE)
						{
							// Copy audio data from Open AL Buffer(s) into DS buffer
									
							// Find out how much data is left in current Open AL Buffer
							alGetBufferi(BufferID, AL_DATA, &Data);
							alGetBufferi(BufferID, AL_SIZE, &DataSize);
									
							if (DataSize == 0)
								DataLeft = 0;
							else
								DataLeft = DataSize - ALSource->BufferPosition;
		
							if (DataLeft > (Part2Size - BytesWritten))
							{
								// Copy (Part1Size - BytesWritten) bytes to Direct Sound buffer
								memcpy((ALubyte*)lpPart2 + BytesWritten, (ALubyte*)Data + ALSource->BufferPosition, Part2Size - BytesWritten);
								ALSource->FinishedQueue = AL_FALSE;	// More data to follow ...
								ALSource->BufferPosition += (Part2Size - BytesWritten);		// Record position in buffer data
								BytesWritten += (Part2Size - BytesWritten);
								break;
							}
							else
							{
								// Not enough data in buffer to fill DS buffer so just copy as much data as possible
								memcpy((ALubyte*)lpPart2 + BytesWritten, (ALubyte*)Data + ALSource->BufferPosition, DataLeft);

								BytesWritten += DataLeft;

								ALSource->BuffersAddedToDSBuffer++;

								ALSource->BufferPosition = 0;

								// Get next valid buffer ID
								ALBufferListItem = ALBufferListItem->next;
									
								if (ALBufferListItem == NULL)
								{
									// No more buffers - check for looping flag
									if (ALSource->param[AL_LOOPING-AL_CONE_INNER_ANGLE].data.i == AL_TRUE)
									{
										// Looping
										ALBufferListItem = ALSource->queue;
									}
									else
									{
										// Not looping and no more buffers
										ALSource->FinishedQueue = AL_TRUE;
										break;
									}
								}

								BufferID = ALBufferListItem->buffer;
							}
						}

						if (BytesWritten < Part2Size)
						{
							// Fill the rest of the buffer with silence
							memset((ALubyte*)lpPart2 + BytesWritten, 0, Part2Size - BytesWritten);
							ALSource->SilenceAdded += (Part2Size - BytesWritten);
						}

						ALSource->OldWriteCursor += Part2Size;
						if (ALSource->OldWriteCursor >= 88200)
							ALSource->OldWriteCursor -= 88200;
					}
				}

				ALSource->OldPlayCursor = PlayCursor;

				IDirectSoundBuffer_Unlock((LPDIRECTSOUNDBUFFER)ALSource->uservalue1, lpPart1, Part1Size, lpPart2, Part2Size);
			} // end else
		}

		ALSource = ALSource->next;
	}

	alcProcessContext(ALContext);

	return;
}



ALCAPI ALCvoid ALCAPIENTRY alcUpdateContext(ALCcontext *context, ALuint type, ALuint name)
{
	ALCcontext *ALContext;
	ALsource *ALSource;
    ALuint status;
	
	ALContext = context;
	ALSource = (ALsource*)name;

	alcSuspendContext(ALContext);

	//Platform specific context updating
	if ((ALContext->Device->DShandle)&&(ALContext->Device->DS3dlistener))
	{
		// Check if we need to update a Source
		if ((type == ALSOURCE) && (alIsSource(name)) && ((ALSource->update1) || (ALSource->update2)))
		{
			// First check for any Open AL Updates (e.g Position, Velocity, Looping etc ...)
			if (ALSource->update1)
				UpdateSource(ALContext, ALSource);

			// Secondly check for any EAX 2 Buffer updates
			if (ALSource->update2)
				UpdateSourceEAXProperties(ALSource);

			// If we need to actually start playing the sound, do it now
			if (ALSource->play)
			{	
				if (ALSource->uservalue1)
				{
					// Start playing the DS Streaming buffer (always looping)
					IDirectSoundBuffer_Play((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,0,0,DSBPLAY_LOOPING);
					ALSource->play=AL_FALSE;
				}
			}

			// Update Source's status
			if (ALSource->uservalue1)
			{
				IDirectSoundBuffer_GetStatus((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,&status);
				ALSource->state=((status&DSBSTATUS_PLAYING)?AL_PLAYING:AL_STOPPED);
			}
		}

		// Check for Listener related updates
		if ((type == ALLISTENER) && (ALContext->Device->DS3dlistener))
		{
			// Update any Open AL Listener Properties (e.g Position, Velocity, Orientation etc ...)
			if (ALContext->Listener.update1)
				UpdateListener(ALContext);

			// Update any EAX Listener Properties
			if (ALContext->Listener.update2)
				UpdateListenerEAXProperties(ALContext);
		}

	}

	alcProcessContext(ALContext);
}


/*
	Update Source
*/
void UpdateSource(ALCcontext *ALContext, ALsource *ALSource)
{
	WAVEFORMATEX OutputType;
	DSBUFFERDESC DSBDescription;
	ALfloat Dir[3], Pos[3], Vel[3];
	ALuint	DataStillToPlay, BytesWritten, DataLeft, BytesPlayed, BuffersToSkip, DataPlayed, DataCount, TotalDataSize;
	ALint	BufferSize, DataCommitted;
	ALint	Relative;
	ALuint	Data, Freq, State, Channels, outerAngle, innerAngle;
	ALfloat Pitch, outerGain, maxDist, minDist, Gain;
	ALvoid *lpPart1;
	ALuint	Part1Size, DataSize;
	ALuint PlayCursor, WriteCursor;
	ALuint BufferID, status, Loop, i;
	ALbufferlistitem *ALBufferListItem;
	ALbufferlistitem *ALBufferListTemp;
		

	if (ALSource->update1 & SUPDATEBUFFERS)
	{
		// Force an update to recalculate the number of buffers processed, and the current buffer
		if (ALSource->state	== AL_PLAYING)
		{
			// Get position in DS Buffer
			IDirectSoundBuffer_GetCurrentPosition((LPDIRECTSOUNDBUFFER)ALSource->uservalue1, &PlayCursor, &WriteCursor);
	
			BytesPlayed = ALSource->BytesPlayed;

			// Calculate amount of data played
			if (ALSource->OldPlayCursor > PlayCursor)
				BytesPlayed += ((88200 - ALSource->OldPlayCursor) + PlayCursor);
			else
				BytesPlayed += (PlayCursor - ALSource->OldPlayCursor);

			// Update current buffer variable

			// Find position in queue
			if (BytesPlayed >= ALSource->TotalBufferDataSize)
				BytesPlayed = (BytesPlayed % ALSource->TotalBufferDataSize);

			ALBufferListItem = ALSource->queue;
			DataSize = 0;
			while (ALBufferListItem != NULL)
			{
				alGetBufferi(ALBufferListItem->buffer, AL_SIZE, &BufferSize);
				DataSize += BufferSize;
				if (DataSize > BytesPlayed)
					break;
				else
					ALBufferListItem = ALBufferListItem->next;
			}

			BufferID = ALBufferListItem->buffer;
			ALSource->param[AL_BUFFER-AL_CONE_INNER_ANGLE].data.i = BufferID;

			DataStillToPlay = ALSource->DataStillToPlay;

			// If we are not looping, calculate how many buffers have been processed
			if (ALSource->param[AL_LOOPING-AL_CONE_INNER_ANGLE].data.i == AL_FALSE)
			{
				if (PlayCursor > ALSource->OldPlayCursor)
					DataStillToPlay -= (PlayCursor - ALSource->OldPlayCursor);
				else
					DataStillToPlay -= ((88200 - ALSource->OldPlayCursor) + PlayCursor);

				if (DataStillToPlay < 0)
					DataStillToPlay = 0;

				// Check if any buffers in the queue have finished playing - if they have adjust
				// their state to PROCESSED

				DataPlayed = ALSource->TotalBufferDataSize - DataStillToPlay;

				DataCount = 0;
				ALSource->BuffersProcessed = 0;
				ALBufferListItem = ALSource->queue;
				while (ALBufferListItem != NULL)
				{
					alGetBufferi(ALBufferListItem->buffer, AL_SIZE, &DataSize);
					DataCount += DataSize;
					if (DataCount <= DataPlayed)
					{
						// Buffer has been played
						ALBufferListItem->bufferstate = PROCESSED;
						ALSource->BuffersProcessed++;
						ALBufferListItem = ALBufferListItem->next;
					}
				else
					ALBufferListItem = NULL;
				}
			}
		}
		ALSource->update1 &= ~SUPDATEBUFFERS;
		if (ALSource->update1 == 0)
				return;
	}
	
	// Check if the Source is being Destroyed
	if (ALSource->update1 == SDELETE)
	{				
		// Destroy source
		if (ALSource->uservalue1)
		{
			IDirectSoundBuffer_Stop((LPDIRECTSOUNDBUFFER)ALSource->uservalue1);
			if (ALSource->uservalue3)
			{
				IKsPropertySet_Release((LPKSPROPERTYSET)ALSource->uservalue3);
				ALSource->uservalue3 = NULL;
			}
			if (ALSource->uservalue2)
			{
				IDirectSound3DBuffer_Release((LPDIRECTSOUND3DBUFFER)ALSource->uservalue2);
				ALSource->uservalue2 = NULL;
			}
			IDirectSoundBuffer_Release((LPDIRECTSOUNDBUFFER)ALSource->uservalue1);
			ALSource->uservalue1=NULL;
			ALSource->update1 &= ~SDELETE;
			if (ALSource->update1 == 0)
				return;
		}
	}

	// Check if we need to generate a new Source
	if (ALSource->update1 & SGENERATESOURCE)
	{
		// Create a streaming DS buffer - 16bit mono 44.1KHz, 1 second in length
		memset(&DSBDescription,0,sizeof(DSBUFFERDESC));
		DSBDescription.dwSize=sizeof(DSBUFFERDESC);
		DSBDescription.dwFlags=DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY|DSBCAPS_CTRL3D|DSBCAPS_GLOBALFOCUS;
		DSBDescription.dwBufferBytes=88200;
		DSBDescription.lpwfxFormat=&OutputType;
		memset(&OutputType,0,sizeof(WAVEFORMATEX));
		OutputType.wFormatTag=WAVE_FORMAT_PCM;
		OutputType.nChannels=1;
		OutputType.wBitsPerSample=16;
		OutputType.nBlockAlign=2;
		OutputType.nSamplesPerSec=44100;
		OutputType.nAvgBytesPerSec=88200;
		OutputType.cbSize=0;
		if (IDirectSound_CreateSoundBuffer(ALContext->Device->DShandle,&DSBDescription,&(LPDIRECTSOUNDBUFFER)ALSource->uservalue1,NULL)==DS_OK)
		{
			IDirectSoundBuffer_SetCurrentPosition((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,0);

			// Get 3D Interface
			if (IDirectSoundBuffer_QueryInterface((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,&IID_IDirectSound3DBuffer,(LPUNKNOWN *)&ALSource->uservalue2)==DS_OK)
			{
				// Get Property Set Interface
				IDirectSound3DBuffer_QueryInterface((LPDIRECTSOUND3DBUFFER)ALSource->uservalue2,&IID_IKsPropertySet,(LPUNKNOWN *)&ALSource->uservalue3);
			}
		}

		ALSource->update1 &= ~SGENERATESOURCE;
		if (ALSource->update1 == 0)
				return;
	}

	// Check if we need to Stop, Start, Pause, or Resume a Source
	if (ALSource->update1 & STATE)
	{
		alGetSourcei((ALuint)ALSource,AL_SOURCE_STATE,&State);

		switch (State)
		{
			case AL_INITIAL:
				if (ALSource->uservalue1)
					IDirectSoundBuffer_SetCurrentPosition((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,0);
				break;
			case AL_PLAYING:
				if (ALSource->play)
				{							
					// Check if this Source is already playing
					IDirectSoundBuffer_GetStatus((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,&status);
					
					// If already playing, stop source, set position to zero, mark all items in 
					// queue as PENDING ready to start playback
					if (status & DSBSTATUS_PLAYING)
					{
						IDirectSoundBuffer_Stop((LPDIRECTSOUNDBUFFER)ALSource->uservalue1);
						IDirectSoundBuffer_SetCurrentPosition((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,0);
						
						// Mark all buffers in the queue as PENDING
						ALBufferListItem = ALSource->queue;
						while (ALBufferListItem != NULL)
						{
							ALBufferListItem->bufferstate = PENDING;
							ALBufferListItem = ALBufferListItem->next;
						}

						ALSource->BuffersProcessed = 0;
						ALSource->BuffersAddedToDSBuffer = 0;
						ALSource->BufferPosition = 0;
						ALSource->BytesPlayed = 0;
						ALSource->DataStillToPlay = 0;
						ALSource->FinishedQueue = AL_FALSE;
						ALSource->OldPlayCursor = 0;
						ALSource->OldWriteCursor = 0;
					}

					// Check that we have some data to play
					if (ALSource->BuffersInQueue == 0)
					{
						ALSource->play = AL_FALSE;
						break;
					}

					// If looping has been enabled, make sure that all buffers are PENDING
					alGetSourcei((ALuint)ALSource, AL_LOOPING, &Loop);
					if (Loop == AL_TRUE)
					{
						ALBufferListItem = ALSource->queue;
						while (ALBufferListItem != NULL)
						{
							ALBufferListItem->bufferstate = PENDING;
							ALBufferListItem = ALBufferListItem->next;
						}
						ALSource->BuffersProcessed = 0;
					}

					// Find position in buffer queue
					BuffersToSkip = ALSource->BuffersAddedToDSBuffer;
					if (BuffersToSkip > ALSource->BuffersInQueue)
						BuffersToSkip = BuffersToSkip % ALSource->BuffersInQueue;

					ALBufferListItem = ALSource->queue;
					for (i = 0; i < BuffersToSkip; i++)
					{
						ALBufferListItem = ALBufferListItem->next;
					}

					// Mark any buffers in the list as processed if they have bufferID == 0
					if ((ALBufferListItem != NULL) && (ALBufferListItem->buffer == 0))
					{
						ALBufferListItem->bufferstate = PROCESSED;
						ALSource->BuffersProcessed++;
						ALBufferListItem = ALBufferListItem->next;
						ALSource->BuffersAddedToDSBuffer++;
					}

					if (ALBufferListItem == NULL)
					{
						// No buffers to play - remove play flag
						ALSource->play = AL_FALSE;
						break;
					}

					// Start multimedia timer (if not already in progress)
					if (g_nTimerID == 0)
						g_nTimerID = timeSetEvent(g_nTimerInterval, 0, &TimerCallback, (DWORD)ALContext, TIME_CALLBACK_FUNCTION | TIME_PERIODIC);

					// Update current buffer variable
					BufferID = ALBufferListItem->buffer;
					ALSource->param[AL_BUFFER-AL_CONE_INNER_ANGLE].data.i = BufferID;

					// Update variables
					ALSource->BytesPlayed = 0;
					ALSource->OldWriteCursor = 0;
					ALSource->OldPlayCursor = 0;

					// Calculate how much data still to play
					alGetBufferi(BufferID, AL_SIZE, &DataSize);
					ALSource->DataStillToPlay = DataSize - ALSource->BufferPosition;

					ALBufferListTemp = ALBufferListItem;

					while (ALBufferListTemp->next != NULL)
					{
						alGetBufferi(ALBufferListTemp->next->buffer, AL_SIZE, &DataSize);
						ALSource->DataStillToPlay += DataSize;
						ALBufferListTemp = ALBufferListTemp->next;
					}
					

					// Check if the buffer is stereo
					alGetBufferi(BufferID, AL_CHANNELS, &Channels);
					if ((Channels == 2) && (ALSource->SourceType == SOURCE3D))
					{
						// Playing a stereo buffer

						// Need to destroy the DS Streaming Mono 3D Buffer and create a Stereo 2D buffer
						if (ALSource->uservalue3)
						{
							IKsPropertySet_Release((LPKSPROPERTYSET)ALSource->uservalue3);
							ALSource->uservalue3 = NULL;
						}
						if (ALSource->uservalue2)
						{
							IDirectSound3DBuffer_Release((LPDIRECTSOUND3DBUFFER)ALSource->uservalue2);
							ALSource->uservalue2 = NULL;
						}
						IDirectSoundBuffer_Release((LPDIRECTSOUNDBUFFER)ALSource->uservalue1);
						ALSource->uservalue1=NULL;

						// Set Caps
						memset(&DSBDescription,0,sizeof(DSBUFFERDESC));
						DSBDescription.dwSize=sizeof(DSBUFFERDESC);
						DSBDescription.dwFlags=DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY|DSBCAPS_GLOBALFOCUS;
						DSBDescription.dwBufferBytes=88200;
						DSBDescription.lpwfxFormat=&OutputType;
						memset(&OutputType,0,sizeof(WAVEFORMATEX));
						OutputType.wFormatTag=WAVE_FORMAT_PCM;
						OutputType.nChannels=2;
						OutputType.wBitsPerSample=16;
						OutputType.nBlockAlign=4;
						OutputType.nSamplesPerSec=44100;
						OutputType.nAvgBytesPerSec=176400;
						OutputType.cbSize=0;
						if (IDirectSound_CreateSoundBuffer(ALContext->Device->DShandle,&DSBDescription,&(LPDIRECTSOUNDBUFFER)ALSource->uservalue1,NULL)==DS_OK)
						{
							IDirectSoundBuffer_SetCurrentPosition((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,0);
						}

						// Set correct volume for new DS Buffer
						if (ALSource->uservalue1)
						{
							alGetSourcef((ALuint)ALSource,AL_GAIN,&Gain);
							Gain = (Gain * ALContext->Listener.Gain);
							if (Gain > 0)
								IDirectSoundBuffer_SetVolume((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,(long)(2000.0*log10(Gain)));
							else
								IDirectSoundBuffer_SetVolume((LPDIRECTSOUNDBUFFER)ALSource->uservalue1, -10000);
						}

						ALSource->SourceType = SOURCE2D;
					}
					else if ((Channels == 1) && (ALSource->SourceType == SOURCE2D))
					{
						// Playing a (3D) Mono buffer

						// Need to destroy the stereo streaming buffer and create a 3D mono one instead
						if (ALSource->uservalue1)
						{
							IDirectSoundBuffer_Release((LPDIRECTSOUNDBUFFER)ALSource->uservalue1);
							ALSource->uservalue1=NULL;
						}

						// Set Caps
						memset(&DSBDescription,0,sizeof(DSBUFFERDESC));
						DSBDescription.dwSize=sizeof(DSBUFFERDESC);
						DSBDescription.dwFlags=DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY|DSBCAPS_CTRL3D|DSBCAPS_GLOBALFOCUS;
						DSBDescription.dwBufferBytes=88200;
						DSBDescription.lpwfxFormat=&OutputType;
						memset(&OutputType,0,sizeof(WAVEFORMATEX));
						OutputType.wFormatTag=WAVE_FORMAT_PCM;
						OutputType.nChannels=1;
						OutputType.wBitsPerSample=16;
						OutputType.nBlockAlign=2;
						OutputType.nSamplesPerSec=44100;
						OutputType.nAvgBytesPerSec=88200;
						OutputType.cbSize=0;

						if (IDirectSound_CreateSoundBuffer(ALContext->Device->DShandle,&DSBDescription,&(LPDIRECTSOUNDBUFFER)ALSource->uservalue1,NULL)==DS_OK)
						{
							IDirectSoundBuffer_SetCurrentPosition((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,0);

							// Get 3D Interface
							if (IDirectSoundBuffer_QueryInterface((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,&IID_IDirectSound3DBuffer,(LPUNKNOWN *)&ALSource->uservalue2)==DS_OK)
							{
								// Get Property Set Interface
								IDirectSound3DBuffer_QueryInterface((LPDIRECTSOUND3DBUFFER)ALSource->uservalue2,&IID_IKsPropertySet,(LPUNKNOWN *)&ALSource->uservalue3);
							}
						}


						// Set correct volume for new DS Buffer
						if (ALSource->uservalue1)
						{
							alGetSourcef((ALuint)ALSource,AL_GAIN,&Gain);
							Gain = (Gain * ALContext->Listener.Gain);
							if (Gain > 0)
								IDirectSoundBuffer_SetVolume((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,(long)(2000.0*log10(Gain)));
							else
								IDirectSoundBuffer_SetVolume((LPDIRECTSOUNDBUFFER)ALSource->uservalue1, -10000);
						}

						ALSource->SourceType = SOURCE3D;
					}

					// Set Direct Sound buffer to frequency of current Open AL buffer multiplied by desired Pitch
					alGetBufferi(BufferID,AL_FREQUENCY, &Freq);
					alGetSourcef((ALuint)ALSource, AL_PITCH, &Pitch);

					IDirectSoundBuffer_SetFrequency((LPDIRECTSOUNDBUFFER)ALSource->uservalue1, (long)(Freq*Pitch));

					// Lock the whole DS buffer
					IDirectSoundBuffer_Lock((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,0,0,&lpPart1,&Part1Size,NULL,NULL,DSBLOCK_ENTIREBUFFER);
						
					BytesWritten = 0;

					while (AL_TRUE)
					{
						// Copy audio data from Open AL Buffer into DS buffer
							
						// Find out how much data is left in current Open AL Buffer
						alGetBufferi(BufferID, AL_DATA, &Data);
						alGetBufferi(BufferID, AL_SIZE, &DataSize);
							
						if (DataSize == 0)
							DataLeft = 0;
						else
							DataLeft = DataSize - ALSource->BufferPosition;

						if (DataLeft > (88200 - BytesWritten))
						{
							// Copy (88200 - BytesWritten) bytes to Direct Sound buffer
							memcpy((ALubyte*)lpPart1 + BytesWritten, (ALubyte*)Data + ALSource->BufferPosition, 88200 - BytesWritten);
							ALSource->FinishedQueue = AL_FALSE;	// More data to follow ...
							ALSource->BufferPosition += (88200 - BytesWritten);		// Record position in buffer data
							BytesWritten += (88200 - BytesWritten);
							break;
						}
						else
						{
							// Not enough data in buffer to fill DS buffer so just copy as much data as possible
							memcpy((ALubyte*)lpPart1 + BytesWritten, (ALubyte*)Data + ALSource->BufferPosition, DataLeft);

							BytesWritten += DataLeft;

							ALSource->BuffersAddedToDSBuffer++;

							ALSource->BufferPosition = 0;

							// Get next valid buffer ID
							ALBufferListItem = ALBufferListItem->next;
								
							if (ALBufferListItem == NULL)
							{
								// No more buffers - check for looping flag
								if (ALSource->param[AL_LOOPING-AL_CONE_INNER_ANGLE].data.i == AL_TRUE)
								{
									// Looping
									ALBufferListItem = ALSource->queue;
								}
								else
								{
									// Not looping and no more buffers
									break;
								}
							}

							BufferID = ALBufferListItem->buffer;
						}
					}

					if (BytesWritten < 88200)
					{
						// Fill the rest of the buffer with silence
						memset((ALubyte*)lpPart1 + BytesWritten, 0, 88200 - BytesWritten);
						ALSource->SilenceAdded = 88200 - BytesWritten;
						ALSource->FinishedQueue = AL_TRUE;		// Set this to true to indicate no more data needs to be copied into buffer
					}

					IDirectSoundBuffer_Unlock((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,lpPart1,Part1Size,NULL,0);						
				}
				break;

			case AL_PAUSED:
				if (ALSource->uservalue1)
				{
					IDirectSoundBuffer_Stop((LPDIRECTSOUNDBUFFER)ALSource->uservalue1);
					ALSource->paused = AL_TRUE;
				}
				break;
			case AL_STOPPED:
				if (ALSource->uservalue1)
				{
					IDirectSoundBuffer_Stop((LPDIRECTSOUNDBUFFER)ALSource->uservalue1);
					IDirectSoundBuffer_SetCurrentPosition((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,0);
					// Re-set variables
//					ALSource->BuffersAddedToDSBuffer = 0;
					ALSource->BufferPosition = 0;
					ALSource->BytesPlayed = 0;
					ALSource->DataStillToPlay = 0;
					ALSource->FinishedQueue = AL_FALSE;
					ALSource->OldPlayCursor = 0;
					ALSource->OldWriteCursor = 0;
					ALSource->SilenceAdded = 0;

					// Set current buffer to first buffer in queue
					if (ALSource->queue)
						BufferID = ALSource->queue->buffer;
					else
						BufferID = 0;
					ALSource->param[AL_BUFFER-AL_CONE_INNER_ANGLE].data.i = BufferID;
				}

				// Mark all buffers in queue as PROCESSED
				ALSource->BuffersProcessed = ALSource->BuffersInQueue;

				ALBufferListItem= ALSource->queue;
				while (ALBufferListItem != NULL)
				{
					ALBufferListItem->bufferstate = PROCESSED;
					ALBufferListItem = ALBufferListItem->next;
				}

				break;
		}

		// End of STATE update
		ALSource->update1 &= ~STATE;
		if (ALSource->update1 == 0)
				return;
	}


	// Check if we need to update the 3D Position of the Source
	if (ALSource->update1 & POSITION)
	{
		if (ALSource->uservalue2)
		{
			alGetSourcefv((ALuint)ALSource,AL_POSITION,Pos);
			IDirectSound3DBuffer_SetPosition((LPDIRECTSOUND3DBUFFER)ALSource->uservalue2,Pos[0],Pos[1],-Pos[2],DS3D_IMMEDIATE);
			ALSource->update1 &= ~POSITION;
			if (ALSource->update1 == 0)
				return;
		}
	}


	// Check if we need to ajust the velocity of the Source
	if (ALSource->update1 & VELOCITY)
	{
		if (ALSource->uservalue2)
		{
			alGetSourcefv((ALuint)ALSource,AL_VELOCITY,Vel);
			IDirectSound3DBuffer_SetVelocity((LPDIRECTSOUND3DBUFFER)ALSource->uservalue2,Vel[0],Vel[1],-Vel[2],DS3D_IMMEDIATE);
			ALSource->update1 &= ~VELOCITY;
			if (ALSource->update1 == 0)
				return;
		}
	}


	// Check if we need to adjust the Orientation of the Source
	if (ALSource->update1 & ORIENTATION)
	{
		if (ALSource->uservalue2)
		{
			alGetSourcefv((ALuint)ALSource,AL_DIRECTION,Dir);
			IDirectSound3DBuffer_SetConeOrientation((LPDIRECTSOUND3DBUFFER)ALSource->uservalue2,Dir[0],Dir[1],-Dir[2],DS3D_IMMEDIATE);
			ALSource->update1 &= ~ORIENTATION;
			if (ALSource->update1 == 0)
				return;
		}
	}

	
	// Check if any Buffers have been added to this Source's queue
	if (ALSource->update1 & SQUEUE)
	{
		if ((ALSource->uservalue1) && (ALSource->state == AL_PLAYING))
		{
			// Some buffer(s) have been added to the queue

			// If silence has been added, then we need to overwrite the silence with new audio
			// data from the buffers in the queue
			if (ALSource->SilenceAdded > 0)
			{
				// Find position of end of valid data
				if (ALSource->OldWriteCursor < ALSource->SilenceAdded)
					ALSource->OldWriteCursor += 88200;

				// Set new Write position to the end of the valid data
				ALSource->OldWriteCursor -= ALSource->SilenceAdded;

				// Read position from next buffer should be set to 0
				ALSource->BufferPosition = 0;

				// We have overwritten the silent data, so reset variable
				ALSource->SilenceAdded = 0;

				// Make sure that the we haven't finished processing the queue !
				ALSource->FinishedQueue = AL_FALSE;
			}
			// Update DataStillToPlay
			ALSource->DataStillToPlay += ALSource->SizeOfBufferDataAddedToQueue;
		}
		ALSource->TotalBufferDataSize += ALSource->SizeOfBufferDataAddedToQueue;
		ALSource->SizeOfBufferDataAddedToQueue = 0;
		ALSource->NumBuffersAddedToQueue = 0;
		ALSource->update1 &= ~SQUEUE;
		if (ALSource->update1 == 0)
				return;
	}

	// Check if any Buffers have been removed from this Source's Queue
	if (ALSource->update1 & SUNQUEUE)
	{
		// Some number of buffers have been removed from the queue

		// We need to update some variables to correctly reflect the new queue

		// The number of BuffersAddedToDSBuffers must be decreased by the number of buffers
		// removed from the queue (or else the Timer function will think we are further through
		// the list than we are)

		// The amount of DataPlayed must be decreased by the total size of the data in the buffers
		// removed from the queue (or the amount of data still to play (TotalDataSize - DataPlayed)
		// will be incorrect)
		if ((ALSource->uservalue1) && (ALSource->state == AL_PLAYING))
		{
//			ALSource->BuffersAddedToDSBuffer -= ALSource->NumBuffersRemovedFromQueue;
			ALSource->BytesPlayed -= ALSource->SizeOfBufferDataRemovedFromQueue;
		}
		ALSource->TotalBufferDataSize -= ALSource->SizeOfBufferDataRemovedFromQueue;
		ALSource->NumBuffersRemovedFromQueue = 0;
		ALSource->SizeOfBufferDataRemovedFromQueue = 0;

		// If we're not playing then reset current buffer (it may have changed)
		if (ALSource->state != AL_PLAYING)
		{
			if (ALSource->queue)
				BufferID = ALSource->queue->buffer;
			else
				BufferID = 0;

			ALSource->param[AL_BUFFER-AL_CONE_INNER_ANGLE].data.i = BufferID;
		}

		ALSource->update1 &= ~SUNQUEUE;
		if (ALSource->update1 == 0)
				return;
	}

	// Check if we need to adjust the volume of the Source
	if (ALSource->update1 & VOLUME)
	{
		if (ALSource->uservalue1)
		{
			alGetSourcef((ALuint)ALSource,AL_GAIN,&Gain);
			Gain = (Gain * ALContext->Listener.Gain);
			if (Gain > 0)
				IDirectSoundBuffer_SetVolume((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,(long)(2000.0*log10(Gain)));
			else
				IDirectSoundBuffer_SetVolume((LPDIRECTSOUNDBUFFER)ALSource->uservalue1, -10000);

			ALSource->update1 &= ~VOLUME;
			if (ALSource->update1 == 0)
				return;
		}
	}


	// Check if we need to adjust the frequency of the Source
	if (ALSource->update1 & FREQUENCY)
	{
		if (ALSource->uservalue1)
		{
			BufferID = ALSource->param[AL_BUFFER-AL_CONE_INNER_ANGLE].data.i;
			if (BufferID == 0)
				Freq = 44100;
			else
				alGetBufferi(BufferID,AL_FREQUENCY,&Freq);
			alGetSourcef((ALuint)ALSource,AL_PITCH,&Pitch);

			IDirectSoundBuffer_SetFrequency((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,(long)(Freq*Pitch));
			ALSource->update1 &= ~FREQUENCY;
			if (ALSource->update1 == 0)
				return;
		}
	}


	// Check if we need to adjust the Min Distance of the Source
	if (ALSource->update1 & MINDIST)
	{
		if (ALSource->uservalue2)
		{
			alGetSourcef((ALuint)ALSource,AL_REFERENCE_DISTANCE,&minDist);
			IDirectSound3DBuffer_SetMinDistance((LPDIRECTSOUND3DBUFFER)ALSource->uservalue2,minDist,DS3D_IMMEDIATE);
			ALSource->update1 &= ~MINDIST;
			if (ALSource->update1 == 0)
				return;
		}
	}


	// Check if we need to adjust the Max Distance of the Source
	if (ALSource->update1 & MAXDIST)
	{
		if (ALSource->uservalue2)
		{
			alGetSourcef((ALuint)ALSource,AL_MAX_DISTANCE,&maxDist);
			IDirectSound3DBuffer_SetMaxDistance((LPDIRECTSOUND3DBUFFER)ALSource->uservalue2,maxDist,DS3D_IMMEDIATE);
			ALSource->update1 &= ~MAXDIST;
			if (ALSource->update1 == 0)
				return;
		}
	}


	// Check if we need to adjust the Cone Outside Volume of the Source
	if (ALSource->update1 & CONEOUTSIDEVOLUME)
	{
		if (ALSource->uservalue2)
		{
			alGetSourcef((ALuint)ALSource,AL_CONE_OUTER_GAIN,&outerGain);
			IDirectSound3DBuffer_SetConeOutsideVolume((LPDIRECTSOUND3DBUFFER)ALSource->uservalue2,(long)(2000.0*log10(outerGain)),DS3D_IMMEDIATE);
			ALSource->update1 &= ~CONEOUTSIDEVOLUME;
			if (ALSource->update1 == 0)
				return;
		}
	}

	
	// Check if we need to update the 3D Mode (Head Relative)
	if (ALSource->update1 & MODE)
	{
		if (ALSource->uservalue2)
		{
			alGetSourcei((ALuint)ALSource, AL_SOURCE_RELATIVE, &Relative);
			IDirectSound3DBuffer_SetMode((LPDIRECTSOUND3DBUFFER)ALSource->uservalue2,Relative ? DS3DMODE_HEADRELATIVE : DS3DMODE_NORMAL,DS3D_IMMEDIATE);
			ALSource->update1 &= ~MODE;
			if (ALSource->update1 == 0)
				return;
		}
	}


	// Check if we ne need to update the Cone Angles for the Source
	if (ALSource->update1 & CONEANGLES)
	{
		if (ALSource->uservalue2)
		{
			alGetSourcei((ALuint)ALSource,AL_CONE_INNER_ANGLE,&innerAngle);
			alGetSourcei((ALuint)ALSource,AL_CONE_OUTER_ANGLE,&outerAngle);
			IDirectSound3DBuffer_SetConeAngles((LPDIRECTSOUND3DBUFFER)ALSource->uservalue2,innerAngle,outerAngle,DS3D_IMMEDIATE);
			ALSource->update1 &= ~CONEANGLES;
			if (ALSource->update1 == 0)
				return;
		}
	}


	// Check if Looping has been enabled / disabled
	if (ALSource->update1 & LOOPED)
	{
		// Only has an effect if the Source is playing
		if ((ALSource->uservalue1) && (ALSource->state == AL_PLAYING))
		{
			// Find out whether Looping has been enabled or disabled
			alGetSourcei((ALuint)ALSource,AL_LOOPING,&Loop);

			if (Loop == AL_TRUE)
			{
				// Looping enabled !

				// All buffers in queue will be needed again, so their state needs to be upgraded
				// to PENDING, and the number of buffers processed set to 0
				ALSource->BuffersProcessed = 0;

				// While calculating the total size of the data (by summing the datasize of each
				// buffer in the queue), set all Buffer states to PENDING
				ALBufferListTemp = ALSource->queue;
				ALSource->DataStillToPlay = 0;

				while (ALBufferListTemp != NULL)
				{
					alGetBufferi(ALBufferListTemp->buffer, AL_SIZE, &DataSize);
					ALSource->DataStillToPlay += DataSize;
					ALBufferListTemp->bufferstate = PENDING;
					ALBufferListTemp = ALBufferListTemp->next;
				}

				// If we have added silence after the valid data, then we need to set the new
				// write position back to the end of the valid data
				if (ALSource->SilenceAdded > 0)
				{
					if (ALSource->OldWriteCursor < ALSource->SilenceAdded)
						ALSource->OldWriteCursor += 88200;

					ALSource->OldWriteCursor -= ALSource->SilenceAdded;

					ALSource->BufferPosition = 0;
				}

				ALSource->FinishedQueue = AL_FALSE;
			}
			else
			{
				// Looping disabled !
				
				// We need to calculate how much data is still to be played
				IDirectSoundBuffer_GetCurrentPosition((LPDIRECTSOUNDBUFFER)ALSource->uservalue1, &PlayCursor, &WriteCursor);
				
				// Calculate amount of data played
				if (ALSource->OldPlayCursor > PlayCursor)
					ALSource->BytesPlayed += ((88200 - ALSource->OldPlayCursor) + PlayCursor);
				else
					ALSource->BytesPlayed += (PlayCursor - ALSource->OldPlayCursor);
				
				ALSource->OldPlayCursor = PlayCursor;

				// Calculate how much data is left to play for the current iteration of the looping data

				TotalDataSize = 0;
				ALBufferListTemp = ALSource->queue;

				while (ALBufferListTemp != NULL)
				{
					alGetBufferi(ALBufferListTemp->buffer, AL_SIZE, &DataSize);
					TotalDataSize += DataSize;
					ALBufferListTemp = ALBufferListTemp->next;
				}

				ALSource->DataStillToPlay = TotalDataSize - (ALSource->BytesPlayed % TotalDataSize);

				if (WriteCursor > PlayCursor)
						DataCommitted = WriteCursor - PlayCursor;
					else
						DataCommitted = (88200 - PlayCursor) + WriteCursor;

				if (DataCommitted > ALSource->DataStillToPlay)
				{
					// Data for the next iteration of the loop has already been committed
					// Therefore increment DataStillToPlay by the total loop size
					ALSource->DataStillToPlay += TotalDataSize;
				}
				else
				{
					DataPlayed = TotalDataSize - ALSource->DataStillToPlay;
					DataCount = 0;
					ALSource->BuffersProcessed = 0;

					ALBufferListItem = ALSource->queue;
					while (ALBufferListItem != NULL)
					{
						alGetBufferi(ALBufferListItem->buffer, AL_SIZE, &DataSize);
						DataCount += DataSize;
						if (DataCount < DataPlayed)
						{
							ALBufferListItem->bufferstate = PROCESSED;
							ALBufferListItem = ALBufferListItem->next;
							ALSource->BuffersProcessed++;
						}
						else
							ALBufferListItem = NULL;
					}
				}

				if (ALSource->DataStillToPlay < 88200)
				{
					// Need to move Write Cursor to end of valid data (so silence can be added
					// after it)
					ALSource->OldWriteCursor = PlayCursor + ALSource->DataStillToPlay;

					if (ALSource->OldWriteCursor >= 88200)
						ALSource->OldWriteCursor -= 88200;

					ALSource->FinishedQueue = AL_TRUE;
				}
			}	
		}
		ALSource->update1 &= ~LOOPED;
		if (ALSource->update1 == 0)
			return;
	}

	return;
}


/*
	Update EAX Buffer Properties
*/
void UpdateSourceEAXProperties(ALsource *ALSource)
{
	ALuint property;

	if (ALSource->update2 & SALLPARAMS)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_ALLPARAMETERS | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP),sizeof(EAXBUFFERPROPERTIES));
			ALSource->update2 &= ~SALLPARAMS;
			if (ALSource->update2 == 0)
				return;
		}
	}

	if (ALSource->update2 & SDIRECT)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_DIRECT | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP.lDirect),sizeof(ALint));
			ALSource->update2 &= ~SDIRECT;
			if (ALSource->update2 == 0)
				return;
		}
	}

	if (ALSource->update2 & SDIRECTHF)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_DIRECTHF | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP.lDirectHF),sizeof(ALint));
			ALSource->update2 &= ~SDIRECTHF;
			if (ALSource->update2 == 0)
				return;
		}
	}
	
	if (ALSource->update2 & SROOM)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_ROOM | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP.lRoom),sizeof(ALint));
			ALSource->update2 &= ~SROOM;
			if (ALSource->update2 == 0)
				return;
		}
	}
	
	if (ALSource->update2 & SROOMHF)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_ROOMHF | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP.lRoomHF),sizeof(ALint));
			ALSource->update2 &= ~SROOMHF;
			if (ALSource->update2 == 0)
				return;
		}
	}
	
	if (ALSource->update2 & SROOMROLLOFFFACTOR)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_ROOMROLLOFFFACTOR | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP.flRoomRolloffFactor),sizeof(ALfloat));
			ALSource->update2 &= ~SROOMROLLOFFFACTOR;
			if (ALSource->update2 == 0)
				return;
		}
	}
	
	if (ALSource->update2 & SOBSTRUCTION)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_OBSTRUCTION | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP.lObstruction),sizeof(ALint));
			ALSource->update2 &= ~SOBSTRUCTION;
			if (ALSource->update2 == 0)
				return;
		}
	}
	
	if (ALSource->update2 & SOBSTRUCTIONLFRATIO)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_OBSTRUCTIONLFRATIO | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP.flObstructionLFRatio),sizeof(ALfloat));
			ALSource->update2 &= ~SOBSTRUCTIONLFRATIO;
			if (ALSource->update2 == 0)
				return;
		}
	}
	
	if (ALSource->update2 & SOCCLUSION)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_OCCLUSION | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP.lOcclusion),sizeof(ALint));
			ALSource->update2 &= ~SOCCLUSION;
			if (ALSource->update2 == 0)
				return;
		}
	}
	
	if (ALSource->update2 & SOCCLUSIONLFRATIO)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP.flOcclusionLFRatio),sizeof(ALfloat));
			ALSource->update2 &= ~SOCCLUSIONLFRATIO;
			if (ALSource->update2 == 0)
				return;
		}
	}
	
	if (ALSource->update2 & SOCCLUSIONROOMRATIO)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP.flOcclusionRoomRatio),sizeof(ALfloat));
			ALSource->update2 &= ~SOCCLUSIONROOMRATIO;
			if (ALSource->update2 == 0)
				return;
		}
	}
	
	if (ALSource->update2 & SOUTSIDEVOLUMEHF)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_OUTSIDEVOLUMEHF | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP.lOutsideVolumeHF),sizeof(ALint));
			ALSource->update2 &= ~SOUTSIDEVOLUMEHF;
			if (ALSource->update2 == 0)
				return;
		}
	}
	
	if (ALSource->update2 & SAIRABSORPTIONFACTOR)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_AIRABSORPTIONFACTOR | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP.flAirAbsorptionFactor),sizeof(ALfloat));
			ALSource->update2 &= ~SAIRABSORPTIONFACTOR;
			if (ALSource->update2 == 0)
				return;
		}
	}

	if (ALSource->update2 & SFLAGS)
	{
		if (ALSource->uservalue3)
		{
			property = DSPROPERTY_EAXBUFFER_FLAGS | ((ALSource->update2 & SDEFERRED) ? DSPROPERTY_EAXBUFFER_DEFERRED : DSPROPERTY_EAXBUFFER_IMMEDIATE);
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				property,NULL,0,&(ALSource->eaxBP.dwFlags),sizeof(ALuint));
			ALSource->update2 &= ~SFLAGS;
			if (ALSource->update2 == 0)
				return;
		}
	}

	if (ALSource->update2 & SCOMMITSETTINGS)
	{
		if (ALSource->uservalue3)
		{
			IKsPropertySet_Set((LPKSPROPERTYSET)ALSource->uservalue3, &DSPROPSETID_EAX_BufferProperties,
				DSPROPERTY_EAXBUFFER_COMMITDEFERREDSETTINGS, NULL, 0, NULL, 0);
			ALSource->update2 &= ~SCOMMITSETTINGS;
			if (ALSource->update2 == 0)
				return;
		}
	}
	return;
}

/*
	Update Open AL Listener Properties
*/
void UpdateListener(ALCcontext *ALContext)
{
	ALfloat		Pos[3],Vel[3], Ori[6];
	ALsource	*ALSource;
	ALfloat		Gain;
	ALuint		i;

	if (ALContext->Listener.update1 & LVOLUME)
	{
		// Setting the volume of the Primary buffer has the effect of setting the volume
		// of the Wave / Direct Sound Mixer Line, so we can't do that

		// Instead we adjust the Gain of every Source

		ALSource = ALContext->Source;

		for (i = 0; i < ALContext->SourceCount; i++)
		{
			if (ALSource->uservalue1)
			{
				// Get current gain for source
				alGetSourcef((ALuint)ALSource,AL_GAIN,&Gain);
				
				Gain = (Gain * ALContext->Listener.Gain);
				
				if (Gain > 0)
					IDirectSoundBuffer_SetVolume((LPDIRECTSOUNDBUFFER)ALSource->uservalue1,(long)(2000.0*log10(Gain)));
				else
					IDirectSoundBuffer_SetVolume((LPDIRECTSOUNDBUFFER)ALSource->uservalue1, -10000);
			}
			ALSource = ALSource->next;
		}

		ALContext->Listener.update1 &= ~LVOLUME;
		if (ALContext->Listener.update1 == 0)
			return;
	}

	if (ALContext->Listener.update1 & LPOSITION)
	{
		Pos[0] = ALContext->Listener.Position[0];
		Pos[1] = ALContext->Listener.Position[1];
		Pos[2] = ALContext->Listener.Position[2];
		IDirectSound3DListener_SetPosition(ALContext->Device->DS3dlistener, Pos[0], Pos[1], -Pos[2],DS3D_IMMEDIATE);
		ALContext->Listener.update1 &= ~LPOSITION;
		if (ALContext->Listener.update1 == 0)
			return;
	}

	if (ALContext->Listener.update1 & LVELOCITY)
	{
		Vel[0] = ALContext->Listener.Velocity[0];
		Vel[1] = ALContext->Listener.Velocity[1];
		Vel[2] = ALContext->Listener.Velocity[2];
		IDirectSound3DListener_SetVelocity(ALContext->Device->DS3dlistener, Vel[0], Vel[1], -Vel[2],DS3D_IMMEDIATE);
		ALContext->Listener.update1 &= ~LVELOCITY;
		if (ALContext->Listener.update1 == 0)
			return;
	}

	if (ALContext->Listener.update1 & LORIENTATION)
	{
		Ori[0] = ALContext->Listener.Forward[0];
		Ori[1] = ALContext->Listener.Forward[1];
		Ori[2] = ALContext->Listener.Forward[2];
		Ori[3] = ALContext->Listener.Up[0];
		Ori[4] = ALContext->Listener.Up[1];
		Ori[5] = ALContext->Listener.Up[2];
		IDirectSound3DListener_SetOrientation(ALContext->Device->DS3dlistener, Ori[0], Ori[1], -Ori[2], Ori[3], Ori[4], -Ori[5], DS3D_IMMEDIATE);
		ALContext->Listener.update1 &= ~LORIENTATION;
		if (ALContext->Listener.update1 == 0)
			return;
	}

	if (ALContext->Listener.update1 & LDOPPLERFACTOR)
	{
		IDirectSound3DListener_SetDopplerFactor(ALContext->Device->DS3dlistener,ALContext->DopplerFactor,DS3D_IMMEDIATE);
		ALContext->Listener.update1 &= ~LDOPPLERFACTOR;
		if (ALContext->Listener.update1 == 0)
			return;
	}

	if (ALContext->Listener.update1 & LROLLOFFFACTOR)
	{
		// Currently we only support a Direct Sound 3D Distance Model, so just set RollOff Factor to 1.0f (default)
		IDirectSound3DListener_SetRolloffFactor (ALContext->Device->DS3dlistener, 1.0f, DS3D_IMMEDIATE);
		ALContext->Listener.update1 &= ~LROLLOFFFACTOR;
		if (ALContext->Listener.update1 == 0)
			return;
	}

	return;
}


/*
	Update EAX Listener Properties
*/
void UpdateListenerEAXProperties(ALCcontext *ALContext)
{
	ALsource		*ALSource;
	LPKSPROPERTYSET lpPropertySet;
	ALuint			i;
	ALuint			property;
	
	// First need to find a Property Set Interface for EAX Listener updates

	// Search through the list of Sources to find one with a valid Property Set Interface (stored
	// in uservalue3)
	
	ALSource = ALContext->Source;
	lpPropertySet = NULL;

	for (i=0;i<ALContext->SourceCount;i++)
	{
		if (alIsSource((ALuint)ALSource))
		{
			if (ALSource->uservalue3)
			{
				lpPropertySet = ALSource->uservalue3;
				break;
			}
		}
		ALSource = ALSource->next;
	}

	if (lpPropertySet == NULL)
	{
		// No Sources have been generated - so we can't set any EAX Listener properties yet ...
		// but this isn't a problem because wouldn't be able to hear any Reverb effects anyway !
		return;
	}

	if (ALContext->Listener.update2 & LALLPARAMS)
	{
		property = DSPROPERTY_EAXLISTENER_ALLPARAMETERS | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP),sizeof(EAXLISTENERPROPERTIES));
		ALContext->Listener.update2 &= ~LALLPARAMS;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LROOM)
	{
		property = DSPROPERTY_EAXLISTENER_ROOM | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.lRoom),sizeof(ALint));
		ALContext->Listener.update2 &= ~LROOM;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LROOMHF)
	{
		property = DSPROPERTY_EAXLISTENER_ROOMHF | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.lRoomHF),sizeof(ALint));
		ALContext->Listener.update2 &= ~LROOMHF;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LROOMROLLOFFFACTOR)
	{
		property = DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.flRoomRolloffFactor),sizeof(ALfloat));
		ALContext->Listener.update2 &= ~LROOMROLLOFFFACTOR;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LDECAYTIME)
	{
		property = DSPROPERTY_EAXLISTENER_DECAYTIME | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.flDecayTime),sizeof(ALfloat));
		ALContext->Listener.update2 &= ~LDECAYTIME;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LDECAYHFRATIO)
	{
		property = DSPROPERTY_EAXLISTENER_DECAYHFRATIO | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.flDecayHFRatio),sizeof(ALfloat));
		ALContext->Listener.update2 &= ~LDECAYHFRATIO;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LREFLECTIONS)
	{
		property = DSPROPERTY_EAXLISTENER_REFLECTIONS | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.lReflections),sizeof(ALint));
		ALContext->Listener.update2 &= ~LREFLECTIONS;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LREFLECTIONSDELAY)
	{
		property = DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.flReflectionsDelay),sizeof(ALfloat));
		ALContext->Listener.update2 &= ~LREFLECTIONSDELAY;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LREVERB)
	{
		property = DSPROPERTY_EAXLISTENER_REVERB | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.lReverb),sizeof(ALint));
		ALContext->Listener.update2 &= ~LREVERB;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LREVERBDELAY)
	{
		property = DSPROPERTY_EAXLISTENER_REVERBDELAY | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.flReverbDelay),sizeof(ALfloat));
		ALContext->Listener.update2 &= ~LREVERBDELAY;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LENVIRONMENT)
	{
		property = DSPROPERTY_EAXLISTENER_ENVIRONMENT | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.dwEnvironment),sizeof(ALuint));
		ALContext->Listener.update2 &= ~LENVIRONMENT;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LENVIRONMENTSIZE)
	{
		property = DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.flEnvironmentSize),sizeof(ALfloat));
		ALContext->Listener.update2 &= ~LENVIRONMENTSIZE;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LENVIRONMENTDIFFUSION)
	{
		property = DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.flEnvironmentDiffusion),sizeof(ALfloat));
		ALContext->Listener.update2 &= ~LENVIRONMENTDIFFUSION;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LAIRABSORPTIONHF)
	{
		property = DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.flAirAbsorptionHF),sizeof(ALfloat));
		ALContext->Listener.update2 &= ~LAIRABSORPTIONHF;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LFLAGS)
	{
		property = DSPROPERTY_EAXLISTENER_FLAGS | ((ALContext->Listener.update2 & LDEFERRED) ? DSPROPERTY_EAXLISTENER_DEFERRED : DSPROPERTY_EAXLISTENER_IMMEDIATE);
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			property,NULL,0,&(ALContext->Listener.eaxLP.dwFlags),sizeof(ALuint));
		ALContext->Listener.update2 &= ~LFLAGS;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	if (ALContext->Listener.update2 & LCOMMITSETTINGS)
	{
		IKsPropertySet_Set(lpPropertySet, &DSPROPSETID_EAX_ListenerProperties,
			DSPROPERTY_EAXLISTENER_COMMITDEFERREDSETTINGS, NULL, 0, NULL, 0);
		ALContext->Listener.update2 &= ~LCOMMITSETTINGS;
		if (ALContext->Listener.update2 == 0)
			return;
	}

	return;
}


ALCAPI ALCdevice* ALCAPIENTRY alcOpenDevice(ALCubyte *deviceName)
{
	DSBUFFERDESC DSBDescription;
	WAVEFORMATEX OutputType;
	ALCdevice *device=NULL;
	ALint vmode=1;
	ALint i;
	DSCAPS dsCaps;
	ALvoid *lpPart1, *lpPart2;
	ALuint dwSize1, dwSize2;

	device=malloc(sizeof(ALCdevice));
	if (device)
	{
		//Initialise device structure
		memset(device,0,sizeof(ALCdevice));
		//Validate device
		device->LastError=AL_NO_ERROR;
		device->InUse=AL_TRUE;
		device->Valid=AL_TRUE;
		//Set output format
		device->Frequency=22050;
		device->Channels=2;
		device->Format=AL_FORMAT_STEREO16;
		//Platform specific
		InitializeCriticalSection(&device->mutex);
		memset(&OutputType,0,sizeof(WAVEFORMATEX));
		OutputType.wFormatTag=WAVE_FORMAT_PCM;
		OutputType.nChannels=device->Channels;
		OutputType.wBitsPerSample=(((device->Format==AL_FORMAT_MONO16)||(device->Format==AL_FORMAT_STEREO16))?16:8);
		OutputType.nBlockAlign=OutputType.nChannels*OutputType.wBitsPerSample/8;
		OutputType.nSamplesPerSec=device->Frequency;
		OutputType.nAvgBytesPerSec=OutputType.nSamplesPerSec*OutputType.nBlockAlign;
		OutputType.cbSize=0;
		//Initialise requested device
		
		if (strcmp(deviceName,"DirectSound3D")==0)
		{
			//Init COM
			CoInitialize(NULL);
			//DirectSound Init code
			if (CoCreateInstance(&CLSID_DirectSound,NULL,CLSCTX_INPROC_SERVER,&IID_IDirectSound,&(device->DShandle))==S_OK)
			{
				if (IDirectSound_Initialize(device->DShandle,NULL)==DS_OK)
				{
					if (IDirectSound_SetCooperativeLevel(device->DShandle,GetForegroundWindow(),DSSCL_PRIORITY)==DS_OK)
					{
						memset(&DSBDescription,0,sizeof(DSBUFFERDESC));
						DSBDescription.dwSize=sizeof(DSBUFFERDESC);
						DSBDescription.dwFlags=DSBCAPS_PRIMARYBUFFER|DSBCAPS_CTRL3D|DSBCAPS_CTRLVOLUME;
						if (IDirectSound_CreateSoundBuffer(device->DShandle,&DSBDescription,&device->DSpbuffer,NULL)==DS_OK)
						{
							if (IDirectSoundBuffer_SetFormat(device->DSpbuffer,&OutputType)==DS_OK)
							{
								if (IDirectSoundBuffer_QueryInterface(device->DSpbuffer,&IID_IDirectSound3DListener,&device->DS3dlistener)==DS_OK)
								{
									// Query number of hardware 3D voices
									memset(&dsCaps, 0, sizeof(DSCAPS));
									dsCaps.dwSize = sizeof(DSCAPS);
									if (IDirectSound_GetCaps(device->DShandle, &dsCaps) == DS_OK)
									{
										device->MaxNoOfSources = dsCaps.dwMaxHw3DStreamingBuffers;
										if (device->MaxNoOfSources > 32)
											device->MaxNoOfSources = 32;
										if (device->MaxNoOfSources < 24)	// Could be Software !
											device->MaxNoOfSources = 24;

										return device;
									}
									IDirectSound3DListener_Release(device->DS3dlistener);
									device->DS3dlistener=NULL;
								}
							}
							IDirectSoundBuffer_Release(device->DSpbuffer);
							device->DSpbuffer=NULL;
						}
					}
					IDirectSound_Release(device->DShandle);
					device->DShandle=NULL;
				}
			}
		}
		else if (strcmp(deviceName,"DirectSound")==0)
		{
			//Init COM
			CoInitialize(NULL);
			//DirectSound Init code
			if (CoCreateInstance(&CLSID_DirectSound,NULL,CLSCTX_INPROC_SERVER,&IID_IDirectSound,&device->DShandle)==S_OK)
			{
				if (IDirectSound_Initialize(device->DShandle,NULL)==DS_OK)
				{
					if (IDirectSound_SetCooperativeLevel(device->DShandle,GetForegroundWindow(),DSSCL_PRIORITY)==DS_OK)
					{
						memset(&DSBDescription,0,sizeof(DSBUFFERDESC));
						DSBDescription.dwSize=sizeof(DSBUFFERDESC);
						DSBDescription.dwFlags=DSBCAPS_PRIMARYBUFFER;
						if (IDirectSound_CreateSoundBuffer(device->DShandle,&DSBDescription,&device->DSpbuffer,NULL)==DS_OK)
						{
							if (IDirectSoundBuffer_SetFormat(device->DSpbuffer,&OutputType)==DS_OK)
							{
								memset(&DSBDescription,0,sizeof(DSBUFFERDESC));
								DSBDescription.dwSize=sizeof(DSBUFFERDESC);
								DSBDescription.dwFlags=DSBCAPS_GLOBALFOCUS|DSBCAPS_GETCURRENTPOSITION2;
								DSBDescription.dwBufferBytes=32768;
								DSBDescription.lpwfxFormat=&OutputType;
								if (IDirectSound_CreateSoundBuffer(device->DShandle,&DSBDescription,&device->DSsbuffer,NULL)==DS_OK)
								{
									if (IDirectSoundBuffer_Lock(device->DSsbuffer,0,0,&lpPart1, &dwSize1, &lpPart2, &dwSize2, DSBLOCK_ENTIREBUFFER ) == DS_OK)
									{
										memset(lpPart1, 0, dwSize1);
										IDirectSoundBuffer_Unlock(device->DSsbuffer,lpPart1, dwSize1, lpPart1, dwSize2);
									}
									if (IDirectSoundBuffer_Play(device->DSsbuffer,0,0,DSBPLAY_LOOPING)==DS_OK)
									{
										device->timer=timeSetEvent(25,0,alcDirectSoundProc,(DWORD)device,TIME_CALLBACK_FUNCTION|TIME_PERIODIC);
										device->MaxNoOfSources = 32;
										return device;
									}
									IDirectSoundBuffer_Release(device->DSsbuffer);
									device->DSsbuffer=NULL;
								}
							}
							IDirectSoundBuffer_Release(device->DSpbuffer);
							device->DSpbuffer=NULL;
						}
					}
					IDirectSound_Release(device->DShandle);
					device->DShandle=NULL;
				}
			}
		}
		else
		{
			//Default to WaveOut code
			if (waveOutOpen(&device->handle,WAVE_MAPPER,&OutputType,0,0,WAVE_FORMAT_DIRECT_QUERY)==MMSYSERR_NOERROR)
			{
				if (waveOutOpen(&device->handle,WAVE_MAPPER,&OutputType,(DWORD)&alcWaveOutProc,(DWORD)0,CALLBACK_FUNCTION)==MMSYSERR_NOERROR)
				{
					device->MaxNoOfSources = 32;
					// Setup Windows Multimedia driver buffers and start playing
					for (i=0;i<3;i++)
					{
						memset(&device->buffer[i],0,sizeof(WAVEHDR));
						device->buffer[i].lpData=malloc(((OutputType.nAvgBytesPerSec/16)&0xfffffff0));
						device->buffer[i].dwBufferLength=((OutputType.nAvgBytesPerSec/16)&0xfffffff0);
						device->buffer[i].dwFlags=0;
						device->buffer[i].dwLoops=0;
						waveOutPrepareHeader(device->handle,&device->buffer[i],sizeof(WAVEHDR));
						if (waveOutWrite(device->handle,&device->buffer[i],sizeof(WAVEHDR))!=MMSYSERR_NOERROR)
						{
							waveOutUnprepareHeader(device->handle,&device->buffer[i],sizeof(WAVEHDR));
							free(device->buffer[i].lpData);
						}
					}
				}
			}
		}
	}
	return device;
}

ALCAPI ALCvoid ALCAPIENTRY alcCloseDevice(ALCdevice *device)
{
	ALint i;

	if (device)
	{
		EnterCriticalSection(&device->mutex);
		//Release timer
		if (device->timer)
			timeKillEvent(device->timer);
		//Platform specific exit
		if (device->DShandle)
		{
			if (device->DS3dlistener)
				IDirectSound3DListener_Release(device->DS3dlistener);
			if (device->DSsbuffer)
				IDirectSoundBuffer_Release(device->DSsbuffer);
			if (device->DSpbuffer)
				IDirectSoundBuffer_Release(device->DSpbuffer);
			if (device->DShandle)
				IDirectSound_Release(device->DShandle);
			//Deinit COM
			CoUninitialize();		
		}
		else
		{
			waveOutReset(device->handle);
			for (i=0;i<3;i++)
			{
				waveOutUnprepareHeader(device->handle,&device->buffer[i],sizeof(WAVEHDR));
				free(device->buffer[i].lpData);
			}
			waveOutClose(device->handle);
		}
		//Release device structure
		DeleteCriticalSection(&device->mutex);
		memset(device,0,sizeof(ALCdevice));
		free(device);
	}
	else alcSetError(ALC_INVALID_DEVICE);
}

