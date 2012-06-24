#ifndef _AL_SOURCE_H_
#define _AL_SOURCE_H_

#define AL_NUM_SOURCE_PARAMS	128

#define ALAPI __declspec(dllexport)
#define ALAPIENTRY __cdecl

#include <al\altypes.h>
//#include "..\include\eax.h"

#ifdef __cplusplus
extern "C" {
#endif

// Flags indicating what Direct Sound parameters need to be updated in the UpdateContext call
#define VOLUME					1
#define FREQUENCY				2
#define POSITION				4
#define VELOCITY				8
#define MODE					16
#define MINDIST					32
#define MAXDIST					64
#define ORIENTATION				128
#define CONEANGLES				256
#define CONEOUTSIDEVOLUME		512
#define LOOPED					1024
#define STATE					2048
#define SDELETE					4096
#define SGENERATESOURCE			8192
#define SQUEUE					16384
#define SUNQUEUE				32768
#define SUPDATEBUFFERS			65536

// Flags indicated what EAX 2.0 Buffer parameters need to be updated in the UpdateContext call
#define SALLPARAMS				1
#define SDIRECT					2
#define SDIRECTHF				4
#define SROOM					8
#define SROOMHF					16
#define SROOMROLLOFFFACTOR		32
#define SOBSTRUCTION			64
#define SOBSTRUCTIONLFRATIO		128
#define SOCCLUSION				256
#define SOCCLUSIONLFRATIO		512
#define SOCCLUSIONROOMRATIO		1024
#define SOUTSIDEVOLUMEHF		2048
#define SAIRABSORPTIONFACTOR	4096
#define SFLAGS					8192
#define SDEFERRED				16384
#define SCOMMITSETTINGS			32768

#define ALSOURCE		1
#define ALLISTENER		2
#define ALBUFFER		3


#define SOURCE3D	0
#define SOURCE2D	1


#define READYTOREMOVE	1

typedef struct ALbufferlistitem
{
	ALuint				buffer;
	ALuint				bufferstate;
	ALuint				flag;
	struct ALbufferlistitem	*next;
} ALbufferlistitem;

typedef struct ALsource_struct
{
	struct 
	{
		union 
		{
			ALfloat fv3[3];
			ALfloat fv6[6];
			ALint   i;
			ALfloat f;
		} data;
		ALboolean valid;
	} param[AL_NUM_SOURCE_PARAMS];
	ALboolean	inuse;
	ALboolean	valid;
	ALboolean	play;
	ALboolean	relative;
	ALboolean	paused;
	ALenum		state;
	ALuint		position;
	ALuint		position_fraction;
	struct ALbufferlistitem	*queue;		// Linked list of buffers in queue
	ALuint		BuffersInQueue;			// Number of buffers in queue
	ALuint		BuffersProcessed;		// Number of buffers already processed (played)
	union
	{
		ALuint		SizeOfBufferDataAddedToQueue;	// Total size of data added to queue by alSourceQueueBuffers
		ALuint		SizeOfBufferDataRemovedFromQueue;	// Total size of data removed from queue by alSourceUnqueueBuffers
	};
	union
	{
		ALuint		NumBuffersAddedToQueue;		// Number of buffers added to queue by alSourceQueueBuffers
		ALuint		NumBuffersRemovedFromQueue;	// Number of buffers removed from queue by alSourceUnqueueBuffers
	};
	ALuint		TotalBufferDataSize;	// Total amount of data contained in the buffers queued for this source
	ALuint		BuffersAddedToDSBuffer;	// Number of buffers whose data has been copied into DS buffer
	ALuint		update1;				// Store changes that need to be made in alUpdateContext
	ALuint		update2;				// Store changes that need to be made in alUpdateContext (EAX Related)
	ALvoid		*uservalue1;
	ALvoid		*uservalue2;
	ALvoid		*uservalue3;
	ALuint		SourceType;				// Stores type of Source (SOURCE3D or SOURCE2D)
	ALuint		BufferPosition;			// Read position in audio data of current buffer
	ALboolean	FinishedQueue;			// Indicates if all the buffer data has been copied to the source
	ALint		DataStillToPlay;		// Amount of audio data still to be played
	ALuint		BytesPlayed;			// Running total of number of bytes played on this source
	ALuint		OldPlayCursor;			// Previous position of Play Cursor
	ALuint		OldWriteCursor;			// Previous position of the Write Cursor
	ALuint		SilenceAdded;			// Number of bytes of silence added to buffer
	struct ALsource_struct *previous;
	struct ALsource_struct *next;
    
//	EAXBUFFERPROPERTIES eaxBP;

} ALsource;


ALAPI ALvoid	ALAPIENTRY alGenSources(ALsizei n,ALuint *sources);
ALAPI ALvoid	ALAPIENTRY alDeleteSources(ALsizei n,ALuint *sources);
ALAPI ALboolean	ALAPIENTRY alIsSource(ALuint source);
ALAPI ALvoid	ALAPIENTRY alSourcef(ALuint source,ALenum pname,ALfloat value);
ALAPI ALvoid	ALAPIENTRY alSourcefv(ALuint source,ALenum pname,ALfloat *values);
ALAPI ALvoid	ALAPIENTRY alSource3f(ALuint source,ALenum pname,ALfloat v1,ALfloat v2,ALfloat v3);
ALAPI ALvoid	ALAPIENTRY alSourcei(ALuint source,ALenum pname,ALint value);
ALAPI ALvoid	ALAPIENTRY alGetSourcef(ALuint source,ALenum pname,ALfloat *value);
ALAPI ALvoid	ALAPIENTRY alGetSourcefv(ALuint source,ALenum pname,ALfloat *values);
ALAPI ALvoid	ALAPIENTRY alGetSourcei(ALuint source,ALenum pname,ALint *value);
ALAPI ALvoid	ALAPIENTRY alSourcePlay(ALuint source);
ALAPI ALvoid	ALAPIENTRY alSourcePlayv(ALsizei n,ALuint *sources); 
ALAPI ALvoid	ALAPIENTRY alSourcePause(ALuint source);
ALAPI ALvoid	ALAPIENTRY alSourcePausev(ALsizei n,ALuint *sources);
ALAPI ALvoid	ALAPIENTRY alSourceStop(ALuint source);
ALAPI ALvoid	ALAPIENTRY alSourceStopv(ALsizei n,ALuint *sources);
ALAPI ALvoid	ALAPIENTRY alSourceRewind(ALuint source);
ALAPI ALvoid	ALAPIENTRY alSourceRewindv(ALsizei n,ALuint *sources);
ALAPI ALvoid	ALAPIENTRY alSourceQueueBuffers( ALuint source, ALsizei n, ALuint* buffers );
ALAPI ALvoid	ALAPIENTRY alSourceUnqueueBuffers( ALuint source, ALsizei n, ALuint* buffers );

#ifdef __cplusplus
}
#endif

#endif