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

#include "ICameraControl.h"

#include "../../Common/Math.inl"
#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/IAssetSystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"
#include "../../Systems/IGame.h"

#include <cmath>
#include <assert.h>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// We serialize all the current camera values if any have been changed 
// via ICameraControl interface, otherwise we reload the initial values
// below, allowing them to be experimented with at runtime
//////////////////////////////////////////////////////////////////////////


// Initial Values
static const AUVec3f initialPos = AUVec3f( 0.0f, -1000.0f, 0.0f );
static const float initialMoveSpeed = 10.0f;


class CameraControl: public ICameraControl
{
public:
	CameraControl() 
	{
		m_bModified = false;

		m_currentPos = initialPos;
		m_targetPos = initialPos;
		m_fMoveSpeed = initialMoveSpeed;

		m_fWindowWidth = 0;
		m_fWindowHeight = 0;
	}

	virtual ~CameraControl()
	{
		if( m_pEntity )
		{
			m_pEntity->SetUpdateable(NULL);
		}
	}


	// IEntityObject

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		AU_ASSERT(pSerializer);
		IEntityObject::Serialize(pSerializer);
		SERIALIZE(m_bModified);
		if (m_bModified)
		{
			SERIALIZE(m_currentPos);
			SERIALIZE(m_targetPos);
			SERIALIZE(m_fMoveSpeed);
		}
	}

	virtual void Init( bool isFirstInit )
	{
		m_pEntity->SetUpdateable( this );

		PerModuleInterface::g_pSystemTable->pGame->GetWindowSize( m_fWindowWidth, m_fWindowHeight );
	}

	// ~IEntityObject

	// IAUUpdateable

	virtual void Update( float deltaTime )
	{
		if (m_currentPos != m_targetPos)
		{
			float fDist = (m_targetPos - m_currentPos).Magnitude();
			float fFrameDist = std::min(fDist, m_fMoveSpeed * deltaTime);
			float fInterp = fFrameDist / fDist;
			m_currentPos = m_currentPos.Lerp(m_targetPos, fInterp);
		}
	}

	// ~IAUUpdateable

	// ICameraControl

	virtual AUVec3f GetCurrentPos() const
	{
		return m_currentPos;
	}

	virtual AUVec3f GetTargetPos() const
	{
		return m_targetPos;
	}

	virtual void SetTargetPos(const AUVec3f& pos)
	{
		m_targetPos = pos;
		m_bModified = true;
	}

	virtual float GetMoveSpeed() const
	{
		return m_fMoveSpeed;
	}

	virtual void SetMoveSpeed(float fSpeed)
	{
		m_fMoveSpeed = std::max(0.f, fSpeed);
		m_bModified = true;
	}

	virtual AUVec3f Project( const AUVec3f& worldPos ) const
	{
		// Camera is simple orthographic, so projection operation is just a translation
		// and a conversion of +z to -y (due to the flip of world space Z to screen space y)
		AUVec3f screenPos = AUVec3f( worldPos.x + m_fWindowWidth * 0.5f - m_currentPos.x,
		                             -( worldPos.z - m_fWindowHeight * 0.5f - m_currentPos.z ),
																 0.0f );

		return screenPos;
	}

	virtual AUVec3f Unproject( const AUVec3f& screenPos ) const
	{
		// Camera is simple orthographic, so unprojection operation is just a translation
		// and a conversion of +y to -z (due to the flip of screen space y to world space Z)
		AUVec3f worldPos = AUVec3f( screenPos.x - m_fWindowWidth * 0.5f + m_currentPos.x,
		                            0.0f,
			                          -( screenPos.y - m_fWindowHeight * 0.5f - m_currentPos.z ) );

		return worldPos;
	}

	// ~ICameraControl


private:

	bool m_bModified;
	AUVec3f m_currentPos;
	AUVec3f m_targetPos;
	float m_fMoveSpeed;

	float m_fWindowWidth;
	float m_fWindowHeight;
};

REGISTERCLASS(CameraControl);




