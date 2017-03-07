#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>


#include <stdio.h>
#include <math.h>

#include "m_config.h"
#include "m_globvars.h"

#include "g_header.h"
#include "g_shapes.h"
#include "g_cloud.h"

#include "g_misc.h"
#include "m_maths.h"
#include "g_motion.h"

//#define COLLISION_FORCE_DIVISOR_LIGHTER 4
//#define COLLISION_FORCE_DIVISOR_HEAVIER 16

#define COLLISION_FORCE_DIVISOR_LIGHTER 16
#define COLLISION_FORCE_DIVISOR_HEAVIER 16

static void check_block_collision(struct proc_struct* pr, struct block_struct* bl);
static int check_notional_block_collision(struct block_struct* bl, int notional_shape, al_fixed notional_x, al_fixed notional_y, al_fixed notional_angle, int notional_proc_mobile, int notional_proc_player_index, struct core_struct** collision_core);

//static int check_shape_shape_collision(struct shape_struct* sh1_dat, int sh2_shape, int sh2_size, al_fixed sh1_x, al_fixed sh1_y, al_fixed sh1_angle, al_fixed sh2_x, al_fixed sh2_y, al_fixed sh2_angle);

//extern int collision_mask [SHAPES] [COLLISION_MASK_SIZE] [COLLISION_MASK_SIZE]; // in g_shape.c

//struct shape_struct shape_dat [SHAPES] [SHAPES_SIZES]; // set up in g_shape.c
extern struct nshape_struct nshape [NSHAPES];

void apply_impulse_to_proc_at_vector_point(struct proc_struct* pr, al_fixed point_angle, al_fixed point_dist, al_fixed force, al_fixed impulse_angle);
void apply_impulse_to_proc_at_collision_vertex(struct proc_struct* pr, int cv, al_fixed force, al_fixed impulse_angle);

// group movement functions:
static void move_group(struct core_struct* group_core);

static void proc_group_collision(struct proc_struct* single_pr, struct proc_struct* group_pr, int collision_vertex);
static void group_proc_collision(struct proc_struct* group_pr, struct proc_struct* single_pr, int collision_vertex);
static void group_group_collision(struct proc_struct* pr, struct proc_struct* pr2, int collision_vertex);


static void check_group_collision(struct core_struct* group_core);
static int test_group_collision(struct core_struct* group_core);
static void check_block_collision_group_member(struct proc_struct* pr, struct block_struct* bl);

static void set_group_motion_prov_values(struct core_struct* group_core);
static void set_group_member_prov_values(struct core_struct* group_core, struct proc_struct* pr, struct proc_struct* upstream_pr, int connection_index);

static void fix_group_speed(struct core_struct* group_core);
static al_fixed collision_accel(al_fixed speed, al_fixed accel);

extern struct nshape_struct nshape [NSHAPES];
/*
#define DRAG_TABLE_SIZE 64
al_fixed drag_table [DRAG_TABLE_SIZE] [DRAG_TABLE_SIZE] [2];
// using a lookup table for this may not be great, but it's better than using the fixed-point hypot/atan2 functions for each core each tick
void init_drag_table(void)
{
	int i, j;

	for (i = 0; i < DRAG_TABLE_SIZE; i ++)
	{
		if (i % 10 == 0)
		 fpr("\n***");
 	for (j = 0; j < DRAG_TABLE_SIZE; j ++)
	 {
	 	al_fixed dist = distance(al_itofix(i) / 10, al_itofix(j) / 10);
	 	al_fixed drag_amount = dist / 100;
		 drag_table [i] [j] [0] = (drag_amount) / 10;
		 drag_table [i] [j] [1] = (drag_amount) / 10;
		if (i % 10 == 0)
		 fpr(" (%i,%i:%f,%f)", i, j, al_fixtof(drag_table [i] [j] [0]), al_fixtof(drag_table [i] [j] [1]));
	 }
	}


}
*/

void run_motion(void)
{

//wait_for_space();
 struct core_struct* co;
 struct proc_struct* pr;
 int c, p;
 block_cart notional_block;
 cart notional_position;


 int i;
 for (i = 0; i < w.players; i ++)
	{
		w.player[i].processes = 0;
		w.player[i].components_current = 0; // these values are updated in the loops below
		w.player[i].components_reserved = 0; // these values are updated in the loops below
	}


// First set up movement values for each core
 for (c = 0; c < w.max_cores; c ++)
 {

  if (w.core [c].exists == 0)
   continue;

  co = &w.core [c];

  w.player[co->player_index].processes ++;
  w.player[co->player_index].components_current += co->group_members_current;
  w.player[co->player_index].components_reserved += co->group_members_max;

  co->group_hit_edge_this_cycle = 0; // this is done for groups in set_group_motion_prov_values

/*  fprintf(stdout, "\nCore %i,%i com %i,%i", al_fixtoi(w.proc[co->process_index].position.x),
																																												al_fixtoi(w.proc[co->process_index].position.y),
																																												al_fixtoi(co->group_centre_of_mass.x),
																																												al_fixtoi(co->group_centre_of_mass.y));*/

// fprintf(stdout, "\nnon-group member %i position %i,%i angle %i spin %i", pr->index, al_fixtoi(pr->position.x), al_fixtoi(pr->position.y), al_fixtoi(pr->angle), al_fixtoi(pr->spin));

  co->group_speed.x += fixed_xpart(co->group_angle + co->constant_accel_angle_offset, co->constant_accel_rate);
  co->group_speed.y += fixed_ypart(co->group_angle + co->constant_accel_angle_offset, co->constant_accel_rate);

//  co->group_speed.x = al_fixmul(co->group_speed.x, (al_itofix(1014) / 1024));
//  co->group_speed.y = al_fixmul(co->group_speed.y, (al_itofix(1014) / 1024));

  co->group_speed.x = (co->group_speed.x * 1014) / 1024;
  co->group_speed.y = (co->group_speed.y * 1014) / 1024;
//  co->group_speed.x = (co->group_speed.x * 1010) / 1024;
  //co->group_speed.y = (co->group_speed.y * 1010) / 1024;

/*
  int drag_table_x = al_fixtoi(abs(co->group_speed.x) * 10);
  if (drag_table_x >= DRAG_TABLE_SIZE)
			drag_table_x = DRAG_TABLE_SIZE - 1;
  int drag_table_y = al_fixtoi(abs(co->group_speed.y) * 10);
  if (drag_table_y >= DRAG_TABLE_SIZE)
			drag_table_y = DRAG_TABLE_SIZE - 1;

		fpr("\n co %i dt %i,%i %f,%f", co->index, drag_table_x, drag_table_y, al_fixtof(drag_table [drag_table_x] [drag_table_y] [0]), al_fixtof(drag_table [drag_table_x] [drag_table_y] [1]));

  co->group_speed.x = co->group_speed.x - al_fixmul(co->group_speed.x, drag_table [drag_table_x] [drag_table_y] [0]);
  co->group_speed.y = co->group_speed.y - al_fixmul(co->group_speed.y, drag_table [drag_table_x] [drag_table_y] [1]);
*/
  co->group_spin += co->constant_spin_change;

  if (!co->mobile)
  {
   co->group_speed.x = al_itofix(0);
   co->group_speed.y = al_itofix(0);
   co->group_spin = al_itofix(0);
  }
   else
			{
    co->group_spin = al_fixmul(co->group_spin, (al_itofix(1010) / 1024)); //SPIN_DRAG_BASE_FIXED);

    if (co->group_spin > SPIN_MAX_FIXED)
     co->group_spin = SPIN_MAX_FIXED;
    if (co->group_spin < NEGATIVE_SPIN_MAX_FIXED)
     co->group_spin = NEGATIVE_SPIN_MAX_FIXED;
			}

//  fpr("\ngroup_spin %f constant_spin_change %f", al_fixtof(co->group_spin), al_fixtof(co->constant_spin_change));
  fix_group_speed(co);

  if (co->group_members_current > 1)
  {
   set_group_motion_prov_values(co);
   continue;
  }

// group members don't get past this point
// single-member groups are treated separately because their calculations can be greatly simplified:


  pr = &w.proc[co->process_index];

/*

//  co->group_speed.x = al_fixmul(co->group_speed.x, co->group_drag);
//  co->group_speed.y = al_fixmul(co->group_speed.y, co->group_drag);

  if (!co->mobile)
  {
   co->group_speed.x = al_itofix(0);
   co->group_speed.y = al_itofix(0);
   co->group_spin = al_itofix(0);
  }
   else
			{
    co->group_spin = al_fixmul(co->group_spin, (al_itofix(980) / 1024)); //SPIN_DRAG_BASE_FIXED);

    if (co->group_spin > SPIN_MAX_FIXED)
     co->group_spin = SPIN_MAX_FIXED;
    if (co->group_spin < NEGATIVE_SPIN_MAX_FIXED)
     co->group_spin = NEGATIVE_SPIN_MAX_FIXED;
			}*/

  co->group_test_angle = co->group_angle + co->group_spin;
  pr->provisional_angle = co->group_test_angle + co->group_member[0].angle_offset;
  fix_fixed_angle(&pr->provisional_angle);
  pr->test_angle = pr->provisional_angle;

  pr->spin = co->group_spin;

  pr->test_position.x = pr->position.x + co->group_speed.x;
  pr->test_position.y = pr->position.y + co->group_speed.y;
  pr->provisional_position.x = pr->test_position.x;
  pr->provisional_position.y = pr->test_position.y;
  pr->prov = 1; // provisional_angle has been set above

  pr->speed.x = co->group_speed.x;
  pr->speed.y = co->group_speed.y;

 }


// Second loop: check for any collisions that would occur if all procs moved freely:

 for (c = 0; c < w.max_cores; c ++)
 {
  if (w.core [c].exists == 0)
//			|| w.core [c].mobile	== 0) // immobile cores don't need collision detection   <- this is wrong now
   continue;

  co = &w.core [c];

  if (co->group_members_current > 1)
  {
   check_group_collision(co);
   continue;
  }

  pr = &w.proc [co->process_index];


#ifdef SANITY_CHECK
  if (pr->position.x > w.fixed_size.x - al_itofix(65)
      || pr->position.y > w.fixed_size.y - al_itofix(65)
      || pr->position.x < al_itofix(65)
      || pr->position.y < al_itofix(65))
//      exit(cl->x);
{
      fprintf(stdout, "\n\rError: proc out of bounds xy(%i,%i) wxy(%i,%i) xs %i ys %i (world_time %i)", al_fixtoi(pr->position.x), al_fixtoi(pr->position.y), w.w_pixels, w.h_pixels, al_fixtoi(pr->speed.x), al_fixtoi(pr->speed.y), w.world_time);
      error_call();
}

  if (co->process_index < 0
		 || co->process_index >= w.max_procs)
		{
			fpr("\nError: core process index out of bounds or doesn't exist: core %i process_index %i", c, co->process_index);
			error_call();
		}

  if (pr->exists == 0)
		{
			fpr("\nError: core process index invalid: process %i doesn't exist (core %i)", co->process_index, c);
			error_call();
		}
#endif

// group members don't get past this point

// This code checks the process' vertices against other processes it might collide with.
// It does not check the process against other process' vertices.
//  - this means that immobile procs still need these collision checks to check whether the immobile proc's vertices collide with another proc
//  - also means that all of these collision functions need to be called even after a collision has been found.

//  int left_or_right, up_or_down;

//  left_or_right = (al_fixtoi(pr->position.x) & (BLOCK_SIZE_PIXELS/2)); // assumes BLOCK_SIZE_PIXELS is a power of 2 (should be a safe assumption; currently it's 128
//  up_or_down = (al_fixtoi(pr->position.y) & (BLOCK_SIZE_PIXELS/2));

  check_block_collision(pr, &w.block [pr->block_position.x] [pr->block_position.y]);

//  if (!up_or_down)
		{
//   if (!left_or_right)
				check_block_collision(pr, &w.block [pr->block_position.x - 1] [pr->block_position.y - 1]);
//			  else
     check_block_collision(pr, &w.block [pr->block_position.x + 1] [pr->block_position.y - 1]);
   check_block_collision(pr, &w.block [pr->block_position.x] [pr->block_position.y - 1]);
		}
//		 else
			{
//    if (!left_or_right)
     check_block_collision(pr, &w.block [pr->block_position.x - 1] [pr->block_position.y + 1]);
//      else
      check_block_collision(pr, &w.block [pr->block_position.x + 1] [pr->block_position.y + 1]);
    check_block_collision(pr, &w.block [pr->block_position.x] [pr->block_position.y + 1]);
			}

//  if (!left_or_right)
   check_block_collision(pr, &w.block [pr->block_position.x - 1] [pr->block_position.y]);
// centre checked above
//    else
     check_block_collision(pr, &w.block [pr->block_position.x + 1] [pr->block_position.y]);
/*
  check_block_collision(pr, &w.block [pr->block_position.x - 1] [pr->block_position.y + 1]);
  check_block_collision(pr, &w.block [pr->block_position.x] [pr->block_position.y + 1]);
  check_block_collision(pr, &w.block [pr->block_position.x + 1] [pr->block_position.y + 1]);
*/
// note that if cl is a member of a group, will not get up to here.

// now check for collisions against adjacent solid blocks (only need to check if the proc's current block is an edge type)
  notional_block = cart_to_block(pr->test_position);

  struct nshape_struct* pr_nshape = &nshape [pr->shape];


#define EDGE_ACCEL al_itofix(3)
#define EDGE_ACCEL_MAX al_itofix(4)
#define NEGATIVE_EDGE_ACCEL_MAX al_itofix(-4)

// TO DO: think about using a lookup table with left, right, up and down max_length for each shape for a range of rotations.

 if (w.block [notional_block.x] [notional_block.y].block_type != BLOCK_NORMAL)
 {
 	notional_position = pr->test_position;
  switch(w.block [notional_block.x] [notional_block.y].block_type)
  {
   case BLOCK_SOLID:
#ifdef SANITY_CHECK
    fprintf(stdout, "\nError at time %i: g_motion.c: run_motion(): proc inside solid block.", w.world_time);
//    error_call();
#endif
    break;
   case BLOCK_EDGE_LEFT:
    if (fixed_to_block(notional_position.x - pr_nshape->max_length) != notional_block.x)
    {
     pr->hit_edge_this_cycle = 1;
     co->group_speed.x += EDGE_ACCEL;
     if (co->group_speed.x > EDGE_ACCEL_MAX)
      co->group_speed.x = EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_RIGHT:
    if (fixed_to_block(notional_position.x + pr_nshape->max_length) != notional_block.x)
    {
     pr->hit_edge_this_cycle = 1;
     co->group_speed.x -= EDGE_ACCEL;
     if (co->group_speed.x < NEGATIVE_EDGE_ACCEL_MAX)
      co->group_speed.x = NEGATIVE_EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_UP:
    if (fixed_to_block(notional_position.y - pr_nshape->max_length) != notional_block.y)
    {
     pr->hit_edge_this_cycle = 1;
     co->group_speed.y += EDGE_ACCEL;
     if (co->group_speed.y > EDGE_ACCEL_MAX)
      co->group_speed.y = EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_DOWN:
    if (fixed_to_block(notional_position.y + pr_nshape->max_length) != notional_block.y)
    {
     pr->hit_edge_this_cycle = 1;
     co->group_speed.y -= EDGE_ACCEL;
     if (co->group_speed.y < NEGATIVE_EDGE_ACCEL_MAX)
      co->group_speed.y = NEGATIVE_EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_UP_LEFT:
// up
    if (fixed_to_block(notional_position.y - pr_nshape->max_length) != notional_block.y)
    {
     pr->hit_edge_this_cycle = 1;
     co->group_speed.y += EDGE_ACCEL;
     if (co->group_speed.y > EDGE_ACCEL_MAX)
      co->group_speed.y = EDGE_ACCEL_MAX;
    }
// left
    if (fixed_to_block(notional_position.x - pr_nshape->max_length) != notional_block.x)
    {
     pr->hit_edge_this_cycle = 1;
     co->group_speed.x += EDGE_ACCEL;
     if (co->group_speed.x > EDGE_ACCEL_MAX)
      co->group_speed.x = EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_UP_RIGHT:
// up
    if (fixed_to_block(notional_position.y - pr_nshape->max_length) != notional_block.y)
    {
     pr->hit_edge_this_cycle = 1;
     co->group_speed.y += EDGE_ACCEL;
     if (co->group_speed.y > EDGE_ACCEL_MAX)
      co->group_speed.y = EDGE_ACCEL_MAX;
    }
// right
    if (fixed_to_block(notional_position.x + pr_nshape->max_length) != notional_block.x)
    {
     pr->hit_edge_this_cycle = 1;
     co->group_speed.x -= EDGE_ACCEL;
     if (co->group_speed.x < NEGATIVE_EDGE_ACCEL_MAX)
      co->group_speed.x = NEGATIVE_EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_DOWN_LEFT:
// down
    if (fixed_to_block(notional_position.y + pr_nshape->max_length) != notional_block.y)
    {
     pr->hit_edge_this_cycle = 1;
     co->group_speed.y -= EDGE_ACCEL;
     if (co->group_speed.y < NEGATIVE_EDGE_ACCEL_MAX)
      co->group_speed.y = NEGATIVE_EDGE_ACCEL_MAX;
    }
// left
    if (fixed_to_block(notional_position.x - pr_nshape->max_length) != notional_block.x)
    {
     pr->hit_edge_this_cycle = 1;
     co->group_speed.x += EDGE_ACCEL;
     if (co->group_speed.x > EDGE_ACCEL_MAX)
      co->group_speed.x = EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_DOWN_RIGHT:
// down
    if (fixed_to_block(notional_position.y + pr_nshape->max_length) != notional_block.y)
    {
     pr->hit_edge_this_cycle = 1;
     co->group_speed.y -= EDGE_ACCEL;
     if (co->group_speed.y < NEGATIVE_EDGE_ACCEL_MAX)
      co->group_speed.y = NEGATIVE_EDGE_ACCEL_MAX;
    }
// right
    if (fixed_to_block(notional_position.x + pr_nshape->max_length) != notional_block.x)
    {
     pr->hit_edge_this_cycle = 1;
     co->group_speed.x -= EDGE_ACCEL;
     if (co->group_speed.x < NEGATIVE_EDGE_ACCEL_MAX)
      co->group_speed.x = NEGATIVE_EDGE_ACCEL_MAX;
    }
    break;

  }
 }

 }

// Third loop: let's move all of the procs that haven't collided:
 for (c = 0; c < w.max_cores; c ++)
 {
//  pr->group_check = 0;

  if (w.core [c].exists == 0)
   continue;

  co = &w.core [c];

  if (co->group_members_current > 1)
  {
    if (co->group_hit_edge_this_cycle == 0)
     move_group(co);
    continue;
  }

// group members don't get past this point

  pr = &w.proc[co->process_index];


#ifdef SANITY_CHECK
  if (pr->position.x > w.fixed_size.x - al_itofix(65)
      || pr->position.y > w.fixed_size.y - al_itofix(65)
      || pr->position.x < al_itofix(65)
      || pr->position.y < al_itofix(65))
//      exit(cl->x);
{
      fprintf(stdout, "\n\rError (3rd loop): proc out of bounds xy(%i,%i) wxy(%i,%i) xs %i ys %i (world_time %i)", al_fixtoi(pr->position.x), al_fixtoi(pr->position.y), w.w_pixels, w.h_pixels, al_fixtoi(pr->speed.x), al_fixtoi(pr->speed.y), w.world_time);
      error_call();
}

  if (co->process_index < 0
		 || co->process_index >= w.max_procs)
		{
			fpr("\nError (3rd loop): core process index out of bounds or doesn't exist: core %i process_index %i", c, co->process_index);
			error_call();
		}

  if (pr->exists == 0)
		{
			fpr("\nError (3rd loop): core process index invalid: process %i doesn't exist (core %i)", co->process_index, c);
			error_call();
		}
#endif

  if (pr->hit_edge_this_cycle)
  {
   pr->hit_edge_this_cycle = 0;
   continue;
  }
/*
  if (!co->mobile)
  {
   continue;
  }*/

  pr->old_position = pr->position;
  pr->old_angle = pr->angle;

  pr->position = pr->test_position;
  pr->angle = pr->test_angle;
  co->group_angle = pr->angle - co->group_member[0].angle_offset; // hack
  co->core_position = pr->position;
  co->group_centre_of_mass = pr->position;

  pr->block_position = cart_to_block(pr->position);

 }

// Finally we update block lists based on all proc movement:

 w.blocktag ++;

 int blocktag = w.blocktag;

// Note that unlike the previous loops, this one goes through procs rather than cores:
 for (p = 0; p < w.max_procs; p ++)
 {
  pr = &w.proc [p];

  if (pr->exists == 0)
   continue;

  pr->block_position = cart_to_block(pr->position);

// If the block's blocktag is old, this is the first proc being added to it this tick.
// So, we update the block's blocktag and put the proc on top with no link down.
// We can happily discard any pointers in the w.block struct; it doesn't matter if they are lost.
// We can also assume that x_block and y_block were set above.
  if (w.block [pr->block_position.x] [pr->block_position.y].tag != blocktag)
  {
   w.block [pr->block_position.x] [pr->block_position.y].tag = blocktag;
   pr->blocklist_down = NULL;
   pr->blocklist_up = NULL;
   w.block [pr->block_position.x] [pr->block_position.y].blocklist_down = pr;
  }
   else
   {
// The block's blocktag is up to date. So we put the new proc on top and set its downlink to the top proc of the block:
    pr->blocklist_down = w.block [pr->block_position.x] [pr->block_position.y].blocklist_down;
    pr->blocklist_up = NULL;
    w.block [pr->block_position.x] [pr->block_position.y].blocklist_down = pr;
// we also set the previous top proc's uplink to cl:
    if (pr->blocklist_down != NULL) // not sure that this is necessary.
     pr->blocklist_down->blocklist_up = pr;
   }

 }



}

/*
static void new_collision_check(struct proc_struct* proc1, struct proc_struct* proc2)
{

	al_fixed proc_dist = distance_oct_xyxy(proc1->position.x, proc1->position.y, proc2->position.x, proc2->position.y)

}*/

/*
This function adds a proc to the blocklist of the correct block in world w.

It needs to be called any time a proc is added to a world, but only once for each proc (the block lists are subsequently maintained in run_pass1 above)

Assumes w.blocktag is correct (should be a safe assumption)
*/
void add_proc_to_blocklist(struct proc_struct* pr)
{

// First we find which block the proc is in:
  pr->block_position = cart_to_block(pr->position);

  if (w.block [pr->block_position.x] [pr->block_position.y].tag != w.blocktag)
  {
// The block's blocktag is old, so we just put the new proc on top with no downlink.
   w.block [pr->block_position.x] [pr->block_position.y].tag = w.blocktag;
   pr->blocklist_down = NULL;
   pr->blocklist_up = NULL;
   w.block [pr->block_position.x] [pr->block_position.y].blocklist_down = pr;
  }
   else
   {
// The block's blocktag is up to date. So we put the new proc on top and set its downlink to the top proc of the block.
// ... but first we need to make sure the proc isn't already in the list
//      (this can happen in rare circumstances where a component is destroyed by a stream/slice (which cause damage differently from projectiles) and then restored in the same tick)
    struct proc_struct* check_proc = w.block [pr->block_position.x] [pr->block_position.y].blocklist_down;
    while (check_proc != NULL)
				{
					if (check_proc == pr)
						return; // pr is already on blocklist
					check_proc = check_proc->blocklist_down;
				}
    pr->blocklist_down = w.block [pr->block_position.x] [pr->block_position.y].blocklist_down;
    pr->blocklist_up = NULL;
    w.block [pr->block_position.x] [pr->block_position.y].blocklist_down = pr;
// we also set the previous top proc's uplink to cl:
    if (pr->blocklist_down != NULL) // not sure that this is necessary.
     pr->blocklist_down->blocklist_up = pr;
   }

}




// Any changes to this function should be reflected in check_block_collision_group_member() below
// also see test_group_collision below
// returns 1 if collision, 0 if not
static void check_block_collision(struct proc_struct* pr, struct block_struct* bl)
{


 if (bl->tag != w.blocktag)
  return; // nothing in this block this tick

 struct proc_struct* check_proc;
 check_proc = bl->blocklist_down;

 int collision_vertex;
 cart collision_position;
 cart vertex_speed;

 al_fixed impulse_angle, force;

 struct nshape_struct* pr1_nshape = &nshape [pr->shape];

 while(check_proc != NULL)
 {
// this function is only for non-group processes (i.e. just a core, with no components) so we don't
//  need to exclude the possibility that check_proc is in the same group as pr
  if (check_proc != pr // i.e. we don't check the proc against itself
   && check_proc->exists // a proc destroyed immediately before this may still be on the blocklist, but needs to be ignored
   && (check_proc->player_index != pr->player_index
				|| (w.core[check_proc->core_index].mobile
					&& w.core[pr->core_index].mobile)))
  {

    if (check_proc->position.x + check_proc->nshape_ptr->max_length > pr->provisional_position.x - pr->nshape_ptr->max_length
     && check_proc->position.x - check_proc->nshape_ptr->max_length < pr->provisional_position.x + pr->nshape_ptr->max_length
     && check_proc->position.y + check_proc->nshape_ptr->max_length > pr->provisional_position.y - pr->nshape_ptr->max_length
     && check_proc->position.y - check_proc->nshape_ptr->max_length < pr->provisional_position.y + pr->nshape_ptr->max_length)
     {

        collision_vertex = check_nshape_nshape_collision(pr1_nshape, check_proc->shape, pr->provisional_position.x, pr->provisional_position.y, pr->provisional_angle, check_proc->position.x, check_proc->position.y, check_proc->angle);

// make sure we're not checking provisional vs provisional for each proc:
        if (collision_vertex == -1
									&& pr->core_index < check_proc->core_index)
          collision_vertex = check_nshape_nshape_collision(pr1_nshape, check_proc->shape, pr->provisional_position.x, pr->provisional_position.y, pr->provisional_angle, check_proc->provisional_position.x, check_proc->provisional_position.y, check_proc->provisional_angle);

/*
      collision_vertex = check_nshape_nshape_collision(pr1_nshape, check_proc->shape, pr->provisional_position.x, pr->provisional_position.y, pr->provisional_angle, check_proc->provisional_position.x, check_proc->provisional_position.y, check_proc->provisional_angle);


      if (collision_vertex == -1)
      {
       collision_vertex = check_nshape_nshape_collision(pr1_nshape, check_proc->shape, pr->position.x, pr->position.y, pr->angle, check_proc->provisional_position.x, check_proc->provisional_position.y, check_proc->provisional_angle);

       if (collision_vertex == -1)
       {
        collision_vertex = check_nshape_nshape_collision(pr1_nshape, check_proc->shape, pr->provisional_position.x, pr->provisional_position.y, pr->provisional_angle, check_proc->position.x, check_proc->position.y, check_proc->angle);
       }
      }
*/

/*
To do next: use check_shape_shape_collision to:
 - check collisions for procs added to world at start
 - check collisions for procs added to groups.

- also need to deal with angles for procs added to groups*/


//      if ((collision_vertex = check_proc_proc_collision(pr, check_proc, cp_x, cp_y, cp_angle)) != -1)
     if (collision_vertex != -1)
     {
      pr->hit_pulse_time = w.world_time;
      check_proc->hit_pulse_time = w.world_time;

      if (w.core[check_proc->core_index].group_members_current > 1)
      {
       proc_group_collision(pr, check_proc, collision_vertex);
      }
      else
      {

       w.core[pr->core_index].contact_core_index = check_proc->core_index;
       w.core[pr->core_index].contact_core_timestamp = w.core[check_proc->core_index].created_timestamp;
       w.core[check_proc->core_index].contact_core_index = w.core[pr->core_index].index;
       w.core[check_proc->core_index].contact_core_timestamp = w.core[pr->core_index].created_timestamp;

      	collision_position = cart_plus_vector(pr->provisional_position, pr->provisional_angle + pr->nshape_ptr->vertex_angle_fixed [collision_vertex], pr->nshape_ptr->vertex_dist_fixed [collision_vertex]);
// calculate the velocity of the vertex of pr that hit check_proc:
       vertex_speed = cart_plus_xy(collision_position,
																																			0 - (pr->position.x + fixed_xpart(pr->angle + pr->nshape_ptr->vertex_angle_fixed [collision_vertex], pr->nshape_ptr->vertex_dist_fixed [collision_vertex])),
																																			0 - (pr->position.y + fixed_ypart(pr->angle + pr->nshape_ptr->vertex_angle_fixed [collision_vertex], pr->nshape_ptr->vertex_dist_fixed [collision_vertex])));

       al_fixed struck_point_angle = get_angle(collision_position.y - check_proc->provisional_position.y, collision_position.x - check_proc->provisional_position.x);
       al_fixed struck_point_dist = distance_oct(collision_position.y - check_proc->provisional_position.y, collision_position.x - check_proc->provisional_position.x);
       al_fixed struck_point_old_x = check_proc->position.x + fixed_xpart(struck_point_angle - check_proc->spin, struck_point_dist);
       al_fixed struck_point_old_y = check_proc->position.y + fixed_ypart(struck_point_angle - check_proc->spin, struck_point_dist);
       al_fixed struck_point_speed_x = collision_position.x - struck_point_old_x;
       al_fixed struck_point_speed_y = collision_position.y - struck_point_old_y;

       al_fixed collision_speed_x = vertex_speed.x - struck_point_speed_x;
       al_fixed collision_speed_y = vertex_speed.y - struck_point_speed_y;

       al_fixed collision_speed = distance_oct(collision_speed_y, collision_speed_x);


       impulse_angle = get_angle(collision_position.y - check_proc->provisional_position.y, collision_position.x - check_proc->provisional_position.x);
       force = (collision_speed * check_proc->mass);//al_itofix(100); //pr_force_share / FORCE_DIVISOR;
       if (pr->mass_for_collision_comparison > check_proc->mass_for_collision_comparison)
								force /= COLLISION_FORCE_DIVISOR_HEAVIER;
								 else
  								force /= COLLISION_FORCE_DIVISOR_LIGHTER;
       apply_impulse_to_proc_at_collision_vertex(pr, collision_vertex, force, impulse_angle);

       impulse_angle = get_angle(collision_position.y - pr->provisional_position.y, collision_position.x - pr->provisional_position.x);
       force = (collision_speed * pr->mass);// / COLLISION_FORCE_DIVISOR;//al_itofix(100); //check_proc_force_share / FORCE_DIVISOR;
       if (check_proc->mass_for_collision_comparison > pr->mass_for_collision_comparison)
								force /= COLLISION_FORCE_DIVISOR_HEAVIER;
								 else
  								force /= COLLISION_FORCE_DIVISOR_LIGHTER;
       apply_impulse_to_proc_at_vector_point(check_proc, struck_point_angle, struck_point_dist, force, impulse_angle);

       fix_group_speed(&w.core[pr->core_index]);
       fix_group_speed(&w.core[check_proc->core_index]);

      } // end of "not a group member" code

      }

     }
  }
  check_proc = check_proc->blocklist_down;
 };

}


// Any changes to this function should be reflected in check_block_collision() above
// also see test_group_collision below
// returns 1 if collision, 0 if not
static void check_block_collision_group_member(struct proc_struct* pr, struct block_struct* bl)
{

 if (bl->tag != w.blocktag)
  return;

 struct proc_struct* check_proc;
 check_proc = bl->blocklist_down;
 int collision_vertex;

 struct nshape_struct* pr1_nshape = &nshape [pr->shape];

 while(check_proc != NULL)
 {
  if (check_proc->core_index
						!= pr->core_index // i.e. we don't check the proc against itself or against members of its own group
   && check_proc->exists
   && (check_proc->player_index != pr->player_index
				|| (w.core[check_proc->core_index].mobile
					&& w.core[pr->core_index].mobile)))
  {

    if (check_proc->position.x + check_proc->nshape_ptr->max_length > pr->provisional_position.x - pr->nshape_ptr->max_length
     && check_proc->position.x - check_proc->nshape_ptr->max_length < pr->provisional_position.x + pr->nshape_ptr->max_length
     && check_proc->position.y + check_proc->nshape_ptr->max_length > pr->provisional_position.y - pr->nshape_ptr->max_length
     && check_proc->position.y - check_proc->nshape_ptr->max_length < pr->provisional_position.y + pr->nshape_ptr->max_length)
     {
//      if ((collision_vertex = check_proc_proc_collision(pr, check_proc, cp_x, cp_y, cp_angle)) != -1)
//      collision_vertex = check_proc_proc_collision(pr, check_proc, pr->provisional_x, pr->provisional_y, pr->provisional_angle, check_proc->provisional_x, check_proc->provisional_y, check_proc->provisional_angle);

        collision_vertex = check_nshape_nshape_collision(pr1_nshape, check_proc->shape, pr->provisional_position.x, pr->provisional_position.y, pr->provisional_angle, check_proc->position.x, check_proc->position.y, check_proc->angle);

// make sure we're not checking provisional vs provisional for each proc:
        if (collision_vertex == -1
									&& pr->core_index < check_proc->core_index)
          collision_vertex = check_nshape_nshape_collision(pr1_nshape, check_proc->shape, pr->provisional_position.x, pr->provisional_position.y, pr->provisional_angle, check_proc->provisional_position.x, check_proc->provisional_position.y, check_proc->provisional_angle);


/*
      collision_vertex = check_nshape_nshape_collision(pr1_nshape, check_proc->shape, pr->provisional_position.x, pr->provisional_position.y, pr->provisional_angle, check_proc->provisional_position.x, check_proc->provisional_position.y, check_proc->provisional_angle);


      if (collision_vertex == -1)
      {
       collision_vertex = check_nshape_nshape_collision(pr1_nshape, check_proc->shape, pr->position.x, pr->position.y, pr->angle, check_proc->provisional_position.x, check_proc->provisional_position.y, check_proc->provisional_angle);

       if (collision_vertex == -1)
       {
        collision_vertex = check_nshape_nshape_collision(pr1_nshape, check_proc->shape, pr->provisional_position.x, pr->provisional_position.y, pr->provisional_angle, check_proc->position.x, check_proc->position.y, check_proc->angle);
       }
// TO DO: optimise by not checking provisional values for immobile procs
      }
*/

//      if ((collision_vertex = check_proc_proc_collision(pr, check_proc, cp_x, cp_y, cp_angle)) != -1)
      if (collision_vertex != -1)
      {
       pr->hit_pulse_time = w.world_time;
       check_proc->hit_pulse_time = w.world_time;

       if (w.core[check_proc->core_index].group_members_current > 1)
       {
        group_group_collision(pr, check_proc, collision_vertex);
       }
        else
        {
         group_proc_collision(pr, check_proc, collision_vertex);
        }

      }

     }

  }
  check_proc = check_proc->blocklist_down;
 };

}


// Just calls check_notional_block_collision for a 3x3 set of blocks
//  doesn't check for collision with solid blocks (use check_notional_solid_block_collision() for that; it doesn't need to be called 9 times)
// *does* check for collisions with immobile processes.
// assumes notional_size, shape, x and y are valid
int check_notional_block_collision_multi(int notional_shape, al_fixed notional_x, al_fixed notional_y, al_fixed notional_angle, int notional_proc_mobile, int notional_proc_player_index, struct core_struct** collision_core)
{

 int notional_block_x = fixed_to_block(notional_x);
 int notional_block_y = fixed_to_block(notional_y);

//  int left_or_right, up_or_down;

  //left_or_right = (al_fixtoi(notional_x) & (BLOCK_SIZE_PIXELS/2)); // assumes BLOCK_SIZE_PIXELS is a power of 2 (should be a safe assumption; currently it's 128
  //up_or_down = (al_fixtoi(notional_y) & (BLOCK_SIZE_PIXELS/2));

  if (check_notional_block_collision(&w.block [notional_block_x] [notional_block_y], notional_shape, notional_x, notional_y, notional_angle, notional_proc_mobile, notional_proc_player_index, collision_core))
   return 1;

//  if (!left_or_right)
		{
//   if (!up_or_down)
   {
			 if (check_notional_block_collision(&w.block [notional_block_x - 1] [notional_block_y - 1], notional_shape, notional_x, notional_y, notional_angle, notional_proc_mobile, notional_proc_player_index, collision_core))
     return 1;
   }
//    else
    {
     if (check_notional_block_collision(&w.block [notional_block_x - 1] [notional_block_y + 1], notional_shape, notional_x, notional_y, notional_angle, notional_proc_mobile, notional_proc_player_index, collision_core))
      return 1;
    }
   if (check_notional_block_collision(&w.block [notional_block_x - 1] [notional_block_y], notional_shape, notional_x, notional_y, notional_angle, notional_proc_mobile, notional_proc_player_index, collision_core))
    return 1;
		}
//		 else
			{

//    if (!up_or_down)
    {
     if (check_notional_block_collision(&w.block [notional_block_x + 1] [notional_block_y - 1], notional_shape, notional_x, notional_y, notional_angle, notional_proc_mobile, notional_proc_player_index, collision_core))
      return 1;
    }
//     else
					{
      if (check_notional_block_collision(&w.block [notional_block_x + 1] [notional_block_y + 1], notional_shape, notional_x, notional_y, notional_angle, notional_proc_mobile, notional_proc_player_index, collision_core))
       return 1;

					}

    if (check_notional_block_collision(&w.block [notional_block_x + 1] [notional_block_y], notional_shape, notional_x, notional_y, notional_angle, notional_proc_mobile, notional_proc_player_index, collision_core))
     return 1;

			}

//  if (!up_or_down)
		{
   if (check_notional_block_collision(&w.block [notional_block_x] [notional_block_y - 1], notional_shape, notional_x, notional_y, notional_angle, notional_proc_mobile, notional_proc_player_index, collision_core))
    return 1;
		}
//		 else
			{
//  check_notional_block_collision(&w.block [notional_block_x] [notional_block_y], notional_shape, notional_size, notional_x, notional_y, notional_angle);
    if (check_notional_block_collision(&w.block [notional_block_x] [notional_block_y + 1], notional_shape, notional_x, notional_y, notional_angle, notional_proc_mobile, notional_proc_player_index, collision_core))
     return 1;
			}

  return 0;

}

//  also see test_group_collision and collision_check_ignore_group below

/*
This function checks whether a non-existent proc with notional properties would collide with another proc if created
It will generally be called 9 times for complete coverage of a 3x3 area

It doesn't detect collisions between mobile and friendly static processes (which aren't registered during movement either)

Doesn't check for collision with a solid block (use check_notional_solid_block_collision for that)

returns 1 if collision, 0 if no collision
*/
static int check_notional_block_collision(struct block_struct* bl, int notional_shape, al_fixed notional_x, al_fixed notional_y, al_fixed notional_angle, int notional_proc_mobile, int notional_proc_player_index, struct core_struct** collision_core)
{

 if (bl->tag != w.blocktag)
  return 0; // nothing currently in this block

 struct proc_struct* check_proc;
 check_proc = bl->blocklist_down;

// static int collision_x, collision_y, force, impulse_angle, vertex_speed_x, vertex_speed_y, collision_vertex;

 struct nshape_struct* notional_nshape = &nshape [notional_shape];

 while(check_proc != NULL)
 {
  //if (check_proc != pr) // i.e. we don't check the proc against itself   <---- not needed for notional check
  if (check_proc->exists
			&& (check_proc->mobile == notional_proc_mobile // mobile procs collide with other mobile procs; static with other static
				|| check_proc->player_index != notional_proc_player_index))	// although mobile procs will also collide with enemy static procs
  {

    if (check_proc->position.x + check_proc->nshape_ptr->max_length > notional_x - notional_nshape->max_length
     && check_proc->position.x - check_proc->nshape_ptr->max_length < notional_x + notional_nshape->max_length
     && check_proc->position.y + check_proc->nshape_ptr->max_length > notional_y - notional_nshape->max_length
     && check_proc->position.y - check_proc->nshape_ptr->max_length < notional_y + notional_nshape->max_length)
     {

// This works differently to the usual check_block_collision function.
// It doesn't need to check provisional_x/y values, as the relevant procs will not move until after the next time provisional values are calculated
// However, it does need to call check_shape_shape_collision for both procs (so that both procs' vertices are taken into account)

// TO DO: This will NOT currently detect procs overlapping unless at least one vertex is overlapping, which may not be true.

//      collision_vertex = check_proc_proc_collision(pr, check_proc, pr->provisional_x, pr->provisional_y, pr->provisional_angle, check_proc->provisional_x, check_proc->provisional_y, check_proc->provisional_angle);
      if (check_nshape_nshape_collision(notional_nshape, check_proc->shape, notional_x, notional_y, notional_angle, check_proc->position.x, check_proc->position.y, check_proc->angle) != -1)
						{
// Note that this function is much more precise than the general collision detection code
							*collision_core = &w.core[check_proc->core_index];
       return 1;
						}
// need to do the reverse as well:
      if (check_nshape_nshape_collision(&nshape [check_proc->shape], notional_shape, check_proc->position.x, check_proc->position.y, check_proc->angle, notional_x, notional_y, notional_angle) != -1)
						{
							*collision_core = &w.core[check_proc->core_index];
       return 1;
						}

     }

  }
  check_proc = check_proc->blocklist_down;
 };

// collision_core will not have been set if this function returns 0
 return 0;

}

int check_notional_solid_block_collision(int notional_shape, al_fixed notional_x, al_fixed notional_y, al_fixed notional_angle)
{

  int notional_x_block = fixed_to_block(notional_x);
  int notional_y_block = fixed_to_block(notional_y);
  struct nshape_struct* pr_nshape = &nshape [notional_shape];

// TO DO: think about using a lookup table with left, right, up and down max_length for each shape for a range of rotations.

  switch(w.block [notional_x_block] [notional_y_block].block_type)
  {
   case BLOCK_NORMAL: break; // do nothing
   case BLOCK_SOLID:
#ifdef SANITY_CHECK
//    fprintf(stdout, "\nError: g_motion.c: check_notional_solid_block_collision(): proc inside solid block.");
//    error_call();
#endif
    return 1; // not an error - this function can be called when one proc is trying to create another and the new proc would be in a solid block.
   case BLOCK_EDGE_LEFT:
    if (fixed_to_block(notional_x - pr_nshape->max_length) != notional_x_block)
    {
     return 1;
    }
    break;
   case BLOCK_EDGE_RIGHT:
    if (fixed_to_block(notional_x + pr_nshape->max_length) != notional_x_block)
    {
     return 1;
    }
    break;
   case BLOCK_EDGE_UP:
    if (fixed_to_block(notional_y - pr_nshape->max_length) != notional_y_block)
    {
     return 1;
    }
    break;
   case BLOCK_EDGE_DOWN:
    if (fixed_to_block(notional_y + pr_nshape->max_length) != notional_y_block)
    {
     return 1;
    }
    break;
   case BLOCK_EDGE_UP_LEFT:
// up
    if (fixed_to_block(notional_y - pr_nshape->max_length) != notional_y_block)
    {
     return 1;
    }
// left
    if (fixed_to_block(notional_x - pr_nshape->max_length) != notional_x_block)
    {
     return 1;
    }
    break;
   case BLOCK_EDGE_UP_RIGHT:
// up
    if (fixed_to_block(notional_y - pr_nshape->max_length) != notional_y_block)
    {
     return 1;
    }
// right
    if (fixed_to_block(notional_x + pr_nshape->max_length) != notional_x_block)
    {
     return 1;
    }
    break;
   case BLOCK_EDGE_DOWN_LEFT:
// down
    if (fixed_to_block(notional_y + pr_nshape->max_length) != notional_y_block)
    {
     return 1;
    }
// left
    if (fixed_to_block(notional_x - pr_nshape->max_length) != notional_x_block)
    {
     return 1;
    }
    break;
   case BLOCK_EDGE_DOWN_RIGHT:
// down
    if (fixed_to_block(notional_y + pr_nshape->max_length) != notional_y_block)
    {
     return 1;
    }
// right
    if (fixed_to_block(notional_x + pr_nshape->max_length) != notional_x_block)
    {
     return 1;
    }
    break;

  }


  return 0; // no collision


}


/*

This function checks whether any of shape shape_dat_1's vertices collide with shape shape_dat_2
It doesn't require either shape to exist or not exist
Has no side effects, and can be used whether either shape is notional or belongs to an actual proc

returns vertex index of shape_1 if collision
returns -1 if no collision

*** Note: DOESN'T check whether any of shape_2's vertices collide with shape_1! Must call in reverse as well to do this.
* /
static int check_shape_shape_collision(struct shape_struct* sh1_dat, int sh2_shape, int sh2_size, al_fixed sh1_x, al_fixed sh1_y, al_fixed sh1_angle, al_fixed sh2_x, al_fixed sh2_y, al_fixed sh2_angle)
{


 al_fixed dist = distance(sh1_y - sh2_y, sh1_x - sh2_x);

 al_fixed angle = get_angle(sh1_y - sh2_y, sh1_x - sh2_x);
 al_fixed angle_diff = angle - sh2_angle;

 unsigned int v, mask_x, mask_y;

 al_fixed sh1_centre_x = MASK_CENTRE_FIXED + fixed_xpart(angle_diff, dist);
 al_fixed sh1_centre_y = MASK_CENTRE_FIXED + fixed_ypart(angle_diff, dist);

 for (v = 0; v < sh1_dat->collision_vertices; v ++)
 {

  mask_x = al_fixtoi(sh1_centre_x + fixed_xpart(sh1_angle + sh1_dat->collision_vertex_angle [v] - sh2_angle, sh1_dat->collision_vertex_dist [v] + al_itofix(1)));
  mask_x >>= COLLISION_MASK_BITSHIFT;
  mask_y = al_fixtoi(sh1_centre_y + fixed_ypart(sh1_angle + sh1_dat->collision_vertex_angle [v] - sh2_angle, sh1_dat->collision_vertex_dist [v] + al_itofix(1)));
  mask_y >>= COLLISION_MASK_BITSHIFT;

  if (mask_x < COLLISION_MASK_SIZE // don't check for < 0 because they're unsigned
   && mask_y < COLLISION_MASK_SIZE)
   {
    if (collision_mask [sh2_shape] [mask_x] [mask_y] <= sh2_size)
    {
     return v;
    }
   }
 }

 return -1;

}
*/



extern unsigned char nshape_collision_mask [NSHAPES] [COLLISION_MASK_SIZE] [COLLISION_MASK_SIZE];

int check_nshape_nshape_collision(struct nshape_struct* nshape1, int nshape2_index, al_fixed sh1_x, al_fixed sh1_y, al_fixed sh1_angle, al_fixed sh2_x, al_fixed sh2_y, al_fixed sh2_angle)
{

 al_fixed dist = distance_oct(sh1_y - sh2_y, sh1_x - sh2_x);

 al_fixed angle = get_angle(sh1_y - sh2_y, sh1_x - sh2_x);
 al_fixed angle_diff = angle - sh2_angle;

 unsigned int v, mask_x, mask_y;

 al_fixed sh1_centre_x = MASK_CENTRE_FIXED + fixed_xpart(angle_diff, dist);
 al_fixed sh1_centre_y = MASK_CENTRE_FIXED + fixed_ypart(angle_diff, dist);

 for (v = 0; v < nshape1->vertices; v ++)
 {

  mask_x = al_fixtoi(sh1_centre_x + fixed_xpart(sh1_angle + nshape1->vertex_angle_fixed [v] - sh2_angle, nshape1->vertex_dist_fixed [v]));// + al_itofix(1)));
  mask_x >>= COLLISION_MASK_BITSHIFT;
  mask_y = al_fixtoi(sh1_centre_y + fixed_ypart(sh1_angle + nshape1->vertex_angle_fixed [v] - sh2_angle, nshape1->vertex_dist_fixed [v]));// + al_itofix(1)));
  mask_y >>= COLLISION_MASK_BITSHIFT;

  if (mask_x < COLLISION_MASK_SIZE // don't check for < 0 because they're unsigned
   && mask_y < COLLISION_MASK_SIZE)
   {
    if (nshape_collision_mask [nshape2_index] [mask_x] [mask_y] >= 2) // >= 2 means ignore 1, which is interface collision
    {
//    	fpr("\ncollision at vertex %i at %i,%i", v, mask_x, mask_y);
     return v;
    }
   }
 }

 return -1;

}




#ifdef NOT_USED
extern ALLEGRO_DISPLAY* display;
extern ALLEGRO_COLOR base_col [BASIC_COLS] [BASIC_SHADES];

#define PLOT_VERTICES 1000
// debug function - no longer used
void plot_point_on_mask(int s, int size, int p1x, int p1y, int p2x, int p2y)
{

// al_set_target_bitmap(display_window);

// al_clear_to_color(base_col [0] [0]);

 ALLEGRO_VERTEX plot_pixel [PLOT_VERTICES];


 int x, y, shade, col2;
 int xa, ya;
 int i = 0;

 for (x = 0; x < COLLISION_MASK_SIZE; x ++)
 {
  for (y = 0; y < COLLISION_MASK_SIZE; y ++)
  {
    shade = SHADE_LOW;
    col2 = COL_GREEN;
    if (collision_mask [s] [size] [x] [y]) // totally wrong now
     shade = SHADE_HIGH;
    if (x == p1x && y == p1y)
    {
     col2 = COL_BLUE;
     shade = SHADE_HIGH;
    }
    if (x == p2x && y == p2y)
    {
     col2 = COL_BLUE;
     shade = SHADE_MAX;
    }
    xa = 500 + x;
    ya = 10 + y;
    if (shade != SHADE_LOW)
    {
     //al_draw_pixel(xa, ya, base_col [col2] [shade]);
     plot_pixel[i].x = xa;
     plot_pixel[i].y = ya;
     plot_pixel[i].z = 0;
     plot_pixel[i].color = colours.base [col2] [shade];
     i ++;
    }
//    al_draw_filled_rectangle(xa, ya, xa + 2, ya + 2, base_col [COL_GREEN] [shade]);
  if (i == PLOT_VERTICES)
  {
   al_draw_prim(plot_pixel, NULL, NULL, 0, PLOT_VERTICES, ALLEGRO_PRIM_POINT_LIST); // May need to put back "-1" after MAP_VERTICES
   i = 0;
  }


  }
 }


 if (i > 0)
 {
   al_draw_prim(plot_pixel, NULL, NULL, 0, i, ALLEGRO_PRIM_POINT_LIST);
 }

 if (settings.option [OPTION_SPECIAL_CURSOR])
  draw_mouse_cursor();
 al_flip_display();

// error_call();

}

#endif


/*
pr is a non-group member, one of whose vertices has collided with a group member (pr2).
(when a group member's vertex hits a non-group proc, use group_proc_collision)
pr2 is a group member.

This function does not handle the reverse situation (group member's vertex collides with non-group member). See group_proc_collision.

*/
static void proc_group_collision(struct proc_struct* single_pr, struct proc_struct* group_pr, int collision_vertex)
{
//fpr("\nproc_group_collision!");
       struct core_struct* group_core = &w.core[group_pr->core_index];

       group_core->contact_core_index = single_pr->core_index;
       group_core->contact_core_timestamp = w.core[single_pr->core_index].created_timestamp;
       w.core[single_pr->core_index].contact_core_index = group_core->index;
       w.core[single_pr->core_index].contact_core_timestamp = group_core->created_timestamp;

       al_fixed collision_x = single_pr->provisional_position.x + fixed_xpart(single_pr->provisional_angle + single_pr->nshape_ptr->vertex_angle_fixed [collision_vertex], single_pr->nshape_ptr->vertex_dist_fixed [collision_vertex]);
       al_fixed collision_y = single_pr->provisional_position.y + fixed_ypart(single_pr->provisional_angle + single_pr->nshape_ptr->vertex_angle_fixed [collision_vertex], single_pr->nshape_ptr->vertex_dist_fixed [collision_vertex]);
// calculate the velocity of the vertex of pr that hit check_proc:
       al_fixed vertex_speed_x = collision_x
                      - (single_pr->position.x + fixed_xpart(single_pr->angle + single_pr->nshape_ptr->vertex_angle_fixed [collision_vertex], single_pr->nshape_ptr->vertex_dist_fixed [collision_vertex]));
       al_fixed vertex_speed_y = collision_y
                      - (single_pr->position.y + fixed_ypart(single_pr->angle + single_pr->nshape_ptr->vertex_angle_fixed [collision_vertex], single_pr->nshape_ptr->vertex_dist_fixed [collision_vertex]));

       al_fixed struck_point_angle = get_angle(collision_y - group_pr->provisional_position.y, collision_x - group_pr->provisional_position.x);
       al_fixed struck_point_dist = distance_oct(collision_y - group_pr->provisional_position.y, collision_x - group_pr->provisional_position.x);
       al_fixed struck_point_old_x = group_pr->position.x + fixed_xpart(struck_point_angle - (group_pr->provisional_angle - group_pr->angle), struck_point_dist);
       al_fixed struck_point_old_y = group_pr->position.y + fixed_ypart(struck_point_angle - (group_pr->provisional_angle - group_pr->angle), struck_point_dist);
       al_fixed struck_point_speed_x = collision_x - struck_point_old_x;
       al_fixed struck_point_speed_y = collision_y - struck_point_old_y;

       al_fixed collision_speed_x = vertex_speed_x - struck_point_speed_x;
       al_fixed collision_speed_y = vertex_speed_y - struck_point_speed_y;

       al_fixed collision_speed = distance_oct(collision_speed_y, collision_speed_x);

       al_fixed impulse_angle = get_angle(collision_y - group_pr->provisional_position.y, collision_x - group_pr->provisional_position.x);
//       int mass_proportion = 1;//(single_pr->mass * 100) / group_core->group_mass;
//       mass_proportion *= mass_proportion;
//       mass_proportion /= 100;
//       al_fixed force = (collision_speed * group_core->group_mass * mass_proportion) / (COLLISION_FORCE_DIVISOR*100);//al_itofix(100); //pr_force_share / FORCE_DIVISOR;
       al_fixed force = (collision_speed * group_core->group_mass);// / (COLLISION_FORCE_DIVISOR);//al_itofix(100); //pr_force_share / FORCE_DIVISOR;
       if (single_pr->mass_for_collision_comparison > group_core->group_mass_for_collision_comparison)
								force /= COLLISION_FORCE_DIVISOR_HEAVIER;
								 else
  								force /= COLLISION_FORCE_DIVISOR_LIGHTER;
       apply_impulse_to_proc_at_collision_vertex(single_pr, collision_vertex, force, impulse_angle);
//       single_pr->collided_this_cycle = 1;

       impulse_angle = get_angle(collision_y - single_pr->provisional_position.y, collision_x - single_pr->provisional_position.x);
//       mass_proportion = (group_core->group_mass * 100) / single_pr->mass;
//       mass_proportion *= mass_proportion;
//       mass_proportion /= 100;
       force = (collision_speed * single_pr->mass);// / (COLLISION_FORCE_DIVISOR);//al_itofix(100); //check_proc_force_share / FORCE_DIVISOR;
       if (group_core->group_mass_for_collision_comparison > single_pr->mass_for_collision_comparison)
								force /= COLLISION_FORCE_DIVISOR_HEAVIER;
								 else
  								force /= COLLISION_FORCE_DIVISOR_LIGHTER;
       apply_impulse_to_group(group_core, struck_point_old_x, struck_point_old_y, force, impulse_angle);
//       group_pr->collided_this_cycle = 1;
//       gr2->collided_this_cycle = 1;

       fix_group_speed(&w.core[single_pr->core_index]);
       fix_group_speed(group_core);



}




/*
Opposite of proc_group_collision - here, a group member's vertex collides with a non-group-member

*/
static void group_proc_collision(struct proc_struct* group_pr, struct proc_struct* single_pr, int collision_vertex)
{

       struct core_struct* group_core = &w.core[group_pr->core_index];

       group_core->contact_core_index = single_pr->core_index;
       group_core->contact_core_timestamp = w.core[single_pr->core_index].created_timestamp;
       w.core[single_pr->core_index].contact_core_index = group_core->index;
       w.core[single_pr->core_index].contact_core_timestamp = group_core->created_timestamp;

       al_fixed collision_x = group_pr->provisional_position.x + fixed_xpart(group_pr->provisional_angle + group_pr->nshape_ptr->vertex_angle_fixed [collision_vertex], group_pr->nshape_ptr->vertex_dist_fixed [collision_vertex]);
       al_fixed collision_y = group_pr->provisional_position.y + fixed_ypart(group_pr->provisional_angle + group_pr->nshape_ptr->vertex_angle_fixed [collision_vertex], group_pr->nshape_ptr->vertex_dist_fixed [collision_vertex]);
// calculate the velocity of the vertex of pr that hit check_proc:
       al_fixed vertex_speed_x = collision_x
                      - (group_pr->position.x + fixed_xpart(group_pr->angle + group_pr->nshape_ptr->vertex_angle_fixed [collision_vertex], group_pr->nshape_ptr->vertex_dist_fixed [collision_vertex]));
       al_fixed vertex_speed_y = collision_y
                      - (group_pr->position.y + fixed_ypart(group_pr->angle + group_pr->nshape_ptr->vertex_angle_fixed [collision_vertex], group_pr->nshape_ptr->vertex_dist_fixed [collision_vertex]));

       al_fixed struck_point_angle = get_angle(collision_y - single_pr->provisional_position.y, collision_x - single_pr->provisional_position.x);
       al_fixed struck_point_dist = distance_oct(collision_y - single_pr->provisional_position.y, collision_x - single_pr->provisional_position.x);
       al_fixed struck_point_old_x = single_pr->position.x + fixed_xpart(struck_point_angle - (single_pr->provisional_angle - single_pr->angle), struck_point_dist);
       al_fixed struck_point_old_y = single_pr->position.y + fixed_ypart(struck_point_angle - (single_pr->provisional_angle - single_pr->angle), struck_point_dist);
       al_fixed struck_point_speed_x = collision_x - struck_point_old_x;
       al_fixed struck_point_speed_y = collision_y - struck_point_old_y;

       al_fixed collision_speed_x = vertex_speed_x - struck_point_speed_x;
       al_fixed collision_speed_y = vertex_speed_y - struck_point_speed_y;

       al_fixed collision_speed = distance_oct(collision_speed_y, collision_speed_x);

       al_fixed impulse_angle = get_angle(collision_y - single_pr->provisional_position.y, collision_x - single_pr->provisional_position.x);
//       int mass_proportion = 1;//(single_pr->mass * 100) / group_core->group_mass;
//       mass_proportion *= mass_proportion;
//       mass_proportion /= 100;
       al_fixed force = (collision_speed * single_pr->mass);// * mass_proportion);// / (COLLISION_FORCE_DIVISOR);//al_itofix(100); //pr_force_share / FORCE_DIVISOR;
       if (group_core->group_mass_for_collision_comparison > single_pr->mass_for_collision_comparison)
								force /= COLLISION_FORCE_DIVISOR_HEAVIER;
								 else
  								force /= COLLISION_FORCE_DIVISOR_LIGHTER;

       apply_impulse_to_group_at_member_vertex(group_core, group_pr, collision_vertex, force, impulse_angle);
//       apply_impulse_to_proc_at_collision_vertex(pr, collision_vertex, force, impulse_angle);
//       group_pr->collided_this_cycle = 1;
//       group_core->collided_this_cycle = 1;

       impulse_angle = get_angle(collision_y - group_pr->provisional_position.y, collision_x - group_pr->provisional_position.x);
//       mass_proportion = (group_core->group_mass * 100) / single_pr->mass;
//       mass_proportion *= mass_proportion;
//       mass_proportion /= 100;
       force = (collision_speed * group_core->group_mass);//al_itofix(100); //check_proc_force_share / FORCE_DIVISOR;
       if (single_pr->mass_for_collision_comparison > group_core->group_mass_for_collision_comparison)
								force /= COLLISION_FORCE_DIVISOR_HEAVIER;
								 else
  								force /= COLLISION_FORCE_DIVISOR_LIGHTER;
       apply_impulse_to_proc_at_vector_point(single_pr, struck_point_angle, struck_point_dist, force, impulse_angle);
//       apply_impulse_to_group(gr2, struck_point_old_x, struck_point_old_y, force, impulse_angle);
//       single_pr->collided_this_cycle = 1;

       fix_group_speed(&w.core[single_pr->core_index]);
       fix_group_speed(group_core);



}


/*

Group/group collisions.
First group is the group whose member's vertex hit a member of the second group

*/
static void group_group_collision(struct proc_struct* vertex_pr, struct proc_struct* other_pr, int collision_vertex)
{

       struct core_struct* group_core1 = &w.core[vertex_pr->core_index];
       struct core_struct* group_core2 = &w.core[other_pr->core_index];

       group_core1->contact_core_index = group_core2->index;
       group_core1->contact_core_timestamp = group_core2->created_timestamp;
       group_core2->contact_core_index = group_core1->index;
       group_core2->contact_core_timestamp = group_core1->created_timestamp;

       al_fixed collision_x = vertex_pr->provisional_position.x + fixed_xpart(vertex_pr->provisional_angle + vertex_pr->nshape_ptr->vertex_angle_fixed [collision_vertex], vertex_pr->nshape_ptr->vertex_dist_fixed [collision_vertex]);
       al_fixed collision_y = vertex_pr->provisional_position.y + fixed_ypart(vertex_pr->provisional_angle + vertex_pr->nshape_ptr->vertex_angle_fixed [collision_vertex], vertex_pr->nshape_ptr->vertex_dist_fixed [collision_vertex]);
// calculate the velocity of the vertex of pr that hit check_proc:
       al_fixed vertex_speed_x = collision_x
                      - (vertex_pr->position.x + fixed_xpart(vertex_pr->angle + vertex_pr->nshape_ptr->vertex_angle_fixed [collision_vertex], vertex_pr->nshape_ptr->vertex_dist_fixed [collision_vertex]));
       al_fixed vertex_speed_y = collision_y
                      - (vertex_pr->position.y + fixed_ypart(vertex_pr->angle + vertex_pr->nshape_ptr->vertex_angle_fixed [collision_vertex], vertex_pr->nshape_ptr->vertex_dist_fixed [collision_vertex]));

       al_fixed struck_point_angle = get_angle(collision_y - other_pr->provisional_position.y, collision_x - other_pr->provisional_position.x);
       al_fixed struck_point_dist = distance_oct(collision_y - other_pr->provisional_position.y, collision_x - other_pr->provisional_position.x);
       al_fixed struck_point_old_x = other_pr->position.x + fixed_xpart(struck_point_angle - (other_pr->provisional_angle - other_pr->angle), struck_point_dist);
       al_fixed struck_point_old_y = other_pr->position.y + fixed_ypart(struck_point_angle - (other_pr->provisional_angle - other_pr->angle), struck_point_dist);
       al_fixed struck_point_speed_x = collision_x - struck_point_old_x;
       al_fixed struck_point_speed_y = collision_y - struck_point_old_y;

       al_fixed collision_speed_x = vertex_speed_x - struck_point_speed_x;
       al_fixed collision_speed_y = vertex_speed_y - struck_point_speed_y;

       al_fixed collision_speed = distance_oct(collision_speed_y, collision_speed_x);

       al_fixed impulse_angle = get_angle(collision_y - other_pr->provisional_position.y, collision_x - other_pr->provisional_position.x);
       al_fixed force = (collision_speed * group_core2->group_mass);//al_itofix(100); //pr_force_share / FORCE_DIVISOR;
       if (group_core1->group_mass_for_collision_comparison > group_core2->group_mass_for_collision_comparison)
								force /= COLLISION_FORCE_DIVISOR_HEAVIER;
								 else
  								force /= COLLISION_FORCE_DIVISOR_LIGHTER;

       apply_impulse_to_group_at_member_vertex(group_core1, vertex_pr, collision_vertex, force, impulse_angle);
//       vertex_pr->collided_this_cycle = 1;
//       gr1->collided_this_cycle = 1;

       impulse_angle = get_angle(collision_y - vertex_pr->provisional_position.y, collision_x - vertex_pr->provisional_position.x);
       force = (collision_speed * group_core1->group_mass);//al_itofix(100); //check_proc_force_share / FORCE_DIVISOR;
       if (group_core2->group_mass_for_collision_comparison > group_core1->group_mass_for_collision_comparison)
								force /= COLLISION_FORCE_DIVISOR_HEAVIER;
								 else
  								force /= COLLISION_FORCE_DIVISOR_LIGHTER;
       apply_impulse_to_group(group_core2, struck_point_old_x, struck_point_old_y, force, impulse_angle);
//       other_pr->collided_this_cycle = 1;
//       gr2->collided_this_cycle = 1;

       fix_group_speed(group_core1);
       fix_group_speed(group_core2);


}

// assumes pr is not a member of a group
void apply_impulse_to_proc_at_vertex(struct proc_struct* pr, int v, al_fixed force, al_fixed impulse_angle)
{
// fpr("\n force %f applied to proc %i (apply_impulse_to_proc_at_vertex)", al_fixtof(force), pr->index);

 if (!w.core[pr->core_index].mobile)
  return;

// force = al_fixmul(force, al_itofix(100));

 al_fixed force_dist_from_centre = pr->nshape_ptr->vertex_dist_fixed [v] / FORCE_DIST_DIVISOR;
 al_fixed lever_angle = pr->nshape_ptr->vertex_angle_fixed [v] + pr->angle;

 al_fixed torque = al_fixmul(al_fixmul(fixed_sin(lever_angle - impulse_angle), force_dist_from_centre), force);

// pr->spin_change -= al_fixdiv(torque, al_itofix(pr->moment));
 pr->spin -= torque / (w.core[pr->core_index].group_moment * TORQUE_DIVISOR);

// fprintf(stdout, "\nproc_at_vertex: v %i impulse_angle %i torque %i spin_change %f", v, fixed_angle_to_int(impulse_angle), fixed_angle_to_int(torque), fixed_to_radians(pr->spin_change));

 w.core[pr->core_index].group_speed.x = collision_accel(w.core[pr->core_index].group_speed.x, al_fixdiv(fixed_xpart(impulse_angle, force), al_itofix(pr->mass)));
 w.core[pr->core_index].group_speed.y = collision_accel(w.core[pr->core_index].group_speed.y, al_fixdiv(fixed_ypart(impulse_angle, force), al_itofix(pr->mass)));

// w.core[pr->core_index].group_speed.x += al_fixdiv(fixed_xpart(impulse_angle, force), al_itofix(pr->mass));
// w.core[pr->core_index].group_speed.y += al_fixdiv(fixed_ypart(impulse_angle, force), al_itofix(pr->mass));

 fix_group_speed(&w.core[pr->core_index]);


}

// assumes pr is not a member of a group
void apply_impulse_to_proc_at_collision_vertex(struct proc_struct* pr, int cv, al_fixed force, al_fixed impulse_angle)
{
// fpr("\n force %f applied to proc %i (apply_impulse_to_proc_at_collision_vertex)", al_fixtof(force), pr->index);

 if (!w.core[pr->core_index].mobile)
  return;

 al_fixed force_dist_from_centre = pr->nshape_ptr->vertex_dist_fixed [cv] / FORCE_DIST_DIVISOR;
 al_fixed lever_angle = pr->nshape_ptr->vertex_angle_fixed [cv] + pr->angle;

 al_fixed torque = al_fixmul(al_fixmul(fixed_sin(lever_angle - impulse_angle), force_dist_from_centre), force);

 pr->spin -= torque / (w.core[pr->core_index].group_moment * TORQUE_DIVISOR);

 w.core[pr->core_index].group_speed.x = collision_accel(w.core[pr->core_index].group_speed.x, al_fixdiv(fixed_xpart(impulse_angle, force), al_itofix(pr->mass)));
 w.core[pr->core_index].group_speed.y = collision_accel(w.core[pr->core_index].group_speed.y, al_fixdiv(fixed_ypart(impulse_angle, force), al_itofix(pr->mass)));

// w.core[pr->core_index].group_speed.x += al_fixdiv(fixed_xpart(impulse_angle, force), al_itofix(pr->mass));
// w.core[pr->core_index].group_speed.y += al_fixdiv(fixed_ypart(impulse_angle, force), al_itofix(pr->mass));

 fix_group_speed(&w.core[pr->core_index]);

}



// assumes pr is not a member of a group
// point_angle and point_dist are a vector from the proc's centre to the collision point.
void apply_impulse_to_proc_at_vector_point(struct proc_struct* pr, al_fixed point_angle, al_fixed point_dist, al_fixed force, al_fixed impulse_angle)
{
// fpr("\n force %f applied to proc %i (apply_impulse_to_proc_at_vector_point)", al_fixtof(force), pr->index);

 if (!w.core[pr->core_index].mobile)
  return;

 al_fixed torque = al_fixmul(al_fixmul(fixed_sin(point_angle - impulse_angle), point_dist), force);

 pr->spin -= torque / (w.core[pr->core_index].group_moment * TORQUE_DIVISOR);

// fprintf(stdout, "\nproc_at_point: pr %i ang,dis %i,%i impulse_angle %i force %f torque %f spin_change %f", pr->index, fixed_angle_to_int(point_angle), al_fixtoi(point_dist), fixed_angle_to_int(impulse_angle), al_fixtof(force), al_fixtof(torque), fixed_to_radians(torque / pr->moment));

 w.core[pr->core_index].group_speed.x = collision_accel(w.core[pr->core_index].group_speed.x, al_fixdiv(fixed_xpart(impulse_angle, force), al_itofix(pr->mass)));
 w.core[pr->core_index].group_speed.y = collision_accel(w.core[pr->core_index].group_speed.y, al_fixdiv(fixed_ypart(impulse_angle, force), al_itofix(pr->mass)));

// w.core[pr->core_index].group_speed.x += al_fixdiv(fixed_xpart(impulse_angle, force), al_itofix(pr->mass));
// w.core[pr->core_index].group_speed.y += al_fixdiv(fixed_ypart(impulse_angle, force), al_itofix(pr->mass));

 fix_group_speed(&w.core[pr->core_index]);


}

static al_fixed collision_accel(al_fixed speed, al_fixed accel)
{

#define COLLISION_ACCEL_CAP 2

	if (speed > 0
		&& accel > 0)
	{
		if (speed > al_itofix(COLLISION_ACCEL_CAP))
		 return speed;
		speed += accel;
		if (speed > al_itofix(COLLISION_ACCEL_CAP))
		 return al_itofix(COLLISION_ACCEL_CAP);
	 return speed;
	}

	if (speed < 0
		&& accel < 0)
	{
		if (speed < al_itofix(-COLLISION_ACCEL_CAP))
		 return speed;
		speed += accel;
		if (speed < al_itofix(-COLLISION_ACCEL_CAP))
		 return al_itofix(-COLLISION_ACCEL_CAP);
	 return speed;
	}

	return speed + accel;


}


static void fix_group_speed(struct core_struct* group_core)
{

 int dist = distance_oct(group_core->group_speed.y, group_core->group_speed.x);
 int dragged = 950;

 if (dist > al_itofix(6))
	{
  if (dist > al_itofix(9))
			dragged = 700;
  group_core->group_speed.x = (group_core->group_speed.x * dragged) / 1024;
  group_core->group_speed.y = (group_core->group_speed.y * dragged) / 1024;
	}

}


// gr should be a core
void apply_impulse_to_group(struct core_struct* group_core, al_fixed x, al_fixed y, al_fixed force, al_fixed impulse_angle)
{
// fpr("\n force %f applied to GROUP (apply_impulse_to_group)", al_fixtof(force));

 if (!group_core->mobile)
  return;

 al_fixed force_dist_from_centre = hypot(y - group_core->group_centre_of_mass.y, x - group_core->group_centre_of_mass.x) / FORCE_DIST_DIVISOR;
 al_fixed lever_angle = get_angle(y - group_core->group_centre_of_mass.y, x - group_core->group_centre_of_mass.x);
 al_fixed torque = al_fixmul(al_fixmul(fixed_sin(lever_angle - impulse_angle), force_dist_from_centre), force); //al_fixdiv(force, TORQUE_DIVISOR_FIXED)));

 group_core->group_spin -= torque / (group_core->group_moment * TORQUE_DIVISOR);

 group_core->group_speed.x = collision_accel(group_core->group_speed.x, al_fixdiv(fixed_xpart(impulse_angle, force), al_itofix(group_core->group_mass)));
 group_core->group_speed.y = collision_accel(group_core->group_speed.y, al_fixdiv(fixed_ypart(impulse_angle, force), al_itofix(group_core->group_mass)));

// group_core->group_speed.x += al_fixdiv(fixed_xpart(impulse_angle, force), al_itofix(group_core->group_mass));
// group_core->group_speed.y += al_fixdiv(fixed_ypart(impulse_angle, force), al_itofix(group_core->group_mass));

 fix_group_speed(group_core);

}


void apply_impulse_to_group_at_member_vertex(struct core_struct* group_core, struct proc_struct* pr, int vertex, al_fixed force, al_fixed impulse_angle)
{

// fpr("\n force %f applied to GROUP (apply_impulse_to_group_at_member_vertex)", al_fixtof(force));

 if (!group_core->mobile)
  return;

 al_fixed x = pr->position.x + fixed_xpart(pr->angle + pr->nshape_ptr->vertex_angle_fixed [vertex], pr->nshape_ptr->vertex_dist_fixed [vertex]);
 al_fixed y = pr->position.y + fixed_ypart(pr->angle + pr->nshape_ptr->vertex_angle_fixed [vertex], pr->nshape_ptr->vertex_dist_fixed [vertex]);

 al_fixed force_dist_from_centre = distance_oct(y - group_core->group_centre_of_mass.y, x - group_core->group_centre_of_mass.x) / FORCE_DIST_DIVISOR;
 al_fixed lever_angle = get_angle(y - group_core->group_centre_of_mass.y, x - group_core->group_centre_of_mass.x);
 al_fixed torque = al_fixmul(al_fixmul(fixed_sin(lever_angle - impulse_angle), force_dist_from_centre), force);

// group_core->spin_change -= al_fixdiv(torque, al_itofix(group_core->moment));
 group_core->group_spin -= torque / (group_core->group_moment * TORQUE_DIVISOR);

// fprintf(stdout, "\ngroup: v %i impulse_angle %i torque %i spin_change %f", vertex, fixed_angle_to_int(impulse_angle), fixed_angle_to_int(torque), fixed_to_radians(group_core->spin_change));

 group_core->group_speed.x = collision_accel(group_core->group_speed.x, al_fixdiv(fixed_xpart(impulse_angle, force), al_itofix(group_core->group_mass)));
 group_core->group_speed.y = collision_accel(group_core->group_speed.y, al_fixdiv(fixed_ypart(impulse_angle, force), al_itofix(group_core->group_mass)));

// group_core->group_speed.x += al_fixdiv(fixed_xpart(impulse_angle, force), al_itofix(group_core->group_mass));
// group_core->group_speed.y += al_fixdiv(fixed_ypart(impulse_angle, force), al_itofix(group_core->group_mass));

 fix_group_speed(group_core);

}



static void set_group_motion_prov_values(struct core_struct* group_core)
{


// First set up the group's core values

 group_core->group_hit_edge_this_cycle = 0;

 if (!group_core->mobile)
 {
  group_core->group_speed.x = 0;//al_itofix(0);
  group_core->group_speed.y = 0;//al_itofix(0);
  group_core->group_spin = 0;//al_itofix(0);
// TO DO: can't return here as we need to set up provisional values. But this is inefficient for immobile procs.
//  - maybe just set them up once (or each time group composition changes)? They shouldn't change otherwise

//  return;
 }
/*

replace after testing!!!
  group_core->group_speed.x = al_fixmul(group_core->group_speed.x, group_core->group_drag);
  group_core->group_speed.y = al_fixmul(group_core->group_speed.y, group_core->group_drag);*/

  group_core->group_test_centre_of_mass.x = group_core->group_centre_of_mass.x + group_core->group_speed.x;
  group_core->group_test_centre_of_mass.y = group_core->group_centre_of_mass.y + group_core->group_speed.y;
  group_core->group_test_angle = group_core->group_angle; // spin is added below

//  group_core->spin += group_core->spin_change;
//  group_core->spin_change = 0;

  if (group_core->group_spin != 0)
  {
   if (group_core->group_spin > SPIN_MAX_FIXED)
    group_core->group_spin = SPIN_MAX_FIXED;
   if (group_core->group_spin < NEGATIVE_SPIN_MAX_FIXED)
    group_core->group_spin = NEGATIVE_SPIN_MAX_FIXED;


   group_core->group_test_angle += group_core->group_spin;

   fix_fixed_angle(&group_core->group_test_angle);

//   group_core->group_spin = al_fixmul(group_core->group_spin, SPIN_DRAG_BASE_FIXED); replace after testing!!!

  }
//fprintf(stdout, "ZZZ(spin:%f;angle:%f;test_angle:%f)", al_fixtof(group_core->group_spin), al_fixtof(group_core->group_angle), al_fixtof(group_core->group_test_angle));


// Now set values for each group member, based on the group_core values:
  set_group_member_prov_values(group_core, &w.proc[group_core->process_index], NULL, 0); // connection_index not relevant for core


}

// recursive function that sets the prov values of each member of a group.
// It may be possible to replace this with a simple loop, but I'm not sure.
// upstream_pr is parent process (i.e. process one step closer to core).
// For the core:
//  - upstream_pr is NULL.
//  - connection_index is not used.
static void set_group_member_prov_values(struct core_struct* group_core, struct proc_struct* pr, struct proc_struct* upstream_pr, int connection_index)
{

  if (group_core->mobile)
  {
   if (upstream_pr != NULL)
   {
    pr->test_position.x = upstream_pr->test_position.x + fixed_xpart(upstream_pr->connection_angle [connection_index] + upstream_pr->test_angle, upstream_pr->connection_dist [connection_index]);
    pr->test_position.y = upstream_pr->test_position.y + fixed_ypart(upstream_pr->connection_angle [connection_index] + upstream_pr->test_angle, upstream_pr->connection_dist [connection_index]);
    pr->test_angle = upstream_pr->test_angle + upstream_pr->connection_angle_difference [connection_index];
   }
    else
    {
// core process:
     pr->test_position.x = group_core->group_test_centre_of_mass.x + fixed_xpart(group_core->core_offset_from_group_centre.angle + group_core->group_test_angle, group_core->core_offset_from_group_centre.magnitude);
     pr->test_position.y = group_core->group_test_centre_of_mass.y + fixed_ypart(group_core->core_offset_from_group_centre.angle + group_core->group_test_angle, group_core->core_offset_from_group_centre.magnitude);
     pr->test_angle = get_fixed_fixed_angle(group_core->group_member[0].angle_offset + group_core->group_angle);
    }
  }
   else
   {
    pr->test_position.x = pr->position.x;
    pr->test_position.y = pr->position.y;
    pr->test_angle = pr->angle;
   }

//fprintf(stdout, "[SGMPV:%i]", pr->index);



 pr->provisional_position.x = pr->test_position.x;
 pr->provisional_position.y = pr->test_position.y;
 pr->provisional_angle = pr->test_angle;

 pr->test_block_position.x = fixed_to_block(pr->test_position.x);
 pr->test_block_position.y = fixed_to_block(pr->test_position.y);


// fprintf(stdout, "\ngroup member %i position %i,%i angle %i spin %i", pr->index, al_fixtoi(pr->position.x), al_fixtoi(pr->position.y), al_fixtoi(pr->angle), pr->spin);

 int i;

 for (i = 1; i < GROUP_CONNECTIONS; i ++) // i starts at 1 to avoid parent process
 {
  if (pr->group_connection_exists [i])
  {
   set_group_member_prov_values(group_core, pr->group_connection_ptr [i], pr, i);
  }
 }

}




static void check_group_collision(struct core_struct* group_core)
{

 test_group_collision(group_core);

}


/*

function that checks whether a group can move to a new position.
requires group test position values to have been filled in; this function then fills in the test position values for each proc. If the movement is possible, these test position values are used.

returns 1 if collision, 0 if no collision

*/
static int test_group_collision(struct core_struct* group_core)
{

// return 0;

 int i;
 int x, y;
 struct block_struct* bl;
 struct proc_struct* pr;

 for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{

// possibly terminate list with -2 here

		if (group_core->group_member[i].exists == 0)
			continue;

		pr = &w.proc[group_core->group_member[i].index];

  for (x = -1; x < 2; x ++)
  {
   for (y = -1; y < 2; y ++)
   {

    bl = &w.block [pr->test_block_position.x + x] [pr->test_block_position.y + y];

    if (bl->tag != w.blocktag)
     continue;

    check_block_collision_group_member(&w.proc[group_core->group_member [i].index], bl);

   }
  }

// now let's check for a collision with the edge of the map:

  al_fixed notional_x = pr->test_position.x;
  al_fixed notional_y = pr->test_position.y;
  struct nshape_struct* pr_nshape = &nshape [pr->shape];

  if (w.block [pr->test_block_position.x] [pr->test_block_position.y].block_type != BLOCK_NORMAL)
  {
  switch(w.block [pr->test_block_position.x] [pr->test_block_position.y].block_type)
  {
   case BLOCK_SOLID:
#ifdef SANITY_CHECK
    fprintf(stdout, "\nError: g_motion.c: test_group_collision(): proc inside solid block (proc %i at (%i,%i) block (%i,%i)).", pr->index, notional_x, notional_y, pr->test_block_position.x, pr->test_block_position.y);
    error_call();
#endif
    break;
   case BLOCK_EDGE_LEFT:
    if (fixed_to_block(notional_x - pr_nshape->max_length) != pr->test_block_position.x)
    {
     pr->hit_edge_this_cycle = 1;
     group_core->group_hit_edge_this_cycle = 1;
     group_core->group_speed.x += EDGE_ACCEL;
     if (group_core->group_speed.x > EDGE_ACCEL_MAX)
      group_core->group_speed.x = EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_RIGHT:
    if (fixed_to_block(notional_x + pr_nshape->max_length) != pr->test_block_position.x)
    {
     pr->hit_edge_this_cycle = 1;
     group_core->group_hit_edge_this_cycle = 1;
     group_core->group_speed.x -= EDGE_ACCEL;
     if (group_core->group_speed.x < NEGATIVE_EDGE_ACCEL_MAX)
      group_core->group_speed.x = NEGATIVE_EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_UP:
    if (fixed_to_block(notional_y - pr_nshape->max_length) != pr->test_block_position.y)
    {
     pr->hit_edge_this_cycle = 1;
     group_core->group_hit_edge_this_cycle = 1;
     group_core->group_speed.y += EDGE_ACCEL;
     if (group_core->group_speed.y > EDGE_ACCEL_MAX)
      group_core->group_speed.y = EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_DOWN:
    if (fixed_to_block(notional_y + pr_nshape->max_length) != pr->test_block_position.y)
    {
     pr->hit_edge_this_cycle = 1;
     group_core->group_hit_edge_this_cycle = 1;
     group_core->group_speed.y -= EDGE_ACCEL;
     if (group_core->group_speed.y < NEGATIVE_EDGE_ACCEL_MAX)
      group_core->group_speed.y = NEGATIVE_EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_UP_LEFT:
// up
    if (fixed_to_block(notional_y - pr_nshape->max_length) != pr->test_block_position.y)
    {
     pr->hit_edge_this_cycle = 1;
     group_core->group_hit_edge_this_cycle = 1;
     group_core->group_speed.y += EDGE_ACCEL;
     if (group_core->group_speed.y > EDGE_ACCEL_MAX)
      group_core->group_speed.y = EDGE_ACCEL_MAX;
    }
// left
    if (fixed_to_block(notional_x - pr_nshape->max_length) != pr->test_block_position.x)
    {
     pr->hit_edge_this_cycle = 1;
     group_core->group_hit_edge_this_cycle = 1;
     group_core->group_speed.x += EDGE_ACCEL;
     if (group_core->group_speed.x > EDGE_ACCEL_MAX)
      group_core->group_speed.x = EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_UP_RIGHT:
// up
    if (fixed_to_block(notional_y - pr_nshape->max_length) != pr->test_block_position.y)
    {
     pr->hit_edge_this_cycle = 1;
     group_core->group_hit_edge_this_cycle = 1;
     group_core->group_speed.y += EDGE_ACCEL;
     if (group_core->group_speed.y > EDGE_ACCEL_MAX)
      group_core->group_speed.y = EDGE_ACCEL_MAX;
    }
// right
    if (fixed_to_block(notional_x + pr_nshape->max_length) != pr->test_block_position.x)
    {
     pr->hit_edge_this_cycle = 1;
     group_core->group_hit_edge_this_cycle = 1;
     group_core->group_speed.x -= EDGE_ACCEL;
     if (group_core->group_speed.x < NEGATIVE_EDGE_ACCEL_MAX)
      group_core->group_speed.x = NEGATIVE_EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_DOWN_LEFT:
// down
    if (fixed_to_block(notional_y + pr_nshape->max_length) != pr->test_block_position.y)
    {
     pr->hit_edge_this_cycle = 1;
     group_core->group_hit_edge_this_cycle = 1;
     group_core->group_speed.y -= EDGE_ACCEL;
     if (group_core->group_speed.y < NEGATIVE_EDGE_ACCEL_MAX)
      group_core->group_speed.y = NEGATIVE_EDGE_ACCEL_MAX;
    }
// left
    if (fixed_to_block(notional_x - pr_nshape->max_length) != pr->test_block_position.x)
    {
     pr->hit_edge_this_cycle = 1;
     group_core->group_hit_edge_this_cycle = 1;
     group_core->group_speed.x += EDGE_ACCEL;
     if (group_core->group_speed.x > EDGE_ACCEL_MAX)
      group_core->group_speed.x = EDGE_ACCEL_MAX;
    }
    break;
   case BLOCK_EDGE_DOWN_RIGHT:
// down
    if (fixed_to_block(notional_y + pr_nshape->max_length) != pr->test_block_position.y)
    {
     pr->hit_edge_this_cycle = 1;
     group_core->group_hit_edge_this_cycle = 1;
     group_core->group_speed.y -= EDGE_ACCEL;
     if (group_core->group_speed.y < NEGATIVE_EDGE_ACCEL_MAX)
      group_core->group_speed.y = NEGATIVE_EDGE_ACCEL_MAX;
    }
// right
    if (fixed_to_block(notional_x + pr_nshape->max_length) != pr->test_block_position.x)
    {
     pr->hit_edge_this_cycle = 1;
     group_core->group_hit_edge_this_cycle = 1;
     group_core->group_speed.x -= EDGE_ACCEL;
     if (group_core->group_speed.x < NEGATIVE_EDGE_ACCEL_MAX)
      group_core->group_speed.x = NEGATIVE_EDGE_ACCEL_MAX;
    }
    break;
   }
  }

	}

 return 0;

}

/*
function that moves a group to a new position.
requires group test position values to have been filled in.
*/
static void move_group(struct core_struct* group_core)
{

  group_core->group_centre_of_mass.x = group_core->group_test_centre_of_mass.x;
  group_core->group_centre_of_mass.y = group_core->group_test_centre_of_mass.y;
  group_core->group_angle = group_core->group_test_angle;

  int i;
  struct proc_struct* pr;

  for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
		{

			if (group_core->group_member[i].exists == 0)
				continue;

			pr = &w.proc[group_core->group_member[i].index];

   pr->speed.x = pr->test_position.x - pr->position.x;
   pr->speed.y = pr->test_position.y - pr->position.y;

   pr->old_position = pr->position;
   pr->old_angle = pr->angle;

   pr->position.x = pr->test_position.x;
   pr->position.y = pr->test_position.y;

   pr->angle = pr->test_angle;

   pr->block_position.x = pr->test_block_position.x;
   pr->block_position.y = pr->test_block_position.y;

		}

		group_core->core_position = w.proc[group_core->process_index].position;

//fprintf(stdout, "\nmove_group2: %f to %f", al_fixtof(group_core->group_angle), al_fixtof(group_core->group_test_angle));

}




