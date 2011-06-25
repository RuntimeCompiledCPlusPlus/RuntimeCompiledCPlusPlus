/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * alc_context.h
 *
 * Prototypes, defines etc for context aquisition and management.
 */
#ifndef _ALC_CONTEXT_H_
#define _ALC_CONTEXT_H_

#include "AL/altypes.h"

#include "../al_types.h"
#include "../al_attenuation.h"

#include "audioconvert.h"

/*
 * _ALC_DEF_FREQ is the default internal mixing frequency.
 */
#define _ALC_DEF_FREQ        44100

/*
 * _ALC_DEF_BUFSIZ is the default length of chunks mixed and written to the
 * audio device.  Must be a power of two.
 */
#define _ALC_DEF_BUFSIZ       1024

/*
 * Canonical stuff
 */
#define _ALC_CANON_FMT           AL_FORMAT_MONO16
#define _ALC_CANON_SPEED         _ALC_DEF_FREQ

#define _ALC_EXTERNAL_FMT        AL_FORMAT_STEREO16
#define _ALC_EXTERNAL_SPEED      _ALC_DEF_FREQ

/* globally accesible data */

/*
 * _alcCCId holds the context id of the current context.
 */
extern ALuint _alcCCId;

/*
 * canon_max and canon_min are the max/min values for PCM data in our
 * canonical format, respectively.
 */
extern const int canon_max, canon_min;
				      
				      

/*
 * canon_format is the canonical format that we represent data internally as.
 */
extern ALenum canon_format;

/*
 * canon_speed is the sampling rate at which we internally represent data.
 */
extern ALuint canon_speed;  

/*
 * _alcDestroyAll( void )
 *
 * Deallocates the data structures allocated in _alcInitContexts.
 */
void _alcDestroyAll( void );

/*
 * _alcInitContext( ALuint cid )
 *
 * Initialize the context named by cid.
 */
AL_context *_alcInitContext( ALuint cid );

/*
 * _alcGetContext( ALuint cid )
 *
 * Returns pointer to the AL_context named by cid, or NULL if cid is not a
 * valid context name.
 */
AL_context *_alcGetContext( ALuint cid );

/*
 * _alcIsContext( ALuint cid )
 *
 * Returns AL_TRUE if cid names a valid context, AL_FALSE otherwise.
 */
ALboolean _alcIsContext( ALuint cid );

/*
 * _alcSetContext( int *attrlist, ALuint cid, AL_device *dev )
 *
 * Sets context id paramaters according to an attribute list and device.
 */
void _alcSetContext( int *attrlist, ALuint cid, AL_device *dev );

/*
 * _alcGetNewContextId( void )
 *
 * Returns a new id for use as a context name.
 */
ALint _alcGetNewContextId( void );

/*
 * _alcInUse( ALuint cid )
 *
 * Returns AL_TRUE if the context named by cid is in use, AL_FALSE otherwise.
 */
ALboolean _alcInUse( ALuint cid );

/*
 * _alcSetUse( ALuint cid, ALboolean value )
 *
 * Sets the use flag of context with id cid to value.
 */
ALboolean _alcSetUse( ALuint cid, ALboolean value );

/*
 * _alcGetReadBufsiz( ALuint cid )
 *
 * Returns the preferred read buffer size of the context named by cid,
 * in bytes.
 */
ALuint _alcGetReadBufsiz( ALuint cid );

/*
 * _alcGetWriteBufsiz( ALuint cid )
 *
 * Returns the preferred write buffer size of the context named by cid,
 * in bytes.
 */
ALuint _alcGetWriteBufsiz( ALuint cid );

/*
 * _alcGetReadFormat( ALuint cid )
 *
 * Returns the preferred read openal format of the context named by cid.
 */
ALenum _alcGetReadFormat( ALuint cid );

/*
 * _alcGetWriteFormat( ALuint cid )
 *
 * Returns the preferred write openal format of the context named by cid.
 */
ALenum _alcGetWriteFormat( ALuint cid );

/*
 * _alcGetReadSpeed( ALuint cid )
 *
 * Returns the preferred sampling rate of the read device associated with the
 * context named by cid.
 */
ALuint _alcGetReadSpeed( ALuint cid );

/*
 * _alcGetWriteSpeed( ALuint cid )
 *
 * Returns the preferred sampling rate of the write device associated with the
 * context named by cid.
 */
ALuint _alcGetWriteSpeed( ALuint cid );

/*
 * _alcGetListener( ALuint cid )
 *
 * Returns a pointer to the listener associated with context named by cid, or
 * NULL if cid does not name a valid context.
 */
AL_listener *_alcGetListener( ALuint cid );

/*
 * _alcIsContextSuspended( ALuint cid )
 *
 * Returns AL_TRUE if this context is suspended, AL_FALSE otherwise.
 * Suspended contexts do not have their sources updated, or mixed.
 */
ALboolean _alcIsContextSuspended( ALuint cid );

/*
 * _alcGetTimeFilters( ALuint cid )
 *
 * Returns a pointer to the time_filter_set associated with the context named
 * by cid, or NULL if cid does not name a context.
 */
time_filter_set *_alcGetTimeFilters( ALuint cid );

/*
 * FL_alcLockContext( ALuint cid, const char *fn, int ln )
 *
 * Locks the mutex associated with the context named by cid, passing fn and ln
 * to _alLockPrintf for debugging purposes.
 */
void FL_alcLockContext( ALuint cid, const char *fn, int ln );

/*
 * FL_alcUnlockContext( ALuint cid, const char *fn, int ln )
 *
 * Unlocks the mutex associated with the context named by cid, passing fn and ln
 * to _alLockPrintf for debugging purposes.
 */
void FL_alcUnlockContext( ALuint cid, const char *fn, int ln );

/*
 * FL_alcLockAllContexts( const char *fn, int ln )
 *
 * Locks the mutex associated guarding all contexts, passing fn and ln to 
 * _alLockPrintf for debugging purposes.
 */
void FL_alcLockAllContexts( const char *fn, int ln );

/*
 * FL_alcUnlockAllContexts( const char *fn, int ln )
 *
 * Unlocks the mutex associated guarding all contexts, passing fn and ln to 
 * _alLockPrintf for debugging purposes.
 */
void FL_alcUnlockAllContexts( const char *fn, int ln );

/*
 * _alcDeviceWrite( ALuint cid, ALvoid *data, ALuint bytes_to_write )
 *
 * Write bytes_to_write worth of data from data to the device associated with
 * the context named by cid.
 */
void _alcDeviceWrite( ALuint cid, ALvoid *data, ALuint bytes_to_write );

/*
 * Read bytes_to_read worth of data from the device associated with the
 * context named by cid.
 */
ALsizei _alcDeviceRead( ALuint cid, void *data, ALuint bytes_to_read );

/* default context macros */
#define _alcDCGetContext()        _alcGetContext(_alcCCId)
#define _alcDCGetReadBufsiz()     _alcGetReadBufsiz(_alcCCId)
#define _alcDCGetWriteBufsiz()    _alcGetWriteBufsiz(_alcCCId)
#define _alcDCGetTimeFilters()    _alcGetTimeFilters(_alcCCId)
#define _alcDCGetFreqFilters()    _alcGetFreqFilters(_alcCCId)
#define _alcDCGetListener()       _alcGetListener(_alcCCId)
#define _alcDCEnableCapture()     _alcEnableCapture(_alcCCId)
#define _alcDCDisableCapture()    _alcDisableCapture(_alcCCId)

#define _alcDCGetReadSpeed()      _alcGetReadSpeed(_alcCCId)
#define _alcDCGetReadFormat()     _alcGetReadFormat(_alcCCId)
#define _alcDCGetWriteSpeed()     _alcGetWriteSpeed(_alcCCId)
#define _alcDCGetWriteFormat()    _alcGetWriteFormat(_alcCCId)
#define _alcDCDeviceWrite(d,b)    _alcDeviceWrite(_alcCCId, d, b)
#define _alcDCDeviceRead(d,b)     _alcDeviceRead(_alcCCId, d, b)

#define _alcDCLockContext()     FL_alcLockContext(_alcCCId, __FILE__, __LINE__)
#define _alcDCUnlockContext()   FL_alcUnlockContext(_alcCCId,__FILE__, __LINE__)
#define _alcUnlockContext(c)    FL_alcUnlockContext(c, __FILE__, __LINE__)
#define _alcLockContext(c)      FL_alcLockContext(c, __FILE__, __LINE__)
#define _alcUnlockAllContexts() FL_alcUnlockAllContexts(__FILE__, __LINE__)
#define _alcLockAllContexts()   FL_alcLockAllContexts(__FILE__, __LINE__)

#endif /* _ALC_CONTEXT_H_ */

#if 0
/*
 * _alcGetContextByIndex( ALuint cindex )
 *
 * Returns a pointer to the AL_context with index cindex, or NULL if cindex is
 * >= the number of available contexts.
 */
AL_context *_alcGetContextByIndex( ALuint cindex );
#endif
