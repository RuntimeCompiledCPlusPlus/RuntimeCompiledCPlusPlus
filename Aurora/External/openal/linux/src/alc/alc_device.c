/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * alc_device.c
 *
 */
#include "al_siteconfig.h"
#include <AL/alc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AL/altypes.h"

#include "al_config.h"
#include "al_main.h"
#include "al_types.h"
#include "al_debug.h"
#include "al_state.h"

#include "alc_context.h"
#include "alc_error.h"
#include "alc_device.h"

#include "arch/interface/interface_sound.h"

/*
 * alcOpenDevice( const ALubyte *deviceSpecifier )
 *
 * Opens a device, using the alrc expression deviceSpecifier to specify
 * attributes for the device.  Returns the device if successful, NULL
 * otherwise.
 */
ALCdevice *alcOpenDevice( const ALubyte *deviceSpecifier ) {
	ALCdevice *retval;
	char dirstr[65];
	static ALboolean first_time = AL_TRUE;
	Rcvar foo = NULL;
	Rcvar direction = NULL;
	Rcvar freq_sym = NULL;
	UNUSED(Rcvar devices) = NULL;

	if( first_time == AL_TRUE ) {
		/* first initialization */
		first_time = AL_FALSE;

		if( _alParseConfig() == AL_FALSE ) {
			_alDebug(ALD_CONFIG, __FILE__, __LINE__,
				"Couldn't parse config file.");
		}
	}

	/* see if the user defined devices, sampling-rate, or direction */
	devices   = rc_lookup( "devices" );
	direction = rc_lookup( "direction" );
	freq_sym  = rc_lookup( "sampling-rate" );

	/* get the attributes requested in the args */
	if( deviceSpecifier ) {
		foo = rc_eval( (const char *) deviceSpecifier );
	}

	/* define each attribute pair */
	rc_foreach( foo, rc_define_list );

	/* redefine the stuff we saved */
	if( direction != NULL ) {
		rc_define( "direction", alrc_quote( direction ));
	}

	if( devices != NULL ) {
		rc_define( "devices", alrc_quote( devices ) );
	}

	if( freq_sym != NULL ) {
		freq_sym = rc_define( "sampling-rate", alrc_quote( freq_sym ));
	}

	direction = rc_lookup( "direction" );
	devices   = rc_lookup( "devices" );
	freq_sym  = rc_lookup( "sampling-rate" );

	if( direction != NULL ) {
		switch( rc_type( direction ) ) {
			case ALRC_STRING:
				rc_tostr0(direction, dirstr, 64);
				break;
			case ALRC_SYMBOL:
				rc_symtostr0(direction, dirstr, 64);
				break;
			default:
				memset( dirstr, 0, 64 );
				break;
		}
	}

	retval = malloc( sizeof *retval );
	if( retval == NULL) {
		/* FIXME: set AL_OUT_OF_MEMORY here? */

		return NULL;
	}

	/* defaults */
	retval->format = _ALC_EXTERNAL_FMT;
	retval->speed  = _ALC_EXTERNAL_SPEED;
	retval->bufsiz = _ALC_DEF_BUFSIZ;
	retval->flags  = ALCD_NONE;

	if( freq_sym != NULL ) {
		switch(rc_type( freq_sym )) {
			case ALRC_INTEGER:
			case ALRC_FLOAT:
				retval->speed = rc_toint( freq_sym );
				break;
			default:
				_alDebug(ALD_CONVERT, __FILE__, __LINE__,
					"invalid type %s for sampling-rate",
					rc_typestr( rc_type( freq_sym ) ));
				break;
		}
	}

	if( strncmp(dirstr, "read", 64 ) == 0 ) {
		/* capture */

		retval->handle = grab_read_audiodevice( );
		if( retval->handle == NULL ) {
			free( retval );
			_alcSetError(ALC_INVALID_DEVICE);
			return NULL;
		}

		retval->flags |= ALCD_READ;
	} else {
		/* write (default) */

		retval->handle = grab_write_audiodevice( );
		if( retval->handle == NULL ) {
			free( retval );
			_alcSetError(ALC_INVALID_DEVICE);
			return NULL;
		}

		retval->flags |= ALCD_WRITE;
	}

	return retval;
}

/*
 * alcCloseDevice( ALCdevice *dev )
 *
 * Closes the device referred to by dev.
 */
void alcCloseDevice( ALCdevice *dev ) {
	release_audiodevice( dev->handle );

	free( dev );

	return;
}

/*
 * _alcDeviceSet( AL_device *dev )
 *
 * Sets the attributes for the device from the settings in the device.  The
 * library is free to change the parameters associated with the device, but
 * until _alcDeviceSet is called, none of the changes are important.
 *
 * Returns AL_TRUE if the setting operation was possible, AL_FALSE otherwise.
 * After a call to this function, the caller should check the members in dev
 * is see what the actual values set where.
 */
ALboolean _alcDeviceSet( AL_device *dev ) {
	ALboolean retval = AL_FALSE;

	if( dev->flags & ALCD_WRITE ) {
		retval = set_write_audiodevice( dev->handle,
			&dev->bufsiz, &dev->format, &dev->speed);
	} else {
		retval = set_read_audiodevice( dev->handle,
			&dev->bufsiz, &dev->format, &dev->speed);
	}

	_alDebug( ALD_CONVERT, __FILE__, __LINE__,
		  "after set_audiodevice, f|s|b 0x%x|%d|%d",
		  dev->format,
		  dev->speed,
		  dev->bufsiz );

	return retval;
}

/*
 * _alcDevicePause( AL_device *dev )
 *
 * Pauses a device.
 */
void _alcDevicePause( AL_device *dev  ) {
        if ( dev )
		pause_audiodevice( dev->handle );
	return;
}

/*
 * _alcDeviceResume( AL_device *dev );
 *
 * Resumes a device.
 */
void _alcDeviceResume( AL_device *dev  ) {
        if ( dev )
		resume_audiodevice( dev->handle );
	return;
}
