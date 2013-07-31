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
        
#ifdef _WIN32
		strcpy_s( buffer, buffer_size, strvalue.CString() );
#else
        strncpy( buffer, strvalue.CString(), buffer_size); //not quite the same, but similar safe effect.
#endif
	}
private:
	Rocket::Core::Event& m_Event;
};

class EventHolder : public Rocket::Core::EventListener
{
public:
	static EventHolder* GetEventHolder( IGUIEventListener* pEvent )
	{
		std::map<IGUIEventListener*, EventHolder*>::iterator found = m_EventListeners.find( pEvent ) ;
		if( found != m_EventListeners.end() )
		{
			return found->second;
		}
		else
		{
			return 0;
		}
	}

	EventHolder( IGUIEventListener* pEvent )
		: m_pEvent( pEvent )
	{
		//there should be no events registered for this id
		assert( m_EventListeners.find( pEvent ) == m_EventListeners.end());
		m_EventListeners[ pEvent ] = this;
	}
	~EventHolder()
	{
		m_EventListeners.erase( m_pEvent );
	}


	virtual void ProcessEvent(Rocket::Core::Event& event)
	{
		GUIEvent guiEvent( event );
		m_pEvent->OnEvent( guiEvent );
	}


private:

	EventHolder();
	EventHolder( EventHolder& event );
	EventHolder& operator=( EventHolder& event );

	IGUIEventListener*	m_pEvent;

	static std::map<IGUIEventListener*, EventHolder*> m_EventListeners;
};
std::map<IGUIEventListener*, EventHolder*> EventHolder::m_EventListeners;



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
#ifdef _WIN32
			strcpy_s( buffer, buffer_size, strvalue.CString() );
#else
            strncpy( buffer, strvalue.CString(), buffer_size); //not quite the same, but similar safe effect.
#endif
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

void GUIElement::AddEventListener( const char* eventname, IGUIEventListener* pEventListener )
{
	assert( m_pElement );
	EventHolder* pEvent = new EventHolder( pEventListener );
	m_pElement->AddEventListener( eventname, pEvent );
}

void GUIElement::RemoveEventListener( const char* eventname, IGUIEventListener* pEventListener)
{
	assert( m_pElement );
	EventHolder* pEvent = EventHolder::GetEventHolder( pEventListener );
	assert( pEvent );
	m_pElement->RemoveEventListener( eventname, pEvent );
	delete pEvent;

}
