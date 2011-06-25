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

#ifndef GUISYSTEM_INCLUDED
#define GUISYSTEM_INCLUDED

#include "../IGUISystem.h"
#include "IGUIInstancer.h"
#include <set>

// Forward declare Rocket::Core::Context
namespace Rocket { namespace Core { class Context; } }

class GUIElement;
class GUIDocument;


class GUISystem : public IGUISystem, public IGUIInstancer
{
public:
	GUISystem();
	~GUISystem();

	// IGUISystem

	virtual void SetContext(Rocket::Core::Context* pContext);
	virtual void ClearStyleSheetCache();
	virtual IGUIDocument* LoadDocument(const char* file, const char* id);
	virtual IGUIDocument*	GetDocument(const char* id);
	virtual IGUIElement* GetLogElement();

	// ~IGUISystem

	// IGUIInstancer

	virtual GUIElement* InstanceElement(Rocket::Core::Element* pElement, bool bHoldingRLReference);
	virtual GUIDocument* InstanceDocument(Rocket::Core::ElementDocument* pDocument, bool bHoldingRLReference);

	virtual void Release(GUIElement* pElement);

	// ~IGUIInstancer

private:

	typedef std::set<GUIElement*> TInstancedElements;

	Rocket::Core::Context* m_pContext;
	TInstancedElements m_instanced;
};

#endif // GUISYSTEM_INCLUDED
