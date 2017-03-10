

#process "harvester"

class auto_move;
class main_move;
class auto_harvest;

core_quad_A, 4096, 
  {object_storage, 0},
  {object_move:auto_move:main_move, -2048},
  {object_harvest:auto_harvest, 0},
  {object_move:auto_move:main_move, 2048},

#code


enum
{
  MODE_SEEK_WELL, // process is wandering randomly looking for a well
  MODE_SEEK_ALLOCATOR, // process is wandering randomly looking for an allocator (usual one destroyed)
  MODE_HARVEST, // process is harvesting data from a data well (or travelling to do so)
  MODE_HARVEST_RETURN, // process has harvested data and is returning to an allocator
  MODES
};


enum
{
// These are the channels that processes in this stage
//  use to communicate with each other.
// There can be up to 8
CHANNEL_MAIN_BASE,
CHANNEL_FLAGSHIP,
CHANNEL_TARGET,
CHANNEL_WELL,
CHANNEL_REQUEST_FOLLOWER,
CHANNEL_HELP
};

enum
{
// these are codes for the first value of a broadcast message.
// they tell the recipient what kind of messge it is.
//  (the recipient needs the same enum declaration)
MESSAGE_MAIN_BASE, // the main base broadcasts a message which prevents other bases taking over as main base
MESSAGE_TARGET_FOUND,
MESSAGE_FLAGSHIP,
MESSAGE_WELL_CLAIM,
MESSAGE_FORMATION,
MESSAGE_REQUEST_FOLLOWER,
MESSAGE_UNDER_ATTACK
};


// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_ALLOCATOR, // process that this process will return to when finished harvesting
  TARGET_ENEMY // any nearby enemy
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
int allocator_x, allocator_y; // location of process this process will return to with data
int source_well_x, source_well_y; // ignore data well near process that built this harvest
int ignore_well_x, ignore_well_y; // ignore data well with a friendly allocator near it

int initialised;

if (initialised == 0)
{
 initialised = 1;
 allocator_x = process[TARGET_ALLOCATOR].get_core_x();
 allocator_y = process[TARGET_ALLOCATOR].get_core_y();
// ignore the data well that probably exists near creation point:
 source_well_x = get_well_x();
 source_well_y = get_well_y();
 mode = MODE_SEEK_WELL;
 listen_channel(CHANNEL_WELL);
 broadcast(1000, // range of broadcast, in pixels
           CHANNEL_REQUEST_FOLLOWER, // channel
           0,
           MESSAGE_REQUEST_FOLLOWER);
 special_AI(0, 406);
 gosub start_wandering;
}

/*
// If under attack, call for help:
if (scan_for_threat(0,0,TARGET_ENEMY))
{
   broadcast_target(4000, // range of broadcast, in pixels
                    4, // processes in this mission use channel 4 to communicate targetting information
                    1, // priority
                    TARGET_ENEMY, // the target will be broadcast so other processes can find it directly
// message contents:
                    1, // message code: indicates target found
                    process[TARGET_ENEMY].get_core_x(),
                    process[TARGET_ENEMY].get_core_y());
}
*/


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
  if (distance_from_xy(allocator_x, allocator_y) > 1000) // don't search near parent
  {
   if (search_for_well() // found nearby well!
//    && get_well_data() > 100
    && (get_well_x() != source_well_x
     || get_well_y() != source_well_y)
    && (get_well_x() != ignore_well_x
     || get_well_y() != ignore_well_y))
   {
    mode = MODE_HARVEST;
    data_well_x = get_well_x();
    data_well_y = get_well_y();
    gosub this_well_is_taken;
   }
  }
  if (get_data_stored() == get_data_capacity())
  {
    mode = MODE_HARVEST_RETURN;
  }
  break;

case MODE_SEEK_ALLOCATOR:
  scan_result = scan_single(0, // x_offset
                            0, // y_offset
                            TARGET_ALLOCATOR, // process memory
                            2, // 2 = accept friendly only
                            0, // components min
                            100, // components max (100 is too high so this will always pass)
                            0b1000); // bitfield for accepting processes with allocators
  if (scan_result == 1)
  {
   allocator_x = process[TARGET_ALLOCATOR].get_core_x();
   allocator_y = process[TARGET_ALLOCATOR].get_core_y();
   mode = MODE_HARVEST_RETURN;
   break;
  }
  if (distance_from_xy(move_x, move_y) < 300)
  {
   gosub start_wandering;
   break;
  }
  auto_move.move_to(move_x, move_y);
  break;

case MODE_HARVEST:
// only gather if near target (avoids inadvertently gathering from another well)
  if (distance_from_xy(data_well_x, data_well_y) < 800)
  {
// check for nearby friendly allocator. If found, ignore this well:
   scan_result = scan_single(0, // x_offset
                             0, // y_offset
                             TARGET_ALLOCATOR, // process memory
                             2, // 2 = accept friendly only
                             0, // components min
                             100, // components max (100 is too high so this will always pass)
                             0b1000); // bitfield for accepting processes with allocators
   if (scan_result == 1)
   {
    allocator_x = process[TARGET_ALLOCATOR].get_core_x();
    allocator_y = process[TARGET_ALLOCATOR].get_core_y();
    gosub this_well_is_taken;
    mode = MODE_SEEK_WELL;
    ignore_well_x = data_well_x;
    ignore_well_y = data_well_y;
    break;
   }
   auto_harvest.gather_data();
   main_move.move_to(data_well_x, data_well_y); // save power while harvesting by only using two move objects.
//   front_move.set_power(0); // turn front move objects off
  }
   else
   {
    auto_move.move_to(data_well_x, data_well_y);  
    gosub listen_for_well_claims;
   }
  if (get_data_stored() == get_data_capacity())
  {
    mode = MODE_HARVEST_RETURN;
  }
  gosub this_well_is_taken;
  break;

case MODE_HARVEST_RETURN:
// check that allocator is still alive:
  if (process[TARGET_ALLOCATOR].visible() <= 0) // allocator no longer exists
  {
// if not, look for a new one
   mode = MODE_SEEK_ALLOCATOR;
   gosub start_wandering;
   break;
  }
  gosub listen_for_well_claims;  
   auto_harvest.give_data(TARGET_ALLOCATOR, 100);
   if (get_data_stored() == 0)
   {
//    broadcast(1000, // range of broadcast, in pixels
//              CHANNEL_REQUEST_FOLLOWER, // channel
//              0,
//              MESSAGE_REQUEST_FOLLOWER); // message code for "please follow me"
    if (data_well_x == 0)
     mode = MODE_SEEK_WELL;
      else
       mode = MODE_HARVEST;
   }
//  }
  auto_move.move_to(allocator_x, allocator_y);
  break;

} // end of mode switch






exit;


start_wandering:
// doesn't set mode, as can be called from different modes.
 if (process[TARGET_ALLOCATOR].visible())
 {
  move_x = process[TARGET_ALLOCATOR].get_core_x() - 6000 + random(12000);
  if (move_x < 600)
   move_x += 6000;
  if (move_x > world_x() - 400)
   move_x -= 6000;
  move_y = process[TARGET_ALLOCATOR].get_core_y() - 6000 + random(12000);
  if (move_y < 600)
   move_y += 6000;
  if (move_y > world_y() - 400)
   move_y -= 6000;
  return;
 }
 move_x = 800 + random(world_x() - 1600);
 move_y = 800 + random(world_y() - 1600);
 return;


this_well_is_taken:
 // tell other harvesters that this well is taken:
 broadcast(-1, // range of broadcast (in pixels). -1 means unlimited range.
           CHANNEL_WELL, // channel - harvesters in this mission use this channel
           0, // priority - 0 or 1. 0 means message may be replaced by priority 1 messages
           MESSAGE_WELL_CLAIM,
           data_well_x, // message contents (variable number of arguments; up to 8)
           data_well_y);
 return;


listen_for_well_claims:
// Now listen to other processes that may have announced that the data well is taken:
  while(next_message())
  {
   if (read_message() == MESSAGE_WELL_CLAIM) // code for well claim
   {
    if (read_message() == data_well_x
     && read_message() == data_well_y)    
    {
     ignore_well_x = data_well_x;
     ignore_well_y = data_well_y;
     mode = MODE_SEEK_WELL;
     gosub start_wandering;
    }
   }
  }
 return;
