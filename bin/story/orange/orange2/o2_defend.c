

#process "defender"

class auto_att_left;
class auto_att_right;
class auto_att_back;

core_static_hex_B, 7736, 
  {object_downlink, -247, 
    {component_prong, // component 1
      {object_pulse_xl:auto_att_left, -1797},
      {object_none, 0},
      {object_pulse_l:auto_att_left, -306},
      {object_uplink, 0},
    }
  },
  {object_downlink, 80, 
    {component_prong, // component 2
      {object_none, 0},
      {object_pulse_xl:auto_att_right, 1626},
      {object_uplink, 0},
      {object_pulse_l:auto_att_right, 195},
    }
  },
  {object_downlink, 1338, 
    {component_cap, // component 4
      {object_pulse_l:auto_att_back, 0},
      {object_interface, 0},
      {object_interface, 0},
      {object_uplink, 0},
    }
  },
  {object_repair_other, 0},
  {object_downlink, 0, 
    {component_cap, // component 3
      {object_interface, 0},
      {object_uplink, 0},
      {object_interface, 0},
      {object_pulse_l:auto_att_back, 0},
    }
  },
  {object_repair, 0}

#code




enum
{
// These are the channels that processes in this stage
//  use to communicate with each other.
// There can be up to 8
CHANNEL_HARVESTER, // not really used as all harvester messages are sent by transmit(), which doesn't use channels
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




// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_BASE, // main base
  TARGET_FRONT,
  TARGET_LEFT,
  TARGET_RIGHT,
  TARGET_BACK
};

charge_interface_max();

auto_att_left.attack_scan(-1000, 400, TARGET_LEFT);
auto_att_right.attack_scan(1000, 400, TARGET_RIGHT);
auto_att_back.attack_scan(4000, 400, TARGET_BACK);


repair_scan(0,0);
restore_scan(0,0);

repair_self();
restore_self();
