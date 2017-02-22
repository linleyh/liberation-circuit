

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
#include "m_input.h"
#include "e_log.h"
#include "m_maths.h"

#include "p_panels.h"

#include "d_draw.h"
#include "d_design.h"
#include "d_geo.h"

#include "g_shapes.h"
#include "g_motion.h"


extern struct nshape_struct nshape [NSHAPES];
extern struct dshape_struct dshape [NSHAPES]; // uses same indices as NSHAPES

void update_design_member_position_recursively(struct template_struct* templ, int m);
int check_template_member_collision(struct template_struct* templ, int m);
int check_nshape_nshape_collision(struct nshape_struct* nshape1, int nshape2_index, al_fixed sh1_x, al_fixed sh1_y, al_fixed sh1_angle, al_fixed sh2_x, al_fixed sh2_y, al_fixed sh2_angle);


// this uses its own templ pointer instead of dwindow.templ because it is called during code compilation from the fixer function in c_fix.c
//  (and we may not want to assume that the compiler will only ever be called on dwindow.templ)
void update_design_member_positions(struct template_struct* templ)
{
	int i;

	for (i = 1; i < GROUP_CONNECTIONS; i ++)
	{
		if (templ->member[0].connection[i].template_member_index != -1)
			update_design_member_position_recursively(templ, templ->member[0].connection[i].template_member_index);
	}

}

// call this when a member (but not the core) moves or is moved
void update_design_member_position_recursively(struct template_struct* templ, int m)
{

   struct nshape_struct* parent_nshape = &nshape[templ->member[templ->member[m].connection[0].template_member_index].shape];
   struct nshape_struct* child_nshape = &nshape[templ->member[m].shape];

	  templ->member[m].group_angle_offset = templ->member[templ->member[m].connection[0].template_member_index].group_angle_offset
	                                              + parent_nshape->link_angle_fixed [templ->member[m].connection[0].reverse_link_index] // - AFX_ANGLE_4
	                                              + (AFX_ANGLE_2 - child_nshape->link_angle_fixed [templ->member[m].connection[0].link_index])
	                                              + templ->member[m].connection_angle_offset;

/*
	   templ->member[m].position.x = templ->member[templ->member[m].connection[0].template_member_index].position.x
                 + (symmetrical_cos(templ->member[templ->member[m].connection[0].template_member_index].group_angle_offset
																		+ parent_nshape->link_angle_fixed [templ->member[m].connection[0].reverse_link_index])
																		* (parent_nshape->link_dist_pixel [templ->member[m].connection[0].reverse_link_index]))
																	- (symmetrical_cos(templ->member[m].group_angle_offset + child_nshape->link_angle_fixed [templ->member[m].connection[0].link_index])
																		* (get_link_dist_pixel(child_nshape->link_dist_pixel [templ->member[m].connection[0].link_index], templ->member[m].connection_angle_offset)));
    templ->member[m].position.y = templ->member[templ->member[m].connection[0].template_member_index].position.y
                 + (symmetrical_sin(templ->member[templ->member[m].connection[0].template_member_index].group_angle_offset
																		+ parent_nshape->link_angle_fixed [templ->member[m].connection[0].reverse_link_index])
																		* (parent_nshape->link_dist_pixel [templ->member[m].connection[0].reverse_link_index]))
																	- (symmetrical_sin(templ->member[m].group_angle_offset + child_nshape->link_angle_fixed [templ->member[m].connection[0].link_index])
																		* (get_link_dist_pixel(child_nshape->link_dist_pixel [templ->member[m].connection[0].link_index], templ->member[m].connection_angle_offset)));
*/


	   templ->member[m].position.x = templ->member[templ->member[m].connection[0].template_member_index].position.x
                 + (symmetrical_cos(templ->member[templ->member[m].connection[0].template_member_index].group_angle_offset
																		+ parent_nshape->link_angle_fixed [templ->member[m].connection[0].reverse_link_index])
																		* (parent_nshape->link_dist_pixel [templ->member[m].connection[0].reverse_link_index]))
																	- (symmetrical_cos(templ->member[m].group_angle_offset + child_nshape->link_angle_fixed [templ->member[m].connection[0].link_index])
																		* (child_nshape->link_dist_pixel [templ->member[m].connection[0].link_index]));
    templ->member[m].position.y = templ->member[templ->member[m].connection[0].template_member_index].position.y
                 + (symmetrical_sin(templ->member[templ->member[m].connection[0].template_member_index].group_angle_offset
																		+ parent_nshape->link_angle_fixed [templ->member[m].connection[0].reverse_link_index])
																		* (parent_nshape->link_dist_pixel [templ->member[m].connection[0].reverse_link_index]))
																	- (symmetrical_sin(templ->member[m].group_angle_offset + child_nshape->link_angle_fixed [templ->member[m].connection[0].link_index])
																		* (child_nshape->link_dist_pixel [templ->member[m].connection[0].link_index]));

 templ->member[m].approximate_angle_offset = get_angle(templ->member[m].position.y, templ->member[m].position.x);
 templ->member[m].approximate_distance = distance(templ->member[m].position.y, templ->member[m].position.x);

 int i;

 for (i = 1; i < GROUP_CONNECTIONS; i ++) // note i begins at 1
	{
		if (templ->member[m].connection[i].template_member_index != -1)
			update_design_member_position_recursively(templ, templ->member[m].connection[i].template_member_index);
	}

// note: be careful about calling other design functions from this one,
//  as they may use dwindow.templ.



}

// should be int
int get_link_dist_pixel(int base_dist, al_fixed link_angle_offset)
{

 al_fixed new_dist;

 if (link_angle_offset >= 0 && link_angle_offset < al_itofix(128))
	 new_dist = base_dist * ((al_itofix(1) / 4) * 3 + link_angle_offset / 100);
	  else
  	 new_dist = base_dist * ((al_itofix(1) / 4) * 3 + (al_itofix(255) - link_angle_offset) / 100);
	return al_fixtoi(new_dist);


}

// Is called by the compiler as well as the designer
// returns the number of collisions found (may double-count some collisions so not exact, but will always be 0 if no collisions and 1+ if any collisions)
int check_template_collisions(struct template_struct* templ)
{

 int m;
 int number_of_collisions = 0;

 for (m = 0; m < GROUP_MAX_MEMBERS; m ++)
	{
		templ->member[m].collision = 0;
	}


 for (m = 0; m < GROUP_MAX_MEMBERS; m ++)
	{
		if (templ->member[m].exists != 0)
		 number_of_collisions += check_template_member_collision(templ, m); // I think this does unnecessary double-checking. TO DO: check and fix if needed
	}

	return number_of_collisions;

}

// checks template member's collision vertices against other design members.
//  and vice versa
// Is called by the compiler as well as the designer
// Currently always returns 0, but sets template member collision values to 1 if collision found
int check_template_member_collision(struct template_struct* templ, int m)
{

	int i;
	struct template_member_struct* other_mem;
	int number_of_collisions = 0;

	for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (m == i
			||	templ->member[i].exists == 0)
		 continue;
// put bounding box test here!!!
		other_mem = &templ->member[i];
		if (check_nshape_nshape_collision(&nshape[templ->member[m].shape],
																																				other_mem->shape,
																																				templ->member[m].position.x, templ->member[m].position.y, templ->member[m].group_angle_offset,
																																				other_mem->position.x, other_mem->position.y, other_mem->group_angle_offset) != -1)
  {
  	templ->member[m].collision = 1;
  	templ->member[i].collision = 1;
  	number_of_collisions++;
  }

	}

 return number_of_collisions;
}


// returns number of obstructions found
//  - may count each obstruction multiple times, so only test 0 or !=0
int check_move_objects_obstruction(struct template_struct* templ)
{

	int i, j;
	int number_of_obstructions = 0;

	for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		templ->member[i].move_obstruction = 0;
	}

	for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (templ->member[i].exists == 0)
		 continue;
		for (j = 0; j < nshape[templ->member[i].shape].links; j ++)
		{
			if (templ->member[i].object[j].type != OBJECT_TYPE_MOVE)
				continue;
   number_of_obstructions += check_single_move_object_obstruction(templ, i, j);
		}
	}

	return number_of_obstructions;

}

// is called by check_move_objects_obstruction() and also by functions in d_design.c when a move object is placed or rotated
int check_single_move_object_obstruction(struct template_struct* templ, int member_index, int object_index)
{

 int number_of_obstructions = 0;

// reset template error if it's MOVE_OBSTRUCTED (other kinds of errors are dealt with elsewhere)
 if (templ->member[member_index].object[object_index].template_error == TEMPLATE_OBJECT_ERROR_MOVE_OBSTRUCTED)
  templ->member[member_index].object[object_index].template_error = TEMPLATE_OBJECT_ERROR_NONE;

	al_fixed test_x = templ->member[member_index].position.x + symmetrical_cos(templ->member[member_index].group_angle_offset + nshape[templ->member[member_index].shape].object_angle_fixed [object_index]) * nshape[templ->member[member_index].shape].object_dist_pixel [object_index];
	al_fixed test_y = templ->member[member_index].position.y + symmetrical_sin(templ->member[member_index].group_angle_offset + nshape[templ->member[member_index].shape].object_angle_fixed [object_index]) * nshape[templ->member[member_index].shape].object_dist_pixel [object_index];
 al_fixed object_angle = templ->member[member_index].group_angle_offset + nshape[templ->member[member_index].shape].object_angle_fixed [object_index] + templ->member[member_index].object[object_index].base_angle_offset;

#define MOVE_LINE_GRAIN 10
// a lower MOVE_LINE_GRAIN value increases sensitivity but also processing time

 const al_fixed member_move_collision_box = al_itofix(MOVE_OBSTRUCTION_BOX_SIZE);

 al_fixed increment_x = fixed_xpart(object_angle, al_itofix(MOVE_LINE_GRAIN));
 al_fixed increment_y = fixed_ypart(object_angle, al_itofix(MOVE_LINE_GRAIN));
 al_fixed bound_min = al_itofix(-300); // need to work out how large the design can be
 al_fixed bound_max = al_itofix(300);

 int i;

 while(TRUE)
 {
	 for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
		{
 		if (templ->member[i].exists == 1
				&& i != member_index
				&& test_x > templ->member[i].position.x - member_move_collision_box
				&& test_x < templ->member[i].position.x + member_move_collision_box
				&& test_y > templ->member[i].position.y - member_move_collision_box
				&& test_y < templ->member[i].position.y + member_move_collision_box)
			{
					templ->member[i].move_obstruction = 1;
					templ->member[member_index].object[object_index].template_error = TEMPLATE_OBJECT_ERROR_MOVE_OBSTRUCTED;
					number_of_obstructions ++;
			}
// could just return here and only detect the first obstruction?
		}

	 test_x += increment_x;
	 test_y += increment_y;
	 if (test_x < bound_min
			|| test_x > bound_max
   || test_y < bound_min
   || test_y > bound_max)
				break;
 }

 return number_of_obstructions;

}

//void clear_template_object_errors(struct template_struct* templ)



/*

How will move object obstruction work?

Every component will have a collision square (maybe change to circle? would look nicer)
Move objects project a straight line with some width.
If line intersects a collision square, object is blocked and member gets a collision setting.

*/
