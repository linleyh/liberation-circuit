/*

g_method_core.c

Functions for calls to built-in (core) methods (i.e. ones that don't require an object)


*/

#include <allegro5/allegro.h>

#include <stdio.h>
#include <math.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"

#include "g_world.h"
#include "v_interp.h"
#include "m_maths.h"

#include "g_method.h"
#include "g_method_uni.h"
#include "g_method_misc.h"
#include "c_keywords.h"

extern struct game_struct game;
extern struct vmstate_struct vmstate; // defined in v_interp.c

#define UMETHOD_CALL_PARAMETERS 6



struct umethod_call_type_struct umethod_call_type [UMETHOD_CALL_TYPES] =
{
// {int parameters},
	{2, KEYWORD_UMETHOD_SIN}, // *UMETHOD_CALL_SIN (angle, length)
	{2, KEYWORD_UMETHOD_COS}, // *UMETHOD_CALL_COS (angle, length)
	{2, KEYWORD_UMETHOD_ATAN2}, // *UMETHOD_CALL_ATAN2 (y, x)
	{2, KEYWORD_UMETHOD_HYPOT}, // *UMETHOD_CALL_HYPOT (y, x)
//	{3}, // UMETHOD_CALL_HYPOT_LESS (y, x, compare_value)
//	{3}, // UMETHOD_CALL_HYPOT_MORE (y, x, compare_value)
	{0, KEYWORD_UMETHOD_WORLD_X}, // *UMETHOD_CALL_WORLD_X ()
	{0, KEYWORD_UMETHOD_WORLD_Y}, // *UMETHOD_CALL_WORLD_Y ()
	{1, KEYWORD_UMETHOD_ABS}, // *UMETHOD_CALL_ABS (value)
	{2, KEYWORD_UMETHOD_ANGLE_DIFFERENCE}, // *UMETHOD_CALL_ANGLE_DIFFERENCE (angle1, 2)
	{2, KEYWORD_UMETHOD_ARC_LENGTH}, // *UMETHOD_CALL_ARC_LENGTH (angle1, 2)


};


// returns 1 if okay to continue, 0 if something happened that should cease program execution (not sure this is currently supported)
s16b call_uni_method(struct core_struct* core, int call_value)
{

	s16b stack_parameters [UMETHOD_CALL_PARAMETERS];

	if (call_value < 0
		|| call_value >= UMETHOD_CALL_TYPES)
	{
		if (w.debug_mode)
 		print_method_error("invalid universal method call", 1, call_value);
		return 0;
	}

// If needed, pull the parameters from the stack:
		if (umethod_call_type[call_value].parameters > 0
		&& !pull_values_from_stack(stack_parameters, umethod_call_type[call_value].parameters))
	{
		if (w.debug_mode)
 		print_method_error("universal method call stack error", 0, 0);
		return 0;
	}

 switch(call_value)
 {
	 case UMETHOD_CALL_SIN:
   vmstate.instructions_left -= 4;
	 	return al_fixtoi(fixed_sin(short_angle_to_fixed(stack_parameters [0])) * (int) stack_parameters [1]);
	 case UMETHOD_CALL_COS:
   vmstate.instructions_left -= 4;
	 	return al_fixtoi(fixed_cos(short_angle_to_fixed(stack_parameters [0])) * (int) stack_parameters [1]);
	 case UMETHOD_CALL_ATAN2:
   vmstate.instructions_left -= INSTRUCTION_COST_ATAN2; // expensive operation
	 	return get_angle_int(stack_parameters [0], stack_parameters [1]);
	 case UMETHOD_CALL_HYPOT:
   vmstate.instructions_left -= INSTRUCTION_COST_HYPOT; // expensive operation
	 	return al_fixtoi(distance(al_itofix(stack_parameters [0]), al_itofix(stack_parameters [1])));
/*	 case UMETHOD_CALL_HYPOT_LESS: // avoids the expensive sqrt functions by just comparing squares
				{
					uint64_t	x_dist = stack_parameters [0] * stack_parameters [0];
					uint64_t	y_dist = stack_parameters [1] * stack_parameters [1];
					uint64_t compare_value = stack_parameters [2] * stack_parameters [2];

					if (x_dist + y_dist < compare_value)
						return 1;
					return 0;
				}
	 case UMETHOD_CALL_HYPOT_MORE: // avoids the expensive sqrt functions by just comparing squares
				{
					uint64_t	x_dist = stack_parameters [0] * stack_parameters [0];
					uint64_t	y_dist = stack_parameters [1] * stack_parameters [1];
					uint64_t compare_value = stack_parameters [2] * stack_parameters [2];

					if (x_dist + y_dist > compare_value)
						return 1;
					return 0;
				}*/
	 case UMETHOD_CALL_WORLD_X:
   vmstate.instructions_left -= 2;
			return w.w_pixels;
	 case UMETHOD_CALL_WORLD_Y:
   vmstate.instructions_left -= 2;
			return w.h_pixels;
		case UMETHOD_CALL_ABS:
   vmstate.instructions_left -= 1;
			return abs(stack_parameters [0]);
		case UMETHOD_CALL_ANGLE_DIFFERENCE:
   vmstate.instructions_left -= 4;
			return angle_difference_signed_int(stack_parameters [0], stack_parameters [1]);
		case UMETHOD_CALL_ARC_LENGTH:
   vmstate.instructions_left -= 4;
			return angle_difference_int(stack_parameters [0] & ANGLE_MASK, stack_parameters [1] & ANGLE_MASK);

 }

 return 0;

}


