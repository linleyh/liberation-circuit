

#process "rbase"

class auto_att_fwd;
class auto_harvest;
class auto_att_back;
class auto_allocate;
class auto_att_left;
class auto_att_right;

core_static_hex_A, 271, 
  {object_downlink, -449, 
    {component_long4, // component 1
      {object_build, 0},
      {object_pulse:auto_att_fwd, 322},
      {object_none, 0},
      {object_uplink, 0},
    }
  },
  {object_downlink, 457, 
    {component_long4, // component 2
      {object_harvest:auto_harvest, 0},
      {object_pulse:auto_att_right, 0},
      {object_none, 0},
      {object_uplink, 0},
    }
  },
  {object_repair, 0},
  {object_downlink, -516, 
    {component_long4, // component 3
      {object_allocate:auto_allocate, 0},
      {object_pulse:auto_att_back, 298},
      {object_none, 0},
      {object_uplink, 0},
    }
  },
  {object_downlink, 497, 
    {component_long4, // component 4
      {object_storage, 0},
      {object_pulse:auto_att_left, 0},
      {object_none, 0},
      {object_uplink, 0},
    }
  },
  {object_none, 0}

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
  TARGET_END

};



// Templates
//  - this process assumes that the templates have been set up in a certain way,
//    as follows:
enum
{
TEMPLATE_BASE,
TEMPLATE_ATTACKER1,
TEMPLATE_ATTACKER2

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

special_AI(0, 5);



auto_harvest.gather_data();


auto_allocate.allocate_data(4); // actually I think the maximum is 2

if (build_counter < 5)
{
 build_result = build_process(TEMPLATE_ATTACKER1, cos(angle - 1024, 400), sin(angle - 1024, 400), angle - 1024, -1);
 if (build_result == 1)
  build_counter ++;
 goto finished_building;
}

// build_counter must be 5, so build a different attacker:

build_result = build_process(TEMPLATE_ATTACKER2, cos(angle - 1024, 400), sin(angle - 1024, 400), angle - 1024, -1);
if (build_result == 1)
 build_counter = 0;



finished_building:

charge_interface_max();

int attacking;

attacking = 0;

attacking += auto_att_left.attack_scan(-3000, 400, TARGET_LEFT);
attacking +=  auto_att_right.attack_scan(3000, 400, TARGET_RIGHT);
attacking +=  auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);

if (attacking > 0)
 special_AI(1, 0);

repair_self(); // tries to repair any damaged components
restore_self(); // tries to restore any destroyed components


auto_harvest.gather_data();
auto_allocate.allocate_data(4); // actually I think the maximum is 2
