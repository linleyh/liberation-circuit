

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>


#include <stdio.h>
#include <string.h>
#include <math.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"

#include "g_misc.h"

#include "c_header.h"

#include "e_header.h"
#include "e_inter.h"
#include "e_help.h"
#include "i_header.h"
#include "i_input.h"
#include "i_view.h"
#include "i_buttons.h"
#include "i_display.h"
#include "m_input.h"
#include "e_log.h"
#include "m_maths.h"

#include "p_panels.h"
#include "d_draw.h"
#include "d_code.h"
#include "d_design.h"
#include "t_template.h"

#include "g_shapes.h"

struct design_window_struct dwindow;
extern struct nshape_struct nshape [NSHAPES];
extern struct dshape_struct dshape [NSHAPES]; // uses same indices as NSHAPES
extern struct vbuf_struct vbuf;
extern struct object_type_struct otype [OBJECT_TYPES];;

extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct fontstruct font [FONTS];
extern struct game_struct game;
extern struct identifierstruct identifier [IDENTIFIERS]; // used to display some information about e.g. core name
extern struct design_sub_button_struct design_sub_button [DSB_STRUCT_SIZE];
extern struct fontstruct font [FONTS];

void draw_template_members(void);
static void draw_select_box(float xa, float ya, float box_size, float line_length, float line_width, timestamp select_time, ALLEGRO_COLOR col);
static void draw_collision_box(float xa, float ya, float box_size, ALLEGRO_COLOR col);
void reset_design_window(void);
static void draw_rotation_arrows(float base_x, float base_y, float rot_x, float rot_y, float arrow_distance, ALLEGRO_COLOR box_col, ALLEGRO_COLOR arrow_col);

static void add_design_quad(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, ALLEGRO_COLOR col);
static void add_design_bquad(float xa, float ya, float xb, float yb, float corner1, float corner2, ALLEGRO_COLOR col);

static void design_help_unhighlighted(int base_x, int base_y);
static void design_help_highlight(int base_x, int base_y);
static void draw_design_help_strings_autocode(int base_x, int base_y, char* help_heading, char* help_line1, char* help_line2, char* help_line3, char* help_line4, int autocode_available);

void init_design_window(void)
{

 dwindow.templ = &templ [0] [0];

 reset_design_window();

}

void reset_design_window(void)
{

	dwindow.window_pos_x = (DESIGN_WINDOW_W / 2) - (panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].w / 2);
	dwindow.window_pos_y = (DESIGN_WINDOW_H / 2) - (panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].h / 2);

 update_slider(SLIDER_DESIGN_SCROLLBAR_H);
 update_slider(SLIDER_DESIGN_SCROLLBAR_V);

 dwindow.highlight_member = -1;
 dwindow.highlight_link = -1;

 dwindow.selected_member = -1;
 dwindow.selected_link = -1;

}

// This function draws the design window based on position of the relevant panel element
void draw_design_window(void)
{

	int base_x1 = panel[PANEL_DESIGN].x1 + panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1; // don't need to add element x1 as it's always 0
	int base_y1 = panel[PANEL_DESIGN].y1 + panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y1; // don't need to add element y1 as it's always 0
	int base_x2 = panel[PANEL_DESIGN].x1 + panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1 + panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].w - 10;
	int base_y2 = panel[PANEL_DESIGN].y1 + panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y1 + panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].h;

 int clip_x2 = base_x2;
 if (clip_x2 > panel[PANEL_DESIGN].x2)
		clip_x2 = panel[PANEL_DESIGN].x2;

// Note that x2 can be outside the panel:
 al_set_clipping_rectangle(base_x1, base_y1, base_x2 - base_x1 + SLIDER_BUTTON_SIZE, base_y2 - base_y1 + SLIDER_BUTTON_SIZE);

// al_clear_to_color(colours.base [COL_GREY] [SHADE_MIN]);
 al_clear_to_color(colours.base [COL_BLUE] [SHADE_MIN]);

// al_draw_rectangle(base_x1 + 3, base_y1 + 3, base_x2 + 93, base_y2 - 3, colours.base [COL_YELLOW] [SHADE_HIGH], 1);

	float centre_x = base_x1 + (DESIGN_WINDOW_W/2) - dwindow.window_pos_x;
	float centre_y = base_y1 + (DESIGN_WINDOW_H/2) - dwindow.window_pos_y;

	if (dwindow.templ->active == 0)
	{
		al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], centre_x, centre_y - 10, ALLEGRO_ALIGN_CENTRE, "Empty template");
 	al_set_clipping_rectangle(panel[PANEL_DESIGN].x1, panel[PANEL_DESIGN].y1, panel[PANEL_DESIGN].w, panel[PANEL_DESIGN].h);
		return;
	}

 add_line(0, base_x1, centre_y, base_x2, centre_y, colours.base [COL_BLUE] [SHADE_LOW]);
 add_line(0, centre_x, base_y1, centre_x, base_y2, colours.base [COL_BLUE] [SHADE_LOW]);
	al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MED], base_x2 - 3, centre_y + 5, ALLEGRO_ALIGN_RIGHT, "Front >>");

 if (dwindow.templ->active)
	{

#define POWER_GRAPH_X (base_x1 + scaleUI_x(FONT_BASIC, 55))
#define POWER_GRAPH_H font[FONT_BASIC].height
#define POWER_GRAPH_Y (base_y2 - POWER_GRAPH_H * 4)
//#define POWER_GRAPH_Y (base_y2 - scaleUI_y(FONT_BASIC, 30))
//scaleUI_y(FONT_BASIC, 8)
#define POWER_GRAPH_Y0 (POWER_GRAPH_Y - POWER_GRAPH_H - 2)
#define POWER_GRAPH_Y2 (POWER_GRAPH_Y + POWER_GRAPH_H + 2)
#define POWER_GRAPH_Y3 (POWER_GRAPH_Y2 + POWER_GRAPH_H + 2)
#define POWER_GRAPH_SCALE 0.4

  int i;

//	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], POWER_GRAPH_X - 2, POWER_GRAPH_Y - 12, ALLEGRO_ALIGN_RIGHT, "power");
	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], (int) POWER_GRAPH_X - 2, (int) POWER_GRAPH_Y, ALLEGRO_ALIGN_RIGHT, "power");
	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], (int) POWER_GRAPH_X - 2, (int) POWER_GRAPH_Y2, ALLEGRO_ALIGN_RIGHT, "demand");
	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], (int) POWER_GRAPH_X - 2, (int) POWER_GRAPH_Y3, ALLEGRO_ALIGN_RIGHT, "data");


 add_design_bquad(POWER_GRAPH_X,
																	 POWER_GRAPH_Y,
																	 POWER_GRAPH_X + nshape[dwindow.templ->member[0].shape].power_capacity * POWER_GRAPH_SCALE,
																	 POWER_GRAPH_Y + POWER_GRAPH_H,
																	 3,
																	 2,
																	 colours.base_trans [COL_YELLOW] [SHADE_MAX] [TRANS_MED]);

	int template_components = -1; // -1 to account for core

	for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (dwindow.templ->member[i].exists)
			template_components ++;
	}

	float component_power_x = POWER_GRAPH_X + nshape[dwindow.templ->member[0].shape].power_capacity * POWER_GRAPH_SCALE;
	float component_power_w = nshape[dwindow.templ->member[0].shape].component_power_capacity * POWER_GRAPH_SCALE;

	for (i = 0; i < template_components; i ++)
	{
		 add_design_bquad(component_power_x + i * component_power_w + 1,
																	 POWER_GRAPH_Y,
																	 component_power_x + (i+1) * component_power_w,
																	 POWER_GRAPH_Y + POWER_GRAPH_H,
																	 2,
																	 1,
																	 colours.base_trans [COL_YELLOW] [SHADE_MAX] [TRANS_MED]);

	}

	int right_text_x;

	right_text_x = component_power_x + (component_power_w * template_components) + 12;

	if (right_text_x + 100 > clip_x2)
	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], clip_x2 - 5, POWER_GRAPH_Y, ALLEGRO_ALIGN_RIGHT, "core %i total %i", nshape[dwindow.templ->member[0].shape].power_capacity, nshape[dwindow.templ->member[0].shape].power_capacity + nshape[dwindow.templ->member[0].shape].component_power_capacity * template_components);
	  else
  	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_YELLOW] [SHADE_LOW], right_text_x, POWER_GRAPH_Y, ALLEGRO_ALIGN_LEFT, "core %i total %i", nshape[dwindow.templ->member[0].shape].power_capacity, nshape[dwindow.templ->member[0].shape].power_capacity + nshape[dwindow.templ->member[0].shape].component_power_capacity * template_components);

																	 int power_use_base_x = POWER_GRAPH_X + dwindow.templ->power_use_base * POWER_GRAPH_SCALE;
																	 int power_use_peak_x = POWER_GRAPH_X + dwindow.templ->power_use_peak * POWER_GRAPH_SCALE;


if (dwindow.templ->power_use_peak > 0)
{
add_design_bquad(POWER_GRAPH_X,
																	 POWER_GRAPH_Y2,
																	 POWER_GRAPH_X + dwindow.templ->power_use_peak * POWER_GRAPH_SCALE,
																	 POWER_GRAPH_Y2 + POWER_GRAPH_H,
																	 2,
																	 2,
																	 colours.base_trans [COL_RED] [SHADE_MAX] [TRANS_MED]);
}


if (dwindow.templ->power_use_base > 0)
{
add_design_bquad(POWER_GRAPH_X,
																	 POWER_GRAPH_Y2,
																	 power_use_base_x,
																	 POWER_GRAPH_Y2 + POWER_GRAPH_H,
																	 2,
																	 2,
																	 colours.base_trans [COL_ORANGE] [SHADE_MAX] [TRANS_MED]);
}

	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_LOW], (int) power_use_peak_x + 12, (int) POWER_GRAPH_Y2, ALLEGRO_ALIGN_LEFT, "base %i peak %i", dwindow.templ->power_use_base, dwindow.templ->power_use_peak);

  int data_cost_x = POWER_GRAPH_X + dwindow.templ->data_cost * POWER_GRAPH_SCALE;


if (dwindow.templ->data_cost > 0)
{
add_design_bquad(POWER_GRAPH_X,
																	 POWER_GRAPH_Y3,
																	 data_cost_x,
																	 POWER_GRAPH_Y3 + POWER_GRAPH_H,
																	 2,
																	 3,
																	 colours.base_trans [COL_BLUE] [SHADE_MAX] [TRANS_MED]);
}

	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MED], (int) data_cost_x + 12, (int) POWER_GRAPH_Y3, ALLEGRO_ALIGN_LEFT, "cost %i", dwindow.templ->data_cost);


  int possible_graph_y = POWER_GRAPH_Y0;


//  if (dwindow.templ->number_of_interface_objects > 0)
		{
// the word "interface" is only displayed if the template actually has an interface
	  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], (int) POWER_GRAPH_X - 2, (int) possible_graph_y, ALLEGRO_ALIGN_RIGHT, "%%power");

	  int power_ratio = 100;
	  if (dwindow.templ->power_use_peak != 0)
	   power_ratio = (dwindow.templ->power_capacity * 100) / dwindow.templ->power_use_peak;
	  float power_ratio_x = POWER_GRAPH_X + power_ratio;// * 0.1;
	  float power_ratio_x_100 = POWER_GRAPH_X + 100;

	  int power_ratio_text_x = power_ratio_x + 12;
	  if (power_ratio < 100)
				power_ratio_text_x = POWER_GRAPH_X + 112;

	  int ratio_bar_colour = COL_BLUE;


			if (power_ratio < 30)
			{
 	  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], power_ratio_text_x, possible_graph_y, ALLEGRO_ALIGN_LEFT, "%i%% - SEVERELY UNDERPOWERED", power_ratio);
 	  ratio_bar_colour = COL_RED;
			}
   else
			if (power_ratio < 50)
			{
 	  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], power_ratio_text_x, possible_graph_y, ALLEGRO_ALIGN_LEFT, "%i%% - very underpowered", power_ratio);
 	  ratio_bar_colour = COL_RED;
			}
   else
			if (power_ratio < 70)
			{
 	  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], power_ratio_text_x, possible_graph_y, ALLEGRO_ALIGN_LEFT, "%i%% - underpowered", power_ratio);
 	  ratio_bar_colour = COL_YELLOW;
			}
   else
			if (power_ratio < 85)
			{
 	  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], power_ratio_text_x, possible_graph_y, ALLEGRO_ALIGN_LEFT, "%i%% - slightly underpowered", power_ratio);
			}
   else
			if (power_ratio < 120)
			{
 	  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], power_ratio_text_x, possible_graph_y, ALLEGRO_ALIGN_LEFT, "%i%% - powered", power_ratio);
			}
   else
			if (power_ratio < 150)
			{
 	  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], power_ratio_text_x, possible_graph_y, ALLEGRO_ALIGN_LEFT, "%i%% - overpowered", power_ratio);
			}
   else
			{
 	  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_PURPLE] [SHADE_HIGH], power_ratio_text_x, possible_graph_y, ALLEGRO_ALIGN_LEFT, "%i%% - very overpowered", power_ratio);
 	  ratio_bar_colour = COL_PURPLE;
			}


   add_design_quad(power_ratio_x_100,
																	  possible_graph_y - 1,
                   power_ratio_x_100 + 1,
																	  possible_graph_y - 1,
                   power_ratio_x_100 + 1,
																	  possible_graph_y + POWER_GRAPH_H + 1,
																	  power_ratio_x_100,
																	  possible_graph_y + POWER_GRAPH_H + 1,
																	  colours.base_trans [COL_GREY] [SHADE_MED] [TRANS_MED]);

   add_design_bquad(POWER_GRAPH_X,
																	   possible_graph_y,
																	   power_ratio_x,
																	   possible_graph_y + POWER_GRAPH_H,
																	   3,
																	   2,
																	   colours.base_trans [ratio_bar_colour] [SHADE_MAX] [TRANS_MED]);


   possible_graph_y -= (POWER_GRAPH_H + 2);

		}





  if (dwindow.templ->number_of_interface_objects > 0)
		{
// the word "interface" is only displayed if the template actually has an interface
	  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], (int) POWER_GRAPH_X - 2, (int) possible_graph_y, ALLEGRO_ALIGN_RIGHT, "interface");

	  int interface_strength = dwindow.templ->number_of_interface_objects * nshape[dwindow.templ->member[0].shape].base_hp_max;
	  float interface_strength_x = POWER_GRAPH_X + interface_strength * 0.1;

   add_design_bquad(POWER_GRAPH_X,
																	   possible_graph_y,
																	   interface_strength_x,
																	   possible_graph_y + POWER_GRAPH_H,
																	   3,
																	   2,
																	   colours.base_trans [COL_PURPLE] [SHADE_MAX] [TRANS_MED]);

 	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_PURPLE] [SHADE_HIGH], (int) (interface_strength_x + 12), (int) possible_graph_y, ALLEGRO_ALIGN_LEFT, "interface %i charge +%i", interface_strength, dwindow.templ->number_of_interface_objects * nshape[dwindow.templ->member[0].shape].interface_charge_rate);

   possible_graph_y -= (POWER_GRAPH_H + 2);

		}


  if (dwindow.templ->number_of_storage_objects > 0)
		{
// only displayed if the template actually has storage
	  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], (int) (POWER_GRAPH_X - 2), (int) possible_graph_y, ALLEGRO_ALIGN_RIGHT, "storage");

	  int data_storage = dwindow.templ->number_of_storage_objects * 64;
	  int data_storage_x = POWER_GRAPH_X + data_storage * 0.3;

   add_design_bquad(POWER_GRAPH_X,
																	   possible_graph_y,
																	   data_storage_x,
																	   possible_graph_y + POWER_GRAPH_H,
																	   3,
																	   2,
																	   colours.base_trans [COL_ORANGE] [SHADE_MAX] [TRANS_MED]);

 	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], (int) (data_storage_x + 12), (int) possible_graph_y, ALLEGRO_ALIGN_LEFT, "data storage %i", data_storage);

   possible_graph_y -= (POWER_GRAPH_H + 2);

		}



//	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_MED], power_use_base_x, POWER_GRAPH_Y3, ALLEGRO_ALIGN_RIGHT, "base %i", dwindow.templ->power_use_base);
//	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_RED] [SHADE_MED], power_use_peak_x, POWER_GRAPH_Y3, ALLEGRO_ALIGN_LEFT, "peak %i", dwindow.templ->power_use_peak);

/*
	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], base_x2 - 4, base_y2 - 26, ALLEGRO_ALIGN_RIGHT, "data cost %i", dwindow.templ->data_cost);
	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], base_x2 - 4, base_y2 - 13, ALLEGRO_ALIGN_RIGHT, "power: capacity %i peak %i base %i", dwindow.templ->power_capacity, dwindow.templ->power_use_peak, dwindow.templ->power_use_base);*/
//  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], panel[PANEL_DESIGN].x1 + 140, panel[PANEL_DESIGN].y1+15, ALLEGRO_ALIGN_LEFT, "Power: capacity %i  use: peak %i average %i base %i", nshape[dwindow.templ->member[0].shape].power_capacity, dwindow.templ->power_use_peak, dwindow.templ->power_use_smoothed, dwindow.templ->power_use_base);
  if (dwindow.templ->modified)
		{
 	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], base_x2 - 4, base_y1 + scaleUI_y(FONT_BASIC, 3), ALLEGRO_ALIGN_RIGHT, "This template has been modified.");
 	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], base_x2 - 4, base_y1 + scaleUI_y(FONT_BASIC, 16), ALLEGRO_ALIGN_RIGHT, "Use <Write Header> or <Autocode>");
 	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], base_x2 - 4, base_y1 + scaleUI_y(FONT_BASIC, 29), ALLEGRO_ALIGN_RIGHT, "to keep changes.");
 	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], base_x2 - 4, base_y1 + scaleUI_y(FONT_BASIC, 42), ALLEGRO_ALIGN_RIGHT, "(Save changes to disk using the");
 	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], base_x2 - 4, base_y1 + scaleUI_y(FONT_BASIC, 55), ALLEGRO_ALIGN_RIGHT, "File menu in the Editor [Ed] panel)");

/*
 	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], base_x1 + 4, base_y2 - 39, ALLEGRO_ALIGN_LEFT, "This template has been modified.");
 	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], base_x1 + 4, base_y2 - 26, ALLEGRO_ALIGN_LEFT, "Use <Write Header> or <Autocode>");
 	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], base_x1 + 4, base_y2 - 13, ALLEGRO_ALIGN_LEFT, "to keep changes.");*/
		}

	}


 draw_vbuf();


 draw_template_members();

 draw_vbuf();

// reset clipping rectangle to draw rest of the panel
	al_set_clipping_rectangle(panel[PANEL_DESIGN].x1, panel[PANEL_DESIGN].y1, panel[PANEL_DESIGN].w, panel[PANEL_DESIGN].h);


}


void draw_template_members(void)
{

	int m,i;
	float base_x = panel[PANEL_DESIGN].x1 + panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1 + (DESIGN_WINDOW_W/2) - dwindow.window_pos_x;
	float base_y = panel[PANEL_DESIGN].y1 + panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y1 + (DESIGN_WINDOW_H/2) - dwindow.window_pos_y;
	float x, y;
	struct template_member_struct* mem;


	struct nshape_struct* nsh;


	for (m = 0; m < GROUP_MAX_MEMBERS; m++)
	{
		if (dwindow.templ->member[m].exists == 0)
			continue;
		mem = &dwindow.templ->member[m];

		x = base_x + al_fixtof(mem->position.x);
		y = base_y + al_fixtof(mem->position.y);
		if (dwindow.highlight_member == m
			&&	control.mouse_panel == PANEL_DESIGN) // don't draw highlight if mouse not in panel)
		{

			ALLEGRO_COLOR highlight_col = al_map_rgba(100,
																																												150,
																																												200,
																																												30);


   draw_proc_outline(x, y, dwindow.templ->member[dwindow.highlight_member].group_angle_offset, dwindow.templ->member[dwindow.highlight_member].shape, 1.3, 0, highlight_col, highlight_col, 1);

/*
   draw_select_box(x,
																			y,
																			30, // box size
																		 5, // line length
																		 2, // line_width
																		 inter.running_time,
																		 colours.base_trans [COL_YELLOW] [SHADE_MAX] [TRANS_THICK]);*/
		}
		if (dwindow.selected_member == m)
		{
			float select_time_adjust = inter.running_time - dwindow.select_member_timestamp;
			if (select_time_adjust > 16)
				select_time_adjust = 16;
			select_time_adjust = 16 - select_time_adjust;
			select_time_adjust += (32 - ((inter.running_time - dwindow.select_member_timestamp) % 32)) * 0.2;

			ALLEGRO_COLOR selected_col = al_map_rgba(50 + select_time_adjust,
																																												80 + select_time_adjust + (dwindow.selected_link	== -1) * 100,
																																												200 + select_time_adjust,
																																												40 + select_time_adjust);
			float selected_scale = 1.3 + select_time_adjust * 0.005;


   draw_proc_outline(x, y, dwindow.templ->member[dwindow.selected_member].group_angle_offset, dwindow.templ->member[dwindow.selected_member].shape, selected_scale, 0, selected_col, colours.none, 1);
/*   draw_select_box(x,
																			y,
																			39, // box size
																		 7, // line length
																		 3, // line_width
																		 dwindow.select_member_timestamp,
																		 colours.base_trans [COL_GREY] [SHADE_MAX] [TRANS_THICK]);*/

			if (dwindow.selected_link	== -1)
			{
			 select_time_adjust = inter.running_time - dwindow.select_member_timestamp;
			 if (select_time_adjust > 16)
				 select_time_adjust = 16;
				select_time_adjust = (16 - select_time_adjust) * 2;

				int highlight_adjust = (dwindow.highlight_rotation_time >= inter.running_time) * 30 + select_time_adjust;

			 ALLEGRO_COLOR highlight_col = map_rgba(160 + highlight_adjust,
																																										 		 160 + highlight_adjust,
																																											 	 100 + highlight_adjust,
																																												  150 + highlight_adjust);

			 ALLEGRO_COLOR highlight_col2 = map_rgba(180 + highlight_adjust,
																																										 		 130 + highlight_adjust,
																																											 	 80 + highlight_adjust,
																																												  190 + highlight_adjust * 2);

				float arrow_dist;

				if (control.mouse_drag == MOUSE_DRAG_DESIGN_MEMBER)
					arrow_dist = -5;
 				 else
							arrow_dist = -1;

    draw_rotation_arrows(x,
																									y,
																									base_x + dwindow.member_rotation_x,
																									base_y + dwindow.member_rotation_y,
																									arrow_dist,
																									highlight_col, highlight_col2);
/*
    draw_select_box(base_x + dwindow.member_rotation_x,
																			 base_y + dwindow.member_rotation_y,
																			 13, // box size
																		  7, // line length
																		  3, // line_width
																		  dwindow.select_member_timestamp,
																		  highlight_col);*/
			}

//			float rotation_x = x
//			for (i = 0; i < nshape[mem->shape].links; i ++)
		}

		if (mem->move_obstruction)
   draw_collision_box(x, y, MOVE_OBSTRUCTION_BOX_SIZE, colours.base_trans [COL_RED] [SHADE_HIGH] [TRANS_MED]);
    else
     draw_collision_box(x, y, MOVE_OBSTRUCTION_BOX_SIZE, colours.base_trans [COL_GREY] [SHADE_MED] [TRANS_FAINT]);


  int design_colour [2] = {PLAN_COL_DESIGN_BASIC, PLAN_COL_DESIGN_ERROR};
  if (dwindow.templ->locked)
			design_colour [0] = PLAN_COL_DESIGN_LOCKED; // should be able to assume that mem->collision and mem->move_obstruction will always be 0 for a locked template
  if (dwindow.templ->modified)
			design_colour [0] = PLAN_COL_DESIGN_MODIFIED;

  draw_proc_shape(x, y, mem->group_angle_offset,// - AFX_ANGLE_4,
																	mem->shape,
																	0, // player_index
																	1, // zoom
															  colours.plan_col [design_colour [((mem->collision | mem->move_obstruction | mem->story_lock_failure) != 0)]]);


   for (i = 0; i < MAX_LINKS; i ++)
			{
				switch(mem->object[i].type)
				{
				 case OBJECT_TYPE_UPLINK: // note downlink isn't drawn
			   draw_link_shape(x, y,
																			   mem->group_angle_offset,// - AFX_ANGLE_4,
																	     mem->shape,
																	     i,
																			   base_x + al_fixtof(dwindow.templ->member[mem->connection[0].template_member_index].position.x),
																			   base_y + al_fixtof(dwindow.templ->member[mem->connection[0].template_member_index].position.y),
																			   dwindow.templ->member[mem->connection[0].template_member_index].group_angle_offset,// - AFX_ANGLE_4,
																			   dwindow.templ->member[mem->connection[0].template_member_index].shape,
																	     mem->connection[0].reverse_link_index,
																	     colours.plan_col [design_colour [((mem->collision | mem->move_obstruction | mem->story_lock_failure) != 0)]] [PROC_COL_LINK],
//																			   proc_col [(mem->collision | mem->move_obstruction | mem->object[i].template_error) != 0] [1] [0],
//																			   proc_col [(mem->collision | mem->move_obstruction | mem->object[i].template_error) != 0] [1] [1],
																			   1); // last number is zoom
						break;

					case OBJECT_TYPE_MOVE:
						{
							float object_x = x + al_fixtoi(fixed_cos(mem->group_angle_offset + nshape[mem->shape].object_angle_fixed [i]) * nshape[mem->shape].object_dist_pixel [i]) + al_fixtoi(fixed_xpart(mem->group_angle_offset + nshape[mem->shape].object_angle_fixed [i] + mem->object[i].base_angle_offset, al_itofix(10)));
							float object_y = y + al_fixtoi(fixed_sin(mem->group_angle_offset + nshape[mem->shape].object_angle_fixed [i]) * nshape[mem->shape].object_dist_pixel [i]) + al_fixtoi(fixed_ypart(mem->group_angle_offset + nshape[mem->shape].object_angle_fixed [i] + mem->object[i].base_angle_offset, al_itofix(10)));
       add_line(4,
					     				  object_x,
     									  object_y,
									       object_x + al_fixtoi(fixed_xpart(mem->group_angle_offset + nshape[mem->shape].object_angle_fixed [i] + mem->object[i].base_angle_offset, al_itofix(400))),
									       object_y + al_fixtoi(fixed_ypart(mem->group_angle_offset + nshape[mem->shape].object_angle_fixed [i] + mem->object[i].base_angle_offset, al_itofix(400))),
										      colours.base_trans [COL_RED] [SHADE_LOW] [TRANS_MED]);
						}
// fall-through

     case OBJECT_TYPE_PULSE:
     case OBJECT_TYPE_PULSE_L:
     case OBJECT_TYPE_PULSE_XL:
     case OBJECT_TYPE_BURST:
     case OBJECT_TYPE_BURST_L:
     case OBJECT_TYPE_BURST_XL:
     case OBJECT_TYPE_SLICE:
     case OBJECT_TYPE_ULTRA:
     case OBJECT_TYPE_ULTRA_DIR:
     case OBJECT_TYPE_STREAM:
     case OBJECT_TYPE_STREAM_DIR:
						{
							float object_x = x + al_fixtoi(fixed_cos(mem->group_angle_offset + nshape[mem->shape].object_angle_fixed [i]) * nshape[mem->shape].object_dist_pixel [i]) + al_fixtoi(fixed_xpart(mem->group_angle_offset + nshape[mem->shape].object_angle_fixed [i] + mem->object[i].base_angle_offset, al_itofix(10)));
							float object_y = y + al_fixtoi(fixed_sin(mem->group_angle_offset + nshape[mem->shape].object_angle_fixed [i]) * nshape[mem->shape].object_dist_pixel [i]) + al_fixtoi(fixed_ypart(mem->group_angle_offset + nshape[mem->shape].object_angle_fixed [i] + mem->object[i].base_angle_offset, al_itofix(10)));
       add_line(4,
					     				  object_x,
     									  object_y,
									       object_x + al_fixtoi(fixed_xpart(mem->group_angle_offset + nshape[mem->shape].object_angle_fixed [i] + mem->object[i].base_angle_offset, al_itofix(400))),
									       object_y + al_fixtoi(fixed_ypart(mem->group_angle_offset + nshape[mem->shape].object_angle_fixed [i] + mem->object[i].base_angle_offset, al_itofix(400))),
										      colours.base_trans [COL_AQUA] [SHADE_HIGH] [TRANS_FAINT]);
						}
					default:
      draw_object(x, y,
																					mem->group_angle_offset,
																					mem->shape,
																					&mem->object[i], // uses same code as drawing objects for the main display
																					NULL, // no object instance
																					NULL, // no core
																					NULL, // no proc
																					i,
																					colours.plan_col [design_colour [((mem->collision | mem->move_obstruction | (mem->object[i].template_error != 0)) != 0)]],
//																			  colours.proc_col [(mem->collision | mem->move_obstruction | mem->object[i].template_error) != 0] [PROC_DAMAGE_COLS - 1],
//																			  proc_col [(mem->collision | mem->move_obstruction | mem->object[i].template_error) != 0] [1] [0],
//																			  proc_col [(mem->collision | mem->move_obstruction | mem->object[i].template_error) != 0] [2] [1],
																			  1); // last number is zoom
/*
      draw_object_base_shape(x, y,
																					mem->group_angle_offset,
																					mem->shape,
																					i,
																			  colours.base [col] [SHADE_MED],
																			  colours.base [col] [SHADE_LOW],
																			  1); // last number is zoom*/
						break;


				}
/*
 - use this code to work out the correct position for a shape's links
				add_line(3, x, y,
													x + (cos(fixed_to_radians(mem->group_angle_offset) + fixed_to_radians(nshape[mem->shape].link_angle_fixed [i])) * al_fixtof(nshape[mem->shape].link_dist_fixed [i])),
													y + (sin(fixed_to_radians(mem->group_angle_offset) + fixed_to_radians(nshape[mem->shape].link_angle_fixed [i])) * al_fixtof(nshape[mem->shape].link_dist_fixed [i])),
													colours.base [COL_YELLOW] [SHADE_MAX]);*/
			}

/*
	 if (mem->connection[0].template_member_index != -1)
	 {
			draw_link_shape(x, y,
																			mem->group_angle_offset,// - AFX_ANGLE_4,
																	  mem->shape,
																	  mem->connection[0].link_index,
																			base_x + al_fixtof(dwindow.templ->member[mem->connection[0].template_member_index].position.x),
																			base_y + al_fixtof(dwindow.templ->member[mem->connection[0].template_member_index].position.y),
																			dwindow.templ->member[mem->connection[0].template_member_index].group_angle_offset,// - AFX_ANGLE_4,
																			dwindow.templ->member[mem->connection[0].template_member_index].shape,
																	  mem->connection[0].reverse_link_index,
																			colours.base [col] [SHADE_MED],
																			colours.base [col] [SHADE_LOW],
																			1); // last number is zoom


	 }*/
/*
 add_line(2,
									 x + al_fixtof(fixed_xpart(mem->group_angle_offset, al_itofix(15))),
									 y + al_fixtof(fixed_ypart(mem->group_angle_offset, al_itofix(15))),
									 x + al_fixtof(fixed_xpart(mem->group_angle_offset, al_itofix(25))),
									 y + al_fixtof(fixed_ypart(mem->group_angle_offset, al_itofix(25))),
										colours.base [COL_GREY] [SHADE_MED]);*/
/*									 x + al_fixtof(fixed_xpart(mem->group_angle_offset - AFX_ANGLE_4, al_itofix(15))),
									 y + al_fixtof(fixed_ypart(mem->group_angle_offset - AFX_ANGLE_4, al_itofix(15))),
									 x + al_fixtof(fixed_xpart(mem->group_angle_offset - AFX_ANGLE_4, al_itofix(25))),
									 y + al_fixtof(fixed_ypart(mem->group_angle_offset - AFX_ANGLE_4, al_itofix(25))),
										colours.base [COL_GREY] [SHADE_MED]);*/


		if (control.mouse_panel == PANEL_DESIGN // don't draw highlight if mouse not in panel
			&&	dwindow.highlight_member == m
			&& dwindow.highlight_link	!= -1)
		{

			ALLEGRO_COLOR highlight_col = al_map_rgba(100,
																																												150,
																																												200,
																																												70);



#define VERTEX_BOX_SIZE 10
   draw_select_box(base_x - (DESIGN_WINDOW_W/2) + dwindow.highlight_link_x,
																			base_y - (DESIGN_WINDOW_H/2) + dwindow.highlight_link_y,
																			14, // box size
																		 2, // line length
																		 1, // line_width
																		 inter.running_time,
																		 highlight_col);


		}
		if (dwindow.selected_member == m
			&& dwindow.selected_link	!= -1)
		{

			float select_time_adjust = inter.running_time - dwindow.select_link_timestamp;
			if (select_time_adjust > 16)
				select_time_adjust = 16;
			select_time_adjust = 16 - select_time_adjust;
			select_time_adjust += (32 - ((inter.running_time - dwindow.select_member_timestamp) % 32)) * 0.2; // note - uses select_member_timestamp rather than select_link_timestamp

			ALLEGRO_COLOR selected_col = al_map_rgba(50 + select_time_adjust,
																																												190 + select_time_adjust,
																																												210 + select_time_adjust,
																																												50 + select_time_adjust);
			float selected_scale = 1.3 + select_time_adjust * 0.01;

			nsh = &nshape [dwindow.templ->member[m].shape];

   draw_select_box(x + al_fixtoi(fixed_cos(dwindow.templ->member[m].group_angle_offset + nsh->object_angle_fixed [dwindow.selected_link]) * nsh->object_dist_pixel [dwindow.selected_link]),
																			y + al_fixtoi(fixed_sin(dwindow.templ->member[m].group_angle_offset + nsh->object_angle_fixed [dwindow.selected_link]) * nsh->object_dist_pixel [dwindow.selected_link]),
																		 11 * selected_scale, // box size
																		 3, // line length
																		 2, // line_width
																		 dwindow.select_link_timestamp,
																		 selected_col);

/*
   draw_select_box(x + al_fixtoi(fixed_cos(dwindow.templ->member[m].group_angle_offset + nsh->object_angle_fixed [dwindow.selected_link]) * nsh->object_dist_pixel [dwindow.selected_link]),
																			y + al_fixtoi(fixed_sin(dwindow.templ->member[m].group_angle_offset + nsh->object_angle_fixed [dwindow.selected_link]) * nsh->object_dist_pixel [dwindow.selected_link]),
																		 13, // box size
																		 3, // line length
																		 2, // line_width
																		 dwindow.select_link_timestamp,
																		 colours.base_trans [COL_GREY] [SHADE_MAX] [TRANS_THICK]);
*/

    int object_type = dwindow.templ->member[dwindow.selected_member].object [dwindow.selected_link].type;

    if (!otype[object_type].object_details.only_zero_angle_offset // can't rotate this kind of object
						&& object_type != OBJECT_TYPE_UPLINK
						&& object_type != OBJECT_TYPE_DOWNLINK)
				{

			  select_time_adjust = inter.running_time - dwindow.select_member_timestamp;
			  if (select_time_adjust > 16)
				  select_time_adjust = 16;
				 select_time_adjust = (16 - select_time_adjust) * 2;

				 int highlight_adjust = (dwindow.highlight_rotation_time >= inter.running_time) * 30 + select_time_adjust;

			  ALLEGRO_COLOR highlight_col = map_rgba(160 + highlight_adjust,
																																										 		 60 + highlight_adjust,
																																											 	 30 + highlight_adjust,
																																												  150 + highlight_adjust);

			  ALLEGRO_COLOR highlight_col2 = map_rgba(190 + highlight_adjust,
																																										 		 70 + highlight_adjust,
																																											 	 40 + highlight_adjust,
																																												  190 + highlight_adjust * 2);

				 float arrow_dist;

				 if (control.mouse_drag == MOUSE_DRAG_DESIGN_OBJECT)
					 arrow_dist = -5;
 				  else
							 arrow_dist = -1;

     draw_rotation_arrows(
x + al_fixtoi(fixed_cos(dwindow.templ->member[m].group_angle_offset + nsh->object_angle_fixed [dwindow.selected_link]) * nsh->object_dist_pixel [dwindow.selected_link]),
																			y + al_fixtoi(fixed_sin(dwindow.templ->member[m].group_angle_offset + nsh->object_angle_fixed [dwindow.selected_link]) * nsh->object_dist_pixel [dwindow.selected_link]),
//																										x,
//																									 y,
																									 base_x + dwindow.link_rotation_x,
																									 base_y + dwindow.link_rotation_y,
																									 arrow_dist,
																									 highlight_col, highlight_col2);
				}

/*
    draw_select_box(base_x + dwindow.link_rotation_x,
																			 base_y + dwindow.link_rotation_y,
																			 13, // box size
																		  7, // line length
																		  3, // line_width
																		  dwindow.select_link_timestamp,
																		  colours.base_trans [COL_RED] [SHADE_MAX] [TRANS_THICK]);
																		  */

		}

	}

}

static void draw_rotation_arrows(float base_x, float base_y, float rot_x, float rot_y, float arrow_distance, ALLEGRO_COLOR box_col, ALLEGRO_COLOR arrow_col)
{

 float box_size = 13;

 add_design_bquad(rot_x - box_size,
																	 rot_y - box_size,
																	 rot_x + box_size,
																	 rot_y + box_size,
																	 5,
																	 3,
																	 box_col);


	float base_angle = atan2(rot_y - base_y, rot_x - base_x);

	float arrow_angle = base_angle - PI/2;

	float arrow_x = rot_x + cos(arrow_angle) * arrow_distance;
	float arrow_y = rot_y + sin(arrow_angle) * arrow_distance;

 add_design_quad(arrow_x + cos(arrow_angle) * 12,
																	arrow_y + sin(arrow_angle) * 12,
																	arrow_x + cos(arrow_angle - 0.5) * 11,
																	arrow_y + sin(arrow_angle - 0.5) * 11,
																	arrow_x + cos(arrow_angle) * 23,
																	arrow_y + sin(arrow_angle) * 23,
																	arrow_x + cos(arrow_angle + 0.5) * 11,
																	arrow_y + sin(arrow_angle + 0.5) * 11,
																	arrow_col);

arrow_angle = base_angle + PI/2;

	arrow_x = rot_x + cos(arrow_angle) * arrow_distance;
	arrow_y = rot_y + sin(arrow_angle) * arrow_distance;

 add_design_quad(arrow_x + cos(arrow_angle) * 12,
																	arrow_y + sin(arrow_angle) * 12,
																	arrow_x + cos(arrow_angle - 0.5) * 11,
																	arrow_y + sin(arrow_angle - 0.5) * 11,
																	arrow_x + cos(arrow_angle) * 23,
																	arrow_y + sin(arrow_angle) * 23,
																	arrow_x + cos(arrow_angle + 0.5) * 11,
																	arrow_y + sin(arrow_angle + 0.5) * 11,
																	arrow_col);



}

static void draw_select_box(float xa, float ya, float box_size, float line_length, float line_width, timestamp select_time, ALLEGRO_COLOR col)
{

 add_design_bquad(xa - box_size,
																	 ya - box_size,
																	 xa + box_size,
																	 ya + box_size,
																	 2,
																	 5,
																	 col);
return;
 add_design_quad(xa - box_size,
																	ya - box_size,
																	xa + box_size,
																	ya - box_size,
																	xa + box_size,
																	ya + box_size,
																	xa - box_size,
																	ya + box_size,
																	col);

return;


 float draw_angle = inter.running_time * 0.015;

 if (select_time > inter.running_time - 8)
		box_size += (8 - (inter.running_time - select_time)) * 0.2;

	int i;
	float bit_angle;

	for (i = 0; i < 4; i ++)
	{
		bit_angle = draw_angle + (PI / 2) * i;
 add_design_quad(xa + cos(bit_angle - 0.3) * box_size,
																	ya + sin(bit_angle - 0.3) * box_size,
																	xa + cos(bit_angle - 0.3) * (box_size + line_width),
																	ya + sin(bit_angle - 0.3) * (box_size + line_width),
																	xa + cos(bit_angle + 0.3) * (box_size + line_width),
																	ya + sin(bit_angle + 0.3) * (box_size + line_width),
																	xa + cos(bit_angle + 0.3) * box_size,
																	ya + sin(bit_angle + 0.3) * box_size,
																	col);

	}
/*
 add_design_quad(xa - box_size - line_length, // bottom outer
																	ya - box_size + line_length,
																	xa - box_size - line_length + line_width, // bottom inner
																	ya - box_size + line_length + line_width,
																	xa - box_size + line_length + line_width,
																	ya - box_size - line_length + line_width,
																	xa - box_size + line_length,
																	ya - box_size - line_length,
																	col);

 add_design_quad(xa - box_size - line_length, // bottom outer
																	ya + box_size - line_length,
																	xa - box_size - line_length + line_width, // bottom inner
																	ya + box_size - line_length - line_width,
																	xa - box_size + line_length + line_width,
																	ya + box_size + line_length - line_width,
																	xa - box_size + line_length,
																	ya + box_size + line_length,
																	col);

 add_design_quad(xa + box_size - line_length, // bottom outer
																	ya + box_size + line_length,
																	xa + box_size - line_length - line_width, // bottom inner
																	ya + box_size + line_length - line_width,
																	xa + box_size + line_length - line_width,
																	ya + box_size - line_length - line_width,
																	xa + box_size + line_length,
																	ya + box_size - line_length,
																	col);

 add_design_quad(xa + box_size - line_length, // bottom outer
																	ya - box_size - line_length,
																	xa + box_size - line_length - line_width, // bottom inner
																	ya - box_size - line_length + line_width,
																	xa + box_size + line_length - line_width,
																	ya - box_size + line_length + line_width,
																	xa + box_size + line_length,
																	ya - box_size + line_length,
																	col);
*/
/*
 xa -= 0.5;
 ya -= 0.5;

// top left
			add_line(2,
												xa - box_size + line_length,
												ya - box_size,
												xa - box_size,
												ya - box_size,
												col);
			add_line(2,
												xa - box_size,
												ya - box_size,
												xa - box_size,
												ya - box_size + line_length,
												col);

// top right
			add_line(2,
												xa + box_size - line_length,
												ya - box_size,
												xa + box_size,
												ya - box_size,
												col);
			add_line(2,
												xa + box_size,
												ya - box_size,
												xa + box_size,
												ya - box_size + line_length,
												col);

// bottom left
			add_line(2,
												xa - box_size + line_length,
												ya + box_size,
												xa - box_size,
												ya + box_size,
												col);
			add_line(2,
												xa - box_size,
												ya + box_size,
												xa - box_size,
												ya + box_size - line_length,
												col);

// bottom right
			add_line(2,
												xa + box_size - line_length,
												ya + box_size,
												xa + box_size,
												ya + box_size,
												col);
			add_line(2,
												xa + box_size,
												ya + box_size,
												xa + box_size,
												ya + box_size - line_length,
												col);
*/
}

static void add_design_quad(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, ALLEGRO_COLOR col)
{

#define DESIGN_QUAD_LAYER 4

	int m = vbuf.vertex_pos_triangle;

	add_tri_vertex(xa, ya, col);
	add_tri_vertex(xb, yb, col);
	add_tri_vertex(xc, yc, col);
	add_tri_vertex(xd, yd, col);

	construct_triangle(DESIGN_QUAD_LAYER, m, m+1, m+2);
	construct_triangle(DESIGN_QUAD_LAYER, m+2, m+3, m);

}


static void add_design_bquad(float xa, float ya, float xb, float yb, float corner1, float corner2, ALLEGRO_COLOR col)
{

#define DESIGN_QUAD_LAYER 4

	int m = vbuf.vertex_pos_triangle;

	add_tri_vertex(xa, ya + corner1, col);
	add_tri_vertex(xa + corner1, ya, col);
	add_tri_vertex(xb - corner2, ya, col);
	add_tri_vertex(xb, ya + corner2, col);
	add_tri_vertex(xb, yb - corner1, col);
	add_tri_vertex(xb - corner1, yb, col);
	add_tri_vertex(xa + corner2, yb, col);
	add_tri_vertex(xa, yb - corner2, col);

	construct_triangle(DESIGN_QUAD_LAYER, m, m+1, m+7);
	construct_triangle(DESIGN_QUAD_LAYER, m+1, m+2, m+7);
	construct_triangle(DESIGN_QUAD_LAYER, m+2, m+6, m+7);
	construct_triangle(DESIGN_QUAD_LAYER, m+2, m+3, m+6);
	construct_triangle(DESIGN_QUAD_LAYER, m+3, m+5, m+6);
	construct_triangle(DESIGN_QUAD_LAYER, m+3, m+4, m+5);

}

static void draw_collision_box(float xa, float ya, float box_size, ALLEGRO_COLOR col)
{

	int m = vbuf.vertex_pos_line;
	add_line_vertex(xa - box_size, ya - box_size, col);
	add_line_vertex(xa + box_size, ya - box_size, col);
	add_line_vertex(xa + box_size, ya + box_size, col);
	add_line_vertex(xa - box_size, ya + box_size, col);

	construct_line(0, m, m+1);
	construct_line(0, m+1, m+2);
	construct_line(0, m+2, m+3);
	construct_line(0, m+3, m);

}

void draw_design_data(void)
{

	int base_x = panel[PANEL_DESIGN].x1 + panel[PANEL_DESIGN].subpanel[FSP_DESIGN_DATA].x1 + 10;
	int base_y = panel[PANEL_DESIGN].y1 + panel[PANEL_DESIGN].subpanel[FSP_DESIGN_DATA].y1 + 3;

#define LOWER_HELP_LINE lower_help_line

// float lower_help_line = panel[PANEL_DESIGN].y1 + panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y2 - 42;
 float lower_help_line = panel[PANEL_LOG].y1 - scaleUI_y(FONT_SQUARE,14);

	switch(dwindow.tools_open)
	{
		case FSP_DESIGN_TOOLS_EMPTY:
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, base_y+2, ALLEGRO_ALIGN_LEFT, "Empty template");  break;
		case FSP_DESIGN_TOOLS_CORE:
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, base_y+2, ALLEGRO_ALIGN_LEFT, "Core selected");
//   if (dwindow.selected_member != -1) // should never happen but check just to make sure
//			{
//    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x + 150, base_y+3, ALLEGRO_ALIGN_LEFT, "Component data cost %i", dwindow.templ->member[dwindow.selected_member].data_cost);
//			}
   if (dwindow.templ->locked)
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, LOWER_HELP_LINE, ALLEGRO_ALIGN_LEFT, "This template is locked and can't be modified.");
/*			  else
					{
      al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, LOWER_HELP_LINE, ALLEGRO_ALIGN_LEFT, "Click and drag the core while holding shift to rotate it.");
      al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, LOWER_HELP_LINE+14, ALLEGRO_ALIGN_LEFT, "Hold control at the same time to lock the front angle.");
					}*/
   break;
		case FSP_DESIGN_TOOLS_MAIN:
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, base_y+2, ALLEGRO_ALIGN_LEFT, "Process design");  break;
		case FSP_DESIGN_TOOLS_MEMBER:
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, base_y+2, ALLEGRO_ALIGN_LEFT, "Component selected");
//   if (dwindow.selected_member != -1) // should never happen but check just to make sure
//			{
//    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x + 150, base_y+3, ALLEGRO_ALIGN_LEFT, "Component data cost %i", dwindow.templ->member[dwindow.selected_member].data_cost);
//			}
   if (dwindow.templ->locked)
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, LOWER_HELP_LINE, ALLEGRO_ALIGN_LEFT, "This template is locked and can't be modified.");
/*			  else
					{
      al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, LOWER_HELP_LINE, ALLEGRO_ALIGN_LEFT, "Click and drag the component while holding shift to rotate it.");
      al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, LOWER_HELP_LINE+14, ALLEGRO_ALIGN_LEFT, "Hold control at the same time to lock the angle.");
					}*/
   break;
		case FSP_DESIGN_TOOLS_EMPTY_LINK:
//   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, base_y+2, ALLEGRO_ALIGN_LEFT, "Empty link selected");  break;
		case FSP_DESIGN_TOOLS_ACTIVE_LINK:
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, base_y+2, ALLEGRO_ALIGN_LEFT, "Object selected");
   if (dwindow.templ->locked)
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, LOWER_HELP_LINE, ALLEGRO_ALIGN_LEFT, "This template is locked and can't be modified.");
/*			  else
					{
      al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, LOWER_HELP_LINE, ALLEGRO_ALIGN_LEFT, "Click and drag the object while holding shift to rotate it (some objects can't be rotated).");
      al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, LOWER_HELP_LINE+14, ALLEGRO_ALIGN_LEFT, "Hold control at the same time to lock the angle.");
					}*/
   break;
		case FSP_DESIGN_TOOLS_DELETE:
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, base_y+2, ALLEGRO_ALIGN_LEFT, "Really delete?");  break;
		case FSP_DESIGN_TOOLS_AUTOCODE:
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, base_y+2, ALLEGRO_ALIGN_LEFT, "Choose process attack routine");
   if (dwindow.templ->locked)
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], base_x - 10, LOWER_HELP_LINE, ALLEGRO_ALIGN_LEFT, "This template is locked and can't be modified.");
   break;
	}

	base_x += scaleUI_x(FONT_BASIC, 130);

// al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 5, base_y, ALLEGRO_ALIGN_LEFT, "design data");
// al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 145, base_y+2, ALLEGRO_ALIGN_LEFT, "data cost %i", dwindow.templ->data_cost);
// al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 255, base_y+2, ALLEGRO_ALIGN_LEFT, "inertia %i", dwindow.templ->total_mass);
// al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 145, base_y+2, ALLEGRO_ALIGN_LEFT, "Template %i (%s) data cost %i inertia %i", dwindow.templ->template_index, dwindow.templ->name, dwindow.templ->data_cost, dwindow.templ->total_mass);
// al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], panel[PANEL_DESIGN].x1 + 140, panel[PANEL_DESIGN].y1+3, ALLEGRO_ALIGN_LEFT, "Player %i Template %i (%s)  data cost %i  inertia %i", dwindow.templ->player_index, dwindow.templ->template_index, dwindow.templ->name, dwindow.templ->data_cost, dwindow.templ->total_mass);
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], panel[PANEL_DESIGN].x1 + scaleUI_x(FONT_SQUARE,140), panel[PANEL_DESIGN].y1+3, ALLEGRO_ALIGN_LEFT, "Player %i Template %i (%s)", dwindow.templ->player_index, dwindow.templ->template_index, dwindow.templ->name);
// if (dwindow.templ->active)
//  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], panel[PANEL_DESIGN].x1 + 140, panel[PANEL_DESIGN].y1+15, ALLEGRO_ALIGN_LEFT, "Power: capacity %i  use: peak %i average %i base %i", nshape[dwindow.templ->member[0].shape].power_capacity, dwindow.templ->power_use_peak, dwindow.templ->power_use_smoothed, dwindow.templ->power_use_base);

	if (control.mouse_panel == PANEL_DESIGN)
//		&&	control.panel_element_highlighted_time == inter.running_time)
	{
		design_help_highlight(base_x, base_y);
	}
	 else
			design_help_unhighlighted(base_x, base_y);

}

static void design_help_unhighlighted(int base_x, int base_y)
{
// the type of help drawn depends on which menu is open:
 switch(dwindow.tools_open)
 {
 	case FSP_DESIGN_TOOLS_CORE:
//   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 15, base_y+2, ALLEGRO_ALIGN_LEFT, "data cost %i", dwindow.templ->data_cost);
   break;

 }


}

const char* object_description [OBJECT_TYPES] [3] =
{
 {"No object.","",""}, //	OBJECT_TYPE_NONE,
 {"Connects a component to its parent component.",
  "Changing this object to a different link changes the way",
  "the component connects to its parent."}, //	OBJECT_TYPE_UPLINK,
//  "Changing this object to a different link changes the way the component connects to its parent."}, //	OBJECT_TYPE_UPLINK,
 {"Connects a component to a child component.",
  "Adding a downlink creates a new component.",
  "To remove, select the child component and delete it."}, //	OBJECT_TYPE_DOWNLINK,
 {"Moves the process. Must not be obstructed by another",
  "component (make sure the line from the object doesn't",
  "cross the square around a component)."}, //	OBJECT_TYPE_MOVE,
 {"Fires a malicious data packet.",
  "Can rotate to track its target.",
  ""}, //	OBJECT_TYPE_PULSE,
 {"Fires a malicious data packet.",
  "Can rotate to track its target.",
  "Large pulse is slower but more powerful than standard pulse."}, //	OBJECT_TYPE_PULSE_L,
 {"Fires a malicious data packet.",
  "Can rotate to track its target.",
  "X-Large pulse is very slow but powerful."}, //	OBJECT_TYPE_PULSE_XL,
 {"Fires a malicious data packet.",
  "Cannot rotate, but is cheaper and stronger than pulse.",
  ""}, //	OBJECT_TYPE_BURST,
 {"Fires a malicious data packet. Cannot rotate.",
  "Large burst is slower but stronger than standard burst.",
  ""}, //	OBJECT_TYPE_BURST_L,
 {"Fires a malicious data packet. Cannot rotate.",
  "X-Large burst is very slow but powerful.",
  ""}, //	OBJECT_TYPE_BURST_XL,
 {"Allows the process to build other processes.",
  "Multiple build objects speed recovery time after building.",
  ""}, //	OBJECT_TYPE_BUILD,
 {"Generates an interface to protect the process.",
  "The interface protects all components, except components",
  "with either an interface object or a move object."}, //	OBJECT_TYPE_INTERFACE,
// {"Provides an interface buffer for use by interface objects.",
//  "Multiple depth objects increase the size and recovery", //	OBJECT_TYPE_INTERFACE_DEPTH,
  //"speed of the buffer."}, //	OBJECT_TYPE_INTERFACE_DEPTH,
// {"","",""}, //	OBJECT_TYPE_INTERFACE_STABILITY,
// {"","",""}, //	OBJECT_TYPE_INTERFACE_RESPONSE,
 {"Allows the process to harvest data from data wells,",
  "and also transfer it to other processes.",
  "Requires at least one storage object."}, //	OBJECT_TYPE_HARVEST,
 {"Stores data harvested from data wells or transferred",
  "from other processes.",
  ""}, //	OBJECT_TYPE_STORAGE,
 {"Allocates data so that it can be used to build processes.",
  "Requires at least one storage object.",
   ""}, //	OBJECT_TYPE_ALLOCATE,
 {"Fires a powerful stream of malicious data. Does",
  "double damage against components not protected by",
  "an interface. Cannot rotate."}, //	OBJECT_TYPE_STREAM,
 {"Fires a powerful stream of malicious data. Does",
  "double damage against components not protected by",
  "an interface. Can rotate to track its target."}, //	OBJECT_TYPE_STREAM_DIR,
 {"Long-range attacking object. Does full damage only near",
  "maximum range.",
  "* The autocoder doesn't support spikes on static processes."}, //	OBJECT_TYPE_SPIKE,
 {"Repairs damage to components of this process, and restores",
  "destroyed components. Multiple repair objects speed up",
  "repair, and reduce recovery time after restoring."}, //	OBJECT_TYPE_REPAIR,
 {"Works like a standard repair object, but can also repair",
  "and restore damage to other nearby processes. Allows",
  "standard repair objects to do this as well."}, //	OBJECT_TYPE_REPAIR_OTHER,
 {"Extremely powerful attack. Cannot rotate.",
  "",
  ""}, //	OBJECT_TYPE_ULTRA,
 {"Extremely powerful attack.",
  "Can rotate to attack its target.",
  ""}, //	OBJECT_TYPE_ULTRA_DIR,
 {"Fires a fast stream of malicious data. Does",
  "double damage against components not protected by",
  "an interface. Can rotate to track its target."}, //	OBJECT_TYPE_SLICE,
 {"Increases stability of interface for this component.",
  "(Halves damage taken by this component's interface",
  "while active.)"}, //	OBJECT_TYPE_STABILITY,

};

static void draw_design_help_strings(int base_x, int base_y, char* help_heading, char* help_line1, char* help_line2, char* help_line3);
static void draw_design_help_strings_for_help_button(int base_x, int base_y);
static void draw_design_help_strings_for_help_more_button(int base_x, int base_y);

// draws help for the button the mouse is over
static void design_help_highlight(int base_x, int base_y)
{

 float line_h = scaleUI_y(FONT_BASIC, 12);

//	if (control.panel_element_highlighted >= FPE_DESIGN_SUB_BUTTON_0
//		&& control.panel_element_highlighted <= FPE_DESIGN_SUB_BUTTON_14)
	if (dwindow.selected_member != -1)
	{

// mouse is over a button for a shape.
// show information about the shape
		if (panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_CORE].open == 1
			|| panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MEMBER].open == 1)
	 {
   int help_shape;
   int help_shape_col = PLAN_COL_DESIGN_BASIC;

  	if (control.panel_element_highlighted_time >= inter.running_time
				&&	control.panel_element_highlighted >= FPE_DESIGN_SUB_BUTTON_0
  		&& control.panel_element_highlighted <= FPE_DESIGN_SUB_BUTTON_14)
  	{
  		help_shape = panel[PANEL_DESIGN].element[control.panel_element_highlighted].value [2];
  		if (help_shape != dwindow.templ->member[dwindow.selected_member].shape)
					help_shape_col = PLAN_COL_DESIGN_MODIFIED;
  	}
  		else
				{
						help_shape = dwindow.templ->member[dwindow.selected_member].shape;
				}

   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 255)), (int) (base_y + 2), ALLEGRO_ALIGN_LEFT, "%s", identifier[nshape[help_shape].keyword_index].name);
			int line_y = base_y + scaleUI_y(FONT_SQUARE, 20);

   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 305)), line_y, ALLEGRO_ALIGN_RIGHT, "total cost");
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 315)), line_y, ALLEGRO_ALIGN_LEFT, "%i", dwindow.templ->member[dwindow.selected_member].data_cost);
   line_y += line_h;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 305)), line_y, ALLEGRO_ALIGN_RIGHT, "angle");
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 315)), line_y, ALLEGRO_ALIGN_LEFT, "%i", dwindow.templ->member[dwindow.selected_member].connection_angle_offset_angle);
   line_y += line_h;
/*   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 305, line_y, ALLEGRO_ALIGN_RIGHT, "gao");
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 315, line_y, ALLEGRO_ALIGN_LEFT, "     %i", fixed_angle_to_int(dwindow.templ->member[dwindow.selected_member].group_angle_offset) & ANGLE_MASK);
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 315, line_y + 12, ALLEGRO_ALIGN_LEFT, "     %i", ANGLE_1 - (fixed_angle_to_int(dwindow.templ->member[dwindow.selected_member].group_angle_offset) & ANGLE_MASK));
*/

   draw_proc_shape(base_x + scaleUI_x(FONT_BASIC, 320),
																			line_y + line_h + 40,
																			0, // angle
																	  help_shape, // this should be the shape index
																	  0,
																	  1, // zoom
																	  colours.plan_col [help_shape_col]);

			line_y = base_y + 116 + scaleUI_y(FONT_SQUARE, 20) + scaleUI_y(FONT_BASIC, 24);

   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 305)), line_y, ALLEGRO_ALIGN_RIGHT, "base cost");
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 315)), line_y, ALLEGRO_ALIGN_LEFT, "%i", nshape[help_shape].data_cost);
//   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 340, line_y, ALLEGRO_ALIGN_RIGHT, "%i (%i)", nshape[panel[PANEL_DESIGN].element[control.panel_element_highlighted].value [2]].data_cost, panel[PANEL_DESIGN].element[control.panel_element_highlighted].value [2]);
   line_y += line_h;
   if (panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_CORE].open == 1)
			{
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 305)), line_y, ALLEGRO_ALIGN_RIGHT, "integrity");
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 315)), line_y, ALLEGRO_ALIGN_LEFT, "%i", nshape[help_shape].base_hp_max);
    line_y += line_h;
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 305)), line_y, ALLEGRO_ALIGN_RIGHT, "core power");
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 315)), line_y, ALLEGRO_ALIGN_LEFT, "%i", nshape[help_shape].power_capacity);
    line_y += line_h;
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 305)), line_y, ALLEGRO_ALIGN_RIGHT, "component power");
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 315)), line_y, ALLEGRO_ALIGN_LEFT, "+%i", nshape[help_shape].component_power_capacity);
    line_y += line_h;
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 305)), line_y, ALLEGRO_ALIGN_RIGHT, "instructions");
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 315)), line_y, ALLEGRO_ALIGN_LEFT, "%i", nshape[help_shape].instructions_per_cycle);
    line_y += line_h;
    if (help_shape < FIRST_MOBILE_NSHAPE)
				{
     line_y += scaleUI_y(FONT_BASIC, 5);
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 255)), line_y, ALLEGRO_ALIGN_LEFT, "this is a static core");
     line_y += line_h;
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], (int) (base_x + scaleUI_x(FONT_BASIC, 255)), line_y, ALLEGRO_ALIGN_LEFT, "and does not move");
     line_y += line_h;
				}
/*    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 315, line_y, ALLEGRO_ALIGN_RIGHT, "build recycle");
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 340, line_y, ALLEGRO_ALIGN_RIGHT, "%i", nshape[help_shape].build_or_restore_time);*/
			}
			return;

	 }

// mouse is over a button for an object.
// show information about the object


	 if (dwindow.selected_link != -1
			&& (panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_ACTIVE_LINK].open == 1
		  || panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_EMPTY_LINK].open == 1))
		{
			struct object_struct display_object;
			int display_object_col = PLAN_COL_DESIGN_BASIC;

  	if (control.panel_element_highlighted_time == inter.running_time
				&&	control.panel_element_highlighted >= FPE_DESIGN_SUB_BUTTON_0
  		&& control.panel_element_highlighted <= FPE_DESIGN_SUB_BUTTON_14)
  	{
 			display_object.type = design_sub_button[panel[PANEL_DESIGN].element[control.panel_element_highlighted].value[0]].value;
  		if (display_object.type != dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type)
					display_object_col = PLAN_COL_DESIGN_MODIFIED;
  	}
  		else
				{
					 display_object.type = dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type;
				}


			int object_detail_base_y = base_y;// - 20;

//			display_object.type = design_sub_button[panel[PANEL_DESIGN].element[control.panel_element_highlighted].value[0]].value; //panel[PANEL_DESIGN].element[control.panel_element_highlighted].value [2];
			display_object.base_angle_offset = 0;

			add_menu_button(base_x + scaleUI_x(FONT_BASIC, 260), object_detail_base_y + 36,
																				base_x + scaleUI_x(FONT_BASIC, 340), object_detail_base_y + 93,
																				colours.base [COL_BLUE] [SHADE_LOW], 6, 3);

			      draw_object(base_x + scaleUI_x(FONT_BASIC, 250), object_detail_base_y + 65,
																					0, // angle
																					NSHAPE_CORE_STATIC_QUAD,
																					&display_object, // uses same code as drawing objects for the main display
																					NULL, // no object instance
																					NULL, // no core
																					NULL, // no proc
																					0, // link
																					colours.plan_col [display_object_col],
																			  1.5); // last number is zoom

				int text_x_265 = base_x + scaleUI_x(FONT_BASIC, 265);
				int text_x_305 = base_x + scaleUI_x(FONT_BASIC, 305);
				int text_x_315 = base_x + scaleUI_x(FONT_BASIC, 315);

    al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + scaleUI_x(FONT_BASIC, 255), object_detail_base_y + 2, ALLEGRO_ALIGN_LEFT, "%s", otype[display_object.type].name);
		 	int line_y = object_detail_base_y + 20;
    if (!otype[display_object.type].object_details.only_zero_angle_offset)
				{
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_305, line_y, ALLEGRO_ALIGN_RIGHT, "angle");
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_315, line_y, ALLEGRO_ALIGN_LEFT, "%i", dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].base_angle_offset_angle);
//     line_y += 12;
				}
    line_y = object_detail_base_y + 100;

    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_305, line_y, ALLEGRO_ALIGN_RIGHT, "cost");
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_315, line_y, ALLEGRO_ALIGN_LEFT, "%i", otype[display_object.type].data_cost);
    line_y += line_h;
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_305, line_y, ALLEGRO_ALIGN_RIGHT, "peak power");
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_315, line_y, ALLEGRO_ALIGN_LEFT, "%i", otype[display_object.type].power_use_peak);
    line_y += line_h;
//    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 305, line_y, ALLEGRO_ALIGN_RIGHT, "average power");
//    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 315, line_y, ALLEGRO_ALIGN_LEFT, "%i", otype[display_object.type].power_use_smoothed);
//    line_y += 12;
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_305, line_y, ALLEGRO_ALIGN_RIGHT, "base power");
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_315, line_y, ALLEGRO_ALIGN_LEFT, "%i", otype[display_object.type].power_use_base);
    line_y += line_h;
    if (otype[display_object.type].object_base_type == OBJECT_BASE_TYPE_ATTACK)
				{
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_305, line_y, ALLEGRO_ALIGN_RIGHT, "damage");
     switch(display_object.type)
     {
     	case OBJECT_TYPE_STREAM:
     	case OBJECT_TYPE_STREAM_DIR:
       al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_315, line_y, ALLEGRO_ALIGN_LEFT, "%i - %i", otype[display_object.type].object_details.damage * STREAM_FIRING_TIME, otype[display_object.type].object_details.damage * STREAM_FIRING_TIME * 2);
       break;
     	case OBJECT_TYPE_SLICE:
       al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_315, line_y, ALLEGRO_ALIGN_LEFT, "%i - %i", otype[display_object.type].object_details.damage * SLICE_FIRING_TIME, otype[display_object.type].object_details.damage * SLICE_FIRING_TIME * 2);
       break;
      case OBJECT_TYPE_SPIKE:
       al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_315, line_y, ALLEGRO_ALIGN_LEFT, "%i - %i", otype[display_object.type].object_details.damage, SPIKE_MAX_DAMAGE);
       break;
      default:
       al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_315, line_y, ALLEGRO_ALIGN_LEFT, "%i", otype[display_object.type].object_details.damage);
       break;
     }
     line_y += line_h;
#ifdef DEBUG_MODE
     int dpc = 1;
     switch(display_object.type)
     {
     	case OBJECT_TYPE_STREAM:
     	case OBJECT_TYPE_STREAM_DIR:
     		dpc = (otype[display_object.type].object_details.damage * STREAM_FIRING_TIME);
       break;
     	case OBJECT_TYPE_SLICE:
     		dpc = (otype[display_object.type].object_details.damage * SLICE_FIRING_TIME);
       break;
      case OBJECT_TYPE_SPIKE:
							dpc = SPIKE_MAX_DAMAGE;
							break;
      default:
      	dpc = otype[display_object.type].object_details.damage;
       break;
     }
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 305, line_y, ALLEGRO_ALIGN_RIGHT, "per cycle");
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 315, line_y, ALLEGRO_ALIGN_LEFT, "%f",
																			(float) dpc / (float) otype[display_object.type].object_details.recycle_time);
     line_y += 12;
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 305, line_y, ALLEGRO_ALIGN_RIGHT, "per cycle per data");
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 315, line_y, ALLEGRO_ALIGN_LEFT, "%f",
																			((float) dpc / (float) otype[display_object.type].data_cost) / (float) otype[display_object.type].object_details.recycle_time);
     line_y += 12;
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 305, line_y, ALLEGRO_ALIGN_RIGHT, "per cycle per power");
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x + 315, line_y, ALLEGRO_ALIGN_LEFT, "%f",
																			((float) dpc / (float) otype[display_object.type].power_use_peak) / (float) otype[display_object.type].object_details.recycle_time);
     line_y += 12;

#endif
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_305, line_y, ALLEGRO_ALIGN_RIGHT, "recycle time");
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_315, line_y, ALLEGRO_ALIGN_LEFT, "%i", otype[display_object.type].object_details.recycle_time / EXECUTION_COUNT);
     line_y += line_h;
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_305, line_y, ALLEGRO_ALIGN_RIGHT, "API type");
     switch(otype[display_object.type].object_details.attack_type)
     {
     	case ATTACK_TYPE_PULSE:
       al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_315, line_y, ALLEGRO_ALIGN_LEFT, "pulse"); break;
     	case ATTACK_TYPE_BURST:
       al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_315, line_y, ALLEGRO_ALIGN_LEFT, "burst"); break;
     	case ATTACK_TYPE_SPIKE:
       al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_315, line_y, ALLEGRO_ALIGN_LEFT, "spike"); break;
     }
     line_y += line_h;
				}
    line_y += scaleUI_y(FONT_BASIC, 8);
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_265, line_y, ALLEGRO_ALIGN_LEFT, "%s", object_description[display_object.type][0]);
    if (object_description[display_object.type][1][0] != 0)
				{
     line_y += scaleUI_y(FONT_BASIC, 14);
     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_265, line_y, ALLEGRO_ALIGN_LEFT, "%s", object_description[display_object.type][1]);
     if (object_description[display_object.type][2][0] != 0)
				 {
      line_y += scaleUI_y(FONT_BASIC, 14);
      al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], text_x_265, line_y, ALLEGRO_ALIGN_LEFT, "%s", object_description[display_object.type][2]);
				 }
				}
    line_y += scaleUI_y(FONT_BASIC, 28);
				switch(dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].template_error)
				{
//					case TEMPLATE_OBJECT_ERROR_INTERFACE_CORE:
//      al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_RED] [SHADE_MED], base_x + 265, line_y, ALLEGRO_ALIGN_LEFT, "Error: interface on core"); break;
//					case TEMPLATE_OBJECT_ERROR_MOVE_INTERFACE:
//      al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_RED] [SHADE_MED], base_x + 265, line_y, ALLEGRO_ALIGN_LEFT, "Error: interface and move on same component"); break;
					case TEMPLATE_OBJECT_ERROR_STATIC_MOVE:
      al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_RED] [SHADE_MED], text_x_265, line_y, ALLEGRO_ALIGN_LEFT, "Error: move on static process"); break;
					case TEMPLATE_OBJECT_ERROR_MOVE_OBSTRUCTED:
      al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_RED] [SHADE_MED], text_x_265, line_y, ALLEGRO_ALIGN_LEFT, "Error: move obstructed by component"); break;
					case TEMPLATE_OBJECT_ERROR_MOBILE_ALLOCATE:
      al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_RED] [SHADE_MED], text_x_265, line_y, ALLEGRO_ALIGN_LEFT, "Error: allocate on non-static process"); break;
					case TEMPLATE_OBJECT_ERROR_STORY_LOCK:
      al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_RED] [SHADE_MED], text_x_265, line_y, ALLEGRO_ALIGN_LEFT, "Error: object needs to be unlocked"); break;
				}
		 return;
		}
	} // end if selected_member != -1

 if (panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_AUTOCODE].open == 1)
	{
		 int ac_base_x = base_x + scaleUI_x(FONT_BASIC, 120);


  	if (control.panel_element_highlighted_time == inter.running_time
				&&	control.panel_element_highlighted >= FPE_DESIGN_SUB_BUTTON_0
  		&& control.panel_element_highlighted <= FPE_DESIGN_SUB_BUTTON_14)
  	{
  		switch(panel[PANEL_DESIGN].element[control.panel_element_highlighted].value [2])
  		{
// although most of the text is here, draw_design_help_strings_autocode() may add some detail based on the reason why a particular autocode type is unavailable
  			case AUTOCODE_NONE:
  		  draw_design_help_strings_autocode(ac_base_x,
																																						  base_y,
																																						  "No attack code",
																																						  "For static processes, and other processes with no main attack.",
																																						  "",
																																						  "The process will perform non-attacking actions, and will defend itself",
																																						  "with any directional attacking objects it has (e.g. pulse).",
																																						  panel[PANEL_DESIGN].element[control.panel_element_highlighted].value [3]);

//* need to remove attack_found for this kind?

							break;
  			case AUTOCODE_STANDARD:
  		  draw_design_help_strings_autocode(ac_base_x,
																																						  base_y,
																																						  "Basic attack",
																																						  "When attacking, process will move to medium range and fire on",
																																						  "its target. If it has retro move objects, it will move backwards",
																																						  "to try to maintain distance.",
																																						  "",
																																						  panel[PANEL_DESIGN].element[control.panel_element_highlighted].value [3]);
							break;
  			case AUTOCODE_CHARGE:
  		  draw_design_help_strings_autocode(ac_base_x,
																																						  base_y,
																																						  "Charge",
																																						  "When attacking, process will move towards its target and try to",
																																						  "collide with it (collisions cause no harm, but may be annoying).",
																																						  "",
																																						  "",
																																						  panel[PANEL_DESIGN].element[control.panel_element_highlighted].value [3]);
							break;
  			case AUTOCODE_BOMBARD:
  		  draw_design_help_strings_autocode(ac_base_x,
																																						  base_y,
																																						  "Bombard",
																																						  "For processes with long-range attacks. Process will try to",
																																						  "stay at maximum distance, and will continue to fire at the last",
																																						  "known position of a target outside scanning range.",
																																						  "",
																																						  panel[PANEL_DESIGN].element[control.panel_element_highlighted].value [3]);
							break;
  			case AUTOCODE_CIRCLE_CW:
  		  draw_design_help_strings_autocode(ac_base_x,
																																						  base_y,
																																						  "Circle clockwise",
																																						  "Process will move in a clockwise circle around the target, firing",
																																						  "inwards if possible.",
																																						  "Not suitable for processes with front-facing fixed attacks.",
																																						  "",
																																						  panel[PANEL_DESIGN].element[control.panel_element_highlighted].value [3]);
							break;
  			case AUTOCODE_CIRCLE_ACW:
  		  draw_design_help_strings_autocode(ac_base_x,
																																						  base_y,
																																						  "Circle anti-clockwise",
																																						  "Process will move in an anti-clockwise circle around the target, firing",
																																						  "inwards if possible.",
																																						  "Not suitable for processes with front-facing fixed attacks.",
																																						  "",
																																						  panel[PANEL_DESIGN].element[control.panel_element_highlighted].value [3]);
							break;
  			case AUTOCODE_ERRATIC:
  		  draw_design_help_strings_autocode(ac_base_x,
																																						  base_y,
																																						  "Erratic",
																																						  "Process will move erratically around target, attacking when possible.",
																																						  "If small and fast, process may be able to evade enemy target-leading",
																																						  "algorithms.",
																																						  "Not suitable for processes with front-facing fixed attacks.",
																																						  panel[PANEL_DESIGN].element[control.panel_element_highlighted].value [3]);
							break;
  			case AUTOCODE_CAUTIOUS:
  		  draw_design_help_strings_autocode(ac_base_x,
																																						  base_y,
																																						  "Cautious",
																																						  "Process will attack target, but retreat if damaged.",
																																						  "",
																																						  "",
																																						  "",
																																						  panel[PANEL_DESIGN].element[control.panel_element_highlighted].value [3]);
							break;
							}
  	}
  	 else
     draw_design_help_strings(ac_base_x, base_y, "Autocoder",
																														                   "Choose how this process will attack.",
																														                   "",
																														                   "");

		return;
	}





	// else
	if (control.panel_element_highlighted_time == inter.running_time)
//		&&	control.panel_element_highlighted >= FPE_DESIGN_SUB_BUTTON_0
//		&& control.panel_element_highlighted <= FPE_DESIGN_SUB_BUTTON_14)
		{
			switch(control.panel_element_highlighted)
			{
				case FPE_DESIGN_TOOLS_EMPTY_NEW:
     draw_design_help_strings(base_x, base_y, "New template",
																														                "Prepares this template for editing.",
																														                "",
																														                "");
					break;
				case FPE_DESIGN_TOOLS_MAIN_AUTO:
     draw_design_help_strings(base_x, base_y, "Write header",
																														                "Updates the process header in this template's source code",
																														                "with code that will generate the process you have designed.",
																														                "Does not update anything after the #code line.");
					break;
				case FPE_DESIGN_TOOLS_MAIN_AUTOCODE:
     draw_design_help_strings(base_x, base_y, "Autocode",
																														                "Automatically generates code to run the process you",
																														                "have designed. Overwrites the current source file.",
																														                "");
					break;
				case FPE_DESIGN_TOOLS_MAIN_SYMM:
     draw_design_help_strings(base_x, base_y, "Symmetry",
																														                "Makes the process into a mirror image around the horizontal",
																														                "centre line.",
																														                "");
					break;
				case FPE_DESIGN_TOOLS_MAIN_LOCK:
     draw_design_help_strings(base_x, base_y, "Lock template",
																														                "Writes the header, then tries to build this process from",
																														                "source code. If successful, locks the template.", "");
					break;
				case FPE_DESIGN_TOOLS_MAIN_UNLOCK:
     draw_design_help_strings(base_x, base_y, "Unlock template",
																														                "Tries to unlock this template so that it can be edited.",
																														                "Will only work if there are no processes built from it.",
																														                "");
					break;
				case FPE_DESIGN_TOOLS_MAIN_DELETE:
     draw_design_help_strings(base_x, base_y, "Delete template",
																														                "Deletes the template and everything in it.",
																														                "Will only work if there are no processes built from it.",
																														                "");
					break;
				case FPE_DESIGN_TOOLS_MAIN_HELP:
     draw_design_help_strings_for_help_button(base_x, base_y + 2);
					break;
				case FPE_DESIGN_TOOLS_MAIN_HELP_MORE:
     draw_design_help_strings_for_help_more_button(base_x, base_y + 2);
					break;
				case FPE_DESIGN_TOOLS_MEMBER_SHAPE:
     draw_design_help_strings(base_x, base_y, "Shape",
																														                "Changes the shape of the selected component.",
																														                "",
																														                "");
					break;
				case FPE_DESIGN_TOOLS_MEMBER_DELETE:
     draw_design_help_strings(base_x, base_y, "Delete component",
																														                "Removes the selected component and any other component",
																														                "downlinked from it.",
																														                "");
					break;
				case FPE_DESIGN_TOOLS_MEMBER_EXIT:
     draw_design_help_strings(base_x, base_y, "Exit",
																														                "Return to the main design menu.",
																														                "",
																														                "");
					break;
				case FPE_DESIGN_TOOLS_CORE_CORE_SHAPE:
     draw_design_help_strings(base_x, base_y, "Core shape",
																														                "Changes the shape of the core.",
																														                "Each different kind of core has a different cost and power capacity.",
																														                "More expensive cores increase recovery time for the builder process.");
					break;
				case FPE_DESIGN_TOOLS_CORE_EXIT:
     draw_design_help_strings(base_x, base_y, "Exit",
																														                "Return to the main design menu.",
																														                "",
																														                "");
					break;




			}

		}
			 else
     draw_design_help_strings(base_x, base_y, "Process designer panel",
																														                "Use this to design new processes.",
																														                "Change the process being edited using the Template (Te) panel.",
																														                "(The template must be unlocked; locked templates can't be edited)");


// may have returned before now

}

static void draw_design_help_strings(int base_x, int base_y, char* help_heading, char* help_line1, char* help_line2, char* help_line3)
{

 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x - 20, base_y + scaleUI_y(FONT_SQUARE, 40), ALLEGRO_ALIGN_LEFT, "%s", help_heading);
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x, base_y + scaleUI_y(FONT_BASIC, 60), ALLEGRO_ALIGN_LEFT, "%s", help_line1);
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x, base_y + scaleUI_y(FONT_BASIC, 75), ALLEGRO_ALIGN_LEFT, "%s", help_line2);
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x, base_y + scaleUI_y(FONT_BASIC, 90), ALLEGRO_ALIGN_LEFT, "%s", help_line3);



}


static void draw_design_help_strings_autocode(int base_x, int base_y, char* help_heading, char* help_line1, char* help_line2, char* help_line3, char* help_line4, int autocode_available)
{

 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x - 20, base_y += scaleUI_y(FONT_SQUARE, 40), ALLEGRO_ALIGN_LEFT, "%s", help_heading);
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 20), ALLEGRO_ALIGN_LEFT, "%s", help_line1);
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 15), ALLEGRO_ALIGN_LEFT, "%s", help_line2);
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 15), ALLEGRO_ALIGN_LEFT, "%s", help_line3);
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 15), ALLEGRO_ALIGN_LEFT, "%s", help_line4);

 switch(autocode_available)
 {
   case AUTOCODE_AVAILABLE_MAYBE_SPIKE:
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 25), ALLEGRO_ALIGN_LEFT, "Warning: process has spike objects, which this");
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 15), ALLEGRO_ALIGN_LEFT, "         attack type does not make good use of");
    break;
   case AUTOCODE_AVAILABLE_MAYBE_NO_FWD_ATT:
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 25), ALLEGRO_ALIGN_LEFT, "Warning: design has no forward attack");
    break;
   case AUTOCODE_AVAILABLE_MAYBE_NO_FWD_OR_R_ATT:
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 25), ALLEGRO_ALIGN_LEFT, "Warning: design has no forward or right directional attack");
    break;
   case AUTOCODE_AVAILABLE_MAYBE_NO_FWD_OR_L_ATT:
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 25), ALLEGRO_ALIGN_LEFT, "Warning: design has no forward or left directional attack");
    break;
   case AUTOCODE_AVAILABLE_MAYBE_POOR_MAIN:
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 25), ALLEGRO_ALIGN_LEFT, "Warning: process has fixed forward attacks, which this");
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 15), ALLEGRO_ALIGN_LEFT, "         attack type does not make good use of");
    break;
   case AUTOCODE_AVAILABLE_MAYBE_NO_SPIKE:
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 25), ALLEGRO_ALIGN_LEFT, "Warning: design has no long-range attack");
    break;
   case AUTOCODE_AVAILABLE_MAYBE_NO_MOVE:
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 25), ALLEGRO_ALIGN_LEFT, "Warning: design has no move objects");
    break;
   case AUTOCODE_AVAILABLE_NO_STATIC:
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_RED] [SHADE_HIGH], base_x, base_y += scaleUI_y(FONT_BASIC, 25), ALLEGRO_ALIGN_LEFT, "Unavailable: design is static");
    break;

 }


}


#define DESIGN_HELP_STRINGS 13

char *design_help_strings [DESIGN_HELP_STRINGS] =
{
"Use the process designer to design the structure of the process in the current template.",
"- Choose templates using the Template panel (click the Te button in the top right).",
"- Open the Editor panel (the Ed button) to see the current template's source code.",
"  - The source code is definitive; the version of the process in the designer needs to",
"    be written to the source code before you can use it. To do this:",
"    - the -Autocode- button generates full source code for the process.",
"    - the -Write header- button just updates the structural part of the source code.",
"  - Load and save source code using the File menu in the Editor panel.",
"  - Compile the source code (using the Compile menu in the Editor panel) to update",
"    the version in the designer.",
"- When you've finished designing the process, you can build it in the game.",
"  - Since the process will be based on the source code, not on the version in",
"    the process designer, make sure you update the source code before you build it!"
};

static void draw_design_help_strings_for_help_button(int base_x, int base_y)
{

 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x - 20, base_y, ALLEGRO_ALIGN_LEFT, "Process design - help");
 float line_y = base_y + scaleUI_y(FONT_SQUARE, 20);

 int i;

 for (i = 0; i < DESIGN_HELP_STRINGS; i ++)
	{

  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x, line_y, ALLEGRO_ALIGN_LEFT, "%s", design_help_strings [i]);
  line_y += scaleUI_y(FONT_BASIC, 15);

	}

}


#define DESIGN_HELP_MORE_STRINGS 13

char *design_help_strings2 [DESIGN_HELP_MORE_STRINGS] =
{
"- In the window above, select components or objects (objects are the little modules",
"  attached to components) to change or remove them.",
"- Use the rotation buttons to rotate components and objects. Hold control to constrain rotation.",
"- Add new components by selecting a link and clicking [new component].",
"- Adding components and objects increases the cost of the process and the time it",
"  takes to build the process.",
"- Some objects require power.",
"  - More expensive cores generate more power (and make their components generate more power too).",
//"    Peak power can exceed power capacity, but not by too much or the process may not work very well.",
"  - The process' -base- demand is the total power consumption of its move and interface objects.",
"  - The process' -peak- demand is the power required if all objects are in use at the same time.",
"  - The process' -%%power- is the ratio of peak power demand to total capacity.",
"    A slightly underpowered process (~70%%) should be okay, as its objects probably won't",
"    all be drawing power at the same time.",


};

static void draw_design_help_strings_for_help_more_button(int base_x, int base_y)
{

 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x - 20, base_y, ALLEGRO_ALIGN_LEFT, "Process design - more help");
 float line_y = base_y + scaleUI_y(FONT_SQUARE, 20);

 int i;

 for (i = 0; i < DESIGN_HELP_MORE_STRINGS; i ++)
	{

  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], base_x, line_y, ALLEGRO_ALIGN_LEFT, "%s", design_help_strings2 [i]);
  line_y += scaleUI_y(FONT_BASIC, 15);

	}

}

