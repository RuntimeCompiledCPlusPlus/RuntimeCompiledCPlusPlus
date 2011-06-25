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

#include "GUISystem.h"
#include "GUIElement.h"
#include "GUIDocument.h"

#include <Rocket/Core.h>
#include <Rocket/Debugger.h>
#include <assert.h>


GUISystem::GUISystem() : m_pContext(0)
{
}

GUISystem::~GUISystem()
{
	TInstancedElements::iterator it = m_instanced.begin();
	TInstancedElements::iterator itEnd = m_instanced.end();
	while (it != itEnd)
	{
		delete *it;
		++it;
	}
}

void GUISystem::SetContext(Rocket::Core::Context* pContext)
{
	m_pContext = pContext;
	assert(m_pContext);
}

void GUISystem::ClearStyleSheetCache()
{
	Rocket::Core::Factory::ClearStyleSheetCache();
}

IGUIDocument* GUISystem::LoadDocument(const char* file, const char* id)
{
	assert(m_pContext);

	// Check for an existing document with this ID
	Rocket::Core::ElementDocument* pExisting = m_pContext->GetDocument(id);
	
	Rocket::Core::ElementDocument* pED = m_pContext->LoadDocument(file);
	if (pED)
	{
		if (pExisting)
		{
			// Delete existing document now that new one has been successfully loaded
			pExisting->Hide();
			m_pContext->UnloadDocument(pExisting);
		}

		pED->SetId(id);
		return InstanceDocument(pED, true);
	}

	return NULL;
}

IGUIDocument* GUISystem::GetDocument(const char* id)
{
	assert(m_pContext);

	Rocket::Core::ElementDocument* pED = m_pContext->GetDocument(id);
	if (pED)
	{
		return InstanceDocument(pED, false);
	}

	return NULL;
}

IGUIElement* GUISystem::GetLogElement()
{
	Rocket::Core::Element* pElement = Rocket::Debugger::GetLogElement();
	if (pElement)
	{
		return InstanceElement(pElement, false);
	}

	return NULL;
}

GUIElement* GUISystem::InstanceElement(Rocket::Core::Element* pElement, bool bHoldingRLReference)
{
	GUIElement* pGUIElement = new GUIElement(this, pElement, bHoldingRLReference);

	m_instanced.insert(pGUIElement);

	return pGUIElement;
}

GUIDocument* GUISystem::InstanceDocument(Rocket::Core::ElementDocument* pDocument, bool bHoldingRLReference)
{
	GUIDocument* pGUIDocument = new GUIDocument(this, pDocument, bHoldingRLReference);

	m_instanced.insert(pGUIDocument);

	return pGUIDocument;
}

void GUISystem::Release(GUIElement* pElement)
{
	GUIElement* pGUIElement = static_cast<GUIElement*>(pElement);

	TInstancedElements::iterator it = m_instanced.find(pGUIElement);
	assert(it != m_instanced.end());
	if (it != m_instanced.end())
	{
		delete *it;
		m_instanced.erase(it);
	}
}


