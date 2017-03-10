

#process "defender"

class auto_att_left;
class auto_att_fwd;
class auto_att_back;
class auto_att_right;

class spike_back;

core_static_hex_C, 7736, 
  {object_downlink, -1097, 
    {component_long6, // component 1
      {object_pulse_xl:auto_att_right, 0},
      {object_uplink, 0},
      {object_spike:spike_back, 0},
      {object_pulse_xl:auto_att_left, 0},
      {object_pulse_xl:auto_att_fwd, 0},
      {object_none, 0}
    }
  },
  {object_downlink, 768, 
    {component_drop, // component 2
      {object_pulse_xl:auto_att_back, 0},
      {object_pulse_xl:auto_att_right, 0},
      {object_spike:spike_back, 0},
      {object_pulse_xl:auto_att_fwd, 180},
      {object_none, 0},
      {object_uplink, 0}
    }
  },
  {object_downlink, 1079, 
    {component_cap, // component 3
      {object_downlink, 190, 
        {component_cap, // component 4
          {object_spike:spike_back, 0},
          {object_none, 0},
          {object_none, 0},
          {object_uplink, 0},
        }
      },
      {object_spike:spike_back, 0},
      {object_none, 0},
      {object_uplink, 0},
    }
  },
  {object_downlink, 814, 
    {component_drop, // component 5
      {object_uplink, 0},
      {object_interface, 0},
      {object_interface, 0},
      {object_interface, 0},
      {object_interface, 0},
      {object_interface, 0}
    }
  },
  {object_downlink, 882, 
    {component_cap, // component 6
      {object_none, 0},
      {object_uplink, 0},
      {object_spike:spike_back, 0},
      {object_downlink, -56, 
        {component_cap, // component 7
          {object_uplink, 0},
          {object_none, 0},
          {object_none, 0},
          {object_spike:spike_back, 0},
        }
      },
    }
  },
  {object_repair_other, 0}

#code



enum
{
// These are the channels that processes in this stage
//  use to communicate with each other.
// There can be up to 8
CHANNEL_HARVESTER, // not really used as all harvester messages are sent by transmit(), which doesn't use channels
CHANNEL_TARGET, // used for TARGET_FOUND messages
CHANNEL_HELP, // used for UNDER_ATTACK messages

CHANNEL_DEFENDER // used by defenders and main base

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
MESSAGE_REQUEST_FOLLOWER,

// messages from defenders:
MESSAGE_TARGET_FRONT,
MESSAGE_TARGET_LEFT,
MESSAGE_TARGET_RIGHT
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
  TARGET_BACK,
  
  TARGET_SPIKE,
  
  TARGET_OLD_FRONT,
  TARGET_OLD_LEFT,
  TARGET_OLD_RIGHT,
  TARGET_OLD_BACK,
};

int message_type;
int angle_diff;

int initialised;

if (!initialised)
{
 attack_mode(2);

// need to work out where this process is in relation to the main base:
 angle_diff = angle_difference(get_core_angle(), process[TARGET_BASE].get_core_angle());
// if (abs(angle_diff) < 500)
 message_type = MESSAGE_TARGET_FRONT; // just make this the default
 
 if (angle_diff < -500)
  message_type = MESSAGE_TARGET_LEFT;
 if (angle_diff > 500)
  message_type = MESSAGE_TARGET_RIGHT;
  
 listen_channel(CHANNEL_DEFENDER); 

 initialised = 1;
}

int transmitted;

transmitted = 0;

charge_interface_max();

// spikes:

if (next_message())
{

 read_message(); // can assume this is a MESSAGE_TARGET_* message so ignore this entry
 
 get_message_target(TARGET_SPIKE);

}

if (process[TARGET_SPIKE].visible())
{
 spike_back.fire_spike_at(TARGET_SPIKE, 0);
}







target_copy(TARGET_OLD_FRONT, TARGET_FRONT);

if (auto_att_fwd.attack_scan(0, 200, TARGET_FRONT) > 0)
{
 if (!target_compare(TARGET_FRONT, TARGET_OLD_FRONT))
 {
  transmit_target(TARGET_BASE, 1, TARGET_FRONT, message_type);
  broadcast_target(-1, CHANNEL_DEFENDER, 1, TARGET_FRONT);
  transmitted = 1;
 }
}

target_copy(TARGET_OLD_LEFT, TARGET_LEFT);

if (auto_att_left.attack_scan(-1000, 400, TARGET_LEFT) > 0)
{
 if (!transmitted
  && !target_compare(TARGET_LEFT, TARGET_OLD_LEFT))
 {
  transmit_target(TARGET_BASE, 1, TARGET_LEFT, message_type);
  broadcast_target(-1, CHANNEL_DEFENDER, 1, TARGET_FRONT);
  transmitted = 1;
 }
}

target_copy(TARGET_OLD_RIGHT, TARGET_RIGHT);

if (auto_att_right.attack_scan(1000, 400, TARGET_RIGHT) > 0)
{
 if (!transmitted
  && !target_compare(TARGET_RIGHT, TARGET_OLD_RIGHT))
 {
  transmit_target(TARGET_BASE, 1, TARGET_RIGHT, message_type);
  broadcast_target(-1, CHANNEL_DEFENDER, 1, TARGET_FRONT);
  transmitted = 1;
 }
}

auto_att_back.attack_scan(4000, 400, TARGET_BACK);


restore_scan(0,0);
repair_scan(0,0);

restore_self();
repair_self();
