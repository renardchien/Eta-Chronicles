/***************************************************************************
 * main.cpp  -  main routines and initialization
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

#include "../core/global_basic.h"
#include "../core/game_core.h"
#include "../core/main.h"
#include "../core/filesystem/resource_manager.h"
#include "../core/filesystem/filesystem.h"
#include "../level/level.h"
#include "../gui/menu.h"
#include "../core/framerate.h"
#include "../video/font.h"
#include "../user/preferences.h"
#include "../audio/sound_manager.h"
#include "../audio/audio.h"
#include "../level/level_editor.h"
#include "../overworld/world_editor.h"
#include "../input/joystick.h"
#include "../overworld/world_manager.h"
#include "../overworld/overworld.h"
#include "../input/mouse.h"
#include "../user/savegame.h"
#include "../input/keyboard.h"
#include "../video/renderer.h"
#include "../objects/level_exit.h"
#include "../core/i18n.h"
#include "../gui/menu_data.h"
#ifdef __APPLE__
// needed for datapath detection
#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h>
#endif

// SMC namespace is set later to exclude main() from it
using namespace SMC;

#if defined( __WIN32__ ) && defined( _DEBUG )
	#undef main
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main( int argc, char **argv )
{
// todo : remove this apple hack
#ifdef __APPLE__
	// dynamic datapath detection for OS X
	// change CWD to point inside bundle so it finds its data (if necessary)
	char path[1024];
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	assert(mainBundle);
	CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
	assert(mainBundleURL);
	CFStringRef cfStringRef = CFURLCopyFileSystemPath( mainBundleURL, kCFURLPOSIXPathStyle);
	assert(cfStringRef);
	CFStringGetCString(cfStringRef, path, 1024, kCFStringEncodingASCII);
	CFRelease(mainBundleURL);
	CFRelease(cfStringRef);

	std::string contents = std::string(path) + std::string("/Contents");
	std::string datapath;

	if( contents.find(".app") != std::string::npos )
	{
		// executable is inside an app bundle, use app bundle-relative paths
		datapath = contents + std::string("/Resources/data/");
	}
	else if( contents.find("/bin") != std::string::npos )
	{
		// executable is installed Unix-way
		datapath = contents.substr( 0, contents.find("/bin") ) + "/share/smc";
	}
	else
	{
		std::cerr << "Warning: Could not determine installation type\n";
	}

	if( !datapath.empty() )
	{
		std::cout << "setting CWD to " << datapath.c_str() << std::endl;
		if( chdir( datapath.c_str() ) != 0 )
		{
			std::cerr << "Warning: Failed changing CWD\n";
		}
	}
#endif

	// convert arguments to a vector string
	vector<std::string> arguments( argv, argv + argc );

	if( argc >= 2 )
	{
		for( unsigned int i = 1; i < arguments.size(); i++ )
		{
			// help
			if( arguments[i] == "--help" || arguments[i] == "-h" )
			{
				printf( "Usage: %s [OPTIONS]\n", arguments[0].c_str() );
				printf( "Where OPTIONS is one of the following:\n" );
				printf( "-h, --help\tDisplay this message\n" );
				printf( "-v, --version\tShow the version of %s\n", CAPTION );
				printf( "-d, --debug\tEnable debug modes with the options : game performance\n" );
				printf( "-l, --level\tLoad the given level\n" );
				printf( "-w, --world\tLoad the given world\n" );
				return EXIT_SUCCESS;
			}
			// version
			else if( arguments[i] == "--version" || arguments[i] == "-v" )
			{
				printf( "%s %s\n", CAPTION, string_trim_from_end( float_to_string( smc_version, 6 ), '0' ).c_str() );
				return EXIT_SUCCESS;
			}
			// debug
			else if( arguments[i] == "--debug" || arguments[i] == "-d" )
			{
				// no value
				if( i + 1 >= arguments.size() )
				{
					printf( "%s requires a value\n", arguments[i].c_str() );
					return EXIT_FAILURE;
				}
				// with value
				else
				{
					for( unsigned int option = i + i; i < arguments.size(); i++ )
					{
						std::string option_str = arguments[option];

						if( option_str == "game" )
						{
							game_debug = 1;
						}
						else if( option_str == "performance" )
						{
							game_debug_performance = 1;
						}
						else
						{
							printf( "Unknown debug option %s\n", option_str.c_str() );
							return EXIT_FAILURE;
						}
					}
				}
			}
			// level loading is handled later
			else if( arguments[i] == "--level" || arguments[i] == "-l" )
			{
				// skip
			}
			// world loading is handled later
			else if( arguments[1] == "--world" || arguments[1] == "-w" )
			{
				// skip
			}
			// unknown argument
			else if( arguments[i].substr( 0, 1 ) == "-" )
			{
				printf( "Unknown argument %s\nUse -h to list all possible arguments\n", arguments[i].c_str() );
				return EXIT_FAILURE;
			}
		}
	}

	try
	{
		// initialize everything
		Init_Game();
	}
	catch( const std::exception &e )
	{
		printf( "Initialization: Exception raised: %s\n", e.what() );
		return EXIT_FAILURE;
	}

	// command line level loading
	if( argc > 2 && ( arguments[1] == "--level" || arguments[1] == "-l" ) && !arguments[2].empty() )
	{
		// set start menu active
		pMenuCore->Load( MENU_START );
		// enter level
		Game_Action = GA_ENTER_LEVEL;
		Game_Action_Data.add( "level", arguments[2] );
	}
	// command line world loading
	else if( argc > 2 && ( arguments[1] == "--world" || arguments[1] == "-w" ) && !arguments[2].empty() )
	{
		// set start menu active
		pMenuCore->Load( MENU_START );
		// enter level
		Game_Action = GA_ENTER_WORLD;
		Game_Action_Data.add( "world", arguments[2] );
	}
	// enter main menu
	else
	{
		Draw_Effect_Out( EFFECT_OUT_BLACK, 1.5f );
		pMenuCore->Load();
		Draw_Effect_In( EFFECT_IN_BLACK, 1.5f );
	}

	// game loop
	while( !game_exit )
	{
		// update
		Update_Game();
		// draw
		Draw_Game();

		// render
		pVideo->Render();

		// update speedfactor
		pFramerate->Update();
	}

	// exit
	Exit_Game();
	// no errors
	return EXIT_SUCCESS;
}

// namespace is set here to exclude main() from it
namespace SMC
{

void Init_Game( void )
{
	// init random number generator
	srand( static_cast<unsigned int>(time( NULL )) );

	// Init Stage 1 - core classes
	pResource_Manager = new cResource_Manager();
	pVideo = new cVideo();
	pAudio = new cAudio();
	pFont = new cFont_Manager();
	pFramerate = new cFramerate();
	pRenderer = new cRenderQueue( 200 );
	pRenderer_GUI = new cRenderQueue( 5 );
	pPreferences = new cPreferences();
	pImage_Manager = new cImage_Manager();
	pSound_Manager = new cSound_Manager();
	pSettingsParser = new cImage_settings();

	// Init Stage 2 - set preferences and init audio and the video screen
	/* Set default user directory
	 * can get overridden later from the preferences
	*/
	pResource_Manager->Set_User_Directory( Get_User_Directory() );
	/* Initialize the fake CEGUI renderer and system for the pPreferences XMLParser,
	 * because CEGUI creates the system normally with the OpenGL-Renderer and OpenGL calls may 
	 * only be made with a valid OpenGL-context, which we would get only by setting 
	 * the videomode first. That would mean we need to init the videomode twice.
	*/
	pVideo->Init_CEGUI_Fake();
	// load user data
	pPreferences->Load();
	// set game language
	I18N_Set_Language( pPreferences->m_language );
	// init translation support
	I18N_Init();
	// delete CEGUI System fake
	pVideo->Delete_CEGUI_Fake();

	// init user dir directory
	pResource_Manager->Init_User_Directory();
	// video init
	pVideo->Init_SDL();
	pVideo->Init_Video();
	pVideo->Init_CEGUI();
	pVideo->Init_CEGUI_Data();
	pFont->Init();
	// audio init
	pAudio->Init();

	pPlayer = new cPlayer();
	// set the first active player available
	pActive_Player = pPlayer;
	pLevel_Manager = new cLevel_Manager();
	// set the first active sprite manager available
	pActive_Sprite_Manager = pActive_Level->m_sprite_manager;
	// set the first camera available
	pActive_Camera = pLevel_Manager->camera;

	// apply preferences
	pPreferences->Apply();

	// draw generic loading screen
	Loading_Screen_Init();
	// initialize image cache
	pVideo->Init_Image_Cache( 0, 1 );

	// Init Stage 3 - game classes
	pPlayer->Init();
	pLevel_Editor = new cEditor_Level();
	pWorld_Editor = new cEditor_World();
	pMouseCursor = new cMouseCursor();
	pKeyboard = new cKeyboard();
	pJoystick = new cJoystick();
	pLevel_Manager->Init();
	pOverworld_Player = new cOverworld_Player();
	pOverworld_Manager = new cOverworld_Manager();

	pHud_Manager = new cHud_Manager();
	pAnimation_Manager = new cAnimation_Manager();
	pMenuCore = new cMenuCore();

	pSavegame = new cSavegame();

	// cache
	Preload_Images( 1 );
	Preload_Sounds( 1 );
	Loading_Screen_Exit();
}

void Exit_Game( void )
{
	if( pPreferences )
	{
		pPreferences->Save();
	}

	pLevel_Manager->Unload();
	pMenuCore->handler->m_level->Unload();

	if( pAudio )
	{
		delete pAudio;
		pAudio = NULL;
	}

	if( pPlayer )
	{
		delete pPlayer;
		pPlayer = NULL;
	}

	if( pHud_Manager )
	{
		delete pHud_Manager;
		pHud_Manager = NULL;
	}

	if( pSound_Manager )
	{
		delete pSound_Manager;
		pSound_Manager = NULL;
	}

	if( pAnimation_Manager )
	{
		delete pAnimation_Manager;
		pAnimation_Manager = NULL;
	}

	if( pLevel_Editor )
	{
		delete pLevel_Editor;
		pLevel_Editor = NULL;
	}

	if( pWorld_Editor )
	{
		delete pWorld_Editor;
		pWorld_Editor = NULL;
	}

	if( pPreferences )
	{
		delete pPreferences;
		pPreferences = NULL;
	}

	if( pSavegame )
	{
		delete pSavegame;
		pSavegame = NULL;
	}

	if( pMouseCursor )
	{
		delete pMouseCursor;
		pMouseCursor = NULL;
	}

	if( pJoystick )
	{
		delete pJoystick;
		pJoystick = NULL;
	}

	if( pKeyboard )
	{
		delete pKeyboard;
		pKeyboard = NULL;
	}

	if( pOverworld_Manager )
	{
		delete pOverworld_Manager;
		pOverworld_Manager = NULL;
	}

	if( pOverworld_Player )
	{
		delete pOverworld_Player;
		pOverworld_Player = NULL;
	}

	if( pLevel_Manager )
	{
		delete pLevel_Manager;
		pLevel_Manager = NULL;
	}

	if( pMenuCore )
	{
		delete pMenuCore;
		pMenuCore = NULL;
	}

	if( pRenderer )
	{
		delete pRenderer;
		pRenderer = NULL;
	}

	if( pRenderer_GUI )
	{
		delete pRenderer_GUI;
		pRenderer_GUI = NULL;
	}

	if( pGuiSystem )
	{
		delete pGuiSystem;
		pGuiSystem = NULL;
	}

	if( pGuiRenderer )
	{
		delete pGuiRenderer;
		pGuiRenderer = NULL;
	}

	if( pVideo )
	{
		delete pVideo;
		pVideo = NULL;
	}

	if( pImage_Manager )
	{
		delete pImage_Manager;
		pImage_Manager = NULL;
	}

	if( pSettingsParser )
	{
		delete pSettingsParser;
		pSettingsParser = NULL;
	}

	if( pFont )
	{
		delete pFont;
		pFont = NULL;
	}

	if( pResource_Manager )
	{
		delete pResource_Manager;
		pResource_Manager = NULL;
	}

	char *last_sdl_error = SDL_GetError();
	if( strlen( last_sdl_error ) > 0 )
	{
		printf( "Last known SDL Error : %s\n", last_sdl_error );
	}

	SDL_Quit();
}

bool Handle_Input_Global( SDL_Event *ev )
{
	switch( ev->type )
	{
		case SDL_QUIT:
		{
			game_exit = 1;
			Clear_Input_Events();

			// handle on all handlers ?
			return 0;
		}
		case SDL_VIDEORESIZE:
		{
			pGuiRenderer->setDisplaySize( CEGUI::Size( static_cast<float>(ev->resize.w), static_cast<float>(ev->resize.h) ) );
			break;
		}
		case SDL_KEYDOWN:
		{
			if( pKeyboard->Key_Down( ev->key.keysym.sym ) )
			{
				return 1;
			}
			break;
		}
		case SDL_KEYUP:
		{
			if( pKeyboard->Key_Up( ev->key.keysym.sym ) )
			{
				return 1;
			}
			break;
		}
		case SDL_JOYBUTTONDOWN:
		{
			if( pJoystick->Handle_Button_Down_Event( ev ) )
			{
				return 1;
			}
			break;
		}
		case SDL_JOYBUTTONUP:
		{
			if( pJoystick->Handle_Button_Up_Event( ev ) )
			{
				return 1;
			}
			break;
		}
		case SDL_JOYHATMOTION:
		{
			pJoystick->Handle_Hat( ev );
			break;
		}
		case SDL_JOYAXISMOTION:
		{
			pJoystick->Handle_Motion( ev );
			break;
		}
		case SDL_ACTIVEEVENT:
		{
			// lost visibility
			if( ev->active.gain == 0 )
			{
				bool music_paused = 0;
				// pause music
				if( pAudio->Is_Music_Playing() )
				{
					pAudio->Pause_Music();
					music_paused = 1;
				}
				SDL_WaitEvent( NULL );
				// resume if music got paused
				if( music_paused )
				{
					pAudio->Resume_Music();
				}
				return 1;
			}
			break;
		}
		default: // other events
		{
			// mouse
			if( pMouseCursor->Handle_Event( ev ) )
			{
				return 1; 
			}

			// send events
			if( Game_Mode == MODE_LEVEL )
			{
				// editor events
				if( pLevel_Editor->enabled )
				{
					if( pLevel_Editor->Handle_Event( ev ) )
					{
						return 1;
					}
				}
			}
			else if( Game_Mode == MODE_OVERWORLD )
			{
				// editor events
				if( pWorld_Editor->enabled )
				{
					if( pWorld_Editor->Handle_Event( ev ) )
					{
						return 1;
					}
				}
			}
			else if( Game_Mode == MODE_MENU )
			{
				if( pMenuCore->Handle_Event( ev ) )
				{
					return 1;
				}
			}
			break;
		}
	}

	return 0;
}

void Update_Game( void )
{
	// do not update if exiting
	if( game_exit )
	{
		return;
	}

	// ## game events
	// level mode
	if( Game_Mode == MODE_LEVEL )
	{
		// if Game Action set
		while( Game_Action != GA_NONE )
		{
			// get current data
			GameAction Current_Game_Action = Game_Action;
			CEGUI::XMLAttributes Current_Game_Action_Data = Game_Action_Data;
			void *Current_Game_Action_ptr = Game_Action_ptr;
			// clear
			Game_Action = GA_NONE;
			Game_Action_Data = CEGUI::XMLAttributes();
			Game_Action_ptr = NULL;

			// handle player downgrade
			if( Current_Game_Action == GA_DOWNGRADE_PLAYER )
			{
				pPlayer->DownGrade_Player( 0, Current_Game_Action_Data.getValueAsBool( "force" ) );
			}
			// activate level exit
			else if( Current_Game_Action == GA_ACTIVATE_LEVEL_EXIT )
			{
				cLevel_Exit *level_exit = static_cast<cLevel_Exit *>(Current_Game_Action_ptr);
				level_exit->Activate();
			}
			// Enter Level
			else if( Current_Game_Action == GA_ENTER_LEVEL )
			{
				// fade out
				if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM )
				{
					Draw_Effect_Out( EFFECT_OUT_BLACK, 3 );
				}
				else
				{
					Draw_Effect_Out();
				}

				// change game mode
				Change_Game_Mode( MODE_LEVEL );
				std::string str_level = Current_Game_Action_Data.getValueAsString( "level" ).c_str();
				// load new level
				if( pActive_Level->Load( str_level ) )
				{
					pActive_Level->Enter();
				}
				// if failed to load exit level
				else
				{
					pPlayer->Goto_Next_Level();
					continue;
				}

				// load entry
				std::string str_entry = Current_Game_Action_Data.getValueAsString( "entry" ).c_str();
				cLevel_Entry *entry = pActive_Level->Get_Entry( str_entry );

				if( !entry && !str_entry.empty() )
				{
					printf( "Warning : Level entry %s not found\n", str_entry.c_str() );
				}

				// set camera position to the entry 
				if( entry )
				{
					// set position
					pPlayer->Set_Pos( entry->Get_Player_Pos_X(), entry->Get_Player_Pos_Y() );
					// center camera position
					pActive_Camera->Center();
					// set invisible for warp animation
					pPlayer->Set_Active( 0 );
				}

				// fade in
				if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM )
				{
					Draw_Effect_In( EFFECT_IN_BLACK, 3 );
				}
				else
				{
					Draw_Effect_In();
				}

				// activate entry
				if( entry )
				{
					pPlayer->Goto_Sub_Level( str_level, str_entry, CAMERA_MOVE_NONE );
				}
			}
			// Enter World
			else if( Current_Game_Action == GA_ENTER_WORLD )
			{
				// Random fade out effect
				Draw_Effect_Out();
				// delay unload level
				pActive_Level->Unload( 1 );
				// set active world
				if( Current_Game_Action_Data.exists( "world" ) )
				{
					pOverworld_Manager->Set_Active( Current_Game_Action_Data.getValueAsString( "world" ).c_str() );
				}
				// set player waypoint
				if( Current_Game_Action_Data.exists( "player_waypoint" ) )
				{
					// get world waypoint
					int waypoint_num = pActive_Overworld->Get_Waypoint_Num( Current_Game_Action_Data.getValueAsString( "player_waypoint" ).c_str() );

					// waypoint available
					if( waypoint_num >= 0 )
					{
						// set all levels accessible until the world link
						pActive_Overworld->Set_Progress( waypoint_num, 0 );
						pOverworld_Player->Set_Waypoint( waypoint_num );
					}
				}
				// enter world
				pActive_Overworld->Enter();
				// draw fade in
				Draw_Effect_In();
			}
			// Enter Menu
			else if( Current_Game_Action == GA_ENTER_MENU )
			{
				// check if level is active
				bool level_active = 0;

				// if level is loaded and active
				if( pActive_Level->Is_Loaded() && !pActive_Level->m_delayed_unload )
				{
					level_active = 1;
				}

				// set exit game mode
				GameMode exit_gamemode = MODE_NOTHING;

				if( level_active )
				{
					exit_gamemode = MODE_LEVEL;
				}
				else
				{
					// fade out
					Draw_Effect_Out( EFFECT_OUT_BLACK, 3 );
				}

				// custom level
				if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM && Current_Game_Action_Data.exists( "level" ) )
				{
					std::string lvl_name = Current_Game_Action_Data.getValueAsString( "level" ).c_str();

					pMenuCore->Load( MENU_START, exit_gamemode );

					// Get Tab Control
					CEGUI::TabControl *tabcontrol = static_cast<CEGUI::TabControl *>(CEGUI::WindowManager::getSingleton().getWindow( "tabcontrol_main" ));
					// Select Level Tab
					tabcontrol->setSelectedTab( "tab_level" );

					// Get Levels Listbox
					CEGUI::Listbox *listbox_levels = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_levels" ));
					// Get Item
					CEGUI::ListboxItem *list_item = listbox_levels->findItemWithText( lvl_name, NULL );
					// select level
					if( list_item )
					{
						listbox_levels->setItemSelectState( list_item, 1 );
						listbox_levels->ensureItemIsVisible( list_item );
					}
				}
				// default menu
				else
				{
					pMenuCore->Load( MENU_MAIN, exit_gamemode );
				}
				
				if( !level_active )
				{
					// fade in
					Draw_Effect_In( EFFECT_IN_BLACK, 3 );
				}
			}

			// Enter Settings
			else if( Current_Game_Action == GA_ENTER_LEVEL_SETTINGS )
			{
				// fade out
				Draw_Effect_Out( EFFECT_OUT_BLACK, 3 );
				// enter
				pLevel_Editor->pSettings->Enter();
				// fade in
				Draw_Effect_In( EFFECT_IN_BLACK, 3 );
			}
		}
	}
	// world mode
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// if Game Action set
		while( Game_Action != GA_NONE )
		{
			// get current data
			GameAction Current_Game_Action = Game_Action;
			CEGUI::XMLAttributes Current_Game_Action_Data = Game_Action_Data;
			void *Current_Game_Action_ptr = Game_Action_ptr;
			// clear
			Game_Action = GA_NONE;
			Game_Action_Data = CEGUI::XMLAttributes();
			Game_Action_ptr = NULL;

			// Enter Level
			if( Current_Game_Action == GA_ENTER_LEVEL )
			{
				// fade out
				Draw_Effect_Out( EFFECT_OUT_FIXED_COLORBOX );
				// change game mode
				Change_Game_Mode( MODE_LEVEL );
				// load new level
				if( Current_Game_Action_Data.exists( "level" ) )
				{
					std::string lvl_name = Current_Game_Action_Data.getValueAsString( "level" ).c_str();

					// load level
					if( pActive_Level->Load( lvl_name ) )
					{
						pActive_Level->Enter();
					}
					// not a valid level
					else
					{
						printf( "Error : Level not found %s\n", lvl_name.c_str() );
						pHud_Debug->Set_Text( _("Loading Level failed : ") + lvl_name );

						// back to overworld
						pActive_Overworld->Enter();
						// walk to the next waypoint
						pActive_Overworld->Goto_Next_Level();
					}
				}
				// reset player
				pPlayer->Reset();
				// fade in
				Draw_Effect_In();
			}
			// Enter World
			else if( Current_Game_Action == GA_ENTER_WORLD )
			{
				// Random fade out effect
				Draw_Effect_Out( EFFECT_OUT_BLACK_TILED_RECTS );
				// set active world
				if( Current_Game_Action_Data.exists( "world" ) )
				{
					pOverworld_Manager->Set_Active( Current_Game_Action_Data.getValueAsString( "world" ).c_str() );
				}
				// set player waypoint
				if( Current_Game_Action_Data.exists( "player_waypoint" ) )
				{
					// get world waypoint
					int waypoint_num = pActive_Overworld->Get_Waypoint_Num( Current_Game_Action_Data.getValueAsString( "player_waypoint" ).c_str() );

					// waypoint available
					if( waypoint_num >= 0 )
					{
						// set all levels accessible until the world link
						pActive_Overworld->Set_Progress( waypoint_num, 0 );
						pOverworld_Player->Set_Waypoint( waypoint_num );
					}
				}
				// enter world
				pActive_Overworld->Enter();
				// draw fade in
				Draw_Effect_In();
			}
			// Enter Menu
			else if( Current_Game_Action == GA_ENTER_MENU )
			{
				pMenuCore->Load( MENU_MAIN, MODE_OVERWORLD );
			}
			// Enter Credits Menu
			else if( Current_Game_Action == GA_ENTER_MENU_CREDITS )
			{
				pMenuCore->Load( MENU_CREDITS );
			}
		}
	}
	// menu mode
	else if( Game_Mode == MODE_MENU )
	{
		// if Game Action set
		while( Game_Action != GA_NONE )
		{
			// get current data
			GameAction Current_Game_Action = Game_Action;
			CEGUI::XMLAttributes Current_Game_Action_Data = Game_Action_Data;
			void *Current_Game_Action_ptr = Game_Action_ptr;
			// clear
			Game_Action = GA_NONE;
			Game_Action_Data = CEGUI::XMLAttributes();
			Game_Action_ptr = NULL;

			// Enter World
			if( Current_Game_Action == GA_ENTER_WORLD )
			{
				// check if world is active
				bool world_active = 0;

				// if world loaded and not entering a new world
				if( (pMenuCore->pMenu_Data && pMenuCore->pMenu_Data->exit_to_gamemode == MODE_OVERWORLD) && !Current_Game_Action_Data.exists( "world" ) )
				{
					world_active = 1;
				}
				else
				{
					// fade out
					Draw_Effect_Out( EFFECT_OUT_BLACK, 3 );
				}

				// todo : fade only out if not same music
				// if origin menu is start menu
				if( pMenuCore->menu_id == MENU_START )
				{
					// fade music out
					pAudio->Fadeout_Music( 1000 );
				}
				// unload menu
				pMenuCore->Unload();
				// set active world
				if( Current_Game_Action_Data.getValueAsBool( "reset_save" ) )
				{
					// reset data
					pPlayer->Reset_Save();
				}
				if( Current_Game_Action_Data.exists( "world" ) )
				{
					pOverworld_Manager->Set_Active( Current_Game_Action_Data.getValueAsString( "world" ).c_str() );
				}
				// set player waypoint
				if( Current_Game_Action_Data.exists( "player_waypoint" ) )
				{
					// get world waypoint
					int waypoint_num = pActive_Overworld->Get_Waypoint_Num( Current_Game_Action_Data.getValueAsString( "player_waypoint" ).c_str() );

					// waypoint available
					if( waypoint_num >= 0 )
					{
						// set all levels accessible until the world link
						pActive_Overworld->Set_Progress( waypoint_num, 0 );
						pOverworld_Player->Set_Waypoint( waypoint_num );
					}
				}
				// enter world
				pActive_Overworld->Enter();
				if( !world_active )
				{
					// fade in
					Draw_Effect_In( EFFECT_IN_BLACK, 3 );
				}
			}
			// Enter Level
			else if( Current_Game_Action == GA_ENTER_LEVEL )
			{
				// check if level is active
				bool orig_level_active = 0;

				// if level loaded and not entering custom level
				if( (pMenuCore->pMenu_Data && pMenuCore->pMenu_Data->exit_to_gamemode == MODE_LEVEL) && !Current_Game_Action_Data.exists( "level" ) )
				{
					orig_level_active = 1;
				}
				else
				{
					// fade music out
					pAudio->Fadeout_Music( 1000 );
					// fade out
					Draw_Effect_Out( EFFECT_OUT_BLACK, 3 );
				}

				// unload menu
				pMenuCore->Unload();
				/* change mode
				 * should be done before loading a level as the correct sprite manager is needed
				*/
				Change_Game_Mode( MODE_LEVEL );
				// load custom level
				if( Current_Game_Action_Data.exists( "level" ) )
				{
					// reset state
					if( Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM )
					{
						pPlayer->Reset_Save();
					}
					// reset player
					else
					{
						pPlayer->Reset( 0 );
					}

					// load new level
					if( pActive_Level->Load( Current_Game_Action_Data.getValueAsString( "level" ).c_str() ) )
					{
						pActive_Level->Enter();
					}
				}

				if( !orig_level_active )
				{
					// fade in
					Draw_Effect_In( EFFECT_IN_BLACK, 3 );
				}
			}
			// Enter Menu
			else if( Current_Game_Action == GA_ENTER_MENU )
			{
				if( pMenuCore->next_menu != MENU_NOTHING )
				{
					GameMode exit_gamemode = MODE_NOTHING;

					if( pMenuCore->pMenu_Data )
					{
						exit_gamemode = pMenuCore->pMenu_Data->exit_to_gamemode;
					}

					pMenuCore->Load( pMenuCore->next_menu, exit_gamemode );
				}
			}
		}
	}
	// level settings mode
	else if( Game_Mode == MODE_LEVEL_SETTINGS )
	{
		// if Game Action set
		while( Game_Action != GA_NONE )
		{
			// get current data
			GameAction Current_Game_Action = Game_Action;
			CEGUI::XMLAttributes Current_Game_Action_Data = Game_Action_Data;
			void *Current_Game_Action_ptr = Game_Action_ptr;
			// clear
			Game_Action = GA_NONE;
			Game_Action_Data = CEGUI::XMLAttributes();
			Game_Action_ptr = NULL;

			// Enter Level
			if( Current_Game_Action == GA_ENTER_LEVEL )
			{
				// fade out
				Draw_Effect_Out( EFFECT_OUT_BLACK, 3 );
				// change mode
				pLevel_Editor->pSettings->Unload();
				Change_Game_Mode( MODE_LEVEL );
				// fade in
				Draw_Effect_In( EFFECT_IN_BLACK, 3 );
			}
		}
	}

	// ## input
	while( SDL_PollEvent( &input_event ) )
	{
		// handle
		Handle_Input_Global( &input_event );
	}

	pMouseCursor->Update();

	// ## audio
	pAudio->Resume_Music();
	pAudio->Update();

	// performance measuring
	pFramerate->m_perf_last_ticks = SDL_GetTicks();

	// ## update
	if( Game_Mode == MODE_LEVEL )
	{
		// input
		pActive_Level->Process_Input();
		pLevel_Editor->Process_Input();

		// update performance timer
		pFramerate->m_perf_timer[PERF_UPDATE_PROCESS_INPUT]->Update();

		// update
		pActive_Level->Update();

		// update performance timer
		pFramerate->m_perf_timer[PERF_UPDATE_LEVEL]->Update();

		// editor
		pLevel_Editor->Update();

		// update performance timer
		pFramerate->m_perf_timer[PERF_UPDATE_LEVEL_EDITOR]->Update();

		// hud
		pHud_Manager->Update();

		// update performance timer
		pFramerate->m_perf_timer[PERF_UPDATE_HUD]->Update();

		// player
		pPlayer->Update();

		// update performance timer
		pFramerate->m_perf_timer[PERF_UPDATE_PLAYER]->Update();

		// player collisions
		if( !editor_enabled )
		{
			pPlayer->Collide_Move();
			pPlayer->Handle_Collisions();
		}

		// update performance timer
		pFramerate->m_perf_timer[PERF_UPDATE_PLAYER_COLLISIONS]->Update();

		// late update for level objects
		pActive_Level->Update_Late();

		// update performance timer
		pFramerate->m_perf_timer[PERF_UPDATE_LATE_LEVEL]->Update();

		// level collisions
		if( !editor_enabled )
		{
			pActive_Sprite_Manager->Handle_Collision_Items();
		}

		// update performance timer
		pFramerate->m_perf_timer[PERF_UPDATE_LEVEL_COLLISIONS]->Update();

		// Camera ( update after new player position was set )
		pActive_Camera->Update();

		// update performance timer
		pFramerate->m_perf_timer[PERF_UPDATE_CAMERA]->Update();

	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		pActive_Overworld->Update();

		// update performance timer
		pFramerate->m_perf_timer[PERF_UPDATE_OVERWORLD]->Update();
	}
	else if( Game_Mode == MODE_MENU )
	{
		pMenuCore->Update();

		// update performance timer
		pFramerate->m_perf_timer[PERF_UPDATE_MENU]->Update();
	}
	else if( Game_Mode == MODE_LEVEL_SETTINGS )
	{
		pLevel_Editor->pSettings->Update();

		// update performance timer
		pFramerate->m_perf_timer[PERF_UPDATE_LEVEL_SETTINGS]->Update();
	}

	// gui
	Gui_Handle_Time();
}

void Draw_Game( void )
{
	// don't draw if exiting
	if( game_exit )
	{
		return;
	}

	// performance measuring
	pFramerate->m_perf_last_ticks = SDL_GetTicks();

	if( Game_Mode == MODE_LEVEL )
	{
		// draw level layer 1
		pActive_Level->Draw_Layer_1();

		// update performance timer
		pFramerate->m_perf_timer[PERF_DRAW_LEVEL_LAYER1]->Update();

		// player draw
		pPlayer->Draw();

		// update performance timer
		pFramerate->m_perf_timer[PERF_DRAW_LEVEL_PLAYER]->Update();

		// draw level layer 2
		pActive_Level->Draw_Layer_2();

		// update performance timer
		pFramerate->m_perf_timer[PERF_DRAW_LEVEL_LAYER2]->Update();

		// hud
		pHud_Manager->Draw();

		// update performance timer
		pFramerate->m_perf_timer[PERF_DRAW_LEVEL_HUD]->Update();

		// level editor
		pLevel_Editor->Draw();

		// update performance timer
		pFramerate->m_perf_timer[PERF_DRAW_LEVEL_EDITOR]->Update();

	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// world
		pActive_Overworld->Draw();

		// update performance timer
		pFramerate->m_perf_timer[PERF_DRAW_OVERWORLD]->Update();
	}
	else if( Game_Mode == MODE_MENU )
	{
		pMenuCore->Draw();

		// update performance timer
		pFramerate->m_perf_timer[PERF_DRAW_MENU]->Update();
	}
	else if( Game_Mode == MODE_LEVEL_SETTINGS )
	{
		pLevel_Editor->pSettings->Draw();

		// update performance timer
		pFramerate->m_perf_timer[PERF_DRAW_LEVEL_SETTINGS]->Update();
	}

	// Mouse
	pMouseCursor->Draw();

	// update performance timer
	pFramerate->m_perf_timer[PERF_DRAW_MOUSE]->Update();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
