

#include "z_poly.h"

/*

This file contains the polygon editor. It generates code that will, when copied into g_shapes.h, generate the component designs.
It's not at all user-friendly, isn't programmed defensively and doesn't need to be compiled in release versions of the game.

Its name starts with z so it stays at the end of the list of files.

*/

// to remove Z_POLY code, also remove definition in z_poly.h
#ifdef Z_POLY
// z_poly is my own process geometry editor.
// It's not supported for general use, as using to design new components requires substantial changes to the code.


#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>


#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"

#include "g_misc.h"

#include "c_header.h"
#include "e_slider.h"

#include "i_header.h"
#include "i_input.h"
#include "i_view.h"
#include "i_display.h"
#include "m_input.h"
#include "g_shapes.h"
#include "m_maths.h"


ALLEGRO_DISPLAY* display;

extern ALLEGRO_EVENT_QUEUE* event_queue;
extern struct fontstruct font [FONTS];
extern struct dshape_struct dshape [NSHAPES];
extern struct vbuf_struct vbuf;


/*
// This struct contains display properties of shapes
struct dshape_struct
{

	// vertices: used only for display (so floats are okay)
 int polys;
 int poly_layer [DSHAPE_POLYS]; // which display layer this poly is on (non-overlapping polys can be on the same layer)
 int poly_colour_level [DSHAPE_POLYS]; // intensity of colour for this poly
 int display_vertices [DSHAPE_POLYS];
 int display_triangles [DSHAPE_POLYS];

 float display_vertex_angle [DSHAPE_POLYS] [DSHAPE_DISPLAY_VERTICES]; // used for display
 float display_vertex_dist [DSHAPE_POLYS] [DSHAPE_DISPLAY_VERTICES]; // used for display

	int display_triangle_index [DSHAPE_POLYS] [DSHAPE_DISPLAY_VERTICES] [3]; // not sure about the DSHAPE_DISPLAY_VERTICES dimension
	int poly_fill_source [DSHAPE_POLYS] [2]; // x/y coordinates of the source used for floodfill for the dshape's collision mask. Any point inside the polygon will do.
// poly_fill_source is a gameplay property rather than a display property so it kind of belongs in nshape_struct. But nshape ignores polygons entirely. Doesn't really matter anyway.

//	int link_poly [MAX_LINKS]; // which poly the link's display parameters are based on. -1 for links that are not based on a particular poly. - no longer used
//	int link_display_vertex [MAX_LINKS]; // which vertex of that poly the link's display parameters are based on - no longer used
	float link_object_angle [MAX_LINKS];
	float link_object_dist [MAX_LINKS];
	float link_point_angle [MAX_LINKS] [4];
	float link_point_dist [MAX_LINKS] [4];

	float link_outer_angle [MAX_LINKS]; // outer is the angle/dist from the inner link point ([1]) to the outer link point ([3])
	float link_outer_dist [MAX_LINKS];

// these are the vectors from the inner link point (link_point [1]) to the left and right side points
	float link_point_side_angle [MAX_LINKS] [2]; // this is an offset from the proc angle, not the link/object angle!
	float link_point_side_dist [MAX_LINKS] [2];

	int links; // how many links

	int outline_vertices;
	al_fixed outline_vertex_angle_fixed [OUTLINE_VERTICES];
	al_fixed outline_vertex_dist_fixed [OUTLINE_VERTICES];
	float outline_vertex_angle [OUTLINE_VERTICES];
	float outline_vertex_dist [OUTLINE_VERTICES];
	int outline_base_vertex; // this is an index in the outline vertex arrays for a vertex to use as a base for a fan (fans can be used for outlines because outlines are generally simpler than polys)

};
*/


enum
{
ZLINK_BASE,
ZLINK_LEFT,
ZLINK_FAR,
ZLINK_RIGHT,
ZLINK_POINT,
ZLINK_OBJECT,

ZLINK_DATA
};

// this struct contains the information for a zshape.
// it is filled in by shape definition functions in g_shape.c
// and by z_poly functions.
struct zshape_struct
{
 int recording;

 int polys;

// when adding anything to this list of poly data, also add to copy_zshape_poly()
 int poly_layer [DSHAPE_POLYS]; // which display layer this poly is on (non-overlapping polys can be on the same layer)
 int poly_colour [DSHAPE_POLYS];
 int vertices [DSHAPE_POLYS];
 int vertex_x [DSHAPE_POLYS] [DSHAPE_DISPLAY_VERTICES];
 int vertex_y [DSHAPE_POLYS] [DSHAPE_DISPLAY_VERTICES];
 int vertex_collision [DSHAPE_POLYS] [DSHAPE_DISPLAY_VERTICES]; // 1 if this is to be a collision vertex
 int fill_source_x [DSHAPE_POLYS];
 int fill_source_y [DSHAPE_POLYS];

// int display_triangles [DSHAPE_POLYS]; needed? this should be derived in the shape creation functions in g_shape.c

// possibly do this by automatically extending poly 0
 int outline_vertices;
 int outline_vertex_x [OUTLINE_VERTICES];
 int outline_vertex_y [OUTLINE_VERTICES];
	int outline_base_vertex; // needed?

 int links;
 int link_x [MAX_LINKS] [ZLINK_DATA];
 int link_y [MAX_LINKS] [ZLINK_DATA];
/* int link_left_x [MAX_LINKS];
 int link_left_y [MAX_LINKS];
 int link_right_x [MAX_LINKS];
 int link_right_y [MAX_LINKS];
 int link_far_x [MAX_LINKS];
 int link_far_y [MAX_LINKS];
 int link_point_x [MAX_LINKS];
 int link_point_y [MAX_LINKS];
 int link_object_x [MAX_LINKS];
 int link_object_y [MAX_LINKS];*/

// ideally mirror axes should be here as well, but that would probably be a pain to implement. They're easy to add by hand in g_shape.c anyway.

// init state:
 int current_poly;

};

struct zshape_struct zshape;

enum
{
ZSELECT_NONE,
ZSELECT_POLY,
ZSELECT_POLY_VERTEX,
ZSELECT_DRAG_VERTEX,
ZSELECT_LINK,
ZSELECT_LINK_VERTEX,
ZSELECT_DRAG_LINK_VERTEX,

};

struct zstate_struct
{

	int select_type;

	int selected_poly;
	int selected_vertex;

	int selected_link;
	int selected_link_vertex;


};

struct zstate_struct zstate;

static void zpoly_display(void);
static void zpoly_input(void);
static void draw_poly(float x, float y, int poly, ALLEGRO_COLOR fill_col, float zoom, int draw_numbers_if_selected);
static void add_zrect(float xa, float ya, float xb, float yb, ALLEGRO_COLOR zcol);
static void add_ztri(int layer, float x1, float y1, float x2, float y2, float x3, float y3, ALLEGRO_COLOR fill_col);
static void zpoly_write_to_file(void);
static void insert_new_poly_after(int after_poly);
static void copy_zshape_poly(int target, int source);
static void mirror_zshape_poly_around_x(int mpoly);
static void mirror_zshape_poly_around_y(int mpoly);
static void mirror_zshape_link_around_x(int mlink);
static void mirror_zshape_link_around_y(int mlink);
static void delete_zshape_poly(int del_poly);
static void copy_zshape_vertex(int zpoly, int target, int source);
static void delete_zshape_vertex(int del_poly, int del_vertex);
static void draw_link(float x, float y, int link, ALLEGRO_COLOR fill_col, float zoom, int draw_numbers_if_selected);
static void insert_new_vertex_after(int zpoly, int after_vertex);
static void delete_zshape_link(int del_link);
static void insert_new_link_after(int after_link);
static void copy_zshape_link(int target, int source);
static void draw_zcross(float x, float y, float cross_size, ALLEGRO_COLOR col);
static void get_zcolour_string(char* str, int col);

void run_zpoly(void)
{

 ALLEGRO_EVENT ev;

 al_set_target_bitmap(al_get_backbuffer(display));
 al_set_clipping_rectangle(0, 0, settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);


 al_show_mouse_cursor(display);

// zpoly_write_to_file();

 zstate.select_type = ZSELECT_NONE;

 while(TRUE)
 {

  zpoly_input();

  zpoly_display();

  al_wait_for_event(event_queue, &ev);

 };


}




static void zpoly_display(void)
{

  al_set_target_bitmap(al_get_backbuffer(display));
  al_clear_to_color(colours.world_background);



//  float x = 800, y = 400;
//  al_fixed angle = 0;

#define MAIN_ZSHAPE_ZOOM 7
#define MAIN_ZSHAPE_X 800
#define MAIN_ZSHAPE_Y 400

// float f_angle;
 float zoom = MAIN_ZSHAPE_ZOOM;
 char colour_string [30];


 int poly, i;
 ALLEGRO_COLOR draw_col;

// f_angle = fixed_to_radians(angle);

#define POLY_SQUARE_SIZE 60

			 add_line(0,
													MAIN_ZSHAPE_X - 400,
													MAIN_ZSHAPE_Y,
													MAIN_ZSHAPE_X + 400,
													MAIN_ZSHAPE_Y,
													colours.base [COL_BLUE] [SHADE_MED]);
			 add_line(0,
													MAIN_ZSHAPE_X,
													MAIN_ZSHAPE_Y - 400,
													MAIN_ZSHAPE_X,
													MAIN_ZSHAPE_Y + 400,
													colours.base [COL_BLUE] [SHADE_MED]);

					if (zstate.select_type == ZSELECT_POLY
      || zstate.select_type == ZSELECT_POLY_VERTEX
					 || zstate.select_type == ZSELECT_DRAG_VERTEX)
				 {
				 	for (i = 0; i < zshape.vertices [zstate.selected_poly]; i ++)
						{
 			   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], 1200, 20 + i * 14, ALLEGRO_ALIGN_RIGHT, "%i.", i);
 			   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], 1230, 20 + i * 14, ALLEGRO_ALIGN_RIGHT, "%i,", zshape.vertex_x [zstate.selected_poly] [i]);
 			   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], 1250, 20 + i * 14, ALLEGRO_ALIGN_RIGHT, "%i", zshape.vertex_y [zstate.selected_poly] [i]);
						}
				 }

					if (zstate.select_type == ZSELECT_LINK
      || zstate.select_type == ZSELECT_LINK_VERTEX
					 || zstate.select_type == ZSELECT_DRAG_LINK_VERTEX)
				 {
				 	for (i = 0; i < ZLINK_DATA; i ++)
						{
 			   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], 1200, 20 + i * 14, ALLEGRO_ALIGN_RIGHT, "%i.", i);
 			   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], 1230, 20 + i * 14, ALLEGRO_ALIGN_RIGHT, "%i,", zshape.link_x [zstate.selected_link] [i]);
 			   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], 1250, 20 + i * 14, ALLEGRO_ALIGN_RIGHT, "%i", zshape.link_y [zstate.selected_link] [i]);
						}
				 }

 for (poly = 0; poly < zshape.polys; poly++)
	{
  draw_poly(MAIN_ZSHAPE_X, MAIN_ZSHAPE_Y, poly, colours.proc_col [0] [PROC_DAMAGE_COLS-1] [0] [zshape.poly_colour [poly]], zoom, 1);

  draw_col = colours.base_trans [COL_BLUE] [SHADE_LOW] [TRANS_FAINT];

  //if (zstate.select_type != ZSELECT_NONE
//			&& zstate.select_type != ZSELECT_POLY
  if ((zstate.select_type == ZSELECT_NONE
				|| zstate.select_type == ZSELECT_POLY
				|| zstate.select_type == ZSELECT_POLY_VERTEX
				|| zstate.select_type == ZSELECT_DRAG_VERTEX)
			&& zstate.selected_poly == poly)
		{
				draw_col = colours.base_trans [COL_BLUE] [SHADE_MED] [TRANS_FAINT];
		}

  add_zrect(10, 10 + poly * (POLY_SQUARE_SIZE+15), 10 + POLY_SQUARE_SIZE, 10 + poly * (POLY_SQUARE_SIZE+15) + POLY_SQUARE_SIZE, draw_col);
  get_zcolour_string(colour_string, zshape.poly_colour [poly]);
  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], 10, 10 + poly * (POLY_SQUARE_SIZE+15) + POLY_SQUARE_SIZE + 1, ALLEGRO_ALIGN_LEFT, "%s", colour_string);


  draw_poly(10 + (POLY_SQUARE_SIZE/2), 10 + poly * (POLY_SQUARE_SIZE+15) + (POLY_SQUARE_SIZE/2), poly, colours.proc_col [0] [PROC_DAMAGE_COLS-1] [0] [zshape.poly_colour [poly]], 0.5, 0);

  draw_poly(1300, 800, poly, colours.proc_col [0] [PROC_DAMAGE_COLS-1] [0] [zshape.poly_colour [poly]], 1, 0);

	}

	for (i = 0; i < zshape.links; i ++)
	{
		draw_link(MAIN_ZSHAPE_X, MAIN_ZSHAPE_Y, i, colours.proc_col [0] [PROC_DAMAGE_COLS-1] [0] [PROC_COL_OBJECT_1], zoom, 1);

  if ((zstate.select_type == ZSELECT_NONE
				|| zstate.select_type == ZSELECT_LINK
				|| zstate.select_type == ZSELECT_LINK_VERTEX
				|| zstate.select_type == ZSELECT_DRAG_LINK_VERTEX)
			&& zstate.selected_link == i)
		{
				draw_col = colours.base_trans [COL_BLUE] [SHADE_MED] [TRANS_FAINT];
		}

  add_zrect(80, 10 + i * (POLY_SQUARE_SIZE+15), 80 + POLY_SQUARE_SIZE, 10 + i * (POLY_SQUARE_SIZE+15) + POLY_SQUARE_SIZE, draw_col);

		draw_link(80 + (POLY_SQUARE_SIZE/2), 10 + i * (POLY_SQUARE_SIZE+15) + (POLY_SQUARE_SIZE/2), i, colours.proc_col [0] [PROC_DAMAGE_COLS-1] [0] [PROC_COL_OBJECT_1], 0.5, 0);

	}

  draw_vbuf();


  al_flip_display();

}

static void add_zrect(float xa, float ya, float xb, float yb, ALLEGRO_COLOR zcol)
{

	add_ztri(1, xa, ya, xb, ya, xb, yb, zcol);
	add_ztri(1, xa, ya, xa, yb, xb, yb, zcol);

}


static void add_ztri(int layer, float x1, float y1, float x2, float y2, float x3, float y3, ALLEGRO_COLOR fill_col)
{

				vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = x1;
   	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = y1;
    vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = fill_col;
    vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
    vbuf.vertex_pos_triangle++;

				vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = x2;
   	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = y2;
    vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = fill_col;
    vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
    vbuf.vertex_pos_triangle++;

				vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = x3;
   	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = y3;
    vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = fill_col;
    vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
    vbuf.vertex_pos_triangle++;
}


static void draw_poly(float x, float y, int poly, ALLEGRO_COLOR fill_col, float zoom, int draw_numbers_if_selected)
{

	 int i;
	 int v;

			for (i = 0; i < zshape.vertices [poly]; i ++)
			{
				v = i + 1;
				if (v >= zshape.vertices [poly])
					v = 0;
			 add_line(0,
													x + (zshape.vertex_x [poly] [i] * zoom),
													y + (zshape.vertex_y [poly] [i] * zoom),
													x + (zshape.vertex_x [poly] [v] * zoom),
													y + (zshape.vertex_y [poly] [v] * zoom),
													fill_col);

			if (draw_numbers_if_selected)
			{

 			  if (zshape.vertex_collision [poly] [i])
							add_zrect(x + (zshape.vertex_x [poly] [i] * zoom) - 4,
																	y + (zshape.vertex_y [poly] [i] * zoom) - 4,
                 x + (zshape.vertex_x [poly] [i] * zoom) + 4,
																	y + (zshape.vertex_y [poly] [i] * zoom) + 4,
																	colours.base_trans [COL_AQUA] [SHADE_MAX] [TRANS_MED]);
//																								colours.base [COL_AQUA] [SHADE_MAX]);

				if (zstate.selected_poly == poly)
				{
					if (zstate.select_type == ZSELECT_POLY)
				 {
 			  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], x + (zshape.vertex_x [poly] [i] * zoom), y + (zshape.vertex_y [poly] [i] * zoom), ALLEGRO_ALIGN_CENTRE, "%i", i);
//																	colours.base_trans [COL_AQUA] [SHADE_MED] [TRANS_MED]);

//					&& zstate.selected_vertex == i
 				}
 				if (zstate.select_type == ZSELECT_POLY_VERTEX
					 || zstate.select_type == ZSELECT_DRAG_VERTEX)
					{
						if (i == zstate.selected_vertex)
 			   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_YELLOW] [SHADE_HIGH], x + (zshape.vertex_x [poly] [i] * zoom), y + (zshape.vertex_y [poly] [i] * zoom), ALLEGRO_ALIGN_CENTRE, "[%i]", i);
 			    else
 			     al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], x + (zshape.vertex_x [poly] [i] * zoom), y + (zshape.vertex_y [poly] [i] * zoom), ALLEGRO_ALIGN_CENTRE, "%i", i);
					}
				}
			}
//				fpr("[v%i:%f,%f-%f,%f]", i);
			}
/*
				if (zstate.selected_poly == poly)
				{
					if (zstate.select_type == ZSELECT_POLY
						||	zstate.select_type == ZSELECT_POLY_VERTEX
					 || zstate.select_type == ZSELECT_DRAG_VERTEX
					 || zstate.select_type == ZSELECT_DRAG_FILL_POINT)
														)
				 {
 			  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], x + (zshape.vertex_x [poly] [i] * zoom), y + (zshape.vertex_y [poly] [i] * zoom), ALLEGRO_ALIGN_CENTRE, "%i", i);
 				}
 				if (zstate.select_type == ZSELECT_POLY_VERTEX
					 || zstate.select_type == ZSELECT_DRAG_VERTEX)
*/

}



static void draw_link(float x, float y, int link, ALLEGRO_COLOR fill_col, float zoom, int draw_numbers_if_selected)
{

	 int i;
	 int v;



			for (i = ZLINK_BASE; i < ZLINK_RIGHT+1; i ++)
			{
				v = i + 1;
				if (v > ZLINK_RIGHT)
					v = ZLINK_BASE;
			 add_line(0,
													x + (zshape.link_x [link] [i] * zoom),
													y + (zshape.link_y [link] [i] * zoom),
													x + (zshape.link_x [link] [v] * zoom),
													y + (zshape.link_y [link] [v] * zoom),
													fill_col);
			 if (zstate.select_type == ZSELECT_LINK)
				{
			  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], x + (zshape.link_x [link] [i] * zoom), y + (zshape.link_y [link] [i] * zoom), ALLEGRO_ALIGN_CENTRE, "%i", i);
				}

				if ((zstate.select_type == ZSELECT_LINK_VERTEX
					 || zstate.select_type == ZSELECT_DRAG_LINK_VERTEX)
					&& zstate.selected_link == link)
				{
					if (i == zstate.selected_link_vertex)
 			  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_YELLOW] [SHADE_HIGH], x + (zshape.link_x [link] [i] * zoom), y + (zshape.link_y [link] [i] * zoom), ALLEGRO_ALIGN_CENTRE, "[%i]", i);
 			   else
			     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], x + (zshape.link_x [link] [i] * zoom), y + (zshape.link_y [link] [i] * zoom), ALLEGRO_ALIGN_CENTRE, "%i", i);
				}
/*				if (zstate.selected_link == poly)
				{
					if (zstate.select_type == ZSELECT_POLY)
				 {
 			  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], x + (zshape.vertex_x [poly] [i] * zoom), y + (zshape.vertex_y [poly] [i] * zoom), ALLEGRO_ALIGN_CENTRE, "%i", i);
//					&& zstate.selected_vertex == i
 				}
 				if (zstate.select_type == ZSELECT_POLY_VERTEX
					 || zstate.select_type == ZSELECT_DRAG_VERTEX)
					{
						if (i == zstate.selected_vertex)
 			   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_YELLOW] [SHADE_HIGH], x + (zshape.vertex_x [poly] [i] * zoom), y + (zshape.vertex_y [poly] [i] * zoom), ALLEGRO_ALIGN_CENTRE, "[%i]", i);
 			    else
 			     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], x + (zshape.vertex_x [poly] [i] * zoom), y + (zshape.vertex_y [poly] [i] * zoom), ALLEGRO_ALIGN_CENTRE, "%i", i);
					}
				}*/
//				fpr("[v%i:%f,%f-%f,%f]", i);
			}

			if (zstate.select_type == ZSELECT_LINK
     || zstate.select_type == ZSELECT_LINK_VERTEX
					|| zstate.select_type == ZSELECT_DRAG_LINK_VERTEX)
			{
				int selector_size = 3;
				if (zstate.selected_link == link
					&&	zstate.selected_link_vertex == ZLINK_POINT
					&& (zstate.select_type == ZSELECT_LINK_VERTEX
 					|| zstate.select_type == ZSELECT_DRAG_LINK_VERTEX))
							selector_size = 5;

				draw_zcross(x + (zshape.link_x [link] [ZLINK_POINT] * zoom),
													   y + (zshape.link_y [link] [ZLINK_POINT] * zoom),
																selector_size,
   													colours.base [COL_GREEN] [SHADE_HIGH]);

				selector_size = 3;
				if (zstate.selected_link == link
					&&	zstate.selected_link_vertex == ZLINK_OBJECT
					&& (zstate.select_type == ZSELECT_LINK_VERTEX
 					|| zstate.select_type == ZSELECT_DRAG_LINK_VERTEX))
							selector_size = 5;

				draw_zcross(x + (zshape.link_x [link] [ZLINK_OBJECT] * zoom),
													   y + (zshape.link_y [link] [ZLINK_OBJECT] * zoom),
																selector_size,
   													colours.base [COL_RED] [SHADE_HIGH]);

			}

}

static void draw_zcross(float x, float y, float cross_size, ALLEGRO_COLOR col)
{

							 add_line(0,
													x - cross_size,
													y,
													x + cross_size,
													y,
													col);
							 add_line(0,
													x,
													y - cross_size,
													x,
													y + cross_size,
													col);


}

/*
void add_line(int layer, float x, float y, float xa, float ya, ALLEGRO_COLOR col);


	 int i, j;
	 float f_angle = 0;
	 float vertex_list_x [DSHAPE_DISPLAY_VERTICES + 1]; // extra entry at the end is for 0
  float vertex_list_y [DSHAPE_DISPLAY_VERTICES + 1];

		int layer = zshape.poly_layer [poly];
//		fill_col = proc_col [dsh->poly_colour_level [poly]];


  for (i = 0; i < dsh->display_vertices [poly]; i ++)
		{
			vertex_list_x [i] = x + fxpart(f_angle + dsh->display_vertex_angle [poly] [i], dsh->display_vertex_dist [poly] [i]) * zoom;
			vertex_list_y [i] = y + fypart(f_angle + dsh->display_vertex_angle [poly] [i], dsh->display_vertex_dist [poly] [i]) * zoom;
		}

		vertex_list_x [dsh->display_vertices [poly]] = vertex_list_x [0];
		vertex_list_y [dsh->display_vertices [poly]] = vertex_list_y [0];

  for (i = 0; i < dsh->display_triangles [poly]; i ++)
		{
   for (j = 0; j < 3; j ++)
		 {

				vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = vertex_list_x [dsh->display_triangle_index [poly] [i] [j]];
   	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = vertex_list_y [dsh->display_triangle_index [poly] [i] [j]];
    vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = fill_col;
    vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
    vbuf.vertex_pos_triangle++;
		 }
		}
*/




static void zpoly_input(void)
{

	get_ex_control(0, 0);

	int i, j;
	int mouse_x = ex_control.mouse_x_pixels;
	int mouse_y = ex_control.mouse_y_pixels;
//	int mouse_lbutton_pressed = (ex_control.mb_press [0] == BUTTON_JUST_PRESSED);

	if (ex_control.mb_press [0] == BUTTON_HELD)
	{
			if (zstate.select_type == ZSELECT_DRAG_VERTEX)
			{
				zshape.vertex_x [zstate.selected_poly] [zstate.selected_vertex] = (mouse_x - MAIN_ZSHAPE_X) / MAIN_ZSHAPE_ZOOM;
				zshape.vertex_y [zstate.selected_poly] [zstate.selected_vertex] = (mouse_y - MAIN_ZSHAPE_Y) / MAIN_ZSHAPE_ZOOM;
			}
			if (zstate.select_type == ZSELECT_DRAG_LINK_VERTEX)
			{
				zshape.link_x [zstate.selected_link] [zstate.selected_link_vertex] = (mouse_x - MAIN_ZSHAPE_X) / MAIN_ZSHAPE_ZOOM;
				zshape.link_y [zstate.selected_link] [zstate.selected_link_vertex] = (mouse_y - MAIN_ZSHAPE_Y) / MAIN_ZSHAPE_ZOOM;
			}
	}
	 else
		{
			if (zstate.select_type == ZSELECT_DRAG_VERTEX)
				zstate.select_type = ZSELECT_POLY_VERTEX;
		}



	if (ex_control.mb_press [0] == BUTTON_JUST_PRESSED)
	{
	 if (mouse_x >= 10 && mouse_x <= 10 + POLY_SQUARE_SIZE)
	 {
 		int poly_box = (mouse_y - 10) / (POLY_SQUARE_SIZE + 15);

		 if (poly_box >= 0
 			&& poly_box < zshape.polys)
 			{
				 zstate.selected_poly = poly_box;
				 zstate.select_type = ZSELECT_POLY;
 			}
	 }
	  else
			{
				 int finished = 0;
//				 int poly_already_selected = 0;
				 int found_poly = -1;
				 int found_vertex = -1;
	    for (i = 0; i < zshape.polys; i ++)
					{
	     for (j = 0; j < zshape.vertices [i]; j ++)
					 {
					 	if (abs(mouse_x - MAIN_ZSHAPE_X - (zshape.vertex_x [i] [j] * MAIN_ZSHAPE_ZOOM))
					 		 + abs(mouse_y - MAIN_ZSHAPE_Y - (zshape.vertex_y [i] [j] * MAIN_ZSHAPE_ZOOM))
					 				< 10)
					 	{
					 		found_poly = i;
					 		found_vertex = j;
					 		if (i == zstate.selected_poly
									&& (zstate.select_type == ZSELECT_POLY
										|| zstate.select_type == ZSELECT_POLY_VERTEX
										|| zstate.select_type == ZSELECT_DRAG_VERTEX))
								{
//									poly_already_selected = 1; // can drag vertex now
									finished = 1;
								}
  			 	 break;
					 	}
					 }
					 if (finished)
							break;
				}
				if (found_poly != -1)
				{
	 	  zstate.selected_poly = found_poly;
	 	  zstate.selected_vertex = found_vertex;
//	 	  if (poly_already_selected)
	 	  if (ex_control.special_key_press [SPECIAL_KEY_SHIFT] > 0)
//						|| ex_control.key_press [ALLEGRO_KEY_RSHIFT] > 0)
	 	   zstate.select_type = ZSELECT_DRAG_VERTEX;
	 	    else
	 	     zstate.select_type = ZSELECT_POLY_VERTEX;
	 	  goto finished_left_mouse_button;
				}


				 int found_link = -1;
				 int found_link_vertex = -1;

	    for (i = 0; i < zshape.links; i ++)
					{
	     for (j = 0; j < ZLINK_DATA; j ++)
					 {
					 	if (abs(mouse_x - MAIN_ZSHAPE_X - (zshape.link_x [i] [j] * MAIN_ZSHAPE_ZOOM))
					 		 + abs(mouse_y - MAIN_ZSHAPE_Y - (zshape.link_y [i] [j] * MAIN_ZSHAPE_ZOOM))
					 				< 10)
		 				{
					 		found_link = i;
					 		found_link_vertex = j;
					 		if (i == zstate.selected_link
									&& (zstate.select_type == ZSELECT_LINK
										|| zstate.select_type == ZSELECT_LINK_VERTEX
										|| zstate.select_type == ZSELECT_DRAG_LINK_VERTEX))
									finished = 1;
  			 	 break;
		 				}
					 }
					 if (finished)
							break;
				 }
				if (found_link != -1)
				{
	 	  zstate.selected_link = found_link;
	 	  zstate.selected_link_vertex = found_link_vertex;
//	 	  if (poly_already_selected)
	 	  if (ex_control.special_key_press [SPECIAL_KEY_SHIFT] > 0)
	 	   zstate.select_type = ZSELECT_DRAG_LINK_VERTEX;
	 	    else
	 	     zstate.select_type = ZSELECT_LINK_VERTEX;
	 	  goto finished_left_mouse_button;
				}



/*
    if (zstate.select_type == ZSELECT_POLY
				 || zstate.select_type == ZSELECT_POLY_VERTEX)
				{
	    for (i = 0; i < zshape.vertices [zstate.selected_poly]; i ++)
					{
						if (abs(mouse_x - MAIN_ZSHAPE_X - (zshape.vertex_x [zstate.selected_poly] [i] * MAIN_ZSHAPE_ZOOM))
							 + abs(mouse_y - MAIN_ZSHAPE_Y - (zshape.vertex_y [zstate.selected_poly] [i] * MAIN_ZSHAPE_ZOOM))
									< 10)
						{
						 zstate.selected_vertex = i;
  				 zstate.select_type = ZSELECT_DRAG_VERTEX;
  				 break;
						}
					}
				}
*/
			}
	} // end code for lmb just pressed

finished_left_mouse_button:

/*

Controls:

N - new poly inserted after the current one. starts off as just a triangle or something
X - following poly (probably one just added by N) is turned into a copy of the current poly mirrored on the vertical axis

V - new vertex inserted after the current one
?+ - rotate vertices clockwise
?- - anticlockwise
C - symmetry for poly that overlaps centreline
    vertex 0 should be first vertex, and others should follow clockwise
     * may not bother to implement this and just do it manually

cursor keys - move whole poly pixel by pixel

delete - deletes current poly or vertex depending on mode

*/

 if (ex_control.unichar_input == 'n')
 {
		if (zstate.select_type == ZSELECT_POLY
			|| zstate.select_type == ZSELECT_POLY_VERTEX)
		 insert_new_poly_after(zstate.selected_poly);
				else
  		 insert_new_poly_after(zshape.polys - 1);
 }

// if (ex_control.key_press [ALLEGRO_KEY_L] == BUTTON_JUST_PRESSED)
 if (ex_control.unichar_input == 'l')
 {
		if (zstate.select_type == ZSELECT_LINK
			|| zstate.select_type == ZSELECT_LINK_VERTEX)
		 insert_new_link_after(zstate.selected_link);
				else
  		 insert_new_link_after(zshape.links - 1);
 }


// if (ex_control.key_press [ALLEGRO_KEY_V] == BUTTON_JUST_PRESSED)
 if (ex_control.unichar_input == 'v')
 {
		if (zstate.select_type == ZSELECT_POLY_VERTEX)
		 insert_new_vertex_after(zstate.selected_poly, zstate.selected_vertex);
 }

// if (ex_control.key_press [ALLEGRO_KEY_C] == BUTTON_JUST_PRESSED)
 if (ex_control.unichar_input == 'c')
 {
		if (zstate.select_type == ZSELECT_POLY_VERTEX)
			zshape.vertex_collision [zstate.selected_poly] [zstate.selected_vertex] ^= 1;
 }

 if (ex_control.unichar_input >= 48
		&& ex_control.unichar_input <= 57)
	{
 		if (zstate.select_type == ZSELECT_POLY
	 		|| zstate.select_type == ZSELECT_POLY_VERTEX)
	 		zshape.poly_colour [zstate.selected_poly] = ex_control.unichar_input - 48;

	}

 if (ex_control.special_key_press [SPECIAL_KEY_DELETE] == BUTTON_JUST_PRESSED)
	{
		if (zstate.select_type == ZSELECT_POLY)
		{
			delete_zshape_poly(zstate.selected_poly);
			zstate.select_type = ZSELECT_NONE;
		}
		if (zstate.select_type == ZSELECT_POLY_VERTEX)
		{
			delete_zshape_vertex(zstate.selected_poly, zstate.selected_vertex);
			zstate.selected_vertex = 0; // probably unnecessary
			zstate.select_type = ZSELECT_POLY;
		}
		if (zstate.select_type == ZSELECT_LINK
			|| zstate.select_type == ZSELECT_LINK_VERTEX)
		{
			delete_zshape_link(zstate.selected_link);
			zstate.select_type = ZSELECT_NONE;
		}
	}

	if (ex_control.unichar_input == 'x')
	{
		if (zstate.select_type == ZSELECT_POLY
			|| zstate.select_type == ZSELECT_POLY_VERTEX)
		{
		 mirror_zshape_poly_around_y(zstate.selected_poly);
		}
		if (zstate.select_type == ZSELECT_LINK
			|| zstate.select_type == ZSELECT_LINK_VERTEX)
		{
		 mirror_zshape_link_around_y(zstate.selected_link);
		}
	}

//	if (ex_control.key_press [ALLEGRO_KEY_S] == BUTTON_JUST_PRESSED)
	if (ex_control.unichar_input == 's')
	{
		if (zstate.select_type == ZSELECT_POLY
			|| zstate.select_type == ZSELECT_POLY_VERTEX)
		{
		 mirror_zshape_poly_around_x(zstate.selected_poly);
		}
		if (zstate.select_type == ZSELECT_LINK
			|| zstate.select_type == ZSELECT_LINK_VERTEX)
		{
		 mirror_zshape_link_around_x(zstate.selected_link);
		}
	}

//	if (ex_control.key_press [ALLEGRO_KEY_W] == BUTTON_JUST_PRESSED)
	if (ex_control.unichar_input == 'w')
	{
		zpoly_write_to_file();
	}

//	if (ex_control.key_press [ALLEGRO_KEY_ESCAPE])
//		safe_exit(0);

}

// mirrors mpoly + 1 around the y axis
static void mirror_zshape_poly_around_y(int mpoly)
{
	int other_poly = mpoly + 1;

	if (other_poly > zshape.polys - 1)
		return;

 copy_zshape_poly(other_poly, mpoly);

 int i;

 for (i = 0; i < zshape.vertices [other_poly]; i ++)
	{
		zshape.vertex_y [other_poly] [i] *= -1;
	}

	zshape.fill_source_y [other_poly] *= -1;

}

// mirrors mpoly + 2 around the y axis
static void mirror_zshape_poly_around_x(int mpoly)
{
	int other_poly = mpoly + 2;

	if (other_poly > zshape.polys - 1)
		return;

 copy_zshape_poly(other_poly, mpoly);

 int i;

 for (i = 0; i < zshape.vertices [other_poly]; i ++)
	{
		zshape.vertex_x [other_poly] [i] *= -1;
	}

	zshape.fill_source_x [other_poly] *= -1;

}

// mirrors mlink + 1 around the y axis
static void mirror_zshape_link_around_y(int mlink)
{
	int other_link = mlink + 1;

	if (other_link > zshape.links - 1)
		return;

 copy_zshape_link(other_link, mlink);

// need to swap left and right vertices:
 zshape.link_x [other_link] [ZLINK_LEFT] = zshape.link_x [mlink] [ZLINK_RIGHT];
 zshape.link_y [other_link] [ZLINK_LEFT] = zshape.link_y [mlink] [ZLINK_RIGHT];
 zshape.link_x [other_link] [ZLINK_RIGHT] = zshape.link_x [mlink] [ZLINK_LEFT];
 zshape.link_y [other_link] [ZLINK_RIGHT] = zshape.link_y [mlink] [ZLINK_LEFT];

 int i;

 for (i = 0; i < ZLINK_DATA; i ++)
	{
		zshape.link_y [other_link] [i] *= -1;
	}
}

// mirrors mlink + 2 around the x axis
static void mirror_zshape_link_around_x(int mlink)
{
	int other_link = mlink + 2;

	if (other_link > zshape.links - 1)
		return;

 copy_zshape_link(other_link, mlink);

 int i;

 for (i = 0; i < ZLINK_DATA; i ++)
	{
		zshape.link_x [other_link] [i] *= -1;
	}
}



static void insert_new_poly_after(int after_poly)
{

	if (after_poly >= DSHAPE_POLYS - 2
		|| zshape.polys >= DSHAPE_POLYS - 1)
	{
		fpr("\ninsert_new_poly_after failed - too many polys.");
		return; // failed
	}

	int i;
	int new_poly_index = after_poly + 1;

// I'm not 100% sure this works properly if there are almost the max number of polys.
	for (i = DSHAPE_POLYS - 2; i >= new_poly_index; i --)
	{
		copy_zshape_poly(i+1, i);
	}

	zshape.polys++;

	zshape.poly_layer [new_poly_index] = 1; // probably doesn't really matter
	zshape.poly_colour [new_poly_index] = 2; // need a way to set this
	zshape.vertices [new_poly_index] = 3;

	zshape.vertex_x [new_poly_index] [0] = 50;
	zshape.vertex_y [new_poly_index] [0] = 0;
	zshape.vertex_x [new_poly_index] [1] = 43;
	zshape.vertex_y [new_poly_index] [1] = 10;
	zshape.vertex_x [new_poly_index] [2] = 57;
	zshape.vertex_y [new_poly_index] [2] = 10;


}

static void insert_new_vertex_after(int zpoly, int after_vertex)
{

	if (after_vertex >= DSHAPE_DISPLAY_VERTICES - 2
		|| zshape.vertices [zpoly] >= DSHAPE_DISPLAY_VERTICES - 1)
	{
		fpr("\ninsert_new_vertex_after failed - too many vertices.");
		return; // failed
	}

 int i;
 int new_vertex_index = after_vertex + 1;

 for (i = DSHAPE_DISPLAY_VERTICES - 2; i >= new_vertex_index; i --)
	{
		copy_zshape_vertex(zpoly, i+1, i);
	}

	zshape.vertices [zpoly] ++;

	zshape.vertex_collision [zpoly] [new_vertex_index] = 0;
	zshape.vertex_x [zpoly] [new_vertex_index] = zshape.vertex_x [zpoly] [after_vertex] + 10;
	zshape.vertex_y [zpoly] [new_vertex_index] = zshape.vertex_y [zpoly] [after_vertex];


}



static void insert_new_link_after(int after_link)
{

	if (after_link >= MAX_LINKS - 2
		|| zshape.links >= MAX_LINKS - 1)
	{
		fpr("\ninsert_new_link_after failed - too many links.");
		return; // failed
	}

 int i;
 int new_link_index = after_link + 1;

 for (i = MAX_LINKS - 2; i >= new_link_index; i --)
	{
		copy_zshape_link(i+1, i);
	}

	zshape.links ++;

#define NEW_LINK_X 50
#define NEW_LINK_Y 0

 zshape.link_x [new_link_index] [ZLINK_BASE] = NEW_LINK_X;
 zshape.link_y [new_link_index] [ZLINK_BASE] = NEW_LINK_Y;
 zshape.link_x [new_link_index] [ZLINK_LEFT] = NEW_LINK_X - 5;
 zshape.link_y [new_link_index] [ZLINK_LEFT] = NEW_LINK_Y - 5;
 zshape.link_x [new_link_index] [ZLINK_FAR] = NEW_LINK_X;
 zshape.link_y [new_link_index] [ZLINK_FAR] = NEW_LINK_Y - 10;
 zshape.link_x [new_link_index] [ZLINK_RIGHT] = NEW_LINK_X + 5;
 zshape.link_y [new_link_index] [ZLINK_RIGHT] = NEW_LINK_Y - 5;
 zshape.link_x [new_link_index] [ZLINK_POINT] = NEW_LINK_X;
 zshape.link_y [new_link_index] [ZLINK_POINT] = NEW_LINK_Y - 12;
 zshape.link_x [new_link_index] [ZLINK_OBJECT] = NEW_LINK_X;
 zshape.link_y [new_link_index] [ZLINK_OBJECT] = NEW_LINK_Y - 15;


}



static void delete_zshape_poly(int del_poly)
{

 int i;

	for (i = del_poly; i < DSHAPE_POLYS - 1; i ++)
	{
		copy_zshape_poly(i, i+1);
	}

	zshape.polys--;

}

static void delete_zshape_vertex(int del_poly, int del_vertex)
{

 if (zshape.vertices [del_poly] <= 3)
	{
		fpr("\nToo few vertices on poly.");
		return;
	}

 int i;

	for (i = del_vertex; i < DSHAPE_DISPLAY_VERTICES - 1; i ++)
	{
		copy_zshape_vertex(del_poly, i, i+1);
	}

	zshape.vertices [del_poly]--;

}

static void delete_zshape_link(int del_link)
{

 int i;

	for (i = del_link; i < MAX_LINKS - 1; i ++)
	{
		copy_zshape_link(i, i+1);
	}

	zshape.links--;


}

static void copy_zshape_poly(int target, int source)
{

 int i;

	zshape.poly_layer [target] = zshape.poly_layer [source];
	zshape.poly_colour [target] = zshape.poly_colour [source];
	zshape.vertices [target] = zshape.vertices [source];

	for (i = 0; i < zshape.vertices [source]; i ++)
	{
		zshape.vertex_x [target] [i] = zshape.vertex_x [source] [i];
		zshape.vertex_y [target] [i] = zshape.vertex_y [source] [i];
		zshape.vertex_collision [target] [i] = zshape.vertex_collision [source] [i];
	}

	zshape.fill_source_x [target] = zshape.fill_source_x [source];
	zshape.fill_source_y [target] = zshape.fill_source_y [source];

}

// only works within the same poly
static void copy_zshape_vertex(int zpoly, int target, int source)
{

	zshape.vertex_x [zpoly] [target] = zshape.vertex_x [zpoly] [source];
	zshape.vertex_y [zpoly] [target] = zshape.vertex_y [zpoly] [source];
	zshape.vertex_collision [zpoly] [target] = zshape.vertex_collision [zpoly] [source];

}



static void copy_zshape_link(int target, int source)
{

 int i;

 for (i = 0; i < ZLINK_DATA; i ++)
	{
		zshape.link_x [target] [i] = zshape.link_x [source] [i];
		zshape.link_y [target] [i] = zshape.link_y [source] [i];
	}


}


static void zpoly_write_to_file(void)
{

 FILE* file = fopen("zpoly.txt", "wt");

 if (!file)
 {
  fpr("Error: failed to open target file.");
  error_call();
 }

 int i, j;
 char colour_string [30];

// first we guess where the fill points will be by trying to put them in the middle of each poly
//  (if this doesn't work because a poly is concave, need to edit manually)
 for (i = 0; i < zshape.polys; i ++)
	{
		zshape.fill_source_x [i] = 0;
		zshape.fill_source_y [i] = 0;

		for (j = 0; j < zshape.vertices [i]; j++)
		{
		 zshape.fill_source_x [i] += zshape.vertex_x [i] [j];
		 zshape.fill_source_y [i] += zshape.vertex_y [i] [j];
		}
	 zshape.fill_source_x [i] /= zshape.vertices [i];
	 zshape.fill_source_y [i] /= zshape.vertices [i];
	}



fprintf(file, "\n	poly_index = POLY_0;");



 for (i = 0; i < zshape.links; i ++)
	{
  fprintf(file, "\n	add_link_at_xy(%i, // link_index", i);
  fprintf(file, "\n	               %i, %i, // centre", zshape.link_x [i] [ZLINK_BASE], zshape.link_y [i] [ZLINK_BASE]);
  fprintf(file, "\n	               %i, %i, // left", zshape.link_x [i] [ZLINK_LEFT], zshape.link_y [i] [ZLINK_LEFT]);
  fprintf(file, "\n	               %i, %i, // right", zshape.link_x [i] [ZLINK_RIGHT], zshape.link_y [i] [ZLINK_RIGHT]);
  fprintf(file, "\n	               %i, %i, // far", zshape.link_x [i] [ZLINK_FAR], zshape.link_y [i] [ZLINK_FAR]);
  fprintf(file, "\n	               %i, %i, // link", zshape.link_x [i] [ZLINK_POINT], zshape.link_y [i] [ZLINK_POINT]);
  fprintf(file, "\n	               %i, %i); // object\n", zshape.link_x [i] [ZLINK_OBJECT], zshape.link_y [i] [ZLINK_OBJECT]);


	}

 for (i = 0; i < zshape.polys; i ++)
	{
		get_zcolour_string(colour_string, zshape.poly_colour [i]);

  fprintf(file, "\n\n	start_dshape_poly(poly_index++, %i, %s);\n", zshape.poly_layer [i], colour_string);

		for (j = 0; j < zshape.vertices [i]; j ++)
		{
   fprintf(file, "\n	add_vertex(%i, %i, %i);", zshape.vertex_x [i] [j], zshape.vertex_y [i] [j], zshape.vertex_collision [i] [j]);
   if (i == 0)
    fprintf(file, "\n	add_outline_vertex_at_last_poly_vertex(6);"); // should probably give more control over this

		}

  fprintf(file, "\n	add_poly_fill_source(%i, %i);", zshape.fill_source_x [i], zshape.fill_source_y [i]);
  fprintf(file, "\n	fix_display_triangles_fan();");
	}

 fprintf(file, "\n\n	// remember to deal with mirror vertices here!");
 fprintf(file, "\n\n	finish_shape();");

 fclose(file);

 fpr("\nwritten to file.");


}

static void get_zcolour_string(char* str, int col)
{

	switch(col)
	{
	 case PROC_COL_UNDERLAY:
		 strcpy(str, "PROC_COL_UNDERLAY"); break;
	 case PROC_COL_CORE_MUTABLE:
		 strcpy(str, "PROC_COL_CORE_MUTABLE"); break;
	 case PROC_COL_MAIN_1:
		 strcpy(str, "PROC_COL_MAIN_1"); break;
	 case PROC_COL_MAIN_2:
		 strcpy(str, "PROC_COL_MAIN_2"); break;
	 case PROC_COL_MAIN_3:
		 strcpy(str, "PROC_COL_MAIN_3"); break;
	 case PROC_COL_LINK:
		 strcpy(str, "PROC_COL_LINK"); break;
	 case PROC_COL_OBJECT_BASE:
		 strcpy(str, "PROC_COL_OBJECT_BASE"); break;
	 case PROC_COL_OBJECT_1:
		 strcpy(str, "PROC_COL_OBJECT_1"); break;
	 case PROC_COL_OBJECT_2:
		 strcpy(str, "PROC_COL_OBJECT_2"); break;


		default:
			strcpy(str, "unknown colour??"); break;

	};


}

void zshape_init(void)
{
	int i;

	zshape.recording = 0;

	zshape.polys = 0;
	zshape.current_poly = 0;

	for (i = 0; i < DSHAPE_POLYS; i ++)
	{
		zshape.vertices [i] = 0;
	}

	zshape.outline_vertices = 0;

	zshape.links = 0;

}

// call this from g_shape.c when starting the shape to be edited.
// call zshape_end() after finishing the shape being edited.
void zshape_start(void)
{
fpr("\nzshape_start");

	zshape.recording = 1;
}

void zshape_end(void)
{
fpr("\nzshape_end");
	zshape.recording = 0;
}

void zshape_add_poly(int poly, int layer, int colour)
{
 if (!zshape.recording)
		return;

fpr("\nzshape_add_poly(%i,%i,%i)", poly, layer, colour);

#ifdef SANITY_CHECK
 if (poly >= DSHAPE_POLYS - 1)
	{
		fpr("\nToo many polygons in zshape.");
		error_call();
	}
#endif

	zshape.current_poly = poly;
	zshape.poly_layer [poly] = layer;
	zshape.poly_colour [poly] = colour;
	zshape.vertices [poly] = 0;

	zshape.polys ++;


}

#define ZPOINT_ADJUST_X (0)

void zshape_add_vertex(int x, int y, int collision)
{
// could check against DSHAPE_DISPLAY_VERTICES but it's a pretty high limit

 if (!zshape.recording)
		return;

x += ZPOINT_ADJUST_X;

fpr("\nzshape_add_vertex(%i,%i,%i)", x, y, collision);

	zshape.vertex_x [zshape.current_poly] [zshape.vertices [zshape.current_poly]] = x;
	zshape.vertex_y [zshape.current_poly] [zshape.vertices [zshape.current_poly]] = y;

	zshape.vertex_collision [zshape.current_poly] [zshape.vertices [zshape.current_poly]] = collision; // note collision not otherwise initialised

 zshape.vertices [zshape.current_poly] ++;

}

void zshape_add_fill_source(int x, int y)
{
x += ZPOINT_ADJUST_X;
	zshape.fill_source_x [zshape.current_poly] = x;
	zshape.fill_source_y [zshape.current_poly] = y;

}

void zshape_add_link(int x, int y, int left_x, int left_y, int right_x, int right_y, int far_x, int far_y, int link_point_x, int link_point_y, int object_x, int object_y)
{

 if (!zshape.recording)
		return;


	zshape.link_x [zshape.links] [ZLINK_BASE] = x + ZPOINT_ADJUST_X;
	zshape.link_y [zshape.links] [ZLINK_BASE] = y;
	zshape.link_x [zshape.links] [ZLINK_LEFT] = left_x + ZPOINT_ADJUST_X;
	zshape.link_y [zshape.links] [ZLINK_LEFT] = left_y;
	zshape.link_x [zshape.links] [ZLINK_RIGHT] = right_x + ZPOINT_ADJUST_X;
	zshape.link_y [zshape.links] [ZLINK_RIGHT] = right_y;
	zshape.link_x [zshape.links] [ZLINK_FAR] = far_x + ZPOINT_ADJUST_X;
	zshape.link_y [zshape.links] [ZLINK_FAR] = far_y;
	zshape.link_x [zshape.links] [ZLINK_POINT] = link_point_x + ZPOINT_ADJUST_X;
	zshape.link_y [zshape.links] [ZLINK_POINT] = link_point_y;
	zshape.link_x [zshape.links] [ZLINK_OBJECT] = object_x + ZPOINT_ADJUST_X;
	zshape.link_y [zshape.links] [ZLINK_OBJECT] = object_y;

	zshape.links++;

}



#endif
// should be end of file
