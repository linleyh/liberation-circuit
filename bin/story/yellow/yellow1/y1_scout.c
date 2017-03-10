

#process "scout"
class auto_att_fwd;
class auto_move;

core_quad_A, 0, 
  {object_pulse:auto_att_fwd, 0},
  {object_move:auto_move, 2048},
  {object_move:auto_move, 0},
  {object_move:auto_move, -2048},
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
  MODE_WANDER, // wandering randomly
  MODES
};


// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder as entry 0
  TARGET_MAIN, // main target
  TARGET_FOUND, // target found for other processes to attack 
};

int other_target_x, other_target_y;

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
int attack_x, attack_y; // location of target found.

int scan_result; // used to hold the results of a scan of nearby processes
int broadcast_count;

int initialised;
if (initialised == 0)
{
 initialised = 1;
 mode = MODE_WANDER;
 special_AI(0, 206);
 gosub start_wandering;
// listen_channel(1); // processes in this mission use channel 1 for "target found" messages
 // but scouts only send these messages; they don't receive them.
}


// What the process does next depends on its current mode
switch(mode)
{


 case MODE_WANDER:
  if (distance_from_xy(move_x, move_y) < 300)
  {
   gosub start_wandering;
   break;
  }
  auto_move.move_to(move_x, move_y);
  gosub scan_for_target;
  break;
  
} // end of mode switch


auto_att_fwd.attack_scan(0, 0, TARGET_MAIN);

if (broadcast_count > 0)
 broadcast_count --;

exit;


start_wandering:
 mode = MODE_WANDER;
 move_x = 800 + random(world_x() - 1600);
 move_y = 800 + random(world_y() - 1600);
 return;



scan_for_target:
// first, scan for a static allocator target
 scan_result = scan_single(0, // x_offset
               0, // y_offset
               TARGET_FOUND,
               0, // accept friendly (0 means ignore friendly)
               0, // minimum components
               100, // maximum components
               0b1000); // bitfield for accepting process with allocator
 if (scan_result)
 {
  attack_x = process[TARGET_FOUND].get_core_x();
  attack_y = process[TARGET_FOUND].get_core_y();
  if (broadcast_count <= 0)
  {
     broadcast_target(-1, // range (-1 means unlimited range)
                      CHANNEL_TARGET, // channel (see the listen_channel() call in other process' code)
                      0, // priority (0 just means a 1 priority message will overwrite this one)
                      TARGET_FOUND, // this target is attached to the broadcast. A listener can retrieve it with get_message_target().
                      MESSAGE_TARGET_FOUND, // message contents
                      attack_x, // message contents. retrieved sequentially by read_message().
                      attack_y); // message contents

     broadcast_count = 48;
     
     special_AI(1, 6);
   
  }
  return; 
 }
// no static target found. Now scan for medium-sized or larger enemies:
 scan_result = scan_single(0, // x_offset
               0, // y_offset
               TARGET_FOUND,
               0, // accept friendly (0 means ignore friendly)
               4, // minimum components
               100, // maximum components
               0); // bitfield for accepting all processes
 if (scan_result)
 {
  attack_x = process[TARGET_FOUND].get_core_x();
  attack_y = process[TARGET_FOUND].get_core_y();
  if (broadcast_count <= 0)
  {
     broadcast_target(-1, // range
                      CHANNEL_TARGET, // channel
                      0, // priority (0 just means a 1 priority message will overwrite this one)
                      TARGET_FOUND, // this target is attached to the broadcast. A listener can retrieve it with get_message_target().
                      MESSAGE_TARGET_FOUND,
                      attack_x, // message contents. retrieved sequentially by read_message().
                      attack_y); // message contents

     broadcast_count = 48;

     special_AI(1, 5);
   
  }
 }
 broadcast_count = 0; // nothing found, so get ready to broadcast again when next target found.
 return;
 
