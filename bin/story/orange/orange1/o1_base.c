


#process "base"

class auto_allocate;
class auto_att_fwd;
class auto_att_right;
class auto_att_back;
class auto_att_left;
class auto_harvest;

core_static_hex_C, 1400, 
  {object_build, 0},
  {object_downlink, -57, 
    {component_box, // component 1
      {object_uplink, 0},
      {object_downlink, -588, 
        {component_bowl, // component 2
          {object_storage, 0},
          {object_none, 0},
          {object_uplink, 0},
          {object_none, 0},
          {object_stream_dir:auto_att_right, -609},
        }
      },
      {object_slice:auto_att_right, 0},
      {object_downlink, 862, 
        {component_bowl, // component 3
          {object_harvest:auto_harvest, 0},
          {object_uplink, 0},
          {object_storage, 0},
          {object_downlink, 1610, 
            {component_cap, // component 4
              {object_none, 0},
              {object_storage, 0},
              {object_none, 0},
              {object_uplink, 0},
            }
          },
          {object_stream_dir:auto_att_back, -1420},
        }
      },
    }
  },
  {object_repair_other, 0},
  {object_downlink, 0, 
    {component_box, // component 5
      {object_uplink, 0},
      {object_interface, 0},
      {object_downlink, -1013, 
        {component_bowl, // component 6
          {object_build, 0},
          {object_none, 0},
          {object_uplink, 0},
          {object_stream_dir:auto_att_back, 0},
          {object_none, 0},
        }
      },
      {object_downlink, -791, 
        {component_bowl, // component 7
          {object_uplink, 0},
          {object_none, 0},
          {object_none, 0},
          {object_none, 0},
          {object_slice:auto_att_left, -1054},
        }
      },
    }
  },
  {object_allocate:auto_allocate, 0},
  {object_downlink, 9, 
    {component_side, // component 8
      {object_uplink, 0},
      {object_downlink, 1012, 
        {component_bowl, // component 9
          {object_none, 0},
          {object_uplink, 0},
          {object_none, 0},
          {object_none, 0},
          {object_stream_dir:auto_att_fwd, 0},
        }
      },
      {object_interface, 0},
      {object_downlink, -836, 
        {component_prong, // component 10
          {object_none, 0},
          {object_uplink, 0},
          {object_stream_dir:auto_att_left, -801},
          {object_storage, 0},
        }
      },
      {object_interface, 0},
      {object_interface, 0}
    }
  }

#code

enum
{
HARVESTERS = 6
};

enum
{
TARGET_PARENT, // TM 0 is set to parent process

// base builds up to 6 harvesters
TARGET_HARVESTER_0,
TARGET_HARVESTER_1,
TARGET_HARVESTER_2,
TARGET_HARVESTER_3,
TARGET_HARVESTER_4,
TARGET_HARVESTER_5,


TARGET_LEFT, // target of directional left attack
TARGET_RIGHT, // target of directional right attack
TARGET_FRONT, // target of directional front attack
TARGET_BACK,

TARGET_MESSAGE // scratch entry

};

enum
{
TEMPLATE_BASE,
TEMPLATE_DEFENCE,
TEMPLATE_HARVESTER_1,
TEMPLATE_HARVESTER_2,
TEMPLATE_HARVESTER_3,

TEMPLATE_GUARD_1,
TEMPLATE_GUARD_2,


};

enum
{
// These are the channels that processes in this stage
//  use to communicate with each other.
// There can be up to 8
CHANNEL_HARVESTER, // used by harvesters
CHANNEL_TARGET, // used for TARGET_FOUND messages
CHANNEL_HELP, // used for UNDER_ATTACK messages

};

enum
{
// these are codes for the first value of a broadcast message.
// they tell the recipient what kind of messge it is.
//  (the recipient needs the same enum declaration)
MESSAGE_NEXT_WELL_PLEASE, // harvester asks main base where to go
MESSAGE_GO_TO_WELL, // base tells harvester where to go
MESSAGE_WELL_FOUND,
MESSAGE_WELL_FOUND_ACK, // base acknowledged well found message
MESSAGE_SEEK_WELLS, // base tells harvester to look for new wells
MESSAGE_TARGET_FOUND,
MESSAGE_UNDER_ATTACK,
MESSAGE_REQUEST_FOLLOWER
};



// Variable declaration and initialisation
//  (note that declaration and initialisation cannot be combined)
//  (also, variables retain their values between execution cycles)


// this base keeps a record of all data wells:
enum
{
DATA_WELLS = 24
};

int data_well_x [DATA_WELLS]; // 0 means well entry empty
int data_well_y [DATA_WELLS];
int data_well_time [DATA_WELLS]; // last time a harvester was despatched to this well
int known_wells;



int core_x, core_y; // location of core
core_x = get_core_x(); // location is updated each cycle
core_y = get_core_y();
int angle; // direction process is pointing
 // angles are in integer degrees from 0 to 8192, with 0 being right,
 // 2048 down, 4096 left and 6144 up.
angle = get_core_angle(); // angle is updated each cycle

int build_result; // build result code (returned by build call)
int build_target_index;

int guard_build;

int initialised;

int cycles;
cycles ++;

if (!initialised)
{
  initialised = 1;
//  listen_channel(CHANNEL_MAIN_BASE);
  special_AI(0, 300);
  
// save nearby data well (can assume that it exists):
 data_well_x [0] = get_well_x();
 data_well_y [0] = get_well_y();
 
 known_wells = 1;
  
}

data_well_time [0] = cycles; // nearby well is always taken

int i;

int new_well_x;
int new_well_y;
int well_already_known;

int target_well_index;
int target_well_time;

     int adjusted_well_time;
     int adjusted_target_well_time;


  while(next_message())
  {
  
// it's possible to run out of instructions while processing messages.
//  let's try to avoid this.
   if (get_instructions_left() < 900)
    goto finished_building;
  
   switch(read_message())
   {

    case MESSAGE_WELL_FOUND:
// format of MESSAGE_WELL_FOUND is type, x, y
//printf("\n wfA %i", get_instructions_left());

     new_well_x = read_message();
     new_well_y = read_message();

// first acknowledge the message so the harvester stops sending it:

     get_message_source(TARGET_MESSAGE);
     
     transmit(TARGET_MESSAGE, 1, MESSAGE_WELL_FOUND_ACK);

// confirm that this well is not already known:

     well_already_known = 0;

     for (i = 0; i < known_wells; i ++)
     {
      if (data_well_x [i] == new_well_x
       && data_well_y [i] == new_well_y)
      {
       well_already_known = 1;
       break;
      } 
     }

     if (well_already_known)
     {
//printf(" K %i", get_instructions_left());
      break;
     }
      
// new well is not known. Save it (can assume that i is first empty entry in well array)
     data_well_x [i] = new_well_x;
     data_well_y [i] = new_well_y;      
     data_well_time [i] = 1; // should go to this well.
     
     known_wells ++;
//printf(" A %i", get_instructions_left());
// finished. new well saved.
     
     break; // end case MESSAGE_WELL_FOUND

    case MESSAGE_NEXT_WELL_PLEASE:
// harvester wants a new data well. This message has no contents.
//printf("\n nwC %i", get_instructions_left());


   get_message_source(TARGET_MESSAGE);

// consider sending them to find a new well:
   if (random(known_wells) == 0)
   {
      transmit(TARGET_MESSAGE, 
               1, 
               MESSAGE_SEEK_WELLS);
   
    break;
   }


// Send them to the well with the lowest time:

     target_well_index = 0;
     
     adjusted_target_well_time = cycles + 2000;


// start i at 1 because well 0 is always taken
     for (i = 1; i < known_wells; i ++)
     {
//      if (data_well_x [i] == 0)
//       break;
      adjusted_well_time = data_well_time [i]
           + distance_xy(process[TARGET_MESSAGE].get_core_x() - data_well_x [i], process[TARGET_MESSAGE].get_core_y() - data_well_y [i]) / 40;
      if (adjusted_well_time < adjusted_target_well_time)
      {
       target_well_index = i;
       adjusted_target_well_time = adjusted_well_time;
      }
     }

     if (target_well_index != 0)
     {

      transmit(TARGET_MESSAGE, 
               1, 
               MESSAGE_GO_TO_WELL,
               data_well_x [target_well_index],
               data_well_y [target_well_index]);

      data_well_time [target_well_index] = cycles;
     }
//printf(" D %i", get_instructions_left());

     break; // end case MESSAGE_NEXT_WELL_PLEASE
   
   } // end switch(read_message())

  } // end while(next_message())


   if (get_instructions_left() < 500)
    goto finished_building;


// try to build from queue (if the queue's empty this will just fail)
build_result = build_from_queue(build_target_index); // Returns 1 and saves new process to build_target_index on success.

// Try to build from the build queue, after checking whether there's anything on the queue:
if (!check_build_queue()
  || build_result == 1)
{

// Now try to build harvesters:


 int number_of_harvesters;
 number_of_harvesters = 0;
 int free_harvester;
 free_harvester = -1;
 int harvesters_needed;
 harvesters_needed = 2 + (cycles / 1000);
 
 for (i = 0; i < HARVESTERS; i ++)
 {
  if (process[TARGET_HARVESTER_0 + i].visible())
   number_of_harvesters ++;
    else
     free_harvester = TARGET_HARVESTER_0 + i;
 }

 
 if (number_of_harvesters < harvesters_needed)
 {
  if (random(get_process_count()) < 10)
   add_to_build_queue(TEMPLATE_HARVESTER_1, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
    else
    {
     if (random(get_process_count()) < 20)
      add_to_build_queue(TEMPLATE_HARVESTER_2, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
       else
        add_to_build_queue(TEMPLATE_HARVESTER_3, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);    
    } 

  if (free_harvester != -1)
   build_target_index = free_harvester;
    else
     build_target_index = TARGET_HARVESTER_0; 

  goto finished_building;
 }

 if (random(cycles) < 200)
  add_to_build_queue(TEMPLATE_GUARD_1, core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
   else
   {    
    add_to_build_queue(TEMPLATE_GUARD_2 + (guard_build & 1), core_x + cos(angle, 400), core_y + sin(angle, 400), angle, 0, 0);
    guard_build ++;
   }

 build_target_index = -1; // don't need to record guards in targetting memory



}


finished_building:


charge_interface_max(); // charges the process' interface. Since the interface is shared across all
 // components with interface objects, this call is not specific to any object or class.
 // charge_interface_max() charges the interface using any power left over.

int attacking;
attacking = 0;

attacking += auto_att_left.attack_scan(-1500, 400, TARGET_LEFT);
attacking += auto_att_right.attack_scan(1500, 400, TARGET_RIGHT);
attacking += auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);
attacking += auto_att_back.attack_scan(4096, 400, TARGET_BACK);

if (attacking > 0
 && get_interface_strength() < 2000)
 special_AI(1, 0);

restore_scan(0,0);
repair_scan(0,0);
restore_self(); // tries to restore any destroyed components
repair_self(); // tries to repair any damaged components


auto_harvest.gather_data();

auto_allocate.allocate_data(4); // actually I think the maximum is 2. Doesn't matter
