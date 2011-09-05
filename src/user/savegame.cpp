/***************************************************************************
 * savegame.cpp  -  Savegame handler
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

#include "../user/savegame.h"
#include "../user/preferences.h"
#include "../core/game_core.h"
#include "../core/obj_manager.h"
#include "../level/level.h"
#include "../overworld/world_manager.h"
#include "../player/player.h"
#include "../overworld/overworld.h"
#include "../core/i18n.h"
#include "../core/filesystem/filesystem.h"
#include "../core/filesystem/resource_manager.h"
// CEGUI
#include "CEGUIXMLParser.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cSave_Overworld_Waypoint *** *** *** *** *** *** *** *** *** *** */

cSave_Overworld_Waypoint :: cSave_Overworld_Waypoint( void )
{
	access = 0;
}

cSave_Overworld_Waypoint :: ~cSave_Overworld_Waypoint( void )
{
	
}

/* *** *** *** *** *** *** *** cSave_Overworld *** *** *** *** *** *** *** *** *** *** */

cSave_Overworld :: cSave_Overworld( void )
{
	
}

cSave_Overworld :: ~cSave_Overworld( void )
{
	for( unsigned int i = 0; i < m_waypoints.size(); i++ )
	{
		delete m_waypoints[i];
	}

	m_waypoints.clear();
}

/* *** *** *** *** *** *** *** cSave_Level_Object *** *** *** *** *** *** *** *** *** *** */

cSave_Level_Object_Property :: cSave_Level_Object_Property( const std::string &n_name /* = "" */, const std::string &n_value /* = "" */ )
{
	m_name = n_name;
	m_value = n_value;
}

/* *** *** *** *** *** *** *** cSave_Level_Object *** *** *** *** *** *** *** *** *** *** */

cSave_Level_Object :: cSave_Level_Object( void )
{
	m_type = TYPE_UNDEFINED;
}

cSave_Level_Object :: ~cSave_Level_Object( void )
{
	m_properties.clear();
}

bool cSave_Level_Object :: exists( const std::string &val_name )
{
	for( Save_Level_Object_ProprtyList::iterator itr = m_properties.begin(), itr_end = m_properties.end(); itr != itr_end; ++itr )
	{
		cSave_Level_Object_Property obj = (*itr);

		if( obj.m_name.compare( val_name ) == 0 )
		{
			// found
			return 1;
		}
	}

	// not found
	return 0;
}

std::string cSave_Level_Object :: Get_Value( const std::string &val_name )
{
	for( Save_Level_Object_ProprtyList::iterator itr = m_properties.begin(), itr_end = m_properties.end(); itr != itr_end; ++itr )
	{
		cSave_Level_Object_Property obj = (*itr);

		if( obj.m_name.compare( val_name ) == 0 )
		{
			// found
			return obj.m_value;
		}
	}

	// not found
	return "";
}

/* *** *** *** *** *** *** *** cSave *** *** *** *** *** *** *** *** *** *** */

cSave :: cSave( void )
{
	Init();
}

cSave :: ~cSave( void )
{
	// clear
	for( Save_Level_ObjectList::iterator itr = level_objects.begin(), itr_end = level_objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	level_objects.clear();

	for( Save_OverworldList::iterator itr = m_overworlds.begin(), itr_end = m_overworlds.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	m_overworlds.clear();
}

void cSave :: Init( void )
{
	// save
	save_time = 0;
	version = 0;

	// player
	lives = 0;
	points = 0;
	goldpieces = 0;
	player_type = 0;
	player_state = 0;
	itembox_item = 0;
	
	// level
	level_posx = 0;
	level_posy = 0;

	// overworld
	overworld_current_waypoint = 0;
}

cSavegame :: cSavegame( void )
{
	error_count = 0;
	save_temp = NULL;
	debug = 0;

	savegame_dir = pResource_Manager->user_data_dir + USER_SAVEGAME_DIR;
}

cSavegame :: ~cSavegame( void )
{
	//
}

int cSavegame :: Load_Game( unsigned int save_slot )
{
	cSave *savegame = Load( save_slot );
	
	// check if unsupported save
	if( savegame->version <= SAVEGAME_VERSION_UNSUPPORTED )
	{
		printf( "Warning : Savegame %d : Versions %d and below are unsupported\n", save_slot, SAVEGAME_VERSION_UNSUPPORTED );
	}

	// level available
	if( savegame->level_name.length() )
	{
		std::string level_name = savegame->level_name;

		// level not found
		if( !pActive_Level->Get_Path( level_name ) )
		{
			printf( "Warning : Savegame %d : Level not found : %s\n", save_slot, level_name.c_str() );
		}
	}

	// unload level
	pActive_Level->Unload();
	// reset player
	pPlayer->Reset_Save();
	// reset custom level mode type
	if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM )
	{
		Game_Mode_Type = MODE_TYPE_DEFAULT;
	}

	// #### Overworld ####

	// set overworld progress
	if( !savegame->m_overworlds.empty() )
	{
		for( Save_OverworldList::iterator itr = savegame->m_overworlds.begin(), itr_end = savegame->m_overworlds.end(); itr != itr_end; ++itr )
		{
			// get savegame overworld pointer
			cSave_Overworld *save_overworld = (*itr);

			// get overworld
			cOverworld *overworld = pOverworld_Manager->Get_from_Name( save_overworld->name );

			if( !overworld )
			{
				printf( "Warning : Savegame %d : Overworld %s not found\n", save_slot, save_overworld->name.c_str() );
				continue;
			}

			for( Save_Overworld_WaypointList::iterator wp_itr = save_overworld->m_waypoints.begin(), wp_itr_end = save_overworld->m_waypoints.end(); wp_itr != wp_itr_end; ++wp_itr )
			{
				// get savegame waypoint pointer
				cSave_Overworld_Waypoint *save_waypoint = (*wp_itr);

				// get overworld waypoint
				cWaypoint *waypoint = overworld->Get_Waypoint( overworld->Get_Waypoint_Num( save_waypoint->destination ) );

				// not found
				if( !waypoint )
				{
					printf( "Warning : Savegame %d : Overworld %s Waypoint %s not found\n", save_slot, save_overworld->name.c_str(), save_waypoint->destination.c_str() );
					continue;
				}

				// set access
				waypoint->Set_Access( save_waypoint->access );
			}
		}
	}

	// if an overworld is active
	if( !savegame->overworld_active.empty() )
	{
		// Set Active Overworld
		if( !pOverworld_Manager->Set_Active( savegame->overworld_active ) )
		{
			printf( "Warning : Savegame %d : Couldn't set Overworld active %s\n", save_slot, savegame->overworld_active.c_str() );
		}

		// Current Waypoint
		if( !pOverworld_Player->Set_Waypoint( savegame->overworld_current_waypoint ) )
		{
			printf( "Warning : Savegame %d : Overworld Current Waypoint %d is invalid\n", save_slot, savegame->overworld_current_waypoint );
		}
	}
	// overworld is not active
	else
	{
		// Set custom level mode
		Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM;
	}

	// #### Player ####

	// below version 8 the sate was the type
	if( savegame->version < 8 )
	{
		// type
		pPlayer->Set_Type( static_cast<Maryo_type>(savegame->player_state), 0, 0 );
	}
	else
	{
		// type
		pPlayer->Set_Type( static_cast<Maryo_type>(savegame->player_type), 0, 0 );
		// state
		pPlayer->m_state = static_cast<Moving_state>(savegame->player_state);
	}


	// in a level
	if( !savegame->level_name.empty() )
	{
		// load level
		if( pActive_Level->Load( savegame->level_name ) )
		{
			pActive_Level->Enter();

			// position
			pPlayer->Set_Pos( savegame->level_posx, savegame->level_posy - static_cast<float>(game_res_h) );

			// Level Objects
			for( Save_Level_ObjectList::iterator itr = savegame->level_objects.begin(), itr_end = savegame->level_objects.end(); itr != itr_end; ++itr )
			{
				// get object pointer
				cSave_Level_Object *save_object = (*itr);

				// get position
				int posx = string_to_int( save_object->Get_Value( "posx" ) );
				int posy = string_to_int( save_object->Get_Value( "posy" ) );

				// get level object
				cSprite *level_object = pActive_Level->m_sprite_manager->Get_from_Position( posx, posy, save_object->m_type );

				// if not anymore available
				if( !level_object )
				{
					printf( "Warning : Savegame object type %d on x %d, y %d not available\n", save_object->m_type, posx, posy );
					continue;
				}

				level_object->Load_From_Savegame( save_object );
			}

			// invincible for a second
			pPlayer->invincible = speedfactor_fps;
		}
		else
		{
			printf( "Error : Couldn't load Savegame Level %s\n", savegame->level_name.c_str() );
		}
	}
	
	pHud_Points->Set_Points( savegame->points );
	pHud_Goldpieces->Set_Gold( savegame->goldpieces );
	pHud_Lives->Set_Lives( savegame->lives );
	pHud_Itembox->Set_Item( static_cast<SpriteType>(savegame->itembox_item), 0 );

	pActive_Camera->Center();

	pHud_Manager->Update();

	pHud_Debug->Set_Text( _("Savegame ") + int_to_string( save_slot ) + _(" loaded") );

	// default is level save
	int retval = 1;

	// if Overworld Save
	if( savegame->level_name.empty() )
	{
		retval = 2;
	}

	delete savegame;
	return retval;
}

bool cSavegame :: Save_Game( unsigned int save_slot, std::string description )
{
	if( pPlayer->maryo_type == MARYO_DEAD || pPlayer->lives < 0 )
	{
		printf( "Error : Couldn't save savegame %s because of invalid game state\n", description.c_str() );
		return 0;
	}

	cSave *savegame = new cSave();

	// Description
	savegame->description = description;
	// Goldpieces
	savegame->goldpieces = pPlayer->goldpieces;

	// Level
	if( pActive_Level->Is_Loaded() )
	{
		// name
		savegame->level_name = Trim_Filename( pActive_Level->data_file, 0, 0 );

		// position
		savegame->level_posx = pPlayer->m_pos_x;
		savegame->level_posy = pPlayer->m_pos_y + game_res_h - 5;

		// Level Objects
		for( cSprite_List::iterator itr = pActive_Level->m_sprite_manager->objects.begin(), itr_end = pActive_Level->m_sprite_manager->objects.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *object = (*itr);

			// get save data
			cSave_Level_Object *save_object = object->Save_To_Savegame();

			// nothing to save
			if( !save_object )
			{
				continue;
			}

			// add
			savegame->level_objects.push_back( save_object );
		}
	}

	// Lives
	savegame->lives = pPlayer->lives;
	// Points
	savegame->points = pPlayer->points;

	// Player type
	savegame->player_type = pPlayer->maryo_type;
	// Player state
	savegame->player_state = pPlayer->m_state;
	// Itembox Item
	savegame->itembox_item = pHud_Itembox->m_item_id;

	// save overworld progress
	for( vector<cOverworld *>::iterator itr = pOverworld_Manager->objects.begin(), itr_end = pOverworld_Manager->objects.end(); itr != itr_end; ++itr )
	{
		// Get Overworld
		cOverworld *overworld = (*itr);

		// create Overworld
		cSave_Overworld *save_overworld = new cSave_Overworld();
		// name
		save_overworld->name = overworld->m_description->name;
		
		// Waypoints
		for( cSprite_List::iterator wp_itr = overworld->m_sprite_manager->objects.begin(), wp_itr_end = overworld->m_sprite_manager->objects.end(); wp_itr != wp_itr_end; ++wp_itr )
		{
			// get waypoint
			cSprite *obj = static_cast<cSprite *>(*wp_itr);

			if( obj->m_type != TYPE_OW_WAYPOINT )
			{
				continue;
			}

			// get waypoint
			cWaypoint *waypoint = static_cast<cWaypoint *>(obj);

			// create savegame waypoint
			cSave_Overworld_Waypoint *save_waypoint = new cSave_Overworld_Waypoint();
			
			// destination
			save_waypoint->destination = waypoint->Get_Destination();
			// set access
			save_waypoint->access = waypoint->access;
			// save
			save_overworld->m_waypoints.push_back( save_waypoint );
		}

		// save
		savegame->m_overworlds.push_back( save_overworld );
	}

	// if an overworld is active and not custom level mode
	if( pActive_Overworld && Game_Mode_Type != MODE_TYPE_LEVEL_CUSTOM )
	{
		// set overworld name
		savegame->overworld_active = pActive_Overworld->m_description->name;

		// if valid waypoint
		if( pOverworld_Player->current_waypoint >= 0 )
		{
			// set current waypoint
			savegame->overworld_current_waypoint = pOverworld_Player->current_waypoint;
		}
	}

	// Time ( seconds since 1970 )
	savegame->save_time = time( NULL );
	// Version
	savegame->version = SAVEGAME_VERSION;

	// Save it
	Save( save_slot, savegame );

	// Print
	if( pHud_Debug )
	{
		pHud_Debug->Set_Text( _("Saved to Slot ") + int_to_string( save_slot ) );
	}

	// Clear
	delete savegame;

	return 1;
}

cSave *cSavegame :: Load( unsigned int save_slot )
{
	save_temp = new cSave();

	std::string filename = savegame_dir + "/" + int_to_string( save_slot ) + ".save";

	if( !File_Exists( filename ) )
	{
		printf( "Error : Savegame Loading : No Savegame found at Slot : %s\n", filename.c_str() );
		cSave *savegame = save_temp;
		save_temp = NULL;
		return savegame;
	}

	try
	{
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/Savegame.xsd", "" );
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "Loading Savegame %s CEGUI Exception %s\n", filename.c_str(), ex.getMessage().c_str() );
		pHud_Debug->Set_Text( _("Savegame Loading failed : ") + (const std::string)ex.getMessage().c_str() );
	}

	// unknown savegame data found
	if( error_count )
	{
		printf( "Warning : Savegame Loading : Errors found at Slot : %d\n", save_slot );
		save_temp->description.insert( 0, _("Unsupported : ") );
	}

	// if no description is available
	if( !save_temp->description.length() )
	{
		save_temp->description = _("No Description");
	}

	cSave *savegame = save_temp;
	save_temp = NULL;
	return savegame;
}

int cSavegame :: Save( unsigned int save_slot, cSave *savegame )
{
	std::string filename = savegame_dir + "/" + int_to_string( save_slot ) + ".save";

	if( File_Exists( filename ) )
	{
		ifstream ifs( filename.c_str(), ios::trunc ); // Delete existing
		ifs.close();
	}

	// empty overworld active
	if( savegame->overworld_active.empty() )
	{
		printf( "Warning : Savegame %s saving : Empty Overworld Active\n", savegame->description.c_str() );
	}

	ofstream file( filename.c_str(), ios::out );
	
	if( !file.is_open() )
	{
		printf( "Error : Couldn't open savegame file for saving. Is the file read-only ?" );
		return 0;
	}

	// xml info
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	// begin Savegame
	file << "<Savegame>" << std::endl;

	// begin Information
	file << "\t<Information>" << std::endl;
		// Description
		file << "\t\t<Property Name=\"description\" Value=\"" << savegame->description << "\" />" << std::endl;
		// Version
		file << "\t\t<Property Name=\"version\" Value=\"" << savegame->version << "\" />" << std::endl;
		// Time
		file << "\t\t<Property Name=\"save_time\" Value=\"" << savegame->save_time << "\" />" << std::endl;
	// end Information
	file << "\t</Information>" << std::endl;

	if( !savegame->level_name.empty() )
	{
		// begin Level
		file << "\t<Level>" << std::endl;

		// Level name
		file << "\t\t<Property Name=\"level_name\" Value=\"" << savegame->level_name << "\" />" << std::endl;
		// Level position
		file << "\t\t<Property Name=\"player_posx\" Value=\"" << savegame->level_posx << "\" />" << std::endl;
		file << "\t\t<Property Name=\"player_posy\" Value=\"" << savegame->level_posy << "\" />" << std::endl;

		// Level Objects
		for( Save_Level_ObjectList::iterator itr = savegame->level_objects.begin(), itr_end = savegame->level_objects.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSave_Level_Object *object = (*itr);

			// begin Level Object
			file << "\t\t<Level_Object>" << std::endl;

			// Object type
			file << "\t\t\t<Property Name=\"type\" Value=\"" << object->m_type << "\" />" << std::endl;

			// Properties
			for( Save_Level_Object_ProprtyList::iterator prop_itr = object->m_properties.begin(), prop_itr_end = object->m_properties.end(); prop_itr != prop_itr_end; ++prop_itr )
			{
				// get properties pointer
				cSave_Level_Object_Property Property = (*prop_itr);

				// property
				file << "\t\t\t<Property Name=\"" << Property.m_name <<"\" Value=\"" << Property.m_value << "\" />" << std::endl;
			}

			// end Level Object
			file << "\t\t</Level_Object>" << std::endl;
		}
		// end Level
		file << "\t</Level>" << std::endl;
	}

	// begin Player
	file << "\t<Player>" << std::endl;
		// Lives
		file << "\t\t<Property Name=\"lives\" Value=\"" << savegame->lives << "\" />" << std::endl;
		// Points
		file << "\t\t<Property Name=\"points\" Value=\"" << savegame->points << "\" />" << std::endl;
		// Goldpieces
		file << "\t\t<Property Name=\"goldpieces\" Value=\"" << savegame->goldpieces << "\" />" << std::endl;
		// type
		file << "\t\t<Property Name=\"type\" Value=\"" << savegame->player_type << "\" />" << std::endl;
		// state
		file << "\t\t<Property Name=\"state\" Value=\"" << savegame->player_state << "\" />" << std::endl;
		// Itembox item
		file << "\t\t<Property Name=\"itembox_item\" Value=\"" << savegame->itembox_item << "\" />" << std::endl;
	// end Player
	file << "\t</Player>" << std::endl;

	// begin Overworld_Data
	file << "\t<Overworld_Data>" << std::endl;
		// active Overworld
		file << "\t\t<Property Name=\"active\" Value=\"" << savegame->overworld_active << "\" />" << std::endl;
		// current Overworld Waypoint
		file << "\t\t<Property Name=\"current_waypoint\" Value=\"" << savegame->overworld_current_waypoint << "\" />" << std::endl;
	// end Overworld_Data
	file << "\t</Overworld_Data>" << std::endl;

	// Overworlds
	for( Save_OverworldList::iterator itr = savegame->m_overworlds.begin(), itr_end = savegame->m_overworlds.end(); itr != itr_end; ++itr )
	{
		// get object pointer
		cSave_Overworld *overworld = (*itr);

		// begin Overworld
		file << "\t<Overworld>" << std::endl;

		// current Overworld
		file << "\t\t<Property Name=\"name\" Value=\"" << overworld->name << "\" />" << std::endl;

		for( Save_Overworld_WaypointList::iterator wp_itr = overworld->m_waypoints.begin(), wp_itr_end = overworld->m_waypoints.end(); wp_itr != wp_itr_end; ++wp_itr )
		{
			// get object pointer
			cSave_Overworld_Waypoint *overworld_waypoint = (*wp_itr);

			// skip empty waypoints
			if( overworld_waypoint->destination.empty() )
			{
				continue;
			}

			// begin Overworld Level
			file << "\t\t<Overworld_Level>" << std::endl;

			// destination
			file << "\t\t\t<Property Name=\"destination\" Value=\"" << overworld_waypoint->destination << "\" />" << std::endl;
			// access
			file << "\t\t\t<Property Name=\"access\" Value=\"" << overworld_waypoint->access << "\" />" << std::endl;

			// end Overworld Level
			file << "\t\t</Overworld_Level>" << std::endl;
		}

		// end Overworld
		file << "\t</Overworld>" << std::endl;
	}

	// end Savegame
	file << "</Savegame>" << std::endl;

	file.close();
	
	if( debug )
	{
		printf( "Saved Savegame %s to slot %d\n", filename.c_str(), save_slot );
	}
	
	return 1;
}

std::string cSavegame :: Get_Description( unsigned int save_slot, bool only_description /* = 0 */ )
{
	std::string savefile, str_description;

	savefile = savegame_dir + "/" + int_to_string( save_slot ) + ".save";

	if( !File_Exists( savefile ) )
	{
		str_description = int_to_string( save_slot ) + ". Free Save";
		return str_description;
	}
	
	cSave *temp_savegame = Load( save_slot );

	// complete description
	if( !only_description )
	{
		str_description = int_to_string( save_slot ) + ". " + temp_savegame->description;

		if( temp_savegame->level_name.empty() )
		{
			str_description += " - " + temp_savegame->overworld_active;
		}
		else
		{
			str_description += _(" -  Level ") + temp_savegame->level_name;
		}

		str_description += _(" - Date ") + Time_to_String( temp_savegame->save_time, "%Y-%m-%d  %H:%M:%S" );
	}
	// only the user description
	else
	{
		return temp_savegame->description;
	}

	delete temp_savegame;
	return str_description;
}

bool cSavegame :: Is_Valid( unsigned int save_slot ) const
{
	return File_Exists( savegame_dir + "/" + int_to_string( save_slot ) + ".save" );
}

// XML element start
void cSavegame :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	// Property/Item/Tag of an Element
	if( element == "Property" )
	{
		xml_attributes.add( attributes.getValueAsString( "Name" ), attributes.getValueAsString( "Value" ) );
	}
}

// XML element end
void cSavegame :: elementEnd( const CEGUI::String &element )
{
	if( element != "Property" )
	{
		if( element == "Information" )
		{
			save_temp->description = xml_attributes.getValueAsString( "description" ).c_str();
			save_temp->version = xml_attributes.getValueAsInteger( "version" );
			save_temp->save_time = xml_attributes.getValueAsInteger( "save_time" );
		}
		else if( element == "Level" )
		{
			Handle_Level( xml_attributes );
		}
		else if( element == "Level_Object" )
		{
			Handle_Level_Object( xml_attributes );
			// don't clear attributes
			return;
		}
		else if( element == "Player" )
		{
			Handle_Player( xml_attributes );
		}
		else if( element == "Overworld_Data" )
		{
			Handle_Overworld_Data( xml_attributes );
		}
		else if( element == "Overworld" )
		{
			Handle_Overworld( xml_attributes );
		}
		else if( element == "Overworld_Level" )
		{
			Handle_Overworld_Level( xml_attributes );
			// don't clear attributes
			return;
		}
		else if( element == "Savegame" )
		{
			// ignore
		}
		else if( element.length() )
		{
			printf( "Warning : Savegame Unknown Element : %s\n", element.c_str() );
		}

		// clear
		xml_attributes = CEGUI::XMLAttributes();
	}
}

void cSavegame :: Handle_Level( const CEGUI::XMLAttributes &attributes )
{
	save_temp->level_name = xml_attributes.getValueAsString( "level_name" ).c_str();
	save_temp->level_posx = xml_attributes.getValueAsFloat( "player_posx" );
	save_temp->level_posy = xml_attributes.getValueAsFloat( "player_posy" );

	// set level objects
	save_temp->level_objects.swap( level_objects );
	level_objects.clear();
}

void cSavegame :: Handle_Level_Object( const CEGUI::XMLAttributes &attributes )
{
	int type = xml_attributes.getValueAsInteger( "type" );

	if( type <= 0 )
	{
		printf( "Warning : Unknown Savegame Level Object type %d\n", type );
		return;
	}

	cSave_Level_Object *object = new cSave_Level_Object();

	// type
	object->m_type = static_cast<SpriteType>(type);
	xml_attributes.remove( "type" );


	// Get Properties
	for( unsigned int i = 0; i < xml_attributes.getCount(); i++ )
	{
		// get property
		std::string property_name = xml_attributes.getName( i ).c_str();

		// ignore level attributes
		if( property_name.compare( "level_name" ) == 0 || property_name.compare( "player_posx" ) == 0 || property_name.compare( "player_posy" ) == 0 )
		{
			continue;
		}

		object->m_properties.push_back( cSave_Level_Object_Property( property_name, xml_attributes.getValue( i ).c_str() ) );
	}

	// remove used Properties
	for( Save_Level_Object_ProprtyList::iterator prop_itr = object->m_properties.begin(), prop_itr_end = object->m_properties.end(); prop_itr != prop_itr_end; ++prop_itr )
	{
		// get property pointer
		cSave_Level_Object_Property Property = (*prop_itr);

		xml_attributes.remove( Property.m_name );
	}

	// add object
	level_objects.push_back( object );
}

void cSavegame :: Handle_Player( const CEGUI::XMLAttributes &attributes )
{
	save_temp->lives = xml_attributes.getValueAsInteger( "lives" );
	save_temp->points = xml_attributes.getValueAsInteger( "points" );
	save_temp->goldpieces = xml_attributes.getValueAsInteger( "goldpieces" );
	save_temp->player_type = xml_attributes.getValueAsInteger( "type" );
	save_temp->player_state = xml_attributes.getValueAsInteger( "state" );
	save_temp->itembox_item = xml_attributes.getValueAsInteger( "itembox_item" );
}

void cSavegame :: Handle_Overworld_Data( const CEGUI::XMLAttributes &attributes )
{
	save_temp->overworld_active = xml_attributes.getValueAsString( "active" ).c_str();
	save_temp->overworld_current_waypoint = xml_attributes.getValueAsInteger( "current_waypoint" );
}

void cSavegame :: Handle_Overworld( const CEGUI::XMLAttributes &attributes )
{
	std::string name = xml_attributes.getValueAsString( "name" ).c_str();

	// Search if Overworld is available
	cOverworld *overworld = pOverworld_Manager->Get_from_Name( name );

	if( !overworld )
	{
		printf( "Warning : Savegame %s Overworld %s not found\n", save_temp->description.c_str(), name.c_str() );
	}

	// Create Savegame Overworld
	cSave_Overworld *save_overworld = new cSave_Overworld();
	// set name
	save_overworld->name = name;
	// set waypoints
	save_overworld->m_waypoints.swap( active_waypoints );
	active_waypoints.clear();
	// save
	save_temp->m_overworlds.push_back( save_overworld );
}

void cSavegame :: Handle_Overworld_Level( const CEGUI::XMLAttributes &attributes )
{
	bool access = xml_attributes.getValueAsBool( "access" );

	cSave_Overworld_Waypoint *waypoint = new cSave_Overworld_Waypoint();

	// destination ( level_name and world_name is pre 0.99.6 )
	if( xml_attributes.exists( "world_name" ) )
	{
		waypoint->destination = xml_attributes.getValueAsString( "world_name" ).c_str();
	}
	else if( xml_attributes.exists( "level_name" ) )
	{
		waypoint->destination = xml_attributes.getValueAsString( "level_name" ).c_str();
	}
	// default
	else
	{
		waypoint->destination = xml_attributes.getValueAsString( "destination" ).c_str();
	}

	waypoint->access = access;

	active_waypoints.push_back( waypoint );

	// clear
	xml_attributes.remove( "level_name" );
	xml_attributes.remove( "world_name" );
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cSavegame *pSavegame = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
