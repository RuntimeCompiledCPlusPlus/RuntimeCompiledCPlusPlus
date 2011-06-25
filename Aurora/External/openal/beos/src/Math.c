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

#include <math.h>
#include "Math.h"

ALvoid alimVectorAdd(const ALfloat *vector1, const ALfloat *vector2, ALfloat *vector)
{
	vector[0] = vector1[0] + vector2[0];
	vector[1] = vector1[1] + vector2[1];
	vector[2] = vector1[2] + vector2[2];
}

ALvoid alimVectorSub(const ALfloat *vector1, const ALfloat *vector2, ALfloat *vector)
{
	vector[0] = vector1[0] - vector2[0];
	vector[1] = vector1[1] - vector2[1];
	vector[2] = vector1[2] - vector2[2];
}

ALvoid alimVectorScale(ALfloat scale, const ALfloat *vector1, ALfloat *vector)
{
	vector[0] = scale * vector1[0];
	vector[1] = scale * vector1[1];
	vector[2] = scale * vector1[2];
}

ALfloat alimVectorDot(const ALfloat *vector1, const ALfloat *vector2)
{
	return (vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2]);
}

ALvoid alimVectorCross(const ALfloat *vector1, const ALfloat *vector2, ALfloat *vector)
{
	vector[0] = vector1[1] * vector2[2] - vector1[2] * vector2[1];
	vector[1] = vector1[2] * vector2[0] - vector1[0] * vector2[2];
	vector[2] = vector1[0] * vector2[1] - vector1[1] * vector2[0];
}

ALfloat alimVectorMagnitude(const ALfloat *vector)
{
	return sqrt(alimVectorDot(vector, vector));
}

ALvoid alimVectorNormalize(ALfloat *ioVector)
{
	ALfloat length, inverseLength;

	length = sqrt(alimVectorDot(ioVector, ioVector));
	if (length >= 0.00001f) {
		inverseLength = 1.0f / length;
		ioVector[0] *= inverseLength;
		ioVector[1] *= inverseLength;
		ioVector[2] *= inverseLength;
	}
}

ALvoid alimMatrixVector(ALfloat matrix[3][3], const ALfloat *vector, ALfloat *outVector)
{
	outVector[0] = matrix[0][0] * vector[0] + matrix[0][1] * vector[1] + matrix[0][2] * vector[2];
	outVector[1] = matrix[1][0] * vector[0] + matrix[1][1] * vector[1] + matrix[1][2] * vector[2];
	outVector[2] = matrix[2][0] * vector[0] + matrix[2][1] * vector[1] + matrix[2][2] * vector[2];
}
