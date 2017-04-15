
#ifndef H_G_METHOD
#define H_G_METHOD

//void active_method_pass_each_tick(void);

//void active_method_pass_after_execution(struct proc_struct* pr);

//s16b call_method(struct proc_struct* pr, struct programstruct* cl, int m, struct methodstruct* methods, int* instr_left);
int call_object_method(struct core_struct* core, int call_value);
int pull_values_from_stack(s16b* stack_parameters, int params);
int call_class_method(struct core_struct* calling_core, int call_value);

void run_objects_before_execution(struct core_struct* core);
void run_objects_after_execution(struct core_struct* core);
void run_objects_each_tick(struct core_struct* core);

enum
{
CALL_MOVE_TO,
CALL_TURN_TO_XY,
CALL_TURN_TO_ANGLE,
CALL_TURN_TO_TARGET,
CALL_TRACK_TARGET,
CALL_APPROACH_XY,
CALL_APPROACH_TARGET,
CALL_APPROACH_TRACK,
CALL_REPOSITION,
CALL_SET_POWER,
CALL_FIRE,
CALL_ROTATE,
CALL_NO_TARGET,
CALL_AIM_AT,
CALL_FIRE_AT,
CALL_INTERCEPT,
//CALL_BUILD_PROCESS, build methods used to be object methods, but that was a mistake
//CALL_BUILD_AS_COMMANDED,
//CALL_BUILD_REPEAT, // repeats the last build command
//CALL_BUILD_RETRY, // like CALL_BUILD_REPEAT but multiple collision failures make it try different locations
CALL_GATHER_DATA,
CALL_GIVE_DATA,
CALL_TAKE_DATA,
CALL_ALLOCATE_DATA,
CALL_FIRE_SPIKE,
CALL_FIRE_SPIKE_AT,
CALL_FIRE_SPIKE_XY,
//CALL_SET_INTERFACE,
CALL_ATTACK_SCAN,
CALL_ATTACK_SCAN_AIM,
CALL_SET_STABILITY,

CALL_TYPES
};

struct call_type_struct
{
	int parameters; // this is the number of parameters that will be pulled off the stack (in addition to the object/class index)
	int keyword_index;
// parameters shouldn't be greater than CALL_PARAMETERS
};






/*
enum
{
METHOD_COST_CAT_NONE,
METHOD_COST_CAT_MIN,
METHOD_COST_CAT_LOW,
METHOD_COST_CAT_MED,
METHOD_COST_CAT_HIGH,
METHOD_COST_CAT_ULTRA,

METHOD_COST_CATEGORIES
};


struct method_costsstruct
{
 int base_cost [METHOD_COST_CATEGORIES];
 int upkeep_cost [METHOD_COST_CATEGORIES];
 int extension_cost [METHOD_COST_CATEGORIES];
 int extension_upkeep_cost [METHOD_COST_CATEGORIES];
};
*/

#endif
