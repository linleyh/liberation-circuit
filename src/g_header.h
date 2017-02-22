
#ifndef H_G_HEADER
#define H_G_HEADER


// BCODE_MAX is the maximum size of any program type's bcode:
//#define BCODE_MAX 16384
//#define PROC_BCODE_SIZE 4096
//#define CLIENT_BCODE_SIZE 8192
//#define SYSTEM_BCODE_SIZE 16384

#define BCODE_MAX 2048


#define EXECUTION_COUNT 16

#define DRAG_BASE 1014
#define DRAG_BASE_FIXED (al_itofix(DRAG_BASE) / 1024)
//#define SPIN_DRAG_BASE 1015
#define SPIN_DRAG_BASE 1002
#define SPIN_DRAG_BASE_FIXED (al_itofix(SPIN_DRAG_BASE) / 1024)

// during deallocation, process does not exist in world, but can be interacted with in some ways
//  and will not be replaced with another.
// it may be useful for this to be the same as BUBBLE_TOTAL_TIME
#define DEALLOCATE_COUNTER 200

#define MAX_SPEED 4
#define MAX_SPEED_FIXED al_itofix(MAX_SPEED)
#define NEG_MAX_SPEED_FIXED al_itofix(-MAX_SPEED)

#define FORCE_DIST_DIVISOR 16

#define GROUP_MAX_MEMBERS 24
#define PLAYERS 4

// any changes to bcode header structure may need to be reflected in:
//  intercode_process_header() in c_inter.c
//  parse_interface_definition() in c_comp.c
//  derive_proc_properties_from_bcode() in g_proc.c
//  derive_program_properties_from_bcode() in g_client.c


#define FILE_NAME_LENGTH 20
#define FILE_PATH_LENGTH 100
// FILE_NAME_LENGTH is the maximum length of a file name string.


struct bcode_op_struct
{
	int type;


};

struct bcode_struct
{
 s16b op [BCODE_MAX];

 s16b src_line [BCODE_MAX]; // used for debugging

// IMPORTANT - this structure can be copied by assignment in copy_template in template.c!

};

/*
The registerstruct contains both registers and the mbank
The methodstruct contains information about each method available to the proc.
*/
/*
// METHOD_EXTENSIONS is the number of types of extensions each method can have
// redefining METHOD_EXTENSIONS requires numerous other changes to be made.
#define METHOD_EXTENSIONS 3

// this structure holds information about the various methods. See g_method.c
struct mtypestruct
{
 char name [30];
 int mclass; // MCLASS_PR etc
 int mbank_size; // this is the amount of space the method takes up in the mbank (in individual addresses)
 int cost_category;
 int external;
// int base_data_cost;
// int base_upkeep_cost; // fixed cost per cycle (not per tick)
 int extension_types; // different types of extension (currently max of 3)
 int max_extensions; // maximum number of extensions (should be 0 if method can't have extensions)
 char extension_name [METHOD_EXTENSIONS] [10];
// int extension_data_cost;
// int extension_upkeep_cost; // fixed cost per cycle per extension
};
*/
#define OBJECT_NAME_LENGTH 24

enum
{
OBJECT_BASE_TYPE_NONE,
OBJECT_BASE_TYPE_LINK,
OBJECT_BASE_TYPE_STD,
OBJECT_BASE_TYPE_MOVE,
OBJECT_BASE_TYPE_ATTACK,
OBJECT_BASE_TYPE_DEFEND,
//OBJECT_BASE_TYPE_BUILD,
OBJECT_BASE_TYPE_MISC,

};


enum
{
// The attack_type determines:
//  - which object methods can be called on this object
//  - which object_instance values (in the object_instance unions) are relevant
ATTACK_TYPE_NONE,
ATTACK_TYPE_PULSE, // uses same approach as pulse (i.e. rotatable attack); used also for e.g. stream_dir
ATTACK_TYPE_BURST, // fixed angle; otherwise same as pulse. E.g. stream as well
ATTACK_TYPE_SPIKE, // fixed angle, but can set firing angle directly

ATTACK_TYPES
};

struct object_details_struct
{
 int only_zero_angle_offset; // 1 if object can only point directly away from component centre
 int attack_type; // ATTACK_TYPE enum
 int packet_speed; // a default value is used for this for non-attacking objects (so that the intercept method won't fail if called with them)
 int power_cost;
 int recycle_time;
 int packet_size;
 int damage;
 int rotate_speed; // pulse etc. object turn rate when aiming
};

struct object_type_struct
{
	char name [OBJECT_NAME_LENGTH];
	int keyword_index; // index of the identifier for the object's name
	int object_base_type;
	int unlock_index;
	int data_cost;
	int power_use_peak; // used in design analysis. Maximum power use in a single cycle
//	int power_use_smoothed; // used in design analysis. Power use averaged out over recycle time.
	int power_use_base; // used in design analysis. Only counts objects likely to be used constantly (mainly move and interface)
	 // the power_use values are a bit complicated to work out - they need to take many different things into account
 struct object_details_struct object_details; // this is a separate struct because it's mostly irrelevant for non-attacking objects
// int intercept_speed; // intercept speed for attacking objects. Has a default value for all other kinds of object.
// int attack_object_index; // index in the attack_object struct. Is 0 for anything not an attack object.
// int power_use; // activation cost - is only used for certain types of objects (mostly attacking objects)
// int recycle_time; // similar to power_use, is only relevant for certain types of object

};

#define POWER_COST_STREAM 160
#define POWER_COST_SPIKE 40
#define SPIKE_BASE_DAMAGE 32
#define SPIKE_STAGE2_DAMAGE 48
#define SPIKE_MAX_DAMAGE 80

#define HARVEST_RATE 4
#define POWER_COST_GATHER_BASE 40
//#define POWER_COST_GATHER_1_DATA 5
#define POWER_COST_GIVE_BASE 20
//#define POWER_COST_GIVE_1_DATA 1

#define ALLOCATE_RATE 3
#define POWER_COST_ALLOCATE_1_DATA 10

// These stream values are used in both firing method call and display function for stream cloud
#define STREAM_WARMUP_LENGTH 32
#define STREAM_FIRING_TIME 32
#define STREAM_COOLDOWN_LENGTH 16

#define STREAM_TOTAL_FIRING_TIME (STREAM_WARMUP_LENGTH+STREAM_FIRING_TIME+STREAM_COOLDOWN_LENGTH)

// step length is used for collision detection and a bit of display stuff
#define STREAM_STEP_PIXELS 5
#define STREAM_FIX_STEP_PIXELS al_itofix(STREAM_STEP_PIXELS)

// slice works a bit differently
//#define SLICE_WARMUP_TIME 16
#define SLICE_FIRING_TIME 8
#define SLICE_RECYCLE_TIME 64
#define POWER_COST_SLICE 70
#define SLICE_TOTAL_FIRING_TIME (SLICE_FIRING_TIME)


#define POWER_COST_REPAIR_1_INTEGRITY 16
#define POWER_COST_RESTORE_COMPONENT 24
// the restore component cost is fairly low because it's applied during the entire recycle period

// STREAM_RECYCLE_TIME is counted from the start of firing, not the end, and so must be more than total of stream phase times
#define STREAM_RECYCLE_TIME 128

#define HARVEST_RECYCLE_TIME 64
// currently this applies to both gather and give (although give transfers much more data each time)

enum // this isn't used in mbanks (it's used in data and for the stream beam cloud)
{
STREAM_STATUS_INACTIVE,
STREAM_STATUS_WARMUP,
STREAM_STATUS_FIRING,
STREAM_STATUS_COOLDOWN
}; // also used for DSTREAM


// GRAIN is the bitshift to go between pixels and x/y values in the coordinate system
//#define GRAIN 10
//#define GRAIN_MULTIPLY 1024

// size of block as an exponent of 2:
//#define BLOCK_SIZE_BITSHIFT 17
// a bitshift of 16 = a block size of 64 pixels or 2^16 GRAIN units (must be at least equal to largest possible size of a proc in any direction)
// Remember that the 16 incorporates the GRAIN bitshift of 10
//#define BLOCK_SIZE (1<<BLOCK_SIZE_BITSHIFT)
//#define BLOCK_SIZE_PIXELS (1<<(BLOCK_SIZE_BITSHIFT-GRAIN))
#define BLOCK_SIZE_PIXELS 128
#define BLOCK_SIZE_FIXED al_itofix(128)

// STUCK_DISPLACE_MAX is the maximum distance a proc that's stuck against another proc will be displaced in an effort to separate them. Must be smaller than a block
#define STUCK_DISPLACE_MAX al_itofix(120)


#define GROUP_CONNECTIONS 7

#define COMMANDS 16
#define TEAMS 8


struct cart_struct
{
	al_fixed x, y;
};
typedef struct cart_struct cart;

struct polar_struct
{
	al_fixed angle, magnitude;
};
typedef struct polar_struct polar;

struct block_cart_struct
{
	int x, y;
};
typedef struct block_cart_struct block_cart;



enum
{
/*
TCOL_FILL_BASE, // underlying shape colour
TCOL_MAIN_EDGE, // edge of process and method base
TCOL_METHOD_EDGE, // edge of method overlays
*/
TCOL_MAP_POINT, // colour of this team's procs on the map
TCOL_MAP_POINT_FAINT, // edge of large proc on map
TCOL_MAP_POINT_THICK, // edge of large proc on map
/*
TCOL_BOX_FILL,
TCOL_BOX_HEADER_FILL,
TCOL_BOX_OUTLINE,
TCOL_BOX_TEXT,
TCOL_BOX_TEXT_FAINT,
TCOL_BOX_TEXT_BOLD,
TCOL_BOX_BUTTON,
*/
TCOLS
};


enum
{
// return values for a build object call
BUILD_SUCCESS = 1,
BUILD_FAIL_NO_BUILD_OBJECTS = 0, //
BUILD_FAIL_TEMPLATE_INACTIVE = -1, // template index valid but not active
BUILD_FAIL_TEMPLATE_INVALID = -2, // template index invalid
BUILD_FAIL_TEMPLATE_ERROR = -3, // something wrong with template (may not be used currently)
BUILD_FAIL_TEMPLATE_NOT_LOCKED = -4,
BUILD_FAIL_TOO_MANY_CORES = -5,
BUILD_FAIL_TOO_MANY_PROCS = -6,
BUILD_FAIL_COLLISION = -7,
BUILD_FAIL_NOT_READY = -8, // build object in cooldown
BUILD_FAIL_OUT_OF_BOUNDS = -9, // a component is out of the game area
BUILD_FAIL_OUT_OF_RANGE = -10, // build location too far from builder
BUILD_FAIL_DATA = -11,
BUILD_FAIL_STATIC_NEAR_WELL = -12, // static core too close to data well
BUILD_FAIL_POWER = -13, // not enough power to operate build objects
//BUILD_FAIL_LOCAL_CONDITIONS = -14, // currently this is just an attempt to build an allocator in LOCAL_CONDITION_SINGLE_ALLOCATOR
};

struct build_queue_struct
{
// list is terminated by entry with active = 0
 int active;
	int core_index; // index of builder - note no timestamp, as anything from a destroyed builder should be removed from the queue
//	timestamp core_timestamp; // timestamp of builder
	int template_index;
	int build_x, build_y;
	int angle;
//	int target_index;
	int repeat;
// note that this struct is copied by assignment in build_queue_button_pressed()	in g_command.c (and maybe elsewhere)
};


#define PLAYER_NAME_LENGTH 13

struct player_struct
{

// REMEMBER: When anything is added to this structure, it may need to be added to load/save routines in f_load.c/f_save.c

 int active; // 1 if player is in game
 char name [PLAYER_NAME_LENGTH];

 int core_index_start; // where this team starts in the proc array
 int core_index_end; // all procs on this team should be below this (the maximum is one less)
 int proc_index_start; // where this team starts in the proc array
 int proc_index_end; // all procs on this team should be below this (the maximum is one less)
// int template_proc_index_start; // index in templ[] array of this player's first proc template (used when creating new proc from template using PR_NEW method). Is -1 if no proc templates available.

/*
 int colour; // index in the TEAM_COLOUR array (TEAM_COLOUR_GREEN etc.)
 int packet_colour; // CLOUD_COL_xxx index
 int drive_colour; // CLOUD_COL_xxx index*/

// these colour values store colour information. They're set to defaults at startup but can
//  be changed by the MT_OB_VIEW method. The values in these variables are used by functions
//  in i_disp_in.c to set up the actual colour arrays (which are of type ALLEGRO_COLOR).
// min and max are supposed to be min and max intensity, but min can actually be == max or > max
//  if a particular colour component is to be left the same or reduced as intensity increases.
// currently alpha depends on intensity and can't be set by the VIEW method.
 int proc_colour_min [3]; // used for procs as well as various other display things
 int proc_colour_max [3];
 int packet_colour_min [3];
 int packet_colour_max [3];
 int drive_colour_min [3];
 int drive_colour_max [3];
 int interface_colour_base [3];
 int interface_colour_charge [3];
 int interface_colour_hit [3];

// struct programstruct client_program; // client.active will be 0 if no client program for this player
// int program_type_allowed; // one of the PLAYER_PROGRAM_ALLOWED enums (basically, operator or delegate)

// int allocate_effect_type;
// struct templstruct templ [TEMPLATES];

 int score;
 int processes; // number of procs (may not be terribly accurate - main use is for display to user)
 int components_current; // number of components (may not be terribly accurate - main use is for display to user)
 int components_reserved; // includes destroyed components of existing cores

 int output_console; // the worldstruct print values are set to these before execution of any program controlled by this player (incl cl, op and proc)
 int output2_console; // the worldstruct print values are set to these before execution of any program controlled by this player (incl cl, op and proc)
 int error_console; // errors for this player are sent to this console
// int default_print_colour;
// int force_console; // can be set by OB_CONSOLE method; forces this player's programs/procs to use this console. If -1, player is muted.

 int data;

 unsigned int random_seed; // used for the random() smethod

#define BUILD_QUEUE_LENGTH 12

 struct build_queue_struct build_queue [BUILD_QUEUE_LENGTH];

 int build_queue_fail_reason; // a BUILD_FAIL enum. BUILD_SUCCESS means no failure.


// variables used for irpt limit:
// int gen_number;

 cart spawn_position; // a process will be spawned here at start of game.
 int spawn_angle; // angle process will spawn at

};

#define INSTRUCTIONS_PROC 2048

// these are special costs for expensive operations.
// they may need to be a bit higher...
#define INSTRUCTION_COST_HYPOT 16
#define INSTRUCTION_COST_ATAN2 16


#define MAX_LINKS 7
// Remember that 4 links requires 5 group_connection elements because core can't have connection 0
#define MAX_OBJECTS 6
#define CLASSES_PER_OBJECT 4

// these are the base ranges of various functions. All ranges extend to a square (with sides 2x the range).
// ranges are generally calculated from the core, even if the thing is being done by an object on another component.
#define SCAN_RANGE_BASE_BLOCKS 7
#define SCAN_RANGE_BASE_PIXELS (SCAN_RANGE_BASE_BLOCKS*BLOCK_SIZE_PIXELS)
#define SCAN_RANGE_BASE_FIXED (al_itofix(SCAN_RANGE_BASE_PIXELS))

#define BUILD_RANGE_BASE_BLOCKS 7
#define BUILD_RANGE_BASE_PIXELS (BUILD_RANGE_BASE_BLOCKS*BLOCK_SIZE_PIXELS)
#define BUILD_RANGE_BASE_FIXED (al_itofix(BUILD_RANGE_BASE_PIXELS))

//#define BROADCAST_RANGE_BASE_BLOCKS 8
//#define BROADCAST_RANGE_BASE_PIXELS (8*BLOCK_SIZE_PIXELS)


//#define STANDARD_RANGE_SCAN 800
//#define STANDARD_RANGE_BUILD 800

enum
{
TEMPLATE_OBJECT_ERROR_NONE,
TEMPLATE_OBJECT_ERROR_MOVE_OBSTRUCTED,
//TEMPLATE_OBJECT_ERROR_MOVE_INTERFACE, // both move and interface objects on same component

//TEMPLATE_OBJECT_ERROR_INTERFACE_CORE, // interface object on core
TEMPLATE_OBJECT_ERROR_STATIC_MOVE, // static core has move object
TEMPLATE_OBJECT_ERROR_MOBILE_ALLOCATE, // non-static core has allocate object
TEMPLATE_OBJECT_ERROR_STORY_LOCK, // in story mode, and object is not yet unlocked

TEMPLATE_OBJECT_ERRORS
};


struct object_struct
{
// This struct contains basic properties present on both processes and templates.
// There's no real reason not to let them change while on a process, although specifically mutable things are generally in object_instance structs
	int type;
	al_fixed base_angle_offset; // offset from 0 (0 is the link's angle)
	int base_angle_offset_angle; // angle_offset in int format
	int object_class [CLASSES_PER_OBJECT];

	int next_similar_object_member;
	int next_similar_object_link;
// some object types use this as a linked list - e.g. build and repair objects. First object information is held in core_struct for these kinds of objects. -1 if last in list.
// use the template's object_struct version of this as the proc versions may point to destroyed members. destroyed member entries in the template object_struct should be ignored.

	int template_error; // is non-zero (TEMPLATE_ERROR_x) if there is some kind of error in the template (e.g. a move object is obstructed). Errors are specific to particular object types (most objects can't have errors)

// IMPORTANT - this structure can be copied by assignment in copy_template in template.c!

// When changing properties, may need to update the link object init/creation/deletion code in d_design.c
};

struct object_instance_struct
{
// this struct contains properties that are relevant to an object on an actual process, but not an object in a template

	al_fixed angle_offset; // offset from 0 (0 is the link's angle)
	int angle_offset_angle; // angle_offset in int format

// if the object is using power between cycles without being called during execution
//  (e.g. a pulse object in recycle, which uses power while recycling)
// these values will be set
	int ongoing_power_cost; // cost each cycle - applied just before execution (if w.world_time < ongoing_power_cost_finish)
	timestamp ongoing_power_cost_finish_time;

	union
	{
		int int_value1; // generic reference used for saving/loading etc
#define MOVE_POWER_MAX 10
		int move_power; // move objects
//		timestamp burst_fire_timestamp; // timestamp when burst should fire. just use packet_fire_timestamp
	 timestamp last_gather_or_give; // harvest object: set for both gather and give
	 timestamp last_allocate; // allocate object: when last operated.
//	 timestamp last_build; // build object: when last operated.

//	 int stream_fire; // 1 if stream is set to fire at the next opportunity
		timestamp attack_fire_timestamp; // timestamp when pulse/burst/stream/etc should fire.
   // this is a request and doesn't guarantee that the object will actually fire.

//		timestamp spike_fire_timestamp; // same as packet_fire_timestamp

//	 int interface_object_active; // 1 if interface is active, 0 otherwise (not currently used - proc's interface status determined from proc->interface_on_process_set_on and core->interface_active)
	};
	union
	{
		int int_value2;
		int move_power_last_cycle; // move objects
		//timestamp packet_last_fired; // when packet was last fired.
//		timestamp burst_last_fired; // when burst was last fired. - just use packet_last_fired
//		timestamp spike_last_fired; // when spike was last fired.
//		timestamp stream_last_fired; // when stream was last fired.
  int gather_or_give; // harvest object: what happened at time last_gather_or_give (0 for gather, 1 for give)
		timestamp first_unbroken_allocate; // allocate object: start of current unbroken string of allocates
//		timestamp build_cooldown; // time when build object will work again - this has been generalised to a single countdown per process in core_struct

		timestamp attack_last_fire_timestamp; // timestamp when pulse/burst/stream/etc last fired.

	};
	union
	{
		int int_value3;
//		int move_power_this_cycle; // move objects - this is the move_power that applied during the current cycle. May differ from move_power during execution if the program has updated move_power.
//  int gather_target_index; // harvest object: index of data well or target core. for cores, may only be valid a short time after giving occurs (as target may be destroyed)
  timestamp second_last_gather_or_give;

		timestamp attack_recycle_timestamp; // timestamp when pulse/burst/stream/etc can fire again.
	};
	union
	{
		al_fixed fixed_value1;
		al_fixed move_spin_change; // spin change per tick per unit power (can be positive or negative). Needs to be recalculated each time group's mass distribution changes.
		al_fixed rotate_to_angle_offset; // target angle offset of rotating object (it will move towards this angle)
  al_fixed spike_angle_offset;
	};
	union
	{
	 al_fixed fixed_value2;
	 al_fixed move_accel_angle_offset; // acceleration angle as an offset from group angle
	};
	union
	{
	 al_fixed fixed_value3;
	 al_fixed move_accel_rate; // acceleration per unit power.
	};
// NOTE: cannot assume any of these values are initialised to zero at process creation!
//  See set_group_object_properties() in g_proc_new.c

// IMPORTANT - this structure can be copied by assignment in copy_template in template.c!
};


struct template_connection_struct
{
	int template_member_index; // -1 if no connection
	int link_index; // link of this proc that is connected to other proc.
 int reverse_connection_index; // index of connection in other proc's connection structure.
 int reverse_link_index; // index of other proc's link back to this proc.

// IMPORTANT - this structure can be copied by assignment in copy_template in template.c!
};
typedef struct template_connection_struct template_connection_struct;

struct template_member_struct
{
 int exists;
 int shape;
 al_fixed group_angle_offset; // offset from pointing directly forwards.
 al_fixed connection_angle_offset; // offset from the angle the proc would have if two vertices just connected directly
 int connection_angle_offset_angle; // offset in angle units. connection_angle_offset should be derived from this. Is basic group angle for core.
// template_object_struct object [MAX_LINKS]; // index is the link the object is on.
 struct object_struct object [MAX_LINKS]; // this is the same as the process object struct
 template_connection_struct connection [GROUP_CONNECTIONS];
 cart position; // x/y position - derived from position in group, which needs to be determined recursively.

 al_fixed approximate_angle_offset; // angle offset from group angle - used for approximate purposes like drawing build indicator
 al_fixed approximate_distance; // distance from core centre - also approximate

 int collision; // is 1 if the member is colliding with another member.
 int move_obstruction; // is 1 if the member is obstructing another member's move object.
 int story_lock_failure; // is 1 if the member shape type not unlocked, and we're in story mode.

#define MAX_DOWNLINKS_FROM_CORE 6
// MAX_DOWNLINKS_FROM_CORE prevents long strings of components that can cause various problems. 6 should be plenty
 int downlinks_from_core; // 0 for core, 1 for procs connected to core, etc

 int data_cost; // cost of component + objects. Used to work out recycle for restore methods.
 int mass; // currently just data_cost * 10

 int interface_can_protect; // is 1 if this member has no objects (move) that prevent interface (but may be 1 even if the process as a whole has no interface)

// IMPORTANT - this structure can be copied by assignment in copy_template in template.c!
//   so be careful using pointers!
};
typedef struct template_member_struct template_member_struct;

#define TEMPLATE_BUTTON_TITLE_STRING_LENGTH 32
#define TEMPLATE_NAME_LENGTH 16

#define OBJECT_CLASSES 16
#define CLASS_NAME_LENGTH 16
#define OBJECT_CLASS_SIZE 16


struct template_struct
{

 int active; // 0/1 inactive/active
 int locked; // 0 if can be edited, 1 if not
 int modified; // 1 if has been changed since the design was last written to, or generated from, source
 int player_index; // which player it belongs to
 int template_index; // index in player's array of templates

 int mission_template; // is 1 if this is an enemy process template from a mission. Prevents various forms of modification and saving.

 char object_class_name [OBJECT_CLASSES] [CLASS_NAME_LENGTH];
 int object_class_active [OBJECT_CLASSES];
// note that the object_class_member and _object arrays are not initialised until the template is fixed.
//  (the classes field in the object struct is used instead)
// this means that they can't be relied on during the design phase.
 int object_class_member [OBJECT_CLASSES] [OBJECT_CLASS_SIZE];
 int object_class_object [OBJECT_CLASSES] [OBJECT_CLASS_SIZE];

// int members;
 struct template_member_struct member [GROUP_MAX_MEMBERS]; // member [0] is core

 int esource_index; // is this used?
 struct source_edit_struct* source_edit; // pointer to source_edit[esource_index] in the editor structure
 struct bcode_struct bcode; // contains bitcode
// struct source_struct source;

 char menu_button_title [TEMPLATE_BUTTON_TITLE_STRING_LENGTH];
 char name [TEMPLATE_NAME_LENGTH];
// char src_file_name [FILE_NAME_LENGTH];

 int data_cost; // updated by calculate_template_cost_and_power in d_design.c. may not be kept up-to-date at all times, so may need to call calculate_template_cost before using.
 int total_mass; // same as for data_cost
 int mobile; // determined by core type
 int build_cooldown_cycles; // currently same as data_cost

 int power_capacity;
 int power_use_peak; // power_use values are just indicators used in process design. actual power use is calculated in more detail
// int power_use_smoothed;
 int power_use_base;

 int first_build_object_member;
 int first_build_object_link;
 int first_repair_object_member;
 int first_repair_object_link;
 int has_allocator; // 1 if has an allocator. used for LOCAL_CONDITION_SINGLE_ALLOCATOR

 int number_of_interface_objects; // currently this should only be used for d_draw display code. It's recalculated for individual cores when created (and when damaged etc.)
 int number_of_storage_objects; // like number_of_interface_objects

// *** IMPORTANT!!
// When adding any fields to this struct, may also need to add them to copy_template in template.c!


};
//typedef struct template_struct template;


#define MESSAGES 8
// MESSAGES is the number of messages a process can receive each cyle. If more are received, priority 1 messages replace priority 0. Messages that don't fit are left out.
#define MESSAGE_LENGTH 8
// MESSAGE_LENGTH is max number of ints in each message
//#define BROADCAST_RANGE SCAN_RANGE_BASE_PIXELS
// BROADCAST_RANGE should probably match default scanning range
#define CHANNELS 8
// processes can ignore channels they don't need to listen to

enum
{
MESSAGE_TYPE_NONE,
MESSAGE_TYPE_TRANSMIT,
MESSAGE_TYPE_TRANSMIT_TARGET,
MESSAGE_TYPE_BROADCAST,
MESSAGE_TYPE_BROADCAST_TARGET,
};

struct message_struct
{
 int type; // MESSAGE_TYPE enum
 int source_index; // index of sending core
 timestamp source_index_timestamp; // creation time of sending core
 cart source_position; // position when message was sent (absolute)
 int channel; // channel message was sent on

 int priority; // 0 or 1 (could have more priorities but that would require annoying sorting)
 int length; // number of values in message data (up to MESSAGE_LENGTH)

 int target_core_index; // index of target core if transmit/broadcast_target
 timestamp target_core_created_timestamp;

 s16b data [MESSAGE_LENGTH]; // contents of message

};


struct command_queue_struct
{
 int type;
 int new_command; // is 1 if command hasn't been read by process yet
 int x, y; // this is the target location of the command. For targetted commands, it's the target location when the command is issued by the user
  //  (although autocoded processes use process methods to find the target)
  // also, x is used for value of number command (not currently implemented)
 timestamp command_time; // game.total_time when command was given
 int target_core; // proc index - not directly available
 timestamp target_core_created;
 int target_member; // index of target member of target core (in core's group_member array) - probably should be directly available
 int control_pressed; // was control pressed when the command was given?

// when adding fields to this struct, remember to add them to CMETHOD_CALL_CLEAR_COMMAND code in g_method_core.c
// also note that this struct may be copied by assignment, so be careful adding pointers!

};
#define COMMAND_QUEUE 4
/*
struct build_command_queue_struct
{

 int active; // 0 if no command
// timestamp build_command_timestamp; // time when most recent command was given. not sure this is needed.
 int build_template;
 int build_x, build_y; // translate from fixed to int when given
 int build_angle; // same
 int build_command_ctrl; // 1 if user pressed control while issuing the build command

// structs of this type may be copied by struct assignment, so avoid using pointers!

};
#define BUILD_COMMAND_QUEUE 4
*/
enum
{
COM_NONE,
COM_LOCATION,
COM_TARGET, // clicked on enemy process
COM_FRIEND, // clicked on friendly process
COM_DATA_WELL, // clicked on data well
//COM_NUMBER // user pressed a number key

COM_TYPES

// when adding a new command type, need to make sure the command-based methods (see g_method_std.c) work properly with it
};

enum
{
ATTACK_MODE_ALL,
ATTACK_MODE_FIRE_1, // some code in g_method.c assumes that fire_1, 2 and 3 will have the values 1, 2 and 3
ATTACK_MODE_FIRE_2,
ATTACK_MODE_FIRE_3,
//ATTACK_MODE_SPARE_POWER, - not currently supported - requires too many calculations for each individual object.
//ATTACK_MODE_ALL_POWER,
ATTACK_MODES // note that core->attack_mode isn't bounds-checked; invalid values should be treated as 0
};
/*
enum
{
STRESS_LOW,
STRESS_MODERATE,
STRESS_HIGH,
STRESS_EXTREME, // should probably be fatal
STRESS_LEVELS
};*/

struct group_member_struct
{
 int exists; // is 0 if this component has been destroyed.
 int index; // process index. w.proc array entries for destroyed components are reserved for future restoration.
 polar position_offset; // location of the group_member relative to the core (must be adjusted for core's angle)
 al_fixed angle_offset; // offset of process' own angle from core's angle
};
typedef struct group_member_struct group_member_struct;

#define MEMORY_SIZE 512
#define PROCESS_MEMORY_SIZE 64

struct core_struct
{

 int exists; // is 1 if proc exists, 0 if proc doesn't exist, -1 if proc doesn't exist and the deallocating counter hasn't counted down yet
	timestamp created_timestamp;
	timestamp destroyed_timestamp;
 int index; // this is the index in the w.core array (which is invisible to the player)
 int process_index; // this is the process occupied by the core.
 int player_index;
 int template_index; // can be used with player_index to work out the template

 timestamp construction_complete_timestamp;

// PROBABLY NEED deallocation counter so that core is deallocated along with core process.
//  otherwise process could exist without a core, or with a subsequently created core

 s16b memory [MEMORY_SIZE];
 int process_memory [PROCESS_MEMORY_SIZE];
 timestamp process_memory_timestamp  [PROCESS_MEMORY_SIZE];

 int power_capacity; // total capacity - determined by core type and maybe by objects?
 int power_left; // amount left to use this cycle. Can be negative in some unusual circumstances.
 int power_use_excess; // amount of power that objects tried to use but couldn't because there wasn't enough. Not super-accurate and really just used for the power use record display for the selected process.
// int power_use_predicted; // guess of how much power used in addition to power_used - includes power that will probably be used by objects like packet and move (which use power after execution), but may be inaccurate
// int power_used_old; // previous cycle's power use
 int instructions_per_cycle; // based on core type
 int instructions_used; // this is updated from the virtual machine state at the end of execution
// int stress;
//#define STRESS_REDUCTION_FACTOR 4
// STRESS_REDUCTION_FACTOR determines how efficient unused power is at reducing stress. Power is divided by this factor (so 2 means 1 stress requires 2 power to remove)
// int stress_level; // STRESS_LOW etc. Worked out by dividing stress by power_capacity. Is probably just worked out once per cycle
 int attack_mode;

 int group_total_hp; // total hp of all components in group.
 int group_total_hp_max_current; // max hp of all existing components (ignores destroyed components)
 int group_total_hp_max_undamaged; // max hp of all components. Could be stored in template_struct as it will be the same for all processes from the same template.

 int contact_core_index; // just stores the most recent core to collide with this one. will be -1 if no contact this cycle
 timestamp contact_core_timestamp;
 int damage_this_cycle; // total damage to group received this cycle (isn't reduced by any repair amounts)
 int damage_source_core_index; // like contact_core_index
 timestamp damage_source_core_timestamp;

// movement etc commands
 struct command_queue_struct command_queue [COMMAND_QUEUE];
 int new_command; // is 1 if there's a new command

// build commands (one only; no queue)
// int new_build_command; // is set to 1 when command given, then to 0 when checked. (is set to 2 if shift being pressed when user clicked build button)
// timestamp build_command_timestamp; // time when most recent command was given. not sure this is needed.
// struct build_command_queue_struct build_command_queue [BUILD_COMMAND_QUEUE];
/* int build_command_template;
 int build_command_x, build_command_y; // translate from fixed to int when given
 int build_command_angle; // same
 int build_command_ctrl; // 1 if user pressed control while issuing the build command
*/
// the following are used for the repeat/retry build call

 int rebuild_template;
 al_fixed rebuild_x, rebuild_y;
 int rebuild_angle;
 int retry_build_collision_count; // number of times the core has tried to build something but failed because of a collision.

 int number_of_build_objects; // is updated whenever process created or composition changes. Used to work out build cooldowns as well as whether to open the build commands when selected.
 timestamp build_cooldown_time; // time when build objects become available again. Applies to all build objects (but multiples reduce it)
 timestamp last_build_time; // timestamp for most recent repair/restore call
 int first_build_object_member; // member on which first build object is found. -1 if none. Start of a linked list that continues in the object struct.
 int first_build_object_link; // link index. May not be -1 if none (use the member value instead)

 int number_of_repair_objects; // similar to number_of_build_objects. Counts both object_repair and object_repair_other.
 int has_repair_other_object; // set to 1 if at least one of the process' repair objects are repair_other (a single repair_other object allows all repair objects to be used on other procs)
 timestamp restore_cooldown_time; // time when repair objects can restore again. Doesn't stop them repairing. Reduced by multiple objects.
 timestamp last_repair_restore_time; // timestamp for most recent repair/restore call
 int first_repair_object_member;
 int first_repair_object_link;

// int number_of_interface_depth_objects; // used to work out maximum recharge speed
 int number_of_interface_objects; // used to work out maximum recharge speed
 int interface_max_charge_rate; // based on number_of_interface_objects and core type
 int interface_charge_rate_per_object; // based on core shape

 int number_of_harvest_objects; // used to work out whether the process can detect data wells (build objects also count)

 struct message_struct message [MESSAGES]; // note that this is not initialised - it relies on messages_received to ignore uninitialised parts
 int messages_received;
 int message_reading; // index of current message being read by process. Is -1 if next_message() not yet called.
  // if message_reading is >= core->messages_received, core has finished reading messages
 int message_position; // position in current message
 int listen_channel [CHANNELS];

 al_fixed scan_range_fixed; // range (in pixels) of the core's scanner. scan covers a square extending this number of pixels from the core.
 float scan_range_float; // used only for display functions

// int execution_count; // countdown to next code execution for this core
 timestamp last_execution_timestamp; // last time the core executed.
 timestamp next_execution_timestamp; // next time the core will execute
 int cycles_executed; // total number of times it's executed
 int selected;
 timestamp select_time;
 timestamp deselect_time;

// code-related data:
// struct pcodestruct bcode;
// struct registerstruct regs;
// struct methodstruct method [METHODS + 1]; // has an extra element for an MTYPE_END at the end
// int execution_count; // countdown to next code execution for this proc
// int active_method_list_permanent [METHODS]; // this is a linked list of active methods, built on proc creation.
// int active_method_list [METHODS]; // this is a linked list of active methods currently doing something, built after each execution and modified
// int active_method_list_back [METHODS]; // backwards links corresponding to active_method_list, to make it a double linked list
 // although the active_method_list arrays have METHODS elements, this is just to ensure sufficient space - the elements don't necessarily line up with the method struct array.


// int irpt; // needs to be changed anyway
// int irpt_max;


// * these should be in template:
// int base_process; // process from which the core and any subprocesses are created.
// int base_process_vertex;


 int mobile; // 1 if core can move, 0 otherwise (is 0 if the core's proc or at least one member of group has mobile==0)
  // should be correct whether or not core is in a group


// probably store groupstruct in core_struct?

 polar core_offset_from_group_centre;
  // This is the offset of the core process from the group's centre of mass. The position of all other group members can be worked out from this.
  // When used it may need to be adjusted by group_angle.
  // The core process' angle offset (from the group angle) is stored in its group_member struct.

 int group_members_max; // number of members of group, including core, if group undamaged.
 int group_members_current; // like group_members_max but is reduced if components are destroyed. Don't use this for loops that need to check all current members, as there may be gaps in the members array.
 int group_mass;
 int group_mass_for_collision_comparison; // same as group_mass, except that if group is static is greatly increased (so that mobile processes bounce off static ones)
 int group_moment; // moment of inertia - see calculate_group_moment_of_inertia() in g_group.c.

 group_member_struct group_member [GROUP_MAX_MEMBERS];

// x, y values: for mobile groups, are the group's centre of mass. For groups with roots (immobile), is the first member
 cart group_centre_of_mass;
 cart group_test_centre_of_mass;
 int centre_of_mass_test_x, centre_of_mass_test_y; // used for calculating centre of mass (which is likely to cause an al_fixed value to overflow)
 cart group_speed;
// al_fixed group_drag; // is equal to the lowest (i.e. most draggy) drag of all group members
 al_fixed group_angle, group_test_angle;
// int angle_angle; // angle in angle units
 al_fixed group_spin;
// al_fixed group_spin_change; // this value is set to zero each tick then accumulates changes to spin that need to be precise (e.g. engine thrust where there might be counterbalanced engines), and is finally added back to spin
 cart core_position; // position of member 0. This is used for things like scan ranges.

 int group_hit_edge_this_cycle;

 al_fixed constant_accel_angle_offset;
 al_fixed constant_accel_rate;
 al_fixed constant_spin_change;

 int interface_available; // core/group has at least 1 interface object
 int interface_active; // interface currently on (although it may be switched off for individual processes. I think)
 int interface_control_status; // core program can change this. If 0, interface is inactive (and interface_active should be 0) even if the interface could be active. Defaults to 1.
 int interface_strength;
 int interface_strength_max;
// int interface_charged_this_cycle; // how much the interface has been charged so far this cycle
//#define INTERFACE_CHARGE_RATE_BASE 8
//#define INTERFACE_CHARGE_RATE_RESPONSE 6
// BASE is normal charge rate. RESPONSE is additional charge rate per INTERFACE_RESPONSE object
 // * response not currently used
// int interface_charge_rate; - not currently used
 timestamp interface_charged_time;
 timestamp interface_broken_time;
#define INTERFACE_BROKEN_TIMER 300
// broken interface can't be raised for 300 ticks (but can be charged)

//#define INTERFACE_CHARGE_PER_OBJECT 4
// maximum charge per cycle per object
#define INTERFACE_CHARGE_POWER_COST 10
// power cost per charge (amount charged depends on core type)
//per point charged <= no
//#define INTERFACE_STRENGTH_PER_OBJECT 96 - this is now based on component hp

// strength per interface_depth object

#define BUILD_POWER_COST 40

 int data_storage_capacity;
 int data_stored;

// visibility_checked values are checked each time a core uses a method that refers to another core.
// the other core's values are set, so that if the core refers to the same core again in the same execution,
//  visibility doesn't need to be recalculated.
 int visibility_checked_by_core_index; // index of checking core. -1 if not yet checked.
 int visibility_check_result; // result of check (1 or 0, or can be -1 if known destroyed)
 timestamp visibility_checked_timestamp; // time when check occurred
 s16b scan_bitfield; // set up at creation and updated if changed. Basically this is a summary of proc characteristics for a scan to check against.
 s16b scan_bitfield_immutable; // this is the base bitfield set up at creation. It deals only with things that won't change if e.g. some components are destroyed.

 int self_destruct; // may be set to 1 by the terminate instruction

#define BUBBLE_TEXT_LENGTH_MAX 40
// BUBBLE_TEXT_LENGTH_MAX must not be more than STRING_MAX_LENGTH
 char bubble_text [BUBBLE_TEXT_LENGTH_MAX];
 int bubble_text_length;
 timestamp bubble_text_time; // w.world_time when bubble printed. If new text printed to bubble at same time, it is added to end.
 timestamp bubble_text_time_adjusted; // like bubble_text_time but may be adjusted if existing bubble replaced (to reduce the bubble phase-in effect)
 int bubble_list; // linked list of all visible cores with bubbles. terminated by -1.
 float bubble_x, bubble_y; // used if core is drawing a bubble
#define BUBBLE_TOTAL_TIME (DEALLOCATE_COUNTER-1)
// BUBBLE_TOTAL_TIME should probably be DEALLOCATE_COUNTER-1
//   to help deal with bubbles that remain visible after deallocation

 int special_AI_type; // set by some story processes using the special_AI() method
 int special_AI_value; // set by some story processes using the special_AI() method
 timestamp special_AI_time; // set by some story processes using the special_AI() method

};


struct proc_struct
{
// REMEMBER: When anything is added to this structure, it may need to be added to load/save routines in f_load.c/f_save.c

// int status;
 int exists; // is 1 if proc exists, 0 if proc doesn't exist
 int reserved; // is part of a group. May be a destroyed component of a group, as long as the core still exists.
	timestamp created_timestamp;
	timestamp destroyed_timestamp; // will be deallocating for a certain time after this.

 int player_index; // which player does this proc belong to?
 int index; // this is the index in the w.proc array (which is invisible to the player)
 int core_index; // index of core (not core_process).
 int group_member_index; // index of proc in core's group_member array. 0 for core. Is also index of proc in template member array.

 int selected; // is process selected?
 timestamp select_time; // time at which the proc was selected. Used to animate the selection graphics
 timestamp deselect_time;

// int deselected; // just used to animate the selection graphics going away (not currently used)
// int map_selected; // similar to selected but appears on the map instead of the main view.

 timestamp hit_pulse_time; // used when component (without interface) hit by packet, or when component collides
 timestamp repaired_timestamp;

 int mass;
 int mass_for_collision_comparison; // like mass, but greatly increased if proc is static (so that any mobile proc that collides with it bounces off)
// int moment; // moment of inertia - only the group value is now used
 int method_mass; // total mass of methods.
 int hp;
 int hp_max;
 int mobile; // 1 if process that proc is a component of can move, 0 if not. Depends on process core type.
// int redundancy; // strength of redundancy method (if any)


 int shape;
 int size;
 int packet_collision_size; // is increased if the proc has a virtual interface
 struct nshape_struct* nshape_ptr;

 cart position;
 block_cart block_position;
 cart speed;
 al_fixed angle;
 al_fixed spin;
// al_fixed spin_change; // this value is set to zero each tick then accumulates changes to spin that need to be precise (e.g. engine thrust where there might be counterbalanced engines), and is finally added back to spin
 int hit_edge_this_cycle;

// these old values are the x/y/angle values for the previous frame. Used in e.g. calculating the velocity of a vertex
 cart old_position;
 al_fixed old_angle;

 // provisional values are
 cart provisional_position;
 al_fixed provisional_angle;

 int prov; // if zero, has not had provisional values calculated this tick (can't compare indices because groups result in procs' movement being processed out-of-order)
// the difference between provisional values and test values is that test values have many more uses and need to be used carefully to avoid interrupting something else
// provisional values have only one use: telling how a proc will move this cycle.

// al_fixed max_length; // this is the maximum radius (from centre point, i.e. the point of (pr->x,pr->y)) of the proc. Is derived from its shape and size.
//  - should move this to template
 al_fixed drag;

// these pointers are used in collision detection, which builds a linked list of procs in each occupied block:
 struct proc_struct* blocklist_up;
 struct proc_struct* blocklist_down;

// s16b target_angle;
// u16b turning;
// s16b pulse_turn; // the direction the cl will turn in this pulse

// All of this group stuff may be unnecessary.
// If any of it is needed, it can probably be moved to templates.

// *** important: group_connection [0] is reserved for the link back to the parent (i.e. the proc one step closer to the core, or the core itself).
//   The core's group_connection [0] should be kept empty.
 struct proc_struct* group_connection_ptr [GROUP_CONNECTIONS]; // this remains valid even if the connected proc has been destroyed (because the entry in the proc array will be reserved in case it's restored)
 int group_connection_exists [GROUP_CONNECTIONS];
 int connected_from [GROUP_CONNECTIONS]; // the other side of the group_connection array - holds the index (from 0 to GROUP_CONNECTIONS-1) of this proc in the group_connection array of the other proc

 al_fixed connection_angle [GROUP_CONNECTIONS]; // the angle of the connection (compared to proc's own angle)
 al_fixed connection_dist [GROUP_CONNECTIONS]; // the distance between the procs
 al_fixed connection_angle_difference [GROUP_CONNECTIONS]; // the difference between the angles of the procs (compared to proc's own angle)
 int number_of_group_connections; // this value must be kept up to date

 int connection_link [GROUP_CONNECTIONS]; // which vertex connection is at
 int connected_from_link [GROUP_CONNECTIONS];

 struct object_struct object [MAX_LINKS]; // basic properties that are used by objects in both template and actual process
 struct object_instance_struct object_instance [MAX_LINKS]; // properties that are only used by objects in processes

// al_fixed group_angle; - now stored in core_struct
// al_fixed group_distance;
// int group_number_from_first; // each group member has a number that indicates its distance from 1st member. Needs to be kept up to date.
// al_fixed group_angle_offset; // offset of proc's angle from group angle

// these test values are used in various places where a group is being moved or assembled and we need to
// check that all procs fit where they need to go.
 al_fixed test_group_distance;
 cart test_position;
 al_fixed test_angle;
 block_cart test_block_position;
 unsigned int group_check;

// int interface_object_present; // can generate interface.
// int interface_depth;
// int interface_on_process_set_on; // setting of interface object on process; controlled by core's program. Is independent of the core interface variables so it doesn't necessarily indicate whether there is an interface on or not.
#define INTERFACE_POWER_USE 10
 int interface_protects; // can generate interface (if process as a whole has one)
 timestamp interface_hit_time;
 timestamp interface_raised_time;
 timestamp interface_lowered_time; // if interface turned off (but not broken) for this proc. May be because whole core's interface lowered, or just this proc's.
#define INTERFACE_STABILITY_POWER_USE 30
 int interface_stability;

 timestamp component_hit_time; // last time component was hit. Use for the hit-checking component method.
 int component_hit_source_index; // I haven't bothered to initialise these so they will only be valid if component_hit_time is not zero.
 timestamp component_hit_source_timestamp;

// timestamp interface_stability_on_time; - currently just does a hit pulse when raised or lowered
// timestamp interface_stability_off_time;
// To check whether proc is actually protected by an interface, need to check:
//  - that core->interface_active == 1
//  - that proc->interface_on_process_set_on == 1

// some of these may be needed:
// int listen_method; // is index of proc's listen method, or -1 if it doesn't have one.
// int allocate_method; // -1 if no allocate method
// int virtual_method; // -1 if no virtual method
// int give_irpt_link_method; // defaults to -1. process can set this to direct resource flow after group separation
// int give_data_link_method;

// int vertex_method [METHOD_VERTICES]; // indicates which method is on this vertex. -1 if no method.

};


//#define NO_PACKETS 500

enum
{
//PACKET_TYPE_BASIC,
//PACKET_TYPE_LBASIC,
//PACKET_TYPE_FBASIC,
//PACKET_TYPE_DPACKET,
//PACKET_TYPE_LDPACKET,
//PACKET_TYPE_FDPACKET,
PACKET_TYPE_PULSE,
//PACKET_TYPE_DPULSE,
PACKET_TYPE_SPIKE1, // spike1-5 are the same thing in different stages
PACKET_TYPE_SPIKE2,
PACKET_TYPE_SPIKE3,
//PACKET_TYPE_SPIKE4,
PACKET_TYPE_BURST,
PACKET_TYPE_ULTRA,


PACKET_TYPES
};

struct packet_struct
{
// REMEMBER: When anything is added to this structure, it may need to be added to load/save routines in f_load.c/f_save.c
 int exists;
 timestamp created_timestamp;
 timestamp lifetime;
 int index;
 int type;
 int player_index;

 int source_core_index; // source_core details used for get_damage_source std method. Can be -1 if invalid for some reason.
 timestamp source_core_timestamp;
 int source_proc; // index of source proc. -1 if source proc doesn't exist
 int source_method; // source method
 int source_vertex; // vertex of source method
 int prand_seed; // pseudorandom seed set at packet creation; used for graphics

 int damage;
 int team_safe; // which team will it not hit? -1 if hits all teams.
 int colour;
 int status; // what this means depends on type
 int fixed_status; // same
// int method_extensions [METHOD_EXTENSIONS];

// int size;
 int collision_size; // additional sizes checked against collision mask. 0 = point, 1 = extra 3 pixels, 2 = 6 pixels etc.

 cart position;
 cart position2; // what this means depends on packet type (currently probably only used for spikes)
 block_cart block_position;
 cart speed;
 al_fixed angle;

 struct packet_struct* blocklist_up;
 struct packet_struct* blocklist_down;

};

// currently w.max_clouds is set to CLOUDS
#define CLOUDS_BITS 11
#define CLOUDS (1<<CLOUDS_BITS)
#define CLOUDS_MASK (CLOUDS-1)

#define CLOUD_DATA 8

struct cloud_struct
{

 int exists; // needed?
 timestamp created_timestamp;
 timestamp lifetime; // cloud should cease to exist after world_time > created+lifetime. this may not be needed.
 timestamp destruction_timestamp; // time when the cloud can be regarded as destroyed

 int type;
 int colour;
// other details can be stored in data array below

 cart position;
 cart position2;
 cart speed;
 float display_size_x1, display_size_y1; // used for bounding-box display code. Not very optimised yet. because these are float, they shouldn't be used in any gameplay code.
 float display_size_x2, display_size_y2; // 1 is top/left, 2 is bottom/right
 al_fixed angle;
 int index;

 int data [CLOUD_DATA]; // various things specific to particular cloud types
 timestamp associated_proc_timestamp; // identifies a proc that the cloud is tied to. Index of proc is stored in data.

};

enum
{
CLOUD_NONE,

CLOUD_PACKET_MISS,
CLOUD_PACKET_HIT,
CLOUD_MAIN_PROC_EXPLODE, // explosion of proc hit by enemy (may or may not be core)
CLOUD_SUB_PROC_EXPLODE, // sub-processes of main proc destroyed
CLOUD_PROC_FRAGMENT, // small fragment of proc
CLOUD_INTERFACE_BREAK, // explosion of interface when broken. Appears in addition to interface fading away on each proc.
CLOUD_STREAM,
CLOUD_SLICE,
CLOUD_SLICE_FADE,
CLOUD_SPIKE_TRAIL,
CLOUD_SPIKE_HIT,
CLOUD_SPIKE_HIT_AT_LONG_RANGE, // when spike does max damage
CLOUD_SPIKE_MISS_AT_LONG_RANGE, // when spike does max damage
CLOUD_SPIKE_MISS,
CLOUD_BURST_HIT,
CLOUD_BURST_MISS,
CLOUD_ULTRA_HIT,
CLOUD_ULTRA_MISS,
CLOUD_BUBBLE_TEXT,
// old cloud types that may be able to be removed:


CLOUD_PACKET_HIT_VIRTUAL,
CLOUD_FAILED_NEW, // outline of failed new proc
CLOUD_DATA_LINE, // line from gen data method to block node
CLOUD_YIELD_LINE, // line from yield method target proc
CLOUD_VIRTUAL_BREAK,

CLOUD_PROC_EXPLODE_LARGE,
CLOUD_PROC_EXPLODE_SMALL,
CLOUD_PROC_EXPLODE_SMALL2,

//CLOUD_STREAM_HIT,

CLOUD_HARVEST_LINE, // line from harvest object to data well
CLOUD_GIVE_LINE, // line from harvest object to target of data transfer
CLOUD_TAKE_LINE, // line from source of data transfer to harvest object
CLOUD_BUILD_LINE, // line from build harvest object to new core
CLOUD_BUILD_FAIL, // tried to build but failed. cloud appears around build object.
CLOUD_REPAIR_LINE, // line from repair object to repaired proc

CLOUD_DEBUG_LINE,
CLOUD_TYPES

};

#define FRAGMENT_BITS 7
#define FRAGMENTS (1<<FRAGMENT_BITS)
#define FRAGMENT_MASK (FRAGMENTS - 1)
#define FRAGMENT_MAX_LIFETIME 128

struct fragment_struct
{

// int exists; // needed?
 timestamp created_timestamp;
// timestamp lifetime;
 timestamp explosion_timestamp; // when fragment explodes
 timestamp destruction_timestamp; // when fragment ceases to exist at all

// int type;
 int colour;
 int fragment_size;

 cart position;
// cart position2;
 cart speed;
 float spin; // only used for display

};

#define DATA_WELLS 24
#define DATA_WELL_RESERVES 2
// DATA_WELL_RESERVES should probably stay as 2
#define DATA_WELL_REPLENISH_RATE 1
// DATA_WELL_REPLENISH_RATE is the replenishment rate per square per reserve

struct well_struct
{
	int active; // set to 0 when the data well has completely run out of data
	cart position;
	block_cart block_position;
	int data; // data currently available
	int data_max; // probably the same for all?
	timestamp last_harvested; // last time data was harvested from this well. Used in display functions.
	timestamp last_transferred; // last time data was transferred from reserve to well
	float spin_rate; // only used for display
	al_fixed static_build_exclusion; // radius of static build exclusion around the data well

	int reserve_data [DATA_WELL_RESERVES];
	int reserve_squares; // currently the same for both reserves

	timestamp last_drawn; // last value of game.total_time when this data well was drawn

};



#define BLOCK_NODES 9
#define NODE_COLS 2
#define VISION_BLOCKS 4
#define BACKBLOCK_LAYERS 4

// block_struct is used for gameplay-related stuff
struct block_struct
{
// REMEMBER: When anything is added to this structure, it may need to be added to load/save routines in f_load.c/f_save.c
  unsigned int tag;
  struct proc_struct* blocklist_down;
  unsigned int packet_tag;
  struct packet_struct* packet_down;
  int block_type; // this is the type used for edge-of-map collision detection
};

// backblock_struct is used for display-related stuff
struct backblock_struct
{

  int backblock_type; // used for display
  int backblock_value; // meaning depends on type (e.g. for data wells it's data well index)

// TO DO: would probably be a useful optimisation to separate the gameplay stuff from the display stuff.

  int node_exists [BLOCK_NODES]; // 0 if node is turned off (1 otherwise). Probably just turns off display.
  int node_x [BLOCK_NODES]; // offset from top left corner of block, in pixels
  int node_y [BLOCK_NODES];
  int node_size [BLOCK_NODES];
  int node_depth [BLOCK_NODES]; // drawing layer - should be 0-3. actual parallax depth depends on w.backblock_parallax values
//  int node_col [BLOCK_NODES] [NODE_COLS];
  int node_team_col [BLOCK_NODES];
  int node_col_saturation [BLOCK_NODES];
// if node is disrupted:
  timestamp node_disrupt_timestamp [BLOCK_NODES];


  timestamp node_colour_change_timestamp [BLOCK_NODES];
  int node_new_colour [BLOCK_NODES]; // if world_time is after node_colour_change_timestamp, node_new_colour is used instead of node_team_col.
  int node_new_saturation [BLOCK_NODES]; // same

  int node_x_base [BLOCK_NODES];
  int node_y_base [BLOCK_NODES];

// pending explosion:
		timestamp node_pending_explosion_timestamp [BLOCK_NODES]; // this is time when explosion starts affecting node
//  int node_pending_explosion_player [BLOCK_NODES]; // index of player causing explosion (used for colour)
  int node_pending_explosion_strength [BLOCK_NODES]; // power of explosion when it reaches this node


// this vision stuff is only used in display code (w.vision_area is used for process visibility):


};

struct vision_block_struct
{

 int proximity; // shortest x or y (orthogonal) distance to core
 timestamp proximity_time; // time those distances apply to (same for whole block)
 timestamp clear_time; // time last seen clearly (not at edge of view)


};


// VISION_AREA_VISIBLE_TIME is how long a vision_area remains visible to a process. 128 may actually be a bit long - not sure
#define VISION_AREA_VISIBLE_TIME 128

struct vision_area_struct
{
	timestamp vision_time;

// timestamp subblock_vision_time [4]; // confusing use of block vs subblock

};

enum
{
BLOCK_NORMAL, // normal empty block
BLOCK_SOLID, // can't be passed
BLOCK_EDGE_LEFT, // put this to the right of a vertical line of solid blocks. acts like a normal block, except that procs in it run collision detection against the three adjacent solid blocks
BLOCK_EDGE_RIGHT,
BLOCK_EDGE_UP,
BLOCK_EDGE_DOWN,
BLOCK_EDGE_UP_LEFT, // put this to the right of a vertical line of solid blocks and below a horizontal line (e.g. top left corner of map).
BLOCK_EDGE_UP_RIGHT,
BLOCK_EDGE_DOWN_LEFT,
BLOCK_EDGE_DOWN_RIGHT,

BLOCK_TYPES
// Note: there is not yet any way to have a convex corner of solid blocks, only concave (like at the corners of the map)


};
/*
enum
{
LOCAL_CONDITION_NONE,
LOCAL_CONDITION_STATIC,
LOCAL_CONDITION_FRAGILE_PROCS,
LOCAL_CONDITION_THIN_INTERFACE,
LOCAL_CONDITION_SINGLE_ALLOCATOR,
LOCAL_CONDITION_SOMETHING, // not sure about this one

LOCAL_CONDITIONS
};*/


struct world_struct
{

  int allocated; // is 0 if world not allocated (and so doesn't need to be freed before using); 1 if allocated (must call deallocate_world() before allocating new world)
   // currently the world_struct is not dynamically allocated so this may not be too relevant. However, it is best not to do anything to the world_struct if !allocated

#define MAX_CORES_PER_PLAYER 128
#define MAX_PROCS_PER_PLAYER 512


#define MAX_CORES (MAX_CORES_PER_PLAYER * PLAYERS)
#define MAX_PROCS (MAX_PROCS_PER_PLAYER * PLAYERS)
#define MAX_PACKETS 800

//#define USE_DYNAMIC_MEMORY

#ifdef USE_DYNAMIC_MEMORY

  struct core_struct* core;
  struct proc_struct* proc;
  struct packet_struct* packet;
  struct cloud_struct* cloud;
  struct block_struct** block;
  struct vision_area_struct** vision_area [PLAYERS];

#else

#define MAXIMUM_BLOCK_SIZE 120

  struct core_struct core [MAX_CORES];
  struct proc_struct proc [MAX_PROCS];
  struct packet_struct packet [MAX_PACKETS];
  struct cloud_struct cloud [CLOUDS];
		int fragment_count;
  struct fragment_struct fragment [FRAGMENTS];
  struct block_struct block [MAXIMUM_BLOCK_SIZE] [MAXIMUM_BLOCK_SIZE];
  struct backblock_struct backblock [MAXIMUM_BLOCK_SIZE] [MAXIMUM_BLOCK_SIZE];
  float backblock_parallax [BACKBLOCK_LAYERS];
  struct vision_block_struct vision_block [MAXIMUM_BLOCK_SIZE] [MAXIMUM_BLOCK_SIZE];
  struct vision_area_struct vision_area [PLAYERS] [MAXIMUM_BLOCK_SIZE] [MAXIMUM_BLOCK_SIZE];

#endif

  int vision_areas_x, vision_areas_y;

  timestamp blocktag;
#define BASE_WORLD_TIME 255
  timestamp world_time; // stars at BASE_WORLD_TIME. Doesn't include time spent paused.
  int world_seconds; // number of seconds; used for time limits and display (may be slightly out because it's an integer calculation)

  int max_cores;
  int max_procs;
  int max_packets;
  int max_clouds; // currently this is set to CLOUDS
  int cores_per_player;
  int procs_per_player; // max procs for each player

  int story_area;
//  int local_condition;

  struct well_struct data_well [DATA_WELLS]; // there aren't enough data wells to justify mallocing this
  int data_wells;

  block_cart blocks; // size of world in blocks
  unsigned int w_pixels, h_pixels; // size of world in pixels
  cart fixed_size; // size of world in cart fixed-point

  int current_output_console; // set to default just before each program runs. If this is -1, prints are muted.
  int current_error_console; // set to default just before each program runs. If this is -1, prints are muted.
  int print_colour;
  int print_proc_action_value3; // this is the value3 that will be set for console actions attached to lines printed by processes. It's set to 0 before each process runs but can be changed by the MT_EX_ACTION method.

  int players; // number of active players (including any computer-controlled ones)
  int command_mode; // 0 for autonomous, 1 for accepts commands

// the following values are retained from w_init and not used during play, but are used when setting up the game from a loaded save:

// player structures contain client/operator programs, if present
  struct player_struct player [PLAYERS]; // playerstructs must be filled from zero onwards, without any gaps

// the world may have a program that runs everything:
//  struct programstruct system_program; // system_program.active will be 0 if no system program
//  s16b system_bcode_op [BCODE_SIZE]; // system's programstruct will have a pointer to this as its bcode

// these are markers that appear on the map - this isn't really a world thing (as they're just part of the interface), but they don't really fit in view_struct either
//  struct markerstruct marker [MARKERS];
//  int last_marker; // this is the last marker put in the world (used to work out which marker subsequent calls to place_marker() should apply to - see SELECT method code in g_method.c and marker code in g_method_clob.c)

 int background_colour [3];
 int hex_base_colour [3];


// debug mode is in-game debugging for process code. Currently it only prints a few extra messages.
 int debug_mode_general; // set by pressing F1? default value for debug_mode for all procs of user's player
 int debug_mode; // 0 or 1. reset to w.debug_mode_general at start of proc execution.



};

struct under_attack_marker_struct
{
	timestamp time_placed_world; // this is w.world_time - determines how long it lasts
//	timestamp time_placed_game; // this is game.total_time - animates
	cart position;//int block_x, block_y; // block location of marker
}; // used in view_struct


#define ZOOM_MAX_LEVEL 3
// note zoom starts at 1

// TO DO: it should be possible to have multiple view_structs in multiple panels. So don't put anything here that would prevent that (put general interfact stuff in control_struct control instead
struct view_struct
{
// REMEMBER: When anything is added to this structure, it may need to be added to load/save routines in f_load.c/f_save.c
 al_fixed camera_x, camera_y;

 int window_x_zoomed, window_y_zoomed; // zoomed pixels
 int window_x_unzoomed, window_y_unzoomed; // pixels, ignoring zoom
 int just_resized; // is 1 if the window has just been resized, or if it's just been loaded up. Otherwise zero. This value is available to client program through METH_VIEW call.
 al_fixed centre_x_zoomed, centre_y_zoomed;
// al_fixed centre_x_unzoomed, centre_y_unzoomed; // pixels, ignoring zoom
 int fps;
 int cycles_per_second;

// struct proc_struct* focus_proc;
 int following; // if 1, follows selected_core [0]

 int w_x_pixel, w_y_pixel; // size of world in pixels
 al_fixed camera_x_min, camera_y_min;
 al_fixed camera_x_max, camera_y_max;
 float zoom;
 int zoom_level; // 1 to ZOOM_MAX_LEVEL
 int zoom_int; // integer zoom
 int screen_shake_time;

 int map_visible; // 0 or 1
 int map_x, map_y;
 int map_w, map_h;

#define BUILD_BUTTON_H 30
#define BUILD_BUTTON_W 140
 int build_buttons_x1;
 int build_buttons_y1;
 int build_buttons_x2;
 int build_buttons_y2;
 int build_queue_buttons_y1; // use same x
 int build_queue_buttons_y2;
 int mouse_on_build_button;
 timestamp mouse_on_build_button_timestamp;
 int mouse_on_build_queue_button;
 timestamp mouse_on_build_queue_button_timestamp;

 al_fixed map_proportion_x;
 al_fixed map_proportion_y;

 int data_box_open;
 float data_box_close_button_x1;
 float data_box_close_button_y1;
 float data_box_close_button_x2;
 float data_box_close_button_y2;

#define UNDER_ATTACK_MARKERS 16
#define UNDER_ATTACK_MARKER_SIZE 16
// UNDER_ATTACK_MARKER_SIZE is size of marker in blocks.
#define UNDER_ATTACK_MARKER_DURATION 223
// in ticks - should be <255 and (divisible by 32) - 1

	struct under_attack_marker_struct under_attack_marker [UNDER_ATTACK_MARKERS];
 timestamp under_attack_marker_last_time; // w.world_time of placing most recent marker. If too long ago, don't need to check any of them.


};


// MBB_PER_BUTTON is how many bits there are for each button
#define MBB_PER_BUTTON 3

enum // mouse button bitfield entries (for control.mbutton_bitfield)
{
MBB_BUT1_PRESSED, // is button 1 (left) being pressed?
MBB_BUT1_JUST_PRESSED, // was button 1 pressed since the previous tick? (PRESSED will also be 1)
MBB_BUT1_JUST_RELEASED, // was button 1 released since the previous tick? (PRESSED will be 0)
MBB_BUT2_PRESSED,
MBB_BUT2_JUST_PRESSED,
MBB_BUT2_JUST_RELEASED,
// need wheel things too
MBB_BITS
};



//enum
//{
//MOUSE_STATUS_OUTSIDE, // unavailable to the game or panels
//MOUSE_STATUS_PANEL, // on a panel
//MOUSE_STATUS_DRAG, // is dragging something

// Not sure the following are needed
//MOUSE_STATUS_GAME, // in the game window and available to the game. Not dragging anything.
//MOUSE_STATUS_GAME_MAP,
//MOUSE_STATUS_GAME_CONSOLE,
//MOUSE_STATUS_GAME_PROCESS
//};


enum
{
KEY_0,
KEY_1,
KEY_2,
KEY_3,
KEY_4,
KEY_5,
KEY_6,
KEY_7,
KEY_8,
KEY_9,

KEY_A,
KEY_B,
KEY_C,
KEY_D,
KEY_E,
KEY_F,
KEY_G,
KEY_H,
KEY_I,
KEY_J,
KEY_K,
KEY_L,
KEY_M,
KEY_N,
KEY_O,
KEY_P,
KEY_Q,
KEY_R,
KEY_S,
KEY_T,
KEY_U,
KEY_V,
KEY_W,
KEY_X,
KEY_Y,
KEY_Z,

KEY_MINUS,
KEY_EQUALS,
KEY_SBRACKET_OPEN,
KEY_SBRACKET_CLOSE,
KEY_BACKSLASH,
KEY_SEMICOLON,
KEY_APOSTROPHE,
KEY_COMMA,
KEY_PERIOD,
KEY_SLASH,

KEY_UP,
KEY_DOWN,
KEY_LEFT,
KEY_RIGHT,

KEY_ENTER,
KEY_BACKSPACE,
KEY_INSERT,
KEY_HOME,
KEY_PGUP,
KEY_PGDN,
KEY_DELETE,
KEY_END,
KEY_TAB,
// KEY_ESCAPE is not available to user programs

KEY_PAD_0,
KEY_PAD_1,
KEY_PAD_2,
KEY_PAD_3,
KEY_PAD_4,
KEY_PAD_5,
KEY_PAD_6,
KEY_PAD_7,
KEY_PAD_8,
KEY_PAD_9,
KEY_PAD_MINUS,
KEY_PAD_PLUS,
KEY_PAD_ENTER,
KEY_PAD_DELETE,

// These are last so that any_key is set to them only if they're the only thing being pressed
KEY_LSHIFT,
KEY_RSHIFT,
KEY_LCTRL,
KEY_RCTRL,

KEY_F1,
KEY_F2,
KEY_F3,
KEY_F4,
KEY_F5,
KEY_F6,
KEY_F7,
KEY_F8,
KEY_F9,
KEY_F10,
KEY_F11,
KEY_F12,

KEYS

};



enum
{
MOUSE_STATUS_OUTSIDE, // unavailable to the game or panels
MOUSE_STATUS_GAME, // in the game window and available to the game. Not dragging anything.
MOUSE_STATUS_PANEL, // on a panel
MOUSE_STATUS_DRAG, // is dragging something
//MOUSE_STATUS_DRAG_SHIFT, // is dragging something and shift is being pressed (not sure if needed)
};


enum
{
MOUSE_DRAG_NONE,
MOUSE_DRAG_SLIDER, // mouse is dragging slider handle
MOUSE_DRAG_TEXT, // mouse is dragging selected text from editor
MOUSE_DRAG_PANEL_RESIZE, // mouse is dragging panel edge
MOUSE_DRAG_DESIGN_MEMBER, // mouse is dragging design panel member
MOUSE_DRAG_DESIGN_OBJECT, // mouse is dragging angle of object on design panel member
MOUSE_DRAG_DESIGN_OBJECT_COPY, // mouse is dragging object on design panel member and pressing shift
MOUSE_DRAG_DESIGN_OBJECT_MOVE, // mouse is dragging object on design panel member
MOUSE_DRAG_BUILD_QUEUE
};


struct control_struct // this struct needs to be folded into ex_control (or vice-versa)
{
// REMEMBER: When anything is added to this structure, it may need to be added to load/save routines in f_load.c/f_save.c
 int mouse_status; // what part of the display the mouse pointer is in
 int mouse_panel; // unreliable if mouse_status != MOUSE_STATUS_PANEL
 int mouse_drag; // MOUSE_DRAG_* enum
 int mouse_drag_panel; // panel from which mouse is dragging something
 int mouse_drag_element; // probably the element that's being dragged
 int mouse_drag_build_queue_button; //
 int mouse_x_world_pixels, mouse_y_world_pixels; // position in the world (not on the screen), in pixels
 int mouse_x_screen_pixels, mouse_y_screen_pixels; // position on the screen, in pixels
// int mbutton_bitfield;
// int mouse_hold_x_pixels [2], mouse_hold_y_pixels [2]; // if button is being held, this is the x/y position where it was when the hold started. x is -1 if the hold started offscreen or when mouse input not accepted.
// int mb_bits [MBB_BITS];
 int mbutton_press [2];
 timestamp mbutton_press_timestamp [2];
// int key_press [KEYS];
// int any_key;
 int panel_element_highlighted;
	int panel_element_highlighted_time;

	int editor_captures_input; // editor captures keyboard and some other input (e.g. mousewheel) when mouse is over non-main panel and editor panel is open

};

enum
{
BUTTON_JUST_RELEASED = -1,
BUTTON_NOT_PRESSED,
BUTTON_JUST_PRESSED,
BUTTON_HELD
};



// a group can't exceed this size

/*
struct groupstruct
{
// REMEMBER: When anything is added to this structure, it may need to be added to load/save routines in f_load.c/f_save.c
 int exists;
 struct proc_struct* first_member;
 int shared_irpt;
 int shared_irpt_max;
 int shared_data;
 int shared_data_max;
 int total_irpt_gain_max; // irpt gain per tick if all irpt methods in group are generating at max capacity

 int total_mass;
 int total_members;
 int moment; // moment of inertia - see calculate_group_moment_of_inertia() in g_group.c.
 int mobile; // 1 if group can move, 0 otherwise (is 0 if at least one proc member has mobile==0)

// x, y values: for mobile groups, are the group's centre of mass. For groups with roots (immobile), is the first member
 al_fixed x, y, test_x, test_y;
 int centre_of_mass_test_x, centre_of_mass_test_y; // used for calculating centre of mass (which is likely to cause an al_fixed value to overflow)
 al_fixed x_speed, y_speed;
 al_fixed drag; // is equal to the lowest (i.e. most draggy) drag of all group members
 al_fixed angle, test_angle;
// int angle_angle; // angle in angle units
 al_fixed spin;
 al_fixed spin_change; // this value is set to zero each tick then accumulates changes to spin that need to be precise (e.g. engine thrust where there might be counterbalanced engines), and is finally added back to spin

 int hit_edge_this_cycle;

// this value is incremented if the group is stuck against a proc or another group. Once it gets high enough, we try to free the group by jittering it around randomly.
 int stuck;

};*/

/*

Every cl has the following variables:

group - contains the group it's a member of (1 only). -1 if no group
next_in_group - index of next member of group


*/





enum
{
COL_GREY,
COL_RED,
COL_GREEN,
COL_BLUE,
COL_YELLOW,
COL_ORANGE,
COL_PURPLE,
COL_TURQUOISE,
COL_AQUA,
COL_CYAN,
COL_DULL,
BASIC_COLS
};

#define BASIC_SHADES 5
// the shade values can be adjusted as needed if BASIC_SHADES is changed
#define SHADE_MIN 0
#define SHADE_LOW 1
#define SHADE_MED 2
#define SHADE_HIGH 3
#define SHADE_MAX 4

enum
{
TRANS_FAINT,
TRANS_MED,
TRANS_THICK,
BASIC_TRANS
};

//#define MAP_W 150
//#define MAP_H 150

// MAP_EDGE_DISTANCE is the distance from the edge of the screen to the edge of the map
#define MAP_EDGE_DISTANCE 50

#define COLLISION_MASK_SIZE 120
#define COLLISION_MASK_BITSHIFT 1
//#define COLLISION_MASK_LEVELS 10

// MASK_CENTRE needs to be adjusted because COLLISION_MASK_BITSHIFT will be applied to it before it
//  is translated to an actual position on the collision mask
#define MASK_CENTRE ((COLLISION_MASK_SIZE/2)<<COLLISION_MASK_BITSHIFT)
#define MASK_CENTRE_FIXED al_itofix(MASK_CENTRE)


// settingsstruct stuff

#define MODE_BUTTON_SIZE 16
#define MODE_BUTTON_SPACING 7
#define MODE_BUTTON_Y 5

enum
{
OPTION_WINDOW_W, // resolution of monitor or size of window
OPTION_WINDOW_H,
OPTION_FULLSCREEN,
OPTION_VOL_MUSIC,
OPTION_VOL_EFFECT,
OPTION_SPECIAL_CURSOR, // draws a special mouse cursor instead of the system one. Included because of a report that the mouse cursor was not displaying correctly.
OPTION_DEBUG, // can be used to set certain debug values without recompiling.
OPTIONS
};

enum
{
SELECTOR_NONE,
SELECTOR_BASIC
};

enum
{
INPUT_WORLD,
INPUT_EDITOR

};

/*
enum
{
EDIT_WINDOW_PROGRAMS,
EDIT_WINDOW_TEMPLATES,
EDIT_WINDOW_EDITOR,
EDIT_WINDOW_SYSMENU,
EDIT_WINDOW_CLOSED,
EDIT_WINDOWS
// EDIT_WINDOW enum must match MODE_BUTTON enum
};*/

enum
{
// from right to left:
MODE_BUTTON_CLOSE, // close all panels
MODE_BUTTON_SYSTEM,
MODE_BUTTON_EDITOR,
MODE_BUTTON_DESIGN,
MODE_BUTTON_TEMPLATES,

#define LEFT_MODE_BUTTON MODE_BUTTON_TEMPLATES
/*
MODE_BUTTON_PROGRAMS,
MODE_BUTTON_TEMPLATES,
MODE_BUTTON_EDITOR,
MODE_BUTTON_SYSMENU,
MODE_BUTTON_CLOSE,*/
MODE_BUTTONS
// MODE_BUTTON enum must match EDIT_WINDOW enum
};

enum
{
MOUSE_CURSOR_BASIC,
MOUSE_CURSOR_PROCESS_FRIENDLY,
MOUSE_CURSOR_PROCESS_ENEMY,
MOUSE_CURSOR_PROCESS_DATA_WELL,
MOUSE_CURSOR_MAP,

MOUSE_CURSOR_RESIZE,
MOUSE_CURSOR_TEXT,
MOUSE_CURSOR_DESIGN_DRAG_OBJECT,
MOUSE_CURSOR_DESIGN_DRAG_OBJECT_COPY,

MOUSE_CURSORS

};


// sets out mutable aspects of the interface (that aren't game settings)
struct inter_struct
{

 int display_w, display_h;

 int panel_input_capture; // which panel keyboard use is sent to (changed by clicking on a different panel)
  // need some clear display change to indicate which panel is capturing input.
  //  - maybe - background colour (and outline?) for panels, console outline for game

 int mode_button_available [MODE_BUTTONS]; // 0 or 1. If 0, mode button displayed but cannot be clicked
 int mode_button_x [MODE_BUTTONS];
 int mode_button_y [MODE_BUTTONS];
 int mode_button_highlight;// [MODE_BUTTONS];
 timestamp mode_button_highlight_time;

// int panel_x_split; // x location of split between game and editor sides of the screen (when editor is up)
 int edit_window_columns; // fix! - this should go into editorstruct

 int mlog_x1, mlog_y1;

 timestamp running_time; // how long has the program been running? (used for various interface things). Incremented in get_ex_control in m_input.c
// differs from game.total_time in that it's not reset when starting a level/mission etc.

// ALLEGRO_MOUSE_CURSOR* mcursor [MOUSE_CURSORS];

};

enum
{
STORY_TYPE_NORMAL,
STORY_TYPE_ADVANCED,
STORY_TYPE_HARD,
STORY_TYPE_ADVANCED_HARD,

STORY_TYPES
};


enum
{
MISSION_TUTORIAL1,
MISSION_TUTORIAL2,

MISSION_BLUE_1,
MISSION_BLUE_2,
MISSION_BLUE_CAPITAL,

MISSION_GREEN_1,
MISSION_GREEN_2,
MISSION_GREEN_CAPITAL,

MISSION_YELLOW_1,
MISSION_YELLOW_2,
MISSION_YELLOW_CAPITAL,

MISSION_ORANGE_1,
MISSION_ORANGE_2,
MISSION_ORANGE_CAPITAL,

MISSION_PURPLE_1,
MISSION_PURPLE_2,
MISSION_PURPLE_CAPITAL,

//MISSION_DARK_BLUE_1,
//MISSION_DARK_BLUE_2,
//MISSION_DARK_BLUE_CAPITAL,

MISSION_RED_1,
MISSION_RED_2,
MISSION_RED_CAPITAL,

MISSIONS
};

// This is a set of game configuration values like system options etc
struct settingsstruct
{
// REMEMBER: When anything is added to this structure, it may need to be added to load/save routines in f_load.c/f_save.c
 int status; // contains a GAME_STATUS enum value which indicates what the user is doing

// int edit_window; // EDIT_WINDOW_CLOSED, _EDITOR or _TEMPLATES

 int sound_on; // if 1, plays sound. If 0, won't play sound at all (is set to 0 if Allegro audio addon fails to initialise)

 int option [OPTIONS];

 int replace_colour [PLAYERS]; // replaces a player's colour with another colour. Set in init file. is -1 if player's colour not replaced.

 char default_template_path [PLAYERS] [TEMPLATES_PER_PLAYER] [FILE_PATH_LENGTH]; // will be length 0 if no default template

 int saved_story_mission_defeated [STORY_TYPES] [MISSIONS]; // 0 not defeated, 1 defeated. This should match the state of the msn.dat file.

};


// game_struct stuff

enum
{
GAME_PHASE_WORLD, // the world is open
GAME_PHASE_PREGAME, // pregame phase (mostly like world phase but with some differences)
// WORLD and PREGAME phases should be 0 and 1 (as some comparisons use if (game.phase > GAME_PHASE_PREGAME) to detect all other phases
GAME_PHASE_TURN, // world is open; waiting for players to complete a turn (not currently implemented)
GAME_PHASE_OVER, // game is over

GAME_PHASE_FORCE_QUIT, // clicked on quit in system menu - forces the game loop to break

GAME_PHASE_MENU, // user is still in start menus (this value won't be encountered much, if ever, in practice)

GAME_PHASES // used in game loading bounds-check
};


enum
{
// these are used by SY_MANAGE method to indicate game ending states
GAME_END_BASIC, // just displays game over sign
GAME_END_MISSION_COMPLETE, // displays "You won" message or something. *** if playing a mission/tutorial, finishing with this status sets the mission status to complete
GAME_END_MISSION_FAILED, // displays failure message
GAME_END_MISSION_FAILED_TIME, // displays failure message - out of time
GAME_END_PLAYER_WON, // displays victory message for player indicated by field 2
GAME_END_DRAW, // displays draw message
GAME_END_DRAW_OUT_OF_TIME, // out of time

GAME_END_STATES

};

enum
{
FAST_FORWARD_OFF,
FAST_FORWARD_JUST_STARTED, // first frame in which FF activated (this status indicates that one more frame should be drawn, to avoid up to a second pause before the "FAST FORWARD" message appears on screen)
FAST_FORWARD_ON
};


enum
{
// These are not yet implemented. All fast forwarding is "skippy"
FAST_FORWARD_TYPE_SMOOTH, // runs the game at max speed, drawing a frame for each tick but not waiting to draw the next tick
FAST_FORWARD_TYPE_SKIP, // Fastest type. Runs the game at max speed for 1 second, then draws a frame, then runs again at max speed etc.
FAST_FORWARD_TYPE_NO_DISPLAY, // runs the game at 4x speed, drawing a frame for each tick if there's time
//FAST_FORWARD_TYPE_8X, // runs the game at 8x speed, drawing a frame for each tick if there's time (and at least one each second)

FAST_FORWARD_TYPES
};

enum
{
GAME_TYPE_BASIC, // basic game played from setup menu (includes multiplayer). Ends when only one player has any surviving processes.
GAME_TYPE_MISSION, // mission. See s_mission.c for ending conditions.

GAME_TYPES
};

enum
{
SPAWN_FAIL_LOCK, // couldn't lock template
SPAWN_FAIL_DATA, // template 0 too expensive

};


// game_struct game is initialised in g_game.c
struct game_struct
{
// REMEMBER: When anything is added to this structure, it may need to be added to load/save routines in f_load.c/f_save.c   <-- not currently implemented
 int phase; // GAME_PHASE_? enums
 int spawn_fail; // when user clicks to start game but one or more players aren't ready (-1 if no spawn fail; otherwise player index)
 int spawn_fail_reason;
 int type; // GAME_TYPE_? enums
 int mission_index; // only relevant in GAME_TYPE_MISSION
 int area_index; // only relevant in GAME_TYPE_MISSION
 int region_index; // only relevant in GAME_TYPE_MISSION
 int region_in_area_index; // currently 0, 1 or 2, or -1 in custom games. Used for sound stuff
//  file names may be used in future if file naming conventions are adopted, allowing automatic turnfile loading.
// char file_path [FILE_PATH_LENGTH]; // path where turnfiles etc are stored. Is determined by path to gamefile/savefile.
// char name [FILE_NAME_LENGTH]; // name of game (actually limited to just a few letters). Is put at start of all turn files.

 int story_type; // custom games are STORY_TYPE_NORMAL

 int user_player_index; // player index of the user (probably 0)

// int turns; // number of turns in game - is 0 for unlimited turns
// int current_turn; // which turn is it now (starts at 0 during pregame; 1 is first actual turn; is always 0 for indefinite game)

 timestamp total_time; // includes time spent paused (unlike w.world_time)

// int minutes_each_turn; // how long each turn is (in minutes) - is 0 for turns of unlimited length
// int current_turn_time_left; // how much time is left in current turn (in ticks)

 int pause_soft; // if 1, game is in soft pause. observer, operator and system programs still run but no others do and nothing else happens
// int pause_hard; // if 1, game is in hard pause. no programs run and ex_control input is not converted to control input.
 int fast_forward; // uses FAST_FORWARD_? enums
 int fast_forward_type; // uses FAST_FORWARD_TYPE_? enums
 int vision_mask; // 0 for soft (transparent) vision mask, 1 for opaque

 int game_over_status; // GAME_END_? enum. Ignored unless game.phase is GAME_PHASE_OVER
 int game_over_value; // some game end statuses use this (e.g. to indicate which player won)
 int game_over_time; // counter; prevents "click to exit" sign appearing immediately


/*
// Note that the sound values aren't saved as part of a saved game
 int play_sound;
 int play_sound_sample;
 int play_sound_note;
 int play_sound_vol;
 int play_sound_counter; // prevents sounds being played too frequently by keeping a minimum number of ticks between them.
*/

};
/*
enum
{
PI_OPTION, // if 1, user can set this between PI_MIN and PI_MAX (starts at PI_DEFAULT). if 0, it will be fixed at PI_DEFAULT
PI_DEFAULT,
PI_MIN,
PI_MAX,
PI_VALUES
};*/

enum
{
COMMAND_MODE_AUTO,
COMMAND_MODE_COMMAND,

COMMAND_MODES
};

// MAX_LIMITED_GAME_TURNS is the maximum number of turns in a game with limited turns
//#define MAX_LIMITED_GAME_TURNS 16
// MAX_UNLIMITED_GAME_TURNS is the maximum number of turns in a game without limited turns (can be almost any amount really, although > 32767 would mean programs would lose track of it)
//#define MAX_UNLIMITED_GAME_TURNS 30000
//#define MAX_TURN_LENGTH_MINUTES 60

// this is a structure to contain all world initialisation values - i.e. independent values that may be set by a user or system program.
// a world can be created based on these values.
// the parameters within which the values are set are determined by a world_preinitstruct, which is determined by the system program (or is left to default values)
struct world_init_struct
{
// REMEMBER: When anything is added to this structure, it may need to be added to load/save routines in f_load.c/f_save.c
 int players; // 2-4 or assymetrical
 int core_setting; // 0-3
 int size_setting; // 0-3 - note that unlike other values this one is not used directly in world generation (w_init.map_size_blocks is used instead)
#define MAP_SIZE_0 60
#define MAP_SIZE_1 80
#define MAP_SIZE_2 100
#define MAP_SIZE_3 120

 int command_mode;

 int game_seed; // 0-999

 char player_name [PLAYERS] [PLAYER_NAME_LENGTH];
// int player_starting_data [PLAYERS]; - this is derived from starting_data_setting
 int starting_data_setting [PLAYERS]; // 0-3 (actual amount is (this + 1) * 300)

// the following map-related information is generated from the various settings above
 int map_size_blocks; // generated for convenience
// int local_condition;
 int story_area;

/*
 block_cart spawn_position [PLAYERS]; // may be nonsense for non-existent players
 int spawn_angle [PLAYERS];

 int data_wells;
 block_cart data_well_position [DATA_WELLS];
	float data_well_spin_rate [DATA_WELLS]; // only used for display
	int reserve_data [DATA_WELLS] [DATA_WELL_RESERVES];
	int reserve_squares [DATA_WELLS]; // currently the same for both reserves
*/

// game parameters
// int game_turns; // 0 means indefinite
// int game_minutes_each_turn; // ignored if indefinite

// basic world parameters
// int cores_per_player;
// int procs_per_player;
// int gen_limit;
// int packets_per_player;
// int max_clouds;
// block_cart blocks; // size of world in blocks

// int may_change_proc_templates [PLAYERS]; // whether a player may change the player's proc templates

// int system_program; // is 1 if there is a system program

// when adding anything to this, may need to add to f_load and f_save.

};





enum
{
PRINT_COL_DGREY,
PRINT_COL_LGREY,
PRINT_COL_WHITE,
PRINT_COL_LBLUE,
PRINT_COL_DBLUE,
PRINT_COL_LRED,
PRINT_COL_DRED,
PRINT_COL_LGREEN,
PRINT_COL_DGREEN, // rgb values are in print_col_value [] in i_disp.c
PRINT_COL_LPURPLE,
PRINT_COL_DPURPLE,
PRINT_COLS
};

#define CONSOLE_LINE_FADE 16

#define BACK_COL_SATURATIONS 4
#define BACK_COL_FADE 8

#define MAP_MINIMUM_SIZE 30


// TORQUE_DIVISOR is an arbitrary value to fine-tune the rotation physics
#define TORQUE_DIVISOR 8
#define TORQUE_DIVISOR_FIXED al_itofix(10)
// FORCE_DIVISOR is an arbitrary value to fine-tune the bounce physics
#define FORCE_DIVISOR 4
#define FORCE_DIVISOR_FIXED al_itofix(FORCE_DIVISOR)
// SPIN_MAX is the maximum spin for procs and groups.
#define SPIN_MAX 64
#define SPIN_MAX_FIXED al_itofix(2)
#define NEGATIVE_SPIN_MAX_FIXED al_itofix(-2)
// al_itofix(2) is AFX_ANGLE_128. ANGLE_128 is 64.

// need to implement GROUP_MAX_DISTANCE properly! Currently it's only applied in f_load.c
#define GROUP_MAX_DISTANCE al_itofix(600)

#define NODE_DISRUPT_TIME_CHANGE 16

// I think 33 is correct
#define TICKS_TO_SECONDS_DIVISOR 33

struct source_edit_struct
{
 int active;
 int type; // SOURCEEDIT_TYPE_SOURCE etc
 int player_index; // index of player this source file belongs to
 int template_index; // template this source file belongs to
 int saved; // is 0 if the source has been modified since last saved (or if it's a new source that hasn't been saved)
// struct source_struct source;

 int from_a_file;
 char src_file_name [FILE_NAME_LENGTH];
 char src_file_path [FILE_PATH_LENGTH];

// The following values are relevant to source code source_edit_structs:

 char text [SOURCE_TEXT_LINES] [SOURCE_TEXT_LINE_LENGTH]; // note that these lines may not be in the correct order. Need to use line_index to work out order.
//  *** text array must be the same as in source_struct (as the code that converts bcode to source code assumes that it can treat source.text in the same way as source_edit.text)

 int line_index [SOURCE_TEXT_LINES]; // this lists lines of text from the text array in the order they appear in in the source.
// int src_file [SOURCE_TEXT_LINES]; // stores the index of the file that the line came from

 int source_colour [SOURCE_TEXT_LINES] [SOURCE_TEXT_LINE_LENGTH]; // contains colour information for syntax highlighting
 int comment_line [SOURCE_TEXT_LINES]; // is 1 if the line starts within a multi-line comment

 int cursor_line;
 int cursor_pos;
 int window_line;
 int window_pos;
 int cursor_base; // used to preserve cursor position when moving up and down through code with lines of different lengths

 int selected;
 int select_fix_line;
 int select_fix_pos;
 int select_free_line;
 int select_free_pos;

// The following values are relevant to binary source_edit_structs:
// struct bcode_struct bcode;

};



enum
{
OBJECT_TYPE_NONE,
OBJECT_TYPE_UPLINK,
OBJECT_TYPE_DOWNLINK,
OBJECT_TYPE_MOVE,
OBJECT_TYPE_PULSE,
OBJECT_TYPE_PULSE_L,
OBJECT_TYPE_PULSE_XL,
OBJECT_TYPE_BURST,
OBJECT_TYPE_BURST_L,
OBJECT_TYPE_BURST_XL,
OBJECT_TYPE_BUILD,
OBJECT_TYPE_INTERFACE,
//OBJECT_TYPE_INTERFACE_DEPTH,
//OBJECT_TYPE_INTERFACE_STABILITY,
//OBJECT_TYPE_INTERFACE_RESPONSE,
OBJECT_TYPE_HARVEST,
OBJECT_TYPE_STORAGE,
OBJECT_TYPE_ALLOCATE,
OBJECT_TYPE_STREAM,
OBJECT_TYPE_STREAM_DIR,
OBJECT_TYPE_SPIKE,
OBJECT_TYPE_REPAIR,
OBJECT_TYPE_REPAIR_OTHER,

OBJECT_TYPE_ULTRA,
OBJECT_TYPE_ULTRA_DIR,
OBJECT_TYPE_SLICE,
OBJECT_TYPE_STABILITY,
OBJECT_TYPES
};
/*
struct template_object_struct
{
	int type;
	int angle_angle;
	al_fixed angle;
	int value;
};
typedef struct template_object_struct template_object_struct;*/





#endif
// don't put anything after this








































