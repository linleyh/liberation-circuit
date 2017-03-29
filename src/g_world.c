#include <allegro5/allegro.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "m_config.h"
#include "m_globvars.h"

#define G_WORLD
#include "g_header.h"
#undef G_WORLD

#include "e_editor.h"
#include "g_motion.h"
#include "g_proc.h"
#include "g_packet.h"
#include "g_cloud.h"
#include "g_world.h"
#include "g_proc_new.h"
#include "g_game.h"
#include "g_method.h"
#include "i_view.h"
#include "i_input.h"
#include "i_console.h"
#include "e_log.h"

#include "g_misc.h"

#include "c_header.h"
#include "t_template.h"

#include "m_maths.h"
#include "i_disp_in.h"
#include "i_background.h"
#include "s_menu.h"

#include "c_prepr.h"

void place_proc_randomly(struct proc_struct* pr);
void init_world_background(void);
void init_hex_block_nodes(int x, int y);
void clear_background_square(int x1, int y1, int x2, int y2);
void clear_background_circle(int centre_block_x, int centre_block_y, int clear_size, int edge_thickness);
void add_data_well(int block_x, int block_y, int w_init_index);

extern struct world_init_struct w_init;
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct game_struct game;
extern struct view_struct view;
extern struct editorstruct editor;

int load_source_file_into_template(char* filename, int player_index, int template_index);
//int load_source_file_into_template(char* filename, int player_index, int player_template);

//static void init_vision_block(int x, int y);
//static void prepare_world_for_game(void);

// This function starts the game, deriving world parameters from a world_init struct.
void start_world(void)
{




 reset_log();

	write_line_to_log("Welcome!", MLOG_COL_HELP);
	write_line_to_log("For keyword help, right-click on a keyword in the editor.", MLOG_COL_HELP);
	write_line_to_log("For more detailed instructions, read manual.html.", MLOG_COL_HELP);
// This text also in enter_story_mode()

 open_template(0, 0);

// prepare_world_for_game();

 start_game();

 inter.mode_button_highlight_time = 0;

 initialise_view(settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);

 view.camera_x = w.player[game.user_player_index].spawn_position.x;
 view.camera_y = w.player[game.user_player_index].spawn_position.y;

 initialise_control();

 init_consoles();
 setup_consoles();

// game.phase = GAME_PHASE_PREGAME;
// game.fast_forward = FAST_FORWARD_OFF;
// game.fast_forward_type = FAST_FORWARD_TYPE_SMOOTH;


}



// This function takes a world_init_struct and sets up a worldstruct based on the world_init_struct.
// Currently, any memory allocation failure causes a fatal error. Could change this (but it would require deallocation of a partially allocated world, which isn't implemented yet)
void new_world_from_world_init(void)
{


// char player_name [PLAYERS] [PLAYER_NAME_LENGTH];



 int i, j;

 w.players = w_init.players;
 w.command_mode = w_init.command_mode;
// w.local_condition = w_init.local_condition;
 w.story_area = w_init.story_area;

 switch(w_init.core_setting)
 {
	 case 0:
	 	w.cores_per_player = 16;
	 	w.procs_per_player = 64;
	 	break;
	 case 1:
	 	w.cores_per_player = 32;
	 	w.procs_per_player = 128;
	 	break;
	 case 2:
	 	w.cores_per_player = 64;
	 	w.procs_per_player = 256;
	 	break;
	 case 3:
	 	w.cores_per_player = MAX_CORES_PER_PLAYER;
	 	w.procs_per_player = MAX_PROCS_PER_PLAYER;
	 	break;
	 default: // should never happen
			fpr("\nError: g_world.c: new_world_from_world_init(): w_init.core_setting = %i", w_init.core_setting);
			error_call();
			break;
 }


 w.max_procs = w.procs_per_player * w.players;
 w.max_cores = w.cores_per_player * w.players;
// w.gen_limit = w_init.gen_limit;
 w.max_packets = w.players * 200; // not sure about this
 w.max_clouds = CLOUDS; // see new cloud code in g_cloud.c if max_clouds is ever set to anything other than CLOUDS

 w.blocks.x = w_init.map_size_blocks;
 w.blocks.y = w_init.map_size_blocks;

 w.fixed_size.x = block_to_fixed(w.blocks.x);
 w.fixed_size.y = block_to_fixed(w.blocks.y);

 w.w_pixels = al_fixtoi(w.fixed_size.x);
 w.h_pixels = al_fixtoi(w.fixed_size.y);

// when adding any dynamic memory allocation to this function, remember to free the memory in deallocate_world() below
#ifdef USE_DYNAMIC_MEMORY

 int p;

 w.block = calloc(w.blocks.x, sizeof(struct block_struct*));
 if (w.block == NULL)
 {
      fprintf(stdout, "g_world.c: Out of memory in allocating w.block");
      error_call();
 }
 for (i = 0; i < w.blocks.x; i ++)
 {
   w.block [i] = calloc(w.blocks.y, sizeof(struct block_struct));
   if (w.block [i] == NULL)
   {
      fprintf(stdout, "g_world.c: Out of memory in allocating w.block");
      error_call();
   }
 }

// allocation code for backblocks not done yet

#endif


// block_type is used for collision detection:

 for (i = 0; i < w.blocks.x; i ++)
 {
  for (j = 0; j < w.blocks.y; j ++)
  {
   w.block [i][j].block_type = BLOCK_NORMAL;
  }
 }

 for (i = 0; i < w.blocks.x; i ++)
 {
  w.block [i] [0].block_type = BLOCK_SOLID;
  w.block [i] [1].block_type = BLOCK_SOLID;
  w.block [i] [2].block_type = BLOCK_EDGE_UP;
  w.block [i] [w.blocks.y - 1].block_type = BLOCK_SOLID;
  w.block [i] [w.blocks.y - 2].block_type = BLOCK_SOLID;
  w.block [i] [w.blocks.y - 3].block_type = BLOCK_EDGE_DOWN;
 }
 for (i = 0; i < w.blocks.y; i ++)
 {
  w.block [0] [i].block_type = BLOCK_SOLID;
  w.block [1] [i].block_type = BLOCK_SOLID;
  if (i>1&&i<(w.blocks.y-2))
			w.block [2] [i].block_type = BLOCK_EDGE_LEFT;
  w.block [w.blocks.x - 1] [i].block_type = BLOCK_SOLID;
  w.block [w.blocks.x - 2] [i].block_type = BLOCK_SOLID;
  if (i>1&&i<(w.blocks.y-2))
   w.block [w.blocks.x - 3] [i].block_type = BLOCK_EDGE_RIGHT;

 }

 w.block [2] [2].block_type = BLOCK_EDGE_UP_LEFT;
 w.block [2] [w.blocks.y - 3].block_type = BLOCK_EDGE_DOWN_LEFT;
 w.block [w.blocks.x - 3] [2].block_type = BLOCK_EDGE_UP_RIGHT;
 w.block [w.blocks.x - 3] [w.blocks.y - 3].block_type = BLOCK_EDGE_DOWN_RIGHT;

// initialise vision_area
 w.vision_areas_x = w.blocks.x;
 w.vision_areas_y = w.blocks.y;
// when adding any dynamic memory allocation to this function, remember to free the memory in deallocate_world() below

#ifdef USE_DYNAMIC_MEMORY


 for (p = 0; p < w.players; p ++)
	{
  w.vision_area [p] = calloc(w.vision_areas_x, sizeof(struct vision_area_struct*));
  if (w.vision_area [p] == NULL)
  {
      fprintf(stdout, "g_world.c: Out of memory in allocating w.vision_area");
      error_call();
  }
  for (i = 0; i < w.vision_areas_x; i ++)
  {
   w.vision_area [p] [i] = calloc(w.vision_areas_y, sizeof(struct vision_area_struct));
   if (w.vision_area [p] [i] == NULL)
   {
      fprintf(stdout, "g_world.c: Out of memory in allocating w.vision_area");
      error_call();
   }
  }
	}

 w.core = calloc(w.max_cores, sizeof(struct core_struct));
 if (w.core == NULL)
 {
      fprintf(stdout, "g_world.c: Out of memory in allocating w.core");
      error_call();
 }
// when adding any dynamic memory allocation to this function, remember to free the memory in deallocate_world() below


// allocate memory for the proc array.
// use calloc because this isn't remotely time-critical.
 w.proc = calloc(w.max_procs, sizeof(struct proc_struct));
 if (w.proc == NULL)
 {
      fprintf(stdout, "g_world.c: Out of memory in allocating w.proc");
      error_call();
 }
// when adding any dynamic memory allocation to this function, remember to free the memory in deallocate_world() below

// now allocate the packet array:
 w.packet = calloc(w.max_packets, sizeof(struct packet_struct));
 if (w.packet == NULL)
 {
      fprintf(stdout, "g_world.c: Out of memory in allocating w.packet");
      error_call();
 }
// when adding any dynamic memory allocation to this function, remember to free the memory in deallocate_world() below

// now allocate the cloud array:
 w.cloud = calloc(w.max_clouds, sizeof(struct cloud_struct));
 if (w.cloud == NULL)
 {
      fprintf(stdout, "g_world.c: Out of memory in allocating w.cloud");
      error_call();
 }
// when adding any dynamic memory allocation to this function, remember to free the memory in deallocate_world() below
#endif

// setup players
 for (i = 0; i < w.players; i ++)
 {
  w.player[i].active = 1;
  w.player[i].core_index_start = i * w.cores_per_player;
  w.player[i].core_index_end = (i+1) * w.cores_per_player;
  w.player[i].proc_index_start = i * w.procs_per_player;
  w.player[i].proc_index_end = (i+1) * w.procs_per_player;
  strcpy(w.player[i].name, w_init.player_name [i]); // if loading from disk, the name copied here will be updated later

  w.player[i].processes = 0;
  w.player[i].components_current = 0;
  w.player[i].components_reserved = 0;

  w.player[i].output_console = 0;
//  w.player[i].default_print_colour = PRINT_COL_LGREY;
  w.player[i].error_console = 0;

//  w.player[i].data = w_init.player_starting_data [i]; // should usually be 300
  w.player[i].data = (w_init.starting_data_setting [i] + 1) * 300;
  w.player[i].build_queue[0].active = 0;
  w.player[i].build_queue_fail_reason = BUILD_SUCCESS; // just means no error

  w.player[i].random_seed = w_init.game_seed + (w_init.core_setting + w_init.players) * (i + 7); // doesn't need to be particularly random

 }

// w.system_output_console = 0;
// w.system_error_console = 0;

 initialise_world();

 w.allocated = 1;

#ifdef DEBUG_MODE
w.player[0].data = 2000;
#endif

}




// This function clears an existing world so it can be re-used (or initialises one just created in new_world_from_world_init()).
// The world's basic parameters (e.g. size, number of procs etc) should have been established, so new_world_from_world_init() must have been called first.
void initialise_world(void)
{

 int p, c, i, j;
 struct core_struct* core;
 struct proc_struct* proc;

 w.world_time = BASE_WORLD_TIME; // currently 255 - allows subtraction of small amounts without wrapping the unsigned value
 w.world_seconds = 0;
 w.debug_mode_general = 0;

#ifdef DEBUG_MODE
 w.debug_mode_general = 1;
#endif

// world_time starts after 0 so that things like deallocation counters can be subtracted from it without running into unsigned int problems

 for (c = 0; c < w.max_cores; c ++)
 {
  core = &w.core [c];
  core->exists = 0;
  core->destroyed_timestamp = 0;
  core->index = c;
 }


 for (p = 0; p < w.max_procs; p ++)
 {
  proc = &w.proc [p];
  proc->exists = 0;
  proc->reserved = 0;
  proc->destroyed_timestamp = 0;
  proc->index = p;
 }

 for (i = 0; i < w.blocks.x; i ++)
 {
  for (j = 0; j < w.blocks.y; j ++)
  {
    w.block [i] [j].tag = 0;
    w.block [i] [j].blocklist_down = NULL;
  }
 }

 w.blocktag = 1; // should probably be 1 more than the value that all of the blocks in the world are set to.

 init_packets();
 init_clouds();

 init_world_background();

}



// This function frees the memory used by the worldstruct
// It doesn't otherwise reinitialise the worldstruct
// Call it only when certain that memory has been allocated for the worldstruct (as otherwise it will try to free already free memory)
void deallocate_world(void)
{

#ifdef SANITY_CHECK
 if (w.allocated == 0)
 {
  fprintf(stdout, "\nError: g_world.c:deallocate_world() called when w.allocated already 0.");
  error_call();
 }
#endif


#ifdef USE_DYNAMIC_MEMORY

 int i, p;

// first, free each column of blocks:
 for (i = 0; i < w.blocks.x; i ++)
 {
  free(w.block [i]);
 }

// now free the row of pointers to the columns:
 free(w.block);

// now do the same for vision areas:
 for (p = 0; p < w.players; p ++)
	{
  for (i = 0; i < w.vision_areas_x; i ++)
  {
   free(w.vision_area [p] [i]);
  }
  free(w.vision_area [p]);
 }

// free the rest of the arrays:
 free(w.core);
 free(w.proc);
 free(w.packet);
 free(w.cloud);


#endif

 w.allocated = 0;

}



void run_world(void)
{

// first run the data wells
 int i, j;

 if ((w.world_time & 255) == 0)
	{
		for (i = 0; i < w.data_wells; i ++)
		{
			if (w.data_well[i].active
				&& w.data_well[i].data < w.data_well[i].data_max) // only draw data from reserve if there's space for it
			{
				int data_transferred;
				for (j = 0; j < DATA_WELL_RESERVES; j++)
				{
					if (w.data_well[i].reserve_data [j] == 0)
						continue;
					data_transferred = w.data_well[i].reserve_squares * DATA_WELL_REPLENISH_RATE;
					if (data_transferred > w.data_well[i].data_max - w.data_well[i].data)
						data_transferred = w.data_well[i].data_max - w.data_well[i].data;
					if (data_transferred > w.data_well[i].reserve_data [j])
						data_transferred = w.data_well[i].reserve_data [j];
					w.data_well[i].reserve_data [j] -= data_transferred;
					w.data_well[i].data += data_transferred;
					w.data_well[i].last_transferred = w.world_time;
				}
			}
		}
	}

// Now run the mission, if there is one:
// if (game.type == GAME_TYPE_

}


void disrupt_block_nodes(al_fixed x, al_fixed y, int player_cause, int size)
{

 unsigned int bx = fixed_to_block(x);
 unsigned int by = fixed_to_block(y);

 int i;

 if (bx >= w.blocks.x
  || by >= w.blocks.y) // don't need to check negatives as bx/by are unsigned
  return; // just fail if out of bounds

 struct backblock_struct* backbl = &w.backblock [bx] [by];

 for (i = 0; i < 9; i ++)
 {
  change_block_node(backbl, i, backbl->node_x [i] + grand(size) - grand(size), backbl->node_y [i] + grand(size) - grand(size), backbl->node_size [i] + grand(size) - grand(size));
  change_block_node_colour(backbl, i, player_cause);
  backbl->node_disrupt_timestamp [i] = w.world_time + NODE_DISRUPT_TIME_CHANGE;
 }

}


void disrupt_single_block_node(al_fixed x, al_fixed y, int player_cause, int size)
{

// we need a slight fine-tuning offset here:
 x -= al_itofix(BLOCK_SIZE_PIXELS / 6);
 y -= al_itofix(BLOCK_SIZE_PIXELS / 6);

 unsigned int bx = fixed_to_block(x);
 unsigned int by = fixed_to_block(y);

//  fprintf(stdout, "\ndisrupt_single_block_node() called: block (%i, %i) (%i, %i) at (%f, %f)", bx, by, bx * BLOCK_SIZE_PIXELS, by * BLOCK_SIZE_PIXELS, al_fixtof(x), al_fixtof(y));


 unsigned int i; // if changed to signed, need to make sure bounds checks below check for negative

 if (bx >= w.blocks.x
  || by >= w.blocks.y) // don't check for negatives as bx/by are unsigned
  return; // just fail if out of bounds

 struct backblock_struct* backbl = &w.backblock [bx] [by];

 int int_x = al_fixtoi(x);
 int int_y = al_fixtoi(y);

//  fprintf(stdout, "\nxy (%f, %f) int_xy (%i, %i)", al_fixtof(x), al_fixtof(y), int_x, int_y);

 int_x -= BLOCK_SIZE_PIXELS * bx;
 int_x *= 3;
 int_x /= BLOCK_SIZE_PIXELS;

 int_y -= BLOCK_SIZE_PIXELS * by;
 int_y *= 3;
 int_y /= BLOCK_SIZE_PIXELS;

 i = (int_x * 3) + int_y;
//  fprintf(stdout, "\nnode (%i) from source (%i, %i)", i, int_x, int_y);

#ifdef SANITY_CHECK
 if (i >= 9) // i is unsigned so don't need to check for negative
 {
  fprintf(stdout, "\ng_world.c: disrupt_single_block_node(): invalid node (%i) from source (%f, %f)", i, al_fixtof(x), al_fixtof(y));
  error_call();
 }
#endif

// for (i = 0; i < 9; i ++)
 {
  change_block_node(backbl, i, backbl->node_x [i] + grand(size) - grand(size), backbl->node_y [i] + grand(size) - grand(size), backbl->node_size [i] + grand(size) - grand(size));
  change_block_node_colour(backbl, i, player_cause);
//  bl->node_x [i] += grand(5) - grand(5);
//  bl->node_y [i] += grand(5) - grand(5);
/*
  bl->node_size [i] += grand(size) - grand(size);
  if (bl->node_size [i] < 5)
   bl->node_size [i] = 5;
  if (bl->node_size [i] > 40)
   bl->node_size [i] = 40;*/
  backbl->node_disrupt_timestamp [i] = w.world_time + NODE_DISRUPT_TIME_CHANGE;


 }

}

#define NODE_LOCATION_MIN -5
#define NODE_LOCATION_MAX (BLOCK_SIZE_PIXELS+5)
#define NODE_SIZE_MIN 5
#define NODE_SIZE_MAX 40

// Moves the block node to new_x,new_y
void change_block_node(struct backblock_struct* backbl, int i, int new_x, int new_y, int new_size)
{
return;
 backbl->node_x [i] = new_x;
 if (backbl->node_x [i] < NODE_LOCATION_MIN)
  backbl->node_x [i] = NODE_LOCATION_MIN;
   else
   {
    if (backbl->node_x [i] > NODE_LOCATION_MAX)
     backbl->node_x [i] = NODE_LOCATION_MAX;
   }

 backbl->node_y [i] = new_y;
 if (backbl->node_y [i] < NODE_LOCATION_MIN)
  backbl->node_y [i] = NODE_LOCATION_MIN;
   else
   {
    if (backbl->node_y [i] > NODE_LOCATION_MAX)
     backbl->node_y [i] = NODE_LOCATION_MAX;
   }

 backbl->node_size [i] = new_size;
 if (backbl->node_size [i] < NODE_SIZE_MIN)
  backbl->node_size [i] = NODE_SIZE_MIN;
   else
   {
    if (backbl->node_size [i] > NODE_SIZE_MAX)
     backbl->node_size [i] = NODE_SIZE_MAX;
   }

}


// Moves the block node back towards its original position
void align_block_node(struct backblock_struct* backbl, int i)
{

 if (backbl->node_x [i] < backbl->node_x_base [i])
	{
		backbl->node_x [i] ++;
	}
	 else
		{
   if (backbl->node_x [i] > backbl->node_x_base [i])
		  backbl->node_x [i] --;
		}

 if (backbl->node_y [i] < backbl->node_y_base [i])
	{
		backbl->node_y [i] ++;
	}
	 else
		{
   if (backbl->node_y [i] > backbl->node_y_base [i])
		  backbl->node_y [i] --;
		}

	backbl->node_size [i] += grand(5) - grand(5);

	if (backbl->node_size [i] < 10)
		backbl->node_size [i] = 10;

	if (backbl->node_size [i] > 22)
		backbl->node_size [i] = 22;

}


void change_block_node_colour(struct backblock_struct* backbl, int i, int player_cause)
{

  if (backbl->node_team_col [i] == player_cause)
  {
   if (backbl->node_col_saturation [i] < BACK_COL_SATURATIONS - 1)
    backbl->node_col_saturation [i] ++;
//   fprintf(stdout, "\nBlock node %i colour matches (%i) saturation now %i", i, player_cause, bl->node_col_saturation [i]);
  }
   else
   {
//    fprintf(stdout, "\nBlock node %i colour (%i) changed to (%i)", i, bl->node_team_col [i], player_cause);
    backbl->node_team_col [i] = player_cause;
    backbl->node_col_saturation [i] = 0;
   }


}


/*

 w.block[x][y].vision_block_x [0] = 16;
 w.block[x][y].vision_block_y [0] = 32;

 w.block[x][y].vision_block_x [1] = 80;
 w.block[x][y].vision_block_y [1] = 32;

 w.block[x][y].vision_block_x [2] = 48;
 w.block[x][y].vision_block_y [2] = 96;

 w.block[x][y].vision_block_x [3] = 112;
 w.block[x][y].vision_block_y [3] = 96;



 int i;

 for (i = 0; i < 4; i ++)
	{

  w.block[x][y].vision_block_x_shrink [i] = 1.5 + grand(100) * 0.004;
	}

 return;


 for (i = 0; i < 4; i ++)
	{
			w.block[x][y].vision_block_x_shrink [i] = 1;
			w.block[x][y].vision_block_y_shrink [i] = 1;
	}

 i = 0;


 float split_x = grand(64) + 32;
 float split_y = grand(64) + 32;

 w.block[x][y].vision_block_x [i] = grand(48) + 8;
 w.block[x][y].vision_block_y [i] = grand(48) + 8;

 i++;

 w.block[x][y].vision_block_x [i] = grand(48) + 8 + split_x;
 w.block[x][y].vision_block_y [i] = grand(48) + 8;

 i++;

 w.block[x][y].vision_block_x [i] = grand(48) + 8;
 w.block[x][y].vision_block_y [i] = grand(48) + 8 + split_y;

 i++;

 w.block[x][y].vision_block_x [i] = grand(48) + 8 + split_x;
 w.block[x][y].vision_block_y [i] = grand(48) + 8 + split_y;
*/
//}


/*

Let's try to get the game to a state where it can be started

Need to:
- make w_init then call start_world
- make a template
- call proc creation functions with the template
- that should be enough?

*/

#include "c_compile.h"
#include "c_header.h"
#include "e_editor.h"
//#define TEST_SOURCE_LINES 30
/*
char test_source [1] [TEST_SOURCE_LINES] [80] =
{
{
"#process",
"core_shape_4, 0,",
"  {object_none, 0},",
"  {object_none, 0},",
"  {object_none, 0},",
"  {object_none, 0}",
"#code",
"",
"int a;"
}
};
*/
/*
struct source_struct
{
// lines in the text array should be null-terminated, although actually they don't have to be as each time text is used bounds-checking is done
 char text [SOURCE_TEXT_LINES] [SOURCE_TEXT_LINE_LENGTH];
//  *** text array must be the same as in source_struct (as the code that converts bcode to source code assumes that it can treat source.text in the same way as source_edit.text)

// int src_file [SOURCE_TEXT_LINES]; // stores the index of the file that the line came from
 int from_a_file; // is 1 if loaded from a file, 0 otherwise (will be 0 if a new empty source file created in the editor, until it's saved)
 char src_file_name [FILE_NAME_LENGTH]; // should be empty if from_a_file == 0
 char src_file_path [FILE_PATH_LENGTH]; // same
};
*/

// Loads one of the pre-set source files into a template and compiles it.
void load_mission_source(char* filename, int player_index, int template_index)
{
	int loaded = load_source_file_into_template(filename, player_index, template_index);//, PATH_TYPE_STORY);

	if (loaded != 1)
	{
		fpr("\nError: failed to load mission file [%s] into template[%i][%i].", filename, player_index, template_index);
// Causes a fatal error on failure as these source files should be known to be valid
		error_call();
	}

#ifndef DEBUG_MODE
// don't automatically lock enemy templates in debug mode
	templ[player_index][template_index].locked = 1; // load_source_file_into_template calls compile(), which should do enough for it to be okay to lock these templates.
#endif

 templ[player_index][template_index].mission_template = 1; // prevents various things

}

// Loads default sources specified in init.txt
// Prints a message (to stdout) on failure, but doesn't break anything
void load_default_source(char* filename, int player_index, int template_index)
{

	fpr("\nLoading default source file [%s] into player %i template %i... ", filename, player_index, template_index);

	int loaded = load_source_file_into_template(filename, player_index, template_index);

 switch(loaded)
 {
 	case -1: // failed to open file
 		fpr(" load failed :(");
 		break;
 	case 0: // failed to compile
 		fpr(" could not compile :(");
 		break;
 	case 1: // success
 		fpr(" loaded.");
 		break;

 }


}

// Returns:
//  -1 failed to load source file
//  0 loaded source file, but failed to compile
//  1 success
int load_source_file_into_template(char* filename, int player_index, int template_index)
{

	int open_the_template = 1;

	if (templ[player_index][template_index].locked)
		open_the_template = 0;

 if (load_source_file_into_template_without_compiling(filename, player_index, template_index, open_the_template) == -1)
		return -1;

 return compile(&templ[player_index][template_index], templ[player_index][template_index].source_edit, COMPILE_MODE_BUILD);
// returns 1 or 0

}

// open_the_template should be 1 if the template is already open and doesn't need to be reset (currently used for editor reload function)
int load_source_file_into_template_without_compiling(char* filename, int player_index, int template_index, int open_the_template)//, int path_type)
{

 int target_esource = (player_index * TEMPLATES_PER_PLAYER) + template_index;

 struct source_struct temp_src;

 if (!load_source_file(filename, &temp_src))
		return -1;
/*	{
		fpr("\n Error: failed to load file [%s]", filename);
		error_call();
	}*/

 if (open_the_template)
  open_new_template(&templ[player_index][template_index]);

 source_to_editor(&temp_src, target_esource);
 editor.source_edit[target_esource].from_a_file = 1;
 editor.source_edit[target_esource].saved = 1;

	return 1;

}

