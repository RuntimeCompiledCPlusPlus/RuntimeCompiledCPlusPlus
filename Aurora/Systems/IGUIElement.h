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

#ifndef IGUIELEMENT_INCLUDED
#define IGUIELEMENT_INCLUDED

struct IGUIEvent
{
	virtual ~IGUIEvent() {}
	virtual void GetParameter(const char* parameter, char* buffer, size_t buffer_size ) const = 0;
};

struct IGUIEventListener
{
	virtual ~IGUIEventListener() {}
	virtual void OnEvent( int event_id, const IGUIEvent& event_info ) = 0;
};


struct IGUIElement
{
	virtual ~IGUIElement() {}

	virtual const char* GetId() const = 0;
	virtual void SetInnerRML(const char* content) = 0;
	virtual void SetProperty(const char* name, const char* value) = 0;
	virtual IGUIElement* GetElementById(const char* id) = 0;
	virtual void GetAttribute(const char* attribute, char* buffer, size_t buffer_size ) const = 0;
	virtual void SetAttribute(const char* name, const char* value) = 0;
	virtual float GetClientWidth() const = 0;
	virtual float GetClientHeight() const = 0;

	// Reference counting
	virtual void AddReference() = 0;
	virtual void RemoveReference() = 0;

	// Add an event with caller provided id to enable differentiating different event names in callback OnEvent
	virtual void AddEventListener( const char* eventname, IGUIEventListener* pEventListener, int event_id  ) = 0;

	// Remove event
	virtual void RemoveEventListener( const char* eventname, IGUIEventListener* pEventListener, int event_id ) = 0;
};

#endif // IGUIELEMENT_INCLUDED