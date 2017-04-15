#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>


#include <stdio.h>

#include "m_config.h"

#include "g_header.h"

#include "g_world.h"
#include "g_method_misc.h"
#include "g_method_std.h"
#include "g_shapes.h"
#include "g_motion.h"
#include "g_misc.h"
#include "g_proc_new.h"
#include "e_header.h"
#include "m_globvars.h"
#include "m_maths.h"
#include "m_input.h"
#include "i_console.h"
#include "i_background.h"
#include "i_header.h"

#include "x_sound.h"

#include "p_panels.h"

#include "g_command.h"




extern struct control_struct control;
extern struct view_struct view;
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct game_struct game;
extern struct notional_proc_struct notional_member [GROUP_MAX_MEMBERS]; // in g_proc_new.c
extern struct consolestruct console [CONSOLES];
extern struct fontstruct font [FONTS];

struct command_struct command;

static void select_a_core(int core_index);
static void add_core_to_selection(int core_index);
static void reset_select_mode(void);
static int check_clicked_on_data_well(al_fixed mouse_x_fixed, al_fixed mouse_y_fixed);
static void select_box(al_fixed xa, al_fixed ya, al_fixed xb, al_fixed yb);
static void select_by_template(int core_index);

static void issue_command_to_selected(int command_type, int command_x, int command_y, int core_index, int member_index, int queued, int control_pressed);
int check_block_visible_to_user(int block_x, int block_y);

static void open_build_buttons(int core_index);
void set_build_buttons_if_open(void);
void build_button_pressed(int template_index);
static void build_place_selected(void);
static void update_build_position(void);
static void update_build_angle(void);
static void give_command_after_build_angle_selected(int queued);
static void check_build_validity(int check_collisions);
static void build_queue_button_pressed(int button);
static void clear_power_record(void);
static int find_first_selected_core(void);
static void build_mobile_process_command(void);
static void set_control_group(int control_group_index, int exclusive);
static void add_to_control_group(int control_group_index);
static void select_control_group(int control_group_index);
static void remove_selection_from_control_group(void);
static int is_core_in_control_group(int core_index, int control_group_index);


int check_proc_visible_to_user(int proc_index);

void init_commands(void)
{
	command.select_mode = SELECT_MODE_NONE;
	command.selected_core [0] = SELECT_TERMINATE;
	command.selected_member = -1;
 command.select_box = 0;
 command.build_mode = BUILD_MODE_NONE;
 command.display_build_buttons = 0;
 clear_power_record();

 int i;

 for (i = 0; i < CONTROL_GROUPS; i ++)
	{
		command.control_group_core [i] [0] = SELECT_TERMINATE;
	}

}

/*

TO DO:

	- control groups on numbers
	 - number: select group
	 - number double-press: focus on group
	 - shift+number: add current selection to control group
	 - ctrl+number: set control group to current selection (may be empty)

	- single-click building for mobile processes? Maybe
	- new interface approach:
		 - remove interface objects (interface_depth becomes just interface)
		 - now all components without move objects generate interface automatically
		 - power cost based on proportion of max interface?
		   - actually should be 10 power (or whatever) for each fully charged interface object,
		     or a proportion of that for partly charged.

	- build commands should always queue.
	 - shift does repeat build
	 - control replaces all (for the current process) with the new one

	- clicking build queue button should select builder? maybe

	- selection: shift-box-select should work properly

*/


void run_commands(void)
{

 int proc_clicked;
 int core_selected;
 int i;

 al_fixed mouse_x_fixed = al_itofix(control.mouse_x_world_pixels);
 al_fixed mouse_y_fixed = al_itofix(control.mouse_y_world_pixels);

 int mouse_x_world = control.mouse_x_world_pixels;
 int mouse_y_world = control.mouse_y_world_pixels;
/*
 if (control.mbutton_press [0] == BUTTON_JUST_PRESSED)
	{
		fpr("\n %i,%i: %i", fixed_to_block(mouse_x_fixed), fixed_to_block(mouse_y_fixed), w.backblock[fixed_to_block(mouse_x_fixed)][fixed_to_block(mouse_y_fixed)].backblock_type);
	}*/

 int mouse_on_map = 0;
 int prevent_edge_scroll = 0;

 if (control.mouse_drag == MOUSE_DRAG_PANEL_RESIZE)
		prevent_edge_scroll = 1; // shouldn't scroll the map while resizing the panel

 if (control.mouse_x_screen_pixels >= view.map_x
		&& control.mouse_x_screen_pixels <= view.map_x + view.map_w
		&& control.mouse_y_screen_pixels >= view.map_y
		&& control.mouse_y_screen_pixels <= view.map_y + view.map_h)
	{
		mouse_on_map = 1;
		mouse_x_world = (control.mouse_x_screen_pixels - view.map_x) / al_fixtof(view.map_proportion_x);
		mouse_y_world = (control.mouse_y_screen_pixels - view.map_y) / al_fixtof(view.map_proportion_y);
		ex_control.mouse_cursor_type = MOUSE_CURSOR_MAP;
	}

// command.mouse_hold = 0;
// first check for new command input:

 int shift_pressed = 0;
 if (ex_control.special_key_press [SPECIAL_KEY_SHIFT] > 0)
			shift_pressed = 1;
	int control_pressed = 0;
	if (ex_control.special_key_press [SPECIAL_KEY_CTRL] > 0)
			control_pressed = 1;

	int proc_under_mouse = -1;

 if (control.mouse_panel == PANEL_MAIN)
	{
			block_cart mouse_cart;
			mouse_cart.x = fixed_to_block(mouse_x_fixed);
			mouse_cart.y = fixed_to_block(mouse_y_fixed);
		 if (verify_block_position(mouse_cart)
				&& check_block_visible_to_user(mouse_cart.x, mouse_cart.y))
			 proc_under_mouse = check_fuzzy_point_collision(mouse_x_fixed, mouse_y_fixed);

			sancheck(proc_under_mouse, -1, w.max_procs, "proc_under_mouse");

			if (proc_under_mouse != -1
			 && !mouse_on_map)
			{
				if (w.proc[proc_under_mouse].player_index == game.user_player_index)
					ex_control.mouse_cursor_type = MOUSE_CURSOR_PROCESS_FRIENDLY;
				  else
					  ex_control.mouse_cursor_type = MOUSE_CURSOR_PROCESS_ENEMY;
			}

//  if (ex_control.mouse_dragging_panel == PANEL_MAIN
	  	if (control.mbutton_press [0] > 0
					&& command.select_box == 0
					&& mouse_on_map)
//					&&	control.mouse_x_screen_pixels >= view.map_x
//					&& control.mouse_x_screen_pixels <= view.map_x + view.map_w
//					&& control.mouse_y_screen_pixels >= view.map_y
//					&& control.mouse_y_screen_pixels <= view.map_y + view.map_h)
				{
//			  command.select_box = 0;

     view.camera_x = al_itofix(mouse_x_world); //(control.mouse_x_screen_pixels - view.map_x) / al_fixtof(view.map_proportion_x));
     view.camera_y = al_itofix(mouse_y_world); //(control.mouse_y_screen_pixels - view.map_y) / al_fixtof(view.map_proportion_y));
     goto skip_main_mouse_input;
				}

		for (i = 0; i < CONTROL_GROUPS; i ++)
		{
// note that there is currently no control group for the 0 key
			if (ex_control.special_key_press [SPECIAL_KEY_CONTROL_GROUP_0 + i] == BUTTON_JUST_PRESSED)
			{
				if (ex_control.special_key_press [SPECIAL_KEY_CTRL])
				{
					if (ex_control.special_key_press [SPECIAL_KEY_SHIFT])
					 set_control_group(i, 1); // adding shift makes it an exclusive set
					  else
  					 set_control_group(i, 0);
				}
				  else
						{
							if (ex_control.special_key_press [SPECIAL_KEY_SHIFT])
   					add_to_control_group(i);
   					 else
									{
							   select_control_group(i);
//							   view.following = 0;
									}
						}
				break;
			}

		}


  if (command.display_build_buttons
   && w.command_mode == COMMAND_MODE_COMMAND)
		{
			for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
			{
			 if (ex_control.special_key_press [SPECIAL_KEY_0 + i] == BUTTON_JUST_PRESSED)
			 {
				 build_button_pressed(i);
				 break;
		 	}
			}
		}

		if (control.mouse_x_screen_pixels >= console[CONSOLE_GENERAL].x
			&& control.mouse_x_screen_pixels <= console[CONSOLE_GENERAL].x + console[CONSOLE_GENERAL].w_pixels
			&& control.mouse_y_screen_pixels > console[CONSOLE_GENERAL].y
			&& control.mouse_y_screen_pixels < console[CONSOLE_GENERAL].y + console[CONSOLE_GENERAL].h_pixels)
		{
			  ex_control.mouse_cursor_type = MOUSE_CURSOR_BASIC;
			  console[CONSOLE_GENERAL].line_highlight = (control.mouse_y_screen_pixels - console[CONSOLE_GENERAL].y) / CONSOLE_LINE_HEIGHT;
			  console[CONSOLE_GENERAL].line_highlight_time = game.total_time;
			  if (control.mbutton_press [0] == BUTTON_JUST_PRESSED)
					{
						int cline_clicked = console[CONSOLE_GENERAL].cpos - console[CONSOLE_GENERAL].h_lines + console[CONSOLE_GENERAL].line_highlight + 1;
						if (cline_clicked >= CLINES)
							cline_clicked -= CLINES;
						if (cline_clicked < 0)
							cline_clicked += CLINES;
						sancheck(cline_clicked, 0, CLINES, "cline_clicked");
						if (console[CONSOLE_GENERAL].cline[cline_clicked].source_index != -1
						 && console[CONSOLE_GENERAL].cline[cline_clicked].source_core_created_timestamp == w.core[console[CONSOLE_GENERAL].cline[cline_clicked].source_index].created_timestamp)
						{
					  sancheck(console[CONSOLE_GENERAL].cline[cline_clicked].source_index, 0, w.max_cores, "core clicked");
					  w.proc[w.core[console[CONSOLE_GENERAL].cline[cline_clicked].source_index].process_index].select_time = game.total_time;
				   select_a_core(console[CONSOLE_GENERAL].cline[cline_clicked].source_index);
					  command.selected_member = 0;
							view.camera_x = w.core[console[CONSOLE_GENERAL].cline[cline_clicked].source_index].core_position.x;
							view.camera_y = w.core[console[CONSOLE_GENERAL].cline[cline_clicked].source_index].core_position.y;
						}
					}
					prevent_edge_scroll = 1;
//     goto skip_main_mouse_input;
		}

  if (command.select_mode == SELECT_MODE_SINGLE_CORE
					&& control.mbutton_press [0] == BUTTON_JUST_PRESSED
			  && control.mouse_x_screen_pixels >= view.data_box_close_button_x1
			  && control.mouse_x_screen_pixels <= view.data_box_close_button_x2
			  && control.mouse_y_screen_pixels >= view.data_box_close_button_y1
			  && control.mouse_y_screen_pixels <= view.data_box_close_button_y2)
		{
			view.data_box_open ^= 1;
   goto skip_main_mouse_input;
		}


//*** TO DO: make sure build queue is displayed for the user player even in non-command mode!

//					 else
					 {
					 	if (command.display_build_buttons
					   &&	control.mouse_x_screen_pixels > view.build_buttons_x1
					   && control.mouse_x_screen_pixels < view.build_buttons_x2
					   && control.mouse_y_screen_pixels > view.build_queue_buttons_y1 // this should have been kept updated so that it's the actual top of the build queue buttons area
				    && control.mouse_y_screen_pixels < view.build_buttons_y2
				    && w.command_mode == COMMAND_MODE_COMMAND)
							{

// need this because of terrible spaghetti code:
        if (control.mbutton_press [0] == BUTTON_NOT_PRESSED)
									command.select_box = 0;

					   if (control.mouse_y_screen_pixels > view.build_buttons_y1
					    && control.mouse_y_screen_pixels < view.build_buttons_y2)
							 {
    			  ex_control.mouse_cursor_type = MOUSE_CURSOR_BASIC;
 								int mouse_on_build_button = (view.build_buttons_y2 - control.mouse_y_screen_pixels) / BUILD_BUTTON_H;
							  mouse_on_build_button = TEMPLATES_PER_PLAYER - mouse_on_build_button - 1; // this should probably be bounds-checked before being used
// remember that template list starts at 1 and ends in 0
//							 mouse_on_build_button = (mouse_on_build_button + 1) % TEMPLATES_PER_PLAYER;
								 view.mouse_on_build_button = mouse_on_build_button;
								 view.mouse_on_build_button_timestamp = game.total_time;
// highlight here
         if (control.mbutton_press [0] == BUTTON_JUST_PRESSED)
 									build_button_pressed(mouse_on_build_button);
 									 else
											{
												if (control.mbutton_press [0] == BUTTON_JUST_RELEASED)
												{
// the following box selection code is duplicated a couple of times below
         				if (command.select_box == 1
         				&& (abs(al_fixtoi(command.mouse_drag_world_x) - control.mouse_x_world_pixels) + abs(al_fixtoi(command.mouse_drag_world_y) - control.mouse_y_world_pixels) > 20
             	 ||	control.mbutton_press_timestamp [0] + 32 < game.total_time))
			          {
			           select_box(command.mouse_drag_world_x, command.mouse_drag_world_y, mouse_x_fixed, mouse_y_fixed);
			          }
		           command.select_box = 0;
												}
											}
							 }
					   if (control.mouse_y_screen_pixels > view.build_queue_buttons_y1
					    && control.mouse_y_screen_pixels < view.build_queue_buttons_y2)
							 {
    			  ex_control.mouse_cursor_type = MOUSE_CURSOR_BASIC;
 								int mouse_on_build_queue_button = (view.build_queue_buttons_y2 - control.mouse_y_screen_pixels) / BUILD_BUTTON_H;
								 view.mouse_on_build_queue_button = mouse_on_build_queue_button; // may not be valid!
								 view.mouse_on_build_queue_button_timestamp = game.total_time;
         if (control.mbutton_press [0] == BUTTON_JUST_PRESSED)
 									build_queue_button_pressed(mouse_on_build_queue_button); // cancels
 									 else
											{
												if (control.mbutton_press [0] == BUTTON_JUST_RELEASED)
												{
// the following box selection code is duplicated a couple of times below
         				if (command.select_box == 1
         				&& (abs(al_fixtoi(command.mouse_drag_world_x) - control.mouse_x_world_pixels) + abs(al_fixtoi(command.mouse_drag_world_y) - control.mouse_y_world_pixels) > 20
             	 ||	control.mbutton_press_timestamp [0] + 32 < game.total_time))
			          {
			           select_box(command.mouse_drag_world_x, command.mouse_drag_world_y, mouse_x_fixed, mouse_y_fixed);
			          }
		           command.select_box = 0;
												}
											}

// highlight here
//         if (control.mbutton_press [0] == BUTTON_JUST_PRESSED)
// 									build_button_pressed(mouse_on_build_button); // remember that mouse_on_build_queue_button may not be valid!
							 }
//   goto skip_main_mouse_input;
							}
							 else
								{

//									if (command.build_mode == BUILD_MODE_PLACE)
//										update_build_position();

		switch(control.mbutton_press [0])
  {
		 case BUTTON_JUST_PRESSED:
		 {
/*		 	if (command.build_mode != BUILD_MODE_NONE)
				{
					if (command.build_mode == BUILD_MODE_PLACE)
						build_place_selected();
					break;
				}*/
				if (command.build_mode != BUILD_MODE_NONE)
				{
					if (command.build_mode == BUILD_MODE_PLACE)
						build_place_selected();
//  	  command.build_mode = BUILD_MODE_NONE; // left-clicking cancels build
//     write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LGREY, -1, 0, "\nBuild command cancelled.");
//     play_interface_sound(SAMPLE_BLIP2, TONE_2F);
  	  break;
				}
// check for shift
    proc_clicked = proc_under_mouse;
			 if (proc_clicked != -1
					&& check_proc_visible_to_user(proc_clicked))
			 {
// user clicked on process.
// check for double click on same process:
      if (command.select_mode == SELECT_MODE_SINGLE_CORE
							&& command.selected_core [0] == w.proc[proc_clicked].core_index
							&& w.proc[proc_clicked].select_time >= game.total_time - 30)
						{
 						sancheck(proc_clicked, 0, w.max_procs, "proc_clicked");
							select_by_template(w.proc[proc_clicked].core_index);
							break;
						}
// unless the user is pressing shift with one or more cores already selected, select the process:
						if (!shift_pressed
						|| (command.select_mode != SELECT_MODE_SINGLE_CORE
							&& command.select_mode != SELECT_MODE_MULTI_CORE))
//						|| command.selected_core [0] == SELECT_TERMINATE) // shift-click same as normal click if nothing selected
					{
						sancheck(proc_clicked, 0, w.max_procs, "proc_clicked (A)");
 				 core_selected = w.proc[proc_clicked].core_index;
						sancheck(core_selected, 0, w.max_cores, "core_selected");
					 w.proc[proc_clicked].select_time = game.total_time;
				  select_a_core(core_selected);
					 command.selected_member = w.proc[proc_clicked].group_member_index;
      play_interface_sound(SAMPLE_BLIP1, TONE_2G);
					}
					 else
						{
// shift-click adds or removes proc
  				 core_selected = w.proc[proc_clicked].core_index;
							if (w.core[core_selected].selected == -1)
							{
								add_core_to_selection(core_selected);
        play_interface_sound(SAMPLE_BLIP1, TONE_2G);
							}
							  else
									{
										remove_core_from_selection(core_selected);
          play_interface_sound(SAMPLE_BLIP1, TONE_2D);
									}
						}
			 }
 			 else
				 {


				 	if (!shift_pressed
							&& !check_clicked_on_data_well(mouse_x_fixed, mouse_y_fixed) // won't treat click as a data well click if pressing shift
							&& inter.mode_button_highlight_time < inter.running_time) // don't deselect if user clicked a mode button
//							&& (control.mouse_x_screen_pixels < inter.mode_buttons_x1 - (MODE_BUTTON_SIZE + MODE_BUTTON_SPACING) * MODE_BUTTONS
//								|| control.mouse_y_screen_pixels > inter.mode_buttons_y1 + (MODE_BUTTON_SIZE + 3)
//								|| control.mouse_y_screen_pixels < inter.mode_buttons_y1 - 5
//								|| control.mouse_x_screen_pixels > inter.mode_buttons_x1))
 					 clear_selection();



 //if (control.mouse_y_screen_pixels < inter.mode_buttons_y1 + (MODE_BUTTON_SIZE + 3)
		//&&	control.mouse_y_screen_pixels > inter.mode_buttons_y1 - 5
		//&& control.mouse_x_screen_pixels >= inter.mode_buttons_x1 - (MODE_BUTTON_SIZE + MODE_BUTTON_SPACING) * MODE_BUTTONS) //inter.display_w - ((MODE_BUTTON_SIZE + MODE_BUTTON_SPACING) * MODE_BUTTONS))
//	{
//		mode_button_input();
//	}


				 }
			 command.select_box = 1;
			 command.mouse_drag_world_x = mouse_x_fixed;
			 command.mouse_drag_world_y = mouse_y_fixed;
		 }
		 break; // end case BUTTON_JUST_PRESSED
		case BUTTON_HELD:
			if (command.build_mode == BUILD_MODE_ANGLE)
				update_build_angle();
//			select_box(command.mouse_drag_world_x, command.mouse_drag_world_y, al_itofix(control.mouse_x_world_pixels), al_itofix(control.mouse_y_world_pixels));
			break;
/*
				break;
		 case BUTTON_JUST_RELEASED:
				if (command.build_mode == BUILD_MODE_ANGLE)
				{
					give_command_after_build_angle_selected(shift_pressed);
     play_interface_sound(SAMPLE_BLIP4, TONE_3C);
					if (shift_pressed)
						command.build_mode = BUILD_MODE_PLACE;
					  else
   					command.build_mode = BUILD_MODE_NONE;
					break;
				}
				break;*/

		case BUTTON_JUST_RELEASED:

				if (command.build_mode == BUILD_MODE_ANGLE)
				{
					give_command_after_build_angle_selected(shift_pressed);
     play_interface_sound(SAMPLE_BLIP4, TONE_3C);
					if (shift_pressed)
						command.build_mode = BUILD_MODE_PLACE;
					  else
   					command.build_mode = BUILD_MODE_NONE;
					break;
				}


	// do a box selection if either the mouse has moved more than 20 pixels, or the button has been held for more than 32 frames
// the following box selection code is duplicated in the special code for build buttons above (to avoid a problem with clicking on a builder proc that is in the build button part of the screen)
				if (command.select_box == 1
				&& (abs(al_fixtoi(command.mouse_drag_world_x) - control.mouse_x_world_pixels) + abs(al_fixtoi(command.mouse_drag_world_y) - control.mouse_y_world_pixels) > 20
   	 ||	control.mbutton_press_timestamp [0] + 32 < game.total_time))

			{
			 select_box(command.mouse_drag_world_x, command.mouse_drag_world_y, mouse_x_fixed, mouse_y_fixed);
			}
		 command.select_box = 0;
			break;
  } // end switch control.mbutton_press [0]
					 } // end else for mouse on map and mouse on build buttons

// Now check right mouse button, but only if 1+ cores selected
 if (w.command_mode == COMMAND_MODE_COMMAND
		&& (command.select_mode == SELECT_MODE_SINGLE_CORE
	  || command.select_mode == SELECT_MODE_MULTI_CORE))
	{
  switch(control.mbutton_press [1])
  {
	  case BUTTON_JUST_PRESSED:
		 	if (command.build_mode != BUILD_MODE_NONE)
				{
  	  command.build_mode = BUILD_MODE_NONE; // left-clicking cancels build
     write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LGREY, -1, 0, "\nBuild command cancelled.");
     play_interface_sound(SAMPLE_BLIP2, TONE_2F);
					break;
				}
/*
				if (command.build_mode != BUILD_MODE_NONE)
				{
  	  command.build_mode = BUILD_MODE_NONE; // right-clicking cancels build
     play_interface_sound(SAMPLE_BLIP2, TONE_2F);
  	  break;
				}*/
	 	 proc_clicked = proc_under_mouse;//check_fuzzy_point_collision(mouse_x_fixed, mouse_y_fixed); // should this be fuzzy?
	 	 if (proc_clicked != -1
					&& check_proc_visible_to_user(proc_clicked))
	 		{
	 			core_selected = w.proc[proc_clicked].core_index;
					sancheck(core_selected, 0, w.max_cores, "core_selected B");
					sancheck(proc_clicked, 0, w.max_procs, "proc_clicked C");
	 			if (w.proc[proc_clicked].player_index != game.user_player_index)
 				 issue_command_to_selected(COM_TARGET, al_fixtoi(w.core[core_selected].core_position.x), al_fixtoi(w.core[core_selected].core_position.y), core_selected, w.proc[proc_clicked].group_member_index, shift_pressed, control_pressed);
 				  else
 				   issue_command_to_selected(COM_FRIEND, al_fixtoi(w.core[core_selected].core_position.x), al_fixtoi(w.core[core_selected].core_position.y), core_selected, w.proc[proc_clicked].group_member_index, shift_pressed, control_pressed);
     play_interface_sound(SAMPLE_BLIP4, TONE_3AS);
			 }
			  else
					{
						for (i = 0; i < w.data_wells; i ++)
						{
							if (w.data_well[i].active
								&& w.data_well[i].position.x > al_itofix(mouse_x_world - 100)
								&& w.data_well[i].position.y > al_itofix(mouse_y_world - 100)
								&& w.data_well[i].position.x < al_itofix(mouse_x_world + 100)
								&& w.data_well[i].position.y < al_itofix(mouse_y_world + 100))
//								&& w.vision_area[game.user_player_index][w.data_well[i].block_position.x][w.data_well[i].block_position.y].vision_time >= w.world_time - VISION_AREA_VISIBLE_TIME)
							{
//						   issue_command_to_selected(COM_DATA_WELL, mouse_x_world, mouse_y_world, -1, -1, shift_pressed, control_pressed);
						   issue_command_to_selected(COM_DATA_WELL, al_fixtoi(w.data_well[i].position.x), al_fixtoi(w.data_well[i].position.y), -1, -1, shift_pressed, control_pressed);
         play_interface_sound(SAMPLE_BLIP4, TONE_2A);
						   break;
							}
						}
						if (i == w.data_wells) // loop must have failed to find anything
						{
						 issue_command_to_selected(COM_LOCATION, mouse_x_world, mouse_y_world, -1, -1, shift_pressed, control_pressed);
       play_interface_sound(SAMPLE_BLIP4, TONE_3G);
						}
					}
		  break; // end BUTTON_JUST_PRESSED
/*
		 case BUTTON_HELD:
			 if (command.build_mode == BUILD_MODE_ANGLE)
			 	update_build_angle();
				break;
		 case BUTTON_JUST_RELEASED:
				if (command.build_mode == BUILD_MODE_ANGLE)
				{
					give_command_after_build_angle_selected(shift_pressed);
     play_interface_sound(SAMPLE_BLIP4, TONE_3C);
					if (shift_pressed)
						command.build_mode = BUILD_MODE_PLACE;
					  else
   					command.build_mode = BUILD_MODE_NONE;
					break;
				}
				break;
*/

  } // end switch control.mbutton_press [1]

		if (command.build_mode == BUILD_MODE_PLACE)
			update_build_position();



//  if (ex_control.unichar_input >= 48
//			&& ex_control.unichar_input <= 57)
		//{
//					issue_command_to_selected(COM_NUMBER, ex_control.unichar_input - 48, 0, -1, -1, shift_pressed, control_pressed);
    // play_interface_sound(SAMPLE_BLIP1, TONE_2C);
//		}
/*			for (i = KEY_0; i < KEY_9; i ++)
			{
				if (control.key_press [i] == BUTTON_JUST_PRESSED)
				{
					issue_command_to_selected(COM_NUMBER, i - KEY_0, 0, -1, -1, shift_pressed, control_pressed);
     play_interface_sound(SAMPLE_BLIP1, TONE_2C);
					break;
				}
			}
		}*/

	}


#define MOUSE_SCROLL_BORDER 70
//#define MOUSE_SCROLL_BORDER_CORNER 80
#define MOUSE_SCROLL_SPEED_DIVISOR 2
#define KEYBOARD_SCROLL_SPEED 20

 int mouse_scroll_x = 0;
 int mouse_scroll_y = 0;

 if (control.mouse_panel == PANEL_MAIN
		&& control.mouse_status != MOUSE_STATUS_OUTSIDE
		&& !prevent_edge_scroll)
	{

// keyboard scroll (just use mouse_scroll values because I'm lazy)
		if (ex_control.special_key_press [SPECIAL_KEY_UP])
			mouse_scroll_y = -KEYBOARD_SCROLL_SPEED;
		  else
				{
		   if (ex_control.special_key_press [SPECIAL_KEY_DOWN])
			   mouse_scroll_y = KEYBOARD_SCROLL_SPEED;
				}
		if (ex_control.special_key_press [SPECIAL_KEY_LEFT])
			mouse_scroll_x = -KEYBOARD_SCROLL_SPEED;
		  else
				{
		   if (ex_control.special_key_press [SPECIAL_KEY_RIGHT])
			   mouse_scroll_x = KEYBOARD_SCROLL_SPEED;
				}

// mode_button exclusion zone:
//		if ((control.mouse_y_screen_pixels > 30
//			|| control.mouse_x_screen_pixels < settings.option [OPTION_WINDOW_W] - 140)
//		&& mouse_on_map == 0)

 if (mouse_on_map == 0)
	{
// don't scroll if the mouse is only in the corner because the player just closed all panels:
  if (inter.block_mode_button_area_scrolling)
		{
//			fpr("\nblocked");
			if (control.mouse_x_screen_pixels < inter.display_w - MODE_BUTTON_SCROLL_BLOCK_W
			 || control.mouse_y_screen_pixels > MODE_BUTTON_SCROLL_BLOCK_H)
					inter.block_mode_button_area_scrolling = 0;

		}
		 else
			{

  	if (control.mbutton_press [2] > 0)
	  {

		  int old_mouse_x_world_pixels = (ex_control.old_mouse_x_pixels / view.zoom + al_fixtoi(view.camera_x - view.centre_x_zoomed)) ;
    int old_mouse_y_world_pixels = (ex_control.old_mouse_y_pixels / view.zoom + al_fixtoi(view.camera_y - view.centre_y_zoomed));

    view.camera_x += al_itofix(old_mouse_x_world_pixels - control.mouse_x_world_pixels) / view.zoom;
    view.camera_y += al_itofix(old_mouse_y_world_pixels - control.mouse_y_world_pixels) / view.zoom;
//fpr("\n cmwp %i,%i old %i,%i ex_old %i,%i", control.mouse_x_world_pixels, control.mouse_y_world_pixels, old_mouse_x_world_pixels, old_mouse_y_world_pixels, ex_control.old_mouse_x_pixels, ex_control.old_mouse_y_pixels);
	  }
	  else
			{



#ifndef RECORDING_VIDEO_2
  if (control.mouse_x_screen_pixels < MOUSE_SCROLL_BORDER)
 		mouse_scroll_x = 0 - ((MOUSE_SCROLL_BORDER - control.mouse_x_screen_pixels) / MOUSE_SCROLL_SPEED_DIVISOR);
 		 else
				{
     if (control.mouse_x_screen_pixels > view.window_x_unzoomed - MOUSE_SCROLL_BORDER)
      mouse_scroll_x = (MOUSE_SCROLL_BORDER - (view.window_x_unzoomed - control.mouse_x_screen_pixels)) / MOUSE_SCROLL_SPEED_DIVISOR;
				}

  if (control.mouse_y_screen_pixels < MOUSE_SCROLL_BORDER)
   mouse_scroll_y = 0 - ((MOUSE_SCROLL_BORDER - control.mouse_y_screen_pixels) / MOUSE_SCROLL_SPEED_DIVISOR);
    else
				{
     if (control.mouse_y_screen_pixels > view.window_y_unzoomed - MOUSE_SCROLL_BORDER)
      mouse_scroll_y = ((MOUSE_SCROLL_BORDER - (view.window_y_unzoomed - control.mouse_y_screen_pixels)) / MOUSE_SCROLL_SPEED_DIVISOR);
				}
#endif

	 if (mouse_scroll_x != 0)
		{
			if (game.fast_forward)
			{
				int fps_adjust = view.fps;
				if (fps_adjust < 60)
					fps_adjust = 60;
			 if (fps_adjust > 500)
					fps_adjust = 500;
 			if (game.fast_forward_type == FAST_FORWARD_TYPE_SMOOTH)
 				mouse_scroll_x *= 60;
 				 else
   				mouse_scroll_x *= 15;
				mouse_scroll_x /= fps_adjust;

			 view.camera_x += al_itofix(mouse_scroll_x);

/*				if (game.fast_forward_type == FAST_FORWARD_TYPE_SMOOTH)
				 view.camera_x += al_itofix(mouse_scroll_x) / 4;
				  else
				   view.camera_x += al_itofix(mouse_scroll_x) / 12;*/
			}
			  else
				  view.camera_x += al_itofix(mouse_scroll_x);
		}
	 if (mouse_scroll_y != 0)
		{
			if (game.fast_forward)
			{
				int fps_adjust = view.fps;
				if (fps_adjust < 60)
					fps_adjust = 60;
			 if (fps_adjust > 500)
					fps_adjust = 500;
 			if (game.fast_forward_type == FAST_FORWARD_TYPE_SMOOTH)
 				mouse_scroll_y *= 60;
 				 else
   				mouse_scroll_y *= 15;
				mouse_scroll_y /= fps_adjust;

			 view.camera_y += al_itofix(mouse_scroll_y);
/*

 			if (game.fast_forward_type == FAST_FORWARD_TYPE_SMOOTH)
	 			view.camera_y += al_itofix(mouse_scroll_y) / 4;
				  else
				   view.camera_y += al_itofix(mouse_scroll_y) / 12;*/
			}
			  else
				  view.camera_y += al_itofix(mouse_scroll_y);
		}
			} // end else for if mouse_button [2] pressed
			} // end else for if inter.block_mode_button_area_scrolling
	} // end if mouse not on map


	} // end if mouse on main panel

 } // end else (mouse on map)
	} // end if (mouse on game display)
	 else
		{
			command.select_box = 0;
//			command.
// anything to cancel here if mouse not on display?
		}

 skip_main_mouse_input: // this is the target of some gotos above that skip most mouse input because the mouse is on the map or console etc

	if (view.following
		&& (command.select_mode == SELECT_MODE_SINGLE_CORE
   || command.select_mode == SELECT_MODE_MULTI_CORE))
	{
		int following_core_index = find_first_selected_core();
		if (following_core_index != -1)
		{
			view.camera_x = w.core[following_core_index].core_position.x;
			view.camera_y = w.core[following_core_index].core_position.y;
		}
	}

	if (view.screen_shake_time > w.world_time
		&& !game.pause_soft
		&& game.watching != WATCH_PAUSED_TO_WATCH)
	{
		int screen_shake_amount = view.screen_shake_time - w.world_time;
		view.camera_x += al_itofix(grand(screen_shake_amount) - grand(screen_shake_amount));
		view.camera_y += al_itofix(grand(screen_shake_amount) - grand(screen_shake_amount));
	}

// not sure the following bounds checks are needed but it can't hurt
 if (view.camera_x < view.camera_x_min)
  view.camera_x = view.camera_x_min;
 if (view.camera_y < view.camera_y_min)
  view.camera_y = view.camera_y_min;
 if (view.camera_x > view.camera_x_max)
  view.camera_x = view.camera_x_max;
 if (view.camera_y > view.camera_y_max)
  view.camera_y = view.camera_y_max;



// make sure anything selected is still visible:
 if (command.select_mode != SELECT_MODE_SINGLE_CORE
		&& command.select_mode != SELECT_MODE_MULTI_CORE)
	{
	int selection_changed = 0;
 for (i = 0; i < SELECT_MAX; i ++)
	{
		if (command.selected_core [i] == SELECT_TERMINATE)
			break;
		if (command.selected_core [i] == SELECT_EMPTY)
			continue;
  sancheck(command.selected_core [i], 0, w.max_cores, "command.selected_core [i]");
		if (!check_proc_visible_to_user(w.core[command.selected_core [i]].process_index))
		{
		 selection_changed = 1;
		 w.core[command.selected_core [i]].selected = -1;
		 w.core[command.selected_core [i]].deselect_time = game.total_time;
// the [i+1] index here assumes that the selected_core list is always terminated by SELECT_TERMINATE
		 if (command.selected_core [i+1] == SELECT_TERMINATE)
		  command.selected_core [i] = SELECT_TERMINATE;
		   else
		    command.selected_core [i] = SELECT_EMPTY;
		}

	}
	if (selection_changed)
  reset_select_mode(); // maybe we should change selection modes now
	} // end check for visible

// if a single core is selected, maintain the stress/power record:
 if (command.select_mode == SELECT_MODE_SINGLE_CORE
		&& !game.pause_soft
		&& game.watching != WATCH_PAUSED_TO_WATCH
	 && w.core[command.selected_core [0]].last_execution_timestamp == w.world_time - 15)
	{
  sancheck(command.selected_core [0], 0, w.max_cores, "command.selected_core [0]");
//		command.stress_record [command.power_use_pos] = w.core[command.selected_core [0]].stress;
//		command.stress_level_record [command.power_use_pos] = w.core[command.selected_core [0]].stress_level;
		command.power_use_record [command.power_use_pos] = w.core[command.selected_core [0]].power_capacity - w.core[command.selected_core [0]].power_left;
		command.power_fail_record [command.power_use_pos] = w.core[command.selected_core [0]].power_use_excess;
		command.power_use_pos ++;
	 if (command.power_use_pos >= POWER_DATA_RECORDS)
			command.power_use_pos = 0;
	}

	if (command.select_mode == SELECT_MODE_DATA_WELL
		&& !check_block_visible_to_user(w.data_well[command.selected_data_well].block_position.x, w.data_well[command.selected_data_well].block_position.y))
	{
		command.select_mode = SELECT_MODE_NONE;
	}


}


// does not assume that xa < xb or ya < yb
static void select_box(al_fixed xa, al_fixed ya, al_fixed xb, al_fixed yb)
{

 int shift_pressed;

 if (ex_control.special_key_press [SPECIAL_KEY_SHIFT] > 0)
	{
		shift_pressed = 1;
	}
		 else
			{
 			shift_pressed = 0;
    clear_selection();
			}

// need to make sure that only the user's cores are selected

	al_fixed x1, y1, x2, y2;

	if (xa < xb)
	{
		x1 = xa;
		x2 = xb;
	}
	 else
		{
			x1 = xb;
			x2 = xa;
		}

	if (ya < yb)
	{
		y1 = ya;
		y2 = yb;
	}
	 else
		{
			y1 = yb;
			y2 = ya;
		}

	int i;
	int select_counter = 0;
	int found_terminate = 0; // if shift_pressed, we may need to add a new terminator to the end (if the old terminator has been overwritten)

	for (i = 0; i < w.max_cores; i ++)
	{
		if (w.core[i].exists == 1
			&& w.core[i].player_index == game.user_player_index
			&& w.core[i].selected == -1
			&& w.core[i].core_position.x +	w.proc[w.core[i].process_index].nshape_ptr->max_length	> x1
			&& w.core[i].core_position.x - w.proc[w.core[i].process_index].nshape_ptr->max_length	< x2
			&& w.core[i].core_position.y + w.proc[w.core[i].process_index].nshape_ptr->max_length	> y1
			&& w.core[i].core_position.y - w.proc[w.core[i].process_index].nshape_ptr->max_length	< y2
			&& check_proc_visible_to_user(w.core[i].process_index))
		{

			if (shift_pressed
				&& !found_terminate) // if the terminate has been found, we can just overwrite instead of looking for empty spaces
			{
// should be safe to assume that the list is terminated:
  	 while(command.selected_core[select_counter] != SELECT_TERMINATE
			 		 && command.selected_core[select_counter] != SELECT_EMPTY)
	   {
		   select_counter++;
	   }
	   if (command.selected_core[select_counter] == SELECT_TERMINATE)
					found_terminate = 1;

	   if (select_counter >= SELECT_MAX - 1)
				 break;
			}

			command.selected_core [select_counter] = i;
			w.core[i].selected = select_counter;
			w.core[i].select_time = game.total_time;
			select_counter ++;

 		if (select_counter >= SELECT_MAX - 1)
	 		break;

		}
	}



 sancheck(select_counter, 0, SELECT_MAX, "select_counter");

// select_counter may still be in the middle of the list (if shift is pressed and the newly selected cores have filled in gaps).
// so move it to the end:
/*
	while(TRUE) // should be safe to assume that the list is terminated
 {
  if (command.selected_core[select_counter] == SELECT_TERMINATE)
	  break;
  select_counter++;
 }
*/

// if the list was cleared, or if the previous terminate was overwritten, need to add a terminate:
 if (!shift_pressed
		|| found_terminate)
	 command.selected_core [select_counter] = SELECT_TERMINATE;

// If only one core selected, go to single select mode (which can open build buttons etc)
//  - unless we're in shift_pressed mode AND there are more cores selected, in which case don't
//    (this can happen if the new core fills in a gap at selected_core [0])
	if (select_counter == 1
		&& (!shift_pressed
				||	command.selected_core [1] == SELECT_TERMINATE))
	{
		select_a_core(command.selected_core [0]);
		command.selected_member = 0;
	}
		else
		{
			if (select_counter > 1) // select_counter could still be 0
			{
				command.select_mode = SELECT_MODE_MULTI_CORE;
    play_interface_sound(SAMPLE_BLIP1, TONE_2G);
			}
		}

}







// core_index is the process that was double-clicked on
static void select_by_template(int core_index)
{

// clear_selection(); - shouldn't be needed as core[core_index] should be selected at 0 and no others selected.

	al_fixed x1 = view.camera_x - view.centre_x_zoomed;
	al_fixed y1 = view.camera_y - view.centre_y_zoomed;
	al_fixed x2 = view.camera_x + view.centre_x_zoomed;
	al_fixed y2 = view.camera_y + view.centre_y_zoomed;

	int i;
	int template_index = w.core[core_index].template_index;
	int select_counter = 1; // note this starts at 1 because 0 already selected

	for (i = 0; i < w.max_cores; i ++)
	{
		if (w.core[i].exists == 1
			&& w.core[i].core_position.x +	w.proc[w.core[i].process_index].nshape_ptr->max_length	> x1
			&& w.core[i].core_position.x - w.proc[w.core[i].process_index].nshape_ptr->max_length	< x2
			&& w.core[i].core_position.y + w.proc[w.core[i].process_index].nshape_ptr->max_length	> y1
			&& w.core[i].core_position.y - w.proc[w.core[i].process_index].nshape_ptr->max_length	< y2
			&& w.core[i].player_index == w.core[core_index].player_index
			&& w.core[i].template_index == template_index
			&& i != core_index
			&& check_proc_visible_to_user(w.core[i].process_index))
		{
   sancheck(select_counter, 0, SELECT_MAX, "select_counter B");
			command.selected_core [select_counter] = i;
			w.core[i].selected = select_counter;
			w.core[i].select_time = game.total_time;
			select_counter ++;
		}
		if (select_counter >= SELECT_MAX - 1)
			break;
	}

 sancheck(select_counter, 0, SELECT_MAX, "select_counter C");

	command.selected_core [select_counter] = SELECT_TERMINATE;
/*
	if (select_counter == 1)
	{
		select_a_core(command.selected_core [0]);
		command.selected_member = 0;
	}
		else*/
		{
			if (select_counter > 1)
			{
				command.select_mode = SELECT_MODE_MULTI_CORE;
    play_interface_sound(SAMPLE_BLIP1, TONE_2G);
			}
		}

}


// selects a single core, cancelling the rest of the selection array (if any)
static void select_a_core(int core_index)
{
	if (command.select_mode == SELECT_MODE_SINGLE_CORE
		&& command.selected_core [0] == core_index)
	{
		return;
	}

 play_interface_sound(SAMPLE_BLIP1, TONE_2G);
 clear_power_record();

// if no cores selected, just select the core:
	if (command.select_mode != SELECT_MODE_SINGLE_CORE
		&& command.select_mode != SELECT_MODE_MULTI_CORE)
	{

  sancheck(core_index, 0, w.max_cores, "select_a_core:core_index");
		command.selected_core [0] = core_index;
		w.core[core_index].selected = 0;
		w.core[core_index].select_time = game.total_time;
		command.selected_core [1] = SELECT_TERMINATE;
		command.select_mode = SELECT_MODE_SINGLE_CORE;
  if (w.core[core_index].number_of_build_objects
			&& w.core[core_index].player_index == game.user_player_index)
		{
	 	open_build_buttons(core_index);

//	 	command.select_box = 0;
		}
	 	 else
     command.display_build_buttons = 0;
		return;
	}

// other cores must be selected. So we need to clear them:

 clear_selection();

	command.select_mode = SELECT_MODE_SINGLE_CORE;
	command.selected_core [0] = core_index;
 sancheck(core_index, 0, w.max_cores, "select_a_core:core_index B");
	w.core[core_index].selected = 0;
	w.core[core_index].select_time = game.total_time;
	command.selected_core [1] = SELECT_TERMINATE;

 if (w.core[core_index].number_of_build_objects
		&& w.core[core_index].player_index == game.user_player_index)
	{
		open_build_buttons(core_index);
	}
		 else
    command.display_build_buttons = 0;

}

static void clear_power_record(void)
{

// clear power/stress record
 command.power_use_pos = 0;
 int i;
 for (i = 0; i < POWER_DATA_RECORDS; i ++)
	{
//		command.stress_record [i] = 0; // don't need to zero out stress_level_record as if stress_record == 0 it's not used
		command.power_use_record [i] = 0;
		command.power_fail_record [i] = 0;
	}

}

static void open_build_buttons(int core_index)
{

// if (w.command_mode != COMMAND_MODE_COMMAND)
//		return;

 command.builder_core_index = core_index;
 set_build_buttons_if_open();

}

// call this function just after opening build buttons by selecting builder proc
// can also call this function if e.g. a template is updated so that the build buttons should change.
void set_build_buttons_if_open(void)
{

	command.display_build_buttons = 1;
 place_build_buttons();

}


void clear_selection(void)
{

	int i;

	for (i = 0; i < SELECT_MAX; i ++)
	{

		if (command.selected_core [i] == SELECT_TERMINATE)
			break;
		if (command.selected_core [i] != SELECT_EMPTY)
		{
   sancheck(command.selected_core [i], 0, w.max_cores, "command.selected_core [i]");
			w.core[command.selected_core [i]].selected = -1;
			w.core[command.selected_core [i]].deselect_time = game.total_time;
		}
	}

	command.selected_core [0] = SELECT_TERMINATE;
	command.selected_member = -1;
	command.build_mode = BUILD_MODE_NONE;
	command.display_build_buttons = 0;
	command.select_mode = SELECT_MODE_NONE;

 view.following = 0;

}

static void add_core_to_selection(int core_index)
{

	if (command.select_mode != SELECT_MODE_SINGLE_CORE
		&& command.select_mode != SELECT_MODE_MULTI_CORE)
	{
		select_a_core(core_index);
		command.selected_member = 0;
		return;
	}

// The following code could skip some steps for SELECT_MODE_SINGLE_CORE, but it doesn't yet:
	int select_counter = 0;

	while(select_counter < SELECT_MAX - 1)
	{
		if (command.selected_core[select_counter] == SELECT_TERMINATE)
		{
			command.selected_core [select_counter + 1] = SELECT_TERMINATE; // add new terminator
			break;
		}
		if (command.selected_core[select_counter] == SELECT_EMPTY)
			break;
		select_counter++;
	}

	if (select_counter >= SELECT_MAX - 1)
		return;

	command.select_mode = SELECT_MODE_MULTI_CORE;
	command.selected_core [select_counter] = core_index;
	w.core[core_index].selected = select_counter;
	w.core[core_index].select_time = game.total_time;

/*
	if (select_counter == 1)
	{
		select_a_core(core_index);
		command.selected_member = 0;
	}
	 else*/

 command.display_build_buttons = 0; // if more than one core selected, don't show build buttons

}


// call this when a core is destroyed or deselected.
// it will make sure the core is no longer selected.
// okay to call whether or not the core is actually selected.
void remove_core_from_selection(int core_index)
{

 sancheck(core_index, 0, w.max_cores, "remove_core_from_selection:core_index");

// first check for destroyed core being only selected core:
	if (w.core[core_index].selected == 0 // means it's at 0 position in command.selected_core array
		&& command.select_mode == SELECT_MODE_SINGLE_CORE)
	{
		command.select_mode = SELECT_MODE_NONE;
		command.selected_core [0] = SELECT_TERMINATE;
		w.core[core_index].selected = -1;
		w.core[core_index].deselect_time = game.total_time;
		command.display_build_buttons = 0;
		return;
	}

	if (w.core[core_index].selected != -1)
	{
		command.selected_core [w.core[core_index].selected] = SELECT_EMPTY;
		w.core[core_index].selected = -1;
		w.core[core_index].deselect_time = game.total_time;
		reset_select_mode();
	}

// must not be selected...

}

// Sets select mode based on command.selected_core array
// Currently doesn't deal with non-proc selection (e.g. data well)
static void reset_select_mode(void)
{

	int i;
	int select_count = 0;
	int last_selected_found = 0;

	for (i = 0; i < SELECT_MAX; i ++)
	{
		if (command.selected_core [i] == SELECT_TERMINATE)
			break;
		if (command.selected_core [i] != SELECT_EMPTY)
		{
			select_count ++;
			last_selected_found = i;
		}
	}

	if (select_count == 0)
	{
		command.select_mode = SELECT_MODE_NONE;
		command.selected_core [0] = SELECT_TERMINATE;
	}
		 else
			{
	   if (select_count > 1)
		   command.select_mode = SELECT_MODE_MULTI_CORE; // should be able to assume that selected_core array is in proper shape
		    else
						{
  		   command.select_mode = SELECT_MODE_SINGLE_CORE;
       clear_power_record();
// we also need to make sure that the single selected core is selected_core [0]
       if (last_selected_found != 0)
							{
								command.selected_core [0] = command.selected_core [last_selected_found];
								command.selected_core [1] = SELECT_TERMINATE;
							 sancheck(command.selected_core [0], 0, w.max_cores, "reset_select_mode:command.selected_core [0]");
								w.core[command.selected_core [0]].selected = 0;
							}
						}
			}



}


static int check_clicked_on_data_well(al_fixed mouse_x_fixed, al_fixed mouse_y_fixed)
{

	int block_x, block_y;

	block_x = fixed_to_block(mouse_x_fixed);
	block_y = fixed_to_block(mouse_y_fixed);

	if (block_x <= 1
		|| block_x >= w.blocks.x - 2
		|| block_y <= 1
		|| block_y >= w.blocks.y - 2)
			return 0;

 if (w.backblock[block_x][block_y].backblock_type == BACKBLOCK_DATA_WELL
		&& check_block_visible_to_user(block_x, block_y))
	{
		command.select_mode = SELECT_MODE_DATA_WELL;
		command.selected_data_well = w.backblock[block_x][block_y].backblock_value;
	 sancheck(command.selected_data_well, 0, DATA_WELLS, "check_clicked_on_data_well:command.selected_data_well");
  play_interface_sound(SAMPLE_BLIP1, TONE_2E);
		return 1;
	}

	return 0;

}


// for number commands command_x is used as the value
static void issue_command_to_selected(int command_type, int command_x, int command_y, int core_index, int member_index, int queued, int control_pressed)
{
	int i;

	for (i = 0; i < SELECT_MAX; i ++)
	{
		if (command.selected_core [i] == SELECT_TERMINATE)
			break;
		if (command.selected_core [i] == SELECT_EMPTY)
			continue;
		sancheck(command.selected_core[i], 0, w.max_cores, "issue_command_to_selected: command.selected_core[i]");
		if (w.core[command.selected_core[i]].player_index == game.user_player_index)
		 add_command(&w.core[command.selected_core[i]], command_type, command_x, command_y, core_index, member_index, queued, control_pressed);
	}

}

// for number commands x is used as the value
int add_command(struct core_struct* core, int command_type, int x, int y, int core_index, int member_index, int queued, int control_pressed)
{
	int new_command_index = 0;



	if (queued)
	{
		while(TRUE)
		{
			if (new_command_index >= COMMAND_QUEUE)
			 return 0; // can't add to queue - command queue full
			if (core->command_queue [new_command_index].type == COM_NONE)
				break;
			new_command_index ++;
		};
	}

		sancheck(new_command_index, 0, COMMAND_QUEUE, "add_command: command_queue length");


	core->command_queue [new_command_index].type = command_type;
	core->command_queue [new_command_index].x = x;
	core->command_queue [new_command_index].y = y;
	core->command_queue [new_command_index].target_core = core_index;
	if (core_index != -1)
	 core->command_queue [new_command_index].target_core_created = w.core[core_index].created_timestamp;
	core->command_queue [new_command_index].target_member = member_index;
	core->command_queue [new_command_index].control_pressed = control_pressed;
	core->command_queue [new_command_index].command_time = game.total_time;

 if (new_command_index < COMMAND_QUEUE - 1)
		core->command_queue [new_command_index+1].type = COM_NONE; // probably not needed if commands are queued, but can't hurt.

	core->new_command = 1;

	return 1;

}

// can be called because of mouse pressing build button on screen, and also player pressing number on keyboard
void build_button_pressed(int template_index)
{
 if (w.command_mode != COMMAND_MODE_COMMAND)
		return;

	if (template_index < 0)
		template_index = 0;
	if (template_index >= TEMPLATES_PER_PLAYER)
		template_index = TEMPLATES_PER_PLAYER - 1;
	if (!templ[game.user_player_index][template_index].active)
		return;

	if (templ[game.user_player_index][template_index].member[0].shape >= FIRST_MOBILE_NSHAPE)
	{
 	command.build_template_index = template_index;
  play_interface_sound(SAMPLE_BLIP2, TONE_2G);
  build_mobile_process_command();
  return;
	}

	command.build_mode = BUILD_MODE_PLACE;
	command.build_template_index = template_index;
	command.build_angle = 0;
	command.default_build_angle = 0;
//	command.display_build_buttons = 0; - no real reason to do this
 play_interface_sound(SAMPLE_BLIP1, TONE_2G);

 write_text_to_console(CONSOLE_GENERAL, PRINT_COL_WHITE, -1, 0, "\nBuilding static process - select build location.");

}

// builds a process without the user having to specify a position.
// assumes command.build_template_index has been set
static void build_mobile_process_command(void)
{

 sancheck(command.selected_core[0], 0, w.max_cores, "build_mobile_process_command: check");

// first verify selected[0]
 if (command.selected_core[0] < 0 // could be SELECT_TERMINATE or SELECT_EMPTY
	 || w.core	[command.selected_core[0]].number_of_build_objects == 0) // could happen if component with build object destroyed since selection.
		return; // maybe print a message to console?

 command.build_mode = BUILD_MODE_NONE; // user may be placing static process and then click a build button

	command.build_position.x = w.core[command.selected_core [0]].core_position.x + fixed_xpart(w.core[command.selected_core [0]].group_angle, al_itofix(400));
	command.build_position.y = w.core[command.selected_core [0]].core_position.y + fixed_ypart(w.core[command.selected_core [0]].group_angle, al_itofix(400));
	command.default_build_angle = 0;
	command.build_angle = w.core[command.selected_core [0]].group_angle; // should probably change this to angle from builder proc

// command.build_template_index should have been set already

	check_build_validity(0); // 0 means don't check for collisions

 if (command.build_fail_range) // build_fail_range should only be 1 if the builder is static. out of range build commands are okay for mobile builders
	{
// this should never happen but check it just to make sure
 	 write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LRED, -1, 0, "\nFailed - out of range.");
 	 return;
	}

 if (command.build_fail_edge)
	{
 	 write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LRED, -1, 0, "\nFailed - location outside map.");
 	 return;
	}

 if (command.build_fail_static)
	{
 	 write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LRED, -1, 0, "\nFailed - static process too close to data well.");
 	 return;
	}


 if (!add_to_build_queue(game.user_player_index,
																				     command.selected_core[0],
																				      command.build_template_index,
																				     al_fixtoi(command.build_position.x),
																				     al_fixtoi(command.build_position.y),
																				     fixed_angle_to_int(command.build_angle),
																				     (ex_control.special_key_press [SPECIAL_KEY_CTRL] > 0), // back or front (ctrl sets it to front)
																				     (ex_control.special_key_press [SPECIAL_KEY_SHIFT] > 0), // repeat
																				     1, // queued (if 0, replaces other build commands for this process)
																				     1))
	{
// add_to_build_queue returns 0 if build queue full (1 on success, -1 on error)
 	 write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LRED, -1, 0, "\nBuild queue full.");
	}

}

// can be called because of mouse pressing build button on screen, and also player pressing number on keyboard
static void build_queue_button_pressed(int button)
{
 if (w.command_mode != COMMAND_MODE_COMMAND)
		return;

// if (command.select_mode != SELECT_MODE_SINGLE_CORE)
//		return; // Maybe this could happen in rare circumstances? not sure

 play_interface_sound(SAMPLE_BLIP2, TONE_2G);


 if (control.mouse_x_screen_pixels >= view.build_buttons_x2 - 42
		&& control.mouse_x_screen_pixels <= view.build_buttons_x2 - 10)
//		&& control.mouse_y_screen_pixels >= button_y - (queue_button_highlight_mouseover * BUILD_BUTTON_H) + 4,
//		&& control.mouse_y_screen_pixels <= button_y - (queue_button_highlight_mouseover * BUILD_BUTTON_H) + BUILD_BUTTON_H - 2 - 4)
	{

   int i;

	 	for (i = button; i < BUILD_QUEUE_LENGTH - 1; i ++)
		 {
		 	w.player[game.user_player_index].build_queue[i] = w.player[game.user_player_index].build_queue[i+1]; // struct assignment!
		 }

   reset_build_queue_buttons_y1(-1);

  	w.player[game.user_player_index].build_queue_fail_reason = BUILD_SUCCESS; // reset any error from a previous build attempt

		 return;

	}


 control.mouse_drag = MOUSE_DRAG_BUILD_QUEUE;
 control.mouse_drag_build_queue_button = button;

 return;

/*
	 	core->build_command_queue [COMMAND_QUEUE-1].active = 0;
   if (core->build_command_queue [0].active != 0)
				core->new_build_command = 1;*/



}



// call this when user clicks on a place after pressing a build button.
static void build_place_selected(void)
{
 if (w.command_mode != COMMAND_MODE_COMMAND)
		return;

	command.build_mode = BUILD_MODE_ANGLE;
	command.build_position.x = al_itofix(control.mouse_x_world_pixels);
	command.build_position.y = al_itofix(control.mouse_y_world_pixels);
	command.default_build_angle = 0;
	command.build_angle = 0; // should probably change this to angle from builder proc

	block_cart new_block_position;

	new_block_position.x = fixed_to_block(command.build_position.x);
	new_block_position.y = fixed_to_block(command.build_position.y);

	if (!verify_block_position(new_block_position))
	{
		command.build_mode = BUILD_MODE_NONE;
//  play_interface_sound(SAMPLE_BLIP1, TONE_1G);
		return;
	}

// play_interface_sound(SAMPLE_BLIP1, TONE_1B);


}

static void update_build_position(void)
{

	if (control.mouse_status == MOUSE_STATUS_PANEL
			&& control.mouse_panel == PANEL_MAIN)
	{
		command.build_position.x = al_itofix(control.mouse_x_world_pixels);
		command.build_position.y = al_itofix(control.mouse_y_world_pixels);
		if (command.build_position.x < BLOCK_SIZE_FIXED)
			command.build_position.x = BLOCK_SIZE_FIXED;
		if (command.build_position.x > w.fixed_size.x - BLOCK_SIZE_FIXED)
			command.build_position.x = w.fixed_size.x - BLOCK_SIZE_FIXED;
		if (command.build_position.y < BLOCK_SIZE_FIXED)
			command.build_position.y = BLOCK_SIZE_FIXED;
		if (command.build_position.y > w.fixed_size.y - BLOCK_SIZE_FIXED)
			command.build_position.y = w.fixed_size.y - BLOCK_SIZE_FIXED;
	}

	check_build_validity(1);

}

// call this function to continually update build angle after build position selected.
static void update_build_angle(void)
{

	if (control.mouse_status == MOUSE_STATUS_PANEL
			&& control.mouse_panel == PANEL_MAIN)
	 command.build_angle = get_angle(al_itofix(control.mouse_y_world_pixels) - command.build_position.y, al_itofix(control.mouse_x_world_pixels) - command.build_position.x);
	  else
				command.build_angle = command.default_build_angle;

	check_build_validity(1);

}

// This function is based on draw_notional_group in i_display.c,
// plus a bit of the new process code from g_proc_new.c.
// It checks whether a proc can be validly built without colliding with anything or being out of range etc
//  and indicates which components will collide.
// Set check_collisions to zero to avoid checking for collisions.
// Because it uses some display-related values it's not perfectly accurate. Need to think about fixing this.
static void check_build_validity(int check_collisions)
{

 struct template_struct* build_templ = &templ[game.user_player_index][command.build_template_index];
 int i;
 command.build_fail_collision = 0; // 1 if any member is in collision
 command.build_fail_edge = 0; // 1 if any member is off map
 command.build_fail_static = 0; // 1 if core is static and on data well
 command.build_fail_range = 0; // 1 if core is static and target out of range

 block_cart new_block_position;

// should be able to assume that command.build_position coordinates are within the world area (see bounds-checking in update_build_position):
 if (build_templ->member[0].shape < FIRST_MOBILE_NSHAPE
		&& !check_static_build_location_for_data_wells(command.build_position.x, command.build_position.y))
//		&& w.backblock[fixed_to_block(command.build_position.x)][fixed_to_block(command.build_position.y)].backblock_type != BACKBLOCK_BASIC_HEX)
	{
		command.build_fail_static = 1;
		for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
		{
   command.build_member_collision [i] = 1;
		}
		return;
	}


 add_notional_member_recursively(&templ[game.user_player_index][command.build_template_index],
																																	0, // member index
																																	command.build_position,
																																	(command.build_angle + templ[game.user_player_index][command.build_template_index].member[0].group_angle_offset) & AFX_MASK,
																																	0); // tells add_notional_member_recursively not to fail

// collision and map edge test using notional values
 for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (build_templ->member [i].exists)
		{
		 command.build_member_collision [i] = 0;
	 	new_block_position.x = fixed_to_block(notional_member[i].position.x);
  	new_block_position.y = fixed_to_block(notional_member[i].position.y);

  	if (!verify_block_position(new_block_position))
		 {
	   command.build_member_collision [i] = 1;
 	  command.build_fail_edge = 1;
		 }
		  else
				{
					struct core_struct* unused_collision_core_parameter;
// can call check_notional_block_collision_multi because target build position has been verified.
		   if (check_collisions
						&& check_block_visible_to_user(new_block_position.x, new_block_position.y) // this is a pretty crude check - think about fixing it
		    && check_notional_block_collision_multi(notional_member[i].shape, notional_member[i].position.x, notional_member[i].position.y, notional_member[i].angle, build_templ->mobile, game.user_player_index, &unused_collision_core_parameter))
		   {
	     command.build_member_collision [i] = 1;
 	    command.build_fail_collision = 1;
		   }
				}
		}
	}

// may need to check for range:
					if (command.selected_core [0] != -1
					 && w.proc [w.core [command.selected_core [0]].process_index].shape < FIRST_MOBILE_NSHAPE // must be a static core (don't care about range if the builder is mobile, as it can move to the location)
					 && distance_oct_xyxy(command.build_position.x, command.build_position.y, w.core [command.selected_core [0]].core_position.x, w.core [command.selected_core [0]].core_position.y) > al_itofix(BUILD_RANGE_BASE_PIXELS))
					{
						for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
						{
	      command.build_member_collision [i] = 1;
						}
 	    command.build_fail_range = 1;
					}


}


// call this function after user finishes selecting build angle. It issues a command to selected[0]
// it will queue if shift pressed (this is the point where shift must be pressed to queue)
// Note: queued=1 means that the command will not replace other build commands from the same core.
//  the build command will be added to the build queue regardless!
static void give_command_after_build_angle_selected(int queued)
{

// first verify selected[0]
 if (command.selected_core[0] < 0 // could be SELECT_TERMINATE or SELECT_EMPTY
	 || w.core	[command.selected_core[0]].number_of_build_objects == 0) // could happen if component with build object destroyed since selection.
		return; // maybe print a message to console?

// for now, put new build command at the *end* of the queue:

 if (command.build_fail_range) // build_fail_range should only be 1 if the builder is static. out of range build commands are okay for mobile builders
	{
 	 write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LRED, -1, 0, "\nFailed - out of range.");
 	 return;
	}

 if (command.build_fail_edge)
	{
 	 write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LRED, -1, 0, "\nFailed - location outside map.");
 	 return;
	}

 if (command.build_fail_static)
	{
 	 write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LRED, -1, 0, "\nFailed - static process too close to data well.");
 	 return;
	}



 if (!add_to_build_queue(game.user_player_index,
																				     command.selected_core[0],
																				     command.build_template_index,
																				     al_fixtoi(command.build_position.x),
																				     al_fixtoi(command.build_position.y),
																				     fixed_angle_to_int(command.build_angle),
																				     (ex_control.special_key_press [SPECIAL_KEY_CTRL] > 0), // back or front (ctrl sets it to front)
																				     (ex_control.special_key_press [SPECIAL_KEY_SHIFT] > 0), // repeat
																				     1, // queued (if 0, replaces other build commands for this process)
//																				     (ex_control.special_key_press [SPECIAL_KEY_CTRL] > 0),
//																				     queued,
																				     1))
	{
// add_to_build_queue returns 0 if build queue full (1 on success, -1 on error)
 	 write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LRED, -1, 0, "\nBuild queue full.");
	}
/*
 w.core	[command.selected_core[0]].new_build_command = 1;

// w.core	[command.selected_core[0]].build_command_timestamp = game.total_time; not currently used
 w.core	[command.selected_core[0]].build_command_queue[new_command_index].active = 1;
 w.core	[command.selected_core[0]].build_command_queue[new_command_index].build_template = command.build_template_index;
 w.core	[command.selected_core[0]].build_command_queue[new_command_index].build_x = al_fixtoi(command.build_position.x);
 w.core	[command.selected_core[0]].build_command_queue[new_command_index].build_y = al_fixtoi(command.build_position.y);
 w.core	[command.selected_core[0]].build_command_queue[new_command_index].build_angle = fixed_angle_to_int(command.build_angle);
 w.core	[command.selected_core[0]].build_command_queue[new_command_index].build_command_ctrl = 0;
 if (ex_control.special_key_press [SPECIAL_KEY_CTRL] > 0)
  w.core	[command.selected_core[0]].build_command_queue[new_command_index].build_command_ctrl = 1;

// if not queued, clear the rest of the queue:
 if (!queued)
	{
		int i;
		for (i = 1; i < BUILD_COMMAND_QUEUE; i ++)
		{
   w.core	[command.selected_core[0]].build_command_queue[i].active = 0;
		}
	}
*/
// now leave it to the process to work out what to do about the build command.

}


// this can be called by user commands,
//  and also by processes calling the add_to_build_queue (or similar) method
// doesn't confirm that build command is possible
// returns 1 on success, 0 if build queue full (calling function may print an error message), -1 on error
s16b add_to_build_queue(int player_index, int builder_core_index, int template_index, int build_x, int build_y, int angle, int back_or_front, int repeat, int queue_for_this_core, int failure_message)
{


 sancheck(builder_core_index, 0, w.max_cores, "add_to_build_queue");

// make sure the builder actually has a build object:
 if (templ[player_index][w.core[builder_core_index].template_index].first_build_object_member == -1)
		return -1;

	if (!queue_for_this_core)
		clear_build_queue_for_core(player_index, builder_core_index);

	int build_queue_index = 0;

	while(w.player[player_index].build_queue[build_queue_index].active)
	{
		build_queue_index ++;
	};

	if (build_queue_index >= BUILD_QUEUE_LENGTH - 2) // room for terminator
	{
// print failure message here if failure_message==1
		return 0; // build queue full
	}

 if (back_or_front)
	{
		build_queue_index = 0; // ignore value calculated before, but the calculation was still needed to confirm room in queue
		int i = BUILD_QUEUE_LENGTH - 1;
		while (i > 0)
		{
			w.player[player_index].build_queue [i] = w.player[player_index].build_queue [i - 1];
			i --;
		}
	}
	 else
		{
// terminate the queue:
   w.player[player_index].build_queue [build_queue_index + 1].active = 0;
		}


	if (build_queue_index == 0)
		w.player[player_index].build_queue_fail_reason = BUILD_SUCCESS; // means no error

 sancheck(build_queue_index, 0, BUILD_QUEUE_LENGTH, "build_queue_index");

 w.player[player_index].build_queue [build_queue_index].active = 1;
 w.player[player_index].build_queue [build_queue_index].core_index = builder_core_index;
	w.player[player_index].build_queue [build_queue_index].template_index = template_index;
	w.player[player_index].build_queue [build_queue_index].build_x = build_x;
	w.player[player_index].build_queue [build_queue_index].build_y = build_y;
	w.player[player_index].build_queue [build_queue_index].angle = angle;
//	w.player[player_index].build_queue [build_queue_index].target_index = target_index;
	w.player[player_index].build_queue [build_queue_index].repeat = repeat;

 if (player_index == game.user_player_index
		&& command.display_build_buttons)
	{
  reset_build_queue_buttons_y1(build_queue_index + 1);
 	w.player[game.user_player_index].build_queue_fail_reason = BUILD_SUCCESS; // reset any error from a previous build attempt
	}


	return 1;

}


// removes all build commands for a particular core from the queue
void clear_build_queue_for_core(int player_index, int remove_core_index)
{

	int check_index;
	int write_index = 0;

 sancheck(remove_core_index, 0, w.max_cores, "remove_core_index");

// may need to clear the build fail indicator:
		if (w.player[player_index].build_queue[0].core_index == remove_core_index)
			w.player[player_index].build_queue_fail_reason = BUILD_SUCCESS; // means no error

	for (check_index = 0; check_index < BUILD_QUEUE_LENGTH; check_index ++)
	{
		if (w.player[player_index].build_queue[check_index].core_index != remove_core_index
			|| !w.player[player_index].build_queue[check_index].active)
		{
   sancheck(write_index, 0, BUILD_QUEUE_LENGTH, "write_index");
			w.player[player_index].build_queue[write_index] = w.player[player_index].build_queue[check_index];
			write_index++;
		}
	}

 if (player_index == game.user_player_index
		&& command.display_build_buttons)
	{

  reset_build_queue_buttons_y1(-1);

 	w.player[game.user_player_index].build_queue_fail_reason = BUILD_SUCCESS; // reset any error from a previous build attempt

  if (write_index != 0
 		&& control.mouse_drag == MOUSE_DRAG_BUILD_QUEUE)
			 control.mouse_drag = MOUSE_DRAG_NONE;
	}

}

void requeue_repeat_build(int player_index)
{

	struct build_queue_struct build_queue_saved = w.player[player_index].build_queue[0];

	build_queue_next(player_index, 0);

	int i = 0;

	while(w.player[player_index].build_queue[i].active)
	{
		i++;
	};

 sancheck(i, 0, BUILD_QUEUE_LENGTH - 1, "requeue_repeat_build:i"); // - 1 is because there's a [+1] just below

	w.player[player_index].build_queue[i] = build_queue_saved;

// terminate:
 w.player[player_index].build_queue[i+1].active = 0;
// (should be able to assume that build_queue[i+1] is valid as build_queue_next should have shifted everything 1 towards the front of the queue

 if (player_index == game.user_player_index
		&& command.display_build_buttons)
	{
  reset_build_queue_buttons_y1(-1);
 	w.player[game.user_player_index].build_queue_fail_reason = BUILD_SUCCESS; // reset any error from a previous build attempt
	}

}

void build_queue_next(int player_index, int reset_build_queue_buttons)
{

	int i;

	for (i = 0; i < BUILD_QUEUE_LENGTH - 1; i ++)
	{
		w.player[player_index].build_queue [i] = w.player[player_index].build_queue [i+1];
	}

 if (control.mouse_drag == MOUSE_DRAG_BUILD_QUEUE)
			control.mouse_drag = MOUSE_DRAG_NONE;

//		if (w.player[player_index].build_queue[0].core_index == remove_core_index)
	w.player[player_index].build_queue_fail_reason = BUILD_SUCCESS; // means no error

 if (reset_build_queue_buttons
		&& player_index == game.user_player_index
		&& command.display_build_buttons)
	{
  reset_build_queue_buttons_y1(-1);
 	w.player[game.user_player_index].build_queue_fail_reason = BUILD_SUCCESS; // reset any error from a previous build attempt
	}



}

// called when the user has dragged a build queue button to another position in the queue.
void rearrange_build_queue(void)
{

 int moved_queue_entry = control.mouse_drag_build_queue_button;

 int new_position = work_out_queue_drag_position();

 if (new_position == -1)
		return;

	if (moved_queue_entry == new_position)
		return;

 sancheck(moved_queue_entry, 0, BUILD_QUEUE_LENGTH, "moved_queue_entry");

	struct build_queue_struct temp_build_queue_entry = w.player[game.user_player_index].build_queue[moved_queue_entry];

	int i;

 if (moved_queue_entry == 0
	 || new_position == 0)
			w.player[game.user_player_index].build_queue_fail_reason = BUILD_SUCCESS; // means no error

// Moving from an earlier position to a later position:
 if (moved_queue_entry < new_position)
	{
		i = moved_queue_entry;

		while (i < new_position)
		{
   sancheck(i, 0, BUILD_QUEUE_LENGTH - 1, "rearrange_build_queue:i"); // note the - 1

			w.player[game.user_player_index].build_queue[i] = w.player[game.user_player_index].build_queue[i+1];
			i ++;
		}

  sancheck(i, 0, BUILD_QUEUE_LENGTH, "rearrange_build_queue:i B"); // note the - 1

	 w.player[game.user_player_index].build_queue[i] = temp_build_queue_entry;
	 return;
	}

// Moving from a later position to an earlier position:
 i = moved_queue_entry;

	while (i > new_position)
	{
   sancheck(i, 0, BUILD_QUEUE_LENGTH - 1, "rearrange_build_queue:i C"); // note the - 1
			w.player[game.user_player_index].build_queue[i] = w.player[game.user_player_index].build_queue[i-1];
			i --;
	}

	 w.player[game.user_player_index].build_queue[i] = temp_build_queue_entry;

}

int work_out_queue_drag_position(void)
{

 int moved_queue_entry = control.mouse_drag_build_queue_button;

 sancheck(moved_queue_entry, 0, BUILD_QUEUE_LENGTH, "work_out_queue_drag_position:moved_queue_entry");

 if (!w.player[game.user_player_index].build_queue[moved_queue_entry].active)
		return -1;

	int queue_y = view.build_buttons_y2 - (BUILD_BUTTON_H * (TEMPLATES_PER_PLAYER + 3)) - 10;

// int new_position = (queue_y - control.mouse_y_screen_pixels - (BUILD_BUTTON_H / 2)) / BUILD_BUTTON_H;
 int new_position = (queue_y - control.mouse_y_screen_pixels + BUILD_BUTTON_H) / BUILD_BUTTON_H;

 sancheck(new_position, 0, BUILD_QUEUE_LENGTH, "work_out_queue_drag_position:new_position");

// new_position ++;

 if (new_position < 0)
		new_position = 0;

	if (new_position >= BUILD_QUEUE_LENGTH)
		new_position = BUILD_QUEUE_LENGTH - 1;


// I'm not sure it's safe to assume that an entry past the first inactive entry will have .active==0,
//  as the list may just be terminated by a single .active==0 entry.
// so let's look back from the front of the queue:

 int queue_length = 0;

 while (w.player[game.user_player_index].build_queue[queue_length].active)
	{
		queue_length ++;
	};

 sancheck(queue_length, 0, BUILD_QUEUE_LENGTH, "work_out_queue_drag_position:queue_length D");
 sancheck(new_position, 0, BUILD_QUEUE_LENGTH, "work_out_queue_drag_position:new_position");

 if	(new_position > queue_length - 1)
		new_position = queue_length - 1;

	return new_position;

}

int check_proc_visible_to_user(int proc_index)
{

 sancheck(proc_index, 0, w.max_procs, "check_proc_visible_to_user");


	if (game.vision_mask
		&&	w.vision_area[game.user_player_index][w.proc[proc_index].block_position.x][w.proc[proc_index].block_position.y].vision_time < w.world_time - VISION_AREA_VISIBLE_TIME)
		return 0;

	return 1;

}


// assumes target block has been bounds-checked
// checks *user* visibility, not *player* (or process) visibility
int check_block_visible_to_user(int block_x, int block_y)
{

	if (game.vision_mask
		&&	w.vision_area[game.user_player_index][block_x][block_y].vision_time < w.world_time - VISION_AREA_VISIBLE_TIME)
		return 0;

	return 1;

}

// Looks for the first selected core which actually exists. Returns -1 if nothing selected
static int find_first_selected_core(void)
{
	int i;

	for (i = 0; i < SELECT_MAX; i ++)
	{
		if (command.selected_core [i] == SELECT_TERMINATE)
			return -1;
		if (command.selected_core [i] != SELECT_EMPTY)
			return command.selected_core [i];
	}

	return -1;

}

// when control-# is pressed this assigns the current selection to a control group
// assumes that control_group_index is 0 to (CONTROL_GROUPS-1) (currently there are 7 control groups)
static void set_control_group(int control_group_index, int exclusive)
{

 if (exclusive)
		remove_selection_from_control_group();

	int i = 0;
	int select_index = 0;

 while(command.selected_core [select_index] != SELECT_TERMINATE)
	{
//		fpr("\n csc [%i] = %i", select_index, command.selected_core [select_index]);
		sancheck(select_index, 0, SELECT_MAX, "set_control_group: select_index");
		if (command.selected_core [select_index] != SELECT_EMPTY
			&& w.core[command.selected_core [select_index]].player_index == game.user_player_index)
		{
//			fpr("\n c.cgc [%i] [%i] = %i", control_group_index, i, command.selected_core [select_index]);
 		command.control_group_core [control_group_index] [i] = command.selected_core [select_index];
 		command.control_group_core_timestamp [control_group_index] [i] = w.core[command.selected_core [select_index]].created_timestamp;
 		i ++;
		}
		select_index ++;
	}
//fpr("\n i %i cgi %i command.control_group_core [control_group_index] [i] %i", i, control_group_index, command.control_group_core [control_group_index] [i]);
//		sancheck(command.control_group_core [control_group_index] [i], 0, w.max_cores, "set_control_group: command.control_group_core [control_group_index]");


	command.control_group_core [control_group_index] [i] = SELECT_TERMINATE;

	char temp_str [60];

	if (i == 0)
	{
  command.control_group_core [control_group_index] [0] = SELECT_TERMINATE; // possible that there are SELECT_EMPTY entries at the start
	 sprintf(temp_str, "\nControl group %i cleared.", control_group_index);
  play_interface_sound(SAMPLE_BLIP1, TONE_2D);
	}
  else
			{
				if (exclusive)
	    sprintf(temp_str, "\nControl group %i set (exclusive).", control_group_index);
	     else
  	    sprintf(temp_str, "\nControl group %i set.", control_group_index);
    play_interface_sound(SAMPLE_BLIP1, TONE_2G);
			}

 write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LBLUE, -1, 0, temp_str);


}



// when shift-# is pressed this adds the current selection to a control group
// assumes that control_group_index is 0-9
static void add_to_control_group(int control_group_index)
{
	int i = 0;
	int select_index = 0;
	int group_full = 0;
	int number_added = 0;

	int found_current_group_member = 0;


 while(command.selected_core [select_index] != SELECT_TERMINATE)
	{
  sancheck(select_index, 0, SELECT_MAX, "add_to_control_group: select_index");
		if (command.selected_core [select_index] != SELECT_EMPTY
//			&& !is_core_in_control_group(command.selected_core [select_index], control_group_index)
			&& w.core[command.selected_core [select_index]].player_index == game.user_player_index)
		{

			if (is_core_in_control_group(command.selected_core [select_index], control_group_index))
			{
				found_current_group_member = 1;
				select_index ++;
				continue;
			}

// need to find an empty control group entry:
   while (command.control_group_core [control_group_index] [i] >= 0 // SELECT_EMPTY and SELECT_TERMINATE are both < 0
						&& w.core[command.control_group_core [control_group_index] [i]].exists
						&& w.core[command.control_group_core [control_group_index] [i]].created_timestamp == command.control_group_core_timestamp [control_group_index] [i])
			{
				i ++;
		  if (i >= SELECT_MAX - 1)
		  {
			  group_full = 1;
			  break; // need to check for this here (unlike in set_control_group) because the user may be trying to add too many processes
		  }
			}
			if (group_full)
				break;
// if we're adding to the end of the control group, need to put the terminator back in (so that the is_core_in_control_group() call in the while() test above will work properly)
			if (command.control_group_core [control_group_index] [i] == SELECT_TERMINATE)
				command.control_group_core [control_group_index] [i + 1] = SELECT_TERMINATE;
 		command.control_group_core [control_group_index] [i] = command.selected_core [select_index];
 		command.control_group_core_timestamp [control_group_index] [i] = w.core[command.selected_core [select_index]].created_timestamp;
 		number_added ++;
 		i ++; // need to add this here so that when the terminator is added after the loop it will be in the correct position.
// need to duplicate the test here
	  if (i >= SELECT_MAX - 1)
	  {
		  group_full = 1;
		  break; // need to check for this here (unlike in set_control_group) because the user may be trying to add too many processes
	  }
		}
		select_index ++;
	}

//	command.control_group_core [control_group_index] [i] = SELECT_TERMINATE; - no, can't assume that i is at the end

	char temp_str [50];

	if (i == 0)
	{
		if (found_current_group_member)
		{
	    sprintf(temp_str, "\nAlready assigned to control group %i.", control_group_index);
     write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LBLUE, -1, 0, temp_str);
     play_interface_sound(SAMPLE_BLIP1, TONE_2D);
     return;
		}
//  command.control_group_core [control_group_index] [0] = SELECT_TERMINATE; // possible that there are SELECT_EMPTY entries at the start
//	 sprintf(temp_str, "\nNothing selected.", control_group_index);
  write_text_to_console(CONSOLE_GENERAL, PRINT_COL_DBLUE, -1, 0, "\nNo processes selected.");
  play_interface_sound(SAMPLE_BLIP1, TONE_2CS);
  return;
	}
  else
			{
				if (number_added == 0)
				{
 	   sprintf(temp_str, "\nControl group %i full.", control_group_index);
     write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LRED, -1, 0, temp_str);
     play_interface_sound(SAMPLE_BLIP1, TONE_2CS);
     return;
				}
 	   else
						{
  	    sprintf(temp_str, "\nAdded to control group %i.", control_group_index);
       write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LBLUE, -1, 0, temp_str);
       play_interface_sound(SAMPLE_BLIP1, TONE_2G);
       return;
						}
			}

}


// when control-# is pressed this assigns the current selection to a control group
// assumes that control_group_index is 0 to (CONTROL_GROUPS-1) (currently there are 7 control groups)
static void remove_selection_from_control_group(void)
{
//	int i = 0;
	int j, k;
	int select_index = 0;

 while(command.selected_core [select_index] != SELECT_TERMINATE)
	{
		if (command.selected_core [select_index] != SELECT_EMPTY
			&& w.core[command.selected_core [select_index]].player_index == game.user_player_index)
		{
   for (j = 0; j < CONTROL_GROUPS; j ++)
			{
 			k = 0;
				while (command.control_group_core [j] [k] != SELECT_TERMINATE)
				{
					if (command.control_group_core [j] [k] == command.selected_core [select_index]
						&& command.control_group_core_timestamp [j] [k] == w.core[command.selected_core [select_index]].created_timestamp)
					{
						command.control_group_core [j] [k] = SELECT_EMPTY;
						break;
					}
					k++;
				}; // end while
			} // end for j
		}
		select_index ++;
	}

}



static int is_core_in_control_group(int core_index, int control_group_index)
{
	int i = 0;
	while (i < SELECT_MAX)
	{
		if (command.control_group_core [control_group_index] [i] == SELECT_TERMINATE)
		{
			return 0;
		}
		if (command.control_group_core [control_group_index] [i] == core_index
			&& w.core[core_index].created_timestamp == command.control_group_core_timestamp [control_group_index] [i])
		{
				return 1;
		}

		i ++;
	}

	return 0;

}

static void select_control_group(int control_group_index)
{


 clear_selection();

	int i = 0;
	int select_index = 0;

	while(command.control_group_core [control_group_index] [i] != SELECT_TERMINATE)
	{
		if (command.control_group_core [control_group_index] [i] != SELECT_EMPTY)
		{
			if (!w.core[command.control_group_core [control_group_index] [i]].exists
				||	w.core[command.control_group_core [control_group_index] [i]].created_timestamp != command.control_group_core_timestamp [control_group_index] [i])
			{
				command.control_group_core [control_group_index] [i] = SELECT_EMPTY; // not strictly necessary
				i ++;
				continue;
			}

//			add_core_to_selection(command.control_group_core [control_group_index] [i]);
			command.selected_core [select_index] = command.control_group_core [control_group_index] [i];
			w.core[command.control_group_core [control_group_index] [i]].selected = select_index;
			w.core[command.control_group_core [control_group_index] [i]].select_time = game.total_time;
			select_index ++;
		}


		i ++;

	} // end loop

//fpr("[i%i:si%i]) ", i, select_index);



	command.selected_core [select_index] = SELECT_TERMINATE;

	char temp_str [50];

	if (select_index == 0)
	{
  command.control_group_core [control_group_index] [0] = SELECT_TERMINATE; // possible that there are SELECT_EMPTY entries at the start
	 sprintf(temp_str, "\nControl group %i empty.", control_group_index);
  write_text_to_console(CONSOLE_GENERAL, PRINT_COL_DBLUE, -1, 0, temp_str);
  play_interface_sound(SAMPLE_BLIP1, TONE_2D);
  return;
	}

// at least one core has been selected...


	if (select_index == 1)
	{
		select_a_core(command.selected_core [0]);
		command.selected_member = 0;
	}
		else
		{
			if (select_index > 1)
			{
				command.select_mode = SELECT_MODE_MULTI_CORE;
    play_interface_sound(SAMPLE_BLIP1, TONE_2G);
			}
		}

	if (command.last_control_group_selected == control_group_index
		&& command.last_control_group_select_time >= game.total_time - 64)
	{
			view.camera_x = w.core[command.selected_core [0]].core_position.x;
			view.camera_y = w.core[command.selected_core [0]].core_position.y;
// no message needed if you do this
	}
	 else
		{

   command.last_control_group_selected = control_group_index;
   command.last_control_group_select_time = game.total_time;

 	 sprintf(temp_str, "\nControl group %i selected.", control_group_index);
   write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LBLUE, -1, 0, temp_str);

		}





}




/*
old debugging function

void print_control_group(int control_group_index, char* text)
{

	fpr("\n CG [%s] ", text);

	int i;

	for (i = 0; i < SELECT_MAX; i ++)
	{
		fpr("%i ", command.control_group_core [control_group_index] [i]);
	}

}*/




void clear_control_groups(void)
{
	int i;

	for (i = 0; i < CONTROL_GROUPS; i ++)
	{

		command.control_group_core [i] [0] = SELECT_TERMINATE;

	}

}


/*

Build commands:

single-selecting a process with a build object causes the build buttons to appear
they're a list of buildable templates above the bottom-left console
clicking on one:
 - enters the build placement mode
 - in BP mode, an outline of the new process follows the mouse.
 - click once to place proc, and hold for a moment to enter build angle mode
 - in build angle mode, choose angle of new process
 - build angle mode ends when button let go

after that, the build command is sent to the process.
build command elements are:
 - template index
 - x,y
 - angle (int)

process can then use these commands to build the new process.

maybe build objects should have a cooldown?


 (object_build).build(template, x_offset, y_offset, angle); - returns 1 (success), 0 (failure) [TO DO: give more information about failure reason]
 (object_build).build_test(template, x_offset, y_offset, angle); - doesn't interrupt build
	get_template_cost(template)
	print_template_name(template)

std methods:
 check_new_build_command() - returns 1 if a new build command has been given
// all remaining build command methods work on the most recent build command given, whenever that was:
get_build_command_x() // this probably needs to be absolute. process will need to convert to an offset
get_build_command_y()
get_build_command_angle()
get_build_command_template()

clear_build_command() // process can choose to ignore build command anyway, but this stops the user interface displaying the build command as pending

// proc build command data:
 int build_command_given; // is set to 1 when command given, then to 0 when checked
 timestamp build_command_timestamp;
 int build_command_template;
 int build_command_x, build_command_y; // translate from fixed to int when given
 int build_command_angle; // same


*/




/*

How will commands work?

 - each core will have a queue of 3 or 4 commands
 - possible commands are:
COM_NONE
COM_LOCATION: x, y
COM_TARGET: x, y, (target - probably don't need this; x/y should be sufficient for the process although the target will need to be stored by the game)


command methods:

get_command_type() - returns command type
get_command_x()
get_command_y()
get_commands() - returns number of commands in queue
clear_command() - clears command [0] and pulls the queue forwards



*/

