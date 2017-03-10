

#process "wander2"

class auto_move;
class auto_att_fwd;

core_pent_B, 0, 
  {object_repair, 0},
  {object_downlink, -165, 
    {component_prong, // component 1
      {object_pulse:auto_att_fwd, 818},
      {object_pulse:auto_att_fwd, 0},
      {object_uplink, 0},
      {object_move:auto_move, 2048},
    }
  },
  {object_move:auto_move, 946},
  {object_move:auto_move, -946},
  {object_downlink, 165, 
    {component_prong, // component 2
      {object_pulse:auto_att_fwd, 0},
      {object_pulse:auto_att_fwd, -818},
      {object_move:auto_move, -2048},
      {object_uplink, 0},
    }
  },

#code




// Process AI modes (these reflect the capabilities of the process)
enum
{
  MODE_WANDER,
  MODE_ATTACK,
  MODES
};


// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder as entry 0
  TARGET_MAIN, // main target
  TARGET_LEFT,
  TARGET_RIGHT
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


int initialised;
if (initialised == 0)
{
 initialised = 1;
 attack_mode(1);
 special_AI(0, 7);
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
   special_AI(1, 1);
// no break; falls through to attack case below
  }
   else
   {
    auto_move.move_to(move_x, move_y);
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

  auto_move.approach_target(TARGET_MAIN, 0, 700); // calls approach_target() on all objects in the auto_move class.
         // Parameters are:
         //  - target's address in targetting memory
         //  - component of target process to attack (0 means the core)
         //  - approach distance (in pixels)
  if (process[TARGET_MAIN].distance_less(1000))
    auto_att_fwd.fire_at(TARGET_MAIN, 0); // tries to fire one object in the auto_att_main class. 8 is firing delay (in ticks)
      // (here, the auto_att_main class is for fixed attack objects that point more or less forwards)
      // fire_1() can be better than just fire() because it spreads out the power use
  break; // end of case MODE_ATTACK
  
} // end of mode switch


// auto_att_left.attack_scan(-3000, 400, TARGET_LEFT); 
// auto_att_right.attack_scan(3000, 400, TARGET_RIGHT); 


//if (get_power_left() > 30)
  repair_self(); // tries to repair any damaged components

//if (get_power_left() > 30)
  restore_self(); // tries to restore any destroyed components




exit; // halts execution (until the next cycle)


start_wandering:
 mode = MODE_WANDER;
// set the target location anywhere that's not too close to the edge of the map:
 move_x = 800 + random(world_x() - 1600); // (world_x/y() return size of map
 move_y = 800 + random(world_y() - 1600);
 return; // jumps back to the statement just after the gosub
