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
#ifndef AUDORIENTATION3D_DEFINED
#define AUDORIENTATION3D_DEFINED

#include "AUVec3f.inl"	//interface relies on class AUVec3f
#include <limits>	//for numerical limits


class AUOrientation3D  
{
public:
	AUOrientation3D() :
		m_v3dForwards(	1.0f, 0.0f, 0.0f ),
		m_v3dUp(		0.0f, 1.0f, 0.0f ),
		m_v3dRight(		0.0f, 0.0f, 1.0f )
	{
	}
	
	AUOrientation3D(	const AUVec3f& v3dForwards_, const AUVec3f& v3dUp_) :
		m_v3dForwards( v3dForwards_ ),
		m_v3dUp( v3dUp_ )
	{
		OrthoNormalise();
	}
	
	//Inspectors
	const AUVec3f& GetForwards()	const { return m_v3dForwards; }
	const AUVec3f& GetUp()		const { return m_v3dUp; }
	const AUVec3f& GetRight()		const { return m_v3dRight; }

	void LoadglViewMatrix(		float *const fglMatrix_ ) const
	{
		//note: OpenGL camera defaults to looking down -z, thus orient z with Forwards
		fglMatrix_[0] = m_v3dRight.x;
		fglMatrix_[1] = m_v3dUp.x;
		fglMatrix_[2] = -m_v3dForwards.x;
		fglMatrix_[3] = 0.0f;

		fglMatrix_[4] = m_v3dRight.y;
		fglMatrix_[5] = m_v3dUp.y;
		fglMatrix_[6] = -m_v3dForwards.y;
		fglMatrix_[7] = 0.0f;

		fglMatrix_[8] = m_v3dRight.z;
		fglMatrix_[9] = m_v3dUp.z;
		fglMatrix_[10] = -m_v3dForwards.z;
		fglMatrix_[11] = 0.0f;

		fglMatrix_[12] = 0.0f;
		fglMatrix_[13] = 0.0f;
		fglMatrix_[14] = 0.0f;
		fglMatrix_[15] = 1.0f;
	}
	
	void LoadglObjectMatrix(	float *const fglMatrix_ ) const
	{
		//note: OpenGL camera defaults to looking down -z, thus orient z with Forwards
		fglMatrix_[0] = m_v3dForwards.x;
		fglMatrix_[1] = m_v3dForwards.y;
		fglMatrix_[2] = m_v3dForwards.z;
		fglMatrix_[3] = 0.0f;

		fglMatrix_[4] = m_v3dUp.x;
		fglMatrix_[5] = m_v3dUp.y;
		fglMatrix_[6] = m_v3dUp.z;
		fglMatrix_[7] = 0.0f;

		fglMatrix_[8] = m_v3dRight.x;
		fglMatrix_[9] = m_v3dRight.y;
		fglMatrix_[10] = m_v3dRight.z;
		fglMatrix_[11] = 0.0f;

		fglMatrix_[12] = 0.0f;
		fglMatrix_[13] = 0.0f;
		fglMatrix_[14] = 0.0f;
		fglMatrix_[15] = 1.0f;
	}

	//Mutators
	AUVec3f& GetForwards()	{ return m_v3dForwards; }
	AUVec3f& GetUp()			{ return m_v3dUp; }
	AUVec3f& GetRight()		{ return m_v3dRight; }
	
	void Set(		const AUVec3f& v3dForwards_,	const AUVec3f& v3dUp_)
	{
		m_v3dForwards = v3dForwards_;
		m_v3dUp = v3dUp_;
		OrthoNormalise();
	}
	
	void Rotate(	const AUVec3f& v3dAxis_,		float fTheta_)
	{
		float fCosTheta = static_cast<float>( cos(fTheta_ ) );
		float fSinTheta = static_cast<float>( sin(fTheta_ ) );

		AUVec3f v3dtemp = v3dAxis_.Cross( m_v3dForwards );
		m_v3dForwards += fSinTheta * v3dtemp +
			( fCosTheta - 1.0f ) * v3dAxis_.Cross( v3dtemp );
		v3dtemp = v3dAxis_.Cross( m_v3dUp );
		m_v3dUp		  += fSinTheta * v3dtemp +
			( fCosTheta - 1.0f )  * v3dAxis_.Cross( v3dtemp );

		//orthonormalise coordinate system
		OrthoNormalise();
	}

protected:
	void OrthoNormalise()
	{
		if( true == m_v3dForwards.Normalise() )
		{
			//have a normalised forwards vector
			m_v3dRight = m_v3dForwards.Cross( m_v3dUp );
			if( true == m_v3dRight.Normalise() )
			{
				//and now have a normalised right vector so safe to generate cross.
				m_v3dUp = m_v3dRight.Cross( m_v3dForwards );
			}
			else
			{
				//have a forwards vector only, so generate an arbitary `up'.
				m_v3dUp.SetX( m_v3dForwards.z );
				m_v3dUp.SetY( m_v3dForwards.x );
				m_v3dUp.SetZ( m_v3dForwards.y );

				//will now get a 'guaranteed' right from this
				m_v3dRight = m_v3dForwards.Cross( m_v3dUp );

				//and so can generate a true up
				m_v3dUp = m_v3dRight.Cross( m_v3dForwards );
			}
		}
		else
		{
			//can't use forwards as our main vector, so try up
			if( true == m_v3dUp.Normalise() )
			{
				//have a up vector only, so generate an arbitary `up'.
				m_v3dForwards.SetX( m_v3dUp.z );
				m_v3dForwards.SetY( m_v3dUp.x );
				m_v3dForwards.SetZ( m_v3dUp.y );

				//will now get a 'guaranteed' right from this
				m_v3dRight = m_v3dForwards.Cross( m_v3dUp );

				//and so can generate a true forwards
				m_v3dForwards = m_v3dUp.Cross( m_v3dRight );
			}
			else
			{
				//have no appropriate starting vectors, so fake it.
				m_v3dForwards.Set(	1.0f, 0.0f, 0.0f ); 
				m_v3dUp.Set(		0.0f, 1.0f, 0.0f ); 
				m_v3dRight.Set(		0.0f, 0.0f, 1.0f ); 
			}

		}
	}

private:
	AUVec3f m_v3dForwards;
	AUVec3f m_v3dUp;
	AUVec3f m_v3dRight;

};

#endif //AUDORIENTATION3D_DEFINED