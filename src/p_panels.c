
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"
#include "i_header.h"

#include "g_misc.h"

#include "e_slider.h"
#include "e_header.h"
#include "e_editor.h"
#include "e_inter.h"
#include "e_help.h"
#include "e_log.h"

#include "i_input.h"
#include "i_view.h"
#include "i_display.h"
#include "i_buttons.h"
#include "t_template.h"
#include "m_input.h"
#include "f_load.h"
#include "f_game.h"
#include "x_sound.h"

#include "s_mission.h"

#include "p_panels.h"
#include "p_init.h"

#include "d_design.h"


//#include "s_setup.h"

extern struct fontstruct font [FONTS];
extern ALLEGRO_DISPLAY* display;

// these queues are declared in g_game.c. They're externed here so that they can be flushed when the editor does something slow.
extern ALLEGRO_EVENT_QUEUE* event_queue; // these queues are initialised in main.c
extern ALLEGRO_EVENT_QUEUE* fps_queue;

extern struct game_struct game; // in g_game.c
extern struct view_struct view;

char mstring [MENU_STRING_LENGTH];



struct panel_struct panel [PANELS];

void subpanel_input(int pan, int subpan);
void mode_button_input(void);
void mode_button(int mode_pressed);
void resize_panel(int p, int new_w, int new_h);
void reset_panel_positions(void);
void reset_mode_buttons(void);
void system_panel_button(int el_index);

// Opens a panel and sets basic parameters
void open_panel(int pan)
{
// don't reset panel width.
 panel[pan].open = 1;

 reset_panel_positions();

}

void close_panel(int pan, int set_panel_restore) // set_panel_restore will be zero if all panels are being closed at once
{

 if (panel[pan].open)
	{

		switch(pan)
		{
 		case PANEL_SYSMENU:
 	 	panel[PANEL_SYSMENU].element [FPE_SYSMENU_CONFIRM_QUIT].open = 0;
 	 	break;

		}

	 panel[pan].open = 0;

	 reset_panel_positions();

	 if (set_panel_restore)
		{
			if (!panel[PANEL_SYSMENU].open
			 && !panel[PANEL_EDITOR].open
			 && !panel[PANEL_DESIGN].open
			 && !panel[PANEL_TEMPLATE].open
			 && !panel[PANEL_BCODE].open)
		 {
// closed all panels. Set panel_restore to just the panel that was just closed:
    int i;
    for (i = 1; i < PANEL_BCODE+1; i ++)
			 {
 				inter.panel_restore [i] = 0;
			 }
			 inter.panel_restore [pan] = 1;
// also block mode button area scrolling:
    inter.block_mode_button_area_scrolling	= 1;
		 }
		}
		 else
		 {
    inter.block_mode_button_area_scrolling	= 1;
		 }

	}

}


void reset_panel_positions(void)
{

 int i;
 int x = inter.display_w;
 int any_open = 0;

 i = 1; //PANELS - 1;

 while (i < PANEL_LOG) // note: doesn't set panel 0
	{
		if (panel[i].open == 1) // panel [0] must always be open
		{
			panel[i].x1 = x - panel[i].w;
			panel[i].x2 = x;
			x = panel[i].x1;// - 1;
			any_open = 1;
//			fpr("\n panel %i x2 %i", i, panel[i].x2);
		}
		i ++;
	}; // end while

 panel[0].x1 = 0;
 panel[0].x2 = x;
 panel[0].w = panel[0].x2;

 if (any_open == 1)
	{
		panel[PANEL_LOG].open = 1;
  panel[PANEL_LOG].x1 = x;
  panel[PANEL_LOG].w = inter.display_w - x;
  log_resized();
  resize_panel(PANEL_LOG, panel[PANEL_LOG].w, panel[PANEL_LOG].h);
  for (i = 0; i < PANEL_LOG; i++)
		{
			if (panel[i].open == 1)
    resize_panel(i, panel[i].w, panel[i].h);
		}
	}
	 else
			panel[PANEL_LOG].open = 0;

 reset_mode_buttons();
 resize_display_window(panel[PANEL_MAIN].w, panel[PANEL_MAIN].h);

}


void resize_panel(int p, int new_w, int new_h)
{

	panel[p].w = new_w;
	panel[p].h = new_h;

//	reset_panel_positions();
 set_subpanel_positions(p);

 if (p == PANEL_EDITOR)
	 change_edit_panel_width();



}


// This sets up the positions of the mode buttons depending on status and position of each panel:
// assumes that if a panel is open it is at least wide enough to hold all necessary mode buttons (so I might need to make them a bit smaller)
void reset_mode_buttons(void)
{

 if (panel[PANEL_DESIGN].open
		|| panel[PANEL_EDITOR].open
		|| panel[PANEL_SYSMENU].open
		|| panel[PANEL_TEMPLATE].open
		|| panel[PANEL_BCODE].open
		|| game.phase == GAME_PHASE_MENU)
	{
		inter.mode_buttons_x1 = inter.display_w - scaleUI_x(FONT_SQUARE,21);
		inter.mode_buttons_y1 = MODE_BUTTON_Y;
	}
	 else
		{
//			inter.mode_buttons_x1 = inter.display_w - 166;
//			inter.mode_buttons_x1 = inter.display_w - scaleUI_x(FONT_SQUARE,156);
			inter.mode_buttons_x1 = inter.display_w - scaleUI_x(FONT_SQUARE,141);
			inter.mode_buttons_y1 = scaleUI_y(FONT_SQUARE,70) + 15;
		}

/*
//	int pan;
//	int buttons_in_current_panel = 0;
	int current_button = MODE_BUTTON_SYSTEM;

#define MODE_BUTTON_TOTAL_SIZE (MODE_BUTTON_SIZE+MODE_BUTTON_SPACING)
// pan = PANEL_SYSTEM;

 for (current_button = 0; current_button < MODE_BUTTONS; current_button ++)
	{
		 inter.mode_button_x [current_button] = inter.display_w - (MODE_BUTTON_SIZE + MODE_BUTTON_SPACING) * (current_button + 1);
	}
*/
}


void run_panels(void)
{
/*
 if (control.key_press [KEY_S] == BUTTON_JUST_PRESSED)
	{
		int i, j, k;
		for (i = 0; i < PANELS; i ++)
		{
			if (panel[i].open == 1)
			{
//				fpr("\nPanel %i: ", i);
				for (j = 0; j < SUBPANELS; j ++)
				{
					if (panel[i].subpanel[j].open == 1)
					{
//						fpr("\n sp %i (%i,%i to %i,%i) ", j, panel[i].subpanel[j].x1, panel[i].subpanel[j].y1, panel[i].subpanel[j].x2, panel[i].subpanel[j].y2);
						for (k = 0; k < ELEMENTS; k ++)
						{
//							if (panel[i].element[k].open == 1
//								&& panel[i].element[k].subpanel == j)
//							{
//						  fpr("\n  el %i (subpan %i) (%i,%i to %i,%i) ", k, j, panel[i].element[k].x1, panel[i].element[k].y1, panel[i].element[k].x2, panel[i].element[k].y2);

//							}
						}
					}
				}
										for (k = 0; k < ELEMENTS; k ++)
						{
//							if (panel[i].element[k].open == 1
//								&& panel[i].element[k].subpanel == j)
							{
//						  fpr("\n  el %i exists %i open %i (subpan %i) (%i,%i to %i,%i) ", k, panel[i].element[k].exists, panel[i].element[k].open, panel[i].element[k].subpanel, panel[i].element[k].x1, panel[i].element[k].y1, panel[i].element[k].x2, panel[i].element[k].y2);

							}
						}

			}
		}
	}
*/
	if (control.mouse_status == MOUSE_STATUS_OUTSIDE)
		return; // no input when mouse outside?


 if (control.mouse_y_screen_pixels < inter.mode_buttons_y1 + (MODE_BUTTON_SIZE + 3)
		&&	control.mouse_y_screen_pixels > inter.mode_buttons_y1 - 5
		&& control.mouse_x_screen_pixels >= inter.mode_buttons_x1 - (MODE_BUTTON_SIZE + MODE_BUTTON_SPACING) * MODE_BUTTONS) //inter.display_w - ((MODE_BUTTON_SIZE + MODE_BUTTON_SPACING) * MODE_BUTTONS))
	{
		mode_button_input();
	}

#define PANEL_MINIMUM_W 400
#define PANEL_MAXIMUM_W scaleUI_x(FONT_BASIC, 900)

 if (control.mouse_drag == MOUSE_DRAG_PANEL_RESIZE)
	{
		panel[control.mouse_drag_panel].x1 = control.mouse_x_screen_pixels;
		panel[control.mouse_drag_panel].w = panel[control.mouse_drag_panel].x2 - panel[control.mouse_drag_panel].x1;
		if (panel[control.mouse_drag_panel].w < PANEL_MINIMUM_W)
		 panel[control.mouse_drag_panel].w = PANEL_MINIMUM_W;
		if (panel[control.mouse_drag_panel].w > PANEL_MAXIMUM_W)
		 panel[control.mouse_drag_panel].w = PANEL_MAXIMUM_W;
		panel[control.mouse_drag_panel].x1 = panel[control.mouse_drag_panel].x2 - panel[control.mouse_drag_panel].w;
		reset_panel_positions();

		if (control.mbutton_press [0] <= 0)
		{
		 control.mouse_drag = MOUSE_DRAG_NONE;
		 return;
		}

		ex_control.mouse_cursor_type = MOUSE_CURSOR_RESIZE;

	}

 if (control.mouse_drag == MOUSE_DRAG_DESIGN_MEMBER)
	{
		if (!mouse_drag_design_member(control.mouse_x_screen_pixels - panel[PANEL_DESIGN].x1 - panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1 - panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].x1,
																											control.mouse_y_screen_pixels - panel[PANEL_DESIGN].y1 - panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y1 - panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].y1,
																											ex_control.special_key_press [SPECIAL_KEY_CTRL] > 0)
				|| control.mbutton_press [0] <= 0)
		{
		 control.mouse_drag = MOUSE_DRAG_NONE;
		 return;
		}
	}

 if (control.mouse_drag == MOUSE_DRAG_DESIGN_OBJECT)
	{
		if (!mouse_drag_design_object(control.mouse_x_screen_pixels - panel[PANEL_DESIGN].x1 - panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1 - panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].x1,
																											control.mouse_y_screen_pixels - panel[PANEL_DESIGN].y1 - panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y1 - panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].y1,
																											ex_control.special_key_press [SPECIAL_KEY_CTRL] > 0)
				|| control.mbutton_press [0] <= 0)
		{
		 control.mouse_drag = MOUSE_DRAG_NONE;
		 return;
		}
	}


	int pan = -1, subpan = -1;

 pan = control.mouse_panel;
/*
#ifdef SANITY_CHECK
 if (pan == -1
		|| pan == PANELS)
	{
  fprintf(stdout, "\nError: p_panel.c: panel_input(): mouse available but not in any panel? (mouse at %i,%i, panel %i)", control.mouse_x_screen_pixels, control.mouse_y_screen_pixels, pan);
  error_call();
	}
#endif*/

 if (pan == -1)
		return;

// now we know that the mouse is in panel [pan]
// check each subpanel of pan:
 for (subpan = 0; subpan < SUBPANELS; subpan++)
	{
/*		fpr("\n pan %i:%i:%i %i,%i %i,%i",
								pan,
								subpan,
								panel[pan].subpanel[subpan].open,
								panel[pan].subpanel[subpan].x1,
								panel[pan].subpanel[subpan].y1,
								panel[pan].subpanel[subpan].x2,
								panel[pan].subpanel[subpan].y2);*/
		if (panel[pan].subpanel[subpan].open
		 && control.mouse_x_screen_pixels >= panel[pan].x1 + panel[pan].subpanel[subpan].x1
		 && control.mouse_x_screen_pixels <= panel[pan].x1 + panel[pan].subpanel[subpan].x2
		 && control.mouse_y_screen_pixels >= panel[pan].y1 + panel[pan].subpanel[subpan].y1
		 && control.mouse_y_screen_pixels <= panel[pan].y1 + panel[pan].subpanel[subpan].y2)
		{
/*			fpr("\n pan %i:%i:%i %i,%i %i,%i",
								pan,
								subpan,
								panel[pan].subpanel[subpan].open,
								panel[pan].subpanel[subpan].x1,
								panel[pan].subpanel[subpan].y1,
								panel[pan].subpanel[subpan].x2,
								panel[pan].subpanel[subpan].y2);*/
			subpanel_input(pan, subpan);
			break;
		}
	}

// now do keyboard input?

}


void subpanel_input(int pan, int subpan)
{
// int mouse_x_in_subpanel = control.mouse_x_screen_pixels - panel[pan].subpanel[subpan].x1;
// int mouse_y_in_subpanel = control.mouse_y_screen_pixels - panel[pan].subpanel[subpan].y1;
 int mouse_x_in_subpanel = control.mouse_x_screen_pixels - panel[pan].x1 - panel[pan].subpanel[subpan].x1;
 int mouse_y_in_subpanel = control.mouse_y_screen_pixels - panel[pan].y1 - panel[pan].subpanel[subpan].y1;
 int el;


 for (el = 0; el < ELEMENTS; el++)
	{
		if (panel[pan].element[el].open
		 && panel[pan].element[el].subpanel == subpan
		 && mouse_x_in_subpanel >= panel[pan].element[el].x1
		 && mouse_y_in_subpanel >= panel[pan].element[el].y1
		 && mouse_x_in_subpanel < panel[pan].element[el].x2
		 && mouse_y_in_subpanel < panel[pan].element[el].y2)
		{
//			fpr("\nIn pan %i subpan %i el %i", pan, subpan, el);
			switch(panel[pan].element[el].type)
			{
			 case PE_TYPE_SCROLLBAR_EL_V_PIXEL:
				case PE_TYPE_SCROLLBAR_EL_H_PIXEL:
			 case PE_TYPE_SCROLLBAR_EL_V_CHAR:
				case PE_TYPE_SCROLLBAR_EL_H_CHAR:
					if (control.mouse_drag == MOUSE_DRAG_NONE)
					 run_slider(panel[pan].element[el].value [1]);
				 break;
				case PE_TYPE_BUTTON:
					if (panel[pan].element[el].last_highlight < inter.running_time - 1)
					{
						panel[pan].element[el].highlight = inter.running_time;
					}
					panel[pan].element[el].last_highlight = inter.running_time;
					control.panel_element_highlighted = el;
					control.panel_element_highlighted_time = inter.running_time;
					if (control.mouse_drag == MOUSE_DRAG_NONE
						&& control.mbutton_press [0] == BUTTON_JUST_PRESSED)
					{
						switch(pan)
						{
						 case PANEL_TEMPLATE:
							 template_panel_button(el);
							 break;
						 case PANEL_DESIGN:
						 	design_panel_button(el);
							 break;
							case PANEL_SYSMENU:
								system_panel_button(el);
								break;
							case PANEL_BCODE:
								break;
//							case PANEL_EDITOR:
//								editor_panel_button(el);
//								break;
						}
					}
					break;
				case PE_TYPE_PANEL_RESIZE:
					ex_control.mouse_cursor_type = MOUSE_CURSOR_RESIZE;
					if (panel[pan].element[el].last_highlight < inter.running_time - 1)
					{
						panel[pan].element[el].highlight = inter.running_time;
					}
					panel[pan].element[el].last_highlight = inter.running_time;
					if (control.mouse_drag == MOUSE_DRAG_NONE
						&& control.mbutton_press [0] == BUTTON_JUST_PRESSED)
					{
						control.mouse_drag = MOUSE_DRAG_PANEL_RESIZE;
						control.mouse_drag_panel = pan;
						control.mouse_drag_element = el;
					}
					break;
				case PE_TYPE_DESIGN_WINDOW:
					design_window_input(mouse_x_in_subpanel - panel[pan].element[el].x1,
																									mouse_y_in_subpanel - panel[pan].element[el].y1); // x and y may still need to be adjusted for window position and zoom
					break;
//				case PE_TYPE_WINDOW:

			}
		}
	}

}



// Call this function only if mouse is within bounding box surrounding all mode buttons
void mode_button_input(void)
{

 int i;

/* for (i = 0; i < MODE_BUTTONS; i ++)
 {
    inter.mode_button_highlight [i] = 0;
 }*/

   inter.mode_button_highlight = -1;

/*
These things are checked before this function is called:

  if (ex_control.mouse_x_pixels >= inter.mode_button_x [0]
   && ex_control.mouse_x_pixels <= inter.mode_button_x [MODE_BUTTONS - 1] + MODE_BUTTON_SIZE
   && ex_control.mouse_y_pixels >= MODE_BUTTON_Y
   && ex_control.mouse_y_pixels <= MODE_BUTTON_Y + MODE_BUTTON_SIZE)*/

  {
   for (i = 0; i < MODE_BUTTONS; i ++)
   {
// don't need to check for y values because that should have been checked before this function was called.
    if (ex_control.mouse_x_pixels >= inter.mode_buttons_x1 - (i * (MODE_BUTTON_SIZE + MODE_BUTTON_SPACING)) - 1
     && ex_control.mouse_x_pixels <= inter.mode_buttons_x1 - (i * (MODE_BUTTON_SIZE + MODE_BUTTON_SPACING)) + MODE_BUTTON_SIZE + 1)
    {
     inter.mode_button_highlight = i;
     inter.mode_button_highlight_time = inter.running_time;
     if (ex_control.mb_press [0] == BUTTON_JUST_PRESSED)
      mode_button(i);
     if (ex_control.mb_press [1] == BUTTON_JUST_PRESSED)
					{
						switch(i)
						{
/*
						 case MODE_BUTTON_EDITOR:
						 	print_help(HELP_MODE_BUTTON_EDITOR); break;
						 case MODE_BUTTON_TEMPLATES:
						 	print_help(HELP_MODE_BUTTON_TEMPLATES); break;
						 case MODE_BUTTON_PROGRAMS:
						 	print_help(HELP_MODE_BUTTON_PROGRAMS); break;
						 case MODE_BUTTON_SYSMENU:
						 	print_help(HELP_MODE_BUTTON_SYSMENU); break;
						 case MODE_BUTTON_CLOSE:
						 	print_help(HELP_MODE_BUTTON_CLOSE); break;*/
						}
					}
     return; // can return because we know the mouse isn't over any other mode button, or over the edge of the panel (for the code below)
    }
   }
  }

/*			fprintf(stdout, "\nZ(%i,%i,%i,%i)", ex_control.mb_press [0] == BUTTON_JUST_PRESSED,
												settings.edit_window != EDIT_WINDOW_CLOSED,
												ex_control.mouse_x_pixels,settings.editor_x_split);*/
/*
		ex_control.panel_drag_ready = 0;

  if (settings.edit_window != EDIT_WINDOW_CLOSED
			&& ex_control.mouse_x_pixels > settings.editor_x_split
			&& ex_control.mouse_x_pixels < settings.editor_x_split + 12)
		{
			ex_control.panel_drag_ready = 1;
			if (ex_control.mb_press [0] == BUTTON_JUST_PRESSED)
			 ex_control.mouse_dragging_panel = 1;
		}

		if (ex_control.mouse_dragging_panel != 0)
		{
			int new_window_columns = (settings.option [OPTION_WINDOW_W] - ex_control.mouse_x_pixels - 30) / editor.text_width + 1;
//			int new_main_window_size = settings.option [OPTION_WINDOW_W] - (settings.edit_window_columns * editor.text_width) - 30;
			if (ex_control.mb_press [0] >= BUTTON_JUST_PRESSED)
			{
	   if (new_window_columns < 68)
					new_window_columns = 68; // 70 is just enough to give space for the template menu
	   if (new_window_columns > 160)
					new_window_columns = 160;
				if (settings.option [OPTION_WINDOW_W] - (new_window_columns * editor.text_width)	< 540)
					new_window_columns = (settings.option [OPTION_WINDOW_W] - 540) / editor.text_width;
				if (settings.edit_window_columns != new_window_columns)
					view.just_resized = 1;
	   settings.edit_window_columns = new_window_columns;
    settings.editor_x_split = settings.option [OPTION_WINDOW_W] - (settings.edit_window_columns * editor.text_width) - 30;
    editor.panel_x = settings.editor_x_split;
    editor.panel_w = settings.option [OPTION_WINDOW_W] - settings.editor_x_split;
    resize_display_window(settings.editor_x_split, settings.option [OPTION_WINDOW_H]);
    ex_control.mouse_on_display = 0;
//    change_edit_panel_width();
			}
			 else
				{
					ex_control.mouse_dragging_panel = 0;
				}
		}
*/
}

// call this when one of the mode buttons is pressed
// see also close_any_edit_window() in g_game.c
void mode_button(int mode_pressed)
{

// if (inter.mode_button_available [mode_pressed] == 0)
//  return;

#define MODE_BUTTON_SAMPLE SAMPLE_BLIP4
#define MODE_BUTTON_TONE_OPEN TONE_3C
#define MODE_BUTTON_TONE_CLOSE TONE_2G

 switch(mode_pressed)
 {
	 case MODE_BUTTON_SYSTEM:
	 	if (panel[PANEL_SYSMENU].open)
			{
				close_panel(PANEL_SYSMENU, 1);
    play_interface_sound(MODE_BUTTON_SAMPLE, MODE_BUTTON_TONE_CLOSE);
// 	 	panel[PANEL_SYSMENU].element [FPE_SYSMENU_CONFIRM_QUIT].open = 0; this is now done by close_panel
			}
			  else
					{
						open_panel(PANEL_SYSMENU);
      play_interface_sound(MODE_BUTTON_SAMPLE, MODE_BUTTON_TONE_OPEN);
					}
			break;
	 case MODE_BUTTON_EDITOR:
	 	if (panel[PANEL_EDITOR].open)
			{
    play_interface_sound(MODE_BUTTON_SAMPLE, MODE_BUTTON_TONE_CLOSE);
				close_panel(PANEL_EDITOR, 1);
			}
			  else
					{
      play_interface_sound(MODE_BUTTON_SAMPLE, MODE_BUTTON_TONE_OPEN);
						open_panel(PANEL_EDITOR);
					}
			break;
	 case MODE_BUTTON_DESIGN:
	 	if (panel[PANEL_DESIGN].open)
			{
    play_interface_sound(MODE_BUTTON_SAMPLE, MODE_BUTTON_TONE_CLOSE);
				close_panel(PANEL_DESIGN, 1);
			}
			  else
					{
      play_interface_sound(MODE_BUTTON_SAMPLE, MODE_BUTTON_TONE_OPEN);
						open_panel(PANEL_DESIGN);
					}
			break;
	 case MODE_BUTTON_TEMPLATES:
	 	if (panel[PANEL_TEMPLATE].open)
			{
    play_interface_sound(MODE_BUTTON_SAMPLE, MODE_BUTTON_TONE_CLOSE);
				close_panel(PANEL_TEMPLATE, 1);
			}
			  else
					{
      play_interface_sound(MODE_BUTTON_SAMPLE, MODE_BUTTON_TONE_OPEN);
						open_panel(PANEL_TEMPLATE);
					}
			break;
	 case MODE_BUTTON_BCODE:
	 	if (panel[PANEL_BCODE].open)
			{
    play_interface_sound(MODE_BUTTON_SAMPLE, MODE_BUTTON_TONE_CLOSE);
				close_panel(PANEL_BCODE, 1);
			}
			  else
					{
      play_interface_sound(MODE_BUTTON_SAMPLE, MODE_BUTTON_TONE_OPEN);
						open_panel(PANEL_BCODE);
					}
			break;
	 case MODE_BUTTON_CLOSE:
//	 	if (inter.mode_buttons_maximised)
   if (panel[PANEL_DESIGN].open
				|| panel[PANEL_EDITOR].open
				|| panel[PANEL_TEMPLATE].open
				|| panel[PANEL_BCODE].open
				|| panel[PANEL_SYSMENU].open)
			{
    play_interface_sound(MODE_BUTTON_SAMPLE, MODE_BUTTON_TONE_CLOSE);
	 	 close_all_panels();
			}
				else
				{
      play_interface_sound(MODE_BUTTON_SAMPLE, MODE_BUTTON_TONE_OPEN);
      int opened_any = 0;
      int i;
      for (i = 1; i < PANEL_BCODE+1; i ++) // note i starts at 1
						{
							if (inter.panel_restore [i])
							{
  						open_panel(i);
  						opened_any = 1;
							}
						}
					 if (!opened_any)
							open_panel(PANEL_SYSMENU);
				}
//			 else
//					inter.mode_buttons_maximised = 1;
			break;
//		case MODE_BUTTON_MIN_MAX:
//			inter.mode_buttons_maximised ^= 1;
//			break;

 }

}

void close_all_panels(void)
{
// actually doesn't close main or log panels

   int i;

	 	for (i = 1; i < PANEL_BCODE+1; i ++) // note i = 1
			{
				if (panel[i].open)
				{
					inter.panel_restore [i] = 1;
					close_panel(i, 0);
				}
				 else
  				inter.panel_restore [i] = 0;
			}


}


//* TO DO before final release:

//	- add some easier levels at the start?
//	- swap build command buttons
//	- add some text after beating first two blue missions that recommends designing new procs.


void system_panel_button(int el_index)
{
	switch(el_index)
	{
	 case FPE_SYSMENU_PAUSE:
		 if (game.pause_soft == 0)
			{
		 	game.pause_soft = 1;
 		 play_interface_sound(SAMPLE_BLIP1, TONE_2E);
			}
		   else
					{
			   game.pause_soft = 0;
		    play_interface_sound(SAMPLE_BLIP1, TONE_2D);
					}
		 break;
	 case FPE_SYSMENU_QUIT:
 	 panel[PANEL_SYSMENU].element [FPE_SYSMENU_CONFIRM_QUIT].open ^= 1;
		 play_interface_sound(SAMPLE_BLIP1, TONE_2CS);
		 break;
		case FPE_SYSMENU_CONFIRM_QUIT:
			game.phase = GAME_PHASE_FORCE_QUIT; // force the main game loop in g_game.c, or the story loop in h_story.c, to exit
		 play_interface_sound(SAMPLE_BLIP1, TONE_2DS);
			break;
	}
}

