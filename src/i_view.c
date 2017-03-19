#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>

#include "m_config.h"

#include "g_header.h"
//#include "i_header.h"

#include "g_misc.h"
#include "m_globvars.h"
#include "m_maths.h"
#include "i_header.h"
#include "i_view.h"
#include "e_inter.h"
#include "t_template.h"



struct view_struct view;

extern ALLEGRO_DISPLAY* display; // in i_display.c
extern ALLEGRO_BITMAP* display_window; // in i_display.c
extern struct fontstruct font [FONTS];

void reset_view_values(int window_w, int window_h);


void init_view_at_startup(int window_w, int window_h)
{
/*
   display_window = al_create_sub_bitmap(al_get_backbuffer(display), 0, 0, window_w, window_h);

   if (display_window == NULL)
   {
    fprintf(stdout, "\nError: i_view.c: init_view_at_startup(): couldn't create display_window sub-bitmap");
    error_call();
   }
*/

// view.zoom_target [ZOOM_MAX_LEVEL + 1] = {10, 16, 26, 36};
#define BASE_WINDOW_H 1000

 view.zoom_target [0] = (10 * window_h) / BASE_WINDOW_H;
 view.zoom_target [1] = (16 * window_h) / BASE_WINDOW_H;
 view.zoom_target [2] = (19 * window_h) / BASE_WINDOW_H;
 view.zoom_target [3] = (24 * window_h) / BASE_WINDOW_H;


/*
 view.zoom_target [0] = (10 * window_h) / BASE_WINDOW_H;
 view.zoom_target [1] = (16 * window_h) / BASE_WINDOW_H;
 view.zoom_target [2] = (22 * window_h) / BASE_WINDOW_H;
 view.zoom_target [3] = (36 * window_h) / BASE_WINDOW_H;
*/
// float zoom_adjust = window_h / 1200;

 reset_view_values(settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);

}


// call this when a new world is started
void initialise_view(int window_w, int window_h)
{

 view.map_w = MAP_DISPLAY_SIZE;
 view.map_h = MAP_DISPLAY_SIZE;

 reset_view_values(window_w, window_h);

// view.paused = 0;
// view.pause_advance_pressed = 0;

 view.w_x_pixel = al_fixtoi(w.fixed_size.x); // width of window in game pixels (may be affected by zoom)
 view.w_y_pixel = al_fixtoi(w.fixed_size.y);
 view.camera_x_min = BLOCK_SIZE_FIXED * 2;
 view.camera_y_min = BLOCK_SIZE_FIXED * 2;
 view.camera_x_max = w.fixed_size.x - (BLOCK_SIZE_FIXED * 2);
 view.camera_y_max = w.fixed_size.y - (BLOCK_SIZE_FIXED * 2);
// view.zoom = 1;
 view.zoom_level = 1;
 view.zoom_int = 16;
	view.zoom = view.zoom_int * 0.03;
// if any of these zoom calculations change, see also zoom_target stuff in i_input.c

 view.map_visible = 1;
 view.map_proportion_x = al_fixdiv(al_itofix(view.map_w), w.fixed_size.x);
 view.map_proportion_y = al_fixdiv(al_itofix(view.map_h), w.fixed_size.y);

// as are these proc data window values:
// view.focus_proc = NULL;
// view.data_window_x = 640;
// view.data_window_y = 50;

 view.mouse_on_build_button = -1;
 view.mouse_on_build_queue_button = -1;
 view.mouse_on_build_button_timestamp = 0;

 view.data_box_open = 1;

 view.following = 0;

 int i;

 for (i = 0; i < UNDER_ATTACK_MARKERS; i ++)
	{
		view.under_attack_marker [i].time_placed_world = 0;
	}
	view.under_attack_marker_last_time = 0;

 view.screen_shake_time = 0;

}


// call this anytime the display window needs to be resized.
void resize_display_window(int window_w, int window_h)
{

   reset_view_values(window_w, window_h);
//   change_edit_panel_width();
//   reset_template_x_values();

}

void reset_view_values(int window_w, int window_h)
{

   view.just_resized = 1;
   view.window_x_unzoomed = window_w;
   view.window_y_unzoomed = window_h;
   view.window_x_zoomed = window_w / view.zoom;
   view.window_y_zoomed = window_h / view.zoom;
   view.centre_x_zoomed = al_itofix(view.window_x_zoomed / 2);
   view.centre_y_zoomed = al_itofix(view.window_y_zoomed / 2);

   view.map_x = window_w - view.map_w - 40;
   view.map_y = window_h - view.map_h - 40;

// horrible magic numbers that end up putting the close button in the right position:
   view.data_box_close_button_x1 = window_w - 10 - scaleUI_x(FONT_SQUARE,30);
//   view.data_box_close_button_y1 = scaleUI_y(FONT_BASIC,114);
   view.data_box_close_button_y1 = scaleUI_y(FONT_SQUARE,70) + 16 + 3;

 		if (view.data_box_close_button_y1 < inter.mode_buttons_y1 + MODE_BUTTON_SIZE + 6)
	 		view.data_box_close_button_y1 = inter.mode_buttons_y1 + MODE_BUTTON_SIZE + 6;


   view.data_box_close_button_x2 = window_w - 10 - scaleUI_x(FONT_SQUARE,3);
   view.data_box_close_button_y2 = view.data_box_close_button_y1 + scaleUI_x(FONT_SQUARE,16) + 6;// - 6;//scaleUI_y(FONT_BASIC,136);

}

