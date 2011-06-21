/***************************************************************************
 * editor.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2006 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_EDITOR_H
#define SMC_EDITOR_H

#include "../core/globals.h"
#include "../objects/sprite.h"
#include "../gui/hud.h"
#include "../video/img_settings.h"
// CEGUI
#include "CEGUIXMLHandler.h"
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cEditor_Object_Settings_Item *** *** *** *** *** *** *** *** *** *** */

class cEditor_Object_Settings_Item
{
public:
	cEditor_Object_Settings_Item( void );
	~cEditor_Object_Settings_Item( void );

	// name
	CEGUI::Window *window_name;
	// settings
	CEGUI::Window *window_setting;
	// if set start new row
	bool advance_row;
};

/* *** *** *** *** *** *** *** *** cEditor_Item_Object *** *** *** *** *** *** *** *** *** */

class cEditor_Item_Object : public CEGUI::ListboxItem
{
public:
	cEditor_Item_Object( const std::string &text );
	virtual ~cEditor_Item_Object( void );

	// Initialize
	void Init( void );

	/*!
	\brief
		Return the rendered pixel size of this list box item.
	\return
		Size object describing the size of the list box item in pixels.
	*/
	virtual	CEGUI::Size getPixelSize( void ) const;
	// draw
	void draw( const CEGUI::Vector3 &position, float alpha, const CEGUI::Rect &clipper ) const;
	void draw( CEGUI::RenderCache &cache, const CEGUI::Rect &targetRect, float zBase, float alpha, const CEGUI::Rect *clipper) const;
	// draw image
	void Draw_Image( void );

	// Text
	CEGUI::ListboxTextItem *list_text;
	// sprite
	cSprite *sprite_obj;
	// preview image scale
	float preview_scale;
};

/* *** *** *** *** *** *** *** *** cEditor_Menu_Object *** *** *** *** *** *** *** *** *** */

class cEditor_Menu_Object : public CEGUI::ListboxTextItem
{
public:
	cEditor_Menu_Object( const std::string &text );
	virtual ~cEditor_Menu_Object( void );

	// Initialize
	void Init( void );

	// Name
	std::string name;
	// tags or function name if function
	std::string tags;

	// if type is a function
	bool bfunction;

	// if Object is an Header
	bool header;
};

/* *** *** *** *** *** *** *** cEditor *** *** *** *** *** *** *** *** *** *** */

class cEditor : public CEGUI::XMLHandler
{
public:
	cEditor( void );
	virtual ~cEditor( void );

	// Initialize Editor
	virtual void Init( void );
	// Unload Editor
	virtual void Unload( void );

	// Toggle
	void Toggle( void );
	// Enable
	virtual void Enable( void );
	/* Disable
	 * native_mode : if unset the current game mode isn't altered
 	*/
	virtual void Disable( bool native_mode = 1 );

	// Update Editor
	virtual void Update( void );
	// Draw the Editor Menus
	virtual void Draw( void );

	// Function : Process_Input
	// static input handler
	void Process_Input( void );
	// Handle Input event
	virtual bool Handle_Event( SDL_Event *ev );
	/* handle key down event
	 * returns true if processed
	*/
	virtual bool Key_Down( SDLKey key );
	/* handle mouse button down event
	 * returns true if processed
	*/
	virtual bool Mouse_Down( Uint8 button );
	/* handle mouse button up event
	 * returns true if processed
	*/
	virtual bool Mouse_Up( Uint8 button );

	// ##### Main Menu

	// Add Menu Entry
	void Add_Menu_Object( const std::string &name, std::string tags, CEGUI::colour normal_color = CEGUI::colour( 1, 1, 1 ) );
	// Set Active Menu Entry
	virtual void Activate_Menu_Item( cEditor_Menu_Object *entry );

	// ##### Item Menu
	// Load an defined Menu
	virtual bool Load_Item_Menu( std::string item_tag );
	// Unload the Menu
	void Unload_Item_Menu( void );
	/* Add an Object to the Item list
	 * if nName is set it will not use the object name
	 * if image is set the default object image is not used
	 */
	void Add_Item_Object( cSprite *sprite, std::string new_name = "", cGL_Surface *image = NULL );
	// Loads all Image Items
	void Load_Image_Items( std::string dir );
	// Active Item Entry
	virtual void Activate_Item( cEditor_Item_Object *entry );

	// #### Editor Functions
	// move the draw position of object to the front or back
	void Change_Draw_Position( cSprite *obj, bool move_back ) const;
	/* copy the given object(s) next to itself into the given direction
	 * if offset is given it will be used instead of the auto calculated direction size
	 * returns the new object(s)
	*/
	cSprite_List Copy_Direction( cSprite_List &objects, const ObjectDirection dir ) const;
	cSprite *Copy_Direction( cSprite *obj, const ObjectDirection dir, int offset = 0 ) const;

	// CEGUI events
	bool Editor_Mouse_Enter( const CEGUI::EventArgs &event ); // Mouse entered Window
	bool Menu_Select( const CEGUI::EventArgs &event ); // Menu selected item
	bool Item_Select( const CEGUI::EventArgs &event ); // Item selected item

	// Menu functions
	void Function_Exit( void );
	virtual bool Function_New( void ) { return 0; };
	virtual void Function_Load( void ) {};
	virtual void Function_Save( bool with_dialog = 0 ) {};
	virtual void Function_Save_as( void ) {};
	virtual void Function_Delete( void ) {};
	virtual void Function_Reload( void ) {};
	virtual void Function_Clear( void ) {};
	virtual void Function_Settings( void ) {};


	// true if editor is active
	bool enabled;

	// Editor filenames
	std::string menu_filename;
	std::string items_filename;

	// Required item tag
	std::string editor_item_tag;
	// editor Camera speed
	float camera_speed;

	// Timer until the Menu will be minimized
	float menu_timer;

	// Objects with tags
	typedef vector<cImage_settings_data *> TaggedItemImageSettingsList;
	TaggedItemImageSettingsList tagged_item_images;
	typedef vector<cSprite *> TaggedItemObjectsList;
	TaggedItemObjectsList tagged_item_objects;

	// CEGUI window
	CEGUI::Window *editor_window;
	CEGUI::Listbox *listbox_menu;
	CEGUI::Listbox *listbox_items;
	CEGUI::TabControl *tabcontrol_menu;

protected:
	// Check if the given tag is available in the string
	bool Is_Tag_Available( const std::string &str, const std::string &tag, unsigned int search_pos = 0 );

	// Draw Editor Help Box
	void Draw_Editor_Help( void );
	/* Add a text line to the Help Box
	 * spacing : can be used f.e. a header
	 * pos_x : the line start position x
	*/
	void Add_Help_Line( std::string key_text, std::string description = "", float spacing = 0, float pos_x = 60 );

	// if true show the editor help
	bool show_editor_help;
	// help sprites
	typedef vector<cHudSprite *> HudSpriteList;
	HudSpriteList help_sprites;
private:
	// XML element start
	virtual void elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes );
	// XML element end
	virtual void elementEnd( const CEGUI::String &element );

	void Handle_Item( const CEGUI::XMLAttributes &attributes );
	void Handle_Menu( const CEGUI::XMLAttributes &attributes );

	// XML element Item Tag list
	CEGUI::XMLAttributes xml_attributes;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
