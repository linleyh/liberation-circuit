
#ifndef H_G_SHAPES
#define H_G_SHAPES

#define NSHAPE_VERTICES 32
#define DSHAPE_DISPLAY_VERTICES 32

#define MIRROR_AXES 16

#define OUTLINE_VERTICES 16


enum
{
POLY_0,
POLY_1,
POLY_2,
POLY_3,
POLY_4,
POLY_5,
POLY_6,
POLY_7,
POLY_8,
POLY_9,
POLY_10,
POLY_11,
POLY_12,
POLY_13,
POLY_14,
POLY_15,
DSHAPE_POLYS
};

enum
{
NSHAPE_CORE_STATIC_QUAD,
NSHAPE_CORE_STATIC_PENT,
NSHAPE_CORE_STATIC_HEX_A,
NSHAPE_CORE_STATIC_HEX_B,
NSHAPE_CORE_STATIC_HEX_C,

NSHAPE_CORE_QUAD_A,
#define FIRST_MOBILE_NSHAPE NSHAPE_CORE_QUAD_A
// FIRST_MOBILE_NSHAPE must be the first non-static core
NSHAPE_CORE_QUAD_B,
NSHAPE_CORE_PENT_A,
NSHAPE_CORE_PENT_B,
NSHAPE_CORE_PENT_C,
NSHAPE_CORE_HEX_A,
NSHAPE_CORE_HEX_B,
NSHAPE_CORE_HEX_C,

NSHAPE_COMPONENT_TRI,
#define FIRST_NONCORE_SHAPE NSHAPE_COMPONENT_TRI
NSHAPE_COMPONENT_FORK,
NSHAPE_COMPONENT_BOX,
NSHAPE_COMPONENT_LONG4,
NSHAPE_COMPONENT_CAP,
NSHAPE_COMPONENT_PRONG,
NSHAPE_COMPONENT_LONG5,
NSHAPE_COMPONENT_PEAK,
NSHAPE_COMPONENT_SNUB,
NSHAPE_COMPONENT_BOWL,
NSHAPE_COMPONENT_LONG6,
NSHAPE_COMPONENT_DROP,
NSHAPE_COMPONENT_SIDE,

/*
NSHAPE_COMPONENT_LONG,
#define FIRST_NONCORE_SHAPE NSHAPE_COMPONENT_LONG
NSHAPE_COMPONENT_LONG4,
NSHAPE_COMPONENT_BOX,
NSHAPE_COMPONENT_LONG5,
NSHAPE_COMPONENT_CAP,
NSHAPE_COMPONENT_TRI,
NSHAPE_COMPONENT_PRONG,
NSHAPE_COMPONENT_FORK,
NSHAPE_COMPONENT_BOWL,
*/

//NSHAPE_WARP,
NSHAPES
};

enum
{
	TRI_FAN,
	TRI_WALK,
	TRIANGULATION_METHODS
};

// This struct contains gameplay properties of shapes
struct nshape_struct
{

// collision vertices: used for collision detection. Can't use floats.
 int vertices;
 int keyword_index; // index of the keyword name for this shape in the identifier array (see c_keywords.c)
 int unlock_index; // index in the UNLOCK array. UNLOCK_NONE means that it doesn't need to be unlocked in story mode.

 al_fixed vertex_angle_fixed [NSHAPE_VERTICES]; // used for display
 int vertex_dist_pixel [NSHAPE_VERTICES]; // in pixels - available through data call
 al_fixed vertex_dist_fixed [NSHAPE_VERTICES]; // from centre of shape

 al_fixed max_length; // longest radius (from zero point) of any point in the shape (in GRAIN units)

 int links; // how many links it can have. Probably max 4.
 al_fixed link_angle_fixed [MAX_LINKS];
 al_fixed link_dist_fixed [MAX_LINKS];
 al_fixed link_dist_pixel [MAX_LINKS];
 al_fixed object_angle_fixed [MAX_LINKS];
 al_fixed object_dist_fixed [MAX_LINKS];
 al_fixed object_dist_pixel [MAX_LINKS];
// this point (from the centre) matches where the linked proc's same point is

// these are used in symmetry/mirroring for processes that fall on the centreline of the designer. The core process always will be; others may be
 int mirror_axes;
 int mirror_axis_angle [MIRROR_AXES];
 int mirrored_object_centreline [MIRROR_AXES] [MAX_LINKS]; // mirrored objects for processes that are on the design centreline, e.g. cores (which has special rules for objects below the centreline)
 int mirrored_object_noncentre [MAX_LINKS]; // mirrored objects for non-core processes that are not on the design centreline

// gameplay properties
 int base_hp_max;
// int build_or_restore_time; // time to build (core) or restore (component)
 int shape_mass; // basic mass of shape with no methods (actually should probably calculate this from data cost)
 int data_cost; // data cost of shape with no methods
	int power_capacity;
	int component_power_capacity;
	int interface_charge_rate;
	int instructions_per_cycle;

// int unlock_index;

};



// This struct contains display properties of shapes
struct dshape_struct
{

	// vertices: used only for display (so floats are okay)
 int polys;
 int poly_layer [DSHAPE_POLYS]; // which display layer this poly is on (non-overlapping polys can be on the same layer)
 int poly_colour_level [DSHAPE_POLYS]; // intensity of colour for this poly
 int display_vertices [DSHAPE_POLYS];

 float display_vertex_angle [DSHAPE_POLYS] [DSHAPE_DISPLAY_VERTICES]; // used for display
 float display_vertex_dist [DSHAPE_POLYS] [DSHAPE_DISPLAY_VERTICES]; // used for display

	int triangulation [DSHAPE_POLYS]; // triangulation method used for each polygon
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
	float outline_vertex_pos [OUTLINE_VERTICES] [2];
	int outline_base_vertex;

// these are used in displaying the interface shatter animation. They indicate a midpoint between the vertex and the previous [0] or next [1] vertex
 float outline_vertex_sides [OUTLINE_VERTICES] [2] [2];

};


struct nshape_init_data_struct
{
	int unlock_index;
	int data_cost;
//	int build_or_restore_time;
	int base_hp_max;
	int power_capacity;
	int component_power_capacity;
	int interface_charge_rate;
	int instructions_per_cycle;
};


void init_nshapes_and_dshapes(void);

#endif
