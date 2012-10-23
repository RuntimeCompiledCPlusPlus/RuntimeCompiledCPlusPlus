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
#include "../../Audio/alManager.h"
#include "../../Audio/alSound.h"

#define BOOST_FILESYSTEM_VERSION 3
#include "boost/filesystem.hpp"
using namespace boost::filesystem;

AssetSystem::AssetSystem(const char* AssetDirName_)
{
	//search for asset directory
	path currPath;
	currPath = current_path();
    bool bAssetDirFound = false;
    
	//test root and downwards for directory
	while( currPath.has_parent_path() )
	{
		path testPath = currPath / AssetDirName_;
		if( exists( testPath ) )
		{
            bAssetDirFound = true;
			m_AssetDirectory = testPath.string();
			break;
		}
		currPath = currPath.parent_path();
	}
    
    if( !bAssetDirFound )
    {
        //could be a development build, so test location of this source file and down
        currPath = __FILE__;
        if( exists( currPath ))
        {
            while( currPath.has_parent_path() )
            {
                path testPath = currPath / AssetDirName_;
                if( exists( testPath ) )
                {
                    bAssetDirFound = true;
                    m_AssetDirectory = testPath.string();
                    break;
                }
                currPath = currPath.parent_path();
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

#ifndef NOALSOUND
	ALBUFFERMAP::iterator currSound = m_AlBuffers.begin();
	while( currSound != m_AlBuffers.end() )
	{
		delete currSound->second;
		++currSound;
	}
#endif

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

CalSound* AssetSystem::CreateSoundFromFile( const char* pFilename, bool looping )
{
	CalSound* pSound= 0;
#ifndef NOALSOUND
	std::string filename( pFilename );
	ALBUFFERMAP::iterator found = m_AlBuffers.find( filename );
	if( found != m_AlBuffers.end() )
	{
		pSound = new CalSound( *(found->second), looping );
	}
	else
	{
		std::string fileToLoad( pFilename );
		FindFile( fileToLoad );
		CalBuffer* pBuffer = new CalBuffer( fileToLoad );
		m_AlBuffers[ filename ] = pBuffer; //use passed in filename for map
		pSound = new CalSound( *pBuffer, looping );
	}
#endif
	return pSound;
}

void AssetSystem::DestroySound( CalSound* pSound )
{
#ifndef NOALSOUND
	delete pSound;
#endif
}

bool AssetSystem::FindFile( std::string& filename )
{
	if( exists( filename ) )
	{
		return true;
	}

	//else try in asset directory
	path testpath = m_AssetDirectory;
	testpath /= filename;

	if( exists( testpath ) )
	{
		filename = testpath.string();
		return true;
	}

	return false;
}
