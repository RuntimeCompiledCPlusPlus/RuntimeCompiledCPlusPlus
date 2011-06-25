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
// File:				"AURenTexture.cpp"
//
// Related Header File:		"AURenTexture.h"
//
// Original Author:			Douglas John Binks (DJB)
//
// Creation Date:			18 Oct 2000
//
// Specification Document:	DJBTODO
//
// Purpose:					Implements the AURenTexture Class, which
//							handles OpenGL texture object loading, currently using PPM
//							image files.
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "AURenTexture.h"
#include "ImagePNM.h"
#include "ImageJPG.h"

// Windows Requirements
#define WIN32_LEAN_AND_MEAN				// Exclude rarely-used stuff from Windows headers
#include <windows.h>

// OpenGL requirements
#include <GL/glew.h>
#include <GL/gl.h>
#include <gl/GLU.h>



#include "assert.h"


using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				AURenTexture::AURenTexture
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			02 May 2000
//
// Purpose:					Creates the OpenGL texture object, sets the texture directory
//							if this is not set and sets the textures properties.
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
AURenTexture::AURenTexture(const string& ImageFile, IImage::EImageScale scale, bool bMakeAlpha_, bool bEdgeClamped_, bool bMipMapped )
{

	IImage* pMyImage;

	//analyse the image type, currently support PPM and JPG using the extensions .ppm and .jpg
	//current analysis looks at last part of name, if m choose ppm and g choose jpg
	string strJPGEnding(".jpg");
	if( string::npos != ImageFile.find( strJPGEnding ) )
	{
		//have a jpeg image
		pMyImage = new ImageJPG(ImageFile, scale);
	}
	else
	{
		//scale not supported by PNMImage
		pMyImage = new ImagePNM(ImageFile);
	}

	Construct( *pMyImage, bMakeAlpha_, bEdgeClamped_, bMipMapped );
	delete pMyImage;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				AURenTexture::AURenTexture
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			02 May 2000
//
// Purpose:					Creates the OpenGL texture object, sets the texture directory
//							if this is not set and sets the textures properties.
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
AURenTexture::AURenTexture(IImage& Image_, bool bMakeAlpha_, bool bEdgeClamped_, bool bMipMapped )
{
	Construct( Image_, bMakeAlpha_, bEdgeClamped_, bMipMapped );
}

void AURenTexture::Construct(IImage& Image_, bool bMakeAlpha_, bool bEdgeClamped_, bool bMipMapped )
{

	glGenTextures( 1, &texNum ); 
	if ( Image_.GetDataSize() )
	{                                   		
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		
		glBindTexture( GL_TEXTURE_2D, texNum );

		if( false == bEdgeClamped_ )
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else
		{
			if( GLEW_EXT_texture_edge_clamp )
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE_EXT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE_EXT);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			}
		}

		// Set the filtering.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//`trilinear mipmapping'
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
						GL_LINEAR_MIPMAP_LINEAR);

		
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		
		switch( Image_.GetFormat() )
		{
		case IImage::RGBA8:	//RGBA image
            if( bMipMapped )
            {
			    gluBuild2DMipmaps( GL_TEXTURE_2D, 4, Image_.GetWidth(), Image_.GetHeight(),
				    		GL_RGBA, GL_UNSIGNED_BYTE, Image_.GetImagePointer() );
            }
            else
            {
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     		   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
               glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Image_.GetWidth(), Image_.GetHeight(), 0,
                            GL_RGBA, GL_UNSIGNED_BYTE, Image_.GetImagePointer() );
            }
			break;
		case IImage::RGB8:	//RGB image
            assert( bMipMapped );  //not implemented yet

			gluBuild2DMipmaps( GL_TEXTURE_2D, 3, Image_.GetWidth(), Image_.GetHeight(),
						GL_RGB, GL_UNSIGNED_BYTE, Image_.GetImagePointer() );
			break;
		case IImage::I8:	//greyscale image
            assert( bMipMapped );  //not implemented yet
			if( true == bMakeAlpha_ )
			{
				char* pacNewImage_data = new char[ Image_.GetDataSize() * 2 ];
				for( unsigned int uiCount = 0; uiCount < Image_.GetDataSize(); uiCount++ )
				{
					unsigned char cLuminance = Image_.GetImagePointer()[ uiCount ];
					pacNewImage_data[ 2*uiCount ]		= cLuminance;
					if( 127 > cLuminance )
					{
						pacNewImage_data[ 2*uiCount + 1 ]	= 2*cLuminance;
					}
					else
					{
						pacNewImage_data[ 2*uiCount + 1 ]	= (char)255;
					}
				}
				gluBuild2DMipmaps( GL_TEXTURE_2D, 2, Image_.GetWidth(), Image_.GetHeight(),
						GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, pacNewImage_data );
				delete[] pacNewImage_data;
			}
			else
			{
				gluBuild2DMipmaps( GL_TEXTURE_2D, 1, Image_.GetWidth(), Image_.GetHeight(),
						GL_LUMINANCE, GL_UNSIGNED_BYTE, Image_.GetImagePointer() );
			}
			break;
		}
		
	}
	//else have empty (white) texture	
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				AURenTexture::~AURenTexture
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			18 Oct 2000
//
// Purpose:					Deletes the texture object.
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
AURenTexture::~AURenTexture()
{
	glDeleteTextures(1, &texNum);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				AURenTexture::~SetTexture
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			18 Oct 2000
//
// Purpose:					Makes this texture object the current OpenGL texture object.
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void AURenTexture::SetTexture()
{
    glBindTexture( GL_TEXTURE_2D, texNum );
}
