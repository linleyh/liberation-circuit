
#include <allegro5/allegro.h>


#include <stdio.h>
#include <string.h>
#include <math.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"

#include "g_misc.h"

#include "c_header.h"

#include "e_header.h"
#include "e_inter.h"
#include "e_help.h"
#include "e_editor.h"
#include "e_clip.h"
#include "i_input.h"
#include "i_view.h"
#include "m_input.h"
#include "e_log.h"
#include "m_maths.h"

#include "p_panels.h"

#include "d_draw.h"
#include "d_design.h"
#include "d_geo.h"
#include "d_code.h"
#include "d_code_header.h"

#include "g_shapes.h"
#include "t_template.h"

#include "c_header.h"
#include "c_keywords.h"

extern struct object_type_struct otype [OBJECT_TYPES];
extern struct editorstruct editor;
extern struct identifierstruct identifier [IDENTIFIERS];
extern struct design_window_struct dwindow;
extern struct nshape_struct nshape [NSHAPES];

//extern template templ [PLAYERS] [TEMPLATES_PER_PLAYER];


char auto_class_name [AUTO_CLASSES] [AUTO_CLASS_NAME_LENGTH] =
{
	"auto_move",
	"auto_retro",
	"auto_att_main",
	"auto_att_fwd",
	"auto_att_left",
	"auto_att_right",
	"auto_att_back",
	"auto_att_spike",
	"auto_harvest",
	"auto_allocate",
	"auto_stability",
//	"auto_build"

};


struct dcode_state_struct dcode_state;

static void generate_dcode(void);

static void work_out_immobile_automodes(void);
static void work_out_mobile_automodes(void);
static void work_out_dcode_main_attack_type(void);
static void add_main_attacking_code(int attack_or_attack_found);
//static void add_dir_attack_code(const char* flag_text, const char* target_index_text, const char* class_name_text, const char* angle_text);
static void add_main_attacking_code_specific(void);
static void add_main_attacking_code_spike_default(int target_visible);
static void add_target_lost_attacking_code(int attack_or_attack_found);

static void dcode_add_line(const char* add_line);

// currently this assumes that autocode will not try to add strings that are too long.
static void dcode_add_string(const char* add_string);
//static void dcode_add_number(int num);
static void dcode_newline(void);
//static void dcode_open_brace(void);
//static void dcode_close_brace(const char* add_text);


int dcode_error(const char* error_message);
void dcode_warning(const char* warning_message);

//static int write_dcode_keyword(int keyword_index);



/*
first check for empty source edit

clear all classes

generate process header, which will set out the classes.
 - it should also fill in:
  - non-class objects that the code generator should know about
  - basic information e.g. mobility

Now work out which modes the process is capable of.

Immobile processes:
 - should probably only accept commands for specific objects
  - in particular:
   - build
   - dir attack (as player may want to direct specific attacks)
	- probably doesn't need modes.
	 - although do we want a way of auto-transferring commands to new processes? Maybe.

Mobile:
accepts:
 move
 a_move
 attack
 harvest (r-click on data well)
 follow (r-click on friendly)
  - if used with process with harvest selected, on process with allocator, turns into give (probably need to do this in game code as processes may not be near each other)
	build
	 - if out of range, process should move there.





*/

// generates code for the present template.
// source code must be completely empty (unlike process header generation - see d_code_header.c)
// can make some guesses about how the process should process commands, although it's not very smart yet.
int autocode(int autocode_type)
{

 reset_log();

 write_line_to_log("Generating source code for process in current template.", MLOG_COL_COMPILER);

 struct template_struct* templ = dwindow.templ;
	int i, j, k;

	if (editor.current_source_edit_index == -1)
		return 0;

	dcode_state.ses = &editor.source_edit [editor.current_source_edit_index];
	dcode_state.autocode_type = autocode_type;

 clear_source_edit_text(dcode_state.ses);

	dcode_state.mobile = 1; // can be set to 0 later
	dcode_state.main_attack_type = MAIN_ATTACK_NONE;

	for (i = 0; i < AUTOMODES; i ++)
	{
		dcode_state.automode [i] = 0;
	}
/*
 for (i = 0; i < SOURCE_TEXT_LINES; i ++)
 {
		if (dcode_state.ses->text [dcode_state.ses->line_index [i]] [0] != '\0')
	 {
		 return dcode_error("template's source file must be empty.");
// TO DO: find and remove lines that only consist of spaces?
	 }
 }*/

// This clears the class list, but does not clear the objects' class data
	for (i = 0; i < OBJECT_CLASSES; i ++)
	{
  templ->object_class_name [i] [0] = '\0';
  templ->object_class_active [i] = 0;
  for (j = 0; j < OBJECT_CLASS_SIZE; j ++)
		{
   templ->object_class_member [i] [j] = -1;
   templ->object_class_object [i] [j] = -1;
		}
	}

// Now remove all classes from objects:
 for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (templ->member[i].exists)
		{
			for (j = 0; j < MAX_OBJECTS; j ++)
			{
				for (k = 0; k < CLASSES_PER_OBJECT; k ++)
				{
				 templ->member[i].object[j].object_class [k] = -1;
				}
			}
		}
	}

 if (!write_design_structure_to_source_edit(1)) // 1 means source_edit and template class list have been verified as empty
		return 0;

	if (templ->member[0].shape < FIRST_MOBILE_NSHAPE
		|| dcode_state.object_type_present [OBJECT_TYPE_MOVE] == 0)
	{
		dcode_state.mobile = 0;
 	work_out_immobile_automodes();
	}
	 else
 	 work_out_mobile_automodes();



// don't reset dcode_state cursor position - write_design_structure_to_source_edit() should have left it at end of process header
//	dcode_state.source_line = 0;
//	dcode_state.cursor_pos = 0;
	dcode_state.indent_level = 0; // do reset this, though

 dcode_newline();

 generate_dcode();

// Update syntax highlighting
// should probably skip over the process header here as it would already have been highlighted
 update_source_lines(dcode_state.ses, 0, dcode_state.source_line + 3);

 write_line_to_log("Autocode complete.", MLOG_COL_COMPILER);

 return 1;

}


// is called for static cores and also if for some reason the process has no move objects.
static void work_out_immobile_automodes(void)
{

	write_line_to_log("- process does not accept move commands (no move objects)", MLOG_COL_TEMPLATE);

	dcode_state.automode [AUTOMODE_IDLE] = 1;

// currently the only mode an immobile process can have is idle.
// it can accept commands, but they don't change its mode.


}

// assumes that:
//  - core is non-static
//  - process has move objects capable of moving it forwards and turning
//    (consider issuing a warning if this isn't true - would need to calculate various things though)
static void work_out_mobile_automodes(void)
{

	dcode_state.automode [AUTOMODE_IDLE] = 1;

	dcode_state.automode [AUTOMODE_MOVE] = 1;
	dcode_state.automode [AUTOMODE_GUARD] = 1;
//	write_line_to_log("- process accepts move commands", MLOG_COL_TEMPLATE);
//	write_line_to_log("- process accepts guard commands", MLOG_COL_TEMPLATE);

	if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_MAIN]
		|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR]
		|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
	{
// should this also accept sidewards attack objects?
		dcode_state.automode [AUTOMODE_ATTACK] = 1;
		dcode_state.automode [AUTOMODE_ATTACK_FOUND] = 1;
//		write_line_to_log("- process accepts attack commands", MLOG_COL_TEMPLATE);
		work_out_dcode_main_attack_type();
	}
	 else
		{
// 		write_line_to_log("- process doesn't accept attack commands (no forwards attacking objects)", MLOG_COL_TEMPLATE);
// dcode_state.main_attack_type is left as MAIN_ATTACK_NONE
		}

	if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_HARVEST])
	{
		dcode_state.automode [AUTOMODE_HARVEST] = 1;
		dcode_state.automode [AUTOMODE_HARVEST_RETURN] = 1;
 	if (!dcode_state.object_type_present [OBJECT_TYPE_STORAGE])
		 write_line_to_log("- Warning: process has harvest object but no storage object", MLOG_COL_WARNING);
//		 write_line_to_log("- process accepts harvest commands", MLOG_COL_TEMPLATE);
//		  else
//  		 write_line_to_log("- Warning: process has harvest object but no storage object", MLOG_COL_WARNING);
	}

	if (dcode_state.object_type_present [OBJECT_TYPE_BUILD])
	{
		dcode_state.automode [AUTOMODE_MOVE_BUILD] = 1;
//		write_line_to_log("- process accepts move-to-build commands", MLOG_COL_TEMPLATE);
	}
/*
	if (dcode_state.object_type_present [OBJECT_TYPE_REPAIR_OTHER])
	{
		dcode_state.automode [AUTOMODE_MOVE_REPAIR] = 1;
		write_line_to_log("- process accepts move-to-repair commands", MLOG_COL_TEMPLATE);
	}*/

}

static void work_out_dcode_main_attack_type(void)
{

// TO DO: give user some control over this (particularly for procs that have both spike and forward attacks)

	if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_MAIN])
	{
 	dcode_state.main_attack_type = MAIN_ATTACK_INTERCEPT;
//		write_line_to_log("  attack mode: intercept", MLOG_COL_TEMPLATE);
		return;
	}

	if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
	{
 	dcode_state.main_attack_type = MAIN_ATTACK_LONG_RANGE;
		//write_line_to_log("  attack mode: long range", MLOG_COL_TEMPLATE);
		return;
	}

	dcode_state.main_attack_type = MAIN_ATTACK_APPROACH; // this is default
//	write_line_to_log("  attack mode: approach", MLOG_COL_TEMPLATE);

}

// this function works out which variables the process will need, and adds declarations and initialisation
static void generate_dcode(void)
{

 int include_check_for_build_command = 0;


	dcode_add_line("// Process AI modes (these reflect the capabilities of the process)");
	dcode_add_line("enum");
	dcode_add_line("{");
	dcode_state.indent_level ++;
	dcode_add_line("MODE_IDLE, // process isn't doing anything ongoing");
	if (dcode_state.automode [AUTOMODE_MOVE])
 	dcode_add_line("MODE_MOVE, // process is moving to target_x, target_y");
	if (dcode_state.automode [AUTOMODE_ATTACK_FOUND])
 	dcode_add_line("MODE_MOVE_ATTACK, // process is moving, but will attack anything it finds along the way");
	if (dcode_state.automode [AUTOMODE_ATTACK])
 	dcode_add_line("MODE_ATTACK, // process is attacking a target it was commanded to attack");
	if (dcode_state.automode [AUTOMODE_ATTACK_FOUND])
 	dcode_add_line("MODE_ATTACK_FOUND, // process is attacking a target it found itself");
	if (dcode_state.automode [AUTOMODE_HARVEST])
 	dcode_add_line("MODE_HARVEST, // process is harvesting data from a data well (or travelling to do so)");
	if (dcode_state.automode [AUTOMODE_HARVEST_RETURN])
 	dcode_add_line("MODE_HARVEST_RETURN, // process has harvested data and is returning to an allocator");
	if (dcode_state.automode [AUTOMODE_MOVE_BUILD])
 	dcode_add_line("MODE_MOVE_BUILD, // process is moving somewhere to build a process there");
	if (dcode_state.automode [AUTOMODE_GUARD])
 	dcode_add_line("MODE_GUARD, // process is circling a friendly process");
 if (dcode_state.autocode_type == AUTOCODE_CAUTIOUS)
 	dcode_add_line("MODE_WITHDRAW, // cautious process is retreating");

	dcode_add_line("MODES");
	dcode_state.indent_level --;
	dcode_add_line("};");

 dcode_newline();
	dcode_add_line("// Commands that the user may give the process");
	dcode_add_line("// (these are fixed and should not be changed, although not all processes accept all commands)");
	dcode_add_line("enum");
	dcode_add_line("{");
	dcode_state.indent_level ++;
	dcode_add_line("COM_NONE, // no command");
	dcode_add_line("COM_LOCATION, // user has right-clicked somewhere on the display or map");
	dcode_add_line("COM_TARGET, // user has right-clicked on an enemy process");
	dcode_add_line("COM_FRIEND, // user has right-clicked on a friendly process");
	dcode_add_line("COM_DATA_WELL // user has right-clicked on a data well");
//	dcode_add_line("COM_NUMBER // user has pressed a number key"); - number commands no longer used - replaced by control groups
	dcode_state.indent_level --;
	dcode_add_line("};");


/* if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_MAIN]
		|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR]
		|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_LEFT_DIR]
		|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_RIGHT_DIR]
		|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_BACK_DIR]
		|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT]
		|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_HARVEST]
		|| dcode_state.object_type_present [OBJECT_TYPE_REPAIR_OTHER])
	{*/
 dcode_newline();
	dcode_add_line("// Targetting information");
	dcode_add_line("// Targetting memory allows processes to track targets (enemy or friend)");
	dcode_add_line("// The following enums are used as indices in the process' targetting memory");
	dcode_add_line("enum");
	dcode_add_line("{");
	dcode_state.indent_level ++;
	dcode_add_line("TARGET_PARENT, // a newly built process starts with its builder in address 0");
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_MAIN]
		|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR]
		|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
 	dcode_add_line("TARGET_MAIN, // main target");
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR])
 	dcode_add_line("TARGET_FRONT, // target of directional forward attack");
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_LEFT_DIR])
 	dcode_add_line("TARGET_LEFT, // target of directional left attack");
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_RIGHT_DIR])
 	dcode_add_line("TARGET_RIGHT, // target of directional right attack");
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_BACK_DIR])
 	dcode_add_line("TARGET_BACK, // target of directional backwards attack");
 if (dcode_state.object_type_present [OBJECT_TYPE_BUILD])
//		&& !dcode_state.unindexed_auto_class_present [AUTO_CLASS_MOVE]) // only static builders copy movement/attack commands
 	dcode_add_line("TARGET_BUILT, // processes built by this process");
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_HARVEST])
 	dcode_add_line("TARGET_ALLOCATOR, // process that this process will return to when finished harvesting");
 if (dcode_state.automode [AUTOMODE_GUARD])
 	dcode_add_line("TARGET_GUARD, // target of guard command");
	if (dcode_state.autocode_type == AUTOCODE_CAUTIOUS)
 	dcode_add_line("TARGET_WITHDRAW, // target that the process is withdrawing from");
// if (dcode_state.object_type_present [OBJECT_TYPE_REPAIR_OTHER])
 	//dcode_add_line("TARGET_REPAIR, // process that this process is trying to repair");

#define AUTOCODE_SELF_DESTRUCT

#ifdef AUTOCODE_SELF_DESTRUCT
 if (!dcode_state.mobile)
	 dcode_add_line("TARGET_SELF_CHECK, // check against self for self-destruct commands");
#endif
	dcode_state.indent_level --;
	dcode_add_line("};");
 dcode_newline();


// now for special code that's only used by directional attack objects:
/*	if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR]
		|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_LEFT_DIR]
		|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_RIGHT_DIR]
		|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_BACK_DIR])*/
	{
/*
  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR])
		{
//  	dcode_add_line("int attacking_front; // is set to 1 if forward directional attack objects have a target");
  	dcode_add_line("int front_attack_primary; // is set to 1 if forward directional attack objects are attacking");
  	dcode_add_line(" // the primary target (e.g. target selected by command). Set to 0 if the objects are available for autonomous fire.");
		}
 	dcode_add_line("int target_component; // target component for an attack command (allows user to");
 	dcode_add_line(" // target specific components)");
*/
/*
  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_LEFT_DIR])
 	 dcode_add_line("int attacking_left; // is set to 1 if left directional attack objects have a target");
  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_RIGHT_DIR])
  	dcode_add_line("int attacking_right; // is set to 1 if right directional attack objects have a target");
  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_BACK_DIR])
 	 dcode_add_line("int attacking_back; // is set to 1 if backwards directional attack objects have a target");

  dcode_newline();
	 dcode_add_line("int other_target_x, other_target_y;");
*/
	}

//	}

 dcode_newline();
	dcode_add_line("// Variable declaration and initialisation");
	dcode_add_line("//  (note that declaration and initialisation cannot be combined)");
	dcode_add_line("//  (also, variables retain their values between execution cycles)");
	dcode_add_line("int core_x, core_y; // location of core");
	dcode_add_line("core_x = get_core_x(); // location is updated each cycle");
	dcode_add_line("core_y = get_core_y();");
 dcode_add_line("int angle; // direction process is pointing");
 dcode_add_line(" // angles are in integer degrees from 0 to 8192, with 0 being right,");
 dcode_add_line(" // 2048 down, 4096 left and 6144 (or -2048) up.");
	dcode_add_line("angle = get_core_angle(); // angle is updated each cycle");

	dcode_newline();
 dcode_add_line("int mode; // what is the process doing? (should be one of the MODE enums)");
	if (dcode_state.automode [AUTOMODE_ATTACK_FOUND])
  dcode_add_line("int saved_mode; // save the process' mode while it's attacking something it found");

 if (dcode_state.mobile)
	{
 	dcode_newline();
	 dcode_add_line("int move_x, move_y; // destination");
	 dcode_add_line("int target_x, target_y; // location of target (to attack, follow etc)");
  dcode_add_line("int circle_target; // targetting memory index of target being circled (used by the circle_around_target subroutine)");
  dcode_add_line("int circle_rotation; // direction to circle the target in (should be 1024 for clockwise, -1024 for anti-clockwise)");
  dcode_add_line("int circle_distance; // distance to maintain from the centre of the circle");
	}

	dcode_newline();

 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR])
	{
//  	dcode_add_line("int attacking_front; // is set to 1 if forward directional attack objects have a target");
 	dcode_add_line("int front_attack_primary; // is set to 1 if forward directional attack objects are attacking");
 	dcode_add_line(" // the primary target (e.g. target selected by command). Set to 0 if the objects are available for autonomous fire.");
	}
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_LEFT_DIR]
		&& dcode_state.autocode_type	== AUTOCODE_CIRCLE_ACW)
	{
 	dcode_add_line("int left_attack_primary; // is set to 1 if left directional attack objects are attacking");
 	dcode_add_line(" // the primary target (e.g. target selected by command). Set to 0 if the objects are available for autonomous fire.");
	}
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_RIGHT_DIR]
		&& dcode_state.autocode_type	== AUTOCODE_CIRCLE_CW)
	{
 	dcode_add_line("int right_attack_primary; // is set to 1 if right directional attack objects are attacking");
 	dcode_add_line(" // the primary target (e.g. target selected by command). Set to 0 if the objects are available for autonomous fire.");
	}
	dcode_add_line("int target_component; // target component for an attack command (allows user to");
	dcode_add_line(" // target specific components)");
	if (dcode_state.autocode_type == AUTOCODE_CAUTIOUS)
	{
		if (!dcode_state.unindexed_auto_class_present [AUTO_CLASS_RETRO])
	  dcode_add_line("int withdraw_angle; // angle of retreat");
  dcode_add_line("int withdraw_x, withdraw_y; // cautious process retreats here");
//	 dcode_add_line("int harass_withdraw; // counts up while attacking. After it reaches a certain value (see below), withdraw");
//  dcode_add_line("int withdraw_x, withdraw_y, withdraw_angle; // withdrawal movement information");
	}


// scan_result may not always be needed, but can't hurt to have it:
	dcode_newline();
 dcode_add_line("int scan_result; // used to hold the results of a scan of nearby processes");

#ifdef AUTOCODE_SELF_DESTRUCT
	dcode_newline();
 dcode_add_line("int self_destruct_primed; // counter for confirming self-destruct command (ctrl-right-click on self)");
#endif

 if (dcode_state.automode [AUTOMODE_HARVEST])
	{
	 dcode_newline();
 	dcode_add_line("// Harvester variables");
  dcode_add_line("int data_well_x, data_well_y; // location of data well");
  dcode_add_line("int allocator_x, allocator_y; // location of process this process will return to with data");
	}

 if (dcode_state.object_type_present [OBJECT_TYPE_BUILD])
	{
	 dcode_newline();
  dcode_add_line("// builder variables");
//  dcode_add_line("int build_multi; // indicates that user has commanded repeat build (by pressing control)");
  dcode_add_line("int build_result; // build result code (returned by build call)");
  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_MOVE])
		{
  dcode_add_line("int build_x, build_y; // build location for move-build commands");
  dcode_add_line("int build_target_angle; // used in calculating target for commands that result in MODE_MOVE_BUILD");
// build_x/y currently only used for mobile builders
		}
	}
 dcode_add_line("int initialised; // set to 1 after initialisation code below run the first time");
 dcode_newline();
 dcode_add_line("if (!initialised)");
	dcode_add_line("{");
	dcode_state.indent_level ++;
 dcode_add_line(" // initialisation code goes here (not all autocoded processes have initialisation code)");
 dcode_add_line("initialised = 1;");
 dcode_add_line("attack_mode(0); // attack objects (if present) will all fire together");
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_MOVE]
		&& dcode_state.mobile)
	{
 dcode_add_line("// move the process forward a bit to stop it obstructing the next process to be built");
 dcode_add_line("mode = MODE_MOVE;");
 dcode_add_line("move_x = core_x + cos(angle, 300);");
 dcode_add_line("move_y = core_y + sin(angle, 300);");
	}
 if (dcode_state.automode [AUTOMODE_HARVEST])
	{
 	dcode_add_line("// Harvester assumes that it was built by an allocator,");
 	dcode_add_line("//  and will return to its parent when full of harvested data.");
  dcode_add_line("target_copy(TARGET_ALLOCATOR, TARGET_PARENT); // copies parent to TARGET_ALLOCATOR");
  dcode_add_line("allocator_x = process[TARGET_ALLOCATOR].get_core_x();");
  dcode_add_line("allocator_y = process[TARGET_ALLOCATOR].get_core_y();");
	}
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_STABILITY])
	{
 	dcode_add_line("auto_stability.set_stability(1); // activate stability objects");
	}
	if (dcode_state.autocode_type == AUTOCODE_CAUTIOUS)
 	dcode_add_line("target_copy(TARGET_GUARD, TARGET_PARENT); // for cautious processes");
	dcode_state.indent_level --;
	dcode_add_line("}");

 dcode_newline();
 dcode_add_line("int verbose; // if 1, process will print various things to the console");
 dcode_newline();
 dcode_add_line("if (check_selected_single()) // returns 1 if the user has selected this process (and no other processes)");
	dcode_add_line("{");
	dcode_state.indent_level ++;
 dcode_add_line("if (!verbose) printf(\"\\nProcess selected.\");");
 dcode_add_line("verbose = 1;");
 dcode_add_line("set_debug_mode(1); // 1 means errors for this process will be printed to the console. Resets to 0 each cycle");
	dcode_state.indent_level --;
	dcode_add_line("}");
	dcode_state.indent_level ++;
	dcode_add_line("else");
	dcode_state.indent_level ++;
	dcode_add_line("verbose = 0;");
	dcode_state.indent_level --;
	dcode_state.indent_level --;

	dcode_newline();
	dcode_newline();
	dcode_add_line("// Accept commands from user");
	dcode_add_line("if (check_new_command() == 1) // returns 1 if a command has been given"); // mention command queues here?
	dcode_add_line("{");
	dcode_state.indent_level ++;
//	if (dcode_state.autocode_type == AUTOCODE_CAUTIOUS)
//	{
//	 dcode_add_line("harass_withdraw = 0; // a new command resets harassment mode to attack");
//	}
/* if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_BUILD])
	{
	dcode_add_line("build_multi = 0; // commands cancel multi-build"); - don't know why this was here.
	}*/
	dcode_add_line("switch(get_command_type()) // get_command_type() returns the type of command given");
	dcode_add_line("{");
	dcode_state.indent_level ++;
 dcode_add_line("case COM_LOCATION:");
// COM_DATA_WELL falls through to COM_LOCATION if process can't harvest (as a data well click was probably meant as a location click)
 if (!dcode_state.automode [AUTOMODE_HARVEST])
  dcode_add_line("case COM_DATA_WELL: // this process can't harvest, so treat data well commands as location commands");
	dcode_state.indent_level ++;
	if (dcode_state.automode [AUTOMODE_MOVE])
	{
	 dcode_add_line("move_x = get_command_x(); // get_command_x() and ...y() return the target location of the command");
	 dcode_add_line("move_y = get_command_y();");
	 if (dcode_state.automode [AUTOMODE_ATTACK])
		{
  dcode_add_line("if (get_command_ctrl() == 0) // returns 1 if control was pressed when the command was given");
	 dcode_add_line("{");
	 dcode_state.indent_level ++;
 	 dcode_add_line("mode = MODE_MOVE;");
 	 dcode_add_line("if (verbose) printf(\"\\nMoving.\");");
	  dcode_state.indent_level --;
	 dcode_add_line("}");
  dcode_state.indent_level ++;
   dcode_add_line("else");
 	 dcode_add_line("{");
   dcode_state.indent_level ++;
    dcode_add_line("mode = MODE_MOVE_ATTACK; // will attack anything found along the way");
  	 dcode_add_line("if (verbose) printf(\"\\nAttack-moving.\");");
    dcode_state.indent_level --;
 	 dcode_add_line("}");
   dcode_state.indent_level --;
//  dcode_state.indent_level --;
		}
		 else
			{
  	 dcode_add_line("mode = MODE_MOVE;");
  	 dcode_add_line("if (verbose) printf(\"\\nMoving.\");");
			}
// do attack-move stuff here
	}
	 else
		{
			if (!dcode_state.mobile
				&& dcode_state.object_type_present [OBJECT_TYPE_BUILD])
  	 dcode_add_line("if (verbose) printf(\"\\nLocation command will be sent to new processes.\");");
				 else
 	    dcode_add_line("if (verbose) printf(\"\\nLocation command not recognised.\");");
		}
 dcode_add_line("break;");
	dcode_state.indent_level --;

	dcode_newline();
 dcode_add_line("case COM_TARGET:");
	dcode_state.indent_level ++;
	if (dcode_state.automode [AUTOMODE_ATTACK])
	{
	 dcode_add_line("get_command_target(TARGET_MAIN); // writes the target of the command to address TARGET_MAIN in targetting memory");
	 dcode_add_line(" // (targetting memory stores the target and allows the process to examine it if it's in scanning range");
	 dcode_add_line(" //  of any friendly process)");
	 dcode_add_line("mode = MODE_ATTACK;");
	 dcode_add_line("target_x = get_command_x();");
  dcode_add_line("target_y = get_command_y();");
	 dcode_add_line("target_component = get_command_target_component(); // allows user to target a specific component");
	 dcode_add_line("if (verbose) printf(\"\\nAttacking.\");");
// do attack-attack stuff here
	}
	 else
		{
			if (!dcode_state.mobile
				&& dcode_state.object_type_present [OBJECT_TYPE_BUILD])
  	 dcode_add_line("if (verbose) printf(\"\\nTarget command will be sent to new processes.\");");
				 else
    	 dcode_add_line("if (verbose) printf(\"\\nTarget command not recognised.\");");
		}
 dcode_add_line("break;");
	dcode_state.indent_level --;
 if (dcode_state.automode [AUTOMODE_HARVEST])
	{
// if !dcode_state.automode [AUTOMODE_HARVEST] is dealt with as a location command above
 	dcode_newline();
	 dcode_add_line("case COM_DATA_WELL:");
 	dcode_state.indent_level ++;
	 dcode_add_line("data_well_x = get_command_x();");
	 dcode_add_line("data_well_y = get_command_y();");
	 dcode_add_line(" // (targetting memory stores the target and allows the process to examine it if it's in scanning range)");
	 dcode_add_line("if (mode != MODE_HARVEST_RETURN) // can reassign target data well while process returning with harvest without changing mode");
 	dcode_state.indent_level ++;
	 dcode_add_line("mode = MODE_HARVEST;");
 	dcode_state.indent_level --;
	 dcode_add_line("if (verbose) printf(\"\\nHarvesting from data well.\");");
	 dcode_add_line("break;");
 	dcode_state.indent_level --;
	}

	dcode_newline();
 dcode_add_line("case COM_FRIEND:");
	dcode_state.indent_level ++;
 if (dcode_state.automode [AUTOMODE_HARVEST])
	{
 // should probably test for whether the target has an allocator object, somehow
	 dcode_add_line("allocator_x = get_command_x();");
	 dcode_add_line("allocator_y = get_command_y();");
	 dcode_add_line("get_command_target(TARGET_ALLOCATOR); // writes the target of the command to address TARGET_ALLOCATOR in targetting memory");
#ifdef AUTOCODE_SELF_DESTRUCT
	 dcode_add_line("if (get_command_ctrl() && process[TARGET_ALLOCATOR].get_core_x() == get_core_x() && process[TARGET_ALLOCATOR].get_core_y() == get_core_y())");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
 	 dcode_add_line("if (self_destruct_primed > 0)");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
 	  dcode_add_line("printf(\"\\nTerminating.\");");
    dcode_add_line("terminate; // this causes the process to self-destruct");
   	dcode_state.indent_level --;
   dcode_add_line("}");
 	 dcode_add_line("printf(\"\\nSelf destruct primed.\");");
 	 dcode_add_line("printf(\"\\nRepeat command (ctrl-right-click self) to confirm.\");");
   dcode_add_line("self_destruct_primed = 20;");
   dcode_add_line("break;");
  	dcode_state.indent_level --;
  dcode_add_line("}");
#endif
	 dcode_add_line("if (mode != MODE_HARVEST)");
 	dcode_state.indent_level ++;
	 dcode_add_line("mode = MODE_HARVEST_RETURN;");
 	dcode_state.indent_level --;
	 dcode_add_line("if (verbose) printf(\"\\nHarvester return set.\");");
	 dcode_add_line("break;");
 	dcode_state.indent_level --;
	}
	 else
		{

/*   if (dcode_state.automode [AUTOMODE_MOVE_REPAIR])
	  {
// This has a lower priority than AUTOMODE_HARVEST because repair can always be initiated just by moving close
	   dcode_add_line("get_command_target(TARGET_REPAIR); // writes the target of the command to address TARGET_REPAIR in targetting memory");
	   dcode_add_line(" // (targetting memory stores the target and allows the process to examine it if it's in scanning range)");
	   dcode_add_line("mode = MODE_MOVE_REPAIR;");
	   dcode_add_line("if (verbose) printf(\"\\nRepairing.\");");
    dcode_add_line("break;");
	   dcode_state.indent_level --;
	  }*/
   if (dcode_state.automode [AUTOMODE_GUARD])
	  {
	   dcode_add_line("get_command_target(TARGET_GUARD); // writes the target of the command to address TARGET_GUARD in targetting memory");
	   dcode_add_line(" // (targetting memory stores the target and allows the process to examine it if it's in scanning range)");
#ifdef AUTOCODE_SELF_DESTRUCT
	 dcode_add_line("if (get_command_ctrl() && process[TARGET_GUARD].get_core_x() == get_core_x() && process[TARGET_GUARD].get_core_y() == get_core_y())");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
 	 dcode_add_line("if (self_destruct_primed > 0)");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
 	  dcode_add_line("printf(\"\\nTerminating.\");");
    dcode_add_line("terminate; // this causes the process to self-destruct");
   	dcode_state.indent_level --;
   dcode_add_line("}");
 	 dcode_add_line("printf(\"\\nSelf destruct primed.\");");
 	 dcode_add_line("printf(\"\\nRepeat command (ctrl-right-click self) to confirm.\");");
   dcode_add_line("self_destruct_primed = 20;");
   dcode_add_line("break;");
  	dcode_state.indent_level --;
  dcode_add_line("}");
#endif
	   dcode_add_line("mode = MODE_GUARD;");
	   dcode_add_line("if (verbose) printf(\"\\nGuarding.\");");
    dcode_add_line("break;");
	   dcode_state.indent_level --;
	  }
	   else
				{

#ifdef AUTOCODE_SELF_DESTRUCT
  dcode_add_line("get_command_target(TARGET_SELF_CHECK); // writes the target of the command to address TARGET_SELF_CHECK in targetting memory");
	 dcode_add_line("if (get_command_ctrl() && process[TARGET_SELF_CHECK].get_core_x() == get_core_x() && process[TARGET_SELF_CHECK].get_core_y() == get_core_y())");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
 	 dcode_add_line("if (self_destruct_primed > 0)");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
 	  dcode_add_line("printf(\"\\nTerminating.\");");
    dcode_add_line("terminate; // this causes the process to self-destruct");
   	dcode_state.indent_level --;
   dcode_add_line("}");
 	 dcode_add_line("printf(\"\\nSelf destruct primed.\");");
 	 dcode_add_line("printf(\"\\nRepeat command (ctrl-right-click self) to confirm.\");");
   dcode_add_line("self_destruct_primed = 20;");
   dcode_add_line("break;");
  	dcode_state.indent_level --;
  dcode_add_line("}");
#endif

			  if (!dcode_state.mobile
				  && dcode_state.object_type_present [OBJECT_TYPE_BUILD])
				 {
  	   dcode_add_line("if (verbose) printf(\"\\nFriendly target command will be sent to new processes.\");");
				 }
			  else
                            {
                              if (!dcode_state.mobile && dcode_state.object_type_present [OBJECT_TYPE_HARVEST])
                                {
                                  dcode_add_line("get_command_target(TARGET_ALLOCATOR); // writes the target of the command to address TARGET_ALLOCATOR in targetting memory");
                                  dcode_add_line("if (verbose) printf(\"\\nResource recipient set.\");");
                                }
                              else
                                {
                                    dcode_add_line("if (verbose) printf(\"\\nFriendly target command not recognised.\");");
                                }
                            }

      dcode_add_line("break;");
      dcode_state.indent_level --;
				}
		}
 	dcode_newline();
  dcode_add_line("default:");
 	dcode_state.indent_level ++;
   dcode_add_line("if (verbose) printf(\"\\nUnrecognised command.\");");
   dcode_add_line("break;");
 	dcode_state.indent_level --;
	dcode_state.indent_level --;
	dcode_newline();
	dcode_add_line("} // end of command type switch");
	dcode_state.indent_level --;
	dcode_add_line("} // end of new command code");

	dcode_state.indent_level = 0;

#ifdef AUTOCODE_SELF_DESTRUCT
 	dcode_newline();
 	dcode_newline();
	 dcode_add_line("if (self_destruct_primed > 0)");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
 	 dcode_add_line("self_destruct_primed --;");
 	 dcode_add_line("if (self_destruct_primed == 0");
 	 dcode_add_line(" && verbose)");
 	 dcode_state.indent_level ++;
 	  dcode_add_line("printf(\"\\nSelf destruct cancelled.\");");
   	dcode_state.indent_level --;
  	dcode_state.indent_level --;
  dcode_add_line("}");
#endif

// Now do special code for static builders (mobile builders are a bit more complicated as they may need to move to the build location)
		if (dcode_state.object_type_present [OBJECT_TYPE_BUILD]
 	&& !dcode_state.unindexed_auto_class_present [AUTO_CLASS_MOVE])
	{
 	dcode_newline();
	 dcode_newline();

  dcode_add_line("// Now try to build a new process from the build queue.");
  dcode_add_line("// This only does anything if a build command for this process is at the front of the queue.");
  dcode_add_line("// Otherwise, it will just fail.");
  dcode_add_line("if (build_from_queue(TARGET_BUILT) == 1) // returns 1 on success");
  dcode_add_line("  copy_commands(TARGET_BUILT); // copies builder's command queue to newly built process");

/*
  dcode_add_line("if (check_new_build_command() == 1) // build commands are separate from move/attack commands");
 	dcode_add_line("{");
 	dcode_state.indent_level ++;
// pressing control while giving a build command is treated as a repeat build command.
// we can't use build_as_commanded() for this because repeat builds are likely to need some location variation (as the things built might not move)
   dcode_add_line("if (get_build_command_ctrl() == 0) // returns 1 if control was pressed at the last point of giving the build command");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
  	 dcode_add_line("build_multi = 0; // just build one");
//    dcode_add_line("build_result = auto_build.build_as_commanded(); // build_as_commanded automatically executes the build command");
    dcode_add_line("if (verbose) printf(\"\\nBuilding.\");");
 	  dcode_state.indent_level --;
   dcode_add_line("}");
	  dcode_state.indent_level ++;
	   dcode_add_line("else");
	   dcode_add_line("{");
	   dcode_state.indent_level ++;
	    dcode_add_line("build_multi = 1; // build repeatedly");
//     dcode_add_line("build_result = auto_build.build_as_commanded(); // build_as_commanded automatically executes the build command");
     dcode_add_line("if (verbose) printf(\"\\nRepeat building.\");");
     dcode_state.indent_level --;
    dcode_add_line("}");
    dcode_state.indent_level --;

   if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_MOVE])
	  {
   dcode_newline();
   dcode_add_line("build_x = get_build_command_x();");
   dcode_add_line("build_y = get_build_command_y();");
   dcode_add_line("if (check_build_range(build_x, build_y) == 0) // 0 means the build location is out of range");
   dcode_add_line("{");
   dcode_state.indent_level ++;
    dcode_add_line("mode = MODE_MOVE_BUILD;");
    dcode_add_line("// let's move to somewhere near the build location, but not right on it (or this process might get in the way)");
    dcode_add_line("build_target_angle = atan2(build_y - core_y, build_x - core_x);");
    dcode_add_line("move_x = build_x - cos(build_target_angle, 300); // stop short of target");
    dcode_add_line("move_y = build_y - sin(build_target_angle, 300);");
//    dcode_add_line("clear_all_commands(); // clears the command queue (move-build commands cannot currently be queued)");
    dcode_add_line("if (verbose) printf(\"\\nMoving to build location.\");");
    dcode_state.indent_level --;
   dcode_add_line("} // end if (check_build_range(build_x, build_y) == 0)");
	  }

   dcode_state.indent_level --;
  dcode_add_line("} // end if (check_new_build_command() == 1)");

  dcode_newline();
  dcode_add_line("if (check_build_command() == 1) // check for queued build commands");
  dcode_add_line("{");
  dcode_state.indent_level ++;
   dcode_add_line("build_result = build_as_commanded(TARGET_BUILT); // build_as_commanded automatically executes the build command");
   dcode_add_line("if (build_result == 1) // 1 means success"); // need to add readable tokens for numbers like this
   dcode_add_line("{");
   dcode_state.indent_level ++;
    if (!dcode_state.unindexed_auto_class_present [AUTO_CLASS_MOVE]) // only static builders copy movement/attack commands like this
     dcode_add_line("copy_commands(TARGET_BUILT); // copies builder's command queue to newly built process");
    dcode_add_line("if (build_multi == 0)");
    dcode_state.indent_level ++;
     dcode_add_line("clear_build_command(); // only clear the command if not multi-building");
     dcode_state.indent_level --;
    dcode_add_line("if (verbose) printf(\"\\nProcess built.\");");
    dcode_state.indent_level --;
   dcode_add_line("} // end if (build_result == 1)");
   dcode_state.indent_level --;
  dcode_add_line("} // end if (check_build_command() == 1)");
*/
 	dcode_newline();
	 dcode_newline();

	} // end special code for processes with build class


/*

non-mobile:

when new build command received:
 - check for control; set multi 0 or 1

when build command on queue
 - try to build.
  - if successful and multi==1, do nothing
  - if successful and multi==0, clear_build_command
  - if unsuccessful, do nothing

mobile

when new build command received:
 - check for control; set multi 0 or 1
 - check range. If out of range, set move-build

when build command on queue
 - same as non-mobile


*/

/*
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_BUILD])
	{
 	dcode_newline();
	 dcode_newline();
  dcode_add_line("if (check_new_build_command() == 1) // build commands are separate from move/attack commands");
 	dcode_add_line("{");
 	dcode_state.indent_level ++;
// pressing control while giving a build command is treated as a repeat build command.
// we can't use build_as_commanded() for this because repeat builds are likely to need some location variation (as the things built might not move)
 	 dcode_add_line("build_multi = 0; // this is set to 1 below if a repeat build command is received");
   dcode_add_line("if (get_build_command_ctrl() == 0) // returns 1 if control was pressed at the last point of giving the build command");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
    dcode_add_line("build_result = auto_build.build_as_commanded(); // build_as_commanded automatically executes the build command");
    dcode_add_line("if (verbose) printf(\"\\nBuilding.\");");
 	  dcode_state.indent_level --;
   dcode_add_line("}");
	  dcode_state.indent_level ++;
	   dcode_add_line("else");
	   dcode_add_line("{");
	   dcode_state.indent_level ++;
	    dcode_add_line("build_multi = 2;");
     dcode_add_line("build_result = auto_build.build_as_commanded(); // build_as_commanded automatically executes the build command");
     dcode_add_line("if (verbose) printf(\"\\nRepeat building.\");");
     dcode_state.indent_level --;
    dcode_add_line("} // end if (get_build_command_ctrl() == 0)");
    dcode_state.indent_level --;

   if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_MOVE])
	  {
   dcode_newline();
   dcode_add_line("if (build_result == -9) // -9 means the build location is out of range"); // need to add readable tokens for numbers like this
   dcode_add_line("{");
   dcode_state.indent_level ++;
    dcode_add_line("mode = MODE_MOVE_BUILD;");
    dcode_add_line("build_x = get_build_command_x();");
    dcode_add_line("build_y = get_build_command_y();");
    dcode_add_line("// let's move to somewhere near the build location, but not right on it (or this process might get in the way)");
    dcode_add_line("build_target_angle = atan2(build_y - core_y, build_x - core_x);");
    dcode_add_line("move_x = build_x - cos(build_target_angle, 300);");
    dcode_add_line("move_y = build_y - sin(build_target_angle, 300);");
    dcode_add_line("clear_all_commands(); // clears the command queue (move-build commands cannot currently be queued)");
    dcode_add_line("if (verbose) printf(\"\\nMoving to build location.\");");
    dcode_state.indent_level --;
   dcode_add_line("} // end if (build_result == -9)");
	  }
   dcode_newline();
   dcode_add_line("if (build_result == 1) // 1 means success"); // need to add readable tokens for numbers like this
   dcode_add_line("{");
   dcode_state.indent_level ++;
    dcode_add_line("clear_build_command();");
    dcode_state.indent_level --;
   dcode_add_line("} // end if (build_result == 1)");

  dcode_state.indent_level --;
  dcode_add_line("} // end if (check_new_build_command() == 1)");
*/


/*
  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_MOVE])
  {
 	dcode_add_line("if (mode != MODE_MOVE_BUILD) // MODE_MOVE_BUILD suppresses multi-build while moving");
 	dcode_add_line("{");
 	dcode_state.indent_level ++;
  }
 	dcode_add_line("if (build_multi == 1)");
 	dcode_add_line("{");
 	dcode_state.indent_level ++;
	  dcode_add_line("auto_build.build_retry(); // if this fails because of a collision or similar, the next retry will be moved slightly");
	  dcode_state.indent_level --;
 	dcode_add_line("} // end if (build_multi == 1)");
  dcode_state.indent_level ++;
   dcode_add_line("else");
   dcode_add_line("{");
   dcode_state.indent_level ++;
    dcode_add_line("if (build_multi == 2) // repeat build command just given");
    dcode_state.indent_level ++;
     dcode_add_line("build_multi = 1;");
     dcode_state.indent_level --;
    dcode_state.indent_level --;
   dcode_add_line("} // end else");
   dcode_state.indent_level --;
  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_MOVE])
  {
 	dcode_state.indent_level --;
 	dcode_add_line("} // end if (mode != MODE_MOVE_BUILD)");
  }
*/


/*
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_BUILD])
	{
 	dcode_newline();
	 dcode_newline();
  dcode_add_line("if (check_new_build_command() == 1) // build commands are separate from move/attack commands");
 	dcode_add_line("{");
 	dcode_state.indent_level ++;
// pressing control while giving a build command is treated as a repeat build command.
// we can't use build_as_commanded() for this because repeat builds are likely to need some location variation (as the things built might not move)
 	 dcode_add_line("build_multi = 0; // this is set to 1 below if a repeat build command is received");
   dcode_add_line("if (get_build_command_ctrl() == 0) // returns 1 if control was pressed at the last point of giving the build command");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
    dcode_add_line("auto_build.build_as_commanded(); // build_as_commanded automatically executes the build command that the user gave");
 	  dcode_state.indent_level --;
   dcode_add_line("}");
	  dcode_state.indent_level ++;
	   dcode_add_line("else");
	   dcode_add_line("{");
	   dcode_state.indent_level ++;
	    dcode_add_line("build_multi = 2;");
     dcode_add_line("auto_build.build_as_commanded(); // build_as_commanded automatically executes the build command that the user gave");
     dcode_state.indent_level --;
    dcode_add_line("} // end if (get_build_command_ctrl() == 0)");
    dcode_state.indent_level --;
   dcode_state.indent_level --;
  dcode_add_line("} // end if (check_new_build_command() == 1)");

 	dcode_newline();
	 dcode_newline();

 	dcode_add_line("if (build_multi == 1)");
 	dcode_add_line("{");
 	dcode_state.indent_level ++;
	  dcode_add_line("auto_build.build_retry(); // if this fails because of a collision or similar, the next retry will be at a different location");
	  dcode_state.indent_level --;
 	dcode_add_line("} // end if (build_multi == 1)");
  dcode_state.indent_level ++;
   dcode_add_line("else");
   dcode_add_line("{");
   dcode_state.indent_level ++;
    dcode_add_line("if (build_multi == 2) // repeat build command just given");
    dcode_state.indent_level ++;
     dcode_add_line("build_multi = 1;");
     dcode_state.indent_level --;
    dcode_state.indent_level --;
   dcode_add_line("} // end else");
   dcode_state.indent_level --;
	}
*/

	dcode_state.indent_level = 0;

// should probably assume that an immobile process with a harvest object is supposed to use it:
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_HARVEST]
		&& dcode_state.mobile == 0)
	{
 	dcode_newline();
	 dcode_newline();
        if (! dcode_state.unindexed_auto_class_present [AUTO_CLASS_ALLOCATE])
          dcode_add_line("auto_harvest.give_data(TARGET_ALLOCATOR, 32);");
 	dcode_add_line("auto_harvest.gather_data();");
	}

 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ALLOCATE])
	{
 	dcode_newline();
	 dcode_newline();
 	dcode_add_line("auto_allocate.allocate_data(4); // actually I think the maximum is 2");
	}

	int primary_attack_spaces = 0;

 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR])
	{
 	dcode_newline();
	 dcode_newline();
	 primary_attack_spaces = 1;
  dcode_add_line("front_attack_primary = 0; // this will be set to 1 if front attack is attacking the main target");
	}
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_LEFT_DIR]
		&& dcode_state.autocode_type == AUTOCODE_CIRCLE_ACW)
	{
		if (!primary_attack_spaces)
		{
 	 dcode_newline();
	  dcode_newline();
	  primary_attack_spaces = 1;
		}
  dcode_add_line("left_attack_primary = 0; // this will be set to 1 if left attack is attacking the main target");
	}
 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_RIGHT_DIR]
		&& dcode_state.autocode_type == AUTOCODE_CIRCLE_CW)
	{
		if (!primary_attack_spaces)
		{
 	 dcode_newline();
	  dcode_newline();
	  primary_attack_spaces = 1;
		}
  dcode_add_line("right_attack_primary = 0; // this will be set to 1 if right attack is attacking the main target");
	}

	dcode_state.indent_level = 0;

	dcode_newline();
	dcode_newline();
	dcode_add_line("// What the process does next depends on its current mode");
	dcode_add_line("switch(mode)");
	dcode_add_line("{");
	dcode_state.indent_level ++;
 if (dcode_state.automode [AUTOMODE_IDLE]) // currently this is always true
	{
  dcode_newline();
 	dcode_add_line("case MODE_IDLE:");
 	dcode_state.indent_level ++;
 	if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_MOVE])
  	dcode_add_line("auto_move.set_power(0); // turn off all objects in the move class");
// mobile builders will check for build commands and move to the target location:

			if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_MOVE]
 			&& dcode_state.object_type_present [OBJECT_TYPE_BUILD])
	  {
//   dcode_newline();
    dcode_add_line("gosub check_for_build_command; // this jumps to the check_for_build_command subroutine, then jumps back on return.");
    dcode_add_line("// (subroutines are at the end of the source code, below)");
    include_check_for_build_command = 1;
	  }
// processes capable of attacking will scan for threats while idle
//  - not sure what to do about processes that are mobile builder processes and also have attack objects.
 	if (dcode_state.automode [AUTOMODE_ATTACK_FOUND])
		{
  	dcode_add_line("// now check for nearby hostile processes");
  	dcode_add_line("scan_result = scan_for_threat(0, 0, TARGET_MAIN); // scan_for_threat finds the hostile process nearest to the scan centre,");
  	dcode_add_line(" // and saves it in the process' targetting memory.");
  	dcode_add_line(" // (parameters are: (x offset of scan centre from core, y offset, targetting memory address))");
  	dcode_add_line("if (scan_result != 0)");
  	dcode_add_line("{");
 	 dcode_state.indent_level ++;
  	dcode_add_line("mode = MODE_ATTACK_FOUND; // later code means that process will attack target in targetting memory 0");
 	 dcode_add_line("target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN");
   dcode_add_line("target_y = process[TARGET_MAIN].get_core_y();");
 	 dcode_add_line("target_component = 0; // attack the core");
  	dcode_add_line("saved_mode = MODE_IDLE; // when leaving MODE_ATTACK_FOUND, will return to this mode");
//  	if (dcode_state.autocode_type == AUTOCODE_HARASS)
//    dcode_add_line("harass_withdraw = 0; // finding a new target resets harassment to attack mode");
   dcode_add_line("if (verbose) printf(\"\\nTarget found; attacking.\");");
 	 dcode_state.indent_level --;
  	dcode_add_line("}");
// 	 dcode_state.indent_level --;
		}
 	dcode_add_line("break;");
 	dcode_state.indent_level --;
	}
 if (dcode_state.automode [AUTOMODE_ATTACK]
		&& dcode_state.automode [AUTOMODE_MOVE]
		&& dcode_state.automode [AUTOMODE_ATTACK_FOUND])
	{
   dcode_newline();
  	dcode_add_line("case MODE_MOVE_ATTACK:");
  	dcode_add_line("// check for nearby hostile processes");
  	dcode_state.indent_level ++;
  	 dcode_add_line("scan_result = scan_for_threat(0, 0, TARGET_MAIN); // scan_for_threat finds the hostile process nearest to the scan centre,");
  	 dcode_add_line(" // and saves it in the process' targetting memory.");
  	 dcode_add_line(" // (parameters are: (x offset of scan centre from core, y offset, targetting memory address))");
  	 dcode_add_line("if (scan_result != 0)");
  	 dcode_add_line("{");
 	  dcode_state.indent_level ++;
  	  dcode_add_line("mode = MODE_ATTACK_FOUND; // later code means that process will attack target in targetting memory 0");
   	 dcode_add_line("target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN");
 	   dcode_add_line("target_y = process[TARGET_MAIN].get_core_y();");
 	   dcode_add_line("target_component = 0; // attack the core");
  	  dcode_add_line("saved_mode = MODE_MOVE_ATTACK; // when leaving MODE_ATTACK_FOUND, will return to this mode");
     dcode_add_line("if (verbose) printf(\"\\nTarget found - attacking.\");");
//  	  if (dcode_state.autocode_type == AUTOCODE_HARASS)
//      dcode_add_line("harass_withdraw = 0; // finding a new target resets harassment to attack mode");
  	  dcode_add_line("break;");
 	   dcode_state.indent_level --;
  	 dcode_add_line("}");
 	  dcode_state.indent_level --;
  	dcode_add_line("// fall through to MODE_MOVE case...");

	}

	if (dcode_state.automode [AUTOMODE_MOVE])
	{
  dcode_newline();
 	dcode_add_line("case MODE_MOVE:");
 	dcode_add_line("// stop moving when within 255 pixels of target (can change to higher or lower values if needed)");
 	dcode_state.indent_level ++;
/*  dcode_add_line("if (core_x > move_x - 255");
  dcode_add_line(" && core_x < move_x + 255");
  dcode_add_line(" && core_y > move_y - 255");
  dcode_add_line(" && core_y < move_y + 255)");*/
  dcode_add_line("if (distance_from_xy_less(move_x, move_y, 255))");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
  dcode_add_line("clear_command(); // cancels the current command. If there's a queue of commands (e.g. shift-move waypoints)");
  dcode_add_line(" //  this moves the queue forward so that check_new_command() will return 1 next cycle.");
  if (dcode_state.automode [AUTOMODE_HARVEST])
		{
  dcode_add_line("// Now, a command to move somewhere near a data well was probably meant as a harvest command:");
  dcode_add_line("if (get_data_stored() < get_data_capacity()");
  dcode_add_line(" && search_for_well())");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
   dcode_add_line("mode = MODE_HARVEST;");
   dcode_add_line("data_well_x = get_well_x();");
   dcode_add_line("data_well_y = get_well_y();");
   dcode_add_line("if (verbose) printf(\"\\nHarvesting.\");");
  	dcode_state.indent_level --;
  dcode_add_line("}");
 	dcode_state.indent_level ++;
   dcode_add_line("else");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
    dcode_add_line("mode = MODE_IDLE;");
    dcode_add_line("if (verbose) printf(\"\\nReached destination.\");");
 	  dcode_state.indent_level --;
   dcode_add_line("}");
  	dcode_state.indent_level --;
		}
		 else
			{
  dcode_add_line("mode = MODE_IDLE;");
  dcode_add_line("if (verbose) printf(\"\\nReached destination.\");");
			}
 	dcode_state.indent_level --;
  dcode_add_line("}");
 	dcode_state.indent_level ++;
  dcode_add_line("else");
 	dcode_state.indent_level ++;
  dcode_add_line("auto_move.move_to(move_x, move_y); // calls move_to for all objects in the move class");
 	dcode_state.indent_level --;
 	dcode_state.indent_level --;
  dcode_add_line("break;");
 	dcode_state.indent_level --;
 }
	if (dcode_state.automode [AUTOMODE_ATTACK_FOUND])
	{
  dcode_newline();
 	dcode_add_line("case MODE_ATTACK_FOUND:");
 	dcode_add_line("// Attack target as long as it's visible.");
 	dcode_add_line("// If target lost or destroyed, go back to previous action.");
 	dcode_state.indent_level ++;
/* 	dcode_state.indent_level ++;
 	dcode_add_line("if (!process[TARGET_MAIN].visible()) // returns zero if the target is not visible or does not exist");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
  dcode_add_line("mode = saved_mode;");
  dcode_add_line("if (verbose) printf(\"\\nTarget not detected.\");");
  dcode_add_line("break;");
 	dcode_state.indent_level --;
  dcode_add_line("}");
 	dcode_add_line("target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN");
 	dcode_add_line("target_y = process[TARGET_MAIN].get_core_y();");*/
  add_main_attacking_code(1);
  dcode_add_line("break;");
 	dcode_state.indent_level --;
	}
	if (dcode_state.automode [AUTOMODE_ATTACK])
	{
  dcode_newline();
 	dcode_add_line("case MODE_ATTACK:");
 	dcode_add_line("// Attack target identified by user command");
 	dcode_state.indent_level ++;
// 	dcode_add_line("target_x = get_command_x(); // get_command_x() returns the target location of the current command.");
//  dcode_add_line("target_y = get_command_y();");
/* 	dcode_add_line("if (target_x == 0) // get_command_x() returns 0 if target no longer visible");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
  dcode_add_line("clear_command(); // clears the current command. If another command is queued,");
  dcode_add_line(" // it will register as a new command and can be retrieved next cycle.");
  dcode_add_line("mode = MODE_IDLE;");
  dcode_add_line("if (verbose) printf(\"\\nTarget not detected.\");");
  dcode_add_line("break;");
 	dcode_state.indent_level --;
  dcode_add_line("}");*/
  add_main_attacking_code(1);
/*
  dcode_add_line("// Now see whether the commanded target is in scan range:");
  dcode_add_line("if (process[TARGET_MAIN].get_core_x() < 0) // returns a negative number if out of scan range");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
  dcode_add_line("auto_move.move_to(target_x, target_y); // calls move_to for all objects in the move class");
 	dcode_state.indent_level --;
  dcode_add_line("}");
 	dcode_state.indent_level ++;
  dcode_add_line("else");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
  add_main_attacking_code();
 	dcode_state.indent_level --;
  dcode_add_line("}");
 	dcode_state.indent_level --;
 	dcode_state.indent_level --;
*/
 	dcode_add_line("break;");
 	dcode_state.indent_level --;
	}
 if (dcode_state.automode [AUTOMODE_HARVEST])
	{
  dcode_newline();
 	dcode_add_line("case MODE_HARVEST:");
 	dcode_state.indent_level ++;
 	dcode_add_line("if (abs(core_x - data_well_x) < 1000 && abs(core_y - data_well_y) < 1000)");
 	dcode_state.indent_level ++;
 	dcode_add_line("auto_harvest.gather_data(); // only gather if near target (avoids inadvertently gathering from another well)");
 	dcode_state.indent_level --;
 	dcode_add_line("auto_move.move_to(data_well_x, data_well_y);");
// 	dcode_add_line("if (get_data_stored() == get_data_capacity())");
  dcode_add_line("if (get_data_stored() == get_data_capacity() || get_damage() > 0)");
 	dcode_add_line("{");
 	dcode_state.indent_level ++;
 	dcode_add_line("mode = MODE_HARVEST_RETURN;");
 	dcode_add_line("if (verbose) printf(\"\\nReturning to allocator.\");");
 	dcode_state.indent_level --;
 	dcode_add_line("}");
 	dcode_add_line("break;");
 	dcode_state.indent_level --;

  dcode_newline();
 	dcode_add_line("case MODE_HARVEST_RETURN:");
 	dcode_state.indent_level ++;

 	dcode_add_line("auto_harvest.give_data(TARGET_ALLOCATOR, 100);");
 	dcode_add_line("auto_move.move_to(allocator_x, allocator_y);");
 	dcode_add_line("if (get_data_stored() == 0");
 	dcode_add_line(" && get_data_capacity() > 0) // if 0, probably means the harvester is damaged.");
 	dcode_add_line("{");
 	dcode_state.indent_level ++;
 	 dcode_add_line("if (data_well_x != 0)");
 	 dcode_add_line("{");
 	 dcode_state.indent_level ++;
 	  dcode_add_line("// if the harvester has a data well to harvest from, go back to harvest mode:");
 	  dcode_add_line("mode = MODE_HARVEST;");
 	  dcode_add_line("if (verbose) printf(\"\\nHarvesting.\");");
 	  dcode_state.indent_level --;
 	 dcode_add_line("}");
 	 dcode_state.indent_level ++;
 	  dcode_add_line("else");
  	 dcode_state.indent_level ++;
   	 dcode_add_line("{");
   	 dcode_state.indent_level ++;
 	    dcode_add_line("// if data_well_x is 0, the harvester has probably just been built and sent a command to");
 	    dcode_add_line("// guard its parent (which it may have interpreted as a command to set its harvest return target).");
 	    dcode_add_line("// if so, it should just go into guard mode:");
 	    dcode_add_line("mode = MODE_GUARD;");
 	    dcode_add_line("target_copy(TARGET_GUARD, TARGET_ALLOCATOR); // copy from TARGET_ALLOCATOR to TARGET_GUARD");
 	    dcode_add_line("if (verbose) printf(\"\\nGuarding.\");");
     	dcode_state.indent_level --;
   	 dcode_add_line("}");
    	dcode_state.indent_level --;
//  	 dcode_add_line("}");
   	dcode_state.indent_level --;
  	dcode_add_line("}");
  	dcode_state.indent_level --;
 	dcode_add_line("break;");
 	dcode_state.indent_level --;
	}

// code for mobile builder process:
	if (dcode_state.automode [AUTOMODE_MOVE]
		&& dcode_state.object_type_present [OBJECT_TYPE_BUILD])
	{
  dcode_newline();
 	dcode_add_line("case MODE_MOVE_BUILD:");
 	dcode_add_line("// stop moving when within 80 pixels of target (can change to higher or lower values if needed)");
 	dcode_state.indent_level ++;
/*  dcode_add_line("if (core_x > build_x - 750");
  dcode_add_line(" && core_x < build_x + 750");
  dcode_add_line(" && core_y > build_y - 750");
  dcode_add_line(" && core_y < build_y + 750) // build range is about 800");*/
  dcode_add_line("if (distance_from_xy_less(build_x, build_y, 800)) // build range is about 1000");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
   dcode_add_line("// now try to build (build_from_queue() will just fail if this process' build command isn't at the front of the queue)");
   dcode_add_line("if (build_from_queue(TARGET_BUILT) == 1) // build_from_queue() returns 1 on success");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
    dcode_add_line("mode = MODE_IDLE;");
 	  dcode_add_line("if (verbose) printf(\"\\nProcess built.\");");
 	  dcode_state.indent_level --;
   dcode_add_line("}");
 	 dcode_state.indent_level --;
  dcode_add_line("}");
 	dcode_state.indent_level ++;
  dcode_add_line("else");
 	dcode_state.indent_level ++;
  dcode_add_line("auto_move.move_to(move_x, move_y); // calls move_to for all objects in the move class");
 	dcode_state.indent_level --;
 	dcode_state.indent_level --;
  dcode_add_line("break;");
 	dcode_state.indent_level --;
 }

 if (dcode_state.automode [AUTOMODE_GUARD])
	{
   dcode_newline();
  	dcode_add_line("case MODE_GUARD:");
 	 dcode_add_line("// Move in circle around friendly target identified by user command");
  	if (dcode_state.automode [AUTOMODE_ATTACK_FOUND])
 	  dcode_add_line("//  and attack any enemies that come near");
 	 dcode_state.indent_level ++;
    dcode_add_line("if (process[TARGET_GUARD].visible()) // returns 1 if target visible. Always returns 1 for a friendly target, if it exists.");
    dcode_add_line("{");
	   dcode_state.indent_level ++;
   	 if (dcode_state.automode [AUTOMODE_ATTACK_FOUND])
				 {
    	dcode_add_line("// check for nearby hostile processes");
//    	dcode_state.indent_level ++;
 	   dcode_add_line("scan_result = scan_for_threat(0, 0, TARGET_MAIN); // scan_for_threat finds the hostile process nearest to the scan centre,");
 	   dcode_add_line(" // and saves it in the process' targetting memory.");
 	   dcode_add_line(" // (parameters are: (x offset of scan centre from core, y offset, targetting memory address))");
 	   dcode_add_line("if (scan_result != 0)");
 	   dcode_add_line("{");
	    dcode_state.indent_level ++;
 	    dcode_add_line("mode = MODE_ATTACK_FOUND; // later code means that process will attack target in targetting memory 0");
   	  dcode_add_line("target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN");
 	    dcode_add_line("target_y = process[TARGET_MAIN].get_core_y();");
    	 dcode_add_line("target_component = 0; // attack the core");
 	    dcode_add_line("saved_mode = MODE_GUARD; // when leaving MODE_ATTACK_FOUND, will return to this mode");
//     	if (dcode_state.autocode_type == AUTOCODE_HARASS)
//       dcode_add_line("harass_withdraw = 0; // finding a new target resets harassment to attack mode");
      dcode_add_line("if (verbose) printf(\"\\nTarget found - attacking.\");");
 	    dcode_add_line("break;");
	     dcode_state.indent_level --;
 	   dcode_add_line("}");
//	    dcode_state.indent_level --;
				 } // end of code to find own target
     dcode_add_line("// Now call the circle_around_target subroutine to make this process circle the guarded process");
     dcode_add_line("circle_target = TARGET_GUARD; // circle_target is the process at the centre of the circle (here it's the process being guarded)");
     dcode_add_line("circle_rotation = 1024; // the process will aim 1024 angle units (45 degrees) around the circle, clockwise.");
     dcode_add_line("circle_distance = 700; // the process will try to stay this far from the centre of the circle");
     dcode_add_line("gosub circle_around_target; // the circle_around_target subroutine is below, near the end of the code");

//     dcode_add_line("auto_move.move_to(process[TARGET_GUARD].get_core_x() - cos(angle_to_guard_target, 400),");
//     dcode_add_line("                  process[TARGET_GUARD].get_core_y() - sin(angle_to_guard_target, 400));");
			  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_MOVE]
 			  && dcode_state.object_type_present [OBJECT_TYPE_BUILD])
	    {
      dcode_add_line("gosub check_for_build_command; // this jumps to the check_for_build_command subroutine, then jumps back on return.");
      dcode_add_line("// (subroutines are at the end of the source code, below)");
      include_check_for_build_command = 1;
	    }
     dcode_add_line("break;");
 	   dcode_state.indent_level --;
    dcode_add_line("}");
    dcode_add_line("// guard target must have been destroyed. Go back to idle mode and check for new commands.");
    dcode_add_line("clear_command(); // cancels the current command. If there's a queue of commands (e.g. shift-move waypoints)");
    dcode_add_line(" //  this moves the queue forward so that check_new_command() will return 1 next cycle.");
    dcode_add_line("mode = MODE_IDLE;");
    dcode_add_line("if (verbose) printf(\"\\nGuard target lost.\");");
    dcode_add_line("break;");
	   dcode_state.indent_level --;
	}

	if (dcode_state.autocode_type == AUTOCODE_CAUTIOUS)
	{
   dcode_newline();
  	dcode_add_line("case MODE_WITHDRAW:");
 	 dcode_add_line("// Trying to retreat");
 	 dcode_add_line("if (get_damage())");
 	 dcode_add_line("  get_damage_source(TARGET_WITHDRAW);");
 	 if (dcode_state.object_type_present [OBJECT_TYPE_INTERFACE])
			{
 	 dcode_add_line("if (get_interface_strength() == get_interface_capacity()");
 	 dcode_add_line(" && get_total_integrity() == get_unharmed_integrity_max())");
			}
			else
			{
 	 dcode_add_line("if (get_total_integrity() == get_unharmed_integrity_max())");
			}
   dcode_add_line("{");
   dcode_state.indent_level ++;
  	 dcode_add_line("mode = saved_mode;");
  	 dcode_add_line("break;");
    dcode_state.indent_level --;
   dcode_add_line("}");
  	if (dcode_state.object_type_present [OBJECT_TYPE_REPAIR]
			 || dcode_state.object_type_present [OBJECT_TYPE_REPAIR_OTHER])
			{
 	 dcode_add_line("if (process[TARGET_WITHDRAW].visible())");
   dcode_add_line("{");
   dcode_state.indent_level ++;
  	 dcode_add_line("withdraw_x = process[TARGET_WITHDRAW].get_core_x();");
  	 dcode_add_line("withdraw_y = process[TARGET_WITHDRAW].get_core_y();");
    dcode_state.indent_level --;
   dcode_add_line("}");
   if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_RETRO])
			{
 	 dcode_add_line("auto_move.approach_xy(withdraw_x, withdraw_y, 1600);");
   if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_MAIN])
			{
//   dcode_add_line("if (process[TARGET_MAIN].distance_less(1000)");
   dcode_add_line("if (distance_from_xy_less(withdraw_x, withdraw_y, 1000))");
//   dcode_add_line(" && arc_length(angle, process[TARGET_MAIN].target_angle()) < 500)");
   dcode_state.indent_level ++;
    dcode_add_line("auto_att_main.fire(0); // tries to fire all objects in the auto_att_main class. 0 is firing delay (in ticks)");
    dcode_state.indent_level --;
   dcode_add_line(" // (attack class is for fixed attack objects that point more or less forwards)");
			} // end if AUTO_CLASS_ATTACK_MAIN
			if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
			{
//    dcode_add_line("if (process[TARGET_MAIN].distance_less(1600))");
// should this check angle as well?
//    dcode_add_line("{");
//    dcode_state.indent_level ++;
    dcode_add_line("auto_att_spike.fire_spike_xy(withdraw_x,withdraw_y); // Calls fire_spike_xy() on all spike objects in the spike class.");
//    dcode_state.indent_level --;
//    dcode_add_line("}");
			} // end if AUTO_CLASS_SPIKE
			} // end if AUTO_CLASS_RETRO
  	else
			{
 	 dcode_add_line("withdraw_angle = atan2(core_y - withdraw_y, core_x - withdraw_x);");
 	 dcode_add_line("auto_move.move_to(withdraw_x + cos(withdraw_angle, 1600), withdraw_y + sin(withdraw_angle, 1600));");
			}
			} // end if process has repair objects
			else
			{
 	 dcode_add_line("if (!process[TARGET_GUARD].visible()) // nothing to retreat to.");
   dcode_add_line("{");
   dcode_state.indent_level ++;
  	 dcode_add_line("mode = saved_mode;");
  	 dcode_add_line("break;");
    dcode_state.indent_level --;
   dcode_add_line("}");
   dcode_add_line("auto_move.approach_target(TARGET_GUARD, 0, 500);");
			}
   dcode_add_line("break;");
   dcode_state.indent_level --;




	}

/*
 if (dcode_state.automode [AUTOMODE_MOVE_REPAIR])
	{
   dcode_newline();
  	dcode_add_line("case MODE_MOVE_REPAIR:");
 	 dcode_add_line("// Repair target identified by user command");
 	 dcode_state.indent_level ++;
 	 dcode_add_line("target_x = get_command_x(); // get_command_x() returns the target location of the current command.");
   dcode_add_line("target_y = get_command_y();");
  	dcode_add_line("auto_move.approach_xy(target_x, target_y, 400);");
   dcode_add_line("restore_other(TARGET_REPAIR); // tries to restore destroyed components. Just fails if out of range.");
   dcode_add_line("repair_other(TARGET_REPAIR); // tries to repair the target. Just fails if out of range.");
 	 dcode_add_line("if (target_x == 0) // get_command_x() returns 0 if target no longer visible");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
   dcode_add_line("clear_command(); // clears the current command. If another command is queued,");
   dcode_add_line(" // it will register as a new command and can be retrieved next cycle.");
   dcode_add_line("mode = MODE_IDLE;");
   dcode_add_line("if (verbose) printf(\"\\nTarget not detected.\");");
 	 dcode_state.indent_level --;
   dcode_add_line("}");
   dcode_add_line("break;");
 	 dcode_state.indent_level --;

	}
*/

	dcode_state.indent_level --;
 dcode_newline();
 dcode_add_line("} // end of mode switch");


 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR])
	{
  dcode_newline();
// front directional attacking objects don't attack autonomously if they're attacking the process' main target
  dcode_add_line("if (front_attack_primary == 0) // is 1 if the forward attack objects are attacking the main target");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
  dcode_add_line("auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);");
 	dcode_state.indent_level --;
  dcode_add_line("}");
	}

 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_LEFT_DIR])
	{
  dcode_newline();
		if (dcode_state.autocode_type != AUTOCODE_CIRCLE_ACW)
		{
   dcode_add_line("auto_att_left.attack_scan(-2048, 400, TARGET_LEFT);");
		}
		 else
			{
    dcode_add_line("if (left_attack_primary == 0) // is 1 if the left attack objects are attacking the main target");
    dcode_add_line("{");
 	  dcode_state.indent_level ++;
    dcode_add_line("auto_att_left.attack_scan(-2048, 400, TARGET_LEFT);");
 	  dcode_state.indent_level --;
    dcode_add_line("}");
			}
	}

 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_RIGHT_DIR])
	{
  dcode_newline();
		if (dcode_state.autocode_type != AUTOCODE_CIRCLE_CW)
		{
   dcode_add_line("auto_att_right.attack_scan(2048, 400, TARGET_RIGHT);");
		}
		 else
			{
    dcode_add_line("if (right_attack_primary == 0) // is 1 if the right attack objects are attacking the main target");
    dcode_add_line("{");
 	  dcode_state.indent_level ++;
    dcode_add_line("auto_att_right.attack_scan(2048, 400, TARGET_RIGHT);");
 	  dcode_state.indent_level --;
    dcode_add_line("}");
			}
	}



 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_BACK_DIR])
	{
	 dcode_newline();
  dcode_add_line("auto_att_back.attack_scan(4096, 400, TARGET_BACK);");
/*  add_dir_attack_code("attacking_back", // flag text
																						"TARGET_BACK", // target_index_text
																						"auto_att_back", // class_name_text
																						"angle + 4096"); // angle_text*/
	}

 if (dcode_state.object_type_present [OBJECT_TYPE_INTERFACE])
	{
  dcode_newline();
  dcode_add_line("charge_interface_max(); // charges the process' interface. Since the interface is shared across all");
  dcode_add_line(" // components with interface objects, this call is not specific to any object or class.");
  dcode_add_line(" // charge_interface_max() charges the interface using as much power as possible");
  dcode_add_line(" // (the charge rate is determined by the maximum interface strength).");
	}

 if (dcode_state.object_type_present [OBJECT_TYPE_REPAIR]
		|| dcode_state.object_type_present [OBJECT_TYPE_REPAIR_OTHER])
	{
   dcode_newline();
//   dcode_add_line("if (get_power_left() > 50)");
// 	 dcode_state.indent_level ++;
   dcode_add_line("restore_self(); // tries to restore any destroyed components");
// 	 dcode_state.indent_level --;
   dcode_newline();
//   dcode_add_line("if (get_power_left() >= 16)");
// 	 dcode_state.indent_level ++;
   dcode_add_line("repair_self(); // tries to repair any damaged components");
// 	 dcode_state.indent_level --;

	 if (dcode_state.object_type_present [OBJECT_TYPE_REPAIR_OTHER])
			{
    dcode_newline();
//    dcode_add_line("if (get_power_left() > 50)");
// 	  dcode_state.indent_level ++;
    dcode_add_line("restore_scan(0,0); // scans for nearby processes with destroyed components and tries to restore them");
// 	  dcode_state.indent_level --;
    dcode_newline();
//    dcode_add_line("if (get_power_left() >= 16)");
// 	  dcode_state.indent_level ++;
    dcode_add_line("repair_scan(0,0); // scans for nearby damaged processes and tries to repair them");
// 	  dcode_state.indent_level --;
			}
	}

 dcode_newline();
 dcode_add_line("exit; // stops execution, until the next cycle");

 dcode_newline();
 dcode_add_line("// if there are any subroutines (called by gosub statements), they go here");
 dcode_newline();

 if (include_check_for_build_command)
	{


 dcode_newline();
 dcode_newline();
 dcode_add_line("check_for_build_command: // this is a label for gosub statements");
 	dcode_state.indent_level ++;
   dcode_add_line("// let's see if there is a build command on the queue for this process");
   dcode_add_line("if (check_build_queue() > 0) // returns the number of build commands on the queue for this process");
   dcode_add_line("{");
  	dcode_state.indent_level ++;
    dcode_add_line("build_x = build_queue_get_x(); // returns absolute location of the first queued command for this process");
    dcode_add_line("build_y = build_queue_get_y(); //  - note that the command may not be at the front of the whole queue");
    dcode_add_line("mode = MODE_MOVE_BUILD;");
    dcode_add_line("// let's move to somewhere near the build location, but not right on it (or this process might get in the way)");
    dcode_add_line("build_target_angle = atan2(build_y - core_y, build_x - core_x);");
    dcode_add_line("move_x = build_x - cos(build_target_angle, 300); // stop short of target");
    dcode_add_line("move_y = build_y - sin(build_target_angle, 300);");
//    dcode_add_line("clear_all_commands(); // clears the command queue (move-build commands cannot currently be queued)");
    dcode_add_line("if (verbose) printf(\"\\nMoving to build location.\");");
    dcode_state.indent_level --;
   dcode_add_line("} // end if (check_build_queue() > 0)");
   dcode_add_line("return; // returns to the statement immediately after the gosub");
  	dcode_state.indent_level --;

	}

	if (dcode_state.mobile)
	{
// the circle_around_target subroutine is used by some attack types, and by the guard command (which all mobile processes have)
//  (although currently it's turned off for attacking)
     dcode_newline();
     dcode_newline();
     dcode_add_line("circle_around_target: // this is a label for gosub statements");
 	   dcode_state.indent_level ++;
      dcode_add_line("// this subroutine makes the process circle around a target.");
      dcode_add_line("// before this subroutine was called, circle_target should have been set to");
      dcode_add_line("//  the target that the process will circle around. The target should be visible.");
      dcode_add_line("// And circle_rotation should have been set to 1024 (clockwise - for guard commands)");
      dcode_add_line("//  or -1024 (anticlockwise - for attack commands)");
      dcode_add_line("int angle_to_circle_target, circle_move_x, circle_move_y;");
      dcode_add_line("angle_to_circle_target = process[circle_target].target_angle();");
      dcode_add_line("angle_to_circle_target += circle_rotation; // this will lead the process in a circle around the target");
      dcode_add_line("circle_move_x = process[circle_target].get_core_x() // location of target");
      dcode_add_line("                + (process[circle_target].get_core_speed_x() * 10) // if the target is moving, aim for a point a little ahead of it");
      dcode_add_line("                - cos(angle_to_circle_target, circle_distance); // work out the radius of the circle around the target");
      dcode_add_line("circle_move_y = process[circle_target].get_core_y()");
      dcode_add_line("                + (process[circle_target].get_core_speed_y() * 10)");
      dcode_add_line("                - sin(angle_to_circle_target, circle_distance);");
      dcode_add_line("auto_move.move_to(circle_move_x, circle_move_y);");
      dcode_add_line("return; // returns to the statement immediately after the gosub");
  	   dcode_state.indent_level --;

	}

/*
	{
	 dcode_newline();
	 dcode_add_line("if (attacking_left == 1)");
	 dcode_add_line("{");
	 dcode_state.indent_level ++;
	 dcode_add_line("other_target_x = process[TARGET_LEFT].get_core_x();");
	 dcode_add_line("other_target_y = process[TARGET_LEFT].get_core_y();");
	 dcode_add_line("if (other_target_x >= 0");
	 dcode_add_line(" && arc_length(angle - 2048, atan2(other_target_y - core_y, other_target_x - core_x)) <= 2048)");
	 dcode_add_line("{");
	 dcode_state.indent_level ++;
	 dcode_add_line("att_left_dir.fire_at(TARGET_LEFT, 0);");
	 dcode_state.indent_level --;
	 dcode_add_line("}");
	 dcode_state.indent_level ++;
	 dcode_add_line("else");
	 dcode_state.indent_level ++;
	 dcode_add_line("{");
	 dcode_state.indent_level ++;
	 dcode_add_line("attacking_left = 0;");
	 dcode_state.indent_level --;
	 dcode_add_line("}");
	 dcode_state.indent_level --;
	 dcode_state.indent_level --;
	 dcode_state.indent_level --;
	 dcode_add_line("}");
	 dcode_state.indent_level ++;
	 dcode_add_line("else");
	 dcode_state.indent_level ++;
	 dcode_add_line("{");
	 dcode_state.indent_level ++;
	 dcode_add_line("scan_result = scan_for_threat(cos(angle - 2048, 400), sin(angle - 2048, 400), TARGET_LEFT);");
	 dcode_add_line("if (scan_result == 1)");
	 dcode_state.indent_level ++;
	 dcode_add_line("attacking_left = 1; // actual suitability of the target will be assessed next cycle");
	 dcode_state.indent_level --;
	 dcode_state.indent_level --;
	 dcode_add_line("}");
	}*/

}

/*

static void add_dir_attack_code(const char* flag_text, const char* target_index_text, const char* class_name_text, const char* angle_text)
{
  char line_string [120];


	 sprintf(line_string, "if (%s == 1)", flag_text);
	 dcode_add_line(line_string);
	 dcode_add_line("{");
	 dcode_state.indent_level ++;
	 sprintf(line_string, "other_target_x = process[%s].get_core_x();", target_index_text);
	 dcode_add_line(line_string);
	 sprintf(line_string, "other_target_y = process[%s].get_core_y();", target_index_text);
	 dcode_add_line(line_string);
	 dcode_add_line("if (other_target_x >= 0 // checks target visibility (will be <0 if target not visible)");
	 sprintf(line_string, " && process[%s].distance_less(1000) // checks distance from this process to target", target_index_text);
	 dcode_add_line(line_string);
	 sprintf(line_string, " && arc_length(%s, atan2(other_target_y - core_y, other_target_x - core_x)) <= 2500)", angle_text);
	 dcode_add_line(line_string);
	 dcode_add_line("{");
	 dcode_state.indent_level ++;
	 sprintf(line_string, "%s.fire_at(%s, 0);", class_name_text, target_index_text);
	 dcode_add_line(line_string);
	 dcode_state.indent_level --;
	 dcode_add_line("}");
	 dcode_state.indent_level ++;
	 dcode_add_line("else");
	 dcode_state.indent_level ++;
	 dcode_add_line("{");
	 dcode_state.indent_level ++;
	 sprintf(line_string, "%s = 0;", flag_text);
	 dcode_add_line(line_string);
	 dcode_state.indent_level --;
	 dcode_add_line("}");
	 dcode_state.indent_level --;
	 dcode_state.indent_level --;
	 dcode_state.indent_level --;
	 dcode_add_line("}");
	 dcode_state.indent_level ++;
	 dcode_add_line("else");
	 dcode_state.indent_level ++;
	 dcode_add_line("{");
	 dcode_state.indent_level ++;
	 sprintf(line_string, "scan_result = scan_for_threat(cos(%s, 400), sin(%s, 400), %s);", angle_text, angle_text, target_index_text);
	 dcode_add_line(line_string);
	 dcode_add_line("if (scan_result == 1)");
	 dcode_add_line("{");
	  dcode_state.indent_level ++;
	  sprintf(line_string, "%s = 1;", flag_text);
	  dcode_add_line(line_string);
 	 sprintf(line_string, "%s.aim_at(%s, 0); // just aim for now. wait until next cycle to fire", class_name_text, target_index_text);
	  dcode_add_line(line_string);
	  dcode_state.indent_level --;
	 dcode_add_line("}");

	 dcode_state.indent_level ++;
	 dcode_add_line("else");
	 dcode_state.indent_level ++;
	 sprintf(line_string, "%s.no_target(); // returns object to its default angle", class_name_text);
	 dcode_add_line(line_string);
	 dcode_state.indent_level --;
	 dcode_state.indent_level --;

	 dcode_state.indent_level --;
//	 dcode_state.indent_level --;
	 dcode_add_line("}");

	 dcode_state.indent_level --;
	 dcode_state.indent_level --;
//	 dcode_state.indent_level --;



}
*/


/*

	int a;
int target_x;
int target_y;
int attack_member;
int x;
int y;
x = get_core_x();
y = get_core_y();
int scan_result;
int mode;

	*/



/*

AUTO_CLASS_MOVE,
AUTO_CLASS_RETRO,
AUTO_CLASS_ATTACK_MAIN,
AUTO_CLASS_ATTACK_FRONT_DIR,
AUTO_CLASS_ATTACK_LEFT_DIR,
AUTO_CLASS_ATTACK_RIGHT_DIR,
AUTO_CLASS_ATTACK_BACK_DIR,
AUTO_CLASS_HARVEST,
AUTO_CLASS_ALLOCATE,
AUTO_CLASS_BUILD,
// need to make changes below if AUTO_CLASSES ever gets close to OBJECT_CLASSES (16) (although actually I think this is caught and will produce an error if there are too many)
AUTO_CLASSES
};

enum
{
AUTOMODE_IDLE,
AUTOMODE_MOVE,
AUTOMODE_ATTACK,
AUTOMODE_ATTACK_FOUND,
AUTOMODE_HARVEST,
AUTOMODE_FOLLOW,
AUTOMODE_MOVE_BUILD, // move somewhere and build

*/

// this function works out what the process's main attack is (fixed or directional) and adds the appropriate code.
// to avoid duplication (particularly between MODE_ATTACK and MODE_ATTACK_FOUND) this could be done as a subroutine
//  but that would make the autocode a bit harder to read.
// 0 for attack (process will go to next command if target destroyed or lost)
// 1 for attack_found (process will return to saved mode if target destroyed or lost)
static void add_main_attacking_code(int attack_or_attack_found)
{

 switch(dcode_state.autocode_type)
 {
	 case AUTOCODE_CAUTIOUS:
   dcode_add_line("if (get_damage()");
   if (dcode_state.object_type_present [OBJECT_TYPE_INTERFACE])
   dcode_add_line(" && get_interface_strength() < 20)");
   else
			{
   dcode_add_line(" && (get_total_integrity() * 2 < get_unharmed_integrity_max()");
   dcode_add_line("  || component[0].get_integrity() * 2 < component[0].get_integrity_max()))");
			}
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
 	  if (!attack_or_attack_found) // i.e. MODE_ATTACK
     dcode_add_line("saved_mode = MODE_ATTACK;");
    dcode_add_line("mode = MODE_WITHDRAW;");
    dcode_add_line("get_damage_source(TARGET_WITHDRAW);");
    dcode_add_line("if (verbose) printf(\"\\nRetreating.\");");
 	  dcode_state.indent_level --;
   dcode_add_line("}");
// fall-through...
	 case AUTOCODE_STANDARD:
// move to medium range and attack
   dcode_add_line("// Now see whether the commanded target is visible:");
   dcode_add_line("//  (targets are visible if within scanning range of any friendly process)");
   dcode_add_line("if (!process[TARGET_MAIN].visible()) // returns zero if not target visible or doesn't exist");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
    dcode_add_line("auto_move.move_to(target_x, target_y); // calls move_to for all objects in the move class");
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
					add_main_attacking_code_spike_default(0); // fires spike at main target if possible
				add_target_lost_attacking_code(attack_or_attack_found);
 	  dcode_state.indent_level --;
   dcode_add_line("}");
 	 dcode_state.indent_level ++;
    dcode_add_line("else");
    dcode_add_line("{");
 	  dcode_state.indent_level ++;
   	 dcode_add_line("target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN");
 	   dcode_add_line("target_y = process[TARGET_MAIN].get_core_y();");
// different code needed here depending on what kind of front attack we have:
     if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_MAIN])
				 {
     dcode_add_line("auto_move.approach_track(TARGET_MAIN,target_component,auto_att_main, 700);");
     dcode_add_line(" // approach_track() works out movement required to hit a target with a particular attacking object,");
     dcode_add_line(" //  and if retro move objects are available, also tries to maintain a certain distance.");
     dcode_add_line(" // It applies a simple target leading algorithm (which requires the attacking class to be identified)");
     dcode_add_line(" // Parameters are:");
     dcode_add_line(" //  - target's address in targetting memory");
     dcode_add_line(" //  - component of target process to attack");
     dcode_add_line(" //  - class of attacking object (the first object in the class will be used)");
     dcode_add_line(" //  - stand-off distance (in pixels)");
				 }
				  else
				 {
     dcode_add_line("auto_move.approach_target(TARGET_MAIN,target_component,700);");
     dcode_add_line(" // approach_target() approaches a target to within a certain distance (700 in this case).");
     dcode_add_line(" //  if the process has retro move objects it will use them to maintain the distance.");
     dcode_add_line(" // Parameters are:");
     dcode_add_line(" //  - target's address in targetting memory");
     dcode_add_line(" //  - component of target process to attack");
     dcode_add_line(" //  - stand-off distance (in pixels)");
				 }
				 add_main_attacking_code_specific(); // fires main attacking objects
				 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
				 	add_main_attacking_code_spike_default(1); // fires spike at main target if possible
 	   dcode_state.indent_level --;
    dcode_add_line("}");
	   dcode_state.indent_level --;
		 break;

		case AUTOCODE_CHARGE:
// move to medium range and attack
   dcode_add_line("// Now see whether the commanded target is in scan range:");
   dcode_add_line("if (!process[TARGET_MAIN].visible()) // returns zero if not target visible or doesn't exist");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
    dcode_add_line("auto_move.move_to(target_x, target_y); // calls move_to for all objects in the move class");
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
					add_main_attacking_code_spike_default(0); // fires spike at main target if possible
				add_target_lost_attacking_code(attack_or_attack_found);
 	  dcode_state.indent_level --;
   dcode_add_line("}");
 	 dcode_state.indent_level ++;
    dcode_add_line("else");
    dcode_add_line("{");
 	  dcode_state.indent_level ++;
   	 dcode_add_line("target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN");
 	   dcode_add_line("target_y = process[TARGET_MAIN].get_core_y();");
// different code needed here depending on what kind of front attack we have:
     if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_MAIN])
			 	{
     dcode_add_line("auto_move.intercept(TARGET_MAIN,0,auto_att_main); // calls intercept() on all objects in the auto_move class.");
     dcode_add_line(" // intercept() works out movement required to hit a target with a particular fixed attacking object.");
     dcode_add_line(" //  (it differs from approach_track in that it has no minimum distance, and may collide with the target)");
     dcode_add_line(" // It applies a simple target leading algorithm (which requires the attacking object to be identified)");
     dcode_add_line(" // Parameters are:");
     dcode_add_line(" //  - target's address in targetting memory");
     dcode_add_line(" //  - component of target process to attack");
     dcode_add_line(" //  - class of attacking object (the first object in the class will be used)");
				 }
				  else
				 {
     dcode_add_line("auto_move.approach_target(TARGET_MAIN,target_component,0);");
     dcode_add_line(" // approach_target() approaches a target to within a certain distance.");
     dcode_add_line(" //  (0 in this case, so it will try to collide with the target)");
     dcode_add_line(" // Parameters are:");
     dcode_add_line(" //  - target's address in targetting memory");
     dcode_add_line(" //  - component of target process to attack");
     dcode_add_line(" //  - stand-off distance (in pixels)");
				 }
				 add_main_attacking_code_specific(); // fires main attacking objects
				 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
				 	add_main_attacking_code_spike_default(1); // fires spike at main target if possible
 	   dcode_state.indent_level --;
    dcode_add_line("}");
	   dcode_state.indent_level --;
		 break;

	 case AUTOCODE_BOMBARD:
// like STANDARD but tries to maintain a much greater distance
//  and will fire at a target's expected location at spike range instead of approaching to visible range.
// (only really works for spike attacks)
   dcode_add_line("// Now see whether the commanded target is visible:");
   dcode_add_line("if (!process[TARGET_MAIN].visible()) // returns zero if not target visible or doesn't exist");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
    dcode_add_line("if (target_destroyed(TARGET_MAIN)) // returns one if target recently destroyed, and");
    dcode_add_line(" // its last location is visible to any friendly process");
    dcode_add_line("{");
 	  dcode_state.indent_level ++;
 	   if (attack_or_attack_found == 0)
				 {
     dcode_add_line("clear_command(); // clears the current command.");
     dcode_add_line("mode = MODE_IDLE; // if there is another command queued, the process will receive");
     dcode_add_line("                  // it next cycle.");
     dcode_add_line("if (verbose) printf(\"\\nTarget not detected.\");");
     dcode_add_line("break;");
				 }
				 else
				 {
     dcode_add_line("mode = saved_mode; // the process goes back to what it was doing");
     dcode_add_line("if (verbose) printf(\"\\nTarget not detected.\");");
     dcode_add_line("break;");
				 }
  	  dcode_state.indent_level --;
    dcode_add_line("}");
    dcode_add_line("// Target lost, and not confirmed destroyed. Process will bombard the target's");
    dcode_add_line("//  last known location until given a new command.");
    dcode_add_line("auto_move.approach_xy(target_x, target_y, 1600); // approaches to within 1600 pixels of location");
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
					add_main_attacking_code_spike_default(0); // fires spike at main target if possible
 	  dcode_state.indent_level --;
   dcode_add_line("}");
 	 dcode_state.indent_level ++;
    dcode_add_line("else");
    dcode_add_line("{");
 	  dcode_state.indent_level ++;
// bombard doesn't really work with burst or other fixed forward attacks
				{
  	 dcode_add_line("target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN");
	   dcode_add_line("target_y = process[TARGET_MAIN].get_core_y();");
    dcode_add_line("auto_move.approach_target(TARGET_MAIN,target_component,1600);");
    dcode_add_line(" // approach_target() approaches a target to within a certain distance (1600 in this case).");
    dcode_add_line(" //  if the process has retro move objects it will use them to maintain the distance.");
    dcode_add_line(" // Parameters are:");
    dcode_add_line(" //  - target's address in targetting memory");
    dcode_add_line(" //  - component of target process to attack");
    dcode_add_line(" //  - stand-off distance (in pixels)");
				}
				add_main_attacking_code_specific(); // fires main attacking objects
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
					add_main_attacking_code_spike_default(1); // fires spike at main target if possible
 	  dcode_state.indent_level --;
   dcode_add_line("}");
	  dcode_state.indent_level --;
		 break;

		case AUTOCODE_CIRCLE_CW:
		case AUTOCODE_CIRCLE_ACW:
   dcode_add_line("// Now see whether the commanded target is in scan range:");
   dcode_add_line("if (!process[TARGET_MAIN].visible()) // returns zero if not target visible or doesn't exist");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
    dcode_add_line("auto_move.move_to(target_x, target_y); // calls move_to for all objects in the move class");
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
					add_main_attacking_code_spike_default(0); // fires spike at main target if possible
				add_target_lost_attacking_code(attack_or_attack_found);
 	  dcode_state.indent_level --;
   dcode_add_line("}");
 	 dcode_state.indent_level ++;
    dcode_add_line("else");
    dcode_add_line("{");
 	  dcode_state.indent_level ++;
   	 dcode_add_line("target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN");
 	   dcode_add_line("target_y = process[TARGET_MAIN].get_core_y();");
     dcode_add_line("circle_target = TARGET_MAIN; // the process will circle around this target");
     if (dcode_state.autocode_type == AUTOCODE_CIRCLE_CW)
      dcode_add_line("circle_rotation = 1024; // the process will aim 1024 angle units (45 degrees) around the circle, clockwise.");
       else
		      dcode_add_line("circle_rotation = -1024; // the process will aim 1024 angle units (45 degrees) around the circle, anti-clockwise.");
     dcode_add_line("circle_distance = 700; // the process will try to stay this far from the centre of the circle");
     dcode_add_line("gosub circle_around_target;");
				 add_main_attacking_code_specific(); // fires main attacking objects
				 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
				 	add_main_attacking_code_spike_default(1); // fires spike at main target if possible
     dcode_state.indent_level --;
    dcode_add_line("}");
    dcode_state.indent_level --;
    break;

		case AUTOCODE_ERRATIC:
   dcode_add_line("// Now see whether the commanded target is in scan range:");
   dcode_add_line("if (!process[TARGET_MAIN].visible()) // returns zero if not target visible or doesn't exist");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
    dcode_add_line("auto_move.move_to(target_x, target_y); // calls move_to for all objects in the move class");
    dcode_add_line("circle_rotation = random(4000) - 2000; // the process will aim this many angle units around the circle.");
    dcode_add_line("circle_distance = 800 + random(400); // the process will try to stay this far from the centre of the circle");
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
					add_main_attacking_code_spike_default(0); // fires spike at main target if possible
				add_target_lost_attacking_code(attack_or_attack_found);
 	  dcode_state.indent_level --;
   dcode_add_line("}");
 	 dcode_state.indent_level ++;
    dcode_add_line("else");
    dcode_add_line("{");
 	  dcode_state.indent_level ++;
   	 dcode_add_line("target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN");
 	   dcode_add_line("target_y = process[TARGET_MAIN].get_core_y();");
     dcode_add_line("circle_target = TARGET_MAIN; // the process will circle around this target");
     dcode_add_line("if (random(12) == 0)");
     dcode_add_line("  circle_rotation = random(4000) - 2000; // the process will aim this many angle units around the circle.");
     dcode_add_line("if (random(4) == 0)");
     dcode_add_line("  circle_distance = 800 + random(400); // the process will try to stay this far from the centre of the circle");
     dcode_add_line("gosub circle_around_target;");
				 add_main_attacking_code_specific(); // fires main attacking objects
				 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
				 	add_main_attacking_code_spike_default(1); // fires spike at main target if possible
     dcode_state.indent_level --;
    dcode_add_line("}");
    dcode_state.indent_level --;
    break;
/*
		case AUTOCODE_CAUTIOUS:
// move to medium range and attack, then withdraw to a safe distance, then attack again
//   dcode_add_line("printf(\"[%i,%i] (%i,%i) d %i hw %i\", core_x, core_y, target_x, target_y, distance_from_xy(target_x, target_y), harass_withdraw);");
   dcode_add_line("if (harass_withdraw > 10)");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
    dcode_add_line("withdraw_angle = atan2(core_y - target_y, core_x - target_x);");
    dcode_add_line("withdraw_x = target_x + cos(withdraw_angle, 1400);");
    dcode_add_line("withdraw_y = target_y + sin(withdraw_angle, 1400);");
    dcode_add_line("auto_move.move_to(withdraw_x, withdraw_y); // calls move_to for all objects in the move class");
    if (dcode_state.object_type_present [OBJECT_TYPE_INTERFACE])
				{
    dcode_add_line("if (get_interface_strength() == get_interface_capacity()");
    dcode_add_line(" && distance_from_xy_more(target_x, target_y, 1300))");
//    dcode_add_line("{");
    dcode_add_line("  harass_withdraw = 0;");
//    dcode_add_line("}");
//    dcode_add_line("  harass_withdraw = 0;");
				}
				 else
					{
    dcode_add_line("if (distance_from_xy_more(target_x, target_y, 1300))");
    dcode_add_line("  harass_withdraw = 0;");
					}
    dcode_add_line("break;");
 	  dcode_state.indent_level --;
   dcode_add_line("}");
   dcode_add_line("// Now see whether the commanded target is in scan range:");
   dcode_add_line("if (!process[TARGET_MAIN].visible()) // returns zero if not target visible or doesn't exist");
   dcode_add_line("{");
 	 dcode_state.indent_level ++;
    dcode_add_line("auto_move.move_to(target_x, target_y); // calls move_to for all objects in the move class");
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
					add_main_attacking_code_spike_default(0); // fires spike at main target if possible
				add_target_lost_attacking_code(attack_or_attack_found);
 	  dcode_state.indent_level --;
   dcode_add_line("}");
 	 dcode_state.indent_level ++;
    dcode_add_line("else");
    dcode_add_line("{");
 	  dcode_state.indent_level ++;
   	 dcode_add_line("target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN");
 	   dcode_add_line("target_y = process[TARGET_MAIN].get_core_y();");
// different code needed here depending on what kind of front attack we have:
     if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_MAIN])
				 {
     dcode_add_line("auto_move.approach_track(TARGET_MAIN,target_component,auto_att_main,900);");
     dcode_add_line("if (distance_from_xy_less(target_x, target_y, 950))");
     dcode_add_line("  harass_withdraw ++;");
				 }
				 else
				 {
     dcode_add_line("auto_move.approach_target(TARGET_MAIN,target_component,900);");
     dcode_add_line(" // approach_target() approaches a target to within a certain distance (900 in this case).");
     dcode_add_line(" //  if the process has retro move objects it will use them to maintain the distance.");
     dcode_add_line(" // Parameters are:");
     dcode_add_line(" //  - target's address in targetting memory");
     dcode_add_line(" //  - component of target process to attack");
     dcode_add_line(" //  - stand-off distance (in pixels)");
     dcode_add_line("if (distance_from_xy_less(target_x, target_y, 950))");
     dcode_add_line("  harass_withdraw ++;");
				 }
				 add_main_attacking_code_specific(); // fires main attacking objects
				 if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
				 	add_main_attacking_code_spike_default(1); // fires spike at main target if possible
 	   dcode_state.indent_level --;
    dcode_add_line("}");
    dcode_state.indent_level --;
		 break;
*/


 }


}

static void add_target_lost_attacking_code(int attack_or_attack_found)
{

	   dcode_add_line("if (distance_from_xy_less(target_x, target_y, 600))");
    dcode_add_line("{");
 	  dcode_state.indent_level ++;
     dcode_add_line("// we should be able to see the target now, so it's either been destroyed");
     dcode_add_line("// or gone out of range.");
 	   if (attack_or_attack_found == 0)
				 {
     dcode_add_line("clear_command(); // clears the current command.");
     dcode_add_line("mode = MODE_IDLE; // if there is another command queued, the process will receive");
     dcode_add_line("                  // it next cycle.");
     dcode_add_line("if (verbose) printf(\"\\nTarget not detected.\");");
     dcode_add_line("break;");
				 }
				 else
				 {
     dcode_add_line("mode = saved_mode; // the process goes back to what it was doing");
     dcode_add_line("if (verbose) printf(\"\\nTarget not detected.\");");
     dcode_add_line("break;");
				 }
  	  dcode_state.indent_level --;
    dcode_add_line("}");

}



static void add_main_attacking_code_specific(void)
{


   if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_MAIN])
			{
//   dcode_add_line("if (process[TARGET_MAIN].distance_less(1000)");
   dcode_add_line("if (distance_from_xy_less(target_x, target_y, 1000)");
   dcode_add_line(" && arc_length(angle, process[TARGET_MAIN].target_angle()) < 500)");
   dcode_state.indent_level ++;
    dcode_add_line("auto_att_main.fire(0); // tries to fire all objects in the auto_att_main class. 0 is firing delay (in ticks)");
    dcode_state.indent_level --;
   dcode_add_line(" // (attack class is for fixed attack objects that point more or less forwards)");
			}
// also try to hit target with forward dir attacking objects, if any:
   if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR])
			{
//    dcode_add_line("if (process[TARGET_MAIN].distance_less(1000))");
   dcode_add_line("if (distance_from_xy_less(target_x, target_y, 1000))");
// don't need to check angle; fire_at() does this automatically
    dcode_add_line("{");
    dcode_state.indent_level ++;
		 		dcode_add_line("auto_att_fwd.fire_at(TARGET_MAIN,target_component); // Calls fire_at() on all objects in the forward directional attack class.");
     dcode_add_line(" // TARGET_MAIN indicates that the target is in targetting memory address TARGET_MAIN.");
     dcode_add_line(" // target_component is the component to fire at (can be set by user's command; defaults to 0 (attack core)).");
     dcode_add_line(" // fire_at() rotates directional attack objects towards the target (with basic leading) and fires.");
			 	dcode_add_line("front_attack_primary = 1; // this means the forward directional attack class will not try to find its own target.");
    dcode_state.indent_level --;
    dcode_add_line("}");
			}

   if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_LEFT_DIR]
				&& dcode_state.autocode_type == AUTOCODE_CIRCLE_ACW)
			{
    dcode_add_line("if (process[TARGET_MAIN].distance_less(1000))");
// don't need to check angle; fire_at() does this automatically
    dcode_add_line("{");
    dcode_state.indent_level ++;
		 		dcode_add_line("auto_att_left.fire_at(TARGET_MAIN,target_component); // Calls fire_at() on all objects in the left directional attack class.");
     dcode_add_line(" // TARGET_MAIN indicates that the target is in targetting memory address TARGET_MAIN.");
     dcode_add_line(" // target_component is the component to fire at (can be set by user's command; defaults to 0 (attack core)).");
     dcode_add_line(" // fire_at() rotates directional attack objects towards the target (with basic leading) and fires.");
			 	dcode_add_line("left_attack_primary = 1; // this means the left directional attack class will not try to find its own target.");
    dcode_state.indent_level --;
    dcode_add_line("}");
			}

   if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_RIGHT_DIR]
				&& dcode_state.autocode_type == AUTOCODE_CIRCLE_CW)
			{
    dcode_add_line("if (process[TARGET_MAIN].distance_less(1000))");
// don't need to check angle; fire_at() does this automatically
    dcode_add_line("{");
    dcode_state.indent_level ++;
		 		dcode_add_line("auto_att_right.fire_at(TARGET_MAIN,target_component); // Calls fire_at() on all objects in the right directional attack class.");
     dcode_add_line(" // TARGET_MAIN indicates that the target is in targetting memory address TARGET_MAIN.");
     dcode_add_line(" // target_component is the component to fire at (can be set by user's command; defaults to 0 (attack core)).");
     dcode_add_line(" // fire_at() rotates directional attack objects towards the target (with basic leading) and fires.");
			 	dcode_add_line("right_attack_primary = 1; // this means the right directional attack class will not try to find its own target.");
    dcode_state.indent_level --;
    dcode_add_line("}");
			}

			if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
			{
    dcode_add_line("if (process[TARGET_MAIN].distance_less(1600))");
// should this check angle as well?
    dcode_add_line("{");
    dcode_state.indent_level ++;
    dcode_add_line("auto_att_spike.fire_spike_xy(target_x,target_y); // Calls fire_spike_xy() on all spike objects in the spike class.");
    dcode_state.indent_level --;
    dcode_add_line("}");
			}

}


// adds code for spikes to attack.
// if target visible, attacks target
// if target not visible, but we have a last known location, fire anyway
//  (assumes that visibility test performed beforehand)
static void add_main_attacking_code_spike_default(int target_visible)
{

 if (target_visible)
	{
     dcode_add_line("if (process[TARGET_MAIN].distance_less(1600))");
     dcode_add_line("{");
     dcode_state.indent_level ++;
      dcode_add_line("auto_att_spike.fire_spike_at(TARGET_MAIN,target_component); // Calls fire_spike_at() on all spike objects in the spike class.");
      dcode_add_line(" // TARGET_MAIN indicates that the target is in targetting memory address TARGET_MAIN.");
      dcode_add_line(" // target_component is the component of the target being attacked.");
      dcode_add_line(" // uses some target prediction (although this is difficult with spikes).");
      dcode_state.indent_level --;
     dcode_add_line("}");
	}
	 else
	 {
       dcode_add_line("if (distance_from_xy_less(target_x, target_y, 1600)) // is this process less than 1600 pixels from target?");
        dcode_add_line("{");
        dcode_state.indent_level ++;
         dcode_add_line("auto_att_spike.fire_spike_xy(target_x,target_y); // Calls fire_spike_xy() on all spike objects in the spike class.");
         dcode_add_line(" // target_x/y is the location (absolute x/y) to fire at.");
         dcode_add_line(" // target_x/y should be the last known location of the target, so this should fire even if the");
         dcode_add_line(" //  target is not currently visible.");
         dcode_state.indent_level --;
        dcode_add_line("}");

	 }


}


#ifdef OLD_MAIN_ATTACK_CODE

// this function works out what the process's main attack is (fixed or directional) and adds the appropriate code.
// to avoid duplication (particularly between MODE_ATTACK and MODE_ATTACK_FOUND) this could be done as a subroutine
//  but that would make the autocode a bit harder to read.
static void add_main_attacking_code(void)
{

 if (dcode_state.main_attack_type != MAIN_ATTACK_LONG_RANGE)
	{
  dcode_add_line("// Now see whether the commanded target is in scan range:");
  dcode_add_line("if (!process[TARGET_MAIN].visible()) // returns zero if not target visible or doesn't exist");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
  dcode_add_line("auto_move.move_to(target_x, target_y); // calls move_to for all objects in the move class");
 	dcode_state.indent_level --;
  dcode_add_line("}");
 	dcode_state.indent_level ++;
  dcode_add_line("else");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
 }

 if (dcode_state.main_attack_type == MAIN_ATTACK_INTERCEPT)
	{
// MAIN_ATTACK_INTERCEPT assumes that auto_att_main is available
  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_RETRO])
		{
    dcode_add_line("auto_move.approach_track(TARGET_MAIN,target_component,auto_att_main, 700);");
    dcode_add_line(" // approach_track() works out movement required to hit a target with a particular attacking object,");
    dcode_add_line(" //  but also tries to maintain a certain distance using retro (forward-facing) move objects.");
    dcode_add_line(" // It applies a simple target leading algorithm (which requires the attacking object to be identified)");
    dcode_add_line(" // Parameters are:");
    dcode_add_line(" //  - target's address in targetting memory");
    dcode_add_line(" //  - component of target process to attack");
    dcode_add_line(" //  - class of attacking object (the first object in the class will be used)");
    dcode_add_line(" //  - stand-off distance (in pixels)");
		}
		 else
			{
//   dcode_add_line("if (abs(target_y - core_y) + abs(target_x - core_x) < 400)");
   dcode_add_line("if (process[TARGET_MAIN].distance_less(800))");
   dcode_state.indent_level ++;
    dcode_add_line("auto_move.track_target(TARGET_MAIN,target_component,auto_att_main);");
    dcode_state.indent_level ++;
	    dcode_add_line("else");
     dcode_state.indent_level ++;
      dcode_add_line("auto_move.intercept(TARGET_MAIN,0,auto_att_main); // calls intercept() on all objects in the auto_move class.");
      dcode_add_line(" // track_target() works out rotation required to hit a target with a particular attacking object.");
      dcode_add_line(" // intercept() works out movement required to hit a target with a particular attacking object.");
      dcode_add_line(" // Both apply a simple target leading algorithm (which requires the attacking object to be identified)");
      dcode_add_line(" // Parameters are:");
      dcode_add_line(" //  - target's address in targetting memory");
      dcode_add_line(" //  - component of target process to attack");
      dcode_add_line(" //  - class of attacking object (the first object in the class will be used)");
      dcode_state.indent_level --;
     dcode_state.indent_level --;
    dcode_state.indent_level --;
			} // end no retro

   dcode_add_line("if (process[TARGET_MAIN].distance_less(1000))");
   dcode_state.indent_level ++;
    dcode_add_line("auto_att_main.fire(0); // tries to fire all objects in the auto_att_main class. 0 is firing delay (in ticks)");
    dcode_state.indent_level --;
   dcode_add_line(" // (attack class is for fixed attack objects that point more or less forwards)");
// also try to hit target with forward dir attacking objects, if any:
   if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR])
			{
    dcode_add_line("if (process[TARGET_MAIN].distance_less(1000))"); // TO DO: work out appropriate range for each attack type
    dcode_add_line("{");
    dcode_state.indent_level ++;
		 		dcode_add_line("auto_att_fwd.fire_at(TARGET_MAIN,target_component); // Calls fire_at() on all objects in the forward directional attack class.");
     dcode_add_line(" // TARGET_MAIN indicates that the target is in targetting memory address TARGET_MAIN.");
     dcode_add_line(" // target_component is the component to fire at (can be set by user's command; defaults to 0 (attack core)).");
     dcode_add_line(" // fire_at() rotates directional attack objects towards the target (with basic leading) and fires.");
			 	dcode_add_line("front_attack_primary = 1; // this means the forward directional attack class will not try to find its own target.");
    dcode_state.indent_level --;
    dcode_add_line("}");
			}
// should really make sure the angle is about right before firing!

	}

 if (dcode_state.main_attack_type == MAIN_ATTACK_CIRCLE)
	{

// MAIN_ATTACK_CIRCLE not currently used

/*  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_RETRO])
		{
    dcode_add_line("auto_move.approach_target(TARGET_MAIN,target_component,700);");
    dcode_add_line(" // approach_target() approaches a target,");
    dcode_add_line(" //  but also tries to maintain a certain distance using retro (forward-facing) move objects.");
    dcode_add_line(" // Parameters are:");
    dcode_add_line(" //  - target's address in targetting memory");
    dcode_add_line(" //  - component of target process to attack");
    dcode_add_line(" //  - stand-off distance (in pixels)");
		}
		 else*/
			{
//   dcode_add_line("if (abs(target_y - core_y) + abs(target_x - core_x) < 400)");
   dcode_add_line("if (process[TARGET_MAIN].distance_less(800))");
   dcode_add_line("{");
   dcode_state.indent_level ++;
    dcode_add_line("circle_target = TARGET_MAIN; // the process will circle around this target");
    dcode_add_line("circle_rotation = -1024; // the process will aim 1024 angle units (45 degrees) around the circle, anti-clockwise.");
    dcode_add_line("gosub circle_around_target;");
    dcode_state.indent_level --;
   dcode_add_line("}");
   dcode_state.indent_level ++;
    dcode_add_line("else");
    dcode_state.indent_level ++;
     dcode_add_line("auto_move.move_to(target_x, target_y); // calls move_to() on all objects in the move class.");
     dcode_state.indent_level --;
    dcode_state.indent_level --;
//   dcode_state.indent_level --;
			} // end no retro

   dcode_add_line("auto_att_fwd.fire_at(TARGET_MAIN,target_component); // Calls fire_at() on all objects in the forward directional attack class.");
   dcode_add_line(" // fire_at() rotates directional attack objects towards the target (with basic leading) and fires.");
   dcode_add_line(" // TARGET_MAIN indicates that the target is in targetting memory address TARGET_MAIN.");
   dcode_add_line(" // target_component is the component of the target to attack.");
			dcode_add_line("front_attack_primary = 1; // the forward directional attack class will not try to find its own target.");
	}


 if (dcode_state.main_attack_type == MAIN_ATTACK_APPROACH)
	{
//  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_RETRO])
		{
// I think approach_target is the right method to use whether or not there are retro objects:
    dcode_add_line("auto_move.approach_target(TARGET_MAIN,target_component,700);");
    dcode_add_line(" // approach_target() approaches a target to within a certain distance (700 in this case).");
    dcode_add_line(" //  if the process has retro move objects it will use them to maintain the distance.");
    dcode_add_line(" // Parameters are:");
    dcode_add_line(" //  - target's address in targetting memory");
    dcode_add_line(" //  - component of target process to attack");
    dcode_add_line(" //  - stand-off distance (in pixels)");
		}
/*		 else
			{
//   dcode_add_line("if (abs(target_y - core_y) + abs(target_x - core_x) < 400)");
   dcode_add_line("if (process[TARGET_MAIN].distance_less(600))");
   dcode_state.indent_level ++;
    dcode_add_line("auto_move.turn_to_target(TARGET_MAIN,target_component);");
    dcode_state.indent_level ++;
	    dcode_add_line("else");
     dcode_state.indent_level ++;
      dcode_add_line("auto_move.move_to(target_x, target_y); // calls move_to() on all objects in the move class.");
      dcode_state.indent_level --;
     dcode_state.indent_level --;
    dcode_state.indent_level --;
			} // end no retro*/

   dcode_add_line("auto_att_fwd.fire_at(TARGET_MAIN,target_component); // Calls fire_at() on all objects in the forward directional attack class.");
   dcode_add_line(" // fire_at() rotates directional attack objects towards the target (with basic leading) and fires.");
   dcode_add_line(" // TARGET_MAIN indicates that the target is in targetting memory address TARGET_MAIN.");
   dcode_add_line(" // target_component is the component of the target to attack.");
			dcode_add_line("front_attack_primary = 1; // the forward directional attack class will not try to find its own target.");
	}


/*
 if (dcode_state.main_attack_type == MAIN_ATTACK_APPROACH)
	{
  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_RETRO])
		{
    dcode_add_line("auto_move.approach_target(TARGET_MAIN,target_component,700);");
    dcode_add_line(" // approach_target() approaches a target,");
    dcode_add_line(" //  but also tries to maintain a certain distance using retro (forward-facing) move objects.");
    dcode_add_line(" // Parameters are:");
    dcode_add_line(" //  - target's address in targetting memory");
    dcode_add_line(" //  - component of target process to attack");
    dcode_add_line(" //  - stand-off distance (in pixels)");
		}
		 else
			{
//   dcode_add_line("if (abs(target_y - core_y) + abs(target_x - core_x) < 400)");
   dcode_add_line("if (process[TARGET_MAIN].distance_less(600))");
   dcode_state.indent_level ++;
    dcode_add_line("auto_move.turn_to_target(TARGET_MAIN,target_component);");
    dcode_state.indent_level ++;
	    dcode_add_line("else");
     dcode_state.indent_level ++;
      dcode_add_line("auto_move.move_to(target_x, target_y); // calls move_to() on all objects in the move class.");
      dcode_state.indent_level --;
     dcode_state.indent_level --;
    dcode_state.indent_level --;
			} // end no retro

   dcode_add_line("auto_att_fwd.fire_at(TARGET_MAIN,target_component); // Calls fire_at() on all objects in the forward directional attack class.");
   dcode_add_line(" // fire_at() rotates directional attack objects towards the target (with basic leading) and fires.");
   dcode_add_line(" // TARGET_MAIN indicates that the target is in targetting memory address TARGET_MAIN.");
   dcode_add_line(" // target_component is the component of the target to attack.");
			dcode_add_line("front_attack_primary = 1; // the forward directional attack class will not try to find its own target.");
	}
*/

 if (dcode_state.main_attack_type != MAIN_ATTACK_LONG_RANGE)
	{
// this matches the code above that checks whether the target is in scan range.
// it isn't relevant to long range attacking
 	dcode_state.indent_level --;
  dcode_add_line("}");
 	dcode_state.indent_level --;
// 	dcode_state.indent_level --;
	}


 if (dcode_state.main_attack_type == MAIN_ATTACK_LONG_RANGE)
	{
// Unlike the other attack types, this one can involve attacking something not currently visible to the firing process.
// So we may need to use target_x/target_y values:

  dcode_add_line("if (!process[TARGET_MAIN].visible()) // returns zero if out of range or doesn't exist");
  dcode_add_line("{");
 	dcode_state.indent_level ++;
  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_RETRO])
		{
   dcode_add_line("auto_move.approach_xy(target_x, target_y, 1200); // calls approach_xy for all objects in the move class");
   dcode_add_line("if (process[TARGET_MAIN].distance_less(1600))");
   dcode_state.indent_level ++;
    dcode_add_line("auto_att_spike.fire_spike_xy(target_x,target_y);"); // Calls fire_spike_xy() on all spike objects in the forward spike class.");
    dcode_state.indent_level --;
		}
    else
				{
//     dcode_add_line("if (abs(target_y - core_y) + abs(target_x - core_x) < 1200)");
     dcode_add_line("if (process[TARGET_MAIN].distance_less(1600))");
     dcode_state.indent_level ++;
     dcode_add_line("{");
 	   dcode_state.indent_level ++;
      dcode_add_line("auto_move.turn_to_xy(target_x,target_y);");
      dcode_add_line("auto_att_spike.fire_spike_xy(target_x,target_y);"); // Calls fire_spike_xy() on all spike objects in the forward spike class.");
 	   dcode_state.indent_level --;
     dcode_add_line("}");
      dcode_state.indent_level ++;
	      dcode_add_line("else");
       dcode_state.indent_level ++;
        dcode_add_line("auto_move.move_to(target_x, target_y); // calls move_to() on all objects in the move class.");
        dcode_state.indent_level --;
       dcode_state.indent_level --;
      dcode_state.indent_level --;
				}

 	dcode_state.indent_level --;
  dcode_add_line("}");
 	dcode_state.indent_level ++;
  dcode_add_line("else");
  dcode_add_line("{");
 	dcode_state.indent_level ++;


  if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_RETRO])
		{
    dcode_add_line("auto_move.approach_target(TARGET_MAIN,target_component,1200);");
    dcode_add_line(" // approach_target() approaches a target,");
    dcode_add_line(" //  but also tries to maintain a certain distance using retro (forward-facing) move objects.");
    dcode_add_line(" // Parameters are:");
    dcode_add_line(" //  - target's address in targetting memory");
    dcode_add_line(" //  - component of target process to attack");
    dcode_add_line(" //  - stand-off distance (in pixels)");
		}
		 else
			{
// should the process turn around and try to get away?
//   dcode_add_line("if (abs(target_y - core_y) + abs(target_x - core_x) < 1200)");
   dcode_add_line("if (process[TARGET_MAIN].distance_less(1200))");
   dcode_state.indent_level ++;
    dcode_add_line("auto_move.turn_to_target(TARGET_MAIN,target_component);");
    dcode_state.indent_level ++;
	    dcode_add_line("else");
     dcode_state.indent_level ++;
      dcode_add_line("auto_move.move_to(target_x, target_y); // calls move_to() on all objects in the move class.");
      dcode_state.indent_level --;
     dcode_state.indent_level --;
    dcode_state.indent_level --;
			} // end no retro

   dcode_add_line("if (process[TARGET_MAIN].distance_less(1600))");
   dcode_state.indent_level ++;
    dcode_add_line("auto_att_spike.fire_spike_at(TARGET_MAIN,target_component); // Calls fire_spike_at() on all spike objects in the forward spike class.");
    dcode_add_line(" // TARGET_MAIN indicates that the target is in targetting memory address TARGET_MAIN.");
    dcode_add_line(" // target_component is the component of the target being attacked.");
    dcode_add_line(" // uses some target prediction (although this is difficult with spikes).");
    dcode_state.indent_level --;
//			dcode_add_line("front_attack_primary = 1; // the forward directional attack class will not try to find its own target.");
//  this doesn't need to set front_attack_primary to 1. any front attack dir methods can find their own targets

 	dcode_state.indent_level --;
  dcode_add_line("} // end code for targets within scan range");
 	dcode_state.indent_level --;
 	dcode_state.indent_level --;

	} // end ATTACK_TYPE_LONG_RANGE

// Currently the above code should cover all possibilities (as without the ATTACK_MAIN or ATTACK_FORWARD_DIR classes this function shouldn't be called)

}


#endif


static void dcode_add_line(const char* add_line)
{
	dcode_newline();
	dcode_add_string(add_line);
//	fpr("\n [%i] %s", dcode_state.indent_level, add_line);
}

// currently this assumes that autocode will not try to add strings that are too long.
static void dcode_add_string(const char* add_string)
{

 strcat(dcode_state.ses->text [dcode_state.ses->line_index [dcode_state.source_line]], add_string);

}

/*

Not currently used, but could be:

static void dcode_add_number(int num)
{

	char num_string [10]; // should just be writing s16b anyway

	snprintf(num_string, 9, "%i", num);

	return dcode_add_string(num_string);

}
*/


static void dcode_newline(void)
{

 dcode_state.source_line++;
// dcode_state.cursor_pos = 0; - this is currently only used in d_code_header for reading text from the source

#ifdef SANITY_CHECK
 if (dcode_state.source_line >= SOURCE_TEXT_LINES)
 {
		fpr("\nError: d_code.c: dcode_newline(): too many source lines.");
		error_call();
 }
// This shouldn't happen unless autocoding gets much more complicated.
#endif

// now indent the new line.
 int i;

 for (i = 0; i < dcode_state.indent_level*2; i++)
	{
  dcode_state.ses->text [dcode_state.ses->line_index [dcode_state.source_line]] [i] = ' ';
	}

 dcode_state.ses->text [dcode_state.ses->line_index [dcode_state.source_line]] [i] = '\0';

}




/*

Not currently used, but could be:

static void dcode_open_brace(void)
{
	dcode_newline();
	dcode_add_string("{");
 dcode_state.indent_level ++;
}

// close_brace adds text afterwards because often we want to identify what the brace is closing
static void dcode_close_brace(const char* add_text)
{
	dcode_newline();
 dcode_state.indent_level --;
	dcode_add_string("} ");
	dcode_add_string(add_text);
}
*/












int dcode_error(const char* error_message)
{


     start_log_line(MLOG_COL_ERROR);
     write_to_log("Error occurred during autocoding.");
     finish_log_line();
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Error: ");
     write_to_log(error_message);
     finish_log_line();

//     dcode_state.error = error_type;
     return 0;


}


void dcode_warning(const char* warning_message)
{


     start_log_line(MLOG_COL_WARNING);
     write_to_log("Autocode warning: ");
     write_to_log(warning_message);
     finish_log_line();


}

