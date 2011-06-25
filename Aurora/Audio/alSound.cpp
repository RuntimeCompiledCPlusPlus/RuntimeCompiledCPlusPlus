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

#include "alSound.h"
#include "alManager.h"

#include <al\al.h>
#include <al\alut.h>

//#define LOGGING_ON


#ifdef LOGGING_ON
#include <fstream>
#include <cstring>
char caALLogFile[] = "LogalSound.txt";
#endif



//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalSound::CalSound
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			26 Jul 2001
//
// Purpose:					Creates the OpenAL sound object.
//
// Inputs:					alBuffer_: buffer to be used
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
CalSound::CalSound( CalBuffer& alBuffer_, bool bLooping_ ) :
	m_albBuffer( alBuffer_ ),  m_bPlayRequested( false ), m_bValid( false ), m_bLooping( bLooping_ ), m_fRefDistance( 10.0f )
{
	CalManager::GetInstance().AddSoundToList( this );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalSound::~CalSound
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			21 Sept 2000
//
// Purpose:					Deletes the OpenAL sound object.
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
CalSound::~CalSound()
{
	UnacquireSource();
	CalManager::GetInstance().RemoveSoundFromList( this );
}



//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalSound::Play
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			21 Sept 2000
//
// Purpose:					Plays the sound at the specified position and velocity.
//
// Inputs:					v3dPosition_: position to play sound at.
//							v3dVelocity_: velocity to play sound at.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void CalSound::Play	(	const AUVec3f&		v3dPosition_,
						const AUVec3f&		v3dVelocity_)
{
	m_v3dPosition = v3dPosition_;
	m_v3dVelocity = v3dVelocity_;
	m_bPlayRequested = true;

	if( true == m_bValid )
	{
		ALfloat palfSourcePos[] = {	v3dPosition_.x,
									v3dPosition_.y,
									v3dPosition_.z };
		ALfloat palfSourceVel[] = {	v3dVelocity_.x,
									v3dVelocity_.y,
									v3dVelocity_.z };

		alSourcefv(	m_aluiSource, AL_POSITION,		palfSourcePos);
		alSourcefv(	m_aluiSource, AL_VELOCITY,		palfSourceVel);
		alSourcef(	m_aluiSource, AL_PITCH,			CalManager::GetInstance().GetGlobalPitch() );


		alSourcePlay( m_aluiSource );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalSound::Play
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			21 Sept 2000
//
// Purpose:					Plays the sound at the specified position.
//
// Inputs:					v3dPosition_: position to play sound at.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void CalSound::Play	(	const AUVec3f&		v3dPosition_ )
{
	m_v3dPosition = v3dPosition_;
	m_bPlayRequested = true;

	if( true == m_bValid )
	{
		ALfloat palfSourcePos[] = {	v3dPosition_.x,
									v3dPosition_.y,
									v3dPosition_.z };

		alSourcefv(	m_aluiSource, AL_POSITION,		palfSourcePos);
		alSourcef(	m_aluiSource, AL_PITCH,			CalManager::GetInstance().GetGlobalPitch() );

		alSourcePlay( m_aluiSource );
	}

}
//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalSound::AcquireSource
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			17 August 2001
//
// Purpose:					Acquires a source for the sound, and if play is requested starts playing.
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void CalSound::AcquireSource()
{
	#ifdef LOGGING_ON
	{
		ofstream ofLogFile(caALLogFile, ios::app);
		ofLogFile << "CalSound::AcquireSource called" << endl;
	}
	#endif

	if( !m_albBuffer.GetIsValid() )
	{
		//should log this
		return;
	}

	if( !m_bValid )
	{
		//do not already have a source, so acquire one


		alGenSources( 1, &m_aluiSource );
		if( AL_NO_ERROR != alGetError() )
		{
			//should log this
			#ifdef LOGGING_ON
			{
				ofstream ofLogFile(caALLogFile, ios::app);
				ofLogFile << "CalSound::AcquireSource - cannot generate a source, produces error." << endl;
			}
			#endif
			return;
		}

		m_bValid = true;	//managed to set up source.

		ALfloat palfSourcePos[] = {		m_v3dPosition.x,
										m_v3dPosition.y,
										m_v3dPosition.z };
		ALfloat palfSourceVel[] = {		m_v3dVelocity.x,
										m_v3dVelocity.y,
										m_v3dVelocity.z };


		//alSourcef(	m_aluiSource, AL_PITCH,		1.0f );
		//alSourcef(	m_aluiSource, AL_GAIN,		1.0f );
		alSourcef(	m_aluiSource, AL_REFERENCE_DISTANCE, m_fRefDistance );

		alSourcefv(	m_aluiSource, AL_POSITION,	palfSourcePos);
		alSourcefv(	m_aluiSource, AL_VELOCITY,	palfSourceVel);
		alSourcef(	m_aluiSource, AL_PITCH,		CalManager::GetInstance().GetGlobalPitch() );
		alSourcei(	m_aluiSource, AL_BUFFER,	m_albBuffer.GetBuffer() );
		if( true == m_bLooping )
		{
			alSourcei(	m_aluiSource, AL_LOOPING,	AL_TRUE);
		}
		else
		{
			alSourcei(	m_aluiSource, AL_LOOPING,	AL_FALSE);
		}
/*		if( AL_NO_ERROR != alGetError() )
		{
			return;
		} */

	}

	if( m_bPlayRequested )
	{
		alSourcePlay( m_aluiSource );
	}

	#ifdef LOGGING_ON
	{
		ofstream ofLogFile(caALLogFile, ios::app);
		ofLogFile << "CalSound::AcquireSource Exiting." << endl;
	}
	#endif

}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalSound::UnacquireSource
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			21 Sept 2000
//
// Purpose:					Deletes the OpenAL sound object.
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void CalSound::UnacquireSource()
{
	#ifdef LOGGING_ON
	{
		ofstream ofLogFile(caALLogFile, ios::app);
		ofLogFile << "CalSound::~CalSound Entering and deleteing source." << endl;
	}
	#endif

	if( m_bValid )
	{
		alDeleteSources ( 1 , &m_aluiSource );
		m_bValid = false;
	}

	#ifdef LOGGING_ON
	{
		ofstream ofLogFile(caALLogFile, ios::app);
		ofLogFile << "CalSound::~CalSound Exiting." << endl;
	}
	#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalSound::SetPosition
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			19 October 2000
//
// Purpose:					Changes the specified position.
//
// Inputs:					v3dPosition_: position to move sound to.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void CalSound::SetPosition(	const AUVec3f&		v3dPosition_ )
{
	if( m_v3dPosition == v3dPosition_ )
	{
		return;
	}
	m_v3dPosition = v3dPosition_;

	if( true == m_bValid )
	{
		ALfloat palfSourcePos[] = {	v3dPosition_.x,
									v3dPosition_.y,
									v3dPosition_.z };

		alSourcefv(	m_aluiSource, AL_POSITION,		palfSourcePos);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalSound::SetPosition
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			20 October 2000
//
// Purpose:					Changes the specified position and velocity.
//
// Inputs:					v3dPosition_: position to move sound to.
//							v3dVelocity_: velocity to play sound at.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void CalSound::SetPosition(	const AUVec3f&		v3dPosition_,
							const AUVec3f&		v3dVelocity_)
{
	if( m_v3dPosition == v3dPosition_ && m_v3dVelocity == v3dVelocity_ )
	{
		return;
	}
	m_v3dPosition = v3dPosition_;
	m_v3dVelocity = v3dVelocity_;

	if( true == m_bValid )
	{
		ALfloat palfSourcePos[] = {	v3dPosition_.x,
									v3dPosition_.y,
									v3dPosition_.z };

		ALfloat palfSourceVel[] = {	v3dVelocity_.x,
									v3dVelocity_.y,
									v3dVelocity_.z };
		alSourcefv(	m_aluiSource, AL_POSITION,		palfSourcePos);
		alSourcefv(	m_aluiSource, AL_VELOCITY,		palfSourceVel);
	}
}

void CalSound::SetReferenceDistance( float distance )
{
	m_fRefDistance = distance;
	if( true == m_bValid )
	{
		alSourcef(	m_aluiSource, AL_REFERENCE_DISTANCE, m_fRefDistance );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalSound::Stop
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			20 October 2000
//
// Purpose:					Stops playing the sound
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void CalSound::Stop()
{
	if( true == m_bValid )
	{
		alSourceStop( m_aluiSource );
	}
	m_bPlayRequested = false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalSound::Stop
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			20 October 2000
//
// Purpose:					Calculates the distance to the listener
//
// Inputs:					None.
//							
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
void CalSound::CalcDistToListenerup2()
{
	AUVec3f v3dDtoL = CalManager::GetInstance().GetListenerPosition() - m_v3dPosition;
	m_fDistToListenerUp2 = v3dDtoL.Dot( v3dDtoL );
}

