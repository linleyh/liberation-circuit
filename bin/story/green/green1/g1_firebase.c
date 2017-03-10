

#process "firebase"
class auto_att_fwd;
class auto_att_right;
class auto_att_left;

core_static_pent, 819, 
  {object_pulse_l:auto_att_fwd, -313},
  {object_pulse:auto_att_right, 0},
  {object_interface, 0},
  {object_pulse:auto_att_left, 0},
  {object_pulse_l:auto_att_fwd, 322},
#code




// Process AI modes (these reflect the capabilities of the process)
enum
{
  MODE_IDLE, // process isn't doing anything ongoing
  MODES
};

// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder in address 0
  TARGET_FRONT, // target of directional forward attack
  TARGET_LEFT, // target of directional left attack
  TARGET_RIGHT, // target of directional right attack
};



int initialised; // set to 1 after initialisation code below run the first time

if (!initialised)
{
   // initialisation code goes here (not all autocoded processes have initialisation code)
  initialised = 1;
  attack_mode(1); // attack objects in each class fire one by one
}

auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);
auto_att_left.attack_scan(-3048, 400, TARGET_LEFT);
auto_att_right.attack_scan(3048, 400, TARGET_RIGHT);

charge_interface_max(); // charges the process' interface. Since the interface is shared across all
 // components with interface objects, this call is not specific to any object or class.
 // charge_interface_max() charges the interface using as much power as possible
 // (the charge rate is determined by the maximum interface strength).

exit; // stops execution, until the next cycle
