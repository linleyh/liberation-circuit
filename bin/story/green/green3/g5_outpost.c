
#process "outpost"
// This process has objects with the following auto classes:
class auto_allocate;
class auto_att_back;
class auto_att_fwd;
class auto_harvest;

// The following auto classes are not currently used by any objects:
class auto_move;
class auto_retro;
class auto_att_main;
class auto_att_left;
class auto_att_right;
class auto_att_spike;
class auto_stability;

core_static_hex_A, 5468, 
  {object_allocate:auto_allocate, 0},
  {object_downlink, 830, 
    {component_cap, // component 2
      {object_pulse_l:auto_att_fwd, 0},
      {object_harvest:auto_harvest, 0},
      {object_storage, 0},
      {object_uplink, 0},
    }
  },
  {object_downlink, 0, 
    {component_tri, // component 3
      {object_uplink, 0},
      {object_interface, -56},
      {object_interface, 0},
    }
  },
  {object_downlink, -866, 
    {component_cap, // component 1
      {object_uplink, 0},
      {object_storage, 0},
      {object_none, 0},
      {object_pulse_l:auto_att_fwd, 0},
    }
  },
  {object_repair, 0},
  {object_pulse_l:auto_att_back, 0}

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

int initialised;

if (!initialised)
{
 special_AI(0, 104);
 initialised = 1;
}

charge_interface_max();


 auto_att_left.attack_scan(-2000, 400, TARGET_LEFT);
 auto_att_right.attack_scan(2000, 400, TARGET_RIGHT);
 auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);
 auto_att_back.attack_scan(4096, 400, TARGET_BACK);

restore_self();
repair_self();



auto_harvest.gather_data();

auto_allocate.allocate_data(4); // actually I think the maximum is 2
