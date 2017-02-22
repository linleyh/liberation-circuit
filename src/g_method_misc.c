#include <allegro5/allegro.h>

#include <stdio.h>
#include <math.h>

#include "m_config.h"


#include "g_header.h"

#include "c_header.h"
#include "e_slider.h"
#include "e_header.h"
#include "e_editor.h"
#include "e_log.h"

#include "g_world.h"
#include "g_misc.h"
#include "g_proc.h"
#include "g_packet.h"
#include "g_motion.h"
#include "g_method_misc.h"
#include "g_cloud.h"
#include "m_globvars.h"
#include "m_maths.h"
#include "t_template.h"
#include "i_error.h"
#include "i_console.h"
#include "g_shapes.h"
#include "v_interp.h"

/*

This file contains some method functions that might be used by any kind of program.

*/




extern struct view_struct view; // TO DO: think about putting a pointer to this in the worldstruct instead of externing it

extern unsigned char nshape_collision_mask [NSHAPES] [COLLISION_MASK_SIZE] [COLLISION_MASK_SIZE];

extern struct control_struct control; // defined in i_input.c. Used here for client process methods.
extern struct vmstate_struct vmstate;


//static void scan_block(int block_x, int block_y, al_fixed scan_x, al_fixed scan_y, int range, struct core_struct* ignore_core);



// returns: 0 success, 1 invalid shape, 2 invalid size
int check_proc_shape_size(int proc_shape, int proc_size)
{

 if (proc_shape < 0 || proc_shape >= NSHAPES)
 {
//  fprintf(stdout, "\n * check_proc_shape_size failed: shape %i (max %i)", proc_shape, SHAPES - 1);
  return 1; // invalid shape
 }

 return 0;

}



// returns index of proc if collision, -1 if not
// x/y should be a point in the map (but okay if outside; this is bounds-checked), in pixels
// minimum_collision_level should be 1 to detect collision with interface, 2 to ignore interface
int check_point_collision(al_fixed x, al_fixed y, int minimum_collision_level)
{

 int i, j;
 int x_block = fixed_to_block(x);
 int y_block = fixed_to_block(y);
 int bx, by;
 struct block_struct* bl;
 struct proc_struct* check_proc;
// al_fixed dist;
 al_fixed angle;
 al_fixed angle_diff;
 unsigned int mask_x; // these are ints as they are used as array indices below
 unsigned int mask_y;

// check collisions:
  for (i = -1; i < 2; i ++)
  {
   bx = x_block + i;
   if (bx < 0 || bx >= w.blocks.x)
    continue;
   for (j = -1; j < 2; j ++)
   {
    by = y_block + j;
    if (by < 0 || by >= w.blocks.y)
     continue;

    bl = &w.block [bx] [by];

    if (bl->tag != w.blocktag)
     continue;

    check_proc = bl->blocklist_down;

    while(check_proc != NULL)
    {
     if (check_proc->exists)
					{
						al_fixed oct_distance = distance_oct_xyxy(x, y, check_proc->position.x, check_proc->position.y);
//      && check_proc->position.x + check_proc->nshape_ptr->max_length > x
//      && check_proc->position.x - check_proc->nshape_ptr->max_length < x
//      && check_proc->position.y + check_proc->nshape_ptr->max_length > y
//      && check_proc->position.y - check_proc->nshape_ptr->max_length < y)
      if (oct_distance < check_proc->nshape_ptr->max_length)
      {

//       dist = distance(y - check_proc->position.y, x - check_proc->position.x);

       angle = get_angle(y - check_proc->position.y, x - check_proc->position.x);
       angle_diff = angle - check_proc->angle;

       mask_x = MASK_CENTRE + al_fixtoi(fixed_xpart(angle_diff, oct_distance));
       mask_x >>= COLLISION_MASK_BITSHIFT;
       mask_y = MASK_CENTRE + al_fixtoi(fixed_ypart(angle_diff, oct_distance));
       mask_y >>= COLLISION_MASK_BITSHIFT;

       if (mask_x < COLLISION_MASK_SIZE
        && mask_y < COLLISION_MASK_SIZE)
        {
         if (nshape_collision_mask [check_proc->shape] [mask_x] [mask_y] >= minimum_collision_level)
         {
          return check_proc->index;
         }
        }

      } // end bounding box check
					} // end if check_proc exists
     check_proc = check_proc->blocklist_down;
    }; // end while
/*
// first do a bounding box
     if (check_proc->exists
      && check_proc->position.x + check_proc->nshape_ptr->max_length > x
      && check_proc->position.x - check_proc->nshape_ptr->max_length < x
      && check_proc->position.y + check_proc->nshape_ptr->max_length > y
      && check_proc->position.y - check_proc->nshape_ptr->max_length < y)
      {

       dist = distance(y - check_proc->position.y, x - check_proc->position.x);

       angle = get_angle(y - check_proc->position.y, x - check_proc->position.x);
       angle_diff = angle - check_proc->angle;

       mask_x = MASK_CENTRE + al_fixtoi(fixed_xpart(angle_diff, dist));
       mask_x >>= COLLISION_MASK_BITSHIFT;
       mask_y = MASK_CENTRE + al_fixtoi(fixed_ypart(angle_diff, dist));
       mask_y >>= COLLISION_MASK_BITSHIFT;

       if (mask_x < COLLISION_MASK_SIZE
        && mask_y < COLLISION_MASK_SIZE)
        {
         if (nshape_collision_mask [check_proc->shape] [mask_x] [mask_y] >= minimum_collision_level)
         {
          return check_proc->index;
         }
        }

      } // end bounding box check

     check_proc = check_proc->blocklist_down;
    }; // end while
*/

   }
  }

 return -1;

}


// This function is similar to check_point_collision, but instead of checking a single point precisely it checks an area (currently a square the size of a block)
//  and returns the nearest proc to the centre (measuring from the proc's centre, not its closest point)
// returns index of proc if collision, -1 if not
// x/y should be a point in the map (but okay if outside), in grain units
int check_fuzzy_point_collision(al_fixed x, al_fixed y)
{

 int i, j;
 int x_block = fixed_to_block(x);
 int y_block = fixed_to_block(y);
 int bx, by;
 struct block_struct* bl;
 struct proc_struct* check_proc;
// al_fixed dist;
 al_fixed closest_dist = al_itofix(60);
 int closest_proc_index = -1;

// check collisions:
  for (i = -1; i < 2; i ++)
  {
   bx = x_block + i;
   if (bx < 0 || bx >= w.blocks.x)
    continue;
   for (j = -1; j < 2; j ++)
   {
    by = y_block + j;
    if (by < 0 || by >= w.blocks.y)
     continue;

    bl = &w.block [bx] [by];

    if (bl->tag != w.blocktag)
     continue;

    check_proc = bl->blocklist_down;

    while(check_proc != NULL)
    {
/*// first do a bounding box
     if (check_proc->exists
      && check_proc->position.x + BLOCK_SIZE_FIXED > x
      && check_proc->position.x - BLOCK_SIZE_FIXED < x
      && check_proc->position.y + BLOCK_SIZE_FIXED > y
      && check_proc->position.y - BLOCK_SIZE_FIXED < y)
      {

       dist = distance(y - check_proc->position.y, x - check_proc->position.x);
*/
     if (check_proc->exists)
					{
						al_fixed oct_distance = distance_oct_xyxy(x, y, check_proc->position.x, check_proc->position.y);
//      && check_proc->position.x + check_proc->nshape_ptr->max_length > x
//      && check_proc->position.x - check_proc->nshape_ptr->max_length < x
//      && check_proc->position.y + check_proc->nshape_ptr->max_length > y
//      && check_proc->position.y - check_proc->nshape_ptr->max_length < y)
      if (oct_distance < check_proc->nshape_ptr->max_length + al_itofix(8))
      {

//       dist = distance(y - check_proc->position.y, x - check_proc->position.x);

       if (oct_distance < closest_dist)
       {
        closest_dist = oct_distance;
        closest_proc_index = check_proc->index;
       }

      } // end bounding box check
					} // end if check_proc->exists

     check_proc = check_proc->blocklist_down;
    }; // end while


   }
  }

 return closest_proc_index;

}



// returns index of proc if collision, -1 if not
// used for things like packet collisions with no friendly fire
// x/y should be a point in the map (but okay if outside; this is bounds-checked), in pixels
int check_point_collision_ignore_team(al_fixed x, al_fixed y, int ignore_player, int minimum_collision_level)
{

 int i, j;
 int x_block = fixed_to_block(x);
 int y_block = fixed_to_block(y);
 int bx, by;
 struct block_struct* bl;
 struct proc_struct* check_proc;
// al_fixed dist;
 al_fixed angle;
 al_fixed angle_diff;
 unsigned int mask_x; // these are ints as they are used as array indices below
 unsigned int mask_y;

// check collisions:
  for (i = -1; i < 2; i ++)
  {
   bx = x_block + i;
   if (bx < 0 || bx >= w.blocks.x)
    continue;
   for (j = -1; j < 2; j ++)
   {
    by = y_block + j;
    if (by < 0 || by >= w.blocks.y)
     continue;

    bl = &w.block [bx] [by];

    if (bl->tag != w.blocktag)
     continue;

    check_proc = bl->blocklist_down;

    while(check_proc != NULL)
    {
// do a bounding box and check player index
     if (check_proc->exists
      && check_proc->player_index != ignore_player)
     {
//      && check_proc->position.x + check_proc->nshape_ptr->max_length > x
//      && check_proc->position.x - check_proc->nshape_ptr->max_length < x
//      && check_proc->position.y + check_proc->nshape_ptr->max_length > y
//      && check_proc->position.y - check_proc->nshape_ptr->max_length < y)
      al_fixed oct_distance = distance_oct_xyxy(x, y, check_proc->position.x, check_proc->position.y);
      if (oct_distance < check_proc->nshape_ptr->max_length)
      {

//       dist = distance(y - check_proc->position.y, x - check_proc->position.x);

       angle = get_angle(y - check_proc->position.y, x - check_proc->position.x);
       angle_diff = angle - check_proc->angle;

       mask_x = MASK_CENTRE + al_fixtoi(fixed_xpart(angle_diff, oct_distance));
       mask_x >>= COLLISION_MASK_BITSHIFT;
       mask_y = MASK_CENTRE + al_fixtoi(fixed_ypart(angle_diff, oct_distance));
       mask_y >>= COLLISION_MASK_BITSHIFT;

       if (mask_x < COLLISION_MASK_SIZE
        && mask_y < COLLISION_MASK_SIZE)
        {
         if (nshape_collision_mask [check_proc->shape] [mask_x] [mask_y] >= minimum_collision_level)
         {
          return check_proc->index;
         }
        }

      } // end bounding box check
     } // end if exists && !ignore_player

     check_proc = check_proc->blocklist_down;
    }; // end while


   }
  }

 return -1;

}




/*
void print_player_template_name(int source_program_type, int source_index, int source_player, int template_number)
{

	int t = get_player_template_index(source_player, template_number);

	if (t == -1
		|| templ[t].contents.file_name [0] == '\0')
	{
  program_write_to_console("[unknown]", source_program_type, source_index, source_player);
		return; // failed - should print an error
	}

	program_write_to_console(templ[t].contents.file_name, source_program_type, source_index, source_player);

}

*/



// This function creates an explosion for proc pr. It doesn't destroy the proc.
void new_proc_fail_cloud(al_fixed x, al_fixed y, int angle, int shape, int size, int player_owner)
{

 struct cloud_struct* cl = new_cloud(CLOUD_FAILED_NEW, 32, x, y);

 if (cl != NULL)
 {
  cl->lifetime = 22;
  cl->angle = angle;
  cl->colour = player_owner;

  cl->data [0] = shape;
  cl->data [1] = size;

 }

}

char method_error_string [120];

void	print_method_error(const char* error_message, int values, int value1)
{

	if (values)
	 sprintf(method_error_string, "\nMethod error [%s:%i] at line %i (bcode %i).", error_message, value1, (int) vmstate.bcode->src_line[vmstate.bcode_pos] + 1, vmstate.bcode_pos);
	  else
	   sprintf(method_error_string, "\nMethod error [%s] at line %i (bcode %i).", error_message, (int) vmstate.bcode->src_line[vmstate.bcode_pos] + 1, vmstate.bcode_pos);

// console_newline(CONSOLE_GENERAL, PRINT_COL_LRED);
	write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LRED, vmstate.core->index, vmstate.core->created_timestamp, method_error_string);

}

// assumes method_error_string already set up with error message
void	print_method_error_string(void)
{

// console_newline(CONSOLE_GENERAL, PRINT_COL_LRED);
	write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LRED, vmstate.core->index, vmstate.core->created_timestamp, method_error_string);

}


