
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


static int mission_add_data_well(int x, int y, int reserve_A, int reserve_B, int reserve_squares, float spin_rate);
//static void mission_mirror_spawns_and_wells(void);

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

	int player_base_cols [PLAYERS] = {TEAM_COL_BLUE,1,2,3}; // index in base_proc_col array
	int player_packet_cols [PLAYERS] = {PACKET_COL_YELLOW_ORANGE,1,2,3}; // index in base_packet_colours array and similar interface array

 int player_base_x, player_base_y;
	int enemy_base_x, enemy_base_y;

	int data_well_index [DATA_WELLS];

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

 	    player_base_cols [1] = TEAM_COL_YELLOW;
	     player_packet_cols [1] = PACKET_COL_WHITE_BLUE;
      set_game_colours(BACK_COLS_BLUE, // index in back_and_hex_colours array
																				   BACK_COLS_BLUE, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
						break;

				 case AREA_BLUE:
      w_init.core_setting = 2;
      w_init.size_setting = 2;
      fix_w_init_size();
      reset_map_init(w_init.map_size_blocks,
																		   AREA_BLUE,
																		   2); // resets map initialisation code in g_world_map.x. 2 means 2 players

 	    player_base_cols [1] = TEAM_COL_YELLOW;
	     player_packet_cols [1] = PACKET_COL_WHITE_BLUE;
      set_game_colours(BACK_COLS_BLUE, // index in back_and_hex_colours array
																				   BACK_COLS_BLUE, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
						break;

				 case AREA_GREEN:
      w_init.core_setting = 3;
      w_init.size_setting = 3;
      fix_w_init_size();
      reset_map_init(w_init.map_size_blocks,
																		   AREA_GREEN,
																		   2); // resets map initialisation code in g_world_map.x. 2 means 2 players
 	    player_base_cols [1] = TEAM_COL_GREEN;
	     player_packet_cols [1] = PACKET_COL_WHITE_YELLOW;
      set_game_colours(BACK_COLS_GREEN, // index in back_and_hex_colours array
																				   BACK_COLS_GREEN, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
						break;

				 case AREA_YELLOW:
      w_init.core_setting = 3;
      w_init.size_setting = 3;
      fix_w_init_size();
      reset_map_init(w_init.map_size_blocks,
																		   AREA_YELLOW,
																		   2); // resets map initialisation code in g_world_map.x. 2 means 2 players
 	    player_base_cols [1] = TEAM_COL_ORANGE;
	     player_packet_cols [1] = PACKET_COL_BLUE_PURPLE;
      set_game_colours(BACK_COLS_YELLOW, // index in back_and_hex_colours array
																				   BACK_COLS_YELLOW, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
						break;


				 case AREA_ORANGE:
      w_init.core_setting = 3;
      w_init.size_setting = 3;
      fix_w_init_size();
      reset_map_init(w_init.map_size_blocks,
																		   AREA_ORANGE,
																		   2); // resets map initialisation code in g_world_map.x. 2 means 2 players
 	    player_base_cols [1] = TEAM_COL_RED;
	     player_packet_cols [1] = PACKET_COL_WHITE_PURPLE;
      set_game_colours(BACK_COLS_ORANGE, // index in back_and_hex_colours array
																				   BACK_COLS_ORANGE, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
						break;



				 case AREA_PURPLE:
      w_init.core_setting = 3;
      w_init.size_setting = 3;
      fix_w_init_size();
      reset_map_init(w_init.map_size_blocks,
																		   AREA_PURPLE,
																		   2); // resets map initialisation code in g_world_map.x. 2 means 2 players
 	    player_base_cols [1] = TEAM_COL_PURPLE;
	     player_packet_cols [1] = PACKET_COL_ORANGE_RED;
      set_game_colours(BACK_COLS_PURPLE, // index in back_and_hex_colours array
																				   BACK_COLS_PURPLE, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
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
 	    player_base_cols [1] = TEAM_COL_WHITE;
	     player_packet_cols [1] = PACKET_COL_ULTRAVIOLET;
      set_game_colours(BACK_COLS_RED, // index in back_and_hex_colours array
																				   BACK_COLS_RED, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
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



 switch(game.mission_index)
 {
//	 case MISSION_ADVANCED1:
//   w_init.command_mode = COMMAND_MODE_AUTO;
// fall-through
	 case MISSION_TUTORIAL1:

	 	player_base_x = 15;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x - 4, player_base_y,
																									2000, 1000, // reserves
																									4, // reserve squares
																									0.002); // spin rate
   add_mdetail_worm_source(player_base_x - 4, player_base_y, 20);


//   add_mdetail_worm_source(player_base_x + 20, player_base_y, 20);



   set_player_spawn_position_by_latest_well(0, 0);
//   set_player_w_init_spawn_angle(0, w_init.data_wells - 1); // - 1 because mission_add_data_well incremenets w_init.data_wells
	 	mission_add_data_well(player_base_x + 13, player_base_y - 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate
   add_mdetail_worm_source(player_base_x +13, player_base_y - 15, 20);
	 	mission_add_data_well(player_base_x + 13, player_base_y + 15,
																									1000, 0, // reserves
																									3, // reserve squares
																									0.001); // spin rate
   add_mdetail_worm_source(player_base_x +13, player_base_y + 15, 20);


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
	 	add_extra_spawn(1, 1,
																			enemy_base_x + 4,
																			enemy_base_y,
																			2048);
	 	add_extra_spawn(1, 1,
																			enemy_base_x,
																			enemy_base_y + 4,
																			4096);


	  break;



//	 case MISSION_ADVANCED2:
//   w_init.command_mode = COMMAND_MODE_AUTO;
// fall-through

default:

	 case MISSION_TUTORIAL2:

	 	player_base_x = 15;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x - 4, player_base_y,
																									2000, 2000, // reserves
																									4, // reserve squares
																									0.003); // spin rate
   set_player_spawn_position_by_latest_well(0, 0);
	 	mission_add_data_well(player_base_x + 10, player_base_y - 20,
																									2000, 700, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(player_base_x + 10, player_base_y + 20,
																									1500, 1100, // reserves
																									3, // reserve squares
																									0.001); // spin rate

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
			break;

	 case MISSION_BLUE_1:
	 	{


   load_mission_source("story/blue/blue1/rbase.c", 1, 0);
   load_mission_source("story/blue/blue1/wander1.c", 1, 1);
   load_mission_source("story/blue/blue1/wander2.c", 1, 2);
   clear_remaining_templates(1, 3);

	 	player_base_x = 11;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);
   set_player_spawn_position_by_latest_well(0, 0);

   int md_index = add_mdetail_ring(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2, 15, 0);
   add_data_well_to_mdetail_ring(md_index,	ANGLE_4, 2000, 1000, 3, 0.001);
 //  add_extra_spawn_by_latest_well(1, 0, 0);

   add_data_well_to_mdetail_ring(md_index, -ANGLE_4, 2000, 1000, 3, 0.001);

   enemy_base_x = w_init.map_size_blocks - 15;
   enemy_base_y = w_init.map_size_blocks / 2;

	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 0,	3, -0.002);

   set_player_spawn_position_by_latest_well(1, ANGLE_2);

	 	}
	  break;


	 case MISSION_BLUE_2:
	 	{

   load_mission_source("story/blue/blue2/b2_rbase.c", 1, 0);
   load_mission_source("story/blue/blue2/b2_wander1.c", 1, 1);
   load_mission_source("story/blue/blue2/b2_wander2.c", 1, 2);
   clear_remaining_templates(1, 3);

//	 	player_base_x = 11;
//	 	player_base_y = w_init.map_size_blocks / 2;
//	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);

   int md_index = add_mdetail_ring(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2, 23, 0);

   add_data_well_to_mdetail_ring(md_index,	ANGLE_6 * 4, 2000, 1000, 3, 0.002);
   set_player_spawn_position_by_latest_well(0, ANGLE_6);

   add_data_well_to_mdetail_ring(md_index,	0, 2000, 0, 3, 0.001);
   add_data_well_to_mdetail_ring(md_index, ANGLE_6, 2000, 0, 3, -0.002);
   set_player_spawn_position_by_latest_well(1, ANGLE_6 * 4);
   add_data_well_to_mdetail_ring(md_index, ANGLE_6 * 2, 2000, 0, 3, -0.001);
//   add_extra_spawn_by_latest_well(1, 0, ANGLE_6 * -1);
   add_data_well_to_mdetail_ring(md_index, ANGLE_6 * 3, 2000, 0, 3, -0.001);
   add_data_well_to_mdetail_ring(md_index, ANGLE_6 * 5, 2000, 0, 3, 0.001);

//   enemy_base_x = w_init.map_size_blocks - 15;
//   enemy_base_y = w_init.map_size_blocks / 2;

//	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	3, -0.002);

	 	}
	  break;


	 case MISSION_BLUE_3:
	 	{

   load_mission_source("story/blue/blue3/b3_rbase.c", 1, 0);
   load_mission_source("story/blue/blue3/b3_wander1.c", 1, 1);
   load_mission_source("story/blue/blue3/b3_wander2.c", 1, 2);
   clear_remaining_templates(1, 3);

//	 	player_base_x = 11;
//	 	player_base_y = w_init.map_size_blocks / 2;
//	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);

   int md_index = add_mdetail_ring(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2, 23, 0);

   add_data_well_to_mdetail_ring(md_index,	ANGLE_6 * 4, 2000, 1000, 3, 0.002);
   set_player_spawn_position_by_latest_well(0, ANGLE_6);

   add_data_well_to_mdetail_ring(md_index,	0, 2000, 0, 3, 0.001);
//   set_player_spawn_position_by_latest_well(1, ANGLE_2);
   add_data_well_to_mdetail_ring(md_index, ANGLE_6, 2000, 0, 3, -0.002);
   set_player_spawn_position_by_latest_well(1, ANGLE_6 * 4);
   add_data_well_to_mdetail_ring(md_index, ANGLE_6 * 2, 2000, 0, 3, -0.001);
//   add_extra_spawn_by_latest_well(1, 0, ANGLE_6 * -1);
   add_data_well_to_mdetail_ring(md_index, ANGLE_6 * 3, 2000, 1000, 3, -0.001);
   add_data_well_to_mdetail_ring(md_index, ANGLE_6 * 5, 2000, 1000, 3, 0.001);

//   enemy_base_x = w_init.map_size_blocks - 15;
//   enemy_base_y = w_init.map_size_blocks / 2;

//	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	3, -0.002);

	 	}
	  break;











/*

Plan for blue missions:

MISSION_BLUE_1
 - just a single base
 - makes weak attackers and weak large attackers
 - very easy

MISSION_BLUE_2
- unlocks pulse_l
- above, left of MISSION_BLUE_1
 - 2 bases
 - like MISSION_BLUE_1, but with different proc designs
 - also, newly built processes orbit parent for a while

MISSION_BLUE_3
- unlocks 5-core
- above, right of MISSION_BLUE_1
 - 2 bases
 - like MISSION_BLUE_1, but with different proc designs
 - also, newly built processes orbit parent for a while

MISSION_BLUE_4
- unlocks component
- left of capital
- 2 or 3 bases, with harvesters
- processes circle parent for a while after being built.
- small attackers follow large attackers
- bases (only) have interface

MISSION_BLUE_5
- unlocks component
- right of capital
- like MISSION_BLUE_4 but with different proc designs
- bases (only) have interface


MISSION_BLUE_CAPITAL
- unlocks interface
- ?special central base
- like MISSION_BLUE_4 but with more extensive use of interface

*/



	 case MISSION_GREEN_1:
	 	{


   load_mission_source("story/green/green1/g1_base.c", 1, 0);
   load_mission_source("story/green/green1/g1_firebase.c", 1, 1);
   load_mission_source("story/green/green1/g1_builder.c", 1, 2);
   clear_remaining_templates(1, 3);

	 	player_base_x = w_init.map_size_blocks - 11;
	 	player_base_y = w_init.map_size_blocks - 20;
//	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);

   int centre_block = w_init.map_size_blocks / 2;

   data_well_index [0] = mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);
   set_player_spawn_position_by_latest_well(0, 5000);

   data_well_index [1] = mission_add_data_well(player_base_x - 30,
																																															player_base_y - 4,
																																															2000, 1000, 4, 0.002);

			add_line_between_data_wells(data_well_index [0], data_well_index [1], 40);


   data_well_index [2] = mission_add_data_well(30, 20,	2000, 1000, 4, 0.002);
			add_line_between_data_wells(data_well_index [1], data_well_index [2], 100);
			add_line_between_data_wells(data_well_index [0], data_well_index [2], 70);

   set_player_spawn_position_by_latest_well(1, ANGLE_8);


//add_mdetail_system(centre_block, centre_block, 3);

/*
   add_data_well_to_mdetail_ring(ring_index_1,	0, 2000, 1000, 3, 0.001);
//   set_player_spawn_position_by_latest_well(1, ANGLE_2);
   add_data_well_to_mdetail_ring(ring_index_1, ANGLE_6, 2000, 1000, 3, -0.002);
   set_player_spawn_position_by_latest_well(1, ANGLE_6 * 4);
   add_data_well_to_mdetail_ring(ring_index_1, ANGLE_6 * 2, 2000, 1000, 3, -0.001);
//   add_extra_spawn_by_latest_well(1, 0, ANGLE_6 * -1);
   add_data_well_to_mdetail_ring(ring_index_1, ANGLE_6 * 3, 2000, 1000, 3, -0.001);
   add_data_well_to_mdetail_ring(ring_index_1, ANGLE_6 * 5, 2000, 1000, 3, 0.001);
*/

   w_init.player_starting_data [1] = 600;

//   enemy_base_x = w_init.map_size_blocks - 15;
//   enemy_base_y = w_init.map_size_blocks / 2;

//	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	3, -0.002);

	 	}
	  break;




	 case MISSION_GREEN_2:
	 	{


   load_mission_source("story/green/green2/g2_base.c", 1, 0);
   load_mission_source("story/green/green2/g2_firebase.c", 1, 1);
   load_mission_source("story/green/green2/g2_builder.c", 1, 2);
   clear_remaining_templates(1, 3);

//	 	player_base_x = 11;
//	 	player_base_y = w_init.map_size_blocks / 2;
//	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);

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

//   enemy_base_x = w_init.map_size_blocks - 15;
//   enemy_base_y = w_init.map_size_blocks / 2;

//	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	3, -0.002);

	 	}
	  break;


	 case MISSION_GREEN_3:
	 	{


   load_mission_source("story/green/green3/g3_base.c", 1, 0);
   load_mission_source("story/green/green3/g3_firebase.c", 1, 1);
   load_mission_source("story/green/green3/g3_builder.c", 1, 2);
   clear_remaining_templates(1, 3);

//	 	player_base_x = 11;
//	 	player_base_y = w_init.map_size_blocks / 2;
//	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);

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

//   enemy_base_x = w_init.map_size_blocks - 15;
//   enemy_base_y = w_init.map_size_blocks / 2;

//	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	3, -0.002);

	 	}
	  break;


	 case MISSION_GREEN_4:
	 	{


   load_mission_source("story/green/green4/g4_base.c", 1, 0);
   load_mission_source("story/green/green4/g4_firebase.c", 1, 1);
   load_mission_source("story/green/green4/g4_builder.c", 1, 2);
   load_mission_source("story/green/green4/g4_spikebase.c", 1, 3);
   clear_remaining_templates(1, 4);

//	 	player_base_x = 11;
//	 	player_base_y = w_init.map_size_blocks / 2;
//	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);

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

//   enemy_base_x = w_init.map_size_blocks - 15;
//   enemy_base_y = w_init.map_size_blocks / 2;

//	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	3, -0.002);

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

//	 	player_base_x = 11;
//	 	player_base_y = w_init.map_size_blocks / 2;
//	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);

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


//   enemy_base_x = w_init.map_size_blocks - 15;
//   enemy_base_y = w_init.map_size_blocks / 2;

//	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	3, -0.002);

	 	}
	  break;





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

 //  add_extra_spawn_by_latest_well(1, 0, 0);

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

/*

   load_mission_source("story/green/green5/g5_base.c", 0, 0);
   load_mission_source("story/green/green5/g5_firebase.c", 0, 1);
   load_mission_source("story/green/green5/g5_builder.c", 0, 2);
   load_mission_source("story/green/green5/g5_spikebase.c", 0, 3);
   load_mission_source("story/green/green5/g5_scout.c", 0, 4);
   w_init.player_starting_data [0] = 1600;
   clear_remaining_templates(0, 5);

*/







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

/*
   load_mission_source("story/purple/purple1/p1_base.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_m_builder.c", 1, templates_used++);
//   load_mission_source("story/yellow/yellow2/y3_harvest.c", 1, templates_used++); // this mission might not actually use harvesters.
   load_mission_source("story/purple/purple1/p1_leader1.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_follower.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_minbase.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_scout.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_follower2.c", 1, templates_used++);
*/
   load_mission_source("story/purple/purple1/p1_base.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_m_builder.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_harvest.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_minbase.c", 1, templates_used++);

   load_mission_source("story/purple/purple1/p1_leader1.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_leader2.c", 1, templates_used++);

   load_mission_source("story/purple/purple1/p1_escort.c", 1, templates_used++);
   load_mission_source("story/purple/purple1/p1_picket.c", 1, templates_used++);


   clear_remaining_templates(1, templates_used);

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


	 	player_base_x = 11;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x, player_base_y,	2000, 1000, 4, 0.002);

   set_player_spawn_position_by_latest_well(0, 0);



   enemy_base_x = w_init.map_size_blocks - 15;
   enemy_base_y = w_init.map_size_blocks / 2;

	 	mission_add_data_well(enemy_base_x, enemy_base_y, 2000, 1000,	4, -0.002);

   set_player_spawn_position_by_latest_well(1, ANGLE_2);
   w_init.player_starting_data [1] = 1600;


	 	mission_add_data_well(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2,	2000, 2000, 4, 0.002);


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




static int mission_add_data_well(int x, int y, int reserve_A, int reserve_B, int reserve_squares, float spin_rate)
{


	return add_data_well_to_map_init(x, y, reserve_A, reserve_B, reserve_squares, spin_rate);


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
