
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
AREA_DARK_BLUE,
AREA_GREY,
AREA_RED,

STORY_AREAS
};


enum
{
MISSION_TUTORIAL1,
MISSION_TUTORIAL2,

MISSION_BLUE_1,
MISSION_BLUE_2,
MISSION_BLUE_3,
MISSION_BLUE_4,
MISSION_BLUE_5,
MISSION_BLUE_6,
MISSION_BLUE_7,
MISSION_BLUE_CAPITAL,

MISSION_GREEN_1,
MISSION_GREEN_2,
MISSION_GREEN_3,
MISSION_GREEN_4,
MISSION_GREEN_5,
MISSION_GREEN_CAPITAL,

MISSION_YELLOW_1,
MISSION_YELLOW_2,
MISSION_YELLOW_3,
MISSION_YELLOW_4,
MISSION_YELLOW_5,
MISSION_YELLOW_6,
MISSION_YELLOW_CAPITAL,

MISSION_ORANGE_1,
MISSION_ORANGE_2,
MISSION_ORANGE_3,
MISSION_ORANGE_4,
MISSION_ORANGE_5,
MISSION_ORANGE_CAPITAL,

MISSION_PURPLE_1,
MISSION_PURPLE_2,
MISSION_PURPLE_3,
MISSION_PURPLE_4,
MISSION_PURPLE_5,
MISSION_PURPLE_6,
MISSION_PURPLE_CAPITAL,

MISSION_DARK_BLUE_1,
MISSION_DARK_BLUE_2,
MISSION_DARK_BLUE_3,
MISSION_DARK_BLUE_4,
MISSION_DARK_BLUE_5,
MISSION_DARK_BLUE_CAPITAL,

MISSION_RED_1,
MISSION_RED_2,
MISSION_RED_3,
MISSION_RED_4,
MISSION_RED_5,
MISSION_RED_6,
MISSION_RED_7,
MISSION_RED_8,
MISSION_RED_9,
MISSION_RED_CAPITAL,

MISSIONS
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

 int connect [SRC_DIRECTIONS]; // default to -1

 int area_index; // which area is this region in
 int mission_index;
 int capital; // 1 if this is the main region of this area

	int defeated; // 1 if player has beaten this region, 0 otherwise
	int unlocked; // derived from defeated values when story mode loaded


};

struct story_struct
{


 struct region_struct region [STORY_REGIONS];

 struct area_struct area [STORY_AREAS];


};



void init_story(void);
void enter_story_mode(void);


#endif
