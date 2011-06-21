/***************************************************************************
 * enemy.h  -  headers for the corresponding cpp file
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

#ifndef SMC_ENEMY_H
#define SMC_ENEMY_H

#include "../objects/animated_sprite.h"
#include "../core/framerate.h"
#include "../audio/audio.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cEnemy : public cAnimated_Sprite
{
public:
	// constructor
	cEnemy( float x = 0.0f, float y = 0.0f );
	// destructor
	virtual ~cEnemy( void );

	// load from savegame
	virtual void Load_From_Savegame( cSave_Level_Object *save_object );
	// save to savegame
	virtual cSave_Level_Object *Save_To_Savegame( void );

	// Set Dead
	virtual void Set_Dead( bool enable = 1 );

	// dying animation update
	virtual void Update_Dying( void );
	// handle basic enemy updates
	virtual void Update( void );
	/* late update
	 * use if it is needed that other objects are already updated
	*/
	virtual void Update_Late( void );
	// update gravity velocity
	virtual void Update_Gravity( void );
	
	// Generates the default Hit Animation Particles
	void Generate_Hit_Animation( cParticle_Emitter *anim = NULL ) const;

	// default collision handler
	virtual void Handle_Collision( cObjectCollision *collision );
	// handle moved out of Level event
	virtual void Handle_out_of_Level( ObjectDirection dir );

	//return enemy life
	virtual signed int return_life_left ();

	//decrease enemy life
	virtual void set_life_left (signed int damage_type);

	// if dead
	bool m_dead;

	// default counter for animations
	float m_counter;

	// sound filename if got killed
	std::string m_kill_sound;
	// points if enemy got killed
	unsigned int m_kill_points;

	// life each enemy has
	signed int m_life_left;

	// is fire able to kill this enemy
	bool m_fire_resistant;

	// if this can be hit from the top
	//bool m_can_be_hit_from_top;
	// if this can be hit from a shell
	bool m_can_be_hit_from_shell;
	// if this moves into an abyss
	//bool m_moves_into_abyss;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
