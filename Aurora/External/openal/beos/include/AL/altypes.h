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
typedef unsigned ALuint;

/** OpenAL 32bit signed integer type. */
typedef int ALint;

/** OpenAL 32bit floating point type. */
typedef float ALfloat;

/** OpenAL 64bit double point type. */
typedef double ALdouble;

/** OpenAL 32bit type. */
typedef unsigned int ALsizei;


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
  AL_SOURCE_AMBIENT				= 0x100D,

  /**
   * Multichannel Sources bypass OpenAl completely, and get
   *  handled by the hardware. No other source type will handle 2 
   *  or more channel sounds, although stereo sound samples 
   *  can get mixed into mono for simplicity.
   */
  AL_SOURCE_MULTICHANNEL		=  0x204,


  /**
   * Aureal's Area source, oh my....
   */
  AL_SOURCE_POINT				=  0x205,


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
   * Directional source, outer cone gain.
   * Range:	   ]0.0-  ]
   * Default:  0.0
   */
  AL_CONE_OUTER_GAIN            = 0x1003,

  /**
   * Specify the pitch to be applied, either at source,
   *  or on mixer results, at listener.
   * Range:   [0.5-2.0]
   * Default: 1.0
   */
  AL_PITCH                      = 0x1004,

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
  AL_POSITION					= 0x1005,
  
  /** Specify the current orientation as forward and up vector. */
  AL_ORIENTATION				= 0x1006,
  
  /** Specify the current direction as forward vector. */
  AL_DIRECTION					= 0x1006,
  
  /** Specify the current velocity in three dimensional space. */
  AL_VELOCITY					= 0x1007,

  /**
   * Indicate whether source is looping.
   * Type: ALboolean?
   * Range:   [AL_TRUE, AL_FALSE]
   * Default: FALSE.
   */
  AL_LOOPING					= 0x1008,

  /**
   * Indicate whether source is meant to be streaming.
   * Type: ALboolean?
   * Range:   [AL_TRUE, AL_FALSE]
   * Default: FALSE.
   */
  AL_STREAMING					= 0x1009,

  /**
   * Indicate the buffer to provide sound samples. 
   * Type: ALuint.
   * Range: any valid Buffer id.
   */
  AL_BUFFER                     = 0x100A,

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
  AL_GAIN                       = 0x100B,

  /**
   * Indicate the environment to apply the source/listener. 
   * Type: ALuint.
   * Range: any valid Environment id.
   */
  AL_ENVIRONMENT_IASIG          = 0x100C,

  /** Sound buffers: format specifier. */
  AL_FORMAT_MONO8               = 0x1100,
  AL_FORMAT_MONO16              = 0x1101,
  AL_FORMAT_STEREO8             = 0x1102,
  AL_FORMAT_STEREO16            = 0x1103,

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


  /** Errors: No Error. */
  AL_NO_ERROR                   = AL_FALSE,


  /** 
   * Illegal value passed as an argument to an AL call.
   * Applies to parameter values, but not to enumerations.
   */
  AL_ILLEGAL_VALUE              = 0xA001,
  
  /**
   * A function was called at inappropriate time,
   *  or in an inappropriate way, causing an illegal state.
   * This can be alVertex3f outside alBegin(..),
   *  or using an incompatible ALenum, object ID,
   *  and/or function.
   */
  AL_ILLEGAL_OPERATION          = 0xA002,

  /**
   * A function could not be completed,
   * because there is not enough memory available.
   */
  AL_OUT_OF_MEMORY				= 0xA003,

  /** Context strings: Vendor Name. */
  AL_VENDOR                     = 0xB001,
  
  AL_VERSION                    = 0xB002,
  AL_RENDERER                   = 0xB003,
  AL_EXTENSIONS                 = 0xB004,

  /** ARB approved MP3 extension. */

  AL_FORMAT_MP3_ARB             = 0xC001,


  /** IASIG Level 2 Environment. */

  /**  
   * Parameter:  IASIG ROOM
   * Type:       float   
   * Range:      [0.0, 1.0]
   * Default:    0.0 
   */
  AL_ENV_ROOM_IASIG                         = 0x3001,

  /**
   * Parameter:  IASIG ROOM_HIGH_FREQUENCY
   * Type:       float
   * Range:      [0.0, 1.0]
   * Default:    1.0 
   */
  AL_ENV_ROOM_HIGH_FREQUENCY_IASIG          = 0x3002,


  /** 
   * Parameter:  IASIG  DECAY_TIME
   * Type:       float
   * Range:      [0.1, 20.0]
   * Default:    1.0 
   */
  AL_ENV_DECAY_TIME_IASIG                   = 0x3003,


  /**
   * Parameter:  IASIG DECAY_HIGH_FREQUENCY_RATIO
   * Type:       float
   * Range:      [0.1, 2.0]
   * Default:    0.5
   */
  AL_ENV_DECAY_HIGH_FREQUENCY_RATIO_IASIG   = 0x3004,


  /**
   * Parameter:  IASIG REFLECTIONS
   * Type:       float  
   * Range:      [0.0, 3.0]
   * Default:    0.0
   */
  AL_ENV_REFLECTIONS_IASIG                  = 0x3005,


  /**
   * Parameter:  IASIG REFLECTIONS_DELAY
   * Type:       float
   * Range:      [0.0, 0.3]
   * Default:    0.02
   */
  AL_ENV_REFLECTIONS_DELAY_IASIG            = 0x3006,


  /**
   * Parameter:  IASIG REVERB
   * Type:       float
   * Range:      [0.0,10.0]
   * Default:    0.0   
   */
  AL_ENV_REVERB_IASIG                       = 0x3007,


  /**
   * Parameter:  IASIG REVERB_DELAY
   * Type:       float
   * Range:      [0.0, 0.1]
   * Default:    0.04
   */
  AL_ENV_REVERB_DELAY_IASIG                 = 0x3008,


  /**
   * Parameter:  IASIG DIFFUSION
   * Type:       float
   * Range:      [0.0, 100.0]
   * Default:    100.0
   */
  AL_ENV_DIFFUSION_IASIG                    = 0x3009,


  /**
   * Parameter:  IASIG DENSITY
   * Type:       float
   * Range:      [0.0, 100.0]
   * Default:    100.0
   */
  AL_ENV_DENSITY_IASIG                      = 0x300A,
  
  
  /**
   * Parameter:  IASIG HIGH_FREQUENCY_REFERENCE
   * Type:       float
   * Range:      [20.0, 20000.0]
   * Default:    5000.0
   */
  AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG     = 0x300B

} ALenum;


#ifdef __cplusplus
}  /* extern "C" */
#endif


#endif /* __altypes_h_ */
