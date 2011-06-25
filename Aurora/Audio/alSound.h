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
// File:				"alSound.h"
//
// Related Source File:		"alSound.cpp"
//
// Original Author:			Douglas John Binks (DJB)
//
// Creation Date:			21 Sept 2000
//
// Purpose:					Defines the interface for the CalSound Class, which
//							handles OpenAL sound object loading, using wav
//							files.
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef ALSOUNDDEF
#define ALSOUNDDEF


#include "alBuffer.h"
#include "../Common/AUVec3f.inl"
 

class CalSound  
{
public:
	CalSound( CalBuffer& m_albBuffer, bool bLooping_ = false );
	virtual ~CalSound();

	void Play(					const AUVec3f&		v3dPosition_);
	void Play(					const AUVec3f&		v3dPosition_,
								const AUVec3f&		v3dVelocity_);
	void SetPosition(			const AUVec3f&		v3dPosition_);
	void SetPosition(			const AUVec3f&		v3dPosition_,
								const AUVec3f&		v3dVelocity_);
	void SetReferenceDistance( float distance );
	void AcquireSource();
	void UnacquireSource();
	void CalcDistToListenerup2();

	const AUVec3f&	GetPosition()			const { return m_v3dPosition; }
	bool				GetIsValid()			const { return m_bValid; }
	bool				GetIsPlayRequested()	const { return m_bPlayRequested; }
	bool				GetIsLooping()			const { return m_bLooping; }
	float				GetDistToListenerUp2()	const { return m_fDistToListenerUp2; }

	unsigned int GetALSource() const
	{
		return m_aluiSource;
	}

	void Stop();

private:
	unsigned int m_aluiSource;
	bool m_bValid;
	bool m_bPlayRequested;
	bool m_bLooping;

	AUVec3f		m_v3dPosition;
	AUVec3f		m_v3dVelocity;
	float			m_fDistToListenerUp2;

	CalBuffer m_albBuffer;
	float		m_fRefDistance;

};

#endif