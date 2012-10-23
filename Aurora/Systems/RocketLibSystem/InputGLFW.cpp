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
#include <GL/glfw.h>

#include "InputGLFW.h"
#include <Rocket/Core/Context.h>
#include <Rocket/Core/Input.h>



static int GetKeyModifierState();
static void InitialiseKeymap();

void GLFWCALL KeyCallback( int key, int action );
void GLFWCALL CharCallback( int character, int action );
void GLFWCALL MouseButtonCallback( int button, int action );
void GLFWCALL MousePosCallback( int x, int y );
void GLFWCALL MouseWheelCallback( int pos );

static const int KEYMAP_SIZE = 512;
static Rocket::Core::Input::KeyIdentifier key_identifier_map[KEYMAP_SIZE];

bool InputGLFW::Initialise()
{
	InitialiseKeymap();

	//init callbacks for glfw
	glfwSetCharCallback( CharCallback );
	glfwSetKeyCallback( KeyCallback );
	glfwSetMouseButtonCallback( MouseButtonCallback );
	glfwSetMousePosCallback( MousePosCallback );
	glfwSetMouseWheelCallback( MouseWheelCallback );

	return true;
}


void GLFWCALL KeyCallback( int key, int action )
{
	if( 0 == Input::GetContext() ) { return; }

	if( GLFW_PRESS == action )
	{
		Input::GetContext()->ProcessKeyDown(key_identifier_map[key], GetKeyModifierState());
		if( key == GLFW_KEY_TAB )
		{
			Input::GetContext()->ProcessTextInput( '\t' );
		}
		if( key == GLFW_KEY_ENTER )
		{
			Input::GetContext()->ProcessTextInput( '\n' );
		}
	}
	else
	{
		Input::GetContext()->ProcessKeyUp(key_identifier_map[key], GetKeyModifierState());
	}
	
}

void GLFWCALL CharCallback( int character, int action )
{
	if( 0 == Input::GetContext() ) { return; }

	if( GLFW_PRESS == action )
	{
		Input::GetContext()->ProcessTextInput( character );
	}
}

void GLFWCALL MouseButtonCallback( int button, int action )
{
	if( 0 == Input::GetContext() ) { return; }

	if( GLFW_RELEASE ==  action )
	{
		Input::GetContext()->ProcessMouseButtonUp( button, GetKeyModifierState());
	}
	else
	{
		Input::GetContext()->ProcessMouseButtonDown( button, GetKeyModifierState());
	}


}


void GLFWCALL MousePosCallback( int x, int y )
{
	if( 0 == Input::GetContext() ) { return; }

	Input::GetContext()->ProcessMouseMove(x, y, GetKeyModifierState());
}


void GLFWCALL MouseWheelCallback( int pos )
{
	if( 0 == Input::GetContext() ) { return; }
	static int pos0 = 0;
	Input::GetContext()->ProcessMouseWheel(pos0-pos, GetKeyModifierState());
	pos0 = pos;
}


static int GetKeyModifierState()
{
	int key_modifier_state = 0;

	// Query the state of all modifier keys
	if( glfwGetKey( GLFW_KEY_CAPS_LOCK ))
	{
		key_modifier_state |= Rocket::Core::Input::KM_CAPSLOCK;
	}

	if( glfwGetKey( GLFW_KEY_LSHIFT ) || glfwGetKey( GLFW_KEY_RSHIFT ) )
	{
		key_modifier_state |= Rocket::Core::Input::KM_SHIFT;
	}

	if( glfwGetKey( GLFW_KEY_KP_NUM_LOCK ))
	{
		key_modifier_state |= Rocket::Core::Input::KM_NUMLOCK;
	}

	if( glfwGetKey( GLFW_KEY_LCTRL ) || glfwGetKey( GLFW_KEY_RCTRL ) )
	{
		key_modifier_state |= Rocket::Core::Input::KM_CTRL;
	}

	if( glfwGetKey( GLFW_KEY_LALT ) || glfwGetKey( GLFW_KEY_RALT ) )
	{
		key_modifier_state |= Rocket::Core::Input::KM_ALT;
	}

	return key_modifier_state;
}

static void InitialiseKeymap()
{
	// Initialise the key map with default values.
	memset(key_identifier_map, 0, sizeof(key_identifier_map));
	
	// Assign individual values.
	key_identifier_map['A'] = Rocket::Core::Input::KI_A;
	key_identifier_map['B'] = Rocket::Core::Input::KI_B;
	key_identifier_map['C'] = Rocket::Core::Input::KI_C;
	key_identifier_map['D'] = Rocket::Core::Input::KI_D;
	key_identifier_map['E'] = Rocket::Core::Input::KI_E;
	key_identifier_map['F'] = Rocket::Core::Input::KI_F;
	key_identifier_map['G'] = Rocket::Core::Input::KI_G;
	key_identifier_map['H'] = Rocket::Core::Input::KI_H;
	key_identifier_map['I'] = Rocket::Core::Input::KI_I;
	key_identifier_map['J'] = Rocket::Core::Input::KI_J;
	key_identifier_map['K'] = Rocket::Core::Input::KI_K;
	key_identifier_map['L'] = Rocket::Core::Input::KI_L;
	key_identifier_map['M'] = Rocket::Core::Input::KI_M;
	key_identifier_map['N'] = Rocket::Core::Input::KI_N;
	key_identifier_map['O'] = Rocket::Core::Input::KI_O;
	key_identifier_map['P'] = Rocket::Core::Input::KI_P;
	key_identifier_map['Q'] = Rocket::Core::Input::KI_Q;
	key_identifier_map['R'] = Rocket::Core::Input::KI_R;
	key_identifier_map['S'] = Rocket::Core::Input::KI_S;
	key_identifier_map['T'] = Rocket::Core::Input::KI_T;
	key_identifier_map['U'] = Rocket::Core::Input::KI_U;	
	key_identifier_map['V'] = Rocket::Core::Input::KI_V;
	key_identifier_map['W'] = Rocket::Core::Input::KI_W;
	key_identifier_map['X'] = Rocket::Core::Input::KI_X;
	key_identifier_map['Y'] = Rocket::Core::Input::KI_Y;
	key_identifier_map['Z'] = Rocket::Core::Input::KI_Z;

	key_identifier_map['0'] = Rocket::Core::Input::KI_0;
	key_identifier_map['1'] = Rocket::Core::Input::KI_1;
	key_identifier_map['2'] = Rocket::Core::Input::KI_2;
	key_identifier_map['3'] = Rocket::Core::Input::KI_3;
	key_identifier_map['4'] = Rocket::Core::Input::KI_4;
	key_identifier_map['5'] = Rocket::Core::Input::KI_5;
	key_identifier_map['6'] = Rocket::Core::Input::KI_6;
	key_identifier_map['7'] = Rocket::Core::Input::KI_7;
	key_identifier_map['8'] = Rocket::Core::Input::KI_8;
	key_identifier_map['9'] = Rocket::Core::Input::KI_9;

	key_identifier_map[GLFW_KEY_BACKSPACE] = Rocket::Core::Input::KI_BACK;
	key_identifier_map[GLFW_KEY_TAB] = Rocket::Core::Input::KI_TAB;

	key_identifier_map[GLFW_KEY_ENTER] = Rocket::Core::Input::KI_RETURN;

	key_identifier_map[GLFW_KEY_PAUSE] = Rocket::Core::Input::KI_PAUSE;
	key_identifier_map[GLFW_KEY_CAPS_LOCK] = Rocket::Core::Input::KI_CAPITAL;

	key_identifier_map[GLFW_KEY_ESC] = Rocket::Core::Input::KI_ESCAPE;

	key_identifier_map[GLFW_KEY_SPACE] = Rocket::Core::Input::KI_SPACE;
	key_identifier_map[GLFW_KEY_PAGEUP] = Rocket::Core::Input::KI_PRIOR;
	key_identifier_map[GLFW_KEY_PAGEDOWN] = Rocket::Core::Input::KI_NEXT;
	key_identifier_map[GLFW_KEY_END] = Rocket::Core::Input::KI_END;
	key_identifier_map[GLFW_KEY_HOME] = Rocket::Core::Input::KI_HOME;
	key_identifier_map[GLFW_KEY_LEFT] = Rocket::Core::Input::KI_LEFT;
	key_identifier_map[GLFW_KEY_UP] = Rocket::Core::Input::KI_UP;
	key_identifier_map[GLFW_KEY_RIGHT] = Rocket::Core::Input::KI_RIGHT;
	key_identifier_map[GLFW_KEY_DOWN] = Rocket::Core::Input::KI_DOWN;
	key_identifier_map[GLFW_KEY_INSERT] = Rocket::Core::Input::KI_INSERT;
	key_identifier_map[GLFW_KEY_DEL] = Rocket::Core::Input::KI_DELETE;

	key_identifier_map[GLFW_KEY_LSUPER] = Rocket::Core::Input::KI_LWIN;
	key_identifier_map[GLFW_KEY_RSUPER] = Rocket::Core::Input::KI_RWIN;

	key_identifier_map[GLFW_KEY_KP_0] = Rocket::Core::Input::KI_NUMPAD0;
	key_identifier_map[GLFW_KEY_KP_1] = Rocket::Core::Input::KI_NUMPAD1;
	key_identifier_map[GLFW_KEY_KP_2] = Rocket::Core::Input::KI_NUMPAD2;
	key_identifier_map[GLFW_KEY_KP_3] = Rocket::Core::Input::KI_NUMPAD3;
	key_identifier_map[GLFW_KEY_KP_4] = Rocket::Core::Input::KI_NUMPAD4;
	key_identifier_map[GLFW_KEY_KP_5] = Rocket::Core::Input::KI_NUMPAD5;
	key_identifier_map[GLFW_KEY_KP_6] = Rocket::Core::Input::KI_NUMPAD6;
	key_identifier_map[GLFW_KEY_KP_7] = Rocket::Core::Input::KI_NUMPAD7;
	key_identifier_map[GLFW_KEY_KP_8] = Rocket::Core::Input::KI_NUMPAD8;
	key_identifier_map[GLFW_KEY_KP_9] = Rocket::Core::Input::KI_NUMPAD9;
	key_identifier_map[GLFW_KEY_KP_MULTIPLY] = Rocket::Core::Input::KI_MULTIPLY;
	key_identifier_map[GLFW_KEY_KP_ADD] = Rocket::Core::Input::KI_ADD;
	key_identifier_map[GLFW_KEY_KP_DECIMAL] = Rocket::Core::Input::KI_SEPARATOR; //i.e. either '.' or ','
	key_identifier_map[GLFW_KEY_KP_SUBTRACT] = Rocket::Core::Input::KI_SUBTRACT;
	key_identifier_map[GLFW_KEY_KP_DECIMAL] = Rocket::Core::Input::KI_DECIMAL;
	key_identifier_map[GLFW_KEY_KP_DIVIDE] = Rocket::Core::Input::KI_DIVIDE;
	key_identifier_map[GLFW_KEY_F1] = Rocket::Core::Input::KI_F1;
	key_identifier_map[GLFW_KEY_F2] = Rocket::Core::Input::KI_F2;
	key_identifier_map[GLFW_KEY_F3] = Rocket::Core::Input::KI_F3;
	key_identifier_map[GLFW_KEY_F4] = Rocket::Core::Input::KI_F4;
	key_identifier_map[GLFW_KEY_F5] = Rocket::Core::Input::KI_F5;
	key_identifier_map[GLFW_KEY_F6] = Rocket::Core::Input::KI_F6;
	key_identifier_map[GLFW_KEY_F7] = Rocket::Core::Input::KI_F7;
	key_identifier_map[GLFW_KEY_F8] = Rocket::Core::Input::KI_F8;
	key_identifier_map[GLFW_KEY_F9] = Rocket::Core::Input::KI_F9;
	key_identifier_map[GLFW_KEY_F10] = Rocket::Core::Input::KI_F10;
	key_identifier_map[GLFW_KEY_F11] = Rocket::Core::Input::KI_F11;
	key_identifier_map[GLFW_KEY_F12] = Rocket::Core::Input::KI_F12;
	key_identifier_map[GLFW_KEY_F13] = Rocket::Core::Input::KI_F13;
	key_identifier_map[GLFW_KEY_F14] = Rocket::Core::Input::KI_F14;
	key_identifier_map[GLFW_KEY_F15] = Rocket::Core::Input::KI_F15;
	key_identifier_map[GLFW_KEY_F16] = Rocket::Core::Input::KI_F16;
	key_identifier_map[GLFW_KEY_F17] = Rocket::Core::Input::KI_F17;
	key_identifier_map[GLFW_KEY_F18] = Rocket::Core::Input::KI_F18;
	key_identifier_map[GLFW_KEY_F19] = Rocket::Core::Input::KI_F19;
	key_identifier_map[GLFW_KEY_F20] = Rocket::Core::Input::KI_F20;
	key_identifier_map[GLFW_KEY_F21] = Rocket::Core::Input::KI_F21;
	key_identifier_map[GLFW_KEY_F22] = Rocket::Core::Input::KI_F22;
	key_identifier_map[GLFW_KEY_F23] = Rocket::Core::Input::KI_F23;
	key_identifier_map[GLFW_KEY_F24] = Rocket::Core::Input::KI_F24;

	key_identifier_map[GLFW_KEY_KP_NUM_LOCK] = Rocket::Core::Input::KI_NUMLOCK;
	key_identifier_map[GLFW_KEY_SCROLL_LOCK] = Rocket::Core::Input::KI_SCROLL;

	key_identifier_map[GLFW_KEY_LSHIFT] = Rocket::Core::Input::KI_LSHIFT;
	key_identifier_map[GLFW_KEY_RSHIFT] = Rocket::Core::Input::KI_RSHIFT;
	key_identifier_map[GLFW_KEY_LCTRL] = Rocket::Core::Input::KI_LCONTROL;
	key_identifier_map[GLFW_KEY_RCTRL] = Rocket::Core::Input::KI_RCONTROL;
	key_identifier_map[GLFW_KEY_LALT] = Rocket::Core::Input::KI_LMENU;
	key_identifier_map[GLFW_KEY_RALT] = Rocket::Core::Input::KI_RMENU;

}
