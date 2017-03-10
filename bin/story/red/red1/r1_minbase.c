

#process "minbase"

class auto_allocate;
class auto_att_fwd;
class auto_att_right;
class auto_harvest;
class auto_att_back;
class auto_att_left;

core_static_hex_B, 0, 
  {object_downlink, 0, 
    {component_peak, // component 1
      {object_uplink, 0},
      {object_downlink, 0, 
        {component_prong, // component 2
          {object_none, 0},
          {object_uplink, 0},
          {object_pulse_l:auto_att_left, 0},
          {object_interface, 0},
        }
      },
      {object_none, 0},
      {object_pulse_l:auto_att_fwd, 0},
      {object_storage, 0},
    }
  },
  {object_allocate:auto_allocate, 0},
  {object_downlink, 0, 
    {component_peak, // component 3
      {object_uplink, 0},
      {object_downlink, 0, 
        {component_prong, // component 4
          {object_none, 0},
          {object_uplink, 0},
          {object_pulse_l:auto_att_right, 0},
          {object_interface, 0},
        }
      },
      {object_none, 0},
      {object_pulse_l:auto_att_right, 0},
      {object_storage, 0},
    }
  },
  {object_build, 0},
  {object_downlink, 0, 
    {component_peak, // component 5
      {object_uplink, 0},
      {object_downlink, 0, 
        {component_prong, // component 6
          {object_none, 0},
          {object_uplink, 0},
          {object_pulse_l:auto_att_back, 0},
          {object_interface, 0},
        }
      },
      {object_harvest:auto_harvest, 0},
      {object_pulse_l:auto_att_back, 0},
      {object_storage, 0},
    }
  },
  {object_repair_other, 0}

#code

// minimalist base.



enum
{
TARGET_PARENT, // TM 0 is set to parent process

TM_HARVESTER_0,
TM_HARVESTER_1,
//TM_HARVESTER_2,
TM_BUILDER_0,
TM_COMMANDER_0,
TM_COMMANDER_1,
TM_COMMANDER_2,

TARGET_LEFT, // target of directional left attack
TARGET_RIGHT, // target of directional right attack
TARGET_FRONT, // target of directional front attack
TARGET_BACK,

TM_END
};

enum
{
TEMPLATE_BASE,
TEMPLATE_BUILDER,
//TEMPLATE_HARVESTER,
TEMPLATE_COMMANDER,
TEMPLATE_ATTACKER,
TEMPLATE_MINBASE,
TEMPLATE_SCOUT,
TEMPLATE_ATTACKER2

};

enum
{
// These are the channels that processes in this stage
//  use to communicate with each other.
// There can be up to 8
CHANNEL_MAIN_BASE,
CHANNEL_TARGET_FOUND

};

enum
{
// these are codes for the first value of a broadcast message.
// they tell the recipient what kind of messge it is.
//  (the recipient needs the same enum declaration)
MESSAGE_MAIN_BASE, // the main base broadcasts a message which prevents other bases taking over as main base
MESSAGE_TARGET_FOUND
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

int i_am_main_base; // is 5 if this is the main base.

int build_result; // build result code (returned by build call)
int build_target_index;

int initialised;

if (!initialised)
{
  i_am_main_base = -1; // need to wait one cycle to see whether another main base exists
  initialised = 1;
  listen_channel(CHANNEL_MAIN_BASE);
  attack_mode(1);
  special_AI(0, 503);
}



// This base has few defences, so it periodically broadcasts its presence to any nearby followers:

int broadcast_count;

if (broadcast_count <= 0)
{

 broadcast(1000, // range of broadcast (in pixels).
           5, // channel - follower processes in this mission listen to channel 5
           0, // priority - 0 or 1. This message doesn't need high priority (which replaces low priority messages)
           40, // message contents - code 40 means "please follow me"
           0); // message contents (can be up to 8 parameters here, but this is a simple message)
 broadcast_count = 100;
}
 else
  broadcast_count --;



// work out whether this is the main base or not:

// First listen for another base's announcement that it is the main base:
if (next_message() // returns 1 if messages received since last cycle
 && read_message() == MESSAGE_MAIN_BASE) // read_message() reads the message contents sequentially
{ 
  i_am_main_base = 0;
  cancel_build_queue();
}
 else
 {
// There may not be another main base.

   i_am_main_base ++;

   if (i_am_main_base > 5)
   {
   
    i_am_main_base = 5;

    broadcast(-1, // range of broadcast (-1 is unlimited range).
              CHANNEL_MAIN_BASE, // channel
              1, // priority - 0 or 1. (1 replaces 0 priority messages)
              MESSAGE_MAIN_BASE); // message contents (can be up to 8 parameters here, but this is a simple message)
              
   }
 
 }

if (i_am_main_base < 5)
 goto finished_building;


// If this is the main base, it's responsible for building all new processes:


int i;

int special_build_counter;

// Try to build from the build queue, after checking whether there's anything on the queue:
if (!check_build_queue()
  || build_from_queue(build_target_index) == 1) // Returns 1 and saves new process to build_target_index on success.
{

 if (special_build_counter < 6)
 {
  if (special_build_counter == 2)
   add_to_build_queue(TEMPLATE_SCOUT, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
    else
    { 
     if (get_process_count() < 20)
      add_to_build_queue(TEMPLATE_ATTACKER, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
       else
        add_to_build_queue(TEMPLATE_ATTACKER2, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
    } 
  build_target_index = -1; // don't save these in targetting memory
  special_build_counter ++;
  goto finished_building;
 }

// check whether each entry in targetting memory has something:
// for (i = TM_HARVESTER_0; i < TM_BUILDER_0; i ++)

 i = TM_BUILDER_0;
 
 {
  if (process[i].visible() <= 0) // doesn't exist (friendly processes are always visible if they exist)
  {
   add_to_build_queue(TEMPLATE_BUILDER, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
   build_target_index = i;
   special_build_counter = 0;
   goto finished_building;
  }
 }

 for (i = TM_COMMANDER_0; i < TM_END; i ++)
 {
  if (process[i].visible() <= 0) // doesn't exist (friendly processes are always visible if they exist)
  {
   add_to_build_queue(TEMPLATE_COMMANDER, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
   build_target_index = i;
   special_build_counter = 0;
   goto finished_building;
  }
 }

}


finished_building:



charge_interface_max(); // charges the process' interface. Since the interface is shared across all

if (get_total_integrity() < get_unharmed_integrity_max() - 400)
 special_AI(1, 3);

auto_att_left.attack_scan(-2000, 400, TARGET_LEFT);
auto_att_right.attack_scan(2000, 400, TARGET_RIGHT);
auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);
auto_att_back.attack_scan(4096, 400, TARGET_BACK);


restore_scan(0,0);
repair_scan(0,0);
restore_self(); // tries to restore any destroyed components
repair_self(); // tries to repair any damaged components


auto_harvest.gather_data();

auto_allocate.allocate_data(4); // actually I think the maximum is 2. Doesn't matter
