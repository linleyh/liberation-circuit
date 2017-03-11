

#process "destroyer"

// This process has objects with the following auto classes:
class auto_att_main;
class auto_move;
class auto_retro;

// The following auto classes are not currently used by any objects:
class auto_att_fwd;
class auto_att_left;
class auto_att_right;
class auto_att_back;
class auto_att_spike;
class auto_harvest;
class auto_allocate;
class auto_stability;

core_pent_A, 0, 
  {object_repair, 0},
  {object_downlink, -1251, 
    {component_cap, // component 1
      {object_uplink, 0},
      {object_none, 0},
      {object_burst:auto_att_main, 1448},
      {object_burst:auto_att_main, 433},
    }
  },
  {object_downlink, 289, 
    {component_prong, // component 2
      {object_uplink, 0},
      {object_move:auto_move:auto_retro, -416},
      {object_move:auto_move, -1972},
      {object_move:auto_move, 1770},
    }
  },
  {object_downlink, -289, 
    {component_prong, // component 3
      {object_move:auto_move:auto_retro, 416},
      {object_uplink, 0},
      {object_move:auto_move, -1770},
      {object_move:auto_move, 1972},
    }
  },
  {object_downlink, 1251, 
    {component_cap, // component 4
      {object_burst:auto_att_main, -433},
      {object_burst:auto_att_main, -1448},
      {object_none, 0},
      {object_uplink, 0},
    }
  },
#code




// Process AI modes (these reflect the capabilities of the process)
enum
{
  MODE_IDLE, // process isn't doing anything ongoing
  MODE_MOVE, // process is moving to target_x, target_y
  MODE_MOVE_ATTACK, // process is moving, but will attack anything it finds along the way
  MODE_ATTACK, // process is attacking a target it was commanded to attack
  MODE_ATTACK_FOUND, // process is attacking a target it found itself
  MODE_GUARD, // process is circling a friendly process
  MODES
};

// Commands that the user may give the process
// (these are fixed and should not be changed, although not all processes accept all commands)
enum
{
  COM_NONE, // no command
  COM_LOCATION, // user has right-clicked somewhere on the display or map
  COM_TARGET, // user has right-clicked on an enemy process
  COM_FRIEND, // user has right-clicked on a friendly process
  COM_DATA_WELL // user has right-clicked on a data well
};

// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder in address 0
  TARGET_MAIN, // main target
  TARGET_GUARD, // target of guard command
};


// Variable declaration and initialisation
//  (note that declaration and initialisation cannot be combined)
//  (also, variables retain their values between execution cycles)
int core_x, core_y; // location of core
core_x = get_core_x(); // location is updated each cycle
core_y = get_core_y();
int angle; // direction process is pointing
 // angles are in integer degrees from 0 to 8192, with 0 being right,
 // 2048 down, 4096 left and 6144 (or -2048) up.
angle = get_core_angle(); // angle is updated each cycle

int mode; // what is the process doing? (should be one of the MODE enums)
int saved_mode; // save the process' mode while it's attacking something it found

int move_x, move_y; // destination
int target_x, target_y; // location of target (to attack, follow etc)
int circle_target; // targetting memory index of target being circled (used by the circle_around_target subroutine)
int circle_rotation; // direction to circle the target in (should be 1024 for clockwise, -1024 for anti-clockwise)
int circle_distance; // distance to maintain from the centre of the circle

int target_component; // target component for an attack command (allows user to
 // target specific components)

int scan_result; // used to hold the results of a scan of nearby processes

int self_destruct_primed; // counter for confirming self-destruct command (ctrl-right-click on self)
int initialised; // set to 1 after initialisation code below run the first time

if (!initialised)
{
   // initialisation code goes here (not all autocoded processes have initialisation code)
  initialised = 1;
  attack_mode(0); // attack objects (if present) will all fire together
  // move the process forward a bit to stop it obstructing the next process to be built
  mode = MODE_MOVE;
  move_x = core_x + cos(angle, 300);
  move_y = core_y + sin(angle, 300);
}

int verbose; // if 1, process will print various things to the console

if (check_selected_single()) // returns 1 if the user has selected this process (and no other processes)
{
  if (!verbose) printf("\nProcess selected.");
  verbose = 1;
  set_debug_mode(1); // 1 means errors for this process will be printed to the console. Resets to 0 each cycle
}
  else
    verbose = 0;


// Accept commands from user
if (check_new_command() == 1) // returns 1 if a command has been given
{
  switch(get_command_type()) // get_command_type() returns the type of command given
  {
    case COM_LOCATION:
    case COM_DATA_WELL: // this process can't harvest, so treat data well commands as location commands
      move_x = get_command_x(); // get_command_x() and ...y() return the target location of the command
      move_y = get_command_y();
      if (get_command_ctrl() == 0) // returns 1 if control was pressed when the command was given
      {
        mode = MODE_MOVE;
        if (verbose) printf("\nMoving.");
      }
        else
        {
          mode = MODE_MOVE_ATTACK; // will attack anything found along the way
          if (verbose) printf("\nAttack-moving.");
        }
      break;
    
    case COM_TARGET:
      get_command_target(TARGET_MAIN); // writes the target of the command to address TARGET_MAIN in targetting memory
       // (targetting memory stores the target and allows the process to examine it if it's in scanning range
       //  of any friendly process)
      mode = MODE_ATTACK;
      target_x = get_command_x();
      target_y = get_command_y();
      target_component = get_command_target_component(); // allows user to target a specific component
      if (verbose) printf("\nAttacking.");
      break;
    
    case COM_FRIEND:
      get_command_target(TARGET_GUARD); // writes the target of the command to address TARGET_GUARD in targetting memory
       // (targetting memory stores the target and allows the process to examine it if it's in scanning range)
      if (get_command_ctrl() && process[TARGET_GUARD].get_core_x() == get_core_x() && process[TARGET_GUARD].get_core_y() == get_core_y())
      {
        if (self_destruct_primed > 0)
        {
          printf("\nTerminating.");
          terminate; // this causes the process to self-destruct
        }
        printf("\nSelf destruct primed.");
        printf("\nRepeat command (ctrl-right-click self) to confirm.");
        self_destruct_primed = 20;
        break;
      }
      mode = MODE_GUARD;
      if (verbose) printf("\nGuarding.");
      break;
    
    default:
      if (verbose) printf("\nUnrecognised command.");
      break;
  
  } // end of command type switch
} // end of new command code


if (self_destruct_primed > 0)
{
  self_destruct_primed --;
  if (self_destruct_primed == 0
   && verbose)
    printf("\nSelf destruct cancelled.");
}


// What the process does next depends on its current mode
switch(mode)
{
  
  case MODE_IDLE:
    auto_move.set_power(0); // turn off all objects in the move class
    // now check for nearby hostile processes
    scan_result = scan_for_threat(0, 0, TARGET_MAIN); // scan_for_threat finds the hostile process nearest to the scan centre,
     // and saves it in the process' targetting memory.
     // (parameters are: (x offset of scan centre from core, y offset, targetting memory address))
    if (scan_result != 0)
    {
      mode = MODE_ATTACK_FOUND; // later code means that process will attack target in targetting memory 0
      target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN
      target_y = process[TARGET_MAIN].get_core_y();
      target_component = 0; // attack the core
      saved_mode = MODE_IDLE; // when leaving MODE_ATTACK_FOUND, will return to this mode
      if (verbose) printf("\nTarget found; attacking.");
    }
    break;
  
  case MODE_MOVE_ATTACK:
  // check for nearby hostile processes
    scan_result = scan_for_threat(0, 0, TARGET_MAIN); // scan_for_threat finds the hostile process nearest to the scan centre,
     // and saves it in the process' targetting memory.
     // (parameters are: (x offset of scan centre from core, y offset, targetting memory address))
    if (scan_result != 0)
    {
      mode = MODE_ATTACK_FOUND; // later code means that process will attack target in targetting memory 0
      target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN
      target_y = process[TARGET_MAIN].get_core_y();
      target_component = 0; // attack the core
      saved_mode = MODE_MOVE_ATTACK; // when leaving MODE_ATTACK_FOUND, will return to this mode
      if (verbose) printf("\nTarget found - attacking.");
      break;
    }
  // fall through to MODE_MOVE case...
  
  case MODE_MOVE:
  // stop moving when within 255 pixels of target (can change to higher or lower values if needed)
    if (distance_from_xy_less(move_x, move_y, 255))
    {
      clear_command(); // cancels the current command. If there's a queue of commands (e.g. shift-move waypoints)
       //  this moves the queue forward so that check_new_command() will return 1 next cycle.
      mode = MODE_IDLE;
      if (verbose) printf("\nReached destination.");
    }
      else
        auto_move.move_to(move_x, move_y); // calls move_to for all objects in the move class
    break;
  
  case MODE_ATTACK_FOUND:
  // Attack target as long as it's visible.
  // If target lost or destroyed, go back to previous action.
    // Now see whether the commanded target is visible:
    //  (targets are visible if within scanning range of any friendly process)
    if (!process[TARGET_MAIN].visible()) // returns zero if not target visible or doesn't exist
    {
      auto_move.move_to(target_x, target_y); // calls move_to for all objects in the move class
      if (distance_from_xy_less(target_x, target_y, 600))
      {
        // we should be able to see the target now, so it's either been destroyed
        // or gone out of range.
        mode = saved_mode; // the process goes back to what it was doing
        if (verbose) printf("\nTarget not detected.");
        break;
      }
    }
      else
      {
        target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN
        target_y = process[TARGET_MAIN].get_core_y();
        auto_move.approach_track(TARGET_MAIN,target_component,auto_att_main, 700);
         // approach_track() works out movement required to hit a target with a particular attacking object,
         //  and if retro move objects are available, also tries to maintain a certain distance.
         // It applies a simple target leading algorithm (which requires the attacking class to be identified)
         // Parameters are:
         //  - target's address in targetting memory
         //  - component of target process to attack
         //  - class of attacking object (the first object in the class will be used)
         //  - stand-off distance (in pixels)
        if (distance_from_xy_less(target_x, target_y, 1000)
         && arc_length(angle, process[TARGET_MAIN].target_angle()) < 500)
          auto_att_main.fire(0); // tries to fire all objects in the auto_att_main class. 0 is firing delay (in ticks)
         // (attack class is for fixed attack objects that point more or less forwards)
      }
    break;
  
  case MODE_ATTACK:
  // Attack target identified by user command
    // Now see whether the commanded target is visible:
    //  (targets are visible if within scanning range of any friendly process)
    if (!process[TARGET_MAIN].visible()) // returns zero if not target visible or doesn't exist
    {
      auto_move.move_to(target_x, target_y); // calls move_to for all objects in the move class
      if (distance_from_xy_less(target_x, target_y, 600))
      {
        // we should be able to see the target now, so it's either been destroyed
        // or gone out of range.
        mode = saved_mode; // the process goes back to what it was doing
        if (verbose) printf("\nTarget not detected.");
        break;
      }
    }
      else
      {
        target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN
        target_y = process[TARGET_MAIN].get_core_y();
        auto_move.approach_track(TARGET_MAIN,target_component,auto_att_main, 700);
         // approach_track() works out movement required to hit a target with a particular attacking object,
         //  and if retro move objects are available, also tries to maintain a certain distance.
         // It applies a simple target leading algorithm (which requires the attacking class to be identified)
         // Parameters are:
         //  - target's address in targetting memory
         //  - component of target process to attack
         //  - class of attacking object (the first object in the class will be used)
         //  - stand-off distance (in pixels)
        if (distance_from_xy_less(target_x, target_y, 1000)
         && arc_length(angle, process[TARGET_MAIN].target_angle()) < 500)
          auto_att_main.fire(0); // tries to fire all objects in the auto_att_main class. 0 is firing delay (in ticks)
         // (attack class is for fixed attack objects that point more or less forwards)
      }
    break;
  
  case MODE_GUARD:
  // Move in circle around friendly target identified by user command
  //  and attack any enemies that come near
    if (process[TARGET_GUARD].visible()) // returns 1 if target visible. Always returns 1 for a friendly target, if it exists.
    {
      // check for nearby hostile processes
      scan_result = scan_for_threat(0, 0, TARGET_MAIN); // scan_for_threat finds the hostile process nearest to the scan centre,
       // and saves it in the process' targetting memory.
       // (parameters are: (x offset of scan centre from core, y offset, targetting memory address))
      if (scan_result != 0)
      {
        mode = MODE_ATTACK_FOUND; // later code means that process will attack target in targetting memory 0
        target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN
        target_y = process[TARGET_MAIN].get_core_y();
        target_component = 0; // attack the core
        saved_mode = MODE_GUARD; // when leaving MODE_ATTACK_FOUND, will return to this mode
        if (verbose) printf("\nTarget found - attacking.");
        break;
      }
      // Now call the circle_around_target subroutine to make this process circle the guarded process
      circle_target = TARGET_GUARD; // circle_target is the process at the centre of the circle (here it's the process being guarded)
      circle_rotation = 1024; // the process will aim 1024 angle units (45 degrees) around the circle, clockwise.
      circle_distance = 700; // the process will try to stay this far from the centre of the circle
      gosub circle_around_target; // the circle_around_target subroutine is below, near the end of the code
      break;
    }
    // guard target must have been destroyed. Go back to idle mode and check for new commands.
    clear_command(); // cancels the current command. If there's a queue of commands (e.g. shift-move waypoints)
     //  this moves the queue forward so that check_new_command() will return 1 next cycle.
    mode = MODE_IDLE;
    if (verbose) printf("\nGuard target lost.");
    break;

} // end of mode switch

restore_self(); // tries to restore any destroyed components

repair_self(); // tries to repair any damaged components

exit; // stops execution, until the next cycle

// if there are any subroutines (called by gosub statements), they go here



circle_around_target: // this is a label for gosub statements
  // this subroutine makes the process circle around a target.
  // before this subroutine was called, circle_target should have been set to
  //  the target that the process will circle around. The target should be visible.
  // And circle_rotation should have been set to 1024 (clockwise - for guard commands)
  //  or -1024 (anticlockwise - for attack commands)
  int angle_to_circle_target, circle_move_x, circle_move_y;
  angle_to_circle_target = process[circle_target].target_angle();
  angle_to_circle_target += circle_rotation; // this will lead the process in a circle around the target
  circle_move_x = process[circle_target].get_core_x() // location of target
                  + (process[circle_target].get_core_speed_x() * 10) // if the target is moving, aim for a point a little ahead of it
                  - cos(angle_to_circle_target, circle_distance); // work out the radius of the circle around the target
  circle_move_y = process[circle_target].get_core_y()
                  + (process[circle_target].get_core_speed_y() * 10)
                  - sin(angle_to_circle_target, circle_distance);
  auto_move.move_to(circle_move_x, circle_move_y);
  return; // returns to the statement immediately after the gosub
