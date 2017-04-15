/*

g_method_core.c

Functions for calls to built-in (core) methods (i.e. ones that don't require an object)


*/

#include <allegro5/allegro.h>

#include <stdio.h>
#include <math.h>

#include "m_config.h"

#include "g_header.h"

#include "g_world.h"
#include "g_misc.h"
#include "g_proc.h"
#include "g_packet.h"
#include "g_group.h"
#include "g_motion.h"
#include "g_method.h"
#include "g_method_sy.h"
#include "g_method_pr.h"
#include "g_method_clob.h"
#include "g_method_misc.h"
#include "g_cloud.h"
#include "m_globvars.h"
#include "m_maths.h"
#include "t_template.h"
#include "x_sound.h"

#include "i_console.h"
#include "i_disp_in.h"
#include "c_keywords.h"
#include "g_method_core.h"

#include "v_interp.h"

extern struct view_struct view; // TO DO: think about putting a pointer to this in the worldstruct instead of externing it
extern struct control_struct control; // defined in i_input.c. Used here for client process methods.
extern struct game_struct game;
extern struct vmstate_struct vmstate; // defined in v_interp.c

#define CMETHOD_CALL_PARAMETERS 6



struct cmethod_call_type_struct cmethod_call_type [CMETHOD_CALL_TYPES] =
{
// {int parameters},
	{0, KEYWORD_CMETHOD_GET_CORE_X}, // *CMETHOD_CALL_GET_CORE_X
	{0, KEYWORD_CMETHOD_GET_CORE_Y}, // *CMETHOD_CALL_GET_CORE_Y
	{0, KEYWORD_CMETHOD_GET_PROCESS_X}, // *CMETHOD_CALL_GET_PROCESS_X
	{0, KEYWORD_CMETHOD_GET_PROCESS_Y}, // *CMETHOD_CALL_GET_PROCESS_Y
	{0, KEYWORD_CMETHOD_GET_CORE_ANGLE}, // *CMETHOD_CALL_GET_CORE_ANGLE
	{0, KEYWORD_CMETHOD_GET_CORE_SPIN}, // *CMETHOD_CALL_GET_CORE_SPIN
	{0, KEYWORD_CMETHOD_GET_CORE_SPEED_X}, // *CMETHOD_CALL_GET_CORE_SPEED_X
	{0, KEYWORD_CMETHOD_GET_CORE_SPEED_Y}, // *CMETHOD_CALL_GET_CORE_SPEED_Y
 {0, KEYWORD_CMETHOD_GET_INTERFACE_STRENGTH}, // *CMETHOD_CALL_GET_INTERFACE_STRENGTH
 {0, KEYWORD_CMETHOD_GET_INTERFACE_CAPACITY}, // *CMETHOD_CALL_GET_INTERFACE_CAPACITY
 {0, KEYWORD_CMETHOD_GET_USER}, // *CMETHOD_CALL_GET_USER
 {0, KEYWORD_CMETHOD_GET_TEMPLATE}, // *CMETHOD_CALL_GET_TEMPLATE
 {0, KEYWORD_CMETHOD_DISTANCE}, // *CMETHOD_CALL_DISTANCE
// {0}, // CMETHOD_CALL_DISTANCE_HYPOT
 {1, KEYWORD_CMETHOD_DISTANCE_LESS}, // *CMETHOD_CALL_DISTANCE_LESS
 {1, KEYWORD_CMETHOD_DISTANCE_MORE}, // *CMETHOD_CALL_DISTANCE_MORE
 {0, KEYWORD_CMETHOD_TARGET_ANGLE}, // *CMETHOD_CALL_TARGET_ANGLE


 {0, KEYWORD_CMETHOD_GET_COMPONENTS}, // *CMETHOD_CALL_GET_COMPONENTS
 {0, KEYWORD_CMETHOD_GET_COMPONENTS_MAX}, // *CMETHOD_CALL_GET_COMPONENTS_MAX
 {0, KEYWORD_CMETHOD_GET_TOTAL_INTEGRITY}, // *CMETHOD_CALL_GET_TOTAL_INTEGRITY
 {0, KEYWORD_CMETHOD_GET_TOTAL_INTEGRITY_MAX}, // *CMETHOD_CALL_GET_TOTAL_INTEGRITY_MAX
 {0, KEYWORD_CMETHOD_GET_UNHARMED_INTEGRITY_MAX}, // *CMETHOD_CALL_GET_UNHARMED_INTEGRITY_MAX
	{0, KEYWORD_CMETHOD_VISIBLE}, // *CMETHOD_CALL_VISIBLE
	{0, KEYWORD_CMETHOD_TARGET_SIGNATURE}, // *CMETHOD_CALL_TARGET_SIGNATURE


// note that since core might not be the calling core, all core methods should be read only

};

s16b call_core_method(struct core_struct* calling_core, struct core_struct* called_core, int call_value, s16b* stack_parameters);
s16b call_member_method(struct core_struct* calling_core, struct core_struct* called_core, int member_world_proc_index, int call_value, s16b* stack_parameters);

int verify_target_core(struct core_struct* calling_core, int target_core_index, struct core_struct** target_core);
int verify_friendly_target_core(struct core_struct* calling_core, int target_core_index, struct core_struct** target_core);

// core may be the calling core, but might not be.
// core must be valid (and not NULL), though.
s16b call_core_method(struct core_struct* calling_core, struct core_struct* called_core, int call_value, s16b* stack_parameters)
{

 vmstate.instructions_left -= 2;

 switch(call_value)
 {
	 case CMETHOD_CALL_GET_CORE_X:
	 	return al_fixtoi(w.proc[called_core->process_index].position.x);
	 case CMETHOD_CALL_GET_CORE_Y:
	 	return al_fixtoi(w.proc[called_core->process_index].position.y);
	 case CMETHOD_CALL_GET_PROCESS_X:
	 	return al_fixtoi(called_core->group_centre_of_mass.x);
	 case CMETHOD_CALL_GET_PROCESS_Y:
	 	return al_fixtoi(called_core->group_centre_of_mass.y);
	 case CMETHOD_CALL_GET_CORE_ANGLE:
	 	return fixed_angle_to_short(called_core->group_angle);
	 case CMETHOD_CALL_GET_CORE_SPIN:
	 	return fixed_angle_to_short(called_core->group_spin * 16);
	 case CMETHOD_CALL_GET_CORE_SPEED_X:
	 	return al_fixtoi(called_core->group_speed.x * 16);
	 case CMETHOD_CALL_GET_CORE_SPEED_Y:
	 	return al_fixtoi(called_core->group_speed.y * 16);
		case CMETHOD_CALL_GET_INTERFACE_STRENGTH:
			return called_core->interface_strength;
		case CMETHOD_CALL_GET_INTERFACE_CAPACITY:
			return called_core->interface_strength_max;
		case CMETHOD_CALL_GET_USER:
			return called_core->player_index;
		case CMETHOD_CALL_GET_TEMPLATE:
			return called_core->template_index;
		case CMETHOD_CALL_DISTANCE:
			if (calling_core == called_core)
				return 0;
			return al_fixtoi(distance_oct_xyxy(w.proc[called_core->process_index].position.x,
																																						w.proc[called_core->process_index].position.y,
																																						w.proc[calling_core->process_index].position.x,
																																						w.proc[calling_core->process_index].position.y));
//			abs(al_fixtoi(w.proc[called_core->process_index].position.x - w.proc[calling_core->process_index].position.x))
//			     + abs(al_fixtoi(w.proc[called_core->process_index].position.y - w.proc[calling_core->process_index].position.y));
/*		case CMETHOD_CALL_DISTANCE_HYPOT:
// returns accurate distance, but is expensive
			if (calling_core == called_core)
				return 0;
   vmstate.instructions_left -= INSTRUCTION_COST_HYPOT;
	 	return al_fixtoi(distance(w.proc[called_core->process_index].position.x - w.proc[calling_core->process_index].position.x,
																				         w.proc[called_core->process_index].position.y - w.proc[calling_core->process_index].position.y));
*/

			case CMETHOD_CALL_DISTANCE_LESS:
				{
					int distance_check = al_fixtoi(distance_oct_xyxy(w.proc[called_core->process_index].position.x,
																																						                w.proc[called_core->process_index].position.y,
																																						                w.proc[calling_core->process_index].position.x,
																																						                w.proc[calling_core->process_index].position.y));
					if (distance_check < stack_parameters [0])
						return 1;
					return 0;

/*
					uint64_t	x_dist = al_fixtoi(w.proc[called_core->process_index].position.x - w.proc[calling_core->process_index].position.x);
					x_dist *= x_dist;
					uint64_t	y_dist = al_fixtoi(w.proc[called_core->process_index].position.y - w.proc[calling_core->process_index].position.y);
					y_dist *= y_dist;
					uint64_t compare_value = stack_parameters [0] * stack_parameters [0];

					if (x_dist + y_dist < compare_value)
						return 1;
					return 0;*/
				}
			case CMETHOD_CALL_DISTANCE_MORE:
				{
					int distance_check = al_fixtoi(distance_oct_xyxy(w.proc[called_core->process_index].position.x,
																																						                w.proc[called_core->process_index].position.y,
																																						                w.proc[calling_core->process_index].position.x,
																																						                w.proc[calling_core->process_index].position.y));
					if (distance_check > stack_parameters [0])
						return 1;
					return 0;
/*
					uint64_t	x_dist = al_fixtoi(w.proc[called_core->process_index].position.x - w.proc[calling_core->process_index].position.x);
					x_dist *= x_dist;
					uint64_t	y_dist = al_fixtoi(w.proc[called_core->process_index].position.y - w.proc[calling_core->process_index].position.y);
					y_dist *= y_dist;
					uint64_t compare_value = stack_parameters [0] * stack_parameters [0];

					if (x_dist + y_dist > compare_value)
						return 1;
					return 0;*/
				}
		case CMETHOD_CALL_TARGET_ANGLE:
			{
    vmstate.instructions_left -= INSTRUCTION_COST_ATAN2; // expensive operation
	 	 return fixed_angle_to_short(get_angle(called_core->core_position.y - calling_core->core_position.y, called_core->core_position.x - calling_core->core_position.x));
			}


		case CMETHOD_CALL_GET_COMPONENTS:
			return called_core->group_members_current;
		case CMETHOD_CALL_GET_COMPONENTS_MAX:
			return called_core->group_members_max;
		case CMETHOD_CALL_GET_TOTAL_INTEGRITY:
			return called_core->group_total_hp;
		case CMETHOD_CALL_GET_TOTAL_INTEGRITY_MAX:
			return called_core->group_total_hp_max_current;
		case CMETHOD_CALL_GET_UNHARMED_INTEGRITY_MAX:
			return called_core->group_total_hp_max_undamaged;
		case CMETHOD_CALL_VISIBLE:
			return 1; // if we got to this point it must be visible
		case CMETHOD_CALL_TARGET_SIGNATURE:
/*			fpr("\n target_signature: target [");
			print_binary(called_core->scan_bitfield);
			fpr("] test [");
			print_binary(stack_parameters [0]);
			fpr("] result %i", (called_core->scan_bitfield & stack_parameters [0]) == 0);*/
//   if ((called_core->scan_bitfield & stack_parameters [0]) == 0)
//				return 0;
//			return 1;
  return called_core->scan_bitfield;

 } // end switch(call_value)

// error: invalid call_value
 vmstate.error_state = 1;
 return 0;


}

// assumes calling_core is valid and not NULL.
// doesn't assume anything about call_value.
s16b call_self_core_method(struct core_struct* calling_core, int call_value)
{

	s16b stack_parameters [CMETHOD_CALL_PARAMETERS];

	if (call_value < 0
		|| call_value >= CMETHOD_CALL_TYPES)
	{
		if (w.debug_mode)
			print_method_error("invalid self process method call type", 1, call_value);
  vmstate.error_state = 1;
		return 0;
	}

// pull_values_from_stack call must come after call_value bounds check as it uses call_value as an array index
	if (!pull_values_from_stack(stack_parameters, cmethod_call_type[call_value].parameters))
	{
//		fpr("\nCouldn't pull values for self core call from stack");
		if (w.debug_mode)
			print_method_error("self process method call stack error", 0, 0);
  vmstate.error_state = 1;
		return 0;
	}

	return call_core_method(calling_core, calling_core, call_value, stack_parameters);

}

// assumes calling_core is either valid or NULL.
// If calling_core is NULL, assumes that the system program is calling the method (may not be implemented yet)
// does not assume anything about call_value.
s16b call_extern_core_method(struct core_struct* calling_core, int call_value)
{



	struct core_struct* target_core;

	s16b stack_parameters [CMETHOD_CALL_PARAMETERS];

	if (call_value < 0
		|| call_value >= CMETHOD_CALL_TYPES)
	{
		if (w.debug_mode)
			print_method_error("invalid external process method call type", 1, call_value);
  vmstate.error_state = 1;
		return 0;
	}



	if (!pull_values_from_stack(stack_parameters, cmethod_call_type[call_value].parameters + 1)) // Note + 1 is for core index
	{
		if (w.debug_mode)
			print_method_error("self process method call stack error", 0, 0);
  vmstate.error_state = 1;
		return 0;
	}


	int target_core_index = stack_parameters [0];

// int core_verification = verify_target_core(calling_core, target_core_index, &target_core);
// if (core_verification == -3) // error (such as invalid target_core_index)
//  vmstate.error_state = 1; // this probably crashes calling program
// if (core_verification != 1)
//		return core_verification;


 if (!verify_target_core(calling_core, target_core_index, &target_core))
		return 0;


// now assume target_core is valid (or verify_target_core() should have returned 0)
	return call_core_method(calling_core, target_core, call_value, &stack_parameters [1]);

}




// checks whether calling_core has access to the core referred to in element target_core_index of calling_core's process memory.
// returns:
//  1 if yes
//  -1 if no but calling_core can be told that the target was just destroyed
//  -2 otherwise (e.g. target core out of range)
//  -3 if there was an error (e.g. invalid target_core_index)
// *** Update: for now just returns 1 if visible, 0 otherwise (error is also 0).
// also sets *target_core (although this is only guaranteed valid and correct if the function returns 1).
// is called by both external core methods and external member methods
int verify_target_core(struct core_struct* calling_core, int target_core_index, struct core_struct** target_core)
{

/*

This is currently not possible

	if (calling_core == NULL)
	{
// method was called by system program or equivalent. This means there are no further checks on whether the calling core has access to the target core.
// it also means that target_core_index is the direct index of the target core in the w.core array:
 	if (target_core_index < 0
 		|| target_core_index >= w.max_cores)
	 {
//		 fpr("\nexternal core (called by system member method call) target index %i (in world core array) out of bounds", target_core_index);
 		if (w.debug_mode)
	 		print_method_error("invalid external core index", 1, target_core_index);
 		return -3;
	 }
		*target_core = &w.core[target_core_index];
		if ((*target_core)->exists == 0
			&& (*target_core)->destroyed_timestamp < w.world_time - DEALLOCATE_COUNTER)
			return -1;
		return 1;
	}
*/
// TO DO: need to cache references to an external core to avoid having to do all of this checking every single time an external core method is called!!
//  - could apply an instruction cost to non-cached calls as well.

// method was called by a core.
// this means that target_core_index is the index of the target core in the calling core's process memory:

   if (target_core_index < 0
	   || target_core_index >= PROCESS_MEMORY_SIZE)
   {
    if (target_core_index == -1)
			 {
 				*target_core = calling_core; // special case - index -1 means self.
				 return 1;
			 }
  		if (w.debug_mode)
	  		print_method_error("invalid external core index", 1, target_core_index);
//    fpr("\nexternal core (called by core %i) target index %i (in process memory) out of bounds", calling_core->index, target_core_index);
	   return 0;//-3;
   }


			if (calling_core->process_memory [target_core_index] == -1)
			 return 0;//-3; // process memory entry is empty



   int target_core_world_index = calling_core->process_memory [target_core_index];
// should be able to assume that any value in process_memory is within bounds.
// but let's confirm this anyway:
#ifdef SANITY_CHECK
   if (target_core_world_index < 0 || target_core_world_index >= w.max_cores)
			{
				fpr("\nError: g_method_core.c: verify_target_core(): target_core_world_index out of bounds (%i)", target_core_world_index);
				error_call();
			}
#endif


// now we know that the target core is valid (whether it exists or not)
			*target_core = &w.core[target_core_world_index]; // although this might not actually be the target (if its timestamp is different) so be sure to check this!

// for friendly targets we can assume visibility as long as the target exists:
			if ((*target_core)->player_index == calling_core->player_index)
			{
    if ((*target_core)->exists == 0
					|| calling_core->process_memory_timestamp [target_core_index] != (*target_core)->created_timestamp)
				 return 0;//-1; // we can tell calling_core that its target has been destroyed

				return 1;
			}



		 if (calling_core->process_memory_timestamp [target_core_index] != (*target_core)->created_timestamp)
			{
				return 0;//-2; // the target core has ceased to exist and been replaced by another core using target_core_index, but calling_core doesn't know this.
			}



// If this function has been called by the same calling core in the same execution,
//  we can re-use the result of the earlier call.
			if ((*target_core)->visibility_checked_by_core_index == calling_core->index
				&& (*target_core)->visibility_checked_timestamp == w.world_time)
				return (*target_core)->visibility_check_result;



// No? so set up the pre-check values for the next call:
   (*target_core)->visibility_checked_by_core_index = calling_core->index;
   (*target_core)->visibility_checked_timestamp = w.world_time;
   (*target_core)->visibility_check_result = 0; // is set to 1 below if visibility check succeeds



   if ((*target_core)->exists == 0)
				return 0;//-2; // calling core is not informed that target has been destroyed unless its destruction is visible


/*

- to avoid having to do this check each time a core method is used, and providing ambiguous results in some case,
  detection of recently destroyed cores has been moved to the target_destroyed() standard method

			if ((*target_core)->exists == 0
    && (*target_core)->destroyed_timestamp >= w.world_time - DEALLOCATE_COUNTER
				&& w.vision_area[calling_core->player_index]
				                [w.proc[(*target_core)->process_index].block_position.x]
										          [w.proc[(*target_core)->process_index].block_position.y].vision_time > w.world_time - VISION_AREA_VISIBLE_TIME)
   {
    (*target_core)->visibility_check_result = -1;
    return 0;//-1; // indicates that target is a recently destroyed core that was destroyed in a location visible to calling core, so the calling core knows it was destroyed.
   }

   if ((*target_core)->exists == 0)
				return 0;//-2; // calling core is not informed that target has been destroyed unless its destruction is visible
*/
/*
			if ((*target_core)->core_position.x < calling_core->core_position.x - calling_core->scan_range_fixed
    || (*target_core)->core_position.x > calling_core->core_position.x + calling_core->scan_range_fixed
    || (*target_core)->core_position.y < calling_core->core_position.y - calling_core->scan_range_fixed
    || (*target_core)->core_position.y > calling_core->core_position.y + calling_core->scan_range_fixed)
    return -2; // target is out of range*/

   if (w.vision_area[calling_core->player_index]
				                [w.proc[(*target_core)->process_index].block_position.x]
										          [w.proc[(*target_core)->process_index].block_position.y].vision_time < w.world_time - VISION_AREA_VISIBLE_TIME)
				return 0;//-2; // target not currently visible



// *target_core = &w.core[target_core_world_index];

 (*target_core)->visibility_check_result = 1; // other visibility check values set above
	return 1;

}

// simplified version of verify_target_core that checks whether target core is friendly
//  (for methods that only work on friendly targets, e.g. messaging)
// since only friendly targets are found, we can skip all of the visibility stuff.
int verify_friendly_target_core(struct core_struct* calling_core, int target_core_index, struct core_struct** target_core)
{

   int target_core_world_index;

   if (target_core_index == -1)
			{
				target_core_world_index = calling_core->index; // special case - index -1 means self.
				return 1;
			}

   if (target_core_index < 0
	   || target_core_index >= PROCESS_MEMORY_SIZE)
   {
  		if (w.debug_mode)
	  		print_method_error("invalid external core index", 1, target_core_index);
	   return 0;
   }

			if (calling_core->process_memory [target_core_index] == -1)
			 return 0;

   target_core_world_index = calling_core->process_memory [target_core_index];

#ifdef SANITY_CHECK
   if (target_core_world_index < 0 || target_core_world_index >= w.max_cores)
			{
				fpr("\nError: g_method_core.c: verify_target_core(): target_core_world_index out of bounds (%i)", target_core_world_index);
				error_call();
			}
#endif

		 if (calling_core->process_memory_timestamp [target_core_index] != w.core[target_core_world_index].created_timestamp)
				return 0; // the target core has ceased to exist and been replaced by another core using target_core_index, but calling_core doesn't know this.

// now we know that the target core exists (or is being deallocated)
			*target_core = &w.core[target_core_world_index];

			if ((*target_core)->exists == 0
				|| (*target_core)->player_index != calling_core->player_index)
   {
    return 0;
   }

	return 1;

}



/*


Member methods


*/



#define MMETHOD_CALL_PARAMETERS 6

struct mmethod_call_type_struct mmethod_call_type [MMETHOD_CALL_TYPES] =
{
// {int parameters},
	{0, KEYWORD_MMETHOD_GET_COMPONENT_X}, // *MMETHOD_CALL_GET_COMPONENT_X
	{0, KEYWORD_MMETHOD_GET_COMPONENT_Y}, // *MMETHOD_CALL_GET_COMPONENT_Y
	{0, KEYWORD_MMETHOD_COMPONENT_EXISTS}, // *MMETHOD_CALL_COMPONENT_EXISTS
	{0, KEYWORD_MMETHOD_GET_INTEGRITY}, // *MMETHOD_CALL_GET_INTEGRITY
	{0, KEYWORD_MMETHOD_GET_INTEGRITY_MAX}, // *MMETHOD_CALL_GET_INTEGRITY_MAX
	{0, KEYWORD_MMETHOD_GET_COMPONENT_HIT}, // *MMETHOD_CALL_GET_COMPONENT_HIT
	{1, KEYWORD_MMETHOD_GET_COMPONENT_HIT_SOURCE}, // *MMETHOD_CALL_GET_COMPONENT_HIT_SOURCE

//	{0}, // MMETHOD_CALL_GET_MEMBER_SHAPE
//	{0}, // MMETHOD_CALL_GET_MEMBER_INTEGRITY

// note that since member might not be a member of the calling core, all member methods should be read only

};

// core may be the calling core, but might not be.
// core must be valid (and not NULL), though.
// member_world_proc_index must be valid and the member must exist
s16b call_member_method(struct core_struct* calling_core, struct core_struct* called_core, int member_world_proc_index, int call_value, s16b* stack_parameters)
{

 switch(call_value)
 {
	 case MMETHOD_CALL_GET_COMPONENT_X:
	 	return al_fixtoi(w.proc[member_world_proc_index].position.x);
	 case MMETHOD_CALL_GET_COMPONENT_Y:
	 	return al_fixtoi(w.proc[member_world_proc_index].position.y);
	 case MMETHOD_CALL_COMPONENT_EXISTS:
			return 1; // would have already returned 0 otherwise
		case MMETHOD_CALL_GET_INTEGRITY:
			return w.proc[member_world_proc_index].hp;
		case MMETHOD_CALL_GET_INTEGRITY_MAX:
			return w.proc[member_world_proc_index].hp_max;
		case MMETHOD_CALL_GET_COMPONENT_HIT:
			if (w.proc[member_world_proc_index].component_hit_time > w.world_time - EXECUTION_COUNT)
				return 1;
			return 0;
		case MMETHOD_CALL_GET_COMPONENT_HIT_SOURCE:
			if (w.proc[member_world_proc_index].component_hit_time > w.world_time - EXECUTION_COUNT)
			{
// remember: can't assume that the proc is a component of calling_core
				if (stack_parameters [0] >= 0
					&& stack_parameters [0] < PROCESS_MEMORY_SIZE)
				{
					calling_core->process_memory [stack_parameters [0]] = w.proc[member_world_proc_index].component_hit_source_index;
					calling_core->process_memory_timestamp	[stack_parameters [0]] = w.proc[member_world_proc_index].component_hit_source_timestamp;
				}
				return 1;
			}
			return 0;
 }

 vmstate.error_state = 1;
 return 0;

}


// assumes calling_core is valid and not NULL.
// doesn't assume anything about call_value.
s16b call_self_member_method(struct core_struct* calling_core, int call_value)
{

	s16b stack_parameters [MMETHOD_CALL_PARAMETERS];

	if (call_value < 0
		|| call_value >= MMETHOD_CALL_TYPES)
	{
//		fpr("\nself core call value %i out of bounds", call_value);
		if (w.debug_mode)
 		print_method_error("invalid self component method call type", 1, call_value);
  vmstate.error_state = 1;
		return 0;
	}

	if (!pull_values_from_stack(stack_parameters, mmethod_call_type[call_value].parameters + 1)) // + 1 is space for the member index
	{
		if (w.debug_mode)
 		print_method_error("self component call stack error", 0, 0);
  vmstate.error_state = 1;
		return 0;
	}

 int group_member_index = stack_parameters [0];

 if (group_member_index < 0
	 || group_member_index >= GROUP_MAX_MEMBERS)
	{
		if (w.debug_mode)
 		print_method_error("self component index error", 1, group_member_index);
  vmstate.error_state = 1;
		return 0;
	}

	if (calling_core->group_member[group_member_index].exists == 0)
	 return 0; // not an error

	return call_member_method(calling_core, calling_core, calling_core->group_member[group_member_index].index, call_value, &stack_parameters [1]);

}



// assumes calling_core is either valid or NULL.
// If calling_core is NULL, assumes that the system program is calling the method (may not be implemented yet)
// does not assume anything about call_value.
s16b call_extern_member_method(struct core_struct* calling_core, int call_value)
{

	struct core_struct* target_core;

	s16b stack_parameters [MMETHOD_CALL_PARAMETERS];

	if (call_value < 0
		|| call_value >= MMETHOD_CALL_TYPES)
	{
		if (w.debug_mode)
 		print_method_error("invalid external component method call type", 1, call_value);
  vmstate.error_state = 1;
		return 0;
	}

	if (!pull_values_from_stack(stack_parameters, mmethod_call_type[call_value].parameters + 2)) // Note + 1 is for core index and member index
	{
		if (w.debug_mode)
 		print_method_error("external component call stack error", 0, 0);
  vmstate.error_state = 1;
		return 0;
	}

	int target_core_index = stack_parameters [0];

// int core_verification = verify_target_core(calling_core, target_core_index, &target_core);
// if (core_verification != 1)
//		return core_verification;

 if (!verify_target_core(calling_core, target_core_index, &target_core))
		return 0;

 int group_member_index = stack_parameters [1];

 if (group_member_index < 0
	 || group_member_index >= GROUP_MAX_MEMBERS)
	{
		if (w.debug_mode)
 		print_method_error("self component index error", 1, group_member_index);
  vmstate.error_state = 1;
		return 0;
	}

	if (target_core->group_member[group_member_index].exists == 0)
	 return 0; // not an error

// shouldn't need to verify target_core->group_member[group_member_index].index (the group member's index in the w.proc[] array) here.

	return call_member_method(calling_core, target_core, target_core->group_member[group_member_index].index, call_value, &stack_parameters [2]);

}




