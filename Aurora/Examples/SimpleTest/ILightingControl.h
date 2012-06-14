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

#ifndef ILIGHTINGCONTROL_INCLUDED
#define ILIGHTINGCONTROL_INCLUDED

#include "IEntityObject.h"
#include "InterfaceIds.h"
#include "../../Systems/IUpdateable.h" 


struct ILightingControl : public  TInterface<IID_ILIGHTINGCONTROL,IEntityObject>, public IAUUpdateable
{
	// All Get/Set expect float[4] as parameter
	virtual void GetBackColor(float* params) = 0; 
	virtual void SetBackColor(const float* params) = 0;
	virtual void GetGlobalAmbient(float* params) = 0; 
	virtual void SetGlobalAmbient(const float* params) = 0;
	virtual void GetLightAmbient(float* params) = 0;
	virtual void SetLightAmbient(const float* params) = 0; 
	virtual void GetLightDiffuse(float* params) = 0;
	virtual void SetLightDiffuse(const float* params) = 0;
	virtual void GetLightSpecular(float* params) = 0;
	virtual void SetLightSpecular(const float* params) = 0;
};


#endif // ILIGHTINGCONTROL_INCLUDED