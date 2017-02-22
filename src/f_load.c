

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"

#include "m_maths.h"

#include "g_misc.h"
#include "g_world.h"
#include "g_motion.h"
#include "g_proc.h"
#include "g_game.h"
//#include "i_header.h"
#include "i_console.h"
#include "i_view.h"
#include "i_disp_in.h"
#include "i_input.h"
#include "t_template.h"

#include "e_log.h"
#include "e_editor.h"

#include "s_mission.h"

#include "f_load.h"

extern ALLEGRO_DISPLAY* display;
extern struct view_struct view;
//extern struct templstruct templ [TEMPLATES]; // see t_template.c
//extern struct consolestruct console [CONSOLES];
extern struct control_struct control;

extern struct world_init_struct w_init;
extern struct game_struct game;


#define WORLD_TIME_LIMIT (1<<30)



struct load_statestruct load_state;


int load_game_from_file(void);
int load_game_struct_from_file(void);
int load_world_from_file(void);
int load_world_properties_from_file(void);
int load_procs_from_file(void);
void load_proc(int p);
int load_packets_from_file(void);
int load_packet(int p);
int load_clouds_from_file(void);
void load_cloud(int c);
int load_blocks_from_file(void);
void load_block(struct block_struct* bl);
int verify_block_type(int x, int y, int want_type);
void block_error(int x, int y, int type, int want_type);
int load_players_from_file(void);
void load_player(int p);
int load_markers_from_file(void);
void load_marker(int m);
int load_view_from_file(void);
int load_control_from_file(void);
//int load_templates_from_file(void);
int load_consoles_etc_from_file(void);
void load_console(int c);
void load_cline(int c, int line);
void load_colours(int* cols, const char* name);

//int verify_group_data(void);
//int verify_group_data_recursive(struct proc_struct* pr, int g, struct groupstruct* gr, int distance_from_first_member, int* proc_group_membership_verified);
//int verify_method_data(struct proc_struct* pr, struct methodstruct* meth, int m);
//void verify_method_data_value(int value, int min, int max, const char* name);
void verify_any_value(int value, int min, int max, const char* name);
//void confirm_method_data_value(int value1, int value2, const char* name);






// call this when load game item selected from game system menu
// opens native file dialogue and loads file
// writes to mlog on failure or error
int load_game(void)
{
/*
 if (!open_load_file("Open saved game", "*.*"))
  return 0;

 load_state.bp = -1; // means first entry read will be 0
 load_state.error = 0;
 load_state.current_buffer_size = 0;

// we've started loading a world, so we may need to deallocate the present world:
 if (w.allocated == 1)
  deallocate_world();

// load first buffer:
 if (!read_load_buffer())
 {
  close_load_file();
  return 0;
 }

 if (!load_game_from_file())
 {
  close_load_file();
  return 0;
 }

 flush_game_event_queues(); // loading may have taken some time

 close_load_file();
 write_line_to_log("Save file loaded.", MLOG_COL_FILE);

*/
 return 1; // success!!

}

// call this as part of loading a game
//  - does not initialise bp, so it can be called after other stuff (e.g. turn file details) has been read
int load_game_from_file(void)
{
/*
 initialise_view(settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
 init_consoles();

 if (!load_game_struct_from_file()
  || !load_world_from_file()
  || !load_view_from_file()
  || !load_templates_from_file()
  || !load_consoles_etc_from_file()
  || !load_control_from_file())
 {
  return 0;
 }

 if (load_state.error == 1)
  return 0;
*/
 return 1;

}


int load_game_struct_from_file(void)
{
/*
 init_game(); // initialises game_struct *** no it doesn't

 load_int(&game.phase, 0, GAME_PHASES - 1, "game phase");
// fprintf(stdout, "\nGame phase %i", game.phase);
 load_int(&game.turns, 0, MAX_LIMITED_GAME_TURNS, "game turns");
 if (load_state.error == 1)
  return 0;
// checked error because current_turn bounds depend on game.turns
 if (game.turns == 0)
  load_int(&game.current_turn, 0, MAX_UNLIMITED_GAME_TURNS, "current turn");
   else
    load_int(&game.current_turn, 0, game.turns, "current turn");
 load_int(&game.minutes_each_turn, 0, MAX_TURN_LENGTH_MINUTES, "turn length");
 load_int(&game.current_turn_time_left, 0, 9000000, "time left"); // in ticks
 load_int(&game.pause_soft, 0, 1, "soft pause");
 load_int(&game.pause_hard, 0, 1, "hard pause");

 if (game.minutes_each_turn == 0)
  game.current_turn_time_left = 0; // current_turn_time_left should always be zero if turns are not time-limited


 if (load_state.error == 1)
  return 0;
*/
 return 1;

}

int load_world_from_file(void)
{
/*
 if (!load_world_properties_from_file())
  return 0;

// now try to catch some complicated errors that can't be detected by simple bounds-checking of loaded values, or that can only be checked when all of the data has been loaded in:
 if (!verify_group_data()) // this also sets some group properties (e.g. total_number)
  return 0; // note that verify_group_data must be called after new_world_from_world_init() (as it uses w.current_check)

 int i, j;
 struct proc_struct* pr;

 for (i = 0; i < w.max_procs; i ++)
 {
  if (w.proc[i].exists > 0)
  {
   pr = &w.proc[i];
   for (j = 0; j < METHODS; j ++)
   {
// verify_method_data tries to detect unreasonable values in the method[].data array (it doesn't check mbank, as the contents of the mbank can be determined by a user)
    verify_method_data(pr, &pr->method[j], j);
   }
  }
  if (load_state.error != 0)
   return 0;
 }

// now work out the w.player[].processes values (these were set to zero earlier):

 for (i = 0; i < w.max_procs; i ++)
 {
  if (w.proc[i].exists != 0) // counts deallocating procs as existing
   w.player[w.proc[i].player_index].processes ++;
 }

 for (i = 0; i < w.players; i ++)
 {
  if (w.player[i].processes > w.procs_per_player)
  {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: player ");
     write_number_to_log(i);
     write_to_log(" has too many processes (");
     write_number_to_log(w.player[i].processes);
     write_to_log("/");
     write_number_to_log(w.procs_per_player);
     write_to_log(").");
     finish_log_line();
     load_state.error = 1;
     return 0;
  }
 }
*/
 return 1;

}


int load_world_properties_from_file(void)
{
/*
// some of these things are #defined in s_setup.h

 int i;

 load_int(&w_init.players, MIN_PLAYERS, MAX_PLAYERS, "players");
 load_int(&w_init.procs_per_player, MIN_procs_per_player, MAX_procs_per_player, "processes per team");
// load_int(&w_init.gen_limit, MIN_GEN_LIMIT, MAX_GEN_LIMIT, "gen limit");
 load_int(&w_init.packets_per_player, MIN_PACKETS, MAX_PACKETS, "packets");
 load_int(&w_init.max_clouds, CLOUDS, CLOUDS, "clouds");
 load_int(&w_init.w_block, MIN_BLOCKS, MAX_BLOCKS, "w_blocks");
 load_int(&w_init.h_block, MIN_BLOCKS, MAX_BLOCKS, "h_blocks");
 for (i = 0; i < w_init.players; i ++)
 {
//  load_int(&w_init.team_colour [i], 0, TEAM_COLOUR_CHOICES - 1, "team colours");
  load_int(&w_init.may_change_client_template [i], 0, 1, "change client template");
  load_int(&w_init.may_change_proc_templates [i], 0, 1, "change process templates");
  strcpy(w_init.player_name[i], "<null>"); // actual player names will be read in directly to the player structs during player loading
 }
 load_int(&w_init.player_clients, 0, 1, "player clients");
 load_int(&w_init.allow_observer, 0, 1, "allow observer");
 load_int(&w_init.player_operator, -1, w_init.players - 1, "operator");
// load_int(&w_init.system_program, 0, 1, "system program");
// load_state.error = 1;
// return 0;
 load_colours(w.background_colour, "background colour");
 map_background_colour();
 load_colours(w.hex_base_colour, "hex base colour");
// hex_base_colour is mapped in individual player colour mapping

 if (load_state.error == 0)
  new_world_from_world_init(); // if this fails (e.g. can't allocate memory) it gives a fatal error
   else
    return 0;
// new_world_from_world_init sets up a number of values not loaded in, like w.current_check and w.max_procs (which is calculated from other values)
// This means that we can use those values from now on (especially w.max_procs, which is used a lot).

// from this point on are values that do not go in w_init, and values that may have been initialised in new_world_from_world_init() but need to be set to the correct values (like w.world_time)

 load_unsigned_int(&w.world_time, 0, WORLD_TIME_LIMIT, "world time");
 load_unsigned_int(&w.total_time, 0, WORLD_TIME_LIMIT, "total time");

// These values are currently set up when loading programs from templates, which won't be done when loading a file.
// Need to initialise properly. ??? <- I can't remember what this comment means now

// load_int(&w.permit_operator_player, 0, 1, "permit operator");
 load_int(&w.actual_operator_player, -1, w_init.players - 1, "actual operator");

// the following functions assume that new_world_from_world_init() has successfully allocated all memory needed for the world
 if (!load_procs_from_file())
  return 0;
 if (!load_groups_from_file())
  return 0;
 if (!load_packets_from_file())
  return 0;
 if (!load_clouds_from_file())
  return 0;

// load_int(&w.blocktag, 0, 10000000, "blocktag");
// NO - w.blocktag is initialised to 1 in new_world_from_world_init() call. when blocks are loaded, their tags are set to 0 or 1 depending on whether something is present.

 if (!load_blocks_from_file())
  return 0;

 if (!load_players_from_file())
  return 0;

// struct playerstruct player [PLAYERS];  - this is saved in save_world_to_file()

 load_int(&w.system_program.active, 0, 1, "system active");
 if (w.system_program.active == 1)
 {
  load_program(&w.system_program, PROGRAM_TYPE_SYSTEM);
 }
 load_int(&w.system_output_console, -1, CONSOLES - 1, "system output console");
 load_int(&w.system_error_console, -1, CONSOLES - 1, "system error console");

 load_int(&w.observer_program.active, 0, 1, "observer active");
 if (w.observer_program.active == 1)
 {
  load_program(&w.observer_program, PROGRAM_TYPE_OBSERVER);
 }
 load_int(&w.observer_output_console, -1, CONSOLES - 1, "observer output console");
 load_int(&w.observer_error_console, -1, CONSOLES - 1, "observer error console");

 if (!load_markers_from_file())
  return 0;
//  int last_marker; this is reset each tick and doesn't need to be saved

 for (i = 0; i < SYSTEM_COMS; i ++)
 {
  load_short(&w.system_com [i], 0, 0, 0, "system com");
 }

 load_int(&w.playing_mission, -1, MISSIONS - 1, "playing mission");

 if (load_state.error)
  return 0;
*/
 return 1;

}


// this function assumes that the proc array has been allocated and initialised to all zeros
// also assumes that world has been set up, so that things like world size information can be relied on.
int load_procs_from_file(void)
{
/*
 int proc_index = 0;

 while(TRUE)
 {
  load_int(&proc_index, -1, w.max_procs - 1, "process index");

  if (proc_index == -1 // finished!
   || load_state.error == 1)
   break;
//  fprintf(stdout, "\n2nd: Proc %i", proc_index);
  if (w.proc[proc_index].exists != 0)
  {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: duplicate process (index ");
     write_number_to_log(proc_index);
     write_to_log(") found.");
     finish_log_line();
     load_state.error = 1;
     return 0;
  }
  if (load_state.error)
   return 0;
  load_proc(proc_index);
 };

 if (load_state.error == 1)
  return 0;
*/
 return 1;

}

void load_proc(int p)
{
/*
 if (load_state.error == 1)
  return;

 int i;

 struct proc_struct* pr = &w.proc[p];
 load_int(&pr->exists, -1, 1, "process exists"); // -1 means that is was destroyed recently and is deallocating
 load_int(&pr->deallocating, 0, DEALLOCATE_COUNTER, "deallocate");
 load_int(&pr->player_index, 0, w.players - 1, "player index");
// pr->index = p; don't need to do this (it's done in world initialisation)
// load_int(&pr->index, 0, w.max_procs - 1, "process index");
// load_int(&pr->index_for_client); not currently used
 for (i = 0; i < COMMANDS; i ++)
 {
  load_short(&pr->command [i], 0, 0, 0, "command");
 }
 load_int(&pr->selected, 0, 1000, "selected"); // shouldn't matter what this is really
 load_int(&pr->select_time, 0, WORLD_TIME_LIMIT, "select_time");
 load_int(&pr->deselected, 0, 1000, "deselected");
 load_int(&pr->map_selected, 0, 1000, "map_selected");
 load_int(&pr->hit, 0, 1000, "hit");
 load_int(&pr->stuck, 0, 1000, "stuck");
 load_proc_pointer(&pr->stuck_against, "stuck against");
 load_int(&pr->mass, 0, 10000, "process mass");
 load_int(&pr->moment, 0, 100000000, "process moment");
 load_int(&pr->method_mass, 0, 10000, "method mass");
 load_int(&pr->hp, 0, 10000, "hp");
 load_int(&pr->hp_max, 0, 10000, "hp max");
 load_int(&pr->mobile, 0, 1, "mobile");
 load_int(&pr->redundancy, 0, 1000, "redundancy");
 load_int(&pr->shape, 0, SHAPES - 1, "process shape");
 load_int(&pr->size, 0, SHAPES_SIZES - 1, "process_size");
 pr->shape_str = &shape_dat [pr->shape] [pr->size];
// struct shape_struct* shape_str; - load function should assign pointer based on shape and size
 load_int(&pr->single_proc_irpt, 0, 10000, "single_proc_irpt");
 load_int(&pr->single_proc_irpt_max, 0, 10000, "single_proc_irpt_max");  // I guess I could check that irpt <= irpt_max
 load_int(&pr->irpt_gain, 0, 10000, "irpt_gain");
 load_int(&pr->irpt_gain_max, 0, 10000, "irpt_gain_max");
 load_int(&pr->single_proc_data, 0, 10000, "single_proc_data");
 load_int(&pr->single_proc_data_max, 0, 10000, "single_proc_data_max");  // I guess I could check that data <= data_max
// load_int(&pr->irpt_gen_number, 0, 100, "gen number");
// load_int(&pr->irpt_use, 0, 10000, "irpt use");
// load_int(&pr->irpt_base, 0, 30000, "irpt base");
 load_int(&pr->base_cost, 0, 10000, "base_cost");
 load_int(&pr->instructions_each_execution, 0, 100000, "instructions each");
 load_int(&pr->instructions_left_after_last_exec, 0, 100000, "instructions left");
 load_object_coordinates(&pr->x, &pr->y, "process location");
 pr->x_block = fixed_to_block(pr->x); // don't need to verify this as pr->x already verified
 pr->y_block = fixed_to_block(pr->y);
// load_int(&pr->x_block, 2, w.w_block - 2, "process x block");
// load_int(&pr->y_block, 2, w.h_block - 2, "process y block");
 load_fixed(&pr->x_speed, 1, NEG_MAX_SPEED_FIXED, MAX_SPEED_FIXED, "process x_speed");
 load_fixed(&pr->y_speed, 1, NEG_MAX_SPEED_FIXED, MAX_SPEED_FIXED, "process y_speed");
 load_fixed(&pr->angle, 0, 0, 0, "angle");
 pr->angle &= AFX_MASK;
 load_fixed(&pr->spin, 1, NEGATIVE_SPIN_MAX_FIXED, SPIN_MAX_FIXED, "process spin");
 load_fixed(&pr->spin_change, 1, NEGATIVE_SPIN_MAX_FIXED, SPIN_MAX_FIXED, "process spin change");
 load_int(&pr->hit_edge_this_cycle, 0, 1, "process hit edge");
 load_object_coordinates(&pr->old_x, &pr->old_y, "process old location");
 load_fixed(&pr->old_angle, 0, 0, 0, "process old angle");
 pr->old_angle &= AFX_MASK;
 load_object_coordinates(&pr->provisional_x, &pr->provisional_y, "process provisional location");
 load_fixed(&pr->provisional_angle, 0, 0, 0, "process provisional angle");
 pr->provisional_angle &= AFX_MASK;
 load_int(&pr->prov, 0, 1, "process prov"); // not sure this needs to be saved
 load_fixed(&pr->max_length, 1, 0, BLOCK_SIZE_FIXED, "process max_length"); // need to just use shape value!
 load_fixed(&pr->drag, 1, 0, al_itofix(2), "process drag");
 load_proc_pointer(&pr->blocklist_up, "process blocklist up");
 load_proc_pointer(&pr->blocklist_down, "process blocklist down");
 load_int(&pr->group, -1, w.max_groups, "process group");
 if (pr->group == -1)
	{
		pr->irpt = &pr->single_proc_irpt;
		pr->irpt_max = &pr->single_proc_irpt_max;
		pr->data = &pr->single_proc_data;
		pr->data_max = &pr->single_proc_data_max;
	}
	 else
		{
		 pr->irpt = &w.group[pr->group].shared_irpt;
		 pr->irpt_max = &w.group[pr->group].shared_irpt_max;
		 pr->data = &w.group[pr->group].shared_data;
		 pr->data_max = &w.group[pr->group].shared_data_max;
		}
 pr->number_of_group_connections = 0;
 for (i = 0; i < GROUP_CONNECTIONS; i ++)
 {
  load_proc_pointer(&pr->group_connection [i], "process connection");
  load_int(&pr->connection_vertex [i], 0, pr->shape_str->vertices - 1, "connection vertex");
  if (load_state.error == 1)
		{
//			fprintf(stdout, "\npr %i con %i shape %i", pr->index, i, pr->shape);
   return;
		}


  load_int(&pr->connected_from [i], 0, GROUP_CONNECTIONS - 1, "connected from");
  if (load_state.error == 1)
   return;
  if (pr->group_connection [i] != NULL)
  {
   pr->number_of_group_connections ++;
   // can only properly verify connected_from if connected proc already loaded (so its index < p):
   if (pr->group_connection [i]->index < p)
   {
    if (pr->group_connection[i]->connected_from [pr->connected_from[i]] != i
     || pr->group_connection[i]->group != pr->group)
    {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: invalid group connection.");
     finish_log_line();
     load_state.error = 1;
     return;
    }
   }
  }
  load_fixed(&pr->connection_angle [i], 0, 0, 0, "connection angle");
  pr->connection_angle [i] &= AFX_MASK;
  load_fixed(&pr->connection_dist [i], 1, al_itofix(0), al_itofix(300), "connection dist");
  load_fixed(&pr->connection_angle_difference [i], 0, 0, 0, "connection angle difference");
  pr->connection_angle_difference [i] &= AFX_MASK;
 }
// load_int(&pr->number_of_group_connections, 0, 3, "group connections"); set above
 load_fixed(&pr->group_angle, 0, 0, 0, "group angle");
 pr->float_angle = fixed_to_radians(pr->angle);
 load_fixed(&pr->group_distance, 1, 0, GROUP_MAX_DISTANCE, "group distance");
 load_int(&pr->group_number_from_first, 0, GROUP_MAX_MEMBERS - 1, "group number");
 load_fixed(&pr->group_angle_offset, 0, 0, 0, "group angle offset");
 pr->group_angle_offset &= AFX_MASK;
// not sure all these test values need to be saved...
 load_fixed(&pr->test_group_distance, 1, 0, GROUP_MAX_DISTANCE, "test group distance");
 load_object_coordinates(&pr->test_x, &pr->test_y, "process test location");
 load_fixed(&pr->test_angle, 0, 0, 0, "process test angle");
 pr->test_angle &= ANGLE_MASK;
// load_int(&pr->test_x_block, 2, w.w_block - 2, "process test x block");
// load_int(&pr->test_y_block, 2, w.h_block - 2, "process test y block");
 pr->group_check = 0;
// load_int(&pr->group_check, 0, 30000000, "group check");
//  * shouldn't save this!
// end test values
 load_bcode(&pr->bcode, PROC_BCODE_SIZE); //, "process bcode");
 load_regs(&pr->regs); //, "process registers");
 load_methods(pr->method, PROGRAM_TYPE_PROCESS, "process methods");
 load_int(&pr->execution_count, 0, 10000, "process execution count");
 load_int_unchecked(&pr->lifetime, "process lifetime");
 if (pr->lifetime < 0) // shouldn't be negative, but can be very high
 {
  simple_load_error("negative process lifetime.");
  return;
 }
// int active_method_list_permanent [METHODS]; // this is a linked list of active methods, built on proc creation.
// int active_method_list [METHODS]; // this is a linked list of active methods currently doing something, built after each execution and modified
// int active_method_list_back [METHODS]; // backwards links corresponding to active_method_list, to make it a double linked list
 load_int(&pr->listen_method, -1, METHODS - 1, "process listen method");
 load_int(&pr->allocate_method, -1, METHODS - 1, "process allocate method");
 load_int(&pr->virtual_method, -1, METHODS - 1, "process virtual method");
 load_int(&pr->give_irpt_link_method, -1, METHODS - 1, "give_irpt_link_method");
 load_int(&pr->give_data_link_method, -1, METHODS - 1, "give_data_link_method");
 if (load_state.error == 1)
  return; // don't want to use give_irpt_link_method or give_data_link_method as array index if either caused an error
 if (pr->give_irpt_link_method != -1
  &&	pr->method[pr->give_irpt_link_method].type != MTYPE_PR_LINK)
	{
  simple_load_error("give_irpt_link_method not a link method.");
  return;
	}
 if (pr->give_data_link_method != -1
  &&	pr->method[pr->give_data_link_method].type != MTYPE_PR_LINK)
	{
  simple_load_error("give_data_link_method not a link method.");
  return;
	}
// load_int(&pr->special_method_penalty, 1, 64, "special method penalty");
// struct proc_struct* scanlist_down; - don't need to save (only relevant during a single scan call)
// al_fixed scan_dist; - same
 load_int_unchecked(&pr->scan_bitfield, "scan bitfield");
 for (i = 0; i < METHOD_VERTICES; i++)
 {
  load_int(&pr->vertex_method [i], -1, METHODS - 1, "vertex method");
  if (load_state.error == 1)
   return;
// make sure vertex_method matches the method value:
  if (pr->vertex_method[i] != -1)
  {
   struct methodstruct* vmeth = &pr->method[pr->vertex_method[i]];
   if (mtype[vmeth->type].external == MEXT_INTERNAL)
   {
    simple_load_error("internal method found on vertex.");
    return;
   }
   if (vmeth->ex_vertex != i)
   {
    simple_load_error("ex_vertex does not match method vertex.");
    return;
   }
// should check ex_angle here to make sure method isn't facing inward, but I can't really be bothered (it won't break anything)
  }
 }

 if (load_state.error == 1)
  return;

// now that the proc has been loaded, we need to verify some stuff:
 for (i = 0; i < METHODS; i ++)
 {
  if (mtype[pr->method[i].type].external != MEXT_INTERNAL)
  {
// make sure its external vertex is within bounds:
   if (pr->method[i].ex_vertex < 0
    || pr->method[i].ex_vertex >= pr->shape_str->vertices)
   {
    load_error(pr->method[i].ex_vertex, 0, pr->shape_str->vertices - 1, "external method vertex");
    return;
   }
// make sure vertex's vertex_method and method's ex_vertex match up
   if (pr->vertex_method[pr->method[i].ex_vertex] != i)
   {
    simple_load_error("vertex method and ex_vertex do not match.");
    return;
   }
  } // end if mtype[].externa != MEXT_INTERNAL
 }
*/

}

/*
// assumes bc is a valid bcodestruct
void load_bcode(struct bcode_struct* bc, int bcode_size)
{

 int i;

 bc->bcode_size = bcode_size;

 for (i = 0; i < bc->bcode_size; i ++)
 {
  load_short(&bc->op[i], 0, 0, 0, "bcode");
// 	if (i < 10)
//			fprintf(stdout, "\nbcode %i: %i", i, bc->op[i]);
 }
 load_int(&bc->type, 0, PROGRAM_TYPES - 1, "bcode type");
 load_int(&bc->shape, 0, SHAPES - 1, "bcode shape");
 load_int(&bc->size, 0, SHAPES_SIZES - 1, "bcode process size");
 bc->printing = 0;
 load_int(&bc->from_a_file, 0, 1, "bcode from a file");
 load_string(bc->src_file_name, FILE_NAME_LENGTH, 0, "bcode source file name");
 load_string(bc->src_file_path, FILE_PATH_LENGTH, 0, "bcode source file path");
 bc->static_length = 0; // not sure about this

}
*/

/*

// methods should be a pointer to an array of METHODS + 1 methodstructs
// vertices should be number of vertices if proc (otherwise doesn't matter)
void load_methods(struct methodstruct* methods, int program_type, const char* name)
{

 int m;
 int i;
 struct methodstruct* meth;

// fprintf(stdout, "\nchecking methods for type %i (name %s)", program_type, name);

 for (m = 0; m < METHODS; m ++)
 {
  meth = &methods[m];
  load_int(&meth->type, 0, MTYPES - 1, "method type");
// check method suitability for program type:
  if ((program_type == PROGRAM_TYPE_PROCESS
    && !check_valid_proc_method(meth->type, 1))
   || (program_type != PROGRAM_TYPE_PROCESS
    && !check_method_type(meth->type, program_type, 1))) // check_method_type can't be used for procs
  {
//    fprintf(stdout, "\nwrong method: program_type %i method_type %i (%s)", program_type, meth->type, mtype[meth->type].name);
    start_log_line(MLOG_COL_ERROR);
    write_to_log("Load error: wrong program type for method type.");
    finish_log_line();
    load_state.error = 1;
    return;
  }
  for (i = 0; i < METHOD_DATA_VALUES; i ++)
  {
   load_int_unchecked(&meth->data[i], "method data");
  }
  for (i = 0; i < METHOD_EXTENSIONS; i ++)
  {
   load_int(&meth->extension[i], 0, 10, "method extension"); // 10 = arbitrary number
  }
  load_int_unchecked(&meth->ex_vertex, "external vertex"); // for procs this is verified later (after all methods and ex_vertex velues have been loaded)
  load_int_unchecked(&meth->ex_angle, "external angle"); // this is also verified later
  meth->ex_angle &= AFX_MASK;
 }

}

int load_groups_from_file(void)
{

 int group_index = 0;

 while(TRUE)
 {
  load_int(&group_index, -1, w.max_groups - 1, "group index");
  if (group_index == -1
   || load_state.error == 1)
   break;
  if (w.group[group_index].exists != 0)
  {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: duplicate group (index ");
     write_number_to_log(group_index);
     write_to_log(") found.");
     finish_log_line();
     load_state.error = 1;
     return 0;
  }
  if (load_state.error)
   return 0;
  load_group(group_index);
 };

 if (load_state.error == 1)
  return 0;

 return 1;

}

void load_group(int g)
{

// int i;

 struct groupstruct* gr = &w.group[g];
 load_int(&gr->exists, -1, 1, "group exists");
 load_proc_pointer(&gr->first_member, "group first member");
 load_int(&gr->shared_irpt, 0, 100000, "group shared irpt");
 load_int(&gr->shared_irpt_max, 0, 100000, "group shared irpt max");
 load_int(&gr->total_irpt_gain_max, 0, 100000, "group irpt gain max");
 load_int(&gr->shared_data, 0, 100000, "group shared data");
 load_int(&gr->shared_data_max, 0, 100000, "group shared data max");
 load_int(&gr->total_mass, 0, 100000, "group total mass");
 load_int(&gr->total_members, 0, GROUP_MAX_MEMBERS, "group member");
 load_int(&gr->moment, 0, 1000000000, "group moment");
 load_int(&gr->mobile, 0, 1, "group mobile");
 load_object_coordinates(&gr->x, &gr->y, "group location");
 load_object_coordinates(&gr->test_x, &gr->test_y, "group test location");
// load_object_coordinates(&gr->centre_of_mass_test_x, &gr->centre_of_mass_test_y, "group centre of mass"); - wrong - these are ints. But in any case there's no need to save these values
 load_fixed(&gr->x_speed, 1, NEG_MAX_SPEED_FIXED, MAX_SPEED_FIXED, "group x speed");
 load_fixed(&gr->y_speed, 1, NEG_MAX_SPEED_FIXED, MAX_SPEED_FIXED, "group y speed");
 load_fixed(&gr->drag, 1, 0, al_itofix(1), "group drag");
 load_fixed(&gr->angle, 0, 0, 0, "group angle");
 gr->angle &= AFX_MASK;
 load_fixed(&gr->test_angle, 0, 0, 0, "group test angle");
 gr->test_angle &= AFX_MASK;
 load_fixed(&gr->spin, 1, NEGATIVE_SPIN_MAX_FIXED, SPIN_MAX_FIXED, "group spin");
 load_fixed(&gr->spin_change, 1, NEGATIVE_SPIN_MAX_FIXED, SPIN_MAX_FIXED, "group spin change");
 load_int(&gr->hit_edge_this_cycle, 0, 1, "group hit edge");
 load_int(&gr->stuck, 0, 1000, "group stuck");

}
*/

int load_packets_from_file(void)
{
/*
 int packet_index = 0;

 while(TRUE)
 {
  load_int(&packet_index, -1, w.max_packets - 1, "packet index");
  if (packet_index == -1
   || load_state.error == 1)
   break;
  if (w.packet[packet_index].exists != 0)
  {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: duplicate packet (index ");
     write_number_to_log(packet_index);
     write_to_log(") found.");
     finish_log_line();
     load_state.error = 1;
     return 0;
  }
  if (load_state.error)
   return 0;
  if (!load_packet(packet_index))
			return 0;
 };

 if (load_state.error == 1)
  return 0;
*/
 return 1;

}

int load_packet(int p)
{
/*
 struct packet_struct* pk = &w.packet[p];
 int i;

 load_int(&pk->exists, 0, 1, "packet exists");
 load_int(&pk->timeout, 0, 1000, "packet timeout");
 load_int(&pk->time, 0, 1000, "packet time");
 pk->index = p;
 load_int(&pk->type, 0, PACKET_TYPES - 1, "packet type");
 load_int(&pk->player_index, 0, MAX_PLAYERS - 1, "packet player");
 load_int(&pk->source_proc, -1, w.max_procs - 1, "packet source proc");
 load_int(&pk->source_method, 0, METHODS - 1, "packet source method");
 load_int(&pk->source_vertex, 0, 1000, "packet source vertex");
 if (load_state.error)
  return 0;
// need to verify source vertex
 if (pk->source_proc != -1)
	{
		if (pk->source_vertex >= w.proc[pk->source_proc].shape_str->vertices)
		{
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: invalid packet source vertex (vertex ");
     write_number_to_log(pk->source_vertex);
     write_to_log(" > ");
     write_number_to_log(w.proc[pk->source_proc].shape_str->vertices);
     write_to_log(").");
     finish_log_line();
     load_state.error = 1;
     return 0;
		}
	}
 load_int_unchecked(&pk->prand_seed, "packet prand seed");

 load_int(&pk->damage, 0, 100000, "packet damage");
 load_int(&pk->team_safe, -1, 100000, "packet team safe"); // doesn't matter if this is a ridiculous value
 load_int(&pk->colour, 0, PLAYERS - 1, "packet colour");
 for (i = 0; i < METHOD_EXTENSIONS; i ++)
 {
  load_int(&pk->method_extensions [i], 0, 5, "packet method ext");
 }
 load_int(&pk->collision_size, 0, 50, "packet collision size");
 load_object_coordinates(&pk->x, &pk->y, "packet location");
 pk->x_block = fixed_to_block(pk->x);
 pk->y_block = fixed_to_block(pk->y);
 load_fixed(&pk->x_speed, 1, al_itofix(-30), al_itofix(30), "packet x speed");
 load_fixed(&pk->y_speed, 1, al_itofix(-30), al_itofix(30), "packet y speed");
 load_fixed(&pk->angle, 0, 0, 0, "packet angle");
 pk->angle &= AFX_MASK;
 load_packet_pointer(&pk->blocklist_up, "packet blocklist up");
 load_packet_pointer(&pk->blocklist_down, "packet blocklist down");
// load_int(&pk->tail_count, 0, 1000, "packet tail count");
// load_object_coordinates(&pk->tail_x, &pk->tail_y, "packet tail location");
*/
 return 1;

}


int load_clouds_from_file(void)
{
/*
 int cloud_index = 0;

 while(TRUE)
 {
  load_int(&cloud_index, -1, w.max_clouds - 1, "cloud index");
  if (cloud_index == -1
   || load_state.error == 1)
   break;
  if (w.cloud[cloud_index].exists != 0)
  {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: duplicate cloud (index ");
     write_number_to_log(cloud_index);
     write_to_log(") found.");
     finish_log_line();
     load_state.error = 1;
     return 0;
  }
  if (load_state.error)
   return 0;
  load_cloud(cloud_index);
 };

 if (load_state.error == 1)
  return 0;
*/
 return 1;

}

void load_cloud(int c)
{
/*
 struct cloud_struct* cl = &w.cloud[c];
 int i;

 load_int(&cl->exists, 0, 1, "cloud exists");
 load_int(&cl->timeout, 0, 1000, "cloud timeout");
 load_int(&cl->type, 0, CLOUD_TYPES - 1, "cloud type");
 load_int(&cl->colour, 0, PLAYERS - 1, "cloud colour"); // MAX VALUE (CLOUD_COLS) MAY NOT BE CORRECT
// load_object_coordinates(&cl->x, &cl->y, "cloud location"); - bounds-checking shouldn't matter for clouds
 load_fixed(&cl->x, 0, 0, 0, "cloud x");
 load_fixed(&cl->y, 0, 0, 0, "cloud y");
 load_fixed(&cl->x_speed, 1, al_itofix(-30), al_itofix(30), "cloud x speed");
 load_fixed(&cl->y_speed, 1, al_itofix(-30), al_itofix(30), "cloud y speed");
 load_fixed(&cl->angle, 0, 0, 0, "cloud angle");
 cl->angle &= AFX_MASK;
 for (i = 0; i < CLOUD_DATA; i ++)
 {
  load_int_unchecked(&cl->data [i], "cloud data");
  switch(cl->type)
  {
   case CLOUD_YIELD_LINE:
    verify_any_value(cl->data [0], 0, w.max_procs - 1, "yield line index");
    break;
  }
 }
*/
}





int load_blocks_from_file(void)
{
/*
 if (load_state.error == 1)
  return 0;

 int i, j;
 struct block_struct* bl;

 for (i = 0; i < w.w_block; i ++)
 {
  for (j = 0; j < w.h_block; j ++)
  {
   bl = &w.block[i][j];
   load_block(bl);
   if (load_state.error == 1)
    return 0;
  }
 }

 if (load_state.error)
  return 0;

// now verify the block types
// Check top two and bottom two rows:
 for (i = 0; i < w.w_block; i ++)
 {
  if (!verify_block_type(i, 0, BLOCK_SOLID)
   || !verify_block_type(i, 1, BLOCK_SOLID)
   || !verify_block_type(i, w.h_block - 1, BLOCK_SOLID)
   || !verify_block_type(i, w.h_block - 2, BLOCK_SOLID))
    return 0;
 }
// Check third and third-from-bottom rows:
 for (i = 3; i < w.w_block - 4; i ++)
 {
  if (!verify_block_type(i, 2, BLOCK_EDGE_UP)
   || !verify_block_type(i, w.h_block - 3, BLOCK_EDGE_DOWN))
    return 0;
 }
// check left two and right two columns
 for (i = 0; i < w.h_block; i ++)
 {
  if (!verify_block_type(0, i, BLOCK_SOLID)
   || !verify_block_type(1, i, BLOCK_SOLID)
   || !verify_block_type(w.w_block - 1, i, BLOCK_SOLID)
   || !verify_block_type(w.w_block - 2, i, BLOCK_SOLID))
    return 0;
 }
// Check third from left and third from right columns:
 for (i = 3; i < w.h_block - 4; i ++)
 {
  if (!verify_block_type(2, i, BLOCK_EDGE_LEFT)
   || !verify_block_type(w.w_block - 3, i, BLOCK_EDGE_RIGHT))
    return 0;
 }
// now check four inner corners:
 if (!verify_block_type(2, 2, BLOCK_EDGE_UP_LEFT)
  || !verify_block_type(w.w_block - 3, 2, BLOCK_EDGE_UP_RIGHT)
  || !verify_block_type(2, w.h_block - 3, BLOCK_EDGE_DOWN_LEFT)
  || !verify_block_type(w.w_block - 3, w.h_block - 3, BLOCK_EDGE_DOWN_RIGHT))
   return 0;
// finally check everything else
 for (i = 3; i < w.w_block - 4; i ++)
 {
  for (j = 3; j < w.h_block - 4; j ++)
  {
  if (!verify_block_type(i, j, BLOCK_NORMAL))
    return 0;
  }
 }

 if (load_state.error)
  return 0;
*/
 return 1;

}

int verify_block_type(int x, int y, int want_type)
{
/*
 if (w.block[x][y].block_type != want_type)
 {
  block_error(x, y, w.block[x][y].block_type, want_type);
  return 0;
 }*/

 return 1;
}

void load_block(struct block_struct* bl)
{
/*
 int i;

 load_proc_pointer(&bl->blocklist_down, "blocklist down");
 if (bl->blocklist_down == NULL)
  bl->tag = 0;
   else
    bl->tag = 1;

 load_packet_pointer(&bl->packet_down, "packet blocklist down");
 if (bl->packet_down == NULL)
  bl->packet_tag = 0;
   else
    bl->packet_tag = 1;

 load_int(&bl->block_type, 0, BLOCK_TYPES - 1, "block type");
 for (i = 0; i < BLOCK_NODES; i++)
 {
  load_int_unchecked(&bl->node_x [i], "block node_x");
  load_int_unchecked(&bl->node_y [i], "block node_y");
  load_int_unchecked(&bl->node_size [i], "block node_y");
  if (load_state.error == 1)
   return;
  change_block_node(bl, i, bl->node_x[i], bl->node_y[i], bl->node_size[i]);
  load_int(&bl->node_team_col[i], 0, PLAYERS - 1, "block node colour");
  load_int(&bl->node_col_saturation[i], 0, BACK_COL_SATURATIONS - 1, "block node saturation");
  load_unsigned_int(&bl->node_disrupt_timestamp[i], 0, w.world_time + NODE_DISRUPT_TIME_CHANGE, "block node timestamp");
  load_int_unchecked(&bl->node_x_base [i], "block node_x_base");
  load_int_unchecked(&bl->node_y_base [i], "block node_y_base");

 }
*/
}

void block_error(int x, int y, int type, int want_type)
{

     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: invalid block type at ");
     write_number_to_log(x);
     write_to_log(",");
     write_number_to_log(y);
     write_to_log(".");
     finish_log_line();

     start_log_line(MLOG_COL_ERROR);
     write_to_log("Type ");
     write_number_to_log(type);
     write_to_log(" (should be ");
     write_number_to_log(want_type);
     write_to_log(").");
     finish_log_line();

     load_state.error = 1;

     return;

}

// assumes w.players has been set
int load_players_from_file(void)
{
/*
 int i;

 for (i = 0; i < w.players; i ++)
 {
  load_player(i);
  if (load_state.error)
   return 0;
 }
*/
 return 1;

}

void load_player(int p)
{
/*
 struct playerstruct* pl = &w.player[p];

// these should be set up in setup_world_from_world_init
//  int active;
//  int proc_index_start; // where this team starts in the proc array
//  int proc_index_end; // all procs on this team should be below this (the maximum is one less)
//  int program_type_allowed;

// load_int(&pl->colour, 0, TEAM_COLOUR_CHOICES - 1, "player colour");
 load_colours(pl->proc_colour_min, "player proc colour_min");
 load_colours(pl->proc_colour_max, "player proc colour_max");
 load_colours(pl->packet_colour_min, "player packet colour_min");
 load_colours(pl->packet_colour_max, "player packet colour_max");
 load_colours(pl->drive_colour_min, "player drive colour_min");
 load_colours(pl->drive_colour_max, "player drive colour_max");
 map_player_base_colours(p);
 map_player_packet_colours(p);
 map_player_drive_colours(p);
 map_player_virtual_colours(p);
 map_background_colour();
// don't need to call map_hex_colours; map_player_base_colours calls it

 load_int(&pl->client_program.active, 0, 1, "player client program active");
 if (pl->client_program.active == 1)
 {
  if (w.actual_operator_player == p)
   load_program(&pl->client_program, PROGRAM_TYPE_OPERATOR);
    else
     load_program(&pl->client_program, PROGRAM_TYPE_DELEGATE);
 }
// load_int(&pl->controlling_user, 0, 1, "player controlling");
 load_int_unchecked(&pl->score, "player score");
 load_int(&pl->allocate_effect_type, 0, ALLOCATE_EFFECT_TYPES - 1, "allocate_effect_type");
// load_int(&pl->processes, 0, w.procs_per_player, "player processes"); - should calculate this instead of loading it
 pl->processes = 0; // will be calculated later from the actual number of procs
 load_int(&pl->output_console, -1, CONSOLES - 1, "player output console");
 load_int(&pl->output2_console, -1, CONSOLES - 1, "player output2 console");
 load_int(&pl->error_console, -1, CONSOLES - 1, "player error console");
// load_int(&pl->default_print_colour, 0, PRINT_COLS - 1, "player print colour");
 load_string(pl->name, PLAYER_NAME_LENGTH, 0, "player name");

*/
}







int load_markers_from_file(void)
{
/*
 int marker_index = 0;

 while(TRUE)
 {
  load_int(&marker_index, -1, MARKERS - 1, "marker index");
  if (marker_index == -1
   || load_state.error == 1)
   break; // finished!
  if (w.marker[marker_index].active != 0)
  {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: duplicate marker (index ");
     write_number_to_log(marker_index);
     write_to_log(") found.");
     finish_log_line();
     load_state.error = 1;
     return 0;
  }
  if (load_state.error)
   return 0;
  load_marker(marker_index);
 };
*/
 return 1;

}

void load_marker(int m)
{
/*
 struct markerstruct* mk = &w.marker[m];

 load_int(&mk->active, 0, 1, "marker active");
 load_int(&mk->timeout, -1, 1000, "marker timeout"); // -1 is indefinite
 load_int(&mk->type, 0, MARKER_TYPES - 1, "marker type");
 load_int(&mk->phase, 0, MARKER_PHASES - 1, "marker phase");
 load_int(&mk->phase_counter, 0, MARKER_PHASE_TIME, "marker phase counter");
 load_int(&mk->colour, 0, BASIC_COLS - 1, "marker colour");
 load_int(&mk->size, 0, MARKER_SIZE_MAX - 1, "marker size");
 load_int_unchecked(&mk->angle, "marker angle");
 mk->angle &= ANGLE_MASK;
 load_int(&mk->spin, -MARKER_SPIN_MAX, MARKER_SPIN_MAX - 1, "marker spin");
 load_fixed(&mk->x, 0, 0, 0, "marker x");
 load_fixed(&mk->y, 0, 0, 0, "marker y");
 load_fixed(&mk->x2, 0, 0, 0, "marker x2");
 load_fixed(&mk->y2, 0, 0, 0, "marker y2");
 load_int(&mk->bind_to_proc, -1, w.max_procs - 1, "marker bind to process");
 load_int(&mk->bind_to_proc2, -1, w.max_procs - 1, "marker bind to process2");
*/
}


int load_view_from_file(void)
{
/*
 load_fixed(&view.camera_x, 0, 0, 0, "camera x");
 load_fixed(&view.camera_y, 0, 0, 0, "camera y");
 load_int(&view.window_x, 480, 10000, "window x"); // zoomed as well? Actually why is this value being saved at all??
 load_int(&view.window_y, 480, 10000, "window y");
// load_int(&view.just_resized, 0, 1, "view resized");
 view.just_resized = 1;
 view.centre_x = al_itofix(view.window_x / 2);
 view.centre_y = al_itofix(view.window_y / 2);
 load_int_unchecked(&view.fps, "fps");
 load_int_unchecked(&view.cycles_per_second, "cps");
// load_int(&view.paused, 0, 1, "paused"); // not sure about this one
// load_int(&view.pause_advance_pressed, 0, 1, "pause advance"); // or this one
 load_proc_pointer(&view.focus_proc, "focus process");

 if (load_state.error == 1)
  return 0;

// several values can be derived from the values loaded in so far:
 view.w_x_pixel = al_fixtoi(w.w_fixed); // width of window in pixels
 view.w_y_pixel = al_fixtoi(w.h_fixed);
 view.camera_x_min = BLOCK_SIZE_FIXED * 2;
 view.camera_y_min = BLOCK_SIZE_FIXED * 2;
 view.camera_x_max = w.w_fixed - (BLOCK_SIZE_FIXED * 2);
 view.camera_y_max = w.h_fixed - (BLOCK_SIZE_FIXED * 2);

 load_int(&view.map_visible, 0, 1, "map visible");
 load_int_unchecked(&view.map_x, "map x");
 load_int_unchecked(&view.map_y, "map y");
 load_int(&view.map_w, MAP_MINIMUM_SIZE, view.window_x, "map size x");
 load_int(&view.map_h, MAP_MINIMUM_SIZE, view.window_y, "map size y");
 load_fixed(&view.map_proportion_x, 0, 0, 0, "map proportion x");
 load_fixed(&view.map_proportion_y, 0, 0, 0, "map proportion y");
*/
 return 1;

}



int load_control_from_file(void)
{
/*
 int i;

 initialise_control();

// probably only need to save values where it is significant whether a button/key is being held, was just pressed etc
 load_int(&control.mbutton_press [0], -1, 2, "left mouse button");
 load_int(&control.mbutton_press [1], -1, 2, "right mouse button");

 for (i = 0; i < KEYS; i ++)
 {
  load_int(&control.key_press [i], -1, 2, "key press");
 }

 if (load_state.error == 1)
  return 0;

*/
 return 1;

}


int load_templates_from_file(void)
{
/*
 reset_templates();

 int templ_index = 0;

 while(TRUE)
 {
  load_int(&templ_index, -1, TEMPLATES - 1, "template index");
  if (templ_index == -1
   || load_state.error == 1)
   break; // finished!
  if (templ[templ_index].type != TEMPL_TYPE_CLOSED)
  {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: duplicate template (index ");
     write_number_to_log(templ_index);
     write_to_log(") found.");
     finish_log_line();
     load_state.error = 1;
     return 0;
  }
  if (load_state.error)
   return 0;
  load_template(templ_index, 1);
 };

 finish_template_setup();
*/
 return 1;

}

// t is template index.
// load_basic_template_details == 1 indicates that we're loading a saved game or gamefile, so the basic details of each template need to be retrieved
// load_basic_template_details == 0 indicates that we're loading a turnfile, so the basic details will come from a saved game or gamefile and don't need to be loaded here.
//   - also means that a new template will not be added.
//   - also doesn't save path
void load_template(int t, int load_basic_template_details)
{
/*
 int t_type = 0;
 int t_player = 0;
 int t_fixed = 0;
 int t_access_index = 0;
 int t_category_heading = 0;

 if (load_basic_template_details == 1)
 {
  load_int(&t_type, 0, TEMPL_TYPES - 1, "template type");
//  fprintf(stdout, "\nTemplate type: %i", t_type);
  load_int(&t_player, -1, w.players - 1, "template player");
  load_int(&t_fixed, 0, 1, "template fixed");
  load_int(&t_access_index, -1, 4, "template access index");
  load_int(&t_category_heading, 0, 1, "template category heading");

  int template_added = add_template(t_type, t_player, t_fixed, t_access_index, t_category_heading);

  if (template_added != t)
  {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: unexpected template index (is ");
     write_number_to_log(template_added);
     write_to_log("; expected ");
     write_number_to_log(t);
     write_to_log(").");
     finish_log_line();
     load_state.error = 1;
     return;
  }
 }

 struct templstruct* tp = &templ[t];

 load_int(&tp->contents.loaded, 0, 1, "template loaded");
 load_string(tp->contents.file_name, FILE_NAME_LENGTH, 0, "template file name");
 if (load_basic_template_details == 1)
  load_string(tp->contents.file_path, FILE_PATH_LENGTH, 0, "template file path");
 load_int(&tp->contents.origin, 0, TEMPL_ORIGINS - 1, "template origin");

 switch(templ[t].type)
 {
  case TEMPL_TYPE_SYSTEM:
   load_bcode(&tp->contents.bcode, SYSTEM_BCODE_SIZE);
   break;
  case TEMPL_TYPE_PROC:
  case TEMPL_TYPE_DEFAULT_PROC:
   load_bcode(&tp->contents.bcode, PROC_BCODE_SIZE);
   break;
  case TEMPL_TYPE_DELEGATE:
  case TEMPL_TYPE_OBSERVER:
  case TEMPL_TYPE_OPERATOR:
  case TEMPL_TYPE_DEFAULT_OPERATOR:
   load_bcode(&tp->contents.bcode, CLIENT_BCODE_SIZE);
   break;
 }

 if (load_basic_template_details == 1
  && tp->contents.loaded == 1)
  set_opened_template(t, t_type, t_fixed);


// int x1, y1, x2, y2; - these should be set during initialisation


*/

}



// loads consoles, channels and score boxes from file
int load_consoles_etc_from_file(void)
{
/*
 int console_index = -1;

 if (load_state.error)
  return 0;

 while(TRUE)
 {
  load_int(&console_index, -1, CONSOLES - 1, "console index");
  if (console_index == -1
   || load_state.error == 1)
   break; // finished!
  if (console[console_index].open != CONSOLE_CLOSED)
  {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: duplicate console (index ");
     write_number_to_log(console_index);
     write_to_log(") found.");
     finish_log_line();
     load_state.error = 1;
     return 0;
  }
  if (load_state.error)
   return 0;
  load_console(console_index);
 };

 if (load_state.error)
  return 0;
*/
 return 1;

}

void load_console(int c)
{
/*
// int i;
 struct consolestruct* con = &console[c];


 load_int_unchecked(&con->x, "console x"); // don't need to check - if console is off screen it isn't drawn and can't be clicked on
 load_int_unchecked(&con->y, "console y");
 load_int(&con->h_lines, CONSOLE_LINES_MIN, CONSOLE_LINES_MAX, "console height");
 load_int(&con->w_letters, CONSOLE_LETTERS_MIN, CONSOLE_LETTERS_MAX, "console line length");
// load_int(con->h_pixels); - this is worked out from lines
// save_int(con->w_pixels); - worked out from letters
 load_int(&con->open, CONSOLE_CLOSED, CONSOLE_OPEN_MAX, "console open");
 load_int(&con->printed_this_tick, 0, 1, "console printed");
// load_int(&con->line_highlight);

 // not sure about the following:
// int button_x [CONSOLE_BUTTONS]; // x position (offset from CONSOLE_X) of buttons that open/close console
// int button_highlight; // indicates mouse is over one of the buttons (-1 if no highlight)

 load_int(&con->style, 0, CONSOLE_STYLES - 1, "console style");
 load_int(&con->font_index, 0, FONTS - 1, "console font");
 load_int(&con->colour_index, 0, BASIC_COLS - 1, "console colour");

// struct slider_struct scrollbar_v;  not sure about this.

 load_string(con->title, CONSOLE_TITLE_LENGTH, 1, "console title");

 set_console_size_etc(c, con->w_letters, con->h_lines, con->font_index, 1); // the 1 means that the console's properties will be updated even though its size values will not be changed



// set_console_size_etc clears the console, so everything that would be affected by that should be below:

 load_int(&con->cpos, 0, CLINES - 1, "console position");
 load_int(&con->window_pos, 0, CLINES, "console window position");

 int cline_index = -1;

 while(TRUE)
 {
  load_int(&cline_index, -1, CLINES - 1, "cline index");
  if (cline_index == -1
   || load_state.error == 1)
   break; // finished!
  if (console[c].cline[cline_index].used != 0)
  {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: duplicate console line (index ");
     write_number_to_log(cline_index);
     write_to_log(") found.");
     finish_log_line();
     load_state.error = 1;
     return;
  }
  if (load_state.error)
   return;
  load_cline(c, cline_index);
 }
*/

}



















void load_cline(int c, int line)
{
/*
 struct consolelinestruct* col = &console[c].cline[line];

// save_int(col->used); // not strictly necessary
 col->used = 1;
 load_int(&col->colour, 0, PRINT_COLS, "console line colour");
 load_string(col->text, CLINE_LENGTH, 0, "console line");
 load_int(&col->source_program_type, -1, PROGRAM_TYPES, "console line program type");
 load_int_unchecked(&col->source_index, "console line source index"); // should be being bounds-checked when used
 load_int(&col->action_type, CONSOLE_ACTION_NONE, CONSOLE_ACTIONS - 1, "console line action");
 load_short(&col->action_val1, 0, 0, 0, "console line action 1");
 load_short(&col->action_val2, 0, 0, 0, "console line action 2");
 load_short(&col->action_val3, 0, 0, 0, "console line action 3");
 load_unsigned_int(&col->time_stamp, 0, WORLD_TIME_LIMIT, "console line time stamp");

// fprintf(stdout, "\nloaded console %i line %i: %s", c, line, col->text);
*/
}


// This function tries to check proc connection and location values for group members against values for the group
// it also sets some things like group member number
// must be called after start_world_from_world_init() call above as it uses w.current_check
int verify_group_data(void)
{
/*
 int i, g;
 struct groupstruct* gr;
 int proc_group_membership_verified [MAX_PROCS_IN_WORLD]; // this is set in verify_group_data_recursive for group members

 for (i = 0; i < w.max_procs; i ++)
 {
  if (w.proc[i].exists <= 0
   || w.proc[i].group == -1)
   proc_group_membership_verified [i] = 1;
    else
     proc_group_membership_verified [i] = 0;
 }

 for (g = 0; g < w.max_groups; g ++)
 {
  gr = &w.group[g];

  gr->total_members = 0; // this is added to in verify_group_data_recursive()

  if (gr->exists == 0)
   continue;

  w.current_check ++;

  if (!verify_group_data_recursive(gr->first_member, g, gr, 0, proc_group_membership_verified))
   return 0;

  if (gr->total_members >= GROUP_MAX_MEMBERS)
  {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: group has too many members.");
     finish_log_line();
     load_state.error = 1;
     return 0;
  }
 }

// now check the proc_group_membership_verified array - all procs that exist should have a 1 value
 for (i = 0; i < w.max_procs; i ++)
 {
  if (proc_group_membership_verified [i] != 1)
  {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: invalid process group membership.");
     finish_log_line();
     load_state.error = 1;
     return 0;
  }
 }
*/
 return 1;
}
/*
// this function checks each group member's group settings
// and also adds up the group's total members
// and sets each proc's proc_group_membership_verified value
int verify_group_data_recursive(struct proc_struct* pr, int g, struct groupstruct* gr, int distance_from_first_member, int* proc_group_membership_verified)
{

 int i;

 if (pr->group != g)
 {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: invalid process group index.");
     finish_log_line();
     load_state.error = 1;
     return 0;
 }

 if (pr->group_number_from_first != distance_from_first_member)
 {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: invalid process group number from first.");
     finish_log_line();
     load_state.error = 1;
     return 0;
 }

// test group location
 al_fixed test_x, test_y;
 test_x = gr->x + fixed_xpart(pr->group_angle + gr->angle, pr->group_distance);
 test_y = gr->y + fixed_ypart(pr->group_angle + gr->angle, pr->group_distance);
 if (!check_object_coordinates(test_x, test_y, "group location"))
  return 0;

// add proc to group member count
 gr->total_members ++;
 pr->group_check = w.current_check;

 for (i = 0; i < GROUP_CONNECTIONS; i ++)
 {
  if (pr->group_connection [i] != NULL
   && pr->group_connection [i]->group_check != w.current_check)
  {
   if (!verify_group_data_recursive(pr->group_connection [i], g, gr, distance_from_first_member + 1, proc_group_membership_verified))
    return 0;
  }
 }

 proc_group_membership_verified [pr->index] = 1;

 return 1;


}*/

/*
void verify_method_data_value(int value, int min, int max, const char* name)
{

 if (load_state.error)
  return;

 if (value < min || value > max)
 {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: method data value ");
     write_to_log(name);
     write_to_log(" is ");
     write_number_to_log(value);
     write_to_log(" (should be ");
     write_number_to_log(min);
     write_to_log(" to ");
     write_number_to_log(max);
     write_to_log(").");
     finish_log_line();

     load_state.error = 1;

 }


}
*/

void verify_any_value(int value, int min, int max, const char* name)
{

 if (load_state.error)
  return;

 if (value < min || value > max)
 {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: value ");
     write_to_log(name);
     write_to_log(" is ");
     write_number_to_log(value);
     write_to_log(" (should be ");
     write_number_to_log(min);
     write_to_log(" to ");
     write_number_to_log(max);
     write_to_log(").");
     finish_log_line();

     load_state.error = 1;

 }


}

// like verify_method_data_value, but just checks for equality
void confirm_method_data_value(int value1, int value2, const char* name)
{

 if (load_state.error)
  return;

 if (value1 != value2)
 {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: method data value ");
     write_to_log(name);
     write_to_log(" is ");
     write_number_to_log(value1);
     write_to_log(" (should be ");
     write_number_to_log(value2);
     write_to_log(").");
     finish_log_line();

     load_state.error = 1;

 }


}

void load_colours(int* cols, const char* name)
{

 load_int_unchecked(&cols [0], name);
 load_int_unchecked(&cols [1], name);
 load_int_unchecked(&cols [2], name);
// load_int_unchecked is used because colours are bounds-checked in the colour setting functions

}



// general functions follow:


void load_int(int* value, int min, int max, const char* name)
{
 int loaded [4];
// char cvalues [4];

 loaded[0] = load_8b(name);
 loaded[1] = load_8b(name);
 loaded[2] = load_8b(name);
 loaded[3] = load_8b(name);

 *value = (loaded[0] * (1<<24)) + (loaded[1] * (1<<16)) + (loaded[2] * (1<<8)) + (loaded[3]);


 if (*value < min
  || *value > max)
 {
  load_error(*value, min, max, name);
  return;
 }

}

// just like int but unsigned
void load_unsigned_int(unsigned int* value, int min, int max, const char* name)
{
 int loaded [4];
// char cvalues [4];

 loaded[0] = load_8b(name);
 loaded[1] = load_8b(name);
 loaded[2] = load_8b(name);
 loaded[3] = load_8b(name);

/* if (!loaded[0]
  || !loaded[1]
  || !loaded[2]
  || !loaded[3])
   return; // error message should have been written*/

 *value = ((unsigned int) loaded[0] << 24) | ((unsigned int) loaded[1] << 16) | ((unsigned int) loaded[2] << 8) | ((unsigned int) loaded[3]);

 if (*value < min
  || *value > max)
 {
  load_error(*value, min, max, name);
  return;
 }

}



void load_int_unchecked(int* value, const char* name)
{
 int loaded [4];
// char cvalues [4];

 loaded[0] = load_8b(name);
 loaded[1] = load_8b(name);
 loaded[2] = load_8b(name);
 loaded[3] = load_8b(name);

/* if (!loaded[0]
  || !loaded[1]
  || !loaded[2]
  || !loaded[3])
   return; // error message should have been written*/

 *value = ((int) loaded[0] << 24) | ((int) loaded[1] << 16) | ((int) loaded[2] << 8) | ((int) loaded[3]);

}


void load_unsigned_int_unchecked(unsigned int* value, const char* name)
{
 int loaded [4];
// char cvalues [4];

 loaded[0] = load_8b(name);
 loaded[1] = load_8b(name);
 loaded[2] = load_8b(name);
 loaded[3] = load_8b(name);

/* if (!loaded[0]
  || !loaded[1]
  || !loaded[2]
  || !loaded[3])
   return; // error message should have been written*/

 *value = ((unsigned int) loaded[0] << 24) | ((unsigned int) loaded[1] << 16) | ((unsigned int) loaded[2] << 8) | ((unsigned int) loaded[3]);

}


void load_short(s16b* value, int check_min_max, int min, int max, const char* name)
{
 int loaded [2];
// char cvalues [2];

 loaded[0] = load_8b(name);
 loaded[1] = load_8b(name);

/* if (!loaded[0]
  || !loaded[1])
   return; // error message should have been written*/

 *value = ((int) loaded[0] << 8) | ((int) loaded[1]);

 if (check_min_max
  && (*value < min
   || *value > max))
 {
  load_error(*value, min, max, name);
  return;
 }

}


void load_fixed(al_fixed* value, int check_min_max, al_fixed min, al_fixed max, const char* name)
{
 int loaded [4];
// char cvalues [4];

 min = al_fixsub(min, 1); // give fixed values a little bit of leeway
 max = al_fixadd(max, 1);

 loaded[0] = load_8b(name);
 loaded[1] = load_8b(name);
 loaded[2] = load_8b(name);
 loaded[3] = load_8b(name);

/* if (!loaded[0]
  || !loaded[1]
  || !loaded[2]
  || !loaded[3])
   return; // error message should have been written*/

 *value = ((int) loaded[0] << 24) | ((int) loaded[1] << 16) | ((int) loaded[2] << 8) | ((int) loaded[3]);

// fprintf(stdout, "\nload_fixed (%s) %f (min %f max %f error %i) :", name, al_fixtof(*value), al_fixtof(min), al_fixtof(max), load_state.error);

 if (check_min_max
 && (*value < min
  || *value >= max))
 {
  load_error_fixed(*value, min, max, name);
  return;
 }

}

// this function loads a string and confirms that it is null terminated
// if accept_line_breaks==0, it will generate an error if \n or \r found
void load_string(char* str, int length, int accept_line_breaks, const char* name)
{

 if (load_state.error == 1)
  return;

 int i;

 for (i = 0; i < length; i ++)
 {
  load_char(str+i, name);
 }

 if (load_state.error == 1)
  return;

 for (i = 0; i < length; i ++)
 {

// Probably better to treat line breaks as terminators:
  if (str[i] == '\0'
			|| str[i] == '\n'
   || str[i] == '\r')
  {
// might as well zero out the rest of the string
   while (i < length)
   {
    str[i] = '\0';
    i++;
   }
   return; // success!
  }
 }

// no null terminator found...
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: ");
     write_to_log(name);
     write_to_log(" not null terminated.");
     finish_log_line();
     load_state.error = 1;

}


void load_char(char* value, const char* name)
{
// int loaded = load_8b(name);

 *value = load_8b(name);


}

int load_8b(const char* name)
{
 if (load_state.error == 1)
  return 0; // fail silently (an error message should already have been written)

 load_state.bp ++;

 if (load_state.bp == load_state.current_buffer_size)
 {
  if (!read_load_buffer()) // read_buffer should display its own error message
  {
   start_log_line(MLOG_COL_ERROR);
   write_to_log("While reading ");
   write_to_log(name);
   write_to_log(".");
   finish_log_line();

   load_state.error = 1;
   return 0;
  }
  load_state.bp = 0;
 }

// *value = load_state.buffer[load_state.bp];

 return load_state.buffer[load_state.bp] & 0xff;

}

// loads an int and converts it to a pointer to proc[loaded value] (or NULL if value is -1)
void load_proc_pointer(struct proc_struct** pr, const char* name)
{

 int pr_index = -1;

 load_int(&pr_index, -1, w.max_procs - 1, name);

 if (load_state.error == 1)
  return;

 if (pr_index == -1)
 {
  *pr = NULL;
  return;
 }

 *pr = &w.proc[pr_index];

}


// loads an int and converts it to a pointer to proc[loaded value] (or NULL if value is -1)
void load_packet_pointer(struct packet_struct** pk, const char* name)
{

 int pk_index = -1;

 load_int(&pk_index, -1, w.max_packets - 1, name);

 if (load_state.error == 1)
  return;

 if (pk_index == -1)
 {
  *pk = NULL;
  return;
 }

 *pk = &w.packet[pk_index];

}


// this function loads x/y coordinates and makes sure that they are within bounds (which means inside the edge blocks)
void load_object_coordinates(al_fixed* x, al_fixed* y, const char* name)
{

 load_fixed(x, 1, al_itofix(-1000), al_itofix(32000), name);
 load_fixed(y, 1, al_itofix(-1000), al_itofix(32000), name);

 if (load_state.error == 1)
 {
  return;
 }

 check_object_coordinates(*x, *y, name);

}

int check_object_coordinates(al_fixed x, al_fixed y, const char* name)
{

 if (x < (BLOCK_SIZE_FIXED * 2)
  || x >= al_fixsub(w.fixed_size.x, (BLOCK_SIZE_FIXED * 2))
  || y < (BLOCK_SIZE_FIXED * 2)
  || y >= al_fixsub(w.fixed_size.y, (BLOCK_SIZE_FIXED * 2)))
 {

     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: ");
     write_to_log(name);
     write_to_log(" (");
     write_number_to_log(al_fixtoi(x));
     write_to_log(", ");
     write_number_to_log(al_fixtoi(y));
     write_to_log(") outside (");
     write_number_to_log(al_fixtoi(al_fixsub(w.fixed_size.x, (BLOCK_SIZE_FIXED * 2))));
     write_to_log(", ");
     write_number_to_log(al_fixtoi(al_fixsub(w.fixed_size.y, (BLOCK_SIZE_FIXED * 2))));
     write_to_log(").");
     finish_log_line();
     load_state.error = 1;
     return 0;
 }

 return 1;

}


void load_error(int value, int min, int max, const char* name)
{

 if (load_state.error == 1)
  return; // must have already displayed error message

     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: value [");
     write_to_log(name);
     write_to_log("] is ");
     write_number_to_log(value);
     write_to_log(" (should be ");
     write_number_to_log(min);
     write_to_log(" to ");
     write_number_to_log(max);
     write_to_log(").");
     finish_log_line();

     load_state.error = 1;

}


// this does the same thing as load_fixed unless the fprintf is used
void load_error_fixed(al_fixed value, al_fixed min, al_fixed max, const char* name)
{

// fprintf(stdout, "\nfixed error: value %f min %f max %f name %s", al_fixtof(value), al_fixtof(min), al_fixtof(max), name);

 if (load_state.error == 1)
  return; // must have already displayed error message

     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: value [");
     write_to_log(name);
     write_to_log("] is ");
     write_number_to_log(al_fixtoi(value));
     write_to_log(" (should be ");
     write_number_to_log(al_fixtoi(min));
     write_to_log(" to ");
     write_number_to_log(al_fixtoi(max));
     write_to_log(").");
     finish_log_line();

     load_state.error = 1;

}


void simple_load_error(const char* name)
{

 if (load_state.error == 1)
  return; // must have already displayed error message

     start_log_line(MLOG_COL_ERROR);
     write_to_log("Load error: ");
     write_to_log(name);
     finish_log_line();

     load_state.error = 1;

}





int open_load_file(const char* dialog_name, const char* file_extension)
{

 load_state.file_dialog = al_create_native_file_dialog("", dialog_name, file_extension,  ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);

 if (load_state.file_dialog == NULL)
 {
  write_line_to_log("Error: couldn't open Allegro file dialog!", MLOG_COL_ERROR);
  return 0; // should this be an error_call()?
 }

 al_show_mouse_cursor(display);
 al_show_native_file_dialog(display, load_state.file_dialog); // this should block everything else until it finishes.
 al_hide_mouse_cursor(display);

 int files_to_open = al_get_native_file_dialog_count(load_state.file_dialog);

 if (files_to_open == 0)
 {
  goto load_fail;
 }

 if (files_to_open > 1)
 {
  write_line_to_log("Can only open one file at a time, sorry.", MLOG_COL_ERROR); // not sure this is necessary if the multiple file flag isn't set for al_create_native_file_dialog()
  goto load_fail;
 }

 const char* file_path_ptr = al_get_native_file_dialog_path(load_state.file_dialog, 0);

// fprintf(stdout, "\nFile number %i path (%s)", files_to_open, file_path_ptr);

 if (strlen(file_path_ptr) >= FILE_PATH_LENGTH) // not sure this is needed
 {
  write_line_to_log("File path too long, sorry.", MLOG_COL_ERROR);
  goto load_fail;
 }

// open the file:
 load_state.file = fopen(file_path_ptr, "rb");

 if (!load_state.file)
 {
  write_line_to_log("Error: failed to open target file.", MLOG_COL_ERROR);
  goto load_fail;
 }

// file is still open for reading. Needs to be closed when reading finished.

 al_destroy_native_file_dialog(load_state.file_dialog);
 return 1;

load_fail:
 al_destroy_native_file_dialog(load_state.file_dialog);
 return 0;

}

int read_load_buffer(void)
{

 int read_in = fread(load_state.buffer, 1, LOAD_BUFFER_SIZE, load_state.file);

 if (ferror(load_state.file)
  || read_in == 0)
 {
//     fprintf(stdout, "\nError: buf_length %i written %i", buf_length, written);
     write_line_to_log("Error: file read failed.", MLOG_COL_ERROR);
     load_state.error = 1;
     load_state.bp = 0; // might as well
     return 0;
 }

 load_state.current_buffer_size = read_in;
// doesn't reset load_state.bp - calling function must do that (as sometimes it needs to be set to 0 and sometimes to -1)
 return 1;

}


void close_load_file(void)
{

 fclose(load_state.file);

}

