/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_ext_loki.h
 *
 * Prototypes for (semi) standard loki extensions
 */
#ifndef AL_EXT_LOKI_H_
#define AL_EXT_LOKI_H_

#include "al_ext_needed.h"
#include <AL/alext.h>

/*
 * Don't use these.  If you're reading this, you should remove these functions
 * and all other reverb functions.  Now.
 */
void alReverbScale_LOKI(ALuint sid, ALfloat param);
void alReverbDelay_LOKI(ALuint sid, ALfloat param);

/*
 * alInitLoki( void )
 *
 * Initializes data structures needed by the loki extensions.
 */
void alInitLoki( void );

/*
 * alFiniLoki( void )
 *
 * Finalizes data structures needed by the loki extensions.
 */
void alFiniLoki( void );

#ifndef OPENAL_EXTENSION

/*
 * we are being built into the standard library, so inform
 * the extension registrar
 */
#define BUILTIN_EXT_LOKI                                      \
	AL_EXT_PAIR(alutLoadMS_ADPCMData_LOKI),               \
	AL_EXT_PAIR(alutLoadIMA_ADPCMData_LOKI),              \
	AL_EXT_PAIR(alutLoadRAW_ADPCMData_LOKI),              \
	AL_EXT_PAIR(alBufferAppendData_LOKI),                 \
	AL_EXT_PAIR(alBufferWriteData_LOKI),                  \
	AL_EXT_PAIR(alBufferAppendWriteData_LOKI),            \
	AL_EXT_PAIR(alReverbScale_LOKI),                      \
	AL_EXT_PAIR(alReverbDelay_LOKI),                      \
	AL_EXT_PAIR(alBombOnError_LOKI),                      \
	AL_EXT_PAIR(alBufferi_LOKI),                          \
	AL_EXT_PAIR(alBufferDataWithCallback_LOKI),           \
	AL_EXT_PAIR(alGenStreamingBuffers_LOKI),              \
	AL_EXT_PAIR(alcGetAudioChannel_LOKI),                 \
	AL_EXT_PAIR(alcSetAudioChannel_LOKI)


/* initialization and destruction functions */

#define BUILTIN_EXT_LOKI_INIT  alInitLoki()
#define BUILTIN_EXT_LOKI_FINI  alFiniLoki()

#else

#define BUILTIN_EXT_LOKI_INIT
#define BUILTIN_EXT_LOKI_FINI

#endif /* OPENAL_EXTENSION */

#endif /* AL_EXT_LOKI_H_ */
