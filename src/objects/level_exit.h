/***************************************************************************
 * level_exit.h  -  header for the corresponding cpp file
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

#ifndef SMC_LEVEL_EXIT_H
#define SMC_LEVEL_EXIT_H

#include "../core/globals.h"
#include "../objects/animated_sprite.h"

namespace SMC
{

/* *** *** *** *** *** *** *** Level Exit types *** *** *** *** *** *** *** *** *** *** */

enum Level_Exit_type
{
	LEVEL_EXIT_BEAM = 0,	// no animation ( f.e. a door or hole )
	LEVEL_EXIT_WARP = 1		// rotated player moves slowly into the destination direction
};


/* *** *** *** *** *** *** *** Level Exit motions *** *** *** *** *** *** *** *** *** *** */

// Only used when destination level is the same level
enum Level_Exit_motion
{
	LEVEL_EXIT_M_FLY = 0,	// Camera flys to entry point
	LEVEL_EXIT_M_BLINK = 1,	// Camera fades out, then in at the entry
	LEVEL_EXIT_M_PATH = 2,	// Camera moves along a defined path
	LEVEL_EXIT_M_PATH_BACKWARDS = 3	// Camera moves along a defined path backwards
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Level exit
 * or if a destination or entry is given it gets you there
*/
class cLevel_Exit : public cAnimated_Sprite
{
public:
	// constructor
	cLevel_Exit( float x, float y );
	// create from stream
	cLevel_Exit( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cLevel_Exit( void );

	// init defaults
	void Init( void );
	// copy this sprite
	virtual cLevel_Exit *Copy( void );

	// create from stream
	virtual void Create_From_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_Stream( ofstream &file );
	// Set direction
	void Set_Direction( const ObjectDirection dir );

	// draw
	virtual void Draw( cSurface_Request *request = NULL );

	// Activate
	void Activate( void );

	// Set the type
	void Set_Type( Level_Exit_type ltype );
	// Set the motion
	void Set_Motion( Level_Exit_motion motion );

	// Set the destination level
	void Set_Level( std::string filename );
	// Return the destination level
	std::string Get_Level( bool with_dir = 1, bool with_end = 1 ) const;

	// Set the destination entry
	void Set_Entry( const std::string &entry_name );
	/* Set the path identifier
	 * only used if motion type is path
	*/
	void Set_Path_Identifier( const std::string &identifier );

	// if draw is valid for the current state and position
	virtual bool Is_Draw_Valid( void );

	// editor activation
	virtual void Editor_Activate( void );
	// editor state update
	virtual void Editor_State_Update( void );
	// editor direction option selected event
	bool Editor_Direction_Select( const CEGUI::EventArgs &event );
	// editor motion option selected event
	bool Editor_Motion_Select( const CEGUI::EventArgs &event);
	// editor destination level text changed event
	bool Editor_Destination_Level_Text_Changed( const CEGUI::EventArgs &event );
	// editor destination entry text changed event
	bool Editor_Destination_Entry_Text_Changed( const CEGUI::EventArgs &event );
	// editor path identifier text changed event
	bool Editor_Path_Identifier_Text_Changed( const CEGUI::EventArgs &event );

	// level exit type
	Level_Exit_type exit_type;
	// motion type
	Level_Exit_motion exit_motion;
	// destination level
	std::string dest_level;
	// destination entry ( only used if in same level )
	std::string dest_entry;
	// string identifier of the linked path
	std::string m_path_identifier;

	// editor type color
	Color editor_color;
	// editor entry name text
	cGL_Surface *editor_entry_name;

private:
	void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
