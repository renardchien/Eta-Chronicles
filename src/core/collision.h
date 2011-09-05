/***************************************************************************
 * collision.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2005 - 2009 Florian Richter
 * Copyright (C) 2005 Amir Taaki ( Circle Collision tests ) - MIT License
 * Copyright (C) 2005 Magnus Norddahl ( Line Collision tests ) - BSD License
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_COLLISION_H
#define SMC_COLLISION_H

#include "../core/global_basic.h"
#include "../core/global_game.h"
#include "../core/obj_manager.h"
#include "SDL.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cObjectCollision *** *** *** *** *** *** *** *** *** *** */

// Object collision data
class cObjectCollision
{
public:
	cObjectCollision( void );
	~cObjectCollision( void );

	/* Set the collision direction
	 * base - the base sprite
	 * col - the colliding sprite
	*/
	void Set_Direction( const cSprite *base, const cSprite *col );
	// returns the colliding object rect
	GL_rect Get_Collision_Object_Rect( void );

	// valid type
	Col_Valid_Type valid_type;

	/* if true this is a receiving collision
	 * and not a normal self detected collision
	*/
	bool received;

	// the object colliding with (only use it in the same frame for now !)
	cSprite *obj;
	// colliding object number
	int number;

	// direction
	ObjectDirection direction;
	// Colliding object array type
	ArrayType m_array;
};

typedef vector<cObjectCollision *> cObjectCollision_List;

/* *** *** *** *** *** *** *** cObjectCollisionType *** *** *** *** *** *** *** *** *** *** */

// collision type class
class cObjectCollisionType : public cObject_Manager<cObjectCollision>
{
public:
	cObjectCollisionType( void );
	virtual ~cObjectCollisionType( void );

	// Add an object collision
	virtual void Add( cObjectCollision *obj );

	// returns true if the given object was found in the list
	bool Is_Included( const cSprite *obj );
	// returns true if the given array type was found in the list
	bool Is_Included( const ArrayType type );
	// returns true if the sprite type was found in the list
	bool Is_Included( const SpriteType type );
	// returns true if the validation type was found in the list
	bool Is_Included( const Col_Valid_Type type );

	// returns the first found sprite if the given array type was found
	cObjectCollision *Find_First( const ArrayType type );
	// returns the first found sprite if the given sprite type was found
	cObjectCollision *Find_First( const SpriteType type );
};

/* *** *** *** *** *** *** *** functions *** *** *** *** *** *** *** *** *** *** */

/* Returns the collision direction
 * base - the base sprite
 * col - the colliding sprite
*/
ObjectDirection Get_Collision_Direction( const cSprite *base, const cSprite *col );

// returns true if a collision is on the top
bool Is_Collision_Top( const GL_rect &base_rect, const GL_rect &col_rect );
// returns true if a collision is on the bottom
bool Is_Collision_Bottom( const GL_rect &base_rect, const GL_rect &col_rect );
// returns true if a collision is on the left
bool Is_Collision_Left( const GL_rect &base_rect, const GL_rect &col_rect );
// returns true if a collision is on the right
bool Is_Collision_Right( const GL_rect &base_rect, const GL_rect &col_rect );

/*
 * Bounding box collision test
 * Checks if the first rect intersects with the second
*/
bool Col_Box( const SDL_Rect &a, const GL_rect &b );

/*
 * Bounding box collision test (SDL_Rect)
 * Checks if the first rect intersects completely with the second
*/
bool Col_Box_full( const SDL_Rect &a, const SDL_Rect &b );

/*
 * tests whether 2 circles intersect
 *
 * Parameters:
 * circle1 - center (x1,y1) with radius r1
 * circle2 - center (x2,y2) with radius r2
 * (allow distance between circles of offset)
*/
bool Col_Circle( float x1, float y1, float r1, float x2, float y2, float r2, int offset = 1);
/*
 * a circle intersection detection algorithm that will use
 * the position of the center of the surface as the center of
 * the circle and approximate the radius using the width and height
 * of the surface (for example a rect of 4x6 would have r = 2.5).
*/
bool Col_Circle( cGL_Surface *surface1, SDL_Rect *a, cGL_Surface *surface2, SDL_Rect *b, int offset = 0 );
bool Col_Circle( cGL_Surface *a, float x1, float y1, cGL_Surface *b, float x2, float y2, int offset );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
