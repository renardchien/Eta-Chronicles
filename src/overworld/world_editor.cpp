/***************************************************************************
 * world_editor.cpp  -  class for the World Editor
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

#include "../core/globals.h"
#include "../overworld/world_editor.h"
#include "../core/game_core.h"
#include "../overworld/overworld.h"
#include "../audio/audio.h"
#include "../core/i18n.h"
#include "../core/filesystem/filesystem.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cEditor_World *** *** *** *** *** *** *** *** *** *** */

cEditor_World :: cEditor_World( void )
: cEditor()
{
	menu_filename = DATA_DIR "/" GAME_EDITOR_DIR "/world_menu.xml";
	items_filename = DATA_DIR "/" GAME_EDITOR_DIR "/world_items.xml";

	editor_item_tag = "world";
	camera_speed = 20;
}

cEditor_World :: ~cEditor_World( void )
{
	//
}

void cEditor_World :: Init( void )
{
	// already loaded
	if( editor_window )
	{
		return;
	}

	// nothing

	cEditor::Init();
}

void cEditor_World :: Enable( void )
{
	// already enabled
	if( enabled )
	{
		return;
	}
	
	editor_world_enabled = 1;
	pOverworld_Manager->draw_layer = 1;

	if( Game_Mode == MODE_OVERWORLD )
	{
		editor_enabled = 1;
	}
	
	cEditor::Enable();
}

void cEditor_World :: Disable( bool native_mode /* = 0 */ )
{
	// already disabled
	if( !enabled )
	{
		return;
	}

	pHud_Debug->Set_Text( _("World Editor disabled") );

	editor_world_enabled = 0;
	pOverworld_Manager->draw_layer = 0;
	pOverworld_Manager->cameramode = 0;

	if( Game_Mode == MODE_OVERWORLD )
	{
		native_mode = 1;
		editor_enabled = 0;
	}

	cEditor::Disable( native_mode );
}

bool cEditor_World :: Key_Down( SDLKey key )
{
	if( !enabled )
	{
		return 0;
	}


	// check basic editor events
	if( cEditor::Key_Down( key ) )
	{
		return 1;
	}
	// save level
	else if( key == SDLK_s && input_event.key.keysym.mod & KMOD_CTRL )
	{
		pActive_Overworld->Save();
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

void cEditor_World :: Activate_Menu_Item( cEditor_Menu_Object *entry )
{
	// If Function
	if( entry->bfunction )
	{
		if( entry->tags.compare( "new" ) == 0 )
		{
			Function_New();
		}
		else if( entry->tags.compare( "load" ) == 0 )
		{
			Function_Load();
		}
		else if( entry->tags.compare( "save" ) == 0 )
		{
			Function_Save();
		}
		/*else if( entry->tags.compare( "save_as" ) == 0 )
		{
			Function_Save_as();
		}*/
		else if( entry->tags.compare( "reload" ) == 0 )
		{
			Function_Reload();
		}
		else if( entry->tags.compare( "clear" ) == 0 )
		{
			Function_Clear();
		}
		/*else if( entry->tags.compare( "settings" ) == 0 )
		{
			Function_Settings();
		}*/
		// unknown level function
		else
		{
			cEditor::Activate_Menu_Item( entry );
		}
	}
	// unknown level function
	else
	{
		cEditor::Activate_Menu_Item( entry );
	}
}

bool cEditor_World :: Function_New( void )
{
	std::string world_name = Box_Text_Input( _("Create a new World"), _("Name") );

	// aborted/invalid
	if( world_name.empty() )
	{
		return 0;
	}

	if( pOverworld_Manager->New( world_name ) )
	{
		pHud_Debug->Set_Text( _("Created ") + world_name );
		return 1;
	}
	else
	{
		pHud_Debug->Set_Text( _("World ") + world_name + _(" already exists") );
	}

	return 0;
}

void cEditor_World :: Function_Load( void )
{
	std::string world_name = _("Name");

	// valid world
	while( world_name.length() )
	{
		world_name = Box_Text_Input( world_name, _("Load an Overworld"), world_name.compare( _("Name") ) == 0 ? 1 : 0 );

		// break if empty
		if( world_name.empty() )
		{
			break;
		}

		cOverworld *new_world = pOverworld_Manager->Get( world_name );

		// success
		if( new_world )
		{
			Game_Action = GA_ENTER_WORLD;
			Game_Action_Data.add( "world", world_name.c_str() );

			pHud_Debug->Set_Text( _("Loaded ") + world_name );
			break;
		}
		// failed
		else
		{
			pAudio->Play_Sound( "error.ogg" );
		}
	}
}

void cEditor_World :: Function_Save( bool with_dialog /* = 0 */ )
{
	// if denied
	if( with_dialog && !Box_Question( _("Save ") + Trim_Filename( pActive_Overworld->m_description->name, 0, 0 ) + " ?" ) )
	{
		return;
	}

	pActive_Overworld->Save();
}

void cEditor_World :: Function_Reload( void )
{
	// if denied
	if( !Box_Question( _("Reload World ?") ) )
	{
		return;
	}

	pActive_Overworld->Save();
	pActive_Overworld->Load();
}

void cEditor_World :: Function_Clear( void )
{
	// if denied
	if( !Box_Question( _("Clear World ?") ) )
	{
		return;
	}

	pActive_Sprite_Manager->Delete_All();
	pActive_Overworld->m_waypoints.clear();
	pOverworld_Player->Reset();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cEditor_World *pWorld_Editor = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
