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
// File:				"JPGImage.cpp"
//
// Related Header File:		"JPGImage.h"
//
// Original Author:			Douglas John Binks (DJB)
//
// Creation Date:			03 Nov 2000
//
// Specification Document:	DJBTODO
//
// Purpose:					This is the implementation of the ImageJPG Class, which
//							is for storing, input/output etc of JPG Images,
//							using the independant JPEG Group libraries.
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "ImageJPG.h"

//INDEPENDANT JPEG GROUP INCLUDES
extern  "C"
{
#include "jpeglib.h"	//main include
#include <setjmp.h>		//used for error reporting from the jpeg library
}

#include <fstream>
using namespace std;


//////////////////////////////////////////////////////////////////////////////////////////
// Function:				ImageJPG::ImageJPG
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			03 Nov 2000
//
// Purpose:					Constructor for empty JPG images.
//
// Inputs:					uiImageWidth_:	width of image to be created
//							uiImageHeight_:	height of image to be created
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
ImageJPG::ImageJPG(unsigned int uiImageWidth_, unsigned int uiImageHeight_)
			: IImage( uiImageWidth_, uiImageHeight_ )
{
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				ImageJPG::ImageJPG
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			03 Nov 2000
//
// Purpose:					Constructor for JPG images from a file.
//
// Inputs:					strFilename_:	filename of image file
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
ImageJPG::ImageJPG(string strFilename_, EImageScale eScale )
{
	struct jpeg_decompress_struct cinfo;	//jpeg decompression `object'
	FILE* infile;							// Source file of jpeg image
	JSAMPARRAY buffer;						// Output row buffer
	int row_stride;							// physical row width in output buffer
	
	errno_t err = fopen_s(&infile, strFilename_.c_str(), "rb");
	if (err)
	{
		//could not open image file
		//DJBTODO: should log this
		return;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Step 1: allocate and initialize JPEG decompression object
	//////////////////////////////////////////////////////////////////////////////////
	
	// Set up the normal JPEG error routines
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);

	// Initialize the JPEG decompression object
	jpeg_create_decompress(&cinfo);
	

	//////////////////////////////////////////////////////////////////////////////////
	// Step 2: specify data source (eg, a file)
	//////////////////////////////////////////////////////////////////////////////////
	jpeg_stdio_src(&cinfo, infile);
	
	//////////////////////////////////////////////////////////////////////////////////
	// Step 3: read file parameters with jpeg_read_header() 
	//////////////////////////////////////////////////////////////////////////////////
	jpeg_read_header(&cinfo, TRUE);
	
	//////////////////////////////////////////////////////////////////////////////////
	// Step 4: set parameters for decompression
	//////////////////////////////////////////////////////////////////////////////////
	
	/* In this example, we don't need to change any of the defaults set by
	* jpeg_read_header(), so we do nothing here.
	*/
	switch( eScale )
	{
	case ScaleOne:
		cinfo.scale_denom = 1;
		break;
	case ScaleOneHalf:
		cinfo.scale_denom = 2;
		break;
	case ScaleOneQuarter:
		cinfo.scale_denom = 4;
		break;
	case ScaleOneEighth:
		cinfo.scale_denom = 8;
		break;
	}
	
	//////////////////////////////////////////////////////////////////////////////////
	// Step 5: Start decompressor
	//////////////////////////////////////////////////////////////////////////////////	
	jpeg_start_decompress(&cinfo);
	
	//allocate memory
	row_stride = cinfo.output_width * cinfo.output_components;
	switch( cinfo.output_components )
	{
	case 1:
		m_eFormat = IImage::I8;
		break;
	case 3:
		m_eFormat = IImage::RGB8;
		break;
	default:;
		//oops, unsupported format... stop
		//DJBTODO should log this
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return;
	}

	m_uiHeight	= cinfo.output_height;
	m_uiWidth	= cinfo.output_width;
	InitialiseStorage();
	buffer = new unsigned char*;
	*buffer = reinterpret_cast<unsigned char*>( GetImagePointer() );
	
	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */
	
	/* Here we use the library's state variable cinfo.output_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	*/
	while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
	* Here the array is only one element long, but you could ask for
	* more than one scanline at a time if that's more convenient.
		*/
		jpeg_read_scanlines(&cinfo, buffer, 1);
		buffer[0] += row_stride;
	}
	
	//////////////////////////////////////////////////////////////////////////////////
	// Step 7: Finish decompression
	//////////////////////////////////////////////////////////////////////////////////
	jpeg_finish_decompress(&cinfo);
	
	//////////////////////////////////////////////////////////////////////////////////
	// Step 8: Release JPEG decompression object/
	//////////////////////////////////////////////////////////////////////////////////
	jpeg_destroy_decompress(&cinfo);
	
	fclose(infile);
	
	delete buffer;
	//At this point you may want to check to see whether any corrupt-data
	//warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	
}


//////////////////////////////////////////////////////////////////////////////////////////
// Function:				ImageJPG::PutToFile
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			03 Nov 2000
//
// Purpose:					Saves image to a PPM file.	NOTE CURRENTLY PUTS IMAGE TO PPM FILE!
//
// Inputs:					Filename: file to create
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void ImageJPG::PutToFile(const char *const pacFilename_)
{
	if(m_uiDatasize)
	{
		ofstream outFile;
		outFile.open(pacFilename_, ios::binary);
		if(!outFile.fail()) //open successful
		{
			//output magic number
			outFile << "P6" << endl;

			//output width and height
			outFile << m_uiWidth << " " << m_uiHeight << endl;

			//output number of colors
			outFile << 255 << endl;

			//output data
			outFile.write( GetImagePointer(), m_uiDatasize );
		}
	}
}
