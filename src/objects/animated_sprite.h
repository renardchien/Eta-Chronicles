/***************************************************************************
 * animated_sprite.h  -  header for the corresponding cpp file
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

#ifndef SMC_ANIMATED_SPRITE_H
#define SMC_ANIMATED_SPRITE_H

#include "../objects/movingsprite.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cAnimation_Surface *** *** *** *** *** *** *** *** *** *** */

class cAnimation_Surface
{
public:
	cAnimation_Surface( void );
	~cAnimation_Surface( void );

	// the image
	cGL_Surface *m_image;
	// time to display in milliseconds
	Uint32 m_time;
};

/* *** *** *** *** *** *** *** cAnimated_Sprite *** *** *** *** *** *** *** *** *** *** */

class cAnimated_Sprite : public cMovingSprite
{
public:
	cAnimated_Sprite( float x = 0.0f, float y = 0.0f );
	virtual ~cAnimated_Sprite( void );
	
	/* Add an image to the animation
	 * NULL image is allowed
	 * time: if not set uses the default display time
	*/
	void Add_Image( cGL_Surface *image, Uint32 time = 0 );
	// Set the animation start and end image
	void Set_Animation_Image_Range( int start, int end );
	/* Set the image using the given array number
	 * if new_start_image is set the default start_image will be set to the given image
	 * if del_img is set the given image will be deleted
	*/
	virtual void Set_Image_Num( const int num, const bool new_startimage = 0, const bool del_img = 0 );
	// Get an array image
	cGL_Surface *Get_Image( const unsigned int num ) const;
	// Clear the image list
	void Clear_Images( void );

	/* Set if the animation is enabled
	 * default : disabled
	*/
	void Set_Animation( bool enabled = 0 );
	// Reset animation back to the first image
	void Reset_Animation( void );

	// update animation
	void Update_Animation( void );

	// Set default image display time
	void Set_Default_Time( Uint32 time = 1000 );
	/* Set display time for all images
	 * default_time: if set also make it the default time
	*/
	void Set_Time_All( Uint32 time, bool default_time = 0 );

	// currently set image array number
	int m_curr_img;
	// if animation is enabled
	bool m_anim_enabled;
	// animation start image
	int m_anim_img_start;
	// animation end image
	int m_anim_img_end;
	// default animation time
	Uint32 m_anim_time_default;
	// animation counter
	Uint32 m_anim_counter;

	// Surface list
	typedef vector<cAnimation_Surface> cAnimation_Surface_List;
	cAnimation_Surface_List m_images;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif

