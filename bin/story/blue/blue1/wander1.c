

#process "wander1"

class auto_att_fwd;
class auto_move;

core_quad_A, 0, 
  {object_pulse:auto_att_fwd, 0},
  {object_move:auto_move, 2048},
  {object_none, 0},
  {object_move:auto_move, -2048},

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

  auto_move.approach_target(TARGET_MAIN, 0, 300); // approach to within 300 pixels of target's core

  if (process[TARGET_MAIN].distance_less(1000))
    auto_att_fwd.fire_at(TARGET_MAIN, 0); 

  break;
  
} // end of mode switch



exit;


start_wandering:
 mode = MODE_WANDER;
 move_x = 800 + random(world_x() - 1600);
 move_y = 800 + random(world_y() - 1600);
 auto_att_fwd.no_target();
 return;
