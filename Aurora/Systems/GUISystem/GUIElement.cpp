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

#include "GUIElement.h"
#include "IGUIInstancer.h"

#include <Rocket/Core.h>
#include <Rocket/Core/EventListener.h>
#include <assert.h>
#include <map>

class GUIEvent : public IGUIEvent
{
public:
	GUIEvent( Rocket::Core::Event& event )
		: m_Event( event )
	{
	}
	virtual void GetParameter(const char* parameter, char* buffer, size_t buffer_size ) const
	{
		Rocket::Core::String strvalue = "";
		if( m_Event.GetParameters() )
		{
			m_Event.GetParameters()->GetInto( parameter, strvalue );
		}
		strcpy_s( buffer, buffer_size, strvalue.CString() );
	}
private:
	Rocket::Core::Event& m_Event;
};

typedef std::pair<IGUIEventListener*,int> EVENTPAIR;

class EventHolder : public Rocket::Core::EventListener
{
public:
	static EventHolder* GetEventHolder( IGUIEventListener* pEvent, int event_id )
	{
		EVENTPAIR theEvent( pEvent, event_id );
		std::map<EVENTPAIR, EventHolder*>::iterator found = m_EventListeners.find( theEvent ) ;
		if( found != m_EventListeners.end() )
		{
			return found->second;
		}
		else
		{
			return 0;
		}
	}

	EventHolder( IGUIEventListener* pEvent, int event_id )
		: m_pEvent( pEvent )
		, m_EventId( event_id )
	{
		//there should be no events registered for this id
		EVENTPAIR theEvent( m_pEvent, m_EventId );
		assert( m_EventListeners.find( theEvent ) == m_EventListeners.end());
		m_EventListeners[ theEvent ] = this;
	}
	~EventHolder()
	{
		EVENTPAIR theEvent( m_pEvent, m_EventId );
		m_EventListeners.erase( theEvent );
	}


	virtual void ProcessEvent(Rocket::Core::Event& event)
	{
		GUIEvent guiEvent( event );
		m_pEvent->OnEvent( m_EventId, guiEvent );
	}


private:

	EventHolder();
	EventHolder( EventHolder& event );
	EventHolder& operator=( EventHolder& event );

	IGUIEventListener*	m_pEvent;
	int					m_EventId;

	static std::map<EVENTPAIR, EventHolder*> m_EventListeners;
};
std::map<EVENTPAIR, EventHolder*> EventHolder::m_EventListeners;



GUIElement::GUIElement(IGUIInstancer* pInstancer, Rocket::Core::Element* pElement, bool bHoldingRLReference)
	: m_pInstancer(pInstancer)
	, m_pElement(pElement)
	, m_bHoldingRLReference(bHoldingRLReference)
{
	assert(m_pInstancer && m_pElement);
}

GUIElement::~GUIElement()
{
	if (m_bHoldingRLReference)
	{
		m_pElement->RemoveReference();
	}
}

void GUIElement::AddReference()
{
	ReferenceCountable::AddReference();
}

void GUIElement::RemoveReference()
{
	ReferenceCountable::RemoveReference();
}

void GUIElement::OnReferenceDeactivate()
{
	m_pInstancer->Release(this);
}

const char* GUIElement::GetId() const
{
	return m_pElement->GetId().CString();
}

void GUIElement::SetInnerRML(const char* content)
{
	m_pElement->SetInnerRML(content);
}

void GUIElement::SetProperty(const char* name, const char* value)
{
	m_pElement->SetProperty(name, value);
}

void GUIElement::GetAttribute(const char* attribute, char* buffer, size_t buffer_size ) const
{
	if( buffer_size && buffer )
	{
		buffer[0] = 0;
		Rocket::Core::Variant* pVar = m_pElement->GetAttribute(attribute);
		if( pVar )
		{
			Rocket::Core::String strvalue;
			pVar->GetInto( strvalue );
			strcpy_s( buffer, buffer_size, strvalue.CString() );
		}
	}
}

void GUIElement::SetAttribute(const char* name, const char* value)
{
	m_pElement->SetAttribute(name, value);
}

float GUIElement::GetClientWidth() const
{
	return m_pElement->GetClientWidth();
}

float GUIElement::GetClientHeight() const
{
	return m_pElement->GetClientHeight();
}

IGUIElement* GUIElement::GetElementById(const char* id)
{
	Rocket::Core::Element* pElement = m_pElement->GetElementById(id);
	if (pElement)
	{
		return m_pInstancer->InstanceElement(pElement, false);
	}

	return NULL;
}

void GUIElement::AddEventListener( const char* eventname, IGUIEventListener* pEventListener, int event_id )
{
	assert( m_pElement );
	EventHolder* pEvent = new EventHolder( pEventListener, event_id );
	m_pElement->AddEventListener( eventname, pEvent );
}

void GUIElement::RemoveEventListener( const char* eventname, IGUIEventListener* pEventListener, int event_id)
{
	assert( m_pElement );
	EventHolder* pEvent = EventHolder::GetEventHolder( pEventListener, event_id );
	assert( pEvent );
	m_pElement->RemoveEventListener( eventname, pEvent );
	delete pEvent;

}
