

#process "rbase"

class auto_att_back;
class auto_harvest;
class auto_allocate;
class auto_att_left;
class auto_att_right;
class auto_att_fwd;

core_static_hex_A, 1400, 
  {object_downlink, -994, 
    {component_cap, // component 2
      {object_storage, 0},
      {object_uplink, 0},
      {object_allocate:auto_allocate, 0},
      {object_pulse_l:auto_att_left, 0},
    }
  },
  {object_downlink, 195, 
    {component_cap, // component 1
      {object_pulse_l:auto_att_back, 0},
      {object_harvest:auto_harvest, 0},
      {object_uplink, 0},
      {object_pulse_l:auto_att_fwd, 0},
    }
  },
  {object_downlink, 0, 
    {component_tri, // component 5
      {object_uplink, 0},
      {object_interface, 0},
      {object_interface, 0},
    }
  },
  {object_downlink, 0, 
    {component_cap, // component 3
      {object_pulse_l:auto_att_fwd, 0},
      {object_uplink, 0},
      {object_storage, 0},
      {object_pulse_l:auto_att_back, 0},
    }
  },
  {object_downlink, 920, 
    {component_cap, // component 4
      {object_pulse_l:auto_att_right, 289},
      {object_build, 0},
      {object_uplink, 766},
      {object_storage, 0},
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

special_AI(0, 9);

auto_harvest.gather_data();


auto_allocate.allocate_data(4); // actually I think the maximum is 2

if (build_counter < 4)
{
 build_result = build_process(TEMPLATE_ATTACKER1, cos(angle, 400), sin(angle, 400), angle, -1);
 if (build_result == 1)
  build_counter ++;
 goto finished_building;
}

// build_counter must be 4, so build something else:

if (build_special == 1)
{
 build_result = build_process(TEMPLATE_ATTACKER2, cos(angle, 400), sin(angle, 400), angle, -1);
 if (build_result == 1)
 {
  build_counter = 0;
  build_special = 0;
 }
}
 else
 {
  build_result = build_process(TEMPLATE_HARVESTER, cos(angle, 400), sin(angle, 400), angle, -1);
  if (build_result == 1)
  {
   build_counter = 0;
   build_special = 1;
  } 
 }



finished_building:

charge_interface_max();

if (get_interface_strength() < 1500)
 special_AI(1, 0);

 auto_att_left.attack_scan(-1200, 400, TARGET_LEFT);
 auto_att_right.attack_scan(1200, 400, TARGET_RIGHT);
 auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);
 auto_att_back.attack_scan(4096, 400, TARGET_BACK);

restore_self(); // tries to restore any destroyed components
repair_self(); // tries to repair any damaged components


auto_harvest.gather_data();

auto_allocate.allocate_data(4); // actually I think the maximum is 2
