
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

#include "i_input.h"
#include "i_view.h"
#include "i_display.h"
#include "i_buttons.h"
#include "t_template.h"

#include "p_panels.h"
#include "d_draw.h"

extern struct game_struct game;
extern struct fontstruct font [FONTS];
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct template_state_struct tstate;

// This function uses some information from the panel struct (e.g. button highlights), but draws the template buttons itself
void draw_template_panel(void)
{

	al_set_clipping_rectangle(panel[PANEL_TEMPLATE].x1, panel[PANEL_TEMPLATE].y1, panel[PANEL_TEMPLATE].w, panel[PANEL_TEMPLATE].h);
	int pan = PANEL_TEMPLATE;
	int el;

 al_clear_to_color(panel[PANEL_TEMPLATE].background_colour);
 int i;

 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], panel[PANEL_TEMPLATE].x1 + 2, panel[PANEL_TEMPLATE].y1 + 2, ALLEGRO_ALIGN_LEFT, "Templates");

// first draw the tabs along the top:
 for (i = 0; i < w.players; i ++)
	{
    		el = FPE_TEMPLATES_TAB_P0 + i;
    		int button_col = COL_BLUE;

     	if (tstate.template_player_tab	== i)
							button_col = COL_CYAN;

					 	add_menu_button(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																							panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x2,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2,
																							colours.base [button_col] [SHADE_MED + (panel[pan].element[el].last_highlight >= inter.running_time - 1)],
																							2, 4);

							add_menu_string(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + 6,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1 + 3,
																							&colours.base [button_col] [SHADE_MAX],
																							ALLEGRO_ALIGN_LEFT,
																							FONT_SQUARE,
																							panel[pan].element[el].name);

	}

 for (i = 0; i < 2; i ++)
	{

     		el = FPE_TEMPLATES_FILE_LOAD + i;

					 	add_menu_button(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																							panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x2,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2,
																							colours.base [COL_BLUE] [SHADE_LOW + (panel[pan].element[el].last_highlight >= inter.running_time - 1)],
																							2, 4);
							add_menu_string(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + 6,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1 + 3,
																							&colours.base [COL_BLUE] [SHADE_HIGH],
																							ALLEGRO_ALIGN_LEFT,
																							FONT_SQUARE,
																							panel[pan].element[el].name);
	}


 int shade;


 el = FPE_TEMPLATES_TEMPL_0 + tstate.current_template;

 					 	add_menu_button(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 - 3,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1 - 3,
																							panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x2 + 3,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2 + 3,
																							colours.base [COL_BLUE] [SHADE_HIGH],
																							8, 14);

// now draw the template buttons (but not their labels):
 for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
	{
		     		el = FPE_TEMPLATES_TEMPL_0 + i;
		     		if (!templ [tstate.template_player_tab] [i].active)
										shade = SHADE_MIN;
									  else
   		     		shade = SHADE_LOW;

   		    shade += (panel[pan].element[el].last_highlight >= inter.running_time - 1);

					 	add_menu_button(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																							panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x2,
																							panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2,
																							colours.base_trans [COL_BLUE] [shade] [TRANS_MED],
																							6, 12);

							if (templ [tstate.template_player_tab] [i].locked)
							{
								add_menu_rectangle(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + scaleUI_x(FONT_SQUARE, 240),
																			     			panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																						     panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + scaleUI_x(FONT_SQUARE, 255),
																						     panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2,
																						     colours.base [COL_BLUE] [SHADE_HIGH]);
								add_menu_rectangle(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + scaleUI_x(FONT_SQUARE, 270),
																			     			panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																						     panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + scaleUI_x(FONT_SQUARE, 285),
																						     panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2,
																						     colours.base [COL_BLUE] [SHADE_HIGH]);
							}

/*								add_menu_quad(panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + 270,
																						panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																						panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + 280,
																						panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1,
																						panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + 220,
																						panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2,
																						panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1 + 230,
																						panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y2,
																						colours.base_trans [COL_BLUE] [SHADE_MED] [TRANS_THICK]);*/

	}



 draw_menu_buttons();

 float button_x, button_y;
 struct template_struct* draw_templ;
 int template_index;
#define TEMPLATE_FILE_PATH_LENGTH 40
 char file_path_string [TEMPLATE_FILE_PATH_LENGTH];

 for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
	{
		el = FPE_TEMPLATES_TEMPL_0 + i;
		button_x = panel[pan].x1 + panel[pan].subpanel[panel[pan].element[el].subpanel].x1 + panel[pan].element[el].x1;
		button_y = panel[pan].y1 + panel[pan].subpanel[panel[pan].element[el].subpanel].y1 + panel[pan].element[el].y1;
//		if (i == TEMPLATES_PER_PLAYER - 1)
//			template_index = 0;
//		  else
//		   template_index = i+1;
		template_index = i;
		draw_templ = &templ [tstate.template_player_tab] [template_index];

		if (draw_templ->active)
		{
		 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], button_x + 8, button_y + 8, ALLEGRO_ALIGN_LEFT, "Template %i", template_index);
		 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], button_x + scaleUI_x(FONT_SQUARE,88), button_y + 8, ALLEGRO_ALIGN_LEFT, "%s", draw_templ->name);
//		 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], button_x + 28, button_y + 30, ALLEGRO_ALIGN_LEFT, "Data cost %i", draw_templ->data_cost); this isn't really all that useful
//  	int file_name_shade = SHADE_HIGH;
  	int file_name_col = COL_BLUE;
  	if (!draw_templ->source_edit->saved)
			{
    file_name_col = COL_PURPLE;
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [file_name_col] [SHADE_MAX], button_x + 26, button_y + scaleUI_y(FONT_SQUARE,24), ALLEGRO_ALIGN_RIGHT, "*");
			}
   snprintf(file_path_string, TEMPLATE_FILE_PATH_LENGTH, "%s", draw_templ->source_edit->src_file_path);
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [file_name_col] [SHADE_MAX], button_x + 28, button_y + scaleUI_y(FONT_SQUARE,24), ALLEGRO_ALIGN_LEFT, "%s", file_path_string);

		 if (draw_templ->locked)
			{
 		 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], button_x + scaleUI_x(FONT_SQUARE, 295), button_y + 30, ALLEGRO_ALIGN_RIGHT, "Locked");
			}
//			 else
//  		 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_HIGH], button_x + 295, button_y + 30, ALLEGRO_ALIGN_RIGHT, "not locked");
		}
		  else
				{
		   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MED], button_x + 8, button_y + 8, ALLEGRO_ALIGN_LEFT, "Template %i", template_index);
  		 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MED], button_x + scaleUI_x(FONT_SQUARE,88), button_y + 8, ALLEGRO_ALIGN_LEFT, "Empty");
				}


	}



}
