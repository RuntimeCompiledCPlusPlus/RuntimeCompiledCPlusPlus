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

#include "include\alMain.h"
#include "include\alError.h"
#include "include\alEax.h"
#include "include\eax.h"
#include <objbase.h>
#include <initguid.h>



ALAPI ALenum ALAPIENTRY EAXGet(const GUID *propertySetID,ALuint property,ALuint source,ALvoid *value,ALuint size)
{
	LPKSPROPERTYSET lpPropertySet;
	ALuint returnsize, i;
	ALsource *Source;
	ALCcontext *ALContext;

	if ( (IsEqualGUID(propertySetID, &DSPROPSETID_EAX20_BufferProperties)) && (alIsSource(source)) )
	{
		Source = ((ALsource *)source);
		
		// If the Property Set interface has been created - get the information from the driver
		if (Source->uservalue3)
		{
			IKsPropertySet_Get((LPKSPROPERTYSET)Source->uservalue3, propertySetID, property, NULL, 0, value, size, &returnsize);
		}
		else
		{
			// Property Set interface not created - return local copies of the EAX parameters

			// Remove the EAXBUFFER DEFERRED setting
			property &= ~DSPROPERTY_EAXBUFFER_DEFERRED;

			switch (property)
			{
				case DSPROPERTY_EAXBUFFER_ALLPARAMETERS:
					memcpy(value, &(Source->eaxBP), sizeof(EAXBUFFERPROPERTIES));
					break;

				case DSPROPERTY_EAXBUFFER_DIRECT:
					*(ALint*)value = Source->eaxBP.lDirect;
					break;

				case DSPROPERTY_EAXBUFFER_DIRECTHF:
					*(ALint*)value = Source->eaxBP.lDirectHF;
					break;

				case DSPROPERTY_EAXBUFFER_ROOM:
					*(ALint*)value = Source->eaxBP.lRoom;
					break;

				case DSPROPERTY_EAXBUFFER_ROOMHF:
					*(ALint*)value = Source->eaxBP.lRoomHF;
					break;

				case DSPROPERTY_EAXBUFFER_ROOMROLLOFFFACTOR:
					*(ALfloat*)value = Source->eaxBP.flRoomRolloffFactor;
					break;

				case DSPROPERTY_EAXBUFFER_OBSTRUCTION:
					*(ALint*)value = Source->eaxBP.lObstruction;
					break;

				case DSPROPERTY_EAXBUFFER_OBSTRUCTIONLFRATIO:
					*(ALfloat*)value = Source->eaxBP.flObstructionLFRatio;
					break;

				case DSPROPERTY_EAXBUFFER_OCCLUSION:
					*(ALint*)value = Source->eaxBP.lOcclusion;
					break;

				case DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO:
					*(ALfloat*)value = Source->eaxBP.flOcclusionLFRatio;
					break;

				case DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO:
					*(ALfloat*)value = Source->eaxBP.flOcclusionRoomRatio;
					break;

				case DSPROPERTY_EAXBUFFER_OUTSIDEVOLUMEHF:
					*(ALint*)value = Source->eaxBP.lOutsideVolumeHF;
					break;

				case DSPROPERTY_EAXBUFFER_AIRABSORPTIONFACTOR:
					*(ALfloat*)value = Source->eaxBP.flAirAbsorptionFactor;
					break;

				case DSPROPERTY_EAXBUFFER_FLAGS:
					*(ALuint*)value = Source->eaxBP.dwFlags;
					break;

				default:
					break; 
			}
		}
	}
	else if (IsEqualGUID(propertySetID, &DSPROPSETID_EAX20_ListenerProperties))
	{
		// Find a source with a valid Property Set Interace (stored in uservalue3)
	
		ALContext = alcGetCurrentContext();

		Source = ALContext->Source;
		lpPropertySet = NULL;

		for (i=0;i<ALContext->SourceCount;i++)
		{
			if (alIsSource((ALuint)Source))
			{
				if (Source->uservalue3)
				{
					lpPropertySet = Source->uservalue3;
					break;
				}
			}
			Source = Source->next;
		}

		if (lpPropertySet)
		{
			IKsPropertySet_Get(lpPropertySet,propertySetID,property,NULL,0,value,size,&returnsize);
		}
		else
		{
			// No sources created yet, return local copies of EAX Listener properties

			// Remove the EAXBUFFER DEFERRED setting if it is present
			property &= ~DSPROPERTY_EAXLISTENER_DEFERRED;

			switch (property)
			{
				case DSPROPERTY_EAXLISTENER_ALLPARAMETERS:
					memcpy(value, &(ALContext->Listener.eaxLP), sizeof(EAXLISTENERPROPERTIES));
					break;

				case DSPROPERTY_EAXLISTENER_ROOM:
					*(ALint*)value = ALContext->Listener.eaxLP.lRoom;
					break;

				case DSPROPERTY_EAXLISTENER_ROOMHF:
					*(ALint*)value = ALContext->Listener.eaxLP.lRoomHF;
					break;

				case DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR:
					*(ALfloat*)value = ALContext->Listener.eaxLP.flRoomRolloffFactor;
					break;

				case DSPROPERTY_EAXLISTENER_DECAYTIME:
					*(ALfloat*)value = ALContext->Listener.eaxLP.flDecayTime;
					break;

				case DSPROPERTY_EAXLISTENER_DECAYHFRATIO:
					*(ALfloat*)value = ALContext->Listener.eaxLP.flDecayHFRatio;
					break;

				case DSPROPERTY_EAXLISTENER_REFLECTIONS:
					*(ALint*)value = ALContext->Listener.eaxLP.lReflections;
					break;

				case DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY:
					*(ALfloat*)value = ALContext->Listener.eaxLP.flReflectionsDelay;
					break;

				case DSPROPERTY_EAXLISTENER_REVERB:
					*(ALint*)value = ALContext->Listener.eaxLP.lReverb;
					break;

				case DSPROPERTY_EAXLISTENER_REVERBDELAY:
					*(ALfloat*)value = ALContext->Listener.eaxLP.flReverbDelay;
					break;

				case DSPROPERTY_EAXLISTENER_ENVIRONMENT:
					*(ALuint*)value = ALContext->Listener.eaxLP.dwEnvironment;
					break;

				case DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE:
					*(ALfloat*)value = ALContext->Listener.eaxLP.flEnvironmentSize;
					break;

				case DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION:
					*(ALfloat*)value = ALContext->Listener.eaxLP.flEnvironmentDiffusion;
					break;

				case DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF:
					*(ALfloat*)value = ALContext->Listener.eaxLP.flAirAbsorptionHF;
					break;

				case DSPROPERTY_EAXLISTENER_FLAGS:
					*(ALuint*)value = ALContext->Listener.eaxLP.dwFlags;
					break;

				default:
					break; 
			}
		}
	}

	return AL_NO_ERROR;
}

ALAPI ALenum ALAPIENTRY EAXSet(const GUID *propertySetID,ALuint property,ALuint source,ALvoid *value,ALuint size)
{
	ALsource *Source;

	ALCcontext *Context;
	
	Context=alcGetCurrentContext();
	alcSuspendContext(Context);

	if ( (IsEqualGUID(propertySetID, &DSPROPSETID_EAX20_BufferProperties)) && (alIsSource(source)) )
	{
		Source = (ALsource *)source;

		// Check Deferred setting
		if (property & DSPROPERTY_EAXBUFFER_DEFERRED)
		{
			Source->update2 |= SDEFERRED;

			// Remove the EAXBUFFER DEFERRED setting
			property &= ~DSPROPERTY_EAXBUFFER_DEFERRED;
		}
		else
		{
			// Deferred flag not specified - so commit settings
			Source->update2 |= SCOMMITSETTINGS;
		}

		switch (property)
		{
			case DSPROPERTY_EAXBUFFER_COMMITDEFERREDSETTINGS:
				Source->update2 |= SCOMMITSETTINGS;
				break;

			case DSPROPERTY_EAXBUFFER_ALLPARAMETERS:
				memcpy(&(Source->eaxBP), value, sizeof(EAXBUFFERPROPERTIES));
				Source->update2 |= SALLPARAMS;
                break;

            case DSPROPERTY_EAXBUFFER_DIRECT:
                Source->eaxBP.lDirect = *(ALint*)value;
				Source->update2 |= SDIRECT;
                break;

            case DSPROPERTY_EAXBUFFER_DIRECTHF:
                Source->eaxBP.lDirectHF = *(ALint*)value;
                Source->update2 |= SDIRECTHF;
				break;

            case DSPROPERTY_EAXBUFFER_ROOM:
                Source->eaxBP.lRoom = *(ALint*)value;
                Source->update2 |= SROOM;
				break;

            case DSPROPERTY_EAXBUFFER_ROOMHF:
                Source->eaxBP.lRoomHF = *(ALint*)value;
                Source->update2 |= SROOMHF;
				break;

            case DSPROPERTY_EAXBUFFER_ROOMROLLOFFFACTOR:
                Source->eaxBP.flRoomRolloffFactor = *(ALfloat*)value;
                Source->update2 |= SROOMROLLOFFFACTOR;
				break;

            case DSPROPERTY_EAXBUFFER_OBSTRUCTION:
                Source->eaxBP.lObstruction = *(ALint*)value;
                Source->update2 |= SOBSTRUCTION;
				break;

            case DSPROPERTY_EAXBUFFER_OBSTRUCTIONLFRATIO:
                Source->eaxBP.flObstructionLFRatio = *(ALfloat*)value;
                Source->update2 |= SOBSTRUCTIONLFRATIO;
				break;

            case DSPROPERTY_EAXBUFFER_OCCLUSION:
                Source->eaxBP.lOcclusion = *(ALint*)value;
                Source->update2 |= SOCCLUSION;
				break;

            case DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO:
                Source->eaxBP.flOcclusionLFRatio = *(ALfloat*)value;
                Source->update2 |= SOCCLUSIONLFRATIO;
				break;

            case DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO:
                Source->eaxBP.flOcclusionRoomRatio = *(ALfloat*)value;
                Source->update2 |= SOCCLUSIONROOMRATIO;
				break;

            case DSPROPERTY_EAXBUFFER_OUTSIDEVOLUMEHF:
                Source->eaxBP.lOutsideVolumeHF = *(ALint*)value;
				Source->update2 |= SOUTSIDEVOLUMEHF;
                break;

            case DSPROPERTY_EAXBUFFER_AIRABSORPTIONFACTOR:
                Source->eaxBP.flAirAbsorptionFactor = *(ALfloat*)value;
                Source->update2 |= SAIRABSORPTIONFACTOR;
				break;

            case DSPROPERTY_EAXBUFFER_FLAGS:
                Source->eaxBP.dwFlags = *(ALuint*)value;
                Source->update2 |= SFLAGS;
				break;

            default:
                break; 
		}

		alcUpdateContext(Context, ALSOURCE, source);

	}
	else if (IsEqualGUID(propertySetID, &DSPROPSETID_EAX20_ListenerProperties))
	{
		// Check Deferred setting
		if (property & DSPROPERTY_EAXLISTENER_DEFERRED)
		{
			Context->Listener.update2 |= LDEFERRED;

			// Remove the EAXLISTENER DEFERRED setting
			property &= ~DSPROPERTY_EAXLISTENER_DEFERRED;
		}
		else
		{
			// Deferred flag not set - commit the settings
			Context->Listener.update2 |= LCOMMITSETTINGS;
		}

		switch (property)
		{
			case DSPROPERTY_EAXLISTENER_COMMITDEFERREDSETTINGS:
				Context->Listener.update2 |= LCOMMITSETTINGS;
				break;

			case DSPROPERTY_EAXLISTENER_ALLPARAMETERS:
				memcpy(&(Context->Listener.eaxLP), value, sizeof(EAXLISTENERPROPERTIES));
				Context->Listener.update2 |= LALLPARAMS;
				break;

			case DSPROPERTY_EAXLISTENER_ROOM:
				Context->Listener.eaxLP.lRoom = *(ALint*)value;
				Context->Listener.update2 |= LROOM;
				break;

			case DSPROPERTY_EAXLISTENER_ROOMHF:
				Context->Listener.eaxLP.lRoomHF = *(ALint*)value;
				Context->Listener.update2 |= LROOMHF;
				break;

			case DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR:
				Context->Listener.eaxLP.flRoomRolloffFactor = *(ALfloat*)value;
				Context->Listener.update2 |= LROOMROLLOFFFACTOR;
				break;

			case DSPROPERTY_EAXLISTENER_DECAYTIME:
				Context->Listener.eaxLP.flDecayTime = *(ALfloat*)value;
				Context->Listener.update2 |= LDECAYTIME;
				break;

			case DSPROPERTY_EAXLISTENER_DECAYHFRATIO:
				Context->Listener.eaxLP.flDecayHFRatio = *(ALfloat*)value;
				Context->Listener.update2 |= LDECAYHFRATIO;
				break;

			case DSPROPERTY_EAXLISTENER_REFLECTIONS:
				Context->Listener.eaxLP.lReflections = *(ALint*)value;
				Context->Listener.update2 |= LREFLECTIONS;
				break;

			case DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY:
				Context->Listener.eaxLP.flReflectionsDelay = *(ALfloat*)value;
				Context->Listener.update2 |= LREFLECTIONSDELAY;
				break;

			case DSPROPERTY_EAXLISTENER_REVERB:
				Context->Listener.eaxLP.lReverb = *(ALint*)value;
				Context->Listener.update2 |= LREVERB;
				break;

			case DSPROPERTY_EAXLISTENER_REVERBDELAY:
				Context->Listener.eaxLP.flReverbDelay = *(ALfloat*)value;
				Context->Listener.update2 |= LREVERBDELAY;
				break;

			case DSPROPERTY_EAXLISTENER_ENVIRONMENT:
				Context->Listener.eaxLP.dwEnvironment = *(ALuint*)value;
				Context->Listener.update2 |= LENVIRONMENT;
				break;

			case DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE:
				Context->Listener.eaxLP.flEnvironmentSize = *(ALfloat*)value;
				Context->Listener.update2 |= LENVIRONMENTSIZE;
				break;

			case DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION:
				Context->Listener.eaxLP.flEnvironmentDiffusion = *(ALfloat*)value;
				Context->Listener.update2 |= LENVIRONMENTDIFFUSION;
				break;

			case DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF:
				Context->Listener.eaxLP.flAirAbsorptionHF = *(ALfloat*)value;
				Context->Listener.update2 |= LAIRABSORPTIONHF;
				break;

			case DSPROPERTY_EAXLISTENER_FLAGS:
				Context->Listener.eaxLP.dwFlags = *(ALuint*)value;
				Context->Listener.update2 |= LFLAGS;
				break;

			default:
				break; 
		}

		alcUpdateContext(Context, ALLISTENER, 0);	
	}

	alcProcessContext(Context);
	return AL_NO_ERROR;
}
