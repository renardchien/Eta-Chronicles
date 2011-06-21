/***************************************************************************
 * keyboard.h  -  header for the corresponding cpp file
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

#ifndef SMC_KEYBOARD_H
#define SMC_KEYBOARD_H

#include "../core/globals.h"
// SDL
#include "SDL.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** cKeyboard *** *** *** *** *** *** *** *** *** */

class cKeyboard
{
public:
	cKeyboard( void );
	~cKeyboard( void );

	// Reset all keys
	void Reset_Keys( void );

	/* Function : CEGUI_Handle_KeyUp
	 * CEGUI Key Up handler
	 * returns true if CEGUI processed the given key up event
	*/
	bool CEGUI_Handle_Key_Up( SDLKey key ) const;

	/* Function : KeyUp
	 * Gives player various cues depending on the key
	 * returns true if the event was processed
	*/
	bool Key_Up( SDLKey key );

	/* Function : CEGUI_Handle_KeyDown
	 * CEGUI Key Down handler
	 * returns true if CEGUI processed the given key down event
	*/
	bool CEGUI_Handle_Key_Down( SDLKey key ) const;

	/* Function : KeyDown
	 * Key Down handler
	 * returns true if the event was processed
	*/
	bool Key_Down( SDLKey key );

	// Get the assigned shortcut
	SDLKey *Get_Shortcut( const input_identifier shortcut_id ) const;
	// Assign a shortcut
	void Assign_Shortcut( const input_identifier shortcut_id, SDLKey new_key );

	// Translate a SDLKey to the proper CEGUI::Key
	unsigned int SDLKey_to_CEGUIKey( const SDLKey key ) const;

	// Pressed keys
	Uint8 keys[SDLK_LAST];
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// global Keyboard pointer
extern cKeyboard *pKeyboard;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
