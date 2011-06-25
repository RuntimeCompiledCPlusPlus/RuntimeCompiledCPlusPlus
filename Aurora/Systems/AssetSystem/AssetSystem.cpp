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

#include "AssetSystem.h"

#include "../../Renderer/AURenMesh.h"

AssetSystem::AssetSystem()
{
}


AssetSystem::~AssetSystem()
{
	MESHMAP::iterator curr = m_Meshes.begin();
	while( curr != m_Meshes.end() )
	{
		delete curr->second;
		++curr;
	}
}

IAURenderableMesh* AssetSystem::CreateRenderableMeshFromFile( const char* pFilename )
{
	IAURenderableMesh* pRenMesh= 0;
	std::string filename( pFilename );
	MESHMAP::iterator found = m_Meshes.find( filename );
	if( found != m_Meshes.end() )
	{
		pRenMesh = new AURenderableMesh( found->second );
	}
	else
	{
		AURenMesh* pMesh = new AURenMesh;
		pMesh->LoadFromFile( filename );
		m_Meshes[ filename ] = pMesh;
		pRenMesh = new AURenderableMesh( pMesh );
	}

	return pRenMesh;

}

void AssetSystem::DestroyRenderableMesh(IAURenderableMesh* pMesh)
{
	delete pMesh;
}