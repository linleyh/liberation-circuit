

#process "harvester2"

class auto_att_back;
class auto_att_fwd;
class auto_move;
class auto_harvest;
class auto_att_right;

core_pent_B, 4096, 
  {object_repair, 0},
  {object_downlink, 877, 
    {component_prong, // component 1
      {object_uplink, 737},
      {object_downlink, -1001, 
        {component_prong, // component 2
          {object_move:auto_move, -1459},
          {object_uplink, 0},
          {object_pulse:auto_att_back, 0},
          {object_interface, 0},
        }
      },
      {object_pulse_l:auto_att_fwd, -883},
      {object_downlink, -6, 
        {component_cap, // component 3
          {object_uplink, 771},
          {object_move:auto_move, 1686},
          {object_interface, 0},
          {object_move:auto_move, -370},
        }
      },
    }
  },
  {object_downlink, 621, 
    {component_cap, // component 4
      {object_pulse_xl:auto_att_fwd, 0},
      {object_harvest:auto_harvest, 0},
      {object_storage, 0},
      {object_uplink, 0},
    }
  },
  {object_downlink, -298, 
    {component_prong, // component 5
      {object_pulse_l:auto_att_right, -393},
      {object_uplink, -68},
      {object_storage, 0},
      {object_storage, 0},
    }
  },
  {object_downlink, 303, 
    {component_prong, // component 6
      {object_move:auto_move, 163},
      {object_interface, 0},
      {object_downlink, 931, 
        {component_cap, // component 7
          {object_move:auto_move, -360},
          {object_move:auto_move, -1039},
          {object_none, 0},
          {object_uplink, 0},
        }
      },
      {object_uplink, 0},
    }
  },

#code


enum
{
  MODE_WANDER,
  MODE_SEEK_WELL, // process is wandering randomly looking for a well
  MODE_HARVEST, // process is harvesting data from a data well (or travelling to do so)
  MODE_HARVEST_RETURN, // process has harvested data and is returning to an allocator
  MODE_REQUEST_WELL,
  MODES
};


enum
{
// These are the channels that processes in this stage
//  use to communicate with each other.
// There can be up to 8
CHANNEL_HARVESTER, // not really used as all harvester messages are sent by transmit(), which doesn't use channels
CHANNEL_TARGET, // used for TARGET_FOUND messages
CHANNEL_HELP, // used for UNDER_ATTACK messages

};

enum
{
// these are codes for the first value of a broadcast message.
// they tell the recipient what kind of messge it is.
//  (the recipient needs the same enum declaration)
MESSAGE_NEXT_WELL_PLEASE, // harvester asks main base where to go
MESSAGE_GO_TO_WELL, // base tells harvester where to go
MESSAGE_WELL_FOUND,
MESSAGE_WELL_FOUND_ACK, // base acknowledged well found message
MESSAGE_SEEK_WELLS, // base tells harvester to look for new wells
MESSAGE_TARGET_FOUND,
MESSAGE_UNDER_ATTACK,
MESSAGE_REQUEST_FOLLOWER
};




// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_BASE, // main base
  TARGET_MAIN, // any nearby enemy

  TARGET_LEFT, // target of directional left attack
  TARGET_RIGHT, // target of directional right attack
  TARGET_FRONT, // target of directional front attack
  TARGET_BACK,

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

int seek_time; // counter for searching for new wells
int broadcast_count; // counter for broadcasting follower requests

int initialised;

int cycles;
cycles ++;

if (initialised == 0)
{
 initialised = 1;
 allocator_x = process[TARGET_BASE].get_core_x();
 allocator_y = process[TARGET_BASE].get_core_y();
// ignore the data well that probably exists near creation point:
 source_well_x = get_well_x();
 source_well_y = get_well_y();
 mode = MODE_REQUEST_WELL;
 special_AI(0, 303);
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


 if (broadcast_count <= 0)
 {
  broadcast(2000,
            CHANNEL_HARVESTER,
            0,
            MESSAGE_REQUEST_FOLLOWER);
  broadcast_count = 50;
 
 }
  else
   broadcast_count --;



  while(next_message())
  {
   switch(read_message())
   {
   
    case MESSAGE_GO_TO_WELL:
     mode = MODE_HARVEST;
     data_well_x = read_message();
     data_well_y = read_message();
     break;
    
    case MESSAGE_SEEK_WELLS:
     mode = MODE_SEEK_WELL;
     seek_time = 400;
     gosub start_wandering;
     break; 
     
    case MESSAGE_WELL_FOUND_ACK: 
     ignore_well_x = get_well_x();
     ignore_well_y = get_well_y();
     break;

   }
  }



gosub scan_for_target;



// What the process does next depends on its current mode
switch(mode)
{

// should only wander if base destroyed
case MODE_WANDER:
  if (distance_from_xy(move_x, move_y) < 300)
  {
   gosub start_wandering;
   break;
  }
  auto_move.move_to(move_x, move_y);
  break;



case MODE_SEEK_WELL:
  seek_time --;
  
  if (seek_time <= 0)
  {
   mode = MODE_REQUEST_WELL;
   break;
  }

  if (distance_from_xy(move_x, move_y) < 300)
  {
   gosub start_wandering;
   break;
  }
  auto_move.move_to(move_x, move_y);
  if (distance_from_xy(allocator_x, allocator_y) > 1000) // don't search near parent
  {
   if (search_for_well() == 1 // found nearby well!
    && (get_well_x() != source_well_x
     || get_well_y() != source_well_y)
    && (get_well_x() != ignore_well_x
     || get_well_y() != ignore_well_y))
   {
//    mode = MODE_HARVEST;
    transmit(TARGET_BASE, 
             1,
             MESSAGE_WELL_FOUND,
             get_well_x(),
             get_well_y());
   }
  }
  if (get_data_stored() == get_data_capacity())
  {
    mode = MODE_HARVEST_RETURN;
  }
  break;


case MODE_HARVEST:
// only gather if near target (avoids inadvertently gathering from another well)
  if (distance_from_xy(data_well_x, data_well_y) < 800)
  {
   auto_harvest.gather_data();
   auto_move.move_to(data_well_x, data_well_y); // save power while harvesting by only using two move objects.
//   front_move.set_power(0); // turn front move objects off
  }
   else
   {
    auto_move.move_to(data_well_x, data_well_y);  
   }
  if (get_data_stored() == get_data_capacity())
  {
    mode = MODE_HARVEST_RETURN;
  }
  if (get_well_data() == 0)
   transmit(TARGET_BASE,
            0,
            MESSAGE_NEXT_WELL_PLEASE);
  break;

case MODE_HARVEST_RETURN:
// check that allocator is still alive:
  if (process[TARGET_BASE].visible() <= 0) // allocator no longer exists
  {
// if not, wander
   mode = MODE_WANDER;
   gosub start_wandering;
   break;
  }
   auto_harvest.give_data(TARGET_BASE, 100);
   if (get_data_stored() == 0)
   {
    mode = MODE_REQUEST_WELL;
   }
//  }
  auto_move.move_to(allocator_x, allocator_y);
  break;
  
case MODE_REQUEST_WELL:
  transmit(TARGET_BASE,
           0,
           MESSAGE_NEXT_WELL_PLEASE);
  break;
           

} // end of mode switch




auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);
//auto_att_left.attack_scan(-2048, 400, TARGET_LEFT);
auto_att_right.attack_scan(2048, 400, TARGET_RIGHT);
auto_att_back.attack_scan(4096, 400, TARGET_BACK);


// give next priority to charging interface:
charge_interface_max();

restore_self();

repair_self();



exit;


start_wandering:
// doesn't set mode, as can be called from different modes.
 move_x = 400 + random(world_x() - 800);
 move_y = 400 + random(world_y() - 800);
 return;



scan_for_target:

// harvesters should only scan sometimes
//  (unlike guards, they don't change modes to a non-scanning mode when they find something)
  if (cycles % 30)
   return;

  if (scan_single(0,0,TARGET_MAIN,0,0,100,0b1000)) // 0b1000 means only processes with allocator
  {
   broadcast_target(-1,
                    CHANNEL_TARGET,
                    1, // priority 1
                    TARGET_MAIN, // target attached to transmission
                    MESSAGE_TARGET_FOUND, 
                    process[TARGET_MAIN].get_core_x(),
                    process[TARGET_MAIN].get_core_y());
   special_AI(1, 3); // under attack
   return;
  }
  if (scan_for_threat(0, 0, TARGET_MAIN))
  {
   broadcast_target(20000,
                    CHANNEL_TARGET,
                    0, // priority 1
                    TARGET_MAIN, // target attached to transmission
                    MESSAGE_TARGET_FOUND, 
                    process[TARGET_MAIN].get_core_x(),
                    process[TARGET_MAIN].get_core_y());
   special_AI(1, 3); // under attack
   return;
  }
  return;
