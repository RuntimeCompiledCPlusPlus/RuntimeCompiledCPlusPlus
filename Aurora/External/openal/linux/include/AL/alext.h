#ifndef _LAL_EXT_H_
#define _LAL_EXT_H_

#include "AL/altypes.h"
#include "alexttypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ALAPI
#define ALAPI extern
#endif

#ifndef ALAPIENTRY
#define ALAPIENTRY
#endif

/* loki */

ALAPI ALfloat alcGetAudioChannel_LOKI(ALuint channel);
typedef ALfloat (*PFNALCGETAUDIOCHANNELPROC)(ALuint channel);

ALAPI void alcSetAudioChannel_LOKI(ALuint channel, ALfloat volume);
typedef void (*PFNALCSETAUDIOCHANNELPROC)(ALuint channel, ALfloat volume);

ALAPI void alBombOnError_LOKI(void);
typedef void (*PFNALBOMBONERRORPROC)(void);

ALAPI void alBufferi_LOKI(ALuint bid, ALenum param, ALint value);
typedef void (*PFNALBUFFERIPROC)(ALuint bid, ALenum param, ALint value);

ALAPI void alBufferDataWithCallback_LOKI(ALuint bid,
		int (*Callback)(ALuint, ALuint, ALshort *, ALenum, ALint, ALint));
typedef void (*PFNALBUFFERDATAWITHCALLBACKPROC)(ALuint bid,
		int (*Callback)(ALuint, ALuint, ALshort *, ALenum, ALint, ALint));

ALAPI void alBufferWriteData_LOKI( ALuint   buffer,
                   ALenum   format,
                   ALvoid*  data,
                   ALsizei  size,
                   ALsizei  freq,
                   ALenum   internalFormat );
typedef void (*PFNALBUFFERWRITEDATAPROC)( ALuint   buffer,
                   ALenum   format,
                   ALvoid*  data,
                   ALsizei  size,
                   ALsizei  freq,
                   ALenum   internalFormat );

ALAPI void ALAPIENTRY alGenStreamingBuffers_LOKI( ALsizei n, ALuint *samples );
typedef void (*PFNALGENSTREAMINGBUFFERSPROC)( ALsizei n, ALuint *samples );

ALAPI ALsizei alBufferAppendData_LOKI( ALuint   buffer,
                            ALenum   format,
                            ALvoid*    data,
                            ALsizei  size,
                            ALsizei  freq );

typedef ALsizei (*PFNALBUFFERAPPENDDATAPROC)( ALuint   buffer,
                            ALenum   format,
                            ALvoid*    data,
                            ALsizei  size,
                            ALsizei  freq );

ALAPI ALsizei alBufferAppendWriteData_LOKI( ALuint   buffer,
                            ALenum   format,
                            ALvoid*  data,
                            ALsizei  size,
                            ALsizei  freq,
			    ALenum internalFormat );

typedef ALsizei (*PFNALBUFFERAPPENDWRITEDATAPROC)( ALuint   buffer,
                            ALenum   format,
                            ALvoid*  data,
                            ALsizei  size,
                            ALsizei  freq,
			    ALenum internalFormat );

/* captures */

ALAPI ALboolean alCaptureInit_EXT( ALenum format, ALuint rate, ALsizei bufferSize );
typedef ALboolean (*PFNALCAPTUREINITPROC)( ALenum format, ALuint rate, ALsizei bufferSize );

ALAPI ALboolean alCaptureDestroy_EXT( ALvoid );
typedef ALboolean (*PFNALCAPTUREDESTROYPROC)( ALvoid );

ALAPI ALboolean alCaptureStart_EXT( ALvoid );
typedef ALboolean (*PFNALCAPTURESTARTPROC)( ALvoid );

ALAPI ALboolean alCaptureStop_EXT( ALvoid );
typedef ALboolean (*PFNALCAPTURESTOPPROC)( ALvoid );

/* Non-blocking device read */
ALAPI ALsizei alCaptureGetData_EXT( ALvoid* data, ALsizei n, ALenum format, ALuint rate );
typedef ALsizei (*PFNALCAPTUREGETDATAPROC)( ALvoid* data, ALsizei n, ALenum format, ALuint rate );

/* vorbis */
ALAPI ALboolean alutLoadVorbis_LOKI(ALuint bid, ALvoid *data, ALint size);
typedef  ALboolean (*PFNALUTLOADVORBISPROC)(ALuint bid, ALvoid *data, ALint size);

/* custom loaders */

ALAPI ALboolean ALAPIENTRY alutLoadRAW_ADPCMData_LOKI(ALuint bid,
				ALvoid *data, ALuint size, ALuint freq,
				ALenum format);

typedef ALboolean (*PFNALUTLOADRAW_ADPCMDATAPROC)( ALuint bid,
				ALvoid *data, ALuint size, ALuint freq,
				ALenum format);

ALAPI ALboolean ALAPIENTRY alutLoadIMA_ADPCMData_LOKI(ALuint bid,
				ALvoid *data, ALuint size,
				alIMAADPCM_state_LOKI *ias);

typedef ALboolean (*ALUTLOADIMA_ADPCMDATAPROC)(ALuint bid,
				ALvoid *data, ALuint size,
				alIMAADPCM_state_LOKI *ias);

ALAPI ALboolean ALAPIENTRY alutLoadMS_ADPCMData_LOKI(ALuint bid,
				void *data, int size,
				alMSADPCM_state_LOKI *mss);

typedef ALboolean (*ALUTLOADMS_ADPCMDATAPROC)(ALuint bid,
				void *data, int size,
				alMSADPCM_state_LOKI *mss);

#ifdef __cplusplus
}
#endif

#endif /* _LAL_EXT_H_ */
