/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * Plugins should include this header.
 *
 */
#ifndef AL_EXT_NEEDED_H_
#define AL_EXT_NEEDED_H_

#include "al_main.h" /* for UNUSED macro */

/*
 * alExtInit_03282000(void)
 *
 * Called by the regular library to initialize plugin data structures.
 */
void alExtInit_03282000( void );

/*
 * alExtFini_03282000(void)
 *
 * Called by the regular library to finalize plugin data structures.
 */
void alExtFini_03282000( void );

/*
 * AL_EXT_PAIR is a macro that expands to a name, address pair
 * for use in the extension table.
 */
#define AL_EXT_PAIR(x) { (const ALubyte *) #x, (void *) x }

#endif /* AL_EXT_NEEDED_H_ */
