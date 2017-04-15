
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
#include "e_help.h"
#include "e_log.h"
#include "e_inter.h"

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
#include "t_draw.h"

#include "p_panels.h"
#include "d_draw.h"
#include "d_design.h"
#include "v_draw_panel.h"


//#include "s_setup.h"

extern struct fontstruct font [FONTS];
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];

extern ALLEGRO_DISPLAY* display;

// these queues are declared in g_game.c. They're externed here so that they can be flushed when the editor does something slow.
extern ALLEGRO_EVENT_QUEUE* event_queue; // these queues are initialised in main.c
extern ALLEGRO_EVENT_QUEUE* fps_queue;

extern struct game_struct game; // in g_game.c
extern struct view_struct view;

char mstring [MENU_STRING_LENGTH];

void display_standard_panel(int pan);
static void print_sysmenu_help(float base_x, float base_y);

/*

Plan: how to draw buttons etc:

- problem: need to draw primitives first, then strings.
 - possibly may need to draw primitives after strings too, sometimes (although maybe just for overwindows)



*/

char mode_button_text [MODE_BUTTONS] [3] =
{
	"X", // not actually used
	"Sy",
	"Ed",
	"De",
	"Te",
	"BC",
//	">>"
};


void draw_panels(void)
{
 int i;

 int any_panel_open = 0;

 for (i = 1; i < PANELS; i ++) // note: starts at 1
	{
		if (!panel[i].open)
			continue;

		any_panel_open = 1;

	 al_set_clipping_rectangle(panel[i].x1, panel[i].y1, panel[i].w, panel[i].h);

  switch(i)
  {
		 case PANEL_EDITOR:
//				display_standard_panel(PANEL_EDITOR);
			 draw_edit_bmp();
			 break;
			case PANEL_LOG:
				display_log();
				break;
			case PANEL_SYSMENU:
    al_clear_to_color(panel[i].background_colour);
    al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], panel[i].x1 + 9, panel[i].y1 + 2, ALLEGRO_ALIGN_LEFT, "System");
				display_standard_panel(PANEL_SYSMENU);
				print_sysmenu_help(panel[i].x1 + scaleUI_x(FONT_BASIC,16), 150);
				break;
			case PANEL_DESIGN:
		  al_clear_to_color(panel[i].background_colour);
    al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], panel[i].x1 + 9, panel[i].y1 + 2, ALLEGRO_ALIGN_LEFT, "Process design");
  		display_standard_panel(PANEL_DESIGN);
				break;
			case PANEL_TEMPLATE:
				draw_template_panel();
//    al_clear_to_color(panel[i].background_colour);
//				display_standard_panel(PANEL_TEMPLATE);
				break;
			case PANEL_BCODE:
    draw_bcode_panel();
				break;

  }

//	 al_draw_rectangle(panel[i].x1 + 5, panel[i].y1 + 5, panel[i].x2 - 5, panel[i].y2 - 5, colours.base [COL_AQUA] [SHADE_HIGH], 1);

	}

	 al_set_clipping_rectangle(0, 0, inter.display_w, inter.display_h);
	 /*
	 float mode_buttons_x1, mode_buttons_y1;



  if (any_panel_open)
 	{
 		mode_buttons_x1 = inter.display_w;
 		mode_buttons_y1 = 5;
 	}
 	 else
			{
 		 mode_buttons_x1 = inter.display_w - 200;
 		 mode_buttons_y1 = 55;
			}
*/

	 for (i = 0; i < MODE_BUTTONS; i ++)
		{
			float mode_button_i_x1 = inter.mode_buttons_x1 - (MODE_BUTTON_SIZE + MODE_BUTTON_SPACING) * (i);

 	 if (i != MODE_BUTTON_CLOSE
				&& panel[i].open)
  	   add_menu_button(mode_button_i_x1 - 1.5, inter.mode_buttons_y1-1.5, mode_button_i_x1 + MODE_BUTTON_SIZE+1.5, inter.mode_buttons_y1 + MODE_BUTTON_SIZE+1.5, colours.base_trans [COL_BLUE] [SHADE_MAX] [2], 2, 4);
//  	   add_menu_button(mode_button_i_x1 - 0.5, inter.mode_buttons_y1-0.5, mode_button_i_x1 + MODE_BUTTON_SIZE+0.5, inter.mode_buttons_y1 + MODE_BUTTON_SIZE+0.5, colours.base_trans [COL_BLUE] [SHADE_MAX] [1], 1, 3);


			if (inter.mode_button_highlight == i
				&& inter.mode_button_highlight_time >= inter.running_time - 1)
				{
// 	   al_draw_filled_rectangle(mode_button_i_x1, inter.mode_buttons_y1, mode_button_i_x1 + MODE_BUTTON_SIZE, inter.mode_buttons_y1 + MODE_BUTTON_SIZE, colours.base_trans [COL_BLUE] [SHADE_HIGH] [2]);
 	   add_menu_button(mode_button_i_x1, inter.mode_buttons_y1, mode_button_i_x1 + MODE_BUTTON_SIZE, inter.mode_buttons_y1 + MODE_BUTTON_SIZE, colours.base_trans [COL_BLUE] [SHADE_HIGH] [2], 2, 4);
				}
 	   else
					{
// 	    al_draw_filled_rectangle(mode_button_i_x1, inter.mode_buttons_y1, mode_button_i_x1 + MODE_BUTTON_SIZE, inter.mode_buttons_y1 + MODE_BUTTON_SIZE, colours.base_trans [COL_BLUE] [SHADE_MED] [1]);
  	   add_menu_button(mode_button_i_x1, inter.mode_buttons_y1, mode_button_i_x1 + MODE_BUTTON_SIZE, inter.mode_buttons_y1 + MODE_BUTTON_SIZE, colours.base_trans [COL_BLUE] [SHADE_MED] [1], 2, 4);
					}

// 	 if (i != MODE_BUTTON_CLOSE
//				&& panel[i].open)
//  	   add_menu_button(mode_button_i_x1, inter.mode_buttons_y1, mode_button_i_x1 + MODE_BUTTON_SIZE, inter.mode_buttons_y1 + MODE_BUTTON_SIZE, colours.base_trans [COL_BLUE] [SHADE_MED] [1], 2, 3);
		}

 draw_vbuf();

// 	   al_draw_rectangle(mode_button_i_x1 - 0.5, inter.mode_buttons_y1-0.5, mode_button_i_x1 + MODE_BUTTON_SIZE+0.5, inter.mode_buttons_y1 + MODE_BUTTON_SIZE+0.5, colours.base [COL_BLUE] [SHADE_MAX], 1);

//			float mode_button_i_x1 = inter.mode_buttons_x1 - (MODE_BUTTON_SIZE + MODE_BUTTON_SPACING) * (i);

// MODE_BUTTON_CLOSE:

				if (any_panel_open)
     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], inter.mode_buttons_x1 + scaleUI_x(FONT_SQUARE,10), inter.mode_buttons_y1 + scaleUI_y(FONT_SQUARE,5), ALLEGRO_ALIGN_CENTER, "X");
      else
       al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], inter.mode_buttons_x1 + scaleUI_x(FONT_SQUARE,10),
																					inter.mode_buttons_y1 + scaleUI_y(FONT_SQUARE,5), ALLEGRO_ALIGN_CENTER, "<<");



		int col;

	 for (i = (MODE_BUTTON_CLOSE+1); i < MODE_BUTTONS; i ++)
		{
			float mode_button_i_x1 = inter.mode_buttons_x1 - (MODE_BUTTON_SIZE + MODE_BUTTON_SPACING) * (i);


 	 if (panel[i].open)
				col = COL_GREY;
				 else
						 col = COL_BLUE;

//   if (i != MODE_BUTTON_MIN_MAX)
    al_draw_textf(font[FONT_SQUARE].fnt, colours.base [col] [SHADE_MAX], mode_button_i_x1 + scaleUI_x(FONT_SQUARE,10), inter.mode_buttons_y1 + scaleUI_y(FONT_SQUARE,5), ALLEGRO_ALIGN_CENTER, "%s", mode_button_text [i]);

		}

//     else
//     {
//     	if (!any_panel_open)
//       al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], inter.mode_button_x[i] + 9, MODE_BUTTON_Y + 5, ALLEGRO_ALIGN_CENTER, "%s", mode_button_text [i]);
//     }

// 	 adtf(inter.mode_button_x[i] + 3, MODE_BUTTON_Y + 3, "%i:%i", i, inter.mode_button_available[i]);
/*

Plan for mode buttons:

- X button still closes all
- >> button is in different place - probably to the left of the player status box.
 - also have it as a mode button in case the player status box is off screen to the left.

Otherwise, the following states:
0. any panel is open. All buttons shown. Pressing >> closes all panels but saves open/closed status.
1. If no panel is open:
 a. mode buttons are hidden. Only << is shown, in both places. Pressing it moves to state b.
 b. mode buttons are shown in non-scrolling area (may need to push status box down). Pressing >> moves to state a.

OR:
 - when no panels are open, mode buttons appear below player status box (outside scroll area)
 - when panels are open, mode buttons appear in top right.
 - X button closes all
  - when all closed, turns into a << button that recalls last settings
  - opening any panel when all closed resets the last settings



*/
/*
	}
	 else
		{
			i = MODE_BUTTON_CLOSE;

			if (inter.mode_button_highlight == i
				&& inter.mode_button_highlight_time >= inter.running_time - 1)
				{
 	   al_draw_filled_rectangle(inter.mode_button_x[i], MODE_BUTTON_Y, inter.mode_button_x[i] + MODE_BUTTON_SIZE, MODE_BUTTON_Y + MODE_BUTTON_SIZE, colours.base_trans [COL_BLUE] [SHADE_HIGH] [2]);
				}
 	   else
					{
 	    al_draw_filled_rectangle(inter.mode_button_x[i], MODE_BUTTON_Y, inter.mode_button_x[i] + MODE_BUTTON_SIZE, MODE_BUTTON_Y + MODE_BUTTON_SIZE, colours.base_trans [COL_BLUE] [SHADE_MED] [1]);
					}


//   al_draw_filled_rectangle(inter.mode_button_x[MODE_BUTTON_CLOSE], MODE_BUTTON_Y, inter.mode_button_x[MODE_BUTTON_CLOSE] + MODE_BUTTON_SIZE, MODE_BUTTON_Y + MODE_BUTTON_SIZE, colours.base_trans [COL_BLUE] [SHADE_MED] [1]);
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], inter.mode_button_x[MODE_BUTTON_CLOSE] + 9, MODE_BUTTON_Y + 5, ALLEGRO_ALIGN_CENTER, "<<");
		}
*/



}

void display_standard_panel(int pan)
{
 clear_vbuf();
 reset_i_buttons();
	al_set_clipping_rectangle(panel[pan].x1, panel[pan].y1, panel[pan].w, panel[pan].h);
	int col, shade;


	int el;

	for (el = 0; el < ELEMENTS; el++)
	{
		if (panel[pan].subpanel[panel[pan].element[el].subpanel].open
			&& panel[pan].element[el].open
		 && panel[pan].element[el].exists) // possibly could remove this if .open is reliable (as if exists == 0, open should == 0 too)
		{
//			 	fpr("\nButton pan %i el %i", pan, el);
			switch(panel[pan].element[el].type)
			{
			 case PE_TYPE_BUTTON:
				 switch(panel[pan].element[el].style)
				 {
					 case BUTTON_STYLE_MENU_BIG:
					 	add_menu_button(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																							panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x2,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2,
																							colours.base [COL_BLUE] [SHADE_MED + (panel[pan].element[el].last_highlight >= inter.running_time - 1)],
																							12, 6);
							add_menu_string(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + 25,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1 + 25 / font[FONT_BASIC].font_scale_y,
																							&colours.base [COL_BLUE] [SHADE_MAX],
																							ALLEGRO_ALIGN_LEFT,
																							FONT_SQUARE,
																							panel[pan].element[el].name);
							break;
/*
						case BUTTON_STYLE_TAB:
					 	add_menu_button(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																							panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x2,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2,
																							colours.base [COL_BLUE] [SHADE_MED + (panel[pan].element[el].last_highlight >= game.total_time - 1)],
																							1, 2);
							add_menu_string(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + 4,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1 + 4,
																							&colours.base [COL_BLUE] [SHADE_MAX],
																							ALLEGRO_ALIGN_LEFT,
																							FONT_BASIC,
																							panel[pan].element[el].name);
							break;
						case BUTTON_STYLE_TEMPLATE:
					 	add_menu_button(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																							panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x2,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2,
																							colours.base [COL_BLUE] [SHADE_LOW + (panel[pan].element[el].last_highlight >= game.total_time - 1)],
																							6, 12);
						add_menu_string(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + 10,
																						panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1 + 10,
																						&colours.base [COL_GREY] [SHADE_MAX],
																						ALLEGRO_ALIGN_LEFT,
																						FONT_SQUARE,
																						templ[panel[pan].element[el].value[0]][panel[pan].element[el].value[1]].menu_button_title);
						if (templ[panel[pan].element[el].value[0]][panel[pan].element[el].value[1]].name [0] != 0)
						{
						 add_menu_string(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + 50,
																						panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1 + 35,
																						&colours.base [COL_BLUE] [SHADE_MAX],
																						ALLEGRO_ALIGN_LEFT,
																						FONT_SQUARE,
																						templ[tstate.template_player_tab][panel[pan].element[el].value[1]].name);
						}

							break;*/
						case BUTTON_STYLE_DESIGN:
// value [0] = special highlight for button which reflects actual state (e.g. category of actual shape of currently selected template member)
					 	add_menu_button(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																							panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x2,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2 - 1,
																							colours.base [COL_BLUE] [SHADE_LOW + (panel[pan].element[el].last_highlight >= inter.running_time - 1)],
																							2, 4);
							shade = SHADE_MAX;
							col = COL_BLUE;
							if (panel[pan].element[el].value [0])
							{
								shade = SHADE_MAX;
								col = COL_GREY;
							}
							add_menu_string(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + 4,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1 + 6,
																							&colours.base [col] [shade],
																							ALLEGRO_ALIGN_LEFT,
																							FONT_BASIC,
																							panel[pan].element[el].name);
							break;
						case BUTTON_STYLE_DESIGN_SUB:
						case BUTTON_STYLE_SUBMENU:
						case BUTTON_STYLE_SUBMENU_LINE:
							{
// value [0] = DSB value (see d_design.c)
// value [1] = special highlight for button which reflects actual state (e.g. actual shape of currently selected template member)
// value [2] = AUTOCODE_* value
// value [3] = set to 1 if button is an inactive autocode button (e.g. static type for a mobile process)
       int dsb_col;
       if (panel[pan].element[el].value [3] == AUTOCODE_AVAILABLE_YES)
								dsb_col = COL_BLUE;
								 else
							  {
								  if (panel[pan].element[el].value [3] < FIRST_UNAVAILABLE_AUTOCODE_RESULT)
									  dsb_col = COL_ORANGE;
								    else
								     dsb_col = COL_RED;
							  }
					 	add_menu_button(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																							panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x2,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2 - 1,
																							colours.base [dsb_col] [SHADE_MIN + (panel[pan].element[el].last_highlight >= inter.running_time - 1) + (panel[pan].element[el].value [1]!=0)],
																							4, 2);
							shade = SHADE_HIGH;
							if (panel[pan].element[el].value [1])
								shade = SHADE_MAX;
							add_menu_string(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + 4,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1 + 6,
																							&colours.base [dsb_col] [shade],
																							ALLEGRO_ALIGN_LEFT,
																							FONT_BASIC,
																							panel[pan].element[el].name);
							}
							break;
				 }
				 break;
				case PE_TYPE_DESIGN_WINDOW:
					draw_design_window();
					break;
				case PE_TYPE_SCROLLBAR_EL_H_PIXEL:
				case PE_TYPE_SCROLLBAR_EL_V_PIXEL:
				case PE_TYPE_SCROLLBAR_EL_H_CHAR:
				case PE_TYPE_SCROLLBAR_EL_V_CHAR:
					draw_scrollbar(panel[pan].element[el].value [1]);
					break;
				case PE_TYPE_PANEL_RESIZE:
					if (panel[pan].element[el].last_highlight >= inter.running_time - 1)
					{
						add_menu_rectangle(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1,
																									0, //	panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																							  panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + 5, //panel[pan].element[el].x2,
																							  panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2,
																							  colours.base [COL_BLUE] [SHADE_MED]);
					}
					 else
						add_menu_rectangle(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1,
																							  0, //panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																							  panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + 2,
																							  panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2,
																							  colours.base [COL_BLUE] [SHADE_LOW]);
					break;
				case PE_TYPE_DESIGN_DATA:
					draw_design_data();
					break;

			}


		}
	}

 draw_menu_buttons();

}

int smh_text_y;
int smh_text_x;

static void print_sysmenu_line(const char* left_string, const char* right_string, int extra_space);

static void print_sysmenu_help(float base_x, float base_y)
{

#define SMH_LINE_HEIGHT scaleUI_y(FONT_BASIC,15)
#define SMH_LINE_HEADER 4
#define SMH_HEADER_X (base_x + 5)
	smh_text_x = base_x + scaleUI_x(FONT_BASIC,69);
	smh_text_y = base_y;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], SMH_HEADER_X - 10, smh_text_y, ALLEGRO_ALIGN_LEFT, "CONTROLS");
 smh_text_y += SMH_LINE_HEIGHT + SMH_LINE_HEADER;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], SMH_HEADER_X, smh_text_y, ALLEGRO_ALIGN_LEFT, "SELECTING");
 smh_text_y += SMH_LINE_HEIGHT + SMH_LINE_HEADER;
 print_sysmenu_line("SELECT", "Left-click on a process", 1);
 print_sysmenu_line("BOX SELECT", "Left-click then hold to drag a selection box", 1);
 print_sysmenu_line("SELECT+", "Hold shift while selecting to add/remove", 1);
 print_sysmenu_line("SET GROUP", "control + z,x,c,v,b,n or m (+shift for exclusive)", 1);
 print_sysmenu_line("ADD TO GROUP", "shift + z-m to add to a control group", 1);
 print_sysmenu_line("SELECT GROUP", "z-m to select a control group", 1);

 smh_text_y += SMH_LINE_HEADER;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], SMH_HEADER_X, smh_text_y, ALLEGRO_ALIGN_LEFT, "COMMANDS (while a process is selected)");
 smh_text_y += SMH_LINE_HEIGHT + SMH_LINE_HEADER;
 print_sysmenu_line("MOVE", "Right-click on the display or the map", 1);
 print_sysmenu_line("ATTACK", "Right-click on an enemy process", 1);
 print_sysmenu_line("ATTACK-MOVE", "Hold control while giving a MOVE command", 1);
 print_sysmenu_line("HARVEST", "Right-click near a data well", 1);
// print_sysmenu_line("", "(The process must have a harvest object.)");
 print_sysmenu_line("RETURN", "Right-click on a process with an allocator", 0);
 print_sysmenu_line("", "to return to that process after harvesting", 1);
 print_sysmenu_line("GUARD", "Right-click on a friendly target", 1);
// print_sysmenu_line("", "(Requires a repair_other object.)");
 print_sysmenu_line("WAYPOINTS", "Hold shift to queue commands", 1);
 print_sysmenu_line("BUILD", "Select a process with a build object, then use", 0);
 print_sysmenu_line("", "the buttons on the left side of screen to choose", 0);
 print_sysmenu_line("", "what to build", 0);
 print_sysmenu_line("", " + shift for repeat build", 0);
 print_sysmenu_line("", " + control to set to the front of the queue", 1);

 smh_text_y += SMH_LINE_HEADER;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], SMH_HEADER_X, smh_text_y, ALLEGRO_ALIGN_LEFT, "OTHER");
 smh_text_y += SMH_LINE_HEIGHT + SMH_LINE_HEADER;
 print_sysmenu_line("PAUSE", "P to pause/unpause", 1);
 print_sysmenu_line("PAUSE ADVANCE", "[ to advance 1 frame", 1);
 print_sysmenu_line("FOLLOW", "f to follow selected process", 1);
 print_sysmenu_line("GO TO ALERT", "space to cycle through <under attack> alerts", 1);
 print_sysmenu_line("DEBUG MODE", "F1 to toggle debug mode", 1);
 print_sysmenu_line("FAST FORWARD", "F2, F3 and F4 to toggle different speeds", 1);
// print_sysmenu_line("FF (SKIP)", "F3 to toggle extra fast forward (skips frames).");
// print_sysmenu_line("FF (ND)", "F4 to toggle super fast forward (no display).");
 print_sysmenu_line("PANELS", "F6, F7, F8, F9 to open/close BC/Te/De/Ed panels", 1);
 print_sysmenu_line("ZOOM", "Use mousewheel to zoom in/out", 1);
 print_sysmenu_line("QUIT", "Escape to quit program entirely", 1);
 print_sysmenu_line("CHANGE PLAYER", "F5 to change player (custom game only)", 1);


}

static void print_sysmenu_line(const char* left_string, const char* right_string, int extra_space)
{

 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_AQUA] [SHADE_MAX], smh_text_x, smh_text_y, ALLEGRO_ALIGN_RIGHT, "%s", left_string);
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], smh_text_x + 10, smh_text_y, ALLEGRO_ALIGN_LEFT, "%s", right_string);
 smh_text_y += SMH_LINE_HEIGHT - 3 + (extra_space * 3);

}
