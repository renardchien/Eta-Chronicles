/***************************************************************************
 * level_entry.cpp  -  entry point to enter a level
 *
 * Copyright (C) 2007 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../objects/level_entry.h"
#include "../player/player.h"
#include "../core/game_core.h"
#include "../audio/audio.h"
#include "../core/framerate.h"
#include "../core/main.h"
#include "../video/gl_surface.h"
#include "../video/font.h"
#include "../video/renderer.h"
#include "../level/level.h"
#include "../core/i18n.h"
// CEGUI
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel_Entry :: cLevel_Entry( float x, float y )
: cAnimated_Sprite( x, y )
{
	cLevel_Entry::Init();
}

cLevel_Entry :: cLevel_Entry( CEGUI::XMLAttributes &attributes )
: cAnimated_Sprite()
{
	cLevel_Entry::Init();
	cLevel_Entry::Create_From_Stream( attributes );
}

cLevel_Entry :: ~cLevel_Entry( void )
{
	if( editor_entry_name )
	{
		delete editor_entry_name;
		editor_entry_name = NULL;
	}
}

void cLevel_Entry :: Init( void )
{
	m_sprite_array = ARRAY_ACTIVE;
	m_type = TYPE_LEVEL_ENTRY;
	m_massive_type = MASS_PASSIVE;
	m_editor_pos_z = 0.112f;
	m_player_range = 1000;

	// size
	m_rect.m_w = 10;
	m_rect.m_h = 20;
	m_col_rect.m_w = m_rect.m_w;
	m_col_rect.m_h = m_rect.m_h;
	m_start_rect.m_w = m_rect.m_w;
	m_start_rect.m_h = m_rect.m_h;

	entry_type = LEVEL_ENTRY_WARP;
	Set_Direction( DIR_UP );

	editor_color = lightblue;
	editor_color.alpha = 128;

	editor_entry_name = NULL;
}

cLevel_Entry *cLevel_Entry :: Copy( void )
{
	cLevel_Entry *level_entry = new cLevel_Entry( m_start_pos_x, m_start_pos_y );
	level_entry->Set_Type( entry_type );
	level_entry->Set_Direction( m_start_direction );
	level_entry->Set_Name( entry_name );

	return level_entry;
}

void cLevel_Entry :: Create_From_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// type
	Set_Type( static_cast<Level_Entry_type>(attributes.getValueAsInteger( "type", entry_type )) );
	// name
	Set_Name( attributes.getValueAsString( "name" ).c_str() );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( m_start_direction ) ).c_str() ) );
}

void cLevel_Entry :: Save_To_Stream( ofstream &file )
{
	// begin level_entry
	file << "\t<level_entry>" << std::endl;

	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(m_start_pos_x) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(m_start_pos_y) << "\" />" << std::endl;
	// type
	file << "\t\t<Property name=\"type\" value=\"" << entry_type << "\" />" << std::endl;
	if( entry_type == LEVEL_ENTRY_WARP )
	{
		// direction
		file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( m_start_direction ) << "\" />" << std::endl;
	}
	// name
	if( !entry_name.empty() )
	{
		file << "\t\t<Property name=\"name\" value=\"" << string_to_xml_string( entry_name ) << "\" />" << std::endl;
	}

	// end level_entry
	file << "\t</level_entry>" << std::endl;
}

void cLevel_Entry :: Set_Direction( const ObjectDirection dir )
{
	// already set
	if( m_direction == dir )
	{
		return;
	}

	cAnimated_Sprite::Set_Direction( dir, 1 );

	Create_Name();
}

void cLevel_Entry :: Create_Name( void )
{
	m_name = _("Level Entry");

	if( entry_type == LEVEL_ENTRY_BEAM )
	{
		m_name += _(" Beam");
	}
	else if( entry_type == LEVEL_ENTRY_WARP )
	{
		m_name += _(" Warp");

		if( m_direction == DIR_UP )
		{
			m_name += " U";
		}
		else if( m_direction == DIR_LEFT )
		{
			m_name += " L";
		}
		else if( m_direction == DIR_DOWN )
		{
			m_name += " D";
		}
		else if( m_direction == DIR_RIGHT )
		{
			m_name += " R";
		}
	}
}

void cLevel_Entry :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( !m_valid_draw )
	{
		return;
	}

	// draw color rect
	pVideo->Draw_Rect( m_col_rect.m_x - pActive_Camera->x, m_col_rect.m_y - pActive_Camera->y, m_col_rect.m_w, m_col_rect.m_h, m_editor_pos_z, &editor_color );

	// draw entry name
	if( editor_entry_name )
	{
		// create request
		cSurface_Request *surface_request = new cSurface_Request();
		// blit
		editor_entry_name->Blit( m_col_rect.m_x + m_col_rect.m_w + 5 - pActive_Camera->x, m_col_rect.m_y - pActive_Camera->y, m_editor_pos_z, surface_request );
		surface_request->shadow_pos = 2;
		surface_request->shadow_color = lightgreyalpha64;
		// add request
		pRenderer->Add( surface_request );
	}
}

void cLevel_Entry :: Activate( void )
{
	// warp player in
	if( entry_type == LEVEL_ENTRY_WARP )
	{
		pAudio->Play_Sound( "leave_pipe.ogg" );

		// set state to linked to stop checking for on ground objects which sometimes changes the position
		Moving_state player_state = pPlayer->m_state;
		pPlayer->m_state = STA_OBJ_LINKED;

		pPlayer->Stop_Ducking();
		pPlayer->Reset_On_Ground();

		// set position
		pPlayer->Set_Pos( Get_Player_Pos_X(), Get_Player_Pos_Y() );

		// set image
		if( m_direction == DIR_UP || m_direction == DIR_DOWN )
		{
			pPlayer->Set_Image_Num( MARYO_IMG_FALL + pPlayer->m_direction );
		}
		else if( m_direction == DIR_LEFT || m_direction == DIR_RIGHT )
		{
			pPlayer->Set_Image_Num( pPlayer->m_direction );

			// set rotation
			if( m_direction == DIR_RIGHT )
			{
				pPlayer->Set_Rotation_Z( 90 );
			}
			else if( m_direction == DIR_LEFT )
			{
				pPlayer->Set_Rotation_Z( 270 );
			}
		}

		// change position z to be behind massive for the animation
		float player_posz = pPlayer->m_pos_z;
		pPlayer->m_pos_z = 0.0799f;

		// move slowly out
		while( 1 )
		{
			if( m_direction == DIR_DOWN )
			{
				pPlayer->Move( 0.0f, 2.8f );

				if( pPlayer->m_pos_y > m_rect.m_y + m_rect.m_h )
				{
					break;
				}
			}
			else if( m_direction == DIR_UP )
			{
				pPlayer->Move( 0.0f, -2.8f );

				if( pPlayer->m_col_rect.m_y + pPlayer->m_col_rect.m_h < m_rect.m_y )
				{
					break;
				}
			}
			else if( m_direction == DIR_RIGHT )
			{
				pPlayer->Move( 2.0f, 0.0f );

				if( pPlayer->m_pos_x > m_rect.m_x + m_rect.m_w )
				{
					break;
				}
			}
			else if( m_direction == DIR_LEFT )
			{
				pPlayer->Move( -2.0f, 0.0f );

				if( pPlayer->m_col_rect.m_x + pPlayer->m_col_rect.m_w < m_rect.m_x )
				{
					break;
				}
			}
			else
			{
				break;
			}

			// update audio
			pAudio->Update();
			// center camera
			pActive_Camera->Center();
			// keep global effect particles on screen
			pActive_Level->m_global_effect->Update_Particles();
			// draw
			Draw_Game();

			pVideo->Render();
			pFramerate->Update();
		}

		// set position z back
		pPlayer->m_pos_z = player_posz;
		// set state back
		pPlayer->m_state = player_state;

		if( m_direction == DIR_RIGHT || m_direction == DIR_LEFT )
		{
			pPlayer->Set_Rotation_Z( 0 );
		}
	}
	// beam player in
	else if( entry_type == LEVEL_ENTRY_BEAM )
	{
		// set position
		pPlayer->Set_Pos( Get_Player_Pos_X(), Get_Player_Pos_Y() );
	}

	pPlayer->Clear_Collisions();
}

void cLevel_Entry :: Set_Type( Level_Entry_type ltype )
{
	entry_type = ltype;

	Create_Name();
}

float cLevel_Entry :: Get_Player_Pos_X( void ) const
{
	if( entry_type == LEVEL_ENTRY_WARP )
	{
		// left
		if( m_direction == DIR_LEFT )
		{
			return m_col_rect.m_x - pPlayer->m_col_pos.m_y + m_col_rect.m_w;
		}
		// right
		else if( m_direction == DIR_RIGHT )
		{
			return m_col_rect.m_x - pPlayer->m_col_pos.m_y - m_col_rect.m_w - pPlayer->m_col_rect.m_w;
		}

		// up/down
		return m_col_rect.m_x - pPlayer->m_col_pos.m_x + ( m_col_rect.m_w * 0.5f ) - ( pPlayer->m_col_rect.m_w * 0.5f );
	}
	else if( entry_type == LEVEL_ENTRY_BEAM )
	{
		return m_col_rect.m_x + ( m_col_rect.m_w * 0.5f ) - pPlayer->m_col_pos.m_y - ( pPlayer->m_col_rect.m_w * 0.5f );
	}

	return 0;
}

float cLevel_Entry :: Get_Player_Pos_Y( void ) const
{
	if( entry_type == LEVEL_ENTRY_WARP )
	{
		// up
		if( m_direction == DIR_UP )
		{
			return m_col_rect.m_y - pPlayer->m_col_pos.m_y + m_col_rect.m_h;
		}
		// down
		else if( m_direction == DIR_DOWN )
		{
			return m_col_rect.m_y - pPlayer->m_col_pos.m_y - 5 - pPlayer->m_rect.m_h;
		}

		// left/right
		return m_col_rect.m_y - pPlayer->m_col_pos.m_y + ( m_col_rect.m_h * 0.5f ) - ( pPlayer->m_col_rect.m_h * 0.5f );
	}
	else if( entry_type == LEVEL_ENTRY_BEAM )
	{
		return m_col_rect.m_y + m_col_rect.m_h - pPlayer->m_col_pos.m_y - pPlayer->m_col_rect.m_h;
	}

	return 0;
}

void cLevel_Entry :: Set_Name( const std::string &str_name )
{
	// delete editor image
	if( editor_entry_name )
	{
		delete editor_entry_name;
		editor_entry_name = NULL;
	}

	// Set new name
	entry_name = str_name;

	// if empty don't create editor image
	if( entry_name.empty() )
	{
		return;
	}

	editor_entry_name = pFont->Render_Text( pFont->m_font_small, entry_name, white );
}

bool cLevel_Entry :: Is_Draw_Valid( void )
{
	// if editor not enabled
	if( !editor_enabled )
	{
		return 0;
	}

	// if not active or not visible on the screen
	if( !m_active || !Is_Visible_On_Screen() )
	{
		return 0;
	}

	return 1;
}

void cLevel_Entry :: Editor_Activate( void )
{
	// get window manager
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// warp
	if( entry_type == LEVEL_ENTRY_WARP )
	{
		// direction
		CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "level_entry_direction" ));
		Editor_Add( UTF8_("Direction"), UTF8_("The direction to come out"), combobox, 100, 105 );

		combobox->addItem( new CEGUI::ListboxTextItem( "up" ) );
		combobox->addItem( new CEGUI::ListboxTextItem( "down" ) );
		combobox->addItem( new CEGUI::ListboxTextItem( "right" ) );
		combobox->addItem( new CEGUI::ListboxTextItem( "left" ) );
		combobox->setText( Get_Direction_Name( m_start_direction ) );

		combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cLevel_Entry::Editor_Direction_Select, this ) );
	}

	// destination entry
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "level_entry_name" ));
	Editor_Add( UTF8_("Name"), UTF8_("Name for identification"), editbox, 150 );

	editbox->setText( entry_name.c_str() );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cLevel_Entry::Editor_Name_Text_Changed, this ) );

	// init
	Editor_Init();
}

bool cLevel_Entry :: Editor_Direction_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_Id( item->getText().c_str() ) );

	return 1;
}

bool cLevel_Entry :: Editor_Name_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Name( str_text );

	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
