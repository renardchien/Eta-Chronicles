/***************************************************************************
 * hud.cpp  -  human user interfaces in the game
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

#include "../gui/hud.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../audio/audio.h"
#include "../video/font.h"
#include "../core/framerate.h"
#include "../level/level.h"
#include "../core/sprite_manager.h"
#include "../objects/bonusbox.h"
#include "../video/renderer.h"
#include "../core/i18n.h"
#include "../core/filesystem/filesystem.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cHudSprite *** *** *** *** *** *** *** *** *** *** */

cHudSprite :: cHudSprite( cGL_Surface *new_image /* = NULL */, float x /* = 0.0f */, float y /* = 0.0f */, bool del_img /* = 0 */ )
: cSprite( new_image, x, y, del_img )
{
	m_player_range = 0;
	m_pos_z = 0.13f;

	Set_Ignore_Camera( 1 );
}

cHudSprite :: ~cHudSprite( void )
{

}

cHudSprite *cHudSprite :: Copy( void )
{
	cHudSprite *hud_sprite = new cHudSprite( m_start_image, m_pos_x, m_pos_y );

	hud_sprite->m_type = m_type;
	hud_sprite->m_sprite_array = m_sprite_array;
	hud_sprite->m_massive_type = m_massive_type;
	hud_sprite->Set_Ignore_Camera( m_no_camera );
	hud_sprite->Set_Shadow_Pos( m_shadow_pos );
	hud_sprite->Set_Shadow_Color( m_shadow_color );

	return hud_sprite;
}

/* *** *** *** *** *** *** *** cHud_Manager *** *** *** *** *** *** *** *** *** *** */

cHud_Manager :: cHud_Manager( void )
: cObject_Manager<cHudSprite>()
{
	m_loaded = 0;

	Load();
}

cHud_Manager :: ~cHud_Manager( void )
{
	Unload();
}

void cHud_Manager :: Load( void )
{
	if( !m_loaded && !objects.empty() )
	{
		Unload();
	}
	else if( m_loaded )
	{
		Update_Text();
		return;
	}

	// Menu Background ( Maryo head and the Goldpiece )
	Add( static_cast<cHudSprite *>(new cMenuBackground()) );
	// Point Display
	pHud_Points = new cPlayerPoints();
	Add( static_cast<cHudSprite *>(pHud_Points) );
	// Time Display
	pHud_Time = new cTimeDisplay();
	Add( static_cast<cHudSprite *>(pHud_Time) );
	// Live Display
	pHud_Lives = new cLiveDisplay();
	Add( static_cast<cHudSprite *>(pHud_Lives) );
	// Gold Display
	pHud_Goldpieces = new cGoldDisplay();
	Add( static_cast<cHudSprite *>(pHud_Goldpieces) );
	// Itembox
	pHud_Itembox = new cItemBox();
	Add( static_cast<cHudSprite *>(pHud_Itembox) );
	//Life Display
	pHud_LifeDisplay = new cLifeDisplay();
	Add( static_cast<cHudSprite *>(pHud_LifeDisplay) );
	// Debug Display
	pHud_Debug = new cDebugDisplay();
	Add( static_cast<cHudSprite *>(pHud_Debug) );

	m_loaded = 1;
}

void cHud_Manager :: Unload( void )
{
	if( objects.empty() )
	{
		return;
	}
	
	Delete_All();

	pHud_Lives = NULL;
	pHud_Goldpieces = NULL;
	pHud_Points = NULL;
	pHud_Time = NULL;
	pHud_Debug = NULL;
	pHud_Itembox = NULL;
	
	m_loaded = 0;
}

void cHud_Manager :: Update_Text( void )
{
	// note : update the life display before you update the timedisplay
	
	if( !objects.empty() )
	{
		cMenuBackground *item = static_cast<cMenuBackground *>(objects[0]);

		if( Game_Mode != MODE_OVERWORLD )
		{
			// goldpiece
			item->m_rect_goldpiece.m_y = item->m_rect_maryo_head.m_y + 6.0f;
		}
		else
		{
			// goldpiece
			item->m_rect_goldpiece.m_y = 7.0f;
		}
	}

	if( pHud_Lives )
	{
		if( Game_Mode != MODE_OVERWORLD )
		{
			pHud_Lives->Set_Pos( game_res_w - game_res_w * 0.1f, 18.0f );
		}
		else
		{
			pHud_Lives->Set_Pos( game_res_w - game_res_w / 7.5f, 4.0f );
		}

		pHud_Lives->Add_Lives( 0 );
	}

	if( pHud_Goldpieces )
	{
		if( Game_Mode != MODE_OVERWORLD )
		{
			pHud_Goldpieces->Set_Pos( 280.0f, 18.0f );
		}
		else
		{
			pHud_Goldpieces->Set_Pos( 280.0f, 4.0f );
		}

		pHud_Goldpieces->Add_Gold( 0 );
	}

	if( pHud_Points )
	{
		if( Game_Mode != MODE_OVERWORLD )
		{
			pHud_Points->Set_Pos( 50.0f, 18.0f );
		}
		else
		{
			pHud_Points->Set_Pos( 50.0f, 4.0f );
		}

		pHud_Points->Add_Points( 0 );
	}

	if( pHud_Time )
	{
		pHud_Time->Set_Pos( game_res_w * 0.70f, 18.0f );
		pHud_Time->Update();
	}

	if( pHud_Debug )
	{
		pHud_Debug->Set_Pos( game_res_w * 0.45f, 80.0f );
		pHud_Debug->Update();
	}

	if( pHud_Itembox ) 
	{
		pHud_Itembox->Set_Pos( game_res_w * 0.49f, 10.0f );
		pHud_Itembox->Update();
	}
	
	if (pHud_LifeDisplay )
	{
		pHud_LifeDisplay->Set_Pos(0.1f, 8.0f );
		pHud_LifeDisplay->Update();
	}
}

void cHud_Manager :: Update( void )
{
	// update HUD objects
	for( HudSpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Update();
	}
}

void cHud_Manager :: Draw( void )
{
	// draw HUD objects
	for( HudSpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cHud_Manager *pHud_Manager = NULL;

/* *** *** *** *** *** *** PointsText *** *** *** *** *** *** *** *** *** *** *** */

PointsText :: PointsText( void )
: cHudSprite()
{
	m_points = 0;
	m_vely = 0.0f;
}

PointsText :: ~PointsText( void )
{
	//
}

/* *** *** *** *** *** *** cMenuBackground *** *** *** *** *** *** *** *** *** *** *** */

cMenuBackground :: cMenuBackground( float x /* = 0.0f */, float y /* = 0.0f */ )
: cHudSprite( NULL, x, y )
{
	m_type = TYPE_MENUBG;
	m_sprite_array = ARRAY_HUD;
	m_name = "HUD Menu Background";

	m_maryo_head = pVideo->Get_Surface( "game/maryo_l.png" );
	m_goldpiece = pVideo->Get_Surface( "game/gold_m.png" );

	if(	!m_maryo_head || !m_goldpiece )
	{
		printf( "Error : MenuBackground images loading failed\n" );
		return;
	}

	// maryo head
	m_rect_maryo_head.m_x = game_res_w * 0.933f;
	m_rect_maryo_head.m_y = 15;
	// goldpiece
	m_rect_goldpiece.m_x = 250;
	m_rect_goldpiece.m_y = 0;
}

cMenuBackground :: ~cMenuBackground( void )
{
	//
}

void cMenuBackground :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_MENU )
	{
		return;
	}

	// maryo head
	if( Game_Mode != MODE_OVERWORLD )
	{
		m_maryo_head->Blit( m_rect_maryo_head.m_x, m_rect_maryo_head.m_y, m_pos_z );
	}

	// goldpiece
	m_goldpiece->Blit( m_rect_goldpiece.m_x, m_rect_goldpiece.m_y, m_pos_z );
}

/* *** *** *** *** *** *** cStatusText *** *** *** *** *** *** *** *** *** *** *** */

cStatusText :: cStatusText( float x /* = 0.0f */, float y /* = 0.0f */ )
: cHudSprite( NULL, x, y )
{
	m_type = TYPE_STATUSTEXT;
	m_sprite_array = ARRAY_HUD;

	Set_Shadow( black, 1.5f );
}

cStatusText :: ~cStatusText( void )
{
	//
}

void cStatusText :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( Game_Mode == MODE_MENU )
	{
		return;
	}

	cHudSprite::Draw();
}

/* *** *** *** *** *** *** cPlayerPoints *** *** *** *** *** *** *** *** *** *** *** */

cPlayerPoints :: cPlayerPoints( float x /* = 0.0f */, float y /* = 0.0f */ )
: cStatusText( x, y )
{
	m_sprite_array = ARRAY_HUD;
	m_type = TYPE_POINTDISPLAY;
	m_name = "HUD Player points";
	
	Set_Points( pPlayer->points );

	m_points_objects.reserve( 50 );
}

cPlayerPoints :: ~cPlayerPoints( void )
{
	Clear();
}

void cPlayerPoints :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_MENU )
	{
		return;
	}
	
	cHudSprite::Draw( request );

	// draw small points
	for( PointsTextList::iterator itr = m_points_objects.begin(); itr != m_points_objects.end(); )
	{
		// get object pointer
		PointsText *obj = (*itr);
		
		// if finished
		if( !obj->m_image )
		{
			itr = m_points_objects.erase( itr );
			delete obj;
		}
		// active
		else
		{
			obj->m_vely -= obj->m_vely * 0.01f * pFramerate->m_speed_factor;
			obj->m_pos_y += obj->m_vely * pFramerate->m_speed_factor;

			if( obj->m_vely > -1.0f )
			{
				obj->Set_Image( NULL );
				continue;
			}

			float x = obj->m_pos_x - pActive_Camera->x;
			float y = obj->m_pos_y - pActive_Camera->y;

			// out in left
			if( x < 0.0f )
			{
				x = 5.0f;
			}
			// out in right
			else if( x > game_res_w )
			{
				x = game_res_w - obj->m_col_rect.m_w - 2.0f; 
			}

			// out on bottom
			if( y < 0.0f )
			{
				y = 5.0f;
			}
			// out on top
			else if( y > game_res_h )
			{
				y = game_res_h - obj->m_col_rect.m_h - 20.0f;
			}

			// create request
			cSurface_Request *request = new cSurface_Request();
			obj->m_image->Blit( x, y, m_pos_z, request );

			// shadow
			request->shadow_color = black;
			request->shadow_pos = 1;

			// color
			request->color = Color( static_cast<Uint8>( 255 - ( obj->m_points / 150 ) ), static_cast<Uint8>( 255 - ( obj->m_points / 150 ) ), static_cast<Uint8>( 255 - ( obj->m_points / 30 ) ) );

			// add request
			pRenderer->Add( request );

			++itr;
		}
	}
}

void cPlayerPoints :: Set_Points( long points )
{
	pPlayer->points = points;

	char text[70];
	sprintf( text, _("Points %08d"), static_cast<int>(pPlayer->points) );
	Set_Image( pFont->Render_Text( pFont->m_font_normal, text, white ), 0, 1 );
}

void cPlayerPoints :: Add_Points( unsigned int points, float x /* = 0.0f */, float y /* = 0.0f */, std::string strtext /* = "" */, const Color &color /* = static_cast<Uint8>(255) */, bool allow_multiplier /* = 0 */ )
{
	if( allow_multiplier )
	{
		points = static_cast<unsigned int>( pPlayer->kill_multiplier * static_cast<float>(points) );
	}

	Set_Points( pPlayer->points + points );

	if( x == 0 || y == 0 || m_points_objects.size() > 50 )
	{
		return;
	}

	// if empty set the points as text
	if( strtext.empty() )
	{
		strtext = int_to_string( points );
	}

	PointsText *new_obj = new PointsText();
	new_obj->Set_Image( pFont->Render_Text( pFont->m_font_small, strtext, color ), 1, 1 );

	new_obj->Set_Pos( x, y );
	new_obj->m_vely = -1.4f;
	new_obj->m_points = points;
	
	// check if it collides with an already active points text
	for( PointsTextList::iterator itr = m_points_objects.begin(); itr != m_points_objects.end(); ++itr )
	{
		// get object pointer
		PointsText *obj = (*itr);

		if( new_obj->m_rect.Intersects( obj->m_col_rect ) )
		{
			new_obj->Move( obj->m_col_rect.m_w + 5, 0, 1 );
		}
	}

	m_points_objects.push_back( new_obj );
}

void cPlayerPoints :: Clear( void )
{
	for( PointsTextList::iterator itr = m_points_objects.begin(), itr_end = m_points_objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	m_points_objects.clear();
}

/* *** *** *** *** *** cGoldDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

cGoldDisplay :: cGoldDisplay( float x /* = 0.0f */, float y /* = 0.0f */ )
: cStatusText( x, y )
{
	m_sprite_array = ARRAY_HUD;
	m_type = TYPE_GOLDDISPLAY;
	m_name = "HUD Goldpieces";

	Set_Gold( pPlayer->goldpieces );
}

cGoldDisplay :: ~cGoldDisplay( void )
{
	//
}

void cGoldDisplay :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_MENU ) 
	{
		return;
	}

	cHudSprite::Draw();
}

void cGoldDisplay :: Set_Gold( int gold )
{
	if( gold >= 100 )
	{
		gold -= 100;
		pAudio->Play_Sound( "item/live_up_2.ogg", RID_1UP_MUSHROOM );	
		pHud_Lives->Add_Lives( 1 );

		pHud_Points->Add_Points( 0, pPlayer->m_pos_x + pPlayer->m_image->m_w/3, pPlayer->m_pos_y + 5, "1UP", lightred );
	}
	
	pPlayer->goldpieces = gold;
	std::string text = int_to_string( pPlayer->goldpieces );

	Color color = Color( static_cast<Uint8>(255), 255, 255 - ( gold * 2 ) );

	Set_Image( pFont->Render_Text( pFont->m_font_normal, text, color ), 0, 1 );
}

void cGoldDisplay :: Add_Gold( int gold )
{
	Set_Gold( pPlayer->goldpieces + gold );
}

/* *** *** *** *** *** cLiveDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

cLiveDisplay :: cLiveDisplay( float x /* = 0.0f */, float y /* = 0.0f */ )
: cStatusText( x, y )
{
	m_sprite_array = ARRAY_HUD;
	m_type = TYPE_LIFEDISPLAY;
	m_name = "HUD Lives";

	Set_Lives( pPlayer->lives );

	Set_Image( NULL );
}

cLiveDisplay :: ~cLiveDisplay( void )
{
	//
}

void cLiveDisplay :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_MENU ) 
	{
		return;
	}

	cHudSprite::Draw();
}

void cLiveDisplay :: Set_Lives( int lives )
{
	pPlayer->lives = lives;
	
	if( lives < 0 )
	{
		return;
	}

	std::string text;

	// if not in Overworld
	if( Game_Mode != MODE_OVERWORLD )
	{
		text = int_to_string( pPlayer->lives ) + "x";
	}
	else
	{
		text = _("Lives : ") + int_to_string( pPlayer->lives );
	}

	Set_Image( pFont->Render_Text( pFont->m_font_normal, text, green ), 0, 1 );

	// set position
	int w, h;

	if( TTF_SizeText( pFont->m_font_normal, text.c_str(), &w, &h ) == 0 )
	{
		Set_Pos_X( (game_res_w * 0.94f) - w );
	}
}

void cLiveDisplay :: Add_Lives( int lives )
{
	Set_Lives( pPlayer->lives + lives );
}

/* *** *** *** *** *** *** *** cTimeDisplay *** *** *** *** *** *** *** *** *** *** */

cTimeDisplay :: cTimeDisplay( float x /* = 0.0f */, float y /* = 0.0f */ )
: cStatusText( x, y )
{
	m_sprite_array = ARRAY_HUD;
	m_type = TYPE_GAMETIMEDISPLAY;
	m_name = "HUD Time";

	Reset();
}

cTimeDisplay :: ~cTimeDisplay( void )
{
	//
}

void cTimeDisplay :: Update( void )
{
	if( editor_enabled || Game_Mode == MODE_OVERWORLD || Game_Mode == MODE_MENU )
	{
		return;
	}

	m_milliseconds += pFramerate->m_elapsed_ticks;

	// not valid
	if( m_milliseconds == 0 )
	{
		return;
	}

	const Uint32 seconds = m_milliseconds / 1000;

	// update is not needed
	if( seconds == m_last_update_seconds )
	{
		return;
	}

	m_last_update_seconds = seconds;

	const Uint32 minutes = seconds / 60;

	// Set new time
	sprintf( m_text, _("Time %02d:%02d"), minutes, seconds - ( minutes * 60 ) );
	Set_Image( pFont->Render_Text( pFont->m_font_normal, m_text, white ), 0, 1 );
}

void cTimeDisplay :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_OVERWORLD || Game_Mode == MODE_MENU )
	{
		return;
	}

	cHudSprite::Draw();
}

void cTimeDisplay :: Reset( void )
{
	sprintf( m_text, "Time" );
	m_last_update_seconds = 1000;
	m_milliseconds = 0;
}

/* *** *** *** *** *** *** *** cItemBox *** *** *** *** *** *** *** *** *** *** */

cItemBox :: cItemBox( float x /* = 0.0f */, float y /* = 0.0f */ )
: cStatusText( x, y )
{
	m_sprite_array = ARRAY_HUD;
	m_type = TYPE_ITEMBOXDISPLAY;
	m_name = "HUD Itembox";

	Set_Image( pVideo->Get_Surface( "game/itembox.png" ) );
	// disable shadow
	Set_Shadow_Pos( 0 );

	m_box_color = white;

	m_item_counter = 0;
	m_item_counter_mod = 0;
	m_item_id = TYPE_UNDEFINED;

	m_item = new cMovingSprite();
	m_item->Set_Ignore_Camera( 1 );
	m_item->m_player_range = 0;
	m_item->Set_Massive_Type( MASS_MASSIVE );
	m_item->m_pos_z = 0.1299f;
}

cItemBox :: ~cItemBox( void )
{
	delete m_item;
}

void cItemBox :: Update( void )
{
	if( m_item_counter )
	{
		m_item->Move( 0.0f, 4.0f );

		if( m_item_counter_mod )
		{
			m_item_counter += pFramerate->m_speed_factor * 10.0f;

			if( m_item_counter >= 90.0f )
			{
				m_item_counter_mod = 0;
				m_item_counter = 90.0f;
			}
		}
		else
		{
			m_item_counter -= pFramerate->m_speed_factor * 10.0f;

			if( m_item_counter <= 0.0f )
			{
				m_item_counter_mod = 1;
				m_item_counter = 1.0f;
			}
		}

		if( m_item->m_pos_y > game_res_h )
		{
			Reset();
		}

		cObjectCollisionType *col_list = m_item->Collision_Check( &m_item->m_col_rect, COLLIDE_ONLY_BLOCKING );

		// if colliding with the player
		if( col_list->Is_Included( TYPE_PLAYER ) )
		{
			// player can send an item back
			SpriteType item_id_temp = m_item_id;
			Reset();
			pPlayer->Get_Item( item_id_temp, 1 );
		}

		delete col_list;
	}
}

void cItemBox :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_OVERWORLD || Game_Mode == MODE_MENU )
	{
		return;
	}

	if( m_item_id && m_item->m_image )
	{
		// with alpha
		if( m_item_counter )
		{
			m_item->Set_Color( 255, 255, 255, 100 + static_cast<Uint8>(m_item_counter) );
		}
		
		m_item->Draw();
	}

	Set_Color( m_box_color.red, m_box_color.green, m_box_color.blue );
	cHudSprite::Draw();
}

void cItemBox :: Set_Item( SpriteType item_type, bool sound /* = 1 */ )
{
	// play sound
	if( sound )
	{
		pAudio->Play_Sound( "itembox_set.ogg" );
	}
	// reset data
	Reset();

	// reset startposition
	m_item->Set_Pos( 0.0f, 0.0f, 1 );
	// reset color
	m_item->Set_Color( white );

	if( item_type == TYPE_MUSHROOM_DEFAULT )
	{
		m_box_color = Color( static_cast<Uint8>(250), 50, 50 );
		m_item->Set_Image( pVideo->Get_Surface( "game/items/mushroom_red.png" ) );
	}
	else if( item_type == TYPE_FIREPLANT )
	{
		m_box_color = Color( static_cast<Uint8>(250), 200, 150 );
		m_item->Set_Image( pVideo->Get_Surface( "game/items/fireplant.png" ) );
	}
	else if( item_type == TYPE_MUSHROOM_BLUE )
	{
		m_box_color = Color( static_cast<Uint8>(100), 100, 250 );
		m_item->Set_Image( pVideo->Get_Surface( "game/items/mushroom_blue.png" ) );
	}


	if( m_item->m_image )
	{
		m_item->Set_Pos( m_pos_x - ( ( m_item->m_image->m_w - m_rect.m_w ) / 2 ), m_pos_y - ( ( m_item->m_image->m_h - m_rect.m_h ) / 2 ) );
	}

	m_item_id = item_type;
}

void cItemBox :: Request_Item( void )
{
	if( !m_item_id || m_item_counter ) 
	{
		return;
	}

	pAudio->Play_Sound( "itembox_get.ogg" );

	m_item_counter = 255.0f;
	// draw item with camera
	m_item->Set_Ignore_Camera( 0 );
	m_item->Set_Pos( m_item->m_pos_x + pActive_Camera->x, m_item->m_pos_y + pActive_Camera->y );
}

void cItemBox :: Push_back( void )
{
	m_item_counter = 0.0f;
	m_item_counter_mod = 0;

	// draw item without camera
	m_item->Set_Ignore_Camera( 1 );
	m_item->Set_Pos( m_item->m_start_pos_x, m_item->m_start_pos_y );
	m_item->Set_Color( white );
}

void cItemBox :: Reset( void )
{
	m_item->Set_Ignore_Camera( 1 );
	m_item_id = TYPE_UNDEFINED;
	m_item_counter = 0.0f;
	m_item_counter_mod = 0;
	m_box_color = white;
}

/* *** *** *** *** *** *** cLifeDisplay *** *** *** *** *** *** *** *** *** *** *** */


cLifeDisplay :: cLifeDisplay( float x /* = 0.0f */, float y /* = 0.0f */ )
: cStatusText( x, y )
{
	m_sprite_array = ARRAY_HUD;
	m_type = TYPE_LIFEDISPLAY;
	m_name = "HUD Life Display";

	Set_Image( pVideo->Get_Surface( "game/life_full.png" ) );
	// disable shadow
	Set_Shadow_Pos( 0 );

	m_box_color = white;

	m_item_counter = 0;
	m_item_counter_mod = 0;
	
	life_amount = 4;
}

cLifeDisplay :: ~cLifeDisplay( void )
{

}

void cLifeDisplay:: Update( void )
{
	if( m_item_counter )
	{

		if( m_item_counter_mod )
		{
			m_item_counter += pFramerate->m_speed_factor * 10.0f;

			if( m_item_counter >= 90.0f )
			{
				m_item_counter_mod = 0;
				m_item_counter = 90.0f;
			}
		}
		else
		{
			m_item_counter -= pFramerate->m_speed_factor * 10.0f;

			if( m_item_counter <= 0.0f )
			{
				m_item_counter_mod = 1;
				m_item_counter = 1.0f;
			}
		}

	}
}

void cLifeDisplay :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_OVERWORLD || Game_Mode == MODE_MENU )
	{
		return;
	}

	Set_Color( m_box_color.red, m_box_color.green, m_box_color.blue );
	cHudSprite::Draw();
}

void cLifeDisplay:: Set_Life( int life_bar )
{
	life_amount = life_bar;

	if( life_bar == 4)
	{
		m_box_color = Color( static_cast<Uint8>(250), 50, 50 );
		Set_Image( pVideo->Get_Surface( "game/life_full.png" ) );
	}
	else if( life_bar == 3 )
	{
		m_box_color = Color( static_cast<Uint8>(250), 200, 150 );
		Set_Image( pVideo->Get_Surface( "game/life_high.png" ) );
	}
	else if( life_bar == 2 )
	{
		m_box_color = Color( static_cast<Uint8>(100), 100, 250 );
		Set_Image( pVideo->Get_Surface( "game/life_mid.png" ) );
	}
	else if ( life_bar == 1)
	{
		m_box_color = Color( static_cast<Uint8>(100), 100, 250 );
		Set_Image( pVideo->Get_Surface( "game/life_low.png" ) );	
	}
	else if ( life_bar == 0)
	{
		m_box_color = Color( static_cast<Uint8>(100), 100, 250 );
		Set_Image( pVideo->Get_Surface( "game/life_bar.png" ) );	
	}
	
	m_box_color = Color( static_cast<Uint8>(158), 132, 103);
}

void cLifeDisplay :: Decrease_Life( int life_decreased)
{
	if (life_decreased > 0)
	{
		if (life_decreased == life_amount || life_decreased > life_amount)
		{
			life_amount = 0;
			m_box_color = Color( static_cast<Uint8>(100), 100, 250 );
			Set_Image( pVideo->Get_Surface( "game/life_bar.png" ) );			
		}
		else
		{
			life_amount = life_amount - life_decreased;
			
			if (life_amount == 3)
			{
				m_box_color = Color( static_cast<Uint8>(100), 100, 250 );
				Set_Image( pVideo->Get_Surface( "game/life_high.png" ) );				
			}
			else if (life_amount == 2)
			{
				m_box_color = Color( static_cast<Uint8>(100), 100, 250 );
				Set_Image( pVideo->Get_Surface( "game/life_mid.png" ) );				
			}
			else if (life_amount == 1)
			{
				m_box_color = Color( static_cast<Uint8>(100), 100, 250 );
				Set_Image( pVideo->Get_Surface( "game/life_low.png" ) );
			}
			else if (life_amount == 0)
			{
				
			}
			
			m_box_color = Color( static_cast<Uint8>(158), 132, 103);
		}
	
	}
}

void cLifeDisplay :: IncreaseLife( int life_increased)
{
	if (life_increased > 0)
	{
		if (life_amount == 4)
		{
			
		}
		else if (life_amount == 3)
		{
			life_amount = 4;
		}
		else if (life_amount == 2)
		{
			if (life_increased > 1)
			{
				life_amount = 4;
			}
			else if (life_increased == 1)
			{
				life_amount++;
			}
		}
		else if (life_amount == 1)
		{
			if(life_increased > 2)
			{
				life_amount = 4;
			}
			else if (life_increased < 3)
			{
				life_amount = life_amount + life_increased;
			}
		}
		
		if (life_amount == 4)
		{
			m_box_color = Color( static_cast<Uint8>(100), 100, 250 );
			Set_Image( pVideo->Get_Surface( "game/life_full.png" ) );		
		}
		else if (life_amount == 3)
		{
			m_box_color = Color( static_cast<Uint8>(100), 100, 250 );
			Set_Image( pVideo->Get_Surface( "game/life_high.png" ) );				
		}
		else if (life_amount == 2)
		{
			m_box_color = Color( static_cast<Uint8>(100), 100, 250 );
			Set_Image( pVideo->Get_Surface( "game/life_mid.png" ) );				
		}
		else if (life_amount == 1)
		{
			m_box_color = Color( static_cast<Uint8>(100), 100, 250 );
			Set_Image( pVideo->Get_Surface( "game/life_low.png" ) );
		}
		
		m_box_color = Color( static_cast<Uint8>(158), 132, 103);
	}
}

void cLifeDisplay :: Reset( void )
{
	life_amount = 4;
	m_item_counter = 0.0f;
	m_item_counter_mod = 0;
	m_box_color = white;
	m_box_color = Color( static_cast<Uint8>(158), 132, 103);
}



/* *** *** *** *** *** *** cDebugDisplay *** *** *** *** *** *** *** *** *** *** *** */

cDebugDisplay :: cDebugDisplay( float x /* = 0.0f */, float y /* = 0.0f */ )
: cStatusText( x, y )
{
	m_sprite_array = ARRAY_HUD;
	m_type = TYPE_DEBUGDISPLAY;
	m_name = "HUD Debug";

	m_text.clear();
	m_text_old.clear();

	// debug box text data
	m_game_mode_last = MODE_NOTHING;
	m_level_old = ".";
	m_obj_counter = -1;
	m_pass_counter = -1;
	m_mass_counter = -1;
	m_enemy_counter = -1;
	m_active_counter = -1;

	// debug text window
	m_window_debug_text = CEGUI::WindowManager::getSingleton().loadWindowLayout( "debugtext.layout" );
	pGuiSystem->getGUISheet()->addChildWindow( m_window_debug_text );
	// debug text
	m_text_debug_text = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_debugmessage" ));
	// hide
	m_text_debug_text->setVisible( 0 );

	// debug box positions
	float tempx = static_cast<float>(game_res_w) - 200;
	float tempy = static_cast<float>(game_res_h) / 2 - 250;

	for( unsigned int i = 0; i < 23; i++ )
	{
		m_sprites.push_back( new cHudSprite( NULL, tempx, tempy ) );
		m_sprites[i]->Set_Shadow( black, 1 );
		m_sprites[i]->m_pos_z = m_pos_z;

		// not the framerate text
		if( i > 2 )
		{
			tempy += 19.0f;
		}

		// active box
		if( i > 10 && i < 16 )
		{
			m_sprites[i]->Set_Pos_X( m_sprites[i]->m_start_pos_x + 5.0f, 1 );
		}
	}

	// fps display position
	m_sprites[0]->Set_Pos( 15.0f, 5.0f, 1 );
	// average
	m_sprites[1]->Set_Pos( 290.0f, 5.0f, 1 );
	// speed factor
	m_sprites[2]->Set_Pos( 480.0f, 5.0f, 1 );

	// Debug type text
	m_sprites[4]->Set_Image( pFont->Render_Text( pFont->m_font_small, _("Level"), lightblue ), 0, 1 );
	m_sprites[16]->Set_Image( pFont->Render_Text( pFont->m_font_small, _("Player"), lightblue ), 0, 1 );

	m_counter = 0.0f;
}

cDebugDisplay :: ~cDebugDisplay( void )
{
	pGuiSystem->getGUISheet()->removeChildWindow( m_window_debug_text );
	CEGUI::WindowManager::getSingleton().destroyWindow( m_window_debug_text );

	for( HudSpriteList::iterator itr = m_sprites.begin(), itr_end = m_sprites.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	m_sprites.clear();
}

void cDebugDisplay :: Update( void )
{
	// no text to display
	if( m_text.empty() )
	{
		return;
	}

	// if time reached hide the text display
	if( m_counter <= 0 )
	{
		m_text.clear();
		m_text_old.clear();

		m_text_debug_text->setVisible( 0 );
		return;
	}

	// update counter
	m_counter -= pFramerate->m_speed_factor;

	// set new text
	if( m_text.compare( m_text_old ) != 0 )
	{
		m_text_old = m_text;
		CEGUI::String gui_text = reinterpret_cast<const CEGUI::utf8*>(m_text.c_str());

		// display the new text
		m_text_debug_text->setText( gui_text );
		m_text_debug_text->setVisible( 1 );

		// update position
		CEGUI::Font *font = CEGUI::FontManager::getSingleton().getFont( "bluebold_medium" );
		float text_width = font->getTextExtent( gui_text ) * global_downscalex;

		if( text_width > 800.0f )
		{
			text_width = 800.0f;
		}

		m_text_debug_text->setWidth( CEGUI::UDim( 0, ( text_width + 15 ) * global_upscalex ) );
		m_text_debug_text->setXPosition( CEGUI::UDim( 0, ( ( game_res_w * 0.5f ) - text_width * 0.5f ) * global_upscalex ) );
	}
}

void cDebugDisplay :: Draw( cSurface_Request *request /* = NULL */ )
{
	// debug mod info
	Draw_Debug_Mode();
	Draw_Performance_Debug_Mode();
}

void cDebugDisplay :: Set_Text( const std::string &ntext, float display_time /* = speedfactor_fps * 2.0f */ )
{
	m_text = ntext;

	if( m_text.empty() )
	{
		m_text = m_text_old;
		m_text_old.empty();

		m_counter = 0;
		return;
	}

	m_counter = display_time;
}

void cDebugDisplay :: Draw_fps( void )
{
	// ### Frames per Second
	m_sprites[0]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, _("FPS : best ") + int_to_string( static_cast<int>(pFramerate->m_fps_best) ) + _(", worst ") + int_to_string( static_cast<int>(pFramerate->m_fps_worst) ) + _(", current ") + int_to_string( static_cast<int>(pFramerate->m_fps) ), white ), 0, 1 );
	// average
	m_sprites[1]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, _("average ") + int_to_string( static_cast<int>(pFramerate->m_fps_average) ), white ), 0, 1 );
	// speed factor
	m_sprites[2]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, _("Speed factor ") + float_to_string( pFramerate->m_speed_factor, 4 ), white ), 0, 1 );
}

void cDebugDisplay :: Draw_Debug_Mode( void )
{
	if( !game_debug || ( Game_Mode == MODE_LEVEL && pPlayer->maryo_type == MARYO_DEAD ) ) 
	{
		return;
	}

	/* ### Extend
	- Ground Object name, position and velocity
	*/

	// black background
	Color color = blackalpha128;
	pVideo->Draw_Rect( static_cast<float>(game_res_w) - 205, static_cast<float>(game_res_h) * 0.08f, 190, 390, m_pos_z - 0.00001f, &color );

	// Active objects rect

	// create request
	cRect_Request *request = new cRect_Request();
	pVideo->Draw_Rect( m_sprites[11]->m_pos_x - 4, m_sprites[11]->m_pos_y - 4, 135, 95, m_pos_z, &white, request );

	// not filled
	request->filled = 0;

	// add request
	pRenderer->Add( request );

	// fps
	Draw_fps();

	std::string temp_text;

	// Camera position
	temp_text = _("Camera : X ") + int_to_string( static_cast<int>(pActive_Camera->x) ) + ", Y " + int_to_string( static_cast<int>(pActive_Camera->y) );
	m_sprites[3]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );

	// Level information
	if( pActive_Level->m_level_filename.compare( m_level_old ) != 0 ) 
	{
		std::string lvl_text = _("Name : ") + Trim_Filename( pActive_Level->m_level_filename, 0, 0 );
		m_level_old = pActive_Level->m_level_filename;

		m_sprites[5]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, lvl_text, white ), 0, 1 );
	}

	// Level objects
	if( m_obj_counter != static_cast<int>(pActive_Sprite_Manager->size()) )
	{
		m_obj_counter = pActive_Sprite_Manager->size();

		temp_text = _("Objects : ") + int_to_string( m_obj_counter );
		m_sprites[6]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	}
	// Passive
	if( m_pass_counter != static_cast<int>(pActive_Sprite_Manager->Get_Size_Array( ARRAY_PASSIVE )) )
	{
		m_pass_counter = pActive_Sprite_Manager->Get_Size_Array( ARRAY_PASSIVE );

		temp_text = _("Passive : ") + int_to_string( m_pass_counter );
		m_sprites[7]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	}
	// Massive
	if( m_mass_counter != static_cast<int>(pActive_Sprite_Manager->Get_Size_Array( ARRAY_MASSIVE )) )
	{
		m_mass_counter = pActive_Sprite_Manager->Get_Size_Array( ARRAY_MASSIVE );

		temp_text = _("Massive : ") + int_to_string( m_mass_counter );
		m_sprites[8]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	}
	// Enemy
	if( m_enemy_counter != static_cast<int>(pActive_Sprite_Manager->Get_Size_Array( ARRAY_ENEMY )) ) 
	{
		m_enemy_counter = pActive_Sprite_Manager->Get_Size_Array( ARRAY_ENEMY );

		temp_text = _("Enemy : ") + int_to_string( m_enemy_counter );
		m_sprites[9]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	}
	// Active
	if( m_active_counter != static_cast<int>(pActive_Sprite_Manager->Get_Size_Array( ARRAY_ACTIVE )) )
	{
		m_active_counter = pActive_Sprite_Manager->Get_Size_Array( ARRAY_ACTIVE );

		temp_text = _("Active : ") + int_to_string( m_active_counter );
		m_sprites[10]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );

		// Halfmassive
		unsigned int halfmassive = 0;

		for( cSprite_List::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->m_type == TYPE_HALFMASSIVE )
			{
				halfmassive++;
			}
		}

		temp_text = _("Halfmassive : ") + int_to_string( halfmassive );
		m_sprites[11]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );

		// Moving Platform
		unsigned int moving_platform = 0;

		for( cSprite_List::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->m_type == TYPE_MOVING_PLATFORM )
			{
				moving_platform++;
			}
		}

		temp_text = _("Moving Platform : ") + int_to_string( moving_platform );
		m_sprites[12]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );

		// Goldbox
		unsigned int goldbox = 0;

		for( cSprite_List::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->m_type == TYPE_BONUSBOX )
			{
				cBonusBox *bonusbox = static_cast<cBonusBox *>(obj);

				if( bonusbox->box_type == TYPE_GOLDPIECE )
				{
					goldbox++;
				}
			}
		}

		temp_text = _("Goldbox : ") + int_to_string( goldbox );
		m_sprites[13]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );

		// Bonusbox
		unsigned int bonusbox_count = 0;

		for( cSprite_List::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->m_type == TYPE_BONUSBOX )
			{
				cBonusBox *bonusbox = static_cast<cBonusBox *>(obj);

				if( bonusbox->box_type != TYPE_GOLDPIECE )
				{
					bonusbox_count++;
				}
			}
		}

		temp_text = _("Bonusbox : ") + int_to_string( bonusbox_count );
		m_sprites[14]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );

		// Other
		unsigned int active_other = m_active_counter - halfmassive - moving_platform - goldbox - bonusbox_count;

		temp_text = _("Other : ") + int_to_string( active_other );
		m_sprites[15]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	}

	// Player information
	// position x
	temp_text = "X1 " + float_to_string( pActive_Player->m_pos_x, 4 ) + "  X2 " + float_to_string( pPlayer->m_col_rect.m_x + pPlayer->m_col_rect.m_w, 4 );
	m_sprites[17]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	// position y
	temp_text = "Y1 " + float_to_string( pActive_Player->m_pos_y, 4 ) + "  Y2 " + float_to_string( pPlayer->m_col_rect.m_y + pPlayer->m_col_rect.m_h, 4 );
	m_sprites[18]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	// velocity
	temp_text = _("Velocity X ") + float_to_string( pPlayer->m_velx, 2 ) + " ,Y " + float_to_string( pPlayer->m_vely, 2 );
	m_sprites[19]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	// moving state
	temp_text = _("Moving State ") + int_to_string( static_cast<int>(pPlayer->m_state) );
	m_sprites[20]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	// ground type
	std::string ground_type;
	if( pPlayer->m_ground_object )
	{
		ground_type = int_to_string( pPlayer->m_ground_object->m_massive_type ) + " (" + Get_Massive_Type_Name( pPlayer->m_ground_object->m_massive_type ) + ")";
	}
	temp_text = _("Ground ") + ground_type;
	m_sprites[21]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	// game mode
	if( Game_Mode != m_game_mode_last )
	{
		m_sprites[22]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, _("Game Mode : ") + int_to_string( Game_Mode ), white ), 0, 1 );
	}

	// draw text
	for( HudSpriteList::iterator itr = m_sprites.begin(), itr_end = m_sprites.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}
}

void cDebugDisplay :: Draw_Performance_Debug_Mode( void )
{
	if( !game_debug_performance ) 
	{
		return;
	}


	std::string temp_text;
	float ypos = game_res_h * 0.08f;

	// black background
	Color color = blackalpha128;
	pVideo->Draw_Rect( 15, ypos, 190, 390, m_pos_z - 0.00001f, &color );

	// don't draw it twice
	if( !game_debug )
	{
		// fps
		Draw_fps();
		m_sprites[0]->Draw();
		m_sprites[1]->Draw();
		m_sprites[2]->Draw();
	}

	vector<std::string> text_strings;
	// draw
	text_strings.push_back( _("Draw") );
	// overworld
	if( Game_Mode == MODE_OVERWORLD )
	{
		text_strings.push_back( _("World : ") + int_to_string( pFramerate->m_perf_timer[PERF_DRAW_OVERWORLD]->ms ) );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
	}
	// menu
	else if( Game_Mode == MODE_MENU )
	{
		text_strings.push_back( _("Menu : ") + int_to_string( pFramerate->m_perf_timer[PERF_DRAW_MENU]->ms ) );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
	}
	// level settings
	else if( Game_Mode == MODE_LEVEL_SETTINGS )
	{
		text_strings.push_back( _("Level Settings : ") + int_to_string( pFramerate->m_perf_timer[PERF_DRAW_LEVEL_SETTINGS]->ms ) );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
	}
	// level (default)
	else
	{
		text_strings.push_back( _("Level Layer 1 : ") + int_to_string( pFramerate->m_perf_timer[PERF_DRAW_LEVEL_LAYER1]->ms ) );
		text_strings.push_back( _("Level Player : ") + int_to_string( pFramerate->m_perf_timer[PERF_DRAW_LEVEL_PLAYER]->ms ) );
		text_strings.push_back( _("Level Layer 2 : ") + int_to_string( pFramerate->m_perf_timer[PERF_DRAW_LEVEL_LAYER2]->ms ) );
		text_strings.push_back( _("Level Hud : ") + int_to_string( pFramerate->m_perf_timer[PERF_DRAW_LEVEL_HUD]->ms ) );
		text_strings.push_back( _("Level Editor : ") + int_to_string( pFramerate->m_perf_timer[PERF_DRAW_LEVEL_EDITOR]->ms ) );
	}
	text_strings.push_back( _("Mouse : ") + int_to_string( pFramerate->m_perf_timer[PERF_DRAW_MOUSE]->ms ) );
	// update
	text_strings.push_back( _("Update") );
	// overworld
	if( Game_Mode == MODE_OVERWORLD )
	{
		text_strings.push_back( _("World : ") + int_to_string( pFramerate->m_perf_timer[PERF_UPDATE_OVERWORLD]->ms ) );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
	}
	// menu
	else if( Game_Mode == MODE_MENU )
	{
		text_strings.push_back( _("Menu : ") + int_to_string( pFramerate->m_perf_timer[PERF_UPDATE_MENU]->ms ) );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
	}
	// level settings
	else if( Game_Mode == MODE_LEVEL_SETTINGS )
	{
		text_strings.push_back( _("Level Settings : ") + int_to_string( pFramerate->m_perf_timer[PERF_UPDATE_LEVEL_SETTINGS]->ms ) );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
	}
	// level (default)
	else
	{
		text_strings.push_back( _("process input : ") + int_to_string( pFramerate->m_perf_timer[PERF_UPDATE_PROCESS_INPUT]->ms ) );
		text_strings.push_back( _("level : ") + int_to_string( pFramerate->m_perf_timer[PERF_UPDATE_LEVEL]->ms ) );
		text_strings.push_back( _("level editor : ") + int_to_string( pFramerate->m_perf_timer[PERF_UPDATE_LEVEL_EDITOR]->ms ) );
		text_strings.push_back( _("hud : ") + int_to_string( pFramerate->m_perf_timer[PERF_UPDATE_HUD]->ms ) );
		text_strings.push_back( _("player : ") + int_to_string( pFramerate->m_perf_timer[PERF_UPDATE_PLAYER]->ms ) );
		text_strings.push_back( _("player collisions : ") + int_to_string( pFramerate->m_perf_timer[PERF_UPDATE_PLAYER_COLLISIONS]->ms ) );
		text_strings.push_back( _("level late : ") + int_to_string( pFramerate->m_perf_timer[PERF_UPDATE_LATE_LEVEL]->ms ) );
		text_strings.push_back( _("level collisions : ") + int_to_string( pFramerate->m_perf_timer[PERF_UPDATE_LEVEL_COLLISIONS]->ms ) );
		text_strings.push_back( _("camera : ") + int_to_string( pFramerate->m_perf_timer[PERF_UPDATE_CAMERA]->ms ) );
	}

	// render
	text_strings.push_back( _("Render") );
	text_strings.push_back( _("Game : ") + int_to_string( pFramerate->m_perf_timer[PERF_RENDER_GAME]->ms ) );
	text_strings.push_back( _("Gui : ") + int_to_string( pFramerate->m_perf_timer[PERF_RENDER_GUI]->ms ) );
	text_strings.push_back( _("Buffer : ") + int_to_string( pFramerate->m_perf_timer[PERF_RENDER_BUFFER]->ms ) );

	unsigned int pos = 0;

	for( vector<std::string>::iterator itr = text_strings.begin(), itr_end = text_strings.end(); itr != itr_end; ++itr )
	{
		// sections
		float xpos = 20;
		ypos += 12;

		// move non header a bit to the right right
		if( pos != 0 && pos != 7 && pos != 17 )
		{
			xpos += 10;
		}
		// if new group starts move a bit more down
		if( pos == 7 || pos == 17 )
		{
			ypos += 10;
		}

		std::string current_text = (*itr);

		cGL_Surface *surface_temp = pFont->Render_Text( pFont->m_font_small, current_text, white );

		// create request
		cSurface_Request *request = new cSurface_Request();
		surface_temp->Blit( xpos, ypos, m_pos_z, request );
		request->delete_texture = 1;

		// shadow
		request->shadow_pos = 1;
		request->shadow_color = black;

		// add request
		pRenderer->Add( request );

		surface_temp->m_auto_del_img = 0;
		delete surface_temp;
		
		pos++;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cPlayerPoints *pHud_Points = NULL;
cDebugDisplay *pHud_Debug = NULL;
cGoldDisplay *pHud_Goldpieces = NULL;
cLiveDisplay *pHud_Lives = NULL;
cTimeDisplay *pHud_Time = NULL;
cItemBox *pHud_Itembox = NULL;
cLifeDisplay *pHud_LifeDisplay = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
