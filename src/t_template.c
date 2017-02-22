/*

This file contains code for the setup menu (which lets the user setup a world with whatever settings are wanted).

Basically it sets up interface elements that are then used by code in s_menu.c to display a menu and deal with input from it.
s_menu.c calls back here for various things.

*/

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"
#include "i_header.h"

#include "g_misc.h"

#include "c_header.h"
#include "c_compile.h"
#include "e_slider.h"
#include "e_header.h"
#include "e_editor.h"
#include "e_log.h"
#include "e_files.h"
//#include "e_build.h"
#include "e_help.h"
#include "e_inter.h"
#include "g_game.h"
//#include "g_shape.h"
//#include "g_client.h"

//#include "c_init.h"
#include "c_prepr.h"
//#include "c_comp.h"
#include "i_input.h"
#include "i_view.h"
#include "i_buttons.h"
#include "m_input.h"
#include "f_turn.h"
#include "h_story.h"

#include "t_template.h"
#include "t_files.h"

#include "g_shapes.h"
#include "p_panels.h"
#include "d_design.h"

#include "x_sound.h"

struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct editorstruct editor;
extern struct story_struct story;
extern struct identifierstruct identifier [IDENTIFIERS]; // used to display some information about e.g. core name

struct template_state_struct tstate;
extern struct object_type_struct otype [OBJECT_TYPES];
extern struct nshape_struct nshape [NSHAPES];
extern struct game_struct game;

static void add_template_object_error(struct template_struct* templ, int member_index, int object_type, int error_type);
int check_template_for_story_unlocks(struct template_struct *check_templ);


void init_all_templates(void)
{
 tstate.template_player_tab = 0;
 tstate.current_template = 0;

 int i, j;

 for (i = 0; i < PLAYERS; i ++)
	{
		for (j = 0; j < TEMPLATES_PER_PLAYER; j ++)
		{
			init_template(&templ [i] [j], i, j);
		}
	}


}


void init_template(struct template_struct* tpl, int player_index, int templ_index)
{

 tpl->player_index = player_index;
 tpl->template_index = templ_index;

// clear_template_but_not_source(tpl);

 tpl->esource_index = (player_index * TEMPLATES_PER_PLAYER) + templ_index;
 tpl->source_edit = &editor.source_edit [tpl->esource_index];
	init_source_edit_struct(templ[player_index][templ_index].source_edit);
	snprintf(tpl->menu_button_title, TEMPLATE_BUTTON_TITLE_STRING_LENGTH-1, "Player %i template %i", player_index, templ_index);

 clear_template_including_source(tpl);

}


// Clears a template that has previously been initialised.
// Used for the "delete" button in the designer.
void clear_template_including_source(struct template_struct* tpl)
{

// if (tpl->active == 0)
//		return;

 clear_template_but_not_source(tpl);

	tpl->active = 0;
	tpl->locked = 0;
	tpl->modified = 0;
	tpl->data_cost = 0;
	tpl->total_mass = 0;
	tpl->power_use_peak = 0;
	tpl->power_capacity = 0;
//	tpl->power_use_smoothed = 0;
	tpl->power_use_base = 0;

	init_source_edit_struct(tpl->source_edit);

}



// This function clears a template without affecting its source code.
// Use when initialising a template as well as when building a template from source.
void clear_template_but_not_source(struct template_struct* clear_template)
{

	int i, j;

	for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		reset_template_member(&clear_template->member[i]);
	}

 for (i = 0; i < BCODE_MAX; i ++)
	{
		clear_template->bcode.op [i] = 0;
		clear_template->bcode.src_line [i] = 0;
	}

	for (i = 0; i < OBJECT_CLASSES; i ++)
	{
  clear_template->object_class_name [i] [0] = '\0';
  clear_template->object_class_active [i] = 0;
  for (j = 0; j < OBJECT_CLASS_SIZE; j ++)
		{
   clear_template->object_class_member [i] [j] = -1;
   clear_template->object_class_object [i] [j] = -1;
		}
	}

	clear_template->name [0] = '\0';

	clear_template->data_cost = 0;
	clear_template->total_mass = 0;
	clear_template->power_capacity = 0;
	clear_template->power_use_peak = 0;
//	clear_template->power_use_smoothed = 0;
	clear_template->power_use_base = 0;
	clear_template->number_of_interface_objects = 0;
	clear_template->number_of_storage_objects = 0;
	clear_template->modified = 1;
 clear_template->mobile = 1;

 clear_template->mission_template = 0;


}

// deletes templates starting from first_template (which can be 0 if needed)
//  used for e.g. mission starts where existing templates should be cleared
void clear_remaining_templates(int player_index, int first_template)
{

	while(first_template < TEMPLATES_PER_PLAYER)
	{
  if (templ[player_index][first_template].active != 0)
	 	clear_template_including_source(&templ[player_index][first_template]);
		first_template++;
	}

}



void reset_template_member(struct template_member_struct* member)
{
	int i, j;

	member->exists = 0;
	member->shape = NSHAPE_CORE_QUAD_A;
	member->group_angle_offset = 0;
	member->connection_angle_offset = 0;
	member->position.x = 0;
	member->position.y = 0;

	for (i = 0; i < MAX_OBJECTS; i ++)
	{
		member->object [i].type = OBJECT_TYPE_NONE;
		for (j = 0; j < CLASSES_PER_OBJECT; j ++)
		{
		 member->object [i].object_class [j] = -1;
		}
	}

	for (i = 0; i < GROUP_CONNECTIONS; i ++)
	{
		member->connection[i].template_member_index = -1;
	}



}

void open_new_template(struct template_struct* tpl)
{

//	if (tpl->active == 1)
//		return; // I don't think this is correct as sometimes the template will already be active.

//	tpl->active = 1;
	tpl->locked = 0;

	clear_template_including_source(tpl);

	tpl->active = 1;
//	tpl->locked = 0;

// initialise the core:
	tpl->member[0].exists = 1;
 tpl->member[0].shape = NSHAPE_CORE_QUAD_A;
 tpl->member[0].group_angle_offset = 0;
 tpl->member[0].connection_angle_offset = 0;
 tpl->member[0].position.x = 0;
 tpl->member[0].position.y = 0;

 tpl->source_edit->active = 1;
 tpl->modified = 0;

 open_template(tpl->player_index, tpl->template_index);

// open_template_in_editor(tpl);

}


void template_panel_button(int element)
{

	if (element >= FPE_TEMPLATES_TAB_P0
		&& element <= FPE_TEMPLATES_TAB_P3)
	{
		if (tstate.template_player_tab != element - FPE_TEMPLATES_TAB_P0)
		{
			int player_tab = element - FPE_TEMPLATES_TAB_P0;
			if (player_tab < w.players)
			{
    play_interface_sound(SAMPLE_BLIP4, TONE_2A);
    open_template(player_tab, 0);
			}
		}
  return;
	}

	if (element == FPE_TEMPLATES_FILE_LOAD)
	{
  play_interface_sound(SAMPLE_BLIP4, TONE_3C);
  load_template_file(tstate.template_player_tab);
		return;
	}

	if (element == FPE_TEMPLATES_FILE_SAVE)
	{
  play_interface_sound(SAMPLE_BLIP4, TONE_3C);
  save_template_file(tstate.template_player_tab);
		return;
	}

	if (element >= FPE_TEMPLATES_TEMPL_0) // currently nothing is after this element
	{
  play_interface_sound(SAMPLE_BLIP4, TONE_3DS);
		open_template(tstate.template_player_tab, element - FPE_TEMPLATES_TEMPL_0);
/*		if (tstate.current_template != element - FPE_TEMPLATES_TEMPL_0)
		{
		 tstate.current_template = element - FPE_TEMPLATES_TEMPL_0;
		 open_template_in_editor(&templ[tstate.template_player_tab] [tstate.current_template]); // tstate.template_player_tab, tstate.current_template);
 		open_template_in_designer(&templ[tstate.template_player_tab] [tstate.current_template]); // (tstate.template_player_tab, tstate.current_template);
		}*/
		return;
	}

}


void open_template(int player_index, int template_index)
{

	  tstate.template_player_tab = player_index;
		 tstate.current_template = template_index;
		 open_template_in_editor(&templ[tstate.template_player_tab] [tstate.current_template]);
 		open_template_in_designer(&templ[tstate.template_player_tab] [tstate.current_template]);

}



// setting copy_design to 0 means the design isn't copied, e.g. for use in compiling a locked template
void copy_template(struct template_struct* target_templ, struct template_struct* source_templ, int copy_design)
{

	int i,j;

	target_templ->active = source_templ->active;
	target_templ->locked = source_templ->locked; // not sure this is right
//	target_templ->player_index = source_templ->player_index; *** no, don't copy this!
//	target_templ->template_index = source_templ->template_index; or this

 if (copy_design)
	{
	 target_templ->data_cost = source_templ->data_cost;
	 target_templ->total_mass = source_templ->total_mass;
	 target_templ->build_cooldown_cycles = source_templ->build_cooldown_cycles;

	 target_templ->power_capacity = source_templ->power_capacity;
	 target_templ->power_use_peak = source_templ->power_use_peak;
//	 target_templ->power_use_smoothed = source_templ->power_use_smoothed;
	 target_templ->power_use_base = source_templ->power_use_base;

	 for (i = 0; i < OBJECT_CLASSES; i ++)
	 {
	 	target_templ->object_class_active [i] = source_templ->object_class_active [i];
	 	for (j = 0; j < CLASS_NAME_LENGTH; j ++)
	 	{
	 		strcpy(target_templ->object_class_name [i], source_templ->object_class_name [i]);
	 	}
	 }

	 for (i = 0; i < OBJECT_CLASSES; i ++)
	 {
		 for (j = 0; j < OBJECT_CLASS_SIZE; j ++)
		 {
		 	target_templ->object_class_member [i] [j] = source_templ->object_class_member [i] [j];
		 	target_templ->object_class_object [i] [j] = source_templ->object_class_object [i] [j];
		 }
	 }

	 for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	 {
	 	target_templ->member [i] = source_templ->member [i];
// this operation involves numerous sub-structs, so need to be careful! There are comments in the relevant sub-struct definitions in g_header.h referring to this function.
// among other things it copies the object structures
	 }

  target_templ->first_build_object_member = source_templ->first_build_object_member;
  target_templ->first_build_object_link = source_templ->first_build_object_link;


  target_templ->first_repair_object_member = source_templ->first_repair_object_member;
  target_templ->first_repair_object_link = source_templ->first_repair_object_link;
	}

  target_templ->has_allocator = source_templ->has_allocator;
 	target_templ->number_of_interface_objects = source_templ->number_of_interface_objects; // not sure this is correct
 	target_templ->number_of_storage_objects = source_templ->number_of_storage_objects;

//	target_templ->esource_index = source_templ->esource_index; - shouldn't copy this
//	target_templ->source_edit = source_templ->source_edit;  - this is not copied! (and it's a pointer anyway)
	target_templ->bcode = source_templ->bcode;
	target_templ->modified = source_templ->modified; // not sure this is correct
	target_templ->mobile = source_templ->mobile;

// menu_button_title is not copied.
// char menu_button_title [TEMPLATE_BUTTON_TITLE_STRING_LENGTH];

 strcpy(target_templ->name, source_templ->name);


}


void calculate_template_cost_and_power(struct template_struct* costed_templ)
{
	int mem, i;

	costed_templ->data_cost = 0;
	costed_templ->power_capacity = nshape[costed_templ->member[0].shape].power_capacity;
	costed_templ->power_use_peak = 0;
//	costed_templ->power_use_smoothed = 0;
	costed_templ->power_use_base = 0;
	costed_templ->number_of_interface_objects = 0;
	costed_templ->number_of_storage_objects = 0;

#define DATA_COST_TO_INERTIA_MULTIPLIER 1

	for (mem = 0; mem < GROUP_MAX_MEMBERS; mem++)
	{
		if (costed_templ->member[mem].exists)
		{
			calculate_template_member_cost(costed_templ, mem);
			costed_templ->member[mem].mass = costed_templ->member[mem].data_cost * DATA_COST_TO_INERTIA_MULTIPLIER;
			costed_templ->data_cost += costed_templ->member[mem].data_cost;
  	if (mem != 0)
  	 costed_templ->power_capacity += nshape[costed_templ->member[0].shape].component_power_capacity;
	  for (i = 0; i < nshape[costed_templ->member[mem].shape].links; i ++)
  	{
  	 costed_templ->power_use_peak += otype[costed_templ->member[mem].object[i].type].power_use_peak;
//  	 costed_templ->power_use_smoothed += otype[costed_templ->member[mem].object[i].type].power_use_smoothed;
  	 costed_templ->power_use_base += otype[costed_templ->member[mem].object[i].type].power_use_base;
  	 switch(costed_templ->member[mem].object[i].type)
  	 {
				 case OBJECT_TYPE_INTERFACE:
					 costed_templ->number_of_interface_objects ++; break;
				 case OBJECT_TYPE_STORAGE:
					 costed_templ->number_of_storage_objects ++; break;
  	 }
  	}
		}
	}

	costed_templ->total_mass = costed_templ->data_cost * DATA_COST_TO_INERTIA_MULTIPLIER; // for now these are directly linked


}

void calculate_template_member_cost(struct template_struct* tpl, int member_index)
{

	tpl->member[member_index].data_cost = nshape[tpl->member[member_index].shape].data_cost;

	int i;

	for (i = 0; i < nshape[tpl->member[member_index].shape].links; i ++)
	{
		tpl->member[member_index].data_cost += otype[tpl->member[member_index].object[i].type].data_cost;
	}

}






void lock_template_members_recursively(struct template_struct* lock_templ, int member_index, int downlink_level);

// Call this to lock a template so that it can be used to create procs.
int lock_template(struct template_struct* lock_templ)
{

 if (lock_templ->locked)
		return 1; // should this fail or print an error message? not sure.

// first build from source:
 if (!compile(lock_templ, lock_templ->source_edit, COMPILE_MODE_LOCK))
	{
 	write_line_to_log("Failed to lock template.", MLOG_COL_ERROR);
  return 0;
	}
//fpr("\n A gt %i (%i) pi %i check %i", game.type, GAME_TYPE_MISSION, lock_templ->player_index, check_template_for_story_unlocks(lock_templ));

// In story mode, make sure that the template has no objects or components that are unavailable:
 if (game.type == GAME_TYPE_MISSION
		&& lock_templ->player_index == 0
		&& !check_template_for_story_unlocks(lock_templ))
	{
// check_template_for_story_unlocks write more detailed messages
 	write_line_to_log("Failed to lock template.", MLOG_COL_ERROR);
  return 0;
	}

	lock_template_members_recursively(lock_templ, 0, 0);

	lock_templ->locked = 1;
	write_line_to_log("Template locked.", MLOG_COL_TEMPLATE);
/*
	if (w.allocated // this check isn't strictly needed, but it's best not to access values in the w struct if !allocated
		&& w.local_condition == LOCAL_CONDITION_STATIC
		&& lock_templ->member[0].shape < FIRST_MOBILE_NSHAPE)
	{
		lock_templ->data_cost = (lock_templ->data_cost + 1) / 2; // + 1 is to make it round up
	}*/

	return 1;

}

int check_template_for_story_unlocks(struct template_struct *check_templ)
{
	int member_index, object_index;
	int return_value = 1;

	for (member_index = 0; member_index < GROUP_MAX_MEMBERS; member_index ++)
	{
		if (!check_templ->member[member_index].exists)
			continue;
		if (!story.unlock[nshape[check_templ->member[member_index].shape].unlock_index])
		{
  	start_log_line(MLOG_COL_ERROR);
  	write_to_log("You haven't unlocked ");
  	write_to_log(identifier[nshape[check_templ->member[member_index].shape].keyword_index].name); // can I use the identifier array like this??
  	write_to_log(" (component ");
  	write_number_to_log(member_index);
  	write_to_log(").");
  	finish_log_line();
			return_value = 0;
			check_templ->member[member_index].story_lock_failure = 1;
		}
		for (object_index = 0; object_index < nshape[check_templ->member[member_index].shape].links; object_index ++)
		{
		 if (!story.unlock[otype[check_templ->member[member_index].object[object_index].type].unlock_index])
		 {
  	 start_log_line(MLOG_COL_ERROR);
  	 write_to_log("You haven't unlocked ");
  	 write_to_log(otype[check_templ->member[member_index].object[object_index].type].name); // can I use the identifier array like this??
  	 write_to_log(" (component ");
  	 write_number_to_log(member_index);
  	 write_to_log(" object ");
  	 write_number_to_log(object_index);
  	 write_to_log(").");
  	 finish_log_line();
  	 check_templ->member[member_index].object[object_index].template_error = TEMPLATE_OBJECT_ERROR_STORY_LOCK;
 			return_value = 0;
		 }
		}
	}

 return return_value;

}


// this function does anything needed to lock template members after it has been verified that the template is valid
// it doesn't do too much currently.
void lock_template_members_recursively(struct template_struct* lock_templ, int member_index, int downlink_level)
{

	lock_templ->member[member_index].downlinks_from_core = downlink_level;
 calculate_template_member_cost(lock_templ, member_index);

	int i;

	for (i = 1; i < MAX_LINKS; i ++) // note - starts at 1 to avoid uplinks
	{
		if (lock_templ->member[member_index].connection[i].template_member_index != -1)
			lock_template_members_recursively(lock_templ, lock_templ->member[member_index].connection[i].template_member_index, downlink_level + 1);
	}


}


// Call this when "unlock template" button is clicked.
// It checks whether any processes based on this template are in world (which prevents unlocking)
void unlock_template(int player_index, int template_index)
{

// First, make sure that the player isn't trying to unlock the mission AI:
//#ifndef RECORDING_VIDEO
// (but allow this in debug mode)
// if (templ[player_index][template_index].mission_template)
 if (templ[player_index][template_index].player_index == 1
		&& game.type == GAME_TYPE_MISSION)
	{
#ifndef DEBUG_MODE
		write_line_to_log("You can't unlock a mission opponent's template.", MLOG_COL_ERROR);
		return;
#else
		write_line_to_log("DEBUG MODE: bypassing mission opponent template unlock check.", MLOG_COL_WARNING);
#endif
	}


// if no world is allocated, easy to unlock
	if (!w.allocated)
	{
		templ[player_index][template_index].locked = 0;
		write_line_to_log("Template unlocked.", MLOG_COL_TEMPLATE);
		return;
	}
/*
	if (game.type == GAME_TYPE_MISSION
		&& player_index != 0)
	{
		write_line_to_log("Can't unlock enemy templates during mission!", MLOG_COL_ERROR);
		return;
	}
 - not sure I should really prevent this, as the player may just want to experiment.
*/

	int c;
	int deallocating = 0;

	for (c = 0; c < w.max_cores; c ++)
	{
		if (w.core[c].player_index != player_index // actually could test this by c index, but let's be safer
			|| w.core[c].template_index != template_index)
			continue;
		if (w.core[c].exists)
		{
		 write_line_to_log("Can't unlock template while processes based on it still exist.", MLOG_COL_ERROR);
		 return;
		}
		if (w.core[c].destroyed_timestamp >= w.world_time - DEALLOCATE_COUNTER)
		 deallocating = 1;
	}

// Tell user specially if the only reason the template can't be unlocked is that a process of this type is deallocating,
//  because this won't be obvious.
	if (deallocating)
	{
		 write_line_to_log("Can't unlock template while a process based on it still exist.", MLOG_COL_ERROR);
		 write_line_to_log(" (the process has been destroyed, but deallocation will take a few ticks)", MLOG_COL_ERROR);
		 return;
	}

	templ[player_index][template_index].locked = 0;
	write_line_to_log("Template unlocked.", MLOG_COL_TEMPLATE);

}


void prepare_templates_for_new_game(void)
{
// I'm not really sure how to prepare templates for a new game.
// For missions, player 0's templates should probably be left as they are between games.
// But they should at least be unlocked.
// Player 1's templates should be cleared, but that can be done in mission-specific s_mission.c code.
// For custom games, both players' templates should probably be left as they are (but unlocked)
//  and cleared only if load template file is selected.

 int i, j;

 for (i = 0; i < PLAYERS; i ++)
	{
		for (j = 0; j < TEMPLATES_PER_PLAYER; j ++)
		{
			templ[i][j].locked = 0;

		}
	}


}



// checks for problems with a template member's objects. is called by a range of different functions (compilation, design etc)
// only checks a single component
// doesn't check move object obstruction!
// doesn't check for problems with the process as a whole, like interface objects without depth objects (see check_template_objects() in c_compile.c for that)
// returns 1 if error found, 0 otherwise
int check_template_member_objects(struct template_struct* check_templ, int member_index)
{

	int i;
	int error_found = 0;

	int mobile = 1;

	if (check_templ->member[0].shape < FIRST_MOBILE_NSHAPE)
		mobile = 0;

	int member_has_object [OBJECT_TYPES];

	for (i = 0; i < OBJECT_TYPES; i ++)
	{
		member_has_object [i] = 0;
	}

	for (i = 0; i < nshape[check_templ->member[member_index].shape].links; i ++)
	{
		member_has_object [check_templ->member[member_index].object[i].type] ++;
// clear errors except for obstructed move object errors, which are dealt with elsewhere (don't worry though if an obstructed move object error is overwritten by another kind of error)
		if (check_templ->member[member_index].object[i].template_error != TEMPLATE_OBJECT_ERROR_MOVE_OBSTRUCTED)
			check_templ->member[member_index].object[i].template_error = TEMPLATE_OBJECT_ERROR_NONE;
	}
/*
 if (member_index == 0
		&& member_has_object [OBJECT_TYPE_INTERFACE])
	{
    write_line_to_log("Error: core can't have interface objects.", MLOG_COL_ERROR);
    add_template_object_error(check_templ, member_index, OBJECT_TYPE_INTERFACE, TEMPLATE_OBJECT_ERROR_INTERFACE_CORE);
				error_found = 1;
	}*/

	if (mobile == 0
		&& member_has_object [OBJECT_TYPE_MOVE])
	{
    write_line_to_log("Error: static process can't have move objects.", MLOG_COL_ERROR);
    add_template_object_error(check_templ, member_index, OBJECT_TYPE_MOVE, TEMPLATE_OBJECT_ERROR_STATIC_MOVE);
				error_found = 1;
	}

	if (mobile
	 && member_has_object [OBJECT_TYPE_ALLOCATE])
	{
    write_line_to_log("Error: only a static process can have an allocate object.", MLOG_COL_ERROR);
    add_template_object_error(check_templ, member_index, OBJECT_TYPE_ALLOCATE, TEMPLATE_OBJECT_ERROR_MOBILE_ALLOCATE);
				error_found = 1;
	}

/*
	if (member_has_object [OBJECT_TYPE_MOVE]
		&& member_has_object [OBJECT_TYPE_INTERFACE])
	{
    write_line_to_log("Error: move and interface objects on same component.", MLOG_COL_ERROR);
    add_template_object_error(check_templ, member_index, OBJECT_TYPE_MOVE, TEMPLATE_OBJECT_ERROR_MOVE_INTERFACE);
    add_template_object_error(check_templ, member_index, OBJECT_TYPE_INTERFACE, TEMPLATE_OBJECT_ERROR_MOVE_INTERFACE);
				error_found = 1;
	}
*/

	return error_found;

}


static void add_template_object_error(struct template_struct* error_templ, int member_index, int object_type, int error_type)
{
	int i;

	for (i = 0; i < nshape[error_templ->member[member_index].shape].links; i ++)
	{
		if (error_templ->member[member_index].object[i].type == object_type)
			error_templ->member[member_index].object[i].template_error = error_type;
	}

}
