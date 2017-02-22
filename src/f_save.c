
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"

#include "g_misc.h"
#include "i_console.h"
#include "t_template.h"

#include "e_log.h"
#include "e_editor.h"

#include "f_save.h"

extern ALLEGRO_DISPLAY* display; // used to display the native file dialog

extern struct game_struct game;
extern struct view_struct view;
//extern struct templstruct templ [TEMPLATES]; // see t_template.c
//extern struct consolestruct console [CONSOLES];
extern struct control_struct control;

struct save_statestruct save_state;

/*

This file contains functions for saving games.

Need to save the following things:

- header containing information about file
- everything in worldstruct
- everything in view_struct
- contents of all templates
- possibly contents of game_struct (not sure about this)
- contents of all consoles (including all channel assignments and similar)

probably don't want to save input stuff.
 - so make sure that the save game function is only called before ex_control is read in and control is filled

Format:
 - world data
 - procs
  - each proc preceded by a number (proc index). -1 indicates end.

*/

int save_game_to_file(void);

int save_game_struct_to_file(void);
int save_world_to_file(void);
int save_world_properties_to_file(void);
int save_procs_to_file(void);
void save_proc(int p);
//void save_regs(struct registerstruct* regs);
//void save_methods(struct methodstruct* methods);
int save_groups_to_file(void);
void save_group(int g);
int save_packets_to_file(void);
void save_packet(int p);
int save_clouds_to_file(void);
void save_cloud(int c);
int save_blocks_to_file(void);
void save_block(struct block_struct* bl);
int save_players_to_file(void);
void save_player(int p);
//void save_program(struct programstruct* cl);
int save_markers_to_file(void);
void save_marker(int m);
int save_view_to_file(void);
int save_templates_to_file(void);
int save_consoles_etc_to_file(void);
void save_console(int c);
void save_cline(int c, int line);
int save_control_to_file(void);
void save_colours(int* col);

//int load_int(int* num);
//int load_short(short* num);
//int load_fixed(al_fixed* num);
//int load_char(char* num);


//int bp;
//int error; // if error is 1, save_char/load_char functions won't work, and save functions will abort at some point (this is to avoid having to check return value of every single call to a save_int etc. function)

// call this when save game item selected from game system menu
// opens native file dialogue and saves file to it
// writes to mlog on failure or error
void save_game(void)
{

 if (!open_save_file("Save game", "*.sav"))
  return;

 save_state.bp = 0;
 save_state.error = 0;

 if (!save_game_to_file())
 {
  close_save_file();
  return;
 }

 if (save_state.bp != 0) // is probably something left in the buffer
  write_save_buffer();

 flush_game_event_queues(); // opening may have taken some time

 close_save_file();
 write_line_to_log("Game saved.", MLOG_COL_FILE);
 return; // success!

}

// call this as part of saving a game
//  - does not initialise bp, so it can be called after other stuff (e.g. turn file details) is written
int save_game_to_file(void)
{

 if (!save_game_struct_to_file()
  || !save_world_to_file()
  || !save_view_to_file()
  || !save_templates_to_file()
  || !save_consoles_etc_to_file()
  || !save_control_to_file())
 {
  return 0;
 }

 return 1;

}



int save_game_struct_to_file(void)
{
/*
 save_int(game.phase);
// save_int(game.type); this is derived from other values when loading
 save_int(game.turns);
 save_int(game.current_turn);
 save_int(game.minutes_each_turn);
 save_int(game.current_turn_time_left);
 save_int(game.pause_soft);
 save_int(game.pause_hard);

 if (save_state.error == 1)
  return 0;
*/
 return 1;

}

// save doesn't bounds check anything - it assumes saved values are valid. Load does all necessary checking.
int save_world_to_file(void)
{

 if (!save_world_properties_to_file())
  return 0;

 return 1;

}

int save_world_properties_to_file(void)
{
/*
 int i;

 save_int(w.players);
 save_int(w.max_procs / w.players);
// save_int(w.gen_limit);
 save_int(w.max_packets / w.players);
 save_int(w.max_clouds);
 save_int(w.w_block);
 save_int(w.h_block);
 for (i = 0; i < w.players; i ++)
 {
//  save_int(w.player[i].colour);
  save_int(w.may_change_client_template [i]);
  save_int(w.may_change_proc_templates [i]);
 }
 save_int(w.player_clients);
 save_int(w.allow_observer);
 save_int(w.player_operator);
 save_colours(w.background_colour);
 save_colours(w.hex_base_colour);

// save_int(w.system_program.active);

// when loading in again, start_world_from_world_init() is called at this point

 save_int(w.world_time);
 save_int(w.total_time);
// save_int(w.permit_operator_player);
 save_int(w.actual_operator_player);

 if (!save_procs_to_file())
  return 0;
 if (!save_groups_to_file())
  return 0;
 if (!save_packets_to_file())
  return 0;
 if (!save_clouds_to_file())
  return 0;
 if (!save_blocks_to_file())
  return 0;

 if (!save_players_to_file())
  return 0;

// struct playerstruct player [PLAYERS];  - this is saved in save_world_to_file()

 if (w.system_program.active == 0)
  save_int(0);
   else
   {
    save_int(1);
    save_program(&w.system_program);
   }
 save_int(w.system_output_console);
 save_int(w.system_error_console);

 if (w.observer_program.active == 0)
  save_int(0);
   else
   {
    save_int(1);
    save_program(&w.observer_program);
   }
 save_int(w.observer_output_console);
 save_int(w.observer_error_console);

 if (!save_markers_to_file())
  return 0;
//  int last_marker; this is reset each tick and doesn't need to be saved

 for (i = 0; i < SYSTEM_COMS; i ++)
 {
  save_short(w.system_com [i]);
 }

 save_int(w.playing_mission);

 if (save_state.error)
  return 0;

*/
 return 1;

}




int save_procs_to_file(void)
{
/*
 int i;

 for (i = 0; i < w.max_procs; i ++)
 {
  if (w.proc[i].exists == 0)
   continue; // for now, save deallocating procs (exists == -1)
  save_int(i); // save proc index
  save_proc(i);
  if (save_state.error)
   return 0;
 }

 save_int(-1); // indicates end of procs
*/
 return 1;
}

void save_proc(int p)
{
/*
 int i;

 struct proc_struct* pr = &w.proc[p];
 save_int(pr->exists);
 save_int(pr->deallocating);
 save_int(pr->player_index);
// save_int(pr->index);
// save_int(pr->index_for_client); not currently used
 for (i = 0; i < COMMANDS; i ++)
 {
  save_short(pr->command [i]);
 }
 save_int(pr->selected);
 save_int(pr->select_time);
 save_int(pr->deselected);
 save_int(pr->map_selected);
 save_int(pr->hit);
 save_int(pr->stuck);
 save_proc_pointer(pr->stuck_against);
 save_int(pr->mass);
 save_int(pr->moment);
 save_int(pr->method_mass);
 save_int(pr->hp);
 save_int(pr->hp_max);
 save_int(pr->mobile);
 save_int(pr->redundancy);
 save_int(pr->shape);
 save_int(pr->size);
// struct shape_struct* shape_str; - load function should assign pointer based on shape and size
 save_int(pr->single_proc_irpt);
 save_int(pr->single_proc_irpt_max);
 save_int(pr->irpt_gain);
 save_int(pr->irpt_gain_max);
 save_int(pr->single_proc_data);
 save_int(pr->single_proc_data_max);
// save_int(pr->irpt_gen_number);
// save_int(pr->irpt_use);
// save_int(pr->irpt_base);
 save_int(pr->base_cost);
 save_int(pr->instructions_each_execution);
 save_int(pr->instructions_left_after_last_exec);
 save_fixed(pr->x);
 save_fixed(pr->y);
// save_int(pr->x_block); derived from x/y
// save_int(pr->y_block);
 save_fixed(pr->x_speed);
 save_fixed(pr->y_speed);
 save_fixed(pr->angle);
 save_fixed(pr->spin);
 save_fixed(pr->spin_change); // is this used? not sure
 save_int(pr->hit_edge_this_cycle);
 save_fixed(pr->old_x);
 save_fixed(pr->old_y);
 save_fixed(pr->old_angle);
 save_fixed(pr->provisional_x);
 save_fixed(pr->provisional_y);
 save_fixed(pr->provisional_angle);
 save_int(pr->prov); // not sure this needs to be saved
 save_fixed(pr->max_length);
 save_fixed(pr->drag);
 save_proc_pointer(pr->blocklist_up);
 save_proc_pointer(pr->blocklist_down);
 save_int(pr->group);
 for (i = 0; i < GROUP_CONNECTIONS; i ++)
 {
  save_proc_pointer(pr->group_connection [i]);
  save_int(pr->connection_vertex [i]);
  save_int(pr->connected_from [i]);
  save_fixed(pr->connection_angle [i]);
  save_fixed(pr->connection_dist [i]);
  save_fixed(pr->connection_angle_difference [i]);
 }
// save_int(pr->number_of_group_connections);
 save_fixed(pr->group_angle);
// float float_angle; // derive this when loading (it's only used in display)
 save_fixed(pr->group_distance);
 save_int(pr->group_number_from_first);
 save_fixed(pr->group_angle_offset);
// not sure all these test values need to be saved...
 save_fixed(pr->test_group_distance);
 save_fixed(pr->test_x);
 save_fixed(pr->test_y);
 save_fixed(pr->test_angle);
// save_int(pr->test_x_block); - I don't think these are ever used without being set just before
// save_int(pr->test_y_block);
// save_int(pr->group_check); - just reset this to 0 when loading
// end test values
 save_bcode(&pr->bcode);
 save_regs(&pr->regs);
 save_methods(pr->method);
 save_int(pr->execution_count);
 save_int(pr->lifetime);
// int active_method_list_permanent [METHODS]; // this is a linked list of active methods, built on proc creation.
// int active_method_list [METHODS]; // this is a linked list of active methods currently doing something, built after each execution and modified
// int active_method_list_back [METHODS]; // backwards links corresponding to active_method_list, to make it a double linked list
 save_int(pr->listen_method);
 save_int(pr->allocate_method);
 save_int(pr->virtual_method);
 save_int(pr->give_irpt_link_method);
 save_int(pr->give_data_link_method);
// save_int(pr->special_method_penalty);
 // struct proc_struct* scanlist_down; - don't need to save (only relevant during a single scan call)
// al_fixed scan_dist; - same
 save_int(pr->scan_bitfield);
 for (i = 0; i < METHOD_VERTICES; i++)
 {
  save_int(pr->vertex_method [i]);
 }
*/
}


// assumes bc is a valid bcode_struct
void save_bcode(struct bcode_struct* bc)
{
/*
 int i;

 for (i = 0; i < bc->bcode_size; i ++)
 {
  save_short(bc->op[i]);
 }
 save_int(bc->type);
 save_int(bc->shape);
 save_int(bc->size);
// save_int(bc->printing); - should initialised this to zero when loading
 save_int(bc->from_a_file);
 for (i = 0; i < FILE_NAME_LENGTH; i ++)
 {
  save_char(bc->src_file_name [i]);
 }
 for (i = 0; i < FILE_PATH_LENGTH; i ++)
 {
  save_char(bc->src_file_path [i]);
 }
// int static_length - shouldn't need to save
*/
}



int save_groups_to_file(void)
{
/*
 int i;

 for (i = 0; i < w.max_groups; i ++)
 {
  if (w.group[i].exists == 0)
   continue;
  save_int(i); // save group index
  save_group(i);
  if (save_state.error)
   return 0;
 }

 save_int(-1); // indicates end of groups
*/
 return 1;

}

void save_group(int g)
{
/*
 struct groupstruct* gr = &w.group[g];

 save_int(gr->exists);
 save_proc_pointer(gr->first_member);
 save_int(gr->shared_irpt);
 save_int(gr->shared_irpt_max);
 save_int(gr->total_irpt_gain_max);
 save_int(gr->shared_data);
 save_int(gr->shared_data_max);
 save_int(gr->total_mass);
 save_int(gr->total_members);
 save_int(gr->moment);
 save_int(gr->mobile);
// fprintf(stdout, "\nsaving group location: %f, %f", al_fixtof(gr->x), al_fixtof(gr->y));
 save_fixed(gr->x);
 save_fixed(gr->y);
 save_fixed(gr->test_x);
 save_fixed(gr->test_y);
// save_int(gr->centre_of_mass_test_x); - no need to save these (they're only used during calculations)
// save_int(gr->centre_of_mass_test_y);
 save_fixed(gr->x_speed);
 save_fixed(gr->y_speed);
 save_fixed(gr->drag);
 save_fixed(gr->angle);
 save_fixed(gr->test_angle);
 save_fixed(gr->spin);
 save_fixed(gr->spin_change);
 save_int(gr->hit_edge_this_cycle);
 save_int(gr->stuck);
*/
}

int save_packets_to_file(void)
{
/*
 int i;

 for (i = 0; i < w.max_packets; i ++)
 {
  if (w.packet[i].exists == 0)
   continue;
  save_int(i); // save packet index
  save_packet(i);
  if (save_state.error)
   return 0;
 }

 save_int(-1); // indicates end of packets
*/
 return 1;

}

void save_packet(int p)
{
/*
 struct packet_struct* pk = &w.packet[p];
 int i;

 save_int(pk->exists);
 save_int(pk->timeout);
 save_int(pk->time);
 save_int(pk->type);
 save_int(pk->player_index);
 save_int(pk->source_proc);
 save_int(pk->source_method);
 save_int(pk->source_vertex);
 save_int(pk->prand_seed);
 save_int(pk->damage);
 save_int(pk->team_safe);
 save_int(pk->colour);
 for (i = 0; i < METHOD_EXTENSIONS; i ++)
 {
  save_int(pk->method_extensions [i]);
 }
 save_int(pk->collision_size);
 save_fixed(pk->x);
 save_fixed(pk->y);
// save_int(pk->x_block); derived from x/y
// save_int(pk->y_block);
 save_fixed(pk->x_speed);
 save_fixed(pk->y_speed);
 save_fixed(pk->angle);
 if (pk->blocklist_up == NULL)
  save_int(-1);
   else
    save_int(pk->blocklist_up->index);
 if (pk->blocklist_down == NULL)
  save_int(-1);
   else
    save_int(pk->blocklist_down->index);
// save_int(pk->tail_count);
// save_fixed(pk->tail_x);
// save_fixed(pk->tail_y);
*/
}

int save_clouds_to_file(void)
{
/*
 int i;

 for (i = 0; i < w.max_clouds; i ++)
 {
  if (w.cloud[i].exists == 0)
   continue;
  save_int(i); // save cloud index
  save_cloud(i);
  if (save_state.error)
   return 0;
 }

 save_int(-1); // indicates end of clouds
*/
 return 1;

}

void save_cloud(int c)
{
/*
 int i;
 struct cloud_struct* cl = &w.cloud[c];

 save_int(cl->exists);
 save_int(cl->timeout);
 save_int(cl->type);
 save_int(cl->colour);
 save_fixed(cl->x);
 save_fixed(cl->y);
 save_fixed(cl->x_speed);
 save_fixed(cl->y_speed);
 save_fixed(cl->angle);
 for (i = 0; i < CLOUD_DATA; i ++)
 {
  save_int(cl->data[i]);
 }
*/
}

int save_blocks_to_file(void)
{
/*
 int i, j;
 struct block_struct* bl;

 for (i = 0; i < w.w_block; i ++)
 {
  for (j = 0; j < w.h_block; j ++)
  {
   bl = &w.block[i][j];
   save_block(bl);
  }
 }

 if (save_state.error)
  return 0;
*/
 return 1;

}

void save_block(struct block_struct* bl)
{
/*
 int i;

// save_int(bl->tag); - blocktags are reinitialised when loading.
 if (bl->tag == w.blocktag)
  save_proc_pointer(bl->blocklist_down);
   else
    save_int(-1); // don't need to save blocklist if blocktag not current
 if (bl->packet_tag == w.blocktag)
 {
  if (bl->packet_down == NULL)
   save_int(-1);
    else
     save_int(bl->packet_down->index);
 }
  else
   save_int(-1);

 save_int(bl->block_type);
 for (i = 0; i < BLOCK_NODES; i++)
 {
  save_int(bl->node_x [i]);
  save_int(bl->node_y [i]);
  save_int(bl->node_size [i]);
  save_int(bl->node_team_col [i]);
  save_int(bl->node_col_saturation [i]);
  save_int(bl->node_disrupt_timestamp [i]);
  save_int(bl->node_x_base [i]);
  save_int(bl->node_y_base [i]);
 }
*/
}


int save_players_to_file(void)
{
/*
 int i;

 for (i = 0; i < w.players; i ++)
 {
//  if (w.player[i].exists == 0) shouldn't be necessary as the player array shouldn't contain any empty players except at the end
//   continue;
//  save_int(i); // save player index
  save_player(i);
  if (save_state.error)
   return 0;
 }

// save_int(-1); // indicates end of players
*/
 return 1;

}

void save_player(int p)
{
/*
 int i;
 struct playerstruct* pl = &w.player[p];

// should derive all of the following values from world information
//  int active;
//  int proc_index_start; // where this team starts in the proc array
//  int proc_index_end; // all procs on this team should be below this (the maximum is one less)
//  int template_proc_index_start; // index in templ[] array of this player's first proc template (used when creating new proc from template using PR_NEW method). Is -1 if no proc templates available.

 save_colours(pl->proc_colour_min);
 save_colours(pl->proc_colour_max);
 save_colours(pl->packet_colour_min);
 save_colours(pl->packet_colour_max);
 save_colours(pl->drive_colour_min);
 save_colours(pl->drive_colour_max);

 if (pl->client_program.active == 0)
  save_int(0);
   else
   {
    save_int(1);
    save_program(&pl->client_program);
   }
// save_int(pl->controlling_user);
 save_int(pl->score);
 save_int(pl->allocate_effect_type);

// save_int(pl->processes);
 save_int(pl->output_console);
 save_int(pl->output2_console);
 save_int(pl->error_console);
// save_int(pl->default_print_colour);

// save_int(pl->gen_number);
 for (i = 0; i < PLAYER_NAME_LENGTH; i ++)
 {
  save_char(pl->name [i]);
 }
*/
}

int save_markers_to_file(void)
{
/*
 int i;

 for (i = 0; i < MARKERS; i ++)
 {
  if (w.marker[i].active == 0)
   continue;
  save_int(i); // save marker index
  save_marker(i);
  if (save_state.error)
   return 0;
 }

 save_int(-1); // indicates end of markers
*/
 return 1;

}

void save_marker(int m)
{
/*
 struct markerstruct* mk = &w.marker[m];

 save_int(mk->active);
 save_int(mk->timeout);
 save_int(mk->type);
 save_int(mk->phase);
 save_int(mk->phase_counter);
 save_int(mk->colour);
 save_int(mk->size);
 save_int(mk->angle);
 save_int(mk->spin);
 save_fixed(mk->x);
 save_fixed(mk->y);
 save_fixed(mk->x2);
 save_fixed(mk->y2);
 save_int(mk->bind_to_proc);
 save_int(mk->bind_to_proc2);

*/
}


int save_view_to_file(void)
{
/*
 save_fixed(view.camera_x);
 save_fixed(view.camera_y);
 save_int(view.window_x);
 save_int(view.window_y);
// save_int(view.just_resized); - is always set to 1 after loading
 save_int(view.fps);
 save_int(view.cycles_per_second);
// save_int(view.paused); // not sure about this one
// save_int(view.pause_advance_pressed); // or this one
 save_proc_pointer(view.focus_proc);
 save_int(view.map_visible);
 save_int(view.map_x);
 save_int(view.map_y);
 save_int(view.map_w);
 save_int(view.map_h);
 save_fixed(view.map_proportion_x);
 save_fixed(view.map_proportion_y);
*/
 return 1;

}

int save_control_to_file(void)
{
/*
 int i;

// probably only need to save values where it is significant whether a button/key is being held, was just pressed etc
 save_int(control.mbutton_press [0]);
 save_int(control.mbutton_press [1]);

 for (i = 0; i < KEYS; i ++)
 {
  save_int(control.key_press [i]);
 }
*/
 return 1;

}


int save_templates_to_file(void)
{
/*
 int i;

 for (i = 0; i < TEMPLATES; i ++)
 {
  if (templ[i].type  == TEMPL_TYPE_CLOSED)
   continue;
  save_int(i); // save templ index
  save_template(i, 1);
  if (save_state.error)
   return 0;
 }

 save_int(-1); // indicates end of templates
*/
 return 1;

}

// t is template index.
// save_basic_template_details == 1 indicates that we're saving a saved game or gamefile, so the basic details of each template need to be stored
// save_basic_template_details == 0 indicates that we're saving a turnfile, so the basic details will come from a saved game or gamefile and don't need to be saved here.
void save_template(int t, int save_basic_template_details)
{
/*
 int i;
 struct templstruct* tp = &templ[t];

 if (save_basic_template_details == 1)
 {
  save_int(tp->type);
// fprintf(stdout, "\nSave template type %i", tp->type);
  save_int(tp->player);
// fprintf(stdout, "\nSave template player %i", tp->player);
  save_int(tp->fixed);
  save_int(tp->contents.access_index);
  save_int(tp->category_heading);
 }

 save_int(tp->contents.loaded);

 for (i = 0; i < FILE_NAME_LENGTH; i++)
 {
  save_char(tp->contents.file_name [i]);
 }
 if (save_basic_template_details == 1)
 {
  for (i = 0; i < FILE_PATH_LENGTH; i++)
  {
   save_char(tp->contents.file_path [i]);
  }
 }
 save_int(tp->contents.origin);
// save_int(tp->highlight); // don't really need to save this

 save_bcode(&tp->contents.bcode);

// int x1, y1, x2, y2; - these should be set during initialisation
*/

}





// saves consoles, channels and score boxes to file
int save_consoles_etc_to_file(void)
{
/*
 int i;

 if (save_state.error)
  return 0;

 for (i = 0; i < CONSOLES; i ++)
 {
// don't both saving closed consoles (even though they might in theory have something in them)
  if (console[i].open == CONSOLE_CLOSED)
   continue;
  save_int(i); // save console index
  save_console(i);
  if (save_state.error)
   return 0;
 }

 save_int(-1); // indicates end of consoles
*/
 return 1;

}

void save_console(int c)
{
/*
 int i;
 struct consolestruct* con = &console[c];

 save_int(con->x);
 save_int(con->y);
 save_int(con->h_lines);
 save_int(con->w_letters);
// save_int(con->h_pixels); - derived from other values
// save_int(con->w_pixels);
 save_int(con->open);
 save_int(con->printed_this_tick);
// save_int(con->line_highlight); - don't bother

 // not sure about the following:
// int button_x [CONSOLE_BUTTONS]; // x position (offset from CONSOLE_X) of buttons that open/close console
// int button_highlight; // indicates mouse is over one of the buttons (-1 if no highlight)

 save_int(con->style);
 save_int(con->font_index);
 save_int(con->colour_index);

// struct slider_struct scrollbar_v;  not sure about this.

 for (i = 0; i < CONSOLE_TITLE_LENGTH; i++)
 {
  save_char(con->title [i]);
 }

 save_int(con->cpos);
 save_int(con->window_pos);

 for (i = 0; i < CLINES; i ++)
 {
//  fprintf(stdout, "\nconsole %i cline %i used %i", c, i, con->cline[i].used);
  if (con->cline[i].used != 0)
  {
   save_int(i);
   save_cline(c, i);
  }
 }

 save_int(-1); // indicates end of clines
*/
}









void save_cline(int c, int line)
{
/*
 struct consolelinestruct* col = &console[c].cline[line];
 int i;

// save_int(col->used); // not strictly necessary
 save_int(col->colour);
 for (i = 0; i < CLINE_LENGTH; i ++)
 {
  save_char(col->text[i]);
 }
 save_int(col->source_program_type);
 save_int(col->source_index);
 save_int(col->action_type);
 save_short(col->action_val1);
 save_short(col->action_val2);
 save_short(col->action_val3);
 save_int(col->time_stamp);

// fprintf(stdout, "\nsaved console %i line %i: %s", c, line, col->text);
*/
}







// saves a proc pointer as -1 if NULL or the proc's index otherwise
void save_proc_pointer(struct proc_struct* pr)
{

 if (pr == NULL)
  save_int(-1);
   else
    save_int(pr->index);

}

void save_colours(int* col)
{
 save_int(col [0]);
 save_int(col [1]);
 save_int(col [2]);
}


// save functions write a number to the save buffer and write the save buffer to disk when it's fullish.
// they all return 0 fail/1 success

int save_int(int num)
{

// fprintf(stdout, "\nsave int: %i", num);

 if (!save_char((num >> 24) & 0xff))
  return 0;
 if (!save_char((num >> 16) & 0xff))
  return 0;
 if (!save_char((num >> 8) & 0xff))
  return 0;
 if (!save_char(num & 0xff))
  return 0;

 return 1;
}

int save_short(s16b num)
{
 if (!save_char((num >> 8) & 0xff))
  return 0;
 if (!save_char(num & 0xff))
  return 0;

 return 1;
}

int save_fixed(al_fixed num)
{
 if (!save_char((num >> 24) & 0xff))
  return 0;
 if (!save_char((num >> 16) & 0xff))
  return 0;
 if (!save_char((num >> 8) & 0xff))
  return 0;
 if (!save_char(num & 0xff))
  return 0;

 return 1;
}

int save_char(char num)
{
 if (save_state.error == 1)
  return 0;
 save_state.buffer[save_state.bp] = num;
 save_state.bp ++;
 if (save_state.bp == SAVE_BUFFER_SIZE)
 {
  if (!write_save_buffer())
   return 0;
 }
 return 1;
}



// This is used anytime we need to save a (binary) file to disk (currently used for saved games, gamefiles and turnfiles)
int open_save_file(const char* dialog_name, const char* file_extension)
{

 save_state.file_dialog = al_create_native_file_dialog("", dialog_name, file_extension,  ALLEGRO_FILECHOOSER_SAVE);

 if (save_state.file_dialog == NULL)
 {
  write_line_to_log("Error: couldn't open Allegro file dialog!", MLOG_COL_ERROR);
  return 0; // should this be an error_call()?
 }

 al_show_mouse_cursor(display);
 al_show_native_file_dialog(display, save_state.file_dialog); // this should block everything else until it finishes.
 al_hide_mouse_cursor(display);

 int files_to_open = al_get_native_file_dialog_count(save_state.file_dialog);

 if (files_to_open == 0)
 {
  goto save_fail;
 }

 if (files_to_open > 1)
 {
  write_line_to_log("Can only open one file at a time, sorry.", MLOG_COL_ERROR); // not sure this is necessary if the multiple file flag isn't set for al_create_native_file_dialog()
  goto save_fail;
 }

 const char* file_path_ptr = al_get_native_file_dialog_path(save_state.file_dialog, 0);

// fprintf(stdout, "\nFile number %i path (%s)", files_to_open, file_path_ptr);

 if (strlen(file_path_ptr) >= FILE_PATH_LENGTH) // not sure this is needed
 {
  write_line_to_log("File path too long, sorry.", MLOG_COL_ERROR);
  goto save_fail;
 }

// open the file:
 save_state.file = fopen(file_path_ptr, "wb");

 if (!save_state.file)
 {
  write_line_to_log("Error: failed to open target file.", MLOG_COL_ERROR);
  goto save_fail;
 }

// file is still open for writing. Needs to be closed when writing finished.

 al_destroy_native_file_dialog(save_state.file_dialog);
 return 1;

save_fail:
 al_destroy_native_file_dialog(save_state.file_dialog);
 return 0;

}

int write_save_buffer(void)
{

 int written = fwrite(save_state.buffer, 1, save_state.bp, save_state.file);

 if (ferror(save_state.file)
  || written != save_state.bp)
 {
//     fprintf(stdout, "\nError: buf_length %i written %i", buf_length, written);
     write_line_to_log("Error: file write failed.", MLOG_COL_ERROR);
     save_state.error = 1;
     save_state.bp = 0; // might as well
     return 0;
 }

 save_state.bp = 0;

 return 1;

}


void close_save_file(void)
{

 fclose(save_state.file);

}

