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

#include "ReferenceCountable.h"
#include <assert.h>


ReferenceCountable::ReferenceCountable(int initial_count /* = 1 */) : m_refCount(initial_count)
{

}

ReferenceCountable::~ReferenceCountable()
{
	assert(m_refCount == 0);
}

int ReferenceCountable::GetReferenceCount()
{
	return m_refCount;
}

void ReferenceCountable::AddReference()
{
	++m_refCount;
	if (m_refCount == 1)
	{
		OnReferenceActivate();
	}
}

void ReferenceCountable::RemoveReference()
{
	assert(m_refCount > 0);
	--m_refCount;
	if (m_refCount == 0)
	{
		OnReferenceDeactivate();
	}
}

ReferenceCountable& ReferenceCountable::operator=(const ReferenceCountable&)
{
	assert(false && "attempting to copy reference countable object. Not a good idea!");
	return *this;
}

void ReferenceCountable::OnReferenceActivate()
{
	// Hook method
}

void ReferenceCountable::OnReferenceDeactivate()
{
	// Hook method
}