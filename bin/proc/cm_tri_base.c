

#process "tri_base"

// This process has objects with the following auto classes:
class auto_allocate;
class auto_att_fwd;
class auto_att_right;
class auto_att_back;
class auto_att_left;
class auto_harvest;

// The following auto classes are not currently used by any objects:
class auto_move;
class auto_retro;
class auto_att_main;
class auto_att_spike;
class auto_stability;

core_static_hex_A, 0, 
  {object_build, 0},
  {object_downlink, -878, 
    {component_prong, // component 1
      {object_pulse:auto_att_fwd, -727},
      {object_downlink, 913, 
        {component_fork, // component 2
          {object_none, 0},
          {object_pulse:auto_att_right, 0},
          {object_uplink, 0},
        }
      },
      {object_storage, 0},
      {object_uplink, 0},
    }
  },
  {object_repair, 0},
  {object_downlink, -831, 
    {component_prong, // component 3
      {object_pulse:auto_att_back, -832},
      {object_downlink, 781, 
        {component_fork, // component 4
          {object_none, 0},
          {object_pulse:auto_att_back, 0},
          {object_uplink, 0},
        }
      },
      {object_storage, 0},
      {object_uplink, 0},
    }
  },
  {object_allocate:auto_allocate, 0},
  {object_downlink, -865, 
    {component_prong, // component 5
      {object_pulse:auto_att_left, -546},
      {object_downlink, 876, 
        {component_fork, // component 6
          {object_none, 0},
          {object_pulse:auto_att_fwd, 0},
          {object_uplink, 0},
        }
      },
      {object_harvest:auto_harvest, 0},
      {object_uplink, 0},
    }
  }
#code




// Process AI modes (these reflect the capabilities of the process)
enum
{
  MODE_IDLE, // process isn't doing anything ongoing
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
  TARGET_FRONT, // target of directional forward attack
  TARGET_LEFT, // target of directional left attack
  TARGET_RIGHT, // target of directional right attack
  TARGET_BACK, // target of directional backwards attack
  TARGET_BUILT, // processes built by this process
  TARGET_ALLOCATOR, // process that this process will return to when finished harvesting
  TARGET_SELF_CHECK, // check against self for self-destruct commands
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

int front_attack_primary; // is set to 1 if forward directional attack objects are attacking
 // the primary target (e.g. target selected by command). Set to 0 if the objects are available for autonomous fire.
int target_component; // target component for an attack command (allows user to
 // target specific components)

int scan_result; // used to hold the results of a scan of nearby processes

int self_destruct_primed; // counter for confirming self-destruct command (ctrl-right-click on self)

// builder variables
int build_result; // build result code (returned by build call)
int initialised; // set to 1 after initialisation code below run the first time

if (!initialised)
{
   // initialisation code goes here (not all autocoded processes have initialisation code)
  initialised = 1;
  attack_mode(0); // attack objects (if present) will all fire together
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
      if (verbose) printf("\nLocation command will be sent to new processes.");
      break;
    
    case COM_TARGET:
      if (verbose) printf("\nTarget command will be sent to new processes.");
      break;
    
    case COM_FRIEND:
      get_command_target(TARGET_SELF_CHECK); // writes the target of the command to address TARGET_SELF_CHECK in targetting memory
      if (get_command_ctrl() && process[TARGET_SELF_CHECK].get_core_x() == get_core_x() && process[TARGET_SELF_CHECK].get_core_y() == get_core_y())
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
      if (verbose) printf("\nFriendly target command will be sent to new processes.");
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


// Now try to build a new process from the build queue.
// This only does anything if a build command for this process is at the front of the queue.
// Otherwise, it will just fail.
if (build_from_queue(TARGET_BUILT) == 1) // returns 1 on success
  copy_commands(TARGET_BUILT); // copies builder's command queue to newly built process




auto_harvest.gather_data();


auto_allocate.allocate_data(4); // actually I think the maximum is 2


front_attack_primary = 0; // this will be set to 1 if front attack is attacking the main target


// What the process does next depends on its current mode
switch(mode)
{
  
  case MODE_IDLE:
    break;

} // end of mode switch

if (front_attack_primary == 0) // is 1 if the forward attack objects are attacking the main target
{
  auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);
}

auto_att_left.attack_scan(-2048, 400, TARGET_LEFT);

auto_att_right.attack_scan(2048, 400, TARGET_RIGHT);

auto_att_back.attack_scan(4096, 400, TARGET_BACK);

restore_self(); // tries to restore any destroyed components

repair_self(); // tries to repair any damaged components

exit; // stops execution, until the next cycle

// if there are any subroutines (called by gosub statements), they go here
