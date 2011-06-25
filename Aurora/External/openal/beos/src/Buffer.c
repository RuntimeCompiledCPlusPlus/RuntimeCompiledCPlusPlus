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

#include "State.h"
#include "Memory.h"
#include "Context.h"
#include "Buffer.h"

ALbuffer *alimCreateBuffer(ALvoid)
{
	ALbuffer *buffer = (ALbuffer *) alimMemAlloc(sizeof(ALbuffer));

	if (buffer != NULL) {
		buffer->format = AL_FORMAT_MONO16;
		buffer->frequency = 22050;
		buffer->bits = 16;
		buffer->channels = 1;
		buffer->size = 0;
		buffer->data = NULL;
	}
	return buffer;
}

ALvoid alimDeleteBuffer(ALbuffer *buffer)
{
	if (buffer != NULL) {
		alimMemFree(buffer->data);
		alimMemFree(buffer);
	}
}

ALvoid alimBufferData(ALcontext *context, ALuint handle, ALenum format, ALvoid *data, ALsizei size, ALsizei frequency)
{
	ALbuffer *buffer = alimContextBuffer(context, handle);
	ALvoid *ptr;

	if (buffer == NULL || data == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alBufferData");
		return;
	}

	ptr = alimMemAlloc(size);
	if (ptr == NULL) {
		alimSetError(context, AL_OUT_OF_MEMORY, "alBufferData");
		return;
	}

	switch (format) {
	case AL_FORMAT_MONO8:
		alimMemFree(buffer->data);
		buffer->format = format;
		buffer->frequency = frequency;
		buffer->bits = 8;
		buffer->channels = 1;
		buffer->size = size;
		buffer->data = ptr;
		alimMemCopy(buffer->data, data, size);
		break;

	case AL_FORMAT_STEREO8:
		alimMemFree(buffer->data);
		buffer->format = format;
		buffer->frequency = frequency;
		buffer->bits = 8;
		buffer->channels = 2;
		buffer->size = size;
		buffer->data = ptr;
		alimMemCopy(buffer->data, data, size);
		break;

	case AL_FORMAT_MONO16:
		alimMemFree(buffer->data);
		buffer->format = format;
		buffer->frequency = frequency;
		buffer->bits = 16;
		buffer->channels = 1;
		buffer->size = size;
		buffer->data = ptr;
		alimMemCopy(buffer->data, data, size);
		break;

	case AL_FORMAT_STEREO16:
		alimMemFree(buffer->data);
		buffer->format = format;
		buffer->frequency = frequency;
		buffer->bits = 16;
		buffer->channels = 2;
		buffer->size = size;
		buffer->data = ptr;
		alimMemCopy(buffer->data, data, size);
		break;

	default:
		alimMemFree(ptr);
		alimSetError(context, AL_ILLEGAL_VALUE, "alBufferData");
		break;
	}
}

ALvoid alimGetBufferf(ALcontext *context, ALuint handle, ALenum pname, ALfloat *value)
{
	ALbuffer *buffer = alimContextBuffer(context, handle);

	if (buffer == NULL || value == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetBufferf");
		return;
	}

	switch (pname) {
	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetBufferf");
		break;
	}
}

ALvoid alimGetBufferi(ALcontext *context, ALuint handle, ALenum pname, ALint *value)
{
	ALbuffer *buffer = alimContextBuffer(context, handle);

	if (buffer == NULL || value == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGetBufferi");
		return;
	}

	switch (pname) {
	case AL_FREQUENCY:
		*value = buffer->frequency;
		break;

	case AL_BITS:
		*value = buffer->bits;
		break;

	case AL_CHANNELS:
		*value = buffer->channels;
		break;

	case AL_SIZE:
		*value = buffer->size;
		break;

	default:
		alimSetError(context, AL_ILLEGAL_VALUE, "alGetBufferi");
		break;
	}
}
