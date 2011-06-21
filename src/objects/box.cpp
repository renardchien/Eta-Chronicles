/***************************************************************************
 * box.cpp  -  class for the basic box handler
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
 
#include "../objects/box.h"
#include "../audio/audio.h"
#include "../core/camera.h"
#include "../core/framerate.h"
#include "../level/level.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../video/gl_surface.h"
#include "../user/savegame.h"
#include "../core/sprite_manager.h"
#include "../core/math/utilities.h"
#include "../core/i18n.h"
#include "../enemies/turtle.h"
#include "../enemies/bosses/turtle_boss.h"
// CEGUI
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** cBaseBox *** *** *** *** *** *** *** *** *** */

cBaseBox :: cBaseBox( float x /* = 0 */, float y /* = 0 */ )
: cAnimated_Sprite( x, y )
{
	m_type = TYPE_ACTIVESPRITE;
	m_sprite_array = ARRAY_ACTIVE;
	m_massive_type = MASS_MASSIVE;
	m_can_be_ground = 1;
	Set_Scale_Directions( 1, 1, 1, 1 );

	box_type = TYPE_UNDEFINED;
	item_image = NULL;
	m_pos_z = 0.055f;

	move_col_dir = DIR_UNDEFINED;
	move_counter = 0.0f;
	move_back = 0;
	// default = usable once
	useable_count = 1;
	start_useable_count = 1;

	box_invisible = BOX_VISIBLE;

	particle_counter_active = 0.0f;
}

cBaseBox :: ~cBaseBox( void )
{
	//
}

void cBaseBox :: Create_From_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	if( box_type != TYPE_SPINBOX && box_type != TYPE_TEXT_BOX )
	{
		// animation
		Set_Animation_Type( attributes.getValueAsString( "animation", anim_type ).c_str() );
	}
	// invisible
	Set_Invisible( static_cast<Box_Invisible_Type>(attributes.getValueAsInteger( "invisible" )) );
	// useable count
	Set_Useable_Count( attributes.getValueAsInteger( "useable_count", start_useable_count ), 1 );
}

void cBaseBox :: Save_To_Stream( ofstream &file )
{
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(m_start_pos_x) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(m_start_pos_y) << "\" />" << std::endl;
	// type
	if( box_type == TYPE_SPINBOX )
	{
		file << "\t\t<Property name=\"type\" value=\"spin\" />" << std::endl;
	}
	else if( box_type == TYPE_TEXT_BOX )
	{
		file << "\t\t<Property name=\"type\" value=\"text\" />" << std::endl;
	}
	else
	{
		file << "\t\t<Property name=\"type\" value=\"bonus\" />" << std::endl;
		// animation type
		file << "\t\t<Property name=\"animation\" value=\"" << anim_type << "\" />" << std::endl;
		// best possible item
		file << "\t\t<Property name=\"item\" value=\"" << box_type << "\" />" << std::endl;
	}
	// invisible
	file << "\t\t<Property name=\"invisible\" value=\"" << box_invisible << "\" />" << std::endl;
	// useable count
	file << "\t\t<Property name=\"useable_count\" value=\"" << start_useable_count << "\" />" << std::endl;
}

void cBaseBox :: Load_From_Savegame( cSave_Level_Object *save_object )
{
	// useable count
	int save_useable_count = string_to_int( save_object->Get_Value( "useable_count" ) );
	Set_Useable_Count( save_useable_count );
}

cSave_Level_Object *cBaseBox :: Save_To_Savegame( void )
{
	// only save if needed
	if( useable_count == start_useable_count )
	{
		return NULL;
	}

	cSave_Level_Object *save_object = new cSave_Level_Object();

	// default values
	save_object->m_type = m_type;
	save_object->m_properties.push_back( cSave_Level_Object_Property( "posx", int_to_string( static_cast<int>(m_start_pos_x) ) ) );
	save_object->m_properties.push_back( cSave_Level_Object_Property( "posy", int_to_string( static_cast<int>(m_start_pos_y) ) ) );

	// Useable Count
	save_object->m_properties.push_back( cSave_Level_Object_Property( "useable_count", int_to_string( useable_count ) ) );

	return save_object;
}

void cBaseBox :: Set_Animation_Type( const std::string &new_anim_type )
{
	// already set
	if( anim_type.compare( new_anim_type ) == 0 )
	{
		return;
	}

	Clear_Images();
	anim_type = new_anim_type;

	if( anim_type.compare( "Bonus" ) == 0 )
	{
		// disabled image
		Add_Image( pVideo->Get_Surface( "game/box/brown1_1.png" ) );
		// animation images
		Add_Image( pVideo->Get_Surface( "game/box/yellow/bonus/1.png" ) );
		Add_Image( pVideo->Get_Surface( "game/box/yellow/bonus/2.png" ) );
		Add_Image( pVideo->Get_Surface( "game/box/yellow/bonus/3.png" ) );
		Add_Image( pVideo->Get_Surface( "game/box/yellow/bonus/4.png" ) );
		Add_Image( pVideo->Get_Surface( "game/box/yellow/bonus/5.png" ) );
		Add_Image( pVideo->Get_Surface( "game/box/yellow/bonus/6.png" ) );

		Set_Animation( 1 );
		Set_Animation_Image_Range( 1, 6 );
		Set_Time_All( 90, 1 );
	}
	else if( anim_type.compare( "Default" ) == 0 )
	{
		// disabled image
		Add_Image( pVideo->Get_Surface( "game/box/brown1_1.png" ) );
		// default
		Add_Image( pVideo->Get_Surface( "game/box/yellow/default.png" ) );

		Set_Animation( 0 );
		Set_Animation_Image_Range( 1, 1 );
	}
	else if( anim_type.compare( "Power" ) == 0 )
	{
		// disabled image
		Add_Image( pVideo->Get_Surface( "game/box/brown1_1.png" ) );
		// default
		Add_Image( pVideo->Get_Surface( "game/box/yellow/power_1.png" ) );

		Set_Animation( 0 );
		Set_Animation_Image_Range( 1, 1 );
	}
	else if( anim_type.compare( "Spin" ) == 0 )
	{
		// disabled image
		Add_Image( pVideo->Get_Surface( "game/box/yellow/spin/disabled.png" ) );
		// animation images
		Add_Image( pVideo->Get_Surface( "game/box/yellow/default.png" ) );
		Add_Image( pVideo->Get_Surface( "game/box/yellow/spin/1.png" ) );
		Add_Image( pVideo->Get_Surface( "game/box/yellow/spin/2.png" ) );
		Add_Image( pVideo->Get_Surface( "game/box/yellow/spin/3.png" ) );
		Add_Image( pVideo->Get_Surface( "game/box/yellow/spin/4.png" ) );
		Add_Image( pVideo->Get_Surface( "game/box/yellow/spin/5.png" ) );

		Set_Animation( 0 );
		Set_Animation_Image_Range( 1, 6 );
		Set_Time_All( 80, 1 );
	}
	else
	{
		printf( "Warning : Unknown Box Animation Type %s\n", anim_type.c_str() );
		Set_Animation_Type( "Bonus" );
	}

	Reset_Animation();
	// set start image
	Set_Image_Num( m_anim_img_start, 1, 0 );
}

void cBaseBox :: Set_Useable_Count( int count, bool new_startcount /* = 0 */ )
{
	useable_count = count;

	if( new_startcount )
	{
		start_useable_count = useable_count;

		// unlimited
		if( start_useable_count < -1 )
		{
			start_useable_count = -1;
		}
	}
}

void cBaseBox :: Set_Invisible( Box_Invisible_Type type )
{
	// already set
	if( box_invisible == type )
	{
		return;
	}

	// remember old type
	Box_Invisible_Type type_old = box_invisible;
	// set new type
	box_invisible = type;

	// was invisible
	if( type_old == BOX_INVISIBLE_MASSIVE || type_old == BOX_INVISIBLE_SEMI_MASSIVE )
	{
		// was semi massive
		if( type_old == BOX_INVISIBLE_SEMI_MASSIVE )
		{
			m_massive_type = MASS_MASSIVE;
		}
	}
	// was ghost
	else if( type_old == BOX_GHOST )
	{
		Set_Color( 255, 255, 255, 255 );
		Set_Color_Combine( 0, 0, 0, 0 );
	}

	// got invisible
	if( type == BOX_INVISIBLE_MASSIVE || type == BOX_INVISIBLE_SEMI_MASSIVE )
	{
		// got semi massive
		if( type == BOX_INVISIBLE_SEMI_MASSIVE )
		{
			m_massive_type = MASS_PASSIVE;
		}
	}
	// got ghost
	else if( type == BOX_GHOST )
	{
		Set_Color( 192, 192, 255, 128 );
		Set_Color_Combine( 0.2f, 0.2f, 0.55f, GL_ADD );
	}

	// create name again
	Create_Name();
}

void cBaseBox :: Activate_Collision( ObjectDirection cdirection )
{
	// if already active ignore event
	if( move_col_dir != DIR_UNDEFINED )
	{
		return;
	}

	// not useable
	if( useable_count == 0 )
	{
		return;
	}

	// if invisible go visible
	if( box_invisible )
	{
		// get massive
		if( box_invisible == BOX_INVISIBLE_SEMI_MASSIVE )
		{
			m_massive_type = MASS_MASSIVE;
		}
	}

	// set scaling based on direction
	if( cdirection == DIR_UP )
	{
		Set_Scale_Directions( 1, 0, 1, 1 );
	}
	else if( cdirection == DIR_DOWN )
	{
		Set_Scale_Directions( 0, 1, 1, 1 );
	}
	else if( cdirection == DIR_LEFT )
	{
		Set_Scale_Directions( 1, 0, 1, 0 );
	}
	else if( cdirection == DIR_RIGHT )
	{
		Set_Scale_Directions( 1, 0, 0, 1 );
	}

	// set collision direction
	move_col_dir = cdirection;
	Update_Valid_Update();

	Check_Collision( Get_Opposite_Direction( move_col_dir ) );
	Activate();

	// set useable count
	if( useable_count > 0 || useable_count == -1 )
	{
		Set_Useable_Count( useable_count - 1 );
	}
}

void cBaseBox :: Update_Collision( void )
{
	// not moving
	if( move_col_dir == DIR_UNDEFINED )
	{
		return;
	}

	// invalid direction
	if( move_col_dir != DIR_UP && move_col_dir != DIR_DOWN && move_col_dir != DIR_RIGHT && move_col_dir != DIR_LEFT )
	{
		printf( "Warning : wrong box collision direction %d\n", move_col_dir );
		move_col_dir = DIR_UNDEFINED;
		Update_Valid_Update();
		return;
	}

	// speed mod
	float mod = pFramerate->m_speed_factor * 0.05f;
	// counter
	move_counter += pFramerate->m_speed_factor * 0.2f;

	// move into the given direction
	if( !move_back )
	{
		// scale
		Add_Scale( mod );

		// Particles
		Generate_Activation_Particles();

		// check if reached final position
		if( move_counter > 1.0f )
		{
			move_back = 1;
			move_counter = 0.0f;
		}
	}
	// move back to the original position
	else
	{
		// scale
		Add_Scale( -mod );

		// check if reached original position
		if(	move_counter > 1.0f )
		{
			move_col_dir = DIR_UNDEFINED;
			Update_Valid_Update();
			move_back = 0;
			move_counter = 0.0f;

			// reset rect
			m_col_pos = m_image->m_col_pos;
			// reset scale
			Set_Scale( 1 );
			Set_Scale_Directions( 1, 1, 1, 1 );
			// reset position
			Set_Pos( m_start_pos_x, m_start_pos_y );
		}
	}
}

void cBaseBox :: Check_Collision( ObjectDirection cdirection )
{
	// additional direction based check position
	float check_x = 0.0f;
	float check_y = 0.0f;

	// set the collision size based on the collision direction
	if( cdirection == DIR_BOTTOM )
	{
		check_y -= 10.0f;
	}
	else if( cdirection == DIR_TOP )
	{
		check_y += 10.0f;
	}
	else if( cdirection == DIR_LEFT ) 
	{
		check_x += 5.0f;
	}
	else if( cdirection == DIR_RIGHT )
	{
		check_x -= 5.0f;
	}

	// collision count
	cObjectCollisionType *col_list = Collision_Check_Relative( check_x, check_y, m_col_rect.m_w - ( check_x * 0.5f ), m_col_rect.m_h - ( check_y * 0.5f ) );

	// handle collisions
	for( cObjectCollision_List::iterator itr = col_list->objects.begin(), itr_end = col_list->objects.end(); itr != itr_end; ++itr )
	{
		cObjectCollision *col_obj = (*itr);

		// send box collision
		col_obj->obj->Handle_Collision_Box( Get_Opposite_Direction( col_obj->direction ), &m_col_rect );

		// Enemy collision
		if( col_obj->m_array == ARRAY_ENEMY )
		{
			Col_Enemy( col_obj->obj );
		}
	}

	delete col_list;
}

void cBaseBox :: Col_Enemy( cSprite *obj )
{
	// todo : handle this on enemy class Handle_Collision_Box()
	// only valid enemies
	if( obj->m_type == TYPE_FURBALL || obj->m_type == TYPE_TURTLE || obj->m_type == TYPE_KRUSH )
	{
		pAudio->Play_Sound( "death_box.wav" );
		static_cast<cMovingSprite *>(obj)->DownGrade( 1 );
	}
}

void cBaseBox :: Activate( void )
{
	// virtual
}

void cBaseBox :: Update( void )
{
	// animate only a visible box or an activated invisible box
	if( box_invisible == BOX_VISIBLE || ( box_invisible == BOX_GHOST && pPlayer->maryo_type == MARYO_GHOST ) || useable_count != start_useable_count )
	{
		// Spinbox animation handling
		if( ( m_type == TYPE_SPINBOX || useable_count != 0 ) && m_anim_enabled )
		{
			// Set_Image in Update_Animation overwrites col_pos
			GL_point col_pos_temp = m_col_pos;

			Update_Animation();

			// save col_pos
			m_col_pos = col_pos_temp;
		}
	}

	Update_Collision();
}

void cBaseBox :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( !m_valid_draw )
	{
		return;
	}

	// editor disabled
	if( !editor_level_enabled )
	{
		// visible box or activated invisible box
		if( box_invisible == BOX_VISIBLE || ( box_invisible == BOX_GHOST && pPlayer->maryo_type == MARYO_GHOST ) || useable_count != start_useable_count )
		{
			cAnimated_Sprite::Draw( request );
		}
	}
	// editor enabled
	else
	{
		// draw invisible box only in editor mode
		if( box_invisible )
		{
			Color color;

			// default invisible
			if( box_invisible == BOX_INVISIBLE_MASSIVE )
			{
				color = Color( static_cast<Uint8>(240), 0, 30, 128 );
			}
			// ghost
			else if( box_invisible == BOX_GHOST )
			{
				color = Color( static_cast<Uint8>(20), 20, 150, 128 );
			}
			// invisible semi massive
			else if( box_invisible == BOX_INVISIBLE_SEMI_MASSIVE )
			{
				color = Color( static_cast<Uint8>(180), 0, 10, 128 );
			}

			pVideo->Draw_Rect( m_start_pos_x - pActive_Camera->x, m_start_pos_y - pActive_Camera->y, m_rect.m_w, m_rect.m_h, m_pos_z, &color );
		}
		// visible box
		else
		{
			cAnimated_Sprite::Draw( request );
		}

		// draw item image
		if( item_image )
		{
			// auto position
			item_image->Blit( m_start_pos_x - ( ( item_image->m_w - m_rect.m_w ) / 2 ) - pActive_Camera->x, m_start_pos_y - ( ( item_image->m_h - m_rect.m_h ) / 2 ) - pActive_Camera->y, m_pos_z + 0.000003f );
		}
	}
}

void cBaseBox :: Generate_Activation_Particles( void )
{
	// no default/unimportant boxes
	if( m_type == TYPE_SPINBOX || m_type == TYPE_TEXT_BOX || box_type == TYPE_GOLDPIECE )
	{
		return;
	}

	particle_counter_active += pFramerate->m_speed_factor;

	while( particle_counter_active > 0.0f )
	{
		cParticle_Emitter *anim = new cParticle_Emitter();
		anim->Set_Pos( m_pos_x + Get_Random_Float( 0.0f, m_rect.m_w ), m_pos_y );
		anim->Set_Pos_Z( m_pos_z - 0.000001f );
		anim->Set_Direction_Range( 180, 180 );
		anim->Set_Speed( 3.1f, 0.5f );
		anim->Set_Scale( 0.7f, 0.1f );

		Color color = white;

		if( box_type == TYPE_MUSHROOM_DEFAULT )
		{
			color = Color( static_cast<Uint8>( 100 + ( rand() % 100 ) ), 20 + ( rand() % 50 ), 10 + ( rand() % 30 ) );
		}
		else if( box_type == TYPE_FIREPLANT )
		{
			color = Color( static_cast<Uint8>( 110 + ( rand() % 150 ) ), rand() % 50, rand() % 30 );
		}
		else if( box_type == TYPE_MUSHROOM_BLUE )
		{
			color = Color( static_cast<Uint8>( rand() % 30 ), ( rand() % 30 ), 150 + ( rand() % 50 ) );
		}
		else if( box_type == TYPE_MUSHROOM_GHOST )
		{
			color = Color( static_cast<Uint8>( 100 + ( rand() % 50 ) ), 100 + ( rand() % 50 ), 100 + ( rand() % 50 ) );
		}
		else if( box_type == TYPE_MUSHROOM_LIVE_1 )
		{
			color = Color( static_cast<Uint8>( rand() % 30 ), 100 + ( rand() % 150 ), rand() % 30 );
		}
		else if( box_type == TYPE_JSTAR )
		{
			color = Color( static_cast<Uint8>( 110 + ( rand() % 150 ) ), 80 + ( rand() % 50 ), rand() % 20 );
		}
		else if( box_type == TYPE_MUSHROOM_POISON )
		{
			color = Color( static_cast<Uint8>( 50 + rand() % 50 ), 100 + ( rand() % 150 ), rand() % 10 );
		}

		anim->Set_Time_to_Live( 0.3f );
		anim->Set_Color( color );
		anim->Set_Blending( BLEND_ADD );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
		pAnimation_Manager->Add( anim );

		particle_counter_active--;
	}
}

bool cBaseBox :: Is_Update_Valid( void )
{
	// if not activateable and not animating
	if( !useable_count && move_col_dir == DIR_UNDEFINED )
	{
		return 0;
	}

	// if not active
	if( !m_active )
	{
		return 0;
	}

	return 1;
}

bool cBaseBox :: Is_Draw_Valid( void )
{
	// if editor not enabled
	if( !editor_enabled )
	{
		// if not active or no image is set
		if( !m_active || !m_image )
		{
			return 0;
		}

		// ghost
		if( box_invisible == BOX_GHOST )
		{
			// maryo is not ghost
			if( pPlayer->maryo_type != MARYO_GHOST )
			{
				return 0;
			}
		}
	}
	// editor enabled
	else
	{
		// if destroyed
		if( m_auto_destroy )
		{
			return 0;
		}

		// no image
		if( !m_start_image && !box_invisible )
		{
			return 0;
		}
	}

	// not visible on the screen
	if( !Is_Visible_On_Screen() )
	{
		return 0;
	}

	return 1;
}

Col_Valid_Type cBaseBox :: Validate_Collision( cSprite *obj )
{
	switch( obj->m_type )
	{
		case TYPE_MUSHROOM_LIVE_1:
		case TYPE_MUSHROOM_DEFAULT:
		case TYPE_MUSHROOM_POISON:
		case TYPE_MUSHROOM_BLUE:
		case TYPE_MUSHROOM_GHOST:
		case TYPE_FGOLDPIECE:
		{
			return COL_VTYPE_BLOCKING;
		}
		default:
		{
			break;
		}
	}

	if( obj->m_massive_type == MASS_MASSIVE )
	{
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

	return COL_VTYPE_NOT_VALID;
}

void cBaseBox :: Handle_Collision_Player( cObjectCollision *collision )
{
	// if player jumps from below or flies against it
	if( collision->direction == DIR_BOTTOM && pPlayer->m_state != STA_FLY )
	{
		if( useable_count != 0 )
		{
			Activate_Collision( Get_Opposite_Direction( collision->direction ) );
		}
		else
		{
			if( Is_Visible_On_Screen() )
			{
				pAudio->Play_Sound( "wall_hit.wav", RID_MARYO_WALL_HIT );
			}
		}
	}
}

void cBaseBox :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	cEnemy *enemy = static_cast<cEnemy *>(pActive_Sprite_Manager->Get_Pointer( collision->number ));

	// if turtle
	if( enemy->m_type == TYPE_TURTLE )
	{
		cTurtle *turtle = static_cast<cTurtle *>(enemy);

		// if not shell
		if( turtle->m_turtle_state != TURTLE_SHELL_RUN && turtle->m_turtle_state != TURTLE_SHELL_STAND )
		{
			return;
		}
	}
	// if turtle boss
	else if( enemy->m_type == TYPE_TURTLE_BOSS )
	{
		cTurtleBoss *turtleboss = static_cast<cTurtleBoss *>(enemy);

		// if not shell
		if( turtleboss->m_turtle_state != TURTLEBOSS_SHELL_RUN && turtleboss->m_turtle_state != TURTLEBOSS_SHELL_STAND )
		{
			return;
		}
	}
	// not a valid enemy
	else
	{
		return;
	}

	if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT || collision->direction == DIR_BOTTOM )
	{
		if( useable_count != 0 && m_type != TYPE_TEXT_BOX )
		{
			Activate_Collision( Get_Opposite_Direction( collision->direction ) );
		}
		else
		{
			if( Is_Visible_On_Screen() )
			{
				pAudio->Play_Sound( "wall_hit.wav" );
			}
		}
	}
}


void cBaseBox :: Editor_Activate( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	if( m_type != TYPE_TEXT_BOX )
	{
		// useable count
		CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_basebox_useable_count" ));
		Editor_Add( UTF8_("Useable Count"), UTF8_("Useable Count"), editbox, 80 );

		editbox->setText( int_to_string( start_useable_count ) );
		editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cBaseBox::Editor_Useable_Count_Text_Changed, this ) );
	}

	// Invisible
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_basebox_invisible" ));
	Editor_Add( UTF8_("Invisible"), UTF8_("Massive is invisible until activated.\nGhost is only visible and activateable if in ghost mode.\nSemi Massive is like Massive but only touchable in the activation direction."), combobox, 120, 100 );

	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Disabled") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Massive") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Ghost") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Semi Massive") ) );

	if( box_invisible == BOX_INVISIBLE_MASSIVE )
	{
		combobox->setText( UTF8_("Massive") );
	}
	else if( box_invisible == BOX_GHOST )
	{
		combobox->setText( UTF8_("Ghost") );
	}
	else if( box_invisible == BOX_INVISIBLE_SEMI_MASSIVE )
	{
		combobox->setText( UTF8_("Semi Massive") );
	}
	else
	{
		combobox->setText( UTF8_("Disabled") );
	}

	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cBaseBox::Editor_Invisible_Select, this ) );

	if( m_type == TYPE_SPINBOX )
	{
		// init
		Editor_Init();
	}
}

bool cBaseBox :: Editor_Useable_Count_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Useable_Count( string_to_int( str_text ), 1 );

	return 1;
}

bool cBaseBox :: Editor_Invisible_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	if( item->getText().compare( UTF8_("Massive") ) == 0 )
	{
		Set_Invisible( BOX_INVISIBLE_MASSIVE );
	}
	else if( item->getText().compare( UTF8_("Ghost") ) == 0 )
	{
		Set_Invisible( BOX_GHOST );
	}
	else if( item->getText().compare( UTF8_("Semi Massive") ) == 0 )
	{
		Set_Invisible( BOX_INVISIBLE_SEMI_MASSIVE );
	}
	else
	{
		Set_Invisible( BOX_VISIBLE );
	}

	return 1;
}

void cBaseBox :: Create_Name( void )
{
	if( box_type == TYPE_UNDEFINED )
	{
		m_name = _("Box Empty");
	}
	else if( box_type == TYPE_POWERUP )
	{
		m_name = _("Box Random");
	}
	else if( box_type == TYPE_SPINBOX )
	{
		m_name = _("Spinbox");
	}
	else if( box_type == TYPE_TEXT_BOX )
	{
		m_name = _("Textbox");
	}
	else if( box_type == TYPE_MUSHROOM_DEFAULT )
	{
		m_name = _("Box Mushroom");
	}
	else if( box_type == TYPE_FIREPLANT )
	{
		m_name = _("Box Mushroom - Fireplant");
	}
	else if( box_type == TYPE_MUSHROOM_BLUE )
	{
		m_name = _("Box Mushroom - Blue Mushroom");
	}
	else if( box_type == TYPE_MUSHROOM_GHOST )
	{
		m_name = _("Box Mushroom - Ghost Mushroom");
	}
	else if( box_type == TYPE_MUSHROOM_LIVE_1 )
	{
		m_name = _("Box 1-UP");
	}
	else if( box_type == TYPE_JSTAR )
	{
		m_name = _("Box Star");
	}
	else if( box_type == TYPE_GOLDPIECE )
	{
		m_name = _("Box Goldpiece");
	}
	else if( box_type == TYPE_MUSHROOM_POISON )
	{
		m_name = _("Box Mushroom Poison");
	}
	else
	{
		m_name = _("Box Unknown Item Type");
	}

	if( box_invisible == BOX_INVISIBLE_MASSIVE )
	{
		m_name.insert( 0, _("Invisible ") );
	}
	else if( box_invisible == BOX_GHOST )
	{
		m_name.insert( 0, _("Ghost ") );
	}
	else if( box_invisible == BOX_INVISIBLE_SEMI_MASSIVE )
	{
		m_name.insert( 0, _("Invisible Semi Massive ") );
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
