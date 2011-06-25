/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * platform.h
 *
 * Defines include for platform specific backends.  Support for backends is
 * determined at configure time, when defines are set in such a way that the
 * specific backend headers are included.
 */
#ifndef PLATFORM_H_
#define PLATFORM_H_

/* native audio support */
#if LINUX_TARGET == 1
#include "arch/linux/lin_dsp.h"
#elif BSD_TARGET == 1
#include "arch/bsd/bsd_dsp.h"
#elif SOLARIS_TARGET == 1
#include "arch/solaris/solaris_native.h"
#elif IRIX_TARGET == 1
#include "arch/irix/iris.h"
#elif DARWIN_TARGET == 1
#include "arch/darwin/darwin_native.h"
#elif WINDOWS_TARGET == 1
#include "arch/windows/windows_native.h"
#endif /* LINUX_TARGET == 1 */

/* Here's the hacky stuff */
#ifdef SDL_SUPPORT
#include "arch/sdl/sdl.h"
#else 
#define grab_read_sdl()               NULL
#define grab_write_sdl()              NULL
#define set_write_sdl(h,b,f,s)        AL_FALSE
#define set_read_sdl(h,b,f,s)         AL_FALSE
#define release_sdl(h)    
#define sdl_blitbuffer(h,d,b) 
#endif /* SDL_SUPPORT */

#ifdef ALSA_SUPPORT
#include "arch/alsa/alsa.h"
#else
#define grab_read_alsa()	      NULL
#define grab_write_alsa()	      NULL
#define set_write_alsa(h,b,f,s)       AL_FALSE
#define set_read_alsa(h,b,f,s)        AL_FALSE
#define release_alsa(h)
#define alsa_blitbuffer(h,d,b)
#endif /* ALSA_SUPPORT */

#ifdef ARTS_SUPPORT
#include "arch/arts/arts.h"
#else
#define grab_read_arts()	      NULL
#define grab_write_arts()	      NULL
#define set_write_arts(h,b,f,s)       AL_FALSE
#define set_read_arts(h,b,f,s)        AL_FALSE
#define release_arts(h)
#define arts_blitbuffer(h,d,b)
#endif /* ARTS_SUPPORT */

#ifdef ESD_SUPPORT
#include "arch/esd/esd.h"
#else
#define grab_read_esd()	              NULL
#define grab_write_esd()	      NULL
#define set_read_esd(h,b,f,s)         AL_FALSE
#define set_write_esd(h,b,f,s)        AL_FALSE
#define release_esd(h)
#define esd_blitbuffer(h,d,b)
#define pause_esd(h)
#define resume_esd(h)
#endif /* ESD_SUPPORT */

#ifdef WAVEOUT_SUPPORT
#include "arch/waveout/waveout.h"
#else
#define grab_read_waveout()	      NULL
#define grab_write_waveout()	      NULL
#define set_read_waveout(h,b,f,s)     AL_FALSE
#define set_write_waveout(h,b,f,s)    AL_FALSE
#define release_waveout(h)
#define waveout_blitbuffer(h,d,b)
#endif /* WAVEOUT_SUPPORT */

#ifdef NULL_SUPPORT
#include "arch/null/null.h"
#else
#define grab_read_null()	      NULL
#define grab_write_null()	      NULL
#define set_read_null(h,b,f,s)        AL_FALSE
#define set_write_null(h,b,f,s)       AL_FALSE
#define release_null(h)
#define null_blitbuffer(h,d,b)
#endif /* NULL_SUPPORT */

#ifdef EMU10K1_SUPPORT
#include "arch/emu10k1/emu10k1.h"
#else
#define grab_read_emu10k1()	      NULL
#define grab_write_emu10k1()	      NULL
#define set_read_emu10k1(h,b,f,s)     AL_FALSE
#define set_write_emu10k1(h,b,f,s)    AL_FALSE
#define release_emu10k1(h)
#define emu10k1_blitbuffer(h,d,b)
#define capture_emu10k1(h,d,b)        0
#endif /* NULL_SUPPORT */

#endif /* PLATFORM_H_ */
