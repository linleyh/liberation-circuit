#include <allegro5/allegro.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "m_config.h"
#include "m_globvars.h"

#include "g_misc.h"
#include "g_header.h"
#include "g_proc.h"

#include "g_motion.h"
#include "g_method.h"
#include "g_cloud.h"
#include "g_method_misc.h"
#include "g_method_clob.h"
#include "g_method_std.h"
#include "g_world.h"
#include "g_world_back.h"
#include "i_error.h"
#include "x_sound.h"

#include "m_maths.h"


#include "g_shapes.h"
#include "g_proc_new.h"
#include "t_template.h"



extern struct nshape_struct nshape [NSHAPES];
extern struct dshape_struct dshape [NSHAPES]; // uses same indices as NSHAPES
extern struct nshape_init_data_struct nshape_init_data [NSHAPES];
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct game_struct game;


void reset_template_member(struct template_member_struct* member);
int start_setting_up_core_from_template(int c, struct core_struct* core, struct template_struct* templ, int player_index);
int add_notional_member_recursively(struct template_struct* build_templ, int member_index, cart new_position, al_fixed new_angle, int allow_failure);
int find_empty_proc(int player_index, int proc_index_start);
int find_empty_core(int player_index);
int setup_core_from_template(int c, struct core_struct* core, struct template_struct* build_templ, int player_index);
void add_process_from_template(int p, struct template_struct* build_templ, struct core_struct* core, int member_index);
static void init_added_or_restored_proc_details(struct proc_struct* proc, int member_index);
void set_group_member_values_from_notional(struct core_struct* core);
void set_basic_group_properties(struct core_struct* core);
void init_group_object_properties(struct core_struct* core);

struct notional_proc_struct notional_member [GROUP_MAX_MEMBERS];
 // notional member array uses same indices as templ->member array.
 // core->group_member array also uses these indices.

// Call this function to create a new core and group from a template.
// Assumes that everything prior to creation (cost etc) has been handled. (although it re-checks cost in case locking the template changes the cost)
// Assumes that template is valid? Not sure. Should probably check this when loading.
// Returns BUILD_FAIL enum on failure, or core index on success (this means a 0 return is a success, even though it's also the BUILD_FAIL code for no build objects)
int create_new_from_template(struct template_struct* build_templ, int player_index, cart core_position, al_fixed group_angle, struct core_struct** collided_core)
{

// Important - until this function has guaranteed that the new proc will be created (look for the comment below),
//  this function should not make changes to the world struct.

 if (build_templ->active == 0)
		return BUILD_FAIL_TEMPLATE_INACTIVE;

	if (build_templ->locked == 0)
	{
		if (!lock_template(build_templ))
			return BUILD_FAIL_TEMPLATE_NOT_LOCKED;
	}

// Although cost should have been checked previously, locking the template may have changed the cost:
 if (w.player[player_index].data < build_templ->data_cost)
 	return BUILD_FAIL_DATA; // not enough data

 int i, c;
 struct core_struct* core;
 al_fixed core_angle = (group_angle + build_templ->member[0].group_angle_offset) & AFX_MASK;
// struct proc_struct* proc;
 int members = 0;

 c = find_empty_core(player_index);

 if (c == -1)
		return BUILD_FAIL_TOO_MANY_CORES; // failed - too many cores already

 int p = w.player[player_index].proc_index_start - 1; // so that find_empty_proc will start at proc_index_start

 for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (build_templ->member[i].exists == 0)
		{
			notional_member[i].index = -1;
			continue;
		}
		p = find_empty_proc(player_index, p+1); // need to start searching at p+1 to avoid picking up the same proc again and again
		if (p == -1
			|| p >= w.player[player_index].proc_index_end)
			return BUILD_FAIL_TOO_MANY_PROCS; // failed - too many procs already
		notional_member[i].index = p;
		members ++;
	}

// fprintf(stdout, "\nCore %i", c);

	core = &w.core[c];
	core->index = c;
//	core->template_index = build_templ->template_index;
 core->group_angle = group_angle & AFX_MASK;
 core->group_members_max = members;


// set up notional physical properties
 if (!add_notional_member_recursively(build_templ, 0, core_position, core_angle, 1))
		return BUILD_FAIL_OUT_OF_BOUNDS; // currently I think this is the only reason why add_notional_member_recursively would fail

// collision test using notional values
 for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (notional_member[i].index == -1)
			continue;
		if (check_notional_block_collision_multi(notional_member[i].shape, notional_member[i].position.x, notional_member[i].position.y, notional_member[i].angle, build_templ->mobile, player_index, collided_core))
		{
			return BUILD_FAIL_COLLISION; // failed - collision
		}
// check_notional_block_collision_multi won't have checked against other members of same group. This should be done when template is verified.
	}

// From this point on, this function will be making substantive changes to the world_struct and should not be able to fail.
//  (function must be able to return at any point above this line without causing side effects in the world_struct)

// set up core
 start_setting_up_core_from_template(c, core, build_templ, player_index);

// set up each member
 for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (notional_member[i].index == -1)
			continue;
  add_process_from_template(notional_member[i].index, build_templ, core, i);
	}
/*
 fpr("\n A core->core_position %i,%i core_position %i,%i proc.position %i,%i",
					al_fixtoi(core->core_position.x), al_fixtoi(core->core_position.y),
					al_fixtoi(core_position.x), al_fixtoi(core_position.y),
					al_fixtoi(w.proc[core->process_index].position.x), al_fixtoi(w.proc[core->process_index].position.y));*/

	core->core_position = w.proc[core->process_index].position;

	if (w.proc[core->process_index].shape < FIRST_MOBILE_NSHAPE)
		core->mobile = 0;

// note that core_position is the position of the core process. The group's position (centre of mass) may be different.

 set_group_member_values_from_notional(core);

 set_basic_group_properties(core);

 init_group_object_properties(core);

// work out the core's immutable scan bitfield values:
 core->scan_bitfield_immutable = 0; // things that can't change during the lifespan of the core. (actually this could be a template value as it's the same for each core created from a particular template)
 if (w.proc[core->process_index].shape < FIRST_MOBILE_NSHAPE)
		core->scan_bitfield_immutable |= (1<<SCAN_BITFIELD_STATIC);
		 else
  		core->scan_bitfield_immutable |= (1<<SCAN_BITFIELD_MOBILE);
 core->scan_bitfield = core->scan_bitfield_immutable; // updated in set_group_object_properties() with details from components

 set_group_object_properties(core); // must be after core->scan_bitfield_immutable initialised

// core->power_capacity = nshape [w.proc[core->process_index].shape].power_capacity; // needs to be updated after core process is initialised

 if (!core->mobile)
  static_build_affects_block_nodes(core_position.x, core_position.y, 180, player_index);

 play_game_sound(SAMPLE_NEW, TONE_1C, 100, 10, core->core_position.x, core->core_position.y);


/*
 fpr("\n E core->core_position %i,%i core_position %i,%i proc.position %i,%i",
					al_fixtoi(core->core_position.x), al_fixtoi(core->core_position.y),
					al_fixtoi(core_position.x), al_fixtoi(core_position.y),
					al_fixtoi(w.proc[core->process_index].position.x), al_fixtoi(w.proc[core->process_index].position.y));
*/
 return c;

}


// This function is used when building a new proc
//  and also in the command functions that display a notional proc during a build command.
//  Its use in check_build_validity() in g_command.c means that it should not fail if allow_failure == 1
int add_notional_member_recursively(struct template_struct* build_templ, int member_index, cart new_position, al_fixed new_angle, int allow_failure)
{

//fprintf(stdout, "\nAdding member %i at %i, %i", member_index, al_fixtoi(new_position.x), al_fixtoi(new_position.y));

	notional_member[member_index].position = new_position;
 notional_member[member_index].block_position = cart_to_block(new_position);
 if (allow_failure
 	&& !verify_block_position(notional_member[member_index].block_position))
	{
//		fprintf(stdout, "\nverify_block_position failed");
		return 0;
	}
 notional_member[member_index].angle = new_angle;
 notional_member[member_index].shape = build_templ->member[member_index].shape;
// notional_member[member_index].size = templ->member[member_index].size;
//  notional_member[member_index].index = ; this has already been done


 int i;

 int child_index;
 cart child_position;
 al_fixed child_angle;
 int parent_link, child_link;
 struct nshape_struct* parent_shape = &nshape [notional_member[member_index].shape];
 struct nshape_struct* child_shape;

 for (i = 1; i < GROUP_CONNECTIONS; i ++) // note: starts at 1 (0 is always connection to parent)
	{
		if (build_templ->member[member_index].connection[i].template_member_index != -1)
		{
			child_index = build_templ->member[member_index].connection[i].template_member_index;
// first find the position of the link the new process is to be created at, plus a bit of separation:
   parent_link = build_templ->member[member_index].connection[i].link_index;
   child_position = cart_plus_vector(new_position,
																																					new_angle + parent_shape->link_angle_fixed [parent_link],
																																					parent_shape->link_dist_fixed [parent_link] + al_itofix(2)); // note + 2

//  fprintf(stdout, "\n	member %i parent vertex %i dist %i ", child_index, parent_vertex, al_fixtoi(parent_shape->vertex_dist [parent_vertex] + al_itofix(2)));
// now add the new process's link position:
   child_shape = &nshape [build_templ->member[child_index].shape];
   child_link = build_templ->member[member_index].connection[i].reverse_link_index;
// now work out the child's new angle
   child_angle = new_angle + parent_shape->link_angle_fixed [parent_link] + (AFX_ANGLE_2 - child_shape->link_angle_fixed [child_link]) + build_templ->member[child_index].connection_angle_offset;
   child_angle &= AFX_MASK;
   add_vector_to_cart(&child_position,
//																					 AFX_ANGLE_2 - (child_angle + child_shape->vertex_angle [child_vertex]),
																					 (child_angle + child_shape->link_angle_fixed [child_link]),
																					 0 - (child_shape->link_dist_fixed [child_link] + al_itofix(2))); // note + 2
//																		0 - al_itofix(get_link_dist_pixel(child_shape->link_dist_pixel [child_link], build_templ->member[child_index].connection_angle_offset)));

   if (!add_notional_member_recursively(build_templ, child_index, child_position, child_angle, allow_failure))
			{
//				fprintf(stdout, "\nFAIL");
				return 0;
			}
	 }
	}

	return 1;

}

/*

This function sets up a core's basic properties on creation.

It does not set up group stuff.

*/
int start_setting_up_core_from_template(int c, struct core_struct* core, struct template_struct* build_templ, int player_index)
{


 core->exists = 1;
 core->created_timestamp = w.world_time;
 core->destroyed_timestamp = 0;
 core->index = c;
 core->process_index = notional_member[0].index; // core is always process 0
 core->player_index = player_index;
 core->template_index = build_templ->template_index;
 core->build_cooldown_time = 0;
 core->last_build_time = 0;

 core->contact_core_index = -1;
 core->damage_this_cycle = 0;
 core->damage_source_core_index = -1;

 core->scan_range_fixed = al_itofix(SCAN_RANGE_BASE_PIXELS);
 core->scan_range_float = SCAN_RANGE_BASE_PIXELS;
 core->selected = -1;
 core->select_time = 0;
 core->deselect_time = 0;
 core->visibility_checked_by_core_index = -1;
// core->visibility_check_result = 0; // doesn't matter
// core->visibility_checked_timestamp = 0; // doesn't matter

 core->group_total_hp_max_undamaged = 0; // other group_total_hp values depend on current values and are set in set_basic_group_properties()

 core->interface_available = 0;
 core->interface_active = 0;
 core->interface_control_status = 1; // defaults to on (doesn't matter if interface unavailable)
 core->interface_strength = 0;
 core->interface_strength_max = 0;
// core->interface_charge_rate = INTERFACE_CHARGE_RATE_BASE; // currently probably 8
 core->interface_charged_time = 0;
// core->interface_charged_this_cycle = 0;
 core->interface_broken_time = 0;

 core->restore_cooldown_time = 0;
 core->last_repair_restore_time = 0;

 core->data_stored = 0;
 core->data_storage_capacity = 0;

	core->self_destruct = 0;


 int i;

 for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		core->group_member[i].index = -1;
		core->group_member[i].exists = 0;
	}

 for (i = 0; i < MEMORY_SIZE; i ++)
	{
		core->memory [i] = 0;
	}

 for (i = 0; i < PROCESS_MEMORY_SIZE; i ++)
	{
		core->process_memory [i] = -1;
//		core->process_memory_timestamp [i] = 0; - shouldn't be necessary if index is -1
	}

//	core->power_capacity = ; // needs to be updated after core process is initialised
	core->power_left = 0; // doesn't need power when it's just been created
	core->power_use_excess = 0; // doesn't need power when it's just been created
//	core->power_use_predicted = 0;
	core->instructions_used = 0;
//	core->stress = 0;
//	core->stress_level = STRESS_LOW;

	core->attack_mode = ATTACK_MODE_ALL;


	for (i = 0; i < COMMAND_QUEUE; i ++)
	{
		core->command_queue [i].type = COM_NONE;
	}
 core->new_command = 0;
/*
// build commands (one only; no queue)
 core->new_build_command = 0;
// core->build_command_timestamp = 0;
	for (i = 0; i < BUILD_COMMAND_QUEUE; i ++)
	{
		core->build_command_queue[i].active = 0;
	}*/

/* build_command_template = -1;
 core->build_command_x = 0;
 core->build_command_y = 0; // translate from fixed to int when given
 core->build_command_angle = 0; // same*/
// core->number_of_build_objects = 0; // this is set in set_object_properties, which is called later

 core->rebuild_template = 0;
 core->rebuild_x = 0;

 core->rebuild_y = 0;
 core->rebuild_angle = 0;
 core->retry_build_collision_count = 0;
 core->first_build_object_member = build_templ->first_build_object_member;
 core->first_build_object_link = build_templ->first_build_object_link;
 core->first_repair_object_member = build_templ->first_repair_object_member;
 core->first_repair_object_link = build_templ->first_repair_object_link;

 core->messages_received = 0;
 core->message_reading = -1;
 for (i = 0; i < CHANNELS; i ++)
	{
		core->listen_channel [i] = 0; // could default to 1 instead but 0 avoids some unnecessary calculations
	}
// contents of message_struct not initialised; we rely on core->messages_received to avoid reading uninitialised contents of struct

 core->last_execution_timestamp = w.world_time;
 core->next_execution_timestamp = w.world_time + 15 + (c & (EXECUTION_COUNT - 1)); // first execution is delayed a bit; the c & 15 should spread out execution a bit between frames
 core->construction_complete_timestamp = core->next_execution_timestamp;
// next_execution_timestamp and construction_complete_timestamp are likely to be reset by the calling function as they depend on the conditions in which the process was built.
 core->cycles_executed = 0;
 core->mobile = 1; // this is fixed later if the core is static
 core->group_speed.x = 0;
 core->group_speed.y = 0;
 core->group_spin = 0;

// core's group_member struct is set up in set_group_physics_properties()

 core->bubble_text [0] = '\0';
 core->bubble_text_time = 0;
 core->bubble_text_time_adjusted = 0;
 core->bubble_text_length = 0;

 core->special_AI_type = 0;
 core->special_AI_value = 0;
 core->special_AI_time = 0;


	return 1;

}

/*
This function sets up a core's group_member values and some group physics properties and similar.
It assumes that the core struct and each of its proc members have been initialised and exist.
It assumes that the core's group_member array includes process indices
 - but does not assume that any other information in the group_member array

It should be able to be called during proc creation and also anytime a group's composition changes (e.g. a component is destroyed)

*/
void set_basic_group_properties(struct core_struct* core)
{


// These values will be filled in below
 core->group_members_current = 0;
 core->group_mass = 0;
// core->group_moment = 0; set below
// core->group_drag = (al_itofix(1014) / 1024);//DRAG_BASE_FIXED;
 core->group_total_hp = 0;
 core->group_total_hp_max_current = 0;
 core->power_capacity = nshape[w.proc[core->process_index].shape].power_capacity;
 core->instructions_per_cycle = nshape[w.proc[core->process_index].shape].instructions_per_cycle;
 core->interface_charge_rate_per_object = nshape[w.proc[core->process_index].shape].interface_charge_rate;
// core->group_total_hp_max_undamaged = ; this is only set at creation

// use int for working out centre of mass to avoid overflowing al_fixed.
// it's okay if centre of mass is not exact
 int temp_centre_of_mass_x = 0;
 int temp_centre_of_mass_y = 0;
 int i;
 struct proc_struct* proc;

 for (i = 0; i < core->group_members_max; i ++)
	{
		if (core->group_member[i].exists == 0)
			continue;
		proc = &w.proc[core->group_member[i].index];
		proc->mobile = core->mobile;
		core->group_members_current ++;
		core->group_mass += proc->mass;
		core->group_total_hp += proc->hp;
		core->group_total_hp_max_current += proc->hp_max;
		temp_centre_of_mass_x += (al_fixtoi(proc->position.x) * proc->mass) / 10;
		temp_centre_of_mass_y += (al_fixtoi(proc->position.y) * proc->mass) / 10;
/*		fprintf(stdout, "\nmember %i proc %i temp+= %i, %i", i, core->group_member[i].index,
												(al_fixtoi(proc->position.x) * proc->mass) / 10,
												(al_fixtoi(proc->position.y) * proc->mass) / 10);*/

//		fprintf(stdout, "\nProc %i total mass %i temp_com %i,%i ", core->group_member[i].index, proc->mass, core->group_mass, al_fixtoi
	}

	core->group_centre_of_mass.x = al_itofix((temp_centre_of_mass_x * 10) / core->group_mass);
	core->group_centre_of_mass.y = al_itofix((temp_centre_of_mass_y * 10) / core->group_mass);

 core->power_capacity += (core->group_members_current - 1) * nshape[w.proc[core->process_index].shape].component_power_capacity; // -1 is to exclude the core

/*
  fprintf(stdout, "\nCore %i,%i com %i,%i temp %i,%i mass %i ", al_fixtoi(w.proc[core->process_index].position.x),
																																												al_fixtoi(w.proc[core->process_index].position.y),
																																												al_fixtoi(core->group_centre_of_mass.x),
																																												al_fixtoi(core->group_centre_of_mass.y),
										temp_centre_of_mass_x,
										temp_centre_of_mass_y,
										core->group_mass);*/

 core->core_offset_from_group_centre = xy_to_polar(w.proc[core->process_index].position.x - core->group_centre_of_mass.x,
																																																			w.proc[core->process_index].position.y - core->group_centre_of_mass.y);
	core->core_offset_from_group_centre.angle -= core->group_angle;

 al_fixed proc_distance_from_centre_of_mass;

 core->group_moment = w.proc[core->process_index].mass;

// Now that we know the centre of mass, we can work out the moment:
 for (i = 1; i < core->group_members_max; i ++)
	{
		if (core->group_member[i].exists == 0)
			continue;
		proc = &w.proc[core->group_member[i].index];
		proc_distance_from_centre_of_mass = distance(proc->position.y - core->group_centre_of_mass.y, proc->position.x - core->group_centre_of_mass.x);
  core->group_moment += proc->mass * al_fixtoi(al_fixmul(proc_distance_from_centre_of_mass / 800, proc_distance_from_centre_of_mass)); // 800 is an arbitrary factor
	}


// To do this properly we should also derive the speed of the group's centre of mass from the speeds of all of its members.

 core->group_mass_for_collision_comparison = core->group_mass;

 if (!core->mobile)
		core->group_mass_for_collision_comparison += 100000; // arbitrarily high number - this just means that a mobile process will tend to bounce off a static one

 core->constant_accel_angle_offset = 0;
	core->constant_accel_rate = 0;
	core->constant_spin_change = 0;


}

// This function sets core->group_member values from notional_member values
void set_group_member_values_from_notional(struct core_struct* core)
{

 int i;

 for (i = 0; i < GROUP_MAX_MEMBERS; i++)
	{
		if (notional_member[i].index == -1)
		{
			core->group_member[i].index = -1;
			core->group_member[i].exists = 0;
			continue;
		}
		core->group_member[i].index = notional_member[i].index;
		core->group_member[i].exists = 1;
		core->group_member[i].position_offset = xy_to_polar(notional_member[i].position.x - notional_member[0].position.x, notional_member[i].position.y - notional_member[0].position.y);
  core->group_member[i].angle_offset = notional_member[i].angle - core->group_angle;
	}

}





// Finds an empty entry in w's proc array at an appropriate place for this team
// Doesn't allocate or otherwise alter the proc
// Starts searching at proc_index_start (use this to find multiple empty procs without needing to set the "exists' value of each one.)
// Returns index of empty proc on success, -1 on failure
int find_empty_proc(int player_index, int proc_index_start)
{

 int p;

 for (p = proc_index_start; p < w.player[player_index].proc_index_end; p ++)
 {
  if (w.proc[p].exists == 0
			&& w.proc[p].reserved == 0
			&& w.proc[p].destroyed_timestamp < w.world_time - DEALLOCATE_COUNTER)
   return p;
 }

 return -1; // team is full

}

int find_empty_core(int player_index)
{

 int c;

 for (c = w.player[player_index].core_index_start; c < w.player[player_index].core_index_end; c ++)
 {
  if (w.core[c].exists == 0
			&& w.core[c].destroyed_timestamp < w.world_time - DEALLOCATE_COUNTER)
   return c;
 }

 return -1; // team is full

}


void add_process_from_template(int p, struct template_struct* build_templ, struct core_struct* core, int member_index)
{

 int i;

	struct proc_struct* proc = &w.proc[p];

// proc->max_length = nshape [proc->shape].max_length;

	proc->reserved = 1; // even if destroyed, this proc won't be re-used as long as its group still exists (so that it can be restored if needed)
	proc->player_index = core->player_index;
	proc->index = p;
	proc->shape = notional_member[member_index].shape;
	proc->core_index = core->index;
	proc->group_member_index = member_index;
	proc->mass = build_templ->member[member_index].mass;
	proc->mass_for_collision_comparison = proc->mass;
	if (!core->mobile)
		proc->mass_for_collision_comparison += 50000; // arbitrarily high number
//	proc->moment = nshape[proc->shape].shape_mass; // fix! (individual proc moment may not be relevant any more - check this)
	proc->hp_max = nshape[build_templ->member[0].shape].base_hp_max; // all components get hp based on core hp
	if ((game.story_type == STORY_TYPE_HARD
	 || game.story_type == STORY_TYPE_ADVANCED_HARD))
	 proc->hp_max += 30;

//	if (w.local_condition == LOCAL_CONDITION_FRAGILE_PROCS) // ... except in fragile proc condition
 	//proc->hp_max = 60;
	proc->hp = proc->hp_max;
 proc->packet_collision_size = proc->size;
 proc->nshape_ptr = &nshape [proc->shape];
 proc->repaired_timestamp = 0;
 proc->interface_stability = 0;
// proc->interface_stability_on_time = 0;
// proc->interface_stability_off_time = 0;

	for (i = 0; i < MAX_LINKS; i ++)
	{
		proc->object[i] = build_templ->member[member_index].object [i];
	}
 init_added_or_restored_proc_details(proc, member_index);

//	proc->max_length = proc->nshape_ptr->max_length;
	proc->drag = DRAG_BASE_FIXED;

//	if (core->interface_strength_max > 0
//		&& build_templ->member[member_index].interface_can_protect)
	if (build_templ->member[member_index].interface_can_protect)
	 proc->interface_protects = 1;
		 else
    proc->interface_protects = 0;

//	proc->interface_object_present = 0; // will be reset later if needed
//	proc->interface_on_process_set_on = 1; // will be reset later if needed
//	proc->interface_depth = 0;

// this stuff should probably be stored just in the template...
 core->group_total_hp_max_undamaged += proc->hp_max;
// core->group_members_max ++; done previously

	template_connection_struct* connect;

	proc->number_of_group_connections = 0;

	int connected_member_index, connected_proc_index;

	 for (i = 0; i < GROUP_CONNECTIONS; i++)
	 {
 		if (build_templ->member[member_index].connection[i].template_member_index == -1)
		 {
 			proc->group_connection_ptr [i] = NULL;
 			proc->group_connection_exists [i] = 0;
			 continue;
		 }
		 proc->number_of_group_connections ++;
		 connected_member_index = build_templ->member[member_index].connection[i].template_member_index;
		 connected_proc_index = notional_member[connected_member_index].index;
		 connect = &build_templ->member[member_index].connection [i];
//		fpr("\n pr %i connection %i link %i to proc %i", p, i, connect->link_index, connected_proc_index);
		 proc->group_connection_ptr [i] = &w.proc[connected_proc_index];
		 proc->group_connection_exists [i] = 1;
		 proc->connected_from [i] = connect->reverse_connection_index;
//fpr("\n New proc: connected_from [%i] = %i", i, proc->connected_from [i]);

		 proc->connection_link [i] = connect->link_index;
		 proc->connected_from_link [i] = connect->reverse_link_index;
		 proc->connection_angle [i] = angle_from_cart_to_cart(proc->position, notional_member[connected_member_index].position) - proc->angle;
		 proc->connection_dist [i] = distance_from_cart_to_cart(proc->position, notional_member[connected_member_index].position);
		 proc->connection_angle_difference [i] = notional_member[connected_member_index].angle - proc->angle;
		 //fprintf(stdout, "\nProc %i connection %i to proc %i member_index %i\n", p, i, connected_proc_index, connected_member_index);
	 }


}

// This function initialises values for a newly added or restored proc
//  Basic values that only need to be set on creation are just in add_process_from_template(), because they don't need to be
//  updated if the proc is destroyed and then restored (as the w.proc[] entry will be reserved)
static void init_added_or_restored_proc_details(struct proc_struct* proc, int member_index)
{

	proc->exists = 1;
 proc->created_timestamp = w.world_time;
	proc->selected = 0;
	proc->select_time = 0;
	proc->deselect_time = 0;
	proc->hit_pulse_time = 0;
	proc->component_hit_time = 0;

// These may be wrong for restored proc... but this isn't particularly easy to fix. Think about it!
	proc->position = notional_member[member_index].position;
	proc->old_position = proc->position;
	proc->block_position = notional_member[member_index].block_position;
	proc->speed.x = 0;
	proc->speed.y = 0;
	proc->angle = notional_member[member_index].angle;
	proc->old_angle = proc->angle;
	proc->spin = 0;
	proc->hit_edge_this_cycle = 0;
	proc->provisional_angle = proc->angle;
	proc->prov = 0;

	proc->interface_raised_time = 0;
	proc->interface_lowered_time = 0;
	proc->interface_hit_time = 0;

	add_proc_to_blocklist(proc);

	int i;

	for (i = 0; i < MAX_LINKS; i ++)
	{
		proc->object_instance[i].angle_offset = proc->object[i].base_angle_offset; // the object_instance version may be able to change. The object version remains as a default (which can be returned to)
		proc->object_instance[i].angle_offset_angle = proc->object[i].base_angle_offset_angle;

	}



}

// assumes that member_index is valid
//  this means that the proc should still be reserved, and all of its connection information etc should still be valid.
s16b restore_component(struct core_struct* core, int player_index, int template_index, int member_index)
{

		notional_member[member_index].index = core->group_member[member_index].index;

		struct template_struct* restore_templ = &templ[player_index][template_index];
		struct template_member_struct* template_member = &restore_templ->member[member_index];

		notional_member[member_index].shape = template_member->shape;

		struct proc_struct* upstream_pr = &w.proc[core->group_member[template_member->connection[0].template_member_index].index];
		int connection_index = template_member->connection[0].reverse_connection_index;

  notional_member[member_index].position.x = upstream_pr->position.x + fixed_xpart(upstream_pr->connection_angle [connection_index] + upstream_pr->angle, upstream_pr->connection_dist [connection_index]);
  notional_member[member_index].position.y = upstream_pr->position.y + fixed_ypart(upstream_pr->connection_angle [connection_index] + upstream_pr->angle, upstream_pr->connection_dist [connection_index]);

  notional_member[member_index].angle = upstream_pr->angle + upstream_pr->connection_angle_difference [connection_index];

		notional_member[member_index].block_position = cart_to_block(notional_member[member_index].position);
		if (!verify_block_position(notional_member[member_index].block_position))
			return 0;

		struct core_struct* unused_collision_core_parameter;
		if (check_notional_block_collision_multi(notional_member[member_index].shape, notional_member[member_index].position.x, notional_member[member_index].position.y, notional_member[member_index].angle, core->mobile, player_index, &unused_collision_core_parameter))
			return 0; // collision failure
// consider allowing if friendly and either or both are friendly.
// consider whether this should repel the process that's in the way, like a build attempt that fails because of a collision does

// Now, restore it:
  struct proc_struct* restored_proc = &w.proc[core->group_member[member_index].index];
  init_added_or_restored_proc_details(restored_proc, member_index);

  restored_proc->group_connection_exists [0] = 1; // connection [0] is always uplink
/*
  fpr("\n*********\n restore_component core->index %i core->process_index %i",
						core->index, core->process_index);

  fpr("\n   restored_proc->index %i",
						restored_proc->index);

  fpr("\n   restored_proc->connected_from[0] %i",
						restored_proc->connected_from [0]);

  fpr("\n   restored_proc->group_connection_ptr[0]->index %i",
						restored_proc->group_connection_ptr[0]->index);

  fpr("\n   restored_proc->group_connection_ptr[0]->group_member_index %i",
						restored_proc->group_connection_ptr[0]->group_member_index);

  fpr("\n   core->group_member[restored_proc->group_connection_ptr[0]->group_member_index].index %i",
						core->group_member[restored_proc->group_connection_ptr[0]->group_member_index].index);

  fpr("\n*********\n");
*/
// set the upstream proc's group_connection_exists:
  w.proc[core->group_member[restored_proc->group_connection_ptr[0]->group_member_index].index].group_connection_exists [restored_proc->connected_from [0]] = 1;

  core->group_member[member_index].exists = 1;

  play_game_sound(SAMPLE_RESTORE, TONE_2C, 160, 1, restored_proc->position.x, restored_proc->position.y);

	 return 1;

}



void init_group_object_properties(struct core_struct* core)
{

	int m;
	int o;
	struct proc_struct* proc;

	for (m = 0; m < core->group_members_max; m++)
	{
		if (core->group_member[m].exists == 0)
		 continue;
		proc = &w.proc[core->group_member[m].index];
		for (o = 0; o < MAX_OBJECTS; o++)
		{

			proc->object_instance[o].ongoing_power_cost = 0;
			proc->object_instance[o].ongoing_power_cost_finish_time = 0;
			proc->object_instance[o].int_value1 = 0;
			proc->object_instance[o].int_value2 = 0;
			proc->object_instance[o].int_value3 = 0;
			proc->object_instance[o].fixed_value1 = 0;
			proc->object_instance[o].fixed_value2 = 0;
			proc->object_instance[o].fixed_value3 = 0;
// note that many things are done in set_group_object_properties() in g_proc.c, which is called when process composition changes (which can change e.g. centre of mass)
// this function should only be used for things that need to be done only on creation
//  (many things also need to be dealt with for partial destruction)

			switch(proc->object[o].type)
			{
//			 case OBJECT_TYPE_INTERFACE:
//			 	proc->object_instance[o].interface_object_active = 1; // this is default
//			 	proc->interface_on_process_set_on = 1; // defaults to on
//				 break;
				case OBJECT_TYPE_PULSE:
				case OBJECT_TYPE_PULSE_L:
				case OBJECT_TYPE_PULSE_XL:
				case OBJECT_TYPE_STREAM_DIR:
				case OBJECT_TYPE_ULTRA_DIR:
  			proc->object_instance[o].rotate_to_angle_offset = proc->object_instance[o].angle_offset;
  			break;


			}
//			 case OBJECT_TYPE_MOVE:
//			 	proc->object_instance[o].move_power = 0; // int value that is set from 0 to 100 by user.
//			 	proc->object_instance[o].move_accel_rate = al_itofix(1);
//			 	calculate_move_object_properties(core, proc, o);
//				 break;
//				case OBJECT_TYPE_PACKET:
//				case OBJECT_TYPE_PACKET_DIR:
//					proc->object_instance[o].packet_fire_timestamp = 0;
//					break;
// remember - object_instance values will NOT be initialised to zero unless this is done here!
//			}
		}
	} // end for m loop that goes through each group member

}


// Call this when a group/single proc is created, and also anytime its mass distribution changes (e.g. a subprocess is destroyed)
void calculate_move_object_properties(struct core_struct* group_core, struct proc_struct* proc, int object_index)
{

 al_fixed force_dist_from_centre;
 al_fixed lever_angle;
 al_fixed impulse_angle;
 al_fixed force = al_itofix(1) / 100;

	if (group_core->group_members_current == 1)
	{
// with a single proc we can just assume that the centre of mass is the same as the nshape centre, and work from the nshape values:
  force_dist_from_centre = proc->nshape_ptr->object_dist_fixed [object_index] / FORCE_DIST_DIVISOR;
  lever_angle = proc->nshape_ptr->object_angle_fixed [object_index];// + proc->angle;
  impulse_angle = proc->nshape_ptr->object_angle_fixed [object_index] + proc->object_instance[object_index].angle_offset;

  proc->object_instance[object_index].move_spin_change = al_fixmul(al_fixmul(symmetrical_sin(lever_angle - impulse_angle), force_dist_from_centre), force) * 1000;
/*
  fpr("\nmove_spin_change: obj %i %f (%f,%f,%f - %f,%f)", object_index,
						al_fixtof(proc->object_instance[object_index].move_spin_change),
						al_fixtof(symmetrical_sin(lever_angle - impulse_angle)),
						al_fixtof(al_fixmul(symmetrical_sin(lever_angle - impulse_angle), force_dist_from_centre)),
						al_fixtof(force_dist_from_centre),
						al_fixtof(lever_angle),
						al_fixtof(impulse_angle)
						);
fpr(" (link_angle %f dist %f)", al_fixtof(proc->nshape_ptr->object_angle_fixed [object_index]), al_fixtof(proc->nshape_ptr->vertex_dist_fixed [object_index]));*/

#define MOVE_ACCEL_MODIFIER 5

  proc->object_instance[object_index].move_spin_change /= group_core->group_moment;

//  fpr(" : %f", al_fixtof(proc->object_instance[object_index].move_spin_change));

  proc->object_instance[object_index].move_accel_rate = ((force * MOVE_ACCEL_MODIFIER) / proc->mass) * 1000;

  proc->object_instance[object_index].move_accel_angle_offset = (proc->nshape_ptr->object_angle_fixed [object_index] + proc->object_instance[object_index].angle_offset + group_core->group_member[0].angle_offset + AFX_ANGLE_2) & AFX_MASK;

	}
	 else
		{
// Need to find displacement of process from centre of mass of group
//  - because centre of mass is used, and the process might not match the template, can't rely on template or nshape values like we can with single procs
			al_fixed x = proc->position.x + fixed_xpart(proc->angle + proc->nshape_ptr->object_angle_fixed [object_index], proc->nshape_ptr->object_dist_fixed [object_index]);
   al_fixed y = proc->position.y + fixed_ypart(proc->angle + proc->nshape_ptr->object_angle_fixed [object_index], proc->nshape_ptr->object_dist_fixed [object_index]);

   force_dist_from_centre = distance(y - group_core->group_centre_of_mass.y, x - group_core->group_centre_of_mass.x) / FORCE_DIST_DIVISOR;
   lever_angle = get_angle(y - group_core->group_centre_of_mass.y, x - group_core->group_centre_of_mass.x) - group_core->group_angle;
//   impulse_angle = (proc->angle - group_core->group_angle) + proc->nshape_ptr->link_angle_fixed [object_index] + proc->object[object_index].angle_offset + AFX_ANGLE_2;
   impulse_angle = (proc->angle - group_core->group_angle) + proc->nshape_ptr->object_angle_fixed [object_index] + proc->object_instance[object_index].angle_offset;// + AFX_ANGLE_2;

   proc->object_instance[object_index].move_spin_change = al_fixmul(al_fixmul(symmetrical_sin(lever_angle - impulse_angle), force_dist_from_centre), force) * 1000;
   proc->object_instance[object_index].move_spin_change /= group_core->group_moment;

   proc->object_instance[object_index].move_accel_rate = ((force * MOVE_ACCEL_MODIFIER) / group_core->group_mass) * 1000;

   proc->object_instance[object_index].move_accel_angle_offset = ((proc->angle + proc->nshape_ptr->object_angle_fixed [object_index] + proc->object_instance[object_index].angle_offset) - group_core->group_angle + AFX_ANGLE_2) & AFX_MASK;

// fprintf(stdout, "\ngroup: v %i impulse_angle %i spin_change %f", object_index, fixed_angle_to_int(impulse_angle), fixed_to_radians(proc->object_instance[object_index].move_spin_change));

		}

}



