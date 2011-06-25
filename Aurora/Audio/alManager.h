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
// File:							"alManager.h"
//
// Related Source/Header File:		"alManager.cpp"
//
// Original Author:					Douglas John Binks (DJB)
//
// Creation Date:					26 July 2001
//
// Purpose:
//	Singleton class which handles 'global' OpenAL management tasks, such as initialisation,
//	listener control and source play control.
//
// Initialise by setting initial positon info:
//  CalManager::GetInstance().SetListener(	listenerPos, listenerVeolocity, listenerOrientation );
//
// Every frame request to play sounds, and update position info as above before if needed:
//  CalManager::GetInstance().PlayRequestedSounds();
//
// Shutdown code:
//	CalManager::CleanUp();
//	CalManager::GetInstance().SetIsEnabled( false );	//disable sounds
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef alManagerDEF
#define alManagerDEF


#include "../Common/AUOrientation3D.inl"
#include "../Common/AUVec3f.inl"
#include "alSound.h"
#include <vector>

typedef std::vector<CalSound*> soundlist;

class CalManager
{
public:
	inline static CalManager& GetInstance()
	{
		//note: in a multithreaded environment, this should be protected
		//with a critical section.
		if( !ms_pSingleton ) { ms_pSingleton = new CalManager; }
		return *ms_pSingleton;
	}

	//Cleans up the sound manager by deleting singleton
	inline static void CleanUp()
	{
		delete ms_pSingleton;
		ms_pSingleton = 0;
	}

	~CalManager();

	void SetListener(		const AUVec3f&		v3dPosition_,
							const AUVec3f&		v3dVelocity_,
							const AUOrientation3D&	o3dOrientation_);
	void PlayRequestedSounds();

	inline bool GetIsValid() const { return m_bValid; }

	inline void AddSoundToList( CalSound* pSound )
	{
		m_lpalsSoundList.push_back( pSound );
	}
	inline void RemoveSoundFromList( CalSound* pSound )
	{
		//m_lpalsSoundList.remove( pSound );
		soundlist::iterator ilalsSoundToPlay = m_lpalsSoundList.begin();
		while( ilalsSoundToPlay != m_lpalsSoundList.end() )
		{
			if( *ilalsSoundToPlay == pSound )
			{
				m_lpalsSoundList.erase( ilalsSoundToPlay );
				return;
			}
			++ilalsSoundToPlay;
		}
	}

	void SetVolume( float Volume );

	void SetGlobalPitch( float pitch );
	float GetGlobalPitch() const
	{
		return m_Pitch;
	}

	inline const AUVec3f& GetListenerPosition() const { return m_v3dListenerPosition; }

	void SetIsEnabled( bool bEnable_ );
	inline const bool GetIsEnabled() { return m_bEnabled; }

private:
	CalManager();
	void RecalcSoundDistToListener();


	static CalManager* ms_pSingleton;
	bool m_bValid;
	bool m_bEnabled;
	AUVec3f m_v3dListenerPosition;

	soundlist m_lpalsSoundList;
	float m_Pitch;
};

//less than through pointer for sound
class CalSoundPointerCompare
{
public:
	bool operator()(const CalSound* pSoundL, const CalSound* pSoundR ) const
	{
		return pSoundL->GetDistToListenerUp2() > pSoundR->GetDistToListenerUp2();
	}
};


#endif