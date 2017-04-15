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
#include "g_proc_new.h"
#include "m_globvars.h"
#include "m_maths.h"
#include "t_template.h"
#include "x_sound.h"

#include "i_console.h"
#include "i_disp_in.h"
#include "i_background.h"
#include "g_method_core.h"
#include "g_method_std.h"
#include "c_keywords.h"
#include "h_story.h"

#include "v_interp.h"

extern struct view_struct view; // TO DO: think about putting a pointer to this in the worldstruct instead of externing it
extern struct control_struct control; // defined in i_input.c. Used here for client process methods.
extern struct game_struct game;
//extern struct object_type_struct otype [OBJECT_TYPES];
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct vmstate_struct vmstate; // defined in v_interp.c

//static s16b repair_process(struct core_struct* target_core, struct core_struct* repairer, int repair_amount);
//static s16b repair_specific_component(struct core_struct* target_core, int component_index, int repair_amount);

static void run_packet_object(struct core_struct* core, struct proc_struct* proc, int object_index);
static void run_stream_object(struct core_struct* core, struct proc_struct* proc, int object_index, int object_type);
static void run_spike_object(struct core_struct* core, struct proc_struct* proc, int object_index, int firing_angle_offset_int);
static void run_slice_object(struct core_struct* core, struct proc_struct* proc, int object_index, int object_type);
//static void run_burst_object(struct core_struct* core, struct proc_struct* proc, int object_index);

//static void rotate_directional_method(int* data_angle, al_fixed* ex_angle, s16b mbank_angle, int turn_speed, int shape, int vertex);
static void rotate_directional_object(struct proc_struct* proc, int object_index, al_fixed turn_speed);

void set_motion_from_move_objects(struct core_struct* core);

//static int check_member_and_object_indices(struct core_struct* core, int member_index, int object_index);

static al_fixed lead_target(struct core_struct* firing_core,
																												struct proc_struct* firing_proc,
																												int object_index,
																												struct core_struct* target_core,
																												struct proc_struct* target_proc,
																			         int intercept_speed);

static al_fixed lead_target_with_fixed_object(struct core_struct* firing_core,
																												struct proc_struct* firing_proc,
																												int object_index,
																												struct core_struct* target_core,
																												struct proc_struct* target_proc,
																			         int intercept_speed);

static al_fixed get_spike_target_angle(struct core_struct* firing_core,
																																struct proc_struct* firing_proc,
																																int object_index,
																																struct core_struct* target_core,
																												    struct proc_struct* target_proc);

static void calculate_move_and_turn(struct core_struct* core, struct proc_struct* proc, int object_index, al_fixed target_angle);
static void calculate_turn(struct core_struct* core, struct proc_struct* proc, int object_index, al_fixed target_angle);
static void stand_off_angle(struct core_struct* core, struct proc_struct* proc, int object_index, al_fixed target_angle, al_fixed target_distance, al_fixed stand_off_distance);
static void calculate_retro_move_and_turn(struct core_struct* core, struct proc_struct* proc, int object_index, al_fixed target_angle);

static int object_uses_power(struct core_struct* core, int power_cost);
/*
static al_fixed lead_target(al_fixed relative_x, al_fixed relative_y,
																							al_fixed relative_speed_x, al_fixed relative_speed_y,
																			    al_fixed base_angle,
																			    int intercept_speed);*/
/*


Let's work out how power/stress will work.





*/


//#define STREAM_IRPT_COST 32

#define CALL_PARAMETERS 6


//int call_known_object(struct core_struct* core, struct proc_struct* proc, int object_index, int call_value, s16b* stack_parameters);
static int call_known_object_or_class(struct core_struct* core, struct proc_struct* proc, int class_index, int object_index, int call_value, s16b* stack_parameters);


struct call_type_struct call_type [CALL_TYPES] =
{
// {int parameters},
	{2, KEYWORD_OMETHOD_MOVE_TO}, // *CALL_MOVE_TO (x, y)
	{2, KEYWORD_OMETHOD_TURN_TO_XY}, // *CALL_TURN_TO_XY (x, y)
	{1, KEYWORD_OMETHOD_TURN_TO_ANGLE}, // *CALL_TURN_TO_ANGLE (angle)
	{2, KEYWORD_OMETHOD_TURN_TO_TARGET}, // *CALL_TURN_TO_TARGET (<process> target, component)
	{3, KEYWORD_OMETHOD_TRACK_TARGET}, // *CALL_TRACK_TARGET (<process> target, component, class index)
	{3, KEYWORD_OMETHOD_APPROACH_XY}, // *CALL_APPROACH_XY (x, y, distance)
	{3, KEYWORD_OMETHOD_APPROACH_TARGET}, // *CALL_APPROACH_TARGET (<process> target, component, distance)
	{4, KEYWORD_OMETHOD_APPROACH_TRACK}, // *CALL_APPROACH_TRACK (<process> target, component, class index, distance)
	{3, KEYWORD_OMETHOD_REPOSITION}, // *CALL_REPOSITION (x, y, angle)
	{1, KEYWORD_OMETHOD_SET_POWER}, // *CALL_SET_POWER (power)
	{1, KEYWORD_OMETHOD_FIRE}, // *CALL_FIRE (firing delay 0-15)
	{1, KEYWORD_OMETHOD_ROTATE}, // *CALL_ROTATE (target_angle_offset)

 {0, KEYWORD_OMETHOD_NO_TARGET}, // *CALL_NO_TARGET
	{2, KEYWORD_OMETHOD_AIM_AT}, // *CALL_AIM_AT (<process> target, component)
	{2, KEYWORD_OMETHOD_FIRE_AT}, // *CALL_FIRE_AT (<process> target, component)

	{3, KEYWORD_OMETHOD_INTERCEPT}, // *CALL_INTERCEPT (<process> target, component, class index)
 {0, KEYWORD_OMETHOD_GATHER_DATA}, // *CALL_GATHER_DATA
 {2, KEYWORD_OMETHOD_GIVE_DATA}, // *CALL_GIVE_DATA (<process> target, data given)
 {2, KEYWORD_OMETHOD_TAKE_DATA}, // *CALL_TAKE_DATA (<process> target, data given)
 {1, KEYWORD_OMETHOD_ALLOCATE_DATA}, // *CALL_ALLOCATE_DATA
 {1, KEYWORD_OMETHOD_FIRE_SPIKE}, // *CALL_FIRE_SPIKE (angle_offset)
 {2, KEYWORD_OMETHOD_FIRE_SPIKE_AT}, // *CALL_FIRE_SPIKE_AT (<process> target, component)
 {2, KEYWORD_OMETHOD_FIRE_SPIKE_XY}, // *CALL_FIRE_SPIKE_XY (x, y)

// spike needs some special methods because of the way it is targetted
// {1}, // *CALL_SET_INTERFACE (on/off)
 {3, KEYWORD_OMETHOD_ATTACK_SCAN}, // *CALL_ATTACK_SCAN (angle_offset, scan_distance, <process> target)
 {3, KEYWORD_OMETHOD_ATTACK_SCAN_AIM}, // *CALL_ATTACK_SCAN_AIM (angle_offset, scan_distance, <process> target)
// remember that parameter numbers do not include member_index and object_index, or class_index.

 {1, KEYWORD_OMETHOD_SET_STABILITY}, // *CALL_SET_STABILITY (on/off)

};



#define DEFAULT_INTERCEPT_SPEED 4
// this is used by the target leading methods (see g_method.c). It shouldn't ever actually be used but might be if a process calls intercept with reference to a non-attacking class for some reason.
#define DEFAULT_POWER_USE 0
// power_use is only relevant for a few kinds of objects.
#define DEFAULT_RECYCLE_TIME 0
// similar


struct object_type_struct otype [OBJECT_TYPES] =
{
// name, keyword, base type, unlock_index, data_cost, power_use_peak, power_use_smoothed, power_use_base,
//  {only_zero_angle_offset, attack_type, packet_speed, power_cost, recycle_time, packet_size, damage, rotate_speed}
	{
		"none", KEYWORD_OBJECT_NONE, OBJECT_BASE_TYPE_NONE, UNLOCK_NONE, 0, 0, 0, {1, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED}, // OBJECT_TYPE_NONE
	},
	{
		"uplink", KEYWORD_OBJECT_UPLINK, OBJECT_BASE_TYPE_LINK, UNLOCK_NONE, 1, 0, 0, {0, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED},  // OBJECT_TYPE_UPLINK
	},
	{
		"downlink", KEYWORD_OBJECT_DOWNLINK, OBJECT_BASE_TYPE_LINK, UNLOCK_NONE, 1, 0, 0, {0, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED}, // OBJECT_TYPE_DOWNLINK
	},
	{
		"move", KEYWORD_OBJECT_MOVE, OBJECT_BASE_TYPE_MOVE, UNLOCK_NONE, 2,
		10, // power_use_peak
		10, // power_use_base
		{0, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED}, // OBJECT_TYPE_MOVE
	},
	{"pulse", KEYWORD_OBJECT_PULSE, OBJECT_BASE_TYPE_ATTACK, UNLOCK_NONE, 4,
		20, // power_use_peak
		0, // power_use_base
	 {0, ATTACK_TYPE_PULSE, 8, 20, 48, 0, 16, 65536}, }, // OBJECT_TYPE_PULSE
	{"pulse_l", KEYWORD_OBJECT_PULSE_L, OBJECT_BASE_TYPE_ATTACK, UNLOCK_OBJECT_PULSE_L, 8,
		40, // power_use_peak
		0, // power_use_base
	 {0, ATTACK_TYPE_PULSE, 6, 40, 64, 1, 32, 44000}, }, // OBJECT_TYPE_PULSE_L
	{"pulse_xl", KEYWORD_OBJECT_PULSE_XL, OBJECT_BASE_TYPE_ATTACK, UNLOCK_OBJECT_PULSE_XL, 12,
		80, // power_use_peak
		0, // power_use_base
	 {0, ATTACK_TYPE_PULSE, 5, 80, 80, 2, 96, 20000}, }, // OBJECT_TYPE_PULSE_XL
	{"burst", KEYWORD_OBJECT_BURST, OBJECT_BASE_TYPE_ATTACK, UNLOCK_NONE, 3,
		20, // power_use_peak
		0, // power_use_base
	 {0, ATTACK_TYPE_BURST, 8, 20, 48, 0, 20}, }, // OBJECT_TYPE_BURST
	{"burst_l", KEYWORD_OBJECT_BURST_L, OBJECT_BASE_TYPE_ATTACK, UNLOCK_OBJECT_PULSE_L, 6,
		40, // power_use_peak
		0, // power_use_base
	 {0, ATTACK_TYPE_BURST, 6, 40, 64, 1, 40}, }, // OBJECT_TYPE_BURST_L

//* consider making damage etc the same for pulse and burst, and just have pulse be more expensive

	{"burst_xl", KEYWORD_OBJECT_BURST_XL, OBJECT_BASE_TYPE_ATTACK, UNLOCK_OBJECT_PULSE_XL, 10,
		80, // power_use_peak
		0, // power_use_base
	 {0, ATTACK_TYPE_BURST, 5, 80, 80, 2, 120}, }, // OBJECT_TYPE_BURST_XL
	{
		"build", KEYWORD_OBJECT_BUILD, OBJECT_BASE_TYPE_STD, UNLOCK_NONE, 64,
		BUILD_POWER_COST, // power_use_peak
		0, // power_use_base
		{1, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED}, // OBJECT_TYPE_BUILD
	},
	{
		"interface", KEYWORD_OBJECT_INTERFACE, OBJECT_BASE_TYPE_DEFEND, UNLOCK_OBJECT_INTERFACE, 16,
		INTERFACE_POWER_USE + INTERFACE_CHARGE_POWER_COST, // power_use_peak
		INTERFACE_POWER_USE, // power_use_base
		{1, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED},  // OBJECT_TYPE_INTERFACE
	},
/*	{
		"interface_depth", KEYWORD_OBJECT_INTERFACE_DEPTH, OBJECT_BASE_TYPE_DEFEND, 8,
  INTERFACE_CHARGE_PER_DEPTH_OBJECT * INTERFACE_CHARGE_POWER_COST,
//  1000,
//		(INTERFACE_STRENGTH_PER_OBJECT / INTERFACE_CHARGE_RATE_FACTOR) * INTERFACE_CHARGE_POWER_COST, // power_use_peak
//		(INTERFACE_STRENGTH_PER_OBJECT / INTERFACE_CHARGE_RATE_FACTOR) * INTERFACE_CHARGE_POWER_COST, // power_use_smoothed
		0, // power_use_base
		{1, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED},  // OBJECT_TYPE_INTERFACE_DEPTH
	},*/
/*	{
		"interface_sta", KEYWORD_OBJECT_INTERFACE_STABILITY, OBJECT_BASE_TYPE_DEFEND, 4, {1, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED}, // OBJECT_TYPE_INTERFACE_STABILITY
	},
	{
		"interface_res", KEYWORD_OBJECT_INTERFACE_RESPONSE, OBJECT_BASE_TYPE_DEFEND, 4, {1, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED},  // OBJECT_TYPE_INTERFACE_RESPONSE
	},*/
	{
		"harvest", KEYWORD_OBJECT_HARVEST, OBJECT_BASE_TYPE_STD, UNLOCK_NONE, 12,
		POWER_COST_GATHER_BASE, // + (HARVEST_RATE * POWER_COST_GATHER_1_DATA), // power_use_peak - may not be quite right as give_data has different costs
		0, //((POWER_COST_GATHER_BASE + (HARVEST_RATE * POWER_COST_GATHER_1_DATA)) * 16) / HARVEST_RECYCLE_TIME, // power_use_smoothed
//		0, // power_use_base
		{1, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED},  // OBJECT_TYPE_HARVEST
	},
	{
		"storage", KEYWORD_OBJECT_STORAGE, OBJECT_BASE_TYPE_STD, UNLOCK_NONE, 2, 0, 0, {1, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED},  // OBJECT_TYPE_STORAGE
	},
	{
		"allocate", KEYWORD_OBJECT_ALLOCATE, OBJECT_BASE_TYPE_STD, UNLOCK_NONE, 64,
		ALLOCATE_RATE * POWER_COST_ALLOCATE_1_DATA, // power_use_peak
//		ALLOCATE_RATE * POWER_COST_ALLOCATE_1_DATA, // power_use_smoothed
		0, // power_use_base
		{1, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED}, // OBJECT_TYPE_ALLOCATE
	},
	{
#define POWER_COST_STREAM 160
		"stream", KEYWORD_OBJECT_STREAM, OBJECT_BASE_TYPE_ATTACK, UNLOCK_OBJECT_STREAM, 32,
		POWER_COST_STREAM, // power_use_peak
		0, // power_use_base
	 {0, ATTACK_TYPE_BURST, 140, POWER_COST_STREAM, STREAM_RECYCLE_TIME, 5, 7}, // OBJECT_TYPE_STREAM
	},
	{
		"stream_dir", KEYWORD_OBJECT_STREAM_DIR, OBJECT_BASE_TYPE_ATTACK, UNLOCK_OBJECT_STREAM, 36,
		POWER_COST_STREAM, // power_use_peak
		0, // power_use_base
	 {0, ATTACK_TYPE_PULSE, 140, POWER_COST_STREAM, STREAM_RECYCLE_TIME, 5, 5, 20000}, // OBJECT_TYPE_STREAM_DIR
	},
	{
		"spike", KEYWORD_OBJECT_SPIKE, OBJECT_BASE_TYPE_ATTACK, UNLOCK_OBJECT_SPIKE, 12,
		POWER_COST_SPIKE,
		0, // power_use_base
	 {1, ATTACK_TYPE_SPIKE, 4, POWER_COST_SPIKE, 256, 5, SPIKE_BASE_DAMAGE}, // OBJECT_TYPE_SPIKE - damage is adjusted by range
	},
	{
		"repair", KEYWORD_OBJECT_REPAIR, OBJECT_BASE_TYPE_DEFEND, UNLOCK_NONE, 8,
		POWER_COST_RESTORE_COMPONENT*2,
		0, // power_use_base
		{1, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED}, // OBJECT_TYPE_REPAIR
	},
	{
		"repair_other", KEYWORD_OBJECT_REPAIR_OTHER, OBJECT_BASE_TYPE_DEFEND, UNLOCK_OBJECT_REPAIR_OTHER, 24,
		POWER_COST_RESTORE_COMPONENT*2,
		0, // power_use_base
		{1, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED}, // OBJECT_TYPE_REPAIR_OTHER
	},
	{"ultra", KEYWORD_OBJECT_ULTRA, OBJECT_BASE_TYPE_ATTACK, UNLOCK_OBJECT_ULTRA, 48,
		200, // power_use_peak
		0, // power_use_base
	 {0, ATTACK_TYPE_BURST, 6, 200, 192, 3, 800}, }, // OBJECT_TYPE_ULTRA
	{"ultra_dir", KEYWORD_OBJECT_ULTRA_DIR, OBJECT_BASE_TYPE_ATTACK, UNLOCK_OBJECT_ULTRA, 60,
		200, // power_use_peak
		0, // power_use_base
	 {0, ATTACK_TYPE_PULSE, 5, 200, 192, 3, 600, 20000}, // OBJECT_TYPE_ULTRA_DIR
	},
	{
		"slice", KEYWORD_OBJECT_SLICE, OBJECT_BASE_TYPE_ATTACK, UNLOCK_OBJECT_SLICE, 18,
		POWER_COST_SLICE, // power_use_peak
		0, // power_use_base
		{0, ATTACK_TYPE_PULSE, 100, POWER_COST_SLICE, SLICE_RECYCLE_TIME, 3, 3, 50000}, // OBJECT_TYPE_SLICE
	},
	{
		"stability", KEYWORD_OBJECT_STABILITY, OBJECT_BASE_TYPE_DEFEND, UNLOCK_OBJECT_STABILITY, 8,
		INTERFACE_STABILITY_POWER_USE, // currently 30
		INTERFACE_STABILITY_POWER_USE, // power_use_base
		{1, ATTACK_TYPE_NONE, DEFAULT_INTERCEPT_SPEED}, // OBJECT_TYPE_STABILITY
	},

};


/*

Revised attack method approach:

packet, packet_l, packet_xl, packet_xxl
 - rotating
 - increasing size means:
		- higher data cost
		- more damage
		- more power use
		- slower packet
		- slower rotation
		- slightly slower recycle?
	- I *think* range should stay about the same

pulse, pulse_l etc
 - just like packet but non-rotating. Use burst graphics.

spike
	- as now

stream
 - as now but need to fix power and targetting so it's useful





*/




// returns 1 if okay to continue, 0 if something happened that should cease program execution (not sure this is currently supported; probably set vmstate error instead)
int call_object_method(struct core_struct* core, int call_value)
{

// fpr("\ncall_object %i", call_value);

	s16b stack_parameters [CALL_PARAMETERS+2]; // +2 is for component, object index

	if (call_value < 0
		|| call_value >= CALL_TYPES)
	{
		if (w.debug_mode)
 		print_method_error("invalid object call type", 1, call_value);
		return 0;
	}

	if (!pull_values_from_stack(stack_parameters, call_type[call_value].parameters + 2))
	{
		if (w.debug_mode)
 		print_method_error("object call stack failure", 0, 0); // error message could be more informative
		return 0;
	}

	int member_index = stack_parameters [0];
	int object_index = stack_parameters [1];

//	fpr(" member_index %i, object_index %i", member_index, object_index);

	if (member_index < 0
		|| member_index >= GROUP_MAX_MEMBERS)
			return 0;

	int proc_index = core->group_member [member_index].index;

//	fpr(" proc_index %i ", proc_index);


	if (proc_index == -1)
	{
//		print_method_error("", 0, 0); // don't print an error - this can happen just because part of a process has been destroyed
		return 0;
	}

	struct proc_struct* proc = &w.proc[proc_index];
// shouldn't need to test for proc->exists, as proc_index would be -1 and this function would already have returned.

// process/objects that exist in template but not in process (e.g. process partly destroyed) should probably give a different error message (if any)

 if (object_index < 0
//	 || object_index >= MAX_OBJECTS
	 || object_index >= proc->nshape_ptr->links) // probably best not to assume that objects above this but within MAX_OBJECTS are empty
 {
		if (w.debug_mode)
	 	print_method_error("object call object index invalid", 1, object_index);
  return 0;
 }

// Now we know that the object being called exists.

 return call_known_object_or_class(core, proc, -1, object_index, call_value, &stack_parameters [2]);

}

int pull_values_from_stack(s16b* stack_parameters, int params)
{

	if (vmstate.stack_pos <= params) // <=? or just <?
	{
		return 0;
	}

	int i;

	for (i = params - 1; i >= 0; i --)
	{
		stack_parameters [i] = vmstate.vm_stack [--vmstate.stack_pos];
	}

	return 1;

}


// This function:
//  - calls an object on a process where both the process and object have been confirmed to exist.
//  - calls a class in a core where the class has been confirmed to exist (although it may have no currently existing members)
// Note that if called by class, stack_parameters will be the same for each class member.
// Doesn't assume that the call_type and stack parameters are valid for the particular object type.
//  - if called for a particular object, class_index is -1
//  - if called for a class, object_index is -1 and proc is NULL (but both may be changed)
static int call_known_object_or_class(struct core_struct* core, struct proc_struct* proc, int class_index, int object_index, int call_value, s16b* stack_parameters)
{
// stack_parameters should be pointer to the start of the specific parameters (i.e. after member and object index, or class index)

// switch(proc->object[object_index].type)

// fpr("\ncall_known_object: core %i proc %i obj_index %i call_value %i stack_params [0] %i [1] %i [2] %i [3] %i", core->index, proc->index, object_index, call_value, stack_parameters [0], stack_parameters [1], stack_parameters [2], stack_parameters [3]);

 struct template_struct* calling_template = NULL; // to avoid compiler warning
 int return_value = 0;
 int class_member_index = -1; // don't use this value until it's incremented before the class member search loop!
 int call_initialised = 0; // this is set to 1 after the method is successfully run for the first object. When 1, some method types assume certain calculations have already been performed.
 int call_finished = 0; // set to 1 if a call does something that concludes a class call (e.g. fails to find a target)
	al_fixed target_x, target_y, target_distance = 0, stand_off_distance = 0; // initialised to 0 to avoid compiler warning about being used uninitialised (which I'm pretty sure can't actually happen)
	int number_of_attacks = 0; // used for attack modes with limited numbers of attacks
	int reposition_mode = 0; // used for reposition calls. initialised to avoid compiler warning.

 al_fixed target_angle = 0;
 al_fixed target_angle_offset = 0; // used for e.g. intercept call (which needs to preserve the result of lead_target between move objects)
  // both are initialised to avoid compiler warning

 int target_visibility;
 struct core_struct* target_core;
 struct proc_struct* target_proc = NULL; // initialised to avoid compiler warning


 if (class_index != -1)
	{
		calling_template = &templ[core->player_index][core->template_index];
	}

 while(TRUE)
	{
// This loop runs once for direct member/object calls
//  and once for each class member for class calls.
// For class calls, it resets proc and object_index each time through the loop.
// Some method types have code that is only executed the first time through the loop (such as expensive target verification code).

		if (class_index != -1)
		{
// search for the next class member in the template's list for this class:
	  class_member_index++; // note that this is init'd to -1 above
		 while (TRUE)
		 {
	 	 if (class_member_index >= OBJECT_CLASS_SIZE)
				 return return_value; // finished
		 	if (calling_template->object_class_member [class_index] [class_member_index] != -1
					&& core->group_member[calling_template->object_class_member [class_index] [class_member_index]].exists)
				{
					proc = &w.proc[core->group_member[calling_template->object_class_member [class_index] [class_member_index]].index];
					object_index = calling_template->object_class_object [class_index] [class_member_index];
					break;
				}
 	  class_member_index++;
		 };
		}


 switch(call_value)
 {
 	case CALL_MOVE_TO:
 		if (proc->object[object_index].type != OBJECT_TYPE_MOVE)
 		{
//  		print_method_error("move_to() method called on invalid object", 0, 0); // not sure whether this is useful
				return_value = 0;
				break;
 		}
 		{ // block used to confine variable scope
//			vmstate.instructions_left	-= 16; // ?
//			int turn_direction, reduced_power_level;
			if (call_initialised == 0)
			{
			 target_x = al_itofix(stack_parameters [0]) - w.proc[core->process_index].position.x;
			 target_y = al_itofix(stack_parameters [1]) - w.proc[core->process_index].position.y;
			 target_angle = get_angle(target_y, target_x);
 			vmstate.instructions_left	-= INSTRUCTION_COST_ATAN2; // get_angle calls atan2

			 call_initialised = 1;
			}

   calculate_move_and_turn(core, proc, object_index, target_angle);
			vmstate.instructions_left	-= 4;

			return_value = 1;
 		}
			break;

 	case CALL_TURN_TO_XY:
 		if (proc->object[object_index].type != OBJECT_TYPE_MOVE)
 		{
				return_value = 0;
				break;
 		}
 		{ // block used to confine variable scope
//			vmstate.instructions_left	-= 16; // ?
			if (call_initialised == 0)
			{
			 target_x = al_itofix(stack_parameters [0]) - w.proc[core->process_index].position.x;
			 target_y = al_itofix(stack_parameters [1]) - w.proc[core->process_index].position.y;
			 target_angle = get_angle(target_y, target_x);
 			vmstate.instructions_left	-= INSTRUCTION_COST_ATAN2;

			 call_initialised = 1;
			}

   calculate_turn(core, proc, object_index, target_angle);
			vmstate.instructions_left	-= 4;

			return_value = 1;
 		}
			break;

 	case CALL_TURN_TO_ANGLE:
 		if (proc->object[object_index].type != OBJECT_TYPE_MOVE)
 		{
				return_value = 0;
				break;
 		}
 		{
    calculate_turn(core, proc, object_index, short_angle_to_fixed(stack_parameters [0]));
			 vmstate.instructions_left	-= 1;
			 return_value = 1;
 		}
			break;

		case CALL_TURN_TO_TARGET:
  case CALL_APPROACH_TARGET:
     if (call_initialised == 0)
					{
      vmstate.instructions_left -= 4;
      target_visibility = verify_target_core(core, stack_parameters [0], &target_core);
      if (!target_visibility)// != 1)
					 {
 						return_value = 0;
 						call_finished = 1; // don't bother running the rest of the class members
						 break;
					 }
      if (stack_parameters [1] < 0
 					 || stack_parameters [1] >= GROUP_MAX_MEMBERS
					  || target_core->group_member[stack_parameters[1]].exists == 0)
					  {
// 						 return_value = 0;
//  						call_finished = 1; // don't bother running the rest of the class members
//						  break;
        target_proc = &w.proc[target_core->process_index];
					  }
					   else
         target_proc = &w.proc[target_core->group_member[stack_parameters[1]].index];
  			 target_angle = get_angle(target_proc->position.y - core->core_position.y, target_proc->position.x - core->core_position.x);
  			 call_initialised = 1;
					}
 		if (call_value == CALL_TURN_TO_TARGET)
    calculate_turn(core, proc, object_index, target_angle);
 		  else
// must be CALL_APPROACH_TARGET
      {
			    if (stack_parameters [2] < 0)
					   stack_parameters [2] = 0;
 			   stand_off_angle(core, proc, object_index, target_angle,
//																						 distance(target_proc->position.y - core->core_position.y, target_proc->position.x - core->core_position.x), // target_distance
																						 distance_oct_xyxy(core->core_position.x, core->core_position.y, target_proc->position.x, target_proc->position.y), // target_distance
																						 al_itofix(stack_parameters [2])); // stand_off_distance
      }
		 vmstate.instructions_left	-= 1;
		 return_value = 1;
			break;

		case CALL_APPROACH_XY:
 		if (proc->object[object_index].type != OBJECT_TYPE_MOVE)
 		{
				return_value = 0;
				break;
 		}
 		{ // block used to confine variable scope
//			vmstate.instructions_left	-= 16; // ?
			if (call_initialised == 0)
			{
			 target_x = al_itofix(stack_parameters [0]) - w.proc[core->process_index].position.x;
			 target_y = al_itofix(stack_parameters [1]) - w.proc[core->process_index].position.y;
			 target_angle = get_angle(target_y, target_x);
			 target_distance = distance_oct(target_y, target_x);//distance(target_y, target_x);
			 if (stack_parameters [2] < 0)
					stack_parameters [2] = 0;
			 stand_off_distance = al_itofix(stack_parameters [2]);
 			vmstate.instructions_left	-= (INSTRUCTION_COST_ATAN2 + INSTRUCTION_COST_HYPOT); // expensive!

			 call_initialised = 1;
			}
// This line may generate a warning about possible uninitialised use of stand_off_distance and target_distance, but I'm pretty sure that can't happen (because of call_initialised):
			stand_off_angle(core, proc, object_index, target_angle, target_distance, stand_off_distance);
//			fpr(" target_angle %i distance %i stand_off_distance %i", fixed_angle_to_int(target_angle), al_fixtoi(target_distance), al_fixtoi(stand_off_distance));

			vmstate.instructions_left	-= 4;

			return_value = 1;
 		}
			break;

		case CALL_REPOSITION:
 		if (proc->object[object_index].type != OBJECT_TYPE_MOVE)
 		{
				return_value = 0;
				break;
 		}
 		{ // block used to confine variable scope
//			vmstate.instructions_left	-= 16; // ?
			if (call_initialised == 0)
			{
			 target_x = al_itofix(stack_parameters [0]) - w.proc[core->process_index].position.x;
			 target_y = al_itofix(stack_parameters [1]) - w.proc[core->process_index].position.y;
			 target_angle = get_angle(target_y, target_x);
			 target_angle_offset = short_angle_to_fixed(stack_parameters [2]); // this is actually the angle the process should end up facing
			 target_distance = distance_oct(target_y, target_x);//distance(target_y, target_x);
#define REPOSITION_MODE_MOVE 0
#define REPOSITION_MODE_TURN 1
#define REPOSITION_MODE_RETRO 2
#define REPOSITION_POWER_REDUCTION_DISTANCE 400
    stand_off_distance = -1;
				if (target_distance < al_itofix(REPOSITION_POWER_REDUCTION_DISTANCE))
				{
					stand_off_distance = al_fixtoi(target_distance);
					if (target_distance < al_itofix(180))
					 reposition_mode = REPOSITION_MODE_TURN;
					  else
							{
								if (angle_difference(target_angle, core->group_angle) > int_angle_to_fixed(2000)
									&& angle_difference(core->group_angle, target_angle_offset) < int_angle_to_fixed(1500))
									reposition_mode = REPOSITION_MODE_RETRO;
								  else
  									reposition_mode = REPOSITION_MODE_MOVE;
							}
				}
				 else
 					reposition_mode = REPOSITION_MODE_MOVE;
/*
fpr("\n repos mode %i target %i,%i ta %i tao %i td %i ads %i %i",
				reposition_mode,
				al_fixtoi(core->core_position.x + target_x),
				al_fixtoi(core->core_position.y + target_y),
				fixed_angle_to_int(target_angle),
				fixed_angle_to_int(target_angle_offset),
				al_fixtoi(target_distance),
				fixed_angle_to_int(angle_difference(target_angle, core->group_angle)),
				fixed_angle_to_int(angle_difference(core->group_angle, target_angle_offset)));
*/
 			vmstate.instructions_left	-= (INSTRUCTION_COST_ATAN2 + INSTRUCTION_COST_HYPOT); // expensive!
			 call_initialised = 1;
			}

			switch(reposition_mode)
			{
			 case REPOSITION_MODE_MOVE:
     calculate_move_and_turn(core, proc, object_index, target_angle);
			 	break;
			 case REPOSITION_MODE_TURN:
     calculate_turn(core, proc, object_index, short_angle_to_fixed(stack_parameters [2]));
			  break;
			 case REPOSITION_MODE_RETRO:
     calculate_retro_move_and_turn(core, proc, object_index, target_angle);
			  break;
			}

			if (stand_off_distance != -1
				&& proc->object_instance[object_index].move_power > 0)
			{
				core->power_left += proc->object_instance[object_index].move_power; // put back any power allocated to the object

				proc->object_instance[object_index].move_power *= stand_off_distance;
				proc->object_instance[object_index].move_power /= (REPOSITION_POWER_REDUCTION_DISTANCE + 200);

 			core->power_left -= proc->object_instance[object_index].move_power;
			}

			vmstate.instructions_left	-= 4;

			return_value = 1;
 		}
			break;

 	case CALL_SET_POWER:
// 		if (otype[proc->object[object_index].type].accept_command [ACCEPT_COMMAND_SET_POWER] == 0)
//				return 0;
			switch(proc->object[object_index].type)
			{
			 case OBJECT_TYPE_MOVE:
			 	if (!call_initialised)
					{
// bounds-check stack_paramaters [0] for all later objects in this call:
						if (stack_parameters [0] < 0)
							stack_parameters [0] = 0;
						  else
   						if (stack_parameters [0] > MOVE_POWER_MAX)
										stack_parameters [0] = MOVE_POWER_MAX;
						call_initialised = 1;
					}
// need to put back the power already used by current setting:
					core->power_left += proc->object_instance[object_index].move_power;

  			if (core->power_left < stack_parameters [0])
					{
 			 	proc->object_instance[object_index].move_power = core->power_left;
 			 	core->power_left = 0;
// don't finish the call here, or modify stack_parameters [0], as it's possible that later objects will be using more power than stack_parameters [0] and the call will actually free up power.
 			 	break;
					}

			 	proc->object_instance[object_index].move_power = stack_parameters [0];

					core->power_left -= proc->object_instance[object_index].move_power;

//					core->power_use_predicted += (proc->object_instance[object_index].move_power + 9) / 10; // need to make sure 0-9 and 100 are treated correctly. See also the code that runs objects after execution
//			 	proc->object_instance[object_index].move_accel = al_itofix(proc->object_instance[object_index].move_power);
  			vmstate.instructions_left	-= 2; // ?
//					call_initialised = 1; // not relevant here
  			return_value = 1;
			 	break;
			}
			return_value = 0;
			break; // failed

		case CALL_ROTATE:
			switch(proc->object[object_index].type)
			{
			 case OBJECT_TYPE_PULSE:
			 case OBJECT_TYPE_PULSE_L:
			 case OBJECT_TYPE_PULSE_XL:
			 case OBJECT_TYPE_STREAM_DIR:
			 case OBJECT_TYPE_ULTRA_DIR:
			 	{
			 		int rotate_to_angle = stack_parameters [0];
			 		if (rotate_to_angle > ANGLE_4)
							rotate_to_angle = ANGLE_4;
			 		if (rotate_to_angle < -ANGLE_4)
							rotate_to_angle = -ANGLE_4;
      proc->object_instance[object_index].rotate_to_angle_offset = int_angle_to_fixed(rotate_to_angle);
   			vmstate.instructions_left	-= 2;
			 	}
					break;
			}
			break; // end CALL_ROTATE

		case CALL_NO_TARGET:
			switch(otype[proc->object[object_index].type].object_details.attack_type)
			{
				case ATTACK_TYPE_PULSE:
// note that an attack_scan or attack_scan_aim call can turn into this
//  - if code is changed here, may need to also change in attack_scan code (which also resets the angle if no target found, but only for the first object)
      proc->object_instance[object_index].rotate_to_angle_offset = proc->object[object_index].base_angle_offset; // this should be the angle the object points in on the design screen
   			vmstate.instructions_left	-= 2;
//   			fpr("\n no_target rtao %f bao %f", al_fixtof(proc->object_instance[object_index].rotate_to_angle_offset), al_fixtof(proc->object[object_index].base_angle_offset));

					break;
			}
			break; // end CALL_ROTATE

		case CALL_FIRE:

			if (!call_initialised)
			{
 		 if (stack_parameters [0] < 0)
				 stack_parameters [0] = 0;
			 if (stack_parameters [0] > 15)
				 stack_parameters [0] = 15;
				call_initialised = 1;
			}

			switch(otype[proc->object[object_index].type].object_details.attack_type)
			{


				case ATTACK_TYPE_BURST:
				case ATTACK_TYPE_PULSE:
// not spike though

	//		 	if (single_call
//					 && proc->object_instance[object_index].packet_last_fired > w.world_time - PACKET_RECYCLE_TIME)
//							break; // single_call will skip any members unable to fire
// stack_parameters [0] restricted to 0-15 above
// even with attack_fire_timestamp set it will only fire if it has recycled:
//			  if (proc->object_instance[object_index].attack_fire_timestamp - proc->object_instance[object_index].packet_last_fired > otype[proc->object[object_index].type].object_details.recycle_time)
			  if (proc->object_instance[object_index].attack_recycle_timestamp <= w.world_time)
					{
						switch(core->attack_mode)
						{
							default:
								if (!object_uses_power(core, otype[proc->object[object_index].type].object_details.power_cost))
									break;
        proc->object_instance[object_index].attack_fire_timestamp = w.world_time + stack_parameters [0]; // stack_parameters [0] limited to 0-15 above
//        number_of_attacks ++; - not needed as number_of_attacks irrelevant for this mode
        break;
							case ATTACK_MODE_FIRE_1:
							case ATTACK_MODE_FIRE_2:
							case ATTACK_MODE_FIRE_3:
							 if (number_of_attacks >= core->attack_mode)
									call_finished = 1;
									 else
										{
											if (!object_uses_power(core, otype[proc->object[object_index].type].object_details.power_cost))
												break;
			        proc->object_instance[object_index].attack_fire_timestamp = w.world_time + stack_parameters [0]; // stack_parameters [0] limited to 0-15 above
           number_of_attacks ++;
										}
							 break;
/*
							case ATTACK_MODE_ALL_POWER:
							 if (otype[proc->object[object_index].type].object_details.power_cost <= core->power_capacity - (core->power_used + core->power_use_predicted))
								{
         core->power_use_predicted += otype[proc->object[object_index].type].object_details.power_cost; // only add predicted power use if the object is not in recycle state
         proc->object_instance[object_index].attack_fire_timestamp = w.world_time + stack_parameters [0];
//        number_of_attacks ++; - not needed as number_of_attacks irrelevant for this mode
								}
								break;*/
						} // end attack_mode switch
					}
 			 vmstate.instructions_left	-= 2;
				 break;

/*			 case OBJECT_TYPE_BURST_DIR:
			 case OBJECT_TYPE_BURST:
// stack_parameters [0] restricted to 0-15 above
			  proc->object_instance[object_index].packet_fire_timestamp = w.world_time + stack_parameters [0];
			  if (proc->object_instance[object_index].packet_fire_timestamp - proc->object_instance[object_index].packet_last_fired > BURST_RECYCLE_TIME)
					{
			   core->power_use_predicted += POWER_COST_BURST; // only add predicted power use if the object is not in recycle state
			   if (call_value == CALL_FIRE_1)
							call_finished = 1; // fire_1 doesn't need to do anything for any remaining objects (unlike fire_1_at, which needs to rotate them)
					}
 			 vmstate.instructions_left	-= 2;
				 break;*/
/*				case OBJECT_TYPE_STREAM:
				case OBJECT_TYPE_STREAM_DIR:
					if (proc->object_instance[object_index].attack_last_fire_timestamp < w.world_time - STREAM_RECYCLE_TIME)

... do recycle properly here
				 {
  			  proc->object_instance[object_index].stream_fire = 1;
  				 core->power_use_predicted += POWER_COST_STREAM;
			    if (call_value == CALL_FIRE_1)
							 call_finished = 1; // fire_1 doesn't need to do anything for any remaining objects (unlike fire_1_at, which needs to rotate them)
				 }
  			vmstate.instructions_left	-= 2;
				 break;*/
/*				case OBJECT_TYPE_SPIKE: - spike uses different methods
// stack_parameters [0] restricted to 0-15 above
			  proc->object_instance[object_index].spike_fire_timestamp = w.world_time + stack_parameters [0];
			  core->power_use_predicted += POWER_COST_SPIKE;
 			 vmstate.instructions_left	-= 2;
				 break;*/
			}
			break;


		case CALL_AIM_AT: // does same as fire_at but doesn't actually fire
		case CALL_FIRE_AT: // can turn into CALL_AIM_AT in some attack modes
			switch(otype[proc->object[object_index].type].object_details.attack_type)
			{
				case ATTACK_TYPE_PULSE:
// fire_at has two parameters: process index of target, member index of target component
     { // block used to confine variable scope
     if (call_initialised == 0)
					{
      vmstate.instructions_left -= 4;
      target_visibility = verify_target_core(core, stack_parameters [0], &target_core);
      if (!target_visibility)// != 1)
					 {
 						return_value = 0;
 						call_finished = 1; // don't bother running the rest of the class members
						 break;
					 }
      if (stack_parameters [1] < 0
 					 || stack_parameters [1] >= GROUP_MAX_MEMBERS
					  || target_core->group_member[stack_parameters[1]].exists == 0)
					  {

// 						 return_value = 0;
//  						call_finished = 1; // don't bother running the rest of the class members
//						  break;
        target_proc = &w.proc[target_core->process_index]; // target core
					  }
					   else
         target_proc = &w.proc[target_core->group_member[stack_parameters[1]].index];
  			 call_initialised = 1;
					}

// after the verify_target_core call returned 1, we can assume that target_core is valid.
     proc->object_instance[object_index].rotate_to_angle_offset = lead_target(core, proc, object_index, target_core, target_proc, otype[proc->object[object_index].type].object_details.packet_speed);
// note that lead_target produces different results for each object, so it can't be used once per class call

					return_value = 1; // method returns 1 if target valid, whether or not anything fired

     if (call_value != CALL_AIM_AT
						&&	proc->object_instance[object_index].attack_recycle_timestamp <= w.world_time)
					{
						switch(core->attack_mode)
						{
							default:
								if (!object_uses_power(core, otype[proc->object[object_index].type].object_details.power_cost))
									break;
#define FIRING_ANGLE_DIFFERENCE AFX_ANGLE_16
  					 if (angle_difference(proc->object_instance[object_index].rotate_to_angle_offset, proc->object_instance[object_index].angle_offset) < FIRING_ANGLE_DIFFERENCE)
   					 proc->object_instance[object_index].attack_fire_timestamp = w.world_time + 8; // 8 gives the object a little time to rotate
//        number_of_attacks ++; - not needed as number_of_attacks irrelevant for this mode
        break;
							case ATTACK_MODE_FIRE_1:
							case ATTACK_MODE_FIRE_2:
							case ATTACK_MODE_FIRE_3:
							 if (number_of_attacks >= core->attack_mode)
								{
 				    call_value = CALL_AIM_AT;
								}
								 else
										{
											if (!object_uses_power(core, otype[proc->object[object_index].type].object_details.power_cost))
												break;
     					 if (angle_difference(proc->object_instance[object_index].rotate_to_angle_offset, proc->object_instance[object_index].angle_offset) < FIRING_ANGLE_DIFFERENCE)
     					  proc->object_instance[object_index].attack_fire_timestamp = w.world_time + 8; // 8 gives the object a little time to rotate
           number_of_attacks ++;
										}
							 break;
/*							case ATTACK_MODE_ALL_POWER:
							 if (otype[proc->object[object_index].type].object_details.power_cost <= core->power_capacity - (core->power_used + core->power_use_predicted))
								{
         core->power_use_predicted += otype[proc->object[object_index].type].object_details.power_cost; // only add predicted power use if the object is not in recycle state
  					  proc->object_instance[object_index].attack_fire_timestamp = w.world_time + 8; // 8 gives the object a little time to rotate
//        number_of_attacks ++; - not needed as number_of_attacks irrelevant for this mode
// doesn't set call_value to CALL_AIM_AT because there may be further objects which use less power and will be able to fire
								}
								break;*/
						} // end attack_mode switch

					}

     vmstate.instructions_left -= 8;
     }
					break; // end general packet firing

			}
//			return_value = 0;
			break;

		case CALL_ATTACK_SCAN: // this can change to CALL_ATTACK_SCAN_AIM if a new target found
		case CALL_ATTACK_SCAN_AIM:

// either of these can change to CALL_NO_TARGET if no target present
			switch(otype[proc->object[object_index].type].object_details.attack_type)
			{
				case ATTACK_TYPE_PULSE:
// attack_scan has two parameters: angle of scan from group angle, and target index
     { // block used to confine variable scope

     if (call_initialised == 0)
					{
      int scan_angle = stack_parameters [0] & ANGLE_MASK;
      int scan_distance = stack_parameters [1];
      int target_index = stack_parameters [2];
//      fpr("\n %i,%i,%i ", scan_angle, scan_distance, target_index);
      if (scan_distance < 0)
							scan_distance = 0;
      if (scan_distance > 2000)
							scan_distance = 2000;
// verify target index (must be valid for this call):
      if (target_index < 0
							|| target_index >= PROCESS_MEMORY_SIZE)
							return -1;
// now check whether there is current target, and if there is whether it's visible and in range:
      target_visibility = 0;
      if ((core->cycles_executed & 31) != 31) // should scan for a new target every now and then even if it already has one
						{
       if (core->process_memory[target_index] != -1)
        target_visibility = verify_target_core(core, target_index, &target_core); // will return 0 if no target in target_index at all
						}
//      fpr("\n as %i target %i vis %i ", core->index, core->process_memory[target_index], target_visibility);
#define SCAN_ATTACK_ANGLE_TOLERANCE 2300


      if (!target_visibility // != 1 / / not visible, does not exist etc. After this check, it should be safe for the following conditions to deref target_core
							|| distance_oct_xyxy(target_core->core_position.x, target_core->core_position.y, core->core_position.x, core->core_position.y) > al_itofix(2000) // think about distance
							|| angle_difference(core->group_angle + int_angle_to_fixed(scan_angle), get_angle(target_core->core_position.y - core->core_position.y, target_core->core_position.x - core->core_position.x)) > int_angle_to_fixed(SCAN_ATTACK_ANGLE_TOLERANCE))
					 {
// No current target. So let's scan for a new one:
       int scan_result = scan_for_auto_attack(core, scan_angle, scan_distance, target_index);
// disregard target found through scanning if it's out of range
								if (scan_result == 0
 							|| distance_oct_xyxy(w.core[core->process_memory[target_index]].core_position.x, w.core[core->process_memory[target_index]].core_position.y, core->core_position.x, core->core_position.y) > al_itofix(2000) // think about distance
	 						|| angle_difference(core->group_angle + int_angle_to_fixed(scan_angle), get_angle(w.core[core->process_memory[target_index]].core_position.y - core->core_position.y, w.core[core->process_memory[target_index]].core_position.x - core->core_position.x)) > int_angle_to_fixed(SCAN_ATTACK_ANGLE_TOLERANCE))
						 {
// no target found. set rest of class to return to base angle by changing call type to CALL_NO_TARGET:
 				   call_value = CALL_NO_TARGET;
// but no_target won't be called for this particular object, so do it here:
        proc->object_instance[object_index].rotate_to_angle_offset = proc->object[object_index].base_angle_offset; // this should be the angle the object points in on the design screen
// 				   return_value = 0;
/*        fpr("no target ");
        if (scan_result == 1)
         fpr("(core %i dist %i angle_diff %i (group_angle %i adjusted %i target %i) ",
													core->process_memory[target_index],
													al_fixtoi(distance_oct_xyxy(w.core[core->process_memory[target_index]].core_position.x, w.core[core->process_memory[target_index]].core_position.y, core->core_position.x, core->core_position.y)),
													fixed_angle_to_int(angle_difference(core->group_angle + int_angle_to_fixed(scan_angle), get_angle(w.core[core->process_memory[target_index]].core_position.y - core->core_position.y, w.core[core->process_memory[target_index]].core_position.x - core->core_position.x))),
													fixed_angle_to_int(core->group_angle),
													fixed_angle_to_int(core->group_angle + int_angle_to_fixed(scan_angle)),
													fixed_angle_to_int(get_angle(w.core[core->process_memory[target_index]].core_position.y - core->core_position.y, w.core[core->process_memory[target_index]].core_position.x - core->core_position.x)));
*/
 				   break;
						 }
// Target found!
       target_core = &w.core[core->process_memory[target_index]];
// For a newly found target, aim first until the objects have had a chance to rotate towards the target:
       call_value = CALL_ATTACK_SCAN_AIM;
//       fpr(" target found");
					 }
// At this point, target_core should have been set either by locating the existing target, or by finding a new one.
      vmstate.instructions_left -= 16; // right?
      target_proc = &w.proc[target_core->group_member[0].index]; // this method always targets the core.
  			 call_initialised = 1;
					}

// after the call initialisation code above, we can assume that target_core is valid.
     proc->object_instance[object_index].rotate_to_angle_offset = lead_target(core, proc, object_index, target_core, target_proc, otype[proc->object[object_index].type].object_details.packet_speed);
// note that lead_target produces different results for each object, so it can't be used once per class call

     return_value = 1; // has found a target

     vmstate.instructions_left -= 8;
     if (call_value != CALL_ATTACK_SCAN_AIM
						&&	proc->object_instance[object_index].attack_recycle_timestamp <= w.world_time)
					{
						switch(core->attack_mode)
						{
							default:
								if (!object_uses_power(core, otype[proc->object[object_index].type].object_details.power_cost))
									break;
  					 if (angle_difference(proc->object_instance[object_index].rotate_to_angle_offset, proc->object_instance[object_index].angle_offset) < FIRING_ANGLE_DIFFERENCE)
   					 proc->object_instance[object_index].attack_fire_timestamp = w.world_time + 8; // 8 gives the object a little time to rotate
//        number_of_attacks ++; - not needed as number_of_attacks irrelevant for this mode
        break;
							case ATTACK_MODE_FIRE_1:
							case ATTACK_MODE_FIRE_2:
							case ATTACK_MODE_FIRE_3:
							 if (number_of_attacks >= core->attack_mode)
								{
 				    call_value = CALL_ATTACK_SCAN_AIM;
								}
								 else
										{

											if (!object_uses_power(core, otype[proc->object[object_index].type].object_details.power_cost))
												break;
     					 if (angle_difference(proc->object_instance[object_index].rotate_to_angle_offset, proc->object_instance[object_index].angle_offset) < FIRING_ANGLE_DIFFERENCE)
      					 proc->object_instance[object_index].attack_fire_timestamp = w.world_time + 8; // 8 gives the object a little time to rotate
           return_value = 1;
           number_of_attacks ++;
										}
							 break;
/*							case ATTACK_MODE_ALL_POWER:
							 if (otype[proc->object[object_index].type].object_details.power_cost <= core->power_capacity - (core->power_used + core->power_use_predicted))
								{
         core->power_use_predicted += otype[proc->object[object_index].type].object_details.power_cost; // only add predicted power use if the object is not in recycle state
  					  proc->object_instance[object_index].attack_fire_timestamp = w.world_time + 8; // 8 gives the object a little time to rotate
//        number_of_attacks ++; - not needed as number_of_attacks irrelevant for this mode
// doesn't set call_value to CALL_AIM_AT because there may be further objects which use less power and will be able to fire
								}
								break;*/
						} // end attack_mode switch

					}


     }
					break; // end ATTACK_TYPE_PULSE firing

/*
			 case OBJECT_TYPE_STREAM_DIR:
// fire_at has two parameters: process index of target, member index of target component
     { // block used to confine variable scope
//     int target_visibility;
//     struct core_struct* target_core;
//     struct proc_struct* target_proc;
     if (call_initialised == 0)
					{
      vmstate.instructions_left -= 4;
      target_visibility = verify_target_core(core, stack_parameters [0], &target_core);
      if (target_visibility != 1)
					 {
 						return_value = 0;
 						call_finished = 1; // don't bother running the rest of the class members
						 break;
					 }
      if (stack_parameters [1] < 0
 					 || stack_parameters [1] >= GROUP_MAX_MEMBERS
					  || target_core->group_member[stack_parameters[1]].exists == 0)
					  {
 						 return_value = 0;
  						call_finished = 1; // don't bother running the rest of the class members
						  break;
					  }
      target_proc = &w.proc[target_core->group_member[stack_parameters[1]].index];
  			 call_initialised = 1;
					}

     al_fixed stream_target_angle = lead_target(core, proc, object_index, target_core, target_proc, otype[proc->object[object_index].type].object_details.packet_speed);

     proc->object_instance[object_index].rotate_to_angle_offset = stream_target_angle;


     if (proc->object_instance[object_index].stream_last_fired + otype[proc->object[object_index].type].object_details.recycle_time < w.world_time)
					{
      if (call_value == CALL_FIRE_AT)
					 {
 				  proc->object_instance[object_index].stream_fire = 1;
 				  core->power_use_predicted += otype[proc->object[object_index].type].object_details.power_cost;
					 }

      if (call_value == CALL_FIRE_1_AT)
					 {
 				  proc->object_instance[object_index].stream_fire = 1;
 				  core->power_use_predicted += otype[proc->object[object_index].type].object_details.power_cost;
 				  call_value = CALL_AIM_AT;
					 }

 					return_value = 1;
					}


     vmstate.instructions_left -= 8;
     }
					break; // end stream_dir firing
*/
			}
//			return_value = 0; // probably called on invalid object
			break;


		case CALL_INTERCEPT:
		case CALL_TRACK_TARGET: // the same except it calls calculate_turn instead of calculate_move_and_turn
		case CALL_APPROACH_TRACK: // similar, but has an extra target_distance parameter and calls stand_off_angle
 		if (proc->object[object_index].type != OBJECT_TYPE_MOVE)
 		{
				return_value = 0;
				break;
 		}
// intercept has three parameters: process index of target, member index of target, class that includes attacking object (first one found will be used)
     { // block used to confine variable scope
     	int attack_object_member = 0; // these default values will be used if no object found
     	int attack_object_object = 0;
//     int target_visibility;
//     struct core_struct* target_core;
//     struct proc_struct* target_proc;
     if (call_initialised == 0)
					{
      vmstate.instructions_left -= 4;
      target_visibility = verify_target_core(core, stack_parameters [0], &target_core);
      if (!target_visibility)// != 1)
					 {
 						return_value = 0;
 						core->power_left += proc->object_instance[object_index].move_power;
 					 proc->object_instance[object_index].move_power = 0;
// 						call_finished = 1; // don't bother running the rest of the class members
						 break;
					 }
      if (stack_parameters [1] < 0
 					 || stack_parameters [1] >= GROUP_MAX_MEMBERS
					  || target_core->group_member[stack_parameters[1]].exists == 0)
					  {
// 						 return_value = 0;
//  						core->power_left += proc->object_instance[object_index].move_power;
//  					 proc->object_instance[object_index].move_power = 0;
//  						call_finished = 1; // don't bother running the rest of the class members
						  //break;
        target_proc = &w.proc[target_core->process_index];
					  }
					   else
         target_proc = &w.proc[target_core->group_member[stack_parameters[1]].index];
  			 call_initialised = 1;
// Now, stack_parameters[2] should be a class index. We use the first object in the class:
      if (stack_parameters [2] >= 0
							&& stack_parameters [2] < OBJECT_CLASSES
							&& templ[core->player_index][core->template_index].object_class_active [stack_parameters [2]])
						{
							int i;
							for (i = 0; i < OBJECT_CLASS_SIZE; i ++)
							{
// check whether template has object, and that object (or at least member with object) exists
//  (the first object in the class is used, regardless of what type it is - if the user wants more control they can declare special classes just for the intercept call)
									if (templ[core->player_index][core->template_index].object_class_member [stack_parameters [2]] [i] != -1 // possibly this loop could break when the first -1 found here, as this array should not have holes in it
									 && core->group_member [templ[core->player_index][core->template_index].object_class_member [stack_parameters [2]] [i]].exists)
									{
										attack_object_member = templ[core->player_index][core->template_index].object_class_member [stack_parameters [2]] [i];
										attack_object_object = templ[core->player_index][core->template_index].object_class_object [stack_parameters [2]] [i];
// horrible
          break;
									}
							}
						}

// if not replaced in the loop above, intercept_speed has a default value of 4 and attack_object_member/object have default values of 0 (which should be valid)

// print a method call error if this happens?

// after the verify_target_core call returned 1, we can assume that target_core is valid.
// and after check_member_and_object_indices returned successfully, we can assume that stack_parameters[2] holds the firing member index and [3] the firing object index

      target_angle_offset = lead_target_with_fixed_object(core,
																																																										&w.proc[core->group_member[attack_object_member].index],
																																																										attack_object_object,
																																																										target_core,
																																																										target_proc,
																																																										otype[w.proc[core->group_member [attack_object_member].index].object[attack_object_object].type].object_details.packet_speed);

// since only one object is being targetted, we use the same target_angle calculation for all movement objects being called
      vmstate.instructions_left -= 8;
					}
// remember that object_index is the move object, not the attacking object
     switch(call_value)
     {
     	case CALL_INTERCEPT:
       calculate_move_and_turn(core, proc, object_index, (core->group_angle + target_angle_offset) & AFX_MASK);
       break;
      case CALL_TRACK_TARGET:
       calculate_turn(core, proc, object_index, (core->group_angle + target_angle_offset) & AFX_MASK);
       break;
      case CALL_APPROACH_TRACK:
      	{
			     if (stack_parameters [3] < 0)
					    stack_parameters [3] = 0;
			     stand_off_distance = al_itofix(stack_parameters [3]);

      	 stand_off_angle(core, proc, object_index, (core->group_angle + target_angle_offset) & AFX_MASK,
																								distance_oct_xyxy(core->core_position.x, core->core_position.y, target_proc->position.x, target_proc->position.y), // target_distance - it's okay to use non-target_leading values for this
//																								distance(core->core_position.y - target_proc->position.y, core->core_position.x - target_proc->position.x), // target_distance - it's okay to use non-target_leading values for this
																								stand_off_distance); // stand_off_distance
      	}
							break;
     }
     vmstate.instructions_left -= 4;

			  return_value = 1;
     }
//			return_value = 0;
			break; // end CALL_INTERCEPT


/*
SPIKES:
4 phases:

1 square is launched from object
2 square rotates to intended angle, ?and comes to rest
3 spike accelerates and elongates
4 spike is at full speed

*/
			case CALL_FIRE_SPIKE:
				if (proc->object[object_index].type != OBJECT_TYPE_SPIKE
				 || proc->object_instance[object_index].attack_recycle_timestamp > w.world_time)
					break;

				switch(core->attack_mode)
				{
					case ATTACK_MODE_FIRE_1:
					case ATTACK_MODE_FIRE_2:
					case ATTACK_MODE_FIRE_3:
					 if (number_of_attacks >= core->attack_mode)
			    call_finished = 1;
					 break;
/*					case ATTACK_MODE_ALL_POWER:
					 if (otype[proc->object[object_index].type].object_details.power_cost > core->power_capacity - (core->power_used + core->power_use_predicted))
							call_finished = 1; // unlike for other object types, we assume that spikes all use the same amount of power
						break;*/
				} // end attack_mode switch

				if (call_finished)
					break;

				if (!object_uses_power(core, otype[proc->object[object_index].type].object_details.power_cost))
					break;

		  proc->object_instance[object_index].attack_fire_timestamp = w.world_time;
				if (stack_parameters [0] < -2048)
  			stack_parameters [0] = -2048;
		 	if (stack_parameters [0] > 2048)
			 	stack_parameters [0] = 2048;
		  proc->object_instance[object_index].spike_angle_offset = stack_parameters [0];
			 vmstate.instructions_left	-= 2;
    number_of_attacks ++;
			 break;

			case CALL_FIRE_SPIKE_AT:
    {
 				if (proc->object[object_index].type != OBJECT_TYPE_SPIKE
	 			 || proc->object_instance[object_index].attack_recycle_timestamp > w.world_time)
		 			break;

				 switch(core->attack_mode)
				 {
						case ATTACK_MODE_FIRE_1:
						case ATTACK_MODE_FIRE_2:
						case ATTACK_MODE_FIRE_3:
					  if (number_of_attacks >= core->attack_mode)
			     call_finished = 1;
					  break;
/*					 case ATTACK_MODE_ALL_POWER:
					  if (otype[proc->object[object_index].type].object_details.power_cost > core->power_capacity - (core->power_used + core->power_use_predicted))
					 		call_finished = 1; // unlike for other object types, we assume that spikes all use the same amount of power
					 	break;*/
				 } // end attack_mode switch

				 if (call_finished)
					 break;

     if (call_initialised == 0)
					{
      vmstate.instructions_left -= 4;
      target_visibility = verify_target_core(core, stack_parameters [0], &target_core);
      if (!target_visibility)// != 1)
					 {
 						return_value = 0;
 						call_finished = 1; // don't bother running the rest of the class members
						 break;
					 }
      if (stack_parameters [1] < 0
 					 || stack_parameters [1] >= GROUP_MAX_MEMBERS
					  || target_core->group_member[stack_parameters[1]].exists == 0)
					  {
// 						 return_value = 0;
//  						call_finished = 1; // don't bother running the rest of the class members
//						  break;
        target_proc = &w.proc[target_core->process_index];
					  }
					   else
         target_proc = &w.proc[target_core->group_member[stack_parameters[1]].index];
  			 call_initialised = 1;
					}
#define SPIKE_SIDE_TRAVEL 300
#define SPIKE_SIDE_SPEED_MULTIPLIER 32

     int spike_target_angle = fixed_angle_to_int(get_spike_target_angle(core, proc, object_index, target_core, target_proc));

				if (!object_uses_power(core, otype[proc->object[object_index].type].object_details.power_cost))
					break;

		  proc->object_instance[object_index].attack_fire_timestamp = w.world_time;
				if (spike_target_angle < -2048)
  			spike_target_angle = -2048;
		 	if (spike_target_angle > 2048)
			 	spike_target_angle = 2048;
		  proc->object_instance[object_index].spike_angle_offset = spike_target_angle;
//		  core->power_use_predicted += POWER_COST_SPIKE;
			 vmstate.instructions_left	-= 2;
			 number_of_attacks ++;
    }
			 break;

			case CALL_FIRE_SPIKE_XY:
    {
				 if (proc->object[object_index].type != OBJECT_TYPE_SPIKE
 				 || proc->object_instance[object_index].attack_recycle_timestamp > w.world_time)
					 break;

				 switch(core->attack_mode)
				 {
						case ATTACK_MODE_FIRE_1:
						case ATTACK_MODE_FIRE_2:
						case ATTACK_MODE_FIRE_3:
					  if (number_of_attacks >= core->attack_mode)
			     call_finished = 1;
					  break;
/*					 case ATTACK_MODE_ALL_POWER:
					  if (otype[proc->object[object_index].type].object_details.power_cost > core->power_capacity - (core->power_used + core->power_use_predicted))
					 		call_finished = 1; // unlike for other object types, we assume that spikes all use the same amount of power
					 	break;*/
				 } // end attack_mode switch

				 if (call_finished)
					 break;

    	al_fixed base_angle = proc->angle + proc->nshape_ptr->object_angle_fixed[object_index];
     base_angle &= AFX_MASK;

     al_fixed source_x = proc->position.x
                         + fixed_xpart(base_angle,
									    																										proc->nshape_ptr->object_dist_fixed[object_index] + al_itofix(SPIKE_SIDE_TRAVEL))
																									+ (proc->speed.x * SPIKE_SIDE_SPEED_MULTIPLIER);
// should really add speed here
     al_fixed source_y = proc->position.y
                         + fixed_ypart(base_angle,
									    																										proc->nshape_ptr->object_dist_fixed[object_index] + al_itofix(SPIKE_SIDE_TRAVEL))
																									+ (proc->speed.y * SPIKE_SIDE_SPEED_MULTIPLIER);


     int spike_target_angle = fixed_angle_to_int(get_angle(al_itofix(stack_parameters [1]) - source_y, al_itofix(stack_parameters [0]) - source_x));

     spike_target_angle = angle_difference_signed_int(fixed_angle_to_int(base_angle), spike_target_angle);

 			if (!object_uses_power(core, otype[proc->object[object_index].type].object_details.power_cost))
					break;
		  proc->object_instance[object_index].attack_fire_timestamp = w.world_time;
				if (spike_target_angle < -2048)
  			spike_target_angle = -2048;
		 	if (spike_target_angle > 2048)
			 	spike_target_angle = 2048;
		  proc->object_instance[object_index].spike_angle_offset = spike_target_angle;
//		  core->power_use_predicted += POWER_COST_SPIKE;
			 vmstate.instructions_left	-= 2;
			 number_of_attacks ++;
    }
			 break;


			case CALL_GATHER_DATA:
				{
		 	 if (proc->object[object_index].type != OBJECT_TYPE_HARVEST
					 || core->data_storage_capacity == 0
					 || core->data_stored == core->data_storage_capacity
					 || proc->object_instance[object_index].last_gather_or_give > w.world_time - HARVEST_RECYCLE_TIME) // currently 64 tick recycle time
					 break;
     vmstate.instructions_left -= 4;
					if (vmstate.nearby_well_index == -2) // not yet calculated
				  find_nearby_well(core); // updates vmstate.nearby_well_index
			  if (vmstate.nearby_well_index == -1)
					{
				  call_finished = 1;
				  return_value = -1;
				  break;
					}
// at this point vmstate.nearby_well_index should be valid
     int data_harvested = HARVEST_RATE;//16;
					if (core->data_stored + data_harvested > core->data_storage_capacity)
						data_harvested = core->data_storage_capacity - core->data_stored;
     if (data_harvested > w.data_well [vmstate.nearby_well_index].data)
						data_harvested = w.data_well [vmstate.nearby_well_index].data;
					if (data_harvested > 0)
					{
					 if (!object_uses_power(core, POWER_COST_GATHER_BASE))
							break;
					 core->data_stored += data_harvested;
					 w.data_well[vmstate.nearby_well_index].data -= data_harvested;
     	w.data_well[vmstate.nearby_well_index].last_harvested = w.world_time;

					 proc->object_instance[object_index].ongoing_power_cost = POWER_COST_GATHER_BASE;// + (POWER_COST_GATHER_1_DATA * data_harvested);
					 proc->object_instance[object_index].ongoing_power_cost_finish_time = w.world_time + HARVEST_RECYCLE_TIME;

					 proc->object_instance[object_index].second_last_gather_or_give = proc->object_instance[object_index].last_gather_or_give; // used for animation
					 proc->object_instance[object_index].last_gather_or_give = w.world_time;
					 proc->object_instance[object_index].gather_or_give = 0; // 0 = gather, 1 = give
//					 proc->object_instance[object_index].gather_target_index = vmstate.nearby_well_index;

      struct cloud_struct* cl = new_cloud(CLOUD_HARVEST_LINE, 32, w.data_well [vmstate.nearby_well_index].position.x, w.data_well [vmstate.nearby_well_index].position.y);

      if (cl != NULL)
      {
       cl->colour = proc->player_index;

//       cl->position2.x = x;
//       cl->position2.y = y;
//      cl->data [0] = ;
       cl->data [0] = proc->index;
       cl->data [1] = object_index;
       cl->associated_proc_timestamp = proc->created_timestamp;
// work out the display bounding box:
       cl->display_size_x1 = -80 - al_fixtof(abs(w.data_well [vmstate.nearby_well_index].position.x - proc->position.x));
       cl->display_size_y1 = -80 - al_fixtof(abs(w.data_well [vmstate.nearby_well_index].position.y - proc->position.y));
       cl->display_size_x2 = 80 + al_fixtof(abs(w.data_well [vmstate.nearby_well_index].position.x - proc->position.x));
       cl->display_size_y2 = 80 + al_fixtof(abs(w.data_well [vmstate.nearby_well_index].position.y - proc->position.y));
      }

						 play_game_sound(SAMPLE_ALLOC, TONE_1G, 80, 0, proc->position.x, proc->position.y);

					}
					return_value += data_harvested; // += will sum total harvest over a class
     vmstate.instructions_left -= 4;
				}
				break;
			case CALL_GIVE_DATA:
				{
		 	 if (proc->object[object_index].type != OBJECT_TYPE_HARVEST
					 || core->data_stored == 0
					 || proc->object_instance[object_index].last_gather_or_give > w.world_time - HARVEST_RECYCLE_TIME)
					 break;
//     static int target_visibility;
//     static struct core_struct* target_core; // is static because it may need to retain its value across a class of harvest objects
     if (call_initialised == 0)
					{
      vmstate.instructions_left -= 4;
      target_visibility = verify_target_core(core, stack_parameters [0], &target_core);
      if (!target_visibility // != 1
							|| distance_oct_xyxy(target_core->core_position.x, target_core->core_position.y,
																												core->core_position.x, core->core_position.y) > core->scan_range_fixed)
					 {
 						return_value = -1;
 						call_finished = 1; // don't bother running the rest of the class members
						 break;
					 }
  			 call_initialised = 1;
					}
					int data_transferred = stack_parameters [1];
					if (data_transferred > 32)
						data_transferred = 32;
					if (data_transferred > core->data_stored)
					 data_transferred = core->data_stored;
					if (target_core->data_stored + data_transferred > target_core->data_storage_capacity)
					 data_transferred = target_core->data_storage_capacity - target_core->data_stored;
					if (data_transferred <= 0)
					{
						return_value = 0;
						call_finished = 1;
						break;
					}
				 if (!object_uses_power(core, POWER_COST_GIVE_BASE))
						break;

					core->data_stored -= data_transferred;
					target_core->data_stored += data_transferred;
//				 core->power_used += POWER_COST_GIVE_BASE + (POWER_COST_GIVE_1_DATA * data_transferred);

				 proc->object_instance[object_index].second_last_gather_or_give = proc->object_instance[object_index].last_gather_or_give; // used for animation
				 proc->object_instance[object_index].last_gather_or_give = w.world_time;
				 proc->object_instance[object_index].gather_or_give = 1; // 0 = gather, 1 = give
//				 proc->object_instance[object_index].gather_target_index = target_core->index;
					return_value += data_transferred; // += will sum total given over a class

      struct cloud_struct* cl = new_cloud(CLOUD_GIVE_LINE, 32, target_core->core_position.x, target_core->core_position.y);

      if (cl != NULL)
      {
       cl->colour = proc->player_index;
//       cl->position2.x = x;
//       cl->position2.y = y;
//      cl->data [0] = ;
       cl->data [0] = proc->index;
       cl->data [1] = object_index;
       cl->associated_proc_timestamp = proc->created_timestamp;
// work out the display bounding box:
       cl->display_size_x1 = -80 - al_fixtof(abs(target_core->core_position.x - proc->position.x));
       cl->display_size_y1 = -80 - al_fixtof(abs(target_core->core_position.y - proc->position.y));
       cl->display_size_x2 = 80 + al_fixtof(abs(target_core->core_position.x - proc->position.x));
       cl->display_size_y2 = 80 + al_fixtof(abs(target_core->core_position.y - proc->position.y));
      }

					 play_game_sound(SAMPLE_ALLOC, TONE_2C, 70, 0, proc->position.x, proc->position.y);

      vmstate.instructions_left -= 4;

				}
				break;

			case CALL_TAKE_DATA:
				{

		 	 if (proc->object[object_index].type != OBJECT_TYPE_HARVEST
					 || core->data_stored == core->data_storage_capacity
					 || proc->object_instance[object_index].last_gather_or_give > w.world_time - HARVEST_RECYCLE_TIME)
					 break;

//     static int target_visibility;
//     static struct core_struct* target_core; // is static because it may need to retain its value across a class of harvest objects
     if (call_initialised == 0)
					{
      vmstate.instructions_left -= 4;
      target_visibility = verify_target_core(core, stack_parameters [0], &target_core);
      if (!target_visibility // != 1
							|| distance_oct_xyxy(target_core->core_position.x, target_core->core_position.y,
																												core->core_position.x, core->core_position.y) > core->scan_range_fixed)
					 {
 						return_value = -1;
 						call_finished = 1; // don't bother running the rest of the class members
						 break;
					 }
  			 call_initialised = 1;
  			 if (target_core->data_stored == 0)
							return 0;
						if (target_core->player_index != core->player_index)
							return 0; // can't do this, sorry
					}
					int data_transferred = stack_parameters [1];
					if (data_transferred > 32)
						data_transferred = 32;
					if (data_transferred > target_core->data_stored)
					 data_transferred = target_core->data_stored;
					if (core->data_stored + data_transferred > core->data_storage_capacity)
					 data_transferred = core->data_storage_capacity - core->data_stored;
					if (data_transferred <= 0)
					{
						return_value = 0;
						call_finished = 1;
						break;
					}
				 if (!object_uses_power(core, POWER_COST_GIVE_BASE))
						break;

					core->data_stored += data_transferred;
					target_core->data_stored -= data_transferred;
//				 core->power_used += POWER_COST_GIVE_BASE + (POWER_COST_GIVE_1_DATA * data_transferred);

				 proc->object_instance[object_index].second_last_gather_or_give = proc->object_instance[object_index].last_gather_or_give; // used for animation
				 proc->object_instance[object_index].last_gather_or_give = w.world_time;
				 proc->object_instance[object_index].gather_or_give = 0; // 0 = gather, 1 = give (gather also used for take)
//				 proc->object_instance[object_index].gather_target_index = target_core->index;
					return_value += data_transferred; // += will sum total given over a class

      struct cloud_struct* cl = new_cloud(CLOUD_TAKE_LINE, 32, target_core->core_position.x, target_core->core_position.y);

      if (cl != NULL)
      {
       cl->colour = proc->player_index;
//       cl->position2.x = x;
//       cl->position2.y = y;
//      cl->data [0] = ;
       cl->data [0] = proc->index;
       cl->data [1] = object_index;
       cl->associated_proc_timestamp = proc->created_timestamp;
// work out the display bounding box:
       cl->display_size_x1 = -80 - al_fixtof(abs(target_core->core_position.x - proc->position.x));
       cl->display_size_y1 = -80 - al_fixtof(abs(target_core->core_position.y - proc->position.y));
       cl->display_size_x2 = 80 + al_fixtof(abs(target_core->core_position.x - proc->position.x));
       cl->display_size_y2 = 80 + al_fixtof(abs(target_core->core_position.y - proc->position.y));
      }

					 play_game_sound(SAMPLE_ALLOC, TONE_2D, 70, 0, proc->position.x, proc->position.y);

      vmstate.instructions_left -= 4;

				}
				break;

			case CALL_ALLOCATE_DATA:
				{
					if (core->data_stored == 0)
//					 || core->stress_level >= STRESS_MODERATE)
					{
					 call_finished = 1;
					 break;
				 }
		 	 if (proc->object[object_index].type != OBJECT_TYPE_ALLOCATE)
						break;
     vmstate.instructions_left -= 4;
					if (proc->object_instance[object_index].last_allocate == w.world_time)
						break;
					int data_allocated = stack_parameters [0];
					if (data_allocated > core->data_stored)
						data_allocated = core->data_stored;
					if (data_allocated > ALLOCATE_RATE)
						data_allocated = ALLOCATE_RATE;
					if (data_allocated <= 0)
						break;
					if (!object_uses_power(core, POWER_COST_ALLOCATE_1_DATA * data_allocated))
						break;
					if (proc->object_instance[object_index].last_allocate != w.world_time - 16)
						proc->object_instance[object_index].first_unbroken_allocate = w.world_time;
					proc->object_instance[object_index].last_allocate = w.world_time;
					core->data_stored -= data_allocated;
					w.player[core->player_index].data += data_allocated;
//				 core->power_used += POWER_COST_ALLOCATE_1_DATA * data_allocated; // no need for a base cost
					return_value += data_allocated; // += will sum total given over a class
				}
				break;

			case CALL_SET_STABILITY:
	 	 if (proc->object[object_index].type != OBJECT_TYPE_STABILITY)
					break;
				if (stack_parameters [0])
				{
					if (proc->interface_stability == 0)
					{
						proc->interface_stability = 1;
						if (core->interface_active)
						{
//						 proc->interface_stability_on_time = w.world_time;
						 proc->interface_hit_time = w.world_time;
						 core->power_left -= INTERFACE_STABILITY_POWER_USE; // currently stability will drain 30 power if possible, but won't turn off if insufficient power available.
						 if (core->power_left < 0)
								core->power_left = 0;
						}
					}
				}
				  else
						{
					  if (proc->interface_stability
								&& core->interface_active)
					  {
//						  proc->interface_stability_off_time = w.world_time;
 						 proc->interface_hit_time = w.world_time;
					  }
					  proc->interface_stability = 0;
						}
				break;
/*

Not currently supported - not entirely sure why but it probably doesn't matter too much

			case CALL_SET_INTERFACE:
	 	 if (proc->object[object_index].type != OBJECT_TYPE_INTERFACE)
					break;
				if (stack_parameters [0])
				{
					if (proc->interface_on_process_set_on == 0)
					{
						proc->interface_on_process_set_on = 1;
						if (core->interface_active)
						{
						 proc->interface_raised_time = w.world_time;
						 play_game_sound(SAMPLE_INT_UP, TONE_1G, 160, 1, proc->position.x, proc->position.y);
//  					proc->object_instance[object_index].interface_object_active = 1;
						}
					}
				}
				  else
						{
					  if (proc->interface_on_process_set_on
//								&& proc->object_instance[object_index].interface_object_active
								&& core->interface_active)
					  {
						  proc->interface_lowered_time = w.world_time;
					  }
					  proc->interface_on_process_set_on = 0;
//					  proc->object_instance[object_index].interface_object_active = 0;
						}
					break;
*/


 }

  if (class_index == -1
			|| call_finished == 1)
			return return_value;

	} // end while(TRUE) loop

 return return_value; // did nothing

}


static int object_uses_power(struct core_struct* core, int power_cost)
{
//if (core->player_index == 1 && core->template_index == 7)
	//fpr("\n core %i using power: left %i cost %i excess %i (%i)", core->index, core->power_left, power_cost, core->power_use_excess, core->power_use_excess + power_cost);
					   if (core->power_left < power_cost)
					   {
					    core->power_use_excess += power_cost;
					    return 0;
					   }

        core->power_left -= power_cost;

        return 1;

}



static void calculate_move_and_turn(struct core_struct* core, struct proc_struct* proc, int object_index, al_fixed target_angle)
{

#define SPIN_CHANGE_THRESHOLD 50
// SPIN_CHANGE_THRESHOLD deals with move objects on, or very close to, the process' main axis


			al_fixed angle_diff;
			int turn_direction, reduced_power_level;

		 turn_direction = delta_turn_towards_angle(fixed_angle_to_int(core->group_angle) & ANGLE_MASK, fixed_angle_to_int(target_angle) & ANGLE_MASK, 1);

		 angle_diff = angle_difference(core->group_angle, target_angle);


//   int divisor = 10;// + (angle_diff / 2);

				if ((proc->object_instance[object_index].move_accel_angle_offset > AFX_ANGLE_4
			 &&	proc->object_instance[object_index].move_accel_angle_offset < (AFX_ANGLE_1-AFX_ANGLE_4))
				 && fixed_angle_to_int(angle_diff) < 800)
			{
// backwards-facing objects should only be used to turn, not move
 					core->power_left += proc->object_instance[object_index].move_power; // put back any power allocated to the object
					 proc->object_instance[object_index].move_power = 0;
					 return;
			}


			reduced_power_level = 10 - fixed_angle_to_int(angle_diff) / 100;// / divisor;


			if (reduced_power_level < 0)
				reduced_power_level = 0;

// if core has spun past target angle, correct:
   if (turn_direction == 1
				&& core->group_spin > 0
				&& angle_difference_signed(core->group_angle + (core->group_spin * 32), target_angle) < 0
			 && proc->object_instance[object_index].move_spin_change < 0) // SPIN_CHANGE_THRESHOLD?
			{

				reduced_power_level = MOVE_POWER_MAX;
			}

   if (turn_direction == -1
				&& core->group_spin < 0
				&& angle_difference_signed(core->group_angle + (core->group_spin * 32), target_angle) > 0
//				&& fixed_angle_to_int(angle_difference(core->group_angle + (core->group_spin * 16), target_angle)) > 0
			 && proc->object_instance[object_index].move_spin_change > 0) // SPIN_CHANGE_THRESHOLD?
			{
				reduced_power_level = MOVE_POWER_MAX;
			}

			core->power_left += proc->object_instance[object_index].move_power; // put back any power allocated to the object


			if ((turn_direction == 1
				 && proc->object_instance[object_index].move_spin_change > -SPIN_CHANGE_THRESHOLD)
				|| (turn_direction == -1
				 && proc->object_instance[object_index].move_spin_change < SPIN_CHANGE_THRESHOLD)
				|| turn_direction == 0)
				{
					 proc->object_instance[object_index].move_power = MOVE_POWER_MAX;
				}
					  else
							{
					   proc->object_instance[object_index].move_power = reduced_power_level;
							}

			if (core->power_left < proc->object_instance[object_index].move_power)
			{
				proc->object_instance[object_index].move_power = core->power_left;
				core->power_left = 0;
				return;
			}

			core->power_left -= proc->object_instance[object_index].move_power; // now use the new amount of power

//			core->power_use_predicted += (proc->object_instance[object_index].move_power + 9) / 10; // need to make sure 0-9 and 100 are treated correctly. See also the code that runs objects after execution

}



static void calculate_turn(struct core_struct* core, struct proc_struct* proc, int object_index, al_fixed target_angle)
{

			al_fixed angle_diff;
			int turn_direction, turn_power;

		 angle_diff = angle_difference(core->group_angle, target_angle);

		 if (angle_diff < al_itofix(4))
		 {
				core->power_left += proc->object_instance[object_index].move_power;
    proc->object_instance[object_index].move_power = 0;
				return;
		 }

		 turn_direction = delta_turn_towards_angle(fixed_angle_to_int(core->group_angle) & ANGLE_MASK, fixed_angle_to_int(target_angle) & ANGLE_MASK, 1);

//   int divisor = 10;// + (angle_diff / 2);

			turn_power = fixed_angle_to_int(angle_diff) / 100;

//		 fpr("\n calc_turn difference %f turn_power %i", al_fixtof(angle_diff), turn_power);


			if (turn_power > MOVE_POWER_MAX)
				turn_power = MOVE_POWER_MAX;

/*
// if core has spun past target angle, correct:
   if (turn_direction == 1
				&& core->group_spin > 0
				&& angle_difference_signed(core->group_angle + (core->group_spin * 32), target_angle) < 0
			 && proc->object_instance[object_index].move_spin_change < 0) // SPIN_CHANGE_THRESHOLD?
			{
				reduced_power_level = 100;
			}

   if (turn_direction == -1
				&& core->group_spin < 0
				&& angle_difference_signed(core->group_angle + (core->group_spin * 32), target_angle) > 0
//				&& fixed_angle_to_int(angle_difference(core->group_angle + (core->group_spin * 16), target_angle)) > 0
			 && proc->object_instance[object_index].move_spin_change > 0) // SPIN_CHANGE_THRESHOLD?
			{
				reduced_power_level = 100;
			}*/

			core->power_left += proc->object_instance[object_index].move_power;

			if ((turn_direction == 1
				 && proc->object_instance[object_index].move_spin_change > -SPIN_CHANGE_THRESHOLD)
				|| (turn_direction == -1
				 && proc->object_instance[object_index].move_spin_change < SPIN_CHANGE_THRESHOLD)
				|| turn_direction == 0)
					 proc->object_instance[object_index].move_power = turn_power;//10;
					  else
					   proc->object_instance[object_index].move_power = 0;//reduced_power_level;


			if (core->power_left < proc->object_instance[object_index].move_power)
			{
				proc->object_instance[object_index].move_power = core->power_left;
				core->power_left = 0;
				return;
			}

			core->power_left -= proc->object_instance[object_index].move_power;


//			core->power_use_predicted += (proc->object_instance[object_index].move_power + 9) / 10; // need to make sure 0-9 and 100 are treated correctly. See also the code that runs objects after execution


}

// this function does the following:
//  1. if core is further than stand_off_distance, calculate_move_and_turn
//  2. if core is closer than stand_off_distance but facing away from target, turn core towards target_angle
//  3. if core is closer than stand_off_distance and facing towards target, move backwards
// only works properly if target has retro move objects
static void stand_off_angle(struct core_struct* core, struct proc_struct* proc, int object_index, al_fixed target_angle, al_fixed target_distance, al_fixed stand_off_distance)
{

	if (target_distance > stand_off_distance)
	{
		calculate_move_and_turn(core, proc, object_index, target_angle);
		return;
	}

	if (angle_difference(core->group_angle, target_angle) > al_itofix(4))
	{
		calculate_turn(core, proc, object_index, target_angle);
		return;
	}

	if (target_distance > stand_off_distance - al_itofix(100))
	{
		core->power_left += proc->object_instance[object_index].move_power;
	 proc->object_instance[object_index].move_power = 0;
		return;
	}

			core->power_left += proc->object_instance[object_index].move_power;

// core is closer than stand_off_distance and pointing towards target. so try to move away
				if (proc->object_instance[object_index].move_accel_angle_offset > AFX_ANGLE_4
 			 &&	proc->object_instance[object_index].move_accel_angle_offset < (AFX_ANGLE_1-AFX_ANGLE_4))
 			 {
					 proc->object_instance[object_index].move_power = MOVE_POWER_MAX;
//   			core->power_use_predicted += 10;//(proc->object_instance[object_index].move_power + 9) / 10; // need to make sure 0-9 and 100 are treated correctly. See also the code that runs objects after execution
 			 }
					  else
					   proc->object_instance[object_index].move_power = 0;

			if (core->power_left < proc->object_instance[object_index].move_power)
			{
				proc->object_instance[object_index].move_power = core->power_left;
				core->power_left = 0;
				return;
			}

			core->power_left -= proc->object_instance[object_index].move_power;

}




// this is like calculate move_and_turn, but uses retro objects
static void calculate_retro_move_and_turn(struct core_struct* core, struct proc_struct* proc, int object_index, al_fixed target_angle)
{

#define SPIN_CHANGE_THRESHOLD 50
// SPIN_CHANGE_THRESHOLD deals with move objects on, or very close to, the process' main axis

			al_fixed angle_diff;
			int turn_direction, reduced_power_level;

			al_fixed adjusted_group_angle = (core->group_angle + AFX_ANGLE_2) & AFX_MASK;

		 turn_direction = delta_turn_towards_angle(fixed_angle_to_int(adjusted_group_angle) & ANGLE_MASK, fixed_angle_to_int(target_angle) & ANGLE_MASK, 1);

		 angle_diff = angle_difference(adjusted_group_angle, target_angle);


//   int divisor = 10;// + (angle_diff / 2);

				if ((proc->object_instance[object_index].move_accel_angle_offset < AFX_ANGLE_4
			 ||	proc->object_instance[object_index].move_accel_angle_offset > (AFX_ANGLE_1-AFX_ANGLE_4))
				 && fixed_angle_to_int(angle_diff) < 800)
			{
// forwards-facing objects should only be used to turn, not move
 					core->power_left += proc->object_instance[object_index].move_power; // put back any power allocated to the object
					 proc->object_instance[object_index].move_power = 0;
					 return;
			}


			reduced_power_level = 10 - fixed_angle_to_int(angle_diff) / 100;// / divisor;

			if (reduced_power_level < 0)
				reduced_power_level = 0;

// if core has spun past target angle, correct:
   if (turn_direction == 1
				&& core->group_spin > 0
				&& angle_difference_signed(adjusted_group_angle + (core->group_spin * 32), target_angle) < 0
			 && proc->object_instance[object_index].move_spin_change < 0) // SPIN_CHANGE_THRESHOLD?
			{

				reduced_power_level = MOVE_POWER_MAX;
			}

   if (turn_direction == -1
				&& core->group_spin < 0
				&& angle_difference_signed(adjusted_group_angle + (core->group_spin * 32), target_angle) > 0
//				&& fixed_angle_to_int(angle_difference(core->group_angle + (core->group_spin * 16), target_angle)) > 0
			 && proc->object_instance[object_index].move_spin_change > 0) // SPIN_CHANGE_THRESHOLD?
			{
				reduced_power_level = MOVE_POWER_MAX;
			}

			core->power_left += proc->object_instance[object_index].move_power; // put back any power allocated to the object


			if ((turn_direction == 1
				 && proc->object_instance[object_index].move_spin_change > -SPIN_CHANGE_THRESHOLD)
				|| (turn_direction == -1
				 && proc->object_instance[object_index].move_spin_change < SPIN_CHANGE_THRESHOLD)
				|| turn_direction == 0)
				{
					 proc->object_instance[object_index].move_power = MOVE_POWER_MAX;
				}
					  else
							{
					   proc->object_instance[object_index].move_power = reduced_power_level;
							}

			if (core->power_left < proc->object_instance[object_index].move_power)
			{
				proc->object_instance[object_index].move_power = core->power_left;
				core->power_left = 0;
				return;
			}

			core->power_left -= proc->object_instance[object_index].move_power; // now use the new amount of power

//			core->power_use_predicted += (proc->object_instance[object_index].move_power + 9) / 10; // need to make sure 0-9 and 100 are treated correctly. See also the code that runs objects after execution

}







int call_class_method(struct core_struct* calling_core, int call_value)
{

	s16b stack_parameters [CALL_PARAMETERS+1]; // +1 is for class index

	if (call_value < 0
		|| call_value >= CALL_TYPES)
	{
		if (w.debug_mode)
 		print_method_error("class call invalid call type", 1, call_value);
		return 0;
	}

	if (!pull_values_from_stack(stack_parameters, call_type[call_value].parameters+1))
	{
		if (w.debug_mode)
 		print_method_error("class call stack error", 0, 0);
		return 0;
	}

	int class_index = stack_parameters [0];

	if (class_index < 0
		|| class_index >= OBJECT_CLASSES)
			return 0;

	struct template_struct* calling_template = &templ[calling_core->player_index][calling_core->template_index];

	if (calling_template->object_class_active [class_index] == 0)
		return 0;

 return call_known_object_or_class(calling_core, NULL, class_index, -1, call_value, &stack_parameters [1]);

}




// call this just after a core executes. It goes through each member's objects and uses energy, sets motion values etc
//  * currently it only does this for move objects, and it doesn't use power (that's handled elsewhere)
void run_objects_after_execution(struct core_struct* core)
{

//	int i, j;

 set_motion_from_move_objects(core);

}


	//for (i = 0; i < core->group_members_max; i++)
	//{
		//if (core->group_member[i].exists)
		//{
//			for (j = 0; j < w.proc[core->group_member[i].index].nshape_ptr->links; j ++)
//			{
//				if (w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost_finish_time > w.world_time)
//					core->power_used += w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost;

//				switch(w.proc[core->group_member[i].index].object[j].type)
//				{
// note that some things which change only when the whole process' composition changes (e.g. components are destroyed or rebuilt) as set in set_object_properties()
// 				case OBJECT_TYPE_MOVE:
/* 					if (core->stress_level >= STRESS_MODERATE)
						{
							if (core->stress_level == STRESS_MODERATE)
							 w.proc[core->group_member[i].index].object_instance[j].move_power /= 2;
							  else
							   w.proc[core->group_member[i].index].object_instance[j].move_power = 0;
						}*/
// should be able to assume move_power >= 0 (this should have been checked for when it was set)
//					 core->power_used += (w.proc[core->group_member[i].index].object_instance[j].move_power + 9) / 10; // need to make sure 0-9 and 100 are treated correctly. See also the code for CALL_SET_POWER etc

// note that power has already been used
//      w.proc[core->group_member[i].index].object_instance[j].move_power_last_cycle = w.proc[core->group_member[i].index].object_instance[j].move_power_this_cycle;
//      w.proc[core->group_member[i].index].object_instance[j].move_power_this_cycle = w.proc[core->group_member[i].index].object_instance[j].move_power;
//	 				break;

//					case OBJECT_TYPE_INTERFACE:
//						if (core->interface_active
//							&& w.proc[core->group_member[i].index].interface_on_process_set_on)
//							&& w.proc[core->group_member[i].index].object_instance[j].interface_object_active)
//								core->power_left -= 10; // I guess this should check to make sure that there's enough power for this. But if there's not that's probably okay as the process won't be doing anything anyway.
								    // could check for this during process definition validation as it will be obvious then if there are too many interface objects.
//						break;
/*
					case OBJECT_TYPE_BUILD:
						if (core->build_cooldown_time >= w.world_time)
							core->power_used += BUILD_POWER_COST;//otype[w.proc[core->group_member[i].index].object[j].type].object_details.power_cost;
						break;
					case OBJECT_TYPE_REPAIR:
					case OBJECT_TYPE_REPAIR_OTHER:
						if (core->restore_cooldown_time >= w.world_time)
							core->power_used += POWER_COST_RESTORE_COMPONENT;//otype[w.proc[core->group_member[i].index].object[j].type].object_details.power_cost;
						break;
*/

/*
				 case OBJECT_TYPE_PACKET_DIR:
				 case OBJECT_TYPE_LPACKET_DIR:
				 case OBJECT_TYPE_FPACKET_DIR:
				 case OBJECT_TYPE_PULSE_DIR:
 				case OBJECT_TYPE_PACKET:
 				case OBJECT_TYPE_LPACKET:
 				case OBJECT_TYPE_FPACKET:
 				case OBJECT_TYPE_PULSE:
 					if (w.proc[core->group_member[i].index].object_instance[j].packet_fire == 1
							&& w.proc[core->group_member[i].index].object_instance[j].packet_last_fired < w.world_time - 64)
						{
							w.proc[core->group_member[i].index].object_instance[j].packet_fire = 0;
							run_packet_object(core, &w.proc[core->group_member[i].index], j);
							core->power_used += POWER_COST_PACKET;
						}
	 				break;
*/

//				}
//			}
		//}
	//}

//	if (core->interface_active)
//		core->power_used += 30; // base rate for having interface

// the call to set_motion_from_move_objects() is separate because it also needs to be called when a group's physical properties change.

// }

// call this just before a core executes. It goes through each member's objects and uses energy
void run_objects_before_execution(struct core_struct* core)
{

	int i, j;

	for (i = 0; i < core->group_members_max; i++)
	{
		if (core->group_member[i].exists)
		{
			for (j = 0; j < w.proc[core->group_member[i].index].nshape_ptr->links; j ++)
			{

				if (w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost_finish_time > w.world_time)
				{
					core->power_left -= w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost;


//if (core->index == 1)
// fpr("\n core %i object %i opc %i time %i power_used %i stress %i", core->index, j, w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost, w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost_finish_time,
//					core->power_used, core->stress);

				}
				 else
						switch(w.proc[core->group_member[i].index].object[j].type)
					 {
					 	case OBJECT_TYPE_MOVE:
        w.proc[core->group_member[i].index].object_instance[j].move_power_last_cycle = w.proc[core->group_member[i].index].object_instance[j].move_power;
						  w.proc[core->group_member[i].index].object_instance[j].move_power = 0;
						  break;
						 case OBJECT_TYPE_INTERFACE:
								if (core->interface_active)
//									&& w.proc[core->group_member[i].index].interface_protects)
								{
//									fpr("\n using interface power");
										core->power_left -= INTERFACE_POWER_USE; // if changed, also change in otype definition
								}
//								 else
//										fpr("\n core %i ia %i ip %i", core->index, core->interface_active, w.proc[core->group_member[i].index].interface_protects);
								break;
							case OBJECT_TYPE_STABILITY:
								if (core->interface_active
									&& w.proc[core->group_member[i].index].interface_protects
									&& w.proc[core->group_member[i].index].interface_stability)
										core->power_left -= INTERFACE_STABILITY_POWER_USE; // if changed, also change in otype definition
								break;
/*						 case OBJECT_TYPE_INTERFACE:
						  if (core->interface_active
							  && w.proc[core->group_member[i].index].interface_on_process_set_on)
//							&& w.proc[core->group_member[i].index].object_instance[j].interface_object_active)
								  core->power_left -= 10; // I guess this should check to make sure that there's enough power for this. But if there's not that's probably okay as the process won't be doing anything anyway.
								    // could check for this during process definition validation as it will be obvious then if there are too many interface objects.
						  break;*/
							}
				}
			}
		}

//	if (core->interface_active)
//		core->power_used += 30; // base rate for having interface

// the call to set_motion_from_move_objects() is separate because it also needs to be called when a group's physical properties change.
 //set_motion_from_move_objects(core);

}




// call this:
//  - just after a core executes
//  - just after a group's physical properties change (e.g. a member is added or removed) - in this case, the base motion values of each move object should have been revised
//     * currently this is not done - it's only called after a core executes.
// It sets the core's constant_accel and constant_spin_change to be applied each tick until next execution
//  TO DO: optimise - shouldn't have to loop through each of every member's objects like this
void set_motion_from_move_objects(struct core_struct* core)
{

	if (core->mobile == 0)
		return;

	core->constant_accel_angle_offset = 0;
	core->constant_accel_rate = 0;
	core->constant_spin_change = 0;

 al_fixed accel_x = 0;
 al_fixed accel_y = 0;

	int i, j;

	for (i = 0; i < core->group_members_max; i++)
	{
		if (core->group_member[i].exists)
		{
			for (j = 0; j < MAX_OBJECTS; j ++)
			{
				switch(w.proc[core->group_member[i].index].object[j].type)
				{
 				case OBJECT_TYPE_MOVE:
 					if (w.proc[core->group_member[i].index].object_instance[j].move_power > 0)
						{
#define MOVE_DIVISOR 300
							accel_x += fixed_xpart(core->group_angle + w.proc[core->group_member[i].index].object_instance[j].move_accel_angle_offset,
																														w.proc[core->group_member[i].index].object_instance[j].move_power * w.proc[core->group_member[i].index].object_instance[j].move_accel_rate) / MOVE_DIVISOR;
							accel_y += fixed_ypart(core->group_angle + w.proc[core->group_member[i].index].object_instance[j].move_accel_angle_offset,
																														w.proc[core->group_member[i].index].object_instance[j].move_power * w.proc[core->group_member[i].index].object_instance[j].move_accel_rate) / MOVE_DIVISOR;
							core->constant_spin_change += w.proc[core->group_member[i].index].object_instance[j].move_power * w.proc[core->group_member[i].index].object_instance[j].move_spin_change / 300;
						}
	 				break;
				}
			}
		}
	}

 core->constant_accel_angle_offset = get_angle(accel_y, accel_x) - core->group_angle;
 core->constant_accel_rate = distance_oct(accel_y, accel_x); // should this be using sqrt distance instead??



}


// call this for each core every tick
//  TO DO: optimise by assembling a (linked?) list of objects to run each tick for each core, to avoid having to check all of them.
void run_objects_each_tick(struct core_struct* core)
{

	int i, j;

	for (i = 0; i < core->group_members_max; i++)
	{
		if (core->group_member[i].exists)
		{
			for (j = 0; j < MAX_OBJECTS; j ++)
			{
				switch(w.proc[core->group_member[i].index].object[j].type)
				{
			 case OBJECT_TYPE_PULSE:
			 case OBJECT_TYPE_PULSE_L:
			 case OBJECT_TYPE_PULSE_XL:
					 rotate_directional_object(&w.proc[core->group_member[i].index], j, otype[w.proc[core->group_member[i].index].object[j].type].object_details.rotate_speed);
			 case OBJECT_TYPE_BURST:
			 case OBJECT_TYPE_BURST_L:
			 case OBJECT_TYPE_BURST_XL:
 					if (w.proc[core->group_member[i].index].object_instance[j].attack_fire_timestamp == w.world_time
							&& w.proc[core->group_member[i].index].object_instance[j].attack_recycle_timestamp <= w.world_time)
//							&& core->stress_level <= STRESS_MODERATE)
						{
							run_packet_object(core, &w.proc[core->group_member[i].index], j);
//							core->power_used += otype[w.proc[core->group_member[i].index].object[j].type].object_details.power_cost;
// assume power_left was reduced when the object was called.
							w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost = otype[w.proc[core->group_member[i].index].object[j].type].object_details.power_cost;
							w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost_finish_time = w.proc[core->group_member[i].index].object_instance[j].attack_recycle_timestamp;
						}
					 break;

	 			case OBJECT_TYPE_STREAM_DIR: // rotation below
	 			case OBJECT_TYPE_STREAM:
	 				if (w.proc[core->group_member[i].index].object_instance[j].attack_fire_timestamp == w.world_time)
						{
//						 w.proc[core->group_member[i].index].object_instance[j].stream_fire = 0; // set to 0 even if firing fails
							if (w.proc[core->group_member[i].index].object_instance[j].attack_recycle_timestamp <= w.world_time)
//							 && core->stress_level <= STRESS_LOW)
						 {
							 w.proc[core->group_member[i].index].object_instance[j].attack_last_fire_timestamp = w.world_time;
							 w.proc[core->group_member[i].index].object_instance[j].attack_recycle_timestamp = w.world_time + STREAM_RECYCLE_TIME;
// 						 core->power_used += POWER_COST_STREAM; - assume power_left was reduced when the object was called.
 							w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost = POWER_COST_STREAM;
	 						w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost_finish_time = w.proc[core->group_member[i].index].object_instance[j].attack_recycle_timestamp;
// 						 play_game_sound(SAMPLE_STREAM1, TONE_1C, 90, 1, w.proc[core->group_member[i].index].position.x, w.proc[core->group_member[i].index].position.y);
						 }
						}

						if (w.proc[core->group_member[i].index].object_instance[j].attack_last_fire_timestamp >= w.world_time - STREAM_TOTAL_FIRING_TIME)
								run_stream_object(core, &w.proc[core->group_member[i].index], j, w.proc[core->group_member[i].index].object[j].type);
//								 else
									{
// no rotation while firing  <- not anymore
										if (w.proc[core->group_member[i].index].object[j].type == OBJECT_TYPE_STREAM_DIR)
										 rotate_directional_object(&w.proc[core->group_member[i].index], j, otype[w.proc[core->group_member[i].index].object[j].type].object_details.rotate_speed);
									}
						break;
			 case OBJECT_TYPE_SLICE:
 					if (w.proc[core->group_member[i].index].object_instance[j].attack_fire_timestamp == w.world_time
							&& w.proc[core->group_member[i].index].object_instance[j].attack_recycle_timestamp <= w.world_time)
						{
							 w.proc[core->group_member[i].index].object_instance[j].attack_last_fire_timestamp = w.world_time;
							 w.proc[core->group_member[i].index].object_instance[j].attack_recycle_timestamp = w.world_time + SLICE_RECYCLE_TIME;
// 						 core->power_used += POWER_COST_STREAM; - assume power_left was reduced when the object was called.
 							w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost = POWER_COST_SLICE;
	 						w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost_finish_time = w.proc[core->group_member[i].index].object_instance[j].attack_recycle_timestamp;
 						 play_game_sound(SAMPLE_SLICE, TONE_2C, 90, 1, w.proc[core->group_member[i].index].position.x, w.proc[core->group_member[i].index].position.y);
						}

						if (w.proc[core->group_member[i].index].object_instance[j].attack_last_fire_timestamp >= w.world_time - SLICE_TOTAL_FIRING_TIME)
								run_slice_object(core, &w.proc[core->group_member[i].index], j, w.proc[core->group_member[i].index].object[j].type);

					 rotate_directional_object(&w.proc[core->group_member[i].index], j, otype[w.proc[core->group_member[i].index].object[j].type].object_details.rotate_speed);

					 break;

				 case OBJECT_TYPE_SPIKE:
 					if (w.proc[core->group_member[i].index].object_instance[j].attack_fire_timestamp == w.world_time
							&& w.proc[core->group_member[i].index].object_instance[j].attack_recycle_timestamp <= w.world_time)
//							&& core->stress_level <= STRESS_MODERATE)
						{
							run_spike_object(core, &w.proc[core->group_member[i].index], j, w.proc[core->group_member[i].index].object_instance[j].spike_angle_offset);
//							core->power_used += otype[w.proc[core->group_member[i].index].object[j].type].object_details.power_cost;
							w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost = otype[w.proc[core->group_member[i].index].object[j].type].object_details.power_cost;
 						w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost_finish_time = w.proc[core->group_member[i].index].object_instance[j].attack_recycle_timestamp;
						}
					 break;

			 case OBJECT_TYPE_ULTRA_DIR:
					 rotate_directional_object(&w.proc[core->group_member[i].index], j, otype[w.proc[core->group_member[i].index].object[j].type].object_details.rotate_speed);
			 case OBJECT_TYPE_ULTRA:
 					if (w.proc[core->group_member[i].index].object_instance[j].attack_fire_timestamp == w.world_time
							&& w.proc[core->group_member[i].index].object_instance[j].attack_recycle_timestamp <= w.world_time)
						{
							run_packet_object(core, &w.proc[core->group_member[i].index], j);
// assume power_left was reduced when the object was called.
							w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost = otype[w.proc[core->group_member[i].index].object[j].type].object_details.power_cost;
							w.proc[core->group_member[i].index].object_instance[j].ongoing_power_cost_finish_time = w.proc[core->group_member[i].index].object_instance[j].attack_recycle_timestamp;
						}
					 break;


				}
			}
		}
	}

}


static void run_packet_object(struct core_struct* core, struct proc_struct* proc, int object_index)
{

//  int packet_type;
  //int packet_lifetime;



		al_fixed vertex_angle = proc->angle + proc->nshape_ptr->object_angle_fixed[object_index];
		al_fixed vertex_dist = proc->nshape_ptr->object_dist_fixed[object_index];

		int packet_index = new_packet(PACKET_TYPE_PULSE, // dummy value - replaced below
																																	core->player_index,
																																	core->index,
																																	core->created_timestamp,
																		               proc->position.x
																		                + fixed_xpart(vertex_angle, proc->nshape_ptr->object_dist_fixed[object_index])
																		                + fixed_xpart(vertex_angle + proc->object_instance[object_index].angle_offset, al_itofix(9)),
																		               proc->position.y
																		                + fixed_ypart(vertex_angle, proc->nshape_ptr->object_dist_fixed[object_index])
																		                + fixed_ypart(vertex_angle + proc->object_instance[object_index].angle_offset, al_itofix(9)));

		if (packet_index == -1) // can fail if too many packets already
			return;

 struct packet_struct* pack = &w.packet[packet_index];

 al_fixed packet_speed = al_itofix(otype[proc->object[object_index].type].object_details.packet_speed);

  int sample_to_play = SAMPLE_ZAP;


  switch(proc->object[object_index].type)
  {
  	default:
		 case OBJECT_TYPE_PULSE:
		 	pack->type = PACKET_TYPE_PULSE;
			 pack->lifetime = 120;
    pack->status = otype[proc->object[object_index].type].object_details.packet_size;
			 break;
		 case OBJECT_TYPE_PULSE_L:
		 	pack->type = PACKET_TYPE_PULSE;
			 pack->lifetime = 150; //
    pack->status = otype[proc->object[object_index].type].object_details.packet_size;
			 break;
		 case OBJECT_TYPE_PULSE_XL:
		 	pack->type = PACKET_TYPE_PULSE;
			 pack->lifetime = 180;
    pack->status = otype[proc->object[object_index].type].object_details.packet_size;
			 break;

		 case OBJECT_TYPE_BURST:
		 	pack->type = PACKET_TYPE_BURST;
			 pack->lifetime = 120;
    pack->status = otype[proc->object[object_index].type].object_details.packet_size;
			 break;
		 case OBJECT_TYPE_BURST_L:
		 	pack->type = PACKET_TYPE_BURST;
			 pack->lifetime = 150;
    pack->status = otype[proc->object[object_index].type].object_details.packet_size;
			 break;
		 case OBJECT_TYPE_BURST_XL:
		 	pack->type = PACKET_TYPE_BURST;
			 pack->lifetime = 180;
    pack->status = otype[proc->object[object_index].type].object_details.packet_size;
			 break;
			case OBJECT_TYPE_ULTRA:
				pack->type = PACKET_TYPE_ULTRA;
			 pack->lifetime = 170;
// status is shade of packet centre (outer is status - 6)
    pack->status = 31;////otype[proc->object[object_index].type].object_details.packet_size;
    sample_to_play = SAMPLE_ULTRA;
			 break;
			case OBJECT_TYPE_ULTRA_DIR:
				pack->type = PACKET_TYPE_ULTRA;
			 pack->lifetime = 170;
    pack->status = 24;//otype[proc->object[object_index].type].object_details.packet_size;
    sample_to_play = SAMPLE_ULTRA;
			 break;

  }

// the old_x etc values are used to work out the velocity of the vertex (which is added to the packet's velocity).
// the calculation isn't perfect, because the packet isn't created exactly at the vertex. But it's close enough.
 al_fixed vertex_x  = proc->position.x + fixed_xpart(vertex_angle, vertex_dist);
 al_fixed vertex_y  = proc->position.y + fixed_ypart(vertex_angle, vertex_dist);
 al_fixed vertex_old_x = proc->old_position.x + fixed_xpart(proc->old_angle + proc->nshape_ptr->object_angle_fixed[object_index], vertex_dist);
 al_fixed vertex_old_y = proc->old_position.y + fixed_ypart(proc->old_angle + proc->nshape_ptr->object_angle_fixed[object_index], vertex_dist);


 pack->speed.x = (vertex_x - vertex_old_x) + fixed_xpart(vertex_angle + proc->object_instance[object_index].angle_offset, packet_speed);
 pack->speed.y = (vertex_y - vertex_old_y) + fixed_ypart(vertex_angle + proc->object_instance[object_index].angle_offset, packet_speed);

 pack->created_timestamp = w.world_time;
// pack->lifetime = packet_lifetime;

 pack->damage = otype[proc->object[object_index].type].object_details.damage;
 pack->colour = proc->player_index;
 pack->team_safe = proc->player_index;

// the packet angle for most packet types needs to be worked out after speed is calculated
// if (proc->object[object_index].type != OBJECT_TYPE_SURGE)
  pack->angle = get_angle(pack->speed.y, pack->speed.x);
//   else
//    pack->angle = vertex_angle + proc->object_instance[object_index].angle_offset;

 pack->collision_size = 0; // - not currently implemented

 pack->source_proc = proc->index;

 proc->object_instance[object_index].attack_last_fire_timestamp = w.world_time;
 proc->object_instance[object_index].attack_recycle_timestamp = w.world_time + otype[proc->object[object_index].type].object_details.recycle_time;

 play_game_sound(sample_to_play, TONE_2E - pack->status*3, 50 + pack->status * 16, 5, pack->position.x, pack->position.y);


}

/*
static void run_burst_object(struct core_struct* core, struct proc_struct* proc, int object_index)
{

  int packet_damage = 100;//0;
  int packet_lifetime = 240;
//  int packet_status;

  int packet_speed = otype[proc->object[object_index].type].object_details.packet_speed;

		al_fixed vertex_angle = proc->angle + proc->nshape_ptr->object_angle_fixed[object_index];
		al_fixed vertex_dist = proc->nshape_ptr->object_dist_fixed[object_index];

		int packet_index = new_packet(PACKET_TYPE_BURST,
																																	core->player_index,
																																	core->index,
																																	core->created_timestamp,
																		               proc->position.x
																		                + fixed_xpart(vertex_angle, proc->nshape_ptr->object_dist_fixed[object_index])
																		                + fixed_xpart(vertex_angle + proc->object_instance[object_index].angle_offset, al_itofix(9)),
																		               proc->position.y
																		                + fixed_ypart(vertex_angle, proc->nshape_ptr->object_dist_fixed[object_index])
																		                + fixed_ypart(vertex_angle + proc->object_instance[object_index].angle_offset, al_itofix(9)));

		if (packet_index == -1) // can fail if too many packets already
			return;

 struct packet_struct* pack = &w.packet[packet_index];

// the old_x etc values are used to work out the velocity of the vertex (which is added to the packet's velocity).
// the calculation isn't perfect, because the packet isn't created exactly at the vertex. But it's close enough.
 al_fixed vertex_x  = proc->position.x + fixed_xpart(vertex_angle, vertex_dist);
 al_fixed vertex_y  = proc->position.y + fixed_ypart(vertex_angle, vertex_dist);
 al_fixed vertex_old_x = proc->old_position.x + fixed_xpart(proc->old_angle + proc->nshape_ptr->object_angle_fixed[object_index], vertex_dist);
 al_fixed vertex_old_y = proc->old_position.y + fixed_ypart(proc->old_angle + proc->nshape_ptr->object_angle_fixed[object_index], vertex_dist);

 pack->speed.x = (vertex_x - vertex_old_x) + fixed_xpart(vertex_angle + proc->object_instance[object_index].angle_offset, al_itofix(packet_speed));
 pack->speed.y = (vertex_y - vertex_old_y) + fixed_ypart(vertex_angle + proc->object_instance[object_index].angle_offset, al_itofix(packet_speed));

 pack->created_timestamp = w.world_time;
 pack->lifetime = packet_lifetime;
// pack->status = packet_status;

 pack->angle = get_angle(pack->speed.y, pack->speed.x);
 pack->damage = packet_damage;
 pack->colour = proc->player_index;
 pack->team_safe = proc->player_index;

 pack->collision_size = 0; //pr->method[m].extension [MEX_PR_PACKET_POWER]; // used for actual collision tests (probably not implemented)

 pack->source_proc = proc->index;

 proc->object_instance[object_index].packet_last_fired = w.world_time;

}
*/


static void run_spike_object(struct core_struct* core, struct proc_struct* proc, int object_index, int firing_angle_offset_int)
{

  int packet_speed = 1; //otype[proc->object[object_index].type].intercept_speed;

		al_fixed vertex_angle = proc->angle + proc->nshape_ptr->object_angle_fixed[object_index];
		al_fixed vertex_dist = proc->nshape_ptr->object_dist_fixed[object_index];
//		al_fixed firing_angle_offset = int_angle_to_fixed(firing_angle_offset_int);

		int packet_index = new_packet(PACKET_TYPE_SPIKE1,
																																	core->player_index,
																																	core->index,
																																	core->created_timestamp,
																		               proc->position.x
																		                + fixed_xpart(vertex_angle, proc->nshape_ptr->object_dist_fixed[object_index] + al_itofix(9)),
																		               proc->position.y
																		                + fixed_ypart(vertex_angle, proc->nshape_ptr->object_dist_fixed[object_index] + al_itofix(9)));

		if (packet_index == -1) // can fail if too many packets already
			return;

 struct packet_struct* pack = &w.packet[packet_index];

// the old_x etc values are used to work out the velocity of the vertex (which is added to the packet's velocity).
// the calculation isn't perfect, because the packet isn't created exactly at the vertex. But it's close enough.
 al_fixed vertex_x  = proc->position.x + fixed_xpart(vertex_angle, vertex_dist);
 al_fixed vertex_y  = proc->position.y + fixed_ypart(vertex_angle, vertex_dist);
 al_fixed vertex_old_x = proc->old_position.x + fixed_xpart(proc->old_angle + proc->nshape_ptr->object_angle_fixed[object_index], vertex_dist);
 al_fixed vertex_old_y = proc->old_position.y + fixed_ypart(proc->old_angle + proc->nshape_ptr->object_angle_fixed[object_index], vertex_dist);

 pack->speed.x = (vertex_x - vertex_old_x) + fixed_xpart(vertex_angle, al_itofix(packet_speed));
 pack->speed.y = (vertex_y - vertex_old_y) + fixed_ypart(vertex_angle, al_itofix(packet_speed));

 pack->created_timestamp = w.world_time;
// pack->lifetime = 32; // this is just its first stage
// pack->fixed_status = packet_status;

 pack->angle = vertex_angle;// + int_angle_to_fixed(firing_angle_offset_int);//get_angle(pack->speed.y, pack->speed.x);
//#define SPIKE_ROTATE_SPEED int_angle_to_fixed(2)
 if (firing_angle_offset_int < 0)
  pack->fixed_status = 0 - int_angle_to_fixed(40);
   else
			{
    if (firing_angle_offset_int > 0)
     pack->fixed_status = int_angle_to_fixed(40);
      else
       pack->fixed_status = 0;
			}

// now introduce a small pseudorandom factor	to spike rotation:
 firing_angle_offset_int += ((w.world_time + object_index) % 5) - 2;

// fpr("\nspike fao %i fs %f", firing_angle_offset_int, al_fixtof(pack->fixed_status));
 pack->lifetime = abs(firing_angle_offset_int) / 40;//angle_difference(vertex_angle, vertex_angle + firing_angle_offset) / 2;
// fpr("\n lifetime %i", pack->lifetime);
 pack->damage = SPIKE_BASE_DAMAGE;
 pack->colour = proc->player_index;
 pack->team_safe = proc->player_index;
 pack->status = -1; // trailing cloud index

 pack->collision_size = 0; //pr->method[m].extension [MEX_PR_PACKET_POWER]; // used for actual collision tests (may not be currently implemented)

 pack->source_proc = proc->index;

 proc->object_instance[object_index].attack_last_fire_timestamp = w.world_time;
 proc->object_instance[object_index].attack_recycle_timestamp = w.world_time + otype[proc->object[object_index].type].object_details.recycle_time;

 play_game_sound(SAMPLE_CHIRP, TONE_2C, 70, 5, pack->position.x, pack->position.y); // was SAMPLE_SPIKE but that may be a bit too harsh

}



// Note that this function assumes that angle offsets can't be 360 degrees (e.g. they can be -90 to 90)
static void rotate_directional_object(struct proc_struct* proc, int object_index, al_fixed turn_speed)
{

// fpr("\nrotate object %i: angle_offset %i rtao %i turn_speed %i", object_index, al_fixtoi(proc->object_instance[object_index].angle_offset), al_fixtoi(proc->object_instance[object_index].rotate_to_angle_offset), al_fixtoi(turn_speed));

	if (proc->object_instance[object_index].angle_offset < proc->object_instance[object_index].rotate_to_angle_offset)
	{
		proc->object_instance[object_index].angle_offset += turn_speed;
		if (proc->object_instance[object_index].angle_offset > proc->object_instance[object_index].rotate_to_angle_offset)
			proc->object_instance[object_index].angle_offset = proc->object_instance[object_index].rotate_to_angle_offset;
		if (proc->object_instance[object_index].angle_offset > AFX_ANGLE_4)
			proc->object_instance[object_index].angle_offset = AFX_ANGLE_4;
	}
	if (proc->object_instance[object_index].angle_offset > proc->object_instance[object_index].rotate_to_angle_offset)
	{
		proc->object_instance[object_index].angle_offset -= turn_speed;
		if (proc->object_instance[object_index].angle_offset < proc->object_instance[object_index].rotate_to_angle_offset)
			proc->object_instance[object_index].angle_offset = proc->object_instance[object_index].rotate_to_angle_offset;
		if (proc->object_instance[object_index].angle_offset < -AFX_ANGLE_4)
			proc->object_instance[object_index].angle_offset = -AFX_ANGLE_4;
	}

}



// Basic target-leading algorithm that returns an offset from base_angle (which can e.g. be put
//  directly into the angle register of a dpacket method).
// relative_x/y are the x/y distance from the source (e.g. the vertex with the dpacket method)
//  to the target.
// relative_speed_x/y are the target's speeds minus the source's speed.
// base_angle is the angle of the source (e.g. the angle from a process to a vertex)
// intercept_speed is the speed of the projectile/intercepting process.
static al_fixed lead_target(struct core_struct* firing_core,
																												struct proc_struct* firing_proc,
																												int object_index,
																												struct core_struct* target_core,
																												struct proc_struct* target_proc,
																			         int intercept_speed)
{

#define LEAD_ITERATIONS 2
// Testing indicates that only a few iterations are really needed.

	al_fixed vertex_angle = firing_proc->angle + firing_proc->nshape_ptr->object_angle_fixed[object_index];// + firing_proc->object[object_index].angle_offset;
 vertex_angle &= AFX_MASK;
	al_fixed vertex_dist = firing_proc->nshape_ptr->object_dist_fixed[object_index];

 al_fixed source_x = firing_proc->position.x
                     + fixed_xpart(vertex_angle,	vertex_dist);
 al_fixed source_y = firing_proc->position.y
                     + fixed_ypart(vertex_angle, vertex_dist);

	al_fixed relative_x = target_proc->position.x - source_x;
	al_fixed relative_y = target_proc->position.y - source_y;

 al_fixed vertex_x = firing_proc->position.x + fixed_xpart(vertex_angle, vertex_dist);
 al_fixed vertex_y = firing_proc->position.y + fixed_ypart(vertex_angle, vertex_dist);
 al_fixed vertex_old_x = firing_proc->old_position.x + fixed_xpart(firing_proc->old_angle + firing_proc->nshape_ptr->object_angle_fixed[object_index], vertex_dist);
 al_fixed vertex_old_y = firing_proc->old_position.y + fixed_ypart(firing_proc->old_angle + firing_proc->nshape_ptr->object_angle_fixed[object_index], vertex_dist);

	al_fixed relative_speed_x = target_proc->speed.x - (vertex_x - vertex_old_x);
	al_fixed relative_speed_y = target_proc->speed.y - (vertex_y - vertex_old_y);

 int i;
 int flight_time;
 al_fixed modified_target_x, modified_target_y;
 al_fixed dist;
// al_fixed intercept_angle;

 modified_target_x = relative_x;
 modified_target_y = relative_y;


 for (i = 0; i < LEAD_ITERATIONS; i ++)
	{
//  dist = distance(modified_target_y, modified_target_x);
  dist = distance_oct(modified_target_y, modified_target_x);

  flight_time = al_fixtoi(dist / intercept_speed) + 4;

	 modified_target_x = relative_x + ((relative_speed_x * flight_time));
	 modified_target_y = relative_y + ((relative_speed_y * flight_time));
	}

 return angle_difference_signed(vertex_angle, get_angle(modified_target_y, modified_target_x));

}


static al_fixed get_spike_target_angle(struct core_struct* firing_core,
																																struct proc_struct* firing_proc,
																																int object_index,
																																struct core_struct* target_core,
																												    struct proc_struct* target_proc)
{

	al_fixed base_angle = firing_proc->angle + firing_proc->nshape_ptr->object_angle_fixed[object_index];
 base_angle &= AFX_MASK;

 al_fixed source_x = firing_proc->position.x
                     + fixed_xpart(base_angle,
																																			firing_proc->nshape_ptr->object_dist_fixed[object_index] + al_itofix(SPIKE_SIDE_TRAVEL))
																					+ (firing_proc->speed.x * SPIKE_SIDE_SPEED_MULTIPLIER);

// should really add speed here
 al_fixed source_y = firing_proc->position.y
                     + fixed_ypart(base_angle,
																																			firing_proc->nshape_ptr->object_dist_fixed[object_index] + al_itofix(SPIKE_SIDE_TRAVEL))
																					+ (firing_proc->speed.y * SPIKE_SIDE_SPEED_MULTIPLIER);

	al_fixed relative_x = target_proc->position.x - source_x;
	al_fixed relative_y = target_proc->position.y - source_y;
	al_fixed relative_speed_x = target_proc->speed.x - firing_proc->speed.x;
	al_fixed relative_speed_y = target_proc->speed.y - firing_proc->speed.y;


 int i;
 int flight_time;
 al_fixed modified_target_x, modified_target_y;
 al_fixed dist;
// al_fixed intercept_angle;

 modified_target_x = relative_x;
 modified_target_y = relative_y;

 for (i = 0; i < LEAD_ITERATIONS; i ++)
	{
//  dist = distance(modified_target_y, modified_target_x);
  dist = distance_oct(modified_target_y, modified_target_x);

  flight_time = al_fixtoi(dist / 6);

	 modified_target_x = relative_x + ((relative_speed_x * flight_time));
	 modified_target_y = relative_y + ((relative_speed_y * flight_time));
	}

 return angle_difference_signed(base_angle, get_angle(modified_target_y, modified_target_x));




}

// like lead_target, but calculates the angle a fixed object should point in to hit the target (as an offset from its current angle).
//  to do: should be able to merge these functions as the difference between them is very small.
static al_fixed lead_target_with_fixed_object(struct core_struct* firing_core,
																												struct proc_struct* firing_proc,
																												int object_index,
																												struct core_struct* target_core,
																												struct proc_struct* target_proc,
																			         int intercept_speed)
{

//#define LEAD_ITERATIONS 2 - defined in lead_target
// Testing indicates that only 2 iterations are really needed.

	al_fixed base_angle = firing_proc->angle + firing_proc->nshape_ptr->object_angle_fixed[object_index];// + firing_proc->object[object_index].angle_offset; ** angle_offset is factored into the return value below
 base_angle &= AFX_MASK;

 al_fixed source_x = firing_proc->position.x
                     + fixed_xpart(base_angle,
																																			firing_proc->nshape_ptr->object_dist_fixed[object_index]);
 al_fixed source_y = firing_proc->position.y
                     + fixed_ypart(base_angle,
																																			firing_proc->nshape_ptr->object_dist_fixed[object_index]);

	al_fixed relative_x = target_proc->position.x - source_x;
	al_fixed relative_y = target_proc->position.y - source_y;
	al_fixed relative_speed_x = target_proc->speed.x - firing_proc->speed.x;
	al_fixed relative_speed_y = target_proc->speed.y - firing_proc->speed.y;


 int i;
 int flight_time;
 al_fixed modified_target_x, modified_target_y;
 al_fixed dist;
// al_fixed intercept_angle;

 modified_target_x = relative_x;
 modified_target_y = relative_y;

 for (i = 0; i < LEAD_ITERATIONS; i ++)
	{
//  dist = distance(modified_target_y, modified_target_x);
  dist = distance_oct(modified_target_y, modified_target_x);

  flight_time = al_fixtoi(dist / intercept_speed) + 4;

	 modified_target_x = relative_x + ((relative_speed_x * flight_time));
	 modified_target_y = relative_y + ((relative_speed_y * flight_time));
	}

/*
if (firing_proc->index == 0
	&& object_index == 0)
 fpr("\nproc %i at %i,%i firing at proc %i at %i,%i; base_angle %i target_angle %i angle_diff %i",
					firing_proc->index,
					al_fixtoi(firing_proc->position.x),
					al_fixtoi(firing_proc->position.y),
					target_proc->index,
					al_fixtoi(target_proc->position.x),
					al_fixtoi(target_proc->position.y),
					al_fixtoi(base_angle),
					al_fixtoi(get_angle(modified_target_y, modified_target_x)),
					al_fixtoi(angle_difference_signed(base_angle, get_angle(modified_target_y, modified_target_x))));
*/

// unlike lead_target, this function needs to add the object's angle to the base_angle
//  (so that the angle offset returned is an offset from the object's actual permanent angle)

 return angle_difference_signed(base_angle + firing_proc->object_instance[object_index].angle_offset, get_angle(modified_target_y, modified_target_x));

}


// Call this for both STREAM and STREAM_DIR objects
// Call only if:
//  - object_instance.stream_fir
static void run_stream_object(struct core_struct* core, struct proc_struct* proc, int object_index, int object_type)
{

 struct cloud_struct* cl;
 int step_limit = 96;
 int steps = step_limit;
 int damage = otype[object_type].object_details.damage;

	int firing_stage = 1; // 0 = warmup, 1 = firing, 2 = cooldown


	int time_since_firing = w.world_time - proc->object_instance[object_index].attack_last_fire_timestamp;

// warmup
 if (time_since_firing <= STREAM_WARMUP_LENGTH)
	{
  steps = ((time_since_firing) * step_limit) / STREAM_WARMUP_LENGTH;
  firing_stage = 0;

  if (time_since_firing == STREAM_WARMUP_LENGTH)
		 play_game_sound(SAMPLE_STREAM2, TONE_1C, 100, 1, proc->position.x, proc->position.y);

	}
	 else
		{
			if (time_since_firing > (STREAM_TOTAL_FIRING_TIME - STREAM_COOLDOWN_LENGTH))
				firing_stage = 2;
			steps += time_since_firing - STREAM_WARMUP_LENGTH;
		}


// cooldown doesn't affect length

/*   else
			{
    if (time_since_firing > STREAM_TOTAL_FIRING_TIME - STREAM_COOLDOWN_LENGTH)
     steps += time_since_firing - STREAM_TOTAL_FIRING_TIME;
			}*/


// cooldown
//  steps += pr->method[m].data [MDATA_PR_STREAM_COUNTER] - STREAM_COOLDOWN_LENGTH;

// Now is firing, warmup up or cooling down. Need to do collision detection for the beam:

 al_fixed start_x, start_y;
 al_fixed x, y;
 al_fixed x_inc, y_inc;
 al_fixed firing_angle_fixed = proc->angle + proc->nshape_ptr->object_angle_fixed [object_index] + proc->object_instance[object_index].angle_offset;

// int steps;
 int i;
 int hit_proc = -1;

// start_x = pr->x + fixed_xpart(pr->angle + pr->shape_str->vertex_angle [pr->method[m].ex_vertex], pr->shape_str->vertex_dist [pr->method[m].ex_vertex] + al_itofix(4));
// start_y = pr->y + fixed_ypart(firing_angle_fixed, pr->shape_str->vertex_dist [pr->method[m].ex_vertex] + al_itofix(4));
 start_x = proc->position.x + fixed_xpart(proc->angle + proc->nshape_ptr->object_angle_fixed [object_index], proc->nshape_ptr->object_dist_fixed [object_index]);
 start_y = proc->position.y + fixed_ypart(proc->angle + proc->nshape_ptr->object_angle_fixed [object_index], proc->nshape_ptr->object_dist_fixed [object_index]);
 start_x += fixed_xpart(proc->angle + proc->nshape_ptr->object_angle_fixed [object_index] + proc->object_instance[object_index].angle_offset, al_itofix(8));
 start_y += fixed_ypart(proc->angle + proc->nshape_ptr->object_angle_fixed [object_index] + proc->object_instance[object_index].angle_offset, al_itofix(8));

 x = start_x;
 y = start_y;
 x_inc = fixed_xpart(firing_angle_fixed, STREAM_FIX_STEP_PIXELS);
 y_inc = fixed_ypart(firing_angle_fixed, STREAM_FIX_STEP_PIXELS);

 for (i = 0; i < steps; i ++)
 {
// collision check
 hit_proc = check_point_collision_ignore_team(x, y, proc->player_index, 1);

 if (hit_proc != -1)
 {
//  w.proc[hit_proc].hit_pulse_time = w.world_time;
/*
    if (firing_stage == 1)
//				&& (w.world_time & 3) == 3)
				{
     cl = new_cloud(CLOUD_STREAM_HIT, 8, x, y);
     if (cl != NULL)
     {
      cl->angle = get_angle(y - w.proc[hit_proc].position.y, x - w.proc[hit_proc].position.x) + int_angle_to_fixed(ANGLE_2 + grand(512) - 255);
      cl->colour = proc->player_index;
      cl->data [0] = w.world_time + proc->speed.x; // used as random seed
     }
				}*/


//  if (pr->method[m].data [MDATA_PR_STREAM_STATUS] == STREAM_STATUS_FIRING)
  //{

   w.proc[hit_proc].component_hit_time = w.world_time;
   w.proc[hit_proc].component_hit_source_index = core->index;
   w.proc[hit_proc].component_hit_source_timestamp = core->created_timestamp;
// Not sure whether component_hit_time should be set if attack is not in the damaging stage. Maybe.

   if (firing_stage == 1)
			{

    if (!w.proc[hit_proc].interface_protects
 				|| !w.core[w.proc[hit_proc].core_index].interface_active)
 			{
     w.proc[hit_proc].hit_pulse_time = w.world_time;
			  damage *= 2;
 			}

//  apply_packet_damage_to_proc(&w.proc[hit_proc], damage, core->player_index, core->index, core->created_timestamp);

    apply_packet_damage_to_proc(&w.proc[hit_proc], damage, core->player_index, core->index, core->created_timestamp);

			}
  //}
  break;
 }


// edge check
  if (x < (al_itofix(2) * BLOCK_SIZE_PIXELS)
   || x > w.fixed_size.x - (al_itofix(2 * BLOCK_SIZE_PIXELS))
   || y < (al_itofix(2) * BLOCK_SIZE_PIXELS)
   || y > w.fixed_size.y - (al_itofix(2 * BLOCK_SIZE_PIXELS)))
   break; // hit_proc will be -1

  x += x_inc;
  y += y_inc;

 }

// now create the beam cloud thing:

     cl = new_cloud(CLOUD_STREAM, 1, start_x, start_y);

     if (cl != NULL)
     {
//      cl->lifetime = 16;
      cl->colour = proc->player_index;
      cl->position2.x = x;
      cl->position2.y = y;
//      cl->data [0] = ;
      cl->data [1] = time_since_firing;
      cl->data [2] = 0;
      if (hit_proc != -1)
       cl->data [2] = 1;
// work out the display bounding box:
      cl->display_size_x1 = -20 - al_fixtof(abs(x - start_x));
      cl->display_size_y1 = -20 - al_fixtof(abs(y - start_y));
      cl->display_size_x2 = 20 + al_fixtof(abs(x - start_x));
      cl->display_size_y2 = 20 + al_fixtof(abs(y - start_y));
     }

}



// Call this for both SURGE and SURGE_DIR objects
static void run_slice_object(struct core_struct* core, struct proc_struct* proc, int object_index, int object_type)
{

 struct cloud_struct* cl;
// int step_limit = 128;
 int steps = 128;
 int steps_taken; // steps actual stepped through
 int damage = otype[object_type].object_details.damage;

	int time_since_firing = w.world_time - proc->object_instance[object_index].attack_last_fire_timestamp;

// Now is firing, warmup up or cooling down. Need to do collision detection for the beam:

 al_fixed start_x, start_y;
 al_fixed x, y;
 al_fixed x_inc, y_inc;
 al_fixed firing_angle_fixed = proc->angle + proc->nshape_ptr->object_angle_fixed [object_index] + proc->object_instance[object_index].angle_offset;

// int steps;
 int i;
 int hit_proc = -1;

// start_x = pr->x + fixed_xpart(pr->angle + pr->shape_str->vertex_angle [pr->method[m].ex_vertex], pr->shape_str->vertex_dist [pr->method[m].ex_vertex] + al_itofix(4));
// start_y = pr->y + fixed_ypart(firing_angle_fixed, pr->shape_str->vertex_dist [pr->method[m].ex_vertex] + al_itofix(4));
 start_x = proc->position.x + fixed_xpart(proc->angle + proc->nshape_ptr->object_angle_fixed [object_index], proc->nshape_ptr->object_dist_fixed [object_index]);
 start_y = proc->position.y + fixed_ypart(proc->angle + proc->nshape_ptr->object_angle_fixed [object_index], proc->nshape_ptr->object_dist_fixed [object_index]);
 start_x += fixed_xpart(proc->angle + proc->nshape_ptr->object_angle_fixed [object_index] + proc->object_instance[object_index].angle_offset, al_itofix(8));
 start_y += fixed_ypart(proc->angle + proc->nshape_ptr->object_angle_fixed [object_index] + proc->object_instance[object_index].angle_offset, al_itofix(8));

 x = start_x;
 y = start_y;
 x_inc = fixed_xpart(firing_angle_fixed, STREAM_FIX_STEP_PIXELS);
 y_inc = fixed_ypart(firing_angle_fixed, STREAM_FIX_STEP_PIXELS);

 for (i = 0; i < steps; i ++)
 {
// collision check
 hit_proc = check_point_collision_ignore_team(x, y, proc->player_index, 1);

 if (hit_proc != -1)
 {
  w.proc[hit_proc].component_hit_time = w.world_time;
  w.proc[hit_proc].component_hit_source_index = core->index;
  w.proc[hit_proc].component_hit_source_timestamp = core->created_timestamp;

/*
    if (firing_stage == 1)
//				&& (w.world_time & 3) == 3)
				{
     cl = new_cloud(CLOUD_STREAM_HIT, 8, x, y);
     if (cl != NULL)
     {
      cl->angle = get_angle(y - w.proc[hit_proc].position.y, x - w.proc[hit_proc].position.x) + int_angle_to_fixed(ANGLE_2 + grand(512) - 255);
      cl->colour = proc->player_index;
      cl->data [0] = w.world_time + proc->speed.x; // used as random seed
     }
				}*/


//  if (pr->method[m].data [MDATA_PR_STREAM_STATUS] == STREAM_STATUS_FIRING)
  //{
   if (!w.proc[hit_proc].interface_protects
				|| !w.core[w.proc[hit_proc].core_index].interface_active)
			{
    w.proc[hit_proc].hit_pulse_time = w.world_time;
			 damage *= 2;
			}

  apply_packet_damage_to_proc(&w.proc[hit_proc], damage, core->player_index, core->index, core->created_timestamp);

  //}
  break;
 }


// edge check
  if (x < (al_itofix(2) * BLOCK_SIZE_PIXELS)
   || x > w.fixed_size.x - (al_itofix(2 * BLOCK_SIZE_PIXELS))
   || y < (al_itofix(2) * BLOCK_SIZE_PIXELS)
   || y > w.fixed_size.y - (al_itofix(2 * BLOCK_SIZE_PIXELS)))
   break; // hit_proc will be -1

  x += x_inc;
  y += y_inc;

 }

  steps_taken = i;


// now create the beam cloud thing:

			if (time_since_firing == SLICE_FIRING_TIME)
			{
     cl = new_cloud(CLOUD_SLICE_FADE, 16, start_x, start_y);

     if (cl != NULL)
     {
//      cl->lifetime = 16;
      cl->colour = proc->player_index;
      cl->position2.x = x;
      cl->position2.y = y;
//      cl->data [0] = ;
      cl->data [0] = steps_taken;
      if (hit_proc != -1)
       cl->data [2] = 1;
// work out the display bounding box:
      cl->display_size_x1 = -20 - al_fixtof(abs(x - start_x));
      cl->display_size_y1 = -20 - al_fixtof(abs(y - start_y));
      cl->display_size_x2 = 20 + al_fixtof(abs(x - start_x));
      cl->display_size_y2 = 20 + al_fixtof(abs(y - start_y));
     }

     return;

			}


     cl = new_cloud(CLOUD_SLICE, 1, start_x, start_y);

     if (cl != NULL)
     {
//      cl->lifetime = 16;
      cl->colour = proc->player_index;
      cl->position2.x = x;
      cl->position2.y = y;
//      cl->data [0] = ;
      cl->data [1] = time_since_firing;
      cl->data [2] = 0;
      if (hit_proc != -1)
       cl->data [2] = 1;
// work out the display bounding box:
      cl->display_size_x1 = -20 - al_fixtof(abs(x - start_x));
      cl->display_size_y1 = -20 - al_fixtof(abs(y - start_y));
      cl->display_size_x2 = 20 + al_fixtof(abs(x - start_x));
      cl->display_size_y2 = 20 + al_fixtof(abs(y - start_y));
     }

}


/*

// Basic target-leading algorithm that returns an offset from base_angle (which can e.g. be put
//  directly into the angle register of a dpacket method).
// relative_x/y are the x/y distance from the source (e.g. the vertex with the dpacket method)
//  to the target.
// relative_speed_x/y are the target's speeds minus the source's speed.
// base_angle is the angle of the source (e.g. the angle from a process to a vertex)
// intercept_speed is the speed of the projectile/intercepting process.
static al_fixed lead_target2(al_fixed relative_x, al_fixed relative_y,
																							al_fixed relative_speed_x, al_fixed relative_speed_y,
																			    al_fixed base_angle,
																			    int intercept_speed)
{

#define LEAD_ITERATIONS 2
// Testing indicates that only 2 iterations are really needed.

 int i;
 int flight_time;
 al_fixed modified_target_x, modified_target_y;
 al_fixed dist;
// al_fixed intercept_angle;

 modified_target_x = relative_x;
 modified_target_y = relative_y;

 for (i = 0; i < LEAD_ITERATIONS; i ++)
	{
  dist = distance(modified_target_y, modified_target_x);

  flight_time = al_fixtoi(dist / intercept_speed);

	 modified_target_x = relative_x + ((relative_speed_x * flight_time));
	 modified_target_y = relative_y + ((relative_speed_y * flight_time));
	}

 return angle_difference_signed(base_angle, get_angle(modified_target_y, modified_target_x));

}
*/




/*
// call this:
//  - when a core is newly created
//   * currently it is not called when a core is newly created. See g_proc_new.c.
//  - when a component is added to or removed from a group
// and after the group's centre of mass has been calculated.
// it recalculates each move object's basic acceleration and spin change properties based on the group's new properties.
void setup_move_objects(struct core_struct* core)
{

	if (core->mobile == 0)
		return;

	int i, j;
	int process_index;

	for (i = 0; i < GROUP_MAX_MEMBERS; i++)
	{
		if (core->group_member[i].index != -1)
		{
			for (j = 0; j < MAX_OBJECTS; j ++)
			{
				if (w.proc[core->group_member[i].index].object[j].type == OBJECT_TYPE_MOVE)
				{
					calculate_move_object_properties(core, &w.proc[core->group_member[i].index], j);
				}
			}
		}
	}

}*/

/*

How will object/class calls work?

e.g. move_to
 - would be best to avoid calculating target vector separately for each object.
 - So, should calculate it when the call is made, then call another function

*/


/*
This is the pass through each proc's methods that occurs each tick.
In some places it assumes that the pass that occurs after each proc's code execution (in active_method_pass_after_execution()) does bounds-checking.


* /
void active_method_pass_each_tick(void)
{

}

// This function is called after a proc's code is executed.
// It runs some methods, and sets up other methods based on values put into the mbank during execution
// The methods may then do things during active_method_pass_each_tick
void active_method_pass_after_execution(struct proc_struct* pr)
{

}
*/


/*

How will build objects work?

New command types:
 -


New methods:
 (core) am_i_selected() - returns:
  - 0 not selected
  - 1 selected; only one selected
  - 2 same as 1 but just happened
  - 3 selected; one of many
  - 4 same as 3 but just happened
  - Maybe:
   - 5 selected; first of multi-select
   - 6 etc

build buttons:
 - when user single-selects a core with a build object somewhere, build buttons automatically appear
 - buttons disappear if core deselected or if other cores also selected
 - also probably have buttons with current build queue - click on to remove from queue.



*/

/*

How will interface objects work?

object_interface // generates interface for single component. adds ?32 to interface strength
object_interface_depth // adds ?64 to interface without generating anything
object_interface_stability // interface takes reduced damage at single component
object_interface_response // ++ charge speed

object methods:
 object_interface
  .set_status(int) - 0 or 1. turning off reduces process' max interface
	object_interface_depth
	 .set_status(int)
	object_interface_stability
	 .set_power(int) - sliding scale
	object_interface_response
	 - none

std methods
 get_interface_strength()
 get_interface_capacity() // max strength
 interface_charge(int) - uses int power to charge interface. capped at value determined by interface response

new values
 core:
  int interface_available; // process has at least 1 interface object
  int interface_strength;
  int interface_strength_max;
  int interface_charge_rate;
  timestamp interface_charged;

process:
  int interface_object_present; // can generate interface.
  int interface_depth;
  timestamp interface_raised;
  timestamp interface_broken;


*/

/*

How will data wells work?

- there'll be new block types and things so they can be displayed
- Wells should be far enough apart so that a proc is only ever within scan range of one.

std methods:
 - search_for_well() - returns 1 if there's a well in scan range
 - get_well_x/y() - returns location (absolute) of well in scan range. If no well, returns -1.
 - get_well_data() - returns data level of well in scan range, or -1 if none.

objects:

object_harvest - gathers data from wells, and also can send data to other procs
 - object.gather_data() - tries to gather data from well within scan range. returns data gathered, or -1 if no nearby well. Costs power.
 - object.give_data(process memory index, int data amount) - try to give data to process. returns 0 or 1
instance variables:
 timestamp last_gather_or_give; // set for both gather and give
 int gather_or_give; // what happened at time last_gather_or_give (0 for gather, 1 for give)
 int target_index; // index of data well or target core. for cores, may only be valid a short time after giving occurs (as target may be destroyed)

object_storage - allows process to store data gathered from wells
 - object.get_data_stored() - if called on a class, returns total used storage of class
 - object.get_data_capacity() - similar
  - probably doesn't need any instance variables as its state can be worked out from general core values.
  * these methods could probably be std methods.

object_allocate - converts stored data to usable data. immovable only.
 - object.allocate_data(int amount); // allocates data.
  - not sure how this should work for a class. For now, just allocate the specified amount of data for each object in the class.
 timestamp last_allocate; // last time object operated




new struct fields to support this:

vmstate
 int nearby_well_index; // set to -2 before each execution
    * done

core_struct
 int data_storage_capacity;
 int data_stored;

*/

/*

How will process-to-process communication work?

- will probably not require a special object.

transmit(int process memory index, int channel?, int value0, int value1, int value2, int value3)
 - returns:
  0 if target process unavailable
  -1 if target process rejects channel?
  1 if message received

broadcast(int channel, int value0-3)
 - just like transmit but transmits to all processes in scan range.

- each core will have a message array with up to ?8 messages of up to 4 ints each
- these will be reset at the *end* of each execution.

check_messages() - returns number of messages received
get_message_type(int message index)
 - returns 0 if no message this cycle
 - 1 if transmitted
 - 2 if broadcast
get_message_channel(int message index) - returns channel the message was sent on
ignore_channel(int channel index) - stops accepting messages from channel
unignore_channel(int channel index) - opposite
get_message_value(int message_index, int value_index) - returns contents of message
save_message_source(int message_index, int process memory index) - saves source of message in process memory. Should probably work regardless of whether source is currently in range.






*/


