

#process "guard3"

class auto_att_left;
class auto_move;
class auto_att_back;
class auto_att_fwd;
class auto_att_right;

core_hex_B, 6251, 
  {object_downlink, 0, 
    {component_long6, // component 1
      {object_none, 0},
      {object_uplink, 0},
      {object_repair, 0},
      {object_pulse:auto_att_back, 0},
      {object_downlink, -1003, 
        {component_cap, // component 2
          {object_uplink, 0},
          {object_interface, 0},
          {object_move:auto_move, 1006},
          {object_move:auto_move, 0},
        }
      },
      {object_downlink, 961, 
        {component_cap, // component 3
          {object_none, 0},
          {object_none, 0},
          {object_none, 0},
          {object_uplink, 0},
        }
      }
    }
  },
  {object_downlink, 423, 
    {component_box, // component 4
      {object_pulse_xl:auto_att_left, 519},
      {object_downlink, 0, 
        {component_cap, // component 5
          {object_pulse_xl:auto_att_fwd, -436},
          {object_none, 0},
          {object_none, 0},
          {object_uplink, 0},
        }
      },
      {object_pulse_l:auto_att_right, 360},
      {object_uplink, 360},
    }
  },
  {object_downlink, 249, 
    {component_long6, // component 6
      {object_pulse_l:auto_att_back, -393},
      {object_uplink, -68},
      {object_none, 0},
      {object_stream_dir:auto_att_fwd, 268},
      {object_downlink, -1130, 
        {component_cap, // component 7
          {object_uplink, 0},
          {object_none, 0},
          {object_none, 0},
          {object_pulse_xl:auto_att_fwd, 873},
        }
      },
      {object_downlink, 256, 
        {component_cap, // component 8
          {object_move:auto_move, 813},
          {object_interface, 0},
          {object_interface, 0},
          {object_uplink, 0},
        }
      }
    }
  },
  {object_repair, 0},
  {object_downlink, 150, 
    {component_long6, // component 9
      {object_uplink, 0},
      {object_downlink, 248, 
        {component_cap, // component 10
          {object_move:auto_move, 772},
          {object_interface, 0},
          {object_uplink, 0},
          {object_interface, 0},
        }
      },
      {object_pulse:auto_att_back, 1253},
      {object_downlink, -905, 
        {component_cap, // component 11
          {object_move:auto_move, -570},
          {object_move:auto_move, -1585},
          {object_uplink, 0},
          {object_interface, 0},
        }
      },
      {object_none, 0},
      {object_repair, 0}
    }
  },
  {object_downlink, 0, 
    {component_box, // component 12
      {object_uplink, 0},
      {object_pulse:auto_att_back, 578},
      {object_downlink, 0, 
        {component_cap, // component 13
          {object_uplink, 0},
          {object_downlink, 455, 
            {component_cap, // component 14
              {object_move:auto_move, 939},
              {object_move:auto_move, -76},
              {object_move:auto_move, -1337},
              {object_uplink, 0},
            }
          },
          {object_interface, 0},
          {object_move:auto_move, 280},
        }
      },
      {object_none, 0},
    }
  }

#code


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



// Process AI modes (these reflect the capabilities of the process)
enum
{
  MODE_GUARD, // process is circling builder
  MODE_FOLLOW, // process is following harvester
  MODE_WANDER, // wandering randomly (should only happen if nothing else to do)
  MODE_ATTACK, // process is attacking target
  MODE_ATTACK_PRIORITY, // process is attacking priority target
  MODES
};


// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder as entry 0
  TARGET_LEADER, // leader this process has found  
  TARGET_FRONT,
  TARGET_LEFT,
  TARGET_RIGHT,
  TARGET_BACK,
  TARGET_MAIN
};

int angle_to_leader;

// Variable declaration and initialisation
//  (note that declaration and initialisation cannot be combined)
//  (also, variables retain their values between execution cycles)
int core_x, core_y; // location of core
core_x = get_core_x(); // location is updated each cycle
core_y = get_core_y();
int angle; // direction process is pointing
 // angles are in integer degrees from 0 to 8192, with 0 being right,
 // 2048 down, 4096 left and 6144 up.
angle = get_core_angle(); // angle is updated each cycle

int mode; // what is the process doing? (should be one of the MODE enums)
int saved_mode; // will return to this mode after attacking something
int attack_x, attack_y;
int using_main_attack;

int move_x, move_y; // destination

int scan_result; // used to hold the results of a scan of nearby processes

int initialised;
if (initialised == 0)
{
 initialised = 1;
// attack_mode(1);
 special_AI(0, 304);
 mode = MODE_GUARD;
 listen_channel(CHANNEL_HARVESTER);
 listen_channel(CHANNEL_TARGET);
 listen_channel(CHANNEL_HELP);

}


using_main_attack = 0;


// What the process does next depends on its current mode
switch(mode)
{


 case MODE_WANDER:
// only do this if nothing else to do.
  if (distance_from_xy(move_x, move_y) < 300)
  {
   gosub start_wandering;
   break;
  }
  auto_move.move_to(move_x, move_y);
  saved_mode = MODE_WANDER;
  gosub scan_for_target;
// should listen for messages here too
  break;
  

 case MODE_GUARD:
// guards parent
  saved_mode = MODE_GUARD;
  if (process[TARGET_PARENT].visible() <= 0) // base destroyed
  {
   gosub start_wandering;
   break;
  }
  angle_to_leader = process[TARGET_PARENT].target_angle();
  angle_to_leader += 1024;
  move_x = process[TARGET_PARENT].get_core_x() - cos(angle_to_leader, 700);
  move_y = process[TARGET_PARENT].get_core_y() - sin(angle_to_leader, 700);
  auto_move.move_to(move_x, move_y);
  gosub scan_for_target;
  gosub listen_for_requests;
  break;

 case MODE_FOLLOW:
  saved_mode = MODE_FOLLOW;
  if (process[TARGET_LEADER].visible() <= 0) // base destroyed
  {
   gosub start_wandering;
   break;
  }
  angle_to_leader = process[TARGET_LEADER].target_angle();
  angle_to_leader += 1024;
  move_x = process[TARGET_LEADER].get_core_x() - cos(angle_to_leader, 700);
  move_y = process[TARGET_LEADER].get_core_y() - sin(angle_to_leader, 700);
  auto_move.move_to(move_x, move_y);
  gosub listen_for_requests;
  gosub scan_for_target;
  break;


 case MODE_ATTACK_PRIORITY:
 case MODE_ATTACK:
  if (!process[TARGET_MAIN].visible())
  {
   if (target_destroyed(TARGET_MAIN))
    special_AI(1, 2);
   mode = saved_mode;
   break;
  }
  if (process[TARGET_MAIN].distance_less(1000))
  {
   auto_att_fwd.fire_at(TARGET_MAIN, random(process[TARGET_MAIN].get_components()));
   using_main_attack = 1;
  }
  auto_move.approach_target(TARGET_MAIN, 0, 500);
  break;
 

} // end of mode switch


if (!using_main_attack)
 auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);

auto_att_left.attack_scan(-2048, 400, TARGET_LEFT);
auto_att_right.attack_scan(2048, 400, TARGET_RIGHT);
auto_att_back.attack_scan(4096, 400, TARGET_BACK);

charge_interface_max();

restore_self();
repair_self();


exit;


start_wandering:
 mode = MODE_WANDER;
 move_x = 800 + random(world_x() - 1600);
 move_y = 800 + random(world_y() - 1600);
 return;
 



scan_for_target:
  if (scan_single(0,0,TARGET_MAIN,0,0,100,0b1000)) // 0b1000 means only processes with allocator
  {
   broadcast_target(10000,
                    CHANNEL_TARGET,
                    1, // priority 1
                    TARGET_MAIN, // target attached to transmission
                    MESSAGE_TARGET_FOUND, 
                    process[TARGET_MAIN].get_core_x(),
                    process[TARGET_MAIN].get_core_y());
   mode = MODE_ATTACK_PRIORITY; 
   special_AI(1, 6);
   return;
  }
  if (scan_for_threat(0, 0, TARGET_MAIN))
  {
   mode = MODE_ATTACK;
   special_AI(1, 5);
  }
  return;






listen_for_requests:
// listen to other procs requesting followers.
  if (check_messages() // returns 1 if message received since previous cycle
   && read_message() == MESSAGE_REQUEST_FOLLOWER // returns contents of message in sequence
   && random(2)) // may or may not decide to follow.
  {
   mode = MODE_FOLLOW;
   get_message_source(TARGET_LEADER); // saves message sender into targetting memory
  }
  return;
