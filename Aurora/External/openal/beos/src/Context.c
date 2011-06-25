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
#include "Thread.h"
#include "Memory.h"
#include "Table.h"
#include "State.h"
#include "Player.h"
#include "Listener.h"
#include "Source.h"
#include "Buffer.h"
#include "Environment.h"
#include "Context.h"
#include "Mixer.h"


static ALuint CurrentContext = 0;

ALvoid alimInit(ALvoid)
{
	alimInitApplication();
	CurrentContext = alimCreateTLS();
	alimSetTLS(CurrentContext, NULL);
}

ALvoid alimExit(ALvoid)
{
	alimDeleteTLS(CurrentContext);
	alimExitApplication();
}

ALcontext *alimGetCurrent(ALvoid)
{
	ALcontext *context = (ALcontext *) alimGetTLS(CurrentContext);
	if (context == NULL)
		alimDebugger("alGetCurrent: No context for the current thread");
	return context;
}

ALvoid alimMakeCurrent(ALcontext *context)
{
	ALcontext *current = (ALcontext *) alimGetTLS(CurrentContext);
	if (current != NULL)
		alimUnlockMutex(current->mutex);
	if (context != NULL)
		alimLockMutex(context->mutex);
	alimSetTLS(CurrentContext, context);
}

static ALvoid alimContextCallback(ALcontext *context, ALvoid *buffer, ALsizei size)
{
	if (context != NULL && buffer != NULL) {
		/* FIXME: lock the mixer and the resources it uses */
		alimMixerData(context, buffer, size);
	}
}

ALcontext *alimCreateContext(ALuint frequency, ALenum format, ALsizei size)
{
	ALcontext *context = (ALcontext *) alimMemAlloc(sizeof(ALcontext));

	if (context != NULL) {
		context->mutex = alimCreateMutex();
		context->player = alimCreatePlayer(frequency, format, size);
		context->mixer = alimCreateMixer(frequency, format, size);
		context->state = alimCreateState();
		context->listener = alimCreateListener();
		context->sources = alimCreateTable();
		context->buffers = alimCreateTable();
		context->environments = alimCreateTable();

		if (context->mutex != NULL && context->player != NULL &&
			context->mixer != NULL && context->state != NULL &&
			context->listener != NULL && context->sources != NULL &&
			context->buffers != NULL && context->environments != NULL) {
			alimPlayerHook(context, alimContextCallback);
			alimPlayerStart(context);
		}
		else {
			alimDeleteContext(context);
			context = NULL;
		}
	}
	return context;
}

ALvoid alimDeleteContext(ALcontext *context)
{
	ALvoid *cookie, *object;

	if (context != NULL) {
		alimPlayerStop(context);
		alimPlayerHook(context, NULL);

		cookie = NULL;
		while ((object = alimTableGetEntries(context->environments, &cookie)) != NULL)
			alimDeleteEnvironmentIASIG((ALenvironment *) object);

		cookie = NULL;
		while ((object = alimTableGetEntries(context->buffers, &cookie)) != NULL)
			alimDeleteBuffer((ALbuffer *) object);

		cookie = NULL;
		while ((object = alimTableGetEntries(context->sources, &cookie)) != NULL)
			alimDeleteSource((ALsource *) object);

		alimDeleteTable(context->environments);
		alimDeleteTable(context->buffers);
		alimDeleteTable(context->sources);
		alimDeleteListener(context->listener);
		alimDeleteState(context->state);
		alimDeleteMixer(context->mixer);
		alimDeletePlayer(context->player);
		alimDeleteMutex(context->mutex);

		alimMemFree(context);
	}
}

ALvoid alimUpdateContext(ALcontext *context)
{
}

ALplayer *alimContextPlayer(ALcontext *context)
{
	return context->player;
}

ALmixer *alimContextMixer(ALcontext *context)
{
	return context->mixer;
}

ALstate *alimContextState(ALcontext *context)
{
	return context->state;
}

ALlistener *alimContextListener(ALcontext *context)
{
	return context->listener;
}

ALsizei alimGenBuffers(ALcontext *context, ALsizei n, ALuint *buffers)
{
	ALbuffer *buffer;
	ALsizei i;

	if (buffers == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGenBuffers");
		return 0;
	}

	for (i = 0; i < n; i++) {
		buffers[i] = alimTableCreateEntry(context->buffers);
		buffer = alimCreateBuffer();

		if (buffers[i] != 0 && buffer != NULL) {
			alimTableSetEntry(context->buffers, buffers[i], buffer);
		}
		else {
			alimDeleteBuffer(buffer);
			alimTableDeleteEntry(context->buffers, buffers[i]);
			alimSetError(context, AL_OUT_OF_MEMORY, "alGenBuffers");
			break;
		}
	}

	return i;
}

ALvoid alimDeleteBuffers(ALcontext *context, ALsizei n, ALuint *buffers)
{
	ALbuffer *buffer;
	ALsizei i;

	if (buffers == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alDeleteBuffers");
		return;
	}

	for (i = 0; i < n; i++) {
		buffer = alimContextBuffer(context, buffers[i]);

		if (buffer != NULL) {
			alimDeleteBuffer(buffer);
			alimTableDeleteEntry(context->buffers, buffers[i]);
		}
		else {
			alimSetError(context, AL_ILLEGAL_VALUE, "alDeleteBuffers");
		}
	}
}

ALbuffer *alimContextBuffer(ALcontext *context, ALuint handle)
{
	return (ALbuffer *) alimTableGetEntry(context->buffers, handle);
}

ALboolean alimIsBuffer(ALcontext *context, ALuint handle)
{
	return alimTableHasEntry(context->buffers, handle);
}

ALsizei alimGenSources(ALcontext *context, ALsizei n, ALuint *sources)
{
	ALsource *source;
	ALsizei i;

	if (sources == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGenSources");
		return 0;
	}

	for (i = 0; i < n; i++) {
		sources[i] = alimTableCreateEntry(context->sources);
		source = alimCreateSource();

		if (sources[i] != 0 && source != NULL) {
			alimTableSetEntry(context->sources, sources[i], source);
		}
		else {
			alimDeleteSource(source);
			alimTableDeleteEntry(context->sources, sources[i]);
			alimSetError(context, AL_OUT_OF_MEMORY, "alGenSources");
			break;
		}
	}

	return i;
}

ALvoid alimDeleteSources(ALcontext *context, ALsizei n, ALuint *sources)
{
	ALsource *source;
	ALsizei i;

	if (sources == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGenSources");
		return;
	}

	for (i = 0; i < n; i++) {
		source = alimContextSource(context, sources[i]);

		if (source != NULL) {
			alimDeleteSource(source);
			alimTableDeleteEntry(context->sources, sources[i]);
		}
		else {
			alimSetError(context, AL_ILLEGAL_VALUE, "alDeleteSources");
		}
	}
}

ALsource *alimContextSource(ALcontext *context, ALuint handle)
{
	return (ALsource *) alimTableGetEntry(context->sources, handle);
}

ALboolean alimIsSource(ALcontext *context, ALuint handle)
{
	return alimTableHasEntry(context->sources, handle);
}

ALsizei alimGenEnvironmentsIASIG(ALcontext *context, ALsizei n, ALuint *environments)
{
	ALenvironment *environment;
	ALsizei i;

	if (environments == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGenEnvironmentsIASIG");
		return 0;
	}

	for (i = 0; i < n; i++) {
		environments[i] = alimTableCreateEntry(context->environments);
		environment = alimCreateEnvironmentIASIG();

		if (environments[i] != 0 && environment != NULL) {
			alimTableSetEntry(context->environments, environments[i], environment);
		}
		else {
			alimDeleteEnvironmentIASIG(environment);
			alimTableDeleteEntry(context->environments, environments[i]);
			alimSetError(context, AL_OUT_OF_MEMORY, "alGenEnvironmentsIASIG");
			break;
		}
	}

	return i;
}

ALvoid alimDeleteEnvironmentsIASIG(ALcontext *context, ALsizei n, ALuint *environments)
{
	ALenvironment *environment;
	ALsizei i;

	if (environments == NULL) {
		alimSetError(context, AL_ILLEGAL_OPERATION, "alGenEnvironmentsIASIG");
		return;
	}

	for (i = 0; i < n; i++) {
		environment = alimContextEnvironmentIASIG(context, environments[i]);

		if (environment != NULL) {
			alimDeleteEnvironmentIASIG(environment);
			alimTableDeleteEntry(context->environments, environments[i]);
		}
		else {
			alimSetError(context, AL_ILLEGAL_VALUE, "alDeleteEnvironmentsIASIG");
		}
	}
}

ALenvironment *alimContextEnvironmentIASIG(ALcontext *context, ALuint handle)
{
	return (ALenvironment *) alimTableGetEntry(context->environments, handle);
}

ALboolean alimIsEnvironmentIASIG(ALcontext *context, ALuint handle)
{
	return alimTableHasEntry(context->environments, handle);
}
