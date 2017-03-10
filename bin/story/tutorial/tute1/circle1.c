

#process "defender2"

class auto_move;
class auto_att_fwd;

core_quad_A, 0, 
  {object_pulse:auto_att_fwd, 0},
  {object_move:auto_move, 2048},
  {object_none, 0},
  {object_move:auto_move, -2048},

#code





// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder as entry 0
  TARGET_MAIN, // target of main attack
};

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

int move_x, move_y; // destination
int centre_x, centre_y; // centre of circle process is orbiting
int scan_result; // used to hold the results of a scan of nearby processes

int initialised; // will start as 0

if (!initialised)
{
// this process just moves in a circle around a point identified when it's created.
 centre_x = core_x + cos(angle + 2048, 400);
 centre_y = core_y + sin(angle + 2048, 400);
 initialised = 1;
}

int angle_to_centre;
angle_to_centre = atan2(core_y - centre_y, core_x - centre_x);
int move_angle;
move_angle = angle_to_centre + 400; // target is always a little clockwise from the current positiono

move_x = centre_x + cos(move_angle, 400);
move_y = centre_y + sin(move_angle, 400);

auto_move.move_to(move_x, move_y);


// now target and fire the left and right pulse objects:
auto_att_fwd.attack_scan(0, 400, TARGET_MAIN);
