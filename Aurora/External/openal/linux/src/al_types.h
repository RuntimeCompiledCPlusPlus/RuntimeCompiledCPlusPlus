/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_types.h
 *
 * internal type definitions
 */
#ifndef _LAL_TYPES_H_
#define _LAL_TYPES_H_

#include "AL/altypes.h"

#include "mutex/mutexlib.h"

#define _ALC_MAX_CHANNELS    6
#define _ALC_MAX_FILTERS     8

#define _ALF_MAX_NAME        15
#define _AL_MAX_DISTANCE     (60.0f)
#define SIZEOFVECTOR         (sizeof(ALfloat) * 3)

typedef struct _ALmatrix {
	ALfloat **data;
	ALint rows;
	ALint cols;
} ALmatrix;

typedef enum _DeviceEnum {
	ALCD_NONE                = 0,
	ALCD_WRITE               = (1<<0),
	ALCD_READ                = (1<<1)
} DeviceEnum;

typedef enum _EnableEnum {
	ALE_NONE                 = 0
} EnableEnum;

typedef enum _SrcEnum {
	ALS_NONE           = 0,
	ALS_REVERB         = (1<<0),
	ALS_NEEDPITCH      = (1<<1)  /* don't increment */
} SrcEnum;

/* flags that are set in the source on a buffer by buffer basis */
typedef enum _QueueEnum {
	ALQ_NONE           = 0,
	ALQ_CALLBACKBUFFER = (1<<0) /* don't increment */
} QueueEnum;

typedef enum _Bufenum {
	ALB_NONE           = 0,
	ALB_STREAMING      = (1<<1),
	ALB_STREAMING_WRAP = (1<<2),
	ALB_CALLBACK       = (1<<3),
	ALB_PENDING_DELETE = (1<<4)
} Bufenum;

typedef struct {
	ALboolean isset;
	ALfloat data;
} AL_floatparam;

typedef struct {
	ALboolean isset;
	ALfloat data[3];
} AL_float3vparam;

typedef struct {
	ALboolean isset;
	ALfloat data[6];
} AL_float6vparam;

typedef struct {
	ALboolean isset;
	ALint data;
} AL_intparam;

typedef struct {
	ALboolean isset;
	ALboolean data;
} AL_boolparam;

typedef struct {
	ALboolean isset;
	ALuint data;
} AL_uintparam;

typedef struct {
	ALboolean isset;
	ALshort data;
} AL_shortparam;


typedef struct _AL_extension {
	const ALubyte *name;
	void *addr;
} AL_extension;

/* internal use destruction callbacks */
typedef void (*DestroyCallback_LOKI)(ALuint id);

/* scratch space to avoid unneeded reallocs */
typedef struct {
	void *data[_ALC_MAX_CHANNELS];
	ALuint len;
} _alDecodeScratch;

typedef struct _AL_bufsourcelist {
	ALuint *sids;
	ALuint size;
	ALuint items;
} AL_bufsourcelist;

/*
 *  Our buffer
 */
typedef struct {
	ALuint bid;         /* unique identifier */

	void *orig_buffers[_ALC_MAX_CHANNELS];    /* original buffer: unadulturated */
	ALuint num_buffers; /* number of populated channels in orig_buffers */

	Bufenum flags;
	/* int refcount; */

	ALuint  size;
	ALshort format;
	ALuint  freq;

	/* our refcount */
	AL_bufsourcelist queue_list;
	AL_bufsourcelist current_list;

	ALuint streampos; /* position for streaming sounds */
	ALuint appendpos; /* position for append operation */

	/* callback for on-the-fly decoding */
	int (*callback)(ALuint, ALuint, ALshort *, ALenum, ALint, ALint);

	DestroyCallback_LOKI destroy_buffer_callback; /* callback for when
						       * buffer is destroyed */
	DestroyCallback_LOKI destroy_source_callback; /* callback for when
						       * source is destroyed */
} AL_buffer;

/*
 * AL_sourcestate is used to store buffer specific parameters.
 */
typedef struct _AL_sourcestate {
	QueueEnum flags;

	AL_boolparam islooping;
} AL_sourcestate;

typedef struct _AL_source {
	/*
	 * non-buffer specific params
	 */
	AL_float3vparam position;
	AL_float3vparam direction;
	AL_float3vparam velocity;

	AL_floatparam gain;
	AL_floatparam attenuationmin;
	AL_floatparam attenuationmax;
	AL_floatparam ref_distance;
	AL_floatparam max_distance;
	AL_floatparam rolloff_factor;

	AL_boolparam isstreaming;
	AL_boolparam isrelative;

	AL_floatparam coneinnerangle;
	AL_floatparam coneouterangle;
	AL_floatparam coneoutergain;

	AL_floatparam pitch;

	struct {
		AL_sourcestate *queuestate;
		ALuint *queue;

		int size;
		int read_index;  /* what al_mixer reads   */
		int write_index; /* what Queue{i,f} reads */
	} bid_queue;  /* buffer queue */

	/* Our state */
	ALenum state;

	struct {
		unsigned long soundpos; /* position into sound */
		void *outbuf;           /* buffer where output data is stored */
		ALuint delay[_ALC_MAX_CHANNELS];
		ALfloat gain[_ALC_MAX_CHANNELS];
	} srcParams; /* values refreshed each iteration through SplitSources */

	SrcEnum flags;

	/* kludges follow */
	void *reverb_buf[_ALC_MAX_CHANNELS]; /* reverb data */
	long reverbpos;
	long reverblen;

	ALfloat reverb_scale;
	ALint   reverb_delay;

	ALuint sid; /* our identifier */
} AL_source;

typedef struct _AL_listener {
	ALfloat Position[3];
	ALfloat Velocity[3];
	ALfloat Orientation[6];

	ALfloat Gain;
} AL_listener;

typedef struct _AL_capture {
	ALuint cpid;
	ALuint state;

	AL_intparam bid;
} AL_capture;

/* pool structures */
typedef struct {
	AL_source data;
	ALboolean inuse;
} spool_node;

typedef struct {
	spool_node *pool;
	ALuint size;
	ALuint *map;        /* { sid, index } pair.  map[index] = sid */
	MutexID *smutexen;  /* array of source locks */
} spool_t;

typedef void time_filter (ALuint c, AL_source *src, AL_buffer *samp,
		   ALshort **buffers, ALuint nc, ALuint len);

/* types */
typedef struct _time_filter_set {
	char name[ _ALF_MAX_NAME + 1 ];
	time_filter *filter;
} time_filter_set;

typedef struct _AL_device {
	void *handle;	     /*
			      * pointer to something that is meaningful
			      * to the OS's sound stuff
			      */

	/* device settings, not internal format */
	ALenum format;
	ALuint speed;
	ALuint bufsiz;

	DeviceEnum flags;
} AL_device;

typedef struct _AL_context {
	AL_listener listener;

	ALuint enable_flags; /* for Enable(), Disable() */

	spool_t source_pool;

	ALboolean _inuse;

	AL_device *write_device;
	AL_device *read_device;

	/* distance of speaker from
	 * origin  [x,y,z].  Ranges from 0.0 (origin) to
	 * 1.0 * scaling_factor
	 *
	 * 0 -> left
	 * 1 -> right
	 * 2 -> left satellite
	 * 3 -> right satellite
	 */
	struct {
		ALfloat pos[3];
	} _speaker_pos[_ALC_MAX_CHANNELS];

	ALfloat doppler_factor;
	ALfloat doppler_velocity;

	ALint alErrorIndex;

	time_filter_set time_filters[_ALC_MAX_FILTERS];

	ALboolean should_sync; /* is mojo updated synchronous? */
	ALboolean issuspended; /* is the context suspended? */

	/* structure for restoring system mixer settings */
	struct {
		ALfloat main;
		ALfloat pcm;
	} restore;

	ALenum distance_model;
	ALfloat (*distance_func)( ALfloat gain, ALfloat rolloff,
				  ALfloat dis, ALfloat ref, ALfloat max );
} AL_context;

typedef struct _alMixEntry {
	ALvoid *data;
	ALint bytes;
} alMixEntry;

#endif /* _LAL_TYPES_H */
