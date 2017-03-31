#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"
#include "t_template.h"
#include "e_slider.h"
#include "e_header.h"
#include "c_header.h"
#include "e_editor.h"
#include "e_help.h"
#include "i_view.h"
#include "i_sysmenu.h"
#include "i_header.h"
#include "i_display.h"
#include "i_buttons.h"
#include "g_misc.h"
#include "c_prepr.h"

#include "m_maths.h"

#include "m_input.h"
#include "x_sound.h"

extern struct editorstruct editor;
extern struct view_struct view;
extern struct game_struct game;

struct key_typestruct key_type [ALLEGRO_KEY_MAX];

//void check_mode_buttons_and_panel_drag(void);
void mode_button(int mode_pressed);
static void close_window_box(void);
static void init_character_input_events(void);
static void read_character_input_event(void);
void init_key_maps(void);
//static void capture_mouse(void);


extern ALLEGRO_DISPLAY* display;

ALLEGRO_EVENT_QUEUE* control_queue;
extern ALLEGRO_EVENT_QUEUE* event_queue;
extern struct fontstruct font [FONTS];


void init_ex_control(void)
{

// Set up the control event queue:
   control_queue = al_create_event_queue();
   if (!control_queue)
   {
      fprintf(stdout, "\nm_input.c: init_ex_control(): Error: failed to create control_queue.");
      safe_exit(-1);
   }

   ALLEGRO_EVENT_SOURCE* acquire_event_source = al_get_mouse_event_source();
   if (acquire_event_source == NULL)
   {
      fprintf(stdout, "\nm_input.c: init_ex_control(): Error: failed to get mouse event source.");
      safe_exit(-1);
   }
   al_register_event_source(control_queue, acquire_event_source);

   acquire_event_source = al_get_display_event_source(display);
   if (acquire_event_source == NULL)
   {
      fprintf(stdout, "\nm_input.c: init_ex_control(): Error: failed to get display event source.");
      safe_exit(-1);
   }
   al_register_event_source(control_queue, acquire_event_source);

 ex_control.mouse_x_pixels = 0;
 ex_control.mouse_y_pixels = 0;
 ex_control.sticky_ctrl = 0;
 ex_control.numlock = 0;
 ex_control.mousewheel_change = 0;
// ex_control.using_slider = 0;
 ex_control.mouse_on_display = 1;
 ex_control.mouse_dragging_panel = 0;
	ex_control.mouse_cursor_type = MOUSE_CURSOR_BASIC;
	ex_control.mouse_grabbed = 0;


 int i;
/*
 for (i = 1; i < ALLEGRO_KEY_MAX; i ++)
 {
  ex_control.key_press [i] = BUTTON_NOT_PRESSED;
 }*/

 for (i = 0; i < MOUSE_BUTTONS; i ++)
	{
  ex_control.mb_press [i] = BUTTON_NOT_PRESSED;
	}

 for (i = 0; i < SPECIAL_KEYS; i ++)
	{
		ex_control.special_key_press [i] = BUTTON_NOT_PRESSED;
		ex_control.special_key_press_time [i] = 0; // inter.running_time should start at 255 or something
	}

 ex_control.keys_pressed = 0;

// need to know current mousewheel position to be able to initialise it:
  ALLEGRO_MOUSE_STATE mouse_state;

  al_get_mouse_state(&mouse_state);

  ex_control.mousewheel_pos = mouse_state.w;

  inter.running_time = 255; // give it a bit of a buffer so that 0 can be used as "a long time ago" for everything

  ex_control.unichar_input = 0;
//  ex_control.unichar_input_received = 0;

  init_character_input_events();

}

// close_button_status indicates what to do if the user clicks on the native close window button or presses escape
// 0 means: open a close_window_box (usual action)
// 1 means: exit immediately (start screen only)
// 2 means: do nothing (probably means this is being called from inside close_window_box())
void get_ex_control(int close_button_status, int grab_mouse_if_captured)
{

  int i;

  ALLEGRO_EVENT control_event;

// This seems like the best place to put the running_time increment, as it should be called constantly in any mode.
  inter.running_time ++;

  read_character_input_event();
/*
  if (ex_control.unichar_input != 0)
		{
			char temp_str [2];
			temp_str [0] = ex_control.unichar_input;
			temp_str [1] = 0;

			fpr("\n key pressed: %i [%s]", ex_control.unichar_input, temp_str);
		}
*/
  while (al_get_next_event(control_queue, &control_event))
  {
   if (control_event.type == ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY)
			{
				if (settings.option [OPTION_CAPTURE_MOUSE])
				{
					if (grab_mouse_if_captured)
					{
 					al_grab_mouse(display);
 					ex_control.mouse_grabbed = 1;
					}
 					else
							{
								if (ex_control.mouse_grabbed)
								{
									al_ungrab_mouse();
    					ex_control.mouse_grabbed = 1;
								}
							}


				}
    ex_control.mouse_on_display = 0;
			}
     else
     {
      if (control_event.type != ALLEGRO_EVENT_DISPLAY_CLOSE)
      {
// general mouse actions:
       if (control_event.mouse.display != NULL)
        ex_control.mouse_on_display = 1;
      }
       else
       {
        if (close_button_status == 0)
         close_window_box(); // program may exit from this.
        if (close_button_status == 1)
         safe_exit(0);
//        if (close_button_status == 2) - do nothing
       }
     }
  }

//   fprintf(stdout, "%i", ex_control.mouse_on_display);


  //if (ex_control.mouse_on_display)
  {

  	ex_control.old_mouse_x_pixels = ex_control.mouse_x_pixels; // these values are used in g_command.c for dragging the screen with the middle mouse button
  	ex_control.old_mouse_y_pixels = ex_control.mouse_y_pixels;


   ALLEGRO_MOUSE_STATE mouse_state;

   al_get_mouse_state(&mouse_state);


   ex_control.mouse_x_pixels = mouse_state.x;
   ex_control.mouse_y_pixels = mouse_state.y;
/*
   if (settings.option [OPTION_CAPTURE_MOUSE])
			{
// This deals with a reported problem with multi-monitor setups.
// If OPTION_CAPTURE_MOUSE is set in init.txt, the mouse is forced back onto the display:
    int relocate_mouse = 0;

// However, this doesn't work - if the mouse leaves the display, its last valid x/y values are kept and these may not be right on the edge.
//  this is now dealt with by


    if (ex_control.mouse_x_pixels >= inter.display_w - 1)
				{
					ex_control.mouse_x_pixels = inter.display_w - 1;
					relocate_mouse = 1;
				}

    if (ex_control.mouse_y_pixels >= inter.display_h - 1)
				{
					ex_control.mouse_y_pixels = inter.display_h - 1;
					relocate_mouse = 1;
				}

    if (ex_control.mouse_x_pixels <= 0)
				{
					ex_control.mouse_x_pixels = 0;
					relocate_mouse = 1;
				}

    if (ex_control.mouse_y_pixels <= 0)
				{
					ex_control.mouse_y_pixels = 0;
					relocate_mouse = 1;
				}

				if (relocate_mouse)
					al_set_mouse_xy(display, ex_control.mouse_x_pixels, ex_control.mouse_y_pixels);



			}
			 else
*/
				{

// it's possible that in some circumstances mouse_state.x/y is outside the expected display area
//  (this may explain one reported bug that I couldn't reproduce).
// the bounds-checks in the following code prevent this:

     if (ex_control.mouse_x_pixels < 0)
			   ex_control.mouse_x_pixels = 0;
     if (ex_control.mouse_x_pixels >= inter.display_w)
			   ex_control.mouse_x_pixels = inter.display_w - 1;


     if (ex_control.mouse_y_pixels < 0)
			   ex_control.mouse_y_pixels = 0;
     if (ex_control.mouse_y_pixels >= inter.display_h)
			   ex_control.mouse_y_pixels = inter.display_h - 1;

				}


   for (i = 0; i < MOUSE_BUTTONS; i ++)
   {
    switch(ex_control.mb_press [i])
    {
     case BUTTON_NOT_PRESSED:
      if (mouse_state.buttons & (1 << i))
       ex_control.mb_press [i] = BUTTON_JUST_PRESSED;
      break;
     case BUTTON_JUST_PRESSED:
      if (mouse_state.buttons & (1 << i))
       ex_control.mb_press [i] = BUTTON_HELD;
        else
          ex_control.mb_press [i] = BUTTON_JUST_RELEASED;
      break;
     case BUTTON_JUST_RELEASED:
      if (mouse_state.buttons & (1 << i))
       ex_control.mb_press [i] = BUTTON_JUST_PRESSED;
        else
         ex_control.mb_press [i] = BUTTON_NOT_PRESSED;
      break;
     case BUTTON_HELD:
      if (!(mouse_state.buttons & (1 << i)))
       ex_control.mb_press [i] = BUTTON_JUST_RELEASED;
      break;
    }
   }


// if the left mouse button isn't being pressed, stop using any slider currently being used:
//   if (ex_control.mb_press [0] <= 0)
//    ex_control.using_slider = 0;

   ex_control.mousewheel_change = 0;

   if (mouse_state.z < ex_control.mousewheel_pos)
    ex_control.mousewheel_change = 1;

   if (mouse_state.z > ex_control.mousewheel_pos)
    ex_control.mousewheel_change = -1;

   ex_control.mousewheel_pos = mouse_state.z;

  }



  ALLEGRO_KEYBOARD_STATE key_state;

  al_get_keyboard_state(&key_state);
  ex_control.keys_pressed = 0;

  if (al_key_down(&key_state, ALLEGRO_KEY_ESCAPE))
  {
   if (close_button_status == 0)
    close_window_box(); // program may exit from this.
   if (close_button_status == 1)
    safe_exit(0);
  }

#ifdef DEBUG_MODE
 if (ex_control.debug_special_keys)
	{
		fpr("\n ex map %i/%i ",
  ex_control.key_code_map [0] [0],
  ex_control.key_code_map [0] [1]);
		for (i = 0; i < KEY_CODES; i ++)
		{
			if (ex_control.key_code_map [i] [0] == 0)
				fpr("kcm [%i][0]=0 ", i);
			if (ex_control.key_code_map [i] [1] == 0)
				fpr("kcm [%i][1]=0 ", i);
		}
	fpr("\n key0A st %i ti %i", ex_control.special_key_press [0], ex_control.special_key_press_time [0]);
	}

#endif

// We deal with key codes in separate steps, so that numlock prevents a pad key being treated as a cursor key
//  (it will be interpreted as a unichar number or . instead)
  for (i = 0; i < KEY_PAD_START; i ++)
  {
   if (ex_control.key_code_map [i] [0] != -1 // no key (used for duplicated remapped keys
				&& al_key_down(&key_state, ex_control.key_code_map [i] [0]))
   {
    ex_control.keys_pressed ++;
   	ex_control.special_key_press_time [ex_control.key_code_map [i] [1]] = inter.running_time;
   }
  }

 if (!ex_control.numlock) // if numlock is on,
	{
  for (i = KEY_PAD_START; i < (KEY_PAD_END + 1); i ++)
  {
   if (ex_control.key_code_map [i] [0] != -1 // no key (used for duplicated remapped keys
				&& al_key_down(&key_state, ex_control.key_code_map [i] [0]))
   {
    ex_control.keys_pressed ++;
   	ex_control.special_key_press_time [ex_control.key_code_map [i] [1]] = inter.running_time;
   }
  }
	}

  for (i = (KEY_PAD_END+1); i < KEY_CODES; i ++)
  {
   if (ex_control.key_code_map [i] [0] != -1 // no key (used for duplicated remapped keys
				&& al_key_down(&key_state, ex_control.key_code_map [i] [0]))
   {
    ex_control.keys_pressed ++;
   	ex_control.special_key_press_time [ex_control.key_code_map [i] [1]] = inter.running_time;
   }
  }







// need to have two loops because some special key values have more than one key code (e.g. lshift and rshift both affect SPECIAL_KEY_SHIFT)

#ifdef DEBUG_MODE
 if (ex_control.debug_special_keys)
	{
 	fpr("\n key0B st %i ti %i", ex_control.special_key_press [0], ex_control.special_key_press_time [0]);
	}

#endif

  for (i = 0; i < SPECIAL_KEYS; i ++)
  {
// this switch takes the most recent status for the key and updates it based on current keypress status
  	switch(ex_control.special_key_press [i])
  	{
			 case BUTTON_JUST_RELEASED:
					if (ex_control.special_key_press_time [i] == inter.running_time)
						ex_control.special_key_press [i] = BUTTON_JUST_PRESSED;
						 else
  						ex_control.special_key_press [i] = BUTTON_NOT_PRESSED;
  			break;
				case BUTTON_NOT_PRESSED:
					if (ex_control.special_key_press_time [i] == inter.running_time)
						ex_control.special_key_press [i] = BUTTON_JUST_PRESSED;
					break;
				case BUTTON_JUST_PRESSED:
					if (ex_control.special_key_press_time [i] == inter.running_time)
						ex_control.special_key_press [i] = BUTTON_HELD;
						 else
  						ex_control.special_key_press [i] = BUTTON_JUST_RELEASED;
					break;
				case BUTTON_HELD:
					if (ex_control.special_key_press_time [i] != inter.running_time)
						ex_control.special_key_press [i] = BUTTON_JUST_RELEASED;
					break;

  	}
  }

#ifdef DEBUG_MODE
 if (ex_control.debug_special_keys)
	{
 	fpr("\n key0C st %i ti %i", ex_control.special_key_press [0], ex_control.special_key_press_time [0]);
	}

#endif

/*
  for (i = 0; i < KEY_CODES; i ++)
  {
// this switch takes the most recent status for the key and updates it based on current keypress status
  	switch(ex_control.special_key_press [ex_control.key_code_map [i] [1]])
  	{
			 case BUTTON_JUST_RELEASED:
					if (ex_control.special_key_press_time [ex_control.key_code_map [i] [1]] == inter.running_time)
						ex_control.special_key_press [ex_control.key_code_map [i] [1]] = BUTTON_JUST_PRESSED;
						 else
  						ex_control.special_key_press [ex_control.key_code_map [i] [1]] = BUTTON_NOT_PRESSED;
  			break;
				case BUTTON_NOT_PRESSED:
					if (ex_control.special_key_press_time [ex_control.key_code_map [i] [1]] == inter.running_time)
						ex_control.special_key_press [ex_control.key_code_map [i] [1]] = BUTTON_JUST_PRESSED;
					break;
				case BUTTON_JUST_PRESSED:
					if (ex_control.special_key_press_time [ex_control.key_code_map [i] [1]] == inter.running_time)
						ex_control.special_key_press [ex_control.key_code_map [i] [1]] = BUTTON_HELD;
					break;
				case BUTTON_HELD:
					if (ex_control.special_key_press_time [ex_control.key_code_map [i] [1]] != inter.running_time)
						ex_control.special_key_press [ex_control.key_code_map [i] [1]] = BUTTON_JUST_RELEASED;
					break;

  	}
  }
*/

/*
  ALLEGRO_KEYBOARD_STATE key_state;

  al_get_keyboard_state(&key_state);
  ex_control.keys_pressed = 0;

  if (al_key_down(&key_state, ALLEGRO_KEY_ESCAPE))
  {
   if (close_button_status == 0)
    close_window_box(); // program may exit from this.
   if (close_button_status == 1)
    safe_exit(0);
  }

  for (i = 1; i < ALLEGRO_KEY_MAX; i ++)
  {
   if (al_key_down(&key_state, i))
   {
    ex_control.keys_pressed ++;
    switch(ex_control.key_press [i])
    {
     case BUTTON_JUST_RELEASED:
     case BUTTON_NOT_PRESSED:
      ex_control.key_press [i] = BUTTON_JUST_PRESSED; break;
     case BUTTON_JUST_PRESSED:
//     case BUTTON_HELD:
      ex_control.key_press [i] = BUTTON_HELD; break;
    }
   }
    else
    {
// not being pressed
     switch(ex_control.key_press [i])
     {
      case BUTTON_JUST_PRESSED:
      case BUTTON_HELD:
       ex_control.key_press [i] = BUTTON_JUST_RELEASED; break;
      case BUTTON_JUST_RELEASED:
//     case BUTTON_HELD:
       ex_control.key_press [i] = BUTTON_NOT_PRESSED; break;
     }
    }
  }
*/

  if (ex_control.special_key_press [SPECIAL_KEY_CTRL] > 0)
//			|| ex_control.key_press [ALLEGRO_KEY_RCTRL] > 0)
		{
			ex_control.sticky_ctrl = 1;
		}
		 else
			{
				if (ex_control.keys_pressed == 0)
					ex_control.sticky_ctrl = 0; // after being set to 1, sticky_ctrl stays as 1 until all keys are released. Used in e_editor.c.
			}


// check_mode_buttons_and_panel_drag();

#ifdef DEBUG_MODE
 if (ex_control.debug_special_keys)
	{
 	fpr("\n key0D st %i ti %i", ex_control.special_key_press [0], ex_control.special_key_press_time [0]);
	}

#endif
}

/*
// called if settings.option [OPTION_CAPTURE_MOUSE] is set
static void capture_mouse(void)
{

							al_grab_mouse(display);

// unfortunately, in windowed mode al_grab_mouse doesn't confine the mouse to the display part of the window:

   if (settings.option [OPTION_FULLSCREEN]
				|| settings.option [OPTION_FULLSCREEN_TRUE])
					return;

// so we need to try to work out which side of the display the mouse went off :(
// let's try to do that using the mouse's recent movement:

  int mouse_move_x = ex_control.mouse_x_pixels - ex_control.old_mouse_x_pixels;
  int mouse_move_y = ex_control.mouse_y_pixels - ex_control.old_mouse_y_pixels;

int relocate_mouse = 0;

    if (ex_control.mouse_x_pixels >= inter.display_w - 1)
				{
					ex_control.mouse_x_pixels = inter.display_w - 1;
					relocate_mouse = 1;
				}

    if (ex_control.mouse_y_pixels >= inter.display_h - 1)
				{
					ex_control.mouse_y_pixels = inter.display_h - 1;
					relocate_mouse = 1;
				}

    if (ex_control.mouse_x_pixels <= 0)
				{
					ex_control.mouse_x_pixels = 0;
					relocate_mouse = 1;
				}

    if (ex_control.mouse_y_pixels <= 0)
				{
					ex_control.mouse_y_pixels = 0;
					relocate_mouse = 1;
				}

				if (relocate_mouse)
					al_set_mouse_xy(display, ex_control.mouse_x_pixels, ex_control.mouse_y_pixels);



}
*/

ALLEGRO_EVENT_QUEUE* char_input_queue;



// initialises the event queue for direct character input.
// called once, at startup.
static void init_character_input_events(void)
{


 char_input_queue = al_create_event_queue();
 al_register_event_source(char_input_queue, al_get_keyboard_event_source());


}


static void read_character_input_event(void)
{

 ex_control.unichar_input = 0;

 ALLEGRO_EVENT char_input_event;

 while(al_get_next_event(char_input_queue, &char_input_event))
	{

	 if (char_input_event.type == ALLEGRO_EVENT_KEY_CHAR)
		{
			ex_control.unichar_input = char_input_event.keyboard.unichar;

 		if (char_input_event.keyboard.modifiers & ALLEGRO_KEYMOD_NUMLOCK)
	 		ex_control.numlock = 1;
		   else
  	 		ex_control.numlock = 0;

			return; // stop reading events once a character found (any further events will be dealt with next tick)
		}

	}

/*
 if (al_get_next_event(char_input_queue, &char_input_event))
	{
	 if (char_input_event.type == ALLEGRO_EVENT_KEY_CHAR)
		{
			ex_control.unichar_input = char_input_event.keyboard.unichar;

		if (char_input_event.keyboard.modifiers & ALLEGRO_KEYMOD_NUMLOCK)
			ex_control.numlock = 1;
		  else
  			ex_control.numlock = 0;

		}
	}
*/
}


void init_key_maps(void)
{

 int i;

 for (i = 0; i < KEY_CODES; i ++)
	{
		ex_control.key_code_map [i] [0] = -1;
		ex_control.key_code_map [i] [1] = -1;
	}

// these can be remapped later by init.txt settings ([0] values may be remapped to -1)

 ex_control.key_code_map [KEY_CODE_BACKSPACE] [0] = ALLEGRO_KEY_BACKSPACE;
 ex_control.key_code_map [KEY_CODE_BACKSPACE] [1] = SPECIAL_KEY_BACKSPACE;
 ex_control.key_code_map [KEY_CODE_DELETE] [0] = ALLEGRO_KEY_DELETE;
 ex_control.key_code_map [KEY_CODE_DELETE] [1] = SPECIAL_KEY_DELETE;
 ex_control.key_code_map [KEY_CODE_TAB] [0] = ALLEGRO_KEY_TAB;
 ex_control.key_code_map [KEY_CODE_TAB] [1] = SPECIAL_KEY_TAB;
 ex_control.key_code_map [KEY_CODE_ENTER] [0] = ALLEGRO_KEY_ENTER;
 ex_control.key_code_map [KEY_CODE_ENTER] [1] = SPECIAL_KEY_ENTER;
 ex_control.key_code_map [KEY_CODE_INSERT] [0] = ALLEGRO_KEY_INSERT;
 ex_control.key_code_map [KEY_CODE_INSERT] [1] = SPECIAL_KEY_INSERT;
// cursor keys
 ex_control.key_code_map [KEY_CODE_LEFT] [0] = ALLEGRO_KEY_LEFT;
 ex_control.key_code_map [KEY_CODE_LEFT] [1] = SPECIAL_KEY_LEFT;
 ex_control.key_code_map [KEY_CODE_RIGHT] [0] = ALLEGRO_KEY_RIGHT;
 ex_control.key_code_map [KEY_CODE_RIGHT] [1] = SPECIAL_KEY_RIGHT;
 ex_control.key_code_map [KEY_CODE_UP] [0] = ALLEGRO_KEY_UP;
 ex_control.key_code_map [KEY_CODE_UP] [1] = SPECIAL_KEY_UP;
 ex_control.key_code_map [KEY_CODE_DOWN] [0] = ALLEGRO_KEY_DOWN;
 ex_control.key_code_map [KEY_CODE_DOWN] [1] = SPECIAL_KEY_DOWN;
 ex_control.key_code_map [KEY_CODE_PGUP] [0] = ALLEGRO_KEY_PGUP;
 ex_control.key_code_map [KEY_CODE_PGUP] [1] = SPECIAL_KEY_PGUP;
 ex_control.key_code_map [KEY_CODE_PGDN] [0] = ALLEGRO_KEY_PGDN;
 ex_control.key_code_map [KEY_CODE_PGDN] [1] = SPECIAL_KEY_PGDN;
 ex_control.key_code_map [KEY_CODE_HOME] [0] = ALLEGRO_KEY_HOME;
 ex_control.key_code_map [KEY_CODE_HOME] [1] = SPECIAL_KEY_HOME;
// number pad
 ex_control.key_code_map [KEY_CODE_PAD_DELETE] [0] = ALLEGRO_KEY_PAD_DELETE;
 ex_control.key_code_map [KEY_CODE_PAD_DELETE] [1] = SPECIAL_KEY_DELETE;
 ex_control.key_code_map [KEY_CODE_PAD_ENTER] [0] = ALLEGRO_KEY_PAD_ENTER;
 ex_control.key_code_map [KEY_CODE_PAD_ENTER] [1] = SPECIAL_KEY_ENTER;
 ex_control.key_code_map [KEY_CODE_PAD_0] [0] = ALLEGRO_KEY_PAD_0;
 ex_control.key_code_map [KEY_CODE_PAD_0] [1] = SPECIAL_KEY_INSERT;
 ex_control.key_code_map [KEY_CODE_PAD_1] [0] = ALLEGRO_KEY_PAD_1;
 ex_control.key_code_map [KEY_CODE_PAD_1] [1] = SPECIAL_KEY_END;
 ex_control.key_code_map [KEY_CODE_PAD_2] [0] = ALLEGRO_KEY_PAD_2;
 ex_control.key_code_map [KEY_CODE_PAD_2] [1] = SPECIAL_KEY_DOWN;
 ex_control.key_code_map [KEY_CODE_PAD_3] [0] = ALLEGRO_KEY_PAD_3;
 ex_control.key_code_map [KEY_CODE_PAD_3] [1] = SPECIAL_KEY_PGDN;
 ex_control.key_code_map [KEY_CODE_PAD_4] [0] = ALLEGRO_KEY_PAD_4;
 ex_control.key_code_map [KEY_CODE_PAD_4] [1] = SPECIAL_KEY_LEFT;
// ex_control.key_code_map [KEY_CODE_PAD_5] [0] = ALLEGRO_KEY_PAD_5;
// ex_control.key_code_map [KEY_CODE_PAD_5] [1] = SPECIAL_KEY_;
 ex_control.key_code_map [KEY_CODE_PAD_6] [0] = ALLEGRO_KEY_PAD_6;
 ex_control.key_code_map [KEY_CODE_PAD_6] [1] = SPECIAL_KEY_RIGHT;
 ex_control.key_code_map [KEY_CODE_PAD_7] [0] = ALLEGRO_KEY_PAD_7;
 ex_control.key_code_map [KEY_CODE_PAD_7] [1] = SPECIAL_KEY_HOME;
 ex_control.key_code_map [KEY_CODE_PAD_8] [0] = ALLEGRO_KEY_PAD_8;
 ex_control.key_code_map [KEY_CODE_PAD_8] [1] = SPECIAL_KEY_UP;
 ex_control.key_code_map [KEY_CODE_PAD_9] [0] = ALLEGRO_KEY_PAD_9;
 ex_control.key_code_map [KEY_CODE_PAD_9] [1] = SPECIAL_KEY_PGUP;
// mod keys
 ex_control.key_code_map [KEY_CODE_SHIFT_L] [0] = ALLEGRO_KEY_LSHIFT;
 ex_control.key_code_map [KEY_CODE_SHIFT_L] [1] = SPECIAL_KEY_SHIFT;
 ex_control.key_code_map [KEY_CODE_SHIFT_R] [0] = ALLEGRO_KEY_RSHIFT;
 ex_control.key_code_map [KEY_CODE_SHIFT_R] [1] = SPECIAL_KEY_SHIFT;
 ex_control.key_code_map [KEY_CODE_CTRL_L] [0] = ALLEGRO_KEY_LCTRL;
 ex_control.key_code_map [KEY_CODE_CTRL_L] [1] = SPECIAL_KEY_CTRL;
 ex_control.key_code_map [KEY_CODE_CTRL_R] [0] = ALLEGRO_KEY_RCTRL;
 ex_control.key_code_map [KEY_CODE_CTRL_R] [1] = SPECIAL_KEY_CTRL;
// function keys
 ex_control.key_code_map [KEY_CODE_F1] [0] = ALLEGRO_KEY_F1;
 ex_control.key_code_map [KEY_CODE_F1] [1] = SPECIAL_KEY_F1;
 ex_control.key_code_map [KEY_CODE_F2] [0] = ALLEGRO_KEY_F2;
 ex_control.key_code_map [KEY_CODE_F2] [1] = SPECIAL_KEY_F2;
 ex_control.key_code_map [KEY_CODE_F3] [0] = ALLEGRO_KEY_F3;
 ex_control.key_code_map [KEY_CODE_F3] [1] = SPECIAL_KEY_F3;
 ex_control.key_code_map [KEY_CODE_F4] [0] = ALLEGRO_KEY_F4;
 ex_control.key_code_map [KEY_CODE_F4] [1] = SPECIAL_KEY_F4;
 ex_control.key_code_map [KEY_CODE_F5] [0] = ALLEGRO_KEY_F5;
 ex_control.key_code_map [KEY_CODE_F5] [1] = SPECIAL_KEY_F5;
 ex_control.key_code_map [KEY_CODE_F6] [0] = ALLEGRO_KEY_F6;
 ex_control.key_code_map [KEY_CODE_F6] [1] = SPECIAL_KEY_F6;
 ex_control.key_code_map [KEY_CODE_F7] [0] = ALLEGRO_KEY_F7;
 ex_control.key_code_map [KEY_CODE_F7] [1] = SPECIAL_KEY_F7;
 ex_control.key_code_map [KEY_CODE_F8] [0] = ALLEGRO_KEY_F8;
 ex_control.key_code_map [KEY_CODE_F8] [1] = SPECIAL_KEY_F8;
 ex_control.key_code_map [KEY_CODE_F9] [0] = ALLEGRO_KEY_F9;
 ex_control.key_code_map [KEY_CODE_F9] [1] = SPECIAL_KEY_F9;
 ex_control.key_code_map [KEY_CODE_F10] [0] = ALLEGRO_KEY_F10;
 ex_control.key_code_map [KEY_CODE_F10] [1] = SPECIAL_KEY_F10;
// numbers
 ex_control.key_code_map [KEY_CODE_1] [0] = ALLEGRO_KEY_1;
 ex_control.key_code_map [KEY_CODE_1] [1] = SPECIAL_KEY_1;
 ex_control.key_code_map [KEY_CODE_2] [0] = ALLEGRO_KEY_2;
 ex_control.key_code_map [KEY_CODE_2] [1] = SPECIAL_KEY_2;
 ex_control.key_code_map [KEY_CODE_3] [0] = ALLEGRO_KEY_3;
 ex_control.key_code_map [KEY_CODE_3] [1] = SPECIAL_KEY_3;
 ex_control.key_code_map [KEY_CODE_4] [0] = ALLEGRO_KEY_4;
 ex_control.key_code_map [KEY_CODE_4] [1] = SPECIAL_KEY_4;
 ex_control.key_code_map [KEY_CODE_5] [0] = ALLEGRO_KEY_5;
 ex_control.key_code_map [KEY_CODE_5] [1] = SPECIAL_KEY_5;
 ex_control.key_code_map [KEY_CODE_6] [0] = ALLEGRO_KEY_6;
 ex_control.key_code_map [KEY_CODE_6] [1] = SPECIAL_KEY_6;
 ex_control.key_code_map [KEY_CODE_7] [0] = ALLEGRO_KEY_7;
 ex_control.key_code_map [KEY_CODE_7] [1] = SPECIAL_KEY_7;
 ex_control.key_code_map [KEY_CODE_8] [0] = ALLEGRO_KEY_8;
 ex_control.key_code_map [KEY_CODE_8] [1] = SPECIAL_KEY_8;
 ex_control.key_code_map [KEY_CODE_9] [0] = ALLEGRO_KEY_9;
 ex_control.key_code_map [KEY_CODE_9] [1] = SPECIAL_KEY_9;
 ex_control.key_code_map [KEY_CODE_0] [0] = ALLEGRO_KEY_0;
 ex_control.key_code_map [KEY_CODE_0] [1] = SPECIAL_KEY_0;

 ex_control.key_code_map [KEY_CODE_Z] [0] = ALLEGRO_KEY_Z;
 ex_control.key_code_map [KEY_CODE_Z] [1] = SPECIAL_KEY_CONTROL_GROUP_0;
 ex_control.key_code_map [KEY_CODE_X] [0] = ALLEGRO_KEY_X;
 ex_control.key_code_map [KEY_CODE_X] [1] = SPECIAL_KEY_CONTROL_GROUP_1;
 ex_control.key_code_map [KEY_CODE_C] [0] = ALLEGRO_KEY_C;
 ex_control.key_code_map [KEY_CODE_C] [1] = SPECIAL_KEY_CONTROL_GROUP_2;
 ex_control.key_code_map [KEY_CODE_V] [0] = ALLEGRO_KEY_V;
 ex_control.key_code_map [KEY_CODE_V] [1] = SPECIAL_KEY_CONTROL_GROUP_3;
 ex_control.key_code_map [KEY_CODE_B] [0] = ALLEGRO_KEY_B;
 ex_control.key_code_map [KEY_CODE_B] [1] = SPECIAL_KEY_CONTROL_GROUP_4;
 ex_control.key_code_map [KEY_CODE_N] [0] = ALLEGRO_KEY_N;
 ex_control.key_code_map [KEY_CODE_N] [1] = SPECIAL_KEY_CONTROL_GROUP_5;
 ex_control.key_code_map [KEY_CODE_M] [0] = ALLEGRO_KEY_M;
 ex_control.key_code_map [KEY_CODE_M] [1] = SPECIAL_KEY_CONTROL_GROUP_6;


// ex_control.key_code_map [KEY_CODE_] [0] = ALLEGRO_KEY_;
// ex_control.key_code_map [KEY_CODE_] [1] = SPECIAL_KEY_;



}


//int special_key_press [SPECIAL_KEYS];

// This function fills the key_type struct array with information about keys. Called at startup.
void init_key_type(void)
{
 int i;

 for (i = 1; i < ALLEGRO_KEY_MAX; i ++)
 {
  key_type [i].type = KEY_TYPE_OTHER; // default

// letters
  if (i >= ALLEGRO_KEY_A
   && i <= ALLEGRO_KEY_Z)
  {
    key_type [i].type = KEY_TYPE_LETTER;
    key_type [i].unshifted = 'a' + i - ALLEGRO_KEY_A;
    key_type [i].shifted = 'A' + i - ALLEGRO_KEY_A;
    continue;
  }

// numbers
  if (i >= ALLEGRO_KEY_0 && i <= ALLEGRO_KEY_9)
  {
    key_type [i].type = KEY_TYPE_NUMBER;
    key_type [i].unshifted = '0' + i - ALLEGRO_KEY_0;
    switch(i)
    {
     case ALLEGRO_KEY_1: key_type [i].shifted = '!'; break;
     case ALLEGRO_KEY_2: key_type [i].shifted = '@'; break;
     case ALLEGRO_KEY_3: key_type [i].shifted = '#'; break;
     case ALLEGRO_KEY_4: key_type [i].shifted = '$'; break;
     case ALLEGRO_KEY_5: key_type [i].shifted = '%'; break;
     case ALLEGRO_KEY_6: key_type [i].shifted = '^'; break;
     case ALLEGRO_KEY_7: key_type [i].shifted = '&'; break;
     case ALLEGRO_KEY_8: key_type [i].shifted = '*'; break;
     case ALLEGRO_KEY_9: key_type [i].shifted = '('; break;
     case ALLEGRO_KEY_0: key_type [i].shifted = ')'; break;
    }
    continue;
  }

// numpad
  if (i >= ALLEGRO_KEY_PAD_0 && i <= ALLEGRO_KEY_PAD_9)
  {
    key_type [i].type = KEY_TYPE_CURSOR;
    continue;
  }

// other things
  switch(i)
  {
   case ALLEGRO_KEY_MINUS:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '-';
    key_type [i].shifted = '_';
    break;
   case ALLEGRO_KEY_EQUALS:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '=';
    key_type [i].shifted = '+';
    break;
   case ALLEGRO_KEY_OPENBRACE:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '[';
    key_type [i].shifted = '{';
    break;
   case ALLEGRO_KEY_CLOSEBRACE:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = ']';
    key_type [i].shifted = '}';
    break;
   case ALLEGRO_KEY_SEMICOLON:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = ';';
    key_type [i].shifted = ':';
    break;
   case ALLEGRO_KEY_QUOTE:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '\'';
    key_type [i].shifted = '"';
    break;
   case ALLEGRO_KEY_COMMA:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = ',';
    key_type [i].shifted = '<';
    break;
   case ALLEGRO_KEY_FULLSTOP:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '.';
    key_type [i].shifted = '>';
    break;
   case ALLEGRO_KEY_SLASH:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '/';
    key_type [i].shifted = '?';
    break;
   case ALLEGRO_KEY_SPACE:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = ' ';
    key_type [i].shifted = ' ';
    break;
   case ALLEGRO_KEY_TILDE: // don't think my keyboard has this key
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '~';
    key_type [i].shifted = '~';
    break;
   case ALLEGRO_KEY_BACKQUOTE:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '`';
    key_type [i].shifted = '~';
    break;
   case ALLEGRO_KEY_PAD_SLASH:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '/';
    key_type [i].shifted = '/';
    break;
   case ALLEGRO_KEY_PAD_ASTERISK:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '*';
    key_type [i].shifted = '*';
    break;
   case ALLEGRO_KEY_PAD_MINUS:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '-';
    key_type [i].shifted = '-';
    break;
   case ALLEGRO_KEY_PAD_PLUS:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '+';
    key_type [i].shifted = '+';
    break;
   case ALLEGRO_KEY_PAD_EQUALS:
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '=';
    key_type [i].shifted = '=';
    break;
   case ALLEGRO_KEY_BACKSLASH:
   case ALLEGRO_KEY_BACKSLASH2: // what is this?
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '\\';
    key_type [i].shifted = '|';
    break;
   case ALLEGRO_KEY_AT: // what is this? I don't think it's on my keyboard
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '@';
    key_type [i].shifted = '@';
    break;
   case ALLEGRO_KEY_CIRCUMFLEX: // not sure about this one either
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = '^';
    key_type [i].shifted = '^';
    break;
   case ALLEGRO_KEY_COLON2: // or this one
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = ':';
    key_type [i].shifted = ':';
    break;
   case ALLEGRO_KEY_SEMICOLON2: // or this one
    key_type [i].type = KEY_TYPE_SYMBOL;
    key_type [i].unshifted = ';';
    key_type [i].shifted = ';';
    break;
// cursor etc keys
   case ALLEGRO_KEY_BACKSPACE:
   case ALLEGRO_KEY_DELETE:
   case ALLEGRO_KEY_TAB:
   case ALLEGRO_KEY_ENTER:
//   case ALLEGRO_KEY_INSERT: // not sure how to treat this one
   case ALLEGRO_KEY_HOME:
   case ALLEGRO_KEY_END:
   case ALLEGRO_KEY_PGUP:
   case ALLEGRO_KEY_PGDN:
   case ALLEGRO_KEY_LEFT:
   case ALLEGRO_KEY_RIGHT:
   case ALLEGRO_KEY_UP:
   case ALLEGRO_KEY_DOWN:
   case ALLEGRO_KEY_PAD_DELETE:
   case ALLEGRO_KEY_PAD_ENTER:
    key_type[i].type = KEY_TYPE_CURSOR;
    break;
   case ALLEGRO_KEY_LSHIFT:
   case ALLEGRO_KEY_RSHIFT:
   case ALLEGRO_KEY_LCTRL:
   case ALLEGRO_KEY_RCTRL:
   case ALLEGRO_KEY_ALT:
   case ALLEGRO_KEY_ALTGR:
    key_type[i].type = KEY_TYPE_MOD;
    break;

// everything else is KEY_TYPE_OTHER (already set above)

  }


 } // end for loop

}



// The following code deals with text entry into the menu interface (e.g. player names) and also editor text boxes (currently just the text search box)
// Currently, any open text box will accept keyboard input (so if there's more than one, all will accept it). Could fix but not a serious issue.

struct text_input_box_struct
{
 char* str;
 int cursor_pos;
 int max_length;
};

struct text_input_box_struct text_input_box [TEXT_BOXES];

void add_char_to_text_box(int b, char input_char);
//int text_box_cursor_etc(int b, int key_pressed);


// Must call this before accepting input in input box
// b is index in text_input_box array
void start_text_input_box(int b, char* input_str, int max_length)
{

// fpr("\nstart_text_input_box(%i, [%s] %i)", b, input_str, max_length);

 text_input_box[b].str = input_str;
 text_input_box[b].str [0] = '\0';
 text_input_box[b].cursor_pos = 0;
 text_input_box[b].max_length = max_length;

}


// assumes start_text_input_box has been called
// returns 1 if enter pressed, 0 otherwise
int accept_text_box_input(int b)
{

  if (ex_control.special_key_press [SPECIAL_KEY_CTRL] != 0)
    return 0; // ignore this because otherwise it will pick up an f when ctrl-f pressed

  if (ex_control.unichar_input != 0
			&& valid_source_character(ex_control.unichar_input) == 1)
		{
   add_char_to_text_box(b, ex_control.unichar_input);
   editor.cursor_flash = CURSOR_FLASH_MAX;
		}

  if (ex_control.special_key_press [SPECIAL_KEY_BACKSPACE] == BUTTON_JUST_PRESSED)
		{
   if (text_input_box[b].cursor_pos == 0)
    return 0;
   text_input_box[b].cursor_pos --;
   text_input_box[b].str [text_input_box[b].cursor_pos] = '\0';
		}

  if (ex_control.special_key_press [SPECIAL_KEY_ENTER] > 0)
		{
			return 1;
		}

/*
  case ALLEGRO_KEY_BACKSPACE:
   if (text_input_box[b].cursor_pos == 0)
    return 0;
   text_input_box[b].cursor_pos --;
   text_input_box[b].str [text_input_box[b].cursor_pos] = '\0';
   break;
  case ALLEGRO_KEY_ENTER:
  case ALLEGRO_KEY_PAD_ENTER:
   return 1;
*/
/*
 int i;


* need to update this to use unichar

// so check whether another key is being pressed instead:
   for (i = 1; i < ALLEGRO_KEY_MAX; i ++)
   {
    if (ex_control.key_press [i] != BUTTON_JUST_PRESSED
     || key_type [i].type == KEY_TYPE_OTHER
     || key_type [i].type == KEY_TYPE_MOD)
      continue;
    editor.cursor_flash = CURSOR_FLASH_MAX;
    if (key_type [i].type == KEY_TYPE_CURSOR)
    {
     return text_box_cursor_etc(b, i);
    }
// must be a character that can be added to the string
     if (ex_control.key_press [ALLEGRO_KEY_LSHIFT]
      || ex_control.key_press [ALLEGRO_KEY_RSHIFT])
       add_char_to_text_box(b, key_type [i].shifted);
        else
         add_char_to_text_box(b, key_type [i].unshifted);
   }
*/

 return 0;

}


void add_char_to_text_box(int b, char input_char)
{

 int str_length = strlen(text_input_box[b].str);

 if (str_length == text_input_box[b].max_length - 1)
  return;

// For now, just add characters at the end. Deal with allowing cursor movement within the string later.

// simple case: char is to be added at end
// if (text_input_box.cursor_pos == str_length)
 {
  text_input_box[b].str [text_input_box[b].cursor_pos] = input_char;
  text_input_box[b].cursor_pos ++;
  text_input_box[b].str [text_input_box[b].cursor_pos] = '\0';
  return;
 }

/*
// more complicated case: push text to right
 int i = text_input_box.max_length - 1;
 while(i > str_length)
 {
  text_input_box.str [i]
 };*/

}
/*
// returns 1 if enter pressed, 0 otherwise
int text_box_cursor_etc(int b, int key_pressed)
{

 switch(key_pressed)
 {
  case ALLEGRO_KEY_BACKSPACE:
   if (text_input_box[b].cursor_pos == 0)
    return 0;
   text_input_box[b].cursor_pos --;
   text_input_box[b].str [text_input_box[b].cursor_pos] = '\0';
   break;
  case ALLEGRO_KEY_ENTER:
  case ALLEGRO_KEY_PAD_ENTER:
   return 1;
 }

 return 0;

}
*/
#define CLOSEWINDOW_W scaleUI_x(FONT_SQUARE,210)
#define CLOSEWINDOW_H scaleUI_y(FONT_SQUARE,60)

// Displays a close window box and blocks everything else until the user either exits the game or continues
static void close_window_box(void)
{

 int x = settings.option [OPTION_WINDOW_W] / 2;
 int y = settings.option [OPTION_WINDOW_H] / 2;
 int mouse_x, mouse_y, just_pressed;
 ALLEGRO_EVENT ev;

 int xa, ya, xb, yb;

 al_set_target_bitmap(al_get_backbuffer(display));
 al_set_clipping_rectangle(0, 0, settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
 reset_i_buttons();

 pause_music();


 while(TRUE)
 {

// use the system mouse cursor while the close window box is there:
 al_show_mouse_cursor(display); // needs to be inside the loop because it doesn't appear to do anything if called while the pointer is off the display (e.g. if it's pressing the close window button)


//  al_clear_to_color(colours.base [COL_BLUE] [SHADE_LOW]);

  get_ex_control(2, 0); // 2 means that clicking the native close window button will not do anything, as the exit game box is already being displayed
  mouse_x = ex_control.mouse_x_pixels;
  mouse_y = ex_control.mouse_y_pixels;
  just_pressed = (ex_control.mb_press [0] == BUTTON_JUST_PRESSED);


//  al_draw_filled_rectangle(x - CLOSEWINDOW_W, y - CLOSEWINDOW_H, x + CLOSEWINDOW_W, y + CLOSEWINDOW_H, colours.base [COL_RED] [SHADE_LOW]);
//  al_draw_rectangle(x - CLOSEWINDOW_W, y - CLOSEWINDOW_H, x + CLOSEWINDOW_W, y + CLOSEWINDOW_H, colours.base [COL_RED] [SHADE_MED], 1);



//  EXIT


  xa = x - CLOSEWINDOW_W + scaleUI_x(FONT_SQUARE,10);
  ya = y + scaleUI_y(FONT_SQUARE,10);
  xb = x - CLOSEWINDOW_W + scaleUI_x(FONT_SQUARE,140);
  yb = y + scaleUI_y(FONT_SQUARE,30);

 add_menu_button(x - CLOSEWINDOW_W, y - CLOSEWINDOW_H, x + CLOSEWINDOW_W, y + CLOSEWINDOW_H, colours.base_trans [COL_RED] [SHADE_LOW] [TRANS_MED], 5, 3);


  if (mouse_x > xa
   && mouse_x < xb
   && mouse_y > ya
   && mouse_y < yb)
  {

//   al_draw_filled_rectangle(xa, ya, xb, yb, colours.base [COL_RED] [SHADE_HIGH]);
   add_menu_button(xa, ya, xb, yb, colours.base_trans [COL_PURPLE] [SHADE_MAX] [TRANS_FAINT], 5, 3);
   add_menu_string(xa + scaleUI_x(FONT_SQUARE,65), ya + scaleUI_y(FONT_SQUARE,6), &colours.base [COL_GREY] [SHADE_MAX], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, "E[x]it to system");
//   add_menu_string(xa + scaleUI_x(FONT_SQUARE,35), ya + scaleUI_y(FONT_SQUARE,18), &colours.base [COL_GREY] [SHADE_MAX], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, "to system");
//   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], xa + 35, ya + 6, ALLEGRO_ALIGN_CENTRE, "Exit");
   if (just_pressed)
    safe_exit(0);
  }
   else
   {
    add_menu_button(xa, ya, xb, yb, colours.base_trans [COL_RED] [SHADE_HIGH] [TRANS_FAINT], 5, 3);
//    al_draw_filled_rectangle(xa, ya, xb, yb, colours.base [COL_RED] [SHADE_MED]);
    add_menu_string(xa + scaleUI_x(FONT_SQUARE,65), ya + scaleUI_y(FONT_SQUARE,6), &colours.base [COL_GREY] [SHADE_MAX], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, "E[x]it to system");
//    add_menu_string(xa + scaleUI_x(FONT_SQUARE,35), ya + scaleUI_y(FONT_SQUARE,18), &colours.base [COL_GREY] [SHADE_MAX], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, "to system");
//    al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], xa + 35, ya + 6, ALLEGRO_ALIGN_CENTRE, "Exit");
   }

//  Quit


  xa = x - CLOSEWINDOW_W + scaleUI_x(FONT_SQUARE,180);
  ya = y + scaleUI_y(FONT_SQUARE,10);
  xb = x - CLOSEWINDOW_W + scaleUI_x(FONT_SQUARE,270);
  yb = y + scaleUI_y(FONT_SQUARE,30);

  if (mouse_x > xa
   && mouse_x < xb
   && mouse_y > ya
   && mouse_y < yb)
  {

//   al_draw_filled_rectangle(xa, ya, xb, yb, colours.base [COL_RED] [SHADE_HIGH]);
   add_menu_button(xa, ya, xb, yb, colours.base_trans [COL_PURPLE] [SHADE_MAX] [TRANS_FAINT], 5, 3);
   add_menu_string(xa + scaleUI_x(FONT_SQUARE,45), ya + scaleUI_y(FONT_SQUARE,6), &colours.base [COL_GREY] [SHADE_MAX], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, "[Q]uit game");
//   add_menu_string(xa + scaleUI_x(FONT_SQUARE,35), ya + scaleUI_y(FONT_SQUARE,18), &colours.base [COL_GREY] [SHADE_MAX], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, "to menu");
//   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], xa + 35, ya + 6, ALLEGRO_ALIGN_CENTRE, "Exit");
   if (just_pressed)
			{
    game.phase = GAME_PHASE_FORCE_QUIT;
    break;
			}
  }
   else
   {
    add_menu_button(xa, ya, xb, yb, colours.base_trans [COL_RED] [SHADE_HIGH] [TRANS_FAINT], 5, 3);
//    al_draw_filled_rectangle(xa, ya, xb, yb, colours.base [COL_RED] [SHADE_MED]);
    add_menu_string(xa + scaleUI_x(FONT_SQUARE,45), ya + scaleUI_y(FONT_SQUARE,6), &colours.base [COL_GREY] [SHADE_MAX], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, "[Q]uit game");
//    add_menu_string(xa + scaleUI_x(FONT_SQUARE,35), ya + scaleUI_y(FONT_SQUARE,18), &colours.base [COL_GREY] [SHADE_MAX], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, "to menu");
//    al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], xa + 35, ya + 6, ALLEGRO_ALIGN_CENTRE, "Exit");
   }


//  CANCEL


  xa = x + CLOSEWINDOW_W - scaleUI_x(FONT_SQUARE,100);
  ya = y + scaleUI_y(FONT_SQUARE,10);
  xb = x + CLOSEWINDOW_W - scaleUI_x(FONT_SQUARE,20);
  yb = y + scaleUI_y(FONT_SQUARE,30);

  if (mouse_x > xa
   && mouse_x < xb
   && mouse_y > ya
   && mouse_y < yb)
  {
//   al_draw_filled_rectangle(xa, ya, xb, yb, colours.base [COL_RED] [SHADE_HIGH]);
   add_menu_button(xa, ya, xb, yb, colours.base_trans [COL_PURPLE] [SHADE_MAX] [TRANS_FAINT], 5, 3);
   add_menu_string(xa + scaleUI_x(FONT_SQUARE,35), ya + scaleUI_y(FONT_SQUARE,6), &colours.base [COL_GREY] [SHADE_MAX], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, "[C]ancel");
//   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], xa + 35, ya + 6, ALLEGRO_ALIGN_CENTRE, "Cancel");
   if (just_pressed)
    break;
  }
   else
   {
//    al_draw_filled_rectangle(xa, ya, xb, yb, colours.base [COL_RED] [SHADE_MED]);
    add_menu_button(xa, ya, xb, yb, colours.base_trans [COL_RED] [SHADE_HIGH] [TRANS_FAINT], 5, 3);
    add_menu_string(xa + scaleUI_x(FONT_SQUARE,35), ya + scaleUI_y(FONT_SQUARE,6), &colours.base [COL_GREY] [SHADE_MAX], ALLEGRO_ALIGN_CENTRE, FONT_SQUARE, "[C]ancel");
//    al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], xa + 35, ya + 6, ALLEGRO_ALIGN_CENTRE, "Cancel");
   }

  draw_menu_buttons();
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], x, y - scaleUI_y(FONT_SQUARE,40), ALLEGRO_ALIGN_CENTRE, "Exit?");
//  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], x, y - 40, ALLEGRO_ALIGN_CENTRE, "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");
//  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], x, y - 65, ALLEGRO_ALIGN_CENTRE, "the quick brown fox jumps over the lazy dog");
//  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], x, y - scaleUI_y(FONT_SQUARE,25), ALLEGRO_ALIGN_CENTRE, "(any unsaved changes will be lost)");

  if (ex_control.unichar_input == 99 // c
			|| ex_control.unichar_input == 67) // C
			break;
	 if (ex_control.unichar_input == 120 // x
			||	ex_control.unichar_input == 88) // X
			safe_exit(0);
	 if (ex_control.unichar_input == 113 // q
			||	ex_control.unichar_input == 81) // Q
		{
    game.phase = GAME_PHASE_FORCE_QUIT;
    break;
		}


//  if (settings.option [OPTION_SPECIAL_CURSOR])
//		{
//			ex_control.mouse_cursor_type = MOUSE_CURSOR_BASIC;
//   draw_mouse_cursor();
//		}
  al_flip_display();
  al_wait_for_event(event_queue, &ev);
//  al_set_target_bitmap(al_get_backbuffer(display));

 };

 flush_game_event_queues();
 al_hide_mouse_cursor(display); // this can be reset by a few other functions (e.g. when native file dialog or exit game screen is up)
 unpause_music();

}



