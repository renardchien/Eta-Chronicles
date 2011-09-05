/***************************************************************************
 * sprite.h  -  header for the corresponding cpp file
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

#ifndef SMC_SPRITE_H
#define SMC_SPRITE_H

#include "../core/global_game.h"
#include "../core/math/rect.h"
#include "../video/video.h"
#include "../core/collision.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cCollidingSprite *** *** *** *** *** *** *** *** *** *** */

class cCollidingSprite
{
public:
	cCollidingSprite( void );
	virtual ~cCollidingSprite( void );

	// Handle collision data
	void Handle_Collisions( void );
	/* Create a collision object
	 * base : base/origin object
	 * col : colliding object ( needed for the collision direction )
	 * valid_type : validation type
	*/
	cObjectCollision *Create_Collision_Object( const cSprite *base, cSprite *col, Col_Valid_Type valid_type ) const;
	/* Add a collision object to the collision list
	 * returns true if successful
	 * add_if_new : add the collision only if the collision object doesn't already collide with us and is deleted if it does
	 */
	bool Add_Collision( cObjectCollision *collision, bool add_if_new = 0 );
	/* Add the collision objects to the collision list
	 * add_if_new : add each collision only if the collision object doesn't already collide with us and is deleted if it does
	 */
	void Add_Collisions( cObjectCollisionType *col_list, bool add_if_new = 0 );

	// Delete the given collision from the list
	void Delete_Collision( cObjectCollision *collision );
	// Delete the last collision
	void Delete_Last_Collision( void );
	/* Check if a collision is active in the given direction 
	 * and returns the collision object number else -1
	 */
	int Is_Collision_In_Direction( const ObjectDirection dir ) const;
	// Returns the first added collision
	cObjectCollision *Get_First_Collision( void ) const;
	/* Returns the last added collision
	 * if only_blocking is set only movement blocking collisions are returned
	*/
	cObjectCollision *Get_Last_Collision( bool only_blocking = 0 ) const;
	// returns true if the given collision obj was found in the list
	bool Is_Collision_Included( const cSprite *obj ) const;
	// Clear the Collision list
	void Clear_Collisions( void );

	// default collision handler
	virtual void Handle_Collision( cObjectCollision *collision );
	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );
	// collision from an enemy
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );
	// collision with passive
	virtual void Handle_Collision_Passive( cObjectCollision *collision );
	// collision from a box
	virtual void Handle_Collision_Box( ObjectDirection cdirection, GL_rect *r2 );

	// object collision list
	cObjectCollision_List collisions;
};

/* *** *** *** *** *** *** *** cSprite *** *** *** *** *** *** *** *** *** *** */

class cSprite : public cCollidingSprite
{
public:
	// if del_img is set the given image will be deleted on change or class deletion
	cSprite( cGL_Surface *new_image = NULL, float x = 0, float y = 0, bool del_img = 0 );
	// create from stream
	cSprite( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cSprite( void );

	// initialize defaults
	virtual void Init( void );
	/* late initialization
	 * this needs linked objects to be already loaded
	*/
	virtual void Init_Links( void );
	// copy this sprite
	virtual cSprite *Copy( void );

	// create from stream
	virtual void Create_From_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_Stream( ofstream &file );

	// load from savegame
	virtual void Load_From_Savegame( cSave_Level_Object *save_object );
	// save to savegame
	virtual cSave_Level_Object *Save_To_Savegame( void );

	/* Sets the image for drawing
	 * if new_start_image is set the default start_image will be set to the given image
	 * if del_img is set the given image will be deleted
	*/
	virtual void Set_Image( cGL_Surface *new_image, bool new_start_image = 0, bool del_img = 0 );

	// Set the sprite type
	void Set_Sprite_Type( SpriteType ntype );
	// Returns the sprite type as string
	std::string Get_Sprite_Type_String( void ) const;

	/* Set if the camera should be ignored
	 * default : disabled
	*/
	void Set_Ignore_Camera( bool enable = 0 );

	// Sets the Position
	void Set_Pos( float x, float y, bool new_startpos = 0 );
	void Set_Pos_X( float x, bool new_startpos = 0 );
	void Set_Pos_Y( float y, bool new_startpos = 0 );
	// Set if active
	void Set_Active( bool enabled );
	/* Set the shadow
	 * if position is set to 0 the shadow is disabled
	*/
	void Set_Shadow( const Color &shadow, float pos );
	/* Set the shadow position
	 * if set to 0 the shadow is disabled
	*/
	void Set_Shadow_Pos( float pos );
	// Set the shadow color
	void Set_Shadow_Color( const Color &shadow );
	// Set image color
	void Set_Color( Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha = 255 );
	void Set_Color( const Color &col );
	/* Set a Color Combination ( GL_ADD, GL_MODULATE or GL_REPLACE )
	 * Addition ( adds white to color )
	 * 1.0 is the maximum and the given color will be white
	 * 0.0 is the minimum and the color will have the default color
	 * Modulation ( adds black to color )
	 * 1.0 is the maximum and the color will have the default color
	 * 0.0 is the minimum and the given color will be black
	 * Replace ( replaces color value )
	 * 1.0 is the maximum and the given color has maximum value
	 * 0.0 is the minimum and the given color has minimum value
	*/
	void Set_Color_Combine( float red, float green, float blue, GLint com_type );

	/* Set if rotation affects the collision rect
	 * only supports 90° steps currently
	 * if enabled col_pos, col_rect and rect must be reset manually before changing rotation
	*/
	void Set_Rotation_Affects_Rect( bool enable = 0 );
	/* Set the rect rotation
	 * does not reset col_pos, col_rect and rect before rotation
	 * default : disabled
	*/
	void Update_Rect_Rotation( void );
	// Set the X rect rotation
	void Update_Rect_Rotation_X( void );
	// Set the Y rect rotation
	void Update_Rect_Rotation_Y( void );
	// Set the Z rect rotation
	void Update_Rect_Rotation_Z( void );

	// Set the X rotation
	void Set_Rotation_X( float rot, bool new_start_rot = 0 );
	// Set the Y rotation
	void Set_Rotation_Y( float rot, bool new_start_rot = 0 );
	// Set the Z rotation
	void Set_Rotation_Z( float rot, bool new_start_rot = 0 );
	// Set the rotation
	void Set_Rotation( float x, float y, float z, bool new_start_rot = 0 );

	// Add X rotation
	void Add_Rotation_X( float rot );
	// Add Y rotation
	void Add_Rotation_Y( float rot );
	// Add Z rotation
	void Add_Rotation_Z( float rot );
	// Add rotation
	void Add_Rotation( float x, float y, float z );

	/* Set if scale affects the collision rect
	 * if enabled previous scale is always undone from rect before setting the new value
	 * default : disabled
	*/
	void Set_Scale_Affects_Rect( bool enable = 0 );
	/* Set which directions of the image get scaled
	 * if all are set scaling is centered and if all are not set scaling is disabled
	 * todo : if enabled with scale_affects_rect it should also scale the rect position
	*/
	void Set_Scale_Directions( bool up = 0, bool down = 1, bool left = 0, bool right = 1 );
	// Set the scale
	void Set_Scale_X( const float scale, const bool new_startscale = 0 );
	void Set_Scale_Y( const float scale, const bool new_startscale = 0 );
	void Set_Scale( const float scale, const bool new_startscale = 0 );

	// Add scale
	void Add_Scale_X( const float val );
	void Add_Scale_Y( const float val );
	void Add_Scale( const float val );

	// Set this sprite on top of the given one
	void Set_On_Top( const cSprite *sprite, bool optimize_hor_pos = 1 );

	/* Move this object
	 * real : if set the speedfactor is not used
	*/
	virtual void Move( float move_x, float move_y, const bool real = 0 );

	// default collision and movement handling
	virtual void Collide_Move( void );

	// Update the position rect values
	void Update_Position_Rect( void );
	// default update
	virtual void Update( void );
	/* late update
	 * use if it is needed that other objects are already updated
	*/
	virtual void Update_Late( void );
	// update drawing validation
	virtual void Update_Valid_Draw( void );
	// update updating validation
	virtual void Update_Valid_Update( void );

	/* Draw
	* if request is NULL automatically creates the request
	*/
	virtual void Draw( cSurface_Request *request = NULL );
	/* only draws the image
	 * no position nor debug updates
	*/
	void Draw_Image( cSurface_Request *request = NULL ) const;

	/* Set the massive type
	 * should be called after setting the new array
	*/
	virtual void Set_Massive_Type( MassiveType mtype );

	// Check if this sprite is on top of the given object
	bool Is_On_Top( const cSprite *obj ) const;

	// if the sprite is visible on the screen
	bool Is_Visible_On_Screen( void ) const;
	// if the Object is in player range
	bool Is_In_Player_Range( void ) const;
	// if update is valid for the current state
	virtual bool Is_Update_Valid( void );
	// if draw is valid for the current state and position
	virtual bool Is_Draw_Valid( void );

	/* set this sprite to destroyed and completely disable it
	 * sprite is still in the sprite manager but only to get possibly replaced
	*/
	virtual void Destroy( void );

	// editor add window object
	void Editor_Add( const CEGUI::String &name, const CEGUI::String &tooltip, CEGUI::Window *window_setting, float obj_width, float obj_height = 28, bool advance_row = 1 );
	// editor activation
	virtual void Editor_Activate( void );
	// editor deactivation
	virtual void Editor_Deactivate( void );
	// editor init
	virtual void Editor_Init( void );
	// editor position update
	virtual void Editor_Position_Update( void );
	// editor state update
	virtual void Editor_State_Update( void );

	// current image used for drawing
	cGL_Surface *m_image;
	// editor and first image
	cGL_Surface *m_start_image;

	// complete image rect
	GL_rect m_rect;
	// editor and first image rect
	GL_rect m_start_rect;
	// collision rect
	GL_rect m_col_rect;
	// collision start point
	GL_point m_col_pos;

	// current position
	float m_pos_x, m_pos_y, m_pos_z;
	// start position
	float m_start_pos_x, m_start_pos_y;
	/* editor z position
	 * it's only used if not 0
	*/
	float m_editor_pos_z;

	// if set rotation not only affects the image but also the rectangle
	bool m_rotation_affects_rect;
	// editor and start rotation
	float m_start_rot_x, m_start_rot_y, m_start_rot_z;
	// rotation
	float m_rot_x, m_rot_y, m_rot_z;
	// if set scale not only affects the image but also the rectangle
	bool m_scale_affects_rect;
	/* which parts of the image get scaled
	 * if all are set scaling is centered
	*/
	bool m_scale_up, m_scale_down, m_scale_left, m_scale_right;
	// editor and start scale
	float m_start_scale_x, m_start_scale_y;
	// scale
	float m_scale_x, m_scale_y;

	// color
	Color m_color;
	// combine type
	GLint m_combine_type;
	// combine color
	float m_combine_color[3];

	// sprite type
	SpriteType m_type;
	// sprite array type
	ArrayType m_sprite_array;
	// massive collision type
	MassiveType m_massive_type;
	// visible name
	std::string m_name;
	// sprite editor tags
	std::string m_editor_tags;

	// true if not using the camera position
	bool m_no_camera;
	// if true we are active and can be updated and drawn
	bool m_active;
	// if spawned it shouldn't be saved
	bool m_spawned;
	// range to the player to get updates
	unsigned int m_player_range;
	// can be used as ground object
	bool m_can_be_ground;

	// delete the given image when it gets unloaded
	bool m_delete_image;
	/* if true this sprite is ready to be replaced with a later created sprite
	 * and this sprite is not used anywhere anymore
	 * should not be used for objects needed by the editor
	 * should be used for not active spawned objects
	*/
	bool m_auto_destroy;
	// shadow position
	float m_shadow_pos;
	// shadow color
	Color m_shadow_color;

	// if drawing is valid
	bool m_valid_draw;
	// if updating is valid
	bool m_valid_update;

	// editor active window list
	typedef vector<cEditor_Object_Settings_Item *> Editor_Object_Settings_List;
	Editor_Object_Settings_List m_editor_windows;
	// width for all name windows based on largest name text width
	float m_editor_window_name_width;
};

typedef vector<cSprite *> cSprite_List;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
