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

#include "../ITimeSystem.h"

// Consider adding a subclassed version that know about logging

class TimeSystem : public ITimeSystem
{
public:
	TimeSystem();
	~TimeSystem();

	// IsWithinSession
	void StartSession(); 
	void EndSession();	

	// IsWithinFrame
	void StartFrame();
	void EndFrame();

	bool IsPaused() const;            
	void Pause(bool bPause);    

	double GetFramePlayTime() const;  
	double GetFrameSessionTime() const; 

	double GetPlayTimeNow() const;    
	double GetSessionTimeNow() const;   
	double GetFrameTimeNow() const;

	double GetLastFrameDuration() const;  
	double GetSmoothFrameDuration() const;

private:

	void Reset();
	double GetRawTime() const;

	// All refer to the start of the current frame. The _current_ time values are obviously not worth storing.
	double m_dSessionStartRaw; // The the "real" time that the session started
	double m_dFramePlayTime;    // How much unpaused time had elapsed when we started the frame?
	double m_dFrameSessionTime;   // How much total time had elapsed...
	double m_dPausedSince;			// 
	double m_dSessionPausedTime;// Accumulated paused time this session, up to the last time we unpaused. Is this too unhygenic?
	double m_dLastFrameDuration;    // How long did the last frame actually take?
	double m_dSmoothFrameDuration;  // How long do we guess this frame will take?

	bool m_bWithinSession, m_bWithinFrame;
	bool m_bPaused, m_bPausedNextFrame;
    
#ifdef _WIN32
	typedef __int64 INT64;
	INT64 m_iPerformanceFreq;   // Divisor for performance frequency values
#endif
};
