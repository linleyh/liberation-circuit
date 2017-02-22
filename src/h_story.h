
#ifndef H_H_STORY
#define H_H_STORY

#define STORY_REGIONS 64

enum
{
SRC_R,
SRC_DR,
SRC_DL,
SRC_L,
SRC_UL,
SRC_UR,
SRC_DIRECTIONS
};

enum
{
AREA_TUTORIAL,
AREA_BLUE,
AREA_GREEN,
AREA_YELLOW,
AREA_ORANGE,
AREA_PURPLE,
//AREA_DARK_BLUE,
//AREA_GREY,
AREA_RED,

STORY_AREAS
};

/*
enum
{
MISSION_TUTORIAL1,
MISSION_TUTORIAL2,

MISSION_BLUE_1,
MISSION_BLUE_2,
MISSION_BLUE_CAPITAL,

MISSION_GREEN_1,
MISSION_GREEN_2,
MISSION_GREEN_CAPITAL,

MISSION_YELLOW_1,
MISSION_YELLOW_2,
MISSION_YELLOW_CAPITAL,

MISSION_ORANGE_1,
MISSION_ORANGE_2,
MISSION_ORANGE_CAPITAL,

MISSION_PURPLE_1,
MISSION_PURPLE_2,
MISSION_PURPLE_CAPITAL,

//MISSION_DARK_BLUE_1,
//MISSION_DARK_BLUE_2,
//MISSION_DARK_BLUE_CAPITAL,

MISSION_RED_1,
MISSION_RED_2,
MISSION_RED_CAPITAL,

MISSIONS
};
*/

enum
{
UNLOCK_NONE,

//UNLOCK_KEY,

UNLOCK_CORE_MOBILE_1,
UNLOCK_CORE_MOBILE_2,
UNLOCK_CORE_MOBILE_3,
UNLOCK_CORE_MOBILE_4,
//UNLOCK_CORE_MOBILE_5,

UNLOCK_CORE_STATIC_1,
UNLOCK_CORE_STATIC_2,
//UNLOCK_CORE_STATIC_3,

//UNLOCK_COMPONENTS_1,
//UNLOCK_COMPONENTS_2,

UNLOCK_OBJECT_INTERFACE,
UNLOCK_OBJECT_REPAIR_OTHER,
UNLOCK_OBJECT_STABILITY,

UNLOCK_OBJECT_PULSE_L,
UNLOCK_OBJECT_PULSE_XL,
//UNLOCK_OBJECT_BURST_XL,

UNLOCK_OBJECT_STREAM,
UNLOCK_OBJECT_SPIKE,
UNLOCK_OBJECT_SLICE,
UNLOCK_OBJECT_ULTRA,

UNLOCKS
// 20
// 33
};

// areas are
struct area_struct
{

	int base_colour [3];

};

struct region_struct
{

 int exists; // may not be selectable
 int visible; // 0 or 1

 int grid_x;
 int grid_y;
 int adjust_x, adjust_y;

 int connect [SRC_DIRECTIONS]; // default to -1

 int area_index; // which area is this region in
 int mission_index;
 int capital; // 1 if this is the main region of this area
 int unlock_index; // one of the UNLOCK enums

	int defeated; // 1 if player has beaten this region, 0 otherwise
	int can_be_played; // derived from defeated values when story mode loaded


};

struct story_struct
{

 int story_type; // STORY_TYPE_NORMAL or STORY_TYPE_ADVANCED, or the HARD versions

 struct region_struct region [STORY_REGIONS];

 struct area_struct area [STORY_AREAS];

 int unlock [UNLOCKS]; // determines which objects/components the player can use in story mode. Determined from area lock


};



void load_story_status_file(void);
//void init_story(void);
void enter_story_mode(int story_type);
void	story_mission_defeated(void);

void	special_AI_method(struct core_struct* core, int value1, int value2);
void special_AI_destroyed(struct core_struct* core);

#endif
