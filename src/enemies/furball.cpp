/***************************************************************************
 * furball.cpp  -  little moving around enemy
 *
 * Copyright (C) 2011 - Cody Van De Mark
 * Copyright (C) 2003 - 2009 Florian Richter (Original)
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../enemies/furball.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../gui/hud.h"
#include "../core/i18n.h"
#include "../video/animation.h"
// CEGUI
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** *** cFurball *** *** *** *** *** *** *** *** *** *** *** */

cFurball :: cFurball( float x, float y )
: cEnemy( x, y )
{
	cFurball::Init();
}

cFurball :: cFurball( CEGUI::XMLAttributes &attributes )
: cEnemy()
{
	cFurball::Init();
	cFurball::Create_From_Stream( attributes );
}

cFurball :: ~cFurball( void )
{
	//
}

void cFurball :: Init( void )
{
	m_type = TYPE_FURBALL;
	m_pos_z = 0.09f;

	m_counter_hit = 0.0f;
	m_counter_running = 0.0f;
	m_running_particle_counter = 0.0f;
	m_downgrade_count = 0;
	m_max_downgrade_count = 5;

	m_color_type = COL_DEFAULT;
	Set_Color( COL_BROWN );
	m_state = STA_FALL;
	Set_Moving_State( STA_WALK );
	Set_Direction( DIR_RIGHT );
}

cFurball *cFurball :: Copy( void )
{
	cFurball *furball = new cFurball( m_start_pos_x, m_start_pos_y );
	furball->Set_Color( m_color_type );
	furball->Set_Direction( m_start_direction );
	if( m_type == TYPE_FURBALL_BOSS )
	{
		furball->Set_Max_Downgrade_Count( m_max_downgrade_count );
	}

	return furball;
}

void cFurball :: Create_From_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// color
	Set_Color( static_cast<DefaultColor>(Get_Color_Id( attributes.getValueAsString( "color", Get_Color_Name( m_color_type ) ).c_str() )) );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( m_start_direction ) ).c_str() ) );
	if( m_type == TYPE_FURBALL_BOSS )
	{
		// max downgrade count
		Set_Max_Downgrade_Count( attributes.getValueAsInteger( "max_downgrade_count", m_max_downgrade_count ) );
	}
}

void cFurball :: Save_To_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"furball\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(m_start_pos_x) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(m_start_pos_y) << "\" />" << std::endl;
	// color
	file << "\t\t<Property name=\"color\" value=\"" << Get_Color_Name( m_color_type ) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( m_start_direction ) << "\" />" << std::endl;
	if( m_type == TYPE_FURBALL_BOSS )
	{
		// max downgrade count
		file << "\t\t<Property name=\"max_downgrade_count\" value=\"" << m_max_downgrade_count << "\" />" << std::endl;
	}

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cFurball :: Load_From_Savegame( cSave_Level_Object *save_object )
{
	cEnemy::Load_From_Savegame( save_object );

	Update_Rotation_Hor_velx();
}

void cFurball :: Set_Max_Downgrade_Count( int nmax_downgrade_counts )
{
	m_max_downgrade_count = nmax_downgrade_counts;

	if( m_max_downgrade_count < 0 )
	{
		m_max_downgrade_count = 0;
	}
}

void cFurball :: Set_Direction( const ObjectDirection dir )
{
	// already set
	if( m_start_direction == dir )
	{
		return;
	}

	cEnemy::Set_Direction( dir, 1 );

	Update_Velocity();
	Update_Rotation_Hor_velx( 1 );
	Create_Name();
}

void cFurball :: Set_Color( const DefaultColor &col )
{
	// already set
	if( m_color_type == col )
	{
		return;
	}

	// clear old images
	Clear_Images();

	m_color_type = col;
	std::string filename_dir;

	if( m_color_type == COL_BROWN )
	{
		filename_dir = "brown";

		m_speed_walk = 2.7f;
		m_kill_points = 10;
		m_life_left = 150;
		m_fire_resistant = 0;
		m_ice_resistance = 0.0f;
		m_can_be_hit_from_shell = 1;
	}
	else if( m_color_type == COL_BLUE )
	{
		filename_dir = "blue";

		m_speed_walk = 4.5f;
		m_kill_points = 50;
		m_life_left = 175;
		m_fire_resistant = 0;
		m_ice_resistance = 0.9f;
		m_can_be_hit_from_shell = 1;
	}
	else if( m_color_type == COL_BLACK )
	{
		filename_dir = "boss";
		m_type = TYPE_FURBALL_BOSS;

		m_speed_walk = 4.0f;
		m_kill_points = 2500;
		m_life_left = 550;
		m_fire_resistant = 1;
		m_ice_resistance = 1.0f;
		m_can_be_hit_from_shell = 0;
	}
	else
	{
		printf( "Error : Unknown Furball Color %d\n", m_color_type );
		return;
	}

	Update_Velocity();
	Update_Rotation_Hor_velx();

	Add_Image( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_1.png" ) );
	Add_Image( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_2.png" ) );
	Add_Image( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_3.png" ) );
	Add_Image( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_4.png" ) );
	Add_Image( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_5.png" ) );
	Add_Image( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_6.png" ) );
	Add_Image( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_7.png" ) );
	Add_Image( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_8.png" ) );
	Add_Image( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/turn.png" ) );
	Add_Image( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/dead.png" ) );

	// boss has hit image
	if( m_type == TYPE_FURBALL_BOSS )
	{
		Add_Image( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/hit.png" ) );
	}
	else
	{
		Add_Image( NULL );
	}

	Set_Image_Num( 0, 1 );
	Create_Name();
}

void cFurball :: Turn_Around( ObjectDirection col_dir /* = DIR_UNDEFINED */ )
{
	cEnemy::Turn_Around( col_dir );

	if( col_dir == DIR_LEFT || col_dir == DIR_RIGHT )
	{
		// set turn around image
		Set_Image_Num( 8 );
		Set_Animation( 0 );
		Reset_Animation();
	}

	Update_Rotation_Hor_velx();
}

void cFurball :: DownGrade( bool force /* = 0 */ )
{
	// boss
	if( !force && m_type == TYPE_FURBALL_BOSS )
	{
		// can not get hit if staying or running
		if( m_state == STA_STAY || m_state == STA_RUN )
		{
			return;
		}

		m_downgrade_count++;
		m_speed_walk += 1.4f;

		// die
		if( m_downgrade_count == m_max_downgrade_count )
		{
			Set_Dead( 1 );
		}
		else
		{
			Generate_Hit_Animation();
			// set hit image
			Set_Image_Num( 10 );
			Set_Moving_State( STA_STAY );
		}
	}
	// normal
	else
	{
		Set_Dead( 1 );
	}

	if( m_dead )
	{
		m_massive_type = MASS_PASSIVE;
		m_counter = 0.0f;
		m_velx = 0.0f;
		m_vely = 0.0f;
		// dead image
		Set_Image_Num( 9 );
		Set_Animation( 0 );

		// default stomp death
		if( !force || m_type == TYPE_FURBALL_BOSS )
		{
			Generate_Hit_Animation();

			if( m_type != TYPE_FURBALL_BOSS )
			{
				Set_Scale_Directions( 1, 0, 1, 1 );
			}
			else
			{
				// set scaling for death animation
				Set_Scale_Affects_Rect( 1 );

				// fade out music
				pAudio->Fadeout_Music( 500 );
				// play finish music
				pAudio->Play_Music( "game/courseclear.ogg", 0, 0 );
			}
		}
		// falling death
		else
		{
			Set_Rotation_Z( 180 );
			Set_Scale_Directions( 1, 1, 1, 1 );
		}
	}
}

void cFurball :: Update_Dying( void )
{
	// stomp death
	if( !Is_Float_Equal( m_rot_z, 180.0f ) )
	{
		// boss
		if( m_type == TYPE_FURBALL_BOSS )
		{
			m_counter += pFramerate->m_speed_factor * 0.5f;

			if( m_scale_x > 0.1f )
			{
				float speed_x = pFramerate->m_speed_factor * 10.0f;

				if( m_direction == DIR_LEFT )
				{
					speed_x *= -1;
				}

				Add_Rotation_Z( speed_x );
				Add_Scale( -pFramerate->m_speed_factor * 0.025f );

				// star animation
				if( m_counter >= 1.0f )
				{
					Generate_Smoke( static_cast<unsigned int>(m_counter), 0.3f );
					m_counter -= static_cast<int>(m_counter);
				}

				// finished scale out animation
				if( m_scale_x <= 0.1f )
				{
					// sound
					pAudio->Play_Sound( m_kill_sound );

					// star explosion animation
					Generate_Smoke( 30 );

					// set empty image
					cMovingSprite::Set_Image( NULL, 0, 0 );
					// reset counter
					m_counter = 0;
				}
			}
			// after scale animation
			else
			{
				// wait some time
				if( m_counter > 20.0f )
				{
					// next level
					pPlayer->Goto_Next_Level();

					// reset scaling
					Set_Scale_Affects_Rect( 0 );
				}
			}
		}
		// normal
		else
		{
			// scale out
			float speed = pFramerate->m_speed_factor * 0.05f;

			Add_Scale_X( -speed * 0.5f );
			Add_Scale_Y( -speed );

			if( m_scale_y < 0.01f )
			{
				Set_Scale( 1.0f );
				Set_Active( 0 );
			}
		}
	}
	// falling death
	else
	{
		m_counter += pFramerate->m_speed_factor * 0.1f;

		// a little bit upwards first
		if( m_counter < 0.3f )
		{
			Move( 0.0f, -5.0f );
		}
		// if not below the screen fall
		else if( m_pos_y < game_res_h + m_col_rect.m_h )
		{
			Move( 0.0f, 20.0f );

			Add_Scale( -pFramerate->m_speed_factor * 0.01f );
		}
		// if below disable
		else
		{
			m_rot_z = 0.0f;
			Set_Scale( 1.0f );
			Set_Active( 0 );
		}
	}
}

void cFurball :: Set_Moving_State( Moving_state new_state )
{
	if( new_state == m_state )
	{
		return;
	}

	if( new_state == STA_STAY )
	{
		Set_Animation( 0 );
	}
	else if( new_state == STA_WALK )
	{
		m_counter_running = 0.0f;

		Set_Animation( 1 );
		Set_Animation_Image_Range( 0, 7 );
		if( m_color_type == COL_BLUE )
		{
			Set_Time_All( 70, 1 );
		}
		else
		{
			Set_Time_All( 80, 1 );
		}
		Reset_Animation();
	}
	else if( new_state == STA_RUN )
	{
		m_counter_hit = 0.0f;

		Set_Animation( 1 );
		Set_Animation_Image_Range( 0, 7 );
		Set_Time_All( 70, 1 );
		Reset_Animation();
	}

	m_state = new_state;

	if( m_state == STA_STAY )
	{
		Update_Velocity();
		Update_Rotation_Hor_velx();
	}
	else if( m_state == STA_WALK )
	{
		Update_Velocity();
		Update_Rotation_Hor_velx();
	}
	else if( m_state == STA_RUN )
	{
		Update_Velocity();
		Update_Rotation_Hor_velx();
	}
}

void cFurball :: Update( void )
{
	cEnemy::Update();

	if( !m_valid_update || !Is_In_Player_Range() )
	{
		return;
	}

	Update_Animation();

	if( m_state == STA_STAY )
	{
		m_counter_hit += pFramerate->m_speed_factor;

		// angry
		if( static_cast<int>(m_counter_hit) % 2 == 1 )
		{
			// slowly fade to the color
			cMovingSprite::Set_Color( Color( static_cast<Uint8>(255), 250 - static_cast<Uint8>( m_counter_hit * 1.5f ), 250 - static_cast<Uint8>( m_counter_hit * 4 ) ) );
		}
		// default
		else
		{
			cMovingSprite::Set_Color( white );
		}

		// rotate slowly
		Set_Rotation_Z( -m_counter_hit * 0.125f );

		// slow down
		if( !Is_Float_Equal( m_velx, 0.0f ) )
		{
			Add_Velocity( -m_velx * 0.25f, 0.0f );

			if( m_velx < 0.3f && m_velx > -0.3f )
			{
				m_velx = 0.0f;
			}
		}

		// finished hit animation
		if( m_counter_hit > 60.0f )
		{
			// run
			Set_Moving_State( STA_RUN );

			// jump a bit up
			m_vely = -5.0f;

			//pAudio->Play_Sound( "enemy/boss/furball/run.wav" );
		}
	}
	else if( m_state == STA_RUN )
	{
		m_counter_running += pFramerate->m_speed_factor;

		// rotate slowly back
		Set_Rotation_Z( -7.5f + m_counter_running * 0.0625f );

		// finished hit animation
		if( m_counter_running > 120.0f )
		{
			// walk
			Set_Moving_State( STA_WALK );
		}

		// running particles
		m_running_particle_counter += pFramerate->m_speed_factor * 1.5f;

		// create particles
		while( m_running_particle_counter > 1.0f )
		{
			cParticle_Emitter *anim = new cParticle_Emitter();

			anim->Set_Emitter_Rect( m_col_rect.m_x, m_col_rect.m_y + m_col_rect.m_h - 2.0f, m_col_rect.m_w );
			anim->Set_Pos_Z( m_pos_z - 0.000001f );

			float vel;

			if( m_velx > 0.0f )
			{
				vel = m_velx;
			}
			else
			{
				vel = -m_velx;
			}

			anim->Set_Image( pVideo->Get_Surface( "animation/particles/smoke_black.png" ) );
			anim->Set_Time_to_Live( 0.6f );
			anim->Set_Scale( 0.2f );
			anim->Set_Speed( vel * 0.08f, 0.1f + vel * 0.1f );

			if( m_direction == DIR_RIGHT )
			{
				anim->Set_Direction_Range( 180.0f, 90.0f );
			}
			else
			{
				anim->Set_Direction_Range( 270.0f, 90.0f );
			}

			// add animation
			pAnimation_Manager->Add( anim );

			m_running_particle_counter--;
		}
	}

	if( m_state != STA_STAY )
	{
		// if turn around image
		if( m_curr_img == 8 )
		{
			m_anim_counter += pFramerate->m_elapsed_ticks;

			// back to normal animation
			if( m_anim_counter >= 200 )
			{
				Reset_Animation();
				Set_Image_Num( m_anim_img_start );
				Set_Animation( 1 );
				Update_Rotation_Hor_velx();
			}
			// rotate the turn image
			else if( m_anim_counter >= 100 )
			{
				Update_Rotation_Hor_velx();
			}
		}
	}

	// gravity
	Update_Gravity();
}

void cFurball :: Generate_Smoke( unsigned int amount /* = 1 */, float particle_scale /* = 0.4f */ ) const
{
	// animation
	cParticle_Emitter *anim = new cParticle_Emitter();
	anim->Set_Pos( m_pos_x + ( m_col_rect.m_w * 0.5f ), m_pos_y + ( m_col_rect.m_h * 0.5f ) );
	anim->Set_Image( pVideo->Get_Surface( "animation/particles/smoke_grey_big.png" ) );
	anim->Set_Quota( amount );
	anim->Set_Pos_Z( m_pos_z + 0.000001f );
	anim->Set_Const_Rotation_Z( -6.0f, 12.0f );
	anim->Set_Time_to_Live( 1.5f );
	anim->Set_Speed( 0.4f, 0.9f );
	anim->Set_Scale( particle_scale, 0.2f );
	anim->Set_Color( black, Color( static_cast<Uint8>(87), 60, 40, 0 ) );
	pAnimation_Manager->Add( anim );
}

void cFurball :: Update_Velocity( void )
{
	if( m_direction == DIR_RIGHT )
	{
		if( m_state == STA_WALK )
		{
			m_velx = m_speed_walk;
		}
		else if( m_state == STA_RUN )
		{
			m_velx = m_speed_walk * 1.5f;
		}
		// stay does slow down automatically
	}
	else
	{
		if( m_state == STA_WALK )
		{
			m_velx = -m_speed_walk;
		}
		else if( m_state == STA_RUN )
		{
			m_velx = -m_speed_walk * 1.5f;
		}
		// stay does slow down automatically
	}
}

bool cFurball :: Is_Update_Valid( void )
{
	if( m_dead || m_freeze_counter )
	{
		return 0;
	}

	return 1;
}

Col_Valid_Type cFurball :: Validate_Collision( cSprite *obj )
{
	// basic validation checking
	Col_Valid_Type basic_valid = Validate_Collision_Ghost( obj );

	// found valid collision
	if( basic_valid != COL_VTYPE_NOT_POSSIBLE )
	{
		return basic_valid;
	}

	if( obj->m_massive_type == MASS_MASSIVE )
	{
		switch( obj->m_type )
		{
			case TYPE_PLAYER:
			{
				if( m_type == TYPE_FURBALL_BOSS )
				{
					// player is invincible
					if( pPlayer->invincible > 0.0f )
					{
						return COL_VTYPE_NOT_VALID;
					}

					return COL_VTYPE_BLOCKING;
				}

				break;
			}
			case TYPE_STATIC_ENEMY:
			{
				if( m_type == TYPE_FURBALL_BOSS )
				{
					return COL_VTYPE_NOT_VALID;
				}

				break;
			}
			case TYPE_SPIKEBALL:
			{
				if( m_type == TYPE_FURBALL_BOSS )
				{
					return COL_VTYPE_NOT_VALID;
				}

				break;
			}
			case TYPE_BALL:
			{
				// ignore balls
				if( m_type == TYPE_FURBALL_BOSS )
				{
					return COL_VTYPE_NOT_VALID;
				}

				break;
			}
			case TYPE_FLYON:
			{
				return COL_VTYPE_NOT_VALID;
			}
			case TYPE_ROKKO:
			{
				return COL_VTYPE_NOT_VALID;
			}
			case TYPE_GEE:
			{
				return COL_VTYPE_NOT_VALID;
			}
			default:
			{
				break;
			}
		}

		return COL_VTYPE_BLOCKING;
	}
	else if( obj->m_massive_type == MASS_HALFMASSIVE )
	{
		// if moving downwards and the object is on bottom
		if( m_vely >= 0.0f && Is_On_Top( obj ) )
		{
			return COL_VTYPE_BLOCKING;
		}
	}
	else if( obj->m_massive_type == MASS_PASSIVE )
	{
		switch( obj->m_type )
		{
			case TYPE_ENEMY_STOPPER:
			{
				return COL_VTYPE_BLOCKING;
			}
			default:
			{
				break;
			}
		}
	}

	return COL_VTYPE_NOT_VALID;
}

void cFurball :: Handle_Collision_Player( cObjectCollision *collision )
{
	// invalid
	if( collision->direction == DIR_UNDEFINED )
	{
		return;
	}

	if( collision->direction == DIR_TOP && pPlayer->m_state != STA_FLY )
	{
		if( m_type == TYPE_FURBALL_BOSS )
		{
			if( m_state == STA_STAY || m_state == STA_RUN )
			{
				pAudio->Play_Sound( "enemy/boss/furball/hit_failed.wav" );
			}
			else
			{
				pAudio->Play_Sound( "enemy/boss/furball/hit.wav" );
			}
		}
		else
		{
			pAudio->Play_Sound( m_kill_sound );
		}

		DownGrade();
		pPlayer->Action_Jump( 1 );

		if( m_dead )
		{
			pHud_Points->Add_Points( m_kill_points, pPlayer->m_pos_x, pPlayer->m_pos_y, "", static_cast<Uint8>(255), 1 );
			pPlayer->Add_Kill_Multiplier();
		}
	}
	else
	{
		pPlayer->DownGrade_Player();

		if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
		{
			Turn_Around( collision->direction );
		}
	}
}

void cFurball :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
	{
		Turn_Around( collision->direction );
	}

	Send_Collision( collision );
}

void cFurball :: Handle_Collision_Massive( cObjectCollision *collision )
{
	if( m_state == STA_OBJ_LINKED )
	{
		return;
	}

	if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
	{
		Turn_Around( collision->direction );
	}

	Send_Collision( collision );
}

void cFurball :: Editor_Activate( void )
{
	// get window manager
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// direction
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_furball_direction" ));
	Editor_Add( UTF8_("Direction"), UTF8_("Starting direction."), combobox, 100, 75 );

	combobox->addItem( new CEGUI::ListboxTextItem( "left" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "right" ) );

	combobox->setText( Get_Direction_Name( m_start_direction ) );
	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cFurball::Editor_Direction_Select, this ) );

	if( m_type == TYPE_FURBALL_BOSS )
	{
		// max downgrades
		CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_furball_max_downgrade_count" ));
		Editor_Add( UTF8_("Downgrades"), UTF8_("Downgrades until death"), editbox, 120 );

		editbox->setValidationString( "^[+]?\\d*$" );
		editbox->setText( int_to_string( m_max_downgrade_count ) );
		editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cFurball::Editor_Max_Downgrade_Count_Text_Changed, this ) );
	}

	// init
	Editor_Init();
}

bool cFurball :: Editor_Direction_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_Id( item->getText().c_str() ) );

	return 1;
}

bool cFurball :: Editor_Max_Downgrade_Count_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Downgrade_Count( string_to_int( str_text ) );

	return 1;
}

void cFurball :: Create_Name( void )
{
	m_name = "Furball ";
	m_name += _(Get_Color_Name( m_color_type ).c_str());
	m_name += " ";
	m_name += _(Get_Direction_Name( m_start_direction ).c_str());
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
