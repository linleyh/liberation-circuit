

#process "spikebase"

class auto_att_spike;

core_static_hex_B, 0, 
  {object_spike:auto_att_spike, 0},
  {object_spike:auto_att_spike, 0},
  {object_spike:auto_att_spike, 0},
  {object_downlink, 0, 
    {component_fork, // component 1
      {object_uplink, 0},
      {object_interface, 0},
      {object_interface, 0},
    }
  },
  {object_spike:auto_att_spike, 0},
  {object_spike:auto_att_spike, 0}

#code




// Process AI modes (these reflect the capabilities of the process)
enum
{
  MODE_IDLE, // process isn't doing anything ongoing
  MODE_ATTACK, // process has something to attack
  MODES
};

// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder in address 0
  TARGET_MAIN, // main target
};



int initialised; // set to 1 after initialisation code below run the first time
int mode;

int target_x, target_y;
int bombard_count; // counter for firing at last known position of target that is no longer visible
int broadcast_count; // counter to prevent too much "target found" broadcast spam

int self_destruct_count; // if nothing happens for long enough, and there
 // are too many processes, will self-destruct.



if (!initialised)
{
   // initialisation code goes here (not all autocoded processes have initialisation code)
  initialised = 1;
  attack_mode(2); // attack objects in each class fire two by two
  listen_channel(1); // other processes send "target found" messages on broadcast channel 1
  special_AI(0, 102);
}


self_destruct_count ++;


// listen for messages (the listen_channel(1) call above allows broadcasts to be received)
if (next_message())
{
// In this story mission, we can assume that any message is a "target found" broadcast with the following format:
//  0: target_x
//  1: target_y
// + an attached target.
 target_x = read_message();
 target_y = read_message();
 get_message_target(TARGET_MAIN);
 mode = MODE_ATTACK;
 bombard_count = 0;
}



switch(mode)
{

 case MODE_ATTACK:
  if (process[TARGET_MAIN].visible())
  {
// save the target location so it can be used if the target goes out of visible range.
   target_x = process[TARGET_MAIN].get_core_x();
   target_y = process[TARGET_MAIN].get_core_y();
   auto_att_spike.fire_spike_at(TARGET_MAIN, 0);
   bombard_count = 0;
   if (process[TARGET_MAIN].distance_more(3000))
    mode = MODE_IDLE;
  }
   else
   {
    auto_att_spike.fire_spike_xy(target_x, target_y);
    bombard_count ++;
    if (bombard_count > 32)
     mode = MODE_IDLE;
   }
   break;

}

if (scan_for_threat(0, 0, TARGET_MAIN))
{
   target_x = process[TARGET_MAIN].get_core_x();
   target_y = process[TARGET_MAIN].get_core_y();
   auto_att_spike.fire_spike_at(TARGET_MAIN, 0);
   bombard_count = 0;
   mode = MODE_ATTACK;
// consider letting other nearby processes know that there is a target here:
   if (broadcast_count <= 0)
   {

     broadcast_target(3000, // range
                      1, // channel (see the listen_channel(1) call above)
                      0, // priority (0 just means a 1 priority message will overwrite this one)
                      TARGET_MAIN, // this target is attached to the broadcast. A listener can retrieve it with get_message_target().
                      target_x, // message contents. retrieved sequentially by read_message().
                      target_y); // message contents

     broadcast_count = 48;
   
   
   
   }

}


if (broadcast_count > 0)
 broadcast_count --;



if (self_destruct_count > 100
 && get_processes_unused() < 10)
  terminate; // self-destruct


charge_interface_max(); // charges the process' interface. Since the interface is shared across all
 // components with interface objects, this call is not specific to any object or class.
 // charge_interface_max() charges the interface using as much power as possible
 // (the charge rate is determined by the maximum interface strength).

exit; // stops execution, until the next cycle
