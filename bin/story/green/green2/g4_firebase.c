

#process "firebase"

class auto_att_fwd;
class auto_att_right;
class auto_att_left;
class auto_att_back;

core_static_hex_A, 1400, 
  {object_pulse:auto_att_right, -313},
  {object_pulse_l:auto_att_right, 0},
  {object_downlink, 0, 
    {component_fork, // component 1
      {object_uplink, 0},
      {object_interface, 0},
      {object_interface, 0},
    }
  },
  {object_pulse_l:auto_att_left, 0},
  {object_pulse:auto_att_fwd, 322},
  {object_pulse_l:auto_att_fwd, 0}

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
  TARGET_BACK
};



int initialised; // set to 1 after initialisation code below run the first time
int self_destruct_count; // if nothing happens for long enough, and there
 // are too many processes, will self-destruct.

int broadcast_sent;

if (!initialised)
{
   // initialisation code goes here (not all autocoded processes have initialisation code)
  initialised = 1;
  attack_mode(1); // attack objects in each class fire one by one
  special_AI(0, 101);
}

self_destruct_count ++;

broadcast_sent = 0;

if (auto_att_fwd.attack_scan(0, 400, TARGET_FRONT)) // returns 1 if something found to attack
{
     broadcast_target(4000, // range
                      1, // channel
                      0, // priority (0 just means a 1 priority message will overwrite this one)
                      TARGET_FRONT, // this target is attached to the broadcast. A listener can retrieve it with get_message_target().
                      process[TARGET_FRONT].get_core_x(), // message contents. retrieved sequentially by read_message().
                      process[TARGET_FRONT].get_core_y()); // message contents
     broadcast_sent = 1;
 self_destruct_count = 0;
} 
 
if (auto_att_left.attack_scan(-2048, 400, TARGET_LEFT))
{
  if (!broadcast_sent)
  {
     broadcast_target(4000, // range
                      1, // channel
                      0, // priority (0 just means a 1 priority message will overwrite this one)
                      TARGET_LEFT, // this target is attached to the broadcast. A listener can retrieve it with get_message_target().
                      process[TARGET_LEFT].get_core_x(), // message contents. retrieved sequentially by read_message().
                      process[TARGET_LEFT].get_core_y()); // message contents
     broadcast_sent = 1;
  }
 self_destruct_count = 0;
} 
 
if (auto_att_right.attack_scan(2048, 400, TARGET_RIGHT))
{
  if (!broadcast_sent)
  {
     broadcast_target(4000, // range
                      1, // channel
                      0, // priority (0 just means a 1 priority message will overwrite this one)
                      TARGET_RIGHT, // this target is attached to the broadcast. A listener can retrieve it with get_message_target().
                      process[TARGET_RIGHT].get_core_x(), // message contents. retrieved sequentially by read_message().
                      process[TARGET_RIGHT].get_core_y()); // message contents
     broadcast_sent = 1;
  }
} 

if (auto_att_back.attack_scan(4096, 400, TARGET_BACK))
{
  if (!broadcast_sent)
  {
     broadcast_target(4000, // range
                      1, // channel
                      0, // priority (0 just means a 1 priority message will overwrite this one)
                      TARGET_BACK, // this target is attached to the broadcast. A listener can retrieve it with get_message_target().
                      process[TARGET_BACK].get_core_x(), // message contents. retrieved sequentially by read_message().
                      process[TARGET_BACK].get_core_y()); // message contents
     broadcast_sent = 1;
  }
} 

if (self_destruct_count > 100
 && get_processes_unused() < 10)
  terminate; // self-destruct


charge_interface_max(); // charges the process' interface. Since the interface is shared across all
 // components with interface objects, this call is not specific to any object or class.
 // charge_interface_max() charges the interface using as much power as possible
 // (the charge rate is determined by the maximum interface strength).

exit; // stops execution, until the next cycle
