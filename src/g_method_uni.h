
#ifndef H_G_METHOD_UNI
#define H_G_METHOD_UNI

s16b call_uni_method(struct core_struct* core, int call_value);

enum
{
UMETHOD_CALL_SIN,
UMETHOD_CALL_COS,
UMETHOD_CALL_ATAN2,
UMETHOD_CALL_HYPOT,
//UMETHOD_CALL_HYPOT_LESS,
//UMETHOD_CALL_HYPOT_MORE,
UMETHOD_CALL_WORLD_X,
UMETHOD_CALL_WORLD_Y,
UMETHOD_CALL_ABS,
UMETHOD_CALL_ANGLE_DIFFERENCE,
UMETHOD_CALL_ARC_LENGTH,

UMETHOD_CALL_TYPES
};

struct umethod_call_type_struct
{
	int parameters; // this is the number of parameters that will be pulled off the stack (in addition to the object/class index)
// parameters shouldn't be greater than CMETHOD_CALL_PARAMETERS
 int keyword_index;
};


#endif
