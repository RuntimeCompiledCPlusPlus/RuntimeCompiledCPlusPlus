#ifndef __al_h_
#define __al_h_

/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2000 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */


#include <AL/altypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ALAPI
#define ALAPIENTRY


#ifndef AL_NO_PROTOTYPES

/**
 * OpenAL Maintenance Functions
 * Initialization and exiting.
 * State Management and Query.
 * Error Handling.
 * Extension Support.
 */

ALAPI ALvoid	ALAPIENTRY alInit(ALint *argc,ALubyte **argv);
ALAPI ALvoid	ALAPIENTRY alExit(ALvoid);

/** State management. */
ALAPI void		ALAPIENTRY alEnable( ALenum capability );
ALAPI void		ALAPIENTRY alDisable( ALenum capability ); 
ALAPI ALboolean ALAPIENTRY alIsEnabled( ALenum capability ); 

/** Application preferences for driver performance choices. */
ALAPI void		ALAPIENTRY alHint( ALenum target, ALenum mode );

/** State retrieval. */
ALAPI ALboolean ALAPIENTRY alGetBoolean( ALenum param );
ALAPI ALint		ALAPIENTRY alGetInteger( ALenum param );
ALAPI ALfloat	ALAPIENTRY alGetFloat( ALenum param );
ALAPI ALdouble	ALAPIENTRY alGetDouble( ALenum param );
ALAPI void		ALAPIENTRY alGetBooleanv( ALenum param, ALboolean* data );
ALAPI void		ALAPIENTRY alGetIntegerv( ALenum param, ALint* data );
ALAPI void		ALAPIENTRY alGetFloatv( ALenum param, ALfloat* data );
ALAPI void		ALAPIENTRY alGetDoublev( ALenum param, ALdouble* data );
ALAPI ALAPIENTRY const ALubyte* alGetString( ALenum param );

/**
 * Error support.
 * Obtain the most recent error generated in the AL state machine.
 */
ALAPI ALenum	ALAPIENTRY alGetError( void );


/**
 * Error support.
 * Obtain a constant string that describes the given error token. 
 */
ALAPI ALAPIENTRY const ALubyte* alGetErrorString( ALenum error );


/** 
 * Extension support.
 * Obtain the address of a function (usually an extension)
 *  with the name fname. All addresses are context-independent. 
 */
ALAPI ALAPIENTRY const ALvoid *alGetProcAddress( const ALubyte* fname );


/**
 * Extension support.
 * Obtain the integer value of an enumeration (usually an extension) with the name ename. 
 */
ALAPI ALenum	ALAPIENTRY alGetEnumValue( const ALubyte* ename );




/**
 * LISTENER
 * Listener is the sample position for a given context.
 * The multi-channel (usually stereo) output stream generated
 *  by the mixer is parametrized by this Listener object:
 *  its position and velocity relative to Sources, within
 *  occluder and reflector geometry.
 */



/**
 *
 * Listener Gain:  default 1.0f.
 */
ALAPI void		ALAPIENTRY alListenerf( ALenum param, ALfloat value );


/**  
 *
 * Listener Position.
 * Listener Velocity.
 */
ALAPI void		ALAPIENTRY alListener3f( ALenum param, ALfloat v1, ALfloat v2, ALfloat v3 ); 


/**
 *
 * Listener Position:        ALfloat[3]
 * Listener Velocity:        ALfloat[3]
 * Listener Orientation:     ALfloat[6]  (forward and up vector).
 */
ALAPI void		ALAPIENTRY alListenerfv( ALenum param, ALfloat* values ); 

ALAPI void		ALAPIENTRY alGetListenerf( ALenum param, ALfloat* value );
ALAPI void		ALAPIENTRY alGetListener3f( ALenum param, ALfloat* v1, ALfloat* v2, ALfloat* v3 ); 
ALAPI void		ALAPIENTRY alGetListenerfv( ALenum param, ALfloat* values ); 


/**
 * SOURCE
 * Source objects are by default localized. Sources
 *  take the PCM data provided in the specified Buffer,
 *  apply Source-specific modifications, and then
 *  submit them to be mixed according to spatial 
 *  arrangement etc.
 */



/** Create Source objects. */
ALAPI ALsizei	ALAPIENTRY alGenSources( ALsizei n, ALuint* sources ); 

/** Delete Source objects. */
ALAPI void		ALAPIENTRY alDeleteSources( ALsizei n, ALuint* sources );

/** Verify a handle is a valid Source. */ 
ALAPI ALboolean ALAPIENTRY alIsSource( ALuint id ); 

/** Set an integer parameter for a Source object. */
ALAPI void		ALAPIENTRY alSourcei( ALuint source, ALenum param, ALint value ); 
ALAPI void		ALAPIENTRY alSourcef( ALuint source, ALenum param, ALfloat value ); 
ALAPI void		ALAPIENTRY alSource3f( ALuint source, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3 );
ALAPI void		ALAPIENTRY alSourcefv( ALuint source, ALenum param, ALfloat* values ); 

/** Get an integer parameter for a Source object. */
ALAPI void		ALAPIENTRY alGetSourcei( ALuint source,  ALenum param, ALint* value );
ALAPI void		ALAPIENTRY alGetSourcef( ALuint source,  ALenum param, ALfloat* value );
ALAPI void		ALAPIENTRY alGetSourcefv( ALuint source, ALenum param, ALfloat* values );

/** Activate a source, start replay. */
ALAPI void		ALAPIENTRY alSourcePlay( ALuint source );

/**
 * Pause a source, 
 *  temporarily remove it from the mixer list.
 */
ALAPI void		ALAPIENTRY alSourcePause( ALuint source );

/**
 * Stop a source,
 *  temporarily remove it from the mixer list,
 *  and reset its internal state to pre-Play.
 * To remove a Source completely, it has to be
 *  deleted following Stop, or before Play.
 */
ALAPI void		ALAPIENTRY alSourceStop( ALuint source );




/**
 * BUFFER
 * Buffer objects are storage space for sample data.
 * Buffers are referred to by Sources. There can be more than
 *  one Source using the same Buffer data. If Buffers have
 *  to be duplicated on a per-Source basis, the driver has to
 *  take care of allocation, copying, and deallocation as well
 *  as propagating buffer data changes.
 */




/** Buffer object generation. */
ALAPI ALsizei	ALAPIENTRY alGenBuffers( ALsizei n, ALuint* buffers );
ALAPI void		ALAPIENTRY alDeleteBuffers( ALsizei n, ALuint* buffers );
ALAPI ALboolean ALAPIENTRY alIsBuffer( ALuint buffer );

/**
 * Specify the data to be filled into a buffer.
 */
ALAPI void		ALAPIENTRY alBufferData( ALuint   buffer,
										 ALenum   format,
										 void *   data,
										 ALsizei  size,
										 ALsizei  freq );


/**
 * Specify data to be filled into a looping buffer.
 * This takes the current position at the time of the
 *  call, and returns the number of samples written.
 */
ALAPI ALsizei	ALAPIENTRY alBufferAppendData( ALuint   buffer,
											   ALenum   format,
											   void *   data,
											   ALsizei  size,
											   ALsizei  freq );

ALAPI void		ALAPIENTRY alGetBufferi( ALuint buffer, ALenum param, ALint*   value );
ALAPI void		ALAPIENTRY alGetBufferf( ALuint buffer, ALenum param, ALfloat* value );




/**
 * EXTENSION: IASIG Level 2 Environment.
 * Environment object generation.
 * This is an EXTension that describes the Environment/Reverb
 *  properties according to IASIG Level 2 specifications.
 */




/**
 * Allocate n environment ids and store them in the array environs.
 * Returns the number of environments actually allocated.
 */
ALAPI ALsizei	ALAPIENTRY alGenEnvironmentIASIG( ALsizei n, ALuint* environs );
ALAPI void		ALAPIENTRY alDeleteEnvironmentIASIG( ALsizei n, ALuint* environs );
ALAPI ALboolean ALAPIENTRY alIsEnvironmentIASIG( ALuint environment );
ALAPI void		ALAPIENTRY alEnvironmentiIASIG( ALuint environment, ALenum param, ALint value );
ALAPI void		ALAPIENTRY alEnvironmentfIASIG( ALuint environment, ALenum param, ALfloat value );
ALAPI void		ALAPIENTRY alGetEnvironmentiIASIG( ALuint environment, ALenum param, ALint* value );
ALAPI void		ALAPIENTRY alGetEnvironmentfIASIG( ALuint environment, ALenum param, ALfloat* value );

#else /* AL_NO_PROTOTYPES */


/** OpenAL Maintenance Functions */

ALAPI void		ALAPIENTRY (*alEnable)( ALenum capability );
ALAPI void		ALAPIENTRY (*alDisable)( ALenum capability ); 
ALAPI ALboolean	ALAPIENTRY (*alIsEnabled)( ALenum capability ); 
ALAPI void      ALAPIENTRY (*alHint)( ALenum target, ALenum mode );
ALAPI ALboolean	ALAPIENTRY (*alGetBoolean)( ALenum param );
ALAPI ALint		ALAPIENTRY (*alGetInteger)( ALenum param );
ALAPI ALfloat	ALAPIENTRY (*alGetFloat)( ALenum param );
ALAPI ALdouble	ALAPIENTRY (*alGetDouble)( ALenum param );
ALAPI void		ALAPIENTRY (*alGetBooleanv)( ALenum param, ALboolean* data );
ALAPI void		ALAPIENTRY (*alGetIntegerv)( ALenum param, ALint* data );
ALAPI void		ALAPIENTRY (*alGetFloatv)( ALenum param, ALfloat* data );
ALAPI void		ALAPIENTRY (*alGetDoublev)( ALenum param, ALdouble* data );
ALAPI const ALubyte* ALAPIENTRY (*GetString)( ALenum param );
ALAPI ALenum	ALAPIENTRY (*alGetError)( void );
ALAPI const ALubyte* ALAPIENTRY (*alGetErrorString)( ALenum error );


#endif /* AL_NO_PROTOTYPES */


#ifdef __cplusplus
}  /* extern "C" */
#endif



#endif /* __al_h_ */
