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
// File:				"PNMImage.h"
//
// Related Source File:		"PNMImage.cpp"
//
// Original Author:			Douglas John Binks (DJB)
//
// Creation Date:			24 Jan 2000
//
// Specification Document:	DJBTODO
//
// Purpose:					This is the interface definition of the ImagePNM Class, which
//							is for storing, input/output etc of PNM Images,
//							which are defined in a file as follows:
//
//							For RGB .ppm files:
//
//							P6					 #P6 for binary image data, P3 for text
//							width height		
//							largest_colour_value #we use 255, thus ignore this
//							RGBRGBRGB...
//
//							For greyscale .pgm files
//
//							P5					 #P5 for binary image data, P2 for text
//							width height		
//							largest_colour_value #we use 255, thus ignore this
//							IIIIIIII...
//
//
//							NOTE: P3 and P2 type (ascii colours r g b ... or I I I ...)
//							are not supported, nor is the .pbm (black or white).
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef PNMImageDef
#define PNMImageDef

#include "IImage.h"
#include <string>
using namespace std;

class ImagePNM  : public IImage
{
public:
	ImagePNM(unsigned int uiImageWidth_, unsigned int uiImageHeight_, EImageFormat eFormat = RGB8 );
	ImagePNM(string strFilename_);
	~ImagePNM();
	virtual void PutToFile(const char *const pacFilename_);
};

#endif 