
#process "builder"

class auto_retro;
class auto_move;
class auto_att_back;
class auto_att_main;
class auto_att_fwd;

core_hex_B, 0, 
  {object_downlink, 0, 
    {component_tri, // component 1
      {object_uplink, 0},
      {object_downlink, -939, 
        {component_cap, // component 2
          {object_none, 0},
          {object_uplink, 0},
          {object_none, 0},
          {object_burst_xl:auto_att_main, 935},
        }
      },
      {object_downlink, 886, 
        {component_cap, // component 3
          {object_burst_xl:auto_att_main, -882},
          {object_none, 0},
          {object_uplink, 0},
          {object_none, 0},
        }
      },
    }
  },
  {object_downlink, 240, 
    {component_cap, // component 4
      {object_pulse_l:auto_att_back, -604},
      {object_uplink, 0},
      {object_build, 0},
      {object_burst_xl:auto_att_main, 307},
    }
  },
  {object_downlink, 903, 
    {component_long5, // component 5
      {object_interface, 0},
      {object_interface, 0},
      {object_interface, 0},
      {object_downlink, 418, 
        {component_peak, // component 6
          {object_uplink, 0},
          {object_downlink, 268, 
            {component_cap, // component 7
              {object_uplink, 0},
              {object_move:auto_move:auto_retro, 2048},
              {object_move:auto_move:auto_retro, 831},
              {object_move:auto_move:auto_retro, -184},
            }
          },
          {object_downlink, -873, 
            {component_cap, // component 8
              {object_uplink, 0},
              {object_move:auto_move, 2048},
              {object_move:auto_move, 959},
              {object_move:auto_move, -55},
            }
          },
          {object_move:auto_move, 1747},
          {object_move:auto_move, -212},
        }
      },
      {object_uplink, 0},
    }
  },
  {object_repair, 0},
  {object_downlink, -903, 
    {component_long5, // component 9
      {object_interface, 0},
      {object_interface, 0},
      {object_interface, 0},
      {object_uplink, 0},
      {object_downlink, -415, 
        {component_peak, // component 10
          {object_uplink, 0},
          {object_downlink, 951, 
            {component_cap, // component 11
              {object_move:auto_move, -22},
              {object_move:auto_move, -1036},
              {object_move:auto_move, -2048},
              {object_uplink, 0},
            }
          },
          {object_downlink, -262, 
            {component_cap, // component 12
              {object_move:auto_move:auto_retro, 184},
              {object_move:auto_move:auto_retro, -831},
              {object_move:auto_move:auto_retro, -2048},
              {object_uplink, 0},
            }
          },
          {object_move:auto_move, 209},
          {object_move:auto_move, -1750},
        }
      },
    }
  },
  {object_downlink, -247, 
    {component_cap, // component 13
      {object_burst_xl:auto_att_main, -300},
      {object_slice:auto_att_fwd, -333},
      {object_uplink, 0},
      {object_pulse_l:auto_att_back, 604},
    }
  }

#code


enum
{
  MODE_SEEK_WELL, // process is wandering randomly looking for a well
  MODE_GUARD, // process is waiting at the main base waiting for a harvester to find a data well
  MODE_BUILD_BASE, // process is trying to build a base near a data well
  MODE_ATTACK, // attacking mobile target
//  MODE_SIEGE_BOMBARD, // attacking static target while staying out of range
//  MODE_SIEGE_APPROACH, // checking whether siege target still exists
  MODES
};

// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
TARGET_PARENT, // parent process automatically stored in address 0
TARGET_MAIN, // main target to attack
TARGET_FWD,
TARGET_BACK,
TARGET_LEFT,
TARGET_RIGHT,
};

enum
{
TEMPLATE_BASE, // assumes that a base process is in template 0
TEMPLATE_FIREBASE, // assumes that a firebase process is in template 1
TEMPLATE_BUILDER,
TEMPLATE_SPIKEBASE,

TEMPLATE_OUTPOST,
TEMPLATE_BUILDER2,

};

// Variable declaration and initialisation
//  (note that declaration and initialisation cannot be combined)
//  (also, variables retain their values between execution cycles)
int core_x, core_y; // location of core
core_x = get_core_x(); // location is updated each cycle
core_y = get_core_y();
int angle; // direction process is pointing
 // angles are in integer degrees from 0 to 8192, with 0 being right,
 // 2048 down, 4096 left and 6144 up.
angle = get_core_angle(); // angle is updated each cycle

int mode; // what is the process doing? (should be one of the MODE enums)
int scan_result;
//int firebase_count; // increases while under attack. Builds a firebase when high enough.
int firebase_angle, firebase_dist;
int firebase_template;
int bombard_count; // time spent bombarding static target while out of sight range

int broadcast_count;

int move_x, move_y; // destination
int attack_x, attack_y;

// Harvester variables
int data_well_x, data_well_y; // location of data well
int ignore_well_x, ignore_well_y; // ignore data well with a friendly allocator near it
int build_angle;

int initialised;

if (initialised == 0)
{
 initialised = 1;
 listen_channel(1); // processes in this mission use channel 1 to broadcast target-found messages
 attack_mode(1);
 mode = MODE_SEEK_WELL;
/*
 broadcast(1000, // range of broadcast, in pixels
           5, // channel
           0,
           40); // message code for "please follow me" */
 special_AI(0, 103);
 gosub start_wandering;
}


charge_interface_max();

if (get_damage()
 && get_interface_strength() < get_interface_capacity() - 100)
{
 if (random(2)) 
  firebase_angle = angle - 2500 + random(1000);
   else
    firebase_angle = angle + 2500 - random(1000);
 firebase_dist = 300 + random(300);
// if (random(2))
//  firebase_template = TEMPLATE_FIREBASE;
//   else
    firebase_template = TEMPLATE_SPIKEBASE;
 build_process(firebase_template,
               cos(firebase_angle, firebase_dist),
               sin(firebase_angle, firebase_dist),
               angle,
               -1);
 special_AI(1, 12);
}



// What the process does next depends on its current mode
switch(mode)
{
/*
 case MODE_GUARD:
  if (!process[TARGET_PARENT].visible()) // visible() always returns 1 for a friendly process, unless it no longer exists
  {
// Parent destroyed. start wandering:
   mode = MODE_SEEK_WELL;
   gosub start_wandering;
   break;
  }
  int angle_to_parent;
// find the angle to the parent, then move in a circle around it:
  angle_to_parent = atan2(process[TARGET_PARENT].get_core_y() - core_y, process[TARGET_PARENT].get_core_x() - core_x);
  angle_to_parent += 1024;
  move_x = process[TARGET_PARENT].get_core_x() - cos(angle_to_parent, 300);
  move_y = process[TARGET_PARENT].get_core_y() - sin(angle_to_parent, 300);
  auto_move.move_to(move_x, move_y);
  gosub listen_for_well_claims;
  break;
*/

case MODE_SEEK_WELL:
  if (distance_from_xy(move_x, move_y) < 300)
  {
   gosub start_wandering;
   break;
  }
  auto_move.move_to(move_x, move_y);
  if (search_for_well() // found nearby well!
   && (get_well_x() != ignore_well_x
    || get_well_y() != ignore_well_y))
  {
   mode = MODE_BUILD_BASE;
   data_well_x = get_well_x();
   data_well_y = get_well_y();
   build_angle = random(8192);
   add_to_build_queue(TEMPLATE_BASE, 
                      data_well_x + cos(build_angle, 500), 
                      data_well_y + sin(build_angle, 500), 
                      build_angle, 
                      1, // front of queue
                      0);
   break;
  }
  gosub scan_for_target;
  if (mode == MODE_SEEK_WELL)
   gosub listen_for_broadcasts;
  break;
  
case MODE_BUILD_BASE:
  auto_move.move_to(data_well_x, data_well_y);
  if (distance_from_xy(data_well_x, data_well_y) < 300)
  {
// first check for a friendly base already near the well.
// If there is one, ignore this well and try to find another.  
   scan_result = scan_single(0, // x_offset
                             0, // y_offset
                             -1, // process memory. -1 means discard target
                             2, // 2 = accept friendly only
                             0, // components min
                             100, // components max (100 is too high so this will always pass)
                             0b100); // bitfield for accepting processes with build object
// usually this scan is for an allocator. But here we want to ignore outposts, which have allocators but not builders.
   if (scan_result > 0)
   {
    ignore_well_x = data_well_x;
    ignore_well_y = data_well_y;
    mode = MODE_SEEK_WELL;
    gosub start_wandering;
    cancel_build_queue();
    break;
   }
// there doesn't appear to be a nearby friendly base. So let's build one!
//  ... although leave some data for building firebases if needed:
   if (get_available_data() < 200)
    break;

   int build_result;
   build_result = build_from_queue(-1); // -1 means not to save built process as target
   if (build_result == 1)
   {
    ignore_well_x = data_well_x;
    ignore_well_y = data_well_y;
    mode = MODE_SEEK_WELL;
    gosub start_wandering;
// don't need to call cancel_build_queue() here as build_from_queue removes the built process from the queue
    break;    
   }
   if (build_result == -7) // collision with static process (may be an outpost)
   {
// this should just result in the builder trying to build the base again, in a slightly different location.
    cancel_build_queue();
    mode = MODE_SEEK_WELL;
   }
  }
  gosub scan_for_target;
  break;

 case MODE_ATTACK:
  auto_move.approach_track(TARGET_MAIN, random(10), auto_att_main, 700);
  if (arc_length(angle, process[TARGET_MAIN].target_angle()) < 1024)
   auto_att_main.fire(1);
  if (!process[TARGET_MAIN].visible()
   || process[TARGET_MAIN].distance_more(3000))
  {
   if (target_destroyed(TARGET_MAIN))
    special_AI(1, 2);
   mode = MODE_SEEK_WELL;
   gosub start_wandering;
  }
  break; 
/*
// siege is like attack, but process will bombard target even if not visible
 case MODE_SIEGE_BOMBARD:
  auto_move.approach_xy(attack_x, attack_y, 2000);
  auto_att_spike.fire_spike_xy(attack_x, attack_y);
  if (!check_xy_visible(attack_x, attack_y))
  {
   bombard_count ++;
   if (bombard_count > 90)
   {
    mode = MODE_SIEGE_APPROACH;
   }
  }
   else
   {
    if (!process[TARGET_MAIN].visible())
    {
     mode = MODE_SEEK_WELL;
     gosub start_wandering;
     break;
    }
   }
  break;
  
 case MODE_SIEGE_APPROACH: 
  auto_move.approach_xy(attack_x, attack_y, 900);
  auto_att_spike.fire_spike_xy(attack_x, attack_y);
  if (check_xy_visible(attack_x, attack_y))
  {
// check for target no longer existing:
   if (!process[TARGET_MAIN].visible())
   {
    mode = MODE_SEEK_WELL;
    gosub start_wandering;
    break;
   }
// target still exists. go back to bombard mode:
   mode = MODE_SIEGE_BOMBARD;
   bombard_count = 0;  
   gosub broadcast_target_found;
   }
  break;
  */

} // end of mode switch


auto_att_back.attack_scan(4096, 400, TARGET_BACK);
auto_att_fwd.attack_scan(0, 400, TARGET_FWD);
//auto_att_left.attack_scan(-2800, 400, TARGET_LEFT);
//auto_att_right.attack_scan(2800, 400, TARGET_RIGHT);

//restore_scan(0,0);
//repair_scan(0,0);
restore_self();
repair_self();


if (broadcast_count > 0)
 broadcast_count --;



exit;


start_wandering:
 move_x = 1500 + random(world_x() - 3000);
 move_y = 1500 + random(world_y() - 3000);
 return;


scan_for_target:
// first, scan for a static target
 scan_result = scan_single(0, // x_offset
               0, // y_offset
               TARGET_MAIN,
               0, // accept friendly (0 means ignore friendly)
               0, // minimum components
               100, // maximum components
               0b1); // bitfield for accepting static processes
 if (scan_result)
 {
  mode = MODE_ATTACK; //MODE_SIEGE_BOMBARD;
  bombard_count = 0;
  attack_x = process[TARGET_MAIN].get_core_x();
  attack_y = process[TARGET_MAIN].get_core_y();
  cancel_build_queue();
  special_AI(1, 1);
  gosub broadcast_target_found;
  return; 
 }
// no static target found. Now scan for medium-sized or larger enemies:
 scan_result = scan_single(0, // x_offset
               0, // y_offset
               TARGET_MAIN,
               0, // accept friendly (0 means ignore friendly)
               0, // minimum components
               100, // maximum components
               0b10); // bitfield for accepting mobile processes
 if (scan_result)
 {
  mode = MODE_ATTACK;
  attack_x = process[TARGET_MAIN].get_core_x();
  attack_y = process[TARGET_MAIN].get_core_y();
  cancel_build_queue();
  special_AI(1, 1);
  gosub broadcast_target_found;
 }
 return;
 

broadcast_target_found:
// consider letting nearby processes know that there's a target here
  if (broadcast_count <= 0)
  {
     broadcast_target(4000, // range
                      1, // channel
                      0, // priority (0 just means a 1 priority message will overwrite this one)
                      TARGET_MAIN, // this target is attached to the broadcast. A listener can retrieve it with get_message_target().
                      attack_x, // message contents. retrieved sequentially by read_message().
                      attack_y); // message contents

     broadcast_count = 48;
   
  }
 return;


listen_for_broadcasts:
  if (next_message())
  {
// In this story mission, we can assume that any message is a "target found" broadcast with the following format:
//  0: target_x
//  1: target_y
// + an attached target.
//   target_x = read_message();
//   target_y = read_message();
   get_message_target(TARGET_MAIN);
   mode = MODE_ATTACK;
  }
  return;
