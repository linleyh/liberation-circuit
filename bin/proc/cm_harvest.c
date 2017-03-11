

#process "harvester"

// This process has objects with the following auto classes:
class auto_harvest;
class auto_move;

// The following auto classes are not currently used by any objects:
class auto_retro;
class auto_att_main;
class auto_att_fwd;
class auto_att_left;
class auto_att_right;
class auto_att_back;
class auto_att_spike;
class auto_allocate;
class auto_stability;

core_quad_A, 4096, 
  {object_harvest:auto_harvest, 0},
  {object_move:auto_move, -2048},
  {object_storage, 0},
  {object_move:auto_move, 2048},
#code




// Process AI modes (these reflect the capabilities of the process)
enum
{
  MODE_IDLE, // process isn't doing anything ongoing
  MODE_MOVE, // process is moving to target_x, target_y
  MODE_HARVEST, // process is harvesting data from a data well (or travelling to do so)
  MODE_HARVEST_RETURN, // process has harvested data and is returning to an allocator
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
  TARGET_ALLOCATOR, // process that this process will return to when finished harvesting
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

int move_x, move_y; // destination
int target_x, target_y; // location of target (to attack, follow etc)
int circle_target; // targetting memory index of target being circled (used by the circle_around_target subroutine)
int circle_rotation; // direction to circle the target in (should be 1024 for clockwise, -1024 for anti-clockwise)
int circle_distance; // distance to maintain from the centre of the circle

int target_component; // target component for an attack command (allows user to
 // target specific components)

int scan_result; // used to hold the results of a scan of nearby processes

int self_destruct_primed; // counter for confirming self-destruct command (ctrl-right-click on self)

// Harvester variables
int data_well_x, data_well_y; // location of data well
int allocator_x, allocator_y; // location of process this process will return to with data
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
  // Harvester assumes that it was built by an allocator,
  //  and will return to its parent when full of harvested data.
  target_copy(TARGET_ALLOCATOR, TARGET_PARENT); // copies parent to TARGET_ALLOCATOR
  allocator_x = process[TARGET_ALLOCATOR].get_core_x();
  allocator_y = process[TARGET_ALLOCATOR].get_core_y();
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
      move_x = get_command_x(); // get_command_x() and ...y() return the target location of the command
      move_y = get_command_y();
      mode = MODE_MOVE;
      if (verbose) printf("\nMoving.");
      break;
    
    case COM_TARGET:
      if (verbose) printf("\nTarget command not recognised.");
      break;
    
    case COM_DATA_WELL:
      data_well_x = get_command_x();
      data_well_y = get_command_y();
       // (targetting memory stores the target and allows the process to examine it if it's in scanning range)
      if (mode != MODE_HARVEST_RETURN) // can reassign target data well while process returning with harvest without changing mode
        mode = MODE_HARVEST;
      if (verbose) printf("\nHarvesting from data well.");
      break;
    
    case COM_FRIEND:
      allocator_x = get_command_x();
      allocator_y = get_command_y();
      get_command_target(TARGET_ALLOCATOR); // writes the target of the command to address TARGET_ALLOCATOR in targetting memory
      if (get_command_ctrl() && process[TARGET_ALLOCATOR].get_core_x() == get_core_x() && process[TARGET_ALLOCATOR].get_core_y() == get_core_y())
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
      if (mode != MODE_HARVEST)
        mode = MODE_HARVEST_RETURN;
      if (verbose) printf("\nHarvester return set.");
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
    break;
  
  case MODE_MOVE:
  // stop moving when within 255 pixels of target (can change to higher or lower values if needed)
    if (distance_from_xy_less(move_x, move_y, 255))
    {
      clear_command(); // cancels the current command. If there's a queue of commands (e.g. shift-move waypoints)
       //  this moves the queue forward so that check_new_command() will return 1 next cycle.
      // Now, a command to move somewhere near a data well was probably meant as a harvest command:
      if (get_data_stored() < get_data_capacity()
       && search_for_well())
      {
        mode = MODE_HARVEST;
        data_well_x = get_well_x();
        data_well_y = get_well_y();
        if (verbose) printf("\nHarvesting.");
      }
        else
        {
          mode = MODE_IDLE;
          if (verbose) printf("\nReached destination.");
        }
    }
      else
        auto_move.move_to(move_x, move_y); // calls move_to for all objects in the move class
    break;
  
  case MODE_HARVEST:
    if (abs(core_x - data_well_x) < 1000 && abs(core_y - data_well_y) < 1000)
      auto_harvest.gather_data(); // only gather if near target (avoids inadvertently gathering from another well)
    auto_move.move_to(data_well_x, data_well_y);
    if (get_data_stored() == get_data_capacity() || get_damage() > 0)
    {
      mode = MODE_HARVEST_RETURN;
      if (verbose) printf("\nReturning to allocator.");
    }
    break;
  
  case MODE_HARVEST_RETURN:
    auto_harvest.give_data(TARGET_ALLOCATOR, 100);
    auto_move.move_to(allocator_x, allocator_y);
    if (get_data_stored() == 0
     && get_data_capacity() > 0) // if 0, probably means the harvester is damaged.
    {
      if (data_well_x != 0)
      {
        // if the harvester has a data well to harvest from, go back to harvest mode:
        mode = MODE_HARVEST;
        if (verbose) printf("\nHarvesting.");
      }
        else
          {
            // if data_well_x is 0, the harvester has probably just been built and sent a command to
            // guard its parent (which it may have interpreted as a command to set its harvest return target).
            // if so, it should just go into guard mode:
            mode = MODE_GUARD;
            target_copy(TARGET_GUARD, TARGET_ALLOCATOR); // copy from TARGET_ALLOCATOR to TARGET_GUARD
            if (verbose) printf("\nGuarding.");
          }
      }
    break;
  
  case MODE_GUARD:
  // Move in circle around friendly target identified by user command
    if (process[TARGET_GUARD].visible()) // returns 1 if target visible. Always returns 1 for a friendly target, if it exists.
    {
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
