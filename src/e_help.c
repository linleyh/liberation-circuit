

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
"", // KEYWORD_OBJECT_SLICE,
"", // KEYWORD_OBJECT_STABILITY,

// compiler keywords:
"Declares a signed 16-bit integer variable.", // KEYWORD_C_INT, // code in ac_init.c assumes that this is the first compiler keyword in this enum.
"Works like the standard C if keyword.", // KEYWORD_C_IF,
"Works like the standard C else keyword.", // KEYWORD_C_ELSE,
"Returns from a gosub.", // KEYWORD_C_RETURN,
"Prints formatted text.\nFormatting accepts \\n and %i.", // KEYWORD_C_PRINTF,
"Prints formatted text to a bubble.\nOtherwise works like printf.", // KEYWORD_C_BUBBLEF,
"Works like the standard C while keyword.", // KEYWORD_C_WHILE,
"Works like the standard C do keyword.", // KEYWORD_C_DO,
"Works like the standard C for keyword.", // KEYWORD_C_FOR,
"Breaks from a loop or switch.", // KEYWORD_C_BREAK,
"Continues a loop.", // KEYWORD_C_CONTINUE,
"Declares an enum.", // KEYWORD_C_ENUM,
"Works like the standard C goto keyword.", // KEYWORD_C_GOTO,
"Like goto, but can be returned.", // KEYWORD_C_GOSUB,
"Works like the standard C switch keyword.", // KEYWORD_C_SWITCH,
"Works like the standard C case keyword.", // KEYWORD_C_CASE,
"Works like the standard C default keyword.", // KEYWORD_C_DEFAULT,
"Finishes execution for this cycle. Does not require brackets.\n", // KEYWORD_C_EXIT,
"Terminates this process (self-destructs). Does not require brackets.\n(Use exit to just cease execution for this cycle.)", // KEYWORD_C_TERMINATE,

"Starts a process method call (or a #process header).\nExample: process[TARGET_MAIN].get_core_x()", // KEYWORD_C_PROCESS,
"A #code line ends a #process header.", // KEYWORD_C_CODE,
"Used in component method calls.\nExample: process[TARGET_MAIN].component[3].get_component_x()", // KEYWORD_C_COMPONENT,
"Used in specific (non-class) object calls.", // KEYWORD_C_OBJECT,
"Used to declare classes of objects, and to call classes by subscript.\nExample: class[2].set_power(0)", // KEYWORD_C_CLASS,

"Object method for move objects.\nset_power(power)\nSets power of move objects (power should be 0 to 10).", // KEYWORD_OMETHOD_SET_POWER,
"Object method for move objects.\nmove_to(destination_x, destination_y)\nTries to use move objects to travel to the destination.", // KEYWORD_OMETHOD_MOVE_TO,
"Object method for move objects.\nturn_to_xy(destination_x, destination_y)\nUses move objects to turn towards the destination.", // KEYWORD_OMETHOD_TURN_TO_XY,
"Object method for move objects.\nturn_to_angle(angle)\nUses move objects to turn towards the specified angle (0 to 8192).", // KEYWORD_OMETHOD_TURN_TO_ANGLE,
"Object method for move objects.\nturn_to_target(target_index, component)\nUses move objects to turn towards a target.\nUse component 0 to target the core.\nExample: auto_move.turn_to_target(TARGET_MAIN, 0)", // KEYWORD_OMETHOD_TURN_TO_TARGET,
"Object method for move objects.\ntrack_target(target_index, component, attack_class)\nUses move objects to aim one or more fixed attacking objects at a target, with some target leading.\nattack_class is the class of the attacking objects.", // KEYWORD_OMETHOD_TRACK_TARGET,
"Object method for move objects.\napproach_xy(destination_x, destination_y, approach_distance)\nUses move objects to approach to within approach_distance pixels of the destination.", // KEYWORD_OMETHOD_APPROACH_XY,
"Object method for move objects.\napproach_target(target_index, component, approach_distance)\nUses move objects to approach to within approach_distance pixels of the target.", // KEYWORD_OMETHOD_APPROACH_TARGET,
"Object method for move objects.\napproach_track(target_index, component, attack_class, approach_distance)\nUses move objects to approach to within approach_distance pixels of the target, and aim at it with fixed attacking objects.\nattack_class should be a class with fixed attacking objects.", // KEYWORD_OMETHOD_APPROACH_TRACK,
"Object method for move objects.\nreposition(destination_x, destination_y, facing_angle)\nUses move objects to try to reposition the process the specified location and facing. Can be used to keep in formation.\nRequires at least one retro move object to work properly.", // KEYWORD_OMETHOD_REPOSITION,

"Object method for pulse, burst and stream objects.\nfire(firing_delay)\nSets objects to fire in a specified number of ticks (0 to 15).", // KEYWORD_OMETHOD_FIRE,
"Object method for rotating attack objects.\nrotate(angle_offset)\nRotates objects to a specified offset (-2048 to 2048) from their zero angle.", // KEYWORD_OMETHOD_ROTATE,
"Object method for rotating attack objects.\nno_target()\nRotates objects back to their default angle.", // KEYWORD_OMETHOD_NO_TARGET,
"Object method for rotating attack objects.\naim_at(target_index, component)\nAims objects at target, but does not fire.\ncomponent is the component of the target to aim at (0 for core).", // KEYWORD_OMETHOD_AIM_AT,
"Object method for rotating attack objects.\nfire_at(target_index, component)\nAims objects at target, then fires.\ncomponent is the component of the target to aim at (0 for core).", // KEYWORD_OMETHOD_FIRE_AT,
"Object method for move objects.\nintercept(target_index, component, attack_class)\nUses move objects to move towards a target and aim a class of fixed attacking objects at it.\nExample: auto_move.intercept(TARGET_MAIN, 0, auto_att_main)", // KEYWORD_OMETHOD_INTERCEPT,
"Object method for harvest objects.\ngather_data()\nGathers data from a nearby data well.\nReturns the amount of data gathered, or -1 if there is no well within range.", // KEYWORD_OMETHOD_GATHER_DATA,
"Object method for harvest objects.\ngive_data(target_index, data_amount)\nTransfers data to another process within scanning range. Returns the amount transferred, or -1 if the target is not within range.", // KEYWORD_OMETHOD_GIVE_DATA,
"Object method for harvest objects.\ntake_data(target_index, data_amount)\nTakes data from a friendly process within scanning range. Returns the amount transferred, or -1 if the target is not within range.", // KEYWORD_OMETHOD_TAKE_DATA,
"Object method for allocate objects.\nallocate_data(data_amount)\nAllocates a specified amount of data stored in storage objects, so that it can be used to build new processes.\nThe maximum rate is 3 data per allocate object per cycle.\nIf data_amount is more than 3, the object will just try to allocate 3.", // KEYWORD_OMETHOD_ALLOCATE_DATA,
"Object method for spike objects.\nfire_spike(angle_offset)\nFires a spike at a specified angle offset (-2048 to 2048).\nNot recommended; use fire_spike_at or fire_spike_xy instead.", // KEYWORD_OMETHOD_FIRE_SPIKE,
"Object method for spike objects.\nfire_spike_at(target_index, component)\nFires a spike at the specified target and component (set component to 0 to target the core).", // KEYWORD_OMETHOD_FIRE_SPIKE_AT,
"Object method for spike objects.\nfire_spike_xy(x_target, y_target)\nFires a spike at the specified location.\nx_target and y_target are absolute values.", // KEYWORD_OMETHOD_FIRE_SPIKE_XY,
//"Object method for interface objects.\nset_interface(setting)\nSets the ", // KEYWORD_OMETHOD_SET_INTERFACE,
"Object method for rotating attack objects.\nattack_scan(scan_angle, scan_distance, target_index)\nScans for a target and, if one is found, fires at it.\nscan_angle (an offset from core angle) and scan_distance are a vector to the centre of the scan. The target is stored in targettng memory at target_index.\nReturns 1 if a target is found, 0 otherwise.", // KEYWORD_OMETHOD_ATTACK_SCAN,
"Object method for rotating attack objects.\nattack_scan_aim(scan_angle, scan_distance, target_index)\nLike attack_scan, but objects only aim and do not fire.", // KEYWORD_OMETHOD_ATTACK_SCAN_AIM,
"Object method for stability object.\nset_stability(setting)\nTurns a stability object off (setting = 0) or on (setting != 0). Turning a stability object off can save power.", // KEYWORD_OMETHOD_SET_STABILITY,

"get_component_x()\nReturns the (absolute) x coordinate of the component.", // KEYWORD_MMETHOD_GET_COMPONENT_X,
"get_component_y()\nReturns the (absolute) y coordinate of the component.", // KEYWORD_MMETHOD_GET_COMPONENT_Y,
"component_exists()\nReturns 1 if the component exists, 0 if it does not.", // KEYWORD_MMETHOD_COMPONENT_EXISTS,
"get_integrity()\nReturns the integrity of the component.\n(Use the get_total_integrity() process method for the process' total integrity.)", // KEYWORD_MMETHOD_GET_INTEGRITY,
"get_integrity_max()\nReturns the maximum integrity of the component.", // KEYWORD_MMETHOD_GET_INTEGRITY_MAX,
"get_component_hit()\nReturns 1 if the component was hit by an attack in the previous cycle (including if the hit was absorbed by an interface).", // KEYWORD_MMETHOD_GET_COMPONENT_HIT,
"get_component_hit_source(target_index)\nReturns 1 if the component was hit by an attack in the previous cycle (including if the hit was absorbed by an interface), and saves the source of the hit to targetting memory at target_index.", // KEYWORD_MMETHOD_GET_COMPONENT_HIT_SOURCE,

"get_core_x()\nReturns the (absolute) x coordinate of the process' core.", // KEYWORD_CMETHOD_GET_CORE_X,
"get_core_y()\nReturns the (absolute) y coordinate of the process' core.", // KEYWORD_CMETHOD_GET_CORE_Y,
"get_process_x()\nReturns the (absolute) x coordinate of the process' centre of inertia.", // KEYWORD_CMETHOD_GET_PROCESS_X,
"get_process_y()\nReturns the (absolute) y coordinate of the process' centre of inertia", // KEYWORD_CMETHOD_GET_PROCESS_Y,
"get_core_angle()\nReturns the angle of the target. This is the angle indicated by 'Front' in the process designer, and not necessarily the angle of the core's 0 link.", // KEYWORD_CMETHOD_GET_CORE_ANGLE,
"get_core_spin()\nReturns the process' rate of spin, in angle units per cycle (not per tick).", // KEYWORD_CMETHOD_GET_CORE_SPIN,
"get_core_speed_x()\nReturns the x component of the process' speed, in pixels per cycle (not per tick).", // KEYWORD_CMETHOD_GET_CORE_SPEED_X,
"get_core_speed_y()\nReturns the y component of the process' speed, in pixels per cycle (not per tick).", // KEYWORD_CMETHOD_GET_CORE_SPEED_Y,
"get_interface_strength()\nReturns the current strength of the process' interface.", // KEYWORD_CMETHOD_GET_INTERFACE_STRENGTH,
"get_interface_capacity()\nReturns the maximum strength of the process' interface.", // KEYWORD_CMETHOD_GET_INTERFACE_CAPACITY,
"get_user()\nReturns the user controlling the process (e.g. 0 for player 0, 1 for player 1, etc.)", // KEYWORD_CMETHOD_GET_USER,
"get_template()\nReturns the template the process was built from.", // KEYWORD_CMETHOD_GET_TEMPLATE,
"distance()\nReturns the distance between the calling process and the target process.", // KEYWORD_CMETHOD_DISTANCE,
"distance_less(distance)\nReturns 1 if the target process is closer than the specified distance from the calling process, or 0 otherwise.", // KEYWORD_CMETHOD_DISTANCE_LESS,
"distance_more(distance)\nReturns 1 if the target process is further than the specified distance from the calling process, or 0 otherwise.", // KEYWORD_CMETHOD_DISTANCE_MORE,
"target_angle()\nReturns the angle from the calling process to the target process. This is an expensive operation.", // KEYWORD_CMETHOD_TARGET_ANGLE
"get_components()\nReturns the number of components the target process has.", // KEYWORD_CMETHOD_GET_COMPONENTS,
"get_components_max()\nReturns the number of components the target process would have if it were undamaged.", // KEYWORD_CMETHOD_GET_COMPONENTS_MAX,
"get_total_integrity()\nReturns the current total integrity of the process' components.", // KEYWORD_CMETHOD_GET_TOTAL_INTEGRITY,
"get_total_integrity_max()\nReturns the current maximum integrity of the process' components (not including any destroyed components).", // KEYWORD_CMETHOD_GET_TOTAL_INTEGRITY_MAX,
"get_unharmed_integrity_max()\nReturns the current maximum integrity of all of the process' components (including any destroyed components).", // KEYWORD_CMETHOD_GET_UNHARMED_INTEGRITY_MAX,
"visible()\nReturns 1 if the process is visible (i.e. within scanning range of any friendly process), 0 otherwise.\nAlways returns 1 for friendly processes.", // KEYWORD_CMETHOD_VISIBLE,
"target_signature()\nReturns the signature bitfield of the process.", // KEYWORD_CMETHOD_TARGET_SIGNATURE,

"scan_for_threat(x_offset, y_offset, target_index)\nScans for a nearby enemy; if found, the enemy nearest to the centre of the scan (set by x/y_offset) is recorded in target_index, and the method returns 1. Otherwise, returns 0.", // KEYWORD_SMETHOD_SCAN_FOR_THREAT,
"check_point(x_offset, y_offset, target_index)\nIf a process is at x_offset,y_offset (these are offsets from the calling process' position), returns 1 and records the target at target_index. Returns -1 on error. Otherwise, returns 0.", // KEYWORD_SMETHOD_CHECK_POINT,
"check_xy_visible(x_absolute, y_absolute)\nReturns 1 if the point at x/y_absolute is within scanning range of any friendly process. Returns 0 otherwise.", // KEYWORD_SMETHOD_CHECK_XY_VISIBLE,
"get_command_type()\n(Command mode only)\nReturns the type of the command at the front of the command queue (see the COM_* enum in any autocoded process for the codes).", // KEYWORD_SMETHOD_GET_COMMAND_TYPE,
"get_command_x()\n(Command mode only)\nReturns the x coordinate of the command at the front of the command queue (e.g. the x coordinate of the destination for a move command).", // KEYWORD_SMETHOD_GET_COMMAND_X,
"get_command_y()\n(Command mode only)\nReturns the y coordinate of the command at the front of the command queue (e.g. the y coordinate of the destination for a move command).", // KEYWORD_SMETHOD_GET_COMMAND_Y,
"get_command_number()\n(not currently used)", // KEYWORD_SMETHOD_GET_COMMAND_NUMBER,
"get_command_ctrl()\n(Command mode only)\nReturns 1 if the user pressed control while issuing the command at the front of the command queue. Returns 0 otherwise.", // KEYWORD_SMETHOD_GET_COMMAND_CTRL,
"get_commands()\n(Command mode only)\nReturns the number of commands on the command queue for this process.", // KEYWORD_SMETHOD_GET_COMMANDS,
"clear_command()\n(Command mode only)\nClears the command at the front of the command queue. If there is another command on the queue, it moves to the front and is treated as a new command for the purposes of check_new_command().", // KEYWORD_SMETHOD_CLEAR_COMMAND,
"clear_all_commands()\n(Command mode only)\nClears the entire command queue for this process.", // KEYWORD_SMETHOD_CLEAR_ALL_COMMANDS,
"get_command_target(target_index)\n(Command mode only)\nSaves the target of the current command (e.g. the target of an attack command) in targetting memory at address target_index.", // KEYWORD_SMETHOD_GET_COMMAND_TARGET,
"get_command_target_component()\n(Command mode only)\nIf the current command has a target (e.g. an attack command), returns the selected component of the target process (0 for core).", // KEYWORD_SMETHOD_GET_COMMAND_TARGET_COMPONENT,
"check_new_command()\n(Command mode only)\nReturns 1 if a new command has been issued since the last cycle, or if clear_command() has resulted in a new commmand being at the front of the queue.", // KEYWORD_SMETHOD_CHECK_NEW_COMMAND,


"build_from_queue(target_index)\nIf the build order at the front of the build queue belongs to this process, tries to build it. Returns 1 and saves the new process to target_index if successful (see the manual for failure codes).", // KEYWORD_SMETHOD_BUILD_FROM_QUEUE,
"check_build_queue()\nReturns the number of build orders on the build queue that belong to this process.", // KEYWORD_SMETHOD_CHECK_BUILD_QUEUE,
"check_build_queue_front()\nReturns 1 if there is a build order for this process at the front of the build queue.", // KEYWORD_SMETHOD_CHECK_BUILD_QUEUE_FRONT,
"add_to_build_queue(template_index, build_x, build_y, angle, back_or_front, repeat)\nAdds a build command to the build queue for this process. See the manual for details.", // KEYWORD_SMETHOD_ADD_TO_BUILD_QUEUE,
"cancel_build_queue()\nClears all build commands for this process on the build queue.", // KEYWORD_SMETHOD_CANCEL_BUILD_QUEUE,
"build_queue_get_template()\nReturns the template index of the first build order on the build queue that belongs to this process. Returns -1 if none found.", // KEYWORD_SMETHOD_BUILD_QUEUE_GET_TEMPLATE,
"build_queue_get_x()\nReturns the x location of the first build order on the build queue that belongs to this process. Returns -1 if none found.", // KEYWORD_SMETHOD_BUILD_QUEUE_GET_X,
"build_queue_get_y()\nReturns the y location of the first build order on the build queue that belongs to this process. Returns -1 if none found.", // KEYWORD_SMETHOD_BUILD_QUEUE_GET_Y,
"build_queue_get_angle()\nReturns the angle of the first build order on the build queue that belongs to this process. Returns -1 if none found.", // KEYWORD_SMETHOD_BUILD_QUEUE_GET_ANGLE,

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
"charge_interface(charge_amount)\nTries to charge the process' interface by a specified amount. Probably better to just use charge_interface_max() instead.", // KEYWORD_SMETHOD_CHARGE_INTERFACE,
"set_interface_general(setting)\nTurns the process' interface on (1) or off (0).", // KEYWORD_SMETHOD_SET_INTERFACE_GENERAL,
"charge_interface_max()\nAttempts to charge the process' interface by the maximum amount (which depends on the core type and the number of interface objects). Returns the number of points charged.", // KEYWORD_SMETHOD_CHARGE_INTERFACE_MAX,

"check_selected()\n(Command mode only)\nReturns 1 if this process is selected by the user controlling it.", // KEYWORD_SMETHOD_CHECK_SELECTED,
"check_selected_single()\n(Command mode only)\nReturns 1 if this process is selected by the user controlling it, and no other processes are selected.", // KEYWORD_SMETHOD_CHECK_SELECTED_SINGLE,
"get_available_data()\nReturns the amount of data available to the user controlling this process (only counts allocated data).", // KEYWORD_SMETHOD_GET_AVAILABLE_DATA,
"search_for_well()\nSearches for a data well in scanning range. Returns 1 if found, -1 on error, 0 otherwise.\nThe process must have either a harvest object or a build object.", // KEYWORD_SMETHOD_SEARCH_FOR_WELL,
"get_well_x()\nReturns the x coordinate of the nearest data well that is within scanning range. Returns 1 if found, -1 on error, 0 otherwise.\nThe process must have either a harvest object or a build object.", // KEYWORD_SMETHOD_GET_WELL_X,
"get_well_y()\nReturns the y coordinate of the nearest data well that is within scanning range. Returns 1 if found, -1 on error, 0 otherwise.\nThe process must have either a harvest object or a build object.", // KEYWORD_SMETHOD_GET_WELL_Y,
"get_well_data()\nReturns the remaining data of the nearest data well that is within scanning range. Returns -1 on error, 0 otherwise.\nThe process must have either a harvest object or a build object.", // KEYWORD_SMETHOD_GET_WELL_DATA,
"get_data_stored()\nReturns the amount of data this process has stored in its storage objects.", // KEYWORD_SMETHOD_GET_DATA_STORED,
"get_data_capacity()\nReturns the total capacity of the process' storage objects.", // KEYWORD_SMETHOD_GET_DATA_CAPACITY,
"scan_single(x_offset, y_offset, target_index, friendly, components_min,\n  components_max, scan_bitfield)\nScans for a single nearby target. Returns 1 if found, 0 otherwise. See the manual for more details.", // KEYWORD_SMETHOD_SCAN_SINGLE,
"scan_multi(x_offset, y_offset, target_index, number_of_targets, friendly,\n  components_min, components_max, scan_bitfield)\nScans for multiple nearby targets. Returns the number found. See the manual for more details.", // KEYWORD_SMETHOD_SCAN_MULTI,
"get_power_capacity()\nReturns this process' total power capacity (use get_power_left() for current remaining power).", // KEYWORD_SMETHOD_GET_POWER_CAPACITY,
"get_power_used()\nReturns the power used by the process so far this cycle.", // KEYWORD_SMETHOD_GET_POWER_USED,
"get_power_left()\nReturns the power remaining for use by the process this cycle.", // KEYWORD_SMETHOD_GET_POWER_LEFT,
"get_instructions_left()\nReturns the instructions (used for executing code and performing calculations) remaining for use by the process this cycle.", // KEYWORD_SMETHOD_GET_INSTRUCTIONS_LEFT,
"set_debug_mode(setting)\nSets debug mode on (0) or on (1) for this process. If on, errors caused by this process will be displayed in the console.", // KEYWORD_SMETHOD_SET_DEBUG_MODE,

"transmit(target_index, priority, <message_0>, <message_1>, ...)\nSends a message to a single friendly target. Returns 1 on success, 0 on failure. See the manual for more information.", // KEYWORD_SMETHOD_TRANSMIT,
"broadcast(range, channel, priority, <message_0>, <message_1>, ...)\nBroadcasts a message to any friendly targets within range that are listening to the specified channel. For unlimited range, set range to -1. See the manual for more information.", // KEYWORD_SMETHOD_BROADCAST,
"transmit_target(target_of_transmission, priority, target_to_transmit,\n  <message_0>, <message_1>, ...)\nSends a message to a single friendly target, with a target attached to the message. Returns 1 on success, 0 on failure. See the manual for more information.", // KEYWORD_SMETHOD_TRANSMIT_TARGET,
"broadcast_target(range, channel, priority, target_to_broadcast,\n  <message_0>, <message_1>, ...)\nLike broadcast(), but with an attached target. Returns 1 on success, 0 on failure. See the manual for more information.", // KEYWORD_SMETHOD_BROADCAST_TARGET,
"check_messages()\nReturns the number of unread messages the process has received since the last cycle (not including any messages retrieved by next_message()).", // KEYWORD_SMETHOD_CHECK_MESSAGES,
"get_message_type()\nReturns the way the current message was sent (0=transmit, 1=transmit_target, 2=broadcast, 3=broadcast_target).\nRequires next_message() to be called first.", // KEYWORD_SMETHOD_GET_MESSAGE_TYPE,
"get_message_channel()\nReturns the channel the current message was sent on.\nRequires next_message() to be called first.", // KEYWORD_SMETHOD_GET_MESSAGE_CHANNEL,
"get_message_source(target_index)\nSaves the source of the current message to targetting memory at target_index.\nRequires next_message() to be called first.", // KEYWORD_SMETHOD_GET_MESSAGE_SOURCE,
"get_message_x()\nReturns the (absolute) x coordinate of the sender of the current message, when it was sent.\nRequires next_message() to be called first.", // KEYWORD_SMETHOD_GET_MESSAGE_X,
"get_message_y()\nReturns the (absolute) y coordinate of the sender of the current message, when it was sent.\nRequires next_message() to be called first.", // KEYWORD_SMETHOD_GET_MESSAGE_Y,
"get_message_target(target_index)\nIf the current message has a target attached, saves the target to targetting memory at target_index and returns 1. Otherwise, returns 0.\nRequires next_message() to be called first.", // KEYWORD_SMETHOD_GET_MESSAGE_TARGET,
"get_message_priority()\nReturns the priority (0 or 1) of the current message.\nRequires next_message() to be called first.", // KEYWORD_SMETHOD_GET_MESSAGE_PRIORITY,
"read_message()\nReturns the contents of the current message. Message values are read sequentially.\nRequires next_message() to be called first.", // KEYWORD_SMETHOD_READ_MESSAGE,
"next_message()\nStarts reading the next message on the queue. Returns 1 if there is a next message, 0 otherwise. Must be called before any messages can be read.", // KEYWORD_SMETHOD_NEXT_MESSAGE,
"ignore_channel(channel)\nStop listening to a channel.", // KEYWORD_SMETHOD_IGNORE_CHANNEL,
"listen_channel(channel)\nStart listening to a channel. This method only needs to be called once (e.g. during initialisation) and the process will continue to listen until stopped by ignore_channel().", // KEYWORD_SMETHOD_LISTEN_CHANNEL,
"ignore_all_channels()\nStop listening to any channels.", // KEYWORD_SMETHOD_IGNORE_ALL_CHANNELS,
"copy_commands(target_index)\n(Command mode only)\nCopies this process' command queue to another process. Autocoded builder processes use this to copy their command queue to newly build processes.", // KEYWORD_SMETHOD_COPY_COMMANDS,
"give_command(target_index, command_type, x, y, command_target, component, queued, control)\n(Both command mode and autonomous mode)\nGives a command to a target friendly process. See the manual for more information", // KEYWORD_SMETHOD_GIVE_COMMAND,
"give_build_command(target_index, template, x, y, angle, back_or_front, repeat, queued)\n(Both command mode and autonomous mode)\nGives a build command to a target friendly process. See the manual for more information", // KEYWORD_SMETHOD_GIVE_BUILD_COMMAND,

"check_build_range(build_x, build_y)\nReturns 1 if the location at (absolute) build_x/y is close enough for this process to build a new process there (build range is equal to visual/scanner range).", // KEYWORD_SMETHOD_CHECK_BUILD_RANGE,
"repair_self()\nAttempts to repair a damaged component of the process.", // KEYWORD_SMETHOD_REPAIR_SELF,
"restore_self()\nAttempts to restore a destroyed component of the process. After restoring a component, the repair object(s) cannot restore another one (but can repair) for a period determined by the data cost of the restored component.", // KEYWORD_SMETHOD_RESTORE_SELF,
"repair_other(target_index)\nRequires a repair_other object.\nAttempts to repair a destroyed component of another process.", // KEYWORD_SMETHOD_REPAIR_OTHER,
"repair_scan(x_offset, y_offset)\nRequires a repair_other object.\nAttempts to repair another process within scanning range. x/y_offset determine the centre of the scan (processes closest to the centre get repaired first).", // KEYWORD_SMETHOD_REPAIR_SCAN,
"restore_other(target_index)\nRequires a repair_other object.\nLike repair_other, but restores a destroyed component.", // KEYWORD_SMETHOD_RESTORE_OTHER,
"restore_scan(x_offset, y_offset)\nRequires a repair_other object.\nLike repair_scan, but restores a destroyed component.", // KEYWORD_SMETHOD_RESTORE_SCAN,
"build_process(template_index, x_offset, y_offset, angle, target_index)\nRequires a build object.\nTries to build a new process, ignoring the build queue. See the manual for more information.", // KEYWORD_SMETHOD_BUILD_PROCESS,
//"build_as_commanded(target_index)", // KEYWORD_SMETHOD_BUILD_AS_COMMANDED,
"build_repeat(target_index)\nRequires a build object.\nRepeats the most recent build_process() call.", // KEYWORD_SMETHOD_BUILD_REPEAT,
"get_template_cost(template_index)\nReturns the data cost of a template. Returns 0 if the template is empty or otherwise invalid.", // KEYWORD_SMETHOD_GET_TEMPLATE_COST,
"random(modulo)\nReturns a (pseudo)random number between 0 and (modulo - 1). Modulo should be positive.", // KEYWORD_SMETHOD_RANDOM,

"check_contact(target_index)\nIf this process collided with another in the last cycle, returns 1 and saves the most recent colliding process to target_index (set target_index to -1 to discard the target). Returns 0 otherwise.", // KEYWORD_SMETHOD_CHECK_CONTACT,
"get_damage()\nReturns the amount of damage taken by the process during the last cycle. Includes damage to an interface.", // KEYWORD_SMETHOD_GET_DAMAGE,
"get_damage_source(target_index)\nIf this process took damage during the last cycle, returns 1 and saves the process that most recently caused the damage to target_index (set target_index to -1 to discard the target). Returns 0 otherwise.", // KEYWORD_SMETHOD_GET_DAMAGE_SOURCE,
"distance_from_xy(x_target, y_target)\nReturns the distance between the process' core and the absolute location x_target,y_target.", // KEYWORD_SMETHOD_DISTANCE_FROM_XY,
"distance_from_xy_less(x_target, y_target, distance)\nReturns 1 if the process' core is less than distance pixels from the absolute location x_target,y_target. Returns 0 otherwise.", // KEYWORD_SMETHOD_DISTANCE_LESS,
"distance_from_xy_more(x_target, y_target, distance)\nReturns 1 if the process' core is more than distance pixels from the absolute location x_target,y_target. Returns 0 otherwise.", // KEYWORD_SMETHOD_DISTANCE_MORE,
"distance_xy(x_component, y_component)\nReturns the distance between 0,0 and x_component,y_component.", // KEYWORD_SMETHOD_DISTANCE_XY,
"target_clear(target_index)\nClears targetting memory at target_index.", // KEYWORD_SMETHOD_TARGET_CLEAR,
"target_compare(target_index_1, target_index_2)\nReturns 1 if both targets are the same process, and 0 if they are not.", // KEYWORD_SMETHOD_TARGET_COMPARE,
"target_copy(target_index_dest, target_index_source)\nCopies an entry in targetting memory.", // KEYWORD_SMETHOD_TARGET_COPY,
"target_destroyed(target_index)\nReturns 1 if the target was destroyed within the last 200 ticks, as long as its last location is visible (to any friendly process). Returns 0 otherwise.\nThis is a standard method rather than a process method because process methods don't work on destroyed processes.", // KEYWORD_SMETHOD_TARGET_DESTROYED,
"attack_mode(setting)\nSets the attack mode for all attacking objects. The setting is retained until this method is called again.\n0=all objects fire together (if there is enough power), 1=one object fires per cycle, 2=two objects fire, 3=three objects fire.", // KEYWORD_SMETHOD_ATTACK_MODE,
"get_process_count()\nReturns the number of processes (cores) belonging to the same player as the calling process.", // KEYWORD_SMETHOD_GET_PROCESS_COUNT,
"get_processes_max()\nReturns the maximum number of processes (cores) each player can have.", // KEYWORD_SMETHOD_GET_PROCESSES_MAX,
"get_processes_unused()\nReturns the number of additional processes (cores) the player controlling the calling process can build.", // KEYWORD_SMETHOD_GET_PROCESSES_UNUSED,
"get_component_count()\nReturns the total number of components of all processes belonging to the same player as the calling process.", // KEYWORD_SMETHOD_GET_COMPONENT_COUNT,
"get_components_max()\nReturns the maximum number of components each player can have.", // KEYWORD_SMETHOD_GET_COMPONENTS_MAX,
"get_components_unused()\nReturns the number of additional components the player controlling the calling process can build.", // KEYWORD_SMETHOD_GET_COMPONENTS_UNUSED,
"special_AI(value1, value2)\nSpecial method that produces AI chatter bubbles for enemies in the single-player mode (these bubbles are generated by the game program itself). Does nothing if called by anyone else.", // KEYWORD_SMETHOD_SPECIAL_AI,

"sin(angle, multiplier)\nReturns the sine of angle (which should be in integer angle units, 0 to 8192) multiplied by the multiplier.", // KEYWORD_UMETHOD_SIN,
"cos(angle, multiplier)\nReturns the cosine of angle (which should be in integer angle units, 0 to 8192) multiplied by the multiplier.", // KEYWORD_UMETHOD_COS,
"atan2(y_component, x_component)\nReturns the angle of a line extending by x_component/y_component. This is an expensive operation.", // KEYWORD_UMETHOD_ATAN2,
"hypot(y_component, x_component)\nReturns an approximation of the hypotenuse of x_component/y_component. This is an expensive operation; it's usually best to use distance_xy() instead.", // KEYWORD_UMETHOD_HYPOT,
"world_x()\nReturns the x size of the map, in pixels.", // KEYWORD_UMETHOD_WORLD_X,
"world_y()\nReturns the x size of the map, in pixels.", // KEYWORD_UMETHOD_WORLD_Y,
"abs(number)\nReturns the absolute magnitude of number.", // KEYWORD_UMETHOD_ABS,
"angle_difference(angle_1, angle_2)\nReturns the shortest distance between two angles, taking wrapping into account. Returns a signed value; for a magnitude, use arc_length().", // KEYWORD_UMETHOD_ANGLE_DIFFERENCE,
"arc_length(angle_1, angle_2)\nReturns the shortest distance between two angles, taking wrapping into account. Returns a (positive) magnitude; for a signed distance, use angle_difference().", // KEYWORD_UMETHOD_ARC_LENGTH,


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

 print_help_string(keyword_help [keyword_index]);

}


void print_help_string(const char* help_str)
{

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

		 print_word [k] = help_str [i];
			if (help_str [i] == ' ')
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

			if (help_str [i] == '\n'
				|| help_str [i] == '\0')
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
			if (help_str [i] == '\0')
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
