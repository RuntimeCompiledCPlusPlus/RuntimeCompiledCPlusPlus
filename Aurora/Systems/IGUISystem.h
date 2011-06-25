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

#ifndef IGUISYSTEM_INCLUDED
#define IGUISYSTEM_INCLUDED

// Simple wrapper around libRocket to provide basic functionality to entities that need to 
// manipulate the GUI. Quite barebones for now, and we can add specific features as needed

#include "ISystem.h"
#include "IGUIDocument.h"
#include "IGUIElement.h"

// Forward declare Rocket::Core::Context
namespace Rocket { namespace Core { class Context; } }


struct IGUISystem : public ISystem
{
	//// ISystem methods
	//SErrorDescriptor UnitTest(ILogSystem *pLog);

	virtual void SetContext(Rocket::Core::Context* pContext) = 0;
	virtual void ClearStyleSheetCache() = 0;

	virtual IGUIDocument* LoadDocument(const char* file, const char* id) = 0;
	virtual IGUIDocument*	GetDocument(const char* id) = 0;
	virtual IGUIElement* GetLogElement() = 0;
};

#endif // IGUISYSTEM_INCLUDED