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

#ifndef REFERENCECOUNTABLE_INCLUDED
#define REFERENCECOUNTABLE_INCLUDED

// Based on libRocket ReferenceCountable class


class ReferenceCountable
{
public:
	ReferenceCountable(int initial_count = 1);
	virtual ~ReferenceCountable();

	virtual int GetReferenceCount();
	virtual void AddReference();
	virtual void RemoveReference();

	// Catches incorrect copy attempts
	ReferenceCountable& operator=(const ReferenceCountable& copy);

protected:
	// Hook method for when reference count climbs above zero
	virtual void OnReferenceActivate();
	// Hook method for when reference count hits zero
	virtual void OnReferenceDeactivate();

private:
	int m_refCount;
};

#endif // REFERENCECOUNTABLE_INCLUDED
