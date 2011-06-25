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
// File:				"Image.cpp"
//
// Related Header File:		"Image.h"
//
// Original Author:			Douglas John Binks (DJB)
//
// Creation Date:			03 Nov 2000
//
// Specification Document:	DJBTODO
//
// Purpose:					This is the implementation of the IImage Class, which
//							is for storing, input/output etc of Images,
//
//////////////////////////////////////////////////////////////////////////////////////////
#include "IImage.h"
#include <string>


using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				IImage::IImage
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			03 Nov 2000
//
// Purpose:					Default Constructor.
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
IImage::IImage()
			 : m_uiWidth(0), m_uiHeight(0), m_uiDatasize(0), m_eFormat(RGB8)
{
	m_pacImage_data = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				IImage::IImage
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			03 Nov 2000
//
// Purpose:					Constructor for empty images.
//
// Inputs:					uiImageWidth_:	width of image to be created
//							uiImageHeight_:	height of image to be created
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
IImage::IImage(unsigned int uiImageWidth_, unsigned int uiImageHeight_, EImageFormat eFormat )
			 : m_uiWidth(uiImageWidth_), m_uiHeight(uiImageHeight_), m_eFormat(eFormat)
{
	InitialiseStorage();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				IImage::InitialiseStorage
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			03 Nov 2000
//
// Purpose:					Initialises the memory for empty images.
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void IImage::InitialiseStorage()
{
	m_uiDatasize = CalcDataSize(m_uiWidth, m_uiHeight, m_eFormat);
	if( m_uiDatasize )
		m_pacImage_data = new char[m_uiDatasize];
	else m_pacImage_data = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				IImage::~IImage
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			03 Nov 2000
//
// Purpose:					Destructor. Deallocates image array
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
IImage::~IImage()
{
	delete[] m_pacImage_data;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				IImage::ScaleImage
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			29 June 2001
//
// Purpose:					Scales the image down by the factor given
//
// Inputs:					eisScale_: the scale to scale to.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void IImage::ScaleImage( EImageScale eisScale_ )
{
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				IImage::ScaleImage
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			29 June 2001
//
// Purpose:					Caculates the datasize.
//
// Inputs:					uiImageWidth_, uiImageHeight_, eFormat as members
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
unsigned int IImage::CalcDataSize( unsigned int uiImageWidth_, unsigned int uiImageHeight_, EImageFormat eFormat_ )
{
	unsigned int uiDatasize;
	switch( eFormat_ )
	{
	case I8:
		uiDatasize = uiImageWidth_*uiImageHeight_;
		break;
	case RGB8:
		uiDatasize = 3*uiImageWidth_*uiImageHeight_;
		break;
	case RGBA8:
		uiDatasize = 4*uiImageWidth_*uiImageHeight_;
		break;
	}
	return uiDatasize;
}
