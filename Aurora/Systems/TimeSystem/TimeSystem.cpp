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

#include "TimeSystem.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"      // For QueryPerformanceCounter
#else
#include <sys/time.h>
#include <unistd.h>
#endif
#include "assert.h"
#include <GL/glfw.h>

/*
	I'm not certain if I'm using the best datatype here for storing the time, and I suspect the divisions could be put off.
	However, it should be more than accurate enough and it is all hidden behind an interface that just presents seconds as doubles.
	Later, this might be worth a rewrite to be really hygenic.
	Returning a 64-bit integer format would also be desirable.
	There is some duplication around pausing.
*/

TimeSystem::TimeSystem(void)
{
	m_dSmoothFrameDuration = 0.01f; // Assume 100Hz for smoothing to start with

#ifdef _WIN32	// Note that IIRC this can change with CPU frequency. Demoing on laptops remember!
	QueryPerformanceFrequency( (LARGE_INTEGER*) &m_iPerformanceFreq );
	assert(m_iPerformanceFreq); // Consider quitting with error
#endif
	Reset();
}

TimeSystem::~TimeSystem(void)
{}

void TimeSystem::StartSession()
{
	assert(!m_bWithinSession);
	Reset(); // Reset the universe
	m_dSessionStartRaw = GetRawTime();
	m_bWithinSession = true;
}

void TimeSystem::EndSession()
{
	assert(m_bWithinSession);
	Reset(); // Reset the universe
	m_bWithinSession = false;
}

void TimeSystem::StartFrame()
{
	assert(!m_bWithinFrame);

	double dSessionTime= GetSessionTimeNow();

	//Update last frame duration
	m_dLastFrameDuration = dSessionTime - m_dFrameSessionTime;
	m_dSmoothFrameDuration = (m_dSmoothFrameDuration * 0.9 + m_dLastFrameDuration * 0.1);
	

	// Update total time
	m_dFrameSessionTime = dSessionTime;
	
	//Update unpaused time
	double dTotalPausedTime = m_dSessionPausedTime; // A "small" number
	if (m_bPaused)
		dTotalPausedTime += (m_dFrameSessionTime - m_dPausedSince);   // small + (large - large)
	m_dFramePlayTime = m_dFrameSessionTime - dTotalPausedTime;

	// Handle pause status
	if (m_bPaused != m_bPausedNextFrame)
	{
		if (m_bPausedNextFrame)
		{
			m_dPausedSince = m_dFrameSessionTime;
		}
		else
		{
			m_dSessionPausedTime = dTotalPausedTime;
			m_dPausedSince = 0.0f; // Just for clarity
		}
		m_bPaused = m_bPausedNextFrame;
	}

	m_bWithinFrame = true;
}


void TimeSystem::EndFrame()
{
	assert(m_bWithinFrame);
	
	m_bWithinFrame = false;
}

bool TimeSystem::IsPaused() const
{ return m_bPaused; }

void TimeSystem::Pause(bool bPause)
{
	assert(!m_bWithinFrame);

	// This is all we do here - we can't adjust timers because in principle, we don't know when the next frame will start.
	m_bPausedNextFrame = bPause;
}

double TimeSystem::GetFramePlayTime() const
{ return m_dFramePlayTime; }

double TimeSystem::GetFrameSessionTime() const
{	return m_dFrameSessionTime; }

double TimeSystem::GetPlayTimeNow() const
{
	// Quite unhappy with this
	double dSessionTime = GetSessionTimeNow();
	double dTotalPausedTime = m_dSessionPausedTime; // A "small" number
	if (m_bPaused)
		dTotalPausedTime += (dSessionTime - m_dPausedSince);   // small + (large - large)
	return dSessionTime - dTotalPausedTime;
}

double TimeSystem::GetSessionTimeNow() const
{
	return GetRawTime() - m_dSessionStartRaw;
}

double TimeSystem::GetFrameTimeNow() const
{
	return GetSessionTimeNow() - m_dFrameSessionTime;
}

double TimeSystem::GetLastFrameDuration() const
{ return m_dLastFrameDuration; }

double TimeSystem::GetSmoothFrameDuration() const
{ return m_dSmoothFrameDuration; }


double TimeSystem::GetRawTime() const
{

    double seconds = glfwGetTime();
    
	return seconds;
}

void TimeSystem::Reset()
{
	m_dSessionStartRaw = 0.0f;
	m_dFramePlayTime = 0.0f;
	m_dFrameSessionTime = 0.0f;
	m_dPausedSince = 0.0f;
	m_dSessionPausedTime = 0.0f;
	m_dLastFrameDuration = 0.0f;
	// Leave the smoothed time alone
	m_bWithinSession = false;
	m_bWithinFrame = false;
	m_bPaused = false;
	m_bPausedNextFrame = false;
}