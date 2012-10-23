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


#include "AURenMesh.h"


#ifndef _WIN32
    #define NO_ASSIMP //Currently not adding assimp support to other platforms
#endif


#include "../Common/AUVec3f.inl" //for cross product used in calculateing normals

// Windows Requirements
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN				// Exclude rarely-used stuff from Windows headers
	#include <windows.h>
#endif //_WIN32


#ifdef __MACH__
#include <OpenGL/gl.h>
#else
// OpenGL requirements
#include <GL/gl.h>
#endif //__MACH__

#ifndef NO_ASSIMP
    #include <assimp.hpp>
    #include <aiScene.h>
    #include <aiPostProcess.h>
#endif //NO_ASSIMP

#ifndef _WIN32
#include <string.h>
int _stricmp( const char* pS1, const char* pS2 )
{
    return strcasecmp( pS1, pS2 );
}
#endif

#include <fstream>
#include <assert.h>

AURenMesh::AURenMesh() :
				m_pafVertexCoordinates( NULL ),
				m_pafTextureCoordinates( NULL ),
				m_pafNormals( NULL ),
				m_pausTriangleIndices( NULL ),
				m_uiNumVertices( 0 ),
				m_uiNumTriangles( 0 )
{
}

AURenMesh::~AURenMesh()
{
	Clear();
}

void AURenMesh::Clear()
{
	delete[] m_pafNormals;
	delete[] m_pafVertexCoordinates;
	delete[] m_pafTextureCoordinates;
	delete[] m_pausTriangleIndices;
}

bool AURenMesh::LoadFromFile( const std::string& strFilename )
{
	// Safely delete any existing data before loading new mesh
	Clear();

	int index = (int)strFilename.size() - 3;
	std::string extension = index >= 0 ? strFilename.substr(index, 3) : "";
	if (!_stricmp(extension.c_str(), "aml"))
	{
		return LoadFromFileAML(strFilename);
	}
	else
	{
		return LoadFromFileImport(strFilename);
	}
}

bool AURenMesh::SaveToFile( const std::string& strFilename )
{
	std::ofstream outFile;
	outFile.open(strFilename.c_str(), std::ios::binary);

	if( !outFile )
	{
		return false;
	}

	outFile << "AML Aurora File" << std::endl;
	outFile << 1 << std::endl; // version
	outFile << m_uiNumVertices << std::endl;
	outFile << m_uiNumTriangles << std::endl;

	outFile.write( reinterpret_cast<char*>( m_pafVertexCoordinates ), 3 * m_uiNumVertices * sizeof( float ) ); 
	outFile.write( reinterpret_cast<char*>( m_pafTextureCoordinates ), 2 * m_uiNumVertices * sizeof( float ) ); 
	outFile.write( reinterpret_cast<char*>( m_pafNormals ), 3 * m_uiNumVertices * sizeof( float ) );
	outFile.write( reinterpret_cast<char*>( m_pausTriangleIndices ), 3 * m_uiNumTriangles * sizeof( unsigned short ) );

	outFile.close();

	return true;
}

bool AURenMesh::LoadFromFileAML( const std::string& strFilename_ )
{
	std::ifstream inFile;
	inFile.open(strFilename_.c_str(), std::ios::binary);

	if( !inFile )
	{
		return false;
	}

	inFile.ignore(10000,'\n');	//ignore first line

	int iVersion;
	inFile >> iVersion;		//currently ignore version number

	inFile >> m_uiNumVertices;

	inFile >> m_uiNumTriangles;

	//now discard end of line
	inFile.ignore(10000,'\n');

	m_pafVertexCoordinates = new float[ 3 * m_uiNumVertices ];
	m_pafNormals = new float[ 3 * m_uiNumVertices ];
	m_pafTextureCoordinates = new float[ 2 * m_uiNumVertices ];
	m_pausTriangleIndices = new unsigned short[ 3 * m_uiNumTriangles ];

	inFile.read( reinterpret_cast<char*>( m_pafVertexCoordinates ), 3 * m_uiNumVertices * sizeof( float ) ); 
	inFile.read( reinterpret_cast<char*>( m_pafTextureCoordinates ), 2 * m_uiNumVertices * sizeof( float ) ); 
	inFile.read( reinterpret_cast<char*>( m_pafNormals ), 3 * m_uiNumVertices * sizeof( float ) );
	inFile.read( reinterpret_cast<char*>( m_pausTriangleIndices ), 3 * m_uiNumTriangles * sizeof( unsigned short ) );


	return true;
}

bool AURenMesh::LoadFromFileImport( const std::string& strFilename )
{
#ifndef NO_ASSIMP
	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile( strFilename, aiProcessPreset_TargetRealtime_Fast );

	if (!pScene || pScene->mNumMeshes == 0)
	{
		return false;
	}

	ProcessScene(pScene);
	
	return true;
#else
    assert( false );
    return false;
#endif
}

void AURenMesh::ProcessScene( const aiScene* pScene )
{
#ifndef NO_ASSIMP
	// Calculate total number of verts and tris across all meshes in scene
	m_uiNumVertices = 0;
	m_uiNumTriangles = 0;
	for (unsigned int i=0; i<pScene->mNumMeshes; ++i)
	{
		aiMesh* pMesh = pScene->mMeshes[i];
		m_uiNumVertices += pMesh->mNumVertices;
		m_uiNumTriangles += pMesh->mNumFaces;
	}
	
	// Allocate sufficent space for all data
	m_pafVertexCoordinates = new float[ 3 * m_uiNumVertices ];
	m_pafNormals = new float[ 3 * m_uiNumVertices ];
	m_pafTextureCoordinates = new float[ 2 * m_uiNumVertices ];
	m_pausTriangleIndices = new unsigned short[ 3 * m_uiNumTriangles ];

	// Iterate through all meshes and load data
	int vertIndex = 0;
	int normalIndex = 0;
	int texIndex = 0;
	int triIndex = 0;
	for (unsigned int i=0; i<pScene->mNumMeshes; ++i)
	{
		aiMesh* pMesh = pScene->mMeshes[i];

		// Load Verts
		for (unsigned int j=0; j<pMesh->mNumVertices; ++j)
		{
			const aiVector3D& vec = pMesh->mVertices[j];
			m_pafVertexCoordinates[vertIndex] = vec.x;
			m_pafVertexCoordinates[vertIndex+1] = vec.y;
			m_pafVertexCoordinates[vertIndex+2] = vec.z;
			vertIndex += 3;
		}

		// Load Normals
		for (unsigned int j=0; j<pMesh->mNumVertices; ++j)
		{
			const aiVector3D& vec = pMesh->mNormals[j];
			m_pafNormals[normalIndex] = vec.x;
			m_pafNormals[normalIndex+1] = vec.y;
			m_pafNormals[normalIndex+2] = vec.z;
			normalIndex += 3;
		}

		// Load Tex Coords
		if (pMesh->HasTextureCoords(0))
		{
			for (unsigned int j=0; j<pMesh->mNumVertices; ++j)
			{
				const aiVector3D& vec = pMesh->mTextureCoords[0][j];
				m_pafTextureCoordinates[texIndex] = vec.x;
				m_pafTextureCoordinates[texIndex+1] = vec.y;
				texIndex += 2;			
			}
		}
		

		// Load Tris
		for (unsigned int j=0; j<pMesh->mNumFaces; ++j)
		{
			const aiFace& tri = pMesh->mFaces[j];
			m_pausTriangleIndices[triIndex] = tri.mIndices[0];
			m_pausTriangleIndices[triIndex+1] = tri.mIndices[1];
			m_pausTriangleIndices[triIndex+2] = tri.mIndices[2];
			triIndex += 3;
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				AURenMesh::AURenMesh
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			21 June 2000
//
// Purpose:					Normalises the size and position of the object to be
//							centered around the origin and of the correct BCube extent.
//
// Inputs:					fBCubeHalfWidth_ : float giving the half width of the
//							bounding cube.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void AURenMesh::NormaliseToBCubeHalfWidth( float fBCubeHalfWidth_ )
{
	//set up min and max variablse for each axis and use a real value from the
	//array to initialise (as a `made up' value may be wrong unless we use
	//floatmax for min etc.).
	float fMinX = m_pafVertexCoordinates[0];
	float fMaxX = m_pafVertexCoordinates[0];
	float fMinY = m_pafVertexCoordinates[1];
	float fMaxY = m_pafVertexCoordinates[1];
	float fMinZ = m_pafVertexCoordinates[2];
	float fMaxZ = m_pafVertexCoordinates[2];

	//Go through array of vertices to find the real min and max
	unsigned int uiCountCoords;
	for( uiCountCoords = 0;
		 uiCountCoords < 3 * m_uiNumVertices;
		 uiCountCoords += 3 )
	{
		if( fMinX > m_pafVertexCoordinates[ uiCountCoords ] )
		{
			fMinX = m_pafVertexCoordinates[ uiCountCoords ];
		}
		else
		{
			if( fMaxX < m_pafVertexCoordinates[ uiCountCoords ] )
			{
				fMaxX = m_pafVertexCoordinates[ uiCountCoords ];
			}
		}

		if( fMinY > m_pafVertexCoordinates[ uiCountCoords + 1 ] )
		{
			fMinY = m_pafVertexCoordinates[ uiCountCoords + 1 ];
		}
		else
		{
			if( fMaxY < m_pafVertexCoordinates[ uiCountCoords + 1 ] )
			{
				fMaxY = m_pafVertexCoordinates[ uiCountCoords + 1 ];
			}
		}

		if( fMinZ > m_pafVertexCoordinates[ uiCountCoords + 2 ] )
		{
			fMinZ = m_pafVertexCoordinates[ uiCountCoords + 2 ];
		}
		else
		{
			if( fMaxZ < m_pafVertexCoordinates[ uiCountCoords + 2 ] )
			{
				fMaxZ = m_pafVertexCoordinates[ uiCountCoords + 2 ];
			}
		}
	}

	//calulate the current center
	float fCenterX = ( fMaxX + fMinX )/2.0f;
	float fCenterY = ( fMaxY + fMinY )/2.0f;
	float fCenterZ = ( fMaxZ + fMinZ )/2.0f;

	//calculate the largest distance^2 from the center
	float fMaxDistance2 = 0.0f;
	float fDistance2;
	float fX2, fY2, fZ2;
	for( uiCountCoords = 0;
		 uiCountCoords < 3 * m_uiNumVertices;
		 uiCountCoords += 3 )
	{
		fX2 = m_pafVertexCoordinates[ uiCountCoords ] - fCenterX;
		fX2 *= fX2;
		fY2 = m_pafVertexCoordinates[ uiCountCoords + 1 ] - fCenterY;
		fY2 *= fY2;
		fZ2 = m_pafVertexCoordinates[ uiCountCoords + 2 ] - fCenterZ;
		fZ2 *= fZ2;
		fDistance2 = fX2 + fY2 + fZ2;
		if( fDistance2 > fMaxDistance2 ) 
		{
			fMaxDistance2 = fDistance2;
		}
	}

	//calculate normalising coefficient such that Xnew = cf * Xold
	float fCoefficient = fBCubeHalfWidth_ / sqrt( fMaxDistance2 );

	//now do normalisation ( use seperate calcs for X,Y,Z so as to use possible
	//parallel floating point units).
	for( uiCountCoords = 0;
		 uiCountCoords < 3 * m_uiNumVertices;
		 uiCountCoords += 3 )
	{
		m_pafVertexCoordinates[ uiCountCoords ] =
			fCoefficient * ( m_pafVertexCoordinates[ uiCountCoords ] - fCenterX );
		m_pafVertexCoordinates[ uiCountCoords + 1 ] =
			fCoefficient * ( m_pafVertexCoordinates[ uiCountCoords + 1 ] - fCenterY );
		m_pafVertexCoordinates[ uiCountCoords + 2 ] =
			fCoefficient * ( m_pafVertexCoordinates[ uiCountCoords + 2 ] - fCenterZ );
	}

}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				AURenMesh::Render
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			25 July 2000
//
// Purpose:					Draws the mesh.
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void AURenMesh::Render(const AUColor* pCol ) const
{
	if( 0 == m_uiNumVertices )
	{
		return;
	}

	const GLfloat pafDiffuseColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat pafSpecularColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat fShininess = 40.0f;


	if( 0 == pCol )
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pafDiffuseColor);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pafDiffuseColor);
	}
	else
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pCol->m_Color.rgba );
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pCol->m_Color.rgba);
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pafSpecularColor);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, fShininess);

	const GLint iNumCoordinatesPerVertex = 3;
	const GLsizei iStride = 0;

	//set up vertex arrays
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glVertexPointer( iNumCoordinatesPerVertex, GL_FLOAT, iStride,
		(const GLvoid*)m_pafVertexCoordinates );
	glNormalPointer( GL_FLOAT, iStride,
		(const GLvoid*)m_pafNormals );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer( 2, GL_FLOAT, iStride,
		(const GLvoid*)m_pafTextureCoordinates );

	//do actual drawing
	glDrawElements( GL_TRIANGLES, 3 * m_uiNumTriangles, GL_UNSIGNED_SHORT, m_pausTriangleIndices );


	//unset vertex arrays
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );

}
