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
static void start_dshape_poly(int poly, int layer, int poly_colour_level);
static void add_vertex(int x, int y, int add_collision_vertex);
static void add_display_triangle(int v1, int v2, int v3);
static void add_vertex_vector(int angle, int dist, int add_collision_vertex);
//static void add_triple_link_vertex(int x, int y, int link_index);
//static void add_triple_link_vertex_vector(int angle, int dist, int previous_angle, int previous_angle_dist, int next_angle, int next_angle_dist, int link_index);
static void add_poly_fill_source(int x, int y);
//void add_link_vector(int angle, int dist);
static void fix_display_triangles_walk(void);
static void fix_display_triangles_fan(void);
//static void add_link_at_last_vertex(int link_index, int link_extra_distance, int object_extra_distance);
static void add_link_at_xy(int link_index, int link_x, int link_y, int left_x, int left_y, int right_x, int right_y, int far_x, int far_y, int link_point_x, int link_point_y, int object_x, int object_y);
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
{8, 200, 100, 14, 2}, // NSHAPE_CORE_STATIC_QUAD,
{14, 300, 120, 18, 3}, // NSHAPE_CORE_STATIC_PENT,
{28, 400, 180, 22, 4}, // NSHAPE_CORE_STATIC_HEX_A,
{64, 500, 240, 34, 5}, // NSHAPE_CORE_STATIC_HEX_B,
{128, 600, 300, 56, 6}, // NSHAPE_CORE_STATIC_HEX_C,


{30, 50, 50, 10, 1}, // NSHAPE_CORE_QUAD_A,
{50, 70, 60, 16, 1}, // NSHAPE_CORE_QUAD_B,
{80, 100, 70, 24, 2}, // NSHAPE_CORE_PENT_A,
{100, 140, 80, 30, 2}, // NSHAPE_CORE_PENT_B,
{130, 180, 90, 36, 3}, // NSHAPE_CORE_PENT_C,
{180, 220, 100, 42, 3}, // NSHAPE_CORE_HEX_A,
{230, 260, 110, 48, 4}, // NSHAPE_CORE_HEX_B,
{300, 300, 120, 54, 5}, // NSHAPE_CORE_HEX_C,


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

{6, 100, 0}, // NSHAPE_COMPONENT_TRI,
{6, 100, 0}, // NSHAPE_COMPONENT_FORK,
{9, 100, 0}, // NSHAPE_COMPONENT_BOX,
{9, 100, 0}, // NSHAPE_COMPONENT_LONG4,
{9, 100, 0}, // NSHAPE_COMPONENT_CAP,
{9, 100, 0}, // NSHAPE_COMPONENT_PRONG,
{12, 100, 0}, // NSHAPE_COMPONENT_LONG5,
{12, 100, 0}, // NSHAPE_COMPONENT_PEAK,
{12, 100, 0}, // NSHAPE_COMPONENT_SNUB,
{12, 100, 0}, // NSHAPE_COMPONENT_BOWL,
{16, 100, 0}, // NSHAPE_COMPONENT_LONG6,
{16, 100, 0}, // NSHAPE_COMPONENT_DROP,
{16, 100, 0}, // NSHAPE_COMPONENT_SIDE,



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
	add_link_at_xy(0, // link_index
	               22, 0, // centre
	               32, -9, // left
	               32, 9, // right
	               38, 0, // far
	               39, 0, // link
	               37, 0); // object

	add_link_at_xy(1, // link_index
	               0, 14, // centre
	               12, 21, // left
	               -11, 21, // right
	               0, 26, // far
	               0, 30, // link
	               0, 26); // object

	add_link_at_xy(2, // link_index
	               -32, 0, // centre
	               -41, 9, // left
	               -41, -9, // right
	               -49, 0, // far
	               -49, 0, // link
	               -47, 0); // object

	add_link_at_xy(3, // link_index
	               0, -14, // centre
	               -12, -21, // left
	               11, -21, // right
	               0, -26, // far
	               0, -30, // link
	               0, -26); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

	add_vertex(38, 0, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, 23, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-48, 0, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, -23, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-2, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(8, -1, 0);
	add_vertex(1, -8, 0);
	add_vertex(1, -12, 0);
	add_vertex(16, -21, 1);
	add_vertex(32, -12, 0);
	add_vertex(21, -1, 0);
	add_poly_fill_source(13, -9);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(8, 1, 0);
	add_vertex(1, 8, 0);
	add_vertex(1, 12, 0);
	add_vertex(16, 21, 1);
	add_vertex(31, 12, 0);
	add_vertex(21, 1, 0);
	add_poly_fill_source(13, 9);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-8, 1, 0);
	add_vertex(-1, 8, 0);
	add_vertex(-1, 12, 0);
	add_vertex(-18, 21, 0);
	add_vertex(-29, 20, 1);
	add_vertex(-35, 16, 0);
	add_vertex(-41, 12, 0);
	add_vertex(-31, 1, 0);
	add_poly_fill_source(-20, 11);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-8, -1, 0);
	add_vertex(-1, -8, 0);
	add_vertex(-1, -12, 0);
	add_vertex(-18, -21, 0);
	add_vertex(-29, -20, 1);
	add_vertex(-35, -16, 0);
	add_vertex(-41, -12, 0);
	add_vertex(-31, -1, 0);
	add_poly_fill_source(-20, -11);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_CORE_MUTABLE);

	add_vertex(0, 6, 0);
	add_vertex(-6, 0, 0);
	add_vertex(0, -6, 0);
	add_vertex(6, 0, 0);
	add_poly_fill_source(0, 0);
	fix_display_triangles_fan();


 add_mirror_axis_at_link(0, 2);
 add_mirror_axis_at_link(1, 3);
 add_mirror_axis_at_link(2, 0);
 add_mirror_axis_at_link(3, 1);

 add_mirror_axis(0, -1);
 add_mirror_axis(ANGLE_2, -1);


	finish_shape();

// ******* NSHAPE_CORE_QUAD_B *******



#ifdef Z_POLY
 zshape_start();
#endif



 start_dshape(NSHAPE_CORE_QUAD_B, KEYWORD_CORE_QUAD_B);

	poly_index = POLY_0;
	add_link_at_xy(0, // link_index
	               17, -13, // centre
	               9, -23, // left
	               27, -8, // right
	               25, -21, // far
	               26, -22, // link
	               23, -19); // object

	add_link_at_xy(1, // link_index
	               17, 13, // centre
	               27, 8, // left
	               9, 23, // right
	               25, 21, // far
	               26, 22, // link
	               23, 19); // object

	add_link_at_xy(2, // link_index
	               -11, 16, // centre
	               -8, 24, // left
	               -28, 14, // right
	               -19, 24, // far
	               -20, 26, // link
	               -18, 23); // object

	add_link_at_xy(3, // link_index
	               -11, -16, // centre
	               -28, -14, // left
	               -8, -24, // right
	               -19, -24, // far
	               -20, -26, // link
	               -18, -23); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

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
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(31, -4, 1);
	add_vertex(31, 3, 1);
	add_vertex(18, 11, 0);
	add_vertex(11, 5, 0);
	add_vertex(11, -6, 0);
	add_vertex(18, -11, 0);
	add_poly_fill_source(20, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-6, -26, 1);
	add_vertex(-9, -16, 0);
	add_vertex(-1, -7, 0);
	add_vertex(9, -7, 0);
	add_vertex(16, -12, 0);
	add_vertex(5, -26, 1);
	add_poly_fill_source(2, -15);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-6, 26, 1);
	add_vertex(-9, 16, 0);
	add_vertex(-1, 7, 0);
	add_vertex(9, 7, 0);
	add_vertex(16, 12, 0);
	add_vertex(5, 26, 1);
	add_poly_fill_source(2, 15);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-47, 7, 1);
	add_vertex(-47, -7, 1);
	add_vertex(-34, -12, 0);
	add_vertex(-11, -14, 0);
	add_vertex(-3, -5, 0);
	add_vertex(-3, 5, 0);
	add_vertex(-11, 14, 0);
	add_vertex(-34, 12, 0);
	add_poly_fill_source(-23, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_CORE_MUTABLE);

	add_vertex(9, -5, 0);
	add_vertex(9, 5, 0);
	add_vertex(-1, 5, 0);
	add_vertex(-1, -5, 0);
	add_poly_fill_source(4, 0);
	fix_display_triangles_fan();

 add_mirror_axis(0, -1);
 add_mirror_axis(ANGLE_2, -1);


	finish_shape();



// ******* NSHAPE_CORE_HEX_A *******

 start_dshape(NSHAPE_CORE_HEX_A, KEYWORD_CORE_HEX_A);

	poly_index = POLY_0;
	add_link_at_xy(0, // link_index
	               35, 0, // centre
	               44, -7, // left
	               44, 7, // right
	               51, 0, // far
	               53, 0, // link
	               49, 0); // object

	add_link_at_xy(1, // link_index
	               13, 18, // centre
	               25, 22, // left
	               7, 27, // right
	               18, 27, // far
	               20, 29, // link
	               17, 25); // object

	add_link_at_xy(2, // link_index
	               -13, 19, // centre
	               -8, 27, // left
	               -29, 23, // right
	               -21, 29, // far
	               -23, 31, // link
	               -21, 28); // object

	add_link_at_xy(3, // link_index
	               -38, 0, // centre
	               -47, 8, // left
	               -47, -8, // right
	               -54, 0, // far
	               -56, 0, // link
	               -52, 0); // object

	add_link_at_xy(4, // link_index
	               -13, -19, // centre
	               -29, -23, // left
	               -8, -27, // right
	               -21, -29, // far
	               -23, -31, // link
	               -21, -28); // object


	add_link_at_xy(5, // link_index
	               13, -18, // centre
	               7, -27, // left
	               25, -22, // right
	               18, -27, // far
	               20, -29, // link
	               17, -25); // object



	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

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
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_CORE_MUTABLE);

	add_vertex(9, 0, 0);
	add_vertex(5, 5, 0);
	add_vertex(-7, 5, 0);
	add_vertex(-10, 0, 0);
	add_vertex(-7, -5, 0);
	add_vertex(5, -5, 0);
	add_poly_fill_source(0, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-14, -17, 0);
	add_vertex(-38, -24, 1);
	add_vertex(-52, -14, 1);
	add_vertex(-37, -1, 0);
	add_vertex(-12, -1, 0);
	add_vertex(-8, -7, 0);
	add_poly_fill_source(-26, -10);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-14, 17, 0);
	add_vertex(-38, 24, 1);
	add_vertex(-52, 14, 1);
	add_vertex(-37, 1, 0);
	add_vertex(-12, 1, 0);
	add_vertex(-8, 7, 0);
	add_poly_fill_source(-26, 10);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-6, -7, 0);
	add_vertex(4, -7, 0);
	add_vertex(12, -17, 0);
	add_vertex(3, -29, 1);
	add_vertex(-5, -29, 1);
	add_vertex(-12, -17, 0);
	add_poly_fill_source(0, -17);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-6, 7, 0);
	add_vertex(4, 7, 0);
	add_vertex(12, 17, 0);
	add_vertex(3, 29, 1);
	add_vertex(-5, 29, 1);
	add_vertex(-12, 17, 0);
	add_poly_fill_source(0, 17);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(6, -7, 0);
	add_vertex(11, -1, 0);
	add_vertex(33, -1, 0);
	add_vertex(46, -11, 1);
	add_vertex(29, -21, 0);
	add_vertex(13, -16, 0);
	add_poly_fill_source(23, -9);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(6, 7, 0);
	add_vertex(11, 1, 0);
	add_vertex(33, 1, 0);
	add_vertex(46, 11, 1);
	add_vertex(29, 21, 0);
	add_vertex(13, 16, 0);
	add_poly_fill_source(23, 9);
	fix_display_triangles_fan();

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
	add_link_at_xy(0, // link_index
	               35, 0, // centre
	               44, -7, // left
	               44, 7, // right
	               51, 0, // far
	               53, 0, // link
	               49, 0); // object

	add_link_at_xy(1, // link_index
	               12, 21, // centre
	               25, 22, // left
	               6, 28, // right
	               18, 28, // far
	               20, 31, // link
	               18, 28); // object

	add_link_at_xy(2, // link_index
	               -12, 21, // centre
	               -6, 28, // left
	               -27, 24, // right
	               -17, 31, // far
	               -18, 34, // link
	               -17, 30); // object

	add_link_at_xy(3, // link_index
	               -38, 0, // centre
	               -47, 8, // left
	               -47, -8, // right
	               -54, 0, // far
	               -56, 0, // link
	               -52, 0); // object

	add_link_at_xy(4, // link_index
	               -12, -21, // centre
	               -27, -24, // left
	               -6, -28, // right
	               -17, -31, // far
	               -18, -34, // link
	               -17, -30); // object


	add_link_at_xy(5, // link_index
	               12, -21, // centre
	               6, -28, // left
	               25, -22, // right
	               18, -28, // far
	               20, -31, // link
	               18, -28); // object



	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

	add_vertex(48, 0, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(16, 25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-20, 25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-51, 0, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-20, -25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(16, -25, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-1, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_CORE_MUTABLE);

	add_vertex(8, 0, 0);
	add_vertex(3, 5, 0);
	add_vertex(-4, 5, 0);
	add_vertex(-9, 0, 0);
	add_vertex(-4, -5, 0);
	add_vertex(3, -5, 0);
	add_poly_fill_source(0, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-3, -7, 0);
	add_vertex(3, -7, 0);
	add_vertex(11, -19, 0);
	add_vertex(3, -29, 1);
	add_vertex(-3, -29, 1);
	add_vertex(-11, -19, 0);
	add_poly_fill_source(0, -18);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-3, 7, 0);
	add_vertex(3, 7, 0);
	add_vertex(11, 19, 0);
	add_vertex(3, 29, 1);
	add_vertex(-3, 29, 1);
	add_vertex(-11, 19, 0);
	add_poly_fill_source(0, 18);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(5, -6, 0);
	add_vertex(10, -1, 0);
	add_vertex(33, -1, 0);
	add_vertex(46, -11, 1);
	add_vertex(29, -21, 1);
	add_vertex(13, -19, 0);
	add_poly_fill_source(22, -9);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(5, 6, 0);
	add_vertex(10, 1, 0);
	add_vertex(33, 1, 0);
	add_vertex(46, 11, 1);
	add_vertex(29, 21, 1);
	add_vertex(13, 19, 0);
	add_poly_fill_source(22, 9);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_CORE_MUTABLE);

	add_vertex(-15, -4, 0);
	add_vertex(-11, 0, 0);
	add_vertex(-15, 4, 0);
	add_vertex(-18, 4, 0);
	add_vertex(-22, 0, 0);
	add_vertex(-18, -4, 0);
	add_poly_fill_source(-16, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-13, -19, 0);
	add_vertex(-5, -7, 0);
	add_vertex(-10, -2, 0);
	add_vertex(-15, -6, 0);
	add_vertex(-18, -6, 0);
	add_vertex(-24, -1, 0);
	add_vertex(-37, -1, 0);
	add_vertex(-48, -12, 0);
	add_vertex(-33, -23, 0);
	add_poly_fill_source(-22, -8);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-13, 19, 0);
	add_vertex(-5, 7, 0);
	add_vertex(-10, 2, 0);
	add_vertex(-15, 6, 0);
	add_vertex(-18, 6, 0);
	add_vertex(-24, 1, 0);
	add_vertex(-37, 1, 0);
	add_vertex(-48, 12, 0);
	add_vertex(-33, 23, 0);
	add_poly_fill_source(-22, 8);
	fix_display_triangles_fan();

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
	add_link_at_xy(0, // link_index
	               42, 0, // centre
	               49, -9, // left
	               49, 9, // right
	               55, 0, // far
	               58, 0, // link
	               55, 0); // object

	add_link_at_xy(1, // link_index
	               16, 22, // centre
	               28, 23, // left
	               8, 32, // right
	               19, 31, // far
	               21, 34, // link
	               19, 31); // object

	add_link_at_xy(2, // link_index
	               -14, 22, // centre
	               -8, 32, // left
	               -27, 25, // right
	               -19, 31, // far
	               -21, 34, // link
	               -19, 31); // object

	add_link_at_xy(3, // link_index
	               -38, 0, // centre
	               -47, 8, // left
	               -47, -8, // right
	               -54, 0, // far
	               -56, 0, // link
	               -52, 0); // object

	add_link_at_xy(4, // link_index
	               -14, -22, // centre
	               -27, -25, // left
	               -8, -32, // right
	               -19, -31, // far
	               -21, -34, // link
	               -19, -31); // object

	add_link_at_xy(5, // link_index
	               16, -22, // centre
	               8, -32, // left
	               28, -23, // right
	               19, -31, // far
	               21, -34, // link
	               19, -31); // object



	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

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
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_CORE_MUTABLE);

	add_vertex(9, 0, 0);
	add_vertex(4, 5, 0);
	add_vertex(-4, 5, 0);
	add_vertex(-9, 0, 0);
	add_vertex(-4, -5, 0);
	add_vertex(4, -5, 0);
	add_poly_fill_source(0, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-3, -7, 0);
	add_vertex(3, -7, 0);
	add_vertex(15, -20, 0);
	add_vertex(4, -35, 1);
	add_vertex(-4, -35, 1);
	add_vertex(-13, -20, 0);
	add_poly_fill_source(0, -20);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-3, 7, 0);
	add_vertex(3, 7, 0);
	add_vertex(15, 20, 0);
	add_vertex(4, 35, 1);
	add_vertex(-4, 35, 1);
	add_vertex(-13, 20, 0);
	add_poly_fill_source(0, 20);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_CORE_MUTABLE);

	add_vertex(-15, -4, 0);
	add_vertex(-11, 0, 0);
	add_vertex(-15, 4, 0);
	add_vertex(-18, 4, 0);
	add_vertex(-22, 0, 0);
	add_vertex(-18, -4, 0);
	add_poly_fill_source(-16, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-15, -20, 0);
	add_vertex(-5, -7, 0);
	add_vertex(-10, -2, 0);
	add_vertex(-15, -6, 0);
	add_vertex(-18, -6, 0);
	add_vertex(-24, -1, 0);
	add_vertex(-37, -1, 0);
	add_vertex(-51, -15, 1);
	add_vertex(-38, -26, 1);
	add_poly_fill_source(-23, -9);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-15, 20, 0);
	add_vertex(-5, 7, 0);
	add_vertex(-10, 2, 0);
	add_vertex(-15, 6, 0);
	add_vertex(-18, 6, 0);
	add_vertex(-24, 1, 0);
	add_vertex(-37, 1, 0);
	add_vertex(-51, 15, 1);
	add_vertex(-38, 26, 1);
	add_poly_fill_source(-23, 9);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(17, -20, 0);
	add_vertex(5, -7, 0);
	add_vertex(10, -2, 0);
	add_vertex(15, -6, 0);
	add_vertex(18, -6, 0);
	add_vertex(24, -1, 0);
	add_vertex(40, -1, 0);
	add_vertex(51, -15, 1);
	add_vertex(33, -22, 0);
	add_poly_fill_source(23, -8);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(17, 20, 0);
	add_vertex(5, 7, 0);
	add_vertex(10, 2, 0);
	add_vertex(15, 6, 0);
	add_vertex(18, 6, 0);
	add_vertex(24, 1, 0);
	add_vertex(40, 1, 0);
	add_vertex(51, 15, 1);
	add_vertex(33, 22, 0);
	add_poly_fill_source(23, 8);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_CORE_MUTABLE);

	add_vertex(15, -4, 0);
	add_vertex(11, 0, 0);
	add_vertex(15, 4, 0);
	add_vertex(18, 4, 0);
	add_vertex(22, 0, 0);
	add_vertex(18, -4, 0);
	add_poly_fill_source(16, 0);
	fix_display_triangles_fan();

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
	add_link_at_xy(0, // link_index
	               33, 0, // centre
	               41, -7, // left
	               41, 7, // right
	               45, 0, // far
	               47, 0, // link
	               45, 0); // object

	add_link_at_xy(1, // link_index
	               0, 19, // centre
	               7, 26, // left
	               -10, 24, // right
	               -3, 30, // far
	               -4, 33, // link
	               -3, 29); // object

	add_link_at_xy(2, // link_index
	               -22, 10, // centre
	               -23, 18, // left
	               -35, 6, // right
	               -33, 15, // far
	               -37, 17, // link
	               -33, 14); // object

	add_link_at_xy(3, // link_index
	               -22, -10, // centre
	               -35, -6, // left
	               -23, -18, // right
	               -33, -15, // far
	               -37, -17, // link
	               -33, -14); // object

	add_link_at_xy(4, // link_index
	               0, -19, // centre
	               -10, -24, // left
	               7, -26, // right
	               -3, -30, // far
	               -4, -33, // link
	               -3, -29); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

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
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(8, -1, 0);
	add_vertex(1, -7, 0);
	add_vertex(1, -18, 0);
	add_vertex(10, -28, 1);
	add_vertex(42, -10, 1);
	add_vertex(32, -1, 0);
	add_poly_fill_source(15, -10);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(8, 1, 0);
	add_vertex(1, 7, 0);
	add_vertex(1, 18, 0);
	add_vertex(10, 28, 1);
	add_vertex(42, 10, 1);
	add_vertex(32, 1, 0);
	add_poly_fill_source(15, 10);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-1, -7, 0);
	add_vertex(-1, -18, 0);
	add_vertex(-19, -27, 1);
	add_vertex(-23, -24, 0);
	add_vertex(-21, -10, 0);
	add_vertex(-7, -4, 0);
	add_poly_fill_source(-12, -15);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-1, 7, 0);
	add_vertex(-1, 18, 0);
	add_vertex(-19, 27, 1);
	add_vertex(-23, 24, 0);
	add_vertex(-21, 10, 0);
	add_vertex(-7, 4, 0);
	add_poly_fill_source(-12, 15);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-8, 3, 0);
	add_vertex(-21, 9, 0);
	add_vertex(-42, 3, 1);
	add_vertex(-42, -3, 1);
	add_vertex(-21, -9, 0);
	add_vertex(-8, -3, 0);
	add_poly_fill_source(-23, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_CORE_MUTABLE);

	add_vertex(0, 6, 0);
	add_vertex(-6, 3, 0);
	add_vertex(-6, -3, 0);
	add_vertex(0, -6, 0);
	add_vertex(6, 0, 0);
	add_poly_fill_source(-1, 0);
	fix_display_triangles_fan();

 add_mirror_axis_at_link(0, -1);
 add_mirror_axis(ANGLE_2, 0);

	finish_shape();

// ******* NSHAPE_CORE_PENT_A *******

 start_dshape(NSHAPE_CORE_PENT_A, KEYWORD_CORE_PENT_A);


	poly_index = POLY_0;
	add_link_at_xy(0, // link_index
	               26, 0, // centre
	               31, -7, // left
	               31, 7, // right
	               38, 0, // far
	               41, 0, // link
	               37, 0); // object

	add_link_at_xy(1, // link_index
	               3, 21, // centre
	               18, 17, // left
	               -4, 27, // right
	               9, 28, // far
	               10, 30, // link
	               8, 27); // object

	add_link_at_xy(2, // link_index
	               -21, 17, // centre
	               -16, 28, // left
	               -35, 12, // right
	               -26, 24, // far
	               -28, 25, // link
	               -26, 23); // object

	add_link_at_xy(3, // link_index
	               -21, -17, // centre
	               -35, -12, // left
	               -16, -28, // right
	               -26, -24, // far
	               -28, -25, // link
	               -26, -23); // object

	add_link_at_xy(4, // link_index
	               3, -21, // centre
	               -4, -27, // left
	               18, -17, // right
	               9, -28, // far
	               10, -30, // link
	               8, -27); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

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
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(8, -1, 0);
	add_vertex(1, -8, 0);
	add_vertex(4, -19, 0);
	add_vertex(22, -14, 0);
	add_vertex(30, -9, 1);
	add_vertex(24, -1, 0);
	add_poly_fill_source(14, -8);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(8, 1, 0);
	add_vertex(1, 8, 0);
	add_vertex(4, 19, 0);
	add_vertex(22, 14, 0);
	add_vertex(30, 9, 1);
	add_vertex(24, 1, 0);
	add_poly_fill_source(14, 8);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(0, -9, 0);
	add_vertex(2, -19, 0);
	add_vertex(-8, -28, 1);
	add_vertex(-14, -28, 1);
	add_vertex(-19, -16, 0);
	add_vertex(-8, -4, 0);
	add_poly_fill_source(-7, -17);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(0, 9, 0);
	add_vertex(2, 19, 0);
	add_vertex(-8, 28, 1);
	add_vertex(-14, 28, 1);
	add_vertex(-19, 16, 0);
	add_vertex(-8, 4, 0);
	add_poly_fill_source(-7, 17);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-9, 3, 0);
	add_vertex(-21, 15, 0);
	add_vertex(-50, 5, 1);
	add_vertex(-50, -5, 1);
	add_vertex(-21, -15, 0);
	add_vertex(-9, -3, 0);
	add_poly_fill_source(-26, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_CORE_MUTABLE);

	add_vertex(0, 7, 0);
	add_vertex(-7, 3, 0);
	add_vertex(-7, -3, 0);
	add_vertex(0, -7, 0);
	add_vertex(7, 0, 0);
	add_poly_fill_source(-1, 0);
	fix_display_triangles_fan();

 add_mirror_axis_at_link(0, -1);
 add_mirror_axis(ANGLE_2, 0);

	finish_shape();


// ******* NSHAPE_CORE_PENT_B *******

 start_dshape(NSHAPE_CORE_PENT_B, KEYWORD_CORE_PENT_B);

	poly_index = POLY_0;
	add_link_at_xy(0, // link_index
	               27, 0, // centre
	               33, -8, // left
	               33, 8, // right
	               38, 0, // far
	               40, 0, // link
	               36, 0); // object

	add_link_at_xy(1, // link_index
	               3, 20, // centre
	               16, 20, // left
	               -2, 31, // right
	               10, 32, // far
	               11, 33, // link
	               9, 29); // object

	add_link_at_xy(2, // link_index
	               -18, 17, // centre
	               -15, 25, // left
	               -34, 14, // right
	               -29, 27, // far
	               -30, 28, // link
	               -27, 24); // object

	add_link_at_xy(3, // link_index
	               -18, -17, // centre
	               -34, -14, // left
	               -15, -25, // right
	               -29, -27, // far
	               -30, -28, // link
	               -27, -24); // object

	add_link_at_xy(4, // link_index
	               3, -20, // centre
	               -2, -31, // left
	               16, -20, // right
	               10, -32, // far
	               11, -33, // link
	               9, -29); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

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
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(7, -1, 0);
	add_vertex(-1, -7, 0);
	add_vertex(5, -19, 0);
	add_vertex(31, -18, 0);
	add_vertex(33, -10, 1);
	add_vertex(26, -1, 0);
	add_poly_fill_source(16, -9);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(7, 1, 0);
	add_vertex(-1, 7, 0);
	add_vertex(5, 19, 0);
	add_vertex(31, 18, 0);
	add_vertex(33, 10, 1);
	add_vertex(26, 1, 0);
	add_poly_fill_source(16, 9);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-2, -8, 0);
	add_vertex(2, -19, 0);
	add_vertex(-4, -32, 1);
	add_vertex(-13, -27, 1);
	add_vertex(-16, -17, 0);
	add_vertex(-8, -4, 0);
	add_poly_fill_source(-6, -17);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-2, 8, 0);
	add_vertex(2, 19, 0);
	add_vertex(-4, 32, 1);
	add_vertex(-13, 27, 1);
	add_vertex(-16, 17, 0);
	add_vertex(-8, 4, 0);
	add_poly_fill_source(-6, 17);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-9, 3, 0);
	add_vertex(-18, 16, 0);
	add_vertex(-40, 11, 1);
	add_vertex(-48, 0, 1);
	add_vertex(-40, -11, 1);
	add_vertex(-18, -16, 0);
	add_vertex(-9, -3, 0);
	add_poly_fill_source(-26, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_CORE_MUTABLE);

	add_vertex(-2, 6, 0);
	add_vertex(-7, 3, 0);
	add_vertex(-7, -3, 0);
	add_vertex(-2, -6, 0);
	add_vertex(6, 0, 0);
	add_poly_fill_source(-2, 0);
	fix_display_triangles_fan();

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
 start_dshape_poly(poly_index++, 0, COL_LEVEL_BASE);

 add_vertex_vector(0 * ANGLE_4, STATIC_QUAD_BASE_SIZE + 16, 1);
 add_outline_vertex_at_last_poly_vertex(6);
 add_vertex_vector(1 * ANGLE_4, STATIC_QUAD_BASE_SIZE + 16, 1);
 add_outline_vertex_at_last_poly_vertex(6);
 add_vertex_vector(2 * ANGLE_4, STATIC_QUAD_BASE_SIZE + 16, 1);
 add_outline_vertex_at_last_poly_vertex(6);
 add_vertex_vector(3 * ANGLE_4, STATIC_QUAD_BASE_SIZE + 16, 1);
 add_outline_vertex_at_last_poly_vertex(6);

 add_poly_fill_source(0, 0);

 fix_display_triangles_walk();
/*
 nshape[NSHAPE_CORE_STATIC_QUAD].mirrored_object [0] [0] = 0;
 nshape[NSHAPE_CORE_STATIC_QUAD].mirrored_object [0] [1] = 1;
 nshape[NSHAPE_CORE_STATIC_QUAD].mirrored_object [0] [2] = 2;
 nshape[NSHAPE_CORE_STATIC_QUAD].mirrored_object [0] [3] = 3;*/

 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

// centre bit:
 start_dshape_poly(poly_index++, 1, COL_LEVEL_CORE);
 add_vertex(6, 0, 0);
 add_vertex(0, 6, 0);
 add_vertex(-6, 0, 0);
 add_vertex(0, -6, 0);
 add_poly_fill_source(0, 0);
 fix_display_triangles_walk();

#define STATIC_QUAD_OUTER_SIZE 20
#define STATIC_QUAD_EDGE_SIZE 8

// down-right quadrant
 start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN);
 add_vertex(1, 8, 0);
 add_vertex(8, 1, 0);
 add_vertex(STATIC_QUAD_OUTER_SIZE, 1, 0);
// add_vertex(STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, STATIC_QUAD_EDGE_SIZE + 1, 1); // collision
 add_vertex(STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, STATIC_QUAD_EDGE_SIZE + 8, 1); // collision
 add_vertex(STATIC_QUAD_EDGE_SIZE + 8, STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, 1); // collision
// add_vertex(STATIC_QUAD_EDGE_SIZE + 1, STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, 1); // collision
 add_vertex(1, STATIC_QUAD_OUTER_SIZE, 0);
 add_poly_fill_source(8, 8);
 fix_display_triangles_walk();

// down-left quadrant
 start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN);
 add_vertex(-1, 8, 0);
 add_vertex(-8, 1, 0);
 add_vertex(-STATIC_QUAD_OUTER_SIZE, 1, 0);
// add_vertex(-STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, STATIC_QUAD_EDGE_SIZE + 1, 1); // collision
 add_vertex(-STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, STATIC_QUAD_EDGE_SIZE + 8, 1); // collision
 add_vertex(-STATIC_QUAD_EDGE_SIZE - 8, STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, 1); // collision
// add_vertex(-STATIC_QUAD_EDGE_SIZE - 1, STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, 1); // collision
 add_vertex(-1, STATIC_QUAD_OUTER_SIZE, 0);
 add_poly_fill_source(-8, 8);
 fix_display_triangles_walk();

// up-left quadrant
 start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN);
 add_vertex(-1, -8, 0);
 add_vertex(-8, -1, 0);
 add_vertex(-STATIC_QUAD_OUTER_SIZE, -1, 0);
// add_vertex(-STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, -STATIC_QUAD_EDGE_SIZE - 1, 1); // collision
 add_vertex(-STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, -STATIC_QUAD_EDGE_SIZE - 8, 1); // collision
 add_vertex(-STATIC_QUAD_EDGE_SIZE - 8, -STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, 1); // collision
// add_vertex(-STATIC_QUAD_EDGE_SIZE - 1, -STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, 1); // collision
 add_vertex(-1, -STATIC_QUAD_OUTER_SIZE, 0);
 add_poly_fill_source(-8, -8);

 fix_display_triangles_walk();

// up-right quadrant
 start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN);
 add_vertex(1, -8, 0);
 add_vertex(8, -1, 0);
 add_vertex(STATIC_QUAD_OUTER_SIZE, -1, 0);
// add_vertex(STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, -STATIC_QUAD_EDGE_SIZE - 1, 1); // collision
 add_vertex(STATIC_QUAD_OUTER_SIZE + STATIC_QUAD_EDGE_SIZE, -STATIC_QUAD_EDGE_SIZE - 8, 1); // collision
 add_vertex(STATIC_QUAD_EDGE_SIZE + 8, -STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, 1); // collision
// add_vertex(STATIC_QUAD_EDGE_SIZE + 1, -STATIC_QUAD_OUTER_SIZE - STATIC_QUAD_EDGE_SIZE, 1); // collision
 add_vertex(1, -STATIC_QUAD_OUTER_SIZE, 0);
 add_poly_fill_source(8, -8);
 fix_display_triangles_walk();

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
 start_dshape_poly(poly_index++, 0, COL_LEVEL_BASE);

 for (i = 0; i < 5; i ++)
	{
  add_vertex_vector(i * ANGLE_5, STATIC_PENT_BASE_SIZE + 6, 1);
  add_outline_vertex_at_last_poly_vertex(6);
	}
 add_poly_fill_source(0, 0);
 fix_display_triangles_walk();

 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

// centre bit:
 start_dshape_poly(poly_index++, 1, COL_LEVEL_CORE);

 for (i = 0; i < 5; i ++)
	{
  add_vertex_vector(i * ANGLE_5, 7, 0);
	}
 add_poly_fill_source(0, 0);
 fix_display_triangles_walk();

 for (i = 0; i < 5; i ++)
	{
  start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN);
  int base_angle;
  base_angle = i * ANGLE_5;
  add_vertex_vector(base_angle + 200, 9, 0);
  add_vertex_vector(base_angle + 50, STATIC_PENT_BASE_SIZE - 3, 0);
  add_vertex_vector(base_angle + ANGLE_16, STATIC_PENT_BASE_SIZE + 9, 1);
  add_vertex_vector(base_angle + ANGLE_5 - ANGLE_16, STATIC_PENT_BASE_SIZE + 9, 1);
  add_vertex_vector(base_angle + ANGLE_5 - 50, STATIC_PENT_BASE_SIZE - 3, 1);
  add_vertex_vector(base_angle + ANGLE_5 - 200, 9, 0);
  add_poly_fill_source(xpart(base_angle + ANGLE_16 + 20, 12), ypart(base_angle + ANGLE_16 + 20, 12)); // xpart/ypart using floating point, but that's okay for poly_fill_source
  fix_display_triangles_walk();

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
 start_dshape_poly(poly_index++, 0, COL_LEVEL_BASE);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, STATIC_HEX_BASE_SIZE + 6, 1);
  add_outline_vertex_at_last_poly_vertex(6);
	}
 add_poly_fill_source(0, 0);
 fix_display_triangles_walk();

 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

// centre bit:
 start_dshape_poly(poly_index++, 1, COL_LEVEL_CORE);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, 7, 0);
	}
 add_poly_fill_source(0, 0);
 fix_display_triangles_walk();

 for (i = 0; i < 6; i ++)
	{
  start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN);
  int base_angle;
  base_angle = i * ANGLE_6;
  add_vertex_vector(base_angle + 200, 9, 0);
  add_vertex_vector(base_angle + 50, STATIC_HEX_BASE_SIZE - 3, 0);
  add_vertex_vector(base_angle + ANGLE_16, STATIC_HEX_BASE_SIZE + 9, 1);
  add_vertex_vector(base_angle + ANGLE_6 - ANGLE_16, STATIC_HEX_BASE_SIZE + 9, 1);
  add_vertex_vector(base_angle + ANGLE_6 - 50, STATIC_HEX_BASE_SIZE - 3, 1);
  add_vertex_vector(base_angle + ANGLE_6 - 200, 9, 0);
  add_poly_fill_source(xpart(base_angle + ANGLE_16 + 20, 12), ypart(base_angle + ANGLE_16 + 20, 12)); // xpart/ypart using floating point, but that's okay for poly_fill_source
  fix_display_triangles_walk();

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
 start_dshape_poly(poly_index++, 0, COL_LEVEL_BASE);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, STATIC_HEX_B_BASE_SIZE + 6, 1);
  add_outline_vertex_at_last_poly_vertex(6);
	}
 add_poly_fill_source(0, 0);
 fix_display_triangles_walk();

 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

// centre bit:
 start_dshape_poly(poly_index++, 1, COL_LEVEL_CORE);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, 9, 0);
	}
 add_poly_fill_source(0, 0);
 fix_display_triangles_walk();

 for (i = 0; i < 6; i ++)
	{
  start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN);
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
  fix_display_triangles_walk();

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
 start_dshape_poly(poly_index++, 0, COL_LEVEL_BASE);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, STATIC_HEX_C_BASE_SIZE + 6, 1);
  add_outline_vertex_at_last_poly_vertex(6);
	}
 add_poly_fill_source(0, 0);
 fix_display_triangles_walk();

 dshape[dshape_init.current_dshape].outline_base_vertex = 0;

// centre bit:
 start_dshape_poly(poly_index++, 1, COL_LEVEL_CORE);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, 12, 0);
	}
 add_poly_fill_source(0, 0);
 fix_display_triangles_walk();
/*
 start_dshape_poly(poly_index++, 1, COL_LEVEL_BASE);

 for (i = 0; i < 6; i ++)
	{
  add_vertex_vector(i * ANGLE_6, 6, 0);
	}
 add_poly_fill_source(0, 0);
 fix_display_triangles_walk();*/

 for (i = 0; i < 6; i ++)
	{
  start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN);
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
  fix_display_triangles_walk();

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

 add_link_at_xy(0, // link_index
																34, 0, // centre
																41, -8, // left
																41, 8, // right
																47, 0, // far
																50, 0, // link
																45, 0); // object


 add_link_at_xy(1, // link_index
																13, 15, // centre
																21, 20, // left
																6, 26, // right
																16, 27, // far
																14, 30, // link
																15, 24); // object

 add_link_at_xy(2, // link_index
																-13, 15, // centre
																-6, 26, // left
																-21, 20, // right
																-16, 27, // far
																-14, 30, // link
																-15, 24); // object

 add_link_at_xy(3, // link_index
																-34, 0, // centre
																-41, 8, // left
																-41, -8, // right
																-47, 0, // far
																-50, 0, // link
																-45, 0); // object

 add_link_at_xy(4, // link_index
																-13, -15, // centre
																-21, -20, // left
																-6, -26, // right
																-16, -27, // far
																-14, -30, // link
																-15, -24); // object

 add_link_at_xy(5, // link_index
																13, -15, // centre
																6, -26, // left
																21, -20, // right
																16, -27, // far
																14, -30, // link
																15, -24); // object

 poly_index = POLY_0;
 start_dshape_poly(poly_index++, 0, COL_LEVEL_BASE);

 add_vertex(43, 0, 1);
 add_outline_vertex_at_last_poly_vertex(4);
 add_vertex(14, 22, 0);
 add_outline_vertex_at_last_poly_vertex(4);
 add_vertex(-14, 22, 0);
 add_outline_vertex_at_last_poly_vertex(4);
 add_vertex(-43, -0, 1);
 add_outline_vertex_at_last_poly_vertex(4);
 add_vertex(-14, -22, 0);
 add_outline_vertex_at_last_poly_vertex(4);
 add_vertex(14, -22, 0);
 add_outline_vertex_at_last_poly_vertex(4);
 add_poly_fill_source(0, 0);
 fix_display_triangles_walk();

// pentagon
 start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN);
 add_vertex(-3, -27, 1);
 add_vertex(3, -27, 1);
 add_vertex(20, 0, 0);
 add_vertex(3, 27, 1);
 add_vertex(-3, 27, 1);
 add_vertex(-20, 0, 0);
 add_poly_fill_source(0, -10);
 fix_display_triangles_walk();

// left-up
 start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN);
 add_vertex(-13, -14, 0);
 add_vertex(-24, -19, 0);
 add_vertex(-40, -10, 1);
 add_vertex(-32, -1, 0);
 add_vertex(-22, -1, 0);
 add_poly_fill_source(-23, -7);
 fix_display_triangles_walk();

// left-down
 start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN);
 add_vertex(-13, 14, 0);
 add_vertex(-24, 19, 0);
 add_vertex(-40, 10, 1);
 add_vertex(-32, 1, 0);
 add_vertex(-22, 1, 0);
 add_poly_fill_source(-23, 7);
 fix_display_triangles_walk();

// right-up
 start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN);
 add_vertex(13, -14, 0);
 add_vertex(24, -19, 0);
 add_vertex(40, -10, 1);
 add_vertex(32, -1, 0);
 add_vertex(22, -1, 0);
 add_poly_fill_source(23, -7);
 fix_display_triangles_walk();

// right-down
 start_dshape_poly(poly_index++, 1, COL_LEVEL_MAIN);
 add_vertex(13, 14, 0);
 add_vertex(24, 19, 0);
 add_vertex(40, 10, 1);
 add_vertex(32, 1, 0);
 add_vertex(22, 1, 0);
 add_poly_fill_source(23, 7);
 fix_display_triangles_walk();




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
	add_link_at_xy(0, // link_index
	               34, 0, // centre
	               40, -7, // left
	               40, 7, // right
	               47, 0, // far
	               50, 0, // link
	               45, 0); // object

	add_link_at_xy(1, // link_index
	               15, -8, // centre
	               11, -20, // left
	               23, -17, // right
	               19, -23, // far
	               20, -25, // link
	               19, -22); // object

	add_link_at_xy(2, // link_index
	               15, 8, // centre
	               23, 17, // left
	               11, 20, // right
	               19, 23, // far
	               20, 25, // link
	               19, 22); // object

	add_link_at_xy(3, // link_index
	               -34, 0, // centre
	               -39, 7, // left
	               -39, -7, // right
	               -47, 0, // far
	               -50, 0, // link
	               -45, 0); // object

	add_link_at_xy(4, // link_index
	               -12, -18, // centre
	               -20, -22, // left
	               -6, -25, // right
	               -16, -29, // far
	               -17, -31, // link
	               -15, -27); // object

	add_link_at_xy(5, // link_index
	               -12, 18, // centre
	               -6, 25, // left
	               -20, 22, // right
	               -16, 29, // far
	               -17, 31, // link
	               -15, 27); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

	add_vertex(44, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(19, 17, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-14, 26, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-43, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-14, -26, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(19, -17, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(1, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-32, 0, 0);
	add_vertex(-12, -16, 0);
	add_vertex(31, 0, 0);
	add_vertex(-12, 16, 0);
	add_poly_fill_source(-6, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-14, -17, 0);
	add_vertex(-23, -22, 0);
	add_vertex(-40, -10, 1);
	add_vertex(-33, -1, 0);
	add_poly_fill_source(-27, -12);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-14, 17, 0);
	add_vertex(-23, 22, 0);
	add_vertex(-40, 10, 1);
	add_vertex(-33, 1, 0);
	add_poly_fill_source(-27, 12);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(26, -18, 1);
	add_vertex(40, -10, 1);
	add_vertex(32, -1, 0);
	add_vertex(17, -7, 0);
	add_poly_fill_source(28, -9);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(26, 18, 1);
	add_vertex(40, 10, 1);
	add_vertex(32, 1, 0);
	add_vertex(17, 7, 0);
	add_poly_fill_source(28, 9);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-4, -26, 0);
	add_vertex(-11, -17, 0);
	add_vertex(13, -9, 0);
	add_vertex(8, -24, 0);
	add_poly_fill_source(1, -19);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-4, 26, 0);
	add_vertex(-10, 18, 0);
	add_vertex(13, 9, 0);
	add_vertex(8, 24, 0);
	add_poly_fill_source(1, 19);
	fix_display_triangles_fan();



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
	add_link_at_xy(0, // link_index
	               0, -14, // centre
	               -8, -21, // left
	               8, -21, // right
	               0, -27, // far
	               0, -29, // link
	               0, -25); // object

	add_link_at_xy(1, // link_index
	               0, 14, // centre
	               8, 21, // left
	               -8, 21, // right
	               0, 27, // far
	               0, 29, // link
	               0, 25); // object

	add_link_at_xy(2, // link_index
	               19, -14, // centre
	               14, -21, // left
	               29, -13, // right
	               25, -25, // far
	               26, -26, // link
	               24, -23); // object

	add_link_at_xy(3, // link_index
	               19, 14, // centre
	               29, 13, // left
	               14, 21, // right
	               25, 25, // far
	               26, 26, // link
	               24, 23); // object

	add_link_at_xy(4, // link_index
	               -19, -14, // centre
	               -29, -13, // right
	               -14, -21, // left
	               -25, -25, // far
	               -26, -26, // link
	               -24, -23); // object

	add_link_at_xy(5, // link_index
	               -19, 14, // centre
	               -14, 21, // right
	               -29, 13, // left
	               -25, 25, // far
	               -26, 26, // link
	               -24, 23); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

	add_vertex(38, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(23, 20, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(12, 18, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, 23, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-11, 18, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-23, 20, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-38, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-23, -20, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-11, -18, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, -23, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(11, -18, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(23, -20, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(0, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(10, 0, 0);
	add_vertex(1, -13, 0);
	add_vertex(12, -22, 1);
	add_vertex(27, 0, 0);
	add_vertex(12, 22, 1);
	add_vertex(1, 13, 0);
	add_poly_fill_source(10, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-10, 0, 0);
	add_vertex(-1, -13, 0);
	add_vertex(-12, -22, 1);
	add_vertex(-27, 0, 0);
	add_vertex(-12, 22, 1);
	add_vertex(-1, 13, 0);
	add_poly_fill_source(-10, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(20, -13, 0);
	add_vertex(33, -12, 1);
	add_vertex(44, -1, 1);
	add_vertex(28, -1, 0);
	add_poly_fill_source(31, -6);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(20, 13, 0);
	add_vertex(33, 12, 1);
	add_vertex(44, 1, 1);
	add_vertex(28, 1, 0);
	add_poly_fill_source(31, 6);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-20, -13, 0);
	add_vertex(-33, -12, 1);
	add_vertex(-44, -1, 1);
	add_vertex(-28, -1, 0);
	add_poly_fill_source(-31, -6);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-20, 13, 0);
	add_vertex(-33, 12, 1);
	add_vertex(-44, 1, 1);
	add_vertex(-28, 1, 0);
	add_poly_fill_source(-31, 6);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(0, -12, 0);
	add_vertex(-8, 0, 0);
	add_vertex(0, 11, 0);
	add_vertex(8, 0, 0);
	add_poly_fill_source(0, 0);
	fix_display_triangles_fan();

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
	add_link_at_xy(0, // link_index
	               36, 0, // centre
	               42, -7, // left
	               42, 7, // right
	               47, 0, // far
	               50, 0, // link
	               45, 0); // object

	add_link_at_xy(1, // link_index
	               0, 19, // centre
	               8, 25, // left
	               -8, 25, // right
	               0, 31, // far
	               0, 33, // link
	               0, 29); // object

	add_link_at_xy(2, // link_index
	               -36, 0, // centre
	               -42, 7, // left
	               -42, -7, // right
	               -47, 0, // far
	               -50, 0, // link
	               -45, 0); // object

	add_link_at_xy(3, // link_index
	               0, -19, // centre
	               -8, -25, // left
	               8, -25, // right
	               0, -31, // far
	               0, -33, // link
	               0, -29); // object


	start_dshape_poly(poly_index++, 1, PROC_COL_UNDERLAY);

	add_vertex(46, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, -28, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-46, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, 28, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(0, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(1, 17, 0);
	add_vertex(20, 1, 0);
	add_vertex(34, 1, 0);
	add_vertex(41, 9, 1);
	add_vertex(10, 24, 1);
	add_poly_fill_source(21, 10);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(1, -17, 0);
	add_vertex(20, -1, 0);
	add_vertex(34, -1, 0);
	add_vertex(41, -9, 1);
	add_vertex(10, -24, 1);
	add_poly_fill_source(21, -10);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-1, -17, 0);
	add_vertex(-20, -1, 0);
	add_vertex(-34, -1, 0);
	add_vertex(-41, -9, 1);
	add_vertex(-10, -24, 1);
	add_poly_fill_source(-21, -10);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-1, 17, 0);
	add_vertex(-20, 1, 0);
	add_vertex(-34, 1, 0);
	add_vertex(-41, 9, 1);
	add_vertex(-10, 24, 1);
	add_poly_fill_source(-21, 10);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(0, -15, 0);
	add_vertex(-18, 0, 0);
	add_vertex(0, 15, 0);
	add_vertex(18, 0, 0);
	add_poly_fill_source(0, 0);
	fix_display_triangles_fan();

	// remember to deal with mirror vertices here!

//	finish_shape();

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
	add_link_at_xy(0, // link_index
	               22, 0, // centre
	               27, -11, // left
	               27, 11, // right
	               34, 0, // far
	               40, 0, // link
	               35, 0); // object

	add_link_at_xy(1, // link_index
	               0, 22, // centre
	               11, 27, // left
	               -11, 27, // right
	               0, 34, // far
	               0, 40, // link
	               0, 35); // object

	add_link_at_xy(2, // link_index
	               -22, 0, // centre
	               -27, 11, // left
	               -27, -11, // right
	               -34, 0, // far
	               -40, 0, // link
	               -35, 0); // object

	add_link_at_xy(3, // link_index
	               0, -22, // centre
	               -11, -27, // left
	               11, -27, // right
	               0, -34, // far
	               0, -40, // link
	               0, -35); // object


	start_dshape_poly(poly_index++, 1, PROC_COL_UNDERLAY);

	add_vertex(31, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, -31, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-31, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(0, 31, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(0, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(1, 14, 0);
	add_vertex(14, 1, 0);
	add_vertex(20, 1, 0);
	add_vertex(28, 16, 1);
	add_vertex(16, 28, 1);
	add_vertex(1, 20, 0);
	add_poly_fill_source(13, 13);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(1, -14, 0);
	add_vertex(14, -1, 0);
	add_vertex(20, -1, 0);
	add_vertex(28, -16, 1);
	add_vertex(16, -28, 1);
	add_vertex(1, -20, 0);
	add_poly_fill_source(13, -13);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-1, -14, 0);
	add_vertex(-14, -1, 0);
	add_vertex(-20, -1, 0);
	add_vertex(-28, -16, 1);
	add_vertex(-16, -28, 1);
	add_vertex(-1, -20, 0);
	add_poly_fill_source(-13, -13);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-1, 14, 0);
	add_vertex(-14, 1, 0);
	add_vertex(-20, 1, 0);
	add_vertex(-28, 16, 1);
	add_vertex(-16, 28, 1);
	add_vertex(-1, 20, 0);
	add_poly_fill_source(-13, 13);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(0, -12, 0);
	add_vertex(-12, 0, 0);
	add_vertex(0, 12, 0);
	add_vertex(12, 0, 0);
	add_poly_fill_source(0, 0);
	fix_display_triangles_fan();

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
	add_link_at_xy(0, // link_index
	               34, 0, // centre
	               41, -8, // left
	               41, 8, // right
	               47, 0, // far
	               50, 0, // link
	               45, 0); // object

	add_link_at_xy(1, // link_index
	               -27, -8, // centre
	               -39, -5, // left
	               -30, -16, // right
	               -40, -12, // far
	               -43, -13, // link
	               -39, -11); // object

	add_link_at_xy(2, // link_index
	               -27, 8, // centre
	               -30, 16, // left
	               -39, 5, // right
	               -40, 12, // far
	               -43, 13, // link
	               -39, 11); // object

	add_link_at_xy(3, // link_index
	               -8, -17, // centre
	               -17, -22, // left
	               3, -22, // right
	               -8, -28, // far
	               -8, -32, // link
	               -8, -27); // object

	add_link_at_xy(4, // link_index
	               -8, 17, // centre
	               3, 22, // left
	               -17, 22, // right
	               -8, 28, // far
	               -8, 32, // link
	               -8, 27); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

	add_vertex(46, 0, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-10, 25, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-35, 9, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-35, -9, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-10, -25, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(-8, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-13, -10, 0);
	add_vertex(-2, 0, 0);
	add_vertex(-13, 10, 0);
	add_vertex(-42, 3, 1);
	add_vertex(-42, -3, 1);
	add_poly_fill_source(-22, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-9, -16, 0);
	add_vertex(-20, -22, 0);
	add_vertex(-29, -17, 1);
	add_vertex(-26, -9, 0);
	add_vertex(-13, -12, 0);
	add_poly_fill_source(-19, -15);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-9, 16, 0);
	add_vertex(-20, 22, 0);
	add_vertex(-29, 17, 1);
	add_vertex(-26, 9, 0);
	add_vertex(-13, 12, 0);
	add_poly_fill_source(-19, 15);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-11, -11, 0);
	add_vertex(-7, -16, 0);
	add_vertex(6, -21, 0);
	add_vertex(40, -10, 1);
	add_vertex(32, -1, 0);
	add_vertex(0, -1, 0);
	add_poly_fill_source(10, -10);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-11, 11, 0);
	add_vertex(-7, 16, 0);
	add_vertex(6, 21, 0);
	add_vertex(40, 10, 1);
	add_vertex(32, 1, 0);
	add_vertex(0, 1, 0);
	add_poly_fill_source(10, 10);
	fix_display_triangles_fan();

 add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0

 nshape[NSHAPE_COMPONENT_LONG5].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_LONG5].mirrored_object_noncentre [1] = 2;
 nshape[NSHAPE_COMPONENT_LONG5].mirrored_object_noncentre [2] = 1;
 nshape[NSHAPE_COMPONENT_LONG5].mirrored_object_noncentre [3] = 4;
 nshape[NSHAPE_COMPONENT_LONG5].mirrored_object_noncentre [4] = 3;

	finish_shape();


// ******* NSHAPE_COMPONENT_SNUB *******


 start_dshape(NSHAPE_COMPONENT_SNUB, KEYWORD_COMPONENT_SNUB);

	poly_index = POLY_0;
	add_link_at_xy(0, // link_index
	               25, 0, // centre
	               34, -8, // left
	               34, 8, // right
	               41, 0, // far
	               43, 0, // link
	               39, 0); // object

	add_link_at_xy(1, // link_index
	               -28, -10, // centre
	               -41, -8, // left
	               -26, -19, // right
	               -37, -19, // far
	               -38, -21, // link
	               -35, -17); // object

	add_link_at_xy(2, // link_index
	               -28, 10, // centre
	               -26, 19, // left
	               -41, 8, // right
	               -37, 19, // far
	               -38, 21, // link
	               -35, 17); // object

	add_link_at_xy(3, // link_index
	               7, -16, // centre
	               -1, -27, // left
	               16, -22, // right
	               7, -28, // far
	               7, -30, // link
	               7, -26); // object

	add_link_at_xy(4, // link_index
	               7, 16, // centre
	               16, 22, // left
	               -1, 27, // right
	               7, 28, // far
	               7, 30, // link
	               7, 26); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

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
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(7, -14, 0);
	add_vertex(23, 0, 0);
	add_vertex(7, 14, 0);
	add_vertex(-44, 6, 1);
	add_vertex(-44, -6, 1);
	add_poly_fill_source(-10, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-3, -28, 1);
	add_vertex(-24, -21, 1);
	add_vertex(-26, -11, 0);
	add_vertex(5, -16, 0);
	add_poly_fill_source(-12, -19);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-3, 28, 1);
	add_vertex(-24, 21, 1);
	add_vertex(-26, 11, 0);
	add_vertex(5, 16, 0);
	add_poly_fill_source(-12, 19);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(8, -15, 0);
	add_vertex(18, -21, 0);
	add_vertex(34, -10, 1);
	add_vertex(24, -1, 0);
	add_poly_fill_source(21, -11);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(8, 15, 0);
	add_vertex(18, 21, 0);
	add_vertex(34, 10, 1);
	add_vertex(24, 1, 0);
	add_poly_fill_source(21, 11);
	fix_display_triangles_fan();

 add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0

 nshape[NSHAPE_COMPONENT_SNUB].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_SNUB].mirrored_object_noncentre [1] = 2;
 nshape[NSHAPE_COMPONENT_SNUB].mirrored_object_noncentre [2] = 1;
 nshape[NSHAPE_COMPONENT_SNUB].mirrored_object_noncentre [3] = 4;
 nshape[NSHAPE_COMPONENT_SNUB].mirrored_object_noncentre [4] = 3;

	finish_shape();

// ******* NSHAPE_COMPONENT_CAP *******

 start_dshape(NSHAPE_COMPONENT_CAP, KEYWORD_COMPONENT_CAP);


	poly_index = POLY_0;
	add_link_at_xy(0, // link_index
	               -7, 29, // centre
	               -4, 43, // left
	               -21, 28, // right
	               -13, 41, // far
	               -15, 45, // link
	               -13, 41); // object

	add_link_at_xy(1, // link_index
	               -19, 10, // centre
	               -25, 23, // left
	               -30, 7, // right
	               -31, 16, // far
	               -33, 18, // link
	               -30, 16); // object

	add_link_at_xy(2, // link_index
	               -19, -10, // centre
	               -30, -7, // left
	               -25, -23, // right
	               -31, -16, // far
	               -33, -18, // link
	               -30, -16); // object

	add_link_at_xy(3, // link_index
	               -7, -29, // centre
	               -21, -28, // left
	               -4, -43, // right
	               -13, -41, // far
	               -15, -45, // link
	               -13, -41); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

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
	add_vertex(-9, 40, 1);
	add_outline_vertex_at_last_poly_vertex(4);
	add_vertex(-27, 13, 1);
	add_outline_vertex_at_last_poly_vertex(4);
	add_vertex(-16, 0, 0);
//	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-27, -13, 1);
	add_outline_vertex_at_last_poly_vertex(4);
	add_vertex(-9, -40, 1);
	add_outline_vertex_at_last_poly_vertex(4);
	add_poly_fill_source(-6, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-10, -12, 0);
	add_vertex(12, -3, 0);
	add_vertex(12, 3, 0);
	add_vertex(-10, 12, 0);
	add_vertex(-32, 4, 1);
	add_vertex(-32, -4, 1);
	add_poly_fill_source(-10, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(12, -6, 0);
	add_vertex(7, -38, 1);
	add_vertex(0, -49, 1);
	add_vertex(-9, -14, 0);
	add_poly_fill_source(2, -26);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-11, -14, 0);
	add_vertex(-18, -11, 0);
	add_vertex(-24, -26, 1);
	add_vertex(-7, -27, 0);
	add_poly_fill_source(-15, -19);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(12, 6, 0);
	add_vertex(7, 38, 1);
	add_vertex(0, 49, 1);
	add_vertex(-9, 14, 0);
	add_poly_fill_source(2, 26);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-11, 14, 0);
	add_vertex(-18, 11, 0);
	add_vertex(-24, 26, 1);
	add_vertex(-7, 27, 0);
	add_poly_fill_source(-15, 19);
	fix_display_triangles_fan();

 add_mirror_axis(0, 0);
 add_mirror_axis(ANGLE_2, 0);

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
	add_link_at_xy(0, // link_index
	               29, 0, // centre
	               35, -6, // left
	               35, 6, // right
	               43, 0, // far
	               45, 0, // link
	               42, 0); // object

	add_link_at_xy(1, // link_index
	               -3, -12, // centre
	               -12, -20, // left
	               4, -20, // right
	               -4, -26, // far
	               -4, -25, // link
	               -4, -23); // object

	add_link_at_xy(2, // link_index
	               -3, 12, // centre
	               4, 20, // left
	               -12, 20, // right
	               -4, 26, // far
	               -4, 25, // link
	               -4, 23); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

	add_vertex(41, 0, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-3, 21, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-27, 0, 0);
	add_outline_vertex_at_last_poly_vertex(6);
	add_vertex(-3, -21, 1);
	add_outline_vertex_at_last_poly_vertex(6);
	add_poly_fill_source(2, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(3, 0, 0);
	add_vertex(-4, 11, 0);
	add_vertex(-15, 20, 1);
	add_vertex(-28, 0, 1);
	add_vertex(-15, -20, 1);
	add_vertex(-4, -11, 0);
	add_poly_fill_source(-10, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(4, -1, 0);
	add_vertex(-2, -11, 0);
	add_vertex(6, -20, 1);
	add_vertex(35, -8, 1);
	add_vertex(28, -1, 0);
	add_poly_fill_source(14, -8);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(4, 1, 0);
	add_vertex(-2, 11, 0);
	add_vertex(6, 20, 1);
	add_vertex(35, 8, 1);
	add_vertex(28, 1, 0);
	add_poly_fill_source(14, 8);
	fix_display_triangles_fan();

 add_mirror_axis(0, 0);
 add_mirror_axis(ANGLE_2, 0);

 nshape[NSHAPE_COMPONENT_TRI].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_TRI].mirrored_object_noncentre [1] = 2;
 nshape[NSHAPE_COMPONENT_TRI].mirrored_object_noncentre [2] = 1;

 add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0

	finish_shape();

// ******* NSHAPE_COMPONENT_PRONG *******

 start_dshape(NSHAPE_COMPONENT_PRONG, KEYWORD_COMPONENT_PRONG);

	poly_index = POLY_0;
	add_link_at_xy(0, // link_index
	               20, -11, // centre
	               16, -22, // left
	               31, -8, // right
	               28, -19, // far
	               31, -21, // link
	               27, -18); // object

	add_link_at_xy(1, // link_index
	               20, 11, // centre
	               31, 8, // left
	               16, 22, // right
	               28, 19, // far
	               31, 21, // link
	               27, 18); // object

	add_link_at_xy(2, // link_index
	               0, -10, // centre
	               -10, -17, // left
	               5, -21, // right
	               -3, -24, // far
	               -4, -26, // link
	               -3, -23); // object

	add_link_at_xy(3, // link_index
	               0, 10, // centre
	               5, 21, // left
	               -10, 17, // right
	               -3, 24, // far
	               -4, 26, // link
	               -3, 23); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

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
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(7, 0, 0);
	add_vertex(-2, 9, 0);
	add_vertex(-13, 17, 0);
	add_vertex(-53, 5, 1);
	add_vertex(-53, -5, 1);
	add_vertex(-13, -17, 0);
	add_vertex(-1, -9, 0);
	add_poly_fill_source(-18, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(8, -1, 0);
	add_vertex(13, -1, 0);
	add_vertex(19, -9, 0);
	add_vertex(13, -25, 1);
	add_vertex(8, -25, 1);
	add_vertex(1, -9, 0);
	add_poly_fill_source(10, -11);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(8, 1, 0);
	add_vertex(13, 1, 0);
	add_vertex(19, 9, 0);
	add_vertex(13, 25, 1);
	add_vertex(8, 25, 1);
	add_vertex(1, 9, 0);
	add_poly_fill_source(10, 11);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(14, 0, 0);
	add_vertex(21, -9, 0);
	add_vertex(43, -3, 1);
	add_vertex(43, 3, 1);
	add_vertex(21, 9, 0);
	add_poly_fill_source(28, 0);
	fix_display_triangles_fan();

 nshape[NSHAPE_COMPONENT_PRONG].mirrored_object_noncentre [0] = 1;
 nshape[NSHAPE_COMPONENT_PRONG].mirrored_object_noncentre [1] = 0;
 nshape[NSHAPE_COMPONENT_PRONG].mirrored_object_noncentre [2] = 3;
 nshape[NSHAPE_COMPONENT_PRONG].mirrored_object_noncentre [3] = 2;

// add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0 - has no mirror axes

	finish_shape();


// ******* NSHAPE_COMPONENT_FORK *******

 start_dshape(NSHAPE_COMPONENT_FORK, KEYWORD_COMPONENT_FORK);

	poly_index = POLY_0;
	add_link_at_xy(0, // link_index
	               7, 0, // centre
	               14, -7, // left
	               14, 7, // right
	               21, 0, // far
	               24, 0, // link
	               21, 0); // object

	add_link_at_xy(1, // link_index
	               -13, -14, // centre
	               -21, -11, // left
	               -11, -26, // right
	               -22, -23, // far
	               -25, -25, // link
	               -22, -23); // object

	add_link_at_xy(2, // link_index
	               -13, 14, // centre
	               -11, 26, // left
	               -21, 11, // right
	               -22, 23, // far
	               -25, 25, // link
	               -22, 23); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

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
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-4, -1, 0);
	add_vertex(-12, -13, 0);
	add_vertex(-7, -41, 1);
	add_vertex(1, -41, 1);
	add_vertex(15, -10, 1);
	add_vertex(6, -1, 0);
	add_poly_fill_source(0, -17);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-4, 1, 0);
	add_vertex(-12, 13, 0);
	add_vertex(-7, 41, 1);
	add_vertex(1, 41, 1);
	add_vertex(15, 10, 1);
	add_vertex(6, 1, 0);
	add_poly_fill_source(0, 17);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-5, 0, 0);
	add_vertex(-13, -12, 0);
	add_vertex(-38, -4, 1);
	add_vertex(-38, 4, 1);
	add_vertex(-13, 12, 0);
	add_poly_fill_source(-21, 0);
	fix_display_triangles_fan();

 nshape[NSHAPE_COMPONENT_FORK].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_FORK].mirrored_object_noncentre [1] = 2;
 nshape[NSHAPE_COMPONENT_FORK].mirrored_object_noncentre [2] = 1;

 add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0

	finish_shape();



// ******* NSHAPE_COMPONENT_BOWL *******


 start_dshape(NSHAPE_COMPONENT_BOWL, KEYWORD_COMPONENT_BOWL);


	poly_index = POLY_0;
	add_link_at_xy(0, // link_index
	               -27, 0, // centre
	               -32, 8, // left
	               -32, -8, // right
	               -39, 0, // far
	               -42, 0, // link
	               -39, 0); // object

	add_link_at_xy(1, // link_index
	               -12, -33, // centre
	               -19, -26, // left
	               -13, -42, // right
	               -22, -37, // far
	               -26, -39, // link
	               -22, -37); // object

	add_link_at_xy(2, // link_index
	               -12, 33, // centre
	               -13, 42, // left
	               -19, 26, // right
	               -22, 37, // far
	               -26, 39, // link
	               -22, 37); // object

	add_link_at_xy(3, // link_index
	               9, -8, // centre
	               14, -21, // left
	               16, -6, // right
	               20, -13, // far
	               23, -13, // link
	               19, -13); // object

	add_link_at_xy(4, // link_index
	               9, 8, // centre
	               16, 6, // left
	               14, 21, // right
	               20, 13, // far
	               23, 13, // link
	               19, 13); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

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
	add_vertex(-36, 0, 1);
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
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-10, -12, 0);
	add_vertex(17, -4, 1);
	add_vertex(17, 4, 1);
	add_vertex(-10, 12, 0);
	add_vertex(-25, 0, 1);
	add_poly_fill_source(-2, 0);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(7, -9, 0);
	add_vertex(13, -23, 1);
	add_vertex(0, -49, 1);
	add_vertex(-13, -54, 1);
	add_vertex(-9, -14, 0);
	add_poly_fill_source(0, -29);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(7, 9, 0);
	add_vertex(13, 23, 1);
	add_vertex(0, 49, 1);
	add_vertex(-13, 54, 1);
	add_vertex(-9, 14, 0);
	add_poly_fill_source(0, 29);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-11, -15, 0);
	add_vertex(-12, -30, 0);
	add_vertex(-32, -12, 1);
	add_vertex(-26, -2, 0);
	add_poly_fill_source(-20, -14);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-11, 15, 0);
	add_vertex(-12, 30, 0);
	add_vertex(-32, 12, 1);
	add_vertex(-26, 2, 0);
	add_poly_fill_source(-20, 14);
	fix_display_triangles_fan();

 add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0

 nshape[NSHAPE_COMPONENT_BOWL].mirrored_object_noncentre [0] = 0;
 nshape[NSHAPE_COMPONENT_BOWL].mirrored_object_noncentre [1] = 2;
 nshape[NSHAPE_COMPONENT_BOWL].mirrored_object_noncentre [2] = 1;
 nshape[NSHAPE_COMPONENT_BOWL].mirrored_object_noncentre [3] = 4;
 nshape[NSHAPE_COMPONENT_BOWL].mirrored_object_noncentre [4] = 3;

	finish_shape();


// ******* NSHAPE_COMPONENT_PEAK *******

 start_dshape(NSHAPE_COMPONENT_PEAK, KEYWORD_COMPONENT_PEAK);

	poly_index = POLY_0;
	add_link_at_xy(0, // link_index
	               -19, 0, // centre
	               -25, 8, // left
	               -25, -8, // right
	               -34, 0, // far
	               -37, 0, // link
	               -32, 0); // object

	add_link_at_xy(1, // link_index
	               -11, -26, // centre
	               -23, -30, // left
	               -3, -33, // right
	               -14, -37, // far
	               -15, -40, // link
	               -14, -35); // object

	add_link_at_xy(2, // link_index
	               -11, 26, // centre
	               -3, 33, // left
	               -23, 30, // right
	               -14, 37, // far
	               -15, 40, // link
	               -14, 35); // object

	add_link_at_xy(3, // link_index
	               12, -15, // centre
	               16, -23, // left
	               22, -10, // right
	               22, -19, // far
	               24, -20, // link
	               20, -18); // object

	add_link_at_xy(4, // link_index
	               12, 15, // centre
	               22, 10, // left
	               16, 23, // right
	               22, 19, // far
	               24, 20, // link
	               20, 18); // object


	start_dshape_poly(poly_index++, 0, PROC_COL_UNDERLAY);

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
	add_vertex(-30, 0, 1);
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
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-11, -24, 0);
	add_vertex(26, -7, 0);
	add_vertex(27, -1, 1);
	add_vertex(-18, -1, 0);
	add_vertex(-26, -12, 0);
	add_poly_fill_source(0, -9);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-11, 24, 0);
	add_vertex(26, 7, 0);
	add_vertex(27, 1, 1);
	add_vertex(-18, 1, 0);
	add_vertex(-26, 12, 0);
	add_poly_fill_source(0, 9);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(11, -16, 0);
	add_vertex(16, -26, 1);
	add_vertex(2, -36, 1);
	add_vertex(-9, -25, 0);
	add_poly_fill_source(5, -25);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(11, 16, 0);
	add_vertex(16, 26, 1);
	add_vertex(2, 36, 1);
	add_vertex(-9, 25, 0);
	add_poly_fill_source(5, 25);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-13, -25, 0);
	add_vertex(-43, -33, 1);
	add_vertex(-38, -16, 0);
	add_vertex(-26, -14, 0);
	add_poly_fill_source(-30, -22);
	fix_display_triangles_fan();

	start_dshape_poly(poly_index++, 1, PROC_COL_MAIN_1);

	add_vertex(-13, 25, 0);
	add_vertex(-43, 33, 1);
	add_vertex(-38, 16, 0);
	add_vertex(-26, 14, 0);
	add_poly_fill_source(-30, 22);
	fix_display_triangles_fan();

 add_mirror_axis_at_link(0, -1); // -1 means no link opposite 0

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



//   DEAR SAM YOUNG,
//   HAPPY BIRTHDAY
//   FROM MERI



// now, add a collision vertex near the middle (this will prevent things like shapes spawning on top of other shapes):
  nshape[ds].vertex_angle_fixed [nshape[ds].vertices] = 0;
  nshape[ds].vertex_dist_fixed [nshape[ds].vertices] = al_itofix(3); // but not exactly on the middle (could cause div by zero somewhere?)
  nshape[ds].vertex_dist_pixel [nshape[ds].vertices] = 3;

  nshape[ds].vertices ++;


}

static void start_dshape_poly(int poly, int layer, int poly_colour_level)
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
		dshape[dshape_init.current_nshape].display_triangles [poly] = 0;
		dshape[dshape_init.current_nshape].poly_layer [poly] = layer;
		dshape[dshape_init.current_nshape].poly_colour_level [poly] = poly_colour_level;

//  dshape_init.current_dshape = ds;
  dshape_init.current_poly = poly;

//  dshape_init.current_nshape = ds; // currently assume nshape index = dshape index
	 dshape[dshape_init.current_nshape].polys ++;

  collision_mask_poly[dshape_init.current_nshape].polys ++;
  collision_mask_poly[dshape_init.current_nshape].vertices [poly] = 0;

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
	dshape[dshape_init.current_nshape].outline_vertex_angle [dshape[dshape_init.current_dshape].outline_vertices] = fixed_to_radians(dshape_init.last_vertex_angle);
	dshape[dshape_init.current_nshape].outline_vertex_dist [dshape[dshape_init.current_dshape].outline_vertices] = al_fixtof(dshape_init.last_vertex_dist_fixed) + extra_distance;

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
	dshape[dshape_init.current_nshape].outline_vertex_angle [dshape[dshape_init.current_dshape].outline_vertices] = fixed_to_radians(dshape[dshape_init.current_nshape].outline_vertex_angle_fixed [dshape[dshape_init.current_dshape].outline_vertices]);
	dshape[dshape_init.current_nshape].outline_vertex_dist [dshape[dshape_init.current_dshape].outline_vertices] = al_fixtof(dshape[dshape_init.current_nshape].outline_vertex_dist_fixed [dshape[dshape_init.current_dshape].outline_vertices]);

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


static void add_display_triangle(int v1, int v2, int v3)
{
	dshape [dshape_init.current_dshape].display_triangle_index [dshape_init.current_poly] [dshape [dshape_init.current_dshape].display_triangles [dshape_init.current_poly]] [0] = v1;
	dshape [dshape_init.current_dshape].display_triangle_index [dshape_init.current_poly] [dshape [dshape_init.current_dshape].display_triangles [dshape_init.current_poly]] [1] = v2;
	dshape [dshape_init.current_dshape].display_triangle_index [dshape_init.current_poly] [dshape [dshape_init.current_dshape].display_triangles [dshape_init.current_poly]] [2] = v3;
	dshape [dshape_init.current_dshape].display_triangles [dshape_init.current_poly] ++;

//fpr("\nADT %i poly %i (%i,%i,%i) dt %i", dshape_init.current_dshape, dshape_init.current_poly, v1,v2,v3,dshape [dshape_init.current_dshape].display_triangles [dshape_init.current_poly]);

}


/*
static void fix_display_triangles_walk(void)
{
 int triangle_vertex [3] = {0, 1, 2};
 int next_walk_vertex = 1;
 int next_vertex_index = 3;

 int total_vertices = dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly] - 1;

 while(TRUE)
	{
		add_display_triangle(triangle_vertex [0], triangle_vertex [1], triangle_vertex [2]);
		triangle_vertex [next_walk_vertex] = next_vertex_index;
		next_vertex_index ++;

		next_walk_vertex ++;
		if (next_walk_vertex == 3)
			next_walk_vertex = 0;
		if (next_vertex_index > total_vertices)
			break;

	};
/ *
 while(low_v < stop_point && high_v >= stop_point && low_v < high_v)
	{
		add_display_triangle(low_v, high_v, low_v + 1);
		if (high_v <= stop_point)
			break;
		add_display_triangle(high_v, low_v + 1, high_v - 1);
  if (dshape_init.current_dshape == NSHAPE_CORE_STATIC_PENT
	  && dshape_init.current_poly == 1)
	 {
		 fpr("\n A: %i, %i, %i", low_v, high_v, low_v + 1);
		 fpr("\n B: %i, %i, %i", high_v, low_v + 1, high_v - 1);
	 }
		low_v ++;
		high_v --;
	};

 if (dshape_init.current_dshape == NSHAPE_CORE_STATIC_PENT
	 && dshape_init.current_poly == 1)
	 wait_for_space();
* /

//fpr("\nFinished shape %i poly %i dt %i", dshape_init.current_dshape, dshape_init.current_poly, dshape [dshape_init.current_dshape].display_triangles [dshape_init.current_poly]);
}
*/


static void fix_display_triangles_walk(void)
{
 int low_v, high_v;

 low_v = 0;
 high_v = dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly] - 1;

 int triangles = 0;
 int total_triangles = dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly] - 2;


 while(TRUE)
	{
		add_display_triangle(low_v, high_v, low_v + 1);
		triangles ++;
		if (triangles >= total_triangles)
			break;
//		if (high_v <= stop_point)
//			break;
		add_display_triangle(high_v, low_v + 1, high_v - 1);
		triangles ++;
		if (triangles >= total_triangles)
			break;
		low_v ++;
		high_v --;
	};

}

// fans out from vertex 0
static void fix_display_triangles_fan(void)
{

 int i;
 int vertices = dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly];

 for (i = 1; i < vertices - 1; i++)
	{
		add_display_triangle(0, i, i + 1);
	}


}


/*

static void fix_display_triangles_walk(void)
{
 int low_v, high_v;

 low_v = 0;
 high_v = dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly] - 1;

// int stop_point = high_v - 2;

// polygons with odd numbers of vertices draw an additional triangle:
// if (dshape[dshape_init.current_dshape].display_vertices [dshape_init.current_poly] & 1)
//		stop_point = high_v - 1;
/ *
 if (dshape_init.current_dshape == NSHAPE_CORE_STATIC_PENT
	 && dshape_init.current_poly == 1)
	{
		fpr("\n low_v %i high_v %i stop_point %i", low_v, high_v, stop_point);

	}* /

// while(low_v < stop_point)// && high_v >= stop_point)
 while(TRUE)// && high_v >= stop_point)
	{
		add_display_triangle(low_v, high_v, low_v + 1);
  if (dshape_init.current_dshape == NSHAPE_CORE_STATIC_PENT
	  && dshape_init.current_poly == 1)
	 {
		 fpr("\n A: %i, %i, %i", low_v, high_v, low_v + 1);
	 }
//		if (high_v <= stop_point)
//			break;
		add_display_triangle(high_v, low_v + 1, high_v - 1);
  if (dshape_init.current_dshape == NSHAPE_CORE_STATIC_PENT
	  && dshape_init.current_poly == 1)
	 {
		 fpr("\n B: %i, %i, %i", high_v, low_v + 1, high_v - 1);
	 }
		low_v ++;
		high_v --;
	};

 if (dshape_init.current_dshape == NSHAPE_CORE_STATIC_PENT
	 && dshape_init.current_poly == 1)
	 wait_for_space();


//fpr("\nFinished shape %i poly %i dt %i", dshape_init.current_dshape, dshape_init.current_poly, dshape [dshape_init.current_dshape].display_triangles [dshape_init.current_poly]);
}



*/

static void finish_shape(void)
{

 struct dshape_struct* dsh = &dshape[dshape_init.current_dshape];
 struct nshape_struct* nsh = &nshape[dshape_init.current_nshape];

 int i;

 float outline_vertex_x [OUTLINE_VERTICES];
 float outline_vertex_y [OUTLINE_VERTICES];

 for (i = 0; i < dsh->outline_vertices; i ++)
	{
		outline_vertex_x [i] = cos(dsh->outline_vertex_angle [i]) * dsh->outline_vertex_dist [i];
		outline_vertex_y [i] = sin(dsh->outline_vertex_angle [i]) * dsh->outline_vertex_dist [i];
// these values are used just below
	}

 for (i = 0; i < dsh->outline_vertices; i ++)
	{
		int previous_vertex = i - 1;
		if (previous_vertex == -1)
		 previous_vertex = dsh->outline_vertices - 1;
		int next_vertex = i + 1;
		if (next_vertex == dsh->outline_vertices)
			next_vertex = 0;

		float mid_point_x = (outline_vertex_x [i]	+ outline_vertex_x [previous_vertex]) / 2;
		float mid_point_y = (outline_vertex_y [i]	+ outline_vertex_y [previous_vertex]) / 2;

		dsh->outline_vertex_side_angle_offset [i] [0] = atan2(mid_point_y, mid_point_x) - dsh->outline_vertex_angle [i];
		dsh->outline_vertex_side_angle_dist [i] [0] = hypot(mid_point_y, mid_point_x); // distance from centre

		mid_point_x = (outline_vertex_x [i]	+ outline_vertex_x [next_vertex]) / 2;
		mid_point_y = (outline_vertex_y [i]	+ outline_vertex_y [next_vertex]) / 2;

		dsh->outline_vertex_side_angle_offset [i] [1] = atan2(mid_point_y, mid_point_x) - dsh->outline_vertex_angle [i];
		dsh->outline_vertex_side_angle_dist [i] [1] = hypot(mid_point_y, mid_point_x); // distance from centre

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


