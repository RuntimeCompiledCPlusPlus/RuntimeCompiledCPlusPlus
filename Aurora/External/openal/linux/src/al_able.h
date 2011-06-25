/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_able.h
 *
 * Prototypes, macros and definitions related to alEnable/alDisable.
 */
#ifndef AL_ABLE_H_
#define AL_ABLE_H_

#include <AL/altypes.h>

/*
 * _alIsEnabled( ALenum param )
 *
 * Non locking version of alIsEnabled.
 */
ALboolean _alIsEnabled(ALenum param);

/*
 * _alEnabled( ALenum param )
 *
 * Non locking version of alEnabled.
 */
void _alEnable(ALenum param);

/*
 * _alDisable( ALenum param )
 *
 * Non locking version of alDisable.
 */
void _alDisable(ALenum param);

#endif /* AL_ABLE_H_ */
