
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


static void mission_add_data_well(int x, int y, int reserve_A, int reserve_B, int reserve_squares, float spin_rate);
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
 w_init.core_setting = 2;
 w_init.size_setting = 2;
 fix_w_init_size();

 w_init.players = 2;
 w_init.command_mode = COMMAND_MODE_COMMAND; // can be set to AUTO below
 strcpy(w_init.player_name [0], "You");
 strcpy(w_init.player_name [1], "Opponent");

 init_mission_init_struct();

// clear_map_init(w_init.map_size_blocks,	w_init.players); // can't do this here because map size varies. It's done below for each mission.


 mission_state.phase = 0;
 mission_state.reveal_player1 = 0;

	int player_base_cols [PLAYERS] = {4,6,2,3}; // index in base_proc_col array
	int player_packet_cols [PLAYERS] = {0,1,2,3}; // index in base_packet_colours array and similar interface array

 int player_base_x, player_base_y;
	int enemy_base_x, enemy_base_y;


   w_init.size_setting = 1;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

   w_init.story_area = game.area_index;


   switch(game.area_index)
   {
 			default:
				 case AREA_BLUE:
				 case AREA_TUTORIAL:
//					 w_init.local_condition = LOCAL_CONDITION_NONE;
      set_game_colours(BACK_COLS_BLUE, // index in back_and_hex_colours array
																				   BACK_COLS_BLUE, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
						break;

				 case AREA_GREEN:
				 	w_init.core_setting = 3;
//					 w_init.local_condition = LOCAL_CONDITION_STATIC;
 	    player_base_cols [1] = 2;
	     player_packet_cols [1] = 2;
      set_game_colours(BACK_COLS_GREEN, // index in back_and_hex_colours array
																				   BACK_COLS_GREEN, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
						break;


				 case AREA_YELLOW:
				 	w_init.core_setting = 3;
//					 w_init.local_condition = LOCAL_CONDITION_FRAGILE_PROCS;
 	    player_base_cols [1] = 2;
	     player_packet_cols [1] = 3;
      set_game_colours(BACK_COLS_YELLOW, // index in back_and_hex_colours array
																				   BACK_COLS_YELLOW, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
						break;


				 case AREA_ORANGE:
				 	w_init.core_setting = 3;
//					 w_init.local_condition = LOCAL_CONDITION_FRAGILE_PROCS;
 	    player_base_cols [1] = 2;
	     player_packet_cols [1] = 3;
      set_game_colours(BACK_COLS_ORANGE, // index in back_and_hex_colours array
																				   BACK_COLS_ORANGE, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
						break;



				 case AREA_PURPLE:
				 	w_init.core_setting = 3;
//					 w_init.local_condition = LOCAL_CONDITION_FRAGILE_PROCS;
 	    player_base_cols [1] = 2;
	     player_packet_cols [1] = 3;
      set_game_colours(BACK_COLS_PURPLE, // index in back_and_hex_colours array
																				   BACK_COLS_PURPLE, // index in back_and_hex_colours array
																				   2, // players in game
																				   player_base_cols, // index in base_proc_col array
																				   player_packet_cols); // index in base_packet_colours array and similar interface array
						break;


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

				 case AREA_RED:
				 	w_init.core_setting = 3;
 	    player_base_cols [1] = 2;
	     player_packet_cols [1] = 3;
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

   w_init.size_setting = 0; // was 0
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

	 	player_base_x = 15;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x - 4, player_base_y,
																									2000, 1000, // reserves
																									4, // reserve squares
																									0.002); // spin rate
   set_player_spawn_position_by_latest_well(0, 0);
//   set_player_w_init_spawn_angle(0, w_init.data_wells - 1); // - 1 because mission_add_data_well incremenets w_init.data_wells
	 	mission_add_data_well(player_base_x + 13, player_base_y - 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(player_base_x + 13, player_base_y + 15,
																									1000, 0, // reserves
																									3, // reserve squares
																									0.001); // spin rate

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

   w_init.size_setting = 1; // was 0
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

	 	player_base_x = 15;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x - 4, player_base_y,
																									2000, 1000, // reserves
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


   w_init.size_setting = 1;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

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

   w_init.size_setting = 1;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

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

   w_init.size_setting = 1;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

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


   w_init.size_setting = 2;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

   load_mission_source("story/green/green1/g1_base.c", 1, 0);
   load_mission_source("story/green/green1/g1_firebase.c", 1, 1);
   load_mission_source("story/green/green1/g1_builder.c", 1, 2);
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


add_mdetail_system(centre_block, centre_block, 3);
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


   w_init.size_setting = 2;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

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


   w_init.size_setting = 2;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

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


   w_init.size_setting = 3;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

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

   w_init.size_setting = 3;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

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


   w_init.size_setting = 3;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

   int templates_used = 0;

   load_mission_source("story/yellow/yellow1/y1_base.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow1/y1_m_builder.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow1/y1_harvest.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow1/y1_leader1.c", 1, templates_used++);
   load_mission_source("story/yellow/yellow1/y1_follower.c", 1, templates_used++);
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

	 	}
	  break;



/*

MISSION_GREEN_1
- unlocks next static core
- procs:
 - interface-protected base
 - medium-sized cruisers wander randomly, building:
  - new bases near data wells
  - defensive bases near those bases

green unlocks:
  - 6-static core x 2
  - next component
  - pulse_xl
  - capital region: spike



MISSION_YELLOW_1
- unlocks next 5-core
- procs:
 - interface-protected base
 - heavy harvesters
 - heavy builders
 - small, swarmy attackers

MISSION_YELLOW_2
- unlocks burst_xl
- procs:
 - interface-protected base
 - heavy harvesters
 - heavy builders
 - small, swarmy attackers
 -



yellow unlocks:

  - remaining 5 core, next 6 core
  - burst_xl, stream
  - capital region: repair_other


MISSION_ORANGE_1
- unlocks a core or something
 -
 - small procs with harass-like attacks; maybe surge


- orange: recovery
 - local condition: thin interface (reduced interface strength)
 - unlocks:
  - 1 more core
  - surge
  - capital region: stream_dir



Box will have:
 - heading (in sub-box) with name
 - defeated/undefeated
 - other information?
 - local conditions
 - reward for defeating

On side (bottom?) of screen:
 - list of active optimisations

Let's work out how story map will work in general:
 - sprawl of regions, with special 'capital' regions in middle/at edge
 - make a path through the regions to unlock stuff

Start with:
 - First 3 mobile cores
 - First 2 static cores
 - 3 and some of 4 link components?
Attack:
 - pulse, burst, burst_l
Defence:
 - repair, repair_other
- Data
 - build, allocate, harvest, storage
- Need to unlock:
 - interface, pulse_x/xl, burst_xl, stream/dir, spike



Optimisations:
 - strong core (double integrity for core only)
 - strong processes (+30% integrity)
 - strong interface (+30% interface strength)
 - ?fast interface (increase interface charge rate)
 - pulse (+ power for pulse)
 - burst power (+ power for burst (higher bonus than pulse power))
 - stream power
 - spike range (+ spike flight time)
 - surge range? power?
 - assemble (reduces build cooldown/construction time)
 - reassemble (increases repair speed/reduces restore cooldown)
 - compression (increases capacity of storage objects - 2x?)
 - fast harvest (increases harvest rate)
 - inline static (reduces total cost of static processes)

Unlockable things:
pulse_l
pulse_xl
burst_xl
stream
stream_dir
spike
surge
ultra
ultra_dir

repair_other
interface
stability

4 mobile cores
3 static cores
8 components

total:

Local conditions:
none
thin interface (32? per object only)
fragile processes (60 integrity regardless of size)
memory control (only first process has allocator)
static environment (increases integrity and power of static cores)
?commitment (increases cost of all processes by 64 or maybe sets a minimum cost of 100)



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

Green (static environment (improved/cheaper static processes))
Enemy/packet colour: greenish/gold
Enemies expand + build defensive static perimeter
Strategy:
 - probably no harvesters
 - small numbers of large processes
  - some have build objects.
 - later regions: static processes have spikes, and coordinate attacks with scouts
  - also set up static siege bases

Yellow (fragile processes (every component has 60hp))
Enemy/packet colour: Orange/red
Strategy:
 - expand using interface-protected builders, bases and harvesters
 - rest of fleet is combined large, interface-protected cruisers + small escorts





*/

/*
	 case MISSION_ADVANCED3:
   w_init.command_mode = COMMAND_MODE_AUTO;
// fall-through
	 case MISSION_MISSION3:
	 	{
   set_game_colours(BACK_COLS_BLUE, // index in back_and_hex_colours array
																				BACK_COLS_BLUE, // index in back_and_hex_colours array
																				2, // players in game
																				player_base_cols, // index in base_proc_col array
																				player_packet_cols); // index in base_packet_colours array and similar interface array

   w_init.size_setting = 1;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

//	 	w_init.spawn_position [0].x = 15;
//	 	w_init.spawn_position [0].y = w_init.map_size_blocks / 2;
	 	player_base_x = 11;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x, player_base_y,
																									2000, 1000, // reserves
																									4, // reserve squares
																									0.002); // spin rate
   set_player_spawn_position_by_latest_well(0, 0);

   int md_index = add_mdetail_ring(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2, 15, 0);
   add_data_well_to_mdetail_ring(md_index,
																																	ANGLE_4,
																									        2000, 1000, // reserves
																									        3, // reserve squares
																									        0.001); // spin rate
   add_data_well_to_mdetail_ring(md_index,
																																	-ANGLE_4,
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



//				generate_map_from_map_init(); - no, this is called later
	 	}
	  break;


	 case MISSION_ADVANCED4:
   w_init.command_mode = COMMAND_MODE_AUTO;
// fall-through
	 case MISSION_MISSION4:
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players
	 	player_base_cols [1] = 1;
	 	player_packet_cols [1] = 1;
   set_game_colours(BACK_COLS_GREEN, // index in back_and_hex_colours array
																				BACK_COLS_GREEN, // index in back_and_hex_colours array
																				2, // players in game
																				player_base_cols, // index in base_proc_col array
																				player_packet_cols); // index in base_packet_colours array and similar interface array

	 	player_base_x = 15;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x - 4, player_base_y,
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

   load_mission_source("missions/mission4/base.c", 1, 0);
   load_mission_source("missions/general/m_builder.c", 1, 1);
   load_mission_source("missions/general/harvest.c", 1, 2);
   load_mission_source("missions/mission4/leader1.c", 1, 3);
   load_mission_source("missions/mission4/follower.c", 1, 4);
   clear_remaining_templates(1, 5);

	 	enemy_base_x = w_init.map_size_blocks - 15;
	 	enemy_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(enemy_base_x + 4, enemy_base_y,
																									2000, 1000, // reserves
																									4, // reserve squares
																									-0.002); // spin rate
   set_player_spawn_position_by_latest_well(1, ANGLE_2);
	 	mission_add_data_well(enemy_base_x - 13, enemy_base_y - 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate
	 	mission_add_data_well(enemy_base_x - 13, enemy_base_y + 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate

	 	mission_add_data_well(w_init.map_size_blocks / 2, w_init.map_size_blocks / 6,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.002); // spin rate
	 	mission_add_data_well(w_init.map_size_blocks / 2, w_init.map_size_blocks - (w_init.map_size_blocks / 6),
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.002); // spin rate
	 	mission_add_data_well(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2,
																									2000, 1500, // reserves
																									3, // reserve squares
																									-0.002); // spin rate


	 	break;


	 case MISSION_ADVANCED5:
   w_init.command_mode = COMMAND_MODE_AUTO;
// fall-through
	 case MISSION_MISSION5:
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players
	 	player_base_cols [1] = 1;
	 	player_packet_cols [1] = 1;
   set_game_colours(BACK_COLS_GREEN, // index in back_and_hex_colours array
																				BACK_COLS_GREEN, // index in back_and_hex_colours array
																				2, // players in game
																				player_base_cols, // index in base_proc_col array
																				player_packet_cols); // index in base_packet_colours array and similar interface array
	 	player_base_x = 15;
	 	player_base_y = 15;
	 	mission_add_data_well(player_base_x - 3, player_base_y - 3,
																									2000, 1000, // reserves
																									4, // reserve squares
																									0.002); // spin rate
   set_player_spawn_position_by_latest_well(0, 0);
	 	mission_add_data_well(player_base_x + 19, player_base_y - 1,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(player_base_x - 1, player_base_y + 19,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate

	 	mission_add_data_well(player_base_x + 46, player_base_y + 5,
																									2000, 900, // reserves
																									2, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(player_base_x + 5, player_base_y + 46,
																									2000, 900, // reserves
																									2, // reserve squares
																									0.001); // spin rate


		mission_mirror_spawns_and_wells();

	 	mission_add_data_well(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2,
																									2500, 1500, // reserves
																									4, // reserve squares
																									-0.003); // spin rate
// }


   load_mission_source("missions/mission5/base.c", 1, 0);
   load_mission_source("missions/general/m_builder.c", 1, 1);
   load_mission_source("missions/general/harvest.c", 1, 2);
   load_mission_source("missions/mission5/large.c", 1, 3);
   load_mission_source("missions/mission5/scout.c", 1, 4);
   clear_remaining_templates(1, 5);

	 	break;


	 case MISSION_ADVANCED6:
   w_init.command_mode = COMMAND_MODE_AUTO;
// fall-through
	 case MISSION_MISSION6:
   w_init.core_setting = 3;
   w_init.size_setting = 3;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

	 	player_base_cols [1] = 2;
	 	player_packet_cols [1] = 2;
   set_game_colours(BACK_COLS_YELLOW, // index in back_and_hex_colours array
																				BACK_COLS_YELLOW, // index in back_and_hex_colours array
																				2, // players in game
																				player_base_cols, // index in base_proc_col array
																				player_packet_cols); // index in base_packet_colours array and similar interface array
	 	player_base_x = 38;
	 	player_base_y = 38;
	 	mission_add_data_well(player_base_x - 3, player_base_y - 3,
																									2000, 1000, // reserves
																									4, // reserve squares
																									0.002); // spin rate
   set_player_spawn_position_by_latest_well(0, 0);
	 	mission_add_data_well(13, 13,
																									1500, 700, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(48, 7,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(7, 48,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate


   load_mission_source("missions/mission6/base.c", 1, 0);
   load_mission_source("missions/general/m_builder.c", 1, 1);
   load_mission_source("missions/general/harvest2.c", 1, 2);
   load_mission_source("missions/mission6/battle.c", 1, 3);
   load_mission_source("missions/mission6/scout.c", 1, 4);
   clear_remaining_templates(1, 5);

	 	enemy_base_x = w_init.map_size_blocks - 12;
	 	enemy_base_y = 45;
	 	mission_add_data_well(enemy_base_x + 3, enemy_base_y - 2,
																									2000, 1000, // reserves
																									4, // reserve squares
																									-0.002); // spin rate
   set_player_spawn_position_by_latest_well(1, ANGLE_2);

   w_init.player_starting_data [1] = 500;

   add_extra_spawn(0, 45, w_init.map_size_blocks - 12, 5000);
	 	mission_add_data_well(mission_init.extra_spawn[mission_init.extra_p1_spawns-1].spawn_x_block - 2,
																									mission_init.extra_spawn[mission_init.extra_p1_spawns-1].spawn_y_block + 3,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate

	 	mission_add_data_well(w_init.map_size_blocks / 2,
																									w_init.map_size_blocks / 3,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(w_init.map_size_blocks / 3,
																									w_init.map_size_blocks / 2,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate

	 	mission_add_data_well(w_init.map_size_blocks - 20,
																									20,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.002); // spin rate
	 	mission_add_data_well(20,
																									w_init.map_size_blocks - 20,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.002); // spin rate
	 	mission_add_data_well(w_init.map_size_blocks - 30,
																									w_init.map_size_blocks - 30,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.002); // spin rate

	 	mission_add_data_well(w_init.map_size_blocks - 55,
																									w_init.map_size_blocks - 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.002); // spin rate
	 	mission_add_data_well(w_init.map_size_blocks - 15,
																									w_init.map_size_blocks - 55,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.002); // spin rate
	 	mission_add_data_well(60,
																									w_init.map_size_blocks - 30,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.002); // spin rate
	 	mission_add_data_well(w_init.map_size_blocks - 30,
																									60,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.002); // spin rate


	 	break;


	 case MISSION_ADVANCED7:
   w_init.command_mode = COMMAND_MODE_AUTO;
// fall-through
	 case MISSION_MISSION7:
   w_init.core_setting = 3;
   w_init.size_setting = 3;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

	 	player_base_cols [1] = 3;
	 	player_packet_cols [1] = 3;
   set_game_colours(BACK_COLS_RED, // index in back_and_hex_colours array
																				BACK_COLS_RED, // index in back_and_hex_colours array
																				2, // players in game
																				player_base_cols, // index in base_proc_col array
																				player_packet_cols); // index in base_packet_colours array and similar interface array
	 	player_base_x = 15;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x - 4, player_base_y,
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


   w_init.player_starting_data [1] = 800; // needed to build base_m6A

   load_mission_source("missions/mission7/base_m7.c", 1, 0);
   load_mission_source("missions/general/m_builder2.c", 1, 1);
   load_mission_source("missions/general/harvest3.c", 1, 2);
   load_mission_source("missions/mission7/spikey2.c", 1, 3);
   load_mission_source("missions/mission7/fighter.c", 1, 4);
   load_mission_source("missions/mission7/fighter2.c", 1, 5);
   load_mission_source("missions/mission7/scout.c", 1, 6);
   clear_remaining_templates(1, 7);

	 	enemy_base_x = w_init.map_size_blocks - 15;
	 	enemy_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(enemy_base_x + 4, enemy_base_y,
																									2000, 1000, // reserves
																									4, // reserve squares
																									-0.002); // spin rate
   set_player_spawn_position_by_latest_well(1, ANGLE_2);
	 	mission_add_data_well(enemy_base_x - 12, enemy_base_y - 12,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate
	 	mission_add_data_well(enemy_base_x - 12, enemy_base_y + 12,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate
	 	mission_add_data_well(enemy_base_x - 26, enemy_base_y,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate
	 	mission_add_data_well(enemy_base_x - 5, enemy_base_y - 25,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate
	 	mission_add_data_well(enemy_base_x - 5, enemy_base_y + 25,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate


	 	mission_add_data_well(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2 - 8,
																									2000, 1500, // reserves
																									4, // reserve squares
																									0.003); // spin rate
	 	mission_add_data_well(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2 + 8,
																									2000, 1500, // reserves
																									4, // reserve squares
																									-0.003); // spin rate
	 	mission_add_data_well(w_init.map_size_blocks / 2 - 8, 12,
																									1200, 800, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(w_init.map_size_blocks / 2 - 8, w_init.map_size_blocks - 12,
																									1200, 800, // reserves
																									3, // reserve squares
																									-0.001); // spin rate


	 	break;


	 case MISSION_ADVANCED8:
   w_init.command_mode = COMMAND_MODE_AUTO;
// fall-through
	 case MISSION_MISSION8:
   w_init.core_setting = 3;
   w_init.size_setting = 3;
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

	 	player_base_cols [1] = 3;
	 	player_packet_cols [1] = 3;
   set_game_colours(BACK_COLS_RED, // index in back_and_hex_colours array
																				BACK_COLS_RED, // index in back_and_hex_colours array
																				2, // players in game
																				player_base_cols, // index in base_proc_col array
																				player_packet_cols); // index in base_packet_colours array and similar interface array
	 	player_base_x = 37;
	 	player_base_y = 37;
	 	mission_add_data_well(player_base_x - 3, player_base_y - 3,
																									2000, 1000, // reserves
																									4, // reserve squares
																									0.002); // spin rate
   set_player_spawn_position_by_latest_well(0, 0);
	 	mission_add_data_well(35, 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(15, 35,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(55, 35,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(35, 55,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate

	 	mission_add_data_well(w_init.map_size_blocks - 30, 30,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(30, w_init.map_size_blocks - 30,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate



//   w_init.player_starting_data [0] = 500; // needed to build base_m6A
   w_init.player_starting_data [1] = 1200;

   load_mission_source("missions/mission8/base_m8.c", 1, 0);
   load_mission_source("missions/general/m_builder2.c", 1, 1);
   load_mission_source("missions/general/harvest3.c", 1, 2);
   load_mission_source("missions/mission7/spikey2.c", 1, 3); // some of these are from the mission6 directory
   load_mission_source("missions/mission8/tank.c", 1, 4);
   load_mission_source("missions/mission7/fighter.c", 1, 5);
   load_mission_source("missions/mission7/fighter2.c", 1, 6);
   load_mission_source("missions/mission7/scout.c", 1, 7);
//   clear_remaining_templates(1, 7);

	 	enemy_base_x = w_init.map_size_blocks - 19;
	 	enemy_base_y = w_init.map_size_blocks - 58;
//   w_init.spawn_angle [1] = 5120;

	 	mission_add_data_well(w_init.map_size_blocks - 12, w_init.map_size_blocks - 12,
																									2000, 1000, // reserves
																									4, // reserve squares
																									-0.002); // spin rate
   set_player_spawn_position_by_latest_well(1, ANGLE_2);


	 	mission_add_data_well(w_init.map_size_blocks - 35, w_init.map_size_blocks - 15,
																									2000, 1000, // reserves
																									4, // reserve squares
																									-0.001); // spin rate

   add_extra_spawn(0, w_init.map_size_blocks - 58, w_init.map_size_blocks - 19, 5120);

	 	mission_add_data_well(w_init.map_size_blocks - 15, w_init.map_size_blocks - 35,
																									2000, 1000, // reserves
																									4, // reserve squares
																									-0.001); // spin rate

	 	mission_add_data_well(w_init.map_size_blocks - 54, w_init.map_size_blocks - 19,
																									2000, 1000, // reserves
																									4, // reserve squares
																									-0.001); // spin rate
	 	mission_add_data_well(w_init.map_size_blocks - 19, w_init.map_size_blocks - 54,
																									2000, 1000, // reserves
																									4, // reserve squares
																									-0.001); // spin rate


	 	mission_add_data_well(w_init.map_size_blocks - 55, w_init.map_size_blocks - 35,
																									2000, 1000, // reserves
																									4, // reserve squares
																									-0.001); // spin rate
	 	mission_add_data_well(w_init.map_size_blocks - 35, w_init.map_size_blocks - 55,
																									2000, 1000, // reserves
																									4, // reserve squares
																									-0.001); // spin rate


	 	mission_add_data_well(w_init.map_size_blocks / 2, w_init.map_size_blocks / 2,
																									2500, 2000, // reserves
																									4, // reserve squares
																									-0.004); // spin rate




	 	break;



	 default:
	 	player_base_x = 15;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x - 4, player_base_y,
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

   load_mission_source("proc/MissionAI/base.c", 1, 0);
   load_mission_source("proc/general/m_builder.c", 1, 1);
   load_mission_source("proc/general/harvest.c", 1, 2);
   load_mission_source("proc/MissionAI/leader1.c", 1, 3);
   load_mission_source("proc/MissionAI/follower.c", 1, 4);

	 	enemy_base_x = w_init.map_size_blocks - 15;
	 	enemy_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(enemy_base_x + 4, enemy_base_y,
																									2000, 1000, // reserves
																									4, // reserve squares
																									-0.002); // spin rate
   set_player_spawn_position_by_latest_well(1, ANGLE_2);
	 	mission_add_data_well(enemy_base_x - 13, enemy_base_y - 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate
	 	mission_add_data_well(enemy_base_x - 13, enemy_base_y + 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate
	 	break;
*/
}

}




static void mission_add_data_well(int x, int y, int reserve_A, int reserve_B, int reserve_squares, float spin_rate)
{


	add_data_well_to_map_init(x, y, reserve_A, reserve_B, reserve_squares, spin_rate);


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
