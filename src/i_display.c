// al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
#include <allegro5/allegro.h>
//#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "m_config.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "g_header.h"
#include "m_globvars.h"
#include "i_header.h"

#include "m_maths.h"
#include "m_input.h"
#include "g_misc.h"
#include "i_console.h"
#include "i_buttons.h"
#include "e_inter.h"
#include "t_template.h"
#include "i_sysmenu.h"
#include "i_display.h"
#include "i_background.h"
#include "h_mission.h"
#include "h_story.h"

#include "p_panels.h"
#include "p_draw.h"

#include "g_shapes.h"
#include "g_command.h"
#include "g_method.h"
#include "v_interp.h"
#include "v_draw_panel.h"

/*

New plan for display:

use indexed_prim

use simpler, opaque polygons


 - "rotor" move object: have rotating circle made of squares that stretch away from the direction of acceleration.


*/


struct vbuf_struct vbuf;



extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct object_type_struct otype [OBJECT_TYPES];
extern struct mission_state_struct mission_state;
extern struct bcode_panel_state_struct bcp_state;


static void draw_map(void);
void draw_proc_explode_cloud(struct cloud_struct* cl, float x, float y);
void draw_proc_fail_cloud(struct cloud_struct* cl, float x, float y);
//void add_proc_diamond(float x, float y, float float_angle, struct shape_struct* sh, int size, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col);
//void add_proc_shape(float x, float y, al_fixed angle, int shape, int size, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR fill2_col, ALLEGRO_COLOR edge_col, float zoom);
//void add_method_base_diamond(float point_x, float point_y, float f_angle, struct shape_struct* sh, int size, int vertex, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col);
//void add_poly_vertex(float x, float y, ALLEGRO_COLOR col);
//static unsigned int proc_rand(struct proc_struct* pr, int special, int mod);
//static unsigned int packet_rand(struct packet_struct* pack, int mod);
static void print_object_information(float text_x, float text_y, int col, const char* ostring, int value, int print_value);

#define VERTEX_LIST_SIZE 32
float vertex_list [VERTEX_LIST_SIZE] [2];

// call this after setting up the vertex_list array with an appropriate number of vertices
//static void add_outline_poly_layer(int layer, int vertices, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col);
static void add_poly_layer(int layer, int vertices, ALLEGRO_COLOR fill_col);
void add_outline_diamond_layer(int layer, float vx1, float vy1, float vx2, float vy2, float vx3, float vy3, float vx4, float vy4, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col);
static void add_diamond_layer(int layer, float vx1, float vy1, float vx2, float vy2, float vx3, float vy3, float vx4, float vy4, ALLEGRO_COLOR fill_col);
//static void add_outline_triangle_layer(int layer, float vx1, float vy1, float vx2, float vy2, float vx3, float vy3, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col);
//static void add_filled_rectangle(int layer, float x1, float y1, float x2, float y2, ALLEGRO_COLOR fill_col);
void draw_stream_beam(float x1, float by1, float x2, float y2, int col, int status, int counter, int hit);
void draw_slice_beam(float x1, float by1, float x2, float y2, int col, int time_since_firing, int hit);
static void draw_beam_triangles(int layer, float x1, float by1, float x2, float y2, ALLEGRO_COLOR stream_col);
static void draw_fade_slice_triangles(int layer, float x1, float by1, float x2, float y2, ALLEGRO_COLOR stream_col, ALLEGRO_COLOR base_col);
static void draw_beam_bloom_triangles(int layer, float x1, float by1, float x2, float y2, ALLEGRO_COLOR centre_col, ALLEGRO_COLOR edge_col);
//void draw_spike_line(float x1, float by1, float x2, float y2, int col, int counter);
//static unsigned int packet_rand(struct packet_struct* pack, int mod);
static void draw_burst_tail(float x, float y, float x_step, float y_step, float packet_angle, int packet_time, int start_time, int end_time, int col);
static void draw_pulse_tail(float x, float y,
																												float x_step, float y_step,
																												float packet_angle,
																												int packet_time,
																												int start_time,
																												int end_time,
																												int total_time,
																												float tail_width,
																												int col,
																												int shade,
																												int drand_seed,
																												int packet_size,
																												float blob_scale,
																												int pulse_or_burst);
/*static void draw_new_pulse_tail(float x, float y,
																												float x_step, float y_step,
																												float packet_angle,
																												int packet_time,
																												int start_time,
																												int end_time,
																												int total_time,
																												float tail_width,
																												int col,
																												int shade,
																												int drand_seed,
																												int packet_size,
																												float blob_scale,
																												int pulse_or_burst);*/
static void draw_new_pulse_tail(float x, float y,
																												float x_step, float y_step,
																												float packet_angle,
																												int total_length, // maximum length of tail (its length if not cutoff at either end)
																												int front_cutoff, // number of steps past the front (will be 0 if pulse still travelling)
																												int origin_cutoff, // number of steps past the origin (will be 0 if end of tail has moved past origin)
																												float tail_width,
																												int col,
																												int shade,
																												float blob_scale);

static void draw_new_move_tail(float x, float y,
																												float move_angle,
																												float move_power,
																												int col,
																												int shade,
																												int move_time,
																												int drand_seed);

static void draw_fragment_tail(float x, float y,
																												float x_step, float y_step,
																												float packet_angle,
																												int packet_time,
																												int start_time,
																												int end_time,
																												int total_time,
																												float tail_width,
																												int col,
																												int shade,
																												int drand_seed,
																												int packet_size,
																												float blob_scale);
/*static void draw_pulse_tail(float x, float y,
																												float x_step, float y_step,
																												float packet_angle,
																												int packet_time,
																												int start_time,
																												int end_time,
																												int total_time,
																												float tail_width,
																												int col,
																												int shade,
																												int drand_seed);*/
static void draw_notional_group(struct template_struct* draw_templ, al_fixed world_x, al_fixed world_y, al_fixed build_angle, float notional_zoom, int draw_col_index, int draw_col_index_error);
static void draw_spray(float x, float y, float spray_size, int base_bit_size, int player_index, int shade, int time_elapsed, int max_time, int spray_bits, int drand_seed);

static void seed_drand(int seed);
static int drand(int mod, int drand_pos_change);

static void vision_check_for_display(void);
//static void set_outer_edge_vision_block(int block_x, int block_y);

static void special_visible_area(cart notional_core_position);

ALLEGRO_DISPLAY* display;
//ALLEGRO_BITMAP* display_window;



ALLEGRO_BITMAP* vision_mask_map [MAP_MASKS];



ALLEGRO_BITMAP* vision_mask;


//ALLEGRO_BITMAP* packet_bmp;

struct fontstruct font [FONTS];

extern struct game_struct game; // in g_game.c
extern struct dshape_struct dshape [NSHAPES]; // uses same indices as NSHAPES
extern struct nshape_struct nshape [NSHAPES];
extern struct command_struct command;


static void draw_command_marker(int core_index);
static void start_radial(float x, float y, int layer, ALLEGRO_COLOR fill_col);
static void add_radial_vertex(float r_angle, float r_dist);
//static void add_radial_vertex_xy(float x_offset, float y_offset);
//static void add_radial_vertex_unzoomed(float r_angle, float r_dist);
static void finish_radial(void);

static void radial_circle(int layer, float x, float y, int vertices, ALLEGRO_COLOR col, float circle_size);
static void double_circle_with_bloom(int layer, float x, float y, float circle_size, int player_index, int base_shade);
static void draw_circle(int layer, float x, float y, float circle_size, ALLEGRO_COLOR col);
//static void double_circle_with_bloom(int layer, float x, float y, float circle_size, int player_index, int base_shade);
//static void radial_blob(int layer, float x, float y, float base_angle, int vertices, ALLEGRO_COLOR col, float base_size, int drand_size, int drand_seed1, int drand_seed2);
//static void radial_elongated_blob_10(int layer, float x, float y, float base_angle, ALLEGRO_COLOR col, float base_size, int drand_size, int drand_seed1, int drand_seed2);

static void bloom_circle(int layer, float x, float y, ALLEGRO_COLOR col_centre, ALLEGRO_COLOR col_edge, float circle_size_zoomed);
//static void bloom_long(int layer, float x, float y, ALLEGRO_COLOR col_centre, ALLEGRO_COLOR col_edge, float circle_size_zoomed, float long_angle, float tail_width, float extra_length);
static void bloom_long(int layer, float x, float y, float angle, float length_zoomed, ALLEGRO_COLOR col_centre_start, ALLEGRO_COLOR col_edge_start, ALLEGRO_COLOR col_edge_end, float circle_size_start_zoomed, float circle_size_end_zoomed);

static void start_ribbon(int layer, float vertex1_x, float vertex1_y, float vertex2_x, float vertex2_y, ALLEGRO_COLOR fill_col);
//static void add_ribbon_vertex_vector(float base_x, float base_y, float angle, float dist, ALLEGRO_COLOR vertex_col);
static void add_ribbon_vertex(float x, float y, ALLEGRO_COLOR vertex_col);

static void draw_ring(int layer,
																						float x, float y,
																						float size,
																						float thickness,
																						int vertices,
																						ALLEGRO_COLOR ring_col);

//static void draw_proc_outline(float x, float y, al_fixed angle, int shape, float scale, int lines_only, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col, float zoom);
static void draw_jaggy_proc_outline(int layer, float x, float y, al_fixed angle, int shape, float scale, int jagginess, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col, float zoom);

static void draw_object_base_shape(float proc_x,
																																			float proc_y,
																																			float angle_float,
																																			float zoom,
																																			int proc_shape,
																																			int proc_link_index,
																																			ALLEGRO_COLOR proc_col);


static void select_arrows(int number, float centre_x, float centre_y, float select_arrow_angle, float dist, float out_dist, float side_angle, float side_dist, ALLEGRO_COLOR arrow_col);
static void draw_text_bubble(float bubble_x, float bubble_y, int bubble_time, int bubble_col, int bubble_text_length, char* bubble_text, int draw_triangle);
static void draw_data_well(int i, int j, struct backblock_struct* backbl,
																				float top_left_corner_x [BACKBLOCK_LAYERS],
                    float top_left_corner_y [BACKBLOCK_LAYERS],
                    int data_well_type,
                    float overlay_alpha);

struct ribbon_state_struct
{

	int layer;
	ALLEGRO_COLOR fill_col;
	int last_vertex_index;
	int current_vertex_index;

};
struct ribbon_state_struct ribstate;

#define BLOOM_RIBBON_VERTICES 64

struct bloom_ribbon_state_struct
{

	int layer;
	ALLEGRO_COLOR vertex_col [2]; // for now just have the same colours for the whole ribbon. Could do per-vertex instead.
	float vertex_x [BLOOM_RIBBON_VERTICES] [3];
	float vertex_y [BLOOM_RIBBON_VERTICES] [3];
	int vertex_pos;

};
struct bloom_ribbon_state_struct bribstate;

static void start_bloom_ribbon(int layer, float centre_x, float centre_y, float left_x, float left_y, float right_x, float right_y, ALLEGRO_COLOR centre_col, ALLEGRO_COLOR edge_col);
static void add_bloom_ribbon_vertices(float centre_x, float centre_y, float left_x, float left_y, float right_x, float right_y);
static void finish_bloom_ribbon(void);


/*
void draw_stream_beam(float x1, float by1, float x2, float y2, int col, int status, int counter, int hit);
void draw_dstream_beam(float x1, float by1, float x2, float y2, int col, int status, int counter, int hit);
void draw_thickline(float x1, float by1, float x2, float y2, float thickness, ALLEGRO_COLOR col);
//void draw_allocate_beam(float x1, float by1, float x2, float y2, int col, int counter);
void draw_allocate_beam2(float x1, float by1, float x2, float y2, int col, int prand_seed, int counter);
void draw_yield_beam(float x1, float by1, int target_proc_index, int col, int prand_seed, int counter);
void zap_line(float x1, float by1, float x2, float y2, int col, int prand_seed, int counter, int wave_amplitude);

void add_triangle(float xa, float ya, float xb, float yb, float xc, float yc, ALLEGRO_COLOR col);
void add_triangle_layer(int layer, float xa, float ya, float xb, float yb, float xc, float yc, ALLEGRO_COLOR col);
void add_outline_triangle(float xa, float ya, float xb, float yb, float xc, float yc, ALLEGRO_COLOR col1, ALLEGRO_COLOR col2);
void add_outline_triangle_layer(int layer, float xa, float ya, float xb, float yb, float xc, float yc, ALLEGRO_COLOR col1, ALLEGRO_COLOR col2);
void add_outline_diamond(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, ALLEGRO_COLOR col1, ALLEGRO_COLOR col2);
void add_diamond(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, ALLEGRO_COLOR col1);
void add_diamond_layer(int layer, float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, ALLEGRO_COLOR col1);
void add_outline_diamond_layer(int layer, float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, ALLEGRO_COLOR col1, ALLEGRO_COLOR col2);
void add_outline_pentagon_layer(int layer, float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, float xe, float ye, ALLEGRO_COLOR col1, ALLEGRO_COLOR col2);
void add_outline_hexagon_layer(int layer, float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, float xe, float ye, float xf, float yf, ALLEGRO_COLOR col1, ALLEGRO_COLOR col2);
void add_outline_square(float xa, float ya, float xb, float yb, ALLEGRO_COLOR col1, ALLEGRO_COLOR col2);
void add_outline_orthogonal_hexagon(float x, float y, float size, ALLEGRO_COLOR col1, ALLEGRO_COLOR col2);

//void add_outline_shape(float x, float y, float float_angle, struct shape_struct* sh, ALLEGRO_COLOR line_col1, ALLEGRO_COLOR line_col2, ALLEGRO_COLOR line_col3, ALLEGRO_COLOR fill_col);
void add_outline_shape(float x, float y, float float_angle, struct shape_struct* sh, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col, ALLEGRO_COLOR edge_col2);
void add_outline_shape2(float x, float y, float float_angle, struct shape_struct* sh, ALLEGRO_COLOR line_col1, ALLEGRO_COLOR line_col2, ALLEGRO_COLOR line_col3, ALLEGRO_COLOR fill_col);
void add_redundancy_lines(float x, float y, float float_angle, struct shape_struct* sh, ALLEGRO_COLOR line_col);

void add_scaled_outline_shape(struct shape_struct* sh, float float_angle, float x, float y, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col, float scale);
void add_scaled_outline(struct shape_struct* sh, float float_angle, float x, float y, ALLEGRO_COLOR edge_col, float scale);


void push_to_poly_buffer(int v, ALLEGRO_COLOR col);
void push_loop_to_line_buffer(int v, ALLEGRO_COLOR col);
void push_to_layer_poly_buffer(int layer, int v, ALLEGRO_COLOR col);
void push_loop_to_layer_line_buffer(int layer, int v, ALLEGRO_COLOR col);

void add_simple_rectangle_layer(int layer, float x, float y, float length, float width, float angle, ALLEGRO_COLOR col);
void add_simple_outline_rectangle_layer(int layer, float x, float y, float length, float width, float angle, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col);
void add_simple_outline_diamond_layer(int layer, float x, float y, float front_length, float back_length, float width, float angle, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col);
void add_simple_outline_triangle_layer(int layer, float x, float y, float angle_1, float length_1, float angle_2, float length_2, float angle_3, float length_3, ALLEGRO_COLOR col1, ALLEGRO_COLOR col2);
*/

void add_line(int layer, float x, float y, float xa, float ya, ALLEGRO_COLOR col);
static void add_orthogonal_hexagon(int layer, float x, float y, float size, ALLEGRO_COLOR col1);
static void add_stretched_hexagon(float x, float y, float size, ALLEGRO_COLOR col1);
static void add_orthogonal_rect(int layer, float xa, float ya, float xb, float yb, ALLEGRO_COLOR col1);
static void add_diagonal_octagon(int layer, float x, float y, float size, ALLEGRO_COLOR col1);

static void draw_data_well_exclusion_zones(void);

/*
static void draw_burst(float x, float y,
																							float size,
																							float angle,
																							int time,
																							int lifetime,
																							ALLEGRO_COLOR col_bright,
																							ALLEGRO_COLOR col_dim);
*/
void check_vbuf(void);
void draw_vbuf(void);



void add_line(int layer, float x, float y, float xa, float ya, ALLEGRO_COLOR col)
{

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

}

void add_line_vertex(float x, float y, ALLEGRO_COLOR col)
{
	vbuf.buffer_line[vbuf.vertex_pos_line].x = x;
	vbuf.buffer_line[vbuf.vertex_pos_line].y = y;
	vbuf.buffer_line[vbuf.vertex_pos_line].color = col;
	++vbuf.vertex_pos_line;
}

void construct_line(int layer, int v1, int v2)
{
	vbuf.index_line [layer] [vbuf.index_pos_line [layer]++] = v1;
	vbuf.index_line [layer] [vbuf.index_pos_line [layer]++] = v2;
}

static void add_orthogonal_hexagon(int layer, float x, float y, float size, ALLEGRO_COLOR col1)
{

	int m = vbuf.vertex_pos_triangle;

	add_tri_vertex(x, y - 1.0 * size, col1);
	add_tri_vertex(x + 0.866 * size, y - 0.5 * size, col1);
	add_tri_vertex(x + 0.866 * size, y + 0.5 * size, col1);
	add_tri_vertex(x, y + 1.0 * size, col1);
	add_tri_vertex(x - 0.866 * size, y + 0.5 * size, col1);
	add_tri_vertex(x - 0.866 * size, y - 0.5 * size, col1);

	construct_triangle(layer, m, m+1, m+5);
	construct_triangle(layer, m+1, m+2, m+5);
	construct_triangle(layer, m+2, m+4, m+5);
	construct_triangle(layer, m+2, m+3, m+4);

}


static void add_stretched_hexagon(float x, float y, float size, ALLEGRO_COLOR col1)
{

	int layer = 0;
	int m = vbuf.vertex_pos_triangle;

	add_tri_vertex(x, y - (BLOCK_SIZE_PIXELS* 0.65) * size, col1);
	add_tri_vertex(x + (BLOCK_SIZE_PIXELS/2) * size, y - (BLOCK_SIZE_PIXELS * 0.35) * size, col1);
	add_tri_vertex(x + (BLOCK_SIZE_PIXELS/2) * size, y + (BLOCK_SIZE_PIXELS * 0.35) * size, col1);
	add_tri_vertex(x, y + (BLOCK_SIZE_PIXELS * 0.65) * size, col1);
	add_tri_vertex(x - (BLOCK_SIZE_PIXELS/2) * size, y + (BLOCK_SIZE_PIXELS * 0.35) * size, col1);
	add_tri_vertex(x - (BLOCK_SIZE_PIXELS/2) * size, y - (BLOCK_SIZE_PIXELS * 0.35) * size, col1);

	construct_triangle(layer, m, m+1, m+5);
	construct_triangle(layer, m+1, m+2, m+5);
	construct_triangle(layer, m+2, m+4, m+5);
	construct_triangle(layer, m+2, m+3, m+4);

}


static void add_diagonal_octagon(int layer, float x, float y, float size, ALLEGRO_COLOR col1)
{

	float diag_dist = 0.707107 * size;
	int m = vbuf.vertex_pos_triangle;

	add_tri_vertex(x + size, y, col1);
	add_tri_vertex(x + diag_dist, y + diag_dist, col1);
	add_tri_vertex(x, y + size, col1);
	add_tri_vertex(x - diag_dist, y + diag_dist, col1);
	add_tri_vertex(x - size, y, col1);
	add_tri_vertex(x - diag_dist, y - diag_dist, col1);
	add_tri_vertex(x, y - size, col1);
	add_tri_vertex(x + diag_dist, y - diag_dist, col1);

	construct_triangle(layer, m, m+1, m+7);
	construct_triangle(layer, m+1, m+2, m+7);
	construct_triangle(layer, m+2, m+6, m+7);
	construct_triangle(layer, m+2, m+3, m+6);
	construct_triangle(layer, m+3, m+5, m+6);
	construct_triangle(layer, m+3, m+4, m+5);

}



static void add_orthogonal_rect(int layer, float xa, float ya, float xb, float yb, ALLEGRO_COLOR col1)
{

	int m = vbuf.vertex_pos_triangle;

	add_tri_vertex(xa, ya, col1);
	add_tri_vertex(xb, ya, col1);
	add_tri_vertex(xb, yb, col1);
	add_tri_vertex(xa, yb, col1);

	construct_triangle(layer, m, m+1, m+2);
	construct_triangle(layer, m+2, m+3, m);

}



extern struct view_struct view;

void run_display(void)
{

 int p; //, pk, c;
 struct proc_struct* pr;
// struct packet_struct* pack;
 struct cloud_struct* cl;
 float x, y; //, x2, y2;
 int i, j;
 int shade;
 int bubble_list_index = -1; // part of linked list used to draw bubble text

 al_set_target_bitmap(vision_mask);
 al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
// al_clear_to_color(colours.black);
#ifndef RECORDING_VIDEO_2
 if (game.vision_mask)
  al_clear_to_color(colours.black);
   else
    al_clear_to_color(al_map_rgba(0,0,0,120));
#else
 if (w.debug_mode == 0)
  al_clear_to_color(colours.black);

#endif

// al_draw_filled_rectangle(0, 0, 500, 500, al_map_rgba(0,0,0,120));
// al_draw_filled_rectangle(150, 150, 450, 450, al_map_rgba(0,0,0,0));


 al_set_target_bitmap(al_get_backbuffer(display));
// REMEMBER that al_set_clipping_rectangle uses width and height!!!
 al_set_clipping_rectangle(0, 0, panel[PANEL_MAIN].w, panel[PANEL_MAIN].h);
// al_set_clipping_rectangle(0, 0, inter.display_w, inter.display_h);

 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

// al_set_target_bitmap(al_get_backbuffer(display));
// al_clear_to_color(colours.black);

//#define SHOW_BLOCKS - doesn't work



#define SHOW_BLOCKS_ATTEMPT_2


#ifdef SHOW_BLOCKS_ATTEMPT_2


//  int block_size = BLOCK_SIZE_PIXELS;
  int screen_width_in_blocks = ((view.window_x_unzoomed / BLOCK_SIZE_PIXELS) / view.zoom) + 4;
  int screen_height_in_blocks = ((view.window_y_unzoomed / BLOCK_SIZE_PIXELS) / view.zoom) + 4;
  int bx;
  int by;
  int k;
  struct backblock_struct* backbl;

/*
// This convoluted series of things seems to be necessary to get the block drawing code to work smoothly
  float camera_x = al_fixtof(view.camera_x);
  float camera_y = al_fixtof(view.camera_y);
  float camera_x_mod_block = fmod(camera_x, BLOCK_SIZE_PIXELS);
  float camera_y_mod_block = fmod(camera_y, BLOCK_SIZE_PIXELS);
  float camera_x_zoomed = (view.window_x_unzoomed / (view.zoom * 2));// * 0.5;
  float camera_y_zoomed = (view.window_y_unzoomed / (view.zoom * 2));// * 0.5;
  float camera_x_zoomed_mod_block = fmod(camera_x_zoomed, BLOCK_SIZE_PIXELS);
  float camera_y_zoomed_mod_block = fmod(camera_y_zoomed, BLOCK_SIZE_PIXELS);

  float camera_offset_x = camera_x_mod_block - camera_x_zoomed_mod_block;
  float camera_offset_y = camera_y_mod_block - camera_y_zoomed_mod_block;
*/

  float camera_edge_x1 = 0;
  float camera_edge_x2 = view.window_x_zoomed;
  float camera_edge_y1 = 0;
  float camera_edge_y2 = view.window_y_zoomed;

// reset the line/poly drawing buffers (must come before these are used)
 clear_vbuf();

#define EDGE_SIZE (BLOCK_SIZE_PIXELS * 2)

#define EDGE_LINE_COL colours.base [COL_GREY] [SHADE_LOW]



 int clip_x1 = 0;
 int clip_y1 = 0;
 int clip_x2 = panel[PANEL_MAIN].w;
 int clip_y2 = panel[PANEL_MAIN].h;
 int reset_clipping = 0;

// First work out the y camera edges, because these are used when displaying the x lines as well:
//  camera_edge_y1 = (view.camera_y / GRAIN_MULTIPLY) - (view.window_y / 2);
  camera_edge_y1 = al_fixtoi(view.camera_y - view.centre_y_zoomed);
  if (camera_edge_y1 < EDGE_SIZE)
  {
   camera_edge_y1 = ((camera_edge_y1 * -1) + EDGE_SIZE) * view.zoom;
   camera_edge_y2 = view.window_y_zoomed;
  }
   else
   {
    camera_edge_y1 = 0;
    camera_edge_y2 = al_fixtoi(view.camera_y + view.centre_y_zoomed);
    if (camera_edge_y2 > view.w_y_pixel - EDGE_SIZE)
    {
//     camera_edge_y2 = view.window_y_zoomed - (camera_edge_y2 - view.w_y_pixel) - EDGE_SIZE;
//     camera_edge_y2 = view.window_y_zoomed + (camera_edge_y2 - view.w_y_pixel) - EDGE_SIZE;
//     camera_edge_y2 -= (view.w_y_pixel - EDGE_SIZE);
//     fprintf(stdout, "\n cey2 %f", camera_edge_y2);
//fprintf(stdout, "A");
    }
   }

// check whether the left side of the map is visible:
  camera_edge_x1 = al_fixtoi(view.camera_x - view.centre_x_zoomed);
  if (camera_edge_x1 < EDGE_SIZE)
  {
   camera_edge_x1 = ((camera_edge_x1 * -1) + EDGE_SIZE) * view.zoom;
   clip_x1 = camera_edge_x1 - 1;
   reset_clipping = 1;
   camera_edge_x2 = view.window_x_unzoomed;
  }
   else
   {
    camera_edge_x1 = 0;
    camera_edge_x2 = al_fixtoi(view.camera_x + view.centre_x_zoomed);
    if (camera_edge_x2 > view.w_x_pixel - EDGE_SIZE)
    {
     camera_edge_x2 = (float) view.window_x_unzoomed - (float) (al_fixtof(view.camera_x) + ((float) view.window_x_zoomed / 2) - ((float) view.w_x_pixel - EDGE_SIZE)) * view.zoom;
     clip_x2 = camera_edge_x2 + 1;
     reset_clipping = 1;
    }
     else
					{
      camera_edge_x2 = view.window_x_unzoomed;
					}
   }

// Now draw the y lines:
  if (al_fixtoi(view.camera_y - view.centre_y_zoomed) < EDGE_SIZE)
  {
   clip_y1 = camera_edge_y1 - 1;
   reset_clipping = 1;
  }
   else
   {
    if (al_fixtoi(view.camera_y + view.centre_y_zoomed) > view.w_y_pixel - EDGE_SIZE)
    {
     camera_edge_y2 = (float) view.window_y_unzoomed - (float) (al_fixtof(view.camera_y + view.centre_y_zoomed) - (float) (view.w_y_pixel - EDGE_SIZE)) * view.zoom;
     clip_y2 = camera_edge_y2 + 1;
     reset_clipping = 1;
    }
   }

  if (reset_clipping == 1)
		{
   al_clear_to_color(colours.black);
   al_set_clipping_rectangle(clip_x1, clip_y1, clip_x2 - clip_x1, clip_y2 - clip_y1);
  }

  al_clear_to_color(colours.world_background);




/*
Work out top left corner of map
work out mod of it by the size of a zoomed block


work out min block_x index by:
 - working out block in centre of screen
 - subtracting (screen_width_in_blocks / 2)

work out float x coordinate for that block in same way as for any object in world


*/


//  float top_left_corner_x = al_fixtof(0 - view.camera_x) * view.zoom + (view.window_x_unzoomed / 2);
//  float top_left_corner_y = al_fixtof(0 - view.camera_y) * view.zoom + (view.window_y_unzoomed / 2);


  int screen_centre_block_x, screen_centre_block_y;

  int min_block_x;
  int min_block_y;
  int max_block_x;
  int max_block_y;

// work out the top left corner of block[min_block_x][min_block_y], in screen pixels
//  float top_left_corner_x;
//  float top_left_corner_y;

//#define BACKBLOCK_PARALLAX 1.04


  screen_centre_block_x = fixed_to_block(view.camera_x); // unaffected by parallax
  screen_centre_block_y = fixed_to_block(view.camera_y);

//  min_block_x = screen_centre_block_x - ((screen_width_in_blocks / 2) * BACKBLOCK_PARALLAX); // need to fix this
//  min_block_y = screen_centre_block_y - ((screen_height_in_blocks / 2) * BACKBLOCK_PARALLAX);
  min_block_x = screen_centre_block_x - ((screen_width_in_blocks / 2) * 1); // need to fix this
  min_block_y = screen_centre_block_y - ((screen_height_in_blocks / 2) * 1);
  max_block_x = min_block_x + screen_width_in_blocks + 1;
  max_block_y = min_block_y + screen_height_in_blocks;

  if (min_block_x < 0)
	  min_block_x = 0;
  if (min_block_y < 0)
	  min_block_y = 0;
  if (max_block_x > w.blocks.x)
	  max_block_x = w.blocks.x;
  if (max_block_y > w.blocks.y)
	  max_block_y = w.blocks.y;

  al_fixed special_camera_x [BACKBLOCK_LAYERS];
  al_fixed special_camera_y [BACKBLOCK_LAYERS];

// work out the top left corner of block[min_block_x][min_block_y], in screen pixels
  float top_left_corner_x [BACKBLOCK_LAYERS];
  float top_left_corner_y [BACKBLOCK_LAYERS];

  for (i = 0; i < BACKBLOCK_LAYERS; i ++)
		{
			special_camera_x [i] = view.camera_x * w.backblock_parallax [i];
			special_camera_y [i] = view.camera_y * w.backblock_parallax [i];

			int layer_min_block_x = screen_centre_block_x - ((screen_width_in_blocks / 2) / w.backblock_parallax [i]);
			int layer_min_block_y = screen_centre_block_y - ((screen_height_in_blocks / 2) / w.backblock_parallax [i]);

   top_left_corner_x [i] = al_fixtof((layer_min_block_x * BLOCK_SIZE_PIXELS) - special_camera_x [i]) * view.zoom + (view.window_x_unzoomed / 2);
   top_left_corner_y [i] = al_fixtof((layer_min_block_y * BLOCK_SIZE_PIXELS) - special_camera_y [i]) * view.zoom + (view.window_y_unzoomed / 2);


//   top_left_corner_x [i] = al_fixtof((min_block_x * BLOCK_SIZE_PIXELS) - special_camera_x) * view.zoom + (view.window_x_unzoomed / 2);
//   top_left_corner_y [i] = al_fixtof((min_block_y * BLOCK_SIZE_PIXELS) - special_camera_y) * view.zoom + (view.window_y_unzoomed / 2);

		}



//  float lower_level_zoom = view.zoom / BACKBLOCK_PARALLAX;

  float bx2, by2;

// 12 should be plenty
#define DEFERRED_DATA_WELL_DRAWINGS 12

	int deferred_data_wells = 0;
	int deferred_data_well_draw_i [DEFERRED_DATA_WELL_DRAWINGS];
	int deferred_data_well_draw_j [DEFERRED_DATA_WELL_DRAWINGS];
	struct backblock_struct* deferred_data_well_draw_backbl [DEFERRED_DATA_WELL_DRAWINGS];



if (!settings.option[OPTION_NO_BACKGROUND])
{

	if (!settings.option[OPTION_FAST_BACKGROUND])
	{
// The following is the code to draw the full background:
  for (i = min_block_x; i < max_block_x; i ++)
  {

   bx = i; //base_bx + i;

   check_vbuf();


   for (j = min_block_y; j < max_block_y; j ++)
   {
    by = j;//base_by + j;

    backbl = &w.backblock [bx] [by];
/*
        bx2 = top_left_corner_x [3] + (BLOCK_SIZE_PIXELS * view.zoom * w.backblock_parallax [3]) * (i); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
        by2 = top_left_corner_y [3] + (BLOCK_SIZE_PIXELS * view.zoom * w.backblock_parallax [3]) * (j); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
        float bx4 = bx2 + (BLOCK_SIZE_PIXELS * view.zoom * w.backblock_parallax [backbl->node_depth [3]]);
        float by4 = by2 + (BLOCK_SIZE_PIXELS * view.zoom * w.backblock_parallax [backbl->node_depth [3]]);
        add_diamond_layer(4,
																										bx2 + 3, by2 + 3,
																										bx4 - 3, by2 + 3,
																										bx4 - 3, by4 - 3,
																										bx2 + 3, by4 - 3,
																										colours.base_trans [COL_RED] [SHADE_LOW] [TRANS_FAINT]);

*/

    switch(backbl->backblock_type)
    {
				 case BACKBLOCK_BASIC_HEX:
				 	{


//       bx2 = top_left_corner_x [] + (BLOCK_SIZE_PIXELS * lower_level_zoom) * (i); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
//       by2 = top_left_corner_y + (BLOCK_SIZE_PIXELS * lower_level_zoom) * (j); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

       float nsize;
       int nfillcol;
       int node_colour;
       int node_saturation;

       for (k = 0; k < 9; k ++)
       {
        if (backbl->node_exists [k] == 0)
									continue;

								float specific_zoom = view.zoom * w.backblock_parallax [backbl->node_depth [k]];

//						  top_left_corner_x = al_fixtof((min_block_x * BLOCK_SIZE_PIXELS) - (view.camera_x / backbl->node_depth [k])) * view.zoom + (view.window_x_unzoomed / 2);
//        top_left_corner_y = al_fixtof((min_block_y * BLOCK_SIZE_PIXELS) - (view.camera_y / backbl->node_depth [k])) * view.zoom + (view.window_y_unzoomed / 2);

       bx2 = top_left_corner_x [backbl->node_depth [k]] + (BLOCK_SIZE_PIXELS * specific_zoom) * (i); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
       by2 = top_left_corner_y [backbl->node_depth [k]] + (BLOCK_SIZE_PIXELS * specific_zoom) * (j); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);


        nsize = backbl->node_size [k];
        nfillcol = 0;
        node_colour = backbl->node_team_col [k];
        node_saturation = backbl->node_col_saturation [k];
        if (w.world_time > backbl->node_colour_change_timestamp [k])
								{
         node_colour = backbl->node_new_colour [k];
								 node_saturation = backbl->node_new_saturation [k];
								}

        if (backbl->node_pending_explosion_timestamp [k] > w.world_time
									&& backbl->node_pending_explosion_timestamp [k] < w.world_time + 32)
								{
//									float explosion_strength_at_node =
         float size_increase = (backbl->node_pending_explosion_timestamp [k] - w.world_time) / 2;
         if (size_increase > 6)
										size_increase = 6;
         nsize += size_increase;
         nfillcol = (backbl->node_pending_explosion_timestamp [k] - w.world_time) / 4;

         if (nfillcol >= BACK_COL_FADE)
          nfillcol = BACK_COL_FADE - 1;
         if (nfillcol < 0)
										nfillcol = 0;


								}

        nsize += 0.5;



       add_orthogonal_hexagon((BACKBLOCK_LAYERS - 1) - backbl->node_depth [k], bx2 + (backbl->node_x [k]) * specific_zoom, by2 + (backbl->node_y [k]) * specific_zoom, nsize * specific_zoom,
                                      colours.back_fill [backbl->node_depth [k]] [node_colour] [node_saturation] [nfillcol]);


//               al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREEN] [SHADE_MAX],
//																				bx2 + (backbl->node_x [k]) * specific_zoom, by2 + (backbl->node_y [k]) * specific_zoom,
//																				0, "%i:%i", j & 1, k);

//               al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREEN] [SHADE_MAX],
//																				bx2 + (backbl->node_x [k]) * specific_zoom, by2 + (backbl->node_y [k]) * specific_zoom, 0,
//																													"%i", backbl->node_depth [k]);


       }
				 	}
					 break;

					case BACKBLOCK_DATA_WELL:
					case BACKBLOCK_DATA_WELL_EDGE:
						{
							if (w.data_well[backbl->backblock_value].last_drawn == game.total_time)
								continue;

							w.data_well[backbl->backblock_value].last_drawn = game.total_time;

							sancheck(deferred_data_wells, 0, DEFERRED_DATA_WELL_DRAWINGS, "deferred_data_wells");

      	deferred_data_well_draw_i [deferred_data_wells] = i;
	      deferred_data_well_draw_j [deferred_data_wells] = j;
	      deferred_data_well_draw_backbl [deferred_data_wells] = backbl;
       deferred_data_wells ++;



						}
      break; // end data well drawing code

    	default:
						break;

    }


   }
  }

	} // end if (!settings.option[OPTION_FAST_BACKGROUND])
	 else
		{

// The following is the code to draw the reduced detail (1) background:
  for (i = min_block_x; i < max_block_x; i ++)
  {

   bx = i; //base_bx + i;

   check_vbuf();


   for (j = min_block_y; j < max_block_y; j ++)
   {
    by = j;//base_by + j;

    backbl = &w.backblock [bx] [by];

    switch(backbl->backblock_type)
    {
				 case BACKBLOCK_BASIC_HEX:
				 	{

       float nsize;
       int nfillcol;
       int node_colour;
       int node_saturation;

       for (k = 0; k < 9; k ++)
       {
        if (backbl->node_exists [k] == 0)
									continue;

								float specific_zoom = view.zoom * w.backblock_parallax [backbl->node_depth [k]];

       bx2 = top_left_corner_x [backbl->node_depth [k]] + (BLOCK_SIZE_PIXELS * specific_zoom) * (i); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
       by2 = top_left_corner_y [backbl->node_depth [k]] + (BLOCK_SIZE_PIXELS * specific_zoom) * (j); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

// node_size is set a bit smaller (in the world generation functions) with this setting activated
        nsize = backbl->node_size [k];
        nfillcol = 0;
        node_colour = backbl->node_team_col [k];
        node_saturation = backbl->node_col_saturation [k];
        if (w.world_time > backbl->node_colour_change_timestamp [k])
								{
         node_colour = backbl->node_new_colour [k];
								 node_saturation = backbl->node_new_saturation [k];
								}

        if (backbl->node_pending_explosion_timestamp [k] > w.world_time
									&& backbl->node_pending_explosion_timestamp [k] < w.world_time + 32)
								{
//									float explosion_strength_at_node =
         float size_increase = (backbl->node_pending_explosion_timestamp [k] - w.world_time) / 2;
         if (size_increase > 6)
										size_increase = 6;
         nsize += size_increase;
         nfillcol = (backbl->node_pending_explosion_timestamp [k] - w.world_time) / 4;

         if (nfillcol >= BACK_COL_FADE)
          nfillcol = BACK_COL_FADE - 1;
         if (nfillcol < 0)
										nfillcol = 0;


								}

//        nsize -= 1;
        nsize *= specific_zoom;

        float diamond_centre_x = bx2 + (backbl->node_x [k]) * specific_zoom;
        float diamond_centre_y = by2 + (backbl->node_y [k]) * specific_zoom;

/*
        add_diamond_layer((BACKBLOCK_LAYERS - 1) - backbl->node_depth [k],
																										diamond_centre_x,
																										diamond_centre_y - nsize,
																										diamond_centre_x + nsize,
																										diamond_centre_y,
																										diamond_centre_x,
																										diamond_centre_y + nsize,
																										diamond_centre_x - nsize,
																										diamond_centre_y,
                          colours.back_fill [backbl->node_depth [k]] [node_colour] [node_saturation] [nfillcol]);
*/

        add_diamond_layer((BACKBLOCK_LAYERS - 1) - backbl->node_depth [k],
																										diamond_centre_x - nsize,
																										diamond_centre_y - nsize,
																										diamond_centre_x + nsize,
																										diamond_centre_y - nsize,
																										diamond_centre_x + nsize,
																										diamond_centre_y + nsize,
																										diamond_centre_x - nsize,
																										diamond_centre_y + nsize,
                          colours.back_fill [backbl->node_depth [k]] [node_colour] [node_saturation] [nfillcol]);



//               al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREEN] [SHADE_MAX],
//																				bx2 + (backbl->node_x [k]) * specific_zoom, by2 + (backbl->node_y [k]) * specific_zoom,
//																				0, "%i:%i", j & 1, k);

//               al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREEN] [SHADE_MAX],
//																				bx2 + (backbl->node_x [k]) * specific_zoom, by2 + (backbl->node_y [k]) * specific_zoom, 0,
//																													"%i", backbl->node_depth [k]);


       }
				 	}
					 break;

					case BACKBLOCK_DATA_WELL:
					case BACKBLOCK_DATA_WELL_EDGE:
						{
							if (w.data_well[backbl->backblock_value].last_drawn == game.total_time)
								continue;

							w.data_well[backbl->backblock_value].last_drawn = game.total_time;

							sancheck(deferred_data_wells, 0, DEFERRED_DATA_WELL_DRAWINGS, "deferred_data_wells");

      	deferred_data_well_draw_i [deferred_data_wells] = i;
	      deferred_data_well_draw_j [deferred_data_wells] = j;
	      deferred_data_well_draw_backbl [deferred_data_wells] = backbl;
       deferred_data_wells ++;



						}
      break; // end data well drawing code

    	default:
						break;

    }


   }
  }







		}


  i = 0;

  while (i < deferred_data_wells)
		{

       draw_data_well(deferred_data_well_draw_i [i],
																						deferred_data_well_draw_j [i],
																						deferred_data_well_draw_backbl [i],
																						top_left_corner_x, top_left_corner_y,
																						w.story_area, 0);


			i++;

		};


} // end if (!settings.option[OPTION_NO_BACKGROUND])
 else
	{
// in no_background mode, only data wells are drawn:

//int test = 0;

  deferred_data_wells = 0;


  for (i = 0; i < w.data_wells; i ++)
		{
			if (w.data_well[i].block_position.x >= min_block_x - 6
				&& w.data_well[i].block_position.x <= max_block_x + 6
				&& w.data_well[i].block_position.y >= min_block_y - 6
				&& w.data_well[i].block_position.y <= max_block_y + 6)
			{
       draw_data_well(w.data_well[i].block_position.x,
																						w.data_well[i].block_position.y,
																						&w.backblock[w.data_well[i].block_position.x][w.data_well[i].block_position.y],
																						top_left_corner_x, top_left_corner_y,
																						w.story_area, 0);

      	deferred_data_well_draw_i [deferred_data_wells] = w.data_well[i].block_position.x;
	      deferred_data_well_draw_j [deferred_data_wells] = w.data_well[i].block_position.y;
	      deferred_data_well_draw_backbl [deferred_data_wells] = &w.backblock[w.data_well[i].block_position.x][w.data_well[i].block_position.y];
       deferred_data_wells ++;


//test ++;
			}
		}

	}

  draw_vbuf();


/*


  screen_centre_block_x = fixed_to_block(view.camera_x);
  screen_centre_block_y = fixed_to_block(view.camera_y);

  min_block_x = screen_centre_block_x - (screen_width_in_blocks / 2);
  min_block_y = screen_centre_block_y - (screen_height_in_blocks / 2);
  max_block_x = min_block_x + screen_width_in_blocks + 1;
  max_block_y = min_block_y + screen_height_in_blocks;

// work out the top left corner of block[min_block_x][min_block_y], in screen pixels
  top_left_corner_x = al_fixtof((min_block_x * BLOCK_SIZE_PIXELS) - view.camera_x) * view.zoom + (view.window_x_unzoomed / 2);
  top_left_corner_y = al_fixtof((min_block_y * BLOCK_SIZE_PIXELS) - view.camera_y) * view.zoom + (view.window_y_unzoomed / 2);















  for (i = min_block_x; i < max_block_x; i ++)
  {

   bx = i; //base_bx + i;


   if (bx < 0)
    continue;
   if (bx >= w.blocks.x)
    break;

   check_vbuf();


   for (j = min_block_y; j < max_block_y; j ++)
   {
    by = j;//base_by + j;

    if (by < 0)
     continue;
    if (by >= w.blocks.y)
     break;

    backbl = &w.backblock [BACKBLOCK_LEVEL_UPPER] [bx] [by];

    switch(backbl->backblock_type)
    {
				 case BACKBLOCK_BASIC_HEX:
				 	{
       bx2 = top_left_corner_x + (BLOCK_SIZE_PIXELS * view.zoom) * (i); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
       by2 = top_left_corner_y + (BLOCK_SIZE_PIXELS * view.zoom) * (j); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

       float nsize;
       int nfillcol;
       int node_colour;
       int node_saturation;

       for (k = 0; k < 9; k ++)
       {
        if (backbl->node_exists [k] == 0)
									continue;
        nsize = backbl->node_size [k];
        nfillcol = 0;
        node_colour = backbl->node_team_col [k];
        node_saturation = backbl->node_col_saturation [k];
        if (w.world_time > backbl->node_colour_change_timestamp [k])
								{
         node_colour = backbl->node_new_colour [k];
								 node_saturation = backbl->node_new_saturation [k];
								}

        if (backbl->node_pending_explosion_timestamp [k] > w.world_time
									&& backbl->node_pending_explosion_timestamp [k] < w.world_time + 32)
								{
//									float explosion_strength_at_node =
         float size_increase = (backbl->node_pending_explosion_timestamp [k] - w.world_time) / 2;
         if (size_increase > 6)
										size_increase = 6;
         nsize += size_increase;
         nfillcol = (backbl->node_pending_explosion_timestamp [k] - w.world_time) / 4;

         if (nfillcol >= BACK_COL_FADE)
          nfillcol = BACK_COL_FADE - 1;
         if (nfillcol < 0)
										nfillcol = 0;


								}

        nsize += 0.5;

node_saturation = 1;

       add_orthogonal_hexagon(0, bx2 + (backbl->node_x [k]) * view.zoom, by2 + (backbl->node_y [k]) * view.zoom, nsize * view.zoom,
                                      colours.back_fill [node_colour] [node_saturation] [nfillcol]);
       }
				 	}
					 break;

					case BACKBLOCK_DATA_WELL:
						{
       bx2 = top_left_corner_x + (BLOCK_SIZE_PIXELS * view.zoom) * (i) + (BLOCK_SIZE_PIXELS/2) * view.zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
       by2 = top_left_corner_y + (BLOCK_SIZE_PIXELS * view.zoom) * (j) + (BLOCK_SIZE_PIXELS/2) * view.zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
//       bx2 = (((i * BLOCK_SIZE_PIXELS) - camera_offset_x) + (BLOCK_SIZE_PIXELS/2)) * view.zoom;
//       by2 = (((j * BLOCK_SIZE_PIXELS) - camera_offset_y) + (BLOCK_SIZE_PIXELS/2)) * view.zoom;

//    fprintf(stdout, "\n [tlc %f,%f] [bx,by %i,%i] [bx2,by2 %f,%f]", top_left_corner_x, top_left_corner_y, bx,by, bx2, by2);


       float well_size = 20;
       seed_drand(bx+by);

       for (k = 0; k < 6; k++)
							{

float base_dist = drand(30, bx);
well_size = base_dist + 32;

#define BASE_WELL_ANGLE (PI/6)

								float left_xpart = cos(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * view.zoom;
								float left_ypart = sin(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * view.zoom;
								float right_xpart = cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * view.zoom;
								float right_ypart = sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * view.zoom;
								vertex_list[0][0] = bx2 + left_xpart * 32;// * well_size;
								vertex_list[0][1] = by2 + left_ypart * 32;// * well_size;
								vertex_list[1][0] = bx2 + right_xpart * 32; //well_size;
								vertex_list[1][1] = by2 + right_ypart * 32; //well_size;
								vertex_list[2][0] = vertex_list[1][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * view.zoom;
								vertex_list[2][1] = vertex_list[1][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * view.zoom;
								vertex_list[3][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * view.zoom;
								vertex_list[3][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * view.zoom;
								vertex_list[4][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * view.zoom;
								vertex_list[4][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * view.zoom;
								vertex_list[5][0] = vertex_list[0][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * view.zoom;
								vertex_list[5][1] = vertex_list[0][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * view.zoom;

        add_poly_layer(0, 6, colours.data_well_hexes);


							}

							int transfer_colour_adjust;

       if (w.world_time - w.data_well[backbl->backblock_value].last_transferred < 32)
								transfer_colour_adjust = 32 - (w.world_time - w.data_well[backbl->backblock_value].last_transferred);
							  else
										transfer_colour_adjust = 0;

							if (w.data_well[backbl->backblock_value].data > 0)
							{
        add_orthogonal_hexagon(0, bx2, by2, (30 * w.data_well[backbl->backblock_value].data * view.zoom) / w.data_well[backbl->backblock_value].data_max, al_map_rgba(220 + transfer_colour_adjust, 200 - transfer_colour_adjust, 50 - transfer_colour_adjust, 180 + transfer_colour_adjust));
							}

							float base_well_ring_angle = PI/6 + (w.world_time * w.data_well[backbl->backblock_value].spin_rate);
#define WELL_RING_RADIUS (140)
       ALLEGRO_COLOR reserve_colour;

							reserve_colour = al_map_rgba(180,
																																				100,
																																				30,
																																				140);

       for (k = 0; k < DATA_WELL_RESERVES; k++)
							{
								if (w.data_well[backbl->backblock_value].reserve_data [k] == 0)
									continue;
								int l;
								float reserve_square_arc = 0.13;
								float reserve_square_length = 36;
								float reserve_data_proportion = w.data_well[backbl->backblock_value].reserve_data [k] * 0.002;
								reserve_square_length *= reserve_data_proportion;
								if (reserve_data_proportion < 1)
								 reserve_square_arc *= reserve_data_proportion;
								for (l = 0; l < w.data_well[backbl->backblock_value].reserve_squares; l ++)
								{
									float square_angle = base_well_ring_angle + ((PI*2) / w.data_well[backbl->backblock_value].reserve_squares) * l;
									add_diamond_layer(0,
																											bx2 + cos(square_angle - reserve_square_arc) * WELL_RING_RADIUS*view.zoom,
																											by2 + sin(square_angle - reserve_square_arc) * WELL_RING_RADIUS*view.zoom,
																											bx2 + cos(square_angle + reserve_square_arc) * WELL_RING_RADIUS*view.zoom,
																											by2 + sin(square_angle + reserve_square_arc) * WELL_RING_RADIUS*view.zoom,
																											bx2 + cos(square_angle + reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*view.zoom,
																											by2 + sin(square_angle + reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*view.zoom,
																											bx2 + cos(square_angle - reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*view.zoom,
																											by2 + sin(square_angle - reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*view.zoom,
																											reserve_colour);
								}
								base_well_ring_angle += PI / w.data_well[backbl->backblock_value].reserve_squares;
							}


						}
      break; // end data well drawing code

    	default:
						break;

    }


   }
  }

  draw_vbuf();


*/























  if (reset_clipping == 1)
   al_set_clipping_rectangle(0, 0, panel[PANEL_MAIN].w, panel[PANEL_MAIN].h);


#endif

// the following code draws the blocks on the screen. It's horribly unoptimised
#ifdef SHOW_BLOCKS
//  int block_size = BLOCK_SIZE_PIXELS;
  int screen_width_in_blocks = ((view.window_x_unzoomed / BLOCK_SIZE_PIXELS) / view.zoom) + 4;
  int screen_height_in_blocks = ((view.window_y_unzoomed / BLOCK_SIZE_PIXELS) / view.zoom) + 3;
  int bx;
  int by;
  int k;
  struct block_struct* bl;


// This convoluted series of things seems to be necessary to get the block drawing code to work smoothly
  float camera_x = al_fixtof(view.camera_x);
  float camera_y = al_fixtof(view.camera_y);
  float camera_x_mod_block = fmod(camera_x, BLOCK_SIZE_PIXELS);
  float camera_y_mod_block = fmod(camera_y, BLOCK_SIZE_PIXELS);
  float camera_x_zoomed = (view.window_x_unzoomed / (view.zoom * 2));// * 0.5;
  float camera_y_zoomed = (view.window_y_unzoomed / (view.zoom * 2));// * 0.5;
  float camera_x_zoomed_mod_block = fmod(camera_x_zoomed, BLOCK_SIZE_PIXELS);
  float camera_y_zoomed_mod_block = fmod(camera_y_zoomed, BLOCK_SIZE_PIXELS);

  float camera_offset_x = camera_x_mod_block - camera_x_zoomed_mod_block;
  float camera_offset_y = camera_y_mod_block - camera_y_zoomed_mod_block;


  float camera_edge_x1 = 0;
  float camera_edge_x2 = view.window_x_zoomed;
  float camera_edge_y1 = 0;
  float camera_edge_y2 = view.window_y_zoomed;

// reset the line/poly drawing buffers (must come before these are used)
 clear_vbuf();

#define EDGE_SIZE (BLOCK_SIZE_PIXELS * 2)

#define EDGE_LINE_COL colours.base [COL_GREY] [SHADE_LOW]



 int clip_x1 = 0;
 int clip_y1 = 0;
 int clip_x2 = panel[PANEL_MAIN].w;
 int clip_y2 = panel[PANEL_MAIN].h;
 int reset_clipping = 0;

// First work out the y camera edges, because these are used when displaying the x lines as well:
//  camera_edge_y1 = (view.camera_y / GRAIN_MULTIPLY) - (view.window_y / 2);
  camera_edge_y1 = al_fixtoi(view.camera_y - view.centre_y_zoomed);
  if (camera_edge_y1 < EDGE_SIZE)
  {
   camera_edge_y1 = ((camera_edge_y1 * -1) + EDGE_SIZE) * view.zoom;
   camera_edge_y2 = view.window_y_zoomed;
  }
   else
   {
    camera_edge_y1 = 0;
    camera_edge_y2 = al_fixtoi(view.camera_y + view.centre_y_zoomed);
    if (camera_edge_y2 > view.w_y_pixel - EDGE_SIZE)
    {
//     camera_edge_y2 = view.window_y_zoomed - (camera_edge_y2 - view.w_y_pixel) - EDGE_SIZE;
//     camera_edge_y2 = view.window_y_zoomed + (camera_edge_y2 - view.w_y_pixel) - EDGE_SIZE;
//     camera_edge_y2 -= (view.w_y_pixel - EDGE_SIZE);
//     fprintf(stdout, "\n cey2 %f", camera_edge_y2);
//fprintf(stdout, "A");
    }
   }

// check whether the left side of the map is visible:
  camera_edge_x1 = al_fixtoi(view.camera_x - view.centre_x_zoomed);
  if (camera_edge_x1 < EDGE_SIZE)
  {
   camera_edge_x1 = ((camera_edge_x1 * -1) + EDGE_SIZE) * view.zoom;
//     add_line(0, camera_edge_x1, camera_edge_y1, camera_edge_x1, clip_y2,
//              EDGE_LINE_COL);
   clip_x1 = camera_edge_x1 - 1;
   reset_clipping = 1;
   camera_edge_x2 = view.window_x_unzoomed;
  }
   else
   {
    camera_edge_x1 = 0;
    camera_edge_x2 = al_fixtoi(view.camera_x + view.centre_x_zoomed);
    if (camera_edge_x2 > view.w_x_pixel - EDGE_SIZE)
    {
//     camera_edge_x2 = view.window_x_unzoomed - (((camera_edge_x2 - view.w_x_pixel) - EDGE_SIZE) * view.zoom);
//     camera_edge_x2 = (float) view.window_x_unzoomed - (float) (al_fixtof(view.camera_x + view.centre_x_zoomed) - ((float) view.w_x_pixel - EDGE_SIZE)) * view.zoom;
     camera_edge_x2 = (float) view.window_x_unzoomed - (float) (al_fixtof(view.camera_x) + ((float) view.window_x_zoomed / 2) - ((float) view.w_x_pixel - EDGE_SIZE)) * view.zoom;
//     add_line(0, camera_edge_x2, camera_edge_y1, camera_edge_x2, clip_y2,
//              EDGE_LINE_COL);
     clip_x2 = camera_edge_x2 + 1;
     reset_clipping = 1;
    }
     else
					{
      camera_edge_x2 = view.window_x_unzoomed;
					}
   }

// Now draw the y lines:
  if (al_fixtoi(view.camera_y - view.centre_y_zoomed) < EDGE_SIZE)
  {
//   add_line(0, camera_edge_x1, camera_edge_y1, camera_edge_x2, camera_edge_y1,
//            EDGE_LINE_COL);
   clip_y1 = camera_edge_y1 - 1;
   reset_clipping = 1;
  }
   else
   {
    if (al_fixtoi(view.camera_y + view.centre_y_zoomed) > view.w_y_pixel - EDGE_SIZE)
    {
//     camera_edge_y2 = view.window_y_unzoomed - (((camera_edge_y2 - view.w_y_pixel) - EDGE_SIZE) * view.zoom);
//     camera_edge_y2 = view.window_y_unzoomed - (((al_fixtoi(view.camera_x) - view.w_y_pixel) - EDGE_SIZE));
     camera_edge_y2 = (float) view.window_y_unzoomed - (float) (al_fixtof(view.camera_y + view.centre_y_zoomed) - (float) (view.w_y_pixel - EDGE_SIZE)) * view.zoom;
     //view.window_y_unzoomed - (((view.w_y_pixel - al_fixtoi(view.camera_x)) - EDGE_SIZE));
//     add_line(0, camera_edge_x1, camera_edge_y2, camera_edge_x2, camera_edge_y2,
//              EDGE_LINE_COL);
     clip_y2 = camera_edge_y2 + 1;
     reset_clipping = 1;
    }
   }

//    al_clear_to_color(colours.world_background);


  if (reset_clipping == 1)
		{
   al_clear_to_color(colours.black);
   al_set_clipping_rectangle(clip_x1, clip_y1, clip_x2 - clip_x1, clip_y2 - clip_y1);
//   al_clear_to_color(colours.world_background);
  }
//		 else

  al_clear_to_color(colours.world_background);



  float bx2, by2;

  int base_bx = ((camera_x - camera_x_mod_block) / BLOCK_SIZE_PIXELS) - ((camera_x_zoomed - camera_x_zoomed_mod_block) / BLOCK_SIZE_PIXELS);

 fpr("\n base_bx %i camera_x %f camera_x_mod_block %f camera_x_zoomed %f camera_x_zoomed_mod_block %f", base_bx, camera_x, camera_x_mod_block, camera_x_zoomed, camera_x_zoomed_mod_block);

  for (i = -2; i < screen_width_in_blocks; i ++)
  {

  bx = base_bx + i;


   if (bx < 0)
    continue;
   if (bx >= w.blocks.x)
    break;

   check_vbuf();

   int base_by = ((camera_y - camera_y_mod_block) / BLOCK_SIZE_PIXELS) - ((camera_y_zoomed - camera_y_zoomed_mod_block) / BLOCK_SIZE_PIXELS);

   for (j = -2; j < screen_height_in_blocks; j ++)
   {
    by = base_by + j;
//    by = fixed_to_block(view.camera_y) - fixed_to_block(view.centre_y_zoomed) + j;

    if (by < 0)
     continue;
    if (by >= w.blocks.y)
     break;

//    fprintf(stdout, "\nbx,by %i,%i", bx, by);

    bl = &w.block [bx] [by];

    switch(bl->backblock_type)
    {
				 case BACKBLOCK_BASIC_HEX:
				 	{
       bx2 = ((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
       by2 = ((j * BLOCK_SIZE_PIXELS) - camera_offset_y);

       float nsize;
       int nfillcol;
       int node_colour;
       int node_saturation;

       for (k = 0; k < 9; k ++)
       {
        if (bl->node_exists [k] == 0)
									continue;
        nsize = bl->node_size [k];
        nfillcol = 0;
        node_colour = bl->node_team_col [k];
        node_saturation = bl->node_col_saturation [k];
        if (w.world_time > bl->node_colour_change_timestamp [k])
								{
         node_colour = bl->node_new_colour [k];
								 node_saturation = bl->node_new_saturation [k];
								}

/*
        if (bl->node_disrupt_timestamp [k] > w.world_time)
        {
         float size_increase = (bl->node_disrupt_timestamp [k] - w.world_time) / 2;
         if (size_increase > 6)
										size_increase = 6;
         nsize += size_increase;
         nfillcol = (bl->node_disrupt_timestamp [k] - w.world_time) / 2;
         if (nfillcol >= BACK_COL_FADE)
          nfillcol = BACK_COL_FADE - 1;

        }
*/

        if (bl->node_pending_explosion_timestamp [k] > w.world_time
									&& bl->node_pending_explosion_timestamp [k] < w.world_time + 32)
								{
//									float explosion_strength_at_node =
         float size_increase = (bl->node_pending_explosion_timestamp [k] - w.world_time) / 2;
         if (size_increase > 6)
										size_increase = 6;
         nsize += size_increase;
         nfillcol = (bl->node_pending_explosion_timestamp [k] - w.world_time) / 4;
//         nfillcol = (bl->node_pending_explosion_timestamp [k] - w.world_time) - (32 - BACK_COL_FADE);
//   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREEN] [SHADE_MAX],
//																				(bx2 + bl->node_x [k]) * view.zoom, (by2 + bl->node_y [k]) * view.zoom,
//																				0, "%i", nfillcol);
         if (nfillcol >= BACK_COL_FADE)
          nfillcol = BACK_COL_FADE - 1;
         if (nfillcol < 0)
										nfillcol = 0;

//										nfillcol = 0;

								}

        nsize += 0.5;



       add_orthogonal_hexagon((bx2 + bl->node_x [k]) * view.zoom, (by2 + bl->node_y [k]) * view.zoom, nsize * view.zoom,
                                      colours.back_fill [node_colour] [node_saturation] [nfillcol]);
//																																						colours.base [COL_TURQUOISE] [SHADE_LOW],
//																																						colours.base [COL_AQUA] [SHADE_MED]);
//   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREEN] [SHADE_MAX],
//																				(bx2 + bl->node_x [k]) * view.zoom, (by2 + bl->node_y [k]) * view.zoom,
																				//0, "%i", nfillcol);
//                                      colours.back_fill [bl->node_team_col [k]] [bl->node_col_saturation [k]] [nfillcol],
//                                      colours.back_fill [bl->node_team_col [k]] [bl->node_col_saturation [k] / 2] [nfillcol]);
       }
				 	}
					 break;

					case BACKBLOCK_DATA_WELL:
						{
       bx2 = (((i * BLOCK_SIZE_PIXELS) - camera_offset_x) + (BLOCK_SIZE_PIXELS/2)) * view.zoom;
       by2 = (((j * BLOCK_SIZE_PIXELS) - camera_offset_y) + (BLOCK_SIZE_PIXELS/2)) * view.zoom;

       float well_size = 20;
       seed_drand(bx+by);

       for (k = 0; k < 6; k++)
							{
								/*
								float left_xpart = cos((PI/3)*k + PI/6 + 0.05) * view.zoom;
								float left_ypart = sin((PI/3)*k + PI/6 + 0.05) * view.zoom;
								float right_xpart = cos((PI/3)*(k+1) + PI/6 - 0.05) * view.zoom;
								float right_ypart = sin((PI/3)*(k+1) + PI/6 - 0.05) * view.zoom;
								vertex_list[0][0] = bx2 + left_xpart * well_size;
								vertex_list[0][1] = by2 + left_ypart * well_size;
								vertex_list[1][0] = bx2 + right_xpart * well_size;
								vertex_list[1][1] = by2 + right_ypart * well_size;
								vertex_list[2][0] = vertex_list[1][0] + cos((PI/3)*(k+1) + PI/6) * 22 * view.zoom;
								vertex_list[2][1] = vertex_list[1][1] + sin((PI/3)*(k+1) + PI/6) * 22 * view.zoom;
								vertex_list[3][0] = bx2 + cos((PI/3)*(k+1) + PI/6 - 0.4) * (well_size + 64) * view.zoom;
								vertex_list[3][1] = by2 + sin((PI/3)*(k+1) + PI/6 - 0.4) * (well_size + 64) * view.zoom;
								vertex_list[4][0] = bx2 + cos((PI/3)*k + PI/6 + 0.4) * (well_size + 64) * view.zoom;
								vertex_list[4][1] = by2 + sin((PI/3)*k + PI/6 + 0.4) * (well_size + 64) * view.zoom;
								vertex_list[5][0] = vertex_list[0][0] + cos((PI/3)*(k) + PI/6) * 22 * view.zoom;
								vertex_list[5][1] = vertex_list[0][1] + sin((PI/3)*(k) + PI/6) * 22 * view.zoom;
*/
float base_dist = drand(30, bx);
well_size = base_dist + 32;

#define BASE_WELL_ANGLE (PI/6)

								float left_xpart = cos(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * view.zoom;
								float left_ypart = sin(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * view.zoom;
								float right_xpart = cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * view.zoom;
								float right_ypart = sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * view.zoom;
								vertex_list[0][0] = bx2 + left_xpart * 32;//* well_size;
								vertex_list[0][1] = by2 + left_ypart * 32;//* well_size;
								vertex_list[1][0] = bx2 + right_xpart * 32; //well_size;
								vertex_list[1][1] = by2 + right_ypart * 32; //well_size;
								vertex_list[2][0] = vertex_list[1][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * view.zoom;
								vertex_list[2][1] = vertex_list[1][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * view.zoom;
								vertex_list[3][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * view.zoom;
								vertex_list[3][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * view.zoom;
								vertex_list[4][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * view.zoom;
								vertex_list[4][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * view.zoom;
								vertex_list[5][0] = vertex_list[0][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * view.zoom;
								vertex_list[5][1] = vertex_list[0][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * view.zoom;

        add_poly_layer(0, 6, colours.base [COL_BLUE] [SHADE_LOW]);//colours.back_fill [0] [0] [0]);


							}

							int transfer_colour_adjust;

       if (w.world_time - w.data_well[bl->backblock_value].last_transferred < 16)
								transfer_colour_adjust = 32 - (w.world_time - w.data_well[bl->backblock_value].last_transferred) * 2;
							  else
										transfer_colour_adjust = 0;

							if (w.data_well[bl->backblock_value].data > 0)
							{
        add_orthogonal_hexagon(bx2, by2, (30 * w.data_well[bl->backblock_value].data * view.zoom) / w.data_well[bl->backblock_value].data_max, al_map_rgba(220 + transfer_colour_adjust, 200, 50, 180 + transfer_colour_adjust));
							}

							float base_well_ring_angle = PI/6 + (w.world_time * w.data_well[bl->backblock_value].spin_rate);
#define WELL_RING_RADIUS (140)
       ALLEGRO_COLOR reserve_colour;
							reserve_colour = al_map_rgba(180 + transfer_colour_adjust,
																																				100 + transfer_colour_adjust,
																																				30 + transfer_colour_adjust,
																																				140 + transfer_colour_adjust);

       for (k = 0; k < DATA_WELL_RESERVES; k++)
							{
								if (w.data_well[bl->backblock_value].reserve_data [k] == 0)
									continue;
								int l;
								float reserve_square_arc = 0.13;
								float reserve_square_length = 36;
								float reserve_data_proportion = w.data_well[bl->backblock_value].reserve_data [k] * 0.002;
								reserve_square_length *= reserve_data_proportion;
								if (reserve_data_proportion < 1)
								 reserve_square_arc *= reserve_data_proportion;
								for (l = 0; l < w.data_well[bl->backblock_value].reserve_squares; l ++)
								{
									float square_angle = base_well_ring_angle + ((PI*2) / w.data_well[bl->backblock_value].reserve_squares) * l;
									add_diamond_layer(0,
																											bx2 + cos(square_angle - reserve_square_arc) * WELL_RING_RADIUS*view.zoom,
																											by2 + sin(square_angle - reserve_square_arc) * WELL_RING_RADIUS*view.zoom,
																											bx2 + cos(square_angle + reserve_square_arc) * WELL_RING_RADIUS*view.zoom,
																											by2 + sin(square_angle + reserve_square_arc) * WELL_RING_RADIUS*view.zoom,
																											bx2 + cos(square_angle + reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*view.zoom,
																											by2 + sin(square_angle + reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*view.zoom,
																											bx2 + cos(square_angle - reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*view.zoom,
																											by2 + sin(square_angle - reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*view.zoom,
																											reserve_colour);
								}
								base_well_ring_angle += PI / w.data_well[bl->backblock_value].reserve_squares;
							}


						}
      break; // end data well drawing code

    	default:
						break;

    }


   }
  }

  draw_vbuf();

  if (reset_clipping == 1)
   al_set_clipping_rectangle(0, 0, panel[PANEL_MAIN].w, panel[PANEL_MAIN].h);

#endif

// int vertex;
// float vx, vy;
// int team_colour;
// float dg_vx;
// float dg_vy;

// int fill_colour;
// ALLEGRO_COLOR method_fill_col;
// ALLEGRO_COLOR join_fill_col;
// ALLEGRO_COLOR method_edge_col;
// ALLEGRO_COLOR base_fill_col;
// ALLEGRO_COLOR link_fill_col;
// ALLEGRO_COLOR main_fill_col;
// ALLEGRO_COLOR main_edge_col;
// ALLEGRO_COLOR under_fill_col;
// ALLEGRO_COLOR under_edge_col;
// ALLEGRO_COLOR method_medium_col;

//	ALLEGRO_COLOR proc_col [PROC_COL_LEVELS] [2];

// al_lock_bitmap(al_get_backbuffer(display), ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);

 struct core_struct* core;

 int immobile_or_mobile;
 int core_index, group_member_index;

 for (immobile_or_mobile = 0; immobile_or_mobile < 2; immobile_or_mobile ++)
	{
		for (core_index = 0; core_index < w.max_cores; core_index ++)
		{
			if (w.core[core_index].exists == 0)
				continue;

// all immobile procs are drawn, then all mobile procs
			if (immobile_or_mobile != w.core[core_index].mobile)
				continue;

			for (group_member_index = 0; group_member_index < w.core[core_index].group_members_max; group_member_index ++)
			{

				if (w.core[core_index].group_member[group_member_index].exists == 0)
					continue;

				p = w.core[core_index].group_member[group_member_index].index;

// for (p = 0; p < w.max_procs; p ++)
// {

//  if (w.proc[p].exists == 0)
//   continue; - shouldn't be necessary, because of group_member[].exists check above

  pr = &w.proc [p];
		core = &w.core[core_index];//[pr->core_index];

//  pr_player = &w.player [pr->player_index];

  x = al_fixtof(pr->position.x - view.camera_x) * view.zoom;
  y = al_fixtof(pr->position.y - view.camera_y) * view.zoom;
  x += view.window_x_unzoomed / 2;
  y += view.window_y_unzoomed / 2;

  if (x < -200 || x > view.window_x_unzoomed + 200
   || y < -200 || y > view.window_y_unzoomed + 200)
    continue;

//   team_colour = pr->player_index;
   shade = PROC_FILL_SHADES - 1;
   if (pr->hp < pr->hp_max)
   {
    shade = (pr->hp * 15) / pr->hp_max;
   }
//   fill_colour = 0;
//   if (pr->hit >= w.world_time - 16)
//    fill_colour = 1;

   int draw_proc = 1;

// if the proc has just been created, we show a special graphic thing:
   if (pr->created_timestamp + 64 > w.world_time)
   {
   	shade = w.world_time - pr->created_timestamp;
   	shade -= templ[pr->player_index][w.core[pr->core_index].template_index].member[pr->group_member_index].downlinks_from_core * 8;

    if (shade < 0)
					continue;

   	float solidity = shade;

   	solidity /= 32;

    if (solidity > 1)
					solidity = 1;

    if (shade > 31)
					shade = 31;

     float float_shade = shade;


   	shade = w.world_time - pr->created_timestamp;

   	shade -= templ[pr->player_index][w.core[pr->core_index].template_index].member[pr->group_member_index].downlinks_from_core * 8;
   	if (shade < 16)
					draw_proc = 0;
   	if (shade > 16)
					shade = 32 - shade;
				if (shade >= 0)
				{

//    float outline_scale = 0.9 + (float_shade / 20) - (float_shade * float_shade / 800);
//    float outline_scale = 0.5 + (float_shade / 20) - (float_shade * float_shade / 700);
    float outline_scale = 0.5 + (float_shade / 10) - (float_shade * float_shade / 400);



			 ALLEGRO_COLOR outline_shade = map_rgba(200 + shade,
																																											50 + shade * 6,
																																											20,// + shade,
																																											1 + shade * 9);


    draw_proc_outline(x, y, pr->angle, pr->shape, outline_scale, 0, outline_shade, outline_shade, view.zoom);


				}

   }
    else
				{

     if (core->construction_complete_timestamp > w.world_time)
			  {

			  	int pulse_time = (w.world_time - pr->created_timestamp) & 31;

      float outline_scale = 1.6 - pulse_time * 0.02;

      shade = (32 - pulse_time);// / 2;
      if (pulse_time < 16)
							shade = pulse_time;// / 2;

						shade /= 2;

/*			   ALLEGRO_COLOR outline_shade = map_rgba(200 + shade,

																																											  50 + shade * 6,
																																											  20,// + shade,
																																											  1 + shade * 9);
*/

      draw_proc_outline(x, y, pr->angle, pr->shape, outline_scale, 0, colours.packet [core->player_index] [shade], colours.packet [core->player_index] [shade], view.zoom);

			  }

				}

				if (pr->repaired_timestamp > w.world_time - 16)
				{

			  	int repair_pulse_time = (w.world_time - pr->repaired_timestamp);

      float outline_scale = 1.6 - repair_pulse_time * 0.04;

//      shade = (32 - repair_pulse_time * 2);// / 2;
//      if (repair_pulse_time < 8)
//							shade = repair_pulse_time * 2;// / 2;

      shade = (16 - repair_pulse_time);// / 2;
      if (repair_pulse_time < 8)
							shade = repair_pulse_time;// / 2;

//						shade /= 2;

/*			   ALLEGRO_COLOR outline_shade = map_rgba(200 + shade,

																																											  50 + shade * 6,
																																											  20,// + shade,
																																											  1 + shade * 9);
*/

      draw_proc_outline(x, y, pr->angle, pr->shape, outline_scale, 0, colours.packet [core->player_index] [shade], colours.packet [core->player_index] [shade], view.zoom);


				}
/*
// these are maximum stress for the level (so stress is LOW if it's 100 or below)
#define STRESS_LEVEL_LOW 100
#define STRESS_LEVEL_MODERATE 400
#define STRESS_LEVEL_HIGH	800*/

//};

int damage_level = (pr->hp * (PROC_DAMAGE_COLS-1)) / pr->hp_max;
//int solidity = 1;
   if (pr->group_member_index == 0) // is core
			{
/*
    int stress_tint = 0;
    if (core->stress > 0)
				{
//					int pulse_time = w.world_time - core->last_execution_timestamp;
     switch(core->stress_level)
     {
     	case STRESS_EXTREME:
 						stress_tint = (core->next_execution_timestamp - w.world_time) * 3;
 						break;
 					case STRESS_HIGH:
				  	if (!(core->cycles_executed & 1))
					   stress_tint = (core->next_execution_timestamp - w.world_time) * 2;
					  break;
 					case STRESS_MODERATE:
		  	  if (!(core->cycles_executed & 3))
				     stress_tint = (core->next_execution_timestamp - w.world_time) * 1.5; // float okay as this is just used for display
				   break;
				  case STRESS_LOW:
	  	   if (!(core->cycles_executed & 7))
		      stress_tint = (core->next_execution_timestamp - w.world_time); // float okay as this is just used for display
							break;

     }

				} // end if core->stress > 0
*/

//     colours.proc_col [pr->player_index] [damage_level] [PROC_COL_CORE_MUTABLE] = map_rgb(colours.base_core_r [pr->player_index] + stress_tint,
//																																																																  colours.base_core_g [pr->player_index] + stress_tint,
//																																																																  colours.base_core_b [pr->player_index] + stress_tint); // map_rgb is bounds-checked wrapper for al_map_rgb

//     colours.proc_col [pr->player_index] [damage_level] [PROC_COL_CORE_MUTABLE] = map_rgb(colours.base_core_r [pr->player_index] + stress_tint + core->stress_level * 40,
//																																																																  colours.base_core_g [pr->player_index] + stress_tint - core->stress_level * 40,
//																																																																  colours.base_core_b [pr->player_index] + stress_tint - core->stress_level * 40); // map_rgb is bounds-checked wrapper for al_map_rgb


     int core_pulse_level = 0;

     if (w.world_time - core->last_execution_timestamp < 16)
					{
						core_pulse_level = (16 - (w.world_time - core->last_execution_timestamp)) * 2;
					}

     colours.proc_col [pr->player_index] [damage_level] [0] [PROC_COL_CORE_MUTABLE] = map_rgb(colours.base_core_r [pr->player_index] + core_pulse_level,
																																																																  colours.base_core_g [pr->player_index] + core_pulse_level,
																																																																  colours.base_core_b [pr->player_index] + core_pulse_level); // map_rgb is bounds-checked wrapper for al_map_rgb

colours.proc_col [pr->player_index] [damage_level] [1] [PROC_COL_CORE_MUTABLE] = colours.proc_col [pr->player_index] [damage_level] [0] [PROC_COL_CORE_MUTABLE]; // fix this!

					if (core->construction_complete_timestamp > w.world_time)
					{


				   int time_until_construction_ends = core->construction_complete_timestamp - w.world_time;

				   float bcon_size = (time_until_construction_ends * 0.02);
				   if (bcon_size > 32)
					   bcon_size = 32;

				   shade = time_until_construction_ends / 4;
				   if (shade > 30)
					   shade = 30;

				   bcon_size += 13;

//				   float core_angle_float = fixed_to_radians(pr->angle);

//				   float bcool_centre_x = x;// * zoom;
//				   float bcool_centre_y = y;// * zoom;

       float layer_rot_speed;

				   {

        layer_rot_speed = 0.07;//(5 + drand(10, 1)) * 0.004;

        if (!(core->index & 1))
					    layer_rot_speed = 0 - layer_rot_speed; // could do better than this

					   float layer_dist_2 = (bcon_size * view.zoom);// / 2;
					   float layer_separation = 1 + (time_until_construction_ends * 0.0002);

					   int pulse_size = (time_until_construction_ends + 16) % 32;

					   float width = 1;//(32 - pulse_size) * 0.3;

					   if (pulse_size < 16)
									width += (16 - pulse_size) * 0.4;

					   if (width > 3)
									width = 3;

					   shade = pulse_size;// * 2;//;

					   if (shade >= 16)
									shade = 31 - pulse_size;

								shade *= 2;

//					   pulse_size = 16 - pulse_size;

//        if (pulse_size < 8)
//									pulse_size = 8;

//					   float inner_dist = layer_dist_2 + 8 * view.zoom;// + pulse_size * 1 * view.zoom;
//					   float outer_dist = layer_dist_2 + 14 * view.zoom;// + pulse_size * 2 * view.zoom;
					   float inner_dist = layer_dist_2;// + pulse_size * 1 * view.zoom;
 				   float outer_dist = layer_dist_2 + 5 * view.zoom;// + pulse_size * 2 * view.zoom;


width = 6;

				    for (i = 0; i < 8; i ++)
				    {
					    float bit_angle = fixed_to_radians(pr->angle) + ((PI*2*i)/8) + time_until_construction_ends * layer_rot_speed;
         float layer_dist = layer_dist_2;
         if (i & 1)
     	    layer_dist *= layer_separation;

					    float side_dist_x = cos(bit_angle + PI/2) * width * view.zoom;
					    float side_dist_y = sin(bit_angle + PI/2) * width * view.zoom;

					    add_diamond_layer(4,
																							    x + cos(bit_angle) * inner_dist + side_dist_x,
																							    y + sin(bit_angle) * inner_dist + side_dist_y,
																							    x + cos(bit_angle) * outer_dist + side_dist_x,
																							    y + sin(bit_angle) * outer_dist + side_dist_y,
																							    x + cos(bit_angle) * outer_dist - side_dist_x,
																							    y + sin(bit_angle) * outer_dist - side_dist_y,
																							    x + cos(bit_angle) * inner_dist - side_dist_x,
																							    y + sin(bit_angle) * inner_dist - side_dist_y,
																							    colours.packet [core->player_index] [shade]);


/*
					    add_diamond_layer(4,
																							    x + cos(bit_angle - width) * inner_dist,
																							    y + sin(bit_angle - width) * inner_dist,
																							    x + cos(bit_angle - width) * outer_dist,
																							    y + sin(bit_angle - width) * outer_dist,
																							    x + cos(bit_angle + width) * outer_dist,
																							    y + sin(bit_angle + width) * outer_dist,
																							    x + cos(bit_angle + width) * inner_dist,
																							    y + sin(bit_angle + width) * inner_dist,
																							    colours.packet [core->player_index] [shade]);
*/
/*
					    add_diamond_layer(4,
																							    x + cos(bit_angle - width) * layer_dist,
																							    y + sin(bit_angle - width) * layer_dist,
																							    x + cos(bit_angle - width) * (layer_dist + (9 * view.zoom)),
																							    y + sin(bit_angle - width) * (layer_dist + (9 * view.zoom)),
																							    x + cos(bit_angle + width) * (layer_dist + (9 * view.zoom)),
																							    y + sin(bit_angle + width) * (layer_dist + (9 * view.zoom)),
																							    x + cos(bit_angle + width) * layer_dist,
																							    y + sin(bit_angle + width) * layer_dist,
																							    colours.packet [core->player_index] [shade]);
*/
				    }
				   }

   			}


//					}

			}

/*
					int hit_pulse_level = 0;

     if (w.world_time - pr->hit_pulse_time < 16)
					{
						hit_pulse_level = 48;//(16 - (w.world_time - pr->hit_pulse_time)) * 8;
					}

     colours.proc_col [pr->player_index] [damage_level] [PROC_COL_MAIN_1] = map_rgb(colours.base_proc_main_r [pr->player_index] [damage_level] + hit_pulse_level,
																																																																  colours.base_proc_main_g [pr->player_index] [damage_level] + hit_pulse_level,
																																																																  colours.base_proc_main_b [pr->player_index] [damage_level] + hit_pulse_level); // map_rgb is bounds-checked wrapper for al_map_rgb
*/

//   colours.proc_col [pr->player_index] [damage_level] [PROC_COL_MAIN_1] = colours.proc_col_main_hit_pulse [pr->player_index] [damage_level] [(w.world_time - pr->hit_pulse_time < 8)];

 int hit_pulse = (w.world_time - pr->hit_pulse_time < 4);

// This is the main proc drawing call:
  if (draw_proc)
   draw_proc_shape(x, y, pr->angle, pr->shape, pr->player_index, view.zoom, colours.proc_col [pr->player_index] [damage_level] [hit_pulse]);
//   base_fill_col = proc_col [5] [0];
//   under_fill_col = proc_col [7] [0];
//   main_edge_col = proc_col [2] [1];

//   draw_proc_outline(x, y, pr->angle, pr->shape,
//                           colours.base_trans [COL_BLUE] [SHADE_MED] [TRANS_FAINT], colours.base_trans [COL_BLUE] [SHADE_LOW] [0], view.zoom);

  if (draw_proc)
   for (i = 0; i < MAX_LINKS; i ++)
			{
				switch(pr->object[i].type)
				{
				 case OBJECT_TYPE_UPLINK: // note downlink isn't drawn
			   draw_link_shape(x, y,
																			   pr->angle,
																	     pr->shape,
																	     pr->connection_link [0],
																			   (al_fixtof(pr->group_connection_ptr [0]->position.x - view.camera_x) * view.zoom) + (view.window_x_unzoomed / 2),
																			   (al_fixtof(pr->group_connection_ptr [0]->position.y - view.camera_y) * view.zoom) + (view.window_y_unzoomed / 2),
																			   pr->group_connection_ptr [0]->angle,
																			   pr->group_connection_ptr [0]->shape,
																	     pr->connected_from_link [0],
																	     colours.proc_col [pr->player_index] [damage_level] [0] [PROC_COL_LINK], // [hit_pulse],
//																			   proc_col [1] [0],
//																			   proc_col [1] [1],
																			   view.zoom);
						break;
					case OBJECT_TYPE_NONE:
						break;
					default:
      draw_object(x, y,
																					pr->angle,
																					pr->shape,
																					&pr->object[i],
																					&pr->object_instance[i],
																					core,
																					pr,
																					i,
																					colours.proc_col [pr->player_index] [damage_level] [0], //[hit_pulse],
//																			  base_fill_col,
//																			  under_fill_col,
//																			  main_edge_col,
																			  view.zoom);

						break;


				}
//				add_line(3, x, y,
//													x + (cos(fixed_to_radians(pr->angle) + fixed_to_radians(pr->nshape_ptr->link_angle_fixed [i])) * al_fixtof(pr->nshape_ptr->link_dist_fixed [i])) * view.zoom,
//													y + (sin(fixed_to_radians(pr->angle) + fixed_to_radians(pr->nshape_ptr->link_angle_fixed [i])) * al_fixtof(pr->nshape_ptr->link_dist_fixed [i])) * view.zoom,
//													colours.base [COL_YELLOW] [SHADE_MAX]);


			}

//  if (pr->interface_object_present)
  if (pr->interface_protects)
		{
			if (core->interface_active)
//				&& pr->interface_on_process_set_on) // this is here (rather than as part of the if (pr->interface_object_present) test because of the else below
			{

#ifdef SANITY_CHECK
    if (core->interface_strength_max == 0)
				{
					fpr("\nError: i_display.c: interface_strength_max is 0");
					error_call();
				}
#endif

    float charge_shade = (float) core->interface_strength / (float) core->interface_strength_max;

    float hit_shade = 0;
				if (pr->interface_hit_time > w.world_time - 16)
				 hit_shade = (pr->interface_hit_time + 16 - w.world_time) * 0.0625; // *0.0625 is /16

				float interface_opacity = 0.2 + core->interface_strength * 0.001;
				if (interface_opacity > 1)
					interface_opacity = 1;

				if (pr->interface_stability)
						interface_opacity *= 5;

				float interface_size_proportion = 1.3;

				if (pr->interface_raised_time > w.world_time - 64)
				{
					float raised_modifier = (pr->interface_raised_time + 64 - w.world_time) * 0.0156;//312;
					if (raised_modifier > hit_shade)
						hit_shade = raised_modifier;
					interface_size_proportion -= (pr->interface_raised_time + 64 - w.world_time) * 0.0025;
				}

				if (core->interface_charged_time > w.world_time - 8)
				{
					hit_shade += (float) (8 + core->interface_charged_time - w.world_time) * 0.01;
					if (hit_shade > 1)
						hit_shade = 1;
				}


// interface colour is based on absolute interface strength rather than a proportional value to make the colour more informative
//  * think about this - would it be better to use absolute strength but spread over the components with active interfaces?
				ALLEGRO_COLOR interface_colour = map_rgba(w.player[pr->player_index].interface_colour_base [0] + (w.player[pr->player_index].interface_colour_hit [0] * hit_shade) + (w.player[pr->player_index].interface_colour_charge [0] * charge_shade),
																																																	w.player[pr->player_index].interface_colour_base [1] + (w.player[pr->player_index].interface_colour_hit [1] * hit_shade) + (w.player[pr->player_index].interface_colour_charge [1] * charge_shade),
																																																	w.player[pr->player_index].interface_colour_base [2] + (w.player[pr->player_index].interface_colour_hit [2] * hit_shade) + (w.player[pr->player_index].interface_colour_charge [2] * charge_shade),
																																																	16 + interface_opacity * 16 + hit_shade * 128);
				ALLEGRO_COLOR interface_edge_colour = map_rgba(
																																																	w.player[pr->player_index].interface_colour_base [0] + (w.player[pr->player_index].interface_colour_hit [0] * hit_shade) + (w.player[pr->player_index].interface_colour_charge [0] * charge_shade),
																																																	w.player[pr->player_index].interface_colour_base [1] + (w.player[pr->player_index].interface_colour_hit [1] * hit_shade) + (w.player[pr->player_index].interface_colour_charge [1] * charge_shade),
																																																	w.player[pr->player_index].interface_colour_base [2] + (w.player[pr->player_index].interface_colour_hit [2] * hit_shade) + (w.player[pr->player_index].interface_colour_charge [2] * charge_shade),
																																																	25 + interface_opacity * 40);

// if the 1.3 scaling factor is changed, may also need to change interface collision mask generation code in init_nshape_collision_masks() in g_shape.c (see the * 13 / 10 bit)
				draw_proc_outline(x, y, pr->angle, pr->shape, interface_size_proportion, 0, interface_colour, interface_edge_colour, view.zoom);

			}
			 else
				{
					if (pr->interface_lowered_time > w.world_time - 16) // this means the core's interface is unbroken (and may be active) but this proc's interface has been specifically lowered
					{

      float hit_shade = (pr->interface_lowered_time + 16 - w.world_time) * 0.0625; // *0.0625 is /16

				  float interface_size_proportion = 1.3 - (w.world_time - pr->interface_lowered_time) * 0.01;

// interface colour is based on absolute interface strength rather than a proportional value to make the colour more informative
//  * think about this - would it be better to use absolute strength but spread over the components with active interfaces?
				  ALLEGRO_COLOR interface_colour = map_rgba(w.player[pr->player_index].interface_colour_base [0] + (w.player[pr->player_index].interface_colour_hit [0] * hit_shade),
																																																	  w.player[pr->player_index].interface_colour_base [1] + (w.player[pr->player_index].interface_colour_hit [1] * hit_shade),
																																																	  w.player[pr->player_index].interface_colour_base [2] + (w.player[pr->player_index].interface_colour_hit [2] * hit_shade),
																																																	  5 + hit_shade * 128);
				  ALLEGRO_COLOR interface_edge_colour = map_rgba(w.player[pr->player_index].interface_colour_base [0] + (w.player[pr->player_index].interface_colour_hit [0] * hit_shade),
																																															  		w.player[pr->player_index].interface_colour_base [1] + (w.player[pr->player_index].interface_colour_hit [1] * hit_shade),
																																																	  w.player[pr->player_index].interface_colour_base [2] + (w.player[pr->player_index].interface_colour_hit [2] * hit_shade),
																																																	  25);

// if the 1.3 scaling factor is changed, may also need to change interface collision mask generation code in init_nshape_collision_masks() in g_shape.c (see the * 13 / 10 bit)
				  draw_proc_outline(x, y, pr->angle, pr->shape, interface_size_proportion, 0, interface_colour, interface_edge_colour, view.zoom);


					}
				}
		}


  if (pr->group_member_index == 0)
		{
			if (w.core[pr->core_index].bubble_text_time >= w.world_time - BUBBLE_TOTAL_TIME)
			{
  			w.core[pr->core_index].bubble_list = bubble_list_index;
					bubble_list_index = pr->core_index;
		 		w.core[pr->core_index].bubble_x = x;
				 w.core[pr->core_index].bubble_y = y - 120 * view.zoom;//;// - scaleUI_y(FONT_SQUARE,120) * view.zoom;
			}
			if (w.core[pr->core_index].selected != -1)
			{
				int time_since_selection = game.total_time - w.core[pr->core_index].select_time;
				if (time_since_selection > 12)
					time_since_selection = 12;

    select_arrows(5, x, y,
																		game.total_time * 0.02, // angle
																		(102.5 - time_since_selection * 3) * view.zoom, // radius
																		5, //float out_dist,
																		PI/2+PI/8, //float side_angle,
																		9, //float side_dist,
																		colours.base [COL_GREY] [SHADE_MAX]);// [TRANS_THICK]); //ALLEGRO_COLOR arrow_col)

			}
		}





//		if (pr->group_member_index != 0
			if (command.selected_core [0] == pr->core_index
			 && command.selected_member == pr->group_member_index
			 && w.core[pr->core_index].group_members_current > 1) // don't display this for single-member groups
		{

				int time_since_selection = game.total_time - w.core[pr->core_index].select_time;
				if (time_since_selection > 12)
					time_since_selection = 12;

    select_arrows(5, x, y,
																		game.total_time * 0.02,// + (PI/5), // angle
																		(80.5 - time_since_selection * 2) * view.zoom, // radius
																		4, //float out_dist,
																		PI/2+PI/8, //float side_angle,
																		8, //float side_dist,
																		colours.base [COL_GREY] [SHADE_MAX]);// [TRANS_THICK]); //ALLEGRO_COLOR arrow_col)
		}

			} // end of group_member loop

  check_vbuf();

 } // end of core draw loop

 draw_vbuf();

 } // end of immobile_or_mobile loop



// if (view.mouse_on_build_queue_button_timestamp == game.total_time)
//	{
/*		if (command.select_mode == SELECT_MODE_SINGLE_CORE
			&& w.core[command.selected_core [0]].build_command_queue [view.mouse_on_build_queue_button].active == 1)
		{
	  draw_notional_group(&templ[game.user_player_index][w.core[command.selected_core[0]].build_command_queue[view.mouse_on_build_queue_button].build_template],
																								al_itofix(w.core[command.selected_core[0]].build_command_queue[view.mouse_on_build_queue_button].build_x),
																								al_itofix(w.core[command.selected_core[0]].build_command_queue[view.mouse_on_build_queue_button].build_y),
																								int_angle_to_fixed(w.core[command.selected_core[0]].build_command_queue[view.mouse_on_build_queue_button].build_angle),
																								view.zoom);
		}*/
//	}
//	 else


  if (command.display_build_buttons)
		{
			i = 0;
			int notional_group_draw_col;
			while(w.player[game.user_player_index].build_queue[i].active
						&& i <	BUILD_QUEUE_LENGTH)
			{
//				sancheck(i, 0, BUILD_QUEUE_LENGTH, "command.display_build_buttons: i outside BUILD_QUEUE_LENGTH");
				sancheck(w.player[game.user_player_index].build_queue[i].template_index, 0, TEMPLATES_PER_PLAYER, "command.display_build_buttons template index");
				if (!templ[game.user_player_index][w.player[game.user_player_index].build_queue[i].template_index].active)
				{
					i ++;
					continue;
				}

		  if (view.mouse_on_build_queue_button == i
					&& view.mouse_on_build_queue_button_timestamp == game.total_time)
					notional_group_draw_col = PLAN_COL_BUILD_QUEUE_HIGHLIGHT;
						else
   				notional_group_draw_col = PLAN_COL_BUILD_QUEUE;

    draw_notional_group(&templ[game.user_player_index][w.player[game.user_player_index].build_queue[i].template_index],
																								al_itofix(w.player[game.user_player_index].build_queue[i].build_x),
																								al_itofix(w.player[game.user_player_index].build_queue[i].build_y),
																								int_angle_to_fixed(w.player[game.user_player_index].build_queue[i].angle),
																								view.zoom,
																								notional_group_draw_col,
																								notional_group_draw_col); // errors aren't worked out for queued builds anyway



				i++;

			}


		}

		{


 if (command.build_mode != BUILD_MODE_NONE)
	{
		  sancheck(command.build_template_index, 0, TEMPLATES_PER_PLAYER, "command.build_mode build_template_index");
	   if (templ[game.user_player_index][command.build_template_index].active)
	   {


      draw_notional_group(&templ[game.user_player_index][command.build_template_index], command.build_position.x, command.build_position.y, command.build_angle, view.zoom, PLAN_COL_BUILD_OKAY, PLAN_COL_BUILD_ERROR);

      draw_data_well_exclusion_zones();

//  draw_notional_group() checks for visibility

	   }
	  }
		}
/*

Maybe do this later (it's quite hard)

draw_link_shape(x, y,
																			   command.build_angle + templ[game.user_player_index][command.build_template_index].member [i].group_angle_offset,
																	     templ[game.user_player_index][command.build_template_index].member [i].shape,
																	     j,
																			   (al_fixtof(pr->group_connection [0]->position.x - view.camera_x) * view.zoom) + (view.window_x_unzoomed / 2),
																			   (al_fixtof(pr->group_connection [0]->position.y - view.camera_y) * view.zoom) + (view.window_y_unzoomed / 2),
																			   pr->group_connection [0]->angle,
																			   pr->group_connection [0]->shape,
																	     pr->connected_from_link [0],
																			   proc_col [1] [0],
																			   proc_col [1] [1],
																			   view.zoom);*/

 draw_vbuf();
// draw_fans();


 struct packet_struct* pack;
 int pk;

 for (pk = 0; pk < w.max_packets; pk ++)
 {
// TO DO: consider optimising this to use blocks instead of searching the entire packet array
  if (w.packet[pk].exists == 0)
   continue;

  pack = &w.packet [pk];

//  x = al_fixtof(pack->x - view.camera_x) + (view.window_x/2);
//  y = al_fixtof(pack->y - view.camera_y) + (view.window_y/2);

  x = al_fixtof(pack->position.x - view.camera_x) * view.zoom;
  y = al_fixtof(pack->position.y - view.camera_y) * view.zoom;
  x += view.window_x_unzoomed / 2;
  y += view.window_y_unzoomed / 2;

  if (x < -200 || x > view.window_x_unzoomed + 200
   || y < -200 || y > view.window_y_unzoomed + 200)
    continue;

//  float front_dist;
//  float side_dist;
//  float back_dist;
  float packet_angle = fixed_to_radians(pack->angle);
//  float size_factor;
  int packet_time;

// If there are a lot of packets on screen, can easily overflow the vertex/triangle buffers. So check:
     if (vbuf.index_pos_triangle [2] >= VERTEX_INDEX_TRIGGER)
						draw_vbuf();

  switch (pack->type)
  {
/*
		 case PACKET_TYPE_BURST:
//  	case PACKET_TYPE_BURST_DIR:
			 {
     packet_time = w.world_time - pack->created_timestamp;

//  			float sharpness = 4 + (w.world_time - pack->created_timestamp) * 0.1;
//  			if (sharpness > 10)
//						sharpness = 10;
     seed_drand(pk - (packet_time/3));
     float blob_size = (drand(10, 1)) * 0.2;


			 	  		add_diamond_layer(3,
																														x + cos(packet_angle) * (10+blob_size) * view.zoom,
																														y + sin(packet_angle) * (10+blob_size) * view.zoom,
																														x + cos(packet_angle+PI/2) * (6+blob_size) * view.zoom,
																														y + sin(packet_angle+PI/2) * (6+blob_size) * view.zoom,
																														x + cos(packet_angle+PI) * (12+blob_size) * view.zoom,
																														y + sin(packet_angle+PI) * (12+blob_size) * view.zoom,
																														x + cos(packet_angle-PI/2) * (6+blob_size) * view.zoom,
																														y + sin(packet_angle-PI/2) * (6+blob_size) * view.zoom,
																														colours.packet [pack->colour] [21]);

			 	  		add_diamond_layer(3,
																														x + cos(packet_angle) * (6+blob_size) * view.zoom,
																														y + sin(packet_angle) * (6+blob_size) * view.zoom,
																														x + cos(packet_angle+PI/2) * (4+blob_size) * view.zoom,
																														y + sin(packet_angle+PI/2) * (4+blob_size) * view.zoom,
																														x + cos(packet_angle+PI) * (8+blob_size) * view.zoom,
																														y + sin(packet_angle+PI) * (8+blob_size) * view.zoom,
																														x + cos(packet_angle-PI/2) * (4+blob_size) * view.zoom,
																														y + sin(packet_angle-PI/2) * (4+blob_size) * view.zoom,
																														colours.packet [pack->colour] [31]);


     blob_size = 10 + drand(5, 1);


   		start_ribbon(2,
																		x + cos(packet_angle) * (blob_size+3) * view.zoom,
																		y + sin(packet_angle) * (blob_size+3) * view.zoom,
																		x + cos(packet_angle + PI/4) * blob_size * view.zoom,
																		y + sin(packet_angle + PI/4) * blob_size * view.zoom,
																		colours.packet [pack->colour] [16]);
					add_ribbon_vertex(x + cos(packet_angle - PI/4) * blob_size * view.zoom,
																		     y + sin(packet_angle - PI/4) * blob_size * view.zoom,
																		     colours.packet [pack->colour] [16]);
					add_ribbon_vertex(x + cos(packet_angle + PI/2) * blob_size * view.zoom,
																		     y + sin(packet_angle + PI/2) * blob_size * view.zoom,
																		     colours.packet [pack->colour] [16]);
					add_ribbon_vertex(x + cos(packet_angle - PI/2) * blob_size * view.zoom,
																		     y + sin(packet_angle - PI/2) * blob_size * view.zoom,
																		     colours.packet [pack->colour] [16]);
					add_ribbon_vertex(x + cos(packet_angle + PI*0.7) * blob_size * view.zoom,
																		     y + sin(packet_angle + PI*0.7) * blob_size * view.zoom,
																		     colours.packet [pack->colour] [16]);
					add_ribbon_vertex(x + cos(packet_angle - PI*0.7) * blob_size * view.zoom,
																		     y + sin(packet_angle - PI*0.7) * blob_size * view.zoom,
																		     colours.packet [pack->colour] [16]);

     float x_step = al_fixtof(0 - pack->speed.x) * view.zoom;
     float y_step = al_fixtof(0 - pack->speed.y) * view.zoom;

     int end_time = packet_time / 3;
     if (end_time > 16)
						end_time = 16;

						x += x_step * (packet_time % 3);
						y += y_step * (packet_time % 3);

//     draw_pulse_tail(x, y, x_step, y_step, packet_angle, packet_time, 0, end_time, 8, pack->colour, pk - (packet_time/3));

			 }
			 continue;
*/
/*
		 case PACKET_TYPE_SURGE:
  		{
//  			float sharpness = 4 + (w.world_time - pack->created_timestamp) * 0.16;
//  			if (sharpness > 10)

				packet_angle = atan2(al_fixtof(pack->speed.y), al_fixtof(pack->speed.x)); // looks better if the movement direction rather than the acceleration direction is displayed

  		add_diamond_layer(3,
																														x + cos(packet_angle) * 16 * view.zoom,
																														y + sin(packet_angle) * 16 * view.zoom,
																														x + cos(packet_angle+PI/2) * 11 * view.zoom,
																														y + sin(packet_angle+PI/2) * 11 * view.zoom,
																														x + cos(packet_angle+PI) * 6 * view.zoom,
																														y + sin(packet_angle+PI) * 6 * view.zoom,
																														x + cos(packet_angle-PI/2) * 11 * view.zoom,
																														y + sin(packet_angle-PI/2) * 11 * view.zoom,
																														colours.packet [pack->colour] [20]);
  		add_diamond_layer(3,
																														x + cos(packet_angle) * 12 * view.zoom,
																														y + sin(packet_angle) * 12 * view.zoom,
																														x + cos(packet_angle+PI/2) * 7 * view.zoom,
																														y + sin(packet_angle+PI/2) * 7 * view.zoom,
																														x + cos(packet_angle+PI) * 4 * view.zoom,
																														y + sin(packet_angle+PI) * 4 * view.zoom,
																														x + cos(packet_angle-PI/2) * 7 * view.zoom,
																														y + sin(packet_angle-PI/2) * 7 * view.zoom,
																														colours.packet [pack->colour] [31]);

     bloom_circle(1, x, y, colours.bloom_centre [pack->colour] [20], colours.bloom_edge [pack->colour] [0], 42 * view.zoom);
  		}
			 continue;
*/

		 case PACKET_TYPE_SPIKE1:
  	case PACKET_TYPE_SPIKE2:
  	case PACKET_TYPE_SPIKE3:
//  	case PACKET_TYPE_SPIKE4:
  		{
  			float sharpness = 4 + (w.world_time - pack->created_timestamp) * 0.1;
  			if (sharpness > 8)
						sharpness = 8;
					float damage_extra_size = 0;
					shade = 20;
					if (pack->damage > SPIKE_BASE_DAMAGE)
					{
							damage_extra_size = (pack->damage - SPIKE_BASE_DAMAGE) * 0.03;
							shade += (pack->damage - SPIKE_BASE_DAMAGE) * 0.2;//0.073;
					}
					sharpness +=	damage_extra_size * 5;

				packet_angle = atan2(al_fixtof(pack->speed.y), al_fixtof(pack->speed.x)); // looks better if the movement direction rather than the acceleration direction is displayed

  		add_diamond_layer(3,
																														x + cos(packet_angle) * sharpness * 1.3 * view.zoom,
																														y + sin(packet_angle) * sharpness * 1.3 * view.zoom,
																														x + cos(packet_angle+PI/2) * (6+damage_extra_size) * view.zoom,
																														y + sin(packet_angle+PI/2) * (6+damage_extra_size) * view.zoom,
																														x + cos(packet_angle+PI) * sharpness * 1.7 * view.zoom,
																														y + sin(packet_angle+PI) * sharpness * 1.7 * view.zoom,
																														x + cos(packet_angle-PI/2) * (6+damage_extra_size) * view.zoom,
																														y + sin(packet_angle-PI/2) * (6+damage_extra_size) * view.zoom,
																														colours.packet [pack->colour] [shade / 2]);
  		add_diamond_layer(3,
																														x + cos(packet_angle) * sharpness * view.zoom,
																														y + sin(packet_angle) * sharpness * view.zoom,
																														x + cos(packet_angle+PI/2) * (4+damage_extra_size) * view.zoom,
																														y + sin(packet_angle+PI/2) * (4+damage_extra_size) * view.zoom,
																														x + cos(packet_angle+PI) * sharpness * view.zoom,
																														y + sin(packet_angle+PI) * sharpness * view.zoom,
																														x + cos(packet_angle-PI/2) * (4+damage_extra_size) * view.zoom,
																														y + sin(packet_angle-PI/2) * (4+damage_extra_size) * view.zoom,
																														colours.packet [pack->colour] [shade]);

/*
  		add_outline_diamond_layer(3,
																														x + cos(packet_angle) * sharpness * view.zoom,
																														y + sin(packet_angle) * sharpness * view.zoom,
																														x + cos(packet_angle+PI/2) * 4 * view.zoom,
																														y + sin(packet_angle+PI/2) * 4 * view.zoom,
																														x + cos(packet_angle+PI) * sharpness * view.zoom,
																														y + sin(packet_angle+PI) * sharpness * view.zoom,
																														x + cos(packet_angle-PI/2) * 4 * view.zoom,
																														y + sin(packet_angle-PI/2) * 4 * view.zoom,
																														colours.packet [pack->colour] [30],
																														colours.packet [pack->colour] [22]);
																														*/
     bloom_circle(1, x, y, colours.bloom_centre [pack->colour] [20], colours.bloom_edge [pack->colour] [0], 32 * view.zoom);

//al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], x + 30, y, ALLEGRO_ALIGN_CENTRE, "%i:%i", w.world_time - pack->created_timestamp, pack->damage);

  		}
			 continue;
/*
			case PACKET_TYPE_SURGE:

// if (pack->status > 0)
			 {
     packet_time = w.world_time - pack->created_timestamp;

     int pulse_or_burst = (pack->type == PACKET_TYPE_BURST);

     float x_step = al_fixtof(0 - pack->speed.x) * view.zoom;
     float y_step = al_fixtof(0 - pack->speed.y) * view.zoom;

     int end_time = packet_time;
     int max_time = 6 + pack->status * 4;
     if (end_time > max_time)
						end_time = max_time;

					float tail_width = 0.25;//(pack->status + 4) * 0.04;

     draw_pulse_tail(x, y, x_step, y_step,
																					packet_angle,
																					packet_time,
																					0, // start_time
																					end_time,
																					max_time,
																					tail_width,
																					pack->colour,
																					12 + pulse_or_burst * 8, // shade
																					pk - packet_time,
																					pack->status,
																					1.3, // blob_scale
																					pulse_or_burst);

       bloom_long(1, x + (x_step * pack->status) / 3, y + (y_step * pack->status) / 3, packet_angle, end_time * hypot(y_step, x_step),
//       bloom_long(1, x, y, packet_angle, end_time * hypot(y_step, x_step),
																		colours.bloom_centre [pack->player_index] [20],
																		colours.bloom_edge [pack->player_index] [10],
																		colours.bloom_edge [pack->player_index] [1],
																		(20 + pack->status * 12 + drand(10, 1)) * view.zoom, (10 + pack->status * 7 + drand(5, 1)) * view.zoom);
// remember packet trail also drawn in packet explosion cloud code (search for PACKET_MISS)

     end_time = packet_time;
     max_time = 3 + pack->status * 3;
     if (end_time > max_time)
						end_time = max_time;
					tail_width = 0.15;//(pack->status + 4) * 0.03;

     draw_pulse_tail(x, y, x_step, y_step,
																					packet_angle,
																					packet_time,
																					0, // start_time
																					end_time,
																					max_time,
																					tail_width,
																					pack->colour,
																					28 + pulse_or_burst, // shade
																					pk - packet_time,
																					pack->status,
																					0.6, // blob_scale
																					pulse_or_burst);


//     bloom_circle(1, x, y, colours.base [COL_AQUA] [SHADE_MED], colours.base [COL_BLUE] [SHADE_MED], 55 * view.zoom);
//     bloom_long(1, x, y, colours.base [COL_AQUA] [SHADE_MED], colours.base [COL_BLUE] [SHADE_MED], 25 * view.zoom, packet_angle+PI/2, 1.9, 0.6);

			 }
			 break;
*/

			case PACKET_TYPE_ULTRA:
// if (pack->status > 0)
			 {
     packet_time = w.world_time - pack->created_timestamp;

     float x_step = al_fixtof(0 - pack->speed.x) * view.zoom;
     float y_step = al_fixtof(0 - pack->speed.y) * view.zoom;

     int end_time = packet_time;
     int max_time = 29;
     if (end_time > max_time)
						end_time = max_time;

					float tail_width = 0.25;//(pack->status + 4) * 0.04;

     draw_pulse_tail(x, y, x_step, y_step,
																					packet_angle,
																					packet_time,
																					0, // start_time
																					end_time,
																					max_time,
																					tail_width,
																					pack->colour,
																					pack->status - 6, // shade
																					pk - packet_time,
																					3,
																					1.4, // blob_scale
																					0);

       bloom_long(1, x + (x_step * 3) / 3, y + (y_step * 3) / 3, packet_angle, end_time * hypot(y_step, x_step),
//       bloom_long(1, x, y, packet_angle, end_time * hypot(y_step, x_step),
																		colours.bloom_centre [pack->player_index] [20],
																		colours.bloom_edge [pack->player_index] [10],
																		colours.bloom_edge [pack->player_index] [1],
																		(56 + drand(10, 1)) * view.zoom, (31 + drand(5, 1)) * view.zoom);
// remember packet trail also drawn in packet explosion cloud code (search for PACKET_MISS)

     end_time = packet_time;
     max_time = 24;
     if (end_time > max_time)
						end_time = max_time;
					tail_width = 0.15;//(pack->status + 4) * 0.03;

     draw_pulse_tail(x, y, x_step, y_step,
																					packet_angle,
																					packet_time,
																					0, // start_time
																					end_time,
																					max_time,
																					tail_width,
																					pack->colour,
																					pack->status, // shade
																					pk - packet_time,
																					3,
																					0.8, // blob_scale
																					0);


//     bloom_circle(1, x, y, colours.base [COL_AQUA] [SHADE_MED], colours.base [COL_BLUE] [SHADE_MED], 55 * view.zoom);
//     bloom_long(1, x, y, colours.base [COL_AQUA] [SHADE_MED], colours.base [COL_BLUE] [SHADE_MED], 25 * view.zoom, packet_angle+PI/2, 1.9, 0.6);

			 }
			 break;


			case PACKET_TYPE_PULSE:
			case PACKET_TYPE_BURST:
			 {
     packet_time = w.world_time - pack->created_timestamp;

//     int pulse_or_burst = (pack->type == PACKET_TYPE_BURST);

     float x_step = al_fixtof(0 - pack->speed.x) * view.zoom;
     float y_step = al_fixtof(0 - pack->speed.y) * view.zoom;

//     int end_time = packet_time;
//     int max_time = 6 + pack->status * 4;
//     if (end_time > max_time)
//						end_time = max_time;

//					float tail_width = 0.25;//(pack->status + 4) * 0.04;
/*
     draw_new_pulse_tail(x, y, x_step, y_step,
																					packet_angle,
																					packet_time,
																					0, // start_time
																					end_time,
																					max_time,
																					tail_width,
																					pack->colour,
																					12 + pulse_or_burst * 8, // shade
																					pk - packet_time,
																					pack->status,
																					1.3, // blob_scale
																					pulse_or_burst);
*/

/*
     int end_time = packet_time;
     int max_time = 24 + pack->status * 8;
     if (end_time > max_time)
						end_time = max_time;*/
//					tail_width = 0.15;//(pack->status + 4) * 0.03;
/*
       bloom_long(1, x + (x_step * pack->status) / 3, y + (y_step * pack->status) / 3, packet_angle, end_time * hypot(y_step, x_step),
//       bloom_long(1, x, y, packet_angle, end_time * hypot(y_step, x_step),
//																		colours.bloom_centre [pack->player_index] [20],
//																		colours.bloom_edge [pack->player_index] [10],
//																		colours.bloom_edge [pack->player_index] [1],
																		colours.packet [pack->player_index] [20],
																		colours.packet [pack->player_index] [0],
																		colours.packet [pack->player_index] [0],
																		(20 + pack->status * 12 + drand(10, 1)) * view.zoom, (10 + pack->status * 7 + drand(5, 1)) * view.zoom);
*/
// remember packet trail also drawn in packet explosion cloud code (search for PACKET_MISS)

     bloom_circle(1, x, y, colours.bloom_centre [pack->colour] [20], colours.bloom_edge [pack->colour] [0], (32 + pack->status * 12) * view.zoom);
/*
       bloom_long(1, x + (x_step * pack->status) / 3, y + (y_step * pack->status) / 3, packet_angle, end_time * hypot(y_step, x_step) / 2,
																		colours.bloom_centre [pack->player_index] [20],
																		colours.bloom_edge [pack->player_index] [10],
																		colours.bloom_edge [pack->player_index] [1],
																		(10 + pack->status * 12) * view.zoom, (7 + pack->status * 7) * view.zoom);
*/

     int total_length = 16 + pack->status * 12; // see also cloud code (in g_packet.c)
     int origin_cutoff;
     if (pack->created_timestamp + total_length > w.world_time)
						origin_cutoff = pack->created_timestamp + total_length - w.world_time;
					  else origin_cutoff = 0;

					float blob_size;

					if (pack->type == PACKET_TYPE_BURST)
					{
						shade = 6;
						blob_size = 3.5;
					}
					 else
						{
						 shade = 0;
						 blob_size = 2.5;
						}


     draw_new_pulse_tail(x, y,
																									x_step, y_step,
																									packet_angle,
																									total_length, // maximum length of tail (its length if not cutoff at either end)
																									0, //			int front_cutoff, // number of steps past the front (will be 0 if pulse still travelling)
																									origin_cutoff,//			int origin_cutoff, // number of steps past the origin (will be 0 if end of tail has moved past origin)
																					    3.0 + pack->status * 2.2, // tail_width
																					    pack->colour,
																									18 + shade,
																									blob_size);

     draw_new_pulse_tail(x, y,
																									x_step, y_step,
																									packet_angle,
																									total_length, // maximum length of tail (its length if not cutoff at either end)
																									0, //			int front_cutoff, // number of steps past the front (will be 0 if pulse still travelling)
																									origin_cutoff,//			int origin_cutoff, // number of steps past the origin (will be 0 if end of tail has moved past origin)
																					    1.0 + pack->status * 1.2, // tail_width
																					    pack->colour,
																									24 + shade,
																									blob_size);

/*
     draw_new_pulse_tail(x, y, x_step, y_step,
																					packet_angle,
																					packet_time,
																					0, // start_time
																					end_time,
																					max_time,
																					3.0 + pack->status * 1.5, // tail_width
																					pack->colour,
//																					12 + pulse_or_burst * 8, // shade
																					20, // shade
																					pk - packet_time,
																					pack->status,
																					2.5, // blob_scale
																					pulse_or_burst);


     draw_new_pulse_tail(x, y, x_step, y_step,
																					packet_angle,
																					packet_time,
																					0, // start_time
																					end_time,
																					max_time,
																					1 + pack->status, // tail_width
																					pack->colour,
//																					12 + pulse_or_burst * 8, // shade
																					26, // shade
																					pk - packet_time,
																					pack->status,
																					2.5, // blob_scale
																					pulse_or_burst);
*/
			 }
			 break;

  } // end packet type switch

// basic packets:




 }

 int fr_index;

 for (fr_index = 0; fr_index < FRAGMENTS; fr_index ++)
 {

// TO DO: optimise this to use blocks instead of searching the entire cloud array?
  if (w.fragment[fr_index].destruction_timestamp < w.world_time)
   continue;


  x = al_fixtof(w.fragment[fr_index].position.x - view.camera_x) * view.zoom;
  y = al_fixtof(w.fragment[fr_index].position.y - view.camera_y) * view.zoom;
  x += view.window_x_unzoomed / 2;
  y += view.window_y_unzoomed / 2;


  if (x < (-200 * view.zoom) || x > view.window_x_unzoomed + (200 * view.zoom)
   || y < (-200 * view.zoom) || y > view.window_y_unzoomed + (200 * view.zoom))
    continue;



     int fr_time = w.world_time - w.fragment[fr_index].created_timestamp;
   	 int fr_time_left = w.fragment[fr_index].destruction_timestamp - w.world_time;

//   	 int explode_time_left;

     if (fr_time_left > 31)
					{
      float fragment_x = x;// + (al_fixtof(w.fragment[fr_index].speed.x) * (cl_time - (cl_time * cl_time * 0.003)) * view.zoom);
      float fragment_y = y;// + (al_fixtof(w.fragment[fr_index].speed.y) * (cl_time - (cl_time * cl_time * 0.003)) * view.zoom);
      float fragment_angle = (fr_index * 0.23) + w.fragment[fr_index].spin * fr_time;//fixed_to_radians(w.fragment[fr_index].spin * fr_time);
      float fragment_size = w.fragment[fr_index].fragment_size;
//      explode_time_left = 31;
   		 add_outline_diamond_layer(3,
																															  fragment_x + cos(fragment_angle) * fragment_size * view.zoom,
																															  fragment_y + sin(fragment_angle) * fragment_size * view.zoom,
																															  fragment_x + cos(fragment_angle + PI / 2) * fragment_size * 0.6 * view.zoom,
																															  fragment_y + sin(fragment_angle + PI / 2) * fragment_size * 0.6 * view.zoom,
																															  fragment_x + cos(fragment_angle + PI) * fragment_size * view.zoom,
																															  fragment_y + sin(fragment_angle + PI) * fragment_size * view.zoom,
																															  fragment_x + cos(fragment_angle - PI / 2) * fragment_size * 0.6 * view.zoom,
																															  fragment_y + sin(fragment_angle - PI / 2) * fragment_size * 0.6 * view.zoom,
																															  colours.proc_col [w.fragment[fr_index].colour] [0] [0] [3],
																															  colours.proc_outline [w.fragment[fr_index].colour]  [0] [3]);

					}
					 else
						{
//							explode_time_left = fr_time_left;

       shade = fr_time_left;
						 if (shade > CLOUD_SHADES-1)
							 shade = CLOUD_SHADES-1;
				   if (shade < 0)
							 shade = 0;

							float expl_size = fr_time_left + (w.fragment[fr_index].fragment_size) - 8;

							if (expl_size > 0)
     	  double_circle_with_bloom(3, x, y, expl_size, w.fragment[fr_index].colour, shade);
     	   else
										continue;
/*
       shade = fr_time_left;
						 if (shade > CLOUD_SHADES-1)
							 shade = CLOUD_SHADES-1;
				   if (shade < 0)
							 shade = 0;

						draw_ring(3,x,y,
															(16 + explode_time * 3 - (explode_time * explode_time * 0.03) + 16),// * size_modifier,
															fr_time_left / 4 + 4,
															36,
															colours.packet [w.fragment[fr_index].colour] [shade]);
*/

						}
/*
     float x_step = al_fixtof(w.fragment[fr_index].speed.x) * 3.0 * view.zoom;
     float y_step = al_fixtof(w.fragment[fr_index].speed.y) * 3.0 * view.zoom;

     int trail_shade = explode_time_left;// - explode_time / 6;

     if (trail_shade < 1)
						trail_shade = 0;

					float end_mult = 2 + fr_time * 0.5;

					if (end_mult > 16)
						end_mult = 16;

   		start_ribbon(1,
																		x + x_step,
																		y + y_step,
																		x + y_step,
																		y - x_step,
																		colours.packet [w.fragment[fr_index].colour] [trail_shade / 2]);
					add_ribbon_vertex(x - y_step,
																		     y + x_step,
																		colours.packet [w.fragment[fr_index].colour] [trail_shade / 2]);
					add_ribbon_vertex(x - x_step * end_mult,
																		     y - y_step * end_mult,
																		     colours.packet [w.fragment[fr_index].colour] [0]);

x_step *= 0.5;
y_step *= 0.5;

   		start_ribbon(2,
																		x + x_step,
																		y + y_step,
																		x + y_step,
																		y - x_step,
																		colours.packet [w.fragment[fr_index].colour] [trail_shade]);
					add_ribbon_vertex(x - y_step,
																		     y + x_step,
																		colours.packet [w.fragment[fr_index].colour] [trail_shade]);
					add_ribbon_vertex(x - x_step * end_mult,
																		     y - y_step * end_mult,
																		     colours.packet [w.fragment[fr_index].colour] [0]);
*/
/*
     float fragment_move_angle = atan2(al_fixtof(w.fragment[fr_index].speed.y), al_fixtof(0 - w.fragment[fr_index].speed.y));

   		start_ribbon(2,
																		x + cos(fragment_move_angle) * 3.0 * view.zoom,
																		y + sin(fragment_move_angle) * 3.0 * view.zoom,
																		x + cos(fragment_move_angle + PI*0.5) * 3.0 * view.zoom,
																		y + sin(fragment_move_angle + PI*0.5) * 3.0 * view.zoom,
																		colours.packet [w.fragment[fr_index].colour] [22]);
					add_ribbon_vertex(x + cos(fragment_move_angle - PI*0.5) * 3.0 * view.zoom,
																		     y + sin(fragment_move_angle - PI*0.5) * 3.0 * view.zoom,
																		colours.packet [w.fragment[fr_index].colour] [22]);
					add_ribbon_vertex(x + cos(fragment_move_angle + PI) * al_fixtof(w.fragment[fr_index].speed.x) * 7.0 * view.zoom,
																		     y + sin(fragment_move_angle + PI) * al_fixtof(w.fragment[fr_index].speed.x) * 7.0 * view.zoom,
																		     colours.none);
*/

if (w.world_time > w.fragment[fr_index].explosion_timestamp)
	continue;

     int end_time = fr_time;// / 4;
     int max_time = 29;
     if (end_time > max_time)
						end_time = max_time;

     float x_step = al_fixtof(0 - w.fragment[fr_index].speed.x) * 1 * view.zoom;
     float y_step = al_fixtof(0 - w.fragment[fr_index].speed.y) * 1 * view.zoom;

     float fragment_move_angle = atan2(y_step, x_step) + PI;

					int start_time;

					if (w.world_time <= w.fragment[fr_index].explosion_timestamp)
						start_time = 0;
					  else
							{
						   start_time = (w.world_time - w.fragment[fr_index].explosion_timestamp);// + 1;
					    if (start_time >= end_time)
						    continue;
							}

//							start_time /= 4;

         int time_until_explode = w.fragment[fr_index].explosion_timestamp - w.world_time;


         float size_prop = time_until_explode * 0.03;

//         if (size_prop < 0)
//										continue;


         draw_fragment_tail(x, y,
																												x_step,
																												y_step,
																												fragment_move_angle,
																												fr_time,
																												start_time, // start_time
																												end_time,
																												max_time,
																												0.9 * size_prop,
																												w.fragment[fr_index].colour,
																												16,
																												fr_index - fr_time, // drand_seed
																												0,
																												2 * size_prop);

         draw_fragment_tail(x, y,
																												x_step,
																												y_step,
																												fragment_move_angle,
																												fr_time,
																												start_time, // start_time
																												end_time,
																												max_time,
																												0.5 * size_prop,
																												w.fragment[fr_index].colour,
																												22,
																												fr_index - fr_time, // drand_seed
																												0,
																												size_prop);


/*					 else
						{
							float cl_time_stopped = cl->lifetime - 32;
       float fragment_x = x + (al_fixtof(cl->speed.x) * (cl_time_stopped - (cl_time_stopped * cl_time_stopped * 0.003)) * view.zoom);
       float fragment_y = y + (al_fixtof(cl->speed.y) * (cl_time_stopped - (cl_time_stopped * cl_time_stopped * 0.003)) * view.zoom);
       float fragment_angle = fixed_to_radians(cl->angle + (cl->data [1] * cl_time_stopped));
       float fragment_size = cl->data [0];
						 int fragment_shade3 = cl_time_left;
						 if (fragment_shade3 >= CLOUD_SHADES)
						 	fragment_shade3 = CLOUD_SHADES-1;
						 fragment_size *= (cl_time_left / 16.0) * view.zoom;
   		 add_outline_diamond_layer(2,
																															  fragment_x + cos(fragment_angle) * fragment_size,
																															  fragment_y + sin(fragment_angle) * fragment_size,
																															  fragment_x + cos(fragment_angle + PI / 2) * 0.6 * fragment_size,
																															  fragment_y + sin(fragment_angle + PI / 2) * 0.6 * fragment_size,
																															  fragment_x + cos(fragment_angle + PI) * fragment_size,
																															  fragment_y + sin(fragment_angle + PI) * fragment_size,
																															  fragment_x + cos(fragment_angle - PI / 2) * 0.6 * fragment_size,
																															  fragment_y + sin(fragment_angle - PI / 2) * 0.6 * fragment_size,
																															  colours.packet [cl->colour] [fragment_shade3 / 2],
																															  colours.packet [cl->colour] [fragment_shade3 / 3]);
						 fragment_size *= 0.7;
   		 add_outline_diamond_layer(2,
																															  fragment_x + cos(fragment_angle) * fragment_size,
																															  fragment_y + sin(fragment_angle) * fragment_size,
																															  fragment_x + cos(fragment_angle + PI / 2) * 0.6 * fragment_size,
																															  fragment_y + sin(fragment_angle + PI / 2) * 0.6 * fragment_size,
																															  fragment_x + cos(fragment_angle + PI) * fragment_size,
																															  fragment_y + sin(fragment_angle + PI) * fragment_size,
																															  fragment_x + cos(fragment_angle - PI / 2) * 0.6 * fragment_size,
																															  fragment_y + sin(fragment_angle - PI / 2) * 0.6 * fragment_size,
																															  colours.packet [cl->colour] [fragment_shade3],
																															  colours.packet [cl->colour] [fragment_shade3 / 2]);

 					bloom_circle(1, fragment_x, fragment_y, colours.bloom_centre [cl->colour] [30], colours.bloom_edge [cl->colour] [0], fragment_size * 3);

						}
*/
   	} // end fragment loop
//   	break;




 int cl_time;
// float cl_size;
// int cl_size2;
// int cl_shade;
// int cl_col;
 float cl_angle;
 int c;


 for (c = 0; c < w.max_clouds; c ++)
 {

// TO DO: optimise this to use blocks instead of searching the entire cloud array?
  if (w.cloud[c].destruction_timestamp < w.world_time)
   continue;

  cl = &w.cloud [c];

  x = al_fixtof(cl->position.x - view.camera_x) * view.zoom;
  y = al_fixtof(cl->position.y - view.camera_y) * view.zoom;
  x += view.window_x_unzoomed / 2;
  y += view.window_y_unzoomed / 2;


  if (x < (cl->display_size_x1 * view.zoom) || x > view.window_x_unzoomed + (cl->display_size_x2 * view.zoom)
   || y < (cl->display_size_y1 * view.zoom) || y > view.window_y_unzoomed + (cl->display_size_y2 * view.zoom))
    continue;


  cl_time = w.world_time - cl->created_timestamp;

  switch(cl->type)
  {

		 case CLOUD_BURST_HIT:
	 	case CLOUD_BURST_MISS:
	 		{

     int cloud_time = w.world_time - cl->data[1]; // data[1] was packet.created_timestamp

//     cl_time = w.world_time - cl->created_timestamp;
     cl_angle = fixed_to_radians(cl->angle);

//  			float sharpness = 4 + (w.world_time - pack->created_timestamp) * 0.1;
//  			if (sharpness > 10)
//						sharpness = 10;
//     seed_drand(cl->data[0] - ((cl->data[1]+cl_time)/3)); // data [0] holds packet index
					seed_drand(cl->data[0] - (cl->data[2]/3));
     float blob_size = (drand(10, 1)) * 0.2;


     blob_size = (48 - cl_time + drand(5, 1)) * 0.2;


   		start_ribbon(2,
																		x + cos(cl_angle) * (blob_size+3) * view.zoom,
																		y + sin(cl_angle) * (blob_size+3) * view.zoom,
																		x + cos(cl_angle + PI/4) * blob_size * view.zoom,
																		y + sin(cl_angle + PI/4) * blob_size * view.zoom,
																		colours.packet [cl->colour] [16]);
					add_ribbon_vertex(x + cos(cl_angle - PI/4) * blob_size * view.zoom,
																		     y + sin(cl_angle - PI/4) * blob_size * view.zoom,
																		     colours.packet [cl->colour] [16]);
					add_ribbon_vertex(x + cos(cl_angle + PI/2) * blob_size * view.zoom,
																		     y + sin(cl_angle + PI/2) * blob_size * view.zoom,
																		     colours.packet [cl->colour] [16]);
					add_ribbon_vertex(x + cos(cl_angle - PI/2) * blob_size * view.zoom,
																		     y + sin(cl_angle - PI/2) * blob_size * view.zoom,
																		     colours.packet [cl->colour] [16]);
					add_ribbon_vertex(x + cos(cl_angle + PI*0.7) * blob_size * view.zoom,
																		     y + sin(cl_angle + PI*0.7) * blob_size * view.zoom,
																		     colours.packet [cl->colour] [16]);
					add_ribbon_vertex(x + cos(cl_angle - PI*0.7) * blob_size * view.zoom,
																		     y + sin(cl_angle - PI*0.7) * blob_size * view.zoom,
																		     colours.packet [cl->colour] [16]);


     float x_step = al_fixtof(0 - cl->speed.x) * view.zoom;
     float y_step = al_fixtof(0 - cl->speed.y) * view.zoom;
/*
//     int end_time = (w.world_time - cl->data[1]) / 3;
     int end_time = (cl->data[1] + cl_time) / 3;
     if (end_time > 16)
						end_time = 16;
//					int start_time = (cl_time+cl->data[2]) / 3;
					int start_time = (cl_time-cl->data[2]) / 3;
//					if (start_time < 0)
//						start_time = 0;
     if (start_time > 16)
						start_time = 16;
*/
     int end_time = cloud_time / 3;
     if (end_time > 16)
						end_time = 16;
					int start_time = (w.world_time - cl->created_timestamp) / 3;

					x += x_step * 3;
					y += y_step * 3;

     draw_burst_tail(x, y, x_step, y_step, cl_angle, cloud_time, start_time, end_time, cl->colour);

     float circle_size = (32 + cl_time *2 - (cl_time * cl_time * 0.035));
     float circle_thickness = circle_size - cl_time;

     if (cl->type == CLOUD_BURST_MISS)
					{
						circle_size /= 2;
						circle_thickness /= 2;
					}

     shade = (48 - cl_time);
     if (shade < 0)
						shade = 0;
     if (shade > CLOUD_SHADES - 1)
						shade = CLOUD_SHADES - 1;
     draw_ring(2,
															x - x_step * 3,
															y - y_step * 3,
															circle_size,
															circle_thickness,
															32,
															colours.packet [cl->colour] [shade]);


					seed_drand(cl->index + cl->position.x);
//					circle_size /= 5;
//					circle_thickness /= 5;

					for (i = 0; i < 8; i ++)
					{
/*
						float disp_angle = i * PI * 0.25;
						float disp_dist = drand(30, 1) * view.zoom;
						int circle2_time = cl_time + drand(24, 1);
      float circle2_size = (6 + drand(4, 1) + circle2_time*0.1);// - (circle2_time * circle2_time * 0.02));
      float circle2_thickness = 14 - circle2_time * 0.3; //circle2_size - circle2_time;
      if (circle2_thickness < 0)
							continue;
      if (circle2_thickness > circle2_size)
							circle2_thickness = circle2_size;
*/
/*
						float disp_angle = i * PI * 0.25;
						float disp_dist = drand(30, 1) * view.zoom;
						int circle2_time = cl_time + drand(24, 1);
      float circle2_size = circle2_time*0.2;// - (circle2_time * circle2_time * 0.02));
      float circle2_thickness = circle2_size + 5 - circle2_time * 0.3; //circle2_size - circle2_time;
      if (circle2_thickness < 0)
							continue;
      if (circle2_thickness > circle2_size)
							circle2_thickness = circle2_size;

//					circle2_size /= 3 + drand(3,0);
					//circle2_thickness /= 3 + drand(3,1);

      draw_ring(2,
															 x + cos(disp_angle) * disp_dist, y + sin(disp_angle) * disp_dist,
															 circle2_size,
															 circle2_thickness,
															 24,
															 colours.packet [cl->colour] [shade]);
*/
					}


	 		}
	 		break;

/*
   case CLOUD_PACKET_MISS:
				check_vbuf();
   	{

//   	 cl_size = 8 - ((float) cl_time / 4);
   	 cl_angle = fixed_to_radians(cl->angle);
   	 seed_drand(cl->position.x ^ cl->position.y);
  x += al_fixtof(cl->position2.x) * cl_time * view.zoom;
  y += al_fixtof(cl->position2.y) * cl_time * view.zoom;

   	 start_radial(x, y, 2, colours.packet [cl->colour] [12]);
   	 float blob_size = (cl->lifetime - cl_time) * 2;
   	 for (i = 0; i < 6; i ++)
				 {
      add_radial_vertex(cl_angle + i*(PI/3), blob_size + drand(4,c));
				 }
				 finish_radial();
   	 start_radial(x, y, 2, colours.packet [cl->colour] [24]);
   	 blob_size *= 0.7;
   	 for (i = 0; i < 6; i ++)
				 {
      add_radial_vertex(cl_angle + i*(PI/3), blob_size + drand(4,c));
				 }
				 finish_radial();
   	}
   	break;
*/

			case CLOUD_SPIKE_HIT_AT_LONG_RANGE:
				{

//     int cloud_time = w.world_time - cl->data[1]; // data[1] was packet.created_timestamp

//     cl_time = w.world_time - cl->created_timestamp;
     cl_angle = fixed_to_radians(cl->angle);

//  			float sharpness = 4 + (w.world_time - pack->created_timestamp) * 0.1;
//  			if (sharpness > 10)
//						sharpness = 10;
//     seed_drand(cl->data[0] - ((cl->data[1]+cl_time)/3)); // data [0] holds packet index

     float blob_size = 8 * view.zoom;//(drand(10, 1)) * 0.2;

					int total_blob_size = 33 - cl_time;

					float left_cos = cos(cl_angle - PI/2);
					float left_sin = sin(cl_angle - PI/2);
					float right_cos = cos(cl_angle + PI/2);
					float right_sin = sin(cl_angle + PI/2);
					float step_x = cos(cl_angle) * 2 * view.zoom;
//					step_x = step_x * total_blob_size / 33;
					float step_y = sin(cl_angle) * 2 * view.zoom;
//					step_y = step_y * total_blob_size / 33;


//					step_x = step_x * total_blob_size / 16;
//					step_y = step_y * total_blob_size / 16;

					int cloud_steps = cl_time;

					seed_drand(cl->data[0] - (cl->data[2]) + cloud_steps);

					float cloud_x = x + step_x * cloud_steps;
					float cloud_y = y + step_y * cloud_steps;


   		start_ribbon(2,
																		cloud_x - step_x,
																		cloud_y - step_y,
																		cloud_x - step_x,
																		cloud_y - step_y,
//																		x + left_cos * blob_size,
//																		y + left_sin * blob_size,
																		colours.packet [cl->colour] [16]);
/*					add_ribbon_vertex(x + right_cos * blob_size,
																		     y + right_sin * blob_size,
																		     colours.packet [cl->colour] [16]);*/

					float adjusted_step_x = step_x * 3;// * total_blob_size / 33;
					float adjusted_step_y = step_y * 3;// * total_blob_size / 33;

					shade = 28;

					if (cl_time > 18)
						shade -= (cl_time - 18) * 2;

					for (i = 0; i < 16; i ++)
					{

											cloud_x += adjusted_step_x;
											cloud_y += adjusted_step_y;

						float base_blob_size = (sin((i+1) * 0.185 + PI)) * 48 * total_blob_size;
						blob_size = (base_blob_size + drand(200, 1)) * 0.03 * view.zoom;
											add_ribbon_vertex(cloud_x + left_cos * blob_size,
																		           cloud_y + left_sin * blob_size,
																		           colours.packet [cl->colour] [shade / 2]);
											add_ribbon_vertex(cloud_x + right_cos * blob_size,
																		           cloud_y + right_sin * blob_size,
																		           colours.packet [cl->colour] [shade / 2]);


					}


											add_ribbon_vertex(cloud_x + adjusted_step_x,
																		           cloud_y + adjusted_step_y,
																		           colours.packet [cl->colour] [shade / 2]);




					seed_drand(cl->data[0] - (cl->data[2]) + cloud_steps);

					cloud_x = x + step_x * (cloud_steps + 1);
					cloud_y = y + step_y * (cloud_steps + 1);


   		start_ribbon(3,
																		cloud_x - step_x,
																		cloud_y - step_y,
																		cloud_x - step_x,
																		cloud_y - step_y,
																		colours.packet [cl->colour] [26]);

					adjusted_step_x = step_x * 2.5;//total_blob_size / 36;
					adjusted_step_y = step_y * 2.5;//total_blob_size / 36;

					for (i = 0; i < 16; i ++)
					{

											cloud_x += adjusted_step_x;
											cloud_y += adjusted_step_y;

						float base_blob_size = (sin((i+1) * 0.185 + PI)) * (32 + drand(10, 1)) * total_blob_size;
						blob_size = (base_blob_size) * 0.023 * view.zoom;
											add_ribbon_vertex(cloud_x + left_cos * blob_size,
																		           cloud_y + left_sin * blob_size,
																		           colours.packet [cl->colour] [shade]);
											add_ribbon_vertex(cloud_x + right_cos * blob_size,
																		           cloud_y + right_sin * blob_size,
																		           colours.packet [cl->colour] [shade]);


					}


											add_ribbon_vertex(cloud_x + adjusted_step_x,
																		           cloud_y + adjusted_step_y,
																		           colours.packet [cl->colour] [shade]);






				}
// fall-through:
   case CLOUD_SPIKE_MISS_AT_LONG_RANGE:
   	{
   		int circle_shade = 28 - cl_time / 2;
   	 double_circle_with_bloom(4, x, y, (33 - cl_time), cl->colour, circle_shade);
   	}
    break;


			case CLOUD_ULTRA_HIT:
				{

					int ultra_shade = 32 - cl_time;
					float ultra_size = 96 - cl_time;

					if (ultra_shade > 0)
					      radial_circle(4,
																			 x,
																	   y,
																			 32, // vertices
																			 colours.packet [cl->colour] [ultra_shade],
																			 ultra_size);

//					int k;

					seed_drand(c);

			  for (k = 0; k < 32; k ++)
					{
						ultra_shade = 32 + k - cl_time;
					 float sub_ultra_angle = drand(628, c) * 0.01;
					 float sub_ultra_dist = drand(48, c) * view.zoom;

//					 if (ultra_shade > 31)
//							ultra_shade = 31;
						if (ultra_shade > 0
							&& ultra_shade < 32)
						{
// anything that calls drand needs to go outside the if
						 ultra_size = (ultra_shade - (k / 2));// * 0.7;
						 float max_size = 24 - (k/2);
						 if (ultra_size > max_size)
								ultra_size = (max_size * 2) - ultra_size;
						 if (ultra_size > 0)
	       radial_circle(4,
   																	  x + cos(sub_ultra_angle) * sub_ultra_dist,
			 														    y + sin(sub_ultra_angle) * sub_ultra_dist,
				 															  8, // vertices
					 														  colours.packet [cl->colour] [ultra_shade],
						 													  ultra_size);
						}
					}


				}
// fall-through...
   case CLOUD_ULTRA_MISS:
				{

					int ultra_circle_size = (53 - cl_time) / 2;

   	 double_circle_with_bloom(4, x, y, ultra_circle_size / 2, cl->colour, ultra_circle_size);

     int total_packet_time = cl_time + cl->data[1]; // data[1] is lifetime of packet up to hit/miss

     cl_angle = fixed_to_radians(cl->angle);

					seed_drand(3 - (cl->data[2]/3));
//     float blob_size = (drand(10, 1)) * 0.2;

//     blob_size = (48 - cl_time + drand(5, 1)) * 0.2;

     int end_time = total_packet_time;
     int max_time = 29;
     if (end_time > max_time)
						end_time = max_time;
					int start_time = (w.world_time - cl->created_timestamp);

					if (start_time >= end_time)
						break;

					float tail_width = 0.25;//(pack->status + 4) * 0.04;
     float x_step = al_fixtof(0 - cl->speed.x) * view.zoom;
     float y_step = al_fixtof(0 - cl->speed.y) * view.zoom;


     draw_pulse_tail(x, y, x_step, y_step,
																					cl_angle,
																					cl_time,
																					start_time, // start_time
																					end_time,
																					max_time,
																					tail_width,
																					cl->colour,
																					cl->data [0] - 6, // shade (data [0] is pack->status)
																					cl->data [3] - total_packet_time + 1, // data [3] is packet index
																					3, // packet size
																					1.4, // blob_scale
																					cl->data [5]); // pulse


     end_time = total_packet_time;
     max_time = 24;
     if (end_time > max_time)
						end_time = max_time;
					tail_width = 0.15;//(pack->status + 4) * 0.03;

					if (start_time >= end_time)
						break;

						int base_bloom_size = (max_time - start_time) * 2;

						if (base_bloom_size < 1)
							base_bloom_size = 1;


       bloom_long(1, x + (x_step * 3) / 3, y + (y_step * 3) / 3, cl_angle, (max_time - start_time) * hypot(y_step, x_step),
//       bloom_long(1, x, y, packet_angle, end_time * hypot(y_step, x_step),
																		colours.bloom_centre [cl->colour] [20],
																		colours.bloom_edge [cl->colour] [10],
																		colours.bloom_edge [cl->colour] [1],
//																		(56 + drand(10, 1)) * view.zoom, (25 + drand(5, 1)) * view.zoom);
//																		(max_time - start_time + drand(10, 1)) * view.zoom, ((max_time - start_time) / 2 + drand(5, 1)) * view.zoom);
																		(base_bloom_size + drand(10, 1)) * view.zoom, (base_bloom_size / 2 + drand(5, 1)) * view.zoom);


     draw_pulse_tail(x, y, x_step, y_step,
																					cl_angle,
																					cl_time,
																					start_time, // start_time
																					end_time,
																					max_time,
																					tail_width,
																					cl->colour,
																					cl->data [0], // shade (data [0] is pack->status)
																					cl->data [3] - total_packet_time + 1, // data [3] is packet index
																					3, // packet size
																					0.8, // blob_scale
																					cl->data [5]); // pulse
				}
break;


   case CLOUD_PACKET_HIT: // currently lasts for cl->data[1] ticks
				check_vbuf();
   	{

     float circle_size = (12 + cl_time) + cl->data[0] * 4;// - (cl_time * cl_time * 0.03));
//     float circle_thickness = 9 - (float) cl_time / 2;

     float circle_thickness = (cl->data[1] - cl_time) * (1 + cl->data[0] * 0.5) * 0.5;

     shade = (cl->data[1] - cl_time) * 2;
     if (shade < 0)
						shade = 0;
     if (shade > CLOUD_SHADES - 1)
						shade = CLOUD_SHADES - 1;

     draw_spray(x, y, 2.0 + (cl->data [0] * 0.3), 5 + cl->data [0], cl->colour, shade, cl_time, cl->data[1], 8, cl->position.x ^ cl->position.y);

//     draw_spray(x, y, 2.0 + (cl->data [0] * 0.3), 5 + cl->data [0], cl->colour, shade, cl_time, cl->data[1], cl->data[1], cl->position.x ^ cl->position.y);

     draw_ring(3,
															x, y,
															circle_size,
															circle_thickness,
															16,
															colours.packet [cl->colour] [shade]);
   	}

// fall-through...
   case CLOUD_PACKET_MISS:
   	{


//     radial_blob(2, x, y, 0, 8, colours.packet [cl->colour] [12], (cl->data[1] - cl_time) * 0.8, 2, 2, 3);
//     radial_blob(2, x, y, 0, 8, colours.packet [cl->colour] [28], (cl->data[1] - cl_time) * 0.4, 2, 2, 3);

     double_circle_with_bloom(4, x, y, (cl->data[1] - cl_time) * 0.8, cl->colour, 28);

//     radial_blob(2, x, y, 0, 8, colours.packet [cl->colour] [12], (cl->lifetime - cl_time) * 1.8, 2, 2, 3);
//     radial_blob(2, x, y, 0, 8, colours.packet [cl->colour] [28], (cl->lifetime - cl_time) * 1.0, 2, 2, 3);

//     bloom_circle(1, x, y, colours.bloom_centre [cl->colour] [20], colours.bloom_edge [cl->colour] [0], (cl->lifetime - cl_time) * 6.0 * view.zoom);

   	 cl_angle = fixed_to_radians(cl->angle);

     float x_step = al_fixtof(0 - cl->speed.x) * view.zoom;
     float y_step = al_fixtof(0 - cl->speed.y) * view.zoom;

     int cloud_time = w.world_time - cl->created_timestamp; // data[4] was w.world_time - packet.created_timestamp

     int total_max_time = 16 + cl->data [0] * 12;


     int max_time = total_max_time;//(w.world_time - cl->created_timestamp);//total_max_time;//38 + cl->data [0] * 4;
     int end_time = total_max_time - cloud_time;// * 2;//cloud_time;
     if (end_time > max_time)
						end_time = max_time;
     if (end_time > cl->data [4])
						end_time = cl->data [4];
//					if (end_time < 3)
//							break;
//					int start_time = (w.world_time - cl->created_timestamp);// * 2;

//					if (start_time >= end_time)
//						break;


     bloom_circle(1, x, y, colours.bloom_centre [cl->colour] [20], colours.bloom_edge [cl->colour] [0], 32 * view.zoom);

     shade = 18 - cloud_time;
     int origin_cutoff = total_max_time - (cl->data [4] + cloud_time) - 1;

     if (origin_cutoff < 0)
						origin_cutoff = 0;

     if (shade >= 0)
      draw_new_pulse_tail(x, y,
																										x_step, y_step,
																										cl_angle,
																										total_max_time, // maximum length of tail (its length if not cutoff at either end)
																										cloud_time,//		int front_cutoff, // number of steps past the front (will be 0 if pulse still travelling)
																										origin_cutoff, // number of steps past the origin (will be 0 if end of tail has moved past origin)
																				     	3.0 + cl->data [0] * 2.2, // tail_width
     																					cl->colour,
																										shade,
																										2.5); // blob_scale,

     shade = 24 - cloud_time;

     if (shade >= 0)
      draw_new_pulse_tail(x, y,
																										x_step, y_step,
																										cl_angle,
																										total_max_time, // maximum length of tail (its length if not cutoff at either end)
																										cloud_time,//		int front_cutoff, // number of steps past the front (will be 0 if pulse still travelling)
																										origin_cutoff, // number of steps past the origin (will be 0 if end of tail has moved past origin)
																				     	1.0 + cl->data [0] * 1.2, // tail_width
     																					cl->colour,
																										shade,
																										2.5); // blob_scale,

/*


     if (shade >= 0)
      draw_new_pulse_tail(x, y, x_step, y_step,
																					cl_angle,
																					cl_time,
																					start_time,
																					end_time,
																					max_time,
																					3.0 + cl->data [0] * 1.5, // tail_width
																					cl->colour,
//																					12 + pulse_or_burst * 8, // shade
																					shade, // shade
																					0,
																					cl->data [0], // packet size
																					2.5, // blob_scale
																					cl->data [5]); // pulse

     shade = 26 - cloud_time;

     if (shade >= 0)
      draw_new_pulse_tail(x, y, x_step, y_step,
																					cl_angle,
																					cl_time,
																					start_time,
																					end_time,
																					max_time,
																					1 + cl->data [0], // tail_width
																					cl->colour,
//																					12 + pulse_or_burst * 8, // shade
																					shade, // shade
																					0,
																					cl->data [0], // packet size
																					2.5, // blob_scale
																					cl->data [5]); // pulse

*/

/*

     int end_time = cloud_time;
     int max_time = 6 + cl->data [0] * 4;
     if (end_time > max_time)
						end_time = max_time;
					int start_time = (w.world_time - cl->created_timestamp);

					if (start_time >= end_time)
						break;

					float tail_width = 0.25;//(pack->status + 4) * 0.04;


     draw_pulse_tail(x, y, x_step, y_step,
																					cl_angle,
																					cl_time,
																					start_time, // start_time
																					end_time,
																					max_time,
																					tail_width,
																					cl->colour,
																					12, // shade
																					cl->data [3] - cl_time + 1, // data [3] is packet index
																					cl->data [0], // packet size
																					1.3, // blob_scale
																					cl->data [5]); // pulse


     end_time = cloud_time;
     max_time = 3 + cl->data [0] * 3;
     if (end_time > max_time)
						end_time = max_time;
					tail_width = 0.15;//(pack->status + 4) * 0.03;

					if (start_time >= end_time)
						break;


       bloom_long(1, x + (x_step * cl->data [0]) / 3, y + (y_step * cl->data [0]) / 3, cl_angle, end_time * hypot(y_step, x_step),
//       bloom_long(1, x, y, packet_angle, end_time * hypot(y_step, x_step),
																		colours.bloom_centre [cl->colour] [20],
																		colours.bloom_edge [cl->colour] [10],
																		colours.bloom_edge [cl->colour] [1],
																		(20 + cl->data [0] * 12 + drand(10, 1)) * view.zoom, (10 + cl->data [0] * 7 + drand(5, 1)) * view.zoom);


     draw_pulse_tail(x, y, x_step, y_step,
																					cl_angle,
																					cl_time,
																					start_time, // start_time
																					end_time,
																					max_time,
																					tail_width,
																					cl->colour,
																					28, // shade
																					cl->data [3] - cl_time + 1, // data [3] is packet index
																					cl->data [0], // packet size
																					0.6, // blob_scale
																					cl->data [5]); // pulse

*/

   	}
   	break;

/*
				check_vbuf();
   	{
//   	 seed_drand(cl->position.x ^ cl->position.y);
//   	 cl_angle = fixed_to_radians(cl->angle + AFX_ANGLE_2);
     float circle_size = (12 + cl_time - (cl_time * cl_time * 0.03));
     float circle_thickness = 9 - (float) cl_time / 2;

     shade = 48 - cl_time * 3;
     if (shade < 0)
						shade = 0;
     if (shade > CLOUD_SHADES - 1)
						shade = CLOUD_SHADES - 1;
     draw_ring(2,
															x, y,
															circle_size,
															circle_thickness,
															16,
															colours.packet [cl->colour] [shade]);

   	 seed_drand(cl->position.x ^ cl->position.y);
   	 cl_angle = fixed_to_radians(cl->angle + AFX_ANGLE_2);
   	 float blob_size = (cl->lifetime - cl_time) * 2;
					float bolt_x, bolt_y, bolt_size;
					shade = blob_size * 3;
					if (shade > 26)
						shade = 26;
   		start_ribbon(2,
																		x,
																		y,
																		x,
																		y,
																		colours.packet [cl->colour] [shade / 2]);
//#define PH_BLOB_SPEED (3.0+(cl->data[0]*0.5))
#define PH_BLOB_SPEED (3.0)
					if (cl_time > 0)
					{
					 for (i = 1; i < cl_time; i ++)
					 {
					 	bolt_size = (blob_size / 4) * (1 + (i*0.2));
					 	float side_displacement = drand(5, c) * (1 + (cl_time * 0.03));
					 	if (drand(2, c))
							{
 						 bolt_x = x + cos(cl_angle) * (i * PH_BLOB_SPEED * view.zoom) + cos(cl_angle + PI/2) * side_displacement * view.zoom;
						  bolt_y = y + sin(cl_angle) * (i * PH_BLOB_SPEED * view.zoom) + sin(cl_angle + PI/2) * side_displacement * view.zoom;
							}
							 else
							 {
  						 bolt_x = x + cos(cl_angle) * (i * PH_BLOB_SPEED * view.zoom) + cos(cl_angle - PI/2) * side_displacement * view.zoom;
						   bolt_y = y + sin(cl_angle) * (i * PH_BLOB_SPEED * view.zoom) + sin(cl_angle - PI/2) * side_displacement * view.zoom;
							 }
  					add_ribbon_vertex_vector(bolt_x, bolt_y, cl_angle + PI/2, bolt_size, ribstate.fill_col);
  					add_ribbon_vertex_vector(bolt_x, bolt_y, cl_angle - PI/2, bolt_size, ribstate.fill_col);
					 }
					}
					blob_size = 2 + cl_time*2;
					if ((cl->lifetime - cl_time) < 8)
							blob_size *= ((cl->lifetime - cl_time) * 0.12);
   		float blob_centre_x = x + (cos(cl_angle) * ((cl_time + 4) * PH_BLOB_SPEED * view.zoom));
   		float blob_centre_y = y + (sin(cl_angle) * ((cl_time + 4) * PH_BLOB_SPEED * view.zoom));

					add_ribbon_vertex_vector(blob_centre_x, blob_centre_y, cl_angle + (PI*2)/3, (blob_size+drand(4,c)), ribstate.fill_col);
					add_ribbon_vertex_vector(blob_centre_x, blob_centre_y, cl_angle - (PI*2)/3, (blob_size+drand(4,c)), ribstate.fill_col);
					add_ribbon_vertex_vector(blob_centre_x, blob_centre_y, cl_angle + PI/3, (blob_size+drand(4,c)), ribstate.fill_col);
					add_ribbon_vertex_vector(blob_centre_x, blob_centre_y, cl_angle - PI/3, (blob_size+drand(4,c)), ribstate.fill_col);
					add_ribbon_vertex_vector(blob_centre_x, blob_centre_y, cl_angle, (blob_size+drand(4,c)), ribstate.fill_col);


   	 blob_size *= 0.5;
   	 start_radial(blob_centre_x, blob_centre_y, 2, colours.packet [cl->colour] [shade]);
   	 for (i = 0; i < 6; i ++)
				 {
      add_radial_vertex(cl_angle + i*(PI/3), blob_size + drand(3,c));
				 }
				 finish_radial();

   	}
   	break;
*/
   case CLOUD_SPIKE_HIT:
				check_vbuf();
   	{
   	 cl_angle = fixed_to_radians(cl->angle);

   	 start_radial(x, y, 3, colours.packet [cl->colour] [12]);
   	 float blob_size = (cl->lifetime - cl_time) * 2;
     bloom_circle(1, x, y, colours.bloom_centre [cl->colour] [20], colours.bloom_edge [cl->colour] [0], blob_size * 3.0 * view.zoom);
   	 shade = (cl->lifetime - cl_time) * 2;
   	 if (shade > CLOUD_SHADES - 1)
						shade = CLOUD_SHADES - 1;
   	 for (i = 0; i < 6; i ++)
				 {
      add_radial_vertex(cl_angle + i*(PI/3), blob_size + drand(4,c));
				 }
				 finish_radial();
   	 start_radial(x, y, 2, colours.packet [cl->colour] [shade]);
   	 blob_size *= 0.7;
   	 for (i = 0; i < 6; i ++)
				 {
      add_radial_vertex(cl_angle + i*(PI/3), blob_size + drand(4,c));
				 }
				 finish_radial();

//				 float bit_size_damage_adjust = 1;

				 float bit_dist = (32 + cl_time * 5) * view.zoom;
				 float bit_size = (cl->lifetime - cl_time) * (3 + cl->data [0] * 0.05) * view.zoom;
				 float bit_x = x - cos(cl_angle) * bit_dist;
				 float bit_y = y - sin(cl_angle) * bit_dist;
				 add_diamond_layer(3,
																							bit_x + cos(cl_angle) * bit_size,
																							bit_y + sin(cl_angle) * bit_size,
																							bit_x + cos(cl_angle + PI/2) * bit_size / 3,
																							bit_y + sin(cl_angle + PI/2) * bit_size / 3,
																							bit_x - cos(cl_angle) * bit_size,
																							bit_y - sin(cl_angle) * bit_size,
																							bit_x - cos(cl_angle + PI/2) * bit_size / 3,
																							bit_y - sin(cl_angle + PI/2) * bit_size / 3,
																							colours.packet [cl->colour] [shade / 2]);
				 bit_size *= 0.7;
				 add_diamond_layer(3,
																							bit_x + cos(cl_angle) * bit_size,
																							bit_y + sin(cl_angle) * bit_size,
																							bit_x + cos(cl_angle + PI/2) * bit_size / 3,
																							bit_y + sin(cl_angle + PI/2) * bit_size / 3,
																							bit_x - cos(cl_angle) * bit_size,
																							bit_y - sin(cl_angle) * bit_size,
																							bit_x - cos(cl_angle + PI/2) * bit_size / 3,
																							bit_y - sin(cl_angle + PI/2) * bit_size / 3,
																							colours.packet [cl->colour] [shade]);


   	}
   	break;

   case CLOUD_SPIKE_MISS:
				check_vbuf();
   	{
   	 cl_angle = fixed_to_radians(cl->angle);

   	 start_radial(x, y, 3, colours.packet [cl->colour] [12]);
   	 float blob_size = (cl->lifetime - cl_time) * 2;
     bloom_circle(1, x, y, colours.bloom_centre [cl->colour] [20], colours.bloom_edge [cl->colour] [0], blob_size * 3.0 * view.zoom);
   	 shade = (cl->lifetime - cl_time) * 2;
   	 if (shade > CLOUD_SHADES - 1)
						shade = CLOUD_SHADES - 1;
   	 for (i = 0; i < 6; i ++)
				 {
      add_radial_vertex(cl_angle + i*(PI/3), blob_size + drand(4,c));
				 }
				 finish_radial();
   	 start_radial(x, y, 2, colours.packet [cl->colour] [shade]);
   	 blob_size *= 0.7;
   	 for (i = 0; i < 6; i ++)
				 {
      add_radial_vertex(cl_angle + i*(PI/3), blob_size + drand(4,c));
				 }
				 finish_radial();

   	}
   	break;


   case CLOUD_MAIN_PROC_EXPLODE:
				check_vbuf();
   	{
   		int bits = 36 + cl->data [1];
     int time_left = cl->lifetime - cl_time;
     float size_modifier = 1 + (cl->data [1] * 0.1); // modifies for number of members in group when it was destroyed
     shade = time_left;
						if (shade > CLOUD_SHADES-1)
							shade = CLOUD_SHADES-1;
				  if (shade < 0)
							shade = 0;
						draw_ring(3,x,y,
															(16 + cl_time * 3 - (cl_time * cl_time * 0.03) + 16) * size_modifier,
															time_left / 4 + 4,
															36,
															colours.packet [cl->colour] [shade]);

						draw_ring(3,x,y,
															(16 + cl_time * 3 - (cl_time * cl_time * 0.03)) * size_modifier,
															time_left / 4,
															36,
															colours.packet [cl->colour] [shade]);

/*   		for (i = 0; i < bits; i ++)
					{
						cl_angle = fixed_to_radians(cl->angle) + (PI/(bits/2))*i;
						float bit_dist = 36 + cl_time * 3 - (cl_time * cl_time * 0.03);
						bit_dist *= size_modifier;
   	  float bit_x = x + cos(cl_angle) * bit_dist * view.zoom;
   	  float bit_y = y + sin(cl_angle) * bit_dist * view.zoom;
						int fragment_shade = (time_left);
						if (fragment_shade > CLOUD_SHADES-1)
							fragment_shade = CLOUD_SHADES-1;
				  if (fragment_shade < 0)
							fragment_shade = 0;
						float bit_front = 9 - ((float) cl_time / 6);
						float bit_side = 3 + ((float) cl_time / 8);
   		 add_outline_diamond_layer(2,
																														 	bit_x + cos(cl_angle) * bit_front * view.zoom,
																														 	bit_y + sin(cl_angle) * bit_front * view.zoom,
																														 	bit_x + cos(cl_angle + PI/2) * bit_side * view.zoom,
																														 	bit_y + sin(cl_angle + PI/2) * bit_side * view.zoom,
																														 	bit_x - cos(cl_angle) * bit_front * view.zoom,
																														 	bit_y - sin(cl_angle) * bit_front * view.zoom,
																														 	bit_x + cos(cl_angle - PI/2) * bit_side * view.zoom,
																														 	bit_y + sin(cl_angle - PI/2) * bit_side * view.zoom,
																															 colours.packet [cl->colour] [fragment_shade],
																															 colours.packet [cl->colour] [fragment_shade / 2]);


					}
*/
   		bits = 8 + cl->data [1] / 2;
   		for (i = 0; i < bits; i ++)
					{
      seed_drand((c*(i+999)) ^ cl->position.x);
      int bit_time = cl_time;// - drand(20, c);
						cl_angle = fixed_to_radians(cl->angle) + (PI/(bits/2))*i;
   	  cl_angle += drand(100, c) * 0.01;
						int fragment_shade = (time_left);
						if (fragment_shade > CLOUD_SHADES-1)
							fragment_shade = CLOUD_SHADES-1;
				  if (fragment_shade < 0)
							fragment_shade = 0;
						float bit_front_modifier = bit_time;
						if (bit_time < 20)
						 bit_front_modifier += (20 - bit_time) * 2.1;
						if (bit_time > 30)
						 bit_front_modifier += (bit_time - 30) * 2.1;
//						if (bit_front_modifier > 78)
//							bit_front_modifier = 78;
						float bit_front = 80 - ((float) bit_front_modifier) + drand(20, c);
						float bit_side = 5 + drand(5, c) - bit_time / 15;
//						if (bit_time > 48)
//								bit_side += ((float) (bit_time - 48) * 2);
      if (bit_front < 1)
	 					bit_front = 1;

						float bit_dist = (bit_time * 2) + drand(50, c);
						bit_dist *= size_modifier;
//						bit_dist += 20;
						if (bit_front > bit_dist)
							bit_front = bit_dist;
   	  float bit_x = x + cos(cl_angle) * bit_dist * view.zoom;
   	  float bit_y = y + sin(cl_angle) * bit_dist * view.zoom;


   		 add_outline_diamond_layer(2,
																														 	bit_x + cos(cl_angle) * bit_front * view.zoom,
																														 	bit_y + sin(cl_angle) * bit_front * view.zoom,
																														 	bit_x + cos(cl_angle + PI/2) * bit_side * view.zoom,
																														 	bit_y + sin(cl_angle + PI/2) * bit_side * view.zoom,
																														 	bit_x - cos(cl_angle) * bit_front * view.zoom,
																														 	bit_y - sin(cl_angle) * bit_front * view.zoom,
																														 	bit_x + cos(cl_angle - PI/2) * bit_side * view.zoom,
																														 	bit_y + sin(cl_angle - PI/2) * bit_side * view.zoom,
																															 colours.packet [cl->colour] [fragment_shade],
																															 colours.packet [cl->colour] [fragment_shade / 2]);


					}



				}

				{
   	 int cl_time_left = cl->lifetime - cl_time;
   	 float main_cloud_size = (cl_time_left) - (cl_time * 0.1);
   	 if (main_cloud_size <= 0)
						break;
   	 main_cloud_size /= 36.0;
   	 seed_drand(cl->position.x ^ cl->position.y);
	    draw_jaggy_proc_outline(1, x, y, cl->angle, cl->data [0], main_cloud_size, 6,
																							colours.packet [cl->colour] [9], colours.packet [cl->colour] [3], view.zoom);
					int bloom_shade = cl_time_left;
					if (bloom_shade > 30)
						bloom_shade = 30;
     float bloom_size = cl_time_left - 10;
     if (bloom_size > 0)
					 bloom_circle(1, x, y, colours.bloom_centre [cl->colour] [bloom_shade], colours.bloom_edge [cl->colour] [0], bloom_size * 5 * view.zoom);
//					bloom_circle(1, x, y, colours.bloom_centre [cl->colour] [bloom_shade], colours.bloom_edge [cl->colour] [bloom_shade], main_cloud_size * 10 * view.zoom);
					main_cloud_size -= cl_time / 32.0;
   	 if (main_cloud_size <= 0)
						break;
   	 seed_drand(cl->position.x ^ cl->position.y);
	    draw_jaggy_proc_outline(2, x, y, cl->angle, cl->data [0], main_cloud_size, 6,
																							colours.packet [cl->colour] [21], colours.packet [cl->colour] [15], view.zoom);

				}
				break; // end main proc explosion

   case CLOUD_SUB_PROC_EXPLODE:
				check_vbuf();
				{
/*
   		int bits = 6 + cl->data [1];
     int time_left = cl->lifetime - cl_time;
     float size_modifier = 1 + (cl->data [1] * 0.1); // modifies for number of members in group when it was destroyed
   		for (i = 0; i < bits; i ++)
					{
						cl_angle = fixed_to_radians(cl->angle) + (PI/(bits/2))*i;
						float bit_dist = 36 + cl_time * 3 - (cl_time * cl_time * 0.03);
						bit_dist *= size_modifier;
   	  float bit_x = x + cos(cl_angle) * bit_dist * view.zoom;
   	  float bit_y = y + sin(cl_angle) * bit_dist * view.zoom;
						int fragment_shade = (time_left);
						if (fragment_shade > CLOUD_SHADES-1)
							fragment_shade = CLOUD_SHADES-1;
				  if (fragment_shade < 0)
							fragment_shade = 0;
						float bit_front = 9 - ((float) cl_time / 6);
						float bit_side = 3 + ((float) cl_time / 8);
   		 add_outline_diamond_layer(2,
																														 	bit_x + cos(cl_angle) * bit_front * view.zoom,
																														 	bit_y + sin(cl_angle) * bit_front * view.zoom,
																														 	bit_x + cos(cl_angle + PI/2) * bit_side * view.zoom,
																														 	bit_y + sin(cl_angle + PI/2) * bit_side * view.zoom,
																														 	bit_x - cos(cl_angle) * bit_front * view.zoom,
																														 	bit_y - sin(cl_angle) * bit_front * view.zoom,
																														 	bit_x + cos(cl_angle - PI/2) * bit_side * view.zoom,
																														 	bit_y + sin(cl_angle - PI/2) * bit_side * view.zoom,
																															 colours.packet [cl->colour] [fragment_shade],
																															 colours.packet [cl->colour] [fragment_shade / 2]);


					}
*/
   	 int cl_time_left = cl->lifetime - cl_time;
   	 float main_cloud_size = (cl_time_left) - (cl_time * 0.1);
   	 if (main_cloud_size <= 0)
						break;
					int bloom_shade = cl_time_left;
					if (bloom_shade > 30)
						bloom_shade = 30;
     float bloom_size = cl_time_left - 10;
     if (bloom_size > 0)
 					bloom_circle(1, x, y, colours.bloom_centre [cl->colour] [bloom_shade], colours.bloom_edge [cl->colour] [0], bloom_size * 5 * view.zoom);
   	 main_cloud_size /= 36.0;
   	 seed_drand(cl->position.x ^ cl->position.y);
	    draw_jaggy_proc_outline(1, x, y, cl->angle, cl->data [0], main_cloud_size, 6,
																							colours.packet [cl->colour] [9], colours.packet [cl->colour] [3], view.zoom);
					main_cloud_size -= cl_time / 32.0;
   	 if (main_cloud_size <= 0)
						break;
   	 seed_drand(cl->position.x ^ cl->position.y);
	    draw_jaggy_proc_outline(2, x, y, cl->angle, cl->data [0], main_cloud_size, 6,
																							colours.packet [cl->colour] [21], colours.packet [cl->colour] [15], view.zoom);
				}

    break;
/*
   case CLOUD_PROC_FRAGMENT:
   	{
   	 int cl_time_left = cl->lifetime - cl_time;
     if (cl_time_left > 32)
					{
      float fragment_x = x + (al_fixtof(cl->speed.x) * (cl_time - (cl_time * cl_time * 0.003)) * view.zoom);
      float fragment_y = y + (al_fixtof(cl->speed.y) * (cl_time - (cl_time * cl_time * 0.003)) * view.zoom);
      float fragment_angle = fixed_to_radians(cl->angle + (cl->data [1] * cl_time));
      float fragment_size = cl->data [0];
   		 add_outline_diamond_layer(2,
																															  fragment_x + cos(fragment_angle) * fragment_size * view.zoom,
																															  fragment_y + sin(fragment_angle) * fragment_size * view.zoom,
																															  fragment_x + cos(fragment_angle + PI / 2) * fragment_size * 0.6 * view.zoom,
																															  fragment_y + sin(fragment_angle + PI / 2) * fragment_size * 0.6 * view.zoom,
																															  fragment_x + cos(fragment_angle + PI) * fragment_size * view.zoom,
																															  fragment_y + sin(fragment_angle + PI) * fragment_size * view.zoom,
																															  fragment_x + cos(fragment_angle - PI / 2) * fragment_size * 0.6 * view.zoom,
																															  fragment_y + sin(fragment_angle - PI / 2) * fragment_size * 0.6 * view.zoom,
																															  colours.proc_col [cl->colour] [0] [0] [3],
																															  colours.proc_outline [cl->colour]  [0] [3]);

					}
					 else
						{
							float cl_time_stopped = cl->lifetime - 32;
       float fragment_x = x + (al_fixtof(cl->speed.x) * (cl_time_stopped - (cl_time_stopped * cl_time_stopped * 0.003)) * view.zoom);
       float fragment_y = y + (al_fixtof(cl->speed.y) * (cl_time_stopped - (cl_time_stopped * cl_time_stopped * 0.003)) * view.zoom);
       float fragment_angle = fixed_to_radians(cl->angle + (cl->data [1] * cl_time_stopped));
       float fragment_size = cl->data [0];
						 int fragment_shade3 = cl_time_left;
						 if (fragment_shade3 >= CLOUD_SHADES)
						 	fragment_shade3 = CLOUD_SHADES-1;
						 fragment_size *= (cl_time_left / 16.0) * view.zoom;
   		 add_outline_diamond_layer(2,
																															  fragment_x + cos(fragment_angle) * fragment_size,
																															  fragment_y + sin(fragment_angle) * fragment_size,
																															  fragment_x + cos(fragment_angle + PI / 2) * 0.6 * fragment_size,
																															  fragment_y + sin(fragment_angle + PI / 2) * 0.6 * fragment_size,
																															  fragment_x + cos(fragment_angle + PI) * fragment_size,
																															  fragment_y + sin(fragment_angle + PI) * fragment_size,
																															  fragment_x + cos(fragment_angle - PI / 2) * 0.6 * fragment_size,
																															  fragment_y + sin(fragment_angle - PI / 2) * 0.6 * fragment_size,
																															  colours.packet [cl->colour] [fragment_shade3 / 2],
																															  colours.packet [cl->colour] [fragment_shade3 / 3]);
						 fragment_size *= 0.7;
   		 add_outline_diamond_layer(2,
																															  fragment_x + cos(fragment_angle) * fragment_size,
																															  fragment_y + sin(fragment_angle) * fragment_size,
																															  fragment_x + cos(fragment_angle + PI / 2) * 0.6 * fragment_size,
																															  fragment_y + sin(fragment_angle + PI / 2) * 0.6 * fragment_size,
																															  fragment_x + cos(fragment_angle + PI) * fragment_size,
																															  fragment_y + sin(fragment_angle + PI) * fragment_size,
																															  fragment_x + cos(fragment_angle - PI / 2) * 0.6 * fragment_size,
																															  fragment_y + sin(fragment_angle - PI / 2) * 0.6 * fragment_size,
																															  colours.packet [cl->colour] [fragment_shade3],
																															  colours.packet [cl->colour] [fragment_shade3 / 2]);

 					bloom_circle(1, fragment_x, fragment_y, colours.bloom_centre [cl->colour] [30], colours.bloom_edge [cl->colour] [0], fragment_size * 3);

						}
   	}
   	break;
*/
   case CLOUD_STREAM:
//   case CLOUD_DSTREAM:
   //case CLOUD_SPIKE_LINE:
    draw_stream_beam(x, y, al_fixtof(cl->position2.x - view.camera_x) * view.zoom + (view.window_x_unzoomed/2), al_fixtof(cl->position2.y - view.camera_y) * view.zoom + (view.window_y_unzoomed/2), cl->colour, cl->data[0], cl->data[1], cl->data [2]);
    break;

   case CLOUD_SLICE:
//   case CLOUD_DSTREAM:
   //case CLOUD_SPIKE_LINE:
    draw_slice_beam(x, y, al_fixtof(cl->position2.x - view.camera_x) * view.zoom + (view.window_x_unzoomed/2), al_fixtof(cl->position2.y - view.camera_y) * view.zoom + (view.window_y_unzoomed/2), cl->colour, cl->data [1], cl->data [2]);
    break;


   case CLOUD_SLICE_FADE:
   	{

   		float slice_circle_size = 31 - cl_time * 2;
   		if (slice_circle_size < 0)
						slice_circle_size = 0;

    	float x2 = al_fixtof(cl->position2.x - view.camera_x) * view.zoom + (view.window_x_unzoomed/2);
    	float y2 = al_fixtof(cl->position2.y - view.camera_y) * view.zoom + (view.window_y_unzoomed/2);

   	 double_circle_with_bloom(4, x2, y2, slice_circle_size, cl->colour, slice_circle_size);

#define SLICE_FADE_RATE 8
   	int fade_steps = (cl->data [0] - cl_time * SLICE_FADE_RATE);
   	if (fade_steps <= 1)
					break;
   	float slice_angle = atan2(y2 - y, x2 - x);
   	x += cos(slice_angle) * STREAM_STEP_PIXELS * cl_time * SLICE_FADE_RATE * view.zoom;
   	y += sin(slice_angle) * STREAM_STEP_PIXELS * cl_time * SLICE_FADE_RATE * view.zoom;
    draw_slice_beam(x,
																				y,
																				x2,
																				y2,
																				cl->colour, cl_time + SLICE_FIRING_TIME, cl->data [2]);
   	}
    break;

/*
   case CLOUD_SURGE_TRAIL:
   	{
   	if (cl->data [1] != 1 // only show the first - the following code deals with others
					|| cl->data[0] == -1) // nothing to connect to
					break;

					int trail_cloud_index = cl->index;
					float trail_angle = fixed_to_radians(cl->angle);
					int next_cloud_index;
					int trail_shade = (cl->created_timestamp + cl->lifetime - w.world_time) * 2;
//					if (trail_shade < 0)
//						trail_shade = 0;
#define SURGE_FRONT_SIZE 3
//#define SURGE_TRAIL_SIZE 3
					start_ribbon(4,
																		(al_fixtof(w.cloud[trail_cloud_index].position.x - view.camera_x) + cos(trail_angle) * SURGE_FRONT_SIZE) * view.zoom + (view.window_x_unzoomed/2),
																		(al_fixtof(w.cloud[trail_cloud_index].position.y - view.camera_y) + sin(trail_angle) * SURGE_FRONT_SIZE) * view.zoom + (view.window_y_unzoomed/2),
																		(al_fixtof(w.cloud[trail_cloud_index].position.x - view.camera_x) + cos(trail_angle + PI/2) * SURGE_FRONT_SIZE) * view.zoom + (view.window_x_unzoomed/2),
																		(al_fixtof(w.cloud[trail_cloud_index].position.y - view.camera_y) + sin(trail_angle + PI/2) * SURGE_FRONT_SIZE) * view.zoom + (view.window_y_unzoomed/2),
																		colours.packet [cl->colour] [trail_shade]);

						add_ribbon_vertex((al_fixtof(w.cloud[trail_cloud_index].position.x - view.camera_x) + cos(trail_angle - PI/2) * SURGE_FRONT_SIZE) * view.zoom + (view.window_x_unzoomed/2),
																		      (al_fixtof(w.cloud[trail_cloud_index].position.y - view.camera_y) + sin(trail_angle - PI/2) * SURGE_FRONT_SIZE) * view.zoom + (view.window_y_unzoomed/2),
																		      colours.packet [cl->colour] [trail_shade]);

				 	trail_cloud_index = w.cloud[trail_cloud_index].data[0];

				 	if (trail_cloud_index == -1)
							break;

   	 while(TRUE)
				 {
				 	next_cloud_index = w.cloud[trail_cloud_index].data[0];
				 	if (next_cloud_index == -1)
							break;
						trail_shade -= 2;
						if (trail_shade < 0)
							trail_shade = 0;
						seed_drand(w.cloud[next_cloud_index].position.x + w.cloud[next_cloud_index].position.y);
						float trail_size = (200 + (drand(50, 1)) * trail_shade) * 0.008;
						add_ribbon_vertex((al_fixtof(w.cloud[trail_cloud_index].position.x - view.camera_x) + cos(trail_angle + PI/2) * trail_size) * view.zoom + (view.window_x_unzoomed/2),
																		      (al_fixtof(w.cloud[trail_cloud_index].position.y - view.camera_y) + sin(trail_angle + PI/2) * trail_size) * view.zoom + (view.window_y_unzoomed/2),
																		      colours.packet [cl->colour] [trail_shade]);
						trail_size = (200 + (drand(50, 1)) * trail_shade) * 0.008;
//      trail_size += 12;
						add_ribbon_vertex((al_fixtof(w.cloud[trail_cloud_index].position.x - view.camera_x) - cos(trail_angle + PI/2) * trail_size) * view.zoom + (view.window_x_unzoomed/2),
																		      (al_fixtof(w.cloud[trail_cloud_index].position.y - view.camera_y) - sin(trail_angle + PI/2) * trail_size) * view.zoom + (view.window_y_unzoomed/2),
																		      colours.packet [cl->colour] [trail_shade]);

						trail_cloud_index = next_cloud_index;
						if (w.cloud[trail_cloud_index].destruction_timestamp <= w.world_time)
							break;
				 }
   	}


    break;
*/

   case CLOUD_SPIKE_TRAIL:
   	{

   	if (cl->data [1] != 1 // only show the first - the following code deals with others
					|| cl->data[0] == -1) // nothing to connect to
					break;

					int trail_cloud_index = cl->index;
					float trail_angle = fixed_to_radians(cl->angle);
					int next_cloud_index;
					int trail_shade = (cl->created_timestamp + cl->lifetime - w.world_time) * 2;
					int adjusted_trail_shade;

					if (!w.cloud[trail_cloud_index].data [2])
					{
						if (trail_shade > 12)
							adjusted_trail_shade = 12;
							 else
									adjusted_trail_shade = trail_shade;
					}
					 else
							adjusted_trail_shade = trail_shade;


//					if (trail_shade < 0)
//						trail_shade = 0;
#define SPIKE_FRONT_SIZE 3
#define SPIKE_TRAIL_SIZE 2
					start_ribbon(4,
																		(al_fixtof(w.cloud[trail_cloud_index].position.x - view.camera_x) + cos(trail_angle) * SPIKE_FRONT_SIZE) * view.zoom + (view.window_x_unzoomed/2),
																		(al_fixtof(w.cloud[trail_cloud_index].position.y - view.camera_y) + sin(trail_angle) * SPIKE_FRONT_SIZE) * view.zoom + (view.window_y_unzoomed/2),
																		(al_fixtof(w.cloud[trail_cloud_index].position.x - view.camera_x) + cos(trail_angle + PI/2) * SPIKE_FRONT_SIZE) * view.zoom + (view.window_x_unzoomed/2),
																		(al_fixtof(w.cloud[trail_cloud_index].position.y - view.camera_y) + sin(trail_angle + PI/2) * SPIKE_FRONT_SIZE) * view.zoom + (view.window_y_unzoomed/2),
																		colours.packet [cl->colour] [adjusted_trail_shade]);

						add_ribbon_vertex((al_fixtof(w.cloud[trail_cloud_index].position.x - view.camera_x) + cos(trail_angle - PI/2) * SPIKE_FRONT_SIZE) * view.zoom + (view.window_x_unzoomed/2),
																		      (al_fixtof(w.cloud[trail_cloud_index].position.y - view.camera_y) + sin(trail_angle - PI/2) * SPIKE_FRONT_SIZE) * view.zoom + (view.window_y_unzoomed/2),
																		      colours.packet [cl->colour] [adjusted_trail_shade]);

				 	trail_cloud_index = w.cloud[trail_cloud_index].data[0];

				 	if (trail_cloud_index == -1)
							break;


   	 while(TRUE)
				 {
				 	next_cloud_index = w.cloud[trail_cloud_index].data[0];
				 	if (next_cloud_index == -1)
							break;
						trail_shade -= 2;
						if (trail_shade < 0)
							trail_shade = 0;

					 if (!w.cloud[trail_cloud_index].data [2])
					 {
 						if (trail_shade > 12)
							 adjusted_trail_shade = 12;
 							 else
									 adjusted_trail_shade = trail_shade;
					 }
 					 else
							 adjusted_trail_shade = trail_shade;


						float pos_x = al_fixtof(w.cloud[trail_cloud_index].position.x - view.camera_x) + view.window_x_zoomed / 2;
						float pos_y = al_fixtof(w.cloud[trail_cloud_index].position.y - view.camera_y) + view.window_y_zoomed / 2;
						float angle_cos = cos(trail_angle + PI/2);
						float angle_sin = sin(trail_angle + PI/2);
						add_ribbon_vertex((pos_x + angle_cos * SPIKE_TRAIL_SIZE) * view.zoom,
																		      (pos_y + angle_sin * SPIKE_TRAIL_SIZE) * view.zoom,
																		      colours.packet [cl->colour] [adjusted_trail_shade]);
						add_ribbon_vertex((pos_x - angle_cos * SPIKE_TRAIL_SIZE) * view.zoom,
																		      (pos_y - angle_sin * SPIKE_TRAIL_SIZE) * view.zoom,
																		      colours.packet [cl->colour] [adjusted_trail_shade]);
/*
						if (w.cloud[trail_cloud_index].data [2])
						{
						 float side_size = (trail_shade + (w.cloud[trail_cloud_index].position.x & 7)) * 0.4;
						 add_diamond_layer(3,
																								 (pos_x + cos(trail_angle) * 3) * view.zoom,
																								 (pos_y + sin(trail_angle) * 3) * view.zoom,
																								 (pos_x + angle_cos * side_size) * view.zoom,
																		       (pos_y + angle_sin * side_size) * view.zoom,
																								 (pos_x - cos(trail_angle) * 3) * view.zoom,
																								 (pos_y - sin(trail_angle) * 3) * view.zoom,
																								 (pos_x - angle_cos * side_size) * view.zoom,
																		       (pos_y - angle_sin * side_size) * view.zoom,
																									colours.packet [cl->colour] [trail_shade]);
						}
*/
						trail_cloud_index = next_cloud_index;
						if (w.cloud[trail_cloud_index].destruction_timestamp <= w.world_time)
							break;
				 }
   	}


    break;

/*
   case CLOUD_STREAM_HIT:
   	{
      seed_drand(cl->data [0]); // provided by function that created it
   	  cl_angle = fixed_to_radians(cl->angle) + PI;
    	 float sh_time_left = (float) (cl->lifetime - cl_time) / 5;
   	  float sh_x = x + cos(cl_angle) * (17 + (float) cl_time * 0) * view.zoom;
   	  float sh_y = y + sin(cl_angle) * (17 + (float) cl_time * 0) * view.zoom;

   	  float sh_front = (5 + drand(6, c)) * sh_time_left * view.zoom;
   	  float sh_side = 2 * sh_time_left * view.zoom;
//   	  float sh_side = (2 + drand(2, c)) * sh_time_left * view.zoom;
//   	  float sh_back = 5 * sh_time_left * view.zoom;
      shade = cl->lifetime - cl_time;
      if (shade > CLOUD_SHADES - 1)
							shade = CLOUD_SHADES - 1;

   		 add_outline_diamond_layer(2,
																														 	sh_x + cos(cl_angle) * sh_front,
																														 	sh_y + sin(cl_angle) * sh_front,
																														 	sh_x + cos(cl_angle + PI/2) * sh_side,
																														 	sh_y + sin(cl_angle + PI/2) * sh_side,
																														 	sh_x - cos(cl_angle) * sh_front,
																														 	sh_y - sin(cl_angle) * sh_front,
																														 	sh_x + cos(cl_angle - PI/2) * sh_side,
																														 	sh_y + sin(cl_angle - PI/2) * sh_side,
																															 colours.packet [cl->colour] [shade],
																															 colours.packet [cl->colour] [shade / 2]);

   	}
				break;
*/

   case CLOUD_INTERFACE_BREAK:
				check_vbuf();
   	{
   	 cl_angle = fixed_to_radians(cl->angle);
	 const float cl_x = cos(cl_angle) * view.zoom, cl_y = sin(cl_angle) * view.zoom;
     struct dshape_struct* dsh = &dshape [cl->data [0]];
   	 float time_left = (cl->lifetime - cl_time); // currently lasts for 32 ticks (see in g_proc.c)

     float outwards = 10;

     seed_drand(c + cl->angle);

     for (i = 0; i < dsh->outline_vertices; i ++)
	    {
						outwards = drand(100, 1) * cl_time * 0.01;
//						outwards = 4 + drand(8, 1) + (cl_time * 0.2);
						float scale = outwards / hypot(dsh->outline_vertex_pos[i][0], dsh->outline_vertex_pos[i][1]);
	    	float outwards_xpart = (cl_x * dsh->outline_vertex_pos[i][0] - cl_y * dsh->outline_vertex_pos[i][1]) * scale;
	    	float outwards_ypart = (cl_y * dsh->outline_vertex_pos[i][0] + cl_x * dsh->outline_vertex_pos[i][1]) * scale;

//	    	float hit_shade = time_left * 0.0312;
	    	float hit_shade = time_left * (0.06 + drand(100, 1) * 0.0005);

	    	if (hit_shade > 1)
							hit_shade = 1;

				ALLEGRO_COLOR interface_colour = map_rgba(w.player[cl->colour].interface_colour_base [0] + (w.player[cl->colour].interface_colour_hit [0] * hit_shade) + (w.player[cl->colour].interface_colour_charge [0] * hit_shade),
																																														w.player[cl->colour].interface_colour_base [1] + (w.player[cl->colour].interface_colour_hit [1] * hit_shade) + (w.player[cl->colour].interface_colour_charge [1] * hit_shade),
																																														w.player[cl->colour].interface_colour_base [2] + (w.player[cl->colour].interface_colour_hit [2] * hit_shade) + (w.player[cl->colour].interface_colour_charge [2] * hit_shade),
																																														hit_shade * 250);

	    	add_diamond_layer(2,
																			     x + outwards_xpart * 1.3,
																			     y + outwards_ypart * 1.3,
																			     x + (cl_x * dsh->outline_vertex_sides[i][0][0] - cl_y * dsh->outline_vertex_sides[i][0][1]) * 1.3 + outwards_xpart,
																			     y + (cl_y * dsh->outline_vertex_sides[i][0][0] + cl_x * dsh->outline_vertex_sides[i][0][1]) * 1.3 + outwards_ypart,
																			     x + (cl_x * dsh->outline_vertex_pos[i][0] - cl_y * dsh->outline_vertex_pos[i][1]) * 1.3 + outwards_xpart,
																			     y + (cl_y * dsh->outline_vertex_pos[i][0] + cl_x * dsh->outline_vertex_pos[i][1]) * 1.3 + outwards_ypart,
																			     x + (cl_x * dsh->outline_vertex_sides[i][1][0] - cl_y * dsh->outline_vertex_sides[i][1][1]) * 1.3 + outwards_xpart,
																			     y + (cl_y * dsh->outline_vertex_sides[i][1][0] + cl_x * dsh->outline_vertex_sides[i][1][1]) * 1.3 + outwards_ypart,
     																			interface_colour);
	    }
		 }

/*
   	{
   	 cl_angle = fixed_to_radians(cl->angle);
     struct dshape_struct* dsh = &dshape [cl->data [0]];
   	 float time_left = (cl->lifetime - cl_time);

     float outwards = 10;

     seed_drand(c + cl->angle);

     for (i = 0; i < dsh->outline_vertices; i ++)
	    {
	    	int next_vertex = (i + 1) % dsh->outline_vertices;
//	    	float outwards_xpart = cos(cl_angle + (dsh->outline_vertex_angle [i] + ((dsh->outline_vertex_angle [next_vertex] - dsh->outline_vertex_angle [i]) * 0.5))) * outwards * view.zoom;
//	    	float outwards_ypart = sin(cl_angle + (dsh->outline_vertex_angle [i] + ((dsh->outline_vertex_angle [next_vertex] - dsh->outline_vertex_angle [i]) * 0.5))) * outwards * view.zoom;
//	    	float outwards_ypart = sin(cl_angle + (dsh->outline_vertex_angle [i] + dsh->outline_vertex_angle [next_vertex]) * 0.5) * outwards * view.zoom;
	    	int outwards_vertex = i;
	    	if (drand(2, 1))
							outwards_vertex = next_vertex;
						outwards = drand(100, 1) * cl_time * 0.01;
	    	float outwards_xpart = cos(cl_angle + dsh->outline_vertex_angle [outwards_vertex]) * outwards * view.zoom;
	    	float outwards_ypart = sin(cl_angle + dsh->outline_vertex_angle [outwards_vertex]) * outwards * view.zoom;

	    	float hit_shade = time_left * 0.0312;

				ALLEGRO_COLOR interface_colour = al_map_rgba(w.player[cl->colour].interface_colour_base [0] + (w.player[cl->colour].interface_colour_var [0] * hit_shade),
																																																	w.player[cl->colour].interface_colour_base [1] + (w.player[cl->colour].interface_colour_var [1] * hit_shade),
																																																	w.player[cl->colour].interface_colour_base [2] + (w.player[cl->colour].interface_colour_var [2] * hit_shade),
																																																	hit_shade * 160);

	    	add_triangle(2,
																			x + outwards_xpart * 1.3,
																			y + outwards_ypart * 1.3,
																			x + cos(cl_angle + dsh->outline_vertex_angle [i]) * dsh->outline_vertex_dist [i] * view.zoom * 1.3 + outwards_xpart,
																			y + sin(cl_angle + dsh->outline_vertex_angle [i]) * dsh->outline_vertex_dist [i] * view.zoom * 1.3 + outwards_ypart,
																			x + cos(cl_angle + dsh->outline_vertex_angle [next_vertex]) * dsh->outline_vertex_dist [next_vertex] * view.zoom * 1.3 + outwards_xpart,
																			y + sin(cl_angle + dsh->outline_vertex_angle [next_vertex]) * dsh->outline_vertex_dist [next_vertex] * view.zoom * 1.3 + outwards_ypart,
																			interface_colour);
	    }
		 }
*/
/*
   		int bits = 12;
     int time_left = cl->lifetime - cl_time;
//     float size_modifier = 1;
   		for (i = 0; i < bits; i ++)
					{
						cl_angle = fixed_to_radians(cl->angle) + (PI/(bits/2))*i;
						float bit_dist = 24 + ((i & 1) * 12) + cl_time * 2;// * 1;// - (cl_time * cl_time * 0.01);
   	  float bit_x = x + cos(cl_angle) * bit_dist * view.zoom;
   	  float bit_y = y + sin(cl_angle) * bit_dist * view.zoom;
						int fragment_shade = (time_left);
						if (fragment_shade > CLOUD_SHADES-1)
							fragment_shade = CLOUD_SHADES-1;
				  if (fragment_shade < 0)
							fragment_shade = 0;
						float bit_front = 21 - ((float) cl_time / 6);
						float bit_side = 3 + ((float) cl_time / 8);
   		 add_outline_diamond_layer(2,
																														 	bit_x + cos(cl_angle) * bit_front * view.zoom,
																														 	bit_y + sin(cl_angle) * bit_front * view.zoom,
																														 	bit_x + cos(cl_angle + PI/2) * bit_side * view.zoom,
																														 	bit_y + sin(cl_angle + PI/2) * bit_side * view.zoom,
																														 	bit_x - cos(cl_angle) * bit_front * view.zoom,
																														 	bit_y - sin(cl_angle) * bit_front * view.zoom,
																														 	bit_x + cos(cl_angle - PI/2) * bit_side * view.zoom,
																														 	bit_y + sin(cl_angle - PI/2) * bit_side * view.zoom,
																															 colours.packet [cl->colour] [fragment_shade],
																															 colours.packet [cl->colour] [fragment_shade / 2]);


					}


				}
*/
    break;

   case CLOUD_BUBBLE_TEXT:
//			 if (w.core[pr->core_index].bubble_text_time >= w.world_time - BUBBLE_TOTAL_TIME)
			 {
// This cloud probably exists because the core has been destroyed.
// However, it should be safe to refer to the core's data structure because
//  it will still be deallocating:
  		 	w.core[cl->data [0]].bubble_list = bubble_list_index;
				 	bubble_list_index = cl->data [0];
		 	 	w.core[cl->data [0]].bubble_x = x;
				  w.core[cl->data [0]].bubble_y = y - 120 * view.zoom;//scaleUI_y(FONT_SQUARE,120) * view.zoom;
			 }
			 break;


/*
    case CLOUD_HARVEST_LINE:
    {
// first check the proc that produced the line still exists:
    if (w.proc[cl->data[0]].exists <= 0
					|| w.proc[cl->data[0]].created_timestamp != cl->associated_proc_timestamp)
						break;
// assume:
//  data[0] is associated proc index
//  data[1] is harvest object index

				float well_x = al_fixtof(cl->position.x - view.camera_x) * view.zoom; // if this is a data transfer to another proc, this could be the proc's location at transfer time
				float well_y = al_fixtof(cl->position.y - view.camera_y) * view.zoom;
    well_x += view.window_x_unzoomed / 2;
    well_y += view.window_y_unzoomed / 2;
    al_fixed vertex_x_fixed = w.proc[cl->data[0]].position.x + fixed_xpart(w.proc[cl->data[0]].angle + w.proc[cl->data[0]].nshape_ptr->object_angle_fixed [cl->data[1]], w.proc[cl->data[0]].nshape_ptr->object_dist_fixed [cl->data[1]]);
    al_fixed vertex_y_fixed = w.proc[cl->data[0]].position.y + fixed_ypart(w.proc[cl->data[0]].angle + w.proc[cl->data[0]].nshape_ptr->object_angle_fixed [cl->data[1]], w.proc[cl->data[0]].nshape_ptr->object_dist_fixed [cl->data[1]]);
    float vx, vy;
    vx = al_fixtof(w.proc[cl->data[0]].position.x - view.camera_x) * view.zoom + (view.window_x_unzoomed / 2);
    vx += (cos(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * (dshape[w.proc[cl->data[0]].shape].link_point_dist [cl->data[1]] [1] + 7)) * view.zoom;
    vy = al_fixtof(w.proc[cl->data[0]].position.y - view.camera_y) * view.zoom + (view.window_y_unzoomed / 2);
    vy += (sin(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * (dshape[w.proc[cl->data[0]].shape].link_point_dist [cl->data[1]] [1] + 7)) * view.zoom;

    float angle_from_well = atan2(vy - well_y, vx - well_x);
    float harvest_line_length_unzoomed = hypot(al_fixtoi(cl->position.y - vertex_y_fixed), al_fixtoi(cl->position.x - vertex_x_fixed));
    int line_segments = (harvest_line_length_unzoomed / 16) + 1;
    float line_segment_length_zoomed = (harvest_line_length_unzoomed / line_segments) * view.zoom;

    float lx, ly;

    seed_drand(cl->created_timestamp + c);

#define HARVEST_LINE_TIME 16

    int line_time = w.world_time - cl->created_timestamp;

    float oscil_angle = PI + (cl->created_timestamp + line_time) * 0.01;
    float oscil_angle_inc = (5 + drand(5, 1)) * -0.03;

    float oscil_angle2 = PI + (cl->created_timestamp + line_time) * 0.02;
    float oscil_angle2_inc = (5 + drand(5, 1)) * -0.01;


    shade = 32 - (line_time * 2);
    if (shade > CLOUD_SHADES - 1)
					shade = CLOUD_SHADES - 1;

				float line_thickness = (float) (HARVEST_LINE_TIME - line_time) / 8;
				float step_x = cos(angle_from_well) * line_segment_length_zoomed;
				float step_y = sin(angle_from_well) * line_segment_length_zoomed;

   		start_ribbon(3,
																		well_x - cos(angle_from_well) * line_thickness * 3 * view.zoom, well_y - sin(angle_from_well) * line_thickness * 3 * view.zoom,
																		well_x - cos(angle_from_well) * line_thickness * 3 * view.zoom, well_y - sin(angle_from_well) * line_thickness * 3 * view.zoom,
																		colours.packet [cl->colour] [shade]);


					add_ribbon_vertex(well_x + cos(angle_from_well - PI/2) * line_thickness * 3 * view.zoom, well_y + sin(angle_from_well - PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);
					add_ribbon_vertex(well_x + cos(angle_from_well + PI/2) * line_thickness * 3 * view.zoom, well_y + sin(angle_from_well + PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);

					float bloom_thickness = line_thickness * 10 * view.zoom;

					bloom_circle(2, well_x, well_y, colours.bloom_centre [cl->colour] [shade], colours.bloom_edge [cl->colour] [0], bloom_thickness);

     start_bloom_ribbon(2,
																								well_x, well_y,
																								well_x, well_y,
																								well_x, well_y,
//																								well_x + cos(angle_from_well - PI/2) * bloom_thickness, well_y + sin(angle_from_well - PI/2) * bloom_thickness,
//																								well_x + cos(angle_from_well + PI/2) * bloom_thickness, well_y + sin(angle_from_well + PI/2) * bloom_thickness,
																								colours.bloom_centre [cl->colour] [shade / 2], colours.bloom_edge [cl->colour] [0]);

    seed_drand(cl->created_timestamp + c);

    for (i = 1; i < line_segments; i ++)
				{
					float side_displacement = cos(oscil_angle) * 64.0;
					side_displacement += cos(oscil_angle2) * 64.0;
					side_displacement += (drand(13, 1) - 6) * (line_time * 0.15);
					if (i < 8)
						side_displacement *= (i / 8.0);
					  else
							{
								if (i > line_segments - 8
								&& (line_segments - i) > 0)
   						side_displacement *= (line_segments - i) / 8.0;
							}
					side_displacement *= view.zoom;
					lx = well_x + step_x * i + cos(angle_from_well - PI/2) * side_displacement;
					ly = well_y + step_y * i + sin(angle_from_well - PI/2) * side_displacement;
					add_ribbon_vertex(lx + cos(angle_from_well - PI/2) * line_thickness * view.zoom, ly + sin(angle_from_well - PI/2) * line_thickness * view.zoom, ribstate.fill_col);
					add_ribbon_vertex(lx + cos(angle_from_well + PI/2) * line_thickness * view.zoom, ly + sin(angle_from_well + PI/2) * line_thickness * view.zoom, ribstate.fill_col);

     add_bloom_ribbon_vertices(
																								lx, ly,
																								lx + cos(angle_from_well - PI/2) * bloom_thickness, ly + sin(angle_from_well - PI/2) * bloom_thickness,
																								lx + cos(angle_from_well + PI/2) * bloom_thickness, ly + sin(angle_from_well + PI/2) * bloom_thickness);

     oscil_angle += oscil_angle_inc;
     oscil_angle2 += oscil_angle2_inc;
				}

					add_ribbon_vertex(vx + cos(angle_from_well - PI/2) * line_thickness * 3 * view.zoom, vy + sin(angle_from_well - PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);
					add_ribbon_vertex(vx + cos(angle_from_well + PI/2) * line_thickness * 3 * view.zoom, vy + sin(angle_from_well + PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);

     add_bloom_ribbon_vertices(
																								vx, vy,
																								vx + cos(angle_from_well - PI/2) * bloom_thickness, vy + sin(angle_from_well - PI/2) * bloom_thickness,
																								vx + cos(angle_from_well + PI/2) * bloom_thickness, vy + sin(angle_from_well + PI/2) * bloom_thickness);

					add_ribbon_vertex(vx + cos(angle_from_well) * line_thickness * 3 * view.zoom, vy + sin(angle_from_well) * line_thickness * 3 * view.zoom, ribstate.fill_col);

					finish_bloom_ribbon(); // no need to finish the non-bloom ribbon. Since the bloom ribbon and the non-bloom ribbon are on different layers, can mix them if needed.

					bloom_circle(2, vx, vy, colours.bloom_centre [cl->colour] [shade], colours.bloom_edge [cl->colour] [0], bloom_thickness);

  }
  break; // end CLOUD_HARVEST_LINE
*/


    case CLOUD_HARVEST_LINE:
    case CLOUD_GIVE_LINE:
    case CLOUD_TAKE_LINE:
    {
// first check the proc that produced the line still exists:
    if (w.proc[cl->data[0]].exists <= 0
					|| w.proc[cl->data[0]].created_timestamp != cl->associated_proc_timestamp)
						break;
// assume:
//  data[0] is associated proc index
//  data[1] is harvest object index

// for CLOUD_GIVE_LINE, well_x and y are the source harvest object
//				float well_x = al_fixtof(cl->position.x - view.camera_x) * view.zoom; // if this is a data transfer to another proc, this could be the proc's location at transfer time
//				float well_y = al_fixtof(cl->position.y - view.camera_y) * view.zoom;
//    well_x += view.window_x_unzoomed / 2;
//    well_y += view.window_y_unzoomed / 2;
    float well_x, well_y;
    float vx, vy;
    float harvest_line_length_unzoomed;
    al_fixed vertex_x_fixed, vertex_y_fixed;
    if (cl->type == CLOUD_HARVEST_LINE)
    {
// target of harvest line is harvest object.
     vertex_x_fixed = w.proc[cl->data[0]].position.x + fixed_xpart(w.proc[cl->data[0]].angle + w.proc[cl->data[0]].nshape_ptr->object_angle_fixed [cl->data[1]], w.proc[cl->data[0]].nshape_ptr->object_dist_fixed [cl->data[1]]);
     vertex_y_fixed = w.proc[cl->data[0]].position.y + fixed_ypart(w.proc[cl->data[0]].angle + w.proc[cl->data[0]].nshape_ptr->object_angle_fixed [cl->data[1]], w.proc[cl->data[0]].nshape_ptr->object_dist_fixed [cl->data[1]]);
     vx = al_fixtof(w.proc[cl->data[0]].position.x - view.camera_x) * view.zoom + (view.window_x_unzoomed / 2);
     vx += (cos(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * (dshape[w.proc[cl->data[0]].shape].link_point_dist [cl->data[1]] [1] + 7)) * view.zoom;
     vy = al_fixtof(w.proc[cl->data[0]].position.y - view.camera_y) * view.zoom + (view.window_y_unzoomed / 2);
     vy += (sin(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * (dshape[w.proc[cl->data[0]].shape].link_point_dist [cl->data[1]] [1] + 7)) * view.zoom;
// source is well
				 well_x = al_fixtof(cl->position.x - view.camera_x) * view.zoom; // if this is a data transfer to another proc, this could be the proc's location at transfer time
     well_x += view.window_x_unzoomed / 2;
				 well_y = al_fixtof(cl->position.y - view.camera_y) * view.zoom;
     well_y += view.window_y_unzoomed / 2;

     harvest_line_length_unzoomed = hypot(al_fixtoi(cl->position.y - vertex_y_fixed), al_fixtoi(cl->position.x - vertex_x_fixed));
    }
     else
					{
						if (cl->type == CLOUD_GIVE_LINE)
						{

       vertex_x_fixed = cl->position.x;
       vertex_y_fixed = cl->position.y;
// source is transferrer object
//      well_x = al_fixtof(w.proc[cl->data[0]].position.x - view.camera_x) * view.zoom + (view.window_x_unzoomed / 2);
//      well_y = al_fixtof(w.proc[cl->data[0]].position.y - view.camera_y) * view.zoom + (view.window_y_unzoomed / 2);
       well_x = al_fixtof(w.proc[cl->data[0]].position.x - view.camera_x) * view.zoom + (view.window_x_unzoomed / 2);
       well_x += (cos(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * (dshape[w.proc[cl->data[0]].shape].link_point_dist [cl->data[1]] [1] + 7)) * view.zoom;
       well_y = al_fixtof(w.proc[cl->data[0]].position.y - view.camera_y) * view.zoom + (view.window_y_unzoomed / 2);
       well_y += (sin(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * (dshape[w.proc[cl->data[0]].shape].link_point_dist [cl->data[1]] [1] + 7)) * view.zoom;

// target of give line is core of transfer target
				   vx = al_fixtof(cl->position.x - view.camera_x) * view.zoom; // if this is a data transfer to another proc, this could be the proc's location at transfer time
       vx += view.window_x_unzoomed / 2;
				   vy = al_fixtof(cl->position.y - view.camera_y) * view.zoom;
       vy += view.window_y_unzoomed / 2;

       harvest_line_length_unzoomed = hypot(al_fixtoi(cl->position.y - w.proc[cl->data[0]].position.y), al_fixtoi(cl->position.x - w.proc[cl->data[0]].position.x));
						}
						 else // must be CLOUD_TAKE_LINE
							{

        vertex_x_fixed = cl->position.x;
        vertex_y_fixed = cl->position.y;

// target of give line is harvest object
        vx = al_fixtof(w.proc[cl->data[0]].position.x - view.camera_x) * view.zoom + (view.window_x_unzoomed / 2);
        vx += (cos(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * (dshape[w.proc[cl->data[0]].shape].link_point_dist [cl->data[1]] [1] + 7)) * view.zoom;
        vy = al_fixtof(w.proc[cl->data[0]].position.y - view.camera_y) * view.zoom + (view.window_y_unzoomed / 2);
        vy += (sin(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * (dshape[w.proc[cl->data[0]].shape].link_point_dist [cl->data[1]] [1] + 7)) * view.zoom;

// source is transferrer core
				    well_x = al_fixtof(cl->position.x - view.camera_x) * view.zoom; // if this is a data transfer to another proc, this could be the proc's location at transfer time
        well_x += view.window_x_unzoomed / 2;
				    well_y = al_fixtof(cl->position.y - view.camera_y) * view.zoom;
        well_y += view.window_y_unzoomed / 2;

        harvest_line_length_unzoomed = hypot(al_fixtoi(cl->position.y - w.proc[cl->data[0]].position.y), al_fixtoi(cl->position.x - w.proc[cl->data[0]].position.x));
							}
					}

    float angle_from_well = atan2(vy - well_y, vx - well_x);
    int line_segments = (harvest_line_length_unzoomed / 17) + 1;
    if (line_segments < 2)
					line_segments = 2;
    int line_segments_shown = cl_time * 3;// * 4;
    if (line_segments_shown > line_segments)
					line_segments_shown = line_segments;
    float line_segment_length_zoomed = 16.0 * view.zoom; //(float) ((float) harvest_line_length_unzoomed / (float) line_segments) * view.zoom;

    float lx, ly;

    seed_drand(cl->created_timestamp + c);

#define HARVEST_LINE_TIME 32

    int line_time = w.world_time - cl->created_timestamp;
    int adjusted_line_time = (line_time * 2) - 32;
    if (adjusted_line_time < 0)
					adjusted_line_time = 0;

    float oscil_angle = PI + (cl->created_timestamp + line_time) * 0.01;
    float oscil_angle_inc = (5 + drand(5, 1)) * -0.03;

    float oscil_angle2 = PI + (cl->created_timestamp + line_time) * 0.02;
    float oscil_angle2_inc = (5 + drand(5, 1)) * -0.01;


    shade = 64 - (line_time * 2);
    if (shade > CLOUD_SHADES - 1)
					shade = CLOUD_SHADES - 1;

				float line_thickness = (float) (32 - adjusted_line_time) * 0.1;
				float step_x = cos(angle_from_well) * line_segment_length_zoomed;
				float step_y = sin(angle_from_well) * line_segment_length_zoomed;

   		start_ribbon(3,
																		well_x - cos(angle_from_well) * line_thickness * 3 * view.zoom, well_y - sin(angle_from_well) * line_thickness * 3 * view.zoom,
																		well_x - cos(angle_from_well) * line_thickness * 3 * view.zoom, well_y - sin(angle_from_well) * line_thickness * 3 * view.zoom,
																		colours.packet [cl->colour] [shade]);


					add_ribbon_vertex(well_x + cos(angle_from_well - PI/2) * line_thickness * 3 * view.zoom, well_y + sin(angle_from_well - PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);
					add_ribbon_vertex(well_x + cos(angle_from_well + PI/2) * line_thickness * 3 * view.zoom, well_y + sin(angle_from_well + PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);

					float bloom_thickness = line_thickness * 10 * view.zoom;

//					bloom_circle(2, well_x, well_y, colours.bloom_centre [cl->colour] [shade / 2], colours.bloom_edge [cl->colour] [0], bloom_thickness);
// TO DO: instead of bloom_circle, make the ribbon take a circle shape around the end
//     start_bloom_ribbon(2,
//																								well_x, well_y,
//																								well_x, well_y,
//																								well_x, well_y,
//																								well_x + cos(angle_from_well - PI/2) * bloom_thickness, well_y + sin(angle_from_well - PI/2) * bloom_thickness,
//																								well_x + cos(angle_from_well + PI/2) * bloom_thickness, well_y + sin(angle_from_well + PI/2) * bloom_thickness,
//																								colours.bloom_centre [cl->colour] [shade / 2], colours.bloom_edge [cl->colour] [0]);

     start_bloom_ribbon(2,
//																								well_x, well_y,
																								well_x, well_y,
//																								well_x, well_y,
																								well_x + cos(angle_from_well - PI) * bloom_thickness, well_y + sin(angle_from_well - PI) * bloom_thickness,
																								well_x + cos(angle_from_well - PI) * bloom_thickness, well_y + sin(angle_from_well - PI) * bloom_thickness,
//																								well_x - cos(angle_from_well + PI/4) * bloom_thickness, well_y - sin(angle_from_well + PI/4) * bloom_thickness,
//																								well_x - cos(angle_from_well - PI/4) * bloom_thickness, well_y - sin(angle_from_well - PI/4) * bloom_thickness,
																								colours.bloom_centre [cl->colour] [shade / 2], colours.bloom_edge [cl->colour] [0]);

     add_bloom_ribbon_vertices(
																								well_x, well_y,
																								well_x - cos(angle_from_well + PI/4) * bloom_thickness, well_y - sin(angle_from_well + PI/4) * bloom_thickness,
																								well_x - cos(angle_from_well - PI/4) * bloom_thickness, well_y - sin(angle_from_well - PI/4) * bloom_thickness);


     add_bloom_ribbon_vertices(
																								well_x, well_y,
																								well_x + cos(angle_from_well - PI/2) * bloom_thickness, well_y + sin(angle_from_well - PI/2) * bloom_thickness,
																								well_x + cos(angle_from_well + PI/2) * bloom_thickness, well_y + sin(angle_from_well + PI/2) * bloom_thickness);


    seed_drand(cl->created_timestamp + c);

    lx = well_x;
    ly = well_y;

    for (i = 1; i < line_segments_shown; i ++)
				{
					float side_displacement = cos(oscil_angle) * 64.0;
//					float i_float = i;
					side_displacement += cos(oscil_angle2) * 64.0;
					side_displacement += (drand(13, 1) - 6) * (adjusted_line_time * 0.08);

//					if (i < line_segments / 2)
//						side_displacement *= side_disp_fraction * i;
//					  else
//						  side_displacement *= 2.0 - (side_disp_fraction * i);

					if (line_segments > 2)
					 side_displacement *= sin((float) i * PI / (float) (line_segments + 1));
/*
					if (i < 4)
						side_displacement *= (i-1) * 0.25;
					  else
							{
								if (i > line_segments - 4
								&& (line_segments - i) > 0)
   						side_displacement *= (line_segments - i - 1) * 0.25;
							}*/
					side_displacement *= view.zoom;
					lx = well_x + step_x * i + cos(angle_from_well - PI/2) * side_displacement;
					ly = well_y + step_y * i + sin(angle_from_well - PI/2) * side_displacement;

					add_ribbon_vertex(lx + cos(angle_from_well - PI/2) * line_thickness * view.zoom, ly + sin(angle_from_well - PI/2) * line_thickness * view.zoom,
//																												colours.packet [cl->colour] [vertex_shade]);
																												ribstate.fill_col);
					add_ribbon_vertex(lx + cos(angle_from_well + PI/2) * line_thickness * view.zoom, ly + sin(angle_from_well + PI/2) * line_thickness * view.zoom,
//																												colours.packet [cl->colour] [vertex_shade]);
																												ribstate.fill_col);
     add_bloom_ribbon_vertices(
																								lx, ly,
																								lx + cos(angle_from_well - PI/2) * bloom_thickness, ly + sin(angle_from_well - PI/2) * bloom_thickness,
																								lx + cos(angle_from_well + PI/2) * bloom_thickness, ly + sin(angle_from_well + PI/2) * bloom_thickness);

     oscil_angle += oscil_angle_inc;
     oscil_angle2 += oscil_angle2_inc;
				}

				 if (line_segments_shown == line_segments)
					{

						float new_angle = atan2(vy - ly, vx - lx);

					add_ribbon_vertex(vx + cos(new_angle - PI/2) * line_thickness * 3 * view.zoom, vy + sin(new_angle - PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);
					add_ribbon_vertex(vx + cos(new_angle + PI/2) * line_thickness * 3 * view.zoom, vy + sin(new_angle + PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);

     add_bloom_ribbon_vertices(
																								vx, vy,
																								vx + cos(new_angle - PI/2) * bloom_thickness, vy + sin(new_angle - PI/2) * bloom_thickness,
																								vx + cos(new_angle + PI/2) * bloom_thickness, vy + sin(new_angle + PI/2) * bloom_thickness);


     add_bloom_ribbon_vertices(
																								vx, vy,
                        vx + cos(new_angle) * bloom_thickness * view.zoom, vy + sin(new_angle) * bloom_thickness * view.zoom,
                        vx + cos(new_angle) * bloom_thickness * view.zoom, vy + sin(new_angle) * bloom_thickness * view.zoom);

					add_ribbon_vertex(vx + cos(new_angle) * line_thickness * 3 * view.zoom, vy + sin(new_angle) * line_thickness * 3 * view.zoom, ribstate.fill_col);

					finish_bloom_ribbon(); // no need to finish the non-bloom ribbon. Since the bloom ribbon and the non-bloom ribbon are on different layers, can mix them if needed.

//					bloom_circle(2, vx, vy, colours.bloom_centre [cl->colour] [shade], colours.bloom_edge [cl->colour] [0], bloom_thickness);

					}
					 else
						{

							lx += step_x / 3;
							ly += step_y / 3;

 						float new_angle = angle_from_well;//atan2(vy - ly, vx - lx);

//					  add_ribbon_vertex(lx + cos(new_angle - PI/2) * line_thickness * 1 * view.zoom, ly + sin(new_angle - PI/2) * line_thickness * 1 * view.zoom, ribstate.fill_col);
//					  add_ribbon_vertex(lx + cos(new_angle + PI/2) * line_thickness * 1 * view.zoom, ly + sin(new_angle + PI/2) * line_thickness * 1 * view.zoom, ribstate.fill_col);

     add_bloom_ribbon_vertices(
																								lx, ly,
//                        lx + cos(angle_from_well) * bloom_thickness * view.zoom, ly + sin(angle_from_well) * bloom_thickness * view.zoom,
																								lx + cos(new_angle - PI/2) * bloom_thickness, ly + sin(new_angle - PI/2) * bloom_thickness,
																								lx + cos(new_angle + PI/2) * bloom_thickness, ly + sin(new_angle + PI/2) * bloom_thickness);

     add_bloom_ribbon_vertices(
																								lx, ly,
                        lx + cos(new_angle) * bloom_thickness * view.zoom, ly + sin(new_angle) * bloom_thickness * view.zoom,
                        lx + cos(new_angle) * bloom_thickness * view.zoom, ly + sin(new_angle) * bloom_thickness * view.zoom);


//					  add_ribbon_vertex(lx + cos(new_angle) * line_thickness * 2 * view.zoom, ly + sin(new_angle) * line_thickness * 2 * view.zoom, ribstate.fill_col);
					  add_ribbon_vertex(lx, ly, ribstate.fill_col);

					  finish_bloom_ribbon(); // no need to finish the non-bloom ribbon. Since the bloom ribbon and the non-bloom ribbon are on different layers, can mix them if needed.

//					  bloom_circle(2, lx, ly, colours.bloom_centre [cl->colour] [shade], colours.bloom_edge [cl->colour] [0], bloom_thickness);

						}


  }
  break; // end CLOUD_HARVEST_LINE

    case CLOUD_REPAIR_LINE:
//    case CLOUD_BUILD_LINE:
    {
// first check the proc that produced the line still exists:
    if (w.proc[cl->data[0]].exists <= 0
					|| w.proc[cl->data[0]].created_timestamp != cl->associated_proc_timestamp)
						break;
// assume:
//  data[0] is associated proc index
//  data[1] is harvest object index

				float built_core_x = al_fixtof(cl->position.x - view.camera_x + cl->speed.x*cl_time) * view.zoom;
				float built_core_y = al_fixtof(cl->position.y - view.camera_y + cl->speed.y*cl_time) * view.zoom;

    built_core_x += view.window_x_unzoomed / 2;
    built_core_y += view.window_y_unzoomed / 2;
//    al_fixed vertex_x_fixed = w.proc[cl->data[0]].position.x + fixed_xpart(w.proc[cl->data[0]].angle + w.proc[cl->data[0]].nshape_ptr->object_angle_fixed [cl->data[1]], w.proc[cl->data[0]].nshape_ptr->object_dist_fixed [cl->data[1]]);
//    al_fixed vertex_y_fixed = w.proc[cl->data[0]].position.y + fixed_ypart(w.proc[cl->data[0]].angle + w.proc[cl->data[0]].nshape_ptr->object_angle_fixed [cl->data[1]], w.proc[cl->data[0]].nshape_ptr->object_dist_fixed [cl->data[1]]);
    float vx, vy;
    vx = al_fixtof(w.proc[cl->data[0]].position.x - view.camera_x) * view.zoom + (view.window_x_unzoomed / 2);
    vx += (cos(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * dshape[w.proc[cl->data[0]].shape].link_object_dist [cl->data[1]]) * view.zoom;
    vy = al_fixtof(w.proc[cl->data[0]].position.y - view.camera_y) * view.zoom + (view.window_y_unzoomed / 2);
    vy += (sin(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * dshape[w.proc[cl->data[0]].shape].link_object_dist [cl->data[1]]) * view.zoom;


//    float total_line_length = hypot(built_core_y - vy, built_core_x - vx);
//    float line_length_unzoomed = hypot(al_fixtoi(cl->position.y - vertex_y_fixed), al_fixtoi(cl->position.x - vertex_x_fixed));


    int line_time = w.world_time - cl->created_timestamp;

    float angle_to_target = atan2(built_core_y - vy, built_core_x - vx);

//    vx -= cos(angle_from_target) * (total_line_length / 16 * line_time);
//    vy -= sin(angle_from_target) * (total_line_length / 16 * line_time);


//				float line_thickness = (float) (HARVEST_LINE_TIME + line_time) / 4;
				float line_thickness = 2;//(float) (33 - line_time) * 0.1;// * view.zoom;// / 2;

//				if (line_time >= 8)
//					line_thickness = 18 - line_time;//*2;

//				if (cl->type == CLOUD_REPAIR_LINE)
//				{
     shade = (32 - line_time) / 2;//(line_time * 2);
//				}
//				 else
//      shade = 32 - (line_time * 2);

    float angle_width = (PI / 2);// * (shade + 10) / 10.0;
//    float angle_width = (PI / 2) * 0.06;

   		start_ribbon(3,
																		vx - cos(angle_to_target) * line_thickness * 3 * view.zoom, vy - sin(angle_to_target) * line_thickness * 3 * view.zoom,
																		vx + cos(angle_to_target + PI/2) * line_thickness * 3 * view.zoom, vy + sin(angle_to_target+PI/2) * line_thickness * 3 * view.zoom,
																		colours.packet [cl->colour] [shade]);

					add_ribbon_vertex(vx + cos(angle_to_target - PI/2) * line_thickness * 3 * view.zoom, vy + sin(angle_to_target - PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);

     float far_dist = hypot(al_fixtoi(cl->position.y - w.proc[cl->data[0]].position.y), al_fixtoi(cl->position.x - w.proc[cl->data[0]].position.x));

//     if (far_dist > 400)// - (shade * 4))
//						far_dist = 400;// - (shade * 4);
//					if (far_dist < 20)
//						far_dist = 20;

//					float far_dist = 255;//hypot(;

far_dist = 12 + (shade);//*= 0.1;
//far_dist = 24;

//					add_ribbon_vertex(vx + cos(angle_to_target + angle_width) * far_dist * view.zoom, vy + sin(angle_to_target + angle_width) * far_dist * view.zoom, colours.packet [cl->colour] [0]);
//					add_ribbon_vertex(vx + cos(angle_to_target - angle_width) * far_dist * view.zoom, vy + sin(angle_to_target - angle_width) * far_dist * view.zoom, colours.packet [cl->colour] [0]);
					add_ribbon_vertex(built_core_x + cos(angle_to_target + angle_width) * far_dist * view.zoom, built_core_y + sin(angle_to_target + angle_width) * far_dist * view.zoom, colours.packet [cl->colour] [0]);
					add_ribbon_vertex(built_core_x + cos(angle_to_target - angle_width) * far_dist * view.zoom, built_core_y + sin(angle_to_target - angle_width) * far_dist * view.zoom, colours.packet [cl->colour] [0]);

//					float bloom_thickness = line_thickness * 10 * view.zoom;

//					bloom_circle(2, built_core_x, built_core_y, colours.bloom_centre [cl->colour] [shade], colours.bloom_edge [cl->colour] [0], bloom_thickness);
    }
					break;
/*




    int line_segments = (total_line_length / 16) + 1;
    float line_segment_length_zoomed = (total_line_length / line_segments);// * view.zoom; - zoom has already been applied

    float lx, ly;

    seed_drand(cl->created_timestamp + c);

//    int line_time = w.world_time - cl->created_timestamp;

    float oscil_angle = PI + (cl->created_timestamp + line_time) * 0.01;
    float oscil_angle_inc = (5 + drand(5, 1)) * -0.03;

    float oscil_angle2 = PI + (cl->created_timestamp + line_time) * 0.02;
    float oscil_angle2_inc = (5 + drand(5, 1)) * -0.01;

//				float line_thickness = 9;//(float) (HARVEST_LINE_TIME - line_time) / 8;
				float step_x = cos(angle_from_target) * line_segment_length_zoomed;
				float step_y = sin(angle_from_target) * line_segment_length_zoomed;

   		start_ribbon(3,
																		built_core_x - cos(angle_from_target) * line_thickness * 3 * view.zoom, built_core_y - sin(angle_from_target) * line_thickness * 3 * view.zoom,
																		built_core_x - cos(angle_from_target) * line_thickness * 3 * view.zoom, built_core_y - sin(angle_from_target) * line_thickness * 3 * view.zoom,
																		colours.packet [cl->colour] [shade]);


					add_ribbon_vertex(built_core_x + cos(angle_from_target - PI/2) * line_thickness * 3 * view.zoom, built_core_y + sin(angle_from_target - PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);
					add_ribbon_vertex(built_core_x + cos(angle_from_target + PI/2) * line_thickness * 3 * view.zoom, built_core_y + sin(angle_from_target + PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);

					float bloom_thickness = line_thickness * 10 * view.zoom;

					bloom_circle(2, built_core_x, built_core_y, colours.bloom_centre [cl->colour] [shade], colours.bloom_edge [cl->colour] [0], bloom_thickness);

     start_bloom_ribbon(2,
																								built_core_x, built_core_y,
																								built_core_x, built_core_y,
																								built_core_x, built_core_y,
//																								well_x + cos(angle_from_well - PI/2) * bloom_thickness, well_y + sin(angle_from_well - PI/2) * bloom_thickness,
//																								well_x + cos(angle_from_well + PI/2) * bloom_thickness, well_y + sin(angle_from_well + PI/2) * bloom_thickness,
																								colours.bloom_centre [cl->colour] [shade / 2], colours.bloom_edge [cl->colour] [0]);

    seed_drand(cl->created_timestamp + c);

    for (i = 1; i < line_segments; i ++)
				{
					float side_displacement = cos(oscil_angle) * 64.0;
					side_displacement += cos(oscil_angle2) * 64.0;
					side_displacement += (drand(13, 1) - 6) * (line_time * 0.15);
					if (i < 8)
						side_displacement *= (i / 8.0);
					  else
							{
								if (i > line_segments - 8
								&& (line_segments - i) > 0)
   						side_displacement *= (line_segments - i) / 8.0;
							}
					side_displacement *= view.zoom;
					lx = built_core_x + step_x * i + cos(angle_from_target - PI/2) * side_displacement;
					ly = built_core_y + step_y * i + sin(angle_from_target - PI/2) * side_displacement;

					add_ribbon_vertex(lx + cos(angle_from_target - PI/2) * line_thickness * view.zoom, ly + sin(angle_from_target - PI/2) * line_thickness * view.zoom, ribstate.fill_col);
					add_ribbon_vertex(lx + cos(angle_from_target + PI/2) * line_thickness * view.zoom, ly + sin(angle_from_target + PI/2) * line_thickness * view.zoom, ribstate.fill_col);

     add_bloom_ribbon_vertices(
																								lx, ly,
																								lx + cos(angle_from_target - PI/2) * bloom_thickness, ly + sin(angle_from_target - PI/2) * bloom_thickness,
																								lx + cos(angle_from_target + PI/2) * bloom_thickness, ly + sin(angle_from_target + PI/2) * bloom_thickness);

     oscil_angle += oscil_angle_inc;
     oscil_angle2 += oscil_angle2_inc;
				}

					add_ribbon_vertex(vx + cos(angle_from_target - PI/2) * line_thickness * 3 * view.zoom, vy + sin(angle_from_target - PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);
					add_ribbon_vertex(vx + cos(angle_from_target + PI/2) * line_thickness * 3 * view.zoom, vy + sin(angle_from_target + PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);

     add_bloom_ribbon_vertices(
																								vx, vy,
																								vx + cos(angle_from_target - PI/2) * bloom_thickness, vy + sin(angle_from_target - PI/2) * bloom_thickness,
																								vx + cos(angle_from_target + PI/2) * bloom_thickness, vy + sin(angle_from_target + PI/2) * bloom_thickness);

					add_ribbon_vertex(vx + cos(angle_from_target) * line_thickness * 3 * view.zoom, vy + sin(angle_from_target) * line_thickness * 3 * view.zoom, ribstate.fill_col);

					finish_bloom_ribbon(); // no need to finish the non-bloom ribbon. Since the bloom ribbon and the non-bloom ribbon are on different layers, can mix them if needed.

					bloom_circle(2, vx, vy, colours.bloom_centre [cl->colour] [shade], colours.bloom_edge [cl->colour] [0], bloom_thickness);

  }
  break; // end CLOUD_HARVEST_LINE

*/
/*

//    case CLOUD_REPAIR_LINE:
    case CLOUD_BUILD_LINE:
    {
// first check the proc that produced the line still exists:
    if (w.proc[cl->data[0]].exists <= 0
					|| w.proc[cl->data[0]].created_timestamp != cl->associated_proc_timestamp)
						break;
// assume:
//  data[0] is associated proc index
//  data[1] is harvest object index

				float built_core_x = al_fixtof(cl->position.x - view.camera_x + cl->speed.x*cl_time) * view.zoom;
				float built_core_y = al_fixtof(cl->position.y - view.camera_y + cl->speed.y*cl_time) * view.zoom;

    built_core_x += view.window_x_unzoomed / 2;
    built_core_y += view.window_y_unzoomed / 2;
//    al_fixed vertex_x_fixed = w.proc[cl->data[0]].position.x + fixed_xpart(w.proc[cl->data[0]].angle + w.proc[cl->data[0]].nshape_ptr->object_angle_fixed [cl->data[1]], w.proc[cl->data[0]].nshape_ptr->object_dist_fixed [cl->data[1]]);
//    al_fixed vertex_y_fixed = w.proc[cl->data[0]].position.y + fixed_ypart(w.proc[cl->data[0]].angle + w.proc[cl->data[0]].nshape_ptr->object_angle_fixed [cl->data[1]], w.proc[cl->data[0]].nshape_ptr->object_dist_fixed [cl->data[1]]);
    float vx, vy;
    vx = al_fixtof(w.proc[cl->data[0]].position.x - view.camera_x) * view.zoom + (view.window_x_unzoomed / 2);
    vx += (cos(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * dshape[w.proc[cl->data[0]].shape].link_object_dist [cl->data[1]]) * view.zoom;
    vy = al_fixtof(w.proc[cl->data[0]].position.y - view.camera_y) * view.zoom + (view.window_y_unzoomed / 2);
    vy += (sin(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * dshape[w.proc[cl->data[0]].shape].link_object_dist [cl->data[1]]) * view.zoom;


    float total_line_length = hypot(built_core_y - vy, built_core_x - vx);
//    float line_length_unzoomed = hypot(al_fixtoi(cl->position.y - vertex_y_fixed), al_fixtoi(cl->position.x - vertex_x_fixed));


    int line_time = w.world_time - cl->created_timestamp;

    float angle_from_target = atan2(vy - built_core_y, vx - built_core_x);

//    vx -= cos(angle_from_target) * (total_line_length / 16 * line_time);
//    vy -= sin(angle_from_target) * (total_line_length / 16 * line_time);


//				float line_thickness = (float) (HARVEST_LINE_TIME + line_time) / 4;
				float line_thickness = (float) (33 - line_time) * view.zoom * 0.5;// / 2;

//				if (line_time >= 8)
//					line_thickness = 18 - line_time;// *2;

				if (cl->type == CLOUD_REPAIR_LINE)
				{
//					line_thickness *= 0.5;
     shade = 16 - (line_time);
				}
				 else
      shade = 32 - (line_time * 2);






    int line_segments = (total_line_length / 16) + 1;
    float line_segment_length_zoomed = (total_line_length / line_segments);// * view.zoom; - zoom has already been applied

    float lx, ly;

    seed_drand(cl->created_timestamp + c);

//    int line_time = w.world_time - cl->created_timestamp;

    float oscil_angle = PI + (cl->created_timestamp + line_time) * 0.01;
    float oscil_angle_inc = (5 + drand(5, 1)) * -0.03;

    float oscil_angle2 = PI + (cl->created_timestamp + line_time) * 0.02;
    float oscil_angle2_inc = (5 + drand(5, 1)) * -0.01;

//				float line_thickness = 9;//(float) (HARVEST_LINE_TIME - line_time) / 8;
				float step_x = cos(angle_from_target) * line_segment_length_zoomed;
				float step_y = sin(angle_from_target) * line_segment_length_zoomed;

   		start_ribbon(3,
																		built_core_x - cos(angle_from_target) * line_thickness * 3 * view.zoom, built_core_y - sin(angle_from_target) * line_thickness * 3 * view.zoom,
																		built_core_x - cos(angle_from_target) * line_thickness * 3 * view.zoom, built_core_y - sin(angle_from_target) * line_thickness * 3 * view.zoom,
																		colours.packet [cl->colour] [shade]);


					add_ribbon_vertex(built_core_x + cos(angle_from_target - PI/2) * line_thickness * 3 * view.zoom, built_core_y + sin(angle_from_target - PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);
					add_ribbon_vertex(built_core_x + cos(angle_from_target + PI/2) * line_thickness * 3 * view.zoom, built_core_y + sin(angle_from_target + PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);

					float bloom_thickness = line_thickness * 10 * view.zoom;

					bloom_circle(2, built_core_x, built_core_y, colours.bloom_centre [cl->colour] [shade], colours.bloom_edge [cl->colour] [0], bloom_thickness);

     start_bloom_ribbon(2,
																								built_core_x, built_core_y,
																								built_core_x, built_core_y,
																								built_core_x, built_core_y,
//																								well_x + cos(angle_from_well - PI/2) * bloom_thickness, well_y + sin(angle_from_well - PI/2) * bloom_thickness,
//																								well_x + cos(angle_from_well + PI/2) * bloom_thickness, well_y + sin(angle_from_well + PI/2) * bloom_thickness,
																								colours.bloom_centre [cl->colour] [shade / 2], colours.bloom_edge [cl->colour] [0]);

    seed_drand(cl->created_timestamp + c);

    for (i = 1; i < line_segments; i ++)
				{
					float side_displacement = cos(oscil_angle) * 64.0;
					side_displacement += cos(oscil_angle2) * 64.0;
					side_displacement += (drand(13, 1) - 6) * (line_time * 0.15);
					if (i < 8)
						side_displacement *= (i / 8.0);
					  else
							{
								if (i > line_segments - 8
								&& (line_segments - i) > 0)
   						side_displacement *= (line_segments - i) / 8.0;
							}
					side_displacement *= view.zoom;
					lx = built_core_x + step_x * i + cos(angle_from_target - PI/2) * side_displacement;
					ly = built_core_y + step_y * i + sin(angle_from_target - PI/2) * side_displacement;

					add_ribbon_vertex(lx + cos(angle_from_target - PI/2) * line_thickness * view.zoom, ly + sin(angle_from_target - PI/2) * line_thickness * view.zoom, ribstate.fill_col);
					add_ribbon_vertex(lx + cos(angle_from_target + PI/2) * line_thickness * view.zoom, ly + sin(angle_from_target + PI/2) * line_thickness * view.zoom, ribstate.fill_col);

     add_bloom_ribbon_vertices(
																								lx, ly,
																								lx + cos(angle_from_target - PI/2) * bloom_thickness, ly + sin(angle_from_target - PI/2) * bloom_thickness,
																								lx + cos(angle_from_target + PI/2) * bloom_thickness, ly + sin(angle_from_target + PI/2) * bloom_thickness);

     oscil_angle += oscil_angle_inc;
     oscil_angle2 += oscil_angle2_inc;
				}

					add_ribbon_vertex(vx + cos(angle_from_target - PI/2) * line_thickness * 3 * view.zoom, vy + sin(angle_from_target - PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);
					add_ribbon_vertex(vx + cos(angle_from_target + PI/2) * line_thickness * 3 * view.zoom, vy + sin(angle_from_target + PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);

     add_bloom_ribbon_vertices(
																								vx, vy,
																								vx + cos(angle_from_target - PI/2) * bloom_thickness, vy + sin(angle_from_target - PI/2) * bloom_thickness,
																								vx + cos(angle_from_target + PI/2) * bloom_thickness, vy + sin(angle_from_target + PI/2) * bloom_thickness);

					add_ribbon_vertex(vx + cos(angle_from_target) * line_thickness * 3 * view.zoom, vy + sin(angle_from_target) * line_thickness * 3 * view.zoom, ribstate.fill_col);

					finish_bloom_ribbon(); // no need to finish the non-bloom ribbon. Since the bloom ribbon and the non-bloom ribbon are on different layers, can mix them if needed.

					bloom_circle(2, vx, vy, colours.bloom_centre [cl->colour] [shade], colours.bloom_edge [cl->colour] [0], bloom_thickness);

  }
  break; // end CLOUD_BUILD_LINE
*/


		case CLOUD_BUILD_LINE:
//		case CLOUD_REPAIR_LINE:
		{
// first check the proc that produced the line still exists:
    if (w.proc[cl->data[0]].exists <= 0
					|| w.proc[cl->data[0]].created_timestamp != cl->associated_proc_timestamp)
						break;
// assume:
//  data[0] is associated proc index
//  data[1] is harvest object index

				float built_core_x = al_fixtof(cl->position.x - view.camera_x + cl->speed.x*cl_time) * view.zoom;
				float built_core_y = al_fixtof(cl->position.y - view.camera_y + cl->speed.y*cl_time) * view.zoom;

    built_core_x += view.window_x_unzoomed / 2;
    built_core_y += view.window_y_unzoomed / 2;
//    al_fixed vertex_x_fixed = w.proc[cl->data[0]].position.x + fixed_xpart(w.proc[cl->data[0]].angle + w.proc[cl->data[0]].nshape_ptr->object_angle_fixed [cl->data[1]], w.proc[cl->data[0]].nshape_ptr->object_dist_fixed [cl->data[1]]);
//    al_fixed vertex_y_fixed = w.proc[cl->data[0]].position.y + fixed_ypart(w.proc[cl->data[0]].angle + w.proc[cl->data[0]].nshape_ptr->object_angle_fixed [cl->data[1]], w.proc[cl->data[0]].nshape_ptr->object_dist_fixed [cl->data[1]]);
    float vx, vy;
    vx = al_fixtof(w.proc[cl->data[0]].position.x - view.camera_x) * view.zoom + (view.window_x_unzoomed / 2);
    vx += (cos(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * dshape[w.proc[cl->data[0]].shape].link_object_dist [cl->data[1]]) * view.zoom;
    vy = al_fixtof(w.proc[cl->data[0]].position.y - view.camera_y) * view.zoom + (view.window_y_unzoomed / 2);
    vy += (sin(fixed_to_radians(w.proc[cl->data[0]].angle) + dshape[w.proc[cl->data[0]].shape].link_object_angle [cl->data[1]]) * dshape[w.proc[cl->data[0]].shape].link_object_dist [cl->data[1]]) * view.zoom;

    float total_line_length = hypot(built_core_y - vy, built_core_x - vx);


    int line_time = w.world_time - cl->created_timestamp;
//    shade = 32 - (line_time * 2);
//    if (shade > CLOUD_SHADES - 1)
//					shade = CLOUD_SHADES - 1;

    float angle_from_target = atan2(vy - built_core_y, vx - built_core_x);

    vx -= cos(angle_from_target) * (total_line_length / 16 * line_time);
    vy -= sin(angle_from_target) * (total_line_length / 16 * line_time);


//				float line_thickness = (float) (HARVEST_LINE_TIME + line_time) / 4;
				float line_thickness = (float) (2 + line_time);// / 2;

//				if (line_time >= 8)
//					line_thickness = 18 - line_time;// *2;

//				if (cl->type == CLOUD_REPAIR_LINE)
//				{
//					line_thickness *= 0.5;
//     shade = 16 - (line_time);
//				}
//				 else
      shade = 32 - (line_time * 2);

   		start_ribbon(3,
																		built_core_x - cos(angle_from_target) * line_thickness * 3 * view.zoom, built_core_y - sin(angle_from_target) * line_thickness * 3 * view.zoom,
																		built_core_x + cos(angle_from_target - PI/2) * line_thickness * 3 * view.zoom, built_core_y + sin(angle_from_target - PI/2) * line_thickness * 3 * view.zoom,
																		colours.packet [cl->colour] [shade]);

					add_ribbon_vertex(built_core_x + cos(angle_from_target + PI/2) * line_thickness * 3 * view.zoom, built_core_y + sin(angle_from_target + PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);

				line_thickness = (float) (HARVEST_LINE_TIME - line_time) / 16;

				add_ribbon_vertex(vx + cos(angle_from_target - PI/2) * line_thickness * 3 * view.zoom, vy + sin(angle_from_target - PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);
				add_ribbon_vertex(vx + cos(angle_from_target + PI/2) * line_thickness * 3 * view.zoom, vy + sin(angle_from_target + PI/2) * line_thickness * 3 * view.zoom, ribstate.fill_col);

				add_ribbon_vertex(vx + cos(angle_from_target) * line_thickness * 3 * view.zoom, vy + sin(angle_from_target) * line_thickness * 3 * view.zoom, ribstate.fill_col);

		}
		break; // end CLOUD_BUILD_LINE

  }

//  al_draw_bitmap_region(clouds_bmp, clouds_index[CLOUDS_BMP_BLOT_R_1].x, clouds_index[CLOUDS_BMP_BLOT_R_1].y, clouds_index[CLOUDS_BMP_BLOT_R_1].w, clouds_index[CLOUDS_BMP_BLOT_R_1].h, x - clouds_index[CLOUDS_BMP_BLOT_R_1].centre_x, y - clouds_index[CLOUDS_BMP_BLOT_R_1].centre_y, 0);
//  check_vbuf();

 }

 draw_vbuf(); // sends poly_buffer and line_buffer to the screen

#ifndef RECORDING_VIDEO_2






  int bubble_core_index = bubble_list_index; // bubble_list_index set above during main core drawing loop


		while(bubble_core_index != -1)
		{

// this list can include existing cores added by the proc drawing code above,
//  or non-existing (but still deallocating) cores added by the cloud drawing code.
//  (deallocating cores don't exist, but data in their core_struct should still
//   be valid for use in things like this)

			 int draw_triangle;
			 if (w.core[bubble_core_index].exists)
					draw_triangle = 1;
				  else
							draw_triangle = 0;

    draw_text_bubble(w.core[bubble_core_index].bubble_x,
																					w.core[bubble_core_index].bubble_y,
																					w.world_time - w.core[bubble_core_index].bubble_text_time_adjusted,
																					w.core[bubble_core_index].player_index,
																					w.core[bubble_core_index].bubble_text_length,
																					w.core[bubble_core_index].bubble_text,
																					draw_triangle);

				bubble_core_index = w.core[bubble_core_index].bubble_list;

	}
#endif

/*
				int bubble_shade;
				int bubble_text_shade;
				int bubble_time = w.world_time - w.core[bubble_core_index].bubble_text_time_adjusted;
				float bubble_size_reduce;
				bubble_shade = bubble_time;
				bubble_shade = 16;
				float adjusted_bubble_x = w.core[bubble_core_index].bubble_x - 20;
			 if (bubble_time < 16)
				{
					bubble_shade = 31 - (bubble_time);
					bubble_size_reduce = (bubble_shade - 16) * -0.2;
					adjusted_bubble_x += bubble_size_reduce;
				}
				 else
					{
			   if (bubble_time > BUBBLE_TOTAL_TIME - 16)
						{
					  bubble_shade = BUBBLE_TOTAL_TIME - bubble_time;
  					bubble_size_reduce = (16 - bubble_shade) * 0.3;
  					adjusted_bubble_x += bubble_size_reduce;

						}
						 else
								bubble_size_reduce = 0;
					}

				if (bubble_shade < 0)
						bubble_shade = 0;



 add_menu_button(adjusted_bubble_x - (10 - bubble_size_reduce),
																	w.core[bubble_core_index].bubble_y - (10 - bubble_size_reduce),
																	adjusted_bubble_x + (10) + w.core[bubble_core_index].bubble_text_length * 7,
																	w.core[bubble_core_index].bubble_y + (20 - bubble_size_reduce),
//																	colours.packet [pr->player_index] [bubble_shade],
																	colours.packet [w.core[bubble_core_index].player_index] [bubble_shade],
																	3, 8);

	add_triangle(4,
														adjusted_bubble_x,
														w.core[bubble_core_index].bubble_y + 22,
														adjusted_bubble_x + 19,
														w.core[bubble_core_index].bubble_y + 22,
														adjusted_bubble_x + 19,
														w.core[bubble_core_index].bubble_y + 52,
														colours.packet [w.core[bubble_core_index].player_index] [bubble_shade]);

				int text_shade = bubble_shade * 2;
				if (text_shade > 31)
					text_shade = 31;

				al_draw_textf(font[FONT_SQUARE].fnt, colours.packet [w.core[bubble_core_index].player_index] [text_shade], adjusted_bubble_x, w.core[bubble_core_index].bubble_y, ALLEGRO_ALIGN_LEFT, "%s", w.core[bubble_core_index].bubble_text);

				bubble_core_index = w.core[bubble_core_index].bubble_list;

	}
*/







 draw_vbuf(); // sends poly_buffer and line_buffer to the screen


#define DRAW_VISION_BLOCKS

#ifdef DRAW_VISION_BLOCKS


//struct vision_area_struct* vision_area;

  vision_check_for_display(); // sets up block data for visibility (fog of war)

  if (game.phase == GAME_PHASE_PREGAME)
			special_visible_area(w.player[game.user_player_index].spawn_position);
/*
  for (i = -1; i < screen_width_in_blocks; i ++)
  {

  bx = ((camera_x - camera_x_mod_block) / BLOCK_SIZE_PIXELS) - ((camera_x_zoomed - camera_x_zoomed_mod_block) / BLOCK_SIZE_PIXELS) + i;

   if (bx < 1)
    continue;
   if (bx >= w.blocks.x - 1)
    break;

 check_vbuf();

   for (j = -1; j < screen_height_in_blocks; j ++)
   {
    by = ((camera_y - camera_y_mod_block) / BLOCK_SIZE_PIXELS) - ((camera_y_zoomed - camera_y_zoomed_mod_block) / BLOCK_SIZE_PIXELS) + j;

    if (by < 1)
     continue;
    if (by >= w.blocks.y - 1)
     break;

    	if (bx < 2 || by < 2
						|| bx >= w.blocks.x - 2
						|| by >= w.blocks.y - 2)
							continue;

    bx2 = ((i * BLOCK_SIZE_PIXELS) - camera_offset_x) * view.zoom;
    by2 = ((j * BLOCK_SIZE_PIXELS) - camera_offset_y) * view.zoom;
*/


 al_set_target_bitmap(vision_mask);

 if (game.vision_mask)
	{
  i = 0;

  while (i < deferred_data_wells)
		{

			if (w.vision_block[deferred_data_well_draw_i [i]][deferred_data_well_draw_j [i]].clear_time == w.world_time)
			 continue;

			float alpha_ch;

							if (w.vision_block[deferred_data_well_draw_i [i]][deferred_data_well_draw_j [i]].proximity_time == w.world_time
								&& w.vision_block[deferred_data_well_draw_i [i]][deferred_data_well_draw_j [i]].clear_time != w.world_time)
       {
        alpha_ch = w.vision_block[deferred_data_well_draw_i [i]][deferred_data_well_draw_j [i]].proximity;// * 0.1;
        if (alpha_ch > 255)
	        alpha_ch = 255;
       }
        else
									alpha_ch = 255;

//	alpha_ch = 255 - alpha_ch;

       draw_data_well(deferred_data_well_draw_i [i],
																						deferred_data_well_draw_j [i],
																						deferred_data_well_draw_backbl [i],
																						top_left_corner_x,
																						top_left_corner_y,
																						-1,
																						alpha_ch);



			i++;
		};

 draw_vbuf();

	}


 al_set_blender(ALLEGRO_ADD, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ALPHA);
 al_set_separate_blender(ALLEGRO_ADD, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ALPHA,
   ALLEGRO_DEST_MINUS_SRC, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ALPHA);


  for (i = min_block_x; i < max_block_x; i ++)
  {

   bx = i; //base_bx + i;


   if (bx < 0)
    continue;
   if (bx >= w.blocks.x)
    break;

   check_vbuf();

//   int base_by = ((camera_y - camera_y_mod_block) / BLOCK_SIZE_PIXELS) - ((camera_y_zoomed - camera_y_zoomed_mod_block) / BLOCK_SIZE_PIXELS);

   for (j = min_block_y; j < max_block_y; j ++)
   {
    by = j;//base_by + j;

    if (by < 0)
     continue;
    if (by >= w.blocks.y)
     break;

//    fprintf(stdout, "[bx,by %i,%i]", bx, by);

       bx2 = top_left_corner_x [0] + (BLOCK_SIZE_PIXELS * view.zoom) * (i); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
       by2 = top_left_corner_y [0] + (BLOCK_SIZE_PIXELS * view.zoom) * (j); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);


//    if (w.vision_area [bx] [by].vision_time < w.world_time)
    if (//w.block[bx][by].vision_block_proximity_time == w.world_time
 				w.vision_block[bx][by].clear_time == w.world_time)
    {
//    	int shadow_prop = w.world_time - w.vision_area [bx] [by].vision_time;

//    	if (shadow_prop >= 128)
//					{
// TO DO: optimise by drawing whole rows at a time, if possible
				int m = vbuf.vertex_pos_triangle;
				const float size_b = (BLOCK_SIZE_PIXELS+10) * view.zoom;
				add_tri_vertex(bx2 - 10 * view.zoom, by2 - 10 * view.zoom, al_map_rgba(0,0,0,0));
				add_tri_vertex(bx2 + size_b, by2 - 10 * view.zoom, al_map_rgba(0,0,0,0));
				add_tri_vertex(bx2 + size_b, by2 + size_b, al_map_rgba(0,0,0,0));
				add_tri_vertex(bx2 - 10 * view.zoom, by2 + size_b, al_map_rgba(0,0,0,0));

				construct_triangle(4, m, m+1, m+2);
				construct_triangle(4, m+2, m+3, m);

			}
					 else
						{
							if (w.vision_block[bx][by].proximity_time == w.world_time
								&& w.vision_block[bx][by].clear_time != w.world_time)
							{

//									float block_size = 0.95;//BLOCK_SIZE_PIXELS * 0.6; //(w.block[bx][by].vision_block_proximity [k * 2 + l] - 108) * 0.2;//(float) ((w.block[bx][by].vision_block_proximity [k * 2 + l] - (BLOCK_SIZE_PIXELS * 6))) * view.zoom;

//									block_size *= 	w.block[bx][by].vision_block_x_shrink [k * 2 + l];

//									if (block_size < 0)
//										continue;
//									if (block_size > 60)
//										block_size = 60;


float alpha_ch = w.vision_block[bx][by].proximity;// * 0.1;
if (alpha_ch > 255)
	alpha_ch = 255;

//	alpha_ch = 255 - alpha_ch;

// block_size *= (255 - alpha_ch);
// block_size /= 120;

// if (block_size > BLOCK_SIZE_PIXELS * 0.6)
		//block_size = BLOCK_SIZE_PIXELS * 0.6;

//																						block_size *= view.zoom;
//alpha_ch /= 2;
float x_offset = 0;//BLOCK_SIZE_PIXELS * 0.5 * view.zoom;
if (j & 1)
	x_offset = BLOCK_SIZE_PIXELS * 0.5 * view.zoom; //BLOCK_SIZE_PIXELS * 0.25 * view.zoom;

add_stretched_hexagon(bx2 + x_offset, by2 + BLOCK_SIZE_PIXELS * 0.5 * view.zoom, view.zoom, al_map_rgba(0,0,0, alpha_ch)); //colours.black);

						}
						}

   }
  }



 //al_set_blender(ALLEGRO_ADD, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ALPHA);
//al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);



 draw_vbuf();

 al_set_target_bitmap(al_get_backbuffer(display));
// REMEMBER that al_set_clipping_rectangle uses width and height!!!
// al_set_clipping_rectangle(0, 0, panel[PANEL_MAIN].w, panel[PANEL_MAIN].h);
 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

 al_draw_bitmap(vision_mask,0,0,0);




#endif


#ifdef DRAW_VISION_BLOCKS2

//struct vision_area_struct* vision_area;

  vision_check_for_display(); // sets up block data for visibility (fog of war)

  if (game.phase == GAME_PHASE_PREGAME)
			special_visible_area(w.player[game.user_player_index].spawn_position);
/*
  for (i = -1; i < screen_width_in_blocks; i ++)
  {

  bx = ((camera_x - camera_x_mod_block) / BLOCK_SIZE_PIXELS) - ((camera_x_zoomed - camera_x_zoomed_mod_block) / BLOCK_SIZE_PIXELS) + i;

   if (bx < 1)
    continue;
   if (bx >= w.blocks.x - 1)
    break;

 check_vbuf();

   for (j = -1; j < screen_height_in_blocks; j ++)
   {
    by = ((camera_y - camera_y_mod_block) / BLOCK_SIZE_PIXELS) - ((camera_y_zoomed - camera_y_zoomed_mod_block) / BLOCK_SIZE_PIXELS) + j;

    if (by < 1)
     continue;
    if (by >= w.blocks.y - 1)
     break;

    	if (bx < 2 || by < 2
						|| bx >= w.blocks.x - 2
						|| by >= w.blocks.y - 2)
							continue;

    bx2 = ((i * BLOCK_SIZE_PIXELS) - camera_offset_x) * view.zoom;
    by2 = ((j * BLOCK_SIZE_PIXELS) - camera_offset_y) * view.zoom;
*/


 al_set_target_bitmap(vision_mask);
 al_set_blender(ALLEGRO_ADD, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ALPHA);
 al_set_separate_blender(ALLEGRO_ADD, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ALPHA,
   ALLEGRO_DEST_MINUS_SRC, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ALPHA);



  for (i = min_block_x; i < max_block_x; i ++)
  {

   bx = i; //base_bx + i;


   if (bx < 0)
    continue;
   if (bx >= w.blocks.x)
    break;

   check_vbuf();

//   int base_by = ((camera_y - camera_y_mod_block) / BLOCK_SIZE_PIXELS) - ((camera_y_zoomed - camera_y_zoomed_mod_block) / BLOCK_SIZE_PIXELS);

   for (j = min_block_y; j < max_block_y; j ++)
   {
    by = j;//base_by + j;

    if (by < 0)
     continue;
    if (by >= w.blocks.y)
     break;

//    fprintf(stdout, "[bx,by %i,%i]", bx, by);

       bx2 = top_left_corner_x + (BLOCK_SIZE_PIXELS * view.zoom) * (i); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
       by2 = top_left_corner_y + (BLOCK_SIZE_PIXELS * view.zoom) * (j); //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);


//    if (w.vision_area [bx] [by].vision_time < w.world_time)
    if (w.block[bx][by].vision_block_proximity_time != w.world_time
 				&& w.block[bx][by].vision_block_clear_time != w.world_time)
    {
//    	int shadow_prop = w.world_time - w.vision_area [bx] [by].vision_time;

//    	if (shadow_prop >= 128)
//					{
// TO DO: optimise by drawing whole rows at a time, if possible
					add_triangle(4,
																		bx2 - 10 * view.zoom,
																		by2 - 10 * view.zoom,
																		bx2 + (BLOCK_SIZE_PIXELS+10) * view.zoom,
																		by2 - 10 * view.zoom,
																		bx2 + (BLOCK_SIZE_PIXELS+10) * view.zoom,
																		by2 + (BLOCK_SIZE_PIXELS+10) * view.zoom,
																		colours.black);
					add_triangle(4,
																		bx2 - 10 * view.zoom,
																		by2 - 10 * view.zoom,
																		bx2 - 10 * view.zoom,
																		by2 + (BLOCK_SIZE_PIXELS+10) * view.zoom,
																		bx2 + (BLOCK_SIZE_PIXELS+10) * view.zoom,
																		by2 + (BLOCK_SIZE_PIXELS+10) * view.zoom,
																		colours.black);
			}
					 else
						{
							if (w.block[bx][by].vision_block_proximity_time == w.world_time
								&& w.block[bx][by].vision_block_clear_time != w.world_time)
							{

									float block_size = BLOCK_SIZE_PIXELS * 0.6 * view.zoom; //(w.block[bx][by].vision_block_proximity [k * 2 + l] - 108) * 0.2;//(float) ((w.block[bx][by].vision_block_proximity [k * 2 + l] - (BLOCK_SIZE_PIXELS * 6))) * view.zoom;

//									block_size *= 	w.block[bx][by].vision_block_x_shrink [k * 2 + l];

//									if (block_size < 0)
//										continue;
//									if (block_size > 60)
//										block_size = 60;


float alpha_ch = w.block[bx][by].vision_block_proximity;// * 0.5;
if (alpha_ch > 255)
	alpha_ch = 255;

 block_size *= alpha_ch;
 block_size /= 250;

//																						block_size *= view.zoom;
//alpha_ch /= 2;
float x_offset = BLOCK_SIZE_PIXELS * 0.5 * view.zoom;
if (j & 1)
	x_offset = 0; //BLOCK_SIZE_PIXELS * 0.25 * view.zoom;

add_stretched_hexagon(bx2 + x_offset, by2 + BLOCK_SIZE_PIXELS * 0.5 * view.zoom, block_size, al_map_rgba(0,0,0, alpha_ch)); //colours.black);
 //al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], bx2 + BLOCK_SIZE_PIXELS / 2, by2 +  BLOCK_SIZE_PIXELS / 2, ALLEGRO_ALIGN_CENTRE, "%f", alpha_ch);

/*
							int l;
//							float shadow_square_size = (w.world_time - w.vision_area [bx] [by].vision_time) * view.zoom * (BLOCK_SIZE_PIXELS / 4) / 128;
							for (k = 0; k < 2; k ++)
							{
								for (l = 0; l < 2; l ++)
								{
									float kx = bx2 + w.block[bx][by].vision_block_x [k * 2 + l] * view.zoom;
									float ky = by2 + w.block[bx][by].vision_block_y [k * 2 + l] * view.zoom;

									float block_size = (w.block[bx][by].vision_block_proximity [k * 2 + l] - 108) * 0.2;//(float) ((w.block[bx][by].vision_block_proximity [k * 2 + l] - (BLOCK_SIZE_PIXELS * 6))) * view.zoom;

									block_size *= 	w.block[bx][by].vision_block_x_shrink [k * 2 + l];

									if (block_size < 0)
										continue;
									if (block_size > 60)
										block_size = 60;


float alpha_ch = block_size * 8;
if (alpha_ch > 255)
	alpha_ch = 255;

																						block_size *= view.zoom;

add_orthogonal_hexagon(kx, ky, block_size, al_map_rgba(0,0,0, alpha_ch)); //colours.black);



								}
							}
*/
						}
						}

   }
  }
 draw_vbuf();

 al_set_target_bitmap(al_get_backbuffer(display));
// REMEMBER that al_set_clipping_rectangle uses width and height!!!
// al_set_clipping_rectangle(0, 0, panel[PANEL_MAIN].w, panel[PANEL_MAIN].h);
 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
 al_draw_bitmap(vision_mask,0,0,0);



#endif

//#define VISION_MASK

#ifdef VISION_MASK

 al_set_target_bitmap(vision_mask);
// al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ALPHA);
 al_set_blender(ALLEGRO_ADD, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ALPHA);

 al_set_separate_blender(ALLEGRO_ADD, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ALPHA,
   ALLEGRO_DEST_MINUS_SRC, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ALPHA);

 ALLEGRO_COLOR visible = al_map_rgba(0,0,0,0);



 for (i = w.player[game.user_player_index].core_index_start; i < w.player[game.user_player_index].core_index_end; i++)
	{
  core = &w.core[i];
  x = al_fixtof(core->core_position.x - view.camera_x) * view.zoom;
  y = al_fixtof(core->core_position.y - view.camera_y) * view.zoom;
  x += view.window_x_unzoomed / 2;
  y += view.window_y_unzoomed / 2;

#define VISIBLE_EDGE_SIZE 155
#define VISIBLE_CIRCLE_VERTICES 16


		float long_dist = (core->scan_range_float + VISIBLE_EDGE_SIZE) * view.zoom;
		float short_dist = core->scan_range_float * view.zoom;


  if (x > 0 - long_dist && x < view.window_x_unzoomed + long_dist
   && y > 0 - long_dist && y < view.window_y_unzoomed + long_dist)
  {

  	start_radial(x, y, 1, visible);
  	start_ribbon(0,
																x + short_dist, y,
																x + long_dist, y,
																visible);

  	for (j = 0; j < VISIBLE_CIRCLE_VERTICES; j++)
			{

			float v_angle = ((PI*2) / VISIBLE_CIRCLE_VERTICES) * j;
  	 add_radial_vertex(v_angle, core->scan_range_float); // add_radial_vertex applies zoom
 			add_ribbon_vertex(x + cos(v_angle) * short_dist, y + sin(v_angle) * short_dist, visible);
 			add_ribbon_vertex(x + cos(v_angle) * long_dist, y + sin(v_angle) * long_dist, colours.black);

//	 		add_ribbon_vertex(x + long_dist, y - long_dist, colours.black);

			}
  	finish_radial();

 			add_ribbon_vertex(x + short_dist, y, visible);
 			add_ribbon_vertex(x + long_dist, y, colours.black);


  }

	}

	draw_vbuf(); // draws vbuf to vision_mask

 al_set_target_bitmap(al_get_backbuffer(display));
// REMEMBER that al_set_clipping_rectangle uses width and height!!!
// al_set_clipping_rectangle(0, 0, panel[PANEL_MAIN].w, panel[PANEL_MAIN].h);
 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);


// al_draw_bitmap(vision_mask,0,0,0);

#endif

// now draw selection and similar overlays:
 if (command.select_box != 0)
	{
		add_orthogonal_rect(2,
       (control.mouse_x_world_pixels - al_fixtoi(view.camera_x)) * view.zoom + view.window_x_unzoomed / 2,
       (control.mouse_y_world_pixels - al_fixtoi(view.camera_y)) * view.zoom + view.window_y_unzoomed / 2,
       al_fixtof(command.mouse_drag_world_x - view.camera_x) * view.zoom + view.window_x_unzoomed / 2,
       al_fixtof(command.mouse_drag_world_y - view.camera_y) * view.zoom + view.window_y_unzoomed / 2,
							al_map_rgba(20,20,40,40));


		const int _m = vbuf.vertex_pos_line;
		add_line_vertex(al_fixtof(command.mouse_drag_world_x - view.camera_x) * view.zoom + view.window_x_unzoomed / 2,
						al_fixtof(command.mouse_drag_world_y - view.camera_y) * view.zoom + view.window_y_unzoomed / 2,
						colours.base_trans [COL_BLUE] [SHADE_HIGH] [TRANS_MED]);
		add_line_vertex((control.mouse_x_world_pixels - al_fixtoi(view.camera_x)) * view.zoom + view.window_x_unzoomed / 2,
						al_fixtof(command.mouse_drag_world_y - view.camera_y) * view.zoom + view.window_y_unzoomed / 2,
						colours.base_trans [COL_BLUE] [SHADE_HIGH] [TRANS_MED]);
		add_line_vertex((control.mouse_x_world_pixels - al_fixtoi(view.camera_x)) * view.zoom + view.window_x_unzoomed / 2,
						(control.mouse_y_world_pixels - al_fixtoi(view.camera_y)) * view.zoom + view.window_y_unzoomed / 2,
						colours.base_trans [COL_BLUE] [SHADE_HIGH] [TRANS_MED]);
		add_line_vertex(al_fixtof(command.mouse_drag_world_x - view.camera_x) * view.zoom + view.window_x_unzoomed / 2,
						(control.mouse_y_world_pixels - al_fixtoi(view.camera_y)) * view.zoom + view.window_y_unzoomed / 2,
						colours.base_trans [COL_BLUE] [SHADE_HIGH] [TRANS_MED]);
		construct_line(2, _m, _m+1);
		construct_line(2, _m+1, _m+2);
		construct_line(2, _m+2, _m+3);
		construct_line(2, _m+3, _m);

	}

/*

	if (command.selected_core [0] != SELECT_TERMINATE)
	{
  for (i = 0; i < SELECT_MAX; i ++)
		{
			if (command.selected_core [i] >= 0)
			{
				draw_command_marker(command.selected_core [i]);
				break; // only draw the first
			}
			if (command.selected_core [i] == SELECT_TERMINATE)
				break; // could be empty selected_core array if all selected cores destroyed.
		}
		if (command.selected_core [1] == SELECT_TERMINATE // just one core selected
			&& command.selected_member != -1)
		{
			selected_member
		}
	}
*/

 if (panel[PANEL_BCODE].open
		&& bcp_state.bcp_mode == BCP_MODE_PROCESS)
	{

    x = al_fixtof(w.core[bcp_state.watch_core_index].core_position.x - view.camera_x) * view.zoom;
    y = al_fixtof(w.core[bcp_state.watch_core_index].core_position.y - view.camera_y) * view.zoom;

    x += view.window_x_unzoomed / 2;
    y += view.window_y_unzoomed / 2;

    if (x > -200 && x < view.window_x_unzoomed + 200
     && y > -200 && y < view.window_y_unzoomed + 200)
    {

    select_arrows(5,
																		x,
																		y,
				  												game.total_time * 0.02 + PI/5,
																		(81.5) * view.zoom, // radius
																  12,
																  PI/5,
																  18,
																  colours.base [COL_ORANGE] [SHADE_MAX]);
    }

    if (bcp_state.mouseover_time == inter.running_time - 1
				 &&	bcp_state.mouseover_type == BCP_MOUSEOVER_CORE_INDEX
				 && bcp_state.mouseover_value >= 0 && bcp_state.mouseover_value < w.max_cores)
				{


     x = al_fixtof(w.core[bcp_state.mouseover_value].core_position.x - view.camera_x) * view.zoom;
     y = al_fixtof(w.core[bcp_state.mouseover_value].core_position.y - view.camera_y) * view.zoom;

     x += view.window_x_unzoomed / 2;
     y += view.window_y_unzoomed / 2;

     if (x > -200 && x < view.window_x_unzoomed + 200
      && y > -200 && y < view.window_y_unzoomed + 200)
     {

     select_arrows(3,
																		 x,
																		 y,
				  												 game.total_time * 0.02 + PI/5,
																		 (83.5) * view.zoom, // radius
																   16,
																   PI/3,
																   19,
																   colours.base [COL_CYAN] [SHADE_MAX]);
     }

				}
	}

//#ifndef RECORDING_VIDEO
// if only one core is selected, and it's the first in the select array, show its targets:
// * actually do this just if one core is selected
	if (command.select_mode == SELECT_MODE_SINGLE_CORE)
	{
		draw_command_marker(command.selected_core [0]);
	}
	 else
		{
	  if (command.select_mode == SELECT_MODE_MULTI_CORE)
	  {
	  	int sc = 0;
	  	while(command.selected_core [sc] != SELECT_TERMINATE)
				{
					if (command.selected_core [sc] != SELECT_EMPTY)
					{
		    draw_command_marker(command.selected_core [sc]);
		    break;
					}
					sc++;
				}

	  }
		}
//#endif

int scaled_box_w = scaleUI_x(FONT_SQUARE,250);
int scaled_box_line_h = scaleUI_y(FONT_SQUARE,15);
int scaled_box_header_h = scaleUI_y(FONT_SQUARE,25);

#define BOX_W scaled_box_w
#define BOX_LINE_H scaled_box_line_h
#define BOX_HEADER_H scaled_box_header_h

int scaled_power_box_w = scaleUI_x(FONT_SQUARE,200);
int scaled_power_box_h = scaleUI_y(FONT_SQUARE,80);

// stress/power graph
#define POWER_BOX_W scaled_power_box_w
//#define STRESS_BOX_H 60
#define POWER_BOX_H scaled_power_box_h


	float box_x = view.window_x_unzoomed - BOX_W - 10;
	float box_x2 = box_x + BOX_W;
 int text_x = box_x + 11;
 int text_x2 = box_x2 - scaleUI_x(FONT_SQUARE,9);
	float box_y = 5;
	float process_box_y;
	int box_lines;
	float	box_h = BOX_HEADER_H + BOX_LINE_H * 3 + 8;
 int text_y = box_y + BOX_HEADER_H + 7;// + BOX_LINE_H;

#ifndef RECORDING_VIDEO_2


if (inter.block_mode_button_area_scrolling)
{
 add_menu_button(view.window_x_unzoomed - MODE_BUTTON_SCROLL_BLOCK_W,
																	-8,
																	view.window_x_unzoomed + 8,
																	MODE_BUTTON_SCROLL_BLOCK_H,
																	colours.base_trans [COL_BLUE] [SHADE_MAX] [TRANS_FAINT], 5, 5);
}


 add_menu_button(box_x, box_y, box_x2, box_y + box_h, colours.base_trans [COL_BLUE] [SHADE_MED] [TRANS_MED], 8, 3);
 add_menu_button(box_x + 3, box_y + 3, box_x2 - 3, box_y + BOX_HEADER_H, colours.base_trans [COL_BLUE] [SHADE_MAX] [TRANS_MED], 8, 3);



 draw_vbuf();

 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], text_x, (int) (box_y + 10), ALLEGRO_ALIGN_LEFT, "%s", w.player[game.user_player_index].name);
 text_y = box_y + BOX_HEADER_H + 7;// + BOX_LINE_H;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "data");
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "%i", w.player[game.user_player_index].data);
 text_y += BOX_LINE_H;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "processes");
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "%i (%i)", w.player[game.user_player_index].processes, w.cores_per_player);
 text_y += BOX_LINE_H;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "components");
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "%i (%i) (%i)", w.player[game.user_player_index].components_current, w.player[game.user_player_index].components_reserved, w.procs_per_player);
#endif

// draw data box:
#ifdef RECORDING_VIDEO_2
 if (FALSE)
#else
 if (command.select_mode == SELECT_MODE_SINGLE_CORE)
#endif
	{

		box_y = box_y + box_h + 3;//scaleUI_y(FONT_SQUARE,28);

		if (box_y < inter.mode_buttons_y1 + MODE_BUTTON_SIZE + 3)
			box_y = inter.mode_buttons_y1 + MODE_BUTTON_SIZE + 3;

		process_box_y = box_y; // this may be used later if the component box needs to be moved out of the way of the map

		core = &w.core[command.selected_core [0]];

  int button_shade = SHADE_HIGH;

		if (!view.data_box_open)
		{
			 add_menu_button(box_x, box_y, box_x2, box_y + BOX_HEADER_H + 3, colours.base_trans [COL_TURQUOISE] [SHADE_MED] [TRANS_MED], 8, 3);
    add_menu_button(box_x + 3, box_y + 3, box_x2 - 3 - scaleUI_x(FONT_SQUARE,30), box_y + BOX_HEADER_H, colours.base_trans [COL_TURQUOISE] [SHADE_MAX] [TRANS_MED], 8, 3);

   button_shade = SHADE_HIGH;

// data box close button:
//  add_menu_button(box_x2 - 30, box_y + 3, box_x2 - 3, box_y + BOX_HEADER_H, colours.base_trans [COL_YELLOW] [SHADE_MAX] [TRANS_MED], 8, 3);
		  if (control.mouse_x_screen_pixels >= view.data_box_close_button_x1
			  && control.mouse_x_screen_pixels <= view.data_box_close_button_x2
			  && control.mouse_y_screen_pixels >= view.data_box_close_button_y1
			  && control.mouse_y_screen_pixels <= view.data_box_close_button_y2)
			   button_shade = SHADE_MAX;

    add_menu_button(view.data_box_close_button_x1, view.data_box_close_button_y1, view.data_box_close_button_x2, view.data_box_close_button_y2, colours.base_trans [COL_TURQUOISE] [button_shade] [TRANS_MED], 8, 3);

    draw_vbuf();

    al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], text_x, (int) (box_y + 10), ALLEGRO_ALIGN_LEFT, "process %i", core->index);
    al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], (int) (text_x2 - scaleUI_x(FONT_SQUARE,32)), (int) (box_y + 10), ALLEGRO_ALIGN_RIGHT, "%s", templ[core->player_index][core->template_index].name);

    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], (int) (view.data_box_close_button_x1 + scaleUI_x(FONT_BASIC,13)), (int) (box_y + scaleUI_x(FONT_BASIC,10)), ALLEGRO_ALIGN_CENTRE, "+");
		 }
		  else
				{

		box_lines = 10;
		if (core->interface_available
			&& core->interface_strength_max	> 0)
			box_lines += 2;
		if (core->data_storage_capacity > 0)
			box_lines ++;

		box_h = BOX_HEADER_H + BOX_LINE_H * box_lines + 8 + POWER_BOX_H + 10;

  add_menu_button(box_x, box_y, box_x2, box_y + box_h, colours.base_trans [COL_TURQUOISE] [SHADE_MED] [TRANS_MED], 8, 3);
  add_menu_button(box_x + 3, box_y + 3, box_x2 - 3 - scaleUI_x(FONT_SQUARE,30), box_y + BOX_HEADER_H, colours.base_trans [COL_TURQUOISE] [SHADE_MAX] [TRANS_MED], 8, 3);

  button_shade = SHADE_HIGH;

// data box close button:
//  add_menu_button(box_x2 - 30, box_y + 3, box_x2 - 3, box_y + BOX_HEADER_H, colours.base_trans [COL_YELLOW] [SHADE_MAX] [TRANS_MED], 8, 3);
		if (control.mouse_x_screen_pixels >= view.data_box_close_button_x1
			&& control.mouse_x_screen_pixels <= view.data_box_close_button_x2
			&& control.mouse_y_screen_pixels >= view.data_box_close_button_y1
			&& control.mouse_y_screen_pixels <= view.data_box_close_button_y2)
			button_shade = SHADE_MAX;

  add_menu_button(view.data_box_close_button_x1, view.data_box_close_button_y1, view.data_box_close_button_x2, view.data_box_close_button_y2, colours.base_trans [COL_TURQUOISE] [button_shade] [TRANS_MED], 8, 3);


  draw_vbuf();

  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], (int) (text_x), (int) (box_y + 10), ALLEGRO_ALIGN_LEFT, "process %i", core->index);
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], (int) (text_x2 - scaleUI_x(FONT_SQUARE,32)), (int) (box_y + 10), ALLEGRO_ALIGN_RIGHT, "%s", templ[core->player_index][core->template_index].name);

  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], (int) (view.data_box_close_button_x1 + scaleUI_x(FONT_BASIC,13)), (int) (box_y + scaleUI_y(FONT_BASIC,11)), ALLEGRO_ALIGN_CENTRE, "X");


  text_y = box_y + BOX_HEADER_H + 7;// + BOX_LINE_H;

		if (core->data_storage_capacity > 0)
		{
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "data");
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "%i (%i)", core->data_stored, core->data_storage_capacity);
   text_y += BOX_LINE_H;
		}

   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "components");
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "%i (%i)", core->group_members_current, core->group_members_max);
   text_y += BOX_LINE_H;
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "integrity");
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "%i (%i) (%i)", core->group_total_hp, core->group_total_hp_max_current, core->group_total_hp_max_undamaged);
   text_y += BOX_LINE_H;


//   if (core->interface_strength > 0) // can be < 0 if broken
			{



    int integrity_bar_max = (text_x2 - text_x - 100);

    add_orthogonal_rect(2, text_x + 10, text_y - 2, text_x + integrity_bar_max + 10, text_y + 10, colours.base_trans [COL_TURQUOISE] [SHADE_MED] [TRANS_THICK]);




    if (core->group_total_hp == core->group_total_hp_max_undamaged)
				{
     add_orthogonal_rect(2, text_x + 12, text_y, text_x + integrity_bar_max + 8, text_y + 8, colours.base_trans [COL_YELLOW] [SHADE_HIGH] [TRANS_THICK]);
				}
				 else
					{
						 if (core->group_total_hp == core->group_total_hp_max_current)
							{
        add_orthogonal_rect(2, text_x + 12, text_y, text_x + 12 + (core->group_total_hp * (integrity_bar_max-4) / core->group_total_hp_max_undamaged), text_y + 8, colours.base_trans [COL_ORANGE] [SHADE_HIGH] [TRANS_THICK]);
							}
							 else
								{
         add_orthogonal_rect(2, text_x + 12, text_y, text_x + 12 + (core->group_total_hp_max_current * (integrity_bar_max-4) / core->group_total_hp_max_undamaged), text_y + 8, colours.base_trans [COL_RED] [SHADE_MED] [TRANS_THICK]);
         add_orthogonal_rect(2, text_x + 12, text_y, text_x + 12 + (core->group_total_hp * (integrity_bar_max-4) / core->group_total_hp_max_undamaged), text_y + 8, colours.base_trans [COL_ORANGE] [SHADE_HIGH] [TRANS_THICK]);
								}
					}
/*
							bar_col = COL_ORANGE;
							bar_shade = SHADE_MAX;

      if (core->interface_broken_time + INTERFACE_BROKEN_TIMER > w.world_time)
						{
							bar_col = COL_PURPLE;
							bar_shade = SHADE_MAX;
						}
						 else
							{
					   bar_col = COL_BLUE;
					   bar_shade = SHADE_MAX;
							}
					}

				float bar_shade = 0.9;

				ALLEGRO_COLOR integrity_bar_colour = al_map_rgba(w.player[core->player_index].interface_colour_base [0] + (w.player[core->player_index].interface_colour_charge [0] * bar_shade),
																																																    	w.player[core->player_index].interface_colour_base [1] + (w.player[core->player_index].interface_colour_charge [1] * bar_shade),
																																															    		w.player[core->player_index].interface_colour_base [2] + (w.player[core->player_index].interface_colour_charge [2] * bar_shade),
																																															    		150);

    add_orthogonal_rect(2, text_x + 12, text_y, text_x + 12 + (core->group_total_hp * (integrity_bar_max-4) / core->group_total_hp_max_undamaged), text_y + 8, integrity_bar_colour);

				bar_shade = 0.3;
				if (core->group_total_hp == core->group_total_hp_max_undamaged)
					bar_shade = 0.6;

				integrity_bar_colour = al_map_rgba(w.player[core->player_index].interface_colour_base [0] + (w.player[core->player_index].interface_colour_charge [0] * bar_shade),
																																																    	w.player[core->player_index].interface_colour_base [1] + (w.player[core->player_index].interface_colour_charge [1] * bar_shade),
																																															    		w.player[core->player_index].interface_colour_base [2] + (w.player[core->player_index].interface_colour_charge [2] * bar_shade),
																																															    		150);

    add_orthogonal_rect(2, text_x + 12, text_y, text_x + 12 + (core->group_total_hp_max_current * (integrity_bar_max-4) / core->group_total_hp_max_undamaged), text_y + 8, integrity_bar_colour);*/
    text_y += BOX_LINE_H;
/*

    int integrity_bar_max = (text_x2 - text_x - 100);

    add_orthogonal_rect(2, text_x + 10, text_y - 2, text_x + integrity_bar_max + 10, text_y + 10, colours.base_trans [COL_TURQUOISE] [SHADE_MED] [TRANS_THICK]);


				float bar_shade = 0.9;

				ALLEGRO_COLOR integrity_bar_colour = al_map_rgba(w.player[core->player_index].interface_colour_base [0] + (w.player[core->player_index].interface_colour_charge [0] * bar_shade),
																																																    	w.player[core->player_index].interface_colour_base [1] + (w.player[core->player_index].interface_colour_charge [1] * bar_shade),
																																															    		w.player[core->player_index].interface_colour_base [2] + (w.player[core->player_index].interface_colour_charge [2] * bar_shade),
																																															    		150);

    add_orthogonal_rect(2, text_x + 12, text_y, text_x + 12 + (core->group_total_hp * (integrity_bar_max-4) / core->group_total_hp_max_undamaged), text_y + 8, integrity_bar_colour);

				bar_shade = 0.3;
				if (core->group_total_hp == core->group_total_hp_max_undamaged)
					bar_shade = 0.6;

				integrity_bar_colour = al_map_rgba(w.player[core->player_index].interface_colour_base [0] + (w.player[core->player_index].interface_colour_charge [0] * bar_shade),
																																																    	w.player[core->player_index].interface_colour_base [1] + (w.player[core->player_index].interface_colour_charge [1] * bar_shade),
																																															    		w.player[core->player_index].interface_colour_base [2] + (w.player[core->player_index].interface_colour_charge [2] * bar_shade),
																																															    		150);

    add_orthogonal_rect(2, text_x + 12, text_y, text_x + 12 + (core->group_total_hp_max_current * (integrity_bar_max-4) / core->group_total_hp_max_undamaged), text_y + 8, integrity_bar_colour);
    text_y += BOX_LINE_H;
*/
		}


  if (core->interface_available
			&& core->interface_strength_max	> 0) // avoids divide by zero below
		{
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "interface");
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "%i (%i)", core->interface_strength, core->interface_strength_max);

   text_y += BOX_LINE_H;


   int interface_bar_max = (text_x2 - text_x - 100);


   add_orthogonal_rect(2, text_x + 10, text_y - 2, text_x + interface_bar_max + 10, text_y + 10, colours.base_trans [COL_TURQUOISE] [SHADE_MED] [TRANS_THICK]);

   if (core->interface_strength > 0) // can be < 0 if broken
			{
/*

				float bar_shade = 0.4;
				if (core->interface_strength == core->interface_strength_max)
					bar_shade = 0.7;

				ALLEGRO_COLOR interface_bar_colour = al_map_rgba(w.player[core->player_index].interface_colour_base [0] + (w.player[core->player_index].interface_colour_charge [0] * bar_shade),
																																																    	w.player[core->player_index].interface_colour_base [1] + (w.player[core->player_index].interface_colour_charge [1] * bar_shade),
																																															    		w.player[core->player_index].interface_colour_base [2] + (w.player[core->player_index].interface_colour_charge [2] * bar_shade),
																																															    		150);
*/

    int bar_col, bar_shade;

    if (core->interface_strength == core->interface_strength_max)
				{
					bar_col = COL_GREY;
					bar_shade = SHADE_MAX;
				}
				 else
					{
      if (core->interface_broken_time + INTERFACE_BROKEN_TIMER > w.world_time)
						{
							bar_col = COL_PURPLE;
							bar_shade = SHADE_MAX;
						}
						 else
							{
					   bar_col = COL_BLUE;
					   bar_shade = SHADE_MAX;
							}
					}
    add_orthogonal_rect(2, text_x + 12, text_y, text_x + 12 + (core->interface_strength * (interface_bar_max-4) / core->interface_strength_max), text_y + 8, colours.base_trans [bar_col] [bar_shade] [TRANS_MED]);
		}

   if (core->interface_broken_time + INTERFACE_BROKEN_TIMER > w.world_time)
    al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_RED] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "broken (%i)", (core->interface_broken_time + INTERFACE_BROKEN_TIMER - w.world_time) / EXECUTION_COUNT);
     else
					{

								{
						   if (core->interface_control_status == 0)
          al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_ORANGE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "lowered");
           else
            al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "active");
								}
					}


   text_y += BOX_LINE_H;

		}



  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "inertia");
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "%i", core->group_mass);
  text_y += BOX_LINE_H;
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "moment");
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "%i", core->group_moment);
//  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "speed %f (%i,%i)", hypot(al_fixtof(core->group_speed.y), al_fixtof(core->group_speed.x)), al_fixtoi(core->group_speed.x * 10), al_fixtoi(core->group_speed.y * 10));
  text_y += BOX_LINE_H;
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "signature");
  char scan_bitfield_text [17];
  int first_nonzero_bit = 0;
  for (i = 0; i < 16; i ++)
		{
			if (core->scan_bitfield & (1 << i))
			{
				scan_bitfield_text [15 - i] = '1';
				first_nonzero_bit = i;
			}
				 else
  				scan_bitfield_text [15 - i] = '0';
		}
		scan_bitfield_text [16] = '\0';
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "0b%s", scan_bitfield_text + (15 - first_nonzero_bit));
//  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "speed %f (%i,%i)", hypot(al_fixtof(core->group_speed.y), al_fixtof(core->group_speed.x)), al_fixtoi(core->group_speed.x * 10), al_fixtoi(core->group_speed.y * 10));
  text_y += BOX_LINE_H;

  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "next cycle");
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "%i", core->next_execution_timestamp - w.world_time);
  text_y += BOX_LINE_H;

  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "instructions used");
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "%i (%i)", core->instructions_used, core->instructions_per_cycle);
  text_y += BOX_LINE_H;
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "power used");

     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_YELLOW] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "%i (%i)", core->power_capacity - core->power_left, core->power_capacity);
  text_y += BOX_LINE_H;
  text_y += BOX_LINE_H;

  float scaled_construct_box_y = scaleUI_y(FONT_SQUARE,50);

#define CONSTRUCT_BOX_Y scaled_construct_box_y
   float power_box_y;// = text_y + CONSTRUCT_BOX_Y;//_BOX_H;// + 2;

  if (core->construction_complete_timestamp > w.world_time)
		{

   power_box_y = text_y + CONSTRUCT_BOX_Y;//_BOX_H;// + 2;

   al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_YELLOW] [SHADE_MAX] [TRANS_THICK], (int) (text_x + scaleUI_x(FONT_SQUARE,20)), (int) (text_y + (CONSTRUCT_BOX_Y) - scaleUI_y(FONT_SQUARE,25)), ALLEGRO_ALIGN_LEFT, "Constructing...");
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base_trans [COL_YELLOW] [SHADE_HIGH] [TRANS_THICK], (int) (text_x + scaleUI_x(FONT_SQUARE,70)), (int) (text_y + (CONSTRUCT_BOX_Y)), ALLEGRO_ALIGN_LEFT, "Ready in %i", (core->construction_complete_timestamp - w.world_time) / EXECUTION_COUNT);
   int line_pos = (core->construction_complete_timestamp - w.world_time) % 20;

   add_orthogonal_rect(2, text_x + scaleUI_x(FONT_SQUARE,10), text_y + CONSTRUCT_BOX_Y - scaleUI_y(FONT_SQUARE,35), text_x + scaleUI_x(FONT_SQUARE,210), text_y + CONSTRUCT_BOX_Y + scaleUI_y(FONT_SQUARE,20), colours.base_trans [COL_ORANGE] [SHADE_MED] [TRANS_FAINT]);
   add_orthogonal_rect(2, text_x + scaleUI_x(FONT_SQUARE,10) + line_pos * scaleUI_x(FONT_SQUARE,10), text_y + CONSTRUCT_BOX_Y - scaleUI_y(FONT_SQUARE,35), text_x + scaleUI_x(FONT_SQUARE,20) + line_pos * scaleUI_x(FONT_SQUARE,10), text_y + CONSTRUCT_BOX_Y + scaleUI_y(FONT_SQUARE,20), colours.base_trans [COL_ORANGE] [SHADE_MAX] [TRANS_MED]);


		}
		 else
			{

   power_box_y = text_y + POWER_BOX_H; //_BOX_H;// + 2;

   add_orthogonal_rect(2, text_x, text_y, text_x + POWER_BOX_W, text_y + POWER_BOX_H, colours.base_trans [COL_BLUE] [SHADE_LOW] [TRANS_MED]);
// this draws a line:
   add_orthogonal_rect(2, text_x, text_y + (POWER_BOX_H/2), text_x + POWER_BOX_W, text_y + (POWER_BOX_H/2) + 1, colours.base_trans [COL_BLUE] [SHADE_HIGH] [TRANS_FAINT]);

   al_draw_textf(font[FONT_BASIC].fnt, colours.base_trans [COL_BLUE] [SHADE_HIGH] [TRANS_THICK], text_x + 2, (int) (text_y + (POWER_BOX_H) - scaleUI_y(FONT_BASIC,9)), ALLEGRO_ALIGN_LEFT, "power");

   al_draw_textf(font[FONT_BASIC].fnt, colours.base_trans [COL_PURPLE] [SHADE_HIGH] [TRANS_MED], (int) (text_x + POWER_BOX_W + 4), (int) (text_y + 2), ALLEGRO_ALIGN_LEFT, "%i", core->power_capacity * 2);
   al_draw_textf(font[FONT_BASIC].fnt, colours.base_trans [COL_BLUE] [SHADE_HIGH] [TRANS_MED], (int) (text_x + POWER_BOX_W + 4), (int) (text_y + (POWER_BOX_H/2) + 2), ALLEGRO_ALIGN_LEFT, "%i", core->power_capacity);
   al_draw_textf(font[FONT_BASIC].fnt, colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_MED], (int) (text_x + POWER_BOX_W + 4), (int) (text_y + (POWER_BOX_H) - 3), ALLEGRO_ALIGN_LEFT, "0");

   j = command.power_use_pos;

   float scaled_power_bar_width = scaleUI_x(FONT_SQUARE,4);


   for (i = 0; i < POWER_DATA_RECORDS; i ++)
			{

				j --;
				if (j < 0)
					j = POWER_DATA_RECORDS - 1;
/*
				if (command.stress_record [j] != 0)
				{
//					int record_stress_level = ;

     add_orthogonal_rect(text_x + STRESS_BOX_W - i * 4 - 3,
																									text_y + STRESS_BOX_H - (command.stress_record [j] * (STRESS_BOX_H / 3)) / core->power_capacity,
																									text_x + STRESS_BOX_W - i * 4,
																									text_y + STRESS_BOX_H,
																									colours.base_trans [stress_level_col [command.stress_level_record [j]]] [SHADE_HIGH] [TRANS_THICK]);
				}
*/
				if (command.power_use_record [j] != 0)
				{
					int power_record_level = (command.power_use_record [j] * (POWER_BOX_H/2)) / core->power_capacity;

     add_orthogonal_rect(2, text_x + POWER_BOX_W - i * scaled_power_bar_width - (scaled_power_bar_width - 1),
																									power_box_y - power_record_level,
																									text_x + POWER_BOX_W - i * scaled_power_bar_width,
																									power_box_y,
																									colours.base_trans [COL_BLUE] [SHADE_HIGH] [TRANS_THICK]);

					if (command.power_fail_record [j] > 0)
					{

					 int power_excess_level = (command.power_fail_record [j] * (POWER_BOX_H/2)) / core->power_capacity;
//					int power_excess_col = COL_PURPLE;
      float power_excess_top = power_box_y - power_record_level - power_excess_level;

      if (power_excess_top < text_y + 2)
						 power_excess_top = text_y + 2;

      add_orthogonal_rect(2, text_x + POWER_BOX_W - i * 4 - 3,
																									 power_excess_top,
																									 text_x + POWER_BOX_W - i * 4,
																									 power_box_y - power_record_level,
																									 colours.base_trans [COL_RED] [SHADE_MED] [TRANS_MED]);

					}


				}

			}

	  } // end if (construction complete)

   text_y += POWER_BOX_H + 7;

// note that this code is inside if (command.select_mode == COMMAND_SINGLE_CORE)
  if (command.selected_member != -1) // can be -1 if e.g. selected member destroyed since selection, but core survives
		{
			struct proc_struct* selected_proc = &w.proc[core->group_member[command.selected_member].index];
			box_y = text_y + 8;//box_y + box_h + 8 + STRESS_BOX_H;
			box_lines = 4 + selected_proc->nshape_ptr->links;

 		box_h = BOX_HEADER_H + BOX_LINE_H * box_lines + 8;

 		if (box_y + box_h > view.map_y - 4)
			{
				box_x -= BOX_W + 4;
				box_x2 -= BOX_W + 4;
				text_x -= BOX_W + 4;
				text_x2 -= BOX_W + 4;
				box_y = process_box_y;
			}

   add_menu_button(box_x, box_y, box_x2, box_y + box_h, colours.base_trans [COL_AQUA] [SHADE_MED] [TRANS_MED], 8, 3);
   add_menu_button(box_x + 3, box_y + 3, box_x2 - 3, box_y + BOX_HEADER_H, colours.base_trans [COL_AQUA] [SHADE_MAX] [TRANS_MED], 8, 3);

   draw_vbuf();

   text_y = box_y + BOX_HEADER_H + 7;// + BOX_LINE_H;

   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], text_x, (int) (box_y + 10), ALLEGRO_ALIGN_LEFT, "component %i", command.selected_member);
//   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], text_x2, box_y + 9, ALLEGRO_ALIGN_RIGHT, "%i", command.selected_member);
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_AQUA] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "integrity");
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_AQUA] [SHADE_MAX], text_x2, text_y, ALLEGRO_ALIGN_RIGHT, "%i (%i)", selected_proc->hp, selected_proc->hp_max);
    text_y += BOX_LINE_H;

   {

    int integrity_bar_max = (text_x2 - text_x - 100);

    add_orthogonal_rect(2, text_x + 10, text_y - 2, text_x + integrity_bar_max + 10, text_y + 10, colours.base_trans [COL_TURQUOISE] [SHADE_HIGH] [TRANS_THICK]);

/*
				float bar_shade = 0.9;

				ALLEGRO_COLOR integrity_bar_colour = al_map_rgba(w.player[core->player_index].interface_colour_base [0] + (w.player[core->player_index].interface_colour_charge [0] * bar_shade),
																																																    	w.player[core->player_index].interface_colour_base [1] + (w.player[core->player_index].interface_colour_charge [1] * bar_shade),
																																															    		w.player[core->player_index].interface_colour_base [2] + (w.player[core->player_index].interface_colour_charge [2] * bar_shade),
																																															    		150);
*/
    add_orthogonal_rect(2, text_x + 12, text_y, text_x + 12 + (selected_proc->hp * (integrity_bar_max-4) / selected_proc->hp_max), text_y + 8, colours.base_trans [COL_ORANGE] [SHADE_HIGH] [TRANS_THICK]);
/*
				bar_shade = 0.3;
				if (core->group_total_hp == core->group_total_hp_max_undamaged)
					bar_shade = 0.6;

				integrity_bar_colour = al_map_rgba(w.player[core->player_index].interface_colour_base [0] + (w.player[core->player_index].interface_colour_var [0] * bar_shade),
																																																    	w.player[core->player_index].interface_colour_base [1] + (w.player[core->player_index].interface_colour_var [1] * bar_shade),
																																															    		w.player[core->player_index].interface_colour_base [2] + (w.player[core->player_index].interface_colour_var [2] * bar_shade),
																																															    		150);

    add_orthogonal_rect(text_x + 12, text_y, text_x + 12 + (core->group_total_hp_max_current * (integrity_bar_max-4) / core->group_total_hp_max_undamaged), text_y + 8, integrity_bar_colour);*/

			}

   text_y += BOX_LINE_H;
   text_y += BOX_LINE_H;
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_AQUA] [SHADE_MAX], text_x, text_y, ALLEGRO_ALIGN_LEFT, "objects:");
   text_y += BOX_LINE_H;
   for (i = 0; i < selected_proc->nshape_ptr->links; i ++)
			{
				if (selected_proc->object[i].type == OBJECT_TYPE_NONE)
     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_AQUA] [SHADE_HIGH], text_x + 5, text_y, ALLEGRO_ALIGN_LEFT, "%i  none", i);
				  else
						{
       al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_AQUA] [SHADE_MAX], text_x + 5, text_y, ALLEGRO_ALIGN_LEFT, "%i  %s", i, otype[selected_proc->object[i].type].name);
       switch(selected_proc->object[i].type)
       {
							 case OBJECT_TYPE_BUILD:
							 	if (core->build_cooldown_time > w.world_time)
          print_object_information(text_x2, text_y, COL_YELLOW, "recycle", (core->build_cooldown_time - w.world_time) / EXECUTION_COUNT, 1);
           else
            print_object_information(text_x2, text_y, COL_AQUA, "ready", 0, 0);
								 break;
							 case OBJECT_TYPE_REPAIR:
							 case OBJECT_TYPE_REPAIR_OTHER:
							 	if (core->restore_cooldown_time > w.world_time)
          print_object_information(text_x2, text_y, COL_YELLOW, "recycle", (core->restore_cooldown_time - w.world_time) / EXECUTION_COUNT, 1);
           else
            print_object_information(text_x2, text_y, COL_AQUA, "ready", 0, 0);
								 break;
							 case OBJECT_TYPE_MOVE:
         print_object_information(text_x2, text_y, COL_YELLOW, "power", selected_proc->object_instance[i].move_power, 1);
								 break;
							 case OBJECT_TYPE_PULSE:
							 case OBJECT_TYPE_PULSE_L:
							 case OBJECT_TYPE_PULSE_XL:
							 case OBJECT_TYPE_BURST:
							 case OBJECT_TYPE_BURST_L:
							 case OBJECT_TYPE_BURST_XL:
							 case OBJECT_TYPE_SPIKE:
							 case OBJECT_TYPE_ULTRA:
							 case OBJECT_TYPE_ULTRA_DIR:
							 	if (selected_proc->object_instance[i].attack_recycle_timestamp > w.world_time)
          print_object_information(text_x2, text_y, COL_YELLOW, "recycle", (selected_proc->object_instance[i].attack_recycle_timestamp - w.world_time) / EXECUTION_COUNT, 1);
           else
            print_object_information(text_x2, text_y, COL_AQUA, "ready", 0, 0);
         break;
							 case OBJECT_TYPE_STREAM:
							 case OBJECT_TYPE_STREAM_DIR:
							 	if (selected_proc->object_instance[i].attack_last_fire_timestamp >= w.world_time - STREAM_TOTAL_FIRING_TIME)
          print_object_information(text_x2, text_y, COL_RED, "firing", 0, 0);
           else
											{
   							 	if (selected_proc->object_instance[i].attack_recycle_timestamp > w.world_time)
             print_object_information(text_x2, text_y, COL_YELLOW, "recycle", (selected_proc->object_instance[i].attack_recycle_timestamp - w.world_time) / EXECUTION_COUNT, 1);
              else
               print_object_information(text_x2, text_y, COL_AQUA, "ready", 0, 0);
											}
							 	break;
							 case OBJECT_TYPE_SLICE:
							 	if (selected_proc->object_instance[i].attack_last_fire_timestamp >= w.world_time - SLICE_TOTAL_FIRING_TIME)
          print_object_information(text_x2, text_y, COL_RED, "firing", 0, 0);
           else
											{
   							 	if (selected_proc->object_instance[i].attack_recycle_timestamp > w.world_time)
             print_object_information(text_x2, text_y, COL_YELLOW, "recycle", (selected_proc->object_instance[i].attack_recycle_timestamp - w.world_time) / EXECUTION_COUNT, 1);
              else
               print_object_information(text_x2, text_y, COL_AQUA, "ready", 0, 0);
											}
							 	break;
							 case OBJECT_TYPE_STABILITY:
							 	if (selected_proc->interface_stability)
          print_object_information(text_x2, text_y, COL_AQUA, "on", 0, 0);
           else
            print_object_information(text_x2, text_y, COL_RED, "off", 0, 0);
         break;
       }
						}
    text_y += BOX_LINE_H;
			}
		}
	} // end if (!view.data_box_open)
	} // end if (single core selected)


	if (command.select_mode == SELECT_MODE_DATA_WELL)
	{



				float time_since_selection = 12;//game.total_time;
//				if (time_since_selection > 12)
//					time_since_selection = 12;

   x = al_fixtof(w.data_well[command.selected_data_well].position.x - view.camera_x) * view.zoom;
   y = al_fixtof(w.data_well[command.selected_data_well].position.y - view.camera_y) * view.zoom;
   x += view.window_x_unzoomed / 2;
   y += view.window_y_unzoomed / 2;

   if (x > -200 && x < view.window_x_unzoomed + 200
    && y > -200 && y < view.window_y_unzoomed + 200)
   {


//  al_draw_rectangle(x - marker_line_dist, y - marker_line_dist, x + marker_line_dist, y + marker_line_dist, colours.base_trans [COL_RED] [SHADE_HIGH] [5], 0);
    select_arrows(4, x, y,
				  												game.total_time * -0.02,
																		(80.5 - time_since_selection * 3) * view.zoom, // radius
																  11,
																  PI/5,
																  16,
																  colours.base [COL_YELLOW] [SHADE_MAX]);
   }

//		float box_x = view.window_x_unzoomed - BOX_W - 10;
//		float box_x2 = box_x + BOX_W;

		box_y = box_y + box_h + 28;

		box_h = BOX_HEADER_H + BOX_LINE_H * 4 + 8;

  add_menu_button(box_x, box_y, box_x2, box_y + box_h, colours.base_trans [COL_YELLOW] [SHADE_MED] [TRANS_MED], 8, 3);
  add_menu_button(box_x + 3, box_y + 3, box_x2 - 3, box_y + BOX_HEADER_H, colours.base_trans [COL_YELLOW] [SHADE_MAX] [TRANS_MED], 8, 3);

  draw_vbuf();

  text_y = box_y;// + BOX_HEADER_H + 7;// + BOX_LINE_H;

  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], text_x, text_y + 10, ALLEGRO_ALIGN_LEFT, "data well");

  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_ORANGE] [SHADE_MAX], text_x, (int) (text_y + BOX_HEADER_H + 7), ALLEGRO_ALIGN_LEFT, "data");
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_ORANGE] [SHADE_MAX], text_x2, (int) (text_y + BOX_HEADER_H + 7), ALLEGRO_ALIGN_RIGHT, "%i (%i)", w.data_well[command.selected_data_well].data, w.data_well[command.selected_data_well].data_max);
  text_y += BOX_LINE_H;
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_ORANGE] [SHADE_MAX], text_x, (int) (text_y + BOX_HEADER_H + 7), ALLEGRO_ALIGN_LEFT, "replenish");
  int replenish_rate = 0;
  if (w.data_well[command.selected_data_well].reserve_data [0] > 0)
			replenish_rate += w.data_well[command.selected_data_well].reserve_squares * DATA_WELL_REPLENISH_RATE;
  if (w.data_well[command.selected_data_well].reserve_data [1] > 0)
			replenish_rate += w.data_well[command.selected_data_well].reserve_squares * DATA_WELL_REPLENISH_RATE;
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_ORANGE] [SHADE_MAX], text_x2, (int) (text_y + BOX_HEADER_H + 7), ALLEGRO_ALIGN_RIGHT, "%i", replenish_rate);
  text_y += BOX_LINE_H;
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_ORANGE] [SHADE_MAX], text_x, (int) (text_y + BOX_HEADER_H + 7), ALLEGRO_ALIGN_LEFT, "reserve A");
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_ORANGE] [SHADE_MAX], text_x2, (int) (text_y + BOX_HEADER_H + 7), ALLEGRO_ALIGN_RIGHT, "%i", w.data_well[command.selected_data_well].reserve_data [0]);
  text_y += BOX_LINE_H;
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_ORANGE] [SHADE_MAX], text_x, (int) (text_y + BOX_HEADER_H + 7), ALLEGRO_ALIGN_LEFT, "reserve B");
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_ORANGE] [SHADE_MAX], text_x2, (int) (text_y + BOX_HEADER_H + 7), ALLEGRO_ALIGN_RIGHT, "%i", w.data_well[command.selected_data_well].reserve_data [1]);
	}

 draw_vbuf(); // sends poly_buffer and line_buffer to the screen - do it here to make sure any selection graphics are drawn before the map
// draw_fans();
#ifndef RECORDING_VIDEO_2
 draw_map();
#else
 if (w.debug_mode == 1)
		draw_map();
#endif
//  al_draw_circle(view.window_x / 2, view.window_y / 2, 2, base_col [COL_GREY] [SHADE_MIN], 1);



//  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], 2, 2, ALLEGRO_ALIGN_LEFT, "fps %i", view.fps);
/*
  if (w.players >= 2)
  {
   i = 1;
//   al_draw_textf(font[FONT_SQUARE].fnt, text_col, sx, sy, ALLEGRO_ALIGN_RIGHT, "p %i(%i)", w.player[i].processes, w.procs_per_player);
   al_draw_textf(font[FONT_SQUARE].fnt, text_col, sx, sy, ALLEGRO_ALIGN_RIGHT, "data %i", w.player[i].data);
   sx -= 80;
   al_draw_textf(font[FONT_SQUARE].fnt, text_col, sx, sy, ALLEGRO_ALIGN_RIGHT, "%s", w.player[i].name);
   sx -= 90;
  }

  i = 0;
// Player 1
  al_draw_textf(font[FONT_SQUARE].fnt, text_col, sx, sy, ALLEGRO_ALIGN_RIGHT, "data %i", w.player[i].data);
  sx -= 80;
  al_draw_textf(font[FONT_SQUARE].fnt, text_col, sx, sy, ALLEGRO_ALIGN_RIGHT, "%s", w.player[i].name);
  sx -= 90;

  if (w.players >= 3)
  {
   sx = STATUS_X;
   sy += 12;

   if (w.players == 4)
   {
    i = 3;
    al_draw_textf(font[FONT_SQUARE].fnt, text_col, sx, sy, ALLEGRO_ALIGN_RIGHT, "data %i", w.player[i].data);
    sx -= 80;
    al_draw_textf(font[FONT_SQUARE].fnt, text_col, sx, sy, ALLEGRO_ALIGN_RIGHT, "%s", w.player[i].name);
    sx -= 90;
   }

   i = 2;
   al_draw_textf(font[FONT_SQUARE].fnt, text_col, sx, sy, ALLEGRO_ALIGN_RIGHT, "data %i", w.player[i].data);
   sx -= 80;
   al_draw_textf(font[FONT_SQUARE].fnt, text_col, sx, sy, ALLEGRO_ALIGN_RIGHT, "%s", w.player[i].name);
   sx -= 90;
  }



 sx = STATUS_X;
 sy += 12;
*/
// sx -= 90;
/* if (game.minutes_each_turn != 0)
 {
  seconds = game.current_turn_time_left * 0.03;
  if (seconds > 3599)
   al_draw_textf(font[FONT_SQUARE].fnt, text_col, sx, sy, ALLEGRO_ALIGN_RIGHT, "turn time %i:%.2i:%.2i", seconds / 3600, (int) (seconds / 60) % 60, seconds % 60);
    else
     al_draw_textf(font[FONT_SQUARE].fnt, text_col, sx, sy, ALLEGRO_ALIGN_RIGHT, "turn time %i:%.2i", (int) (seconds / 60) % 60, seconds % 60);
  sx -= 120;
 }*/
/*
 if (game.turns == 0)
 {
  al_draw_textf(font[FONT_SQUARE].fnt, text_col, sx, sy, ALLEGRO_ALIGN_RIGHT, "turn %i", game.current_turn);
  sx -= 40;
 }
  else
  {
   al_draw_textf(font[FONT_SQUARE].fnt, text_col, sx, sy, ALLEGRO_ALIGN_RIGHT, "turn %i/%i", game.current_turn, game.turns);
   sx -= 50;
  }
*/



 draw_panels(); // also draws mode buttons

 al_set_clipping_rectangle(0, 0, panel[PANEL_MAIN].w, panel[PANEL_MAIN].h); // needed after call to draw_panels, which resets clipping

#define LINE_1_Y 100
#define LINE_2_Y 125
#define LINE_3_Y 150


 switch(game.phase)
 {


#define GO_LINE_1_Y 160
#define GO_LINE_2_Y 125


  case GAME_PHASE_OVER:
   switch(game.game_over_status)
   {
    case GAME_END_BASIC: // probably not used
     al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 - GO_LINE_1_Y, ALLEGRO_ALIGN_CENTRE, "GAME OVER");
     break;
    case GAME_END_MISSION_COMPLETE:
     al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 - GO_LINE_1_Y, ALLEGRO_ALIGN_CENTRE, "MISSION COMPLETE");
//     if (
     break;
    case GAME_END_MISSION_FAILED:
     al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 - GO_LINE_1_Y, ALLEGRO_ALIGN_CENTRE, "MISSION FAILED :(");
     break;
    case GAME_END_MISSION_FAILED_TIME:
     al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 - GO_LINE_1_Y, ALLEGRO_ALIGN_CENTRE, "MISSION FAILED :(");
     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 - GO_LINE_2_Y, ALLEGRO_ALIGN_CENTRE, "OUT OF TIME");
     break;
    case GAME_END_PLAYER_WON:
     al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 - GO_LINE_1_Y, ALLEGRO_ALIGN_CENTRE, "GAME OVER");
     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 - GO_LINE_2_Y, ALLEGRO_ALIGN_CENTRE, "%s WINS!", w.player[game.game_over_value].name);
     break;
    case GAME_END_DRAW:
     al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 - GO_LINE_1_Y, ALLEGRO_ALIGN_CENTRE, "GAME OVER");
     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 - GO_LINE_2_Y, ALLEGRO_ALIGN_CENTRE, "STATUS: DRAW");
     break;
    case GAME_END_DRAW_OUT_OF_TIME:
     al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 - GO_LINE_1_Y, ALLEGRO_ALIGN_CENTRE, "GAME OVER");
     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 - GO_LINE_2_Y, ALLEGRO_ALIGN_CENTRE, "STATUS: DRAW (OUT OF TIME)");
     break;


   }
//   break;

// fall-through...

  case GAME_PHASE_WORLD:
#ifndef RECORDING_VIDEO_2
   if (game.pause_soft)
    al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 + LINE_1_Y, ALLEGRO_ALIGN_CENTRE, "PAUSED");
   if (game.watching == WATCH_PAUSED_TO_WATCH)
    al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 + LINE_1_Y + 45, ALLEGRO_ALIGN_CENTRE, "EXECUTING WATCHED PROCESS");
   if (view.following)
    al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 + LINE_1_Y + 90, ALLEGRO_ALIGN_CENTRE, "FOLLOWING");
#endif
//   if (view.under_attack_marker_last_time > w.world_time - UNDER_ATTACK_MARKER_DURATION)
//    al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_RED] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 + LINE_1_Y + 80, ALLEGRO_ALIGN_CENTRE, "UNDER ATTACK");
   if (game.fast_forward > 0)
   {
   	switch(game.fast_forward_type)
   	{
   		case FAST_FORWARD_TYPE_SMOOTH:
      al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 + LINE_2_Y, ALLEGRO_ALIGN_CENTRE, "FAST FORWARD"); break;
   		case FAST_FORWARD_TYPE_NO_DISPLAY:
      al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 + LINE_2_Y, ALLEGRO_ALIGN_CENTRE, "FAST FORWARD (NO DISPLAY)"); break;
   		case FAST_FORWARD_TYPE_SKIP:
      al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 + LINE_2_Y, ALLEGRO_ALIGN_CENTRE, "FAST FORWARD (SKIP)"); break;
   	}
   }
   break;

 }

 display_consoles_and_buttons();

/*
 if (game.pause_hard) // can still enter hard pause when in a non-world game phase (as the system/observer/operator will otherwise continue running)
 {
   al_draw_textf(font[FONT_SQUARE_LARGE].fnt, colours.base [COL_GREY] [SHADE_MAX], view.window_x_unzoomed / 2, view.window_y_unzoomed / 2 + LINE_3_Y, ALLEGRO_ALIGN_CENTRE, "HALTED");
 }
*/

// if (settings.option [OPTION_SPECIAL_CURSOR])
	{
  al_set_clipping_rectangle(0, 0, settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
  draw_mouse_cursor();
	}
 al_flip_display();
 al_set_target_bitmap(al_get_backbuffer(display));


}


static void print_object_information(float text_x, float text_y, int col, const char* ostring, int value, int print_value)
{

  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [col] [SHADE_MAX], text_x - 20, text_y, ALLEGRO_ALIGN_RIGHT, "%s", ostring);

  if (print_value)
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [col] [SHADE_MAX], text_x - 15, text_y, ALLEGRO_ALIGN_LEFT, "%i", value);

}


// called by both burst packet drawing code, and burst tail cloud drawing code (which draws the tail of a burst packet that has been destroyed)
static void draw_burst_tail(float x, float y, float x_step, float y_step, float packet_angle, int packet_time, int start_time, int end_time, int col)
{
 int i;

/*     float blob_size = 10 + drand(5, -1);


   		start_ribbon(2,
																		x + cos(packet_angle) * (blob_size+3) * view.zoom,
																		y + sin(packet_angle) * (blob_size+3) * view.zoom,
																		x + cos(packet_angle + PI/4) * blob_size * view.zoom,
																		y + sin(packet_angle + PI/4) * blob_size * view.zoom,
																		colours.packet [col] [16]);
					add_ribbon_vertex(x + cos(packet_angle - PI/4) * blob_size * view.zoom,
																		     y + sin(packet_angle - PI/4) * blob_size * view.zoom,
																		     colours.packet [col] [16]);
					add_ribbon_vertex(x + cos(packet_angle + PI/2) * blob_size * view.zoom,
																		     y + sin(packet_angle + PI/2) * blob_size * view.zoom,
																		     colours.packet [col] [16]);
					add_ribbon_vertex(x + cos(packet_angle - PI/2) * blob_size * view.zoom,
																		     y + sin(packet_angle - PI/2) * blob_size * view.zoom,
																		     colours.packet [col] [16]);
					add_ribbon_vertex(x + cos(packet_angle + PI*0.7) * blob_size * view.zoom,
																		     y + sin(packet_angle + PI*0.7) * blob_size * view.zoom,
																		     colours.packet [col] [16]);
					add_ribbon_vertex(x + cos(packet_angle - PI*0.7) * blob_size * view.zoom,
																		     y + sin(packet_angle - PI*0.7) * blob_size * view.zoom,
																		     colours.packet [col] [16]);
*/
     x = x - cos(packet_angle) * 16 * view.zoom;
     y = y - sin(packet_angle) * 16 * view.zoom;

/*
					if (start_time > 0)
					{
							drand(5, start_time);
//							x += x_step * start_time * 3;
//							y += y_step * start_time * 3;
					}*/

					int drr;

     for (i = start_time; i < end_time; i ++)
					{

						drr = drand(25, 1);

						float tail_size = (17 - i) * (float) ((drr % 5 + 2) * 0.15);
						float tail_size2 = (17 - i) * (float) ((drr / 5 + 2) * 0.15);
					add_ribbon_vertex(x + cos(packet_angle + PI/2) * tail_size * view.zoom,
																		     y + sin(packet_angle + PI/2) * tail_size * view.zoom,
																		     colours.packet [col] [16]);// - i]);
					add_ribbon_vertex(x + cos(packet_angle - PI/2) * tail_size2 * view.zoom,
																		     y + sin(packet_angle - PI/2) * tail_size2 * view.zoom,
																		     colours.packet [col] [16]);// - i]);
//					}
/*
						float tail_size = ((drand(100, 1) + 2) * 0.1); //(17 - i) * (float) (drand(5, 1) * 0.15);
//						float tail_size = (17 - i) * (float) (drand(5, 1) * 0.15);
						float tail_size2 = tail_size + (17 - i) * 0.4;
						tail_size = 5;
						tail_size2 = -5;
						if (drand(2, 0))
						{
 					add_ribbon_vertex(x + cos(packet_angle + PI/2) * tail_size * view.zoom,
																		      y + sin(packet_angle + PI/2) * tail_size * view.zoom,
 																		     colours.packet [col] [16 - i]);
					 add_ribbon_vertex(x + cos(packet_angle + PI/2) * tail_size2 * view.zoom,
																		      y + sin(packet_angle + PI/2) * tail_size2 * view.zoom,
 																		     colours.packet [col] [16 - i]);
						}
						 else
							{
								add_ribbon_vertex(x + cos(packet_angle - PI/2) * tail_size2 * view.zoom,
																		        y + sin(packet_angle - PI/2) * tail_size2 * view.zoom,
   																		     colours.packet [col] [16 - i]);
					   add_ribbon_vertex(x + cos(packet_angle - PI/2) * tail_size * view.zoom,
																		        y + sin(packet_angle - PI/2) * tail_size * view.zoom,
 																		       colours.packet [col] [16 - i]);

							}
/ *


/ *
						float tail_size = (17 - i) * (float) (drand(5, 1) * 0.15);
					add_ribbon_vertex(x + cos(packet_angle + PI/2) * tail_size * view.zoom,
																		     y + sin(packet_angle + PI/2) * tail_size * view.zoom,
																		     colours.packet [col] [16 - i]);
					add_ribbon_vertex(x + cos(packet_angle - PI/2) * tail_size * view.zoom,
																		     y + sin(packet_angle - PI/2) * tail_size * view.zoom,
																		     colours.packet [col] [16 - i]);
*/
						x += x_step;// * 3;
						y += y_step;// * 3;

					}



}


// called by both pulse packet drawing code, and pulse tail cloud drawing code (which draws the tail of a pulse packet that has been destroyed)
static void draw_pulse_tail(float x, float y,
																												float x_step, float y_step,
																												float packet_angle,
																												int packet_time,
																												int start_time,
																												int end_time,
																												int total_time,
																												float tail_width,
																												int col,
																												int shade,
																												int drand_seed,
																												int packet_size,
																												float blob_scale,
																												int pulse_or_burst)
{

pulse_or_burst = 0;

//     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], x + 40, y + 24 + shade * 2, ALLEGRO_ALIGN_CENTRE, "s %i e %i t %i sd %i", start_time, end_time, total_time, drand_seed + start_time);

     float min_blob_size = total_time - start_time;

     if (min_blob_size > 12)
						min_blob_size = 12;

     seed_drand(drand_seed + start_time);
     float blob_size = (drand(4, 1) + min_blob_size) * blob_scale;

					float side_dist_x = cos(packet_angle + PI/2) * view.zoom;
					float side_dist_y = sin(packet_angle + PI/2) * view.zoom;
					float side2_dist_x = cos(packet_angle - PI/2) * view.zoom;
					float side2_dist_y = sin(packet_angle - PI/2) * view.zoom;

					int adjusted_shade = shade - start_time;

					if (adjusted_shade < 0)
						return;
/*
   		start_ribbon(2,
																		x + cos(packet_angle) * (blob_size+3) * view.zoom,
																		y + sin(packet_angle) * (blob_size+3) * view.zoom,
																		x + side_dist_x * blob_size,
																		y + side_dist_y * blob_size,
																		colours.packet [col] [adjusted_shade]);
					add_ribbon_vertex(x + side2_dist_x * blob_size,
																		     y + side2_dist_y * blob_size,
																		     colours.packet [col] [adjusted_shade]);
*/

   		start_ribbon(2,
																		x + cos(packet_angle) * (blob_size+3) * view.zoom,
																		y + sin(packet_angle) * (blob_size+3) * view.zoom,
																		x + cos(packet_angle + PI/4) * (blob_size + 2) * view.zoom,
																		y + sin(packet_angle + PI/4) * (blob_size + 2) * view.zoom,
																		colours.packet [col] [adjusted_shade]);
					add_ribbon_vertex(x + cos(packet_angle - PI/4) * (blob_size + 2) * view.zoom,
																		     y + sin(packet_angle - PI/4) * (blob_size + 2) * view.zoom,
																		     colours.packet [col] [adjusted_shade]);
					add_ribbon_vertex(x - side2_dist_x * blob_size,
																		     y - side2_dist_y * blob_size,
																		     colours.packet [col] [adjusted_shade]);
					add_ribbon_vertex(x + side2_dist_x * blob_size,
																		     y + side2_dist_y * blob_size,
																		     colours.packet [col] [adjusted_shade]);

     int i;

     seed_drand(drand_seed + start_time);

     x = x - cos(packet_angle) * (5+packet_size) * view.zoom;
     y = y - sin(packet_angle) * (5+packet_size) * view.zoom;

					int drr;

     for (i = start_time; i < end_time; i ++)
					{

						drr = drand(25, 1);

						float tail_size = (total_time - i) * (float) ((drr % 3 + 2) * tail_width);
//						float tail_size2 = (total_time - i) * (float) ((drr / 5 + 2) * tail_width);

					add_ribbon_vertex(x + side_dist_x * tail_size,
																		     y + side_dist_y * tail_size,
																		     colours.packet [col] [adjusted_shade]);// - i]);

					if (pulse_or_burst)
						tail_size = (total_time - i) * (float) ((drr / 5 + 2) * tail_width);

					add_ribbon_vertex(x + side2_dist_x * tail_size,
																		     y + side2_dist_y * tail_size,
																		     colours.packet [col] [adjusted_shade]);// - i]);

						x += x_step;
						y += y_step;

						adjusted_shade --;
						if (adjusted_shade < 0)
							break;

					}

					add_ribbon_vertex(x,
																		     y,
																		     colours.packet [col] [0]);

}

// called by both pulse packet drawing code, and pulse tail cloud drawing code (which draws the tail of a pulse packet that has been destroyed)
static void draw_new_pulse_tail(float x, float y,
																												float x_step, float y_step,
																												float packet_angle,
																												int total_length, // maximum length of tail (its length if not cutoff at either end)
																												int front_cutoff, // number of steps past the front (will be 0 if pulse still travelling)
																												int origin_cutoff, // number of steps past the origin (will be 0 if end of tail has moved past origin)
																												float tail_width,
																												int col,
																												int shade,
																												float blob_scale)
{

//pulse_or_burst = 0;

//     seed_drand(drand_seed + start_time);
//     float blob_size = (3 + packet_size) * blob_scale;;//(drand(4, 1) + 4 + packet_size) * blob_scale;

					float side_dist_x = cos(packet_angle + PI/2);
					float side_dist_y = sin(packet_angle + PI/2);

					float blob_multiplier = blob_scale * tail_width * view.zoom;

// first draw the front:

   		start_ribbon(3,
																		x + cos(packet_angle) * blob_multiplier,
																		y + sin(packet_angle) * blob_multiplier,
																		x + cos(packet_angle + PI/5) * blob_multiplier * 0.7,
																		y + sin(packet_angle + PI/5) * blob_multiplier * 0.7,
																		colours.packet [col] [shade]);

					add_ribbon_vertex(x + cos(packet_angle - PI/5) * blob_multiplier * 0.7,
																		     y + sin(packet_angle - PI/5) * blob_multiplier * 0.7,
																		     colours.packet [col] [shade]);
/*
					add_ribbon_vertex(x + side_dist_x * blob_multiplier * 0.5,
																		     y + side_dist_y * blob_multiplier * 0.5,
																		     colours.packet [col] [shade - 2]);

					add_ribbon_vertex(x - side_dist_x * blob_multiplier * 0.5,
																		     y - side_dist_y * blob_multiplier * 0.5,
																		     colours.packet [col] [shade - 2]);
*/

					float adjusted_side_dist_x = side_dist_x * view.zoom * tail_width;
					float adjusted_side_dist_y = side_dist_y * view.zoom * tail_width;

     int reduced_shade = shade - 4;

     if (reduced_shade < 0)
						reduced_shade = 0;

					add_ribbon_vertex(x + adjusted_side_dist_x + x_step,
																		     y + adjusted_side_dist_y + y_step,
																		     colours.packet [col] [reduced_shade]);
					add_ribbon_vertex(x - adjusted_side_dist_x + x_step,
																		     y - adjusted_side_dist_y + y_step,
																		     colours.packet [col] [reduced_shade]);

					int actual_length = (total_length - origin_cutoff) - front_cutoff;

// May need to draw an origin point:

     if (origin_cutoff > 0)
					{

						float end_centre_x = x + x_step * actual_length;
      float end_centre_y = y + y_step * actual_length;

//      float flash_size = origin_cutoff * tail_width * 0.1 * view.zoom;
      float flash_size = tail_width * 2.0 * view.zoom;

      int end_shade = origin_cutoff;//(shade - ((total_length - origin_cutoff) * 2)) * 2;

      if (end_shade > shade)
							end_shade = shade;
      if (end_shade < 0)
							end_shade = 0;

					 add_ribbon_vertex(end_centre_x + adjusted_side_dist_x,// * flash_size,// * end_size_proportion,
																		      end_centre_y + adjusted_side_dist_y,// * flash_size,// * end_size_proportion,
																		      colours.packet [col] [end_shade]);// - i]);

					 add_ribbon_vertex(end_centre_x - adjusted_side_dist_x,// * flash_size,// * end_size_proportion,
																		      end_centre_y - adjusted_side_dist_y,// * flash_size,// * end_size_proportion,
																		      colours.packet [col] [end_shade]);// - i]);

						end_centre_x += x_step * 2;
						end_centre_y += y_step * 2;

					 add_ribbon_vertex(end_centre_x + side_dist_x * flash_size * 1.2,// * end_size_proportion,
																		      end_centre_y + side_dist_y * flash_size * 1.2,// * end_size_proportion,
																		      colours.packet [col] [end_shade]);// - i]);

					 add_ribbon_vertex(end_centre_x - side_dist_x * flash_size * 1.2,// * end_size_proportion,
																		      end_centre_y - side_dist_y * flash_size * 1.2,// * end_size_proportion,
																		      colours.packet [col] [end_shade]);// - i]);

					 add_ribbon_vertex(end_centre_x + cos(packet_angle + PI) * flash_size,
																		      end_centre_y + sin(packet_angle + PI) * flash_size,
																		      colours.packet [col] [end_shade]);// - i]);


					}
					 else
						{

					 add_ribbon_vertex(x + x_step * actual_length + adjusted_side_dist_x,
																		      y + y_step * actual_length + adjusted_side_dist_y,
																		      colours.packet [col] [0]);// - i]);

					 add_ribbon_vertex(x + x_step * actual_length - adjusted_side_dist_x,
																		      y + y_step * actual_length - adjusted_side_dist_y,
																		      colours.packet [col] [0]);// - i]);

						}

				return;

}



static void draw_fragment_tail(float x, float y,
																												float x_step, float y_step,
																												float packet_angle,
																												int packet_time,
																												int start_time,
																												int end_time,
																												int total_time,
																												float tail_width,
																												int col,
																												int shade,
																												int drand_seed,
																												int packet_size,
																												float blob_scale)
{

//     al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], x + 40, y + 24 + shade * 2, ALLEGRO_ALIGN_CENTRE, "s %i e %i t %i sd %i", start_time, end_time, total_time, drand_seed + start_time);

     float min_blob_size = total_time - start_time;

     if (min_blob_size > 12)
						min_blob_size = 12;

     seed_drand(drand_seed + start_time);
     float blob_size = (drand(4, 1) + min_blob_size) * blob_scale;

					float side_dist_x = cos(packet_angle + PI/2) * view.zoom;
					float side_dist_y = sin(packet_angle + PI/2) * view.zoom;
					float side2_dist_x = cos(packet_angle - PI/2) * view.zoom;
					float side2_dist_y = sin(packet_angle - PI/2) * view.zoom;

					int adjusted_shade = shade - start_time;

					if (adjusted_shade < 0)
						return;

						float tail_size = tail_width * 30;//(total_time - i) * (float) ((drr % 3 + 2) * tail_width);

  	  double_circle_with_bloom(1, x, y, blob_size, col, adjusted_shade);

   		start_ribbon(2,
																		x + cos(packet_angle) * (tail_size) * view.zoom,
																		y + sin(packet_angle) * (tail_size) * view.zoom,
																		x + side_dist_x * tail_size,
																		y + side_dist_y * tail_size,
																		colours.packet [col] [adjusted_shade]);
					add_ribbon_vertex(x + side2_dist_x * tail_size,
																		     y + side2_dist_y * tail_size,
																		     colours.packet [col] [adjusted_shade]);

     int i;

     seed_drand(drand_seed + start_time + 1);

     x = x - cos(packet_angle) * (5+packet_size) * view.zoom;
     y = y - sin(packet_angle) * (5+packet_size) * view.zoom;

					int drr;

     for (i = start_time; i < end_time; i ++)
					{

						x_step /= 0.94; // see the drag effect in run_fragments() in g_cloud.c
						y_step /= 0.94; //  (0.94 is the drag multiplier)

						drr = drand(25, 1);

//						float tail_size = tail_width * 50;//(total_time - i) * (float) ((drr % 3 + 2) * tail_width);

//						float tail_size2 = (total_time - i) * (float) ((drr / 5 + 2) * tail_width);

      float ribbon_centre_x, ribbon_centre_y;

      if (drr & 1)//drand(2, 0))
						{
							ribbon_centre_x = x + side_dist_x * 3;
							ribbon_centre_y = y + side_dist_y * 3;
						}
						 else
						 {
							 ribbon_centre_x = x - side_dist_x * 3;
							 ribbon_centre_y = y - side_dist_y * 3;
						 }

					add_ribbon_vertex(ribbon_centre_x + side_dist_x * tail_size,
																		     ribbon_centre_y + side_dist_y * tail_size,
																		     colours.packet [col] [adjusted_shade]);// - i]);

//					if (pulse_or_burst)
//						tail_size = (total_time - i) * (float) ((drr / 5 + 2) * tail_width);

					add_ribbon_vertex(ribbon_centre_x + side2_dist_x * tail_size,
																		     ribbon_centre_y + side2_dist_y * tail_size,
																		     colours.packet [col] [adjusted_shade]);// - i]);

						x += x_step;
						y += y_step;

						adjusted_shade --;
						if (adjusted_shade < 0)
							break;

						tail_size *= 0.98;

					}

					add_ribbon_vertex(x,
																		     y,
																		     colours.packet [col] [0]);

}



/*
// called by both pulse packet drawing code, and pulse tail cloud drawing code (which draws the tail of a pulse packet that has been destroyed)
static void draw_new_pulse_tail(float x, float y,
																												float x_step, float y_step,
																												float packet_angle,
																												int packet_time,
																												int start_time,
																												int end_time,
																												int total_time,
																												float tail_width,
																												int col,
																												int shade,
																												int drand_seed,
																												int packet_size,
																												float blob_scale,
																												int pulse_or_burst)
{

//pulse_or_burst = 0;

//     seed_drand(drand_seed + start_time);
//     float blob_size = (3 + packet_size) * blob_scale;;//(drand(4, 1) + 4 + packet_size) * blob_scale;

					float side_dist_x = cos(packet_angle + PI/2);
					float side_dist_y = sin(packet_angle + PI/2);

					float blob_multiplier = blob_scale * tail_width * view.zoom;

   		start_ribbon(3,
																		x + cos(packet_angle) * blob_multiplier,
																		y + sin(packet_angle) * blob_multiplier,
																		x + cos(packet_angle + PI/5) * blob_multiplier * 0.7,
																		y + sin(packet_angle + PI/5) * blob_multiplier * 0.7,
																		colours.packet [col] [shade]);

					add_ribbon_vertex(x + cos(packet_angle - PI/5) * blob_multiplier * 0.7,
																		     y + sin(packet_angle - PI/5) * blob_multiplier * 0.7,
																		     colours.packet [col] [shade]);

					add_ribbon_vertex(x + side_dist_x * blob_multiplier * 0.5,
																		     y + side_dist_y * blob_multiplier * 0.5,
																		     colours.packet [col] [shade - 2]);

					add_ribbon_vertex(x - side_dist_x * blob_multiplier * 0.5,
																		     y - side_dist_y * blob_multiplier * 0.5,
																		     colours.packet [col] [shade - 2]);


					side_dist_x *= view.zoom;
					side_dist_y *= view.zoom;

					side_dist_x *= tail_width;
					side_dist_y *= tail_width;

     int reduced_shade = shade - 4;

     if (reduced_shade < 0)
						reduced_shade = 0;

					add_ribbon_vertex(x + side_dist_x + x_step,
																		     y + side_dist_y + y_step,
																		     colours.packet [col] [reduced_shade]);
					add_ribbon_vertex(x - side_dist_x + x_step,
																		     y - side_dist_y + y_step,
																		     colours.packet [col] [reduced_shade]);

     int end_shade = shade - end_time - (start_time * 2);

//     if (end_shade < 0)
						end_shade = 0;

					int time_left_before_end_fades = total_time - end_time - start_time;

					if (time_left_before_end_fades > 0)
					{


      float end_centre_x = x + x_step * end_time;
      float end_centre_y = y + y_step * end_time;

      float flash_size = time_left_before_end_fades * tail_width * 0.1 * view.zoom;

					 add_ribbon_vertex(x + x_step * end_time + side_dist_x,// * flash_size,// * end_size_proportion,
																		      y + y_step * end_time + side_dist_y,// * flash_size,// * end_size_proportion,
																		      colours.packet [col] [0]);// - i]);

					 add_ribbon_vertex(x + x_step * end_time - side_dist_x,// * flash_size,// * end_size_proportion,
																		      y + y_step * end_time - side_dist_y,// * flash_size,// * end_size_proportion,
																		      colours.packet [col] [0]);// - i]);

					 add_ribbon_vertex(end_centre_x - cos(packet_angle - PI/3) * flash_size,
																		      end_centre_y - sin(packet_angle - PI/3) * flash_size,
																		      colours.packet [col] [0]);// - i]);
					 add_ribbon_vertex(end_centre_x - cos(packet_angle + PI/3) * flash_size,
																		      end_centre_y - sin(packet_angle + PI/3) * flash_size,
																		      colours.packet [col] [0]);// - i]);
					 add_ribbon_vertex(end_centre_x + cos(packet_angle + PI) * flash_size,
																		      end_centre_y + sin(packet_angle + PI) * flash_size,
																		      colours.packet [col] [0]);// - i]);


						return;

					}

					float main_tail_steps = end_time;// - 7;


					 add_ribbon_vertex(x + x_step * main_tail_steps + side_dist_x,
																		      y + y_step * main_tail_steps + side_dist_y,
																		      colours.packet [col] [end_shade]);// - i]);

					 add_ribbon_vertex(x + x_step * main_tail_steps - side_dist_x,
																		      y + y_step * main_tail_steps - side_dist_y,
																		      colours.packet [col] [end_shade]);// - i]);

					 add_ribbon_vertex(x + x_step * end_time,
																		      y + y_step * end_time,
																		      colours.packet [col] [end_shade]);// - i]);




				return;

}

*/



static void draw_new_move_tail(float x, float y,
																												float move_angle,
																												float move_power,
																												int col,
																												int shade,
																												int move_time,
																												int drand_seed)
{

//pulse_or_burst = 0;

//     seed_drand(drand_seed + start_time);
//     float blob_size = (3 + packet_size) * blob_scale;;//(drand(4, 1) + 4 + packet_size) * blob_scale;

//					float side_dist_x = cos(packet_angle + PI/2);
//					float side_dist_y = sin(packet_angle + PI/2);

//					float blob_multiplier = blob_scale * tail_width * view.zoom;

     float tail_size = move_power * view.zoom;
//     int shade = 25;

   		start_ribbon(3,
                  x + cos(move_angle - PI/2) * tail_size * 0.3,
																		y + sin(move_angle - PI/2) * tail_size * 0.3,
																		x + cos(move_angle + PI/2) * tail_size * 0.3,
																		y + sin(move_angle + PI/2) * tail_size * 0.3,
																		colours.packet [col] [shade]);
/*
					add_ribbon_vertex(x + cos(move_angle + PI/5) * tail_size * 0.7,
																		     y + sin(move_angle + PI/5) * tail_size * 0.7,
																		     colours.packet [col] [shade]);*/

					int second_shade = shade - 6;

					if (second_shade < 2)
						second_shade = 2;

					add_ribbon_vertex(x + cos(move_angle - PI/2 - PI/5) * tail_size * 0.7,
																		     y + sin(move_angle - PI/2 - PI/5) * tail_size * 0.7,
																		     colours.packet [col] [second_shade]);

					add_ribbon_vertex(x + cos(move_angle + PI/2 + PI/5) * tail_size * 0.7,
																		     y + sin(move_angle + PI/2 + PI/5) * tail_size * 0.7,
																		     colours.packet [col] [second_shade]);

					float end_angle = move_power * 0.02 - 0.1;

					if (end_angle < 0)
					{

					add_ribbon_vertex(x + cos(move_angle + PI) * tail_size * 6.7,
																		     y + sin(move_angle + PI) * tail_size * 6.7,
																		     colours.packet [col] [0]);

					return;


					}

					add_ribbon_vertex(x + cos(move_angle + PI + end_angle) * tail_size * 6.7,
																		     y + sin(move_angle + PI + end_angle) * tail_size * 6.7,
																		     colours.packet [col] [0]);

					add_ribbon_vertex(x + cos(move_angle + PI - end_angle) * tail_size * 6.7,
																		     y + sin(move_angle + PI - end_angle) * tail_size * 6.7,
																		     colours.packet [col] [0]);

}




/*


// called by both pulse packet drawing code, and pulse tail cloud drawing code (which draws the tail of a pulse packet that has been destroyed)
static void draw_pulse_tail(float x, float y,
																												float x_step, float y_step,
																												float packet_angle,
																												int packet_time,
																												int start_time,
																												int end_time,
																												int total_time,
																												float tail_width,
																												int col,
																												int shade,
																												int drand_seed)
{



     seed_drand(pk - (packet_time));
     float blob_size = (drand(4 + pack->status*2, 1)) * 0.2;


   		start_ribbon(2,
																		x + cos(packet_angle) * (blob_size+3) * view.zoom,
																		y + sin(packet_angle) * (blob_size+3) * view.zoom,
																		x + cos(packet_angle + PI/4) * blob_size * view.zoom,
																		y + sin(packet_angle + PI/4) * blob_size * view.zoom,
																		colours.packet [pack->colour] [16]);
					add_ribbon_vertex(x + cos(packet_angle - PI/4) * blob_size * view.zoom,
																		     y + sin(packet_angle - PI/4) * blob_size * view.zoom,
																		     colours.packet [pack->colour] [16]);

     float x_step = al_fixtof(0 - pack->speed.x) * view.zoom;
     float y_step = al_fixtof(0 - pack->speed.y) * view.zoom;

     int end_time = packet_time;
     int max_time = 8 + pack->status * 3;
     if (end_time > max_time)
						end_time = max_time;
					float tail_width = 0.25;//(pack->status + 4) * 0.04;

						x += x_step;
						y += y_step;

     draw_pulse_tail(x, y, x_step, y_step,
																					packet_angle,
																					packet_time,
																					0, // start_time
																					end_time,
																					max_time,
																					tail_width,
																					pack->colour,
																					12, // shade
																					pk - packet_time);



 int i;

     seed_drand(drand_seed);

     x = x - cos(packet_angle) * 8 * view.zoom;
     y = y - sin(packet_angle) * 8 * view.zoom;

     float save_x = x;
     float save_y = y;

					int drr;

					float side_dist_x = cos(packet_angle + PI/2) * view.zoom;
					float side_dist_y = sin(packet_angle + PI/2) * view.zoom;
					float side2_dist_x = cos(packet_angle - PI/2) * view.zoom;
					float side2_dist_y = sin(packet_angle - PI/2) * view.zoom;

     for (i = start_time; i < end_time; i ++)
					{

						drr = drand(25, 1);

						float tail_size = (total_time - i) * (float) ((drr % 5 + 2) * tail_width);
						float tail_size2 = (total_time - i) * (float) ((drr / 5 + 2) * tail_width);

					add_ribbon_vertex(x + side_dist_x * tail_size,
																		     y + side_dist_y * tail_size,
																		     colours.packet [col] [shade]);// - i]);
					add_ribbon_vertex(x + side2_dist_x * tail_size2,
																		     y + side2_dist_y * tail_size2,
																		     colours.packet [col] [shade]);// - i]);

						x += x_step;// * 3;
						y += y_step;// * 3;

					}

					add_ribbon_vertex(x,
																		     y,
																		     colours.packet [col] [0]);

}


*/

static void draw_notional_group(struct template_struct* draw_templ, al_fixed world_x, al_fixed world_y, al_fixed build_angle, float notional_zoom, int draw_col_index, int draw_col_index_error)
{

 int i, j;
 float x, y;

 ALLEGRO_COLOR* draw_colour;

		for (i = 0; i < GROUP_MAX_MEMBERS; i++)
		{
			if (draw_templ->member [i].exists)
			{
				al_fixed notional_proc_angle = build_angle + draw_templ->member [i].approximate_angle_offset;
    x = al_fixtof(world_x + fixed_xpart(notional_proc_angle, draw_templ->member [i].approximate_distance) - view.camera_x) * notional_zoom;
    y = al_fixtof(world_y + fixed_ypart(notional_proc_angle, draw_templ->member [i].approximate_distance) - view.camera_y) * notional_zoom;
//    y = al_fixtof(command.build_position.y - view.camera_y) * notional_zoom;
    x += view.window_x_unzoomed / 2;
    y += view.window_y_unzoomed / 2;

    if (x < -200 || x > view.window_x_unzoomed + 200
     || y < -200 || y > view.window_y_unzoomed + 200)
      continue;

    draw_colour = colours.plan_col [draw_col_index];

    if (command.build_member_collision [i])
					draw_colour = colours.plan_col [draw_col_index_error];

// This draws the notional proc to be built:
    draw_proc_shape(x, y, build_angle + draw_templ->member [i].group_angle_offset,
																				draw_templ->member [i].shape,
                    0,
                    notional_zoom, draw_colour); //colours.packet [game.user_player_index]);


   for (j = 0; j < MAX_LINKS; j ++)
			{
				switch(draw_templ->member [i].object[j].type)
				{
				 case OBJECT_TYPE_UPLINK: // note downlink isn't drawn

						break;
					case OBJECT_TYPE_NONE:
						break;
					default:
      draw_object(x, y,
																					build_angle + draw_templ->member [i].group_angle_offset,
																					draw_templ->member [i].shape,
																					&draw_templ->member [i].object[j],
																					NULL, //&pr->object_instance[i],
																					NULL, // core
																					NULL, // pr,
																					j,
																					draw_colour,
//																					colours.packet [game.user_player_index],
																			  notional_zoom);

						break;


				}
			}

			}
		}


} // end draw_notional_group()



static void draw_command_marker(int core_index)
{

	struct core_struct* core = &w.core[core_index];

	int member_proc_index;
 int time_since_selection;

 int queue_index;
 float x, y;


 for (queue_index = 0; queue_index < COMMAND_QUEUE; queue_index ++)
	{
	 switch(core->command_queue [queue_index].type)
	 {
	  default:
	  case COM_NONE:
 		 return; // note return (not break or continue)
		 case COM_LOCATION:
    x = (core->command_queue [queue_index].x - al_fixtof(view.camera_x)) * view.zoom;
    y = (core->command_queue [queue_index].y - al_fixtof(view.camera_y)) * view.zoom;

    x += view.window_x_unzoomed / 2;
    y += view.window_y_unzoomed / 2;

    if (x < -200 || x > view.window_x_unzoomed + 200
     || y < -200 || y > view.window_y_unzoomed + 200)
      continue;

				time_since_selection = game.total_time - core->command_queue [queue_index].command_time;
				if (time_since_selection > 12)
					time_since_selection = 12;

//  al_draw_rectangle(x - marker_line_dist, y - marker_line_dist, x + marker_line_dist, y + marker_line_dist, colours.base_trans [COL_RED] [SHADE_HIGH] [5], 0);
    select_arrows(3, x, y,
				  												game.total_time * -0.02,
																		(60.5 - time_since_selection * 3) * view.zoom, // radius
																  11,
																  PI/5,
																  16,
																  colours.base [COL_AQUA] [SHADE_MAX]);

			 break;
		 case COM_TARGET:
				 if (w.core[core->command_queue[queue_index].target_core].exists == 0
 					|| w.core[core->command_queue[queue_index].target_core].created_timestamp != core->command_queue[queue_index].target_core_created
 					|| !check_proc_visible_to_user(w.core[core->command_queue[queue_index].target_core].process_index))
					 continue;
				 if (core->command_queue[queue_index].target_member == -1)
 				 member_proc_index = w.core[core->command_queue[queue_index].target_core].process_index;
				   else
							{
								if (!w.core[core->command_queue[queue_index].target_core].group_member [core->command_queue[queue_index].target_member].exists)
									member_proc_index = -1;
								  else
 				      member_proc_index = w.core[core->command_queue[queue_index].target_core].group_member [core->command_queue[queue_index].target_member].index;
							}
				 if (member_proc_index == -1) // possible if member destroyed
 					continue; // or highlight core?

     x = (al_fixtof(w.proc[member_proc_index].position.x - view.camera_x)) * view.zoom;
     y = (al_fixtof(w.proc[member_proc_index].position.y - view.camera_y)) * view.zoom;

     x += view.window_x_unzoomed / 2;
     y += view.window_y_unzoomed / 2;

     if (x < -200 || x > view.window_x_unzoomed + 200
      || y < -200 || y > view.window_y_unzoomed + 200)
       continue;

				 time_since_selection = game.total_time - core->command_queue [queue_index].command_time;
				 if (time_since_selection > 12)
 					time_since_selection = 12;

     select_arrows(3, x, y,
				  												 game.total_time * -0.02,
																		 (90.5 - time_since_selection * 3) * view.zoom, // radius
																   11, // out_dist
																   PI/5, // side_angle
																   19, // side_dist
																   colours.base [COL_RED] [SHADE_MAX]);

			 break;
		 case COM_FRIEND:
				 if (w.core[core->command_queue[queue_index].target_core].exists == 0
 					|| w.core[core->command_queue[queue_index].target_core].created_timestamp != core->command_queue[queue_index].target_core_created)
					 continue;
// ignore target_member - just highlight core
				 member_proc_index = w.core[core->command_queue[queue_index].target_core].process_index;

     x = (al_fixtof(w.proc[member_proc_index].position.x - view.camera_x)) * view.zoom;
     y = (al_fixtof(w.proc[member_proc_index].position.y - view.camera_y)) * view.zoom;

     x += view.window_x_unzoomed / 2;
     y += view.window_y_unzoomed / 2;

     if (x < -200 || x > view.window_x_unzoomed + 200
      || y < -200 || y > view.window_y_unzoomed + 200)
       continue;

				 time_since_selection = game.total_time - core->command_queue [queue_index].command_time;
				 if (time_since_selection > 12)
 					time_since_selection = 12;

     select_arrows(3, x, y,
				  												 game.total_time * -0.02,
																		 (90.5 - time_since_selection * 3) * view.zoom, // radius
																   11, // out_dist
																   PI/5, // side_angle
																   19, // side_dist
																   colours.base [COL_CYAN] [SHADE_MAX]);
			 break;
		 case COM_DATA_WELL:
    x = (core->command_queue [queue_index].x - al_fixtof(view.camera_x)) * view.zoom;
    y = (core->command_queue [queue_index].y - al_fixtof(view.camera_y)) * view.zoom;

    x += view.window_x_unzoomed / 2;
    y += view.window_y_unzoomed / 2;

    if (x < -200 || x > view.window_x_unzoomed + 200
     || y < -200 || y > view.window_y_unzoomed + 200)
      continue;

				time_since_selection = game.total_time - core->command_queue [queue_index].command_time;
				if (time_since_selection > 12)
					time_since_selection = 12;

//  al_draw_rectangle(x - marker_line_dist, y - marker_line_dist, x + marker_line_dist, y + marker_line_dist, colours.base_trans [COL_RED] [SHADE_HIGH] [5], 0);
    select_arrows(6, x, y,
				  												game.total_time * -0.02,
																		(80.5 - time_since_selection * 3) * view.zoom, // radius
																  11,
																  PI/5,
																  16,
																  colours.base [COL_YELLOW] [SHADE_MAX]);

			 break;
	 }


	} // end queue_index loop

}


static void select_arrows(int number, float centre_x, float centre_y, float select_arrow_angle, float dist, float out_dist, float side_angle, float side_dist, ALLEGRO_COLOR arrow_col)
{

#ifdef RECORDING_VIDEO_2
return;
#endif

	 float angle_inc = (PI * 2) / number;
	 int i;
	 float arrow_x, arrow_y;

				for (i = 0; i < number; i ++)
				{
					arrow_x = centre_x + cos(select_arrow_angle) * dist;
					arrow_y = centre_y + sin(select_arrow_angle) * dist;
					const int m = vbuf.vertex_pos_triangle;
					add_tri_vertex(arrow_x, arrow_y, arrow_col);
					add_tri_vertex(arrow_x + cos(select_arrow_angle + side_angle) * side_dist * view.zoom,
								arrow_y + sin(select_arrow_angle + side_angle) * side_dist * view.zoom, arrow_col);
					add_tri_vertex(arrow_x + cos(select_arrow_angle) * out_dist * view.zoom,
								arrow_y + sin(select_arrow_angle) * out_dist * view.zoom, arrow_col);
					add_tri_vertex(arrow_x + cos(select_arrow_angle - side_angle) * side_dist * view.zoom,
								arrow_y + sin(select_arrow_angle - side_angle) * side_dist * view.zoom, arrow_col);
					construct_triangle(4, m, m+1, m+2);
					construct_triangle(4, m+2, m+3, m);
					select_arrow_angle += angle_inc;
				}

}



struct radial_state_struct
{
	float centre_x, centre_y;
	int layer;
	int vertex_counter;
	ALLEGRO_COLOR fill_col;
};

struct radial_state_struct radstate;


static void start_radial(float x, float y, int layer, ALLEGRO_COLOR fill_col)
{
	radstate.centre_x = x;
	radstate.centre_y = y;
	radstate.layer = layer;
	radstate.vertex_counter = 0;
	radstate.fill_col = fill_col;

}

static void add_radial_vertex(float r_angle, float r_dist)
{
	vertex_list [radstate.vertex_counter] [0] = radstate.centre_x + (cos(r_angle) * r_dist * view.zoom);
	vertex_list [radstate.vertex_counter] [1] = radstate.centre_y + (sin(r_angle) * r_dist * view.zoom);
	radstate.vertex_counter ++;
}


/*
static void add_radial_vertex_xy(float x_offset, float y_offset)
{
	vertex_list [radstate.vertex_counter] [0] = radstate.centre_x + (x_offset * view.zoom);
	vertex_list [radstate.vertex_counter] [1] = radstate.centre_y + (y_offset * view.zoom);
	radstate.vertex_counter ++;

}

static void add_radial_vertex_unzoomed(float r_angle, float r_dist)
{
	vertex_list [radstate.vertex_counter] [0] = radstate.centre_x + (cos(r_angle) * r_dist);
	vertex_list [radstate.vertex_counter] [1] = radstate.centre_y + (sin(r_angle) * r_dist);
	radstate.vertex_counter ++;
}
*/


static void finish_radial(void)
{
 add_poly_layer(radstate.layer, radstate.vertex_counter, radstate.fill_col);
}

static void start_ribbon(int layer, float vertex1_x, float vertex1_y, float vertex2_x, float vertex2_y, ALLEGRO_COLOR fill_col)
{
	ribstate.layer = layer;
	ribstate.fill_col = fill_col;
	ribstate.last_vertex_index = vbuf.vertex_pos_triangle;
	ribstate.current_vertex_index = vbuf.vertex_pos_triangle + 1;

	add_tri_vertex(vertex1_x, vertex1_y, fill_col);
	add_tri_vertex(vertex2_x, vertex2_y, fill_col);

};

/*
static void add_ribbon_vertex_vector(float base_x, float base_y, float angle, float dist, ALLEGRO_COLOR vertex_col)
{
	add_ribbon_vertex(base_x + cos(angle) * dist * view.zoom, base_y + sin(angle) * dist * view.zoom, vertex_col);
}*/

static void add_ribbon_vertex(float x, float y, ALLEGRO_COLOR vertex_col)
{
	int tmp = ribstate.last_vertex_index;
	ribstate.last_vertex_index = ribstate.current_vertex_index;
	ribstate.current_vertex_index = vbuf.vertex_pos_triangle;

	add_tri_vertex(x, y, vertex_col);

	construct_triangle(ribstate.layer, tmp, ribstate.last_vertex_index, ribstate.current_vertex_index);

// it shouldn't be necessary to do anything in particular to finish the ribbon.

}


static void start_bloom_ribbon(int layer, float centre_x, float centre_y, float left_x, float left_y, float right_x, float right_y, ALLEGRO_COLOR centre_col, ALLEGRO_COLOR edge_col)
{
	bribstate.layer = layer;
	bribstate.vertex_col [0] = centre_col;
	bribstate.vertex_col [1] = edge_col;
	bribstate.vertex_pos = 0;

	bribstate.vertex_x [bribstate.vertex_pos] [0] = centre_x;
	bribstate.vertex_y [bribstate.vertex_pos] [0] = centre_y;
	bribstate.vertex_x [bribstate.vertex_pos] [1] = left_x;
	bribstate.vertex_y [bribstate.vertex_pos] [1] = left_y;
	bribstate.vertex_x [bribstate.vertex_pos] [2] = right_x;
	bribstate.vertex_y [bribstate.vertex_pos] [2] = right_y;

	bribstate.vertex_pos ++;

};

static void add_bloom_ribbon_vertices(float centre_x, float centre_y, float left_x, float left_y, float right_x, float right_y)
{

 if (bribstate.vertex_pos >= BLOOM_RIBBON_VERTICES)
	{
		finish_bloom_ribbon();
 	bribstate.vertex_pos = 0;
// now just carry on as before. Might be a seam but it probably won't be too noticeable.
	}

	bribstate.vertex_x [bribstate.vertex_pos] [0] = centre_x;
	bribstate.vertex_y [bribstate.vertex_pos] [0] = centre_y;
	bribstate.vertex_x [bribstate.vertex_pos] [1] = left_x;
	bribstate.vertex_y [bribstate.vertex_pos] [1] = left_y;
	bribstate.vertex_x [bribstate.vertex_pos] [2] = right_x;
	bribstate.vertex_y [bribstate.vertex_pos] [2] = right_y;

	bribstate.vertex_pos ++;

}

// assumes that bribstate.vertex_pos is at least 1
static void finish_bloom_ribbon(void)
{
	int i, m = vbuf.vertex_pos_triangle;

// first do left-hand side of ribbon:

	for (i = 0; i < bribstate.vertex_pos; ++i)
	{
		vbuf.buffer_triangle[m+3*i].x = bribstate.vertex_x [i] [0];
		vbuf.buffer_triangle[m+3*i].y = bribstate.vertex_y [i] [0];
		vbuf.buffer_triangle[m+3*i].color = bribstate.vertex_col [0];

		vbuf.buffer_triangle[m+3*i+1].x = bribstate.vertex_x [i] [1];
		vbuf.buffer_triangle[m+3*i+1].y = bribstate.vertex_y [i] [1];
		vbuf.buffer_triangle[m+3*i+1].color = bribstate.vertex_col [1];

		vbuf.buffer_triangle[m+3*i+2].x = bribstate.vertex_x [i] [2];
		vbuf.buffer_triangle[m+3*i+2].y = bribstate.vertex_y [i] [2];
		vbuf.buffer_triangle[m+3*i+2].color = bribstate.vertex_col [1];
	}

	for (i = 0; i < bribstate.vertex_pos - 1; ++i)
	{
		construct_triangle(bribstate.layer, m+3*i, m+3*i+1, m+3*i+3);
		construct_triangle(bribstate.layer, m+3*i+1, m+3*i+3, m+3*i+4);

		// now do right-hand side of ribbon:
		construct_triangle(bribstate.layer, m+3*i, m+3*i+2, m+3*i+3);
		construct_triangle(bribstate.layer, m+3*i+2, m+3*i+3, m+3*i+5);
	}

	vbuf.vertex_pos_triangle += 3*bribstate.vertex_pos;

}

static void draw_ring(int layer,
																						float x, float y,
																						float size,
																						float thickness,
																						int vertices,
																						ALLEGRO_COLOR ring_col)
{

	size *= view.zoom;
	thickness *= view.zoom;

	start_ribbon(layer, x + size, y, x + size - thickness, y, ring_col);

	float angle_inc = (PI*2)/vertices;

	int i;

	for (i = 1; i < vertices + 1; i ++) // note i = 1
	{
		add_ribbon_vertex(x + cos(angle_inc * i) * size,
																				y + sin(angle_inc * i) * size,
																				ring_col);
		add_ribbon_vertex(x + cos(angle_inc * i) * (size - thickness),
																				y + sin(angle_inc * i) * (size - thickness),
																				ring_col);
	}

}

/*
#define BURST_VERTICES 10

static void draw_burst(float x, float y,
																							float size,
																							float angle,
																							int time,
																							int lifetime,
																							ALLEGRO_COLOR col_bright,
																							ALLEGRO_COLOR col_dim)
{

// float burst_vertex [BURST_VERTICES] [2] [2];

 int i;
 float time_proportion = (float) (lifetime - time) / lifetime;

 float x2 = x - cos(angle) * (size * time_proportion) * view.zoom;
 float y2 = y - sin(angle) * (size * time_proportion) * view.zoom;

 float inner_x, inner_y, outer_x, outer_y;
 float angle_inc = (PI*2) / BURST_VERTICES;
 float last_inner_x, last_inner_y, last_outer_x, last_outer_y;

	float point_angle = angle - angle_inc;
 last_outer_x = x + cos(point_angle) * size * view.zoom;
 last_outer_y = y + sin(point_angle) * size * view.zoom;
 last_inner_x = x2 + cos(point_angle) * (size * (1 - time_proportion)) * view.zoom;
 last_inner_y = y2 + sin(point_angle) * (size * (1 - time_proportion)) * view.zoom;

 for (i = 0; i < BURST_VERTICES; i ++)
	{
		point_angle = angle + (angle_inc*i);
  outer_x = x + cos(point_angle) * size * view.zoom;
  outer_y = y + sin(point_angle) * size * view.zoom;
  inner_x = x2 + cos(point_angle) * (size * (1 - time_proportion)) * view.zoom;
  inner_y = y2 + sin(point_angle) * (size * (1 - time_proportion)) * view.zoom;

  add_triangle(2, inner_x, inner_y, last_outer_x, last_outer_y, last_inner_x, last_inner_y,
															col_dim);

  add_triangle(2, inner_x, inner_y, outer_x, outer_y, last_outer_x, last_outer_y,
															col_dim);

		last_inner_x = inner_x;
		last_inner_y = inner_y;
		last_outer_x = outer_x;
		last_outer_y = outer_y;

	}

}
*/

// This function produces pseudo-random numbers from proc properties.
// It is designed to return the same value in each game tick, so that while the game is paused
//  things stop animating.
// It's only used in display functions where it can't affect gameplay, so its extremely low quality doesn't matter
// Also, it's not saved in saved games.
// Assumes mod is not zero
/*
static unsigned int proc_rand(struct proc_struct* pr, int special, int mod)
{

// The mixture of int and al_fixed shouldn't matter.
 return (pr->position.x + pr->position.y + special + w.core[pr->core_index].execution_count) % mod;

}*/

void clear_vbuf(void)
{

 int i;

 vbuf.vertex_pos_line = 0;
 vbuf.vertex_pos_triangle = 0;

 for (i = 0; i < DISPLAY_LAYERS; i ++)
 {
 	vbuf.index_pos_triangle [i] = 0;
 	vbuf.index_pos_line [i] = 0;
 }
}
/*
static unsigned int packet_rand(struct packet_struct* pack, int mod)
{
 return (pack->position.x + pack->position.y) % mod;
}*/


static void draw_spray(float x, float y, float spray_size, int base_bit_size, int player_index, int shade, int time_elapsed, int max_time, int spray_bits, int drand_seed)
{

	int i;

	float spray_angle_inc = (PI * 2) / spray_bits;

	seed_drand(drand_seed);
	float bit_size_base = (((max_time - time_elapsed) * base_bit_size) / max_time) * view.zoom;

	for (i = 0; i < spray_bits; i ++)
	{
		float bit_angle = spray_angle_inc * i + drand(10, 1) * 0.2;
		float bit_centre_dist = (time_elapsed + drand(5, 1)) * spray_size * view.zoom;
		float bit_size = bit_size_base;
		float bit_inner_dist = bit_centre_dist / 2;//(time_elapsed + drand(5, 1)) * zoomed_size * 0.5;
		float bit_cos = cos(bit_angle);
		float bit_sin = sin(bit_angle);
		float bit_centre_x = x + bit_cos * bit_centre_dist;
		float bit_centre_y = y + bit_sin * bit_centre_dist;
		int bit_shade = shade + drand(5, 1);
		if (bit_shade >= CLOUD_SHADES)
			bit_shade = CLOUD_SHADES - 1;

		add_diamond_layer(3,
																				bit_centre_x + bit_cos * bit_size,
																				bit_centre_y + bit_sin * bit_size,
																				bit_centre_x + cos(bit_angle + PI/2) * bit_size,
																				bit_centre_y + sin(bit_angle + PI/2) * bit_size,
																				x + bit_cos * bit_inner_dist,
																				y + bit_sin * bit_inner_dist,
																				bit_centre_x + cos(bit_angle - PI/2) * bit_size,
																				bit_centre_y + sin(bit_angle - PI/2) * bit_size,
																				colours.packet [player_index] [bit_shade]);
	}


}

void draw_stream_beam(float x1, float by1, float x2, float y2, int col, int status, int counter, int hit)
{

 float stream_angle = atan2(y2 - by1, x2 - x1);

// first draw the outer (non-damaging) part of the stream:

 int shade;

	shade = counter;

	float proportion = 1;

	if (counter < STREAM_WARMUP_LENGTH)
	{
		proportion = (float) counter / STREAM_WARMUP_LENGTH;
	}
	 else
		{
			if (counter >= STREAM_TOTAL_FIRING_TIME - STREAM_COOLDOWN_LENGTH)
			{
		  proportion = (float) (STREAM_TOTAL_FIRING_TIME-counter) / STREAM_COOLDOWN_LENGTH;
	   shade = (STREAM_TOTAL_FIRING_TIME-counter) / 2;
			}
			 else
				{
					proportion = (float) (10 + (48 - counter)) * 0.1;
					if (proportion < 1)
						proportion = 1;
				}
		}

	if (shade > CLOUD_SHADES - 15)
		shade = CLOUD_SHADES - 15;

 float beam_base_flash_size = 6 * proportion * view.zoom;
 float beam_width = 4 * proportion * view.zoom;
 float beam_end_flash_size = 6 * proportion * view.zoom;

 if (hit)
		beam_end_flash_size = 9 * proportion * view.zoom;

// back
 vertex_list [0] [0] = x1 + cos(stream_angle + PI) * beam_base_flash_size;
 vertex_list [0] [1] = by1 + sin(stream_angle + PI) * beam_base_flash_size;
// right
 vertex_list [1] [0] = x1 + cos(stream_angle + PI/2) * beam_base_flash_size;
 vertex_list [1] [1] = by1 + sin(stream_angle + PI/2) * beam_base_flash_size;
// left
 vertex_list [4] [0] = x1 + cos(stream_angle - PI/2) * beam_base_flash_size;
 vertex_list [4] [1] = by1 + sin(stream_angle - PI/2) * beam_base_flash_size;
// front (not used directly
	float base_front_x = x1 + cos(stream_angle) * beam_base_flash_size;
	float base_front_y = by1 + sin(stream_angle) * beam_base_flash_size;

// right-front (beam base)
 vertex_list [2] [0] = base_front_x + cos(stream_angle + PI/2) * beam_width;
 vertex_list [2] [1] = base_front_y + sin(stream_angle + PI/2) * beam_width;
// left-front (beam base)
 vertex_list [3] [0] = base_front_x + cos(stream_angle - PI/2) * beam_width;
 vertex_list [3] [1] = base_front_y + sin(stream_angle - PI/2) * beam_width;

 float reverse_stream_angle = stream_angle + PI;

	float end_front_x = x2 + cos(reverse_stream_angle) * beam_end_flash_size;
	float end_front_y = y2 + sin(reverse_stream_angle) * beam_end_flash_size;
// other end:
// beam end base right
 vertex_list [5] [0] = end_front_x + cos(reverse_stream_angle + PI/2) * beam_width;
 vertex_list [5] [1] = end_front_y + sin(reverse_stream_angle + PI/2) * beam_width;
// beam end base left
 vertex_list [6] [0] = end_front_x + cos(reverse_stream_angle - PI/2) * beam_width;
 vertex_list [6] [1] = end_front_y + sin(reverse_stream_angle - PI/2) * beam_width;
// left
 vertex_list [7] [0] = x2 + cos(reverse_stream_angle - PI/2) * beam_end_flash_size;
 vertex_list [7] [1] = y2 + sin(reverse_stream_angle - PI/2) * beam_end_flash_size;
// far end
 vertex_list [8] [0] = x2 + cos(reverse_stream_angle + PI) * beam_end_flash_size;
 vertex_list [8] [1] = y2 + sin(reverse_stream_angle + PI) * beam_end_flash_size;
// right
 vertex_list [9] [0] = x2 + cos(reverse_stream_angle + PI/2) * beam_end_flash_size;
 vertex_list [9] [1] = y2 + sin(reverse_stream_angle + PI/2) * beam_end_flash_size;

 draw_beam_triangles(1, x1, by1, x2, y2, colours.packet [col] [shade]);

// bloom

// bloom_circle(1, x1, by1, colours.bloom_centre [col] [shade], colours.bloom_edge [col] [0], beam_base_flash_size * 10);
// bloom_circle(1, x2, y2, colours.bloom_centre [col] [shade], colours.bloom_edge [col] [0], beam_end_flash_size * 10);

 float bloom_base_size = beam_base_flash_size * 5;
 float bloom_width = beam_width * 5;
 float bloom_end_size = beam_end_flash_size * 5;

// back
 vertex_list [0] [0] = x1 + cos(stream_angle + PI) * bloom_base_size;
 vertex_list [0] [1] = by1 + sin(stream_angle + PI) * bloom_base_size;
// right
 vertex_list [1] [0] = x1 + cos(stream_angle + PI/2) * bloom_base_size;
 vertex_list [1] [1] = by1 + sin(stream_angle + PI/2) * bloom_base_size;
// left
 vertex_list [4] [0] = x1 + cos(stream_angle - PI/2) * bloom_base_size;
 vertex_list [4] [1] = by1 + sin(stream_angle - PI/2) * bloom_base_size;
// front (not used directly
	base_front_x = x1 + cos(stream_angle) * bloom_base_size;
	base_front_y = by1 + sin(stream_angle) * bloom_base_size;

// right-front (beam base)
 vertex_list [2] [0] = base_front_x + cos(stream_angle + PI/2) * bloom_width;
 vertex_list [2] [1] = base_front_y + sin(stream_angle + PI/2) * bloom_width;
// left-front (beam base)
 vertex_list [3] [0] = base_front_x + cos(stream_angle - PI/2) * bloom_width;
 vertex_list [3] [1] = base_front_y + sin(stream_angle - PI/2) * bloom_width;

// float reverse_stream_angle = stream_angle + PI;

	end_front_x = x2 + cos(reverse_stream_angle) * bloom_end_size;
	end_front_y = y2 + sin(reverse_stream_angle) * bloom_end_size;
// other end:
// beam end base right
 vertex_list [5] [0] = end_front_x + cos(reverse_stream_angle + PI/2) * bloom_width;
 vertex_list [5] [1] = end_front_y + sin(reverse_stream_angle + PI/2) * bloom_width;
// beam end base left
 vertex_list [6] [0] = end_front_x + cos(reverse_stream_angle - PI/2) * bloom_width;
 vertex_list [6] [1] = end_front_y + sin(reverse_stream_angle - PI/2) * bloom_width;
// left
 vertex_list [7] [0] = x2 + cos(reverse_stream_angle - PI/2) * bloom_end_size;
 vertex_list [7] [1] = y2 + sin(reverse_stream_angle - PI/2) * bloom_end_size;
// far end
 vertex_list [8] [0] = x2 + cos(reverse_stream_angle + PI) * bloom_end_size;
 vertex_list [8] [1] = y2 + sin(reverse_stream_angle + PI) * bloom_end_size;
// right
 vertex_list [9] [0] = x2 + cos(reverse_stream_angle + PI/2) * bloom_end_size;
 vertex_list [9] [1] = y2 + sin(reverse_stream_angle + PI/2) * bloom_end_size;


 draw_beam_bloom_triangles(2, x1, by1, x2, y2, colours.bloom_centre [col] [shade], colours.bloom_edge [col] [shade]);

// inner stream

 if (counter < STREAM_WARMUP_LENGTH
		|| counter >= (STREAM_TOTAL_FIRING_TIME - STREAM_COOLDOWN_LENGTH))
		return; // no inner stream during warmup

	shade = CLOUD_SHADES - 1;

 beam_base_flash_size = 4 * proportion * view.zoom;
 beam_width = 2 * proportion * view.zoom;
 beam_end_flash_size = 4 * proportion * view.zoom;

 if (hit)
		beam_end_flash_size = 5 * proportion * view.zoom;

// back
 vertex_list [0] [0] = x1 + cos(stream_angle + PI) * beam_base_flash_size;
 vertex_list [0] [1] = by1 + sin(stream_angle + PI) * beam_base_flash_size;
// right
 vertex_list [1] [0] = x1 + cos(stream_angle + PI/2) * beam_base_flash_size;
 vertex_list [1] [1] = by1 + sin(stream_angle + PI/2) * beam_base_flash_size;
// left
 vertex_list [4] [0] = x1 + cos(stream_angle - PI/2) * beam_base_flash_size;
 vertex_list [4] [1] = by1 + sin(stream_angle - PI/2) * beam_base_flash_size;
// front (not used directly
	base_front_x = x1 + cos(stream_angle) * beam_base_flash_size;
	base_front_y = by1 + sin(stream_angle) * beam_base_flash_size;

// right-front (beam base)
 vertex_list [2] [0] = base_front_x + cos(stream_angle + PI/2) * beam_width;
 vertex_list [2] [1] = base_front_y + sin(stream_angle + PI/2) * beam_width;
// left-front (beam base)
 vertex_list [3] [0] = base_front_x + cos(stream_angle - PI/2) * beam_width;
 vertex_list [3] [1] = base_front_y + sin(stream_angle - PI/2) * beam_width;

// float reverse_stream_angle = stream_angle + PI;

	end_front_x = x2 + cos(reverse_stream_angle) * beam_end_flash_size;
	end_front_y = y2 + sin(reverse_stream_angle) * beam_end_flash_size;
// other end:
// beam end base right
 vertex_list [5] [0] = end_front_x + cos(reverse_stream_angle + PI/2) * beam_width;
 vertex_list [5] [1] = end_front_y + sin(reverse_stream_angle + PI/2) * beam_width;
// beam end base left
 vertex_list [6] [0] = end_front_x + cos(reverse_stream_angle - PI/2) * beam_width;
 vertex_list [6] [1] = end_front_y + sin(reverse_stream_angle - PI/2) * beam_width;
// left
 vertex_list [7] [0] = x2 + cos(reverse_stream_angle - PI/2) * beam_end_flash_size;
 vertex_list [7] [1] = y2 + sin(reverse_stream_angle - PI/2) * beam_end_flash_size;
// far end
 vertex_list [8] [0] = x2 + cos(reverse_stream_angle + PI) * beam_end_flash_size;
 vertex_list [8] [1] = y2 + sin(reverse_stream_angle + PI) * beam_end_flash_size;
// right
 vertex_list [9] [0] = x2 + cos(reverse_stream_angle + PI/2) * beam_end_flash_size;
 vertex_list [9] [1] = y2 + sin(reverse_stream_angle + PI/2) * beam_end_flash_size;

 draw_beam_triangles(2, x1, by1, x2, y2, colours.packet [col] [shade]);

// finally, bloom:



}



void draw_slice_beam(float x1, float by1, float x2, float y2, int col, int time_since_firing, int hit)
{

 float stream_angle = atan2(y2 - by1, x2 - x1);

// first draw the outer (non-damaging) part of the stream:

 int shade_base;
 int shade_end;
	int fade_time = time_since_firing - SLICE_FIRING_TIME;

	shade_base = 27 - time_since_firing * 2;
 	if (shade_base < 0)
	 	shade_base = 0;

	shade_end = 31;// - time_since_firing;

	if (fade_time > 0)
		shade_end -= fade_time * 2;
	if (shade_end < 0)
 	shade_end = 0;

/*	if (fade_time > 0)
	{
		shade_base -= fade_time * 4;
 	if (shade_base < 0)
	 	shade_base = 0;
		shade_end -= fade_time * 3;
 	if (shade_end < 0)
	 	shade_end = 0;
	}*/


	float proportion = 2 - (time_since_firing * 0.05);
//	float proportion = 3 - (time_since_firing * 0.1);
/*
	if (counter < STREAM_WARMUP_LENGTH)
	{
		proportion = (float) counter / STREAM_WARMUP_LENGTH;
	}
	 else
		{
			if (counter >= STREAM_TOTAL_FIRING_TIME - STREAM_COOLDOWN_LENGTH)
			{
		  proportion = (float) (STREAM_TOTAL_FIRING_TIME-counter) / STREAM_COOLDOWN_LENGTH;
	   shade = (STREAM_TOTAL_FIRING_TIME-counter) / 2;
			}
			 else
				{
					proportion = (float) (10 + (48 - counter)) * 0.1;
					if (proportion < 1)
						proportion = 1;
				}
		}
*/
//	if (shade > CLOUD_SHADES - 15)
		//shade = CLOUD_SHADES - 15;

 float beam_base_flash_size;
 if (fade_time <= 0)
  beam_base_flash_size = 8 * proportion * view.zoom;
   else
				beam_base_flash_size = 1 * proportion * view.zoom;
 float beam_width = 2 * proportion * view.zoom;
 float beam_end_flash_size = 3 * proportion * view.zoom;

 if (hit)
		beam_end_flash_size = 12 * proportion * view.zoom;

// back
 vertex_list [0] [0] = x1 + cos(stream_angle + PI) * beam_base_flash_size;
 vertex_list [0] [1] = by1 + sin(stream_angle + PI) * beam_base_flash_size;
// right
 vertex_list [1] [0] = x1 + cos(stream_angle + PI/2) * beam_base_flash_size;
 vertex_list [1] [1] = by1 + sin(stream_angle + PI/2) * beam_base_flash_size;
// left
 vertex_list [4] [0] = x1 + cos(stream_angle - PI/2) * beam_base_flash_size;
 vertex_list [4] [1] = by1 + sin(stream_angle - PI/2) * beam_base_flash_size;
// front (not used directly
	float base_front_x = x1 + cos(stream_angle) * beam_base_flash_size;
	float base_front_y = by1 + sin(stream_angle) * beam_base_flash_size;

// right-front (beam base)
 vertex_list [2] [0] = base_front_x + cos(stream_angle + PI/2) * beam_width;
 vertex_list [2] [1] = base_front_y + sin(stream_angle + PI/2) * beam_width;
// left-front (beam base)
 vertex_list [3] [0] = base_front_x + cos(stream_angle - PI/2) * beam_width;
 vertex_list [3] [1] = base_front_y + sin(stream_angle - PI/2) * beam_width;

 float reverse_stream_angle = stream_angle + PI;

	float end_front_x = x2 + cos(reverse_stream_angle) * beam_end_flash_size;
	float end_front_y = y2 + sin(reverse_stream_angle) * beam_end_flash_size;
// other end:
// beam end base right
 vertex_list [5] [0] = end_front_x + cos(reverse_stream_angle + PI/2) * beam_width;
 vertex_list [5] [1] = end_front_y + sin(reverse_stream_angle + PI/2) * beam_width;
// beam end base left
 vertex_list [6] [0] = end_front_x + cos(reverse_stream_angle - PI/2) * beam_width;
 vertex_list [6] [1] = end_front_y + sin(reverse_stream_angle - PI/2) * beam_width;
// left
 vertex_list [7] [0] = x2 + cos(reverse_stream_angle - PI/2) * beam_end_flash_size;
 vertex_list [7] [1] = y2 + sin(reverse_stream_angle - PI/2) * beam_end_flash_size;
// far end
 vertex_list [8] [0] = x2 + cos(reverse_stream_angle + PI) * beam_end_flash_size;
 vertex_list [8] [1] = y2 + sin(reverse_stream_angle + PI) * beam_end_flash_size;
// right
 vertex_list [9] [0] = x2 + cos(reverse_stream_angle + PI/2) * beam_end_flash_size;
 vertex_list [9] [1] = y2 + sin(reverse_stream_angle + PI/2) * beam_end_flash_size;

 if (fade_time <= 0)
  draw_beam_triangles(1, x1, by1, x2, y2, colours.packet [col] [shade_end / 2]);
   else
    draw_fade_slice_triangles(1, x1, by1, x2, y2, colours.packet [col] [shade_end / 2], colours.packet [col] [shade_base / 2]);
//    draw_fade_slice_triangles(layer, x1, by1, float x2, float y2, ALLEGRO_COLOR stream_col, ALLEGRO_COLOR base_col)
// bloom

// bloom_circle(1, x1, by1, colours.bloom_centre [col] [shade], colours.bloom_edge [col] [0], beam_base_flash_size * 10);
// bloom_circle(1, x2, y2, colours.bloom_centre [col] [shade], colours.bloom_edge [col] [0], beam_end_flash_size * 10);

 float bloom_base_size;

 float bloom_proportion = 1;



 if (fade_time <= 0)
  bloom_base_size = beam_base_flash_size * 5;
   else
			{
    bloom_proportion = 1 - (fade_time * 0.0625);
    bloom_base_size = beam_base_flash_size * 8 * bloom_proportion;
			}
 float bloom_width = beam_width * bloom_proportion * 5;
 float bloom_end_size = beam_end_flash_size * bloom_proportion * 5;

// back
 vertex_list [0] [0] = x1 + cos(stream_angle + PI) * bloom_base_size;
 vertex_list [0] [1] = by1 + sin(stream_angle + PI) * bloom_base_size;
// right
 vertex_list [1] [0] = x1 + cos(stream_angle + PI/2) * bloom_base_size;
 vertex_list [1] [1] = by1 + sin(stream_angle + PI/2) * bloom_base_size;
// left
 vertex_list [4] [0] = x1 + cos(stream_angle - PI/2) * bloom_base_size;
 vertex_list [4] [1] = by1 + sin(stream_angle - PI/2) * bloom_base_size;
// front (not used directly
	base_front_x = x1 + cos(stream_angle) * bloom_base_size;
	base_front_y = by1 + sin(stream_angle) * bloom_base_size;

// right-front (beam base)
 vertex_list [2] [0] = base_front_x + cos(stream_angle + PI/2) * bloom_width;
 vertex_list [2] [1] = base_front_y + sin(stream_angle + PI/2) * bloom_width;
// left-front (beam base)
 vertex_list [3] [0] = base_front_x + cos(stream_angle - PI/2) * bloom_width;
 vertex_list [3] [1] = base_front_y + sin(stream_angle - PI/2) * bloom_width;

// float reverse_stream_angle = stream_angle + PI;

	end_front_x = x2 + cos(reverse_stream_angle) * bloom_end_size;
	end_front_y = y2 + sin(reverse_stream_angle) * bloom_end_size;
// other end:
// beam end base right
 vertex_list [5] [0] = end_front_x + cos(reverse_stream_angle + PI/2) * bloom_width;
 vertex_list [5] [1] = end_front_y + sin(reverse_stream_angle + PI/2) * bloom_width;
// beam end base left
 vertex_list [6] [0] = end_front_x + cos(reverse_stream_angle - PI/2) * bloom_width;
 vertex_list [6] [1] = end_front_y + sin(reverse_stream_angle - PI/2) * bloom_width;
// left
 vertex_list [7] [0] = x2 + cos(reverse_stream_angle - PI/2) * bloom_end_size;
 vertex_list [7] [1] = y2 + sin(reverse_stream_angle - PI/2) * bloom_end_size;
// far end
 vertex_list [8] [0] = x2 + cos(reverse_stream_angle + PI) * bloom_end_size;
 vertex_list [8] [1] = y2 + sin(reverse_stream_angle + PI) * bloom_end_size;
// right
 vertex_list [9] [0] = x2 + cos(reverse_stream_angle + PI/2) * bloom_end_size;
 vertex_list [9] [1] = y2 + sin(reverse_stream_angle + PI/2) * bloom_end_size;


 draw_beam_bloom_triangles(2, x1, by1, x2, y2, colours.bloom_centre [col] [shade_base], colours.bloom_edge [col] [shade_base]);

// inner stream
/*
 if (counter < STREAM_WARMUP_LENGTH
		|| counter >= (STREAM_TOTAL_FIRING_TIME - STREAM_COOLDOWN_LENGTH))
		return; // no inner stream during warmup*/
//return;
//	shade = 28;

 if (fade_time <= 0)
  beam_base_flash_size = 6 * proportion * view.zoom;
   else
    beam_base_flash_size = 1 * proportion * view.zoom;
 beam_width = 0.7 * proportion * view.zoom;
 beam_end_flash_size = 2 * proportion * view.zoom;

 if (hit)
		beam_end_flash_size = 8 * proportion * view.zoom;

// back
 vertex_list [0] [0] = x1 + cos(stream_angle + PI) * beam_base_flash_size;
 vertex_list [0] [1] = by1 + sin(stream_angle + PI) * beam_base_flash_size;
// right
 vertex_list [1] [0] = x1 + cos(stream_angle + PI/2) * beam_base_flash_size;
 vertex_list [1] [1] = by1 + sin(stream_angle + PI/2) * beam_base_flash_size;
// left
 vertex_list [4] [0] = x1 + cos(stream_angle - PI/2) * beam_base_flash_size;
 vertex_list [4] [1] = by1 + sin(stream_angle - PI/2) * beam_base_flash_size;
// front (not used directly
	base_front_x = x1 + cos(stream_angle) * beam_base_flash_size;
	base_front_y = by1 + sin(stream_angle) * beam_base_flash_size;

// right-front (beam base)
 vertex_list [2] [0] = base_front_x + cos(stream_angle + PI/2) * beam_width;
 vertex_list [2] [1] = base_front_y + sin(stream_angle + PI/2) * beam_width;
// left-front (beam base)
 vertex_list [3] [0] = base_front_x + cos(stream_angle - PI/2) * beam_width;
 vertex_list [3] [1] = base_front_y + sin(stream_angle - PI/2) * beam_width;

// float reverse_stream_angle = stream_angle + PI;

	end_front_x = x2 + cos(reverse_stream_angle) * beam_end_flash_size;
	end_front_y = y2 + sin(reverse_stream_angle) * beam_end_flash_size;
// other end:
// beam end base right
 vertex_list [5] [0] = end_front_x + cos(reverse_stream_angle + PI/2) * beam_width;
 vertex_list [5] [1] = end_front_y + sin(reverse_stream_angle + PI/2) * beam_width;
// beam end base left
 vertex_list [6] [0] = end_front_x + cos(reverse_stream_angle - PI/2) * beam_width;
 vertex_list [6] [1] = end_front_y + sin(reverse_stream_angle - PI/2) * beam_width;
// left
 vertex_list [7] [0] = x2 + cos(reverse_stream_angle - PI/2) * beam_end_flash_size;
 vertex_list [7] [1] = y2 + sin(reverse_stream_angle - PI/2) * beam_end_flash_size;
// far end
 vertex_list [8] [0] = x2 + cos(reverse_stream_angle + PI) * beam_end_flash_size;
 vertex_list [8] [1] = y2 + sin(reverse_stream_angle + PI) * beam_end_flash_size;
// right
 vertex_list [9] [0] = x2 + cos(reverse_stream_angle + PI/2) * beam_end_flash_size;
 vertex_list [9] [1] = y2 + sin(reverse_stream_angle + PI/2) * beam_end_flash_size;

 if (fade_time <= 0)
  draw_beam_triangles(2, x1, by1, x2, y2, colours.packet [col] [shade_end]);
   else
    draw_fade_slice_triangles(1, x1, by1, x2, y2, colours.packet [col] [shade_end], colours.packet [col] [shade_base]);

// finally, bloom:



}







static void draw_beam_triangles(int layer, float x1, float by1, float x2, float y2, ALLEGRO_COLOR stream_col)
{
	int i, m = vbuf.vertex_pos_triangle;

	for (i = 0; i < 10; ++i)
		add_tri_vertex(vertex_list[i][0], vertex_list[i][1], stream_col);

	add_tri_vertex(x1, by1, stream_col);
	add_tri_vertex(x2, y2, stream_col);

	construct_triangle(layer, m+10, m+3, m+4);
	construct_triangle(layer, m+10, m+4, m);
	construct_triangle(layer, m+10, m, m+1);
	construct_triangle(layer, m+10, m+1, m+2);
	construct_triangle(layer, m+10, m+2, m+3);
	construct_triangle(layer, m+2, m+3, m+5);
	construct_triangle(layer, m+2, m+5, m+6);

// far side
	construct_triangle(layer, m+11, m+5, m+6);
	construct_triangle(layer, m+11, m+6, m+7);
	construct_triangle(layer, m+11, m+7, m+8);
	construct_triangle(layer, m+11, m+8, m+9);
	construct_triangle(layer, m+11, m+9, m+5);

//	add_line(2, x1, by1, x2, y2, colours.base_trans [col] [8] [2]);

}


static void draw_fade_slice_triangles(int layer, float x1, float by1, float x2, float y2, ALLEGRO_COLOR stream_col, ALLEGRO_COLOR base_col)
{
	int i, m = vbuf.vertex_pos_triangle;

	for (i = 0; i < 5; ++i)
		add_tri_vertex(vertex_list[i][0], vertex_list[i][1], base_col);

	for (i = 5; i < 10; ++i)
		add_tri_vertex(vertex_list[i][0], vertex_list[i][1], stream_col);

	add_tri_vertex(x1, by1, base_col);
	add_tri_vertex(x2, y2, stream_col);

	construct_triangle(layer, m+10, m+3, m+4);
	construct_triangle(layer, m+10, m+4, m);
	construct_triangle(layer, m+10, m, m+1);
	construct_triangle(layer, m+10, m+1, m+2);
	construct_triangle(layer, m+10, m+2, m+3);
	construct_triangle(layer, m+2, m+3, m+5);
	construct_triangle(layer, m+2, m+5, m+6);

// far side
	construct_triangle(layer, m+11, m+5, m+6);
	construct_triangle(layer, m+11, m+6, m+7);
	construct_triangle(layer, m+11, m+7, m+8);
	construct_triangle(layer, m+11, m+8, m+9);
	construct_triangle(layer, m+11, m+9, m+5);

//	add_line(2, x1, by1, x2, y2, colours.base_trans [col] [8] [2]);

}


/*
Vertices:
base of beam
 0 back
 1 right
 2 right-front
 3 left-front
 4 left
end of beam
 5 base right
 6 base left
 7 left
 8 far end
 9 right
*/
static void draw_beam_bloom_triangles(int layer, float x1, float by1, float x2, float y2, ALLEGRO_COLOR centre_col, ALLEGRO_COLOR edge_col)
{
	int i, m = vbuf.vertex_pos_triangle;

	for (i = 0; i < 10; ++i)
		add_tri_vertex(vertex_list[i][0], vertex_list[i][1], edge_col);

	add_tri_vertex(x1, by1, centre_col);
	add_tri_vertex(x2, y2, centre_col);

	construct_triangle(layer, m+10, m+2, m+1);
	construct_triangle(layer, m+10, m+1, m);
	construct_triangle(layer, m+10, m, m+4);
	construct_triangle(layer, m+10, m+4, m+3);
	construct_triangle(layer, m+10, m+3, m+11);
	construct_triangle(layer, m+11, m+3, m+5);
	construct_triangle(layer, m+11, m+5, m+9);
	construct_triangle(layer, m+11, m+9, m+8);
	construct_triangle(layer, m+11, m+8, m+7);
	construct_triangle(layer, m+11, m+7, m+6);
	construct_triangle(layer, m+11, m+6, m+10);
	construct_triangle(layer, m+6, m+2, m+10);

}

/*
// Is this actually used?
void draw_spike_line(float x1, float by1, float x2, float y2, int col, int counter)
{

 float line_angle = atan2(y2 - by1, x2 - x1);

 int shade;

	shade = counter;

	float proportion = 1;

	if (shade > CLOUD_SHADES - 15)
		shade = CLOUD_SHADES - 1;

 float beam_base_flash_size = 4 * proportion * view.zoom;
 float beam_width = 0.5 * proportion * view.zoom;
 float beam_end_flash_size = 4 * proportion * view.zoom;

// back
 vertex_list [0] [0] = x1 + cos(line_angle + PI) * beam_base_flash_size;
 vertex_list [0] [1] = by1 + sin(line_angle + PI) * beam_base_flash_size;
// right
 vertex_list [1] [0] = x1 + cos(line_angle + PI/2) * beam_base_flash_size;
 vertex_list [1] [1] = by1 + sin(line_angle + PI/2) * beam_base_flash_size;
// left
 vertex_list [4] [0] = x1 + cos(line_angle - PI/2) * beam_base_flash_size;
 vertex_list [4] [1] = by1 + sin(line_angle - PI/2) * beam_base_flash_size;
// front (not used directly
	float base_front_x = x1 + cos(line_angle) * beam_base_flash_size;
	float base_front_y = by1 + sin(line_angle) * beam_base_flash_size;

// right-front (beam base)
 vertex_list [2] [0] = base_front_x + cos(line_angle + PI/2) * beam_width;
 vertex_list [2] [1] = base_front_y + sin(line_angle + PI/2) * beam_width;
// left-front (beam base)
 vertex_list [3] [0] = base_front_x + cos(line_angle - PI/2) * beam_width;
 vertex_list [3] [1] = base_front_y + sin(line_angle - PI/2) * beam_width;

 float reverse_stream_angle = line_angle + PI;

	float end_front_x = x2 + cos(reverse_stream_angle) * beam_end_flash_size;
	float end_front_y = y2 + sin(reverse_stream_angle) * beam_end_flash_size;
// other end:
// beam end base right
 vertex_list [5] [0] = end_front_x + cos(reverse_stream_angle + PI/2) * beam_width;
 vertex_list [5] [1] = end_front_y + sin(reverse_stream_angle + PI/2) * beam_width;
// beam end base left
 vertex_list [6] [0] = end_front_x + cos(reverse_stream_angle - PI/2) * beam_width;
 vertex_list [6] [1] = end_front_y + sin(reverse_stream_angle - PI/2) * beam_width;
// left
 vertex_list [7] [0] = x2 + cos(reverse_stream_angle - PI/2) * beam_end_flash_size;
 vertex_list [7] [1] = y2 + sin(reverse_stream_angle - PI/2) * beam_end_flash_size;
// far end
 vertex_list [8] [0] = x2 + cos(reverse_stream_angle + PI) * beam_end_flash_size;
 vertex_list [8] [1] = y2 + sin(reverse_stream_angle + PI) * beam_end_flash_size;
// right
 vertex_list [9] [0] = x2 + cos(reverse_stream_angle + PI/2) * beam_end_flash_size;
 vertex_list [9] [1] = y2 + sin(reverse_stream_angle + PI/2) * beam_end_flash_size;

 stream_beam_triangles(1, x1, by1, x2, y2, colours.packet [col] [shade]);

}
*/


/*

New approach to drawing:

*/



void draw_proc_shape(float x, float y, al_fixed angle, int shape, int player_index, float zoom, ALLEGRO_COLOR* proc_col)
{

 float f_angle;
 struct dshape_struct* dsh = &dshape [shape];


 int i, poly, layer;

 f_angle = fixed_to_radians(angle);

 ALLEGRO_COLOR fill_col;
// ALLEGRO_COLOR edge_col;

 for (poly = 0; poly < dsh->polys; poly++)
	{

//		if(poly==POLY_1)continue;
		layer = dsh->poly_layer [poly];
		fill_col = proc_col [dsh->poly_colour_level [poly]];
/*
		if (dsh->poly_colour_level == PROC_COL_MAIN_2 // currently this means core
			&& stress > 0)
		{
			if (stress < 10)
			{
				if (counter & 128
			}

		}*/
		//edge_col = proc_col [dsh->poly_colour_level [poly]] [1];

		int m = vbuf.vertex_pos_triangle; //, n = vbuf.index_pos_triangle[layer];
		float _x, _y;
		for (i = 0; i < dsh->display_vertices [poly]; ++i)
		{
			_x = x + fxpart(f_angle + dsh->display_vertex_angle [poly] [i], dsh->display_vertex_dist [poly] [i]) * zoom;
			_y = y + fypart(f_angle + dsh->display_vertex_angle [poly] [i], dsh->display_vertex_dist [poly] [i]) * zoom;
			add_tri_vertex(_x, _y, fill_col);
		}

/*
		for (i = 0; i < dsh->display_vertices [poly]; i ++)
		{

			vbuf.buffer_line[vbuf.vertex_pos_line + i].x = vertex_list_x [i];
			vbuf.buffer_line[vbuf.vertex_pos_line + i].y = vertex_list_y [i];
			vbuf.buffer_line[vbuf.vertex_pos_line + i].color = edge_col;

			vbuf.index_line [layer] [vbuf.index_pos_line [layer]++] = vbuf.vertex_pos_line + i;
			vbuf.index_line [layer] [vbuf.index_pos_line [layer]++] = vbuf.vertex_pos_line + i + 1;

		}
		vbuf.index_line [layer] [vbuf.index_pos_line [layer] - 1] = vbuf.vertex_pos_line;
		vbuf.vertex_pos_line += dsh->display_vertices [poly];
*/

		switch(dsh->triangulation[poly])
		{
			case TRI_FAN:
				for (i = 1; i < dsh->display_vertices[poly] - 1; ++i)
					construct_triangle(layer, m, m+i, m+i+1);
				break;
			case TRI_WALK:
			{
				int quads = dsh->display_vertices[poly]/2 - 1;
				
				for (i = 0; i < quads; ++i)
				{
					construct_triangle(layer, m+i, m+i+1, m+dsh->display_vertices[poly]-i-1);
					construct_triangle(layer, m+i+1, m+dsh->display_vertices[poly]-i-2, m+dsh->display_vertices[poly]-i-1);
				}
				if (dsh->display_vertices[poly] % 2 != 0)
					construct_triangle(layer, m + quads, m + quads + 1, m + quads + 2);
			}
				break;
			default:
				break;
		}

	}


  check_vbuf();



}


void draw_proc_outline(float x, float y, al_fixed angle, int shape, float scale, int lines_only, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col, float zoom)
{

 float f_angle;
 struct dshape_struct* dsh = &dshape [shape];

 int i, j;

 f_angle = fixed_to_radians(angle);
 const float f_x = cos(f_angle) * zoom * scale, f_y = sin(f_angle) * zoom * scale;

	int layer = 4;
//	float vertex_jagginess;

	int m = vbuf.vertex_pos_triangle, n = vbuf.vertex_pos_line;
	float _x, _y;
	for (i = 0; i < dsh->outline_vertices; ++i)
	{
		_x = x + f_x * dsh->outline_vertex_pos[i][0] - f_y * dsh->outline_vertex_pos[i][1];
		_y = y + f_y * dsh->outline_vertex_pos[i][0] + f_x * dsh->outline_vertex_pos[i][1];
		add_tri_vertex(_x, _y, fill_col);
		add_line_vertex(_x, _y, edge_col);
	}

	for (i = 0; i < dsh->outline_vertices - 1; ++i)
		construct_line(layer, n+i, n+i+1);

	construct_line(layer, n + dsh->outline_vertices - 1, n);

		if (lines_only)
			return;


	int vertex_list_index = dsh->outline_base_vertex + 1;

	if (vertex_list_index >= dsh->outline_vertices)
		vertex_list_index = 0;

	n = vbuf.index_pos_triangle[layer];

	for (i = 1; i < dsh->outline_vertices - 1; ++i)
	{
		for (j = 0; j < 3; ++j)
		{
			vbuf.index_triangle [layer] [n++] = m + dsh->outline_base_vertex;
			vbuf.index_triangle [layer] [n++] = m + vertex_list_index;

			if (vertex_list_index >= dsh->outline_vertices - 1)
				vbuf.index_triangle [layer] [n++] = m + 0;
			else
				vbuf.index_triangle [layer] [n++] = m + vertex_list_index + 1;
		}
		vertex_list_index ++;
		if (vertex_list_index >= dsh->outline_vertices)
			vertex_list_index = 0;
	}

	vbuf.index_pos_triangle[layer] = n;

  check_vbuf();



}


static void draw_jaggy_proc_outline(int layer, float x, float y, al_fixed angle, int shape, float scale, int jagginess, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col, float zoom)
{

 float f_angle;
 struct dshape_struct* dsh = &dshape [shape];

 int i, j;

 f_angle = fixed_to_radians(angle);
 const float f_x = cos(f_angle) * zoom * scale, f_y = sin(f_angle) * zoom * scale;

 float vertex_list_x [OUTLINE_VERTICES + 1]; // extra entry at the end is for 0
 float vertex_list_y [OUTLINE_VERTICES + 1];

	float vertex_jagginess;

 for (i = 0; i < dsh->outline_vertices; i ++)
	{
// drand_seed should have been called before this function!
  if (i & 1)
		 vertex_jagginess = (10 + drand(jagginess, angle)) * 0.1;
		  else
					vertex_jagginess = 1;
		vertex_list_x [i] = x + (f_x * dsh->outline_vertex_pos[i][0] - f_y * dsh->outline_vertex_pos[i][1]) * vertex_jagginess;
		vertex_list_y [i] = y + (f_y * dsh->outline_vertex_pos[i][0] + f_x * dsh->outline_vertex_pos[i][1]) * vertex_jagginess;
	}

	vertex_list_x [dsh->outline_vertices] = vertex_list_x [0];
	vertex_list_y [dsh->outline_vertices] = vertex_list_y [0];

/*
  for (i = 0; i < dsh->outline_vertices; i ++)
		{

	  vbuf.buffer_line[vbuf.vertex_pos_line].x = vertex_list_x [i];
	  vbuf.buffer_line[vbuf.vertex_pos_line].y = vertex_list_y [i];
   vbuf.buffer_line[vbuf.vertex_pos_line].color = edge_col;
   vbuf.index_line [layer] [vbuf.index_pos_line [layer]++] = vbuf.vertex_pos_line;
   vbuf.vertex_pos_line++;

	  vbuf.buffer_line[vbuf.vertex_pos_line].x = vertex_list_x [i + 1];
	  vbuf.buffer_line[vbuf.vertex_pos_line].y = vertex_list_y [i + 1];
   vbuf.buffer_line[vbuf.vertex_pos_line].color = edge_col;
   vbuf.index_line [layer] [vbuf.index_pos_line [layer]++] = vbuf.vertex_pos_line;
   vbuf.vertex_pos_line++;

		}
*/

  int vertex_list_index = dsh->outline_base_vertex + 1;

  if (vertex_list_index >= dsh->outline_vertices)
			vertex_list_index = 0;

	int m = vbuf.vertex_pos_triangle, n = vbuf.index_pos_triangle[layer];

	for (i = 0; i < dsh->outline_vertices; ++i)
	{
		vbuf.buffer_triangle[m+i].x = vertex_list_x[i];
		vbuf.buffer_triangle[m+i].y = vertex_list_y[i];
		vbuf.buffer_triangle[m+i].color = fill_col;
	}

	for (i = 1; i < dsh->outline_vertices - 1; ++i)
	{
		for (j = 0; j < 3; ++j)
		{
			vbuf.index_triangle [layer] [n++] = m + dsh->outline_base_vertex;
			vbuf.index_triangle [layer] [n++] = m + vertex_list_index;

			if (vertex_list_index >= dsh->outline_vertices - 1)
				vbuf.index_triangle [layer] [n++] = m + 0;
			else
				vbuf.index_triangle [layer] [n++] = m + vertex_list_index + 1;
		}
		vertex_list_index ++;
		if (vertex_list_index >= dsh->outline_vertices)
			vertex_list_index = 0;
	}

	vbuf.vertex_pos_triangle += dsh->outline_vertices;
	vbuf.index_pos_triangle[layer] = n;


  check_vbuf();



}


void draw_object(float proc_x, float proc_y,
																					al_fixed proc_angle,
																					int proc_shape,
																					struct object_struct* obj,
																					struct object_instance_struct* obj_inst, // NULL if drawn on design menu
																					struct core_struct* core, // NULL if drawn on design menu
																					struct proc_struct* proc, // NULL if drawn on design menu
																					int proc_link_index,
																					ALLEGRO_COLOR* proc_col,
//																					ALLEGRO_COLOR fill_col_under,
//																					ALLEGRO_COLOR edge_col,
																					float zoom)
{

#define view error_use_zoom_instead
// this define makes sure I don't accidentally use view.zoom (or similar view values) instead of the local zoom.
//  (can't use view values because this function is used for the designer, which shouldn't use view values)

 float object_angle_offset;

 if (obj_inst == NULL)
		object_angle_offset = fixed_to_radians(obj->base_angle_offset);
	  else
	  	object_angle_offset = fixed_to_radians(obj_inst->angle_offset);

// first draw the base:


	float angle_float = fixed_to_radians(proc_angle);

	int i;

 float f_angle, vx, vy;

 vx = proc_x + (cos(angle_float + dshape[proc_shape].link_object_angle [proc_link_index]) * dshape[proc_shape].link_object_dist [proc_link_index]) * zoom;
 vy = proc_y + (sin(angle_float + dshape[proc_shape].link_object_angle [proc_link_index]) * dshape[proc_shape].link_object_dist [proc_link_index]) * zoom;

 float left_vertex_x, left_vertex_y, right_vertex_x, right_vertex_y;
 float left_front_vertex_x, left_front_vertex_y, right_front_vertex_x, right_front_vertex_y;
 float left_back_vertex_x, left_back_vertex_y, right_back_vertex_x, right_back_vertex_y;
 float front_vertex_x, front_vertex_y, back_vertex_x, back_vertex_y;

 float dist_front;
 float dist_side;
 float dist_back;
 int shade;
 float extension_length;

#define OBJECT_UNDERLAY_LAYER 2
#define OBJECT_MAIN_LAYER 3
#define OBJECT_OVER_LAYER 4

 switch(obj->type)
 {

	 case OBJECT_TYPE_REPAIR:
	 case OBJECT_TYPE_REPAIR_OTHER:
  	{

    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);

//     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;
     float outwards = 0.5;

	 		float inner_point_x = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;
    float inner_point_y = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;

	  if (obj_inst != NULL) // will be NULL if this is being drawn on the design screen
			{

			if	(core->last_repair_restore_time > w.world_time - 32)
			{
// This could be repair or restore
    int line_time = w.world_time - core->last_repair_restore_time;
    outwards += (float) (32 - line_time) * 0.08;
			}


			if	(core->restore_cooldown_time > w.world_time)
			{
				int time_until_cooldown_ends = core->restore_cooldown_time - w.world_time;

    outwards += time_until_cooldown_ends * 0.03;
    if (outwards > 3)
					outwards = 3;

				float bcool_size = (time_until_cooldown_ends * 0.02);
				if (bcool_size > 16)
					bcool_size = 16;

				shade = time_until_cooldown_ends / 8;
				if (shade > 12)
					shade = 12;

				bcool_size += 10;

				float bcool_centre_x = inner_point_x + cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 8 * zoom;
				float bcool_centre_y = inner_point_y + sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 8 * zoom;

    float layer_rot_speed;

				{

     layer_rot_speed = 0.05;//(5 + drand(10, 1)) * 0.004;

     if (!(core->index & 1))
					 layer_rot_speed = 0 - layer_rot_speed; // could do better than this

					float layer_dist_2 = (bcool_size * zoom) / 2;
					float layer_separation = 1 + (time_until_cooldown_ends * 0.0002);

				 for (i = 0; i < 12; i ++)
				 {
					 float bit_angle = angle_float + ((PI*2*i)/12) + time_until_cooldown_ends * layer_rot_speed;
      float layer_dist = layer_dist_2;
      if (i & 1)
     	 layer_dist *= layer_separation;

					 add_diamond_layer(4,
																							 bcool_centre_x + cos(bit_angle) * layer_dist,
																							 bcool_centre_y + sin(bit_angle) * layer_dist,
																							 bcool_centre_x + cos(bit_angle) * (layer_dist + (9 * zoom)),
																							 bcool_centre_y + sin(bit_angle) * (layer_dist + (9 * zoom)),
																							 bcool_centre_x + cos(bit_angle + PI/11) * (layer_dist + (9 * zoom)),
																							 bcool_centre_y + sin(bit_angle + PI/11) * (layer_dist + (9 * zoom)),
																							 bcool_centre_x + cos(bit_angle + PI/11) * layer_dist,
																							 bcool_centre_y + sin(bit_angle + PI/11) * layer_dist,
																							 colours.packet [core->player_index] [shade]);

				 }
				}

			}

			}

    float outwards_xpart = cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]);
    float outwards_ypart = sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]);

// Left
   float left_side_xpart = cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]);
   float left_side_ypart = sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]);

#define REPAIR_OUTWARDS 16

// inner left
		  vertex_list [0] [0] = inner_point_x + left_side_xpart * (5+outwards) * zoom;
		  vertex_list [0] [1] = inner_point_y + left_side_ypart * (5+outwards) * zoom;
// outer left
		  vertex_list [1] [0] = vertex_list [0] [0] + left_side_xpart * (6) * zoom;
		  vertex_list [1] [1] = vertex_list [0] [1] + left_side_ypart * (6) * zoom;
// front
		  vertex_list [2] [0] = inner_point_x + (outwards_xpart * REPAIR_OUTWARDS * zoom) + (left_side_xpart * outwards * zoom);
		  vertex_list [2] [1] = inner_point_y + (outwards_ypart * REPAIR_OUTWARDS * zoom) + (left_side_ypart * outwards * zoom);
// outer front
		  vertex_list [3] [0] = vertex_list [2] [0] - (outwards_xpart * 6 * zoom);
		  vertex_list [3] [1] = vertex_list [2] [1] - (outwards_ypart * 6 * zoom);
//		  vertex_list [3] [0] = vertex_list [2] [0] - (outwards_xpart * 9 * zoom);
//		  vertex_list [3] [1] = vertex_list [2] [1] - (outwards_ypart * 9 * zoom);


    add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

// Right
   float right_side_xpart = cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]);
   float right_side_ypart = sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]);

// inner right
		  vertex_list [0] [0] = inner_point_x + right_side_xpart * (5+outwards) * zoom;
		  vertex_list [0] [1] = inner_point_y + right_side_ypart * (5+outwards) * zoom;
// outer right
		  vertex_list [1] [0] = vertex_list [0] [0] + right_side_xpart * (6) * zoom;
		  vertex_list [1] [1] = vertex_list [0] [1] + right_side_ypart * (6) * zoom;
// outer front
		  vertex_list [2] [0] = inner_point_x + (outwards_xpart * REPAIR_OUTWARDS * zoom) + (right_side_xpart * outwards * zoom);
		  vertex_list [2] [1] = inner_point_y + (outwards_ypart * REPAIR_OUTWARDS * zoom) + (right_side_ypart * outwards * zoom);
// outer front
		  vertex_list [3] [0] = vertex_list [2] [0] - (outwards_xpart * 6 * zoom);
		  vertex_list [3] [1] = vertex_list [2] [1] - (outwards_ypart * 6 * zoom);

// inner front
//		  vertex_list [3] [0] = vertex_list [2] [0] - (outwards_xpart * 9 * zoom);
//		  vertex_list [3] [1] = vertex_list [2] [1] - (outwards_ypart * 9 * zoom);

    add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

    if (obj->type == OBJECT_TYPE_REPAIR_OTHER)
				{
					vertex_list [0] [0] = inner_point_x;
					vertex_list [0] [1] = inner_point_y;

 		  vertex_list [1] [0] = inner_point_x + left_side_xpart * (4) * zoom;
	 	  vertex_list [1] [1] = inner_point_y + left_side_ypart * (4) * zoom;

		   vertex_list [2] [0] = inner_point_x + (outwards_xpart * (REPAIR_OUTWARDS-7) * zoom);
		   vertex_list [2] [1] = inner_point_y + (outwards_ypart * (REPAIR_OUTWARDS-7) * zoom);

 		  vertex_list [3] [0] = inner_point_x + right_side_xpart * (4) * zoom;
	 	  vertex_list [3] [1] = inner_point_y + right_side_ypart * (4) * zoom;

     add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

				}

				if (core != NULL
					&&	core->restore_cooldown_time > w.world_time)
				{

     int cooldown_time = core->restore_cooldown_time - w.world_time;

     int circle_time = (cooldown_time % 16);

      shade = circle_time;// * 2;

						float circle_size = (16 - circle_time) * 0.6;

      radial_circle(4,
																			 inner_point_x + cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 7 * zoom,
																	   inner_point_y + sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 7 * zoom,
																			 12, // vertices
																			 colours.packet [core->player_index] [shade],
																			 circle_size);
					}

  	}
			break; // end repair object

	 case OBJECT_TYPE_SPIKE:
  	{
     float fire_outwards = 2.0;

     int fire_time = 1000;

	  if (obj_inst != NULL) // will be NULL if this is being drawn on the design screen
			{

    fire_time = w.world_time - obj_inst->attack_last_fire_timestamp;
    if (fire_time < 32)
				{
     fire_outwards -= (float) (32 - fire_time) * 0.06;
				}



			}

	 		float inner_point_x = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * (dshape[proc_shape].link_point_dist [proc_link_index] [1])) * zoom;
    float inner_point_y = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * (dshape[proc_shape].link_point_dist [proc_link_index] [1])) * zoom;

    float outwards_xpart = cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]) * zoom;
    float outwards_ypart = sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]) * zoom;

    float left_xpart = cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 0.9 * zoom;
    float left_ypart = sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 0.9 * zoom;
    float right_xpart = cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 0.9 * zoom;
    float right_ypart = sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 0.9 * zoom;

    float left_xpart2 = cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index] - PI/4) * zoom;
    float left_ypart2 = sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index] - PI/4) * zoom;
    float right_xpart2 = cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index] + PI/4) * zoom;
    float right_ypart2 = sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index] + PI/4) * zoom;



// underlay:
// inner centre
		  vertex_list [0] [0] = inner_point_x + outwards_xpart * 4;
		  vertex_list [0] [1] = inner_point_y + outwards_ypart * 4;
// left
		  vertex_list [1] [0] = inner_point_x + left_xpart * 0.6;
		  vertex_list [1] [1] = inner_point_y + left_ypart * 0.6;
// outer left
		  vertex_list [2] [0] = inner_point_x + outwards_xpart * 9 + left_xpart2 * 3;
		  vertex_list [2] [1] = inner_point_y + outwards_ypart * 9 + left_ypart2 * 3;
// outer right
		  vertex_list [3] [0] = inner_point_x + outwards_xpart * 9 + right_xpart2 * 3;
		  vertex_list [3] [1] = inner_point_y + outwards_ypart * 9 + right_ypart2 * 3;
// right
		  vertex_list [4] [0] = inner_point_x + right_xpart * 0.6;
		  vertex_list [4] [1] = inner_point_y + right_ypart * 0.6;

    add_poly_layer(OBJECT_UNDERLAY_LAYER, 5, proc_col [PROC_COL_OBJECT_BASE]);


// inner centre
		  vertex_list [0] [0] = inner_point_x;// + outwards_xpart * 2;
		  vertex_list [0] [1] = inner_point_y;// + outwards_ypart * 2;
// inner left
		  vertex_list [1] [0] = inner_point_x + left_xpart;
		  vertex_list [1] [1] = inner_point_y + left_ypart;
// outer left
#define SPIKE_CONVEX 5
		  vertex_list [2] [0] = inner_point_x + left_xpart/2 + outwards_xpart * SPIKE_CONVEX;
		  vertex_list [2] [1] = inner_point_y + left_ypart/2 + outwards_ypart * SPIKE_CONVEX;
// outer left-centre
		  vertex_list [3] [0] = inner_point_x + left_xpart/4 + outwards_xpart * SPIKE_CONVEX;
		  vertex_list [3] [1] = inner_point_y + left_ypart/4 + outwards_ypart * SPIKE_CONVEX;
// outer left-centre_2
		  vertex_list [4] [0] = inner_point_x + left_xpart/5 + outwards_xpart * SPIKE_CONVEX;
		  vertex_list [4] [1] = inner_point_y + left_ypart/5 + outwards_ypart * SPIKE_CONVEX;
// outer right-centre_2
		  vertex_list [5] [0] = inner_point_x + right_xpart/5 + outwards_xpart * SPIKE_CONVEX;
		  vertex_list [5] [1] = inner_point_y + right_ypart/5 + outwards_ypart * SPIKE_CONVEX;
// outer right-centre
		  vertex_list [6] [0] = inner_point_x + right_xpart/4 + outwards_xpart * SPIKE_CONVEX;
		  vertex_list [6] [1] = inner_point_y + right_ypart/4 + outwards_ypart * SPIKE_CONVEX;
// outer right
		  vertex_list [7] [0] = inner_point_x + right_xpart/2 + outwards_xpart * SPIKE_CONVEX;
		  vertex_list [7] [1] = inner_point_y + right_ypart/2 + outwards_ypart * SPIKE_CONVEX;
// inner right
		  vertex_list [8] [0] = inner_point_x + right_xpart;
		  vertex_list [8] [1] = inner_point_y + right_ypart;

    add_poly_layer(OBJECT_MAIN_LAYER, 9, proc_col [PROC_COL_OBJECT_1]);

    float fire_outwards_xpart = fire_outwards * outwards_xpart;
    float fire_outwards_ypart = fire_outwards * outwards_ypart;

    float fire_part_left_x = vertex_list [2] [0] + fire_outwards_xpart;
    float fire_part_left_y = vertex_list [2] [1] + fire_outwards_ypart;
    float fire_part_left_centre_x = vertex_list [3] [0] + fire_outwards_xpart;
    float fire_part_left_centre_y = vertex_list [3] [1] + fire_outwards_ypart;
    float fire_part_left_centre2_x = vertex_list [4] [0] + fire_outwards_xpart;
    float fire_part_left_centre2_y = vertex_list [4] [1] + fire_outwards_ypart;
    float fire_part_right_centre2_x = vertex_list [5] [0] + fire_outwards_xpart;
    float fire_part_right_centre2_y = vertex_list [5] [1] + fire_outwards_ypart;
    float fire_part_right_centre_x = vertex_list [6] [0] + fire_outwards_xpart;
    float fire_part_right_centre_y = vertex_list [6] [1] + fire_outwards_ypart;
    float fire_part_right_x = vertex_list [7] [0] + fire_outwards_xpart;
    float fire_part_right_y = vertex_list [7] [1] + fire_outwards_ypart;

// far outer part:
    vertex_list [0] [0] = inner_point_x + outwards_xpart * 15 + right_xpart2 * 2 + fire_outwards_xpart;
    vertex_list [0] [1] = inner_point_y + outwards_ypart * 15 + right_ypart2 * 2 + fire_outwards_ypart;
    vertex_list [1] [0] = fire_part_right_x;
    vertex_list [1] [1] = fire_part_right_y;
    vertex_list [2] [0] = fire_part_right_centre_x;
    vertex_list [2] [1] = fire_part_right_centre_y;
    vertex_list [3] [0] = fire_part_right_centre2_x;
    vertex_list [3] [1] = fire_part_right_centre2_y;
    vertex_list [4] [0] = fire_part_left_centre2_x;
    vertex_list [4] [1] = fire_part_left_centre2_y;
    vertex_list [5] [0] = fire_part_left_centre_x;
    vertex_list [5] [1] = fire_part_left_centre_y;
    vertex_list [6] [0] = fire_part_left_x;
    vertex_list [6] [1] = fire_part_left_y;
    vertex_list [7] [0] = inner_point_x + outwards_xpart * 15 + left_xpart2 * 2 + fire_outwards_xpart;
    vertex_list [7] [1] = inner_point_y + outwards_ypart * 15 + left_ypart2 * 2 + fire_outwards_ypart;

/*
    vertex_list [0] [0] = fire_part_left_centre2_x + outwards_xpart * 7;
    vertex_list [0] [1] = fire_part_left_centre2_y + outwards_ypart * 7;
    vertex_list [1] [0] = fire_part_right_centre2_x + outwards_xpart * 7;
    vertex_list [1] [1] = fire_part_right_centre2_y + outwards_ypart * 7;
    vertex_list [2] [0] = fire_part_right_x;
    vertex_list [2] [1] = fire_part_right_y;
    vertex_list [3] [0] = fire_part_right_centre_x;
    vertex_list [3] [1] = fire_part_right_centre_y;
    vertex_list [4] [0] = fire_part_right_centre2_x;
    vertex_list [4] [1] = fire_part_right_centre2_y;
    vertex_list [5] [0] = fire_part_left_centre2_x;
    vertex_list [5] [1] = fire_part_left_centre2_y;
    vertex_list [6] [0] = fire_part_left_centre_x;
    vertex_list [6] [1] = fire_part_left_centre_y;
    vertex_list [7] [0] = fire_part_left_x;
    vertex_list [7] [1] = fire_part_left_y;
*/
    add_poly_layer(OBJECT_MAIN_LAYER, 8, proc_col [PROC_COL_OBJECT_1]);

    if (fire_time < 32)
				{

      shade = (32 - fire_time) / 2;

						float circle_size = (32 - fire_time) * 0.3;

						float circle_x = vertex_list [0] [0];
						float circle_y = vertex_list [0] [1];

      radial_circle(4,
																			 circle_x,
																	   circle_y,
																			 8, // vertices
																			 colours.packet [core->player_index] [(32 - fire_time) / 2],
																			 circle_size);

      radial_circle(4,
																			 circle_x,
																	   circle_y,
																			 8, // vertices
																			 colours.packet [core->player_index] [(32 - fire_time)],
																			 circle_size * 0.6);

				}

  	}
			break; // end spike object


//	 case OBJECT_TYPE_INTERFACE_DEPTH:
	 case OBJECT_TYPE_INTERFACE:
	 	{

    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);

	 		float inner_point_x = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;
    float inner_point_y = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;

    float outwards_xpart = cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]);
    float outwards_ypart = sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]);

// Left
// (these are out of order)

// inner left
		  vertex_list [5] [0] = inner_point_x + cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * 1 * zoom;
		  vertex_list [5] [1] = inner_point_y + sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * 1 * zoom;
// inner outside left
		  vertex_list [4] [0] = vertex_list [5] [0] + outwards_xpart * 6 * zoom;
		  vertex_list [4] [1] = vertex_list [5] [1] + outwards_ypart * 6 * zoom;
		  float save_vertex_4_left_x = vertex_list [4] [0];
		  float save_vertex_4_left_y = vertex_list [4] [1];
// inside elbow
		  vertex_list [3] [0] = vertex_list [4] [0] + cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 0.4 * zoom;
		  vertex_list [3] [1] = vertex_list [4] [1] + sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 0.4 * zoom;
// far inside
		  vertex_list [2] [0] = vertex_list [3] [0] + outwards_xpart * 11 * zoom;
		  vertex_list [2] [1] = vertex_list [3] [1] + outwards_ypart * 11 * zoom;
// outside elbow
		  vertex_list [0] [0] = inner_point_x + cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * zoom;
		  vertex_list [0] [1] = inner_point_y + sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * zoom;
// far outside
		  vertex_list [1] [0] = vertex_list [0] [0] + outwards_xpart * 9 * zoom;
		  vertex_list [1] [1] = vertex_list [0] [1] + outwards_ypart * 9 * zoom;

    add_poly_layer(OBJECT_MAIN_LAYER, 6, proc_col [PROC_COL_OBJECT_1]);

// Right

// inner right
		  vertex_list [5] [0] = inner_point_x + cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * 1 * zoom;
		  vertex_list [5] [1] = inner_point_y + sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * 1 * zoom;
// inner outside right

		  vertex_list [4] [0] = vertex_list [5] [0] + outwards_xpart * 6 * zoom;
		  vertex_list [4] [1] = vertex_list [5] [1] + outwards_ypart * 6 * zoom;
// inside elbow
		  vertex_list [3] [0] = vertex_list [4] [0] + cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 0.4 * zoom;
		  vertex_list [3] [1] = vertex_list [4] [1] + sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 0.4 * zoom;
// far inside
		  vertex_list [2] [0] = vertex_list [3] [0] + outwards_xpart * 11 * zoom;
		  vertex_list [2] [1] = vertex_list [3] [1] + outwards_ypart * 11 * zoom;
// outside elbow
		  vertex_list [0] [0] = inner_point_x + cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * zoom;
		  vertex_list [0] [1] = inner_point_y + sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * zoom;
// far outside
		  vertex_list [1] [0] = vertex_list [0] [0] + outwards_xpart * 9 * zoom;
		  vertex_list [1] [1] = vertex_list [0] [1] + outwards_ypart * 9 * zoom;

    add_poly_layer(OBJECT_MAIN_LAYER, 6, proc_col [PROC_COL_OBJECT_1]);

    if (obj_inst != NULL
					&& core->interface_strength > 0) // should display charge level even if interface not actually in use
				{
     float interface_level = (float) core->interface_strength / (float) core->interface_strength_max; // is there a better way to do this?

     vertex_list [0] [0] = save_vertex_4_left_x + cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0] + 0.2) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 0.3 * zoom;
     vertex_list [0] [1] = save_vertex_4_left_y + sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0] + 0.2) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 0.3 * zoom;
     vertex_list [1] [0] = inner_point_x + outwards_xpart * 8 * zoom;
     vertex_list [1] [1] = inner_point_y + outwards_ypart * 8 * zoom;
     vertex_list [2] [0] = vertex_list [4] [0] + cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1] - 0.2) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 0.3 * zoom;
     vertex_list [2] [1] = vertex_list [4] [1] + sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1] - 0.2) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 0.3 * zoom;
     vertex_list [3] [0] = vertex_list [2] [0] + outwards_xpart * 11 * interface_level * zoom;
     vertex_list [3] [1] = vertex_list [2] [1] + outwards_ypart * 11 * interface_level * zoom;
     vertex_list [4] [0] = vertex_list [0] [0] + outwards_xpart * 11 * interface_level * zoom;
     vertex_list [4] [1] = vertex_list [0] [1] + outwards_ypart * 11 * interface_level * zoom;

     shade = 12;
     if (core->interface_strength == core->interface_strength_max)
						shade = 16;

     if (core->interface_charged_time > w.world_time - 16)
					{
      shade = 12 + 8 + (core->interface_charged_time + 16 - w.world_time) / 2;
					}

     add_poly_layer(OBJECT_MAIN_LAYER, 5, colours.packet [core->player_index] [shade]);

				}
/*
// inner front
//		  vertex_list [0] [0] = inner_point_x + cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 6 * zoom;
//		  vertex_list [0] [1] = inner_point_y + sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 6 * zoom;
 vertex_list [0] [0] = inner_point_x + (cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]) * (dshape[proc_shape].link_outer_dist [proc_link_index] / 2)) * zoom;
 vertex_list [0] [1] = inner_point_y + (sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]) * (dshape[proc_shape].link_outer_dist [proc_link_index] / 2)) * zoom;

// inner L edge
		  vertex_list [1] [0] = inner_point_x + cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * OTI_INNER_SIZE * zoom;
		  vertex_list [1] [1] = inner_point_y + sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * OTI_INNER_SIZE * zoom;
// outer L edge
		  vertex_list [2] [0] = inner_point_x + cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * zoom;
		  vertex_list [2] [1] = inner_point_y + sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * zoom;
// outer L forward
		  vertex_list [3] [0] = vertex_list [2] [0] + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 4) * zoom;
		  vertex_list [3] [1] = vertex_list [2] [1] + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 4) * zoom;
// outer centre
//		  vertex_list [4] [0] = vertex_list [0] [0] + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 12) * zoom;
//		  vertex_list [4] [1] = vertex_list [0] [1] + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 12) * zoom;

 vertex_list [4] [0] = inner_point_x + (cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]) * (dshape[proc_shape].link_outer_dist [proc_link_index] + 5)) * zoom;
 vertex_list [4] [1] = inner_point_y + (sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]) * (dshape[proc_shape].link_outer_dist [proc_link_index] + 5)) * zoom;

// outer R edge
//  - out of order
		  vertex_list [6] [0] = inner_point_x + cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * zoom;
		  vertex_list [6] [1] = inner_point_y + sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * zoom;
// outer R forward
//  - out of order
		  vertex_list [5] [0] = vertex_list [6] [0] + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 4) * zoom;
		  vertex_list [5] [1] = vertex_list [6] [1] + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 4) * zoom;
// outer R edge
		  vertex_list [7] [0] = inner_point_x + cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * zoom;
		  vertex_list [7] [1] = inner_point_y + sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * zoom;
// inner R edge
		  vertex_list [8] [0] = inner_point_x + cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * OTI_INNER_SIZE * zoom;
		  vertex_list [8] [1] = inner_point_y + sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * OTI_INNER_SIZE * zoom;

*/
//    add_poly_layer(OBJECT_MAIN_LAYER, 9, proc_col [PROC_COL_OBJECT_1]);

	 	}
		 break;

/*
			 case OBJECT_TYPE_PULSE:
			 case OBJECT_TYPE_PULSE_L:
			 case OBJECT_TYPE_PULSE_XL:
    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);

     {
     int object_extra_size = obj->type - OBJECT_TYPE_PULSE;

     dist_front = 12 + object_extra_size;
     dist_side = 8 + object_extra_size;
     dist_back = 6 + object_extra_size;


     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;

			  if (obj_inst != NULL // will be NULL if this is being drawn on the design screen
						&& obj_inst->attack_last_fire_timestamp > w.world_time - 16)
					{

// first draw the underlay:
  vertex_list [0] [0] = vx + cos(f_angle) * (dist_front - 2) * zoom;
  vertex_list [0] [1] = vy + sin(f_angle) * (dist_front - 2) * zoom;
  vertex_list [1] [0] = vx + cos(f_angle + PI/2) * (dist_side - 2) * zoom;
  vertex_list [1] [1] = vy + sin(f_angle + PI/2) * (dist_side - 2) * zoom;
  vertex_list [2] [0] = vx + cos(f_angle + PI) * (dist_back - 2) * zoom;
  vertex_list [2] [1] = vy + sin(f_angle + PI) * (dist_back - 2) * zoom;
  vertex_list [3] [0] = vx + cos(f_angle - PI/2) * (dist_side - 2) * zoom;
  vertex_list [3] [1] = vy + sin(f_angle - PI/2) * (dist_side - 2) * zoom;

  add_poly_layer(OBJECT_UNDERLAY_LAYER, 4, proc_col [PROC_COL_UNDERLAY]);



      float time_since_firing = w.world_time - obj_inst->attack_last_fire_timestamp;
      float separation;

							  separation = (16 - (time_since_firing)) / 2;

						separation *= zoom;

       	front_vertex_x = vx + cos(f_angle) * (dist_front / 2) * zoom;
       	front_vertex_y = vy + sin(f_angle) * (dist_front / 2) * zoom;

       	back_vertex_x = vx + cos(f_angle + PI) * dist_back * zoom;
       	back_vertex_y = vy + sin(f_angle + PI) * dist_back * zoom;

       	left_vertex_x = back_vertex_x + cos(f_angle - PI/4) * dist_side * zoom;
       	left_vertex_y = back_vertex_y + sin(f_angle - PI/4) * dist_side * zoom;

       	right_vertex_x = back_vertex_x + cos(f_angle + PI/4) * dist_side * zoom;
       	right_vertex_y = back_vertex_y + sin(f_angle + PI/4) * dist_side * zoom;

       	float displaced_x = cos(f_angle + PI) * separation;
       	float displaced_y = sin(f_angle + PI) * separation;


  vertex_list [0] [0] = front_vertex_x + displaced_x;
  vertex_list [0] [1] = front_vertex_y + displaced_y;
  vertex_list [1] [0] = left_vertex_x + displaced_x;
  vertex_list [1] [1] = left_vertex_y + displaced_y;
  vertex_list [2] [0] = back_vertex_x + displaced_x;
  vertex_list [2] [1] = back_vertex_y + displaced_y;
  vertex_list [3] [0] = right_vertex_x + displaced_x;
  vertex_list [3] [1] = right_vertex_y + displaced_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);


		separation /= 4;

  displaced_x = cos(f_angle + PI/2) * separation;
  displaced_y = sin(f_angle + PI/2) * separation;

  vertex_list [0] [0] = front_vertex_x + displaced_x;
  vertex_list [0] [1] = front_vertex_y + displaced_y;
  vertex_list [1] [0] = front_vertex_x + displaced_x + cos(f_angle) * (dist_front/2) * zoom;
  vertex_list [1] [1] = front_vertex_y + displaced_y + sin(f_angle) * (dist_front/2) * zoom;
  vertex_list [2] [0] = right_vertex_x + displaced_x;
  vertex_list [2] [1] = right_vertex_y + displaced_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 3, proc_col [PROC_COL_OBJECT_1]);

  displaced_x = cos(f_angle - PI/2) * separation;
  displaced_y = sin(f_angle - PI/2) * separation;

  vertex_list [0] [0] = front_vertex_x + displaced_x;
  vertex_list [0] [1] = front_vertex_y + displaced_y;
  vertex_list [1] [0] = front_vertex_x + displaced_x + cos(f_angle) * (dist_front/2) * zoom;
  vertex_list [1] [1] = front_vertex_y + displaced_y + sin(f_angle) * (dist_front/2) * zoom;
  vertex_list [2] [0] = left_vertex_x + displaced_x;
  vertex_list [2] [1] = left_vertex_y + displaced_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 3, proc_col [PROC_COL_OBJECT_1]);


  if (time_since_firing > 16)
			break;

  shade = (32 - (time_since_firing * 2));
  if (shade >= CLOUD_SHADES)
			shade = CLOUD_SHADES - 1;

		float flash_size = (float) (17 - time_since_firing) * zoom * 2.0;// * 0.8;

		front_vertex_x = vx + cos(f_angle) * 9 * zoom;
		front_vertex_y = vy + sin(f_angle) * 9 * zoom;

  bloom_circle(OBJECT_OVER_LAYER, front_vertex_x, front_vertex_y, colours.bloom_centre [proc->player_index] [shade], colours.bloom_edge [proc->player_index] [0], flash_size * 3);

  vertex_list [0] [0] = front_vertex_x + cos(f_angle) * flash_size;
  vertex_list [0] [1] = front_vertex_y + sin(f_angle) * flash_size;
  vertex_list [1] [0] = front_vertex_x + cos(f_angle + PI/2) * flash_size / 2;
  vertex_list [1] [1] = front_vertex_y + sin(f_angle + PI/2) * flash_size / 2;
  vertex_list [2] [0] = front_vertex_x + cos(f_angle + PI) * flash_size;
  vertex_list [2] [1] = front_vertex_y + sin(f_angle + PI) * flash_size;
  vertex_list [3] [0] = front_vertex_x + cos(f_angle - PI/2) * flash_size / 2;
  vertex_list [3] [1] = front_vertex_y + sin(f_angle - PI/2) * flash_size / 2;

//  add_poly_layer(OBJECT_OVER_LAYER, 4, colours.packet [proc->player_index] [shade]);//, colours.packet [proc->player_index] [shade / 2]);


     radial_circle(2,
																			front_vertex_x,
																			front_vertex_y,
																			16,
																			colours.packet [proc->player_index] [shade],
																			4 + shade / 2);

						break;
					} // end of part of packet case

//     vx += (cos(f_angle) * 3) * zoom;
//     vy += (sin(f_angle) * 3) * zoom;


       	front_vertex_x = vx + cos(f_angle) * dist_front * zoom;
       	front_vertex_y = vy + sin(f_angle) * dist_front * zoom;

       	back_vertex_x = vx + cos(f_angle + PI) * dist_back * zoom;
       	back_vertex_y = vy + sin(f_angle + PI) * dist_back * zoom;

       	left_vertex_x = back_vertex_x + cos(f_angle - PI/4) * dist_side * zoom;
       	left_vertex_y = back_vertex_y + sin(f_angle - PI/4) * dist_side * zoom;

       	right_vertex_x = back_vertex_x + cos(f_angle + PI/4) * dist_side * zoom;
       	right_vertex_y = back_vertex_y + sin(f_angle + PI/4) * dist_side * zoom;

  vertex_list [0] [0] = front_vertex_x;
  vertex_list [0] [1] = front_vertex_y;
  vertex_list [1] [0] = left_vertex_x;
  vertex_list [1] [1] = left_vertex_y;
  vertex_list [2] [0] = back_vertex_x;
  vertex_list [2] [1] = back_vertex_y;
  vertex_list [3] [0] = right_vertex_x;
  vertex_list [3] [1] = right_vertex_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

//  if (obj_inst != NULL)
//   al_draw_textf(font[FONT_SQUARE].fnt, fill_col, vx - 20, vy, ALLEGRO_ALIGN_RIGHT, "%i", obj_inst->move_power);
     }

  break;
*/

			 case OBJECT_TYPE_BURST:
			 case OBJECT_TYPE_BURST_L:
			 case OBJECT_TYPE_BURST_XL:
    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);
    {
     int burst_object_extra_size = obj->type - OBJECT_TYPE_BURST;

     dist_front = 13 + burst_object_extra_size;
     dist_side = 9 + burst_object_extra_size;
     dist_back = 7 + burst_object_extra_size;


     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;

			  if (obj_inst != NULL // will be NULL if this is being drawn on the design screen
						&& obj_inst->attack_last_fire_timestamp > w.world_time - 16)
					{

// first draw the underlay:
  vertex_list [0] [0] = vx + cos(f_angle) * (dist_front - 2) * zoom;
  vertex_list [0] [1] = vy + sin(f_angle) * (dist_front - 2) * zoom;
  vertex_list [1] [0] = vx + cos(f_angle + PI/2) * (dist_side - 2) * zoom;
  vertex_list [1] [1] = vy + sin(f_angle + PI/2) * (dist_side - 2) * zoom;
  vertex_list [2] [0] = vx + cos(f_angle + PI) * (dist_back - 2) * zoom;
  vertex_list [2] [1] = vy + sin(f_angle + PI) * (dist_back - 2) * zoom;
  vertex_list [3] [0] = vx + cos(f_angle - PI/2) * (dist_side - 2) * zoom;
  vertex_list [3] [1] = vy + sin(f_angle - PI/2) * (dist_side - 2) * zoom;

  add_poly_layer(OBJECT_UNDERLAY_LAYER, 4, proc_col [PROC_COL_UNDERLAY]);



      float time_since_firing = w.world_time - obj_inst->attack_last_fire_timestamp;
      float separation;

				  separation = (16 - (time_since_firing)) / 2;

						separation *= zoom;

       	front_vertex_x = vx + cos(f_angle) * (dist_front / 2) * zoom;
       	front_vertex_y = vy + sin(f_angle) * (dist_front / 2) * zoom;

       	back_vertex_x = vx + cos(f_angle + PI) * dist_back * zoom;
       	back_vertex_y = vy + sin(f_angle + PI) * dist_back * zoom;

       	left_vertex_x = back_vertex_x + cos(f_angle - PI/4) * dist_side * zoom;
       	left_vertex_y = back_vertex_y + sin(f_angle - PI/4) * dist_side * zoom;

       	right_vertex_x = back_vertex_x + cos(f_angle + PI/4) * dist_side * zoom;
       	right_vertex_y = back_vertex_y + sin(f_angle + PI/4) * dist_side * zoom;

       	float displaced_x = cos(f_angle + PI) * separation;
       	float displaced_y = sin(f_angle + PI) * separation;

  vertex_list [0] [0] = front_vertex_x + displaced_x;
  vertex_list [0] [1] = front_vertex_y + displaced_y;
  vertex_list [1] [0] = left_vertex_x + displaced_x;
  vertex_list [1] [1] = left_vertex_y + displaced_y;
  vertex_list [2] [0] = back_vertex_x + displaced_x;
  vertex_list [2] [1] = back_vertex_y + displaced_y;
  vertex_list [3] [0] = right_vertex_x + displaced_x;
  vertex_list [3] [1] = right_vertex_y + displaced_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

		separation /= 4;

  displaced_x = cos(f_angle + PI/2) * separation;
  displaced_y = sin(f_angle + PI/2) * separation;

  vertex_list [0] [0] = front_vertex_x + displaced_x;
  vertex_list [0] [1] = front_vertex_y + displaced_y;
  vertex_list [1] [0] = front_vertex_x + displaced_x + cos(f_angle) * (dist_front/2) * zoom;
  vertex_list [1] [1] = front_vertex_y + displaced_y + sin(f_angle) * (dist_front/2) * zoom;
  vertex_list [2] [0] = vx + displaced_x + cos(f_angle + 0.3) * (dist_front - 2) * zoom;
  vertex_list [2] [1] = vy + displaced_y + sin(f_angle + 0.3) * (dist_front - 2) * zoom;
  vertex_list [3] [0] = right_vertex_x + displaced_x;
  vertex_list [3] [1] = right_vertex_y + displaced_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

  displaced_x = cos(f_angle - PI/2) * separation;
  displaced_y = sin(f_angle - PI/2) * separation;

  vertex_list [0] [0] = front_vertex_x + displaced_x;
  vertex_list [0] [1] = front_vertex_y + displaced_y;
  vertex_list [1] [0] = front_vertex_x + displaced_x + cos(f_angle) * (dist_front/2) * zoom;
  vertex_list [1] [1] = front_vertex_y + displaced_y + sin(f_angle) * (dist_front/2) * zoom;
  vertex_list [2] [0] = vx + displaced_x + cos(f_angle - 0.3) * (dist_front - 2) * zoom;
  vertex_list [2] [1] = vy + displaced_y + sin(f_angle - 0.3) * (dist_front - 2) * zoom;
  vertex_list [3] [0] = left_vertex_x + displaced_x;
  vertex_list [3] [1] = left_vertex_y + displaced_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

  shade = (32 - (time_since_firing * 2));
  if (shade >= CLOUD_SHADES)
			shade = CLOUD_SHADES - 1;

	 double_circle_with_bloom(4, front_vertex_x, front_vertex_y, 12 + (shade * 0.3) * (1 + burst_object_extra_size), proc->player_index, shade);


/*

  if (time_since_firing > 16)
			break;

  shade = (32 - (time_since_firing * 2));
  if (shade >= CLOUD_SHADES)
			shade = CLOUD_SHADES - 1;

		float flash_size = (float) (17 - time_since_firing) * zoom;// * 0.8;

		front_vertex_x = vx + cos(f_angle) * 9 * zoom;
		front_vertex_y = vy + sin(f_angle) * 9 * zoom;

  bloom_circle(OBJECT_OVER_LAYER, front_vertex_x, front_vertex_y, colours.bloom_centre [proc->player_index] [shade], colours.bloom_edge [proc->player_index] [0], flash_size * 3);


     radial_circle(2,
																			front_vertex_x,
																			front_vertex_y,
																			16,
																			colours.packet [proc->player_index] [shade],
																			4 + shade / 2);

*/
						break;
					} // end of part of packet case


       	front_vertex_x = vx + cos(f_angle) * dist_front * zoom;
       	front_vertex_y = vy + sin(f_angle) * dist_front * zoom;

       	left_front_vertex_x = vx + cos(f_angle - 0.3) * (dist_front - 2) * zoom;
       	left_front_vertex_y = vy + sin(f_angle - 0.3) * (dist_front - 2) * zoom;

       	right_front_vertex_x = vx + cos(f_angle + 0.3) * (dist_front - 2) * zoom;
       	right_front_vertex_y = vy + sin(f_angle + 0.3) * (dist_front - 2) * zoom;


       	back_vertex_x = vx + cos(f_angle + PI) * dist_back * zoom;
       	back_vertex_y = vy + sin(f_angle + PI) * dist_back * zoom;

       	left_vertex_x = back_vertex_x + cos(f_angle - PI/4) * dist_side * zoom;
       	left_vertex_y = back_vertex_y + sin(f_angle - PI/4) * dist_side * zoom;

       	right_vertex_x = back_vertex_x + cos(f_angle + PI/4) * dist_side * zoom;
       	right_vertex_y = back_vertex_y + sin(f_angle + PI/4) * dist_side * zoom;

  vertex_list [0] [0] = front_vertex_x;
  vertex_list [0] [1] = front_vertex_y;
  vertex_list [1] [0] = left_front_vertex_x;
  vertex_list [1] [1] = left_front_vertex_y;
  vertex_list [2] [0] = left_vertex_x;
  vertex_list [2] [1] = left_vertex_y;
  vertex_list [3] [0] = back_vertex_x;
  vertex_list [3] [1] = back_vertex_y;
  vertex_list [4] [0] = right_vertex_x;
  vertex_list [4] [1] = right_vertex_y;
  vertex_list [5] [0] = right_front_vertex_x;
  vertex_list [5] [1] = right_front_vertex_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 6, proc_col [PROC_COL_OBJECT_1]);
    }
  break;


	 case OBJECT_TYPE_BUILD:
  	{

    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);

    float outwards = 0.5;

	 		float inner_point_x = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * (dshape[proc_shape].link_point_dist [proc_link_index] [1])) * zoom;
    float inner_point_y = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * (dshape[proc_shape].link_point_dist [proc_link_index] [1])) * zoom;

	  if (obj_inst != NULL // will be NULL if this is being drawn on the design screen
//				&& core->last_build_time > w.world_time - 32)
				&& core->build_cooldown_time > w.world_time)
			{
				int time_since_build = w.world_time - core->last_build_time;
				int time_until_cooldown_ends = core->build_cooldown_time - w.world_time;
/*

     shade = time_until_cooldown_ends;
     if (shade > 27)
						shade = 27;

					float blob_size = time_until_cooldown_ends;
					if (blob_size > 80)
						blob_size = 80;

     radial_blob(4,
																	inner_point_x + cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 7 * zoom,
																	inner_point_y + sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 7 * zoom, 0, 12, colours.packet [core->player_index] [shade / 2], blob_size * 0.12, 4, w.world_time * 2, w.world_time * 3);

     radial_blob(4,
																	inner_point_x + cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 7 * zoom,
																	inner_point_y + sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 7 * zoom, 0, 12, colours.packet [core->player_index] [shade], blob_size * 0.06, 3, w.world_time * 2, w.world_time * 3);
*/

    outwards += time_until_cooldown_ends * 0.03;
    if (outwards > 3)
					outwards = 3;

				float bcool_size = (time_until_cooldown_ends * 0.02);
				if (bcool_size > 16)
					bcool_size = 16;

				shade = time_until_cooldown_ends / 2;
				if (shade > 24)
					shade = 24;

				if (time_since_build < 16)
				{
					bcool_size *= time_since_build * 0.0625;
					outwards *= time_since_build * 0.0625;
				}

				bcool_size += 24;

//				bcool_size *= zoom;

				float bcool_centre_x = inner_point_x + cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 8 * zoom;
				float bcool_centre_y = inner_point_y + sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 8 * zoom;

//				float blob_size = (bcool_size * 0.08) + zoom;

//    int layer_time;
    float layer_rot_speed;

//    int j = 0;

//				for (j = 0; j < 2; j ++)
				{

//     layer_time = time_since_build % 128;
//     layer_seed = ((time_since_build + (j * 64)) / 128) + (j * 100);
//     seed_drand(layer_seed);
//     layer_blocks = 5 + drand(3, 1);
     layer_rot_speed = 0.03;//(5 + drand(10, 1)) * 0.004;
//     if (drand(2, 1))
     if (core->index & 1)
					 layer_rot_speed = 0 - layer_rot_speed; // could do better than this

//				 shade = 25;

//				 if (layer_time < 16)
// 					shade = layer_time * 2;
//				 if (layer_time > 112)
//					 shade = (128 - layer_time) * 2;

//					float layer_dist_2 = (bcool_size + (j * 30 * zoom)) / 2;
//					float layer_dist_2 = (bcool_size + (j * 30)) / 2;
					float layer_dist_2 = (bcool_size * zoom) / 2;
					float layer_separation = 1 + (time_until_cooldown_ends * 0.0002);

				 for (i = 0; i < 16; i ++)
				 {
					 float bit_angle = angle_float + ((PI*2*i)/16) + time_since_build * layer_rot_speed;
//					 float base_x = bcool_centre_x + cos(bit_angle) * layer_dist;
//					 float base_y = bcool_centre_y + sin(bit_angle) * layer_dist;
      float layer_dist = layer_dist_2;
      if (i & 1)
     	 layer_dist *= layer_separation;

					 add_diamond_layer(4,
																							 bcool_centre_x + cos(bit_angle) * layer_dist,
																							 bcool_centre_y + sin(bit_angle) * layer_dist,
																							 bcool_centre_x + cos(bit_angle) * (layer_dist + (6 * zoom)),
																							 bcool_centre_y + sin(bit_angle) * (layer_dist + (6 * zoom)),
																							 bcool_centre_x + cos(bit_angle + PI/11) * (layer_dist + (6 * zoom)),
																							 bcool_centre_y + sin(bit_angle + PI/11) * (layer_dist + (6 * zoom)),
																							 bcool_centre_x + cos(bit_angle + PI/11) * layer_dist,
																							 bcool_centre_y + sin(bit_angle + PI/11) * layer_dist,
																							 colours.packet [core->player_index] [shade]);

/*																							 base_x + cos(bit_angle + PI/4) * 4 * zoom,
																							 base_y + sin(bit_angle + PI/4) * 4 * zoom,
																							 base_x + cos(bit_angle + PI-PI/4) * 4 * zoom,
																							 base_y + sin(bit_angle + PI-PI/4) * 4 * zoom,
																							 base_x + cos(bit_angle + PI+PI/4) * 4 * zoom,
																							 base_y + sin(bit_angle + PI+PI/4) * 4 * zoom,
																							 base_x + cos(bit_angle - PI/4) * 4 * zoom,
																							 base_y + sin(bit_angle - PI/4) * 4 * zoom,
																							 colours.packet [core->player_index] [shade]);*/
				 }
				}



			}


//#define BCOOL_APPROACH_1
#ifdef BCOOL_APPROACH_1
	  if (obj_inst != NULL // will be NULL if this is being drawn on the design screen
//				&& core->last_build_time > w.world_time - 32)
				&& core->build_cooldown_time > w.world_time)
			{
				int time_since_build = w.world_time - core->last_build_time;
				int time_until_cooldown_ends = core->build_cooldown_time - w.world_time;

    int line_time = w.world_time - core->last_build_time;
    outwards += time_until_cooldown_ends * 0.03;
    if (outwards > 3)
					outwards = 3;

				float bcool_size = (time_until_cooldown_ends * 0.08);
				if (bcool_size > 24)
					bcool_size = 24;

				shade = bcool_size;

				if (time_since_build < 16)
				{
					bcool_size *= time_since_build * 0.0625;
					outwards *= time_since_build * 0.0625;
				}

				bcool_size += 8;

				bcool_size *= zoom;

				float bcool_centre_x = inner_point_x + cos(angle_float) * 8 * zoom;
				float bcool_centre_y = inner_point_y + sin(angle_float) * 8 * zoom;

				float blob_size = (bcool_size * 0.08) + zoom;

    int layer_time, layer_seed, layer_blocks;
    float layer_rot_speed;

    int j;

				for (j = 0; j < 2; j ++)
				{

     layer_time = (time_since_build + (j * 64)) % 128;
     layer_seed = ((time_since_build + (j * 64)) / 128) + (j * 100);
     seed_drand(layer_seed);
     layer_blocks = 5 + drand(3, 1);
     layer_rot_speed = (5 + drand(10, 1)) * 0.004;
//     if (drand(2, 1))
     if (layer_seed & 1)
					 layer_rot_speed = 0 - layer_rot_speed;

				 shade = 25;

				 if (layer_time < 16)
 					shade = layer_time * 2;
				 if (layer_time > 112)
					 shade = (128 - layer_time) * 2;

					float layer_dist = (bcool_size + (j * 30 * zoom)) / 2;

				 for (i = 0; i < layer_blocks; i ++)
				 {
					 float bit_angle = angle_float + ((PI*2*i)/layer_blocks) + layer_time * layer_rot_speed;
					 float base_x = bcool_centre_x + cos(bit_angle) * layer_dist;
					 float base_y = bcool_centre_y + sin(bit_angle) * layer_dist;
					 add_diamond_layer(4,
																							 base_x + cos(bit_angle + PI/4) * 4 * zoom,
																							 base_y + sin(bit_angle + PI/4) * 4 * zoom,
																							 base_x + cos(bit_angle + PI-PI/4) * 4 * zoom,
																							 base_y + sin(bit_angle + PI-PI/4) * 4 * zoom,
																							 base_x + cos(bit_angle + PI+PI/4) * 4 * zoom,
																							 base_y + sin(bit_angle + PI+PI/4) * 4 * zoom,
																							 base_x + cos(bit_angle - PI/4) * 4 * zoom,
																							 base_y + sin(bit_angle - PI/4) * 4 * zoom,
																							 colours.packet [core->player_index] [shade]);
				 }
				}



			}
#endif
/*
	  if (obj_inst != NULL // will be NULL if this is being drawn on the design screen
//				&& core->last_build_time > w.world_time - 32)
				&& core->build_cooldown_time > w.world_time)
			{
				int time_since_build = w.world_time - core->last_build_time;
				int time_until_cooldown_ends = core->build_cooldown_time - w.world_time;

    int line_time = w.world_time - core->last_build_time;
    outwards += time_until_cooldown_ends * 0.03;
    if (outwards > 3)
					outwards = 3;

				float bcool_size = time_until_cooldown_ends * 0.08;
				if (bcool_size > 24)
					bcool_size = 24;

				shade = bcool_size;

				if (time_since_build < 16)
				{
					bcool_size *= time_since_build * 0.0625;
					outwards *= time_since_build * 0.0625;
				}

				bcool_size *= zoom;

				float bcool_centre_x = inner_point_x + cos(angle_float) * 8 * zoom;
				float bcool_centre_y = inner_point_y + sin(angle_float) * 8 * zoom;

				float blob_size = (bcool_size * 0.08) + zoom;

    add_diamond_layer(4,
																						bcool_centre_x + cos(angle_float) * blob_size,
																						bcool_centre_y + sin(angle_float) * blob_size,
																						bcool_centre_x + cos(angle_float + PI/2) * blob_size,
																						bcool_centre_y + sin(angle_float + PI/2) * blob_size,
																						bcool_centre_x + cos(angle_float + PI) * blob_size,
																						bcool_centre_y + sin(angle_float + PI) * blob_size,
																						bcool_centre_x + cos(angle_float - PI/2) * blob_size,
																						bcool_centre_y + sin(angle_float - PI/2) * blob_size,
 																					colours.packet [core->player_index] [shade]);

				for (i = 0; i < 7; i ++)
				{
					float bit_angle = angle_float + ((PI*2*i)/7) + time_since_build * 0.04;
					float base_x = bcool_centre_x + cos(bit_angle) * bcool_size;
					float base_y = bcool_centre_y + sin(bit_angle) * bcool_size;
					add_diamond_layer(4,
																							base_x + cos(bit_angle + PI/4) * 3 * zoom,
																							base_y + sin(bit_angle + PI/4) * 3 * zoom,
																							base_x + cos(bit_angle + PI-PI/4) * 3 * zoom,
																							base_y + sin(bit_angle + PI-PI/4) * 3 * zoom,
																							base_x + cos(bit_angle + PI+PI/4) * 3 * zoom,
																							base_y + sin(bit_angle + PI+PI/4) * 3 * zoom,
																							base_x + cos(bit_angle - PI/4) * 3 * zoom,
																							base_y + sin(bit_angle - PI/4) * 3 * zoom,
																							colours.packet [core->player_index] [shade]);
				}


			}
*/

    float left_xpart = cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]);
    float left_ypart = sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]);

    float right_xpart = cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]);
    float right_ypart = sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]);

    float outwards_xpart = cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]);
    float outwards_ypart = sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]);

#define BUILD_INNER_THICK 5
#define BUILD_INNER_LENGTH 9

// Left
// middle inner
		  vertex_list [0] [0] = inner_point_x + left_xpart * outwards * zoom;
		  vertex_list [0] [1] = inner_point_y + left_ypart * outwards * zoom;
// middle outer
		  vertex_list [1] [0] = vertex_list [0] [0] + outwards_xpart * BUILD_INNER_THICK * zoom;
		  vertex_list [1] [1] = vertex_list [0] [1] + outwards_ypart * BUILD_INNER_THICK * zoom;
// left outer
		  vertex_list [2] [0] = vertex_list [1] [0] + left_xpart * BUILD_INNER_LENGTH * zoom;
		  vertex_list [2] [1] = vertex_list [1] [1] + left_ypart * BUILD_INNER_LENGTH * zoom;
// left inner
		  vertex_list [3] [0] = vertex_list [0] [0] + left_xpart * (BUILD_INNER_LENGTH + 4) * zoom;
		  vertex_list [3] [1] = vertex_list [0] [1] + left_ypart * (BUILD_INNER_LENGTH + 4) * zoom;

    add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

// Right
// middle inner
		  vertex_list [0] [0] = inner_point_x + right_xpart * outwards * zoom;
		  vertex_list [0] [1] = inner_point_y + right_ypart * outwards * zoom;
// middle outer
		  vertex_list [1] [0] = vertex_list [0] [0] + outwards_xpart * BUILD_INNER_THICK * zoom;
		  vertex_list [1] [1] = vertex_list [0] [1] + outwards_ypart * BUILD_INNER_THICK * zoom;
// left outer
		  vertex_list [2] [0] = vertex_list [1] [0] + right_xpart * BUILD_INNER_LENGTH * zoom;
		  vertex_list [2] [1] = vertex_list [1] [1] + right_ypart * BUILD_INNER_LENGTH * zoom;
// left inner
		  vertex_list [3] [0] = vertex_list [0] [0] + right_xpart * (BUILD_INNER_LENGTH + 4) * zoom;
		  vertex_list [3] [1] = vertex_list [0] [1] + right_ypart * (BUILD_INNER_LENGTH + 4)  * zoom;

    add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

// Middle
 	  vertex_list [0] [0] = inner_point_x + outwards_xpart * (BUILD_INNER_THICK + 1 + outwards) * zoom;
		  vertex_list [0] [1] = inner_point_y + outwards_ypart * (BUILD_INNER_THICK + 1 + outwards) * zoom;
// left
		  vertex_list [1] [0] = vertex_list [0] [0] + left_xpart * BUILD_INNER_LENGTH * zoom;
		  vertex_list [1] [1] = vertex_list [0] [1] + left_ypart * BUILD_INNER_LENGTH * zoom;
// far
		  vertex_list [2] [0] = vertex_list [0] [0] + outwards_xpart * 15 * zoom;
		  vertex_list [2] [1] = vertex_list [0] [1] + outwards_ypart * 15 * zoom;
// right
		  vertex_list [3] [0] = vertex_list [0] [0] + right_xpart * BUILD_INNER_LENGTH  * zoom;
		  vertex_list [3] [1] = vertex_list [0] [1] + right_ypart * BUILD_INNER_LENGTH  * zoom;

    add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

  	}
		 break; // end OBJECT_TYPE_BUILD


/*
		   f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset + PI/4;

		   float outwards = 0;
		   if (obj_inst != NULL
						&& w.world_time < core->build_cooldown)
					{
      int line_time = core->build_cooldown - w.world_time;
      outwards = (float) (line_time) * 0.2;
      if (outwards > (32 * 0.2))
							outwards = 32 * 0.2;
					}


		   float bit_angle;
		   float bit_x, bit_y;

     for (i = 0; i < 4; i ++)
			  {
  				bit_angle = f_angle + (PI/2) * i;
  				bit_x = vx + cos(bit_angle) * (7.0 + outwards) * zoom;
  				bit_y = vy + sin(bit_angle) * (7.0 + outwards) * zoom;
				  vertex_list [0] [0] = bit_x + cos(bit_angle) * 9.0 * zoom;
				  vertex_list [0] [1] = bit_y + sin(bit_angle) * 9.0 * zoom;
				  vertex_list [1] [0] = bit_x + cos(bit_angle + PI/2) * 5.5 * zoom;
				  vertex_list [1] [1] = bit_y + sin(bit_angle + PI/2) * 5.5 * zoom;
				  vertex_list [2] [0] = bit_x + cos(bit_angle + PI) * 7.0 * zoom;
				  vertex_list [2] [1] = bit_y + sin(bit_angle + PI) * 7.0 * zoom;
				  vertex_list [3] [0] = bit_x + cos(bit_angle - PI/2) * 5.5 * zoom;
				  vertex_list [3] [1] = bit_y + sin(bit_angle - PI/2) * 5.5 * zoom;
	     add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);
 		  }

				  vertex_list [0] [0] = vx + cos(f_angle) * 7.0 * zoom;
				  vertex_list [0] [1] = vy + sin(f_angle) * 7.0 * zoom;
				  vertex_list [1] [0] = vx + cos(f_angle + PI/2) * 7.0 * zoom;
				  vertex_list [1] [1] = vy + sin(f_angle + PI/2) * 7.0 * zoom;
				  vertex_list [2] [0] = vx + cos(f_angle + PI) * 7.0 * zoom;
				  vertex_list [2] [1] = vy + sin(f_angle + PI) * 7.0 * zoom;
				  vertex_list [3] [0] = vx + cos(f_angle - PI/2) * 7.0 * zoom;
				  vertex_list [3] [1] = vy + sin(f_angle - PI/2) * 7.0 * zoom;
      add_poly_layer(OBJECT_UNDERLAY_LAYER, 4, proc_col [PROC_COL_OBJECT_2]);



 	 }
		 break; // end OBJECT_TYPE_BUILD
*/


	 case OBJECT_TYPE_STABILITY:
	 	{

			vertex_list [0] [0] = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;
			vertex_list [0] [1] = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;

			vertex_list [1] [0] = vertex_list [0] [0] + (cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 1) * zoom;
			vertex_list [1] [1] = vertex_list [0] [1] + (sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 1) * zoom;

   f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;

//			float outwards_x = cos(f_angle) * zoom;
//			float outwards_y = sin(f_angle) * zoom;

			vertex_list [2] [0] = vertex_list [1] [0] + cos(f_angle - 0.5) * 9 * zoom;
			vertex_list [2] [1] = vertex_list [1] [1] + sin(f_angle - 0.5) * 9 * zoom;

			vertex_list [4] [0] = vertex_list [0] [0] + (cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 1) * zoom;
			vertex_list [4] [1] = vertex_list [0] [1] + (sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 1) * zoom;

			vertex_list [3] [0] = vertex_list [4] [0] + cos(f_angle + 0.5) * 9 * zoom;
			vertex_list [3] [1] = vertex_list [4] [1] + sin(f_angle + 0.5) * 9 * zoom;

	  add_poly_layer(OBJECT_MAIN_LAYER, 5, proc_col [PROC_COL_OBJECT_1]);

	 	}
		 break; // end OBJECT_TYPE_STABILITY


	 case OBJECT_TYPE_ALLOCATE:
	 	{
/*
    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);
*/

   f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;

   for (i = 0; i < 6; i ++)
			{
				float point_angle = f_angle + (PI/3) * i;
				vertex_list [i] [0] = vx + cos(point_angle) * 6.0 * zoom;
				vertex_list [i] [1] = vy + sin(point_angle) * 6.0 * zoom;
 		}
	  add_poly_layer(OBJECT_MAIN_LAYER, 6, proc_col [PROC_COL_OBJECT_1]);

			vertex_list [0] [0] = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;
			vertex_list [0] [1] = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;

//			vertex_list [1] [0] = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [2]) * dshape[proc_shape].link_point_dist [proc_link_index] [2]) * zoom;
//			vertex_list [1] [1] = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [2]) * dshape[proc_shape].link_point_dist [proc_link_index] [2]) * zoom;
			vertex_list [1] [0] = vertex_list [0] [0] + (cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 1) * zoom;
			vertex_list [1] [1] = vertex_list [0] [1] + (sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 1) * zoom;

   for (i = 2; i < 5; i ++)
			{
				float point_angle = f_angle + (PI/3) * i;
				vertex_list [i] [0] = vx + cos(point_angle) * 8.0 * zoom;
				vertex_list [i] [1] = vy + sin(point_angle) * 8.0 * zoom;
 		}


			vertex_list [5] [0] = vertex_list [0] [0] + (cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 1) * zoom;
			vertex_list [5] [1] = vertex_list [0] [1] + (sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 1) * zoom;
//			vertex_list [5] [0] = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_dist [proc_link_index] [0]) * zoom;
//			vertex_list [5] [1] = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_dist [proc_link_index] [0]) * zoom;

	  add_poly_layer(OBJECT_MAIN_LAYER, 6, proc_col [PROC_COL_OBJECT_1]);



	  if (obj_inst == NULL
				||	obj_inst->last_allocate < w.world_time - 32)
				break; // nothing to show
			float vane_size; // 0 to 15
			int vane_shade;
	  if (obj_inst->first_unbroken_allocate > w.world_time - 16) // just started allocating
			{
				vane_size = w.world_time - obj_inst->first_unbroken_allocate;
				vane_shade = 31 - vane_size;
			}
			 else
				{
					if (obj_inst->last_allocate < w.world_time - 16)
					{
						vane_size = 32 + obj_inst->last_allocate - w.world_time;
  				vane_shade = vane_size;
					}
					 else
						{
							vane_size = 16;
							vane_shade = 15;
						}
				}
				vane_size *= 0.7 * zoom;
				for (i = 0; i < 3; i ++)
				{
					float vane_angle = f_angle + (PI*0.6667) * i + ((w.world_time & 63) * PI/96);
					float vane_x = vx + cos(vane_angle) * 20 * zoom;
					float vane_y = vy + sin(vane_angle) * 20 * zoom;
					add_outline_diamond_layer(OBJECT_MAIN_LAYER, vane_x + cos(vane_angle - PI/6) * vane_size,
																																		vane_y + sin(vane_angle - PI/6) * vane_size,
																																		vane_x + cos(vane_angle + PI/6) * vane_size,
																																		vane_y + sin(vane_angle + PI/6) * vane_size,
																																		vane_x + cos(vane_angle + PI - PI/12) * vane_size,
																																		vane_y + sin(vane_angle + PI - PI/12) * vane_size,
																																		vane_x + cos(vane_angle + PI + PI/12) * vane_size,
  																																vane_y + sin(vane_angle + PI + PI/12) * vane_size,
																																		colours.drive [proc->player_index] [vane_shade],
																																		colours.drive [proc->player_index] [vane_shade/2]);
				}

	 	}
		 break; // end OBJECT_TYPE_ALLOCATE

 	case OBJECT_TYPE_MOVE:
    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);

     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;

     vx -= (cos(f_angle)) * zoom;
     vy -= (sin(f_angle)) * zoom;


     dist_front = 12 * zoom;// + pr->method [pr->vertex_method [vertex]].extension [0];
     dist_side = 5 * zoom;// + pr->method [pr->vertex_method [vertex]].extension [0] / 2;
     dist_back = 4 * zoom;// + pr->method [pr->vertex_method [vertex]].extension [0];
    	extension_length = 2 * zoom;// + pr->method [pr->vertex_method [vertex]].extension [0];


       	left_front_vertex_x = vx + cos(f_angle - PI/28) * dist_front;
       	left_front_vertex_y = vy + sin(f_angle - PI/28) * dist_front;
       	right_front_vertex_x = vx + cos(f_angle + PI/28) * dist_front;
       	right_front_vertex_y = vy + sin(f_angle + PI/28) * dist_front;

       	left_back_vertex_x = vx + cos(f_angle + PI + PI/6) * dist_back;
       	left_back_vertex_y = vy + sin(f_angle + PI + PI/6) * dist_back;
       	right_back_vertex_x = vx + cos(f_angle + PI - PI/6) * dist_back;
       	right_back_vertex_y = vy + sin(f_angle + PI - PI/6) * dist_back;

       	left_vertex_x = left_back_vertex_x + cos(f_angle - PI/4) * dist_side;
       	left_vertex_y = left_back_vertex_y + sin(f_angle - PI/4) * dist_side;

       	right_vertex_x = right_back_vertex_x + cos(f_angle + PI/4) * dist_side;
       	right_vertex_y = right_back_vertex_y + sin(f_angle + PI/4) * dist_side;

  vertex_list [0] [0] = right_front_vertex_x + cos(f_angle + PI/2) * extension_length;
  vertex_list [0] [1] = right_front_vertex_y + sin(f_angle + PI/2) * extension_length;
  vertex_list [1] [0] = left_front_vertex_x + cos(f_angle - PI/2) * extension_length;
  vertex_list [1] [1] = left_front_vertex_y + sin(f_angle - PI/2) * extension_length;
  vertex_list [2] [0] = left_vertex_x + cos(f_angle - PI/6) * extension_length;
  vertex_list [2] [1] = left_vertex_y + sin(f_angle - PI/6) * extension_length;
  vertex_list [3] [0] = left_back_vertex_x;
  vertex_list [3] [1] = left_back_vertex_y;
  vertex_list [4] [0] = right_back_vertex_x;
  vertex_list [4] [1] = right_back_vertex_y;
  vertex_list [5] [0] = right_vertex_x + cos(f_angle + PI/6) * extension_length;
  vertex_list [5] [1] = right_vertex_y + sin(f_angle + PI/6) * extension_length;

  add_poly_layer(OBJECT_MAIN_LAYER, 6, proc_col [PROC_COL_OBJECT_1]);

  if (obj_inst != NULL)
		{
     if (obj_inst->move_power > 0
					 || obj_inst->move_power_last_cycle > 0)
					{

						int move_power_display = obj_inst->move_power;
						int move_time = 16 - (core->next_execution_timestamp - w.world_time);

					 if (obj_inst->move_power < obj_inst->move_power_last_cycle)
					 {
					 	move_power_display = obj_inst->move_power_last_cycle - move_time;// * 10;
					 	if (move_power_display < obj_inst->move_power)
								move_power_display = obj_inst->move_power;
					 }
					  else
							{
					   if (obj_inst->move_power > obj_inst->move_power_last_cycle)
					   {
					 	  move_power_display = obj_inst->move_power_last_cycle + move_time;// * 10;
					 	  if (move_power_display > obj_inst->move_power)
								  move_power_display = obj_inst->move_power;
					   }
							}

						if (move_power_display <= 0)
 						break;

// inner part
   	  float pulse_size = 1 + ((float) move_power_display / 4);// + drand(30,1) * 0.1;
   	  float pulse_dist = 9 + pulse_size;// + drand(20,1) * 0.1;
   	  float pulse_x = vx + cos(f_angle) * pulse_dist * zoom;
   	  float pulse_y = vy + sin(f_angle) * pulse_dist * zoom;


//      float x_step = cos(f_angle) * 4.0 * zoom;
//      float y_step = sin(f_angle) * 4.0 * zoom;

      seed_drand((proc->index + proc_link_index) - move_time);
      int end_time = 3 + drand(3, 1) + move_power_display / 3;//25;//packet_time;

//      int max_time = 12;// + pack->status * 4;
//      if (end_time > max_time)
// 						end_time = max_time;

//					 float tail_width = (move_power_display * 0.02) + 0.09;//0.07;

						float tail_size = move_power_display;

						tail_size += (16 - move_time) * 0.06;


					 int tail_shade = tail_size * 2;
					 if (tail_shade > 24)
							tail_shade = 24;


      draw_new_move_tail(pulse_x, pulse_y,
																									f_angle + PI,
																									tail_size,
																									proc->player_index,
																									tail_shade,
																									move_time,
																									1);//			int drand_seed)

					 tail_shade = tail_size * 3;
					 if (tail_shade > 29)
							tail_shade = 29;

						pulse_size += 	(move_time) * 0.1;

      draw_new_move_tail(pulse_x + cos(f_angle) * pulse_size * zoom,
																									pulse_y + sin(f_angle) * pulse_size * zoom,
																									f_angle + PI,
																									tail_size * 0.6,
																									proc->player_index,
																									tail_shade,
																									move_time,
																									1);//			int drand_seed)


       bloom_long(1, pulse_x, pulse_y,
																		f_angle + PI,
																		end_time * 4.0 * zoom,
																		colours.bloom_centre [proc->player_index] [24],
																		colours.bloom_edge [proc->player_index] [12],
																		colours.bloom_edge [proc->player_index] [1],
																		(20 + move_power_display * 0.12) * zoom,
																		(10 + move_power_display * 0.7) * zoom);

/*
     draw_new_pulse_tail(pulse_x, pulse_y, x_step, y_step,
																					f_angle + PI,
																					 move_time,
																					0, // start_time
																					end_time,
																					end_time,
																					tail_width * 30.0, // tail_width
																					 proc->player_index,
																					 6 + move_power_display,// / 16, // shade
																					 (proc->index + proc_link_index) - move_time,
																					 4, // size
																					 0.6 + (drand(30, 1) + move_power_display * 10) * 0.02, // blob_scale
																					 0); // burst?*/

/*
      draw_pulse_tail(pulse_x, pulse_y,
																						x_step, y_step,
																					 f_angle + PI,
																					 move_time,
																					 0, // start_time
																					 end_time,
																					 end_time,
																					 tail_width,
																					 proc->player_index,
																					 6 + move_power_display,// / 16, // shade
																					 (proc->index + proc_link_index) - move_time,
																					 4, // size
																					 0.6 + (drand(30, 1) + move_power_display * 10) * 0.002, // blob_scale
																					 0); // burst?*/
/*
      draw_pulse_tail(pulse_x, pulse_y,
																						x_step, y_step,
																					 f_angle + PI,
																					 move_time,
																					 0, // start_time
																					 end_time,
																					 end_time,
																					 tail_width,
																					 proc->player_index,
																					 6 + move_power_display,// / 16, // shade
																					 (proc->index + proc_link_index) - move_time,
																					 4, // size
																					 0.6 + (drand(30, 1) + move_power_display * 10) * 0.002, // blob_scale
																					 0); // burst?


       bloom_long(1, pulse_x, pulse_y,
																		f_angle + PI,
																		end_time * 4.0 * zoom,
																		colours.bloom_centre [proc->player_index] [24],
																		colours.bloom_edge [proc->player_index] [12],
																		colours.bloom_edge [proc->player_index] [1],
																		(20 + move_power_display * 0.12) * zoom,
																		(10 + move_power_display * 0.7) * zoom);

					 tail_width = (move_power_display * 0.009) + 0.04;//0.025;
      seed_drand((proc->index + proc_link_index) - move_time);
      end_time = 1 + drand(3, 1) + move_power_display / 4;//30;//packet_time;

      draw_pulse_tail(pulse_x, pulse_y,
																						x_step, y_step,
																					 f_angle + PI,
																					 move_time,
																					 0, // start_time
																					 end_time,
																					 end_time,
																					 tail_width,
																					 proc->player_index,
																					 12 + move_power_display, // shade
																					 (proc->index + proc_link_index) - move_time,
																					 2, // size
																					 0.4 + (drand(20, 1) + move_power_display * 10) * 0.001, // blob_scale
																					 0); // burst?
*/
/*

						seed_drand(proc->index + proc->position.x); // can assume proc != NULL if obj_inst != NULL
   	  float pulse_size = 2 + ((float) obj_inst->move_power / 20) + drand(30,1) * 0.1;
   	  float pulse_dist = 13 + pulse_size + drand(20,1) * 0.1;
   	  float pulse_x = vx + cos(f_angle) * pulse_dist * zoom;
   	  float pulse_y = vy + sin(f_angle) * pulse_dist * zoom;
   	  int pulse_shade = obj_inst->move_power / 10;
// outer part


      radial_elongated_blob_10(OBJECT_OVER_LAYER, pulse_x, pulse_y, f_angle, colours.packet [proc->player_index] [6 + pulse_shade],
																															pulse_size, 2, proc->position.x ^ w.world_time, proc->position.y);

// inner part
   	  pulse_size = 1 + ((float) obj_inst->move_power / 40) + drand(30,1) * 0.1;
   	  pulse_dist = 16 + pulse_size + drand(20,1) * 0.1;


   	  pulse_x = vx + cos(f_angle) * pulse_dist * zoom;
   	  pulse_y = vy + sin(f_angle) * pulse_dist * zoom;


//      radial_elongated_blob_10(2, pulse_x, pulse_y, f_angle, colours.packet [proc->player_index] [6 + pulse_shade],
//																															pulse_size, 2, (int) proc_x ^ w.world_time, proc_y);

      radial_blob(OBJECT_OVER_LAYER, pulse_x, pulse_y,
																		f_angle, 8, colours.packet [proc->player_index] [18 + pulse_shade],
																		pulse_size, 2, proc->position.x ^ w.world_time, proc->position.y);
*/
					}





//   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], vx - 20, vy, ALLEGRO_ALIGN_RIGHT, "%i;%f", obj_inst->move_power, al_fixtof(obj_inst->move_accel_rate));
		}

  break;

			 case OBJECT_TYPE_PULSE:
			 case OBJECT_TYPE_PULSE_L:
			 case OBJECT_TYPE_PULSE_XL:
    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);

     {
     int object_extra_size = obj->type - OBJECT_TYPE_PULSE;

     dist_front = 12 + object_extra_size;
     dist_side = 8 + object_extra_size;
     dist_back = 6 + object_extra_size;


     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;

			  if (obj_inst != NULL // will be NULL if this is being drawn on the design screen
						&& obj_inst->attack_last_fire_timestamp > w.world_time - 16)
					{

// first draw the underlay:
  vertex_list [0] [0] = vx + cos(f_angle) * (dist_front - 2) * zoom;
  vertex_list [0] [1] = vy + sin(f_angle) * (dist_front - 2) * zoom;
  vertex_list [1] [0] = vx + cos(f_angle + PI/2) * (dist_side - 2) * zoom;
  vertex_list [1] [1] = vy + sin(f_angle + PI/2) * (dist_side - 2) * zoom;
  vertex_list [2] [0] = vx + cos(f_angle + PI) * (dist_back - 2) * zoom;
  vertex_list [2] [1] = vy + sin(f_angle + PI) * (dist_back - 2) * zoom;
  vertex_list [3] [0] = vx + cos(f_angle - PI/2) * (dist_side - 2) * zoom;
  vertex_list [3] [1] = vy + sin(f_angle - PI/2) * (dist_side - 2) * zoom;
//				vertex_list [4] [0],
//				vertex_list [4] [1]);

  add_poly_layer(OBJECT_UNDERLAY_LAYER, 4, proc_col [PROC_COL_UNDERLAY]);



      float time_since_firing = w.world_time - obj_inst->attack_last_fire_timestamp;
      float separation;
/*      if (time_since_firing < 4)
							separation = time_since_firing;
						  else*/
							  separation = (16 - (time_since_firing)) / 2;

						separation *= zoom;

       	front_vertex_x = vx + cos(f_angle) * (dist_front / 2) * zoom;
       	front_vertex_y = vy + sin(f_angle) * (dist_front / 2) * zoom;

       	back_vertex_x = vx + cos(f_angle + PI) * dist_back * zoom;
       	back_vertex_y = vy + sin(f_angle + PI) * dist_back * zoom;

       	left_vertex_x = back_vertex_x + cos(f_angle - PI/4) * dist_side * zoom;
       	left_vertex_y = back_vertex_y + sin(f_angle - PI/4) * dist_side * zoom;

       	right_vertex_x = back_vertex_x + cos(f_angle + PI/4) * dist_side * zoom;
       	right_vertex_y = back_vertex_y + sin(f_angle + PI/4) * dist_side * zoom;

       	float displaced_x = cos(f_angle + PI) * separation;
       	float displaced_y = sin(f_angle + PI) * separation;


  vertex_list [0] [0] = front_vertex_x + displaced_x;
  vertex_list [0] [1] = front_vertex_y + displaced_y;
  vertex_list [1] [0] = left_vertex_x + displaced_x;
  vertex_list [1] [1] = left_vertex_y + displaced_y;
  vertex_list [2] [0] = back_vertex_x + displaced_x;
  vertex_list [2] [1] = back_vertex_y + displaced_y;
  vertex_list [3] [0] = right_vertex_x + displaced_x;
  vertex_list [3] [1] = right_vertex_y + displaced_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

/*
 	separation = (32 - (time_since_firing)) / 4;
 	if (separation > 4)
			separation = 4;*/

		separation /= 4;

  displaced_x = cos(f_angle + PI/2) * separation;
  displaced_y = sin(f_angle + PI/2) * separation;

  vertex_list [0] [0] = front_vertex_x + displaced_x;
  vertex_list [0] [1] = front_vertex_y + displaced_y;
  vertex_list [1] [0] = front_vertex_x + displaced_x + cos(f_angle) * (dist_front/2) * zoom;
  vertex_list [1] [1] = front_vertex_y + displaced_y + sin(f_angle) * (dist_front/2) * zoom;
  vertex_list [2] [0] = right_vertex_x + displaced_x;
  vertex_list [2] [1] = right_vertex_y + displaced_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 3, proc_col [PROC_COL_OBJECT_1]);

  displaced_x = cos(f_angle - PI/2) * separation;
  displaced_y = sin(f_angle - PI/2) * separation;

  vertex_list [0] [0] = front_vertex_x + displaced_x;
  vertex_list [0] [1] = front_vertex_y + displaced_y;
  vertex_list [1] [0] = front_vertex_x + displaced_x + cos(f_angle) * (dist_front/2) * zoom;
  vertex_list [1] [1] = front_vertex_y + displaced_y + sin(f_angle) * (dist_front/2) * zoom;
  vertex_list [2] [0] = left_vertex_x + displaced_x;
  vertex_list [2] [1] = left_vertex_y + displaced_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 3, proc_col [PROC_COL_OBJECT_1]);


//  if (time_since_firing > 16)
//			break;

  shade = (32 - (time_since_firing * 2));
  if (shade >= CLOUD_SHADES)
			shade = CLOUD_SHADES - 1;

	 double_circle_with_bloom(4, front_vertex_x, front_vertex_y, 12 + (shade * 0.3) * (1 + object_extra_size), proc->player_index, shade);


/*
     radial_circle(2,
																			front_vertex_x,
																			front_vertex_y,
																			16,
																			colours.packet [proc->player_index] [shade],
																			4 + shade / 2);
																			*/
/*
     draw_ring(2,
															front_vertex_x,
															front_vertex_y,
															4 + shade / 2,
															4 + shade / 2,
															16,
															colours.packet [proc->player_index] [shade]);*/
						break;
					} // end of part of packet case

//     vx += (cos(f_angle) * 3) * zoom;
//     vy += (sin(f_angle) * 3) * zoom;


       	front_vertex_x = vx + cos(f_angle) * dist_front * zoom;
       	front_vertex_y = vy + sin(f_angle) * dist_front * zoom;

       	back_vertex_x = vx + cos(f_angle + PI) * dist_back * zoom;
       	back_vertex_y = vy + sin(f_angle + PI) * dist_back * zoom;

       	left_vertex_x = back_vertex_x + cos(f_angle - PI/4) * dist_side * zoom;
       	left_vertex_y = back_vertex_y + sin(f_angle - PI/4) * dist_side * zoom;

       	right_vertex_x = back_vertex_x + cos(f_angle + PI/4) * dist_side * zoom;
       	right_vertex_y = back_vertex_y + sin(f_angle + PI/4) * dist_side * zoom;

  vertex_list [0] [0] = front_vertex_x;
  vertex_list [0] [1] = front_vertex_y;
  vertex_list [1] [0] = left_vertex_x;
  vertex_list [1] [1] = left_vertex_y;
  vertex_list [2] [0] = back_vertex_x;
  vertex_list [2] [1] = back_vertex_y;
  vertex_list [3] [0] = right_vertex_x;
  vertex_list [3] [1] = right_vertex_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

//  if (obj_inst != NULL)
//   al_draw_textf(font[FONT_SQUARE].fnt, fill_col, vx - 20, vy, ALLEGRO_ALIGN_RIGHT, "%i", obj_inst->move_power);
     }

  break;

/*
			 case OBJECT_TYPE_ULTRA:
			 case OBJECT_TYPE_ULTRA_DIR:
    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);
    {
//     int burst_object_extra_size = obj->type - OBJECT_TYPE_BURST;

     dist_front = 6 * zoom;// + burst_object_extra_size;
     dist_side = 4 * zoom;// + burst_object_extra_size;
     dist_back = 7 * zoom;// + burst_object_extra_size;
     float dist_back2;
     if (obj->type == OBJECT_TYPE_ULTRA)
						dist_back2 = 6 * zoom;
					  else
								dist_back2 = 5 * zoom;

     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;

     float side_offset_x = cos(f_angle + PI/2);
     float side_offset_y = sin(f_angle + PI/2);

     float dist_outer = 3;
     float dist_side_under = 2.5 * zoom;


       	front_vertex_x = vx + cos(f_angle) * (dist_front + dist_outer * zoom);
       	front_vertex_y = vy + sin(f_angle) * (dist_front + dist_outer * zoom);

       	back_vertex_x = vx + cos(f_angle + PI) * dist_back;
       	back_vertex_y = vy + sin(f_angle + PI) * dist_back;

  vertex_list [0] [0] = front_vertex_x + side_offset_x * dist_side_under;
  vertex_list [0] [1] = front_vertex_y + side_offset_y * dist_side_under;
  vertex_list [1] [0] = front_vertex_x - side_offset_x * dist_side_under;
  vertex_list [1] [1] = front_vertex_y - side_offset_y * dist_side_under;
  vertex_list [2] [0] = back_vertex_x - side_offset_x * dist_side_under;
  vertex_list [2] [1] = back_vertex_y - side_offset_y * dist_side_under;
  vertex_list [3] [0] = back_vertex_x + side_offset_x * dist_side_under;
  vertex_list [3] [1] = back_vertex_y + side_offset_y * dist_side_under;

  add_poly_layer(OBJECT_UNDERLAY_LAYER, 4, proc_col [PROC_COL_OBJECT_2]);


       	front_vertex_x = vx + cos(f_angle) * dist_front;
       	front_vertex_y = vy + sin(f_angle) * dist_front;

//       	back_vertex_x = vx + cos(f_angle + PI) * dist_back;
//       	back_vertex_y = vy + sin(f_angle + PI) * dist_back;

  vertex_list [0] [0] = front_vertex_x + side_offset_x * dist_side;
  vertex_list [0] [1] = front_vertex_y + side_offset_y * dist_side;
  vertex_list [1] [0] = front_vertex_x - side_offset_x * dist_side;
  vertex_list [1] [1] = front_vertex_y - side_offset_y * dist_side;
  vertex_list [2] [0] = back_vertex_x - side_offset_x * dist_back2;
  vertex_list [2] [1] = back_vertex_y - side_offset_y * dist_back2;
  vertex_list [3] [0] = back_vertex_x - cos(f_angle) * 3;
  vertex_list [3] [1] = back_vertex_y - sin(f_angle) * 3;
  vertex_list [4] [0] = back_vertex_x + side_offset_x * dist_back2;
  vertex_list [4] [1] = back_vertex_y + side_offset_y * dist_back2;
//  vertex_list [4] [0] = right_vertex_x;
//  vertex_list [4] [1] = right_vertex_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 5, proc_col [PROC_COL_OBJECT_1]);


    }
  break;
*/
/*
			 case OBJECT_TYPE_PULSE_XL:
    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);

     {
     int object_extra_size = obj->type - OBJECT_TYPE_PULSE;

     dist_front = 12 + object_extra_size;
     dist_side = 8 + object_extra_size;
     dist_back = 6 + object_extra_size;


     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;

			  if (obj_inst != NULL // will be NULL if this is being drawn on the design screen
						&& obj_inst->attack_last_fire_timestamp > w.world_time - 16)
					{

// first draw the underlay:
  vertex_list [0] [0] = vx + cos(f_angle) * (dist_front - 2) * zoom;
  vertex_list [0] [1] = vy + sin(f_angle) * (dist_front - 2) * zoom;
  vertex_list [1] [0] = vx + cos(f_angle + PI/2) * (dist_side - 2) * zoom;
  vertex_list [1] [1] = vy + sin(f_angle + PI/2) * (dist_side - 2) * zoom;
  vertex_list [2] [0] = vx + cos(f_angle + PI) * (dist_back - 2) * zoom;
  vertex_list [2] [1] = vy + sin(f_angle + PI) * (dist_back - 2) * zoom;
  vertex_list [3] [0] = vx + cos(f_angle - PI/2) * (dist_side - 2) * zoom;
  vertex_list [3] [1] = vy + sin(f_angle - PI/2) * (dist_side - 2) * zoom;

  add_poly_layer(OBJECT_UNDERLAY_LAYER, 4, proc_col [PROC_COL_UNDERLAY]);



      float time_since_firing = w.world_time - obj_inst->attack_last_fire_timestamp;
      float separation;
/ *      if (time_since_firing < 4)
							separation = time_since_firing;
						  else* /
							  separation = (16 - (time_since_firing)) / 2;

						separation *= zoom;

       	front_vertex_x = vx + cos(f_angle) * (dist_front / 2) * zoom;
       	front_vertex_y = vy + sin(f_angle) * (dist_front / 2) * zoom;

       	back_vertex_x = vx + cos(f_angle + PI) * dist_back * zoom;
       	back_vertex_y = vy + sin(f_angle + PI) * dist_back * zoom;

       	left_vertex_x = back_vertex_x + cos(f_angle - PI/4) * dist_side * zoom;
       	left_vertex_y = back_vertex_y + sin(f_angle - PI/4) * dist_side * zoom;

       	right_vertex_x = back_vertex_x + cos(f_angle + PI/4) * dist_side * zoom;
       	right_vertex_y = back_vertex_y + sin(f_angle + PI/4) * dist_side * zoom;

       	float displaced_x = cos(f_angle + PI) * separation;
       	float displaced_y = sin(f_angle + PI) * separation;


  vertex_list [0] [0] = front_vertex_x + displaced_x;
  vertex_list [0] [1] = front_vertex_y + displaced_y;
  vertex_list [1] [0] = left_vertex_x + displaced_x;
  vertex_list [1] [1] = left_vertex_y + displaced_y;
  vertex_list [2] [0] = back_vertex_x + displaced_x;
  vertex_list [2] [1] = back_vertex_y + displaced_y;
  vertex_list [3] [0] = right_vertex_x + displaced_x;
  vertex_list [3] [1] = right_vertex_y + displaced_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

/ *
 	separation = (32 - (time_since_firing)) / 4;
 	if (separation > 4)
			separation = 4;* /

		separation /= 4;

  displaced_x = cos(f_angle + PI/2) * separation;
  displaced_y = sin(f_angle + PI/2) * separation;

  vertex_list [0] [0] = front_vertex_x + displaced_x;
  vertex_list [0] [1] = front_vertex_y + displaced_y;
  vertex_list [1] [0] = front_vertex_x + displaced_x + cos(f_angle) * (dist_front/2) * zoom;
  vertex_list [1] [1] = front_vertex_y + displaced_y + sin(f_angle) * (dist_front/2) * zoom;
  vertex_list [2] [0] = right_vertex_x + displaced_x;
  vertex_list [2] [1] = right_vertex_y + displaced_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 3, proc_col [PROC_COL_OBJECT_1]);

  displaced_x = cos(f_angle - PI/2) * separation;
  displaced_y = sin(f_angle - PI/2) * separation;

  vertex_list [0] [0] = front_vertex_x + displaced_x;
  vertex_list [0] [1] = front_vertex_y + displaced_y;
  vertex_list [1] [0] = front_vertex_x + displaced_x + cos(f_angle) * (dist_front/2) * zoom;
  vertex_list [1] [1] = front_vertex_y + displaced_y + sin(f_angle) * (dist_front/2) * zoom;
  vertex_list [2] [0] = left_vertex_x + displaced_x;
  vertex_list [2] [1] = left_vertex_y + displaced_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 3, proc_col [PROC_COL_OBJECT_1]);


  if (time_since_firing > 16)
			break;

  shade = (32 - (time_since_firing * 2));
  if (shade >= CLOUD_SHADES)
			shade = CLOUD_SHADES - 1;

		float flash_size = (float) (17 - time_since_firing) * zoom * 2.0;// * 0.8;

		front_vertex_x = vx + cos(f_angle) * 9 * zoom;
		front_vertex_y = vy + sin(f_angle) * 9 * zoom;

  bloom_circle(OBJECT_OVER_LAYER, front_vertex_x, front_vertex_y, colours.bloom_centre [proc->player_index] [shade], colours.bloom_edge [proc->player_index] [0], flash_size * 3);

  vertex_list [0] [0] = front_vertex_x + cos(f_angle) * flash_size;
  vertex_list [0] [1] = front_vertex_y + sin(f_angle) * flash_size;
  vertex_list [1] [0] = front_vertex_x + cos(f_angle + PI/2) * flash_size / 2;
  vertex_list [1] [1] = front_vertex_y + sin(f_angle + PI/2) * flash_size / 2;
  vertex_list [2] [0] = front_vertex_x + cos(f_angle + PI) * flash_size;
  vertex_list [2] [1] = front_vertex_y + sin(f_angle + PI) * flash_size;
  vertex_list [3] [0] = front_vertex_x + cos(f_angle - PI/2) * flash_size / 2;
  vertex_list [3] [1] = front_vertex_y + sin(f_angle - PI/2) * flash_size / 2;

//  add_poly_layer(OBJECT_OVER_LAYER, 4, colours.packet [proc->player_index] [shade]);//, colours.packet [proc->player_index] [shade / 2]);


     radial_circle(2,
																			front_vertex_x,
																			front_vertex_y,
																			16,
																			colours.packet [proc->player_index] [shade],
																			4 + shade / 2);
/ *
     draw_ring(2,
															front_vertex_x,
															front_vertex_y,
															4 + shade / 2,
															4 + shade / 2,
															16,
															colours.packet [proc->player_index] [shade]);* /
						break;
					} // end of part of packet case

//     vx += (cos(f_angle) * 3) * zoom;
//     vy += (sin(f_angle) * 3) * zoom;


       	front_vertex_x = vx + cos(f_angle) * dist_front * zoom;
       	front_vertex_y = vy + sin(f_angle) * dist_front * zoom;

       	back_vertex_x = vx + cos(f_angle + PI) * dist_back * zoom;
       	back_vertex_y = vy + sin(f_angle + PI) * dist_back * zoom;

       	left_vertex_x = back_vertex_x + cos(f_angle - PI/4) * dist_side * zoom;
       	left_vertex_y = back_vertex_y + sin(f_angle - PI/4) * dist_side * zoom;

       	right_vertex_x = back_vertex_x + cos(f_angle + PI/4) * dist_side * zoom;
       	right_vertex_y = back_vertex_y + sin(f_angle + PI/4) * dist_side * zoom;

  vertex_list [0] [0] = front_vertex_x;
  vertex_list [0] [1] = front_vertex_y;
  vertex_list [1] [0] = left_vertex_x;
  vertex_list [1] [1] = left_vertex_y;
  vertex_list [2] [0] = back_vertex_x;
  vertex_list [2] [1] = back_vertex_y;
  vertex_list [3] [0] = right_vertex_x;
  vertex_list [3] [1] = right_vertex_y;

  add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

//  if (obj_inst != NULL)
//   al_draw_textf(font[FONT_SQUARE].fnt, fill_col, vx - 20, vy, ALLEGRO_ALIGN_RIGHT, "%i", obj_inst->move_power);
     }

  break;
*/
  case OBJECT_TYPE_STREAM:
  case OBJECT_TYPE_STREAM_DIR:
			{
    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);

//     dist_front = 6;// * zoom;// + pr->method [pr->vertex_method [vertex]].extension [0];
//     dist_side = 8;// * zoom;// + pr->method [pr->vertex_method [vertex]].extension [0] / 2;
     dist_back = 8;// * zoom;// + pr->method [pr->vertex_method [vertex]].extension [0];
//     float dist_far_front = 8;

     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;

    	right_back_vertex_x = vx + cos(f_angle + PI - PI/12) * dist_back * zoom;
    	right_back_vertex_y = vy + sin(f_angle + PI - PI/12) * dist_back * zoom;

    	left_back_vertex_x = vx + cos(f_angle + PI + PI/12) * dist_back * zoom;
    	left_back_vertex_y = vy + sin(f_angle + PI + PI/12) * dist_back * zoom;

    	left_vertex_x = vx + cos(f_angle - PI/2) * dist_back * zoom;
    	left_vertex_y = vy + sin(f_angle - PI/2) * dist_back * zoom;

    	right_vertex_x = vx + cos(f_angle + PI/2) * dist_back * zoom;
    	right_vertex_y = vy + sin(f_angle + PI/2) * dist_back * zoom;

// front vertices are calculated from left_front to make rotation around left_front_vertex easy (could also have been around right)
    	front_vertex_x = left_vertex_x + cos(f_angle + PI/4) * 11.31370 * zoom; // sqrt(128)
    	front_vertex_y = left_vertex_y + sin(f_angle + PI/4) * 11.31370 * zoom;

    	float far_front_vertex_x = left_vertex_x + cos(f_angle + 0.463647) * 17.88854 * zoom; // atan(8/16), sqrt(256+64)
    	float far_front_vertex_y = left_vertex_y + sin(f_angle + 0.463647) * 17.88854 * zoom;


			  if (obj_inst != NULL // will be NULL if this is being drawn on the design screen
						&& obj_inst->attack_last_fire_timestamp > w.world_time - STREAM_TOTAL_FIRING_TIME)
					{
       vertex_list [0] [0] = vx + cos(f_angle + PI) * 6 * zoom;
       vertex_list [0] [1] = vy + sin(f_angle + PI) * 6 * zoom;
       vertex_list [1] [0] = vx + cos(f_angle - PI / 2) * 7 * zoom;
       vertex_list [1] [1] = vy + sin(f_angle - PI / 2) * 7 * zoom;
       vertex_list [2] [0] = vx + cos(f_angle) * 14 * zoom;
       vertex_list [2] [1] = vy + sin(f_angle) * 14 * zoom;
       vertex_list [3] [0] = vx + cos(f_angle + PI / 2) * 7 * zoom;
       vertex_list [3] [1] = vy + sin(f_angle + PI / 2) * 7 * zoom;



       add_poly_layer(OBJECT_UNDERLAY_LAYER, 4, proc_col [PROC_COL_OBJECT_2]);


       vertex_list [0] [0] = left_vertex_x;
       vertex_list [0] [1] = left_vertex_y;
       vertex_list [1] [0] = left_back_vertex_x;
       vertex_list [1] [1] = left_back_vertex_y;
       vertex_list [2] [0] = right_back_vertex_x;
       vertex_list [2] [1] = right_back_vertex_y;
       vertex_list [3] [0] = right_vertex_x;
       vertex_list [3] [1] = right_vertex_y;
       vertex_list [4] [0] = front_vertex_x;
       vertex_list [4] [1] = front_vertex_y;

       add_poly_layer(OBJECT_MAIN_LAYER, 5, proc_col [PROC_COL_OBJECT_1]);

       int side_angle_base = (w.world_time - obj_inst->attack_last_fire_timestamp);

       if (side_angle_base > 24)
							{
								if (side_angle_base < 40)
								 side_angle_base = 24;
							   else
										 side_angle_base = 64 - side_angle_base;
							}

       float side_angle = side_angle_base * 0.015;

       vertex_list [0] [0] = left_vertex_x;
       vertex_list [0] [1] = left_vertex_y;
       vertex_list [1] [0] = left_vertex_x + cos(f_angle + PI/4 - side_angle) * 11.31370 * zoom; // sqrt(128);
       vertex_list [1] [1] = left_vertex_y + sin(f_angle + PI/4 - side_angle) * 11.31370 * zoom;;
       vertex_list [2] [0] = left_vertex_x + cos(f_angle + 0.463647 - side_angle) * 17.88854 * zoom;
       vertex_list [2] [1] = left_vertex_y + sin(f_angle + 0.463647 - side_angle) * 17.88854 * zoom;

       add_poly_layer(OBJECT_MAIN_LAYER, 3, proc_col [PROC_COL_OBJECT_1]);


       vertex_list [0] [0] = right_vertex_x;
       vertex_list [0] [1] = right_vertex_y;
       vertex_list [1] [0] = right_vertex_x + cos(f_angle - PI/4 + side_angle) * 11.31370 * zoom; // sqrt(128);
       vertex_list [1] [1] = right_vertex_y + sin(f_angle - PI/4 + side_angle) * 11.31370 * zoom;;
       vertex_list [2] [0] = right_vertex_x + cos(f_angle - 0.463647 + side_angle) * 17.88854 * zoom;
       vertex_list [2] [1] = right_vertex_y + sin(f_angle - 0.463647 + side_angle) * 17.88854 * zoom;

       add_poly_layer(OBJECT_MAIN_LAYER, 3, proc_col [PROC_COL_OBJECT_1]);


					}
					 else
						{

       vertex_list [0] [0] = left_vertex_x;
       vertex_list [0] [1] = left_vertex_y;
       vertex_list [1] [0] = left_back_vertex_x;
       vertex_list [1] [1] = left_back_vertex_y;
       vertex_list [2] [0] = right_back_vertex_x;
       vertex_list [2] [1] = right_back_vertex_y;
       vertex_list [3] [0] = right_vertex_x;
       vertex_list [3] [1] = right_vertex_y;
       vertex_list [4] [0] = far_front_vertex_x;
       vertex_list [4] [1] = far_front_vertex_y;

       add_poly_layer(OBJECT_MAIN_LAYER, 5, proc_col [PROC_COL_OBJECT_1]);


						}


			}
			break; // end OBJECT_TYPE_STREAM


	 case OBJECT_TYPE_ULTRA:
	 case OBJECT_TYPE_ULTRA_DIR:
//	 case OBJECT_TYPE_BURST_L:
	 //case OBJECT_TYPE_BURST_XL:
//	 case OBJECT_TYPE_BURST_XXL:
			{
    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);

     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;

				float side_separation_x;// = cos(f_angle + PI/2) * 0.5 * zoom;
				float side_separation_y;// = sin(f_angle + PI/2) * 0.5 * zoom;

    float back_separation_x;// = cos(f_angle + PI) * 0.5 * zoom;
				float back_separation_y;// = sin(f_angle + PI) * 0.5 * zoom;

							float side_separation_amount = 0.5;
							float back_separation_amount = 0.5;

			  if (obj_inst != NULL // will be NULL if this is being drawn on the design screen
						&& obj_inst->attack_last_fire_timestamp > w.world_time - 128)
					{
							float time_since_firing = w.world_time - obj_inst->attack_last_fire_timestamp;
							if (time_since_firing < 8)
							{
								side_separation_amount = time_since_firing / 4;
								back_separation_amount = time_since_firing;
							}
							  else
									{
										if (time_since_firing < 96)
										{
								   side_separation_amount = 2;
								   back_separation_amount = 8 - (time_since_firing-8) / 8;
								   if (back_separation_amount < 0.5)
												back_separation_amount = 0.5;
										}
										 else
											{
								    side_separation_amount = (128 - time_since_firing) / 16;
								    if (side_separation_amount < 0.5)
												 side_separation_amount = 0.5;
											}
									}

        if (time_since_firing < 32)
								{

         shade = (32 - (time_since_firing));
         if (shade >= CLOUD_SHADES)
			       shade = CLOUD_SHADES - 1;

	        double_circle_with_bloom(4, vx + cos(f_angle) * (8), vy + sin(f_angle) * (8), 10 + (shade * 2), proc->player_index, shade);

								}



       if (time_since_firing <= 32)
							{

/*
		      float flash_size = (float) (32 - time_since_firing) * zoom;// * 0.8;

		      front_vertex_x = vx + cos(f_angle) * 2 * zoom;
		      front_vertex_y = vy + sin(f_angle) * 2 * zoom;

  vertex_list [0] [0] = front_vertex_x + cos(f_angle) * flash_size;
  vertex_list [0] [1] = front_vertex_y + sin(f_angle) * flash_size;
  vertex_list [1] [0] = front_vertex_x + cos(f_angle + PI/2) * flash_size / 2;
  vertex_list [1] [1] = front_vertex_y + sin(f_angle + PI/2) * flash_size / 2;
  vertex_list [2] [0] = front_vertex_x + cos(f_angle + PI) * flash_size;
  vertex_list [2] [1] = front_vertex_y + sin(f_angle + PI) * flash_size;
  vertex_list [3] [0] = front_vertex_x + cos(f_angle - PI/2) * flash_size / 2;
  vertex_list [3] [1] = front_vertex_y + sin(f_angle - PI/2) * flash_size / 2;

  add_poly_layer(OBJECT_MAIN_LAYER+1, 4, colours.packet [proc->player_index] [shade / 2]);//, colours.packet [proc->player_index] [shade / 2]);

     radial_circle(4,
																			 vertex_list [0] [0],
																			 vertex_list [0] [1],
																			16,
																			colours.packet [proc->player_index] [shade],
																			4 + shade);

  flash_size /= 2;

  vertex_list [0] [0] = front_vertex_x + cos(f_angle) * flash_size;
  vertex_list [0] [1] = front_vertex_y + sin(f_angle) * flash_size;
  vertex_list [1] [0] = front_vertex_x + cos(f_angle + PI/2) * flash_size / 2;
  vertex_list [1] [1] = front_vertex_y + sin(f_angle + PI/2) * flash_size / 2;
  vertex_list [2] [0] = front_vertex_x + cos(f_angle + PI) * flash_size;
  vertex_list [2] [1] = front_vertex_y + sin(f_angle + PI) * flash_size;
  vertex_list [3] [0] = front_vertex_x + cos(f_angle - PI/2) * flash_size / 2;
  vertex_list [3] [1] = front_vertex_y + sin(f_angle - PI/2) * flash_size / 2;

  add_poly_layer(OBJECT_MAIN_LAYER+1, 4, colours.packet [proc->player_index] [shade]);//, colours.packet [proc->player_index] [shade / 2]);
*/

							}


					}

       back_separation_x = cos(f_angle + PI) * back_separation_amount * zoom;
				   back_separation_y = sin(f_angle + PI) * back_separation_amount * zoom;

				   side_separation_x = cos(f_angle + PI/2) * side_separation_amount * zoom;
				   side_separation_y = sin(f_angle + PI/2) * side_separation_amount * zoom;

#define OTB_SIZE 6

        vertex_list [0] [0] = vx + cos(f_angle) * (OTB_SIZE+2) * zoom + side_separation_x;
        vertex_list [0] [1] = vy + sin(f_angle) * (OTB_SIZE+2) * zoom + side_separation_y;
        vertex_list [1] [0] = vx + cos(f_angle + PI/12) * (OTB_SIZE+7) * zoom + side_separation_x;
        vertex_list [1] [1] = vy + sin(f_angle + PI/12) * (OTB_SIZE+7) * zoom + side_separation_y;
        vertex_list [2] [0] = vx + cos(f_angle + PI/2 + PI/8) * (OTB_SIZE+2) * zoom + side_separation_x;
        vertex_list [2] [1] = vy + sin(f_angle + PI/2 + PI/8) * (OTB_SIZE+2) * zoom + side_separation_y;
        vertex_list [3] [0] = vx + cos(f_angle + PI - PI/6) * (OTB_SIZE+2) * zoom + side_separation_x;
        vertex_list [3] [1] = vy + sin(f_angle + PI - PI/6) * (OTB_SIZE+2) * zoom + side_separation_y;
//        vertex_list [4] [0] = vx + cos(f_angle + PI - PI/10) * 6 * zoom + side_separation_x;
//        vertex_list [4] [1] = vy + sin(f_angle + PI - PI/10) * 6 * zoom + side_separation_y;
        vertex_list [4] [0] = vx + cos(f_angle + PI) * 1 * zoom + side_separation_x;
        vertex_list [4] [1] = vy + sin(f_angle + PI) * 1 * zoom + side_separation_y;

        add_poly_layer(OBJECT_MAIN_LAYER, 5, proc_col [PROC_COL_OBJECT_1]);


        vertex_list [0] [0] = vx + cos(f_angle) * (OTB_SIZE+2) * zoom - side_separation_x;
        vertex_list [0] [1] = vy + sin(f_angle) * (OTB_SIZE+2) * zoom - side_separation_y;
        vertex_list [1] [0] = vx + cos(f_angle - PI/12) * (OTB_SIZE+7) * zoom - side_separation_x;
        vertex_list [1] [1] = vy + sin(f_angle - PI/12) * (OTB_SIZE+7) * zoom - side_separation_y;
        vertex_list [2] [0] = vx + cos(f_angle - PI/2 - PI/8) * (OTB_SIZE+2) * zoom - side_separation_x;
        vertex_list [2] [1] = vy + sin(f_angle - PI/2 - PI/8) * (OTB_SIZE+2) * zoom - side_separation_y;
        vertex_list [3] [0] = vx + cos(f_angle - PI + PI/6) * (OTB_SIZE+2) * zoom - side_separation_x;
        vertex_list [3] [1] = vy + sin(f_angle - PI + PI/6) * (OTB_SIZE+2) * zoom - side_separation_y;
//        vertex_list [4] [0] = vx + cos(f_angle - PI + PI/10) * 6 * zoom - side_separation_x;
//        vertex_list [4] [1] = vy + sin(f_angle - PI + PI/10) * 6 * zoom - side_separation_y;
        vertex_list [4] [0] = vx + cos(f_angle - PI) * 1 * zoom - side_separation_x;
        vertex_list [4] [1] = vy + sin(f_angle - PI) * 1 * zoom - side_separation_y;

        add_poly_layer(OBJECT_MAIN_LAYER, 5, proc_col [PROC_COL_OBJECT_1]);


        vertex_list [0] [0] = vx + cos(f_angle + PI) * 1 * zoom + back_separation_x;
        vertex_list [0] [1] = vy + sin(f_angle + PI) * 1 * zoom + back_separation_y;
        vertex_list [1] [0] = vx + cos(f_angle - PI + PI/6) * (OTB_SIZE) * zoom + back_separation_x;
        vertex_list [1] [1] = vy + sin(f_angle - PI + PI/6) * (OTB_SIZE) * zoom + back_separation_y;
        vertex_list [2] [0] = vx + cos(f_angle + PI) * (OTB_SIZE+4) * zoom + back_separation_x;
        vertex_list [2] [1] = vy + sin(f_angle + PI) * (OTB_SIZE+4) * zoom + back_separation_y;
        vertex_list [3] [0] = vx + cos(f_angle + PI - PI/6) * (OTB_SIZE) * zoom + back_separation_x;
        vertex_list [3] [1] = vy + sin(f_angle + PI - PI/6) * (OTB_SIZE) * zoom + back_separation_y;

        add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

        vertex_list [0] [0] = vx + cos(f_angle) * 3 * zoom;
        vertex_list [0] [1] = vy + sin(f_angle) * 3 * zoom;
        vertex_list [1] [0] = vx + cos(f_angle + PI/2) * (OTB_SIZE+0) * zoom;
        vertex_list [1] [1] = vy + sin(f_angle + PI/2) * (OTB_SIZE+0) * zoom;
        vertex_list [2] [0] = vx + cos(f_angle + PI) * (OTB_SIZE+4) * zoom;
        vertex_list [2] [1] = vy + sin(f_angle + PI) * (OTB_SIZE+4) * zoom;
        vertex_list [3] [0] = vx + cos(f_angle - PI/2) * (OTB_SIZE+0) * zoom;
        vertex_list [3] [1] = vy + sin(f_angle - PI/2) * (OTB_SIZE+0) * zoom;

        add_poly_layer(OBJECT_UNDERLAY_LAYER, 4, proc_col [PROC_COL_OBJECT_2]);



			}
			break;

	 case OBJECT_TYPE_SLICE:
			{
    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);

     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;

				float side_separation_x;// = cos(f_angle + PI/2) * 0.5 * zoom;
				float side_separation_y;// = sin(f_angle + PI/2) * 0.5 * zoom;

    float back_separation_x;// = cos(f_angle + PI) * 0.5 * zoom;
				float back_separation_y;// = sin(f_angle + PI) * 0.5 * zoom;

							float side_separation_amount = 1.5;
							float back_separation_amount = 0.5;

			  if (obj_inst != NULL // will be NULL if this is being drawn on the design screen
						&& obj_inst->attack_last_fire_timestamp > w.world_time - 128)
					{
							float time_since_firing = w.world_time - obj_inst->attack_last_fire_timestamp;
							if (time_since_firing < 8)
							{
								side_separation_amount = time_since_firing / 4;
								back_separation_amount = time_since_firing;
							}
							  else
									{
										if (time_since_firing < 96)
										{
								   side_separation_amount = 2.5;
								   back_separation_amount = 8 - (time_since_firing-8) / 8;
								   if (back_separation_amount < 0.5)
												back_separation_amount = 0.5;
										}
										 else
											{
								    side_separation_amount = (128 - time_since_firing) / 16;
								    if (side_separation_amount < 1.5)
												 side_separation_amount = 1.5;
											}
									}



//       if (time_since_firing < 40)
							{
/*								int charge_flash = time_since_firing;
								if (charge_flash >= 24)
									charge_flash = 40 - charge_flash;
									 else
										{
											if (charge_flash > 16)
												charge_flash = 16;
										}*/
/*
        shade = charge_flash;
//        if (shade >= CLOUD_SHADES)
//			      shade = CLOUD_SHADES - 1;

		      float flash_size = (float) (charge_flash) * zoom;// * 0.8;

		      front_vertex_x = vx + cos(f_angle) * 2 * zoom;
		      front_vertex_y = vy + sin(f_angle) * 2 * zoom;

  vertex_list [0] [0] = front_vertex_x + cos(f_angle) * flash_size;
  vertex_list [0] [1] = front_vertex_y + sin(f_angle) * flash_size;*/
/*  vertex_list [1] [0] = front_vertex_x + cos(f_angle + PI/2) * flash_size / 2;
  vertex_list [1] [1] = front_vertex_y + sin(f_angle + PI/2) * flash_size / 2;
  vertex_list [2] [0] = front_vertex_x + cos(f_angle + PI) * flash_size;
  vertex_list [2] [1] = front_vertex_y + sin(f_angle + PI) * flash_size;
  vertex_list [3] [0] = front_vertex_x + cos(f_angle - PI/2) * flash_size / 2;
  vertex_list [3] [1] = front_vertex_y + sin(f_angle - PI/2) * flash_size / 2;

  add_poly_layer(OBJECT_MAIN_LAYER+1, 4, colours.packet [proc->player_index] [shade / 2]);//, colours.packet [proc->player_index] [shade / 2]);
*/
/*     radial_circle(4,
																			 vertex_list [0] [0],
																			 vertex_list [0] [1],
																			16,
																			colours.packet [proc->player_index] [shade],
																			charge_flash * 2);*/
/*
  flash_size /= 2;

  vertex_list [0] [0] = front_vertex_x + cos(f_angle) * flash_size;
  vertex_list [0] [1] = front_vertex_y + sin(f_angle) * flash_size;
  vertex_list [1] [0] = front_vertex_x + cos(f_angle + PI/2) * flash_size / 2;
  vertex_list [1] [1] = front_vertex_y + sin(f_angle + PI/2) * flash_size / 2;
  vertex_list [2] [0] = front_vertex_x + cos(f_angle + PI) * flash_size;
  vertex_list [2] [1] = front_vertex_y + sin(f_angle + PI) * flash_size;
  vertex_list [3] [0] = front_vertex_x + cos(f_angle - PI/2) * flash_size / 2;
  vertex_list [3] [1] = front_vertex_y + sin(f_angle - PI/2) * flash_size / 2;

  add_poly_layer(OBJECT_MAIN_LAYER+1, 4, colours.packet [proc->player_index] [shade]);//, colours.packet [proc->player_index] [shade / 2]);
*/

							}


					}

       back_separation_x = cos(f_angle + PI) * back_separation_amount * zoom;
				   back_separation_y = sin(f_angle + PI) * back_separation_amount * zoom;

				   side_separation_x = cos(f_angle + PI/2) * side_separation_amount * zoom;
				   side_separation_y = sin(f_angle + PI/2) * side_separation_amount * zoom;

#define SOTB_SIZE 6

        vertex_list [0] [0] = vx + cos(f_angle) * (SOTB_SIZE+2) * zoom + side_separation_x;
        vertex_list [0] [1] = vy + sin(f_angle) * (SOTB_SIZE+2) * zoom + side_separation_y;
//        vertex_list [1] [0] = vx + cos(f_angle + PI/12) * (SOTB_SIZE+7) * zoom + side_separation_x;
//        vertex_list [1] [1] = vy + sin(f_angle + PI/12) * (SOTB_SIZE+7) * zoom + side_separation_y;
        vertex_list [1] [0] = vx + cos(f_angle + PI/2 + PI/8) * (SOTB_SIZE+2) * zoom + side_separation_x;
        vertex_list [1] [1] = vy + sin(f_angle + PI/2 + PI/8) * (SOTB_SIZE+2) * zoom + side_separation_y;
        vertex_list [2] [0] = vx + cos(f_angle + PI - PI/6) * (SOTB_SIZE+4) * zoom + side_separation_x;
        vertex_list [2] [1] = vy + sin(f_angle + PI - PI/6) * (SOTB_SIZE+4) * zoom + side_separation_y;
//        vertex_list [4] [0] = vx + cos(f_angle + PI - PI/10) * 6 * zoom + side_separation_x;
//        vertex_list [4] [1] = vy + sin(f_angle + PI - PI/10) * 6 * zoom + side_separation_y;
        vertex_list [3] [0] = vx + cos(f_angle) * 8 * zoom + side_separation_x;
        vertex_list [3] [1] = vy + sin(f_angle) * 8 * zoom + side_separation_y;

        add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

        vertex_list [0] [0] = vx + cos(f_angle) * (SOTB_SIZE+2) * zoom - side_separation_x;
        vertex_list [0] [1] = vy + sin(f_angle) * (SOTB_SIZE+2) * zoom - side_separation_y;
//        vertex_list [1] [0] = vx + cos(f_angle - PI/12) * (SOTB_SIZE+7) * zoom - side_separation_x;
//        vertex_list [1] [1] = vy + sin(f_angle - PI/12) * (SOTB_SIZE+7) * zoom - side_separation_y;
        vertex_list [1] [0] = vx + cos(f_angle - PI/2 - PI/8) * (SOTB_SIZE+2) * zoom - side_separation_x;
        vertex_list [1] [1] = vy + sin(f_angle - PI/2 - PI/8) * (SOTB_SIZE+2) * zoom - side_separation_y;
        vertex_list [2] [0] = vx + cos(f_angle - PI + PI/6) * (SOTB_SIZE+4) * zoom - side_separation_x;
        vertex_list [2] [1] = vy + sin(f_angle - PI + PI/6) * (SOTB_SIZE+4) * zoom - side_separation_y;
//        vertex_list [4] [0] = vx + cos(f_angle - PI + PI/10) * 6 * zoom - side_separation_x;
//        vertex_list [4] [1] = vy + sin(f_angle - PI + PI/10) * 6 * zoom - side_separation_y;
        vertex_list [3] [0] = vx + cos(f_angle) * 8 * zoom - side_separation_x;
        vertex_list [3] [1] = vy + sin(f_angle) * 8 * zoom - side_separation_y;

        add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);


        vertex_list [0] [0] = vx + cos(f_angle) * 8 * zoom + back_separation_x;
        vertex_list [0] [1] = vy + sin(f_angle) * 8 * zoom + back_separation_y;
        vertex_list [1] [0] = vx + cos(f_angle - PI + PI/6) * (SOTB_SIZE + 2) * zoom + back_separation_x;
        vertex_list [1] [1] = vy + sin(f_angle - PI + PI/6) * (SOTB_SIZE + 2) * zoom + back_separation_y;
        vertex_list [2] [0] = vx + cos(f_angle + PI) * (SOTB_SIZE+4) * zoom + back_separation_x;
        vertex_list [2] [1] = vy + sin(f_angle + PI) * (SOTB_SIZE+4) * zoom + back_separation_y;
        vertex_list [3] [0] = vx + cos(f_angle + PI - PI/6) * (SOTB_SIZE + 2) * zoom + back_separation_x;
        vertex_list [3] [1] = vy + sin(f_angle + PI - PI/6) * (SOTB_SIZE + 2) * zoom + back_separation_y;

        add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

        vertex_list [0] [0] = vx + cos(f_angle) * 3 * zoom;
        vertex_list [0] [1] = vy + sin(f_angle) * 3 * zoom;
        vertex_list [1] [0] = vx + cos(f_angle + PI/2) * (SOTB_SIZE+0) * zoom;
        vertex_list [1] [1] = vy + sin(f_angle + PI/2) * (SOTB_SIZE+0) * zoom;
        vertex_list [2] [0] = vx + cos(f_angle + PI) * (SOTB_SIZE+4) * zoom;
        vertex_list [2] [1] = vy + sin(f_angle + PI) * (SOTB_SIZE+4) * zoom;
        vertex_list [3] [0] = vx + cos(f_angle - PI/2) * (SOTB_SIZE+0) * zoom;
        vertex_list [3] [1] = vy + sin(f_angle - PI/2) * (SOTB_SIZE+0) * zoom;

        add_poly_layer(OBJECT_UNDERLAY_LAYER, 4, proc_col [PROC_COL_OBJECT_2]);



			}
			break;

  case OBJECT_TYPE_HARVEST:
  	{
//     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;
    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);

     float outwards = 0.5;
     float outer_outwards = 0.5;

	 		float inner_point_x = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;
    float inner_point_y = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;



// harvest recycle currently 64

	  if (obj_inst != NULL) // will be NULL if this is being drawn on the design screen
			{
				if (obj_inst->last_gather_or_give > w.world_time - (HARVEST_RECYCLE_TIME + 16)) // HARVEST_RECYCLE_TIME currently 64
				{

     int line_time = w.world_time - obj_inst->last_gather_or_give;

     int circle_time = (line_time % 32);

     if (obj_inst->gather_or_give == 1)
						circle_time = 32 - circle_time;

     if (line_time < 64)
					{

      shade = circle_time;// * 2;

						float circle_size = (31 - circle_time) * 0.6;

      radial_circle(4,
																			 inner_point_x + cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]) * 7 * zoom,
																	   inner_point_y + sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]) * 7 * zoom,
																			 12, // vertices
																			 colours.packet [core->player_index] [shade],
																			 circle_size);
					}

      circle_time = ((line_time+16) % 32);

      if (obj_inst->gather_or_give == 1)
						 circle_time = 32 - circle_time;



//     if (circle_time < line_time)
					{

      shade = circle_time;// * 2;

						float circle_size = (31 - circle_time) * 0.6;

      radial_circle(4,
																			 inner_point_x + cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]) * 7 * zoom,
																	   inner_point_y + sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]) * 7 * zoom,
																			 12, // vertices
																			 colours.packet [core->player_index] [shade],
																			 circle_size);
					}

/*
     radial_blob(4,
																	inner_point_x + cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 7 * zoom,
																	inner_point_y + sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 7 * zoom, 0, 12, colours.packet [core->player_index] [shade / 2], (80 - line_time) * 0.12, 4, w.world_time * 2, w.world_time * 3);

     radial_blob(4,
																	inner_point_x + cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 7 * zoom,
																	inner_point_y + sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * 7 * zoom, 0, 12, colours.packet [core->player_index] [shade], (80 - line_time) * 0.06, 3, w.world_time * 2, w.world_time * 3);
*/
     if (line_time < 8)
				 {
      outwards += (float) line_time * 0.16;
      if (obj_inst->second_last_gather_or_give > w.world_time - 80)
       outer_outwards += 8 * 0.32;
        else
         outer_outwards += line_time * 0.32;
					}
					 else
						{
							if (line_time < 32)
						 {
		      outwards += 16 * 0.08;
        outer_outwards += 16 * 0.16;
						 }
						  else
								{
									if (line_time < 64)
									{
  		      outwards += (64 - line_time) * 0.04;
          outer_outwards += 16 * 0.16;
									}
									 else
										{
           outer_outwards += (80 - line_time) * 0.16;
										}
								}
						}

/*
     if (line_time < 16)
				 {
      outwards += (float) line_time * 0.08;
      outer_outwards = outwards;
					}
					 else
						{
							if (line_time < 32)
						 {
		      outwards += 16 * 0.08;
        outer_outwards += (float) line_time * 0.08;
						 }
						  else
								{
									if (line_time < 64)
									{
  		      outwards += (64 - line_time) * 0.04;
          outer_outwards += 32 * 0.08;
									}
									 else
										{
           outer_outwards += (80 - line_time) * 0.08;
										}
								}
						}
*/
/*
				if (obj_inst->last_gather_or_give > w.world_time - 32)//HARVEST_LINE_TIME
//				&& obj_inst->gather_or_give == 0) // gather
			 {
     outwards += (float) (32 - line_time) * 0.08;
			 }*/
			}
			}

    float outwards_xpart = cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]);
    float outwards_ypart = sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]);

// Left
   float side_xpart = cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]);
   float side_ypart = sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]);

// inner left
		  vertex_list [0] [0] = inner_point_x + side_xpart * (5+outwards) * zoom;
		  vertex_list [0] [1] = inner_point_y + side_ypart * (5+outwards) * zoom;
// outer left
		  vertex_list [1] [0] = vertex_list [0] [0] + side_xpart * (4) * zoom;
		  vertex_list [1] [1] = vertex_list [0] [1] + side_ypart * (4) * zoom;
// outer front
		  vertex_list [2] [0] = inner_point_x + (outwards_xpart * 15 * zoom) + (side_xpart * outwards * zoom);
		  vertex_list [2] [1] = inner_point_y + (outwards_ypart * 15 * zoom) + (side_ypart * outwards * zoom);
// inner front
		  vertex_list [3] [0] = vertex_list [2] [0] - (outwards_xpart * 7 * zoom);
		  vertex_list [3] [1] = vertex_list [2] [1] - (outwards_ypart * 7 * zoom);


    add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

// Left outer
// inner left
		  vertex_list [0] [0] = inner_point_x + side_xpart * (10+outer_outwards) * zoom;
		  vertex_list [0] [1] = inner_point_y + side_ypart * (10+outer_outwards) * zoom;
// outer left
		  vertex_list [1] [0] = vertex_list [0] [0] + side_xpart * (4) * zoom;
		  vertex_list [1] [1] = vertex_list [0] [1] + side_ypart * (4) * zoom;
// outer front
		  vertex_list [2] [0] = inner_point_x + (outwards_xpart * 23 * zoom) + (side_xpart * outer_outwards * zoom);
		  vertex_list [2] [1] = inner_point_y + (outwards_ypart * 23 * zoom) + (side_ypart * outer_outwards * zoom);
// inner front
		  vertex_list [3] [0] = vertex_list [2] [0] - (outwards_xpart * 7 * zoom);
		  vertex_list [3] [1] = vertex_list [2] [1] - (outwards_ypart * 7 * zoom);

    add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);


// Right
   side_xpart = cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]);
   side_ypart = sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]);

// inner right
		  vertex_list [0] [0] = inner_point_x + side_xpart * (5+outwards) * zoom;
		  vertex_list [0] [1] = inner_point_y + side_ypart * (5+outwards) * zoom;
// outer right
		  vertex_list [1] [0] = vertex_list [0] [0] + side_xpart * (4) * zoom;
		  vertex_list [1] [1] = vertex_list [0] [1] + side_ypart * (4) * zoom;
// outer front
		  vertex_list [2] [0] = inner_point_x + (outwards_xpart * 15 * zoom) + (side_xpart * outwards * zoom);
		  vertex_list [2] [1] = inner_point_y + (outwards_ypart * 15 * zoom) + (side_ypart * outwards * zoom);
// inner front
		  vertex_list [3] [0] = vertex_list [2] [0] - (outwards_xpart * 7 * zoom);
		  vertex_list [3] [1] = vertex_list [2] [1] - (outwards_ypart * 7 * zoom);

    add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

// inner right
		  vertex_list [0] [0] = inner_point_x + side_xpart * (10+outer_outwards) * zoom;
		  vertex_list [0] [1] = inner_point_y + side_ypart * (10+outer_outwards) * zoom;
// outer right
		  vertex_list [1] [0] = vertex_list [0] [0] + side_xpart * (4) * zoom;
		  vertex_list [1] [1] = vertex_list [0] [1] + side_ypart * (4) * zoom;
// outer front
		  vertex_list [2] [0] = inner_point_x + (outwards_xpart * 23 * zoom) + (side_xpart * outer_outwards * zoom);
		  vertex_list [2] [1] = inner_point_y + (outwards_ypart * 23 * zoom) + (side_ypart * outer_outwards * zoom);
// inner front
		  vertex_list [3] [0] = vertex_list [2] [0] - (outwards_xpart * 7 * zoom);
		  vertex_list [3] [1] = vertex_list [2] [1] - (outwards_ypart * 7 * zoom);

    add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);


  	}
			break; // end harvest object

/*
  case OBJECT_TYPE_HARVEST:
  	{
     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index] + object_angle_offset;
     float outwards = 0;

	  if (obj_inst != NULL // will be NULL if this is being drawn on the design screen
				&& obj_inst->last_gather_or_give > w.world_time - HARVEST_LINE_TIME
				&& obj_inst->gather_or_give == 0) // gather
			{

    int line_time = w.world_time - obj_inst->last_gather_or_give;
    outwards = (float) (HARVEST_LINE_TIME - line_time) * 0.2;

			}

			float v2x = vx - cos(f_angle) * 3 * zoom;
			float v2y = vy - sin(f_angle) * 3 * zoom;


 			vertex_list [0] [0] = v2x + cos(f_angle) * 7 * zoom;
				vertex_list [0] [1] = v2y + sin(f_angle) * 7 * zoom;
 			vertex_list [1] [0] = v2x + cos(f_angle + PI/5) * 7 * zoom;
				vertex_list [1] [1] = v2y + sin(f_angle + PI/5) * 7 * zoom;
 			vertex_list [2] [0] = v2x + cos(f_angle) * -3 * zoom;
				vertex_list [2] [1] = v2y + sin(f_angle) * -3 * zoom;
 			vertex_list [3] [0] = v2x + cos(f_angle - PI/5) * 7 * zoom;
				vertex_list [3] [1] = v2y + sin(f_angle - PI/5) * 7 * zoom;


    add_poly_layer(OBJECT_UNDERLAY_LAYER, 4, proc_col [4]);

				float outwards_x = cos(f_angle + PI) * outwards * zoom;
				float outwards_y = sin(f_angle + PI) * outwards * zoom;

 			vertex_list [0] [0] = v2x + cos(f_angle) * 4 * zoom + outwards_x;
				vertex_list [0] [1] = v2y + sin(f_angle) * 4 * zoom + outwards_y;
 			vertex_list [1] [0] = v2x + cos(f_angle + PI/2) * 6 * zoom + outwards_x;
				vertex_list [1] [1] = v2y + sin(f_angle + PI/2) * 6 * zoom + outwards_y;
 			vertex_list [2] [0] = v2x + cos(f_angle + PI - PI/10) * 5 * zoom + outwards_x;
				vertex_list [2] [1] = v2y + sin(f_angle + PI - PI/10) * 5 * zoom + outwards_y;
 			vertex_list [3] [0] = v2x + cos(f_angle + PI + PI/10) * 5 * zoom + outwards_x;
				vertex_list [3] [1] = v2y + sin(f_angle + PI + PI/10) * 5 * zoom + outwards_y;
 			vertex_list [4] [0] = v2x + cos(f_angle - PI/2) * 6 * zoom + outwards_x;
				vertex_list [4] [1] = v2y + sin(f_angle - PI/2) * 6 * zoom + outwards_y;

				add_poly_layer(OBJECT_MAIN_LAYER, 5, proc_col [6]);

				outwards_x = cos(f_angle + PI/3) * outwards * zoom;
				outwards_y = sin(f_angle + PI/3) * outwards * zoom;

 			vertex_list [0] [0] = v2x + cos(f_angle + PI/32) * 5 * zoom + outwards_x;
				vertex_list [0] [1] = v2y + sin(f_angle + PI/32) * 5 * zoom + outwards_y;
 			vertex_list [1] [0] = v2x + cos(f_angle + PI/2 - PI/19) * 6 * zoom + outwards_x;
				vertex_list [1] [1] = v2y + sin(f_angle + PI/2 - PI/19) * 6 * zoom + outwards_y;
 			vertex_list [2] [0] = v2x + cos(f_angle + PI/5) * 8 * zoom + outwards_x;
				vertex_list [2] [1] = v2y + sin(f_angle + PI/5) * 8 * zoom + outwards_y;
 			vertex_list [3] [0] = v2x + cos(f_angle + PI/64) * 12 * zoom + outwards_x;
				vertex_list [3] [1] = v2y + sin(f_angle + PI/64) * 12 * zoom + outwards_y;
				add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [6]);

				outwards_x = cos(f_angle - PI/3) * outwards * zoom;
				outwards_y = sin(f_angle - PI/3) * outwards * zoom;

 			vertex_list [0] [0] = v2x + cos(f_angle - PI/32) * 5 * zoom + outwards_x;
				vertex_list [0] [1] = v2y + sin(f_angle - PI/32) * 5 * zoom + outwards_y;
 			vertex_list [1] [0] = v2x + cos(f_angle - PI/2 + PI/19) * 6 * zoom + outwards_x;
				vertex_list [1] [1] = v2y + sin(f_angle - PI/2 + PI/19) * 6 * zoom + outwards_y;
 			vertex_list [2] [0] = v2x + cos(f_angle - PI/5) * 8 * zoom + outwards_x;
				vertex_list [2] [1] = v2y + sin(f_angle - PI/5) * 8 * zoom + outwards_y;
 			vertex_list [3] [0] = v2x + cos(f_angle - PI/64) * 12 * zoom + outwards_x;
				vertex_list [3] [1] = v2y + sin(f_angle - PI/64) * 12 * zoom + outwards_y;
				add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [6]);

  	}
			break; // end harvest object
*/
	 case OBJECT_TYPE_STORAGE:
	 	{
    draw_object_base_shape(proc_x,
																								   proc_y,
																								   angle_float,
																								   zoom,
																								   proc_shape,
																								   proc_link_index,
																								   proc_col [PROC_COL_OBJECT_BASE]);

     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index];// + object_angle_offset; // angle_offset strictly unnecessary, but can't hurt


	 		float inner_point_x = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;
    float inner_point_y = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;

    float outwards_xpart = cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]);
    float outwards_ypart = sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]);

    float left_xpart = cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]);// * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * zoom;
    float left_ypart = sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]);// * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * zoom;
    float right_xpart = cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]);// * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * zoom;
    float right_ypart = sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]);// * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * zoom;

// left
    vertex_list [0] [0] = inner_point_x + left_xpart * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * zoom;
    vertex_list [0] [1] = inner_point_y + left_ypart * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * zoom;

    vertex_list [1] [0] = inner_point_x + left_xpart * (dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 0.4) * zoom;
    vertex_list [1] [1] = inner_point_y + left_ypart * (dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 0.4) * zoom;

    vertex_list [2] [0] = vertex_list [1] [0] + outwards_xpart * 16 * zoom;
    vertex_list [2] [1] = vertex_list [1] [1] + outwards_ypart * 16 * zoom;

    vertex_list [3] [0] = vertex_list [0] [0] + outwards_xpart * 13 * zoom;
    vertex_list [3] [1] = vertex_list [0] [1] + outwards_ypart * 13 * zoom;

    add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);

// right
    vertex_list [0] [0] = inner_point_x + right_xpart * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * zoom;
    vertex_list [0] [1] = inner_point_y + right_ypart * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * zoom;

    vertex_list [1] [0] = inner_point_x + right_xpart * (dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 0.4) * zoom;
    vertex_list [1] [1] = inner_point_y + right_ypart * (dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 0.4) * zoom;

    vertex_list [2] [0] = vertex_list [1] [0] + outwards_xpart * 16 * zoom;
    vertex_list [2] [1] = vertex_list [1] [1] + outwards_ypart * 16 * zoom;

    vertex_list [3] [0] = vertex_list [0] [0] + outwards_xpart * 13 * zoom;
    vertex_list [3] [1] = vertex_list [0] [1] + outwards_ypart * 13 * zoom;

    add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);


    int storage_level;
    if (core == NULL)
	    storage_level = 0;
 			  else
 			  {
				   storage_level = (core->data_stored * 100) / core->data_storage_capacity; // data_storage_capacity should never be 0 if core has a storage object
 			  }

 			int storage_bars = (storage_level + 24) / 25;

 			if (storage_bars > 0)
				{
 			 for (i = 0; i < storage_bars; i ++)
					{

      vertex_list [0] [0] = inner_point_x + left_xpart * (dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 0.35) * zoom + outwards_xpart * (1 + i * 4) * zoom;
      vertex_list [0] [1] = inner_point_y + left_ypart * (dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * 0.35) * zoom + outwards_ypart * (1 + i * 4) * zoom;

      vertex_list [1] [0] = inner_point_x + right_xpart * (dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 0.35) * zoom + outwards_xpart * (1 + i * 4) * zoom;
      vertex_list [1] [1] = inner_point_y + right_ypart * (dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * 0.35) * zoom + outwards_ypart * (1 + i * 4) * zoom;

      vertex_list [2] [0] = vertex_list [1] [0] + outwards_xpart * 3 * zoom;
      vertex_list [2] [1] = vertex_list [1] [1] + outwards_ypart * 3 * zoom;

      vertex_list [3] [0] = vertex_list [0] [0] + outwards_xpart * 3 * zoom;
      vertex_list [3] [1] = vertex_list [0] [1] + outwards_ypart * 3 * zoom;

      int bar_storage_level = 25;

      if (i == storage_bars - 1)
						{
							bar_storage_level = (storage_level + 24) % 25;
						}

      ALLEGRO_COLOR bar_colour = al_map_rgba(120 + bar_storage_level * 4, 70 + bar_storage_level * 4, 40 + bar_storage_level * 3, bar_storage_level * 8);

      add_poly_layer(OBJECT_MAIN_LAYER, 4, bar_colour);
				 }
				}



/*







     f_angle = angle_float + dshape[proc_shape].link_object_angle [proc_link_index];// + object_angle_offset; // angle_offset strictly unnecessary, but can't hurt

  float storage_level;
  if (core == NULL)
			storage_level = 0;
 			else
 			{
					storage_level = (float) (core->data_stored * 8) / core->data_storage_capacity; // data_storage_capacity should never be 0 if core has at least 1 storage object
 			}



	 		float inner_point_x = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;
    float inner_point_y = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;

    float outwards_xpart = cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]) * zoom;
    float outwards_ypart = sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index]) * zoom;

    float left_xpart = cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * zoom;
    float left_ypart = sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [0] * zoom;
    float right_xpart = cos(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * zoom;
    float right_ypart = sin(angle_float + dshape[proc_shape].link_point_side_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_side_dist [proc_link_index] [1] * zoom;

// inner right
		  vertex_list [0] [0] = inner_point_x + right_xpart;
		  vertex_list [0] [1] = inner_point_y + right_ypart;
// inner centre
		  vertex_list [1] [0] = inner_point_x;
		  vertex_list [1] [1] = inner_point_y;
// inner left
		  vertex_list [2] [0] = inner_point_x + left_xpart;
		  vertex_list [2] [1] = inner_point_y + left_ypart;
// outer left
		  vertex_list [3] [0] = inner_point_x + outwards_xpart * 13 + cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index] - PI/2) * 3 * zoom;
		  vertex_list [3] [1] = inner_point_y + outwards_ypart * 13 + sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index] - PI/2) * 3 * zoom;
// outer right
		  vertex_list [4] [0] = inner_point_x + outwards_xpart * 13 + cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index] + PI/2) * 3 * zoom;;
		  vertex_list [4] [1] = inner_point_y + outwards_ypart * 13 + sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index] + PI/2) * 3 * zoom;;

*/
/*
// outer left
		  vertex_list [3] [0] = inner_point_x + left_xpart/4 + outwards_xpart * 13;
		  vertex_list [3] [1] = inner_point_y + left_ypart/4 + outwards_ypart * 13;
// outer right
		  vertex_list [4] [0] = inner_point_x + right_xpart/4 + outwards_xpart * 13;
		  vertex_list [4] [1] = inner_point_y + right_ypart/4 + outwards_ypart * 13;*/

//    add_poly_layer(OBJECT_MAIN_LAYER, 5, proc_col [PROC_COL_OBJECT_1]);
/*
// front left
  vertex_list [0] [0] = vx + cos(f_angle - PI/4) * 6 * zoom;
  vertex_list [0] [1] = vy + sin(f_angle - PI/4) * 6 * zoom;
// front right
  vertex_list [1] [0] = vx + cos(f_angle + PI/4) * 6 * zoom;
  vertex_list [1] [1] = vy + sin(f_angle + PI/4) * 6 * zoom;
// centre right
  vertex_list [2] [0] = vx + cos(f_angle + PI/2) * 6 * zoom;
  vertex_list [2] [1] = vy + sin(f_angle + PI/2) * 6 * zoom;
// back right
  vertex_list [3] [0] = vx + cos(f_angle + PI - PI/5) * 6 * zoom;
  vertex_list [3] [1] = vy + sin(f_angle + PI - PI/5) * 6 * zoom;
// back
//  vertex_list [3] [0] = vx + cos(f_angle + PI) * 7 * zoom;
//  vertex_list [3] [1] = vy + sin(f_angle + PI) * 7 * zoom;

// back left
  vertex_list [4] [0] = vx + cos(f_angle + PI + PI/5) * 6 * zoom;
  vertex_list [4] [1] = vy + sin(f_angle + PI + PI/5) * 6 * zoom;
// centre left
  vertex_list [5] [0] = vx + cos(f_angle - PI/2) * 6 * zoom;
  vertex_list [5] [1] = vy + sin(f_angle - PI/2) * 6 * zoom;

  add_poly_layer(OBJECT_MAIN_LAYER, 6, proc_col [PROC_COL_OBJECT_1]);*/
/*
  float save_vertex_3_x = vertex_list [3] [0];
  float save_vertex_3_y = vertex_list [3] [1];

// storage line:
   if (storage_level > 0)
			{
		  vertex_list [0] [0] = inner_point_x + outwards_xpart * 13 + cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index] - PI/2) * 2 * zoom;
		  vertex_list [0] [1] = inner_point_y + outwards_ypart * 13 + sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index] - PI/2) * 2 * zoom;

		  vertex_list [1] [0] = inner_point_x + outwards_xpart * 13 + cos(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index] + PI/2) * 2 * zoom;;
		  vertex_list [1] [1] = inner_point_y + outwards_ypart * 13 + sin(angle_float + dshape[proc_shape].link_outer_angle [proc_link_index] + PI/2) * 2 * zoom;;

				vertex_list [2] [0] = vertex_list [1] [0] + (cos(f_angle) * (storage_level) * zoom);
				vertex_list [2] [1] = vertex_list [1] [1] + (sin(f_angle) * (storage_level) * zoom);

				vertex_list [3] [0] = vertex_list [0] [0] + (cos(f_angle) * (storage_level) * zoom);
				vertex_list [3] [1] = vertex_list [0] [1] + (sin(f_angle) * (storage_level) * zoom);

    add_poly_layer(2, 4, colours.base_trans [COL_YELLOW] [4] [TRANS_MED]);

			}



// front left
    vertex_list [0] [0] = save_vertex_3_x + cos(f_angle) * storage_level * zoom;
    vertex_list [0] [1] = save_vertex_3_y + sin(f_angle) * storage_level * zoom;
// front right
    vertex_list [1] [0] = vertex_list [4] [0] + cos(f_angle) * storage_level * zoom;
    vertex_list [1] [1] = vertex_list [4] [1] + sin(f_angle) * storage_level * zoom;
// further front right
 			vertex_list [2] [0] = vertex_list [1] [0] + cos(f_angle - PI/10) * 4 * zoom;
 			vertex_list [2] [1] = vertex_list [1] [1] + sin(f_angle - PI/10) * 4 * zoom;
// further front left
 			vertex_list [3] [0] = vertex_list [0] [0] + cos(f_angle + PI/10) * 4 * zoom;
 			vertex_list [3] [1] = vertex_list [0] [1] + sin(f_angle + PI/10) * 4 * zoom;

    add_poly_layer(OBJECT_MAIN_LAYER, 4, proc_col [PROC_COL_OBJECT_1]);
*/

   }
		 break;

 } // end switch(object_type)

#undef view

}


static void draw_object_base_shape(float proc_x,
																																			float proc_y,
																																			float angle_float,
																																			float zoom,
																																			int proc_shape,
																																			int proc_link_index,
																																			ALLEGRO_COLOR proc_col)
{

	int layer = 1;

	int m = vbuf.vertex_pos_triangle;
	float x, y;

	x = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_dist [proc_link_index] [0]) * zoom;
	y = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [0]) * dshape[proc_shape].link_point_dist [proc_link_index] [0]) * zoom;
	add_tri_vertex(x, y, proc_col);
	x = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;
	y = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [1]) * dshape[proc_shape].link_point_dist [proc_link_index] [1]) * zoom;
	add_tri_vertex(x, y, proc_col);
	x = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [2]) * dshape[proc_shape].link_point_dist [proc_link_index] [2]) * zoom;
	y = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [2]) * dshape[proc_shape].link_point_dist [proc_link_index] [2]) * zoom;
	add_tri_vertex(x, y, proc_col);
	x = proc_x + (cos(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [3]) * dshape[proc_shape].link_point_dist [proc_link_index] [3]) * zoom;
	y = proc_y + (sin(angle_float + dshape[proc_shape].link_point_angle [proc_link_index] [3]) * dshape[proc_shape].link_point_dist [proc_link_index] [3]) * zoom;
	add_tri_vertex(x, y, proc_col);

	construct_triangle(layer, m, m+1, m+2);
	construct_triangle(layer, m+2, m+3, m);

}



void draw_link_shape(float child_x, float child_y,
																					al_fixed child_angle,
																					int child_shape,
																					int child_link_index,
																					float parent_x, float parent_y,
																					al_fixed parent_angle,
																					int parent_shape,
																					int parent_link_index,
																					ALLEGRO_COLOR proc_col,
																					float zoom)
{

	float parent_angle_float = fixed_to_radians(parent_angle);
	float child_angle_float = fixed_to_radians(child_angle);

	int layer = 2;

	int m = vbuf.vertex_pos_triangle;
	float x, y;

	x = parent_x + (cos(parent_angle_float + dshape[parent_shape].link_point_angle [parent_link_index] [0]) * dshape[parent_shape].link_point_dist [parent_link_index] [0]) * zoom;
	y = parent_y + (sin(parent_angle_float + dshape[parent_shape].link_point_angle [parent_link_index] [0]) * dshape[parent_shape].link_point_dist [parent_link_index] [0]) * zoom;
	add_tri_vertex(x, y, proc_col);
	x = parent_x + (cos(parent_angle_float + dshape[parent_shape].link_point_angle [parent_link_index] [1]) * dshape[parent_shape].link_point_dist [parent_link_index] [1]) * zoom;
	y = parent_y + (sin(parent_angle_float + dshape[parent_shape].link_point_angle [parent_link_index] [1]) * dshape[parent_shape].link_point_dist [parent_link_index] [1]) * zoom;
	add_tri_vertex(x, y, proc_col);
	x = parent_x + (cos(parent_angle_float + dshape[parent_shape].link_point_angle [parent_link_index] [2]) * dshape[parent_shape].link_point_dist [parent_link_index] [2]) * zoom;
	y = parent_y + (sin(parent_angle_float + dshape[parent_shape].link_point_angle [parent_link_index] [2]) * dshape[parent_shape].link_point_dist [parent_link_index] [2]) * zoom;
	add_tri_vertex(x, y, proc_col);

	x = child_x + (cos(child_angle_float + dshape[child_shape].link_point_angle [child_link_index] [0]) * dshape[child_shape].link_point_dist [child_link_index] [0]) * zoom;
	y = child_y + (sin(child_angle_float + dshape[child_shape].link_point_angle [child_link_index] [0]) * dshape[child_shape].link_point_dist [child_link_index] [0]) * zoom;
	add_tri_vertex(x, y, proc_col);
	x = child_x + (cos(child_angle_float + dshape[child_shape].link_point_angle [child_link_index] [1]) * dshape[child_shape].link_point_dist [child_link_index] [1]) * zoom;
	y = child_y + (sin(child_angle_float + dshape[child_shape].link_point_angle [child_link_index] [1]) * dshape[child_shape].link_point_dist [child_link_index] [1]) * zoom;
	add_tri_vertex(x, y, proc_col);
	x = child_x + (cos(child_angle_float + dshape[child_shape].link_point_angle [child_link_index] [2]) * dshape[child_shape].link_point_dist [child_link_index] [2]) * zoom;
	y = child_y + (sin(child_angle_float + dshape[child_shape].link_point_angle [child_link_index] [2]) * dshape[child_shape].link_point_dist [child_link_index] [2]) * zoom;
	add_tri_vertex(x, y, proc_col);

	construct_triangle(layer, m, m+1, m+2);
	construct_triangle(layer, m, m+2, m+3);
	construct_triangle(layer, m, m+3, m+5);
	construct_triangle(layer, m+3, m+4, m+5);


}


void check_vbuf(void)
{
	if (vbuf.vertex_pos_line >= VERTEX_BUFFER_TRIGGER
		||	vbuf.vertex_pos_triangle >= VERTEX_BUFFER_TRIGGER
		|| vbuf.index_pos_triangle [0] >= VERTEX_INDEX_TRIGGER
		|| vbuf.index_pos_triangle [1] >= VERTEX_INDEX_TRIGGER
		|| vbuf.index_pos_triangle [2] >= VERTEX_INDEX_TRIGGER
		|| vbuf.index_pos_triangle [3] >= VERTEX_INDEX_TRIGGER
		|| vbuf.index_pos_triangle [4] >= VERTEX_INDEX_TRIGGER
		|| vbuf.index_pos_line [0] >= VERTEX_INDEX_TRIGGER
		|| vbuf.index_pos_line [1] >= VERTEX_INDEX_TRIGGER
		|| vbuf.index_pos_line [2] >= VERTEX_INDEX_TRIGGER
		|| vbuf.index_pos_line [3] >= VERTEX_INDEX_TRIGGER
		|| vbuf.index_pos_line [4] >= VERTEX_INDEX_TRIGGER)
		draw_vbuf();

}




void draw_vbuf(void)
{
//al_hold_bitmap_drawing(1);
// fprintf(stdout, "\ndraw: vp %i ", vbuf.vertex_pos);
//fprintf(stdout, "\ndraw_vbuf");
	int i;

	for (i = 0; i < DISPLAY_LAYERS; i ++)
	{
//  fprintf(stdout, "tp[%i] %i ", i, vbuf.index_pos_triangle [i]);

		if (vbuf.index_pos_triangle [i] > 0)
   al_draw_indexed_prim(vbuf.buffer_triangle,
																							 NULL, // vertex declaration
																							 NULL, // texture
																							 vbuf.index_triangle [i],
																							 vbuf.index_pos_triangle [i],
																							 ALLEGRO_PRIM_TRIANGLE_LIST);
//  if (game.pause_soft == 0)
		vbuf.index_pos_triangle [i] = 0;


		if (vbuf.index_pos_line [i] > 0)
   al_draw_indexed_prim(vbuf.buffer_line,
																							 NULL, // vertex declaration
																							 NULL, // texture
																							 vbuf.index_line [i],
																							 vbuf.index_pos_line [i],
																							 ALLEGRO_PRIM_LINE_LIST);
// if (i == 0)
  //fprintf(stdout, "lp[%i] %i ", i, vbuf.index_pos_line [i]);

		vbuf.index_pos_line [i] = 0;

	}

	vbuf.vertex_pos_triangle = 0;
	vbuf.vertex_pos_line = 0;
//al_hold_bitmap_drawing(0);
}


static void bloom_circle(int layer, float x, float y, ALLEGRO_COLOR col_centre, ALLEGRO_COLOR col_edge, float circle_size_zoomed)
{

//fpr ("\n bc at %f,%f size %f", x, y, circle_size_zoomed);
 int vertices = 10;


 int i;
 float angle_inc = PI*2/vertices;

// col_centre = al_map_rgba(250, 140, 80, 80);
// col_edge = al_map_rgba(250, 10, 10, 0);
// col_centre = al_map_rgba(200, 220, 250, 80);
// col_edge = al_map_rgba(10, 10, 250, 0);

	int m = vbuf.vertex_pos_triangle;

	add_tri_vertex(x, y, col_centre);

	for (i = 0; i < vertices; ++i)
	{
		add_tri_vertex(x + cos(i * angle_inc) * circle_size_zoomed, y + sin(i * angle_inc) * circle_size_zoomed, col_edge);
	}

	for (i = 1; i < vertices; ++i)
		construct_triangle(layer, m, m+i, m+i+1);

	construct_triangle(layer, m, m+vertices, m+1);


}


static void bloom_long(int layer, float x, float y, float angle, float length_zoomed, ALLEGRO_COLOR col_centre_start, ALLEGRO_COLOR col_edge_start, ALLEGRO_COLOR col_edge_end, float circle_size_start_zoomed, float circle_size_end_zoomed)
{

	int vertices = 10; // number of vertices on each half-circle end

	int i, m = vbuf.vertex_pos_triangle;
	const float angle_inc = PI/vertices; // half circle

// col_centre = al_map_rgba(250, 140, 80, 80);
// col_edge = al_map_rgba(250, 10, 10, 0);
// col_centre = al_map_rgba(200, 220, 250, 80);
// col_edge = al_map_rgba(10, 10, 250, 0);

	float old_x, old_y;

	float base_angle = angle - PI/2;

	old_x = x + cos(base_angle) * circle_size_start_zoomed;
	old_y = y + sin(base_angle) * circle_size_start_zoomed;

	add_tri_vertex(x, y, col_centre_start);
	add_tri_vertex(old_x, old_y, col_edge_start);

	for (i = 1; i < vertices + 1; ++i)
	{
		old_x = x + cos(base_angle + i * angle_inc) * circle_size_start_zoomed;
		old_y = y + sin(base_angle + i * angle_inc) * circle_size_start_zoomed;

		add_tri_vertex(old_x, old_y, col_edge_start);
	}

	float end_x = x - cos(angle) * length_zoomed;
	float end_y = y - sin(angle) * length_zoomed;
	base_angle = angle + PI/2;


// do first run through the loop outside because one vertex colour is different:

	for (i = 0; i < vertices + 1; ++i)
	{
		old_x = end_x + cos(base_angle + i * angle_inc) * circle_size_end_zoomed;
		old_y = end_y + sin(base_angle + i * angle_inc) * circle_size_end_zoomed;

		add_tri_vertex(old_x, old_y, col_edge_end);
	}

	for (i = 1; i < 2*vertices + 2; ++i)
		construct_triangle(layer, m, m+i, m+i+1);

	construct_triangle(layer, m + 2*vertices + 2, m, m + 1);




}

/*
static void bloom_long(int layer, float x, float y, ALLEGRO_COLOR col_centre, ALLEGRO_COLOR col_edge, float circle_size_zoomed, float long_angle, float tail_width, float extra_length)
{


 int vertices = 24;


 int i;
 float angle_inc = PI*2/vertices;

 col_centre = al_map_rgba(250, 140, 80, 80);
 col_edge = al_map_rgba(250, 10, 10, 0);
// col_centre = al_map_rgba(200, 220, 250, 180);
// col_edge = al_map_rgba(10, 10, 250, 40);

 float vertex_angle = 0;
// float elongation = 1 + sin(vertex_angle - long_angle) * extra_length;
// float elongation = (0.4 + sin(vertex_angle - long_angle)) * extra_length;
 float
  elongation = (1 + sin(vertex_angle - long_angle) * tail_width) * extra_length;
 if (elongation < 1)
		elongation = 1;
 float vertex_dist = circle_size_zoomed * elongation;

 float old_x = x + vertex_dist;
 float old_y = y;



 for (i = 0; i < vertices; i ++)
 {

				vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = x;
   	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = y;
    vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col_centre;
    vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
    vbuf.vertex_pos_triangle++;

				vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = old_x;
   	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = old_y;
    vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col_edge;
    vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
    vbuf.vertex_pos_triangle++;

    vertex_angle = i * angle_inc;
//    elongation = 1 + sin(vertex_angle - long_angle) * extra_length;
//    elongation = (0.4 + sin(vertex_angle - long_angle)) * extra_length;
  elongation = (1 + sin(vertex_angle - long_angle) * tail_width) * extra_length;
    if (elongation < 1)
   		elongation = 1;
    vertex_dist = circle_size_zoomed * elongation;

    old_x = x + cos(vertex_angle) * vertex_dist;
    old_y = y + sin(vertex_angle) * vertex_dist;

				vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = old_x;
   	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = old_y;
    vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col_edge;
    vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
    vbuf.vertex_pos_triangle++;

 }

				vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = x;
   	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = y;
    vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col_centre;
    vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
    vbuf.vertex_pos_triangle++;

				vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = old_x;
   	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = old_y;
    vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col_edge;
    vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
    vbuf.vertex_pos_triangle++;

    vertex_angle = 0;
//    elongation = 1 + sin(vertex_angle - long_angle) * extra_length;
//    elongation = (0.4 + sin(vertex_angle - long_angle)) * extra_length;
  elongation = (1 + sin(vertex_angle - long_angle) * tail_width) * extra_length;
    if (elongation < 1)
		   elongation = 1;
    vertex_dist = circle_size_zoomed * elongation;

				vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = x + vertex_dist;;
   	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = y;
    vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col_edge;
    vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = vbuf.vertex_pos_triangle;
    vbuf.vertex_pos_triangle++;


}
*/

static void radial_circle(int layer, float x, float y, int vertices, ALLEGRO_COLOR col, float circle_size)
{

 int i;
 float angle_inc = PI*2/vertices;

 start_radial(x, y, layer, col);

 for (i = 0; i < vertices; i ++)
 {
  add_radial_vertex(i*angle_inc, circle_size);
 }

 finish_radial();

}


static void double_circle_with_bloom(int layer, float x, float y, float circle_size, int player_index, int base_shade)
{

 sancheck(layer, 1, DISPLAY_LAYERS, "double_circle_with_bloom: layer");

 bloom_circle(layer - 1, x, y, colours.bloom_centre [player_index] [base_shade], colours.bloom_edge [player_index] [base_shade], circle_size);
 draw_circle(layer - 1, x, y, circle_size, colours.packet [player_index] [base_shade / 2]);

 draw_circle(layer, x, y, circle_size * 0.8, colours.packet [player_index] [base_shade]);

}

static void draw_circle(int layer, float x, float y, float circle_size, ALLEGRO_COLOR col)
{

 circle_size *= view.zoom;

 int i;

 vertex_list [0] [0] = x;
 vertex_list [0] [1] = y;

 for (i = 0; i < 26; i ++) // remember - vertex_list has 32 elements
	{
  vertex_list [i+1] [0] = x + (cos(i * PI/12) * circle_size);
	 vertex_list [i+1] [1] = y + (sin(i * PI/12) * circle_size);
	}

 add_poly_layer(layer, i, col);

}




/*
static void radial_blob(int layer, float x, float y, float base_angle, int vertices, ALLEGRO_COLOR col, float base_size, int drand_size, int drand_seed1, int drand_seed2)
{

 int i;
 float angle_inc = PI*2/vertices;

 seed_drand(drand_seed1);
 start_radial(x, y, layer, col);

 for (i = 0; i < vertices; i ++)
 {
  add_radial_vertex(base_angle + i*angle_inc, (base_size + drand(drand_size,drand_seed2)));
 }

 finish_radial();

}
*/
/*
// vertices should be an even number
static void radial_elongated_blob_10(int layer, float x, float y, float base_angle, ALLEGRO_COLOR col, float base_size, int drand_size, int drand_seed1, int drand_seed2)
{

 seed_drand(drand_seed1);
 start_radial(x, y, layer, col);

 add_radial_vertex(base_angle, (base_size * 1.7) + drand(drand_size,drand_seed1));
 add_radial_vertex(base_angle + (PI*0.10), (base_size * 1.5) + drand(drand_size,drand_seed1));
 add_radial_vertex(base_angle + (PI*0.2), (base_size * 1.2) + drand(drand_size,drand_seed1));
 add_radial_vertex(base_angle + (PI*0.45), (base_size * 1.1) + drand(drand_size,drand_seed1));
 add_radial_vertex(base_angle + (PI*0.7), (base_size * 1.05) + drand(drand_size,drand_seed1));
 add_radial_vertex(base_angle + (PI), (base_size * 1.0) + drand(drand_size,drand_seed1));
 add_radial_vertex(base_angle + (PI*1.3), (base_size * 1.05) + drand(drand_size,drand_seed1));
 add_radial_vertex(base_angle + (PI*1.55), (base_size * 1.1) + drand(drand_size,drand_seed1));
 add_radial_vertex(base_angle + (PI*1.8), (base_size * 1.2) + drand(drand_size,drand_seed1));
 add_radial_vertex(base_angle + (PI*1.90), (base_size * 1.5) + drand(drand_size,drand_seed1));


 finish_radial();

}
*/


/*
void add_method_base_diamond(float point_x, float point_y, float f_angle, struct shape_struct* sh, int size, int vertex, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col)
{

//#define NOTCH_SEPARATION (size + 1)
#define NOTCH_SEPARATION (size)

 point_x += cos(f_angle) * 3;
 point_y += sin(f_angle) * 3;

//#define NOTCH_SEPARATION (0)

// The order of the vertices is necessary for concave vertices:
              add_outline_diamond(point_x + cos(f_angle + fixed_to_radians(sh->external_angle [EXANGLE_HIGH] [vertex])) * (sh->vertex_notch_sidewards [vertex] - NOTCH_SEPARATION),
                          point_y + sin(f_angle + fixed_to_radians(sh->external_angle [EXANGLE_HIGH] [vertex])) * (sh->vertex_notch_sidewards [vertex] - NOTCH_SEPARATION),
																										point_x,
                          point_y,
                          point_x + cos(f_angle + fixed_to_radians(sh->external_angle [EXANGLE_LOW] [vertex])) * (sh->vertex_notch_sidewards [vertex] - NOTCH_SEPARATION),
                          point_y + sin(f_angle + fixed_to_radians(sh->external_angle [EXANGLE_LOW] [vertex])) * (sh->vertex_notch_sidewards [vertex] - NOTCH_SEPARATION),
                          point_x - cos(f_angle) * (sh->vertex_notch_inwards [vertex] - NOTCH_SEPARATION),
                          point_y - sin(f_angle) * (sh->vertex_notch_inwards [vertex] - NOTCH_SEPARATION),
                          fill_col,
                          edge_col);

}
*/


/*

void add_outline_shape2(float x, float y, float float_angle, struct shape_struct* sh, ALLEGRO_COLOR line_col1, ALLEGRO_COLOR line_col2, ALLEGRO_COLOR line_col3, ALLEGRO_COLOR fill_col)
{

 int i;

 outline_buffer [outline_pos].vertex_start = outline_vertex_pos;
 outline_buffer [outline_pos].vertices = sh->vertices;
 outline_buffer [outline_pos].line_col [0] = line_col1;
 outline_buffer [outline_pos].line_col [1] = line_col2;
 outline_buffer [outline_pos].line_col [2] = line_col3;
 outline_pos ++;

 for (i = 0; i < sh->vertices; i ++)
 {
  poly_buffer [poly_pos].x = x;
  poly_buffer [poly_pos].y = y;
  poly_buffer [poly_pos].z = 0;
  poly_buffer [poly_pos].color = fill_col;
  poly_pos ++;
  poly_buffer [poly_pos].x = x + fxpart(float_angle + sh->vertex_angle_float [i], sh->vertex_dist_pixel [i]);
  poly_buffer [poly_pos].y = y + fypart(float_angle + sh->vertex_angle_float [i], sh->vertex_dist_pixel [i]);
  poly_buffer [poly_pos].z = 0;
  poly_buffer [poly_pos].color = fill_col;

  outline_vertex [outline_vertex_pos++] = poly_buffer [poly_pos].x;
  outline_vertex [outline_vertex_pos++] = poly_buffer [poly_pos].y;

  poly_pos ++;
//  outline_pos ++;
  poly_buffer [poly_pos].x = x + fxpart(float_angle + sh->vertex_angle_float [i + 1], sh->vertex_dist_pixel [i + 1]);
  poly_buffer [poly_pos].y = y + fypart(float_angle + sh->vertex_angle_float [i + 1], sh->vertex_dist_pixel [i + 1]);
  poly_buffer [poly_pos].z = 0;
  poly_buffer [poly_pos].color = fill_col;
  poly_pos ++;


 }

  outline_vertex [outline_vertex_pos++] = poly_buffer [poly_pos - 1].x;
  outline_vertex [outline_vertex_pos++] = poly_buffer [poly_pos - 1].y;


 check_buffer_sizes();


}
*/


/*
void add_scaled_outline_shape(struct shape_struct* sh, float float_angle, float x, float y, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col, float scale)
{

 int i;


 for (i = 0; i < sh->vertices; i ++)
 {
  layer_poly_buffer [2] [layer_poly_pos [2]].x = x;
  layer_poly_buffer [2] [layer_poly_pos [2]].y = y;
  layer_poly_buffer [2] [layer_poly_pos [2]].z = 0;
  layer_poly_buffer [2] [layer_poly_pos [2]].color = fill_col;
  layer_poly_pos [2] ++;
  layer_poly_buffer [2] [layer_poly_pos [2]].x = x + fxpart(float_angle + sh->vertex_angle_float [i], sh->vertex_dist_pixel [i] * scale);
  layer_poly_buffer [2] [layer_poly_pos [2]].y = y + fypart(float_angle + sh->vertex_angle_float [i], sh->vertex_dist_pixel [i] * scale);
  layer_poly_buffer [2] [layer_poly_pos [2]].z = 0;
  layer_poly_buffer [2] [layer_poly_pos [2]].color = fill_col;

  layer_line_buffer [2] [layer_line_pos [2]].x = layer_poly_buffer [2] [layer_poly_pos [2]].x;
  layer_line_buffer [2] [layer_line_pos [2]].y = layer_poly_buffer [2] [layer_poly_pos [2]].y;
  layer_line_buffer [2] [layer_line_pos [2]].z = 0;
  layer_line_buffer [2] [layer_line_pos [2]].color = edge_col;

  layer_poly_pos [2] ++;
  layer_line_pos [2] ++;

  layer_poly_buffer [2] [layer_poly_pos [2]].x = x + fxpart(float_angle + sh->vertex_angle_float [i + 1], sh->vertex_dist_pixel [i + 1] * scale);
  layer_poly_buffer [2] [layer_poly_pos [2]].y = y + fypart(float_angle + sh->vertex_angle_float [i + 1], sh->vertex_dist_pixel [i + 1] * scale);
  layer_poly_buffer [2] [layer_poly_pos [2]].z = 0;
  layer_poly_buffer [2] [layer_poly_pos [2]].color = fill_col;

  layer_line_buffer [2] [layer_line_pos [2]].x = layer_poly_buffer [2] [layer_poly_pos [2]].x;
  layer_line_buffer [2] [layer_line_pos [2]].y = layer_poly_buffer [2] [layer_poly_pos [2]].y;
  layer_line_buffer [2] [layer_line_pos [2]].z = 0;
  layer_line_buffer [2] [layer_line_pos [2]].color = edge_col;
  layer_line_pos [2] ++;

  layer_poly_pos [2] ++;


 }

//  outline_vertex [outline_vertex_pos++] = poly_buffer [poly_pos - 1].x;
//  outline_vertex [outline_vertex_pos++] = poly_buffer [poly_pos - 1].y;


 check_buffer_sizes();


}
*/

/*


void add_scaled_outline(struct shape_struct* sh, float float_angle, float x, float y, ALLEGRO_COLOR edge_col, float scale)
{

 int i;

 for (i = 0; i < sh->vertices; i ++)
 {

  layer_line_buffer [2] [layer_line_pos [2]].x = x + fxpart(float_angle + sh->vertex_angle_float [i], sh->vertex_dist_pixel [i] * scale);
  layer_line_buffer [2] [layer_line_pos [2]].y = y + fypart(float_angle + sh->vertex_angle_float [i], sh->vertex_dist_pixel [i] * scale);
  layer_line_buffer [2] [layer_line_pos [2]].z = 0;
  layer_line_buffer [2] [layer_line_pos [2]].color = edge_col;

  layer_line_pos [2] ++;

  layer_line_buffer [2] [layer_line_pos [2]].x = x + fxpart(float_angle + sh->vertex_angle_float [i + 1], sh->vertex_dist_pixel [i + 1] * scale);
  layer_line_buffer [2] [layer_line_pos [2]].y = y + fypart(float_angle + sh->vertex_angle_float [i + 1], sh->vertex_dist_pixel [i + 1] * scale);
  layer_line_buffer [2] [layer_line_pos [2]].z = 0;
  layer_line_buffer [2] [layer_line_pos [2]].color = edge_col;
  layer_line_pos [2] ++;


 }

 check_buffer_sizes();

}

*/

void add_outline_diamond_layer(int layer, float vx1, float vy1, float vx2, float vy2, float vx3, float vy3, float vx4, float vy4, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col)
{

	int m = vbuf.vertex_pos_line;
	add_line_vertex(vx1, vy1, edge_col);
	add_line_vertex(vx2, vy2, edge_col);
	add_line_vertex(vx3, vy3, edge_col);
	add_line_vertex(vx4, vy4, edge_col);
	construct_line(layer, m, m+1);
	construct_line(layer, m+1, m+2);
	construct_line(layer, m+2, m+3);
	construct_line(layer, m+3, m);

	m = vbuf.vertex_pos_triangle;

	add_tri_vertex(vx1, vy1, fill_col);
	add_tri_vertex(vx2, vy2, fill_col);
	add_tri_vertex(vx3, vy3, fill_col);
	add_tri_vertex(vx4, vy4, fill_col);

	construct_triangle(layer, m, m+1, m+2);
	construct_triangle(layer, m+2, m+3, m);

}


static void add_diamond_layer(int layer, float vx1, float vy1, float vx2, float vy2, float vx3, float vy3, float vx4, float vy4, ALLEGRO_COLOR fill_col)
{

	int m = vbuf.vertex_pos_triangle;

	add_tri_vertex(vx1, vy1, fill_col);
	add_tri_vertex(vx2, vy2, fill_col);
	add_tri_vertex(vx3, vy3, fill_col);
	add_tri_vertex(vx4, vy4, fill_col);

	construct_triangle(layer, m, m+1, m+2);
	construct_triangle(layer, m+2, m+3, m);

}

/*

 - should be able to put this back (just commented it out to avoid warnings about unused static functions)

static void add_outline_triangle_layer(int layer, float vx1, float vy1, float vx2, float vy2, float vx3, float vy3, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col)
{

	add_line(layer, vx1, vy1, vx2, vy2, edge_col);
	add_line(layer, vx2, vy2, vx3, vy3, edge_col);
	add_line(layer, vx3, vy3, vx1, vy1, edge_col);

	int m = vbuf.vertex_pos_triangle;
	add_tri_vertex(vx1, vy1, fill_col);
	add_tri_vertex(vx2, vy2, fill_col);
	add_tri_vertex(vx3, vy3, fill_col);

	construct_triangle(layer, m, m+1, m+2);

}
*/
/*
// call this after setting up the vertex_list array with an appropriate number of vertices
// currently uses vertex 0 as a source for a fan (could also walk through the vertices, which would allow more complex shapes)
static void add_outline_poly_layer(int layer, int vertices, ALLEGRO_COLOR fill_col, ALLEGRO_COLOR edge_col)
{

	int i, m = vbuf.vertex_pos_triangle, n = vbuf.vertex_pos_line;

	for (i = 0; i < vertices; ++i)
		add_line_vertex(vertex_list [i] [0], vertex_list [i] [1], edge_col);

	for (i = 0; i < vertices - 1; ++i)
		construct_line(layer, n+i, n+i+1);

	construct_line(layer, n + vertices - 1, n);

	for (i = 0; i < vertices; ++i)
		add_tri_vertex(vertex_list [i] [0], vertex_list [i] [1], fill_col);

	for (i = 1; i < vertices - 1; ++i)
		construct_triangle(layer, m, m+i, m+i+1);

 check_buffer_sizes();

}
*/

// call this after setting up the vertex_list array with an appropriate number of vertices
// currently uses vertex 0 as a source for a fan (could also walk through the vertices, which would allow more complex shapes)
static void add_poly_layer(int layer, int vertices, ALLEGRO_COLOR fill_col)
{

	int i, m = vbuf.vertex_pos_triangle;

	for (i = 0; i < vertices; ++i)
		add_tri_vertex(vertex_list[i][0], vertex_list[i][1], fill_col);

	for (i = 1; i < vertices - 1; ++i)
		construct_triangle(layer, m, m+i, m+i+1);

}

void add_tri_vertex(float x, float y, ALLEGRO_COLOR col)
{
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = x;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = y;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
	++vbuf.vertex_pos_triangle;
}

void construct_triangle(int layer, int v1, int v2, int v3)
{
	vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = v1;
	vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = v2;
	vbuf.index_triangle [layer] [vbuf.index_pos_triangle [layer]++] = v3;
}


/*
static void add_filled_rectangle(int layer, float x1, float y1, float x2, float y2, ALLEGRO_COLOR fill_col)
{
	int m = vbuf.vertex_pos_triangle;
	add_vertex(x1, y1, fill_col);
	add_vertex(x2, y1, fill_col);
	add_vertex(x2, y2, fill_col);
	add_vertex(x1, y2, fill_col);

	construct_triangle(layer, m, m+1, m+2);
	construct_triangle(layer, m+2, m+3, m);
}
*/
#define MAP_VERTICES 1000
#define MAP_W view.map_w
#define MAP_H view.map_h

#define VISION_CIRCLE_LAYER 4
// VISION_CIRCLE_LAYER should only be used for drawing the vision mask onto the map
#define MAP_DETAIL_LAYER 2
// MAP_DETAIL_LAYER must be different from VISION_CIRCLE_LAYER

/*
Can't use any of the drawing buffers except the basic line buffer
(to change this, add tests for other buffers to the end of this function)
*/
static void draw_map(void)
{

// if (view.map_visible == 0)
//  return;

 int i, j;
 int c;
 float map_base_x = view.map_x;//view.window_x - view.map_w - 50;
 float map_base_y = view.map_y;//view.window_y - view.map_h - 50;

 al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	al_draw_bitmap(vision_mask_map [MAP_MASK_BASE], map_base_x, map_base_y, 0);

 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

// al_draw_filled_rectangle(map_base_x, map_base_y, map_base_x + MAP_W, map_base_y + MAP_H, al_map_rgb(10,20,40));
// al_draw_rectangle(map_base_x, map_base_y, map_base_x + MAP_W, map_base_y + MAP_H, colours.base [COL_BLUE] [SHADE_HIGH], 1);

 if (w.world_seconds > 3599)
  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], map_base_x + MAP_W, map_base_y - scaleUI_y(FONT_BASIC,12), ALLEGRO_ALIGN_RIGHT, "%i:%.2i:%.2i", w.world_seconds / 3600, (int) (w.world_seconds / 60) % 60, w.world_seconds % 60);
   else
    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], map_base_x + MAP_W, map_base_y - scaleUI_y(FONT_BASIC,12), ALLEGRO_ALIGN_RIGHT, "%i:%.2i", (int) (w.world_seconds / 60) % 60, w.world_seconds % 60);

  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_MED], map_base_x + MAP_W, map_base_y + MAP_H + 5, ALLEGRO_ALIGN_RIGHT, "fps %i", view.fps);

// al_set_clipping_rectangle(map_base_x, map_base_y, MAP_W, MAP_H);


 float vcircle_size;
 ALLEGRO_COLOR visible = al_map_rgba(0,0,0,0);

 ALLEGRO_VERTEX map_pixel [MAP_VERTICES];

#ifdef SANITY_CHECK
 if (MAP_VERTICES < w.max_cores)
	{
		fpr("\n Error: i_display.c: draw_map(): MAP_VERTICES (%i) less than w.max_cores (%i)", MAP_VERTICES, w.max_cores);
		error_call();
	}
#endif

 i = 0;

 struct core_struct* core;

 float x;// = al_fixtof(al_fixmul(view.camera_x, view.map_proportion_x));
 float y;// = al_fixtof(al_fixmul(view.camera_y, view.map_proportion_y));
 float base_x = al_fixtof(al_fixmul(view.camera_x, view.map_proportion_x));
 float base_y = al_fixtof(al_fixmul(view.camera_y, view.map_proportion_y));
 float box_size_x = al_fixtof(view.map_proportion_x) * view.window_x_zoomed;
 float box_size_y = al_fixtof(view.map_proportion_y) * view.window_y_zoomed;
// float xa, ya;


 for (c = 0; c < w.max_cores; c ++)
 {
  core = &w.core [c];
  if (core->exists == 0)
   continue;

  float point_pos_x = map_base_x + al_fixtof(al_fixmul(core->core_position.x, view.map_proportion_x));
  float point_pos_y = map_base_y + al_fixtof(al_fixmul(core->core_position.y, view.map_proportion_y));

  int point_centre = TCOL_MAP_POINT_MED;
  int point_lr = TCOL_MAP_POINT_MIN;
  int point_ud = TCOL_MAP_POINT_MIN;
  int point_diag = -1;


//  if (core->group_members_current > 1)
//		{
//			point_ud = 1;
//   if (core->group_members_current > 4)
//				point_lr = 1;
   if (core->group_members_current > 8)
			{
				point_lr = TCOL_MAP_POINT_MED;
				point_diag = TCOL_MAP_POINT_MIN;
    if (core->group_members_current > 12)
				{
 				point_ud = TCOL_MAP_POINT_MED;
 				point_diag = TCOL_MAP_POINT_MED;
     if (core->group_members_current > 15)
					{
				  point_centre = TCOL_MAP_POINT_MAX;
					}
				}
			}
//		}


  map_pixel[i].x = point_pos_x;
  map_pixel[i].y = point_pos_y;
  map_pixel[i].z = 0;
  map_pixel[i].color = colours.team [core->player_index] [point_centre];
  i++;


//		if (point_ud)
		{
    map_pixel[i].x = point_pos_x;
    map_pixel[i].y = point_pos_y - 1;
    map_pixel[i].z = 0;
    map_pixel[i].color = colours.team [core->player_index] [point_ud];
    i++;
    map_pixel[i].x = point_pos_x;
    map_pixel[i].y = point_pos_y + 1;
    map_pixel[i].z = 0;
    map_pixel[i].color = colours.team [core->player_index] [point_ud];
    i++;
		}

//		if (point_lr)
		{
    map_pixel[i].x = point_pos_x - 1;
    map_pixel[i].y = point_pos_y;
    map_pixel[i].z = 0;
    map_pixel[i].color = colours.team [core->player_index] [point_lr];
    i++;
    map_pixel[i].x = point_pos_x + 1;
    map_pixel[i].y = point_pos_y;
    map_pixel[i].z = 0;
    map_pixel[i].color = colours.team [core->player_index] [point_lr];
    i++;
		}

		if (point_diag != -1)
		{
    map_pixel[i].x = point_pos_x - 1;
    map_pixel[i].y = point_pos_y - 1;
    map_pixel[i].z = 0;
    map_pixel[i].color = colours.team [core->player_index] [point_diag];
    i++;
    map_pixel[i].x = point_pos_x - 1;
    map_pixel[i].y = point_pos_y + 1;
    map_pixel[i].z = 0;
    map_pixel[i].color = colours.team [core->player_index] [point_diag];
    i++;
    map_pixel[i].x = point_pos_x + 1;
    map_pixel[i].y = point_pos_y - 1;
    map_pixel[i].z = 0;
    map_pixel[i].color = colours.team [core->player_index] [point_diag];
    i++;
    map_pixel[i].x = point_pos_x + 1;
    map_pixel[i].y = point_pos_y + 1;
    map_pixel[i].z = 0;
    map_pixel[i].color = colours.team [core->player_index] [point_diag];
    i++;
		}


  if (i == MAP_VERTICES)
  {
   al_draw_prim(map_pixel, NULL, NULL, 0, MAP_VERTICES, ALLEGRO_PRIM_POINT_LIST); // May need to put back "-1" after MAP_VERTICES
   i = 0;
  }

// prepare to draw holes in the vision mask around the player's cores:
//  (these will actually be drawn below)
  if (core->player_index == game.user_player_index)
		{

 		vcircle_size = core->scan_range_float * al_fixtof(view.map_proportion_x);
 		add_diagonal_octagon(VISION_CIRCLE_LAYER,
																								  point_pos_x - map_base_x,
																								  point_pos_y - map_base_y,
																								  vcircle_size,
																								  visible);
  }


 }

/*
 for (c = 0; c < w.max_cores; c ++)
 {
  core = &w.core [c];
  if (core->exists == 0)
   continue;

  map_pixel[i].x = map_base_x + al_fixtof(al_fixmul(core->core_position.x, view.map_proportion_x));
  map_pixel[i].y = map_base_y + al_fixtof(al_fixmul(core->core_position.y, view.map_proportion_y));
  map_pixel[i].z = 0;
  map_pixel[i].color = colours.team [core->player_index] [TCOL_MAP_POINT];

  if (core->group_members_current > 1)

  if (i == MAP_VERTICES)
  {
   al_draw_prim(map_pixel, NULL, NULL, 0, MAP_VERTICES, ALLEGRO_PRIM_POINT_LIST); // May need to put back "-1" after MAP_VERTICES
   i = 0;
  }

// prepare to draw holes in the vision mask around the player's cores:
//  (these will actually be drawn below)
  if (core->player_index == game.user_player_index)
		{

 		vcircle_size = core->scan_range_float * al_fixtof(view.map_proportion_x);
 		add_filled_rectangle(VISION_CIRCLE_LAYER,
																								map_pixel[i].x - map_base_x	- vcircle_size,
																								map_pixel[i].y - map_base_y	- vcircle_size,
																								map_pixel[i].x - map_base_x	+ vcircle_size,
																								map_pixel[i].y - map_base_y	+ vcircle_size,
																								visible);

  }


  i ++;

 }
*/


// i retains its value here so it can be used for assembling the map selection list:

 float last_x, last_y;

// now draw map selection:
 for (c = 0; c < w.max_cores; c ++)
 {
  core = &w.core [c];
  if (core->exists == 0
			|| core->selected == -1)
   continue;

  x = map_base_x + al_fixtof(al_fixmul(core->core_position.x, view.map_proportion_x));
  y = map_base_y + al_fixtof(al_fixmul(core->core_position.y, view.map_proportion_y));


  for (j = -1; j < 2; j ++)
  {
   map_pixel[i].x = x - 2;
   map_pixel[i].y = y + j;
   map_pixel[i].z = 0;
   map_pixel[i].color = colours.base [COL_GREY] [SHADE_MAX];

   i ++;

   if (i == MAP_VERTICES)
   {
    al_draw_prim(map_pixel, NULL, NULL, 0, MAP_VERTICES, ALLEGRO_PRIM_POINT_LIST); // May need to put back "-1" after MAP_VERTICES
    i = 0;
   }

   map_pixel[i].x = x + 2;
   map_pixel[i].y = y + j;
   map_pixel[i].z = 0;
   map_pixel[i].color = colours.base [COL_GREY] [SHADE_MAX];

   i ++;

   if (i == MAP_VERTICES)
   {
    al_draw_prim(map_pixel, NULL, NULL, 0, MAP_VERTICES, ALLEGRO_PRIM_POINT_LIST); // May need to put back "-1" after MAP_VERTICES
    i = 0;
   }

  }

  if (core->player_index != game.user_player_index)
			continue;

// only player-controlled cores get past here

		last_x = x;
		last_y = y;

  for (j = 0; j < COMMAND_QUEUE; j++)
		{
			if (core->command_queue [j].type == COM_NONE)
				break;


			switch (core->command_queue [j].type)
			{
			 case COM_LOCATION:
		  case COM_DATA_WELL:
			 	add_line(MAP_DETAIL_LAYER,
														last_x,
														last_y,
														map_base_x + al_fixtof(view.map_proportion_x * core->command_queue[j].x),
														map_base_y + al_fixtof(view.map_proportion_y * core->command_queue[j].y),
														colours.base_trans [COL_GREY] [SHADE_MED] [TRANS_MED]);

					last_x = map_base_x + al_fixtof(view.map_proportion_x * core->command_queue[j].x);
					last_y = map_base_y + al_fixtof(view.map_proportion_y * core->command_queue[j].y);

     map_pixel[i].x = last_x;
     map_pixel[i].y = last_y;
     map_pixel[i].z = 0;
     map_pixel[i].color = colours.base [COL_GREY] [SHADE_MAX];

     i ++;

					break;
			 case COM_TARGET:
			 	if (w.core[core->command_queue[j].target_core].exists <= 0
						|| core->command_queue[j].target_core_created != w.core[core->command_queue[j].target_core].created_timestamp
						|| !check_proc_visible_to_user(w.core[core->command_queue[j].target_core].process_index))
						break;
			 	add_line(MAP_DETAIL_LAYER,
														last_x,
														last_y,
														map_base_x + al_fixtof(al_fixmul(view.map_proportion_x, w.core[core->command_queue[j].target_core].core_position.x)),
														map_base_y + al_fixtof(al_fixmul(view.map_proportion_y, w.core[core->command_queue[j].target_core].core_position.y)),
														colours.base_trans [COL_RED] [SHADE_MED] [TRANS_MED]);

														last_x = map_base_x + al_fixtof(al_fixmul(view.map_proportion_x, w.core[core->command_queue[j].target_core].core_position.x));
														last_y = map_base_y + al_fixtof(al_fixmul(view.map_proportion_y, w.core[core->command_queue[j].target_core].core_position.y));
     map_pixel[i].x = last_x;
     map_pixel[i].y = last_y;
     map_pixel[i].z = 0;
     map_pixel[i].color = colours.base [COL_GREY] [SHADE_MAX];

     i ++;

					break;
			 case COM_FRIEND:
			 	if (w.core[core->command_queue[j].target_core].exists <= 0
						|| core->command_queue[j].target_core_created != w.core[core->command_queue[j].target_core].created_timestamp)
						break;
			 	add_line(MAP_DETAIL_LAYER,
														last_x,
														last_y,
														map_base_x + al_fixtof(al_fixmul(view.map_proportion_x, w.core[core->command_queue[j].target_core].core_position.x)),
														map_base_y + al_fixtof(al_fixmul(view.map_proportion_y, w.core[core->command_queue[j].target_core].core_position.y)),
														colours.base_trans [COL_CYAN] [SHADE_MED] [TRANS_MED]);

														last_x = map_base_x + al_fixtof(al_fixmul(view.map_proportion_x, w.core[core->command_queue[j].target_core].core_position.x));
														last_y = map_base_y + al_fixtof(al_fixmul(view.map_proportion_y, w.core[core->command_queue[j].target_core].core_position.y));
     map_pixel[i].x = last_x;
     map_pixel[i].y = last_y;
     map_pixel[i].z = 0;
     map_pixel[i].color = colours.base [COL_GREY] [SHADE_MAX];

     i ++;

					break;

			}
		}

 }



// draw_vbuf();


 if (i > 0)
 {
   al_draw_prim(map_pixel, NULL, NULL, 0, i, ALLEGRO_PRIM_POINT_LIST);
 }

// now draw the vision mask:

 al_set_target_bitmap(vision_mask_map [MAP_MASK_DRAWN]);
 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

// al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
#ifndef RECORDING_VIDEO_2
 if (game.vision_mask
 	&& !mission_state.reveal_player1)
 	al_draw_bitmap(vision_mask_map [MAP_MASK_OPAQUE], 0, 0, 0);
//  al_clear_to_color(colours.black);
   else
   	al_draw_bitmap(vision_mask_map [MAP_MASK_TRANS], 0, 0, 0);
//    al_clear_to_color(al_map_rgba(0,0,0,120));
#else
   	al_draw_bitmap(vision_mask_map [MAP_MASK_TRANS], 0, 0, 0);
#endif

 al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);

// this bit is code from draw_vbuf(), but just for a single layer:
		if (vbuf.index_pos_triangle [VISION_CIRCLE_LAYER] > 0)
   al_draw_indexed_prim(vbuf.buffer_triangle,
																							 NULL, // vertex declaration
																							 NULL, // texture
																							 vbuf.index_triangle [VISION_CIRCLE_LAYER],
																							 vbuf.index_pos_triangle [VISION_CIRCLE_LAYER],
																							 ALLEGRO_PRIM_TRIANGLE_LIST);
		vbuf.index_pos_triangle [VISION_CIRCLE_LAYER] = 0;


 al_set_target_bitmap(al_get_backbuffer(display));
// al_set_clipping_rectangle(0, 0, panel[PANEL_MAIN].w, panel[PANEL_MAIN].h);
// al_set_clipping_rectangle(0, 0, inter.display_w, inter.display_h);

 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

//#ifdef DRAW_MAP_VISION_MASK
 al_draw_bitmap(vision_mask_map [MAP_MASK_DRAWN], map_base_x, map_base_y, 0);
//#endif


// add data wells:
 for (j = 0; j < w.data_wells; j ++)
	{
		 		add_orthogonal_hexagon(MAP_DETAIL_LAYER, map_base_x + al_fixtof(al_fixmul(view.map_proportion_x, w.data_well[j].position.x)),
																												map_base_y + al_fixtof(al_fixmul(view.map_proportion_y, w.data_well[j].position.y)),
																												3,
																												colours.base_trans [COL_YELLOW] [SHADE_HIGH] [TRANS_MED]);

	}

// add under attack markers
  if (view.under_attack_marker_last_time >= w.world_time - UNDER_ATTACK_MARKER_DURATION)
		{
			for (i = 0; i < UNDER_ATTACK_MARKERS; i ++)
			{
				if (view.under_attack_marker [i].time_placed_world >= w.world_time - UNDER_ATTACK_MARKER_DURATION)
				{
					int marker_shade = 31 - ((w.world_time - view.under_attack_marker [i].time_placed_world) % 32);

		 		add_orthogonal_hexagon(MAP_DETAIL_LAYER, map_base_x + al_fixtof(al_fixmul(view.map_proportion_x, view.under_attack_marker[i].position.x)),
																												map_base_y + al_fixtof(al_fixmul(view.map_proportion_y, view.under_attack_marker[i].position.y)),
																												16 + marker_shade / 4,
																												colours.packet [0] [marker_shade]);

				}
			}
		}

		if (vbuf.index_pos_triangle [MAP_DETAIL_LAYER] > 0)
   al_draw_indexed_prim(vbuf.buffer_triangle,
																							 NULL, // vertex declaration
																							 NULL, // texture
																							 vbuf.index_triangle [MAP_DETAIL_LAYER],
																							 vbuf.index_pos_triangle [MAP_DETAIL_LAYER],
																							 ALLEGRO_PRIM_TRIANGLE_LIST);
		vbuf.index_pos_triangle [MAP_DETAIL_LAYER] = 0;



	if (vbuf.index_pos_line [MAP_DETAIL_LAYER] > 0)
	{
   al_draw_indexed_prim(vbuf.buffer_line,
																							 NULL, // vertex declaration
																							 NULL, // texture
																							 vbuf.index_line [MAP_DETAIL_LAYER],
																							 vbuf.index_pos_line [MAP_DETAIL_LAYER],
																							 ALLEGRO_PRIM_LINE_LIST);

		vbuf.index_pos_line [MAP_DETAIL_LAYER] = 0;
	}

// finally draw the box indicating what's on the screen:
 al_draw_rectangle(map_base_x + base_x - box_size_x/2, map_base_y + base_y - box_size_y/2, map_base_x + base_x + box_size_x/2, map_base_y + base_y + box_size_y/2,
   colours.base [COL_BLUE] [SHADE_HIGH], 1);

 if (mission_state.reveal_player1)
  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_GREY] [SHADE_HIGH], map_base_x, map_base_y + MAP_H + 3, ALLEGRO_ALIGN_LEFT, "Opponent revealed");

/*
// currently only the basic line buffer is checked here:
 if (line_pos > 0)
 {
  al_draw_prim(line_buffer, NULL, NULL, 0, line_pos, ALLEGRO_PRIM_LINE_LIST);
  line_pos = 0;
 }*/


}


#define DRAND_BITS 8
#define DRAND_SIZE (1<<DRAND_BITS)
#define DRAND_MASK (DRAND_SIZE-1)

int drand_list [DRAND_SIZE];
int drand_pos;

// drand functions provide a simple pseudorandom number generator that produces reliable results when called in the same conditions.
// it allows display functions to use random numbers while stopping animations during pauses.
void init_drand(void)
{
	int i;

 srand(1);

	for (i = 0; i < DRAND_SIZE; i ++)
	{
		drand_list [i] = (int) ((int) rand() + ((int) rand() << 16)); // assumes that rand returns 16 bits
	}

	drand_pos = 0;

}


static void seed_drand(int seed)
{

// drand_seed = seed;

	drand_pos = seed; // don't need to bounds-check as drand() always does so.

}

static int drand(int mod, int drand_pos_change)
{


// drand_seed = drand_seed * 1103515245 + 12345;
// return (unsigned int)(drand_seed / 65536) % rand_max;


	drand_pos += drand_pos_change;
	return drand_list [drand_pos & DRAND_MASK] % mod;
}



/*

static void reset_fan_index(void)
{
 fan_index[0].start_position = -1;
 fan_index_pos = 0;
 fan_buffer_pos = 0;
}

static int start_fan(float x, float y, ALLEGRO_COLOR col)
{
 if (fan_index_pos >= FAN_INDEX_TRIGGER
  || fan_buffer_pos >= FAN_BUFFER_TRIGGER)
  {
  return 0; // just fail if there are too many fans
  }
 fan_index[fan_index_pos].start_position = fan_buffer_pos;
 fan_buffer[fan_buffer_pos].x = x;
 fan_buffer[fan_buffer_pos].y = y;
 fan_buffer[fan_buffer_pos].z = 0;
 fan_buffer[fan_buffer_pos].color = col;
 fan_buffer_pos ++;
 return 1;
}

static void add_fan_vertex(float x, float y, ALLEGRO_COLOR col)
{
 fan_buffer[fan_buffer_pos].x = x;
 fan_buffer[fan_buffer_pos].y = y;
 fan_buffer[fan_buffer_pos].z = 0;
 fan_buffer[fan_buffer_pos].color = col;
 fan_buffer_pos ++;
}

static void finish_fan(void)
{
 add_fan_vertex(fan_buffer[fan_index[fan_index_pos].start_position + 1].x,
                fan_buffer[fan_index[fan_index_pos].start_position + 1].y,
                fan_buffer[fan_index[fan_index_pos].start_position + 1].color);
 fan_index[fan_index_pos].vertices = fan_buffer_pos - fan_index[fan_index_pos].start_position;

 fan_index_pos ++;
 fan_index[fan_index_pos].start_position = -1;

}

// finishes a fan without closing it by linking the last vertex to the first
static void finish_fan_open(void)
{
 fan_index[fan_index_pos].vertices = fan_buffer_pos - fan_index[fan_index_pos].start_position;

 fan_index_pos ++;
 fan_index[fan_index_pos].start_position = -1;

}


#define FAN_VERTICES_MAX 64
static void draw_fans(void)
{

 int fan_vertex_list [FAN_VERTICES_MAX];
 int i, j;

 i = 0;

 while(fan_index[i].start_position != -1)
 {

  for (j = 0; j < fan_index[i].vertices; j ++)
  {
   fan_vertex_list [j] = fan_index[i].start_position + j;
  }

  al_draw_indexed_prim(fan_buffer, NULL, NULL, fan_vertex_list, fan_index[i].vertices, ALLEGRO_PRIM_TRIANGLE_FAN);
  i++;
 };

 reset_fan_index();

}
*/

static void draw_data_well_exclusion_zones(void)
{

 int i;
// float exclusion_zone_pixels;

 for (i = 0; i < DATA_WELLS; i ++)
	{

		if (w.data_well[i].active == 0)
			continue;

  float x = al_fixtof(w.data_well[i].position.x - view.camera_x) * view.zoom;
  float y = al_fixtof(w.data_well[i].position.y - view.camera_y) * view.zoom;
  x += view.window_x_unzoomed / 2;
  y += view.window_y_unzoomed / 2;

  float exclusion_zone_pixels_zoomed = al_fixtof(w.data_well[i].static_build_exclusion) * view.zoom;

  if (x < 0 - exclusion_zone_pixels_zoomed
		 || x > view.window_x_unzoomed + exclusion_zone_pixels_zoomed
   || y < 0 - exclusion_zone_pixels_zoomed
   || y > view.window_y_unzoomed + exclusion_zone_pixels_zoomed)
    continue;

    select_arrows(12, x, y,
																		game.total_time * 0.002,// + (PI/5), // angle
																		exclusion_zone_pixels_zoomed, // radius
																		12, //float out_dist,
																	 PI/2+PI/8, //float side_angle,
																		16, //float side_dist,
																		colours.base_trans [COL_ORANGE] [SHADE_MAX] [TRANS_FAINT]);// [TRANS_THICK]); //ALLEGRO_COLOR arrow_col)

//  add_orthogonal_hexagon(3, x, y, exclusion_zone_pixels_zoomed, colours.base_trans [COL_ORANGE] [SHADE_HIGH] [TRANS_FAINT]);


	}

}


void draw_mouse_cursor(void)
{

	if (!ex_control.mouse_on_display)
		return;
//return;
 float x = ex_control.mouse_x_pixels;
 float y = ex_control.mouse_y_pixels;

 int i;
 int mcol = COL_GREY;
/*
add_outline_diamond_layer(0,
																			x, y,
																			x + 12, y + 11,
																			x + 8, y + 19,
																			x, y + 18,
																			colours.base [COL_GREY] [SHADE_MAX],
																			colours.base_trans [COL_GREY] [SHADE_MIN] [TRANS_FAINT]);
*/

 switch(ex_control.mouse_cursor_type)
 {

	 case MOUSE_CURSOR_BASIC:
	 default: // map etc just use basic

   add_diamond_layer(0,
																			  x - 2, y - 4,
																			  x + 15, y + 11,
																			  x + 10, y + 21,
																			  x - 2, y + 21,
																			  colours.base_trans [COL_GREY] [SHADE_MIN] [TRANS_FAINT]);

   add_diamond_layer(0,
																			  x, y,
																			  x + 12, y + 11,
																			  x + 8, y + 19,
																			  x, y + 19,
																			  colours.base [COL_GREY] [SHADE_MAX]);
    break;
	 case MOUSE_CURSOR_PROCESS_ENEMY:
	 	mcol = COL_RED;
	 case MOUSE_CURSOR_PROCESS_FRIENDLY:
   {
   	float x1, y1;
   	int extra_size_bit = 3 - ((inter.running_time / 8) % 4);
   	int extra_size_bit2 = (extra_size_bit + 1) % 4;
   	int shade;

   	for (i = 0; i < 4; i ++)
				{
   	 x1 = 5 + i * 5;
   	 shade = SHADE_MED;
   	 if (i == extra_size_bit)
					{
   	  y1 = 4;
   	  shade = SHADE_MAX;
					}
					  else
							{
   	    if (i == extra_size_bit2)
								{
   	     y1 = 3 - ((inter.running_time / 2) % 4);
   	     shade = SHADE_HIGH;
								}
   	      else
   							 y1 = 0;
							}

	 	 add_orthogonal_rect(0,
																							 x - x1 + 1, y - y1 - 5,
																							 x - x1 - 4, y + y1 + 5,
																			     colours.base_trans [COL_GREY] [SHADE_MIN] [TRANS_FAINT]);
	 	 add_orthogonal_rect(0,
																							 x + x1 - 1, y - y1 - 5,
																							 x + x1 + 4, y + y1 + 5,
																			     colours.base_trans [COL_GREY] [SHADE_MIN] [TRANS_FAINT]);

	 	 add_orthogonal_rect(0,
																							 x - x1, y - y1 - 3,
																							 x - x1 - 2, y + y1 + 3,
  																			   colours.base [mcol] [shade]);
	 	 add_orthogonal_rect(0,
																							 x + x1, y - y1 - 3,
																							 x + x1 + 2, y + y1 + 3,
  																			   colours.base [mcol] [shade]);

				}
   }

    break;
   case MOUSE_CURSOR_RESIZE:
   add_diamond_layer(0,
																			  x - 4, y,
																			  x - 3, y - 8,
																			  x - 15, y,
																			  x - 3, y + 8,
																			  colours.base_trans [COL_GREY] [SHADE_MIN] [TRANS_FAINT]);
   add_diamond_layer(0,
																			  x + 4, y,
																			  x + 3, y - 8,
																			  x + 15, y,
																			  x + 3, y + 8,
																			  colours.base_trans [COL_GREY] [SHADE_MIN] [TRANS_FAINT]);

   add_diamond_layer(0,
																			  x - 6, y,
																			  x - 4, y - 6,
																			  x - 13, y,
																			  x - 4, y + 6,
																			  colours.base [COL_GREY] [SHADE_MAX]);

   add_diamond_layer(0,
																			  x + 6, y,
																			  x + 4, y - 6,
																			  x + 13, y,
																			  x + 4, y + 6,
																			  colours.base [COL_GREY] [SHADE_MAX]);
			break;
	 case MOUSE_CURSOR_TEXT:
	 	y += 2;
   add_diamond_layer(0,
																			  x - 1, y - 8,
																			  x + 2, y - 8,
																			  x + 2, y + 8,
																			  x - 1, y + 8,
																			  colours.base_trans [COL_GREY] [SHADE_MIN] [TRANS_FAINT]);
   add_diamond_layer(0,
																			  x - 4, y - 9,
																			  x + 5, y - 9,
																			  x + 5, y - 6,
																			  x - 4, y - 6,
																			  colours.base_trans [COL_GREY] [SHADE_MIN] [TRANS_FAINT]);
   add_diamond_layer(0,
																			  x - 4, y + 9,
																			  x + 5, y + 9,
																			  x + 5, y + 6,
																			  x - 4, y + 6,
																			  colours.base_trans [COL_GREY] [SHADE_MIN] [TRANS_FAINT]);

   add_diamond_layer(0,
																			  x, y - 7,
																			  x + 1, y - 7,
																			  x + 1, y + 7,
																			  x, y + 7,
																			  colours.base [COL_GREY] [SHADE_HIGH]);
   add_diamond_layer(0,
																			  x - 3, y - 8,
																			  x + 4, y - 8,
																			  x + 4, y - 7,
																			  x - 3, y - 7,
																			  colours.base [COL_GREY] [SHADE_HIGH]);
   add_diamond_layer(0,
																			  x - 3, y + 8,
																			  x + 4, y + 8,
																			  x + 4, y + 7,
																			  x - 3, y + 7,
																			  colours.base [COL_GREY] [SHADE_HIGH]);
    break;


   case MOUSE_CURSOR_DESIGN_DRAG_OBJECT_COPY:
    add_diamond_layer(0,
																			   x + 12 - 5, y + 2 - 1,
																			   x + 12 - 5, y + 2 + 1,
																			   x + 12 + 5, y + 2 + 1,
																			   x + 12 + 5, y + 2 - 1,
//																			  colours.base [COL_GREY] [SHADE_MAX]);
 																			  colours.base [COL_GREY] [SHADE_HIGH]);
    add_diamond_layer(0,
																			   x + 12 - 1, y + 2 - 5,
																			   x + 12 - 1, y + 2 + 5,
																			   x + 12 + 1, y + 2 + 5,
																			   x + 12 + 1, y + 2 - 5,
//																			  colours.base [COL_GREY] [SHADE_MAX]);
 																			  colours.base [COL_GREY] [SHADE_HIGH]);

// fall-through:
		 case MOUSE_CURSOR_DESIGN_DRAG_OBJECT:

   add_diamond_layer(0,
																			  x - 12, y - 12,
																			  x - 12, y + 12,
																			  x + 12, y + 12,
																			  x + 12, y - 12,
//																			  colours.base [COL_GREY] [SHADE_MAX]);
																			  colours.base_trans [COL_GREY] [SHADE_HIGH] [TRANS_FAINT]);

   add_diamond_layer(0,
																			  x, y,
																			  x + 12, y + 11,
																			  x + 8, y + 19,
																			  x, y + 19,
																			  colours.base [COL_GREY] [SHADE_MAX]);

   add_diamond_layer(0,
																			  x - 2, y - 4,
																			  x + 15, y + 11,
																			  x + 10, y + 21,
																			  x - 2, y + 21,
																			  colours.base_trans [COL_GREY] [SHADE_MIN] [TRANS_FAINT]);

   add_diamond_layer(0,
																			  x, y,
																			  x + 12, y + 11,
																			  x + 8, y + 19,
																			  x, y + 19,
																			  colours.base [COL_GREY] [SHADE_MAX]);


			 break;

 }

   i = 0;

   al_draw_indexed_prim(vbuf.buffer_triangle,
																							 NULL, // vertex declaration
																							 NULL, // texture
																							 vbuf.index_triangle [i],
																							 vbuf.index_pos_triangle [i],
																							 ALLEGRO_PRIM_TRIANGLE_LIST);
		vbuf.index_pos_triangle [i] = 0;
/*
   al_draw_indexed_prim(vbuf.buffer_line,
																							 NULL, // vertex declaration
																							 NULL, // texture
																							 vbuf.index_line [i],
																							 vbuf.index_pos_line [i],
																							 ALLEGRO_PRIM_LINE_LIST);

		vbuf.index_pos_line [i] = 0;
*/

	vbuf.vertex_pos_triangle = 0;
//	vbuf.vertex_pos_line = 0;

 ex_control.mouse_cursor_type = MOUSE_CURSOR_BASIC; // this will be updated as needed

// al_draw_line(x, y, x + 7, y, colours.base [COL_GREY] [SHADE_MAX], 2);
// al_draw_line(x, y, x, y + 7, colours.base [COL_GREY] [SHADE_MAX], 2);

}



//static void vision_block_check(struct block_struct* bl, int base_pos);//, int* subblock_pos);
//static void vision_block_check_corner(struct block_struct* bl, int base_pos_x, int base_pos_y);



static void draw_text_bubble(float bubble_x, float bubble_y, int bubble_time, int bubble_col, int bubble_text_length, char* bubble_text, int draw_triangle)
{

//* make sure these are legible!

				int bubble_shade;
				float bubble_size_reduce;
				bubble_shade = bubble_time;
				bubble_shade = 16;
				float adjusted_bubble_x = bubble_x - 20;
			 if (bubble_time < 16)
				{
					bubble_shade = 31 - (bubble_time);
					bubble_size_reduce = (bubble_shade - 16) * -0.2;
					adjusted_bubble_x += bubble_size_reduce;
				}
				 else
					{
			   if (bubble_time > BUBBLE_TOTAL_TIME - 16)
						{
					  bubble_shade = BUBBLE_TOTAL_TIME - bubble_time;
  					bubble_size_reduce = (16 - bubble_shade) * 0.3;
  					adjusted_bubble_x += bubble_size_reduce;
						}
						 else
								bubble_size_reduce = 0;
					}

				if (bubble_shade < 0)
						bubble_shade = 0;



 add_menu_button(adjusted_bubble_x - (10 - bubble_size_reduce),
																	bubble_y - scaleUI_y(FONT_SQUARE,(10 - bubble_size_reduce)),
																	adjusted_bubble_x + (10) + bubble_text_length * font[FONT_SQUARE].width,
																	bubble_y + scaleUI_y(FONT_SQUARE,(20 - bubble_size_reduce)),
//																	colours.packet [pr->player_index] [bubble_shade],
																	colours.packet [bubble_col] [bubble_shade],
																	3, 8);
if (draw_triangle)
{
	int _m = vbuf.vertex_pos_triangle;
	add_tri_vertex(adjusted_bubble_x, bubble_y + scaleUI_y(FONT_SQUARE,22), colours.packet [bubble_col] [bubble_shade]);
	add_tri_vertex(adjusted_bubble_x + 19, bubble_y + scaleUI_y(FONT_SQUARE,22), colours.packet [bubble_col] [bubble_shade]);
	add_tri_vertex(adjusted_bubble_x + 19, bubble_y + 52, colours.packet [bubble_col] [bubble_shade]);
	construct_triangle(4, _m, _m+1, _m+2);
}

				int text_shade = bubble_shade * 2;
				if (text_shade > 31)
 				al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], (int) adjusted_bubble_x, (int) bubble_y, ALLEGRO_ALIGN_LEFT, "%s", bubble_text);
//					text_shade = 31;
 else
				al_draw_textf(font[FONT_SQUARE].fnt, colours.packet [bubble_col] [text_shade], (int) adjusted_bubble_x, (int) bubble_y, ALLEGRO_ALIGN_LEFT, "%s", bubble_text);


}




// this function does the fog-of-war effect for the display routines
// it ignores cores that are too far out of range
// it doesn't affect the w.vision_area array and doesn't affect gameplay, so it doesn't have to be run if there's no fog-of-war.
static void vision_check_for_display(void)
{

	int c;
	int base_min_x, base_max_x, base_min_y, base_max_y;
	int min_x, max_x, min_y, max_y;
//	int clipped_left, clipped_right, clipped_top, clipped_bottom;
	int i,j;

	struct core_struct* core;

	int block_min_x = (al_fixtoi(view.camera_x) - (view.window_x_unzoomed / (view.zoom * 2))) / BLOCK_SIZE_PIXELS - 2;
	if (block_min_x < 1)
		block_min_x = 1;
	int block_min_y = (al_fixtoi(view.camera_y) - (view.window_y_unzoomed / (view.zoom * 2))) / BLOCK_SIZE_PIXELS - 2;
	if (block_min_y < 1)
		block_min_y = 1;
	int block_max_x = block_min_x + ((view.window_x_unzoomed / BLOCK_SIZE_PIXELS) / view.zoom) + 7;
	if (block_max_x >= w.blocks.x - 1)
		block_max_x = w.blocks.x - 1;
	int block_max_y = block_min_y + ((view.window_y_unzoomed / BLOCK_SIZE_PIXELS) / view.zoom) + 6;
	if (block_max_y >= w.blocks.y - 1)
		block_max_y = w.blocks.y - 1;

	for (c = w.player[game.user_player_index].core_index_start; c < w.player[game.user_player_index].core_index_end; c ++)
	{
		if (w.core[c].exists <= 0
			&& w.core[c].destroyed_timestamp < w.world_time - DEALLOCATE_COUNTER) // this means deallocating cores are still seen for a little while
			continue; // TO DO: vision areas generated by recently destroyed procs should shrink for a while
		core = &w.core[c];

//#define VISION_SIZE_BLOCKS 6

		base_min_x = w.proc[core->process_index].block_position.x - SCAN_RANGE_BASE_BLOCKS - 1;
		base_min_y = w.proc[core->process_index].block_position.y - SCAN_RANGE_BASE_BLOCKS - 1;
		base_max_x = w.proc[core->process_index].block_position.x + SCAN_RANGE_BASE_BLOCKS + 2;
		base_max_y = w.proc[core->process_index].block_position.y + SCAN_RANGE_BASE_BLOCKS + 2;

		if (base_min_x > block_max_x
			|| base_min_y > block_max_y
			|| base_max_x < block_min_x
			|| base_max_y < block_min_y)
				continue; // core's vision range is not visible on screen

		if (base_min_x < block_min_x)
			min_x = block_min_x;
			 else
					min_x = base_min_x;
	 if (base_min_y < block_min_y)
			min_y = block_min_y;
			 else
					min_y = base_min_y;
		if (base_max_x > block_max_x)
			max_x = block_max_x;
			 else
  			max_x = base_max_x;
		if (base_max_y > block_max_y)
			max_y = block_max_y;
			 else
  			max_y = base_max_y;

		int core_x = al_fixtoi(core->core_position.x);
		int core_y = al_fixtoi(core->core_position.y);

		int vision_range = al_fixtoi(core->scan_range_fixed) + 64;

		if (w.core[c].exists <= 0)
		{
			vision_range *= DEALLOCATE_COUNTER - (w.world_time - w.core[c].destroyed_timestamp);
			vision_range /= DEALLOCATE_COUNTER;
		}
		 else
			{
				if (w.core[c].created_timestamp > w.world_time - 16)
				{
			  vision_range *= (w.world_time - w.core[c].created_timestamp);
			  vision_range /= 16;
				}
			}

		for (i = min_x; i < max_x;	i ++)
		{
		 for (j = min_y; j < max_y;	j ++)
		 {
//		 	struct block_struct* bl = &w.block[i][j];

		 	if (w.vision_block[i][j].clear_time == w.world_time)
					continue;

    int dist_x = abs(core_x - (i * BLOCK_SIZE_PIXELS + (j&1) * (BLOCK_SIZE_PIXELS / 2)));
    int dist_y = abs(core_y - j * BLOCK_SIZE_PIXELS);

    int dist;
/*    if (dist_x > dist_y)
					dist = dist_x;
				  else
				   dist = dist_y;*/

// approximate octagonal distance:
    if (dist_x > dist_y)
				{
					dist = 0.41 * dist_y + 0.94124 * dist_x;
				}
				  else
						{
  					dist = 0.41 * dist_x + 0.94124 * dist_y;
						}



				if (w.vision_block[i][j].proximity_time != w.world_time)
				{
					w.vision_block[i][j].proximity_time = w.world_time;
					w.vision_block[i][j].proximity = 100000;
				}
/*
				if (dist < BLOCK_SIZE_PIXELS * 2)
				{
//		   w.block[i][j].vision_block_clear_time = w.world_time;
//		   w.block[i][j].vision_block_proximity_time = w.world_time;
//					continue;
				}*/

				if (dist > vision_range)//BLOCK_SIZE_PIXELS * 5)
				{
//					if (bl->vision_block_proximity_time != w.world_time)
//					{
//		    w.block[i][j].vision_block_clear_time = w.world_time;
//					 bl->vision_block_proximity_time = w.world_time;
//					 bl->vision_block_proximity = 100000;
//					}
					continue;
				}

//				dist -= (vision_range - BLOCK_SIZE_PIXELS * (SCAN_RANGE_BASE_BLOCKS / 2));//BLOCK_SIZE_PIXELS * 3;
//				dist -= (vision_range);// - BLOCK_SIZE_PIXELS * (SCAN_RANGE_BASE_BLOCKS));//BLOCK_SIZE_PIXELS * 3;
				dist -= (vision_range) - BLOCK_SIZE_PIXELS * 2;//(SCAN_RANGE_BASE_BLOCKS * 2);//BLOCK_SIZE_PIXELS * 3;

				if (dist < 0)
					dist = 0;

				w.vision_block[i][j].proximity_time = w.world_time;
				if (w.vision_block[i][j].proximity > dist)
					w.vision_block[i][j].proximity = dist;

		 }
		}

/*
  int clear_min_x = base_min_x;
  if (clear_min_x < 0)
			clear_min_x = 0;
  int clear_min_y = base_min_y;
  if (clear_min_y < 0)
			clear_min_y = 0;
  int clear_max_x = base_max_x;
  if (clear_max_x > w.blocks.x)
			clear_max_x = w.blocks.x;
  int clear_max_y = base_max_y;
  if (clear_max_y > w.blocks.y)
			clear_max_y = w.blocks.y;

// clear out the centre:
		for (j = clear_min_y + 2; j < clear_max_y - 2;	j ++)
//		for (j = min_y; j < max_y;	j ++)
		{
		 for (i = clear_min_x + 2; i < clear_max_x - 2;	i ++)
//		 for (i = min_x; i < max_x;	i ++)
		 {
		   w.block[i][j].vision_block_clear_time = w.world_time;
		 }
		}

		int x_offset = 0;

// now set the outside so it's at a great distance (and appears black but with the appropriate edges) unless something else can see it:
//  (okay to do e.g. min_x - 1 because min_x's minimum value is 1)
		for (j = min_y - 1; j < max_y + 1;	j ++)
		{
			set_outer_edge_vision_block(min_x - 1, j);
			set_outer_edge_vision_block(max_x, j);
			set_outer_edge_vision_block(max_x + 1, j);
		}
		for (i = min_x; i < max_x;	i ++)
		{
			set_outer_edge_vision_block(i, min_y - 2);
			set_outer_edge_vision_block(i, min_y - 1);
			set_outer_edge_vision_block(i, max_y);
			set_outer_edge_vision_block(i, max_y + 1);
//			set_outer_edge_vision_block(i, max_y);
		}


//		int dist;
		int core_x = al_fixtoi(core->core_position.x);
		int core_y = al_fixtoi(core->core_position.y);

// left side


  int column;

  if (min_x <= base_min_x + 1)
		{
   column = min_x + 1;
 		for (j = min_y + 1; j < max_y - 1;	j ++)
	 	{
	 		x_offset = (BLOCK_SIZE_PIXELS / 2) * !(j & 1);
				vision_block_check(&w.block[column][j],
																							(column * BLOCK_SIZE_PIXELS) + x_offset - core_x);
	 	}

   if (min_x == base_min_x)
		 {
  		for (j = min_y + 1; j < max_y - 1;	j ++)
	 	 {
 	 		x_offset = (BLOCK_SIZE_PIXELS / 2) * !(j & 1);
 				vision_block_check(&w.block[min_x][j],
																							 (min_x * BLOCK_SIZE_PIXELS) + x_offset - core_x);
	 	 }
		 }
		}

// right side

  if (max_x >= base_max_x - 2)
		{
   column = max_x - 2;
 		for (j = min_y + 1; j < max_y - 1;	j ++)
	 	{
	 		x_offset = (BLOCK_SIZE_PIXELS / 2) * !(j & 1);
				vision_block_check(&w.block[column][j],
																						 (column * BLOCK_SIZE_PIXELS) + x_offset - core_x);
	 	}

   column = max_x - 1;
	 	if (max_x == base_max_x)
		 {
  		for (j = min_y + 1; j < max_y - 1;	j ++)
	 	 {
 	 		x_offset = (BLOCK_SIZE_PIXELS / 2) * !(j & 1);
 				vision_block_check(&w.block[column][j],
																						 (column * BLOCK_SIZE_PIXELS) + x_offset - core_x);
	 	 }
		 }
		}

// upper side


  int row = min_y + 1;

  if (min_y <= base_min_y + 1)
		{
 		for (i = min_x + 1; i < max_x - 1;	i ++)
	 	{
				vision_block_check(&w.block[i][row],
																							(row * BLOCK_SIZE_PIXELS) - core_y);
	 	}
//   row = min_y + 1;
	  if (min_y == base_min_y)
 		{
  		for (i = min_x + 1; i < max_x - 1;	i ++)
	  	{
			 	vision_block_check(&w.block[i][min_y],
																							(min_y * BLOCK_SIZE_PIXELS) - core_y);
	 	 }
		 }
		}

// lower side


  row = max_y - 2;
  if (max_y >= base_max_y - 2)
		{
 		for (i = min_x + 1; i < max_x - 1;	i ++)
	 	{
				vision_block_check(&w.block[i][row],
																							(row * BLOCK_SIZE_PIXELS) - core_y);
	 	}
   row = max_y - 1;
   if (max_y == base_max_y)
		 {
  		for (i = min_x + 1; i < max_x - 1;	i ++)
	 	 {
 				vision_block_check(&w.block[i][row],
																							(row * BLOCK_SIZE_PIXELS) - core_y);
	 	 }
		 }
		}

// corners
		if (min_x <= base_min_x + 1)
	 {
		 if (min_y <= base_min_y + 1)
    vision_block_check_corner(&w.block[min_x+1][min_y+1], ((min_x+1) * BLOCK_SIZE_PIXELS) - core_x, ((min_y+1) * BLOCK_SIZE_PIXELS) - core_y);
		 if (max_y >= base_max_y - 1)
    vision_block_check_corner(&w.block[min_x+1][max_y-2], ((min_x+1) * BLOCK_SIZE_PIXELS) - core_x, ((max_y-2) * BLOCK_SIZE_PIXELS) - core_y);

			if (min_x == base_min_x)
		 {
			 if (min_y == base_min_y)
     vision_block_check_corner(&w.block[min_x][min_y], (min_x * BLOCK_SIZE_PIXELS) - core_x, (min_y * BLOCK_SIZE_PIXELS) - core_y);
			 if (max_y == base_max_y)
     vision_block_check_corner(&w.block[min_x][max_y-1], (min_x * BLOCK_SIZE_PIXELS) - core_x, ((max_y-1) * BLOCK_SIZE_PIXELS) - core_y);
		 }
	 }

		if (max_x >= base_max_x - 1)
	 {
		 if (min_y <= base_min_y + 1)
    vision_block_check_corner(&w.block[max_x-2][min_y+1], ((max_x-2) * BLOCK_SIZE_PIXELS) - core_x, ((min_y+1) * BLOCK_SIZE_PIXELS) - core_y);
		 if (max_y >= base_max_y - 1)
    vision_block_check_corner(&w.block[max_x-2][max_y-2], ((max_x-2) * BLOCK_SIZE_PIXELS) - core_x, ((max_y-2) * BLOCK_SIZE_PIXELS) - core_y);

			if (max_x == base_max_x)
		 {
			 if (min_y == base_min_y)
     vision_block_check_corner(&w.block[max_x-1][min_y], ((max_x-1) * BLOCK_SIZE_PIXELS) - core_x, (min_y * BLOCK_SIZE_PIXELS) - core_y);
			 if (max_y == base_max_y)
     vision_block_check_corner(&w.block[max_x-1][max_y-1], ((max_x-1) * BLOCK_SIZE_PIXELS) - core_x, ((max_y-1) * BLOCK_SIZE_PIXELS) - core_y);
		 }
	 }
*/


	}









/*
	int c;
	int base_min_x, base_max_x, base_min_y, base_max_y;
	int min_x, max_x, min_y, max_y;
//	int clipped_left, clipped_right, clipped_top, clipped_bottom;
	int i,j;

	struct core_struct* core;

	int block_min_x = (al_fixtoi(view.camera_x) - (view.window_x_unzoomed / (view.zoom * 2))) / BLOCK_SIZE_PIXELS - 2;
	if (block_min_x < 1)
		block_min_x = 1;
	int block_min_y = (al_fixtoi(view.camera_y) - (view.window_y_unzoomed / (view.zoom * 2))) / BLOCK_SIZE_PIXELS - 2;
	if (block_min_y < 1)
		block_min_y = 1;
	int block_max_x = block_min_x + ((view.window_x_unzoomed / BLOCK_SIZE_PIXELS) / view.zoom) + 6;
	if (block_max_x >= w.blocks.x - 1)
		block_max_x = w.blocks.x - 1;
	int block_max_y = block_min_y + ((view.window_y_unzoomed / BLOCK_SIZE_PIXELS) / view.zoom) + 6;
	if (block_max_y >= w.blocks.y - 1)
		block_max_y = w.blocks.y - 1;

	for (c = w.player[game.user_player_index].core_index_start; c < w.player[game.user_player_index].core_index_end; c ++)
	{
		if (w.core[c].exists <= 0
			&& w.core[c].destroyed_timestamp < w.world_time - DEALLOCATE_COUNTER) // this means deallocating cores are still seen for a little while
			continue; // TO DO: vision areas generated by recently destroyed procs should shrink for a while
		core = &w.core[c];

#define VISION_SIZE_BLOCKS 6

		base_min_x = w.proc[core->process_index].block_position.x - VISION_SIZE_BLOCKS;
		base_min_y = w.proc[core->process_index].block_position.y - VISION_SIZE_BLOCKS;
		base_max_x = w.proc[core->process_index].block_position.x + VISION_SIZE_BLOCKS + 1;
		base_max_y = w.proc[core->process_index].block_position.y + VISION_SIZE_BLOCKS + 1;

		if (base_min_x > block_max_x
			|| base_min_y > block_max_y
			|| base_max_x < block_min_x
			|| base_max_y < block_min_y)
				continue; // core's vision range is not visible on screen

		if (base_min_x < block_min_x)
			min_x = block_min_x;
			 else
					min_x = base_min_x;
	 if (base_min_y < block_min_y)
			min_y = block_min_y;
			 else
					min_y = base_min_y;
		if (base_max_x > block_max_x)
			max_x = block_max_x;
			 else
  			max_x = base_max_x;
		if (base_max_y > block_max_y)
			max_y = block_max_y;
			 else
  			max_y = base_max_y;

  int clear_min_x = base_min_x;
  if (clear_min_x < 0)
			clear_min_x = 0;
  int clear_min_y = base_min_y;
  if (clear_min_y < 0)
			clear_min_y = 0;
  int clear_max_x = base_max_x;
  if (clear_max_x > w.blocks.x)
			clear_max_x = w.blocks.x;
  int clear_max_y = base_max_y;
  if (clear_max_y > w.blocks.y)
			clear_max_y = w.blocks.y;

// clear out the centre:
		for (j = clear_min_y + 2; j < clear_max_y - 2;	j ++)
//		for (j = min_y; j < max_y;	j ++)
		{
		 for (i = clear_min_x + 2; i < clear_max_x - 2;	i ++)
//		 for (i = min_x; i < max_x;	i ++)
		 {
		   w.block[i][j].vision_block_clear_time = w.world_time;
		 }
		}

// now set the outside so it's at a great distance (and appears black but with the appropriate edges) unless something else can see it:
//  (okay to do e.g. min_x - 1 because min_x's minimum value is 1)
		for (j = min_y - 1; j < max_y + 1;	j ++)
		{
			set_outer_edge_vision_block(min_x - 1, j);
			set_outer_edge_vision_block(max_x, j);
		}
		for (i = min_x; i < max_x;	i ++)
		{
			set_outer_edge_vision_block(i, min_y - 1);
			set_outer_edge_vision_block(i, max_y);
		}


//		int dist;
		int core_x = al_fixtoi(core->core_position.x);
		int core_y = al_fixtoi(core->core_position.y);

// left side


  int column;

  if (min_x <= base_min_x + 1)
		{
   column = min_x + 1;
 		for (j = min_y + 1; j < max_y - 1;	j ++)
	 	{
				vision_block_check(&w.block[column][j],
																							(column * BLOCK_SIZE_PIXELS) - core_x,
																							w.block[column][j].vision_block_x);
	 	}

   if (min_x == base_min_x)
		 {
  		for (j = min_y + 1; j < max_y - 1;	j ++)
	 	 {
 				vision_block_check(&w.block[min_x][j],
																							 (min_x * BLOCK_SIZE_PIXELS) - core_x,
 																							w.block[column][j].vision_block_x);
	 	 }
		 }
		}

// right side

  if (max_x >= base_max_x - 2)
		{
   column = max_x - 2;
 		for (j = min_y + 1; j < max_y - 1;	j ++)
	 	{
				vision_block_check(&w.block[column][j],
																						 (column * BLOCK_SIZE_PIXELS) - core_x,
																							w.block[column][j].vision_block_x);
	 	}

   column = max_x - 1;
	 	if (max_x == base_max_x)
		 {
  		for (j = min_y + 1; j < max_y - 1;	j ++)
	 	 {
 				vision_block_check(&w.block[column][j],
																						 (column * BLOCK_SIZE_PIXELS) - core_x,
																							w.block[column][j].vision_block_x);
	 	 }
		 }
		}

// upper side


  int row = min_y + 1;

  if (min_y <= base_min_y + 1)
		{
 		for (i = min_x + 1; i < max_x - 1;	i ++)
	 	{
				vision_block_check(&w.block[i][row],
																							(row * BLOCK_SIZE_PIXELS) - core_y,
																							w.block[i][row].vision_block_y);
	 	}
//   row = min_y + 1;
	  if (min_y == base_min_y)
 		{
  		for (i = min_x + 1; i < max_x - 1;	i ++)
	  	{
			 	vision_block_check(&w.block[i][min_y],
																							(min_y * BLOCK_SIZE_PIXELS) - core_y,
																							w.block[i][row].vision_block_y);
	 	 }
		 }
		}

// lower side


  row = max_y - 2;
  if (max_y >= base_max_y - 2)
		{
 		for (i = min_x + 1; i < max_x - 1;	i ++)
	 	{
				vision_block_check(&w.block[i][row],
																							(row * BLOCK_SIZE_PIXELS) - core_y,
																							w.block[i][row].vision_block_y);
	 	}
   row = max_y - 1;
   if (max_y == base_max_y)
		 {
  		for (i = min_x + 1; i < max_x - 1;	i ++)
	 	 {
 				vision_block_check(&w.block[i][row],
																							(row * BLOCK_SIZE_PIXELS) - core_y,
																							w.block[i][row].vision_block_y);
	 	 }
		 }
		}

// corners
		if (min_x <= base_min_x + 1)
	 {
		 if (min_y <= base_min_y + 1)
    vision_block_check_corner(&w.block[min_x+1][min_y+1], ((min_x+1) * BLOCK_SIZE_PIXELS) - core_x, ((min_y+1) * BLOCK_SIZE_PIXELS) - core_y);
		 if (max_y >= base_max_y - 1)
    vision_block_check_corner(&w.block[min_x+1][max_y-2], ((min_x+1) * BLOCK_SIZE_PIXELS) - core_x, ((max_y-2) * BLOCK_SIZE_PIXELS) - core_y);

			if (min_x == base_min_x)
		 {
			 if (min_y == base_min_y)
     vision_block_check_corner(&w.block[min_x][min_y], (min_x * BLOCK_SIZE_PIXELS) - core_x, (min_y * BLOCK_SIZE_PIXELS) - core_y);
			 if (max_y == base_max_y)
     vision_block_check_corner(&w.block[min_x][max_y-1], (min_x * BLOCK_SIZE_PIXELS) - core_x, ((max_y-1) * BLOCK_SIZE_PIXELS) - core_y);
		 }
	 }

		if (max_x >= base_max_x - 1)
	 {
		 if (min_y <= base_min_y + 1)
    vision_block_check_corner(&w.block[max_x-2][min_y+1], ((max_x-2) * BLOCK_SIZE_PIXELS) - core_x, ((min_y+1) * BLOCK_SIZE_PIXELS) - core_y);
		 if (max_y >= base_max_y - 1)
    vision_block_check_corner(&w.block[max_x-2][max_y-2], ((max_x-2) * BLOCK_SIZE_PIXELS) - core_x, ((max_y-2) * BLOCK_SIZE_PIXELS) - core_y);

			if (max_x == base_max_x)
		 {
			 if (min_y == base_min_y)
     vision_block_check_corner(&w.block[max_x-1][min_y], ((max_x-1) * BLOCK_SIZE_PIXELS) - core_x, (min_y * BLOCK_SIZE_PIXELS) - core_y);
			 if (max_y == base_max_y)
     vision_block_check_corner(&w.block[max_x-1][max_y-1], ((max_x-1) * BLOCK_SIZE_PIXELS) - core_x, ((max_y-1) * BLOCK_SIZE_PIXELS) - core_y);
		 }
	 }




















	}
*/
}










static void draw_data_well(int i, int j, struct backblock_struct* backbl,
																				float top_left_corner_x [BACKBLOCK_LAYERS],
                    float top_left_corner_y [BACKBLOCK_LAYERS],
                    int data_well_type,
                    float overlay_alpha)
{


							int special_i, special_j; // versions of i and j adjusted for the actual location of the data well, which may not be this block

							int k;
							float bx2, by2;

							if (backbl->backblock_type == BACKBLOCK_DATA_WELL_EDGE)
							{
								special_i = w.data_well[backbl->backblock_value].block_position.x;
								special_j = w.data_well[backbl->backblock_value].block_position.y;
							}
							 else
								{
									special_i = i;
									special_j = j;
								}

							float specific_zoom = view.zoom * w.backblock_parallax [2];

							float bx3 [2];
							float by3 [2];
							float base_part_zoom [2];

							base_part_zoom [0] = view.zoom * w.backblock_parallax [2];;
							base_part_zoom [1] = view.zoom * w.backblock_parallax [3];;

       bx3 [0] = top_left_corner_x [2] + (BLOCK_SIZE_PIXELS * base_part_zoom [0]) * (special_i) + (BLOCK_SIZE_PIXELS/2) * base_part_zoom [0]; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
       by3 [0] = top_left_corner_y [2] + (BLOCK_SIZE_PIXELS * base_part_zoom [0]) * (special_j) + (BLOCK_SIZE_PIXELS/2) * base_part_zoom [0]; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
       bx3 [1] = top_left_corner_x [3] + (BLOCK_SIZE_PIXELS * base_part_zoom [1]) * (special_i) + (BLOCK_SIZE_PIXELS/2) * base_part_zoom [1]; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
       by3 [1] = top_left_corner_y [3] + (BLOCK_SIZE_PIXELS * base_part_zoom [1]) * (special_j) + (BLOCK_SIZE_PIXELS/2) * base_part_zoom [1]; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
//       bx2 = (((i * BLOCK_SIZE_PIXELS) - camera_offset_x) + (BLOCK_SIZE_PIXELS/2)) * view.zoom;
//       by2 = (((j * BLOCK_SIZE_PIXELS) - camera_offset_y) + (BLOCK_SIZE_PIXELS/2)) * view.zoom;

//    fprintf(stdout, "\n [tlc %f,%f] [bx,by %i,%i] [bx2,by2 %f,%f]", top_left_corner_x, top_left_corner_y, bx,by, bx2, by2);


       float well_size = 20;
       seed_drand(special_i+special_j);


       switch(data_well_type)
       {

							 case -1: // this is drawn on top of the vision mask
								 add_orthogonal_hexagon(4,
																																top_left_corner_x [0] + (BLOCK_SIZE_PIXELS * view.zoom * w.backblock_parallax [0]) * (special_i) + (BLOCK_SIZE_PIXELS/2) * view.zoom * w.backblock_parallax [0],
																																top_left_corner_y [0] + (BLOCK_SIZE_PIXELS * view.zoom * w.backblock_parallax [0]) * (special_j) + (BLOCK_SIZE_PIXELS/2) * view.zoom * w.backblock_parallax [0],
																																								24,// - overlay_alpha * 0.02,
																																								al_map_rgb(overlay_alpha * 0.5, overlay_alpha * 0.35, overlay_alpha * 0.15));
																																								//al_map_rgba(200, 160, 100, overlay_alpha));

								 add_orthogonal_hexagon(4,
																																top_left_corner_x [0] + (BLOCK_SIZE_PIXELS * view.zoom * w.backblock_parallax [0]) * (special_i) + (BLOCK_SIZE_PIXELS/2) * view.zoom * w.backblock_parallax [0],
																																top_left_corner_y [0] + (BLOCK_SIZE_PIXELS * view.zoom * w.backblock_parallax [0]) * (special_j) + (BLOCK_SIZE_PIXELS/2) * view.zoom * w.backblock_parallax [0],
																																								20,// - overlay_alpha * 0.02,
																																								colours.black);

								 break;


							 case AREA_BLUE:
       	case AREA_TUTORIAL:
								{
                            for (k = 0; k < 6; k++)
                            {
                                float base_dist = drand(40, special_i) + 44;
                                well_size = base_dist + 142 + drand(40, special_i);

                                float bit_dist = 0.15;//0.05 + (drand(100, 1) * 0.008);
                                float centre_dist = 42 + drand(30, 1);

                                int bit_layer = 1;//(k+backbl->backblock_value) & 1;

                                specific_zoom = base_part_zoom [bit_layer];
                                bx2 = bx3 [bit_layer];
                                by2 = by3 [bit_layer];

#define BASE_WELL_ANGLE_2 0
//(PI/6)

                                float left_xpart = cos(BASE_WELL_ANGLE_2 + (PI/3)*k + bit_dist) * specific_zoom;
                                float left_ypart = sin(BASE_WELL_ANGLE_2 + (PI/3)*k + bit_dist) * specific_zoom;
                                float right_xpart = cos(BASE_WELL_ANGLE_2 + (PI/3)*(k+1) - bit_dist) * specific_zoom;
                                float right_ypart = sin(BASE_WELL_ANGLE_2 + (PI/3)*(k+1) - bit_dist) * specific_zoom;
                                vertex_list[0][0] = bx2 + left_xpart * centre_dist;//* well_size;
                                vertex_list[0][1] = by2 + left_ypart * centre_dist;//* well_size;
                                vertex_list[1][0] = bx2 + right_xpart * centre_dist; //well_size;
                                vertex_list[1][1] = by2 + right_ypart * centre_dist; //well_size;
                                vertex_list[2][0] = vertex_list[1][0] + cos(BASE_WELL_ANGLE_2 + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[2][1] = vertex_list[1][1] + sin(BASE_WELL_ANGLE_2 + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[3][0] = bx2 + cos(BASE_WELL_ANGLE_2 + (PI/3)*(k+1) - 0.45) * (well_size + 96) * specific_zoom;
                                vertex_list[3][1] = by2 + sin(BASE_WELL_ANGLE_2 + (PI/3)*(k+1) - 0.45) * (well_size + 96) * specific_zoom;
                                vertex_list[4][0] = bx2 + cos(BASE_WELL_ANGLE_2 + (PI/3)*k + 0.45) * (well_size + 96) * specific_zoom;
                                vertex_list[4][1] = by2 + sin(BASE_WELL_ANGLE_2 + (PI/3)*k + 0.45) * (well_size + 96) * specific_zoom;
                                vertex_list[5][0] = vertex_list[0][0] + cos(BASE_WELL_ANGLE_2 + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[5][1] = vertex_list[0][1] + sin(BASE_WELL_ANGLE_2 + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;

                                add_poly_layer(bit_layer ^ 1, 6, colours.data_well_hexes [bit_layer ^ 1]);//base [COL_BLUE] [SHADE_MED]);//data_well_hexes);


                            }


                            well_size = 40;
                            specific_zoom = view.zoom * w.backblock_parallax [1];

                            bx2 = top_left_corner_x [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);


                            for (k = 0; k < 6; k++)
                            {

                                float base_dist = drand(30, special_i) + 24;
                                well_size = base_dist + 22;

#define BASE_WELL_ANGLE (PI/6)

                                float left_xpart = cos(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float left_ypart = sin(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float right_xpart = cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                float right_ypart = sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                vertex_list[0][0] = bx2 + left_xpart * 32;//* well_size;
                                vertex_list[0][1] = by2 + left_ypart * 32;//* well_size;
                                vertex_list[1][0] = bx2 + right_xpart * 32; //well_size;
                                vertex_list[1][1] = by2 + right_ypart * 32; //well_size;
                                vertex_list[2][0] = vertex_list[1][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[2][1] = vertex_list[1][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[3][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[3][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[4][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[4][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[5][0] = vertex_list[0][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[5][1] = vertex_list[0][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;

                                add_poly_layer(1, 6, colours.data_well_hexes [2]);


                            }


                            int transfer_colour_adjust;

                            if (w.world_time - w.data_well[backbl->backblock_value].last_transferred < 32)
                             transfer_colour_adjust = 32 - (w.world_time - w.data_well[backbl->backblock_value].last_transferred);
                              else
                               transfer_colour_adjust = 0;

                            if (w.data_well[backbl->backblock_value].data > 0)
                            {

                                specific_zoom = view.zoom;

                                bx2 = top_left_corner_x [0] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                                by2 = top_left_corner_y [0] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

                                add_orthogonal_hexagon(2, bx2, by2, (30 * w.data_well[backbl->backblock_value].data * view.zoom) / w.data_well[backbl->backblock_value].data_max, map_rgba(220 + transfer_colour_adjust, 200 - transfer_colour_adjust, 50 - transfer_colour_adjust, 180 + transfer_colour_adjust));
                            }

                            specific_zoom = view.zoom * w.backblock_parallax [1];

                            bx2 = top_left_corner_x [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

                            float base_well_ring_angle = PI/6 + (w.world_time * w.data_well[backbl->backblock_value].spin_rate);
#define WELL_RING_RADIUS (110)
                            ALLEGRO_COLOR reserve_colour;

                            reserve_colour = al_map_rgba(180,
                            100,
                            30,
                            140);

                            for (k = 0; k < DATA_WELL_RESERVES; k++)
                            {
                                if (w.data_well[backbl->backblock_value].reserve_data [k] == 0)
                                    continue;
                                int l;
                                float reserve_square_arc = 0.13;
                                float reserve_square_length = 36;
                                float reserve_data_proportion = w.data_well[backbl->backblock_value].reserve_data [k] * 0.002;
                                reserve_square_length *= reserve_data_proportion;
                                if (reserve_data_proportion < 1)
                                 reserve_square_arc *= reserve_data_proportion;
                                for (l = 0; l < w.data_well[backbl->backblock_value].reserve_squares; l ++)
                                {
                                    float square_angle = base_well_ring_angle + ((PI*2) / w.data_well[backbl->backblock_value].reserve_squares) * l;
                                    add_diamond_layer(1,
                                    bx2 + cos(square_angle - reserve_square_arc) * WELL_RING_RADIUS*specific_zoom,
                                    by2 + sin(square_angle - reserve_square_arc) * WELL_RING_RADIUS*specific_zoom,
                                    bx2 + cos(square_angle + reserve_square_arc) * WELL_RING_RADIUS*specific_zoom,
                                    by2 + sin(square_angle + reserve_square_arc) * WELL_RING_RADIUS*specific_zoom,
                                    bx2 + cos(square_angle + reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*specific_zoom,
                                    by2 + sin(square_angle + reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*specific_zoom,
                                    bx2 + cos(square_angle - reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*specific_zoom,
                                    by2 + sin(square_angle - reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*specific_zoom,
                                    reserve_colour);
                                }
                                base_well_ring_angle += PI / w.data_well[backbl->backblock_value].reserve_squares;
                            }

								}
        break; // end AREA_BLUE/AREA_TUTORIAL

default:
							 case AREA_GREEN:
								{



                            well_size = 140;
                            specific_zoom = view.zoom * w.backblock_parallax [3];

                            bx2 = top_left_corner_x [3] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [3] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);


                            for (k = 0; k < 6; k++)
                            {

                                float base_dist = drand(30, special_i) + 44;
                                well_size = base_dist + 122;
#undef BASE_WELL_ANGLE
#define BASE_WELL_ANGLE (PI/6)

                                float left_xpart = cos(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float left_ypart = sin(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float right_xpart = cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                float right_ypart = sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                vertex_list[0][0] = bx2 + left_xpart * 32;//* well_size;
                                vertex_list[0][1] = by2 + left_ypart * 32;//* well_size;
                                vertex_list[1][0] = bx2 + right_xpart * 32; //well_size;
                                vertex_list[1][1] = by2 + right_ypart * 32; //well_size;
                                vertex_list[2][0] = vertex_list[1][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[2][1] = vertex_list[1][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[3][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[3][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[4][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[4][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[5][0] = vertex_list[0][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[5][1] = vertex_list[0][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;

                                add_poly_layer(1, 6, colours.data_well_hexes [0]);


                            }



                            well_size = 140;
                            specific_zoom = view.zoom * w.backblock_parallax [2];

                            bx2 = top_left_corner_x [2] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [2] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);


                            for (k = 0; k < 6; k++)
                            {

                                float base_dist = drand(30, special_i) + 44;
                                well_size = base_dist + 72;
#undef BASE_WELL_ANGLE
#define BASE_WELL_ANGLE (0)

                                float left_xpart = cos(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float left_ypart = sin(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float right_xpart = cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                float right_ypart = sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                vertex_list[0][0] = bx2 + left_xpart * 32;//* well_size;
                                vertex_list[0][1] = by2 + left_ypart * 32;//* well_size;
                                vertex_list[1][0] = bx2 + right_xpart * 32; //well_size;
                                vertex_list[1][1] = by2 + right_ypart * 32; //well_size;
                                vertex_list[2][0] = vertex_list[1][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[2][1] = vertex_list[1][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[3][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[3][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[4][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[4][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[5][0] = vertex_list[0][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[5][1] = vertex_list[0][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;

                                add_poly_layer(1, 6, colours.data_well_hexes [1]);


                            }




                            well_size = 40;
                            specific_zoom = view.zoom * w.backblock_parallax [1];

                            bx2 = top_left_corner_x [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);


                            for (k = 0; k < 6; k++)
                            {

                                float base_dist = drand(30, special_i) + 24;
                                well_size = base_dist + 22;

#undef BASE_WELL_ANGLE
#define BASE_WELL_ANGLE (PI/6)

                                float left_xpart = cos(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float left_ypart = sin(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float right_xpart = cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                float right_ypart = sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                vertex_list[0][0] = bx2 + left_xpart * 32;//* well_size;
                                vertex_list[0][1] = by2 + left_ypart * 32;//* well_size;
                                vertex_list[1][0] = bx2 + right_xpart * 32; //well_size;
                                vertex_list[1][1] = by2 + right_ypart * 32; //well_size;
                                vertex_list[2][0] = vertex_list[1][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[2][1] = vertex_list[1][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[3][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[3][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[4][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[4][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[5][0] = vertex_list[0][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[5][1] = vertex_list[0][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;

                                add_poly_layer(1, 6, colours.data_well_hexes [2]);


                            }



                            int transfer_colour_adjust;

                            if (w.world_time - w.data_well[backbl->backblock_value].last_transferred < 32)
                             transfer_colour_adjust = 32 - (w.world_time - w.data_well[backbl->backblock_value].last_transferred);
                              else
                               transfer_colour_adjust = 0;

                            if (w.data_well[backbl->backblock_value].data > 0)
                            {

                                specific_zoom = view.zoom;

                                bx2 = top_left_corner_x [0] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                                by2 = top_left_corner_y [0] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

                                add_orthogonal_hexagon(2, bx2, by2, (30 * w.data_well[backbl->backblock_value].data * view.zoom) / w.data_well[backbl->backblock_value].data_max, al_map_rgba(220 + transfer_colour_adjust, 200 - transfer_colour_adjust, 50 - transfer_colour_adjust, 180 + transfer_colour_adjust));
                            }

                            specific_zoom = view.zoom * w.backblock_parallax [1];

                            bx2 = top_left_corner_x [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

                            float base_well_ring_angle = PI/6 + (w.world_time * w.data_well[backbl->backblock_value].spin_rate);

#undef WELL_RING_RADIUS
#define WELL_RING_RADIUS 140
                            ALLEGRO_COLOR reserve_colour;

                            reserve_colour = al_map_rgba(180,
                            100,
                            30,
                            140);

                            for (k = 0; k < DATA_WELL_RESERVES; k++)
                            {
                                if (w.data_well[backbl->backblock_value].reserve_data [k] == 0)
                                    continue;
                                int l;
                                float well_ring_radius;
                                if (k == 0)
																																	well_ring_radius = 140 * specific_zoom;
																																  else
																																	  well_ring_radius = 120 * specific_zoom;
                                float reserve_square_arc = 0.26;
                                float reserve_square_length = 18.0 * specific_zoom;
                                float reserve_data_proportion = w.data_well[backbl->backblock_value].reserve_data [k] * 0.002;
                                reserve_square_length *= reserve_data_proportion;
                                if (reserve_data_proportion < 1)
                                 reserve_square_arc *= reserve_data_proportion;
                                for (l = 0; l < w.data_well[backbl->backblock_value].reserve_squares; l ++)
                                {
                                    float square_angle = base_well_ring_angle + ((PI*2) / w.data_well[backbl->backblock_value].reserve_squares) * l;
                                    add_diamond_layer(1,
                                    bx2 + cos(square_angle + reserve_square_arc) * well_ring_radius,
                                    by2 + sin(square_angle + reserve_square_arc) * well_ring_radius,
                                    bx2 + cos(square_angle) * (well_ring_radius + reserve_square_length * 1.3),
                                    by2 + sin(square_angle) * (well_ring_radius + reserve_square_length * 1.3),
                                    bx2 + cos(square_angle - reserve_square_arc) * well_ring_radius,
                                    by2 + sin(square_angle - reserve_square_arc) * well_ring_radius,
                                    bx2 + cos(square_angle) * (well_ring_radius - reserve_square_length),
                                    by2 + sin(square_angle) * (well_ring_radius - reserve_square_length),
                                    reserve_colour);
                                }
                                base_well_ring_angle += PI / w.data_well[backbl->backblock_value].reserve_squares;
                            }


								} // end AREA_GREEN
								break;

							 case AREA_YELLOW:
									{
									 float vertex_dist [4] [6];

									 vertex_dist [0] [0] = 47;
									 vertex_dist [0] [1] = 47;
									 vertex_dist [0] [2] = 47;
									 vertex_dist [0] [3] = 47;
									 vertex_dist [0] [4] = 47;
									 vertex_dist [0] [5] = 47;


									 for (k = 0; k < 3; k++)
										{
											int l;
               for (l = 0; l < 6; l ++)
														 {
															 vertex_dist [k + 1] [l] = vertex_dist [k] [l] + 48 + 8 + drand(11, special_i);// + drand(22, special_i);
														 }
										}

									 for (k = 2; k >= 0; k--)
										{


                            specific_zoom = view.zoom * w.backblock_parallax [k + 1];

                            bx2 = top_left_corner_x [k + 1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [k + 1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);


              vertex_list [0] [0] = bx2;
              vertex_list [0] [1] = by2 - 1.0 * vertex_dist [k + 1] [0] * specific_zoom;
              vertex_list [1] [0] = bx2 + 0.866 * vertex_dist [k + 1] [1] * specific_zoom;
              vertex_list [1] [1] = by2 - 0.5 * vertex_dist [k + 1] [1] * specific_zoom;
              vertex_list [2] [0] = bx2 + 0.866 * vertex_dist [k + 1] [2] * specific_zoom;
              vertex_list [2] [1] = by2 + 0.5 * vertex_dist [k + 1] [2] * specific_zoom;
              vertex_list [3] [0] = bx2;
              vertex_list [3] [1] = by2 + 1.0 * vertex_dist [k + 1] [3] * specific_zoom;
              vertex_list [4] [0] = bx2 - 0.866 * vertex_dist [k + 1] [4] * specific_zoom;
              vertex_list [4] [1] = by2 + 0.5 * vertex_dist [k + 1] [4] * specific_zoom;
              vertex_list [5] [0] = bx2 - 0.866 * vertex_dist [k + 1] [5] * specific_zoom;
              vertex_list [5] [1] = by2 - 0.5 * vertex_dist [k + 1] [5] * specific_zoom;

              add_poly_layer(2, 6, colours.data_well_hexes [2 - k]);


              vertex_list [0] [0] = bx2;
              vertex_list [0] [1] = by2 - 1.0 * (vertex_dist [k] [0] + 22) * specific_zoom;
              vertex_list [1] [0] = bx2 + 0.866 * (vertex_dist [k] [1] + 22) * specific_zoom;
              vertex_list [1] [1] = by2 - 0.5 * (vertex_dist [k] [1] + 22) * specific_zoom;
              vertex_list [2] [0] = bx2 + 0.866 * (vertex_dist [k] [2] + 22) * specific_zoom;
              vertex_list [2] [1] = by2 + 0.5 * (vertex_dist [k] [2] + 22) * specific_zoom;
              vertex_list [3] [0] = bx2;
              vertex_list [3] [1] = by2 + 1.0 * (vertex_dist [k] [3] + 22) * specific_zoom;
              vertex_list [4] [0] = bx2 - 0.866 * (vertex_dist [k] [4] + 22) * specific_zoom;
              vertex_list [4] [1] = by2 + 0.5 * (vertex_dist [k] [4] + 22) * specific_zoom;
              vertex_list [5] [0] = bx2 - 0.866 * (vertex_dist [k] [5] + 22) * specific_zoom;
              vertex_list [5] [1] = by2 - 0.5 * (vertex_dist [k] [5] + 22) * specific_zoom;

              add_poly_layer(2, 6, colours.world_background);

									 }


//          bx2 = top_left_corner_x [k] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
//          by2 = top_left_corner_y [k] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);


//          add_orthogonal_hexagon(0, bx2, by2, 42 * view.zoom, colours.world_background);


                            int transfer_colour_adjust;

                            if (w.world_time - w.data_well[backbl->backblock_value].last_transferred < 32)
                             transfer_colour_adjust = 32 - (w.world_time - w.data_well[backbl->backblock_value].last_transferred);
                              else
                               transfer_colour_adjust = 0;

                            if (w.data_well[backbl->backblock_value].data > 0)
                            {

                                specific_zoom = view.zoom;

                                bx2 = top_left_corner_x [0] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                                by2 = top_left_corner_y [0] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

                                add_orthogonal_hexagon(4, bx2, by2, (30 * w.data_well[backbl->backblock_value].data * view.zoom) / w.data_well[backbl->backblock_value].data_max, al_map_rgba(220 + transfer_colour_adjust, 200 - transfer_colour_adjust, 50 - transfer_colour_adjust, 180 + transfer_colour_adjust));
                            }


                            specific_zoom = view.zoom * w.backblock_parallax [1];

                            bx2 = top_left_corner_x [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

                            float base_well_ring_angle = PI/6 + (w.world_time * w.data_well[backbl->backblock_value].spin_rate);
#undef WELL_RING_RADIUS
#define WELL_RING_RADIUS (80)
                            ALLEGRO_COLOR reserve_colour;

                            reserve_colour = al_map_rgba(180,
                            100,
                            30,
                            140);

                            for (k = 0; k < DATA_WELL_RESERVES; k++)
                            {
                                if (w.data_well[backbl->backblock_value].reserve_data [k] == 0)
                                    continue;
                                int l;
                                float reserve_square_arc_inner = 0.16;
                                float reserve_square_arc_outer = 0.13;
                                float reserve_square_length = 42;
                                float reserve_data_proportion = w.data_well[backbl->backblock_value].reserve_data [k] * 0.002;
                                reserve_square_length *= reserve_data_proportion * specific_zoom;
                                if (reserve_data_proportion < 1)
																																{
                                 reserve_square_arc_inner *= reserve_data_proportion;
                                 reserve_square_arc_outer *= reserve_data_proportion;
																																}
                                for (l = 0; l < w.data_well[backbl->backblock_value].reserve_squares; l ++)
                                {
                                    float square_angle = base_well_ring_angle + ((PI*2) / w.data_well[backbl->backblock_value].reserve_squares) * l;
                                    float square_dist = 70 * specific_zoom;//(80 + drand(60, 1)) * specific_zoom;
                                    float special_bx2 = bx2;// + (drand(60, 1) - 30) * specific_zoom;
                                    float special_by2 = by2;// + (drand(60, 1) - 30) * specific_zoom;
                                    add_diamond_layer(3,
                                    special_bx2 + cos(square_angle - reserve_square_arc_inner) * square_dist,
                                    special_by2 + sin(square_angle - reserve_square_arc_inner) * square_dist,
                                    special_bx2 + cos(square_angle + reserve_square_arc_inner) * square_dist,
                                    special_by2 + sin(square_angle + reserve_square_arc_inner) * square_dist,
                                    special_bx2 + cos(square_angle + reserve_square_arc_outer) * (square_dist + reserve_square_length),
                                    special_by2 + sin(square_angle + reserve_square_arc_outer) * (square_dist + reserve_square_length),
                                    special_bx2 + cos(square_angle - reserve_square_arc_outer) * (square_dist + reserve_square_length),
                                    special_by2 + sin(square_angle - reserve_square_arc_outer) * (square_dist + reserve_square_length),
                                    reserve_colour);
                                }
                                base_well_ring_angle += PI / w.data_well[backbl->backblock_value].reserve_squares;
                            }


									}
									break; // end AREA_YELLOW


       	case AREA_ORANGE:
								{

                            well_size = 40;


#define ORANGE_WELL_BASE_LAYER 2
                            specific_zoom = view.zoom * w.backblock_parallax [ORANGE_WELL_BASE_LAYER];

                            bx2 = top_left_corner_x [ORANGE_WELL_BASE_LAYER] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [ORANGE_WELL_BASE_LAYER] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

                            add_orthogonal_hexagon(1, bx2, by2, 112 * specific_zoom, colours.data_well_hexes [1]);
                            add_orthogonal_hexagon(1, bx2, by2, 80 * specific_zoom, colours.world_background);

                            specific_zoom = view.zoom * w.backblock_parallax [1];

                            bx2 = top_left_corner_x [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);


                            for (k = 0; k < 6; k++)
                            {

                                if ((k + backbl->backblock_value) & 1)
																																		continue;


                                float base_dist = 36;//drand(30, special_i) + 24;
                                well_size = base_dist + 22;

                                float base_angle = (float) w.world_time * w.data_well [backbl->backblock_value].spin_rate;


                                float left_xpart = cos(base_angle + (PI/3)*k + 0.05) * specific_zoom;
                                float left_ypart = sin(base_angle + (PI/3)*k + 0.05) * specific_zoom;
                                float right_xpart = cos(base_angle + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                float right_ypart = sin(base_angle + (PI/3)*(k+1) - 0.05) * specific_zoom;

                                float far_dist = well_size + 100 + drand(30, special_i);

                                vertex_list[0][0] = bx2 + left_xpart * 36;//* well_size;
                                vertex_list[0][1] = by2 + left_ypart * 36;//* well_size;
                                vertex_list[1][0] = bx2 + right_xpart * 36; //well_size;
                                vertex_list[1][1] = by2 + right_ypart * 36; //well_size;
                                vertex_list[2][0] = vertex_list[1][0] + cos(base_angle + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[2][1] = vertex_list[1][1] + sin(base_angle + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;

                                vertex_list[3][0] = vertex_list[2][0] + cos(base_angle + (PI/3)*(k+0.5)) * (far_dist) * specific_zoom;
                                vertex_list[3][1] = vertex_list[2][1] + sin(base_angle + (PI/3)*(k+0.5)) * (far_dist) * specific_zoom;
                                vertex_list[4][0] = vertex_list[1][0] + cos(base_angle + (PI/3)*(k+0.5)) * (far_dist + 52) * specific_zoom;
                                vertex_list[4][1] = vertex_list[1][1] + sin(base_angle + (PI/3)*(k+0.5)) * (far_dist + 52) * specific_zoom;
                                vertex_list[5][0] = vertex_list[0][0] + cos(base_angle + (PI/3)*(k+0.5)) * (far_dist + 52) * specific_zoom;
                                vertex_list[5][1] = vertex_list[0][1] + sin(base_angle + (PI/3)*(k+0.5)) * (far_dist + 52) * specific_zoom;

                                vertex_list[7][0] = vertex_list[0][0] + cos(base_angle + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[7][1] = vertex_list[0][1] + sin(base_angle + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;

                                vertex_list[6][0] = vertex_list[7][0] + cos(base_angle + (PI/3)*(k+0.5)) * (far_dist) * specific_zoom;
                                vertex_list[6][1] = vertex_list[7][1] + sin(base_angle + (PI/3)*(k+0.5)) * (far_dist) * specific_zoom;


/*

                                vertex_list[0][0] = bx2 + left_xpart * 32;// well_size;
                                vertex_list[0][1] = by2 + left_ypart * 32;// well_size;
                                vertex_list[1][0] = bx2 + right_xpart * 32; //well_size;
                                vertex_list[1][1] = by2 + right_ypart * 32; //well_size;
                                vertex_list[2][0] = vertex_list[1][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[2][1] = vertex_list[1][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[3][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[3][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[4][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[4][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[5][0] = vertex_list[0][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[5][1] = vertex_list[0][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;

*/

                                add_poly_layer(1, 8, colours.data_well_hexes [2]);


                            }



                            int transfer_colour_adjust;

                            if (w.world_time - w.data_well[backbl->backblock_value].last_transferred < 32)
                             transfer_colour_adjust = 32 - (w.world_time - w.data_well[backbl->backblock_value].last_transferred);
                              else
                               transfer_colour_adjust = 0;

                            if (w.data_well[backbl->backblock_value].data > 0)
                            {

                                specific_zoom = view.zoom;

                                bx2 = top_left_corner_x [0] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                                by2 = top_left_corner_y [0] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

                                add_orthogonal_hexagon(2, bx2, by2, (30 * w.data_well[backbl->backblock_value].data * view.zoom) / w.data_well[backbl->backblock_value].data_max, al_map_rgba(220 + transfer_colour_adjust, 200 - transfer_colour_adjust, 50 - transfer_colour_adjust, 180 + transfer_colour_adjust));
                            }

                            specific_zoom = view.zoom * w.backblock_parallax [1];

                            bx2 = top_left_corner_x [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

//                            float base_well_ring_angle = PI/6 + (w.world_time * w.data_well[backbl->backblock_value].spin_rate);
//#define WELL_ORANGE_RING_RADIUS (140)
/*                            ALLEGRO_COLOR reserve_colour;

                            reserve_colour = al_map_rgba(180,
                            100,
                            30,
                            140);*/

                            for (k = 0; k < DATA_WELL_RESERVES; k++)
                            {

                                if (w.data_well[backbl->backblock_value].reserve_data [k] == 0)
                                    continue;

                                int l;

																																float circle_base_rad = 220 - k * 70;

																																int circle_distribution = w.data_well[backbl->backblock_value].reserve_squares * 24;

																																float circle_thickness = w.data_well[backbl->backblock_value].reserve_data [k] * 0.005;
																																if (circle_thickness > 10)
																																	circle_thickness = 10;

                                for (l = 0; l < w.data_well[backbl->backblock_value].reserve_squares * 2; l ++)
                                {

                                	float circle_rad = ((w.world_time) + (l * 24)) % (circle_distribution * 2);
                                	circle_rad *= 0.5;

                                	int circle_alpha;
                                	if (circle_rad < (circle_distribution / 2))
																																		circle_alpha = circle_rad * 4;
																																	  else
  																																		circle_alpha = (circle_distribution - circle_rad) * 4;

  																															if (circle_alpha > 100)
																																		circle_alpha = 100;

                                 ALLEGRO_COLOR res_col = al_map_rgba(250,
                                 40 + circle_alpha,
                                 30,
                                 circle_alpha);

//                            	    draw_ring(1, bx2, by2, circle_rad * specific_zoom, 24, 64, res_col2);
                            	    draw_ring(1, bx2, by2, circle_base_rad - circle_rad, circle_thickness, 64, res_col);

                                }

/*
                                int l;
                                float reserve_square_arc = 0.13;
                                float reserve_square_length = 36;
                                float reserve_data_proportion = w.data_well[backbl->backblock_value].reserve_data [k] * 0.002;
                                reserve_square_length *= reserve_data_proportion;
                                if (reserve_data_proportion < 1)
                                 reserve_square_arc *= reserve_data_proportion;
                                for (l = 0; l < w.data_well[backbl->backblock_value].reserve_squares; l ++)
                                {
                                    float square_angle = base_well_ring_angle + ((PI*2) / w.data_well[backbl->backblock_value].reserve_squares) * l;
                                    add_diamond_layer(1,
                                    bx2 + cos(square_angle - reserve_square_arc) * WELL_RING_RADIUS*specific_zoom,
                                    by2 + sin(square_angle - reserve_square_arc) * WELL_RING_RADIUS*specific_zoom,
                                    bx2 + cos(square_angle + reserve_square_arc) * WELL_RING_RADIUS*specific_zoom,
                                    by2 + sin(square_angle + reserve_square_arc) * WELL_RING_RADIUS*specific_zoom,
                                    bx2 + cos(square_angle + reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*specific_zoom,
                                    by2 + sin(square_angle + reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*specific_zoom,
                                    bx2 + cos(square_angle - reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*specific_zoom,
                                    by2 + sin(square_angle - reserve_square_arc) * (WELL_RING_RADIUS + reserve_square_length)*specific_zoom,
                                    reserve_colour);
                                }
                                base_well_ring_angle += PI / w.data_well[backbl->backblock_value].reserve_squares;

*/
                            }

								}
        break; // end AREA_ORANGE


       	case AREA_PURPLE:
								{

                            well_size = 40;



                            specific_zoom = view.zoom * w.backblock_parallax [1];

                            bx2 = top_left_corner_x [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

                            float inner_size = 42 * specific_zoom;
                            float outer_size = 128 * specific_zoom;
                            float far_outer_size = 240 * specific_zoom;


                            for (k = 0; k < 3; k++)
                            {

//                            	if (k != 0)
//																														break;



                                float base_angle;
                                if (backbl->backblock_value & 1)
																																	base_angle = (PI/-6) + ((PI*2) / 3) * k;
																																  else
																																			base_angle = (PI/6) + ((PI*2) / 3) * k;
                                float left_inner_angle = base_angle - PI/3 + 0.18;
                                float left_outer_angle = base_angle - PI/3 + 0.05;
                                float right_inner_angle = base_angle + PI/3 - 0.18;
                                float right_outer_angle = base_angle + PI/3 - 0.05;


                                vertex_list[0][0] = bx2 + cos(base_angle) * inner_size;
                                vertex_list[0][1] = by2 + sin(base_angle) * inner_size;
                                vertex_list[1][0] = bx2 + cos(right_inner_angle) * inner_size;
                                vertex_list[1][1] = by2 + sin(right_inner_angle) * inner_size;
                                vertex_list[2][0] = bx2 + cos(right_outer_angle) * outer_size;
                                vertex_list[2][1] = by2 + sin(right_outer_angle) * outer_size;
#define FAR_OUTER_ANGLE 0.04
                                vertex_list[3][0] = bx2 + cos(base_angle + FAR_OUTER_ANGLE) * far_outer_size;
                                vertex_list[3][1] = by2 + sin(base_angle + FAR_OUTER_ANGLE) * far_outer_size;
                                vertex_list[4][0] = bx2 + cos(base_angle - FAR_OUTER_ANGLE) * far_outer_size;
                                vertex_list[4][1] = by2 + sin(base_angle - FAR_OUTER_ANGLE) * far_outer_size;

                                vertex_list[5][0] = bx2 + cos(left_outer_angle) * outer_size;
                                vertex_list[5][1] = by2 + sin(left_outer_angle) * outer_size;
                                vertex_list[6][0] = bx2 + cos(left_inner_angle) * inner_size;
                                vertex_list[6][1] = by2 + sin(left_inner_angle) * inner_size;
//                                vertex_list[5][0] = bx2 + cos(left_angle) * 36;
//                                vertex_list[5][1] = by2 + sin(left_angle) * 36;



                                add_poly_layer(1, 7, colours.data_well_hexes [2]);


                            }



                            int transfer_colour_adjust;

                            if (w.world_time - w.data_well[backbl->backblock_value].last_transferred < 32)
                             transfer_colour_adjust = 32 - (w.world_time - w.data_well[backbl->backblock_value].last_transferred);
                              else
                               transfer_colour_adjust = 0;

                            if (w.data_well[backbl->backblock_value].data > 0)
                            {

                                specific_zoom = view.zoom;

                                bx2 = top_left_corner_x [0] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                                by2 = top_left_corner_y [0] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

                                add_orthogonal_hexagon(2, bx2, by2, (30 * w.data_well[backbl->backblock_value].data * view.zoom) / w.data_well[backbl->backblock_value].data_max, al_map_rgba(220 + transfer_colour_adjust, 200 - transfer_colour_adjust, 50 - transfer_colour_adjust, 180 + transfer_colour_adjust));
                            }

                            specific_zoom = view.zoom * w.backblock_parallax [1];

                            bx2 = top_left_corner_x [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

                            ALLEGRO_COLOR reserve_colour;

                            reserve_colour = al_map_rgba(180,
                            100,
                            30,
                            140);


                            for (k = 0; k < DATA_WELL_RESERVES; k++)
                            {
                                if (w.data_well[backbl->backblock_value].reserve_data [k] == 0)
                                    continue;
                                int l;
                                float reserve_square_arc_inner;// = 0.16;
                                float reserve_square_arc_outer;// = 0.13;
                                float reserve_square_length = 42;
                                float reserve_data_proportion = w.data_well[backbl->backblock_value].reserve_data [k] * 0.001;
                                float base_dist, base_well_ring_angle;
//                                base_well_ring_angle += (PI / w.data_well[backbl->backblock_value].reserve_squares) * l;
                                reserve_square_length *= reserve_data_proportion * specific_zoom;
                                if (reserve_data_proportion < 1)
																																{
                                 reserve_square_arc_inner *= reserve_data_proportion;
                                 reserve_square_arc_outer *= reserve_data_proportion;
																																}
																																if (k == 0)
																																{
                                     base_dist = 180 * specific_zoom;//(80 + drand(60, 1)) * specific_zoom;
                                     base_well_ring_angle = PI/6 + (w.world_time * w.data_well[backbl->backblock_value].spin_rate);
                                     reserve_square_arc_inner = 0.16;
                                     reserve_square_arc_outer = 0.13;
																																}
																																 else
																																	{
                                      base_dist = 150 * specific_zoom;//(80 + drand(60, 1)) * specific_zoom;
                                      base_well_ring_angle = PI/6 + (w.world_time * w.data_well[backbl->backblock_value].spin_rate * -1);
                                      reserve_square_arc_inner = 0.24;
                                      reserve_square_arc_outer = 0.32;
																																	}
                                for (l = 0; l < w.data_well[backbl->backblock_value].reserve_squares; l ++)
                                {
                                    float special_bx2 = bx2;// + (drand(60, 1) - 30) * specific_zoom;
                                    float special_by2 = by2;// + (drand(60, 1) - 30) * specific_zoom;
                                    if (k == 0)
																																				{
                                     float square_angle = base_well_ring_angle + ((PI*2) / w.data_well[backbl->backblock_value].reserve_squares) * l;
                                     add_diamond_layer(3,
                                      special_bx2 + cos(square_angle - reserve_square_arc_inner) * base_dist,
                                      special_by2 + sin(square_angle - reserve_square_arc_inner) * base_dist,
                                      special_bx2 + cos(square_angle + reserve_square_arc_inner) * base_dist,
                                      special_by2 + sin(square_angle + reserve_square_arc_inner) * base_dist,
                                      special_bx2 + cos(square_angle + reserve_square_arc_outer) * (base_dist + reserve_square_length),
                                      special_by2 + sin(square_angle + reserve_square_arc_outer) * (base_dist + reserve_square_length),
                                      special_bx2 + cos(square_angle - reserve_square_arc_outer) * (base_dist + reserve_square_length),
                                      special_by2 + sin(square_angle - reserve_square_arc_outer) * (base_dist + reserve_square_length),
                                      reserve_colour);
																																				}
																																				   else
																																							{
                                        float square_angle = base_well_ring_angle + ((PI*2) / w.data_well[backbl->backblock_value].reserve_squares) * l;
                                        add_diamond_layer(3,
                                         special_bx2 + cos(square_angle - reserve_square_arc_inner) * base_dist,
                                         special_by2 + sin(square_angle - reserve_square_arc_inner) * base_dist,
                                         special_bx2 + cos(square_angle + reserve_square_arc_inner) * base_dist,
                                         special_by2 + sin(square_angle + reserve_square_arc_inner) * base_dist,
                                         special_bx2 + cos(square_angle + reserve_square_arc_outer) * (base_dist - reserve_square_length),
                                         special_by2 + sin(square_angle + reserve_square_arc_outer) * (base_dist - reserve_square_length),
                                         special_bx2 + cos(square_angle - reserve_square_arc_outer) * (base_dist - reserve_square_length),
                                         special_by2 + sin(square_angle - reserve_square_arc_outer) * (base_dist - reserve_square_length),
                                         reserve_colour);
                                       }
//                                base_well_ring_angle += (PI/2) / w.data_well[backbl->backblock_value].reserve_squares;
//                                     base_well_ring_angle += (PI / w.data_well[backbl->backblock_value].reserve_squares);
                              }
                            }

								}
        break; // end AREA_PURPLE



							 case AREA_RED:
								{


/*
                            well_size = 140;
                            specific_zoom = view.zoom * w.backblock_parallax [3];

                            bx2 = top_left_corner_x [3] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [3] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);


                            for (k = 0; k < 6; k++)
                            {

                                float base_dist = drand(30, special_i) + 44;
                                well_size = base_dist + 122;
#undef BASE_WELL_ANGLE
#define BASE_WELL_ANGLE (PI/6)

                                float left_xpart = cos(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float left_ypart = sin(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float right_xpart = cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                float right_ypart = sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                vertex_list[0][0] = bx2 + left_xpart * 32;// * well_size;
                                vertex_list[0][1] = by2 + left_ypart * 32;// * well_size;
                                vertex_list[1][0] = bx2 + right_xpart * 32; //well_size;
                                vertex_list[1][1] = by2 + right_ypart * 32; //well_size;
                                vertex_list[2][0] = vertex_list[1][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[2][1] = vertex_list[1][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[3][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*(k+0.5)) * (well_size + 64) * specific_zoom;
                                vertex_list[3][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*(k+0.5)) * (well_size + 64) * specific_zoom;
//                                vertex_list[4][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
//                                vertex_list[4][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[4][0] = vertex_list[0][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;
                                vertex_list[4][1] = vertex_list[0][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size / 2 + 12) * specific_zoom;

                                add_poly_layer(1, 4, colours.data_well_hexes [0]);


                            }

*/


                            well_size = 140;
                            specific_zoom = view.zoom * w.backblock_parallax [2];

                            bx2 = top_left_corner_x [2] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [2] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);


                            for (k = 0; k < 6; k++)
                            {

                                float base_dist = drand(30, special_i) + 44;
                                well_size = base_dist + 112 + drand(80, 1);
#undef BASE_WELL_ANGLE
#define BASE_WELL_ANGLE (0)

                                float left_xpart = cos(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float left_ypart = sin(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float right_xpart = cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                float right_ypart = sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                vertex_list[0][0] = bx2 + left_xpart * 32;//  * well_size;
                                vertex_list[0][1] = by2 + left_ypart * 32;// * well_size;
                                vertex_list[1][0] = bx2 + right_xpart * 32; //well_size;
                                vertex_list[1][1] = by2 + right_ypart * 32; //well_size;
                                vertex_list[2][0] = vertex_list[1][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size) * specific_zoom;
                                vertex_list[2][1] = vertex_list[1][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size) * specific_zoom;
//                                vertex_list[3][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*(k+0.5) - 0.45) * (well_size + 48) * specific_zoom;
//                                vertex_list[3][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*(k+0.5) - 0.45) * (well_size + 48) * specific_zoom;
                                vertex_list[3][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*(k+0.5)) * (well_size * 1.5) * specific_zoom;
                                vertex_list[3][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*(k+0.5)) * (well_size * 1.5) * specific_zoom;
//                                vertex_list[5][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*(k+0.5) + 0.45) * (well_size + 48) * specific_zoom;
//                                vertex_list[5][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*(k+0.5) + 0.45) * (well_size + 48) * specific_zoom;
                                vertex_list[4][0] = vertex_list[0][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size) * specific_zoom;
                                vertex_list[4][1] = vertex_list[0][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size) * specific_zoom;

                                add_poly_layer(1, 5, colours.data_well_hexes [1]);


                            }




                            well_size = 40;
                            specific_zoom = view.zoom * w.backblock_parallax [1];

                            bx2 = top_left_corner_x [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);


                            for (k = 0; k < 6; k++)
                            {

                                float base_dist = drand(30, special_i) + 24;
                                well_size = base_dist + 60 + drand(60, 1);

#undef BASE_WELL_ANGLE
#define BASE_WELL_ANGLE (PI/6)

                                float left_xpart = cos(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float left_ypart = sin(BASE_WELL_ANGLE + (PI/3)*k + 0.05) * specific_zoom;
                                float right_xpart = cos(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                float right_ypart = sin(BASE_WELL_ANGLE + (PI/3)*(k+1) - 0.05) * specific_zoom;
                                vertex_list[0][0] = bx2 + left_xpart * 32;//* well_size;
                                vertex_list[0][1] = by2 + left_ypart * 32;//* well_size;
                                vertex_list[1][0] = bx2 + right_xpart * 32; //well_size;
                                vertex_list[1][1] = by2 + right_ypart * 32; //well_size;
                                vertex_list[2][0] = vertex_list[1][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size) * specific_zoom;
                                vertex_list[2][1] = vertex_list[1][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k+1)) * (well_size) * specific_zoom;
                                vertex_list[3][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*(k+0.5)) * (well_size * 1.5) * specific_zoom;
                                vertex_list[3][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*(k+0.5)) * (well_size * 1.5) * specific_zoom;
//                                vertex_list[4][0] = bx2 + cos(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
//                                vertex_list[4][1] = by2 + sin(BASE_WELL_ANGLE + (PI/3)*k + 0.45) * (well_size + 64) * specific_zoom;
                                vertex_list[4][0] = vertex_list[0][0] + cos(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size) * specific_zoom;
                                vertex_list[4][1] = vertex_list[0][1] + sin(BASE_WELL_ANGLE + (PI/3)*(k)) * (well_size) * specific_zoom;

                                add_poly_layer(1, 5, colours.data_well_hexes [2]);


                            }


                            int transfer_colour_adjust;

                            if (w.world_time - w.data_well[backbl->backblock_value].last_transferred < 32)
                             transfer_colour_adjust = 32 - (w.world_time - w.data_well[backbl->backblock_value].last_transferred);
                              else
                               transfer_colour_adjust = 0;

                            if (w.data_well[backbl->backblock_value].data > 0)
                            {

                                specific_zoom = view.zoom;

                                bx2 = top_left_corner_x [0] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                                by2 = top_left_corner_y [0] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

                                add_orthogonal_hexagon(2, bx2, by2, (30 * w.data_well[backbl->backblock_value].data * view.zoom) / w.data_well[backbl->backblock_value].data_max, al_map_rgba(220 + transfer_colour_adjust, 200 - transfer_colour_adjust, 50 - transfer_colour_adjust, 180 + transfer_colour_adjust));
                            }

                            specific_zoom = view.zoom * w.backblock_parallax [1];

                            bx2 = top_left_corner_x [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_i) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);
                            by2 = top_left_corner_y [1] + (BLOCK_SIZE_PIXELS * specific_zoom) * (special_j) + (BLOCK_SIZE_PIXELS/2) * specific_zoom; //((i * BLOCK_SIZE_PIXELS) - camera_offset_x);

//                            float base_well_ring_angle = (w.world_time * w.data_well[backbl->backblock_value].spin_rate);

#undef WELL_RING_RADIUS
#define WELL_RING_RADIUS 140
                            ALLEGRO_COLOR reserve_colour;

                            reserve_colour = al_map_rgba(180,
                            100,
                            30,
                            140);

                            seed_drand(special_i+special_j + w.world_time);


                            for (k = 0; k < DATA_WELL_RESERVES; k++)
                            {
                                if (w.data_well[backbl->backblock_value].reserve_data [k] == 0)
                                    continue;
                                int swirl, l;
                                float reserve_angle;
                                float swirl_spin;
                                if (k == 0)
																																	swirl_spin = w.data_well[backbl->backblock_value].spin_rate;
																																	 else
  																																	swirl_spin = 0 - w.data_well[backbl->backblock_value].spin_rate;
																																reserve_angle = (w.world_time * swirl_spin);
																																int reserve_swirl_steps = 10 + w.data_well[backbl->backblock_value].reserve_data [k] / 35;//3 + w.data_well[backbl->backblock_value].reserve_data [k] / 100;
																																float reserve_dist = (250 - k * 100) * specific_zoom;
																																float reserve_step_arc = 0 - swirl_spin;
                              	 float reserve_width;// = ((reserve_swirl_steps) * 0.4 + drand(6, -1)) * specific_zoom;
                              	 float reserve_width_prop = 1;
                              	 for (swirl = 0; swirl < w.data_well[backbl->backblock_value].reserve_squares; swirl ++)
																																{
//																																	float reserve_front_x = bx2 + cos(reserve_angle) * reserve_dist;
//																																	float reserve_front_y = by2 + sin(reserve_angle) * reserve_dist;

                              	 reserve_width = ((reserve_swirl_steps) * 0.4 + drand(6, -1)) * specific_zoom;
/*
																																 start_ribbon(1,
																																													 bx2 + cos(reserve_angle - reserve_step_arc * 6) * reserve_dist,
																																													 by2 + sin(reserve_angle - reserve_step_arc * 6) * reserve_dist,
																																													 bx2 + cos(reserve_angle - reserve_step_arc * 4.5) * (reserve_dist + reserve_width * 0.6),
																																													 by2 + sin(reserve_angle - reserve_step_arc * 4.5) * (reserve_dist + reserve_width * 0.6),
																																													 reserve_colour);
																																 add_ribbon_vertex(bx2 + cos(reserve_angle - reserve_step_arc * 4.5) * (reserve_dist - reserve_width * 0.6),
																																													      by2 + sin(reserve_angle - reserve_step_arc * 4.5) * (reserve_dist - reserve_width * 0.6),
																																																		 reserve_colour);
																																 add_ribbon_vertex(bx2 + cos(reserve_angle + reserve_step_arc) * (reserve_dist + reserve_width),
																																													      by2 + sin(reserve_angle + reserve_step_arc) * (reserve_dist + reserve_width),
																																																		 reserve_colour);
																																 add_ribbon_vertex(bx2 + cos(reserve_angle + reserve_step_arc) * (reserve_dist - reserve_width),
																																													      by2 + sin(reserve_angle + reserve_step_arc) * (reserve_dist - reserve_width),
																																																		 reserve_colour);
*/

																																 start_ribbon(1,
																																													 bx2 + cos(reserve_angle - reserve_step_arc * 4) * reserve_dist,
																																													 by2 + sin(reserve_angle - reserve_step_arc * 4) * reserve_dist,
																																													 bx2 + cos(reserve_angle + reserve_step_arc) * (reserve_dist + reserve_width),
																																													 by2 + sin(reserve_angle + reserve_step_arc) * (reserve_dist + reserve_width),
																																													 reserve_colour);
																																 add_ribbon_vertex(bx2 + cos(reserve_angle + reserve_step_arc) * (reserve_dist - reserve_width),
																																													      by2 + sin(reserve_angle + reserve_step_arc) * (reserve_dist - reserve_width),
																																																		 reserve_colour);


                                 for (l = 2; l < reserve_swirl_steps; l ++) // note starts at 2
                                 {
                                	 reserve_width_prop = (float) (reserve_swirl_steps - l) / (float) (reserve_swirl_steps);
                                	 reserve_width = ((reserve_swirl_steps - l) * 0.4 + drand(6, -1)) * specific_zoom * reserve_width_prop;
                                    float step_angle = reserve_angle + reserve_step_arc * l;
																																  add_ribbon_vertex(bx2 + cos(step_angle) * (reserve_dist + reserve_width),
																																													       by2 + sin(step_angle) * (reserve_dist + reserve_width),
																																																		  reserve_colour);
																																  add_ribbon_vertex(bx2 + cos(step_angle) * (reserve_dist - reserve_width),
																																													       by2 + sin(step_angle) * (reserve_dist - reserve_width),
																																																		  reserve_colour);

                                 }

                                 reserve_angle += (2 * PI) / w.data_well[backbl->backblock_value].reserve_squares;

																																}
                            }


								} // end AREA_ORANGE
								break;



       }




} // end data wells
















/*
// use this for blocks where one coordinate (x or y) will always be irrelevant (only the relevant coordinate is used to calculate base_pos, and subblock_pos is the relevant coordinate)
static void vision_block_check(struct block_struct* bl, int base_pos)
{
    int dist;

				if (bl->vision_block_proximity_time != w.world_time)
				{
					bl->vision_block_proximity_time = w.world_time;
					bl->vision_block_proximity = 100000;
				}

    dist = abs(base_pos) - (BLOCK_SIZE_PIXELS*4);

				if (bl->vision_block_proximity > dist)
					bl->vision_block_proximity = dist;

}

// use this for blocks where the most distant of x and y should be used for each subblock
static void vision_block_check_corner(struct block_struct* bl, int base_pos_x, int base_pos_y)
{

 int dist_x, dist_y, dist;


				if (bl->vision_block_proximity_time != w.world_time)
				{
					bl->vision_block_proximity_time = w.world_time;
					bl->vision_block_proximity = 100000;
				}

     dist_x = abs(base_pos_x) - (BLOCK_SIZE_PIXELS*4);
     dist_y = abs(base_pos_y) - (BLOCK_SIZE_PIXELS*4);
     if (dist_x > dist_y)
						dist = dist_x;
					  else
								dist = dist_y;
					if (bl->vision_block_proximity > dist)
						bl->vision_block_proximity = dist;

}

static void set_outer_edge_vision_block(int block_x, int block_y)
{

	if (w.block[block_x][block_y].vision_block_proximity_time == w.world_time)
		return; // if anything has been done to the block this tick, we don't need to do anything here

	w.block[block_x][block_y].vision_block_proximity_time = w.world_time;

	w.block[block_x][block_y].vision_block_proximity = 50000;

}*/

// Currently this is just used for pregame, to display the visible area where the player's first proc will spawn
static void special_visible_area(cart notional_core_position)
{



	int base_min_x, base_max_x, base_min_y, base_max_y;
	int min_x, max_x, min_y, max_y;
	int i,j;

	int block_x = fixed_to_block(notional_core_position.x);
	int block_y = fixed_to_block(notional_core_position.y);
	int core_x = al_fixtoi(notional_core_position.x);
	int core_y = al_fixtoi(notional_core_position.y);

	int block_min_x = (al_fixtoi(view.camera_x) - (view.window_x_unzoomed / (view.zoom * 2))) / BLOCK_SIZE_PIXELS - 2;
	if (block_min_x < 2)
		block_min_x = 2;
	int block_min_y = (al_fixtoi(view.camera_y) - (view.window_y_unzoomed / (view.zoom * 2))) / BLOCK_SIZE_PIXELS - 2;
	if (block_min_y < 2)
		block_min_y = 2;
	int block_max_x = block_min_x + ((view.window_x_unzoomed / BLOCK_SIZE_PIXELS) / view.zoom) + 6;
	if (block_max_x >= w.blocks.x - 2)
		block_max_x = w.blocks.x - 2;
	int block_max_y = block_min_y + ((view.window_y_unzoomed / BLOCK_SIZE_PIXELS) / view.zoom) + 6;
	if (block_max_y >= w.blocks.y - 2)
		block_max_y = w.blocks.y - 2;

//#define VISION_SIZE_BLOCKS 6

		base_min_x = block_x - SCAN_RANGE_BASE_BLOCKS;
		base_min_y = block_y - SCAN_RANGE_BASE_BLOCKS;
		base_max_x = block_x + SCAN_RANGE_BASE_BLOCKS + 1;
		base_max_y = block_y + SCAN_RANGE_BASE_BLOCKS + 1;

		if (base_min_x > block_max_x
			|| base_min_y > block_max_y
			|| base_max_x < block_min_x
			|| base_max_y < block_min_y)
				return; // core's vision range is not visible on screen

		if (base_min_x < block_min_x)
			min_x = block_min_x;
			 else
					min_x = base_min_x;
	 if (base_min_y < block_min_y)
			min_y = block_min_y;
			 else
					min_y = base_min_y;
		if (base_max_x > block_max_x)
			max_x = block_max_x;
			 else
  			max_x = base_max_x;
		if (base_max_y > block_max_y)
			max_y = block_max_y;
			 else
  			max_y = base_max_y;

		int vision_range = SCAN_RANGE_BASE_PIXELS + 64;

		for (i = min_x; i < max_x;	i ++)
		{
		 for (j = min_y; j < max_y;	j ++)
		 {
//		 	struct block_struct* bl = &w.block[i][j];

		 	if (w.vision_block[i][j].clear_time == w.world_time)
					continue;

//    int dist_x = abs(core_x - (i * BLOCK_SIZE_PIXELS + (j&1) * (BLOCK_SIZE_PIXELS / 2)));
//    int dist_y = abs(core_y - j * BLOCK_SIZE_PIXELS);

    int dist_x = abs(core_x - (i * BLOCK_SIZE_PIXELS + (j&1) * (BLOCK_SIZE_PIXELS / 2)));
    int dist_y = abs(core_y - j * BLOCK_SIZE_PIXELS);

    int dist;

// approximate octagonal distance:
    if (dist_x > dist_y)
				{
					dist = 0.41 * dist_y + 0.94124 * dist_x;
				}
				  else
						{
  					dist = 0.41 * dist_x + 0.94124 * dist_y;
						}


// Surely this rubbish can be optimised

				if (w.vision_block[i][j].proximity_time != w.world_time)
				{
					w.vision_block[i][j].proximity_time = w.world_time;
					w.vision_block[i][j].proximity = 100000;
				}


				if (dist > vision_range)//BLOCK_SIZE_PIXELS * 5)
				{
					continue;
				}

				dist -= (vision_range - BLOCK_SIZE_PIXELS * 3);//BLOCK_SIZE_PIXELS * 3;

				if (dist < 0)
					dist = 0;

				w.vision_block[i][j].proximity_time = w.world_time;
				if (w.vision_block[i][j].proximity > dist)
					w.vision_block[i][j].proximity = dist;

		 }
		}


/*
	int block_x = fixed_to_block(notional_core_position.x);
	int block_y = fixed_to_block(notional_core_position.y);

	int base_min_x, base_max_x, base_min_y, base_max_y;
	int min_x, max_x, min_y, max_y;
//	int clipped_left, clipped_right, clipped_top, clipped_bottom;
	int i,j;

	int block_min_x = (al_fixtoi(view.camera_x) - (view.window_x_unzoomed / (view.zoom * 2))) / BLOCK_SIZE_PIXELS - 2;
	if (block_min_x < 1)
		block_min_x = 1;
	int block_min_y = (al_fixtoi(view.camera_y) - (view.window_y_unzoomed / (view.zoom * 2))) / BLOCK_SIZE_PIXELS - 2;
	if (block_min_y < 1)
		block_min_y = 1;
	int block_max_x = block_min_x + ((view.window_x_unzoomed / BLOCK_SIZE_PIXELS) / view.zoom) + 6;
	if (block_max_x >= w.blocks.x - 1)
		block_max_x = w.blocks.x - 1;
	int block_max_y = block_min_y + ((view.window_y_unzoomed / BLOCK_SIZE_PIXELS) / view.zoom) + 6;
	if (block_max_y >= w.blocks.y - 1)
		block_max_y = w.blocks.y - 1;


		base_min_x = block_x - VISION_SIZE_BLOCKS;
		base_min_y = block_y - VISION_SIZE_BLOCKS;
		base_max_x = block_x + VISION_SIZE_BLOCKS + 1;
		base_max_y = block_y + VISION_SIZE_BLOCKS + 1;

		if (base_min_x > block_max_x
			|| base_min_y > block_max_y
			|| base_max_x < block_min_x
			|| base_max_y < block_min_y)
				return; // core's vision range is not visible on screen

		if (base_min_x < block_min_x)
			min_x = block_min_x;
			 else
					min_x = base_min_x;
	 if (base_min_y < block_min_y)
			min_y = block_min_y;
			 else
					min_y = base_min_y;
		if (base_max_x > block_max_x)
			max_x = block_max_x;
			 else
  			max_x = base_max_x;
		if (base_max_y > block_max_y)
			max_y = block_max_y;
			 else
  			max_y = base_max_y;

  int clear_min_x = base_min_x;
  if (clear_min_x < 0)
			clear_min_x = 0;
  int clear_min_y = base_min_y;
  if (clear_min_y < 0)
			clear_min_y = 0;
  int clear_max_x = base_max_x;
  if (clear_max_x > w.blocks.x)
			clear_max_x = w.blocks.x;
  int clear_max_y = base_max_y;
  if (clear_max_y > w.blocks.y)
			clear_max_y = w.blocks.y;


// clear out the centre:
		for (j = clear_min_y + 2; j < clear_max_y - 2;	j ++)
		{
		 for (i = clear_min_x + 2; i < clear_max_x - 2;	i ++)
		 {
		   w.block[i][j].vision_block_clear_time = w.world_time;
		 }
		}

// now set the outside so it's at a great distance (and appears black but with the appropriate edges) unless something else can see it:
//  (okay to do e.g. min_x - 1 because min_x's minimum value is 1)
		for (j = min_y - 1; j < max_y + 1;	j ++)
		{
			set_outer_edge_vision_block(min_x - 1, j);
			set_outer_edge_vision_block(max_x, j);
		}
		for (i = min_x; i < max_x;	i ++)
		{
			set_outer_edge_vision_block(i, min_y - 1);
			set_outer_edge_vision_block(i, max_y);
		}


//		int dist;
		int core_x = al_fixtoi(notional_core_position.x);
		int core_y = al_fixtoi(notional_core_position.y);

// left side


  int column;

  if (min_x <= base_min_x + 1)
		{
   column = min_x + 1;
 		for (j = min_y + 1; j < max_y - 1;	j ++)
	 	{
				vision_block_check(&w.block[column][j],
																							(column * BLOCK_SIZE_PIXELS) - core_x);
	 	}

   if (min_x == base_min_x)
		 {
  		for (j = min_y + 1; j < max_y - 1;	j ++)
	 	 {
 				vision_block_check(&w.block[min_x][j],
																							 (min_x * BLOCK_SIZE_PIXELS) - core_x);
	 	 }
		 }
		}

// right side

  if (max_x >= base_max_x - 2)
		{
   column = max_x - 2;
 		for (j = min_y + 1; j < max_y - 1;	j ++)
	 	{
				vision_block_check(&w.block[column][j],
																						 (column * BLOCK_SIZE_PIXELS) - core_x);
	 	}

   column = max_x - 1;
	 	if (max_x == base_max_x)
		 {
  		for (j = min_y + 1; j < max_y - 1;	j ++)
	 	 {
 				vision_block_check(&w.block[column][j],
																						 (column * BLOCK_SIZE_PIXELS) - core_x);
	 	 }
		 }
		}

// upper side


  int row = min_y + 1;

  if (min_y <= base_min_y + 1)
		{
 		for (i = min_x + 1; i < max_x - 1;	i ++)
	 	{
				vision_block_check(&w.block[i][row],
																							(row * BLOCK_SIZE_PIXELS) - core_y);
	 	}
//   row = min_y + 1;
	  if (min_y == base_min_y)
 		{
  		for (i = min_x + 1; i < max_x - 1;	i ++)
	  	{
			 	vision_block_check(&w.block[i][min_y],
																							(min_y * BLOCK_SIZE_PIXELS) - core_y);
	 	 }
		 }
		}

// lower side


  row = max_y - 2;
  if (max_y >= base_max_y - 2)
		{
 		for (i = min_x + 1; i < max_x - 1;	i ++)
	 	{
				vision_block_check(&w.block[i][row],
																							(row * BLOCK_SIZE_PIXELS) - core_y);
	 	}
   row = max_y - 1;
   if (max_y == base_max_y)
		 {
  		for (i = min_x + 1; i < max_x - 1;	i ++)
	 	 {
 				vision_block_check(&w.block[i][row],
																							(row * BLOCK_SIZE_PIXELS) - core_y);
	 	 }
		 }
		}

// corners
		if (min_x <= base_min_x + 1)
	 {
		 if (min_y <= base_min_y + 1)
    vision_block_check_corner(&w.block[min_x+1][min_y+1], ((min_x+1) * BLOCK_SIZE_PIXELS) - core_x, ((min_y+1) * BLOCK_SIZE_PIXELS) - core_y);
		 if (max_y >= base_max_y - 1)
    vision_block_check_corner(&w.block[min_x+1][max_y-2], ((min_x+1) * BLOCK_SIZE_PIXELS) - core_x, ((max_y-2) * BLOCK_SIZE_PIXELS) - core_y);

			if (min_x == base_min_x)
		 {
			 if (min_y == base_min_y)
     vision_block_check_corner(&w.block[min_x][min_y], (min_x * BLOCK_SIZE_PIXELS) - core_x, (min_y * BLOCK_SIZE_PIXELS) - core_y);
			 if (max_y == base_max_y)
     vision_block_check_corner(&w.block[min_x][max_y-1], (min_x * BLOCK_SIZE_PIXELS) - core_x, ((max_y-1) * BLOCK_SIZE_PIXELS) - core_y);
		 }
	 }

		if (max_x >= base_max_x - 1)
	 {
		 if (min_y <= base_min_y + 1)
    vision_block_check_corner(&w.block[max_x-2][min_y+1], ((max_x-2) * BLOCK_SIZE_PIXELS) - core_x, ((min_y+1) * BLOCK_SIZE_PIXELS) - core_y);
		 if (max_y >= base_max_y - 1)
    vision_block_check_corner(&w.block[max_x-2][max_y-2], ((max_x-2) * BLOCK_SIZE_PIXELS) - core_x, ((max_y-2) * BLOCK_SIZE_PIXELS) - core_y);

			if (max_x == base_max_x)
		 {
			 if (min_y == base_min_y)
     vision_block_check_corner(&w.block[max_x-1][min_y], ((max_x-1) * BLOCK_SIZE_PIXELS) - core_x, (min_y * BLOCK_SIZE_PIXELS) - core_y);
			 if (max_y == base_max_y)
     vision_block_check_corner(&w.block[max_x-1][max_y-1], ((max_x-1) * BLOCK_SIZE_PIXELS) - core_x, ((max_y-1) * BLOCK_SIZE_PIXELS) - core_y);
		 }
	 }

*/


}

#ifdef USE_SYSTEM_MOUSE_CURSOR

// initialisation function. In this file so it can use various drawing functions
void make_mouse_cursor(ALLEGRO_BITMAP* mc_bmp, int mcursor)
{


 al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
 al_set_target_bitmap(mc_bmp);
 al_clear_to_color(al_map_rgba(0,0,0,0));
 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

 clear_vbuf();

 float x = 0;
 float y = 0;
 int x_focus = 0, y_focus = 0;

 switch(mcursor)
 {
 	case MOUSE_CURSOR_BASIC:
 	default:
   add_outline_diamond_layer(0,
																			  x+1, y,
																			  x + 17, y + 15,
																			  x + 12, y + 25,
																			  x+1, y + 24,
																			  colours.base [COL_GREY] [SHADE_MAX],
																			  colours.base [COL_GREY] [SHADE_MED]);
/*
   add_diamond_layer(0,
																			  x + 2, y + 4,
																			  x + 14, y + 15,
																			  x + 10, y + 23,
																			  x + 2, y + 20,
																			  colours.base [COL_GREY] [SHADE_MAX]);*/
    break;
/*
	 case MOUSE_CURSOR_PROCESS_FRIENDLY:
   add_diamond_layer(0,
																			  x - 2, y - 4,
																			  x + 15, y + 11,
																			  x + 10, y + 21,
																			  x - 2, y + 20,
																			  colours.base_trans [COL_GREY] [SHADE_MIN] [TRANS_FAINT]);

   add_diamond_layer(0,
																			  x, y,
																			  x + 12, y + 11,
																			  x + 8, y + 19,
																			  x, y + 18,
																			  colours.base [COL_BLUE] [SHADE_MAX]);
    break;
	 case MOUSE_CURSOR_PROCESS_ENEMY:
   add_diamond_layer(0,
																			  x - 2, y - 4,
																			  x + 15, y + 11,
																			  x + 10, y + 21,
																			  x - 2, y + 20,
																			  colours.base_trans [COL_GREY] [SHADE_MIN] [TRANS_FAINT]);

   add_diamond_layer(0,
																			  x, y,
																			  x + 12, y + 11,
																			  x + 8, y + 19,
																			  x, y + 18,
																			  colours.base [COL_RED] [SHADE_MAX]);
    break;
	 case MOUSE_CURSOR_MAP:
   add_diamond_layer(0,
																			  x - 2, y - 4,
																			  x + 15, y + 11,
																			  x + 10, y + 21,
																			  x - 2, y + 20,
																			  colours.base_trans [COL_GREY] [SHADE_MIN] [TRANS_FAINT]);

   add_diamond_layer(0,
																			  x, y,
																			  x + 12, y + 11,
																			  x + 8, y + 19,
																			  x, y + 18,
																			  colours.base [COL_GREEN] [SHADE_MAX]);
    break;

   case MOUSE_CURSOR_RESIZE:
   add_diamond_layer(0,
																			  x - 1, y,
																			  x - 6, y - 6,
																			  x - 13, y,
																			  x - 6, y + 6,
																			  colours.base [COL_GREY] [SHADE_MAX]);

   add_diamond_layer(0,
																			  x + 1, y,
																			  x + 6, y - 6,
																			  x + 13, y,
																			  x + 6, y + 6,
																			  colours.base [COL_GREY] [SHADE_MAX]);
			break;
*/
 }

int i = 0;
/*
   al_draw_indexed_prim(vbuf.buffer_triangle,
																							 NULL, // vertex declaration
																							 NULL, // texture
																							 vbuf.index_triangle [i],
																							 vbuf.index_pos_triangle [i],
																							 ALLEGRO_PRIM_TRIANGLE_LIST);
		vbuf.index_pos_triangle [i] = 0;

	vbuf.vertex_pos_triangle = 0;
*/
	draw_vbuf();

	inter.mcursor [mcursor] = al_create_mouse_cursor(mc_bmp, x_focus, y_focus);
	if (inter.mcursor [mcursor] == NULL)
	{
		fpr("\n i_display.c: make_mouse_cursor(): failed on cursor %i.", mcursor);
		error_call();
	}


}

void set_mouse_cursor(int mc_index)
{

	al_set_mouse_cursor(display, inter.mcursor [mc_index]);

}
#endif

//#define DRAW_SHAPE_DATA


// wrapper around al_map_rgb that does bounds-checking
ALLEGRO_COLOR map_rgb(int r, int g, int b)
{

 if (r < 0)
  r = 0;
 if (r > 255)
  r = 255;
 if (g < 0)
  g = 0;
 if (g > 255)
  g = 255;
 if (b < 0)
  b = 0;
 if (b > 255)
  b = 255;

 return al_map_rgb(r, g, b);

}

// wrapper around al_map_rgba that does bounds-checking
ALLEGRO_COLOR map_rgba(int r, int g, int b, int a)
{

 if (r < 0)
  r = 0;
 if (r > 255)
  r = 255;
 if (g < 0)
  g = 0;
 if (g > 255)
  g = 255;
 if (b < 0)
  b = 0;
 if (b > 255)
  b = 255;
 if (a < 0)
  a = 0;
 if (a > 255)
  a = 255;


 return al_map_rgba(r, g, b, a);

}


#ifdef DRAW_SHAPE_DATA

void draw_a_proc_shape_data(int s, float x, float y);

// This generates a special image for the manual (so a screenshot can be taken of it) then pretends to have an error and exits.
void draw_proc_shape_data(void)
{

 al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
 ALLEGRO_BITMAP* shape_bmp = al_create_bitmap(1000, 3000);
 if (!shape_bmp)
	{
		fprintf(stdout, "\nError: shape_bmp not created.");
		error_call();
	}
 al_set_target_bitmap(shape_bmp);

// al_set_target_bitmap(al_get_backbuffer(display));
 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
 al_clear_to_color(colours.base [COL_BLUE] [SHADE_MIN]);

 reset_fan_index();

 int s;
#define DATA_BASE_X 140
 float x = DATA_BASE_X;
 float y = 100;

 for (s = 0; s < SHAPES; s ++)
 {
  draw_a_proc_shape_data(s, x, y);
  x += 240;
  if (x > 1000
			|| s == SHAPE_4SQUARE - 1
			|| s == SHAPE_5PENTAGON - 1
			|| s == SHAPE_6HEXAGON - 1
			|| s == SHAPE_8OCTAGON - 1)
  {
   y += 300;
   x = DATA_BASE_X;
  }
 }

 draw_fans();
 draw_from_buffers();

 al_save_bitmap("shape_test.bmp", shape_bmp);
/*
 if (settings.option [OPTION_SPECIAL_CURSOR])
  draw_mouse_cursor();
 al_flip_display();
*/
 error_call();

}

#define DRAW_TEAM 0
#define DRAW_SIZE 1
#define DRAW_LINE 12

void draw_a_proc_shape_data(int s, float x, float y)
{

 struct proc_struct draw_pr; // this just needs to be initialised to the extent that add_proc_shape() uses
 struct shape_struct* sh = &shape_dat [s] [DRAW_SIZE];
 float f_angle = -PI/2;

 int i;

 for (i = 0; i < SHAPES_VERTICES; i ++)
 {
  draw_pr.vertex_method [i] = -1;
 }

 add_proc_shape(&draw_pr, x, y, f_angle, sh, s, DRAW_SIZE,
                colours.proc_fill [DRAW_TEAM] [PROC_FILL_SHADES - 1] [0],
                colours.team [DRAW_TEAM] [TCOL_FILL_BASE],
                colours.team [DRAW_TEAM] [TCOL_MAIN_EDGE]);

 float vx, vy;

 for (i = 0; i < sh->vertices; i ++)
 {
  vx = x + fxpart(f_angle + sh->vertex_angle_float [i], sh->vertex_dist_pixel [i] + 10);
  vy = y + fypart(f_angle + sh->vertex_angle_float [i], sh->vertex_dist_pixel [i] + 10);
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], vx, vy - 4, ALLEGRO_ALIGN_CENTRE, "%i", i);
 }

 char sh_str [30] = "nothing";

 switch(s)
 {
  case SHAPE_3TRIANGLE: strcpy(sh_str, "SHAPE_3TRIANGLE"); break;
  case SHAPE_4SQUARE: strcpy(sh_str, "SHAPE_4SQUARE"); break;
  case SHAPE_4DIAMOND: strcpy(sh_str, "SHAPE_4DIAMOND"); break;
  case SHAPE_4POINTY: strcpy(sh_str, "SHAPE_4POINTY"); break;
  case SHAPE_4TRAP: strcpy(sh_str, "SHAPE_4TRAP"); break;
  case SHAPE_4IRREG_L: strcpy(sh_str, "SHAPE_4IRREG_L"); break;
  case SHAPE_4IRREG_R: strcpy(sh_str, "SHAPE_4IRREG_R"); break;
  case SHAPE_4ARROW: strcpy(sh_str, "SHAPE_4ARROW"); break;
  case SHAPE_5PENTAGON: strcpy(sh_str, "SHAPE_5PENTAGON"); break;
  case SHAPE_5POINTY: strcpy(sh_str, "SHAPE_5POINTY"); break;
  case SHAPE_5LONG: strcpy(sh_str, "SHAPE_5LONG"); break;
  case SHAPE_5WIDE: strcpy(sh_str, "SHAPE_5WIDE"); break;
  case SHAPE_6HEXAGON: strcpy(sh_str, "SHAPE_6HEXAGON"); break;
  case SHAPE_6POINTY: strcpy(sh_str, "SHAPE_6POINTY"); break;
  case SHAPE_6LONG: strcpy(sh_str, "SHAPE_6LONG"); break;
  case SHAPE_6IRREG_L: strcpy(sh_str, "SHAPE_6IRREG_L"); break;
  case SHAPE_6IRREG_R: strcpy(sh_str, "SHAPE_6IRREG_R"); break;
  case SHAPE_6ARROW: strcpy(sh_str, "SHAPE_6ARROW"); break;
  case SHAPE_6STAR: strcpy(sh_str, "SHAPE_6STAR"); break;
  case SHAPE_8OCTAGON: strcpy(sh_str, "SHAPE_8OCTAGON"); break;
  case SHAPE_8POINTY: strcpy(sh_str, "SHAPE_8POINTY"); break;
  case SHAPE_8LONG: strcpy(sh_str, "SHAPE_8LONG"); break;
  case SHAPE_8STAR: strcpy(sh_str, "SHAPE_8STAR"); break;
 }




#define DRAW_X_COL 30
#define DRAW_X_COL_NAME 30

 float line_y = y + 80;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], x, line_y, ALLEGRO_ALIGN_CENTRE, "%s", sh_str);
 line_y += DRAW_LINE + 10;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], x - DRAW_X_COL_NAME, line_y, ALLEGRO_ALIGN_RIGHT, "SIZE");
 for (i = 0; i < SHAPES_SIZES; i ++)
 {
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], x + i * DRAW_X_COL, line_y, ALLEGRO_ALIGN_RIGHT, "%i", i);
 }
 line_y += DRAW_LINE;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], x - DRAW_X_COL_NAME, line_y, ALLEGRO_ALIGN_RIGHT, "base mass");
 for (i = 0; i < SHAPES_SIZES; i ++)
 {
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MED], x + i * DRAW_X_COL, line_y, ALLEGRO_ALIGN_RIGHT, "%i", shape_dat [s] [i].shape_mass);
 }
 line_y += DRAW_LINE;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], x - DRAW_X_COL_NAME, line_y, ALLEGRO_ALIGN_RIGHT, "max method mass");
 for (i = 0; i < SHAPES_SIZES; i ++)
 {
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MED], x + i * DRAW_X_COL, line_y, ALLEGRO_ALIGN_RIGHT, "%i", shape_dat [s] [i].mass_max - shape_dat [s] [i].shape_mass);
 }
 line_y += DRAW_LINE;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], x - DRAW_X_COL_NAME, line_y, ALLEGRO_ALIGN_RIGHT, "max hp");
 for (i = 0; i < SHAPES_SIZES; i ++)
 {
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MED], x + i * DRAW_X_COL, line_y, ALLEGRO_ALIGN_RIGHT, "%i", shape_dat [s] [i].base_hp_max);
 }
 line_y += DRAW_LINE;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], x - DRAW_X_COL_NAME, line_y, ALLEGRO_ALIGN_RIGHT, "irpt buffer");
 for (i = 0; i < SHAPES_SIZES; i ++)
 {
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MED], x + i * DRAW_X_COL, line_y, ALLEGRO_ALIGN_RIGHT, "%i", shape_dat [s] [i].base_irpt_max);
 }
 line_y += DRAW_LINE;
 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_HIGH], x - DRAW_X_COL_NAME, line_y, ALLEGRO_ALIGN_RIGHT, "data buffer");
 for (i = 0; i < SHAPES_SIZES; i ++)
 {
  al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MED], x + i * DRAW_X_COL, line_y, ALLEGRO_ALIGN_RIGHT, "%i", shape_dat [s] [i].base_data_max);
 }

 int base_hp_max;
 int base_irpt_max;
 int base_data_max;
 int mass_max;
 int shape_mass; // basic mass of shape with no methods
/*
   shape_dat [i] [j].base_hp_max = shape_solidity [i] * (5 + j); //shape_type[i].hp * (5 + j);
   shape_dat [i] [j].base_irpt_max = shape_solidity [i] * (4 + j) * 20; //shape_type[i].irpt * (5 + j);
   shape_dat [i] [j].base_data_max = shape_solidity [i] * (4 + j) * 2; //shape_type[i].data * (5 + j);
   shape_dat [i] [j].method_mass_max = shape_solidity [i] * (8 + (j * 4));
   shape_dat [i] [j].shape_mass = shape_solidity [i] * (3 + j); //shape_type[i].shape_mass * (5 + j);
*/

}




#endif
// ends ifdef DRAW_SHAPE_DATA

//#define DRAW_SHAPE_TEST

#ifdef DRAW_SHAPE_TEST


void draw_a_proc_shape_test(int s, int size, int method_vertices, float x, float y);

// This generates a special image for the manual (so a screenshot can be taken of it) then pretends to have an error and exits.
void draw_proc_shape_test(void)
{

 al_set_target_bitmap(al_get_backbuffer(display));
 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
 al_clear_to_color(colours.base [COL_BLUE] [SHADE_MIN]);

 int s;
 int size;
 float x = 60;
 float y = 40;

 for (s = 2; s < SHAPES; s ++)
 {
 	for (size = 0; size < 4; size ++)
		{
   draw_a_proc_shape_test(s, size, -1, x, y);
   draw_a_proc_shape_test(s, size, 1, x, y + 400);
   y += 60 + (size * 25);
		}
		y = 40;
  x += 90;
 }

 draw_from_buffers();
 draw_fans();

 if (settings.option [OPTION_SPECIAL_CURSOR])
  draw_mouse_cursor();
 al_flip_display();
 al_set_target_bitmap(al_get_backbuffer(display));

 error_call();

}

#define DRAW_TEAM 0
#define DRAW_SIZE 1
#define DRAW_LINE 12

void draw_a_proc_shape_test(int s, int size, int method_vertices, float x, float y)
{

 struct proc_struct draw_pr; // this just needs to be initialised to the extent that add_proc_shape() uses
 struct shape_struct* sh = &shape_dat [s] [size];
 float f_angle = -PI/2;

 int i;

 for (i = 0; i < SHAPES_VERTICES; i ++)
 {
  draw_pr.vertex_method [i] = method_vertices;
 }

 add_proc_shape(&draw_pr, x, y, f_angle, sh, s, size,
                colours.proc_fill [DRAW_TEAM] [PROC_FILL_SHADES - 1] [0],
                colours.team [DRAW_TEAM] [TCOL_FILL_BASE],
                colours.team [DRAW_TEAM] [TCOL_MAIN_EDGE]);



}



#endif


