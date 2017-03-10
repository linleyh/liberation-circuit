

#process "picket"

class auto_att_fwd;
class auto_move;

core_quad_B, 0, 
  {object_pulse_l:auto_att_fwd, 249},
  {object_pulse:auto_att_fwd, -455},
  {object_move:auto_move, 1198},
  {object_move:auto_move, -1198},

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
  MODE_ATTACK,
  MODE_SCOUT,
  MODE_HELP,
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
  TARGET_MAIN
};

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

int mode; // what is the process doing? (should be one of the MODE enums)
int saved_mode; // will return to this mode after attacking something
int attack_x, attack_y;
int using_main_attack;

int move_x, move_y; // destination

int scan_result; // used to hold the results of a scan of nearby processes

int initialised;
if (initialised == 0)
{
 initialised = 1;
 attack_mode(1);
 special_AI(0, 402);
 mode = MODE_GUARD;
 target_copy(TARGET_LEADER, TARGET_PARENT);
 listen_channel(CHANNEL_FLAGSHIP);
 listen_channel(CHANNEL_HELP);
}


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
  saved_mode = MODE_WANDER;
  gosub scan_for_any_target;
  gosub listen_for_requests;
  break;
  

 case MODE_GUARD:
  saved_mode = MODE_GUARD;
  if (process[TARGET_LEADER].visible() <= 0) // leader doesn't exist
  {
   gosub start_wandering;
   break;
  }
//  if (!random(100))
//  {
//   gosub start_scouting;
//   break;
//  }
//  angle_to_leader = atan2(process[TARGET_PARENT].get_core_y() - core_y, process[TARGET_PARENT].get_core_x() - core_x);
  angle_to_leader = process[TARGET_LEADER].target_angle();
  angle_to_leader += 1024;
  move_x = process[TARGET_LEADER].get_core_x() - cos(angle_to_leader, 700);
  move_y = process[TARGET_LEADER].get_core_y() - sin(angle_to_leader, 700);
  auto_move.move_to(move_x, move_y);
  gosub scan_for_any_target;
  gosub listen_for_requests;
  break;


 case MODE_FORMATION:
  saved_mode = MODE_FORMATION;
  if (!process[TARGET_LEADER].visible())
  {
   if (process[TARGET_PARENT].visible())
   {
    target_copy(TARGET_LEADER, TARGET_PARENT);
    mode = MODE_GUARD;
    break;
   }
   gosub start_wandering;
   break;
  }
  if (!random(500))
  {
   gosub start_scouting;
   break;
  }
  angle_to_leader = process[TARGET_LEADER].target_angle();
  angle_to_leader += 1024;
  move_x = process[TARGET_LEADER].get_core_x() - cos(angle_to_leader, 1600);
  move_y = process[TARGET_LEADER].get_core_y() - sin(angle_to_leader, 1600);
  auto_move.move_to(move_x, move_y);
  gosub scan_for_any_target;
  gosub scan_for_flagship_target;
  gosub listen_for_requests;
  break;
  
 case MODE_ATTACK:
  if (!process[TARGET_MAIN].visible())
  {
   if (target_destroyed(TARGET_MAIN))
    special_AI(1, 2);
   mode = saved_mode;
   break;
  }
  if (process[TARGET_MAIN].distance_less(1000))
  {
   auto_att_fwd.fire_at(TARGET_MAIN, 0);
   using_main_attack = 1;
  }
  auto_move.approach_target(TARGET_MAIN, 0, 500);
  break;
 
 case MODE_SCOUT:
  if (distance_from_xy_less(move_x, move_y, 500))
  {
   mode = saved_mode;
   break;
  }
  auto_move.move_to(move_x, move_y); 
//  gosub scan_for_any_target;
  gosub scan_for_flagship_target;
  break;
  
 case MODE_HELP:
  if (distance_from_xy_less(move_x, move_y, 500))
  {
   mode = saved_mode;
   break;
  }
  auto_move.move_to(move_x, move_y); 
  gosub scan_for_any_target;
//  gosub scan_for_flagship_target;
  break;

} // end of mode switch


if (!using_main_attack)
 auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);



exit;


start_wandering:
 mode = MODE_WANDER;
 move_x = 800 + random(world_x() - 1600);
 move_y = 800 + random(world_y() - 1600);
 return;
 

start_scouting: 
 mode = MODE_SCOUT;
 move_x = 500 + random(world_x() - 1000);
 move_y = 500 + random(world_y() - 1000);
 return;


listen_for_requests:
 if (next_message())
 {
  switch(read_message())
  {
  
   case MESSAGE_FLAGSHIP:
    get_message_source(TARGET_LEADER);
    mode = MODE_FORMATION; 
    break;
    
   case MESSAGE_UNDER_ATTACK:
    saved_mode = mode;
    mode = MODE_HELP;
    move_x = get_message_x();
    move_y = get_message_y();
    break; 

  }
 }
 return; 


scan_for_flagship_target:
// this subroutine is only called if there's a flagship
  if (scan_single(0,0,TARGET_MAIN,0,0,100,0b1000) // 0b1000 means only processes with allocator
   || scan_single(0,0,TARGET_MAIN,0,6,100,0)) // 0 means any process. The 6 is minimum size.
  {
   transmit_target(TARGET_LEADER, // target of transmission
                   0, // priority 0 - this isn't really a high priority message
                   TARGET_MAIN, // target attached to transmission
                   MESSAGE_TARGET_FOUND, 
                   process[TARGET_MAIN].get_core_x(),
                   process[TARGET_MAIN].get_core_y());
   mode = MODE_ATTACK; 
   special_AI(1, 6);
  }
  return;



scan_for_any_target:
  if (scan_for_threat(0, 0, TARGET_MAIN))
  {
   mode = MODE_ATTACK;
   special_AI(1, 5);
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
