/***************************************************************************
 * bonusbox.cpp  -  class for bonusbox
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

#include "../objects/bonusbox.h"
#include "../objects/star.h"
#include "../gui/hud.h"
#include "../player/player.h"
#include "../audio/audio.h"
#include "../core/framerate.h"
#include "../level/level.h"
#include "../level/level_editor.h"
#include "../core/game_core.h"
#include "../objects/goldpiece.h"
#include "../core/sprite_manager.h"
#include "../core/i18n.h"
// CEGUI
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** cBonusBox *** *** *** *** *** *** *** *** *** */

cBonusBox :: cBonusBox( float x, float y )
: cBaseBox( x, y )
{
	cBonusBox::Init();
}

cBonusBox :: cBonusBox( CEGUI::XMLAttributes &attributes )
: cBaseBox()
{
	cBonusBox::Init();
	cBonusBox::Create_From_Stream( attributes );
}

cBonusBox :: ~cBonusBox( void )
{
	for( MovingSpriteList::iterator itr = active_items.begin(), itr_end = active_items.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	active_items.clear();
}

void cBonusBox :: Init( void )
{
	m_type = TYPE_BONUSBOX;
	force_best_item = 0;
	m_player_range = 5000;
	m_can_be_on_ground = 0;

	Set_Animation_Type( "Bonus" );
	gold_color = COL_DEFAULT;
	Set_Goldcolor( COL_YELLOW );

	Create_Name();
}

cBonusBox *cBonusBox :: Copy( void )
{
	cBonusBox *bonusbox = new cBonusBox( m_start_pos_x, m_start_pos_y );
	bonusbox->Set_Animation_Type( anim_type );
	bonusbox->Set_Bonus_Type( box_type );
	bonusbox->Set_Invisible( box_invisible );
	bonusbox->Set_Force_Best_Item( force_best_item );
	bonusbox->Set_Goldcolor( gold_color );
	bonusbox->Set_Useable_Count( start_useable_count, 1 );

	return bonusbox;
}

void cBonusBox :: Create_From_Stream( CEGUI::XMLAttributes &attributes )
{
	cBaseBox::Create_From_Stream( attributes );

	// item
	Set_Bonus_Type( static_cast<SpriteType>(attributes.getValueAsInteger( "item" )) );
	// force best possible item
	Set_Force_Best_Item( attributes.getValueAsBool( "force_best_item" ) );
	// gold color
	if( box_type == TYPE_GOLDPIECE )
	{
		Set_Goldcolor( Get_Color_Id( attributes.getValueAsString( "gold_color", Get_Color_Name( gold_color ) ).c_str() ) );
	}
}

void cBonusBox :: Save_To_Stream( ofstream &file )
{
	// begin box
	file << "\t<box>" << std::endl;

	cBaseBox::Save_To_Stream( file );

	// force best possible item
	file << "\t\t<Property name=\"force_best_item\" value=\"" << force_best_item << "\" />" << std::endl;
	// gold color
	if( box_type == TYPE_GOLDPIECE )
	{
		file << "\t\t<Property name=\"gold_color\" value=\"" << Get_Color_Name( gold_color ) << "\" />" << std::endl;
	}

	// end box
	file << "\t</box>" << std::endl;
}

void cBonusBox :: Set_Useable_Count( int count, bool new_startcount /* = 0 */ )
{
	cBaseBox::Set_Useable_Count( count, new_startcount );

	// disable
	if( !useable_count )
	{
		Reset_Animation();
		Set_Animation( 0 );
		Set_Image_Num( 0 );
	}
	// enable
	else
	{
		Set_Animation( 1 );
	}

	Update_Valid_Update();
}

void cBonusBox :: Set_Bonus_Type( SpriteType nbonus_type )
{
	// already set
	if( box_type == nbonus_type )
	{
		return;
	}

	box_type = nbonus_type;

	// set item image
	if( box_type == TYPE_UNDEFINED )
	{
		item_image = NULL;
	}
	else if( box_type == TYPE_POWERUP )
	{
		// force always best item
		force_best_item = 1;
		item_image = pVideo->Get_Surface( "game/editor/unknown.png" );
	}
	else if( box_type == TYPE_MUSHROOM_DEFAULT )
	{
		item_image = pVideo->Get_Surface( "game/items/mushroom_red.png" );
	}
	else if( box_type == TYPE_FIREPLANT )
	{
		item_image = pVideo->Get_Surface( "game/items/fireplant.png" );
	}
	else if( box_type == TYPE_MUSHROOM_BLUE )
	{
		item_image = pVideo->Get_Surface( "game/items/mushroom_blue.png" );
	}
	else if( box_type == TYPE_MUSHROOM_GHOST )
	{
		item_image = pVideo->Get_Surface( "game/items/mushroom_ghost.png" );
	}
	else if( box_type == TYPE_MUSHROOM_LIVE_1 )
	{
		item_image = pVideo->Get_Surface( "game/items/mushroom_green.png" );
	}
	else if( box_type == TYPE_JSTAR )
	{
		item_image = pVideo->Get_Surface( "game/items/star.png" );
	}
	else if( box_type == TYPE_GOLDPIECE )
	{
		if( gold_color == COL_RED )
		{
			item_image = pVideo->Get_Surface( "game/items/goldpiece/red/1.png" );
		}
		else
		{
			item_image = pVideo->Get_Surface( "game/items/goldpiece/yellow/1.png" );
		}
	}
	else if( box_type == TYPE_MUSHROOM_POISON )
	{
		item_image = pVideo->Get_Surface( "game/items/mushroom_poison.png" );
	}
	else
	{
		item_image = NULL;
		printf( "Error : Unknown BonusBox Item type : %d\n", box_type );
	}

	// recreate name
	Create_Name();
}

void cBonusBox :: Set_Force_Best_Item( bool enable )
{
	// can't be set if random 
	if( box_type == TYPE_POWERUP )
	{
		return;
	}

	force_best_item = enable;
}

void cBonusBox :: Set_Goldcolor( DefaultColor ncolor )
{
	// already set
	if( gold_color == ncolor )
	{
		return;
	}

	if( ncolor == COL_DEFAULT )
	{
		gold_color = COL_YELLOW;
	}

	gold_color = ncolor;

	if( box_type != TYPE_GOLDPIECE )
	{
		return;
	}

	if( gold_color == COL_YELLOW )
	{
		item_image = pVideo->Get_Surface( "game/items/goldpiece/yellow/1.png" );
		m_name = _("Box Goldpiece Yellow");
	}
	else if( gold_color == COL_RED )
	{
		item_image = pVideo->Get_Surface( "game/items/goldpiece/red/1.png" );
		m_name = _("Box Goldpiece Red");
	}
	else
	{
		printf( "Warning : Unknown Bonusbox Gold Color %d\n", gold_color );
	}
}

void cBonusBox :: Activate( void )
{
	// boxed item
	cMovingSprite *box_item = NULL;

	bool random = 0;

	// random
	if( box_type == TYPE_POWERUP )
	{
		int r = rand() % 5;

		if( r == 0 )
		{
			box_type = TYPE_MUSHROOM_DEFAULT;
		}
		else if( r == 1 )
		{
			box_type = TYPE_FIREPLANT;
		}
		else if( r == 2 )
		{
			box_type = TYPE_MUSHROOM_BLUE;
		}
		else if( r == 3 )
		{
			box_type = TYPE_MUSHROOM_GHOST;
		}
		else if( r == 4 )
		{
			box_type = TYPE_JSTAR;
		}

		random = 1;
	}

	Maryo_type current_maryo_type;

	// use original type
	if( pPlayer->maryo_type == MARYO_GHOST )
	{
		current_maryo_type = pPlayer->maryo_type_temp_power;
	}
	// already using original type
	else
	{
		current_maryo_type = pPlayer->maryo_type;
	}

	// no item
	if( box_type == TYPE_UNDEFINED )
	{
		pAudio->Play_Sound( "item/empty_box.wav" );
	}
	// check if lower item should be used if no force best item
	else if( !force_best_item && ( box_type == TYPE_FIREPLANT || box_type == TYPE_MUSHROOM_BLUE ) && 
		( current_maryo_type == MARYO_SMALL || ( ( current_maryo_type == MARYO_FIRE || current_maryo_type == MARYO_ICE ) && !pHud_Itembox->m_item_id ) ) )
	{
		pAudio->Play_Sound( "sprout_1.ogg" );

		cMushroom *mushroom = new cMushroom( m_start_pos_x - ( ( item_image->m_w - m_rect.m_w ) / 2 ), m_start_pos_y - ( ( item_image->m_h - m_rect.m_h ) / 2 ) );
		box_item = static_cast<cMovingSprite *>(mushroom);
	}
	else if( box_type == TYPE_FIREPLANT )
	{
		pAudio->Play_Sound( "sprout_1.ogg" );
		box_item = static_cast<cMovingSprite *>(new cFirePlant( m_start_pos_x - ( ( item_image->m_w - m_rect.m_w ) / 2 ), m_start_pos_y ));
	}
	else if( box_type == TYPE_MUSHROOM_DEFAULT || box_type == TYPE_MUSHROOM_LIVE_1 || box_type == TYPE_MUSHROOM_POISON || box_type == TYPE_MUSHROOM_BLUE || box_type == TYPE_MUSHROOM_GHOST )
	{
		pAudio->Play_Sound( "sprout_1.ogg" );

		cMushroom *mushroom = new cMushroom( m_start_pos_x - ( ( item_image->m_w - m_rect.m_w ) / 2 ), m_start_pos_y - ( ( item_image->m_h - m_rect.m_h ) / 2 ) );
		mushroom->Set_Type( box_type );
		box_item = static_cast<cMovingSprite *>(mushroom);
	}
	else if( box_type == TYPE_JSTAR )
	{
		pAudio->Play_Sound( "sprout_1.ogg" );
		cjStar *star = new cjStar( m_start_pos_x - ( ( item_image->m_w - m_rect.m_w ) / 2 ), m_start_pos_y );
		star->Set_On_Top( this );
		star->m_spawned = 1;
		// add to global objects
		pActive_Sprite_Manager->Add( star );
	}
	else if( box_type == TYPE_GOLDPIECE )
	{
		pAudio->Play_Sound( "item/goldpiece_1.ogg" );

		cJGoldpiece *goldpiece = new cJGoldpiece( m_start_pos_x - ( ( item_image->m_w - m_rect.m_w ) / 2 ), m_pos_y );
		goldpiece->Set_Gold_Color( gold_color );
		// add to global objects
		pActive_Sprite_Manager->Add( goldpiece );
	}
	else
	{
		printf( "Error : Unknown bonusbox item type %d\n", box_type );
		return;
	}

	// set back random state
	if( random )
	{
		box_type = TYPE_POWERUP;
	}

	if( box_item )
	{
		// set posz behind box
		box_item->m_pos_z = m_pos_z - 0.000001f;
		// set spawned
		box_item->m_spawned = 1;

		// add to item list
		active_items.push_back( box_item );
		Update_Valid_Update();
	}
}

void cBonusBox :: Update( void )
{
	if( !m_valid_update || !Is_In_Player_Range() )
	{
		return;
	}

	// update active items
	for( MovingSpriteList::iterator itr = active_items.begin(); itr != active_items.end(); )
	{
		cPowerUp *powerup = static_cast<cPowerUp *>(*itr);

		if( !powerup->m_active )
		{
			++itr;
			continue;
		}

		float diff = powerup->m_pos_y - (m_pos_y - powerup->m_col_rect.m_h - powerup->m_col_pos.m_y);

		// position over the box reached
		if( diff < 0 )
		{
			// clear animation counter
			powerup->counter = 0;

			// set powerup default posz
			powerup->m_pos_z = 0.05f;

			// set the item on top
			powerup->Set_On_Top( this, 0 );
			// add the item to the level objects
			pActive_Sprite_Manager->Add( powerup );

			// remove from array
			itr = active_items.erase( itr );
			Update_Valid_Update();
		}
		// move upwards
		else
		{
			powerup->counter += pFramerate->m_speed_factor;
			powerup->Move( 0, -1.0f - (diff * 0.1f) );

			++itr;
		}
	}

	cBaseBox::Update();
}

void cBonusBox :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( !m_valid_draw )
	{
		return;
	}

	for( MovingSpriteList::iterator itr = active_items.begin(), itr_end = active_items.end(); itr != itr_end; ++itr )
	{
		cMovingSprite *obj = (*itr);

		if( !obj->m_active )
		{
			continue;
		}
		
		obj->Draw();
	}

	cBaseBox::Draw( request );
}

bool cBonusBox :: Is_Update_Valid( void )
{
	// if item is in animation
	if( !active_items.empty() )
	{
		return 1;
	}

	return cBaseBox::Is_Update_Valid();
}

void cBonusBox :: Editor_Activate( void )
{
	// BaseBox Settings first
	cBaseBox::Editor_Activate();

	// get window manager
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// Animation
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_bonusbox_animation" ));
	Editor_Add( UTF8_("Animation"), UTF8_("Use the Power animation if the box has a good or needed item for this level"), combobox, 120, 100 );

	combobox->addItem( new CEGUI::ListboxTextItem( "Default" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "Bonus" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "Power" ) );

	combobox->setText( reinterpret_cast<const CEGUI::utf8*>(anim_type.c_str()) );

	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cBonusBox::Editor_Animation_Select, this ) );

	// Item
	combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_bonusbox_item" ));
	Editor_Add( UTF8_("Item"), UTF8_("The item that gets spawned"), combobox, 160, 140 );

	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Empty") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Random") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Mushroom") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Fireplant") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Mushroom Blue") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Mushroom Ghost") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Mushroom 1-UP") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Star") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Goldpiece") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Mushroom Poison") ) );

	if( box_type == TYPE_MUSHROOM_DEFAULT )
	{
		combobox->setText( UTF8_("Mushroom") );
	}
	else if( box_type == TYPE_FIREPLANT )
	{
		combobox->setText( UTF8_("Fireplant") );
	}
	else if( box_type == TYPE_MUSHROOM_BLUE )
	{
		combobox->setText( UTF8_("Mushroom Blue") );
	}
	else if( box_type == TYPE_MUSHROOM_GHOST )
	{
		combobox->setText( UTF8_("Mushroom Ghost") );
	}
	else if( box_type == TYPE_MUSHROOM_LIVE_1 )
	{
		combobox->setText( UTF8_("Mushroom 1-UP") );
	}
	else if( box_type == TYPE_JSTAR )
	{
		combobox->setText( UTF8_("Star") );
	}
	else if( box_type == TYPE_GOLDPIECE )
	{
		combobox->setText( UTF8_("Goldpiece") );
	}
	else if( box_type == TYPE_MUSHROOM_POISON )
	{
		combobox->setText( UTF8_("Mushroom Poison") );
	}
	else if( box_type == TYPE_POWERUP )
	{
		combobox->setText( UTF8_("Random") );
	}
	else
	{
		combobox->setText( UTF8_("Empty") );
	}

	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cBonusBox::Editor_Item_Select, this ) );

	// Force best item
	combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_bonusbox_force_best_item" ));
	Editor_Add( UTF8_("Force item"), UTF8_("Force best available item when activated"), combobox, 120, 80 );

	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Enabled") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Disabled") ) );

	if( force_best_item )
	{
		combobox->setText( UTF8_("Enabled") );
	}
	else
	{
		combobox->setText( UTF8_("Disabled") );
	}

	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cBonusBox::Editor_Force_best_item_Select, this ) );

	// gold color
	combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_bonusbox_gold_color" ));
	Editor_Add( UTF8_("Gold color"), UTF8_("Gold color if the item is a goldpiece"), combobox, 100, 80 );

	combobox->addItem( new CEGUI::ListboxTextItem( "yellow" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "red" ) );
	combobox->setText( Get_Color_Name( gold_color ) );

	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cBonusBox::Editor_Gold_Color_Select, this ) );

	// init
	Editor_Init();
}

void cBonusBox :: Editor_State_Update( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// Force best item
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.getWindow( "editor_bonusbox_force_best_item" ));

	if( box_type == TYPE_UNDEFINED || box_type == TYPE_POWERUP || box_type == TYPE_MUSHROOM_DEFAULT || box_type == TYPE_MUSHROOM_LIVE_1 || box_type == TYPE_MUSHROOM_POISON || 
		box_type == TYPE_MUSHROOM_GHOST || box_type == TYPE_JSTAR || box_type == TYPE_GOLDPIECE )
	{
		combobox->setEnabled( 0 );
	}
	else
	{
		combobox->setEnabled( 1 );
	}

	// gold color
	combobox = static_cast<CEGUI::Combobox *>(wmgr.getWindow( "editor_bonusbox_gold_color" ));

	if( box_type != TYPE_GOLDPIECE )
	{
		combobox->setEnabled( 0 );
	}
	else
	{
		combobox->setEnabled( 1 );
	}
}

bool cBonusBox :: Editor_Animation_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Animation_Type( item->getText().c_str() );

	return 1;
}

bool cBonusBox :: Editor_Item_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	if( item->getText().compare( UTF8_("Mushroom") ) == 0 )
	{
		Set_Bonus_Type( TYPE_MUSHROOM_DEFAULT );
	}
	else if( item->getText().compare( UTF8_("Fireplant") ) == 0 )
	{
		Set_Bonus_Type( TYPE_FIREPLANT );
	}
	else if( item->getText().compare( UTF8_("Mushroom Blue") ) == 0 )
	{
		Set_Bonus_Type( TYPE_MUSHROOM_BLUE );
	}
	else if( item->getText().compare( UTF8_("Mushroom Ghost") ) == 0 )
	{
		Set_Bonus_Type( TYPE_MUSHROOM_GHOST );
	}
	else if( item->getText().compare( UTF8_("Mushroom 1-UP") ) == 0 )
	{
		Set_Bonus_Type( TYPE_MUSHROOM_LIVE_1 );
	}
	else if( item->getText().compare( UTF8_("Star") ) == 0 )
	{
		Set_Bonus_Type( TYPE_JSTAR );
	}
	else if( item->getText().compare( UTF8_("Goldpiece") ) == 0 )
	{
		Set_Bonus_Type( TYPE_GOLDPIECE );
	}
	else if( item->getText().compare( UTF8_("Mushroom Poison") ) == 0 )
	{
		Set_Bonus_Type( TYPE_MUSHROOM_POISON );
	}
	else if( item->getText().compare( UTF8_("Random") ) == 0 )
	{
		Set_Bonus_Type( TYPE_POWERUP );
	}
	else
	{
		Set_Bonus_Type( TYPE_UNDEFINED );
	}

	Editor_State_Update();

	return 1;
}

bool cBonusBox :: Editor_Force_best_item_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	if( item->getText().compare( UTF8_("Enabled") ) == 0 )
	{
		Set_Force_Best_Item( 1 );
	}
	else
	{
		Set_Force_Best_Item( 0 );
	}

	return 1;
}

bool cBonusBox :: Editor_Gold_Color_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Goldcolor( Get_Color_Id( item->getText().c_str() ) );

	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
