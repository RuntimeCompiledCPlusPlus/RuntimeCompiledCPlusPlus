#ifndef TESTLIB_H_
#define TESTLIB_H_

#include "../src/al_siteconfig.h"
#include "AL/altypes.h"
#include "../config.h"

/*
 * function pointer for LOKI extensions
 */
ALfloat	(*talcGetAudioChannel)(ALuint channel);
void	(*talcSetAudioChannel)(ALuint channel, ALfloat volume);

void	(*talMute)(ALvoid);
void	(*talUnMute)(ALvoid);

void	(*talReverbScale)(ALuint sid, ALfloat param);
void	(*talReverbDelay)(ALuint sid, ALfloat param);
void	(*talBombOnError)(void);

void	(*talBufferi)(ALuint bid, ALenum param, ALint value);

ALuint  (*talBufferAppendData)(ALuint bid, ALenum format, ALvoid *data, ALint freq, ALint samples);
ALuint  (*talBufferAppendWriteData)(ALuint bid, ALenum format, ALvoid *data, ALint freq, ALint samples, ALenum internalFormat);

ALboolean (*alCaptureInit) ( ALenum format, ALuint rate, ALsizei bufferSize );
ALboolean (*alCaptureDestroy) ( ALvoid );
ALboolean (*alCaptureStart) ( ALvoid );
ALboolean (*alCaptureStop) ( ALvoid );
ALsizei (*alCaptureGetData) ( ALvoid* data, ALsizei n, ALenum format, ALuint rate );

/* new ones */
void (*talGenStreamingBuffers)(ALsizei n, ALuint *bids );
ALboolean (*talutLoadRAW_ADPCMData)(ALuint bid,
				ALvoid *data, ALuint size, ALuint freq,
				ALenum format);
ALboolean (*talutLoadIMA_ADPCMData)(ALuint bid,
				ALvoid *data, ALuint size, ALuint freq,
				ALenum format);
ALboolean (*talutLoadMS_ADPCMData)(ALuint bid,
				ALvoid *data, ALuint size, ALuint freq,
				ALenum format);

void micro_sleep(unsigned int n);

void fixup_function_pointers(void);

ALboolean SourceIsPlaying(ALuint sid);

#endif /* TESTLIB_H_ */
