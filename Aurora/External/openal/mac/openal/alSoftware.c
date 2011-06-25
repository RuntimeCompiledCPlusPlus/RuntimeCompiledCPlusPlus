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

#include <Math.h>
#include "globals.h" 
#include "alSource.h"
#include "alSoftware.h"

extern ALenum gDistanceModel;

ALAPI ALvoid ALAPIENTRY alCrossproduct(ALfloat vector1[3],ALfloat vector2[3],ALfloat vector3[3])
{
	vector3[0]=(vector1[1]*vector2[2]-vector1[2]*vector2[1]);
	vector3[1]=(vector1[2]*vector2[0]-vector1[0]*vector2[2]);
	vector3[2]=(vector1[0]*vector2[1]-vector1[1]*vector2[0]);
}

ALAPI ALfloat ALAPIENTRY alDotproduct(ALfloat vector1[3],ALfloat vector2[3])
{
	return (vector1[0]*vector2[0]+vector1[1]*vector2[1]+vector1[2]*vector2[2]);
}

ALAPI ALvoid ALAPIENTRY alNormalize(ALfloat vector[3])
{
	ALfloat length;

	length=(float)sqrt(alDotproduct(vector,vector));
	vector[0]/=length;
	vector[1]/=length;
	vector[2]/=length;
}

ALAPI ALvoid ALAPIENTRY alMatrixVector(ALfloat matrix[3][3],ALfloat vector[3])
{
	ALfloat result[3];

	result[0]=matrix[0][0]*vector[0]+matrix[0][1]*vector[1]+matrix[0][2]*vector[2];
	result[1]=matrix[1][0]*vector[0]+matrix[1][1]*vector[1]+matrix[1][2]*vector[2];
	result[2]=matrix[2][0]*vector[0]+matrix[2][1]*vector[1]+matrix[2][2]*vector[2];
	BlockMove(result, vector, sizeof(result));
}

ALAPI ALvoid ALAPIENTRY alCalculateSourceParameters(ALuint source,ALfloat *Pitch,ALfloat *Panning,ALfloat *Volume)
{
    ALfloat Position[3],Velocity[3],Distance;
	ALfloat U[3],V[3],N[3];
	ALfloat Matrix[3][3];

	if (alIsSource(source))
	{
		//Convert source position to listener's coordinate system
		alGetSourcef(source,AL_PITCH,Pitch);
		alGetSourcef(source,AL_GAIN,Volume);
		alGetSourcefv(source,AL_POSITION,Position);
		alGetSourcefv(source,AL_VELOCITY,Velocity);
		//1. Translate Listener to origin
		Position[0]-=gListener.Position[0];
		Position[1]-=gListener.Position[1];
		Position[2]-=gListener.Position[2];
		//2. Align coordinate system axis
		alCrossproduct(gListener.Up,gListener.Forward,U);
		alNormalize(U);
		alCrossproduct(gListener.Forward,U,V);
		alNormalize(V);
		BlockMove(gListener.Forward, N, sizeof(N));
		alNormalize(N);
		Matrix[0][0]=U[0]; Matrix[0][1]=V[0]; Matrix[0][2]=N[0];
		Matrix[1][0]=U[1]; Matrix[1][1]=V[1]; Matrix[1][2]=N[1];
		Matrix[2][0]=U[2]; Matrix[2][1]=V[2]; Matrix[2][2]=N[2];
		alMatrixVector(Matrix,Position);			
		//3. Convert into falloff and panning
		Distance=(float)sqrt(Position[0]*Position[0]+Position[1]*Position[1]+Position[2]*Position[2]) * gDistanceScale;
		if (Distance)		
			*Panning=(float)(0.5+0.5*0.707*cos(atan2(Position[2],Position[0])));
		else
			*Panning=(float)(0.5);
			
			
		if ((gDistanceModel!= AL_NONE) && (Distance > 0))
		{
			if (gDistanceModel==AL_INVERSE_DISTANCE_CLAMPED)
			{
				Distance=(Distance<gSource[source].referenceDistance?gSource[source].referenceDistance:Distance);
				Distance=(Distance>gSource[source].maxDistance?gSource[source].maxDistance:Distance);
			}
			*Volume=(gSource[source].gain*gListener.Gain*gSource[source].referenceDistance)/(gSource[source].referenceDistance+gSource[source].rolloffFactor*(Distance-gSource[source].referenceDistance));
		}
		else
			*Volume=(gSource[source].gain*gListener.Gain);	
			
		//4. Calculate doppler
		//Pitch*=((343.0f-ListenerSpeed)/(343.0f+SourceSpeed));
	}
}