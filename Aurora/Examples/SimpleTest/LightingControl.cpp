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

#include "ILightingControl.h"

#include "../../Common/Math.inl"
#include "../../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../../Systems/SystemTable.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/IAssetSystem.h"
#include "../../Systems/ILogSystem.h"
#include "../../RuntimeObjectSystem/ISimpleSerializer.h"

#include <assert.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////
// We serialize all the current lighting values if any have been changed 
// via ILightingControl interface, otherwise we reload the initial values
// below, allowing them to be experimented with at runtime
//////////////////////////////////////////////////////////////////////////


// Initial values
static const float intensity = 0.6f;
static const float back_color[4] = {0.3f, 0.0f, 0.0f, 1.0f};	// red
static const float global_ambient[4] = {0.2f, 0.2f, 0.2f, 1.0f};	// bright
static const float light_ambient[4] = {intensity/20.0f, intensity/20.0f, intensity/20.0f, 1.0f};
static const float light_diffuse[4] = {intensity*1.2f, intensity, intensity, 1.0f};
static const float light_specular[4] = {intensity*1.2f, intensity*0.8f, intensity*0.8f, 1.0f};


class LightingControl: public ILightingControl
{
public:
	LightingControl() 
	{
		m_bModified = false;
		m_back_color.resize(4);
		m_global_ambient.resize(4);
		m_light_ambient.resize(4);
		m_light_diffuse.resize(4);
		m_light_specular.resize(4);

		CopyArray4(back_color, &m_back_color[0]);
		CopyArray4(global_ambient, &m_global_ambient[0]);
		CopyArray4(light_ambient, &m_light_ambient[0]);
		CopyArray4(light_diffuse, &m_light_diffuse[0]);
		CopyArray4(light_specular, &m_light_specular[0]);
	}

	virtual ~LightingControl()
	{
		if( m_pEntity )
		{
			m_pEntity->SetUpdateable(NULL);
		}
	}


	// IEntityObject

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
		AU_ASSERT(pSerializer);
		IEntityObject::Serialize(pSerializer);
		SERIALIZE(m_bModified);
		if (m_bModified)
		{
			SERIALIZE(m_back_color);
			SERIALIZE(m_global_ambient);
			SERIALIZE(m_light_ambient);
			SERIALIZE(m_light_diffuse);
			SERIALIZE(m_light_specular);
		}
	}

	virtual void Init( bool isFirstInit )
	{
		m_pEntity->SetUpdateable( this );
	}

	// ~IEntityObject

	// IAUUpdateable

	virtual void Update( float deltaTime )
	{
		// Do nothing for now
	}

	// ~IAUUpdateable

	// ILightingControl

	virtual void GetBackColor(float* params)
	{
		CopyArray4(&m_back_color[0], params);
	}

	virtual void SetBackColor(const float* params)
	{
		CopyArray4(params, &m_back_color[0]);
		m_bModified = true;
	}

	virtual void GetGlobalAmbient(float* params)
	{
		CopyArray4(&m_global_ambient[0], params);
	}

	virtual void SetGlobalAmbient(const float* params)
	{
		CopyArray4(params, &m_global_ambient[0]);
		m_bModified = true;
	}

	virtual void GetLightAmbient(float* params)
	{
		CopyArray4(&m_light_ambient[0], params);
	}

	virtual void SetLightAmbient(const float* params)
	{
		CopyArray4(params, &m_light_ambient[0]);
		m_bModified = true;
	}

	virtual void GetLightDiffuse(float* params)
	{
		CopyArray4(&m_light_diffuse[0], params);
	}

	virtual void SetLightDiffuse(const float* params)
	{
		CopyArray4(params, &m_light_diffuse[0]);
		m_bModified = true;
	}

	virtual void GetLightSpecular(float* params)
	{
		CopyArray4(&m_light_specular[0], params);
	}

	virtual void SetLightSpecular(const float* params)
	{
		CopyArray4(params, &m_light_specular[0]);
		m_bModified = true;
	}

	// ~ILightingControl


private:

	void CopyArray4(const float* src, float* dest)
	{
		dest[0] = src[0];
		dest[1] = src[1];
		dest[2] = src[2];
		dest[3] = src[3];
	}


	// Private Members

	bool m_bModified;
	std::vector<float> m_back_color;
	std::vector<float> m_global_ambient;
	std::vector<float> m_light_ambient;
	std::vector<float> m_light_diffuse;
	std::vector<float> m_light_specular;
};

REGISTERCLASS(LightingControl);




