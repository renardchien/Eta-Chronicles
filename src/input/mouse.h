/***************************************************************************
 * mouse.h  -  header for the corresponding cpp file
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

#ifndef SMC_MOUSE_H
#define SMC_MOUSE_H

#include "../core/globals.h"
#include "../objects/movingsprite.h"
#include "../core/math/rect.h"
#include "../core/math/vector.h"

namespace SMC
{

/* *** *** *** *** *** *** cSelectedObject *** *** *** *** *** *** *** *** *** *** *** */

class cSelectedObject
{
public:
	cSelectedObject( void );
	~cSelectedObject( void );
	
	// base object
	cSprite *obj;
	// additional selection mouse position
	int mouse_w, mouse_h;

	// if user controlled
	bool user;
};

typedef vector<cSelectedObject *> SelectedObjectList;

/* *** *** *** *** *** *** cCopyObject *** *** *** *** *** *** *** *** *** *** *** */

class cCopyObject
{
public:
	cCopyObject( void );
	~cCopyObject( void );
	
	// base object
	cSprite *obj;
};

typedef vector<cCopyObject *> CopyObjectList;

/* *** *** *** *** *** *** cMouseCursor *** *** *** *** *** *** *** *** *** *** *** */

class cMouseCursor : public cMovingSprite
{
public:
	cMouseCursor( void );
	virtual ~cMouseCursor( void );

	// resets editor features
	void Reset( bool clear_copy_buffer = 1 );
	// only reset buttons
	void Reset_Keys( void );

	// Handles the Mouse events
	bool Handle_Event( SDL_Event *ev );
	/* GUI handle mouse down function
	 * Returns true if GUI handled the event
	*/
	bool Handle_Mouse_Down( Uint8 button );
	/* GUI handle mouse up function
	 * Returns true if GUI handled the event
	*/
	bool Handle_Mouse_Up( Uint8 button );

	/* Get the first object colliding with the mouse which is valid for the editor
	 * returns the collision if an object was found else NULL
	 * The collision data should be deleted if not used anymore
	*/
	cObjectCollision *Get_First_Editor_Collsion( float px = 0.0f, float py = 0.0f );
	/* Get the first object colliding with the mouse
	 * returns the collision if an object was found else NULL
	 * The collision data should be deleted if not used anymore
	*/
	cObjectCollision *Get_First_Mouse_Collision( const GL_rect &mouse_rect );

	// Update
	virtual void Update( void );
	// Draw
	virtual void Draw( void );
	// Render
	void Render( void ) const;

	// Update the Mouse Position
	void Update_Position( void );
	// Update the Double Click event
	void Update_Doubleclick( void );

	// Editor default click
	void Click( void );
	// Editor default float click
	void Double_Click( bool activate = 1 );

	// ## Mouse Object
	// Set new mouse object
	void Set_Mouse_Object( cSprite *sprite );
	// update the mouse object position
	void Update_Mouse_Object( void );
	// clear mouse object
	void Clear_Mouse_Object( void );

	// ## Copy Objects
	// Add new copy object
	void Add_Copy_Object( cSprite *sprite );
	void Add_Copy_Objects( cSprite_List &spritelist );
	/* Remove a copy_object
	 * returns true if removed
	*/
	bool Remove_Copy_Object( const cSprite *sprite );
	// clear copy object list
	void Clear_Copy_Objects( void );
	/* returns the position of the first top left copy object
	 * if no copy_objects returns 0, 0
	*/
	GL_Vector Get_Copy_Object_Base( float px, float py );
	// paste copy object list
	void Paste_Copy_Objects( float px, float py );

	// ## Selected Objects
	/* Add new selected_object
	 * Returns true if object got added or changed to user object
	*/
	bool Add_Selected_Object( cSprite *sprite, bool from_user = 0 );
	void Add_Selected_Objects( cSprite_List &spritelist, bool from_user = 0 );
	/* Remove a selected_object
	 * returns true if found and removed
	*/
	bool Remove_Selected_Object( const cSprite *sprite, bool no_user = 0 );
	// Returns all selected objects
	cSprite_List Get_Selected_Objects( void );
	// clear selected object list
	void Clear_Selected_Objects( void );
	// updates the selected objects positions
	void Update_Selected_Objects( void );
	/* returns true if the given object is a selected_object
	 * if only_user is given only check user objects
	*/
	bool Is_Selected_Object( const cSprite *sprite, bool only_user = 0 );
	// deletes the selected objects
	void Delete_Selected_Objects( void );
	// returns the selected objects size
	unsigned int Get_Selected_Object_Size( void ) const;

	// ## Active Object
	// Set new active object
	void Set_Active_Object( cSprite *sprite );
	// clear active object
	void Clear_Active_Object( void );

	/* Copies the given object to the given position
	 * object is automatically added to the object manager
	 * returns the new object
	*/
	cSprite *Copy( cSprite *ncopy_object, float px, float py ) const;
	// Deletes the given Object
	void Delete( cSprite *sprite );

	// Draws a rect around the m_hovering_object and selected objects
	void Draw_Object_Rects( void );

	// Start selection mode
	void Start_Selection( void );
	// End selection mode
	void End_Selection( void );
	// update selection mode
	void Update_Selection( void );

	// Toggle Mover mode
	void Toggle_Mover_Mode( void );
	// Updates the Mover Mode
	void Mover_Update( Sint16 move_x, Sint16 move_y );
	// Updates the Leveleditor Mouse
	void Editor_Update( void );
	
	// current internal mouse position
	int m_x, m_y;
	
	// selection mode
	bool m_selection_mode;
	// Selection rect
	GL_rect m_selection_rect;

	// if activated the mouse cursor movement moves the screen	
	bool m_mover_mode;
	// fast copy mode
	bool m_fastcopy_mode;

	/* selected objects
	 * the mouse object is also always a selected object
	*/
	SelectedObjectList m_selected_objects;
	// currently colliding object with the mouse
	cSelectedObject *m_hovering_object;
	// objects selected for copying
	CopyObjectList m_copy_objects;
	// settings activated object
	cSprite *m_active_object;

	// buttons pressed state
	bool m_left, m_right, m_middle;

	// last clicked object
	cSprite *m_last_clicked_object;
	// counter for catching double-clicks
	float m_click_counter;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Mouse
extern cMouseCursor *pMouseCursor;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
