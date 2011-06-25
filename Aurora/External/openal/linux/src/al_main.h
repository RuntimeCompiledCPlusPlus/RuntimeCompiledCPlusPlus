/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_main.h
 *
 * Miscellanous prototypes, macros and definitions
 */
#ifndef _AL_MAIN_H_
#define _AL_MAIN_H_

#include "al_siteconfig.h"
#include "alc/alc_context.h"

#include <AL/altypes.h>

/*
 * If compiling with gcc, then we can use attributes.
 *
 * UNUNSED(x) flags a parameter or variable as (potentially) being unused, so
 * that gcc doesn't complain about it with -Wunused.
 */
#ifdef __GNUC__
    #ifndef DARWIN_TARGET /* darwin os uses a cc based on gcc and have __GNUC__ defined */
    #define UNUSED(x) x __attribute((unused))
    #else 
    #define UNUSED(x) x 
    #endif /* DARWIN_TARGET */
#else
#define UNUSED(x) x 
#endif /* GNU_C_ */

/*
 * _alLockPrintf( const char *str, const char *fn, int line );
 *
 * _alLockPrintf is used for debugging purposes.  If DEBUG_LOCK is defined,
 * calls to _alLockPrintf generate a print to stderr.  If not, these calls are
 * optimized away.
 */
#ifndef DEBUG_LOCK
#define _alLockPrintf(x, f, l)
#else
int _alLockPrintf( const char *str, const char *fn, int line );
#endif /* DEBUG_LOCK */

/*
 * _al_ACformatbits(fmt)
 *
 * evaluates to the depth of the passed audioconvert format, in bits.
 */
#define _al_ACformatbits(fmt) (fmt & 0x00FF)

/*
 * _al_ALCHANNELS(fmt)
 *
 * evaluates to the number of channels in an openal format.
 */
#define _al_ALCHANNELS(fmt) ((fmt==AL_FORMAT_MONO16)? 1 : ((fmt==AL_FORMAT_MONO8)?1:2))

/*
 * ustrcmp(s1, s2)
 * ustrncmp(s1, s2, n)
 * ustrncpy(s1, s2, n)
 *
 * These macros are make calls to the associated regular-library str calls,
 * but cast their arguments to const char * before doing so.  This is to aid
 * the manipulate of const ALubyte * strings.
 */
#define ustrcmp(s1, s2)     strcmp((const char *) s1, (const char *) s2)
#define ustrncmp(s1, s2, n) strncmp((const char *) s1, \
				    (const char *) s2, \
				    n)
#define ustrncpy(s1, s2, n) strncpy((char *) s1, \
				    (const char *) s2, \
				    n)

/*
 * offset_memcpy( dest, offset, source, length )
 *
 * Like memcpy, but copies to dst + offset instead of to dst.
 */
#define offset_memcpy(d,o,s,l) memcpy(((char *)d) + o, s, l)

/*
 * memcpy_offset( dest, source, offset, length )
 *
 * Like memcpy, but copies from src + offset instead of from src.
 */
#define memcpy_offset(d,s,o,l) memcpy(d, (char *) s + o, l)

/*
 * _alStub( const char *str )
 *
 * If DEBUG_STUB is defined, _alStub prints out a warning message.  If not, no
 * action is taken.
 *
 */
#ifdef DEBUG_STUB
void _alStub( const char *str );
#else
#define _alStub( s )
#endif /* DEBUG_STUB */

/*
 * _alInit( void )
 *
 * Does misc. initialization for the library.
 */
ALboolean _alInit( void );

/*
 * _alExit( void )
 *
 * Deallocates the data structures created by _alInit.
 */
void _alExit( void );

/*
 * _al_formatbits( ALenum format )
 *
 * Returns bit depth of format.
 */
ALbyte _al_formatbits( ALenum format );

/*
 * _al_AL2FMT( ALuint channels, ALuint bits )
 *
 * Returns the openal format that has the number of channels channels and the
 * bit depth bits.
 */
ALenum _al_AL2FMT( ALuint channels, ALuint bits );

/*
 * _al_AC2ALFMT( ALuint acformat, ALuint channels )
 *
 * Returns the openal format equivilant to the audioconvert format acformat,
 * with the number of channels specified by channels.
 */
ALenum _al_AC2ALFMT( ALuint acformat, ALuint channels );

/*
 * _al_AL2ACFMT( ALenum alfmt )
 *
 * Returns the equivilant (sort of) audioconvert format specified by alfmt.
 * audioconvert formats do not have channel information, so this should be
 * combined with _al_ALCHANNELS.
 */
ALushort _al_AL2ACFMT( ALenum alfmt );

/*
 * _al_formatscale( ALenum format, ALuint new_channel_num )
 *
 * Returns the openal format that is identical to format, but with sufficient
 * channel width to accomedate new_channel_num channels.
 */
ALenum _al_formatscale( ALenum format, ALuint new_channel_num );

/*
 * _al_PCMRatioify( ALuint ffreq, ALuint tfreq,
 *                  ALenum ffmt, ALenum tfmt,
 *                  ALuint samples )
 *
 * Returns the number of byte necessary to contain samples worth of data, if
 * the data undergoes a conversion from ffreq to tfreq in the sampling-rate
 * and from ffmt to tfmt in terms of format.
 */
ALuint _al_PCMRatioify( ALuint ffreq, ALuint tfreq,
			ALenum ffmt, ALenum tfmt,
			ALuint samples );

/*
 *  _al_RAWFORMAT( ALenum format )
 *
 *  Returns AL_TRUE if format is an openal format specifying raw pcm data,
 *  AL_FALSE otherwise.
 */
ALboolean _al_RAWFORMAT( ALenum format );

/*
 * _alFloatMul( ALshort *bpt, ALfloat sa, ALuint len )
 *
 * Multiplies each ALshort in bpt (len bytes long) by sa, clamped above
 * by 32767 and below by -32768.  Only appropriate for 0.0 <= sa <= 1.0.
 */
void _alFloatMul( ALshort *bpt, ALfloat sa, ALuint len );

/*
 * _alMatrixAlloc( int rows, int cols )
 *
 * Allocates, initializes, and returns a matrix with the dimensions matching
 * the passed arguments, or NULL on error.
 */
ALmatrix *_alMatrixAlloc( int rows, int cols );

/*
 * _alMatrixFree( ALmatrix *m )
 *
 * Frees a matrix allocated with _alMatrixAlloc.
 */
void _alMatrixFree( ALmatrix *m );

/*
 * _alMatrixMul( ALmatrix *result, ALmatrix *m1, ALmatrix *m2 )
 *
 * Multiplies two matrices and places the result in result.
 */
void _alMatrixMul( ALmatrix *result, ALmatrix *m1, ALmatrix *m2 );

/*
 * _alStartMixerThread( void )
 *
 * Starts the mixer thread.
 */
void _alStartMixerThread( void );

/*
 * _alWaitForMixerThreadToDie( void )
 *
 * Waits for the mixer thread to die.  This call does not kill the mixer
 * thread, it just hangs around poking the body.
 */
void _alWaitForMixerThreadToDie( void );

/*
 * _alBuffersCopy( void **dsts, void **srcs, int len, int nc )
 *
 * Copies srcs[0..nc-1][0..(len/2)-1] to dsts[0..nc-1][0..(len/2)-1].
 */
void _alBuffersCopy( void **dsts, void **srcs, int len, int nc );

/*
 * _alBuffersAppend( void **dsts, void **srcs, int len, int offset, int nc )
 *
 * Copies srcs[0..nc-1][0..(len/2)-1] to
 * dsts[0..nc-1][offset/2..((offset + len)/2)-1].
 */
void _alBuffersAppend( void **dsts, void **srcs, int len, int offset, int nc );

/*
 * _alSlurp( const char *fname, void **buffer )
 *
 * slurp file named by fname to into *buffer, mallocing memory.
 */
int _alSlurp( const char *fname, void **buffer );

/*
 * _alDBToLinear( ALfloat dBs )
 *
 * Convert a logarithmic gain to a linear one.
 */
ALfloat _alDBToLinear(ALfloat dBs);

/*
 * _alLinearToDB( ALfloat linear )
 *
 * Convert a linear gain to a logarithmic one.
 */
ALfloat _alLinearToDB( ALfloat linear );

/*
 * _alMicroSleep( unsigned int n)
 *
 * sleep for n microseconds
 */
void _alMicroSleep( unsigned int n );

/*
 * _alDegreeToRadian( ALfloat degree )
 *
 * Convert degree argument to radians
 */
ALfloat _alDegreeToRadian( ALfloat degree );

/*
 * Functions for verifying values fall between min and max,
 * inclusive.
 */

/*
 * _alCheckRangef( ALfloat val, ALfloat min, ALfloat max )
 *
 * Returns AL_TRUE if val is between min and max, inclusive.
 */
ALboolean _alCheckRangef( ALfloat val, ALfloat min, ALfloat max );

/*
 * _alCheckRangeb( ALfloat val )
 *
 * Returns AL_TRUE if val is either AL_TRUE or AL_FALSE.
 */
ALboolean _alCheckRangeb( ALboolean val );

/*
 * _alIsZeroVector( ALfloat *fv1 )
 *
 * Returns true if fv1 == { 0.0f, 0.0f, 0.0f }
 */
ALboolean _alIsZeroVector( ALfloat *fv1 );

/*
 * the buffers that sources are split into in SplitSources and
 * Collapsed from in CollapseSources.  Filters work on these
 * intermediate buffers, each of which contains one mono channel of
 * the source data.
 *
 * f_buffers contain PCM data
 */

extern _alDecodeScratch f_buffers;

/*
 * _alSmallestPowerOfTwo( ALuint num )
 *
 * Returns smallest power of two large that meets or exceeds num.
 */
ALuint _alSmallestPowerOfTwo( ALuint num );

/*
 * _alIsFinite( ALfloat v )
 *
 * Returns AL_TRUE if v is a finite, non NaN value, AL_FALSE otherwise.
 */
ALboolean _alIsFinite( ALfloat v );

#if 0
/*
 * math headers don't react well to -ansi
 */
#ifndef M_PI
#define M_PI		3.14159265358979323846	/* pi */
#endif

#ifndef M_PI_2
#define M_PI_2          (M_PI / 2.0f)
#endif

#ifndef M_PI_4
#define M_PI_4          (M_PI / 4.0f)
#endif 

extern double sqrt( double x );
extern double sin ( double a );
extern double cos ( double a );
extern double acos( double a );
extern double fabs( double v );
extern int   isinf( double v );
extern int   isnan( double v );
#endif

#endif
