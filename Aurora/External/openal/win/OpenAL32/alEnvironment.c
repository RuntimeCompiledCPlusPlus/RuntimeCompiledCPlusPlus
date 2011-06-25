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
#include "include\alMain.h"
#include "include\alError.h"
#include "include\alEnvironment.h"

static ALenvironment *Environment=NULL;
static ALuint EnvironmentCount=0;

ALAPI ALvoid ALAPIENTRY alGenEnvironmentIASIG(ALsizei n,ALuint *environments)
{
	ALenvironment *ALEnvironment;
	ALsizei i=0;

	if (!Environment)
	{
		Environment=malloc(sizeof(ALenvironment));
		if (Environment)
		{
			memset(Environment,0,sizeof(ALenvironment));
			environments[i]=(ALuint)Environment;
			Environment->valid=AL_TRUE;
			alEnvironmentfIASIG(environments[i],AL_ENV_ROOM_IASIG,0.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_ROOM_HIGH_FREQUENCY_IASIG,1.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_DECAY_TIME_IASIG,1.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_DECAY_HIGH_FREQUENCY_RATIO_IASIG,0.5f);
			alEnvironmentfIASIG(environments[i],AL_ENV_REFLECTIONS_IASIG,0.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_REFLECTIONS_DELAY_IASIG,0.02f);
			alEnvironmentfIASIG(environments[i],AL_ENV_REVERB_IASIG,0.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_REVERB_DELAY_IASIG,0.04f);
			alEnvironmentfIASIG(environments[i],AL_ENV_DIFFUSION_IASIG,100.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_DENSITY_IASIG,100.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG,5000.0f);
			EnvironmentCount++;
			i++;
		}
		ALEnvironment=Environment;
	}
	else
	{
		ALEnvironment=Environment;
		while (ALEnvironment->next)
			ALEnvironment=ALEnvironment->next;
	}
	
	while ((ALEnvironment)&&(i<n))
	{
		ALEnvironment->next=malloc(sizeof(ALenvironment));
		if (ALEnvironment->next)
		{
			memset(ALEnvironment->next,0,sizeof(ALenvironment));
			environments[i]=(ALuint)ALEnvironment->next;
			ALEnvironment->next->previous=ALEnvironment;
			ALEnvironment->next->valid=AL_TRUE;
			alEnvironmentfIASIG(environments[i],AL_ENV_ROOM_IASIG,0.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_ROOM_HIGH_FREQUENCY_IASIG,1.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_DECAY_TIME_IASIG,1.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_DECAY_HIGH_FREQUENCY_RATIO_IASIG,0.5f);
			alEnvironmentfIASIG(environments[i],AL_ENV_REFLECTIONS_IASIG,0.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_REFLECTIONS_DELAY_IASIG,0.02f);
			alEnvironmentfIASIG(environments[i],AL_ENV_REVERB_IASIG,0.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_REVERB_DELAY_IASIG,0.04f);
			alEnvironmentfIASIG(environments[i],AL_ENV_DIFFUSION_IASIG,100.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_DENSITY_IASIG,100.0f);
			alEnvironmentfIASIG(environments[i],AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG,5000.0f);
			EnvironmentCount++;
			i++;
		}
		ALEnvironment=ALEnvironment->next;
	}
	//if (i!=n) alSetError(AL_OUT_OF_MEMORY);
}

ALAPI ALvoid ALAPIENTRY alDeleteEnvironmentIASIG(ALsizei n,ALuint *environments)
{
	ALenvironment *ALEnvironment;
	ALsizei i;

	for (i=0;i<n;i++)
	{
		if (alIsEnvironmentIASIG(environments[i]))
		{
			ALEnvironment=((ALenvironment *)environments[i]);
			if (ALEnvironment->previous)
				ALEnvironment->previous->next=ALEnvironment->next;
			else
				Environment=ALEnvironment->next;
			if (ALEnvironment->next)
				ALEnvironment->next->previous=ALEnvironment->previous;
			memset(ALEnvironment,0,sizeof(ALenvironment));
			EnvironmentCount--;
			free(ALEnvironment);
		}
		else alSetError(AL_INVALID_OPERATION);
	}
}

ALAPI ALboolean ALAPIENTRY alIsEnvironmentIASIG(ALuint environment)
{
	ALboolean result=AL_FALSE;
	ALenvironment *ALEnvironment;
	
	ALEnvironment=((ALenvironment *)environment);
	if (ALEnvironment)
	{
		if ((ALEnvironment->previous==NULL)||(ALEnvironment->previous->next==ALEnvironment))
		{
			if ((ALEnvironment->next==NULL)||(ALEnvironment->next->previous==ALEnvironment))
				result=AL_TRUE;
		}
	}
	return result;
}

ALAPI ALvoid ALAPIENTRY alEnvironmentfIASIG(ALuint environment,ALenum pname,ALfloat value)
{
	ALenvironment *ALEnvironment;

	if (alIsEnvironmentIASIG(environment))
	{
		ALEnvironment=((ALenvironment *)environment);
		switch(pname) 
		{
			case AL_ENV_ROOM_IASIG:
				if ((value>=0.0f)&&(value<=1.0f))
					ALEnvironment->room=value;
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_ENV_ROOM_HIGH_FREQUENCY_IASIG:
				if ((value>=0.0)&&(value<=1.0f))
					ALEnvironment->room_hf=value;
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_ENV_DECAY_TIME_IASIG:
				if ((value>=0.1f)&&(value<=20.0f))
					ALEnvironment->decay_time=value;
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_ENV_DECAY_HIGH_FREQUENCY_RATIO_IASIG:
				if ((value>=0.1f)&&(value<=2.0f))
					ALEnvironment->decay_hf_ratio=value;
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_ENV_REFLECTIONS_IASIG:
				if ((value>=0.0f)&&(value<=3.0f))
					ALEnvironment->reflections=value;
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_ENV_REFLECTIONS_DELAY_IASIG:
				if ((value>=0.0f)&&(value<=0.3f))
					ALEnvironment->reflections_delay=value;
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_ENV_REVERB_IASIG:
				if ((value>=0.0f)&&(value<=10.0f))
					ALEnvironment->reverb=value;
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_ENV_REVERB_DELAY_IASIG:
				if ((value>=0.0f)&&(value<=0.1f))
					ALEnvironment->reverb_delay=value;
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_ENV_DIFFUSION_IASIG:
				if ((value>=0.0f)&&(value<=100.0f))
					ALEnvironment->diffusion=value;
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_ENV_DENSITY_IASIG:
				if ((value>=0.0f)&&(value<=100.0f))
					ALEnvironment->density=value;
				else
					alSetError(AL_INVALID_VALUE);
				break;
			case AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG:
				if ((value>=20.0f)&&(value<=20000.0f))
					ALEnvironment->hf_reference=value;
				else
					alSetError(AL_INVALID_VALUE);
				break;
			default:
				alSetError(AL_INVALID_OPERATION);
				break;
		}
	}
	else alSetError(AL_INVALID_OPERATION);
}

ALAPI ALvoid ALAPIENTRY alEnvironmentiIASIG(ALuint environment,ALenum pname,ALint value)
{
	ALenvironment *ALEnvironment;

	if (alIsEnvironmentIASIG(environment))
	{
		ALEnvironment=((ALenvironment *)environment);
		switch(pname) 
		{
			default:
				alSetError(AL_INVALID_OPERATION);
				break;
		}
	}
	else alSetError(AL_INVALID_OPERATION);
}

ALAPI ALvoid ALAPIENTRY alGetEnvironmentfIASIG(ALuint environment,ALenum pname,ALfloat *value)
{
	ALenvironment *ALEnvironment;

	if (alIsEnvironmentIASIG(environment))
	{
		ALEnvironment=((ALenvironment *)environment);
		switch(pname) 
		{
			case AL_ENV_ROOM_IASIG:
				*value=ALEnvironment->room;
				break;
			case AL_ENV_ROOM_HIGH_FREQUENCY_IASIG:
				*value=ALEnvironment->room_hf;
				break;
			case AL_ENV_DECAY_TIME_IASIG:
				*value=ALEnvironment->decay_time;
				break;
			case AL_ENV_DECAY_HIGH_FREQUENCY_RATIO_IASIG:
				*value=ALEnvironment->decay_hf_ratio;
				break;
			case AL_ENV_REFLECTIONS_IASIG:
				*value=ALEnvironment->reflections;
				break;
			case AL_ENV_REFLECTIONS_DELAY_IASIG:
				*value=ALEnvironment->reflections_delay;
				break;
			case AL_ENV_REVERB_IASIG:
				*value=ALEnvironment->reverb;
				break;
			case AL_ENV_REVERB_DELAY_IASIG:
				*value=ALEnvironment->reverb_delay;
				break;
			case AL_ENV_DIFFUSION_IASIG:
				*value=ALEnvironment->diffusion;
				break;
			case AL_ENV_DENSITY_IASIG:
				*value=ALEnvironment->density;
				break;
			case AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG:
				*value=ALEnvironment->hf_reference;
				break;
			default:
				alSetError(AL_INVALID_OPERATION);
				break;
		}
	}
	else alSetError(AL_INVALID_OPERATION);
}

ALAPI ALvoid ALAPIENTRY alGetEnvironmentiIASIG(ALuint environment,ALenum pname,ALint *value)
{
	ALenvironment *ALEnvironment;

	if (alIsEnvironmentIASIG(environment))
	{
		ALEnvironment=((ALenvironment *)environment);
		switch(pname) 
		{
			default:
				alSetError(AL_INVALID_OPERATION);
				break;
		}
	}
	else alSetError(AL_INVALID_OPERATION);
}















