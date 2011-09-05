/***************************************************************************
 * menu_data.cpp  -  menu data and handling classes
 *
 * Copyright (C) 2004 - 2009 Florian Richter
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
#include "../gui/menu_data.h"
#include "../audio/audio.h"
#include "../core/game_core.h"
#include "../video/font.h"
#include "../overworld/overworld.h"
#include "../user/preferences.h"
#include "../input/joystick.h"
#include "../input/mouse.h"
#include "../core/framerate.h"
#include "../user/savegame.h"
#include "../video/renderer.h"
#include "../level/level.h"
#include "../input/keyboard.h"
#include "../level/level_editor.h"
#include "../core/math/utilities.h"
#include "../core/i18n.h"
#include "../core/math/size.h"
#include "../core/filesystem/filesystem.h"
#include "../core/filesystem/resource_manager.h"
// boost filesystem
#include "boost/filesystem/convenience.hpp"
namespace fs = boost::filesystem;

namespace SMC
{

/* *** *** *** *** *** *** *** *** cMenu_Base *** *** *** *** *** *** *** *** *** */

cMenu_Base :: cMenu_Base( void )
{
	guiwindow = NULL;
	action = 0;
	menu_posy = 140;
	text_color = Color( static_cast<Uint8>(255), 251, 98 );
	text_color_value = Color( static_cast<Uint8>(255), 190, 30 );

	exit_to_gamemode = MODE_NOTHING;
}

cMenu_Base :: ~cMenu_Base( void )
{
	if( guiwindow )
	{
		pGuiSystem->getGUISheet()->removeChildWindow( guiwindow );
		CEGUI::WindowManager::getSingleton().destroyWindow( guiwindow );
	}

	for( HudSpriteList::iterator itr = drawlist.begin(), itr_end = drawlist.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}
	
	drawlist.clear();
}

void cMenu_Base :: Init( void )
{
	// continue playing level/world music but not if finished level 
	if( !pAudio->Is_Music_Playing() || ( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM && !pActive_Level->Is_Loaded() ) )
	{
		pAudio->Play_Music( "game/menu.ogg", -1, 0, 1500 );
	}

	// init
	Change_Game_Mode( MODE_MENU );

	layout_file = "";
}

void cMenu_Base :: Init_GUI( void )
{
	if( layout_file.empty() )
	{
		return;
	}

	guiwindow = CEGUI::WindowManager::getSingleton().loadWindowLayout( layout_file.c_str() );
	pGuiSystem->getGUISheet()->addChildWindow( guiwindow );
}

void cMenu_Base :: Update( void )
{
	// animation
	pMenuCore->pMenu_AnimManager->Update();
	// hud
	pHud_Manager->Update();
}

void cMenu_Base :: Draw( void )
{
	pVideo->Clear_Screen();

	if( exit_to_gamemode == MODE_LEVEL )
	{
		pActive_Level->m_sprite_manager->Update_Items_Valid_Draw();
		// draw level layer 1
		pActive_Level->Draw_Layer_1();
		// draw alpha rect
		pVideo->Draw_Rect( NULL, 0.125f, &blackalpha128 );

		// gui
		pMenuCore->handler->Draw( 0 );
	}
	else if( exit_to_gamemode == MODE_OVERWORLD )
	{
		pActive_Overworld->m_sprite_manager->Update_Items_Valid_Draw();
		// draw world layer 1
		pActive_Overworld->Draw_Layer_1();
		// draw alpha rect
		pVideo->Draw_Rect( NULL, 0.125f, &blackalpha128 );

		// gui
		pMenuCore->handler->Draw( 0 );
	}
	else
	{
		// animation
		pMenuCore->pMenu_AnimManager->Draw();
		// gui
		pMenuCore->handler->Draw();
	}


	// menu items
	for( HudSpriteList::iterator itr = drawlist.begin(), itr_end = drawlist.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}
}

void cMenu_Base :: Draw_End( void )
{
	// hud
	pHud_Manager->Draw();
}

void cMenu_Base :: Set_Exit_To_Game_Mode( GameMode gamemode )
{
	exit_to_gamemode = gamemode;
}

void cMenu_Base :: Exit( void )
{
	// virtual
}

/* *** *** *** *** *** *** *** *** cMenu_Main *** *** *** *** *** *** *** *** *** */

cMenu_Main :: cMenu_Main( void )
: cMenu_Base()
{

}

cMenu_Main :: ~cMenu_Main( void )
{

}

void cMenu_Main :: Init( void )
{
	cMenu_Base::Init();

	cMenu_Item *temp_item = NULL;

	layout_file = "menu/main.layout";

	cGL_Surface *credits = NULL;

	if( exit_to_gamemode == MODE_NOTHING )
	{
		credits = pFont->Render_Text( pFont->m_font_normal, _("Credits"), yellow );
	}

	// Start
	temp_item = pMenuCore->Auto_Menu( "start.png", "start.png", menu_posy );
	temp_item->image_menu->Set_Pos( temp_item->m_pos_x + ( temp_item->image_default->m_col_rect.m_w + 16 ), temp_item->m_pos_y );
	pMenuCore->handler->Add_Menu_Item( temp_item );
	// Options
	menu_posy += 60;
	temp_item = pMenuCore->Auto_Menu( "options.png", "options.png", menu_posy );
	temp_item->image_menu->Set_Pos( temp_item->m_pos_x - temp_item->image_menu->m_col_rect.m_w - 16, temp_item->m_pos_y );
	pMenuCore->handler->Add_Menu_Item( temp_item );
	// Load
	menu_posy += 60;
	temp_item = pMenuCore->Auto_Menu( "load.png", "load.png", menu_posy );
	temp_item->image_menu->Set_Pos( temp_item->m_pos_x + ( temp_item->image_default->m_col_rect.m_w + 16 ), temp_item->m_pos_y );
	pMenuCore->handler->Add_Menu_Item( temp_item );
	// Save
	menu_posy += 60;
	temp_item = pMenuCore->Auto_Menu( "save.png", "save.png", menu_posy );
	temp_item->image_menu->Set_Pos( temp_item->m_pos_x - temp_item->image_menu->m_col_rect.m_w - 16, temp_item->m_pos_y );
	pMenuCore->handler->Add_Menu_Item( temp_item );
	// Quit
	menu_posy += 60;
	temp_item = pMenuCore->Auto_Menu( "quit.png", "", menu_posy, 1 );
	temp_item->image_menu->Set_Pos( temp_item->m_pos_x + temp_item->m_col_rect.m_w + 16, temp_item->m_pos_y );
	pMenuCore->handler->Add_Menu_Item( temp_item );

	if( exit_to_gamemode == MODE_NOTHING )
	{
		// Credits
		temp_item = new cMenu_Item();
		temp_item->image_default->Set_Image( credits );
		temp_item->Set_Pos( static_cast<float>(game_res_w) * 0.45f, static_cast<float>(game_res_h) - 30 );
		pMenuCore->handler->Add_Menu_Item( temp_item, 1.5f, grey );

		drawlist.push_back( new cHudSprite( credits, -200, 0, 1 ) );
		// SDL logo
		drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/logo_sdl.png" ), static_cast<float>(game_res_w) * 0.04f, static_cast<float>(game_res_h) * 0.935f ) );
	}

	Init_GUI();
}

void cMenu_Main :: Init_GUI( void )
{
	cMenu_Base::Init_GUI();

	CEGUI::Window *text_version = CEGUI::WindowManager::getSingleton().getWindow( "text_version" );
	text_version->setProperty( "Text", UTF8_("Version ") + static_cast<CEGUI::String>(string_trim_from_end( float_to_string( smc_version, 6 ), '0' )) );

	// if in a level/world
	if( exit_to_gamemode != MODE_NOTHING )
	{
		CEGUI::Window *text_website = CEGUI::WindowManager::getSingleton().getWindow( "text_website" );
		text_website->hide();
	}
}

void cMenu_Main :: Update( void )
{
	cMenu_Base::Update();

	if( !action )
	{
		return;
	}

	action = 0;

	// Start
	if( pMenuCore->handler->active == 0 )
	{
		pMenuCore->next_menu = MENU_START;
		Game_Action = GA_ENTER_MENU;
	}
	// Options
	else if( pMenuCore->handler->active == 1 )
	{
		pMenuCore->next_menu = MENU_OPTIONS;
		Game_Action = GA_ENTER_MENU;
	}
	// Load
	else if( pMenuCore->handler->active == 2 )
	{
		pMenuCore->next_menu = MENU_LOAD;
		Game_Action = GA_ENTER_MENU;
	}
	// Save
	else if( pMenuCore->handler->active == 3 )
	{
		pMenuCore->next_menu = MENU_SAVE;
		Game_Action = GA_ENTER_MENU;
	}
	// Quit
	else if( pMenuCore->handler->active == 4 )
	{
		game_exit = 1;
	}
	// Credits
	else if( pMenuCore->handler->active == 5 )
	{
		pMenuCore->next_menu = MENU_CREDITS;
		Game_Action = GA_ENTER_MENU;
	}
}

void cMenu_Main :: Draw( void )
{
	cMenu_Base::Draw();
	Draw_End();
}

void cMenu_Main :: Exit( void )
{
	if( exit_to_gamemode == MODE_LEVEL )
	{
		Game_Action = GA_ENTER_LEVEL;
	}
	else if( exit_to_gamemode == MODE_OVERWORLD )
	{
		Game_Action = GA_ENTER_WORLD;
	}
}

/* *** *** *** *** *** *** *** *** cMenu_Start *** *** *** *** *** *** *** *** *** */

cMenu_Start :: cMenu_Start( void )
: cMenu_Base()
{

}

cMenu_Start :: ~cMenu_Start( void )
{

}

void cMenu_Start :: Init( void )
{
	listbox_search_buffer_counter = 0;

	cMenu_Base::Init();

	layout_file = "menu/start.layout";

	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/start.png" ), static_cast<float>(game_res_w) * 0.02f, 140 ) );
	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/overworld.png" ), static_cast<float>(game_res_w) / 20, 210 ) );

	Init_GUI();
}

void cMenu_Start :: Init_GUI( void )
{
	cMenu_Base::Init_GUI();

	// Tab Control
	CEGUI::TabControl *tabcontrol = static_cast<CEGUI::TabControl *>(CEGUI::WindowManager::getSingleton().getWindow( "tabcontrol_main" ));
	tabcontrol->activate();

	// events
	tabcontrol->subscribeEvent( CEGUI::TabControl::EventSelectionChanged, CEGUI::Event::Subscriber( &cMenu_Start::TabControl_Selection_Changed, this ) );
	tabcontrol->subscribeEvent( CEGUI::Window::EventKeyDown, CEGUI::Event::Subscriber( &cMenu_Start::TabControl_Keydown, this ) );

	// Worlds
	CEGUI::Listbox *listbox_worlds = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_worlds" ));
	
	// events
	listbox_worlds->subscribeEvent( CEGUI::Window::EventKeyDown, CEGUI::Event::Subscriber( &cMenu_Start::Listbox_Keydown, this ) );
	listbox_worlds->subscribeEvent( CEGUI::Window::EventCharacterKey, CEGUI::Event::Subscriber( &cMenu_Start::Listbox_Character_Key, this ) );

	// overworld names
	for( vector<cOverworld *>::iterator itr = pOverworld_Manager->objects.begin(), itr_end = pOverworld_Manager->objects.end(); itr != itr_end; ++itr )
	{
		cOverworld_description *world = (*itr)->m_description;

// show all worlds in debug builds
#ifndef _DEBUG
		if( !world->visible )
		{
			continue;
		}
#endif
		
		CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem( reinterpret_cast<const CEGUI::utf8*>(world->name.c_str()), 0 );
		// game world
		if( !world->user )
		{
			item->setTextColours( CEGUI::colour( 1, 0.8f, 0.6f ) );
		}
		// user world
		else
		{
			item->setTextColours( CEGUI::colour( 0.8f, 1, 0.6f ) );
		}

		item->setSelectionColours( CEGUI::colour( 0.33f, 0.33f, 0.33f ) );
		item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
		listbox_worlds->addItem( item );
	}

	// Level Listbox
	CEGUI::Listbox *listbox_levels = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_levels" ));
	listbox_levels->setSortingEnabled( 1 );

	//CEGUI::Listbox *listbox_weapons = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_weapons" ));
	//listbox_weapons->setSortingEnabled( 1 );

	// events
	listbox_levels->subscribeEvent( CEGUI::Window::EventKeyDown, CEGUI::Event::Subscriber( &cMenu_Start::Listbox_Keydown, this ) );
	listbox_levels->subscribeEvent( CEGUI::Window::EventCharacterKey, CEGUI::Event::Subscriber( &cMenu_Start::Listbox_Character_Key, this ) );

	//listbox_weapons->subscribeEvent( CEGUI::Window::EventKeyDown, CEGUI::Event::Subscriber( &cMenu_Start::Listbox_Keydown, this ) );
	//listbox_weapons->subscribeEvent( CEGUI::Window::EventCharacterKey, CEGUI::Event::Subscriber( &cMenu_Start::Listbox_Character_Key, this ) );

	// get game level
	Get_Levels( DATA_DIR "/" GAME_LEVEL_DIR, CEGUI::colour( 1, 0.8f, 0.6f ) );
	// get user level
	Get_Levels( pResource_Manager->user_data_dir + USER_LEVEL_DIR, CEGUI::colour( 0.8f, 1, 0.6f ) );

	//Get_Weapons( DATA_DIR "/" GAME_LEVEL_DIR, CEGUI::colour( 1, 0.8f, 0.6f ) );
	// get user level
	//Get_Weapons( pResource_Manager->user_data_dir + USER_LEVEL_DIR, CEGUI::colour( 0.8f, 1, 0.6f ) );

	// World Listbox events
	listbox_worlds->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &cMenu_Start::World_Select, this ) );
	listbox_worlds->subscribeEvent( CEGUI::Listbox::EventMouseDoubleClick, CEGUI::Event::Subscriber( &cMenu_Start::World_Select_final_list, this ) );
	// Level Listbox events
	listbox_levels->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &cMenu_Start::Level_Select, this ) );
	listbox_levels->subscribeEvent( CEGUI::Listbox::EventMouseDoubleClick, CEGUI::Event::Subscriber( &cMenu_Start::Level_Select_Final_List, this ) );

	//listbox_weapons->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &cMenu_Start::Weapon_Select, this ) );
	//listbox_weapons->subscribeEvent( CEGUI::Listbox::EventMouseDoubleClick, CEGUI::Event::Subscriber( &cMenu_Start::Weapon_Select_Final_List, this ) );
	// Level Buttons
	CEGUI::PushButton *button_new = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_level_new" ));
	button_new->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cMenu_Start::Button_Level_New_Clicked, this ) );
	CEGUI::PushButton *button_edit = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_level_edit" ));
	button_edit->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cMenu_Start::Button_Level_Edit_Clicked, this ) );
	CEGUI::PushButton *button_delete = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_level_delete" ));
	button_delete->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cMenu_Start::Button_Level_Delete_Clicked, this ) );

	// select first Item
	listbox_worlds->setItemSelectState( static_cast<size_t>(0), 1 );

	// Button Enter 
	CEGUI::PushButton *button_enter = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_enter" ));
	// Button back
	CEGUI::PushButton *button_back = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_back" ));

	// events
	button_enter->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cMenu_Start::Button_Enter_Clicked, this ) );
	button_back->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cMenu_Start::Button_Back_Clicked, this ) );

	// Set focus
	listbox_worlds->activate();
}

void cMenu_Start :: Update( void )
{
	// if search buffer is active
	if( listbox_search_buffer_counter > 0.0f )
	{
		listbox_search_buffer_counter -= pFramerate->m_speed_factor;

		// if time limit reached search buffer is abandoned
		if( listbox_search_buffer_counter <= 0.0f )
		{
			listbox_search_buffer_counter = 0.0f;
			listbox_search_buffer.clear();
		}
	}

	cMenu_Base::Update();

	if( !action )
	{
		return;
	}

	action = 0;

	// enter
	Load_Selected();
}

void cMenu_Start :: Draw( void )
{
	cMenu_Base::Draw();
	Draw_End();
}

void cMenu_Start :: Exit( void )
{
	pMenuCore->next_menu = MENU_MAIN;
	Game_Action = GA_ENTER_MENU;
}



void cMenu_Start :: Get_Levels( std::string dir, CEGUI::colour color )
{
	// Level Listbox
	CEGUI::Listbox *listbox_levels = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_levels" ));

	// get directory length for erasing
	int dir_length = dir.length() + 1;
	// get all files
	vector<std::string> lvl_files = Get_Directory_Files( dir, "", 0, 0 );

	// list all available levels
	for( vector<std::string>::iterator itr = lvl_files.begin(), itr_end = lvl_files.end(); itr != itr_end; ++itr )
	{
		// get filename
		std::string lvl_name = (*itr);
		// remove base directory
		lvl_name.erase( 0, dir_length );

		// erase file type only if smclvl
		if( lvl_name.rfind( ".smclvl" ) != std::string::npos )
		{
			lvl_name.erase( lvl_name.rfind( ".smclvl" ) );
		}

		// create listbox item
		CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem( reinterpret_cast<const CEGUI::utf8*>(lvl_name.c_str()), 0 );

		// if smclvl
		if( lvl_name.rfind( ".txt" ) == std::string::npos )
		{
			item->setTextColours( color );
		}
		// grey out old txt levels
		else
		{
			item->setTextColours( CEGUI::colour( 0.6f, 0.6f, 0.6f ) );
		}

		// check if item with the same name already exists
		CEGUI::ListboxTextItem *item_old = static_cast<CEGUI::ListboxTextItem *>(listbox_levels->findItemWithText( lvl_name, NULL ));
		
		if( item_old )
		{
			// mix colors
			item->setTextColours( item->getTextColours().d_top_left, item->getTextColours().d_top_right, item_old->getTextColours().d_bottom_left, item_old->getTextColours().d_bottom_right );
			// remove old item
			listbox_levels->removeItem( item_old );
		}


		item->setSelectionColours( CEGUI::colour( 0.33f, 0.33f, 0.33f ) );
		item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
		listbox_levels->addItem( item );
	}
}

void cMenu_Start :: Load_Selected( void )
{
	// Get Tab Control
	CEGUI::TabControl *tabcontrol = static_cast<CEGUI::TabControl *>(CEGUI::WindowManager::getSingleton().getWindow( "tabcontrol_main" ));

	// World
	if( tabcontrol->getSelectedTabIndex() == 0 )
	{
		CEGUI::ListboxItem *item = (static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_worlds" )))->getFirstSelectedItem();

		// load world
		if( item )
		{
			Load_World( item->getText().c_str() );
		}
	}
	// Level
	else
	{
		CEGUI::ListboxItem *item = (static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_levels" )))->getFirstSelectedItem();

		// load level
		if( item )
		{
			Load_Level( item->getText().c_str() );
		}
	}
	/*else
	{
		CEGUI::ListboxItem *item = (static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_weapons" )))->getFirstSelectedItem();

		// load level
		if( item )
		{
			Load_Weapon( item->getText().c_str() );
		}
	} */
}

void cMenu_Start :: Load_World( std::string name )
{
	if( pPlayer->points > 0 && !Box_Question( _("This will reset your current progress.\nContinue ?") ) )
	{
		return;
	}

	cOverworld *new_world = pOverworld_Manager->Get_from_Name( name );

	// if not available
	if( !new_world )
	{
		pHud_Debug->Set_Text( _("Couldn't load overworld ") + name, static_cast<float>(speedfactor_fps) );
	}
	// if successfully set active
	else
	{
		// enter world
		Game_Action = GA_ENTER_WORLD;
		Game_Action_Data.add( "world", name.c_str() );
		Game_Action_Data.add( "reset_save", "1" );
	}
}

bool cMenu_Start :: Load_Level( std::string level_name )
{
	// if not available
	if( !pActive_Level->Get_Path( level_name ) )
	{
		pAudio->Play_Sound( "error.ogg" );
		pHud_Debug->Set_Text( _("Couldn't load level ") + level_name, static_cast<float>(speedfactor_fps) );
		return 0;
	}

	// enter level
	Game_Action = GA_ENTER_LEVEL;
	Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM;
	Game_Action_Data.add( "level", level_name.c_str() );

	return 1;
}

/*
bool cMenu_Start :: Load_Weapon( std::string weapon_name)
{

	return 1;	
} */

bool cMenu_Start :: TabControl_Selection_Changed( const CEGUI::EventArgs &e )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( e );
	CEGUI::TabControl *tabcontrol = static_cast<CEGUI::TabControl *>( windowEventArgs.window );

	if( tabcontrol->getSelectedTabIndex() == 0 )
	{
		static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_worlds" ))->activate();
	}
	else if( tabcontrol->getSelectedTabIndex() == 1 )
	{
		static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_levels" ))->activate();
	}
	//else if( tabcontrol->getSelectedTabIndex() == 2)
	//{
	//	static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_weapons" ))->activate();
	//}

	return 1;
}

bool cMenu_Start :: TabControl_Keydown( const CEGUI::EventArgs &e )
{
	const CEGUI::KeyEventArgs &ke = static_cast<const CEGUI::KeyEventArgs &>(e);

	// Return
	if( ke.scancode == CEGUI::Key::Return || ke.scancode == CEGUI::Key::NumpadEnter )
	{
		Load_Selected();

		return 1;
	}
	// Shift Tab
	else if( ( pKeyboard->keys[SDLK_LSHIFT] || pKeyboard->keys[SDLK_RSHIFT] ) && ke.scancode == CEGUI::Key::Tab )
	{
		// Get Tab Control
		CEGUI::TabControl *tabcontrol = static_cast<CEGUI::TabControl *>(CEGUI::WindowManager::getSingleton().getWindow( "tabcontrol_main" ));

		// if last tab
		if( tabcontrol->getSelectedTabIndex() + 1 == tabcontrol->getTabCount() )
		{
			tabcontrol->setSelectedTabAtIndex( 0 );
		}
		// select next tab
		else
		{
			tabcontrol->setSelectedTabAtIndex( tabcontrol->getSelectedTabIndex() + 1 );
		}

		return 1;
	}

	return 0;
}

bool cMenu_Start :: Listbox_Keydown( const CEGUI::EventArgs &e )
{
	const CEGUI::KeyEventArgs &ke = static_cast<const CEGUI::KeyEventArgs &>(e);

	// Get the Listbox
	CEGUI::Listbox *listbox = static_cast<CEGUI::Listbox *>(ke.window);

	// Down/Up
	if( ke.scancode == CEGUI::Key::ArrowDown || ke.scancode == CEGUI::Key::ArrowUp || ke.scancode == CEGUI::Key::PageDown || ke.scancode == CEGUI::Key::PageUp ||
			 ke.scancode == CEGUI::Key::Home || ke.scancode == CEGUI::Key::End )
	{
		int new_selected = 0;
		int last_selected = 0;

		// get selected item
		CEGUI::ListboxItem *last_selected_item = listbox->getFirstSelectedItem();

		// if something is selected
		if( last_selected_item != NULL )
		{
			last_selected = listbox->getItemIndex( last_selected_item );
		}

		// down
		if( ke.scancode == CEGUI::Key::ArrowDown )
		{
			new_selected = last_selected + 1;
		}
		// up
		else if( ke.scancode == CEGUI::Key::ArrowUp )
		{
			new_selected = last_selected - 1;
		}
		// page down
		else if( ke.scancode == CEGUI::Key::PageDown )
		{
			// todo : should skip all visible items
			new_selected = last_selected + 10;
		}
		// page up
		else if( ke.scancode == CEGUI::Key::PageUp )
		{
			// todo : should skip all visible items
			new_selected = last_selected - 10;
		}
		// home
		else if( ke.scancode == CEGUI::Key::Home )
		{
			new_selected = 0;
		}
		// end
		else if( ke.scancode == CEGUI::Key::End )
		{
			new_selected = listbox->getItemCount() - 1;
		}

		// if after last item
		if( new_selected >= static_cast<int>(listbox->getItemCount()) )
		{
			// select first
			if( last_selected == static_cast<int>(listbox->getItemCount()) - 1 )
			{
				new_selected = 0;
			}
			// select last
			else
			{
				new_selected = listbox->getItemCount() - 1;
			}
		}
		// if before first item
		else if( new_selected < 0 )
		{
			// select last
			if( last_selected == 0 )
			{
				new_selected = listbox->getItemCount() - 1;
			}
			// select first
			else
			{
				new_selected = 0;
			}
		}

		listbox->setItemSelectState( new_selected, 1 );
		listbox->ensureItemIsVisible( new_selected );

		return 1;
	}

	return 0;
}

bool cMenu_Start :: Listbox_Character_Key( const CEGUI::EventArgs &e )
{
	const CEGUI::KeyEventArgs &ke = static_cast<const CEGUI::KeyEventArgs &>(e);

	// Get the Listbox
	CEGUI::Listbox *listbox = static_cast<CEGUI::Listbox *>(ke.window);

	if( listbox->getFont()->isCodepointAvailable( ke.codepoint ) )
	{
		listbox_search_buffer_counter = speedfactor_fps;
		listbox_search_buffer.insert( listbox_search_buffer.end(), 1, ke.codepoint );

		// new selected if found
		CEGUI::ListboxItem *new_selected = NULL;

		// search the list
		size_t index = 0;

		while( index < listbox->getItemCount() )
		{
			CEGUI::ListboxItem *item = listbox->getListboxItemFromIndex( index );

			// found
			if( item->getText().substr( 0, listbox_search_buffer.length() ).compare( listbox_search_buffer ) == 0 )
			{
				new_selected = item;
				break;
			}
			// no match
			else
			{
				index++;
			}
		}

		// set new item selected
		if( new_selected )
		{
			listbox->setItemSelectState( new_selected, 1 );
			listbox->ensureItemIsVisible( new_selected );
		}
	}

	return 0;
}

bool cMenu_Start :: World_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// World Comment
	CEGUI::Editbox *editbox_world_comment = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_world_comment" ));

	// set world comment
	if( item )
	{
		editbox_world_comment->setText( reinterpret_cast<const CEGUI::utf8*>(pOverworld_Manager->Get_from_Name( item->getText().c_str() )->m_description->comment.c_str()) );
	}
	// clear
	else
	{
		editbox_world_comment->setText( "" );
	}

	return 1;
}

bool cMenu_Start :: World_Select_final_list( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// load world
	if( item )
	{
		Load_World( item->getText().c_str() );
	}

	return 1;
}

bool cMenu_Start :: Level_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// set level something
	if( item )
	{
		// todo : needs level manager
	}
	// clear
	else
	{
		// todo : needs level manager
	}

	return 1;
}

/*
bool cMenu_Start :: Weapon_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// set level something
	if( item )
	{
		// todo : needs level manager
	}
	// clear
	else
	{
		// todo : needs level manager
	}

	return 1;
} 

*/

bool cMenu_Start :: Level_Select_Final_List( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// load level
	if( item )
	{
		Load_Level( item->getText().c_str() );
	}

	return 1;
}

/*
bool cMenu_Start :: Weapon_Select_Final_List( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// load weapon
	if( item )
	{
		Load_Weapon( item->getText().c_str() );
	}

	return 1;
} 
*/

bool cMenu_Start :: Button_Level_New_Clicked( const CEGUI::EventArgs &event )
{
	// Create level
	if( !pLevel_Editor->Function_New() )
	{
		// aborted/failed
		return 0;
	}

	// Enter level
	Game_Action = GA_ENTER_LEVEL;
	Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM_EDITOR;

	return 1;
}

bool cMenu_Start :: Button_Level_Edit_Clicked( const CEGUI::EventArgs &event )
{
	// Get Selected Level
	CEGUI::Listbox *listbox_levels = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_levels" ));
	CEGUI::ListboxItem *item = listbox_levels->getFirstSelectedItem();

	// load level
	if( item && Load_Level( item->getText().c_str() ) )
	{
		Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM_EDITOR;
	}

	return 1;
}

bool cMenu_Start :: Button_Level_Delete_Clicked( const CEGUI::EventArgs &event )
{
	// Get Selected Level
	CEGUI::Listbox *listbox_levels = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_levels" ));
	CEGUI::ListboxItem *item = listbox_levels->getFirstSelectedItem();

	// load level
	if( item )
	{
		std::string filename = item->getText().c_str();

		// if denied
		if( !Box_Question( _("Delete ") + filename + " ?" ) )
		{
			return 1;
		}

		// only user directory
		if( pActive_Level->Get_Path( filename, 1 ) )
		{
			Delete_File( filename );
			listbox_levels->removeItem( item );
		}
	}

	return 1;
}

bool cMenu_Start :: Button_Enter_Clicked( const CEGUI::EventArgs &event )
{
	Load_Selected();
	return 1;
}

bool cMenu_Start :: Button_Back_Clicked( const CEGUI::EventArgs &event )
{
	Exit();
	return 1;
}

/* *** *** *** *** *** *** *** *** cMenu_Options *** *** *** *** *** *** *** *** *** */

cMenu_Options :: cMenu_Options( void )
: cMenu_Base()
{

}

cMenu_Options :: ~cMenu_Options( void )
{

}

void cMenu_Options :: Init( void )
{
	cMenu_Base::Init();

	cMenu_Item *temp_item = NULL;
	menu_posy = 100;

	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/options.png" ), game_res_w * 0.01f, menu_posy, 0 ) );

	// Game
	menu_posy += 60;
	temp_item = new cMenu_Item();
	temp_item = pMenuCore->Auto_Menu( "game.png", "", menu_posy );
	temp_item->Set_Pos_X( game_res_w * 0.07f, 1 );
	pMenuCore->handler->Add_Menu_Item( temp_item );
	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/game.png" ), game_res_w * 0.01f, menu_posy ) );
	// Video
	menu_posy += 60;
	temp_item = new cMenu_Item();
	temp_item = pMenuCore->Auto_Menu( "video.png", "", menu_posy );
	temp_item->Set_Pos_X( game_res_w * 0.07f, 1 );
	pMenuCore->handler->Add_Menu_Item( temp_item );
	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/video.png" ), game_res_w * 0.01f, menu_posy ) );
	// Audio
	menu_posy += 60;
	temp_item = new cMenu_Item();
	temp_item = pMenuCore->Auto_Menu( "audio.png", "", menu_posy );
	temp_item->Set_Pos_X( game_res_w * 0.07f, 1 );
	pMenuCore->handler->Add_Menu_Item( temp_item );
	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/audio.png" ), game_res_w * 0.01f, menu_posy ) );
	// Controls
	menu_posy += 60;
	temp_item = new cMenu_Item();
	temp_item = pMenuCore->Auto_Menu( "controls.png", "", menu_posy );
	temp_item->Set_Pos_X( game_res_w * 0.07f, 1 );
	pMenuCore->handler->Add_Menu_Item( temp_item );
	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/controls.png" ), game_res_w * 0.01f, menu_posy ) );
	// back
	menu_posy += 60;
	cGL_Surface *back1 = pFont->Render_Text( pFont->m_font_normal, _("Back"), text_color );
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( back1 );
	temp_item->Set_Pos( game_res_w * 0.10f, menu_posy, 1 );
	temp_item->isquit = 1;
	pMenuCore->handler->Add_Menu_Item( temp_item, 1.5f, grey );
	drawlist.push_back( new cHudSprite( back1, -200, 0, 1 ) );
}

void cMenu_Options :: Init_GUI( void )
{
	cMenu_Base::Init_GUI();
}

void cMenu_Options :: Update( void )
{
	cMenu_Base::Update();

	if( !action )
	{
		return;
	}

	// only menu actions
	if( pMenuCore->handler->active > 4 )
	{
		return;
	}

	action = 0;

	// Game
	if( pMenuCore->handler->active == MENU_GAME )
	{
		pMenuCore->next_menu = MENU_OPTIONS;
		pMenuCore->options_menu_id	= MENU_GAME;
		Game_Action = GA_ENTER_MENU;
	}
	 // Video
	else if( pMenuCore->handler->active == MENU_VIDEO )
	{
		pMenuCore->next_menu = MENU_OPTIONS;
		pMenuCore->options_menu_id	= MENU_VIDEO;
		Game_Action = GA_ENTER_MENU;
	}
	// Audio
	else if( pMenuCore->handler->active == MENU_AUDIO )
	{
		pMenuCore->next_menu = MENU_OPTIONS;
		pMenuCore->options_menu_id	= MENU_AUDIO;
		Game_Action = GA_ENTER_MENU;
	}
	// Controls
	else if( pMenuCore->handler->active == MENU_CONTROLS )
	{
		pMenuCore->next_menu = MENU_OPTIONS;
		pMenuCore->options_menu_id = MENU_CONTROLS;
		Game_Action = GA_ENTER_MENU;
	}
	// back
	else if( pMenuCore->handler->active == 4 )
	{
		Exit();
	}
}

void cMenu_Options :: Draw( void )
{
	cMenu_Base::Draw();
}

void cMenu_Options :: Exit( void )
{
	pMenuCore->next_menu = MENU_MAIN;
	pPreferences->Save();
	Game_Action = GA_ENTER_MENU;
}

/* *** *** *** *** *** *** *** *** cMenu_Options_Game *** *** *** *** *** *** *** *** *** */

cMenu_Options_Game :: cMenu_Options_Game( void )
: cMenu_Options()
{

}

cMenu_Options_Game :: ~cMenu_Options_Game( void )
{

}

void cMenu_Options_Game :: Init( void )
{
	cMenu_Options::Init();

	layout_file = "menu/options_game.layout";

	pMenuCore->handler->Set_Active( MENU_GAME );

	Init_GUI();
}

void cMenu_Options_Game :: Init_GUI( void )
{
	cMenu_Options::Init_GUI();

	// get the CEGUI window manager
	CEGUI::WindowManager &windowmanager = CEGUI::WindowManager::getSingleton();

	// always run
	CEGUI::Window *text_always_run = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_always_run" ));
	text_always_run->setText( UTF8_("Always Run") );

	combo_always_run = static_cast<CEGUI::Combobox *>(windowmanager.getWindow( "combo_always_run" ));

	CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem( UTF8_("On"), 0 );
	item->setTextColours( CEGUI::colour( 0, 1, 0 ) );
	combo_always_run->addItem( item );
	item = new CEGUI::ListboxTextItem( UTF8_("Off"), 1 );
	item->setTextColours( CEGUI::colour( 0, 0, 1 ) );
	combo_always_run->addItem( item );

	if( pPreferences->m_always_run )
	{
		combo_always_run->setText( UTF8_("On") );
	}
	else
	{
		combo_always_run->setText( UTF8_("Off") );
	}

	combo_always_run->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Game::Always_Run_Select, this ) );

	// Camera Horizontal Speed
	CEGUI::Window *text_camera_hor_speed = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_camera_hor_speed" ));
	text_camera_hor_speed->setText( UTF8_("Camera Hor Speed") );

	spinner_camera_hor_speed = static_cast<CEGUI::Spinner *>(windowmanager.getWindow( "spinner_camera_hor_speed" ));
	spinner_camera_hor_speed->setCurrentValue( pLevel_Manager->camera->hor_offset_speed );

	spinner_camera_hor_speed->subscribeEvent( CEGUI::Spinner::EventValueChanged, CEGUI::Event::Subscriber( &cMenu_Options_Game::Camera_Hor_Select, this ) );

	// Camera Vertical Speed
	CEGUI::Window *text_camera_ver_speed = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_camera_ver_speed" ));
	text_camera_ver_speed->setText( UTF8_("Camera Ver Speed") );

	spinner_camera_ver_speed = static_cast<CEGUI::Spinner *>(windowmanager.getWindow( "spinner_camera_ver_speed" ));
	spinner_camera_ver_speed->setCurrentValue( pLevel_Manager->camera->ver_offset_speed );

	spinner_camera_ver_speed->subscribeEvent( CEGUI::Spinner::EventValueChanged, CEGUI::Event::Subscriber( &cMenu_Options_Game::Camera_Ver_Select, this ) );

	// language
	CEGUI::Window *text_language = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_language" ));
	text_language->setText( UTF8_("Language") );

	combo_language = static_cast<CEGUI::Combobox *>(windowmanager.getWindow( "combo_language" ));

	item = new CEGUI::ListboxTextItem( UTF8_("default"), 0 );
	item->setTextColours( CEGUI::colour( 0, 1, 0 ) );
	combo_language->addItem( item );

	// get available languages
	vector<std::string> language_files = Get_Directory_Files( DATA_DIR "/" GAME_TRANSLATION_DIR, ".none", 1, 0 );
	// add english as it is the base language and not in the translation directory
	language_files.push_back( DATA_DIR "/" GAME_TRANSLATION_DIR "/" "en" );

	for( vector<std::string>::iterator itr = language_files.begin(), itr_end = language_files.end(); itr != itr_end; ++itr )
	{
		// get filename
		std::string filename = (*itr);

		// if not directory
		if( filename.rfind( "." ) != std::string::npos )
		{
			continue;
		}

		// remove data dir
		filename.erase( 0, strlen( DATA_DIR "/" GAME_TRANSLATION_DIR "/" ) );

		item = new CEGUI::ListboxTextItem( filename, 1 );
		item->setTextColours( CEGUI::colour( 0, 0, 1 ) );
		combo_language->addItem( item );
	}

	if( pPreferences->m_language.empty() )
	{
		combo_language->setText( UTF8_("default") );
	}
	else
	{
		combo_language->setText( pPreferences->m_language );
	}

	combo_language->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Game::Language_Select, this ) );

	// menu level
	CEGUI::Window *text_menu_level = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_menu_level" ));
	text_menu_level->setText( UTF8_("Menu Level") );

	combo_menu_level = static_cast<CEGUI::Combobox *>(windowmanager.getWindow( "combo_menu_level" ));

	item = new CEGUI::ListboxTextItem( "menu_green_1", 0 );
	combo_menu_level->addItem( item );
	item = new CEGUI::ListboxTextItem( "menu_blue_1", 0 );
	combo_menu_level->addItem( item );

	combo_menu_level->setText( pPreferences->m_menu_level );

	combo_menu_level->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Game::Menu_Level_Select, this ) );
	combo_menu_level->getEditbox()->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cMenu_Options_Game::Menu_Level_Text_Changed, this ) );

	// Reset Game
	CEGUI::PushButton *button_reset_game = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_reset_game" ));
	button_reset_game->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cMenu_Options_Game::Button_Reset_Game_Clicked, this ) );
	button_reset_game->setText( UTF8_("Reset") );

	// Editor tab
	// show item images
	CEGUI::Window *text_editor_show_item_images = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_editor_show_item_images" ));
	text_editor_show_item_images->setText( UTF8_("Show images") );

	combo_editor_show_item_images = static_cast<CEGUI::Combobox *>(windowmanager.getWindow( "combo_editor_show_item_images" ));

	item = new CEGUI::ListboxTextItem( UTF8_("On"), 0 );
	item->setTextColours( CEGUI::colour( 0, 1, 0 ) );
	combo_editor_show_item_images->addItem( item );
	item = new CEGUI::ListboxTextItem( UTF8_("Off"), 1 );
	item->setTextColours( CEGUI::colour( 0, 0, 1 ) );
	combo_editor_show_item_images->addItem( item );

	if( pPreferences->m_editor_show_item_images )
	{
		combo_editor_show_item_images->setText( UTF8_("On") );
	}
	else
	{
		combo_editor_show_item_images->setText( UTF8_("Off") );
	}

	combo_editor_show_item_images->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Game::Editor_Show_Item_Images_Select, this ) );

	// item image size
	CEGUI::Window *text_editor_item_image_size = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_editor_item_image_size" ));
	text_editor_item_image_size->setText( UTF8_("Item image size") );

	spinner_editor_item_image_size = static_cast<CEGUI::Spinner *>(windowmanager.getWindow( "spinner_editor_item_image_size" ));
	spinner_editor_item_image_size->setCurrentValue( static_cast<float>(pPreferences->m_editor_item_image_size) );

	spinner_editor_item_image_size->subscribeEvent( CEGUI::Spinner::EventValueChanged, CEGUI::Event::Subscriber( &cMenu_Options_Game::Editor_Item_Image_Size_Select, this ) );

	// editor mouse auto hide
	CEGUI::Window *text_editor_mouse_auto_hide = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_editor_mouse_auto_hide" ));
	text_editor_mouse_auto_hide->setText( UTF8_("Auto-Hide Mouse") );

	combo_editor_mouse_auto_hide = static_cast<CEGUI::Combobox *>(windowmanager.getWindow( "combo_editor_mouse_auto_hide" ));

	item = new CEGUI::ListboxTextItem( UTF8_("On"), 0 );
	item->setTextColours( CEGUI::colour( 0, 1, 0 ) );
	combo_editor_mouse_auto_hide->addItem( item );
	item = new CEGUI::ListboxTextItem( UTF8_("Off"), 1 );
	item->setTextColours( CEGUI::colour( 0, 0, 1 ) );
	combo_editor_mouse_auto_hide->addItem( item );

	if( pPreferences->m_editor_mouse_auto_hide )
	{
		combo_editor_mouse_auto_hide->setText( UTF8_("On") );
	}
	else
	{
		combo_editor_mouse_auto_hide->setText( UTF8_("Off") );
	}

	combo_editor_mouse_auto_hide->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Game::Editor_Auto_Hide_Mouse_Select, this ) );

	// Reset Editor
	CEGUI::PushButton *button_reset_editor = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_reset_editor" ));
	button_reset_editor->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cMenu_Options_Game::Button_Reset_Editor_Clicked, this ) );
	button_reset_editor->setText( UTF8_("Reset") );
}

void cMenu_Options_Game :: Update( void )
{
	cMenu_Options::Update();

	if( !action )
	{
		return;
	}

	cMenu_Options::Update();

	// only video actions
	if( pMenuCore->handler->active <= 4 )
	{
		return;
	}

	action = 0;

	// always run
	if( pMenuCore->handler->active == 5 )
	{
		pPreferences->m_always_run = !pPreferences->m_always_run;

		if( pPreferences->m_always_run )
		{
			combo_always_run->setText( UTF8_("On") );
		}
		else
		{
			combo_always_run->setText( UTF8_("Off") );
		}
	}
	// Camera Horizontal
	else if( pMenuCore->handler->active == 6 )
	{
		// nothing
	}
	// Camera Vertical
	else if( pMenuCore->handler->active == 7 )
	{
		// nothing
	}
	// language
	else if( pMenuCore->handler->active == 8 )
	{
		unsigned int selected = combo_language->getItemIndex( combo_language->findItemWithText( combo_language->getText(), NULL ) );

		CEGUI::ListboxItem *new_selected = NULL;

		// last item selected
		if( selected == combo_language->getItemCount() - 1 )
		{
			new_selected = combo_language->getListboxItemFromIndex( 0 );
		}
		// select next item
		else
		{
			new_selected = combo_language->getListboxItemFromIndex( selected + 1 );
		}
		
		combo_language->setText( new_selected->getText() );
		combo_language->setItemSelectState( new_selected, 1 );
		Language_Select( CEGUI::WindowEventArgs( combo_language ) );
	}
	// menu level
	else if( pMenuCore->handler->active == 9 )
	{
		unsigned int selected = combo_menu_level->getItemIndex( combo_menu_level->findItemWithText( combo_menu_level->getText(), NULL ) );

		CEGUI::ListboxItem *new_selected = NULL;

		// last item selected
		if( selected == combo_menu_level->getItemCount() - 1 )
		{
			new_selected = combo_menu_level->getListboxItemFromIndex( 0 );
		}
		// select next item
		else
		{
			new_selected = combo_menu_level->getListboxItemFromIndex( selected + 1 );
		}
		
		combo_menu_level->setText( new_selected->getText() );
		combo_menu_level->setItemSelectState( new_selected, 1 );
		Menu_Level_Select( CEGUI::WindowEventArgs( combo_menu_level ) );
	}
	// editor show item images
	else if( pMenuCore->handler->active == 10 )
	{
		pPreferences->m_editor_show_item_images = !pPreferences->m_editor_show_item_images;

		if( pPreferences->m_editor_show_item_images )
		{
			combo_editor_show_item_images->setText( UTF8_("On") );
		}
		else
		{
			combo_editor_show_item_images->setText( UTF8_("Off") );
		}
	}
	// editor item image size
	else if( pMenuCore->handler->active == 11 )
	{
		// nothing
	}
	// editor auto mouse hide
	else if( pMenuCore->handler->active == 12 )
	{
		pPreferences->m_editor_mouse_auto_hide = !pPreferences->m_editor_mouse_auto_hide;

		if( pPreferences->m_editor_mouse_auto_hide )
		{
			combo_editor_show_item_images->setText( UTF8_("On") );
		}
		else
		{
			combo_editor_show_item_images->setText( UTF8_("Off") );
		}
	}
}

void cMenu_Options_Game :: Draw( void )
{
	cMenu_Options::Draw();
	Draw_End();
}

bool cMenu_Options_Game :: Always_Run_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox*>( windowEventArgs.window )->getSelectedItem();

	bool always_run = 0;

	if( item->getText().compare( UTF8_("On") ) == 0 )
	{
		always_run = 1;
	}

	pPreferences->m_always_run = always_run;

	return 1;
}

bool cMenu_Options_Game :: Camera_Hor_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::Spinner *spinner_camera_hor = static_cast<CEGUI::Spinner *>( windowEventArgs.window );
	
	pLevel_Manager->camera->hor_offset_speed = spinner_camera_hor->getCurrentValue();
	pPreferences->m_camera_hor_speed = spinner_camera_hor->getCurrentValue();

	return 1;
}

bool cMenu_Options_Game :: Camera_Ver_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::Spinner *spinner_camera_ver = static_cast<CEGUI::Spinner *>( windowEventArgs.window );
	
	pLevel_Manager->camera->ver_offset_speed = spinner_camera_ver->getCurrentValue();
	pPreferences->m_camera_ver_speed = spinner_camera_ver->getCurrentValue();

	return 1;
}

bool cMenu_Options_Game :: Language_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox*>( windowEventArgs.window )->getSelectedItem();

	// default
	if( item->getText().compare( UTF8_("default") ) == 0 )
	{
		pPreferences->m_language = "";
	}
	// force
	else
	{
		pPreferences->m_language = item->getText().c_str();
	}

	return 1;
}

bool cMenu_Options_Game :: Menu_Level_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox*>( windowEventArgs.window )->getSelectedItem();

	pPreferences->m_menu_level = item->getText().c_str();

	return 1;
}

bool cMenu_Options_Game :: Menu_Level_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	pPreferences->m_menu_level = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	return 1;
}


bool cMenu_Options_Game :: Editor_Show_Item_Images_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox*>( windowEventArgs.window )->getSelectedItem();

	bool show_item_images = 0;

	if( item->getText().compare( UTF8_("On") ) == 0 )
	{
		show_item_images = 1;
	}

	pPreferences->m_editor_show_item_images = show_item_images;

	return 1;
}

bool cMenu_Options_Game :: Editor_Item_Image_Size_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::Spinner *spinner_item_image_size = static_cast<CEGUI::Spinner *>( windowEventArgs.window );

	pPreferences->m_editor_item_image_size = static_cast<unsigned int>(spinner_item_image_size->getCurrentValue());

	return 1;
}

bool cMenu_Options_Game :: Editor_Auto_Hide_Mouse_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox*>( windowEventArgs.window )->getSelectedItem();

	bool auto_hide_mouse = 0;

	if( item->getText().compare( UTF8_("On") ) == 0 )
	{
		auto_hide_mouse = 1;
	}

	pPreferences->m_editor_mouse_auto_hide = auto_hide_mouse;

	return 1;
}

bool cMenu_Options_Game :: Button_Reset_Game_Clicked( const CEGUI::EventArgs &event )
{
	pPreferences->Reset_Game();

	// clear
	pMenuCore->next_menu = MENU_OPTIONS;
	Game_Action = GA_ENTER_MENU;

	return 1;
}

bool cMenu_Options_Game :: Button_Reset_Editor_Clicked( const CEGUI::EventArgs &event )
{
	pPreferences->Reset_Editor();

	// clear
	pMenuCore->next_menu = MENU_OPTIONS;
	Game_Action = GA_ENTER_MENU;

	return 1;
}

/* *** *** *** *** *** *** *** *** cMenu_Options_Video *** *** *** *** *** *** *** *** *** */

cMenu_Options_Video :: cMenu_Options_Video( void )
: cMenu_Options()
{

}

cMenu_Options_Video :: ~cMenu_Options_Video( void )
{

}

void cMenu_Options_Video :: Init( void )
{
	cMenu_Options::Init();

	layout_file = "menu/options_video.layout";


	// Video Info
	vid_w = pPreferences->m_video_screen_w;
	vid_h = pPreferences->m_video_screen_h;
	vid_bpp = pPreferences->m_video_screen_bpp;
	vid_fullscreen = pPreferences->m_video_fullscreen;
	vid_vsync = pPreferences->m_video_vsync;
	vid_geometry_detail = pVideo->m_geometry_quality;
	vid_texture_detail = pVideo->m_texture_quality;

	pMenuCore->handler->Set_Active( MENU_VIDEO );

	Init_GUI();
}

void cMenu_Options_Video :: Init_GUI( void )
{
	cMenu_Options::Init_GUI();

	// Resolution
	CEGUI::Window *text_resolution = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_resolution" ));
	text_resolution->setText( UTF8_("Resolution") );

	combo_resolution = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_resolution" ));
	
	vector<cSize_Int> valid_resolutions = pVideo->Get_Supported_Resolutions();
	CEGUI::ListboxTextItem *item;

	// add to listbox
	for( vector<cSize_Int>::iterator itr = valid_resolutions.begin(), itr_end = valid_resolutions.end(); itr != itr_end; ++itr )
	{
		// get resolution
		cSize_Int res = (*itr);

		if( res.m_width <= 0 || res.m_height <= 0 )
		{
			continue;
		}

		// calculate aspect ratio
		float ar = static_cast<float>(res.m_width) / static_cast<float>(res.m_height);

		item = new CEGUI::ListboxTextItem( int_to_string( res.m_width ) + "x" + int_to_string( res.m_height ), 3 );
		CEGUI::colour color( 0, 0, 0 );
		// if a badly stretched resolution, display it in red
		if( ar < 1.1f || ar > 1.5f )
		{
			color.setGreen( 0 );
			color.setRed( 1 );
		}
		// good resolution
		else
		{
			// calculate difference from a default 1.333 resolution
			float diff_from_default;

			if( ar > 1.333f )
			{
				diff_from_default = ( ar - 1.333f ) * 4;
			}
			else
			{
				diff_from_default = -( ar - 1.333f ) * 4;
			}

			color.setGreen( 1 - diff_from_default );
			color.setRed( diff_from_default );
		}
		item->setTextColours( color );
		combo_resolution->addItem( item );
	}

	std::string temp = int_to_string( pPreferences->m_video_screen_w ) + "x" + int_to_string( pPreferences->m_video_screen_h );
	combo_resolution->setText( temp.c_str() );

	combo_resolution->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Video::Res_Select, this ) );

	// Bpp
	CEGUI::Window *text_bpp = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_bpp" ));
	text_bpp->setText( UTF8_("Bpp") );

	combo_bpp = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_bpp" ));

	item = new CEGUI::ListboxTextItem( "16", 0 );
	item->setTextColours( CEGUI::colour( 1, 0.6f, 0.3f ) );
	combo_bpp->addItem( item );
	item = new CEGUI::ListboxTextItem( "32", 1 );
	item->setTextColours( CEGUI::colour( 0, 1, 0 ) );
	combo_bpp->addItem( item );

	combo_bpp->setText( int_to_string( pPreferences->m_video_screen_bpp ) );

	combo_bpp->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Video::Bpp_Select, this ) );

	// Fullscreen
	CEGUI::Window *text_fullscreen = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_fullscreen" ));
	text_fullscreen->setText( UTF8_("Fullscreen") );

	combo_fullscreen = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_fullscreen" ));

	item = new CEGUI::ListboxTextItem( UTF8_("On"), 0 );
	item->setTextColours( CEGUI::colour( 0, 1, 0 ) );
	combo_fullscreen->addItem( item );
	item = new CEGUI::ListboxTextItem( UTF8_("Off"), 1 );
	item->setTextColours( CEGUI::colour( 0, 0, 1 ) );
	combo_fullscreen->addItem( item );

	if( pPreferences->m_video_fullscreen )
	{
		combo_fullscreen->setText( UTF8_("On") );
	}
	else
	{
		combo_fullscreen->setText( UTF8_("Off") );
	}

	combo_fullscreen->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Video::Fullscreen_Select, this ) );

	// VSync
	CEGUI::Window *text_vsync = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_vsync" ));
	text_vsync->setText( UTF8_("VSync") );

	combo_vsync = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_vsync" ));

	item = new CEGUI::ListboxTextItem( UTF8_("On"), 0 );
	item->setTextColours( CEGUI::colour( 0, 1, 0 ) );
	combo_vsync->addItem( item );
	item = new CEGUI::ListboxTextItem( UTF8_("Off"), 1 );
	item->setTextColours( CEGUI::colour( 0, 0, 1 ) );
	combo_vsync->addItem( item );

	if( pPreferences->m_video_vsync )
	{
		combo_vsync->setText( UTF8_("On") );
	}
	else
	{
		combo_vsync->setText( UTF8_("Off") );
	}

	combo_vsync->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Video::Vsync_Select, this ) );

	// Geometry quality
	CEGUI::Window *text_geometry_quality = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_geometry_quality" ));
	text_geometry_quality->setText( UTF8_("Geometry Quality") );

	slider_geometry_quality = static_cast<CEGUI::Slider *>(CEGUI::WindowManager::getSingleton().getWindow( "slider_geometry_quality" ));
	slider_geometry_quality->setCurrentValue( pVideo->m_geometry_quality );
	slider_geometry_quality->subscribeEvent( CEGUI::Slider::EventValueChanged, CEGUI::Event::Subscriber( &cMenu_Options_Video::Slider_Geometry_Quality_Changed, this ) );

	// Texture quality
	CEGUI::Window *text_texture_quality = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_texture_quality" ));
	text_texture_quality->setText( UTF8_("Texture Quality") );

	slider_texture_quality = static_cast<CEGUI::Slider *>(CEGUI::WindowManager::getSingleton().getWindow( "slider_texture_quality" ));
	slider_texture_quality->setCurrentValue( pVideo->m_texture_quality );
	slider_texture_quality->subscribeEvent( CEGUI::Slider::EventValueChanged, CEGUI::Event::Subscriber( &cMenu_Options_Video::Slider_Texture_Quality_Changed, this ) );

	// Reset
	CEGUI::PushButton *button_reset = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_reset" ));
	button_reset->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cMenu_Options_Video::Button_Reset_Clicked, this ) );
	button_reset->setText( UTF8_("Reset") );

	// Apply
	CEGUI::PushButton *button_apply = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_apply" ));
	button_apply->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cMenu_Options_Video::Button_Apply_Clicked, this ) );
	button_apply->setText( UTF8_("Apply") );

	// Recreate Cache
	CEGUI::PushButton *button_recreate_cache = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_recreate_cache" ));
	button_recreate_cache->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cMenu_Options_Video::Button_Recreate_Cache_Clicked, this ) );
	button_recreate_cache->setText( UTF8_("Recreate Cache") );
}

void cMenu_Options_Video :: Update( void )
{
	cMenu_Options::Update();

	if( !action )
	{
		return;
	}

	cMenu_Options::Update();

	// only video actions
	if( pMenuCore->handler->active <= 4 )
	{
		return;
	}

	action = 0;

	// Resolution
	if( pMenuCore->handler->active == 5 )
	{
		// Resolution
		combo_resolution = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_resolution" ));
		unsigned int selected = combo_resolution->getItemIndex( combo_resolution->findItemWithText( combo_resolution->getText(), NULL ) );

		CEGUI::ListboxItem *new_selected = NULL;

		// last item selected
		if( selected == combo_resolution->getItemCount() - 1 )
		{
			new_selected = combo_resolution->getListboxItemFromIndex( 0 );
		}
		// select next item
		else
		{
			new_selected = combo_resolution->getListboxItemFromIndex( selected + 1 );
		}
		
		combo_resolution->setText( new_selected->getText() );
		combo_resolution->setItemSelectState( new_selected, 1 );
		Res_Select( CEGUI::WindowEventArgs( combo_resolution ) );
	}
	// BPP
	else if( pMenuCore->handler->active == 6 )
	{
		if( vid_bpp == 16 )
		{
			vid_bpp = 32;
		}
		else if( vid_bpp == 32 )
		{
			vid_bpp = 16;
		}

		combo_bpp->setText( int_to_string( vid_bpp ).c_str() );	
	}
	// Fullscreen
	else if( pMenuCore->handler->active == 7 )
	{
		vid_fullscreen = !vid_fullscreen;

		if( vid_fullscreen )
		{
			combo_fullscreen->setText( UTF8_("On") );	
		}
		else
		{
			combo_fullscreen->setText( UTF8_("Off") );
		}
	}
	// VSync
	else if( pMenuCore->handler->active == 8 )
	{
		vid_vsync = !vid_vsync;

		if( vid_vsync )
		{
			combo_vsync->setText( UTF8_("On") );	
		}
		else
		{
			combo_vsync->setText( UTF8_("Off") );
		}
	}
}

void cMenu_Options_Video :: Draw( void )
{
	cMenu_Options::Draw();

	Draw_End();
}

bool cMenu_Options_Video :: Res_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	std::string temp = item->getText().c_str();

	// get end of height value if text is after resolution string
	std::string::size_type height_end = temp.find( " " );

	if( height_end == std::string::npos )
	{
		height_end = temp.length();
	}

	// get resolution
	unsigned int w = string_to_int( temp.substr( 0, temp.find( "x" ) ) );
	unsigned int h = string_to_int( temp.substr( temp.find( "x" ) + 1, height_end ) );

	// is it supported
	if( !pVideo->Test_Video( w, h, vid_bpp ) )
	{
		return 0;
	}

	// set new selected resolution
	vid_w = w;
	vid_h = h;

	return 1;
}

bool cMenu_Options_Video :: Bpp_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	unsigned int bpp = string_to_int( item->getText().c_str() );

	if( !pVideo->Test_Video( vid_w, vid_h, bpp ) )
	{
		return 0;
	}

	vid_bpp = bpp;

	return 1;
}

bool cMenu_Options_Video :: Fullscreen_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	bool bfullscreen = 0;

	if( item->getText().compare( UTF8_("On") ) == 0 )
	{
		bfullscreen = 1;
	}

	vid_fullscreen = bfullscreen;

	return 1;
}

bool cMenu_Options_Video :: Vsync_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	bool bvsync = 0;

	if( item->getText().compare( UTF8_("On") ) == 0 )
	{
		bvsync = 1;
	}

	vid_vsync = bvsync;

	return 1;
}

bool cMenu_Options_Video :: Slider_Geometry_Quality_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	// set new value
	vid_geometry_detail = static_cast<CEGUI::Slider *>( windowEventArgs.window )->getCurrentValue();

	return 1;
}

bool cMenu_Options_Video :: Slider_Texture_Quality_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	// set new value
	vid_texture_detail = static_cast<CEGUI::Slider *>( windowEventArgs.window )->getCurrentValue();

	return 1;
}

bool cMenu_Options_Video :: Button_Reset_Clicked( const CEGUI::EventArgs &event )
{
	CEGUI::ListboxItem *list_item = combo_resolution->findItemWithText( int_to_string( cPreferences::m_video_screen_w_default ) + "x" + int_to_string( cPreferences::m_video_screen_h_default ), NULL );
	if( list_item )
	{
		combo_resolution->setItemSelectState( list_item, 1 );

		vid_w = cPreferences::m_video_screen_w_default;
		vid_h = cPreferences::m_video_screen_h_default;
	}

	list_item = combo_bpp->findItemWithText( int_to_string( cPreferences::m_video_screen_bpp_default ), NULL );
	if( list_item )
	{
		combo_bpp->setItemSelectState( list_item, 1 );
		vid_bpp = cPreferences::m_video_screen_bpp_default;
	}

	if( cPreferences::m_video_fullscreen_default )
	{
		combo_fullscreen->setText( UTF8_("On") );
	}
	else
	{
		combo_fullscreen->setText( UTF8_("Off") );
	}
	vid_fullscreen = cPreferences::m_video_fullscreen_default;

	if( cPreferences::m_video_vsync_default )
	{
		combo_vsync->setText( UTF8_("On") );
	}
	else
	{
		combo_vsync->setText( UTF8_("Off") );
	}
	vid_vsync = cPreferences::m_video_vsync_default;

	slider_geometry_quality->setCurrentValue( cPreferences::m_geometry_quality_default );
	vid_geometry_detail = cPreferences::m_geometry_quality_default;

	slider_texture_quality->setCurrentValue( cPreferences::m_texture_quality_default );
	vid_texture_detail = cPreferences::m_texture_quality_default;

	return 1;
}

bool cMenu_Options_Video :: Button_Apply_Clicked( const CEGUI::EventArgs &event )
{
	// draw reinitialization text
	Draw_Static_Text( _("Reinitialization"), &green, NULL, 0 );

	pGuiSystem->renderGUI();
	pRenderer->Render();
	SDL_GL_SwapBuffers();

	// apply new settings
	pPreferences->Apply_Video( vid_w, vid_h, vid_bpp, vid_fullscreen, vid_vsync, vid_geometry_detail, vid_texture_detail );

	// clear
	pMenuCore->next_menu = MENU_OPTIONS;
	Game_Action = GA_ENTER_MENU;

	return 1;
}

bool cMenu_Options_Video :: Button_Recreate_Cache_Clicked( const CEGUI::EventArgs &event )
{
	Loading_Screen_Init();

	// save textures for reloading from file
	pImage_Manager->Grab_Textures( 1, 1 );

	// recreate cache
	pVideo->Init_Image_Cache( 1, 1 );

	// restore textures
	pImage_Manager->Restore_Textures( 1 );

	Loading_Screen_Exit();

	return 1;
}

/* *** *** *** *** *** *** *** *** cMenu_Options_Audio *** *** *** *** *** *** *** *** *** */

cMenu_Options_Audio :: cMenu_Options_Audio( void )
: cMenu_Options()
{

}

cMenu_Options_Audio :: ~cMenu_Options_Audio( void )
{

}

void cMenu_Options_Audio :: Init( void )
{
	cMenu_Options::Init();

	layout_file = "menu/options_audio.layout";
	
	pMenuCore->handler->Set_Active( MENU_AUDIO );

	Init_GUI();
}

void cMenu_Options_Audio :: Init_GUI( void )
{
	cMenu_Options::Init_GUI();

	// Audio Hz
	CEGUI::Window *text_hz = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_hz" ));
	text_hz->setText( UTF8_("Hertz (Hz)") );
	text_hz->setTooltipText( UTF8_("You should only change the value if the audio is scratchy.") );

	combo_audio_hz = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_audio_hz" ));

	CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem( "22050", 0 );
	item->setTextColours( CEGUI::colour( 1, 0, 0 ) );
	combo_audio_hz->addItem( item );
	item = new CEGUI::ListboxTextItem( "44100", 1 );
	item->setTextColours( CEGUI::colour( 0, 1, 0 ) );
	combo_audio_hz->addItem( item );
	item = new CEGUI::ListboxTextItem( "48000", 2 );
	item->setTextColours( CEGUI::colour( 0, 0, 1 ) );
	combo_audio_hz->addItem( item );

	// Set current value
	combo_audio_hz->setText( int_to_string( pPreferences->m_audio_hz ) );

	combo_audio_hz->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Audio::Audio_Hz_Select, this ) );


	// Music
	CEGUI::Window *text_music = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_music" ));
	text_music->setText( UTF8_("Music") );
	text_music->setTooltipText( UTF8_("Enable to play music. You need to have the Music Addon installed.") );

	combo_music = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_music" ));

	item = new CEGUI::ListboxTextItem( UTF8_("On"), 0 );
	item->setTextColours( CEGUI::colour( 0, 1, 0 ) );
	combo_music->addItem( item );
	item = new CEGUI::ListboxTextItem( UTF8_("Off"), 1 );
	item->setTextColours( CEGUI::colour( 0, 0, 1 ) );
	combo_music->addItem( item );

	if( pAudio->m_music_enabled )
	{
		combo_music->setText( UTF8_("On") );
	}
	else
	{
		combo_music->setText( UTF8_("Off") );
	}

	combo_music->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Audio::Music_Select, this ) );

	// music volume slider
	slider_music = static_cast<CEGUI::Slider *>(CEGUI::WindowManager::getSingleton().getWindow( "slider_music_volume" ));
	slider_music->setTooltipText( UTF8_("Set the Music Volume.") );

	slider_music->setCurrentValue( static_cast<float>(pAudio->m_music_volume) );
	slider_music->subscribeEvent( CEGUI::Slider::EventValueChanged, CEGUI::Event::Subscriber( &cMenu_Options_Audio::Music_Vol_Changed, this ) );
	

	// Sounds
	CEGUI::Window *text_sound = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_sound" ));
	text_sound->setText( UTF8_("Sound") );
	text_sound->setTooltipText( UTF8_("Enable to play Sounds.") ) ;

	combo_sounds = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_sounds" ));

	item = new CEGUI::ListboxTextItem( UTF8_("On"), 0 );
	item->setTextColours( CEGUI::colour( 0, 1, 0 ) );
	combo_sounds->addItem( item );
	item = new CEGUI::ListboxTextItem( UTF8_("Off"), 1 );
	item->setTextColours( CEGUI::colour( 1, 0.6f, 0.3f ) );
	combo_sounds->addItem( item );

	if( pAudio->m_sound_enabled )
	{
		combo_sounds->setText( UTF8_("On") );
	}
	else
	{
		combo_sounds->setText( UTF8_("Off") );
	}

	combo_sounds->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Audio::Sound_Select, this ) );

	// sound volume slider
	slider_sound = static_cast<CEGUI::Slider *>(CEGUI::WindowManager::getSingleton().getWindow( "slider_sound_volume" ));
	slider_sound->setTooltipText( UTF8_("Set the Sound Volume.") );

	slider_sound->setCurrentValue( static_cast<float>(pAudio->m_sound_volume) );
	slider_sound->subscribeEvent( CEGUI::Slider::EventValueChanged, CEGUI::Event::Subscriber( &cMenu_Options_Audio::Sound_Vol_Changed, this ) );

	// Reset
	CEGUI::PushButton *button_reset = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_reset" ));
	button_reset->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cMenu_Options_Audio::Button_Reset_Clicked, this ) );
	button_reset->setText( UTF8_("Reset") );
}

void cMenu_Options_Audio :: Update( void )
{
	cMenu_Options::Update();

	if( !action )
	{
		return;
	}

	cMenu_Options::Update();

	// only audio actions
	if( pMenuCore->handler->active <= 4 )
	{
		return;
	}

	action = 0;

	// Hz
	if( pMenuCore->handler->active == 5 )
	{
		unsigned int selected = combo_audio_hz->getItemIndex( combo_audio_hz->findItemWithText( combo_audio_hz->getText(), NULL ) );

		CEGUI::ListboxItem *new_selected = NULL;

		// last item selected
		if( selected == combo_audio_hz->getItemCount() - 1 )
		{
			new_selected = combo_audio_hz->getListboxItemFromIndex( 0 );
		}
		// select next item
		else
		{
			new_selected = combo_audio_hz->getListboxItemFromIndex( selected + 1 );
		}
		
		combo_audio_hz->setText( new_selected->getText() );
		combo_audio_hz->setItemSelectState( new_selected, 1 );
		Audio_Hz_Select( CEGUI::WindowEventArgs( combo_audio_hz ) );
	}
	// Music
	else if( pMenuCore->handler->active == 6 )
	{
		pAudio->Toggle_Music();

		if( pAudio->m_music_enabled )
		{
			combo_music->setText( UTF8_("On") );
		}
		else
		{
			combo_music->setText( UTF8_("Off") );
		}
	}
	// Sounds
	else if( pMenuCore->handler->active == 7 )
	{
		pAudio->Toggle_Sounds();

		if( pAudio->m_sound_enabled )
		{
			combo_sounds->setText( UTF8_("On") );
		}
		else
		{
			combo_sounds->setText( UTF8_("Off") );
		}
	}
}

void cMenu_Options_Audio :: Draw( void )
{
	cMenu_Options::Draw();

	Draw_End();
}

bool cMenu_Options_Audio :: Audio_Hz_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	pPreferences->m_audio_hz = string_to_int( item->getText().c_str() );

	// draw reloading text
	Draw_Static_Text( _("Reloading"), &green, NULL, 0 );
	// reload
	pAudio->Close();
	pSound_Manager->Delete_All();
	pAudio->Init();
	// todo : add sound manager function to reload sounds and music when needed
	Preload_Sounds();

	return 1;
}

bool cMenu_Options_Audio :: Music_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	bool music_enabled = 0;

	if( item->getText().compare( UTF8_("On") ) == 0 )
	{
		music_enabled = 1;
	}

	if( pAudio->m_music_enabled != music_enabled )
	{
		pAudio->Toggle_Music();
	}

	return 1;
}

bool cMenu_Options_Audio :: Music_Vol_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	Uint8 val = static_cast<Uint8>(static_cast<CEGUI::Slider *>( windowEventArgs.window )->getCurrentValue());

	pAudio->Set_Music_Volume( val );
	// save volume
	pAudio->m_music_volume = val;

	return 1;
}

bool cMenu_Options_Audio :: Sound_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	bool sound_enabled = 0;

	if( item->getText().compare( UTF8_("On") ) == 0 )
	{
		sound_enabled= 1;
	}

	if( pAudio->m_sound_enabled != sound_enabled )
	{
		pAudio->Toggle_Sounds();
	}

	return 1;
}

bool cMenu_Options_Audio :: Sound_Vol_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	Uint8 val = static_cast<Uint8>(static_cast<CEGUI::Slider *>( windowEventArgs.window )->getCurrentValue());

	pAudio->Set_Sound_Volume( val );
	// save volume
	pAudio->m_sound_volume = val;

	return 1;
}

bool cMenu_Options_Audio :: Button_Reset_Clicked( const CEGUI::EventArgs &event )
{
	pPreferences->Reset_Audio();

	// clear
	pMenuCore->next_menu = MENU_OPTIONS;
	Game_Action = GA_ENTER_MENU;

	return 1;
}

/* *** *** *** *** *** *** *** *** cMenu_Options_Controls *** *** *** *** *** *** *** *** *** */

cMenu_Options_Controls :: cMenu_Options_Controls( void )
: cMenu_Options()
{
	//
}

cMenu_Options_Controls :: ~cMenu_Options_Controls( void )
{
	//
}

void cMenu_Options_Controls :: Init( void )
{
	cMenu_Options::Init();

	layout_file = "menu/options_controls.layout";
	menu_posy = 220;

	pMenuCore->handler->Set_Active( MENU_CONTROLS );

	Init_GUI();
}

void cMenu_Options_Controls :: Init_GUI( void )
{
	cMenu_Options::Init_GUI();

	// Keyboard listbox
	CEGUI::Window *text_keyboard_shortcuts = CEGUI::WindowManager::getSingleton().getWindow( "text_keyboard_shortcuts" );
	text_keyboard_shortcuts->setText( UTF8_("Shortcuts") );

	CEGUI::MultiColumnList *listbox_keyboard = static_cast<CEGUI::MultiColumnList *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_keyboard" ));

	listbox_keyboard->addColumn( UTF8_("Name"), 0, CEGUI::UDim( 0.47f, 0 ) );
	listbox_keyboard->addColumn( UTF8_("Key"), 1, CEGUI::UDim( 0.47f, 0 ) );
	Build_Shortcut_List();

	listbox_keyboard->subscribeEvent( CEGUI::MultiColumnList::EventMouseDoubleClick, CEGUI::Event::Subscriber( &cMenu_Options_Controls::Keyboard_List_Double_Click, this ) );

	// Keyboard scroll speed
	CEGUI::Window *text_keyboard_scroll_speed = CEGUI::WindowManager::getSingleton().getWindow( "text_keyboard_scroll_speed" );
	text_keyboard_scroll_speed->setText( UTF8_("Scroll Speed") );

	CEGUI::Slider *slider_scoll_speed = static_cast<CEGUI::Slider *>(CEGUI::WindowManager::getSingleton().getWindow( "slider_keyboard_scroll_speed" ));
	slider_scoll_speed->setCurrentValue( pPreferences->m_scroll_speed );
	slider_scoll_speed->subscribeEvent( CEGUI::Slider::EventValueChanged, CEGUI::Event::Subscriber( &cMenu_Options_Controls::Keyboard_Slider_Scroll_Speed_Changed, this ) );

	// Reset Keyboard
	CEGUI::PushButton *button_reset_keyboard = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_reset_keyboard" ));
	button_reset_keyboard->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cMenu_Options_Controls::Button_Reset_Keyboard_Clicked, this ) );
	button_reset_keyboard->setText( UTF8_("Reset") );

	// Joystick analog jump text
	CEGUI::Window *text_joystick_analog_jump = CEGUI::WindowManager::getSingleton().getWindow( "text_joystick_analog_jump" );
	text_joystick_analog_jump->setText( UTF8_("Analog Jump") );

	text_joystick_analog_jump->subscribeEvent( CEGUI::Window::EventMouseClick, CEGUI::Event::Subscriber( &cMenu_Options_Controls::Joystick_Analog_Jump_Click, this ) );
	
	// Joystick name
	CEGUI::Window *text_joystick_name = CEGUI::WindowManager::getSingleton().getWindow( "text_joystick_name" );
	text_joystick_name->setText( UTF8_("Joystick") );

	text_joystick_name->subscribeEvent( CEGUI::Window::EventMouseClick, CEGUI::Event::Subscriber( &cMenu_Options_Controls::Joystick_Name_Click, this ) );

	CEGUI::Combobox *combo_joy = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_joy" ));

	// Add None
	CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem( UTF8_("None"), 0 );
	item->setTextColours( CEGUI::colour( 0, 0, 1 ) );
	combo_joy->addItem( item );

	// Add all Joy names
	vector<std::string> joy_names = pJoystick->Get_Names();

	for( unsigned int i = 0; i < joy_names.size(); i++ )
	{
		item = new CEGUI::ListboxTextItem( joy_names[i], 1 );
		item->setTextColours( CEGUI::colour( 0.3f, 1, 0.3f ) );
		combo_joy->addItem( item );
	}

	// Selected Item
	CEGUI::ListboxTextItem *selected_item = NULL;

	// Set current Joy name
	if( pPreferences->m_joy_enabled )
	{
		selected_item = static_cast<CEGUI::ListboxTextItem *>( combo_joy->findItemWithText( pJoystick->Get_Name(), NULL ) );
	}
	// disabled
	else
	{
		selected_item = static_cast<CEGUI::ListboxTextItem *>( combo_joy->getListboxItemFromIndex( 0 ) );
	}
	// set Item
	combo_joy->setText( selected_item->getText() );

	combo_joy->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Controls::Joystick_Name_Select, this ) );

	// Joystick analog jump
	CEGUI::Combobox *combo_joy_analog_jump = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_joy_analog_jump" ));

	item = new CEGUI::ListboxTextItem( UTF8_("On"), 0 );
	item->setTextColours( CEGUI::colour( 0, 0, 1 ) );
	combo_joy_analog_jump->addItem( item );
	item = new CEGUI::ListboxTextItem( UTF8_("Off"), 1 );
	item->setTextColours( CEGUI::colour( 0, 1, 0 ) );
	combo_joy_analog_jump->addItem( item );

	if( pPreferences->m_joy_analog_jump )
	{
		combo_joy_analog_jump->setText( UTF8_("On") );
	}
	else
	{
		combo_joy_analog_jump->setText( UTF8_("Off") );
	}

	combo_joy_analog_jump->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cMenu_Options_Controls::Joystick_Analog_Jump_Select, this ) );

	// Joystick axis horizontal
	CEGUI::Window *text_joystick_axis_hor = CEGUI::WindowManager::getSingleton().getWindow( "text_joystick_axis_hor" );
	text_joystick_axis_hor->setText( UTF8_("Axis Hor") );

	CEGUI::Spinner *spinner_joystick_axis_hor = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_joystick_axis_hor" ));
	spinner_joystick_axis_hor->setCurrentValue( static_cast<float>(pPreferences->m_joy_axis_hor) );
	spinner_joystick_axis_hor->subscribeEvent( CEGUI::Spinner::EventValueChanged, CEGUI::Event::Subscriber( &cMenu_Options_Controls::Joystick_Spinner_Axis_Hor_Changed, this ) );

	// Joystick axis vertical
	CEGUI::Window *text_joystick_axis_ver = CEGUI::WindowManager::getSingleton().getWindow( "text_joystick_axis_ver" );
	text_joystick_axis_ver->setText( UTF8_("Ver") );

	CEGUI::Spinner *spinner_joystick_axis_ver = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_joystick_axis_ver" ));
	spinner_joystick_axis_ver->setCurrentValue( static_cast<float>(pPreferences->m_joy_axis_ver) );
	spinner_joystick_axis_ver->subscribeEvent( CEGUI::Spinner::EventValueChanged, CEGUI::Event::Subscriber( &cMenu_Options_Controls::Joystick_Spinner_Axis_Ver_Changed, this ) );

	// Joystick shortcut listbox
	CEGUI::Window *text_joystick_shortcuts = CEGUI::WindowManager::getSingleton().getWindow( "text_joystick_shortcuts" );
	text_joystick_shortcuts->setText( UTF8_("Shortcuts") );

	CEGUI::MultiColumnList *listbox_joystick = static_cast<CEGUI::MultiColumnList *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_joystick" ));

	listbox_joystick->addColumn( UTF8_("Name"), 0, CEGUI::UDim( 0.47f, 0 ) );
	listbox_joystick->addColumn( UTF8_("Button"), 1, CEGUI::UDim( 0.47f, 0 ) );
	Build_Shortcut_List( 1 );

	listbox_joystick->subscribeEvent( CEGUI::MultiColumnList::EventMouseDoubleClick, CEGUI::Event::Subscriber( &cMenu_Options_Controls::Joystick_List_Double_Click, this ) );

	// Reset Joystick
	CEGUI::PushButton *button_reset_joystick = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_reset_joystick" ));
	button_reset_joystick->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cMenu_Options_Controls::Button_Reset_Joystick_Clicked, this ) );
	button_reset_joystick->setText( UTF8_("Reset") );
}

void cMenu_Options_Controls :: Update( void )
{
	cMenu_Options::Update();

	if( !action )
	{
		return;
	}

	cMenu_Options::Update();

	// Controls
	if( pMenuCore->handler->active <= 4 )
	{
		return;
	}

	action = 0;
}

void cMenu_Options_Controls :: Draw( void )
{
	cMenu_Options::Draw();

	Draw_End();
}

void cMenu_Options_Controls :: Build_Shortcut_List( bool joystick /* = 0 */ )
{
	// Get Listbox
	CEGUI::MultiColumnList *listbox = NULL;

	// Keyboard
	if( !joystick )
	{
		listbox = static_cast<CEGUI::MultiColumnList *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_keyboard" ));
	}
	// Joystick
	else
	{
		listbox = static_cast<CEGUI::MultiColumnList *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_joystick" ));
	}

	listbox->resetList();

	// build shortcut list
	vector<cShortcut_item> shortcuts;

	// only keyboard
	if( !joystick )
	{
		shortcuts.push_back( cShortcut_item( UTF8_("Up"), INP_UP ) );
		shortcuts.push_back( cShortcut_item( UTF8_("Down"), INP_DOWN ) );
		shortcuts.push_back( cShortcut_item( UTF8_("Left"), INP_LEFT ) );
		shortcuts.push_back( cShortcut_item( UTF8_("Right"), INP_RIGHT ) );
	}

	shortcuts.push_back( cShortcut_item( UTF8_("Jump"), INP_JUMP ) );
	shortcuts.push_back( cShortcut_item( UTF8_("Shoot"), INP_SHOOT ) );
	shortcuts.push_back( cShortcut_item( UTF8_("Action"), INP_ACTION ) );
	shortcuts.push_back( cShortcut_item( UTF8_("Weapon1"), INP_WEAPON1 ) );
	shortcuts.push_back( cShortcut_item( UTF8_("Weapon2"), INP_WEAPON2 ) );
	shortcuts.push_back( cShortcut_item( UTF8_("Weapon3"), INP_WEAPON3 ) );
	shortcuts.push_back( cShortcut_item( UTF8_("Weapon4"), INP_WEAPON4 ) );
	shortcuts.push_back( cShortcut_item( UTF8_("Weapon5"), INP_WEAPON5 ) );
	shortcuts.push_back( cShortcut_item( UTF8_("Weapon6"), INP_WEAPON6 ) );
	shortcuts.push_back( cShortcut_item( UTF8_("Weapon7"), INP_WEAPON7 ) );
	shortcuts.push_back( cShortcut_item( UTF8_("Weapon8"), INP_WEAPON8 ) );
	shortcuts.push_back( cShortcut_item( UTF8_("Weapon9"), INP_WEAPON9 ) );
	shortcuts.push_back( cShortcut_item( UTF8_("Weapon0"), INP_WEAPON0 ) );


	// only joystick
	if( joystick )
	{
		shortcuts.push_back( cShortcut_item( UTF8_("Item"), INP_ITEM ) );
		shortcuts.push_back( cShortcut_item( UTF8_("Exit"), INP_EXIT ) );
	}

	// add all available shortcuts
	for( vector<cShortcut_item>::iterator itr = shortcuts.begin(), itr_end = shortcuts.end(); itr != itr_end; ++itr )
	{
		cShortcut_item shortcut_item = (*itr);
		
		CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem( shortcut_item.m_name, shortcut_item.m_id );
		//item->setTextColours( colour( 0.6f, 0.6f, 0.6f ) );
		item->setSelectionColours( CEGUI::colour( 0.33f, 0.33f, 0.33f ) );
		item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
		unsigned int row_id = listbox->addRow( item, 0 );

		// Get shortcut key name
		std::string shortcut_key;
		// Keyboard
		if( !joystick )
		{
			SDLKey *key = pKeyboard->Get_Shortcut( shortcut_item.m_id );
			shortcut_key = SDL_GetKeyName( *key );
		}
		// Joystick
		else
		{
			Uint8 *button = pJoystick->Get_Shortcut( shortcut_item.m_id );
			shortcut_key = int_to_string( *button );
		}

		item = new CEGUI::ListboxTextItem( shortcut_key, 0 );
		//item->setTextColours( colour( 0.6f, 0.6f, 0.6f ) );
		item->setSelectionColours( CEGUI::colour( 0.33f, 0.33f, 0.33f ) );
		item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
		listbox->setItem( item, 1, row_id );
	}
}

void cMenu_Options_Controls :: Set_Shortcut( std::string shortcut_name, input_identifier shortcut_id, bool joystick /* = 0 */ )
{
	std::string info_text;

	if( !joystick )
	{
		info_text += _("Press a key");
	}
	else
	{
		info_text += _("Press a button");
	}

	Draw_Static_Text( info_text + _(" for ") + shortcut_name + _(". Press ESC to cancel."), &orange, NULL, 0 );

	bool sub_done = 0;

	while( !sub_done )
	{
		// no event
		if( !SDL_PollEvent( &input_event ) )
		{
			continue;
		}

		if( input_event.key.keysym.sym == SDLK_ESCAPE || input_event.key.keysym.sym == SDLK_BACKSPACE )
		{
			sub_done = 1;
			break;
		}

		if( !joystick && input_event.type != SDL_KEYDOWN )
		{
			continue;
		}
		else if( joystick && input_event.type != SDL_JOYBUTTONDOWN )
		{
			continue;
		}

		// Keyboard
		if( !joystick )
		{
			pKeyboard->Assign_Shortcut( shortcut_id, input_event.key.keysym.sym );
		}
		// Joystick
		else
		{
			pJoystick->Assign_Shortcut( shortcut_id, input_event.jbutton.button );
		}

		sub_done = 1;
	}

	Build_Shortcut_List( joystick );
}

void cMenu_Options_Controls :: Joy_Default( unsigned int index )
{
	pPreferences->m_joy_enabled = 1;
	pPreferences->m_joy_name = SDL_JoystickName( index );

	// initialize and if no joystick available disable
	pJoystick->Init();
}

void cMenu_Options_Controls :: Joy_Disable( void )
{
	pPreferences->m_joy_enabled = 0;
	pPreferences->m_joy_name.clear();

	// close the joystick
	pJoystick->Stick_Close();
}

bool cMenu_Options_Controls :: Keyboard_List_Double_Click( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::MultiColumnList *>( windowEventArgs.window )->getFirstSelectedItem();

	// set shortcut
	if( item )
	{
		Set_Shortcut( item->getText().c_str(), static_cast<input_identifier>(item->getID()) );
	}

	return 1;
}

bool cMenu_Options_Controls :: Keyboard_Slider_Scroll_Speed_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	// set new value
	pPreferences->m_scroll_speed = static_cast<CEGUI::Slider *>( windowEventArgs.window )->getCurrentValue();

	return 1;
}

bool cMenu_Options_Controls :: Joystick_Name_Click( const CEGUI::EventArgs &event )
{
	// Get Joystick Combo
	CEGUI::Combobox *combo_joy = static_cast<CEGUI::Combobox *>( CEGUI::WindowManager::getSingleton().getWindow( "combo_joy" ) );
	
	// selected item id
	int selected_item = 0;
	// current list item
	CEGUI::ListboxTextItem *list_item = static_cast<CEGUI::ListboxTextItem *>( combo_joy->findItemWithText( combo_joy->getText(), NULL ) );

	// if selected
	if( list_item )
	{
		selected_item = combo_joy->getItemIndex( list_item );
	}

	// select first
	if( selected_item >= SDL_NumJoysticks() )
	{
		selected_item = 0;
	}
	// select next item
	else
	{
		selected_item++;
	}

	// Disable Joy
	if( selected_item == 0 )
	{
		Joy_Disable();
	}
	// Select Joy
	else
	{
		Joy_Default( selected_item - 1 );
	}

	// check if initialization succeeded
	if( selected_item )
	{
		// initialized
		if( pPreferences->m_joy_enabled )
		{
			Draw_Static_Text( _("Enabled : ") + pJoystick->Get_Name(), &yellow );
		}
		// failed
		else
		{
			selected_item = 0;
		}
	}

	combo_joy->setText( combo_joy->getListboxItemFromIndex( selected_item )->getText() );

	return 1;
}

bool cMenu_Options_Controls :: Joystick_Analog_Jump_Click( const CEGUI::EventArgs &event )
{
	pPreferences->m_joy_analog_jump = !pPreferences->m_joy_analog_jump;

	if( pPreferences->m_joy_analog_jump )
	{
		CEGUI::WindowManager::getSingleton().getWindow( "combo_joy_analog_jump" )->setText( _("On") );
	}
	else
	{
		CEGUI::WindowManager::getSingleton().getWindow( "combo_joy_analog_jump" )->setText( _("Off") );
	}

	return 1;
}

bool cMenu_Options_Controls :: Joystick_Name_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::Combobox *combo = static_cast<CEGUI::Combobox*>( windowEventArgs.window );
	CEGUI::ListboxItem *item = combo->getSelectedItem();

	if( item->getText().compare( _("None") ) == 0 )
	{
		Joy_Disable();
	}
	else
	{
		Joy_Default( combo->getItemIndex( item ) - 1 );
	}

	return 1;
}

bool cMenu_Options_Controls :: Joystick_Analog_Jump_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox*>( windowEventArgs.window )->getSelectedItem();

	bool analog_jump = 0;

	if( item->getText().compare( _("On") ) == 0 )
	{
		analog_jump = 1;
	}

	pPreferences->m_joy_analog_jump = analog_jump;

	return 1;
}

bool cMenu_Options_Controls :: Joystick_Spinner_Axis_Hor_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	// set new value
	pPreferences->m_joy_axis_hor = static_cast<int>(static_cast<CEGUI::Spinner *>( windowEventArgs.window )->getCurrentValue());

	return 1;
}

bool cMenu_Options_Controls :: Joystick_Spinner_Axis_Ver_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	// set new value
	pPreferences->m_joy_axis_ver = static_cast<int>(static_cast<CEGUI::Spinner *>( windowEventArgs.window )->getCurrentValue());

	return 1;
}

bool cMenu_Options_Controls :: Joystick_List_Double_Click( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::MultiColumnList *>( windowEventArgs.window )->getFirstSelectedItem();

	// set shortcut
	if( item )
	{
		Set_Shortcut( item->getText().c_str(), static_cast<input_identifier>(item->getID()), 1 );
	}

	return 1;
}

bool cMenu_Options_Controls :: Button_Reset_Keyboard_Clicked( const CEGUI::EventArgs &event )
{
	pPreferences->Reset_Keyboard();

	// clear
	pMenuCore->next_menu = MENU_OPTIONS;
	Game_Action = GA_ENTER_MENU;

	return 1;
}

bool cMenu_Options_Controls :: Button_Reset_Joystick_Clicked( const CEGUI::EventArgs &event )
{
	pPreferences->Reset_Joystick();

	// clear
	pMenuCore->next_menu = MENU_OPTIONS;
	Game_Action = GA_ENTER_MENU;

	return 1;
}

/* *** *** *** *** *** *** *** *** cMenu_Savegames *** *** *** *** *** *** *** *** *** */

cMenu_Savegames :: cMenu_Savegames( bool ntype_save )
: cMenu_Base()
{
	type_save = ntype_save;

	for( unsigned int i = 0; i < 9; i++ )
	{
		savegame_temp.push_back( new cHudSprite() );
	}
}

cMenu_Savegames :: ~cMenu_Savegames( void )
{
	for( HudSpriteList::iterator itr = savegame_temp.begin(), itr_end = savegame_temp.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	savegame_temp.clear();
}

void cMenu_Savegames :: Init( void )
{
	cMenu_Base::Init();

	cMenu_Item *temp_item = NULL;

	cGL_Surface *back1 = pFont->Render_Text( pFont->m_font_normal, _("Back"), text_color );

	Update_Saved_Games_Text();

	// savegame descriptions
	for( HudSpriteList::iterator itr = savegame_temp.begin(), itr_end = savegame_temp.end(); itr != itr_end; ++itr )
	{
		temp_item = new cMenu_Item();
		temp_item->image_default->Set_Image( (*itr)->m_image );
		temp_item->Set_Pos( static_cast<float>(game_res_w) / 5, menu_posy );
		pMenuCore->handler->Add_Menu_Item( temp_item, 1.5f, grey );
		
		menu_posy += temp_item->image_default->m_col_rect.m_h;
	}

	// back
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( back1 );
	temp_item->Set_Pos( static_cast<float>(game_res_w) / 18, 450 );
	temp_item->isquit = 1;
	pMenuCore->handler->Add_Menu_Item( temp_item, 1.5f, grey );

	if( type_save )
	{
		drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/save.png" ), game_res_w * 0.2f, game_res_h * 0.15f ) );
		drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/save.png" ), game_res_w * 0.07f, game_res_h * 0.24f ) );
	}
	else
	{
		drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/load.png" ), game_res_w * 0.2f, game_res_h * 0.15f ) );
		drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/load.png" ), game_res_w * 0.07f, game_res_h * 0.24f ) );
	}
	drawlist.push_back( new cHudSprite( back1, -200, 0, 1 ) );

	Init_GUI();
}

void cMenu_Savegames :: Init_GUI( void )
{
	cMenu_Base::Init_GUI();
}

void cMenu_Savegames :: Update( void )
{
	cMenu_Base::Update();

	if( !action )
	{
		return;
	}

	action = 0;

	// back
	if( pMenuCore->handler->active == 9 )
	{
		Exit();
		return;
	}

	if( !type_save )
	{
		Update_Load();
	}
	else
	{
		Update_Save();
	}
}

void cMenu_Savegames :: Draw( void )
{
	cMenu_Base::Draw();
	Draw_End();
}

void cMenu_Savegames :: Exit( void )
{
	pMenuCore->next_menu = MENU_MAIN;
	Game_Action = GA_ENTER_MENU;
}

void cMenu_Savegames :: Update_Load( void )
{
	// not valid
	if( !pSavegame->Is_Valid( pMenuCore->handler->active + 1 ) )
	{
		return;
	}

	pAudio->Fadeout_Music( 1000 );
	pAudio->Play_Sound( "savegame_load.ogg" );

	int save_type = pSavegame->Load_Game( pMenuCore->handler->active + 1 );

	// Level save
	if( save_type == 1 )
	{
		Game_Action = GA_ENTER_LEVEL;
	}
	// World save
	else if( save_type == 2 )
	{
		Game_Action = GA_ENTER_WORLD;
	}
}

void cMenu_Savegames :: Update_Save( void )
{
	// not valid
	if( pOverworld_Player->current_waypoint < 0 && !pActive_Level->Is_Loaded() )
	{
		return;
	}

	std::string descripion = Set_Save_Description( pMenuCore->handler->active + 1 );
	
	pFramerate->Reset();

	if( descripion.compare( "Not enough Points" ) == 0 ) 
	{
		Game_Action = GA_ENTER_MENU;
		pMenuCore->next_menu = MENU_MAIN;
		return;
	}

	if( descripion.empty() )
	{
		return;
	}

	pAudio->Play_Sound( "savegame_save.ogg" );

	// no costs in debug builds
#ifndef _DEBUG
	if( pActive_Level->Is_Loaded() )
	{
		pHud_Points->Set_Points( pPlayer->points - 3000 );
	}
#endif
	// save
	pSavegame->Save_Game( pMenuCore->handler->active + 1, descripion );

	Game_Action = GA_ENTER_MENU;
	pMenuCore->next_menu = MENU_MAIN;
}

std::string cMenu_Savegames :: Set_Save_Description( unsigned int save_slot )
{
	if( save_slot == 0 || save_slot > 9 )
	{
		return "";
	}
// save always in debug builds
#ifndef _DEBUG
	if( pActive_Level->Is_Loaded() && pPlayer->points < 3000 )
	{
		Clear_Input_Events();
		Draw_Static_Text( _("3000 Points needed for saving in a level.\nSaving on the Overworld is free.") );

		return "Not enough Points";
	}
#endif
	std::string save_description;

	bool auto_erase_description = 0;

	// if Savegame exists use old description
	if( pSavegame->Is_Valid( save_slot ) )
	{
		save_description.clear();
		// get only the description
		save_description = pSavegame->Get_Description( save_slot, 1 );
	}
	else
	{
		// use default description
		save_description = _("No Description");
		auto_erase_description = 1;
	}

	return Box_Text_Input( save_description, _("Enter Description"), auto_erase_description );
}

void cMenu_Savegames :: Update_Saved_Games_Text( void )
{
	unsigned int save_slot = 0;

	for( HudSpriteList::iterator itr = savegame_temp.begin(), itr_end = savegame_temp.end(); itr != itr_end; ++itr )
	{
		save_slot++;
		(*itr)->Set_Image( pFont->Render_Text( pFont->m_font_normal, pSavegame->Get_Description( save_slot ), text_color_value ), 1, 1 );
	}
}

/* *** *** *** *** *** *** *** *** cMenu_Credits *** *** *** *** *** *** *** *** *** */

cMenu_Credits :: cMenu_Credits( void )
: cMenu_Base()
{

}

cMenu_Credits :: ~cMenu_Credits( void )
{

}

void cMenu_Credits :: Init( void )
{
	pAudio->Play_Music( "land/hyper_1.ogg", -1, 1, 1500 );

	cMenu_Base::Init();

	pAudio->Fadeout_Music( 1500 );

	cMenu_Item *temp_item = NULL;
	menu_posy = game_res_h * 1.1f;

	// black background because of fade alpha
	glClearColor( 0, 0, 0, 1 );

	// fade in
	Fade();

	cGL_Surface *back1 = pFont->Render_Text( pFont->m_font_normal, _("Back"), text_color );

	// clear credits
	drawlist.clear();

	// add credits texts
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "========= Eta Chronicles Development =========", white ), 0, 20, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "       ** Project Fork of Secret Maryo Chronicles **      ", white ), 0, 20, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Cody Van De Mark", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( black, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Eta Chronicles Developer", white ), 0, -3, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Eta Chronicles Project Leader", white ), 0, -3, 1 ) );	
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " ", white ), 0, 20, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "=== Original Secret Maryo Chronicles Development ===", white ), 0, 20, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Florian Richter (FluXy)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( black, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Dedicated Developer", white ), 0, -3, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Organizer and Project Leader", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Holger Fey (Nemo)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lila, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Game Tester", white ), 0, -3, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - German Publicity", white ), 0, -3, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Torrent Packager", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Grant ... (youngheart80)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( green, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Graphic Designer", white ), 0, -3, 1 ) );

	// Most Valued Persons (MVP)
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "-- Most Valued Persons (MVP) --", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lightgrey, 1 );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "... (Helios)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( Color( 0.8f, 0.8f, 0.1f ), 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Graphic Designer", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Robert W... (BowserJr)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lightgreen, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Forum and Wiki Moderator", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "... (Sauer2)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( Color( 0.1f, 0.6f, 0.1f ), 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Level Contributor", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "... (Simpletoon)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( Color( 0.2f, 0.2f, 0.8f ), 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Developer", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Markus Hiebl (Frostbringer)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( Color( 0.9f, 0.1f, 0.8f ), 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Graphic Designer", white ), 0, -3, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Level Contributor", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Tristan Heaven (nyhm)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lightblue, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Gentoo eBuild Maintainer", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Muammar El Khatib (muammar)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lightred, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Debian Package Maintainer", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "David Hernandez (vencabot_teppoo)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( Color( 0.8f, 0.6f, 0.2f ), 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Music Artist", white ), 0, -3, 1 ) );

	// Retired
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "-- Retired --", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lightgrey, 1 );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Mark Richards (dteck)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( blue, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Graphic Designer", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Mario Fink (maYO)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( blue, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Graphic Designer", white ), 0, -3, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Website Graphic Designer", white ), 0, -3, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Other Support", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "... (Polilla86)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( Color( 0.7f, 0.1f, 0.2f ), 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Graphic Designer", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Ursula ... (Pipgirl)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( Color( 0.2f, 0.9f, 0.2f ), 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Graphic Designer", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Tobias Maasland (Weirdnose)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( Color( 0.9f, 0.7f, 0.2f ), 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Level and World Contributor", white ), 0, -3, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Assistant Developer", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Robert ... (Consonance)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lightred, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Sound and Music Artist", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Justin ... (LoXodonte)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lightblue, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Music Artist", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Matt J... (mattwj)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( red, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - eDonkey Packager", white ), 0, -3, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Quality Assurance", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Bodhi Crandall-Rus (Boder)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( green, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - All Hands Person", white ), 0, -3, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Game Tester", white ), 0, -3, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Assistant Graphic Designer", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "John Daly (Johnlein)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( yellow, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Graphic Designer", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Gustavo Gutierrez (Enzakun)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lightred, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Maryo Graphic Designer", white ), 0, -3, 1 ) );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Thomas Huth (Thothy)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( greenyellow, 1 );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, " - Linux Maintainer", white ), 0, -3, 1 ) );

	// Thanks
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "-- Thanks --", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lightblue, 1 );

	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Jason Cox (XOC)", white ), 0, 0, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Ricardo Cruz", white ), 0, 0, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Devendra (Yuki),", white ), 0, 0, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Hans de Goede (Hans)", white ), 0, 0, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "... (xPatrickx)", white ), 0, 0, 1 ) );
	drawlist.push_back( new cHudSprite( pFont->Render_Text( pFont->m_font_normal, "Rolando Gonzalez (rolosworld)", white ), 0, 0, 1 ) );

	// set credits position
	for( HudSpriteList::iterator itr = drawlist.begin(), itr_end = drawlist.end(); itr != itr_end; ++itr )
	{
		// get object
		cHudSprite *obj = (*itr);

		// set shadow if not set
		if( obj->m_shadow_pos == 0 )
		{
			obj->Set_Shadow( grey, 1 );
		}
		// set position
		obj->m_pos_x += static_cast<float>(game_res_w) / 5;
		obj->m_pos_y += menu_posy;
		// set posz behind front passive
		obj->m_pos_z = 0.096f;
		// set color combine
		obj->Set_Color_Combine( 0, 0, 0, GL_MODULATE );
		obj->m_color.alpha = 0;
		obj->m_shadow_color.alpha = 0;

		menu_posy = obj->m_pos_y + obj->m_col_rect.m_h;
	}

	// back
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( back1 );
	temp_item->Set_Pos( static_cast<float>(game_res_w) / 18, 250 );
	temp_item->isquit = 1;
	pMenuCore->handler->Add_Menu_Item( temp_item, 2, grey );
	
	drawlist.push_back( new cHudSprite( back1, -200, 0, 1 ) );

	Init_GUI();
}

void cMenu_Credits :: Init_GUI( void )
{
	cMenu_Base::Init_GUI();
}

void cMenu_Credits :: Update( void )
{
	cMenu_Base::Update();

	for( HudSpriteList::iterator itr = drawlist.begin(), itr_end = drawlist.end(); itr != itr_end; ++itr )
	{
		cHudSprite *obj = (*itr);

		// long inactive reset
		if( obj->m_pos_y < -2400 )
		{
			obj->Set_Pos_Y( static_cast<float>(game_res_h) * 1.1f );
		}
		// fading out
		else if( obj->m_pos_y < game_res_h * 0.3f )
		{
			float new_value = obj->m_combine_color[0] - ( pFramerate->m_speed_factor * 0.01f );

			if( new_value < 0 )
			{
				new_value = 0;
			}

			obj->Set_Color_Combine( new_value, new_value, new_value, GL_MODULATE );
			obj->m_color.alpha = static_cast<Uint8>( new_value * 255 );
			obj->m_shadow_color.alpha = obj->m_color.alpha;
		}
		// fading in
		else if( obj->m_pos_y < game_res_h * 0.9f )
		{
			float new_value = obj->m_combine_color[0] + ( pFramerate->m_speed_factor * 0.01f );

			if( new_value > 1 )
			{
				new_value = 1;

				// add particles
				if( obj->m_combine_color[0] < 1 )
				{
					cParticle_Emitter *anim = new cParticle_Emitter();
					anim->Set_Emitter_Rect( Get_Random_Float( game_res_w * 0.1f, game_res_w * 0.8f ), Get_Random_Float( game_res_h * 0.1f, game_res_h * 0.2f ), Get_Random_Float( 0, 10 ), Get_Random_Float( 0, 10 ) );
					anim->Set_Quota( 5 + (rand() % 25) );
					if( rand() % 2 )
					{
						anim->Set_Image( pVideo->Get_Surface( "animation/particles/snowflake_1.png" ) );
						anim->Set_Direction_Range( Get_Random_Float( 30, 70 ), 80 );
						anim->Set_Scale( 0.3f, 0.2f );
						anim->Set_Blending( BLEND_ADD );
					}
					else
					{
						anim->Set_Image( pVideo->Get_Surface( "animation/particles/star.png" ) );
						anim->Set_Direction_Range( 0, 360 );
						anim->Set_Scale( 0.2f, 0.1f );
					}
					anim->Set_Time_to_Live( 1.4f, 0.5f );
					anim->Set_Fading_Size( 1 );
					anim->Set_Color( Color( static_cast<Uint8>( 100 + ( rand() % 155 ) ), 100 + ( rand() % 155 ), 100 + ( rand() % 155 ) ) );
					anim->Set_Speed( 1.6f, 0.7f );
					anim->Set_Const_Rotation_Z( -5, 10 );
					anim->Set_Vertical_Gravity( 0.02f );
					anim->Set_Pos_Z( 0.16f );
					pMenuCore->pMenu_AnimManager->Add( anim );
				}
			}

			obj->Set_Color_Combine( new_value, new_value, new_value, GL_MODULATE );
			obj->m_color.alpha = static_cast<Uint8>( new_value * 255 );
			obj->m_shadow_color.alpha = obj->m_color.alpha;
		}

		// default upwards scroll
		obj->Move( 0, -1.1f );
	}

	if( !action )
	{
		return;
	}

	action = 0;

	// back
	if( pMenuCore->handler->active == 0 )
	{
		Exit();
	}
}

void cMenu_Credits :: Draw( void )
{
	// do not draw if exiting
	if( pMenuCore->next_menu != MENU_NOTHING )
	{
		return;
	}

	cMenu_Base::Draw();

	// darken background
	cRect_Request *request = new cRect_Request();
	pVideo->Draw_Rect( NULL, 0.095f, &pMenuCore->handler->m_level->m_background_manager->Get_Pointer( 0 )->m_color_2, request );
	request->color.red = static_cast<Uint8>(request->color.red * 0.1f);
	request->color.green = static_cast<Uint8>(request->color.green * 0.1f);
	request->color.blue = static_cast<Uint8>(request->color.blue * 0.1f);
	request->color.alpha = 195;
	pRenderer->Add( request );

	Draw_End();
}

void cMenu_Credits :: Exit( void )
{
	// stop credits music
	pAudio->Fadeout_Music();
	pAudio->Play_Music( "game/menu.ogg", -1, 0, 1500 );

	// fade out
	Fade( 0 );

	// white background
	glClearColor( 1, 1, 1, 1 );

	// set menu gradient colors back
	pMenuCore->handler->m_level->m_background_manager->Get_Pointer( 0 )->m_color_1.alpha = 255;
	pMenuCore->handler->m_level->m_background_manager->Get_Pointer( 0 )->m_color_2.alpha = 255;

	pMenuCore->next_menu = MENU_MAIN;
	Game_Action = GA_ENTER_MENU;
}

void cMenu_Credits :: Fade( bool fadein /* = 1 */ )
{
	// fade counter
	float counter;
	// move speed
	float move_speed;

	if( fadein )
	{
		counter = 255;
		move_speed = -2;
	}
	else
	{
		counter = 60;
		move_speed = 2;
	}

	// get logo
	cSprite *logo = pMenuCore->handler->m_level->m_sprite_manager->Get_from_Position( 180, 20, TYPE_FRONT_PASSIVE );

	// fade out
	while( 1 )
	{
		// # Update

		if( fadein )
		{
			counter -= 4.5f * pFramerate->m_speed_factor;
			move_speed -= 1 * pFramerate->m_speed_factor;

			if( counter < 60 )
			{
				break;
			}

			// move logo out
			if( logo && logo->m_pos_y > -200 )
			{
				logo->Move( 0, move_speed );
			}
		}
		else
		{
			counter += 5 * pFramerate->m_speed_factor;
			move_speed += 1 * pFramerate->m_speed_factor;

			if( counter > 255 )
			{
				break;
			}

			// move logo in
			if( logo && logo->m_pos_y < 20 )
			{
				logo->Move( 0, move_speed );

				if( logo->m_pos_y > 20 )
				{
					logo->Set_Pos_Y( 20 );
				}
			}
		}

		// set menu gradient colors
		pMenuCore->handler->m_level->m_background_manager->Get_Pointer( 0 )->m_color_1.alpha = static_cast<Uint8>(counter);
		pMenuCore->handler->m_level->m_background_manager->Get_Pointer( 0 )->m_color_2.alpha = static_cast<Uint8>(counter);

		// # Draw

		// clear
		pVideo->Clear_Screen();

		// draw menu
		pMenuCore->handler->Draw();
		pMenuCore->pMenu_AnimManager->Draw();

		// create request
		cRect_Request *request = new cRect_Request();
		pVideo->Draw_Rect( NULL, 0.095f, &pMenuCore->handler->m_level->m_background_manager->Get_Pointer( 0 )->m_color_2, request );
		request->color.red = static_cast<Uint8>(request->color.red * 0.1f);
		request->color.green = static_cast<Uint8>(request->color.green * 0.1f);
		request->color.blue = static_cast<Uint8>(request->color.blue * 0.1f);
		request->color.alpha = 255 - static_cast<Uint8>(counter);
		// add request
		pRenderer->Add( request );

		pVideo->Render();

		// # framerate
		pFramerate->Update();
		// maximum fps
		Correct_Frame_Time( 100 );
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
