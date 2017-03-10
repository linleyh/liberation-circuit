
#process "m_builder"

class auto_move;

core_pent_A, 0, 
  {object_build, 0},
  {object_move:auto_move, 2048},
  {object_move:auto_move, 946},
  {object_move:auto_move, -946},
  {object_move:auto_move, -2048},

#code


enum
{
  MODE_SEEK_WELL, // process is wandering randomly looking for a well
  MODE_BUILD_BASE, // process is trying to build a base near a data well
  MODES
};

// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
TARGET_PARENT, // parent process automatically stored in address 0
TARGET_LEFT,
TARGET_RIGHT
};

enum
{
TEMPLATE_BASE, // assumes that a base process is in template 0
TEMPLATE_BUILDER,
//TEMPLATE_HARVESTER,
TEMPLATE_COMMANDER,
TEMPLATE_ATTACKER,
TEMPLATE_MINBASE
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
int broadcast_count;

int initialised;

if (initialised == 0)
{
 initialised = 1;
// listen_channel(2); // processes in this mission use channel 2 for messages about building
// listen_channel(3); //  and 3 for messages about harvesting and data wells
 mode = MODE_SEEK_WELL;
 gosub start_wandering;
}



if (broadcast_count <= 0)
{
// this process periodically broadcasts its presence to any nearby followers:
 broadcast(1000, // range of broadcast (in pixels).
           5, // channel - follower processes in this mission listen to channel 5
           0, // priority - 0 or 1. This message doesn't need high priority (which replaces low priority messages)
           40, // message contents - code 40 means "please follow me"
           0); // message contents (can be up to 8 parameters here, but this is a simple message)
 broadcast_count = 50;
}
 else
  broadcast_count --;




// What the process does next depends on its current mode
switch(mode)
{

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
   add_to_build_queue(TEMPLATE_MINBASE, 
                      data_well_x + cos(build_angle, 500), 
                      data_well_y + sin(build_angle, 500), 
                      build_angle, 
                      0,
                      0);
   break;
  }
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
    cancel_build_queue();
    gosub start_wandering;
    break;
   }
// there doesn't appear to be a nearby friendly base. So let's build one!
// the call to add_to_build_queue above should have left a base on the build queue, so we can try to build it:
   int build_result;
   build_result = build_from_queue(-1); // -1 means not to save built process as target
   if (build_result == 1)
   {
    ignore_well_x = data_well_x;
    ignore_well_y = data_well_y;
    mode = MODE_SEEK_WELL;
// don't need to call cancel_build_queue() here as build_from_queue removes the built process from the queue
    gosub start_wandering;
    break;    
   }
  }
  break;

} // end of mode switch


//auto_att_left.attack_scan(-3000, 300, TARGET_LEFT);
//auto_att_right.attack_scan(-3000, 300, TARGET_LEFT);

restore_self();
repair_self();

exit;


start_wandering:
 move_x = 800 + random(world_x() - 1600);
 move_y = 800 + random(world_y() - 1600);
 return;
