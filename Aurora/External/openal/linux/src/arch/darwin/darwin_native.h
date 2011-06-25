/* darwin_native.h
 *
 *
 * Mac OS X backend for OpenAL
 * only PowerPC target on MacOS X is supported (Darwin is not supported alone)
 *
 * file originally created on jan 2000 by
 * guillaume borios (gborios@free.fr) and florent boudet (flobo@iname.com)
 * to help the PineApple project (http://ios.free.fr) run on MOSX
 *
 * Version : Alpha 2
 */
 
#ifndef MOSX_DSP_H_
#define MOSX_DSP_H_

#include <AL/alc.h>
#include <AL/altypes.h>


void *grab_read_native(void);
void *grab_write_native(void);

ALboolean set_write_native(void *handle,
		     unsigned int *bufsiz,
		     unsigned int *fmt,
		     unsigned int *speed);
ALboolean set_read_native(void *handle,
		     unsigned int *bufsiz,
		     unsigned int *fmt,
		     unsigned int *speed);

int   grab_mixerfd(void);
void  native_blitbuffer(void *handle, void *data, int bytes);
void  release_native(void *handle);

float get_nativechannel(void *handle, ALCenum channel);
int   set_nativechannel(void *handle, ALCenum channel, float volume);

void pause_nativedevice(void *handle);
void resume_nativedevice(void *handle);

ALsizei capture_nativedevice(void *handle, void *capture_buffer, int bufsiz);
			  
#endif /* MOSX */
