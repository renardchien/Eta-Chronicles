/***************************************************************************
 * editor.cpp  -  class for the basic editor
 *
 * Copyright (C) 2006 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/editor.h"
#include "../core/game_core.h"
#include "../core/framerate.h"
#include "../audio/audio.h"
#include "../video/font.h"
#include "../video/animation.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "../input/joystick.h"
#include "../user/preferences.h"
#include "../level/level.h"
#include "../player/player.h"
#include "../overworld/world_manager.h"
#include "../video/renderer.h"
#include "../core/sprite_manager.h"
#include "../overworld/overworld.h"
#include "../core/i18n.h"
#include "../core/filesystem/filesystem.h"
// boost filesystem
#include "boost/filesystem/convenience.hpp"
namespace fs = boost::filesystem;
// CEGUI
#include "CEGUIXMLParser.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cEditor_Object_Settings_Item *** *** *** *** *** *** *** *** *** *** */

cEditor_Object_Settings_Item :: cEditor_Object_Settings_Item( void )
{
	window_name = NULL;
	window_setting = NULL;
	advance_row = 1;
}

cEditor_Object_Settings_Item :: ~cEditor_Object_Settings_Item( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	wmgr.destroyWindow( window_name );
	wmgr.destroyWindow( window_setting );
}

/* *** *** *** *** *** *** *** *** cEditor_Item_Object *** *** *** *** *** *** *** *** *** */

cEditor_Item_Object :: cEditor_Item_Object( const std::string &text )
: ListboxItem( "" )
{
	list_text = new CEGUI::ListboxTextItem( reinterpret_cast<const CEGUI::utf8*>(text.c_str()) );

	sprite_obj = NULL;
	preview_scale = 1;
}

cEditor_Item_Object :: ~cEditor_Item_Object( void )
{
	delete list_text;

	if( sprite_obj )
	{
		delete sprite_obj;
	}
}

void cEditor_Item_Object :: Init( void )
{
	// CEGUI settings
	list_text->setTextColours( Get_Massive_Type_Color( sprite_obj->m_massive_type ).Get_cegui_Color() );
	list_text->setSelectionColours( CEGUI::colour( 0.33f, 0.33f, 0.33f ) );
	list_text->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );

	// image dimension text
	// string size_text = int_to_string( static_cast<int>(image->w) ) + "x" + int_to_string( static_cast<int>(image->h) );

	// get scale
	preview_scale = pVideo->Get_Scale( sprite_obj->m_start_image, static_cast<float>(pPreferences->m_editor_item_image_size) * 2.0f, static_cast<float>(pPreferences->m_editor_item_image_size) );

	// check if name is fitting
	if( sprite_obj->m_name.length() > 25 )
	{
		sprite_obj->m_name.erase( 25 );
		sprite_obj->m_name += "|";
	}

	// set position
	sprite_obj->Set_Pos_X( 20.0f, 1 );

	/* Don't set sprite settings which could get copied
	 * into the level if selected like shadow and z position
	*/
}

CEGUI::Size cEditor_Item_Object :: getPixelSize( void ) const
{
	CEGUI::Size tmp = list_text->getPixelSize();

	if( pPreferences->m_editor_show_item_images )
	{
		tmp.d_height += (pPreferences->m_editor_item_image_size + 10) * global_upscaley;
	}

	return tmp;
}

void cEditor_Item_Object :: draw( const CEGUI::Vector3 &position, float alpha, const CEGUI::Rect &clipper ) const
{
	// draw text
	list_text->draw( position, alpha, clipper );
}

void cEditor_Item_Object :: draw( CEGUI::RenderCache &cache, const CEGUI::Rect &targetRect, float zBase, float alpha, const CEGUI::Rect *clipper ) const
{
	// draw text
	list_text->draw( cache, targetRect, zBase, alpha, clipper );
}

void cEditor_Item_Object :: Draw_Image( void )
{
	// no image available to blit
	if( !sprite_obj->m_start_image || !pPreferences->m_editor_show_item_images )
	{
		return;
	}

	const CEGUI::Listbox *owner = static_cast<const CEGUI::Listbox *>( getOwnerWindow() );

	// if item is not visible
	if( !owner->isVisible() )
	{
		return;
	}

	// force valid draw
	sprite_obj->m_valid_draw = 1;

	// create request
	cSurface_Request *request = new cSurface_Request();

	// scale
	sprite_obj->m_start_scale_x = preview_scale;
	sprite_obj->m_start_scale_y = preview_scale;
	// set editor scale directions
	bool scale_up_orig = sprite_obj->m_scale_up;
	bool scale_down_orig = sprite_obj->m_scale_down;
	bool scale_left_orig = sprite_obj->m_scale_left;
	bool scale_right_orig = sprite_obj->m_scale_right;
	sprite_obj->Set_Scale_Directions( 0, 1, 0, 1 );
	// draw image
	sprite_obj->Draw_Image( request );
	// reset scale
	sprite_obj->m_start_scale_x = 1;
	sprite_obj->m_start_scale_y = 1;
	// set original scale directions
	sprite_obj->Set_Scale_Directions( scale_up_orig, scale_down_orig, scale_left_orig, scale_right_orig );

	// ignore camera
	request->no_camera = 1;
	// position z
	request->pos_z = 0.9f;

	// set shadow
	request->shadow_color = blackalpha128;
	request->shadow_pos = 2;

	// add request
	pRenderer_GUI->Add( request );
}

/* *** *** *** *** *** *** *** *** cEditor_Menu_Object *** *** *** *** *** *** *** *** *** */

cEditor_Menu_Object :: cEditor_Menu_Object( const std::string &text )
: ListboxTextItem( text.c_str() )
{
	bfunction = 0;
	header = 0;
}

cEditor_Menu_Object :: ~cEditor_Menu_Object( void )
{

}

void cEditor_Menu_Object :: Init( void )
{
	setSelectionColours( CEGUI::colour( 0.33f, 0.33f, 0.33f ) );
	setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
}

/* *** *** *** *** *** *** *** cEditor *** *** *** *** *** *** *** *** *** *** */

cEditor :: cEditor( void )
{
	enabled = 0;

	camera_speed = 35;
	menu_timer = 0;
	show_editor_help = 0;
	editor_window = NULL;
	listbox_menu = NULL;
	listbox_items = NULL;
	tabcontrol_menu = NULL;
}

cEditor :: ~cEditor( void )
{
	cEditor::Unload();
}

void cEditor :: Init( void )
{
	// already loaded
	if( editor_window )
	{
		return;
	}

	// Create Editor CEGUI Window
	editor_window = CEGUI::WindowManager::getSingleton().loadWindowLayout( "editor.layout" );
	pGuiSystem->getGUISheet()->addChildWindow( editor_window );

	// Get TabControl
	tabcontrol_menu = static_cast<CEGUI::TabControl *>(CEGUI::WindowManager::getSingleton().getWindow( "tabcontrol_editor" ));
	// fixme : CEGUI does not detect the mouse enter event if in the listbox or any other window in it
	// TabControl Menu Tab Events
	tabcontrol_menu->getTabContents( "editor_tab_menu" )->subscribeEvent( CEGUI::Window::EventMouseEnters, CEGUI::Event::Subscriber( &cEditor::Editor_Mouse_Enter, this ) );
	// TabControl Items Tab Events
	tabcontrol_menu->getTabContents( "editor_tab_items" )->subscribeEvent( CEGUI::Window::EventMouseEnters, CEGUI::Event::Subscriber( &cEditor::Editor_Mouse_Enter, this ) );

	// Get Menu Listbox
	listbox_menu = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editor_menu" ));
	// Menu Listbox events
	listbox_menu->subscribeEvent( CEGUI::Window::EventMouseEnters, CEGUI::Event::Subscriber( &cEditor::Editor_Mouse_Enter, this ) );
	listbox_menu->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &cEditor::Menu_Select, this ) );
	// Get Items Listbox
	listbox_items = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editor_items" ));
	// Items Listbox events
	listbox_items->subscribeEvent( CEGUI::Window::EventMouseEnters, CEGUI::Event::Subscriber( &cEditor::Editor_Mouse_Enter, this ) );
	listbox_items->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &cEditor::Item_Select, this ) );

	// Get Items
	if( !File_Exists( items_filename ) )
	{
		printf( "Error : Editor Loading : No Item file found : %s\n", items_filename.c_str() );
		return;
	}
	// Parse Items
	CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, items_filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/Editor_Items.xsd", "" );

	// Get all image items
	Load_Image_Items( DATA_DIR "/" GAME_PIXMAPS_DIR );

	// Get Menu
	if( !File_Exists( menu_filename ) )
	{
		printf( "Error : Editor Loading : No Menu file found : %s\n", menu_filename.c_str() );
		return;
	}
	// Parse Menu
	CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, menu_filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/Editor_Menu.xsd", "" );
}

void cEditor :: Unload( void )
{
	// Unload Items
	Unload_Item_Menu();

	// if editor window is loaded
	if( editor_window )
	{
		pGuiSystem->getGUISheet()->removeChildWindow( editor_window );
		CEGUI::WindowManager::getSingleton().destroyWindow( editor_window );
		editor_window = NULL;
		listbox_menu = NULL;
		listbox_items = NULL;
		tabcontrol_menu = NULL;
	}

	// Tagged Items
	for( TaggedItemObjectsList::iterator itr = tagged_item_objects.begin(), itr_end = tagged_item_objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	tagged_item_objects.clear();

	// Tagged Image Settings
	for( TaggedItemImageSettingsList::iterator itr = tagged_item_images.begin(), itr_end = tagged_item_images.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	tagged_item_images.clear();


	// Help Sprites
	for( HudSpriteList::iterator itr = help_sprites.begin(), itr_end = help_sprites.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	help_sprites.clear();
}

void cEditor :: Toggle( void )
{
	// enable
	if( !enabled )
	{
		Enable();
	}
	// disable
	else
	{
		Disable();
	}
}

void cEditor :: Enable( void )
{
	// already enabled
	if( enabled )
	{
		return;
	}

	// Draw Loading Text
	Draw_Static_Text( _("Loading"), &orange, NULL, 0 );

	// Basic Initialize
	Init();

	pAudio->Play_Sound( "editor/enter.ogg" );
	pHud_Debug->Set_Text( _("Editor enabled") );
	pAnimation_Manager->Delete_All();

	pMouseCursor->Set_Active( 1 );

	// update player position rect
	pActive_Player->Update_Position_Rect();
	// update sprite manager position rect
	for( cSprite_List::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Update_Position_Rect();
	}

	pActive_Camera->Update_Position();
	enabled = 1;
}

void cEditor :: Disable( bool native_mode /* = 1 */ )
{
	// already disabled
	if( !enabled )
	{
		return;
	}

	Unload();

	enabled = 0;
	// disable help screen
	show_editor_help = 0;

	if( native_mode )
	{
		pAudio->Play_Sound( "editor/leave.ogg" );

		// player
		pActive_Player->Update_Position_Rect();
		// sprite manager
		for( cSprite_List::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			(*itr)->Update_Position_Rect();
		}

		pActive_Camera->Center();
		pMouseCursor->Reset( 0 );

		// ask if editor should save
		Function_Save( 1 );
		
		pMouseCursor->Set_Active( 0 );
	}
}

void cEditor :: Update( void )
{
	if( !enabled )
	{
		return;
	}

	// if visible
	if( listbox_menu->isVisible( 1 ) )
	{
		// if timed out
		if( menu_timer >= speedfactor_fps * 2 )
		{
			// fade out
			float new_alpha = editor_window->getAlpha() - (pFramerate->m_speed_factor * 0.05f);

			if( new_alpha <= 0.0f )
			{
				new_alpha = 1.0f;

				/* move editor window out
				 * fixme: this could fire an mouse enter event if mouse is over it
				*/
				editor_window->setXPosition( CEGUI::UDim( -0.19f, 0.0f ) );
				// Hide Listbox
				listbox_menu->hide();
				listbox_items->hide();
				menu_timer = 0.0f;
			}

			editor_window->setAlpha( new_alpha );
		}
		// if active
		else
		{
			// fade in
			if( editor_window->getAlpha() < 1.0f )
			{
				float new_alpha = editor_window->getAlpha() + pFramerate->m_speed_factor * 0.1f;

				if( new_alpha > 1.0f )
				{
					new_alpha = 1.0f;
				}

				editor_window->setAlpha( new_alpha );
			}
			// inactive counter
			else if( Is_Float_Equal( editor_window->getXPosition().asRelative( 1 ), 0.0f ) )
			{
				// if mouse is over the window
				if( tabcontrol_menu->isHit( CEGUI::MouseCursor::getSingletonPtr()->getPosition() ) )
				{
					menu_timer = 0.0f;
				}
				// inactive
				else
				{
					menu_timer += pFramerate->m_speed_factor;
				}
			}
		}
	}

	pMouseCursor->Editor_Update();
}

void cEditor :: Draw( void )
{
	if( !enabled )
	{
		return;
	}

	Color color;

	// Camera limit bottom line
	if( pActive_Camera->y > pActive_Camera->limit_rect.m_y && pActive_Camera->y + pActive_Camera->limit_rect.m_y < game_res_h )
	{
		int start_x = 0;

		if( pActive_Camera->x < 0 )
		{
			start_x = -static_cast<int>(pActive_Camera->x);
		}

		color = Color( static_cast<Uint8>(0), 0, 100, 192 );
		pVideo->Draw_Line( static_cast<float>(start_x), static_cast<float>(game_res_h - static_cast<int>(pActive_Camera->y)), static_cast<float>(game_res_w), static_cast<float>( game_res_h - static_cast<int>(pActive_Camera->y) ), 0.124f, &color );
	}
	// Camera limit top line
	if( pActive_Camera->y + game_res_h > pActive_Camera->limit_rect.m_y + pActive_Camera->limit_rect.m_h && pActive_Camera->y < pActive_Camera->limit_rect.m_y + pActive_Camera->limit_rect.m_h )
	{
		int start_x = 0;

		if( pActive_Camera->x < pActive_Camera->limit_rect.m_x )
		{
			start_x = -static_cast<int>(pActive_Camera->x);
		}

		color = Color( static_cast<Uint8>(20), 20, 150, 192 );
		pVideo->Draw_Line( static_cast<float>(start_x), ( pActive_Camera->limit_rect.m_y + pActive_Camera->limit_rect.m_h ) - pActive_Camera->y, static_cast<float>(game_res_w), ( pActive_Camera->limit_rect.m_y + pActive_Camera->limit_rect.m_h ) - pActive_Camera->y, 0.124f, &color );
	}

	// Camera limit left line
	if( pActive_Camera->x < 0 && pActive_Camera->x > -game_res_w )
	{
		int start_y = game_res_h;

		if( pActive_Camera->y < game_res_h )
		{
			start_y = game_res_h - static_cast<int>(pActive_Camera->y);
		}

		color = Color( static_cast<Uint8>(0), 100, 0, 192 );
		pVideo->Draw_Line( static_cast<float>( 0 - pActive_Camera->x ), static_cast<float>(start_y), static_cast<float>( -pActive_Camera->x ), 0, 0.124f, &color );
	}
	// Camera limit right line
	if( pActive_Camera->x < pActive_Camera->limit_rect.m_x + pActive_Camera->limit_rect.m_w && pActive_Camera->x > -game_res_w + pActive_Camera->limit_rect.m_x + pActive_Camera->limit_rect.m_w )
	{
		int start_y = game_res_h;

		if( pActive_Camera->y < game_res_h )
		{
			start_y = game_res_h - static_cast<int>(pActive_Camera->y);
		}

		color = Color( static_cast<Uint8>(20), 150, 20, 192 );
		pVideo->Draw_Line( static_cast<float>( pActive_Camera->limit_rect.m_x + pActive_Camera->limit_rect.m_w - pActive_Camera->x ), static_cast<float>(start_y), static_cast<float>( pActive_Camera->limit_rect.m_x + pActive_Camera->limit_rect.m_w - pActive_Camera->x ), 0, 0.124f, &color );
	}

	// if editor window is active
	if( editor_window->getXPosition().asRelative( 1 ) >= 0 )
	{
		// Listbox dimension
		float list_posy = listbox_items->getUnclippedPixelRect().d_top * global_downscaley;
		float list_height = listbox_items->getUnclippedPixelRect().getHeight() * global_downscaley;
		// Vertical ScrollBar Position
		float scroll_pos = listbox_items->getVertScrollbar()->getScrollPosition() * global_downscaley;
		// font height
		float font_height = CEGUI::FontManager::getSingleton().getFont( "bluebold_medium" )->getFontHeight() * global_downscaley;

		// draw items
		for( unsigned int i = 0; i < listbox_items->getItemCount(); i++ )
		{
			// Get item
			cEditor_Item_Object *item = static_cast<cEditor_Item_Object *>( listbox_items->getListboxItemFromIndex( i ) );
			// Item height
			float item_height = item->getPixelSize().d_height * global_downscaley;
			// Item position
			float item_posy = list_posy + ( item_height * i );
			float item_image_posy = item_posy + ( font_height * 2 );

			// not visible
			if( item_posy + item_height > list_posy + list_height + scroll_pos || item_image_posy < list_posy + scroll_pos )
			{
				continue;
			}

			item->sprite_obj->Set_Pos_Y( item_image_posy - scroll_pos, 1 );
			item->Draw_Image();
		}
	}

	if( show_editor_help )
	{
		Draw_Editor_Help();
	}
}

void cEditor :: Process_Input( void )
{
	if( !enabled )
	{
		return;
	}

	// Drag Delete
	if( ( pKeyboard->keys[SDLK_RCTRL] || pKeyboard->keys[SDLK_LCTRL] ) && pMouseCursor->m_right )
	{
		cObjectCollision *col = pMouseCursor->Get_First_Editor_Collsion();

		if( col )
		{
			pMouseCursor->Delete( col->obj );
			delete col;
		}
	}

	// Camera Movement
	if( pKeyboard->keys[SDLK_RIGHT] || pJoystick->right )
	{
		if( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] )
		{
			pActive_Camera->Move( camera_speed * pFramerate->m_speed_factor * 3 * pPreferences->m_scroll_speed, 0 );
		}
		else
		{
			pActive_Camera->Move( camera_speed * pFramerate->m_speed_factor * pPreferences->m_scroll_speed, 0 );
		}
	}
	else if( pKeyboard->keys[SDLK_LEFT] || pJoystick->left )
	{
		if( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] )
		{
			pActive_Camera->Move( -( camera_speed * pFramerate->m_speed_factor * 3 * pPreferences->m_scroll_speed ), 0 );
		}
		else
		{
			pActive_Camera->Move( -( camera_speed * pFramerate->m_speed_factor * pPreferences->m_scroll_speed ), 0 );
		}
	}
	if( pKeyboard->keys[SDLK_UP] || pJoystick->up )
	{
		if( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] )
		{
			pActive_Camera->Move( 0, -( camera_speed * pFramerate->m_speed_factor * 3 * pPreferences->m_scroll_speed ) );
		}
		else
		{
			pActive_Camera->Move( 0, -( camera_speed * pFramerate->m_speed_factor * pPreferences->m_scroll_speed ) );
		}
	}
	else if( pKeyboard->keys[SDLK_DOWN] || pJoystick->down )
	{
		if( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] )
		{
			pActive_Camera->Move( 0, camera_speed * pFramerate->m_speed_factor * 3 * pPreferences->m_scroll_speed );
		}
		else
		{
			pActive_Camera->Move( 0, camera_speed * pFramerate->m_speed_factor * pPreferences->m_scroll_speed );
		}
	}
}

bool cEditor :: Handle_Event( SDL_Event *ev )
{
	if( !enabled )
	{
		return 0;
	}

	switch( ev->type )
	{
	case SDL_MOUSEMOTION:
	{
		if( pMouseCursor->m_mover_mode )
		{
			pMouseCursor->Mover_Update( input_event.motion.xrel, input_event.motion.yrel );
		}

		break;
	}
	default: // other events
	{
		break;
	}
	}

	return 0;
}

bool cEditor :: Key_Down( SDLKey key )
{
	if( !enabled )
	{
		return 0;
	}

	if( key == SDLK_F1 )
	{
		show_editor_help = !show_editor_help;
	}
	// focus level start
	else if( key == SDLK_HOME )
	{
		pActive_Camera->Set_Pos( 0, 0 );
	}
	// move the screen size to the right
	else if( key == SDLK_n )
	{
		pActive_Camera->Move( static_cast<float>(game_res_w), 0 );
	}
	// move the screen size to the left
	else if( key == SDLK_p )
	{
		pActive_Camera->Move( -static_cast<float>(game_res_w), 0 );
	}
	// push selected objects into the front
	else if( key == SDLK_KP_PLUS )
	{
		for( SelectedObjectList::iterator itr = pMouseCursor->m_selected_objects.begin(), itr_end = pMouseCursor->m_selected_objects.end(); itr != itr_end; ++itr )
		{
			cSelectedObject *sel_obj = (*itr);

			Change_Draw_Position( sel_obj->obj, 0 );
		}
	}
	// push selected objects into the back
	else if( key == SDLK_KP_MINUS )
	{
		for( SelectedObjectList::iterator itr = pMouseCursor->m_selected_objects.begin(), itr_end = pMouseCursor->m_selected_objects.end(); itr != itr_end; ++itr )
		{
			cSelectedObject *sel_obj = (*itr);

			Change_Draw_Position( sel_obj->obj, 1 );
		}
	}
	// Precise Pixel-Positioning or copy into direction
	// only with a hovering object functions
	if( ( key == SDLK_KP2 || key == SDLK_KP4 || key == SDLK_KP6 || key == SDLK_KP8 ) && pMouseCursor->m_hovering_object->obj )
	{
		if( pMouseCursor->m_fastcopy_mode )
		{
			ObjectDirection dir = DIR_UNDEFINED;

			// down
			if( key == SDLK_KP2 )
			{
				dir = DIR_DOWN;
			}
			// left
			else if( key == SDLK_KP4 )
			{
				dir = DIR_LEFT;
			}
			// right
			else if( key == SDLK_KP6 )
			{
				dir = DIR_RIGHT;
			}
			// up
			else if( key == SDLK_KP8 )
			{
				dir = DIR_UP;
			}

			// get currently selected objects
			cSprite_List objects = pMouseCursor->Get_Selected_Objects();
			// copy objects
			cSprite_List new_objects = Copy_Direction( objects, dir );

			// add new objects
			for( cSprite_List::iterator itr = new_objects.begin(), itr_end = new_objects.end(); itr != itr_end; ++itr )
			{
				cSprite *obj = (*itr);

				pMouseCursor->Add_Selected_Object( obj, 1 );
			}
			
			// deselect old objects
			for( cSprite_List::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
			{
				cSprite *obj = (*itr);

				pMouseCursor->Remove_Selected_Object( obj );
			}
		}
		else
		{
			// down
			if( key == SDLK_KP2 )
			{
				pActive_Camera->Move( 0, 1 );
			}
			// left
			else if( key == SDLK_KP4 )
			{
				pActive_Camera->Move( -1, 0 );
			}
			// right
			else if( key == SDLK_KP6 )
			{
				pActive_Camera->Move( 1, 0 );
			}
			// up
			else if( key == SDLK_KP8 )
			{
				pActive_Camera->Move( 0, -1 );
			}
		}
	}
	// deselect everything
	else if( key == SDLK_a && input_event.key.keysym.mod & KMOD_CTRL && input_event.key.keysym.mod & KMOD_SHIFT )
	{
		pMouseCursor->Clear_Selected_Objects();
	}
	// select everything
	else if( key == SDLK_a && input_event.key.keysym.mod & KMOD_CTRL )
	{
		pMouseCursor->Clear_Selected_Objects();

		// player
		pMouseCursor->Add_Selected_Object( pActive_Player, 1 );
		// sprite manager
		for( cSprite_List::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			cSprite *obj = (*itr);

			pMouseCursor->Add_Selected_Object( obj, 1 );
		}
	}
	// Paste copy buffer objects
	else if( key == SDLK_INSERT || ( key == SDLK_v && input_event.key.keysym.mod & KMOD_CTRL ) )
	{
		pMouseCursor->Paste_Copy_Objects( static_cast<float>(static_cast<int>(pMouseCursor->m_pos_x)), static_cast<float>(static_cast<int>(pMouseCursor->m_pos_y)) );
	}
	// Cut selected Sprites to the copy buffer
	else if( key == SDLK_x && input_event.key.keysym.mod & KMOD_CTRL )
	{
		pMouseCursor->Clear_Copy_Objects();

		for( SelectedObjectList::iterator itr = pMouseCursor->m_selected_objects.begin(), itr_end = pMouseCursor->m_selected_objects.end(); itr != itr_end; ++itr )
		{
			cSelectedObject *sel_obj = (*itr);

			pMouseCursor->Add_Copy_Object( sel_obj->obj );
		}

		pMouseCursor->Delete_Selected_Objects();
	}
	// Add selected Sprites to the copy buffer
	else if( key == SDLK_c && input_event.key.keysym.mod & KMOD_CTRL )
	{
		pMouseCursor->Clear_Copy_Objects();

		for( SelectedObjectList::iterator itr = pMouseCursor->m_selected_objects.begin(), itr_end = pMouseCursor->m_selected_objects.end(); itr != itr_end; ++itr )
		{
			cSelectedObject *sel_obj = (*itr);

			pMouseCursor->Add_Copy_Object( sel_obj->obj );
		}
	}
	// Delete mouse object
	else if( key == SDLK_DELETE && pMouseCursor->m_hovering_object->obj )
	{
		pMouseCursor->Delete( pMouseCursor->m_hovering_object->obj );
	}
	// if shift got pressed remove mouse object for possible mouse selection
	else if( ( key == SDLK_RSHIFT || key == SDLK_LSHIFT ) && pMouseCursor->m_hovering_object->obj )
	{
		pMouseCursor->Clear_Mouse_Object();
	}
	// Delete selected objects
	else if( key == SDLK_DELETE )
	{
		pMouseCursor->Delete_Selected_Objects();
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cEditor :: Mouse_Down( Uint8 button )
{
	if( !enabled )
	{
		return 0;
	}

	// left
	if( button == SDL_BUTTON_LEFT )
	{
		pMouseCursor->Click();
		
		// auto hide if enabled
		if( pMouseCursor->m_hovering_object->obj && pPreferences->m_editor_mouse_auto_hide )
		{
			pMouseCursor->Set_Active( 0 );
		}
	}
	// middle
	else if( button == SDL_BUTTON_MIDDLE )
	{
		// Activate fast copy mode
		if( pMouseCursor->m_hovering_object->obj )
		{
			pMouseCursor->m_fastcopy_mode = 1;
			return 1;
		}
		// Mover mode
		else
		{
			pMouseCursor->Toggle_Mover_Mode();
			return 1;
		}
	}
	// right
	else if( button == SDL_BUTTON_RIGHT )
	{
		if( !pMouseCursor->m_left )
		{
			pMouseCursor->Delete( pMouseCursor->m_hovering_object->obj );
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

bool cEditor :: Mouse_Up( Uint8 button )
{
	if( !enabled )
	{
		return 0;
	}

	// left
	if( button == SDL_BUTTON_LEFT )
	{
		pMouseCursor->End_Selection();

		// unhide
		if( pPreferences->m_editor_mouse_auto_hide )
		{
			pMouseCursor->Set_Active( 1 );
		}
	}
	// middle
	else if( button == SDL_BUTTON_MIDDLE )
	{
		pMouseCursor->m_fastcopy_mode = 0;
	}
	else
	{
		// not processed
		return 0;
	}

	// button got processed
	return 1;
}

void cEditor :: Add_Menu_Object( const std::string &name, std::string tags, CEGUI::colour normal_color /* = CEGUI::colour( 1, 1, 1 ) */ )
{
	// Create Menu Object
	cEditor_Menu_Object *new_menu = new cEditor_Menu_Object( name );

	// if function
	if( tags.find( "function:" ) == 0 )
	{
		new_menu->bfunction = 1;
		// cut out the function identifier
		tags.erase( 0, 9 );
	}

	// if header
	if( tags.find( "header" ) == 0 )
	{
		new_menu->header = 1;
		// cut out the function identifier
		tags.erase( 0, 6 );

		// header color rect
		new_menu->setTextColours( normal_color, normal_color, CEGUI::colour( 0.5f, 0.5f, 0.5f ), CEGUI::colour( 0.5f, 0.5f, 0.5f ) );
		// not selectable
		new_menu->setDisabled( 1 );
		// set tooltip
		new_menu->setTooltipText( "Header " + name );
	}
	// if not a header
	else
	{
		new_menu->setTextColours( normal_color );
	}

	// if default items menu
	if( !new_menu->bfunction && !new_menu->header )
	{
		// set tooltip
		new_menu->setTooltipText( "Tags used " + tags );
	}

	new_menu->tags = tags;
	new_menu->Init();

	// Add Listbox item
	listbox_menu->addItem( new_menu );
}

void cEditor :: Activate_Menu_Item( cEditor_Menu_Object *entry )
{
	// Function
	if( entry->bfunction )
	{
		if( entry->tags.compare( "exit" ) == 0 )
		{
			Function_Exit();
		}
		else
		{
			printf( "Unknown Function %s\n", entry->tags.c_str() );
		}
	}
	// Header
	else if( entry->header )
	{
		return;
	}
	// Item Menu
	else
	{
		if( Load_Item_Menu( entry->tags ) )
		{
			// Select Items Tab
			tabcontrol_menu->setSelectedTab( "editor_tab_items" );
		}
		// failed
		else
		{
			printf( "Unknown Menu Type %s\n", entry->tags.c_str() );
		}
	}
}

bool cEditor :: Load_Item_Menu( std::string item_tags )
{
	if( item_tags.empty() )
	{
		return 0;
	}

	Unload_Item_Menu();

	// Convert to Array Tags
	vector<std::string> array_tags;

	// Convert
	while( item_tags.length() )
	{
		std::string::size_type pos = item_tags.find( ";" );

		// last item
		if( pos == std::string::npos )
		{
			array_tags.push_back( item_tags );
			item_tags.clear();
			break;
		}

		// add tag
		array_tags.push_back( item_tags.substr( 0, pos ) );
		// remove tag
		item_tags.erase( 0, pos + 1 );
	}

	unsigned int tag_pos = 0;

	// Get all Images with the Tags
	for( TaggedItemImageSettingsList::iterator itr = tagged_item_images.begin(), itr_end = tagged_item_images.end(); itr != itr_end; ++itr )
	{
		cImage_settings_data *settings = (*itr);

		// search
		while( Is_Tag_Available( settings->m_editor_tags, array_tags[tag_pos] ) )
		{
			tag_pos++;

			// found all tags
			if( tag_pos >= array_tags.size() )
			{
				cGL_Surface *image = pVideo->Get_Surface( settings->m_base );
				// Create sprite
				cSprite *new_sprite = new cSprite( image );
				// default massivetype
				new_sprite->Set_Sprite_Type( static_cast<SpriteType>(image->m_type) );
				// Add new Sprite
				Add_Item_Object( new_sprite );

				break;
			}
		}

		tag_pos = 0;
	}

	// Get all Objects with the Tags
	for( TaggedItemObjectsList::iterator itr = tagged_item_objects.begin(), itr_end = tagged_item_objects.end(); itr != itr_end; ++itr )
	{
		cSprite *object = (*itr);

		// search
		while( Is_Tag_Available( object->m_editor_tags, array_tags[tag_pos] ) )
		{
			tag_pos++;

			// found all tags
			if( tag_pos >= array_tags.size() )
			{
				// Add Objects
				Add_Item_Object( object->Copy() );

				break;
			}
		}

		tag_pos = 0;
	}

	return 1;
}

void cEditor :: Unload_Item_Menu( void )
{
	// already unloaded
	if( !CEGUI::WindowManager::getSingleton().isWindowPresent( "editor_items" ) )
	{
		return;
	}

	// Clear Listbox
	if( listbox_items )
	{
		listbox_items->resetList();
	}
}


void cEditor :: Add_Item_Object( cSprite *sprite, std::string new_name /* = "" */, cGL_Surface *image /* = NULL */ )
{
	// if invalid
	if( !sprite )
	{
		printf( "Warning : Invalid Editor Item\n" );
		return;
	}

	// set correct array if not given
	if( sprite->m_sprite_array == ARRAY_UNDEFINED )
	{
		printf( "Warning : Editor sprite %s array not set\n", sprite->m_name.c_str() );

		if( sprite->m_massive_type == MASS_PASSIVE )
		{
			sprite->m_sprite_array = ARRAY_PASSIVE;
		}
		else if( sprite->m_massive_type == MASS_MASSIVE )
		{
			sprite->m_sprite_array = ARRAY_MASSIVE;
		}
		else if( sprite->m_massive_type == MASS_HALFMASSIVE )
		{
			sprite->m_sprite_array = ARRAY_ACTIVE;
		}
	}

	// set correct type if not given
	if( sprite->m_type == TYPE_UNDEFINED )
	{
		printf( "Warning : Editor sprite %s type not set\n", sprite->m_name.c_str() );

		if( sprite->m_massive_type == MASS_PASSIVE )
		{
			sprite->m_type = TYPE_PASSIVE;
		}
		else if( sprite->m_massive_type == MASS_MASSIVE )
		{
			sprite->m_type = TYPE_MASSIVE;
		}
		else if( sprite->m_massive_type == MASS_HALFMASSIVE )
		{
			sprite->m_type = TYPE_HALFMASSIVE;
		}
		else if( sprite->m_massive_type == MASS_CLIMBABLE )
		{
			sprite->m_type = TYPE_CLIMBABLE;
		}
	}

	// if no image is given use the sprite start image
	if( !image )
	{
		// special object
		if( sprite->m_type == TYPE_ENEMY_STOPPER || sprite->m_type == TYPE_LEVEL_EXIT || sprite->m_type == TYPE_LEVEL_ENTRY || sprite->m_type == TYPE_SOUND || sprite->m_type == TYPE_ANIMATION || sprite->m_type == TYPE_PATH )
		{
			sprite->m_image = pVideo->Get_Surface( "game/editor/special.png" );
			sprite->m_start_image = sprite->m_image;
		}

		image = sprite->m_start_image;
	}

	// set object name
	std::string obj_name;

	if( new_name.length() )
	{
		obj_name = new_name;
	}
	else if( sprite->m_name.length() )
	{
		obj_name = sprite->m_name;
	}
	// no object name available
	else
	{
		if( image )
		{
			obj_name = image->Get_Filename( 0, 0 );
		}

		// Warn if using filename
		printf( "Warning : editor object %s with no name given\n", obj_name.c_str() );
	}

	cEditor_Item_Object *new_item = new cEditor_Item_Object( obj_name );

	// object pointer
	new_item->sprite_obj = sprite;

	// Initialize
	new_item->Init();

	// Add Item
	listbox_items->addItem( new_item );
}

void cEditor :: Load_Image_Items( std::string dir )
{
	vector<std::string> image_files = Get_Directory_Files( dir, ".settings" );

	// load all available objects
	for( vector<std::string>::iterator itr = image_files.begin(), itr_end = image_files.end(); itr != itr_end; ++itr )
	{
		// get filename
		std::string filename = (*itr);

		// load settings
		cImage_settings_data *settings = pSettingsParser->Get( filename );

		// if settings are available
		if( settings )
		{
			// if required editor tag is available
			if( settings->m_editor_tags.find( editor_item_tag ) != std::string::npos )
			{
				// set base to the filename
				settings->m_base = filename;
				// add real image
				tagged_item_images.push_back( settings );
			}
		}
	}
}

void cEditor :: Activate_Item( cEditor_Item_Object *entry )
{
	// invalid
	if( !entry )
	{
		printf( "Error : Invalid Editor Item\n" );
		return;
	}

	// create copy from editor item
	cSprite *new_sprite = entry->sprite_obj->Copy();

	// if copying failed
	if( !new_sprite )
	{
		printf( "Error : Editor Sprite %s copy failed\n", entry->sprite_obj->m_name.c_str() );
		return;
	}
	
	new_sprite->Set_Pos( pMouseCursor->m_pos_x, pMouseCursor->m_pos_y, 1 );

	// hide editor window
	editor_window->setXPosition( CEGUI::UDim( -0.19f, 0 ) );
	// Hide Listbox
	listbox_menu->hide();
	listbox_items->hide();

	// add item
	pActive_Sprite_Manager->Add( new_sprite );

	// Set Mouse Objects
	pMouseCursor->m_left = 1;
	pMouseCursor->m_hovering_object->mouse_h = static_cast<int>( new_sprite->m_col_rect.m_h / 2 );
	pMouseCursor->m_hovering_object->mouse_w = static_cast<int>( new_sprite->m_col_rect.m_w / 2 );
	pMouseCursor->Set_Mouse_Object( new_sprite );
}

bool cEditor :: Is_Tag_Available( const std::string &str, const std::string &tag, unsigned int search_pos /* = 0 */ )
{
	// found tag position
	std::string::size_type pos = str.find( tag, search_pos );

	// not found
	if( pos == std::string::npos )
	{
		return 0;
	}

	// tag end position
	std::string::size_type end_pos = pos + tag.length();

	// if tag starting position is valid
	if( pos == 0 || str.substr( pos - 1, 1 ).compare( ";" ) == 0  )
	{
		// if tag ending position is valid
		if( end_pos == str.length() || str.substr( end_pos, 1 ).compare( ";" ) == 0  )
		{
			return 1;
		}
	}

	// not valid - continue search
	return Is_Tag_Available( str, tag, end_pos );
}

void cEditor :: Draw_Editor_Help( void )
{
	// Help Window Background Rect
	pVideo->Draw_Rect( 50, 5, 700, 565, 0.58f, &blackalpha192 );

	// no help text set
	if( help_sprites.empty() )
	{
		// Add/Create the Help Text
		// todo : create a CEGUI help box with tabs and translate with gettext then
		Add_Help_Line( "Editor Help", "", 5, 300 );
		Add_Help_Line( "F1", "Toggle this Help Window" );
		Add_Help_Line( "F8", "Open / Close the Level editor" );
		Add_Help_Line( "F10", "Toggle sound effects" );
		Add_Help_Line( "F11", "Toggle music play" );
		Add_Help_Line( "Home", "Focus level start" );
		Add_Help_Line( "End", "Focus last level exit" );
		Add_Help_Line( "Ctrl + L", "Load a Level" );
		Add_Help_Line( "Ctrl + W", "Load an Overworld" );
		Add_Help_Line( "N", "Step one screen to the right ( Next Screen )" );
		Add_Help_Line( "P", "Step one screen to the left ( Previous Screen )" );
		Add_Help_Line( "M", "Cycle selected object(s) through massive types" );
		Add_Help_Line( "Massive types (color):" );
		Add_Help_Line( "Massive(red) ->   Halfmassive(orange) ->   Climbable(lila) ->   Passive(green) ->   Front Passive(green)", "" , 0, 80 );
		Add_Help_Line( "Ctrl + S", "Save the current Level" );
		Add_Help_Line( "Ctrl + D", "Toggle debug mode" );
		Add_Help_Line( "Ctrl + P", "Toggle performance mode" );
		Add_Help_Line( "Ctrl + A", "Select all objects" );
		Add_Help_Line( "Ctrl + X", "Cut currently selected objects" );
		Add_Help_Line( "Ctrl + C", "Copy currently selected objects" );
		Add_Help_Line( "Ctrl + V", "Paste current copied / cutted objects" );
		Add_Help_Line( "Insert", "Paste current copied / cutted objects" );
		Add_Help_Line( "Del", "If Mouse is over an object: Delete current object" );
		Add_Help_Line( "Del", "If Mouse has nothing selected: Delete selected objects" );
		Add_Help_Line( "Numpad:" );
		Add_Help_Line( " +", "Bring object to front" );
		Add_Help_Line( " -", "Send object to back" );
		Add_Help_Line( " 2/4/6/8", "Move selected object 1 pixel into the direction" );
		Add_Help_Line( "Mouse:" );
		Add_Help_Line( " Left (Hold)", "Drag objects" );
		Add_Help_Line( " Left (Click)", "With shift to select / deselect single objects" );
		Add_Help_Line( " Right", "Delete intersecting Object" );
		Add_Help_Line( " Middle", "Toggle Mover Mode" );
		Add_Help_Line( "Arrow keys:" );
		Add_Help_Line( " Use arrow keys to move around. Press shift for faster movement" );
	}

	// draw
	for( HudSpriteList::iterator itr = help_sprites.begin(), itr_end = help_sprites.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}
}

void cEditor :: Add_Help_Line( std::string key_text, std::string description /* = "" */, float spacing /* = 0 */, float pos_x /* = 60 */ )
{
	// create help sprites
	cHudSprite *help_sprite_key_text = new cHudSprite();
	cHudSprite *help_sprite_text = new cHudSprite();
	// with shadow
	help_sprite_key_text->Set_Shadow( black, 0.5f );
	help_sprite_text->Set_Shadow( black, 0.5f );
	// position in front
	help_sprite_key_text->m_pos_z = 0.591f;
	help_sprite_text->m_pos_z = 0.59f;

	// Set Y position
	float pos_y = spacing;
	// if not the first help sprite use the last position
	if( !help_sprites.empty() )
	{
		// get last help sprite
		cHudSprite *last_hud_sprite = help_sprites[ help_sprites.size() - 1 ];
		// set correct position
		pos_y += last_hud_sprite->m_pos_y + last_hud_sprite->m_rect.m_h;
	}
	// first item
	else
	{
		pos_y += 5;
	}

	// text must be filled with something to get created by Render_Text
	if( key_text.empty() )
	{
		key_text = " ";
	}
	if( description.empty() )
	{
		description = " ";
	}

	// set key text
	help_sprite_key_text->Set_Image( pFont->Render_Text( pFont->m_font_very_small, key_text, lightorange ), 0, 1 );
	help_sprite_key_text->Set_Pos( pos_x, pos_y, 1 );
	// set text
	help_sprite_text->Set_Image( pFont->Render_Text( pFont->m_font_very_small, description, white ), 0, 1 );
	help_sprite_text->Set_Pos( pos_x + 90, pos_y, 1 );

	// add to array
	help_sprites.push_back( help_sprite_key_text );
	help_sprites.push_back( help_sprite_text );
}

void cEditor :: Change_Draw_Position( cSprite *obj, bool move_back ) const
{
	// empty object
	if( !obj )
	{
		return;
	}

	// check if valid object
	if( !( obj->m_sprite_array == ARRAY_MASSIVE || obj->m_sprite_array == ARRAY_PASSIVE || obj->m_sprite_array == ARRAY_ACTIVE ) )
	{
		return;
	}

	// move to front
	if( !move_back )
	{
		// last object is drawn first
		cSprite *last = pActive_Sprite_Manager->objects.back();

		// if already in front
		if( obj == last )
		{
			return;
		}

		pActive_Sprite_Manager->Delete( obj, 0 );
		pActive_Sprite_Manager->objects.back() = obj;
		pActive_Sprite_Manager->objects.insert( pActive_Sprite_Manager->objects.end() - 1, last );

		obj->m_pos_z = pActive_Sprite_Manager->Get_Last( obj->m_type )->m_pos_z + 0.000001f;
	}
	// move to back
	else
	{
		// first object is drawn last
		cSprite *first = pActive_Sprite_Manager->objects.front();

		// if already in back
		if( obj == first )
		{
			return;
		}

		pActive_Sprite_Manager->Delete( obj, 0 );
		pActive_Sprite_Manager->objects.front() = obj;
		pActive_Sprite_Manager->objects.insert( pActive_Sprite_Manager->objects.begin() + 1, first );

		obj->m_pos_z = pActive_Sprite_Manager->Get_First( obj->m_type )->m_pos_z - 0.000001f;
	}
}

cSprite_List cEditor :: Copy_Direction( cSprite_List &objects, const ObjectDirection dir ) const
{
	// additional direction objects offset
	unsigned int offset = 0;

	// get the objects difference offset
	if( dir == DIR_LEFT || dir == DIR_RIGHT )
	{
		// first object
		cSprite *first = objects[0];

		for( unsigned int i = 1; i < objects.size(); i++ )
		{
			if( objects[i]->m_start_pos_x < first->m_start_pos_x )
			{
				first = objects[i];
			}
		}

		// last object
		cSprite *last = objects[0];

		for( unsigned int i = 1; i < objects.size(); i++ )
		{
			if( objects[i]->m_start_pos_x + objects[i]->m_start_rect.m_w > last->m_start_pos_x + last->m_start_rect.m_w )
			{
				last = objects[i];
			}
		}

		// Set X offset
		offset = static_cast<int>( last->m_start_pos_x - first->m_start_pos_x + last->m_start_rect.m_w );
	}
	else if( dir == DIR_UP || dir == DIR_DOWN )
	{
		// first object
		cSprite *first = objects[0];

		for( unsigned int i = 1; i < objects.size(); i++ )
		{
			if( objects[i]->m_start_pos_y < first->m_start_pos_y )
			{
				first = objects[i];
			}
		}

		// last object
		cSprite *last = objects[0];

		for( unsigned int i = 1; i < objects.size(); i++ )
		{
			if( objects[i]->m_start_pos_y + objects[i]->m_start_rect.m_h > last->m_start_pos_y + last->m_start_rect.m_h )
			{
				last = objects[i];
			}
		}

		// Set Y offset
		offset = static_cast<int>( last->m_start_pos_y - first->m_start_pos_y + last->m_start_rect.m_h );
	}

	// new copied objects
	cSprite_List new_objects;

	for( cSprite_List::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		new_objects.push_back( Copy_Direction( obj, dir, offset ) );
	}

	// return only new objects
	return new_objects;
}

cSprite *cEditor :: Copy_Direction( cSprite *obj, const ObjectDirection dir, int offset /* = 0 */ ) const
{
	float w = 0, h = 0;

	if( dir == DIR_LEFT )
	{
		if( offset )
		{
			w = -static_cast<float>(offset);
		}
		else
		{
			w = -obj->m_start_rect.m_w;
		}
	}
	else if( dir == DIR_RIGHT )
	{
		if( offset )
		{
			w = static_cast<float>(offset);
		}
		else
		{
			w = obj->m_start_rect.m_w;
		}
	}
	else if( dir == DIR_UP )
	{
		if( offset )
		{
			h = -static_cast<float>(offset);
		}
		else
		{
			h = -obj->m_start_rect.m_h;
		}
	}
	else if( dir == DIR_DOWN )
	{
		if( offset )
		{
			h = static_cast<float>(offset);
		}
		else
		{
			h = obj->m_start_rect.m_h;
		}
	}

	// only move camera if obj is the mouse object
	if( pMouseCursor->m_hovering_object->obj == obj )
	{
		pActive_Camera->Move( w, h );
	}

	return pMouseCursor->Copy( obj, obj->m_start_pos_x + w, obj->m_start_pos_y + h );
}

bool cEditor :: Editor_Mouse_Enter( const CEGUI::EventArgs &event )
{
	// ignore if a button is pressed
	if( pMouseCursor->m_left || pMouseCursor->m_middle || pMouseCursor->m_right )
	{
		return 1;
	}

	// if not visible
	if( !listbox_items->isVisible( 1 ) )
	{
		// fade in
		editor_window->setXPosition( CEGUI::UDim( 0, 0 ) );
		editor_window->setAlpha( 0.0f );

		// Show Listbox
		listbox_menu->show();
		listbox_items->show();
	}
	
	menu_timer = 0.0f;

	return 1;
}

bool cEditor :: Menu_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// set item
	if( item )
	{
		Activate_Menu_Item( static_cast<cEditor_Menu_Object *>(item) );
	}
	// clear ?
	else
	{
		// todo : clear
	}

	return 1;
}

bool cEditor :: Item_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// activate item
	if( item )
	{
		Activate_Item( static_cast<cEditor_Item_Object *>(item) );
	}

	return 1;
}

void cEditor :: Function_Exit( void )
{
	pKeyboard->Key_Down( SDLK_F8 );
}

// XML element start
void cEditor :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	// Property/Item/Tag of an Element
	if( element == "Property" )
	{
		xml_attributes.add( attributes.getValueAsString( "Name" ), attributes.getValueAsString( "Value" ) );
	}
}

// XML element end
void cEditor :: elementEnd( const CEGUI::String &element )
{
	if( element != "Property" )
	{
		if( element == "Item" )
		{
			// Menu Item
			if( xml_attributes.getValueAsString( "tags" ).length() )
			{
				Handle_Menu( xml_attributes );
			}
			// Items Item
			else
			{
				Handle_Item( xml_attributes );
			}
		}
		else if( element == "Items" || element == "Menu" )
		{
			// ignore
		}
		else if( element.length() )
		{
			printf( "Warning : Editor Unknown Item Element : %s\n", element.c_str() );
		}

		// clear
		xml_attributes = CEGUI::XMLAttributes();
	}
}

void cEditor :: Handle_Item( const CEGUI::XMLAttributes &attributes )
{
	// element name must be given
	CEGUI::String name = xml_attributes.getValueAsString( "object_name" );
	CEGUI::String tags = xml_attributes.getValueAsString( "object_tags" );

	// create
	cSprite *object = NULL;

	// Level object
	if( editor_level_enabled )
	{
		object = Get_Level_Object( name, xml_attributes );
	}
	// Overworld object
	else
	{
		object = Get_World_Object( name, xml_attributes );
	}


	// if creation failed
	if( !object )
	{
		return;
	}

	// set editor tags
	object->m_editor_tags = tags.c_str();

	// Add Item Object
	tagged_item_objects.push_back( object );
}

void cEditor :: Handle_Menu( const CEGUI::XMLAttributes &attributes )
{
	std::string name = xml_attributes.getValueAsString( "name" ).c_str();
	std::string tags = xml_attributes.getValueAsString( "tags" ).c_str();

	Add_Menu_Object( name, tags, CEGUI::PropertyHelper::stringToColour( xml_attributes.getValueAsString( "color", "FFFFFFFF" ) ) );
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
