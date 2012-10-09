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

#ifndef DEFINITIONS_DEFINED
#define DEFINITIONS_DEFINED

#ifndef _WIN32
	//the following won't work with the templated variety
    #define _snprintf_s(a,b,c,...) snprintf(a,b,__VA_ARGS__)
#endif

// All typedefs, defines and macros start AU_ to avoid conflicts


typedef int AUEntityId;           // (Will be a) salted id for uniquely identifying entities

struct IRuntimeObjectSystem;
struct IEntitySystem;
struct ITimeSystem;
struct ILogSystem;
struct IAssetSystem;

struct IAUEntity;
struct IAURenderable;
struct IAURenderableMesh;
struct IAUUpdateable;
struct ISimpleSerializer;
struct IObjectFactorySystem;
struct IGUISystem;
struct IFileChangeNotifier;
struct IGame;
class CalSound;

#endif // DEFINITIONS_DEFINED