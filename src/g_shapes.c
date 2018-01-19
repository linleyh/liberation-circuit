#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <math.h>
#include <stdio.h>

#include "m_config.h"
#include "g_header.h"
#include "g_shapes.h"
#include "g_misc.h"
#include "m_maths.h"
#include "c_keywords.h"
#include "i_header.h"
#include "h_story.h"

#include "z_poly.h"

struct nshape_struct nshape [NSHAPES];
struct dshape_struct dshape [NSHAPES]; // uses same indices as NSHAPES

// this struct holds polygon information for use in the collision mask drawing function.
// it could be held in nshape instead, but is only used during initialisation
struct collision_mask_poly_struct
{
	int polys;
	int vertices [DSHAPE_POLYS];
	int vertex_x [DSHAPE_POLYS] [DSHAPE_DISPLAY_VERTICES];
	int vertex_y [DSHAPE_POLYS] [DSHAPE_DISPLAY_VERTICES];
	int fill_source_x [DSHAPE_POLYS];
	int fill_source_y [DSHAPE_POLYS];
};

struct collision_mask_poly_struct collision_mask_poly [NSHAPES];

struct dshape_init_state_struct
{
	int current_dshape;
	int current_poly;

	int current_nshape;

	int last_display_vertex;
	al_fixed last_vertex_angle;
	al_fixed last_vertex_dist_fixed;
};
struct dshape_init_state_struct dshape_init;

static void start_dshape(int ds, int keyword_index);
static void start_dshape_poly(int poly, int layer, int poly_colour_level, int triangulation);
void add_collision_vertices_fixed(int num_vtx, const al_fixed* coords);
void add_vertices_fixed(int num_vtx, const al_fixed* coords);
void add_collision_vertices(int num_vtx, const int* coords);
void dshape_poly(int layer, int poly_colour_level, int triangulation, int num_vtx, const int* coords);
static void add_vertex(int x, int y, int add_collision_vertex);
static void add_vertex_vector(int angle, int dist, int add_collision_vertex);
//static void add_triple_link_vertex(int x, int y, int link_index);
//static void add_triple_link_vertex_vector(int angle, int dist, int previous_angle, int previous_angle_dist, int next_angle, int next_angle_dist, int link_index);
static void add_poly_fill_source(int x, int y);
//void add_link_vector(int angle, int dist);
//static void add_link_at_last_vertex(int link_index, int link_extra_distance, int object_extra_distance);
static void add_link_at_xy(int link_index, int link_x, int link_y, int left_x, int left_y, int right_x, int right_y, int far_x, int far_y, int link_point_x, int link_point_y, int object_x, int object_y);
void add_links(int num_links, const int* coords);
static void add_link_at_xy_fixed(int link_index, al_fixed link_x, al_fixed link_y, al_fixed left_x, al_fixed left_y, al_fixed right_x, al_fixed right_y, al_fixed far_x, al_fixed far_y, al_fixed link_point_x, al_fixed link_point_y, al_fixed object_x, al_fixed object_y);
static void add_link_vector(int link_index, int link_angle, int link_dist, int left_angle, int left_dist, int right_angle, int right_dist, int link_extra_distance, int object_extra_distance, int far_point_distance);
static void finish_shape(void);
static void calculate_mirrored_objects_for_mirror_axis(int axis_index, int front_vertex, int back_vertex);

void add_mirror_axis_at_link(int link_index, int opposite_link_index);
void add_mirror_axis(int axis_angle, int opposite_vertex);

//static void add_outline_vertex(float x, float y);
//static void add_outline_vertex_vector(float angle, float dist);
static void add_outline_vertex_at_last_poly_vertex(int extra_distance);
static void add_outline_vertex_at_xy(int x, int y);


static void draw_line_on_nshape_mask(int s, int level, al_fixed xa, al_fixed ya, al_fixed xb, al_fixed yb);
static void set_nshape_mask_pixel(int s, int level, int x, int y);
static void floodfill_nshape_mask(int s, int level, int x, int y);
void init_nshape_collision_masks(void);

void test_draw_mask(int s);


/*

Plan for shapes:
3 basic types:
core
core_static
component

core will tend to be elongated, but mostly symmetrical
core_static will be regular, with radial symmetry
 - much higher power rating for less cost
component can be anything

Next ones to implement:
core_static_4
core_static_5
core_static_6

core_


cost will be 8-12 per component, with cores costing more
cost for objects:
 - move: 2
 - packet: 2
 - packet_dir: 3
 - build: 16
 - harvest: 8?
 - allocate: 32
 - storage: 2 (or 1?)
 - interface: 3?
 - interface_depth: 3
 - interface_stability 3
 - link: 1 (up/down)
 - power+1: 2
 - power+2: 4 etc.

core_static_quad: 16
core_static_5: 20 ? (maybe no 5 for static?)
core_static_hex: 24

core (same as static?)

component:
4 links: 8
5: 10
6: 12

Cost affects:
- data cost
- inertia (should generally be 1-to-1)
- cooldown for build method

Hit points:
- probably just a flat 100 (64? 128?)
- however, certain components will be hardened. This will increase cost/inertia + increase hp + graphic difference

Need to work out what they'll look like.

- each core will have patterns on it that will pulse according to stress level
 - the pulse will start at the cycle time (although maybe not every cycle? that might be too fast)
 - stress level will determine the length of the pulse, and the starting brightness.



*/

const struct nshape_init_data_struct nshape_init_data [NSHAPES] =
{
// data cost, hp, core power, component power, interface charge rate
{UNLOCK_NONE, 8, 200, 100, 14, 2, 1024}, // NSHAPE_CORE_STATIC_QUAD,
{UNLOCK_NONE, 14, 300, 120, 18, 3, 1280}, // NSHAPE_CORE_STATIC_PENT,
{UNLOCK_NONE, 28, 400, 180, 22, 4, 1536}, // NSHAPE_CORE_STATIC_HEX_A,
{UNLOCK_CORE_STATIC_1, 64, 500, 240, 34, 5, 1792}, // NSHAPE_CORE_STATIC_HEX_B,
{UNLOCK_CORE_STATIC_2, 128, 600, 300, 56, 6, 2048}, // NSHAPE_CORE_STATIC_HEX_C,


{UNLOCK_NONE, 30, 50, 50, 10, 1, 1024}, // NSHAPE_CORE_QUAD_A,
{UNLOCK_NONE, 50, 70, 60, 16, 1, 1024}, // NSHAPE_CORE_QUAD_B,
{UNLOCK_NONE, 80, 100, 70, 24, 2, 1280}, // NSHAPE_CORE_PENT_A,
{UNLOCK_CORE_MOBILE_1, 100, 140, 80, 30, 2, 1280}, // NSHAPE_CORE_PENT_B,
{UNLOCK_CORE_MOBILE_1, 130, 180, 90, 36, 3, 1280}, // NSHAPE_CORE_PENT_C,
{UNLOCK_CORE_MOBILE_2, 180, 220, 100, 42, 3, 1536}, // NSHAPE_CORE_HEX_A,
{UNLOCK_CORE_MOBILE_3, 230, 260, 110, 48, 4, 1536}, // NSHAPE_CORE_HEX_B,
{UNLOCK_CORE_MOBILE_4, 300, 300, 120, 54, 5, 1536}, // NSHAPE_CORE_HEX_C,


/*
{20, 50, 50, 10, 1}, // NSHAPE_CORE_QUAD_A,
{40, 70, 80, 14, 1}, // NSHAPE_CORE_QUAD_B,
{60, 100, 120, 18, 2}, // NSHAPE_CORE_PENT_A,
{81, 140, 150, 22, 2}, // NSHAPE_CORE_PENT_B,
{100, 180, 180, 26, 3}, // NSHAPE_CORE_PENT_C,
{130, 220, 220, 36, 4}, // NSHAPE_CORE_HEX_A,
{160, 260, 240, 48, 4}, // NSHAPE_CORE_HEX_B,
{200, 300, 260, 56, 5}, // NSHAPE_CORE_HEX_C,

*/

/*
//{data_cost, hp_max, power_capacity, component_power_capacity
{8, 200, 100, 14}, // NSHAPE_CORE_STATIC_QUAD,
{14, 300, 160, 16, }, // NSHAPE_CORE_STATIC_PENT,
{28, 400, 240, 18}, // NSHAPE_CORE_STATIC_HEX_A,
{56, 500, 360, 20}, // NSHAPE_CORE_STATIC_HEX_B,
{92, 600, 560, 22}, // NSHAPE_CORE_STATIC_HEX_C,

{20, 60, 50, 10}, // NSHAPE_CORE_QUAD_A,
{32, 90, 80, 12}, // NSHAPE_CORE_QUAD_B,
{48, 120, 120, 14}, // NSHAPE_CORE_PENT_A,
{64, 150, 170, 16}, // NSHAPE_CORE_PENT_B,
{88, 180, 220, 18}, // NSHAPE_CORE_PENT_C,
{112, 200, 300, 20}, // NSHAPE_CORE_HEX_A,
{136, 220, 420, 21}, // NSHAPE_CORE_HEX_B,
{180, 250, 560, 22}, // NSHAPE_CORE_HEX_C,
*/


{UNLOCK_NONE, 6, 100, 0}, // NSHAPE_COMPONENT_TRI,
{UNLOCK_NONE, 6, 100, 0}, // NSHAPE_COMPONENT_FORK,
{UNLOCK_NONE, 9, 100, 0}, // NSHAPE_COMPONENT_BOX,
{UNLOCK_NONE, 9, 100, 0}, // NSHAPE_COMPONENT_LONG4,
{UNLOCK_NONE, 9, 100, 0}, // NSHAPE_COMPONENT_CAP,
{UNLOCK_NONE, 9, 100, 0}, // NSHAPE_COMPONENT_PRONG,
{UNLOCK_CORE_MOBILE_1, 12, 100, 0}, // NSHAPE_COMPONENT_LONG5,
{UNLOCK_CORE_STATIC_1, 12, 100, 0}, // NSHAPE_COMPONENT_PEAK,
{UNLOCK_CORE_MOBILE_1, 12, 100, 0}, // NSHAPE_COMPONENT_SNUB,
{UNLOCK_CORE_MOBILE_2, 12, 100, 0}, // NSHAPE_COMPONENT_BOWL,
{UNLOCK_CORE_MOBILE_3, 16, 100, 0}, // NSHAPE_COMPONENT_LONG6,
{UNLOCK_CORE_STATIC_2, 16, 100, 0}, // NSHAPE_COMPONENT_DROP,
{UNLOCK_CORE_MOBILE_4, 16, 100, 0}, // NSHAPE_COMPONENT_SIDE,



};
/*

Build and restore time -

- let's try just making it the total data cost, in cycles
 - reduced for static - halved?

- restore time:
 - can be the same

*/

void init_nshapes_and_dshapes(void)
{

#define LINK_EXTRA_DIST 14
#define OBJECT_EXTRA_DIST 8

#define NS_BASIC_DISP_LONG 24
#define NS_BASIC_DISP_MED 10
#define NS_BASIC_DISP_SHORT 6



int i;
int poly_index;


#ifdef Z_POLY
 zshape_init(); // this initialises the zshape code so it can be started below for a specific shape
#endif




#define COL_LEVEL_BASE PROC_COL_UNDERLAY
#define COL_LEVEL_MAIN PROC_COL_MAIN_1
#define COL_LEVEL_CORE PROC_COL_CORE_MUTABLE

// ******* NSHAPE_CORE_QUAD_A *******


 start_dshape(NSHAPE_CORE_QUAD_A, KEYWORD_CORE_QUAD_A);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int quad_a_links[] = {
		22, 0, 32, -9, 32, 9, 38, 0, 39, 0, 37, 0,
		0, 14, 12, 21, -11, 21, 0, 26, 0, 30, 0, 26,
		-32, 0, -41, 9, -41, -9, -49, 0, -49, 0, -47, 0,
		0, -14, -12, -21, 11, -21, 0, -26, 0, -30, 0, -26
	};
	add_links(4, quad_a_links);

	add_outline_vertex_at_xy(44, 0);
	add_outline_vertex_at_xy(0, 29);
	add_outline_vertex_at_xy(-54, 0);
	add_outline_vertex_at_xy(0, -29);

	const int quad_a_collision[] = {38, 0, 0, 23, -48, 0, 0, -23, 16, -21, 16, 21, -29, 20, -29, -20};
	add_collision_vertices(8, quad_a_collision);

	const int quad_a_underlay[] = {38, 0, 0, 23, -48, 0, 0, -23};
	dshape_poly(0, PROC_COL_UNDERLAY, TRI_FAN, 4, quad_a_underlay);
	add_poly_fill_source(-2, 0);

	const int quad_a_main0[] = {8, -1, 1, -8, 1, -12, 16, -21, 32, -12, 21, -1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, quad_a_main0);
	add_poly_fill_source(13, -9);

	const int quad_a_main1[] = {8, 1, 1, 8, 1, 12, 16, 21, 31, 12, 21, 1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, quad_a_main1);
	add_poly_fill_source(13, 9);

	const int quad_a_main2[] = {-8, 1, -1, 8, -1, 12, -18, 21, -29, 20, -35, 16, -41, 12, -31, 1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 8, quad_a_main2);
	add_poly_fill_source(-20, 11);

	const int quad_a_main3[] = {-8, -1, -1, -8, -1, -12, -18, -21, -29, -20, -35, -16, -41, -12, -31, -1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 8, quad_a_main3);
	add_poly_fill_source(-20, -11);

	const int quad_a_core[] = {0, 6, -6, 0, 0, -6, 6, 0};
	dshape_poly(1, PROC_COL_CORE_MUTABLE, TRI_FAN, 4, quad_a_core);
	add_poly_fill_source(0, 0);


 add_mirror_axis_at_link(0, 2);
 add_mirror_axis_at_link(1, 3);
 add_mirror_axis_at_link(2, 0);
 add_mirror_axis_at_link(3, 1);

 add_mirror_axis(0, -1);
 add_mirror_axis(ANGLE_2, -1);


	finish_shape();

// ******* NSHAPE_CORE_QUAD_B *******





 start_dshape(NSHAPE_CORE_QUAD_B, KEYWORD_CORE_QUAD_B);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int quad_b_links[] = {
		17, -13, 9, -23, 27, -8, 25, -21, 26, -22, 23, -19,
		17, 13, 27, 8, 9, 23, 25, 21, 26, 22, 23, 19,
		-11, 16, -8, 24, -28, 14, -19, 24, -20, 26, -18, 23,
		-11, -16, -28, -14, -8, -24, -19, -24, -20, -26, -18, -23
	};
	add_links(4, quad_b_links);

	const int quad_b_collision[] = {31, -4, 31, 3, -6, -26, 5, -26, -6, 26, 5, 26, -47, 7, -47, -7};
	add_collision_vertices(8, quad_b_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(27, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(22, 18, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-18, 21, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-37, 5, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-37, -5, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-18, -21, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(22, -18, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-5, 0);

	const int quad_b_main0[] = {31, -4, 31, 3, 18, 11, 11, 5, 11, -6, 18, -11};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, quad_b_main0);
	add_poly_fill_source(20, 0);

	const int quad_b_main1[] = {-6, -26, -9, -16, -1, -7, 9, -7, 16, -12, 5, -26};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, quad_b_main1);
	add_poly_fill_source(2, -15);

	const int quad_b_main2[] = {-6, 26, -9, 16, -1, 7, 9, 7, 16, 12, 5, 26};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, quad_b_main2);
	add_poly_fill_source(2, 15);

	const int quad_b_main3[] = {-47, 7, -47, -7, -34, -12, -11, -14, -3, -5, -3, 5, -11, 14, -34, 12};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 8, quad_b_main3);
	add_poly_fill_source(-23, 0);

	const int quad_b_core[] = {9, -5, 9, 5, -1, 5, -1, -5};
	dshape_poly(1, PROC_COL_CORE_MUTABLE, TRI_FAN, 4, quad_b_core);
	add_poly_fill_source(4, 0);

 add_mirror_axis(0, -1);
 add_mirror_axis(ANGLE_2, -1);


	finish_shape();



// ******* NSHAPE_CORE_HEX_A *******

 start_dshape(NSHAPE_CORE_HEX_A, KEYWORD_CORE_HEX_A);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int hex_a_links[] = {
		35, 0, 44, -7, 44, 7, 51, 0, 53, 0, 49, 0,
		13, 18, 25, 22, 7, 27, 18, 27, 20, 29, 17, 25,
		-13, 19, -8, 27, -29, 23, -21, 29, -23, 31, -21, 28,
		-38, 0, -47, 8, -47, -8, -54, 0, -56, 0, -52, 0,
		-13, -19, -29, -23, -8, -27, -21, -29, -23, -31, -21, -28,
		13, -18, 7, -27, 25, -22, 18, -27, 20, -29, 17, -25
	};
	add_links(6, hex_a_links);


	const int hex_a_collision[] = {-38, -24, -52, -14, -38, 24, -52, 14, 3, -29, -5, -29, 3, 29, -5, 29, 46, -11, 46, 11};
	add_collision_vertices(10, hex_a_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(48, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(16, 24, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-20, 23, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-51, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-20, -23, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(16, -24, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-1, 0);

	const int hex_a_core[] = {9, 0, 5, 5, -7, 5, -10, 0, -7, -5, 5, -5};
	dshape_poly(1, PROC_COL_CORE_MUTABLE, TRI_FAN, 6, hex_a_core);
	add_poly_fill_source(0, 0);

	const int hex_a_main0[] = {-14, -17, -38, -24, -52, -14, -37, -1, -12, -1, -8, -7};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, hex_a_main0);
	add_poly_fill_source(-26, -10);

	const int hex_a_main1[] = {-14, 17, -38, 24, -52, 14, -37, 1, -12, 1, -8, 7};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, hex_a_main1);
	add_poly_fill_source(-26, 10);

	const int hex_a_main2[] = {-6, -7, 4, -7, 12, -17, 3, -29, -5, -29, -12, -17};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, hex_a_main2);
	add_poly_fill_source(0, -17);

	const int hex_a_main3[] = {-6, 7, 4, 7, 12, 17, 3, 29, -5, 29, -12, 17};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, hex_a_main3);
	add_poly_fill_source(0, 17);

	const int hex_a_main4[] = {6, -7, 11, -1, 33, -1, 46, -11, 29, -21, 13, -16};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, hex_a_main4);
	add_poly_fill_source(23, -9);

	const int hex_a_main5[] = {6, 7, 11, 1, 33, 1, 46, 11, 29, 21, 13, 16};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, hex_a_main5);
	add_poly_fill_source(23, 9);

 add_mirror_axis_at_link(0, 3);
// add_mirror_axis_at_link(1, 4);
// add_mirror_axis_at_link(2, 5);
 add_mirror_axis_at_link(3, 0);
// add_mirror_axis_at_link(4, 1);
// add_mirror_axis_at_link(5, 2);

 finish_shape();

// ******* NSHAPE_CORE_HEX_B *********

 start_dshape(NSHAPE_CORE_HEX_B, KEYWORD_CORE_HEX_B);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int hex_b_links[] = {
		35, 0, 44, -7, 44, 7, 51, 0, 53, 0, 49, 0,
		12, 21, 25, 22, 6, 28, 18, 28, 20, 31, 18, 28,
		-12, 21, -6, 28, -27, 24, -17, 31, -18, 34, -17, 30,
		-38, 0, -47, 8, -47, -8, -54, 0, -56, 0, -52, 0,
		-12, -21, -27, -24, -6, -28, -17, -31, -18, -34, -17, -30,
		12, -21, 6, -28, 25, -22, 18, -28, 20, -31, 18, -28
	};
	add_links(6, hex_b_links);


	const int hex_b_collision[] = {48, 0, -51, 0, 3, -29, -3, -29, 3, 29, -3, 29, 46, -11, 29, -21, 46, 11, 29, 21};
	add_collision_vertices(10, hex_b_collision);
	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(48, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(16, 25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-20, 25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-51, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-20, -25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(16, -25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-1, 0);

	const int hex_b_core[] = {8, 0, 3, 5, -4, 5, -9, 0, -4, -5, 3, -5};
	dshape_poly(1, PROC_COL_CORE_MUTABLE, TRI_FAN, 6, hex_b_core);
	add_poly_fill_source(0, 0);

	const int hex_b_main0[] = {-3, -7, 3, -7, 11, -19, 3, -29, -3, -29, -11, -19};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, hex_b_main0);
	add_poly_fill_source(0, -18);

	const int hex_b_main1[] = {-3, 7, 3, 7, 11, 19, 3, 29, -3, 29, -11, 19};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, hex_b_main1);
	add_poly_fill_source(0, 18);

	const int hex_b_main2[] = {5, -6, 10, -1, 33, -1, 46, -11, 29, -21, 13, -19};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, hex_b_main2);
	add_poly_fill_source(22, -9);

	const int hex_b_main3[] = {5, 6, 10, 1, 33, 1, 46, 11, 29, 21, 13, 19};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, hex_b_main3);
	add_poly_fill_source(22, 9);

	const int hex_b_ecore[] = {-15, -4, -11, 0, -15, 4, -18, 4, -22, 0, -18, -4};
	dshape_poly(1, PROC_COL_CORE_MUTABLE, TRI_FAN, 6, hex_b_ecore);
	add_poly_fill_source(-16, 0);

	const int hex_b_main4[] = {-13, -19, -5, -7, -10, -2, -15, -6, -18, -6, -24, -1, -37, -1, -48, -12, -33, -23};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 9, hex_b_main4);
	add_poly_fill_source(-22, -8);

	const int hex_b_main5[] = {-13, 19, -5, 7, -10, 2, -15, 6, -18, 6, -24, 1, -37, 1, -48, 12, -33, 23};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 9, hex_b_main5);
	add_poly_fill_source(-22, 8);

 add_mirror_axis_at_link(0, 3);
// add_mirror_axis_at_link(1, 4);
// add_mirror_axis_at_link(2, 5);
 add_mirror_axis_at_link(3, 0);
// add_mirror_axis_at_link(4, 1);
// add_mirror_axis_at_link(5, 2);

	finish_shape();


// ******* NSHAPE_CORE_HEX_C *********

 start_dshape(NSHAPE_CORE_HEX_C, KEYWORD_CORE_HEX_C);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int hex_c_links[] = {
		42, 0, 49, -9, 49, 9, 55, 0, 58, 0, 55, 0,
		16, 22, 28, 23, 8, 32, 19, 31, 21, 34, 19, 31,
		-14, 22, -8, 32, -27, 25, -19, 31, -21, 34, -19, 31,
		-38, 0, -47, 8, -47, -8, -54, 0, -56, 0, -52, 0,
		-14, -22, -27, -25, -8, -32, -19, -31, -21, -34, -19, -31,
		16, -22, 8, -32, 28, -23, 19, -31, 21, -34, 19, -31,
	};
	add_links(6, hex_c_links);

	const int hex_c_collision[] = {4, -35, -4, -35, 4, 35, -4, 35, -51, -15, -38, -26, -51, 15, -38, 26, 51, -15, 51, 15};
	add_collision_vertices(10, hex_c_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(53, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(17, 28, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-17, 28, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-51, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-17, -28, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(17, -28, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(0, 0);

	const int hex_c_core[] = {9, 0, 4, 5, -4, 5, -9, 0, -4, -5, 4, -5};
	dshape_poly(1, PROC_COL_CORE_MUTABLE, TRI_FAN, 6, hex_c_core);
	add_poly_fill_source(0, 0);

	const int hex_c_main0[] = {-3, -7, 3, -7, 15, -20, 4, -35, -4, -35, -13, -20};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, hex_c_main0);
	add_poly_fill_source(0, -20);

	const int hex_c_main1[] = {-3, 7, 3, 7, 15, 20, 4, 35, -4, 35, -13, 20};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, hex_c_main1);
	add_poly_fill_source(0, 20);

	const int hex_c_ecore0[] = {-15, -4, -11, 0, -15, 4, -18, 4, -22, 0, -18, -4};
	dshape_poly(1, PROC_COL_CORE_MUTABLE, TRI_FAN, 6, hex_c_ecore0);
	add_poly_fill_source(-16, 0);

	const int hex_c_main2[] = {-15, -20, -5, -7, -10, -2, -15, -6, -18, -6, -24, -1, -37, -1, -51, -15, -38, -26};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 9, hex_c_main2);
	add_poly_fill_source(-23, -9);

	const int hex_c_main3[] = {-15, 20, -5, 7, -10, 2, -15, 6, -18, 6, -24, 1, -37, 1, -51, 15, -38, 26};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 9, hex_c_main3);
	add_poly_fill_source(-23, 9);

	const int hex_c_main4[] = {17, -20, 5, -7, 10, -2, 15, -6, 18, -6, 24, -1, 40, -1, 51, -15, 33, -22};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 9, hex_c_main4);
	add_poly_fill_source(23, -8);

	const int hex_c_main5[] = {17, 20, 5, 7, 10, 2, 15, 6, 18, 6, 24, 1, 40, 1, 51, 15, 33, 22};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 9, hex_c_main5);
	add_poly_fill_source(23, 8);

	const int hex_c_ecore1[] = {15, -4, 11, 0, 15, 4, 18, 4, 22, 0, 18, -4};
	dshape_poly(1, PROC_COL_CORE_MUTABLE, TRI_FAN, 6, hex_c_ecore1);
	add_poly_fill_source(16, 0);

 add_mirror_axis_at_link(0, 3);
// add_mirror_axis_at_link(1, 4);
// add_mirror_axis_at_link(2, 5);
 add_mirror_axis_at_link(3, 0);
// add_mirror_axis_at_link(4, 1);
// add_mirror_axis_at_link(5, 2);

	finish_shape();



// ******* NSHAPE_CORE_PENT_C *******

 start_dshape(NSHAPE_CORE_PENT_C, KEYWORD_CORE_PENT_C);


	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int pent_c_links[] = {
		33, 0, 41, -7, 41, 7, 45, 0, 47, 0, 45, 0,
		0, 19, 7, 26, -10, 24, -3, 30, -4, 33, -3, 29,
		-22, 10, -23, 18, -35, 6, -33, 15, -37, 17, -33, 14,
		-22, -10, -35, -6, -23, -18, -33, -15, -37, -17, -33, -14,
		0, -19, -10, -24, 7, -26, -3, -30, -4, -33, -3, -29
	};
	add_links(5, pent_c_links);

	const int pent_c_collision[] = {10, -28, 42, -10, 10, 28, 42, 10, -19, -27, -19, 27, -42, 3, -42, -3};
	add_collision_vertices(8, pent_c_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(44, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-2, 25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-27, 14, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-35, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-27, -14, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-2, -25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-8, 0);

	const int pent_c_main0[] = {8, -1, 1, -7, 1, -18, 10, -28, 42, -10, 32, -1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_c_main0);
	add_poly_fill_source(15, -10);

	const int pent_c_main1[] = {8, 1, 1, 7, 1, 18, 10, 28, 42, 10, 32, 1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_c_main1);
	add_poly_fill_source(15, 10);

	const int pent_c_main2[] = {-1, -7, -1, -18, -19, -27, -23, -24, -21, -10, -7, -4};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_c_main2);
	add_poly_fill_source(-12, -15);

	const int pent_c_main3[] = {-1, 7, -1, 18, -19, 27, -23, 24, -21, 10, -7, 4};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_c_main3);
	add_poly_fill_source(-12, 15);

	const int pent_c_main4[] = {-8, 3, -21, 9, -42, 3, -42, -3, -21, -9, -8, -3};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_c_main4);
	add_poly_fill_source(-23, 0);

	const int pent_c_core[] = {0, 6, -6, 3, -6, -3, 0, -6, 6, 0};
	dshape_poly(1, PROC_COL_CORE_MUTABLE, TRI_FAN, 5, pent_c_core);
	add_poly_fill_source(-1, 0);

 add_mirror_axis_at_link(0, -1);
 add_mirror_axis(ANGLE_2, 0);

	finish_shape();

// ******* NSHAPE_CORE_PENT_A *******

 start_dshape(NSHAPE_CORE_PENT_A, KEYWORD_CORE_PENT_A);


	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int pent_a_links[] = {
		26, 0, 31, -7, 31, 7, 38, 0, 41, 0, 37, 0,
		3, 21, 18, 17, -4, 27, 9, 28, 10, 30, 8, 27,
		-21, 17, -16, 28, -35, 12, -26, 24, -28, 25, -26, 23,
		-21, -17, -35, -12, -16, -28, -26, -24, -28, -25, -26, -23,
		3, -21, -4, -27, 18, -17, 9, -28, 10, -30, 8, -27
	};
	add_links(5, pent_a_links);

	const int pent_a_collision[] = {30, -9, 30, 9, -8, -28, -14, -28, -8, 28, -14, 28, -50, 5, -50, -5};
	add_collision_vertices(8, pent_a_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(34, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(5, 25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-23, 23, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-44, -1, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-23, -23, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(5, -25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-7, 0);

	const int pent_a_main0[] = {8, -1, 1, -8, 4, -19, 22, -14, 30, -9, 24, -1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_a_main0);
	add_poly_fill_source(14, -8);

	const int pent_a_main1[] = {8, 1, 1, 8, 4, 19, 22, 14, 30, 9, 24, 1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_a_main1);
	add_poly_fill_source(14, 8);

	const int pent_a_main2[] = {0, -9, 2, -19, -8, -28, -14, -28, -19, -16, -8, -4};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_a_main2);
	add_poly_fill_source(-7, -17);

	const int pent_a_main3[] = {0, 9, 2, 19, -8, 28, -14, 28, -19, 16, -8, 4};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_a_main3);
	add_poly_fill_source(-7, 17);

	const int pent_a_main4[] = {-9, 3, -21, 15, -50, 5, -50, -5, -21, -15, -9, -3};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_a_main4);
	add_poly_fill_source(-26, 0);

	const int pent_a_core[] = {0, 7, -7, 3, -7, -3, 0, -7, 7, 0};
	dshape_poly(1, PROC_COL_CORE_MUTABLE, TRI_FAN, 5, pent_a_core);
	add_poly_fill_source(-1, 0);

 add_mirror_axis_at_link(0, -1);
 add_mirror_axis(ANGLE_2, 0);

	finish_shape();


// ******* NSHAPE_CORE_PENT_B *******

 start_dshape(NSHAPE_CORE_PENT_B, KEYWORD_CORE_PENT_B);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int pent_b_links[] = {
		27, 0, 33, -8, 33, 8, 38, 0, 40, 0, 36, 0,
		3, 20, 16, 20, -2, 31, 10, 32, 11, 33, 9, 29,
		-18, 17, -15, 25, -34, 14, -29, 27, -30, 28, -27, 24,
		-18, -17, -34, -14, -15, -25, -29, -27, -30, -28, -27, -24,
		3, -20, -2, -31, 16, -20, 10, -32, 11, -33, 9, -29
	};
	add_links(5, pent_b_links);

	const int pent_b_collision[] = {33, -10, 33, 10, -4, -32, -13, -27, -4, 32, -13, 27, -40, 11, -48, 0, -40, -11};
	add_collision_vertices(9, pent_b_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(33, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(6, 29, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-26, 22, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-34, 7, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-34, -7, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-26, -22, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(6, -29, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-10, 0);

	const int pent_b_main0[] = {7, -1, -1, -7, 5, -19, 31, -18, 33, -10, 26, -1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_b_main0);
	add_poly_fill_source(16, -9);

	const int pent_b_main1[] = {7, 1, -1, 7, 5, 19, 31, 18, 33, 10, 26, 1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_b_main1);
	add_poly_fill_source(16, 9);

	const int pent_b_main2[] = {-2, -8, 2, -19, -4, -32, -13, -27, -16, -17, -8, -4};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_b_main2);
	add_poly_fill_source(-6, -17);

	const int pent_b_main3[] = {-2, 8, 2, 19, -4, 32, -13, 27, -16, 17, -8, 4};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, pent_b_main3);
	add_poly_fill_source(-6, 17);

	const int pent_b_main4[] = {-9, 3, -18, 16, -40, 11, -48, 0, -40, -11, -18, -16, -9, -3};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 7, pent_b_main4);
	add_poly_fill_source(-26, 0);

	const int pent_b_core[] = {-2, 6, -7, 3, -7, -3, -2, -6, 6, 0};
	dshape_poly(1, PROC_COL_CORE_MUTABLE, TRI_FAN, 5, pent_b_core);
	add_poly_fill_source(-2, 0);

 add_mirror_axis_at_link(0, -1);
 add_mirror_axis(ANGLE_2, 0);

	finish_shape();



// ******* NSHAPE_CORE_STATIC_QUAD *******

 start_dshape(NSHAPE_CORE_STATIC_QUAD, KEYWORD_CORE_STATIC_QUAD);

#define STATIC_QUAD_BASE_SIZE 19
#define STATIC_QUAD_LINK_EDGE_DIST 8
#define STATIC_QUAD_LINK_EDGE_ADJUST 3

// front
 add_link_at_xy(0, // link_index
																STATIC_QUAD_BASE_SIZE + 3, 0, // centre
																STATIC_QUAD_BASE_SIZE + STATIC_QUAD_LINK_EDGE_DIST, -STATIC_QUAD_LINK_EDGE_DIST-STATIC_QUAD_LINK_EDGE_ADJUST, // left
																STATIC_QUAD_BASE_SIZE + STATIC_QUAD_LINK_EDGE_DIST, STATIC_QUAD_LINK_EDGE_DIST+STATIC_QUAD_LINK_EDGE_ADJUST, // right
																34, 0, // far
																40, 0, // link
																35, 0); // object

// right
 add_link_at_xy(1, // link_index
																0, STATIC_QUAD_BASE_SIZE + 3, // centre
																STATIC_QUAD_LINK_EDGE_DIST+STATIC_QUAD_LINK_EDGE_ADJUST, STATIC_QUAD_BASE_SIZE + STATIC_QUAD_LINK_EDGE_DIST, // left
																-STATIC_QUAD_LINK_EDGE_DIST-STATIC_QUAD_LINK_EDGE_ADJUST, STATIC_QUAD_BASE_SIZE + STATIC_QUAD_LINK_EDGE_DIST, // right
																0, 34, // far
																0, 40, // link
																0, 35); // object


// back
 add_link_at_xy(2, // link_index
																-STATIC_QUAD_BASE_SIZE - 3, 0, // centre
																-STATIC_QUAD_BASE_SIZE - STATIC_QUAD_LINK_EDGE_DIST, STATIC_QUAD_LINK_EDGE_DIST+STATIC_QUAD_LINK_EDGE_ADJUST, // left
																-STATIC_QUAD_BASE_SIZE - STATIC_QUAD_LINK_EDGE_DIST, -STATIC_QUAD_LINK_EDGE_DIST-STATIC_QUAD_LINK_EDGE_ADJUST, // right
																-34, 0, // far
																-40, 0, // link
																-35, 0); // object

// left
 add_link_at_xy(3, // link_index
																0, -STATIC_QUAD_BASE_SIZE - 3, // centre
																-STATIC_QUAD_LINK_EDGE_DIST-STATIC_QUAD_LINK_EDGE_ADJUST, -STATIC_QUAD_BASE_SIZE - STATIC_QUAD_LINK_EDGE_DIST, // left
																STATIC_QUAD_LINK_EDGE_DIST+STATIC_QUAD_LINK_EDGE_ADJUST, -STATIC_QUAD_BASE_SIZE - STATIC_QUAD_LINK_EDGE_DIST, // right
																0, -34, // far
																0, -40, // link
																0, -35); // object


 poly_index = POLY_0;
 start_dshape_poly(poly_index++, 0, COL_LEVEL_BASE, TRI_WALK);

 add_vertex_vector(0 * ANGLE_4, STATIC_QUAD_BASE_SIZE + 16, 1);
 add_outline_vertex_at_last_poly_vertex(6);
 add_vertex_vector(1 * ANGLE_4, STATIC_QUAD_BASE_SIZE + 16, 1);
 add_outline_vertex_at_last_poly_vertex(6);
 add_vertex_vector(2 * ANGLE_4, STATIC_QUAD_BASE_SIZE + 16, 1);
 add_outline_vertex_at_last_poly_vertex(6);
 add_vertex_vector(3 * ANGLE_4, STATIC_QUAD_BASE_SIZE + 16, 1);
 add_outline_vertex_at_last_poly_vertex(6);

 add_poly_fill_source(0, 0);
/*
 nshape[NSHAPE_CORE_STATIC_QUAD].mirrored_object [0] [0] = 0;
 nshape[NSHAPE_CORE_STATIC_QUAD].mirrored_object [0] [1] = 1;
 nshape[NSHAPE_CORE_STATIC_QUAD].mirrored_object [0] [2] = 2;
 nshape[NSHAPE_CORE_STATIC_QUAD].mirrored_object [0] [3] = 3;*/

 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

// centre bit:
 start_dshape_poly(poly_index++, 1, COL_LEVEL_CORE, TRI_WALK);
 add_vertex(6, 0, 0);
 add_vertex(0, 6, 0);
 add_vertex(-6, 0, 0);
 add_vertex(0, -6, 0);
 add_poly_fill_source(0, 0);

#define STATIC_QUAD_OUTER_SIZE 20
#define STATIC_QUAD_EDGE_SIZE 8

// down-right quadrant
 start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN, TRI_WALK);
 add_vertex(1, 8, 0);
 add_vertex(8, 1, 0);
 add_vertex(STATIC_QUAD_OUTER_SIZE, 1, 0);
// add_vertex(STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, STATIC_QUAD_EDGE_SIZE + 1, 1); // collision
 add_vertex(STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, STATIC_QUAD_EDGE_SIZE + 8, 1); // collision
 add_vertex(STATIC_QUAD_EDGE_SIZE + 8, STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, 1); // collision
// add_vertex(STATIC_QUAD_EDGE_SIZE + 1, STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, 1); // collision
 add_vertex(1, STATIC_QUAD_OUTER_SIZE, 0);
 add_poly_fill_source(8, 8);

// down-left quadrant
 start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN, TRI_WALK);
 add_vertex(-1, 8, 0);
 add_vertex(-8, 1, 0);
 add_vertex(-STATIC_QUAD_OUTER_SIZE, 1, 0);
// add_vertex(-STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, STATIC_QUAD_EDGE_SIZE + 1, 1); // collision
 add_vertex(-STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, STATIC_QUAD_EDGE_SIZE + 8, 1); // collision
 add_vertex(-STATIC_QUAD_EDGE_SIZE - 8, STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, 1); // collision
// add_vertex(-STATIC_QUAD_EDGE_SIZE - 1, STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, 1); // collision
 add_vertex(-1, STATIC_QUAD_OUTER_SIZE, 0);
 add_poly_fill_source(-8, 8);

// up-left quadrant
 start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN, TRI_WALK);
 add_vertex(-1, -8, 0);
 add_vertex(-8, -1, 0);
 add_vertex(-STATIC_QUAD_OUTER_SIZE, -1, 0);
// add_vertex(-STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, -STATIC_QUAD_EDGE_SIZE - 1, 1); // collision
 add_vertex(-STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, -STATIC_QUAD_EDGE_SIZE - 8, 1); // collision
 add_vertex(-STATIC_QUAD_EDGE_SIZE - 8, -STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, 1); // collision
// add_vertex(-STATIC_QUAD_EDGE_SIZE - 1, -STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, 1); // collision
 add_vertex(-1, -STATIC_QUAD_OUTER_SIZE, 0);
 add_poly_fill_source(-8, -8);

// up-right quadrant
 start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN, TRI_WALK);
 add_vertex(1, -8, 0);
 add_vertex(8, -1, 0);
 add_vertex(STATIC_QUAD_OUTER_SIZE, -1, 0);
// add_vertex(STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, -STATIC_QUAD_EDGE_SIZE - 1, 1); // collision
 add_vertex(STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, -STATIC_QUAD_EDGE_SIZE - 8, 1); // collision
 add_vertex(STATIC_QUAD_EDGE_SIZE + 8, -STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, 1); // collision
// add_vertex(STATIC_QUAD_EDGE_SIZE + 1, -STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, 1); // collision
 add_vertex(1, -STATIC_QUAD_OUTER_SIZE, 0);
 add_poly_fill_source(8, -8);

 add_mirror_axis(ANGLE_8, -1); // -1 means that no vertex is opposite this axis
 add_mirror_axis((ANGLE_4) + ANGLE_8, -1);
 add_mirror_axis((2*ANGLE_4) + ANGLE_8, -1);
 add_mirror_axis((3*ANGLE_4) + ANGLE_8, -1);

 add_mirror_axis_at_link(0, 2);
 add_mirror_axis_at_link(1, 3);
 add_mirror_axis_at_link(2, 0);
 add_mirror_axis_at_link(3, 1);

 finish_shape();


// ******* NSHAPE_CORE_STATIC_PENT *******

 start_dshape(NSHAPE_CORE_STATIC_PENT, KEYWORD_CORE_STATIC_PENT);

#define STATIC_PENT_BASE_SIZE 30

 for (i = 0; i < 5; i ++)
	{
		add_link_vector(i,
																		ANGLE_5 * i, STATIC_PENT_BASE_SIZE - 2,
																		-ANGLE_8 - 150, 13,  // left
																		ANGLE_8 + 150, 13, // right
																		18,14,14);

	}


 poly_index = POLY_0;
 start_dshape_poly(poly_index++, 0, COL_LEVEL_BASE, TRI_WALK);

 for (i = 0; i < 5; i ++)
	{
  add_vertex_vector(i * ANGLE_5, STATIC_PENT_BASE_SIZE + 6, 1);
  add_outline_vertex_at_last_poly_vertex(6);
	}
 add_poly_fill_source(0, 0);

 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

// centre bit:
 start_dshape_poly(poly_index++, 1, COL_LEVEL_CORE, TRI_WALK);

 for (i = 0; i < 5; i ++)
	{
  add_vertex_vector(i * ANGLE_5, 7, 0);
	}
 add_poly_fill_source(0, 0);

 for (i = 0; i < 5; i ++)
	{
  start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN, TRI_WALK);
  int base_angle;
  base_angle = i * ANGLE_5;
  add_vertex_vector(base_angle + 200, 9, 0);
  add_vertex_vector(base_angle + 50, STATIC_PENT_BASE_SIZE - 3, 0);
  add_vertex_vector(base_angle + ANGLE_16, STATIC_PENT_BASE_SIZE + 9, 1);
  add_vertex_vector(base_angle + ANGLE_5 - ANGLE_16, STATIC_PENT_BASE_SIZE + 9, 1);
  add_vertex_vector(base_angle + ANGLE_5 - 50, STATIC_PENT_BASE_SIZE - 3, 1);
  add_vertex_vector(base_angle + ANGLE_5 - 200, 9, 0);
  add_poly_fill_source(xpart(base_angle + ANGLE_16 + 20, 12), ypart(base_angle + ANGLE_16 + 20, 12)); // xpart/ypart using floating point, but that's okay for poly_fill_source

	}


 add_mirror_axis(ANGLE_10, 3);
 add_mirror_axis((ANGLE_5) + ANGLE_10, 4);
 add_mirror_axis((2*ANGLE_5) + ANGLE_10, 0);
 add_mirror_axis((3*ANGLE_5) + ANGLE_10, 1);
 add_mirror_axis((4*ANGLE_5) + ANGLE_10, 2);

 add_mirror_axis_at_link(0, -1); // no links have links directly opposite
 add_mirror_axis_at_link(1, -1);
 add_mirror_axis_at_link(2, -1);
 add_mirror_axis_at_link(3, -1);
 add_mirror_axis_at_link(4, -1);
/*
 nshape[NSHAPE_CORE_STATIC_PENT].mirrored_object [0] [0] = 0;
 nshape[NSHAPE_CORE_STATIC_PENT].mirrored_object [0] [1] = 1;
 nshape[NSHAPE_CORE_STATIC_PENT].mirrored_object [0] [2] = 2;
 nshape[NSHAPE_CORE_STATIC_PENT].mirrored_object [0] [3] = 3;
 nshape[NSHAPE_CORE_STATIC_PENT].mirrored_object [0] [4] = 4;*/

 dshape[dshape_init.current_dshape].outline_base_vertex = 0;
 finish_shape();


// ******* NSHAPE_CORE_STATIC_HEX_A *******

 start_dshape(NSHAPE_CORE_STATIC_HEX_A, KEYWORD_CORE_STATIC_HEX_A);

#define STATIC_HEX_BASE_SIZE 30

 for (i = 0; i < 6; i ++)
	{
		add_link_vector(i,
																		ANGLE_6 * i, STATIC_HEX_BASE_SIZE - 2,
																		-ANGLE_8 - 150, 12,  // left
																		ANGLE_8 + 150, 12, // right
																		18,14,14);

	}


 poly_index = POLY_0;
 start_dshape_poly(poly_index++, 0, COL_LEVEL_BASE, TRI_WALK);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, STATIC_HEX_BASE_SIZE + 6, 1);
  add_outline_vertex_at_last_poly_vertex(6);
	}
 add_poly_fill_source(0, 0);

 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

// centre bit:
 start_dshape_poly(poly_index++, 1, COL_LEVEL_CORE, TRI_WALK);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, 7, 0);
	}
 add_poly_fill_source(0, 0);

 for (i = 0; i < 6; i ++)
	{
  start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN, TRI_WALK);
  int base_angle;
  base_angle = i * ANGLE_6;
  add_vertex_vector(base_angle + 200, 9, 0);
  add_vertex_vector(base_angle + 50, STATIC_HEX_BASE_SIZE - 3, 0);
  add_vertex_vector(base_angle + ANGLE_16, STATIC_HEX_BASE_SIZE + 9, 1);
  add_vertex_vector(base_angle + ANGLE_6 - ANGLE_16, STATIC_HEX_BASE_SIZE + 9, 1);
  add_vertex_vector(base_angle + ANGLE_6 - 50, STATIC_HEX_BASE_SIZE - 3, 1);
  add_vertex_vector(base_angle + ANGLE_6 - 200, 9, 0);
  add_poly_fill_source(xpart(base_angle + ANGLE_16 + 20, 12), ypart(base_angle + ANGLE_16 + 20, 12)); // xpart/ypart using floating point, but that's okay for poly_fill_source

	}

// static hex cores probably shouldn't have any mirror axes

 add_mirror_axis(ANGLE_12, -1);
 add_mirror_axis((ANGLE_6) + ANGLE_12, -1);
 add_mirror_axis((2*ANGLE_6) + ANGLE_12, -1);
 add_mirror_axis((3*ANGLE_6) + ANGLE_12, -1);
 add_mirror_axis((4*ANGLE_6) + ANGLE_12, -1);
 add_mirror_axis((5*ANGLE_6) + ANGLE_12, -1);

 add_mirror_axis_at_link(0, 3);
 add_mirror_axis_at_link(1, 4);
 add_mirror_axis_at_link(2, 5);
 add_mirror_axis_at_link(3, 0);
 add_mirror_axis_at_link(4, 1);
 add_mirror_axis_at_link(5, 2);

 finish_shape();



// ******* NSHAPE_CORE_STATIC_HEX_B *******

 start_dshape(NSHAPE_CORE_STATIC_HEX_B, KEYWORD_CORE_STATIC_HEX_B);

#define STATIC_HEX_B_BASE_SIZE 30

 for (i = 0; i < 6; i ++)
	{
		int left_angle = 150;
		int right_angle = 150;
		if (i == 1 || i == 4)
			right_angle = 10;
		if (i == 2 || i == 5)
			left_angle = 10;

		add_link_vector(i,
																		ANGLE_6 * i, STATIC_HEX_B_BASE_SIZE - 2,
																		-ANGLE_8 - left_angle, 12,  // left
																		ANGLE_8 + right_angle, 12, // right
																		18,14,14);

	}


 poly_index = POLY_0;
 start_dshape_poly(poly_index++, 0, COL_LEVEL_BASE, TRI_WALK);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, STATIC_HEX_B_BASE_SIZE + 6, 1);
  add_outline_vertex_at_last_poly_vertex(6);
	}
 add_poly_fill_source(0, 0);

 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

// centre bit:
 start_dshape_poly(poly_index++, 1, COL_LEVEL_CORE, TRI_WALK);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, 9, 0);
	}
 add_poly_fill_source(0, 0);

 for (i = 0; i < 6; i ++)
	{
  start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN, TRI_WALK);
  int base_angle;
  base_angle = i * ANGLE_6;
  add_vertex_vector(base_angle + 150, 11, 0);
  add_vertex_vector(base_angle + 50, STATIC_HEX_B_BASE_SIZE - 3, 0);
  if (i == 1 || i == 4)
		{
   add_vertex_vector(base_angle + ANGLE_16, STATIC_HEX_B_BASE_SIZE + 15, 1);
   add_vertex_vector(base_angle + ANGLE_6 - ANGLE_16, STATIC_HEX_B_BASE_SIZE + 15, 1);
		}
		 else
			{
    add_vertex_vector(base_angle + ANGLE_16, STATIC_HEX_B_BASE_SIZE + 9, 1);
    add_vertex_vector(base_angle + ANGLE_6 - ANGLE_16, STATIC_HEX_B_BASE_SIZE + 9, 1);
			}
  add_vertex_vector(base_angle + ANGLE_6 - 50, STATIC_HEX_B_BASE_SIZE - 3, 1);
  add_vertex_vector(base_angle + ANGLE_6 - 150, 11, 0);
  add_poly_fill_source(xpart(base_angle + ANGLE_16 + 20, 12), ypart(base_angle + ANGLE_16 + 20, 12)); // xpart/ypart using floating point, but that's okay for poly_fill_source

	}

 add_mirror_axis(ANGLE_12, -1);
 add_mirror_axis((ANGLE_6) + ANGLE_12, -1);
 add_mirror_axis((2*ANGLE_6) + ANGLE_12, -1);
 add_mirror_axis((3*ANGLE_6) + ANGLE_12, -1);
 add_mirror_axis((4*ANGLE_6) + ANGLE_12, -1);
 add_mirror_axis((5*ANGLE_6) + ANGLE_12, -1);

 add_mirror_axis_at_link(0, 3);
 add_mirror_axis_at_link(1, 4);
 add_mirror_axis_at_link(2, 5);
 add_mirror_axis_at_link(3, 0);
 add_mirror_axis_at_link(4, 1);
 add_mirror_axis_at_link(5, 2);

 finish_shape();


// ******* NSHAPE_CORE_STATIC_HEX_C *******

 start_dshape(NSHAPE_CORE_STATIC_HEX_C, KEYWORD_CORE_STATIC_HEX_C);

#define STATIC_HEX_C_BASE_SIZE 30

 for (i = 0; i < 6; i ++)
	{
		int left_angle = 150;
		int right_angle = 150;
		if (i == 1 || i == 3 || i == 5)
			right_angle = 20;
		if (i == 0 || i == 2 || i == 4)
			left_angle = 20;

		add_link_vector(i,
																		ANGLE_6 * i, STATIC_HEX_C_BASE_SIZE - 2,
																		-ANGLE_8 - left_angle, 12,  // left
																		ANGLE_8 + right_angle, 12, // right
																		18,14,14);

	}


 poly_index = POLY_0;
 start_dshape_poly(poly_index++, 0, COL_LEVEL_BASE, TRI_WALK);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, STATIC_HEX_C_BASE_SIZE + 6, 1);
  add_outline_vertex_at_last_poly_vertex(6);
	}
 add_poly_fill_source(0, 0);

 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

// centre bit:
 start_dshape_poly(poly_index++, 1, COL_LEVEL_CORE, TRI_WALK);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, 12, 0);
	}
 add_poly_fill_source(0, 0);
/*
 start_dshape_poly(poly_index++, 1, COL_LEVEL_BASE, TRI_WALK);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, 6, 0);
	}
 add_poly_fill_source(0, 0);
 fix_display_triangles_walk();*/

 for (i = 0; i < 6; i ++)
	{
  start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN, TRI_WALK);
  int base_angle;
  base_angle = i * ANGLE_6;
  add_vertex_vector(base_angle + 120, 14, 0);
  add_vertex_vector(base_angle + 50, STATIC_HEX_C_BASE_SIZE - 3, 0);
  if (i == 1 || i == 3 || i == 5)
		{
   add_vertex_vector(base_angle + ANGLE_16, STATIC_HEX_C_BASE_SIZE + 15, 1);
   add_vertex_vector(base_angle + ANGLE_6 - ANGLE_16, STATIC_HEX_C_BASE_SIZE + 15, 1);
		}
		 else
			{
    add_vertex_vector(base_angle + ANGLE_16, STATIC_HEX_C_BASE_SIZE + 9, 1);
    add_vertex_vector(base_angle + ANGLE_6 - ANGLE_16, STATIC_HEX_C_BASE_SIZE + 9, 1);
			}
  add_vertex_vector(base_angle + ANGLE_6 - 50, STATIC_HEX_C_BASE_SIZE - 3, 1);
  add_vertex_vector(base_angle + ANGLE_6 - 120, 14, 0);
  add_poly_fill_source(xpart(base_angle + ANGLE_16 + 20, 16), ypart(base_angle + ANGLE_16 + 20, 16)); // xpart/ypart using floating point, but that's okay for poly_fill_source

	}

 add_mirror_axis(ANGLE_12, -1);
 add_mirror_axis((ANGLE_6) + ANGLE_12, -1);
 add_mirror_axis((2*ANGLE_6) + ANGLE_12, -1);
 add_mirror_axis((3*ANGLE_6) + ANGLE_12, -1);
 add_mirror_axis((4*ANGLE_6) + ANGLE_12, -1);
 add_mirror_axis((5*ANGLE_6) + ANGLE_12, -1);

 add_mirror_axis_at_link(0, 3);
 add_mirror_axis_at_link(1, 4);
 add_mirror_axis_at_link(2, 5);
 add_mirror_axis_at_link(3, 0);
 add_mirror_axis_at_link(4, 1);
 add_mirror_axis_at_link(5, 2);

 finish_shape();

// ******* NSHAPE_COMPONENT_LONG6 *******


 start_dshape(NSHAPE_COMPONENT_LONG6, KEYWORD_COMPONENT_LONG6);

	// centre, left, right, far, link, object
	const int long6_links[] = {
		34, 0, 41, -8, 41, 8, 47, 0, 50, 0, 45, 0,
		13, 15, 21, 20, 6, 26, 16, 27, 14, 30, 15, 24,
		-13, 15, -6, 26, -21, 20, -16, 27, -14, 30, -15, 24,
		-34, 0, -41, 8, -41, -8, -47, 0, -50, 0, -45, 0,
		-13, -15, -21, -20, -6, -26, -16, -27, -14, -30, -15, -24,
		13, -15, 6, -26, 21, -20, 16, -27, 14, -30, 15, -24
	};
	add_links(6, long6_links);

	const int long6_collision[] = {43, 0, -43, 0, -3, -27, 3, -27, 3, 27, -3, 27, -40, -10, -40, 10, 40, -10, 40, 10};
	add_collision_vertices(10, long6_collision);
	
 poly_index = POLY_0;
 start_dshape_poly(poly_index++, 0, COL_LEVEL_BASE, TRI_WALK);

 add_vertex(43, 0, 0);
 add_outline_vertex_at_last_poly_vertex(4);
 add_vertex(14, 22, 0);
 add_outline_vertex_at_last_poly_vertex(4);
 add_vertex(-14, 22, 0);
 add_outline_vertex_at_last_poly_vertex(4);
 add_vertex(-43, -0, 0);
 add_outline_vertex_at_last_poly_vertex(4);
 add_vertex(-14, -22, 0);
 add_outline_vertex_at_last_poly_vertex(4);
 add_vertex(14, -22, 0);
 add_outline_vertex_at_last_poly_vertex(4);
 add_poly_fill_source(0, 0);

// pentagon
 const int long6_0[] = {-3, -27, 3, -27, 20, 0, 3, 27, -3, 27, -20, 0};
 dshape_poly(1, COL_LEVEL_MAIN, TRI_WALK, 6, long6_0);
 add_poly_fill_source(0, -10);

// left-up
 const int long6_1[] = {-13, -14, -24, -19, -40, -10, -32, -1, -22, -1};
 dshape_poly(1, COL_LEVEL_MAIN, TRI_WALK, 5, long6_1);
 add_poly_fill_source(-23, -7);

// left-down
 const int long6_2[] = {-13, 14, -24, 19, -40, 10, -32, 1, -22, 1};
 dshape_poly(1, COL_LEVEL_MAIN, TRI_WALK, 5, long6_2);
 add_poly_fill_source(-23, 7);

// right-up
 const int long6_3[] = {13, -14, 24, -19, 40, -10, 32, -1, 22, -1};
 dshape_poly(1, COL_LEVEL_MAIN, TRI_WALK, 5, long6_3);
 add_poly_fill_source(23, -7);

// right-down
 const int long6_4[] = {13, 14, 24, 19, 40, 10, 32, 1, 22, 1};
 dshape_poly(1, COL_LEVEL_MAIN, TRI_WALK, 5, long6_4);
 add_poly_fill_source(23, 7);




	// remember to deal with mirror vertices here!



// noncore shapes have mirrored_object_noncentre, for use when they're not on the design centreline
 nshape[NSHAPE_COMPONENT_LONG6].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_LONG6].mirrored_object_noncentre [1] = 5;
 nshape[NSHAPE_COMPONENT_LONG6].mirrored_object_noncentre [2] = 4;
 nshape[NSHAPE_COMPONENT_LONG6].mirrored_object_noncentre [3] = 3;
 nshape[NSHAPE_COMPONENT_LONG6].mirrored_object_noncentre [4] = 2;
 nshape[NSHAPE_COMPONENT_LONG6].mirrored_object_noncentre [5] = 1;

// always use add_mirror_axis_at_link if axis passes through a link
 add_mirror_axis_at_link(0, 3);
 add_mirror_axis_at_link(3, 0);


 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

 finish_shape();


// ******* NSHAPE_COMPONENT_DROP *******

 start_dshape(NSHAPE_COMPONENT_DROP, KEYWORD_COMPONENT_DROP);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int drop_links[] = {
		34, 0, 40, -7, 40, 7, 47, 0, 50, 0, 45, 0,
		11, -14, 8, -22, 24, -18, 18, -26, 19, -28, 17, -24,
		11, 14, 24, 18, 8, 22, 18, 26, 19, 28, 17, 24,
		-34, 0, -39, 7, -39, -7, -47, 0, -50, 0, -45, 0,
		-14, -18, -23, -23, -7, -25, -17, -29, -18, -30, -16, -27,
		-14, 18, -7, 25, -23, 23, -17, 29, -18, 30, -16, 27
	};
	add_links(6, drop_links);

	const int drop_collision[] = {-17, 25, -17, -25, -4, -26, -4, 26, -40, -10, -40, 10, 40, -10, 40, 10};
	add_collision_vertices(8, drop_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(44, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(18, 20, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-17, 25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-43, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-17, -25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(18, -20, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(0, 0);

	const int drop_0[] = {-26, 0, -4, -26, 5, -24, 15, 0, 5, 24, -4, 26};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, drop_0);
	add_poly_fill_source(-1, 0);

	const int drop_1[] = {-15, -16, -26, -23, -40, -10, -33, -1, -28, -1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, drop_1);
	add_poly_fill_source(-28, -10);

	const int drop_2[] = {-15, 16, -26, 23, -40, 10, -33, 1, -28, 1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, drop_2);
	add_poly_fill_source(-28, 10);

	const int drop_3[] = {27, -17, 40, -10, 32, -1, 17, -1, 12, -12};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, drop_3);
	add_poly_fill_source(25, -8);

	const int drop_4[] = {27, 17, 40, 10, 32, 1, 17, 1, 12, 12};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, drop_4);
	add_poly_fill_source(25, 8);


// noncore shapes have mirrored_object_noncentre, for use when they're not on the design centreline
 nshape[NSHAPE_COMPONENT_DROP].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_DROP].mirrored_object_noncentre [1] = 2;
 nshape[NSHAPE_COMPONENT_DROP].mirrored_object_noncentre [2] = 1;
 nshape[NSHAPE_COMPONENT_DROP].mirrored_object_noncentre [3] = 3;
 nshape[NSHAPE_COMPONENT_DROP].mirrored_object_noncentre [4] = 5;
 nshape[NSHAPE_COMPONENT_DROP].mirrored_object_noncentre [5] = 4;

// always use add_mirror_axis_at_link if axis passes through a link
 add_mirror_axis_at_link(0, 3);
 add_mirror_axis_at_link(3, 0);


 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

 finish_shape();


// ******* NSHAPE_COMPONENT_SIDE *******



 start_dshape(NSHAPE_COMPONENT_SIDE, KEYWORD_COMPONENT_SIDE);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int side_links[] = {
		0, -18, -8, -24, 8, -24, 0, -30, 0, -31, 0, -27,
		0, 18, 8, 24, -8, 24, 0, 30, 0, 31, 0, 27,
		20, -14, 18, -22, 32, -10, 27, -22, 28, -23, 26, -20,
		20, 14, 32, 10, 18, 22, 27, 22, 28, 23, 26, 20,
		-20, -14, -32, -10, -18, -22, -27, -22, -28, -23, -26, -20,
		-20, 14, -18, 22, -32, 10, -27, 22, -28, 23, -26, 20
	};
	add_links(6, side_links);

	const int side_collision[] = {-16, -24, -16, 24, 16, -24, 16, 24, 43, -5, 43, 4, -43, -5, -43, 4};
	add_collision_vertices(8, side_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(37, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(25, 19, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(12, 20, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, 26, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-12, 20, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-25, 19, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-37, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-25, -19, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-12, -20, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, -26, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(12, -20, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(25, -19, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(0, 0);

	const int side_0[] = {-1, -17, -11, -25, -16, -24, -18, -13, -7, -3, -1, -7};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, side_0);
	add_poly_fill_source(-9, -14);

	const int side_1[] = {-1, 17, -11, 25, -16, 24, -18, 13, -7, 3, -1, 7};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, side_1);
	add_poly_fill_source(-9, 14);

	const int side_2[] = {1, -17, 11, -25, 16, -24, 18, -13, 7, -3, 1, -7};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, side_2);
	add_poly_fill_source(9, -14);

	const int side_3[] = {1, 17, 11, 25, 16, 24, 18, 13, 7, 3, 1, 7};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, side_3);
	add_poly_fill_source(9, 14);

	const int side_4[] = {0, -6, -6, -2, -6, 2, 0, 6, 6, 2, 6, -2};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, side_4);
	add_poly_fill_source(0, 0);

	const int side_5[] = {20, -12, 43, -5, 43, 4, 20, 12, 8, 2, 8, -2};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, side_5);
	add_poly_fill_source(23, 0);

	const int side_6[] = {-20, -12, -43, -5, -43, 4, -20, 12, -8, 2, -8, -2};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, side_6);
	add_poly_fill_source(-23, 0);

// noncore shapes have mirrored_object_noncentre, for use when they're not on the design centreline
 nshape[NSHAPE_COMPONENT_SIDE].mirrored_object_noncentre [0] = 1;
 nshape[NSHAPE_COMPONENT_SIDE].mirrored_object_noncentre [1] = 0;
 nshape[NSHAPE_COMPONENT_SIDE].mirrored_object_noncentre [2] = 3;
 nshape[NSHAPE_COMPONENT_SIDE].mirrored_object_noncentre [3] = 2;
 nshape[NSHAPE_COMPONENT_SIDE].mirrored_object_noncentre [4] = 5;
 nshape[NSHAPE_COMPONENT_SIDE].mirrored_object_noncentre [5] = 4;

// always use add_mirror_axis_at_link if axis passes through a link
 add_mirror_axis_at_link(0, 3);
 add_mirror_axis_at_link(3, 0);


 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

 finish_shape();


// ******* NSHAPE_COMPONENT_LONG4 *******


 start_dshape(NSHAPE_COMPONENT_LONG4, KEYWORD_COMPONENT_LONG4);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int long4_links[] = {
		36, 0, 42, -7, 42, 7, 52, 0, 54, 0, 49, 0,
		0, 15, 7, 21, -7, 21, 0, 27, 0, 29, 0, 25,
		-25, 0, -28, 8, -28, -8, -39, 0, -41, 0, -35, 0,
		0, -15, -7, -21, 7, -21, 0, -27, 0, -29, 0, -25
	};
	add_links(4, long4_links);

	const int long4_collision[] = {42, 10, 10, 21, 42, -10, 10, -21, -27, -11, -10, -21, -27, 11, -10, 21};
	add_collision_vertices(8, long4_collision);

	start_dshape_poly(poly_index++, 1, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(50, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, -23, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-37, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, 23, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(3, 0);

	const int long4_0[] = {1, 13, 1, 9, 24, 1, 34, 1, 42, 10, 10, 21};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, long4_0);
	add_poly_fill_source(18, 9);

	const int long4_1[] = {1, -13, 1, -9, 24, -1, 34, -1, 42, -10, 10, -21};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, long4_1);
	add_poly_fill_source(18, -9);

	const int long4_2[] = {-1, -13, -1, -9, -12, -1, -23, -1, -27, -11, -10, -21};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, long4_2);
	add_poly_fill_source(-12, -9);

	const int long4_3[] = {-1, 13, -1, 9, -12, 1, -23, 1, -27, 11, -10, 21};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, long4_3);
	add_poly_fill_source(-12, 9);

	const int long4_4[] = {0, -7, -10, 0, 0, 7, 21, 0};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 4, long4_4);
	add_poly_fill_source(2, 0);

 nshape[NSHAPE_COMPONENT_LONG4].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_LONG4].mirrored_object_noncentre [1] = 3;
 nshape[NSHAPE_COMPONENT_LONG4].mirrored_object_noncentre [2] = 2;
 nshape[NSHAPE_COMPONENT_LONG4].mirrored_object_noncentre [3] = 1;

// always use add_mirror_axis_at_link if axis passes through a link
 add_mirror_axis_at_link(0, 2);
 add_mirror_axis_at_link(2, 0);
 add_mirror_axis_at_link(1, 3);
 add_mirror_axis_at_link(3, 1);

 finish_shape();

// ******* NSHAPE_COMPONENT_BOX *******


 start_dshape(NSHAPE_COMPONENT_BOX, KEYWORD_COMPONENT_BOX);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int box_links[] = {
		22, 0, 27, -11, 27, 11, 34, 0, 40, 0, 35, 0,
		0, 22, 11, 27, -11, 27, 0, 34, 0, 40, 0, 35,
		-22, 0, -27, 11, -27, -11, -34, 0, -40, 0, -35, 0,
		0, -22, -11, -27, 11, -27, 0, -34, 0, -40, 0, -35
	};
	add_links(4, box_links);

	const int box_collision[] = {28, 16, 16, 28, 28, -16, 16, -28, -28, -16, -16, -28, -28, 16, -16, 28};
	add_collision_vertices(8, box_collision);

	start_dshape_poly(poly_index++, 1, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(31, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, -31, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-31, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, 31, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(0, 0);

	const int box_0[] = {1, 14, 14, 1, 20, 1, 28, 16, 16, 28, 1, 20};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, box_0);
	add_poly_fill_source(13, 13);

	const int box_1[] = {1, -14, 14, -1, 20, -1, 28, -16, 16, -28, 1, -20};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, box_1);
	add_poly_fill_source(13, -13);

	const int box_2[] = {-1, -14, -14, -1, -20, -1, -28, -16, -16, -28, -1, -20};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, box_2);
	add_poly_fill_source(-13, -13);

	const int box_3[] = {-1, 14, -14, 1, -20, 1, -28, 16, -16, 28, -1, 20};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, box_3);
	add_poly_fill_source(-13, 13);

	const int box_4[] = {0, -12, -12, 0, 0, 12, 12, 0};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 4, box_4);
	add_poly_fill_source(0, 0);

	nshape[NSHAPE_COMPONENT_BOX].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_BOX].mirrored_object_noncentre [1] = 3;
 nshape[NSHAPE_COMPONENT_BOX].mirrored_object_noncentre [2] = 2;
 nshape[NSHAPE_COMPONENT_BOX].mirrored_object_noncentre [3] = 1;

// always use add_mirror_axis_at_link if axis passes through a link
 add_mirror_axis_at_link(0, 2);
 add_mirror_axis_at_link(2, 0);
 add_mirror_axis_at_link(1, 3);
 add_mirror_axis_at_link(3, 1);

 finish_shape();



// ******* NSHAPE_COMPONENT_LONG5 *******


 start_dshape(NSHAPE_COMPONENT_LONG5, KEYWORD_COMPONENT_LONG5);


	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int long5_links[] = {
		34, 0, 41, -8, 41, 8, 47, 0, 50, 0, 45, 0,
		-27, -8, -39, -5, -30, -16, -40, -12, -43, -13, -39, -11,
		-27, 8, -30, 16, -39, 5, -40, 12, -43, 13, -39, 11,
		-8, -17, -17, -22, 3, -22, -8, -28, -8, -32, -8, -27,
		-8, 17, 3, 22, -17, 22, -8, 28, -8, 32, -8, 27
	};
	add_links(5, long5_links);

	const int long5_collision[] = {46, 0, -10, 25, -10, -25, -42, 3, -42, -3, -29, -17, -29, 17, 40, -10, 40, 10};
	add_collision_vertices(9, long5_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(46, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-10, 25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-35, 9, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-35, -9, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-10, -25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-8, 0);

	const int long5_0[] = {-13, -10, -2, 0, -13, 10, -42, 3, -42, -3};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, long5_0);
	add_poly_fill_source(-22, 0);

	const int long5_1[] = {-9, -16, -20, -22, -29, -17, -26, -9, -13, -12};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, long5_1);
	add_poly_fill_source(-19, -15);

	const int long5_2[] = {-9, 16, -20, 22, -29, 17, -26, 9, -13, 12};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, long5_2);
	add_poly_fill_source(-19, 15);

	const int long5_3[] = {-11, -11, -7, -16, 6, -21, 40, -10, 32, -1, 0, -1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, long5_3);
	add_poly_fill_source(10, -10);

	const int long5_4[] = {-11, 11, -7, 16, 6, 21, 40, 10, 32, 1, 0, 1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, long5_4);
	add_poly_fill_source(10, 10);

 add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0
 add_mirror_axis(ANGLE_2, 0);

 nshape[NSHAPE_COMPONENT_LONG5].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_LONG5].mirrored_object_noncentre [1] = 2;
 nshape[NSHAPE_COMPONENT_LONG5].mirrored_object_noncentre [2] = 1;
 nshape[NSHAPE_COMPONENT_LONG5].mirrored_object_noncentre [3] = 4;
 nshape[NSHAPE_COMPONENT_LONG5].mirrored_object_noncentre [4] = 3;

	finish_shape();


// ******* NSHAPE_COMPONENT_SNUB *******


#ifdef Z_POLY
 zshape_start();
#endif


 start_dshape(NSHAPE_COMPONENT_SNUB, KEYWORD_COMPONENT_SNUB);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int snub_links[] = {
		25, 0, 34, -8, 34, 8, 41, 0, 43, 0, 39, 0,
		-28, -10, -41, -8, -26, -19, -37, -19, -38, -21, -35, -17,
		-28, 10, -26, 19, -41, 8, -37, 19, -38, 21, -35, 17,
		6, -17, -1, -27, 16, -22, 7, -28, 7, -30, 7, -26,
		6, 17, 16, 22, -1, 27, 7, 28, 7, 30, 7, 26
	};
	add_links(5, snub_links);

	const int snub_collision[] = {-44, 6, -44, -6, -3, -28, -24, -21, -3, 28, -24, 21, 34, -10, 34, 10};
	add_collision_vertices(8, snub_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(39, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(6, 27, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-36, 16, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-42, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-36, -16, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(6, -27, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-10, 0);

	const int snub_0[] = {-25, -9, -2, -3, 1, 0, -2, 3, -25, 9, -44, 6, -44, -6};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 7, snub_0);
	add_poly_fill_source(-20, 0);

	const int snub_1[] = {-3, -28, -24, -21, -26, -11, -2, -5, 5, -16};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, snub_1);
	add_poly_fill_source(-10, -16);

	const int snub_2[] = {-3, 28, -24, 21, -26, 11, -2, 5, 5, 16};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, snub_2);
	add_poly_fill_source(-10, 16);

	const int snub_3[] = {6, -15, 18, -21, 34, -10, 24, -1, 2, -1, -1, -4};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, snub_3);
	add_poly_fill_source(13, -8);

	const int snub_4[] = {6, 15, 18, 21, 34, 10, 24, 1, 2, 1, -1, 4};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, snub_4);
	add_poly_fill_source(13, 8);

 add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0
 add_mirror_axis(ANGLE_2, 0);

 nshape[NSHAPE_COMPONENT_SNUB].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_SNUB].mirrored_object_noncentre [1] = 2;
 nshape[NSHAPE_COMPONENT_SNUB].mirrored_object_noncentre [2] = 1;
 nshape[NSHAPE_COMPONENT_SNUB].mirrored_object_noncentre [3] = 4;
 nshape[NSHAPE_COMPONENT_SNUB].mirrored_object_noncentre [4] = 3;

	finish_shape();

// ******* NSHAPE_COMPONENT_CAP *******

 start_dshape(NSHAPE_COMPONENT_CAP, KEYWORD_COMPONENT_CAP);


	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int cap_links[] = {
		-7, 29, -4, 43, -21, 28, -13, 41, -15, 45, -13, 41,
		-19, 10, -25, 23, -30, 7, -31, 16, -33, 18, -30, 16,
		-19, -10, -30, -7, -25, -23, -31, -16, -33, -18, -30, -16,
		-7, -29, -21, -28, -4, -43, -13, -41, -15, -45, -13, -41
	};
	add_links(4, cap_links);

	const int cap_collision[] = {-9, 40, -27, 13, -27, -13, -9, -40, -32, 4, -32, -4, 7, -38, 0, -49, -24, -26, 7, 38, 0, 49, -24, 26};
	add_collision_vertices(12, cap_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(3, -36, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(7, -16, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(8, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(7, 16, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(2, 36, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-9, 40, 0);
	add_outline_vertex_at_last_poly_vertex(4);
	add_vertex(-27, 13, 0);
	add_outline_vertex_at_last_poly_vertex(4);
	add_vertex(-16, 0, 0);
//	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-27, -13, 0);
	add_outline_vertex_at_last_poly_vertex(4);
	add_vertex(-9, -40, 0);
	add_outline_vertex_at_last_poly_vertex(4);
	add_poly_fill_source(-6, 0);

	const int cap_0[] = {-10, -12, 12, -3, 12, 3, -10, 12, -32, 4, -32, -4};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, cap_0);
	add_poly_fill_source(-10, 0);

	const int cap_1[] = {12, -6, 7, -38, 0, -49, -9, -14};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 4, cap_1);
	add_poly_fill_source(2, -26);

	const int cap_2[] = {-11, -14, -18, -11, -24, -26, -7, -27};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 4, cap_2);
	add_poly_fill_source(-15, -19);

	const int cap_3[] = {12, 6, 7, 38, 0, 49, -9, 14};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 4, cap_3);
	add_poly_fill_source(2, 26);

	const int cap_4[] = {-11, 14, -18, 11, -24, 26, -7, 27};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 4, cap_4);
	add_poly_fill_source(-15, 19);

 add_mirror_axis(0, -1);
 add_mirror_axis(ANGLE_2, -1);

 nshape[NSHAPE_COMPONENT_CAP].mirrored_object_noncentre [0] = 3;
 nshape[NSHAPE_COMPONENT_CAP].mirrored_object_noncentre [1] = 2;
 nshape[NSHAPE_COMPONENT_CAP].mirrored_object_noncentre [2] = 1;
 nshape[NSHAPE_COMPONENT_CAP].mirrored_object_noncentre [3] = 0;

// has no mirror axes

 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

 finish_shape();


// ******* NSHAPE_COMPONENT_TRI *******

 start_dshape(NSHAPE_COMPONENT_TRI, KEYWORD_COMPONENT_TRI);


	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int tri_links[] = {
		29, 0, 35, -6, 35, 6, 43, 0, 45, 0, 42, 0,
		-3, -12, -12, -20, 4, -20, -4, -26, -4, -25, -4, -23,
		-3, 12, 4, 20, -12, 20, -4, 26, -4, 25, -4, 23
	};
	add_links(3, tri_links);

	const int tri_collision[] = {41, 0, -3, 21, -3, -21, -15, 20, -28, 0, -15, -20, 6, -20, 35, -8, 6, 20, 35, 8};
	add_collision_vertices(10, tri_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(41, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-3, 21, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-27, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-3, -21, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(2, 0);

	const int tri_0[] = {3, 0, -4, 11, -15, 20, -28, 0, -15, -20, -4, -11};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, tri_0);
	add_poly_fill_source(-10, 0);

	const int tri_1[] = {4, -1, -2, -11, 6, -20, 35, -8, 28, -1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, tri_1);
	add_poly_fill_source(14, -8);

	const int tri_2[] = {4, 1, -2, 11, 6, 20, 35, 8, 28, 1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, tri_2);
	add_poly_fill_source(14, 8);

 add_mirror_axis(ANGLE_2, 0);

 nshape[NSHAPE_COMPONENT_TRI].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_TRI].mirrored_object_noncentre [1] = 2;
 nshape[NSHAPE_COMPONENT_TRI].mirrored_object_noncentre [2] = 1;

 add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0

	finish_shape();

// ******* NSHAPE_COMPONENT_PRONG *******

 start_dshape(NSHAPE_COMPONENT_PRONG, KEYWORD_COMPONENT_PRONG);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int prong_links[] = {
		20, -11, 16, -22, 31, -8, 28, -19, 31, -21, 27, -18,
		20, 11, 31, 8, 16, 22, 28, 19, 31, 21, 27, 18,
		0, -10, -10, -17, 5, -21, -3, -24, -4, -26, -3, -23,
		0, 10, 5, 21, -10, 17, -3, 24, -4, 26, -3, 23
	};
	add_links(4, prong_links);

	const int prong_collision[] = {-53, 5, -53, -5, 13, -25, 8, -25, 13, 25, 8, 25, 43, -3, 43, 3};
	add_collision_vertices(8, prong_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(33, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(24, 17, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-2, 21, 0);
	add_outline_vertex_at_last_poly_vertex(5);
	add_vertex(-32, 0, 0);
	add_outline_vertex_at_xy(-44, 8);
	add_outline_vertex_at_xy(-44, -8);
//	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-2, -21, 0);
	add_outline_vertex_at_last_poly_vertex(5);
	add_vertex(24, -17, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(7, 0);

	const int prong_0[] = {7, 0, -2, 9, -13, 17, -53, 5, -53, -5, -13, -17, -1, -9};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 7, prong_0);
	add_poly_fill_source(-18, 0);

	const int prong_1[] = {8, -1, 13, -1, 19, -9, 13, -25, 8, -25, 1, -9};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, prong_1);
	add_poly_fill_source(10, -11);

	const int prong_2[] = {8, 1, 13, 1, 19, 9, 13, 25, 8, 25, 1, 9};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, prong_2);
	add_poly_fill_source(10, 11);

	const int prong_3[] = {14, 0, 21, -9, 43, -3, 43, 3, 21, 9};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, prong_3);
	add_poly_fill_source(28, 0);

 nshape[NSHAPE_COMPONENT_PRONG].mirrored_object_noncentre [0] = 1;
 nshape[NSHAPE_COMPONENT_PRONG].mirrored_object_noncentre [1] = 0;
 nshape[NSHAPE_COMPONENT_PRONG].mirrored_object_noncentre [2] = 3;
 nshape[NSHAPE_COMPONENT_PRONG].mirrored_object_noncentre [3] = 2;

// add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0 - has no mirror axes

	finish_shape();


// ******* NSHAPE_COMPONENT_FORK *******

 start_dshape(NSHAPE_COMPONENT_FORK, KEYWORD_COMPONENT_FORK);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int fork_links[] = {
		7, 0, 14, -7, 14, 7, 21, 0, 24, 0, 21, 0,
		-13, -14, -21, -11, -11, -26, -22, -23, -25, -25, -22, -23,
		-13, 14, -11, 26, -21, 11, -22, 23, -25, 25, -22, 23
	};
	add_links(3, fork_links);

	const int fork_collision[] = {-7, -41, 1, -41, 15, -10, -7, 41, 1, 41, 15, 10, -38, -4, -38, 4};
	add_collision_vertices(8, fork_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(16, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-1, 29, 0);
	add_outline_vertex_at_xy(4, 36);
//	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-18, 21, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_outline_vertex_at_xy(-32, 8);
	add_outline_vertex_at_xy(-32, -8);
	add_vertex(-18, -21, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-1, -29, 0);
	add_outline_vertex_at_xy(4, -36);
//	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-4, 0);

	const int fork_0[] = {-4, -1, -12, -13, -7, -41, 1, -41, 15, -10, 6, -1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, fork_0);
	add_poly_fill_source(0, -17);

	const int fork_1[] = {-4, 1, -12, 13, -7, 41, 1, 41, 15, 10, 6, 1};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 6, fork_1);
	add_poly_fill_source(0, 17);

	const int fork_2[] = {-5, 0, -13, -12, -38, -4, -38, 4, -13, 12};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, fork_2);
	add_poly_fill_source(-21, 0);

 nshape[NSHAPE_COMPONENT_FORK].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_FORK].mirrored_object_noncentre [1] = 2;
 nshape[NSHAPE_COMPONENT_FORK].mirrored_object_noncentre [2] = 1;

 add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0
 add_mirror_axis(ANGLE_2, 0);

	finish_shape();



// ******* NSHAPE_COMPONENT_BOWL *******


 start_dshape(NSHAPE_COMPONENT_BOWL, KEYWORD_COMPONENT_BOWL);


	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int bowl_links[] = {
		-27, 0, -32, 8, -32, -8, -39, 0, -42, 0, -39, 0,
		-12, -33, -19, -26, -13, -42, -22, -37, -26, -39, -22, -37,
		-12, 33, -13, 42, -19, 26, -22, 37, -26, 39, -22, 37,
		9, -8, 14, -21, 16, -6, 20, -13, 23, -13, 19, -13,
		9, 8, 16, 6, 14, 21, 20, 13, 23, 13, 19, 13
	};
	add_links(5, bowl_links);

	const int bowl_collision[] = {-36, 0, 17, -4, 17, 4, -25, 0, 13, -23, 0, -49, -13, -54, 13, 23, 0, 49, -13, 54, -32, -12, -32, 12};
	add_collision_vertices(12, bowl_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(15, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(16, 12, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-3, 47, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-18, 36, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-18, 22, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-36, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-18, -22, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-18, -36, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-3, -47, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(16, -12, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-6, 0);

	const int bowl_0[] = {-10, -12, 17, -4, 17, 4, -10, 12, -25, 0};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, bowl_0);
	add_poly_fill_source(-2, 0);

	const int bowl_1[] = {7, -9, 13, -23, 0, -49, -13, -54, -9, -14};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, bowl_1);
	add_poly_fill_source(0, -29);

	const int bowl_2[] = {7, 9, 13, 23, 0, 49, -13, 54, -9, 14};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, bowl_2);
	add_poly_fill_source(0, 29);

	const int bowl_3[] = {-11, -15, -12, -30, -32, -12, -26, -2};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 4, bowl_3);
	add_poly_fill_source(-20, -14);

	const int bowl_4[] = {-11, 15, -12, 30, -32, 12, -26, 2};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 4, bowl_4);
	add_poly_fill_source(-20, 14);

 add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0
 add_mirror_axis(0, 0);

 nshape[NSHAPE_COMPONENT_BOWL].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_BOWL].mirrored_object_noncentre [1] = 2;
 nshape[NSHAPE_COMPONENT_BOWL].mirrored_object_noncentre [2] = 1;
 nshape[NSHAPE_COMPONENT_BOWL].mirrored_object_noncentre [3] = 4;
 nshape[NSHAPE_COMPONENT_BOWL].mirrored_object_noncentre [4] = 3;

	finish_shape();


// ******* NSHAPE_COMPONENT_PEAK *******

 start_dshape(NSHAPE_COMPONENT_PEAK, KEYWORD_COMPONENT_PEAK);

	poly_index = POLY_0;
	// centre, left, right, far, link, object
	const int peak_links[] = {
		-19, 0, -25, 8, -25, -8, -34, 0, -37, 0, -32, 0,
		-11, -26, -23, -30, -3, -33, -14, -37, -15, -40, -14, -35,
		-11, 26, -3, 33, -23, 30, -14, 37, -15, 40, -14, 35,
		12, -15, 16, -23, 22, -10, 22, -19, 24, -20, 20, -18,
		12, 15, 22, 10, 16, 23, 22, 19, 24, 20, 20, 18
	};
	add_links(5, peak_links);

	const int peak_collision[] = {-30, 0, 27, -1, 27, 1, 16, -26, 2, -36, 16, 26, 2, 36, -43, -33, -43, 33};
	add_collision_vertices(9, peak_collision);

	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY, TRI_FAN);

	add_vertex(22, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(20, 18, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-13, 35, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-29, 24, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-22, 10, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-30, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-22, -10, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-29, -24, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-13, -35, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(20, -18, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-9, 0);

	const int peak_0[] = {-11, -24, 26, -7, 27, -1, -18, -1, -26, -12};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, peak_0);
	add_poly_fill_source(0, -9);

	const int peak_1[] = {-11, 24, 26, 7, 27, 1, -18, 1, -26, 12};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 5, peak_1);
	add_poly_fill_source(0, 9);

	const int peak_2[] = {11, -16, 16, -26, 2, -36, -9, -25};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 4, peak_2);
	add_poly_fill_source(5, -25);

	const int peak_3[] = {11, 16, 16, 26, 2, 36, -9, 25};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 4, peak_3);
	add_poly_fill_source(5, 25);

	const int peak_4[] = {-13, -25, -43, -33, -38, -16, -26, -14};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 4, peak_4);
	add_poly_fill_source(-30, -22);

	const int peak_5[] = {-13, 25, -43, 33, -38, 16, -26, 14};
	dshape_poly(1, PROC_COL_MAIN_1, TRI_FAN, 4, peak_5);
	add_poly_fill_source(-30, 22);

 add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0
 add_mirror_axis(0, 0);

 nshape[NSHAPE_COMPONENT_PEAK].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_PEAK].mirrored_object_noncentre [1] = 2;
 nshape[NSHAPE_COMPONENT_PEAK].mirrored_object_noncentre [2] = 1;
 nshape[NSHAPE_COMPONENT_PEAK].mirrored_object_noncentre [3] = 4;
 nshape[NSHAPE_COMPONENT_PEAK].mirrored_object_noncentre [4] = 3;

	finish_shape();

// after all shapes:

 init_nshape_collision_masks();
//  test_draw_mask(NSHAPE_CORE_STATIC_QUAD);

}

static void start_dshape(int ds, int keyword_index)
{

	 nshape[ds].vertices = 0;
	 nshape[ds].links = 0;
	 nshape[ds].max_length = 1;
	 nshape[ds].keyword_index = keyword_index;

	 nshape[ds].mirror_axes = 0; // some nshapes have no mirror axes


	 dshape[ds].polys = 0;
	 dshape[ds].links = 0; // ?
	 dshape[ds].outline_vertices = 0;

  dshape_init.current_nshape = ds; // currently assume nshape index = dshape index
  dshape_init.current_dshape = ds; // currently assume nshape index = dshape index

  collision_mask_poly[ds].polys = 0;

  nshape[ds].base_hp_max = nshape_init_data [ds].base_hp_max;
//  nshape[ds].build_or_restore_time = nshape_init_data [ds].build_or_restore_time;
  nshape[ds].data_cost = nshape_init_data [ds].data_cost;
  nshape[ds].shape_mass = nshape[ds].data_cost;// * 10;
  nshape[ds].power_capacity = nshape_init_data [ds].power_capacity;
  nshape[ds].component_power_capacity = nshape_init_data [ds].component_power_capacity;
  nshape[ds].interface_charge_rate = nshape_init_data [ds].interface_charge_rate;
  nshape[ds].instructions_per_cycle = nshape_init_data [ds].instructions_per_cycle;
  nshape[ds].unlock_index = nshape_init_data [ds].unlock_index;

// now, add a collision vertex near the middle (this will prevent things like shapes spawning on top of other shapes):
  nshape[ds].vertex_angle_fixed [nshape[ds].vertices] = 0;
  nshape[ds].vertex_dist_fixed [nshape[ds].vertices] = al_itofix(3); // but not exactly on the middle (could cause div by zero somewhere?)
  nshape[ds].vertex_dist_pixel [nshape[ds].vertices] = 3;

  nshape[ds].vertices ++;


}

static void start_dshape_poly(int poly, int layer, int poly_colour_level, int triangulation)
{

#ifdef SANITY_CHECK
  if (poly >= DSHAPE_POLYS)
		{
			fpr("\nError: g_shapes.h: start_dshape_poly(): too many polygons for shape %i", dshape_init.current_nshape);
			error_call();
		}
#endif

#ifdef Z_POLY
 zshape_add_poly(poly, layer, poly_colour_level);
#endif



		dshape[dshape_init.current_nshape].display_vertices [poly] = 0;
		dshape[dshape_init.current_nshape].poly_layer [poly] = layer;
		dshape[dshape_init.current_nshape].poly_colour_level [poly] = poly_colour_level;
		dshape[dshape_init.current_nshape].triangulation[poly] = triangulation;

//  dshape_init.current_dshape = ds;
  dshape_init.current_poly = poly;

//  dshape_init.current_nshape = ds; // currently assume nshape index = dshape index
	 dshape[dshape_init.current_nshape].polys ++;

  collision_mask_poly[dshape_init.current_nshape].polys ++;
  collision_mask_poly[dshape_init.current_nshape].vertices [poly] = 0;

}

void dshape_poly(int layer, int poly_colour_level, int triangulation, int num_vtx, const int* coords)
{
	int poly = dshape[dshape_init.current_nshape].polys;
	dshape[dshape_init.current_nshape].display_vertices [poly] = num_vtx;
	dshape[dshape_init.current_nshape].poly_layer [poly] = layer;
	dshape[dshape_init.current_nshape].poly_colour_level [poly] = poly_colour_level;
	dshape[dshape_init.current_nshape].triangulation[poly] = triangulation;

	dshape_init.current_poly = poly;
	dshape[dshape_init.current_nshape].polys ++;

	collision_mask_poly[dshape_init.current_nshape].polys ++;
	collision_mask_poly[dshape_init.current_nshape].vertices [poly] = num_vtx;

	int i;
	float x, y;
	for(i = 0; i < num_vtx; ++i)
	{
		x = coords[2*i]; y = coords[2*i+1];
		dshape [dshape_init.current_dshape].display_vertex_angle [dshape_init.current_poly] [i] = atan2(y, x);
		dshape [dshape_init.current_dshape].display_vertex_dist [dshape_init.current_poly] [i] = hypot(y, x);
		collision_mask_poly[dshape_init.current_dshape].vertex_x [dshape_init.current_poly] [i] = x;
		collision_mask_poly[dshape_init.current_dshape].vertex_y [dshape_init.current_poly] [i] = y;
	}
}

static void add_poly_fill_source(int x, int y)
{
//fpr("\n add_poly_fill_source for shape %i poly %i at %i,%i", dshape_init.current_dshape, dshape_init.current_poly, x, y);


#ifdef Z_POLY
 zshape_add_fill_source(x, y);
#endif

 collision_mask_poly[dshape_init.current_dshape].fill_source_x [dshape_init.current_poly] = x;
 collision_mask_poly[dshape_init.current_dshape].fill_source_y [dshape_init.current_poly] = y;

}

void add_collision_vertices_fixed(int num_vtx, const al_fixed* coords)
{
	int i;
	al_fixed x, y;
	for (i = 0; i < num_vtx; ++i)
	{
		x = coords[2*i]; y = coords[2*i+1];
		nshape[dshape_init.current_nshape].vertex_angle_fixed [i] = get_angle(y, x);
		nshape[dshape_init.current_nshape].vertex_dist_fixed [i] = distance(y, x);
		nshape[dshape_init.current_nshape].vertex_dist_pixel [i] = al_fixtoi(nshape[dshape_init.current_nshape].vertex_dist_fixed [i]);
		if (nshape[dshape_init.current_nshape].max_length < nshape[dshape_init.current_nshape].vertex_dist_fixed [i])
			nshape[dshape_init.current_nshape].max_length = nshape[dshape_init.current_nshape].vertex_dist_fixed [i];
	}
	nshape[dshape_init.current_nshape].vertices += num_vtx;
}

void add_vertices_fixed(int num_vtx, const al_fixed* coords)
{
	int i;
	al_fixed x, y;
	for(i = 0; i < num_vtx; ++i)
	{
		x = coords[2*i]; y = coords[2*i+1];
		dshape [dshape_init.current_dshape].display_vertex_angle [dshape_init.current_poly] [i] = atan2(y, x);
		dshape [dshape_init.current_dshape].display_vertex_dist [dshape_init.current_poly] [i] = hypot(y, x) / (1 << 16);
		collision_mask_poly[dshape_init.current_dshape].vertex_x [dshape_init.current_poly] [i] = al_fixtoi(x);
		collision_mask_poly[dshape_init.current_dshape].vertex_y [dshape_init.current_poly] [i] = al_fixtoi(y);
	}
	collision_mask_poly[dshape_init.current_dshape].vertices [dshape_init.current_poly] += num_vtx;
	dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly] += num_vtx;
}

void add_collision_vertices(int num_vtx, const int* coords)
{
	int i;
	al_fixed x, y;
	for (i = 0; i < num_vtx; ++i)
	{
		x = al_itofix(coords[2*i]); y = al_itofix(coords[2*i+1]);
		nshape[dshape_init.current_nshape].vertex_angle_fixed [i] = get_angle(y, x);
		nshape[dshape_init.current_nshape].vertex_dist_fixed [i] = distance(y, x);
		nshape[dshape_init.current_nshape].vertex_dist_pixel [i] = al_fixtoi(nshape[dshape_init.current_nshape].vertex_dist_fixed [i]);
		if (nshape[dshape_init.current_nshape].max_length < nshape[dshape_init.current_nshape].vertex_dist_fixed [i])
			nshape[dshape_init.current_nshape].max_length = nshape[dshape_init.current_nshape].vertex_dist_fixed [i];
	}
	nshape[dshape_init.current_nshape].vertices += num_vtx;
}

// display vertices must be added in an order that reflects how they will be drawn.
static void add_vertex(int x, int y, int add_collision_vertex)
{
#ifdef SANITY_CHECK
 if (dshape[dshape_init.current_dshape].display_vertices[dshape_init.current_poly] >= DSHAPE_DISPLAY_VERTICES)
	{
		fpr("\nError: g_shapes.c: add_display_vertex(): too many display vertices on shape %i.", dshape_init.current_dshape);
		error_call();
	}
#endif

#ifdef Z_POLY
 zshape_add_vertex(x, y, add_collision_vertex);
#endif

 dshape [dshape_init.current_dshape].display_vertex_angle [dshape_init.current_poly] [dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly]] = atan2(y, x);
 dshape [dshape_init.current_dshape].display_vertex_dist [dshape_init.current_poly] [dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly]] = hypot(y, x);

 if (add_collision_vertex)
	{
  nshape[dshape_init.current_nshape].vertex_angle_fixed [nshape[dshape_init.current_nshape].vertices] = get_angle(al_itofix(y), al_itofix(x));
  nshape[dshape_init.current_nshape].vertex_dist_fixed [nshape[dshape_init.current_nshape].vertices] = distance(al_itofix(y), al_itofix(x));
  nshape[dshape_init.current_nshape].vertex_dist_pixel [nshape[dshape_init.current_nshape].vertices] = al_fixtoi(nshape[dshape_init.current_nshape].vertex_dist_fixed [nshape[dshape_init.current_nshape].vertices]);
  if (nshape[dshape_init.current_nshape].max_length < nshape[dshape_init.current_nshape].vertex_dist_fixed [nshape[dshape_init.current_nshape].vertices])
			nshape[dshape_init.current_nshape].max_length = nshape[dshape_init.current_nshape].vertex_dist_fixed [nshape[dshape_init.current_nshape].vertices];

  nshape[dshape_init.current_nshape].vertices ++;
	}

// last_vertex stuff is used to calculate link position if this vertex is the base of a link
//  (because the vertex will probably just be a display vertex, so only float data would otherwise be available)
	dshape_init.last_vertex_angle = get_angle(al_itofix(y), al_itofix(x));
	dshape_init.last_vertex_dist_fixed = distance(al_itofix(y), al_itofix(x));
	dshape_init.last_display_vertex = dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly] - 1;

 collision_mask_poly[dshape_init.current_dshape].vertex_x [dshape_init.current_poly] [collision_mask_poly[dshape_init.current_dshape].vertices [dshape_init.current_poly]] = x;
 collision_mask_poly[dshape_init.current_dshape].vertex_y [dshape_init.current_poly] [collision_mask_poly[dshape_init.current_dshape].vertices [dshape_init.current_poly]] = y;

 collision_mask_poly[dshape_init.current_dshape].vertices [dshape_init.current_poly] ++;
 dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly] ++;


}
/*
static void add_link_at_last_vertex(int link_index, int link_extra_distance, int object_extra_distance)
{
//	int link_index = nshape[dshape_init.current_nshape].links;

	nshape[dshape_init.current_nshape].link_angle_fixed [link_index] = dshape_init.last_vertex_angle;
	nshape[dshape_init.current_nshape].link_dist_fixed [link_index] = dshape_init.last_vertex_dist_fixed + al_itofix(link_extra_distance);
	nshape[dshape_init.current_nshape].link_dist_pixel [link_index] = al_fixtoi(dshape_init.last_vertex_dist_fixed) + link_extra_distance;
	nshape[dshape_init.current_nshape].object_dist_fixed [link_index] = dshape_init.last_vertex_dist_fixed + al_itofix(object_extra_distance);
	nshape[dshape_init.current_nshape].object_dist_pixel [link_index] = al_fixtoi(dshape_init.last_vertex_dist_fixed) + object_extra_distance;

	dshape[dshape_init.current_dshape].link_poly [link_index] = dshape_init.current_poly;
	dshape[dshape_init.current_dshape].link_display_vertex [link_index] = dshape_init.last_display_vertex;

 if (nshape[dshape_init.current_nshape].links <= link_index)
	{
		nshape[dshape_init.current_nshape].links = link_index + 1;
		dshape[dshape_init.current_nshape].links = link_index + 1;
	}

// fpr("\n adding link %i to shape %i angle %i dist %i (links %i)", link_index, dshape_init.current_nshape, al_fixtoi(nshape[dshape_init.current_nshape].link_angle_fixed [link_index]), al_fixtoi(nshape[dshape_init.current_nshape].object_dist_fixed [link_index]), nshape[dshape_init.current_nshape].links);

}
*/

// display vertices must be added in an order that reflects how they will be drawn.
static void add_vertex_vector(int angle, int dist, int add_collision_vertex)
{
#ifdef SANITY_CHECK
 if (dshape[dshape_init.current_dshape].display_vertices[dshape_init.current_poly] >= DSHAPE_DISPLAY_VERTICES)
	{
		fpr("\nError: g_shapes.c: add_display_vertex(): too many display vertices on shape %i.", dshape_init.current_dshape);
		error_call();
	}
#endif
 dshape [dshape_init.current_dshape].display_vertex_angle [dshape_init.current_poly] [dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly]] = angle_to_radians(angle);
 dshape [dshape_init.current_dshape].display_vertex_dist [dshape_init.current_poly] [dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly]] = dist;

 if (add_collision_vertex)
	{
  nshape[dshape_init.current_nshape].vertex_angle_fixed [nshape[dshape_init.current_nshape].vertices] = int_angle_to_fixed(angle);
  nshape[dshape_init.current_nshape].vertex_dist_fixed [nshape[dshape_init.current_nshape].vertices] = al_itofix(dist);
  nshape[dshape_init.current_nshape].vertex_dist_pixel [nshape[dshape_init.current_nshape].vertices] = al_fixtoi(nshape[dshape_init.current_nshape].vertex_dist_fixed [nshape[dshape_init.current_nshape].vertices]);
  if (nshape[dshape_init.current_nshape].max_length < nshape[dshape_init.current_nshape].vertex_dist_fixed [nshape[dshape_init.current_nshape].vertices])
			nshape[dshape_init.current_nshape].max_length = nshape[dshape_init.current_nshape].vertex_dist_fixed [nshape[dshape_init.current_nshape].vertices];

  nshape[dshape_init.current_nshape].vertices ++;

	}

// last_vertex stuff is used to calculate link position if this vertex is the base of a link
//  (because the vertex will probably just be a display vertex, so only float data would otherwise be available)
	dshape_init.last_vertex_angle = int_angle_to_fixed(angle);
	dshape_init.last_vertex_dist_fixed = al_itofix(dist);
	dshape_init.last_display_vertex = dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly] - 1;

 collision_mask_poly[dshape_init.current_dshape].vertex_x [dshape_init.current_poly] [collision_mask_poly[dshape_init.current_dshape].vertices [dshape_init.current_poly]] = al_fixtoi(fixed_xpart(dshape_init.last_vertex_angle, dshape_init.last_vertex_dist_fixed));
 collision_mask_poly[dshape_init.current_dshape].vertex_y [dshape_init.current_poly] [collision_mask_poly[dshape_init.current_dshape].vertices [dshape_init.current_poly]] = al_fixtoi(fixed_ypart(dshape_init.last_vertex_angle, dshape_init.last_vertex_dist_fixed));

 collision_mask_poly[dshape_init.current_dshape].vertices [dshape_init.current_poly] ++;

 dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly] ++;



}
/*
// assumes that vertices are being added in increasing angles
static void add_triple_link_vertex(int x, int y, int link_index)
{

	al_fixed fixed_x = al_itofix(x);
	al_fixed fixed_y = al_itofix(y);

	al_fixed vertex_angle = get_angle(fixed_y, fixed_x);

#define TRIPLE_VERTEX_ANGLE (AFX_ANGLE_8 + AFX_ANGLE_16)
#define TRIPLE_VERTEX_INDENT 12

// the fixed_xpart/fixed_ypart calls are wrong because TRIPLE_VERTEX_INDENT is an int instead of fixed,
//  but somehow they work anyway (I think fixed_xpart must return an int when dist parameter is an int, or something)
 add_vertex(x + fixed_xpart(vertex_angle - TRIPLE_VERTEX_ANGLE, TRIPLE_VERTEX_INDENT), y + fixed_ypart(vertex_angle - TRIPLE_VERTEX_ANGLE, TRIPLE_VERTEX_INDENT), 1);
 add_vertex(x, y, 0);
 add_link_at_last_vertex(link_index, LINK_EXTRA_DIST, OBJECT_EXTRA_DIST);
 add_vertex(x + fixed_xpart(vertex_angle + TRIPLE_VERTEX_ANGLE, TRIPLE_VERTEX_INDENT), y + fixed_ypart(vertex_angle + TRIPLE_VERTEX_ANGLE, TRIPLE_VERTEX_INDENT), 1);

}


// assumes that vertices are being added in increasing angles
static void add_triple_link_vertex_vector(int angle, int dist, int previous_angle, int previous_angle_dist, int next_angle, int next_angle_dist, int link_index)
{

previous_angle_dist = 12;
next_angle_dist = 12;

	al_fixed fixed_x = fixed_xpart(int_angle_to_fixed(angle), al_itofix(dist));
	al_fixed fixed_y = fixed_ypart(int_angle_to_fixed(angle), al_itofix(dist));
	int x = al_fixtoi(fixed_x);
	int y = al_fixtoi(fixed_y);

	al_fixed fixed_angle = int_angle_to_fixed(angle);

 add_vertex(x + al_fixtoi(fixed_xpart(fixed_angle - int_angle_to_fixed(previous_angle), al_itofix(previous_angle_dist))), y + al_fixtoi(fixed_ypart(fixed_angle - int_angle_to_fixed(previous_angle), al_itofix(previous_angle_dist))), 1);
 add_vertex(x, y, 0);
 add_link_at_last_vertex(link_index, LINK_EXTRA_DIST, OBJECT_EXTRA_DIST);
 add_vertex(x + al_fixtoi(fixed_xpart(fixed_angle + int_angle_to_fixed(next_angle), al_itofix(next_angle_dist))), y + al_fixtoi(fixed_ypart(fixed_angle + int_angle_to_fixed(next_angle), al_itofix(next_angle_dist))), 1);

}
*/
/*

// not currently used

static void add_outline_vertex(float x, float y)
{
#ifdef SANITY_CHECK
 if (dshape[dshape_init.current_dshape].outline_vertices >= OUTLINE_VERTICES - 1)
	{
		fpr("\n Error: g_shapes.c: add_outline_vertex(): dshape %i has too many outline vertices.", dshape_init.current_dshape);
		error_call();
	}
#endif

 dshape[dshape_init.current_dshape].outline_vertex_angle [dshape[dshape_init.current_dshape].outline_vertices] = atan2(y, x);
 dshape[dshape_init.current_dshape].outline_vertex_dist [dshape[dshape_init.current_dshape].outline_vertices] = hypot(y, x);

	dshape[dshape_init.current_dshape].outline_vertices ++;

}

static void add_outline_vertex_vector(float angle, float dist)
{
#ifdef SANITY_CHECK
 if (dshape[dshape_init.current_dshape].outline_vertices >= OUTLINE_VERTICES - 1)
	{
		fpr("\n Error: g_shapes.c: add_outline_vertex(): dshape %i has too many outline vertices.", dshape_init.current_dshape);
		error_call();
	}
#endif

 dshape[dshape_init.current_dshape].outline_vertex_angle [dshape[dshape_init.current_dshape].outline_vertices] = angle;
 dshape[dshape_init.current_dshape].outline_vertex_dist [dshape[dshape_init.current_dshape].outline_vertices] = dist;

	dshape[dshape_init.current_dshape].outline_vertices ++;

}
*/

static void add_outline_vertex_at_last_poly_vertex(int extra_distance)
{
#ifdef SANITY_CHECK
 if (dshape[dshape_init.current_dshape].outline_vertices >= OUTLINE_VERTICES - 1)
	{
		fpr("\n Error: g_shapes.c: add_outline_vertex(): dshape %i has too many outline vertices.", dshape_init.current_dshape);
		error_call();
	}
#endif

//fpr("\n adding outline vertex %i dist %f", dshape[dshape_init.current_dshape].outline_vertices, extra_distance);

	dshape[dshape_init.current_nshape].outline_vertex_angle_fixed [dshape[dshape_init.current_dshape].outline_vertices] = dshape_init.last_vertex_angle;
	dshape[dshape_init.current_nshape].outline_vertex_dist_fixed [dshape[dshape_init.current_dshape].outline_vertices] = dshape_init.last_vertex_dist_fixed + al_itofix(extra_distance);

// these are float values used only for display:
	dshape[dshape_init.current_nshape].outline_vertex_pos [dshape[dshape_init.current_dshape].outline_vertices][0] = cos(fixed_to_radians(dshape_init.last_vertex_angle)) * (al_fixtof(dshape_init.last_vertex_dist_fixed) + extra_distance);
	dshape[dshape_init.current_nshape].outline_vertex_pos [dshape[dshape_init.current_dshape].outline_vertices][1] = sin(fixed_to_radians(dshape_init.last_vertex_angle)) * (al_fixtof(dshape_init.last_vertex_dist_fixed) + extra_distance);

	dshape[dshape_init.current_dshape].outline_vertices ++;

}

static void add_outline_vertex_at_xy(int x, int y)
{
#ifdef SANITY_CHECK
 if (dshape[dshape_init.current_dshape].outline_vertices >= OUTLINE_VERTICES - 1)
	{
		fpr("\n Error: g_shapes.c: add_outline_vertex(): dshape %i has too many outline vertices.", dshape_init.current_dshape);
		error_call();
	}
#endif

	dshape[dshape_init.current_nshape].outline_vertex_angle_fixed [dshape[dshape_init.current_dshape].outline_vertices] = get_angle(al_itofix(y), al_itofix(x));
	dshape[dshape_init.current_nshape].outline_vertex_dist_fixed [dshape[dshape_init.current_dshape].outline_vertices] = distance(al_itofix(y), al_itofix(x));

// these are float values used only for display:
	dshape[dshape_init.current_nshape].outline_vertex_pos [dshape[dshape_init.current_dshape].outline_vertices][0] = x;
	dshape[dshape_init.current_nshape].outline_vertex_pos [dshape[dshape_init.current_dshape].outline_vertices][1] = y;

	dshape[dshape_init.current_dshape].outline_vertices ++;


}



/*
void add_link_vector(int angle, int dist)
{

#ifdef SANITY_CHECK
  if (nshape[dshape_init.current_dshape].links >= MAX_LINKS)
  {
  	fpr("\nError: g_shapes.c: add_link_vector(): too many links (max %i).", MAX_LINKS);
  	error_call();
  }
#endif

  nshape[dshape_init.current_dshape].link_angle_fixed [nshape[dshape_init.current_dshape].links] = int_angle_to_fixed(angle);
  nshape[dshape_init.current_dshape].link_dist_fixed [nshape[dshape_init.current_dshape].links] = al_itofix(dist);
  nshape[dshape_init.current_dshape].link_dist_pixel [nshape[dshape_init.current_dshape].links] = dist;
	 nshape[dshape_init.current_dshape].links ++;

}*/

static void finish_shape(void)
{

 struct dshape_struct* dsh = &dshape[dshape_init.current_dshape];
 struct nshape_struct* nsh = &nshape[dshape_init.current_nshape];

 int i;

 for (i = 0; i < dsh->outline_vertices; i ++)
	{
		int previous_vertex = i - 1;
		if (previous_vertex == -1)
		 previous_vertex = dsh->outline_vertices - 1;
		int next_vertex = i + 1;
		if (next_vertex == dsh->outline_vertices)
			next_vertex = 0;

		dsh->outline_vertex_sides [i] [0] [0] = (dsh->outline_vertex_pos[i][0]	+ dsh->outline_vertex_pos[previous_vertex][0]) / 2;
		dsh->outline_vertex_sides [i] [0] [1] = (dsh->outline_vertex_pos[i][1]	+ dsh->outline_vertex_pos[previous_vertex][1]) / 2;

		dsh->outline_vertex_sides [i] [1] [0] = (dsh->outline_vertex_pos[i][0]	+ dsh->outline_vertex_pos[next_vertex][0]) / 2;
		dsh->outline_vertex_sides [i] [1] [1] = (dsh->outline_vertex_pos[i][1]	+ dsh->outline_vertex_pos[next_vertex][1]) / 2;

	}



// This is a bit of a hack that makes sure links that are supposed to be symmetrical are actually symmetrical
 if (dshape_init.current_nshape >= FIRST_NONCORE_SHAPE)
	{
  for (i = 0; i < nsh->links; i ++)
	 {
		 if (nsh->link_angle_fixed [i] > 0 && nsh->link_angle_fixed [i] < AFX_ANGLE_2)
			{
				int mirrored_object = nsh->mirrored_object_noncentre [i];
				nsh->link_angle_fixed [mirrored_object] = AFX_ANGLE_1 - nsh->link_angle_fixed [i];
				nsh->link_dist_fixed [mirrored_object] = nsh->link_dist_fixed [i];
				nsh->link_dist_pixel [mirrored_object] = nsh->link_dist_pixel [i];
				nsh->object_angle_fixed [mirrored_object] =  AFX_ANGLE_1 - nsh->object_angle_fixed [i];
				nsh->object_dist_fixed [mirrored_object] = nsh->object_dist_fixed [i];
				nsh->object_dist_pixel [mirrored_object] = nsh->object_dist_pixel [i];
			}
	 }
	}
	 else
		{
			if (dshape_init.current_nshape >= FIRST_MOBILE_NSHAPE)
			{
    for (i = 0; i < nsh->links; i ++)
	   {
// 		 if (nsh->link_angle_fixed [i] > AFX_ANGLE_2)
			  {
			  	int mirrored_object = nsh->mirrored_object_centreline [0] [i];
//			  	fpr("\n nsh %i link %i mo %i", dshape_init.current_nshape, i, mirrored_object);
			  	if (mirrored_object < 0)
			  	 continue;
			  	nsh->link_angle_fixed [mirrored_object] = AFX_ANGLE_1 - nsh->link_angle_fixed [i];
			  	nsh->link_dist_fixed [mirrored_object] = nsh->link_dist_fixed [i];
			  	nsh->link_dist_pixel [mirrored_object] = nsh->link_dist_pixel [i];
			  	nsh->object_angle_fixed [mirrored_object] = AFX_ANGLE_1 - nsh->object_angle_fixed [i];
			  	nsh->object_dist_fixed [mirrored_object] = nsh->object_dist_fixed [i];
			  	nsh->object_dist_pixel [mirrored_object] = nsh->object_dist_pixel [i];
			  }
	   }
			}
		}



#ifdef Z_POLY
 zshape_end(); // put this at end of code to create shape
#endif


/*
 struct dshape_struct* dsh = &dshape[dshape_init.current_dshape];

 int left_display_vertex, right_display_vertex, middle_display_vertex;
 int poly;
 int i;
 float vx, vy, vx2, vy2, vx3, vy3, v_angle, v_dist;

 for (i = 0; i < dsh->links; i ++)
	{

		poly = dsh->link_poly [i];

		if (poly == -1)
			continue; // link has already been set up.

		middle_display_vertex = dsh->link_display_vertex [i];

	 left_display_vertex = middle_display_vertex - 1;
	 if (left_display_vertex < 0)
 		left_display_vertex = dsh->display_vertices [poly] - 1;

 	right_display_vertex = middle_display_vertex + 1;
	  if (right_display_vertex >= dsh->display_vertices [poly])
 			right_display_vertex = 0;

#define LINK_POINT_SEPARATION 2
//#define LINK_OBJECT_SEPARATION 8
#define LINK_OBJECT_SEPARATION OBJECT_EXTRA_DIST
 	dsh->link_point_angle	[i] [1] = dsh->display_vertex_angle [poly] [middle_display_vertex];
 	dsh->link_point_dist	[i] [1] = dsh->display_vertex_dist [poly] [middle_display_vertex] + LINK_POINT_SEPARATION;
 	dsh->link_point_angle	[i] [3] = dsh->display_vertex_angle [poly] [middle_display_vertex];
 	dsh->link_point_dist	[i] [3] = dsh->display_vertex_dist [poly] [middle_display_vertex] + LINK_POINT_SEPARATION + 8;
 	dsh->link_object_angle	[i] = dsh->display_vertex_angle [poly] [middle_display_vertex];
// 	fpr("\n dsh %i loa [%i] (mdv %i) = %f", dshape_init.current_dshape, i, middle_display_vertex, dsh->link_object_angle	[i]);
 	dsh->link_object_dist	[i] = dsh->display_vertex_dist [poly] [middle_display_vertex] + LINK_OBJECT_SEPARATION;

// find location of middle vertex:
  vx = cos(dsh->display_vertex_angle [poly] [middle_display_vertex]) * dsh->display_vertex_dist [poly] [middle_display_vertex];
  vy = sin(dsh->display_vertex_angle [poly] [middle_display_vertex]) * dsh->display_vertex_dist [poly] [middle_display_vertex];

// find location of left vertex:
  vx2 = cos(dsh->display_vertex_angle [poly] [left_display_vertex]) * dsh->display_vertex_dist [poly] [left_display_vertex];
  vy2 = sin(dsh->display_vertex_angle [poly] [left_display_vertex]) * dsh->display_vertex_dist [poly] [left_display_vertex];
// find angle/dist from middle to left vertex:
  v_angle = atan2(vy2 - vy, vx2 - vx);
  v_dist = hypot(vy2 - vy, vx2 - vx);
// new point is 80% of the way to the left vertex
  vx3 = cos(dsh->link_point_angle	[i] [1]) * dsh->link_point_dist	[i] [1]
        + cos(v_angle) * v_dist * 0.7;//(v_dist * 80) / 100);
  vy3 = sin(dsh->link_point_angle	[i] [1]) * dsh->link_point_dist	[i] [1]
        + sin(v_angle) * v_dist * 0.7;//(v_dist * 80) / 100);
 	dsh->link_point_angle	[i] [0] = atan2(vy3, vx3);
 	dsh->link_point_dist	[i] [0] = hypot(vy3, vx3);
// 	fpr("\n L %f,%f %f,%f, %f,%f", al_fixtof(vx), al_fixtof(vy), al_fixtof(vx2), al_fixtof(vy2), al_fixtof(vx3), al_fixtof(vx3));

// find location of right vertex:
  vx2 = cos(dsh->display_vertex_angle [poly] [right_display_vertex]) * dsh->display_vertex_dist [poly] [right_display_vertex];
  vy2 = sin(dsh->display_vertex_angle [poly] [right_display_vertex]) * dsh->display_vertex_dist [poly] [right_display_vertex];
// find angle/dist from middle to right vertex:
  v_angle = atan2(vy2 - vy, vx2 - vx);
  v_dist = hypot(vy2 - vy, vx2 - vx);
// new point is 80% of the way to the right vertex
  vx3 = cos(dsh->link_point_angle	[i] [1]) * dsh->link_point_dist	[i] [1]
        + cos(v_angle) * v_dist * 0.7;//(v_dist * 80) / 100);
  vy3 = sin(dsh->link_point_angle	[i] [1]) * dsh->link_point_dist	[i] [1]
        + sin(v_angle) * v_dist * 0.7;//(v_dist * 80) / 100);
 	dsh->link_point_angle	[i] [2] = atan2(vy3, vx3);
 	dsh->link_point_dist	[i] [2] = hypot(vy3, vx3);
// 	fpr("\n R %i,%i %i,%i, %i,%i", al_fixtoi(vx), al_fixtoi(vy), al_fixtoi(vx2), al_fixtoi(vy2), al_fixtoi(vx3), al_fixtoi(vx3));


	} // end i loop
*/

}

// wrapper function around add_link_at_xy_fixed() which converts ints to fixed
// basically exists to make coding shapes easier
static void add_link_at_xy(int link_index, int link_x, int link_y, int left_x, int left_y, int right_x, int right_y, int far_x, int far_y, int link_point_x, int link_point_y, int object_x, int object_y)
{

#ifdef Z_POLY
 zshape_add_link(link_x,
																	link_y,
																	left_x,
																	left_y,
																	right_x,
																	right_y,
																	far_x,
																	far_y,
																	link_point_x,
																	link_point_y,
																	object_x,
																	object_y);
#endif



 add_link_at_xy_fixed(link_index,
																						al_itofix(link_x), al_itofix(link_y),
																						al_itofix(left_x), al_itofix(left_y),
																						al_itofix(right_x), al_itofix(right_y),
																						al_itofix(far_x), al_itofix(far_y),
																						al_itofix(link_point_x), al_itofix(link_point_y),
																						al_itofix(object_x), al_itofix(object_y));


}

void add_links(int num_links, const int* coords)
{
	int i;
	for(i = 0; i < num_links; ++i)
	{
		add_link_at_xy_fixed(i, al_itofix(coords[12*i]), al_itofix(coords[12*i+1]),
							 al_itofix(coords[12*i+2]), al_itofix(coords[12*i+3]),
							 al_itofix(coords[12*i+4]), al_itofix(coords[12*i+5]),
							 al_itofix(coords[12*i+6]), al_itofix(coords[12*i+7]),
							 al_itofix(coords[12*i+8]), al_itofix(coords[12*i+9]),
							 al_itofix(coords[12*i+10]), al_itofix(coords[12*i+11]));
	}
}

// wrapper function around add_link_at_xy_fixed()
// basically exists to make coding shapes easier
static void add_link_vector(int link_index, int link_angle, int link_dist, int left_angle, int left_dist, int right_angle, int right_dist, int link_extra_distance, int object_extra_distance, int far_point_distance)
{

 al_fixed link_angle_fixed = int_angle_to_fixed(link_angle);
 al_fixed link_dist_fixed = al_itofix(link_dist);

 al_fixed link_x_fixed = fixed_xpart(link_angle_fixed, link_dist_fixed);
 al_fixed link_y_fixed = fixed_ypart(link_angle_fixed, link_dist_fixed);

 add_link_at_xy_fixed(link_index,
																						link_x_fixed, link_y_fixed,
																						link_x_fixed + fixed_xpart(int_angle_to_fixed(link_angle + left_angle), al_itofix(left_dist)),
																						link_y_fixed + fixed_ypart(int_angle_to_fixed(link_angle + left_angle), al_itofix(left_dist)),
																						link_x_fixed + fixed_xpart(int_angle_to_fixed(link_angle + right_angle), al_itofix(right_dist)),
																						link_y_fixed + fixed_ypart(int_angle_to_fixed(link_angle + right_angle), al_itofix(right_dist)),
																						link_x_fixed + fixed_xpart(int_angle_to_fixed(link_angle), al_itofix(far_point_distance)),
																						link_y_fixed + fixed_ypart(int_angle_to_fixed(link_angle), al_itofix(far_point_distance)),
																						link_x_fixed + fixed_xpart(int_angle_to_fixed(link_angle), al_itofix(link_extra_distance)),
																						link_y_fixed + fixed_ypart(int_angle_to_fixed(link_angle), al_itofix(link_extra_distance)),
																						link_x_fixed + fixed_xpart(int_angle_to_fixed(link_angle), al_itofix(object_extra_distance)),
																						link_y_fixed + fixed_ypart(int_angle_to_fixed(link_angle), al_itofix(object_extra_distance)));

}

// puts a link at specific location.
// Use this to place links that are not connected to specific polys
// links added this way are skipped by fix_links()
static void add_link_at_xy_fixed(int link_index, al_fixed link_x, al_fixed link_y, al_fixed left_x, al_fixed left_y, al_fixed right_x, al_fixed right_y, al_fixed far_x, al_fixed far_y, al_fixed link_point_x, al_fixed link_point_y, al_fixed object_x, al_fixed object_y)
{
//	int link_index = nshape[dshape_init.current_nshape].links;
	nshape[dshape_init.current_nshape].link_angle_fixed [link_index] = get_angle(link_point_y, link_point_x);
	nshape[dshape_init.current_nshape].link_dist_fixed [link_index] = distance(link_point_y, link_point_x);// + al_itofix(link_extra_distance);
	nshape[dshape_init.current_nshape].link_dist_pixel [link_index] = al_fixtoi(distance(link_point_y, link_point_x));
	nshape[dshape_init.current_nshape].object_angle_fixed [link_index] = get_angle(object_y, object_x);
	nshape[dshape_init.current_nshape].object_dist_fixed [link_index] = distance(object_y, object_x);
	nshape[dshape_init.current_nshape].object_dist_pixel [link_index] = al_fixtoi(distance(object_y, object_x));

 struct dshape_struct* dsh = &dshape[dshape_init.current_nshape];

//	dsh->link_poly [link_index] = -1; // this prevents the link values being updated by fix_links
//	dshape[dshape_init.current_dshape].link_display_vertex [link_index] = dshape_init.last_display_vertex;

 if (nshape[dshape_init.current_nshape].links <= link_index)
	{
		nshape[dshape_init.current_nshape].links = link_index + 1;
		dsh->links = link_index + 1;
	}

// float vx, vy, vx2, vy2, vx3, vy3, v_angle, v_dist;

//#define LINK_POINT_SEPARATION 2
//#define LINK_OBJECT_SEPARATION 8
//#define LINK_OBJECT_SEPARATION OBJECT_EXTRA_DIST
// ^^^ these are defined above

// floats are okay here because we're working out dshape (display) not nshape (gameplay-affecting) values
 	dsh->link_point_angle	[link_index] [0] = atan2(al_fixtof(left_y), al_fixtof(left_x)); //dsh->display_vertex_angle [poly] [middle_display_vertex];
 	dsh->link_point_dist	[link_index] [0] = hypot(al_fixtof(left_y), al_fixtof(left_x));// + LINK_POINT_SEPARATION; //dsh->display_vertex_dist [poly] [middle_display_vertex] + LINK_POINT_SEPARATION;
 	dsh->link_point_angle	[link_index] [1] = atan2(al_fixtof(link_y), al_fixtof(link_x)); //dsh->display_vertex_angle [poly] [middle_display_vertex];
 	dsh->link_point_dist	[link_index] [1] = hypot(al_fixtof(link_y), al_fixtof(link_x));// + LINK_POINT_SEPARATION; //dsh->display_vertex_dist [poly] [middle_display_vertex] + LINK_POINT_SEPARATION;
 	dsh->link_point_angle	[link_index] [2] = atan2(al_fixtof(right_y), al_fixtof(right_x)); //dsh->display_vertex_angle [poly] [middle_display_vertex];
 	dsh->link_point_dist	[link_index] [2] = hypot(al_fixtof(right_y), al_fixtof(right_x));// + LINK_POINT_SEPARATION; //dsh->display_vertex_dist [poly] [middle_display_vertex] + LINK_POINT_SEPARATION;
 	dsh->link_point_angle	[link_index] [3] = atan2(al_fixtof(far_y), al_fixtof(far_x)); //dsh->display_vertex_angle [poly] [middle_display_vertex];
 	dsh->link_point_dist	[link_index] [3] = hypot(al_fixtof(far_y), al_fixtof(far_x)); //dsh->display_vertex_dist [poly] [middle_display_vertex] + LINK_POINT_SEPARATION;

 	dsh->link_object_angle	[link_index] = atan2(al_fixtof(object_y), al_fixtof(object_x)); //dsh->display_vertex_angle [poly] [middle_display_vertex];
// 	fpr("\n dsh %i loa [%i] (mdv %i) = %f", dshape_init.current_dshape, i, middle_display_vertex, dsh->link_object_angle	[i]);
 	dsh->link_object_dist	[link_index] = hypot(al_fixtof(object_y), al_fixtof(object_x)); //dsh->display_vertex_dist [poly] [middle_display_vertex] + LINK_OBJECT_SEPARATION;

 	dsh->link_point_side_angle [link_index] [0] = atan2(al_fixtof(left_y) - al_fixtof(link_y), al_fixtof(left_x) - al_fixtof(link_x));
 	dsh->link_point_side_dist [link_index] [0] = hypot(al_fixtof(left_y) - al_fixtof(link_y), al_fixtof(left_x) - al_fixtof(link_x));
 	dsh->link_point_side_angle [link_index] [1] = atan2(al_fixtof(right_y) - al_fixtof(link_y), al_fixtof(right_x) - al_fixtof(link_x));
 	dsh->link_point_side_dist [link_index] [1] = hypot(al_fixtof(right_y) - al_fixtof(link_y), al_fixtof(right_x) - al_fixtof(link_x));

  dsh->link_outer_angle [link_index] = atan2(al_fixtof(far_y - link_y), al_fixtof(far_x - link_x));
  dsh->link_outer_dist [link_index] = hypot(al_fixtof(far_y - link_y), al_fixtof(far_x - link_x));

// fpr("\n adding link %i to shape %i angle %i dist %i (links %i)", link_index, dshape_init.current_nshape, al_fixtoi(nshape[dshape_init.current_nshape].link_angle_fixed [link_index]), al_fixtoi(nshape[dshape_init.current_nshape].object_dist_fixed [link_index]), nshape[dshape_init.current_nshape].links);

}




// opposite_link_index can be -1 if link_index is not opposite another link
void add_mirror_axis_at_link(int link_index, int opposite_link_index)
{
#ifdef SANITY_CHECK
	if (nshape[dshape_init.current_nshape].mirror_axes >= MIRROR_AXES)
	{
		fpr("\n Error: g_shapes.c: add_mirror_axis(): too many mirror axes on shape %i", dshape_init.current_nshape);
		error_call();
	}
#endif

	int cma = nshape[dshape_init.current_nshape].mirror_axes;

 nshape[dshape_init.current_nshape].mirror_axis_angle [cma] = fixed_angle_to_int(nshape[dshape_init.current_nshape].link_angle_fixed [link_index]);

 if (abs(nshape[dshape_init.current_nshape].mirror_axis_angle [cma]) < 16)
		nshape[dshape_init.current_nshape].mirror_axis_angle [cma] = 0; // to avoid rounding errors at front-facing vertex


// int counter = 0;

// int i = link_index - 1;
// int j = link_index + 1;


 nshape[dshape_init.current_nshape].mirrored_object_centreline [cma] [link_index] = -1;
// if (opposite_link_index == -1)
//		opposite_link_index = -100; // will prevent it being found

 calculate_mirrored_objects_for_mirror_axis(cma, link_index, opposite_link_index);


 nshape[dshape_init.current_nshape].mirror_axes ++;


}

 int mirrored_object [MIRROR_AXES] [MAX_OBJECTS];

// This function assumes that the axis is not at a vertex
//  - opposite_vertex can be -1 if no opposite vertex either
void add_mirror_axis(int axis_angle, int opposite_vertex)
{
#ifdef SANITY_CHECK
	if (nshape[dshape_init.current_nshape].mirror_axes >= MIRROR_AXES)
	{
		fpr("\n Error: g_shapes.c: add_mirror_axis(): too many core mirror axes on shape %i", dshape_init.current_nshape);
		error_call();
	}
#endif

	int cma = nshape[dshape_init.current_nshape].mirror_axes;

 nshape[dshape_init.current_nshape].mirror_axis_angle [cma] = axis_angle & ANGLE_MASK;

 calculate_mirrored_objects_for_mirror_axis(cma, -1, opposite_vertex); // -1 means no front vertex
/*
 int counter = 0;

 int i = previous_object;
 int j = previous_object + 1;

 while(counter < nshape[dshape_init.current_nshape].links)
	{
		if (i < 0)
			i += nshape[dshape_init.current_nshape].links;
		if (j >= nshape[dshape_init.current_nshape].links)
			j -= nshape[dshape_init.current_nshape].links;

		nshape[dshape_init.current_nshape].mirrored_object [cma] [i] = j;

		i --;
		j ++;
		counter ++;

	};*/

 nshape[dshape_init.current_nshape].mirror_axes ++;

}


// This is a horrible brute-force way of finding which objects match each other when a core is being made symmetrical.
// I tried to work out a better way but couldn't.
// But it's only called during initialisation, so that's okay.
// front/back_vertex links are ignored (neither mirrored nor removed). Set to -1 if not relevant to this axis.
static void calculate_mirrored_objects_for_mirror_axis(int axis_index, int front_vertex, int back_vertex)
{

	int i, j;
	int object_angle;
	int reflected_object_angle;
	int test_object_angle;
	int angle_diff;
	int closest_angle_diff;
	int closest_object;

	for (i = 0; i < nshape[dshape_init.current_nshape].links; i ++)
	{
		if (i == front_vertex || i == back_vertex)
		{
 		nshape[dshape_init.current_nshape].mirrored_object_centreline [axis_index] [i] = -1; // -1 means this object is ignored when making process symmetrical
 		continue;
		}

  object_angle = (fixed_angle_to_int(nshape[dshape_init.current_nshape].link_angle_fixed [i]) - nshape[dshape_init.current_nshape].mirror_axis_angle [axis_index]) & ANGLE_MASK;

/*  if ((object_angle >= ANGLE_1 - 32
			|| object_angle <= 32)
			|| (object_angle >= ANGLE_2 - 32
				&& object_angle <= ANGLE_2 + 32)) // -4/+4 are there in case the object isn't perfectly aligned due to fixed/int conversion
		{
 		nshape[dshape_init.current_nshape].mirrored_object_centreline [axis_index] [i] = -1; // -1 means this object is ignored when making process symmetrical (because it's on the line)
 		continue;
		}*/
  if (object_angle <= ANGLE_2) // -64 to allow for rounding errors
		{
 		nshape[dshape_init.current_nshape].mirrored_object_centreline [axis_index] [i] = -2; // -2 means this object is deleted when making process symmetrical (because it's below the line)
 		continue;
		}
  reflected_object_angle = (0 - object_angle) & ANGLE_MASK;
/*  if (object_angle == 0
			|| object_angle == ANGLE_2)
		{
 		nshape[dshape_init.current_nshape].mirrored_object_centreline [axis_index] [i] = -1;
 		continue;
		}*/

	 closest_object = -1;
	 closest_angle_diff = 10000;
	 for (j = 0; j < nshape[dshape_init.current_nshape].links; j ++)
		{
			if (j == i)
				continue;
			test_object_angle = (fixed_angle_to_int(nshape[dshape_init.current_nshape].link_angle_fixed [j]) - nshape[dshape_init.current_nshape].mirror_axis_angle [axis_index]) & ANGLE_MASK;
			angle_diff = angle_difference_int(test_object_angle, reflected_object_angle);
			if (angle_diff < closest_angle_diff)
			{
				closest_angle_diff = angle_diff;
				closest_object = j;
			}
		}
		nshape[dshape_init.current_nshape].mirrored_object_centreline [axis_index] [i] = closest_object;
	}
/*
 if (dshape_init.current_dshape == NSHAPE_CORE_STATIC_HEX)
	{
  fpr("\n axis %i angle %i (%i,%i): ", axis_index, nshape[dshape_init.current_nshape].core_mirror_axis_angle [axis_index], front_vertex, back_vertex);
 for (i = 0; i < nshape[dshape_init.current_nshape].links; i ++)
	{
		fpr(" (%i %i)", i, nshape[dshape_init.current_nshape].mirrored_object_centreline [axis_index] [i]);
	}
	}*/

}


unsigned char nshape_collision_mask [NSHAPES] [COLLISION_MASK_SIZE] [COLLISION_MASK_SIZE];

// For now we will just draw on collision vertices.
// This may or may not work.

void init_nshape_collision_masks(void)
{

 int x, y, s, v;
 al_fixed xa, ya, xb = 0, yb = 0;
 int poly = 0;
	int next_vertex;

//  int total_area = 0;
//  int base_area = 0;


 for (s = 0; s < NSHAPES; s ++)
 {
  for (x = 0; x < COLLISION_MASK_SIZE; x ++)
  {
   for (y = 0; y < COLLISION_MASK_SIZE; y ++)
   {
    nshape_collision_mask [s] [x] [y] = 0;//COLLISION_MASK_LEVELS;
   }
  }

// first draw the outline with value 1
//  - this is used for some purposes (e.g. bullet collisions) but ignored for others (e.g. proc-proc collisions)
  for (v = 0; v < dshape[s].outline_vertices; v ++)
		{
   	next_vertex = v + 1;
   	if (next_vertex >= dshape[s].outline_vertices)
					next_vertex = 0;


// the * 13 / 10 is there because interfaces are scaled up from the basic outline values by 1.3
    xa = MASK_CENTRE_FIXED + fixed_xpart(dshape[s].outline_vertex_angle_fixed [v], (dshape[s].outline_vertex_dist_fixed [v] * 13) / 10);
    xa = al_itofix(al_fixtoi(xa) >> COLLISION_MASK_BITSHIFT);
    ya = MASK_CENTRE_FIXED + fixed_ypart(dshape[s].outline_vertex_angle_fixed [v], (dshape[s].outline_vertex_dist_fixed [v] * 13) / 10);
    ya = al_itofix(al_fixtoi(ya) >> COLLISION_MASK_BITSHIFT);
    xb = MASK_CENTRE_FIXED + fixed_xpart(dshape[s].outline_vertex_angle_fixed [next_vertex], (dshape[s].outline_vertex_dist_fixed [next_vertex] * 13) / 10);
    xb = al_itofix(al_fixtoi(xb) >> COLLISION_MASK_BITSHIFT);
    yb = MASK_CENTRE_FIXED + fixed_ypart(dshape[s].outline_vertex_angle_fixed [next_vertex], (dshape[s].outline_vertex_dist_fixed [next_vertex] * 13) / 10);
    yb = al_itofix(al_fixtoi(yb) >> COLLISION_MASK_BITSHIFT);

    draw_line_on_nshape_mask(s, 1, xa, ya, xb, yb);		// note -1
		}

   floodfill_nshape_mask(s, 1,
																									MASK_CENTRE >> COLLISION_MASK_BITSHIFT,
																									MASK_CENTRE >> COLLISION_MASK_BITSHIFT); // outline should include MASK_CENTRE, MASK_CENTRE

//if (s == NSHAPE_COMPONENT_CAP)
//  test_draw_mask(s);


  for (poly = 0; poly < collision_mask_poly[s].polys; poly ++)
  {
   for (v = 0; v < collision_mask_poly[s].vertices[poly]; v ++)
   {
   	next_vertex = v + 1;
   	if (next_vertex >= collision_mask_poly[s].vertices[poly])
					next_vertex = 0;


    xa = MASK_CENTRE_FIXED + al_itofix(collision_mask_poly[s].vertex_x[poly][v]);
    xa = al_itofix(al_fixtoi(xa) >> COLLISION_MASK_BITSHIFT);
    ya = MASK_CENTRE_FIXED + al_itofix(collision_mask_poly[s].vertex_y[poly][v]);
    ya = al_itofix(al_fixtoi(ya) >> COLLISION_MASK_BITSHIFT);
    xb = MASK_CENTRE_FIXED + al_itofix(collision_mask_poly[s].vertex_x[poly][next_vertex]);
    xb = al_itofix(al_fixtoi(xb) >> COLLISION_MASK_BITSHIFT);
    yb = MASK_CENTRE_FIXED + al_itofix(collision_mask_poly[s].vertex_y[poly][next_vertex]);
    yb = al_itofix(al_fixtoi(yb) >> COLLISION_MASK_BITSHIFT);


    draw_line_on_nshape_mask(s, poly+2, xa, ya, xb, yb);

   }
//test_draw_mask(s);
   floodfill_nshape_mask(s, poly+2,
																									(MASK_CENTRE + collision_mask_poly[s].fill_source_x[poly]) >> COLLISION_MASK_BITSHIFT,
																									(MASK_CENTRE + collision_mask_poly[s].fill_source_y[poly]) >> COLLISION_MASK_BITSHIFT);

//																									fpr("\n floodfill s %i poly %i at %i,%i", s, poly,
//																									collision_mask_poly[s].fill_source_x[poly],
//																									collision_mask_poly[s].fill_source_y[poly]);
//test_draw_mask(s);
//if (s == NSHAPE_COMPONENT_CAP)
//  test_draw_mask(s);


  }

/*
  for (poly = 0; poly < dshape[s].polys; poly ++)
  {

// we start by drawing lines around the edges of the shape:
   for (v = 0; v < nshape [s].vertices - 1; v ++)
   {
    xa = MASK_CENTRE_FIXED + fixed_xpart(nshape [s].vertex_angle_fixed [v], nshape [s].vertex_dist_fixed [v]);
    xa = al_itofix(al_fixtoi(xa) >> COLLISION_MASK_BITSHIFT);
    ya = MASK_CENTRE_FIXED + fixed_ypart(nshape [s].vertex_angle_fixed [v], nshape [s].vertex_dist_fixed [v]);
    ya = al_itofix(al_fixtoi(ya) >> COLLISION_MASK_BITSHIFT);
    xb = MASK_CENTRE_FIXED + fixed_xpart(nshape [s].vertex_angle_fixed [v+1], nshape [s].vertex_dist_fixed [v+1]);
    xb = al_itofix(al_fixtoi(xb) >> COLLISION_MASK_BITSHIFT);
    yb = MASK_CENTRE_FIXED + fixed_ypart(nshape [s].vertex_angle_fixed [v+1], nshape [s].vertex_dist_fixed [v+1]);
    yb = al_itofix(al_fixtoi(yb) >> COLLISION_MASK_BITSHIFT);


    draw_line_on_nshape_mask(s, poly+1, xa, ya, xb, yb);

   }

   v = 0;

    xa = MASK_CENTRE_FIXED + fixed_xpart(nshape [s].vertex_angle_fixed [v], nshape [s].vertex_dist_fixed [v]);
    xa = al_itofix(al_fixtoi(xa) >> COLLISION_MASK_BITSHIFT);
    ya = MASK_CENTRE_FIXED + fixed_ypart(nshape [s].vertex_angle_fixed [v], nshape [s].vertex_dist_fixed [v]);
    ya = al_itofix(al_fixtoi(ya) >> COLLISION_MASK_BITSHIFT);
    draw_line_on_nshape_mask(s, poly+1, xa, ya, xb, yb);
test_draw_mask(s);
   floodfill_nshape_mask(s, poly+1, MASK_CENTRE >> COLLISION_MASK_BITSHIFT, MASK_CENTRE >> COLLISION_MASK_BITSHIFT);

  }
  */
/*
  int i, j;
  total_area = 0;
  for (i = 0; i < COLLISION_MASK_SIZE; i ++)
		{
			for (j = 0; j < COLLISION_MASK_SIZE; j ++)
			{
				if (collision_mask [s] [i] [j] <= 2)
					total_area ++;
			}
		}
*/

 } // end of NSHAPES loop


}


static void draw_line_on_nshape_mask(int s, int level, al_fixed xa, al_fixed ya, al_fixed xb, al_fixed yb)
{

// fprintf(stdout, "\n line s %i level %i (%i, %i) to (%i, %i)\n", s, level, al_fixtoi(xa), al_fixtoi(ya), al_fixtoi(xb), al_fixtoi(yb));
 int x, y;

 int inc_x, inc_y;

 set_nshape_mask_pixel(s, level, al_fixtoi(xa), al_fixtoi(ya));
 set_nshape_mask_pixel(s, level, al_fixtoi(xb), al_fixtoi(yb));


 if (xa == xb)
 {
  if (ya == yb)
  {
//   collision_mask [s] [size] [xa] [ya] = 1;
//   set_mask_pixel(s, size, xa, ya);
   return;
  }
  if (ya < yb)
   inc_y = 1;
    else
     inc_y = -1;
  y = al_fixtoi(ya);
  set_nshape_mask_pixel(s, level, al_fixtoi(xa), y);
  while (y != al_fixtoi(yb))
  {
   y += inc_y;
   set_nshape_mask_pixel(s, level, al_fixtoi(xa), y);
  };
  return;
 }

 if (ya == yb)
 {
// don't need to allow for xa == xb as that will have been caught above
  if (xa < xb)
   inc_x = 1;
    else
     inc_x = -1;
  x = al_fixtoi(xa);
  set_nshape_mask_pixel(s, level, x, al_fixtoi(ya));
  while (x != al_fixtoi(xb))
  {
   x += inc_x;
   set_nshape_mask_pixel(s, level, x, al_fixtoi(ya));
  };
  return;
 }

 al_fixed dx, dy;
 al_fixed fx, fy;

 al_fixed temp_fixed;
 int timeout = 1000;

 if (xb < xa)
 {
// flip coordinates
  temp_fixed = xb;
  xb = xa;
  xa = temp_fixed;
  temp_fixed = yb;
  yb = ya;
  ya = temp_fixed;
 }

 fx = xa;
 fy = ya;
 x = al_fixtoi(xa);
 y = al_fixtoi(ya);
 dx = al_fixdiv(xb - xa, yb - ya);
 dy = al_fixdiv(yb - ya, xb - xa);

// fprintf(stdout, "\ndx = %f dy = %f\n", al_fixtof(dx), al_fixtof(dy));

 if (dy < al_itofix(1) && dy > al_itofix(-1))
 {
//  dy /= dx;
  while(x != al_fixtoi(xb) || y != al_fixtoi(yb))
  {
   x ++;
   fy += dy;
   y = al_fixtoi(fy);
   set_nshape_mask_pixel(s, level, x, y);
//   fprintf(stdout, "1st loop: (%i,%i) for (%i,%i) f(%f,%f)\n", x, y, xb, yb, dx, dy);
   timeout --;
   if (!timeout)
   {
//    test_draw_mask(s, 0, 0);
    fprintf(stdout, "\ng_shapes.c: draw_line_on_nshape_mask(): line draw timed out (shape %i level %i xy %i,%i xb,yb %i,%i xa,ya %i,%i dx,dy %f,%f).", s, level, x, y, al_fixtoi(xb), al_fixtoi(yb), al_fixtoi(xa), al_fixtoi(ya), al_fixtof(dx), al_fixtof(dy));
    error_call();
   }
   if (x == al_fixtoi(xb) && (y == al_fixtoi(yb) - 1 || y == al_fixtoi(yb) + 1))
    break;
   if (y == al_fixtoi(yb) && (x == al_fixtoi(xb) - 1 || x == al_fixtoi(xb) + 1))
    break;
  };
  set_nshape_mask_pixel(s, level, al_fixtoi(xb), al_fixtoi(yb));
  return;
 }
  else
  {
//   dx /= dy;
   if (dx < al_itofix(0))
    dx *= -1;
   while(x != al_fixtoi(xb) || y != al_fixtoi(yb))
   {
    if (dy > al_itofix(0))
     y ++;
      else
       y --;
    fx += dx;
    x = al_fixtoi(fx);
    set_nshape_mask_pixel(s, level, x, y);
//   fprintf(stdout, "2nd loop: (%i,%i) for (%i,%i) f(%f,%f)\n", x, y, xb, yb, dx, dy);
   if (x == al_fixtoi(xb) && (y == al_fixtoi(yb) - 1 || y == al_fixtoi(yb) + 1))
    break;
   if (y == al_fixtoi(yb) && (x == al_fixtoi(xb) - 1 || x == al_fixtoi(xb) + 1))
    break;
/*   timeout --;
   if (!timeout)
    error_call();*/
   };
   set_nshape_mask_pixel(s, level, al_fixtoi(xb), al_fixtoi(yb));
   return;
  }

/*
line drawing routine works like this:

we start off assuming that xa < xb (although ya may be > yb)

then we work out whether dx > dy

if dy is smaller:

then we add dy to fy
if (int) fy doesn't increase, we x++ and draw a point
if (int) fy increases by 1 we x++ and draw a point

if dx is smaller, we do the same but x/y flipped

*/

}

static void set_nshape_mask_pixel(int s, int level, int x, int y)
{

// fprintf(stdout, " (%i,%i)", x, y);

 if (x >= 0 && y >= 0 && x < COLLISION_MASK_SIZE && y < COLLISION_MASK_SIZE)
  nshape_collision_mask [s] [x] [y] = level;

}

// recursive floodfill function
static void floodfill_nshape_mask(int s, int level, int x, int y)
{

#ifdef SANITY_CHECK

 if (x < 0
  || x >= COLLISION_MASK_SIZE
  || y < 0
  || y >= COLLISION_MASK_SIZE)
 {
  fprintf(stdout, "\nError: g_shape.c: floodfill_mask(): out of bounds shape %i level %i", s, level);
  return;
//  error_call();
 }

#endif

 if (nshape_collision_mask [s] [x] [y] == level)
  return;

 nshape_collision_mask [s] [x] [y] = level;

 floodfill_nshape_mask(s, level, x - 1, y);
 floodfill_nshape_mask(s, level, x + 1, y);
 floodfill_nshape_mask(s, level, x, y - 1);
 floodfill_nshape_mask(s, level, x, y + 1);



}

#define TEST_MASK

#ifdef TEST_MASK

#include "i_header.h"

extern ALLEGRO_DISPLAY* display;
//extern ALLEGRO_COLOR base_col [BASIC_COLS] [BASIC_SHADES];
extern struct coloursstruct colours;

// debug function
void test_draw_mask(int s)
{

 al_set_target_bitmap(al_get_backbuffer(display));

 al_clear_to_color(colours.base [0] [0]);

 ALLEGRO_COLOR mask_colour [25];
 int i;
 for (i = 0; i < 25; i ++)
 {
  mask_colour [i] = al_map_rgb(i * 10, i * ((i % 2)) * 5, 100);
 }

 int x, y;
 int xa, ya;

 for (x = 0; x < COLLISION_MASK_SIZE; x ++)
 {
  for (y = 0; y < COLLISION_MASK_SIZE; y ++)
  {
    xa = 10 + x * 3;
    ya = 10 + y * 3;
    if (nshape_collision_mask [s] [x] [y])
     al_draw_filled_rectangle(xa, ya, xa + 2, ya + 2, mask_colour [nshape_collision_mask [s] [x] [y]]);
      else
       al_draw_filled_rectangle(xa, ya, xa + 2, ya + 2, colours.base [COL_GREEN] [SHADE_LOW]);

//      else
//       al_draw_filled_rectangle(xa, ya, xa + 2, ya + 2, mask_colour [collision_mask [s] [x] [y]]);
  }
 }


 al_flip_display();
// wait_for_space();

// error_call();
 wait_for_space();

}

#endif



/*

Everything below here is part of the special shape-drawing code


* /

extern ALLEGRO_EVENT_QUEUE* event_queue; // these queues are initialised in main.c

#define SD_LAYERS 3
#define SD_VERTICES 24
#define SD_GRID_SIZE 4
#define SD_TOTAL_SIZE 100
#define SD_CENTRE 50

struct sdstate_struct
{
	int vertex_exists [SD_LAYERS] [SD_VERTICES];
	int vertex_x [SD_LAYERS] [SD_VERTICES];
	int vertex_y [SD_LAYERS] [SD_VERTICES];

	int selected_vertex;
	int selected_vertex_layer;
	int dragging_vertex;

	int editing_layer;

};
struct sdstate_struct sdstate;

void draw_shapes(void)
{

 init_ex_control();

 flush_game_event_queues();

 int i, j, x, y;
 int mouse_x, mouse_y;

 for (i = 0; i < SD_LAYERS; i++)
	{
		for (j = 0; j < SD_VERTICES; j++)
		{
			sdstate.vertex_exists [i] [j] = 0;
		}
	}

	sdstate.selected_vertex = -1;
	sdstate.selected_vertex_layer = -1;
	sdstate.dragging_vertex = -1;
	sdstate.editing_layer = SD_LAYERS-1;


 ALLEGRO_EVENT ev;

 do // main game loop
 {

 al_set_target_bitmap(al_get_backbuffer(display));
 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
 al_clear_to_color(colours.world_background);

  get_ex_control(0); // ex_control needs to be updated even when halted (control will not be updated)
  run_input(); // this derives control (available to programs) from ex_control. doesn't need to be run for user to be able to use mode buttons and interact with sysmenu/editor/templates etc (as these use ex_control directly)

  mouse_x = ex_control.mouse_x_pixels / SD_GRID_SIZE;
  mouse_y = ex_control.mouse_y_pixels / SD_GRID_SIZE;


  for (i = 0; i < SD_TOTAL_SIZE; i++)
		{
   for (j = 0; j < SD_TOTAL_SIZE; j++)
			{
				al_draw_filled_rectangle(i * SD_GRID_SIZE, j * SD_GRID_SIZE, i * SD_GRID_SIZE + (SD_GRID_SIZE-1), j * SD_GRID_SIZE + (SD_GRID_SIZE-1), colours.base [COL_GREY] [SHADE_MED]);
			}
		}

		int vertex_col;

		for (i = 0; i < SD_LAYERS; i ++)
		{
			switch(i)
			{
				default:
				case 0: vertex_col = COL_BLUE; break;
				case 1: vertex_col = COL_AQUA; break;
				case 2: vertex_col = COL_TURQUOISE; break;
			}
			for (j = 0; j < SD_VERTICES; j ++)
			{
				if (sdstate.vertex_exists [i] [j] == 0)
					break;
				x = sdstate.vertex_x [i] [j] - SD_CENTRE;
				y = sdstate.vertex_y [i] [j] - SD_CENTRE;
				if (sdstate.selected_vertex_layer == i
					&& sdstate.selected_vertex == j)
				  al_draw_rectangle(x * SD_GRID_SIZE - 1, y * SD_GRID_SIZE - 1, x * SD_GRID_SIZE + (SD_GRID_SIZE-1) + 1, y * SD_GRID_SIZE + (SD_GRID_SIZE-1) + 1, colours.base [vertex_col] [SHADE_MAX], 0);

				if (mouse_x == i
				 && mouse_y == j)
				  al_draw_rectangle(x * SD_GRID_SIZE - 1, y * SD_GRID_SIZE - 1, x * SD_GRID_SIZE + (SD_GRID_SIZE-1) + 1, y * SD_GRID_SIZE + (SD_GRID_SIZE-1) + 1, colours.base [COL_YELLOW] [SHADE_MAX], 0);

				al_draw_filled_rectangle(x * SD_GRID_SIZE, y * SD_GRID_SIZE, x * SD_GRID_SIZE + (SD_GRID_SIZE-1), y * SD_GRID_SIZE + (SD_GRID_SIZE-1), colours.base [vertex_col] [SHADE_HIGH]);

			}
		}


  if (ex_control.mb_press [0] == BUTTON_JUST_PRESSED)

// wait for the timer so we can go to the next tick (unless we're fast-forwarding or the timer has already expired)
   al_wait_for_event(event_queue, &ev);

 } while (TRUE); // end main game loop




}

*/


