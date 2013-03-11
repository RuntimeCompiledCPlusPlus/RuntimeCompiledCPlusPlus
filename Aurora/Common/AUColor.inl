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
#ifndef AUCOLOR_DEF
#define AUCOLOR_DEF

#include <string.h> // for memcpy on linux

class AUColor
{
public:
	AUColor(float Red = 0.0f, float Green = 0.0f, float Blue = 0.0f, float Alpha = 0.0f)
	{		
		m_Color.r = Red;
		m_Color.g = Green;
		m_Color.b = Blue;
		m_Color.a = Alpha;
	}

	AUColor( const float rgba[4] )
	{
		memcpy( m_Color.rgba, rgba, sizeof( m_Color.rgba ) );
	}

	union ColorUnion
	{
		struct
		{
			float r;
			float g;
			float b;
			float a;
		};
		float rgba[4];
	} m_Color;

};


#endif //AUCOLOR_DEF