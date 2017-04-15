/*

This file contains code for the setup menu (which lets the user setup a world with whatever settings are wanted).

Basically it sets up interface elements that are then used by code in s_menu.c to display a menu and deal with input from it.
s_menu.c calls back here for various things.

*/

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"
#include "i_header.h"
#include "m_maths.h"

#include "g_misc.h"

#include "c_header.h"
#include "e_slider.h"
#include "e_header.h"
#include "e_editor.h"
#include "e_help.h"
#include "e_log.h"
#include "g_game.h"
#include "g_world.h"

#include "i_input.h"
#include "i_view.h"
#include "i_display.h"
#include "i_disp_in.h"
#include "i_buttons.h"
#include "t_template.h"
#include "m_input.h"
#include "f_load.h"
#include "f_game.h"
#include "x_sound.h"

#include "s_mission.h"
#include "s_menu.h"
#include "g_world_back.h"
#include "g_world_map.h"
#include "g_world_map_2.h"

#include "h_story.h"

extern struct map_init_struct map_init;


//#include "s_setup.h"

extern struct fontstruct font [FONTS];
extern ALLEGRO_DISPLAY* display;

// these queues are declared in g_game.c. They're externed here so that they can be flushed when the editor does something slow.
extern ALLEGRO_EVENT_QUEUE* event_queue; // these queues are initialised in main.c
extern ALLEGRO_EVENT_QUEUE* fps_queue;

extern struct game_struct game; // in g_game.c
extern struct view_struct view;

void init_w_init(void);
static void enter_map_code(void);
static int read_map_code_value(char read_char, int number);

//void generate_w_init_map(void);
static void reset_map_for_menu(void);
//static void place_player_on_w_init_map(int player_index, int spawn_x, int spawn_y, int flip_x, int flip_y);
//static void seed_mrand(int mrand_seed);
//static int mrand(int range);

static void print_key_codes(void);


char mstring [MENU_STRING_LENGTH];

#define ELEMENT_NAME_SIZE 30

enum
{
EL_TYPE_ACTION, // does something (e.g. move to another menu) when clicked.
EL_TYPE_SLIDER, // has a slider - not currently used, may not work
EL_TYPE_COLOUR, // gives a set of colours to choose from
EL_TYPE_HEADING, // does nothing and can't be selected
EL_TYPE_SELECT, // select from a list
EL_TYPES
};

enum // these are particular sliders used in particular elements.
{
EL_SLIDER_PLAYERS,
EL_SLIDER_PROCS,
EL_SLIDER_TURNS,
EL_SLIDER_MINUTES,
//EL_SLIDER_GEN_LIMIT,
EL_SLIDER_W_BLOCK,
EL_SLIDER_H_BLOCK,
EL_SLIDERS
};

enum
{
// start menu
EL_ACTION_MISSION,
EL_ACTION_ADVANCED_MISSION,
EL_ACTION_STORY,
EL_ACTION_STORY_ADVANCED,
EL_ACTION_STORY_HARD,
EL_ACTION_STORY_ADVANCED_HARD,
EL_ACTION_TUTORIAL,
EL_ACTION_SETUP_GAME,
EL_ACTION_LOAD,
//EL_ACTION_SETUP,
//EL_ACTION_TUTORIAL,
EL_ACTION_OPTIONS,
EL_ACTION_QUIT,
EL_ACTION_NAME, // player name input
//EL_ACTION_SAVE_GAMEFILE,
EL_ACTION_LOAD_GAMEFILE,
EL_ACTION_START_MISSION,

// setup menu
EL_ACTION_START_GAME_FROM_SETUP,
EL_ACTION_ENTER_CODE,
EL_ACTION_RANDOMISE_CODE,

// misc
EL_ACTION_BACK_TO_START,
EL_ACTION_NONE

};

struct slider_struct element_slider [EL_SLIDERS];

// menu_liststruct holds the lists for creating menu elements.
// lists should be constant (mutable values are in menu_elementstruct)
struct menu_liststruct
{
 int type; // basic type of the element (e.g. button that does something, slider, etc.)
 int action; // specific effect of element (unique to each element, although a single element can be present in multiple menu types)
 int start_value; // if the element has a value (e.g. a slider) this is the default
 char name [ELEMENT_NAME_SIZE];
 int slider_index; // is -1 if element doesn't have a slider
// int help_type; // HELP_* string to print when right-clicked (see e_help.c)

};

// each time a menu is opened, a list of menu_elementstructs is initialised with values based on menu_liststruct
struct menu_elementstruct
{
 int list_index;
 int x1, y1, x2, y2, w, h; // location/size

 int type;
 int value;
 int fixed; // if 1, the value cannot be changed (probably because it's a PI_VALUES things and its PI_OPTION is set to zero)
 int highlight;
 int slider_index;

};


enum
{
	// This list must be ordered in the same way as the big menu_list array below
EL_MAIN_HEADING,
EL_MAIN_START_GAME,
EL_MAIN_START_GAME_ADVANCED,
EL_MAIN_START_GAME_HARD,
EL_MAIN_START_GAME_ADVANCED_HARD,
EL_MAIN_TUTORIAL,
EL_MAIN_SETUP_GAME,
//EL_MAIN_LOAD,
EL_MAIN_LOAD_GAMEFILE,
//EL_MAIN_SETUP,
//EL_MAIN_TUTORIAL,
//EL_MAIN_OPTIONS,
EL_MAIN_QUIT,

EL_SETUP_HEADING,
EL_SETUP_START,

/*
Setup buttons:
start
players
//  time limit - short (16 mins), medium (32) long (64), unlimited - no don't do this
cores/procs - few (8/32), some (16/64), many (32/128), heaps (64/256)
map size - small, medium, large
game code
randomise

exit

Codes
first letter: players A=2,B=3,C=4,???D=4:assymetrical
second letter: map size A,B,C
third letter: cores/procs A-D
4-6 seed numbers (0-9)

*/


EL_SETUP_PLAYERS,
EL_SETUP_COMMAND_MODE,
//EL_SETUP_TURNS,
//EL_SETUP_TIME,
EL_SETUP_MAP_SIZE,
EL_SETUP_CORES,
EL_SETUP_DATA,
EL_SETUP_CODE,
EL_SETUP_RANDOMISE_CODE,
//EL_SETUP_GEN_LIMIT,
// EL_SETUP_PACKETS,  should probably derive this from procs rather than allowing it to be set
//EL_SETUP_W_BLOCK,
//EL_SETUP_H_BLOCK,
EL_SETUP_BACK_TO_START,
/*EL_SETUP_PLAYER_COL_0,
EL_SETUP_PLAYER_COL_1,
EL_SETUP_PLAYER_COL_2,
EL_SETUP_PLAYER_COL_3,*/
EL_SETUP_PLAYER_NAME_0,
EL_SETUP_PLAYER_NAME_1,
EL_SETUP_PLAYER_NAME_2,
EL_SETUP_PLAYER_NAME_3,
//EL_SETUP_SAVE_GAMEFILE,

EL_MISSIONS_HEADING_TUTORIAL,
EL_MISSIONS_T1,
EL_MISSIONS_T2,
EL_MISSIONS_T3,
EL_MISSIONS_HEADING_ADV_TUTORIAL,
EL_MISSIONS_T4,
EL_TUTORIAL_BACK,
EL_MISSIONS_HEADING_MISSIONS,
EL_MISSIONS_M1,
EL_MISSIONS_M2,
EL_MISSIONS_M3,
EL_MISSIONS_M4,
EL_MISSIONS_M5,
EL_MISSIONS_M6,
EL_MISSIONS_M7,
EL_MISSIONS_M8,
EL_MISSIONS_BACK,
EL_MISSIONS_HEADING_ADVANCED_MISSIONS,
EL_ADVANCED_MISSIONS_M1,
EL_ADVANCED_MISSIONS_M2,
EL_ADVANCED_MISSIONS_M3,
EL_ADVANCED_MISSIONS_M4,
EL_ADVANCED_MISSIONS_M5,
EL_ADVANCED_MISSIONS_M6,
EL_ADVANCED_MISSIONS_M7,
EL_ADVANCED_MISSIONS_M8,
EL_ADVANCED_MISSIONS_BACK,


// when adding a new member to this list, must also add to the menu_list list below
ELS
};

char *locked_string = {"LOCKED"};

struct menu_liststruct menu_list [ELS] =
{
// main menu
 {
  EL_TYPE_HEADING,
  EL_ACTION_NONE, // action
  0, // start_value (used as mission index)
  "START MENU", // name
  -1, // slider_index
//  HELP_NONE, // help_type
 }, // EL_MAIN_HEADING
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_STORY, // action
  0, // start_value
  "START", // name
  -1, // slider_index
//  HELP_MISSION_MENU, // help_type
 }, // EL_MAIN_START_GAME
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_STORY_ADVANCED, // action
  0, // start_value
  "START  <AUTONOMOUS>", // name
  -1, // slider_index
//  HELP_ADVANCED_MISSION_MENU, // help_type
 }, // EL_MAIN_START_GAME_ADVANCED
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_STORY_HARD, // action
  0, // start_value
  "START  <HARD>", // name
  -1, // slider_index
//  HELP_MISSION_MENU, // help_type
 }, // EL_MAIN_START_GAME_HARD
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_STORY_ADVANCED_HARD, // action
  0, // start_value
  "START  <AUTONOMOUS+HARD>", // name
  -1, // slider_index
//  HELP_ADVANCED_MISSION_MENU, // help_type
 }, // EL_MAIN_START_GAME_ADVANCED_HARD
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_TUTORIAL, // action
  0, // start_value
  "TUTORIAL", // name
  -1, // slider_index
//  HELP_TUTORIAL_MENU, // help_type
 }, // EL_MAIN_TUTORIAL
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_SETUP_GAME, // action
  0, // start_value
  "CUSTOM GAME", // name
  -1, // slider_index
//  HELP_USE_SYSFILE, // help_type
 }, // EL_MAIN_SETUP_GAME
/* {
  EL_TYPE_ACTION, // type
  EL_ACTION_LOAD, // action
  0, // start_value
  "Load Saved Game", // name
  -1, // slider_index
  HELP_LOAD, // help_type
 }, // EL_MAIN_LOAD*/
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_LOAD_GAMEFILE, // action
  0, // start_value
  "LOAD GAMEFILE", // name
  -1, // slider_index
//  HELP_LOAD_GAMEFILE, // help_type
 }, // EL_MAIN_LOAD_GAMEFILE
/* {
  EL_TYPE_ACTION, // type
  EL_ACTION_OPTIONS, // action
  0, // start_value
  "Options", // name
  -1, // slider_index
  HELP_OPTIONS, // help_type
 }, // EL_MAIN_OPTIONS*/
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_QUIT, // action
  0, // start_value
  "EXIT", // name
  -1, // slider_index
//  HELP_MAIN_EXIT, // help_type
 }, // EL_MAIN_EXIT

// setup menu
 {
  EL_TYPE_HEADING,
  EL_ACTION_NONE, // action
  0, // start_value (used as mission index)
  "CUSTOM GAME", // name
  -1, // slider_index
//  HELP_NONE, // help_type
 }, // EL_SETUP_HEADING
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_START_GAME_FROM_SETUP, // action
  0, // start_value
  "START", // name
  -1, // slider_index
//  HELP_SETUP_START, // help_type
 }, // EL_SETUP_START
 {
  EL_TYPE_SELECT, // type
  2, // minimum value
  4, // maximum value
  "PLAYERS", // name
  -1, // slider_index
//  HELP_SETUP_PLAYERS, // help_type
 }, // EL_SETUP_PLAYERS
 {
  EL_TYPE_SELECT, // type
  0, // minimum value
  1, // maximum value
  "COMMAND MODE", // name
  -1, // slider_index
//  HELP_SETUP_PLAYERS, // help_type
 }, // EL_SETUP_COMMAND_MODE
/* {
  EL_TYPE_SLIDER, // type
  0, // action
  0, // start_value - derived from pre_init
  "Turns", // name
  EL_SLIDER_TURNS, // slider_index
  HELP_SETUP_TURNS, // help_type
 }, // EL_SETUP_TURNS*/
/* {
  EL_TYPE_SLIDER, // type
  0, // action
  0, // start_value - derived from pre_init
  "Game length (minutes)", // name
  EL_SLIDER_MINUTES, // slider_index
  HELP_SETUP_MINUTES, // help_type
 }, // EL_SETUP_MINUTES*/
 {
  EL_TYPE_SELECT, // type
  0, // minimum value
  3, // maximum value
  "MAP SIZE", // name
  -1, // slider_index
//  HELP_SETUP_W_BLOCK, // help_type
 }, // EL_SETUP_MAP_SIZE
 {
  EL_TYPE_SELECT, // type
  0, // minimum value
  3, // maximum value
  "PROCESSES", // name
  -1, // slider_index
//  HELP_SETUP_PROCS, // help_type
 }, // EL_SETUP_CORES
 {
  EL_TYPE_SELECT, // type
  0, // minimum value
  3, // maximum value
  "STARTING DATA", // name
  -1, // slider_index
 }, // EL_SETUP_DATA
/* {
  EL_TYPE_SLIDER, // type
  0, // action
  0, // start_value - derived from pre_init
  "Components", // name
  EL_SLIDER_PROCS, // slider_index
  HELP_SETUP_PROCS, // help_type
 }, // EL_SETUP_PROCS*/
/* {
  EL_TYPE_SLIDER, // type
  0, // action
  0, // start_value - derived from pre_init
  "Gen limit", // name
  EL_SLIDER_GEN_LIMIT, // slider_index
 }, // EL_SETUP_GEN_LIMIT*/
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_ENTER_CODE, // minimum value
  4, // maximum value
  "MAP CODE", // name
  -1, // slider_index
//  HELP_SETUP_W_BLOCK, // help_type
 }, // EL_SETUP_CODE
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_RANDOMISE_CODE, // action
  0, // maximum value
  "RANDOMISE CODE SEED", // name
  -1, // slider_index
//  HELP_SETUP_W_BLOCK, // help_type
 }, // EL_SETUP_RANDOMISE_CODE
/* {
  EL_TYPE_SLIDER, // type
  0, // action
  0, // start_value - derived from pre_init
  "World size (y)", // name
  EL_SLIDER_H_BLOCK, // slider_index
  HELP_SETUP_H_BLOCK, // help_type
 }, // EL_SETUP_H_BLOCK*/
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_BACK_TO_START, // action
  0, // start_value
  "BACK", // name
  -1, // slider_index
//  HELP_SETUP_BACK_TO_START, // help_type
 }, // EL_SETUP_BACK_TO_START
 {
  EL_TYPE_ACTION,
  EL_ACTION_NAME, // action
  0, // start_value (used as player index)
  "Player 0 name", // name
  -1, // slider_index
//  HELP_SETUP_PLAYER_NAME, // help_type
 }, // EL_SETUP_PLAYER_NAME_0
 {
  EL_TYPE_ACTION,
  EL_ACTION_NAME, // action (player index)
  1, // start_value (used as player index)
  "Player 1 name", // name
  -1, // slider_index
//  HELP_SETUP_PLAYER_NAME, // help_type
 }, // EL_SETUP_PLAYER_NAME_1
 {
  EL_TYPE_ACTION,
  EL_ACTION_NAME, // action (player index)
  2, // start_value (used as player index)
  "Player 2 name", // name
  -1, // slider_index
//  HELP_SETUP_PLAYER_NAME, // help_type
 }, // EL_SETUP_PLAYER_NAME_2
 {
  EL_TYPE_ACTION,
  EL_ACTION_NAME, // action (player index)
  3, // start_value (used as player index)
  "Player 3 name", // name
  -1, // slider_index
//  HELP_SETUP_PLAYER_NAME, // help_type
 }, // EL_SETUP_PLAYER_NAME_3
/* {
  EL_TYPE_ACTION,
  EL_ACTION_SAVE_GAMEFILE, // action
  0, // start_value (used as player index)
  "Save gamefile", // name
  -1, // slider_index
  HELP_SETUP_SAVE_GAMEFILE, // help_type
 }, // EL_SETUP_SAVE_GAMEFILE*/

// Missions
 {
  EL_TYPE_HEADING,
  EL_ACTION_NONE, // action
  0, // start_value (used as mission index)
  "Tutorials", // name
  -1, // slider_index
//  HELP_NONE, // help_type
 }, // EL_MISSIONS_HEADING_TUTORIAL
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
  MISSION_TUTORIAL1, // start_value (used as mission index)
  "Tutorial: basics", // name
  -1, // slider_index
//  HELP_TUTORIAL_1, // help_type
 }, // EL_MISSIONS_T1
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
  MISSION_TUTORIAL2, // start_value (used as mission index)
  "Tutorial: building + attacking", // name
  -1, // slider_index
//  HELP_TUTORIAL_2, // help_type
 }, // EL_MISSIONS_T2
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
  0,//  MISSION_TUTORIAL3, // start_value (used as mission index)
  "Tutorial: templates", // name
  -1, // slider_index
//  HELP_TUTORIAL_3, // help_type
 }, // EL_MISSIONS_T3
 {
  EL_TYPE_HEADING,
  EL_ACTION_NONE, // action
  0, // start_value (used as mission index)
  "Advanced tutorial", // name
  -1, // slider_index
//  HELP_NONE, // help_type
 }, // EL_MISSIONS_HEADING_ADV_TUTORIAL
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//  MISSION_TUTORIAL4, // start_value (used as mission index)
  "Tutorial: delegation", // name
  -1, // slider_index
//  HELP_TUTORIAL_4, // help_type
 }, // EL_MISSIONS_T4
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_BACK_TO_START, // action
  0, // start_value
  "Back", // name
  -1, // slider_index
//  HELP_TUTORIAL_BACK, // help_type
 }, // EL_TUTORIAL_BACK
 {
  EL_TYPE_HEADING,
  EL_ACTION_NONE, // action
  0, // start_value (used as mission index)
  "MISSIONS", // name
  -1, // slider_index
//  HELP_NONE, // help_type
 }, // EL_MISSIONS_HEADING_MISSIONS
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//  MISSION_MISSION1, // start_value (used as mission index)
  "MISSION 1 - TUTORIAL", // name
  -1, // slider_index
//  HELP_MISSION, // help_type
 }, // EL_MISSIONS_M1
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_MISSION2, // start_value (used as mission index)
  "MISSION 2 - TUTORIAL", // name
  -1, // slider_index
//  HELP_MISSION, // help_type
 }, // EL_MISSIONS_M2
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_MISSION3, // start_value (used as mission index)
  "MISSION 3", // name
  -1, // slider_index
//  HELP_MISSION, // help_type
 }, // EL_MISSIONS_M3
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_MISSION4, // start_value (used as mission index)
  "MISSION 4", // name
  -1, // slider_index
//  HELP_MISSION, // help_type
 }, // EL_MISSIONS_M4
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_MISSION5, // start_value (used as mission index)
  "MISSION 5", // name
  -1, // slider_index
//  HELP_MISSION, // help_type
 }, // EL_MISSIONS_M5
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_MISSION6, // start_value (used as mission index)
  "MISSION 6", // name
  -1, // slider_index
//  HELP_MISSION, // help_type
 }, // EL_MISSIONS_M6
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_MISSION7, // start_value (used as mission index)
  "MISSION 7", // name
  -1, // slider_index
//  HELP_MISSION, // help_type
 }, // EL_MISSIONS_M7
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_MISSION8, // start_value (used as mission index)
  "MISSION 8", // name
  -1, // slider_index
//  HELP_MISSION, // help_type
 }, // EL_MISSIONS_M8
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_BACK_TO_START, // action
  0, // start_value
  "BACK", // name
  -1, // slider_index
//  HELP_MISSIONS_BACK, // help_type
 }, // EL_MISSIONS_BACK
 {
  EL_TYPE_HEADING,
  EL_ACTION_NONE, // action
  0, // start_value (used as mission index)
  "ADVANCED MISSIONS", // name
  -1, // slider_index
//  HELP_NONE, // help_type
 }, // EL_MISSIONS_HEADING_MISSIONS
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_ADVANCED1, // start_value (used as mission index)
  "ADVANCED MISSION 1", // name
  -1, // slider_index
//  HELP_ADVANCED_MISSION, // help_type
 }, // EL_ADVANCED_MISSIONS_M1
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_ADVANCED2, // start_value (used as mission index)
  "ADVANCED MISSION 2", // name
  -1, // slider_index
//  HELP_ADVANCED_MISSION, // help_type
 }, // EL_ADVANCED_MISSIONS_M2
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_ADVANCED3, // start_value (used as mission index)
  "ADVANCED MISSION 3", // name
  -1, // slider_index
//  HELP_ADVANCED_MISSION, // help_type
 }, // EL_ADVANCED_MISSIONS_M3
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_ADVANCED4, // start_value (used as mission index)
  "ADVANCED MISSION 4", // name
  -1, // slider_index
//  HELP_ADVANCED_MISSION, // help_type
 }, // EL_ADVANCED_MISSIONS_M4
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_ADVANCED5, // start_value (used as mission index)
  "ADVANCED MISSION 5", // name
  -1, // slider_index
//  HELP_ADVANCED_MISSION, // help_type
 }, // EL_ADVANCED_MISSIONS_M5
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_ADVANCED6, // start_value (used as mission index)
  "ADVANCED MISSION 6", // name
  -1, // slider_index
//  HELP_ADVANCED_MISSION, // help_type
 }, // EL_ADVANCED_MISSIONS_M6
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_ADVANCED7, // start_value (used as mission index)
  "ADVANCED MISSION 7", // name
  -1, // slider_index
//  HELP_ADVANCED_MISSION, // help_type
 }, // EL_ADVANCED_MISSIONS_M7
 {
  EL_TYPE_ACTION,
  EL_ACTION_START_MISSION, // action
0,//    MISSION_ADVANCED8, // start_value (used as mission index)
  "ADVANCED MISSION 8", // name
  -1, // slider_index
//  HELP_ADVANCED_MISSION, // help_type
 }, // EL_ADVANCED_MISSIONS_M8
 {
  EL_TYPE_ACTION, // type
  EL_ACTION_BACK_TO_START, // action
  0, // start_value
  "BACK", // name
  -1, // slider_index
//  HELP_MISSIONS_BACK, // help_type
 }, // EL_ADVANCED_MISSIONS_BACK




/*
 {
  EL_TYPE_COL, // type
  0, // action
  0, // start_value - derived from pre_init
  "Colour", // name
  -1, // slider_index
 }, // EL_SETUP_COL*/



};

enum
{
MENU_MAIN,
MENU_SETUP,
MENU_MISSIONS,
MENU_ADVANCED_MISSIONS,
MENU_TUTORIAL,

MENUS
};

int menu_list_main [] =
{
EL_MAIN_HEADING,
EL_MAIN_START_GAME,
EL_MAIN_START_GAME_HARD,
EL_MAIN_START_GAME_ADVANCED,
EL_MAIN_START_GAME_ADVANCED_HARD,
//EL_MAIN_TUTORIAL,
EL_MAIN_SETUP_GAME,
//EL_MAIN_LOAD,
//EL_MAIN_LOAD_GAMEFILE,
//EL_MAIN_SETUP,
//EL_MAIN_TUTORIAL,
//EL_MAIN_OPTIONS,
EL_MAIN_QUIT,
-1 // terminates list
};

int menu_list_setup [] =
{
EL_SETUP_HEADING,
EL_SETUP_START,
EL_SETUP_COMMAND_MODE,
EL_SETUP_PLAYERS,
//EL_SETUP_TURNS,
//EL_SETUP_MINUTES,
EL_SETUP_MAP_SIZE,
EL_SETUP_CORES,
EL_SETUP_DATA,
EL_SETUP_CODE,
EL_SETUP_RANDOMISE_CODE,
//EL_SETUP_PROCS,
//EL_SETUP_GEN_LIMIT,
// EL_SETUP_PACKETS,
//EL_SETUP_W_BLOCK,
//EL_SETUP_H_BLOCK,
/*EL_SETUP_PLAYER_COL_0, // must come after EL_SETUP_PLAYERS (as some colour menu options aren't shown if the game has fewer players)
EL_SETUP_PLAYER_COL_1,
EL_SETUP_PLAYER_COL_2,
EL_SETUP_PLAYER_COL_3,*/
EL_SETUP_PLAYER_NAME_0,
EL_SETUP_PLAYER_NAME_1,
EL_SETUP_PLAYER_NAME_2,
EL_SETUP_PLAYER_NAME_3,
//EL_SETUP_SAVE_GAMEFILE,
EL_SETUP_BACK_TO_START,

-1 // terminates list
};

int menu_list_missions [] =
{
EL_MISSIONS_HEADING_MISSIONS,
EL_MISSIONS_M1,
EL_MISSIONS_M2,
EL_MISSIONS_M3,
EL_MISSIONS_M4,
EL_MISSIONS_M5,
EL_MISSIONS_M6,
EL_MISSIONS_M7,
EL_MISSIONS_M8,
EL_MISSIONS_BACK,
-1 // terminates list
};

int menu_list_advanced_missions [] =
{
EL_MISSIONS_HEADING_ADVANCED_MISSIONS,
EL_ADVANCED_MISSIONS_M1,
EL_ADVANCED_MISSIONS_M2,
EL_ADVANCED_MISSIONS_M3,
EL_ADVANCED_MISSIONS_M4,
EL_ADVANCED_MISSIONS_M5,
EL_ADVANCED_MISSIONS_M6,
EL_ADVANCED_MISSIONS_M7,
EL_ADVANCED_MISSIONS_M8,
EL_ADVANCED_MISSIONS_BACK,
-1 // terminates list
};


int menu_list_tutorial [] =
{
EL_MISSIONS_HEADING_TUTORIAL,
EL_MISSIONS_T1,
EL_MISSIONS_T2,
EL_MISSIONS_T3,
EL_MISSIONS_HEADING_ADV_TUTORIAL,
EL_MISSIONS_T4,
EL_TUTORIAL_BACK,
-1 // terminates list
};



// MAX_ELEMENTS is the max number of elements in one menu
#define MAX_ELEMENTS 30
#define MENU_H (40 + scaleUI_y(FONT_SQUARE_LARGE,10))
#define MENU_W scaleUI_x(FONT_SQUARE_LARGE,300)

struct menu_elementstruct menu_element [MAX_ELEMENTS];

struct world_init_struct w_init; // this is the world_init generated by world setup menus

enum
{
MENU_TYPE_NORMAL, // vertical list of options
//MENU_TYPE_PREGAME, // just shows text

};

/*enum
{
PREGAME_BUTTON_GO,
PREGAME_BUTTON_BACK,
PREGAME_BUTTONS

};*/

enum
{
MENU_TEXT_NONE,
MENU_TEXT_PLAYER_0_NAME,
MENU_TEXT_PLAYER_1_NAME,
MENU_TEXT_PLAYER_2_NAME,
MENU_TEXT_PLAYER_3_NAME,
MENU_TEXT_MAP_CODE
};

#define MENU_STRIPES 24
#define STRIPE_COLS 12

enum
{
MAP_CODE_PLAYERS,
MAP_CODE_SIZE,
MAP_CODE_CORES,
MAP_CODE_DATA,
MAP_CODE_SEED_0,
MAP_CODE_SEED_1,
MAP_CODE_SEED_2,

MAP_CODE_LENGTH
};

struct menu_statestruct
{
 int menu_type; // MENU_TYPE_NORMAL or MENU_TYPE_PREGAME
 int menu_index;

 int elements; // this is the number of elements the current menu has. Set by open_menu()
 int h; // total height (in pixels) of menu display
 int window_pos; // if menu is too long to display, this is the pos of the top of screen
// int edit_window;
 struct slider_struct mscrollbar; // vertical scrollbar if menu is too long to display
 int use_scrollbar; // 0 if no scrollbar, 1 if scrollbar
 int menu_templ_state; // e.g. MENU_TEMPL_STATE_MAIN
 int menu_text_box; // MENU_TEXT_xxx (is MENU_TEXT_NONE if no text box open)

 char map_code_string [MAP_CODE_LENGTH + 1];
 char map_code_string_temp [MAP_CODE_LENGTH + 1]; // just used for the text box for entering the map code

/* int pregame_button_x [PREGAME_BUTTONS];
 int pregame_button_y [PREGAME_BUTTONS];
 int pregame_button_w [PREGAME_BUTTONS];
 int pregame_button_h [PREGAME_BUTTONS];
 int pregame_button_highlight [PREGAME_BUTTONS];*/

 int stripe_group_col;
 int stripe_group_shade;
 int stripe_group_time;
 int stripe_next_group_count;
 int stripe_next_stripe;

 int stripe_exists [MENU_STRIPES];
 float stripe_x [MENU_STRIPES];
 float stripe_size [MENU_STRIPES];
 int stripe_col [MENU_STRIPES];
 int stripe_shade [MENU_STRIPES];

 ALLEGRO_COLOR stripe_al_col [STRIPE_COLS];

};

struct menu_statestruct mstate;


void run_menu_input(void);
void open_menu(int menu_index);
void reset_menu_templates(void);
void display_menu_1(void);
void display_menu_2(void);
void menu_loop(void);
void run_menu(void);
void run_game_from_menu(void);
static void fix_map_code(void);

int get_menu_element_value(int t_index);
void run_intro_screen(void);
void run_menu_stripes(int draw);
void draw_menu_button(float xa, float ya, float xb, float yb, ALLEGRO_COLOR button_colour);

#define MENU_X 100

// this function initialises the menu state when a menu is opened
// it populates the menu_element list using values from the relevant menu_list.
// it may do other things as well.
void open_menu(int menu_index)
{


// we don't want the new menu to receive the mouse button press (which I think is particularly a problem for sliders):
 if (ex_control.mb_press [0] == BUTTON_JUST_PRESSED)
  ex_control.mb_press [0] = BUTTON_HELD;

 int i = 0;
 int using_list_index;
 int x = MENU_X;
 int y = 50;
 if (settings.option [OPTION_WINDOW_H] >= 800)
		y = 100;
// int values_from_pre [3] = {0,0,0};
 int *mlist; // this is a pointer to an array of elements, which must be -1 terminated.

 switch(menu_index)
 {
  case MENU_MAIN:
   mlist = menu_list_main;
   reset_menu_templates();
   break;
  case MENU_SETUP:
   mlist = menu_list_setup;
   reset_map_for_menu();
   break;
  case MENU_MISSIONS:
   mlist = menu_list_missions;
   break;
  case MENU_ADVANCED_MISSIONS:
			mlist = menu_list_advanced_missions;
			break;
  case MENU_TUTORIAL:
			mlist = menu_list_tutorial;
			break;
  default:
   fprintf(stdout, "\ns_menu.c: open_menu(): unrecognised menu_index %i", menu_index);
   error_call();
   mlist = menu_list_main; // avoids compiler warning
   break;
 }

 mstate.menu_index = menu_index;


// if (settings.edit_window == EDIT_WINDOW_CLOSED)
//  open_templates();

// inter.mode_button_available [MODE_BUTTON_SYSTEM] = 1;
// inter.mode_button_available [MODE_BUTTON_TEMPLATES] = 1;
// inter.mode_button_available [MODE_BUTTON_EDITOR] = 1;
// inter.mode_button_available [MODE_BUTTON_DESIGN] = 1;
// inter.mode_button_available [MODE_BUTTON_CLOSE] = 1;
// inter.mode_button_available [MODE_BUTTON_MIN_MAX] = 1;

 int ml = 0;

 while (mlist [ml] != -1)
 {
#ifdef SANITY_CHECK
  if (i >= MAX_ELEMENTS)
  {
   fprintf(stdout, "\nError: s_menu.c: open_menu(): too many elements (%i) in menu %i (max %i).", i, menu_index, MAX_ELEMENTS);
   error_call();
  }
#endif
  using_list_index = mlist [ml];
// some mlist entries may not be displayed, depending on initialisation factors (currently, this is only done to remove player colour options for players that can't exist)
/*  switch (using_list_index)
  {
//   case EL_SETUP_PLAYER_COL_1:
   case EL_SETUP_PLAYER_NAME_1:
    if (max_players < 2)
    {
     ml ++;
     continue;
    }
    break;
//   case EL_SETUP_PLAYER_COL_2:
   case EL_SETUP_PLAYER_NAME_2:
    if (max_players < 3)
    {
     ml ++;
     continue;
    }
    break;
//   case EL_SETUP_PLAYER_COL_3:
   case EL_SETUP_PLAYER_NAME_3:
    if (max_players < 4)
    {
     ml ++;
     continue;
    }
    break;
  }
*/
  menu_element [i].list_index = using_list_index;
  menu_element [i].x1 = x;
  menu_element [i].y1 = y;
  menu_element [i].w = MENU_W;
  menu_element [i].h = MENU_H;
  menu_element [i].x2 = x + menu_element [i].w;
  menu_element [i].y2 = y + menu_element [i].h;
  menu_element [i].highlight = 0;
  menu_element [i].value = menu_list[using_list_index].start_value;
  menu_element [i].type = menu_list[using_list_index].type;
  menu_element [i].fixed = 0; // can be changed in derive_value_from_preinit_array
// some values are derived from pre_init:
  switch(menu_element [i].list_index)
  {
/*
   case EL_SETUP_PLAYERS: // derive_value_from_preinit_array() takes values from w_pre_init and puts them in values_from_pre and the value field of menu_element [i]
    derive_value_from_preinit_array(values_from_pre, w_pre_init.players, &menu_element [i]);
    max_players = values_from_pre [0]; // default value (which is used if value fixed)
    if (menu_element[i].fixed == 0) // if not fixed, use the maximum value that players can be set to
     max_players = values_from_pre [2];
    break;*/
/*   case EL_SETUP_TURNS:
    derive_value_from_preinit_array(values_from_pre, w_pre_init.game_turns, &menu_element [i]);
    break;
   case EL_SETUP_MINUTES:
    derive_value_from_preinit_array(values_from_pre, w_pre_init.game_minutes_each_turn, &menu_element [i]);
    break;
   case EL_SETUP_PROCS:
    derive_value_from_preinit_array(values_from_pre, w_pre_init.procs_per_player, &menu_element [i]);
    break;*/
//   case EL_SETUP_GEN_LIMIT:
//    derive_value_from_preinit_array(values_from_pre, w_pre_init.gen_limit, &menu_element [i]);
//    break;
/*   case EL_SETUP_W_BLOCK:
    derive_value_from_preinit_array(values_from_pre, w_pre_init.w_block, &menu_element [i]);
    break;
   case EL_SETUP_H_BLOCK:
    derive_value_from_preinit_array(values_from_pre, w_pre_init.h_block, &menu_element [i]);
    break;
*/

  }

  menu_element[i].slider_index = menu_list[using_list_index].slider_index;

  if (menu_element [i].slider_index != -1)
  {
/*
   init_slider(&element_slider [menu_element [i].slider_index],
               &menu_element [i].value, // value_pointer
               SLIDEDIR_HORIZONTAL,  // dir
               values_from_pre [1], // value_min
               values_from_pre [2], // value_max
               100, // total_length
               1, // button_increment
               1, // track_increment
               1, // slider_represents_size
               x + 160, // x
               y + 20, // y
               SLIDER_BUTTON_SIZE, // thickness
               COL_GREY, // colour
               0); // hidden_if_unused*/
  }
  y += MENU_H; // the final value of y is used to determine mstate.h
  i ++;
  ml ++;
 };

 y += 30; // a bit of space at the end

 mstate.elements = i;
 mstate.h = y;
 mstate.window_pos = 0;

 if (mstate.h > settings.option [OPTION_WINDOW_H])
 {
/*   mstate.use_scrollbar = 1;
   init_slider(&mstate.mscrollbar, // *sl
               &mstate.window_pos, // *value_pointer
               SLIDEDIR_VERTICAL, //dir
               0, // value_min
               mstate.h - settings.option [OPTION_WINDOW_H], // value_max
               settings.option [OPTION_WINDOW_H], // total_length
               9, // button_increment
               80, // track_increment
               settings.option [OPTION_WINDOW_H], // slider_represents_size
               settings.editor_x_split - SLIDER_BUTTON_SIZE, // x
               0, // y
               SLIDER_BUTTON_SIZE, // thickness
               COL_GREEN, // colour
               0); // hidden_if_unused
*/
 }
  else
  {
   mstate.use_scrollbar = 0;
  }

 mstate.menu_text_box = MENU_TEXT_NONE;


}

// call this whenever we want to go back to just having a single gamefile template
void reset_menu_templates(void)
{

//  setup_system_template();

}


void display_menu_1(void)
{

 al_set_target_bitmap(al_get_backbuffer(display));
 al_set_clipping_rectangle(0, 0, settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);

 run_menu_stripes(1);

}


enum
{
SMS_PLAYERS_2,
SMS_PLAYERS_3,
SMS_PLAYERS_4,
SMS_CORES_FEW,
SMS_CORES_SOME,
SMS_CORES_MANY,
SMS_CORES_HEAPS,
SMS_SIZE_SMALL,
SMS_SIZE_MEDIUM,
SMS_SIZE_LARGE,
SMS_SIZE_HUGE,
SMS_COMMAND_MODE_AUTO,
SMS_COMMAND_MODE_COMMAND,
SMS_DATA_300,
SMS_DATA_600,
SMS_DATA_900,
SMS_DATA_1200,
SMS_HARD_1,
SMS_ADVANCED_1,
SMS_ADVANCED_2,
SMS_HARD_ADVANCED_1,
SMS_STRINGS
};


const char *setup_menu_string [SMS_STRINGS] =
{
"2", // SMS_PLAYERS_2,
"3", // SMS_PLAYERS_3,
"4", // SMS_PLAYERS_4,
"16", // SMS_CORES_FEW,
"32", // SMS_CORES_SOME,
"64", // SMS_CORES_MANY,
"128", // SMS_CORES_HEAPS,
"small", // SMS_SIZE_SMALL,
"medium", // SMS_SIZE_MEDIUM,
"large", // SMS_SIZE_LARGE,
"huge", // SMS_SIZE_HUGE,
"auto", // SMS_COMMAND_MODE_AUTO,
"command", // SMS_COMMAND_MODE_COMMAND,
"300", // SMS_DATA_300,
"600", // SMS_DATA_600,
"900", // SMS_DATA_900,
"1200", // SMS_DATA_1200,
"Your opponents' processes are stronger and more plentiful.", // SMS_HARD_1
"You cannot give commands.", // SMS_ADVANCED_1
" Your processes must be coded to act by themselves.", // SMS_ADVANCED_2
"Hard mode and autonomous mode at the same time.", // SMS_HARD_ADVANCED_1
};


// when this is called, the editor or template windows may have already been drawn on the right side
void display_menu_2(void)
{


 al_set_target_bitmap(al_get_backbuffer(display));
 al_set_clipping_rectangle(0, 0, settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
 reset_i_buttons();

 int i, j, y1, y2;

   for (i = 0; i < mstate.elements; i ++)
   {
    y1 = menu_element[i].y1 - mstate.window_pos;
    y2 = menu_element[i].y2 - mstate.window_pos;
#define MENU_HEADING_OFFSET (-15)
    if (menu_element[i].type == EL_TYPE_HEADING)
    {
      add_menu_button(menu_element[i].x1 + 1 + MENU_HEADING_OFFSET, y1 + 1, menu_element[i].x2 - 1 + MENU_HEADING_OFFSET, y2 - 1, colours.base_trans [COL_TURQUOISE] [SHADE_MED] [TRANS_MED], 4, 9);
      add_menu_string(menu_element[i].x1 + 15 + MENU_HEADING_OFFSET, y1 + 22, &colours.base [COL_GREY] [SHADE_MAX], ALLEGRO_ALIGN_LEFT, FONT_SQUARE_LARGE, menu_list[menu_element[i].list_index].name);
      continue;
    }

#define SELECT_BUTTON_W scaleUI_x(FONT_SQUARE,60)
#define SELECT_BUTTON_MIDDLE (SELECT_BUTTON_W/2)
#define SELECT_BUTTON_GAP 5
#define SELECT_BUTTON_H scaleUI_y(FONT_SQUARE,15)
    if (menu_element[i].type == EL_TYPE_SELECT)
    {
     add_menu_string(menu_element[i].x1 + 15, y1 + 8, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_LEFT, FONT_SQUARE_LARGE, menu_list[menu_element[i].list_index].name);
    	int select_buttons = menu_list[menu_element[i].list_index].start_value - menu_list[menu_element[i].list_index].action + 1;
					int sb_y = y1 + 25;
    	for (j = 0; j < select_buttons; j++)
					{
						int sb_x = menu_element[i].x1 + 30 + (j*(SELECT_BUTTON_W+SELECT_BUTTON_GAP));
						int sb_col = COL_BLUE;
						int sb_shade = SHADE_MED;
						if (ex_control.mouse_x_pixels >= sb_x
       && ex_control.mouse_x_pixels <= sb_x + SELECT_BUTTON_W
       && ex_control.mouse_y_pixels >= sb_y
       && ex_control.mouse_y_pixels <= sb_y + SELECT_BUTTON_H)
      {
      	sb_shade = SHADE_HIGH;
      }
      add_menu_button(sb_x, sb_y,
																						sb_x + SELECT_BUTTON_W, sb_y + SELECT_BUTTON_H,
																						colours.base_trans [sb_col] [sb_shade] [TRANS_THICK], 4, 2);

						switch(menu_element[i].list_index)
						{
							case EL_SETUP_PLAYERS:
        add_menu_string(sb_x + SELECT_BUTTON_MIDDLE, sb_y + 3, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, setup_menu_string [SMS_PLAYERS_2 + j]);
        if (w_init.players == j + 2)
         add_menu_button(sb_x - 2, sb_y - 2,
				   																		sb_x + SELECT_BUTTON_W + 2, sb_y + SELECT_BUTTON_H + 2,
							   															colours.base_trans [COL_CYAN] [SHADE_HIGH] [TRANS_MED], 6, 3);
        break;
							case EL_SETUP_CORES:
        add_menu_string(sb_x + SELECT_BUTTON_MIDDLE, sb_y + 3, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, setup_menu_string [SMS_CORES_FEW + j]);
        if (w_init.core_setting == j)
         add_menu_button(sb_x - 2, sb_y - 2,
				   																		sb_x + SELECT_BUTTON_W + 2, sb_y + SELECT_BUTTON_H + 2,
							   															colours.base_trans [COL_CYAN] [SHADE_HIGH] [TRANS_MED], 6, 3);
        break;
							case EL_SETUP_DATA:
        add_menu_string(sb_x + SELECT_BUTTON_MIDDLE, sb_y + 3, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, setup_menu_string [SMS_DATA_300 + j]);
        if (w_init.starting_data_setting [0] == j) // assume that settings for all players are the same
         add_menu_button(sb_x - 2, sb_y - 2,
				   																		sb_x + SELECT_BUTTON_W + 2, sb_y + SELECT_BUTTON_H + 2,
							   															colours.base_trans [COL_CYAN] [SHADE_HIGH] [TRANS_MED], 6, 3);
        break;
							case EL_SETUP_MAP_SIZE:
        add_menu_string(sb_x + SELECT_BUTTON_MIDDLE, sb_y + 3, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, setup_menu_string [SMS_SIZE_SMALL + j]);
        if (w_init.size_setting == j)
         add_menu_button(sb_x - 2, sb_y - 2,
				   																		sb_x + SELECT_BUTTON_W + 2, sb_y + SELECT_BUTTON_H + 2,
							   															colours.base_trans [COL_CYAN] [SHADE_HIGH] [TRANS_MED], 6, 3);
        break;
							case EL_SETUP_COMMAND_MODE:
        add_menu_string(sb_x + SELECT_BUTTON_MIDDLE, sb_y + 3, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, setup_menu_string [SMS_COMMAND_MODE_AUTO + j]);
        if (w_init.command_mode == j)
         add_menu_button(sb_x - 2, sb_y - 2,
				   																		sb_x + SELECT_BUTTON_W + 2, sb_y + SELECT_BUTTON_H + 2,
							   															colours.base_trans [COL_CYAN] [SHADE_HIGH] [TRANS_MED], 6, 3);
        break;
						}
					}
					continue;
    }

/*
								if (menu_list[menu_element[i].list_index].action == EL_ACTION_START_MISSION)
								{
									if (missions.locked [menu_list[menu_element[i].list_index].start_value] == 1)
									{
          add_menu_button(menu_element[i].x1 + 1, y1 + 1, menu_element[i].x2 - 1, y2 - 1, colours.base_trans [COL_TURQUOISE] [SHADE_LOW] [TRANS_FAINT], 6, 3);
          add_menu_string(menu_element[i].x1 + 15, y1 + 22, &colours.base [COL_BLUE] [SHADE_MED], ALLEGRO_ALIGN_LEFT, FONT_SQUARE_LARGE, locked_string);
									}
           else
											{
            if (menu_element[i].highlight)
             add_menu_button(menu_element[i].x1 + 1, y1 + 1, menu_element[i].x2 - 1, y2 - 1, colours.base_trans [COL_BLUE] [SHADE_HIGH] [TRANS_THICK], 6, 3);
              else
               add_menu_button(menu_element[i].x1 + 1, y1 + 1, menu_element[i].x2 - 1, y2 - 1, colours.base_trans [COL_BLUE] [SHADE_MED] [TRANS_THICK], 6, 3);
            add_menu_string(menu_element[i].x1 + 15, y1 + 22, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_LEFT, FONT_SQUARE_LARGE, menu_list[menu_element[i].list_index].name);


											}


									continue;
								}
*/


        switch(menu_element[i].list_index)
        {
         case EL_SETUP_PLAYER_NAME_0:
         case EL_SETUP_PLAYER_NAME_1:
         case EL_SETUP_PLAYER_NAME_2:
         case EL_SETUP_PLAYER_NAME_3:
         	if (w_init.players <= menu_element[i].list_index - EL_SETUP_PLAYER_NAME_0)
										{
           add_menu_button(menu_element[i].x1 + 1, y1 + 1, menu_element[i].x2 - 1, y2 - 1, colours.base_trans [COL_BLUE] [SHADE_LOW] [TRANS_MED], 9, 4);
           continue;
										}
    	     add_menu_string(menu_element[i].x1 + scaleUI_x(FONT_SQUARE,145), y1 + 22, &colours.base [COL_BLUE] [SHADE_MAX], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, w_init.player_name [menu_element[i].list_index - EL_SETUP_PLAYER_NAME_0]);
    	     add_menu_string(menu_element[i].x1 + scaleUI_x(FONT_SQUARE,134), y1 + 21, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, "[             ]");
          if (menu_element[i].highlight)
           add_menu_button(menu_element[i].x1 + 1, y1 + 1, menu_element[i].x2 - 1, y2 - 1, colours.base_trans [COL_BLUE] [SHADE_HIGH] [TRANS_THICK], 9, 4);
            else
             add_menu_button(menu_element[i].x1 + 1, y1 + 1, menu_element[i].x2 - 1, y2 - 1, colours.base_trans [COL_BLUE] [SHADE_MED] [TRANS_THICK], 9, 4);
          add_menu_string(menu_element[i].x1 + 15, y1 + 22, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, menu_list[menu_element[i].list_index].name);
          continue;
         case EL_SETUP_CODE:
    	     add_menu_string(menu_element[i].x1 + scaleUI_x(FONT_SQUARE,145), y1 + 22, &colours.base [COL_BLUE] [SHADE_MAX], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, mstate.map_code_string);
    	     add_menu_string(menu_element[i].x1 + scaleUI_x(FONT_SQUARE,134), y1 + 21, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, "[        ]");
         	break;
         case EL_MAIN_START_GAME_HARD:
          if (menu_element[i].highlight)
          {
    	      add_menu_string(menu_element[i].x2 + 20, y1 + 17, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, setup_menu_string [SMS_HARD_1]);
//    	      add_menu_string(menu_element[i].x2 + 20, y1 + 30, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, setup_menu_string [SMS_ADVANCED_2]);
          }
										break;
         case EL_MAIN_START_GAME_ADVANCED:
          if (menu_element[i].highlight)
          {
    	      add_menu_string(menu_element[i].x2 + 20, y1 + 17, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, setup_menu_string [SMS_ADVANCED_1]);
    	      add_menu_string(menu_element[i].x2 + 20, y1 + 17 + scaleUI_y(FONT_SQUARE, 13), &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, setup_menu_string [SMS_ADVANCED_2]);
          }
										break;
         case EL_MAIN_START_GAME_ADVANCED_HARD:
          if (menu_element[i].highlight)
          {
    	      add_menu_string(menu_element[i].x2 + 20, y1 + 17, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, setup_menu_string [SMS_HARD_ADVANCED_1]);
//    	      add_menu_string(menu_element[i].x2 + 20, y1 + 30, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, setup_menu_string [SMS_ADVANCED_2]);
          }
										break;
        }

    if (menu_element[i].highlight)
     add_menu_button(menu_element[i].x1 + 1, y1 + 1, menu_element[i].x2 - 1, y2 - 1, colours.base_trans [COL_BLUE] [SHADE_HIGH] [TRANS_THICK], 9, 4);
      else
       add_menu_button(menu_element[i].x1 + 1, y1 + 1, menu_element[i].x2 - 1, y2 - 1, colours.base_trans [COL_BLUE] [SHADE_MED] [TRANS_THICK], 9, 4);

    add_menu_string(menu_element[i].x1 + 15, y1 + 22, &colours.base [COL_GREY] [SHADE_HIGH], ALLEGRO_ALIGN_LEFT, FONT_SQUARE_LARGE, menu_list[menu_element[i].list_index].name);

   }

   draw_button_buffer();
   draw_menu_strings();

#define MAP_SIZE_FACTOR 4

   if (mstate.menu_index == MENU_SETUP)
			{
				float base_x = MENU_X + MENU_W + 30;
				float base_y = 200;
				al_draw_filled_rectangle(base_x,
																													base_y,
																													base_x + map_init.map_size_blocks * MAP_SIZE_FACTOR,
																													base_y + map_init.map_size_blocks * MAP_SIZE_FACTOR,
																													colours.base [COL_GREY] [SHADE_MIN]);
					for (i = 0; i < map_init.players; i ++)
					{
				  al_draw_filled_rectangle(base_x + map_init.spawn_position[i].x * MAP_SIZE_FACTOR - 3,
																													  base_y + map_init.spawn_position[i].y * MAP_SIZE_FACTOR - 3,
                               base_x + map_init.spawn_position[i].x * MAP_SIZE_FACTOR + 3,
																													  base_y + map_init.spawn_position[i].y * MAP_SIZE_FACTOR + 3,
																													  colours.base [i] [SHADE_HIGH]);
					}
					for (i = 0; i < map_init.data_wells; i ++)
					{
				  al_draw_filled_rectangle(base_x + map_init.data_well_position[i].x * MAP_SIZE_FACTOR - 2,
																													  base_y + map_init.data_well_position[i].y * MAP_SIZE_FACTOR - 2,
                               base_x + map_init.data_well_position[i].x * MAP_SIZE_FACTOR + 2,
																													  base_y + map_init.data_well_position[i].y * MAP_SIZE_FACTOR + 2,
																													  colours.base [COL_YELLOW] [SHADE_HIGH]);

					}
			}


// finally, if a text box is open need to draw it over the top:
 switch(mstate.menu_text_box)
 {
 	case MENU_TEXT_PLAYER_0_NAME:
 	case MENU_TEXT_PLAYER_1_NAME:
 	case MENU_TEXT_PLAYER_2_NAME:
 	case MENU_TEXT_PLAYER_3_NAME:
   {
#define NAME_BOX_X (MENU_X - 20)
#define NAME_BOX_W scaleUI_x(FONT_SQUARE,250)
#define NAME_BOX_Y 245
#define NAME_BOX_H scaleUI_y(FONT_SQUARE,55)
#define NAME_TEXT_BOX_X 50
#define NAME_TEXT_BOX_W scaleUI_x(FONT_SQUARE,90)
#define NAME_TEXT_BOX_Y 25
#define NAME_TEXT_BOX_H scaleUI_y(FONT_SQUARE,15)
     int naming_player = mstate.menu_text_box - MENU_TEXT_PLAYER_0_NAME;
#ifdef SANITY_CHECK
    if (naming_player < 0
     || naming_player >= PLAYERS)
    {
     fprintf(stdout, "\nError: s_menu.c:display_menu_2(): naming_player out of bounds (%i)", naming_player);
     error_call();
    }
#endif
     char* naming_player_name = w_init.player_name[naming_player];
     al_draw_filled_rectangle(NAME_BOX_X, NAME_BOX_Y, NAME_BOX_X + NAME_BOX_W, NAME_BOX_Y + NAME_BOX_H, colours.base [COL_BLUE] [SHADE_LOW]);
     al_draw_rectangle(NAME_BOX_X, NAME_BOX_Y, NAME_BOX_X + NAME_BOX_W, NAME_BOX_Y + NAME_BOX_H, colours.base [COL_BLUE] [SHADE_MAX], 1);
     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], NAME_BOX_X + 10, NAME_BOX_Y + 6, ALLEGRO_ALIGN_LEFT, "Player %i name", naming_player);
// draw box for text to appear in
     al_draw_filled_rectangle(NAME_BOX_X + NAME_TEXT_BOX_X, NAME_BOX_Y + NAME_TEXT_BOX_Y, NAME_BOX_X + NAME_TEXT_BOX_X + NAME_TEXT_BOX_W, NAME_BOX_Y + NAME_TEXT_BOX_Y + NAME_TEXT_BOX_H, colours.base [COL_BLUE] [SHADE_MIN]);
     al_draw_rectangle(NAME_BOX_X + NAME_TEXT_BOX_X, NAME_BOX_Y + NAME_TEXT_BOX_Y, NAME_BOX_X + NAME_TEXT_BOX_X + NAME_TEXT_BOX_W, NAME_BOX_Y + NAME_TEXT_BOX_Y + NAME_TEXT_BOX_H, colours.base [COL_BLUE] [SHADE_HIGH], 1);

     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], NAME_BOX_X + NAME_TEXT_BOX_X + 3, NAME_BOX_Y + NAME_TEXT_BOX_Y + 3, ALLEGRO_ALIGN_LEFT, "%s", naming_player_name);
// TO DO: cursor flash
     int cursor_x = NAME_BOX_X + NAME_TEXT_BOX_X + 3 + (strlen(naming_player_name) * font[FONT_SQUARE].width);
     int cursor_y = NAME_BOX_Y + NAME_TEXT_BOX_Y + 1;
     al_draw_filled_rectangle(cursor_x, cursor_y, cursor_x + scaleUI_x(FONT_SQUARE,2), cursor_y + scaleUI_y(FONT_SQUARE,12), colours.base [COL_GREY] [SHADE_MAX]);
  }
  break; // end player naming
  case MENU_TEXT_MAP_CODE:
  {
     al_draw_filled_rectangle(NAME_BOX_X, NAME_BOX_Y, NAME_BOX_X + NAME_BOX_W, NAME_BOX_Y + NAME_BOX_H, colours.base [COL_BLUE] [SHADE_LOW]);
     al_draw_rectangle(NAME_BOX_X, NAME_BOX_Y, NAME_BOX_X + NAME_BOX_W, NAME_BOX_Y + NAME_BOX_H, colours.base [COL_BLUE] [SHADE_MAX], 1);
     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], NAME_BOX_X + 10, NAME_BOX_Y + 6, ALLEGRO_ALIGN_LEFT, "Enter map code");
// draw box for text to appear in
     al_draw_filled_rectangle(NAME_BOX_X + NAME_TEXT_BOX_X, NAME_BOX_Y + NAME_TEXT_BOX_Y, NAME_BOX_X + NAME_TEXT_BOX_X + NAME_TEXT_BOX_W, NAME_BOX_Y + NAME_TEXT_BOX_Y + NAME_TEXT_BOX_H, colours.base [COL_BLUE] [SHADE_MIN]);
     al_draw_rectangle(NAME_BOX_X + NAME_TEXT_BOX_X, NAME_BOX_Y + NAME_TEXT_BOX_Y, NAME_BOX_X + NAME_TEXT_BOX_X + NAME_TEXT_BOX_W, NAME_BOX_Y + NAME_TEXT_BOX_Y + NAME_TEXT_BOX_H, colours.base [COL_BLUE] [SHADE_HIGH], 1);

     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], NAME_BOX_X + NAME_TEXT_BOX_X + 3, NAME_BOX_Y + NAME_TEXT_BOX_Y + 3, ALLEGRO_ALIGN_LEFT, "%s", mstate.map_code_string_temp);
// TO DO: cursor flash
     int cursor_x = NAME_BOX_X + NAME_TEXT_BOX_X + 3 + (strlen(mstate.map_code_string_temp) * font[FONT_SQUARE].width);
     int cursor_y = NAME_BOX_Y + NAME_TEXT_BOX_Y + 1;
     al_draw_filled_rectangle(cursor_x, cursor_y, cursor_x + scaleUI_x(FONT_SQUARE,2), cursor_y + scaleUI_y(FONT_SQUARE,12), colours.base [COL_GREY] [SHADE_MAX]);
  }
  break;
 }

// if (settings.option [OPTION_SPECIAL_CURSOR])
  draw_mouse_cursor();
 al_flip_display();
 al_set_target_bitmap(al_get_backbuffer(display));

}

void draw_menu_button(float xa, float ya, float xb, float yb, ALLEGRO_COLOR button_colour)
{

#define BUTTON_NOTCH_Y 6
//#define BUTTON_NOTCH_X (BUTTON_NOTCH_Y / 3.0)
#define BUTTON_NOTCH_X (BUTTON_NOTCH_Y)
     static ALLEGRO_VERTEX button_fan [8];
     int b = 0;

     button_fan[b].x = xa + BUTTON_NOTCH_X;
     button_fan[b].y = ya;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xb - BUTTON_NOTCH_X*2;
     button_fan[b].y = ya;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xb;
     button_fan[b].y = ya + BUTTON_NOTCH_Y*2;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xb;
     button_fan[b].y = yb - BUTTON_NOTCH_Y;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xb - BUTTON_NOTCH_X;
     button_fan[b].y = yb;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xa + BUTTON_NOTCH_X*2;
     button_fan[b].y = yb;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xa;
     button_fan[b].y = yb - BUTTON_NOTCH_Y*2;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xa;
     button_fan[b].y = ya + BUTTON_NOTCH_Y;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;


/*
     button_fan[b].x = xa;
     button_fan[b].y = ya;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xb - BUTTON_NOTCH_X;
     button_fan[b].y = ya;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xb;
     button_fan[b].y = ya + BUTTON_NOTCH_Y;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xb;
     button_fan[b].y = yb;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xa + BUTTON_NOTCH_X;
     button_fan[b].y = yb;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xa;
     button_fan[b].y = yb - BUTTON_NOTCH_Y;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
*/
/*
     button_fan[b].x = xa + BUTTON_NOTCH_X;
     button_fan[b].y = ya;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xb;
     button_fan[b].y = ya;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xb;
     button_fan[b].y = yb - BUTTON_NOTCH_Y;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xb - BUTTON_NOTCH_X;
     button_fan[b].y = yb;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xa;
     button_fan[b].y = yb;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;
     button_fan[b].x = xa;
     button_fan[b].y = ya + BUTTON_NOTCH_Y;
     button_fan[b].z = 0;
     button_fan[b].color = button_colour;
     b++;*/


//  al_draw_prim(fan_buffer, NULL, NULL, fan_vertex_list, fan_index[i].vertices, ALLEGRO_PRIM_TRIANGLE_FAN);
  al_draw_prim(button_fan, NULL, NULL, 0, b, ALLEGRO_PRIM_TRIANGLE_FAN);





}



// This function should only be called once, from the startup routines in m_main
void start_menus(void)
{

 game.phase = GAME_PHASE_MENU;

 mstate.menu_type = MENU_TYPE_NORMAL;

 mstate.stripe_group_col = 0;
 mstate.stripe_group_shade = SHADE_HIGH;
 mstate.stripe_group_time = 30;
 mstate.stripe_next_group_count = 200;
 mstate.stripe_next_stripe = 40;
 int i;

 for (i = 0; i < MENU_STRIPES; i ++)
	{
  mstate.stripe_exists [i] = 0;
  mstate.stripe_x [i] = 0;
  mstate.stripe_size [i] = 5;
  mstate.stripe_col [i] = 3;
  mstate.stripe_shade [i] = SHADE_MAX;
	}

	mstate.stripe_al_col [0] = al_map_rgb(10, 60, 110);
	mstate.stripe_al_col [1] = al_map_rgb(10, 40, 120);
	mstate.stripe_al_col [2] = al_map_rgb(30, 30, 90);
	mstate.stripe_al_col [3] = al_map_rgb(30, 15, 100);
	mstate.stripe_al_col [4] = al_map_rgb(50, 10, 90);
	mstate.stripe_al_col [5] = al_map_rgb(50, 5, 140);
	mstate.stripe_al_col [6] = al_map_rgb(10, 70, 100);
	mstate.stripe_al_col [7] = al_map_rgb(10, 30, 120);
	mstate.stripe_al_col [8] = al_map_rgb(40, 30, 110);
	mstate.stripe_al_col [9] = al_map_rgb(30, 23, 70);
	mstate.stripe_al_col [10] = al_map_rgb(30, 80, 120);
	mstate.stripe_al_col [11] = al_map_rgb(120, 10, 80); // rare colour

 for (i = 0; i < settings.option [OPTION_WINDOW_W]; i ++)
 {
 	run_menu_stripes(0);
 }

 run_intro_screen();

 init_w_init(); // initialises world_init values for the setup menu

 fix_map_code();

 open_menu(MENU_MAIN);

 menu_loop();

}

void init_w_init(void) // also called from s_mission.c
{

	w_init.players = 2;
	w_init.core_setting = 2;
//	w_init.starting_data_setting = 0;
	w_init.game_seed = 0;
//	w_init.data_wells = 0;

	w_init.size_setting = 2;
	w_init.command_mode = COMMAND_MODE_AUTO;
 fix_w_init_size();

	int i;

	for (i = 0; i < PLAYERS; i ++)
	{
		sprintf(w_init.player_name [i], "Player %i", i);
		w_init.starting_data_setting [i] = 0;
//		w_init.player_starting_data [i] = (w_init.starting_data_setting + 1) * 300; // may be changed by some missions
	}

// this function doesn't initialise everything - it leaves some things (like player spawn positions) that must be initialised when the game is being started.

}

static void fix_map_code(void)
{

	mstate.map_code_string [MAP_CODE_PLAYERS] = 'A' + w_init.players - 2;
	mstate.map_code_string [MAP_CODE_SIZE] = 'A' + w_init.size_setting;
	mstate.map_code_string [MAP_CODE_CORES] = 'A' + w_init.core_setting;
	mstate.map_code_string [MAP_CODE_DATA] = 'A' + w_init.starting_data_setting [0]; // assume all are the same
	mstate.map_code_string [MAP_CODE_SEED_0] = '0' + w_init.game_seed / 100;
	mstate.map_code_string [MAP_CODE_SEED_1] = '0' + (w_init.game_seed / 10) % 10;
	mstate.map_code_string [MAP_CODE_SEED_2] = '0' + (w_init.game_seed) % 10;

	mstate.map_code_string [MAP_CODE_LENGTH] = '\0';

}

// this function handles timing etc for the menu interface.
// it also calls game functions etc if the game is started from a menu
// it can co-exist with the template and editor windows
void menu_loop(void)
{


 al_flush_event_queue(event_queue);
 al_flush_event_queue(fps_queue);

 ALLEGRO_EVENT ev;

 do
 {

  rand(); // change the rand state (this prevents the music being predictable, as it's seeded by rand())

  display_menu_1(); // prepares screen for menu and possible editor/templates to be written

  run_menu();

  run_menu_input(); // note that this function can result in a change in menu

  display_menu_2(); // finishes drawing menu stuff

//  check_sound_queue();

  al_wait_for_event(event_queue, &ev);

 } while (TRUE);


}

// this function does basic maintenance stuff for the current menu
void run_menu(void)
{

 int i;

 for (i = 0; i < mstate.elements; i ++)
 {
  menu_element[i].highlight = 0;
 }

}

void run_menu_input(void)
{

 get_ex_control(0, 0);

 int i;
 int mouse_x = ex_control.mouse_x_pixels;
// int mouse_y = ex_control.mouse_y_pixels; // ignores window_pos
 int abs_mouse_y = ex_control.mouse_y_pixels + mstate.window_pos; // absolute mouse_y
 int just_pressed = (ex_control.mb_press [0] == BUTTON_JUST_PRESSED);
// int rmb_just_pressed = (ex_control.mb_press [1] == BUTTON_JUST_PRESSED);
/*
 if (just_pressed == 1)
	{
		static int sample_play;
  play_interface_sound(sample_play, TONE_1C);
  sample_play++;
  sample_play %= 3;
	}*/


// fprintf(stdout, "\n(%i: %i, my %i amy %i)", mstate.h, mstate.window_pos, mouse_y, abs_mouse_y);

 switch(mstate.menu_text_box)
 {
	 case MENU_TEXT_PLAYER_0_NAME:
 	case MENU_TEXT_PLAYER_1_NAME:
 	case MENU_TEXT_PLAYER_2_NAME:
 	case MENU_TEXT_PLAYER_3_NAME:
   if (accept_text_box_input(TEXT_BOX_PLAYER_NAME) == 1
    || just_pressed == 1)
     mstate.menu_text_box = MENU_TEXT_NONE;
   return;
  case MENU_TEXT_MAP_CODE:
   if (accept_text_box_input(TEXT_BOX_MAP_CODE) == 1
    || just_pressed == 1)
   {
   	 enter_map_code(); // derives map information from map code
     reset_map_for_menu();
     mstate.menu_text_box = MENU_TEXT_NONE;
   }
   return;


 }

// if (mstate.use_scrollbar)
//  && mstate.menu_type != MENU_TYPE_PREGAME) // pregame menu doesn't have a scrollbar (but use_scrollbar is retained from the previous menu in case we return there)
//  run_slider(&mstate.mscrollbar, 0, 0);

/*

// check for the mouse pointer being in the editor/template window:
  if (settings.edit_window != EDIT_WINDOW_CLOSED
   && mouse_x >= settings.editor_x_split)
  {
   if (just_pressed)
    settings.keyboard_capture = INPUT_EDITOR;
   return;
  }
*/
/*
 if (mstate.menu_type == MENU_TYPE_PREGAME)
 {
  for (i = 0; i < PREGAME_BUTTONS; i ++)
  {
   if (mouse_x >= mstate.pregame_button_x [i]
    && mouse_x <= mstate.pregame_button_x [i] + mstate.pregame_button_w [i]
    && mouse_y >= mstate.pregame_button_y [i] // use mouse_y not abs_mouse_y because the menu that opened the pregame menu may have been scrolled down
    && mouse_y <= mstate.pregame_button_y [i] + mstate.pregame_button_h [i])
   {
    mstate.pregame_button_highlight [i] = 1;
    if (just_pressed)
    {
     switch(i)
     {
      case PREGAME_BUTTON_GO:
// assume that derive_world_init_from_menu() was called when the pregame menu was opened, so world_init will have been set up
       if (!setup_world_programs_from_templates()) // this function initialises programs as well as copying from templates. Must be called before run_game().
        return; // if setup_world_programs_from_templates() fails it writes a failure message to the mlog
       run_game_from_menu(1);
       return; // note return, not break!
      case PREGAME_BUTTON_BACK:
       mstate.menu_type = MENU_TYPE_NORMAL; // should just go back to the setup menu or similar
       game.phase = GAME_PHASE_MENU;
       reset_menu_templates(); // TO DO: when new menu types are added, this call may need to identify the menu we are returning to
       return; // note return, not break!
     }
    }
   }
    else
     mstate.pregame_button_highlight [i] = 0;
  } // end for i loop
  return;
 } // end MENU_TYPE_PREGAME
*/

 for (i = 0; i < mstate.elements; i ++)
 {

//   if (menu_element[i].slider_index != -1
//    && !menu_element[i].fixed)
//    run_slider(&element_slider [menu_element[i].slider_index], 0, mstate.window_pos);

  if (mouse_x > menu_element[i].x1
   && mouse_x < menu_element[i].x2
   && abs_mouse_y > menu_element[i].y1
   && abs_mouse_y < menu_element[i].y2)
  {
   menu_element[i].highlight = 1;
   if (just_pressed)
   {
    if (menu_list[menu_element[i].list_index].type == EL_TYPE_ACTION)
    {
     switch(menu_list[menu_element[i].list_index].action)
     {
      case EL_ACTION_QUIT:
// TO DO: think about what happens here if the user has unsaved source tabs open in the editor.
       safe_exit(0);
       break;
      case EL_ACTION_STORY:
//      case EL_ACTION_MISSION:
       play_interface_sound(SAMPLE_BLIP1, TONE_2C);
       // first load gamefile (and setup system program for use if appropriate)
//       setup_templates_for_mission_menu();
//       open_menu(MENU_MISSIONS);
//       init_story(STORY_TYPE_NORMAL);
       enter_story_mode(STORY_TYPE_NORMAL);
       break;
      case EL_ACTION_STORY_ADVANCED:
//      case EL_ACTION_ADVANCED_MISSION:
       play_interface_sound(SAMPLE_BLIP1, TONE_2D);
//       init_story(STORY_TYPE_ADVANCED);
       enter_story_mode(STORY_TYPE_ADVANCED); // need to set to autonomous mode at some point!
       // first load gamefile (and setup system program for use if appropriate)
//       setup_templates_for_mission_menu();
//       setup_templates_for_advanced_mission_menu();
//       open_menu(MENU_ADVANCED_MISSIONS);
       break;
      case EL_ACTION_STORY_HARD:
       play_interface_sound(SAMPLE_BLIP1, TONE_2C);
       enter_story_mode(STORY_TYPE_HARD);
       break;
      case EL_ACTION_STORY_ADVANCED_HARD:
       play_interface_sound(SAMPLE_BLIP1, TONE_2D);
       enter_story_mode(STORY_TYPE_ADVANCED_HARD); // need to set to autonomous mode at some point!
       break;
      case EL_ACTION_TUTORIAL:
       play_interface_sound(SAMPLE_BLIP1, TONE_2E);
       open_menu(MENU_TUTORIAL);
       break;
      case EL_ACTION_START_GAME_FROM_SETUP:
      	{
       play_interface_sound(SAMPLE_BLIP1, TONE_2C);
       game.type = GAME_TYPE_BASIC; // i.e. not playing a mission
       game.story_type = STORY_TYPE_NORMAL; // nothing special
//       w_init.story_area = (w_init.game_seed % (STORY_AREAS - 1)) + 1; // AREA_BLUE to AREA_RED (not AREA_TUTORIAL)
// TO DO: allow custom games to be in any area
       game.area_index = AREA_BLUE; // these game values are just used to generate the music. Should probably fix them.
       w_init.story_area = AREA_BLUE;
       game.region_in_area_index = -1; // means to use random music

      	int player_base_cols [PLAYERS] = {TEAM_COL_BLUE,1,2,3}; // index in base_proc_col array
	      int player_packet_cols [PLAYERS] = {PACKET_COL_YELLOW_ORANGE,1,2,3}; // index in base_packet_colours array and similar interface array

       set_game_colours(BACK_COLS_BLUE, // index in back_and_hex_colours array
																				    BACK_COLS_BLUE, // index in back_and_hex_colours array
																				    w_init.players, // players in game
																				    player_base_cols, // index in base_proc_col array
																				    player_packet_cols); // index in base_packet_colours array and similar interface array

       new_world_from_world_init();
       generate_random_map(w_init.story_area, w_init.map_size_blocks, w_init.players, w_init.game_seed);
//       game.area_index = AREA_BLUE; // these game values are just used to generate the music. Should probably fix them.
       game.region_in_area_index = 0;
       reset_log();
       open_template(0, 0);
       start_world();
       run_game_from_menu();
      	}
       break;
/*      case EL_ACTION_SAVE_GAMEFILE:
       play_interface_sound(SAMPLE_BLIP1, TONE_2C);
// this button appears in the setup menu
// it generates a gamefile containing all of the current menu and w_init settings, as well as the system program's bcode
// when loaded (from main menu) it lets the player choose which player they are, then goes straight to pregame
       derive_world_init_from_menu();
       if (save_gamefile()) // in f_game.c
       {
        play_interface_sound(SAMPLE_BLIP1, TONE_2G);
        open_menu(MENU_MAIN); // don't go back to main menu if save failed
       }
         else
          play_interface_sound(SAMPLE_BLIP1, TONE_1E);
       break;*/

      case EL_ACTION_LOAD_GAMEFILE:
/*       play_interface_sound(SAMPLE_BLIP1, TONE_3C);
       if (load_gamefile() // first loads a gamefile
        && use_sysfile_from_template()) // then tries to use the system file that load_gamefile() should have loaded into template 0
       {
        play_interface_sound(SAMPLE_BLIP1, TONE_3C);
//        setup_templates_for_game_start();
//        mstate.menu_templ_state = MENU_TEMPL_STATE_PREGAME;
//        open_pregame_menu(); // this changes the menu type but leaves the elements unchanged - they will be used later by derive_world_init_from_menu
        run_game_from_menu(1, -1); // 1 means needs to initialise (because not loading from saved game); -1 means not playing a mission
       }
        else
        {
         play_interface_sound(SAMPLE_BLIP1, TONE_1E);
//         reset_menu_templates();
        }*/
       break;

      case EL_ACTION_BACK_TO_START:
       play_interface_sound(SAMPLE_BLIP1, TONE_2E);
       open_menu(MENU_MAIN);
       break;

      case EL_ACTION_SETUP_GAME:
       play_interface_sound(SAMPLE_BLIP1, TONE_3C);
       open_menu(MENU_SETUP);
       break;


/*
      case EL_ACTION_USE_SYSFILE:
       play_interface_sound(SAMPLE_BLIP1, TONE_2C);
       if (use_sysfile_from_template()) // in s_setup.c. If the gamefile results in a system program, this sets w.system_program.active to 1
       {
// use_sysfile_from_template() will have called derive_program_properties_from_bcode(), so w_pre_init will be usable.
// next step is to use the now filled-in w_pre_init as the basis of a setup menu:
        open_menu(MENU_SETUP);
       } // on failure, use_sysfile_from_template() writes error message to mlog. We don't need to otherwise deal with it failing.
       break;*/
/*
      case EL_ACTION_LOAD:
       play_interface_sound(SAMPLE_BLIP1, TONE_2C);
       if (load_game())
       {
        play_interface_sound(SAMPLE_BLIP1, TONE_2G);
        run_game_from_menu(0, w.playing_mission); // 0 means don't initialise world as load_game() has already done so. load_game() has also set w.playing_mission.
       }
        else
        {
         play_interface_sound(SAMPLE_BLIP1, TONE_1E);
         reset_menu_templates(); // load_game may have partially loaded templates before exiting
        }
       break;*/

      case EL_ACTION_NAME:
       play_interface_sound(SAMPLE_BLIP1, TONE_2A);
       start_text_input_box(TEXT_BOX_PLAYER_NAME, w_init.player_name[menu_list[menu_element[i].list_index].start_value], PLAYER_NAME_LENGTH);
       mstate.menu_text_box = MENU_TEXT_PLAYER_0_NAME + menu_list[menu_element[i].list_index].start_value;
       break;

      case EL_ACTION_ENTER_CODE:
       play_interface_sound(SAMPLE_BLIP1, TONE_2A);
       start_text_input_box(TEXT_BOX_MAP_CODE, mstate.map_code_string_temp, MAP_CODE_LENGTH+1);
       mstate.menu_text_box = MENU_TEXT_MAP_CODE;
       break;

      case EL_ACTION_RANDOMISE_CODE:
       play_interface_sound(SAMPLE_BLIP1, TONE_2A);
							w_init.game_seed = grand(1000);
							fix_map_code();
       reset_map_for_menu();
							break;

      case EL_ACTION_START_MISSION:
/*
      	if (missions.locked [menu_list[menu_element[i].list_index].start_value])
							{
        play_interface_sound(SAMPLE_BLIP1, TONE_1A);
        break;
							}
       play_interface_sound(SAMPLE_BLIP1, TONE_2C);
       game.type = GAME_TYPE_MISSION;
       game.mission_index = menu_list[menu_element[i].list_index].start_value;
							prepare_templates_for_new_game();
       prepare_for_mission(); // sets up w_init so that start_world will prepare the world for a mission
        // also loads in enemy templates and does other preparation for a mission
       new_world_from_world_init();
       generate_map_from_map_init();
//       generate_random_map(w_init.map_size_blocks, w_init.players, w_init.game_seed);
       start_world();
       run_game_from_menu();*/
       break;

     } // end of switch action

    } // end of if action

    if (menu_list[menu_element[i].list_index].type == EL_TYPE_SELECT
					&& ex_control.mouse_y_pixels >= menu_element[i].y1 + 25
					&& ex_control.mouse_y_pixels <= menu_element[i].y1 + 25 + SELECT_BUTTON_H)
				{
					int x_offset = ex_control.mouse_x_pixels - (menu_element[i].x1 + 30);
					if (x_offset % (int) (SELECT_BUTTON_W+SELECT_BUTTON_GAP) < SELECT_BUTTON_W)
					{
					 int select_button = x_offset / (SELECT_BUTTON_W+SELECT_BUTTON_GAP);
				  switch(menu_element[i].list_index)
				  {
						 case EL_SETUP_PLAYERS:
							 if (select_button >= 0
								 && select_button <= 2)
								 {
 									w_init.players = select_button + 2;
          fix_map_code();
          reset_map_for_menu();
          play_interface_sound(SAMPLE_BLIP1, TONE_2A);
								 }
							 break;
						 case EL_SETUP_CORES:
							 if (select_button >= 0
								 && select_button <= 4)
								 {
 									w_init.core_setting = select_button;
          fix_map_code();
          reset_map_for_menu();
          play_interface_sound(SAMPLE_BLIP1, TONE_2A);
								 }
							 break;
						 case EL_SETUP_DATA:
							 if (select_button >= 0
								 && select_button <= 3)
								 {
								 	int player_index;
								 	for (player_index = 0; player_index < PLAYERS; player_index ++)
										{
								 	 w_init.starting_data_setting [player_index] = select_button;
										}
          fix_map_code();
          play_interface_sound(SAMPLE_BLIP1, TONE_2A);
								 }
							 break;
						 case EL_SETUP_MAP_SIZE:
							 if (select_button >= 0
								 && select_button <= 4)
								 {
 									w_init.size_setting = select_button;
          fix_map_code();
          reset_map_for_menu();
          play_interface_sound(SAMPLE_BLIP1, TONE_2A);
								 }
							 break;
						 case EL_SETUP_COMMAND_MODE:
							 if (select_button >= 0
								 && select_button < COMMAND_MODES)
								 {
 									w_init.command_mode = select_button;
          play_interface_sound(SAMPLE_BLIP1, TONE_2A);
								 }
							 break;
				  }
					}
				} // end of if EL_TYPE_SELECT

   } // end of if just_pressed
//   if (rmb_just_pressed)
//			{
//				print_help(menu_list[menu_element[i].list_index].help_type);
//			}
   break;
  } // end of if mouse in menu

 } // end of for i loop for menu elements

// number of lines scrolled by moving the mousewheel:
#define EDITOR_MOUSEWHEEL_SPEED 48

// check for mousewheel movement:
 if (mstate.use_scrollbar)
 {
  if (ex_control.mousewheel_change == 1)
  {
   mstate.window_pos += EDITOR_MOUSEWHEEL_SPEED;
   if (mstate.window_pos > mstate.h - settings.option [OPTION_WINDOW_H])
    mstate.window_pos = mstate.h - settings.option [OPTION_WINDOW_H];
   slider_moved_to_value(&mstate.mscrollbar, mstate.window_pos);
  }
  if (ex_control.mousewheel_change == -1)
  {
   mstate.window_pos -= EDITOR_MOUSEWHEEL_SPEED;
   if (mstate.window_pos < 0)
    mstate.window_pos = 0;
   slider_moved_to_value(&mstate.mscrollbar, mstate.window_pos);
  }
 }


// may not reach here

}

// call this when a map code has been entered
// won't do anything if the map code is the wrong length
static void enter_map_code(void)
{

 int i;

 if (strlen(mstate.map_code_string_temp) != MAP_CODE_LENGTH)
		return;

// read_map_code_value with 0 final argument always returns a number 0-3. With 1 it returns 0-9.

	int code_value = read_map_code_value(mstate.map_code_string_temp [MAP_CODE_PLAYERS], 0);
// need to adjust player number because A means 2, B = 3 etc
	code_value += 2;
	if (code_value > 4)
		code_value = 4;
	w_init.players = code_value;

	code_value = read_map_code_value(mstate.map_code_string_temp [MAP_CODE_CORES], 0);
	w_init.core_setting = code_value;

	code_value = read_map_code_value(mstate.map_code_string_temp [MAP_CODE_SIZE], 0);
	w_init.size_setting = code_value;

	code_value = read_map_code_value(mstate.map_code_string_temp [MAP_CODE_DATA], 0);
	for (i = 0; i < PLAYERS; i ++)
	{
	 w_init.starting_data_setting [i] = code_value;
	}


	w_init.game_seed = 0;

	code_value = read_map_code_value(mstate.map_code_string_temp [MAP_CODE_SEED_0], 1);
	w_init.game_seed += code_value * 100;

	code_value = read_map_code_value(mstate.map_code_string_temp [MAP_CODE_SEED_1], 1);
	w_init.game_seed += code_value * 10;

	code_value = read_map_code_value(mstate.map_code_string_temp [MAP_CODE_SEED_2], 1);
	w_init.game_seed += code_value;

	fix_map_code(); // this takes the w_init values and writes the correct code to mstate.map_code_string

}

static int read_map_code_value(char read_char, int number)
{

	if (number) // expects 0-9
	{
		if (read_char < '0'
			|| read_char > '9')
			return 0;

		return read_char - '0';
	}

	switch(read_char)
	{
  default:
	 case 'a':
	 case 'A':
		 return 0;
		case 'b':
		case 'B':
			return 1;
		case 'c':
		case 'C':
			return 2;
		case 'd':
		case 'D':
			return 3;
	}

	return 0;

}


void run_game_from_menu(void)
{

// at this point we need to wait for the user to let go of the left mouse button
//  (otherwise the game will regard it as having just been clicked when the game starts)

//  ALLEGRO_EVENT ev;
/*
  while (TRUE)
  {
   al_wait_for_event(event_queue, &ev);
   get_ex_control();
   if (ex_control.mb_press [0] != BUTTON_HELD)
    break;
  }*/


 al_flush_event_queue(event_queue);
 al_flush_event_queue(fps_queue);

// clear_sound_list();
// reset_music(rand());


//       game.phase = GAME_PHASE_PREGAME;
       run_game();

// finished, so we return to menu:
       game.phase = GAME_PHASE_MENU;
//       settings.edit_window = EDIT_WINDOW_CLOSED;
       mstate.menu_type = MENU_TYPE_NORMAL;
       open_menu(MENU_MAIN);
//       reset_menu_templates(); - not needed - open_menu(MENU_MAIN) calls this

}


// finds a menu_element based on list[t_index] and returns its value
// assumes element exists (exits with error on failure)
int get_menu_element_value(int t_index)
{

 int i = 0;

 while(TRUE)
 {
  if (i >= mstate.elements)
  {
   fprintf(stdout, "\nError: could not find menu list member %i.", t_index);
   error_call();
  }
  if (menu_element[i].list_index == t_index)
   return menu_element[i].value;
  i ++;
 };

}

/*
int mrand_pos;

static void seed_mrand(int mrand_seed)
{

	mrand_pos = mrand_seed; // will be bounds-checked when used

}


#define MAP_RAND_SIZE 1024
#define MAP_RAND_MASK (MAP_RAND_SIZE-1)

int map_rand [MAP_RAND_SIZE] =
{
33943, 64987, 19207, 7112, 13984, 33647, 10108, 47942, 47213, 46343, 2212, 62813, 13486, 48441, 61430, 60053, 7967, 48057,
54056, 29742, 55785, 43429, 31763, 21220, 52862, 37967, 1969, 23447, 29530, 33752, 7047, 21245, 22164, 53559, 36503, 40822,
27199, 61491, 37137, 44361, 30707, 53286, 6116, 37083, 37402, 48365, 44472, 49864, 32301, 52830, 9330, 50009, 59043, 36976,
5091, 20359, 23697, 58038, 2373, 26396, 47515, 35803, 13423, 22986, 37573, 7270, 45281, 22889, 16490, 33350, 30550, 63299,
1801, 31416, 5435, 25902, 51058, 18462, 46267, 44916, 12913, 59261, 63558, 8926, 41144, 4808, 17574, 62329, 62202, 39193,
13629, 19204, 4060, 30029, 48331, 16369, 48017, 17235, 18694, 16943, 44245, 47426, 39393, 60128, 1107, 55256, 42120, 31977,
8057, 37240, 45579, 17380, 30694, 32681, 1568, 44998, 24384, 28228, 46344, 28478, 31501, 61059, 5182, 2737, 40641, 62682,
27556, 19904, 22268, 39040, 7851, 4023, 59060, 10062, 9412, 38031, 56416, 42172, 50370, 1168, 48666, 43644, 30296, 7120,
32384, 16439, 49217, 59395, 64221, 57949, 15940, 35066, 35075, 36953, 12635, 18640, 49847, 54731, 16306, 40584, 64171, 62704,
46079, 4608, 3197, 62439, 64986, 21233, 52489, 34343, 11146, 29684, 28972, 32863, 63359, 7171, 54060, 64787, 10562, 27399,
41394, 13667, 5734, 19362, 18347, 13057, 7273, 52316, 29753, 10687, 7869, 11746, 14812, 24771, 56921, 7806, 21960, 64052,
16628, 45602, 39332, 63413, 38599, 43778, 23358, 43237, 60371, 53973, 48393, 51263, 64045, 34617, 43729, 42406, 4943, 48428,
34162, 21394, 51005, 43521, 30499, 51113, 61649, 61508, 23117, 12687, 46377, 19628, 5215, 4034, 18219, 44955, 51133, 26008,
539, 37484, 12399, 29823, 4354, 12135, 11049, 17532, 36356, 57471, 54909, 6668, 49974, 53669, 64782, 9264, 14280, 2998, 55010,
3753, 61814, 22010, 46697, 59217, 11605, 9704, 50829, 49920, 32, 58045, 46174, 56634, 9230, 65523, 26424, 39516, 1954, 51420,
62470, 43267, 47772, 59097, 35956, 59597, 33864, 18514, 37177, 18261, 34670, 44371, 35077, 57936, 8000, 63247, 49208, 14570,
62794, 22911, 29462, 33380, 6972, 26231, 64774, 40216, 36634, 54739, 26044, 8346, 16234, 6016, 19146, 36687, 4433, 61856, 17114,
3064, 50988, 2575, 52201, 7092, 49259, 14289, 54453, 18048, 61594, 65360, 36404, 21244, 13643, 3810, 52932, 24671, 14300, 51197,
30800, 28469, 967, 37909, 19151, 21072, 48267, 60542, 65479, 16405, 13609, 31243, 29062, 24042, 59461, 61735, 27262, 43539, 62790,
64376, 42672, 40279, 8405, 7513, 20110, 65399, 4184, 41076, 58788, 37310, 35165, 43583, 43509, 37235, 54820, 1920, 2193, 2231,
12257, 65509, 57041, 48466, 42423, 4209, 58971, 432, 19674, 55615, 18286, 54917, 17297, 61080, 45644, 2803, 33513, 19003, 63101,
55001, 55296, 22792, 6992, 30732, 65425, 38698, 36032, 1463, 25402, 40994, 30664, 44777, 61647, 13174, 63982, 20732, 39513, 42490,
28826, 29748, 45785, 736, 24637, 59116, 37155, 61439, 21760, 1449, 6524, 58201, 32059, 19191, 20333, 58630, 25477, 21948, 4946,
41724, 6491, 11943, 31393, 23149, 7743, 52466, 47190, 61446, 31476, 54699, 954, 17125, 10402, 11011, 18572, 17016, 13021, 14533,
47447, 60943, 29461, 32973, 63985, 14469, 18949, 3032, 33175, 40357, 41998, 11790, 60342, 29564, 16716, 41492, 51459, 44032, 39528,
3007, 60235, 18808, 39117, 28152, 7168, 28285, 40472, 5254, 55417, 23605, 3132, 398, 12067, 39422, 13521, 53713, 31541, 50781, 38147,
26076, 6606, 21617, 62173, 8508, 20949, 3483, 51100, 3116, 5389, 32138, 62956, 14246, 62720, 63012, 13645, 22067, 38455, 30728, 29482,
28086, 38065, 55511, 11419, 58658, 57789, 12633, 49685, 10218, 43684, 37579, 49518, 58574, 17147, 33617, 1465, 675, 44177, 4790, 50282,
30210, 41922, 35978, 31159, 34112, 38741, 45778, 4951, 33699, 41274, 61501, 19487, 56170, 44945, 10194, 62589, 62093, 25625, 16833, 47492,
9191, 7725, 32114, 11027, 8401, 4720, 8096, 10175, 34912, 27156, 59465, 54698, 39026, 18445, 29819, 61807, 26807, 37820, 10531, 17304,
43938, 16253, 1350, 6784, 52031, 36797, 26990, 43924, 7123, 55313, 24615, 38664, 10165, 18815, 60130, 31126, 2148, 47839, 39578, 14956,
52394, 26890, 34734, 56324, 32203, 33353, 55635, 12501, 25582, 32578, 19993, 58725, 54996, 8772, 30458, 14936, 12301, 22112, 52712, 27354,
16076, 49040, 6703, 28897, 32275, 53139, 50482, 28566, 2879, 59907, 11474, 59167, 18287, 22415, 39044, 7003, 12045, 46545, 39024, 23855,
50989, 8888, 30120, 11001, 62150, 43084, 33932, 17169, 34787, 40322, 15619, 51086, 62978, 25827, 39692, 53123, 58225, 15494, 64252, 39132,
49466, 37530, 61182, 246, 58760, 28023, 9463, 43737, 14503, 32393, 1371, 33302, 10776, 41809, 15760, 27302, 17381, 2946, 47350, 65035, 19414,
22242, 5787, 63349, 64535, 62672, 50740, 19217, 32308, 39213, 48092, 60647, 49626, 59911, 8579, 48899, 27201, 33086, 26953, 3722, 31122, 8228,
8099, 37097, 8952, 30277, 44037, 44417, 30592, 28069, 51429, 41201, 8826, 52794, 53039, 38047, 4344, 38264, 25637, 43111, 39372, 50952, 25344,
28650, 56194, 24660, 2029, 9415, 26243, 18801, 16656, 632, 16526, 31231, 27449, 2337, 37620, 16090, 58961, 24471, 36085, 22299, 27455, 55089,
61760, 21778, 57220, 22121, 35558, 11412, 63284, 8162, 43356, 53555, 49104, 41086, 51264, 29037, 17017, 4655, 61094, 32643, 18333, 21525, 58161,
57602, 60427, 38304, 16397, 35293, 46237, 27999, 46754, 42353, 31741, 39230, 35334, 18781, 38882, 14833, 64950, 57531, 31233, 573, 26820, 60616,
64848, 8173, 57481, 46163, 64967, 1182, 39879, 9738, 34742, 54345, 10913, 63348, 50888, 23036, 25629, 59614, 31984, 24475, 19807, 32945, 60016,
19889, 49406, 29627, 3485, 42967, 28543, 3902, 37843, 24951, 47474, 28567, 4353, 13004, 53967, 63892, 10487, 41481, 55880, 55930, 22044, 8967,
23596, 15438, 284, 16092, 60048, 58544, 55406, 8536, 41343, 65019, 26280, 25526, 44762, 28531, 32760, 20387, 1232, 53395, 53861, 39994, 27464,
24412, 39266, 54785, 16911, 31346, 14129, 13890, 58799, 64418, 65219, 33131, 9124, 40764, 9986, 26399, 58271, 13425, 11022, 26149, 55281, 49989,
56404, 65271, 60043, 7736, 37273, 1251, 12774, 49234, 60669, 3033, 44834, 3587, 37510, 57913, 9261, 12582, 46112, 41234, 56025, 22401, 10999, 6090,
45114, 20771, 24326, 49372, 23538, 15928, 62467, 64562, 59518, 11168, 40516, 47648, 26295, 14069, 36072, 51324, 2168, 25422, 24843, 50644, 29264, 35070,
15342, 57857, 8227, 9344, 45782, 5277, 51139, 50596, 54110, 60260, 62506, 59466, 46800, 54863, 22075, 21011, 55528, 30721, 22018, 24531, 56098, 6550,
55691, 53919, 21, 62259, 47422, 34704, 54689, 42717, 17876, 30997, 13128, 4555, 2562, 6123, 14288, 35453, 5072, 27618, 8322, 22295, 54236, 2187, 35417,
44692, 62138, 24634, 41123, 64958, 49422, 46133, 22426, 31015, 61723, 15000, 17594, 27534, 61336, 18253, 52647, 36648, 44328, 10405, 56485, 37561, 49031,
58219, 34773, 54580, 56386, 59782, 42564, 26221, 36188, 46385, 54712, 25348, 55599, 35250, 71, 30684, 3728, 43451, 45259, 41148, 59456, 20232, 10217,
4639, 57855, 36730, 64743, 56527, 31136, 38014, 64560, 6791, 65183, 24505, 22913, 30332, 16634, 12653, 35201, 11936, 23917, 14631, 4333, 4207, 5361,
52756, 28887, 56003, 42330, 48847, 51177, 47221, 30927, 50181, 31835, 31299, 42734, 57748, 60140, 40469, 56708, 59256, 28595
};

*/

/*
static int mrand(int range)
{
 if (range <= 0)
		return 0;

	return map_rand [mrand_pos++ & MAP_RAND_MASK] % range;
}
*/




ALLEGRO_BITMAP* title_bitmap;

// assumes settings already setup
void run_intro_screen(void)
{


 al_set_target_bitmap(al_get_backbuffer(display));

 al_flush_event_queue(event_queue);
 al_flush_event_queue(fps_queue);

 ALLEGRO_EVENT ev;
 int mouse_x;
 int mouse_y;
 int just_pressed;
 int y_line;
 int screen_centre_x = settings.option [OPTION_WINDOW_W] / 2;
 int key_test = 0;

#define START_BOX_Y (settings.option [OPTION_WINDOW_H] / 2)
#define START_BOX_W scaleUI_x(FONT_SQUARE_LARGE,150)
#define START_BOX_H scaleUI_y(FONT_SQUARE_LARGE,20)

 do
 {

  get_ex_control(1, 0); // 1 means that clicking the native close window button will exit immediately
  mouse_x = ex_control.mouse_x_pixels;
  mouse_y = ex_control.mouse_y_pixels;
  just_pressed = (ex_control.mb_press [0] == BUTTON_JUST_PRESSED);

// key test is for people using non-QWERTY keyboards. See init.txt.
  if (ex_control.unichar_input == 'k')
			key_test = 1;

//  if (ex_control.key_press [ALLEGRO_KEY_ESCAPE] > 0)
//    return;

//  al_clear_to_color(colours.base [COL_BLUE] [SHADE_LOW]);
  run_menu_stripes(1);


//  al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], settings.option [OPTION_WINDOW_W] / 2, 200, ALLEGRO_ALIGN_CENTRE, "L I B E R A T I O N   C I R C U I T");
   al_draw_bitmap(title_bitmap, settings.option [OPTION_WINDOW_W] / 2 - 300, 200, 0);



  reset_i_buttons();

  if (mouse_x >= screen_centre_x - START_BOX_W
   && mouse_x <= screen_centre_x + START_BOX_W
   && mouse_y >= START_BOX_Y - START_BOX_H
   && mouse_y <= START_BOX_Y + START_BOX_H)
  {
   add_menu_button(screen_centre_x - START_BOX_W, START_BOX_Y - START_BOX_H,
                    screen_centre_x + START_BOX_W, START_BOX_Y + START_BOX_H,
                    colours.base_trans [COL_BLUE] [SHADE_HIGH] [TRANS_THICK], 9, 4);
/*
   al_draw_rectangle(screen_centre_x - START_BOX_W, START_BOX_Y - START_BOX_H,
                    screen_centre_x + START_BOX_W, START_BOX_Y + START_BOX_H,
                    colours.base [COL_BLUE] [SHADE_HIGH], 1);*/

   if (just_pressed)
   {
    play_interface_sound(SAMPLE_BLIP1, TONE_2C);
    return;
   }

  }
   else
    add_menu_button(screen_centre_x - START_BOX_W, START_BOX_Y - START_BOX_H,
                    screen_centre_x + START_BOX_W, START_BOX_Y + START_BOX_H,
                    colours.base_trans [COL_BLUE] [SHADE_MED] [TRANS_THICK], 9, 4);

  draw_menu_buttons();

  al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], settings.option [OPTION_WINDOW_W] / 2, START_BOX_Y - 4, ALLEGRO_ALIGN_CENTRE, ">>   START   <<");

  y_line = scaleUI_y(FONT_SQUARE,100);
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], settings.option [OPTION_WINDOW_W] - 50, settings.option [OPTION_WINDOW_H] - y_line, ALLEGRO_ALIGN_RIGHT, "version 1.3");
  y_line -= scaleUI_y(FONT_SQUARE,25);
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], settings.option [OPTION_WINDOW_W] - 50, settings.option [OPTION_WINDOW_H] - y_line, ALLEGRO_ALIGN_RIGHT, "Copyright 2017 Linley Henzell");
  y_line -= scaleUI_y(FONT_SQUARE,15);
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], settings.option [OPTION_WINDOW_W] - 50, settings.option [OPTION_WINDOW_H] - y_line, ALLEGRO_ALIGN_RIGHT, "Free software (GPL v3 or later)");
  y_line -= scaleUI_y(FONT_SQUARE,15);
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], settings.option [OPTION_WINDOW_W] - 50, settings.option [OPTION_WINDOW_H] - y_line, ALLEGRO_ALIGN_RIGHT, "Built with Allegro 5");
  y_line -= scaleUI_y(FONT_SQUARE,15);
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], settings.option [OPTION_WINDOW_W] - 50, settings.option [OPTION_WINDOW_H] - y_line, ALLEGRO_ALIGN_RIGHT, "Procedural music based on Otomata by Batuhan Bozkurt");

  if (key_test)
			print_key_codes();


//  if (settings.option [OPTION_SPECIAL_CURSOR])
   draw_mouse_cursor();
  al_flip_display();
  al_set_target_bitmap(al_get_backbuffer(display));
  al_wait_for_event(event_queue, &ev);

 } while (TRUE);

}


static void print_key_codes(void)
{


  ALLEGRO_KEYBOARD_STATE key_state;

  al_get_keyboard_state(&key_state);

  int i;
  float text_y = 400;

  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], 400, text_y, ALLEGRO_ALIGN_RIGHT, "Key code test mode");
  text_y += scaleUI_y(FONT_SQUARE,15);
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], 400, text_y, ALLEGRO_ALIGN_RIGHT, "(see init.txt for more information)");
  text_y += scaleUI_y(FONT_SQUARE,15);
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], 400, text_y, ALLEGRO_ALIGN_RIGHT, "Press a key:");

  int keys_pressed = 0;

  for (i = 0; i < ALLEGRO_KEY_MAX; i ++)
		{
			if (al_key_down(&key_state, i))
			{
				keys_pressed ++;
    al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], 405, text_y, ALLEGRO_ALIGN_LEFT, "%i", i);
    text_y += scaleUI_y(FONT_SQUARE,15);
    if (i == ALLEGRO_KEY_ESCAPE)
					error_call();
			}
		}

		if (!keys_pressed)
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MED], 405, text_y, ALLEGRO_ALIGN_LEFT, "none");


}



#define STRIPE_POLY_BUFFER 1000
#define STRIPE_SPEED 1.31

ALLEGRO_VERTEX stripe_poly_buffer [STRIPE_POLY_BUFFER];
int stripe_vertex_pos;
void add_stripe_vertex(float x, float y, int col, int shade);

void run_menu_stripes(int draw)
{

 int i;
 int top_displace = settings.option [OPTION_WINDOW_H] / 3;

// first move all stripes and count down:
 mstate.stripe_next_group_count --;
 mstate.stripe_group_time ++;

 if (mstate.stripe_next_group_count <= 0)
	{
		mstate.stripe_next_group_count = 300 + grand(300);
  mstate.stripe_group_time = 0;
  mstate.stripe_next_stripe = 1;
					  int new_col = grand(STRIPE_COLS - 1);
					  if (new_col == mstate.stripe_group_col)
								new_col ++; // this is the only way to get col 11
					  mstate.stripe_group_col = new_col;
//					  mstate.stripe_group_col ++;
					  mstate.stripe_group_col %= STRIPE_COLS;

//					  fprintf(stdout, "\nnew_col %i", mstate.stripe_group_col );
	}

 if (mstate.stripe_next_stripe > 0)
 {
		mstate.stripe_next_stripe --;
		if (mstate.stripe_next_stripe == 0)
		{
			for (i = 0; i < MENU_STRIPES; i ++)
			{
				if (mstate.stripe_exists [i] == 0)
				{
					mstate.stripe_exists [i] = 1;
					mstate.stripe_x [i] = settings.option [OPTION_WINDOW_W];
				 mstate.stripe_col [i] = mstate.stripe_group_col;
				 mstate.stripe_shade [i] = mstate.stripe_group_shade;
					if (mstate.stripe_group_time < 100)
					{
					 mstate.stripe_size [i] = 10 + grand(40);
					 mstate.stripe_next_stripe = mstate.stripe_size [i] + 5 + grand(50);
					}
					 else
					 {
						 mstate.stripe_next_stripe = -1; // no more stripes until end
					  mstate.stripe_size [i] = (mstate.stripe_next_group_count + 200) * STRIPE_SPEED;
					 }
					break;
				}
			}
		}
 }


 for (i = 0; i < MENU_STRIPES; i ++)
	{
		if (mstate.stripe_exists [i] == 0)
			continue;
		mstate.stripe_x [i] -= STRIPE_SPEED;
		if (mstate.stripe_x [i] + mstate.stripe_size [i] < top_displace * -1)
			mstate.stripe_exists [i] = 0;
	}

 if (!draw)
		return;

// now draw:

 al_clear_to_color(colours.base [COL_BLUE] [SHADE_LOW]);
/*
 for (i = 0; i < MENU_STRIPES; i++)
	{
		if (mstate.stripe_exists [i] != 0)
		{
	  al_draw_filled_rectangle(mstate.stripe_x [i], 0, mstate.stripe_x [i] + mstate.stripe_size [i], settings.option [OPTION_WINDOW_H] + 1, colours.base [mstate.stripe_col [i]] [mstate.stripe_shade [i]]);
		}
	}*/

 stripe_vertex_pos = 0;

 for (i = 0; i < MENU_STRIPES; i++)
	{
		if (mstate.stripe_exists [i] != 0)
		{
			add_stripe_vertex(mstate.stripe_x [i], settings.option [OPTION_WINDOW_H] + 1, mstate.stripe_col [i], mstate.stripe_shade [i]);
			add_stripe_vertex(mstate.stripe_x [i] + top_displace, -1, mstate.stripe_col [i], mstate.stripe_shade [i]);
			add_stripe_vertex(mstate.stripe_x [i] + mstate.stripe_size [i], settings.option [OPTION_WINDOW_H] + 1, mstate.stripe_col [i], mstate.stripe_shade [i]);

			add_stripe_vertex(mstate.stripe_x [i] + mstate.stripe_size [i], settings.option [OPTION_WINDOW_H] + 1, mstate.stripe_col [i], mstate.stripe_shade [i]);
			add_stripe_vertex(mstate.stripe_x [i] + top_displace, -1, mstate.stripe_col [i], mstate.stripe_shade [i]);
			add_stripe_vertex(mstate.stripe_x [i] + top_displace + mstate.stripe_size [i], -1, mstate.stripe_col [i], mstate.stripe_shade [i]);

			if (stripe_vertex_pos >= STRIPE_POLY_BUFFER - 6)
				break;
		}
	}



 al_draw_prim(stripe_poly_buffer, NULL, NULL, 0, stripe_vertex_pos, ALLEGRO_PRIM_TRIANGLE_LIST);


}

void add_stripe_vertex(float x, float y, int col, int shade)
{

			stripe_poly_buffer [stripe_vertex_pos].x = x;
			stripe_poly_buffer [stripe_vertex_pos].y = y;
			stripe_poly_buffer [stripe_vertex_pos].z = 0;
			stripe_poly_buffer [stripe_vertex_pos].color = mstate.stripe_al_col [col];
			stripe_vertex_pos ++;

}


static void reset_map_for_menu(void)
{

 fix_w_init_size();

 generate_scattered_map(w_init.story_area,
																					w_init.map_size_blocks,
																					w_init.players,
																					w_init.game_seed);

}
