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
#ifndef AUVEC3F_DEFINED
#define AUVEC3F_DEFINED

#include <cmath>
#include <limits>


class AUVec3f
{
public:
	AUVec3f(float fX_ = 0.0f, float fY_ = 0.0f, float fZ_ = 0.0f) :
		x( fX_ ),
		y( fY_ ),
		z( fZ_ )
	{

	}

	AUVec3f( const AUVec3f& v3din_ ) : 
		x( v3din_.x ),
		y( v3din_.y ),
		z( v3din_.z )
	{
	}

	//Mutators
	void SetX( float fX_ ) { x = fX_; }
	void SetY( float fY_ ) { y = fY_; }
	void SetZ( float fZ_ ) { z = fZ_; }
	void Set( float fX_, float fY_, float fZ_ )
	{
		x = fX_;
		y = fY_;
		z = fZ_;
	}
	
	float		Dot(	const AUVec3f& V_ ) const
	{
		return x * V_.x
			 + y * V_.y
			 + z * V_.z;
	}
	
	AUVec3f	Cross(	const AUVec3f& V_ ) const
	{
		AUVec3f newV(	y * V_.z - z * V_.y,
						z * V_.x - x * V_.z,
						x * V_.y - y * V_.x );

		return newV;
	}
	
	bool		Normalise()
	{
		float fMagnitude = Magnitude();
		if(fMagnitude != 0.0f)
		{
			x /= fMagnitude;
			y /= fMagnitude;
			z /= fMagnitude;
			return true;
		}
		else
		{
			return false;
		}
	}

	AUVec3f GetNormalised() const
	{
		AUVec3f v = *this;
		v.Normalise();
		return v;
	}

	AUVec3f	Lerp(	const AUVec3f& V_, float t ) const
	{
		return (*this + (V_ - *this) * t);
	}

	bool IsInfinite() const
	{
		float inf = std::numeric_limits<float>::infinity();
		return ( x == inf || y == inf || z == inf );
	}

	void SetInfinite()
	{
		x = y = z = std::numeric_limits<float>::infinity();
	}

	bool IsZero() const
	{
		return (x == 0.0f && y == 0.0f && z == 0.0f);
	}
	
	//Operators
	AUVec3f&	operator+=( const AUVec3f& V_ )
	{
		x += V_.x;
		y += V_.y;
		z += V_.z;
		return *this;
	}
	
	AUVec3f&	operator-=( const AUVec3f& V_ )
	{
		x -= V_.x;
		y -= V_.y;
		z -= V_.z;
		return *this;
	}
	
	AUVec3f&	operator*=( const float& fScalar_ )
	{
		x *= fScalar_;
		y *= fScalar_;
		z *= fScalar_;
		return *this;
	}

	AUVec3f&	operator/=( const float& fScalar_ )
	{
		x /= fScalar_;
		y /= fScalar_;
		z /= fScalar_;
		return *this;
	}
	
	bool operator==( const AUVec3f& V_ ) const
	{
		return ( x == V_.x ) && ( y == V_.y ) && ( z == V_.z );
	}

	bool operator!=( const AUVec3f& V_ ) const
	{
		return !(*this == V_);
	}
	
	float Magnitude() const
	{
		return static_cast<float>( sqrt( Dot( *this ) ) );
	}

	float MagnitudeSqr() const
	{
		return static_cast<float>( Dot( *this ) );
	}

	AUVec3f operator+( const AUVec3f& V2_ ) const
	{
		AUVec3f newV_( *this );
		return ( newV_ += V2_ );
	}

	AUVec3f operator-( const AUVec3f& V2_ ) const
	{
		AUVec3f newV_( *this );
		return ( newV_ -= V2_ );
	}

	AUVec3f operator*( const float& fScalar_ ) const
	{
		AUVec3f newV_( *this );
		return ( newV_ *= fScalar_ );
	}


	AUVec3f operator/( const float& fScalar_ )
	{
		AUVec3f newV_( *this );
		return ( newV_ /= fScalar_ );
	}

	AUVec3f operator-()
	{
		AUVec3f newV_( -x, -y, -z );
		return newV_;
	}

	float x;
	float y;
	float z;
};


//functions which do not have to be members
inline AUVec3f operator*( const float& fScalar_, const AUVec3f& V1_ )
{
	return V1_ * fScalar_;
}

#endif //AUVEC3F_DEFINED
