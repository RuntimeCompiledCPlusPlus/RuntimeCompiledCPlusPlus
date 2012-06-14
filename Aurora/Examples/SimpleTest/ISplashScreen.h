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

#ifndef ISPLASHSCREEN_INCLUDED
#define ISPLASHSCREEN_INCLUDED

#include "IEntityObject.h"
#include "InterfaceIds.h"
#include "../../Systems/IUpdateable.h" 

struct ISplashScreen : public  TInterface<IID_ISPLASHSCREEN,IEntityObject>, public IAUUpdateable
{
	virtual void SetImage( const char* imageFile ) = 0;
	virtual void SetMinViewTime( float fSeconds ) = 0; // Min time before user can close
	virtual void SetFadeInTime( float fSeconds ) = 0;   // Default is 0
	virtual void SetFadeOutTime( float fSeconds ) = 0;  // Default is 0
	virtual void SetAutoClose( bool bAutoClose ) = 0;  // Auto close after min time
	virtual bool ReadyToClose() const = 0;
};


#endif // ISPLASHSCREEN_INCLUDED