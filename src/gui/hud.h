/***************************************************************************
 * hud.h  -  header for the corresponding cpp file
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

#ifndef SMC_HUD_H
#define SMC_HUD_H

#include "../objects/movingsprite.h"
#include "../core/obj_manager.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cHudSprite *** *** *** *** *** *** *** *** *** *** */

class cHudSprite : public cSprite
{
public:
	cHudSprite( cGL_Surface *new_image = NULL, float x = 0.0f, float y = 0.0f, bool del_img = 0 );
	virtual ~cHudSprite( void );
	
	// copy this sprite
	virtual cHudSprite *Copy( void );
};

/* *** *** *** *** *** *** *** cHud_Manager *** *** *** *** *** *** *** *** *** *** */

class cHud_Manager : public cObject_Manager<cHudSprite>
{
public:
	cHud_Manager( void );
	virtual ~cHud_Manager( void );

	// Load the complete HUD
	void Load( void );
	// Unload the complete HUD
	void Unload( void );

	// Update and reload text
	void Update_Text( void );
	// Update the objects
	void Update( void );
	// Draw the objects
	void Draw( void );

	typedef vector<cHudSprite *> HudSpriteList;

	// true if loaded
	bool m_loaded;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The HUD Manager
extern cHud_Manager *pHud_Manager;

/* *** *** *** *** *** *** *** *** PointsText *** *** *** *** *** *** *** *** *** */

struct PointsText : public cHudSprite
{
public:
	PointsText( void );
	virtual ~PointsText( void );

	float m_vely;
	unsigned int m_points;
};

/* *** *** *** *** *** cMenuBackground *** *** *** *** *** *** *** *** *** *** *** *** */

class cMenuBackground : public cHudSprite
{
public:
	cMenuBackground( float x = 0.0f, float y = 0.0f );
	virtual ~cMenuBackground( void );

	virtual void Draw( cSurface_Request *request = NULL );

	cGL_Surface *m_maryo_head;
	cGL_Surface *m_goldpiece;

	GL_point m_rect_maryo_head;
	GL_point m_rect_goldpiece;
};

/* *** *** *** *** *** cStatusText *** *** *** *** *** *** *** *** *** *** *** *** */

class cStatusText : public cHudSprite
{
public:
	cStatusText( float x = 0.0f, float y = 0.0f );
	virtual ~cStatusText( void );
	
	virtual void Draw( cSurface_Request *request = NULL );
};

/* *** *** *** *** *** cPlayerPoints *** *** *** *** *** *** *** *** *** *** *** *** */

class cPlayerPoints : public cStatusText
{
public:
	cPlayerPoints( float x = 0.0f, float y = 0.0f );
	virtual ~cPlayerPoints( void );

	void Set_Points( long points );
	void Add_Points( unsigned int points, float x = 0.0f, float y = 0.0f, std::string strtext = "", const Color &color = static_cast<Uint8>(255), bool allow_multiplier = 0 );

	// removes all point texts
	void Clear( void );

	virtual void Draw( cSurface_Request *request = NULL );

	typedef vector<PointsText *> PointsTextList;
	PointsTextList m_points_objects;
};

/* *** *** *** *** *** cGoldDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

class cGoldDisplay : public cStatusText
{
public:
	cGoldDisplay( float x = 0.0f, float y = 0.0f );
	virtual ~cGoldDisplay( void );

	void Set_Gold( int gold );
	void Add_Gold( int gold );

	virtual void Draw( cSurface_Request *request = NULL );
};

/* *** *** *** *** *** cLiveDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

class cLiveDisplay : public cStatusText
{
public:
	cLiveDisplay( float x = 0.0f, float y = 0.0f );
	virtual ~cLiveDisplay( void );

	void Set_Lives( int lives );
	void Add_Lives( int lives );

	virtual void Draw( cSurface_Request *request = NULL );
};

/* *** *** *** *** *** cTimeDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

class cTimeDisplay : public cStatusText
{
public:
	cTimeDisplay( float x = 0.0f, float y = 0.0f );
	virtual ~cTimeDisplay( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurface_Request *request = NULL );

	// reset
	void Reset( void );

	char m_text[50];
	Uint32 m_last_update_seconds;
	Uint32 m_milliseconds;
};

/* *** *** *** *** *** cItemBox *** *** *** *** *** *** *** *** *** *** *** *** */

class cItemBox : public cStatusText
{
public:
	cItemBox( float x = 0.0f, float y = 0.0f );
	virtual ~cItemBox( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurface_Request *request = NULL );

	/* Set the item
	* sound : if set the box sound is played
	*/
	void Set_Item( SpriteType item_type, bool sound = 1 );
	// Activates the itembox
	void Request_Item( void );
	// push the item back to the itembox
	void Push_back( void );
	
	void Reset( void );

	/* The current Item
	 * uses the Item defines
	 */
	SpriteType m_item_id;

	// alpha effect
	float m_item_counter;
	// alpha effect mod
	bool m_item_counter_mod;

	// itembox color
	Color m_box_color;

	// stored item
	cMovingSprite *m_item;
};


/* *** *** *** *** *** cLifeDisplay *** *** *** *** *** *** *** *** *** *** *** *** */


class cLifeDisplay : public cStatusText
{
public:
	cLifeDisplay( float x = 0.0f, float y = 0.0f );
	virtual ~cLifeDisplay( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurface_Request *request = NULL );

	//set life amount
	void Set_Life( int life_bar);
	
	//decreases life amount by amount decreased
	void Decrease_Life( int life_decreased );

	//increases life from mushroom by amount increased
	void IncreaseLife( int life_increased);

	//resets life bar
	void Reset( void );

	// alpha effect
	float m_item_counter;
	// alpha effect mod
	bool m_item_counter_mod;

	// itembox color
	Color m_box_color;
	
	//life_amount
	int life_amount;
};


/* *** *** *** *** *** cDebugDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

class cDebugDisplay : public cStatusText
{
public:
	cDebugDisplay( float x = 0.0f, float y = 0.0f );
	virtual ~cDebugDisplay( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurface_Request *request = NULL );
	// draw the frames per second info
	void Draw_fps( void );
	// draw the debug mode info
	void Draw_Debug_Mode( void );
	// draw the performance debug mode info
	void Draw_Performance_Debug_Mode( void );

	// set the debug text to display
	void Set_Text( const std::string &ntext, float display_time = speedfactor_fps * 2.0f );

	// display text
	std::string m_text, m_text_old;
	// text counter
	float m_counter;

	// CEGUI debug text
	CEGUI::Window *m_window_debug_text, *m_text_debug_text;

	// last game mode
	GameMode m_game_mode_last;
	// last level text
	std::string m_level_old;
	// last object counters
	int m_obj_counter, m_pass_counter, m_mass_counter, m_enemy_counter, m_active_counter;
	// sprites
	typedef vector<cHudSprite *> HudSpriteList;
	HudSpriteList m_sprites;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The HUD
extern cPlayerPoints *pHud_Points;
extern cDebugDisplay *pHud_Debug;
extern cGoldDisplay *pHud_Goldpieces;
extern cLiveDisplay *pHud_Lives;
extern cTimeDisplay *pHud_Time;
extern cItemBox *pHud_Itembox;
extern cLifeDisplay *pHud_LifeDisplay;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
