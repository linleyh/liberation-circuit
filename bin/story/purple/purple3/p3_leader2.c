
#process "flagship2"
// This process has objects with the following auto classes:
class auto_move;
class auto_att_main;
class auto_stability;
class auto_att_right;
class auto_att_left;
class stability_1;
class stability_2;
class stability_4;
class stability_11;
class stability_12;
class stability_14;
class auto_att_fwd;

// The following auto classes are not currently used by any objects:
class auto_retro;
class auto_att_back;
class auto_att_spike;
class auto_harvest;
//class auto_allocate;

core_hex_C, 0, 
  {object_repair, 0},
  {object_downlink, 432, 
    {component_side, // component 1
      {object_none, 0},
      {object_uplink, 0},
      {object_downlink, 163, 
        {component_long5, // component 2
          {object_downlink, 0, 
            {component_cap, // component 3
              {object_uplink, 0},
              {object_move:auto_move, 784},
              {object_move:auto_move, -522},
              {object_move:auto_move, -1383},
            }
          },
          {object_burst_l:auto_att_main, 413},
          {object_uplink, 0},
          {object_pulse_l:auto_att_right, 1063},
          {object_stability:stability_2:auto_stability, 0},
        }
      },
      {object_stability:stability_1:auto_stability, 0},
      {object_stream_dir:auto_att_fwd, -732},
      {object_downlink, 733, 
        {component_cap, // component 4
          {object_burst_l:auto_att_main, -421},
          {object_none, 0},
          {object_stability:stability_4:auto_stability, 0},
          {object_uplink, 0},
        }
      }
    }
  },
  {object_downlink, -88, 
    {component_cap, // component 5
      {object_downlink, 0, 
        {component_cap, // component 6
          {object_move:auto_move, -271},
          {object_move:auto_move, -1286},
          {object_interface, 0},
          {object_uplink, 0},
        }
      },
      {object_interface, 0},
      {object_downlink, -1072, 
        {component_cap, // component 7
          {object_uplink, 0},
          {object_interface, 0},
          {object_move:auto_move, 1282},
          {object_interface, 0},
        }
      },
      {object_uplink, 0},
    }
  },
  {object_repair_other, 0},
  {object_downlink, 88, 
    {component_cap, // component 8
      {object_uplink, 0},
      {object_downlink, 1072, 
        {component_cap, // component 9
          {object_interface, 0},
          {object_move:auto_move, -1282},
          {object_interface, 0},
          {object_uplink, 0},
        }
      },
      {object_interface, 0},
      {object_downlink, 0, 
        {component_cap, // component 10
          {object_uplink, 0},
          {object_interface, 0},
          {object_move:auto_move, 1286},
          {object_move:auto_move, 271},
        }
      },
    }
  },
  {object_downlink, -432, 
    {component_side, // component 11
      {object_uplink, 0},
      {object_none, 0},
      {object_stability:stability_11:auto_stability, 0},
      {object_downlink, -163, 
        {component_long5, // component 12
          {object_downlink, 0, 
            {component_cap, // component 13
              {object_move:auto_move, 1383},
              {object_move:auto_move, 522},
              {object_move:auto_move, -784},
              {object_uplink, 0},
            }
          },
          {object_uplink, 0},
          {object_burst_l:auto_att_main, -413},
          {object_stability:stability_12:auto_stability, 0},
          {object_pulse_l:auto_att_left, -1063},
        }
      },
      {object_downlink, -729, 
        {component_cap, // component 14
          {object_uplink, 0},
          {object_stability:stability_14:auto_stability, 0},
          {object_none, 0},
          {object_burst_l:auto_att_main, 421},
        }
      },
      {object_stream_dir:auto_att_fwd, 732}
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
CHANNEL_HELP,
CHANNEL_BOMBARD
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
  MODE_WANDER,
  MODE_ATTACK,
  MODE_HELP,
  MODES
};


// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder as entry 0
  TARGET_MAIN, // main target
  TARGET_FRONT, // target of directional forward attack
  TARGET_BACK,
  TARGET_LEFT,
  TARGET_RIGHT
};


int attacking_front; // is set to 1 if forward directional attack objects have a target
int front_attack_primary; // is set to 1 if forward directional attack objects are attacking
 // the primary target (e.g. target selected by command). Set to 0 if the objects are available for autonomous fire.
int attacking_back, attacking_left, attacking_right;

int other_target_x, other_target_y;

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
int saved_mode; // save the process' mode while it's attacking something it found
int using_main_attack;

int move_x, move_y; // destination
int target_x, target_y; // location of target (to attack, follow etc)
int attack_x, attack_y;

int scan_result; // used to hold the results of a scan of nearby processes

int broadcast_count;
int cycles;
cycles ++;

int initialised;
if (initialised == 0)
{
 initialised = 1;
 listen_channel(CHANNEL_TARGET); // scouts in this misson use this channel to broadcast "target found" messages
 listen_channel(CHANNEL_HELP); // used by outposts etc under attack
 mode = MODE_WANDER;
 attack_mode(2); // needs to be 2 because auto_att_main has 6 objects
 special_AI(0, 401);
 gosub start_wandering;
}

using_main_attack = 0; // this will be set to 1 if process is attacking its main target

if (broadcast_count <= 0)
{
// Because this is a leader, it periodically broadcasts its presence to any nearby followers:
 broadcast(-1, // range of broadcast (-1 means unlimited range).
           CHANNEL_FLAGSHIP,
           0, // priority - 0 or 1. This message doesn't need high priority (which replaces low priority messages)
           MESSAGE_FLAGSHIP); // message contents - code 40 means "please follow me"
 broadcast_count = 100;
}
 else
  broadcast_count --;

// What the process does next depends on its current mode
switch(mode)
{


 case MODE_WANDER:
  gosub listen_for_broadcasts;
 case MODE_HELP: 
  if (distance_from_xy(move_x, move_y) < 500)
  {
   gosub start_wandering;
   break;
  }
  if (scan_single(0,0,TARGET_MAIN,0,6,100,0) // 0 means any target
   || scan_single(0,0,TARGET_MAIN,0,0,100,0b1000)) // 0b1000 means only processes with allocator
  {
   mode = MODE_ATTACK;
   if (process[TARGET_MAIN].target_signature() & 0b1000) // 0b1000 means only processes with allocator
   {
        special_AI(1, 6);
        broadcast_target(-1, // range (-1 means unlimited range)
                      CHANNEL_TARGET, // channel (see the listen_channel(1) call in other process' code)
                      0, // priority (0 just means a 1 priority message will overwrite this one)
                      TARGET_MAIN, // this target is attached to the broadcast. A listener can retrieve it with get_message_target().
                      MESSAGE_TARGET_FOUND, // message contents
                      process[TARGET_MAIN].get_core_x(), // message contents. retrieved sequentially by read_message().
                      process[TARGET_MAIN].get_core_y()); // message contents
   
   
   }
    else
     special_AI(1, 5);
   break;
  }
  auto_move.move_to(move_x, move_y);
  break;
  
 case MODE_ATTACK: // attacking something it found itself
  if (process[TARGET_MAIN].visible() <= 0) // target no longer exists, or is out of range
  {
   if (target_destroyed(TARGET_MAIN))
    special_AI(1, 2);
   auto_move.move_to(attack_x, attack_y);
   if (distance_from_xy_less(attack_x, attack_y, 600))
   {
    special_AI(1, 7);
    gosub start_wandering;
   } 
   break;
  }
  attack_x = process[TARGET_MAIN].get_core_x();
  attack_y = process[TARGET_MAIN].get_core_y();
  auto_move.approach_track(TARGET_MAIN, random(20), auto_att_main, 600);
/*
  if (process[TARGET_MAIN].distance() < 1000
   && arc_length(angle, atan2(process[TARGET_MAIN].get_core_y() - core_y, process[TARGET_MAIN].get_core_x() - core_x)) <= 1000)
  { 
   auto_att_main.fire(0);
//   using_main_attack = 1;
  }  */
  break;
  
  
} // end of mode switch

//if (!using_main_attack)
if (auto_att_fwd.attack_scan(0, 400, TARGET_FRONT))
 auto_att_main.fire(0);
auto_att_left.attack_scan(-2048, 400, TARGET_LEFT);
auto_att_right.attack_scan(2048, 400, TARGET_RIGHT);
//auto_att_back.attack_scan(4096, 400, TARGET_BACK);


// give next priority to charging interface:
charge_interface_max();

int component_hit_time [15]; // only some of these values are used. There's plenty of memory.

enum
{
 STABILITY_TIME = 20
};

// check for stability:
 if (component[1].get_component_hit())
  component_hit_time [1] = cycles + STABILITY_TIME;
 if (component[2].get_component_hit())
  component_hit_time [2] = cycles + STABILITY_TIME;
 if (component[4].get_component_hit())
  component_hit_time [4] = cycles + STABILITY_TIME;
 if (component[11].get_component_hit())
  component_hit_time [11] = cycles + STABILITY_TIME;
 if (component[12].get_component_hit())
  component_hit_time [12] = cycles + STABILITY_TIME;
 if (component[14].get_component_hit())
  component_hit_time [14] = cycles + STABILITY_TIME;

  
 if (component_hit_time [1] > cycles)
  stability_1.set_stability(1);
   else 
    stability_1.set_stability(0);

 if (component_hit_time [2] > cycles)
  stability_2.set_stability(1);
   else 
    stability_2.set_stability(0);
    
 if (component_hit_time [4] > cycles)
  stability_4.set_stability(1);
   else 
    stability_4.set_stability(0);

 if (component_hit_time [11] > cycles)
  stability_11.set_stability(1);
   else 
    stability_11.set_stability(0);

 if (component_hit_time [12] > cycles)
  stability_12.set_stability(1);
   else 
    stability_12.set_stability(0);

 if (component_hit_time [14] > cycles)
  stability_14.set_stability(1);
   else 
    stability_14.set_stability(0);
    


restore_self();

repair_self();

restore_scan(0,0); // scans for nearby processes with destroyed components and tries to restore them

repair_scan(0,0); // scans for nearby damaged processes and tries to repair them
  
exit;


start_wandering:
 mode = MODE_WANDER;
 move_x = 800 + random(world_x() - 1600);
 move_y = 800 + random(world_y() - 1600);
 return;



listen_for_broadcasts:
// listen for messages (the listen_channel() call above allows broadcasts to be received)
 if (next_message())
 {
  switch(read_message())
  {
   case MESSAGE_TARGET_FOUND:
// expected format:
//  0: MESSAGE_TARGET_FOUND
//  1: target_x
//  2: target_y
// + an attached target.
    get_message_target(TARGET_MAIN);  
    attack_x = read_message();
    attack_y = read_message();
    mode = MODE_ATTACK;
        broadcast_target(-1, // range (-1 means unlimited range)
                      CHANNEL_TARGET, // channel (see the listen_channel(1) call in other process' code)
                      0, // priority (0 just means a 1 priority message will overwrite this one)
                      TARGET_MAIN, // this target is attached to the broadcast. A listener can retrieve it with get_message_target().
                      MESSAGE_TARGET_FOUND, // message contents
                      process[TARGET_MAIN].get_core_x(), // message contents. retrieved sequentially by read_message().
                      process[TARGET_MAIN].get_core_y()); // message contents
    break;
    
   case MESSAGE_UNDER_ATTACK:
    mode = MODE_HELP;
    move_x = get_message_x();
    move_y = get_message_y();
    break; 
  }
 }
 return;
