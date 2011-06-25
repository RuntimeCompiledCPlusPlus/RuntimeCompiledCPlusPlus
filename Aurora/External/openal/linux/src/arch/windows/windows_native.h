/* vi:set ts=8:
 *
 * windows_native.h
 *
 * Native Windows(tm) backend.
 */
#ifndef WINDOWS_NATIVE_H_
#define WINDOWS_NATIVE_H_

#include "AL/alc.h"

/* native windows audio stuff */
void *grab_read_native(void);
void *grab_write_native(void);

ALboolean set_write_native(void *handle,
		     unsigned int *bufsiz,
		     ALenum *fmt,
		     unsigned int *speed);
ALboolean set_read_native(void *handle,
		     unsigned int *bufsiz,
		     ALenum *fmt,
		     unsigned int *speed);

int   grab_mixerfd(void);
void native_blitbuffer(void *handle, void *data, int bytes);
void release_native(void *handle);

float get_nativechannel(void *handle, ALCenum channel);
int set_nativechannel(void *handle, ALCenum channel, float volume);

void pause_nativedevice(void *handle);
void resume_nativedevice(void *handle);

ALsizei capture_nativedevice(void *handle, void *capture_buffer, int bufsiz);
			  
#endif /* WINDOWS_NATIVE_H_ */
