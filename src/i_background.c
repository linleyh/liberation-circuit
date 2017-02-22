
#ifdef BSHAPES

// code for complex background shapes - not currently used

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <math.h>
#include <stdio.h>

#include "m_config.h"
#include "g_header.h"
#include "g_misc.h"
#include "m_maths.h"
#include "i_background.h"


struct bshape_init_struct
{
	struct bshape_struct* current_bshape;
};
struct bshape_init_struct bshape_init;

struct bshape_struct bshape [BSHAPES];
struct background_block_struct backblock [BACKBLOCK_TYPES];



static void start_bshape(int bshape_index);
static void add_bshape_vertex(int x, int y);
static void add_bshape_vertex_vector(int angle, int dist);
static void fix_bshape_triangles_walk(void);
static void add_bshape_triangle(int v1, int v2, int v3);

void init_bshapes(void)
{

 start_bshape(BSHAPE_THING);
 add_bshape_vertex(0, 40);
 add_bshape_vertex(20, 0);

 add_bshape_vertex(0, -20);
 add_bshape_vertex(-20, 0);

 fix_bshape_triangles_walk();


}

static void start_bshape(int bshape_index)
{

	bshape[bshape_index].vertices = 0;
	bshape[bshape_index].triangles = 0;

	bshape_init.current_bshape = &bshape[bshape_index];

}


static void add_bshape_vertex(int x, int y)
{

#ifdef SANITY_CHECK
 if (bshape_init.current_bshape->vertices >= BSHAPE_VERTICES)
	{
		fpr("\n Error: i_background.c: add_bshape_vertex(): too many vertices");
		error_call();
	}
#endif

 bshape_init.current_bshape->vertex_angle [bshape_init.current_bshape->vertices] = atan2(y, x);
 bshape_init.current_bshape->vertex_dist [bshape_init.current_bshape->vertices] = hypot(y, x);
 bshape_init.current_bshape->vertices ++;

}


static void add_bshape_vertex_vector(int angle, int dist)
{


#ifdef SANITY_CHECK
 if (bshape_init.current_bshape->vertices >= BSHAPE_VERTICES)
	{
		fpr("\n Error: i_background.c: add_bshape_vertex_vector(): too many vertices");
		error_call();
	}
#endif

 bshape_init.current_bshape->vertex_angle [bshape_init.current_bshape->vertices] = angle_to_radians(angle);
 bshape_init.current_bshape->vertex_dist [bshape_init.current_bshape->vertices] = dist;
 bshape_init.current_bshape->vertices ++;

}



// generates the triangle index (which orders vertices)
static void fix_bshape_triangles_walk(void)
{
 int low_v, high_v;

 low_v = 0;
 high_v = bshape_init.current_bshape->vertices - 1;

 while(low_v < high_v - 2)
	{
		add_bshape_triangle(low_v, high_v, low_v + 1);
		add_bshape_triangle(high_v, low_v + 1, high_v - 1);
		low_v ++;
		high_v --;
	};

//fpr("\nFinished shape %i poly %i dt %i", dshape_init.current_dshape, dshape_init.current_poly, dshape [dshape_init.current_dshape].display_triangles [dshape_init.current_poly]);
}

// only called by fix_bshape_triangles_walk()
static void add_bshape_triangle(int v1, int v2, int v3)
{

	bshape_init.current_bshape->triangle_index [bshape_init.current_bshape->triangles] [0] = v1;
	bshape_init.current_bshape->triangle_index [bshape_init.current_bshape->triangles] [1] = v2;
	bshape_init.current_bshape->triangle_index [bshape_init.current_bshape->triangles] [2] = v3;
	bshape_init.current_bshape->triangles++;

//fpr("\nADT %i poly %i (%i,%i,%i) dt %i", dshape_init.current_dshape, dshape_init.current_poly, v1,v2,v3,dshape [dshape_init.current_dshape].display_triangles [dshape_init.current_poly]);

}


#endif
