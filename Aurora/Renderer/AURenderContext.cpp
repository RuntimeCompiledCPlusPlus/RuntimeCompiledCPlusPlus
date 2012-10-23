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

#include "AURenderContext.h"

#include "IAURenderable.h"

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


AURenderContext::AURenderContext()
{
}


AURenderContext::~AURenderContext()
{
}

AUDynArray<AUEntityId> g_entities;

void AURenderContext::Render( IEntitySystem* pEntitySystem )
{
	//loop through setting matrices and calling render.
	pEntitySystem->GetAll( g_entities );

	for( size_t i = 0; i < g_entities.Size(); ++i )
	{

		IAUEntity* pEntity = pEntitySystem->Get( g_entities[ i ] );
		IAURenderable* pRenderable = pEntity->GetRenderable();
		if( pRenderable )
		{
			glPushMatrix();

			glMatrixMode(GL_MODELVIEW);

			// Translation
			const AUVec3f& t = pEntity->GetPosition();
			glTranslatef(	t.x, t.y, t.z );
				
			// Rotation
			float fglMatrix[16];
			pEntity->GetOrientation().LoadglObjectMatrix(fglMatrix);
			glMultMatrixf(fglMatrix);

			// Scale
			glEnable(GL_NORMALIZE); // Needed so normals don't get wrecked by scaling - not sure how costly it is though
			const AUVec3f& s = pEntity->GetScale();
			glScalef(	s.x, s.y, s.z );

			pRenderable->Render();
			glPopMatrix();
		}
	}

}