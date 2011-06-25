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

#include "RenderWindow.h"

// Windows Requirements
#define WIN32_LEAN_AND_MEAN				// Exclude rarely-used stuff from Windows headers
#include <windows.h>


// OpenGL requirements
#include <GL/gl.h>


class RenderWindowPlatformImpl
{
public:
	RenderWindowPlatformImpl()
		: m_irInstance( NULL )
		, m_drDevice( NULL )
		, m_grGraphics( NULL )
		, m_wrWindow( NULL )
	{
	}
	HINSTANCE m_irInstance;
	HDC m_drDevice;
	HGLRC m_grGraphics;
	HWND m_wrWindow;
};


const wchar_t c_caAppName[80] = L"App";

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


RendererWindow::RendererWindow()
 : m_pPlatformImpl( NULL )
 , m_bFullScreen( false )
 , m_bWindowOpen( false )
 , m_bShutdown( false )
 , m_bActive( true )
 , m_bWMCloseCalled( false )
 , m_bWMMouseInClient( false )
 , m_uiWidth( 800 )
 , m_uiHeight( 600 )
 , m_uiWindowXPos( 100 )
 , m_uiWindowYPos( 100 )
 , m_uiBitsPerPixel( 32 )

{
}

RendererWindow::~RendererWindow()
{
	if(m_bWindowOpen) CloseWindow();
	delete m_pPlatformImpl;
}


bool RendererWindow::Initialise()
{
	m_pPlatformImpl = new RenderWindowPlatformImpl();
	m_pPlatformImpl->m_irInstance = GetModuleHandle(NULL);
	
	// Register window class - non extended version
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_pPlatformImpl->m_irInstance;
	wc.hIcon = NULL;	//LoadIcon(m_pPlatformImpl->m_irInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW +1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = c_caAppName;
    if( !RegisterClass(&wc) )
	{
		MessageBox(NULL, L"Failed to register window class", NULL, MB_ICONERROR | MB_OK);
		return false;
	}



    // Create window
	if( true == m_bFullScreen )
	{

		DEVMODE dmFullScreenMode;
		memset( &dmFullScreenMode, 0, sizeof( dmFullScreenMode ) );
		dmFullScreenMode.dmPelsWidth	= m_uiWidth;
		dmFullScreenMode.dmPelsHeight	= m_uiHeight;
		dmFullScreenMode.dmBitsPerPel	= m_uiBitsPerPixel;
		dmFullScreenMode.dmSize			= sizeof(dmFullScreenMode);
		dmFullScreenMode.dmFields		= DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
		ChangeDisplaySettings( &dmFullScreenMode, 0 );

		m_uiWindowWidth = GetSystemMetrics( SM_CXSCREEN );
		m_uiWindowHeight = GetSystemMetrics( SM_CYSCREEN );
		m_uiWindowXPos = 0;
		m_uiWindowYPos = 0;

		//TODO: need to pass pointer to this here with CreateWindowEx so we can use SetWindowLong data with this (map of hwnd to data)
		m_pPlatformImpl->m_wrWindow = CreateWindow(c_caAppName, c_caAppName,
                        WS_POPUPWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                        0, 0, 
						m_uiWindowWidth, m_uiWindowHeight,
                        NULL, NULL, m_pPlatformImpl->m_irInstance, NULL);
	}
	else	
	{
		m_uiWindowWidth = m_uiWidth;
		m_uiWindowHeight = m_uiHeight;
		m_uiWindowXPos = 0;
		m_uiWindowYPos = 0;
	    m_pPlatformImpl->m_wrWindow = CreateWindow(c_caAppName, c_caAppName,
                        WS_POPUPWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS
						| WS_CAPTION | WS_MINIMIZEBOX,
                        0, 0, 
						m_uiWindowWidth, m_uiWindowHeight,
                        NULL, NULL, m_pPlatformImpl->m_irInstance, NULL);
	}

	if( 0 == m_pPlatformImpl->m_wrWindow )
	{
		//error, so close the 'window'
		CloseWindow();
		MessageBox(NULL, L"Failed to create window", NULL, MB_ICONERROR | MB_OK);
		return false;
	}
	
	
	m_pPlatformImpl->m_drDevice = GetDC(m_pPlatformImpl->m_wrWindow);


	PIXELFORMATDESCRIPTOR pfd = { 
	    sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd 
	    1,                     // version number 
	    PFD_DRAW_TO_WINDOW |   // support window 
	    PFD_SUPPORT_OPENGL |   // support OpenGL 
	    PFD_DOUBLEBUFFER,      // double buffered 
	    PFD_TYPE_RGBA,         // RGBA type 
	    16,                    // 16-bit color depth 
	    0, 0, 0, 0, 0, 0,      // color bits ignored 
	    0,                     // no alpha buffer 
	    0,                     // shift bit ignored 
	    0,                     // no accumulation buffer 
	    0, 0, 0, 0,            // accum bits ignored 
	    16,                    // 16-bit z-buffer	 
	    0,                     // no stencil buffer 
	    0,                     // no auxiliary buffer 
	    PFD_MAIN_PLANE,        // main layer 
	    0,                     // reserved 
	    0, 0, 0                // layer masks ignored 
	}; 
 
	int iPixelFormat = ChoosePixelFormat(m_pPlatformImpl->m_drDevice, &pfd);
	SetPixelFormat(m_pPlatformImpl->m_drDevice, iPixelFormat, &pfd);

	m_pPlatformImpl->m_grGraphics = wglCreateContext(m_pPlatformImpl->m_drDevice);
	if( 0 == m_pPlatformImpl->m_grGraphics )
	{
		//error, so close the 'window'
		CloseWindow();
		MessageBox(NULL, L"Failed to create OpenGL context.", NULL, MB_ICONERROR | MB_OK);
		return false;
	}

	wglMakeCurrent(m_pPlatformImpl->m_drDevice, m_pPlatformImpl->m_grGraphics);
	if( 0 == m_pPlatformImpl->m_grGraphics )
	{
		//error, so close the 'window'
		CloseWindow();
		MessageBox(NULL, L"Failed to make OpenGL context current.", NULL, MB_ICONERROR | MB_OK);
		return false;
	}

	//at this stage should check to see if 

	ShowWindow(m_pPlatformImpl->m_wrWindow, SW_SHOW);
    UpdateWindow(m_pPlatformImpl->m_wrWindow);

	m_bWindowOpen = true;

	//initExtensions();	//initialise OpenGL extensions

	return true;
}


void RendererWindow::CloseWindow()
{
	if(m_bWindowOpen) 
	{
 		m_bWindowOpen = false;
		if( 0 != m_pPlatformImpl->m_grGraphics )
		{
			// make the rendering context not current         
			wglMakeCurrent(NULL, NULL) ; 

			// delete the rendering context         
			wglDeleteContext( m_pPlatformImpl->m_grGraphics );
		}
	}


	if( true == m_bFullScreen )
	{
		ChangeDisplaySettings( NULL, 0 );
		m_bFullScreen = false;	//reset fullscreen parameter.
	}
}



void RendererWindow::SetWindowed(unsigned int uiXpos_, unsigned int uiYpos_,
					unsigned int uiWidth_, unsigned int uiHeight_)
{
	m_uiWindowWidth = uiWidth_;
	m_uiWindowHeight = uiHeight_;
	m_uiWindowXPos = uiXpos_;
	m_uiWindowYPos = uiYpos_;
	if( m_bWindowOpen && (true == m_bFullScreen))
	{
		//window is open and currently fullscreen, so change to windowed
		SetWindowLong(m_pPlatformImpl->m_wrWindow, GWL_STYLE,
			GetWindowLong(m_pPlatformImpl->m_wrWindow, GWL_STYLE)
			| WS_CAPTION | WS_MINIMIZEBOX);	//style bordered
		MoveWindow(m_pPlatformImpl->m_wrWindow, m_uiWindowXPos, m_uiWindowYPos,
					m_uiWindowWidth, m_uiWindowHeight, TRUE);
		RECT rctWindow;
		GetClientRect( m_pPlatformImpl->m_wrWindow, &rctWindow );
		m_uiWindowHeight = rctWindow.bottom;
		m_uiWindowWidth = rctWindow.right;
		glViewport(0, 0, m_uiWindowWidth, m_uiWindowHeight);
		RedrawWindow(NULL, NULL, NULL,
			RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			//tell windows to redraw the screen

		ChangeDisplaySettings( NULL, 0 );
	}
	m_bFullScreen = false;
}

void RendererWindow::SetFullScreen()
{
	if( m_bWindowOpen )
	{
		//window is open and currently windowed, so set to fullscreen
		DEVMODE dmFullScreenMode;
		memset( &dmFullScreenMode, 0, sizeof( dmFullScreenMode ) );
		dmFullScreenMode.dmPelsWidth	= m_uiWidth;
		dmFullScreenMode.dmPelsHeight	= m_uiHeight;
		dmFullScreenMode.dmBitsPerPel	= m_uiBitsPerPixel;
		dmFullScreenMode.dmSize			= sizeof(dmFullScreenMode);
		dmFullScreenMode.dmFields		= DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
		ChangeDisplaySettings( &dmFullScreenMode, 0 );

		m_uiWindowWidth = GetSystemMetrics( SM_CXSCREEN );
		m_uiWindowHeight = GetSystemMetrics( SM_CYSCREEN );
		m_uiWindowXPos = 0;
		m_uiWindowYPos = 0;

		SetWindowLong(m_pPlatformImpl->m_wrWindow, GWL_STYLE, 
			GetWindowLong(m_pPlatformImpl->m_wrWindow, GWL_STYLE)
			& ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX));	//style no border
		MoveWindow(m_pPlatformImpl->m_wrWindow, 0, 0, m_uiWindowWidth, m_uiWindowHeight, TRUE);
		glViewport(0, 0, m_uiWindowWidth, m_uiWindowHeight);
	}
	m_bFullScreen = true;
}



LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg,
                         WPARAM wParam, LPARAM lParam)
{
	//NEED TO IMPLEMENT
    switch(uMsg)
    {
       case WM_DESTROY:

			PostQuitMessage(0);
			return 0;
 
		default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void RendererWindow::SetShutdown()
{
	PostMessage( m_pPlatformImpl->m_wrWindow, WM_CLOSE, NULL, NULL );
}


void RendererWindow::SwapBuffers()
{
	if( ( 0 != m_pPlatformImpl->m_grGraphics ) && ( 0 != m_pPlatformImpl->m_drDevice ) )
	{
		::SwapBuffers(m_pPlatformImpl->m_drDevice);
	}

	//do windows messaging after swapping
	MSG msg;
	while( PeekMessage(&msg, m_pPlatformImpl->m_wrWindow, 0, 0, PM_REMOVE) )
	{
		DispatchMessage(&msg);
	}
}
