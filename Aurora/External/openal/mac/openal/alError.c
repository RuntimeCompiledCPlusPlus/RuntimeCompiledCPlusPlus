/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2000 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */
 
#include "globals.h"
#include "alError.h"
 
// AL_ERROR functions
ALAPI ALenum ALAPIENTRY alGetError (ALvoid)
{
	ALenum returnError;
	
	returnError = alLastError;
	alLastError = AL_NO_ERROR;
	  
	return returnError;
}

ALAPI ALvoid ALAPIENTRY alSetError (ALenum errorCode)
{
    if (alLastError == AL_NO_ERROR)
    {
		alLastError=errorCode;
	}
}

