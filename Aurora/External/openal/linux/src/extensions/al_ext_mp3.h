/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_ext_mp3.h
 *
 * Hack.
 */
#ifndef AL_EXT_MP3
#define AL_EXT_MP3

#include "AL/altypes.h"

/*
 * alutLoadMP3_LOKI( ALuint bid, ALvoid *data, ALint size )
 *
 * Associates bid with the mp3 data stream data, of size size in bytes.
 * Returns AL_TRUE if this was possible, AL_FALSE otherwise.
 */
ALboolean alutLoadMP3_LOKI( ALuint bid, ALvoid *data, ALint size );

/*
 *  MP3_Callback( ALuint sid, ALuint bid,
 *                ALshort *outdata,
 *                ALenum format, ALint freq, ALint samples);
 *
 * Callback function implementing mp3 decoding.
 */
ALint MP3_Callback(ALuint sid, ALuint bid,
			     ALshort *outdata,
			     ALenum format, ALint freq, ALint samples);

#ifndef OPENAL_EXTENSION

#ifdef SMPEG_SUPPORT


/*
 * we are being built into the standard library, so inform
 * the extension registrar
 */
#define BUILTIN_EXT_MP3                                       \
	AL_EXT_PAIR(alutLoadMP3_LOKI)

/* initialization and destruction functions */

#define BUILTIN_EXT_MP3_INIT
#define BUILTIN_EXT_MP3_FINI

#else

/*
 * Without smpeg support (--enable-smpeg) we can't do mp3s, so we
 * don't define any extensions.
 */

#define BUILTIN_EXT_MP3_INIT
#define BUILTIN_EXT_MP3_FINI

#endif /* SMPEG_SUPPORT */

#endif /* OPENAL_EXTENSION */

#endif /* AL_EXT_MP3 */
