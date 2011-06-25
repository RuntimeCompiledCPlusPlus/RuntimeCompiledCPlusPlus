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
// File:							"alBuffer.h"
//
// Related Source/Header File:		"alBuffer.cpp"
//
// Original Author:					Douglas John Binks (DJB)
//
// Creation Date:					26 July 2001
//
// Purpose:
//	Handles OpenAL sound object loading into an OpenAL buffer, using wav files.
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef ALBUFFERDEF
#define ALBUFFERDEF



#include <string>
using namespace std;

class CalBuffer
{
public:
	CalBuffer( const string& strWavFileName_ );

	//Copy constructor
	CalBuffer( CalBuffer& alBuffer_ ) :
		m_aluiBuffer(		alBuffer_.m_aluiBuffer ),
		m_bValid(			alBuffer_.m_bValid ),
		m_puiNumReferences(	alBuffer_.m_puiNumReferences )
	{
		++*m_puiNumReferences;
	}


	~CalBuffer();

	//inspector to check validity of buffer (can be used if true)
	inline bool GetIsValid() const { return m_bValid; }
	inline unsigned int GetBuffer() const { return m_aluiBuffer; }

	//assignment operator
	CalBuffer& operator=( CalBuffer& alBuffer_ )
	{
		m_aluiBuffer		= alBuffer_.m_aluiBuffer;
		m_bValid			= alBuffer_.m_bValid;
		m_puiNumReferences	= alBuffer_.m_puiNumReferences;
		++*m_puiNumReferences;
	}

private:
	unsigned int m_aluiBuffer;
	bool m_bValid;
	unsigned int* m_puiNumReferences;
};

#endif