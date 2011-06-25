#ifndef ALSA_H_
#define ALSA_H_

#include "AL/altypes.h"

void *grab_read_alsa( void );
void *grab_write_alsa( void );

void *release_alsa( void *handle );

void alsa_blitbuffer( void *handle, void *data, int bytes );
ALboolean set_read_alsa( void *handle,
			 ALuint *bufsiz,
			 ALenum *fmt,
			 ALuint *speed);
ALboolean set_write_alsa( void *handle,
			  ALuint *bufsiz,
			  ALenum *fmt,
			  ALuint *speed);

#endif /* ALSA_H_ */
