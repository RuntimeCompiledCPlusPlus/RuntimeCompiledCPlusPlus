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

#pragma once

#ifndef GUIELEMENT_INCLUDED
#define GUIELEMENT_INCLUDED

#include "../IGUIElement.h"
#include "ReferenceCountable.h"

struct IGUIInstancer;

// Forward declare Rocket::Core::Element
namespace Rocket { namespace Core { class Element; } }


class GUIElement : public ReferenceCountable, public IGUIElement
{
public:
	GUIElement(IGUIInstancer* pInstancer, Rocket::Core::Element* pElement, bool bHoldingRLReference);
	virtual ~GUIElement();

	// IGUIElement

	virtual const char* GetId() const;
	virtual void SetInnerRML(const char* content);
	virtual void SetProperty(const char* name, const char* value);
	virtual IGUIElement* GetElementById(const char* id);
	virtual void GetAttribute(const char* attribute, char* buffer, size_t buffer_size ) const;
	virtual void SetAttribute(const char* name, const char* value);
	virtual float GetClientWidth() const;
	virtual float GetClientHeight() const;

	virtual void AddReference();
	virtual void RemoveReference();

	virtual void AddEventListener( const char* eventname, IGUIEventListener* pEventListener );
	virtual void RemoveEventListener( const char* eventname, IGUIEventListener* pEventListener );

	// ~IGUIElement

protected:
	// ReferenceCountable

	virtual void OnReferenceDeactivate();

	// ~ReferenceCountable

private:
	Rocket::Core::Element* m_pElement;
	IGUIInstancer* m_pInstancer;
	bool m_bHoldingRLReference;
};

#endif // GUIELEMENT_INCLUDED
