#ifndef __altypes_h_
#define __altypes_h_

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


#ifdef __cplusplus
extern "C" {
#endif


/** OpenAL boolean type. */
typedef char ALboolean;

/** OpenAL void type */
typedef void ALvoid;

/** OpenAL 8bit signed byte. */
typedef char ALbyte;

/** OpenAL 8bit unsigned byte. */
typedef unsigned char ALubyte;

/** OpenAL 16bit signed short integer type. */
typedef short ALshort;

/** OpenAL 16bit unsigned short integer type. */
typedef unsigned short ALushort;

/** OpenAL 32bit unsigned integer type. */
typedef unsigned int ALuint;

/** OpenAL 32bit signed integer type. */
typedef int ALint;

/** OpenAL 32bit floating point type. */
typedef float ALfloat;

/** OpenAL 64bit double point type. */
typedef double ALdouble;

/** OpenAL 32bit type. */
typedef unsigned int ALsizei;

/** OpenAL 32 bit bitfield */
typedef unsigned int ALbitfield;

/** OpenAL clamped (0 to 1) float */
typedef float ALclampf;

/** OpenAL clamped (0 to 1) double */
typedef double ALclampd;

/** AL_NONE = 0 */
#ifndef AL_NONE
#define AL_NONE 0
#endif

typedef enum {
  /* Bad value. */
  AL_INVALID					=-1,

  /* Boolean False. */
  AL_FALSE                      = 0,

  /** Boolean True. */
  AL_TRUE                       = 1,


  /**
   * Indicate the type of AL_SOURCE.
   * Sources can be spatialized 
   */
  AL_SOURCE_TYPE				=  0x200,

  /** Indicate Source has absolute coordinates. */
  AL_SOURCE_ABSOLUTE			=  0x201,


  /** Indicate Source has listener relative coordinates. */
  AL_SOURCE_RELATIVE			=  0x202,

  /**
   * Indicate Source is ambient (not localized by application).
   * It is up to the driver how it wants to handle this case.
   * Ideally, the (human) listener should be unable to localize
   *  this one. Easy :-).
   */
  AL_SOURCE_AMBIENT				= 0x100B,

  /**
   * Multichannel Sources bypass OpenAl completely, and get
   *  handled by the hardware. No other source type will handle 2 
   *  or more channel sounds, although stereo sound samples 
   *  can get mixed into mono for simplicity.
   */
  AL_SOURCE_MULTICHANNEL		=  0x204,


  /**
   * request regarding the state of a source
   */
  AL_SOURCE_STATE				=  0x205,
  
  /**
   * request the number of buffers in the queue for a source
   */
  AL_BUFFERS_QUEUED             =  0x206,
  
  /**
   * request the number of buffers that have been processed for a source
   */
  AL_BUFFERS_PROCESSED          =  0x207,


  /**
   * Directional source, inner cone angle, in degrees.
   * Range:    [0-180] 
   * Default:  90
   */
  AL_CONE_INNER_ANGLE           = 0x1001,

  /**
   * Directional source, outer cone angle, in degrees.
   * Range:    [0-180] 
   * Default:  90
   */
  AL_CONE_OUTER_ANGLE           = 0x1002,

  /**
   * Specify the pitch to be applied, either at source,
   *  or on mixer results, at listener.
   * Range:   [0.5-2.0]
   * Default: 1.0
   */
  AL_PITCH                      = 0x1003,

  /** 
   * Specify the current location in three dimensional space.
   * OpenAL, like OpenGL, uses a right handed coordinate system,
   *  where in a frontal default view X (thumb) points right, 
   *  Y points up (index finger), and Z points towards the
   *  viewer/camera (middle finger). 
   * To switch from a left handed coordinate system, flip the
   *  sign on the Z coordinate.
   * Listener position is always in the world coordinate system.
   */ 
  AL_POSITION					= 0x1004,
  
  /** Specify the current direction as forward and up vector. */
  AL_DIRECTION   				= 0x1005,
  
  /** Specify the current direction as forward vector. */
  AL_ORIENTATION   				= 0x1005,
  
  /** Specify the current velocity in three dimensional space. */
  AL_VELOCITY					= 0x1006,

  /**
   * Indicate whether source is looping.
   * Type: ALboolean?
   * Range:   [AL_TRUE, AL_FALSE]
   * Default: FALSE.
   */
  AL_LOOPING                    = 0x1007,

  /**
   * Indicate the buffer to provide sound samples. 
   * Type: ALuint.
   * Range: any valid Buffer id.
   */
  AL_BUFFER                     = 0x1008,

  /**
   * Indicate the gain (volume amplification) applied. 
   * Type:   ALfloat.
   * Range:  ]0.0-  ]
   * A value of 1.0 means un-attenuated/unchanged.
   * Each division by 2 equals an attenuation of -6dB.
   * Each multiplicaton with 2 equals an amplification of +6dB.
   * A value of 0.0 is meaningless with respect to a logarithmic
   *  scale; it is interpreted as zero volume - the channel
   *  is effectively disabled.
   */
  AL_GAIN                       = 0x1009,
  
  /**
   * Additional distance-model-related per-source parameters
   */
  AL_MAX_DISTANCE               = 0x100A,
  AL_MIN_GAIN                   = 0x100B,
  AL_MAX_GAIN                   = 0x100C,
  AL_ROLLOFF_FACTOR             = 0x100D,
  AL_REFERENCE_DISTANCE         = 0x100E,


  /** Sound buffers: format specifier. */
  AL_FORMAT_MONO8               = 0x1010,
  AL_FORMAT_MONO16              = 0x1011,
  AL_FORMAT_STEREO8             = 0x1012,
  AL_FORMAT_STEREO16            = 0x1013,
  
  /**
   * Indicate source state
   */
  AL_INITIAL                    = 0x1014,
  AL_PLAYING                    = 0x1015,
  AL_PAUSED                     = 0x1016,
  AL_STOPPED                    = 0x1017,

  /** 
   * Sound buffers: frequency, in units of Hertz [Hz].
   * This is the number of samples per second. Half of the
   *  sample frequency marks the maximum significant
   *  frequency component.
   */
  AL_FREQUENCY                  = 0x2001,


  AL_BITS						= 0x2002,

  
  AL_CHANNELS					= 0x2003,


  AL_SIZE						= 0x2004,


  /** Errors */
  AL_NO_ERROR                   = AL_FALSE,
  AL_INVALID_NAME               = 0xA001,
  AL_INVALID_ENUM               = 0xA002,
  AL_INVALID_VALUE              = 0xA003,
  AL_INVALID_OPERATION          = 0xA004,
  AL_OUT_OF_MEMORY              = 0xA005,
  

  /** Context strings: Vendor Name. */
  AL_VENDOR                     = 0xB001,
  
  AL_VERSION                    = 0xB002,
  AL_RENDERER                   = 0xB003,
  AL_EXTENSIONS                 = 0xB004,

  /**
   * Doppler velocity.  Default 1.0
   */
  AL_DOPPLER_VELOCITY           = 0xC001,
  
  /**
  * Distance model.  Default AL_INVERSE_DISTANCE_CLAMPED
  */
  AL_DISTANCE_MODEL             = 0xC002,

  /** Distance models. */

  AL_INVERSE_DISTANCE           = 0xD000,
  AL_INVERSE_DISTANCE_CLAMPED   = 0xD001

} ALenum;



#ifdef __cplusplus
}  /* extern "C" */
#endif



#endif /* __al_h_ */
