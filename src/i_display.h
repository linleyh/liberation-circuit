
#ifndef H_I_DISPLAY
#define H_I_DISPLAY

#include "i_header.h"

void init_drand(void);

void run_display(void);
void clear_vbuf(void);
void draw_mouse_cursor(void);
void set_mouse_cursor(int mc_index);
void make_mouse_cursor(ALLEGRO_BITMAP* mc_bmp, int mcursor);

ALLEGRO_COLOR map_rgb(int r, int g, int b); // bounds-checked wrapper for al_map_rgb
ALLEGRO_COLOR map_rgba(int r, int g, int b, int a); // bounds-checked wrapper for al_map_rgba

void add_line(int layer, float x, float y, float xa, float ya, ALLEGRO_COLOR col);
void add_line_vertex(float x, float y, ALLEGRO_COLOR col);
void construct_line(int layer, int v1, int v2);
void add_tri_vertex(float x, float y, ALLEGRO_COLOR col);
void construct_triangle(int layer, int v1, int v2, int v3);

void check_vbuf(void);
void draw_vbuf(void);

void add_proc_shape(float x, float y, al_fixed angle, int shape, int size, ALLEGRO_COLOR* proc_col, float zoom);
void draw_proc_shape(float x, float y, al_fixed angle, int shape, int player_index, float zoom, ALLEGRO_COLOR* proc_col);

void draw_link_shape(float child_x, float child_y,
																					al_fixed child_angle,
																					int child_shape,
																					int child_link_index,
																					float parent_x, float parent_y,
																					al_fixed parent_angle,
																					int parent_shape,
																					int parent_link_index,
																					ALLEGRO_COLOR proc_col,
//																					ALLEGRO_COLOR edge_col,
																					float zoom);
/*
void draw_object_base_shape(float proc_x, float proc_y,
																					al_fixed proc_angle,
																					int proc_shape,
																					int proc_link_index,
																					ALLEGRO_COLOR* proc_col,
//																					ALLEGRO_COLOR fill_col,
//																					ALLEGRO_COLOR edge_col,
																					float zoom);*/

void draw_object(float proc_x, float proc_y,
																					al_fixed proc_angle,
																					int proc_shape,
																					struct object_struct* obj,
																					struct object_instance_struct* obj_inst, // NULL if drawn on design menu
																					struct core_struct* core, // NULL if drawn on design menu
																					struct proc_struct* proc, // NULL if drawn on design menu
																					int proc_link_index,
																					ALLEGRO_COLOR* proc_col,
//																					ALLEGRO_COLOR fill_col,
//																					ALLEGRO_COLOR fill_col_under,
//																					ALLEGRO_COLOR edge_col,
																					float zoom);

void draw_proc_outline(float x, float y, al_fixed angle, int shape, float scale, int lines_only, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col, float zoom);

#endif
