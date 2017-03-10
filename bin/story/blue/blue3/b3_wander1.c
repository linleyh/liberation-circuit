

#process "wander1"

class auto_move;
class auto_att_fwd;

core_quad_A, 4096, 
  {object_move:auto_move, 0},
  {object_downlink, 1078, 
    {component_tri, // component 2
      {object_move:auto_move, -1295},
      {object_uplink, 0},
      {object_pulse:auto_att_fwd, 0},
    }
  },
  {object_pulse:auto_att_fwd, 0},
  {object_downlink, -1078, 
    {component_tri, // component 1
      {object_move:auto_move, 1295},
      {object_pulse:auto_att_fwd, 0},
      {object_uplink, 0},
    }
  },

#code




// Process AI modes (these reflect the capabilities of the process)
enum
{
  MODE_WANDER,
  MODE_ATTACK,
  MODE_SEEK,
  MODES
};


// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder as entry 0
  TARGET_MAIN, // main target
  TARGET_OPPORTUNITY // something it comes across while looking for a target.
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

int move_x, move_y; // destination
int target_x, target_y; // location of target (to attack, follow etc)

int scan_result; // used to hold the results of a scan of nearby processes

int initialised;
if (initialised == 0)
{
 initialised = 1;
 attack_mode(1);
 listen_channel(1);
 special_AI(0, 6);
 gosub start_wandering;
}

// What the process does next depends on its current mode
switch(mode)
{


 case MODE_WANDER:
  if (distance_from_xy(move_x, move_y) < 300)
  {
   gosub start_wandering;
   break;
  }
  if (scan_for_threat(0, 0, TARGET_MAIN))
  {
   mode = MODE_ATTACK;
   if (process[TARGET_MAIN].target_signature() & 0b1000) // 0b1000 is the signature for an allocator
   {
     broadcast_target(-1, // range of broadcast; -1 means unlimited range
               1, // channel
               0, // priority
               TARGET_MAIN, // target to broadcast
               10, // message code for "target_found"
               process[TARGET_MAIN].get_core_x(),
               process[TARGET_MAIN].get_core_y());
    special_AI(1, 6);
   }
    else
    {
     broadcast_target(3000, // range of broadcast
               1, // channel
               0, // priority
               TARGET_MAIN, // target to broadcast
               10, // message code for "target_found"
               process[TARGET_MAIN].get_core_x(),
               process[TARGET_MAIN].get_core_y());
     special_AI(1, 1);
    }
// no break; falls through to attack case below
  }
   else
   {
    auto_move.move_to(move_x, move_y);
    if (next_message())
    {
     if (read_message() == 10)
     {
      mode = MODE_SEEK;
      move_x = read_message();
      move_y = read_message();
      get_message_target(TARGET_MAIN);      
     }
    }
    break;
   }
// fall through...
 case MODE_ATTACK:
  if (process[TARGET_MAIN].visible() <= 0) // target no longer exists, or is out of range
  {
   if (target_destroyed(TARGET_MAIN))
    special_AI(1, 2);
   gosub start_wandering; // give up and go back to wandering around randomly
   break;
  }

  auto_move.approach_target(TARGET_MAIN, 0, 600); // approach to within 600 pixels of target's core

  if (process[TARGET_MAIN].distance_less(1000))
    auto_att_fwd.fire_at(TARGET_MAIN, 0); 
     else
      auto_att_fwd.attack_scan(0, 400, TARGET_OPPORTUNITY);

  break;
  
 case MODE_SEEK:

  auto_att_fwd.attack_scan(0, 400, TARGET_OPPORTUNITY);

  if (process[TARGET_MAIN].visible() <= 0) // target no longer exists, or is out of range
  {
// check whether the target should be in range:
   if (distance_from_xy_less(move_x, move_y, 500))
   {
    gosub start_wandering;
    break;
   }
   auto_move.move_to(move_x, move_y);
   break;
  } 
  move_x = process[TARGET_MAIN].get_core_x();
  move_y = process[TARGET_MAIN].get_core_y();
  auto_move.move_to(move_x, move_y);
  if (distance_from_xy_less(move_x, move_y, 700))
   mode = MODE_ATTACK;
  break; 
  
} // end of mode switch



exit;


start_wandering:
 mode = MODE_WANDER;
 move_x = 800 + random(world_x() - 1600);
 move_y = 800 + random(world_y() - 1600);
 return;
