/***************************************************************************
 * enemystopper.h  -  header for the corresponding cpp file
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

#ifndef SMC_ENEMYSTOPPER_H
#define SMC_ENEMYSTOPPER_H

#include "../core/globals.h"
#include "../objects/animated_sprite.h"

namespace SMC
{

/* *** *** *** *** *** cEnemyStopper *** *** *** *** *** *** *** *** *** *** *** *** */

class cEnemyStopper : public cAnimated_Sprite
{
public:
	// constructor
	cEnemyStopper( float x, float y );
	// create from stream
	cEnemyStopper( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cEnemyStopper( void );
	
	// init defaults
	void Init( void );
	// copy
	virtual cEnemyStopper *Copy( void );

	// create from stream
	virtual void Create_From_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_Stream( ofstream &file );

	// draw
	virtual void Draw( cSurface_Request *request = NULL );

	// if draw is valid for the current state and position
	virtual bool Is_Draw_Valid( void );

	// editor color
	Color editor_color;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
