

#process "rbase"

class auto_harvest;
class auto_allocate;
class auto_att_right;
class auto_att_fwd;
class auto_att_back;
class auto_att_left;

core_static_hex_B, 0, 
  {object_harvest:auto_harvest, 0},
  {object_downlink, 0, 
    {component_long5, // member 1
      {object_uplink, 0},
      {object_interface_depth, 0},
      {object_interface, 0},
      {object_downlink, 499, 
        {component_bowl, // member 2
          {object_build, 0},
          {object_uplink, 0},
          {object_interface, 0},
          {object_pulse_l:auto_att_right, 0},
          {object_pulse_l:auto_att_right, 0},
        }
      },
      {object_downlink, -617, 
        {component_bowl, // member 3
          {object_storage, 0},
          {object_interface, 0},
          {object_uplink, 0},
          {object_pulse_l:auto_att_fwd, 0},
          {object_pulse_l:auto_att_right, 0},
        }
      },
    }
  },
  {object_allocate:auto_allocate, 0},
  {object_downlink, 0, 
    {component_long5, // member 4
      {object_uplink, 0},
      {object_interface_depth, 0},
      {object_interface, 0},
      {object_downlink, 455, 
        {component_bowl, // member 5
          {object_build, 0},
          {object_uplink, 0},
          {object_interface, 0},
          {object_pulse_l:auto_att_back, 0},
          {object_pulse_l:auto_att_left, 0},
        }
      },
      {object_downlink, -525, 
        {component_bowl, // member 6
          {object_storage, 0},
          {object_interface, 0},
          {object_uplink, 0},
          {object_pulse_l:auto_att_right, 0},
          {object_pulse_l:auto_att_back, 0},
        }
      },
    }
  },
  {object_repair, 0},
  {object_downlink, 0, 
    {component_long5, // member 7
      {object_uplink, 0},
      {object_interface_depth, 0},
      {object_interface, 0},
      {object_downlink, 541, 
        {component_bowl, // member 8
          {object_build, 0},
          {object_uplink, 0},
          {object_interface, 0},
          {object_pulse_l:auto_att_left, 0},
          {object_pulse_l:auto_att_fwd, 0},
        }
      },
      {object_downlink, -432, 
        {component_bowl, // member 9
          {object_storage, 0},
          {object_interface, 0},
          {object_uplink, 0},
          {object_pulse_l:auto_att_left, 0},
          {object_pulse_l:auto_att_left, 0},
        }
      },
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
  TARGET_LEFT, // target of directional left attack
  TARGET_RIGHT, // target of directional right attack
  TARGET_BACK, // target of directional backwards attack
// The following addresses are used to track processes that this process
//  has built, to make sure it builds the right number of each:
  TARGET_HARVESTER_0,
  TARGET_HARVESTER_1,
  TARGET_HARVESTER_2,
  TARGET_BUILDER_0,
  TARGET_COMMANDER_0,
  TARGET_COMMANDER_1,
  TARGET_COMMANDER_2,
  TARGET_END

};



// Templates
//  - this process assumes that the templates have been set up in a certain way,
//    as follows:
enum
{
TEMPLATE_BASE,
TEMPLATE_BUILDER,
TEMPLATE_HARVESTER,
TEMPLATE_COMMANDER,
TEMPLATE_ATTACKER

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

int scan_result; // used to hold the results of a scan of nearby processes

// builder variables
int build_result; // build result code (returned by build call)



auto_harvest.gather_data();


auto_allocate.allocate_data(4); // actually I think the maximum is 2

int i;

int special_build_counter;

if (special_build_counter < 3)
{
 build_result = build_process(TEMPLATE_ATTACKER, cos(angle, 400), sin(angle, 400), angle, -1);
 if (build_result == 1)
  special_build_counter ++;
 goto finished_building;
}

for (i = TM_COMMANDER_0; i < TM_END; i ++)
{
 if (process[i].get_core_x() < 0) // doesn't exist
 {
  build_result = build_process(TEMPLATE_COMMANDER, cos(angle, 400), sin(angle, 400), angle, i);
  if (build_result == 1)
   special_build_counter = 0;
  goto finished_building;
 }
}

// check whether each entry in targetting memory has something:
for (i = TM_HARVESTER_0; i < TM_BUILDER_0; i ++)
{
 if (process[i].get_core_x() < 0) // doesn't exist
 {
  build_result = build_process(TEMPLATE_HARVESTER, cos(angle, 400), sin(angle, 400), angle, i);
  if (build_result == 1)
   special_build_counter = 0;
  goto finished_building;
 }
}

i = TM_BUILDER_0;
if (process[i].get_core_x() < 0) // doesn't exist
{
 build_result = build_process(TEMPLATE_BUILDER, cos(angle, 400), sin(angle, 400), angle, i);
 if (build_result == 1)
  special_build_counter = 0;
 goto finished_building;
}

build_result = build_process(TEMPLATE_ATTACKER, -400, 0, 4096, -1);
if (build_result == 1)
 special_build_counter ++;

finished_building:

auto_harvest.gather_data();

auto_allocate.allocate_data(4); // actually I think the maximum is 2








 auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);

 auto_att_left.attack_scan(-2048, 400, TARGET_LEFT);

 auto_att_right.attack_scan(2048, 400, TARGET_RIGHT);

 auto_att_back.attack_scan(4096, 400, TARGET_BACK);

spare_power_to_interface(); // charges the process' interface. Since the interface is shared across all
 // components with interface objects, this call is not specific to any object or class.
 // spare_power_to_interface() charges the interface using any power left over that will not be needed
 // to reduce stress.

if (get_power_left() > 120)
  repair_self(); // tries to repair any damaged components

if (get_power_left() > 120)
  restore_self(); // tries to restore any destroyed components
