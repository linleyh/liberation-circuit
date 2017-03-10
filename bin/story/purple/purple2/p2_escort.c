

#process "escort"

class auto_retro;
class auto_move;
class auto_att_fwd;
class auto_att_right;
class auto_att_left;

core_hex_A, 0, 
  {object_repair, 0},
  {object_downlink, 340, 
    {component_peak, // component 2
      {object_uplink, 0},
      {object_none, 0},
      {object_none, 0},
      {object_pulse_xl:auto_att_fwd, -384},
      {object_pulse_l:auto_att_right, 0},
    }
  },
  {object_downlink, 64, 
    {component_long6, // component 1
      {object_move:auto_move, 1154},
      {object_move:auto_move, -168},
      {object_move:auto_move, -1624},
      {object_uplink, 0},
      {object_interface, 0},
      {object_move:auto_move:auto_retro, -1370}
    }
  },
  {object_downlink, 0, 
    {component_long5, // component 3
      {object_uplink, 0},
      {object_interface, 0},
      {object_interface, 0},
      {object_move:auto_move, -1669},
      {object_move:auto_move, 1669},
    }
  },
  {object_downlink, -64, 
    {component_long6, // component 4
      {object_move:auto_move, -1154},
      {object_move:auto_move:auto_retro, 1370},
      {object_interface, 0},
      {object_uplink, -1432},
      {object_move:auto_move, 1624},
      {object_move:auto_move, 168}
    }
  },
  {object_downlink, -340, 
    {component_peak, // component 5
      {object_uplink, 0},
      {object_none, 0},
      {object_none, 0},
      {object_pulse_l:auto_att_left, 0},
      {object_pulse_xl:auto_att_fwd, 384},
    }
  }

#code



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



// Process AI modes (these reflect the capabilities of the process)
enum
{
  MODE_GUARD, // process is circling builder
  MODE_FORMATION, // process is in formation with flagship
  MODE_WANDER, // wandering randomly (should only happen if all bases and flagships destroyed)
  MODES
};


// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder as entry 0
  TARGET_LEADER, // leader this process has found  
  TARGET_FRONT,
  TARGET_LEFT,
  TARGET_RIGHT,
  TARGET_BACK,
  TARGET_SCAN
};

int attacking_back; // is set to 1 if backwards directional attack objects have a target
int other_target_x, other_target_y;
int angle_to_leader;

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

int formation_angle;
int formation_dist;

int mode; // what is the process doing? (should be one of the MODE enums)
int using_main_attack;

int move_x, move_y; // destination

int scan_result; // used to hold the results of a scan of nearby processes
int cycles;


int initialised;
if (initialised == 0)
{
 initialised = 1;
 attack_mode(1);
 mode = MODE_GUARD;
 listen_channel(CHANNEL_FLAGSHIP);
 special_AI(0, 404);
// the builder should have sent it a transmission with its position in the flagship formation: 
 if (next_message()
  && read_message() == MESSAGE_FORMATION)
 { 
  formation_angle = read_message();
  formation_dist = read_message();
 }
}

cycles ++;
using_main_attack = 0;


// What the process does next depends on its current mode
switch(mode)
{


 case MODE_WANDER:
  if (distance_from_xy(move_x, move_y) < 300)
  {
   gosub start_wandering;
   break;
  }
  auto_move.move_to(move_x, move_y);
  gosub listen_for_flagship;
//  listen_channel(CHANNEL_MAIN_BASE);
  break;
  

 case MODE_GUARD:
  if (process[TARGET_PARENT].visible() <= 0) // leader doesn't exist
  {
   mode = MODE_WANDER;
   gosub start_wandering;
   break;
  }
//  angle_to_leader = atan2(process[TARGET_PARENT].get_core_y() - core_y, process[TARGET_PARENT].get_core_x() - core_x);
  angle_to_leader = process[TARGET_PARENT].target_angle();
  angle_to_leader += 1024;
  move_x = process[TARGET_PARENT].get_core_x() - cos(angle_to_leader, 700);
  move_y = process[TARGET_PARENT].get_core_y() - sin(angle_to_leader, 700);
  auto_move.move_to(move_x, move_y);
  gosub listen_for_flagship;
  break;


 case MODE_FORMATION:
  if (!process[TARGET_LEADER].visible())
  {
   if (process[TARGET_PARENT].visible())
   {
    mode = MODE_GUARD;
    break;
   }
   mode = MODE_WANDER;
   gosub start_wandering;
   break;
  }
  angle_to_leader = process[TARGET_LEADER].get_core_angle() + formation_angle;
  auto_move.reposition(process[TARGET_LEADER].get_core_x() + cos(angle_to_leader, formation_dist),
                       process[TARGET_LEADER].get_core_y() + sin(angle_to_leader, formation_dist),
                       process[TARGET_LEADER].get_core_angle());
  gosub scan_for_flagship_target;
  break;

} // end of mode switch

 charge_interface_max();
 restore_self();
 repair_self();

 
 if (target_destroyed(TARGET_LEFT)
  || target_destroyed(TARGET_RIGHT)
  || target_destroyed(TARGET_BACK))
   special_AI(1, 2);
    else
    {
     if (get_total_integrity() < 200
      && get_damage() > 20)
      special_AI(1, 4);
    }


//if (!using_main_attack)
// auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);
 if (cycles & 1)
  auto_att_left.attack_scan(-2048, 400, TARGET_LEFT);
   else
    auto_att_right.attack_scan(2048, 400, TARGET_RIGHT);

// auto_att_back.attack_scan(4096, 400, TARGET_BACK);

exit;


start_wandering:
 mode = MODE_WANDER;
 move_x = 800 + random(world_x() - 1600);
 move_y = 800 + random(world_y() - 1600);
 return;


listen_for_flagship:
 if (next_message()
  && read_message() == MESSAGE_FLAGSHIP)
 {
  get_message_source(TARGET_LEADER);
  mode = MODE_FORMATION; 
 }
 return; 

scan_for_flagship_target:
// this subroutine is only called if there's a flagship
  if (scan_single(0,0,TARGET_SCAN,0,0,100,0b1000) // 0b1000 means only processes with allocator
   || scan_single(0,0,TARGET_SCAN,0,5,100,0)) // 0 means any target
  {
   transmit_target(TARGET_LEADER, // target of transmission
                   0, // priority 0 - this isn't really a high priority message
                   TARGET_SCAN, // target attached to transmission
                   MESSAGE_TARGET_FOUND, 
                   process[TARGET_SCAN].get_core_x(),
                   process[TARGET_SCAN].get_core_y());
   special_AI(1, 6);
  }
  return;



/*
listen_for_requests:
// listen to other procs requesting followers.
  if (check_messages() // returns 1 if message received since previous cycle
   && read_message() == MESSAGE_REQUEST_FOLLOWER // returns contents of message in sequence
   && random(2)) // may or may not decide to follow.
  {
   mode = MODE_GUARD;
   get_message_source(TARGET_LEADER); // saves message sender into targetting memory
  }
  return;
*/
