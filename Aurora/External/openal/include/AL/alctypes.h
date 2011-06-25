#ifndef _ALCTYPES_H_
#define _ALCTYPES_H_

struct _AL_device;
typedef struct _AL_device ALCdevice;

typedef int ALCenum;

/* Enumerant values begin at column 50. No tabs. */

/* bad value */
#define ALC_INVALID                              0

/**
 * followed by <int> Hz
 */
#define ALC_FREQUENCY                            0x100

/**
 * followed by <int> Hz
 */
#define ALC_REFRESH                              0x101

/**
 * followed by AL_TRUE, AL_FALSE
 */
#define ALC_SYNC                                 0x102

/**
 * errors
 */

/**
 * No error
 */
#define ALC_NO_ERROR                             0

/**
 * No device
 */
#define ALC_INVALID_DEVICE                       0x200

/**
 * invalid context ID
 */
#define ALC_INVALID_CONTEXT                      0x201

/**
 * bad enum
 */
#define ALC_INVALID_ENUM                         0x202

/**
 * bad value
 */
#define ALC_INVALID_VALUE                        0x203

#endif /* _ALCTYPES_H */
