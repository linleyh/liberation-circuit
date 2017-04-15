#include <allegro5/allegro.h>

#include <stdio.h>

#include "m_config.h"

#include "g_header.h"

#include "m_maths.h"
#include "g_misc.h"
#include "g_motion.h"
#include "g_command.h"
#include "m_globvars.h"
#include "m_input.h"
#include "e_slider.h"
#include "e_header.h"
#include "c_header.h"
#include "e_editor.h"
//#include "e_slider.h"
#include "i_console.h"
#include "i_view.h"

#include "p_panels.h"

void initialise_control(void);
void run_mouse_drag(void);
void	go_to_under_attack_marker(void);

extern struct game_struct game;
extern struct view_struct view;
extern struct command_struct command;

struct control_struct control;



// this array indicates which allegro_key enum corresponds to each KEY_? enum. Used for allowing system/clob programs to access keyboard
int corresponding_allegro_key [KEYS] =
{

ALLEGRO_KEY_0, // KEY_0,
ALLEGRO_KEY_1, // KEY_1,
ALLEGRO_KEY_2, // KEY_2,
ALLEGRO_KEY_3, // KEY_3,
ALLEGRO_KEY_4, // KEY_4,
ALLEGRO_KEY_5, // KEY_5,
ALLEGRO_KEY_6, // KEY_6,
ALLEGRO_KEY_7, // KEY_7,
ALLEGRO_KEY_8, // KEY_8,
ALLEGRO_KEY_9, // KEY_9,

ALLEGRO_KEY_A, // KEY_A,
ALLEGRO_KEY_B, // KEY_B,
ALLEGRO_KEY_C, // KEY_C,
ALLEGRO_KEY_D, // KEY_D,
ALLEGRO_KEY_E, // KEY_E,
ALLEGRO_KEY_F, // KEY_F,
ALLEGRO_KEY_G, // KEY_G,
ALLEGRO_KEY_H, // KEY_H,
ALLEGRO_KEY_I, // KEY_I,
ALLEGRO_KEY_J, // KEY_J,
ALLEGRO_KEY_K, // KEY_K,
ALLEGRO_KEY_L, // KEY_L,
ALLEGRO_KEY_M, // KEY_M,
ALLEGRO_KEY_N, // KEY_N,
ALLEGRO_KEY_O, // KEY_O,
ALLEGRO_KEY_P, // KEY_P,
ALLEGRO_KEY_Q, // KEY_Q,
ALLEGRO_KEY_R, // KEY_R,
ALLEGRO_KEY_S, // KEY_S,
ALLEGRO_KEY_T, // KEY_T,
ALLEGRO_KEY_U, // KEY_U,
ALLEGRO_KEY_V, // KEY_V,
ALLEGRO_KEY_W, // KEY_W,
ALLEGRO_KEY_X, // KEY_X,
ALLEGRO_KEY_Y, // KEY_Y,
ALLEGRO_KEY_Z, // KEY_Z,

ALLEGRO_KEY_MINUS, // KEY_MINUS,
ALLEGRO_KEY_EQUALS, // KEY_EQUALS,
ALLEGRO_KEY_OPENBRACE, // KEY_SBRACKET_OPEN,
ALLEGRO_KEY_CLOSEBRACE, // KEY_SBRACKET_CLOSE,
ALLEGRO_KEY_BACKSLASH, // KEY_BACKSLASH,
ALLEGRO_KEY_SEMICOLON, // KEY_SEMICOLON,
ALLEGRO_KEY_QUOTE, // KEY_APOSTROPHE,
ALLEGRO_KEY_COMMA, // KEY_COMMA,
ALLEGRO_KEY_FULLSTOP, // KEY_PERIOD,
ALLEGRO_KEY_SLASH, // KEY_SLASH,

ALLEGRO_KEY_UP, // KEY_UP,
ALLEGRO_KEY_DOWN, // KEY_DOWN,
ALLEGRO_KEY_LEFT, // KEY_LEFT,
ALLEGRO_KEY_RIGHT, // KEY_RIGHT,

ALLEGRO_KEY_ENTER, // KEY_ENTER,
ALLEGRO_KEY_BACKSPACE, // KEY_BACKSPACE,
ALLEGRO_KEY_INSERT, // KEY_INSERT,
ALLEGRO_KEY_HOME, // KEY_HOME,
ALLEGRO_KEY_PGUP, // KEY_PGUP,
ALLEGRO_KEY_PGDN, // KEY_PGDN,
ALLEGRO_KEY_DELETE, // KEY_DELETE,
ALLEGRO_KEY_END, // KEY_END,



ALLEGRO_KEY_TAB, // KEY_TAB,
// KEY_ESCAPE is not available to user programs

ALLEGRO_KEY_PAD_0, // KEY_PAD_0,
ALLEGRO_KEY_PAD_1, // KEY_PAD_1,
ALLEGRO_KEY_PAD_2, // KEY_PAD_2,
ALLEGRO_KEY_PAD_3, // KEY_PAD_3,
ALLEGRO_KEY_PAD_4, // KEY_PAD_4,
ALLEGRO_KEY_PAD_5, // KEY_PAD_5,
ALLEGRO_KEY_PAD_6, // KEY_PAD_6,
ALLEGRO_KEY_PAD_7, // KEY_PAD_7,
ALLEGRO_KEY_PAD_8, // KEY_PAD_8,
ALLEGRO_KEY_PAD_9, // KEY_PAD_9,
ALLEGRO_KEY_PAD_MINUS, // KEY_PAD_MINUS,
ALLEGRO_KEY_PAD_PLUS, // KEY_PAD_PLUS,
ALLEGRO_KEY_PAD_ENTER, // KEY_PAD_ENTER,
ALLEGRO_KEY_PAD_DELETE, // KEY_PAD_DELETE,

ALLEGRO_KEY_LSHIFT, // KEY_LSHIFT,
ALLEGRO_KEY_RSHIFT, // KEY_RSHIFT,
ALLEGRO_KEY_LCTRL, // KEY_LCTRL,
ALLEGRO_KEY_RCTRL, // KEY_RCTRL,

ALLEGRO_KEY_F1, // KEY_F1
ALLEGRO_KEY_F2, // etc.
ALLEGRO_KEY_F3,
ALLEGRO_KEY_F4,
ALLEGRO_KEY_F5,
ALLEGRO_KEY_F6,
ALLEGRO_KEY_F7,
ALLEGRO_KEY_F8,
ALLEGRO_KEY_F9,
ALLEGRO_KEY_F10,
ALLEGRO_KEY_F11,
ALLEGRO_KEY_F12,

//KEYS

};



// this is called at startup, and when input capture is set to the editor, and also when loading from disk
void initialise_control(void)
{
 control.mouse_status = MOUSE_STATUS_PANEL; // TO DO: make sure this is correct (could be wrong if it's possible for game to start with editor open and mouse in editor window)
 control.mouse_panel = PANEL_MAIN;
 control.mouse_x_world_pixels = 0;
 control.mouse_y_world_pixels = 0;
 control.mouse_x_screen_pixels = 0;
 control.mouse_y_screen_pixels = 0;
 control.mbutton_press_timestamp [0] = 0;
 control.mbutton_press_timestamp [1] = 0;

 control.mouse_drag = MOUSE_DRAG_NONE;
 control.mouse_drag_panel = 0;
 control.mouse_drag_element = 0;

 control.panel_element_highlighted = -1;
	control.panel_element_highlighted_time = 0;

	control.editor_captures_input = 0;


 int i;

 for (i = 0; i < MOUSE_BUTTONS; i ++)
	{
  control.mbutton_press [i] = BUTTON_NOT_PRESSED;
	}

// for (i = 0; i < KEYS; i ++)
// {
  //control.key_press [i] = BUTTON_NOT_PRESSED;
// }
// control.any_key = -1;

// control.mouse_hold_x_pixels [0] = -1;
// control.mouse_hold_x_pixels [1] = -1;
/*
 int i;

 for (i = 0; i < MBB_BITS; i ++)
 {
  control.mb_bits [i] = 0;
 }*/


}


void run_input(void)
{


/*
  if (game.phase == GAME_PHASE_PREGAME)
		{
   control.mouse_status = MOUSE_STATUS_OUTSIDE;
   control.any_key = -1;
			return;
		}
*/
  int i;

// this code removes focus from a proc that has been destroyed
//  - possibly it can be removed? (TO DO: check whether everything focus_proc does can be delegated to client program)
  //if (view.focus_proc != NULL
   //&& view.focus_proc->exists == 0)
  //{
//   view.focus_proc = NULL;
//   reset_proc_box_height(NULL);
  //}

  control.editor_captures_input = 0;

  if (ex_control.mouse_on_display == 0)
  {
   control.mouse_status = MOUSE_STATUS_OUTSIDE;
   goto mouse_unavailable;
  }

 control.mouse_status = MOUSE_STATUS_PANEL;
 control.mouse_panel = PANEL_MAIN;

	for (i = 0; i < PANELS; i ++)
	{
		if (panel[i].open
			&&	control.mouse_x_screen_pixels >= panel[i].x1
			&& control.mouse_y_screen_pixels >= panel[i].y1
			&&	control.mouse_x_screen_pixels <= panel[i].x2
			&& control.mouse_y_screen_pixels <= panel[i].y2)
			{
			 control.mouse_panel = i;
			 break;
			} // probably use <= because maximum mouse_x should be in rightmost open panel

	}

 if (control.mouse_panel != PANEL_MAIN
		&& panel[PANEL_EDITOR].open)
		control.editor_captures_input = 1;
			else
	  {

// check for the mouse pointer being in the console window, a score box etc:
    check_mouse_on_consoles_etc(ex_control.mouse_x_pixels, ex_control.mouse_y_pixels, (ex_control.mb_press [0] == BUTTON_JUST_PRESSED));
	  }

// not sure if the following need to be set if the editor is capturing input

  control.mouse_x_world_pixels = (ex_control.mouse_x_pixels / view.zoom + al_fixtoi(view.camera_x - view.centre_x_zoomed)) ;
  control.mouse_y_world_pixels = (ex_control.mouse_y_pixels / view.zoom + al_fixtoi(view.camera_y - view.centre_y_zoomed));

  control.mouse_x_screen_pixels = ex_control.mouse_x_pixels;
  control.mouse_y_screen_pixels = ex_control.mouse_y_pixels;

//  run_mouse_interface();

mouse_unavailable:

  for (i = 0; i < MOUSE_BUTTONS; i ++)
  {
   switch(control.mbutton_press [i])
   {
    case BUTTON_NOT_PRESSED:
     if (ex_control.mb_press [i] != BUTTON_NOT_PRESSED)
					{
      control.mbutton_press [i] = BUTTON_JUST_PRESSED;
      control.mbutton_press_timestamp [i] = game.total_time;
					}
     break;
    case BUTTON_JUST_RELEASED:
     if (ex_control.mb_press [i] <= BUTTON_NOT_PRESSED)
      control.mbutton_press [i] = BUTTON_NOT_PRESSED;
       else
					  {
        control.mbutton_press [i] = BUTTON_JUST_PRESSED;
        control.mbutton_press_timestamp [i] = game.total_time;
					  }
     break;
    case BUTTON_JUST_PRESSED:
     if (ex_control.mb_press [i] <= BUTTON_NOT_PRESSED)
      control.mbutton_press [i] = BUTTON_JUST_RELEASED;
       else
        control.mbutton_press [i] = BUTTON_HELD;
     break;
    case BUTTON_HELD:
     if (ex_control.mb_press [i] <= BUTTON_NOT_PRESSED)
      control.mbutton_press [i] = BUTTON_JUST_RELEASED;
     break;
   }
  }

 if (control.mouse_drag != MOUSE_DRAG_NONE)
	{
  run_mouse_drag();
	}


// control.any_key = -1;
/*
// we need to specially calculate whether the keys have just been pressed etc, rather than relying on ex_control values, because the control_struct is saved to file and ex_control may not be reliable when loading/saving
 for (i = KEYS - 1; i >= 0; i --) // Counts down so that any_key detects letters in preference to shift, control etc
 {
  switch(control.key_press[i])
  {
   case BUTTON_JUST_RELEASED:
    if (ex_control.key_press [corresponding_allegro_key [i]] == BUTTON_NOT_PRESSED)
     control.key_press [i] = BUTTON_NOT_PRESSED;
      else
						{
       control.key_press [i] = BUTTON_JUST_PRESSED;
       control.any_key = i;
						}
    break;
   case BUTTON_NOT_PRESSED:
    if (ex_control.key_press [corresponding_allegro_key [i]] != BUTTON_NOT_PRESSED)
				{
     control.key_press [i] = BUTTON_JUST_PRESSED; // if ex_control value is BUTTON_JUST_RELEASED, control value will be set to BUTTON_JUST_PRESSED and then (probably) to BUTTON_JUST_RELEASED the next tick
     control.any_key = i;
				}
    break;
   case BUTTON_JUST_PRESSED:
    if (ex_control.key_press [corresponding_allegro_key [i]] <= BUTTON_NOT_PRESSED) // <= BUTTON_NOT_PRESSED means BUTTON_NOT_PRESSED (0) or BUTTON_JUST_RELEASED (-1)
     control.key_press [i] = BUTTON_JUST_RELEASED;
      else
						{
       control.key_press [i] = BUTTON_HELD;
//       control.any_key = i;
						}
    break;
   case BUTTON_HELD:
    if (ex_control.key_press [corresponding_allegro_key [i]] <= BUTTON_NOT_PRESSED) // <= BUTTON_NOT_PRESSED means BUTTON_NOT_PRESSED (0) or BUTTON_JUST_RELEASED (-1)
     control.key_press [i] = BUTTON_JUST_RELEASED;
    break;
  }
 }

*/



 if (ex_control.special_key_press [SPECIAL_KEY_F1] == BUTTON_JUST_PRESSED)
	{
		if (!w.debug_mode_general)
		{
			write_text_to_console(CONSOLE_GENERAL, PRINT_COL_DBLUE, -1, 0, "Debug mode activated.");
			w.debug_mode_general = 1;
		}
		 else
 		{
	 		write_text_to_console(CONSOLE_GENERAL, PRINT_COL_DBLUE, -1, 0, "Debug mode deactivated.");
		 	w.debug_mode_general = 0;
		 }
	}

// fast forward only available in world phase (not pregame or game over):
//  (game.fast_forward is also set to 0 in game over code)
 if (game.phase == GAME_PHASE_WORLD
		&& !control.editor_captures_input)
	{

 if (ex_control.special_key_press [SPECIAL_KEY_F2] == BUTTON_JUST_PRESSED)
	{
		if (game.fast_forward == 0
			|| (game.fast_forward != 0
				&& game.fast_forward_type != FAST_FORWARD_TYPE_SMOOTH))
		{
			game.fast_forward = 1;
			game.fast_forward_type = FAST_FORWARD_TYPE_SMOOTH;
		}
		  else
			  game.fast_forward = 0;
	}

 if (ex_control.special_key_press [SPECIAL_KEY_F3] == BUTTON_JUST_PRESSED)
	{
		if (game.fast_forward == 0
			|| (game.fast_forward != 0
				&& game.fast_forward_type != FAST_FORWARD_TYPE_SKIP))
		{
			game.fast_forward = 1;
			game.fast_forward_type = FAST_FORWARD_TYPE_SKIP;
		}
		  else
			  game.fast_forward = 0;
	}

 if (ex_control.special_key_press [SPECIAL_KEY_F4] == BUTTON_JUST_PRESSED)
	{
		if (game.fast_forward == 0
			|| (game.fast_forward != 0
				&& game.fast_forward_type != FAST_FORWARD_TYPE_NO_DISPLAY))
		{
			game.fast_forward = 1;
			game.fast_forward_type = FAST_FORWARD_TYPE_NO_DISPLAY;
		}
		  else
			  game.fast_forward = 0;
	}

	}

//	if (game.phase == GAME_PHASE_WORLD
//		|| game.phase == GAME_PHASE_PREGAME)
	{

 	if (ex_control.special_key_press [SPECIAL_KEY_F6] == BUTTON_JUST_PRESSED)
 	{
		 if (panel[PANEL_BCODE].open)
 			close_panel(PANEL_BCODE, 0);
			  else
 					open_panel(PANEL_BCODE);
 	}


 	if (ex_control.special_key_press [SPECIAL_KEY_F7] == BUTTON_JUST_PRESSED)
 	{
		 if (panel[PANEL_TEMPLATE].open)
 			close_panel(PANEL_TEMPLATE, 0);
			  else
 					open_panel(PANEL_TEMPLATE);
 	}

 	if (ex_control.special_key_press [SPECIAL_KEY_F8] == BUTTON_JUST_PRESSED)
 	{
		 if (panel[PANEL_DESIGN].open)
 			close_panel(PANEL_DESIGN, 0);
			  else
 					open_panel(PANEL_DESIGN);
 	}

 	if (ex_control.special_key_press [SPECIAL_KEY_F9] == BUTTON_JUST_PRESSED)
 	{
// 		fpr("\nA[%i]", panel[PANEL_EDITOR].open);
		 if (panel[PANEL_EDITOR].open)
 			close_panel(PANEL_EDITOR, 0);
			  else
 					open_panel(PANEL_EDITOR);
 	}
/*
 	if (ex_control.special_key_press [SPECIAL_KEY_F8] == BUTTON_JUST_PRESSED)
 	{
		 if (panel[PANEL_SYSMENU].open)
 			close_panel(PANEL_SYSMENU, 0);
			  else
 					open_panel(PANEL_SYSMENU);
 	}*/

	}







 if (control.editor_captures_input)
	 return;

/*
	if (ex_control.mousewheel_change == -1
		 && control.mouse_panel == PANEL_MAIN)
	{
		if (view.zoom_int < view.zoom_target [3])
		 view.zoom_int ++;
		view.zoom = view.zoom_int * 0.03;
		reset_view_values(view.window_x_unzoomed, view.window_y_unzoomed);
//		view.zoom += 0.009;
//		reset_view_values(view.window_x_unzoomed, view.window_y_unzoomed);
	}

	if	(ex_control.mousewheel_change == 1
		 && control.mouse_panel == PANEL_MAIN)
	{
		if (view.zoom_int > view.zoom_target [1])
		 view.zoom_int --;
		view.zoom = view.zoom_int * 0.03;
		reset_view_values(view.window_x_unzoomed, view.window_y_unzoomed);
//		view.zoom -= 0.009;
//		if (view.zoom < 0.3)
//			view.zoom = 0.3;
//		reset_view_values(view.window_x_unzoomed, view.window_y_unzoomed);
	}
*/
//int zoom_target [ZOOM_MAX_LEVEL + 1] = {10, 16, 26, 36};
/*
// if any of this zoom stuff changes, may also need to change in initialise_view() in i_view.c
	if (view.zoom_int != view.zoom_target [view.zoom_level])
	{
		if (view.zoom_int < view.zoom_target [view.zoom_level])
			view.zoom_int ++;
		if (view.zoom_int > view.zoom_target [view.zoom_level])
			view.zoom_int --;
		view.zoom = view.zoom_int * 0.03;
		reset_view_values(view.window_x_unzoomed, view.window_y_unzoomed);
	}
*/

/*

	if (ex_control.mousewheel_change == -1
		 && control.mouse_panel == PANEL_MAIN)
	{
		if (view.zoom_level < ZOOM_MAX_LEVEL)
		 view.zoom_level ++;
//		view.zoom += 0.009;
//		reset_view_values(view.window_x_unzoomed, view.window_y_unzoomed);
	}

	if	(ex_control.mousewheel_change == 1
		 && control.mouse_panel == PANEL_MAIN)
	{
		if (view.zoom_level > 1)
		 view.zoom_level --;
//		view.zoom -= 0.009;
//		if (view.zoom < 0.3)
//			view.zoom = 0.3;
//		reset_view_values(view.window_x_unzoomed, view.window_y_unzoomed);
	}

//int zoom_target [ZOOM_MAX_LEVEL + 1] = {10, 16, 26, 36};

// if any of this zoom stuff changes, may also need to change in initialise_view() in i_view.c
	if (view.zoom_int != view.zoom_target [view.zoom_level])
	{
		if (view.zoom_int < view.zoom_target [view.zoom_level])
			view.zoom_int ++;
		if (view.zoom_int > view.zoom_target [view.zoom_level])
			view.zoom_int --;
		view.zoom = view.zoom_int * 0.03;
		reset_view_values(view.window_x_unzoomed, view.window_y_unzoomed);
	}

*/

	if (ex_control.mousewheel_change == -1
		 && control.mouse_panel == PANEL_MAIN)
	{
		if (view.zoom_level < ZOOM_MAX_LEVEL)
		 view.zoom_level ++;
//		view.zoom += 0.009;
//		reset_view_values(view.window_x_unzoomed, view.window_y_unzoomed);
	}

	if	(ex_control.mousewheel_change == 1
		 && control.mouse_panel == PANEL_MAIN)
	{
		if (view.zoom_level > 1)
		 view.zoom_level --;
//		view.zoom -= 0.009;
//		if (view.zoom < 0.3)
//			view.zoom = 0.3;
//		reset_view_values(view.window_x_unzoomed, view.window_y_unzoomed);
	}

//int zoom_target [ZOOM_MAX_LEVEL + 1] = {10, 16, 26, 36};

// if any of this zoom stuff changes, may also need to change in initialise_view() in i_view.c
	if (view.zoom_int != view.zoom_target [view.zoom_level])
	{
		if (view.zoom_int < view.zoom_target [view.zoom_level])
			view.zoom_int ++;
		if (view.zoom_int > view.zoom_target [view.zoom_level])
			view.zoom_int --;
		view.zoom = view.zoom_int * 0.03;
		reset_view_values(view.window_x_unzoomed, view.window_y_unzoomed);
	}


 if (ex_control.unichar_input == 'f')
	{
		view.following ^= 1;
	}


 if (ex_control.unichar_input == ' ')
	{
		go_to_under_attack_marker();
	}

 if (ex_control.unichar_input == 'p') // fix!
	{
		if (game.pause_soft == 0)
			game.pause_soft = 1;
		  else
				{
			  game.pause_soft = 0;
			  flush_game_event_queues();
				}
	}

 static int pause_advance = 0;

 if (pause_advance == 1)
	{
		game.pause_soft = 1;
		pause_advance = 0;
	}

 if (ex_control.unichar_input == '[')
	{
		if (pause_advance == 0)
		{
			game.pause_soft = 0;
			pause_advance = 1;
		}
	}


#ifdef DEBUG_MODE
 if (ex_control.special_key_press [SPECIAL_KEY_F5] == BUTTON_JUST_PRESSED)
#else
 if (ex_control.special_key_press [SPECIAL_KEY_F5] == BUTTON_JUST_PRESSED
		&& game.type != GAME_TYPE_MISSION)
#endif
	{
		clear_selection();
		clear_control_groups(); // there's not really any sensible way to deal with control groups when switching players (other than having a separate control group array for each possible player... nah)
		game.user_player_index ++;
		if (game.user_player_index >= w.players)
			game.user_player_index = 0;

	}
/*
#ifdef DEBUG_MODE

 if (ex_control.special_key_press [SPECIAL_KEY_F7] == BUTTON_JUST_PRESSED)
	{
		if (ex_control.debug_special_keys == 0)
			ex_control.debug_special_keys = 1;
 		 else
					ex_control.debug_special_keys = 0;
	}


 if (ex_control.special_key_press [SPECIAL_KEY_F8] == BUTTON_JUST_PRESSED)
	{

static int mrand_seed;
fpr("\n resetting music - area %i seed %i", game.area_index, mrand_seed);
		 reset_music(game.area_index, -1, mrand_seed);

		 mrand_seed++;

	}


#endif
*/

// control may not reach here (e.g. if input being captured by editor  - control.editor_captures_input == 1)

}

void go_to_under_attack_marker(void)
{

 if (view.under_attack_marker_last_time < w.world_time - UNDER_ATTACK_MARKER_DURATION)
		return; // no current markers

	int i;

 int current_marker = -1;
 al_fixed closest_distance = view.centre_x_zoomed;
 int distance_from_camera;

 for (i = 0; i < UNDER_ATTACK_MARKERS; i ++)
	{
		if (view.under_attack_marker [i].time_placed_world >= w.world_time - UNDER_ATTACK_MARKER_DURATION)
		{
			distance_from_camera = abs(view.camera_x - view.under_attack_marker[i].position.x) + abs(view.camera_y - view.under_attack_marker[i].position.y);
			if (distance_from_camera < closest_distance)
			{
				closest_distance = distance_from_camera;
				current_marker = i;
			}
		}
	}

// current_marker may be -1 at this point

	i = current_marker + 1;

	while(i < UNDER_ATTACK_MARKERS - 1)
	{
		if (view.under_attack_marker [i].time_placed_world >= w.world_time - UNDER_ATTACK_MARKER_DURATION)
		{
			view.camera_x = view.under_attack_marker[i].position.x;
			view.camera_y = view.under_attack_marker[i].position.y;
			return;
		}
		i++;
	}

	i = 0;

	while(i < current_marker)
	{
		if (view.under_attack_marker [i].time_placed_world >= w.world_time - UNDER_ATTACK_MARKER_DURATION)
		{
			view.camera_x = view.under_attack_marker[i].position.x;
			view.camera_y = view.under_attack_marker[i].position.y;
			return;
		}
		i++;
	}


}

// called if mouse_drag != MOUSE_DRAG_NONE
void run_mouse_drag(void)
{

	int released = 0;

 if (control.mbutton_press [0] <= 0)
		released = 1;

	switch(control.mouse_drag)
	{
	 case MOUSE_DRAG_SLIDER:
	 	run_slider(panel[control.mouse_drag_panel].element[control.mouse_drag_element].value [1]);
	 	if (released == 1)
			{
				control.mouse_drag = MOUSE_DRAG_NONE;
			}
		 break;
		case MOUSE_DRAG_BUILD_QUEUE:
			if (!command.display_build_buttons)
			{
				control.mouse_drag = MOUSE_DRAG_NONE;
				break;
			}
				if (control.mouse_x_screen_pixels > view.build_buttons_x1
		   && control.mouse_x_screen_pixels < view.build_buttons_x2)
				{
//		   if (control.mouse_y_screen_pixels > view.build_queue_buttons_y1
//	     && control.mouse_y_screen_pixels < view.build_queue_buttons_y2)
//	   {
  			if (released)
		  	{
//		  		fpr("\nRearrange");
	   	 	rearrange_build_queue();
 						control.mouse_drag = MOUSE_DRAG_NONE;
		  	}
//	   }
				}
	    else
						control.mouse_drag = MOUSE_DRAG_NONE;


/*
				if (control.mouse_x_screen_pixels > view.build_buttons_x1
		   && control.mouse_x_screen_pixels < view.build_buttons_x2
		   && control.mouse_y_screen_pixels > view.build_queue_buttons_y1
	    && control.mouse_y_screen_pixels < view.build_queue_buttons_y2)
	   {
  			if (released)
		  	{
		  		fpr("\nRearrange");
	   	 	rearrange_build_queue();
 						control.mouse_drag = MOUSE_DRAG_NONE;
		  	}
	   }
	    else
						control.mouse_drag = MOUSE_DRAG_NONE;
*/
			break;

	}

}


