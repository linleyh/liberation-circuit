

#process "rbase"

class auto_att_back;
class auto_att_fwd;
class auto_harvest;
class auto_att_right;
class auto_att_left;
class auto_allocate;

core_static_hex_B, 1400, 
  {object_downlink, -638, 
    {component_long5, // component 7
      {object_allocate:auto_allocate, 0},
      {object_none, 0},
      {object_pulse_l:auto_att_right, 0},
      {object_uplink, 0},
      {object_pulse_l:auto_att_fwd, -863},
    }
  },
  {object_downlink, -48, 
    {component_long5, // component 1
      {object_downlink, -121, 
        {component_cap, // component 2
          {object_pulse_l:auto_att_left, 0},
          {object_none, 0},
          {object_none, 0},
          {object_uplink, 0},
        }
      },
      {object_pulse_l:auto_att_fwd, 0},
      {object_harvest:auto_harvest, 0},
      {object_pulse_l:auto_att_right, 0},
      {object_uplink, 0},
    }
  },
  {object_downlink, 0, 
    {component_box, // component 3
      {object_uplink, 0},
      {object_interface, 0},
      {object_interface, -112},
      {object_interface, 0},
    }
  },
  {object_downlink, 48, 
    {component_long5, // component 4
      {object_downlink, 89, 
        {component_cap, // component 6
          {object_uplink, 0},
          {object_none, 0},
          {object_none, 0},
          {object_pulse_l:auto_att_back, 0},
        }
      },
      {object_storage, 0},
      {object_pulse_l:auto_att_fwd, 0},
      {object_uplink, 0},
      {object_pulse_l:auto_att_left, 0},
    }
  },
  {object_downlink, 638, 
    {component_long5, // component 5
      {object_build, 0},
      {object_pulse_l:auto_att_left, 0},
      {object_none, 0},
      {object_pulse_l:auto_att_fwd, 1035},
      {object_uplink, 0},
    }
  },
  {object_repair, 0}

#code




// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder in address 0
  TARGET_LEFT, // target of directional left attack
  TARGET_RIGHT, // target of directional right attack
  TARGET_FRONT, // target of directional front attack
  TARGET_BACK, // target of directional back attack
  
  TARGET_HARVESTER1,
  TARGET_HARVESTER2,
  TARGET_HARVESTER3,

  TARGET_END

};



// Templates
//  - this process assumes that the templates have been set up in a certain way,
//    as follows:
enum
{
TEMPLATE_BASE,
TEMPLATE_ATTACKER1,
TEMPLATE_ATTACKER2,
TEMPLATE_HARVESTER

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

// builder variables
int build_result; // build result code (returned by build call)
int build_counter;

int build_special;
int target_memory_index;

int initialised;

if (!initialised)
{
 attack_mode(1);
 build_special = 3;
 initialised = 1;
 special_AI(0, 10);
}


auto_harvest.gather_data();


auto_allocate.allocate_data(4); // actually I think the maximum is 2

if (build_counter > 0)
{
 build_result = build_process(TEMPLATE_ATTACKER1, cos(angle, 400), sin(angle, 400), angle, -1);
 if (build_result == 1)
  build_counter --;
 goto finished_building;
}

// build_counter must be 4, so build something else:

if (build_special == 0)
{
 build_result = build_process(TEMPLATE_ATTACKER2, cos(angle, 400), sin(angle, 400), angle, -1);
 if (build_result == 1)
 {
  build_counter = 3;
  build_special = 1;
 }
}
 else
 {
  build_result = build_process(TEMPLATE_HARVESTER, cos(angle, 400), sin(angle, 400), angle, -1);
  if (build_result == 1)
  {
   build_counter = 3;
   build_special --;
  } 
 }



finished_building:

charge_interface_max();

int attacking;

attacking = 0;

attacking += auto_att_left.attack_scan(-1800, 400, TARGET_LEFT);
attacking += auto_att_right.attack_scan(1800, 400, TARGET_RIGHT);
attacking += auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);
attacking += auto_att_back.attack_scan(4096, 400, TARGET_BACK);

if (attacking > 0)
 special_AI(1, 0);

restore_self(); // tries to restore any destroyed components
repair_self(); // tries to repair any damaged components


auto_harvest.gather_data();

auto_allocate.allocate_data(4); // actually I think the maximum is 2
