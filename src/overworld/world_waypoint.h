/***************************************************************************
 * waypoint.h  -  header for the corresponding cpp file
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

#ifndef SMC_WORLD_WAYPOINT_H
#define SMC_WORLD_WAYPOINT_H

#include "../core/globals.h"
#include "../video/video.h"
#include "../objects/movingsprite.h"

namespace SMC
{

/* *** *** *** *** *** Waypoint types *** *** *** *** *** *** *** *** *** *** *** *** */

enum Waypoint_type
{
	WAYPOINT_NORMAL = 1,
	WAYPOINT_WORLD_LINK = 2 // Enters another World
};

/* *** *** *** *** *** *** cWaypoint *** *** *** *** *** *** *** *** *** *** *** */

class cWaypoint : public cSprite
{
public:
	// constructor
	cWaypoint( void );
	// create from stream
	cWaypoint( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cWaypoint( void );
	
	// Init defaults
	void Init( void );

	// copy this object
	virtual cWaypoint *Copy( void );

	// create from stream
	virtual void Create_From_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_Stream( ofstream &file );

	// Update
	virtual void Update( void );
	// Draw
	virtual void Draw( cSurface_Request *request = NULL );

	// Set Access
	void Set_Access( bool enabled, bool new_start_access = 0 );

	// Set the Destination
	void Set_Destination( std::string str );
	// Returns the Destination
	std::string Get_Destination( bool with_dir = 0, bool with_end = 0 ) const;

	// editor activation
	virtual void Editor_Activate( void );
	// editor type option selected event
	bool Editor_Type_Select( const CEGUI::EventArgs &event );
	// editor destination text changed event
	bool Editor_Destination_Text_Changed( const CEGUI::EventArgs &event );
	// editor access option selected event
	bool Editor_Access_Select( const CEGUI::EventArgs &event );
	// editor direction backward option selected event
	bool Editor_Backward_Direction_Select( const CEGUI::EventArgs &event );
	// editor direction forward option selected event
	bool Editor_Forward_Direction_Select( const CEGUI::EventArgs &event );

	/* backward and forward direction
	 */
	ObjectDirection direction_backward, direction_forward;

	/* The Waypoint type
	 * see the definitions
	*/
	Waypoint_type waypoint_type;
	// destination
	std::string destination;

	// if this waypoint is accessible
	bool access;
	// the default access defined in the definition
	bool access_default;

	// color for the glim effect
	float gcolor;
	// glim effect type switch
	bool glim;


	// white arrow
	cGL_Surface *arrow_white_l, *arrow_white_r, *arrow_white_u, *arrow_white_d;
	// blue arrow
	cGL_Surface *arrow_blue_l, *arrow_blue_r, *arrow_blue_u, *arrow_blue_d;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
