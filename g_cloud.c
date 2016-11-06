#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>


#include <stdio.h>
#include <math.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"

#include "g_proc.h"

#include "g_misc.h"
#include "m_maths.h"
#include "g_world.h"

#include "g_cloud.h"


int new_cloud_index;

void init_clouds(void)
{

 int c;

 for (c = 0; c < w.max_clouds; c++)
 {
  w.cloud[c].created_timestamp = 0;
  w.cloud[c].lifetime = 0;
  w.cloud[c].destruction_timestamp = 0;
  w.cloud[c].index = c;
 }

 new_cloud_index = 0;

}

// returns index of new packet on success, -1 on failure
struct cloud_struct* new_cloud(int type, int cloud_lifetime, al_fixed x, al_fixed y)
{

 struct cloud_struct* cl;

 int total_cloud_counter = 0;

 while(TRUE)
	{
		new_cloud_index = (new_cloud_index + 1) & CLOUDS_MASK;
		if (w.world_time >= w.cloud[new_cloud_index].destruction_timestamp)
			break; // this should always be true unless there are lots of long-lived clouds in the game
		total_cloud_counter ++;
		if (total_cloud_counter >= CLOUDS)
			return NULL;
	};

 cl = &w.cloud[new_cloud_index];

 cl->created_timestamp = w.world_time;
 cl->destruction_timestamp = w.world_time + cloud_lifetime;
 cl->lifetime = cloud_lifetime;
 cl->type = type;
 cl->position.x = x;
 cl->position.y = y;
 cl->display_size_x1 = -50; // radius (right-angled) in pixels at 1 zoom. Note that these are floats.
 cl->display_size_y1 = -50;
 cl->display_size_x2 = 50;
 cl->display_size_y2 = 50;

 return cl;

}


/*

Need to simplify clouds!

Should be able to:
 - give each cloud a creation time
 - give each cloud a destruction time
 - if (world_time > destruction_time) cloud is not displayed, and can be re-used.
 - this should remove the need to manage clouds each tick.

*/

void run_clouds(void)
{

/*
 int c;
 struct cloud_struct* cl;

 for (c = 0; c < w.max_clouds; c++)
 {
  if (!w.cloud[c].exists)
   continue;
  cl = &w.cloud[c];
// see if its time has run out:
  if (cl->created_timestamp + cl->lifetime < w.world_time)
  {
   destroy_cloud(cl);
   continue;
  }

  switch(cl->type)
  {
   case CLOUD_PROC_EXPLODE_SMALL:
    cl->speed.x *= 950;
    cl->speed.x /= 1000;
    cl->speed.y *= 950;
    cl->speed.y /= 1000;
    cl->angle += int_angle_to_fixed(cl->data [2]);
    if (abs(cl->speed.x) + abs(cl->speed.y) < al_itofix(1)
     || cl->position.x <= (BLOCK_SIZE_FIXED * 2)
     || cl->position.y <= (BLOCK_SIZE_FIXED * 2)
     || cl->position.x >= w.fixed_size.x - (BLOCK_SIZE_FIXED * 2)
     || cl->position.y >= w.fixed_size.y - (BLOCK_SIZE_FIXED * 2))
    {
     cl->type = CLOUD_PROC_EXPLODE_SMALL2;
     cl->lifetime = 30;
     cl->speed.x = 0;
     cl->speed.y = 0;
     disrupt_single_block_node(cl->position.x, cl->position.y, cl->data [4], 5);
    }
    break;
   case CLOUD_YIELD_LINE:
// one end of a yield line is the centre of a proc, so we need to make sure the proc still exists:
    if (w.proc[cl->data[0]].exists <= 0)
    {
     cl->exists = 0;
     // it doesn't really matter if some of the cloud's values are changed later in this loop
    }
    break;
  }

  cl->position.x += cl->speed.x;
  cl->position.y += cl->speed.y;

 }
*/

}





