/***************************************************************************
 * joystick.cpp  -  Joystick handling class
 *
 * Copyright (C) 2003 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/globals.h"
#include "../input/keyboard.h"
#include "../input/joystick.h"
#include "../user/preferences.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../gui/hud.h"
#include "../gui/menu.h"
#include "../level/level.h"
#include "../overworld/overworld.h"

namespace SMC
{

/* *** *** *** *** *** *** cJoystick *** *** *** *** *** *** *** *** *** *** *** */

cJoystick :: cJoystick( void )
{
	joystick = NULL;

	stick_open = 0;

	cur_stick = 0;
	num_buttons = 0;
	num_axes = 0;
	num_balls = 0;

	debug = 0;

	Reset_keys();

	Init();
}

cJoystick :: ~cJoystick( void )
{
	Close();
}

int cJoystick :: Init( void )
{
	// if not enabled
	if( !pPreferences->m_joy_enabled )
	{
		return 0;
	}

	int joy_count = SDL_NumJoysticks();

	// no joystick available
	if( joy_count <= 0 )
	{
		printf( "No joysticks available\n" );
		pPreferences->m_joy_enabled = 0;
		return 0;
	}

	if( debug )
	{
		printf( "Joysticks found : %d\n\n", joy_count );
	}

	unsigned int default_joy = 0;

	// if default joy name is given
	if( !pPreferences->m_joy_name.empty() )
	{
		vector<std::string> joy_names = Get_Names();

		for( unsigned int i = 0; i < joy_names.size(); i++ )
		{
			// found default joy
			if( joy_names[i].compare( pPreferences->m_joy_name ) == 0 )
			{
				default_joy = i;
				break;
			}
		}
	}

	// setup
	SDL_JoystickEventState( SDL_ENABLE );
	Stick_Open( default_joy );

	if( debug )
	{
		printf( "Joypad System Initialized\n" );
	}

	return 1;
}

void cJoystick :: Close( void )
{
	Stick_Close();
}

bool cJoystick :: Stick_Open( unsigned int index )
{
	// if a joystick is already opened close it first
	if( stick_open )
	{
		Stick_Close();
	}

	joystick = SDL_JoystickOpen( index );

	if( !joystick )
	{
		printf( "Couldn't open joystick %d\n", index );
		stick_open = 0;
		return 0;
	}

	cur_stick = index;

	num_buttons = SDL_JoystickNumButtons( joystick );
	num_axes = SDL_JoystickNumAxes( joystick );
	num_balls = SDL_JoystickNumBalls( joystick );

	// setup available buttons
	buttons.assign( num_buttons, 0 );

	if( debug )
	{
		printf( "Opened Joystick %d\n", cur_stick );
		printf( "Name: %s\n", Get_Name().c_str() );
		printf( "Number of Buttons: %d\n", num_buttons );
		printf( "Number of Axes: %d\n", num_axes );
		printf( "Number of Balls: %d\n\n", num_balls );
	}

	stick_open = 1;
	return 1;
}

void cJoystick :: Stick_Close( void )
{
	// not available
	if( !joystick )
	{
		return;
	}

	SDL_JoystickClose( joystick );
	joystick = NULL;

	Reset_keys();

	num_buttons = 0;
	num_axes = 0;
	num_balls = 0;

	buttons.clear();
	stick_open = 0;

	if( debug )
	{
		printf( "Joystick %d closed\n", cur_stick );
	}

	cur_stick = 0;
}

void cJoystick :: Reset_keys( void )
{
	// clear buttons
	std::fill( buttons.begin(), buttons.end(), static_cast<bool>(0) );
	
	left = 0;
	right = 0;
	up = 0;
	down = 0;
}

void cJoystick :: Handle_Hat( SDL_Event *ev )
{
	// up
	if( ev->jhat.value == SDL_HAT_UP )
	{
		if( !up )
		{
			pKeyboard->Key_Down( pPreferences->m_key_up );
			up = 1;
		}

		if( down )
		{
			pKeyboard->Key_Up( pPreferences->m_key_down );
			down = 0;
		}
	}
	else
	{
		if( up )
		{
			pKeyboard->Key_Up( pPreferences->m_key_up );
			up = 0;
		}
	}

	// down
	if( ev->jhat.value == SDL_HAT_DOWN )
	{
		if( !down )
		{
			pKeyboard->Key_Down( pPreferences->m_key_down );
			down = 1;
		}

		if( up )
		{
			pKeyboard->Key_Up( pPreferences->m_key_up );
			up = 0;
		}
	}
	else
	{
		if( down )
		{
			pKeyboard->Key_Up( pPreferences->m_key_down );
			down = 0;
		}
	}

	// left
	if( ev->jhat.value == SDL_HAT_LEFT )
	{
		if( !left )
		{
			pKeyboard->Key_Down( pPreferences->m_key_left );
			left = 1;
		}

		if( right )
		{
			pKeyboard->Key_Up( pPreferences->m_key_right );
			right = 0;
		}
	}
	else
	{
		if( left )
		{
			pKeyboard->Key_Up( pPreferences->m_key_left );
			left = 0;
		}
	}

	// right
	if( ev->jhat.value == SDL_HAT_RIGHT)
	{
		if( !right )
		{
			pKeyboard->Key_Down( pPreferences->m_key_right );
			right = 1;
		}

		if( left )
		{
			pKeyboard->Key_Up( pPreferences->m_key_left );
			left = 0;
		}
	}
	else
	{
		if( right )
		{
			pKeyboard->Key_Up( pPreferences->m_key_right );
			right = 0;
		}
	}
}

void cJoystick :: Handle_Motion( SDL_Event *ev )
{
	// Vertical Axis
	if( ev->jaxis.axis == pPreferences->m_joy_axis_ver )
	{
		// Up
		if( ev->jaxis.value < -pPreferences->m_joy_axis_threshold )
		{
			if( debug )
			{
				printf( "Joystick %d : Up Button pressed\n", cur_stick );
			}

			if( !up )
			{
				pKeyboard->Key_Down( pPreferences->m_key_up );
				up = 1;
			}

			if( down )
			{
				pKeyboard->Key_Up( pPreferences->m_key_down );
				down = 0;
			}
		}
		// Down
		else if( ev->jaxis.value > pPreferences->m_joy_axis_threshold )
		{
			if( debug )
			{
				printf( "Joystick %d : Down Button pressed\n", cur_stick );
			}

			if( !down )
			{
				pKeyboard->Key_Down( pPreferences->m_key_down );
				down = 1;
			}

			if( up )
			{
				pKeyboard->Key_Up( pPreferences->m_key_up );
				up = 0;
			}
		}
		// No Down/Left
		else
		{
			if( down )
			{
				pKeyboard->Key_Up( pPreferences->m_key_down );
				down = 0;
			}

			if( up )
			{
				pKeyboard->Key_Up( pPreferences->m_key_up );
				up = 0;
			}
		}
	}
	// Horizontal Axis
	else if( ev->jaxis.axis == pPreferences->m_joy_axis_hor )
	{
		// Left
		if( ev->jaxis.value < -pPreferences->m_joy_axis_threshold )
		{
			if( debug )
			{
				printf( "Joystick %d : Left Button pressed\n", cur_stick );
			}

			if( !left )
			{
				pKeyboard->Key_Down( pPreferences->m_key_left );
				left = 1;
			}

			if( right )
			{
				pKeyboard->Key_Up( pPreferences->m_key_right );
				right = 0;
			}
		}
		// Right
		else if( ev->jaxis.value > pPreferences->m_joy_axis_threshold )
		{
			if( debug )
			{
				printf( "Joystick %d : Right Button pressed\n", cur_stick );
			}

			if( !right )
			{
				pKeyboard->Key_Down( pPreferences->m_key_right );
				right = 1;
			}

			if( left )
			{
				pKeyboard->Key_Up( pPreferences->m_key_left );
				left = 0;
			}
		}
		// No Left/Right
		else
		{
			if( left )
			{
				pKeyboard->Key_Up( pPreferences->m_key_left );
				left = 0;
			}

			if( right )
			{
				pKeyboard->Key_Up( pPreferences->m_key_right );
				right = 0;
			}
		}
	}
}

bool cJoystick :: Handle_Button_Down_Event( SDL_Event *ev )
{
	// not enabled or opened
	if( !pPreferences->m_joy_enabled || !stick_open )
	{
		return 0;
	}

	Set_Button( ev->jbutton.button, 1 );

	// handle button in the current mode
	if( Game_Mode == MODE_LEVEL )
	{
		// processed by the level
		if( pActive_Level->Joy_Button_Down( ev->jbutton.button ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// processed by the overworld
		if( pActive_Overworld->Joy_Button_Down( ev->jbutton.button ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_MENU )
	{
		// processed by the menu
		if( pMenuCore->Joy_Button_Down( ev->jbutton.button ) )
		{
			return 1;
		}
	}

	if( ev->jbutton.button < buttons.size() )
	{
		// Jump
		if( ev->jbutton.button == pPreferences->m_joy_button_jump )
		{
			//
		}
		// Shoot
		else if( ev->jbutton.button == pPreferences->m_joy_button_shoot )
		{
			pKeyboard->Key_Down( pPreferences->m_key_shoot );
			return 1;
		}
		// Request Itembox Item
		else if( ev->jbutton.button == pPreferences->m_joy_button_item )
		{
			// not handled
			return 1;
		}
		// Interaction
		else if( ev->jbutton.button == pPreferences->m_joy_button_action )
		{
			pKeyboard->Key_Down( pPreferences->m_key_action );
			return 1;
		}
		// Exit
		else if( ev->jbutton.button == pPreferences->m_joy_button_exit )
		{
			pKeyboard->Key_Down( SDLK_ESCAPE );
			return 1;
		}
		// Pause
		else if( ev->jbutton.button == 9 )
		{
			pKeyboard->Key_Down( SDLK_PAUSE );
			return 1;
		}
	}

	return 0;
}

bool cJoystick :: Handle_Button_Up_Event( SDL_Event *ev )
{
	// not enabled or opened
	if( !pPreferences->m_joy_enabled || !stick_open )
	{
		return 0;
	}

	Set_Button( ev->jbutton.button, 0 );

	// handle button in the current mode
	if( Game_Mode == MODE_LEVEL )
	{
		// processed by the level
		if( pActive_Level->Joy_Button_Up( ev->jbutton.button ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// processed by the overworld
		if( pActive_Overworld->Joy_Button_Up( ev->jbutton.button ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_MENU )
	{
		// processed by the menu
		if( pMenuCore->Joy_Button_Up( ev->jbutton.button ) )
		{
			return 1;
		}
	}

	if( ev->jbutton.button < buttons.size() )
	{
		if( ev->jbutton.button == pPreferences->m_joy_button_jump )
		{
			pKeyboard->Key_Up( pPreferences->m_key_jump );
			return 1;
		}
		else if( ev->jbutton.button == pPreferences->m_joy_button_shoot )
		{
			pKeyboard->Key_Up( pPreferences->m_key_shoot );
			return 1;
		}
		else if( ev->jbutton.button == pPreferences->m_joy_button_item )
		{
			// not handled
		}
		else if( ev->jbutton.button == pPreferences->m_joy_button_action )
		{
			pKeyboard->Key_Up( pPreferences->m_key_action );
			return 1;
		}
		else if( ev->jbutton.button == pPreferences->m_joy_button_exit )
		{
			pKeyboard->Key_Up( SDLK_ESCAPE );
			return 1;
		}
	}

	return 0;
}

std::string cJoystick :: Get_Name( void ) const
{
	return SDL_JoystickName( cur_stick );
}

vector<std::string> cJoystick :: Get_Names( void ) const
{
	vector<std::string> names;
	// get joy count
	int joy_count = SDL_NumJoysticks();

	// joystick names
	for( int i = 0; i < joy_count; i++ )
	{
		names.push_back( SDL_JoystickName( i ) );
	}

	return names;
}

void cJoystick :: Set_Button( Uint8 num, bool pressed )
{
	// not available
	if( num >= buttons.size() )
	{
		return;
	}

	if( debug )
	{
		if( pressed )
		{
			printf( "Joystick %d : Joy Button %d pressed\n", cur_stick, num );
		}
		else
		{
			printf( "Joystick %d : Joy Button %d released\n", cur_stick, num );
		}
	}

	buttons[num] = pressed;
}

Uint8 *cJoystick :: Get_Shortcut( input_identifier shortcut_id ) const
{
	if( shortcut_id == INP_JUMP )
	{
		return &pPreferences->m_joy_button_jump;
	}
	else if( shortcut_id == INP_ITEM )
	{
		return &pPreferences->m_joy_button_item;
	}
	else if( shortcut_id == INP_SHOOT )
	{
		return &pPreferences->m_joy_button_shoot;
	}
	else if( shortcut_id == INP_ACTION )
	{
		return &pPreferences->m_joy_button_action;
	}
	else if( shortcut_id == INP_EXIT )
	{
		return &pPreferences->m_joy_button_exit;
	}

	return NULL;
}

void cJoystick :: Assign_Shortcut( input_identifier shortcut_id, Uint8 new_button )
{
	Uint8 *button = Get_Shortcut( shortcut_id );
	*button = new_button;
}

bool cJoystick :: Left( void ) const
{
	if( pPreferences->m_joy_enabled && input_event.type == SDL_JOYAXISMOTION && input_event.jaxis.value < -pPreferences->m_joy_axis_threshold && 
		input_event.jaxis.axis == pPreferences->m_joy_axis_hor )
	{
		return 1;
	}

	return 0;
}

bool cJoystick :: Right( void ) const
{
	if( pPreferences->m_joy_enabled && input_event.type == SDL_JOYAXISMOTION && input_event.jaxis.value > pPreferences->m_joy_axis_threshold && 
		input_event.jaxis.axis == pPreferences->m_joy_axis_hor )
	{
		return 1;
	}
	
	return 0;
}

bool cJoystick :: Up( void ) const
{
	if( pPreferences->m_joy_enabled && input_event.type == SDL_JOYAXISMOTION && input_event.jaxis.value < -pPreferences->m_joy_axis_threshold && 
		input_event.jaxis.axis == pPreferences->m_joy_axis_ver )
	{
		return 1;
	}
	
	return 0;
}

bool cJoystick :: Down( void ) const
{
	if( pPreferences->m_joy_enabled && input_event.type == SDL_JOYAXISMOTION && input_event.jaxis.value > pPreferences->m_joy_axis_threshold && 
		input_event.jaxis.axis == pPreferences->m_joy_axis_ver )
	{
		return 1;
	}
	
	return 0;
}

bool cJoystick :: Button( Uint8 num )
{
	// if available and pressed
	if( num < buttons.size() && buttons[num] )
	{
		return 1;
	}

	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cJoystick *pJoystick = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
