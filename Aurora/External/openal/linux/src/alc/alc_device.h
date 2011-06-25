/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * alc_device.h
 *
 * Prototypes, defines etc for device aquisition and management.
 */
#ifndef ALC_DEVICE_H_
#define ALC_DEVICE_H_

#include "alc_context.h"
#include "AL/altypes.h"

/*
 * _alcDevicePause( AL_device *dev )
 *
 * Pauses a device.
 */
void _alcDevicePause( AL_device *dev );

/*
 * _alcDeviceResume( AL_device *dev );
 *
 * Resumes a device.
 */
void _alcDeviceResume( AL_device *dev );

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
ALboolean _alcDeviceSet( AL_device *dev );

#endif /* _ALC_DEVICE_H_ */
