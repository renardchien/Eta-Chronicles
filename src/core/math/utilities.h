/***************************************************************************
 * utilities.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2005 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_UTILITIES_H
#define SMC_UTILITIES_H

#include "../../core/global_basic.h"
#include "../../core/global_game.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** Math utility functions *** *** *** *** *** *** *** *** *** */

// return a random floating point value between the given values
float Get_Random_Float( float min, float max );

// Checks if number is power of 2 and if not returns the next power of two size
unsigned int Get_Power_of_2( unsigned int size );

/* Returns true if the number is valid
 * accept_floating_point: if is set also accept floating point values
*/
bool Is_Valid_Number( std::string num, bool accept_floating_point = 1 );

// Returns true if the floats are equal with the given tolerance
bool Is_Float_Equal( float a, float b, float tolerance = 0.0001f );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
