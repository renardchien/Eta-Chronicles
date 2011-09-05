/***************************************************************************
 * menu.cpp  -  menu handler
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
#include "../gui/menu.h"
#include "../gui/menu_data.h"
#include "../core/game_core.h"
#include "../core/framerate.h"
#include "../input/mouse.h"
#include "../audio/audio.h"
#include "../player/player.h"
#include "../video/gl_surface.h"
#include "../level/level.h"
#include "../overworld/overworld.h"
#include "../user/preferences.h"
// CEGUI
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cMenu_Item :: cMenu_Item( void )
: cHudSprite()
{
	Set_Scale_Directions( 1, 1, 1, 1 );
	active = 0;
	isquit = 0;

	image_default = new cHudSprite();
	image_menu = new cHudSprite();
}

cMenu_Item :: ~cMenu_Item( void )
{
	if( image_default )
	{
		delete image_default;
	}

	if( image_menu )
	{
		delete image_menu;
	}
}

void cMenu_Item :: Set_Active( bool nactive /* = 0 */ )
{
	active = nactive;
	m_rot_z = 0;
	Set_Scale( 1 );

	if( !nactive )
	{
		Set_Color_Combine( 0, 0, 0, 0 );
	}
}

void cMenu_Item :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( active )
	{
		// rotation is used for the scale state
		if( !m_rot_z )
		{
			Add_Scale( ( 1.2f / m_image->m_w ) * pFramerate->m_speed_factor );
		}
		else
		{
			Add_Scale( -( 1.2f / m_image->m_w ) * pFramerate->m_speed_factor );
		}

		if( m_image->m_w * m_scale_x > m_image->m_w + 10.0f )
		{
			m_rot_z = 0.0001f;
		}
		else if( m_scale_x < 1.0f )
		{
			m_rot_z = 0.0f;
		}
	}

	cHudSprite::Draw( request );

	if( active )
	{
		float strength = m_image->m_w * ( m_scale_x - 1 );

		// boost color to yellow
		Set_Color_Combine( strength / 40, strength / 40, 0, GL_ADD );

		m_pos_x = m_start_pos_x;
		m_pos_y = m_start_pos_y;

		image_menu->Draw();
	}
}

/* *** *** *** *** *** *** cMenuHandler *** *** *** *** *** *** *** *** *** *** *** */

cMenuHandler :: cMenuHandler( void )
{
	m_level = new cLevel();
	m_player = new cSprite();
	m_player->Set_Massive_Type( MASS_PASSIVE );
	Reset();

	m_level->Load( pPreferences->m_menu_level );

	// SMC logo image
	cHudSprite *sprite = new cHudSprite( pVideo->Get_Surface( "game/logo/smc_big_1.png" ), 180, 20 );
	sprite->Set_Scale( 0.8f );
	sprite->Set_Sprite_Type( TYPE_FRONT_PASSIVE );
	m_level->m_sprite_manager->Add( sprite );
}

cMenuHandler :: ~cMenuHandler( void )
{
	Reset();

	delete m_level;
	delete m_player;
}

void cMenuHandler :: Add_Menu_Item( cMenu_Item *item, float shadow_pos /* = 0 */, Color shadow_color /* = static_cast<Uint8>(0) */ )
{
	if( !item )
	{
		printf( "Menu item is NULL ( current Menu size : %d )\n", Get_Size() );
		return;
	}

	item->Set_Shadow_Pos( shadow_pos );
	item->Set_Shadow_Color( shadow_color );
	item->Set_Image( item->image_default->m_image );
	items.push_back( item );

	if( active == -1 && Get_Size() == 1 )
	{
		Set_Active( 0 );
	}
}

void cMenuHandler :: Reset( void )
{
	for( MenuList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	items.clear();

	// nothing is active
	active = -1;
}

void cMenuHandler :: Set_Active( int num )
{
	// if not already active and exists
	if( num == static_cast<int>(active) || num >= static_cast<int>(items.size()) || ( num >= 0 && !items[num] ) )
	{
		return;
	}

	if( num >= 0 && static_cast<unsigned int>(num) < items.size() )
	{
		// set last active item un-active
		if( active >= 0 && static_cast<unsigned int>(active) < items.size() )
		{
			items[active]->Set_Active( 0 );
		}
	}
	else if( num == -1 )
	{
		items[active]->Set_Active( 0 );
	}

	active = num;

	if( active >= 0 )
	{
		items[active]->Set_Active( 1 );
	}
}

void cMenuHandler :: Update_Mouse( void )
{
	int found = -1;

	// check
	for( unsigned int i = 0; i < items.size(); i++ )
	{
		if( items[i]->m_col_rect.Intersects( static_cast<float>(pMouseCursor->m_x), static_cast<float>(pMouseCursor->m_y) ) )
		{
			found = i;
			break;
		}
	}

	// ignore mouse init
	if( found < 0 && input_event.motion.x == pMouseCursor->m_x )
	{
		return;
	}

	Set_Active( found );
}

void cMenuHandler :: Update( void )
{
	// level
	m_level->Update();
	// collision and movement handling
	pActive_Sprite_Manager->Handle_Collision_Items();
}

void cMenuHandler :: Draw( bool with_background /* = 1 */ )
{
	if( with_background )
	{
		// draw menu level
		m_level->Draw_Layer_1();
		m_level->m_global_effect->Draw();
	}

	// menu items
	for( MenuList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}
}

cMenu_Item *cMenuHandler :: Get_Active_Item( void )
{
	if( active < 0 || static_cast<unsigned int>(active) > items.size() )
	{
		return NULL;
	}

	return items[active];
}

unsigned int cMenuHandler :: Get_Size( void ) const
{
	return static_cast<unsigned int>(items.size());
}

/* *** *** *** *** *** *** *** cMenuCore *** *** *** *** *** *** *** *** *** *** */

cMenuCore :: cMenuCore( void )
{
	menu_id = MENU_NOTHING;
	next_menu = MENU_NOTHING;
	options_menu_id = MENU_GAME;

	camera = new cCamera();
	pMenu_Data = NULL;
	handler = new cMenuHandler();

	// particle animation
	pMenu_AnimManager = new cAnimation_Manager();

	// left side
	cParticle_Emitter *anim = new cParticle_Emitter();
	anim->Set_Emitter_Rect( -100, 0, 0, game_res_h * 0.5f );
	anim->Set_Emitter_Time_to_Live( -1 );
	anim->Set_Emitter_Iteration_Interval( 16 );
	anim->Set_Direction_Range( 350, 20 );
	anim->Set_Image( pVideo->Get_Surface( "clouds/default_1/1_middle.png" ) );
	anim->Set_Time_to_Live( 800 );
	anim->Set_Fading_Alpha( 0 );
	anim->Set_Scale( 0.2f, 0.2f );
	anim->Set_Color( Color( static_cast<Uint8>(255), 255, 255, 200 ), Color( static_cast<Uint8>(0), 0, 0, 55 ) );
	anim->Set_Speed( 0.05f, 0.005f );
	anim->Set_Pos_Z( 0.0015f, 0.0004f );

	pMenu_AnimManager->Add( anim );

	// right side
	anim = new cParticle_Emitter();
	anim->Set_Emitter_Rect( static_cast<float>(game_res_w) + 100, 0, 0, static_cast<float>(game_res_h) * 0.5f );
	anim->Set_Emitter_Time_to_Live( -1 );
	anim->Set_Emitter_Iteration_Interval( 16 );
	anim->Set_Direction_Range( 170, 20 );
	anim->Set_Image( pVideo->Get_Surface( "clouds/default_1/1_middle.png" ) );
	anim->Set_Time_to_Live( 800 );
	anim->Set_Fading_Alpha( 0 );
	anim->Set_Scale( 0.2f, 0.2f );
	anim->Set_Color( Color( static_cast<Uint8>(255), 255, 255, 200 ), Color( static_cast<Uint8>(0), 0, 0, 55 ) );
	anim->Set_Speed( 0.05f, 0.005f );
	anim->Set_Pos_Z( 0.0015f, 0.0004f );

	pMenu_AnimManager->Add( anim );
}

cMenuCore :: ~cMenuCore( void )
{
	Unload();

	delete camera;
	delete handler;
	delete pMenu_AnimManager;
}

bool cMenuCore :: Handle_Event( SDL_Event *ev )
{
	switch( ev->type )
	{
	case SDL_MOUSEMOTION:
	{
		handler->Update_Mouse();
		break;
	}
	// other events
	default:
	{
		break;
	}
	}

	return 0;
}

bool cMenuCore :: Key_Down( SDLKey key )
{
	// Down
	if( key == SDLK_DOWN )
	{
		if( handler->Get_Size() <= static_cast<unsigned int>( handler->active + 1 ) )
		{
			handler->Set_Active( 0 );
		}
		else
		{
			handler->Set_Active( handler->active + 1 );
		}
	}
	// Up
	else if( key == SDLK_UP )
	{
		if( handler->active <= 0 )
		{
			handler->Set_Active( handler->Get_Size() - 1 );
		}
		else
		{
			handler->Set_Active( handler->active - 1 );
		}
	}
	// Activate Button
	else if( key == SDLK_RETURN || key == SDLK_KP_ENTER )
	{
		if( pMenu_Data )
		{
			pMenu_Data->action = 1;
		}
	}
	// Fast Debug Level entering
	else if( key == SDLK_x && input_event.key.keysym.mod & KMOD_CTRL )
	{
		// random level name
		std::string lvl_name;

		if( !CEGUI::WindowManager::getSingleton().isWindowPresent( "listbox_levels" ) )
		{
			// Create virtual start menu
			cMenu_Start *menu_start = new cMenu_Start();

			menu_start->Init();
			// Get Levels Listbox
			CEGUI::Listbox *listbox_levels = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_levels" ));
			// select random level
			listbox_levels->setItemSelectState( rand() % listbox_levels->getItemCount(), 1 );
			// get level name
			lvl_name = listbox_levels->getFirstSelectedItem()->getText().c_str();
			// destroy virtual menu
			delete menu_start;
		}
		else
		{
			// Get Levels Listbox
			CEGUI::Listbox *listbox_levels = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_levels" ));
			// select random level
			listbox_levels->setItemSelectState( rand() % listbox_levels->getItemCount(), 1 );
			// get level name
			lvl_name = listbox_levels->getFirstSelectedItem()->getText().c_str();
		}

		Game_Action = GA_ENTER_LEVEL;
		Game_Action_Data.add( "level", lvl_name.c_str() );
	}
	// exit
	else if( key == SDLK_ESCAPE )
	{
		pMenu_Data->Exit();
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cMenuCore :: Key_Up( SDLKey key )
{
	// nothing yet
	if( 0 )
	{
		//
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cMenuCore :: Joy_Button_Down( Uint8 button )
{
	// Activate button
	if( button == pPreferences->m_joy_button_action )
	{
		if( pMenu_Data )
		{
			pMenu_Data->action = 1;
		}
	}
	// exit
	else if( button == pPreferences->m_joy_button_exit )
	{
		pMenu_Data->Exit();
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cMenuCore :: Joy_Button_Up( Uint8 button )
{
	// nothing yet
	if( 0 )
	{
		//
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cMenuCore :: Mouse_Down( Uint8 button )
{
	// nothing yet
	if( button == SDL_BUTTON_LEFT )
	{
		cMenu_Item *item = handler->Get_Active_Item();

		if( item && item->m_col_rect.Intersects( static_cast<float>(pMouseCursor->m_x), static_cast<float>(pMouseCursor->m_y) ) )
		{
			pMenu_Data->action = 1;
			return 1;
		}
	}
	else
	{
		// not processed
		return 0;
	}

	// button got processed
	return 1;
}

bool cMenuCore :: Mouse_Up( Uint8 button )
{
	// nothing yet
	if( 0 )
	{
		//
	}
	else
	{
		// not processed
		return 0;
	}

	// button got processed
	return 1;
}

cMenu_Item *cMenuCore :: Auto_Menu( std::string imagename, std::string imagefilename_menu, float ypos /* = 0 */, bool is_quit /* = 0 */ )
{
	cMenu_Item *temp_item = new cMenu_Item();

	// the menu image
	if( imagefilename_menu.length() > 0 )
	{
		temp_item->image_menu->Set_Image( pVideo->Get_Surface( DATA_DIR "/" GAME_PIXMAPS_DIR "/menu/items/" + imagefilename_menu ), 1 );
	}

	// the active image
	if( imagename.length() > 0 )
	{
		temp_item->image_default->Set_Image( pVideo->Get_Surface( DATA_DIR "/" GAME_PIXMAPS_DIR "/menu/" + imagename ), 1 );
	}

	// position and initialization
	temp_item->Set_Pos( ( game_res_w * 0.5f ) - ( temp_item->image_default->m_col_rect.m_w * 0.5f ), ypos );
	temp_item->isquit = is_quit;

	return temp_item;
}

void cMenuCore :: Load( MenuID menu /* = MENU_MAIN */, GameMode exit_gamemode /* = MODE_NOTHING */ )
{
	Unload();
	// reset menu handler
	handler->Reset();

	// unload level if possible
	if( pActive_Level->m_delayed_unload )
	{
		pActive_Level->Unload();
	}

	// clear mouse active object
	pMouseCursor->Double_Click( 0 );

	// default background color to white
	glClearColor( 1, 1, 1, 1 );

	// if not entering from another menu
	if( next_menu == MENU_NOTHING )
	{
		// clear animations
		static_cast<cParticle_Emitter *>(pMenu_AnimManager->objects[0])->Clear();

		// set camera position
		camera->Set_Pos( 0, 0 );

		// pre update
		pFramerate->m_speed_factor = 4.0f;

		for( unsigned int i = 0; i < speedfactor_fps * 200; i++ )
		{
			pMenu_AnimManager->Update();
		}

		pFramerate->Update();
	}
	else
	{
		next_menu = MENU_NOTHING;
	}

	// Set ID
	menu_id = menu;

	// ## Create menu class
	// Main
	if( menu_id == MENU_MAIN )
	{
		pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Main());
	}
	// Start
	else if( menu_id == MENU_START )
	{
		pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Start());
	}
	// Options
	else if( menu_id == MENU_OPTIONS )
	{
		// Game
		if( options_menu_id == MENU_GAME )
		{
			pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Options_Game());
		}
		// Video
		else if( options_menu_id == MENU_VIDEO )
		{
			pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Options_Video());
		}
		// Audio
		else if( options_menu_id == MENU_AUDIO )
		{
			pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Options_Audio());
		}
		// Controls
		else if( options_menu_id == MENU_CONTROLS )
		{
			pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Options_Controls());
		}
	}
	// Load
	else if( menu_id == MENU_LOAD )
	{
		pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Savegames( 0 ));
	}
	// Save
	else if( menu_id == MENU_SAVE )
	{
		pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Savegames( 1 ));
	}
	// Credits
	else if( menu_id == MENU_CREDITS )
	{
		pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Credits());
	}

	pMenu_Data->Set_Exit_To_Game_Mode( exit_gamemode );
	pMenu_Data->Init();
}

void cMenuCore :: Unload( void )
{
	menu_id = MENU_NOTHING;

	if( pMenu_Data )
	{
		delete pMenu_Data;
		pMenu_Data = NULL;
	}
}

void cMenuCore :: Update( void ) 
{
	if( !pMenu_Data )
	{
		return;
	}

	// if not in a level/world
	if( pMenu_Data->exit_to_gamemode == MODE_NOTHING )
	{
		handler->Update();
	}

	pMenu_Data->Update();
}

void cMenuCore :: Draw( void ) 
{
	if( !pMenu_Data )
	{
		return;
	}

	pMenu_Data->Draw();

	// if no vsync limit the fps for not stressing CPU usage
	if( !pPreferences->m_video_vsync )
	{
		Correct_Frame_Time( 100 );
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cMenuCore *pMenuCore = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
