//
// Copyright (c) 2010-2011 Matthew Jack and Doug Binks
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once

#ifndef IFILECHANGENOTIFIER_INCLUDED
#define IFILECHANGENOTIFIER_INCLUDED

#include "../RuntimeCompiler/AUArray.h"
#include <set>

struct IFileChangeListener;

struct IFileChangeNotifier
{
	virtual bool IsMonitoringActive() const = 0;
	virtual void SetMonitoringActive( bool bActive ) = 0;
	
	virtual float GetMinTimeBetweenNotifications() const = 0;
	virtual void SetMinTimeBetweenNotifications( float fMinTime ) = 0;
	
	// Delay to allow multiple file changes to accumulate before notifying listeners
	virtual float GetChangeNotifyDelay() const = 0;
	virtual void SetChangeNotifyDelay( float fDelay ) = 0;

	virtual void Update( float fTimeDelta ) = 0;
	virtual void Watch( const char *filename, IFileChangeListener *pListener ) = 0; // can be file or directory

	virtual void RemoveListener( IFileChangeListener *pListener ) = 0;
    virtual ~IFileChangeNotifier() {}
};


// IFileChangeListener will automatically deregister with FileChangeNotifier
// it registered with (via Watch method) on destruction
// Will not give expected results if the listener registers with multiple notifiers
struct IFileChangeListener
{
	IFileChangeListener() : _registeredNotifier(0) {}
	virtual ~IFileChangeListener()
	{
		if (_registeredNotifier)
		{
			_registeredNotifier->RemoveListener(this);
		}
	}


	// Listener must make copies of strings if it wants to store them
	virtual void OnFileChange( const IAUDynArray<const char*>& filelist ) = 0;


	// Should be called by IFileChangeNotifier implementation only
	void OnRegisteredWithNotifier( IFileChangeNotifier* pNotifier )
	{
		_registeredNotifier = pNotifier;
	}

private:
	IFileChangeNotifier* _registeredNotifier;
};


#endif // IFILECHANGENOTIFIER_INCLUDED