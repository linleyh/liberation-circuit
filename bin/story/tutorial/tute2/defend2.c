

#process "defender"
// This process has objects with the following auto classes:
class auto_att_left;
class auto_att_back;
class auto_att_fwd;

// The following auto classes are not currently used by any objects:
class auto_move;
class auto_retro;
class auto_att_main;
class auto_att_right;
class auto_att_spike;
class auto_harvest;
class auto_allocate;
class auto_stability;

core_static_hex_A, 0, 
  {object_downlink, 1427, 
    {component_long5, // component 1
      {object_none, 0},
      {object_none, 0},
      {object_none, 0},
      {object_pulse:auto_att_fwd, 0},
      {object_uplink, 0},
    }
  },
  {object_none, 0},
  {object_downlink, 1530, 
    {component_long5, // component 2
      {object_none, 0},
      {object_none, 0},
      {object_none, 0},
      {object_pulse:auto_att_back, 0},
      {object_uplink, 0},
    }
  },
  {object_repair_other, 0},
  {object_downlink, 1195, 
    {component_long5, // component 3
      {object_none, 0},
      {object_none, 0},
      {object_none, 0},
      {object_pulse:auto_att_left, 0},
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
  TARGET_PARENT, // a newly built process starts with its builder as entry 0
  TARGET_FRONT, // target of directional forward attack
  TARGET_LEFT, // target of directional left attack
  TARGET_RIGHT, // target of directional right attack
  TARGET_BACK, // target of directional backwards attack
};

attack_mode(1);

// All that this process does is sit around and wait until something comes near, then attack it.

repair_self();
restore_self();

auto_att_left.attack_scan(-3000, 400, TARGET_LEFT);
auto_att_right.attack_scan(3000, 400, TARGET_RIGHT);
auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);
auto_att_back.attack_scan(4096, 400, TARGET_BACK);
