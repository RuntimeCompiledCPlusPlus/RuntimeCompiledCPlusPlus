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

#ifndef IGUIDOCUMENT_INCLUDED
#define IGUIDOCUMENT_INCLUDED

struct IGUIElement;


struct IGUIDocument
{
	virtual IGUIElement* Element() = 0; // access IGUIElement interface, doesn't add a reference
	virtual void Show() = 0;
	virtual void Hide() = 0;

	// Reference counting
	virtual void AddReference() = 0;
	virtual void RemoveReference() = 0;
};

#endif // IGUIDOCUMENT_INCLUDED