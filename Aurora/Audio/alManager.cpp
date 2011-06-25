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
// File:							"alManager.cpp"
//
// Related Source/Header File:		"alManager.h"
//
// Original Author:					Douglas John Binks (DJB)
//
// Creation Date:					26 July 2001
//
// Purpose:
//	Singleton class which handles 'global' OpenAL management tasks, such as initialisation,
//	listener control and source play control.
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "alManager.h"
#include <algorithm>
#include <al\al.h>
#include <al\alut.h>

using namespace std;

CalManager* CalManager::ms_pSingleton = 0;

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalManager::CalManager
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			26 Jul 2001
//
// Purpose:					Constructor. Initialises OpenAL
//
// Inputs:					None.
//
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
CalManager::CalManager()
	: m_bEnabled( true )
	, m_bValid( false )
	, m_Pitch( 1.0f )
{

	//initialise OpenAL;
	alutInit( 0, 0 );
	if( AL_NO_ERROR != alGetError() )
	{
		//should log this
		return;
	}

	//set up some default global attributes
	alDopplerVelocity ( 0.2f );

	//set up listener position at a default location.
	ALfloat listenerPos[] = {0.0,0.0,0.0};
	ALfloat listenerVel[] = {0.0,0.0,0.0};
	ALfloat	listenerOri[] = {0.0,0.0,1.0, 0.0,1.0,0.0};
	alListenerfv( AL_POSITION,		listenerPos);
	alListenerfv( AL_VELOCITY,		listenerVel);
	alListenerfv( AL_ORIENTATION,	listenerOri);

	//if we've got this far OpenAL initialisation is OK
	m_bValid = true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalManager::~CalManager
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			26 Jul 2001
//
// Purpose:					Destructor.
//
// Inputs:					None.
//
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
CalManager::~CalManager()
{
	if( true == m_bValid )
	{
		alutExit();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalManager::SetListener
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			28 Aug 2001
//
// Purpose:					Set up the listener position, velocity and orientation,
//							and then ensures that nearby sounds are played.
//
// Inputs:					
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void CalManager::SetListener(	const AUVec3f&		v3dPosition_,
								const AUVec3f&		v3dVelocity_,
								const AUOrientation3D&	o3dOrientation_)
{
	m_v3dListenerPosition = v3dPosition_;
	if( m_bValid )
	{

		ALfloat palfListenerPos[] = {	v3dPosition_.x,
										v3dPosition_.y,
										v3dPosition_.z };
		ALfloat palfListenerVel[] = {	v3dVelocity_.x,
										v3dVelocity_.y,
										v3dVelocity_.z };
		ALfloat	palfListenerOri[] = {	o3dOrientation_.GetForwards().x,
										o3dOrientation_.GetForwards().y,
										o3dOrientation_.GetForwards().z,
										o3dOrientation_.GetUp().x,
										o3dOrientation_.GetUp().y,
										o3dOrientation_.GetUp().z };

		alListenerfv( AL_POSITION,		palfListenerPos);
		alListenerfv( AL_VELOCITY,		palfListenerVel);
		alListenerfv( AL_ORIENTATION,	palfListenerOri);
		
		if( m_bEnabled )
		{
			RecalcSoundDistToListener();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalManager::RecalcSoundDistToListener
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			17 Aug 2001
//
// Purpose:					Recalculates the distance from the sound to the listener
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void CalManager::RecalcSoundDistToListener()
{
	soundlist::iterator ilalsSoundToPlay = m_lpalsSoundList.begin();
	while( ilalsSoundToPlay != m_lpalsSoundList.end() )
	{
		(*ilalsSoundToPlay)->CalcDistToListenerup2();
		++ilalsSoundToPlay;
	}
}

void CalManager::SetVolume( float Volume )
{
	//0.0f-1.0f value
	alListenerf( AL_GAIN, Volume);
}

void CalManager::SetGlobalPitch( float pitch )
{
	m_Pitch = pitch;
	soundlist::iterator ilalsSoundToPlay = m_lpalsSoundList.begin();
	while( ilalsSoundToPlay != m_lpalsSoundList.end() )
	{
		if( (*ilalsSoundToPlay)->GetIsPlayRequested() && (*ilalsSoundToPlay)->GetIsValid() )
		{
			alSourcef(	(*ilalsSoundToPlay)->GetALSource(), AL_PITCH, m_Pitch);
		}
		++ilalsSoundToPlay;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalManager::PlayRequestedSounds
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			17 Aug 2001
//
// Purpose:					Ensures that nearby sounds are played.
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void CalManager::PlayRequestedSounds()
{
	if( 0 == m_lpalsSoundList.size()  || false == m_bEnabled )
	{
		//not enabled or no requested sounds in list so return.
		return;
	}

	sort( m_lpalsSoundList.begin(), m_lpalsSoundList.end(), CalSoundPointerCompare() );

	bool bNoMoreFreeSources = false;

	soundlist::iterator ilalsSoundToAcquire = m_lpalsSoundList.end();
	do
	{
		--ilalsSoundToAcquire;
		if( (*ilalsSoundToAcquire)->GetIsPlayRequested() && !(*ilalsSoundToAcquire)->GetIsValid() )
		{
			//sound is requested to be played, but has not got a source.
			
			if( false == bNoMoreFreeSources )
			{
				//attach a source
				(*ilalsSoundToAcquire)->AcquireSource();
			}

			if( ( false == (*ilalsSoundToAcquire)->GetIsValid() ) &&
				( ilalsSoundToAcquire != m_lpalsSoundList.begin() ) )
			{
				bNoMoreFreeSources = true;

				//still do not have a source, so must steal one
				//look to see if there is a later sound which has a source.
				soundlist::iterator ilalsSoundToUnacquire = m_lpalsSoundList.begin();
				while( ilalsSoundToUnacquire != ilalsSoundToAcquire )
				{
					if( (*ilalsSoundToUnacquire)->GetIsValid() )
					{
						(*ilalsSoundToUnacquire)->UnacquireSource();
						(*ilalsSoundToAcquire)->AcquireSource();

						if( false == (*ilalsSoundToUnacquire)->GetIsLooping() )
						{
							(*ilalsSoundToUnacquire)->Stop();	//stop the playing
						}

						break;
					}
					++ilalsSoundToUnacquire;
				}

				if( ilalsSoundToUnacquire == ilalsSoundToAcquire )
				{
					//have reached the end of the list and there are no sources to steal, so exit loop
					break;
				}
			}

		}
	} while( ilalsSoundToAcquire != m_lpalsSoundList.begin() );
}


//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalManager::SetIsEnabled
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			28 Aug 2001
//
// Purpose:					Enables / Disables sound playing.
//
// Inputs:					bEnable_ : true to enable, false to disable.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void CalManager::SetIsEnabled( bool bEnable_ )
{

	if( false == bEnable_ )
	{
		soundlist::iterator ilalsSoundToPlay = m_lpalsSoundList.begin();
		while( ilalsSoundToPlay != m_lpalsSoundList.end() )
		{
			(*ilalsSoundToPlay)->Stop();
			++ilalsSoundToPlay;
		}
	}

	m_bEnabled = bEnable_;
}
