

#process "defender"
// This process has objects with the following auto classes:
class auto_att_right;
class auto_att_left;

// The following auto classes are not currently used by any objects:
class auto_move;
class auto_retro;
class auto_att_main;
class auto_att_fwd;
class auto_att_back;
class auto_att_spike;
class auto_harvest;
class auto_allocate;
class auto_stability;

core_static_quad, 0, 
  {object_none, 0},
  {object_pulse:auto_att_right, 0},
  {object_none, 0},
  {object_pulse:auto_att_left, 0},

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

int initialised;

if (!initialised)
{
 special_AI(0, 1); // special story mission call that determines bubble chatter
 initialised = 1;
}


// All that this process does is sit around and wait until something comes near, then attack it.

int attacking;
attacking = 0;


//attacking += auto_att_fwd.attack_scan(0, 300, TARGET_FRONT);
attacking += auto_att_left.attack_scan(-2048, 300, TARGET_LEFT);
attacking += auto_att_right.attack_scan(2048, 300, TARGET_RIGHT);
//attacking += auto_att_back.attack_scan(4096, 300, TARGET_BACK);

if (attacking > 0)
 special_AI(1, 0); // may display a message
