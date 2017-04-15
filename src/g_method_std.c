/*

g_method_core.c

Functions for calls to built-in (core) methods (i.e. ones that don't require an object)


*/

#include <allegro5/allegro.h>

#include <stdio.h>
#include <math.h>
#include "stdint.h"

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
#include "g_method_core.h"
#include "g_method_misc.h"
#include "g_cloud.h"
#include "g_proc_new.h"
#include "m_globvars.h"
#include "m_maths.h"
#include "t_template.h"
#include "x_sound.h"
#include "g_command.h"
#include "h_story.h"

#include "i_console.h"
#include "i_disp_in.h"
#include "i_background.h"

#include "v_interp.h"

#include "g_method_std.h"
#include "c_keywords.h"

extern struct view_struct view; // TO DO: think about putting a pointer to this in the worldstruct instead of externing it
extern struct control_struct control; // defined in i_input.c. Used here for client process methods.
extern struct game_struct game;
extern struct vmstate_struct vmstate; // defined in v_interp.c
extern struct command_struct command;
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];

#define SMETHOD_CALL_PARAMETERS 6

struct smethod_call_type_struct smethod_call_type [SMETHOD_CALL_TYPES] =
{
// {int parameters},
	{3, KEYWORD_SMETHOD_SCAN_FOR_THREAT}, // *SMETHOD_CALL_SCAN_FOR_THREAT (x_offset, y_offset, process memory address)
 {3, KEYWORD_SMETHOD_CHECK_POINT}, // *SMETHOD_CALL_CHECK_POINT (x, y, process memory address)
 {2, KEYWORD_SMETHOD_CHECK_XY_VISIBLE}, // *SMETHOD_CALL_CHECK_XY_VISIBLE (x, y)
 {0, KEYWORD_SMETHOD_GET_COMMAND_TYPE}, // *SMETHOD_CALL_GET_COMMAND_TYPE,
 {0, KEYWORD_SMETHOD_GET_COMMAND_X}, // *SMETHOD_CALL_GET_COMMAND_X,
 {0, KEYWORD_SMETHOD_GET_COMMAND_Y}, // *SMETHOD_CALL_GET_COMMAND_Y,
 {0, KEYWORD_SMETHOD_GET_COMMAND_NUMBER}, // -SMETHOD_CALL_GET_COMMAND_NUMBER,
 {0, KEYWORD_SMETHOD_GET_COMMAND_CTRL}, // *SMETHOD_CALL_GET_COMMAND_CTRL,
 {0, KEYWORD_SMETHOD_GET_COMMANDS}, // *SMETHOD_CALL_GET_COMMANDS,
 {0, KEYWORD_SMETHOD_CLEAR_COMMAND}, // *SMETHOD_CALL_CLEAR_COMMAND,
 {0, KEYWORD_SMETHOD_CLEAR_ALL_COMMANDS}, // *SMETHOD_CALL_CLEAR_ALL_COMMANDS,
 {1, KEYWORD_SMETHOD_GET_COMMAND_TARGET}, // *SMETHOD_CALL_GET_COMMAND_TARGET, (process memory address)
 {0, KEYWORD_SMETHOD_GET_COMMAND_TARGET_COMPONENT}, // *SMETHOD_CALL_GET_COMMAND_TARGET_COMPONENT, (no process memory involvement; this just returns the member index (or -1 if the member no longer exists))
 {0, KEYWORD_SMETHOD_CHECK_NEW_COMMAND}, // *SMETHOD_CALL_CHECK_NEW_COMMAND
 {1, KEYWORD_SMETHOD_BUILD_FROM_QUEUE}, // *SMETHOD_CALL_BUILD_FROM_QUEUE,
 {0, KEYWORD_SMETHOD_CHECK_BUILD_QUEUE}, // *SMETHOD_CALL_CHECK_BUILD_QUEUE,
 {0, KEYWORD_SMETHOD_CHECK_BUILD_QUEUE_FRONT}, // *SMETHOD_CALL_CHECK_BUILD_QUEUE_FRONT,
 {6, KEYWORD_SMETHOD_ADD_TO_BUILD_QUEUE}, // *SMETHOD_CALL_ADD_TO_BUILD_QUEUE, (template_index, x, y, angle, back_or_front, repeat)
 {0, KEYWORD_SMETHOD_CANCEL_BUILD_QUEUE}, // *SMETHOD_CALL_CANCEL_BUILD_QUEUE,
 {0, KEYWORD_SMETHOD_BUILD_QUEUE_GET_TEMPLATE}, // *SMETHOD_CALL_BUILD_QUEUE_GET_TEMPLATE,
 {0, KEYWORD_SMETHOD_BUILD_QUEUE_GET_X}, // *SMETHOD_CALL_BUILD_QUEUE_GET_X,
 {0, KEYWORD_SMETHOD_BUILD_QUEUE_GET_Y}, // *SMETHOD_CALL_BUILD_QUEUE_GET_Y,
 {0, KEYWORD_SMETHOD_BUILD_QUEUE_GET_ANGLE}, // *SMETHOD_CALL_BUILD_QUEUE_GET_ANGLE,



/*
 {0}, // *SMETHOD_CALL_CHECK_NEW_BUILD_COMMAND
 {0}, // *SMETHOD_CALL_CHECK_BUILD_COMMAND
 {0}, // *SMETHOD_CALL_GET_BUILD_COMMAND_X
 {0}, // *SMETHOD_CALL_GET_BUILD_COMMAND_Y
 {0}, // *SMETHOD_CALL_GET_BUILD_COMMAND_ANGLE
 {0}, // *SMETHOD_CALL_GET_BUILD_COMMAND_TEMPLATE
 {0}, // *SMETHOD_CALL_GET_BUILD_COMMAND_CTRL
 {0}, // *SMETHOD_CALL_CLEAR_BUILD_COMMAND
 {0}, // *SMETHOD_CALL_CLEAR_ALL_BUILD_COMMANDS*/
// {0}, // SMETHOD_CALL_GET_INTERFACE_STRENGTH *** this should be a core method to allow a core to query it for other cores
// {0}, // SMETHOD_CALL_GET_INTERFACE_CAPACITY ** same
 {1, KEYWORD_SMETHOD_CHARGE_INTERFACE}, // *SMETHOD_CALL_CHARGE_INTERFACE * not this one though
 {1, KEYWORD_SMETHOD_SET_INTERFACE_GENERAL}, // *SMETHOD_CALL_SET_INTERFACE_GENERAL (0 or 1)
// {0}, // SMETHOD_CALL_SPARE_POWER_TO_INTERFACE
 {0, KEYWORD_SMETHOD_CHARGE_INTERFACE_MAX}, // *SMETHOD_CALL_CHARGE_INTERFACE_MAX

 {0, KEYWORD_SMETHOD_CHECK_SELECTED}, // *SMETHOD_CALL_CHECK_SELECTED
 {0, KEYWORD_SMETHOD_CHECK_SELECTED_SINGLE}, // *SMETHOD_CALL_CHECK_SELECTED_SINGLE
 {0, KEYWORD_SMETHOD_GET_AVAILABLE_DATA}, // *SMETHOD_CALL_GET_AVAILABLE_DATA
 {0, KEYWORD_SMETHOD_SEARCH_FOR_WELL}, // *SMETHOD_CALL_SEARCH_FOR_WELL
 {0, KEYWORD_SMETHOD_GET_WELL_X}, // *SMETHOD_CALL_GET_WELL_X
 {0, KEYWORD_SMETHOD_GET_WELL_Y}, // *SMETHOD_CALL_GET_WELL_Y
 {0, KEYWORD_SMETHOD_GET_WELL_DATA}, // *SMETHOD_CALL_GET_WELL_DATA
 {0, KEYWORD_SMETHOD_GET_DATA_STORED}, // *SMETHOD_CALL_GET_DATA_STORED
 {0, KEYWORD_SMETHOD_GET_DATA_CAPACITY}, // *SMETHOD_CALL_GET_DATA_CAPACITY
 {7, KEYWORD_SMETHOD_SCAN_SINGLE}, // *SMETHOD_CALL_SCAN_SINGLE // (x_offset, y_offset, target memory, accept_or_require_friendly, components_min, components_max, scan_bitfield)
 {8, KEYWORD_SMETHOD_SCAN_MULTI}, // *SMETHOD_CALL_SCAN_MULTI // (x_offset, y_offset, target memory, number of targets, accept_or_require_friendly, components_min, components_max, scan_bitfield)
// {6}, // SMETHOD_CALL_SCAN_FOR_TEMPLATE // (x_offset, y_offset, target memory, number of targets, user_index, template_index)
 {0, KEYWORD_SMETHOD_GET_POWER_CAPACITY}, // *SMETHOD_CALL_GET_POWER_CAPACITY,
 {0, KEYWORD_SMETHOD_GET_POWER_USED}, // *SMETHOD_CALL_GET_POWER_USED,
 {0, KEYWORD_SMETHOD_GET_POWER_LEFT}, // *SMETHOD_CALL_GET_POWER_LEFT,
 {0, KEYWORD_SMETHOD_GET_INSTRUCTIONS_LEFT}, // *SMETHOD_CALL_GET_INSTRUCTIONS_LEFT,
// {0}, // SMETHOD_CALL_GET_POWER_USED_ACTUAL,
// {0}, // SMETHOD_CALL_GET_POWER_LEFT_ACTUAL,
// {0}, // SMETHOD_CALL_GET_STRESS,
// {0}, // SMETHOD_CALL_GET_STRESS_PERCENT,
 {1, KEYWORD_SMETHOD_SET_DEBUG_MODE}, // *SMETHOD_CALL_SET_DEBUG_MODE,
 {-2, KEYWORD_SMETHOD_TRANSMIT}, // *SMETHOD_CALL_TRANSMIT, (target, priority, <message...>) -3 means a variable number of parameters, with a minimum of 3
#define TRANSMIT_PARAMETERS 2

 {-3, KEYWORD_SMETHOD_BROADCAST}, // *SMETHOD_CALL_BROADCAST, (range, channel, priority, <message...>)
#define BROADCAST_PARAMETERS 3
 {-3, KEYWORD_SMETHOD_TRANSMIT_TARGET}, // *SMETHOD_CALL_TRANSMIT_TARGET, (target_of_transmit, priority, target_to_transmit, <message...>) -4 means a variable number of parameters, with a minimum of 4
#define TRANSMIT_TARGET_PARAMETERS 3
 {-4, KEYWORD_SMETHOD_BROADCAST_TARGET}, // *SMETHOD_CALL_BROADCAST_TARGET, (range, channel, priority, target_to_transmit, <message...>)
#define BROADCAST_TARGET_PARAMETERS 4
 {0, KEYWORD_SMETHOD_CHECK_MESSAGES}, // *SMETHOD_CALL_CHECK_MESSAGES,
 {0, KEYWORD_SMETHOD_GET_MESSAGE_TYPE}, // *SMETHOD_CALL_GET_MESSAGE_TYPE,
 {0, KEYWORD_SMETHOD_GET_MESSAGE_CHANNEL}, // *SMETHOD_CALL_GET_MESSAGE_CHANNEL,
 {1, KEYWORD_SMETHOD_GET_MESSAGE_SOURCE}, // *SMETHOD_CALL_GET_MESSAGE_SOURCE, (target memory index)
 {0, KEYWORD_SMETHOD_GET_MESSAGE_X}, // *SMETHOD_CALL_GET_MESSAGE_X,
 {0, KEYWORD_SMETHOD_GET_MESSAGE_Y}, // *SMETHOD_CALL_GET_MESSAGE_Y,
 {1, KEYWORD_SMETHOD_GET_MESSAGE_TARGET}, // *SMETHOD_CALL_GET_MESSAGE_TARGET, (target memory index)
 {0, KEYWORD_SMETHOD_GET_MESSAGE_PRIORITY}, // *SMETHOD_CALL_GET_MESSAGE_PRIORITY,
 {0, KEYWORD_SMETHOD_READ_MESSAGE}, // *SMETHOD_CALL_READ_MESSAGE,
 {0, KEYWORD_SMETHOD_NEXT_MESSAGE}, // *SMETHOD_CALL_NEXT_MESSAGE,
 {1, KEYWORD_SMETHOD_IGNORE_CHANNEL}, // *SMETHOD_CALL_IGNORE_CHANNEL,
 {1, KEYWORD_SMETHOD_LISTEN_CHANNEL}, // *SMETHOD_CALL_LISTEN_CHANNEL,
 {0, KEYWORD_SMETHOD_IGNORE_ALL_CHANNELS}, // *SMETHOD_CALL_IGNORE_ALL_CHANNELS,
 {1, KEYWORD_SMETHOD_COPY_COMMANDS}, // *SMETHOD_CALL_COPY_COMMANDS, (target)
 {8, KEYWORD_SMETHOD_GIVE_COMMAND}, // *SMETHOD_CALL_GIVE_COMMAND, (target_index, command_type, x, y, command_target, component, queued, control)
 {8, KEYWORD_SMETHOD_GIVE_BUILD_COMMAND}, // *SMETHOD_CALL_GIVE_BUILD_COMMAND, (target_index, template, x, y, angle, back_or_front, repeat, queued)

 {2, KEYWORD_SMETHOD_CHECK_BUILD_RANGE}, // *SMETHOD_CALL_CHECK_BUILD_RANGE, (x, y)
 {0, KEYWORD_SMETHOD_REPAIR_SELF}, // *SMETHOD_CALL_REPAIR_SELF,
 {0, KEYWORD_SMETHOD_RESTORE_SELF}, // *SMETHOD_CALL_RESTORE_SELF,
 {1, KEYWORD_SMETHOD_REPAIR_OTHER}, // *SMETHOD_CALL_REPAIR_OTHER, (target memory)
 {2, KEYWORD_SMETHOD_REPAIR_SCAN}, // *SMETHOD_CALL_REPAIR_SCAN, (x_offset, y_offset)
 {1, KEYWORD_SMETHOD_RESTORE_OTHER}, // *SMETHOD_CALL_RESTORE_OTHER, (target memory)
 {2, KEYWORD_SMETHOD_RESTORE_SCAN}, // *SMETHOD_CALL_RESTORE_SCAN, (x_offset, y_offset)

 {5, KEYWORD_SMETHOD_BUILD_PROCESS}, // *SMETHOD_CALL_BUILD_PROCESS (template_index, x_offset, y_offset, angle, target address);
// {1}, // *SMETHOD_CALL_BUILD_AS_COMMANDED (target address)
 {1, KEYWORD_SMETHOD_BUILD_REPEAT}, // *SMETHOD_CALL_BUILD_REPEAT (target address)
 {1, KEYWORD_SMETHOD_GET_TEMPLATE_COST}, // *SMETHOD_CALL_GET_TEMPLATE_COST (template index)
 {1, KEYWORD_SMETHOD_RANDOM}, // *SMETHOD_CALL_RANDOM (mod)

// these methods, which are a kind of sense of touch for processes, could be core methods but that would cause problems with synchronisation (as they could be called at any point during target's cycle, while the
//  values in core_struct are reset at the start of each cycle)
 {1, KEYWORD_SMETHOD_CHECK_CONTACT}, // *SMETHOD_CALL_CHECK_CONTACT (target memory)
 {0, KEYWORD_SMETHOD_GET_DAMAGE}, // *SMETHOD_CALL_GET_DAMAGE
 {1, KEYWORD_SMETHOD_GET_DAMAGE_SOURCE}, // *SMETHOD_CALL_GET_DAMAGE_SOURCE (target memory)
 {2, KEYWORD_SMETHOD_DISTANCE_FROM_XY}, // *SMETHOD_CALL_DISTANCE_FROM_XY,
// {2}, // SMETHOD_CALL_DISTANCE_XY_HYPOT,
 {3, KEYWORD_SMETHOD_DISTANCE_LESS}, // *SMETHOD_CALL_DISTANCE_FROM_XY_LESS,
 {3, KEYWORD_SMETHOD_DISTANCE_MORE}, // *SMETHOD_CALL_DISTANCE_FROM_XY_MORE,
 {2, KEYWORD_SMETHOD_DISTANCE_XY}, // *SMETHOD_CALL_DISTANCE_XY,

// methods for measuring distance to target are done as core methods

 {1, KEYWORD_SMETHOD_TARGET_CLEAR}, // *SMETHOD_CALL_TARGET_CLEAR, (target_index)
 {2, KEYWORD_SMETHOD_TARGET_COMPARE}, // *SMETHOD_CALL_TARGET_COMPARE, (target1, target2)
 {2, KEYWORD_SMETHOD_TARGET_COPY}, // *SMETHOD_CALL_TARGET_COPY, (target_dest, target_source)
 {1, KEYWORD_SMETHOD_TARGET_DESTROYED}, // *SMETHOD_CALL_TARGET_DESTROYED, (target_index)

 {1, KEYWORD_SMETHOD_ATTACK_MODE}, // *SMETHOD_CALL_ATTACK_MODE, (mode)


 {0, KEYWORD_SMETHOD_GET_PROCESS_COUNT}, // *SMETHOD_CALL_GET_PROCESS_COUNT,
 {0, KEYWORD_SMETHOD_GET_PROCESSES_MAX}, // *SMETHOD_CALL_GET_PROCESSES_MAX,
 {0, KEYWORD_SMETHOD_GET_PROCESSES_UNUSED}, // *SMETHOD_CALL_GET_PROCESSES_UNUSED,
 {0, KEYWORD_SMETHOD_GET_COMPONENT_COUNT}, // *SMETHOD_CALL_GET_COMPONENT_COUNT,
 {0, KEYWORD_SMETHOD_GET_COMPONENTS_MAX}, // *SMETHOD_CALL_GET_COMPONENTS_MAX,
 {0, KEYWORD_SMETHOD_GET_COMPONENTS_UNUSED}, // *SMETHOD_CALL_GET_COMPONENTS_UNUSED,

 {2, KEYWORD_SMETHOD_SPECIAL_AI}, // *SMETHOD_CALL_SPECIAL_AI,


/*
To implement:
- get_well_reserve()
- get_well_replenish();

- get_template_cost(template_index)
- get_template_recycle(template_index)


*/
};

static int build_call(struct core_struct* core, int build_template, al_fixed build_x, al_fixed build_y, int build_angle, int process_memory_address);
static void place_build_lines(struct core_struct* core, cart target_position);
static void place_build_line(struct proc_struct* building_proc, int object_index, cart target_position);
static void place_repair_line(struct proc_struct* repairing_proc, int object_index, struct proc_struct* target_proc);//cart target_position);

static void build_scanlist(struct core_struct* scanning_core);
static s16b check_point(struct core_struct* core, s16b* stack_parameters);
//static s16b scan_for_threat(struct core_struct* core, s16b* stack_parameters);
static s16b scan_single(struct core_struct* core, s16b* stack_parameters, int components_min, int components_max, s16b scan_bitfield, int accept_or_require_friendly);
static s16b scan_multi(struct core_struct* core, s16b* stack_parameters);
//static s16b scan_for_template(struct core_struct* core, s16b* stack_parameters);

static s16b charge_interface(struct core_struct* core, int charge_amount);
static s16b try_to_raise_general_interface(struct core_struct* core);

void find_nearby_well(struct core_struct* core);

static s16b repair_process(struct core_struct* target_core, struct core_struct* repairer);
static s16b repair_specific_component(struct core_struct* target_core, struct core_struct* repairer, int component_index, int repair_amount);
static s16b scan_repair(struct core_struct* core, s16b* stack_parameters, int repair_amount);
static s16b restore_specific_component(struct core_struct* target_core, int member_index);
static s16b restore_process(struct core_struct* target_core, struct core_struct* restorer);
static s16b scan_restore(struct core_struct* core, s16b* stack_parameters);

static int standard_method_uses_power(struct core_struct* core, int power_cost);
static void set_ongoing_power_cost_for_object_type(struct core_struct* core, int object_type1, int object_type2, int power_cost, timestamp power_cost_finish_time);

static s16b write_message(struct core_struct* target_core, int channel, int priority, int message_type, struct core_struct* source_core, int transmitted_target_core_index, timestamp transmitted_target_core_timestamp, int message_length, s16b* message);

static int find_first_build_queue_entry(int player_index, int core_index);

//#define POWER_COST_PACKET 10
#define POWER_COST_INTERFACE_CHARGE_1 2


// returns 1 if okay to continue, 0 if something happened that should cease program execution (not sure this is currently supported)
// variable_parameters is number of parameters, for a call_std_var call only
s16b call_std_method(struct core_struct* core, int call_value, int variable_parameters)
{

	s16b stack_parameters [SMETHOD_VARIABLE_PARAMS_MAX+2]; // SMETHOD_VARIABLE_PARAMS_MAX should be the most parameters any call will have
	int i;

	if (call_value < 0
		|| call_value >= SMETHOD_CALL_TYPES)
	{
		if (w.debug_mode)
 		print_method_error("invalid standard method call type", 1, call_value);
		return 0;
	}

// If needed, pull the parameters from the stack:
		if (smethod_call_type[call_value].parameters > 0
		&& !pull_values_from_stack(stack_parameters, smethod_call_type[call_value].parameters))
	{
		if (w.debug_mode)
 		print_method_error("standard method call stack error", 0, 0);
		return 0;
	}
	 else
		{
 		if (smethod_call_type[call_value].parameters < 0 // variable parameters
				&& !pull_values_from_stack(stack_parameters, variable_parameters))
			{
  		if (w.debug_mode)
 		  print_method_error("variable-parameter standard method call stack error", 0, 0);
		  return 0;
			}
		}

 vmstate.instructions_left -= 2; // default cost - some methods cost more

 switch(call_value)
 {
  case SMETHOD_CALL_SCAN_FOR_THREAT:
   return scan_single(core, stack_parameters, 0, 10000, 0, 0); // 0-10000 are min and max components. next 0 means accept any target signature. 0 means do not accept friendly targets.
//			return scan_for_threat(core, stack_parameters);
  case SMETHOD_CALL_SCAN_SINGLE: //  (x_offset, y_offset, target memory, accept_or_require_friendly, components_min, components_max, scan_bitfield)
   return scan_single(core, stack_parameters, stack_parameters [4], stack_parameters [5], stack_parameters [6], stack_parameters [3]); // 0xFFFF means accept whole bitfield (don't filter targets). 0 means do not accept friendly targets.

 // (x_offset, y_offset, target memory, accept_or_require_friendly, components_min, components_max, scan_bitfield)

//			return scan_single(core, stack_parameters);
  case SMETHOD_CALL_SCAN_MULTI:
			return scan_multi(core, stack_parameters);
//		case SMETHOD_CALL_SCAN_FOR_TEMPLATE:
//			return scan_for_template(core, stack_parameters);
		case SMETHOD_CALL_CHECK_POINT:
			return check_point(core, stack_parameters);
		case SMETHOD_CALL_CHECK_XY_VISIBLE:
			{
			 int check_block_x = stack_parameters [0] / BLOCK_SIZE_PIXELS;
			 int check_block_y = stack_parameters [1] / BLOCK_SIZE_PIXELS;
	   if (check_block_x < 1 || check_block_x >= w.blocks.x - 1
		   || check_block_y < 1 || check_block_y >= w.blocks.y - 1)
			   return 0;
			 if (w.vision_area[core->player_index][check_block_x][check_block_y].vision_time >= w.world_time - VISION_AREA_VISIBLE_TIME)
					return 1;
				return 0;
			}
	 case SMETHOD_CALL_GET_COMMAND_TYPE:
			return core->command_queue[0].type;
	 case SMETHOD_CALL_GET_COMMAND_X:
/*	 	if (core->command_queue[0].type == COM_NONE)
				return 0;

	 	if (core->command_queue[0].type == COM_TARGET)
			{
				if (core->command_queue[0].target_core != -1
					&&	w.core[core->command_queue[0].target_core].exists
					&& w.core[core->command_queue[0].target_core].created_timestamp == core->command_queue[0].target_core_created
					&& w.vision_area[core->player_index][w.proc[w.core[core->command_queue[0].target_core].process_index].block_position.x][w.proc[w.core[core->command_queue[0].target_core].process_index].block_position.y].vision_time >= w.world_time - VISION_AREA_VISIBLE_TIME)
//					&& check_proc_visible_to_user(w.core[core->command_queue[0].target_core].process_index))
				{
     int target_member_index = core->command_queue[0].target_member;
// if the targetted member no longer exists, target the core instead (TO DO: think about whether this is correct - could target parent component?)
     if (w.core[core->command_queue[0].target_core].group_member[target_member_index].exists == 0)
						target_member_index = 0;
				 return al_fixtoi(w.proc[w.core[core->command_queue[0].target_core].group_member[target_member_index].index].position.x);
				}
				  else
							return 0;
 - No - now it just returns the location when the command was issued
 - commands with no location should have set x/y to 0 when given
			}*/
// must be COM_LOCATION or COM_DATA_WELL
			return core->command_queue[0].x;
	 case SMETHOD_CALL_GET_COMMAND_Y:
/*	 	if (core->command_queue[0].type == COM_NONE)
				return 0;
	 	if (core->command_queue[0].type == COM_TARGET)
			{
// Because the target has been identified by a command, we don't need to worry about deallocation:
				if (core->command_queue[0].target_core != -1
					&&	w.core[core->command_queue[0].target_core].exists
					&& w.core[core->command_queue[0].target_core].created_timestamp == core->command_queue[0].target_core_created
					&& w.vision_area[core->player_index][w.proc[w.core[core->command_queue[0].target_core].process_index].block_position.x][w.proc[w.core[core->command_queue[0].target_core].process_index].block_position.y].vision_time >= w.world_time - VISION_AREA_VISIBLE_TIME)
//					&& check_proc_visible_to_user(w.core[core->command_queue[0].target_core].process_index))
				{
     int target_member_index = core->command_queue[0].target_member;
// if the targetted member no longer exists, target the core instead (TO DO: think about whether this is correct)
     if (w.core[core->command_queue[0].target_core].group_member[target_member_index].exists == 0)
						target_member_index = 0;
				 return al_fixtoi(w.proc[w.core[core->command_queue[0].target_core].group_member[target_member_index].index].position.y);
//				 return al_fixtoi(w.core[core->command_queue[0].target_core].core_position.y);
				}
				  else
							return 0;
			}
// must be COM_LOCATION or COM_DATA_WELL*/
			return core->command_queue[0].y;
		case SMETHOD_CALL_GET_COMMAND_NUMBER: // not currently supported
//	 	if (core->command_queue[0].type != COM_NUMBER)
				return 0;
//			return core->command_queue[0].x; // x is used for number
		case SMETHOD_CALL_GET_COMMAND_CTRL:
	 	if (core->command_queue[0].type == COM_NONE)
				return 0;
			return core->command_queue[0].control_pressed;
	 case SMETHOD_CALL_GET_COMMANDS:
	 	{
	 		int commands_in_queue = 0;
 	 	for (i = 0; i < COMMAND_QUEUE; i ++)
			 {
 				if (core->command_queue [i].type != COM_NONE)
						commands_in_queue++;
			 }
 	 return commands_in_queue;
	 	}
 	case SMETHOD_CALL_CLEAR_COMMAND:
	 	for (i = 1; i < COMMAND_QUEUE; i ++) // note for i = 1
		 {
		 	core->command_queue [i-1].type = core->command_queue [i].type;
		 	core->command_queue [i-1].x = core->command_queue [i].x;
		 	core->command_queue [i-1].y = core->command_queue [i].y;
		 	core->command_queue [i-1].target_core = core->command_queue [i].target_core;
		 	core->command_queue [i-1].target_core_created = core->command_queue [i].target_core_created;
		 	core->command_queue [i-1].target_member = core->command_queue [i].target_member;
		 	core->command_queue [i-1].control_pressed = core->command_queue [i].control_pressed;
		 }
	 	core->command_queue [COMMAND_QUEUE-1].type = COM_NONE;
   if (core->command_queue [0].type != COM_NONE)
				core->new_command = 1;
		 return 1;
 	case SMETHOD_CALL_CLEAR_ALL_COMMANDS:
	 	for (i = 0; i < COMMAND_QUEUE; i ++)
		 {
		 	core->command_queue [i].type = COM_NONE;
		 }
		 return 1;
		case SMETHOD_CALL_GET_COMMAND_TARGET:
// Because the target has been identified by a command, we don't need to worry about deallocation:
			if (core->command_queue[0].target_core != -1
				&&	w.core[core->command_queue[0].target_core].exists
				&& w.core[core->command_queue[0].target_core].created_timestamp == core->command_queue[0].target_core_created
				&& stack_parameters [0] >= 0
				&& stack_parameters [0] < PROCESS_MEMORY_SIZE)
				{
					core->process_memory [stack_parameters[0]] = core->command_queue[0].target_core;
					core->process_memory_timestamp [stack_parameters[0]] = core->command_queue[0].target_core_created;
					return 1;
				}
			return 0;
		case SMETHOD_CALL_GET_COMMAND_TARGET_COMPONENT:
// Because the target has been identified by a command, we don't need to worry about deallocation:
			if (core->command_queue[0].target_core != -1
				&&	w.core[core->command_queue[0].target_core].exists
				&& w.core[core->command_queue[0].target_core].created_timestamp == core->command_queue[0].target_core_created)
				return core->command_queue[0].target_member; // doesn't guarantee target member exists
//				&& core->group_member[core->command_queue[0].target_member].index != -1 // can happen if member destroyed
//				&& w.proc[core->group_member[core->command_queue[0].target_member].index].exists > 0) // probably not needed due to previous check
   return -1;
		case SMETHOD_CALL_CHECK_NEW_COMMAND:
			if (core->new_command == 1)
			{
				core->new_command = 0;
				return 1;
			}
			return 0;
		case SMETHOD_CALL_BUILD_FROM_QUEUE: // (target_index)
			{
			if (!w.player[core->player_index].build_queue[0].active
				|| w.player[core->player_index].build_queue[0].core_index != core->index)
				return 0;

		 if (w.player[core->player_index].data < templ[core->player_index][w.player[core->player_index].build_queue[0].template_index].data_cost)
		 {
		 	 w.player[core->player_index].build_queue_fail_reason	= BUILD_FAIL_DATA;
		 	 return BUILD_FAIL_DATA;
		 }

    int build_result = build_call(core,
																							w.player[core->player_index].build_queue[0].template_index, // template index
																							al_itofix(w.player[core->player_index].build_queue[0].build_x), // build_x
																							al_itofix(w.player[core->player_index].build_queue[0].build_y), // build_y
																							w.player[core->player_index].build_queue[0].angle, // build_angle
																							stack_parameters [0]); // process memory address - build_call will bounds-check it

    w.player[core->player_index].build_queue_fail_reason = build_result; // BUILD_SUCCESS means no failure

				if (build_result == BUILD_SUCCESS)
				{
					if (w.player[core->player_index].build_queue[0].repeat)
						requeue_repeat_build(core->player_index);
					  else
								build_queue_next(core->player_index, 1);
				}
				return build_result;

			}
		case SMETHOD_CALL_CHECK_BUILD_QUEUE:
			{
				int entries_found = 0;

				for (i = 0; i < BUILD_QUEUE_LENGTH; i ++)
				{
			  if (!w.player[core->player_index].build_queue[i].active)
						break;
					if (w.player[core->player_index].build_queue[i].core_index == core->index)
						entries_found ++;
				}
				return entries_found;
			}
			return 0;
		case SMETHOD_CALL_CHECK_BUILD_QUEUE_FRONT:
			if (w.player[core->player_index].build_queue[0].active
				&& w.player[core->player_index].build_queue[0].core_index == core->index)
			  return 1;
			return 0;
		case SMETHOD_CALL_ADD_TO_BUILD_QUEUE: //{6}, (template_index, x, y, angle, back_or_front, repeat)

// verify the parameters:
   if (stack_parameters [0] < 0
				|| stack_parameters [0] >= TEMPLATES_PER_PLAYER)
			{
				 if (w.debug_mode)
					 print_method_error("add_to_build_queue invalid template index", 1, stack_parameters [0]);
					return -1;
			}

   if (stack_parameters [1] < 255
				|| stack_parameters [1] >= w.w_pixels - 255)
			{
				 if (w.debug_mode)
					 print_method_error("add_to_build_queue invalid build_x", 1, stack_parameters [1]);
					return -1;
			}

   if (stack_parameters [2] < 255
				|| stack_parameters [2] >= w.h_pixels - 255)
			{
				 if (w.debug_mode)
					 print_method_error("add_to_build_queue invalid build_y", 1, stack_parameters [2]);
					return -1;
			}
// angle can be anything
// target_index can be anything
// back_or_front currently ignored
// repeat dealt with below

			return add_to_build_queue(core->player_index,
																						       core->index,
																						       stack_parameters [0], // template
																						       stack_parameters [1], // x
																						       stack_parameters [2], // y
																						       stack_parameters [3] & ANGLE_MASK,  // angle
																						       stack_parameters [4], // back_or_front
																						       (stack_parameters [5] != 0), // repeat
																						       1, // queue_for_this_core - maybe?
																						       0); // failure_message - maybe?
		case SMETHOD_CALL_CANCEL_BUILD_QUEUE:
			clear_build_queue_for_core(core->player_index, core->index);
			return 1; // should probably return the number removed from the queue
		case SMETHOD_CALL_BUILD_QUEUE_GET_TEMPLATE:
			{
				int queue_index = find_first_build_queue_entry(core->player_index, core->index);
				if (queue_index == -1)
					return -1;
				return w.player[core->player_index].build_queue[queue_index].template_index;
			}
		case SMETHOD_CALL_BUILD_QUEUE_GET_X:
			{
				int queue_index = find_first_build_queue_entry(core->player_index, core->index);
				if (queue_index == -1)
					return -1;
				return w.player[core->player_index].build_queue[queue_index].build_x;
			}
		case SMETHOD_CALL_BUILD_QUEUE_GET_Y:
			{
				int queue_index = find_first_build_queue_entry(core->player_index, core->index);
				if (queue_index == -1)
					return -1;
				return w.player[core->player_index].build_queue[queue_index].build_y;
			}
		case SMETHOD_CALL_BUILD_QUEUE_GET_ANGLE:
			{
				int queue_index = find_first_build_queue_entry(core->player_index, core->index);
				if (queue_index == -1)
					return -1;
				return w.player[core->player_index].build_queue[queue_index].angle;
			}

/*
		case SMETHOD_CALL_CHECK_NEW_BUILD_COMMAND:
			if (core->new_build_command == 1)
			{
				core->new_build_command = 0;
				return 1;
			}
			return 0;
		case SMETHOD_CALL_CHECK_BUILD_COMMAND:
			return core->build_command_queue [0].active;
		case SMETHOD_CALL_GET_BUILD_COMMAND_X:
			if (!core->build_command_queue[0].active)
				return 0;
			return core->build_command_queue[0].build_x;
		case SMETHOD_CALL_GET_BUILD_COMMAND_Y:
			if (!core->build_command_queue[0].active)
				return 0;
			return core->build_command_queue[0].build_y;
		case SMETHOD_CALL_GET_BUILD_COMMAND_ANGLE:
			if (!core->build_command_queue[0].active)
				return 0;
			return core->build_command_queue[0].build_angle;
		case SMETHOD_CALL_GET_BUILD_COMMAND_TEMPLATE:
			if (!core->build_command_queue[0].active)
				return 0;
			return core->build_command_queue[0].build_template;
		case SMETHOD_CALL_GET_BUILD_COMMAND_CTRL:
			if (!core->build_command_queue[0].active)
				return 0;
			return core->build_command_queue[0].build_command_ctrl;
		case SMETHOD_CALL_CLEAR_BUILD_COMMAND:
	 	for (i = 1; i < BUILD_COMMAND_QUEUE; i ++) // note for i = 1
		 {
		 	core->build_command_queue [i-1] = core->build_command_queue [i]; // struct assignment!
		 }
	 	core->build_command_queue [COMMAND_QUEUE-1].active = 0;
   if (core->build_command_queue [0].active != 0)
				core->new_build_command = 1;
		 return 1;
 	case SMETHOD_CALL_CLEAR_ALL_BUILD_COMMANDS:
	 	for (i = 0; i < BUILD_COMMAND_QUEUE; i ++)
		 {
		 	core->build_command_queue [i].active = 0;
		 }
		 return 1;*/
		case SMETHOD_CALL_CHARGE_INTERFACE: // note that calls to get information about interface are core methods
			return charge_interface(core, stack_parameters [0]);
/*		case SMETHOD_CALL_SPARE_POWER_TO_INTERFACE:
			{
// dumps power to interface.
//  - doesn't dump unused power that is predicted to be used
//  - doesn't dump power that could be used to reduce stress
			 int charge_power = core->power_capacity - (core->power_used + core->power_use_predicted);
			 charge_power -= core->stress * STRESS_REDUCTION_FACTOR;
			 if (charge_power <= 0)
					return 0;
			 return charge_interface(core, charge_power);
			}*/
//		case SMETHOD_CALL_ALL_POWER_TO_INTERFACE:
		case SMETHOD_CALL_CHARGE_INTERFACE_MAX:
			{
// like spare_power_to_interface() but ignores stress
//			 int charge_power = core->power_capacity - (core->power_used + core->power_use_predicted);
			 if (core->power_left <= 0)
					return 0;
			 return charge_interface(core, 5000); // 5000 is just an arbitrarily high value. It will be reduced to the actual maximum possible by charge_interface()
			}
		case SMETHOD_CALL_SET_INTERFACE_GENERAL:
//				if (core->interface_available == 0) - there's no real reason to fail this call even if it can't do anything. May be relevant if a process' components with interface are destroyed but it is subsequently repaired
			if (stack_parameters [0])
			{
				core->interface_control_status = 1;
				if (!core->interface_active)
				 return try_to_raise_general_interface(core);
				return 1;
			}
// must be 0:
			if (core->interface_control_status == 0)
				return 0; // already off - does nothing
			if (core->interface_active)
			{
				core->interface_control_status = 0;
				return lower_general_interface(core);
			}
			return 0;

		case SMETHOD_CALL_CHECK_SELECTED:
			if (core->selected != -1
				&& w.command_mode == COMMAND_MODE_COMMAND
				&&	core->player_index == game.user_player_index)
				return 1;
			return 0;
		case SMETHOD_CALL_CHECK_SELECTED_SINGLE:
			if (command.selected_core [0] == core->index
				&& command.selected_core [1] == SELECT_TERMINATE
				&& w.command_mode == COMMAND_MODE_COMMAND
				&&	core->player_index == game.user_player_index)
				return 1;
			return 0;
		case SMETHOD_CALL_GET_AVAILABLE_DATA:
			return w.player[core->player_index].data;
		case SMETHOD_CALL_SEARCH_FOR_WELL:
			if (core->number_of_harvest_objects + core->number_of_build_objects == 0)
				return -1; // must have at least one of either type to detect data wells
			if (vmstate.nearby_well_index == -2) // not yet calculated
				find_nearby_well(core); // updates vmstate.nearby_well_index
			if (vmstate.nearby_well_index == -1)
				return 0;
			return 1; // means there is a nearby well
		case SMETHOD_CALL_GET_WELL_X:
//			if (vmstate.nearby_well_index == -1)
//				fpr("\n core %i at %i,%i nwi==-1", core->index, al_fixtoi(core->core_position.x), al_fixtoi(core->core_position.y));
			if (core->number_of_harvest_objects + core->number_of_build_objects == 0)
			{
//				fpr("\n C core %i at %i,%i nwi==-1", core->index, al_fixtoi(core->core_position.x), al_fixtoi(core->core_position.y));
				return -1; // must have at least one of either type to detect data wells
			}
			if (vmstate.nearby_well_index == -2) // not yet calculated
				find_nearby_well(core); // updates vmstate.nearby_well_index
			if (vmstate.nearby_well_index == -1)
			{
//				fpr("\n B core %i at %i,%i nwi==-1", core->index, al_fixtoi(core->core_position.x), al_fixtoi(core->core_position.y));
				return -1;
			}
			return al_fixtoi(w.data_well[vmstate.nearby_well_index].position.x);
		case SMETHOD_CALL_GET_WELL_Y:
			if (core->number_of_harvest_objects + core->number_of_build_objects == 0)
				return -1; // must have at least one of either type to detect data wells
			if (vmstate.nearby_well_index == -2) // not yet calculated
				find_nearby_well(core); // updates vmstate.nearby_well_index
			if (vmstate.nearby_well_index == -1)
				return -1;
			return al_fixtoi(w.data_well[vmstate.nearby_well_index].position.y);
		case SMETHOD_CALL_GET_WELL_DATA:
			if (core->number_of_harvest_objects + core->number_of_build_objects == 0)
				return -1; // must have at least one of either type to detect data wells
			if (vmstate.nearby_well_index == -2) // not yet calculated
				find_nearby_well(core); // updates vmstate.nearby_well_index
			if (vmstate.nearby_well_index == -1)
				return -1;
			return w.data_well[vmstate.nearby_well_index].data;
		case SMETHOD_CALL_GET_DATA_STORED:
			return core->data_stored;
		case SMETHOD_CALL_GET_DATA_CAPACITY:
			return core->data_storage_capacity;
		case SMETHOD_CALL_GET_POWER_CAPACITY:
			return core->power_capacity;
		case SMETHOD_CALL_GET_POWER_USED:
			return core->power_capacity - core->power_left;
		case SMETHOD_CALL_GET_POWER_LEFT:
			return core->power_left;
		case SMETHOD_CALL_GET_INSTRUCTIONS_LEFT:
			return vmstate.instructions_left;
/*		case SMETHOD_CALL_GET_POWER_USED_ACTUAL:
			return core->power_used;
		case SMETHOD_CALL_GET_POWER_LEFT_ACTUAL:
			return core->power_capacity - core->power_used;
		case SMETHOD_CALL_GET_STRESS:
			return core->stress;
		case SMETHOD_CALL_GET_STRESS_PERCENT:
			return (core->stress * 100) / core->power_capacity;*/
		case SMETHOD_CALL_SET_DEBUG_MODE:
			w.debug_mode = (stack_parameters [0] != 0);
			return 0;
//SMETHOD_CALL_DISTANCE_FROM_XY,
//SMETHOD_CALL_DISTANCE_FROM_XY_HYPOT,
		case SMETHOD_CALL_TRANSMIT:
			{
// parameters: target index, channel, priority, messages... (number of messages is variable_parameters - 3)
    struct core_struct* transmit_target_core;
    if (verify_friendly_target_core(core, stack_parameters [0], &transmit_target_core) != 1)
					return 0;
//				if (stack_parameters [1] < 0 // check message channel
//				 || stack_parameters [1] >= CHANNELS)
//					return 0; // could write an error message
				if (stack_parameters [1] != 0 // check message priority
				 && stack_parameters [1] != 1)
					return 0; // could write an error message
				if (variable_parameters >= MESSAGE_LENGTH + TRANSMIT_PARAMETERS)
				{
					vmstate.error_state = 1; // this error would probably mess up the stack, so let's return
					print_method_error("too many parameters to transmit call", 0, 0); // really should catch this at compilation!
					return 0;
				}
//				if (transmit_target_core->listen_channel [stack_parameters [1]])
     return write_message(transmit_target_core, 0, stack_parameters [1], MESSAGE_TYPE_TRANSMIT, core, -1, 0, variable_parameters - TRANSMIT_PARAMETERS, &stack_parameters [TRANSMIT_PARAMETERS]);
				return 0;
			}
		case SMETHOD_CALL_TRANSMIT_TARGET: // TO DO: could probably combine with code for transmit()
			{
// parameters: target index, channel, priority, transmitted_target, messages... (number of messages is variable_parameters - 3)
    struct core_struct* transmit_target_core;
    if (verify_friendly_target_core(core, stack_parameters [0], &transmit_target_core) != 1)
					return 0;
//				if (stack_parameters [1] < 0 // check message channel
//				 || stack_parameters [1] >= CHANNELS)
//					return 0; // could write an error message
				if (stack_parameters [1] != 0 // check message priority
				 && stack_parameters [1] != 1)
					return 0; // could write an error message
				if (variable_parameters >= MESSAGE_LENGTH + TRANSMIT_TARGET_PARAMETERS)
				{
					vmstate.error_state = 1; // this error would probably mess up the stack, so let's return
					print_method_error("too many parameters to transmit_target call", 0, 0); // really should catch this at compilation!
					return 0;
				}
				int transmitted_target_index = stack_parameters [2];
				if (transmitted_target_index < 0
					|| transmitted_target_index >= PROCESS_MEMORY_SIZE)
//					|| core->process_memory [transmitted_target_index] == -1) // don't think this matters - should be okay to transmit null target
					return 0;
// shouldn't be necessary to check whether the transmitted_target actually exists
//				if (transmit_target_core->listen_channel [stack_parameters [1]])
     return write_message(transmit_target_core, 0, stack_parameters [1], MESSAGE_TYPE_TRANSMIT_TARGET, core, core->process_memory [transmitted_target_index], core->process_memory_timestamp [transmitted_target_index], variable_parameters - TRANSMIT_TARGET_PARAMETERS, &stack_parameters [TRANSMIT_TARGET_PARAMETERS]);
				return 0;
			}

		case SMETHOD_CALL_BROADCAST:
			{
// this should cost quite a few instructions, or even some power
				if (stack_parameters [1] < 0 // check message channel
				 || stack_parameters [1] >= CHANNELS)
					return 0; // could write an error message
				if (stack_parameters [2] != 0 // check message priority
				 && stack_parameters [2] != 1)
					return 0; // could write an error message
				al_fixed broadcast_range = al_itofix(stack_parameters [0]);
				if (broadcast_range <= 0) // check range
					broadcast_range = al_itofix(30000); // if range set to <= 0, it's treated as maximum range
				if (variable_parameters >= MESSAGE_LENGTH + BROADCAST_PARAMETERS)
				{
					vmstate.error_state = 1; // this error would probably mess up the stack, so let's return
					print_method_error("too many parameters to broadcast call", 0, 0); // really should catch this at compilation!
					return 0;
				}
				vmstate.instructions_left -= 64; // seems reasonable - this should be an expensive operation. Could cost more.
// additional power cost for broadcast?
    for (i = w.player[core->player_index].core_index_start; i < w.player[core->player_index].core_index_end; i ++)
				{
					if (w.core[i].exists > 0
						&& i != core->index
      && distance_oct_xyxy(core->core_position.x, core->core_position.y, w.core[i].core_position.x, w.core[i].core_position.y) < broadcast_range
						&& w.core[i].listen_channel [stack_parameters [1]])
					{
      write_message(&w.core[i], stack_parameters [1], stack_parameters [2], MESSAGE_TYPE_BROADCAST, core, -1, 0, variable_parameters - BROADCAST_PARAMETERS, &stack_parameters [BROADCAST_PARAMETERS]);
					}
				}
			}
			return 1;

		case SMETHOD_CALL_BROADCAST_TARGET:
			{
// this should cost quite a few instructions, or even some power
				if (stack_parameters [1] < 0 // check message channel
				 || stack_parameters [1] >= CHANNELS)
					return 0; // could write an error message
				if (stack_parameters [2] != 0 // check message priority
				 && stack_parameters [2] != 1)
					return 0; // could write an error message
				al_fixed broadcast_range = al_itofix(stack_parameters [0]);
				if (broadcast_range <= 0) // check range
					broadcast_range = al_itofix(30000); // if range set to <= 0, it's treated as maximum range
				if (variable_parameters >= MESSAGE_LENGTH + BROADCAST_TARGET_PARAMETERS)
				{
					vmstate.error_state = 1; // this error would probably mess up the stack, so let's return
					print_method_error("too many parameters to broadcast_target call", 0, 0); // really should catch this at compilation!
					return 0;
				}
				int transmitted_target_index = stack_parameters [3];
				if (transmitted_target_index < 0
					|| transmitted_target_index >= PROCESS_MEMORY_SIZE)
//					|| core->process_memory [transmitted_target_index] == -1) // don't think this matters - should be okay to transmit null target
					return 0;
// shouldn't be necessary to check whether the transmitted_target actually exists
				vmstate.instructions_left -= 64; // seems reasonable - this should be an expensive operation. Could cost more.
// additional power cost for broadcast?
    for (i = w.player[core->player_index].core_index_start; i < w.player[core->player_index].core_index_end; i ++)
				{
					if (w.core[i].exists > 0
						&& i != core->index
      && distance_oct_xyxy(core->core_position.x, core->core_position.y, w.core[i].core_position.x, w.core[i].core_position.y) < broadcast_range
						&& w.core[i].listen_channel [stack_parameters [1]])
					{
      write_message(&w.core[i], stack_parameters [1], stack_parameters [2], MESSAGE_TYPE_BROADCAST_TARGET, core, core->process_memory [transmitted_target_index], core->process_memory_timestamp [transmitted_target_index], variable_parameters - BROADCAST_TARGET_PARAMETERS, &stack_parameters [BROADCAST_TARGET_PARAMETERS]);
					}
				}
			}
			return 1;


		case SMETHOD_CALL_CHECK_MESSAGES:
			return core->messages_received - 1 - core->message_reading; // +1 because message_reading starts at -1
		case SMETHOD_CALL_GET_MESSAGE_TYPE:
			if (core->message_reading < 0
			 || core->message_reading >= core->messages_received)
				return MESSAGE_TYPE_NONE;
			return core->message[core->message_reading].type;
		case SMETHOD_CALL_GET_MESSAGE_CHANNEL:
			if (core->message_reading < 0
			 || core->message_reading >= core->messages_received)
				return -1;
			return core->message[core->message_reading].channel;
		case SMETHOD_CALL_GET_MESSAGE_SOURCE:
			if (core->message_reading < 0
			 || core->message_reading >= core->messages_received)
				return 0;
			if (stack_parameters[0] < 0
				|| stack_parameters[0] >= PROCESS_MEMORY_SIZE)
				return 0;
			core->process_memory [stack_parameters [0]] = core->message[core->message_reading].source_index;
			core->process_memory_timestamp [stack_parameters [0]] = core->message[core->message_reading].source_index_timestamp;
// source_index may no longer exist, but doesn't matter
			return 1;
		case SMETHOD_CALL_GET_MESSAGE_X:
			if (core->message_reading < 0
			 || core->message_reading >= core->messages_received)
				return -1;
			return al_fixtoi(core->message[core->message_reading].source_position.x);
		case SMETHOD_CALL_GET_MESSAGE_Y:
			if (core->message_reading < 0
			 || core->message_reading >= core->messages_received)
				return -1;
			return al_fixtoi(core->message[core->message_reading].source_position.y);
		case SMETHOD_CALL_GET_MESSAGE_TARGET:
			if (core->message_reading >= core->messages_received
				|| core->message_reading < 0
				|| stack_parameters [0] < 0
				|| stack_parameters [0] >= PROCESS_MEMORY_SIZE)
				return -1;
			core->process_memory [stack_parameters [0]] = core->message[core->message_reading].target_core_index;
			core->process_memory_timestamp [stack_parameters [0]] = core->message[core->message_reading].target_core_created_timestamp;
			if (core->process_memory [stack_parameters [0]] == -1)
				return 0;
			return 1;
		case SMETHOD_CALL_GET_MESSAGE_PRIORITY:
			if (core->message_reading < 0
			 || core->message_reading >= core->messages_received)
				return -1;
			return core->message[core->message_reading].priority;
		case SMETHOD_CALL_READ_MESSAGE:
			if (core->message_reading < 0
				||	core->message_reading >= core->messages_received
				|| core->message_position >= core->message[core->message_reading].length)
				return 0;
			return core->message[core->message_reading].data [core->message_position++]; // note message_position is incremented here
		case SMETHOD_CALL_NEXT_MESSAGE:
			if (core->message_reading >= core->messages_received)
				return 0;
			core->message_reading ++;
			core->message_position = 0;
			if (core->message_reading >= core->messages_received)
				return 0;
			return 1;
		case SMETHOD_CALL_IGNORE_CHANNEL:
			if (stack_parameters[0] < 0
				|| stack_parameters[0] >= CHANNELS)
					return 1;
			core->listen_channel [stack_parameters[0]] = 0;
			return 1;
		case SMETHOD_CALL_LISTEN_CHANNEL:
			if (stack_parameters[0] < 0
				|| stack_parameters[0] >= CHANNELS)
					return 1;
			core->listen_channel [stack_parameters[0]] = 1;
			return 1;
		case SMETHOD_CALL_IGNORE_ALL_CHANNELS:
			for (i = 0; i < CHANNELS; i ++)
			{
				core->listen_channel [i] = 0;
			}
			return 1;
		case SMETHOD_CALL_COPY_COMMANDS:
			{
    struct core_struct* transmit_target_core;
    if (verify_friendly_target_core(core, stack_parameters [0], &transmit_target_core) != 1)
					return 0;
				for (i = 0; i < COMMAND_QUEUE; i ++)
				{
					if (core->command_queue[i].type == COM_NONE)
						break;
					transmit_target_core->command_queue[i] = core->command_queue[i];
					transmit_target_core->command_queue[i].new_command = 1;
					transmit_target_core->command_queue[i].command_time = game.total_time;
				}
				if (transmit_target_core->command_queue[0].type != COM_NONE)
  			transmit_target_core->new_command = 1;
			}
			return 1;
		case SMETHOD_CALL_GIVE_COMMAND:
			{
    struct core_struct* transmit_target_core;
    if (verify_friendly_target_core(core, stack_parameters [0], &transmit_target_core) != 1)
					return 0;
// stack_parameters:
//  0 target
//  1 command type
//  2 x
//  3 y
//  4 target of command (index in core's process memory)
//  5 component of target of command
//  6 queued
//  7 control_pressed
				if (stack_parameters [1] < 0
					|| stack_parameters [1] >= COM_TYPES)
					return 0;
// shouldn't need to bounds-check x or y
	   int command_target = stack_parameters [4];
				if (command_target < 0
					|| command_target >= PROCESS_MEMORY_SIZE)
					command_target = -1;
				  else
						{
							command_target = core->process_memory [command_target]; // might be -1
						}
					int member_target = stack_parameters [5];
					if (member_target < 0
						|| member_target >= GROUP_MAX_MEMBERS)
						member_target = 0;
					int queued = 0;
					if (stack_parameters [6])
						queued = 1;
					int control_pressed = 0;
					if (stack_parameters [7])
						control_pressed = 1;

     return add_command(transmit_target_core,
																								stack_parameters [1],
																        stack_parameters [2],
																        stack_parameters [3],
																        command_target,
																        member_target,
																        queued,
																        control_pressed);

			}
			return 1;
		case SMETHOD_CALL_GIVE_BUILD_COMMAND:
			{
    struct core_struct* transmit_target_core;
    if (verify_friendly_target_core(core, stack_parameters [0], &transmit_target_core) != 1)
					return 0;
// stack_parameters:
//  0 target
//  1 template
//  2 x
//  3 y
//  4 angle
//  5 back_or_front of queue
//  6 repeat
//  7 queued

// verify the parameters:
   if (stack_parameters [1] < 0
				|| stack_parameters [1] >= TEMPLATES_PER_PLAYER)
			{
				 if (w.debug_mode)
					 print_method_error("give_build_command invalid template index", 1, stack_parameters [1]);
					return 0;
			}

   if (stack_parameters [2] < 255
				|| stack_parameters [2] >= w.w_pixels - 255)
			{
				 if (w.debug_mode)
					 print_method_error("give_build_command invalid build_x", 1, stack_parameters [2]);
					return -1;
			}

   if (stack_parameters [3] < 255
				|| stack_parameters [3] >= w.h_pixels - 255)
			{
				 if (w.debug_mode)
					 print_method_error("give_build_command invalid build_y", 1, stack_parameters [3]);
					return -1;
			}
// angle can be anything
// target_index can be anything
// back_or_front currently ignored
// repeat dealt with below

			return add_to_build_queue(core->player_index,
																						       transmit_target_core->index,
																						       stack_parameters [1], // template
																						       stack_parameters [2], // x
																						       stack_parameters [3], // y
																						       stack_parameters [4] & ANGLE_MASK,  // angle
																						       stack_parameters [5], // back_or_front
																						       (stack_parameters [6] != 0), // repeat
																						       (stack_parameters [7] != 0), // queue_for_this_core - maybe?
																						       0); // failure_message - maybe?

			}
		case SMETHOD_CALL_CHECK_BUILD_RANGE:
//			if (abs(al_fixtoi(core->core_position.x) - stack_parameters [0]) > BUILD_RANGE_BASE_PIXELS
//				|| abs(al_fixtoi(core->core_position.y) - stack_parameters [1]) > BUILD_RANGE_BASE_PIXELS)
   if (distance_oct_xyxy(core->core_position.x, core->core_position.y, al_itofix(stack_parameters [0]), al_itofix(stack_parameters [1])) > BUILD_RANGE_BASE_FIXED)
				return 0;
			return 1;
		case SMETHOD_CALL_REPAIR_SELF:
			{
			 if (core->number_of_repair_objects == 0
				 || core->last_repair_restore_time == w.world_time) // can only be used once per cycle
//				 || core->stress_level >= STRESS_MODERATE)
				 return -1; // error
#define HP_REPAIRED_PER_OBJECT 1
				return repair_process(core, core);
			}
		case SMETHOD_CALL_REPAIR_OTHER:
			{
			 if (core->last_repair_restore_time == w.world_time
//				 || core->stress_level >= STRESS_MODERATE
				 || core->number_of_repair_objects == 0
					|| core->has_repair_other_object == 0)
				 return -1; // error
    struct core_struct* repair_target_core;
    if (verify_friendly_target_core(core, stack_parameters [0], &repair_target_core) != 1)
					return -1;
//				fpr("\ndist %i,%i range %i", al_fixtoi(), al_fixtoi(), al_fixtoi());
    if (distance_oct_xyxy(repair_target_core->core_position.x, repair_target_core->core_position.y,
																										core->core_position.x, core->core_position.y) > SCAN_RANGE_BASE_FIXED)
						return -1;
				return repair_process(repair_target_core, core);
			}
		case SMETHOD_CALL_REPAIR_SCAN:
			 if (core->last_repair_restore_time == w.world_time
//				 || core->stress_level >= STRESS_MODERATE
				 || core->number_of_repair_objects == 0
					|| core->has_repair_other_object == 0)
				 return -1; // error
			return scan_repair(core, stack_parameters, core->number_of_repair_objects * HP_REPAIRED_PER_OBJECT);
		case SMETHOD_CALL_RESTORE_SELF:
			{
			 if (core->last_repair_restore_time == w.world_time // can only be used once per cycle
				 || core->restore_cooldown_time > w.world_time
//				 || core->stress_level >= STRESS_MODERATE
				 || core->number_of_repair_objects == 0)
				 return -1; // error
				return restore_process(core, core);
			}
		case SMETHOD_CALL_RESTORE_OTHER:
			{
			 if (core->last_repair_restore_time == w.world_time
				 || core->restore_cooldown_time > w.world_time
//				 || core->stress_level >= STRESS_MODERATE
				 || core->number_of_repair_objects == 0
					|| core->has_repair_other_object == 0)
				 return -1; // error
    struct core_struct* repair_target_core;
    if (verify_friendly_target_core(core, stack_parameters [0], &repair_target_core) != 1)
					return -1;
//				if (abs(al_fixtoi(core->core_position.x - repair_target_core->core_position.x)) > SCAN_RANGE_BASE_PIXELS
//					|| abs(al_fixtoi(core->core_position.y - repair_target_core->core_position.y)) > SCAN_RANGE_BASE_PIXELS)
				if (distance_oct_xyxy(core->core_position.x, core->core_position.y, repair_target_core->core_position.x, repair_target_core->core_position.y) > SCAN_RANGE_BASE_FIXED)
					return -1; // out of range
				return restore_process(repair_target_core, core);
			}
		case SMETHOD_CALL_RESTORE_SCAN:
			 if (core->last_repair_restore_time == w.world_time
				 || core->restore_cooldown_time > w.world_time
//				 || core->stress_level >= STRESS_MODERATE
				 || core->number_of_repair_objects == 0
					|| core->has_repair_other_object == 0)
				 return -1; // error
			return scan_restore(core, stack_parameters);


		 case SMETHOD_CALL_BUILD_PROCESS:
		 	{
     return build_call(core,
																							stack_parameters [0], // template index
																							core->core_position.x + al_itofix(stack_parameters [1]), // build_x
																							core->core_position.y + al_itofix(stack_parameters [2]), // build_y
																							stack_parameters [3], // build_angle
																							stack_parameters [4]); // process memory address

		 	}
			 break;
/*
			case SMETHOD_CALL_BUILD_AS_COMMANDED:
		 	{
     return build_call(core,
																							core->build_command_queue[0].build_template, // template index
																							al_itofix(core->build_command_queue[0].build_x), // build_x
																							al_itofix(core->build_command_queue[0].build_y), // build_y
																							core->build_command_queue[0].build_angle, // build_angle
																							stack_parameters [0]); // process memory address
		 	}
			 break;*/
			case SMETHOD_CALL_BUILD_REPEAT:
				{
     return build_call(core,
																							core->rebuild_template, // template index
																							core->rebuild_x, // build_x
																							core->rebuild_y, // build_y
																							core->rebuild_angle, // build_angle
																							stack_parameters [0]); // process memory address
				}
				break;
			case SMETHOD_CALL_GET_TEMPLATE_COST:
				{
					int costed_template_index = stack_parameters [0];
					if (costed_template_index < 0
						|| costed_template_index >= TEMPLATES_PER_PLAYER
						|| !templ[core->player_index][costed_template_index].active)
						return 0;
					return templ[core->player_index][costed_template_index].data_cost;
				}
				break;
			case SMETHOD_CALL_RANDOM:
				{
					if (stack_parameters [0] <= 0) // mod
						return 0;

     w.player[core->player_index].random_seed = w.player[core->player_index].random_seed * 1103515245 + 12345;
     return (unsigned int)(w.player[core->player_index].random_seed / 65536) % stack_parameters [0];

				}

			case SMETHOD_CALL_CHECK_CONTACT:
				if (core->contact_core_index != -1)
				{
					if (stack_parameters [0] >= 0
						&& stack_parameters [0] < PROCESS_MEMORY_SIZE)
					{
						core->process_memory [stack_parameters [0]] = core->contact_core_index;
						core->process_memory_timestamp [stack_parameters [0]] = core->contact_core_timestamp;
					}
					return 1;
				}
				return 0;
			case SMETHOD_CALL_GET_DAMAGE:
				return core->damage_this_cycle;
			case SMETHOD_CALL_GET_DAMAGE_SOURCE:
				if (core->damage_source_core_index != -1)
				{
					if (stack_parameters [0] >= 0
						&& stack_parameters [0] < PROCESS_MEMORY_SIZE)
					{
						core->process_memory [stack_parameters [0]] = core->damage_source_core_index;
						core->process_memory_timestamp [stack_parameters [0]] = core->damage_source_core_timestamp;
					}
					return 1;
				}
				return 0;

			case SMETHOD_CALL_DISTANCE_FROM_XY:
//				fpr("\n distance_from_xy(): oct %i hypot %i", distance_oct_xyxy(core->core_position.x, core->core_position.y, al_itofix(stack_parameters [0]), al_itofix(stack_parameters [1])),
				return al_fixtoi(distance_oct_xyxy(core->core_position.x, core->core_position.y, al_itofix(stack_parameters [0]), al_itofix(stack_parameters [1])));
//				return abs(al_fixtoi(core->core_position.x) - stack_parameters [0]) + abs(al_fixtoi(core->core_position.y) - stack_parameters [1]);
/*			case SMETHOD_CALL_DISTANCE_FROM_XY_HYPOT:
    vmstate.instructions_left -= INSTRUCTION_COST_HYPOT;
	 	 return al_fixtoi(distance(al_itofix(stack_parameters [0]) - core->core_position.x,
																				          al_itofix(stack_parameters [1]) - core->core_position.y));*/
			case SMETHOD_CALL_DISTANCE_XY:
				return al_fixtoi(distance_oct_xyxy(0, 0, al_itofix(stack_parameters [0]), al_itofix(stack_parameters [1])));
			case SMETHOD_CALL_DISTANCE_FROM_XY_LESS:
				{
//					int dist = al_fixtoi(distance_oct_xyxy(core->core_position.x, core->core_position.y, al_itofix(stack_parameters [0]), al_itofix(stack_parameters [1])));
					if (al_fixtoi(distance_oct_xyxy(core->core_position.x, core->core_position.y, al_itofix(stack_parameters [0]), al_itofix(stack_parameters [1]))) < stack_parameters [2])
						return 1;
					return 0;
/*					uint64_t	x_dist = al_fixtoi(core->core_position.x) - stack_parameters [0];
					x_dist *= x_dist;
					uint64_t	y_dist = al_fixtoi(core->core_position.y) - stack_parameters [1];
					y_dist *= y_dist;
					uint64_t compare_value = stack_parameters [2] * stack_parameters [2];

					if (x_dist + y_dist < compare_value)
						return 1;
					return 0;*/
				}
			case SMETHOD_CALL_DISTANCE_FROM_XY_MORE:
				{
//					int distance = al_fixtoi(distance_oct_xyxy(core->core_position.x, core->core_position.y, al_itofix(stack_parameters [0]), al_itofix(stack_parameters [1])));
					if (al_fixtoi(distance_oct_xyxy(core->core_position.x, core->core_position.y, al_itofix(stack_parameters [0]), al_itofix(stack_parameters [1]))) > stack_parameters [2])
						return 1;
					return 0;

/*					uint64_t	x_dist = al_fixtoi(core->core_position.x) - stack_parameters [0];
					x_dist *= x_dist;
					uint64_t	y_dist = al_fixtoi(core->core_position.y) - stack_parameters [1];
					y_dist *= y_dist;
					uint64_t compare_value = stack_parameters [2] * stack_parameters [2];

					if (x_dist + y_dist > compare_value)
						return 1;
					return 0;*/
				}

			case SMETHOD_CALL_TARGET_CLEAR:
				if (stack_parameters [0] >= 0
					&& stack_parameters [0] < PROCESS_MEMORY_SIZE)
				{
					core->process_memory [stack_parameters [0]] = -1;
					return 1;
				}
				return 0;
			case SMETHOD_CALL_TARGET_COMPARE:
			 if (stack_parameters [0] >= 0
			 	&& stack_parameters [0] < PROCESS_MEMORY_SIZE
			 	&& stack_parameters [1] >= 0
			 	&& stack_parameters [1] < PROCESS_MEMORY_SIZE)
			 {
			 	if (core->process_memory [stack_parameters [0]] == core->process_memory [stack_parameters [1]]
			 		&& core->process_memory_timestamp [stack_parameters [0]] == core->process_memory_timestamp [stack_parameters [1]])
			 	return 1;
			 }
				return 0;
			case SMETHOD_CALL_TARGET_COPY:
				if (stack_parameters [0] >= 0
					&& stack_parameters [0] < PROCESS_MEMORY_SIZE
					&& stack_parameters [1] >= 0
					&& stack_parameters [1] < PROCESS_MEMORY_SIZE)
				{
// does not confirm that source is non-empty
					core->process_memory [stack_parameters [0]] = core->process_memory [stack_parameters [1]];
					core->process_memory_timestamp [stack_parameters [0]] = core->process_memory_timestamp [stack_parameters [1]];
					return 1;
				}
				return 0;
			case SMETHOD_CALL_TARGET_DESTROYED:
//fpr("\n TD target %i ", core->process_memory [stack_parameters [0]]);
				if (stack_parameters [0] < 0
					|| stack_parameters [0] >= PROCESS_MEMORY_SIZE
					|| core->process_memory [stack_parameters [0]] == -1)
					return 0;//-1;
/*fpr(" at %i,%i [%i,%i] vis %i (now %i) ex %i dt %i",
				al_fixtoi(w.core[core->process_memory [stack_parameters [0]]].core_position.x),
				al_fixtoi(w.core[core->process_memory [stack_parameters [0]]].core_position.y),
				w.proc[w.core[core->process_memory [stack_parameters [0]]].process_index].block_position.x,
				w.proc[w.core[core->process_memory [stack_parameters [0]]].process_index].block_position.y,
				w.vision_area[core->player_index]
				                  [w.proc[w.core[core->process_memory [stack_parameters [0]]].process_index].block_position.x]
										            [w.proc[w.core[core->process_memory [stack_parameters [0]]].process_index].block_position.y].vision_time,
				w.world_time,
				w.core[core->process_memory [stack_parameters [0]]].exists,
				w.core[core->process_memory [stack_parameters [0]]].destroyed_timestamp);*/

			 if (w.core[core->process_memory [stack_parameters [0]]].exists == 0
					&& w.core[core->process_memory [stack_parameters [0]]].created_timestamp == core->process_memory_timestamp [stack_parameters [0]]
     && w.core[core->process_memory [stack_parameters [0]]].destroyed_timestamp >= w.world_time - DEALLOCATE_COUNTER
     && (w.core[core->process_memory [stack_parameters [0]]].player_index == core->player_index
						||	w.vision_area[core->player_index]
				                  [w.proc[w.core[core->process_memory [stack_parameters [0]]].process_index].block_position.x]
										            [w.proc[w.core[core->process_memory [stack_parameters [0]]].process_index].block_position.y].vision_time > w.world_time - VISION_AREA_VISIBLE_TIME))
//				                  [w.proc[w.core[core->process_memory [stack_parameters [0]]].process_index].block_position.x]
//										            [w.proc[w.core[core->process_memory [stack_parameters [0]]].process_index].block_position.y].vision_time > w.world_time - VISION_AREA_VISIBLE_TIME))
    {
//    	fpr(" ret 1");
					return 1;
    }
//   	fpr(" ret 0");
				return 0;




			case SMETHOD_CALL_ATTACK_MODE:
				core->attack_mode = stack_parameters [0]; // not bounds-checked - invalid values treated as 0 (all objects attack)
				return 1;

			case SMETHOD_CALL_GET_PROCESS_COUNT:
				return w.player[core->player_index].processes;
			case SMETHOD_CALL_GET_PROCESSES_MAX:
				return w.max_cores;
			case SMETHOD_CALL_GET_PROCESSES_UNUSED:
				return w.max_cores - w.player[core->player_index].processes;
			case SMETHOD_CALL_GET_COMPONENT_COUNT:
				return w.player[core->player_index].components_reserved; // reserved is probably a more useful value than current
			case SMETHOD_CALL_GET_COMPONENTS_MAX:
				return w.max_procs;
			case SMETHOD_CALL_GET_COMPONENTS_UNUSED:
				return w.max_procs - w.player[core->player_index].components_reserved;
			case SMETHOD_CALL_SPECIAL_AI:
				special_AI_method(core, stack_parameters [0], stack_parameters [1]);
				return 0;




//			case SMETHOD_CALL_DISTANCE_FROM_TARGET:
//				return abs(al_fixtoi(core->core_position.x) - stack_parameters [0]) + abs(al_fixtoi(core->core_position.y) - stack_parameters [1]);

 }

 return 0;

}


// similar to object_uses_power in g_method.c
// note that not all power use by standard methods goes through this
static int standard_method_uses_power(struct core_struct* core, int power_cost)
{


					   if (core->power_left < power_cost)
					   {
					    core->power_use_excess += power_cost;
					    return 0;
					   }

        core->power_left -= power_cost;

        return 1;

}



// if successful, saves newly build process to process_memory_address (skips this if address out out bounds)
static int build_call(struct core_struct* core, int build_template, al_fixed build_x, al_fixed build_y, int build_angle, int process_memory_address)
{

 if (core->build_cooldown_time >= w.world_time)
 {
// 	if (w.debug_mode)
// 	 print_method_error("build object not ready", 0, 0); this just spams too many messages
 	return BUILD_FAIL_NOT_READY;
 }

// check for sufficient power. Actual power use happens later.
 if (core->power_left < BUILD_POWER_COST * core->number_of_build_objects)
	{
		if (w.debug_mode)
		 print_method_error("not enough power to build", 0, 0);
		return BUILD_FAIL_POWER;
	}

// set values for repeat/retry build to use if necessary (doesn't matter if they're invalid as they'll be checked in this function if used)
     core->rebuild_template = build_template;
     core->rebuild_x = build_x;
     core->rebuild_y = build_y;
     core->rebuild_angle = build_angle;

// test for builder range first because a BUILD_FAIL_OUT_OF_RANGE indicates to a mobile builder that it should move to the build area
/*					if (build_x < core->core_position.x - al_itofix(BUILD_RANGE_BASE_PIXELS)
					 || build_x > core->core_position.x + al_itofix(BUILD_RANGE_BASE_PIXELS)
						||	build_y < core->core_position.y - al_itofix(BUILD_RANGE_BASE_PIXELS)
					 || build_y > core->core_position.y + al_itofix(BUILD_RANGE_BASE_PIXELS))*/
					if (distance_oct_xyxy(core->core_position.x, core->core_position.y, build_x, build_y) > BUILD_RANGE_BASE_FIXED)
					{
 					if (w.debug_mode)
    		 print_method_error("build call out of range", 0, 0);
						return BUILD_FAIL_OUT_OF_RANGE;
					}

		 	 if (core->number_of_build_objects == 0)
					{
 					if (w.debug_mode)
    		 print_method_error("process has no build objects", 0, 0);
						return BUILD_FAIL_NO_BUILD_OBJECTS;
					}

	    if (build_template < 0
						|| build_template >= TEMPLATES_PER_PLAYER)
					{
 					if (w.debug_mode)
    		 print_method_error("build method called on invalid template", 1, build_template);
						return BUILD_FAIL_TEMPLATE_INVALID; // invalid template index
					}

     if (!templ[core->player_index][build_template].active)
					{
 					if (w.debug_mode)
    		 print_method_error("build method called on empty template", 1, build_template);
     	return BUILD_FAIL_TEMPLATE_INACTIVE; // invalid template
					}

     if (w.player[core->player_index].data < templ[core->player_index][build_template].data_cost)
					{
/* 					if (w.debug_mode)
						{
  					sprintf(method_error_string, "\nnot enough data to build (have %i, need %i)", w.player[core->player_index].data, templ[core->player_index][build_template].data_cost);
    		 print_method_error_string();
						}*/
     	return BUILD_FAIL_DATA; // not enough data
					}

// work out location
     cart new_core_position;
	    new_core_position.x = build_x;
	    new_core_position.y = build_y;

	    block_cart new_core_block_position;
	    new_core_block_position.x = fixed_to_block(build_x);
	    new_core_block_position.y = fixed_to_block(build_y);

	    if (new_core_block_position.x < 2
						|| new_core_block_position.x > w.blocks.x - 3
						|| new_core_block_position.y < 2
						|| new_core_block_position.y > w.blocks.y - 3)
					{
 					if (w.debug_mode)
    		 print_method_error("out of bounds", 0, 0);
						return BUILD_FAIL_OUT_OF_BOUNDS;
					}

// now we can use new_core_block_position to subscript the world block array:

	    if (templ[core->player_index][build_template].member[0].shape < FIRST_MOBILE_NSHAPE
 					&& !check_static_build_location_for_data_wells(new_core_position.x, new_core_position.y))
//						&& w.backblock[new_core_block_position.x][new_core_block_position.y].backblock_type != BACKBLOCK_BASIC_HEX)
					{
 					if (w.debug_mode)
    		 print_method_error("static core too near data well", 0, 0);
						return BUILD_FAIL_STATIC_NEAR_WELL;
					}
/*
					if (w.local_condition == LOCAL_CONDITION_SINGLE_ALLOCATOR
						&& templ[core->player_index][build_template].has_allocator)
					{
// 					if (w.debug_mode)
   		 print_method_error("local conditions prevent allocator being built", 0, 0);
						return BUILD_FAIL_LOCAL_CONDITIONS;
					}*/

					struct core_struct* collided_core;

	    int build_result = create_new_from_template(&templ[core->player_index][build_template], core->player_index, new_core_position, int_angle_to_fixed(build_angle), &collided_core);
// create_new_from_template() returns core index on success, or a BUILD_FAIL result otherwise (the BUILD_FAIL results it can return are all < 0)

	    if (build_result < 0)
					{

							 if (build_result == BUILD_FAIL_COLLISION)
							 {
// try to nudge the obstructing process out of the way (only works if it's friendly):
         if (collided_core->mobile
										&& core->player_index == collided_core->player_index)
									{
							 	 al_fixed nudge_angle = get_angle(collided_core->core_position.y - core->core_position.y, collided_core->core_position.x - core->core_position.x);
							 	 collided_core->group_speed.x += fixed_xpart(nudge_angle, al_itofix(100) / collided_core->group_mass);
							 	 collided_core->group_speed.y += fixed_ypart(nudge_angle, al_itofix(100) / collided_core->group_mass);
          place_build_lines(core, collided_core->core_position);
									}
//    					if (w.debug_mode
									//&& !collided_core->mobile)
          //print_method_error("collision", 0, 0);
							 }


 					if (w.debug_mode)
						{
						 switch(build_result)
						 {
							 case BUILD_FAIL_TOO_MANY_CORES:
     		  print_method_error("process limit reached", 0, 0); break;
							 case BUILD_FAIL_TOO_MANY_PROCS:
     		  print_method_error("component limit reached", 0, 0); break;
							 case BUILD_FAIL_OUT_OF_BOUNDS:
     		  print_method_error("location outside world", 0, 0); break;
/*
 - collision is dealt with outside the debug_mode test, as the nudge needs to work without debug mode
							 case BUILD_FAIL_COLLISION:
							 {
// try to nudge the obstructing process out of the way (only works if it's friendly):
         if (collided_core->mobile
										&& core->player_index == collided_core->player_index)
									{
							 	 al_fixed nudge_angle = get_angle(collided_core->core_position.y - core->core_position.y, collided_core->core_position.x - core->core_position.x);
							 	 collided_core->group_speed.x += fixed_xpart(nudge_angle, al_itofix(100) / collided_core->group_mass);
							 	 collided_core->group_speed.y += fixed_ypart(nudge_angle, al_itofix(100) / collided_core->group_mass);
          place_build_lines(core, collided_core->core_position);
									}
         print_method_error("collision", 0, 0); break;
							 }
							 */
							 case BUILD_FAIL_TEMPLATE_NOT_LOCKED:
     		  print_method_error("failed to lock template", 0, 0); break;
						 }
						}
						return build_result; // build_result should hold the BUILD_FAIL code
					}

// success!

				 core->last_build_time = w.world_time; // should this be earlier, so it's set even if the build fails? hm.
				 int build_cooldown_cycles = templ[core->player_index][build_template].build_cooldown_cycles / core->number_of_build_objects;
				 core->build_cooldown_time = w.world_time + ((build_cooldown_cycles + 1) * EXECUTION_COUNT); // number_of_build_objects has been confirmed to be non-zero above
				 w.core[build_result].next_execution_timestamp = w.world_time + (((build_cooldown_cycles / 2) + 1) * EXECUTION_COUNT);
				 w.core[build_result].construction_complete_timestamp = w.core[build_result].next_execution_timestamp;
//fpr("\n bct %i templ %i (templ[%i][%i])", core->build_cooldown_time, templ[core->player_index][build_template].build_cooldown_time, core->player_index, build_template);
//fpr("\n bct %i templ %i nsh %i", core->build_cooldown_time, templ[core->player_index][build_template].build_cooldown_time, nshape[templ[core->player_index][build_template].member[0].shape].build_or_restore_time);
// if changing build cooldown time calculation, may also need to change build button display code in i_console.c
				 w.player[core->player_index].data -= templ[core->player_index][build_template].data_cost;


     place_build_lines(core, new_core_position);
/*
					int member_index = core->first_build_object_member;
					int object_index = core->first_build_object_link;

//					fpr("\n Start: member_index %i object_index %i", member_index, object_index);
					int counter = 0;

				 while(member_index != -1)
					{
						if (w.proc[core->group_member[member_index].index].exists == 1)
 				  place_build_line(&w.proc[core->group_member[member_index].index], object_index, new_core_position);
// even if member doesn't exist, its proc entry should be reserved and its object arrays should point to the next member
      int old_member_index = member_index;
 				 member_index = w.proc[core->group_member[member_index].index].object[object_index].next_similar_object_member;
 				 object_index = w.proc[core->group_member[old_member_index].index].object[object_index].next_similar_object_link;
//					fpr("\n Loop: member_index %i object_index %i", member_index, object_index);
					counter ++;
					if (counter > 20)
						break;
					};
*/

					core->retry_build_collision_count = 0;
//					core->power_used += BUILD_POWER_COST * core->number_of_build_objects;
     core->power_left -= BUILD_POWER_COST * core->number_of_build_objects; // power left has been checked above to make sure there's enough
					set_ongoing_power_cost_for_object_type(core, OBJECT_TYPE_BUILD, -1, BUILD_POWER_COST, core->build_cooldown_time);

					if (process_memory_address >= 0
						&& process_memory_address < PROCESS_MEMORY_SIZE)
					{
// no error for out-of-bounds; programs are expected to use -1 if no memorisation is needed
						core->process_memory [process_memory_address] = build_result; // this should hold the new core's index
						core->process_memory_timestamp [process_memory_address] = w.core[build_result].created_timestamp;
					}

// Also, the builder process is written to the new process' targetting memory:
     w.core[build_result].process_memory [0] = core->index;
     w.core[build_result].process_memory_timestamp [0] = core->created_timestamp;

				 return 1;


}

static void place_build_lines(struct core_struct* core, cart target_position)
{


					int member_index = core->first_build_object_member;
					int object_index = core->first_build_object_link;

//					fpr("\n Start: member_index %i object_index %i", member_index, object_index);
					int counter = 0;

				 while(member_index != -1)
					{
						if (w.proc[core->group_member[member_index].index].exists == 1)
 				  place_build_line(&w.proc[core->group_member[member_index].index], object_index, target_position);
// even if member doesn't exist, its proc entry should be reserved and its object arrays should point to the next member
      int old_member_index = member_index;
 				 member_index = w.proc[core->group_member[member_index].index].object[object_index].next_similar_object_member;
 				 object_index = w.proc[core->group_member[old_member_index].index].object[object_index].next_similar_object_link;
//					fpr("\n Loop: member_index %i object_index %i", member_index, object_index);
					counter ++;
					if (counter > 20)
						break;
					};


}

static void place_build_line(struct proc_struct* building_proc, int object_index, cart target_position)
{

      struct cloud_struct* cl = new_cloud(CLOUD_BUILD_LINE, 16, target_position.x, target_position.y);

      if (cl != NULL)
      {
       cl->colour = building_proc->player_index;
//       cl->position2.x = x;
//       cl->position2.y = y;
//      cl->data [0] = ;
       cl->data [0] = building_proc->index;
       cl->data [1] = object_index;
       cl->speed.x = 0; // new core is assumed to be immobile
       cl->speed.y = 0;
       cl->associated_proc_timestamp = building_proc->created_timestamp;
// work out the display bounding box:
       cl->display_size_x1 = -80 - al_fixtof(abs(target_position.x - building_proc->position.x));
       cl->display_size_y1 = -80 - al_fixtof(abs(target_position.y - building_proc->position.y));
       cl->display_size_x2 = 80 + al_fixtof(abs(target_position.x - building_proc->position.x));
       cl->display_size_y2 = 80 + al_fixtof(abs(target_position.y - building_proc->position.y));
      }


}

// also used for restore
static void place_repair_line(struct proc_struct* repairing_proc, int object_index, struct proc_struct* target_proc)//cart target_position)
{

      struct cloud_struct* cl = new_cloud(CLOUD_REPAIR_LINE, 16, target_proc->position.x, target_proc->position.y);

      if (cl != NULL)
      {
       cl->colour = repairing_proc->player_index;
//       cl->position2.x = x;
//       cl->position2.y = y;
//      cl->data [0] = ;
       cl->data [0] = repairing_proc->index;
       cl->data [1] = object_index;
       cl->speed = target_proc->speed;
       cl->associated_proc_timestamp = repairing_proc->created_timestamp;
// work out the display bounding box:
       cl->display_size_x1 = -80 - al_fixtof(abs(target_proc->position.x - repairing_proc->position.x));
       cl->display_size_y1 = -80 - al_fixtof(abs(target_proc->position.y - repairing_proc->position.y));
       cl->display_size_x2 = 80 + al_fixtof(abs(target_proc->position.x - repairing_proc->position.x));
       cl->display_size_y2 = 80 + al_fixtof(abs(target_proc->position.y - repairing_proc->position.y));
      }


}



// call when message sent to target_core
// assumes that:
//  - target_core is a valid target
//  - channel is a valid channel, and target_core is listening to it
//  - priority is 0 or 1
//  - message_length is <= MESSAGE_LENGTH
// target_core and source_core are not guaranteed to be different
static s16b write_message(struct core_struct* target_core, int channel, int priority, int message_type, struct core_struct* source_core, int transmitted_target_core_index, timestamp transmitted_target_core_timestamp, int message_length, s16b* message)
{

	int msg_index = 0;
	int i;

	if (target_core->messages_received < MESSAGES)
	{
		msg_index = target_core->messages_received;
		target_core->messages_received ++;
	}
	 else
		{
// message buffer full.
   if (!priority)
				return 0; // priority 0 messages get discarded
// Must be priority 1. So look for a priority 0 message to replace
			for (msg_index = 0; msg_index < MESSAGES; msg_index ++)
			{
				if (target_core->message[msg_index].priority == 0)
					break;
			}
			if (msg_index == MESSAGES)
				return 0; // message buffer is already full of priority 1 messages
// otherwise - replace message[msg_index]
		}

 target_core->message[msg_index].channel = channel;
 target_core->message[msg_index].priority = priority;
 target_core->message[msg_index].source_index = source_core->index;
 target_core->message[msg_index].source_index_timestamp = source_core->created_timestamp;
 target_core->message[msg_index].source_position = source_core->core_position;
 target_core->message[msg_index].type = message_type;
 target_core->message[msg_index].length = message_length;
 target_core->message[msg_index].target_core_index = transmitted_target_core_index;
 target_core->message[msg_index].target_core_created_timestamp = transmitted_target_core_timestamp;

 for (i = 0; i < message_length; i ++)
	{
		target_core->message[msg_index].data [i] = message [i];
	}

// zero out the remaining values in the received message:
	if (message_length < MESSAGE_LENGTH)
	{
		for (i = message_length; i < MESSAGE_LENGTH; i ++)
		{
 		target_core->message[msg_index].data [i] = 0;
		}
	}

 return 1;
}



// searches for a nearby well and updates vmstate.nearby_well_index
// ideally wells should be spaced far enough apart that it doesn't matter that this is a very rough calculation
void find_nearby_well(struct core_struct* core)
{

				int nearest_well_index = -1;
				al_fixed nearest_well_distance = al_itofix(20000);
				al_fixed x_dist, y_dist;

				int i;

				for (i = 0; i < w.data_wells; i++)
				{
					x_dist = abs(core->core_position.x - w.data_well[i].position.x);
					if (x_dist > nearest_well_distance)
						continue;
					y_dist = abs(core->core_position.y - w.data_well[i].position.y);
					if (y_dist > nearest_well_distance)
						continue;
					if (x_dist > y_dist)
						nearest_well_distance = x_dist;
					  else
						  nearest_well_distance = y_dist;
					nearest_well_index = i;
				}

				if (nearest_well_distance <= core->scan_range_fixed)
					vmstate.nearby_well_index = nearest_well_index;
					 else
					  vmstate.nearby_well_index = -1;

}


// called by both repair and repair_other calls
//  so target_core and repairer may be the same
// finds a component to repair and calls repair_specific_component on it
// sets power use for repairer core (any additional use for repair_other should be applied elsewhere)
// returns number of hp repaired (might be 0 if target undamaged), or -1 if error (not currently possible?)
//  does not check range (assumes this has already been dealt with)
static s16b repair_process(struct core_struct* target_core, struct core_struct* repairer)
{
// could check for group hp total here... (not currently kept track of)

 if (target_core->group_total_hp == target_core->group_total_hp_max_current)
		return 0; // no damage to repair

	int repair_amount = 0;

 int i;

 for (i = 0; i < target_core->group_members_max; i ++)
	{
		if (target_core->group_member[i].exists != 0
			&& w.proc[target_core->group_member[i].index].hp < w.proc[target_core->group_member[i].index].hp_max)
		{

			 repair_amount = repair_specific_component(target_core, repairer, i, repairer->number_of_repair_objects);

				if (repair_amount > 0)
				{
					repairer->last_repair_restore_time = w.world_time;
//			  repairer->power_used += repair_amount * POWER_COST_REPAIR_1_INTEGRITY;


					int member_index = repairer->first_repair_object_member;
					int object_index = repairer->first_repair_object_link;

//					fpr("\n Start: member_index %i object_index %i", member_index, object_index);
					int counter = 0;

				 while(member_index != -1)
					{
						if (w.proc[repairer->group_member[member_index].index].exists == 1)
 				  place_repair_line(&w.proc[repairer->group_member[member_index].index], object_index, &w.proc[target_core->group_member[i].index]);//w.proc[target_core->group_member[i].index].position);
// even if member doesn't exist, its proc entry should be reserved and its object arrays should point to the next member
      int old_member_index = member_index;
 				 member_index = w.proc[repairer->group_member[member_index].index].object[object_index].next_similar_object_member;
 				 object_index = w.proc[repairer->group_member[old_member_index].index].object[object_index].next_similar_object_link;
//					fpr("\n Loop: member_index %i object_index %i", member_index, object_index);
					counter ++;
					if (counter > 20)
						break;
					};


//			  place_build_line(&w.proc[core->group_member[member_index].index], object_index, new_core_position);

/*
      struct cloud_struct* cl = new_cloud(CLOUD_HARVEST_LINE, 16, target_core->core_position.x, target_core->core_position.y);

      if (cl != NULL)
      {
       cl->colour = proc->player_index;
       cl->data [0] = proc->index;
       cl->data [1] = object_index;
       cl->associated_proc_timestamp = proc->created_timestamp;
// work out the display bounding box:
       cl->display_size_x1 = -80 - al_fixtof(abs(target_core->core_position.x - proc->position.x));
       cl->display_size_y1 = -80 - al_fixtof(abs(target_core->core_position.y - proc->position.y));
       cl->display_size_x2 = 80 + al_fixtof(abs(target_core->core_position.x - proc->position.x));
       cl->display_size_y2 = 80 + al_fixtof(abs(target_core->core_position.y - proc->position.y));
      }*/
				}
			 return repair_amount;
		}
	}

	return 0;

}

// called when a specific component to be repaired has been identified
// returns the number of hp repaired.
// assumes that repair is possible and component is damaged.
// also takes power from the repairing core (which may be the same as target_core)
static s16b repair_specific_component(struct core_struct* target_core, struct core_struct* repairer, int component_index, int repair_amount)
{

 struct proc_struct* target_proc = &w.proc[target_core->group_member[component_index].index];

 if (repair_amount > target_proc->hp_max - target_proc->hp)
		repair_amount = target_proc->hp_max - target_proc->hp;

	if (!standard_method_uses_power(repairer, repair_amount * POWER_COST_REPAIR_1_INTEGRITY))
		return 0; // TO DO: maybe allow partial repair? But that's getting a bit complicated.

	target_proc->hp += repair_amount;
	target_core->group_total_hp += repair_amount;
	target_proc->repaired_timestamp = w.world_time;

	return repair_amount;

}


// called by both restore and restore_other calls
//  so target_core and restorer may be the same
// finds a component to restore and calls restore_specific_component on it
// sets power use for restorer core
// returns number of hp repaired (might be 0 if target undamaged), or -1 if error (not currently possible?)
static s16b restore_process(struct core_struct* target_core, struct core_struct* restorer)
{
// could check for group hp total here... (not currently kept track of)

 int power_cost = restorer->number_of_repair_objects * POWER_COST_RESTORE_COMPONENT;

 if (restorer->power_left < power_cost)
	{
		restorer->power_use_excess += power_cost;
		return 0;
	}

 int i;

 for (i = 0; i < target_core->group_members_max; i ++)
	{
// It should be possible to assume that upstream components will always come before downstream components in the group_member array.
		if (target_core->group_member[i].exists == 0)
//			&& w.proc[target_core->group_member[i].index].hp < w.proc[target_core->group_member[i].index].hp_max)
		{
			 int restore_result = restore_specific_component(target_core, i);

				if (restore_result)
				{
					restorer->last_repair_restore_time = w.world_time;
					restorer->restore_cooldown_time = w.world_time + ((templ[target_core->player_index][target_core->template_index].member[i].data_cost * 2) / restorer->number_of_repair_objects + 1) * EXECUTION_COUNT;
//			  restorer->power_used += POWER_COST_RESTORE_COMPONENT * restorer->number_of_repair_objects;
     restorer->power_left -= power_cost;
			  set_ongoing_power_cost_for_object_type(restorer, OBJECT_TYPE_REPAIR, OBJECT_TYPE_REPAIR_OTHER, POWER_COST_RESTORE_COMPONENT, restorer->restore_cooldown_time);
			  // note that POWER_COST_RESTORE_COMPONENT is different from the otype peak value, because the otype peak value includes the power used by this object, immediately after execution, if it's in cooldown
				}
			 return restore_result;
		}
	}

	return 0;

}

// can set object_type2 to -1
//  TO DO: need to add lists of objects of same type to template structure, to reduce the size of these horrible loops
static void set_ongoing_power_cost_for_object_type(struct core_struct* core, int object_type1, int object_type2, int power_cost, timestamp power_cost_finish_time)
{
	int i, j;

	for (i = 0; i < core->group_members_max; i++)
	{
		if (core->group_member[i].exists)
		{
			for (j = 0; j < w.proc[core->group_member[i].index].nshape_ptr->links; j ++)
			{
				if (w.proc[core->group_member[i].index].object[j].type == object_type1
					|| w.proc[core->group_member[i].index].object[j].type == object_type2)
				{
					w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost = power_cost;
					w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost_finish_time = power_cost_finish_time;
				}
			}
		}
	}



}


static s16b restore_specific_component(struct core_struct* target_core, int member_index)
{

 s16b restore_result = restore_component(target_core, target_core->player_index, target_core->template_index, member_index);

 if (restore_result)
	{
		struct proc_struct* proc = &w.proc[target_core->group_member[member_index].index];
		proc->hp = proc->hp_max / 4;
 	proc->repaired_timestamp = w.world_time;
		reset_group_after_composition_change(target_core);
	}

	return restore_result;

}

/*

How will interface charging work?

maximum charge_amount to be determined by dividing max interface by something and adding 1
each point costs a certain amount of power
calling charge_interface() allows you to set the number of points charged.
 Usually you'll just call it with 1000 or something to charge at max rate using as much of the available power as possible.


*/

static s16b charge_interface(struct core_struct* core, int charge_amount)
{
 if (core->interface_available == 0 // no interface
		|| core->interface_charged_time == w.world_time) // can only charge once per cycle
//		|| core->stress_level >= STRESS_MODERATE)
		return 0;

	int strength_gained = 0; // this must come before any goto finished_charging (because the value may be used)
	int objects_used = core->number_of_interface_objects;
	int strength_gained_adjusted = 0;

	if (core->interface_strength == core->interface_strength_max
	 || charge_amount <= 0)
		goto finished_charging;

	if (objects_used > charge_amount)
		objects_used = charge_amount;

// Limit objects_used by the amount of power available:
 int power_cost = objects_used * INTERFACE_CHARGE_POWER_COST;
 if (power_cost > core->power_left)
		objects_used = core->power_left / INTERFACE_CHARGE_POWER_COST;
// actual power use is recalculated below
//fpr("\n A str_gain %i power_cost %i core->power_left %i", strength_gained, power_cost, core->power_left);
	if (objects_used == 0)
		goto finished_charging;

	strength_gained = objects_used * core->interface_charge_rate_per_object;

//	if (charge_amount > core->interface_charge_rate)
//		charge_amount = core->interface_charge_rate;

// Strength_gained is subject to various limits - need to apply them all:

// Limit strength_gained according to the number of interface_depth objects and the core type:
 int max_charge = (core->interface_max_charge_rate);
 if (strength_gained > max_charge)
	{
		strength_gained = max_charge;
		strength_gained_adjusted = 1;
	}

// Limit strength_gained to avoid exceeding maximum strength:
	if (strength_gained + core->interface_strength	> core->interface_strength_max)
	{
		strength_gained = core->interface_strength_max - core->interface_strength;
		strength_gained_adjusted = 1;
	}


// may need to recalculate power_cost based on the actual amount charged
  if (strength_gained_adjusted)
		{
		 power_cost = (strength_gained + (core->interface_charge_rate_per_object - 1))
		              / core->interface_charge_rate_per_object;
		 power_cost *= INTERFACE_CHARGE_POWER_COST;
		}

	core->interface_strength += strength_gained;
	//core->interface_charged_this_cycle += strength_gained;
	core->interface_charged_time = w.world_time;
//	core->power_used += POWER_COST_INTERFACE_CHARGE_1 * strength_gained;
	core->power_left -= power_cost;//INTERFACE_CHARGE_POWER_COST * strength_gained;
//fpr("\n B str_gain %i power_cost %i core->power_left %i", strength_gained, power_cost, core->power_left);

finished_charging:

// if interface inactive but control_status is on, try to raise (even if no actual strength was gained above):
	if (!core->interface_active
 	&& core->interface_control_status
	 && core->interface_strength > 0)
 {
			try_to_raise_general_interface(core);
	}

	return strength_gained;

}

static s16b try_to_raise_general_interface(struct core_struct* core)
{

// TO DO: put some of these tests in the calling code (as not all of them are relevant in all cases)
 if (core->interface_broken_time >= w.world_time - INTERFACE_BROKEN_TIMER // still in cooldown after being broken by damage
		|| core->interface_strength <= 0 // can't raise if interface not charged
		|| !core->interface_control_status // won't raise if turned off (not sure this test is really needed)
		|| core->interface_active // already raised
		|| !core->interface_available) // unavailable
//		|| core->stress_level >= STRESS_MODERATE)
		return 0; // could return a more useful error code

	core->interface_active = 1;

	int i;

// Problem: no power is used in the cycle during which the interface is raised.
// This doesn't really make sense... fix?
// (power will be used right at the start of the next cycle, though - maybe that's okay)

 for (i = 0; i < core->group_members_max; i ++)
	{
		if (core->group_member[i].exists != 0
			&& w.proc[core->group_member[i].index].interface_protects)
//			&& w.proc[core->group_member[i].index].interface_object_present
//			&& w.proc[core->group_member[i].index].interface_on_process_set_on) // interface objects can be turned off
			 w.proc[core->group_member[i].index].interface_raised_time = w.world_time;
	}

 play_game_sound(SAMPLE_INT_UP, TONE_1G, 160, 1, core->core_position.x, core->core_position.y);

	return 1;

}

// Assumes that the calling function will set core->interface_control_status to 0 if needed
//  - this isn't always needed (e.g. this function is called when stress too high)
s16b lower_general_interface(struct core_struct* core)
{

 if (!core->interface_active // already lowered
		|| !core->interface_available) // unavailable
		return 0; // could return a more useful error code

	core->interface_active = 0;

	int i;

 for (i = 0; i < core->group_members_max; i ++)
	{
		if (core->group_member[i].exists
			&& w.proc[core->group_member[i].index].interface_protects)
//			&& w.proc[core->group_member[i].index].interface_object_present
//			&& w.proc[core->group_member[i].index].interface_on_process_set_on) // interface objects can be turned off
			 w.proc[core->group_member[i].index].interface_lowered_time = w.world_time;
	}

	return 1;

}


struct scanlist_struct scanlist;

// builds a list of all cores in scanning range of scanning_core (or at least SCANLIST_SIZE of them)
// the list can then be used in other scanning functions.
// any scanning function that uses the scanlist should check scanlist.current, and call this if it's false.
static void build_scanlist(struct core_struct* scanning_core)
{

 vmstate.instructions_left -= 16;

	scanlist.list_size = 0;

	al_fixed scan_range = scanning_core->scan_range_fixed;

	int i;
	al_fixed scan_x = scanning_core->core_position.x;
	al_fixed scan_y = scanning_core->core_position.y;
	int scanning_core_index = scanning_core->index;

	int total_cores = w.cores_per_player * w.players;

	for (i = 0; i < total_cores; i ++)
	{
// don't exclude friendly cores
			if (w.core[i].exists == 0
				|| i == scanning_core_index)
				continue;
// need to test both distance and visibility, even though both should have the same range, because the way they're calculated is slightly different.
			if (distance_oct_xyxy(w.core[i].core_position.x, w.core[i].core_position.y,
																									scan_x, scan_y) <= scan_range
				&& w.vision_area[scanning_core->player_index]
				                [w.proc[w.core[i].process_index].block_position.x]
										          [w.proc[w.core[i].process_index].block_position.y].vision_time >= w.world_time - VISION_AREA_VISIBLE_TIME)
//			if (abs(w.core[i].core_position.x - scan_x) + abs(w.core[i].core_position.y - scan_y) <= scan_range)
			{
/*
if (w.vision_area[scanning_core->player_index]
				                [w.proc[w.core[i].process_index].block_position.x]
										          [w.proc[w.core[i].process_index].block_position.y].vision_time < w.world_time - VISION_AREA_VISIBLE_TIME)
*/

				scanlist.index [scanlist.list_size] = i;
				scanlist.core_x [scanlist.list_size] = w.core[i].core_position.x;
				scanlist.core_y [scanlist.list_size] = w.core[i].core_position.y;
//				fpr("\n scanlist scan_range %i oct %i hypot %i", al_fixtoi(scan_range), al_fixtoi(distance_oct_xyxy(w.core[i].core_position.x, w.core[i].core_position.y,
//																									scan_x, scan_y)),
//												al_fixtoi(distance(scan_y - w.core[i].core_position.y, scan_x - w.core[i].core_position.x)));
//				scanlist.scan_bitfield [scanlist.list_size] = w.core[i].scan_bitfield;
				scanlist.list_size ++;
			 if (scanlist.list_size >= SCANLIST_SIZE)
				 break; // unlikely but possible.
			}
	}

	scanlist.current = 1;

}

/*
static s16b scan_for_threat(struct core_struct* core, s16b* stack_parameters)
{

 if (!scanlist.current)
		build_scanlist(core);

	if (scanlist.list_size == 0)
		return 0;

// scan_x and scan_y: this function finds the closest core to scan_x,scan_y
// these aren't bounds-checked - the bounds-checking is only needed to find out which targets are in range of the scan.
	al_fixed scan_x = al_itofix(stack_parameters [0]) + core->core_position.x;
	al_fixed scan_y = al_itofix(stack_parameters [1]) + core->core_position.y;

	int process_memory_address = stack_parameters [2];

	if (process_memory_address < 0
		|| process_memory_address >= PROCESS_MEMORY_SIZE)
	{
 		if (w.debug_mode)
  		print_method_error("invalid targetting memory address for scan call", 1, process_memory_address);
			return -1; // error
	}

	al_fixed scan_range = core->scan_range_fixed;

	int i;
	al_fixed target_core_distance;
	int closest_target_core = -1;
	al_fixed closest_distance = al_itofix(30000);
	int target_core_index;

	for (i = 0; i < scanlist.list_size; i ++)
	{

   target_core_index = scanlist.index [i];

// need to do friendly core exclusion by testing against bitfield
   if (w.core[target_core_index].player_index == core->player_index)
				continue;


// don't need to exclude the scanning core or non-existent cores as they aren't added to the scanlist
			target_core_distance = abs(w.core[target_core_index].core_position.x - scan_x) + abs(w.core[target_core_index].core_position.y - scan_y);
// note that the distance calculation is not precise at all
//  - could use squared values for everything? although would probably need to convert to int to avoid overflowing al_fixed
			if (target_core_distance < closest_distance)
			{
				closest_distance = target_core_distance;
				closest_target_core = target_core_index;
			}
	}

 vmstate.instructions_left -= 8;

 if (closest_target_core == -1
		|| closest_distance > scan_range)
		return 0;

	core->process_memory [process_memory_address] = closest_target_core; // process_memory_address has already been bounds-checked
	core->process_memory_timestamp [process_memory_address] = w.core[closest_target_core].created_timestamp;

 return 1;

}
*/

// called by auto attack methods in g_method.c
// target_index probably should have been verified as valid before calling
//  - this guarantees that a return value of 1 means a target has been put in targetting memory
s16b scan_for_auto_attack(struct core_struct* core, int angle, int scan_distance, int target_index)
{

 s16b mock_stack_parameters [3];

// first two are the x/y offsets of the scan centre:
 mock_stack_parameters [0] = al_fixtoi(fixed_xpart(core->group_angle + int_angle_to_fixed(angle), al_itofix(scan_distance)));
 mock_stack_parameters [1] = al_fixtoi(fixed_ypart(core->group_angle + int_angle_to_fixed(angle), al_itofix(scan_distance)));
 mock_stack_parameters [2] = target_index;

	return scan_single(core, mock_stack_parameters, 0, 100, 0, 0);

}


static s16b scan_single(struct core_struct* core, s16b* stack_parameters, int components_min, int components_max, s16b scan_bitfield, int accept_or_require_friendly)
{

 if (!scanlist.current)
		build_scanlist(core);

	if (scanlist.list_size == 0)
		return 0;

// scan_x and scan_y: this function finds the closest core to scan_x,scan_y
// these aren't bounds-checked - the scanlist has already been bounds-checked.
	al_fixed scan_x = al_itofix(stack_parameters [0]) + core->core_position.x;
	al_fixed scan_y = al_itofix(stack_parameters [1]) + core->core_position.y;

	int process_memory_address = stack_parameters [2];

/*	if (process_memory_address < 0
		|| process_memory_address >= PROCESS_MEMORY_SIZE)
	{
 		if (w.debug_mode)
  		print_method_error("invalid targetting memory address for scan call", 1, process_memory_address);
			return -1; // error
	}*/

//	al_fixed scan_range = core->scan_range_fixed;

	int i;
	al_fixed target_core_distance;
	int closest_target_core = -1;
	al_fixed closest_distance = al_itofix(30000);
	int target_core_index;

	for (i = 0; i < scanlist.list_size; i ++)
	{

   target_core_index = scanlist.index [i];

   if (w.core[target_core_index].player_index == core->player_index)
			{
			 if (!accept_or_require_friendly) // accept_or_require_friendly==0 means ignore friendly completely
				 continue;
			}
			 else
				{
			  if (accept_or_require_friendly == 2)
				  continue;	// accept_or_require_friendly==2 means only accept friendly
				}

//#define SHOW_BITFIELD_COMPARISON

#ifdef SHOW_BITFIELD_COMPARISON
			fpr("\n core %i target %i bitfield: scan ", core->index, target_core_index);
			int b;
			char bitstring [20];
			for (b = 0; b < 16; b ++)
			{
				if ((scan_bitfield & (1 << b)) != 0)
					bitstring [b] = '1';
				  else
							bitstring [b] = '0';
			}
			bitstring [b] = 0;
			fpr("%i [%s] target ", scan_bitfield, bitstring);
			for (b = 0; b < 16; b ++)
			{
				if ((w.core[target_core_index].scan_bitfield & (1 << b)) != 0)
					bitstring [b] = '1';
				  else
							bitstring [b] = '0';
			}
			bitstring [b] = 0;
			fpr("%i [%s]", w.core[target_core_index].scan_bitfield, bitstring);
#endif
   if ((w.core[target_core_index].scan_bitfield & scan_bitfield) != scan_bitfield) // requires the target's bitfield to contain all bits in the scanning test bitfield. It can have others too.
				continue;
			if (w.core[target_core_index].group_members_current < components_min
				|| w.core[target_core_index].group_members_current > components_max)
				continue;


// don't need to exclude the scanning core or non-existent cores as they aren't added to the scanlist
			target_core_distance = distance_oct_xyxy(w.core[target_core_index].core_position.x, w.core[target_core_index].core_position.y, scan_x, scan_y);
//	abs(w.core[target_core_index].core_position.x - scan_x) + abs(w.core[target_core_index].core_position.y - scan_y);
// note that the distance calculation is not precise at all
//  - could use squared values for everything? although would probably need to convert to int to avoid overflowing al_fixed
			if (target_core_distance < closest_distance)
			{
				closest_distance = target_core_distance;
				closest_target_core = target_core_index;
			}
	}

 vmstate.instructions_left -= 8;

 if (closest_target_core == -1)
		return 0;

	if (process_memory_address >= 0
		&& process_memory_address < PROCESS_MEMORY_SIZE)
	{
 	core->process_memory [process_memory_address] = closest_target_core; // process_memory_address has already been bounds-checked
	 core->process_memory_timestamp [process_memory_address] = w.core[closest_target_core].created_timestamp;
 }


 return 1;

}



// more complex scan that involves sorting
static s16b scan_multi(struct core_struct* core, s16b* stack_parameters)
{

 if (!scanlist.current)
		build_scanlist(core);

	if (scanlist.list_size == 0)
		return 0;

// scan_x and scan_y: this function finds the closest core to scan_x,scan_y
// these aren't bounds-checked - the bounds-checking is only needed to find out which targets are in range of the scan.
	al_fixed scan_x = al_itofix(stack_parameters [0]) + core->core_position.x;
	al_fixed scan_y = al_itofix(stack_parameters [1]) + core->core_position.y;


#define MULTI_SCAN_TARGETS 6

	int number_of_targets = stack_parameters [3];
	if (number_of_targets < 0)
		number_of_targets = 0;
	if (number_of_targets > MULTI_SCAN_TARGETS)
		number_of_targets = MULTI_SCAN_TARGETS;

// invalid target memory address means no recording
	int process_memory_address = stack_parameters [2];
	if (process_memory_address < 0
		|| process_memory_address >= PROCESS_MEMORY_SIZE - number_of_targets - 1) // is the - 1 right??
		number_of_targets = 0;


//	int accept_or_require_friendly = stack_parameters [4]; // none of these really need to be bounds-checked so the stack_parameters values can be used directly
//	int components_min = stack_parameters [5];
//	int components_max = stack_parameters [6];


// {8}, // SMETHOD_CALL_SCAN_MULTI // (x_offset, y_offset, target memory, number of targets, accept_or_require_friendly, components_min, components_max, scan_bitfield)

//	al_fixed scan_range = core->scan_range_fixed;

	int i, j, k;
	al_fixed target_core_distance;
	int targets_found = 0; // return value. can be greater than the number specified by the call (extra targets found just won't be recorded)
//	int targets_recorded = 0; // capped at the number specified by the call
	int target_core_list [MULTI_SCAN_TARGETS];
	al_fixed target_distance [MULTI_SCAN_TARGETS];
	for (i = 0; i < MULTI_SCAN_TARGETS; i ++)
	{
		target_core_list [i] = -1;
		target_distance [i] = al_itofix(30000);
	}
//	al_fixed furthest_distance = al_itofix(30000);
//	int closest_target_core = -1;
//	al_fixed closest_distance = al_itofix(30000);
	int target_core_index;

	for (i = 0; i < scanlist.list_size; i ++)
	{

   target_core_index = scanlist.index [i];

   if (w.core[target_core_index].player_index == core->player_index
			 && !stack_parameters [4]) // stack_parameters [4] is accept_or_require_friendly
				continue;
   if ((w.core[target_core_index].scan_bitfield & stack_parameters [7]) != stack_parameters [7])
				continue;
			if (w.core[target_core_index].group_members_current < stack_parameters [5] // min & max components
				|| w.core[target_core_index].group_members_current > stack_parameters [6])
				continue;

// don't need to exclude the scanning core or non-existent cores as they aren't added to the scanlist
			target_core_distance = distance_oct_xyxy(w.core[target_core_index].core_position.x, w.core[target_core_index].core_position.y, scan_x, scan_y);
//			abs(w.core[target_core_index].core_position.x - scan_x) + abs(w.core[target_core_index].core_position.y - scan_y);
// note that the distance calculation is not precise at all
//  - could use squared values for everything? although would probably need to convert to int to avoid overflowing al_fixed

			targets_found ++;

// if not recording any targets, can continue here as targets don't need to be sorted.
   if (number_of_targets == 0)
				continue;

/*
// fill up the target list:
			if (targets_found < number_of_targets)
			{
				target_core_list [targets_found] = target_core_index;
				target_distance [targets_found] = target_core_distance;
				targets_found ++;
				continue;
			}

// target list full. So we may need to sort.
*/


// first, reject case where target is further than furthest target in sorted list:
//			if (target_core_distance >= furthest_distance)
//				continue;

// now count through target list (not sure if target list is large enough to justify a better sorting method)

			j = 0;
			k = 0;

//Need to fix this:
			while (j < number_of_targets)
			{
				if (target_core_distance < target_distance [j])
				{
					k = number_of_targets - 1;
					while (k > j)
					{
// push the list back to make room
						target_core_list [k] = target_core_list [k-1];
						target_distance [k] = target_distance [k-1];
						k--;
					}
					target_core_list [j] = target_core_index;
					target_distance [j] = target_core_distance;
//					targets_recorded ++; no don't do this - here, we know that the new target is replacing another one
					break;
				}
				j++;
			}

	}

// if the scan is just to count targets, not record them, we can just return the number found
 if (number_of_targets == 0)
	{
		vmstate.instructions_left -= 12; // cheaper as no sorting required
		return targets_found;
	}

 vmstate.instructions_left -= 36;// + (targets_found * 4);??

 if (targets_found == 0)
		return 0;

	int targets_to_record = targets_found;
	if (targets_to_record > number_of_targets)
		targets_to_record = number_of_targets;
// number_of_targets is capped at MULTI_SCAN_TARGETS above

	for (i = 0; i < targets_to_record; i ++)
	{
// should be able to assume (based on checks above) that there is enough room in process memory
//	 if (process_memory_address + i >= PROCESS_MEMORY_SIZE)
//		 return targets_found;

	 core->process_memory [process_memory_address + i] = target_core_list [i]; // process_memory_address(+SCAN_MULTI_TARGETS) has already been bounds-checked
	 core->process_memory_timestamp [process_memory_address + i] = w.core[target_core_list [i]].created_timestamp;
	}

 return targets_found;

}



/*

Scan calls:

scan_single(int x_offset, int y_offset, int address, int team_bitfield, int pass_filter, int reject_filter);
scan_multi(int x_offset, int y_offset, int address, int number, int team_bitfield, int pass_filter, int reject_filter);

to use team bitfield:
 - new core function (not std function): get_user() - returns player index of core

pass/reject bitfields:
sfilter_single_member,
sfilter_small, // < 4 members
sfilter_medium, // 4-9 members
sfilter_large, // 10+ members
sfilter_allocator, // has allocator object
sfilter_gather, // has gather object
sfilter_storage, // has storage object

*/


/*
// should be able to merge this with scan_for_threat
static s16b scan_single(struct core_struct* core, s16b* stack_parameters)
{

 if (!scanlist.current)
		build_scanlist(core);

	if (scanlist.list_size == 0)
		return 0;

// scan_x and scan_y: this function finds the closest core to scan_x,scan_y
// these aren't bounds-checked - the bounds-checking is only needed to find out which targets are in range of the scan.
	al_fixed scan_x = al_itofix(stack_parameters [0]) + core->core_position.x;
	al_fixed scan_y = al_itofix(stack_parameters [1]) + core->core_position.y;

	int process_memory_address = stack_parameters [2];

	if (process_memory_address < 0
		|| process_memory_address >= PROCESS_MEMORY_SIZE)
			return -1; // error

	al_fixed scan_range = core->scan_range_fixed;



 return 1;

}
*/


static s16b scan_repair(struct core_struct* core, s16b* stack_parameters, int repair_amount)
{

 if (!scanlist.current)
		build_scanlist(core);

	if (scanlist.list_size == 0)
		return -1;

// scan_x and scan_y: this function finds the closest friendly core to scan_x,scan_y which has at least 1 damaged component
// these aren't bounds-checked - the bounds-checking is only needed to find out which targets are in range of the scan.
	al_fixed scan_x = al_itofix(stack_parameters [0]) + core->core_position.x;
	al_fixed scan_y = al_itofix(stack_parameters [1]) + core->core_position.y;

	int i;
	al_fixed target_core_distance;
	int closest_target_core = -1;
	al_fixed closest_distance = al_itofix(30000);
	int target_core_index;

	for (i = 0; i < scanlist.list_size; i ++)
	{

   target_core_index = scanlist.index [i];

// don't need to exclude the scanning core or non-existent cores as they aren't added to the scanlist
   if (w.core[target_core_index].player_index != core->player_index
				|| w.core[target_core_index].group_total_hp == w.core[target_core_index].group_total_hp_max_current) // no damage to repair
				continue;

			target_core_distance = distance_oct_xyxy(w.core[target_core_index].core_position.x, w.core[target_core_index].core_position.y, scan_x, scan_y);
//	abs(w.core[target_core_index].core_position.x - scan_x) + abs(w.core[target_core_index].core_position.y - scan_y);
// note that the distance calculation is not precise at all
//  - could use squared values for everything? although would probably need to convert to int to avoid overflowing al_fixed
			if (target_core_distance < closest_distance)
			{
				closest_distance = target_core_distance;
				closest_target_core = target_core_index;
			}
	}

 vmstate.instructions_left -= 8;

 if (closest_target_core == -1)
		return -1;

//	core->process_memory [process_memory_address] = closest_target_core; // process_memory_address has already been bounds-checked
//	core->process_memory_timestamp [process_memory_address] = w.core[closest_target_core].created_timestamp;

 return repair_process(&w.core[closest_target_core], core);

}


static s16b scan_restore(struct core_struct* core, s16b* stack_parameters)
{

 if (!scanlist.current)
		build_scanlist(core);

	if (scanlist.list_size == 0)
		return -1;

// scan_x and scan_y: this function finds the closest friendly core to scan_x,scan_y which has at least 1 damaged component
// these aren't bounds-checked - the bounds-checking is only needed to find out which targets are in range of the scan.
	al_fixed scan_x = al_itofix(stack_parameters [0]) + core->core_position.x;
	al_fixed scan_y = al_itofix(stack_parameters [1]) + core->core_position.y;

	int i;
	al_fixed target_core_distance;
	int closest_target_core = -1;
	al_fixed closest_distance = al_itofix(30000);
	int target_core_index;

	for (i = 0; i < scanlist.list_size; i ++)
	{

   target_core_index = scanlist.index [i];

// don't need to exclude the scanning core or non-existent cores as they aren't added to the scanlist
   if (w.core[target_core_index].player_index != core->player_index
				|| w.core[target_core_index].group_members_current == w.core[target_core_index].group_members_max) // no destroyed components to restore
				continue;

			target_core_distance = distance_oct_xyxy(w.core[target_core_index].core_position.x, w.core[target_core_index].core_position.y, scan_x, scan_y);
			//abs(w.core[target_core_index].core_position.x - scan_x) + abs(w.core[target_core_index].core_position.y - scan_y);
// note that the distance calculation is not precise at all
//  - could use squared values for everything? although would probably need to convert to int to avoid overflowing al_fixed
			if (target_core_distance < closest_distance)
			{
				closest_distance = target_core_distance;
				closest_target_core = target_core_index;
			}
	}

 vmstate.instructions_left -= 8;

 if (closest_target_core == -1)
		return -1;

//	core->process_memory [process_memory_address] = closest_target_core; // process_memory_address has already been bounds-checked
//	core->process_memory_timestamp [process_memory_address] = w.core[closest_target_core].created_timestamp;

 return restore_process(&w.core[closest_target_core], core);

}




// need to rename this as it's not a real scan function
static s16b check_point(struct core_struct* core, s16b* stack_parameters)
{

	int process_memory_address = stack_parameters [2];

	if (process_memory_address < 0
		|| process_memory_address >= PROCESS_MEMORY_SIZE)
			return -1;

	if (distance_oct(al_itofix(stack_parameters [0]), al_itofix(stack_parameters [1])) > core->scan_range_fixed)
			return -1;

	int target_process = check_point_collision(al_itofix(stack_parameters [0]) + core->core_position.x, al_itofix(stack_parameters [1]) + core->core_position.y, 1);
	if (target_process == -1)
		return 0;

	int found_core = w.proc[target_process].core_index;

	core->process_memory [process_memory_address] = found_core; // process_memory_address has already been bounds-checked
	core->process_memory_timestamp [process_memory_address] = w.core[found_core].created_timestamp;

	return 1;
}


static int find_first_build_queue_entry(int player_index, int core_index)
{

	int i;

	for (i = 0; i < BUILD_QUEUE_LENGTH; i ++)
	{
		if (!w.player[player_index].build_queue[i].active)
			return -1;
		if (w.player[player_index].build_queue[i].core_index == core_index)
			return i;
	}

	return -1; // should never be reached

}


// returns 0 if too near a data well, 1 if okay
int check_static_build_location_for_data_wells(al_fixed build_x, al_fixed build_y)
{

	int i;

	for (i = 0; i < DATA_WELLS; i ++)
	{
		if (w.data_well[i].active
		 && distance_oct_xyxy(build_x, build_y, w.data_well[i].position.x, w.data_well[i].position.y) < w.data_well[i].static_build_exclusion)
		 return 0; // fail
// distance_oct_xyxy is probably fast enough that a first bounding box check isn't needed
	}

 return 1; // all good

}



/*

Let's work out how processes will communicate with each other.

#define MESSAGES 8
// MESSAGES is the number of messages a process can receive each cyle. If more are received, priority 1 messages replace priority 0. Messages that don't fit are left out.
#define MESSAGE_LENGTH 8
// MESSAGE_LENGTH is max number of ints in each message
#define BROADCAST_RANGE 800
// BROADCAST_RANGE should probably match default scanning range
#define CHANNELS 8
// processes can ignore channels they don't need to listen to

enum
{
MESSAGE_TYPE_NONE,
MESSAGE_TYPE_TELL,
MESSAGE_TYPE_BROADCAST

}

struct message_struct
{
 int type; // MESSAGE_TYPE enum
 int source_index; // index of sending core
 timestamp source_index_timestamp; // creation time of sending core
 cart source_position; // position when message was sent (absolute)
 int channel; // channel message was sent on

 int priority; // 0 or 1 (could have more priorities but that would require annoying sorting)
 int length; // number of ints in message (up to MESSAGE_LENGTH)

};


message_struct message [MESSAGES];
int messages_received;
int message_reading; // index of current message being read by process (up
int listen_channel [CHANNELS];

standard methods:




tell(<process target>, channel, priority, <message contents> (up to MESSAGE_LENGTH; remainder just sent as zeros))
broadcast(range, channel, priority, <message contents>)

check_messages() - returns number of messages
get_message_type()
get_message_channel()
get_message_source(target index)
get_message_x()
get_message_y()
get_message_priority()
read_message()

next_message() // move to next message. returns 1 if there is one, otherwise 0
// no need for clear_all_messages() as messages are just reset each cycle

// default is to ignore all channels (could do the other way, but this works as a slight optimisation)
ignore_channel(channel index)
listen_channel(channel index)
ignore_all_channels()


*/

