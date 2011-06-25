/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_main.c
 *
 * stuff that doesn't fit anywhere else.  Also, global initialization/
 * finitization.
 *
 */
#include "al_siteconfig.h"

#include <AL/al.h>
#include <AL/alc.h>

#include "al_debug.h"
#include "al_types.h"
#include "al_main.h"
#include "al_buffer.h"
#include "al_source.h"
#include "al_mixer.h"
#include "al_ext.h"
#include "al_config.h"
#include "al_vector.h"
#include "alc/alc_context.h"

#include "threads/threadlib.h"

#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "audioconvert.h"

/* standard extensions
 *
 * To avoid having these built in (ie, using the plugin arch), don't
 * include these headers and move the files from EXT_OBJS to EXT_DLL_OBJS.
 */
#include "extensions/al_ext_loki.h"
#include "extensions/al_ext_mp3.h"
#include "extensions/al_ext_vorbis.h"
#include "extensions/al_ext_capture.h"

#ifndef M_PI
#define M_PI		3.14159265358979323846	/* pi */
#endif /* M_PI */

/*
 * mixer thread's ID, if it needs one
 */
extern ThreadID mixthread; 

/*
 * pcm buffers that filters act on
 */
_alDecodeScratch f_buffers;

/*
 * Our extension functions
 */
static AL_extension exts[] = {
	{ (const ALubyte *) "alLokiTest",
	           (void *) alLokiTest },
#ifdef BUILTIN_EXT_LOKI
	BUILTIN_EXT_LOKI,
#endif /* BUILDIN_EXT_LOKI */
#ifdef BUILTIN_EXT_MP3
	BUILTIN_EXT_MP3,
#endif /* BUILDIN_EXT_MP3 */
#ifdef BUILTIN_EXT_VORBIS
	BUILTIN_EXT_VORBIS,
#endif /* BUILDIN_EXT_VORBIS */
#ifdef BUILTIN_EXT_CAPTURE
	BUILTIN_EXT_CAPTURE,
#endif
	{ NULL, NULL }
};

/*
 * _alInit( void )
 *
 * _alInit is called when the "first" context is created.  If all
 * contexts are deleted, and then one is created, it is called again.
 *
 * Returns AL_TRUE unless some weird sort of memory allocation problem occurs,
 * in which case AL_FALSE is returned.
 */
ALboolean _alInit( void ) {
	ALboolean err;
	ALuint i;

	for(i = 0; i < _ALC_MAX_CHANNELS; i++) {
		f_buffers.data[i]   = NULL;
	}

	f_buffers.len = 0;

	/* buffer initializations */
	err = _alInitBuffers();
	if(err == AL_FALSE) {
		return AL_FALSE;
	}

	/* extension initilizations */
	err = _alInitExtensions();
	if(err == AL_FALSE) {
		_alDestroyBuffers();

		return AL_FALSE;
	}

#ifdef BUILTIN_EXT_LOKI
	/* FIXME: dynamic-ify this */
	/* register extension groups */
	_alRegisterExtensionGroup( (const ALubyte*) "ALC_LOKI_audio_channel" );
	_alRegisterExtensionGroup( (const ALubyte*) "AL_LOKI_buffer_data_callback" );
	_alRegisterExtensionGroup( (const ALubyte*) "AL_LOKI_IMA_ADPCM_format" );
	_alRegisterExtensionGroup( (const ALubyte*) "AL_LOKI_play_position" );

#ifdef VORBIS_SUPPORT
	_alRegisterExtensionGroup( (const ALubyte*) "AL_EXT_vorbis" );
#endif /* CAPTURE_SUPPORT */

#ifdef CAPTURE_SUPPORT
	_alRegisterExtensionGroup( (const ALubyte*) "AL_EXT_capture" );
#endif /* CAPTURE_SUPPORT */

#endif /* BUILTIN_EXT_LOKI */

	for(i = 0; exts[i].addr != NULL; i++) {
		_alRegisterExtension(exts[i].name, exts[i].addr);
	}

	/* do builtin extensions initialization */
	BUILTIN_EXT_LOKI_INIT;
	BUILTIN_EXT_CAPTURE_INIT;
	BUILTIN_EXT_MP3_INIT;
	BUILTIN_EXT_VORBIS_INIT;

	return AL_TRUE;
}

/*
 * _alExit( void )
 *
 * Finalizes things when the last context is deleted.
 *
 * FIXME: we can probably clean a lot of this up now that we have
 * alc{Open,Close}Device.
 */
void _alExit( void ) {
	int i;

#ifndef NO_THREADS
	/* we could be sync, so we check mixthread for a valid ID */
	if(mixthread != NULL) {
		time_for_mixer_to_die = AL_TRUE;
		
		tlWaitThread( mixthread );

		while( time_for_mixer_to_die == AL_TRUE ) {
			_alMicroSleep(100000);
		}
	}
#endif /* NO_THREADS */

	for(i = 0; i < _ALC_MAX_CHANNELS; i++) {
		if(f_buffers.data[i] != NULL) {
			free( f_buffers.data[i] );
			f_buffers.data[i] = NULL;
		}
	}

	f_buffers.len = 0;

	_alDestroyConfig();

	_alDestroyExtensions();
	_alDestroyExtensionGroups( );
	_alDestroyMixer();
	_alDestroyFilters();

	_alcDestroyAll();

	_alDestroyBuffers(); /* buffers after mixer and alc destroy */

	/* do builtin extensions destruction */
	BUILTIN_EXT_LOKI_FINI;
	BUILTIN_EXT_CAPTURE_FINI;
	BUILTIN_EXT_MP3_FINI;
	BUILTIN_EXT_VORBIS_FINI;

	return;
}

/*
 * _alStub( const char *str )
 *
 * If DEBUG_STUB is defined, _alStub prints out a warning message.  If not, no
 * action is taken.
 *
 */
#ifdef DEBUG_STUB
void _alStub( const char *str ) {
	fprintf(stderr, "%s stub function\n", str);
	
	return;
}
#endif

/*
 * _alLockPrintf( const char *str, const char *fn, int line );
 *
 * _alLockPrintf is used for debugging purposes.  If DEBUG_LOCK is defined,
 * calls to _alLockPrintf generate a print to stderr.  If not, these calls are
 * optimized away.
 */
#ifdef DEBUG_LOCK
int _alLockPrintf( const char *msg, const char *fn, int ln ) {
	static char threadstr[2048];
	char blanks[] = "                             ";
	int maxlen = 18 - (strlen(fn) + log10(ln));

	blanks[maxlen] = '\0';

	sprintf(threadstr, "%s[%u]", blanks, tlSelfThread());

	return _alDebug(ALD_LOCK, fn, ln, "%s %s", threadstr, msg);
}
#endif

/*
 * _al_AC2ALFMT( ALuint acformat, ALuint channels )
 *
 * Returns the openal format equivilant to the audioconvert format acformat,
 * with the number of channels specified by channels.
 */
ALenum _al_AC2ALFMT( ALuint acformat, ALuint channels ) {
	switch( acformat ) {
		case AUDIO_U8:
			if(channels == 2) {
				return AL_FORMAT_STEREO8;
			}
			if(channels == 1) {
				return AL_FORMAT_MONO8;
			}
			break;
		case AUDIO_S16LSB:
		case AUDIO_S16MSB:
			if(channels == 2) {
				return AL_FORMAT_STEREO16;
			}
			if(channels == 1)
				return AL_FORMAT_MONO16;
			break;
	}

#ifdef DEBUG_CONVERT
	fprintf( stderr, "AC2ALFMT: wtf? format = 0x%x\n", acformat );
#endif

	return -1;
}

/*
 * _al_AL2ACFMT( ALenum alfmt )
 *
 * Returns the equivilant (sort of) audioconvert format specified by alfmt.
 * audioconvert formats do not have channel information, so this should be
 * combined with _al_ALCHANNELS.
 */
ALushort _al_AL2ACFMT( ALenum alformat ) {
	switch( alformat ) {
		case AL_FORMAT_STEREO8:
		case AL_FORMAT_MONO8:
			return AUDIO_U8;
		case AL_FORMAT_STEREO16:
		case AL_FORMAT_MONO16:
			return AUDIO_S16;
		default:
			break;
	}

#ifdef DEBUG_CONVERT
	fprintf(stderr, "AL2ACFMT: wtf? format = 0x%x\n", alformat);
#endif

	return -1;
}

/*
 * _al_formatscale( ALenum format, ALuint new_channel_num )
 *
 * Returns the openal format that is identical to format, but with sufficient
 * channel width to accomedate new_channel_num channels.
 */
ALenum _al_formatscale(ALenum format, ALuint new_channel_num) {
	int fmt_bits = _al_formatbits(format);

	switch(new_channel_num) {
		case 1:
		  switch(fmt_bits) {
			  case 8: return AL_FORMAT_MONO8; break;
			  case 16: return AL_FORMAT_MONO16; break;
			  default: return -1;
		  }
		  break;
		case 2:
		  switch(fmt_bits) {
			  case 8: return AL_FORMAT_STEREO8; break;
			  case 16: return AL_FORMAT_STEREO16; break;
			  default: return -1;
		  }
		  break;
		default:
#ifdef DEBUG_CONVERT
		  fprintf(stderr,
		  	"No support for %d channel AL format, sorry\n",
			new_channel_num);
#endif /* DEBUG_CONVERT */
		  break;
	}

	return -1;
}

/*
 * _alBuffersAppend( void **dsts, void **srcs, int len, int offset, int nc )
 *
 * Copies srcs[0..nc-1][0..(len/2)-1] to
 * dsts[0..nc-1][offset/2..((offset + len)/2)-1].
 */
void _alBuffersAppend(void **dsts, void **srcs, int len, int offset, int nc) {
	char *dstp;
	char *srcp;
	int i;
	int k;

	for(i = 0; i < nc; i++) {
		dstp = dsts[i];
		srcp = srcs[i];

		dstp += offset;

		for(k = 0; k < len; k++) {
			dstp[k] = srcp[k];
		}
	}

	return;
}

/*
 * _alBuffersCopy( void **dsts, void **srcs, int len, int nc )
 *
 * Copies srcs[0..nc-1][0..(len/2)-1] to dsts[0..nc-1][0..(len/2)-1].
 */
void _alBuffersCopy(void **dsts, void **srcs, int len, int nc) {
	ALshort *dstp;
	ALshort *srcp;
	int i;

	len /= sizeof(ALshort);

	for(i = 0; i < nc; i++) {
		dstp = dsts[i];
		srcp = srcs[i];

		memcpy(dstp, srcp, len);
	}

	return;
}

/*
 * _alRotatePointAboutAxis( ALfloat angle, ALfloat *point, ALfloat *axis )
 *
 * Rotates point angle radians about axis.
 *
 * angle  - in radians
 * point  - x/y/z
 * axis   - x/y/z (unit vector)
 *
 * FIXME: check my math
 * FIXME: needs to check args
 */
void _alRotatePointAboutAxis( ALfloat angle, ALfloat *point, ALfloat *axis ) {
	ALmatrix *m;
	ALmatrix *pm;
	ALmatrix *rm;

	float s;
	float c;
	float t;

	float x = axis[0];
	float y = axis[1];
	float z = axis[2];
	int i;

	if(angle == 0.0) {
		/* FIXME: use epsilon? */
		return;
	}

	s = sin( angle );
	c = cos( angle );
	t = 1.0 - c;

	m  = _alMatrixAlloc(3, 3);
	pm = _alMatrixAlloc(1, 3);
	rm = _alMatrixAlloc(1, 3);

#if 1
	m->data[0][0] = t * x * x + c;
	m->data[0][1] = t * x * y - s * z;
	m->data[0][2] = t * x * z + s * y;

	m->data[1][0] = t * x * y + s * z;
	m->data[1][1] = t * y * y + c;
	m->data[1][2] = t * y * z - s * x;

	m->data[2][0] = t * x * z - s * y;
	m->data[2][1] = t * y * z + s * x;
	m->data[2][2] = t * z * z + c;
#else
	m->data[0][0] = t * x * x + c;
	m->data[1][0] = t * x * y - s * z;
	m->data[2][0] = t * x * z + s * y;

	m->data[0][1] = t * x * y + s * z;
	m->data[1][1] = t * y * y + c;
	m->data[2][1] = t * y * z - s * x;

	m->data[0][2] = t * x * z - s * y;
	m->data[1][2] = t * y * z + s * x;
	m->data[2][2] = t * z * z + c;
#endif

	for(i = 0; i < 3; i++) {
		pm->data[0][i] = point[i];
		rm->data[0][i] = 0;
	}

	/*
	 * rm = pm * m
	 */
	_alMatrixMul(rm, pm, m);

	for(i = 0; i < 3; i++) {
		point[i] = rm->data[0][i];
	}
	
	_alMatrixFree(m);
	_alMatrixFree(pm);
	_alMatrixFree(rm);

	return;
}

/*
 * _alMatrixMul( ALmatrix *result, ALmatrix *m1, ALmatrix *m2 )
 *
 * Multiplies m1 by m2, populating result.
 *
 *  FIXME: please check my math
 */
void _alMatrixMul( ALmatrix *result, ALmatrix *m1, ALmatrix *m2 ) {
	int m2cols = m2->cols;
	int m1rows = m1->rows;
	int m1cols = m1->cols;
	int i;
	int j;
	int k;

	ALfloat sum;

	for(i = 0; i < m2cols; i++) {
		for(j = 0; j < m1rows; j++) {
			sum = 0.0f;

			for(k = 0; k < m1cols; k++) {
				sum += m1->data[j][k] * m2->data[k][i];
			}

			result->data[j][i] = sum;
		}
	}

	return;
}

/*
 * _alMatrixAlloc( int rows, int cols )
 *
 * Allocates, initializes, and returns a matrix with the dimensions matching
 * the passed arguments, or NULL on error.
 */
ALmatrix *_alMatrixAlloc(int rows, int cols) {
	ALmatrix *retval;
	int i;

	retval = malloc(sizeof *retval);
	if(retval == NULL) {
		return NULL;
	}

	retval->data = malloc(rows * sizeof *retval->data);
	if(retval->data == NULL) {
		return NULL;
	}

	for(i = 0; i < rows; i++) {
		/* FIXME: clean return on error */
		retval->data[i] = malloc(cols * sizeof *retval->data[i]);
	}

	retval->rows = rows;
	retval->cols = cols;

	return retval;
}

/*
 * _alMatrixFree( ALmatrix *m )
 *
 * Frees a matrix allocated with _alMatrixAlloc.
 */
void _alMatrixFree(ALmatrix *m) {
	int i;

	if(m == NULL) {
		return;
	}

	for(i = 0; i < m->rows; i++) {
		free( m->data[i] );
	}

	free(m->data);
	free(m);

	return;
}

/*
 * _alSlurp( const char *fname, void **buffer )
 *
 * slurp file named by fname to into *buffer, mallocing memory.
 */
int _alSlurp(const char *fname, void **buffer) {
	struct stat buf;	
	FILE *fh;
	ALint len;

	if((fname == NULL) || (buffer == NULL)) {
		return -1;
	}

	if(stat(fname, &buf) == -1) {
		/* couldn't stat file */
		return -1;
	}

	len = (ALint) buf.st_size;
	if(len <= 0) {
		return -1;
	}

	fh = fopen(fname, "rb");
	if(fh == NULL) {
		/* couldn't open file */
		return -1;
	}

	*buffer = malloc(len);
	if(*buffer == NULL) {
		return -1;
	}

	if(fread(*buffer, len, 1, fh) < 1) {
		free(*buffer);

		return -1;
	}

	fclose( fh );

	return len;
}

/*
 * _al_PCMRatioify( ALuint ffreq, ALuint tfreq,
 *                  ALenum ffmt, ALenum tfmt,
 *                  ALuint samples )
 *
 * Returns the number of byte necessary to contain samples worth of data, if
 * the data undergoes a conversion from ffreq to tfreq in the sampling-rate
 * and from ffmt to tfmt in terms of format.
 */
ALuint _al_PCMRatioify( ALuint ffreq, ALuint tfreq,
			ALenum ffmt, ALenum tfmt,
			ALuint samples ) {
	samples *= ((float) tfreq / ffreq );

	samples *= (_al_formatbits( ffmt ) / 8 );
	samples /= (_al_formatbits( tfmt ) / 8 );

	return samples;
}

/*
 * _al_AL2FMT( ALuint channels, ALuint bits )
 *
 * Returns the openal format that has the number of channels channels and the
 * bit depth bits.
 */
ALenum _al_AL2FMT(ALuint channels, ALuint bits) {
	switch(channels) {
		case 1:
			if(bits == 8) return AL_FORMAT_MONO8;
			if(bits == 16) return AL_FORMAT_MONO16;
			break;
		case 2:
			if(bits == 16) return AL_FORMAT_STEREO8;
			if(bits == 16) return AL_FORMAT_STEREO16;
			break;
	}

	return -1;
}

#ifdef _WIN32
/* sleep for n microseconds
 *
 * Well, not really.  For Windows, we divide
 * by 10 and sleep for milliseconds
 */
void _alMicroSleep(unsigned int n) {
	Sleep(n / 1000);

	return;
}

#else

/* sleep for n microseconds */
void _alMicroSleep(unsigned int n) {
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = n;

	select(0, NULL, NULL, NULL, &tv);

	return;
}

#endif /* _WIN32 */

/*
 * _alDegreeToRadian( ALfloat degree )
 *
 * Returns radian equilvilant of degree.
 *
 */
ALfloat _alDegreeToRadian( ALfloat degree ) {
	return degree * (M_PI / 180.0f);
}

/*
 * _alCheckRangef( ALfloat val, ALfloat min, ALfloat max )
 *
 * Returns AL_TRUE if val is between min and max, inclusive.
 */
ALboolean _alCheckRangef(ALfloat val, ALfloat min, ALfloat max) {
	ALboolean retval = AL_TRUE;

#ifdef DEBUG
	if( _alIsFinite(val) == 0 ) {
		retval = AL_FALSE;
	}
#endif

	if(val < min) {
		retval = AL_FALSE;
	}
	if(val > max) {
		retval = AL_FALSE;
	}

	return retval;
}

/*
 * _alCheckRangeb( ALfloat val )
 *
 * Returns AL_TRUE if val is either AL_TRUE or AL_FALSE.
 */
ALboolean _alCheckRangeb(ALboolean b) {
	switch(b) {
		case AL_TRUE:
		case AL_FALSE:
			return AL_TRUE;
		default:
			break;
	}

	return AL_FALSE;
}

/*
 * _alIsZeroVector( ALfloat *fv1 )
 *
 * Returns true if fv1 == { 0.0f, 0.0f, 0.0f }
 */
ALboolean _alIsZeroVector(ALfloat *fv) {
	if(fv[0] != 0.0f) {
		return AL_FALSE;
	}

	if(fv[1] != 0.0f) {
		return AL_FALSE;
	}

	if(fv[2] != 0.0f) {
		return AL_FALSE;
	}

	return AL_TRUE;
}

/*
 * _alLinearToDB( ALfloat linear )
 *
 * Convert a linear gain to a logarithmic one.
 */
ALfloat _alLinearToDB(ALfloat linear) {
	static const float logtab[] = {
		0.00, 0.001, 0.002, 0.003, 0.004, 0.005, 0.01, 0.011,
		0.012, 0.013, 0.014, 0.015, 0.016, 0.02, 0.021, 0.022,
		0.023, 0.024, 0.025, 0.03, 0.031, 0.032, 0.033, 0.034,
		0.04, 0.041, 0.042, 0.043, 0.044, 0.05, 0.051, 0.052,
		0.053, 0.054, 0.06, 0.061, 0.062, 0.063, 0.064, 0.07,
		0.071, 0.072, 0.073, 0.08, 0.081, 0.082, 0.083, 0.084,
		0.09, 0.091, 0.092, 0.093, 0.094, 0.10, 0.101, 0.102,
		0.103, 0.11, 0.111, 0.112, 0.113, 0.12, 0.121, 0.122,
		0.123, 0.124, 0.13, 0.131, 0.132, 0.14, 0.141, 0.142,
		0.143, 0.15, 0.151, 0.152, 0.16, 0.161, 0.162, 0.17,
		0.171, 0.172, 0.18, 0.181, 0.19, 0.191, 0.192, 0.20,
		0.201, 0.21, 0.211, 0.22, 0.221, 0.23, 0.231, 0.24, 0.25,
		0.251, 0.26, 0.27, 0.271, 0.28, 0.29, 0.30, 0.301, 0.31,
		0.32, 0.33, 0.34, 0.35, 0.36, 0.37, 0.38, 0.39, 0.40,
		0.41, 0.43, 0.50, 0.60, 0.65, 0.70, 0.75, 0.80, 0.85,
		0.90, 0.95, 0.97, 0.99 };
	const int logmax = sizeof logtab / sizeof *logtab;

	if(linear <= 0.0) {
		return 0.0;
	}

	if(linear >= 1.0) {
		return 1.0;
	}

	return logtab[(int) (logmax * linear)];
}

/*
 * _alDBToLinear( ALfloat dBs )
 *
 * Convert a logarithmic gain to a linear one.
 *
 * FIXME: So kludgey.
 */
ALfloat _alDBToLinear(ALfloat dBs) {
	static const float logtab[] = {
		0.00, 0.001, 0.002, 0.003, 0.004, 0.005, 0.01, 0.011,
		0.012, 0.013, 0.014, 0.015, 0.016, 0.02, 0.021, 0.022,
		0.023, 0.024, 0.025, 0.03, 0.031, 0.032, 0.033, 0.034,
		0.04, 0.041, 0.042, 0.043, 0.044, 0.05, 0.051, 0.052,
		0.053, 0.054, 0.06, 0.061, 0.062, 0.063, 0.064, 0.07,
		0.071, 0.072, 0.073, 0.08, 0.081, 0.082, 0.083, 0.084,
		0.09, 0.091, 0.092, 0.093, 0.094, 0.10, 0.101, 0.102,
		0.103, 0.11, 0.111, 0.112, 0.113, 0.12, 0.121, 0.122,
		0.123, 0.124, 0.13, 0.131, 0.132, 0.14, 0.141, 0.142,
		0.143, 0.15, 0.151, 0.152, 0.16, 0.161, 0.162, 0.17,
		0.171, 0.172, 0.18, 0.181, 0.19, 0.191, 0.192, 0.20,
		0.201, 0.21, 0.211, 0.22, 0.221, 0.23, 0.231, 0.24, 0.25,
		0.251, 0.26, 0.27, 0.271, 0.28, 0.29, 0.30, 0.301, 0.31,
		0.32, 0.33, 0.34, 0.35, 0.36, 0.37, 0.38, 0.39, 0.40,
		0.41, 0.43, 0.50, 0.60, 0.65, 0.70, 0.75, 0.80, 0.85,
		0.90, 0.95, 0.97, 0.99 };
	const int logmax = sizeof logtab / sizeof *logtab;
	int max = logmax;
	int min = 0;
	int mid;
	int last = -1;

	if(dBs <= 0.0) {
		return 0.0;
	}

	if(dBs >= 1.0) {
		return 1.0;
	}

	mid = (max - min) / 2;
	do {
		last = mid;

		if(logtab[mid] == dBs) {
			break;
		}

		if(logtab[mid] < dBs) {
			/* too low */
			min = mid;
		} else { 
			/* too high */
			max = mid;
		}

		mid = min + ((max - min) / 2);
	} while(last != mid);

	return ((float) mid / logmax);
}

/*
 *  _al_RAWFORMAT( ALenum format )
 *
 *  Returns AL_TRUE if format is an openal format specifying raw pcm data,
 *  AL_FALSE otherwise.
 */
ALboolean _al_RAWFORMAT(ALenum format) {
	switch(format) {
		case AL_FORMAT_MONO16:
		case AL_FORMAT_MONO8:
		case AL_FORMAT_STEREO16:
		case AL_FORMAT_STEREO8:
			return AL_TRUE;
		default:
			break;
	}

	return AL_FALSE;
}

/*
 * _al_formatbits( ALenum format )
 *
 * Returns bit depth of format.
 */
ALbyte _al_formatbits(ALenum format) {
	switch(format) {
		case AL_FORMAT_MONO16:
		case AL_FORMAT_STEREO16:
		case AL_FORMAT_IMA_ADPCM_MONO16_EXT:
		case AL_FORMAT_IMA_ADPCM_STEREO16_EXT:
			return 16;
			break;
		case AL_FORMAT_MONO8:
		case AL_FORMAT_STEREO8:
			return 8;
			break;
	}

	ASSERT(0);

	return -1;
}

/*
 * _alSmallestPowerOfTwo( ALuint num )
 *
 * Returns smallest power of two large that meets or exceeds num.
 */
ALuint _alSmallestPowerOfTwo( ALuint num ) {
	ALuint retval = 1;

	while( retval < num ) {
		retval <<= 1;
	}

	return retval;
}

/*
 * _alIsFinite( ALfloat v )
 *
 * Returns AL_TRUE if v is a finite, non NaN value, AL_FALSE otherwise.
 */
ALboolean _alIsFinite( ALfloat v ) {
	/* skip infinite test for now */
	if(v == v) {
		return AL_TRUE;
	}

	return AL_FALSE;
}
