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

#include <assert.h>
#include <math.h>
#include "Thread.h"
#include "Memory.h"
#include "State.h"
#include "Player.h"
#include "Mixer.h"
#include "Math.h"
#include "Context.h"


/*
 * FIXME:
 * - What happens if the user releases a source or buffer while the mixer
 *   is still using any of them? [ Use reference counting ]
 *
 * - The mixer should be made thread-safe by locking every resources its
 *   using or by changing the context locking mechanism?
 *
 * TODO:
 * - Synchronize the changes to the sources, buffers, environments, etc.
 *   with the channels of the mixer.
 *
 * - Compute channel parameters based on 3D positional information,
 *   environment settings, speaker configuration, etc.
 *
 * - Support FFT and FIR/IIR filters, more buffer formats, etc.
 */

enum {
	AL_MAX_CHANNELS	= 32,
	AL_CHANNEL_FRACTION_BITS = 12
};

static const ALfloat AL_SOUND_VELOCITY = 343.0f;
static const ALfloat AL_REFERENCE_DISTANCE = 1.0f;
static const ALfloat AL_EPSILON = 0.00001f;
static const ALfloat AL_SPEAKER_ANGLE = 40.0f;


static ALvoid alimMixerDR(ALlistener *listener, ALchannel *channel)
{
	assert(listener != NULL);
	assert(channel != NULL);
	assert(channel->source != NULL);

	/* compute relative position and velocity of the source */
	alimVectorSub(channel->source->position, listener->position, channel->dr.position);
	alimVectorSub(channel->source->velocity, listener->velocity, channel->dr.velocity);

	/* compute the distance and speed of the source */
	channel->dr.distance = alimVectorMagnitude(channel->dr.position);
}

static ALvoid alimMixerDoppler(ALchannel *channel)
{
	ALfloat velocity;

	assert(channel != NULL);

	/* compute the doppler pitch shift for the source */
	velocity = alimVectorDot(channel->dr.position, channel->dr.velocity);
	velocity /= channel->dr.distance + AL_EPSILON;

	if (velocity > -AL_SOUND_VELOCITY)
		channel->doppler.pitch = AL_SOUND_VELOCITY / (AL_SOUND_VELOCITY + velocity);
	else
		channel->doppler.pitch = 0.0f;
}

static ALvoid alimMixerRolloff(ALchannel *channel)
{
	assert(channel != NULL);

	/* compute the distance attenuation for the source */
	if (channel->dr.distance >= AL_REFERENCE_DISTANCE)
		channel->rolloff.gain = AL_REFERENCE_DISTANCE / channel->dr.distance;
	else
		channel->rolloff.gain = 1.0f;
}

static ALvoid alimMixerCone(ALchannel *channel)
{
	ALfloat angle, scale, innerAngle, outerAngle, innerGain, outerGain;

	assert(channel != NULL);
	assert(channel->source != NULL);

	/* compute the attenuation of the radiation pattern */
	if (channel->source->coneInnerAngle >= 180) {
		/* omnidirectional sound source */
		channel->cone.gain = 1.0f;
	}
	else {
		/* compute cosine of the inner and outer angles */
		innerAngle = cos(2.0 * M_PI / 360.0 * channel->source->coneInnerAngle);
		outerAngle = cos(2.0 * M_PI / 360.0 * channel->source->coneOuterAngle);

		innerGain = 1.0f;
		outerGain = channel->source->coneOuterGain;

		/* angle of the listener with respect to the direction of source */
		angle = -alimVectorDot(channel->dr.position, channel->source->direction);

		scale = alimVectorMagnitude(channel->dr.position) *
			alimVectorMagnitude(channel->source->direction);
		innerAngle *= scale;
		outerAngle *= scale;

		if (angle >= innerAngle) {
			/* listener inside of the inner cone */
			channel->cone.gain = innerGain;
		}
		else if (angle >= outerAngle) {
			/* listener inside of the outer cone */
			channel->cone.gain = innerGain +
				(innerAngle - angle) / (innerAngle - outerAngle) * (outerGain - innerGain);
		}
		else {
			/* listener outside of the cones */
			channel->cone.gain = outerGain;
		}
	}
}

static ALvoid alimMixerPanning(ALlistener *listener, ALchannel *channel)
{
	float pan, muffle, azim, x, z;

	assert(listener != NULL);
	assert(channel != NULL);

	/* align the coordinate system of the source */
	alimMatrixVector(listener->orientation, channel->dr.position, channel->pan.position);
	alimVectorNormalize(channel->pan.position);

	/* compute stereo position of the channel */
	x = channel->pan.position[0];
	z = channel->pan.position[2];
	
	/* FIXME: use a better algorithm */
	if (z >= 0.0f)
		azim = 180 * acos(x) / M_PI;
	else
		azim = 360 - 180 * acos(x) / M_PI;

	if (azim >= 0 && azim <= 180)
		pan = 1 - azim / 180;
	else
		pan = (azim - 180) / 180;

	if (azim >= 90 && azim <= 270) {
		muffle = 0.0f + (0.9f - 0.0f) * (azim - 90) / 180;
	}
	else {
		if (azim >= 270)
			azim -= 360;
		muffle = 0.0f + (0.9f - 0.0f) * (90 - azim) / 180;
	}
	
	/* save stereo position and muffling coefficient */
	channel->pan.position[0] = 1 - pan;
	channel->pan.position[1] = pan;
	channel->pan.position[2] = muffle;
}

static ALvoid alimMixerResample8M(ALchannel *channel, ALfloat *buffer, ALsizei count)
{
	ALbyte *data;
	ALuint fraction, pitch;
	ALfloat volume[2];
 	ALint decay, smp;

	assert(buffer != NULL);
	assert(channel != NULL);
	assert(channel->buffer != NULL);

	data = (ALbyte *) channel->buffer->data;

	if (data != NULL) {
		data += channel->position;
		fraction = channel->fraction;
		pitch = channel->pitch;

		volume[0] = 256 * channel->volume[0];
		volume[1] = 256 * channel->volume[1];

		decay = (ALint) (65536 * channel->muffle.decay);
		smp = channel->muffle.sample;

		while (count-- != 0) {
			ALuint i = fraction >> AL_CHANNEL_FRACTION_BITS;
			ALuint f = fraction & ((1 << AL_CHANNEL_FRACTION_BITS) - 1);
			
			ALbyte s0 = (ALbyte) (data[i] ^ 0x80);
			ALbyte s1 = (ALbyte) (data[i+1] ^ 0x80);
			
			ALshort sample = s0 + ((f * (s1 - s0)) >> AL_CHANNEL_FRACTION_BITS);
			
			smp = (sample += ((decay * (smp - sample)) >> 16));

			buffer[0] += volume[0] * sample;
			buffer[1] += volume[1] * sample;

			fraction += pitch;
			buffer += 2;
		}

		channel->muffle.sample = smp;

		channel->position += fraction >> AL_CHANNEL_FRACTION_BITS;
		channel->fraction = fraction & ((1 << AL_CHANNEL_FRACTION_BITS) - 1);
	}
}

static ALvoid alimMixerResample16M(ALchannel *channel, ALfloat *buffer, ALsizei count)
{
	ALshort *data;
	ALuint fraction, pitch;
	ALfloat volume[2];
 	ALint decay, smp;
 
	assert(buffer != NULL);
	assert(channel != NULL);
	assert(channel->buffer != NULL);

	data = (ALshort *) channel->buffer->data;

	if (data != NULL) {
		data += channel->position;
		fraction = channel->fraction;
		pitch = channel->pitch;

		volume[0] = channel->volume[0];
		volume[1] = channel->volume[1];

		decay = (int)(65536 * channel->muffle.decay);
		smp = channel->muffle.sample;
		
		while (count-- != 0) {
			ALuint i = fraction >> AL_CHANNEL_FRACTION_BITS;
			ALuint f = fraction & ((1 << AL_CHANNEL_FRACTION_BITS) - 1);

			ALshort s0 = data[i];
			ALshort s1 = data[i + 1];

			ALshort sample = s0 + ((f * (s1 - s0)) >> AL_CHANNEL_FRACTION_BITS);
			
			smp = (sample += ((decay * (smp - sample)) >> 16));

			buffer[0] += volume[0] * sample;
			buffer[1] += volume[1] * sample;

			fraction += pitch;
			buffer += 2;
		}
		
		channel->muffle.sample = smp;

		channel->position += fraction >> AL_CHANNEL_FRACTION_BITS;
		channel->fraction = fraction & ((1 << AL_CHANNEL_FRACTION_BITS) - 1);
	}
}

static ALvoid alimMixerConvert16S(const ALfloat *floatBuffer, ALshort *buffer, ALsizei count)
{
	count <<= 1;
	while (count-- != 0) {
		ALint sample = (ALint) *floatBuffer++;
		if (sample < -32768)
			sample = -32768;
		else if (sample > 32767)
			sample = 32767;
		*buffer++ = (ALshort) sample;
	}
}



ALmixer *alimCreateMixer(ALuint frequency, ALenum format, ALsizei size)
{
	ALmixer *mixer = (ALmixer *) alimMemAlloc(sizeof(ALmixer));
	ALsizei i;

	if (mixer != NULL) {
		mixer->mutex = alimCreateMutex();
		mixer->buffer = alimMemAlloc(sizeof(ALfloat) * size);
		mixer->channels = alimMemAlloc(sizeof(ALchannel) * AL_MAX_CHANNELS);
		mixer->frequency = frequency;
		mixer->format = format;
		mixer->size = size;

		if (mixer->mutex != NULL && mixer->buffer != NULL && mixer->channels != NULL) {
			for (i = 0; i < AL_MAX_CHANNELS; i++) {
				mixer->channels[i].source = NULL;
				mixer->channels[i].buffer = NULL;

				mixer->channels[i].active = AL_FALSE;
				mixer->channels[i].dirty = AL_FALSE;
				mixer->channels[i].pitch = 0;

				mixer->channels[i].position = 0;
				mixer->channels[i].fraction = 0;

				mixer->channels[i].volume[0] = 1.0f;
				mixer->channels[i].volume[1] = 1.0f;

				mixer->channels[i].muffle.decay = 0.0f;
				mixer->channels[i].muffle.sample = 0.0f;

				mixer->channels[i].dr.position[0] = 0.0f;
				mixer->channels[i].dr.position[1] = 0.0f;
				mixer->channels[i].dr.position[2] = 0.0f;

				mixer->channels[i].dr.velocity[0] = 0.0f;
				mixer->channels[i].dr.velocity[1] = 0.0f;
				mixer->channels[i].dr.velocity[2] = 0.0f;

				mixer->channels[i].dr.distance = 0.0f;

				mixer->channels[i].doppler.pitch = 1.0f;
				mixer->channels[i].rolloff.gain = 1.0f;
				mixer->channels[i].cone.gain = 1.0f;

				mixer->channels[i].pan.position[0] = 0.0f;
				mixer->channels[i].pan.position[1] = 0.0f;
				mixer->channels[i].pan.position[2] = 0.0f;
			}
		}
		else {
			alimDeleteMixer(mixer);
			mixer = NULL;
		}
	}
	return mixer;
}

ALvoid alimDeleteMixer(ALmixer *mixer)
{
	if (mixer != NULL) {
		alimMemFree(mixer->channels);
		alimMemFree(mixer->buffer);
		alimDeleteMutex(mixer->mutex);
		alimMemFree(mixer);
	}
}

ALvoid alimMixerStartSource(ALcontext *context, ALsource *source)
{
	ALmixer *mixer = alimContextMixer(context);
	ALsizei i;

	if (mixer == NULL || source == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alMixerStartSource");
		return;
	}

	if (source->channel == NULL) {
		for (i = 0; i < AL_MAX_CHANNELS; i++) {
			if (mixer->channels[i].source == NULL) {
				source->channel = mixer->channels + i;
				break;
			}
		}
	}

	if (source->channel == NULL) {
		alimSetError(context, AL_OUT_OF_MEMORY, "alMixerStartSource");
		return;
	}

	source->channel->active = AL_TRUE;
	source->channel->dirty = AL_TRUE;
	source->channel->source = source;

	alimMixerUpdateSource(context, source);
}

ALvoid alimMixerStopSource(ALcontext *context, ALsource *source)
{
	ALmixer *mixer = alimContextMixer(context);

	if (mixer == NULL || source == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alMixerStopSource");
		return;
	}

	if (source->channel != NULL) {
		source->channel->active = AL_FALSE;
		source->channel->source = NULL;
		source->channel->position = 0;
		source->channel->fraction = 0;
		source->channel = NULL;
	}
}

ALvoid alimMixerPauseSource(ALcontext *context, ALsource *source)
{
	ALmixer *mixer = alimContextMixer(context);

	if (mixer == NULL || source == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alMixerPauseSource");
		return;
	}

	if (source->channel != NULL) {
		source->channel->active = AL_TRUE;
	}
}

ALvoid alimMixerUpdateSource(ALcontext *context, ALsource *source)
{
	ALmixer *mixer = alimContextMixer(context);

	if (mixer == NULL || source == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alMixerUpdateSource");
		return;
	}

	/* update the channel associated to the source */
	if (source->channel != NULL) {
		source->channel->buffer = alimContextBuffer(context, source->buffer);
		source->channel->dirty = AL_TRUE;
	}
}

ALvoid alimMixerUpdateListener(ALcontext *context)
{
	ALmixer *mixer = alimContextMixer(context);
	ALsizei i;

	if (mixer == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alMixerUpdateListener");
		return;
	}

	/* update all the active channels */
	for (i = 0; i < AL_MAX_CHANNELS; i++) {
		if (mixer->channels[i].source != NULL)
			alimMixerUpdateSource(context, mixer->channels[i].source);
	}
}

ALvoid alimMixerData(ALcontext *context, ALvoid *buffer, ALsizei size)
{
	ALmixer *mixer = alimContextMixer(context);
	ALlistener *listener = alimContextListener(context);
	ALsizei i, frames;

	if (mixer == NULL || listener == NULL || buffer == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alMixerData");
		return;
	}

	if (mixer->size < size) {
		alimSetError(context, AL_ILLEGAL_VALUE, "alMixerData");
		return;
	}

	/* number of frames in the buffer */
	frames = size;
	if (mixer->format == AL_FORMAT_STEREO8)
		frames >>= 1;
	if (mixer->format == AL_FORMAT_MONO16)
		frames >>= 1;
	if (mixer->format == AL_FORMAT_STEREO16)
		frames >>= 2;

	/* initialize the mixing buffer */
	for (i = 0; i < 2 * frames; i++) {
		mixer->buffer[i] = 0.0f;
	}

	/* process the active channels */
	for (i = 0; i < AL_MAX_CHANNELS; i++) {
		ALchannel *channel = mixer->channels + i;

		if (channel->source != NULL && channel->buffer != NULL && channel->buffer->size != 0) {
			ALfloat *buffer = mixer->buffer;
			ALsizei size = frames;

			/* update the channel parameters whenever required */
			if (channel->dirty == AL_TRUE) {
				channel->dirty = AL_FALSE;

				/* compute the filter parameters of the channel */
				alimMixerDR(listener, channel);
				alimMixerDoppler(channel);
				alimMixerRolloff(channel);
				alimMixerCone(channel);
				alimMixerPanning(listener, channel);

				/* compute pitch shift and volume for the channel */
				channel->pitch = (ALuint) (
					channel->doppler.pitch * channel->source->pitch *
					(channel->buffer->frequency << AL_CHANNEL_FRACTION_BITS) /
					mixer->frequency);

				channel->volume[0] =
				channel->volume[1] = listener->gain * channel->source->gain *
					channel->rolloff.gain * channel->cone.gain;

				channel->volume[0] *= channel->pan.position[0];
				channel->volume[1] *= channel->pan.position[1];
				
				channel->muffle.decay = channel->pan.position[2];
			}

			if (channel->pitch != 0) {
				while (channel->active == AL_TRUE && size != 0) {
					/* compute the channel buffer size in frames */
					ALsizei count = channel->buffer->size;
					if (channel->buffer->format == AL_FORMAT_STEREO8)
						count >>= 1;
					if (channel->buffer->format == AL_FORMAT_MONO16)
						count >>= 1;
					if (channel->buffer->format == AL_FORMAT_STEREO16)
						count >>= 2;

					/* check the channel position */
					if (channel->position >= count) {
						if (channel->source->looping) {
							channel->position -= count;
						}
						else {
							channel->source->channel = NULL;
							channel->active = AL_FALSE;
							channel->source = NULL;
							channel->position = 0;
							channel->fraction = 0;
							break;
						}
					}

					/* compute number of frames to resample from the buffer */
					count -= channel->position;
					if (count <= (1 << (31 - AL_CHANNEL_FRACTION_BITS))) {
						count = (count << AL_CHANNEL_FRACTION_BITS) - channel->fraction;
						if (count < size * channel->pitch)
							count = (count + channel->pitch - 1) / channel->pitch;
						else
							count = size;
					}
					else {
						/* FIXME: for now assume that there are enough samples */
						count = size;
					}

					assert(count != 0);

					/* TODO: add filters and mixers for other buffer formats */
					if (channel->buffer->format == AL_FORMAT_MONO8)
						alimMixerResample8M(channel, buffer, count);

					if (channel->buffer->format == AL_FORMAT_MONO16)
						alimMixerResample16M(channel, buffer, count);

					buffer += count << 1;
					size -= count;
				}
			}
		}
	}

	/* TODO: add conversors for other output formats */
	if (mixer->format == AL_FORMAT_STEREO16)
		alimMixerConvert16S(mixer->buffer, buffer, frames);
}
