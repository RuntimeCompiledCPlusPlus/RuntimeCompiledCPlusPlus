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

#include "IGUIDocument.h"
#include <cstring>

struct IGUIEvent
{
	virtual ~IGUIEvent() {}
	virtual void GetParameter(const char* parameter, char* buffer, size_t buffer_size ) const = 0;
};

struct IGUIEventListener
{
    virtual ~IGUIEventListener() {}
	virtual void OnEvent( const IGUIEvent& event_info ) = 0;
};

struct IGUIElement
{
public:
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

	// Add an event listener
	virtual void AddEventListener( const char* pName, IGUIEventListener* pEventListener  ) = 0;

	// Remove an event listener
	virtual void RemoveEventListener( const char* pName, IGUIEventListener* pEventListener ) = 0;
};

// IGUISingleEventListener can only listen to single events, but has auto removal for easy usuage
class IGUISingleEventListener : public IGUIEventListener
{
    char*           m_pEventname;
    IGUIElement*    m_pElement;

public:
    IGUISingleEventListener( )
        : m_pEventname( 0 )
        , m_pElement( 0 )
    {
    }

    const char* GetName()
    {
        return m_pEventname;
    }

    IGUIElement* GetElement()
    {
        return m_pElement;
    }

    void AddEventToElementInDoc( const char* pEventname, const char* pElementname_, IGUIDocument* pDocument )
    {
       IGUIElement* pElement = pDocument->Element()->GetElementById( pElementname_ );
       if( pElement )
       {
            AddEventToElement( pEventname, pElement );
            pElement->RemoveReference(); // a ref is added in addevent
       }
    }


    // AddEventToElement adds a reference to pElement which it will remove when the
    // event is deleted or RemoveEvent is called.
    void AddEventToElement( const char* pEventname, IGUIElement* pElement )
    {
        RemoveEvent();
        m_pEventname = new char[ strlen(  pEventname ) + 1 ];
        strcpy( m_pEventname, pEventname );
        m_pElement = pElement;
        m_pElement->AddReference();
        m_pElement->AddEventListener( m_pEventname, this );
    }

    void RemoveEvent()
    {
        if( m_pElement )
        {
            m_pElement->RemoveEventListener( m_pEventname, this );
            m_pElement->RemoveReference();
            m_pElement = 0;
            delete[] m_pEventname;
            m_pEventname = 0;
       }
    }

    virtual ~IGUISingleEventListener()
    {
        RemoveEvent();
    }

};

#endif // IGUIELEMENT_INCLUDED