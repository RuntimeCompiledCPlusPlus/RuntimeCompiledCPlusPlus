/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * ac_endian.h
 *
 * This file contains macros and prototypes for endian management.
 */
#include "al_siteconfig.h"
#include "AL/altypes.h"

ALushort swap16(ALushort D);
ALuint swap32(ALuint D);

ALubyte *cp16le(ALubyte *rawdata, ALushort *reader16);
ALubyte *cp32le(ALubyte *rawdata, ALuint *reader32);

#ifdef WORDS_BIGENDIAN
#define swap16le(x) swap16(x)
#define swap32le(x) swap32(x)
#define swap16be(x) (x)
#define swap32be(x) (x)
#else
#define swap16le(x) (x)
#define swap32le(x) (x)
#define swap16be(x) swap16(x)
#define swap32be(x) swap32(x)

#endif /* __big_endian */
