/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_listen.h
 *
 * Prototypes, macros and definitions related to the management of listeners.
 *
 */
#ifndef _AL_LISTENER_H_
#define _AL_LISTENER_H_

#include "al_types.h"
#include "alc/alc_context.h"

/*
 * _alInitListener( AL_listener *listener )
 *
 * Initializes a listener to the default values for all its elements.
 */
void _alInitListener( AL_listener *listener );

/*
 * _alDestroyListener( AL_listener *listener )
 *
 * Performs any needed finalization on a listener.
 */
void _alDestroyListener( AL_listener *listener );

/*
 * _alGetListenerParam( ALuint cid, ALenum param )
 *
 * Returns a pointer to the listener attribute specified by param for the
 * listener in the context named by cid.  If param is not a valid listener,
 * then NULL is returned.
 */
void *_alGetListenerParam( ALuint cid, ALenum param );

#define _alDCGetListenerParam(p) _alGetListenerParam( _alcCCId, p )

#endif /* _AL_LISTENER_H_ */
