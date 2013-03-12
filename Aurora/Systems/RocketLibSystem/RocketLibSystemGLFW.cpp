/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "RocketLibSystem.h"
#include <Rocket/Core.h>
#include "InputGLFW.h"
#include "RocketLibSystemFileInterface.h"
#include <stdio.h>
#include <GL/glfw.h>

#include "../Systems.h"
#include "../IAssetSystem.h"

//#ifndef _WIN32
//#include <libproc.h>
//#endif

void GLFWCALL WindowResize( int width, int height );


static bool running = true;
static int g_WindowSize[4];


static RocketLibSystemFileInterface* file_interface = NULL;

bool RocketLibSystem::Initialise()
{
	if( GL_FALSE == glfwInit() )
	{
		return false;
	}
	glfwSetTime(0.0);

    const char* pathToAssets = gSys->pAssetSystem->GetAssetDirectory();
    file_interface = new RocketLibSystemFileInterface(Rocket::Core::String(pathToAssets));
    
	Rocket::Core::SetFileInterface(file_interface);

	return true;
}

void RocketLibSystem::Shutdown()
{
	glfwTerminate();

	delete file_interface;
	file_interface = NULL;
}

void RocketLibSystem::GetViewport( int WindowSize[4] )
{
	memcpy( WindowSize, g_WindowSize, sizeof( g_WindowSize ) );
}

bool RocketLibSystem::OpenWindow(const char* name, bool attach_opengl)
{

	glfwOpenWindow( 640, 768, 8, 8, 8, 8, 24, 8, GLFW_WINDOW );
	glfwSetWindowTitle( name );
	glfwSetWindowSizeCallback( WindowResize );
	glGetIntegerv( GL_VIEWPORT, g_WindowSize);

	InputGLFW::Initialise();


    return true;
}

void RocketLibSystem::CloseWindow()
{
	glfwCloseWindow();
}

// Flips the OpenGL buffers.
void RocketLibSystem::FlipBuffers()
{
	glfwSwapBuffers();
}

void RocketLibSystem::EventLoop(RocketLibSystemIdleFunction idle_function)
{
	while (running)
	{
		if( !glfwGetWindowParam( GLFW_OPENED ) )
		{
			running = false;
		}
		else
		{

			idle_function();

			if( !glfwGetWindowParam( GLFW_ACTIVE ) )
			{
				glfwSleep( 0.1 );
			}
		}
	}
}

void RocketLibSystem::RequestExit()
{
	running = false;
}

void RocketLibSystem::DisplayError(const char* fmt, ...)
{
	//not implemented
}

float RocketLibSystem::GetElapsedTime() 
{
	return (float)glfwGetTime();
}

void GLFWCALL WindowResize( int width, int height )
{
	glViewport(0, 0, width, height);
	glGetIntegerv( GL_VIEWPORT, g_WindowSize);
}


void RocketLibSystem::PreRenderRocketLib()
{
	// Set up the GL state for rocket
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable( GL_ALPHA_TEST );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();
	glOrtho(0, g_WindowSize[2], g_WindowSize[3], 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

