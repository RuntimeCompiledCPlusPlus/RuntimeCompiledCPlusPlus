/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_ext_vorbis.h
 *
 * Hack.
 */
#ifndef AL_EXT_VORBIS
#define AL_EXT_VORBIS

#include "AL/altypes.h"

/*
 * Vorbis_Callback( ALuint sid, ALuint bid,
 *                  ALshort *outdata,
 *                  ALenum format, ALint freq, ALint samples )
 *
 * Callback which enabled alutLoadVorbis_LOKI to work.
 *
 */
ALint Vorbis_Callback(ALuint sid, ALuint bid,
			     ALshort *outdata,
			     ALenum format, ALint freq, ALint samples);

#ifndef OPENAL_EXTENSION

#ifdef VORBIS_SUPPORT


/*
 * we are being built into the standard library, so inform
 * the extension registrar
 */
#define BUILTIN_EXT_VORBIS                                       \
	AL_EXT_PAIR(alutLoadVorbis_LOKI)

/* initialization and destruction functions
 *
 * none needed for vorbis
 */

#define BUILTIN_EXT_VORBIS_INIT
#define BUILTIN_EXT_VORBIS_FINI

#else

/*
 * Without libvorvis (--enable-vorbis) we can't do vorbis, so we
 * don't define any extensions, or init/fini functions.
 */

#define BUILTIN_EXT_VORBIS_INIT
#define BUILTIN_EXT_VORBIS_FINI

#endif /* VORBIS_SUPPORT */

#endif /* OPENAL_EXTENSION */

#endif /* AL_EXT_VORBIS */
