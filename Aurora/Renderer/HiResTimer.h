//////////////////////////////////////////////////////////////////////////////////////////
// File:				"CHiResTimer.h"
//
// Related Source File:		"CHiResTimer.cpp"
//
// Original Author:			Douglas John Binks (DJB)
//
// Creation Date:			03 Feb 2000
//
// Specification Document:	DJBTODO
//
// Purpose:					Defines the interface for the CHiResTimer Class, which
//							provides a high resolution timer (better than 1ms, and
//							in some cases as good as 1us).
//							This timer is differential - ie it counts the time since
//							the timer was created.
//
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef HiResTimerDef
#define HiResTimerDef

class CHiResTimer  
{
public:
	CHiResTimer();

	void Reset();

	double GetElapsedSecs() const;

private:

#ifdef WIN32
	static double m_sdTimerFreq;
	__int64 m_i64Win32StartTime;
#else
	struct timeval m_tvLinuxStartTime;
#endif

};

#endif 