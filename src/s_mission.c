/*
This file contains functions for setting up missions.

It loads up mission files and sets up menus and worlds based on them.

Missions will work like this:
 - at start, a game program will be loaded in.
 - this program will contain setup routines that will load other programs into hidden game templates (will need to revise template code a bit to deal with this)
  - the hidden templates may just be player zero's templates, which the game program will have access to (along with other templates).
 - it will then be able to make use of these templates as needed.
 - it will also use a special game method to specify what templates the player may use.
  - it will need a set of game methods, only available to game programs, that will allow it to do things like this.
 - after it has run once, it will modify itself so as not to run these setup routines again.
Then, the player will:
 - see a text description of the mission and a "start" button. The player will be able to use the template and editor windows. Templates may come pre-loaded but may be changeable.
 - when clicks "start", proc zero will be placed in world and game will start to run.

*/


#ifdef OLD_MISSIONS

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
#include "s_mission.h"
#include "t_files.h"
#include "t_template.h"

struct mission_state_struct mission_state; // just need one struct for the current mission


/*struct missionsstruct
{
	int status [MISSIONS]; // MISSION_STATUS_* enum
	int locked [MISSIONS]; // 0 or 1
};*/

extern struct world_init_struct w_init;
extern struct game_struct game;
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];

struct missionsstruct missions;


void clear_mission_status(void);
void unlock_basic_missions(void);
void process_mission_locks(void);

static void mission_add_data_well(int x, int y, int reserve_A, int reserve_B, int reserve_squares, float spin_rate);
static void mission_mirror_spawns_and_wells(void);

struct extra_spawnstruct
{
	int template_index;
	int spawn_x_block, spawn_y_block;
	int spawn_angle;
};
#define EXTRA_SPAWNS 16

// current_missionstruct holds details about a mission currently being played or initialised.
struct mission_init_struct
{
 int phase; // exactly what this

// initialisation information
	int extra_p1_spawns; // number of additional player 1 processes to spawn in the mission
	struct extra_spawnstruct extra_spawn [EXTRA_SPAWNS];

};

struct mission_init_struct mission_init;
void init_mission_init_struct(void);
void add_extra_spawn(int template_index, int spawn_x_block, int spawn_y_block, int spawn_angle);

void init_mission_init_struct(void)
{
	mission_init.extra_p1_spawns = 0; // don't bother initialising the extra_spawn struct
}

void add_extra_spawn(int template_index, int spawn_x_block, int spawn_y_block, int spawn_angle)
{

	if (mission_init.extra_p1_spawns >= EXTRA_SPAWNS)
	{
		fpr("\nError in s_mission.c: add_extra_spawn(): too many extra spawns.");
		error_call();
	}

	mission_init.extra_spawn[mission_init.extra_p1_spawns].template_index = template_index;
	mission_init.extra_spawn[mission_init.extra_p1_spawns].spawn_x_block = spawn_x_block;
	mission_init.extra_spawn[mission_init.extra_p1_spawns].spawn_y_block = spawn_y_block;
	mission_init.extra_spawn[mission_init.extra_p1_spawns].spawn_angle = spawn_angle;

	mission_init.extra_p1_spawns++;

}

// resets mission status - all missions are set to unfinished (avoid doing this if possible!)
void clear_mission_status(void)
{
	int i;

	for (i = 0; i < MISSIONS; i ++)
	{
		missions.status [i] = MISSION_STATUS_UNFINISHED;
		missions.locked [i] = 1;
	}

// unlock the tutorials and the first mission:
 unlock_basic_missions();

}


void unlock_basic_missions(void)
{

	missions.locked [MISSION_TUTORIAL1] = 0;
	missions.locked [MISSION_TUTORIAL2] = 0;
	missions.locked [MISSION_TUTORIAL3] = 0;
	missions.locked [MISSION_TUTORIAL4] = 0;

	missions.locked [MISSION_MISSION1] = 0;
	missions.locked [MISSION_MISSION2] = 0;
	missions.locked [MISSION_MISSION3] = 0;
	missions.locked [MISSION_MISSION4] = 0;
	missions.locked [MISSION_MISSION5] = 0;
	missions.locked [MISSION_MISSION6] = 0;
	missions.locked [MISSION_MISSION7] = 0;
	missions.locked [MISSION_MISSION8] = 0;

	missions.locked [MISSION_ADVANCED1] = 0;
	missions.locked [MISSION_ADVANCED2] = 0;
	missions.locked [MISSION_ADVANCED3] = 0;
/*
	missions.locked [MISSION_ADVANCED4] = 0;
	missions.locked [MISSION_ADVANCED5] = 0;
	missions.locked [MISSION_ADVANCED6] = 0;
	missions.locked [MISSION_ADVANCED7] = 0;
	missions.locked [MISSION_ADVANCED8] = 0;*/

}

// prepares w_init etc for a mission
// can cause a fatal error if it fails to load a file from disk (in which case the whole program stops, because this shouldn't happen)
void prepare_for_mission(void)
{

 init_w_init();

// How to deal with templates?
// prepare_templates_for_new_game in s_mission.c will have unlocked all templates,
//  but player 1's need to be cleared entirely.
// Player 0's templates should probably be initialised at program start-up and then left alone.
//  - need to think about this. It won't work if at some point I make some objects etc. available only in certain circumstances.
// *** one semi-solution may be to allow init.txt to specify default files to be loaded in at startup


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

/*
#define MAP_SIZE_0 60
#define MAP_SIZE_1 80
#define MAP_SIZE_2 100
#define MAP_SIZE_3 120
*/
 w_init.players = 2;
 w_init.command_mode = COMMAND_MODE_COMMAND; // can be set to AUTO below
 strcpy(w_init.player_name [0], "You");
 strcpy(w_init.player_name [1], "Opponent");

 init_mission_init_struct();

// clear_map_init(w_init.map_size_blocks,	w_init.players); // can't do this here because map size varies. It's done below for each mission.


 mission_state.phase = 0;
 mission_state.reveal_player1 = 0;

	int player_base_cols [PLAYERS] = {0,1,2,3}; // index in base_proc_col array
	int player_packet_cols [PLAYERS] = {0,1,2,3}; // index in base_packet_colours array and similar interface array

 int player_base_x, player_base_y;
	int enemy_base_x, enemy_base_y;



//	 case MISSION_ADVANCED3:
//   w_init.command_mode = COMMAND_MODE_AUTO;
// fall-through
//	 case MISSION_MISSION3:
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




 switch(game.mission_index)
 {
	 case MISSION_ADVANCED1:
   w_init.command_mode = COMMAND_MODE_AUTO;
// fall-through
	 case MISSION_MISSION1:
   set_game_colours(BACK_COLS_BLUE, // index in back_and_hex_colours array
																				BACK_COLS_BLUE, // index in back_and_hex_colours array
																				2, // players in game
																				player_base_cols, // index in base_proc_col array
																				player_packet_cols); // index in base_packet_colours array and similar interface array

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

   load_mission_source("missions/mission1/defend.c", 1, 0);
   load_mission_source("missions/mission1/circle.c", 1, 1);
   clear_remaining_templates(1, 2);

	 	enemy_base_x = w_init.map_size_blocks - 15;
	 	enemy_base_y = w_init.map_size_blocks / 2;
//			w_init.spawn_angle [1] = 4096;


//* pretty sure this won't work - need to set map_init some other way

	 	add_extra_spawn(1,
																			enemy_base_x - 4,
																			enemy_base_y,
																			-2048);
	 	add_extra_spawn(1,
																			enemy_base_x,
																			enemy_base_y - 4,
																			0);
	 	add_extra_spawn(1,
																			enemy_base_x + 4,
																			enemy_base_y,
																			2048);
	 	add_extra_spawn(1,
																			enemy_base_x,
																			enemy_base_y + 4,
																			4096);


	  break;



	 case MISSION_ADVANCED2:
   w_init.command_mode = COMMAND_MODE_AUTO;
// fall-through
	 case MISSION_MISSION2:
   set_game_colours(BACK_COLS_BLUE, // index in back_and_hex_colours array
																				BACK_COLS_BLUE, // index in back_and_hex_colours array
																				2, // players in game
																				player_base_cols, // index in base_proc_col array
																				player_packet_cols); // index in base_packet_colours array and similar interface array

   w_init.size_setting = 1; // was 0
   fix_w_init_size();
   clear_map_init(w_init.map_size_blocks,	2); // resets map initialisation code in g_world_map.x. 2 means 2 players

	 	player_base_x = 15;
	 	player_base_y = w_init.map_size_blocks / 2;
	 	mission_add_data_well(player_base_x - 4, player_base_y,
																									2000, 1000, // reserves
																									4, // reserve squares
																									0.002); // spin rate
   set_player_spawn_position_by_latest_well(0, 0);
	 	mission_add_data_well(player_base_x + 10, player_base_y - 20,
																									2000, 700, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(player_base_x + 10, player_base_y + 20,
																									1500, 1100, // reserves
																									3, // reserve squares
																									0.001); // spin rate

   load_mission_source("missions/mission2/defend.c", 1, 0);
   load_mission_source("missions/mission2/circle.c", 1, 1);
   clear_remaining_templates(1, 2);

	 	enemy_base_x = w_init.map_size_blocks - 15;
	 	enemy_base_y = w_init.map_size_blocks / 2;
//			w_init.spawn_angle [1] = 4096;

	 	add_extra_spawn(1,
																			enemy_base_x - 4,
																			enemy_base_y,
																			-2048);
	 	add_extra_spawn(1,
																			enemy_base_x,
																			enemy_base_y - 4,
																			0);
	 	add_extra_spawn(1,
																			enemy_base_x + 4,
																			enemy_base_y,
																			2048);
	 	add_extra_spawn(1,
																			enemy_base_x,
																			enemy_base_y + 4,
																			4096);

   int extra_spawn_x = w_init.map_size_blocks - 25;
   int extra_spawn_y = w_init.map_size_blocks / 4;

	 	add_extra_spawn(0,
																			extra_spawn_x,
																			extra_spawn_y,
																			4096);
	 	add_extra_spawn(1,
																			extra_spawn_x - 4,
																			extra_spawn_y,
																			-2048);
	 	add_extra_spawn(1,
																			extra_spawn_x,
																			extra_spawn_y - 4,
																			0);
	 	add_extra_spawn(1,
																			extra_spawn_x + 4,
																			extra_spawn_y,
																			2048);
	 	add_extra_spawn(1,
																			extra_spawn_x,
																			extra_spawn_y + 4,
																			4096);


   extra_spawn_x = w_init.map_size_blocks - 25;
   extra_spawn_y = w_init.map_size_blocks - (w_init.map_size_blocks / 4);

	 	add_extra_spawn(0,
																			extra_spawn_x,
																			extra_spawn_y,
																			4096);
	 	add_extra_spawn(1,
																			extra_spawn_x - 4,
																			extra_spawn_y,
																			-2048);
	 	add_extra_spawn(1,
																			extra_spawn_x,
																			extra_spawn_y - 4,
																			0);
	 	add_extra_spawn(1,
																			extra_spawn_x + 4,
																			extra_spawn_y,
																			2048);
	 	add_extra_spawn(1,
																			extra_spawn_x,
																			extra_spawn_y + 4,
																			4096);
			break;


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
/*
	 	mission_add_data_well(player_base_x + 13, player_base_y - 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate
	 	mission_add_data_well(player_base_x + 13, player_base_y + 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									0.001); // spin rate
*/
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
/*
//   set_player_w_init_spawn_angle(1, w_init.data_wells - 1); // - 1 because mission_add_data_well incremenets w_init.data_wells
	 	mission_add_data_well(enemy_base_x - 13, enemy_base_y - 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate
	 	mission_add_data_well(enemy_base_x - 13, enemy_base_y + 15,
																									2000, 1000, // reserves
																									3, // reserve squares
																									-0.001); // spin rate
*/


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

}
/*

Message codes for mission AI (these are the first value in the message contents)


+ indicates that there should be a target attached

0 - nothing (this is the default used for errors etc.)

1+ [ch 4] - target found (x, y)
2+ [ch 4] - allocator found (x, y) (like target found but known to be static and probably a high priority)

10 [ch 1] - message to bases and others that might be interested (broadcast): I am the main base

20 [ch 2] - message to builders (broadcast): please don't build as I am waiting for enough data to build something big (broadcast each cycle; when message stops, okay to build again)

30 [ch 3] - message to bases and harvesters (broadcast): I found a new data well (x, y)
31 - message to main base (transmitted): request transmission of all known data well locations
33 - [ch 3] message to harvesters: data well at specified location is claimed (well_status, x, y)
    - may be sent by harvester or by builder. Can be sent repeatedly
    - well status is 0 for mobile harvester claim, 1 for static harvester claim
     - in practice, 0 is likely to be ignored by builders looking for somewhere to build a base while they will listen to 1

34 - transmitted response to 31: list of x,y coordinates of all known data well locations. 34 has wells 1-3. Terminated by 0
35 - second response to 31 (wells 4-7)
36 - third response to 31 (wells 8-11)
37 - fourth response to 31 (wells 12-15)
38 - fifth response to 31 (well 16)

40 [ch 5] - message from leader to nearby processes - if you are free please follow me
41 - message transmitted from parent to new process - please guard me
42+ - message transmitted from parent to new process - please guard target, which isn't me.


Channels used for mission AI:

0 - not used (used by transmissions, so best to avoid confusion)
1 - listened to by bases
2 - listened to by builders in general
3 - listened to by harvesters and builders
4 - targetting information; listened to by any attack processes
5 - coordination messages, eg messages from leader to followers. followers in general may listen



1 - harvester tells other harvesters that its data well is taken.
 message value 0:
  0: being harvested by mobile harvester
   message values 1, 2:
	   x, y of well
  1: being harvested by fixed harvester
   message values 1, 2:
	   x, y of well

2 - broadcasts to static builders
 message 0:
  0: please don't build anything for now
  1: there is a main base (main base broadcasts this to prevent other bases deciding that they are the main base)


3 - enemy target found/destroyed
 message 0:
  0: target confirmed destroyed (probably only relevant for allocators)
  1: target found (this is probably broadcast to nearby processes)
   message 1, 2: x, y of target
   - broadcast_target should be used
  2: high priority target found
   message 1, 2: x, y of target
   - broadcast_target should be used





Channels used for mission AI:

0 - leader broadcasts to nearby followers
 *** need to reassign this as channel 0 is also used by transmissions

1 - harvester tells other harvesters that its data well is taken.
 message value 0:
  0: being harvested by mobile harvester
   message values 1, 2:
	   x, y of well
  1: being harvested by fixed harvester
   message values 1, 2:
	   x, y of well

2 - broadcasts to static builders
 message 0:
  0: please don't build anything for now
  1: there is a main base (main base broadcasts this to prevent other bases deciding that they are the main base)


3 - enemy target found/destroyed
 message 0:
  0: target confirmed destroyed (probably only relevant for allocators)
  1: target found (this is probably broadcast to nearby processes)
   message 1, 2: x, y of target
   - broadcast_target should be used
  2: high priority target found
   message 1, 2: x, y of target
   - broadcast_target should be used

Let's plan missions!

Mission 1
- effectively a tutorial
- just a few static processes around the place
- small map.
- tutorial messages
- processes:
 - weak static processes
 - weak mobile processes, maybe circling the static ones

Mission 2
- base with basic defences should just spawn weak processes that wander randomly.
- processes:
 - base with simple defences
 - wandering attack process

Mission 3
- processes:
 - base with simple defences
 - leader
 - follower
 - harvester

Mission 4
- Basic wandering attackers serve as scouts
 - when they find an enemy allocator, they broadcast its location to all other attackers
 - other attackers then all try to converge on target
 - if an attacker reaches target and doesn't see an allocator there, it broadcasts a cancel order
- processes:
 - base with defences
 - wandering scout/attacker
 - slightly bigger attacker
 - harvester
 - mobile builder

Mission 5
- Enemy bases assemble armies around them.
- When armies reach a certain size(?)
   (or just after a certain number built?)
   they attack. Probably use scouts for this as well.
  - maybe attack could be triggered by building a capital ship process that the others all follow.
- processes:
 - base with defences
 - follower (circles base, then circles leader; returns to base if no leader)
 - maybe other kinds of followers?
 - harvester? Maybe don't need harvester in all cases
 - mobile builder
 - scout




Mobile builders:
 - building more than one of these at a time is probably going to be wasteful.
 - Possibly: m-builder should broadcast an "m-builder present" signal to all static builders.
	- they will build a new one if a certain number of cycles passes without the broadcast.
	 - when building one, they will broadcast the signal themselves.




*/


 }

//}


static void mission_add_data_well(int x, int y, int reserve_A, int reserve_B, int reserve_squares, float spin_rate)
{


	add_data_well_to_map_init(x, y, reserve_A, reserve_B, reserve_squares, spin_rate);

	/*
#ifdef SANITY_CHECK
 if (w_init.data_wells >= DATA_WELLS)
	{
		fpr("\nError: s_mission.c: mission_add_data_well(): too many data wells");
		error_call();
	}
#endif

 map_init.data_well_position [w_init.data_wells].x = x;
 map_init.data_well_position [w_init.data_wells].y = y;
 map_init.data_well_reserve_data [w_init.data_wells] [0] = reserve_A;
 map_init.data_well_reserve_data [w_init.data_wells] [1] = reserve_B;
 map_init.data_well_reserve_squares [w_init.data_wells] = reserve_squares;
 map_init.data_well_spin_rate [w_init.data_wells] = spin_rate;

 map_init.data_wells ++;
*/
}


void mission_spawn_extra_p1_processes(void)
{

//fpr("\n mission_init.extra_p1_spawns %i", mission_init.extra_p1_spawns);
//return;

	int i = 0;

	while (i < mission_init.extra_p1_spawns)
	{
//		fpr("\n spawn %i template %i (%i,%i) %i", i, current_mission.extra_spawn[i].template_index, current_mission.extra_spawn[i].spawn_x_block, current_mission.extra_spawn[i].spawn_y_block, current_mission.extra_spawn[i].spawn_angle);

		cart spawn_position;
		spawn_position.x = block_to_fixed(mission_init.extra_spawn[i].spawn_x_block);
		spawn_position.y = block_to_fixed(mission_init.extra_spawn[i].spawn_y_block);

		struct core_struct* unused_collided_core;

  int new_process_index = create_new_from_template(&templ[1][mission_init.extra_spawn[i].template_index], 1, spawn_position, int_angle_to_fixed(mission_init.extra_spawn[i].spawn_angle), &unused_collided_core);
// create_new_from_template() should not fail, but check the result just in case:
  if (new_process_index >= 0) // relevant BUILD_FAIL codes are all negative
		{
// processes built at start of game don't wait to start executing.
   w.core[new_process_index].next_execution_timestamp = w.world_time + 15;
   w.core[new_process_index].construction_complete_timestamp = w.core[new_process_index].next_execution_timestamp;
// set parent to process 0 for player 1
   w.core[new_process_index].process_memory [0] = w.player[1].core_index_start;
   w.core[new_process_index].process_memory_timestamp [0] = 0;
		}
// 	fpr(" [%i]", retval);
// 	create_new_from_template(&templ[1][current_mission.extra_spawn[i].template_index], 1, spawn_position, al_itofix(current_mission.extra_spawn[i].spawn_angle));
  i ++;
	};

}


// makes a mirror image of player 0's spawn location and all data wells.
// more data wells can be added afterwards
static void mission_mirror_spawns_and_wells(void)
{
/*
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
*/

}

void load_mission_status_file(void)
{


 clear_mission_status();

#define MISSIONFILE_SIZE 128

 FILE *missionfile;
 char buffer [MISSIONFILE_SIZE];

 missionfile = fopen("msn.dat", "rb");

 if (!missionfile)
 {
  fprintf(stdout, "\n mission status (default)");
  return;
 }

 int read_in = fread(buffer, 1, MISSIONFILE_SIZE, missionfile);

 if (ferror(missionfile)
  || read_in == 0)
 {
  fprintf(stdout, "\nFailed to read mission status from msn.dat. Starting with default mission status.");
  fclose(missionfile);
  return;
 }

 int i;

 for (i = 0; i < MISSIONS; i ++)
	{
		missions.status [i] = buffer [i];
		if (missions.status [i] < 0
		 || missions.status [i] >= MISSION_STATUSES)
		{
			fprintf(stdout, "\nFailure: mission %i status (%i) invalid (should be %i to %i).\nStating with default missions status.", i, missions.status [i], 0, MISSION_STATUSES);
   fclose(missionfile);
   clear_mission_status();
   return;
		}
	}

// Now process the loaded statuses to produce the correct locked/unlocked settings:
 unlock_basic_missions(); // unlocks tutorials and mission 1

 process_mission_locks(); // unlocks any mission if the previous mission has been completed.

 fclose(missionfile);

//missions.status [MISSION_MISSION6] = MISSION_STATUS_FINISHED;
//missions.locked [MISSION_MISSION7] = 0;
//save_mission_status_file();

  fprintf(stdout, "\n mission status (loaded)");

 return;
}


void process_mission_locks(void)
{

 int i;

 for (i = MISSION_MISSION1; i < MISSIONS - 1; i ++)
	{
  if (missions.status [i] != MISSION_STATUS_UNFINISHED)
	 	missions.locked [i + 1] = 0;
	}

}

void save_mission_status_file(void)
{

 process_mission_locks(); // this makes sure that any change in missions statuses is immediately reflected in unlocks

 char buffer [MISSIONFILE_SIZE];

 FILE *file;

// open the file:
 file = fopen("msn.dat", "wb");

 if (!file)
 {
  fprintf(stdout, "\nFailed to save mission status to msn.dat: couldn't open file.");
  return;
 }

 int i;

 for (i = 0; i < MISSIONS; i ++)
	{
		buffer [i] = missions.status [i];
	}

 int written = fwrite(buffer, 1, MISSIONS, file);

 if (written != MISSIONS)
 {
  fprintf(stdout, "\nFailed to save mission status to msn.dat: couldn't write data (tried to write %i; wrote %i).", MISSIONS, written);
  fclose(file);
  return;
 }

 fclose(file);


}

#endif
