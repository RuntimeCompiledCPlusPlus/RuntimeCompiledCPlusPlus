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

#include <stdlib.h>
#include <string.h>
#include "include\alExtension.h"
#include "include\alEax.h"

static ALextension extension[]=  {	
	{ "EAX",					  (ALvoid *) NULL				      },
	{ "EAX2.0",					  (ALvoid *) NULL				      },
	{ "I3DL2",					  (ALvoid *) NULL				      },
	{ NULL,						  (ALvoid *) NULL					  } };

static ALfunction  function[]=   {	
	{ "EAXGet",					  (ALvoid *) EAXGet					  },
	{ "EAXSet",					  (ALvoid *) EAXSet					  },
	{ NULL,						  (ALvoid *) NULL					  } };

static ALenums	   enumeration[]={	
	{ NULL,						  (ALenum  ) 0 						  } };

ALAPI ALboolean ALAPIENTRY alIsExtensionPresent(ALubyte *extName)
{
	ALsizei i=0;

	while ((extension[i].extName)&&(strcmp((char *)extension[i].extName,(char *)extName)))
		i++;
	return (extension[i].extName!=NULL?AL_TRUE:AL_FALSE);
}

ALAPI ALvoid * ALAPIENTRY alGetProcAddress(ALubyte *funcName)
{
	ALsizei i=0;

	while ((function[i].funcName)&&(strcmp((char *)function[i].funcName,(char *)funcName)))
		i++;
	return function[i].address;
}

ALAPI ALenum ALAPIENTRY alGetEnumValue(ALubyte *enumName)
{
	ALsizei i=0;

	while ((enumeration[i].enumName)&&(strcmp((char *)enumeration[i].enumName,(char *)enumName)))
		i++;
	return enumeration[i].value;
}
