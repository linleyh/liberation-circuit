

#process "defence"

class auto_stability;
class auto_att_fwd;

core_static_hex_B, 0, 
  {object_downlink, 0, 
    {component_peak, // component 6
      {object_uplink, 0},
      {object_stability:auto_stability, 0},
      {object_none, 0},
      {object_pulse_xl:auto_att_fwd, 0},
      {object_pulse_xl:auto_att_fwd, 0},
    }
  },
  {object_downlink, 333, 
    {component_cap, // component 2
      {object_stability:auto_stability, 0},
      {object_uplink, 0},
      {object_none, 0},
      {object_slice:auto_att_fwd, 0},
    }
  },
  {object_downlink, 340, 
    {component_cap, // component 1
      {object_none, 0},
      {object_uplink, 0},
      {object_none, 0},
      {object_none, 0},
    }
  },
  {object_downlink, 0, 
    {component_long4, // component 3
      {object_uplink, 0},
      {object_interface, 0},
      {object_interface, -32},
      {object_interface, 0},
    }
  },
  {object_downlink, -340, 
    {component_cap, // component 4
      {object_none, 0},
      {object_none, 0},
      {object_uplink, 0},
      {object_none, 0},
    }
  },
  {object_downlink, -333, 
    {component_cap, // component 5
      {object_slice:auto_att_fwd, 0},
      {object_none, 0},
      {object_uplink, 0},
      {object_stability:auto_stability, 0},
    }
  }

#code




// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder in address 0
  TARGET_MAIN, // main target
  TARGET_FRONT, // target of directional forward attack
};

/*
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
*/
int initialised; // set to 1 after initialisation code below run the first time

if (!initialised)
{
   // initialisation code goes here (not all autocoded processes have initialisation code)
  initialised = 1;
  attack_mode(1); // attack objects (if present) will all fire together
  auto_stability.set_stability(1);
}



  auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);

charge_interface_max(); // charges the process' interface. Since the interface is shared across all
 // components with interface objects, this call is not specific to any object or class.
 // charge_interface_max() charges the interface using as much power as possible
 // (the charge rate is determined by the maximum interface strength).

exit; // stops execution, until the next cycle

// if there are any subroutines (called by gosub statements), they go here
