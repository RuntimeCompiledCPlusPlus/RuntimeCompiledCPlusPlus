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
// File:				"Image.h"
//
// Related Source File:		"Image.cpp"
//
// Original Author:			Douglas John Binks (DJB)
//
// Creation Date:			03 Nov 2000
//
// Specification Document:	DJBTODO
//
// Purpose:					This is the interface definition of the IImage Class, which
//							is the base interface for storing, input/output etc of Images
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef ImageDef
#define ImageDef


class IImage  
{
public:
	enum  EImageScale
	{
		ScaleOne,
		ScaleOneHalf,
		ScaleOneQuarter,
		ScaleOneEighth
	};
	enum EImageFormat
	{
		I8,			//8bit intensity
		RGB8,		//8bit red-green-blue
		RGBA8,		//8bit red-green-blue-alpha
	};
	IImage(unsigned int uiImageWidth_, unsigned int uiImageHeight_, EImageFormat eFormat = RGB8 );
	IImage();
	virtual ~IImage();
	inline unsigned int GetWidth()		const {return m_uiWidth;}
	inline unsigned int GetHeight()		const {return m_uiHeight;}
	inline unsigned int GetDataSize()	const {return m_uiDatasize;}
	inline EImageFormat	GetFormat()		const {return m_eFormat;}
	virtual void PutToFile(const char *const pacFilename_) = 0;


	/////////////////////////////////////////////////////////////////////////////////////
	//	Function to get a  pointer to the image array (so that it can be directly
	//	read to). Use Extreme care with this function.
	/////////////////////////////////////////////////////////////////////////////////////
	inline char* GetImagePointer() 
	{
		return m_pacImage_data;
	};

protected:
	void InitialiseStorage();
	void ScaleImage( EImageScale eisScale_ );
	unsigned int CalcDataSize(	unsigned int uiImageWidth_,
								unsigned int uiImageHeight_,
								EImageFormat eFormat_ );
	unsigned int m_uiWidth;
	unsigned int m_uiHeight;
	unsigned int m_uiDatasize;
	EImageFormat m_eFormat;

private:
	char* m_pacImage_data;

};

#endif 