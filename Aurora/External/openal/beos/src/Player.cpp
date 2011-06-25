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

#include <Application.h>
#include <SoundPlayer.h>
#include <ByteOrder.h>
extern "C" {
#include "State.h"
#include "Memory.h"
#include "Context.h"
#include "Player.h"
}

static void alimPlayerHook(void *cookie, void *buffer, size_t size, const media_raw_audio_format &format)
{
	ALcontext *context = (ALcontext *) cookie;

	if (context != NULL && buffer != NULL) {
		ALplayer *player = alimContextPlayer(context);
		if (player != NULL && player->hook != NULL)
			player->hook(context, buffer, size);
	}
}

ALvoid alimInitApplication(ALvoid)
{
	if (be_app == NULL)
		be_app = new BApplication("application/x-vnd.openal.player");
}

ALvoid alimExitApplication(ALvoid)
{
}

ALplayer *alimCreatePlayer(ALuint frequency, ALenum format, ALsizei size)
{
	ALplayer *player = (ALplayer *) alimMemAlloc(sizeof(ALplayer));

	if (player != NULL) {
		media_raw_audio_format fmt;

		fmt.frame_rate = frequency;
		fmt.channel_count = (format == AL_FORMAT_MONO8 || format == AL_FORMAT_MONO16 ? 1 : 2);
		fmt.format = (format == AL_FORMAT_MONO8 || format == AL_FORMAT_STEREO8 ?
			media_raw_audio_format::B_AUDIO_UCHAR :
			media_raw_audio_format::B_AUDIO_SHORT);
		fmt.byte_order = B_MEDIA_HOST_ENDIAN;
		fmt.buffer_size = size;

		player->device = new BSoundPlayer(&fmt, "OpenAL", alimPlayerHook);
		player->frequency = frequency;
		player->format = format;
		player->size = size;
		player->hook = NULL;
	}
	return player;
}

ALvoid alimDeletePlayer(ALplayer *player)
{
	if (player != NULL) {
		delete (BSoundPlayer *) player->device;
		alimMemFree(player);
	}
}

ALvoid alimPlayerStart(ALcontext *context)
{
	ALplayer *player = alimContextPlayer(context);

	BSoundPlayer *sound;

	if (player != NULL && player->device != NULL) {
		sound = (BSoundPlayer *) player->device;
		sound->SetCookie(context);
		sound->SetVolumeDB(0.0f);
		sound->Start();
	}
}

ALvoid alimPlayerStop(ALcontext *context)
{
	ALplayer *player = alimContextPlayer(context);

	BSoundPlayer *sound;

	if (player != NULL && player->device != NULL) {
		sound = (BSoundPlayer *) player->device;
		sound->Stop();
		sound->SetCookie(NULL);
	}
}

ALvoid alimPlayerHook(ALcontext *context, ALvoid (*hook)(ALcontext *, ALvoid *, ALsizei))
{
	ALplayer *player = alimContextPlayer(context);

	if (player != NULL && player->device != NULL) {
		BSoundPlayer *sound = (BSoundPlayer *) player->device;
		player->hook = hook;
		sound->SetHasData(player->hook != NULL);
	}
}
