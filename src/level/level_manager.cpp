/***************************************************************************
 * level_manager.cpp  -  class for handling levels
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

#include "../level/level_manager.h"

namespace SMC
{

/* *** *** *** *** *** cLevel_Manager *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel_Manager :: cLevel_Manager( void )
{
	pActive_Level = new cLevel();
	camera = new cCamera();
}

cLevel_Manager :: ~cLevel_Manager( void )
{
	if( pActive_Level )
	{
		delete pActive_Level;
		pActive_Level = NULL;
	}

	delete camera;
}

void cLevel_Manager :: Init( void )
{

}

void cLevel_Manager :: Unload( void )
{
	pActive_Level->Unload();
}

bool cLevel_Manager :: Load( std::string filename, bool delayed /* = 0 */, bool sub_level /* = 0 */ )
{
	return 1;
}


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Level information handler
cLevel_Manager *pLevel_Manager = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
