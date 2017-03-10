

#process "outpost"

class auto_allocate;
class auto_harvest;
class auto_att_fwd;
class auto_att_right;
class auto_att_left;
class auto_att_back;

core_static_hex_C, 6840, 
  {object_build, 0},
  {object_downlink, 0, 
    {component_drop, // component 1
      {object_uplink, 0},
      {object_downlink, 0, 
        {component_cap, // component 2
          {object_none, 0},
          {object_none, 0},
          {object_interface, 0},
          {object_uplink, 0},
        }
      },
      {object_downlink, 0, 
        {component_cap, // component 3
          {object_uplink, 0},
          {object_interface, 0},
          {object_none, 0},
          {object_storage, 0},
        }
      },
      {object_stream_dir:auto_att_fwd, 0},
      {object_pulse_l:auto_att_right, 639},
      {object_pulse_l:auto_att_left, -735}
    }
  },
  {object_repair_other, 0},
  {object_allocate:auto_allocate, 0},
  {object_downlink, 0, 
    {component_drop, // component 4
      {object_uplink, 0},
      {object_downlink, 0, 
        {component_cap, // component 5
          {object_none, 0},
          {object_none, 0},
          {object_interface, 0},
          {object_uplink, 0},
        }
      },
      {object_downlink, 0, 
        {component_cap, // component 6
          {object_uplink, 0},
          {object_interface, 0},
          {object_none, 0},
          {object_storage, 0},
        }
      },
      {object_pulse_xl:auto_att_back, 0},
      {object_pulse_l:auto_att_left, 667},
      {object_pulse_l:auto_att_right, -707}
    }
  },
  {object_harvest:auto_harvest, 0}

#code

enum
{
ESCORTS = 8
};

enum
{
TARGET_PARENT, // TM 0 is set to parent process

TM_HARVESTER,
//TM_HARVESTER_1,
//TM_HARVESTER_2,
TM_BUILDER,
TM_FLAGSHIP,
TM_ESCORT_0,
TM_ESCORT_1,
TM_ESCORT_2,
TM_ESCORT_3,
TM_ESCORT_4,
TM_ESCORT_5,
TM_ESCORT_6,
TM_ESCORT_7,


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
TEMPLATE_HARVESTER,
TEMPLATE_OUTPOST,

TEMPLATE_FLAGSHIP1,
TEMPLATE_FLAGSHIP2,

TEMPLATE_ESCORT,
TEMPLATE_PICKET


};

enum
{
// These are the channels that processes in this stage
//  use to communicate with each other.
// There can be up to 8
CHANNEL_MAIN_BASE,
CHANNEL_FLAGSHIP,
CHANNEL_TARGET,
CHANNEL_WELL,
CHANNEL_REQUEST_FOLLOWER,
CHANNEL_HELP

};

enum
{
// these are codes for the first value of a broadcast message.
// they tell the recipient what kind of messge it is.
//  (the recipient needs the same enum declaration)
MESSAGE_MAIN_BASE, // the main base broadcasts a message which prevents other bases taking over as main base
MESSAGE_TARGET_FOUND,
MESSAGE_FLAGSHIP,
MESSAGE_WELL_CLAIM,
MESSAGE_FORMATION,
MESSAGE_REQUEST_FOLLOWER,
MESSAGE_UNDER_ATTACK
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
int attacking;

int build_result; // build result code (returned by build call)
int build_target_index;
int picket_count;
int builder_count;

int broadcast_count;

int initialised;
int cycles;
cycles ++;

if (!initialised)
{
  i_am_main_base = -1; // need to wait one cycle to see whether another main base exists
  initialised = 1;
  listen_channel(CHANNEL_MAIN_BASE);
  special_AI(0, 403);
}


// work out whether this is the main base or not:
//  (we do this even for template 0 because the same code is used for both 0 and the other base)

// First listen for another base's announcement that it is the main base:
if (next_message() // returns 1 if messages received since last cycle
 && read_message() == MESSAGE_MAIN_BASE) // read_message() reads the message contents sequentially
{ 
  i_am_main_base = 0;
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


int i;

int special_build_counter;
int number_of_escorts;
int escorts_needed;

// try to build from queue (if the queue's empty this will just fail)
build_result = build_from_queue(build_target_index); // Returns 1 and saves new process to build_target_index on success.

// Try to build from the build queue, after checking whether there's anything on the queue:
if (!check_build_queue()
  || build_result == 1)
{

// if the new process is an escort, send it its position in the flagship formation
 if (build_result == 1
  && build_target_index >= TM_ESCORT_0
  && build_target_index <= TM_ESCORT_7)
 {
  int formation_pos, formation_angle, formation_dist;
  formation_pos = build_target_index - TM_ESCORT_0;
  if (formation_pos & 1)
  {
   formation_angle = 200 + formation_pos * 400;
   formation_dist = 800 - formation_pos * 50;
  }
   else
   {
    formation_angle = -400 - formation_pos * 400;
    formation_dist = 800 - formation_pos * 50;
   }

  transmit(build_target_index, 1, 
   MESSAGE_FORMATION, 
   formation_angle,
   formation_dist); 
// the new process will be able to read the transmission in its first cycle.  
 }



// Or a harvester
 if (process[TM_HARVESTER].visible() <= 0) // doesn't exist (friendly processes are always visible if they exist)
 {
   add_to_build_queue(TEMPLATE_HARVESTER, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
   build_target_index = TM_HARVESTER;
   goto finished_building;
 }


// If this is not the main base, build pickets:
 if (i_am_main_base < 5)
 {
   if (picket_count <= 0)
   {
    add_to_build_queue(TEMPLATE_PICKET, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
    picket_count = 40;
   } 
    else
     picket_count --;
   goto finished_building;
 }


// maybe try to build a builder:
 if (process[TM_BUILDER].visible() <= 0) // doesn't exist (friendly processes are always visible if they exist)
 {
   if (builder_count <= 0)
   {
    add_to_build_queue(TEMPLATE_BUILDER, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
    build_target_index = TM_BUILDER;
    builder_count = 50;
    goto finished_building;
   }
    else
     builder_count --; 
 }

// Now try to build escorts:


 number_of_escorts = 0;
 int free_escort;
 free_escort = -1;
 
 for (i = 0; i < ESCORTS; i ++)
 {
  if (process[TM_ESCORT_0 + i].visible())
   number_of_escorts ++;
    else
     free_escort = TM_ESCORT_0 + i;
 }

 escorts_needed = 3 + (cycles / 1000);
 
 if (escorts_needed > ESCORTS)
  escorts_needed = ESCORTS;

 if (number_of_escorts < escorts_needed)
 {
  add_to_build_queue(TEMPLATE_ESCORT, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
  if (free_escort != -1)
   build_target_index = free_escort;
    else
     build_target_index = TM_ESCORT_0; 

  goto finished_building;
 }

// finally, consider building a flagship:

 if (!process[TM_FLAGSHIP].visible())
 {
  add_to_build_queue(TEMPLATE_FLAGSHIP2, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
  build_target_index = TM_FLAGSHIP;
 } 

}


finished_building:


charge_interface_max(); // charges the process' interface. Since the interface is shared across all
 // components with interface objects, this call is not specific to any object or class.
 // charge_interface_max() charges the interface using any power left over.
 
attacking = 0; 

attacking += auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);
attacking += auto_att_back.attack_scan(4096, 400, TARGET_BACK);
attacking += auto_att_left.attack_scan(-2000, 400, TARGET_LEFT);
attacking += auto_att_right.attack_scan(2000, 400, TARGET_RIGHT);

if (attacking > 0)
{
 special_AI(1, 0);
 broadcast_count --;
 if (broadcast_count <= 0)
 {
  broadcast_count = 50;
  broadcast(9000,
            CHANNEL_HELP,
            0,
            MESSAGE_UNDER_ATTACK);
 }
} 


restore_scan(0,0);
repair_scan(0,0);
restore_self(); // tries to restore any destroyed components
repair_self(); // tries to repair any damaged components


auto_harvest.gather_data();

auto_allocate.allocate_data(4); // actually I think the maximum is 2. Doesn't matter
