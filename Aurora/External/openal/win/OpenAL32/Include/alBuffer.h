#ifndef _AL_BUFFER_H_
#define _AL_BUFFER_H_

#define ALAPI __declspec(dllexport)
#define ALAPIENTRY __cdecl

#include <al\altypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UNUSED		0
#define PENDING		1
#define PROCESSED	2

typedef struct ALbuffer_struct 
{
	ALenum		format;
	ALshort *	data;
	ALsizei		size;
	ALsizei		frequency;
	ALuint		writepos;
	ALuint		playpos;
	ALenum		state;
	ALuint		refcount;		// Number of sources using this buffer (deletion can only occur when this is 0)
	struct ALbuffer_struct *previous;
	struct ALbuffer_struct *next;
	//Add mutex per buffer !!
} ALbuffer;

ALAPI ALvoid	ALAPIENTRY alGenBuffers(ALsizei n,ALuint *buffers);
ALAPI ALvoid	ALAPIENTRY alGenStreamingBuffers(ALsizei n,ALuint *buffers);
ALAPI ALvoid	ALAPIENTRY alDeleteBuffers(ALsizei n,ALuint *buffers);
ALAPI ALboolean	ALAPIENTRY alIsBuffer(ALuint buffer);
ALAPI ALvoid	ALAPIENTRY alBufferData(ALuint buffer,ALenum format,ALvoid *data,ALsizei size,ALsizei freq);
ALAPI ALvoid	ALAPIENTRY alGetBufferf(ALuint buffer,ALenum pname,ALfloat *value);
ALAPI ALvoid	ALAPIENTRY alGetBufferi(ALuint buffer,ALenum pname,ALint *value);

#ifdef __cplusplus
}
#endif

#endif