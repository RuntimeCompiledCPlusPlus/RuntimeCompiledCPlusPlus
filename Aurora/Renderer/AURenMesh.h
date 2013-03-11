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
#ifndef AURENDMESH_DEF
#define AURENDMESH_DEF

#include <string>
#include "../Common/AUColor.inl"
#include "IAURenderable.h"

struct aiScene;

class AURenMesh  
{
public:
	AURenMesh();
	bool LoadFromFile( const std::string& strFilename );
	bool SaveToFile( const std::string& strFilename ); // Saves as AML
	virtual ~AURenMesh();

	void NormaliseToBCubeHalfWidth( float fBCubeHalfWidth );
	void Render( const AUColor* pCol = 0 ) const;

protected:
	bool LoadFromFileAML( const std::string& strFilename ); // For native AML format
	bool LoadFromFileImport( const std::string& strFilename ); // Use AssImp library for other formats
	void ProcessScene( const aiScene* pScene ); // Convert AssImp imported scene into internal data structures
	void Clear();
	float* m_pafVertexCoordinates;
	float* m_pafTextureCoordinates;
	float* m_pafNormals;
	unsigned short* m_pausTriangleIndices;
	unsigned int m_uiNumVertices;
	unsigned int m_uiNumTriangles;

};

class AURenderableMesh : public IAURenderableMesh
{
public:
	AURenderableMesh( AURenMesh* pMesh ) :
		m_pMesh( pMesh ),
		m_Color( 1.0f, 1.0f, 1.0f, 1.0f )
	{
	}

	virtual const AUColor& GetColor() const
	{
		return m_Color;
	}
	virtual void SetColor( const AUColor& color )
	{
		m_Color = color;
	}

	virtual void Render() const
	{
		m_pMesh->Render( &m_Color );
	}

	AURenMesh* GetMesh()
	{
		return m_pMesh;
	}
private:
	AURenMesh*	m_pMesh;
	AUColor		m_Color;
};

#endif //AURENDMESH_DEF
