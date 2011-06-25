/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2000 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */
 
#define TARGET_CARBON 0

// includes
#include <Windows.h>
#include <ToolUtils.h>
#include <Controls.h>
#include "alut.h"

// EAX-test
#define OPENAL
#define INITGUID
#include "eax.h"

// defines
#define MAXLONG			0x7FFFFFFF

#define rNewWindow		128 // window ID

#define rMenubar		128 // menu stuff
#define mApple			128
#define iAbout			1
#define mFile			129
#define iQuit			11
#define mEdit			130
#define mSelect			131

#define cButtonPlay		128 // control IDs
#define cButtonStart  	129
#define cButtonPause    130
#define cButtonStop		131
#define cButtonPlusX    132
#define cButtonMinusX   133
#define cButtonPlusY    134
#define cButtonMinusY   135
#define cButtonPlusZ    136
#define cButtonMinusZ   137
#define cSelect  		138

#define rTextStringList 128 // string resources


// typedefs
typedef struct
{
	ControlHandle hBtnPlay;
	ControlHandle hBtnStart;
	ControlHandle hBtnPause;
	ControlHandle hBtnStop;
	ControlHandle hBtnPlusX;
	ControlHandle hBtnMinusX;
	ControlHandle hBtnPlusY;
	ControlHandle hBtnMinusY;
	ControlHandle hBtnPlusZ;
	ControlHandle hBtnMinusZ;
	ControlHandle hSelect;
} DocRec; // holds control handles

typedef DocRec **DocRecHandle;

// prototypes
void Initmanagers(void);
void CreateMainMenu(void);
void CreateMainWindow(void);
void ProcessEvent(EventRecord *);
void ProcessMouseDown(EventRecord *);
void ProcessMenuChoice(SInt32);
void ProcessEvent (EventRecord *);
void Activate(EventRecord *);
void ActivateWindow(WindowPtr, Boolean);
void OSEvent(EventRecord *);
void ProcessControl(EventRecord *, WindowPtr);
void UpdateLoc();
void Update(EventRecord *);
void GetControls(WindowPtr);

// globals
Boolean gDone;  // indicates when main loop should be exited
Boolean gbInBackground;  // indicates when program is in background
unsigned int gSourceID[2]; // two source IDs
unsigned int gSampleSet[4]; // two sample set ID numbers
float gLoc0[3]; // location of source #0
EAXSet pfPropSet;
EAXGet pfPropGet;
Boolean bEAXDetected;

// main
int main(void)
{
	EventRecord	EventRec;
	DocRecHandle hDocRec; 
	char *pBuffer1, *pBuffer2;
	long lBuffer1Len, lBuffer2Len;
	ALenum formatBuffer1;
	ALsizei freqBuffer1;
	FSSpec sfFile;
	Str255 sFileName;
	short fRefNum;
	SInt16 iNumSources, iNumSampleSets;
	// EAX test
	unsigned long ulEAXVal;
	long lGlobalReverb;
	
	// init globals
	gDone = false; // we haven't started yet...
	gLoc0[0] = 0;
	gLoc0[1] = 0;
	gLoc0[2] = 0;
	
	// initialize OpenAL
	alutInit(NULL, 0);
	
	// EAX test -- set EAX environment if EAX is available
	if (alIsExtensionPresent((ALubyte *)"EAX") == AL_TRUE)
	{
		pfPropSet = alGetProcAddress((ALubyte *)"EAXSet");
		if (pfPropSet != NULL)
		{
		    bEAXDetected = true;
		    
		    lGlobalReverb = 0; // covers attenuation range of -10000 (-100dB) to 0 (0dB)
			pfPropSet((ALvoid *)&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOM, 0, &lGlobalReverb, sizeof(unsigned long));
			ulEAXVal = EAX_ENVIRONMENT_GENERIC;
			pfPropSet((ALvoid *)&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENT, 0, &ulEAXVal, sizeof(unsigned long));
		} else
		{
		    bEAXDetected = false;
		}
	}

	// load up some audio data...
	alutLoadWAVFile((char *)"\pboom.wav",&formatBuffer1, (void **) &pBuffer1,(unsigned int *)&lBuffer1Len,&freqBuffer1);
	
	lBuffer2Len = 1200000; // max amount of data to read
	pBuffer2 = NewPtrClear(lBuffer2Len); // create a long buffer 
	if (pBuffer2 == NULL)
	{
		gDone = true;  // don't let program run if can't get buffer space
	} else
	{
		if (FSMakeFSSpec(0,0,"\pfunk_s.pcm",&sfFile) == 0)
		{
			FSpOpenDF(&sfFile, fsRdPerm, &fRefNum); // load some data into the buffer
			FSRead(fRefNum, &lBuffer2Len, pBuffer2);			
 			FSClose(fRefNum);
		}
	}

	// generate two OpenAL sample sets and two sources
	iNumSources = 2;
	iNumSampleSets = 4;
	alGenSources(iNumSources, &gSourceID[0]);
	if (iNumSources == 2)  // FIXME: really should have error-checking here
	{
		alGenBuffers(iNumSampleSets, &gSampleSet[0]);
		if (iNumSampleSets == 4)  // FIXME: need error-checking here
		{
		    // now have two buffers and two sources, so need to fill buffers and attach to sources
			alBufferData(gSampleSet[0], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);
			alSourcei(gSourceID[0], AL_BUFFER, gSampleSet[0]);
			
			alBufferData(gSampleSet[1], AL_FORMAT_STEREO16, pBuffer2, lBuffer2Len/3, 22050);
			alBufferData(gSampleSet[2], AL_FORMAT_STEREO16, (pBuffer2 + (lBuffer2Len/3)), lBuffer2Len/3, 22050);
			alBufferData(gSampleSet[3], AL_FORMAT_STEREO16, (pBuffer2 + 2 * (lBuffer2Len/3)), lBuffer2Len/3, 22050);
			alQueuei(gSourceID[1], AL_BUFFER, gSampleSet[1]);
			alQueuei(gSourceID[1], AL_BUFFER, gSampleSet[2]);
			alQueuei(gSourceID[1], AL_BUFFER, gSampleSet[3]);
		}
	}
	 
	// get rid of un-needed buffer pointers
	alutUnloadWAV(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);
	if (pBuffer2 != NULL)
	{
		DisposePtr(pBuffer2);
	}
	
	Initmanagers();
	CreateMainMenu();
	CreateMainWindow();
	
	// main event loop
	while (!gDone)  
	{
	 	if (WaitNextEvent(everyEvent,&EventRec, MAXLONG ,NULL)) 
	 		ProcessEvent(&EventRec);
	}
	
	
	// terminate OpenAL
	alutExit();

	return 0; 	
}

// initialize OS managers
void Initmanagers(void)
{
#if !(TARGET_CARBON)
	MaxApplZone();
	MoreMasters();
	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
#endif
	InitCursor();
	
	FlushEvents(everyEvent, 0);
}

// create the main menu
void CreateMainMenu (void)
{
	Handle hMenuBar;
	MenuHandle hMenu;
	
	hMenuBar = GetNewMBar(rMenubar);
	if (hMenuBar == NULL)
	{
	 	ExitToShell ();
	}
	SetMenuBar(hMenuBar);
	DrawMenuBar();
	
	hMenu = GetMenuHandle(mApple);
	if (hMenu == NULL)
	{
	 	ExitToShell();
	} else
	{
/* ***** #if TARGET_CARBON
		DeleteMenuItem(hMenu, iQuit);
		DeleteMenuItem(hMenu, iQuit - 1);
#else */
		AppendResMenu(hMenu, 'DRVR');
//#endif
	}
}
	 

// create the main window
void CreateMainWindow (void)
{
	WindowRef	pWindow;
	DocRecHandle hDocRec;
	
	/*     // can create main window without a resource...
	Rect	windRect;
	windRect = qd.screenBits.bounds;
	InsetRect(&windRect, 50, 50);
	pWindow = NewCWindow(nil, &windRect, "\pMinApp", true, documentProc, 
						(WindowPtr) -1, false, 0);
	*/
	
	pWindow = GetNewWindow(rNewWindow, NULL, (WindowRef) - 1L);
		
	SetPort((GrafPtr) GetWindowPort(pWindow));						// set window to current graf port
	
	if (!(hDocRec = (DocRecHandle) NewHandle(sizeof(DocRec))))
		ExitToShell();
		
	SetWRefCon(pWindow, (SInt32) hDocRec);  // attach control handle structure to the window
	GetControls(pWindow);  // get controls for window from the resource fork
	ShowWindow(pWindow);
}

// process an event
void ProcessEvent(EventRecord *pEventRec)
{
	SInt8 charCode;
	
	switch(pEventRec->what)
	{
		case mouseDown:
			ProcessMouseDown(pEventRec);
			break;
			
		case keyDown:
		case autoKey:
			charCode = pEventRec->message & charCodeMask;
			if((pEventRec->modifiers & cmdKey) != 0)
			{
				ProcessMenuChoice(MenuKey(charCode));
			}
			break;
			
		case activateEvt:
			Activate(pEventRec);
			break;
		
		case osEvt:
			OSEvent(pEventRec);
			HiliteMenu(0);
			break;
			
		case updateEvt:
			Update(pEventRec);
			break;
	}
}

// process mouseDown event
void ProcessMouseDown(EventRecord *pEventRec)
{
	WindowPtr	pWindow;
	SInt16		PartCode;
	RgnHandle   grayRgn;
	Rect		bounds;

	PartCode = FindWindow(pEventRec->where, &pWindow);
	
	switch(PartCode)
	{
		case inMenuBar:
			ProcessMenuChoice(MenuSelect(pEventRec->where));
			break;
			
#if !(TARGET_CARBON)
		case inSysWindow:
			SystemClick(pEventRec, pWindow);
			break;
#endif
			
		case inContent:
			if (pWindow != FrontWindow())
				SelectWindow(pWindow);
			else
				ProcessControl(pEventRec, pWindow);
			break;
			
		case inDrag:
		    grayRgn = GetGrayRgn();
#if TARGET_CARBON
			GetRegionBounds(grayRgn, &bounds);
#else
			bounds = grayRgn[0]->rgnBBox;
#endif
			DragWindow(pWindow, pEventRec->where, &bounds);
			break;
			
		case inGoAway:
			if (TrackGoAway(pWindow, pEventRec->where) == true)
				gDone = true;
			break;
	}
}

// process menu selection
void ProcessMenuChoice(SInt32 MenuChoice)
{
	SInt16	MenuID, MenuItem;
	Str255	ItemName;
	SInt16	daDriverRefNum;

	
	MenuID = HiWord(MenuChoice);
	MenuItem = LoWord(MenuChoice);
	
	if (MenuID == 0) return;
	
	switch(MenuID)
	{
		case mApple:
		  	if (MenuItem == iAbout)
		  		SysBeep(10);
#if !(TARGET_CARBON)
		  	else
		  	{
				GetMenuItemText(GetMenuHandle(mApple),MenuItem,ItemName);
		  		daDriverRefNum = OpenDeskAcc(ItemName);
		  	}
#endif
		  	break;
		  	
		 case mFile:
		 	if (MenuItem == iQuit)
		 		gDone = true;
		 	break;
	}
	
	HiliteMenu (0);
}

// Activate
void Activate (EventRecord *pEventRec)
{
	WindowPtr	pWindow;
	Boolean		bBecomingActive;
	
	pWindow = (WindowPtr) pEventRec->message;
	
	bBecomingActive = ((pEventRec->modifiers & activeFlag) == activeFlag);
	
	ActivateWindow(pWindow, bBecomingActive);
}

// ActivateWindow
void ActivateWindow (WindowPtr pWindow, Boolean bBecomingActive)
{
	DocRecHandle	hDocRec;
	SInt16	HiliteState;
	
	hDocRec = (DocRecHandle) (GetWRefCon(pWindow));
	
	if (bBecomingActive)
		HiliteState = 0;
	else
		HiliteState = 255;
		
	HiliteControl((*hDocRec)->hBtnPlay, HiliteState);
	HiliteControl((*hDocRec)->hBtnStart, HiliteState);
	HiliteControl((*hDocRec)->hBtnPause, HiliteState);
	HiliteControl((*hDocRec)->hBtnStop, HiliteState);
}

// OSEvent
void OSEvent(EventRecord *pEventRec)
{
	switch ((pEventRec->message >> 24) & 0x000000FF)
	{
		case suspendResumeMessage:
			// DrawGrowIcon(FrontWindow());
			gbInBackground = ((pEventRec->message & resumeFlag) == 0);
			ActivateWindow(FrontWindow(), !gbInBackground);
			break;
			
		case mouseMovedMessage:
			break;
	}
}

// ProcessControl -- figure out which control has been activated and react
void ProcessControl(EventRecord *pEventRec, WindowPtr pWindow)
{
	Point mouseXY;
	ControlPartCode partCode;
	ControlHandle hControl;
	DocRecHandle hDocRec;
	SInt16 controlValue;
	unsigned long ulEAXVal;
	ALint tempInt;
	
	mouseXY = pEventRec->where;
	GlobalToLocal(&mouseXY);
	
	if (partCode = FindControl(mouseXY, pWindow, &hControl))
	{
		hDocRec = (DocRecHandle) (GetWRefCon(pWindow));
		if (hControl == (*hDocRec)->hBtnPlay)
		{
			// play short sound if not already playing
			alGetSourcei(gSourceID[0], AL_SOURCE_STATE, &tempInt);
			if (tempInt != AL_PLAYING)
			{
				alSourcePlay(gSourceID[0]);
			}
		}
		if (hControl == (*hDocRec)->hBtnStart)
		{
			// play long stream
			alSourcei(gSourceID[1], AL_LOOPING, AL_TRUE);
			alSourcePlay(gSourceID[1]);
		}
		if (hControl == (*hDocRec)->hBtnPause)
		{
			alSourcePause(gSourceID[1]);
		}
		if (hControl == (*hDocRec)->hBtnStop)
		{
			alSourceStop(gSourceID[1]);
		}
		if (hControl == (*hDocRec)->hBtnPlusX)
		{
			gLoc0[0] += 1;
			alSourcefv(gSourceID[0], AL_POSITION, gLoc0);
			UpdateLoc();
		}
		if (hControl == (*hDocRec)->hBtnMinusX)
		{
			gLoc0[0] -= 1;
			alSourcefv(gSourceID[0], AL_POSITION, gLoc0);
			UpdateLoc();
		}
		if (hControl == (*hDocRec)->hBtnPlusY)
		{
			gLoc0[1] += 1;
			alSourcefv(gSourceID[0], AL_POSITION, gLoc0);
			UpdateLoc();
		}
		if (hControl == (*hDocRec)->hBtnMinusY)
		{
			gLoc0[1] -= 1;
			alSourcefv(gSourceID[0], AL_POSITION, gLoc0);
			UpdateLoc();
		}
		if (hControl == (*hDocRec)->hBtnPlusZ)
		{
			gLoc0[2] += 1;
			alSourcefv(gSourceID[0], AL_POSITION, gLoc0);
			UpdateLoc();
		}
		if (hControl == (*hDocRec)->hBtnMinusZ)
		{
			gLoc0[2] -= 1;
			alSourcefv(gSourceID[0], AL_POSITION, gLoc0);
			UpdateLoc();
		}
		if (hControl == (*hDocRec)->hSelect)
		{
			TrackControl(hControl,mouseXY,(ControlActionUPP) -1);
			controlValue = GetControlValue(hControl);
			if (alIsExtensionPresent((ALubyte *)"EAX") == AL_TRUE)
			{
				if (pfPropSet != NULL)
				{
					ulEAXVal = (unsigned long) controlValue - 1;
					pfPropSet((ALvoid *)&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENT, 0, &ulEAXVal, sizeof(unsigned long));
				}
			}	
		}	
	}
}

// UpdateLoc -- update displayed location
void UpdateLoc ()
{
	Str255	textString;
	Rect    clearRect;
	Pattern whitePattern;
	
#if TARGET_CARBON
	GetQDGlobalsWhite(&whitePattern);
#else
	whitePattern = qd.white;
#endif
	
	SetRect(&clearRect, 80, 80, 190, 120);
	FillRect(&clearRect, &whitePattern);

	sprintf((char *) textString, "((%d, %d, %d)", (int) gLoc0[0], (int) gLoc0[1], (int) gLoc0[2]);
	textString[0] = strlen(textString);
	MoveTo(95, 100);
	DrawString(textString);
}

// Update -- update window display
void Update (EventRecord *pEventRec)
{
	WindowRef pWindow;
	RgnHandle visibleRgn = NULL;
	Str255	textString;
	
	pWindow = (WindowRef) pEventRec->message;
	
	BeginUpdate(pWindow); // NOTE:  BeginUpdate/EndUpdate are critical to process of putting a program in the background/foreground

#if TARGET_CARBON
	visibleRgn = NewRgn();
	GetPortVisibleRegion(GetWindowPort(pWindow), visibleRgn);
#else
	visibleRgn = pWindow->visRgn;
#endif
		
	if (!EmptyRgn(visibleRgn))
	{
		SetPort((GrafPtr) GetWindowPort(pWindow));
		UpdateControls(pWindow, visibleRgn);
		
		GetIndString(textString, rTextStringList, 1);
		MoveTo(45,45);
		DrawString(textString);
		
		GetIndString(textString, rTextStringList, 2);
		MoveTo(45,155);
		DrawString(textString);
		
		if (bEAXDetected == true)
		{
			GetIndString(textString, rTextStringList, 3);
			MoveTo(45,305);
			DrawString(textString);
		}
		
		UpdateLoc();
	}

#if TARGET_CARBON
	DisposeRgn(visibleRgn);
#endif
	
	EndUpdate(pWindow);
}

// GetControls -- attach resources to hDocRec for window
void GetControls(WindowPtr pWindow)
{
	DocRecHandle hDocRec;
	
	hDocRec = (DocRecHandle) (GetWRefCon(pWindow));
	
	// put handles to the controls into hDocRec
	if (!((*hDocRec)->hBtnPlay = GetNewControl(cButtonPlay, pWindow)))
		ExitToShell();
		
	if (!((*hDocRec)->hBtnStart = GetNewControl(cButtonStart, pWindow)))
		ExitToShell();
		
	if (!((*hDocRec)->hBtnPause = GetNewControl(cButtonPause, pWindow)))
		ExitToShell();
		
	if (!((*hDocRec)->hBtnStop = GetNewControl(cButtonStop, pWindow)))
		ExitToShell();
		
	if (!((*hDocRec)->hBtnPlusX = GetNewControl(cButtonPlusX, pWindow)))
		ExitToShell();
		
	if (!((*hDocRec)->hBtnMinusX = GetNewControl(cButtonMinusX, pWindow)))
		ExitToShell();
		
	if (!((*hDocRec)->hBtnPlusY = GetNewControl(cButtonPlusY, pWindow)))
		ExitToShell();
		
	if (!((*hDocRec)->hBtnMinusY = GetNewControl(cButtonMinusY, pWindow)))
		ExitToShell();
	
	if (!((*hDocRec)->hBtnPlusZ = GetNewControl(cButtonPlusZ, pWindow)))
		ExitToShell();
		
	if (!((*hDocRec)->hBtnMinusZ = GetNewControl(cButtonMinusZ, pWindow)))
		ExitToShell();
	
	if (bEAXDetected == true)
	{	
		if (!((*hDocRec)->hSelect = GetNewControl(cSelect, pWindow)))
			ExitToShell();
    }
}


