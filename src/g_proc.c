#include <allegro5/allegro.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "m_config.h"
#include "m_globvars.h"

#include "g_misc.h"
#include "g_header.h"
#include "g_proc.h"

#include "g_group.h"
#include "g_motion.h"
#include "g_method.h"
#include "g_command.h"
#include "g_cloud.h"
#include "g_method_misc.h"
#include "g_method_clob.h"
#include "g_method_std.h"
#include "g_proc_new.h"
#include "g_world.h"
#include "g_shapes.h"
#include "h_story.h"
#include "i_error.h"
#include "t_template.h"
#include "x_sound.h"


#include "m_maths.h"


extern struct command_struct command;
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct nshape_struct nshape [NSHAPES];
extern struct view_struct view;

void hurt_proc(int p, int damage, int cause_team);


static void noncore_proc_explodes(struct proc_struct* destroyed_pr, int destroyer_team);
static void explosion_fragments(al_fixed x, al_fixed y, al_fixed x_speed, al_fixed y_speed, int fragments, int player_index);

void reset_group_after_composition_change(struct core_struct* core);
static void destroy_procs_recursively(struct core_struct* core, struct proc_struct* destroyed_pr, int destroyer_team);
static void destroy_a_proc(struct proc_struct* destroyed_pr, int destroyer_team);
void shake_screen(al_fixed x, al_fixed y, int shaken);

// wrapper around hurt_proc for cases where the damage is caused by something that can be caught by virtual interface.
// damage sources that are not caught by virtual interface should call hurt_proc directly
void apply_packet_damage_to_proc(struct proc_struct* pr, int damage, int cause_team, int cause_core_index, timestamp cause_core_timestamp)
{

		struct core_struct* core = &w.core[pr->core_index];

		core->damage_source_core_index = cause_core_index;
		core->damage_source_core_timestamp = cause_core_timestamp;
		core->damage_this_cycle += damage;

// if (pr->interface_object_present
//	 && pr->interface_on_process_set_on) // checks for core->interface_active just below
 if (pr->interface_protects) // checks for core->interface_active just below
	{

		if (core->interface_active)
		{
   if (pr->interface_stability)
    damage /= 2;
   core->interface_strength -= damage;
			if (core->interface_strength > 0)
			{
				pr->interface_hit_time = w.world_time;
				return;
			}
// interface broken:
//   fpr("\n broken interface core %i", core->index);
   core->interface_active = 0;
   core->interface_broken_time = w.world_time;
		 play_game_sound(SAMPLE_INT_BREAK, TONE_1G, 140, 1, pr->position.x, pr->position.y);
   int i;
   for (i = 0; i < core->group_members_max; i ++)
			{
				if (core->group_member[i].exists
				 &&	w.proc[core->group_member[i].index].interface_protects)
//				 &&	w.proc[core->group_member[i].index].interface_object_present
//				 && w.proc[core->group_member[i].index].interface_on_process_set_on)
				{
     struct cloud_struct* cl = new_cloud(CLOUD_INTERFACE_BREAK, 32, w.proc[core->group_member[i].index].position.x, w.proc[core->group_member[i].index].position.y);
     if (cl != NULL)
     {
      cl->angle = w.proc[core->group_member[i].index].angle;
      cl->colour = w.proc[core->group_member[i].index].player_index;
      cl->data [0] = w.proc[core->group_member[i].index].shape;
      cl->speed.x = 0;
      cl->speed.y = 0;
      cl->display_size_x1 = -200;
      cl->display_size_y1 = -200;
      cl->display_size_x2 = 200;
      cl->display_size_y2 = 200;
     }
				}
			}
/*
   struct cloud_struct* cl = new_cloud(CLOUD_INTERFACE_BREAK, 32, pr->position.x, pr->position.y);
   if (cl != NULL)
   {
    cl->angle = pr->angle;
    cl->colour = pr->player_index;
    cl->data [0] = pr->shape;
    cl->speed.x = 0;
    cl->speed.y = 0;
    cl->display_size_x1 = -200;
    cl->display_size_y1 = -200;
    cl->display_size_x2 = 200;
    cl->display_size_y2 = 200;
   }

*/
// after interface is broken strength may be -ve, in which case the interface may need to recharge for a while.
   return;
		}

	}

/*
 if (pr->virtual_method != -1
  && pr->method[pr->virtual_method].data [MDATA_PR_VIRTUAL_STATE] > 0)
 {
  if (damage >= pr->method[pr->virtual_method].data [MDATA_PR_VIRTUAL_STATE])
  {
   damage -= pr->method[pr->virtual_method].data [MDATA_PR_VIRTUAL_STATE];

   virtual_method_break(pr);

  }
   else
   {
    pr->method[pr->virtual_method].data [MDATA_PR_VIRTUAL_STATE] -= damage;
    pr->method[pr->virtual_method].data [MDATA_PR_VIRTUAL_PULSE] += (damage / 2);
    if (pr->method[pr->virtual_method].data [MDATA_PR_VIRTUAL_PULSE] > VIRTUAL_METHOD_PULSE_MAX)
     pr->method[pr->virtual_method].data [MDATA_PR_VIRTUAL_PULSE] = VIRTUAL_METHOD_PULSE_MAX;
    return;
   }
 }
*/
 if (damage <= 0)
  return;

 pr->hit_pulse_time = w.world_time;

 hurt_proc(pr->index, damage, cause_team);

}





// cause_team is the player index of the player that caused the damage (e.g. owner of packet). Can be proc's own team if self-inflicted.
// currently this function does not set the damage recording values for the damaged core. The function that calls this one should do so.
void hurt_proc(int p, int damage, int cause_team)
{
// if (cause_team != w.proc[p].player_index)
//		return;
// if (w.proc[p].group_member_index == 0)
//		return;
// damage /= 2;
 w.proc[p].hp -= damage;
 w.core[w.proc[p].core_index].group_total_hp -= damage;
 if (w.proc[p].hp <= 0)
 {
 	if (w.proc[p].group_member_index == 0) // is a core
			core_proc_explodes(&w.proc[p], cause_team);
			 else
     noncore_proc_explodes(&w.proc[p], cause_team);
//  destroy_proc(&w.proc[p]);
 }

}


int procs_destroyed_in_this_explosion; // used to work out size of explosion. Not used for core explosions because group size is known.

// This function destroys an entire process from the core.
// It then destroys group members using the group_member array rather than recursively through connections,
//  because the fact that the group no longer exists means that some steps involved in removing a proc from a group can be ignored.
void core_proc_explodes(struct proc_struct* core_pr, int destroyer_team)
{

 struct core_struct* core = &w.core[core_pr->core_index];

 struct cloud_struct* cl = new_cloud(CLOUD_MAIN_PROC_EXPLODE, 64, core_pr->position.x, core_pr->position.y);
 if (cl != NULL)
 {
  cl->angle = core_pr->angle;
  cl->colour = core_pr->player_index;
  cl->data [0] = core_pr->shape;
  cl->data [1] = core->group_members_current;
  cl->speed.x = 0;
  cl->speed.y = 0;
  cl->display_size_x1 = -300;
  cl->display_size_y1 = -300;
  cl->display_size_x2 = 300;
  cl->display_size_y2 = 300;
 }
 explosion_fragments(core_pr->position.x, core_pr->position.y, core_pr->speed.x, core_pr->speed.y, 6, core_pr->player_index);
 explosion_affects_block_nodes(core_pr->position.x, core_pr->position.y, 200 + (core->group_members_current * 20), core_pr->player_index);

	int i;

 remove_core_from_selection(core_pr->core_index);

 if (templ[core->player_index][core->template_index].first_build_object_member != -1)
		clear_build_queue_for_core(core->player_index, core->index);

	if (core->special_AI_type != 0)
		special_AI_destroyed(core); // this may create a bubble that will be turned into a cloud below.

	core->exists = 0;
	core->destroyed_timestamp = w.world_time;

	if (core->bubble_text_time > w.world_time - BUBBLE_TOTAL_TIME)
	{
// if the core had a bubble, it stays on as a cloud (and refers to bubble data in the core's core_struct,
//  which should still be usable because the core will be deallocating)
  struct cloud_struct* bubble_cl = new_cloud(CLOUD_BUBBLE_TEXT, BUBBLE_TOTAL_TIME, core_pr->position.x, core_pr->position.y);

  if (bubble_cl != NULL)
  {
  	bubble_cl->created_timestamp = core->bubble_text_time;
   bubble_cl->destruction_timestamp = bubble_cl->created_timestamp + BUBBLE_TOTAL_TIME;
   bubble_cl->data [0] = core->index;
   bubble_cl->display_size_x1 = -300;
   bubble_cl->display_size_y1 = -40;
   bubble_cl->display_size_x2 = 300;
   bubble_cl->display_size_y2 = 40;
  }
	}

	destroy_a_proc(&w.proc[core->process_index], destroyer_team);
 w.proc[core->process_index].reserved = 0;

	for (i = 1; i < core->group_members_max; i++) // note for i = 1
	{
		sancheck(core->group_member[i].index, 0, w.max_procs, "core_proc_explodes: core->group_member[i].index");
		w.proc[core->group_member[i].index].reserved = 0; // core has been destroyed, so proc no longer reserved
		if (core->group_member[i].exists)
		{
    cl = new_cloud(CLOUD_SUB_PROC_EXPLODE, 64, w.proc[core->group_member[i].index].position.x, w.proc[core->group_member[i].index].position.y);
    if (cl != NULL)
	   {
     cl->angle = w.proc[core->group_member[i].index].angle;
     cl->colour = core->player_index;
     cl->data [0] = w.proc[core->group_member[i].index].shape;
     cl->data [1] = 1;
     cl->speed.x = 0;
     cl->speed.y = 0;
     cl->display_size_x1 = -200;
     cl->display_size_y1 = -200;
     cl->display_size_x2 = 200;
     cl->display_size_y2 = 200;
	   }
   procs_destroyed_in_this_explosion ++;
   explosion_fragments(w.proc[core->group_member[i].index].position.x, w.proc[core->group_member[i].index].position.y, w.proc[core->group_member[i].index].speed.x, w.proc[core->group_member[i].index].speed.y, 6, core->player_index);
			destroy_a_proc(&w.proc[core->group_member[i].index], destroyer_team);
		}
	}

// play_game_sound(SAMPLE_BANG, TONE_2C - (procs_destroyed_in_this_explosion / 3), 100, 10, core->core_position.x, core->core_position.y);
 int bang_sample = SAMPLE_BANG;
 if (procs_destroyed_in_this_explosion > 1)
		bang_sample = SAMPLE_BANG2;
 play_game_sound(bang_sample, TONE_2C - (procs_destroyed_in_this_explosion / 3), 50, 10, core->core_position.x, core->core_position.y);

 shake_screen(core->core_position.x, core->core_position.y, 16 + procs_destroyed_in_this_explosion);

}

void shake_screen(al_fixed x, al_fixed y, int shaken)
{

//if (game.fast_forward != FAST_FORWARD_OFF
 if (abs(x - view.camera_x) > (view.centre_x_zoomed + al_itofix(100))
  || abs(y - view.camera_y) > (view.centre_y_zoomed + al_itofix(100)))
//		|| (game.vision_mask
//			&& w.vision_area[game.user_player_index]
//			                [fixed_to_block(x)]
//									          [fixed_to_block(y)].vision_time < w.world_time - VISION_AREA_VISIBLE_TIME))
   return;


 if (view.screen_shake_time < w.world_time + shaken)
  view.screen_shake_time = w.world_time + shaken;


}
// This function destroys a non-core process and everything downlinked from it
static void noncore_proc_explodes(struct proc_struct* destroyed_pr, int destroyer_team)
{
/*

 struct cloud_struct* cl = new_cloud(CLOUD_PROC_EXPLODE, 32, destroyed_pr->position.x, destroyed_pr->position.y);
 if (cl != NULL)
	{
  cl->angle = destroyed_pr->angle;
  cl->colour = destroyer_team;
  cl->data [0] = destroyed_pr->shape;
  cl->speed.x = 0;
  cl->speed.y = 0;
	}*/


 struct proc_struct* parent_pr = destroyed_pr->group_connection_ptr [0];
 int parent_connection_index = destroyed_pr->connected_from [0];
// int parent_downlink_index = destroyed_pr->connected_from_link [0];

// parent_pr->group_connection [parent_connection_index] = NULL;
 parent_pr->group_connection_exists [parent_connection_index] = 0;
	procs_destroyed_in_this_explosion = w.core[destroyed_pr->core_index].group_members_current; // subtracted from below

	int i;

	for (i = 1; i < GROUP_CONNECTIONS; i++) // Note i starts at 1 (don't destroy uplink)
	{
		if (destroyed_pr->group_connection_exists [i])
		{
   destroy_procs_recursively(&w.core[destroyed_pr->core_index], destroyed_pr->group_connection_ptr[i], destroyer_team);
		}
	}

 w.core[destroyed_pr->core_index].group_member[destroyed_pr->group_member_index].exists = 0;
// core->group_members--; this is dealt with by set_basic_group_properties()
 destroy_a_proc(destroyed_pr, destroyer_team);

 play_game_sound(SAMPLE_BANG, TONE_2C, 60, 10, destroyed_pr->position.x, destroyed_pr->position.y);

 reset_group_after_composition_change(&w.core[destroyed_pr->core_index]);

 procs_destroyed_in_this_explosion -= w.core[destroyed_pr->core_index].group_members_current;

// fpr("\n procs_destroyed_in_this_explosion %i", procs_destroyed_in_this_explosion);

 struct cloud_struct* cl = new_cloud(CLOUD_SUB_PROC_EXPLODE, 64, destroyed_pr->position.x, destroyed_pr->position.y);
 if (cl != NULL)
 {
  cl->angle = destroyed_pr->angle;
  cl->colour = destroyed_pr->player_index; //destroyer_team;
  cl->data [0] = destroyed_pr->shape;
  cl->data [1] = procs_destroyed_in_this_explosion;
  cl->speed.x = 0;
  cl->speed.y = 0;
  cl->display_size_x1 = -200;
  cl->display_size_y1 = -200;
  cl->display_size_x2 = 200;
  cl->display_size_y2 = 200;
 }
 explosion_fragments(destroyed_pr->position.x, destroyed_pr->position.y, destroyed_pr->speed.x, destroyed_pr->speed.y, 6, destroyed_pr->player_index);
 explosion_affects_block_nodes(destroyed_pr->position.x, destroyed_pr->position.y, 200 + (procs_destroyed_in_this_explosion * 20), destroyed_pr->player_index);



 shake_screen(destroyed_pr->position.x, destroyed_pr->position.y, 6 + procs_destroyed_in_this_explosion);


}

// the core may be the only component of the group (if all sub-processes have been destroyed)
void reset_group_after_composition_change(struct core_struct* core)
{

 set_basic_group_properties(core);

 set_group_object_properties(core);

}

// call this at proc creation
// or when proc's composition changes (component destroyed etc)
void set_group_object_properties(struct core_struct* core)
{

// first reset some values that will be reset below if certain objects are present:
 core->interface_available = 0;
 core->interface_strength_max = 0;
// core->interface_charge_rate = INTERFACE_CHARGE_RATE_BASE;
 core->data_storage_capacity = 0;
 core->number_of_build_objects = 0;
 core->number_of_repair_objects = 0;
 core->has_repair_other_object = 0;
 core->number_of_interface_objects = 0;
 core->interface_max_charge_rate = 0;
 core->number_of_harvest_objects = 0;
 core->scan_bitfield = core->scan_bitfield_immutable; // scan_bitfield_immutable contains basic stuff that can't be changed by removing components

 int i, j;

 for (i = 0; i < core->group_members_max; i++)
	{
		if (core->group_member[i].exists == 0)
			continue;
		for (j = 0; j < w.proc[core->group_member[i].index].nshape_ptr->links; j++)
		{
			switch(w.proc[core->group_member[i].index].object[j].type)
			{
				case OBJECT_TYPE_MOVE:
				 calculate_move_object_properties(core, &w.proc[core->group_member[i].index], j);
				 break;
				case OBJECT_TYPE_INTERFACE:
					core->interface_available = 1; // can be set to 0 below if there are not interface_depth objects
//					core->interface_strength_max += 132;
//					w.proc[core->group_member[i].index].interface_available = 1;
// probably needs to be done each cycle too (to take account of possibility of being switched off) - although it might be best not to allow switch-off for individual procs
// note that some details of interface objects are initialised only at proc creation (see init_group_object_properties() in g_proc_new.c)
//					break;
//				case OBJECT_TYPE_INTERFACE_DEPTH:
//					core->interface_available = 1;
/*     if (w.local_condition == LOCAL_CONDITION_THIN_INTERFACE)
 					core->interface_strength_max += 32;
 					 else*/
							{
					   core->interface_strength_max += nshape[w.proc[core->process_index].shape].base_hp_max; // uses shape, rather than proc, hp_max to bypass fragile proc local condition
							}
					core->number_of_interface_objects ++;
					core->interface_max_charge_rate += nshape[w.proc[core->process_index].shape].interface_charge_rate;
					break;
//				case OBJECT_TYPE_INTERFACE_STABILITY:
//					w.proc[core->group_member[i].index].interface_stability
//					break;
//				case OBJECT_TYPE_INTERFACE_RESPONSE:
//					core->interface_charge_rate += INTERFACE_CHARGE_RATE_RESPONSE;
//					break;
				case OBJECT_TYPE_STORAGE:
					core->data_storage_capacity += 64;
					break;
				case OBJECT_TYPE_BUILD:
				 core->number_of_build_objects ++;
					core->scan_bitfield |= (1 << SCAN_BITFIELD_OBJ_BUILD);
// note: linked list for similar objects is set in the template object structures (which are copied to the core structures)
				 break;
				case OBJECT_TYPE_REPAIR_OTHER:
					core->scan_bitfield |= (1 << SCAN_BITFIELD_OBJ_REPAIR_OTHER);
				 core->has_repair_other_object = 1;
// fall through
				case OBJECT_TYPE_REPAIR:
				 core->number_of_repair_objects ++;
//					core->scan_bitfield |= SCAN_BITFIELD_OBJ_BUILD;
				 break;
				case OBJECT_TYPE_ALLOCATE:
					core->scan_bitfield |= (1 << SCAN_BITFIELD_OBJ_ALLOCATE);
					break;
				case OBJECT_TYPE_HARVEST:
					core->scan_bitfield |= (1 << SCAN_BITFIELD_OBJ_GATHER);
					core->number_of_harvest_objects++;
					break;
				case OBJECT_TYPE_SPIKE:
					core->scan_bitfield |= (1 << SCAN_BITFIELD_OBJ_SPIKE);
					break;
				case OBJECT_TYPE_STREAM:
				case OBJECT_TYPE_STREAM_DIR:
					core->scan_bitfield |= (1 << SCAN_BITFIELD_OBJ_STREAM);
					break;
				case OBJECT_TYPE_BURST:
				case OBJECT_TYPE_BURST_L:
				case OBJECT_TYPE_BURST_XL:
					core->scan_bitfield |= (1 << SCAN_BITFIELD_OBJ_BURST);
					break;
				case OBJECT_TYPE_PULSE:
				case OBJECT_TYPE_PULSE_L:
				case OBJECT_TYPE_PULSE_XL:
					core->scan_bitfield |= (1 << SCAN_BITFIELD_OBJ_PULSE);
					break;
				case OBJECT_TYPE_ULTRA:
				case OBJECT_TYPE_ULTRA_DIR:
					core->scan_bitfield |= (1 << SCAN_BITFIELD_OBJ_ULTRA);
					break;
				case OBJECT_TYPE_SLICE:
					core->scan_bitfield |= (1 << SCAN_BITFIELD_OBJ_SLICE);
					break;

			}
		}
	}

// clean up:

 if (core->interface_available == 0
		|| core->interface_strength_max == 0)
	{
		core->interface_available = 0;
		core->interface_strength_max = 0;
		core->interface_active = 0;
// TO DO: need some kind of effect if the last depth object is destroyed when there is still an active interface (currently interface will just disappear)
	}
	 else
			core->scan_bitfield |= (1 << SCAN_BITFIELD_OBJ_INTERFACE);

 if (core->interface_strength > core->interface_strength_max)
	{
		core->interface_strength = core->interface_strength_max;
	}

	if (core->data_stored > core->data_storage_capacity)
		core->data_stored = core->data_storage_capacity;

}


// This function destroys a non-core proc, and procs downlink of a non-core proc that has been destroyed.
static void destroy_procs_recursively(struct core_struct* core, struct proc_struct* destroyed_pr, int destroyer_team)
{
	int i;

 core->group_member[destroyed_pr->group_member_index].exists = 0;

// core->group_members--; this is dealt with by set_basic_group_properties()
 destroy_a_proc(destroyed_pr, destroyer_team);
	procs_destroyed_in_this_explosion ++;

// note loop starts at 1
	for (i = 1; i < GROUP_CONNECTIONS; i++)
	{
		if (destroyed_pr->group_connection_exists[i])
		{
   struct cloud_struct* cl = new_cloud(CLOUD_SUB_PROC_EXPLODE, 64, destroyed_pr->group_connection_ptr[i]->position.x, destroyed_pr->group_connection_ptr[i]->position.y);
   if (cl != NULL)
	  {
    cl->angle = destroyed_pr->group_connection_ptr[i]->angle;
    cl->colour = destroyed_pr->player_index; //destroyer_team;
    cl->data [0] = destroyed_pr->group_connection_ptr[i]->shape;
    cl->data [1] = 1;
    cl->speed.x = 0;
    cl->speed.y = 0;
    cl->display_size_x1 = -200;
    cl->display_size_y1 = -200;
    cl->display_size_x2 = 200;
    cl->display_size_y2 = 200;
	  }
   explosion_fragments(destroyed_pr->group_connection_ptr[i]->position.x, destroyed_pr->group_connection_ptr[i]->position.y, destroyed_pr->group_connection_ptr[i]->speed.x, destroyed_pr->group_connection_ptr[i]->speed.y, 6, destroyed_pr->player_index);

			destroy_procs_recursively(core, destroyed_pr->group_connection_ptr[i], destroyer_team);
		}
	}

}

static void destroy_a_proc(struct proc_struct* destroyed_pr, int destroyer_team)
{


 if (w.core[destroyed_pr->core_index].selected == 0 // 0 means it's the first in the selection list
		&& command.select_mode == SELECT_MODE_SINGLE_CORE)
		command.selected_member = -1; // deselect this proc (but not core) if it was specifically selected

 destroyed_pr->exists = 0;
 destroyed_pr->hp = 0;
 destroyed_pr->destroyed_timestamp = w.world_time;

}

static void explosion_fragments(al_fixed x, al_fixed y, al_fixed x_speed, al_fixed y_speed, int fragments, int player_index)
{

	int i;
//	struct cloud_struct* cl;
	al_fixed base_fragment_angle = grand(AFX_ANGLE_1);
	int fragment_speed_int;
	al_fixed fragment_speed;
	cart fragment_position;
	cart fragment_vel;
	al_fixed fragment_angle;
	int explode_time;

	for (i = 0; i < fragments; i ++)
	{
  fragment_angle = base_fragment_angle + (AFX_ANGLE_1 * i / fragments) + grand(AFX_ANGLE_8);
  fragment_speed_int = 2 + grand(12);
  fragment_speed = al_itofix(fragment_speed_int);
  fragment_vel.x = fixed_xpart(fragment_angle, fragment_speed);
  fragment_vel.y = fixed_ypart(fragment_angle, fragment_speed);
//  fragment_vel.x = x_speed + fixed_xpart(fragment_angle, fragment_speed);
//  fragment_vel.y = y_speed + fixed_ypart(fragment_angle, fragment_speed);
  fragment_position.x = x + fragment_vel.x * 5;
  fragment_position.y = y + fragment_vel.y * 5;
  explode_time = fragment_speed_int * 5;// + //grand(20) + fragment_speed_int * 4;


  if (!create_fragment(fragment_position, fragment_vel,
																							6 + grand(10), // size
																							explode_time, // explosion_time
																							explode_time + 32, // lifetime
																							player_index)) // colour
			return; // fragment array full.
	}

}

/*
// Sets a proc's virtual method to broken and creates the appropriate cloud.
void virtual_method_break(struct proc_struct* pr)
{

	  pr->method[pr->virtual_method].data [MDATA_PR_VIRTUAL_STATE] = (0 - VIRTUAL_METHOD_RECYCLE);
//   pr->special_method_penalty --;
   struct cloud_struct* cl = new_cloud(CLOUD_VIRTUAL_BREAK, pr->x, pr->y);

   if (cl != NULL)
			{
    cl->timeout = 16;
    cl->angle = pr->angle;
    cl->colour = pr->player_index;
    cl->data [0] = pr->shape;
    cl->data [1] = pr->size;
    cl->x_speed = 0;//pr->x_speed;
    cl->y_speed = 0;//pr->y_speed;
			}

}*/
/*
// This function creates an explosion for proc pr. It doesn't destroy the proc.
// Team is the team that destroyed the proc (or proc's own team if self-destructed or similar)
void proc_explodes(struct proc_struct* pr, int destroyer_team)
{

 disrupt_block_nodes(pr->x, pr->y, destroyer_team, 5);
 play_game_sound(SAMPLE_KILL, TONE_2E - (pr->size * 4), 50, pr->x, pr->y);

 struct cloud_struct* cl = new_cloud(CLOUD_PROC_EXPLODE_LARGE, pr->x, pr->y);
 if (cl == NULL)
  return; // too many clouds

 cl->timeout = 22;
 cl->angle = pr->angle;
 cl->colour = destroyer_team;
 cl->data [0] = pr->shape;
 cl->data [1] = pr->size;
 cl->x_speed = 0;
 cl->y_speed = 0;

 int i;
 int angle;
 al_fixed speed;

 for (i = 2; i < CLOUD_DATA; i ++)
 {
  cl->data [i] = grand(ANGLE_1);
 }

 int fragments = 8 + (pr->size * 4);
 int fragment_angle = grand(ANGLE_1);


  for (i = 0; i < fragments; i ++)
  {

   cl = new_cloud(CLOUD_PROC_EXPLODE_SMALL, pr->x, pr->y);
   if (cl == NULL)
    return; // too many clouds

   angle = fragment_angle;
   fragment_angle += grand(ANGLE_4);
   speed = al_itofix(100 + grand(1500));

   cl->timeout = 300; // cloud turns into proc_explode_small2 when it stops moving
   cl->angle = int_angle_to_fixed(angle);
   cl->colour = pr->player_index;

   cl->x_speed = fixed_xpart(cl->angle, speed / 100);
   cl->y_speed = fixed_ypart(cl->angle, speed / 100);
   cl->x += fixed_xpart(cl->angle, speed / 25);
   cl->y += fixed_ypart(cl->angle, speed / 25);

   cl->data [0] = 3 + grand(10 + pr->size * 4); // size 1
   cl->data [1] = 3 + grand(10 + pr->size * 4); // size 2
   cl->data [2] = -ANGLE_32 + (grand(ANGLE_16));
//   cl->data [3] = w.player [pr->player_index].colour;
   cl->data [4] = destroyer_team;

  }

}
*/

/*
This function partially removes a proc from the world.
It:
 - sets proc's exists value to -1 (deallocating) (will later be changed to 0 (non-existent))
 - sets proc's deallocating counter to 100 (it's decremented each tick, and when it reaches zero the proc is completely removed)
 - removes proc from any group it's in
 - if proc's bcode_struct has a bnotestruct (which would have been malloc'd), free it (bcodenote not currently implemented)

Does not remove proc from blocklist. Relies on any blocklist check to also check exists value.

Does not subtract 1 from player.processes. This is done when deallocation is complete

*/
/*
int destroy_proc(struct proc_struct* pr)
{

// w.player[pr->player_index].gen_number -= pr->irpt_gen_number;

// unbind_process(pr->index); // removes all markers bound to this process

// if (pr->allocate_method != -1)
  //remove_process_with_allocate(pr);

// if (pr->group != -1)
// {
// severs all connections
  //extract_destroyed_proc_from_group(pr);
// }

 pr->exists = 0;
 pr->hp = 0;
 pr->destroyed_timestamp = w.world_time;


//Need to extract process from its group
// - and may need to destroy core if process is a core process

 return 1;

}
*/


