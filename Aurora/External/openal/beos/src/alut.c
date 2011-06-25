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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>


enum {
	WAVE_FORMAT_UNKNOWN 	= 0x0000,
	WAVE_FORMAT_PCM			= 0x0001,
	WAVE_FORMAT_ADPCM		= 0x0002,
	WAVE_FORMAT_ALAW		= 0x0006,
	WAVE_FORMAT_MULAW		= 0x0007,
	WAVE_FORMAT_IMA_ADPCM	= 0x0011,
	WAVE_FORMAT_MPEG		= 0x0050
};

typedef struct {
	ALushort	wFormatTag;
	ALushort	nChannels;
	ALuint		nSamplesPerSec;
	ALuint		nAvgBytesPerSec;
	ALushort	nBlockAlign;
	ALushort	wBitsPerSample;
} WAVEFORMAT;

typedef struct {
	ALuint		fccID;
	ALint		dwSize;
	ALuint		fccType;
} RIFFHEADER;

typedef struct {
	ALuint		fccTag;
	ALint		dwSize;
} CHUNKHEADER;


static ALvoid *alutContext = NULL;

static ALvoid alutADPCMDecoder(const char *in, short *out, int len);


ALUTAPI ALvoid ALUTAPIENTRY alutInit(int *argc, char **argv)
{
	alInit((ALint *) argc, (ALubyte **) argv);
	alutContext = alcCreateContext(22050, AL_FORMAT_STEREO16, 2048);
	alcMakeCurrent(alutContext);
}

ALUTAPI ALvoid ALUTAPIENTRY alutExit(void)
{
	alcMakeCurrent(NULL);
	alcDeleteContext(alutContext);
	alExit();
}

ALUTAPI ALvoid ALUTAPIENTRY alutLoadWAV(const ALubyte *name, ALenum *format, ALvoid **data, ALsizei *size, ALsizei *freq)
{
	FILE *file;
	RIFFHEADER header;
	CHUNKHEADER chunk;
	WAVEFORMAT fmt;

	*format = AL_FORMAT_MONO16;
	*data = NULL;
	*size = 0;
	*freq = 22050;
	
	if ((file = fopen(name, "rb")) != NULL) {
	
		/* FIXME: only works in little endian machines */

		if (fread(&header, 1, sizeof(header), file) == sizeof(header) &&
			memcmp(&header.fccID, "RIFF", 4) == 0 && memcmp(&header.fccType, "WAVE", 4) == 0) {

			header.dwSize += (header.dwSize & 1) - sizeof(header.fccType);

			while (header.dwSize > 0) {
				if (fread(&chunk, 1, sizeof(chunk), file) != sizeof(chunk))
					break;
					
				chunk.dwSize += chunk.dwSize & 1;

				if (memcmp(&chunk.fccTag, "fmt ", 4) == 0) {
					if (fread(&fmt, 1, sizeof(fmt), file) != sizeof(fmt))
						break;

					if (fmt.nChannels == 1) {
						if (fmt.wBitsPerSample == 8)
							*format = AL_FORMAT_MONO8;
						else
							*format = AL_FORMAT_MONO16;
					}
					else {
						if (fmt.wBitsPerSample == 8)
							*format = AL_FORMAT_STEREO8;
						else
							*format = AL_FORMAT_STEREO16;
					}
				
					*freq = fmt.nSamplesPerSec;

					fseek(file, chunk.dwSize - sizeof(fmt), SEEK_CUR);
				}
				else if (memcmp(&chunk.fccTag, "data", 4) == 0) {
					if (fmt.wFormatTag == WAVE_FORMAT_PCM) {
						*size = chunk.dwSize;
						*data = malloc(chunk.dwSize);
						if (fread(*data, 1, chunk.dwSize, file) != chunk.dwSize)
							break;
					}
					else if (fmt.wFormatTag == WAVE_FORMAT_IMA_ADPCM) {
						*size = 4*chunk.dwSize;
						*data = malloc(4*chunk.dwSize);
						if (fread((char *) *data + 3*chunk.dwSize, 1, chunk.dwSize, file) != chunk.dwSize)
							break;
						alutADPCMDecoder((char *) *data + 3*chunk.dwSize, (short *) *data, chunk.dwSize);
					}
					else {
						fseek(file, chunk.dwSize, SEEK_CUR);
					}
				}
				else {
					fseek(file, chunk.dwSize, SEEK_CUR);
				}
				
				header.dwSize -= chunk.dwSize + sizeof(chunk);
			}
		}
	}
	fclose(file);
}


static ALvoid alutADPCMDecoder(const char *in, short *out, int len)
{
	/* Intel ADPCM step variation table */
	static const int kIndexTable[16] = {
	    -1, -1, -1, -1, 2, 4, 6, 8,
	    -1, -1, -1, -1, 2, 4, 6, 8,
	};
	
	static const int kStepSizeTable[89] = {
	    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
	};

	int predictor, index, nibble;

	predictor = 0;
	index = 0;

	while (len > 0) {
		int value = *in++;

		for (nibble = 0; nibble < 2; nibble++) {
			int delta, step, diff;
			
			delta = value & 0x0f;
			value >>= 4;
			
			step = kStepSizeTable[index];
	
			index += kIndexTable[delta];
			if (index < 0)
				index = 0;
			else if (index > 88)
				index = 88;
			
			diff = step >> 3;
			if ((delta & 4) != 0)
				diff += step;
			if ((delta & 2) != 0)
				diff += step >> 1;
			if ((delta & 1) != 0)
				diff += step >> 2;
			
			if ((delta & 8) != 0)
				predictor -= diff;
			else
				predictor += diff;
			
			if (predictor > 32767)
				predictor = 32767;
			else if (predictor < -32768)
				predictor = -32768;
			
			*out++ = predictor;
		}

		len--;
	}
}
