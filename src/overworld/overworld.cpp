/***************************************************************************
 * overworld.cpp  -  Overworld class
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
#include "../overworld/overworld.h"
#include "../audio/audio.h"
#include "../core/game_core.h"
#include "../level/level_editor.h"
#include "../overworld/world_editor.h"
#include "../core/framerate.h"
#include "../gui/menu.h"
#include "../user/preferences.h"
#include "../video/font.h"
#include "../input/mouse.h"
#include "../input/joystick.h"
#include "../input/keyboard.h"
#include "../level/level.h"
#include "../core/i18n.h"
#include "../core/filesystem/filesystem.h"
#include "../core/filesystem/resource_manager.h"
// CEGUI
#include "CEGUIXMLParser.h"
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** cOverworld_description *** *** *** *** *** *** *** *** *** */

cOverworld_description :: cOverworld_description( void )
{
	path = "world_1";
	name = _("Unnamed");
	visible = 1;
	user = 0;

	comment = _("Empty");
}

cOverworld_description :: ~cOverworld_description( void )
{
	//
}

void cOverworld_description :: Load( void )
{
	std::string filename = Get_Full_Path() + "/description.xml";

	// filename not valid
	if( !File_Exists( filename ) )
	{
		printf( "Error : Couldn't open World description file : %s\n", filename.c_str() );
		return;
	}

	// Load Description
	CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/World/Description.xsd", "" );
}

void cOverworld_description :: Save( void )
{
	std::string save_dir = pResource_Manager->user_data_dir + USER_WORLD_DIR + "/" + path;
	std::string filename = save_dir + "/description.xml";

	ofstream file( filename.c_str(), ios::out | ios::trunc );

	if( !file )
	{
		pHud_Debug->Set_Text( _("Couldn't save world description ") + filename );
		return;
	}

	// xml info
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	// begin Description
	file << "<Description>" << std::endl;

	// begin World
	file << "\t<World>" << std::endl;
		// name
		file << "\t\t<Property Name=\"name\" Value=\"" << string_to_xml_string( name ) << "\" />" << std::endl;
		// visible
		file << "\t\t<Property Name=\"visible\" Value=\"" << visible << "\" />" << std::endl;
	// end World
	file << "\t</World>" << std::endl;


	// end Description
	file << "</Description>" << std::endl;
	file.close();
}

std::string cOverworld_description :: Get_Full_Path( void ) const
{
	// return user world if available
	if( File_Exists( pResource_Manager->user_data_dir + USER_WORLD_DIR + "/" + path + "/description.xml" ) )
	{
		return pResource_Manager->user_data_dir + USER_WORLD_DIR + "/" + path;
	}

	// return game world
	return DATA_DIR "/" GAME_OVERWORLD_DIR "/" + path;
}

// XML element start
void cOverworld_description :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	// Property of an Element
	if( element == "Property" )
	{
		xml_attributes.add( attributes.getValueAsString( "Name" ), attributes.getValueAsString( "Value" ) );
	}
}

// XML element end
void cOverworld_description :: elementEnd( const CEGUI::String &element )
{
	if( element != "Property" )
	{
		if( element == "World" )
		{
			handle_world( xml_attributes );
		}
		else if( element == "Description" )
		{
			// ignore
		}
		else if( element.length() )
		{
			printf( "Warning : World Description Unknown element : %s\n", element.c_str() );
		}

		// clear
		xml_attributes = CEGUI::XMLAttributes();
	}
}

void cOverworld_description :: handle_world( const CEGUI::XMLAttributes &attributes )
{
	name = attributes.getValueAsString( "name", name.c_str() ).c_str();
	visible = attributes.getValueAsBool( "visible", 1 );
}

/* *** *** *** *** *** *** *** *** cOverworld *** *** *** *** *** *** *** *** *** */

cOverworld :: cOverworld( void )
{
	m_sprite_manager = new cWorld_Sprite_Manager( this );
	m_description = new cOverworld_description();
	m_layer = new cLayer( this );

	m_engine_version = -1;
	m_last_saved = 0;
	m_background_color = Color();
	m_musicfile = "overworld/land_1.ogg";
	m_hud_world_name = new cHudSprite( NULL, 10, static_cast<float>(game_res_h) - 30, 0 );
	m_hud_world_name->Set_Shadow( black, 1.5f );
	m_hud_level_name = new cHudSprite( NULL, 350, 2, 0 );
	m_hud_level_name->Set_Shadow( black, 1.5f );

	m_next_level = 0;

	m_player_start_waypoint = 0;
	m_player_moving_state = STA_STAY;
}

cOverworld :: ~cOverworld( void )
{
	Unload();

	delete m_sprite_manager;
	delete m_description;
	delete m_layer;
	delete m_hud_level_name;
	delete m_hud_world_name;
}

void cOverworld :: Enter( bool delayed /* = 0 */ )
{
	if( game_exit )
	{
		return;
	}

	// enter on the next update
	if( delayed )
	{
		Game_Action = GA_ENTER_WORLD;
		return;
	}

	// unload level if possible
	if( pActive_Level->m_delayed_unload )
	{
		pActive_Level->Unload();
	}

	// if not loaded
	if( !Is_Loaded() )
	{
		return;
	}

	// change game mode
	Change_Game_Mode( MODE_OVERWORLD );

	// if player start waypoint not set
	if( pOverworld_Player->current_waypoint < 0 )
	{
		pOverworld_Player->Reset();
		pOverworld_Player->Set_Waypoint( m_player_start_waypoint, 1 );
	}

	// if goto next level
	if( m_next_level )
	{
		Goto_Next_Level();
	}

	// if on start waypoint
	if( pOverworld_Player->current_waypoint == static_cast<int>(m_player_start_waypoint) )
	{
		// if player state is walk
		if( m_player_moving_state == STA_WALK )
		{
			// walk to the next Waypoint
			pOverworld_Player->Start_Walk( m_waypoints[pOverworld_Player->current_waypoint]->direction_forward );
		}
	}

	Update_Camera();
}

bool cOverworld :: Load( void )
{
	Unload();

	// set active for loading
	pActive_Overworld = this;

	// description
	m_description->Load();

	// layer
	std::string layer_filename = m_description->Get_Full_Path() + "/layer.xml";

	if( !File_Exists( layer_filename ) )
	{
		printf( "Couldn't find World Layer file : %s from %s\n", layer_filename.c_str(), m_description->path.c_str() );
		return 0;
	}

	m_layer->Load( layer_filename );

	// world
	std::string world_filename = m_description->Get_Full_Path() + "/world.xml";

	if( !File_Exists( world_filename ) )
	{
		printf( "Couldn't find World file : %s from %s\n", world_filename.c_str(), m_description->path.c_str() );
		return 0;
	}

	try
	{
		// parse overworld
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, world_filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/World/World.xsd", "" );
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "Loading World %s CEGUI Exception %s\n", world_filename.c_str(), ex.getMessage().c_str() );
		pHud_Debug->Set_Text( _("World Loading failed : ") + (const std::string)ex.getMessage().c_str() );
	}

	// engine version entry not set
	if( m_engine_version < 0 )
	{
		m_engine_version = 0;
	}

	m_hud_world_name->Set_Image( pFont->Render_Text( pFont->m_font_normal, m_description->name, yellow ), 1, 1 );

	return 1;
}

void cOverworld :: Unload( void )
{
	// not loaded
	if( !Is_Loaded() )
	{
		return;
	}

	// set active for unloading
	pActive_Overworld = this;

	// Waypoints
	m_waypoints.clear();
	// Layer
	m_layer->Delete_All();
	// Objects
	m_sprite_manager->Delete_All();

	// no engine version
	m_engine_version = -1;
	m_last_saved = 0;

	pActive_Overworld = NULL;
}

void cOverworld :: Save( void )
{
	pAudio->Play_Sound( "editor/save.ogg" );

	std::string save_dir = pResource_Manager->user_data_dir + USER_WORLD_DIR + "/" + m_description->path;
	// Create directory if new world
	if( !Dir_Exists( save_dir ) )
	{
		Create_Directory( save_dir );
	}

	std::string filename = save_dir + "/world.xml";

	ofstream file( filename.c_str(), ios::out | ios::trunc );

	if( !file )
	{
		pHud_Debug->Set_Text( _("Couldn't save world ") + filename );
		return;
	}

	// xml info
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	// begin overworld
	file << "<overworld>" << std::endl;

	// begin info
	file << "\t<information>" << std::endl;
	// game version
	file << "\t\t<Property name=\"game_version\" value=\"" << smc_version << "\" />" << std::endl;
	// engine version
	file << "\t\t<Property name=\"engine_version\" value=\"" << world_engine_version << "\" />" << std::endl;
	// time ( seconds since 1970 )
	file << "\t\t<Property name=\"save_time\" value=\"" << time( NULL ) << "\" />" << std::endl;
	// end info
	file << "\t</information>" << std::endl;

	// begin settings
	file << "\t<settings>" << std::endl;
	// music
	file << "\t\t<Property name=\"music\" value=\"" << string_to_xml_string( m_musicfile ) << "\" />" << std::endl;
	// end settings
	file << "\t</settings>" << std::endl;

	// begin background
	file << "\t<background>" << std::endl;
	// color
	file << "\t\t<Property name=\"color_red\" value=\"" << static_cast<int>(m_background_color.red) << "\" />" << std::endl;
	file << "\t\t<Property name=\"color_green\" value=\"" << static_cast<int>(m_background_color.green) << "\" />" << std::endl;
	file << "\t\t<Property name=\"color_blue\" value=\"" << static_cast<int>(m_background_color.blue) << "\" />" << std::endl;
	// end background
	file << "\t</background>" << std::endl;

	// begin player
	file << "\t<player>" << std::endl;
	// start waypoint
	file << "\t\t<Property name=\"waypoint\" value=\"" << m_player_start_waypoint << "\" />" << std::endl;
	// moving state
	file << "\t\t<Property name=\"moving_state\" value=\"" << static_cast<int>(m_player_moving_state) << "\" />" << std::endl;
	// end player
	file << "\t</player>" << std::endl;
	// objects
	for( cSprite_List::iterator itr = m_sprite_manager->objects.begin(), itr_end = m_sprite_manager->objects.end(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		// skip spawned and destroyed objects
		if( obj->m_spawned || obj->m_auto_destroy )
		{
			continue;
		}

		// save to file stream
		obj->Save_To_Stream( file );
	}

	// end overworld
	file << "</overworld>" << std::endl;
	file.close();

	// save Layer
	if( m_layer->Save( save_dir + "/layer.xml" ) )
	{
		printf( "Saving World %s Layer failed\n", m_description->name.c_str() );
	}

	// save description
	m_description->Save();

	// show info
	pHud_Debug->Set_Text( _("World ") + m_description->name + _(" saved") );
}

void cOverworld :: Draw( void )
{
	// Background
	pVideo->Clear_Screen();
	Draw_Layer_1();

	// Player
	pOverworld_Player->Draw();
	// Hud
	Draw_HUD();

	// Editor
	pWorld_Editor->Draw();
}

void cOverworld :: Draw_Layer_1( void )
{
	pVideo->Draw_Rect( NULL, 0.0001f, &m_background_color );

	// Map
	m_sprite_manager->Draw_Items();
	// Animations
	pAnimation_Manager->Draw();
}

void cOverworld :: Draw_HUD( void )
{
	// if not editor mode
	if( !editor_world_enabled )
	{
		// Background
		Color color = Color( static_cast<Uint8>(230), 170, 0, 128 );
		pVideo->Draw_Rect( 0, 0, static_cast<float>(game_res_w), 30, 0.12f, &color );
		// Line
		color = Color( static_cast<Uint8>(200), 150, 0, 128 );
		pVideo->Draw_Rect( 0, 30, static_cast<float>(game_res_w), 5, 0.121f, &color );

		// Overworld name and level
		m_hud_world_name->Draw();
		m_hud_level_name->Draw();
	}

	// hud
	pHud_Manager->Draw();
}

void cOverworld :: Update( void )
{
	// editor
	pWorld_Editor->Process_Input();

	if( !editor_world_enabled )
	{
		// Camera
		Update_Camera();
		// Map
		m_sprite_manager->Update_Items();
		// Player
		pOverworld_Player->Update();
		// Animations
		pAnimation_Manager->Update();
	}

	// hud
	pHud_Manager->Update();
	// Editor
	pWorld_Editor->Update();
}

void cOverworld :: Update_Camera( void )
{
	if( editor_world_enabled )
	{
		return;
	}

	// todo : move to a Process_Input function
	if( pOverworld_Manager->cameramode )
	{
		if( pKeyboard->keys[pPreferences->m_key_right] || ( pJoystick->right && pPreferences->m_joy_enabled ) )
		{
			pActive_Camera->Move( pFramerate->m_speed_factor * 15, 0 );
		}
		else if( pKeyboard->keys[pPreferences->m_key_left] || ( pJoystick->left && pPreferences->m_joy_enabled ) )
		{
			pActive_Camera->Move( pFramerate->m_speed_factor * -15, 0 );
		}
		if( pKeyboard->keys[pPreferences->m_key_up] || ( pJoystick->up && pPreferences->m_joy_enabled ) )
		{
			pActive_Camera->Move( 0, pFramerate->m_speed_factor * -15 );
		}
		else if( pKeyboard->keys[pPreferences->m_key_down] || ( pJoystick->down && pPreferences->m_joy_enabled ) )
		{
			pActive_Camera->Move( 0, pFramerate->m_speed_factor * 15 );
		}
	}
	// default player camera
	else
	{
		pActive_Camera->Update();
	}
}

bool cOverworld :: Key_Down( SDLKey key )
{
	if( key == SDLK_LEFT )
	{
		if( !pOverworld_Manager->cameramode && !editor_world_enabled )
		{
			pOverworld_Player->Action_Interact( INP_LEFT );
		}
		return 0;
	}
	else if( key == SDLK_RIGHT )
	{
		if( !pOverworld_Manager->cameramode && !editor_world_enabled )
		{
			pOverworld_Player->Action_Interact( INP_RIGHT );
		}
		return 0;
	}
	else if( key == SDLK_UP )
	{
		if( !pOverworld_Manager->cameramode && !editor_world_enabled )
		{
			pOverworld_Player->Action_Interact( INP_UP );
		}
		return 0;
	}
	else if( key == SDLK_DOWN )
	{
		if( !pOverworld_Manager->cameramode && !editor_world_enabled )
		{
			pOverworld_Player->Action_Interact( INP_DOWN );
		}
		return 0;
	}
	else if( key == SDLK_c && !editor_world_enabled )
	{
		pOverworld_Manager->cameramode = !pOverworld_Manager->cameramode;
	}
	else if( key == SDLK_F8 )
	{
		pWorld_Editor->Toggle();
	}
	else if( key == SDLK_d && input_event.key.keysym.mod & KMOD_CTRL )
	{
		pOverworld_Manager->debugmode = !pOverworld_Manager->debugmode;
		game_debug = pOverworld_Manager->debugmode;
	}
	else if( key == SDLK_l && pOverworld_Manager->debugmode )
	{
		// toggle layer drawing
		pOverworld_Manager->draw_layer = !pOverworld_Manager->draw_layer;
	}
	else if( pKeyboard->keys[SDLK_g] && pKeyboard->keys[SDLK_o] && pKeyboard->keys[SDLK_d] )
	{
		// all waypoint access
		pActive_Overworld->Set_Progress( pActive_Overworld->m_waypoints.size(), 1 );
	}
	else if( key == SDLK_F3 && pOverworld_Manager->debugmode )
	{
		pActive_Overworld->Goto_Next_Level();
	}
	// Exit
	else if( key == SDLK_ESCAPE || key == SDLK_BACKSPACE )
	{
		pOverworld_Player->Action_Interact( INP_EXIT );
	}
	// Action
	else if( key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE )
	{
		pOverworld_Player->Action_Interact( INP_ACTION );
	}
	// ## editor
	else if( pWorld_Editor->Key_Down( key ) )
	{
		// processed by the editor
		return 1;
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cOverworld :: Key_Up( SDLKey key )
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

bool cOverworld :: Mouse_Down( Uint8 button )
{
	// ## editor
	if( pWorld_Editor->Mouse_Down( button ) )
	{
		// processed by the editor
		return 1;
	}
	else
	{
		// not processed
		return 0;
	}

	// button got processed
	return 1;
}

bool cOverworld :: Mouse_Up( Uint8 button )
{
	// ## editor
	if( pWorld_Editor->Mouse_Up( button ) )
	{
		// processed by the editor
		return 1;
	}
	else
	{
		// not processed
		return 0;
	}

	// button got processed
	return 1;
}

bool cOverworld :: Joy_Button_Down( Uint8 button )
{
	// Exit
	if( button == pPreferences->m_joy_button_exit )
	{
		pOverworld_Player->Action_Interact( INP_EXIT );
	}
	// Action
	else if( button == pPreferences->m_joy_button_action )
	{
		pOverworld_Player->Action_Interact( INP_ACTION );
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cOverworld :: Joy_Button_Up( Uint8 button )
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

void cOverworld :: Set_Progress( unsigned int normal_level, bool force /* = 1 */ )
{
	unsigned int level_num = 0;

	for( WaypointList::iterator itr = m_waypoints.begin(), itr_end = m_waypoints.end(); itr != itr_end; ++itr )
	{
		cWaypoint *obj = (*itr);

		// accessible
		if( normal_level >= level_num )
		{
			obj->Set_Access( 1 );
		}
		// force unset
		else if( force )
		{
			obj->Set_Access( 0 );
		}

		level_num++;
	}
}

cWaypoint *cOverworld :: Get_Waypoint( const std::string &name )
{
	for( WaypointList::iterator itr = m_waypoints.begin(), itr_end = m_waypoints.end(); itr != itr_end; ++itr )
	{
		cWaypoint *obj = (*itr);

		// accessible
		if( obj->destination.compare( name ) == 0 )
		{
			return obj;
		}
	}

	return NULL;
}

cWaypoint *cOverworld :: Get_Waypoint( unsigned int num )
{
	if( num >= m_waypoints.size() )
	{
		// out of bounds
		return NULL;
	}

	// available
	return m_waypoints[num];
}

int cOverworld :: Get_Level_Waypoint_Num( std::string level_name )
{
	// erase file type if set
	if( level_name.rfind( ".txt" ) != std::string::npos || level_name.rfind( ".smclvl" ) != std::string::npos )
	{
		level_name.erase( level_name.rfind( "." ) );
	}

	return Get_Waypoint_Num( level_name );
}

int cOverworld :: Get_Waypoint_Num( const std::string &name )
{
	int count = 0;

	// search waypoint
	for( WaypointList::iterator itr = m_waypoints.begin(), itr_end = m_waypoints.end(); itr != itr_end; ++itr )
	{
		cWaypoint *obj = (*itr);

		if( obj->destination.compare( name ) == 0 )
		{
			// found
			return count;
		}

		count++;
	}

	// not found
	return -1;
}

int cOverworld :: Get_Waypoint_Collision( const GL_rect &rect_2 )
{
	int count = 0;

	for( WaypointList::iterator itr = m_waypoints.begin(), itr_end = m_waypoints.end(); itr != itr_end; ++itr )
	{
		cWaypoint *obj = (*itr);

		if( rect_2.Intersects( obj->m_rect ) )
		{
			return count;
		}

		count++;
	}

	return -1;
}

int cOverworld :: Get_Last_Valid_Waypoint( void )
{
	// no waypoints
	if( m_waypoints.empty() )
	{
		return -1;
	}

	for( int i = m_waypoints.size() - 1; i > 0; i-- )
	{
		if( m_waypoints[i]->access )
		{
			return i;
		}
	}

	return -1;
}

void cOverworld :: Update_Waypoint_text( void )
{
	// get waypoint
	cWaypoint *waypoint = m_waypoints[pOverworld_Player->current_waypoint];

	// set color
	Color color = static_cast<Uint8>(0);

	if( waypoint->waypoint_type == WAYPOINT_NORMAL )
	{
		color = lightblue;
	}
	else if( waypoint->waypoint_type == WAYPOINT_WORLD_LINK )
	{
		color = green;
	}
	
	m_hud_level_name->Set_Image( pFont->Render_Text( pFont->m_font_normal, waypoint->Get_Destination(), color ), 1, 1 );
}

bool cOverworld :: Goto_Next_Level( void )
{
	// if not in overworld only goto next level on overworld enter
	if( Game_Mode != MODE_OVERWORLD )
	{
		m_next_level = 1;
		return 0;
	}

	m_next_level = 0;

	cWaypoint *current_waypoint = pOverworld_Player->Get_Waypoint();

	// no Waypoint
	if( !current_waypoint )
	{
		return 0;
	}

	// Waypoint forward direction is invalid/unset
	if( current_waypoint->direction_forward == DIR_UNDEFINED )
	{
		return 0;
	}

	// Get Layer Line in front
	cLayer_Line_Point_Start *front_line = pOverworld_Player->Get_Front_Line( current_waypoint->direction_forward );

	if( !front_line )
	{
		return 0;
	}

	// Get forward Waypoint
	cWaypoint *next_waypoint = front_line->Get_End_Waypoint();

	// if no next waypoint available
	if( !next_waypoint )
	{
		return 0;
	}

	// if next waypoint is new
	if( !next_waypoint->access )
	{
		next_waypoint->Set_Access( 1 );

		// animation
		cParticle_Emitter *anim = new cParticle_Emitter();
		anim->Set_Pos( next_waypoint->m_rect.m_x + ( next_waypoint->m_rect.m_w * 0.5f ), next_waypoint->m_rect.m_y + ( next_waypoint->m_rect.m_h * 0.5f ) );
		anim->Set_Emitter_Time_to_Live( 1.5f );
		anim->Set_Emitter_Iteration_Interval( 0.05f );
		anim->Set_Quota( 1 );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
		anim->Set_Pos_Z( 0.081f );
		anim->Set_Time_to_Live( 1.3f );
		anim->Set_Speed( 1, 1 );
		anim->Set_Scale( 0.5f, 0.4f );
		anim->Set_Const_Rotation_Z( -6, 12 );

		// World Waypoint
		if( next_waypoint->waypoint_type == WAYPOINT_WORLD_LINK )
		{
			anim->Set_Color( lightgreen, Color( static_cast<Uint8>(60), 0, 60, 0 ) );
		}
		else
		{
			anim->Set_Color( orange, Color( static_cast<Uint8>(6), 60, 20, 0 ) );
		}

		// add animation
		pAnimation_Manager->Add( anim );
	}

	pOverworld_Player->Start_Walk( current_waypoint->direction_forward );

	return  1;
}

void cOverworld :: Reset_Waypoints( void )
{
	for( WaypointList::iterator itr = m_waypoints.begin(), itr_end = m_waypoints.end(); itr != itr_end; ++itr )
	{
		cWaypoint *obj = (*itr);

		obj->Set_Access( obj->access_default );
	}
}

bool cOverworld :: Is_Loaded( void ) const
{
	// if not loaded version is -1
	if( m_engine_version >= 0 )
	{
		return 1;
	}

	return 0;
}

// XML element start
void cOverworld :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	// Property of an Element
	if( element == "Property" )
	{
		m_xml_attributes.add( attributes.getValueAsString( "name" ), attributes.getValueAsString( "value" ) );
	}
}

// XML element end
void cOverworld :: elementEnd( const CEGUI::String &element )
{
	if( element != "Property" )
	{
		if( element == "information" )
		{
			m_engine_version = m_xml_attributes.getValueAsInteger( "engine_version" );
			m_last_saved = m_xml_attributes.getValueAsInteger( "save_time" );
		}
		else if( element == "settings" )
		{
			// Author
			//author = m_xml_attributes.getValueAsString( "author" ).c_str();
			// Version
			//version = m_xml_attributes.getValueAsString( "version" ).c_str();
			// Music
			m_musicfile = m_xml_attributes.getValueAsString( "music" ).c_str();
			// Camera Limits
			//pOverworld_Manager->camera->Set_Limits( GL_rect( static_cast<float>(m_xml_attributes.getValueAsInteger( "cam_limit_x" )), static_cast<float>(m_xml_attributes.getValueAsInteger( "cam_limit_y" )), static_cast<float>(m_xml_attributes.getValueAsInteger( "cam_limit_w" )), static_cast<float>(m_xml_attributes.getValueAsInteger( "cam_limit_h" )) ) );
		}
		else if( element == "player" )
		{
			// Start Waypoint
			m_player_start_waypoint = m_xml_attributes.getValueAsInteger( "waypoint" );
			// Moving State
			m_player_moving_state = static_cast<Moving_state>(m_xml_attributes.getValueAsInteger( "moving_state" ));
		}
		else if( element == "background" )
		{
			m_background_color = Color( static_cast<Uint8>(m_xml_attributes.getValueAsInteger( "color_red" )), m_xml_attributes.getValueAsInteger( "color_green" ), m_xml_attributes.getValueAsInteger( "color_blue" ) );
		}
		else
		{
			// get World object
			cSprite *object = Get_World_Object( element, m_xml_attributes );
			
			// valid
			if( object )
			{
				m_sprite_manager->Add( object );
			}
			else if( element == "overworld" )
			{
				// ignore
			}
			else if( element.length() )
			{
				printf( "Warning : Overworld Unknown element : %s\n", element.c_str() );
			}
		}

		// clear
		m_xml_attributes = CEGUI::XMLAttributes();
	}
}

cSprite *Get_World_Object( const CEGUI::String &element, CEGUI::XMLAttributes &attributes )
{
	if( element == "sprite" )
	{
		// old position name
		if( attributes.exists( "filename" ) )
		{
			attributes.add( "image", attributes.getValueAsString( "filename" ) );
			attributes.add( "posx", attributes.getValueAsString( "pos_x" ) );
			attributes.add( "posy", attributes.getValueAsString( "pos_y" ) );
		}

		// create sprite
		cSprite *object = new cSprite( attributes );
		// set sprite type
		object->Set_Sprite_Type( TYPE_PASSIVE );

		return object;
	}
	else if( element == "waypoint" )
	{
		return new cWaypoint( attributes );
	}
	else if( element == "sound" )
	{
		return new cRandom_Sound( attributes );
	}
	else if( element == "line" )
	{
		return new cLayer_Line_Point_Start( attributes );
	}

	return NULL;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cOverworld *pActive_Overworld = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
