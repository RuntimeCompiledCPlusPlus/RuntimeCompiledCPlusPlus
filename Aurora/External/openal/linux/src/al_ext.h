/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_ext.h
 *
 * Extension handling.
 */
#ifndef _AL_EXT_H_
#define _AL_EXT_H_

#include "al_siteconfig.h"
#include "al_types.h"
#include "al_filter.h"
#include "al_config.h"

/*
 * the LAL... defines are used in conjunction with plugins, to determine the
 * symbols defined in each at runtime.
 */

#define LAL_EXT_SUFFIX		03282000
#define LAL_EXT_TABLE		alExtension
#define LAL_EXT_INIT		alExtInit
#define LAL_EXT_FINI		alExtFini

#define LAL_EXT_TABLE_STR	PASTE(LAL_EXT_TABLE, LAL_EXT_SUFFIX)
#define LAL_EXT_INIT_STR	PASTE(LAL_EXT_INIT, LAL_EXT_SUFFIX)
#define LAL_EXT_FINI_STR	PASTE(LAL_EXT_FINI, LAL_EXT_SUFFIX)

#define PASTE__(a1)             #a1
#define PASTE_(a1, a2)          PASTE__(a1 ## _ ## a2)
#define PASTE(a1, a2)           PASTE_(a1, a2)

/* bookkeeping stuff */

/*
 * _alInitExtensions( void )
 *
 * Initialize data structures neccesary for the registration and management of
 * extension groups and extension functions.  Return AL_TRUE if successful,
 * AL_FALSE otherwise.
 */
ALboolean _alInitExtensions( void );

/*
 * _alDestroyExtensions( void )
 *
 * Deallocs the data structures allocated in _alInitExtensions.
 */
void _alDestroyExtensions( void );

/*
 * _alRegisterExtensionGroup( const ALubyte *extGroup )
 *
 * Registers an extension group.  Before registration, queries of
 * alIsExtensionPresent( extGroup) will return AL_FALSE, after will
 * return AL_TRUE.
 *
 * Returns AL_TRUE if registration was successful, AL_FALSE otherwise.
 */
ALboolean _alRegisterExtensionGroup( const ALubyte *extGroup );

/*
 * _alDestroyExtensionGroups( void )
 *
 * Destroy data structures needed to hold extension group information.
 */
void _alDestroyExtensionGroups( void );

/*
 * _alGetExtensionStrings( ALubyte *buffer, int size )
 *
 * Gets a list of extension groups registered, populating buffer up to size.
 *
 * Returns AL_FALSE if size < 1, AL_TRUE otherwise.
 */
ALboolean _alGetExtensionStrings( ALubyte *buffer, int size );

/*
 * _alRegisterExtension( const ALubyte *name, ALvoid *addr )
 *
 * Adds a function to the available extension registry.  Before registration,
 * queries of alGetProcAddress( name ) will return NULL.  After, they will
 * return addr.
 *
 * Returns AL_FALSE if name has already been registered, AL_TRUE otherwise.
 */
ALboolean _alRegisterExtension( const ALubyte *name, ALvoid *addr );

/*
 * _alLoadDL( const char *fname )
 *
 * dlopens a shared object, gets the extension table from it, and
 * the runs _alRegisterExtension on each extension pair in it.
 *
 * Returns AL_TRUE if fname refers to a valid plugin, AL_FALSE otherwise.
 */
ALboolean _alLoadDL( const char *fname );

/*
 *  Functions that extensions can call to add functionality.
 */

/*
 * lal_addTimeFilter( const char *name, time_filter *addr )
 *
 * add a filter by name, replacing if already there.  Returns
 * AL_TRUE if function was added or replaced, AL_FALSE if that
 * wasn't possible.
 */
ALboolean lal_addTimeFilter( const char *name, time_filter *addr );

/*
 * alLokiTest( void *dummy )
 *
 * For testing purposes
 */
void alLokiTest( void *dummy );

/*
 * look up bindings for symbols.
 *
 * FIXME: should replace with rc_lookup functions.
 */
#define lal_GetGlobalScalar(s,t,r)   _alGetGlobalScalar(s,t,r)
#define lal_GetGlobalVector(s,t,n,r) _alGetGlobalScalar(s,t,n,r)

#endif /* _AL_EXT_H_ */ 
