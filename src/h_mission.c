
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"
#include "m_maths.h"

#include "g_misc.h"

#include "c_header.h"
#include "c_prepr.h"
#include "e_slider.h"
#include "e_header.h"
#include "g_header.h"
#include "g_world.h"
#include "g_world_map.h"
#include "g_world_map_2.h"
#include "e_log.h"
#include "g_proc_new.h"

#include "i_input.h"
#include "i_view.h"
#include "i_disp_in.h"
#include "m_input.h"
#include "s_menu.h"
#include "t_files.h"
#include "t_template.h"
#include "h_mission.h"
#include "h_story.h"


struct mission_state_struct mission_state; // just need one struct for the current mission


/*struct missionsstruct
{
	int status [MISSIONS]; // MISSION_STATUS_* enum
	int locked [MISSIONS]; // 0 or 1
};*/

extern struct world_init_struct w_init;
extern struct game_struct game;
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];


static int mission_add_data_well(int data_well_type, float spin_sign, int x, int y);
static int mission_add_data_well_to_ring(int data_well_type, float spin_sign, int md_index, int angle);
//static void mission_mirror_spawns_and_wells(void);
static int add_orange_data_well(int centre_well, int data_well_type, int angle, int distance_from_centre, int line_thickness);
static int mission_add_data_well_to_circle(int data_well_type, int angle, int dist, float spin_sign, int centre_x, int centre_y);
static void mission_add_red_data_well_ring(int data_well_type, int centre_x, int centre_y, int circle_size, int wells, int starting_angle);

struct extra_spawnstruct
{
	int player_index;
	int template_index;
	int spawn_x_block, spawn_y_block;
	int spawn_angle;
};
#define EXTRA_SPAWNS 16

// current_missionstruct holds details about a mission currently being played or initialised.
struct mission_init_struct
{

// initialisation information
	int extra_spawns; // number of additional player 1 processes to spawn in the mission
	struct extra_spawnstruct extra_spawn [EXTRA_SPAWNS];

#define MISSION_DATA_WELL_TYPES 4
 int data_well_reserves [MISSION_DATA_WELL_TYPES];
 int data_well_reserve_A [MISSION_DATA_WELL_TYPES];
 int data_well_reserve_B [MISSION_DATA_WELL_TYPES];
 float data_well_spin [MISSION_DATA_WELL_TYPES];

};

struct mission_init_struct mission_init;
void init_mission_init_struct(void);
void add_extra_spawn(int player_index, int template_index, int spawn_x_block, int spawn_y_block, int spawn_angle);

void init_mission_init_struct(void)
{
	mission_init.extra_spawns = 0; // don't bother initialising the extra_spawn struct
}

void add_extra_spawn(int player_index, int template_index, int spawn_x_block, int spawn_y_block, int spawn_angle)
{

	if (mission_init.extra_spawns >= EXTRA_SPAWNS)
	{
		fpr("\nError in s_mission.c: add_extra_spawn(): too many extra spawns.");
		error_call();
	}

	mission_init.extra_spawn[mission_init.extra_spawns].player_index = player_index;
	mission_init.extra_spawn[mission_init.extra_spawns].template_index = template_index;
	mission_init.extra_spawn[mission_init.extra_spawns].spawn_x_block = spawn_x_block;
	mission_init.extra_spawn[mission_init.extra_spawns].spawn_y_block = spawn_y_block;
	mission_init.extra_spawn[mission_init.extra_spawns].spawn_angle = spawn_angle;

	mission_init.extra_spawns++;

}


// prepares w_init etc for a mission
// can cause a fatal error if it fails to load a file from disk (in which case the whole program stops, because this shouldn't happen)
void prepare_for_mission(void)
{

	w_init.starting_data_setting [0] = 0;
	w_init.starting_data_setting [1] = 0; // this is likely to be set again later

 init_w_init();

 int i, j;

// Clear all of non-user player's templates:
	for (i = 1; // note i = 1
	     i < PLAYERS; i ++)
	{
  for (j = 0; j < TEMPLATES_PER_PLAYER; j ++)
	 {
		 clear_template_including_source(&templ[i][j]);
	 }
	}

// set up large data wells:
// (these default values may be updated below for particular areas or missions):
      mission_init.data_well_reserves [0] = 4;
      mission_init.data_well_reserve_A [0] = 2000;
      mission_init.data_well_reserve_B [0] = 1000;
      mission_init.data_well_spin [0] = 0.004;
// medium:
      mission_init.data_well_reserves [1] = 3;
      mission_init.data_well_reserve_A [1] = 1800;
      mission_init.data_well_reserve_B [1] = 900;
      mission_init.data_well_spin [1] = 0.003;
// small:
      mission_init.data_well_reserves [2] = 3;
      mission_init.data_well_reserve_A [2] = 1800;
      mission_init.data_well_reserve_B [2] = 0;
      mission_init.data_well_spin [2] = 0.002;
// small: (currently 3 is only used for orange, which sets its own values for the following)
      mission_init.data_well_reserves [3] = 2;
      mission_init.data_well_reserve_A [3] = 1200;
      mission_init.data_well_reserve_B [3] = 0;
      mission_init.data_well_spin [3] = 0.0015;

// default settings:
 w_init.core_setting = 3;
 w_init.size_setting = 3;
 fix_w_init_size();

 w_init.players = 2;
 w_init.command_mode = COMMAND_MODE_COMMAND; // can be set to AUTO below
 strcpy(w_init.player_name [0], "You");
 strcpy(w_init.player_name [1], "Opponent");

 init_mission_init_struct();

// clear_map_init(w_init.map_size_blocks,	w_init.players); // can't do this here because map size varies. It's done below for each mission.


 mission_state.phase = 0;
 mission_state.reveal_player1 = 0;

 mission_state.union_value1 = 0;
 mission_state.union_value2 = 0;
 mission_state.union_value3 = 0;
 mission_state.union_value4 = 0;
 mission_state.union_value5 = 0;

 int player_base_x, player_base_y;
	int enemy_base_x, enemy_base_y;

	int data_well_index [DATA_WELLS]; // This is never used.

	data_well_index [0] = 0;

/*

PLAYER
- blue + yellow/orange

BLUE/TUTE
- yellow + blue/white

GREEN
- green + white/yellow

YELLOW
- orange + blue/purple

ORANGE
- red + blue/white

PURPLE
- purple + orange/red

RED
- white + ultraviolet?



enum
{
TEAM_COL_BLUE, // player
TEAM_COL_YELLOW, // BLUE
TEAM_COL_GREEN, // GREEN
TEAM_COL_WHITE, // RED

TEAM_COL_PURPLE,
TEAM_COL_ORANGE, // YELLOW
TEAM_COL_RED,

TEAM_COLS

};

enum
{
PACKET_COL_YELLOW_ORANGE,
PACKET_COL_WHITE_BLUE,
PACKET_COL_WHITE_YELLOW,
PACKET_COL_WHITE_PURPLE,
PACKET_COL_ORANGE_RED,

PACKET_COL_BLUE_PURPLE,
PACKET_COL_ULTRAVIOLET,

PACKET_COLS
};


*/

   w_init.size_setting = 1;
   fix_w_init_size();

   w_init.story_area = game.area_index;


   set_game_colours_for_area(game.area_index, 2);

   switch(game.area_index)
   {
 			default:
				 case AREA_TUTORIAL:
      w_init.core_setting = 1;
      w_init.size_setting = 1;
      fix_w_init_size();
      reset_map_init(w_init.map_size_blocks,
																		   AREA_BLUE,
																		   2); // resets map initialisation code in g_world_map.x. 2 means 2 players

						break;

				 case AREA_BLUE:
      w_init.core_setting = 2;
      w_init.size_setting = 2;
      if (game.mission_index == MISSION_BLUE_CAPITAL)
						{
       w_init.size_setting = 3;
						}
      fix_w_init_size();
      reset_map_init(w_init.map_size_blocks,
																		   AREA_BLUE,
																		   2); // resets map initialisation code in g_world_map.x. 2 means 2 players

						break;

				 case AREA_GREEN:
      w_init.core_setting = 3;
      w_init.size_setting = 3;
      fix_w_init_size();
      reset_map_init(w_init.map_size_blocks,
																		   AREA_GREEN,
																		   2); // resets map initialisation code in g_world_map.x. 2 means 2 players

      mission_init.data_well_reserves [2] = 3;
      mission_init.data_well_reserve_A [2] = 900;
      mission_init.data_well_reserve_B [2] = 1800;
      mission_init.data_well_spin [2] = 0.003;

						break;

				 case AREA_YELLOW:
      w_init.core_setting = 3;
      w_init.size_setting = 3;
      fix_w_init_size();
      reset_map_init(w_init.map_size_blocks,
																		   AREA_YELLOW,
																		   2); // resets map initialisation code in g_world_map.x. 2 means 2 players
						break;


				 case AREA_ORANGE:
      w_init.core_setting = 3;
      w_init.size_setting = 3;
      fix_w_init_size();
      reset_map_init(w_init.map_size_blocks,
																		   AREA_ORANGE,
																		   2); // resets map initialisation code in g_world_map.x. 2 means 2 players

// large (for enemy base)
      mission_init.data_well_reserves [0] = 3;
      mission_init.data_well_reserve_A [0] = 1000;
      mission_init.data_well_reserve_B [0] = 2000;
      mission_init.data_well_spin [0] = 0.004;
// medium (for player base)
      mission_init.data_well_reserves [1] = 2;
      mission_init.data_well_reserve_A [1] = 900;
      mission_init.data_well_reserve_B [1] = 1800;
      mission_init.data_well_spin [1] = 0.003;
// small:
      mission_init.data_well_reserves [2] = 1;
      mission_init.data_well_reserve_A [2] = 900;
      mission_init.data_well_reserve_B [2] = 1800;
      mission_init.data_well_spin [2] = 0.002;
// smallest:
      mission_init.data_well_reserves [3] = 1;
      mission_init.data_well_reserve_A [3] = 0;
      mission_init.data_well_reserve_B [3] = 1800;
      mission_init.data_well_spin [3] = 0.001;

						break;



				 case AREA_PURPLE:
      w_init.core_setting = 3;
      w_init.size_setting = 3;
      fix_w_init_size();
      reset_map_init(w_init.map_size_blocks,
																		   AREA_PURPLE,
																		   2); // resets map initialisation code in g_world_map.x. 2 means 2 players

      mission_init.data_well_reserves [2] = 3;
      mission_init.data_well_reserve_A [2] = 900;
      mission_init.data_well_reserve_B [2] = 1800;
      mission_init.data_well_spin [2] = 0.003;
						break;

/*
				 case AREA_DARK_BLUE:
				 	w_init.core_setting = 3;
//					 w_init.local_condition = LOCAL_CONDITION_FRAGILE_PROCS;
 	    player_base_cols [1] = 2;
	     player_packet_cols [1] = 3;
      set_game_colours(BACK_COLS_BLUE_DARK, // index in back_and_hex_colours array
																				   BACK_COLS_BLUE_DARK, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
						break;
*/
				 case AREA_RED:
      w_init.core_setting = 3;
      w_init.size_setting = 3;
      fix_w_init_size();
      reset_map_init(w_init.map_size_blocks,
																		   AREA_RED,
																		   2); // resets map initialisation code in g_world_map.x. 2 means 2 players

      mission_init.data_well_spin [0] = 0.025;
      mission_init.data_well_spin [1] = 0.025;
      mission_init.data_well_spin [2] = 0.025;

      mission_init.data_well_reserve_A [0] = 1600;
      mission_init.data_well_reserve_B [0] = 1600;

						break;

   }

/*
//	 	w_init.spawn_position [0].x = 15;
//	 	w_init.spawn_position [0].y = w_init.map_size_blocks / 2;
	 	player_base_x = 11;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x, player_base_y,
																									2000, 1000, // reserves
																									4, // reserve squares
																									0.002); // spin rate
   set_player_spawn_position_by_latest_well(0, 0);

	 	mission_add_data_well(player_base_x + 13, player_base_y - 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(player_base_x + 13, player_base_y + 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate

   load_mission_source("missions/mission3/rbase.c", 1, 0);
   load_mission_source("missions/mission3/wander1.c", 1, 1);
   load_mission_source("missions/mission3/wander2.c", 1, 2);
   clear_remaining_templates(1, 3);

   enemy_base_x = w_init.map_size_blocks - 15;
   enemy_base_y = w_init.map_size_blocks / 2;

	 	mission_add_data_well(enemy_base_x, enemy_base_y,
																									2000, 0, // reserves
																									3, // reserve squares
																									-0.002); // spin rate

   set_player_spawn_position_by_latest_well(1, ANGLE_2);

//   set_player_w_init_spawn_angle(1, w_init.data_wells - 1); // - 1 because mission_add_data_well incremenets w_init.data_wells
	 	mission_add_data_well(enemy_base_x - 13, enemy_base_y - 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate
	 	mission_add_data_well(enemy_base_x - 13, enemy_base_y + 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate


//				generate_map_from_map_init(); - no, this is called later
   }
// break;
*/

#define DEFAULT_DISTANCE_FROM_WELL 512

 switch(game.mission_index)
 {
//	 case MISSION_ADVANCED1:
//   w_init.command_mode = COMMAND_MODE_AUTO;
// fall-through
	 case MISSION_TUTORIAL1:

	 	game.region_in_area_index = 0;
	 	player_base_x = 15;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(0, 1, player_base_x - 4, player_base_y);
//   add_mdetail_worm_source(player_base_x - 4, player_base_y, 20);


//   add_mdetail_worm_source(player_base_x + 20, player_base_y, 20);



   set_player_spawn_position_by_latest_well(0, 0, DEFAULT_DISTANCE_FROM_WELL);
//   set_player_w_init_spawn_angle(0, w_init.data_wells - 1); // - 1 because mission_add_data_well incremenets w_init.data_wells
	 	mission_add_data_well(1, -1, player_base_x + 13, player_base_y - 15);
//   add_mdetail_worm_source(player_base_x +13, player_base_y - 15, 20);
	 	mission_add_data_well(1, -1, player_base_x + 13, player_base_y + 15);
//   add_mdetail_worm_source(player_base_x +13, player_base_y + 15, 20);


   load_mission_source("story/tutorial/tute1/defend1.c", 1, 0);
   load_mission_source("story/tutorial/tute1/circle1.c", 1, 1);
   clear_remaining_templates(1, 2);

	 	enemy_base_x = w_init.map_size_blocks - 15;
	 	enemy_base_y = w_init.map_size_blocks / 2;

   set_player_spawn_position(1, enemy_base_x, enemy_base_y, 0);

	 	add_extra_spawn(1, 1,
																			enemy_base_x - 4,
																			enemy_base_y,
																			-2048);
	 	add_extra_spawn(1, 1,
																			enemy_base_x,
																			enemy_base_y - 4,
																			0);
/*	 	add_extra_spawn(1, 1,
																			enemy_base_x + 4,
																			enemy_base_y,
																			2048);
	 	add_extra_spawn(1, 1,
																			enemy_base_x,
																			enemy_base_y + 4,
																			4096);
*/

	  break;



//	 case MISSION_ADVANCED2:
//   w_init.command_mode = COMMAND_MODE_AUTO;
// fall-through

//default:

	 case MISSION_TUTORIAL2:

	 	game.region_in_area_index = 1;

	 	player_base_x = 15;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(0, 1, player_base_x - 4, player_base_y);
   set_player_spawn_position_by_latest_well(0, 0, DEFAULT_DISTANCE_FROM_WELL);
	 	mission_add_data_well(0, -1, player_base_x + 10, player_base_y - 20);
	 	mission_add_data_well(0, -1, player_base_x + 10, player_base_y + 20);

   load_mission_source("story/tutorial/tute2/defend2.c", 1, 0);
   load_mission_source("story/tutorial/tute2/circle2.c", 1, 1);
   clear_remaining_templates(1, 2);

	 	enemy_base_x = w_init.map_size_blocks - 15;
	 	enemy_base_y = w_init.map_size_blocks / 2;

   set_player_spawn_position(1, enemy_base_x, enemy_base_y, 0);

	 	add_extra_spawn(1, 1,
																			enemy_base_x - 4,
																			enemy_base_y,
																			-2048);
	 	add_extra_spawn(1, 1,
																			enemy_base_x,
																			enemy_base_y - 4,
																			0);
	 	add_extra_spawn(1, 1,
																			enemy_base_x + 4,
																			enemy_base_y,
																			2048);
	 	add_extra_spawn(1, 1,
																			enemy_base_x,
																			enemy_base_y + 4,
																			4096);
/*
   int extra_spawn_x = w_init.map_size_blocks - 25;
   int extra_spawn_y = w_init.map_size_blocks / 4;

	 	add_extra_spawn(1, 0,
																			extra_spawn_x,
																			extra_spawn_y,
																			4096);
	 	add_extra_spawn(1, 1,
																			extra_spawn_x - 4,
																			extra_spawn_y,
																			-2048);
	 	add_extra_spawn(1, 1,
																			extra_spawn_x,
																			extra_spawn_y - 4,
																			0);
	 	add_extra_spawn(1, 1,
																			extra_spawn_x + 4,
																			extra_spawn_y,
																			2048);
	 	add_extra_spawn(1, 1,
																			extra_spawn_x,
																			extra_spawn_y + 4,
																			4096);


   extra_spawn_x = w_init.map_size_blocks - 25;
   extra_spawn_y = w_init.map_size_blocks - (w_init.map_size_blocks / 4);

	 	add_extra_spawn(1, 0,
																			extra_spawn_x,
																			extra_spawn_y,
																			4096);
	 	add_extra_spawn(1, 1,
																			extra_spawn_x - 4,
																			extra_spawn_y,
																			-2048);
	 	add_extra_spawn(1, 1,
																			extra_spawn_x,
																			extra_spawn_y - 4,
																			0);
	 	add_extra_spawn(1, 1,
																			extra_spawn_x + 4,
																			extra_spawn_y,
																			2048);
	 	add_extra_spawn(1, 1,
																			extra_spawn_x,
																			extra_spawn_y + 4,
																			4096);
*/
			break;

	 case MISSION_BLUE_1:
	 	{

	 	game.region_in_area_index = 0;

   load_mission_source("story/blue/blue1/rbase.c", 1, 0);
   load_mission_source("story/blue/blue1/wander1.c", 1, 1);
   load_mission_source("story/blue/blue1/wander2.c", 1, 2);
   clear_remaining_templates(1, 3);


   int base_data_well_angle = ANGLE_3;

   int md_index = add_mdetail_ring(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2, 30, 0);

   mission_add_data_well_to_ring(0, 1, md_index,	base_data_well_angle);
   set_player_spawn_position_by_latest_well(0, base_data_well_angle + ANGLE_2, DEFAULT_DISTANCE_FROM_WELL);

   mission_add_data_well_to_ring(1, 1, md_index,	base_data_well_angle + ANGLE_6);
   mission_add_data_well_to_ring(2, -1, md_index,	base_data_well_angle + ANGLE_6 * 2);
   mission_add_data_well_to_ring(0, -1, md_index,	base_data_well_angle + ANGLE_6 * 3);
   set_player_spawn_position_by_latest_well(1, base_data_well_angle, DEFAULT_DISTANCE_FROM_WELL);
   mission_add_data_well_to_ring(1, -1, md_index,	base_data_well_angle + ANGLE_6 * 4);
   mission_add_data_well_to_ring(2, 1, md_index,	base_data_well_angle + ANGLE_6 * 5);

/*
   int md_index = add_mdetail_ring(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2, 23, 0);

   int base_data_well_angle = ANGLE_3;

	 	mission_add_data_well_to_ring(0, 1, md_index, base_data_well_angle);
   set_player_spawn_position_by_latest_well(0, base_data_well_angle + ANGLE_2);

   mission_add_data_well_to_ring(1, 1, md_index,	base_data_well_angle + ANGLE_4);
   mission_add_data_well_to_ring(1, -1, md_index, base_data_well_angle-ANGLE_4);

	 	mission_add_data_well_to_ring(0, -1, md_index, base_data_well_angle + ANGLE_2);
   set_player_spawn_position_by_latest_well(1, base_data_well_angle);
*/
	 	}
	  break;


	 case MISSION_BLUE_2:
	 	{

	 	game.region_in_area_index = 1;

   load_mission_source("story/blue/blue2/b2_rbase.c", 1, 0);
   load_mission_source("story/blue/blue2/b2_wander1.c", 1, 1);
   load_mission_source("story/blue/blue2/b2_wander2.c", 1, 2);
   load_mission_source("story/blue/blue2/b2_harvest.c", 1, 3);
   clear_remaining_templates(1, 4);

   w_init.starting_data_setting [1] = 1;


   int base_data_well_angle = ANGLE_8 * 3;

   int md_index_centre = add_mdetail_ring(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2, 37, 0);
   int md_index_A = add_mdetail_ring(w_init.map_size_blocks / 2,
																																					w_init.map_size_blocks / 2,
																																					15, 0);

   mission_add_data_well_to_ring(0, 1, md_index_A,	base_data_well_angle);
   set_player_spawn_position_by_latest_well(0, base_data_well_angle + ANGLE_2, DEFAULT_DISTANCE_FROM_WELL);

   mission_add_data_well_to_ring(0, -1, md_index_centre,	base_data_well_angle + ANGLE_2);
   set_player_spawn_position_by_latest_well(1, base_data_well_angle, DEFAULT_DISTANCE_FROM_WELL);

   mission_add_data_well_to_ring(1, 1, md_index_centre,	ANGLE_4 + ANGLE_8);
   mission_add_data_well_to_ring(1, -1, md_index_centre,	- ANGLE_8);

   mission_add_data_well_to_ring(1, 1, md_index_centre,	ANGLE_2 + ANGLE_8);
   mission_add_data_well_to_ring(1, -1, md_index_centre,	ANGLE_8);

   mission_add_data_well_to_ring(2, -1, md_index_A,	ANGLE_2 + ANGLE_8);
   mission_add_data_well_to_ring(2, 1, md_index_A,	ANGLE_8);
   mission_add_data_well_to_ring(2, -1, md_index_A,	-ANGLE_8);



	 	}
	  break;


	 case MISSION_BLUE_CAPITAL:
	 	{

	 	game.region_in_area_index = 2;

   load_mission_source("story/blue/blue3/b3_rbase.c", 1, 0);
   load_mission_source("story/blue/blue3/b3_wander1.c", 1, 1);
   load_mission_source("story/blue/blue3/b3_wander2.c", 1, 2);
   load_mission_source("story/blue/blue3/b3_harvest.c", 1, 3);
   clear_remaining_templates(1, 4);

   w_init.starting_data_setting [1] = 1;

   int base_data_well_angle = ANGLE_8 * 3;

   int md_index_large = add_mdetail_ring(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2, 43, 0);
   int md_index_small = add_mdetail_ring(w_init.map_size_blocks - 47,
																																					47,
																																					15, 0);

// outer ring:
   mission_add_data_well_to_ring(0, 1, md_index_large,	base_data_well_angle);
   set_player_spawn_position_by_latest_well(0, base_data_well_angle + ANGLE_2, DEFAULT_DISTANCE_FROM_WELL);

   mission_add_data_well_to_ring(1, 1, md_index_large,	base_data_well_angle + ANGLE_8);
   mission_add_data_well_to_ring(1, 1, md_index_large,	base_data_well_angle - ANGLE_8);

   mission_add_data_well_to_ring(2, 1, md_index_large,	base_data_well_angle + ANGLE_4 + ANGLE_16);
   mission_add_data_well_to_ring(2, 1, md_index_large,	base_data_well_angle - ANGLE_4 - ANGLE_16);

   mission_add_data_well_to_ring(2, -1, md_index_large,	base_data_well_angle + ANGLE_2 + ANGLE_16);
   mission_add_data_well_to_ring(2, -1, md_index_large,	base_data_well_angle + ANGLE_2 - ANGLE_16);

   mission_add_data_well_to_ring(0, -1, md_index_small,	base_data_well_angle + ANGLE_2);
   set_player_spawn_position_by_latest_well(1, base_data_well_angle, DEFAULT_DISTANCE_FROM_WELL);

   mission_add_data_well_to_ring(1, 1, md_index_small, base_data_well_angle + ANGLE_6);
   mission_add_data_well_to_ring(1, -1, md_index_small,	base_data_well_angle - ANGLE_6);



	 	}
	  break;






	 case MISSION_GREEN_1:
	 	{

	 	game.region_in_area_index = 0;

   int template_index = 0;

   load_mission_source("story/green/green1/g1_base.c", 1, template_index++);
   load_mission_source("story/green/green1/g1_firebase.c", 1, template_index++);
   load_mission_source("story/green/green1/g1_builder.c", 1, template_index++);
   clear_remaining_templates(1, template_index);

   int centre_block = w_init.map_size_blocks / 2;
   int dwi = 0;

#define GREEN_1_DIST_SHORT 12
#define GREEN_1_DIST_MED 14
#define GREEN_1_DIST_LONG 28
#define GREEN_1_DIST_LONGEST 36


   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block - GREEN_1_DIST_SHORT,
																																															centre_block - GREEN_1_DIST_SHORT);

   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block - GREEN_1_DIST_LONG,
																																															centre_block - GREEN_1_DIST_MED);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block - GREEN_1_DIST_MED,
																																															centre_block - GREEN_1_DIST_LONG);
   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															centre_block - GREEN_1_DIST_LONGEST,
																																															centre_block - GREEN_1_DIST_LONGEST);
   set_player_spawn_position_by_latest_well(1, 1000, DEFAULT_DISTANCE_FROM_WELL);

			add_line_between_data_wells(0, 1, 90);
			add_line_between_data_wells(1, 3, 90);
			add_line_between_data_wells(3, 2, 90);
			add_line_between_data_wells(2, 0, 90);


   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block + GREEN_1_DIST_SHORT,
																																															centre_block + GREEN_1_DIST_SHORT);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block + GREEN_1_DIST_LONG,
																																															centre_block + GREEN_1_DIST_MED);
   set_player_spawn_position_by_latest_well(0, 5000, DEFAULT_DISTANCE_FROM_WELL);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block + GREEN_1_DIST_MED,
																																															centre_block + GREEN_1_DIST_LONG);
   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															centre_block + GREEN_1_DIST_LONGEST,
																																															centre_block + GREEN_1_DIST_LONGEST);

			add_line_between_data_wells(4, 5, 90);
			add_line_between_data_wells(5, 7, 90);
			add_line_between_data_wells(7, 6, 90);
			add_line_between_data_wells(6, 4, 90);


   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block + GREEN_1_DIST_SHORT,
																																															centre_block - GREEN_1_DIST_SHORT);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block + GREEN_1_DIST_LONG,
																																															centre_block - GREEN_1_DIST_MED);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block + GREEN_1_DIST_MED,
																																															centre_block - GREEN_1_DIST_LONG);
   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															centre_block + GREEN_1_DIST_LONGEST,
																																															centre_block - GREEN_1_DIST_LONGEST);

			add_line_between_data_wells(8, 9, 90);
			add_line_between_data_wells(9, 11, 90);
			add_line_between_data_wells(11, 10, 90);
			add_line_between_data_wells(10, 8, 90);


   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block - GREEN_1_DIST_SHORT,
																																															centre_block + GREEN_1_DIST_SHORT);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block - GREEN_1_DIST_LONG,
																																															centre_block + GREEN_1_DIST_MED);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block - GREEN_1_DIST_MED,
																																															centre_block + GREEN_1_DIST_LONG);
   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															centre_block - GREEN_1_DIST_LONGEST,
																																															centre_block + GREEN_1_DIST_LONGEST);

			add_line_between_data_wells(12, 13, 90);
			add_line_between_data_wells(13, 15, 90);
			add_line_between_data_wells(15, 14, 90);
			add_line_between_data_wells(14, 12, 90);

   w_init.starting_data_setting [1] = 3;

	 	}
	  break;



	 case MISSION_GREEN_2:
	 	{

	 	game.region_in_area_index = 1;

 		int template_index = 0;

   load_mission_source("story/green/green2/g4_base.c", 1, template_index++);
   load_mission_source("story/green/green2/g4_firebase.c", 1, template_index++);
   load_mission_source("story/green/green2/g4_builder.c", 1, template_index++);
   load_mission_source("story/green/green2/g4_spikebase.c", 1, template_index++);
   load_mission_source("story/green/green2/g4_outpost.c", 1, template_index++);
   load_mission_source("story/green/green2/g4_builder2.c", 1, template_index++);
   clear_remaining_templates(1, template_index);

   int centre_block_x, centre_block_y;
   int dwi = 0;

#define GREEN_2_CENTRE_DISTANCE 18
#define GREEN_2A_CENTRE_DISTANCE 16

#define GREEN_2_DIST_1 0
#define GREEN_2_DIST_2 10
#define GREEN_2_DIST_3 -3
#define GREEN_2_DIST_4 10
#define GREEN_2_DIST_5 11

   centre_block_x = (w_init.map_size_blocks / 2) - GREEN_2_CENTRE_DISTANCE;
   centre_block_y = (w_init.map_size_blocks / 2) - GREEN_2_CENTRE_DISTANCE;

   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_2,
																																															centre_block_y + GREEN_2_DIST_1);
   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_1,
																																															centre_block_y + GREEN_2_DIST_2);

   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_4,
																																															centre_block_y - GREEN_2_DIST_3);

   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_5,
																																															centre_block_y - GREEN_2_DIST_5);
   set_player_spawn_position_by_latest_well(1, 1000, DEFAULT_DISTANCE_FROM_WELL);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_3,
																																															centre_block_y - GREEN_2_DIST_4);


			add_line_between_data_wells(0, 1, 70);
			add_line_between_data_wells(1, 2, 70);
			add_line_between_data_wells(2, 3, 70);
			add_line_between_data_wells(3, 4, 70);
			add_line_between_data_wells(4, 0, 70);

   centre_block_x = (w_init.map_size_blocks / 2) + GREEN_2A_CENTRE_DISTANCE;
   centre_block_y = (w_init.map_size_blocks / 2) - GREEN_2A_CENTRE_DISTANCE;



#define GREEN_2A_DIST_1 8
#define GREEN_2A_DIST_2 5

   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x + GREEN_2A_DIST_1,
																																															centre_block_y - GREEN_2A_DIST_1);


   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x + GREEN_2A_DIST_2,
																																															centre_block_y + GREEN_2A_DIST_2);


   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															centre_block_x - GREEN_2A_DIST_1,
																																															centre_block_y + GREEN_2A_DIST_1);

   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x - GREEN_2A_DIST_2,
																																															centre_block_y - GREEN_2A_DIST_2);


			add_line_between_data_wells(5 + 0, 5 + 1, 70);
			add_line_between_data_wells(5 + 1, 5 + 2, 70);
			add_line_between_data_wells(5 + 2, 5 + 3, 70);
			add_line_between_data_wells(5 + 3, 5 + 0, 70);

   centre_block_x = (w_init.map_size_blocks / 2) + GREEN_2_CENTRE_DISTANCE;
   centre_block_y = (w_init.map_size_blocks / 2) + GREEN_2_CENTRE_DISTANCE;


   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_2,
																																															centre_block_y - GREEN_2_DIST_1);
   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_1,
																																															centre_block_y - GREEN_2_DIST_2);

   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_4,
																																															centre_block_y + GREEN_2_DIST_3);
   set_player_spawn_position_by_latest_well(0, 5000, DEFAULT_DISTANCE_FROM_WELL);

   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_5,
																																															centre_block_y + GREEN_2_DIST_5);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_3,
																																															centre_block_y + GREEN_2_DIST_4);




			add_line_between_data_wells(9 + 0, 9 + 1, 70);
			add_line_between_data_wells(9 + 1, 9 + 2, 70);
			add_line_between_data_wells(9 + 2, 9 + 3, 70);
			add_line_between_data_wells(9 + 3, 9 + 4, 70);
			add_line_between_data_wells(9 + 4, 9 + 0, 70);

   centre_block_x = (w_init.map_size_blocks / 2) - GREEN_2A_CENTRE_DISTANCE;
   centre_block_y = (w_init.map_size_blocks / 2) + GREEN_2A_CENTRE_DISTANCE;



   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x + GREEN_2A_DIST_1,
																																															centre_block_y - GREEN_2A_DIST_1);


   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x + GREEN_2A_DIST_2,
																																															centre_block_y + GREEN_2A_DIST_2);


   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															centre_block_x - GREEN_2A_DIST_1,
																																															centre_block_y + GREEN_2A_DIST_1);

   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x - GREEN_2A_DIST_2,
																																															centre_block_y - GREEN_2A_DIST_2);



			add_line_between_data_wells(14 + 0, 14 + 1, 70);
			add_line_between_data_wells(14 + 1, 14 + 2, 70);
			add_line_between_data_wells(14 + 2, 14 + 3, 70);
			add_line_between_data_wells(14 + 3, 14 + 0, 70);

   w_init.starting_data_setting [1] = 6;

	 	}
	  break;


	 case MISSION_GREEN_CAPITAL:
	 	{

	 	game.region_in_area_index = 2;

 		int template_index = 0;

   load_mission_source("story/green/green3/g5_base.c", 1, template_index++);
   load_mission_source("story/green/green3/g5_firebase.c", 1, template_index++);
   load_mission_source("story/green/green3/g5_builder.c", 1, template_index++);
   load_mission_source("story/green/green3/g5_spikebase.c", 1, template_index++);
   load_mission_source("story/green/green3/g5_outpost.c", 1, template_index++);
   load_mission_source("story/green/green3/g5_builder2.c", 1, template_index++);
   clear_remaining_templates(1, template_index);
/*
   template_index = 0;

   load_mission_source("story/yellow/yellow3/y3_base.c", 0, template_index++);
   load_mission_source("story/yellow/yellow3/y3_m_builder.c", 0, template_index++);
//   load_mission_source("story/yellow/yellow3/y3_harvest.c", 1, template_index++);
   load_mission_source("story/yellow/yellow3/y3_leader1.c", 0, template_index++);
//   load_mission_source("story/yellow/yellow3/y3_leader2.c", 1, template_index++);
   load_mission_source("story/yellow/yellow3/y3_follower.c", 0, template_index++);
   load_mission_source("story/yellow/yellow3/y3_minbase.c", 0, template_index++);
   load_mission_source("story/yellow/yellow3/y3_scout.c", 0, template_index++);
   load_mission_source("story/yellow/yellow3/y3_follower2.c", 0, template_index++);
   clear_remaining_templates(0, template_index);
   w_init.player_starting_data [0] = 2000;
*/

   int centre_block_x, centre_block_y;
   int dwi = 0;

#define GREEN_3_CENTRE_DISTANCE 19
#define GREEN_3A_CENTRE_DISTANCE 13



   centre_block_x = (w_init.map_size_blocks / 2) - GREEN_3_CENTRE_DISTANCE;
   centre_block_y = (w_init.map_size_blocks / 2) - GREEN_3_CENTRE_DISTANCE;

   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_2,
																																															centre_block_y + GREEN_2_DIST_1);
   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_1,
																																															centre_block_y + GREEN_2_DIST_2);

   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_4,
																																															centre_block_y - GREEN_2_DIST_3);

   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_5,
																																															centre_block_y - GREEN_2_DIST_5);
   set_player_spawn_position_by_latest_well(1, 1000, DEFAULT_DISTANCE_FROM_WELL);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_3,
																																															centre_block_y - GREEN_2_DIST_4);


			add_line_between_data_wells(0, 1, 70);
			add_line_between_data_wells(1, 2, 70);
			add_line_between_data_wells(2, 3, 70);
			add_line_between_data_wells(3, 4, 70);
			add_line_between_data_wells(4, 0, 70);

   centre_block_x = (w_init.map_size_blocks / 2) + GREEN_3A_CENTRE_DISTANCE;
   centre_block_y = (w_init.map_size_blocks / 2) - GREEN_3A_CENTRE_DISTANCE;


   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_2,
																																															centre_block_y + GREEN_2_DIST_1);
   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_1,
																																															centre_block_y + GREEN_2_DIST_2);

   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_4,
																																															centre_block_y - GREEN_2_DIST_3);

   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_5,
																																															centre_block_y - GREEN_2_DIST_5);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_3,
																																															centre_block_y - GREEN_2_DIST_4);



			add_line_between_data_wells(5 + 0, 5 + 1, 70);
			add_line_between_data_wells(5 + 1, 5 + 2, 70);
			add_line_between_data_wells(5 + 2, 5 + 3, 70);
			add_line_between_data_wells(5 + 3, 5 + 4, 70);
			add_line_between_data_wells(5 + 4, 5 + 0, 70);

   centre_block_x = (w_init.map_size_blocks / 2) + GREEN_3_CENTRE_DISTANCE;
   centre_block_y = (w_init.map_size_blocks / 2) + GREEN_3_CENTRE_DISTANCE;


   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_2,
																																															centre_block_y - GREEN_2_DIST_1);
   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_1,
																																															centre_block_y - GREEN_2_DIST_2);

   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_4,
																																															centre_block_y + GREEN_2_DIST_3);
   set_player_spawn_position_by_latest_well(0, 5000, DEFAULT_DISTANCE_FROM_WELL);

   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_5,
																																															centre_block_y + GREEN_2_DIST_5);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_3,
																																															centre_block_y + GREEN_2_DIST_4);




			add_line_between_data_wells(10 + 0, 10 + 1, 70);
			add_line_between_data_wells(10 + 1, 10 + 2, 70);
			add_line_between_data_wells(10 + 2, 10 + 3, 70);
			add_line_between_data_wells(10 + 3, 10 + 4, 70);
			add_line_between_data_wells(10 + 4, 10 + 0, 70);

   centre_block_x = (w_init.map_size_blocks / 2) - GREEN_3A_CENTRE_DISTANCE;
   centre_block_y = (w_init.map_size_blocks / 2) + GREEN_3A_CENTRE_DISTANCE;


   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_2,
																																															centre_block_y - GREEN_2_DIST_1);
   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block_x + GREEN_2_DIST_1,
																																															centre_block_y - GREEN_2_DIST_2);

   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_4,
																																															centre_block_y + GREEN_2_DIST_3);

   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_5,
																																															centre_block_y + GREEN_2_DIST_5);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block_x - GREEN_2_DIST_3,
																																															centre_block_y + GREEN_2_DIST_4);



			add_line_between_data_wells(15 + 0, 15 + 1, 70);
			add_line_between_data_wells(15 + 1, 15 + 2, 70);
			add_line_between_data_wells(15 + 2, 15 + 3, 70);
			add_line_between_data_wells(15 + 3, 15 + 4, 70);
			add_line_between_data_wells(15 + 4, 15 + 0, 70);

   w_init.starting_data_setting [1] = 6;

	 	}
	  break;




	 case MISSION_YELLOW_1:
	 	{

	 	game.region_in_area_index = 0;

   int template_index = 0;

   load_mission_source("story/yellow/yellow1/y1_base.c", 1, template_index++);
   load_mission_source("story/yellow/yellow1/y1_m_builder.c", 1, template_index++);
   load_mission_source("story/yellow/yellow1/y1_harvest.c", 1, template_index++);
   load_mission_source("story/yellow/yellow1/y1_leader1.c", 1, template_index++);
   load_mission_source("story/yellow/yellow1/y1_leader2.c", 1, template_index++);
   load_mission_source("story/yellow/yellow1/y1_follower.c", 1, template_index++);
   load_mission_source("story/yellow/yellow1/y1_minbase.c", 1, template_index++);
   load_mission_source("story/yellow/yellow1/y1_scout.c", 1, template_index++);
   clear_remaining_templates(1, template_index);

   int centre_block = w_init.map_size_blocks / 2;

	 	player_base_x = 25;
	 	player_base_y = 25;
//	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);
   int dwi = 0;

   data_well_index [dwi++] = mission_add_data_well(0, 1, player_base_x, player_base_y);
   set_player_spawn_position_by_latest_well(0, ANGLE_8, DEFAULT_DISTANCE_FROM_WELL);

   data_well_index [dwi++] = mission_add_data_well(1,
																																															1,
																																															player_base_x + 20,
																																															player_base_y + 5);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															1,
																																															player_base_x + 5,
																																															player_base_y + 20);

   data_well_index [dwi++] = mission_add_data_well(2,
																																															1,
																																															centre_block + 15,
																																															centre_block - 15);
   data_well_index [dwi++] = mission_add_data_well(2,
																																															-1,
																																															centre_block - 15,
																																															centre_block + 15);

   data_well_index [dwi++] = mission_add_data_well(1,
																																															1,
																																															centre_block + 15,
																																															centre_block - 45);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block - 15,
																																															centre_block + 45);

   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block + 45,
																																															centre_block - 32);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															1,
																																															centre_block - 45,
																																															centre_block + 32);



   data_well_index [dwi++] = mission_add_data_well(0, -1, w_init.map_size_blocks - 25, w_init.map_size_blocks - 25);
   set_player_spawn_position_by_latest_well(1, -ANGLE_8, DEFAULT_DISTANCE_FROM_WELL);

   data_well_index [dwi++] = mission_add_data_well(0, -1, w_init.map_size_blocks - 30, w_init.map_size_blocks - 45);
   data_well_index [dwi++] = mission_add_data_well(0, -1, w_init.map_size_blocks - 45, w_init.map_size_blocks - 30);


   w_init.starting_data_setting [1] = 1;

	 	}
	  break;



	 case MISSION_YELLOW_2:
	 	{

	 	game.region_in_area_index = 1;

   int template_index = 0;

   load_mission_source("story/yellow/yellow2/y2_base.c", 1, template_index++);
   load_mission_source("story/yellow/yellow2/y2_m_builder.c", 1, template_index++);
   load_mission_source("story/yellow/yellow2/y2_harvest.c", 1, template_index++);
   load_mission_source("story/yellow/yellow2/y2_leader1.c", 1, template_index++);
   load_mission_source("story/yellow/yellow2/y2_leader2.c", 1, template_index++);
   load_mission_source("story/yellow/yellow2/y2_follower.c", 1, template_index++);
   load_mission_source("story/yellow/yellow2/y2_minbase.c", 1, template_index++);
   load_mission_source("story/yellow/yellow2/y2_scout.c", 1, template_index++);
   clear_remaining_templates(1, template_index);

   int centre_block = w_init.map_size_blocks / 2;

   w_init.starting_data_setting [1] = 3;

	 	player_base_x = 25;
	 	player_base_y = 25;
//	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);
   int dwi = 0;

   data_well_index [dwi++] = mission_add_data_well(0, 1, player_base_x, player_base_y);
   set_player_spawn_position_by_latest_well(0, ANGLE_8, DEFAULT_DISTANCE_FROM_WELL);


   data_well_index [dwi++] = mission_add_data_well(1,
																																															1,
																																															player_base_x + 30,
																																															player_base_y + 3);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															1,
																																															player_base_x + 3,
																																															player_base_y + 30);

   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															centre_block,
																																															centre_block);


   data_well_index [dwi++] = mission_add_data_well(1,
																																															1,
																																															centre_block + 25,
																																															centre_block - 42);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block - 25,
																																															centre_block + 42);

   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															centre_block + 43,
																																															centre_block - 29);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															1,
																																															centre_block - 43,
																																															centre_block + 29);



   data_well_index [dwi++] = mission_add_data_well(0, -1, w_init.map_size_blocks - 20, w_init.map_size_blocks - 20);
   set_player_spawn_position_by_latest_well(1, -ANGLE_8, DEFAULT_DISTANCE_FROM_WELL);

   data_well_index [dwi++] = mission_add_data_well(0, -1, w_init.map_size_blocks - 35, w_init.map_size_blocks - 59);
	 	add_extra_spawn(1, 6,
																			w_init.map_size_blocks - 30,
																			w_init.map_size_blocks - 59,
																			4096);
   data_well_index [dwi++] = mission_add_data_well(0, -1, w_init.map_size_blocks - 59, w_init.map_size_blocks - 35);
	 	add_extra_spawn(1, 6,
																			w_init.map_size_blocks - 59,
																			w_init.map_size_blocks - 30,
																			-2048);

   data_well_index [dwi++] = mission_add_data_well(1, -1, w_init.map_size_blocks - 16, w_init.map_size_blocks - 43);
   data_well_index [dwi++] = mission_add_data_well(1, -1, w_init.map_size_blocks - 43, w_init.map_size_blocks - 16);



	 	}
	  break;


	 case MISSION_YELLOW_CAPITAL:
	 	{

	 	game.region_in_area_index = 2;

   int template_index = 0;

   load_mission_source("story/yellow/yellow3/y3_base.c", 1, template_index++);
   load_mission_source("story/yellow/yellow3/y3_m_builder.c", 1, template_index++);
   load_mission_source("story/yellow/yellow3/y3_harvest.c", 1, template_index++);
   load_mission_source("story/yellow/yellow3/y3_leader1.c", 1, template_index++);
   load_mission_source("story/yellow/yellow3/y3_leader2.c", 1, template_index++);
   load_mission_source("story/yellow/yellow3/y3_follower.c", 1, template_index++);
   load_mission_source("story/yellow/yellow3/y3_minbase.c", 1, template_index++);
   load_mission_source("story/yellow/yellow3/y3_scout.c", 1, template_index++);

   clear_remaining_templates(1, template_index);


   w_init.starting_data_setting [1] = 4;

//   int centre_block = w_init.map_size_blocks / 2;

	 	player_base_x = 35;
	 	player_base_y = 35;
//	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);
   int dwi = 0;

   data_well_index [dwi++] = mission_add_data_well(0, 1, player_base_x, player_base_y);
   set_player_spawn_position_by_latest_well(0, ANGLE_8, DEFAULT_DISTANCE_FROM_WELL);


   data_well_index [dwi++] = mission_add_data_well(1,
																																															1,
																																															player_base_x - 21,
																																															player_base_y + 3);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															1,
																																															player_base_x + 3,
																																															player_base_y - 21);

   int main_block = 70;

   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															main_block,
																																															main_block);


   data_well_index [dwi++] = mission_add_data_well(1,
																																															1,
																																															main_block - 17,
																																															main_block - 17);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															1,
																																															main_block + 17,
																																															main_block - 17);
/*
	 	add_extra_spawn(1, 4,
																			main_block + 14,
																			main_block - 14,
																			3072);
	 	add_extra_spawn(1, 4,
																			main_block - 14,
																			main_block + 14,
																			-1024);
*/

   data_well_index [dwi++] = mission_add_data_well(1,
																																															1,
																																															main_block - 17,
																																															main_block + 17);
   data_well_index [dwi++] = mission_add_data_well(0,
																																															1,
																																															main_block + 17,
																																															main_block + 17);
   set_player_spawn_position_by_latest_well(1, ANGLE_2+ANGLE_8, DEFAULT_DISTANCE_FROM_WELL);

   data_well_index [dwi++] = mission_add_data_well(0,
																																															1,
																																															main_block + 32,
																																															main_block + 32);
	 	add_extra_spawn(1, 6,
																			main_block + 36,
																			main_block + 36,
																			4096 + 1024);
	 	add_extra_spawn(1, 6,
																			main_block + 4,
																			main_block + 4,
																			4096 + 1024);


   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															main_block + 38,
																																															main_block);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															main_block - 38,
																																															main_block);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															main_block,
																																															main_block + 38);
   data_well_index [dwi++] = mission_add_data_well(1,
																																															-1,
																																															main_block,
																																															main_block - 38);


   data_well_index [dwi++] = mission_add_data_well(2,
																																															1,
																																															main_block - 40,
																																															main_block + 33);
   data_well_index [dwi++] = mission_add_data_well(2,
																																															1,
																																															main_block + 33,
																																															main_block - 40);




	 	}
	  break;


	 case MISSION_PURPLE_1:
	 	{

	 	game.region_in_area_index = 0;

   int template_index = 0;

   load_mission_source("story/purple/purple1/p1_base.c", 1, template_index++);
   load_mission_source("story/purple/purple1/p1_m_builder.c", 1, template_index++);
   load_mission_source("story/purple/purple1/p1_harvest.c", 1, template_index++);
   load_mission_source("story/purple/purple1/p1_minbase.c", 1, template_index++);
   load_mission_source("story/purple/purple1/p1_leader1.c", 1, template_index++);
   load_mission_source("story/purple/purple1/p1_leader2.c", 1, template_index++);
   load_mission_source("story/purple/purple1/p1_escort.c", 1, template_index++);
   load_mission_source("story/purple/purple1/p1_picket.c", 1, template_index++);
   load_mission_source("story/purple/purple1/p1_defence.c", 1, template_index++);
   clear_remaining_templates(1, template_index);

   int centre_block = w_init.map_size_blocks / 2;

   w_init.starting_data_setting [1] = 11;

//	 	player_base_x = 25;
//	 	player_base_y = centre_block;
//	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);
   int dwi = 0;

#define MP1_SHORT 12
#define MP1_LONG 25

   data_well_index [dwi++] = mission_add_data_well(0, 1, 15, centre_block);
   set_player_spawn_position_by_latest_well(0, 0, DEFAULT_DISTANCE_FROM_WELL);

   data_well_index [dwi++] = mission_add_data_well(1, 1, 15 + MP1_SHORT, centre_block - MP1_LONG);
   data_well_index [dwi++] = mission_add_data_well(1, 1, 15 + MP1_LONG, centre_block + MP1_SHORT);

   data_well_index [dwi++] = mission_add_data_well(0, 1, centre_block, 15);
   data_well_index [dwi++] = mission_add_data_well(1, 1, centre_block - MP1_SHORT, 15 + MP1_LONG);
   data_well_index [dwi++] = mission_add_data_well(1, 1, centre_block + MP1_LONG, 15 + MP1_SHORT);

   data_well_index [dwi++] = mission_add_data_well(0, 1, centre_block, w_init.map_size_blocks - 15);
   data_well_index [dwi++] = mission_add_data_well(1, 1, centre_block - MP1_LONG, w_init.map_size_blocks - 15 - MP1_SHORT);
   data_well_index [dwi++] = mission_add_data_well(1, 1, centre_block + MP1_SHORT, w_init.map_size_blocks - 15 - MP1_LONG);

   data_well_index [dwi++] = mission_add_data_well(0, 1, w_init.map_size_blocks - 15, centre_block);
   set_player_spawn_position_by_latest_well(1, ANGLE_2, 700);

   data_well_index [dwi++] = mission_add_data_well(1, 1, w_init.map_size_blocks - 15 - MP1_LONG, centre_block - MP1_SHORT);
   data_well_index [dwi++] = mission_add_data_well(1, 1, w_init.map_size_blocks - 15 - MP1_SHORT, centre_block + MP1_LONG);

   add_mdetail_worm_source_to_all_wells();

//   add_mdetail_worm_source(player_base_x, player_base_y, 15);

/*


   data_well_index [1] = mission_add_data_well(1,
																																															1,
																																															player_base_x + 20,
																																															player_base_y);
   add_mdetail_worm_source(player_base_x + 20, player_base_y, 15);
   data_well_index [2] = mission_add_data_well(1,
																																															1,
																																															player_base_x,
																																															player_base_y + 20);
   add_mdetail_worm_source(player_base_x, player_base_y + 20, 15);
*/

//   data_well_index [dwi++] = mission_add_data_well(0, 1, w_init.map_size_blocks - 25, w_init.map_size_blocks - 25);



	 	}
	  break;


	 case MISSION_PURPLE_2:
	 	{

	 	game.region_in_area_index = 1;

   int template_index = 0;

   load_mission_source("story/purple/purple2/p2_base.c", 1, template_index++);
   load_mission_source("story/purple/purple2/p2_m_builder.c", 1, template_index++);
   load_mission_source("story/purple/purple2/p2_harvest.c", 1, template_index++);
   load_mission_source("story/purple/purple2/p2_minbase.c", 1, template_index++);
   load_mission_source("story/purple/purple2/p2_leader1.c", 1, template_index++);
   load_mission_source("story/purple/purple2/p2_leader2.c", 1, template_index++);
   load_mission_source("story/purple/purple2/p2_escort.c", 1, template_index++);
   load_mission_source("story/purple/purple2/p2_picket.c", 1, template_index++);
   load_mission_source("story/purple/purple2/p2_defence.c", 1, template_index++);
   clear_remaining_templates(1, template_index);

   w_init.starting_data_setting [1] = 12;


   int centre_block = w_init.map_size_blocks / 2;

//	 	player_base_x = 25;
//	 	player_base_y = centre_block;
//	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);
   int dwi = 0;

#define P2_CIRCLE_RADIUS 46

   data_well_index [dwi++] = mission_add_data_well_to_circle(0, ANGLE_2, P2_CIRCLE_RADIUS, 1, centre_block, centre_block);
   set_player_spawn_position_by_latest_well(0, 0, DEFAULT_DISTANCE_FROM_WELL);

   data_well_index [dwi++] = mission_add_data_well_to_circle(0, 0, P2_CIRCLE_RADIUS, -1, centre_block, centre_block);
   set_player_spawn_position_by_latest_well(1, ANGLE_2, 700);

   data_well_index [dwi++] = mission_add_data_well_to_circle(1, ANGLE_8, P2_CIRCLE_RADIUS, 1, centre_block, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(1, -ANGLE_8, P2_CIRCLE_RADIUS, 1, centre_block, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(1, ANGLE_4, P2_CIRCLE_RADIUS, 1, centre_block, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(1, -ANGLE_4, P2_CIRCLE_RADIUS, 1, centre_block, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(1, ANGLE_2 + ANGLE_8, P2_CIRCLE_RADIUS, 1, centre_block, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(1, ANGLE_2 - ANGLE_8, P2_CIRCLE_RADIUS, 1, centre_block, centre_block);

   data_well_index [dwi++] = mission_add_data_well_to_circle(2, ANGLE_4 - 600, P2_CIRCLE_RADIUS - 14, 1, centre_block, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(2, ANGLE_4 + 600, P2_CIRCLE_RADIUS - 14, 1, centre_block, centre_block);

   data_well_index [dwi++] = mission_add_data_well_to_circle(2, -ANGLE_4 - 600, P2_CIRCLE_RADIUS - 14, 1, centre_block, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(2, -ANGLE_4 + 600, P2_CIRCLE_RADIUS - 14, 1, centre_block, centre_block);

//   data_well_index [dwi++] = mission_add_data_well_to_circle(1, ANGLE_4, P2_CIRCLE_RADIUS - 19, 1, centre_block, centre_block);
//   data_well_index [dwi++] = mission_add_data_well_to_circle(1, -ANGLE_4, P2_CIRCLE_RADIUS - 19, 1, centre_block, centre_block);

   data_well_index [dwi++] = mission_add_data_well_to_circle(2, ANGLE_2 - 450, P2_CIRCLE_RADIUS - 9, 1, centre_block, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(2, ANGLE_2 + 450, P2_CIRCLE_RADIUS - 9, 1, centre_block, centre_block);

   data_well_index [dwi++] = mission_add_data_well_to_circle(2, -450, P2_CIRCLE_RADIUS - 9, 1, centre_block, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(2, 450, P2_CIRCLE_RADIUS - 9, 1, centre_block, centre_block);




   add_mdetail_worm_source_to_all_wells();

	 	}
	  break;


	 case MISSION_PURPLE_CAPITAL:
	 	{

	 	game.region_in_area_index = 2;


   int template_index = 0;

   load_mission_source("story/purple/purple3/p3_base.c", 1, template_index++);
   load_mission_source("story/purple/purple3/p3_m_builder.c", 1, template_index++);
   load_mission_source("story/purple/purple3/p3_harvest.c", 1, template_index++);
   load_mission_source("story/purple/purple3/p3_minbase.c", 1, template_index++);
   load_mission_source("story/purple/purple3/p3_leader1.c", 1, template_index++);
   load_mission_source("story/purple/purple3/p3_leader2.c", 1, template_index++);
   load_mission_source("story/purple/purple3/p3_escort.c", 1, template_index++);
   load_mission_source("story/purple/purple3/p3_picket.c", 1, template_index++);
   load_mission_source("story/purple/purple3/p3_picket2.c", 1, template_index++);
   load_mission_source("story/purple/purple3/p3_defence.c", 1, template_index++);
//   clear_remaining_templates(1, template_index);

   w_init.starting_data_setting [1] = 12;

   int centre_block = w_init.map_size_blocks / 2;

//	 	player_base_x = 25;
//	 	player_base_y = centre_block;
//	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);
   int dwi = 0;

#define P3_CIRCLE_RADIUS 43

   data_well_index [dwi++] = mission_add_data_well_to_circle(0, ANGLE_2, P3_CIRCLE_RADIUS, 1, centre_block + 4, centre_block);
   set_player_spawn_position_by_latest_well(0, 0, DEFAULT_DISTANCE_FROM_WELL);

   data_well_index [dwi++] = mission_add_data_well_to_circle(0, 0, P3_CIRCLE_RADIUS, -1, centre_block + 4, centre_block);

   data_well_index [dwi++] = mission_add_data_well_to_circle(1, ANGLE_2 - 900, P3_CIRCLE_RADIUS, -1, centre_block + 3, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(1, ANGLE_2 + 900, P3_CIRCLE_RADIUS, 1, centre_block + 3, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(1, ANGLE_2 - 1900, P3_CIRCLE_RADIUS, -1, centre_block + 3, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(1, ANGLE_2 + 1900, P3_CIRCLE_RADIUS, 1, centre_block + 3, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(1, ANGLE_2 - 2900, P3_CIRCLE_RADIUS, -1, centre_block + 3, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(1, ANGLE_2 + 2900, P3_CIRCLE_RADIUS, 1, centre_block + 3, centre_block);

   data_well_index [dwi++] = mission_add_data_well_to_circle(0, 600, P3_CIRCLE_RADIUS - 12, 1, centre_block + 3, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(0, -600, P3_CIRCLE_RADIUS - 12, -1, centre_block + 3, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(1, 1800, P3_CIRCLE_RADIUS - 11, -1, centre_block + 3, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(1, -1800, P3_CIRCLE_RADIUS - 11, 1, centre_block + 3, centre_block);

   data_well_index [dwi++] = mission_add_data_well_to_circle(1, 1500, P3_CIRCLE_RADIUS - 31, -1, centre_block + 3, centre_block);
   data_well_index [dwi++] = mission_add_data_well_to_circle(1, -1500, P3_CIRCLE_RADIUS - 31, 1, centre_block + 3, centre_block);

   data_well_index [dwi++] = mission_add_data_well_to_circle(0, 0, P3_CIRCLE_RADIUS - 26, -1, centre_block + 3, centre_block);
   set_player_spawn_position_by_latest_well(1, ANGLE_2, 700);



   add_mdetail_worm_source_to_all_wells();

	 	}
	  break;


	 case MISSION_ORANGE_1:
	 	{

	 	game.region_in_area_index = 0;

   int template_index = 0;

   load_mission_source("story/orange/orange1/o1_base.c", 1, template_index++);
   load_mission_source("story/orange/orange1/o1_harvest.c", 1, template_index++);
   load_mission_source("story/orange/orange1/o1_harvest.c", 1, template_index++);
   load_mission_source("story/orange/orange1/o1_harvest2.c", 1, template_index++);
   load_mission_source("story/orange/orange1/o1_harvest3.c", 1, template_index++);
   load_mission_source("story/orange/orange1/o1_guard1.c", 1, template_index++);
   load_mission_source("story/orange/orange1/o1_guard2.c", 1, template_index++);
   load_mission_source("story/orange/orange1/o1_guard3.c", 1, template_index++);
   clear_remaining_templates(1, template_index);

   int centre_block = w_init.map_size_blocks / 2;

   int dwi = 0;

#define ORANGE_LINE_THICKNESS_1 90
#define ORANGE_LINE_THICKNESS_2 40

	 	data_well_index [dwi++] = mission_add_data_well(0, -1, centre_block, centre_block);
   set_player_spawn_position_by_latest_well(1, ANGLE_2 + 600, DEFAULT_DISTANCE_FROM_WELL);

// lower right branch
   data_well_index [dwi++] = add_orange_data_well(0, 2, 900, 5000, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_1 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_1, 3, -700, 2800, ORANGE_LINE_THICKNESS_2);
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_1, 1, 1500, 2900, ORANGE_LINE_THICKNESS_2);
   set_player_spawn_position_by_latest_well(0, ANGLE_2 + ANGLE_8, DEFAULT_DISTANCE_FROM_WELL);

// upper right
   data_well_index [dwi++] = add_orange_data_well(0, 2, -500, 5000, ORANGE_LINE_THICKNESS_1);

// top
   data_well_index [dwi++] = add_orange_data_well(0, 2, -1450, 4000, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_2 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_2, 3, -900, 3200, ORANGE_LINE_THICKNESS_2);
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_2, 3, -3000, 3000, ORANGE_LINE_THICKNESS_2);

// upper left
   data_well_index [dwi++] = add_orange_data_well(0, 2, 5800, 3700, ORANGE_LINE_THICKNESS_1);

// upper left
   data_well_index [dwi++] = add_orange_data_well(0, 2, 5000, 3600, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_3 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_3, 3, 3400, 3000, ORANGE_LINE_THICKNESS_2);
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_3, 3, -2600, 2600, ORANGE_LINE_THICKNESS_2);
   int sub_centre_well_3A = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_3A, 3, -3200, 2000, ORANGE_LINE_THICKNESS_2);

// left
   data_well_index [dwi++] = add_orange_data_well(0, 2, 3800, 3800, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_4 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_4, 3, 3200, 2200, ORANGE_LINE_THICKNESS_2);

// down
   data_well_index [dwi++] = add_orange_data_well(0, 2, 2700, 3400, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_5 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_5, 3, 1200, 2200, ORANGE_LINE_THICKNESS_2);
   int sub_centre_well_6 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_6, 3, 1200, 2200, ORANGE_LINE_THICKNESS_2);
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_6, 3, 3400, 2200, ORANGE_LINE_THICKNESS_2);

   w_init.starting_data_setting [1] = 5;

	 	}
	  break;



	 case MISSION_ORANGE_2:
	 	{

	 	game.region_in_area_index = 1;

   int template_index = 0;

   load_mission_source("story/orange/orange2/o2_base.c", 1, template_index++);
   load_mission_source("story/orange/orange2/o2_defend.c", 1, template_index++);
   load_mission_source("story/orange/orange2/o2_harvest.c", 1, template_index++);
   load_mission_source("story/orange/orange2/o2_harvest2.c", 1, template_index++);
   load_mission_source("story/orange/orange2/o2_harvest3.c", 1, template_index++);
   load_mission_source("story/orange/orange2/o2_guard1.c", 1, template_index++);
   load_mission_source("story/orange/orange2/o2_guard2.c", 1, template_index++);
   load_mission_source("story/orange/orange2/o2_guard3.c", 1, template_index++);
   clear_remaining_templates(1, template_index);

   int centre_block = w_init.map_size_blocks / 2;

   int dwi = 0;

#define ORANGE_LINE_THICKNESS_1 90
#define ORANGE_LINE_THICKNESS_2 40

	 	data_well_index [dwi++] = mission_add_data_well(0, -1, centre_block, centre_block);
   set_player_spawn_position_by_latest_well(1, ANGLE_2 + 600, DEFAULT_DISTANCE_FROM_WELL);

// lower right branch
   data_well_index [dwi++] = add_orange_data_well(0, 2, 1500, 4000, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_1 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_1, 1, 400, 2900, ORANGE_LINE_THICKNESS_2);
   set_player_spawn_position_by_latest_well(0, ANGLE_2 + ANGLE_8, DEFAULT_DISTANCE_FROM_WELL);

// right
   data_well_index [dwi++] = add_orange_data_well(0, 2, -200, 3000, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_1A = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_1A, 3, -900, 2500, ORANGE_LINE_THICKNESS_2);
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_1A, 3, 800, 2200, ORANGE_LINE_THICKNESS_2);
   int sub_centre_well_1B = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_1B, 3, 400, 1800, ORANGE_LINE_THICKNESS_2);

// top
   data_well_index [dwi++] = add_orange_data_well(0, 2, -1250, 4500, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_2 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_2, 3, -700, 3000, ORANGE_LINE_THICKNESS_2);
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_2, 3, -3000, 3000, ORANGE_LINE_THICKNESS_2);
   int sub_centre_well_2A = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_2A, 3, -4000, 3000, ORANGE_LINE_THICKNESS_2);

// upper left
   data_well_index [dwi++] = add_orange_data_well(0, 2, 5800, 3700, ORANGE_LINE_THICKNESS_1);

// upper left
   data_well_index [dwi++] = add_orange_data_well(0, 2, 5000, 3800, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_3 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_3, 3, 3400, 2800, ORANGE_LINE_THICKNESS_2);
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_3, 3, -2900, 3200, ORANGE_LINE_THICKNESS_2);

// left
   data_well_index [dwi++] = add_orange_data_well(0, 2, 3600, 4900, ORANGE_LINE_THICKNESS_1);

// down
   data_well_index [dwi++] = add_orange_data_well(0, 2, 2900, 3700, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_5 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_5, 3, 1400, 2900, ORANGE_LINE_THICKNESS_2);
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_5, 3, 2900, 3700, ORANGE_LINE_THICKNESS_2);

   w_init.starting_data_setting [1] = 7;

	 	}
	  break;


	 case MISSION_ORANGE_CAPITAL:
	 	{

	 	game.region_in_area_index = 2;

   int template_index = 0;

   load_mission_source("story/orange/orange3/o3_base.c", 1, template_index++);
   load_mission_source("story/orange/orange3/o3_defend.c", 1, template_index++);
   load_mission_source("story/orange/orange3/o3_harvest.c", 1, template_index++);
   load_mission_source("story/orange/orange3/o3_harvest2.c", 1, template_index++);
   load_mission_source("story/orange/orange3/o3_harvest3.c", 1, template_index++);
   load_mission_source("story/orange/orange3/o3_guard1.c", 1, template_index++);
   load_mission_source("story/orange/orange3/o3_guard2.c", 1, template_index++);
   load_mission_source("story/orange/orange3/o3_guard3.c", 1, template_index++);
   clear_remaining_templates(1, template_index);

   int centre_block = w_init.map_size_blocks / 2;

   int dwi = 0;

#define ORANGE_LINE_THICKNESS_1 90
#define ORANGE_LINE_THICKNESS_2 40

	 	data_well_index [dwi++] = mission_add_data_well(0, -1, centre_block, centre_block);
   set_player_spawn_position_by_latest_well(1, ANGLE_2 + 600, DEFAULT_DISTANCE_FROM_WELL);

// lower right branch
   data_well_index [dwi++] = add_orange_data_well(0, 1, 900, 6900, ORANGE_LINE_THICKNESS_1);
   set_player_spawn_position_by_latest_well(0, ANGLE_2 + ANGLE_8, DEFAULT_DISTANCE_FROM_WELL);

// right
   data_well_index [dwi++] = add_orange_data_well(0, 2, -400, 4000, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_1A = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_1A, 3, 800, 2900, ORANGE_LINE_THICKNESS_2);
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_1A, 3, -900, 2300, ORANGE_LINE_THICKNESS_2);

// top
   data_well_index [dwi++] = add_orange_data_well(0, 2, -1500, 3500, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_2 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_2, 3, -850, 3000, ORANGE_LINE_THICKNESS_2);

// upper left
   data_well_index [dwi++] = add_orange_data_well(0, 2, 5500, 3300, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_2A = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_2A, 2, -1200, 3200, ORANGE_LINE_THICKNESS_2);
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_2A, 2, 4900, 3300, ORANGE_LINE_THICKNESS_2);

// left
   data_well_index [dwi++] = add_orange_data_well(0, 2, 4300, 3200, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_3 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_3, 3, 4700, 2400, ORANGE_LINE_THICKNESS_2);
   int sub_centre_well_3A = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_3A, 3, 2400, 2800, ORANGE_LINE_THICKNESS_2);

// down left
   data_well_index [dwi++] = add_orange_data_well(0, 2, 3200, 4100, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_4 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_4, 3, 3500, 3000, ORANGE_LINE_THICKNESS_2);

// down
   data_well_index [dwi++] = add_orange_data_well(0, 2, 1950, 3700, ORANGE_LINE_THICKNESS_1);
   int sub_centre_well_5 = dwi - 1;
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_5, 3, 900, 3200, ORANGE_LINE_THICKNESS_2);
   data_well_index [dwi++] = add_orange_data_well(sub_centre_well_5, 3, 3100, 3700, ORANGE_LINE_THICKNESS_2);

   w_init.starting_data_setting [1] = 9;

	 	}
	  break;


	 case MISSION_RED_1:
	 	{

	 	game.region_in_area_index = 0;

   int template_index = 0;

   load_mission_source("story/red/red1/r1_base.c", 1, template_index++);
//   load_mission_source("story/red/red1/r1_leader1.c", 1, template_index++);
   load_mission_source("story/red/red1/r1_m_builder.c", 1, template_index++);
   load_mission_source("story/red/red1/r1_leader1.c", 1, template_index++);
   load_mission_source("story/red/red1/r1_follower.c", 1, template_index++);
   load_mission_source("story/red/red1/r1_minbase.c", 1, template_index++);
   load_mission_source("story/red/red1/r1_scout.c", 1, template_index++);
   load_mission_source("story/red/red1/r1_follower2.c", 1, template_index++);

   int centre_block = w_init.map_size_blocks / 2;
   int dwi = 0;

//   int player_spawn_x = centre_block - 12;
//   int enemy_spawn_x = centre_block + 12;

   int ring_centre_x = centre_block - 10;
   int ring_centre_y = 42;

   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															ring_centre_x,
																																															ring_centre_y);

   set_player_spawn_position_by_latest_well(1, 1024, DEFAULT_DISTANCE_FROM_WELL);

   int base_angle = 450;

   mission_add_red_data_well_ring(1, ring_centre_x, ring_centre_y, 16, 6, base_angle + ANGLE_1 / 12);
//   mission_add_red_data_well_ring(1, ring_centre_x, ring_centre_y, 28, 6, base_angle);
   mission_add_red_data_well_ring(1, ring_centre_x, ring_centre_y, 44, 10, base_angle + ANGLE_1 / 20);
   mission_add_red_data_well_ring(1, ring_centre_x, ring_centre_y, 58, 10, base_angle);
// dwi isn't useable after thism


   set_player_spawn_position_by_specified_well(0, 16, -1024, DEFAULT_DISTANCE_FROM_WELL);


   w_init.starting_data_setting [1] = 12;

	 	}
	  break;


	 case MISSION_RED_2:
	 	{

	 	game.region_in_area_index = 1;

   int template_index = 0;

   load_mission_source("story/red/red2/r2_base.c", 1, template_index++);
   load_mission_source("story/red/red2/r2_m_builder.c", 1, template_index++);
   load_mission_source("story/red/red2/r2_harvest.c", 1, template_index++);
   load_mission_source("story/red/red2/r2_minbase.c", 1, template_index++);
   load_mission_source("story/red/red2/r2_leader1.c", 1, template_index++);
   load_mission_source("story/red/red2/r2_leader2.c", 1, template_index++);
   load_mission_source("story/red/red2/r2_escort.c", 1, template_index++);
   load_mission_source("story/red/red2/r2_picket.c", 1, template_index++);
   load_mission_source("story/red/red2/r2_defence.c", 1, template_index++);

   clear_remaining_templates(1, template_index);
   int centre_block = w_init.map_size_blocks / 2;
   int dwi = 0;

//   int player_spawn_x = centre_block - 12;
//   int enemy_spawn_x = centre_block + 12;

   int ring_centre_x = centre_block + 10;
   int ring_centre_y = 50;

   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															ring_centre_x,
																																															ring_centre_y);

   set_player_spawn_position_by_latest_well(1, 1024, DEFAULT_DISTANCE_FROM_WELL);

   int base_angle = 250;

   mission_add_red_data_well_ring(1, ring_centre_x, ring_centre_y, 16, 6, base_angle + ANGLE_1 / 12);
   mission_add_red_data_well_ring(1, ring_centre_x, ring_centre_y, 28, 6, base_angle);
   mission_add_red_data_well_ring(1, ring_centre_x, ring_centre_y, 42, 6, base_angle + ANGLE_1 / 12);
   mission_add_red_data_well_ring(1, ring_centre_x, ring_centre_y, 58, 6, base_angle);
// dwi isn't useable after this


   set_player_spawn_position_by_specified_well(0, 19, -1024, DEFAULT_DISTANCE_FROM_WELL);


   w_init.starting_data_setting [1] = 12;

	 	}
	  break;




	 case MISSION_RED_CAPITAL:
	 	{

	 	game.region_in_area_index = 2;

   int template_index = 0;

   load_mission_source("story/red/red3/r3_base.c", 1, template_index++);
   load_mission_source("story/red/red3/r3_m_builder.c", 1, template_index++);
   load_mission_source("story/red/red3/r3_harvest.c", 1, template_index++);
   load_mission_source("story/red/red3/r3_minbase.c", 1, template_index++);
   load_mission_source("story/red/red3/r3_leader1.c", 1, template_index++);
   load_mission_source("story/red/red3/r3_leader2.c", 1, template_index++);
   load_mission_source("story/red/red3/r3_escort.c", 1, template_index++);
   load_mission_source("story/red/red3/r3_picket.c", 1, template_index++);
   load_mission_source("story/red/red3/r3_defence.c", 1, template_index++);

   w_init.starting_data_setting [1] = 20;

   clear_remaining_templates(1, template_index);

   int centre_block = w_init.map_size_blocks / 2;
//   int dwi = 0;

//   int player_spawn_x = centre_block - 12;
//   int enemy_spawn_x = centre_block + 12;

   int ring_centre_x = centre_block;
   int ring_centre_y = centre_block;

   int dwi = 0;

   data_well_index [dwi++] = mission_add_data_well(0,
																																															-1,
																																															ring_centre_x,
																																															ring_centre_y);

   set_player_spawn_position_by_latest_well(1, 1024, DEFAULT_DISTANCE_FROM_WELL);

//   set_player_spawn_position(1, centre_block, centre_block, 0);
//   set_player_spawn_position_by_latest_well(1, -2048, DEFAULT_DISTANCE_FROM_WELL);

   int base_angle = 450;

   mission_add_red_data_well_ring(0, ring_centre_x, ring_centre_y, 16, 6, base_angle + ANGLE_1 / 12);
   mission_add_red_data_well_ring(1, ring_centre_x, ring_centre_y, 30, 6, base_angle);
   mission_add_red_data_well_ring(1, ring_centre_x, ring_centre_y, 46, 6, base_angle + ANGLE_1 / 12);
//   mission_add_red_data_well_ring(1, ring_centre_x, ring_centre_y, 54, 6, base_angle);
// dwi isn't useable after thism


   set_player_spawn_position_by_specified_well(0, 13, -1024, DEFAULT_DISTANCE_FROM_WELL);



	 	}
	  break;

	 default:
			{
// This should never happen. It just gets rid of an annoying compiler warning:
   w_init.starting_data_setting [1] = data_well_index [0];
			}
			break;

/*

Plan for orange:

single rich data well in centre, or a bit off centre
 - with smaller ones radiating outwards
  - outer wells have large reserves (maybe?) but extremely slow replenishment

powerful base in centre
 - no outposts get built
  - maybe capital has an inner ring of wells with outposts pre-built

enemies are harvesters with large capacity.
 - they wander clockwise (just inside ring of wells) or anticlockwise (just outside - to avoid collision)
 - when near full, they request a freighter from main base (need to implement "take_data(target)")
also medium-sized escorts

guards follow harvesters, and respond to attacks.
 - also sometimes seek out enemies.

* actually the circles idea may not really work.
- how about:
 - main base keeps track of all data wells
  - discovered by sending harvesters out on scouting missions
 - also keeps track of time since last harvest
 - when a harvester in scout mode finds a data well
Done.
- Now, how does it attack?
 - builds guards that orbit main base
  - when harvesters finish dropping off data, they request followers.
  - each follower has a chance to follow.
	- when any process is under attack, it broadcasts a help-wanted signal with range of about 3000
	 - or if it finds a priority target, broadcasts it with longer range.
	- attackers follow.
- Types of attackers:
 - one attacks straightforwardly, using approach_target to about 800. Could use burst.
  - very heavily interfaced, strong repair.
 - one uses circle.
  - maybe could use long-range circle with spike
 - one chooses an attack vector based on enemy movement
  - every x cycles, uses current enemy movement to try to guess which side is its back.
   - circles around to that angle, then attacks random components using slice or stream.
  - tries to attack large targets only, if possible.



procs:

main base
defence
harvester_1 // fast - built early for scouting
harvester_2 // bigger
harvester_3 // biggest

guard_1
guard_2


*/




/*

	 case MISSION_GREEN_2:
	 	{


   load_mission_source("story/green/green2/g2_base.c", 1, 0);
   load_mission_source("story/green/green2/g2_firebase.c", 1, 1);
   load_mission_source("story/green/green2/g2_builder.c", 1, 2);
   clear_remaining_templates(1, 3);

   int centre_block = w_init.map_size_blocks / 2;

   int ring_index_1 = add_mdetail_ring(centre_block, centre_block, 24, 0);
   int ring_index_2 = add_mdetail_ring(centre_block - 28, centre_block - 28, 8, 0);
   int ring_index_3 = add_mdetail_ring(centre_block + 28, centre_block + 28, 8, 0);

   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_2 + ANGLE_8, 2000, 1000, 3, 0.002);
   set_player_spawn_position_by_latest_well(0, ANGLE_8);

   add_data_well_to_mdetail_ring(ring_index_2,	ANGLE_2 + ANGLE_8, 2000, 1000, 3, 0.002);


   add_data_well_to_mdetail_ring(ring_index_1, ANGLE_8, 2000, 1000, 3, -0.002);
   set_player_spawn_position_by_latest_well(1, ANGLE_2 + ANGLE_8);

   add_data_well_to_mdetail_ring(ring_index_3, ANGLE_8, 2000, 1000, 3, -0.002);


   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 2, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 3, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 4, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 6, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 7, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	0, 2000, 1000, 3, 0.002);


   w_init.player_starting_data [1] = 600;

	 	}
	  break;



	 case MISSION_GREEN_CAPITAL:
	 	{

   load_mission_source("story/green/green5/g5_base.c", 1, 0);
   load_mission_source("story/green/green5/g5_firebase.c", 1, 1);
   load_mission_source("story/green/green5/g5_builder.c", 1, 2);
   load_mission_source("story/green/green5/g5_spikebase.c", 1, 3);
   load_mission_source("story/green/green5/g5_scout.c", 1, 4);
   clear_remaining_templates(1, 5);


   load_mission_source("story/green/green5/g5_base.c", 0, 0);
   load_mission_source("story/green/green5/g5_firebase.c", 0, 1);
   load_mission_source("story/green/green5/g5_builder.c", 0, 2);
   load_mission_source("story/green/green5/g5_spikebase.c", 0, 3);
   load_mission_source("story/green/green5/g5_scout.c", 0, 4);
   w_init.player_starting_data [0] = 1600;
   clear_remaining_templates(0, 5);


   int centre_block = w_init.map_size_blocks / 2;

   int ring_index_1 = add_mdetail_ring(centre_block, centre_block, 24, 0);
   int ring_index_2 = add_mdetail_ring(centre_block - 28, centre_block - 28, 8, 0);
   int ring_index_3 = add_mdetail_ring(centre_block + 28, centre_block + 28, 8, 0);

   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_2 + ANGLE_8, 2000, 1000, 4, 0.002);

   add_data_well_to_mdetail_ring(ring_index_2,	ANGLE_2 + ANGLE_8, 2000, 1000, 3, 0.002);
   set_player_spawn_position_by_latest_well(0, ANGLE_8);


   add_data_well_to_mdetail_ring(ring_index_1, ANGLE_8, 2000, 1000, 4, -0.002);
   set_player_spawn_position_by_latest_well(1, ANGLE_2 + ANGLE_8);

   add_data_well_to_mdetail_ring(ring_index_3, ANGLE_8, 2000, 1000, 3, -0.002);


   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 2, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 3, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 4, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 6, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 7, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	0, 2000, 1000, 3, 0.002);


   w_init.player_starting_data [1] = 1600;


   int ring_index_4 = add_mdetail_ring(centre_block, centre_block, 36, 0);

   add_data_well_to_mdetail_ring(ring_index_4,	ANGLE_8 * 2, 3000, 2000, 4, 0.002);
   add_data_well_to_mdetail_ring(ring_index_4,	ANGLE_8 * 3, 3000, 2000, 4, 0.002);
   add_data_well_to_mdetail_ring(ring_index_4,	ANGLE_8 * 4, 3000, 2000, 4, 0.002);
   add_data_well_to_mdetail_ring(ring_index_4,	ANGLE_8 * 6, 3000, 2000, 4, 0.002);
   add_data_well_to_mdetail_ring(ring_index_4,	ANGLE_8 * 7, 3000, 2000, 4, 0.002);
   add_data_well_to_mdetail_ring(ring_index_4,	0, 2000, 1000, 3, 0.002);


	 	}
	  break;


*/

/*
	 case MISSION_GREEN_3:
	 	{


   load_mission_source("story/green/green3/g3_base.c", 1, 0);
   load_mission_source("story/green/green3/g3_firebase.c", 1, 1);
   load_mission_source("story/green/green3/g3_builder.c", 1, 2);
   clear_remaining_templates(1, 3);


   int centre_block = w_init.map_size_blocks / 2;

   int ring_index_1 = add_mdetail_ring(centre_block, centre_block, 24, 0);
   int ring_index_2 = add_mdetail_ring(centre_block - 28, centre_block - 28, 8, 0);
   int ring_index_3 = add_mdetail_ring(centre_block + 28, centre_block + 28, 8, 0);

   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_2 + ANGLE_8, 2000, 1000, 4, 0.002);
   set_player_spawn_position_by_latest_well(0, ANGLE_8);

   add_data_well_to_mdetail_ring(ring_index_2,	ANGLE_2 + ANGLE_8, 2000, 1000, 3, 0.002);


   add_data_well_to_mdetail_ring(ring_index_1, ANGLE_8, 2000, 1000, 4, -0.002);
   set_player_spawn_position_by_latest_well(1, ANGLE_2 + ANGLE_8);

   add_data_well_to_mdetail_ring(ring_index_3, ANGLE_8, 2000, 1000, 3, -0.002);


   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 2, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 3, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 4, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 6, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 7, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	0, 2000, 1000, 3, 0.002);


   w_init.player_starting_data [1] = 600;


	 	}
	  break;


	 case MISSION_GREEN_4:
	 	{


   load_mission_source("story/green/green4/g4_base.c", 1, 0);
   load_mission_source("story/green/green4/g4_firebase.c", 1, 1);
   load_mission_source("story/green/green4/g4_builder.c", 1, 2);
   load_mission_source("story/green/green4/g4_spikebase.c", 1, 3);
   clear_remaining_templates(1, 4);


   int centre_block = w_init.map_size_blocks / 2;

   int ring_index_1 = add_mdetail_ring(centre_block, centre_block, 24, 0);
   int ring_index_2 = add_mdetail_ring(centre_block - 28, centre_block - 28, 8, 0);
   int ring_index_3 = add_mdetail_ring(centre_block + 28, centre_block + 28, 8, 0);

   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_2 + ANGLE_8, 2000, 1000, 4, 0.002);
   set_player_spawn_position_by_latest_well(0, ANGLE_8);

   add_data_well_to_mdetail_ring(ring_index_2,	ANGLE_2 + ANGLE_8, 2000, 1000, 3, 0.002);


   add_data_well_to_mdetail_ring(ring_index_1, ANGLE_8, 2000, 1000, 4, -0.002);
   set_player_spawn_position_by_latest_well(1, ANGLE_2 + ANGLE_8);

   add_data_well_to_mdetail_ring(ring_index_3, ANGLE_8, 2000, 1000, 3, -0.002);


   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 2, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 3, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 4, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 6, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	ANGLE_8 * 7, 2000, 1000, 3, 0.002);
   add_data_well_to_mdetail_ring(ring_index_1,	0, 2000, 1000, 3, 0.002);


   w_init.player_starting_data [1] = 600;


	 	}
	  break;
*/

/*



	 case MISSION_YELLOW_1:
	 	{


   int templates_used = 0;

   load_mission_source("story/yellow/yellow1/y1_base.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow1/y1_m_builder.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow1/y1_harvest.c", 1, templates_used++); // this mission might not actually use harvesters.
   load_mission_source("story/yellow/yellow1/y1_leader1.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow1/y1_leader2.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow1/y1_follower.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow1/y1_minbase.c", 1, templates_used++);
   clear_remaining_templates(1, templates_used);

	 	player_base_x = 11;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);
   set_player_spawn_position_by_latest_well(0, 0);

   int md_index = add_mdetail_ring(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2, 28, 0);
   add_data_well_to_mdetail_ring(md_index,	ANGLE_4, 2000, 1000, 3, 0.001);
   add_data_well_to_mdetail_ring(md_index,	ANGLE_16, 2000, 1000, 3, 0.001);
   add_data_well_to_mdetail_ring(md_index,	-ANGLE_16, 2000, 1000, 3, 0.001);
   add_data_well_to_mdetail_ring(md_index,	ANGLE_2 + ANGLE_16, 2000, 1000, 3, 0.001);
   add_data_well_to_mdetail_ring(md_index,	ANGLE_2 - ANGLE_16, 2000, 1000, 3, 0.001);

   add_data_well_to_mdetail_ring(md_index, -ANGLE_4, 2000, 1000, 3, 0.001);

   enemy_base_x = w_init.map_size_blocks - 15;
   enemy_base_y = w_init.map_size_blocks / 2;

	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	4, -0.002);

   set_player_spawn_position_by_latest_well(1, ANGLE_2);
   w_init.player_starting_data [1] = 600;

	 	}
	  break;



	 case MISSION_YELLOW_2:
	 	{


   int templates_used = 0;

   load_mission_source("story/yellow/yellow2/y2_base.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow2/y2_m_builder.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow2/y2_harvest.c", 1, templates_used++); // this mission might not actually use harvesters.
   load_mission_source("story/yellow/yellow2/y2_leader1.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow2/y2_leader2.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow2/y2_follower.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow2/y2_minbase.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow2/y2_scout.c", 1, templates_used++);
   clear_remaining_templates(1, templates_used);

	 	player_base_x = 11;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);
   set_player_spawn_position_by_latest_well(0, 0);

   int md_index = add_mdetail_ring(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2, 28, 0);
   add_data_well_to_mdetail_ring(md_index,	ANGLE_4, 2000, 1000, 3, 0.001);
   add_data_well_to_mdetail_ring(md_index,	ANGLE_16, 2000, 1000, 3, 0.001);
   add_data_well_to_mdetail_ring(md_index,	-ANGLE_16, 2000, 1000, 3, 0.001);
   add_data_well_to_mdetail_ring(md_index,	ANGLE_2 + ANGLE_16, 2000, 1000, 3, 0.001);
   add_data_well_to_mdetail_ring(md_index,	ANGLE_2 - ANGLE_16, 2000, 1000, 3, 0.001);

 //  add_extra_spawn_by_latest_well(1, 0, 0);

   add_data_well_to_mdetail_ring(md_index, -ANGLE_4, 2000, 1000, 3, 0.001);

   enemy_base_x = w_init.map_size_blocks - 15;
   enemy_base_y = w_init.map_size_blocks / 2;

	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	4, -0.002);

   set_player_spawn_position_by_latest_well(1, ANGLE_2);
   w_init.player_starting_data [1] = 600;

	 	}
	  break;


	 case MISSION_YELLOW_3:
	 	{


   int templates_used = 0;

   load_mission_source("story/yellow/yellow3/y3_base.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow3/y3_m_builder.c", 1, templates_used++);
//   load_mission_source("story/yellow/yellow2/y3_harvest.c", 1, templates_used++); // this mission might not actually use harvesters.
   load_mission_source("story/yellow/yellow3/y3_leader1.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow3/y3_follower.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow3/y3_minbase.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow3/y3_scout.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow3/y3_follower2.c", 1, templates_used++);
   clear_remaining_templates(1, templates_used);


	 	player_base_x = 11;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);
   set_player_spawn_position_by_latest_well(0, 0);

   int md_index = add_mdetail_ring(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2, 28, 0);
   add_data_well_to_mdetail_ring(md_index,	ANGLE_4, 2000, 1000, 3, 0.001);
   add_data_well_to_mdetail_ring(md_index,	ANGLE_16, 2000, 1000, 3, 0.001);
   add_data_well_to_mdetail_ring(md_index,	-ANGLE_16, 2000, 1000, 3, 0.001);
   add_data_well_to_mdetail_ring(md_index,	ANGLE_2 + ANGLE_16, 2000, 1000, 3, 0.001);
   add_data_well_to_mdetail_ring(md_index,	ANGLE_2 - ANGLE_16, 2000, 1000, 3, 0.001);

 //  add_extra_spawn_by_latest_well(1, 0, 0);

   add_data_well_to_mdetail_ring(md_index, -ANGLE_4, 2000, 1000, 3, 0.001);

   enemy_base_x = w_init.map_size_blocks - 15;
   enemy_base_y = w_init.map_size_blocks / 2;

	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	4, -0.002);

   set_player_spawn_position_by_latest_well(1, ANGLE_2);
   w_init.player_starting_data [1] = 1600;


	 	mission_add_data_well(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2,	2000, 2000, 4, 0.002);
	 	mission_add_data_well(w_init.map_size_blocks / 3, w_init.map_size_blocks / 2,	2000, 2000, 4, 0.002);
	 	mission_add_data_well((w_init.map_size_blocks * 2) / 3, w_init.map_size_blocks / 2,	2000, 2000, 4, 0.002);

	 	}
	  break;




	 case MISSION_PURPLE_1:
	 	{


   int templates_used = 0;


   load_mission_source("story/yellow/yellow3/y3_base.c", 0, templates_used++);
   load_mission_source("story/yellow/yellow3/y3_m_builder.c", 0, templates_used++);
//   load_mission_source("story/yellow/yellow2/y3_harvest.c", 1, templates_used++); // this mission might not actually use harvesters.
   load_mission_source("story/yellow/yellow3/y3_leader1.c", 0, templates_used++);
   load_mission_source("story/yellow/yellow3/y3_follower.c", 0, templates_used++);
   load_mission_source("story/yellow/yellow3/y3_minbase.c", 0, templates_used++);
   load_mission_source("story/yellow/yellow3/y3_scout.c", 0, templates_used++);
   load_mission_source("story/yellow/yellow3/y3_follower2.c", 0, templates_used++);
   clear_remaining_templates(0, templates_used);
   templates_used = 0;
   w_init.player_starting_data [0] = 1600;

   load_mission_source("story/purple/purple1/p1_base.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_m_builder.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_harvest.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_minbase.c", 1, templates_used++);

   load_mission_source("story/purple/purple1/p1_leader1.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_leader2.c", 1, templates_used++);

   load_mission_source("story/purple/purple1/p1_escort.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_picket.c", 1, templates_used++);


   clear_remaining_templates(1, templates_used);

*/

/*

How will purple work?

- 0 is well-defended main base.
- 1 is mobile builder
- 2 is secondary base
- 3 is static defence
- 4 is harvester
- 5 is flagship
- 6 is escort
- 7 is picket

in later purple missions:
- 8 is alternative flagship
- 9 is alternative escort


harvesters:
 - wander randomly
 - when they find a well, broadcast a well claim.
 - keep going back to that well. Periodically broadcast a well claim.
  - also, check for allocators
	- listen for well claims. do not claim claimed well.

mobile builders:
 - listen for well claims and move to the nearest one.

- main base:
 - builds a fleet of a certain size. Size probably increases as time goes on.
 - stores flagship and escorts in targetting memory. Replaces them when destroyed.
 - sends each escort its index. Escort uses this to find a position in the formation.
 - pickets probably just circle around flagship, searching for allocators or other targets.
 - builds flagship last.

- secondary bases
	- build one harvester each?
	- when available data gets high (indicating main base can't build fast enough),
	  build pickets. Pickets orbit the base until needed.
 - probably have dormant main base code in case main base destroyed

- flagship
 - when built, broadcasts its presence. All escorts and pickets follow it.
 - takes fleet to the centre of the map then the corners
 - when suitable target found, attacks it.

- escorts
 - use reposition() method to maintain formation around flagship.
 - if flagship destroyed, returns to main base.

- pickets
 - orbit builder base
  - may also protect harvesters and m_build?
 - when a flagship appears, move around it in a swarm (like orbiting, but distance from centre changes)
 - engage targets. If target is large enough, or has allocator, tell flagship (probably using low priority transmit)





*/



/*



	 	player_base_x = 11;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);
   add_mdetail_worm_source(player_base_x, player_base_y, 40);

   set_player_spawn_position_by_latest_well(0, 0);



   enemy_base_x = w_init.map_size_blocks - 15;
   enemy_base_y = w_init.map_size_blocks / 2;

	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	4, -0.002);
   add_mdetail_worm_source(enemy_base_x, enemy_base_y, 40);

   set_player_spawn_position_by_latest_well(1, ANGLE_2);
   w_init.player_starting_data [1] = 1600;


	 	mission_add_data_well(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2,	2000, 2000, 4, 0.002);
   add_mdetail_worm_source(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2, 20);


	 	}
	  break;





	 case MISSION_ORANGE_1:
	 	{

   int templates_used = 0;


   load_mission_source("story/purple/purple1/p1_base.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_m_builder.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_harvest.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_minbase.c", 1, templates_used++);

   load_mission_source("story/purple/purple1/p1_leader1.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_leader2.c", 1, templates_used++);

   load_mission_source("story/purple/purple1/p1_escort.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_picket.c", 1, templates_used++);


   clear_remaining_templates(1, templates_used);


	 	player_base_x = 11;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.04);

   set_player_spawn_position_by_latest_well(0, 0);



   enemy_base_x = w_init.map_size_blocks - 15;
   enemy_base_y = w_init.map_size_blocks / 2;

	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	3, -0.04);

   set_player_spawn_position_by_latest_well(1, ANGLE_2);
   w_init.player_starting_data [1] = 1600;


	 	mission_add_data_well(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2,	2000, 2000, 2, 0.04);


	 	}
	  break;


	 case MISSION_RED_1:
	 	{

   int templates_used = 0;
   int new_well_index;

   w_init.player_starting_data [0] = 1600;


   load_mission_source("story/purple/purple1/p1_base.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_m_builder.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_harvest.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_minbase.c", 1, templates_used++);

   load_mission_source("story/purple/purple1/p1_leader1.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_leader2.c", 1, templates_used++);

   load_mission_source("story/purple/purple1/p1_escort.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_picket.c", 1, templates_used++);


   clear_remaining_templates(1, templates_used);

	 	mission_add_data_well(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2,	2000, 2000, 5, -0.004);



	 	player_base_x = 11;
	 	player_base_y = 25;
	 	new_well_index = mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);

   set_player_spawn_position_by_latest_well(0, 0);
			add_line_between_data_wells(0, new_well_index, 40);



   enemy_base_x = w_init.map_size_blocks - 15;
   enemy_base_y = w_init.map_size_blocks - 30;

	 	new_well_index = mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	3, -0.002);
			add_line_between_data_wells(0, new_well_index, 40);

   set_player_spawn_position_by_latest_well(1, ANGLE_2);
   w_init.player_starting_data [1] = 1600;

	 	new_well_index = mission_add_data_well(40, 11, 2000, 1000,	2, -0.002);
			add_line_between_data_wells(0, new_well_index, 40);



//	 	mission_add_data_well(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2,	2000, 2000, 4, 0.002);


	 	}
	  break;
*/

/*

Unlockable things:
pulse_l
pulse_xl
burst_xl
stream
stream_dir
spike
slice
ultra
ultra_dir

repair_other
interface
stability

4 mobile cores
3 static cores
8 components

total: 27





Areas:
- tutorial
	- two first regions

- blue: deep learning
 - basic enemies. No local conditions
 - 6 regions
 - unlocks:
  - next 5-core
  - next component x 2
  - pulse_l
  - capital region: interface

2nd row:
- green: learning supervision
 - local condition: static environment (improved/cheaper static processes)
 - 5 regions
 - unlocks:
  - 6-static core x 2
  - next component
  - pulse_xl
  - capital region: spike

- yellow: input/output filter
 - local condition: fragile processes (every component has 60hp)
 - 5 regions
 - unlocks:
  - remaining 5 core, next 6 core
  - burst_xl, stream
  - capital region: repair_other


3rd row
- orange: recovery
 - local condition: thin interface (reduced interface strength)
 - unlocks:
  - 1 more core
  - surge
  - capital region: stream_dir

- purple:
 - local condition: memory control (only one allocator)
 - unlocks:
  - final static core
  - capital region: stability

- dark blue
 - local condition: commitment (processes cost minimum 100)
 - unlocks:
  - ultra
  - capital region: ultra_dir

- black?
 - local condition: autonomous mode
 - unlocks:
  - more optimisations


Final:
- red: firewall
 - local condition: interface charge rate doubled (or maybe no local condition)
 - unlocks:
  - capital region: freedom!

missions:

Blue:
Enemy/packet colour: yellow/blue
should be relatively easy.
Only capital region has procs with interface.
Enemies do not expand.
General enemy approach:
 - start with one or more static harvesters/allocators
 - sometimes: make harvesters
 - make single procs that wander randomly
  - and all attack allocator when found
  - also, broadcast enemy found signals nearby
	- sometimes make larger processes that follow same rules

- first 3 regions:
 - enemies don't coordinate. just wander randomly and attack when target found.
 - no harvesters

- next 2
 - enemies don't coordinate either.
 - harvesters

- final 2 + capital
 - more use of interface
 - harvesters
 - maybe hardened harvesters that are built if simple harvesters destroyed?
 - enemies will broadcast targets nearby.


Green
Enemy/packet colour: greenish/gold
Enemies expand + build defensive static perimeter
Strategy:
 - probably no harvesters
 - small numbers of large processes
  - some have build objects.
 - later regions: static processes have spikes, and coordinate attacks with scouts
  - also set up static siege bases

Yellow
Enemy/packet colour: Orange/red
Strategy:
 - economy starts using harvesters, then moves to builders after a while (or if harvesters destroyed?)
 - expand using interface-protected builders, bases and harvesters
 - rest of fleet is combined large, interface-protected cruisers + small escorts
 -




*/

}

}



void set_game_colours_for_area(int area_index, int players)
{

	int player_base_cols [PLAYERS] = {TEAM_COL_BLUE,1,2,3}; // index in base_proc_col array
	int player_packet_cols [PLAYERS] = {PACKET_COL_YELLOW_ORANGE,1,2,3}; // index in base_packet_colours array and similar interface array

	switch(area_index)
	{

default: // default should never happen
		case AREA_TUTORIAL:
		case AREA_BLUE:
 	    player_base_cols [1] = TEAM_COL_YELLOW;
	     player_packet_cols [1] = PACKET_COL_WHITE_BLUE;
      set_game_colours(BACK_COLS_BLUE, // index in back_and_hex_colours array
																				   BACK_COLS_BLUE, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
				break;

				 case AREA_GREEN:
 	    player_base_cols [1] = TEAM_COL_GREEN;
	     player_packet_cols [1] = PACKET_COL_WHITE_YELLOW;
      set_game_colours(BACK_COLS_GREEN, // index in back_and_hex_colours array
																				   BACK_COLS_GREEN, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array

						break;

					case AREA_YELLOW:
 	    player_base_cols [1] = TEAM_COL_ORANGE;
	     player_packet_cols [1] = PACKET_COL_BLUE_PURPLE;
      set_game_colours(BACK_COLS_YELLOW, // index in back_and_hex_colours array
																				   BACK_COLS_YELLOW, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
						break;

					case AREA_ORANGE:
 	    player_base_cols [1] = TEAM_COL_ORANGE;
	     player_packet_cols [1] = PACKET_COL_WHITE_PURPLE;
      set_game_colours(BACK_COLS_ORANGE, // index in back_and_hex_colours array
																				   BACK_COLS_ORANGE, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
						break;

					case AREA_PURPLE:
 	    player_base_cols [1] = TEAM_COL_RED;
	     player_packet_cols [1] = PACKET_COL_ULTRAVIOLET;
      set_game_colours(BACK_COLS_PURPLE, // index in back_and_hex_colours array
																				   BACK_COLS_PURPLE, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
      break;

					case AREA_RED:
 	    player_base_cols [1] = TEAM_COL_WHITE;
	     player_packet_cols [1] = PACKET_COL_ORANGE_RED;
      set_game_colours(BACK_COLS_RED, // index in back_and_hex_colours array
																				   BACK_COLS_RED, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
      break;


	}

}



static int mission_add_data_well(int data_well_type, float spin_sign, int x, int y)
{


	return add_data_well_to_map_init(x, y,
																																		mission_init.data_well_reserve_A [data_well_type],
																																		mission_init.data_well_reserve_B [data_well_type],
																																		mission_init.data_well_reserves [data_well_type],
																																		mission_init.data_well_spin [data_well_type] * spin_sign);


}


static int mission_add_data_well_to_ring(int data_well_type, float spin_sign, int md_index, int angle)
{

	   return add_data_well_to_mdetail_ring(md_index,
																																		       angle,
																																		       mission_init.data_well_reserve_A [data_well_type],
																																		       mission_init.data_well_reserve_B [data_well_type],
																																		       mission_init.data_well_reserves [data_well_type],
																																		       mission_init.data_well_spin [data_well_type] * spin_sign);

}



static int mission_add_data_well_to_circle(int data_well_type, int angle, int dist, float spin_sign, int centre_x, int centre_y)
{


 al_fixed fixed_angle = int_angle_to_fixed(angle);
 int ring_size_pixels = dist * BLOCK_SIZE_PIXELS;

	al_fixed well_x_fixed = block_to_fixed(centre_x) + fixed_xpart(fixed_angle, al_itofix(ring_size_pixels));
	al_fixed well_y_fixed = block_to_fixed(centre_y) + fixed_ypart(fixed_angle, al_itofix(ring_size_pixels));

	int well_x_block = fixed_to_block(well_x_fixed);
	int well_y_block = fixed_to_block(well_y_fixed);

	if (well_x_block < 7
		|| well_x_block >= w_init.map_size_blocks - 7
		|| well_y_block < 7
		|| well_y_block >= w_init.map_size_blocks - 7)
			return 0; // mission_add_red_data_well_ring() can produce wells outside of the map, which should be ignored.

 return add_data_well_to_map_init(well_x_block, well_y_block,
																											       mission_init.data_well_reserve_A [data_well_type],
																																		mission_init.data_well_reserve_B [data_well_type],
																																		mission_init.data_well_reserves [data_well_type],
																																		mission_init.data_well_spin [data_well_type] * spin_sign);

}


static void mission_add_red_data_well_ring(int data_well_type, int centre_x, int centre_y, int circle_size, int wells, int starting_angle)
{

	int i;

	float spin_sign;

//starting_angle += 150;

	for (i = 0; i < wells; i ++)
	{
		if (i & 1)
			spin_sign = 1;
		  else
  			spin_sign = -1;
  mission_add_data_well_to_circle(data_well_type,
																																		starting_angle + (ANGLE_1 / wells) * i,
																																		circle_size,
																																		spin_sign,
																																		centre_x,
																																		centre_y);
	}

}


static int add_orange_data_well(int centre_well, int data_well_type, int angle, int distance_from_centre, int line_thickness)
{

 block_cart origin_block = get_well_block_position(centre_well);

	al_fixed centre_well_x = block_to_fixed(origin_block.x);
	al_fixed centre_well_y = block_to_fixed(origin_block.y);

	al_fixed new_well_x = centre_well_x + fixed_xpart(int_angle_to_fixed(angle), al_itofix(distance_from_centre));
	al_fixed new_well_y = centre_well_y + fixed_ypart(int_angle_to_fixed(angle), al_itofix(distance_from_centre));

	int new_well_block_x = fixed_to_block(new_well_x);
	int new_well_block_y = fixed_to_block(new_well_y);

	int spin_sign = 1;

	if (centre_well == 0)
		spin_sign = -1;

	int new_well_index = mission_add_data_well(data_well_type,
																																spin_sign,
																																new_well_block_x,
																																new_well_block_y);

			add_line_between_data_wells(centre_well, new_well_index, line_thickness);


   return new_well_index;



}


// called from g_game.c at start of mission
void mission_spawn_extra_processes(void)
{

//fpr("\n mission_init.extra_p1_spawns %i", mission_init.extra_p1_spawns);
//return;

	int i = 0;

	while (i < mission_init.extra_spawns)
	{
//		fpr("\n spawn %i template %i (%i,%i) %i", i, current_mission.extra_spawn[i].template_index, current_mission.extra_spawn[i].spawn_x_block, current_mission.extra_spawn[i].spawn_y_block, current_mission.extra_spawn[i].spawn_angle);

		cart spawn_position;
		spawn_position.x = block_to_fixed(mission_init.extra_spawn[i].spawn_x_block);
		spawn_position.y = block_to_fixed(mission_init.extra_spawn[i].spawn_y_block);

		struct core_struct* unused_collided_core;

  int new_process_index = create_new_from_template(&templ[mission_init.extra_spawn[i].player_index][mission_init.extra_spawn[i].template_index], mission_init.extra_spawn[i].player_index, spawn_position, int_angle_to_fixed(mission_init.extra_spawn[i].spawn_angle), &unused_collided_core);
// create_new_from_template() should not fail, but check the result just in case:
  if (new_process_index >= 0) // relevant BUILD_FAIL codes are all negative
		{
// processes built at start of game don't wait to start executing.
   w.core[new_process_index].next_execution_timestamp = w.world_time + 15;
   w.core[new_process_index].construction_complete_timestamp = w.core[new_process_index].next_execution_timestamp;
// set parent to process 0 for player
//   * not sure about this
   w.core[new_process_index].process_memory [0] = w.player[mission_init.extra_spawn[i].player_index].core_index_start;
   w.core[new_process_index].process_memory_timestamp [0] = 0; // is this even correct?
		}
// 	fpr(" [%i]", retval);
// 	create_new_from_template(&templ[1][current_mission.extra_spawn[i].template_index], 1, spawn_position, al_itofix(current_mission.extra_spawn[i].spawn_angle));
  i ++;
	};

}

/*
// makes a mirror image of player 0's spawn location and all data wells.
// more data wells can be added afterwards
static void mission_mirror_spawns_and_wells(void)
{
/ *
	int i;

	enemy_base_x = w_init.map_size_blocks - w_init.spawn_position [0].x;
	enemy_base_y = w_init.map_size_blocks - w_init.spawn_position [0].y;

	int existing_data_wells = w_init.data_wells;

	for (i = 0; i < existing_data_wells; i ++)
	{
	 	mission_add_data_well(w_init.map_size_blocks - w_init.data_well_position [i].x, w_init.map_size_blocks - w_init.data_well_position [i].y,
																									w_init.reserve_data [i] [0], w_init.reserve_data [i] [1], // reserves
																									w_init.reserve_squares [i], // reserve squares
																									0 - w_init.data_well_spin_rate [i]); // spin rate

	 }
* /

}
*/
