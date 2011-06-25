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

//////////////////////////////////////////////////////////////////////////////////////////
// File:				"AURenTexture.h"
//
// Related Source File:		"AURenTexture.cpp"
//
// Original Author:			Douglas John Binks (DJB)
//
// Creation Date:			18 Oct 2000
//
// Specification Document:	DJBTODO
//
// Purpose:					Defines the interface for the AURenTexture Class, which
//							handles OpenGL texture object loading, currently using PPM
//							image files.
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef AURenTextureDef
#define AURenTextureDef

#include "IImage.h"
#include <string>


class AURenTexture  
{
public:
	AURenTexture( const std::string& ImageFilename, IImage::EImageScale scale = IImage::ScaleOne, bool bMakeAlpha_ = false, bool bEdgeClamped_ = false, bool bMipMapped = true );
	AURenTexture( IImage& Image, bool bMakeAlpha_ = false, bool bEdgeClamped_ = false, bool bMipMapped = true );
	~AURenTexture();
	void SetTexture();
private:
	void Construct( IImage& Image, bool bMakeAlpha_, bool bEdgeClamped_, bool bMipMapped = true );
	unsigned int texNum;		//OpenGL texture object identifier.
};



#endif