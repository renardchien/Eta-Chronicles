/***************************************************************************
 * game_core.cpp  -  globally used variables and functions
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

#include "../core/game_core.h"
#include "../core/main.h"
#include "../video/font.h"
#include "../audio/audio.h"
#include "../core/framerate.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "../input/joystick.h"
#include "../level/level_editor.h"
#include "../overworld/world_editor.h"
#include "../player/player.h"
#include "../video/renderer.h"
#include "../level/level.h"
#include "../core/sprite_manager.h"
#include "../overworld/overworld.h"
#include "../gui/menu.h"
#include "../core/i18n.h"
#include "../core/filesystem/filesystem.h"
// boost filesystem
#include "boost/filesystem/convenience.hpp"
namespace fs = boost::filesystem;

#ifdef __unix__
	// needed for the clipboard access
	#  include <SDL_syswm.h>
#elif __APPLE__
	// needed for the clipboard access
	#include <Carbon/Carbon.h>
#endif

namespace SMC
{

/* *** *** *** *** *** *** *** *** Variables *** *** *** *** *** *** *** *** *** */

bool game_exit = 0;
GameMode Game_Mode = MODE_NOTHING;
GameModeType Game_Mode_Type = MODE_TYPE_DEFAULT;
GameAction Game_Action = GA_NONE;
CEGUI::XMLAttributes Game_Action_Data;
void *Game_Action_ptr = NULL;

int game_res_w = 800;
int game_res_h = 600;

bool game_debug = 0, game_debug_performance = 0;

SDL_Event input_event;

float global_upscalex = 1, global_upscaley = 1;
float global_downscalex = 1, global_downscaley = 1;

bool editor_enabled = 0;
bool editor_level_enabled = 0;
bool editor_world_enabled = 0;

cSprite_Manager *pActive_Sprite_Manager = NULL;
cCamera *pActive_Camera = NULL;
cSprite *pActive_Player = NULL;

/* *** *** *** *** *** *** *** cDialogBox *** *** *** *** *** *** *** *** *** *** */

cDialogBox :: cDialogBox( void )
{
	finished = 0;
	window = NULL;
	mouse_hide = 0;
}

cDialogBox :: ~cDialogBox( void )
{

}

void cDialogBox :: Init( void )
{
	// load layout
	window = CEGUI::WindowManager::getSingleton().loadWindowLayout( layout_file );
	pGuiSystem->getGUISheet()->addChildWindow( window );

	// hide mouse on exit
	if( !pMouseCursor->m_active )
	{
		mouse_hide = 1;
		pMouseCursor->Set_Active( 1 );
	}
}

void cDialogBox :: Exit( void )
{
	// hide mouse
	if( mouse_hide )
	{
		pMouseCursor->Set_Active( 0 );
	}

	pGuiSystem->getGUISheet()->removeChildWindow( window );
	CEGUI::WindowManager::getSingleton().destroyWindow( window );
}

void cDialogBox :: Draw( void )
{
	Draw_Game();
	pVideo->Draw_Gradient( NULL, 0.905f, &whitealpha128, &black, DIR_VERTICAL );

	pVideo->Render();

	pMouseCursor->Update_Position();
}

void cDialogBox :: Update( void )
{
	pFramerate->Update();
	Gui_Handle_Time();
}

/* *** *** *** *** *** *** *** cDialogBox_Text *** *** *** *** *** *** *** *** *** *** */

cDialogBox_Text :: cDialogBox_Text( void )
: cDialogBox()
{
	layout_file = "box_text.layout";
}

cDialogBox_Text :: ~cDialogBox_Text( void )
{

}

void cDialogBox_Text :: Init( std::string title_text )
{
	cDialogBox::Init();

	// get window
	CEGUI::FrameWindow *box_window = static_cast<CEGUI::FrameWindow *>(CEGUI::WindowManager::getSingleton().getWindow( "box_text_window" ));
	box_window->setText( reinterpret_cast<const CEGUI::utf8*>(title_text.c_str()) );
	box_window->setSizingEnabled( 0 );
	box_window->getCloseButton()->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cDialogBox_Text::Button_window_quit_clicked, this ) );

	// get editbox
	box_editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "box_text_editbox" ));
}

std::string cDialogBox_Text :: Enter( std::string default_text, std::string title_text, bool auto_no_text /* = 1 */ )
{
	Init( title_text );

	box_editbox->setText( reinterpret_cast<const CEGUI::utf8*>(default_text.c_str()) );
	box_editbox->setTooltipText( reinterpret_cast<const CEGUI::utf8*>(title_text.c_str()) );
	box_editbox->activate();
	box_editbox->setCaratIndex( default_text.length() );

	finished = 0;

	while( !finished )
	{
		while( SDL_PollEvent( &input_event ) )
		{
			if( input_event.type == SDL_KEYDOWN )
			{
				if( auto_no_text && default_text.compare( box_editbox->getText().c_str() ) == 0 )
				{
					box_editbox->setText( "" );
					// only the first time
					auto_no_text = 0;
				}

				if( input_event.key.keysym.sym == SDLK_ESCAPE )
				{
					box_editbox->setText( "" );
					finished = 1;
				}
				else if( input_event.key.keysym.sym == SDLK_RETURN || input_event.key.keysym.sym == SDLK_KP_ENTER )
				{
					finished = 1;
				}
				else
				{
					pKeyboard->CEGUI_Handle_Key_Down( input_event.key.keysym.sym );
				}
			}
			else if( input_event.type == SDL_KEYUP )
			{
				pKeyboard->CEGUI_Handle_Key_Up( input_event.key.keysym.sym );
			}
			else
			{
				pMouseCursor->Handle_Event( &input_event );
			}
		}

		Update();
		Draw();
	}

	std::string return_string = box_editbox->getText().c_str();

	Exit();
	return return_string;
}

bool cDialogBox_Text :: Button_window_quit_clicked( const CEGUI::EventArgs &event )
{
	box_editbox->setText( "" );
	finished = 1;
	return 1;
}

/* *** *** *** *** *** *** *** cDialogBox_Question *** *** *** *** *** *** *** *** *** *** */

cDialogBox_Question :: cDialogBox_Question( void )
: cDialogBox()
{
	layout_file = "box_question.layout";
	box_window = NULL;
	return_value = -1;
}

cDialogBox_Question :: ~cDialogBox_Question( void )
{

}

void cDialogBox_Question :: Init( bool with_cancel )
{
	cDialogBox::Init();

	// get window
	box_window = static_cast<CEGUI::FrameWindow *>(CEGUI::WindowManager::getSingleton().getWindow( "box_question_window" ));
	box_window->moveToFront();

	// subscribe close button
	if( with_cancel )
	{
		box_window->getCloseButton()->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cDialogBox_Question::Button_cancel_clicked, this ) );
	}
	else
	{
		box_window->getCloseButton()->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cDialogBox_Question::Button_no_clicked, this ) );
	}
}

int cDialogBox_Question :: Enter( std::string text, bool with_cancel /* = 0 */ )
{
	Init( with_cancel );

	// get text
	CEGUI::Editbox *box_text = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "box_question_text" ));
	box_text->setText( reinterpret_cast<const CEGUI::utf8*>(text.c_str()) );


	// align text
	CEGUI::Font *font = CEGUI::FontManager::getSingleton().getFont( "bluebold_medium" );
	// fixme : Can't handle multiple lines of text
	float text_width = font->getTextExtent( text ) * global_downscalex;

	if( text_width > 250 )
	{
		box_window->setWidth( CEGUI::UDim( 0, ( text_width + 15 ) * global_upscalex ) );
		box_window->setXPosition( CEGUI::UDim( 0, ( game_res_w * 0.5f - text_width * 0.5f ) * global_upscalex ) );
	}

	// Button Yes
	CEGUI::PushButton *button_yes = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "box_question_button_yes" ));
	// Button No
	CEGUI::PushButton *button_no = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "box_question_button_no" ));
	// Button Cancel
	CEGUI::PushButton *button_cancel = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "box_question_button_cancel" ));

	// if without cancel
	if( !with_cancel )
	{
		button_cancel->hide();
	}

	// events
	button_yes->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cDialogBox_Question::Button_yes_clicked, this ) );
	button_no->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cDialogBox_Question::Button_no_clicked, this ) );
	button_cancel->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cDialogBox_Question::Button_cancel_clicked, this ) );

	finished = 0;

	while( !finished )
	{
		Draw();

		while( SDL_PollEvent( &input_event ) )
		{
			if( input_event.type == SDL_KEYDOWN )
			{
				if( input_event.key.keysym.sym == SDLK_ESCAPE )
				{
					if( with_cancel )
					{
						return_value = -1;
					}
					else
					{
						return_value = 0;
					}

					finished = 1;
				}
				else if( input_event.key.keysym.sym == SDLK_RETURN || input_event.key.keysym.sym == SDLK_KP_ENTER )
				{
					return_value = 1;
					finished = 1;
				}
				else
				{
					pKeyboard->CEGUI_Handle_Key_Down( input_event.key.keysym.sym );
				}
			}
			else if( input_event.type == SDL_KEYUP )
			{
				pKeyboard->CEGUI_Handle_Key_Up( input_event.key.keysym.sym );
			}
			else
			{
				pMouseCursor->Handle_Event( &input_event );
			}
		}

		Update();
	}

	Exit();

	return return_value;
}

bool cDialogBox_Question :: Button_yes_clicked( const CEGUI::EventArgs &event )
{
	return_value = 1;
	finished = 1;
	return 1;
}

bool cDialogBox_Question :: Button_no_clicked( const CEGUI::EventArgs &event )
{
	return_value = 0;
	finished = 1;
	return 1;
}

bool cDialogBox_Question :: Button_cancel_clicked( const CEGUI::EventArgs &event )
{
	return_value = -1;
	finished = 1;
	return 1;
}

/* *** *** *** *** *** *** *** Functions *** *** *** *** *** *** *** *** *** *** */

void Change_Game_Mode( const GameMode new_mode )
{
	// always set for level
	if( new_mode == MODE_LEVEL )
	{
		// set active level manager
		pActive_Sprite_Manager = pActive_Level->m_sprite_manager;
		// set active camera
		pActive_Camera = pLevel_Manager->camera;
		// set active player
		pActive_Player = pPlayer;
	}
	// always set for overworld
	else if( new_mode == MODE_OVERWORLD )
	{
		// set active level manager
		pActive_Sprite_Manager = pActive_Overworld->m_sprite_manager;
		// set active camera
		pActive_Camera = pOverworld_Manager->camera;
		// set active player
		pActive_Player = pOverworld_Player;
	}
	// always set for menu
	else if( new_mode == MODE_MENU )
	{
		// set active level manager
		pActive_Sprite_Manager = pMenuCore->handler->m_level->m_sprite_manager;
		// set active camera
		pActive_Camera = pMenuCore->camera;
		// set active player
		pActive_Player = pMenuCore->handler->m_player;
	}
	// always set for level settings
	else if( new_mode == MODE_LEVEL_SETTINGS )
	{
		// set active camera
		pActive_Camera = pLevel_Editor->pSettings->camera;
	}

	// already set
	if( Game_Mode == new_mode )
	{
		// world to world
		if( new_mode == MODE_OVERWORLD )
		{
			// fade out music
			pAudio->Fadeout_Music( 500 );
			// play music
			pAudio->Play_Music( pActive_Overworld->m_musicfile, -1, 0, 3000 );

			// center camera position
			pActive_Camera->Center();
		}
		// level to level
		else if( new_mode == MODE_LEVEL )
		{
			// fade out music
			pAudio->Fadeout_Music( 500 );

			// reset player
			pPlayer->Reset();
			// center camera position
			pActive_Camera->Center();
		}

		return;
	}

	// remember old mode
	GameMode old_mode = Game_Mode;
	// set new mode first
	Game_Mode = new_mode;

	// mode was level
	if( old_mode == MODE_LEVEL )
	{
		pAnimation_Manager->Delete_All();

		pJoystick->Reset_keys();

		// hide editor window if visible
		if( pLevel_Editor->enabled )
		{
			if( pLevel_Editor->editor_window->isVisible() )
			{
				pLevel_Editor->editor_window->hide();
			}
		}

		// if new mode should play different music
		if( new_mode != MODE_MENU && new_mode != MODE_LEVEL_SETTINGS )
		{
			// fade out music
			pAudio->Fadeout_Music( 1000 );
		}
	}
	// mode was overworld
	else if( old_mode == MODE_OVERWORLD )
	{
		pAnimation_Manager->Delete_All();

		// hide editor window if visible
		if( pWorld_Editor->enabled )
		{
			if( pWorld_Editor->editor_window->isVisible() )
			{
				pWorld_Editor->editor_window->hide();
			}
		}

		// if new mode is not menu
		if( new_mode != MODE_MENU )
		{
			// fade out music
			pAudio->Fadeout_Music( 1000 );

			// clear input
			Clear_Input_Events();
		}
	}
	// mode was menu
	else if( old_mode == MODE_MENU )
	{
		// hide mouse
		if( !editor_enabled )
		{
			pMouseCursor->Set_Active( 0 );
		}
	}
	// mode was settings
	else if( old_mode == MODE_LEVEL_SETTINGS )
	{
		//
	}


	// mode gets level
	if( new_mode == MODE_LEVEL )
	{
		if( pActive_Level->Is_Loaded() )
		{
			// play music
			if( pActive_Level->m_valid_music )
			{
				pAudio->Play_Music( pActive_Level->m_musicfile, -1, 0, 1000 );
			}
			else if( pAudio->m_music_enabled )
			{
				printf( "Warning : Music file not found %s\n", pActive_Level->m_musicfile.c_str() );
			}
		}

		// disable world editor
		pWorld_Editor->Disable();

		// set editor enabled state
		editor_enabled = pLevel_Editor->enabled;

		if( pLevel_Editor->enabled )
		{
			if( !pLevel_Editor->editor_window->isVisible() )
			{
				pLevel_Editor->editor_window->show();
				pMouseCursor->Set_Active( 1 );
			}
		}
		// enable editor
		else if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM_EDITOR )
		{
			Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM;
			pLevel_Editor->Enable();
		}

		// update camera
		pActive_Camera->Update_Position();
		pActive_Camera->Center();

		// Update Hud Text and position
		pHud_Manager->Update_Text();

		// reset speedfactor
		pFramerate->Reset();
	}
	// mode gets overworld
	else if( new_mode == MODE_OVERWORLD )
	{
		// play music
		pAudio->Play_Music( pActive_Overworld->m_musicfile, -1, 0, 3000 );

		// reset custom level mode type
		if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM )
		{
			Game_Mode_Type = MODE_TYPE_DEFAULT;
		}

		// disable level editor
		pLevel_Editor->Disable();

		// set editor enabled state
		editor_enabled = pWorld_Editor->enabled;

		if( pWorld_Editor->enabled )
		{
			if( !pWorld_Editor->editor_window->isVisible() )
			{
				pWorld_Editor->editor_window->show();
				pMouseCursor->Set_Active( 1 );
			}
		}

		// update camera
		pActive_Camera->Update_Position();

		// Update Hud Text and position
		pHud_Manager->Update_Text();

		// reset speedfactor
		pFramerate->Reset();
	}
	// mode gets menu
	else if( new_mode == MODE_MENU )
	{
		editor_enabled = 0;

		// update camera
		pActive_Camera->Update_Position();
		// show mouse
		pMouseCursor->Set_Active( 1 );
		// position HUD
		pHud_Manager->Update_Text();
	}
	// mode gets settings
	else if( new_mode == MODE_LEVEL_SETTINGS )
	{
		editor_enabled = 0;

		// update camera
		pActive_Camera->Update_Position();

		if( pMouseCursor->m_active_object )
		{
			pMouseCursor->Clear_Active_Object();
		}
	}
}

std::string Time_to_String( time_t t, const char *format )
{
	char str_time[60];
	strftime( str_time, 60, format, localtime( &t ) );

	return str_time;
}

void Clear_Input_Events( void )
{
	while( SDL_PollEvent( &input_event ) )
	{
		// todo : keep Windowmanager quit events ?
		// ignore all events
	}

	// Reset keys
	pKeyboard->Reset_Keys();
	pMouseCursor->Reset_Keys();
	pJoystick->Reset_keys();
}

ObjectDirection Get_Opposite_Direction( const ObjectDirection direction )
{
	switch( direction )
	{
		case DIR_UP:
		{
			return DIR_DOWN;
		}
		case DIR_DOWN:
		{
			return DIR_UP;
		}
		case DIR_LEFT:
		{
			return DIR_RIGHT;
		}
		case DIR_RIGHT:
		{
			return DIR_LEFT;
		}
		case DIR_HORIZONTAL:
		{
			return DIR_VERTICAL;
		}
		case DIR_VERTICAL:
		{
			return DIR_HORIZONTAL;
		}
		default:
		{
			break;
		}
	}

	return DIR_UNDEFINED;
}

std::string Get_Direction_Name( const ObjectDirection dir )
{
	switch( dir )
	{
		case DIR_UNDEFINED:
		{
			return N_("undefined");
		}
		case DIR_LEFT:
		{
			return N_("left");
		}
		case DIR_RIGHT:
		{
			return N_("right");
		}
		case DIR_UP:
		{
			return N_("up");
		}
		case DIR_DOWN:
		{
			return N_("down");
		}
		case DIR_TOP_LEFT:
		{
			return N_("top_left");
		}
		case DIR_TOP_RIGHT:
		{
			return N_("top_right");
		}
		case DIR_BOTTOM_LEFT:
		{
			return N_("bottom_left");
		}
		case DIR_BOTTOM_RIGHT:
		{
			return N_("bottom_right");
		}
		case DIR_LEFT_TOP:
		{
			return N_("left_top");
		}
		case DIR_LEFT_BOTTOM:
		{
			return N_("left_bottom");
		}
		case DIR_RIGHT_TOP:
		{
			return N_("right_top");
		}
		case DIR_RIGHT_BOTTOM:
		{
			return N_("right_bottom");
		}
		case DIR_HORIZONTAL:
		{
			return N_("horizontal");
		}
		case DIR_VERTICAL:
		{
			return N_("vertical");
		}
		case DIR_ALL:
		{
			return N_("all");
		}
		case DIR_FIRST:
		{
			return N_("first");
		}
		case DIR_LAST:
		{
			return N_("last");
		}
		default:
		{
			break;
		}
	}

	return "";
}

ObjectDirection Get_Direction_Id( const std::string &str_direction )
{
	if( str_direction.compare( "undefined" ) == 0 )
	{
		return DIR_UNDEFINED;
	}
	else if( str_direction.compare( "left" ) == 0 )
	{
		return DIR_LEFT;
	}
	else if( str_direction.compare( "right" ) == 0 )
	{
		return DIR_RIGHT;
	}
	else if( str_direction.compare( "up" ) == 0 )
	{
		return DIR_UP;
	}
	else if( str_direction.compare( "down" ) == 0 )
	{
		return DIR_DOWN;
	}
	else if( str_direction.compare( "top_left" ) == 0 )
	{
		return DIR_TOP_LEFT;
	}
	else if( str_direction.compare( "top_right" ) == 0 )
	{
		return DIR_TOP_RIGHT;
	}
	else if( str_direction.compare( "bottom_left" ) == 0 )
	{
		return DIR_BOTTOM_LEFT;
	}
	else if( str_direction.compare( "bottom_right" ) == 0 )
	{
		return DIR_BOTTOM_RIGHT;
	}
	else if( str_direction.compare( "left_top" ) == 0 )
	{
		return DIR_LEFT_TOP;
	}
	else if( str_direction.compare( "left_bottom" ) == 0 )
	{
		return DIR_LEFT_BOTTOM;
	}
	else if( str_direction.compare( "right_top" ) == 0 )
	{
		return DIR_RIGHT_TOP;
	}
	else if( str_direction.compare( "right_bottom" ) == 0 )
	{
		return DIR_RIGHT_BOTTOM;
	}
	else if( str_direction.compare( "horizontal" ) == 0 )
	{
		return DIR_HORIZONTAL;
	}
	else if( str_direction.compare( "vertical" ) == 0 )
	{
		return DIR_VERTICAL;
	}
	else if( str_direction.compare( "all" ) == 0 )
	{
		return DIR_ALL;
	}
	else if( str_direction.compare( "first" ) == 0 )
	{
		return DIR_FIRST;
	}
	else if( str_direction.compare( "last" ) == 0 )
	{
		return DIR_LAST;
	}

	return DIR_UNDEFINED;
}


SpriteType Get_Sprite_Type_Id( const std::string &str_type )
{
	if( str_type.compare( "massive" ) == 0 )
	{
		return TYPE_MASSIVE;
	}
	else if( str_type.compare( "passive" ) == 0 )
	{
		return TYPE_PASSIVE;
	}
	else if( str_type.compare( "front_passive" ) == 0 )
	{
		return TYPE_FRONT_PASSIVE;
	}
	else if( str_type.compare( "halfmassive" ) == 0 )
	{
		return TYPE_HALFMASSIVE;
	}
	else if( str_type.compare( "climbable" ) == 0 )
	{
		return TYPE_CLIMBABLE;
	}
	else
	{
		printf( "Warning : Unknown Sprite Type String %s\n", str_type.c_str() );
	}
	
	return TYPE_UNDEFINED;
}

Color Get_Sprite_Color( const cSprite *sprite )
{
	switch( sprite->m_sprite_array )
	{
		case ARRAY_ENEMY:
		{
			return red;
		}
		case ARRAY_ACTIVE:
		{
			return blue;
		}
		case ARRAY_MASSIVE:
		{
			if( sprite->m_type == TYPE_PLAYER )
			{
				return lila;
			}

			return orange;
		}
		case ARRAY_PASSIVE:
		{
			if( sprite->m_type == TYPE_FRONT_PASSIVE )
			{
				return greenyellow;
			}

			return green;
		}
		case ARRAY_HUD:
		{
			return blackalpha128;
		}
		default:
		{
			break;
		}
	}

	return lightgreyalpha64;
}

std::string Get_Massive_Type_Name( const MassiveType mtype )
{
	switch( mtype )
	{
		case MASS_PASSIVE:
		{
			return "passive";
		}
		case MASS_MASSIVE:
		{
			return "massive";
		}
		case MASS_HALFMASSIVE:
		{
			return "halfmassive";
		}
		case MASS_CLIMBABLE:
		{
			return "climbable";
		}
		default:
		{
			break;
		}
	}

	return "";
}

MassiveType Get_Massive_Type_Id( const std::string &str_massivetype )
{
	if( str_massivetype.compare( "passive" ) == 0 )
	{
		return MASS_PASSIVE;
	}
	else if( str_massivetype.compare( "massive" ) == 0 )
	{
		return MASS_MASSIVE;
	}
	else if( str_massivetype.compare( "halfmassive" ) == 0 )
	{
		return MASS_HALFMASSIVE;
	}
	else if( str_massivetype.compare( "climbable" ) == 0 )
	{
		return MASS_CLIMBABLE;
	}

	return MASS_PASSIVE;
}

Color Get_Massive_Type_Color( MassiveType mtype )
{
	switch( mtype )
	{
		case MASS_MASSIVE:
		{
			return lightred;
		}
		case MASS_HALFMASSIVE:
		{
			return orange;
		}
		case MASS_PASSIVE:
		{
			return lightgreen;
		}
		case MASS_CLIMBABLE:
		{
			return lila;
		}
		default:
		{
			break;
		}
	}

	return white;
}

std::string Get_Ground_Type_Name( const GroundType gtype )
{
	switch( gtype )
	{
		case GROUND_NORMAL:
		{
			return "normal";
		}
		case GROUND_EARTH:
		{
			return "earth";
		}
		case GROUND_ICE:
		{
			return "ice";
		}
		case GROUND_SAND:
		{
			return "sand";
		}
		case GROUND_STONE:
		{
			return "stone";
		}
		case GROUND_PLASTIC:
		{
			return "plastic";
		}
		default:
		{
			break;
		}
	}

	return "";
}

GroundType Get_Ground_Type_Id( const std::string &str_groundtype )
{
	if( str_groundtype.compare( "normal" ) == 0 )
	{
		return GROUND_NORMAL;
	}
	else if( str_groundtype.compare( "earth" ) == 0 )
	{
		return GROUND_EARTH;
	}
	else if( str_groundtype.compare( "ice" ) == 0 )
	{
		return GROUND_ICE;
	}
	else if( str_groundtype.compare( "sand" ) == 0 )
	{
		return GROUND_SAND;
	}
	else if( str_groundtype.compare( "stone" ) == 0 )
	{
		return GROUND_STONE;
	}
	else if( str_groundtype.compare( "plastic" ) == 0 )
	{
		return GROUND_PLASTIC;
	}

	return GROUND_NORMAL;
}

std::string Get_Color_Name( const DefaultColor color )
{
	switch( color )
	{
		case COL_DEFAULT:
		{
			return N_("default");
		}
		case COL_WHITE:
		{
			return N_("white");
		}
		case COL_BLACK:
		{
			return N_("black");
		}
		case COL_RED:
		{
			return N_("red");
		}
		case COL_ORANGE:
		{
			return N_("orange");
		}
		case COL_YELLOW:
		{
			return N_("yellow");
		}
		case COL_GREEN:
		{
			return N_("green");
		}
		case COL_BLUE:
		{
			return N_("blue");
		}
		case COL_BROWN:
		{
			return N_("brown");
		}
		case COL_GREY:
		{
			return N_("grey");
		}
		default:
		{
			break;
		}
	}

	return "";
}

DefaultColor Get_Color_Id( const std::string &str_color )
{
	if( str_color.compare( "white" ) == 0 )
	{
		return COL_WHITE;
	}
	else if( str_color.compare( "black" ) == 0 )
	{
		return COL_BLACK;
	}
	else if( str_color.compare( "red" ) == 0 )
	{
		return COL_RED;
	}
	else if( str_color.compare( "orange" ) == 0 )
	{
		return COL_ORANGE;
	}
	else if( str_color.compare( "yellow" ) == 0 )
	{
		return COL_YELLOW;
	}
	else if( str_color.compare( "green" ) == 0 )
	{
		return COL_GREEN;
	}
	else if( str_color.compare( "blue" ) == 0 )
	{
		return COL_BLUE;
	}
	else if( str_color.compare( "brown" ) == 0 )
	{
		return COL_BROWN;
	}
	else if( str_color.compare( "grey" ) == 0 )
	{
		return COL_GREY;
	}

	return COL_DEFAULT;
}

void Gui_Handle_Time( void )
{
	static float last_time_pulse = 0;

	// get current "run-time" in seconds
	float t = 0.001f * SDL_GetTicks();

	// inject the time that passed since the last call
	CEGUI::System::getSingleton().injectTimePulse( static_cast<float>( t - last_time_pulse ) );

	// store the new time as the last time
	last_time_pulse = t;
}

void Draw_Static_Text( const std::string &text, const Color *color_text /* = &white */, const Color *color_bg /* = NULL */, bool wait_for_input /* = 1 */ )
{
	// fixme : Can't handle multiple lines of text. Change to MultiLineEditbox or use HorzFormatting=WordWrapLeftAligned property.
	bool draw = 1;

	// Statictext window
	CEGUI::Window *window_statictext = CEGUI::WindowManager::getSingleton().loadWindowLayout( "statictext.layout" );
	pGuiSystem->getGUISheet()->addChildWindow( window_statictext );
	// get default text
	CEGUI::Window *text_default = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_default" ));

	// set text
	text_default->setProperty( "TextColours", CEGUI::PropertyHelper::colourToString( CEGUI::colour( static_cast<float>(color_text->red) / 255, static_cast<float>(color_text->green) / 255, static_cast<float>(color_text->blue) / 255, 1 ) ) );
	CEGUI::String gui_text = reinterpret_cast<const CEGUI::utf8*>(text.c_str());
	text_default->setText( gui_text );

	// align text
	CEGUI::Font *font = CEGUI::FontManager::getSingleton().getFont( "bluebold_medium" );
	float text_width = font->getTextExtent( gui_text ) * global_downscalex;

	text_default->setWidth( CEGUI::UDim( 0, ( text_width + 15 ) * global_upscalex ) );
	text_default->setXPosition( CEGUI::UDim( 0, ( game_res_w * 0.5f - text_width * 0.5f ) * global_upscalex ) );
	text_default->moveToFront();

	// set window height
	text_default->setHeight( CEGUI::UDim( 0, font->getFontHeight() * font->getFormattedLineCount( gui_text, text_default->getUnclippedInnerRect(), CEGUI::LeftAligned ) + ( 12 * global_upscaley ) ) );

	while( draw )
	{
		Draw_Game();

		// draw background
		if( color_bg )
		{
			// create request
			cRect_Request *request = new cRect_Request();

			pVideo->Draw_Rect( NULL, 0.9f, color_bg, request );
			request->render_count = wait_for_input ? 4 : 1;

			// add request
			pRenderer->Add( request );
		}

		pVideo->Render();

		if( wait_for_input )
		{
			while( SDL_PollEvent( &input_event ) )
			{
				if( input_event.type == SDL_KEYDOWN || input_event.type == SDL_JOYBUTTONDOWN || input_event.type == SDL_MOUSEBUTTONDOWN )
				{
					draw = 0;
				}
			}

			// slow down
			Correct_Frame_Time( 60 );
		}
		else
		{
			draw = 0;
		}

		pFramerate->Update();
	}

	// Clear possible active input
	if( wait_for_input )
	{
		Clear_Input_Events();
	}

	pGuiSystem->getGUISheet()->removeChildWindow( window_statictext );
	CEGUI::WindowManager::getSingleton().destroyWindow( window_statictext );
}

std::string Box_Text_Input( const std::string &default_text, const std::string &title_text, bool auto_no_text /* = 1 */ )
{
	cDialogBox_Text *box = new cDialogBox_Text();
	std::string return_value = box->Enter( default_text, title_text, auto_no_text );
	delete box;

	return return_value;
}

int Box_Question( const std::string &text, bool with_cancel /* = 0 */ )
{
	cDialogBox_Question *box = new cDialogBox_Question();
	int return_value = box->Enter( text, with_cancel );
	delete box;

	return return_value;
}

void Preload_Images( bool draw_gui /* = 0 */ )
{
	// progress bar
	CEGUI::ProgressBar *progress_bar = NULL;

	if( draw_gui )
	{
		// get progress bar
		progress_bar = static_cast<CEGUI::ProgressBar *>(CEGUI::WindowManager::getSingleton().getWindow( "progress_bar" ));
		progress_bar->setProgress( 0 );
		// set loading screen text
		Loading_Screen_Draw_Text( _("Loading Images") );
	}

	// image files
	vector<std::string> image_files;

	// player
	vector<std::string> player_small_images = Get_Directory_Files( DATA_DIR "/" GAME_PIXMAPS_DIR "/maryo/small", ".png", 0, 0 );
	vector<std::string> player_big_images = Get_Directory_Files( DATA_DIR "/" GAME_PIXMAPS_DIR "/maryo/big", ".png", 0, 0 );
	vector<std::string> player_fire_images = Get_Directory_Files( DATA_DIR "/" GAME_PIXMAPS_DIR "/maryo/fire", ".png", 0, 0 );
	vector<std::string> player_ice_images = Get_Directory_Files( DATA_DIR "/" GAME_PIXMAPS_DIR "/maryo/ice", ".png", 0, 0 );
	vector<std::string> player_ghost_images = Get_Directory_Files( DATA_DIR "/" GAME_PIXMAPS_DIR "/maryo/ghost", ".png", 0, 0 );

	image_files.insert( image_files.end(), player_small_images.begin(), player_small_images.end() );
	image_files.insert( image_files.end(), player_big_images.begin(), player_big_images.end() );
	image_files.insert( image_files.end(), player_fire_images.begin(), player_fire_images.end() );
	image_files.insert( image_files.end(), player_ice_images.begin(), player_ice_images.end() );
	image_files.insert( image_files.end(), player_ghost_images.begin(), player_ghost_images.end() );

	// Mushrooms
	image_files.push_back( "game/items/mushroom_red.png" );
	image_files.push_back( "game/items/mushroom_green.png" );
	image_files.push_back( "game/items/mushroom_blue.png" );
	image_files.push_back( "game/items/mushroom_ghost.png" );
	// Fireplant
	image_files.push_back( "game/items/fireplant.png" );
	image_files.push_back( "game/items/fireplant_left.png" );
	image_files.push_back( "game/items/fireplant_right.png" );
	// Star
	image_files.push_back( "game/items/star.png" );
	// Feather
	//image_files.push_back( "game/items/feather_1.png" );
	// Yellow Goldpiece
	image_files.push_back( "game/items/goldpiece/yellow/1.png" );
	image_files.push_back( "game/items/goldpiece/yellow/2.png" );
	image_files.push_back( "game/items/goldpiece/yellow/3.png" );
	image_files.push_back( "game/items/goldpiece/yellow/4.png" );
	image_files.push_back( "game/items/goldpiece/yellow/5.png" );
	image_files.push_back( "game/items/goldpiece/yellow/6.png" );
	image_files.push_back( "game/items/goldpiece/yellow/7.png" );
	image_files.push_back( "game/items/goldpiece/yellow/8.png" );
	image_files.push_back( "game/items/goldpiece/yellow/9.png" );
	image_files.push_back( "game/items/goldpiece/yellow/10.png" );
	image_files.push_back( "game/items/goldpiece/yellow/1_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/2_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/3_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/4_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/5_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/6_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/7_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/8_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/9_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/10_falling.png" );
	// Red Goldpiece
	image_files.push_back( "game/items/goldpiece/red/1.png" );
	image_files.push_back( "game/items/goldpiece/red/2.png" );
	image_files.push_back( "game/items/goldpiece/red/3.png" );
	image_files.push_back( "game/items/goldpiece/red/4.png" );
	image_files.push_back( "game/items/goldpiece/red/5.png" );
	image_files.push_back( "game/items/goldpiece/red/6.png" );
	image_files.push_back( "game/items/goldpiece/red/7.png" );
	image_files.push_back( "game/items/goldpiece/red/8.png" );
	image_files.push_back( "game/items/goldpiece/red/9.png" );
	image_files.push_back( "game/items/goldpiece/red/10.png" );
	image_files.push_back( "game/items/goldpiece/red/1_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/2_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/3_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/4_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/5_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/6_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/7_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/8_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/9_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/10_falling.png" );

	// Brown Box
	image_files.push_back( "game/box/brown1_1.png" );	

	// Light animation
	image_files.push_back( "animation/light_1/1.png" );
	image_files.push_back( "animation/light_1/2.png" );
	image_files.push_back( "animation/light_1/3.png" );
	// Fire animation
	image_files.push_back( "animation/fire_1/1.png" );
	image_files.push_back( "animation/fire_1/2.png" );
	image_files.push_back( "animation/fire_1/3.png" );
	image_files.push_back( "animation/fire_1/4.png" );
	// Particle animation
	image_files.push_back( "animation/particles/smoke.png" );
	image_files.push_back( "animation/particles/smoke_black.png" );
	image_files.push_back( "animation/particles/light.png" );
	image_files.push_back( "animation/particles/dirt.png" );
	image_files.push_back( "animation/particles/ice_1.png" );
	image_files.push_back( "animation/particles/cloud.png" );
	image_files.push_back( "animation/particles/axis.png" );

	// Ball
	image_files.push_back( "animation/fireball/1.png" );
	image_files.push_back( "animation/iceball/1.png" );

	// HUD
	image_files.push_back( "game/maryo_l.png" );
	image_files.push_back( "game/gold_m.png" );
	image_files.push_back( "game/itembox.png" );

	unsigned int loaded_files = 0;
	unsigned int file_count = image_files.size();

	// load images
	for( vector<std::string>::iterator itr = image_files.begin(), itr_end = image_files.end(); itr != itr_end; ++itr )
	{
		// get filename
		std::string filename = (*itr);

		// preload image
		pVideo->Get_Surface( filename );

		// count files
		loaded_files++;

		if( draw_gui )
		{
			// update progress
			progress_bar->setProgress( static_cast<float>(loaded_files) / static_cast<float>(file_count) );

			Loading_Screen_Draw();
		}
	}
}

void Preload_Sounds( bool draw_gui /* = 0 */ )
{
	// skip caching if disabled
	if( !pAudio->m_sound_enabled )
	{
		return;
	}

	// progress bar
	CEGUI::ProgressBar *progress_bar = NULL;

	if( draw_gui )
	{
		// get progress bar
		progress_bar = static_cast<CEGUI::ProgressBar *>(CEGUI::WindowManager::getSingleton().getWindow( "progress_bar" ));
		progress_bar->setProgress( 0 );
		// set loading screen text
		Loading_Screen_Draw_Text( _("Loading Sounds") );
	}

	// sound files
	vector<std::string> sound_files;

	// player
	sound_files.push_back( "wall_hit.wav" );
	sound_files.push_back( "player/dead.ogg" );
	sound_files.push_back( "itembox_get.ogg" );
	sound_files.push_back( "itembox_set.ogg" );
	sound_files.push_back( "player/pickup_item.wav" );
	sound_files.push_back( "player/jump_small.ogg" );
	sound_files.push_back( "player/jump_small_power.ogg" );
	sound_files.push_back( "player/jump_big.ogg" );
	sound_files.push_back( "player/jump_big_power.ogg" );
	sound_files.push_back( "player/jump_ghost.ogg" );
	// todo : create again
	//sound_files.push_back( "player/maryo_au.ogg" );
	sound_files.push_back( "player/powerdown.ogg" );
	sound_files.push_back( "player/ghost_end.ogg" );
	sound_files.push_back( "player/maryo_stop.ogg" );
	sound_files.push_back( "enter_pipe.ogg" );
	sound_files.push_back( "leave_pipe.ogg" );

	// items
	sound_files.push_back( "item/star_kill.ogg" );
	sound_files.push_back( "item/fireball.ogg" );
	sound_files.push_back( "item/fireball_explode.wav" );
	sound_files.push_back( "item/fireball_repelled.wav" );
	sound_files.push_back( "item/fireball_explosion.wav" );
	sound_files.push_back( "item/fireplant.ogg" );
	sound_files.push_back( "item/goldpiece_1.ogg" );
	sound_files.push_back( "item/goldpiece_red.wav" );
	sound_files.push_back( "item/live_up.ogg" );
	sound_files.push_back( "item/live_up_2.ogg" );
	sound_files.push_back( "item/mushroom.ogg" );
	sound_files.push_back( "item/mushroom_ghost.ogg" );
	sound_files.push_back( "item/moon.ogg" );

	// box
	sound_files.push_back( "item/empty_box.wav" );
	sound_files.push_back( "death_box.wav" );

	// enemies
	// eato
	sound_files.push_back( "enemy/eato/die.ogg" );
	// gee
	sound_files.push_back( "enemy/gee/die.ogg" );
	// furball
	sound_files.push_back( "enemy/furball/die.ogg" );
	// furball boss
	sound_files.push_back( "enemy/boss/furball/hit.wav" );
	sound_files.push_back( "enemy/boss/furball/hit_failed.wav" );
	// flyon
	sound_files.push_back( "enemy/flyon/die.ogg" );
	// krush
	sound_files.push_back( "enemy/krush/die.ogg" );
	// rokko
	sound_files.push_back( "enemy/rokko/activate.wav" );
	sound_files.push_back( "enemy/rokko/hit.wav" );
	// spika
	sound_files.push_back( "enemy/spika/move.ogg" );
	// thromp
	sound_files.push_back( "enemy/thromp/hit.ogg" );
	sound_files.push_back( "enemy/thromp/die.ogg" );
	// turtle
	sound_files.push_back( "enemy/turtle/hit.ogg" );
	sound_files.push_back( "enemy/turtle/shell/hit.ogg" );
	sound_files.push_back( "enemy/turtle/stand_up.wav" );
	// turtle boss
	sound_files.push_back( "enemy/boss/turtle/big_hit.ogg" );
	sound_files.push_back( "enemy/boss/turtle/shell_attack.ogg" );
	sound_files.push_back( "enemy/boss/turtle/power_up.ogg" );

	// default
	sound_files.push_back( "sprout_1.ogg" );
	sound_files.push_back( "stomp_1.ogg" );
	sound_files.push_back( "stomp_2.ogg" );
	sound_files.push_back( "stomp_4.ogg" );

	// savegame
	sound_files.push_back( "savegame_load.ogg" );
	sound_files.push_back( "savegame_save.ogg" );

	// overworld
	sound_files.push_back( "waypoint_reached.ogg" );

	unsigned int loaded_files = 0;
	unsigned int file_count = sound_files.size();

	// load images
	for( vector<std::string>::iterator itr = sound_files.begin(), itr_end = sound_files.end(); itr != itr_end; ++itr )
	{
		// get filename
		std::string filename = (*itr);

		// preload it
		pAudio->Get_Sound_File( filename );

		// count files
		loaded_files++;

		if( draw_gui )
		{
			// update progress
			progress_bar->setProgress( static_cast<float>(loaded_files) / static_cast<float>(file_count) );

			Loading_Screen_Draw();
		}
	}
}

void Relocate_Image( CEGUI::XMLAttributes &xml_attributes, const std::string &filename_old, const std::string &filename_new, const CEGUI::String &attribute_name /* = "image" */ )
{
	if( xml_attributes.getValueAsString( attribute_name ).compare( filename_old ) == 0 || xml_attributes.getValueAsString( attribute_name ).compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" + filename_old ) == 0 )
	{
		xml_attributes.remove( attribute_name );
		xml_attributes.add( attribute_name, filename_new );
	}
}

std::string Get_Clipboard_Content( void )
{
	std::string content;
#ifdef _WIN32
	if( OpenClipboard( NULL ) )
	{
		HANDLE h = GetClipboardData( CF_TEXT );

		// no handle
		if( !h )
		{
			printf( "Could not get clipboard data\n" );
			CloseClipboard();
			return content;
		}

		// get content
		content = static_cast<char *>(GlobalLock( h ));

		// clean up
		GlobalUnlock( h );
		CloseClipboard();
	}
#elif __APPLE__
	// not tested
	ScrapRef scrap;
	if( ::GetCurrentScrap( &scrap ) != noErr )
	{
		return false;
	}

	Size bytecount = 0;
	OSStatus status = ::GetScrapFlavorSize( scrap, kScrapFlavorTypeText, &bytecount );
	if( status != noErr )
	{
		return false;
	}

	char *buffer = new char[bytecount];
	if( ::GetScrapFlavorData( scrap, kScrapFlavorTypeText, &bytecount, buffer ) == noErr )
	{
		content = static_cast<char *>(buffer);
	}

	delete[] buffer;
#elif __unix__
	// only works with the cut-buffer method (xterm) and not with the more recent selections method
	SDL_SysWMinfo sdlinfo;
	SDL_VERSION( &sdlinfo.version );
	if( SDL_GetWMInfo( &sdlinfo ) )
	{
		sdlinfo.info.x11.lock_func();
		Display *display = sdlinfo.info.x11.display;
		int count = 0;
		char *msg = XFetchBytes( display, &count );
		if( msg )
		{
			if( count > 0 )
			{
				content.append( msg, count );
			}

			XFree( msg );
		}

		sdlinfo.info.x11.unlock_func();
	}
#endif
	return content;
}

void Set_Clipboard_Content( std::string str )
{
#ifdef _WIN32
	if( OpenClipboard( NULL ) )
	{
		if( !EmptyClipboard() )
		{
			printf( "Failed to empty clipboard\n" );
			return;
		}

		unsigned int length = ( str.length() + 1 ) * sizeof(std::string::allocator_type);
		HANDLE h = GlobalAlloc( (GMEM_MOVEABLE|GMEM_DDESHARE|GMEM_ZEROINIT), length );

		if( !h )
		{
			printf( "Could not allocate clipboard memory\n" );
			return;
		}

		void *data = GlobalLock( h );

		if( !data )
		{
			GlobalFree( h );
			CloseClipboard();
			printf( "Could not lock clipboard memory\n" );
			return;
		}

		memcpy( data, str.c_str(), length );
		GlobalUnlock( h );

		HANDLE data_result = SetClipboardData( CF_TEXT, h );

		if( !data_result )
		{
			GlobalFree( h );
			CloseClipboard();
			printf( "Could not set clipboard data\n" );
		}

		CloseClipboard();
	}
#elif __APPLE__
	// not implemented
#elif __unix__
	SDL_SysWMinfo sdlinfo;
	SDL_VERSION( &sdlinfo.version );
	if( SDL_GetWMInfo( &sdlinfo ) )
	{
		sdlinfo.info.x11.lock_func();
		Display *display = sdlinfo.info.x11.display;
		Window window = sdlinfo.info.x11.window;

		XChangeProperty( display, DefaultRootWindow(display), XA_CUT_BUFFER0, XA_STRING, 8, PropModeReplace, static_cast<const unsigned char *>(static_cast<const void *>(str.c_str())), str.length() );

		if( XGetSelectionOwner( display, XA_PRIMARY ) != window )
		{
			XSetSelectionOwner( display, XA_PRIMARY, window, CurrentTime );
		}

		sdlinfo.info.x11.unlock_func();
	}
#endif
}

bool GUI_Copy_To_Clipboard( bool cut )
{
	CEGUI::Window *sheet = CEGUI::System::getSingleton().getGUISheet();

	// no sheet
	if( !sheet )
	{
		return 0;
	}

	CEGUI::Window *window_active = sheet->getActiveChild();

	// no active window
	if( !window_active )
	{
		return 0;
	}

	CEGUI::String sel_text;
	const CEGUI::String &type = window_active->getType();

	// MultiLineEditbox
	if( type.find( "/MultiLineEditbox" ) != CEGUI::String::npos )
	{
		CEGUI::MultiLineEditbox *editbox = static_cast<CEGUI::MultiLineEditbox*>(window_active);
		CEGUI::String::size_type beg = editbox->getSelectionStartIndex();
		CEGUI::String::size_type len = editbox->getSelectionLength();
		sel_text = editbox->getText().substr( beg, len ).c_str();

		// if cutting
		if( cut )
		{
			if( editbox->isReadOnly() )
			{
				return 0;
			}

			CEGUI::String new_text = editbox->getText();
			editbox->setText( new_text.erase( beg, len ) );
		}
	}
	// Editbox
	else if( type.find( "/Editbox" ) != CEGUI::String::npos )
	{
		CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox*>(window_active);
		CEGUI::String::size_type beg = editbox->getSelectionStartIndex();
		CEGUI::String::size_type len = editbox->getSelectionLength();
		sel_text = editbox->getText().substr( beg, len ).c_str();

		// if cutting
		if( cut )
		{
			if( editbox->isReadOnly() )
			{
				return 0;
			}

			CEGUI::String new_text = editbox->getText();
			editbox->setText( new_text.erase( beg, len ) );
		}
	}
	else
	{
		return 0;
	}

	Set_Clipboard_Content( sel_text.c_str() );
	return 1;
}

bool GUI_Paste_From_Clipboard( void )
{
	CEGUI::Window *sheet = CEGUI::System::getSingleton().getGUISheet();

	// no sheet
	if( !sheet )
	{
		return 0;
	}

	CEGUI::Window *window_active = sheet->getActiveChild();

	// no active window
	if( !window_active )
	{
		return 0;
	}

	const CEGUI::String &type = window_active->getType();

	// MultiLineEditbox
	if( type.find( "/MultiLineEditbox" ) != CEGUI::String::npos )
	{
		CEGUI::MultiLineEditbox *editbox = static_cast<CEGUI::MultiLineEditbox*>(window_active);

		if( editbox->isReadOnly() )
		{
			return 0;
		}

		CEGUI::String::size_type beg = editbox->getSelectionStartIndex();
		CEGUI::String::size_type len = editbox->getSelectionLength();

		CEGUI::String new_text = editbox->getText();
		// erase selected text
		new_text.erase( beg, len );

		// get clipboard text
		CEGUI::String clipboard_text = reinterpret_cast<const CEGUI::utf8*>(Get_Clipboard_Content().c_str());
		// set new text
		editbox->setText( new_text.insert( beg, clipboard_text ) );
		// set new carat index
		editbox->setCaratIndex( editbox->getCaratIndex() + clipboard_text.length() );
	}
	// Editbox
	else if( type.find( "/Editbox" ) != CEGUI::String::npos )
	{
		CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox*>(window_active);

		if( editbox->isReadOnly() )
		{
			return 0;
		}

		CEGUI::String::size_type beg = editbox->getSelectionStartIndex();
		CEGUI::String::size_type len = editbox->getSelectionLength();

		CEGUI::String new_text = editbox->getText();
		// erase selected text
		new_text.erase( beg, len );

		// get clipboard text
		CEGUI::String clipboard_text = reinterpret_cast<const CEGUI::utf8*>(Get_Clipboard_Content().c_str());
		// set new text
		editbox->setText( new_text.insert( beg, clipboard_text ) );
		// set new carat index
		editbox->setCaratIndex( editbox->getCaratIndex() + clipboard_text.length() );
	}
	else
	{
		return 0;
	}

	return 1;
}

std::string int_to_string( const int number )
{
	std::ostringstream temp;
	temp << number;

	return temp.str();
}

std::string string_trim_from_end( std::string str, const char c )
{
	// find last position from end which is not the given character
	size_t endpos = str.find_last_not_of( c );

	// if all match or empty
	if( endpos == std::string::npos )
	{
		return str;
	}
	else
	{
		return str.substr( 0, endpos + 1 );
	}
}

// from stringencoders for float_to_string
/**
 * Powers of 10
 * 10^0 to 10^9
 */
static const double pow_of_10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

// from stringencoders for float_to_string
static void strreverse(char* begin, char* end)
{
	char aux;
	while (end > begin)
		aux = *end, *end-- = *begin, *begin++ = aux;
}

// from stringencoders with modifications ( Copyright (C) 2007 Nick Galbreath ) - BSD License
std::string float_to_string( const float number, int prec /* = 6 */ )
{
	double value = number;
	/* if input is larger than thres_max, revert to native */
	const double thres_max = static_cast<double>(0x7FFFFFFF);

	double diff = 0.0;
	char str[64];
	char* wstr = str;

	if (prec < 0)
	{
		prec = 0;
	}
	else if (prec > 6)
	{
		/* precision of >= 7 for float can lead to overflow errors */
		prec = 6;
	}

	/* we'll work in positive values and deal with the
	   negative sign issue later */
	int neg = 0;
	if (value < 0)
	{
		neg = 1;
		value = -value;
	}

	int whole = static_cast<int>(value);
	double tmp = (value - whole) * pow_of_10[prec];
	uint32_t frac = static_cast<uint32_t>(tmp);
	diff = tmp - frac;

	if(diff > 0.5)
	{
		++frac;
		/* handle rollover, e.g.  case 0.99 with prec 1 is 1.0  */
		if(frac >= pow_of_10[prec])
		{
			frac = 0;
			++whole;
		}
	}
	else if( diff == 0.5 && ((frac == 0) || (frac & 1)) )
	{
		/* if halfway, round up if odd, OR
		   if last digit is 0.  That last part is strange */
		++frac;
	}

	/* for very large numbers switch back to native */
	/*
		normal printf behavior is to print EVERY whole number digit
		which can be 100s of characters overflowing your buffers == bad
	*/
	if( value > thres_max )
	{
		std::ostringstream temp;
		temp.setf( std::ios_base::fixed );
		temp << number;

		return temp.str();
	}

	if( prec == 0 )
	{
		diff = value - whole;
		if(diff > 0.5)
		{
			/* greater than 0.5, round up, e.g. 1.6 -> 2 */
			++whole;
		}
		else if(diff == 0.5 && (whole & 1))
		{
			/* exactly 0.5 and ODD, then round up */
			/* 1.5 -> 2, but 2.5 -> 2 */
			++whole;
		}
	}
	else
	{
		int count = prec;
		// now do fractional part, as an unsigned number
		do
		{
			--count;
			*wstr++ = 48 + (frac % 10);
		}
		while(frac /= 10);
		// add extra 0s
		while(count-- > 0) *wstr++ = '0';
		// add decimal
		*wstr++ = '.';
	}

	// do whole part
	// Take care of sign
	// Conversion. Number is reversed.
	do *wstr++ = 48 + (whole % 10); while (whole /= 10);
	if(neg)
	{
		*wstr++ = '-';
	}

	*wstr='\0';
	strreverse(str, wstr-1);

	return str;
}

// string conversion helper
template <class T> bool from_string( T &t, const std::string &s, std::ios_base &(*f)(std::ios_base&) )
{
	std::istringstream iss( s );
	return !(iss >> f >> t).fail();
}

float string_to_float( const std::string &str )
{
	float num;
	// use helper
	from_string<float>( num, str, std::dec );
	return num;
}

int string_to_int( const std::string &str )
{
	int num;
	// use helper
	from_string<int>( num, str, std::dec );
	return num;
}

double string_to_double( const std::string &str )
{
	double num;
	// use helper
	from_string<double>( num, str, std::dec );
	return num;
}

std::string string_to_xml_string( const std::string &str )
{
	std::string res;
	res.reserve( str.size() * 2 );

	const std::string::const_iterator iterEnd = str.end();
	for( std::string::const_iterator iter = str.begin(); iter != iterEnd ; ++iter )
	{
		switch( *iter )
		{
			case '<':
				res += "&lt;";
				break;

			case '>':
				res += "&gt;";
				break;

			case '&':
				res += "&amp;";
				break;

			case '\'':
				res += "&apos;";
				break;

			case '"':
				res += "&quot;";
				break;

			case '\n':
				res += "&lt;br/&gt;";
				break;

			default:
				res += *iter;
		}
	}

	return res;
}

std::string xml_string_to_string( std::string str )
{
	while( 1 )
	{
		std::string::size_type pos = str.find( "<br/>" );

		if( pos == std::string::npos )
		{
			break;
		}

		str.replace( pos, 5, "\n" );
	}

	return str;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
