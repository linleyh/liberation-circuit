
#process "builder"

class auto_att_left;
class auto_att_right;
class auto_move;
class auto_att_back;

core_pent_C, 0, 
  {object_downlink, 0, 
    {component_bowl, // component 1
      {object_uplink, 0},
      {object_none, 0},
      {object_none, 0},
      {object_pulse_l:auto_att_left, -825},
      {object_pulse_l:auto_att_right, 851},
    }
  },
  {object_downlink, 577, 
    {component_bowl, // component 2
      {object_uplink, 0},
      {object_interface, 0},
      {object_move:auto_move, -1439},
      {object_move:auto_move, 2048},
      {object_pulse:auto_att_back, 535},
    }
  },
  {object_repair, 0},
  {object_build, 0},
  {object_downlink, -577, 
    {component_bowl, // component 3
      {object_uplink, 0},
      {object_move:auto_move, 1439},
      {object_interface, 0},
      {object_pulse:auto_att_back, -535},
      {object_move:auto_move, -2048},
    }
  },

#code


enum
{
  MODE_SEEK_WELL, // process is wandering randomly looking for a well
  MODE_GUARD, // process is waiting at the main base waiting for a harvester to find a data well
  MODE_BUILD_BASE, // process is trying to build a base near a data well
  MODES
};

// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
TARGET_PARENT, // parent process automatically stored in address 0
TARGET_FWD,
TARGET_BACK,
TARGET_LEFT,
TARGET_RIGHT,
};

enum
{
TEMPLATE_BASE, // assumes that a base process is in template 0
TEMPLATE_FIREBASE, // assumes that a firebase process is in template 1
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

int move_x, move_y; // destination

// Harvester variables
int data_well_x, data_well_y; // location of data well
int ignore_well_x, ignore_well_y; // ignore data well with a friendly allocator near it
int build_angle;

int initialised;

if (initialised == 0)
{
 initialised = 1;
 listen_channel(2); // processes in this mission use channel 2 for messages about building
 listen_channel(3); //  and 3 for messages about harvesting and data wells
 attack_mode(1);
 mode = MODE_SEEK_WELL;
/*
 broadcast(1000, // range of broadcast, in pixels
           5, // channel
           0,
           40); // message code for "please follow me" */
 gosub start_wandering;
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
/*  
  while(check_messages()) // returns 1 if message received since previous cycle
  {
   if (read_message() == 30 // message code for data well claim
    && read_message() == 0) // means well claimed by mobile harvester
   {
    mode = MODE_BUILD_BASE;
    data_well_x = read_message();
    data_well_y = read_message();
    build_angle = random(8192);
    add_to_build_queue(TEMPLATE_BASE, 
                       data_well_x + cos(build_angle, 500), 
                       data_well_y + sin(build_angle, 500), 
                       build_angle, 
                       0,
                       0);
    break;
   }
   next_message();
  }
*/  
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
                             0b1000); // bitfield for accepting processes with allocators
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
  }
  break;



} // end of mode switch

charge_interface_max();

auto_att_back.attack_scan(4096, 400, TARGET_BACK);
//auto_att_fwd.attack_scan(0, 400, TARGET_FWD);
auto_att_left.attack_scan(-1800, 400, TARGET_LEFT);
auto_att_right.attack_scan(1800, 400, TARGET_RIGHT);

restore_self();
repair_self();


exit;


start_wandering:
 move_x = 800 + random(world_x() - 1600);
 move_y = 800 + random(world_y() - 1600);
 return;
/*
listen_for_well_claims:
  while(check_messages()) // returns 1 if message received since previous cycle
  {
   if (read_message() == 30 // message code for data well claim
    && read_message() == 0) // means well claimed by mobile harvester
   {
    mode = MODE_BUILD_BASE;
    data_well_x = read_message();
    data_well_y = read_message();
    build_angle = random(8192);
    add_to_build_queue(TEMPLATE_BASE, 
                       data_well_x + cos(build_angle, 500), 
                       data_well_y + sin(build_angle, 500), 
                       build_angle, 
                       0,
                       0);
    break;
   }
   next_message();
  }
return;
*/
