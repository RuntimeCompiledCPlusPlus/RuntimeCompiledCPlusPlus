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
#include "globals.h"
#include "alSource.h"
#include "alSoftware.h"
#include "sm.h"
 
 // Sound Manager functions
void smPlaySegment(unsigned int source)
{
	OSErr Err;
	ExtSoundHeader SoundHdr;
	SndCommand BufferCommand;
	int iBufferNum;
	ALfloat Pitch, Panning, Volume;
	QueueEntry *pQE;

	iBufferNum = gSource[source].srcBufferNum;
		
	if ((gSource[source].channelPtr != NULL) && (gBuffer[iBufferNum].size > 0)) // have a sound channel and data, so play...
	{	
		
		gSource[source].state = AL_PLAYING;
		
		// reset volume as needed for 3D environment
		if (gBuffer[iBufferNum].channels == 1) // make sure this is a mono buffer before positioning it
		{
			alCalculateSourceParameters(source,&Pitch,&Panning,&Volume);
			if (Volume > 1.0f) Volume = 1.0f;
			smSetSourceVolume(source, (kFullVolume * Volume * ((Panning < 0.5) ? Panning * 2.0 : (1))),
									(kFullVolume * Volume * ((Panning < 0.5) ? (1): (2.0 - Panning * 2.0))));
		} else
		{
			Volume = (gSource[source].gain*gListener.Gain);
			if (Volume > 1.0f) Volume = 1.0f;
			smSetSourceVolume(source, (kFullVolume * Volume), (kFullVolume * Volume));
		}
		// create sound header
		if (gSource[source].readOffset > gBuffer[iBufferNum].size) { gSource[source].readOffset = 0; }
		SoundHdr.samplePtr = (char *) gBuffer[iBufferNum].data + gSource[source].readOffset;
		if ((gSource[source].readOffset + AL_INTERNAL_BUFFERS_SIZE) <= gBuffer[iBufferNum].size)
		{
			SoundHdr.numFrames = AL_INTERNAL_BUFFERS_SIZE;
		} else
		{
			SoundHdr.numFrames = (gBuffer[iBufferNum].size - gSource[source].readOffset);
		}
		if (gBuffer[iBufferNum].bits == 16) { SoundHdr.numFrames = SoundHdr.numFrames / 2; }
		SoundHdr.numFrames = SoundHdr.numFrames / gBuffer[iBufferNum].channels;
		SoundHdr.sampleSize = gBuffer[iBufferNum].bits;
		SoundHdr.numChannels = gBuffer[iBufferNum].channels;
		switch (gBuffer[iBufferNum].frequency)
		{
			case 11025:	SoundHdr.sampleRate = rate11khz;
					break;
			case 22050: SoundHdr.sampleRate = rate22khz;
					break;
			case 44100: SoundHdr.sampleRate = rate44khz;
				    break;
			default: SoundHdr.sampleRate = rate44khz;
		}
		SoundHdr.loopStart = 0;
		SoundHdr.loopEnd = 0;
		SoundHdr.encode = extSH;
		SoundHdr.baseFrequency = 60;
		//SoundHdr.AIFFSampleRate = 
		SoundHdr.markerChunk = NULL;
		SoundHdr.instrumentChunks = NULL;
		SoundHdr.AESRecording = NULL;
				
		// create buffer command
		BufferCommand.cmd = bufferCmd;
		BufferCommand.param1 = NULL;
		BufferCommand.param2 = (long) &SoundHdr;
				
		// send buffer command
		Err = SndDoImmediate (gSource[source].channelPtr, &BufferCommand);
			
		// reset pitch if necessary
		if (gSource[source].pitch != 1.0f)
		{
			smSetSourcePitch(source, gSource[source].pitch);
		}
				
		// create callback command
		BufferCommand.cmd = callBackCmd;
		BufferCommand.param1 = NULL;
		BufferCommand.param2 = source;
				
		// send callback command
		Err = SndDoCommand (gSource[source].channelPtr, &BufferCommand, true);
	} else // evaluate if should start processing queue...
	{
	    // find first un-processed queue (or don't find any to process)
		pQE = gSource[source].ptrQueue;
		if (pQE != NULL)
		{
			while (pQE->processed == AL_TRUE)
			{
				pQE = pQE->pNext;
				if (pQE == NULL) break;
			}
		}
		
		// if there's a queue to process, do it...
		if (pQE != NULL)
		{
			pQE->processed = AL_TRUE;
			gSource[source].srcBufferNum = pQE->bufferNum;
			smPlaySegment(source);
		}
	}
}

pascal void smService (SndChannelPtr chan, SndCommand* acmd)
{
	ALuint source;
	QueueEntry *pQE;
	
	source = (ALuint) acmd->param2;
	
	// evaluate whether or not the buffer has been over-run
	gSource[source].readOffset += AL_INTERNAL_BUFFERS_SIZE;
	if (gSource[source].readOffset >= gBuffer[gSource[source].srcBufferNum].size)
	{
	    // check if there is a new queue to go to -- if not, then reset queue processed flags, decrement loop counter, and restart
		pQE = gSource[source].ptrQueue;
		if (pQE != NULL)
		{
			while (pQE->processed == AL_TRUE)
			{
				pQE = pQE->pNext;
				if (pQE == NULL) break;
			}
		}
	    
	    if (pQE != NULL) // process next queued buffer
	    {
	    	pQE->processed = AL_TRUE;
	    	gSource[source].srcBufferNum = pQE->bufferNum;
	    	gSource[source].readOffset = 0;
	    } else // completed all buffers, so reset buffer processed flags and decrement loop counter
	    {
	    	pQE = gSource[source].ptrQueue; // reset all processed flags
	    	while (pQE != NULL)
	    	{
	    		pQE->processed = AL_FALSE;
	    		pQE = pQE->pNext;
	    	}
	    	
	    	pQE = gSource[source].ptrQueue; // if there is a queue, stage first buffer
	    	if (pQE != NULL)
	    	{
	    		gSource[source].srcBufferNum = pQE->bufferNum;
	    		gSource[source].readOffset = 0;
	    		pQE->processed = AL_TRUE;
	    	}
	    	
			if (gSource[source].looping != 1) 
			{ 
				gSource[source].state = AL_STOPPED; 
			}
		}
	}
	
	// if now stopped, reset read pointer
	if (gSource[source].state == AL_STOPPED)
	{
		gSource[source].readOffset = 0;
	}
	
	// evaluate if more data needs to be played -- if so then call smPlaySegment
	if (
		(gSource[source].state == AL_PLAYING) &&
	    (((gSource[source].readOffset == 0) && (gSource[source].looping == 1)) || (gSource[source].readOffset != 0))
	   )
	{
		smPlaySegment(source);
	}
}

void smSetSourceVolume (int source, int rVol, int lVol)
{
	SndCommand VolCommand;
	OSErr Err;
	
	// create volume command
	VolCommand.cmd = volumeCmd;
	VolCommand.param1 = NULL;
	VolCommand.param2 = (rVol << 16) + lVol;
				
	// send volume command
	Err = SndDoImmediate (gSource[source].channelPtr, &VolCommand);
}

void smSetSourcePitch(int source, float value)
{
	SndCommand SoundCommand;
	OSErr Err;
	long Rate;
	
	SoundCommand.cmd = getRateCmd;
	SoundCommand.param1 = NULL;
	SoundCommand.param2 = (long) &Rate;
				
	Err = SndDoImmediate (gSource[source].channelPtr, &SoundCommand);
	
	if (!(Err))
	{
		SoundCommand.cmd = rateCmd;
		SoundCommand.param1 = NULL;
		SoundCommand.param2 = Rate * value;
				
		Err = SndDoImmediate (gSource[source].channelPtr, &SoundCommand);
	}
	
}

void smSourceInit(unsigned int source)
{
 	OSErr Err;
 	
    if (gSource[source].channelPtr != NULL)
    {
    	smSourceKill(source);
    } else
    {
		Err = SndNewChannel(&gSource[source].channelPtr, sampledSynth, initMono, gpSMRtn); // create sound channel
		gSource[source].readOffset = 0; 
	}
}

void smSourceKill(unsigned int source)
{
	OSErr Err;
	
	if (gSource[source].channelPtr != NULL)
	{
		SndDisposeChannel(gSource[source].channelPtr, true);
		gSource[source].channelPtr = NULL;
		gSource[source].srcBufferNum = AL_MAXBUFFERS + 1;
	}
}

