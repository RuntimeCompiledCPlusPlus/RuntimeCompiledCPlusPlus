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

#ifndef __AL_MATH_H
#define __AL_MATH_H

#include <AL/altypes.h>

ALvoid alimVectorAdd(const ALfloat *vector1, const ALfloat *vector2, ALfloat *vector);

ALvoid alimVectorSub(const ALfloat *vector1, const ALfloat *vector2, ALfloat *vector);

ALvoid alimVectorScale(ALfloat scale, const ALfloat *vector1, ALfloat *vector);

ALfloat alimVectorDot(const ALfloat *vector1, const ALfloat *vector2);

ALvoid alimVectorCross(const ALfloat *vector1, const ALfloat *vector2, ALfloat *vector);

ALfloat alimVectorMagnitude(const ALfloat *vector);

ALvoid alimVectorNormalize(ALfloat *ioVector);

ALvoid alimMatrixVector(ALfloat matrix[3][3], const ALfloat *vector, ALfloat *outVector);

#endif
