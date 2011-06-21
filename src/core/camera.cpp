/***************************************************************************
 * camera.cpp  -  class for handling screen camera movement
 *
 * copyright (C) 2006 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/camera.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../core/framerate.h"
#include "../input/mouse.h"
#include "../overworld/world_manager.h"
#include "../level/level.h"
#include "../overworld/overworld.h"
#include "../core/main.h"
#include "../audio/audio.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cCamera *** *** *** *** *** *** *** *** *** *** */

const GL_rect cCamera::default_limits = GL_rect( 0, 0, 20000, -4000 );

cCamera :: cCamera( void )
{
	x = 0.0f;
	y = 0.0f;

	x_offset = 0.0f;
	y_offset = 0.0f;
	hor_offset_speed = 0.3f;
	ver_offset_speed = 0.2f;

	fixed_hor_vel = 0.0f;

	// default camera limit
	Reset_Limits();
}

cCamera :: ~cCamera( void )
{

}

void cCamera :: Set_Pos( float nx, float ny )
{
	// level mode
	if( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD )
	{
		if( !editor_enabled )
		{
			// camera offset
			nx += x_offset;
			ny += y_offset;
			
			// camera limits
			Update_Limit( nx, ny );
		}
	}

	x = nx;
	y = ny;

	Update_Position();
}

void cCamera :: Set_Pos_X( float nx )
{
	// level mode
	if( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD )
	{
		if( !editor_enabled )
		{
			// camera offset
			nx += x_offset;
			
			// camera limits
			Update_Limit_X( nx );
		}
	}

	x = nx;

	Update_Position();
}

void cCamera :: Set_Pos_Y( float ny )
{
	// level mode
	if( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD )
	{
		if( !editor_enabled )
		{
			// camera offset
			ny += y_offset;

			// camera limits
			Update_Limit_Y( ny );
		}
	}

	y = ny;

	Update_Position();
}

void cCamera :: Move( const float move_x, const float move_y )
{
	if( ( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD ) && !editor_enabled )
	{
		Set_Pos( x + move_x - x_offset, y + move_y - y_offset );
	}
	else
	{
		Set_Pos( x + move_x, y + move_y );
	}
}

bool cCamera :: Move_to_Position_Gradually( const float px, const float py, const unsigned int frames /* = 200 */ )
{
	for( float i = 0.0f; i < frames; i += pFramerate->m_speed_factor )
	{
		if( Step_to_Position_Gradually( px, py ) == 0 )
		{
			return 0;
		}

		if( Game_Mode == MODE_LEVEL )
		{
			// keep global effect particles on screen
			pActive_Level->m_global_effect->Update_Particles();
		}

		// update audio
		pAudio->Update();
		// draw
		Draw_Game();

		pVideo->Render();
		pFramerate->Update();
	}

	return 1;
}

bool cCamera :: Step_to_Position_Gradually( const float px, const float py )
{
	// limit to nearest possible position
	float px_final = px + x_offset;
	float py_final = py + y_offset;
	Update_Limit( px_final, py_final );

	// check distance to new position
	float distance_x = px_final - x;
	float distance_y = py_final - y;
	// velocity
	float vel_x = ( distance_x * 0.04f ) * pFramerate->m_speed_factor;
	float vel_y = ( distance_y * 0.04f ) * pFramerate->m_speed_factor;
	// true if reached position
	bool reached_x = 0;
	bool reached_y = 0;

	if( distance_x > 2.0f )
	{
		if( vel_x < 2.0f )
		{
			vel_x = 2.0f;
		}
	}
	else if( distance_x < -2.0f )
	{
		if( vel_x > -2.0f )
		{
			vel_x = -2.0f;
		}
	}
	// reached destination position x
	else
	{
		reached_x = 1;
	}

	if( distance_y > 2.0f )
	{
		if( vel_y < 2.0f )
		{
			vel_y = 2.0f;
		}
	}
	else if( distance_y < -2.0f )
	{
		if( vel_y > -2.0f )
		{
			vel_y = -2.0f;
		}
	}
	// reached destination position y
	else
	{
		reached_y = 1;
	}

	// reached position
	if( reached_x && reached_y )
	{
		return 0;
	}
	// move
	else
	{
		Move( vel_x, vel_y );
	}

	// position not reached
	return 1;
}

void cCamera :: Center( const ObjectDirection direction /* = DIR_ALL */ )
{
	// Center camera on a not changing player position
	if( direction == DIR_VERTICAL )
	{
		Set_Pos_X( Get_Center_Pos_X() );
	}
	else if( direction == DIR_HORIZONTAL )
	{
		Set_Pos_Y( Get_Center_Pos_Y() );
	}
	else if( direction == DIR_ALL )
	{
		Set_Pos( Get_Center_Pos_X(), Get_Center_Pos_Y() );
	}
}

float cCamera :: Get_Center_Pos_X( void ) const
{
	return pActive_Player->m_col_rect.m_x + 5 - ( game_res_w * 0.5f );
}

float cCamera :: Get_Center_Pos_Y( void ) const
{
	return pActive_Player->m_col_rect.m_y + pActive_Player->m_col_rect.m_h - 5.0f - ( game_res_h * 0.5f );
}

void cCamera :: Reset_Limits( void )
{
	limit_rect = default_limits;
}

void cCamera :: Set_Limits( const GL_rect &rect )
{
	limit_rect = rect;

	// minimal size
	if( limit_rect.m_w < 800.0f )
	{
		limit_rect.m_w = 800.0f;
	}
	if( limit_rect.m_h > 0.0f )
	{
		limit_rect.m_h = 0.0f;
	}
}

void cCamera :: Set_Limit_X( const float val )
{
	limit_rect.m_x = val;
}

void cCamera :: Set_Limit_Y( const float val )
{
	limit_rect.m_y = val;
}

void cCamera :: Set_Limit_W( const float val )
{
	limit_rect.m_w = val;
}

void cCamera :: Set_Limit_H( const float val )
{
	limit_rect.m_h = val;
}

void cCamera :: Update_Limit( float &nx, float &ny ) const
{
	Update_Limit_X( nx );
	Update_Limit_Y( ny );
}

void cCamera :: Update_Limit_X( float &nx ) const
{
	// left
	if( nx < limit_rect.m_x )
	{
		nx = limit_rect.m_x;
	}
	// right
	else if( nx + game_res_w > limit_rect.m_x + limit_rect.m_w )
	{
		nx = limit_rect.m_x + limit_rect.m_w - game_res_w;
	}
}

void cCamera :: Update_Limit_Y( float &ny ) const
{
	// down
	if( ny > limit_rect.m_y )
	{
		ny = limit_rect.m_x;
	}
	// up
	else if( ny < limit_rect.m_y + limit_rect.m_h )
	{
		ny = limit_rect.m_y + limit_rect.m_h;
	}
}

void cCamera :: Update( void ) 
{
	// level
	if( Game_Mode == MODE_LEVEL )
	{
		// no leveleditor mode
		if( !editor_enabled )
		{
			// player is moving vertical
			if( pPlayer->m_vely != 0 && ver_offset_speed > 0.0f )
			{
				pPlayer->no_vely_counter = 0.0f;

				if( ( y_offset < 100.0f && pPlayer->m_vely > 0.0f ) || ( y_offset > -100.0f && pPlayer->m_vely < 0.0f ) )
				{
					y_offset += ( pPlayer->m_vely * ver_offset_speed ) * pFramerate->m_speed_factor;
				}
			}
			// slowly remove offset
			else 
			{
				pPlayer->no_vely_counter += pFramerate->m_speed_factor;

				if( pPlayer->no_vely_counter > 10.0f && ( y_offset > 20.0f || y_offset < -20.0f ) )
				{
					y_offset += -( y_offset / 30 ) * pFramerate->m_speed_factor;
				}
			}

			if( fixed_hor_vel )
			{
				// todo : remove hackiness
				x += fixed_hor_vel * pFramerate->m_speed_factor;
				// check limit
				Update_Limit_X( x );
				// center one side
				Center( DIR_HORIZONTAL );

				// scrolls to the right
				if( fixed_hor_vel > 0.0f )
				{
					// out of camera on the left side
					if( pPlayer->m_col_rect.m_x + pPlayer->m_col_rect.m_w < x )
					{
						pPlayer->DownGrade_Player( 1, 1 );
					}
				}
				// scrolls to the left
				else
				{
					// out of camera on the right side
					if( pPlayer->m_col_rect.m_x - game_res_w > x )
					{
						pPlayer->DownGrade_Player( 1, 1 );
					}
				}
			}
			else
			{
				// player is moving horizontal
				if( pPlayer->m_velx != 0 && hor_offset_speed > 0.0f )
				{
					pPlayer->no_velx_counter = 0.0f;

					if( ( x_offset < 200.0f && pPlayer->m_velx > 0.0f ) || ( x_offset > -200.0f && pPlayer->m_velx < 0.0f ) )
					{
						x_offset += ( pPlayer->m_velx * hor_offset_speed ) * pFramerate->m_speed_factor;
					}
				}
				// slowly remove offset
				else
				{
					pPlayer->no_velx_counter += pFramerate->m_speed_factor;

					if( pPlayer->no_velx_counter > 10.0f && ( x_offset > 40.0f || x_offset < -40.0f ) )
					{
						x_offset += -( x_offset / 50 ) * pFramerate->m_speed_factor;
					}
				}

				// set position
				Center();
			}
		}
	}
	// world
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// Left
		if( pActive_Player->m_pos_x - x < game_res_w * 0.4f )
		{
			Set_Pos_X( pActive_Player->m_pos_x - game_res_w * 0.4f );
		}
		// Right
		else if( pActive_Player->m_pos_x - x > game_res_w * 0.6f )
		{
			Set_Pos_X( pActive_Player->m_pos_x - game_res_w * 0.6f );
		}
		
		// Up
		if( pActive_Player->m_pos_y - y < game_res_h * 0.4f )
		{
			Set_Pos_Y( pActive_Player->m_pos_y - game_res_h * 0.4f );
		}
		// Down
		else if( pActive_Player->m_pos_y - y > game_res_h * 0.6f )
		{
			Set_Pos_Y( pActive_Player->m_pos_y - game_res_h * 0.6f );
		}
	}
}

void cCamera :: Update_Position( void ) const
{
	// mouse
	pMouseCursor->Update_Position();

	if( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD )
	{
		// update player
		pActive_Player->Update_Valid_Draw();
		// update sprite manager
		pActive_Sprite_Manager->Update_Items_Valid_Draw();

		// editor
		if( editor_enabled )
		{
			// update settings activated object position
			if( pMouseCursor->m_active_object )
			{
				pMouseCursor->m_active_object->Editor_Position_Update();
			}
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
