

#process "follower"

class auto_att_fwd;
class auto_move;

core_quad_B, 4096, 
  {object_move:auto_move, 874},
  {object_move:auto_move, -874},
  {object_pulse:auto_att_fwd, 1198},
  {object_pulse:auto_att_fwd, -1198},

#code


enum
{
// These are the channels that processes in this stage
//  use to communicate with each other.
// There can be up to 8
CHANNEL_MAIN_BASE,
CHANNEL_TARGET,
CHANNEL_WELL,
CHANNEL_REQUEST_FOLLOWER,

};

enum
{
// these are codes for the first value of a broadcast message.
// they tell the recipient what kind of messge it is.
//  (the recipient needs the same enum declaration)
MESSAGE_MAIN_BASE, // the main base broadcasts a message which prevents other bases taking over as main base
MESSAGE_TARGET_FOUND,
MESSAGE_REQUEST_FOLLOWER,
MESSAGE_WELL_CLAIM
};


// Process AI modes (these reflect the capabilities of the process)
enum
{
  MODE_GUARD, // process is guarding something
  MODE_ATTACK, // process is attacking something
  MODE_WANDER, // wandering randomly
  MODES
};


// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder as entry 0
  TARGET_LEADER, // leader this process has found  
  TARGET_MAIN, // main target
  TARGET_BACK, // target of directional backwards attack (if this process has one)
};

int attacking_back; // is set to 1 if backwards directional attack objects have a target
int other_target_x, other_target_y;
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

int move_x, move_y; // destination

int scan_result; // used to hold the results of a scan of nearby processes

int initialised;
if (initialised == 0)
{
 initialised = 1;
 attack_mode(1);
 mode = MODE_GUARD;
 target_copy(TARGET_LEADER, TARGET_PARENT); // copies parent to address TARGET_LEADER
 listen_channel(CHANNEL_REQUEST_FOLLOWER); // processes in this mission use this channel to request followers
 special_AI(0, 204);
}


if (target_destroyed(TARGET_LEADER))
 special_AI(1, 10);

// What the process does next depends on its current mode
switch(mode)
{


 case MODE_WANDER:
  if (distance_from_xy(move_x, move_y) < 300)
  {
   gosub start_wandering;
   break;
  }
  if (scan_for_threat(cos(angle, 400), sin(angle, 400), TARGET_MAIN))
  {
   mode = MODE_ATTACK;
   special_AI(1, 1);
   broadcast_target(4000, // range
                    CHANNEL_TARGET, // channel
                    0, // priority (0 just means a 1 priority message will overwrite this one)
                    TARGET_MAIN, // this target is attached to the broadcast. A listener can retrieve it with get_message_target().
                    MESSAGE_TARGET_FOUND,
                    process[TARGET_MAIN].get_core_x(), // message contents. retrieved sequentially by read_message().
                    process[TARGET_MAIN].get_core_y()); // message contents
  }
// Now listen for a broadcast from a leader process:
  gosub listen_for_requests;
  auto_move.move_to(move_x, move_y);
  break;
  
// MODE_GUARD is like MODE_FOLLOW but the process will follow a new leader if one is built
 case MODE_GUARD:
  if (process[TARGET_LEADER].visible() <= 0) // leader doesn't exist
  {
   mode = MODE_WANDER;
   gosub start_wandering;
   break;
  }
  if (process[TARGET_LEADER].distance() < 700
   && scan_for_threat(cos(angle, 400), sin(angle, 400), TARGET_MAIN))
  {
   special_AI(1, 1);
   mode = MODE_ATTACK;
   broadcast_target(4000, // range
                    CHANNEL_TARGET, // channel
                    0, // priority (0 just means a 1 priority message will overwrite this one)
                    TARGET_MAIN, // this target is attached to the broadcast. A listener can retrieve it with get_message_target().
                    MESSAGE_TARGET_FOUND,
                    process[TARGET_MAIN].get_core_x(), // message contents. retrieved sequentially by read_message().
                    process[TARGET_MAIN].get_core_y()); // message contents
  }
  angle_to_leader = atan2(process[TARGET_LEADER].get_core_y() - core_y, process[TARGET_LEADER].get_core_x() - core_x);
  angle_to_leader += 1024;
  move_x = process[TARGET_LEADER].get_core_x() - cos(angle_to_leader, 400);
  move_y = process[TARGET_LEADER].get_core_y() - sin(angle_to_leader, 400);
  auto_move.move_to(move_x, move_y);
  gosub listen_for_requests; // consider following any other processes that come along.
  break;
  
 case MODE_ATTACK: // attacking something it found itself
  if (process[TARGET_LEADER].visible() > 0 // leader exists (friendly targets are always visible if they exist)
   && process[TARGET_LEADER].distance() > 1200) // don't stray too far from leader
  {
   mode = MODE_GUARD;
   break;
  }
  if (process[TARGET_MAIN].visible() <= 0) // target no longer exists, or is out of range
  {
   if (target_destroyed(TARGET_MAIN))
    special_AI(1, 2);
   mode = MODE_GUARD;
   break;
  }
  auto_move.approach_target(TARGET_MAIN, 0, 300);
  if (process[TARGET_MAIN].distance() < 1000
   && arc_length(angle, atan2(process[TARGET_MAIN].get_core_y() - core_y, process[TARGET_MAIN].get_core_x() - core_x)) <= 2048)
   auto_att_fwd.fire_at(TARGET_MAIN, 0);
  break;
  

} // end of mode switch





exit;


start_wandering:
 mode = MODE_WANDER;
 move_x = 800 + random(world_x() - 1600);
 move_y = 800 + random(world_y() - 1600);
 return;


listen_for_requests:
// listen to other procs requesting followers.
  if (next_message() // returns 1 if message received since previous cycle
   && read_message() == MESSAGE_REQUEST_FOLLOWER // returns contents of message in sequence
   && random(2)) // may or may not decide to follow.
  {
   mode = MODE_GUARD;
   get_message_source(TARGET_LEADER); // saves message sender into targetting memory
  }
  return;
