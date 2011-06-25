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

#ifndef GRAPHICSWINDOWDEF
#define GRAPHICSWINDOWDEF

class RenderWindowPlatformImpl;

class RendererWindow  
{
public:

	RendererWindow();
	~RendererWindow();
	bool Initialise();


	//the following functions return the member variables described below
	inline unsigned int		GetWindowWidth()		const { return m_uiWindowWidth;	}
	inline unsigned int		GetWindowHeight()		const { return m_uiWindowHeight;	}
	inline bool				GetIsShutdown()			const { return m_bShutdown;		}
	inline bool				GetIsOpen()				const { return m_bWindowOpen;	}
	inline bool				GetIsActive()			const { return m_bActive;		}
	inline bool				GetIsFullScreen()		const { return m_bFullScreen;	}

	void				SetWindowed(	unsigned int uiXpos_ = 0,
										unsigned int uiYpos_ = 0,
										unsigned int uiWidth_ = 533,
										unsigned int uiHeight_ = 400 );
	void				SetFullScreen();
	void				SetShutdown();



	//SwapBuffers - swaps bask and front buffers and processes window messages.
	void				SwapBuffers();

	void CloseWindow();

private:
	RenderWindowPlatformImpl*	m_pPlatformImpl;

	bool m_bShutdown;
	bool m_bActive;
	bool m_bWindowOpen;
	bool m_bFullScreen;
	bool m_bWMCloseCalled;
	bool m_bWMMouseInClient;
	unsigned int m_uiWindowWidth;
	unsigned int m_uiWindowHeight;
	unsigned int m_uiWindowXPos;
	unsigned int m_uiWindowYPos;


	//fullscreen display settings
	unsigned int m_uiWidth;
	unsigned int m_uiHeight;
	unsigned int m_uiBitsPerPixel;
};

#endif