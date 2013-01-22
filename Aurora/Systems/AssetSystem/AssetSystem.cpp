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
#include "../../RuntimeCompiler/FileSystemUtils.h"

AssetSystem::AssetSystem(const char* AssetDirName_)
{
	//search for asset directory
	FileSystemUtils::Path currPath;
	currPath = FileSystemUtils::GetCurrentPath();
    bool bAssetDirFound = false;
    
	//test root and downwards for directory
	while( currPath.HasParentPath() )
	{
		FileSystemUtils::Path testPath = currPath / AssetDirName_;
		if( testPath.Exists() )
		{
            bAssetDirFound = true;
			m_AssetDirectory = testPath.m_string;
			break;
		}
		currPath = currPath.ParentPath();
	}
    
    if( !bAssetDirFound )
    {
        //could be a development build, so test location of this source file and down
        currPath = __FILE__;
		if(currPath.Exists() )
        {
            while( currPath.HasParentPath() )
            {
                FileSystemUtils::Path testPath = currPath / AssetDirName_;
                if( testPath.Exists() )
                {
                    bAssetDirFound = true;
					m_AssetDirectory = testPath.m_string;
                    break;
                }
                currPath = currPath.ParentPath();
            }           
        }
    }
}


AssetSystem::~AssetSystem()
{
	MESHMAP::iterator currMesh = m_Meshes.begin();
	while( currMesh != m_Meshes.end() )
	{
		delete currMesh->second;
		++currMesh;
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
		std::string fileToLoad( pFilename );
		FindFile( fileToLoad );
		AURenMesh* pMesh = new AURenMesh;
		pMesh->LoadFromFile( fileToLoad );
		m_Meshes[ filename ] = pMesh; //use passed in filename for map
		pRenMesh = new AURenderableMesh( pMesh );
	}

	return pRenMesh;

}

void AssetSystem::DestroyRenderableMesh(IAURenderableMesh* pMesh)
{
	delete pMesh;
}

bool AssetSystem::FindFile( std::string& filename )
{
	if( FileSystemUtils::Path( filename ).Exists() )
	{
		return true;
	}

	//else try in asset directory
	FileSystemUtils::Path testpath = m_AssetDirectory;
	testpath = testpath / filename;

	if( testpath.Exists() )
	{
		filename = testpath.m_string;
		return true;
	}

	return false;
}
