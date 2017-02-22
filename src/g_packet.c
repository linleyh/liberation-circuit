#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>


#include <stdio.h>
#include <math.h>

#include "m_config.h"
#include "m_globvars.h"

#include "g_header.h"

#include "g_proc.h"
#include "g_cloud.h"
#include "g_world.h"

#include "g_misc.h"
#include "m_maths.h"
#include "g_shapes.h"

int check_packet_collision(struct packet_struct* pack, struct block_struct* bl, al_fixed x, al_fixed y);
void destroy_packet(struct packet_struct* pack);
void packet_explodes(struct packet_struct* pk, struct proc_struct* pr_hit);

extern unsigned char nshape_collision_mask [NSHAPES] [COLLISION_MASK_SIZE] [COLLISION_MASK_SIZE];

void init_packets(void)
{

 int pk;

 for (pk = 0; pk < w.max_packets; pk++)
 {
  w.packet[pk].exists = 0;
  w.packet[pk].index = pk;
//  w.packet[pk].source_proc = -1;
 }

}

// returns index of new packet on success, -1 on failure
int new_packet(int type, int player_index, int source_core_index, timestamp source_core_created, al_fixed x, al_fixed y)
{

 int pk;
 struct packet_struct* pack;

 for (pk = 0; pk < w.max_packets; pk++)
 {
  if (!w.packet[pk].exists)
   break;
 }

 if (pk == w.max_packets)
  return -1;
 pack = &w.packet[pk];

 pack->exists = 1;
 pack->player_index = player_index;
 pack->source_core_index = source_core_index;
 pack->source_core_timestamp = source_core_created;
 pack->type = type;
 pack->created_timestamp = w.world_time;
 pack->position.x = x;
 pack->position.y = y;
 pack->block_position.x = fixed_to_block(x);
 pack->block_position.y = fixed_to_block(y);

 pack->prand_seed = w.world_time + x + y; // can be anything as long as it's just random enough

 pack->collision_size = 0;

 return pk;

}



void run_packets(void)
{

 int pk;
 struct packet_struct* pack;
 int i, j, bx, by;
 int proc_hit, finished;

 for (pk = 0; pk < w.max_packets; pk++)
 {
  if (!w.packet[pk].exists)
   continue;
  pack = &w.packet[pk];
// see if its time has run out:
  if (pack->created_timestamp + pack->lifetime < w.world_time)
  {
   packet_explodes(pack, NULL);
   destroy_packet(pack);
   continue;
  }

  pack->position.x += pack->speed.x;
  pack->position.y += pack->speed.y;

  pack->block_position.x = fixed_to_block(pack->position.x);
  pack->block_position.y = fixed_to_block(pack->position.y);

  if (w.block [pack->block_position.x] [pack->block_position.y].block_type == BLOCK_SOLID)
  {
   packet_explodes(pack, NULL);
   destroy_packet(pack);
   continue;
  }


  switch(pack->type)
  {
/*
		 case PACKET_TYPE_SPIKE1:
// SPIKE1 - just launched
		 	if (pack->created_timestamp + pack->lifetime == w.world_time)
		 	{
					pack->type = PACKET_TYPE_SPIKE2;
					pack->lifetime = 64;
		 	}
			 break;
		 case PACKET_TYPE_SPIKE2:
// SPIKE2 - stopping and turning
    pack->speed.x = al_fixmul(pack->speed.x, al_itofix(1000) / 1024);
    pack->speed.y = al_fixmul(pack->speed.y, al_itofix(1000) / 1024);
		 	if (pack->created_timestamp + pack->lifetime == w.world_time)
		 	{
					pack->type = PACKET_TYPE_SPIKE3;
					pack->lifetime += 32;
		 	}
			 break;
		 case PACKET_TYPE_SPIKE3:
// SPIKE2 - stopping and turning
    pack->speed.x += fixed_xpart(pack->angle, al_itofix(1) / 16);
    pack->speed.y += fixed_ypart(pack->angle, al_itofix(1) / 16);
		 	if (pack->created_timestamp + pack->lifetime == w.world_time)
		 	{
					pack->type = PACKET_TYPE_SPIKE4;
					pack->lifetime += 128;
		 	}
			 break;
			case PACKET_TYPE_SPIKE4:
				break;
*/

		 case PACKET_TYPE_SPIKE1:
//		 	{
// SPIKE1 - just launched. turning
//  assume that lifetime has been worked out so it will end when packet is pointing right way

    pack->speed.x += fixed_xpart(pack->angle, al_itofix(1) / 4);
    pack->speed.y += fixed_ypart(pack->angle, al_itofix(1) / 4);

    pack->speed.x = al_fixmul(pack->speed.x, al_itofix(1000) / 1024);
    pack->speed.y = al_fixmul(pack->speed.y, al_itofix(1000) / 1024);


    pack->angle += pack->fixed_status; // dodgy because status is int and angle is al_fixed, but can get away with it

				pack->position2.x = pack->position.x;
				pack->position2.y = pack->position.y;


		 	if (pack->created_timestamp + pack->lifetime == w.world_time)
		 	{
					pack->type = PACKET_TYPE_SPIKE2;
					pack->lifetime = 64; // remember that it has already existed for part of this
					pack->position2.x = pack->position.x;
					pack->position2.y = pack->position.y;

		 	}


    if (pack->status != -1
					|| pack->created_timestamp == w.world_time)
    {
//     struct cloud_struct* cl = new_cloud(CLOUD_SPIKE_TRAIL, 16, pack->position.x - fixed_xpart(pack->angle, al_itofix(10)), pack->position.y - fixed_ypart(pack->angle, al_itofix(10)));
     struct cloud_struct* cl = new_cloud(CLOUD_SPIKE_TRAIL, 16, pack->position.x, pack->position.y);

     if (cl != NULL)
     {
     	if (pack->status != -1)
						{
						 cl->position2.x = w.cloud[pack->status].position.x - fixed_xpart(pack->angle, al_itofix(10));
						 cl->position2.y = w.cloud[pack->status].position.y - fixed_ypart(pack->angle, al_itofix(10));
       w.cloud[pack->status].data [1] = 0; // cloud is no longer first cloud in line
						}
						 else
							{
						  cl->position2.x = pack->position.x - fixed_xpart(pack->angle, al_itofix(10));
						  cl->position2.y = pack->position.y - fixed_ypart(pack->angle, al_itofix(10));
							}
						cl->angle = pack->angle; // used to draw cloud
      cl->colour = pack->colour;
      cl->data [0] = pack->status; // index of previous cloud (may be -1)
      cl->data [1] = 1; // cloud is first cloud
      cl->data [2] = 0; // no side-spikes
     	pack->status = cl->index;
     }
      else
						 pack->status = -1;
    }
			 break;
		 case PACKET_TYPE_SPIKE2:
// SPIKE2 - just accelerating


		 	if (pack->created_timestamp + pack->lifetime == w.world_time)
		 	{
					pack->type = PACKET_TYPE_SPIKE3;
					pack->lifetime += 192;
		 	}

//    pack->speed.x = al_fixmul(pack->speed.x, al_itofix(1000) / 1024);
//    pack->speed.y = al_fixmul(pack->speed.y, al_itofix(1000) / 1024);
//    break;
// fall through...
		 case PACKET_TYPE_SPIKE3:

    pack->speed.x += fixed_xpart(pack->angle, al_itofix(1) / 4);
    pack->speed.y += fixed_ypart(pack->angle, al_itofix(1) / 4);

    pack->speed.x = al_fixmul(pack->speed.x, al_itofix(1000) / 1024);
    pack->speed.y = al_fixmul(pack->speed.y, al_itofix(1000) / 1024);

//    if (pack->type == PACKET_TYPE_SPIKE3
//					&& ((w.world_time - pack->created_timestamp) & 3) == 3)
//						pack->damage ++;

    if (pack->type == PACKET_TYPE_SPIKE3)
				{
					if (w.world_time - pack->created_timestamp == 90)
						pack->damage = SPIKE_STAGE2_DAMAGE;
					if (w.world_time - pack->created_timestamp == 160)
						pack->damage = SPIKE_MAX_DAMAGE;
				}


    if (pack->status != -1)
					{
//     struct cloud_struct* cl = new_cloud(CLOUD_SPIKE_TRAIL, 16, pack->position.x - fixed_xpart(pack->angle, al_itofix(10)), pack->position.y - fixed_ypart(pack->angle, al_itofix(10)));
      struct cloud_struct* cl = new_cloud(CLOUD_SPIKE_TRAIL, 16, pack->position.x, pack->position.y);

      if (cl != NULL)
      {
		 				cl->position2.x = w.cloud[pack->status].position.x - fixed_xpart(pack->angle, al_itofix(10));
				 		cl->position2.y = w.cloud[pack->status].position.y - fixed_ypart(pack->angle, al_itofix(10));
		 				cl->angle = pack->angle; // used to draw cloud
       w.cloud[pack->status].data [1] = 0; // cloud is no longer first cloud in line
       cl->colour = pack->colour;
       cl->data [0] = pack->status; // index of previous cloud
       cl->data [1] = 1; // cloud is first cloud
       cl->data [2] = (pack->damage == SPIKE_MAX_DAMAGE); // draw side-spikes
      	pack->status = cl->index;
      }
       else
						  pack->status = -1;
    }
			 break;



/*
		 case PACKET_TYPE_SURGE:

    if (pack->created_timestamp < w.world_time + 32)
				{
     pack->speed.x += fixed_xpart(pack->angle, al_itofix(1) / 16);
     pack->speed.y += fixed_ypart(pack->angle, al_itofix(1) / 16);
				}

//    pack->speed.x = al_fixmul(pack->speed.x, al_itofix(1000) / 1024);
//    pack->speed.y = al_fixmul(pack->speed.y, al_itofix(1000) / 1024);


    if (pack->status != -1
					|| pack->created_timestamp == w.world_time)
    {
//     struct cloud_struct* cl = new_cloud(CLOUD_SPIKE_TRAIL, 16, pack->position.x - fixed_xpart(pack->angle, al_itofix(10)), pack->position.y - fixed_ypart(pack->angle, al_itofix(10)));
     struct cloud_struct* cl = new_cloud(CLOUD_SURGE_TRAIL, 16, pack->position.x, pack->position.y);

     if (cl != NULL)
     {
     	if (pack->status != -1)
						{
						 cl->position2.x = w.cloud[pack->status].position.x - fixed_xpart(pack->angle, al_itofix(10));
						 cl->position2.y = w.cloud[pack->status].position.y - fixed_ypart(pack->angle, al_itofix(10));
       w.cloud[pack->status].data [1] = 0; // previous cloud is no longer first cloud in line
						}
						 else
							{
						  cl->position2.x = pack->position.x - fixed_xpart(pack->angle, al_itofix(10));
						  cl->position2.y = pack->position.y - fixed_ypart(pack->angle, al_itofix(10));
							}
//													cl->position2.x = w.cloud[pack->status].position.x - fixed_xpart(pack->angle, al_itofix(10));
//						cl->position2.y = w.cloud[pack->status].position.y - fixed_ypart(pack->angle, al_itofix(10));
						cl->angle = pack->angle; // used to draw cloud
//      w.cloud[pack->status].data [1] = 0; // cloud is no longer first cloud in line
      cl->colour = pack->colour;
      cl->data [0] = pack->status; // index of previous cloud
      cl->data [1] = 1; // cloud is first cloud
     	pack->status = cl->index;
     }
      else
						 pack->status = -1;
    }

			 break;
*/
/*
		 case PACKET_TYPE_SPIKE3:
		 	{
     struct cloud_struct* cl = new_cloud(CLOUD_SPIKE_TRAIL, 16, pack->position.x, pack->position.y);

     if (cl != NULL)
     {
      cl->colour = pack->colour;
      cl->position2.x = pack->position.x + pack->speed.x;
      cl->position2.y = pack->position.y + pack->speed.y;
      cl->data [1] = 16;
      cl->data [2] = 0;
     }
		 	}
		 	break;*/

/*

How will spike trails work?

 - packet creates cloud and sets one of its points
 - packet sets status to cloud index
 - next tick:
 - packet creates cloud and sets one of its points
	 - packet sets other point to location of previous cloud
	 - packet sets cl->data [0] of new cloud to index of previous cloud
	 - packet sets cl->data [1] of previous cloud to 0
	 - packet sets cl->data [1] of new cloud to 1


	- display:
	 - ignores spike lines with cl->data [1] of 0
	 - when it reaches line with cl->data [1] == 1, it draws all of them - stopping at the one with just 1 (0?) ticks to go

This approach probably means that the first clouds to be created need to start with very low lifetimes

*/


/*
		 case PACKET_TYPE_SPIKE1:
// SPIKE1 - just launched. turning and accelerating
//  assume that lifetime has been worked out so it will end when packet is pointing right way
//    fpr("\n angle %f + %f = ", al_fixtof(pack->angle), al_fixtof(pack->fixed_status));
    pack->angle += pack->fixed_status; // dodgy because status is int and angle is al_fixed, but can get away with it
//    fpr("%f", al_fixtof(pack->angle));
//    pack->speed.x += fixed_xpart(pack->angle, al_itofix(1) / 8);
//    pack->speed.y += fixed_ypart(pack->angle, al_itofix(1) / 8);

    pack->speed.x = al_fixmul(pack->speed.x, al_itofix(990) / 1024);
    pack->speed.y = al_fixmul(pack->speed.y, al_itofix(990) / 1024);

		 	if (pack->created_timestamp + pack->lifetime == w.world_time)
		 	{
					pack->type = PACKET_TYPE_SPIKE2;
					pack->lifetime = 128; // remember that it has already existed for part of this
					pack->position2.x = pack->position.x;
					pack->position2.y = pack->position.y;
					pack->speed.x = 0;
					pack->speed.y = 0;
		 	}
			 break;
		 case PACKET_TYPE_SPIKE2:
// SPIKE2 - just accelerating
    pack->speed.x += fixed_xpart(pack->angle, al_itofix(1) / 16);
    pack->speed.y += fixed_ypart(pack->angle, al_itofix(1) / 16);

//    pack->speed.x = al_fixmul(pack->speed.x, al_itofix(1000) / 1024);
//    pack->speed.y = al_fixmul(pack->speed.y, al_itofix(1000) / 1024);


		 	if (pack->created_timestamp + pack->lifetime == w.world_time)
		 	{
					pack->type = PACKET_TYPE_SPIKE3;
					pack->lifetime += 64;
		 	}
//			 break;
		 case PACKET_TYPE_SPIKE3:
		 	{
     struct cloud_struct* cl = new_cloud(CLOUD_SPIKE_TRAIL, 8, pack->position.x, pack->position.y);

     if (cl != NULL)
     {
      cl->colour = pack->colour;
      cl->position2.x = pack->position.x - fixed_xpart(pack->angle, al_fixtoi(10));
      cl->position2.y = pack->position.y - fixed_ypart(pack->angle, al_fixtoi(10));
      cl->data [1] = 16;
      cl->data [2] = 0;
// I think this is wrong (or at least badly optimised):
      cl->display_size_x1 = -20 - al_fixtof(abs(cl->position2.x - cl->position.x));
      cl->display_size_y1 = -20 - al_fixtof(abs(cl->position2.y - cl->position.y));
      cl->display_size_x2 = 20 + al_fixtof(abs(cl->position2.x - cl->position.x));
      cl->display_size_y2 = 20 + al_fixtof(abs(cl->position2.y - cl->position.y));
     }
		 	}

		 	{
     struct cloud_struct* cl = new_cloud(CLOUD_SPIKE_LINE, 1, pack->position2.x, pack->position2.y);

     if (cl != NULL)
     {
      cl->colour = pack->colour;
      cl->position2.x = pack->position.x - fixed_xpart(pack->angle, al_fixtoi(10));
      cl->position2.y = pack->position.y - fixed_ypart(pack->angle, al_fixtoi(10));
      cl->data [1] = 16;
      cl->data [2] = 0;
// I think this is wrong (or at least badly optimised):
      cl->display_size_x1 = -20 - al_fixtof(abs(cl->position2.x - cl->position.x));
      cl->display_size_y1 = -20 - al_fixtof(abs(cl->position2.y - cl->position.y));
      cl->display_size_x2 = 20 + al_fixtof(abs(cl->position2.x - cl->position.x));
      cl->display_size_y2 = 20 + al_fixtof(abs(cl->position2.y - cl->position.y));
     }

// SPIKE3 - nothing
		 	}
			 break;
*/


/*
New approach to spikes:
 - are launched from object at vertex base angle
 - turn towards target angle (number of ticks to turn is precalculated)
 - accelerate and sharpen while turning
 - then just keep accelerating

stages:
SPIKE1 - turns towards target angle, accelerates
SPIKE2 - has finished turning but continues to accel
SPIKE3 - has finished accelerating. Final stage
 - sharpness should be based on overall lifetime, regardless of type

How to do the trail?
 - if no drag, should be able to derive path from current location, speed and lifetime
 - otherwise, will need to chain clouds together. bleh



*/

// SPIKE3 - accelerating and elongating

// Spike4 - final stage

  }


// TO DO: can fix this.
  if (pack->source_proc != -1)
		{
			if (w.proc[pack->source_proc].exists)
				pack->source_proc = -1;
		}

// check collisions:
  finished = 0;
  for (i = -1; i < 2; i ++)
  {
   bx = pack->block_position.x + i;
   if (bx < 0 || bx >= w.blocks.x)
    continue;
   for (j = -1; j < 2; j ++)
   {
    by = pack->block_position.y + j;
    if (by < 0 || by >= w.blocks.y)
     continue;
    proc_hit = check_packet_collision(pack, &w.block [bx] [by], pack->position.x, pack->position.y);
    if (proc_hit != -1)
    {
    	w.proc[proc_hit].component_hit_time = w.world_time; // hit_pulse_time set (in apply_packet_damage_to_proc()) only if component not protected by interface
     w.proc[proc_hit].component_hit_source_index = pack->source_core_index;
     w.proc[proc_hit].component_hit_source_timestamp = pack->source_core_timestamp;
     apply_packet_damage_to_proc(&w.proc[proc_hit], pack->damage, pack->player_index, pack->source_core_index, pack->source_core_timestamp);
     packet_explodes(pack, &w.proc[proc_hit]);
     destroy_packet(pack);
     finished = 1;
     break;
    }
   }
   if (finished)
    break;
  }
  if (finished)
   continue;

//  pack->tail_count++;

// now put it on the blocklist:
// This works just like blocktags for procs - see g_motion.c
  if (w.block [pack->block_position.x] [pack->block_position.y].packet_tag != w.blocktag)
  {
   w.block [pack->block_position.x] [pack->block_position.y].packet_tag = w.blocktag;
   pack->blocklist_down = NULL;
   pack->blocklist_up = NULL;
   w.block [pack->block_position.x] [pack->block_position.y].packet_down = pack;
  }
   else
   {
// The block's packet_blocktag is up to date. So we put the new packet on top and set its downlink to the top packet of the block:
    pack->blocklist_down = w.block [pack->block_position.x] [pack->block_position.y].packet_down;
    pack->blocklist_up = NULL;
    w.block [pack->block_position.x] [pack->block_position.y].packet_down = pack;
// we also set the previous top proc's uplink to pack:
    if (pack->blocklist_down != NULL) // not sure that this is necessary.
     pack->blocklist_down->blocklist_up = pack;
   }

 }


}


// returns index of proc if collision, -1 if not
int check_packet_collision(struct packet_struct* pack, struct block_struct* bl, al_fixed x, al_fixed y)
{

 if (bl->tag != w.blocktag)
  return -1;

 int minimum_collision_mask_level; // is set to 1 if proc has interface, 2 otherwise (2 means ignore collision with interface)

//fpr("\n check packet_collision");


 struct proc_struct* check_proc;
 check_proc = bl->blocklist_down;
// static int collision_x, collision_y;

// int size_check;

 while(check_proc != NULL)
 {
/* 	fpr(" check_proc %i exists %i ts %i:%i from %i,%i to %i,%i packet %i,%i", check_proc->index,
							check_proc->exists,
							check_proc->player_index,
							pack->team_safe,
							al_fixtoi(check_proc->position.x - check_proc->nshape_ptr->max_length),
							al_fixtoi(check_proc->position.y - check_proc->nshape_ptr->max_length),
							al_fixtoi(check_proc->position.x + check_proc->nshape_ptr->max_length),
							al_fixtoi(check_proc->position.y + check_proc->nshape_ptr->max_length),
							al_fixtoi(x), al_fixtoi(y));*/

// first check team safety and do a bounding box
    if (check_proc->player_index != pack->team_safe
     && check_proc->exists
     && check_proc->position.x + check_proc->nshape_ptr->max_length > x
     && check_proc->position.x - check_proc->nshape_ptr->max_length < x
     && check_proc->position.y + check_proc->nshape_ptr->max_length > y
     && check_proc->position.y - check_proc->nshape_ptr->max_length < y)
     {

      al_fixed dist = distance(y - check_proc->position.y, x - check_proc->position.x);

      al_fixed angle = get_angle(y - check_proc->position.y, x - check_proc->position.x);
      al_fixed angle_diff = angle - check_proc->angle;

//      size_check = check_proc->size + pack->collision_size;

      unsigned int mask_x = MASK_CENTRE + al_fixtoi(fixed_xpart(angle_diff, dist));
      mask_x >>= COLLISION_MASK_BITSHIFT;
      unsigned int mask_y = MASK_CENTRE + al_fixtoi(fixed_ypart(angle_diff, dist));
      mask_y >>= COLLISION_MASK_BITSHIFT;

      minimum_collision_mask_level = 2;
//      if (check_proc->interface_object_present
//							&& check_proc->interface_on_process_set_on
      if (check_proc->interface_protects
							&& w.core[check_proc->core_index].interface_active)
								minimum_collision_mask_level = 1;

      if (mask_x < COLLISION_MASK_SIZE
       && mask_y < COLLISION_MASK_SIZE)
       {
        if (nshape_collision_mask [check_proc->shape] [mask_x] [mask_y] >= minimum_collision_mask_level)
        {
         return check_proc->index;
        }
       }

     }

  check_proc = check_proc->blocklist_down;
 };

 return -1;

}

// Call this when a packet explodes (because it hit something, or because it ran out of time)
// If pr_hit is NULL, the packet is treated as having missed
void packet_explodes(struct packet_struct* pk, struct proc_struct* pr_hit)
{

 struct cloud_struct* cl;

 switch(pk->type)
 {
/*
  case PACKET_TYPE_SURGE:
   if (pr_hit != NULL)
   {

    cl = new_cloud(CLOUD_SURGE_HIT, 32, pk->position.x, pk->position.y);

    if (cl != NULL)
    {
      cl->angle = get_angle(al_fixsub(pr_hit->position.y, pk->position.y), al_fixsub(pr_hit->position.x, pk->position.x)) + AFX_ANGLE_2;
      cl->colour = pk->colour;
    }
   }
    else
				{

     cl = new_cloud(CLOUD_SURGE_MISS, 32, pk->position.x, pk->position.y);

     if (cl != NULL)
     {
      cl->angle = pk->angle;
      cl->colour = pk->colour;
     }
				}
    pulse_block_node(pk->position.x, pk->position.y);

  	break;
*/
  case PACKET_TYPE_SPIKE3:
  	if (pk->damage == SPIKE_MAX_DAMAGE)
			{
				 if (pr_hit != NULL)
     {

      cl = new_cloud(CLOUD_SPIKE_HIT_AT_LONG_RANGE, 32, pk->position.x, pk->position.y);

      if (cl != NULL)
      {
        cl->angle = get_angle(al_fixsub(pr_hit->position.y, pk->position.y), al_fixsub(pr_hit->position.x, pk->position.x)) + AFX_ANGLE_2;
        cl->colour = pk->colour;
      }
     }
      else
				  {

       cl = new_cloud(CLOUD_SPIKE_MISS_AT_LONG_RANGE, 32, pk->position.x, pk->position.y);

       if (cl != NULL)
       {
        cl->angle = pk->angle;
        cl->colour = pk->colour;
       }
				  }
      pulse_block_node(pk->position.x, pk->position.y);
  	 break;

			}


  case PACKET_TYPE_SPIKE1:
  case PACKET_TYPE_SPIKE2:
//  case PACKET_TYPE_SPIKE4:
   if (pr_hit != NULL)
   {

    cl = new_cloud(CLOUD_SPIKE_HIT, 16, pk->position.x, pk->position.y);

    if (cl != NULL)
    {
      cl->angle = get_angle(al_fixsub(pr_hit->position.y, pk->position.y), al_fixsub(pr_hit->position.x, pk->position.x));
      cl->colour = pk->colour;
      cl->data [0] = 0;
      if (pk->damage > SPIKE_BASE_DAMAGE)
							cl->data [0] = pk->damage - SPIKE_BASE_DAMAGE;
//      cl->data [2] = w.world_time + pk->speed.x; // used as random seed
//      cl->data [0] = 5; // size of cloud (depends on packet type; set when packet created)
    }
   }
    else
				{

     cl = new_cloud(CLOUD_SPIKE_MISS, 16, pk->position.x, pk->position.y);

     if (cl != NULL)
     {
      cl->angle = pk->angle;
      cl->colour = pk->colour;
     }
				}
    pulse_block_node(pk->position.x, pk->position.y);

  	break;
/*
  case PACKET_TYPE_BURST:
   {

    if (pr_hit != NULL)
     cl = new_cloud(CLOUD_BURST_HIT, 48, pk->position.x, pk->position.y);
      else
       cl = new_cloud(CLOUD_BURST_MISS, 48, pk->position.x, pk->position.y);

    if (cl != NULL)
    {
      cl->angle = pk->angle;//get_angle(al_fixsub(pr_hit->position.y, pk->position.y), al_fixsub(pr_hit->position.x, pk->position.x));
      cl->colour = pk->colour;
      cl->speed = pk->speed;
      cl->data [0] = pk->index; // used as drand seed
      cl->data [1] = pk->created_timestamp;//w.world_time - pk->created_timestamp; - not ideal - timestamp is unsigned int. But this probably isn't going to overflow.
      cl->data [2] = w.world_time - pk->created_timestamp;//cl->data [1] % 3;
//      cl->data [2] = w.world_time + pk->speed.x; // used as random seed
//      cl->data [0] = 5; // size of cloud (depends on packet type; set when packet created)
    }
   }

  	break;
*/


  case PACKET_TYPE_BURST:
  case PACKET_TYPE_PULSE:
   if (pr_hit != NULL)
   {

//   	pk->status = 0;//8;//grand(4);
//    cl = new_cloud(CLOUD_PACKET_HIT, 14 + (pk->status), pk->position.x, pk->position.y);
//    cl = new_cloud(CLOUD_PACKET_HIT, 16 + (pk->status * 6), pk->position.x, pk->position.y);
     cl = new_cloud(CLOUD_PACKET_HIT, 16 + pk->status * 12, pk->position.x, pk->position.y);

    if (cl != NULL)
    {
//      cl->lifetime = 16;

      cl->angle = pk->angle;// get_angle(al_fixsub(pr_hit->position.y, pk->position.y), al_fixsub(pr_hit->position.x, pk->position.x));
      cl->colour = pk->colour;
      cl->speed = pk->speed;
      cl->data [0] = pk->status; // size of cloud (depends on packet type; set when packet created
      sancheck(pk->status, 0, 3, "pk->status");
      switch(pk->status)
      {
      	case 0:
        cl->data [1] = 16; break; // size of cloud (depends on packet type; set when packet created)
      	case 1:
        cl->data [1] = 28; break; // size of cloud (depends on packet type; set when packet created)
      	case 2:
        cl->data [1] = 36; break; // size of cloud (depends on packet type; set when packet created)
      }
      cl->data [2] = w.world_time + pk->speed.x; // used as random seed
      cl->data [3] = pk->index; // used as random seed
      cl->data [4] = w.world_time - pk->created_timestamp; // time between packet creation and cloud creation
      cl->data [5] = (pk->type == PACKET_TYPE_BURST);
/*    	 if (pr_hit->virtual_method != -1
							&& pr_hit->method[pr_hit->virtual_method].data [MDATA_PR_VIRTUAL_STATE] > 0)
						{
								cl->type = CLOUD_PACKET_HIT_VIRTUAL;
								cl->colour = pr_hit->player_index;

								cl->position.x -= pk->speed.x * 4;
								cl->position.y -= pk->speed.y * 4;
						}*/
    }
   }
    else
    {
//     cl = new_cloud(CLOUD_PACKET_MISS, 8 + (pk->status * 2), pk->position.x, pk->position.y);
     cl = new_cloud(CLOUD_PACKET_MISS, 12 + pk->status * 4, pk->position.x, pk->position.y);



     if (cl != NULL)
     {
      cl->angle = pk->angle;
      cl->colour = pk->colour;
      cl->speed = pk->speed;
      cl->data [0] = pk->status; // size of cloud (depends on packet type; set when packet created)
      cl->data [1] = 12 + (pk->status * 4); // size of cloud (depends on packet type; set when packet created)
      cl->data [2] = w.world_time + pk->speed.x; // used as random seed
      cl->data [3] = pk->index; // used as random seed
      cl->data [4] = pk->created_timestamp; // used as random seed
//      cl->data [5] = cl->lifetime; // used as random seed
/*
//      cl->lifetime = 16;
      cl->angle = pk->angle;
      cl->colour = pk->colour;
      cl->data [2] = w.world_time + pk->speed.x; // used as random seed
      cl->data [0] = pk->status; // size of cloud (depends on packet type; set when packet created)
      cl->position2.x = pk->speed.x;// / 2;
      cl->position2.y = pk->speed.y;// / 2;*/
//      cl->data [1] = pk->size * 2;
//      cl->data [2] = pk->size;
     }
    }
//    disrupt_block_nodes(pk->x, pk->y, 5);
//    disrupt_single_block_node(pk->position.x, pk->position.y, pk->player_index, 5);
    pulse_block_node(pk->position.x, pk->position.y);
   break;

/*
  case PACKET_TYPE_SURGE:
   if (pr_hit != NULL)
   {

     cl = new_cloud(CLOUD_SURGE_HIT, 16 + pk->status * 12, pk->position.x, pk->position.y);

    if (cl != NULL)
    {

      cl->angle = pk->angle;// get_angle(al_fixsub(pr_hit->position.y, pk->position.y), al_fixsub(pr_hit->position.x, pk->position.x));
      cl->colour = pk->colour;
      cl->speed = pk->speed;
      cl->data [0] = pk->status; // size of cloud (depends on packet type; set when packet created)
      cl->data [1] = 16 + (pk->status * 6); // size of cloud (depends on packet type; set when packet created)
      cl->data [2] = w.world_time + pk->speed.x; // used as random seed
      cl->data [3] = pk->index; // used as random seed
      cl->data [4] = w.world_time - pk->created_timestamp; // time between packet creation and cloud creation
//      cl->data [5] = (pk->type == PACKET_TYPE_BURST);
    }
   }
    else
    {
     cl = new_cloud(CLOUD_SURGE_MISS, 12 + pk->status * 4, pk->position.x, pk->position.y);

     if (cl != NULL)
     {
      cl->angle = pk->angle;
      cl->colour = pk->colour;
      cl->speed = pk->speed;
      cl->data [0] = pk->status; // size of cloud (depends on packet type; set when packet created)
      cl->data [1] = 16 + (pk->status * 6); // size of cloud (depends on packet type; set when packet created)
      cl->data [2] = w.world_time + pk->speed.x; // used as random seed
      cl->data [3] = pk->index; // used as random seed
      cl->data [4] = pk->created_timestamp; // used as random seed
     }
    }
    pulse_block_node(pk->position.x, pk->position.y);
   break;
*/

  case PACKET_TYPE_ULTRA:
   if (pr_hit != NULL)
   {

     cl = new_cloud(CLOUD_ULTRA_HIT, 16 + 3 * 12, pk->position.x, pk->position.y);

    if (cl != NULL)
    {

      cl->angle = pk->angle;// get_angle(al_fixsub(pr_hit->position.y, pk->position.y), al_fixsub(pr_hit->position.x, pk->position.x));
      cl->colour = pk->colour;
      cl->speed = pk->speed;
      cl->data [0] = pk->status; // size of cloud (depends on packet type; set when packet created)
      cl->data [1] = w.world_time - pk->created_timestamp;
      cl->data [2] = w.world_time + pk->speed.x; // used as random seed
      cl->data [3] = pk->index; // used as random seed
      cl->data [4] = w.world_time - pk->created_timestamp; // time between packet creation and cloud creation
//      cl->data [5] = (pk->type == PACKET_TYPE_BURST);
    }
   }
    else
    {
     cl = new_cloud(CLOUD_ULTRA_MISS, 16 + 3 * 12, pk->position.x, pk->position.y);
//     cl = new_cloud(CLOUD_ULTRA_HIT, 16 + pk->status * 12, pk->position.x, pk->position.y);

     if (cl != NULL)
     {
      cl->angle = pk->angle;
      cl->colour = pk->colour;
      cl->speed = pk->speed;
      cl->data [0] = pk->status; // size of cloud (depends on packet type; set when packet created)
      cl->data [1] = w.world_time - pk->created_timestamp;
      cl->data [2] = w.world_time + pk->speed.x; // used as random seed
      cl->data [3] = pk->index; // used as random seed
      cl->data [4] = pk->created_timestamp; // used as random seed
     }
    }
    pulse_block_node(pk->position.x, pk->position.y);
   break;


 } // end switch

} // end packet_explodes



void destroy_packet(struct packet_struct* pack)
{

 pack->exists = 0;

}







