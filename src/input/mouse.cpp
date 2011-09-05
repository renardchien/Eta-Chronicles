/***************************************************************************
 * mouse.cpp  -  mouse handling class
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
#include "../input/mouse.h"
#include "../input/keyboard.h"
#include "../core/game_core.h"
#include "../level/level_editor.h"
#include "../overworld/world_editor.h"
#include "../overworld/overworld.h"
#include "../gui/menu.h"
#include "../level/level.h"
#include "../user/preferences.h"
#include "../core/framerate.h"
#include "../objects/goldpiece.h"
#include "../objects/level_exit.h"
#include "../objects/bonusbox.h"
#include "../enemies/furball.h"
#include "../video/font.h"
#include "../video/renderer.h"
#include "../core/i18n.h"

namespace SMC
{

/* *** *** *** *** *** cSelectedObject *** *** *** *** *** *** *** *** *** *** *** *** */

cSelectedObject :: cSelectedObject( void )
{
	obj = NULL;

	mouse_h = 0;
	mouse_w = 0;

	user = 0;
}

cSelectedObject :: ~cSelectedObject( void )
{

}

/* *** *** *** *** *** cCopyObject *** *** *** *** *** *** *** *** *** *** *** *** */

cCopyObject :: cCopyObject( void )
{
	obj = NULL;
}

cCopyObject :: ~cCopyObject( void )
{
	if( obj )
	{
		delete obj;
	}
}

/* *** *** *** *** *** cMouseCursor *** *** *** *** *** *** *** *** *** *** *** *** */

cMouseCursor :: cMouseCursor( void )
: cMovingSprite()
{
	m_type = TYPE_MOUSECURSOR;

	m_click_counter = 0.0f;

	m_selection_mode = 0;
	m_selection_rect = GL_rect( 0, 0, 0, 0 );

	m_hovering_object = new cSelectedObject();
	m_active_object = NULL;

	m_fastcopy_mode = 0;
	m_mover_mode = 0;
	m_last_clicked_object = NULL;

	Reset_Keys();

	SDL_GetMouseState( &m_x, &m_y );
	// scale to the virtual game size
	m_x = static_cast<int>( static_cast<float>(m_x) * global_downscalex );
	m_y = static_cast<int>( static_cast<float>(m_y) * global_downscaley );
}

cMouseCursor :: ~cMouseCursor( void )
{
	Clear_Copy_Objects();
	Clear_Selected_Objects();
	delete m_hovering_object;
}

void cMouseCursor :: Reset( bool clear_copy_buffer /* = 1 */ )
{
	// only clear copy buffer if requested
	if( clear_copy_buffer )
	{
		Clear_Copy_Objects();
	}
	Clear_Selected_Objects();
	Clear_Mouse_Object();
	Clear_Active_Object();

	// change to default cursor
	if( m_mover_mode )
	{
		CEGUI::MouseCursor::getSingleton().setImage( "TaharezLook", "MouseArrow" );
	}

	m_mover_mode = 0;
	m_selection_mode = 0;
	m_fastcopy_mode = 0;

	Clear_Collisions();

	// show mouse
	if( editor_enabled )
	{
		Set_Active( 1 );
	}
}

void cMouseCursor :: Reset_Keys( void )
{
	m_left = 0;
	m_right = 0;
	m_middle = 0;
}

bool cMouseCursor :: Handle_Event( SDL_Event *ev )
{
	switch( ev->type )
	{
		case SDL_MOUSEMOTION:
		{
			pGuiSystem->injectMousePosition( static_cast<float>(ev->motion.x), static_cast<float>(ev->motion.y) );
			Update_Position();
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			if( Handle_Mouse_Up( ev->button.button ) )
			{
				// processed
				return 1;
			}

			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			if( Handle_Mouse_Down( ev->button.button ) )
			{
				// processed
				return 1;
			}

			break;
		}
		default:
		{
			break;
		}
	}

	return 0;
}

bool cMouseCursor :: Handle_Mouse_Down( Uint8 button )
{
	switch( button )
	{
		// mouse buttons
		case SDL_BUTTON_LEFT:
		{
			if( CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::LeftButton ) )
			{
				return 1;
			}
			m_left = 1;
			break;
		}
		case SDL_BUTTON_MIDDLE:
		{
			if( CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::MiddleButton ) )
			{
				return 1;
			}
			m_middle = 1;
			break;
		}
		case SDL_BUTTON_RIGHT:
		{
			if( CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::RightButton ) )
			{
				return 1;
			}
			m_right = 1;
			break;
		}
		// mouse wheel
		case SDL_BUTTON_WHEELDOWN:
		{
			if( CEGUI::System::getSingleton().injectMouseWheelChange( -1 ) )
			{
				return 1;
			}
			break;
		}
		case SDL_BUTTON_WHEELUP:
		{
			if( CEGUI::System::getSingleton().injectMouseWheelChange( +1 ) )
			{
				return 1;
			}
			break;
		}
		default:
		{
			break;
		}
	}

	// handle button in the current mode
	if( Game_Mode == MODE_LEVEL )
	{
		// processed by the level
		if( pActive_Level->Mouse_Down( button ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// processed by the overworld
		if( pActive_Overworld->Mouse_Down( button ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_MENU )
	{
		// processed by the menu
		if( pMenuCore->Mouse_Down( button ) )
		{
			return 1;
		}
	}

	return 0;
}

bool cMouseCursor :: Handle_Mouse_Up( Uint8 button )
{
	switch ( button )
	{
		case SDL_BUTTON_LEFT:
		{
			m_left = 0;
			if( CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::LeftButton ) )
			{
				return 1;
			}
		}
		break;
		case SDL_BUTTON_MIDDLE:
		{
			m_middle = 0;
			if( CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::MiddleButton ) )
			{
				return 1;
			}
		}
		break;
		case SDL_BUTTON_RIGHT:
		{
			m_right = 0;
			if( CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::RightButton ) )
			{
				return 1;
			}
		}
		break;
		default:
		{
			break;
		}
	}

	// handle button in the current mode
	if( Game_Mode == MODE_LEVEL )
	{
		// processed by the level
		if( pActive_Level->Mouse_Up( button ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// processed by the overworld
		if( pActive_Overworld->Mouse_Up( button ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_MENU )
	{
		// processed by the menu
		if( pMenuCore->Mouse_Up( button ) )
		{
			return 1;
		}
	}

	return 0;
}

cObjectCollision *cMouseCursor :: Get_First_Editor_Collsion( float px /* = 0.0f */, float py /* = 0.0f */ )
{
	if( m_mover_mode )
	{
		return NULL;
	}

	// Get CEGUI Window containing the mouse
	CEGUI::Window *mouse_window = pGuiSystem->getWindowContainingMouse();

	// if mouse is over a blocking CEGUI window
	if( mouse_window && !mouse_window->isMousePassThroughEnabled() )
	{
		return NULL;
	}

	// mouse rect
	GL_rect mouse_rect;

	// use given posx
	if( !Is_Float_Equal( px, 0.0f ) )
	{
		mouse_rect.m_x = px;
	}
	else
	{
		mouse_rect.m_x = m_pos_x;
	}

	// use given posy
	if( !Is_Float_Equal( py, 0.0f ) )
	{
		mouse_rect.m_y = py;
	}
	else
	{
		mouse_rect.m_y = m_pos_y;
	}

	mouse_rect.m_w = 1.0f;
	mouse_rect.m_h = 1.0f;

	// no collisions
	return Get_First_Mouse_Collision( mouse_rect );
}

cObjectCollision *cMouseCursor :: Get_First_Mouse_Collision( const GL_rect &mouse_rect )
{
	cSprite_List sprite_objects;
	pActive_Sprite_Manager->Get_Objects_sorted( sprite_objects, 1, 1 );

	// check objects
	for( cSprite_List::reverse_iterator itr = sprite_objects.rbegin(), itr_end = sprite_objects.rend(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		// don't check spawned or destroyed objects
		if( obj->m_spawned || obj->m_auto_destroy )
		{
			continue;
		}

		if( mouse_rect.Intersects( obj->m_start_rect ) )
		{
			return Create_Collision_Object( this, obj, COL_VTYPE_INTERNAL );
		}
	}

	return NULL;
}

void cMouseCursor :: Update( void )
{
	// only if editor is enabled
	if( !editor_enabled )
	{
		return;
	}

	Update_Doubleclick();
}

void cMouseCursor :: Draw( void )
{
	// only if editor is enabled
	if( !editor_enabled )
	{
		return;
	}

	Update_Selection();

	// if in Level or Overworld
	if( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD )
	{
		// draw if not in mover mode
		if( !m_mover_mode )
		{
			Draw_Object_Rects();
		}
	}
}

void cMouseCursor :: Render( void ) const
{
	if( !m_active )
	{
		return;
	}

	// Render CEGUI Mouse
	pGuiRenderer->setQueueingEnabled( 0 );
	CEGUI::MouseCursor *mouse = CEGUI::MouseCursor::getSingletonPtr();
	mouse->setVisible( 1 );
	mouse->draw();
	mouse->setVisible( 0 );
}

void cMouseCursor :: Update_Position( void )
{
	if( !m_mover_mode )
	{
		SDL_GetMouseState( &m_x, &m_y );
		// scale to the virtual game size
		m_x = static_cast<int>( static_cast<float>(m_x) * global_downscalex );
		m_y = static_cast<int>( static_cast<float>(m_y) * global_downscaley );
	}

	if( editor_enabled )
	{
		if( !m_mover_mode )
		{
			Set_Pos( m_x + pActive_Camera->x, m_y + pActive_Camera->y, 1 );
		}

		Update_Mouse_Object();
		Update_Selected_Objects();
	}
	else
	{
		Set_Pos( static_cast<float>(m_x), static_cast<float>(m_y), 1 );
	}
}

void cMouseCursor :: Update_Doubleclick( void )
{
	if( m_click_counter )
	{
		m_click_counter -= pFramerate->m_speed_factor;

		if( m_click_counter < 0.0f )
		{
			m_click_counter = 0.0f;
		}
	}
}

void cMouseCursor :: Click( void )
{
	if( m_mover_mode )
	{
		return;
	}

	// if no hovering object
	if( !m_hovering_object->obj )
	{
		// if shift is pressed
		if( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] )
		{
			cObjectCollision *col = Get_First_Editor_Collsion();

			// collision with an object
			if( col )
			{
				// add object
				if( !Add_Selected_Object( col->obj, 1 ) )
				{
					// if already added remove it
					Remove_Selected_Object( col->obj );
				}

				delete col;
			}
		}

		Start_Selection();
		m_click_counter = 0.0f;
		return;
	}

	if( m_click_counter > 0.0f )
	{
		// if last clicked object was the same
		if( m_last_clicked_object == m_hovering_object->obj )
		{
			Double_Click();
		}
		else
		{
			m_click_counter = 0.0f;
		}
	}
	else
	{
		// double click counter
		m_click_counter = speedfactor_fps * 0.3f;
		// save last clicked object
		m_last_clicked_object = m_hovering_object->obj;
	}
}

void cMouseCursor :: Double_Click( bool activate /* = 1 */ )
{
	Clear_Active_Object();

	// add new
	if( activate )
	{
		Set_Active_Object( m_hovering_object->obj );
	}

	m_click_counter = 0.0f;
}

void cMouseCursor :: Set_Mouse_Object( cSprite *sprite )
{
	// return if mouse object is the same or in mouse selection mode
	if( m_hovering_object->obj == sprite || ( sprite && ( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] || m_selection_mode ) ) )
	{
		return;
	}

	// remove old m_hovering_object from selected objects
	if( m_hovering_object->obj )
	{
		Remove_Selected_Object( m_hovering_object->obj, 1 );
	}

	// set new mouse object
	m_hovering_object->obj = sprite;
	// add new mouse object to selected objects
	Add_Selected_Object( sprite );

	if( !sprite )
	{
		m_hovering_object->mouse_w = 0;
		m_hovering_object->mouse_h = 0;
	}
}

void cMouseCursor :: Update_Mouse_Object( void )
{
	if( !editor_enabled || !m_hovering_object->obj || ( m_mover_mode && ( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD ) ) )
	{
		return;
	}

	// left mouse is pressed
	if( m_left )
	{
		// set new position
		m_hovering_object->obj->Set_Pos( static_cast<float>( static_cast<int>(m_pos_x) - m_hovering_object->mouse_w ), static_cast<float>( static_cast<int>(m_pos_y) - m_hovering_object->mouse_h ), 1 );

		// update object settings position
		if( m_active_object && m_active_object == m_hovering_object->obj )
		{
			m_active_object->Editor_Position_Update();
		}
	}
	// left mouse is not pressed
	else
	{
		// update mouse object positioning mouse width and height
		m_hovering_object->mouse_w = static_cast<int>(m_pos_x) - static_cast<int>(m_hovering_object->obj->m_start_pos_x);
		m_hovering_object->mouse_h = static_cast<int>(m_pos_y) - static_cast<int>(m_hovering_object->obj->m_start_pos_y);
	}
}

void cMouseCursor :: Clear_Mouse_Object( void )
{
	Set_Mouse_Object( NULL );
}

void cMouseCursor :: Add_Copy_Object( cSprite *sprite )
{
	if( !sprite )
	{
		return;
	}

	// check if not already added
	for( CopyObjectList::iterator itr = m_copy_objects.begin(), itr_end = m_copy_objects.end(); itr != itr_end; ++itr )
	{
		if( (*itr)->obj == sprite )
		{
			return;
		}
	}

	cSprite *copy = sprite->Copy();

	// can't be copied
	if( !copy )
	{
		return;
	}

	// insert object
	cCopyObject *copy_object = new cCopyObject();
	copy_object->obj = copy;
	m_copy_objects.push_back( copy_object );
}

void cMouseCursor :: Add_Copy_Objects( cSprite_List &spritelist )
{
	if( spritelist.empty() )
	{
		return;
	}

	// insert all objects
	for( cSprite_List::iterator itr = spritelist.begin(), itr_end = spritelist.end(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		Add_Copy_Object( obj );
	}
}

bool cMouseCursor :: Remove_Copy_Object( const cSprite *sprite )
{
	if( !sprite )
	{
		return 0;
	}

	for( CopyObjectList::iterator itr = m_copy_objects.begin(), itr_end = m_copy_objects.end(); itr != itr_end; ++itr )
	{
		cCopyObject *copy_obj = (*itr);

		if( copy_obj->obj == sprite )
		{
			m_copy_objects.erase( itr );
			delete copy_obj;
			
			return 1;
		}
	}

	return 0;
}

void cMouseCursor :: Clear_Copy_Objects( void )
{
	for( CopyObjectList::iterator itr = m_copy_objects.begin(), itr_end = m_copy_objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	m_copy_objects.clear();
}

GL_Vector cMouseCursor :: Get_Copy_Object_Base( float px, float py )
{
	GL_Vector vec = GL_Vector();

	if( m_copy_objects.empty() )
	{
		return vec;
	}

	// set a base
	float base_x = m_copy_objects[0]->obj->m_pos_x;
	float base_y = m_copy_objects[0]->obj->m_pos_y;

	for( CopyObjectList::iterator itr = m_copy_objects.begin(), itr_end = m_copy_objects.end(); itr != itr_end; ++itr )
	{
		cCopyObject *copy_obj = (*itr);

		// if more in top or left
		if( copy_obj->obj->m_pos_x < base_x || copy_obj->obj->m_pos_y < base_y )
		{
			base_x = copy_obj->obj->m_pos_x;
			base_y = copy_obj->obj->m_pos_y;
		}
	}

	vec.x = px - base_x;
	vec.y = py - base_y;

	return vec;
}

void cMouseCursor :: Paste_Copy_Objects( float px, float py )
{
	if( m_copy_objects.empty() )
	{
		return;
	}

	cSprite_List new_objects;

	for( CopyObjectList::iterator itr = m_copy_objects.begin(), itr_end = m_copy_objects.end(); itr != itr_end; ++itr )
	{
		cCopyObject *copy_obj = (*itr);

		// set base vector
		GL_Vector base_vec = Get_Copy_Object_Base( copy_obj->obj->m_pos_x, copy_obj->obj->m_pos_y );

		// copy
		cSprite* new_object = Copy( copy_obj->obj, px + base_vec.x, py + base_vec.y );

		if( new_object )
		{
			new_objects.push_back( new_object );
		}
	}

	if( !m_copy_objects.empty() )
	{
		m_hovering_object->mouse_h = static_cast<int>( m_copy_objects[0]->obj->m_col_rect.m_h / 2 );
		m_hovering_object->mouse_w = static_cast<int>( m_copy_objects[0]->obj->m_col_rect.m_w / 2 );
	}

	// pasted objects are selected
	Clear_Selected_Objects();
	Add_Selected_Objects( new_objects, 1 );
}

bool cMouseCursor :: Add_Selected_Object( cSprite *sprite, bool from_user /* = 0 */ )
{
	if( !sprite )
	{
		return 0;
	}

	// check if not already added
	for( SelectedObjectList::iterator itr = m_selected_objects.begin(), itr_end = m_selected_objects.end(); itr != itr_end; ++itr )
	{
		cSelectedObject *sel_obj = (*itr);

		if( sel_obj->obj == sprite )
		{
			// overwrite user if given
			if( from_user && !sel_obj->user )
			{
				sel_obj->user = 1;
				return 1;
			}

			return 0;
		}
	}

	// insert object
	cSelectedObject *selected_object = new cSelectedObject();
	selected_object->obj = sprite;
	selected_object->user = from_user;
	m_selected_objects.push_back( selected_object );

	return 1;
}

void cMouseCursor :: Add_Selected_Objects( cSprite_List &spritelist, bool from_user /* = 0 */ )
{
	if( spritelist.empty() )
	{
		return;
	}

	// insert all objects
	for( cSprite_List::iterator itr = spritelist.begin(), itr_end = spritelist.end(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		Add_Selected_Object( obj, from_user );
	}
}

bool cMouseCursor :: Remove_Selected_Object( const cSprite *sprite, bool no_user /* = 0 */ )
{
	if( !sprite )
	{
		return 0;
	}

	for( SelectedObjectList::iterator itr = m_selected_objects.begin(), itr_end = m_selected_objects.end(); itr != itr_end; ++itr )
	{
		cSelectedObject *sel_obj = (*itr);

		if( sel_obj->obj == sprite )
		{
			// don't delete user added selected object
			if( no_user && sel_obj->user )
			{
				return 0;
			}

			m_selected_objects.erase( itr );
			delete sel_obj;
			
			return 1;
		}
	}

	return 0;
}

cSprite_List cMouseCursor :: Get_Selected_Objects( void )
{
	cSprite_List spritelist;

	for( SelectedObjectList::iterator itr = m_selected_objects.begin(), itr_end = m_selected_objects.end(); itr != itr_end; ++itr )
	{
		cSelectedObject *object = (*itr);

		spritelist.push_back( object->obj );
	}

	return spritelist;
}

void cMouseCursor :: Clear_Selected_Objects( void )
{
	for( SelectedObjectList::iterator itr = m_selected_objects.begin(), itr_end = m_selected_objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	m_selected_objects.clear();
}

void cMouseCursor :: Update_Selected_Objects( void )
{
	if( !editor_enabled || ( m_mover_mode && ( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD ) ) )
	{
		return;
	}

	// update selected objects positioning mouse width and height
	for( SelectedObjectList::iterator itr = m_selected_objects.begin(), itr_end = m_selected_objects.end(); itr != itr_end; ++itr )
	{
		cSelectedObject *sel_obj = (*itr);

		if( !m_left || m_selection_mode )
		{
			sel_obj->mouse_w = static_cast<int>(m_pos_x) - static_cast<int>(sel_obj->obj->m_start_pos_x);
			sel_obj->mouse_h = static_cast<int>(m_pos_y) - static_cast<int>(sel_obj->obj->m_start_pos_y);
		}
	}

	// if a user selected object is not active
	if( !Is_Selected_Object( m_hovering_object->obj, 1 ) )
	{
		return;
	}

	for( SelectedObjectList::iterator itr = m_selected_objects.begin(), itr_end = m_selected_objects.end(); itr != itr_end; ++itr )
	{
		cSelectedObject *sel_obj = (*itr);

		// left mouse is pressed
		if( m_left )
		{
			// set new position
			sel_obj->obj->Set_Pos( static_cast<float>(static_cast<int>(m_pos_x) - sel_obj->mouse_w), static_cast<float>(static_cast<int>(m_pos_y) - sel_obj->mouse_h), 1 );

			// update object settings position
			if( m_active_object && m_active_object == sel_obj->obj )
			{
				m_active_object->Editor_Position_Update();
			}
		}
	}
}

bool cMouseCursor :: Is_Selected_Object( const cSprite *sprite, bool only_user /* = 0 */ )
{
	if( !sprite )
	{
		return 0;
	}

	for( SelectedObjectList::iterator itr = m_selected_objects.begin(), itr_end = m_selected_objects.end(); itr != itr_end; ++itr )
	{
		cSelectedObject *sel_obj = (*itr);

		if( sel_obj->obj == sprite )
		{
			// if only user objects
			if( only_user && !sel_obj->user )
			{
				continue;
			}

			// found
			return 1;
		}
	}

	return 0;
}

void cMouseCursor :: Delete_Selected_Objects( void )
{
	for( int i = m_selected_objects.size() - 1; i >= 0; i-- )
	{
		Delete( m_selected_objects[i]->obj );
	}

	Clear_Selected_Objects();
}

unsigned int cMouseCursor :: Get_Selected_Object_Size( void ) const
{
	return m_selected_objects.size();
}

void cMouseCursor :: Set_Active_Object( cSprite *sprite )
{
	// clear existing
	Clear_Active_Object();

	if( sprite )
	{
		m_active_object = sprite;
		m_active_object->Editor_Activate();
	}
}

void cMouseCursor :: Clear_Active_Object( void )
{
	if( !m_active_object )
	{
		return;
	}

	m_active_object->Editor_Deactivate();
	m_active_object = NULL;
}

cSprite *cMouseCursor :: Copy( cSprite *ncopy_object, float px, float py ) const
{
	if( !ncopy_object )
	{
		return NULL;
	}

	// only copy in editor mode
	if( !editor_enabled )
	{
		printf( "Warning : No editor enabled for copy object: %s\n", ncopy_object->m_name.c_str() );
		return NULL;
	}

	// invalid array
	if( !( ncopy_object->m_sprite_array == ARRAY_MASSIVE || ncopy_object->m_sprite_array == ARRAY_PASSIVE || ncopy_object->m_sprite_array == ARRAY_ACTIVE || ncopy_object->m_sprite_array == ARRAY_ENEMY || ncopy_object->m_sprite_array == ARRAY_ANIM ) )
	{
		printf( "Warning : cMouseCursor copy : unknown object array : %d\n", ncopy_object->m_sprite_array );
		return NULL;
	}

	// copy
	cSprite *new_sprite = ncopy_object->Copy();

	// failed to copy
	if( !new_sprite )
	{
		debug_print( "Warning : Mouse object copying failed for %s\n", ncopy_object->m_name.c_str() );
		return NULL;
	}

	// set position
	new_sprite->Set_Pos( px, py, 1 );
	// add it
	pActive_Sprite_Manager->Add( new_sprite );

	return new_sprite;
}

void cMouseCursor :: Delete( cSprite *sprite )
{
	// invalid
	if( !sprite )
	{
		return;
	}

	// invalid array
	if( !( sprite->m_sprite_array == ARRAY_PASSIVE || sprite->m_sprite_array == ARRAY_MASSIVE || sprite->m_sprite_array == ARRAY_ACTIVE || sprite->m_sprite_array == ARRAY_ENEMY || sprite->m_sprite_array == ARRAY_ANIM ) )
	{
		return;
	}

	// deactivate active object
	if( m_active_object == sprite )
	{
		Double_Click( 0 );
	}

	// remove if current player active item object
	if( pPlayer->active_object == sprite )
	{
		pPlayer->active_object = NULL;
	}

	// remove selected object
	Remove_Selected_Object( sprite );
	// remove copy object
	Remove_Copy_Object( sprite );

	// delete object
	if( editor_enabled )
	{
		sprite->Destroy();
	}
}

void cMouseCursor :: Draw_Object_Rects( void )
{
	// current hover rect
	GL_rect hover_rect;
	// current object color
	Color obj_color = white;

	if( !m_left && m_hovering_object->obj )
	{
		hover_rect.m_x = m_hovering_object->obj->m_start_pos_x - pActive_Camera->x;
		hover_rect.m_y = m_hovering_object->obj->m_start_pos_y - pActive_Camera->y;
		hover_rect.m_w = m_hovering_object->obj->m_start_rect.m_w;
		hover_rect.m_h = m_hovering_object->obj->m_start_rect.m_h;

		// get object color
		obj_color = Get_Massive_Type_Color( m_hovering_object->obj->m_massive_type );

		// create request
		cRect_Request *request = new cRect_Request();
		pVideo->Draw_Rect( &hover_rect, 0.6f, &obj_color, request );

		if( m_fastcopy_mode )
		{
			request->color.alpha = 64;
		}
		// not fastcopy
		else
		{
			// not filled
			request->filled = 0;
		}

		// add request
		pRenderer->Add( request );
	}


	// draw selected objects if not left mouse is pressed, shift selection or mouse selection
	if( !m_left || ( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] ) || m_selection_mode )
	{
		obj_color = white;

		for( SelectedObjectList::iterator itr = m_selected_objects.begin(), itr_end = m_selected_objects.end(); itr != itr_end; ++itr )
		{
			cSprite *object = (*itr)->obj;

			// check if visible on screen or mouse object
			if( !object->Is_Visible_On_Screen() || object == m_hovering_object->obj )
			{
				continue;
			}

			hover_rect.m_x = object->m_start_pos_x - pActive_Camera->x;
			hover_rect.m_y = object->m_start_pos_y - pActive_Camera->y;
			hover_rect.m_w = object->m_start_rect.m_w;
			hover_rect.m_h = object->m_start_rect.m_h;

			// object color
			obj_color = Get_Massive_Type_Color( object->m_massive_type );

			// z position
			float pos_z = 0.5f;

			// different z position for other massive type
			if( object->m_massive_type == MASS_HALFMASSIVE )
			{
				pos_z += 0.001f;
			}
			else if( object->m_massive_type == MASS_MASSIVE )
			{
				pos_z += 0.002f;
			}
			else if( object->m_massive_type == MASS_CLIMBABLE )
			{
				pos_z += 0.003f;
			}

			// create request
			cRect_Request *rect_request = new cRect_Request();
			pVideo->Draw_Rect( &hover_rect, pos_z, &obj_color, rect_request );

			// not filled
			rect_request->filled = 0;
			// with stipple
			rect_request->stipple_pattern = 0xAAAA;

			// add request
			pRenderer->Add( rect_request );
		}
	}
}

void cMouseCursor :: Start_Selection( void )
{
	Clear_Mouse_Object();
	m_selection_mode = 1;
	m_selection_rect.m_x = m_x + pActive_Camera->x;
	m_selection_rect.m_y = m_y + pActive_Camera->y;
}

void cMouseCursor :: End_Selection( void )
{
	m_selection_mode = 0;

	Update_Position();
}

void cMouseCursor :: Update_Selection( void )
{
	if( !m_selection_mode )
	{
		return;
	}

	m_selection_rect.m_w = m_x - m_selection_rect.m_x + pActive_Camera->x;
	m_selection_rect.m_h = m_y - m_selection_rect.m_y + pActive_Camera->y;

	// create collision checking rect
	GL_rect rect = GL_rect( m_selection_rect.m_x, m_selection_rect.m_y, 0, 0 );

	// negative to positive
	if( m_selection_rect.m_w < 0 )
	{
		rect.m_x = m_selection_rect.m_x + m_selection_rect.m_w;
		rect.m_w = -m_selection_rect.m_w;
	}
	// positive
	else
	{
		rect.m_w = m_selection_rect.m_w;
	}

	// negative to positive
	if( m_selection_rect.m_h < 0 )
	{
		rect.m_y = m_selection_rect.m_y + m_selection_rect.m_h;
		rect.m_h = -m_selection_rect.m_h;
	}
	// positive
	else
	{
		rect.m_h = m_selection_rect.m_h;
	}

	// if rect is below minimum
	if( rect.m_w < 2 && rect.m_h < 2 )
	{
		return;
	}

	// only clear if not shift is pressed
	if( !( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] ) )
	{
		Clear_Selected_Objects();
	}

	// add selected objects
	for( cSprite_List::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		// don't check spawned/destroyed objects
		if( obj->m_spawned || obj->m_auto_destroy )
		{
			continue;
		}

		// if it doesn't collide with the rect
		if( !rect.Intersects( obj->m_rect ) )
		{
			continue;
		}
		
		Add_Selected_Object( obj, 1 );
	}

	if( rect.Intersects( pActive_Player->m_rect ) )
	{
		Add_Selected_Object( pActive_Player, 1 );
	}

	// ## inner rect
	cRect_Request *rect_request = new cRect_Request();
	pVideo->Draw_Rect( m_selection_rect.m_x - pActive_Camera->x + 0.5f, m_selection_rect.m_y - pActive_Camera->y + 0.5f, m_selection_rect.m_w, m_selection_rect.m_h, 0.509f, &white, rect_request );

	// not filled
	rect_request->filled = 0;
	// color
	rect_request->color.alpha = 128;

	// add request
	pRenderer->Add( rect_request );

	// ## outer rect
	// create request
	rect_request = new cRect_Request();
	pVideo->Draw_Rect( m_selection_rect.m_x - pActive_Camera->x, m_selection_rect.m_y - pActive_Camera->y, m_selection_rect.m_w, m_selection_rect.m_h, 0.51f, &lightblue, rect_request );

	// not filled
	rect_request->filled = 0;

	// add request
	pRenderer->Add( rect_request );
}

void cMouseCursor :: Toggle_Mover_Mode( void )
{
	m_mover_mode = !m_mover_mode;

	if( m_mover_mode )
	{
		CEGUI::MouseCursor::getSingleton().setImage( "TaharezLook", "MouseMoveCursor" );
	}
	else
	{
		CEGUI::MouseCursor::getSingleton().setImage( "TaharezLook", "MouseArrow" );
	}
}

void cMouseCursor :: Mover_Update( Sint16 move_x, Sint16 move_y )
{
	if( !m_mover_mode )
	{
		return;
	}

	// mouse moves the camera
	pActive_Camera->Move( move_x, move_y );
	// keep mouse at it's position
	SDL_WarpMouse( static_cast<Uint16>(m_x * global_upscalex), static_cast<Uint16>(m_y * global_upscaley) );

	SDL_Event inEvent;
	SDL_PeepEvents( &inEvent, 1, SDL_GETEVENT, SDL_MOUSEMOTIONMASK );

	while( SDL_PollEvent( &inEvent ) )
	{
		switch( inEvent.type )
		{
			case SDL_MOUSEBUTTONDOWN:
			{
				if( inEvent.button.button == 2 )
				{
					m_mover_mode = 0;
				}
				break;
			}
			case SDL_KEYDOWN:
			{
				m_mover_mode = 0;
				pKeyboard->Key_Down( inEvent.key.keysym.sym );
				break;
			}
			case SDL_QUIT:
			{
				game_exit = 1;
				break;
			}
		}
	}
}

void cMouseCursor :: Editor_Update( void )
{
	if( !editor_enabled )
	{
		return;
	}

	cObjectCollision *col = Get_First_Editor_Collsion();

	// if no collision
	if( !col )
	{
		if( !m_left )
		{
			Clear_Mouse_Object();
		}
		else
		{
			// no mouse object
			if( !m_hovering_object->obj )
			{
				// clear active object
				Double_Click( 0 );

				// clear selected objects if no shift is pressed
				if( !( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] ) )
				{
					Clear_Selected_Objects();
				}
			}
		}

		return;
	}

	// mouse object type string
	std::string type_name;

	// set object data
	if( col->obj )
	{
		type_name = col->obj->m_name;

		if( !m_left || !m_hovering_object->obj )
		{
			Set_Mouse_Object( col->obj );
		}
	}

	// Set the massivetype text
	if( col->m_array == ARRAY_MASSIVE )
	{
		if( col->obj->m_massive_type == MASS_HALFMASSIVE )
		{
			type_name.insert( 0, _("Halfmassive - ") );
		}
		else
		{
			type_name.insert( 0, _("Massive - ") );
		}
	}
	else if( col->m_array == ARRAY_PASSIVE )
	{
		if( col->obj->m_type == TYPE_OW_LINE_START || col->obj->m_type == TYPE_OW_LINE_END || col->obj->m_type == TYPE_OW_WAYPOINT )
		{
			// ignore
		}
		else if( col->obj->m_type == TYPE_FRONT_PASSIVE )
		{
			type_name.insert( 0, _("Front Passive - ") );
		}
		else
		{
			type_name.insert( 0, _("Passive - ") );
		}
	}
	else if( col->m_array == ARRAY_ACTIVE )
	{
		if( col->obj->m_type == TYPE_HALFMASSIVE )
		{
			type_name.insert( 0, _("Halfmassive - ") );
		}
		else if( col->obj->m_type == TYPE_CLIMBABLE )
		{
			type_name.insert( 0, _("Climbable - ") );
		}
	}

	// if valid object draw position text
	if( m_hovering_object->obj )
	{
		// position text
		std::string info = "X : " + int_to_string( static_cast<int>(m_hovering_object->obj->m_start_pos_x) ) + "  Y : " + int_to_string( static_cast<int>(m_hovering_object->obj->m_start_pos_y) );
		
		if( game_debug )
		{
			info.insert( 0, "Start " );
		}

		// create text surface
		cGL_Surface *position_info = pFont->Render_Text( pFont->m_font_small, info, white );


		// create request
		cSurface_Request *request = new cSurface_Request();
		position_info->Blit( static_cast<float>( m_x + 20 ), static_cast<float>( m_y + 35 ), 0.52f, request );
		request->delete_texture = 1;

		// shadow
		request->shadow_pos = 1.0f;
		request->shadow_color = black;

		// add request
		pRenderer->Add( request );

		// cSurface_Request deletes it
		position_info->m_auto_del_img = 0;
		delete position_info;

		// if in debug mode draw current position X, Y, Z and if available editor Z
		if( game_debug )
		{
			info = "Curr.  X : " + int_to_string( static_cast<int>(m_hovering_object->obj->m_pos_x) ) + "  Y : " + int_to_string( static_cast<int>(m_hovering_object->obj->m_pos_y) ) + "  Z : " + float_to_string( m_hovering_object->obj->m_pos_z, 6 );

			// if also got editor z position
			if( !Is_Float_Equal( col->obj->m_editor_pos_z, 0.0f ) )
			{
				info.insert( info.length(), _("  Editor Z : ") + float_to_string( m_hovering_object->obj->m_editor_pos_z, 6 ) );
			}

			// create text surface
			position_info = pFont->Render_Text( pFont->m_font_small, info, white );
			
			// create request
			request = new cSurface_Request();
			position_info->Blit( static_cast<float>( m_x + 20 ), static_cast<float>( m_y + 55 ), 0.52f, request );
			request->delete_texture = 1;

			// shadow
			request->shadow_pos = 1.0f;
			request->shadow_color = black;

			// add request
			pRenderer->Add( request );

			// cSurface_Request deletes it
			position_info->m_auto_del_img = 0;
			delete position_info;
		}
	}

	if( pHud_Debug->m_counter <= 0.0f )
	{
		pHud_Debug->Set_Text( type_name, 1 );
	}

	delete col;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cMouseCursor *pMouseCursor = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
