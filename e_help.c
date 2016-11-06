

#include <allegro5/allegro.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "c_header.h"
#include "e_header.h"
#include "m_globvars.h"
#include "e_complete.h"

#include "g_misc.h"
#include "e_log.h"
#include "e_help.h"
#include "e_editor.h"
#include "c_keywords.h"

//ALLEGRO_COLOR mlog_col [MLOG_COLS];

//extern char completion_table [KEYWORDS] [COMPLETION_TABLE_STRING_LENGTH];
//extern int completion_table_keyword_index [KEYWORDS];

//extern int alphabet_index [26]; // index to start point in completion table

extern struct identifierstruct identifier [IDENTIFIERS];

/*

This file contains things for the right-click help facility.
 (not currently implemented)

*/

const char* const help_string [] =
{
"", // HELP_NONE - does nothing when clicked

};

const char* keyword_help [] =
{
"", // KEYWORD_CORE_QUAD_A,
"", // KEYWORD_CORE_QUAD_B,
"", // KEYWORD_CORE_PENT_A,
"", // KEYWORD_CORE_PENT_B,
"", // KEYWORD_CORE_PENT_C,
"", // KEYWORD_CORE_HEX_A,
"", // KEYWORD_CORE_HEX_B,
"", // KEYWORD_CORE_HEX_C,
"", // KEYWORD_CORE_STATIC_QUAD,
"", // KEYWORD_CORE_STATIC_PENT,
"", // KEYWORD_CORE_STATIC_HEX_A,
"", // KEYWORD_CORE_STATIC_HEX_B,
"", // KEYWORD_CORE_STATIC_HEX_C,
"", // KEYWORD_COMPONENT_TRI,
"", // KEYWORD_COMPONENT_FORK,
"", // KEYWORD_COMPONENT_BOX,
"", // KEYWORD_COMPONENT_LONG4,
"", // KEYWORD_COMPONENT_CAP,
"", // KEYWORD_COMPONENT_PRONG,
"", // KEYWORD_COMPONENT_LONG5,
"", // KEYWORD_COMPONENT_PEAK,
"", // KEYWORD_COMPONENT_SNUB,
"", // KEYWORD_COMPONENT_BOWL,
"", // KEYWORD_COMPONENT_LONG6,
"", // KEYWORD_COMPONENT_DROP,
"", // KEYWORD_COMPONENT_SIDE,


"", // KEYWORD_OBJECT_NONE,
"", // KEYWORD_OBJECT_UPLINK,
"", // KEYWORD_OBJECT_DOWNLINK,
"", // KEYWORD_OBJECT_MOVE,
"", // KEYWORD_OBJECT_PULSE,
"", // KEYWORD_OBJECT_PULSE_L,
"", // KEYWORD_OBJECT_PULSE_XL,
"", // KEYWORD_OBJECT_BURST,
"", // KEYWORD_OBJECT_BURST_L,
"", // KEYWORD_OBJECT_BURST_XL,
"", // KEYWORD_OBJECT_BUILD,
"", // KEYWORD_OBJECT_INTERFACE,
//"", // KEYWORD_OBJECT_INTERFACE_DEPTH,
"", // KEYWORD_OBJECT_HARVEST,
"", // KEYWORD_OBJECT_STORAGE,
"", // KEYWORD_OBJECT_ALLOCATE,
"", // KEYWORD_OBJECT_STREAM,
"", // KEYWORD_OBJECT_STREAM_DIR,
"", // KEYWORD_OBJECT_SPIKE,
"", // KEYWORD_OBJECT_REPAIR,
"", // KEYWORD_OBJECT_REPAIR_OTHER,
"", // KEYWORD_OBJECT_ULTRA,
"", // KEYWORD_OBJECT_ULTRA_DIR,
"", // KEYWORD_OBJECT_SURGE,
"", // KEYWORD_OBJECT_STABILITY,

// compiler keywords:
"Declares a signed 16-bit integer variable.", // KEYWORD_C_INT, // code in ac_init.c assumes that this is the first compiler keyword in this enum.
"", // KEYWORD_C_IF,
"", // KEYWORD_C_ELSE,
"Returns from a gosub.", // KEYWORD_C_RETURN,
"Prints formatted text.", // KEYWORD_C_PRINTF,
"", // KEYWORD_C_WHILE,
"", // KEYWORD_C_DO,
"", // KEYWORD_C_FOR,
"", // KEYWORD_C_BREAK,
"", // KEYWORD_C_CONTINUE,
"", // KEYWORD_C_ENUM,
"", // KEYWORD_C_GOTO,
"Like goto, but can be returned.", // KEYWORD_C_GOSUB,
"", // KEYWORD_C_SWITCH,
"", // KEYWORD_C_CASE,
"", // KEYWORD_C_DEFAULT,
"Finishes execution for this cycle. Does not require brackets.", // KEYWORD_C_EXIT,
"Terminates this process (self-destruct). Does not require brackets.", // KEYWORD_C_TERMINATE,

"Starts a process method call (or a #process header).", // KEYWORD_C_PROCESS,
"Ends a #process header.", // KEYWORD_C_CODE,
"Used in component method calls.", // KEYWORD_C_COMPONENT,
"Used in specific (non-class) object calls.", // KEYWORD_C_OBJECT,
"Used to declare classes of objects, and to call classes by subscript.", // KEYWORD_C_CLASS,

"Object method for move objects.\nset_power(power)", // KEYWORD_OMETHOD_SET_POWER,
"Object method for move objects.\nmove_to(destination_x, destination_y)", // KEYWORD_OMETHOD_MOVE_TO,
"Object method for move objects.\nturn_to_xy(destination_x, destination_y)", // KEYWORD_OMETHOD_TURN_TO_XY,
"Object method for move objects.\nturn_to_angle(angle)", // KEYWORD_OMETHOD_TURN_TO_ANGLE,
"Object method for move objects.\nturn_to_target(target_index, component)", // KEYWORD_OMETHOD_TURN_TO_TARGET,
"Object method for move objects.\ntrack_target(target_index, component, attack_class)", // KEYWORD_OMETHOD_TRACK_TARGET,
"Object method for move objects.\napproach_xy(destination_x, destination_y, approach_distance)", // KEYWORD_OMETHOD_APPROACH_XY,
"Object method for move objects.\napproach_target(target_index, component, approach_distance)", // KEYWORD_OMETHOD_APPROACH_TARGET,
"Object method for move objects.\napproach_track(target_index, component, attacking_class, approach_distance)", // KEYWORD_OMETHOD_APPROACH_TRACK,

"Object method for pulse, burst and stream objects.\nfire(firing_delay)", // KEYWORD_OMETHOD_FIRE,
"Object method for rotating attack objects.\nrotate(angle_offset)", // KEYWORD_OMETHOD_ROTATE,
"Object method for rotating attack objects.\nno_target()", // KEYWORD_OMETHOD_NO_TARGET,
"Object method for rotating attack objects.\naim_at(target_index, component)", // KEYWORD_OMETHOD_AIM_AT,
"Object method for rotating attack objects.\nfire_at(target_index, component)", // KEYWORD_OMETHOD_FIRE_AT,
"Object method for move objects.\nintercept(target_index, component, attack_class)", // KEYWORD_OMETHOD_INTERCEPT,
"Object method for harvest objects.\ngather_data()", // KEYWORD_OMETHOD_GATHER_DATA,
"Object method for harvest objects.\ngive_data(target_index, data_amount)", // KEYWORD_OMETHOD_GIVE_DATA,
"Object method for allocate objects.\nallocate_data(data_amount)", // KEYWORD_OMETHOD_ALLOCATE_DATA,
"Object method for spike objects.\nfire_spike(angle_offset)", // KEYWORD_OMETHOD_FIRE_SPIKE,
"Object method for spike objects.\nfire_spike_at(target_index, component)", // KEYWORD_OMETHOD_FIRE_SPIKE_AT,
"Object method for spike objects.\nfire_spike_xy(x_target, y_target)", // KEYWORD_OMETHOD_FIRE_SPIKE_XY,
"Object method for interface objects.\nset_interface(setting)", // KEYWORD_OMETHOD_SET_INTERFACE,
"Object method for rotating attack objects.\nattack_scan(scan_angle, scan_distance, target_index)", // KEYWORD_OMETHOD_ATTACK_SCAN,
"Object method for rotating attack objects.\nattack_scan_aim(scan_angle, scan_distance, target_index)", // KEYWORD_OMETHOD_ATTACK_SCAN_AIM,
"Object method for stability object.\nset_stability(setting)", // KEYWORD_OMETHOD_SET_STABILITY,

"get_component_x()", // KEYWORD_MMETHOD_GET_COMPONENT_X,
"get_component_y()", // KEYWORD_MMETHOD_GET_COMPONENT_Y,
"component_exists()", // KEYWORD_MMETHOD_COMPONENT_EXISTS,
"get_integrity()", // KEYWORD_MMETHOD_GET_INTEGRITY,
"get_integrity_max()", // KEYWORD_MMETHOD_GET_INTEGRITY_MAX,

"get_core_x()", // KEYWORD_CMETHOD_GET_CORE_X,
"get_core_y()", // KEYWORD_CMETHOD_GET_CORE_Y,
"get_process_x()", // KEYWORD_CMETHOD_GET_PROCESS_X,
"get_process_y()", // KEYWORD_CMETHOD_GET_PROCESS_Y,
"get_core_angle()", // KEYWORD_CMETHOD_GET_CORE_ANGLE,
"get_core_spin()", // KEYWORD_CMETHOD_GET_CORE_SPIN,
"get_core_speed_x()", // KEYWORD_CMETHOD_GET_CORE_SPEED_X,
"get_core_speed_y()", // KEYWORD_CMETHOD_GET_CORE_SPEED_Y,
"get_interface_strength()", // KEYWORD_CMETHOD_GET_INTERFACE_STRENGTH,
"get_interface_capacity()", // KEYWORD_CMETHOD_GET_INTERFACE_CAPACITY,
"get_user()", // KEYWORD_CMETHOD_GET_USER,
"get_template()", // KEYWORD_CMETHOD_GET_TEMPLATE,
"distance()", // KEYWORD_CMETHOD_DISTANCE,
"distance_less(distance)", // KEYWORD_CMETHOD_DISTANCE_LESS,
"distance_more(distance)", // KEYWORD_CMETHOD_DISTANCE_MORE,
"target_angle()", // KEYWORD_CMETHOD_TARGET_ANGLE
"get_components()", // KEYWORD_CMETHOD_GET_COMPONENTS,
"get_components_max()", // KEYWORD_CMETHOD_GET_COMPONENTS_MAX,
"get_total_integrity()", // KEYWORD_CMETHOD_GET_TOTAL_INTEGRITY,
"get_total_integrity_max()", // KEYWORD_CMETHOD_GET_TOTAL_INTEGRITY_MAX,
"get_unharmed_integrity_max()", // KEYWORD_CMETHOD_GET_UNHARMED_INTEGRITY_MAX,
"visible()", // KEYWORD_CMETHOD_VISIBLE,
"target_signature(target_index)", // KEYWORD_CMETHOD_TARGET_SIGNATURE,

"scan_for_threat(x_offset, y_offset, target_index)", // KEYWORD_SMETHOD_SCAN_FOR_THREAT,
"check_point(x_offset, y_offset, target_index)", // KEYWORD_SMETHOD_CHECK_POINT,
"check_xy_visible(x_absolute, y_absolute)", // KEYWORD_SMETHOD_CHECK_XY_VISIBLE,
"get_command_type()", // KEYWORD_SMETHOD_GET_COMMAND_TYPE,
"get_command_x()", // KEYWORD_SMETHOD_GET_COMMAND_X,
"get_command_y()", // KEYWORD_SMETHOD_GET_COMMAND_Y,
"get_command_number()", // KEYWORD_SMETHOD_GET_COMMAND_NUMBER,
"get_command_ctrl()", // KEYWORD_SMETHOD_GET_COMMAND_CTRL,
"get_commands()", // KEYWORD_SMETHOD_GET_COMMANDS,
"clear_command()", // KEYWORD_SMETHOD_CLEAR_COMMAND,
"clear_all_commands()", // KEYWORD_SMETHOD_CLEAR_ALL_COMMANDS,
"get_command_target(target_index)", // KEYWORD_SMETHOD_GET_COMMAND_TARGET,
"get_command_target_component()", // KEYWORD_SMETHOD_GET_COMMAND_TARGET_COMPONENT,
"check_new_command()", // KEYWORD_SMETHOD_CHECK_NEW_COMMAND,


"build_from_queue(target_index)", // KEYWORD_SMETHOD_BUILD_FROM_QUEUE,
"check_build_queue()", // KEYWORD_SMETHOD_CHECK_BUILD_QUEUE,
"check_build_queue_front()", // KEYWORD_SMETHOD_CHECK_BUILD_QUEUE_FRONT,
"add_to_build_queue(template_index, build_x, build_y, angle, back_or_front, repeat)", // KEYWORD_SMETHOD_ADD_TO_BUILD_QUEUE,
"cancel_build_queue()", // KEYWORD_SMETHOD_CANCEL_BUILD_QUEUE,
"build_queue_get_template()", // KEYWORD_SMETHOD_BUILD_QUEUE_GET_TEMPLATE,
"build_queue_get_x()", // KEYWORD_SMETHOD_BUILD_QUEUE_GET_X,
"build_queue_get_y()", // KEYWORD_SMETHOD_BUILD_QUEUE_GET_Y,
"build_queue_get_angle()", // KEYWORD_SMETHOD_BUILD_QUEUE_GET_ANGLE,

/*
"check_new_build_command()", // KEYWORD_SMETHOD_CHECK_NEW_BUILD_COMMAND,
"check_build_command()", // KEYWORD_SMETHOD_CHECK_BUILD_COMMAND,
"get_build_command_x()", // KEYWORD_SMETHOD_GET_BUILD_COMMAND_X,
"get_build_command_y()", // KEYWORD_SMETHOD_GET_BUILD_COMMAND_Y,
"get_build_command_angle()", // KEYWORD_SMETHOD_GET_BUILD_COMMAND_ANGLE,
"get_build_command_template()", // KEYWORD_SMETHOD_GET_BUILD_COMMAND_TEMPLATE,
"get_build_command_ctrl()", // KEYWORD_SMETHOD_GET_BUILD_COMMAND_CTRL,
"clear_build_command()", // KEYWORD_SMETHOD_CALL_CLEAR_BUILD_COMMAND,
"clear_all_build_commands()", // KEYWORD_SMETHOD_CALL_CLEAR_ALL_BUILD_COMMANDS,*/
"charge_interface(charge_amount)", // KEYWORD_SMETHOD_CHARGE_INTERFACE,
"set_interface_general(setting)", // KEYWORD_SMETHOD_SET_INTERFACE_GENERAL,
"charge_interface_max()", // KEYWORD_SMETHOD_CHARGE_INTERFACE_MAX,

"check_selected()", // KEYWORD_SMETHOD_CHECK_SELECTED,
"check_selected_single()", // KEYWORD_SMETHOD_CHECK_SELECTED_SINGLE,
"get_available_data()", // KEYWORD_SMETHOD_GET_AVAILABLE_DATA,
"search_for_well()", // KEYWORD_SMETHOD_SEARCH_FOR_WELL,
"get_well_x()", // KEYWORD_SMETHOD_GET_WELL_X,
"get_well_y()", // KEYWORD_SMETHOD_GET_WELL_Y,
"get_well_data()", // KEYWORD_SMETHOD_GET_WELL_DATA,
"get_data_stored()", // KEYWORD_SMETHOD_GET_DATA_STORED,
"get_data_capacity()", // KEYWORD_SMETHOD_GET_DATA_CAPACITY,
"scan_single(x_offset, y_offset, target_index, friendly, components_min,\n  components_max, scan_bitfield)", // KEYWORD_SMETHOD_SCAN_SINGLE,
"scan_multi(x_offset, y_offset, target_index, number_of_targets, friendly,\n  components_min, components_max, scan_bitfield)", // KEYWORD_SMETHOD_SCAN_MULTI,
"get_power_capacity()", // KEYWORD_SMETHOD_GET_POWER_CAPACITY,
"get_power_used()", // KEYWORD_SMETHOD_GET_POWER_USED,
"get_power_left()", // KEYWORD_SMETHOD_GET_POWER_LEFT,
"set_debug_mode(setting)", // KEYWORD_SMETHOD_SET_DEBUG_MODE,

"transmit(target_index, priority, <message_0>, <message_1>, ...)", // KEYWORD_SMETHOD_TRANSMIT,
"broadcast(range, channel, priority, <message_0>, <message_1>, ...)", // KEYWORD_SMETHOD_BROADCAST,
"transmit_target(target_of_transmission, priority, target_to_transmit,\n  <message_0>, <message_1>, ...)", // KEYWORD_SMETHOD_TRANSMIT_TARGET,
"broadcast_target(range, channel, priority, target_to_broadcast,\n  <message_0>, <message_1>, ...)", // KEYWORD_SMETHOD_BROADCAST_TARGET,
"check_messages()", // KEYWORD_SMETHOD_CHECK_MESSAGES,
"get_message_type()", // KEYWORD_SMETHOD_GET_MESSAGE_TYPE,
"get_message_channel()", // KEYWORD_SMETHOD_GET_MESSAGE_CHANNEL,
"get_message_source(target_index)", // KEYWORD_SMETHOD_GET_MESSAGE_SOURCE,
"get_message_x()", // KEYWORD_SMETHOD_GET_MESSAGE_X,
"get_message_y()", // KEYWORD_SMETHOD_GET_MESSAGE_Y,
"get_message_target(target_index)", // KEYWORD_SMETHOD_GET_MESSAGE_TARGET,
"get_message_priority()", // KEYWORD_SMETHOD_GET_MESSAGE_PRIORITY,
"read_message()", // KEYWORD_SMETHOD_READ_MESSAGE,
"next_message()", // KEYWORD_SMETHOD_NEXT_MESSAGE,
"ignore_channel(channel)", // KEYWORD_SMETHOD_IGNORE_CHANNEL,
"listen_channel(channel)", // KEYWORD_SMETHOD_LISTEN_CHANNEL,
"ignore_all_channels()", // KEYWORD_SMETHOD_IGNORE_ALL_CHANNELS,
"copy_commands(target_index)", // KEYWORD_SMETHOD_COPY_COMMANDS,
"check_build_range(build_x, build_y)", // KEYWORD_SMETHOD_CHECK_BUILD_RANGE,
"repair_self()", // KEYWORD_SMETHOD_REPAIR_SELF,
"restore_self()", // KEYWORD_SMETHOD_RESTORE_SELF,
"repair_other(target_index)", // KEYWORD_SMETHOD_REPAIR_OTHER,
"repair_scan(x_offset, y_offset)", // KEYWORD_SMETHOD_REPAIR_SCAN,
"restore_other(target_index)", // KEYWORD_SMETHOD_RESTORE_OTHER,
"restore_scan(x_offset, y_offset)", // KEYWORD_SMETHOD_RESTORE_SCAN,
"build_process(template_index, x_offset, y_offset, angle, target_index)", // KEYWORD_SMETHOD_BUILD_PROCESS,
//"build_as_commanded(target_index)", // KEYWORD_SMETHOD_BUILD_AS_COMMANDED,
"build_repeat(target_index)", // KEYWORD_SMETHOD_BUILD_REPEAT,
"get_template_cost(template_index)", // KEYWORD_SMETHOD_GET_TEMPLATE_COST,
"random(modulo)", // KEYWORD_SMETHOD_RANDOM,

"check_contact(target_index)", // KEYWORD_SMETHOD_CHECK_CONTACT,
"get_damage()", // KEYWORD_SMETHOD_GET_DAMAGE,
"get_damage_source(target_index)", // KEYWORD_SMETHOD_GET_DAMAGE_SOURCE,
"distance_from_xy(x_target, y_target)", // KEYWORD_SMETHOD_DISTANCE_FROM_XY,
"distance_from_xy_less(x_target, y_target, distance)", // KEYWORD_SMETHOD_DISTANCE_LESS,
"distance_from_xy_more(x_target, y_target, distance)", // KEYWORD_SMETHOD_DISTANCE_MORE,
"distance_xy(x_component, y_component)", // KEYWORD_SMETHOD_DISTANCE_XY,
"target_clear(target_index)", // KEYWORD_SMETHOD_TARGET_CLEAR,
"target_compare(target_index_1, target_index_2)", // KEYWORD_SMETHOD_TARGET_COMPARE,
"target_copy(target_index_dest, target_index_source)", // KEYWORD_SMETHOD_TARGET_COPY,
"target_destroyed(target_index)", // KEYWORD_SMETHOD_TARGET_DESTROYED,
"attack_mode(setting)", // KEYWORD_SMETHOD_ATTACK_MODE,
"get_process_count()", // KEYWORD_SMETHOD_GET_PROCESS_COUNT,
"get_processes_max()", // KEYWORD_SMETHOD_GET_PROCESSES_MAX,
"get_processes_unused()", // KEYWORD_SMETHOD_GET_PROCESSES_UNUSED,
"get_component_count()", // KEYWORD_SMETHOD_GET_COMPONENT_COUNT,
"get_components_max()", // KEYWORD_SMETHOD_GET_COMPONENTS_MAX,
"get_components_unused()", // KEYWORD_SMETHOD_GET_COMPONENTS_UNUSED,

"sin(angle, multiplier)", // KEYWORD_UMETHOD_SIN,
"cos(angle, multiplier)", // KEYWORD_UMETHOD_COS,
"atan2(y_component, x_component)", // KEYWORD_UMETHOD_ATAN2,
"hypot(y_component, x_component)", // KEYWORD_UMETHOD_HYPOT,
"world_x()", // KEYWORD_UMETHOD_WORLD_X,
"world_y()", // KEYWORD_UMETHOD_WORLD_Y,
"abs(number)", // KEYWORD_UMETHOD_ABS,
"angle_difference(angle_1, angle_2)", // KEYWORD_UMETHOD_ANGLE_DIFFERENCE,
"arc_length(angle_1, angle_2)", // KEYWORD_UMETHOD_ARC_LENGTH,


};

void print_keyword_help(int keyword_index);

// gets a word at a location in a source_edit
// location must be valid (and not past the end of the line)
// returns length of the word (or 0 on failure)
void editor_help_click(struct source_edit_struct* se, int source_line, int source_pos)
{

 int check_pos = source_pos;

 char check_word [IDENTIFIER_MAX_LENGTH + 2] = "";

 char read_char;
 int read_char_type;
 int word_pos = 0;

 reset_log();

// first we look left from the cursor to find the start of the word:
fpr("\n finding:");
 while(TRUE)
	{
		read_char = se->text [se->line_index [source_line]] [check_pos];
		read_char_type = get_source_char_type(read_char);

		if (read_char_type != SCHAR_LETTER
   && read_char_type != SCHAR_NUMBER)
		{
			 check_pos ++;
			 break;
		}

		if (check_pos <= 0)
			break;

		check_pos --;
	}

// now we read the word in

 while(TRUE)
	{
		read_char = se->text [se->line_index [source_line]] [check_pos];
		read_char_type = get_source_char_type(read_char);

		if ((read_char_type != SCHAR_LETTER
    && read_char_type != SCHAR_NUMBER)
   || check_pos >= SOURCE_TEXT_LINE_LENGTH - 1)
   break;

		check_word [word_pos] = read_char;
		word_pos ++;
		check_word [word_pos] = 0;

		if (word_pos >= IDENTIFIER_MAX_LENGTH - 1)
			break;

		check_pos ++;

	}

	if (word_pos == 0)
	{
		write_line_to_log("No word found.", MLOG_COL_HELP);
		return;
	}


// int first_letter = check_word [0] - 'a';

 if (check_word [0] < 'a' || check_word [0] > 'z')
		goto unknown_word; // this makes it case-sensitive (currently all keywords start with lower case)

// int starting_pos = alphabet_index [first_letter];

// if (starting_pos == -1)
//		goto unknown_word; // no keyword starts with the same letter as check_word


 int i, j, found = 0;
/*
 for (i = starting_pos; i < KEYWORDS; i ++)
	{

  j = 0;
  found = 0;

  while(TRUE)
		{
   if (check_word [j] != completion_table [i] [j]
			 || completion_table [i] [j] == '\0')
		 {
		 	break;
		 }
		 j ++;
   if (j >= word_pos)
			{
				found = 1;
				break;
			}
		};

		if (found == 1
		 || completion_table [i] [0] != check_word [0])
			break;

	}
*/

 for (i = 0; i < KEYWORDS; i ++)
	{

  j = 0;
  found = 0;

  while(TRUE)
		{
   if (check_word [j] != identifier[i].name [j])
		 {
		 	break;
		 }
   if (identifier[i].name [j] == '\0')
			{
				found = 1;
				break;
			}
		 j ++;
		};

		if (found == 1)
			break;

	}

 if (found == 0)
		goto unknown_word;

	int keyword_index = i;

 start_log_line(MLOG_COL_HELP);
 write_to_log("Help: ");
 write_to_log(check_word);
 finish_log_line();
 switch(identifier[keyword_index].type)
 {
 	case CTOKEN_TYPE_IDENTIFIER_C_KEYWORD:
   write_line_to_log("Standard C keyword.", MLOG_COL_HELP);
   break;
 	case CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD:
   write_line_to_log("Standard keyword.", MLOG_COL_HELP);
   break;
 	case CTOKEN_TYPE_IDENTIFIER_CMETHOD:
   write_line_to_log("Process method.", MLOG_COL_HELP);
   break;
 	case CTOKEN_TYPE_IDENTIFIER_OMETHOD:
//   write_line_to_log("Object method.", MLOG_COL_HELP); - more detailed information is given in the strings above
   break;
 	case CTOKEN_TYPE_IDENTIFIER_CORE_SHAPE:
   write_line_to_log("Core type.", MLOG_COL_HELP);
   break;
 	case CTOKEN_TYPE_IDENTIFIER_MMETHOD:
   write_line_to_log("Component method.", MLOG_COL_HELP);
   break;
 	case CTOKEN_TYPE_IDENTIFIER_SMETHOD:
 	case CTOKEN_TYPE_IDENTIFIER_UMETHOD:
   write_line_to_log("Standard method.", MLOG_COL_HELP);
   break;
 	case CTOKEN_TYPE_IDENTIFIER_SHAPE:
   write_line_to_log("Non-core component type.", MLOG_COL_HELP);
   break;
 	case CTOKEN_TYPE_IDENTIFIER_OBJECT:
   write_line_to_log("Object type.", MLOG_COL_HELP);
   break;
  default:
   write_line_to_log("Unknown keyword type?", MLOG_COL_ERROR);
   return;

 }

 if (keyword_help [keyword_index] [0] != 0)
  print_keyword_help(keyword_index);

 return;



unknown_word:
 start_log_line(MLOG_COL_HELP);
 write_to_log("Unknown word (");
 write_to_log(check_word);
 write_to_log(")");
 finish_log_line();


}




void print_keyword_help(int keyword_index)
{

#ifdef SANITY_CHECK

 if (keyword_index < 0 || keyword_index >= KEYWORDS)
	{
		fprintf(stdout, "\nError: e_help.c: print_help(): invalid help string %i (should be 0 to %i).", keyword_index, KEYWORDS-1);
		error_call();
	}

#endif


// Need to go through and break down the help_string strings into printable bits:

// PRINT_STR_LENGTH is the number of characters that fit on a single line.
#define PRINT_STR_LENGTH 73

// if (help_type == HELP_NONE)
//		return; // does nothing at all (this is for things like clicking on a heading in the start menu

// reset_log();

 int i, j, k;
 char print_str [PRINT_STR_LENGTH] = "";
 char print_word [PRINT_STR_LENGTH] = "";

 i = 0;
 j = 0;
 k = 0;

		while(TRUE)
		{

		 print_word [k] = keyword_help [keyword_index] [i];
			if (keyword_help [keyword_index] [i] == ' ')
			{
				if (k + j >= PRINT_STR_LENGTH - 1)
				{
					write_line_to_log(print_str, MLOG_COL_HELP);
					print_word [k + 1] = '\0';
			  strcpy(print_str, print_word);
					j = k + 1;
			  k = -1;
				}
				 else
					{
 					print_word [k + 1] = '\0';
						strcat(print_str, print_word);
						j += k + 1;
						k = -1;
					}
			}

			if (keyword_help [keyword_index] [i] == '\n'
				|| keyword_help [keyword_index] [i] == '\0')
			{
				if (k + j >= PRINT_STR_LENGTH - 1)
				{
					write_line_to_log(print_str, MLOG_COL_HELP);
					print_word [k] = '\0';
			  strcpy(print_str, print_word);
					write_line_to_log(print_str, MLOG_COL_HELP);
					print_word [0] = '\0';
					print_str [0] = '\0';
					j = 0;//k;
			  k = -1;
				}
				 else
					{
 					print_word [k] = '\0';
						strcat(print_str, print_word);
 					write_line_to_log(print_str, MLOG_COL_HELP);
 					print_str [0] = '\0';
						j = 0;
						k = -1;
					}
			}
			if (keyword_help [keyword_index] [i] == '\0')
			{
				if (strlen(print_str) != 0)
					write_line_to_log(print_str, MLOG_COL_HELP);
				break;
			}

		 i ++;
		 k ++;

		};




}


/*

void print_help(int help_type)
{

#ifdef SANITY_CHECK

 if (help_type < 0 || help_type >= HELP_STRINGS)
	{
		fprintf(stdout, "\nError: e_help.c: print_help(): invalid help string %i (should be 0 to %i).", help_type, HELP_STRINGS);
		error_call();
	}

#endif

// Need to go through and break down the help_string strings into printable bits:

// PRINT_STR_LENGTH is the number of characters that fit on a single line.
#define PRINT_STR_LENGTH 73

// if (help_type == HELP_NONE)
//		return; // does nothing at all (this is for things like clicking on a heading in the start menu

 reset_log();

 int i, j, k;
 char print_str [PRINT_STR_LENGTH] = "";
 char print_word [30] = "";

 i = 0;
 j = 0;
 k = 0;

		while(TRUE)
		{
		 print_word [k] = help_string [help_type] [i];
			if (help_string [help_type] [i] == ' ')
			{
				if (k + j >= PRINT_STR_LENGTH - 1)
				{
					write_line_to_log(print_str, MLOG_COL_HELP);
					print_word [k + 1] = '\0';
			  strcpy(print_str, print_word);
					j = k + 1;
			  k = -1;
				}
				 else
					{
 					print_word [k + 1] = '\0';
						strcat(print_str, print_word);
						j += k + 1;
						k = -1;
					}
			}
			if (help_string [help_type] [i] == '\n'
				|| help_string [help_type] [i] == '\0')
			{
				if (k + j >= PRINT_STR_LENGTH - 1)
				{
					write_line_to_log(print_str, MLOG_COL_HELP);
					print_word [k] = '\0';
			  strcpy(print_str, print_word);
					write_line_to_log(print_str, MLOG_COL_HELP);
					print_word [0] = '\0';
					print_str [0] = '\0';
					j = 0;//k;
			  k = -1;
				}
				 else
					{
 					print_word [k] = '\0';
						strcat(print_str, print_word);
 					write_line_to_log(print_str, MLOG_COL_HELP);
 					print_str [0] = '\0';
						j = 0;
						k = -1;
					}
			}
			if (help_string [help_type] [i] == '\0')
			{
				if (strlen(print_str) != 0)
					write_line_to_log(print_str, MLOG_COL_HELP);
				break;
			}

		 i ++;
		 k ++;

		};




}

*/
