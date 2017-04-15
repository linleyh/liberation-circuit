
#ifndef H_G_METHOD_CORE
#define H_G_METHOD_CORE

enum
{
CMETHOD_CALL_GET_CORE_X,
CMETHOD_CALL_GET_CORE_Y,
CMETHOD_CALL_GET_PROCESS_X,
CMETHOD_CALL_GET_PROCESS_Y,
CMETHOD_CALL_GET_CORE_ANGLE,
CMETHOD_CALL_GET_CORE_SPIN,
CMETHOD_CALL_GET_CORE_SPEED_X,
CMETHOD_CALL_GET_CORE_SPEED_Y,
CMETHOD_CALL_GET_INTERFACE_STRENGTH,
CMETHOD_CALL_GET_INTERFACE_CAPACITY,
CMETHOD_CALL_GET_USER,
CMETHOD_CALL_GET_TEMPLATE,
CMETHOD_CALL_DISTANCE,
//CMETHOD_CALL_DISTANCE_HYPOT,
CMETHOD_CALL_DISTANCE_LESS,
CMETHOD_CALL_DISTANCE_MORE,
CMETHOD_CALL_TARGET_ANGLE,
CMETHOD_CALL_GET_COMPONENTS,
CMETHOD_CALL_GET_COMPONENTS_MAX,
CMETHOD_CALL_GET_TOTAL_INTEGRITY,
CMETHOD_CALL_GET_TOTAL_INTEGRITY_MAX,
CMETHOD_CALL_GET_UNHARMED_INTEGRITY_MAX,
CMETHOD_CALL_VISIBLE,
CMETHOD_CALL_TARGET_SIGNATURE,

// now need core angle and process angle

CMETHOD_CALL_TYPES
};

struct cmethod_call_type_struct
{
	int parameters; // this is the number of parameters that will be pulled off the stack (in addition to the object/class index)
// parameters shouldn't be greater than CMETHOD_CALL_PARAMETERS
 int keyword_index;
};


enum
{
MMETHOD_CALL_GET_COMPONENT_X,
MMETHOD_CALL_GET_COMPONENT_Y,
MMETHOD_CALL_COMPONENT_EXISTS,
MMETHOD_CALL_GET_INTEGRITY,
MMETHOD_CALL_GET_INTEGRITY_MAX,
MMETHOD_CALL_GET_COMPONENT_HIT,
MMETHOD_CALL_GET_COMPONENT_HIT_SOURCE,

MMETHOD_CALL_TYPES
};

struct mmethod_call_type_struct
{
	int parameters; // this is the number of parameters that will be pulled off the stack (in addition to the object/class index)
// parameters shouldn't be greater than MMETHOD_CALL_PARAMETERS
 int keyword_index;
};

s16b call_self_core_method(struct core_struct* calling_core, int call_value);
s16b call_extern_core_method(struct core_struct* calling_core, int call_value);

s16b call_self_member_method(struct core_struct* calling_core, int call_value);
s16b call_extern_member_method(struct core_struct* calling_core, int call_value);

int verify_target_core(struct core_struct* calling_core, int target_core_index, struct core_struct** target_core);
int verify_friendly_target_core(struct core_struct* calling_core, int target_core_index, struct core_struct** target_core);

#endif
