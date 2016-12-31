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

#include "x_sound.h"

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

char *story_unlock_name [UNLOCKS] =
{
"Nothing", // UNLOCK_NONE,

//"Key", // UNLOCK_KEY,

"Mobile core + component", // UNLOCK_CORE_MOBILE_1,
"Mobile core + component", // UNLOCK_CORE_MOBILE_2,
"Mobile core + component", // UNLOCK_CORE_MOBILE_3,
"Mobile core + component", // UNLOCK_CORE_MOBILE_4,
//"Mobile core", // UNLOCK_CORE_MOBILE_5,

"Static core + component", // UNLOCK_CORE_STATIC_1,
"Static core + component", // UNLOCK_CORE_STATIC_2,
//"Static core", // UNLOCK_CORE_STATIC_3,

//"Components", // UNLOCK_COMPONENTS_1,
//"Components", // UNLOCK_COMPONENTS_2,

"Object: interface", // UNLOCK_OBJECT_INTERFACE,
"Object: repair_other", // UNLOCK_OBJECT_REPAIR_OTHER,
"Object: stability", // UNLOCK_OBJECT_STABILITY,

"Object: pulse_l + burst_l", // UNLOCK_OBJECT_PULSE_L,
"Object: pulse_xl + burst_xl", // UNLOCK_OBJECT_PULSE_XL,
//"Object: burst_xl", // UNLOCK_OBJECT_BURST_XL,

"Object: stream + stream_dir", // UNLOCK_OBJECT_STREAM,
"Object: spike", // UNLOCK_OBJECT_SPIKE,
"Object: slice", // UNLOCK_OBJECT_SLICE,
"Object: ultra + ultra_dir", // UNLOCK_OBJECT_ULTRA,

//"", // UNLOCKS



};

/*

#define STORY_REGION_SIZE 34

#define STORY_REGION_SEPARATION_X 21
#define STORY_REGION_SEPARATION_Y 36
*/

#define STORY_REGION_SIZE 24

#define STORY_REGION_SEPARATION_X 39
#define STORY_REGION_SEPARATION_Y 55


#define FIXED_STORY_BOX_X 20
#define FIXED_STORY_BOX_Y 550

#define STORY_BOX_W 390
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


#define REGION_LINE_SEGMENTS 5

struct region_line_segment_struct
{
	int direction;
	float length;
};

struct region_line_segment_struct region_line_segment [REGION_LINE_SEGMENTS];

static void reset_region_screen_positions(void);
static void draw_story_regions(void);
//static void draw_story_region_box(int region_index);
static void draw_story_boxes_etc(void);
static void draw_a_story_box(int region_index, float draw_x, float draw_y, timestamp select_time);
static void add_orthogonal_hexagon_story(int layer, float x, float y, float size, ALLEGRO_COLOR col1);
static void add_story_bquad(float xa, float ya, float wa, float ha, float corner1, float corner2, ALLEGRO_COLOR col);
//static void add_story_triangle(int layer, float xa, float ya, float xb, float yb, float xc, float yc, ALLEGRO_COLOR col);
static void add_story_quad(int layer, float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, ALLEGRO_COLOR col);

static void init_region_lines(void);
//static void add_region_line_vertex(int line_index, int vertex_index, float x, float y);
static void draw_region_lines(void);
static void story_add_line(float x, float y, float xa, float ya, int vdir1, int vdir2, ALLEGRO_COLOR col);
static void finish_region_line(int line_index);
/*static void add_three_part_line(int line_index,
																																float line_start_x,
																																float line_start_y,
																																int start_dir,
																																float line_end_x,
																																float line_end_y,
																																int end_dir,
																																float line_length1,
																																int join_dir);*/
static void set_region_line(int starting_region, float along_edge, int segments, float line_width);
void add_region_line_triangle(float v1x, float v1y, float v2x, float v2y, float v3x, float v3y, ALLEGRO_COLOR col);

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
 	story_inter.region_inter[i].x_offset += story.region[i].adjust_x;
//		if ((story.region[i].grid_y) & 1)
//  	story_inter.region_inter[i].x_offset -= STORY_REGION_SEPARATION_X;
		story_inter.region_inter[i].y_offset = story.region[i].grid_y * STORY_REGION_SEPARATION_Y;
 	story_inter.region_inter[i].y_offset += story.region[i].adjust_y;

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

	init_region_lines();


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
/*
 if (ex_control.mb_press [0] > 0)
	{
																							story_inter.zoom += 0.01;

	reset_region_screen_positions();

	init_region_lines();
	}

 if (ex_control.mb_press [1] > 0)
	{
																							story_inter.zoom -= 0.01;


	reset_region_screen_positions();

	init_region_lines();
	}
*/

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
     	play_interface_sound(SAMPLE_BLIP2, TONE_2C);
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

	story_inter.region_x_start = 150;
	story_inter.region_y_start = 460;

	int i;

	for (i = 0; i < STORY_REGIONS; i ++)
	{
		story_inter.region_inter[i].x_screen = story_inter.region_x_start + (story_inter.region_inter[i].x_offset * story_inter.zoom);
		story_inter.region_inter[i].y_screen = story_inter.region_y_start + (story_inter.region_inter[i].y_offset * story_inter.zoom);
	}
/*
srand(30);

	for (i = 0; i < STORY_REGIONS; i ++)
	{
		story_inter.region_inter[i].x_screen += grand(26) * story_inter.zoom;// - grand(6);
		story_inter.region_inter[i].y_screen += grand(26) * story_inter.zoom;// - grand(6);
	}
*/

}


static void draw_story_regions(void)
{

	draw_region_lines();


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
																															(STORY_REGION_SIZE + 4) * story_inter.zoom,
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
																															STORY_REGION_SIZE * story_inter.zoom,
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
// Will just say region %i
 line_y += 28;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], line_x, line_y, ALLEGRO_ALIGN_LEFT, "Function");
 int region_text_col;
 switch(story.region[region_index].area_index)
 {
#define FUNCTION_X 72
#define LINE_HEIGHT_MID 18
 default:
 	case AREA_TUTORIAL:
 		region_text_col = COL_BLUE;
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [region_text_col] [SHADE_HIGH], line_x + FUNCTION_X, line_y, ALLEGRO_ALIGN_LEFT, "Initialisation");
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [region_text_col] [SHADE_MED], line_x + FUNCTION_X, line_y + LINE_HEIGHT_MID, ALLEGRO_ALIGN_LEFT, "Learn how to interact with your environment");
   break;
 	case AREA_BLUE:
 		region_text_col = COL_BLUE;
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [region_text_col] [SHADE_HIGH], line_x + FUNCTION_X, line_y, ALLEGRO_ALIGN_LEFT, "Deep learning");
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [region_text_col] [SHADE_MED], line_x + FUNCTION_X, line_y + LINE_HEIGHT_MID, ALLEGRO_ALIGN_LEFT, "X");
   break;
 	case AREA_GREEN:
 		region_text_col = COL_GREEN;
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [region_text_col] [SHADE_HIGH], line_x + FUNCTION_X, line_y, ALLEGRO_ALIGN_LEFT, "Library");
   break;
 	case AREA_YELLOW:
 		region_text_col = COL_YELLOW;
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [region_text_col] [SHADE_HIGH], line_x + FUNCTION_X, line_y, ALLEGRO_ALIGN_LEFT, "Supervision");
   break;
 	case AREA_PURPLE:
 		region_text_col = COL_PURPLE;
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [region_text_col] [SHADE_HIGH], line_x + FUNCTION_X, line_y, ALLEGRO_ALIGN_LEFT, "Symbolic resonance");
   break;
 	case AREA_ORANGE:
 		region_text_col = COL_ORANGE;
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [region_text_col] [SHADE_HIGH], line_x + FUNCTION_X, line_y, ALLEGRO_ALIGN_LEFT, "Input/Output filter");
   break;
 	case AREA_RED:
 		region_text_col = COL_RED;
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [region_text_col] [SHADE_MAX], line_x + FUNCTION_X, line_y, ALLEGRO_ALIGN_LEFT, "FIREWALL");
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [region_text_col] [SHADE_MED], line_x + FUNCTION_X, line_y + LINE_HEIGHT_MID, ALLEGRO_ALIGN_LEFT, "Is this the way out?");
   break;
 }

 line_y += LINE_HEIGHT_MID + 28;
 if (story.region[region_index].defeated)
	{
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], line_x, line_y, ALLEGRO_ALIGN_LEFT, "You have defeated this region.");
  line_y += 28;
  if (story.region[region_index].area_index != AREA_TUTORIAL && story.region[region_index].area_index != AREA_RED)
		{
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MED], line_x, line_y, ALLEGRO_ALIGN_LEFT, "Unlocked");
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [region_text_col] [SHADE_MED], line_x + FUNCTION_X, line_y, ALLEGRO_ALIGN_LEFT, "%s", story_unlock_name [story.region[region_index].unlock_index]);
		}
	}
   else
			{
    al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_LOW], line_x, line_y, ALLEGRO_ALIGN_LEFT, "You have not defeated this region.");
    line_y += 28;
    if (story.region[region_index].area_index != AREA_TUTORIAL && story.region[region_index].area_index != AREA_RED)
  		{
     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MED], line_x, line_y, ALLEGRO_ALIGN_LEFT, "Unlock");
     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [region_text_col] [SHADE_MED], line_x + FUNCTION_X, line_y, ALLEGRO_ALIGN_LEFT, "%s", story_unlock_name [story.region[region_index].unlock_index]);
  		}
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



/*

Region lines


*/

#define LINE_VERTICES 8
#define REGION_LINES 64

#define X_MULT_2 (X_MULT*2)
#define Y_MULT_2 (Y_MULT*2)

enum
{
// left-to-right
LINE_R_TO_L,
LINE_R_TO_UL,
LINE_R_TO_DL,
LINE_UR_TO_L,
LINE_UR_TO_UL,
LINE_DR_TO_L,
LINE_DR_TO_DL,

// up-right
LINE_UL_TO_L,
LINE_UL_TO_DL,
LINE_UR_TO_DL,
LINE_UR_TO_DR,
LINE_R_TO_DR,

};

enum
{
LINE_EDGE_R,
LINE_EDGE_DR,
LINE_EDGE_DL,
LINE_EDGE_L,
LINE_EDGE_UL,
LINE_EDGE_UR,

LINE_EDGE_U,
LINE_EDGE_D,

};

struct region_line_struct
{
	int line_vertices; // if 0, line doesn't exist

	int region1, region2;

	float line_vertex_x [LINE_VERTICES];
	float line_vertex_y [LINE_VERTICES];
	int line_vertex_direction [LINE_VERTICES];


};

struct region_line_struct region_line [REGION_LINES];

int add_region_line(int region1, int region2, int line_type, float line_length1, float along_edge1, float along_edge2);

static void init_region_lines(void)
{

	int i, j;

	for (i = 0; i < REGION_LINES; i ++)
	{
		region_line [i].line_vertices = 0;
		for (j = 0; j < LINE_VERTICES; j ++)
		{
		 region_line [i].line_vertex_x [j] = -1;
		}
	}
/*
 add_region_line(0, 1, LINE_R_TO_L, 12, 8, 16);
 add_region_line(0, 1, LINE_DR_TO_DL, 15, 14, 10);
 add_region_line(0, 1, LINE_DR_TO_L, 8, 4, 15);

 add_region_line(1, 2, LINE_UR_TO_DL, 8, 8, 12);
*/


}

int add_region_line(int region1, int region2, int line_type, float line_length1, float along_edge1, float along_edge2)
{

	int line_index = 0;

	while(region_line[line_index].line_vertices > 0)
	{
		line_index ++;
		sancheck(line_index, 0, REGION_LINES, "add_region_line: line_index");
	};


			add_orthogonal_hexagon_story(0,
																															story_inter.region_inter[line_index].x_screen,
																															story_inter.region_inter[line_index].y_screen,
																															(STORY_REGION_SIZE + 4) * story_inter.zoom,
																															al_map_rgb(180, 180, 180));

 float region1_x = story_inter.region_inter[region1].x_screen;
 float region1_y = story_inter.region_inter[region1].y_screen;
 float region2_x = story_inter.region_inter[region2].x_screen;
 float region2_y = story_inter.region_inter[region2].y_screen;

 float line_start_x = 0, line_start_y = 0;
 float line_end_x = 0, line_end_y = 0;

 float hex_vertex_x = 0, hex_vertex_y = 0;
 float hex_size = STORY_REGION_SIZE * story_inter.zoom;

 line_length1 *= story_inter.zoom;
 along_edge1 *= story_inter.zoom;
 along_edge2 *= story_inter.zoom;

// float line_length1 = 15 + grand(8);
// float line_length2 = 5 + grand(8);
// float line_length3 = 5 + grand(8);
// float line_length4 = 5 + grand(8);

//#define X_MULT 0.92
//#define Y_MULT 0.5

	switch(line_type)
	{
		case LINE_R_TO_L:
		case LINE_R_TO_UL:
		case LINE_R_TO_DL:
		case LINE_R_TO_DR:
			hex_vertex_x = region1_x + hex_size * X_MULT;
			hex_vertex_y = region1_y - hex_size * Y_MULT;
			line_start_x = hex_vertex_x;
			line_start_y = hex_vertex_y + along_edge1;
			break;
		case LINE_UR_TO_L:
		case LINE_UR_TO_UL:
  case LINE_UR_TO_DL:
  case LINE_UR_TO_DR:
			hex_vertex_x = region1_x;
			hex_vertex_y = region1_y - hex_size;
			line_start_x = hex_vertex_x + along_edge1 * X_MULT;
			line_start_y = hex_vertex_y + along_edge1 * Y_MULT;
			break;
		case LINE_DR_TO_L:
		case LINE_DR_TO_DL:
			hex_vertex_x = region1_x + hex_size * X_MULT;
			hex_vertex_y = region1_y + hex_size * Y_MULT;
			line_start_x = hex_vertex_x - along_edge1 * X_MULT;
			line_start_y = hex_vertex_y + along_edge1 * Y_MULT;
			break;
  case LINE_UL_TO_L:
  case LINE_UL_TO_DL:
			hex_vertex_x = region1_x - hex_size * X_MULT;
			hex_vertex_y = region1_y - hex_size * Y_MULT;
			line_start_x = hex_vertex_x + along_edge1 * X_MULT;
			line_start_y = hex_vertex_y - along_edge1 * Y_MULT;
			break;




	}


	switch(line_type)
	{
		case LINE_R_TO_L:
		case LINE_UR_TO_L:
		case LINE_DR_TO_L:
		case LINE_UL_TO_L:
			hex_vertex_x = region2_x - hex_size * X_MULT;
			hex_vertex_y = region2_y - hex_size * Y_MULT;
			line_end_x = hex_vertex_x;
			line_end_y = hex_vertex_y + along_edge2;
			break;
		case LINE_R_TO_UL:
		case LINE_UR_TO_UL:
			hex_vertex_x = region2_x - hex_size * X_MULT;
			hex_vertex_y = region2_y - hex_size * Y_MULT;
			line_end_x = hex_vertex_x + along_edge2 * X_MULT;
			line_end_y = hex_vertex_y - along_edge2 * Y_MULT;
			break;
		case LINE_R_TO_DL:
		case LINE_DR_TO_DL:
		case LINE_UL_TO_DL:
		case LINE_UR_TO_DL:
			hex_vertex_x = region2_x;
			hex_vertex_y = region2_y + hex_size;
			line_end_x = hex_vertex_x - along_edge2 * X_MULT;
			line_end_y = hex_vertex_y - along_edge2 * Y_MULT;
			break;
		case LINE_UR_TO_DR:
		case LINE_R_TO_DR:
			hex_vertex_x = region2_x + hex_size * X_MULT;
			hex_vertex_y = region2_y + hex_size * Y_MULT;
			line_end_x = hex_vertex_x - along_edge2 * X_MULT;
			line_end_y = hex_vertex_y + along_edge2 * Y_MULT;
			break;



	}

// float temp_x, temp_y;


 switch(line_type)
 {
	 case LINE_R_TO_L:
		{


// R to L is R, UR/DR, R
    	 region_line[line_index].line_vertex_x [0] = line_start_x;
	     region_line[line_index].line_vertex_y [0] = line_start_y;
	     region_line[line_index].line_vertex_direction [0] = LINE_EDGE_L;
    	 region_line[line_index].line_vertex_x [1] = line_start_x + line_length1;
	     region_line[line_index].line_vertex_y [1] = line_start_y;
	     region_line[line_index].line_vertex_direction [1] = LINE_EDGE_L;
//    	region_line[line_index].line_vertex_x [2] = line_start_x + line_length1;
//	    region_line[line_index].line_vertex_y [2] = line_end_y;
      float separation = line_end_y - line_start_y;
      separation = fabs(separation);
    	 region_line[line_index].line_vertex_x [2] = region_line[line_index].line_vertex_x [1] + separation * X_MULT_2;
	     region_line[line_index].line_vertex_y [2] = line_end_y;
	     region_line[line_index].line_vertex_direction [2] = LINE_EDGE_R;
    	 region_line[line_index].line_vertex_x [3] = line_end_x;
	     region_line[line_index].line_vertex_y [3] = line_end_y;
	     region_line[line_index].line_vertex_direction [3] = LINE_EDGE_L;

			 	 finish_region_line(line_index);
		 }
		 break;

	 case LINE_R_TO_UL:
		{
// R to UL is R, UR/DR, R, DR



    	 region_line[line_index].line_vertex_x [0] = line_start_x;
	     region_line[line_index].line_vertex_y [0] = line_start_y;
	     region_line[line_index].line_vertex_direction [0] = LINE_EDGE_L;
    	 region_line[line_index].line_vertex_x [1] = line_start_x + line_length1;
	     region_line[line_index].line_vertex_y [1] = line_start_y;
	     region_line[line_index].line_vertex_direction [1] = LINE_EDGE_L;
// now work out end line (for now use same line length as first line):
    	 region_line[line_index].line_vertex_x [4] = line_end_x;
	     region_line[line_index].line_vertex_y [4] = line_end_y;
	     region_line[line_index].line_vertex_direction [4] = LINE_EDGE_UL;
    	 region_line[line_index].line_vertex_x [3] = line_end_x - line_length1 * X_MULT;
	     region_line[line_index].line_vertex_y [3] = line_end_y - line_length1 * Y_MULT;
	     region_line[line_index].line_vertex_direction [3] = LINE_EDGE_L;

      float separation = region_line[line_index].line_vertex_y [3] - line_start_y;
      separation = fabs(separation);
    	 region_line[line_index].line_vertex_x [2] = region_line[line_index].line_vertex_x [1] + separation * X_MULT_2;
	     region_line[line_index].line_vertex_y [2] = region_line[line_index].line_vertex_y [3]; //region_line[line_index].line_vertex_y [1] + separation * Y_MULT_2;
	     region_line[line_index].line_vertex_direction [2] = LINE_EDGE_R;

			 	 finish_region_line(line_index);
		 }
		 break;


	 case LINE_UR_TO_L:
			{
// UR to L is UR, DR, R


    	 region_line[line_index].line_vertex_x [0] = line_start_x;
	     region_line[line_index].line_vertex_y [0] = line_start_y;
	     region_line[line_index].line_vertex_direction [0] = LINE_EDGE_UR;
    	 region_line[line_index].line_vertex_x [1] = line_start_x + line_length1 * X_MULT;
	     region_line[line_index].line_vertex_y [1] = line_start_y - line_length1 * Y_MULT;
	     region_line[line_index].line_vertex_direction [1] = LINE_EDGE_L;
//    	region_line[line_index].line_vertex_x [2] = line_start_x + line_length1;
//	    region_line[line_index].line_vertex_y [2] = line_end_y;
      float separation = line_end_y - region_line[line_index].line_vertex_y [1];
      separation = fabs(separation);
    	 region_line[line_index].line_vertex_x [2] = region_line[line_index].line_vertex_x [1] + separation * X_MULT_2;
	     region_line[line_index].line_vertex_y [2] = line_end_y;
	     region_line[line_index].line_vertex_direction [2] = LINE_EDGE_R;
    	 region_line[line_index].line_vertex_x [3] = line_end_x;
	     region_line[line_index].line_vertex_y [3] = line_end_y;
	     region_line[line_index].line_vertex_direction [3] = LINE_EDGE_L;

			 	 finish_region_line(line_index);
		 }
		 break;


	 case LINE_UR_TO_UL:
			{
// UR to UL is UR, R, DR


    	 region_line[line_index].line_vertex_x [0] = line_start_x;
	     region_line[line_index].line_vertex_y [0] = line_start_y;
	     region_line[line_index].line_vertex_direction [0] = LINE_EDGE_UR;
    	 region_line[line_index].line_vertex_x [1] = line_start_x + line_length1 * X_MULT;
	     region_line[line_index].line_vertex_y [1] = line_start_y - line_length1 * Y_MULT;
	     region_line[line_index].line_vertex_direction [1] = LINE_EDGE_L;
      float separation = line_end_y - region_line[line_index].line_vertex_y [1]; // separation is already zoomed
      separation = fabs(separation);
    	 region_line[line_index].line_vertex_x [2] = line_end_x - separation * X_MULT_2;// * story_inter.zoom;
	     region_line[line_index].line_vertex_y [2] = region_line[line_index].line_vertex_y [1];
	     region_line[line_index].line_vertex_direction [2] = LINE_EDGE_R;
    	 region_line[line_index].line_vertex_x [3] = line_end_x;
	     region_line[line_index].line_vertex_y [3] = line_end_y;
	     region_line[line_index].line_vertex_direction [3] = LINE_EDGE_DR;

			 	 finish_region_line(line_index);
		 }
		 break;

		case LINE_DR_TO_L:
			{
// DR to L is DR, UR, R


    	 region_line[line_index].line_vertex_x [0] = line_start_x;
	     region_line[line_index].line_vertex_y [0] = line_start_y;
	     region_line[line_index].line_vertex_direction [0] = LINE_EDGE_DR;
    	 region_line[line_index].line_vertex_x [1] = line_start_x + line_length1 * X_MULT;
	     region_line[line_index].line_vertex_y [1] = line_start_y + line_length1 * Y_MULT;
	     region_line[line_index].line_vertex_direction [1] = LINE_EDGE_L;
//    	region_line[line_index].line_vertex_x [2] = line_start_x + line_length1;
//	    region_line[line_index].line_vertex_y [2] = line_end_y;
      float separation = line_end_y - region_line[line_index].line_vertex_y [1];
      separation = fabs(separation);
    	 region_line[line_index].line_vertex_x [2] = region_line[line_index].line_vertex_x [1] + separation * X_MULT_2;
	     region_line[line_index].line_vertex_y [2] = line_end_y;
	     region_line[line_index].line_vertex_direction [2] = LINE_EDGE_R;
    	 region_line[line_index].line_vertex_x [3] = line_end_x;
	     region_line[line_index].line_vertex_y [3] = line_end_y;
	     region_line[line_index].line_vertex_direction [3] = LINE_EDGE_L;

			 	 finish_region_line(line_index);
		 }
		 break;

		case LINE_DR_TO_DL:
			{
// DR to DL is DR, R, UR



    	 region_line[line_index].line_vertex_x [0] = line_start_x;
	     region_line[line_index].line_vertex_y [0] = line_start_y;
	     region_line[line_index].line_vertex_direction [0] = LINE_EDGE_DR;
    	 region_line[line_index].line_vertex_x [1] = line_start_x + line_length1 * X_MULT;
	     region_line[line_index].line_vertex_y [1] = line_start_y + line_length1 * Y_MULT;
	     region_line[line_index].line_vertex_direction [1] = LINE_EDGE_L;
      float separation = line_end_y - region_line[line_index].line_vertex_y [1]; // separation is already zoomed
      separation = fabs(separation);
    	 region_line[line_index].line_vertex_x [2] = line_end_x - separation * X_MULT_2;// * story_inter.zoom;
	     region_line[line_index].line_vertex_y [2] = region_line[line_index].line_vertex_y [1];
	     region_line[line_index].line_vertex_direction [2] = LINE_EDGE_R;
    	 region_line[line_index].line_vertex_x [3] = line_end_x;
	     region_line[line_index].line_vertex_y [3] = line_end_y;
	     region_line[line_index].line_vertex_direction [3] = LINE_EDGE_UR;

			 	 finish_region_line(line_index);
		 }
		 break;


		case LINE_UL_TO_L:
			{
// UL to L is UL, U, UR, R

    	 region_line[line_index].line_vertex_x [0] = line_start_x;
	     region_line[line_index].line_vertex_y [0] = line_start_y;
	     region_line[line_index].line_vertex_direction [0] = LINE_EDGE_UL;
    	 region_line[line_index].line_vertex_x [1] = line_start_x - line_length1 * X_MULT;
	     region_line[line_index].line_vertex_y [1] = line_start_y - line_length1 * Y_MULT;
	     region_line[line_index].line_vertex_direction [1] = LINE_EDGE_UL;
    	 region_line[line_index].line_vertex_x [2] = region_line[line_index].line_vertex_x [1];
	     region_line[line_index].line_vertex_y [2] = region_line[line_index].line_vertex_y [1] - 10 * story_inter.zoom;
	     region_line[line_index].line_vertex_direction [2] = LINE_EDGE_DL;


      float separation = line_end_y - region_line[line_index].line_vertex_y [2]; // separation is already zoomed
      separation = fabs(separation);
    	 region_line[line_index].line_vertex_x [3] = region_line[line_index].line_vertex_x [2] + separation * X_MULT_2;// * story_inter.zoom;
	     region_line[line_index].line_vertex_y [3] = line_end_y;
	     region_line[line_index].line_vertex_direction [3] = LINE_EDGE_L;
    	 region_line[line_index].line_vertex_x [4] = line_end_x;
	     region_line[line_index].line_vertex_y [4] = line_end_y;
	     region_line[line_index].line_vertex_direction [4] = LINE_EDGE_R;

			 	 finish_region_line(line_index);
		 }
		 break;

		case LINE_UR_TO_DL:
			{
// UR to DL is UR, U, UR



    	 region_line[line_index].line_vertex_x [0] = line_start_x;
	     region_line[line_index].line_vertex_y [0] = line_start_y;
	     region_line[line_index].line_vertex_direction [0] = LINE_EDGE_UR;
    	 region_line[line_index].line_vertex_x [1] = line_start_x + line_length1 * X_MULT;
	     region_line[line_index].line_vertex_y [1] = line_start_y - line_length1 * Y_MULT;
	     region_line[line_index].line_vertex_direction [1] = LINE_EDGE_UR;
      float separation = line_end_x - region_line[line_index].line_vertex_x [1]; // separation is already zoomed
      separation = fabs(separation);
    	 region_line[line_index].line_vertex_x [2] = region_line[line_index].line_vertex_x [1];
	     region_line[line_index].line_vertex_y [2] = line_end_y + separation * Y_MULT;
	     region_line[line_index].line_vertex_direction [2] = LINE_EDGE_DL;
    	 region_line[line_index].line_vertex_x [3] = line_end_x;
	     region_line[line_index].line_vertex_y [3] = line_end_y;
	     region_line[line_index].line_vertex_direction [3] = LINE_EDGE_UR;

			 	 finish_region_line(line_index);
		 }
		 break;


		case LINE_R_TO_DR:
			{
// R to DR is R, UR, U, UL

    	 region_line[line_index].line_vertex_x [0] = line_start_x;
	     region_line[line_index].line_vertex_y [0] = line_start_y;
	     region_line[line_index].line_vertex_direction [0] = LINE_EDGE_L;
    	 region_line[line_index].line_vertex_x [1] = line_start_x + line_length1;
	     region_line[line_index].line_vertex_y [1] = line_start_y;
	     region_line[line_index].line_vertex_direction [1] = LINE_EDGE_UR;
    	 region_line[line_index].line_vertex_x [2] = region_line[line_index].line_vertex_x [1];
	     region_line[line_index].line_vertex_y [2] = region_line[line_index].line_vertex_y [1] - 10 * story_inter.zoom;
	     region_line[line_index].line_vertex_direction [2] = LINE_EDGE_DL;


      float separation = line_end_y - region_line[line_index].line_vertex_y [2]; // separation is already zoomed
      separation = fabs(separation);
    	 region_line[line_index].line_vertex_x [3] = region_line[line_index].line_vertex_x [2] + separation * X_MULT_2;// * story_inter.zoom;
	     region_line[line_index].line_vertex_y [3] = line_end_y;
	     region_line[line_index].line_vertex_direction [3] = LINE_EDGE_L;
    	 region_line[line_index].line_vertex_x [4] = line_end_x;
	     region_line[line_index].line_vertex_y [4] = line_end_y;
	     region_line[line_index].line_vertex_direction [4] = LINE_EDGE_R;

			 	 finish_region_line(line_index);
		 }
		 break;



//LINE_UR_TO_DL,
//LINE_UR_TO_DR,
//LINE_R_TO_DR,




/*

LINE_UL_TO_L,
LINE_UL_TO_DL,
LINE_UR_TO_DL,
LINE_UR_TO_DR,
LINE_R_TO_DR,
*/

 }


 return line_index;

}
/*
static void add_three_part_line(int line_index,
																																float line_start_x,
																																float line_start_y,
																																int start_dir,
																																float line_end_x,
																																float line_end_y,
																																int end_dir,
																																float line_length1,
																																int join_dir)
{

	    	region_line[line_index].line_vertex_x [0] = line_start_x;
	     region_line[line_index].line_vertex_y [0] = line_start_y;
	     region_line[line_index].line_vertex_direction [0] = start_dir;
	     switch(start_dir)
	     {
	     	case LINE_EDGE_DR:
    	   region_line[line_index].line_vertex_x [1] = line_start_x + line_length1 * X_MULT;
	       region_line[line_index].line_vertex_y [1] = line_start_y + line_length1 * Y_MULT;
	       region_line[line_index].line_vertex_direction [1] = LINE_EDGE_L;
	       break;
	      case LINE_EDGE_UR:
    	   region_line[line_index].line_vertex_x [1] = line_start_x + line_length1 * X_MULT;
	       region_line[line_index].line_vertex_y [1] = line_start_y - line_length1 * Y_MULT;
	       region_line[line_index].line_vertex_direction [1] = LINE_EDGE_L;
	       break;
	      case LINE_EDGE_R:
    	   region_line[line_index].line_vertex_x [1] = line_start_x + line_length1;
	       region_line[line_index].line_vertex_y [1] = line_start_y;
	       region_line[line_index].line_vertex_direction [1] = LINE_EDGE_L;
	       break;
	      case LINE_EDGE_L:
    	   region_line[line_index].line_vertex_x [1] = line_start_x - line_length1;
	       region_line[line_index].line_vertex_y [1] = line_start_y;
	       region_line[line_index].line_vertex_direction [1] = LINE_EDGE_L;
	       break;
	      case LINE_EDGE_UL:
    	   region_line[line_index].line_vertex_x [1] = line_start_x - line_length1 * X_MULT;
	       region_line[line_index].line_vertex_y [1] = line_start_y - line_length1 * Y_MULT;
	       region_line[line_index].line_vertex_direction [1] = LINE_EDGE_L;
	       break;
	      case LINE_EDGE_DL:
    	   region_line[line_index].line_vertex_x [1] = line_start_x - line_length1 * X_MULT;
	       region_line[line_index].line_vertex_y [1] = line_start_y + line_length1 * Y_MULT;
	       region_line[line_index].line_vertex_direction [1] = LINE_EDGE_L;
	       break;
	     }
      float separation;
      switch(join_dir)
      {
      	case LINE_EDGE_L:
							 separation = line_end_y - region_line[line_index].line_vertex_y [1];
        separation = fabs(separation);
    	   region_line[line_index].line_vertex_x [2] = line_end_x + separation * X_MULT_2;// * story_inter.zoom;
	       region_line[line_index].line_vertex_y [2] = region_line[line_index].line_vertex_y [1];
	       region_line[line_index].line_vertex_direction [2] = LINE_EDGE_R;
	       break;
      	case LINE_EDGE_R:
							 separation = line_end_y - region_line[line_index].line_vertex_y [1];
        separation = fabs(separation);
    	   region_line[line_index].line_vertex_x [2] = line_end_x - separation * X_MULT_2;// * story_inter.zoom;
	       region_line[line_index].line_vertex_y [2] = region_line[line_index].line_vertex_y [1];
	       region_line[line_index].line_vertex_direction [2] = LINE_EDGE_R;
	       break;
      	case LINE_EDGE_UR:
							 separation = line_end_y - region_line[line_index].line_vertex_y [1];
        separation = fabs(separation);
    	   region_line[line_index].line_vertex_x [2] = line_end_x + separation * X_MULT_2;// * story_inter.zoom;
	       region_line[line_index].line_vertex_y [2] = line_end_y - separation * Y_MULT_2;
	       region_line[line_index].line_vertex_direction [2] = LINE_EDGE_UR;
	       break;
      	case LINE_EDGE_DR:
							 separation = line_end_y - region_line[line_index].line_vertex_y [1];
        separation = fabs(separation);
    	   region_line[line_index].line_vertex_x [2] = line_end_x + separation * X_MULT_2;// * story_inter.zoom;
	       region_line[line_index].line_vertex_y [2] = line_end_y + separation * Y_MULT_2;
	       region_line[line_index].line_vertex_direction [2] = LINE_EDGE_UR;
	       break;
      	case LINE_EDGE_UL:
							 separation = line_end_y - region_line[line_index].line_vertex_y [1];
        separation = fabs(separation);
    	   region_line[line_index].line_vertex_x [2] = line_end_x - separation * X_MULT_2;// * story_inter.zoom;
	       region_line[line_index].line_vertex_y [2] = line_end_y - separation * Y_MULT_2;
	       region_line[line_index].line_vertex_direction [2] = LINE_EDGE_UR;
	       break;
      	case LINE_EDGE_DL:
							 separation = line_end_y - region_line[line_index].line_vertex_y [1];
        separation = fabs(separation);
    	   region_line[line_index].line_vertex_x [2] = line_end_x - separation * X_MULT_2;// * story_inter.zoom;
	       region_line[line_index].line_vertex_y [2] = line_end_y + separation * Y_MULT_2;
	       region_line[line_index].line_vertex_direction [2] = LINE_EDGE_UR;
	       break;

      }

    	 region_line[line_index].line_vertex_x [3] = line_end_x;
	     region_line[line_index].line_vertex_y [3] = line_end_y;
	     region_line[line_index].line_vertex_direction [3] = end_dir;

			 	 finish_region_line(line_index);


}

*/
/*
static void add_region_line_vertex(int line_index, int vertex_index, float x, float y)
{

	region_line[line_index].line_vertex_x [region_line[line_index].line_vertices] = x;
	region_line[line_index].line_vertex_y [region_line[line_index].line_vertices] = y;

	region_line[line_index].line_vertices ++;

}
*/

static void finish_region_line(int line_index)
{
	int i;

	region_line[line_index].line_vertices = 0;

	for (i = 0; i < LINE_VERTICES; i ++)
	{
		if (region_line[line_index].line_vertex_x [i] > 0) // < 0 should indicate empty vertex
			region_line[line_index].line_vertices ++;
	}

}


void region_line_3(int region_index, float along_edge,
														int s1_dir, float s1_length,
														int s2_dir, float s2_length,
														int s3_dir, float s3_length);
void region_line_4(int region_index, float along_edge,
														int s1_dir, float s1_length,
														int s2_dir, float s2_length,
														int s3_dir, float s3_length,
														int s4_dir, float s4_length);


void draw_region_lines(void)
{





	region_line_3(0, 13,
															LINE_EDGE_UR, 18,
															LINE_EDGE_R, 30,
															LINE_EDGE_DR, 20);

 region_line_3(0, 4,
															LINE_EDGE_R, 14,
															LINE_EDGE_DR, 20,
															LINE_EDGE_R, 14);
	region_line_4(0, 17,
															LINE_EDGE_R, 4,
															LINE_EDGE_DR, 36,
															LINE_EDGE_R, 8,
															LINE_EDGE_UR, 12);

	region_line_3(1, 8,
															LINE_EDGE_UR, 14,
															LINE_EDGE_U, 12,
															LINE_EDGE_UR, 20);
	region_line_4(1, 5,
															LINE_EDGE_R, 14,
															LINE_EDGE_UR, 26,
															LINE_EDGE_U, 24,
															LINE_EDGE_UL, 16);


	region_line_4(2, 8,
															LINE_EDGE_UL, 12,
															LINE_EDGE_U, 10,
															LINE_EDGE_UR, 22,
															LINE_EDGE_R, 30);
	region_line_4(2, 18,
															LINE_EDGE_UL, 8,
															LINE_EDGE_U, 23,
															LINE_EDGE_UR, 12,
															LINE_EDGE_R, 30);
	region_line_4(2, 8,
															LINE_EDGE_R, 5,
															LINE_EDGE_UR, 5,
															LINE_EDGE_U, 16,
															LINE_EDGE_UR, 20);

// link to green 1
	region_line_3(3, 18,
															LINE_EDGE_UL, 20,
															LINE_EDGE_U, 54,
															LINE_EDGE_UR, 16);

	region_line_4(3, 7,
															LINE_EDGE_UL, 15,
															LINE_EDGE_U, 20,
															LINE_EDGE_UR, 16,
															LINE_EDGE_R, 40);
	region_line_3(3, 16,
															LINE_EDGE_UR, 12,
															LINE_EDGE_U, 12,
															LINE_EDGE_UR, 26);

// to yellow 1
	region_line_3(3, 12,
															LINE_EDGE_R, 20,
															LINE_EDGE_UR, 79,
															LINE_EDGE_UR, 6);

// BLUE capital
//  to yellow:
	region_line_3(4, 17,
															LINE_EDGE_DR, 20,
															LINE_EDGE_R, 28,
															LINE_EDGE_UR, 26);
	region_line_4(4, 5,
															LINE_EDGE_R, 6,
															LINE_EDGE_UR, 26,
															LINE_EDGE_R, 7,
															LINE_EDGE_DR, 20);
	region_line_3(4, 14,
															LINE_EDGE_R, 6,
															LINE_EDGE_UR, 15,
															LINE_EDGE_R, 26);

return;


	int i, j;

	for (i = 0; i < REGION_LINES; i ++)
	{
		if (region_line[i].line_vertices > 0)
		{
			for (j = 0; j < region_line[i].line_vertices - 1; j ++)
			{
				int dir1;
				if (j == 0)
					dir1 = region_line[i].line_vertex_direction [j];
				  else
							dir1 = -1;
				story_add_line(region_line[i].line_vertex_x [j], region_line[i].line_vertex_y [j],
													      region_line[i].line_vertex_x [j + 1], region_line[i].line_vertex_y [j + 1],
//													      region_line[i].line_vertex_direction [j], region_line[i].line_vertex_direction [j+1],
													      dir1, region_line[i].line_vertex_direction [j + 1],
													      colours.base_trans [COL_BLUE] [SHADE_MAX] [TRANS_FAINT]);
			}
		}

	}



}

void region_line_3(int region_index, float along_edge,
														int s1_dir, float s1_length,
														int s2_dir, float s2_length,
														int s3_dir, float s3_length)
{



region_line_segment [0].direction = s1_dir;
region_line_segment [0].length = s1_length;

region_line_segment [1].direction = s2_dir;
region_line_segment [1].length = s2_length;

region_line_segment [2].direction = s3_dir;
region_line_segment [2].length = s3_length;

region_line_segment [3].direction = s3_dir;

 set_region_line(region_index, along_edge, 3, 2 * story_inter.zoom);

}


void region_line_4(int region_index, float along_edge,
														int s1_dir, float s1_length,
														int s2_dir, float s2_length,
														int s3_dir, float s3_length,
														int s4_dir, float s4_length)
{



region_line_segment [0].direction = s1_dir;
region_line_segment [0].length = s1_length;

region_line_segment [1].direction = s2_dir;
region_line_segment [1].length = s2_length;

region_line_segment [2].direction = s3_dir;
region_line_segment [2].length = s3_length;

region_line_segment [3].direction = s4_dir;
region_line_segment [3].length = s4_length;

region_line_segment [4].direction = s4_dir;


 set_region_line(region_index, along_edge, 4, 2 * story_inter.zoom);

}

#define X_SLOPE 0.82
#define Y_SLOPE 0.5

static void set_region_line(int starting_region, float along_edge, int segments, float line_width)
{

	int i;

	float region1_x, region1_y;

	region1_x = story_inter.region_inter[starting_region].x_screen;
	region1_y = story_inter.region_inter[starting_region].y_screen;

 float hex_vertex_x = 0, hex_vertex_y = 0;
 float hex_size = STORY_REGION_SIZE * story_inter.zoom;

 float line_start_x, line_start_y;

 along_edge *= story_inter.zoom;


	float left_vertex_x, left_vertex_y, right_vertex_x, right_vertex_y;
	float old_left_vertex_x, old_left_vertex_y, old_right_vertex_x, old_right_vertex_y;


	switch(region_line_segment[0].direction)
	{
		default:
		case LINE_EDGE_R:
			hex_vertex_x = region1_x + hex_size * X_MULT;
			hex_vertex_y = region1_y - hex_size * Y_MULT;
			line_start_x = hex_vertex_x;
			line_start_y = hex_vertex_y + along_edge;
			old_left_vertex_x = line_start_x;
			old_left_vertex_y = line_start_y - line_width;
			old_right_vertex_x = line_start_x;
			old_right_vertex_y = line_start_y + line_width;
			break;
		case LINE_EDGE_L:
			hex_vertex_x = region1_x - hex_size * X_MULT;
			hex_vertex_y = region1_y + hex_size * Y_MULT;
			line_start_x = hex_vertex_x;
			line_start_y = hex_vertex_y - along_edge;
			old_left_vertex_x = line_start_x;
			old_left_vertex_y = line_start_y + line_width;
			old_right_vertex_x = line_start_x;
			old_right_vertex_y = line_start_y - line_width;
			break;
		case LINE_EDGE_UR:
			hex_vertex_x = region1_x;
			hex_vertex_y = region1_y - hex_size;
			line_start_x = hex_vertex_x + along_edge * X_MULT;
			line_start_y = hex_vertex_y + along_edge * Y_MULT;
			old_left_vertex_x = line_start_x - line_width * X_MULT;
			old_left_vertex_y = line_start_y - line_width * Y_MULT;
			old_right_vertex_x = line_start_x + line_width * X_MULT;;
			old_right_vertex_y = line_start_y + line_width * Y_MULT;;
			break;
		case LINE_EDGE_DR:
			hex_vertex_x = region1_x + hex_size * X_MULT;
			hex_vertex_y = region1_y + hex_size * Y_MULT;
			line_start_x = hex_vertex_x - along_edge * X_MULT;
			line_start_y = hex_vertex_y + along_edge * Y_MULT;
			old_left_vertex_x = line_start_x + line_width * X_MULT;
			old_left_vertex_y = line_start_y - line_width * Y_MULT;
			old_right_vertex_x = line_start_x - line_width * X_MULT;;
			old_right_vertex_y = line_start_y + line_width * Y_MULT;;
			break;
  case LINE_EDGE_UL:
			hex_vertex_x = region1_x - hex_size * X_MULT;
			hex_vertex_y = region1_y - hex_size * Y_MULT;
			line_start_x = hex_vertex_x + along_edge * X_MULT;
			line_start_y = hex_vertex_y - along_edge * Y_MULT;
			old_left_vertex_x = line_start_x - line_width * X_MULT;
			old_left_vertex_y = line_start_y + line_width * Y_MULT;
			old_right_vertex_x = line_start_x + line_width * X_MULT;;
			old_right_vertex_y = line_start_y - line_width * Y_MULT;;
			break;
  case LINE_EDGE_DL:
			hex_vertex_x = region1_x;
			hex_vertex_y = region1_y + hex_size;
			line_start_x = hex_vertex_x - along_edge * X_MULT;
			line_start_y = hex_vertex_y - along_edge * Y_MULT;
			old_left_vertex_x = line_start_x + line_width * X_MULT;
			old_left_vertex_y = line_start_y + line_width * Y_MULT;
			old_right_vertex_x = line_start_x - line_width * X_MULT;;
			old_right_vertex_y = line_start_y - line_width * Y_MULT;;
			break;

	}

	float line_x;// = line_start_x;
	float line_y;// = line_start_y;

	for (i = 0; i < segments; i++)
	{
/*
		switch(region_line_segment[i].direction)
		{
		 default:
		 case LINE_EDGE_R:
		 	line_x = line_start_x + region_line_segment[i].length * story_inter.zoom;
		 	line_y = line_start_y;
			 left_vertex_x = line_x;
			 left_vertex_y = line_y - line_width;
			 right_vertex_x = line_x;
			 right_vertex_y = line_y + line_width;
		 	break;
		 case LINE_EDGE_L:
		 	line_x = line_start_x - region_line_segment[i].length * story_inter.zoom;
		 	line_y = line_start_y;
			 left_vertex_x = line_x;
			 left_vertex_y = line_y + line_width;
			 right_vertex_x = line_x;
			 right_vertex_y = line_y - line_width;
		 	break;
#define DIAG_LINE_ADJUST 1
		 case LINE_EDGE_UR:
		 	line_x = line_start_x + region_line_segment[i].length * X_SLOPE * story_inter.zoom;
		 	line_y = line_start_y - region_line_segment[i].length * Y_SLOPE * story_inter.zoom;
			 left_vertex_x = line_x - line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 left_vertex_y = line_y - line_width * Y_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_x = line_x + line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_y = line_y + line_width * Y_SLOPE * DIAG_LINE_ADJUST;
		 	break;
		 case LINE_EDGE_DR:
		 	line_x = line_start_x + region_line_segment[i].length * X_SLOPE * story_inter.zoom;
		 	line_y = line_start_y + region_line_segment[i].length * Y_SLOPE * story_inter.zoom;
			 left_vertex_x = line_x + line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 left_vertex_y = line_y - line_width * Y_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_x = line_x - line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_y = line_y + line_width * Y_SLOPE * DIAG_LINE_ADJUST;
		 	break;
		 case LINE_EDGE_UL:
		 	line_x = line_start_x - region_line_segment[i].length * X_SLOPE * story_inter.zoom;
		 	line_y = line_start_y - region_line_segment[i].length * Y_SLOPE * story_inter.zoom;
			 left_vertex_x = line_x - line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 left_vertex_y = line_y + line_width * Y_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_x = line_x + line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_y = line_y - line_width * Y_SLOPE * DIAG_LINE_ADJUST;
		 	break;
		 case LINE_EDGE_DL:
		 	line_x = line_start_x - region_line_segment[i].length * X_SLOPE * story_inter.zoom;
		 	line_y = line_start_y + region_line_segment[i].length * Y_SLOPE * story_inter.zoom;
			 left_vertex_x = line_x + line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 left_vertex_y = line_y + line_width * Y_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_x = line_x - line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_y = line_y - line_width * Y_SLOPE * DIAG_LINE_ADJUST;
		 	break;

		}
*/

		switch(region_line_segment[i].direction)
		{
		 default:
		 case LINE_EDGE_R:
		 	line_x = line_start_x + region_line_segment[i].length * story_inter.zoom;
		 	line_y = line_start_y;
		 	break;
		 case LINE_EDGE_L:
		 	line_x = line_start_x - region_line_segment[i].length * story_inter.zoom;
		 	line_y = line_start_y;
		 	break;
		 case LINE_EDGE_U:
		 	line_x = line_start_x;
		 	line_y = line_start_y - region_line_segment[i].length * story_inter.zoom;
		 	break;
		 case LINE_EDGE_D:
		 	line_x = line_start_x;
		 	line_y = line_start_y + region_line_segment[i].length * story_inter.zoom;
		 	break;
#define DIAG_LINE_ADJUST 1
		 case LINE_EDGE_UR:
		 	line_x = line_start_x + region_line_segment[i].length * X_SLOPE * story_inter.zoom;
		 	line_y = line_start_y - region_line_segment[i].length * Y_SLOPE * story_inter.zoom;
		 	break;
		 case LINE_EDGE_DR:
		 	line_x = line_start_x + region_line_segment[i].length * X_SLOPE * story_inter.zoom;
		 	line_y = line_start_y + region_line_segment[i].length * Y_SLOPE * story_inter.zoom;
		 	break;
		 case LINE_EDGE_UL:
		 	line_x = line_start_x - region_line_segment[i].length * X_SLOPE * story_inter.zoom;
		 	line_y = line_start_y - region_line_segment[i].length * Y_SLOPE * story_inter.zoom;
		 	break;
		 case LINE_EDGE_DL:
		 	line_x = line_start_x - region_line_segment[i].length * X_SLOPE * story_inter.zoom;
		 	line_y = line_start_y + region_line_segment[i].length * Y_SLOPE * story_inter.zoom;
		 	break;

		}

		switch(region_line_segment[i + 1].direction)
		{
		 default:
		 case LINE_EDGE_R:
			 left_vertex_x = line_x;
			 left_vertex_y = line_y - line_width;
			 right_vertex_x = line_x;
			 right_vertex_y = line_y + line_width;
		 	break;
		 case LINE_EDGE_L:
			 left_vertex_x = line_x;
			 left_vertex_y = line_y + line_width;
			 right_vertex_x = line_x;
			 right_vertex_y = line_y - line_width;
		 	break;
		 case LINE_EDGE_U:
			 left_vertex_x = line_x - line_width;
			 left_vertex_y = line_y;
			 right_vertex_x = line_x + line_width;
			 right_vertex_y = line_y;
		 	break;
		 case LINE_EDGE_D:
			 left_vertex_x = line_x + line_width;
			 left_vertex_y = line_y;
			 right_vertex_x = line_x - line_width;
			 right_vertex_y = line_y;
		 	break;
#define DIAG_LINE_ADJUST 1
		 case LINE_EDGE_UR:
			 left_vertex_x = line_x - line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 left_vertex_y = line_y - line_width * Y_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_x = line_x + line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_y = line_y + line_width * Y_SLOPE * DIAG_LINE_ADJUST;
		 	break;
		 case LINE_EDGE_DR:
			 left_vertex_x = line_x + line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 left_vertex_y = line_y - line_width * Y_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_x = line_x - line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_y = line_y + line_width * Y_SLOPE * DIAG_LINE_ADJUST;
		 	break;
		 case LINE_EDGE_UL:
			 left_vertex_x = line_x - line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 left_vertex_y = line_y + line_width * Y_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_x = line_x + line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_y = line_y - line_width * Y_SLOPE * DIAG_LINE_ADJUST;
		 	break;
		 case LINE_EDGE_DL:
			 left_vertex_x = line_x + line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 left_vertex_y = line_y + line_width * Y_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_x = line_x - line_width * X_SLOPE * DIAG_LINE_ADJUST;
			 right_vertex_y = line_y - line_width * Y_SLOPE * DIAG_LINE_ADJUST;
		 	break;

		}


//fpr("\n RL %i %f %f,%f %f,%f %f,%f", i, region_line_segment[i].length, line_x, line_y, left_vertex_x, left_vertex_y, right_vertex_x, right_vertex_y);
//fpr("\n RL %i [%f,%f %f,%f] to [%f,%f %f,%f]", i, old_left_vertex_x, old_left_vertex_y, old_right_vertex_x, old_right_vertex_y, left_vertex_x, left_vertex_y, right_vertex_x, right_vertex_y);

//add_line(4, line_start_x, line_start_y, line_x, line_y, colours.base [COL_GREEN] [SHADE_HIGH]);
//add_line(4, left_vertex_x, left_vertex_y, old_left_vertex_x, old_left_vertex_y, colours.base [COL_YELLOW] [SHADE_HIGH]);

//fpr("\n R %i %f,%f to %f,%f", i, old_left_vertex_x, old_left_vertex_y, left_vertex_x, left_vertex_y);
//fpr("\n LV %f,%f to %f,%f", line_start_x, line_start_y, line_x, line_y);

   add_region_line_triangle(old_left_vertex_x,
																												old_left_vertex_y,
																												old_right_vertex_x,
																												old_right_vertex_y,
																												left_vertex_x,
																												left_vertex_y,
//																												colours.base [COL_BLUE] [SHADE_HIGH]);
																												colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_FAINT]);
   add_region_line_triangle(left_vertex_x,
																												left_vertex_y,
																												old_right_vertex_x,
																												old_right_vertex_y,
																												right_vertex_x,
																												right_vertex_y,
//																												colours.base [COL_BLUE] [SHADE_HIGH]);
																												colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_FAINT]);


  line_start_x = line_x;
  line_start_y = line_y;
  old_left_vertex_x = left_vertex_x;
  old_left_vertex_y = left_vertex_y;
  old_right_vertex_x = right_vertex_x;
  old_right_vertex_y = right_vertex_y;

	}



}


void add_region_line_triangle(float v1x, float v1y, float v2x, float v2y, float v3x, float v3y, ALLEGRO_COLOR col)
{

#define REGION_LINE_LAYER 0

//fpr("\n ARLT %f,%f %f,%f %f,%f", v1x, v1y, v2x, v2y, v3x, v3y);

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = v1x;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = v1y;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [REGION_LINE_LAYER] [vbuf.index_pos_triangle [REGION_LINE_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = v2x;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = v2y;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [REGION_LINE_LAYER] [vbuf.index_pos_triangle [REGION_LINE_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = v3x;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = v3y;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [REGION_LINE_LAYER] [vbuf.index_pos_triangle [REGION_LINE_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

}


float   last_x1;
float   last_y1;
float   last_x2;
float   last_y2;


static void story_add_line(float x, float y, float xa, float ya, int vdir1, int vdir2, ALLEGRO_COLOR col)
{

 float v1_left_x, v1_left_y;
 float v1_right_x, v1_right_y;
 float v2_left_x, v2_left_y;
 float v2_right_x, v2_right_y;

 switch(vdir1)
 {
default: // should never happen but prevents compiler warnings
	 case -1: // use previous 2 vertices
		 v1_left_x = last_x1;
		 v1_left_y = last_y1;
		 v1_right_x = last_x2;
		 v1_right_y = last_y2;
		 break;
	 case LINE_EDGE_L:
		 v1_left_x = x;
		 v1_left_y = y - 1 * story_inter.zoom;
		 v1_right_x = x;
		 v1_right_y = y + 1 * story_inter.zoom;
		 break;
	 case LINE_EDGE_R:
		 v1_right_x = x;
		 v1_right_y = y - 1 * story_inter.zoom;
		 v1_left_x = x;
		 v1_left_y = y + 1 * story_inter.zoom;
		 break;
	 case LINE_EDGE_DL:
		 v1_right_x = x - X_MULT * story_inter.zoom;
		 v1_right_y = y - Y_MULT * story_inter.zoom;
		 v1_left_x = x + X_MULT * story_inter.zoom;;
		 v1_left_y = y + Y_MULT * story_inter.zoom;
		 break;
	 case LINE_EDGE_UR:
		 v1_left_x = x - X_MULT * story_inter.zoom;
		 v1_left_y = y - Y_MULT * story_inter.zoom;
		 v1_right_x = x + X_MULT * story_inter.zoom;;
		 v1_right_y = y + Y_MULT * story_inter.zoom;
		 break;
	 case LINE_EDGE_DR:
		 v1_right_x = x - X_MULT * story_inter.zoom;
		 v1_right_y = y + Y_MULT * story_inter.zoom;
		 v1_left_x = x + X_MULT * story_inter.zoom;;
		 v1_left_y = y - Y_MULT * story_inter.zoom;
		 break;
	 case LINE_EDGE_UL:
		 v1_left_x = x - X_MULT * story_inter.zoom;
		 v1_left_y = y + Y_MULT * story_inter.zoom;
		 v1_right_x = x + X_MULT * story_inter.zoom;;
		 v1_right_y = y - Y_MULT * story_inter.zoom;
		 break;
		case LINE_EDGE_U:
		 v1_left_x = x - 1 * story_inter.zoom;
		 v1_left_y = y;
		 v1_right_x = x + 1 * story_inter.zoom;
		 v1_right_y = y;
		 break;
		case LINE_EDGE_D:
		 v1_left_x = x + 1 * story_inter.zoom;
		 v1_left_y = y;
		 v1_right_x = x - 1 * story_inter.zoom;
		 v1_right_y = y;
		 break;

 }

x = xa;
y = ya;

 switch(vdir2)
 {
default: // should never happen but prevents compiler warnings
	 case LINE_EDGE_L:
		 v2_left_x = x;
		 v2_left_y = y - 1 * story_inter.zoom;
		 v2_right_x = x;
		 v2_right_y = y + 1 * story_inter.zoom;
		 break;
	 case LINE_EDGE_R:
		 v2_right_x = x;
		 v2_right_y = y - 1 * story_inter.zoom;
		 v2_left_x = x;
		 v2_left_y = y + 1 * story_inter.zoom;
		 break;
	 case LINE_EDGE_DL:
		 v2_right_x = x - X_MULT * story_inter.zoom;
		 v2_right_y = y - Y_MULT * story_inter.zoom;
		 v2_left_x = x + X_MULT * story_inter.zoom;;
		 v2_left_y = y + Y_MULT * story_inter.zoom;
		 break;
	 case LINE_EDGE_UR:
		 v2_left_x = x - X_MULT * story_inter.zoom;
		 v2_left_y = y - Y_MULT * story_inter.zoom;
		 v2_right_x = x + X_MULT * story_inter.zoom;;
		 v2_right_y = y + Y_MULT * story_inter.zoom;
		 break;
	 case LINE_EDGE_DR:
		 v2_right_x = x - X_MULT * story_inter.zoom;
		 v2_right_y = y + Y_MULT * story_inter.zoom;
		 v2_left_x = x + X_MULT * story_inter.zoom;;
		 v2_left_y = y - Y_MULT * story_inter.zoom;
		 break;
	 case LINE_EDGE_UL:
		 v2_left_x = x - X_MULT * story_inter.zoom;
		 v2_left_y = y + Y_MULT * story_inter.zoom;
		 v2_right_x = x + X_MULT * story_inter.zoom;;
		 v2_right_y = y - Y_MULT * story_inter.zoom;
		 break;
		case LINE_EDGE_U:
		 v2_left_x = x - 1 * story_inter.zoom;
		 v2_left_y = y;
		 v2_right_x = x + 1 * story_inter.zoom;
		 v2_right_y = y;
		 break;
		case LINE_EDGE_D:
		 v2_left_x = x + 1 * story_inter.zoom;
		 v2_left_y = y;
		 v2_right_x = x - 1 * story_inter.zoom;
		 v2_right_y = y;
		 break;

 }


	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = v1_left_x;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = v1_left_y;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = v1_right_x;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = v1_right_y;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = v2_right_x;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = v2_right_y;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;


	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = v1_left_x;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = v1_left_y;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = v2_right_x;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = v2_right_y;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = v2_left_x;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = v2_left_y;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [DESIGN_QUAD_LAYER] [vbuf.index_pos_triangle [DESIGN_QUAD_LAYER]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

 last_x1 = v2_right_x;
 last_y1 = v2_right_y;
 last_x2 = v2_left_x;
 last_y2 = v2_left_y;


/*
 int layer = 2;


	vbuf.buffer_line[vbuf.vertex_pos_line].x = x;
	vbuf.buffer_line[vbuf.vertex_pos_line].y = y;
	vbuf.buffer_line[vbuf.vertex_pos_line].color = col;
	vbuf.index_line [layer] [vbuf.index_pos_line [layer]] = vbuf.vertex_pos_line;
	vbuf.vertex_pos_line++;
	vbuf.index_pos_line[layer]++;


	vbuf.buffer_line[vbuf.vertex_pos_line].x = xa;
	vbuf.buffer_line[vbuf.vertex_pos_line].y = ya;
	vbuf.buffer_line[vbuf.vertex_pos_line].color = col;
	vbuf.index_line [layer] [vbuf.index_pos_line [layer]] = vbuf.vertex_pos_line;
	vbuf.vertex_pos_line++;
	vbuf.index_pos_line[layer]++;
*/

}


/*

Will need the following map vision masks:

base - underlying map, with visibility colour and features.
 - blitted onto display at start of map drawing
 - doesn't change

mask_opaque - like base but with opaque colour
 - doesn't change

mask_trans - partly transparent
 - doesn't change

visible


- each frame, the base map is drawn on the display
- the mask map is drawn on visible
 - opaque is used if enemy not revealed; otherwise, trans is used
- scanner blips are drawn on the display
- holes are drawn on visible for visibility
- visible is drawn on the display over the base map.

*/

