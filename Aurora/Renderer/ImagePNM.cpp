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
// File:				"PNMImage.cpp"
//
// Related Header File:		"PNMImage.h"
//
// Original Author:			Douglas John Binks (DJB)
//
// Creation Date:			09 Nov 2000
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
#include "ImagePNM.h"

#include <fstream>
using namespace std;


//////////////////////////////////////////////////////////////////////////////////////////
// Function:				ImagePNM::ImagePNM
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			09 Nov 2000
//
// Purpose:					Constructor for empty PNM images.
//
// Inputs:					uiImageWidth_:	width of image to be created
//							uiImageHeight_:	height of image to be created
//							eFormat: format of image
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
ImagePNM::ImagePNM(unsigned int uiImageWidth_, unsigned int uiImageHeight_, EImageFormat eFormat)
			: IImage( uiImageWidth_, uiImageHeight_, eFormat )
{
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				ImagePNM::ImagePNM
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			09 Nov 2000
//
// Purpose:					Constructor for PNM images from a file.
//
// Inputs:					strFilename_:	filename of image file
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
ImagePNM::ImagePNM(string strFilename_)
{
	ifstream inFile;
	inFile.open(strFilename_.c_str(), ios::binary);
	char *tempbuffer = new char[80]; //maximum length of line is 70, so 80 enough!
	if(!inFile.fail()) //open successful
	{
		enum ReadState
		{
			AwaitingWidth,
			ReadingWidth,
			AwaitingHeight,
			ReadingHeight,
			AwaitingColours,
			ReadingColours,
			AwaitingPixelData,
		} instage = AwaitingWidth;

		//get "magic number"
		inFile.read(tempbuffer, 2);
		if( !inFile.eof() && (tempbuffer[0] == 'P'))
		{
			switch( tempbuffer[1] )
			{
			case '5':
				m_eFormat = I8;
				break;
			case '6':
				m_eFormat = RGB8;
				break;
			default:
				//unknown type, reset state and exit
				delete[] tempbuffer;
				return;
			}
			//must handle cases of #, or all values on one line etc.
			unsigned int pos = 0;
			do
			{
				inFile.read(tempbuffer+pos, 1);
				if(!inFile.eof())
				{
					switch(tempbuffer[pos])
					{
					case '\n':
					case ' ' :
					case '\t':
					case '\r':
						switch(instage)
						{
						case AwaitingWidth:	
							break;
						case ReadingWidth:	//have read in all of first parameter
							m_uiWidth = atoi(tempbuffer); //try to translate
							instage = AwaitingHeight; //go to reading second parameter
							pos=0;			//reset position in buffer
							break;
						case AwaitingHeight:	
							break;
						case ReadingHeight:	//have read in all of first parameter
							m_uiHeight = atoi(tempbuffer); //try to translate
							instage = AwaitingColours; //go to awaiting second parameter
							pos=0;			//reset position in buffer
							break;
						case AwaitingColours:	
							break;
						case ReadingColours:	//have read in all of first parameter
							instage = AwaitingPixelData;//go to awaiting third parameter
							pos=0;			//reset position in buffer
							break;
						}
						break;
						
					case '#':	// # enountered, read to end of line and discard
							inFile.ignore(80, '\n');
							break;
							
					default:	//might be somthing, keep in in buffer
						switch(instage)
						{
						case AwaitingWidth:	
							instage = ReadingWidth;	//go to reading first parameter
							pos++;
							break;
						case ReadingWidth:
							pos++;
							break;
						case AwaitingHeight:	
							instage = ReadingHeight;//go to reading second parameter
							pos++;
							break;
						case ReadingHeight:
							pos++;
							break;
						case AwaitingColours:	
							instage = ReadingColours;//go to reading third parameter
							pos++;
							break;
						case ReadingColours:
							pos++;
							break;
						}							
					}
				}
				
			} while( !inFile.eof() && (instage !=  AwaitingPixelData) && (pos < 70));
			
			if( AwaitingPixelData == instage  ) 
			{
				InitialiseStorage();	//this sets the appropriate datasize
				inFile.read( GetImagePointer(), m_uiDatasize );
			}
			else
			{
				m_uiWidth = 0;
				m_uiHeight = 0;
			}
		}
	}
	delete[] tempbuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				ImagePNM::~ImagePNM
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			09 Nov 2000
//
// Purpose:					Destructor.
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
ImagePNM::~ImagePNM()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				ImagePNM::PutToFile
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			09 Nov 2000
//
// Purpose:					Saves image to a PNM file.
//
// Inputs:					Filename: file to create
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void ImagePNM::PutToFile(const char *const pacFilename_)
{
	if(m_uiDatasize)
	{
		ofstream outFile;
		outFile.open(pacFilename_, ios::binary);
		if(!outFile.fail()) //open successful
		{
			unsigned int uiPixSize;
			//output magic number
			switch( m_eFormat )
			{
			case IImage::I8:
				outFile << "P5" << endl;
				uiPixSize = 1;
				break;
			case IImage::RGB8:
				outFile << "P6" << endl;
				uiPixSize = 3;
				break;
			default:
				//unknown format, do not save data
				return;
			}

			//output width and height
			outFile << GetWidth() << " " << GetHeight() << endl;

			//output number of colors	(allways 255 in our case)
			outFile << 255 << endl;

			//output data in reverse line order as ogl is back to front
			unsigned int uiLineWidth =  GetWidth() * uiPixSize;
			char* pacLineStart = GetImagePointer() + GetDataSize() - uiLineWidth;
			for( unsigned int uiLine = 0; uiLine < GetHeight(); ++uiLine )
			{
				outFile.write(  pacLineStart, uiLineWidth );
				pacLineStart -= uiLineWidth;
			}
		}

	}
}
