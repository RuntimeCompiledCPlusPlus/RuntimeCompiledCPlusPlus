/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_ext_capture.h
 *
 * Prototypes for audio recording extension
 */
#ifndef AL_EXT_CAPTURE_H_
#define AL_EXT_CAPTURE_H_

#include "al_ext_needed.h"
#include "al_types.h"

#include <AL/alext.h>

/*
 * alInitCapture( void )
 *
 * Initializes data structures needed by all captures.
 */
void alInitCapture( void );

/*
 * alFiniCapture( void )
 *
 * Finalizes all things needing finalization by all captures.
 */
void alFiniCapture( void );

#ifndef OPENAL_EXTENSION

/*
 * we are being built into the standard library, so inform
 * the extension registrar
 */
#define BUILTIN_EXT_CAPTURE                                        \
	AL_EXT_PAIR(alCaptureInit_EXT),                            \
	AL_EXT_PAIR(alCaptureStart_EXT),                           \
	AL_EXT_PAIR(alCaptureStop_EXT),                            \
	AL_EXT_PAIR(alCaptureGetData_EXT),                         \
	AL_EXT_PAIR(alCaptureDestroy_EXT)                         \

/* initialization and destruction functions */

#define BUILTIN_EXT_CAPTURE_INIT  alInitCapture()
#define BUILTIN_EXT_CAPTURE_FINI  alFiniCapture()

#else

#define BUILTIN_EXT_CAPTURE_INIT
#define BUILTIN_EXT_CAPTURE_FINI

#endif /* OPENAL_EXTENSION */

#endif /* AL_EXT_CAPTURE_H_ */
