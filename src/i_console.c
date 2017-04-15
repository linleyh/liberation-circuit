#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "g_method.h"
#include "e_slider.h"
#include "m_globvars.h"
#include "m_input.h"
#include "m_maths.h"
#include "i_console.h"
#include "i_header.h"
#include "i_buttons.h"
#include "i_display.h"
#include "g_command.h"
#include "g_shapes.h"

#include "g_misc.h"
#include "p_panels.h"

#include "x_sound.h"

extern struct control_struct control;
extern struct game_struct game;
extern struct command_struct command;
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];

extern struct fontstruct font [FONTS];
extern struct view_struct view;

//Check target_destroyed - is it working properly?




struct consolestruct console [CONSOLES];



void console_newline(int console_index, int print_colour);

// This function is called in initialisation just before the main game loop starts.
// It's also called during game loading (to zero out all of the console data)
void init_consoles(void)
{

 int i;

 for (i = 0; i < CONSOLES; i ++)
 {
  clear_console(i);
 }



// general console
 console[CONSOLE_GENERAL].h_lines = 6;
 console[CONSOLE_GENERAL].h_pixels = console[CONSOLE_GENERAL].h_lines * CONSOLE_LINE_HEIGHT;
 console[CONSOLE_GENERAL].w_letters = 66;
 console[CONSOLE_GENERAL].w_pixels = scaleUI_x(FONT_SQUARE, (console[CONSOLE_GENERAL].w_letters * 7 + 3)) + scaleUI_x(FONT_BASIC, 29); // +25 is to leave space for the source index on the left
/* fpr("\n base %i scaled %i factor %f test %f",
					console[CONSOLE_GENERAL].w_letters * 7 + 32,
					console[CONSOLE_GENERAL].w_pixels,
					font[FONT_SQUARE].font_scale_x,
					scaleUI_x(FONT_SQUARE,100));*/
 console[CONSOLE_GENERAL].x = 40;
 console[CONSOLE_GENERAL].y = settings.option [OPTION_WINDOW_H] - console[CONSOLE_GENERAL].h_pixels - 40;

// system console
 console[CONSOLE_SYSTEM].h_lines = 8;
 console[CONSOLE_SYSTEM].h_pixels = console[CONSOLE_SYSTEM].h_lines * CONSOLE_LINE_HEIGHT;
 console[CONSOLE_SYSTEM].w_letters = SYSTEM_CONSOLE_WIDTH_LETTERS;
 console[CONSOLE_SYSTEM].w_pixels = scaleUI_x(FONT_SQUARE, (console[CONSOLE_SYSTEM].w_letters * 7 + 3)) + scaleUI_x(FONT_BASIC, 29); // ???
 console[CONSOLE_SYSTEM].x = 40;
 console[CONSOLE_SYSTEM].y = 40;



// fprintf(stdout, "\nInit console scrollbar: clines %i min %i h_lines %i", CLINES, CLINES - console[i].h_lines, console[i].h_lines);
//void init_slider(struct slider_struct* sl, int* value_pointer, int dir, int value_min, int value_max, int total_length, int button_increment, int track_increment, int slider_represents_size, int x, int y, int thickness)

// place_proc_box(640, 50, NULL);
// proc_box.button_highlight = 0;
// proc_box.maximised = 1;

}

void setup_consoles(void)
{

 // not currently used

}

void clear_console(int console_index)
{


  sancheck(console_index, 0, CONSOLES, "clear_console:console_index");

  console[console_index].cpos = 0;
  console[console_index].current_line_length = 0;
  console[console_index].line_highlight = -1;
  console[console_index].line_highlight_time = 0;
  console[console_index].source_index = -1;
  console[console_index].time_written = 0;

  int i;

  for (i = 0; i < CLINES; i++)
		{
			console[console_index].cline[i].used = 0;
			console[console_index].cline[i].colour = COL_GREY;
			console[console_index].cline[i].text[0] = '\0';
			console[console_index].cline[i].source_index = -1;
			console[console_index].cline[i].time_written = 0;
		}

}


void run_consoles(void)
{

 int i;

 for (i = 0; i < CONSOLES; i ++)
 {
/*
  if (console[i].open != CONSOLE_CLOSED
   && (console[i].style == CONSOLE_STYLE_BASIC
    || console[i].style == CONSOLE_STYLE_BASIC_UP))
   run_slider(&console[i].scrollbar_v, 0, 0);*/
 }

// console_tick_rollover();

}


char *build_fail_message [4] =
{
	"collision",
	"out of range",
	"too near data well",
	"out of bounds"

};

void display_consoles_and_buttons(void)
{

	int i;

#ifndef RECORDING_VIDEO_2

	int line_index;
	int c;

//	for (c = 0; c < CONSOLES; c ++)
	{

		c = CONSOLE_GENERAL; // currently only the general console (bottom left) is displayed fully.

//	 al_draw_filled_rectangle(console[c].x, console[c].y, console[c].x + console[c].w_pixels, console[c].y + console[c].h_pixels, colours.base_trans [COL_BLUE] [SHADE_MED] [1]);
			add_menu_button(console[c].x, console[c].y, console[c].x + console[c].w_pixels, console[c].y + console[c].h_pixels,
																			colours.base_trans [COL_BLUE] [SHADE_MED] [TRANS_MED], 7, 3);


		line_index = console[c].cpos;
	 for (i = 0; i < console[c].h_lines; i ++)
		{
			sancheck(line_index, 0, CLINES, "display_consoles_and_buttons: line_index");
			if (console[c].cline[line_index].time_written > w.world_time - 64)
			 add_menu_button(console[c].x + 3, console[c].y + console[c].h_pixels - ((i+1)*CONSOLE_LINE_HEIGHT) + 1, console[c].x + console[c].w_pixels - 6, console[c].y + console[c].h_pixels - ((i)*CONSOLE_LINE_HEIGHT) - 1,
																				al_map_rgba(80, 120, 200, 129 - (w.world_time - console[c].cline[line_index].time_written) * 2), 5, 3);

//				 															colours.base_trans [COL_BLUE] [SHADE_MED] [TRANS_MED], 3, 1);

			line_index --;
			if (line_index < 0)
				line_index = CLINES-1;
		}

		if (console[c].line_highlight_time >= game.total_time)
			 add_menu_button(console[c].x + 3, console[c].y + ((console[c].line_highlight)*CONSOLE_LINE_HEIGHT) + 1, console[c].x + console[c].w_pixels - 6, console[c].y + ((console[c].line_highlight+1)*CONSOLE_LINE_HEIGHT) - 1,
																				al_map_rgba(100, 140, 200, 80), 5, 3);


		draw_menu_buttons();

		line_index = console[c].cpos;
	 for (i = 0; i < console[c].h_lines; i ++)
		{
			sancheck(line_index, 0, CLINES, "display_consoles_and_buttons: line_index B");
			if (console[c].cline[line_index].source_index != -1)
 			al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], console[c].x + scaleUI_x(FONT_BASIC,22), console[c].y + console[c].h_pixels - ((i+1)*CONSOLE_LINE_HEIGHT) + 5, ALLEGRO_ALIGN_RIGHT, "%i", console[c].cline[line_index].source_index);
			al_draw_textf(font[FONT_SQUARE].fnt, colours.print [console[c].cline[line_index].colour], console[c].x + scaleUI_x(FONT_BASIC,29), console[c].y + console[c].h_pixels - ((i+1)*CONSOLE_LINE_HEIGHT) + 4, ALLEGRO_ALIGN_LEFT, "%s", console[c].cline[line_index].text);

			line_index --;
			if (line_index < 0)
				line_index = CLINES-1;
		}

	}

c = CONSOLE_SYSTEM; // system console gets special minimalist treatment:

 if (console[c].time_written > w.world_time - 64)
	{
			add_menu_button(console[c].x, console[c].y, console[c].x + console[c].w_pixels, console[c].y + console[c].h_pixels,
																			colours.packet [1] [(console[c].time_written + 64 - w.world_time) / 2], 7, 3);
	}



		line_index = console[c].cpos;
	 for (i = 0; i < console[c].h_lines; i ++)
		{
			sancheck(line_index, 0, CLINES, "display_consoles_and_buttons: line_index C");
			al_draw_textf(font[FONT_SQUARE].fnt, colours.print [console[c].cline[line_index].colour], console[c].x + 29, console[c].y + console[c].h_pixels - ((i+1)*CONSOLE_LINE_HEIGHT) + 4, ALLEGRO_ALIGN_LEFT, "%s", console[c].cline[line_index].text);

			line_index --;
			if (line_index < 0)
				line_index = CLINES-1;
		}

#endif


// TO DO: initialise this in a more sensible way

	if (command.display_build_buttons)
	{

  reset_i_buttons();
		int button_y = view.build_buttons_y2 - (BUILD_BUTTON_H * TEMPLATES_PER_PLAYER);

		int button_shade;
		int button_colour;
		int button_trans;
		int template_index = 1;

// Only display the build command buttons in command mode (the queue is displayed in auto mode though)
	if (w.command_mode == COMMAND_MODE_COMMAND)
	{
  for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
		{
			template_index = i;
			button_colour = COL_TURQUOISE;
			button_trans = TRANS_MED;
			if (!templ[game.user_player_index][template_index].active)
			{
				button_shade = SHADE_LOW;
				button_trans = TRANS_FAINT;
// 			sprintf(build_button_string [i], "Empty");
			}
			 else
				{
			  if (view.mouse_on_build_button_timestamp == game.total_time
				  && view.mouse_on_build_button == template_index)
				 {
					  button_shade = SHADE_HIGH;
				 }
			     else
					    button_shade = SHADE_MED;

					if (templ[game.user_player_index][template_index].member[0].shape < FIRST_MOBILE_NSHAPE)
					{
						button_colour = COL_BLUE;

						if (command.build_mode != BUILD_MODE_NONE
							&& command.build_template_index == i)
						{
    	   al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_BLUE] [SHADE_HIGH] [TRANS_THICK], view.build_buttons_x2 + 6, button_y + (i * BUILD_BUTTON_H) + scaleUI_y(FONT_SQUARE,7), ALLEGRO_ALIGN_LEFT, "Place static process >>");
  					 button_shade = SHADE_HIGH;
						}

					}
//  			sprintf(build_button_string [i], "%s", templ[game.user_player_index][i].name);
//  			sprintf(build_button_string [i], "%s (%i)", templ[game.user_player_index][i].name, templ[game.user_player_index][i].data_cost);
				}

			add_menu_button(view.build_buttons_x1, button_y + (i * BUILD_BUTTON_H), view.build_buttons_x2, button_y + (i * BUILD_BUTTON_H) + BUILD_BUTTON_H - 2,
																			colours.base_trans [button_colour] [button_shade] [button_trans], 7, 3);
//			add_menu_string(view.build_buttons_x1 + 6, button_y + (i * BUILD_BUTTON_H) + 10, &colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_THICK], 0, FONT_SQUARE, build_button_string [i]);
//			al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH],
//																	view.build_buttons_x1 + 2, button_y - (buttons_drawn * BUILD_BUTTON_H) + 2, ALLEGRO_ALIGN_LEFT, "Build %i:%s", i, templ[game.user_player_index][i].name);
//			buttons_drawn++;
		}
	 //if (buttons_drawn > 0)
//		 draw_menu_buttons();

		add_menu_button(view.build_buttons_x1 - 3, button_y - BUILD_BUTTON_H, view.build_buttons_x2 - 13, button_y - 2,
																			colours.base_trans [COL_BLUE] [SHADE_MAX] [TRANS_MED], 7, 3);


   draw_vbuf();

	  al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_THICK], view.build_buttons_x1 + 3, button_y - BUILD_BUTTON_H + scaleUI_y(FONT_SQUARE,7), ALLEGRO_ALIGN_LEFT, "Build");


   template_index = 0;

   for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
		 {
		 	float text_y = button_y + (i * BUILD_BUTTON_H) + scaleUI_y(FONT_SQUARE,7);

		  al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_MED], view.build_buttons_x1 + 6, text_y, ALLEGRO_ALIGN_LEFT, "%i", template_index);
 			if (templ[game.user_player_index][template_index].active)
			 {
			  al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_THICK], view.build_buttons_x1 + scaleUI_x(FONT_SQUARE,22), text_y, ALLEGRO_ALIGN_LEFT, "%s", templ[game.user_player_index][template_index].name);
			  al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_THICK], view.build_buttons_x2 - 8, text_y, ALLEGRO_ALIGN_RIGHT, "%i", templ[game.user_player_index][template_index].data_cost);
//			  al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_THICK], view.build_buttons_x2 - 8, button_y + (i * BUILD_BUTTON_H) + 10, ALLEGRO_ALIGN_RIGHT, "%i", templ[game.user_player_index][template_index].data_cost);

//				 core->build_cooldown_time = w.world_time + ((templ[core->player_index][build_template].build_cooldown_time / core->number_of_build_objects + 1) * EXECUTION_COUNT); // number_of_build_objects has been confirmed to be non-zero above

			 }
			  else
 			  al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_MED], view.build_buttons_x1 + 22, text_y, ALLEGRO_ALIGN_LEFT, "empty");
 			template_index++;
// 			template_index %= TEMPLATES_PER_PLAYER;
		 }



		 if (command.build_mode != BUILD_MODE_NONE)
			{
				button_y = view.build_buttons_y2 - 5;
				int fail_message = 0;
#define BUILD_FAIL_BUTTON_W 160
#define BUILD_FAIL_BUTTON_H 30
				if (command.build_fail_collision)
				{
							add_menu_button(view.build_buttons_x2 + 5, button_y - BUILD_FAIL_BUTTON_H, view.build_buttons_x2 + BUILD_FAIL_BUTTON_W, button_y - 2,
																			colours.base_trans [COL_RED] [SHADE_MAX] [TRANS_MED], 7, 3);
							add_menu_string(view.build_buttons_x2 + 12, button_y - BUILD_FAIL_BUTTON_H + 10, &colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_THICK], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, build_fail_message [0]);
							fail_message = 1;
				   button_y -= BUILD_FAIL_BUTTON_H;
				}
				if (command.build_fail_range)
				{
							add_menu_button(view.build_buttons_x2 + 5, button_y - BUILD_FAIL_BUTTON_H, view.build_buttons_x2 + BUILD_FAIL_BUTTON_W, button_y - 2,
																			colours.base_trans [COL_RED] [SHADE_MAX] [TRANS_MED], 7, 3);
							add_menu_string(view.build_buttons_x2 + 12, button_y - BUILD_FAIL_BUTTON_H + 10, &colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_THICK], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, build_fail_message [1]);
							fail_message = 1;
				   button_y -= BUILD_FAIL_BUTTON_H;
				}
				if (command.build_fail_static)
				{
							add_menu_button(view.build_buttons_x2 + 5, button_y - BUILD_FAIL_BUTTON_H, view.build_buttons_x2 + BUILD_FAIL_BUTTON_W, button_y - 2,
																			colours.base_trans [COL_RED] [SHADE_MAX] [TRANS_MED], 7, 3);
							add_menu_string(view.build_buttons_x2 + 12, button_y - BUILD_FAIL_BUTTON_H + 10, &colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_THICK], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, build_fail_message [2]);
							fail_message = 1;
   				button_y -= BUILD_FAIL_BUTTON_H;
				}
				if (command.build_fail_edge)
				{
							add_menu_button(view.build_buttons_x2 + 5, button_y - BUILD_FAIL_BUTTON_H, view.build_buttons_x2 + BUILD_FAIL_BUTTON_W, button_y - 2,
																			colours.base_trans [COL_RED] [SHADE_MAX] [TRANS_MED], 7, 3);
							add_menu_string(view.build_buttons_x2 + 12, button_y - BUILD_FAIL_BUTTON_H + 10, &colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_THICK], ALLEGRO_ALIGN_LEFT, FONT_SQUARE, build_fail_message [3]);
							fail_message = 1;
   				button_y -= BUILD_FAIL_BUTTON_H;
				}
				if (fail_message)
				{
					draw_menu_buttons();
//					draw_menu_strings();
				}

			}


		button_y = view.build_buttons_y2 - (BUILD_BUTTON_H * (TEMPLATES_PER_PLAYER + 3)) - 10;

	} // end if w.command_mode == COMMAND_MODE_COMMAND
	 else
 		button_y = view.build_buttons_y2 - BUILD_BUTTON_H * 2 - 10;


// queue: (should be able to assume that command.builder_core_index is valid if display_build_buttons is 1

//   fpr("\nQueue: ");

//  for (i = 0; i < BUILD_COMMAND_QUEUE; i ++)
		{
//   fpr(" %i", w.core[command.builder_core_index].build_command_queue[i].active);

		}
/*
  for (i = 0; i < BUILD_QUEUE_LENGTH; i ++)
		{
			if (w.core[command.builder_core_index].build_command_queue[i].active == 0)
				break;

			  if (view.mouse_on_build_queue_button_timestamp == game.total_time
				  && view.mouse_on_build_queue_button == i)
					  button_shade = SHADE_HIGH;
			     else
					    button_shade = SHADE_MED;


			add_menu_button(view.build_buttons_x1, button_y - (i * BUILD_BUTTON_H), view.build_buttons_x2, button_y - (i * BUILD_BUTTON_H) + BUILD_BUTTON_H - 2,
																			colours.base_trans [COL_AQUA] [button_shade] [TRANS_MED], 3, 7);


		}

  for (i = 0; i < BUILD_COMMAND_QUEUE; i ++)
		{
			if (w.core[command.builder_core_index].build_command_queue[i].active == 0)
				break;

			template_index = w.core[command.builder_core_index].build_command_queue[i].build_template;
	  al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_THICK], view.build_buttons_x1 + 22, button_y - (i * BUILD_BUTTON_H) + 10, ALLEGRO_ALIGN_LEFT, "%s", templ[game.user_player_index][template_index].name);
   if (w.core[command.builder_core_index].build_command_queue[i].build_command_ctrl)
	   al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_TURQUOISE] [SHADE_MAX] [TRANS_THICK], view.build_buttons_x2 - 8, button_y - (i * BUILD_BUTTON_H) + 10, ALLEGRO_ALIGN_RIGHT, "+");

		}


*/

  int queue_header_col = COL_AQUA;
// yellow means temporary failure (probably)
// red means error or permanent failure (e.g. invalid template or build location out of bounds)
  char queue_header_text [50] = "Queue";

  switch(w.player[game.user_player_index].build_queue_fail_reason)
  {
			case BUILD_FAIL_DATA:
				if (w.player[game.user_player_index].build_queue[0].template_index != -1
					&& templ[game.user_player_index][w.player[game.user_player_index].build_queue[0].template_index].data_cost > w.player[game.user_player_index].data)
			  sprintf(queue_header_text, "Need %i data", templ[game.user_player_index][w.player[game.user_player_index].build_queue[0].template_index].data_cost - w.player[game.user_player_index].data);
		  break;
			case BUILD_FAIL_NOT_READY:
				sancheck(w.player[game.user_player_index].build_queue[0].core_index, 0, w.max_cores, "display_consoles_and_buttons: core_index");
				if (w.core[w.player[game.user_player_index].build_queue[0].core_index].build_cooldown_time > w.world_time)
		   sprintf(queue_header_text, "Recycle %i", (w.core[w.player[game.user_player_index].build_queue[0].core_index].build_cooldown_time - w.world_time) / EXECUTION_COUNT);
		  break;
			case BUILD_FAIL_COLLISION:
		  strcpy(queue_header_text, "Collision");
    queue_header_col = COL_YELLOW;
    break;
			case BUILD_FAIL_NO_BUILD_OBJECTS:
		  strcpy(queue_header_text, "No build object"); // this can happen if the process' component with a build object is destroyed
    queue_header_col = COL_YELLOW;
    break;
   case BUILD_FAIL_OUT_OF_BOUNDS:
		  strcpy(queue_header_text, "Invalid location");//, w.player[game.user_player_index].build_queue[0].build_x, w.player[game.user_player_index].build_queue[0].build_y);
    queue_header_col = COL_RED;
    break;
   case BUILD_FAIL_OUT_OF_RANGE:
		  strcpy(queue_header_text, "Out of range");
    queue_header_col = COL_YELLOW;
    break;
   case BUILD_FAIL_POWER:
		  strcpy(queue_header_text, "Not enough power");
    queue_header_col = COL_YELLOW;
    break;
   case BUILD_FAIL_STATIC_NEAR_WELL:
		  strcpy(queue_header_text, "Too near well");
    queue_header_col = COL_RED;
    break;
   case BUILD_FAIL_TEMPLATE_ERROR:
   case BUILD_FAIL_TEMPLATE_INVALID:
   case BUILD_FAIL_TEMPLATE_INACTIVE:
   case BUILD_FAIL_TEMPLATE_NOT_LOCKED:
		  strcpy(queue_header_text, "Template error");
    queue_header_col = COL_RED;
    break;
   case BUILD_FAIL_TOO_MANY_CORES:
		  strcpy(queue_header_text, "Too many processes");
    queue_header_col = COL_YELLOW;
    break;
   case BUILD_FAIL_TOO_MANY_PROCS:
		  strcpy(queue_header_text, "Too many components");
    queue_header_col = COL_YELLOW;
    break;
  }



  int queue_button_highlight_mouseover = -1;
  int queue_button_highlight_drag = -1;
  int button_col = COL_AQUA;

  if (control.mouse_drag == MOUSE_DRAG_BUILD_QUEUE)
		{

			queue_button_highlight_drag = control.mouse_drag_build_queue_button;
/*
   int queue_drag_target_position = work_out_queue_drag_position() - 1;

   if (queue_drag_target_position != -1)
			{
						  add_menu_button(view.build_buttons_x1 + 12, button_y - (queue_drag_target_position * BUILD_BUTTON_H) - 9, view.build_buttons_x2 - 12, button_y - (queue_drag_target_position * BUILD_BUTTON_H) + 9,// - BUILD_BUTTON_H - 2,
				 															colours.base_trans [COL_GREEN] [SHADE_MAX] [TRANS_FAINT], 3, 7);

			}
*/

		}
//		 else
			{

			  if (view.mouse_on_build_queue_button_timestamp == game.total_time)
						queue_button_highlight_mouseover = view.mouse_on_build_queue_button;

/*
			  if (view.mouse_on_build_queue_button_timestamp == game.total_time
				  && view.mouse_on_build_queue_button == i)
					  button_shade = SHADE_HIGH;
			     else
					    button_shade = SHADE_MED;*/

			}

  for (i = 0; i < BUILD_QUEUE_LENGTH; i ++)
		{
			if (w.player[game.user_player_index].build_queue[i].active == 0)
				break;

			if (i == queue_button_highlight_mouseover)
			{
				button_shade = SHADE_HIGH;
			}
			  else
  				button_shade = SHADE_MED;

			if (i == queue_button_highlight_drag)
				button_col = COL_GREEN;
			  else
  				button_col = COL_AQUA;


			add_menu_button(view.build_buttons_x1, button_y - (i * BUILD_BUTTON_H), view.build_buttons_x2, button_y - (i * BUILD_BUTTON_H) + BUILD_BUTTON_H - 2,
																			colours.base_trans [button_col] [button_shade] [TRANS_MED], 3, 7);

		}

// Queue header button:
		  add_menu_button(view.build_buttons_x1 - 3, button_y + (BUILD_BUTTON_H*1), view.build_buttons_x2 - 13, button_y + (BUILD_BUTTON_H*2) - 2,// - BUILD_BUTTON_H - 2,
				 															colours.base_trans [queue_header_col] [SHADE_MAX] [TRANS_MED], 3, 7);

   int draw_cancel_x = 0;

 sancheck(queue_button_highlight_mouseover, -1, BUILD_QUEUE_LENGTH, "queue_button_highlight_mouseover");

// add cancel button:
    if (queue_button_highlight_mouseover != -1
					&& w.player[game.user_player_index].build_queue[queue_button_highlight_mouseover].active
				 && queue_button_highlight_drag == -1
				 && w.command_mode == COMMAND_MODE_COMMAND)
				{

					draw_cancel_x = 1;

					if (control.mouse_x_screen_pixels >= view.build_buttons_x2 - 42
					 && control.mouse_x_screen_pixels <= view.build_buttons_x2 - 10
					 && control.mouse_y_screen_pixels >= button_y - (queue_button_highlight_mouseover * BUILD_BUTTON_H) // + 4
						&& control.mouse_y_screen_pixels <= button_y - (queue_button_highlight_mouseover * BUILD_BUTTON_H) + BUILD_BUTTON_H - 2)// - 4)
							button_shade = SHADE_HIGH;
					   else
									button_shade = SHADE_MED;

			  add_menu_button(view.build_buttons_x2 - 42,
																					button_y - (queue_button_highlight_mouseover * BUILD_BUTTON_H) + 4,
																					view.build_buttons_x2 - 10,
																					button_y - (queue_button_highlight_mouseover * BUILD_BUTTON_H) + BUILD_BUTTON_H - 2 - 4,
																			  colours.base_trans [COL_YELLOW] [button_shade] [TRANS_MED], 3, 7);


				}


    draw_vbuf();

  if (draw_cancel_x)
  	  al_draw_textf(font[FONT_BASIC].fnt,
																			colours.base_trans [COL_YELLOW] [SHADE_MAX] [TRANS_MED],
																			view.build_buttons_x2 - 26,
																			button_y - (queue_button_highlight_mouseover * BUILD_BUTTON_H) + scaleUI_y(FONT_BASIC,8),
																			ALLEGRO_ALIGN_CENTRE,
																			"X");

  for (i = 0; i < BUILD_QUEUE_LENGTH; i ++)
		{
			if (w.player[game.user_player_index].build_queue[i].active == 0)
				break;

			template_index = w.player[game.user_player_index].build_queue[i].template_index;
			sancheck(template_index, 0, TEMPLATES_PER_PLAYER, "console template_index");
	  al_draw_textf(font[FONT_BASIC].fnt, colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_THICK], view.build_buttons_x1 + 10, button_y - (i * BUILD_BUTTON_H) + scaleUI_y(FONT_BASIC,9), ALLEGRO_ALIGN_LEFT, "%i", w.player[game.user_player_index].build_queue[i].core_index);
	  al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_THICK], view.build_buttons_x1 + 32, button_y - (i * BUILD_BUTTON_H) + scaleUI_y(FONT_SQUARE,7), ALLEGRO_ALIGN_LEFT, "%s", templ[game.user_player_index][template_index].name);
   if (w.player[game.user_player_index].build_queue[i].repeat)
	   al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_TURQUOISE] [SHADE_MAX] [TRANS_THICK], view.build_buttons_x2 - 8, button_y - (i * BUILD_BUTTON_H) + scaleUI_y(FONT_SQUARE,7), ALLEGRO_ALIGN_RIGHT, "+");

		}




//			if (w.core[command.builder_core_index].build_command_queue[0].active != 0)
			{



	   al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_THICK], view.build_buttons_x1 + 5, button_y + BUILD_BUTTON_H + scaleUI_y(FONT_SQUARE,7), ALLEGRO_ALIGN_LEFT, "%s", queue_header_text);
			}






	} // end of code to display build buttons and build queue

 if (game.phase == GAME_PHASE_PREGAME)
 {
 	 int shade = SHADE_MED;

 	 if (control.mouse_panel == PANEL_MAIN
				&&	control.mouse_x_screen_pixels > view.window_x_unzoomed / 2 - scaleUI_x(FONT_SQUARE_LARGE, 180)
			 && control.mouse_x_screen_pixels < view.window_x_unzoomed / 2 + scaleUI_x(FONT_SQUARE_LARGE, 180)
			 && control.mouse_y_screen_pixels > view.window_y_unzoomed / 2 - scaleUI_y(FONT_SQUARE_LARGE, 60)
			 && control.mouse_y_screen_pixels < view.window_y_unzoomed / 2 + scaleUI_y(FONT_SQUARE_LARGE, 30))
					shade = SHADE_MAX;

	  add_menu_button(view.window_x_unzoomed / 2 - scaleUI_x(FONT_SQUARE_LARGE, 180),
																			view.window_y_unzoomed / 2 - scaleUI_y(FONT_SQUARE_LARGE, 60),
																			view.window_x_unzoomed / 2 + scaleUI_x(FONT_SQUARE_LARGE, 180),
																			view.window_y_unzoomed / 2 + scaleUI_y(FONT_SQUARE_LARGE, 30),
			 															colours.base_trans [COL_AQUA] [shade] [TRANS_MED], 12, 7);
// button dimensions also dealt with in run_pregame() in g_game.c

   draw_vbuf();

   al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 - 20, ALLEGRO_ALIGN_CENTRE, ">>> CLICK WHEN READY <<<");

   if (game.spawn_fail != -1)
			{

 	  add_menu_button(view.window_x_unzoomed / 2 - scaleUI_x(FONT_SQUARE, 195),
																			 view.window_y_unzoomed / 2 + scaleUI_y(FONT_SQUARE, 55),
																			 view.window_x_unzoomed / 2 + scaleUI_x(FONT_SQUARE, 195),
																			 view.window_y_unzoomed / 2 + scaleUI_y(FONT_SQUARE, 125),
			 															 colours.base_trans [COL_RED] [SHADE_LOW] [TRANS_FAINT], 12, 7);
// button dimensions also dealt with in run_pregame() in g_game.c

   draw_vbuf();

   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_RED] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 + scaleUI_y(FONT_SQUARE, 65), ALLEGRO_ALIGN_CENTRE, "Player %i spawn failure", game.spawn_fail);
   switch(game.spawn_fail_reason)
   {
   	case SPAWN_FAIL_LOCK:
     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_RED] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 + scaleUI_y(FONT_SQUARE, 85), ALLEGRO_ALIGN_CENTRE, "Template 0 could not be locked.");
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_RED] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 + scaleUI_y(FONT_SQUARE, 105), ALLEGRO_ALIGN_CENTRE, "Check the template and make sure it's loaded properly.");
     break;
   	case SPAWN_FAIL_DATA:
     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_RED] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 + scaleUI_y(FONT_SQUARE, 90), ALLEGRO_ALIGN_CENTRE, "Template 0 data cost too high (maximum %i).", w.player[game.spawn_fail].data);
     break;
   }


			}
 }
  else
		{
   if (game.phase == GAME_PHASE_OVER
			 && game.game_over_time > 60)
   {
   	 int shade = SHADE_MED;

 	   if (control.mouse_panel == PANEL_MAIN
				  &&	control.mouse_x_screen_pixels > view.window_x_unzoomed / 2 - scaleUI_x(FONT_SQUARE_LARGE, 180)
			   && control.mouse_x_screen_pixels < view.window_x_unzoomed / 2 + scaleUI_x(FONT_SQUARE_LARGE, 180)
			   && control.mouse_y_screen_pixels > view.window_y_unzoomed / 2 - scaleUI_y(FONT_SQUARE_LARGE, 60)
			   && control.mouse_y_screen_pixels < view.window_y_unzoomed / 2 + scaleUI_y(FONT_SQUARE_LARGE, 30))
					  shade = SHADE_MAX;

	    add_menu_button(view.window_x_unzoomed / 2 - scaleUI_x(FONT_SQUARE_LARGE, 180),
																			  view.window_y_unzoomed / 2 - scaleUI_y(FONT_SQUARE_LARGE, 60),
																			  view.window_x_unzoomed / 2 + scaleUI_x(FONT_SQUARE_LARGE, 180),
																			  view.window_y_unzoomed / 2 + scaleUI_y(FONT_SQUARE_LARGE, 30),
			 															  colours.base_trans [COL_YELLOW] [shade] [TRANS_MED], 12, 7);
// button dimensions also dealt with in run_pregame() in g_game.c

     draw_vbuf();

     al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 - 20, ALLEGRO_ALIGN_CENTRE, ">>> CLICK TO EXIT <<<");
   }
    else
     draw_vbuf();
		}


}


void place_build_buttons(void)
{
	view.build_buttons_x1 = console[CONSOLE_GENERAL].x;
	view.build_buttons_y2 = console[CONSOLE_GENERAL].y - 5;
	view.build_buttons_x2 = view.build_buttons_x1 + BUILD_BUTTON_W;
	view.build_buttons_y1 = view.build_buttons_y2 - (TEMPLATES_PER_PLAYER * BUILD_BUTTON_H);
	if (w.command_mode == COMMAND_MODE_COMMAND)
	 view.build_queue_buttons_y2 = view.build_buttons_y2 - (BUILD_BUTTON_H * (TEMPLATES_PER_PLAYER + 2)) - 10; //view.build_buttons_y1 - 5;
	  else
	   view.build_queue_buttons_y2 = view.build_buttons_y2; //view.build_buttons_y1 - 5;

 reset_build_queue_buttons_y1(-1);
//		button_y = view.build_buttons_y2 - (BUILD_BUTTON_H * (TEMPLATES_PER_PLAYER + 3)) - 10;


}

// works out the top of the build queue button thing.
// if queue_length == -1, this function will work out the queue length itself
void reset_build_queue_buttons_y1(int queue_length)
{
	if (queue_length == -1)
	{
		queue_length = 0;
		while (w.player[game.user_player_index].build_queue[queue_length].active)
		{
			queue_length ++;
			sancheck(queue_length, 0, BUILD_QUEUE_LENGTH, "reset_build_queue_buttons_y1: queue_length");
		}
	}

	view.build_queue_buttons_y1 = view.build_queue_buttons_y2 - (queue_length * BUILD_BUTTON_H);

}

void check_mouse_on_consoles_etc(int mouse_x, int mouse_y, int left_button)
{


}

void write_text_to_console(int console_index, int print_colour, int text_source, int source_core_created_timestamp, char* write_text)
{

sancheck(console_index, 0, CONSOLES, "write_text_to_console: console_index");

// if the most recently written line was written by a different core, or in a different tick, go to next line:
 if ((console[console_index].source_index != text_source
		 || console[console_index].time_written != w.world_time)
				&& write_text[0] != '\n') // no need for newline if process has written newline itself
			console_newline(console_index, print_colour);

	int line_pos = console[console_index].current_line_length;
	int write_text_pos = 0;

	while(write_text [write_text_pos] != '\0')
	{
  if (write_text [write_text_pos] == '\n')
		{
  	sancheck(console[console_index].cpos, 0, CLINES, "write_text_to_console: console[console_index].cpos A");
  	console[console_index].cline[console[console_index].cpos].text [line_pos] = '\0';
  	if (write_text_pos != 0) // don't indicate a newly written line on the old line if the \n is at the start of the new line
	   console[console_index].cline[console[console_index].cpos].time_written = w.world_time;
			console_newline(console_index, print_colour);
			write_text_pos ++;
			line_pos = 0;
			continue;
		}
		if (line_pos >= console[console_index].w_letters - 2)
		{
  	sancheck(console[console_index].cpos, 0, CLINES, "write_text_to_console: console[console_index].cpos B");
  	console[console_index].cline[console[console_index].cpos].text [line_pos] = '\0';
   console[console_index].cline[console[console_index].cpos].time_written = w.world_time;
			console_newline(console_index, print_colour);
			line_pos = 0;
		}
 	sancheck(console[console_index].cpos, 0, CLINES, "write_text_to_console: console[console_index].cpos C");
		console[console_index].cline[console[console_index].cpos].text [line_pos] = write_text [write_text_pos];
		write_text_pos ++;
		line_pos ++;

	};

	sancheck(console[console_index].cpos, 0, CLINES, "write_text_to_console: console[console_index].cpos D");

	console[console_index].cline[console[console_index].cpos].text [line_pos] = '\0';
 console[console_index].cline[console[console_index].cpos].time_written = w.world_time;
	console[console_index].current_line_length = line_pos;

	console[console_index].cline[console[console_index].cpos].source_index = text_source;
	console[console_index].cline[console[console_index].cpos].source_core_created_timestamp = source_core_created_timestamp;

 console[console_index].source_index = text_source;
 console[console_index].time_written = w.world_time;


}


void console_newline(int console_index, int print_colour)
{
	console[console_index].cpos++;
	if (console[console_index].cpos >= CLINES)
		console[console_index].cpos = 0;

	console[console_index].cline[console[console_index].cpos].text [0] = '\0';
	console[console_index].current_line_length = 0;
	console[console_index].cline[console[console_index].cpos].source_index = -1;
	console[console_index].cline[console[console_index].cpos].colour = print_colour;

}



// This function is bypassed by bubble text written as a result of special_AI calls
//  - see special_AI_method()
void write_text_to_bubble(int core_index, timestamp print_timestamp, char* write_text)
{

 if (w.core[core_index].bubble_text_time != print_timestamp)
	{
		w.core[core_index].bubble_text [0] = '\0';
		w.core[core_index].bubble_text_length = 0;
	 w.core[core_index].bubble_text_time = print_timestamp;
		if (w.core[core_index].bubble_text_time >= print_timestamp - BUBBLE_TOTAL_TIME)
   w.core[core_index].bubble_text_time_adjusted = print_timestamp - 11;
		  else
  		 w.core[core_index].bubble_text_time_adjusted = print_timestamp;
	}

	int bubble_text_pos = w.core[core_index].bubble_text_length;
	int write_text_pos = 0;

	while(write_text [write_text_pos] != '\0')
	{
  if (write_text [write_text_pos] == '\n')
		{
			bubble_text_pos = 0;
 		w.core[core_index].bubble_text [0] = '\0';
	 	w.core[core_index].bubble_text_length = 0;
			write_text_pos ++;
			continue;
		}
		if (bubble_text_pos >= BUBBLE_TEXT_LENGTH_MAX - 3)
		{
 		bubble_text_pos ++;
			break;
		}
		w.core[core_index].bubble_text [bubble_text_pos] = write_text [write_text_pos];
		write_text_pos ++;
		bubble_text_pos ++;
	};

	w.core[core_index].bubble_text [bubble_text_pos] = '\0';
	w.core[core_index].bubble_text_length = bubble_text_pos;



 play_game_sound(SAMPLE_BUBBLE, TONE_2C, 90, 1, w.core[core_index].core_position.x, w.core[core_index].core_position.y);


}


