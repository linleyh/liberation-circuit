

#process "base"

class auto_allocate;
class auto_harvest;
class auto_att_right;
class auto_att_back;
class auto_att_left;
class auto_att_fwd;

core_static_hex_C, 811, 
  {object_build, 0},
  {object_downlink, 0, 
    {component_peak, // component 1
      {object_uplink, 0},
      {object_downlink, 0, 
        {component_fork, // component 2
          {object_interface, 0},
          {object_interface, 0},
          {object_uplink, 0},
        }
      },
      {object_harvest:auto_harvest, 0},
      {object_pulse_xl:auto_att_right, 116},
      {object_pulse_l:auto_att_right, -168},
    }
  },
  {object_repair_other, 0},
  {object_downlink, 0, 
    {component_peak, // component 3
      {object_uplink, 0},
      {object_downlink, 0, 
        {component_fork, // component 4
          {object_interface, 0},
          {object_interface, 0},
          {object_uplink, 0},
        }
      },
      {object_storage, 0},
      {object_pulse_l:auto_att_back, -246},
      {object_pulse_xl:auto_att_left, 13},
    }
  },
  {object_allocate:auto_allocate, 0},
  {object_downlink, 0, 
    {component_peak, // component 5
      {object_uplink, 0},
      {object_downlink, 0, 
        {component_fork, // component 6
          {object_interface, 0},
          {object_interface, 0},
          {object_uplink, 0},
        }
      },
      {object_storage, 0},
      {object_pulse_xl:auto_att_left, 281},
      {object_pulse_l:auto_att_fwd, -479},
    }
  }

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
  TARGET_BACK
};

// Templates
//  - this process assumes that the templates have been set up in a certain way,
//    as follows:
enum
{
TEMPLATE_BASE,
TEMPLATE_FIREBASE,
TEMPLATE_BUILDER,
TEMPLATE_SPIKEBASE,

TEMPLATE_OUTPOST,
TEMPLATE_BUILDER2,

};



// Variable declaration and initialisation
//  (note that declaration and initialisation cannot be combined)
//  (also, variables retain their values between execution cycles)
int core_x, core_y; // location of core
int angle; // direction process is pointing
 // angles are in integer degrees from 0 to 8192, with 0 being right,
 // 2048 down, 4096 left and 6144 (or -2048) up.
int initialised;
int build_result;
int firebases;
int firebase_angle, firebase_dist;
int attacking;

if (!initialised)
{

 core_x = get_core_x(); // location is updated each cycle
 core_y = get_core_y();
 angle = get_core_angle(); // angle is updated each cycle
 
 attack_mode(1);
 special_AI(0, 100);

 gosub add_builder_to_queue;


 initialised = 1;

}

 if (get_available_data() > 200)
 {
  build_result = build_from_queue(-1); // -1 means not to record target
  if (build_result == 1)
   gosub add_builder_to_queue;

 } 


if (firebases < 3
 || (get_damage()
  && get_interface_strength() < get_interface_capacity() - 100))
{
 firebase_angle = random(8192);
 firebase_dist = 300 + random(500);

// this ignores the queue
build_result =  build_process(TEMPLATE_SPIKEBASE,
                  cos(firebase_angle, firebase_dist),
                  sin(firebase_angle, firebase_dist),
                  firebase_angle,
                  -1);
if (build_result == 1)
 firebases ++;

}



 charge_interface_max();


attacking = 0;

attacking += auto_att_left.attack_scan(-2000, 400, TARGET_LEFT);
attacking += auto_att_right.attack_scan(2000, 400, TARGET_RIGHT);
attacking += auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);
attacking += auto_att_back.attack_scan(4096, 400, TARGET_BACK);

if (attacking > 0
 && get_interface_strength() < 1500)
  special_AI(1, 0);


restore_scan(0,0); // tries to restore any destroyed components
repair_scan(0,0); // tries to repair any damaged components
restore_self();
repair_self();

auto_harvest.gather_data();

auto_allocate.allocate_data(4); // actually I think the maximum is 2



exit;


   
add_builder_to_queue:   

  if (get_process_count() < 8)
   add_to_build_queue(TEMPLATE_BUILDER2, 
                      core_x + cos(angle, 500), 
                      core_y + sin(angle, 500), 
                      angle, 
                      0,
                      0); // repeat   
    else
     add_to_build_queue(TEMPLATE_BUILDER, 
                        core_x + cos(angle, 500), 
                        core_y + sin(angle, 500), 
                        angle, 
                        0,
                        0); // repeat   
 return;
