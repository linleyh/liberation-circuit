

#process "minbase"

class auto_allocate;
class auto_att_back;
class auto_att_left;
class auto_harvest;
class auto_att_fwd;
class auto_att_right;

core_static_hex_B, 4112, 
  {object_repair, 0},
  {object_downlink, 0, 
    {component_prong, // component 1
      {object_interface, 0},
      {object_pulse_l:auto_att_left, 1308},
      {object_uplink, 0},
      {object_storage, 0},
    }
  },
  {object_allocate:auto_allocate, 0},
  {object_downlink, 0, 
    {component_peak, // component 2
      {object_uplink, 0},
      {object_storage, 0},
      {object_harvest:auto_harvest, 0},
      {object_pulse_xl:auto_att_fwd, 0},
      {object_storage, 0},
    }
  },
  {object_build, 0},
  {object_downlink, 0, 
    {component_prong, // component 3
      {object_pulse_l:auto_att_right, -950},
      {object_interface, 0},
      {object_interface, 0},
      {object_uplink, 0},
    }
  }

#code

// minimalist base.



enum
{
TARGET_PARENT, // TM 0 is set to parent process

TM_HARVESTER_0,
//TM_HARVESTER_1,
//TM_HARVESTER_2,
TM_BUILDER_0,
TM_COMMANDER_0,
TM_COMMANDER_1,
TM_COMMANDER_2,

TARGET_LEFT, // target of directional left attack
TARGET_RIGHT, // target of directional right attack
TARGET_FRONT, // target of directional front attack

TM_END
};

enum
{
TEMPLATE_BASE,
TEMPLATE_BUILDER,
TEMPLATE_HARVESTER,
TEMPLATE_COMMANDER,
TEMPLATE_COMMANDER2,
TEMPLATE_ATTACKER,
TEMPLATE_MINBASE,
TEMPLATE_SCOUT

};

enum
{
// These are the channels that processes in this stage
//  use to communicate with each other.
// There can be up to 8
CHANNEL_MAIN_BASE,
CHANNEL_TARGET,
CHANNEL_WELL,
CHANNEL_REQUEST_FOLLOWER,

};

enum
{
// these are codes for the first value of a broadcast message.
// they tell the recipient what kind of messge it is.
//  (the recipient needs the same enum declaration)
MESSAGE_MAIN_BASE, // the main base broadcasts a message which prevents other bases taking over as main base
MESSAGE_TARGET_FOUND,
MESSAGE_REQUEST_FOLLOWER,
MESSAGE_WELL_CLAIM
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
int broadcast_count;

int build_result; // build result code (returned by build call)
int build_target_index;

int initialised;

if (!initialised)
{
  i_am_main_base = -1; // need to wait one cycle to see whether another main base exists
  initialised = 1;
  listen_channel(CHANNEL_MAIN_BASE);
  special_AI(0, 205);
}







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

 if (special_build_counter < 3)
 {
  if (special_build_counter == 2)
   add_to_build_queue(TEMPLATE_SCOUT, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
    else
     add_to_build_queue(TEMPLATE_ATTACKER, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
  build_target_index = -1; // don't save these in targetting memory
  special_build_counter ++;
  goto finished_building;
 }

// check whether each entry in targetting memory has something:


 i = TM_HARVESTER_0;
 
 if (process[i].visible() <= 0) // doesn't exist (friendly processes are always visible if they exist)
 {
   add_to_build_queue(TEMPLATE_HARVESTER, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
   build_target_index = i;
   special_build_counter = 0;
   goto finished_building;
 }

 i = TM_BUILDER_0;
 
 if (process[i].visible() <= 0) // doesn't exist (friendly processes are always visible if they exist)
 {
   add_to_build_queue(TEMPLATE_BUILDER, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
   build_target_index = i;
   special_build_counter = 0;
   goto finished_building;
 }

 for (i = TM_COMMANDER_0; i < TM_END; i ++)
 {
  if (process[i].visible() <= 0) // doesn't exist (friendly processes are always visible if they exist)
  {
   add_to_build_queue(TEMPLATE_COMMANDER2, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
   build_target_index = i;
   special_build_counter = 0;
   goto finished_building;
  }
 }

}


finished_building:

// This base has few defences, so it periodically broadcasts its presence to any nearby followers:

if (broadcast_count <= 0)
{
// this process periodically broadcasts its presence to any nearby followers:
 broadcast(1000, // range of broadcast (in pixels).
           CHANNEL_REQUEST_FOLLOWER, // channel - follower processes in this mission listen to channel 5
           0, // priority - 0 or 1. This message doesn't need high priority (which replaces low priority messages)
           MESSAGE_REQUEST_FOLLOWER); // message contents
 broadcast_count = 50;
}
 else
  broadcast_count --;




if (get_damage() > 0)
 special_AI(1, 3);


//charge_interface_max(); // charges the process' interface. Since the interface is shared across all

charge_interface_max();

auto_att_fwd.attack_scan(0, 300, TARGET_FRONT);
auto_att_left.attack_scan(3000, 300, TARGET_LEFT);
auto_att_right.attack_scan(-3000, 300, TARGET_RIGHT);

restore_self();
repair_self();

auto_harvest.gather_data();

auto_allocate.allocate_data(4); // actually I think the maximum is 2. Doesn't matter
