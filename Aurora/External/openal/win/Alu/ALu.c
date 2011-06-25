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
#include <OpenAL32\include\alSource.h>
#include <OpenAL32\include\alBuffer.h>
#include <OpenAL32\include\alError.h>
#include <OpenAL32\include\alState.h>

ALUAPI ALint ALUAPIENTRY aluF2L(ALfloat Value)
{
	double temp;

	temp=Value+(((65536.0*65536.0*16.0)+(65536.0*65536.0*8.0))*65536.0);
	return *((long *)&temp);
}

ALUAPI ALshort ALUAPIENTRY aluF2S(ALfloat Value)
{
	double temp;
	long i;

	temp=Value+(((65536.0*65536.0*16.0)+(65536.0*65536.0*8.0))*65536.0);
	i=(*((long *)&temp));
	if (i>32767)
		i=32767;
	else if (i<-32768)
		i=-32768;
	return ((short)i);
}

ALUAPI ALvoid ALUAPIENTRY aluCrossproduct(ALfloat *inVector1,ALfloat *inVector2,ALfloat *outVector)
{
	outVector[0]=(inVector1[1]*inVector2[2]-inVector1[2]*inVector2[1]);
	outVector[1]=(inVector1[2]*inVector2[0]-inVector1[0]*inVector2[2]);
	outVector[2]=(inVector1[0]*inVector2[1]-inVector1[1]*inVector2[0]);
}

ALUAPI ALfloat ALUAPIENTRY aluDotproduct(ALfloat *inVector1,ALfloat *inVector2)
{
	return (inVector1[0]*inVector2[0]+inVector1[1]*inVector2[1]+inVector1[2]*inVector2[2]);
}

ALUAPI ALvoid ALUAPIENTRY aluNormalize(ALfloat *inVector)
{
	ALfloat length,inverse_length;

	length=(ALfloat)sqrt(aluDotproduct(inVector,inVector));
	if (length != 0)
	{
		inverse_length=(1.0f/length);
		inVector[0]*=inverse_length;
		inVector[1]*=inverse_length;
		inVector[2]*=inverse_length;
	}
}

ALUAPI ALvoid ALUAPIENTRY aluMatrixVector(ALfloat matrix[3][3],ALfloat *vector)
{
	ALfloat result[3];

	result[0]=matrix[0][0]*vector[0]+matrix[0][1]*vector[1]+matrix[0][2]*vector[2];
	result[1]=matrix[1][0]*vector[0]+matrix[1][1]*vector[1]+matrix[1][2]*vector[2];
	result[2]=matrix[2][0]*vector[0]+matrix[2][1]*vector[1]+matrix[2][2]*vector[2];
	memcpy(vector,result,sizeof(result));
}

ALUAPI ALvoid ALUAPIENTRY aluCalculateSourceParameters(ALuint source,ALuint channels,ALfloat *drysend,ALfloat *wetsend,ALfloat *pitch)
{
	ALfloat ListenerOrientation[6],ListenerPosition[3],ListenerVelocity[3];
	ALfloat Direction[3],Position[3],Velocity[3],SourceToListener[3];
	ALfloat InnerAngle,OuterAngle,OuterGain,Angle,Distance;
	ALfloat Pitch,Volume,PanningFB,PanningLR,ListenerGain;
	ALfloat MinVolume,MaxVolume,MinDist,MaxDist,Rolloff;
	ALfloat U[3],V[3],N[3];
	ALfloat DopplerFactor;
	ALuint DistanceModel;
	ALfloat Matrix[3][3];
	ALint HeadRelative;
	ALenum Error;

	if (alIsSource(source))
	{
		//Get global properties
		alGetFloatv(AL_DOPPLER_FACTOR,&DopplerFactor);
		alGetIntegerv(AL_DISTANCE_MODEL,&DistanceModel);
		//Get source properties
		alGetSourcef(source,AL_PITCH,&Pitch);
		alGetSourcef(source,AL_GAIN,&Volume);
		alGetSourcef(source,AL_MIN_GAIN,&MinVolume);
		alGetSourcef(source,AL_MAX_GAIN,&MaxVolume);
		alGetSourcef(source,AL_REFERENCE_DISTANCE,&MinDist);
		alGetSourcef(source,AL_MAX_DISTANCE,&MaxDist);
		alGetSourcef(source,AL_ROLLOFF_FACTOR,&Rolloff);
		alGetSourcefv(source,AL_POSITION,Position);
		alGetSourcefv(source,AL_VELOCITY,Velocity);
		alGetSourcefv(source,AL_DIRECTION,Direction);
		alGetSourcef(source,AL_CONE_OUTER_GAIN,&OuterGain);
		alGetSourcef(source,AL_CONE_INNER_ANGLE,&InnerAngle);
		alGetSourcef(source,AL_CONE_OUTER_ANGLE,&OuterAngle);
		alGetSourcei(source,AL_SOURCE_RELATIVE,&HeadRelative);
		//Get listener properties
		alGetListenerf(AL_GAIN,&ListenerGain);
		alGetListenerfv(AL_POSITION,ListenerPosition);
		alGetListenerfv(AL_VELOCITY,ListenerVelocity);
		alGetListenerfv(AL_ORIENTATION,ListenerOrientation);
		//1. Translate Listener to origin (convert to head relative)
		if (HeadRelative==AL_FALSE)
		{
			Position[0]-=ListenerPosition[0];
			Position[1]-=ListenerPosition[1];
			Position[2]-=ListenerPosition[2];
		}
		//2. Align coordinate system axes
		ListenerOrientation[2]=-ListenerOrientation[2]; // convert to right handed
		aluCrossproduct(&ListenerOrientation[3],&ListenerOrientation[0],U);
		aluNormalize(U);
		aluCrossproduct(&ListenerOrientation[0],U,V);
		aluNormalize(V);
		memcpy(N,&ListenerOrientation[0],sizeof(N));
		aluNormalize(N);
		Matrix[0][0]=U[0]; Matrix[0][1]=V[0]; Matrix[0][2]=N[0];
		Matrix[1][0]=U[1]; Matrix[1][1]=V[1]; Matrix[1][2]=N[1];
		Matrix[2][0]=U[2]; Matrix[2][1]=V[2]; Matrix[2][2]=N[2];
		aluMatrixVector(Matrix,Position);
		//3. Calculate distance attenuation
		Distance=(ALfloat)sqrt(aluDotproduct(Position,Position));
		if (DistanceModel!=AL_NONE)
		{
			if (DistanceModel==AL_INVERSE_DISTANCE_CLAMPED)
			{
				Distance=(Distance<MinDist?MinDist:Distance);
				Distance=(Distance>MaxDist?MaxDist:Distance);
			}
			Volume=(Volume*ListenerGain*MinDist)/(MinDist+Rolloff*(Distance-MinDist));
		}
		else
			Volume=(Volume*ListenerGain        );
		//4. Apply directional soundcones
		SourceToListener[0]=-Position[0];	
		SourceToListener[1]=-Position[1];
		SourceToListener[2]=-Position[2];	
		aluNormalize(Direction);
		aluNormalize(SourceToListener);
		Angle=(ALfloat)(180.0*acos(aluDotproduct(Direction,SourceToListener))/3.141592654f);
		if ((Angle>=InnerAngle)&&(Angle<=OuterAngle))
			Volume=(Volume*(1.0f+(OuterGain-1.0f)*(Angle-InnerAngle)/(OuterAngle-InnerAngle)));
		else if (Angle>OuterAngle)
			Volume=(Volume*(1.0f+(OuterGain-1.0f)                                           ));
		//5. Calculate differential velocity
		Velocity[0]-=ListenerVelocity[0];
		Velocity[1]-=ListenerVelocity[1];
		Velocity[2]-=ListenerVelocity[2];
		aluMatrixVector(Matrix,Velocity);		
		//6. Calculate doppler
		if ((DopplerFactor!=0.0f)&&(Distance!=0.0f))
			pitch[0]=(ALfloat)((Pitch*DopplerFactor)/(1.0+(aluDotproduct(Velocity,Position)/(343.0f*Distance))));
		else
			pitch[0]=(ALfloat)((Pitch			   )/(1.0+(aluDotproduct(Velocity,Position)/(343.0f         ))));
		//7. Convert into channel volumes
		if (Distance != 0)
		{
			aluNormalize(Position);
			if (Position[0]<0)
				PanningLR=(ALfloat)(0.5-0.4*sqrt(-Position[0]));
			else
				PanningLR=(ALfloat)(0.5+0.4*sqrt( Position[0]));
			if (Position[2]<0)
				PanningFB=(ALfloat)(0.5-0.4*sqrt(-Position[2]));
			else
				PanningFB=(ALfloat)(0.5+0.4*sqrt( Position[2]));
		}
		else
		{
			PanningLR=0.5;	
			PanningFB=0.5;
		}

		switch (channels)
		{
			case 1:
				drysend[0]=(Volume*1.0f  );
				wetsend[0]=(Volume*0.0f  );
				break;
			case 2:
			default:
				drysend[0]=(Volume*1.0f  *(ALfloat)sqrt(1.0f-PanningLR));
				drysend[1]=(Volume*1.0f  *(ALfloat)sqrt(     PanningLR));
				wetsend[0]=(Volume*0.0f  *(ALfloat)sqrt(1.0f-PanningLR));
				wetsend[1]=(Volume*0.0f  *(ALfloat)sqrt(     PanningLR));
			 	break;
			case 3:
				drysend[0]=(Volume*1.0f  *(ALfloat)sqrt(1.0f-PanningLR));
				drysend[1]=(Volume*1.0f  *(ALfloat)sqrt(     PanningLR));
				drysend[2]=(Volume*1.0f  *(ALfloat)sqrt(     PanningFB));
				wetsend[0]=(Volume*0.0f  *(ALfloat)sqrt(1.0f-PanningLR));
				wetsend[1]=(Volume*0.0f  *(ALfloat)sqrt(     PanningLR));
				wetsend[2]=(Volume*0.0f  *(ALfloat)sqrt(     PanningFB));
				break;
			case 4:
				drysend[0]=(Volume*1.0f  *(ALfloat)sqrt((1.0f-PanningLR)*(1.0f-PanningFB)));
				drysend[1]=(Volume*1.0f  *(ALfloat)sqrt((     PanningLR)*(1.0f-PanningFB)));
				drysend[2]=(Volume*1.0f  *(ALfloat)sqrt((1.0f-PanningLR)*(     PanningFB)));
				drysend[3]=(Volume*1.0f  *(ALfloat)sqrt((     PanningLR)*(     PanningFB)));
				wetsend[0]=(Volume*0.0f  *(ALfloat)sqrt((1.0f-PanningLR)*(1.0f-PanningFB)));
				wetsend[1]=(Volume*0.0f  *(ALfloat)sqrt((     PanningLR)*(1.0f-PanningFB)));
				wetsend[2]=(Volume*0.0f  *(ALfloat)sqrt((1.0f-PanningLR)*(     PanningFB)));
				wetsend[3]=(Volume*0.0f  *(ALfloat)sqrt((     PanningLR)*(     PanningFB)));
				break;
		}	
		Error=alGetError();
	}
}

ALUAPI ALvoid ALUAPIENTRY aluMixData(ALvoid *context,ALvoid *buffer,ALsizei size,ALenum format)
{
	ALfloat Pitch,DrySend[OUTPUTCHANNELS],WetSend[OUTPUTCHANNELS];
	static float DryBuffer[BUFFERSIZE][OUTPUTCHANNELS];
	static float WetBuffer[BUFFERSIZE][OUTPUTCHANNELS];
	ALuint BlockAlign,BytesPerSample,BufferSize;
	ALuint DataSize,DataPosInt,DataPosFrac;
	ALint Looping,increment,State;
	ALuint Channels,Bits,Frequency;
	ALuint Buffer,fraction;
	ALCcontext *ALContext;
	ALsizei SamplesToDo;
	ALshort value,*Data;
	ALsource *ALSource;
	ALsizei i,j,k;
	ALenum Error;
	ALbufferlistitem *BufferListItem;
	ALuint loop;

	if (1)
	{
		ALContext=((ALCcontext *)context);
		alcSuspendContext(ALContext);
		if ((buffer)&&(size))
		{
			//Figure output format variables
			switch (format)
			{
				case AL_FORMAT_MONO8:
					BlockAlign=1;
					BytesPerSample=1;
					break;
				case AL_FORMAT_STEREO8:
					BlockAlign=2;
					BytesPerSample=1;
					break;
				case AL_FORMAT_MONO16:
					BlockAlign=2;
					BytesPerSample=2;
					break;
				case AL_FORMAT_STEREO16:
				default:
					BlockAlign=4;
					BytesPerSample=2;
					break;
			}
			//Setup variables
			ALSource=ALContext->Source;
			SamplesToDo=((size/BlockAlign)<BUFFERSIZE?(size/BlockAlign):BUFFERSIZE);
			//Clear mixing buffer
			memset(DryBuffer,0,SamplesToDo*OUTPUTCHANNELS*sizeof(ALfloat));
			memset(WetBuffer,0,SamplesToDo*OUTPUTCHANNELS*sizeof(ALfloat));
			//Actual mixing loop
			for (i=0;i<ALContext->SourceCount;i++)
			{
				j=0;
				alGetSourcei((ALuint)ALSource,AL_SOURCE_STATE,&State);
				while ((State==AL_PLAYING)&&(j<SamplesToDo))
				{
					aluCalculateSourceParameters((ALuint)ALSource,OUTPUTCHANNELS,DrySend,WetSend,&Pitch);
					//Get buffer info
					alGetSourcei((ALuint)ALSource,AL_BUFFER,&Buffer);
					if (Buffer)
					{
						alGetBufferi(Buffer,AL_DATA,(ALint *)&Data);
						alGetBufferi(Buffer,AL_BITS,&Bits);
						alGetBufferi(Buffer,AL_SIZE,&DataSize);
						alGetBufferi(Buffer,AL_CHANNELS,&Channels);
						alGetBufferi(Buffer,AL_FREQUENCY,&Frequency);
						Pitch=((Pitch*Frequency)/ALContext->Frequency);
						DataSize=(DataSize/(Bits*Channels/8));
						//Get source info
						DataPosInt=ALSource->position;
						DataPosFrac=ALSource->position_fraction;
						//Figure out how many samples we can mix.
						BufferSize=(ALuint)(((ALfloat)(DataSize-DataPosInt))/Pitch)+1;
						BufferSize=(BufferSize<(SamplesToDo-j)?BufferSize:(SamplesToDo-j));
						//Actual sample mixing loop
						Data+=DataPosInt*Channels;
						increment=aluF2L(Pitch*(1L<<FRACTIONBITS));
						while (BufferSize--)
						{
							k=DataPosFrac>>FRACTIONBITS; fraction=DataPosFrac&FRACTIONMASK;
							switch (Channels)
							{
								case 0x01:
									value=((Data[k]*((1L<<FRACTIONBITS)-fraction))+(Data[k+1]*(fraction)))>>FRACTIONBITS;
									DryBuffer[j][0]+=((ALfloat)value)*DrySend[0];
									DryBuffer[j][1]+=((ALfloat)value)*DrySend[1];
									WetBuffer[j][0]+=((ALfloat)value)*WetSend[0];
									WetBuffer[j][1]+=((ALfloat)value)*WetSend[1];
									break;
								case 0x02:
									value=((Data[k*2  ]*((1L<<FRACTIONBITS)-fraction))+(Data[k*2+2]*(fraction)))>>FRACTIONBITS;
									DryBuffer[j][0]+=((ALfloat)value)*DrySend[0];
									WetBuffer[j][0]+=((ALfloat)value)*WetSend[0];
									value=((Data[k*2+1]*((1L<<FRACTIONBITS)-fraction))+(Data[k*2+3]*(fraction)))>>FRACTIONBITS;
									DryBuffer[j][1]+=((ALfloat)value)*DrySend[1];
									WetBuffer[j][1]+=((ALfloat)value)*WetSend[1];
									break;
								default:
									break;
							}
							DataPosFrac+=increment;
							j++;
						}
						DataPosInt+=(DataPosFrac>>FRACTIONBITS);
						DataPosFrac=(DataPosFrac&FRACTIONMASK);
						//Update source info
						ALSource->position=DataPosInt;
						ALSource->position_fraction=DataPosFrac;
					}
					//Handle looping sources
					if ((!Buffer)||(DataPosInt>=DataSize))
					{
						//queueing
						if (ALSource->queue)
						{
							alGetSourcei((ALuint)ALSource,AL_LOOPING,&Looping);
							if (ALSource->BuffersAddedToDSBuffer < (ALSource->BuffersInQueue-1))
							{
								BufferListItem = ALSource->queue;
								for (loop = 0; loop <= ALSource->BuffersAddedToDSBuffer; loop++)
								{
									if (BufferListItem)
									{
										if (!Looping) BufferListItem->bufferstate=PROCESSED;
										BufferListItem = BufferListItem->next;
									}
								}
								if (!Looping) ALSource->BuffersProcessed++;
								if (BufferListItem)
									ALSource->param[AL_BUFFER-AL_CONE_INNER_ANGLE].data.i=BufferListItem->buffer;
								ALSource->position=0;
								ALSource->position_fraction=0;
								ALSource->BuffersAddedToDSBuffer++;
							}
							else
							{
								alSourceStop((ALuint)ALSource);
								alGetSourcei((ALuint)ALSource,AL_LOOPING,&Looping);
								if (Looping)
								{
									alSourceRewind((ALuint)ALSource);
									alSourcePlay((ALuint)ALSource);
								}
							}
						}
						else
						{
							alSourceStop((ALuint)ALSource);
							alGetSourcei((ALuint)ALSource,AL_LOOPING,&Looping);
							if (Looping)
							{
								alSourceRewind((ALuint)ALSource);
								alSourcePlay((ALuint)ALSource);
							}
						}
					}
					//Get source state
					alGetSourcei((ALuint)ALSource,AL_SOURCE_STATE,&State);
				}
				ALSource=ALSource->next;
			}
			//Post processing loop
			switch (format)
			{
				case AL_FORMAT_MONO8:
					for (i=0;i<(size/BytesPerSample);i++)
						((ALubyte *)buffer)[i]=aluF2S(DryBuffer[i][0]+DryBuffer[i][1]+WetBuffer[i][0]+WetBuffer[i][1])+128;
					break;
				case AL_FORMAT_STEREO8:
					for (i=0;i<(size/BytesPerSample);i++)
						((ALubyte *)buffer)[i]=aluF2S(DryBuffer[i>>1][i&1]+WetBuffer[i>>1][i&1])+128;
					break;
				case AL_FORMAT_MONO16:
					for (i=0;i<(size/BytesPerSample);i++)
						((ALshort *)buffer)[i]=aluF2S(DryBuffer[i][0]+DryBuffer[i][1]+WetBuffer[i][0]+WetBuffer[i][1]);
					break;
				case AL_FORMAT_STEREO16:
				default:
					for (i=0;i<(size/BytesPerSample);i++)
						((ALshort *)buffer)[i]=aluF2S(DryBuffer[i>>1][i&1]+WetBuffer[i>>1][i&1]);
					break;
			}
		}
		Error=alGetError();
		alcProcessContext(ALContext);
	}
}

