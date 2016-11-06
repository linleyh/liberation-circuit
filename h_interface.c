/*

Interface code for story mode.

draws map etc and receives input.


*/
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"
#include "m_maths.h"

#include "g_misc.h"

#include "c_header.h"
#include "c_prepr.h"
#include "e_slider.h"
#include "e_header.h"
#include "e_editor.h"
#include "g_header.h"
#include "g_world.h"
#include "g_world_map.h"
#include "g_world_map_2.h"
#include "e_log.h"
#include "g_proc_new.h"

#include "p_draw.h"
#include "p_panels.h"

#include "i_header.h"
#include "i_input.h"
#include "i_view.h"
#include "i_disp_in.h"
#include "i_display.h"
#include "m_input.h"
#include "s_menu.h"
#include "s_mission.h"
#include "t_files.h"
#include "t_template.h"

#include "h_story.h"

enum
{
STORY_REGION_COL_BLUE,

STORY_REGION_COLS
};



int story_area_base_col [STORY_AREAS] [3] =
{

	{30, 60, 100}, // AREA_TUTORIAL
	{20, 50, 100}, // AREA_BLUE
	{20, 100, 40}, // AREA_GREEN
	{100, 70, 20}, // AREA_YELLOW
	{120, 40, 10}, // AREA_ORANGE
	{80, 30, 100}, // AREA_PURPLE
	{10, 40, 70}, // AREA_DARK_BLUE
	{70, 70, 70}, // AREA_GREY
	{140, 0, 0}, // AREA_RED


};

/*
#define STORY_REGION_SIZE 24

#define STORY_REGION_SEPARATION_X 16
#define STORY_REGION_SEPARATION_Y 26
*/

#define STORY_REGION_SIZE 34

#define STORY_REGION_SEPARATION_X 21
#define STORY_REGION_SEPARATION_Y 36

#define FIXED_STORY_BOX_X 20
#define FIXED_STORY_BOX_Y 550

#define STORY_BOX_W 380
#define STORY_BOX_H 140

#define GO_BUTTON_X (FIXED_STORY_BOX_X + STORY_BOX_W + 40)
#define GO_BUTTON_Y FIXED_STORY_BOX_Y
#define GO_BUTTON_SIZE STORY_BOX_H


struct region_inter_struct
{

	int x_offset; // this is an unzoomed offset from the starting region.
	int y_offset;

	float x_screen; // this is the absolute position of the region on the actual screen (zoomed).
	float y_screen;

	int area_index;

//	int colour;

	timestamp region_highlight_time;

};

struct story_inter_struct
{

 int region_x_start; // unzoomed position of region 0
 int region_y_start; // unzoomed position of region 0

 struct region_inter_struct region_inter [STORY_REGIONS];

	int region_mouse_over;
	int region_selected;
	timestamp region_select_time;
	timestamp region_mouse_over_time;

	float zoom;

#define STORY_RIPPLES 12
#define STORY_RIPPLE_TIME 96

	float ripple_x [STORY_RIPPLES];
	float ripple_y [STORY_RIPPLES];
	timestamp ripple_time [STORY_RIPPLES];

};

struct story_inter_struct story_inter;

extern struct story_struct story;
extern struct game_struct game;

extern struct fontstruct font [FONTS];
extern struct vbuf_struct vbuf;
extern ALLEGRO_DISPLAY* display;

static void reset_region_screen_positions(void);
static void draw_story_regions(void);
//static void draw_story_region_box(int region_index);
static void draw_story_boxes_etc(void);
static void draw_a_story_box(int region_index, float draw_x, float draw_y, timestamp select_time);
static void add_orthogonal_hexagon_story(int layer, float x, float y, float size, ALLEGRO_COLOR col1);
static void add_story_bquad(float xa, float ya, float wa, float ha, float corner1, float corner2, ALLEGRO_COLOR col);
//static void add_story_triangle(int layer, float xa, float ya, float xb, float yb, float xc, float yc, ALLEGRO_COLOR col);
static void add_story_quad(int layer, float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, ALLEGRO_COLOR col);

// called once, at start of game
//  (actually currently called each time story mode is entered - fix?)
void init_story_interface(void)
{

	int i;

	story_inter.zoom = 1;

	for (i = 0; i < STORY_REGIONS; i ++)
	{
		story_inter.region_inter[i].x_offset = story.region[i].grid_x * STORY_REGION_SEPARATION_X * 2;
 	story_inter.region_inter[i].x_offset += (story.region[i].grid_y) * STORY_REGION_SEPARATION_X;
//		if ((story.region[i].grid_y) & 1)
//  	story_inter.region_inter[i].x_offset -= STORY_REGION_SEPARATION_X;
		story_inter.region_inter[i].y_offset = story.region[i].grid_y * STORY_REGION_SEPARATION_Y;

		story_inter.region_inter[i].area_index = story.region[i].area_index;

		story_inter.region_inter[i].region_highlight_time = 0;
	}

	for (i = 0; i < STORY_RIPPLES; i ++)
	{
//		story_inter.ripple_region [i] = -1;
		story_inter.ripple_time [i] = 0;

	}
/*
 story_inter.region_x_offset [1] = STORY_REGION_SEPARATION_X;
 story_inter.region_y_offset [1] = STORY_REGION_SEPARATION_Y;

 story_inter.region_x_offset [2] = STORY_REGION_SEPARATION_X;
 story_inter.region_y_offset [2] = -STORY_REGION_SEPARATION_Y;

 story_inter.region_x_offset [3] = STORY_REGION_SEPARATION_X * 2;
 story_inter.region_y_offset [3] = 0;
*/

	reset_region_screen_positions();


}


// called when entering story interface from start menu or game
void open_story_interface(void)
{

 inter.mode_button_available [MODE_BUTTON_SYSTEM] = 1;
 inter.mode_button_available [MODE_BUTTON_TEMPLATES] = 1;
 inter.mode_button_available [MODE_BUTTON_EDITOR] = 1;
 inter.mode_button_available [MODE_BUTTON_DESIGN] = 1;
 inter.mode_button_available [MODE_BUTTON_CLOSE] = 1;

	story_inter.region_mouse_over = -1;

 story_inter.region_selected = -1;
 story_inter.region_select_time = 0;


}


void draw_story_interface(void)
{

 al_set_target_bitmap(al_get_backbuffer(display));
 al_set_clipping_rectangle(0, 0, panel[PANEL_MAIN].w, panel[PANEL_MAIN].h);

 al_clear_to_color(al_map_rgb(10,20,40));

// al_set_clipping_rectangle(0, 0, panel[PANEL_MAIN].w, panel[PANEL_MAIN].h);

 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

 draw_story_regions();

 draw_story_boxes_etc();

 draw_panels();

 draw_mouse_cursor();

// al_show_mouse_cursor(display); // needs to be inside the loop because it doesn't appear to do anything if called while the pointer is off the display (e.g. if it's pressing the close window button)

// draw here...

 al_flip_display();

}


void story_input(void)
{

  get_ex_control(1); // 1 means pressing escape or the close window button closes the game. Need to fix.

  run_input();

  int mouse_x = ex_control.mouse_x_pixels;
  int mouse_y = ex_control.mouse_y_pixels;
  int just_pressed = (control.mouse_panel == PANEL_MAIN)
                     && (ex_control.mb_press [0] == BUTTON_JUST_PRESSED);

// TO DO: work out how to stop mouse clicks on the panel buttons register as being on the main panel if no other panels are open

  int i;
  float closest_distance = 100000;
  int closest_region = -1;

// 	story_inter.region_mouse_over = -1;

  if (mouse_x < panel[PANEL_MAIN].w)
		{
			for (i = 0; i < STORY_REGIONS; i ++)
			{
				if (!story.region[i].exists
				 || !story.region[i].visible)
						continue;
				float region_dist = abs(mouse_x - story_inter.region_inter[i].x_screen) + abs(mouse_y - story_inter.region_inter[i].y_screen);

				if (region_dist < STORY_REGION_SIZE * story_inter.zoom
					&& region_dist < closest_distance)
				{
					closest_distance = region_dist;
					closest_region = i;
				}
			}


 		if (story_inter.region_mouse_over != closest_region)
					story_inter.region_mouse_over_time = inter.running_time;


			story_inter.region_mouse_over = closest_region; // may be -1

			if (closest_region != -1)
			{
				story_inter.region_inter[closest_region].region_highlight_time = inter.running_time;
//				fpr("\n %i(%i,%i)", closest_region, story.region[closest_region].grid_x, story.region[closest_region].grid_y);
			}

		}
		 else
   	story_inter.region_mouse_over = -1;



		if (just_pressed)
		{

   if (story_inter.region_selected != -1
				&&	mouse_x >= GO_BUTTON_X
				&& mouse_x <= GO_BUTTON_X + GO_BUTTON_SIZE
				&& mouse_y >= GO_BUTTON_Y
				&& mouse_y <= GO_BUTTON_Y + GO_BUTTON_SIZE)
			{

       game.type = GAME_TYPE_MISSION;
       game.mission_index = story.region[story_inter.region_selected].mission_index;
       game.area_index = story.region[story_inter.region_selected].area_index;
							prepare_templates_for_new_game();
       prepare_for_mission(); // sets up w_init so that start_world will prepare the world for a mission
        // also loads in enemy templates and does other preparation for a mission
       new_world_from_world_init();
       generate_map_from_map_init();
//       generate_random_map(w_init.map_size_blocks, w_init.players, w_init.game_seed);
       start_world();
       run_game_from_menu();
//   				story_inter.region_selected = -1;
       return;


			}



			if (story_inter.region_mouse_over == -1)
			{
				story_inter.region_selected = -1;
//				story_inter.region_select_time = inter.running_time;
			}
			 else
				{
					if (story_inter.region_selected != story_inter.region_mouse_over)
					{
					 story_inter.region_selected = story_inter.region_mouse_over;
					 story_inter.region_select_time = inter.running_time;
					}
					for (i = 0; i < STORY_RIPPLES; i ++)
					{
						if (story_inter.ripple_time [i] < inter.running_time - STORY_RIPPLE_TIME)
					 {
					 	story_inter.ripple_time [i] = inter.running_time;
					 	story_inter.ripple_x [i] = story_inter.region_inter[story_inter.region_selected].x_screen;
					 	story_inter.ripple_y [i] = story_inter.region_inter[story_inter.region_selected].y_screen;
					 	break;
					 }
					}
// need sound here...
				}

		}

  run_panels();
  run_editor();


}


static void reset_region_screen_positions(void)
{

	story_inter.region_x_start = 180;
	story_inter.region_y_start = 500;

	int i;

	for (i = 0; i < STORY_REGIONS; i ++)
	{
		story_inter.region_inter[i].x_screen = story_inter.region_x_start + (story_inter.region_inter[i].x_offset * story_inter.zoom);
		story_inter.region_inter[i].y_screen = story_inter.region_y_start + (story_inter.region_inter[i].y_offset * story_inter.zoom);
	}

}


static void draw_story_regions(void)
{

	int i, j, k;

	int highlighted;

	int col_r, col_g, col_b;


	for (i = 0; i < STORY_REGIONS; i ++)
	{

		if (!story.region[i].exists // probably unnecessary considering visible check
			|| !story.region[i].visible)
			continue;

		if (story_inter.region_inter[i].region_highlight_time > inter.running_time - 16)
			highlighted = story_inter.region_inter[i].region_highlight_time + 16 - inter.running_time;
		  else
					highlighted = 0;


		highlighted *= 5;

		for (j = 0; j < STORY_RIPPLES; j ++)
		{
			if (story_inter.ripple_time [j] > inter.running_time - STORY_RIPPLE_TIME)
			{
				float dist_from_ripple_centre = hypot(story_inter.region_inter[i].y_screen - story_inter.ripple_y [j], story_inter.region_inter[i].x_screen - story_inter.ripple_x [j]);
				int ripple_size = (inter.running_time - story_inter.ripple_time [j] + 8) * 6;
				int ripple_highlight = 64 - abs(dist_from_ripple_centre - ripple_size);
				if (ripple_highlight > 0)
					highlighted += ripple_highlight / 4;
			}
		}


		if (story_inter.region_selected == i)
		{

		add_orthogonal_hexagon_story(0,
																															story_inter.region_inter[i].x_screen,
																															story_inter.region_inter[i].y_screen,
																															23 * story_inter.zoom,
																															al_map_rgb(180, 180, 180));
																															//al_map_rgb(120, 170, 200));

		}

		col_r = story_area_base_col [story_inter.region_inter[i].area_index] [0];
		col_g = story_area_base_col [story_inter.region_inter[i].area_index] [1];
		col_b = story_area_base_col [story_inter.region_inter[i].area_index] [2];

// unlocked but not yet defeated:
		if (!story.region[i].defeated)
		{
			col_r *= 0.7;
			col_g *= 0.7;
			col_b *= 0.7;
			for (j = 0; j < SRC_DIRECTIONS; j ++)
			{
				if (story.region[i].connect[j] != -1
					&& story.region[story.region[i].connect[j]].defeated)
				{
					int other_region = story.region[i].connect[j];
					float other_x = story_inter.region_inter[other_region].x_screen;
					float other_y = story_inter.region_inter[other_region].y_screen;
					float angle_from_other = atan2(story_inter.region_inter[i].y_screen - other_y, story_inter.region_inter[i].x_screen - other_x);
					float dist_from_other = hypot(story_inter.region_inter[i].y_screen - other_y, story_inter.region_inter[i].x_screen - other_x);
					for (k = 0; k < 4; k ++)
					{
					timestamp adjusted_running_time = inter.running_time + (k * 24);
					float extra_dist = ((adjusted_running_time + other_region * 8) % 96);
					extra_dist *= 0.3;

					float tri_x = other_x + cos(angle_from_other) * (dist_from_other / 8 + extra_dist);
					float tri_y = other_y + sin(angle_from_other) * (dist_from_other / 8 + extra_dist);

					int shade = (adjusted_running_time + other_region * 8) % 96;
					if (shade > 48)
					 shade = 96 - shade;

					float tri_prop = 0.5 + shade * 0.01;

     add_story_quad(1,
																								tri_x + cos(angle_from_other) * 6 * tri_prop * story_inter.zoom,
																								tri_y + sin(angle_from_other) * 6 * tri_prop * story_inter.zoom,
																								tri_x + cos(angle_from_other + PI * 0.7) * 5 * tri_prop * story_inter.zoom,
																								tri_y + sin(angle_from_other + PI * 0.7) * 5 * tri_prop * story_inter.zoom,
																								tri_x + cos(angle_from_other) * -1 * tri_prop * story_inter.zoom,
																								tri_y + sin(angle_from_other) * -1 * tri_prop * story_inter.zoom,
																								tri_x + cos(angle_from_other - PI * 0.7) * 5 * tri_prop * story_inter.zoom,
																								tri_y + sin(angle_from_other - PI * 0.7) * 5 * tri_prop * story_inter.zoom,
																								map_rgba(shade * 3, shade * 3, shade * 3, shade * 3));

/*
     add_story_triangle(1,
																								tri_x + cos(angle_from_other) * 6 * tri_prop * story_inter.zoom,
																								tri_y + sin(angle_from_other) * 6 * tri_prop * story_inter.zoom,
																								tri_x + cos(angle_from_other + PI * 0.7) * 5 * tri_prop * story_inter.zoom,
																								tri_y + sin(angle_from_other + PI * 0.7) * 5 * tri_prop * story_inter.zoom,
																								tri_x + cos(angle_from_other - PI * 0.7) * 5 * tri_prop * story_inter.zoom,
																								tri_y + sin(angle_from_other - PI * 0.7) * 5 * tri_prop * story_inter.zoom,
																								map_rgba(shade * 3, shade * 3, shade * 3, shade * 2));
																								*/
					}
				}
			}
		}

		add_orthogonal_hexagon_story(0,
																															story_inter.region_inter[i].x_screen,
																															story_inter.region_inter[i].y_screen,
																															21 * story_inter.zoom,
																															map_rgb(col_r + highlighted, col_g + highlighted, col_b + highlighted));

		if (story.region[i].capital)
		{
		col_r = story_area_base_col [story_inter.region_inter[i].area_index] [0] + 30;
		col_g = story_area_base_col [story_inter.region_inter[i].area_index] [1] + 30;
		col_b = story_area_base_col [story_inter.region_inter[i].area_index] [2] + 30;


		add_orthogonal_hexagon_story(0,
																															story_inter.region_inter[i].x_screen,
																															story_inter.region_inter[i].y_screen,
																															16 * story_inter.zoom,
																															map_rgb(col_r + highlighted, col_g + highlighted, col_b + highlighted));

		}
	}

 draw_vbuf();

// if (story_inter.region_mouse_over != -1)
//		draw_story_region_box(story_inter.region_mouse_over);

}

static void draw_story_boxes_etc(void)
{

 if (story_inter.region_mouse_over != -1)
	{
		draw_a_story_box(story_inter.region_mouse_over,
																			story_inter.region_inter[story_inter.region_mouse_over].x_screen + 120,
																			story_inter.region_inter[story_inter.region_mouse_over].y_screen - STORY_BOX_H / 2,
																			story_inter.region_mouse_over_time);
	}

 if (story_inter.region_selected != -1)
	{


 	int go_button_shade = SHADE_LOW;

 	if (ex_control.mouse_x_pixels >= GO_BUTTON_X
			&& ex_control.mouse_x_pixels <= GO_BUTTON_X + GO_BUTTON_SIZE
			&& ex_control.mouse_y_pixels >= GO_BUTTON_Y
			&& ex_control.mouse_y_pixels <= GO_BUTTON_Y + GO_BUTTON_SIZE)
				go_button_shade = SHADE_MED;

 	add_story_bquad(GO_BUTTON_X, FIXED_STORY_BOX_Y, GO_BUTTON_SIZE, GO_BUTTON_SIZE, 6, 12, colours.base_trans [COL_GREEN] [go_button_shade] [TRANS_MED]);

		draw_a_story_box(story_inter.region_selected, FIXED_STORY_BOX_X, FIXED_STORY_BOX_Y, story_inter.region_select_time);

  al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX],
																GO_BUTTON_X + GO_BUTTON_SIZE / 2,
																GO_BUTTON_Y + GO_BUTTON_SIZE / 2 - 6,
																ALLEGRO_ALIGN_CENTRE, "GO >>");


	}

}

static void draw_a_story_box(int region_index, float draw_x, float draw_y, timestamp select_time)
{

 int area_index = story.region[region_index].area_index;
 int highlight = select_time + 4 - inter.running_time;

 if (highlight < 0)
		highlight = 0;

	highlight *= 6;

 ALLEGRO_COLOR base_box_col = map_rgba(story_area_base_col [area_index] [0] + highlight,
																																							story_area_base_col [area_index] [1] + highlight,
																																							story_area_base_col [area_index] [2] + highlight,
																																							80 + highlight);



	add_story_bquad(draw_x, draw_y, STORY_BOX_W, STORY_BOX_H, 7, 3, base_box_col);

	add_story_bquad(draw_x + 4, draw_y + 4, STORY_BOX_W - 8, 22, 6, 2, base_box_col);


 draw_vbuf();

	float line_x = draw_x + 10;
	float line_y = draw_y + 10;

 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], line_x, line_y, ALLEGRO_ALIGN_LEFT, "Region %i mission %i", region_index, story.region[region_index].mission_index);
 line_y += 22;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], line_x, line_y, ALLEGRO_ALIGN_LEFT, "Local conditions:");
 switch(story.region[region_index].area_index)
 {
 default:
 	case AREA_BLUE:
 	case AREA_TUTORIAL:
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MED], line_x + 122, line_y, ALLEGRO_ALIGN_LEFT, "none"); break;
 	case AREA_GREEN:
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREEN] [SHADE_HIGH], line_x + 122, line_y, ALLEGRO_ALIGN_LEFT, "static environment");
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREEN] [SHADE_MED], line_x + 122, line_y + 16, ALLEGRO_ALIGN_LEFT, "(static processes cost less data)");
   break;
 	case AREA_YELLOW:
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_ORANGE] [SHADE_HIGH], line_x + 122, line_y, ALLEGRO_ALIGN_LEFT, "fragile processes");
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_ORANGE] [SHADE_MED], line_x + 122, line_y + 16, ALLEGRO_ALIGN_LEFT, "(all components have 60 integrity)");
   break;
 }


}


float vertex_list [6] [2];

static void add_orthogonal_hexagon_story(int layer, float x, float y, float size, ALLEGRO_COLOR col1)
{

#define X_MULT 0.92
#define Y_MULT 0.5

		vertex_list [0] [0] = x;
		vertex_list [0] [1] = y - 1.0 * size;
		vertex_list [1] [0] = x + X_MULT * size;
		vertex_list [1] [1] = y - Y_MULT * size;
		vertex_list [2] [0] = x + X_MULT * size;
		vertex_list [2] [1] = y + Y_MULT * size;
		vertex_list [3] [0] = x;
		vertex_list [3] [1] = y + 1.0 * size;
		vertex_list [4] [0] = x - X_MULT * size;
		vertex_list [4] [1] = y + Y_MULT * size;
		vertex_list [5] [0] = x - X_MULT * size;
		vertex_list [5] [1] = y - Y_MULT * size;


		vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = vertex_list [0] [0];
 	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = vertex_list [0] [1];
  vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col1;
  vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
  vbuf.vertex_pos_triangle++;

		vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = vertex_list [1] [0];
 	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = vertex_list [1] [1];
  vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col1;
  vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
  vbuf.vertex_pos_triangle++;

		vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = vertex_list [5] [0];
 	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = vertex_list [5] [1];
  vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col1;
  vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
  vbuf.vertex_pos_triangle++;

		vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = vertex_list [2] [0];
 	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = vertex_list [2] [1];
  vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col1;
  vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
  vbuf.vertex_pos_triangle++;

		vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = vertex_list [1] [0];
 	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = vertex_list [1] [1];
  vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col1;
  vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
  vbuf.vertex_pos_triangle++;

		vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = vertex_list [5] [0];
 	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = vertex_list [5] [1];
  vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col1;
  vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
  vbuf.vertex_pos_triangle++;


		vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = vertex_list [2] [0];
 	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = vertex_list [2] [1];
  vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col1;
  vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
  vbuf.vertex_pos_triangle++;

		vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = vertex_list [4] [0];
 	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = vertex_list [4] [1];
  vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col1;
  vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
  vbuf.vertex_pos_triangle++;

		vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = vertex_list [5] [0];
 	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = vertex_list [5] [1];
  vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col1;
  vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
  vbuf.vertex_pos_triangle++;


		vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = vertex_list [2] [0];
 	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = vertex_list [2] [1];
  vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col1;
  vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
  vbuf.vertex_pos_triangle++;

		vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = vertex_list [3] [0];
 	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = vertex_list [3] [1];
  vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col1;
  vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
  vbuf.vertex_pos_triangle++;

		vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = vertex_list [4] [0];
 	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = vertex_list [4] [1];
  vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col1;
  vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
  vbuf.vertex_pos_triangle++;

}
/*
static void add_story_triangle(int layer, float xa, float ya, float xb, float yb, float xc, float yc, ALLEGRO_COLOR col)
{

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xb;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yb;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xc;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yc;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;



}
*/

static void add_story_quad(int layer, float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, ALLEGRO_COLOR col)
{

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xb;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yb;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xc;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yc;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;




	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xc;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yc;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xd;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yd;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;


}


static void add_story_bquad(float xa, float ya, float wa, float ha, float corner1, float corner2, ALLEGRO_COLOR col)
{

 float xb = xa + wa;
 float yb = ya + ha;


#define DESIGN_QUAD_LAYER 4

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya + corner1;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yb - corner2;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa + corner2;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yb;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

// 1A
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya + corner1;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa + corner1;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa + corner2;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yb;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

// 2
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa + corner1;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa + corner2;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yb;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xb - corner1;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yb;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

// 3
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa + corner1;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xb - corner1;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yb;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xb;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yb - corner1;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;


// 4
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa + corner1;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xb;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yb - corner1;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xb;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya + corner2;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

// 5
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa + corner1;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xb - corner2;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xb;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya + corner2;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;



}

