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

enum RCppOptimizationLevel
{
	RCCPPOPTIMIZATIONLEVEL_DEFAULT = 0,		// RCCPPOPTIMIZATIONLEVEL_DEBUG in DEBUG, RCCPPOPTIMIZATIONLEVEL_PERF in release. This is the default state.
	RCCPPOPTIMIZATIONLEVEL_DEBUG,			// Low optimization, improve debug experiece. Default in DEBUG
	RCCPPOPTIMIZATIONLEVEL_PERF,			// Optimization for performance, debug experience may suffer. Default in RELEASE
	RCCPPOPTIMIZATIONLEVEL_NOT_SET,			// No optimization set in compile, so either underlying compiler default or set through SetAdditionalCompileOptions
	RCCPPOPTIMIZATIONLEVEL_SIZE,			// Size of enum, do not use to set opt level
};

static const char* RCppOptimizationLevelStrings[] = 
{
	"DEFAULT",		
	"DEBUG",			
	"PERF",			
	"NOT_SET",			
};

// GetActualOptimizationLevel - translates DEFAULT into DEUG or PERF
inline RCppOptimizationLevel GetActualOptimizationLevel( RCppOptimizationLevel optimizationLevel_ )
{
	if( RCCPPOPTIMIZATIONLEVEL_DEFAULT == optimizationLevel_ )
	{
	#ifdef _DEBUG
		optimizationLevel_ = RCCPPOPTIMIZATIONLEVEL_DEBUG;
	#else
		optimizationLevel_ = RCCPPOPTIMIZATIONLEVEL_PERF;
	#endif
	}
	return optimizationLevel_;
}