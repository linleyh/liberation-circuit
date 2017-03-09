

#ifndef H_D_CODE
#define H_D_CODE


#define DTOKEN_LENGTH 32


struct dtoken_struct
{

	char name [DTOKEN_LENGTH];

};


enum
{
AUTOCODE_NONE,
AUTOCODE_STANDARD,
AUTOCODE_CHARGE,
AUTOCODE_BOMBARD,
AUTOCODE_CIRCLE_CW,
AUTOCODE_CIRCLE_ACW,
AUTOCODE_ERRATIC,
AUTOCODE_CAUTIOUS,

AUTOCODE_TYPES
};


// quadrants are used for working out which class some objects belong to (e.g. attack_forward)
enum
{
// order of quadrants must match order of directional classes (forward, left, right, back)
QUADRANT_FORWARD,
QUADRANT_LEFT,
QUADRANT_RIGHT,
QUADRANT_BACK
};


enum
{
MAIN_ATTACK_NONE, // proc doesn't attack
MAIN_ATTACK_APPROACH, // proc approaches to short range of target, then turns to face it (use this if main attack is dir forwards)
MAIN_ATTACK_CIRCLE, // [not currently used] - proc circles target (use if main attack is fixed forwards)
MAIN_ATTACK_INTERCEPT, // proc uses intercept method (use this if main attack is fixed forwards)
MAIN_ATTACK_LONG_RANGE, // proc approaches to long range then turns to face target only (use for spike). If it has retro move, will use standoff to maintain distance
};


#define AUTO_CLASS_NAME_LENGTH 16
enum
{
AUTO_CLASS_MOVE,
AUTO_CLASS_RETRO, // I don't think this is used directly, but if retro move objects are present some code is generated differently (e.g. stand_off movement calls are used instead of turn_to or intercept)
AUTO_CLASS_ATTACK_MAIN,
AUTO_CLASS_ATTACK_FRONT_DIR,
AUTO_CLASS_ATTACK_LEFT_DIR,
AUTO_CLASS_ATTACK_RIGHT_DIR,
AUTO_CLASS_ATTACK_BACK_DIR,
AUTO_CLASS_SPIKE_FRONT,
AUTO_CLASS_HARVEST,
AUTO_CLASS_ALLOCATE,
AUTO_CLASS_STABILITY,
//AUTO_CLASS_BUILD,
// need to make changes below if AUTO_CLASSES ever gets close to OBJECT_CLASSES (16) (although actually I think this is caught and will produce an error if there are too many)
AUTO_CLASSES
};


enum
{
AUTOMODE_IDLE,
AUTOMODE_MOVE,
AUTOMODE_ATTACK,
AUTOMODE_ATTACK_FOUND,
AUTOMODE_HARVEST,
AUTOMODE_HARVEST_RETURN, // returning to allocator from harvest
AUTOMODE_GUARD, // circle around target
AUTOMODE_MOVE_BUILD, // move somewhere and build


//AUTOMODE_MOVE_REPAIR, // move somewhere and repair a friendly process

AUTOMODES
};

struct dcode_state_struct
{
	int source_line; // remember that this is the index in the source_edit.line_index array, not in the source_edit.text array!
	int cursor_pos;
	int indent_level;
	int process_structure_lines;
	int error_type;
	struct source_edit_struct* ses;

	int autocode_type;
	int main_attack_type;
 int auto_class_index [AUTO_CLASSES]; // the AUTO_CLASS indices don't necessarily match the template's class indices. This array gives the template class index for an AUTO_CLASS.
 int object_type_present [OBJECT_TYPES];
 int unindexed_auto_class_present [AUTO_CLASSES]; // number of objects in class AUTO_CLASS (0 if class not present). Doesn't need to be referenced through auto_class_index.
// int unindexed_auto_class_number [AUTO_CLASSES]; // number of objects in an unindexed auto class

#define DCODE_BUFFER_LENGTH 8000
 char dcode_buffer [DCODE_BUFFER_LENGTH];

// autocode stuff
 int mobile; // 0 if immobile, 1 otherwise
 int automode [AUTOMODES];



};


int autocode(int autocode_type);

int dcode_error(const char* error_message);
void dcode_warning(const char* warning_message);



#endif
