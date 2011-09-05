/***************************************************************************
 * level_manager.h  -  header for the corresponding cpp file
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

#ifndef SMC_LEVEL_MANAGER_H
#define SMC_LEVEL_MANAGER_H

#include "../core/globals.h"
#include "../core/obj_manager.h"
#include "../core/camera.h"
#include "../level/level.h"

namespace SMC
{

// default files for levels
#define LEVEL_DEFAULT_MUSIC "land/land_5.ogg"
#define LEVEL_DEFAULT_BACKGROUND "game/background/green_junglehills.png"

/* *** *** *** *** *** cLevel_Manager  *** *** *** *** *** *** *** *** *** *** *** *** */

class cLevel_Manager : public cObject_Manager<cLevel>
{
public:
	cLevel_Manager( void );
	virtual ~cLevel_Manager( void );

	// Load level descriptions
	void Init( void );
	// Unload
	void Unload( void );

	/* Load level
	 * delayed : if set the level will be loaded on the next update
	 * sub_level : if set previous level isn't unloaded
	*/
	bool Load( std::string filename, bool delayed = 0, bool sub_level = 0 );

	// level camera
	cCamera *camera;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Level Manager
extern cLevel_Manager *pLevel_Manager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
