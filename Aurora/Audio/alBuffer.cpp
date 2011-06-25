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
// File:							"alBuffer.cpp"
//
// Related Source/Header File:		"alBuffer.h"
//
// Original Author:					Douglas John Binks (DJB)
//
// Creation Date:					26 July 2001
//
// Purpose:
//	Handles OpenAL sound object loading into an OpenAL buffer, using wav files.
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "alBuffer.h"
#include "alManager.h"
#include <al\al.h>
#include <al\alut.h>

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalBuffer::CalBuffer
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			26 Jul 2001
//
// Purpose:					Constructor. Loads wav file into buffer
//
// Inputs:					strWavFileName_: name of wav file to use.
//
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
CalBuffer::CalBuffer( const string& strWavFileName_ )
{

	m_bValid = false;		//start with an invalid buffer

	//initialise the number of references
	m_puiNumReferences = new unsigned int;
	*m_puiNumReferences = 1;

	//check OpenAL initialisation
	if( !CalManager::GetInstance().GetIsValid() )
	{
		//should log this
		return;
	}

	//load sound into a sound buffer
	alGenBuffers( 1, &m_aluiBuffer);
	if( AL_NO_ERROR != alGetError() )
	{
		//should log this
		return;
	}

	ALsizei alsiSize, alsiFreq;
	ALenum	aleFormat;
	//ALsizei	aleFormat;
	ALvoid* palvData;
	ALboolean albLooping;
	//ALsizei	alsiBits;

	//need non const char* ... so cast
	char* paWavFileName = const_cast<char*>( strWavFileName_.c_str() );


	alutLoadWAVFile( paWavFileName, &aleFormat, &palvData, &alsiSize, &alsiFreq, &albLooping );
	//alutLoadWAV( paWavFileName, &palvData, &aleFormat, &alsiSize, &alsiFreq, &alsiBits );
	if( AL_NO_ERROR != alGetError() )
	{
		//should log this
		alDeleteBuffers(1, &m_aluiBuffer);
		return;
	}


	alBufferData(	m_aluiBuffer, aleFormat, palvData, alsiSize, alsiFreq);
	if( AL_NO_ERROR != alGetError() )
	{
		//should log this
		alDeleteBuffers(1, &m_aluiBuffer);
		return;
	}

	alutUnloadWAV(				aleFormat, palvData, alsiSize, alsiFreq);
	if( AL_NO_ERROR != alGetError() )
	{
		//should log this
		alDeleteBuffers(1, &m_aluiBuffer);
		return;
	}

	//if we've got this far the buffer is valid
	m_bValid = true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function:				CalBuffer::~CalBuffer
//
// Last Modified by:		Douglas John Binks (DJB)
//
// Last Modified:			26 Jul 2001
//
// Purpose:					Destructor. Unloads wav file
//
// Inputs:					None.
//
// Outputs:					None.
//
// Returns:					None.
//
//////////////////////////////////////////////////////////////////////////////////////////
CalBuffer::~CalBuffer()
{
	//delete buffer if all references are gone
	if( ( m_bValid ) && ( 0 == --*m_puiNumReferences ) )
	{
		alDeleteBuffers(1, &m_aluiBuffer);
		delete m_puiNumReferences;
	}
}

