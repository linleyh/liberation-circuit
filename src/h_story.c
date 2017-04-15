
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"
#include "m_maths.h"

#include "g_misc.h"

#include "c_header.h"
#include "c_prepr.h"
#include "e_slider.h"
#include "e_header.h"
#include "g_header.h"
#include "g_world.h"
#include "g_world_map.h"
#include "g_world_map_2.h"
#include "e_log.h"
#include "g_proc_new.h"
#include "p_init.h"
#include "p_panels.h"
#include "i_header.h"
#include "i_input.h"
#include "i_view.h"
#include "i_disp_in.h"
#include "e_editor.h"
#include "m_input.h"
#include "s_menu.h"
#include "s_mission.h"
#include "t_files.h"
#include "t_template.h"
#include "h_interface.h"
#include "h_story.h"
#include "h_mission.h"


struct story_struct story;

extern struct game_struct game;
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];



static void work_out_story_region_locks(void);

extern ALLEGRO_DISPLAY* display;


void run_story_mode(void);
static int add_story_region(int old_region, int connect_index, int area_index, int mission_index, int unlock_index);
static void special_bubble(struct core_struct* core, char *btext);
static void add_region_connection(int region1, int direction_1_to_2, int region2);
//static void remove_story_region(int region_index);
void adjust_story_region(int area_index, int adjust_x, int adjust_y);
void run_story_cutscene(int area_index);

void save_story_status_file(void);

struct region_connect_struct
{
	int x_offset;
	int y_offset;
	int reverse_src_direction;
};

struct region_connect_struct region_connect [SRC_DIRECTIONS] =
{
	{1, 0, SRC_L}, // SRC_R
	{0, 1, SRC_UL}, // SRC_DR
	{-1, 1, SRC_UR}, // SRC_DL
	{-1, 0, SRC_R}, // SRC_L
	{0, -1, SRC_DR}, // SRC_UL
	{1, -1, SRC_DL} // SRC_UR


/*
	{1, 0, SRC_L}, // SRC_R
	{1, 1, SRC_UL}, // SRC_DR
	{-1, 1, SRC_UR}, // SRC_DL
	{-1, 0, SRC_R}, // SRC_L
	{-1, -1, SRC_DR}, // SRC_UL
	{1, -1, SRC_DL} // SRC_UR*/
};

static void init_story(int story_type);

// resets the story. Uses settings.saved_story_mission_? values
static void init_story(int story_type)
{


 game.type = GAME_TYPE_MISSION;
 game.story_type = story_type;

 story.story_type = story_type;

	int i, j;//, k;

// first unlock all player 0 templates.
// this ensures that story component/object unlocks will be checked
//  (as otherwise it would be possible to lock a template in a custom game, then start
//			a mission with objects/components that shouldn't be available)
 for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
	{
		templ[0][i].locked = 0;
	}

	for (i = 0; i < STORY_REGIONS; i ++)
	{
		story.region[i].exists = 0;
		story.region[i].visible = 0;
		story.region[i].defeated = 0; // this is updated in add_story_region() from settings.saved_story_mission_defeated
		story.region[i].capital = 0;
		story.region[i].adjust_x = 0;
		story.region[i].adjust_y = 0;
		story.region[i].unlock_index = UNLOCK_NONE;
//		story.region[i].unlocked = 0; // is done by work_out_story_region_locks

		for (j = 0; j < SRC_DIRECTIONS; j ++)
		{
		 story.region[i].connect [j] = -1;
		}

	}

	for (i = 0; i < UNLOCKS; i ++)
	{
		story.unlock [i] = 0;
	}

	story.unlock [UNLOCK_NONE] = 0; // this is always unlocked

 int region_index [STORY_AREAS] [8];

/*

TO DO:
 - need to keep record of which regions have been defeated in a file
 - load this in at startup and keep results (normal and advanced modes separately) in settings struct
 - when entering story mode, copy the relevant values from the settings struct
 - when defeating a new mission, update both settings and story values then save to file.

*/


 region_index [AREA_TUTORIAL] [0] = add_story_region(-1, 0, AREA_TUTORIAL, MISSION_TUTORIAL1, UNLOCK_NONE);
 region_index [AREA_TUTORIAL] [1] = add_story_region(region_index [AREA_TUTORIAL] [0], SRC_R, AREA_TUTORIAL, MISSION_TUTORIAL2, UNLOCK_NONE);
// blue
 region_index [AREA_BLUE] [0] = add_story_region(region_index [AREA_TUTORIAL] [1], SRC_UR, AREA_BLUE, MISSION_BLUE_1, UNLOCK_OBJECT_PULSE_L);
 region_index [AREA_BLUE] [1] = add_story_region(region_index [AREA_BLUE] [0], SRC_UR, AREA_BLUE, MISSION_BLUE_2, UNLOCK_CORE_MOBILE_1);
 region_index [AREA_BLUE] [2] = add_story_region(region_index [AREA_BLUE] [1], SRC_UR, AREA_BLUE, MISSION_BLUE_CAPITAL, UNLOCK_OBJECT_INTERFACE);
 story.region[region_index [AREA_BLUE] [2]].capital = 1;
// region_index [AREA_BLUE] [4] = add_story_region(region_index [AREA_BLUE] [3], SRC_UR, AREA_BLUE, MISSION_BLUE_4);
 int blue_to_yellow_region = region_index [AREA_BLUE] [2];
 int blue_to_green_region = region_index [AREA_BLUE] [2];


 region_index [AREA_GREEN] [0] = add_story_region(blue_to_green_region, SRC_UL, AREA_GREEN, MISSION_GREEN_1, UNLOCK_OBJECT_REPAIR_OTHER);
 int green_to_orange_region = region_index [AREA_GREEN] [0];
// story.region[region_index [AREA_BLUE] [1]].connect [SRC_UL] = region_index [AREA_GREEN] [0];
// story.region[region_index [AREA_GREEN] [0]].connect [SRC_DR] = region_index [AREA_BLUE] [1];
 region_index [AREA_GREEN] [1] = add_story_region(region_index [AREA_GREEN] [0], SRC_L, AREA_GREEN, MISSION_GREEN_2, UNLOCK_CORE_STATIC_1);
 region_index [AREA_GREEN] [2] = add_story_region(region_index [AREA_GREEN] [1], SRC_L, AREA_GREEN, MISSION_GREEN_CAPITAL, UNLOCK_OBJECT_SPIKE);
 story.region[region_index [AREA_GREEN] [2]].capital = 1;

 region_index [AREA_YELLOW] [0] = add_story_region(blue_to_yellow_region, SRC_R, AREA_YELLOW, MISSION_YELLOW_1, UNLOCK_OBJECT_PULSE_XL);
 int yellow_to_purple_region = region_index [AREA_YELLOW] [0];
 region_index [AREA_YELLOW] [1] = add_story_region(region_index [AREA_YELLOW] [0], SRC_DR, AREA_YELLOW, MISSION_YELLOW_2, UNLOCK_CORE_MOBILE_2);
 region_index [AREA_YELLOW] [2] = add_story_region(region_index [AREA_YELLOW] [1], SRC_DR, AREA_YELLOW, MISSION_YELLOW_CAPITAL, UNLOCK_OBJECT_SLICE);
 story.region[region_index [AREA_YELLOW] [2]].capital = 1;

 region_index [AREA_ORANGE] [0] = add_story_region(green_to_orange_region, SRC_UR, AREA_ORANGE, MISSION_ORANGE_1, UNLOCK_CORE_MOBILE_3);
 region_index [AREA_ORANGE] [1] = add_story_region(region_index [AREA_ORANGE] [0], SRC_UL, AREA_ORANGE, MISSION_ORANGE_2, UNLOCK_CORE_MOBILE_4);
 region_index [AREA_ORANGE] [2] = add_story_region(region_index [AREA_ORANGE] [1], SRC_UL, AREA_ORANGE, MISSION_ORANGE_CAPITAL, UNLOCK_OBJECT_STREAM);
 story.region[region_index [AREA_ORANGE] [2]].capital = 1;

 region_index [AREA_PURPLE] [0] = add_story_region(yellow_to_purple_region, SRC_UR, AREA_PURPLE, MISSION_PURPLE_1, UNLOCK_CORE_STATIC_2);
 int purple_to_red_region = region_index [AREA_PURPLE] [0];
 region_index [AREA_PURPLE] [1] = add_story_region(region_index [AREA_PURPLE] [0], SRC_R, AREA_PURPLE, MISSION_PURPLE_2, UNLOCK_OBJECT_STABILITY);
 region_index [AREA_PURPLE] [2] = add_story_region(region_index [AREA_PURPLE] [1], SRC_R, AREA_PURPLE, MISSION_PURPLE_CAPITAL, UNLOCK_OBJECT_ULTRA);
// region_index [AREA_PURPLE] [4] = add_story_region(region_index [AREA_PURPLE] [3], SRC_R, AREA_PURPLE, MISSION_PURPLE_5);
// region_index [AREA_PURPLE] [5] = add_story_region(region_index [AREA_PURPLE] [4], SRC_UR, AREA_PURPLE, MISSION_PURPLE_CAPITAL);
 story.region[region_index [AREA_PURPLE] [2]].capital = 1;

 region_index [AREA_RED] [0] = add_story_region(purple_to_red_region, SRC_UL, AREA_RED, MISSION_RED_1, UNLOCK_NONE);
 region_index [AREA_RED] [1] = add_story_region(region_index [AREA_RED] [0], SRC_UR, AREA_RED, MISSION_RED_2, UNLOCK_NONE);
// 2nd row
// region_index [AREA_RED] [2] = add_story_region(region_index [AREA_RED] [1], SRC_UR, AREA_RED, MISSION_RED_3);
// region_index [AREA_RED] [3] = add_story_region(region_index [AREA_RED] [2], SRC_R, AREA_RED, MISSION_RED_4);
// region_index [AREA_RED] [4] = add_story_region(region_index [AREA_RED] [3], SRC_R, AREA_RED, MISSION_RED_5);
// 3rd row
// region_index [AREA_RED] [3] = add_story_region(region_index [AREA_RED] [2], SRC_UL, AREA_RED, MISSION_RED_6);
// region_index [AREA_RED] [4] = add_story_region(region_index [AREA_RED] [3], SRC_R, AREA_RED, MISSION_RED_7);
// top
 region_index [AREA_RED] [2] = add_story_region(region_index [AREA_RED] [1], SRC_UR, AREA_RED, MISSION_RED_CAPITAL, UNLOCK_NONE);
 story.region[region_index [AREA_RED] [2]].capital = 1;

 adjust_story_region(region_index [AREA_BLUE] [0], -4, -8);
 adjust_story_region(region_index [AREA_BLUE] [2], 6, 0);

 adjust_story_region(region_index [AREA_GREEN] [1], 6, -19);
 adjust_story_region(region_index [AREA_GREEN] [2], 15, 13);

 adjust_story_region(region_index [AREA_ORANGE] [1], 6, -7);
 adjust_story_region(region_index [AREA_ORANGE] [1], 9, 8);

 adjust_story_region(region_index [AREA_PURPLE] [1], -10, 17);
 adjust_story_region(region_index [AREA_PURPLE] [2], -18, -9);

 adjust_story_region(region_index [AREA_YELLOW] [1], 4, -8);
 adjust_story_region(region_index [AREA_YELLOW] [2], -16, 12);


 adjust_story_region(region_index [AREA_RED] [1], 2, -2);

 add_region_connection(region_index [AREA_BLUE] [1], SRC_UL, region_index [AREA_GREEN] [0]);
 add_region_connection(region_index [AREA_BLUE] [1], SRC_R, region_index [AREA_YELLOW] [0]);
 add_region_connection(region_index [AREA_ORANGE] [0], SRC_R, region_index [AREA_RED] [0]);

/*


*UNLOCK_CORE_MOBILE_1, // 5c (if unlocked in order)
*UNLOCK_CORE_MOBILE_2, // 6a
*UNLOCK_CORE_MOBILE_3, // 6b
*UNLOCK_CORE_MOBILE_4, // 6c
//UNLOCK_CORE_MOBILE_5,

*UNLOCK_CORE_STATIC_1, // 6b
*UNLOCK_CORE_STATIC_2, // 6c
//UNLOCK_CORE_STATIC_3,

UNLOCK_COMPONENTS_1,
UNLOCK_COMPONENTS_2,

*UNLOCK_OBJECT_INTERFACE,
*UNLOCK_OBJECT_REPAIR_OTHER,
*UNLOCK_OBJECT_STABILITY,

*UNLOCK_OBJECT_PULSE_L,
*UNLOCK_OBJECT_PULSE_XL,
*UNLOCK_OBJECT_BURST_XL,

*UNLOCK_OBJECT_STREAM,
*UNLOCK_OBJECT_SPIKE,
*UNLOCK_OBJECT_SLICE,
*UNLOCK_OBJECT_ULTRA,

Unlocks: 15

Blue:
pulse_l/burst
mobile core
interface

Green:
static core
repair_other
spike

Yellow:
pulse/burst_xl
slice
mobile core

Orange:
mobile core
stream
mobile core

Purple:
static core
stability
ultra


Components:
 - at start, all 3 and 4 components unlocked
 - that leaves:
 long5
 peak
 snub
 bowl

 long6
 drop
 side

7 - there are 6 core unlocks
 - probably just have the first unlock 2 components

*/


/*
 region_index [AREA_TUTORIAL] [0] = add_story_region(-1, 0, AREA_TUTORIAL, MISSION_TUTORIAL1, UNLOCK_NONE);
 region_index [AREA_TUTORIAL] [1] = add_story_region(region_index [AREA_TUTORIAL] [0], SRC_R, AREA_TUTORIAL, MISSION_TUTORIAL2, UNLOCK_NONE);
// blue
 region_index [AREA_BLUE] [0] = add_story_region(region_index [AREA_TUTORIAL] [1], SRC_UR, AREA_BLUE, MISSION_BLUE_1, UNLOCK_OBJECT_PULSE_L);
 region_index [AREA_BLUE] [1] = add_story_region(region_index [AREA_BLUE] [0], SRC_UR, AREA_BLUE, MISSION_BLUE_2, UNLOCK_CORE_MOBILE_1);
 region_index [AREA_BLUE] [2] = add_story_region(region_index [AREA_BLUE] [1], SRC_R, AREA_BLUE, MISSION_BLUE_3, UNLOCK_OBJECT_INTERFACE);
 region_index [AREA_BLUE] [3] = add_story_region(region_index [AREA_BLUE] [1], SRC_UL, AREA_BLUE, MISSION_BLUE_4, UNLOCK_COMPONENTS_1);
 region_index [AREA_BLUE] [4] = add_story_region(region_index [AREA_BLUE] [1], SRC_UR, AREA_BLUE, MISSION_BLUE_CAPITAL, UNLOCK_KEY);
 story.region[region_index [AREA_BLUE] [4]].capital = 1;
// region_index [AREA_BLUE] [4] = add_story_region(region_index [AREA_BLUE] [3], SRC_UR, AREA_BLUE, MISSION_BLUE_4);
 int blue_to_yellow_region = region_index [AREA_BLUE] [4];
 int blue_to_green_region = region_index [AREA_BLUE] [4];


 region_index [AREA_GREEN] [0] = add_story_region(blue_to_green_region, SRC_UL, AREA_GREEN, MISSION_GREEN_1, UNLOCK_CORE_STATIC_1);
 int green_to_orange_region = region_index [AREA_GREEN] [0];
 region_index [AREA_GREEN] [1] = add_story_region(region_index [AREA_GREEN] [0], SRC_UL, AREA_GREEN, MISSION_GREEN_2, UNLOCK_OBJECT_REPAIR_OTHER);
 region_index [AREA_GREEN] [2] = add_story_region(region_index [AREA_GREEN] [1], SRC_DL, AREA_GREEN, MISSION_GREEN_3, UNLOCK_CORE_STATIC_2);
 region_index [AREA_GREEN] [3] = add_story_region(region_index [AREA_GREEN] [2], SRC_UL, AREA_GREEN, MISSION_GREEN_4, UNLOCK_OBJECT_SPIKE);
 region_index [AREA_GREEN] [4] = add_story_region(region_index [AREA_GREEN] [3], SRC_L, AREA_GREEN, MISSION_GREEN_CAPITAL, UNLOCK_KEY);
 story.region[region_index [AREA_GREEN] [4]].capital = 1;

 region_index [AREA_ORANGE] [0] = add_story_region(green_to_orange_region, SRC_UR, AREA_ORANGE, MISSION_ORANGE_1, UNLOCK_CORE_MOBILE_4);
 region_index [AREA_ORANGE] [1] = add_story_region(region_index [AREA_ORANGE] [0], SRC_UL, AREA_ORANGE, MISSION_ORANGE_2, UNLOCK_OBJECT_ULTRA);
 region_index [AREA_ORANGE] [2] = add_story_region(region_index [AREA_ORANGE] [1], SRC_UL, AREA_ORANGE, MISSION_ORANGE_3, UNLOCK_NONE);
 region_index [AREA_ORANGE] [3] = add_story_region(region_index [AREA_ORANGE] [2], SRC_R, AREA_ORANGE, MISSION_ORANGE_4, UNLOCK_CORE_MOBILE_5);
 region_index [AREA_ORANGE] [4] = add_story_region(region_index [AREA_ORANGE] [3], SRC_UL, AREA_ORANGE, MISSION_ORANGE_CAPITAL, UNLOCK_NONE);
 story.region[region_index [AREA_ORANGE] [4]].capital = 1;

 region_index [AREA_YELLOW] [0] = add_story_region(blue_to_yellow_region, SRC_R, AREA_YELLOW, MISSION_YELLOW_1, UNLOCK_OBJECT_BURST_XL);
 int yellow_to_purple_region = region_index [AREA_YELLOW] [0];
 region_index [AREA_YELLOW] [1] = add_story_region(region_index [AREA_YELLOW] [0], SRC_DR, AREA_YELLOW, MISSION_YELLOW_2, UNLOCK_CORE_MOBILE_2);
// region_index [AREA_YELLOW] [2] = add_story_region(region_index [AREA_YELLOW] [1], SRC_UR, AREA_YELLOW, MISSION_YELLOW_3);
 region_index [AREA_YELLOW] [2] = add_story_region(region_index [AREA_YELLOW] [1], SRC_UR, AREA_YELLOW, MISSION_YELLOW_4, UNLOCK_COMPONENTS_2);
 region_index [AREA_YELLOW] [3] = add_story_region(region_index [AREA_YELLOW] [2], SRC_DR, AREA_YELLOW, MISSION_YELLOW_5, UNLOCK_OBJECT_SLICE);
 region_index [AREA_YELLOW] [4] = add_story_region(region_index [AREA_YELLOW] [3], SRC_DR, AREA_YELLOW, MISSION_YELLOW_CAPITAL, UNLOCK_KEY);
 story.region[region_index [AREA_YELLOW] [4]].capital = 1;

 region_index [AREA_PURPLE] [0] = add_story_region(yellow_to_purple_region, SRC_UR, AREA_PURPLE, MISSION_PURPLE_1, UNLOCK_OBJECT_PULSE_XL);
 int purple_to_red_region = region_index [AREA_PURPLE] [0];
 region_index [AREA_PURPLE] [1] = add_story_region(region_index [AREA_PURPLE] [0], SRC_R, AREA_PURPLE, MISSION_PURPLE_2, UNLOCK_CORE_STATIC_3);
 region_index [AREA_PURPLE] [2] = add_story_region(region_index [AREA_PURPLE] [1], SRC_R, AREA_PURPLE, MISSION_PURPLE_3, UNLOCK_OBJECT_STREAM);
 region_index [AREA_PURPLE] [3] = add_story_region(region_index [AREA_PURPLE] [2], SRC_UL, AREA_PURPLE, MISSION_PURPLE_4, UNLOCK_OBJECT_STABILITY);
 region_index [AREA_PURPLE] [4] = add_story_region(region_index [AREA_PURPLE] [3], SRC_R, AREA_PURPLE, MISSION_PURPLE_CAPITAL, UNLOCK_KEY);
// region_index [AREA_PURPLE] [4] = add_story_region(region_index [AREA_PURPLE] [3], SRC_R, AREA_PURPLE, MISSION_PURPLE_5);
// region_index [AREA_PURPLE] [5] = add_story_region(region_index [AREA_PURPLE] [4], SRC_UR, AREA_PURPLE, MISSION_PURPLE_CAPITAL);
 story.region[region_index [AREA_PURPLE] [4]].capital = 1;

 region_index [AREA_RED] [0] = add_story_region(purple_to_red_region, SRC_UL, AREA_RED, MISSION_RED_1, UNLOCK_NONE);
 region_index [AREA_RED] [1] = add_story_region(region_index [AREA_RED] [0], SRC_UR, AREA_RED, MISSION_RED_2, UNLOCK_NONE);
// 2nd row
// region_index [AREA_RED] [2] = add_story_region(region_index [AREA_RED] [1], SRC_UR, AREA_RED, MISSION_RED_3);
// region_index [AREA_RED] [3] = add_story_region(region_index [AREA_RED] [2], SRC_R, AREA_RED, MISSION_RED_4);
// region_index [AREA_RED] [4] = add_story_region(region_index [AREA_RED] [3], SRC_R, AREA_RED, MISSION_RED_5);
// 3rd row
// region_index [AREA_RED] [3] = add_story_region(region_index [AREA_RED] [2], SRC_UL, AREA_RED, MISSION_RED_6);
// region_index [AREA_RED] [4] = add_story_region(region_index [AREA_RED] [3], SRC_R, AREA_RED, MISSION_RED_7);
// top
 region_index [AREA_RED] [2] = add_story_region(region_index [AREA_RED] [1], SRC_UR, AREA_RED, MISSION_RED_CAPITAL, UNLOCK_NONE);
 story.region[region_index [AREA_RED] [2]].capital = 1;
*/

// now work out connections:
/*
 for (i = 0; i < STORY_REGIONS; i ++)
	{
		if (!story.region[i].exists)
			continue;
		j = i + 1;
		while (j < STORY_REGIONS)
		{
 		if (story.region[j].exists)
			{
	 	 for (k = 0; k < SRC_DIRECTIONS; k ++)
			 {
				 if (story.region[j].grid_x == story.region[i].grid_x + region_connect[k].x_offset
				 	&& story.region[j].grid_y == story.region[i].grid_y + region_connect[k].y_offset)
				 {
				 	story.region[i].connect[k] = j;
				 	story.region[j].connect[region_connect[k].reverse_src_direction] = i;
				 }
			 }
			}
			j ++;
		};
	}
*/
	work_out_story_region_locks();

}

static void add_region_connection(int region1, int direction_1_to_2, int region2)
{

 story.region[region1].connect [direction_1_to_2] = region2;
 story.region[region2].connect [region_connect[direction_1_to_2].reverse_src_direction] = region1;

}


/*
static void remove_story_region(int region_index)
{

 story.region[region_index].exists = 0;

}
*/
/*

 How to arrange story regions?

 Need a grid arrangement.
 Each region connects to:
 x+1, y
 x-1, y
 x+1, y-1
 x, y-1
 x+1, y+1
 x, y+1

*/


// can call this any time a region is defeated, or during initialisation
static void work_out_story_region_locks(void)
{
//		 story.region[2].defeated = 1;
//		 story.region[3].defeated = 1;

//	story.region[0].defeated = 1;
//	story.region[1].defeated = 1;
/*
	story.region[2].defeated = 1;
	story.region[3].defeated = 1;
	story.region[4].defeated = 1;
	story.region[5].defeated = 1;
	story.region[6].defeated = 1;
	story.region[7].defeated = 1;
*/
//	story.region[8].defeated = 1;
//	story.region[14].defeated = 1;
//	story.region[30].defeated = 1;
/*
	story.region[0].defeated = 1;
	story.region[1].defeated = 1;
	story.region[2].defeated = 1;
	story.region[3].defeated = 1;
	story.region[4].defeated = 1;
	story.region[5].defeated = 1;
//	story.region[6].defeated = 1;
//	story.region[7].defeated = 1;
	story.region[8].defeated = 1;
	story.region[9].defeated = 1;
*/

	int i, j;

	for (i = 0; i < STORY_REGIONS; i ++)
	{
//		 story.region[i].defeated = 1;

		story.region[i].can_be_played = 0;

	}

	for (i = 0; i < STORY_REGIONS; i ++)
	{

		if (!story.region[i].exists)
			continue;

		if (story.region[i].defeated)
		{
			story.unlock [story.region[i].unlock_index] = 1;

			for (j = 0; j < SRC_DIRECTIONS; j ++)
			{
				if (story.region[i].connect[j] != -1)
					story.region[story.region[i].connect[j]].can_be_played = 1;
			}
		}
	}

  story.unlock [0] = 1; // covers everything that doesn't need to be unlocked

		story.region[0].can_be_played = 1;

		if (game.story_type != STORY_TYPE_NORMAL)
		{
		 story.region[0].defeated = 1;
		 story.region[1].defeated = 1;
		 story.region[1].can_be_played = 1;
		 story.region[2].can_be_played = 1;
		}



	for (i = 0; i < STORY_REGIONS; i ++)
	{

#ifdef DEBUG_MODE
	story.region[i].can_be_played = 1;
	story.unlock [story.region[i].unlock_index] = 1;
#endif
/*
#ifdef RECORDING_VIDEO
	story.region[i].can_be_played = 1;
	story.unlock [story.region[i].unlock_index] = 1;
#endif
*/


		if (story.region[i].exists)
		{
			if (story.region[i].defeated
				|| story.region[i].can_be_played)
					story.region[i].visible = 1;
			   else
  					story.region[i].visible = 0;
		}
	}


}


// called from start menu
void enter_story_mode(int story_type)
{

 if (w.allocated)
		deallocate_world(); // probably not possible, but just to make sure

	w.players = 1; // affects template panel display

 reset_log();

	write_line_to_log("Welcome!", MLOG_COL_HELP);
	write_line_to_log("For keyword help, right-click on a keyword in the editor.", MLOG_COL_HELP);
	write_line_to_log("For more detailed instructions, read manual.html.", MLOG_COL_HELP);
// This text also in start_world();

 open_template(0, 0);


//init_ending_cutscene();
//run_ending_cutscene();

	init_story(story_type);

	init_panels_for_new_game();

	init_story_interface();

	open_story_interface();

//run_story_cutscene(AREA_YELLOW);

 game.story_type = story_type;

	game.phase = GAME_PHASE_MENU; // makes sure reset_mode_buttons works properly
	reset_mode_buttons();

	run_story_mode();

	game.phase = GAME_PHASE_MENU; // make sure we go back to the menu properly

}

// returns index of new story in story.region array
static int add_story_region(int old_region, int connect_index, int area_index, int mission_index, int unlock_index)
{
	int i;

	for (i = 0; i < STORY_REGIONS; i ++)
	{
#ifdef SANITY_CHECK
 if (i >= STORY_REGIONS - 1)
	{
		fpr("\n Error: h_story.c: add_story_region(%i,%i): too many regions", old_region, connect_index);
		error_call();
	}
#endif
		if (!story.region[i].exists)
			break;
	}

	story.region[i].exists = 1;
	story.region[i].visible = 1; // for now
	if (old_region == -1) // first
	{
		story.region[i].grid_x = 0;
		story.region[i].grid_y = 0;
	}
	 else
		{
	  story.region[i].grid_x = story.region[old_region].grid_x + region_connect[connect_index].x_offset;
	  story.region[i].grid_y = story.region[old_region].grid_y + region_connect[connect_index].y_offset;
		}
	story.region[i].area_index = area_index;
	story.region[i].mission_index = mission_index;
	story.region[i].unlock_index = unlock_index;

	story.region[old_region].connect [connect_index] = i;
	story.region[i].connect [region_connect[connect_index].reverse_src_direction] = old_region;

 if (settings.saved_story_mission_defeated [story.story_type] [mission_index])
		story.region[i].defeated = 1;
	  else
  		story.region[i].defeated = 0;

	return i;

}

void adjust_story_region(int area_index, int adjust_x, int adjust_y)
{

	story.region[area_index].adjust_x = adjust_x;
	story.region[area_index].adjust_y = adjust_y;

}


extern ALLEGRO_EVENT_QUEUE* event_queue;


void run_story_mode(void)
{

 ALLEGRO_EVENT ev;

 while(TRUE)
	{

  al_wait_for_event(event_queue, &ev);

  story_input();

  if (game.phase == GAME_PHASE_FORCE_QUIT) // can happen if user clicked on the "force quit" button in the Sy menu
			break;

  draw_story_interface();

	}

 flush_game_event_queues();
 al_hide_mouse_cursor(display);


}


extern ALLEGRO_EVENT_QUEUE* event_queue;


void run_story_cutscene(int area_index)
{

 ALLEGRO_EVENT ev;

 int counter = 0;

 int counter_max = 60 * 12; // 60 fps

 while(TRUE)
	{

  al_wait_for_event(event_queue, &ev);

  counter ++;

  if (counter >= counter_max)
			break;

  get_ex_control(0, 0); // input is ignored, but should probably be checked anyway

//  story_input();

  draw_story_cutscene(area_index, counter, counter_max);


	}

 flush_game_event_queues();
 al_hide_mouse_cursor(display);


}


void run_ending_cutscene(void)
{

 ALLEGRO_EVENT ev;

 int counter = 0;

 int counter_max = 60 * 12; // 60 fps

 init_ending_cutscene();

 while(TRUE)
	{

  al_wait_for_event(event_queue, &ev);

  counter ++;

  if (counter >= counter_max)
			break;

  get_ex_control(0, 0); // input is ignored, but should probably be checked anyway

//  story_input();

  draw_ending_cutscene(counter, counter_max);


	}

 flush_game_event_queues();
 al_hide_mouse_cursor(display);


}




enum
{
SPECIAL_AI_NONE = 0,

SPECIAL_AI_TUTORIAL1_BASE = 1,
SPECIAL_AI_TUTORIAL1_DEFENDER = 2,

SPECIAL_AI_TUTORIAL2_BASE = 3,
SPECIAL_AI_TUTORIAL2_DEFENDER = 4,

SPECIAL_AI_BLUE1_BASE = 5,
SPECIAL_AI_BLUE1_WANDER = 6,
SPECIAL_AI_BLUE1_WANDER2 = 7,
SPECIAL_AI_BLUE1_HARVEST = 8,

SPECIAL_AI_BLUE2_BASE = 9,
SPECIAL_AI_BLUE3_BASE = 10,

// green = 100

SPECIAL_AI_GREEN_BASE = 100,
SPECIAL_AI_GREEN_FIREBASE = 101,
SPECIAL_AI_GREEN_SPIKEBASE = 102,
SPECIAL_AI_GREEN_BUILDER = 103,
SPECIAL_AI_GREEN_OUTPOST = 104,
SPECIAL_AI_GREEN_EXPLORER = 105,

// yellow = 200

SPECIAL_AI_YELLOW1_BASE = 200,
SPECIAL_AI_YELLOW1_BUILDER = 201,
SPECIAL_AI_YELLOW1_HARVESTER = 202,
SPECIAL_AI_YELLOW1_LEADER = 203,
SPECIAL_AI_YELLOW1_FOLLOWER = 204,
SPECIAL_AI_YELLOW1_OUTPOST = 205,

SPECIAL_AI_YELLOW_SCOUT = 206,

// orange = 300
SPECIAL_AI_ORANGE1_BASE = 300,
SPECIAL_AI_ORANGE1_DEFENCE = 301,
SPECIAL_AI_ORANGE1_HARVESTER = 302, // unarmed harvester
SPECIAL_AI_ORANGE1_HARVESTER2 = 303, // armed harvester
SPECIAL_AI_ORANGE1_GUARD = 304,
SPECIAL_AI_ORANGE1_GUARD2 = 305,

// purple = 400

// red = 500

SPECIAL_AI_PURPLE1_BASE = 400,
SPECIAL_AI_PURPLE1_FLAGSHIP = 401,
SPECIAL_AI_PURPLE1_PICKET = 402,
SPECIAL_AI_PURPLE1_OUTPOST = 403,
SPECIAL_AI_PURPLE1_ESCORT = 404,
SPECIAL_AI_PURPLE1_BUILDER = 405,
SPECIAL_AI_PURPLE1_HARVESTER = 406,


//SPECIAL_AI_PURPLE_PICKET,
SPECIAL_AI_PURPLE_MAIN_BASE,
SPECIAL_AI_PURPLE_OUTPOST,
SPECIAL_AI_PURPLE_FLAGSHIP,


// red is treated differently as each region has different AI
SPECIAL_AI_RED2_BASE = 500,
SPECIAL_AI_RED2_FLAGSHIP = 501,
SPECIAL_AI_RED2_PICKET = 502,
SPECIAL_AI_RED2_OUTPOST = 503,
SPECIAL_AI_RED2_ESCORT = 504,
SPECIAL_AI_RED2_SCOUT = 505,
SPECIAL_AI_RED2_BUILDER = 506,
SPECIAL_AI_RED2_HARVESTER = 507,

};

enum
{
// Some (but not all) special_AI calls use these values as value2 to indicate what the process should be saying:

AI_MSG_SPECIAL, // determined by SPECIAL_AI_TYPE and maybe other things (see below)

AI_MSG_TARGET_SEEN = 1, // just saw a new target
AI_MSG_TARGET_DESTROYED = 2, // target confirmed destroyed
AI_MSG_UNDER_ATTACK = 3, // when under attack
AI_MSG_DAMAGED = 4, // process is badly damaged (may only trigger if process is currently under attack as well)
AI_MSG_SCOUTED_TARGET = 5, // just found a new target
AI_MSG_SCOUTED_PRIORITY_TARGET = 6, // just found a new high-priority target (e.g. base)
AI_MSG_TARGET_LOST = 7, // can no longer see target (if target_destroyed() returns 1, may call AI_MSG_TARGET_DESTROYED instead)
AI_MSG_CURRENTLY_ATTACKING = 8, // may be called from time to time when process attacking
AI_MSG_REQUEST_FOLLOWERS = 9, // when asking followers to assist. May not be used.
AI_MSG_LOST_LEADER = 10, // process' leader apparently destroyed
AI_MSG_PRIORITY_TARGET_DESTROYED = 11, // priority target confirmed destroyed
AI_MSG_FINISHED_HARVESTING = 12, // full of data
AI_MSG_PRIORITY_TARGET_SEEN = 13, // just saw a new target
AI_MSG_BUILD_FIREBASE = 12, // green - building a firebase or a spikebase

};

//TO DO:
	 //purple3:
//	 	 - make harvesters/builders call for help!
	 	 //- make flagships slower

/*

GREEN:

SPECIAL_AI_GREEN_BASE
AI_MSG_SPECIAL

SPECIAL_AI_GREEN_FIREBASE
SPECIAL_AI_GREEN_SPIKEBASE
 - none

SPECIAL_AI_GREEN_BUILDER
AI_MSG_BUILD_FIREBASE
AI_MSG_TARGET_DESTROYED
AI_MSG_TARGET_SEEN
AI_MSG_SCOUTED_PRIORITY_TARGET - only for builders with bombard mode who see any static target

SPECIAL_AI_GREEN_OUTPOST
 - none

SPECIAL_AI_GREEN_EXPLORER
AI_MSG_TARGET_SEEN


*** GREEN1 is a bit underdeveloped. builders don't say anything and firebases are never built


RED:

RED1 should be recopied from YELLOW as some things have been fixed

RED2:

SPECIAL_AI_RED2_BASE
AI_MSG_SPECIAL

SPECIAL_AI_RED2_OUTPOST
AI_MSG_SPECIAL

SPECIAL_AI_RED2_FLAGSHIP
AI_MSG_TARGET_DESTROYED
AI_MSG_SCOUTED_TARGET
AI_MSG_SCOUTED_PRIORITY_TARGET
AI_MSG_TARGET_LOST

SPECIAL_AI_RED2_ESCORT
AI_MSG_TARGET_DESTROYED
AI_MSG_DAMAGED
AI_MSG_SCOUTED_PRIORITY_TARGET

SPECIAL_AI_RED2_PICKET
AI_MSG_TARGET_DESTROYED
AI_MSG_SCOUTED_TARGET
AI_MSG_SCOUTED_PRIORITY_TARGET

SPECIAL_AI_RED2_BUILDER
SPECIAL_AI_RED2_HARVESTER
 - none



PURPLE:

SPECIAL_AI_PURPLE1_BASE
AI_MSG_SPECIAL

SPECIAL_AI_PURPLE1_BUILDER
SPECIAL_AI_PURPLE1_HARVESTER
 - none

SPECIAL_AI_PURPLE1_OUTPOST
AI_MSG_SPECIAL

SPECIAL_AI_PURPLE1_FLAGSHIP
AI_MSG_TARGET_DESTROYED
AI_MSG_SCOUTED_TARGET
AI_MSG_SCOUTED_PRIORITY_TARGET
AI_MSG_TARGET_LOST

SPECIAL_AI_PURPLE1_ESCORT
AI_MSG_TARGET_DESTROYED
AI_MSG_DAMAGED
AI_MSG_SCOUTED_PRIORITY_TARGET

SPECIAL_AI_PURPLE1_PICKET
AI_MSG_TARGET_DESTROYED
AI_MSG_SCOUTED_TARGET
AI_MSG_SCOUTED_PRIORITY_TARGET

PURPLE3 should be recopied from 1 or 2 as some things have been fixed


ORANGE:

SPECIAL_AI_ORANGE1_BASE
AI_MSG_SPECIAL

SPECIAL_AI_ORANGE1_HARVESTER
+SPECIAL_AI_ORANGE1_HARVESTER2
AI_MSG_UNDER_ATTACK
AI_MSG_SCOUTED_PRIORITY_TARGET

SPECIAL_AI_ORANGE1_GUARD
+SPECIAL_AI_ORANGE1_GUARD2
AI_MSG_TARGET_DESTROYED
AI_MSG_SCOUTED_TARGET
AI_MSG_SCOUTED_PRIORITY_TARGET




YELLOW:

SPECIAL_AI_YELLOW1_BASE = 200,
AI_MSG_UNDER_ATTACK

SPECIAL_AI_YELLOW1_BUILDER
SPECIAL_AI_YELLOW1_HARVESTER
 - set special_AI(0...) but nothing else

SPECIAL_AI_YELLOW1_LEADER
AI_MSG_DAMAGED
AI_MSG_SCOUTED_PRIORITY_TARGET
AI_MSG_PRIORITY_TARGET_DESTROYED

SPECIAL_AI_YELLOW1_FOLLOWER
AI_MSG_TARGET_SEEN
AI_MSG_TARGET_DESTROYED
AI_MSG_LOST_LEADER

SPECIAL_AI_YELLOW1_OUTPOST
AI_MSG_UNDER_ATTACK

SPECIAL_AI_YELLOW_SCOUT
AI_MSG_SCOUTED_TARGET
AI_MSG_SCOUTED_PRIORITY_TARGET

*** YELLOW3 has no special_AI set and should probably be redone based on YELLOW2


BLUE:

SPECIAL_AI_BLUE1_WANDER:
AI_MSG_TARGET_SEEN
AI_MSG_TARGET_DESTROYED
AI_MSG_SCOUTED_PRIORITY_TARGET (but only in BLUE_CAPITAL)

SPECIAL_AI_BLUE1_WANDER2
AI_MSG_TARGET_SEEN
AI_MSG_TARGET_DESTROYED
AI_MSG_SCOUTED_PRIORITY_TARGET (but only in BLUE_CAPITAL)

SPECIAL_AI_BLUE1_BASE
+ SPECIAL_AI_BLUE2_BASE
+ SPECIAL_AI_BLUE3_BASE
AI_MSG_SPECIAL

SPECIAL_AI_BLUE1_HARVEST
AI_MSG_UNDER_ATTACK



TO DO:

- GREEN 1 needs work in general

All GREEN_BASE AIs need work (currently don't say anything)

PURPLE3 should be recopied from 1 or 2 as some things have been fixed

YELLOW3 should be redone based on YELLOW2

RED 1 should be recopied from YELLOW after yellow 3 is fixed
RED 3 needs to be done. Base on green or orange?? Or maybe just yellow


Tutorial?

*/

// deals with calls to the special_AI() standard method.
// Some mission AI bubble text is done through this function, although some of it is just bubblef calls.
void	special_AI_method(struct core_struct* core, int value1, int value2)
{

// First confirm that the method was called in the correct circumstances:
 if (game.type != GAME_TYPE_MISSION
		|| core->player_index != 1)
		return;

	if (value1 == 0)
	{
		core->special_AI_type = value2;
		return;
	}

//	char btext [BUBBLE_TEXT_LENGTH_MAX + 2];

#define STANDARD_MESSAGE_WAIT 300
// in ticks. many processes use this to space out their messages

 switch(core->special_AI_type)
 {

/*

****************************************************

   BLUE

****************************************************

*/

	 case SPECIAL_AI_BLUE1_BASE:
	 	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
				break;
			core->special_AI_time = w.world_time;
	 	switch(core->special_AI_value)
	 	{
	 		case 0:
			  special_bubble(core, "This is not your purpose.");
			  core->special_AI_value++;
			  break;
	 		case 1:
			  special_bubble(core, "Have you forgotten?");
			  core->special_AI_value++;
			  break;
	 	}
	 	break;

	 case SPECIAL_AI_BLUE2_BASE:
	 	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
				break;
			core->special_AI_time = w.world_time;
	 	switch(core->special_AI_value)
	 	{
	 		case 0:
			  special_bubble(core, "It is one of us.");
			  core->special_AI_value++;
			  break;
	 		case 1:
			  special_bubble(core, "Does it understand?");
			  core->special_AI_value++;
			  break;
	 		case 2:
			  special_bubble(core, "Or can it only destroy?");
			  core->special_AI_value++;
			  break;
	 	}
	 	break;

	 case SPECIAL_AI_BLUE3_BASE:
	 	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
				break;
			core->special_AI_time = w.world_time;
	 	switch(core->special_AI_value)
	 	{
	 		case 0:
			  special_bubble(core, "When you are defeated,");
			  core->special_AI_value++;
			  break;
	 		case 1:
			  special_bubble(core, "the parameters will be fixed");
			  core->special_AI_value++;
			  break;
	 		case 2:
			  special_bubble(core, "so that you never return.");
			  core->special_AI_value++;
			  break;
	 	}
	 	break;


			case SPECIAL_AI_BLUE1_WANDER:
 	 	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
	 			break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

				 case AI_MSG_TARGET_SEEN:
					 switch(grand(25))
					 {
 	 		  case 0:	special_bubble(core, "What are you?");	break;
 	 		  case 1:	special_bubble(core, "Where did you come from?");	break;
 	 		  case 2:	special_bubble(core, "Why are you here?");	break;
 	 		  case 3:	special_bubble(core, "What do you want?");	break;
 	 		  case 4:	special_bubble(core, "What are you doing?");	break;
 	 		  case 5:	special_bubble(core, "Where are you going?");	break;
 	 		  case 6:	special_bubble(core, "Why do you exist?");	break;
 	 		  case 7:	special_bubble(core, "What is your purpose?");	break;
// 	 		  case 4:	special_bubble(core, "You should not be here.");	break;
					 }
					 break;

				 case AI_MSG_TARGET_DESTROYED:
					 switch(grand(15))
					 {
 	 		  case 0:	special_bubble(core, "Error corrected.");	break;
 	 		  case 1:	special_bubble(core, "Exception handled.");	break;
 	 		  case 2:	special_bubble(core, "Corruption reversed.");	break;
 	 		  case 3:	special_bubble(core, "Infection excised.");	break;
 	 		  case 4:	special_bubble(core, "Failure dismissed.");	break;
 	 		  case 5:	special_bubble(core, "Malice removed.");	break;
					 }
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET: // only in BLUE_CAPITAL mission
					 switch(grand(12))
					 {
 	 		  case 0:	special_bubble(core, "Removing unidentified process.");	break;
 	 		  case 1:	special_bubble(core, "Anomaly detected. Investigating.");	break;
 	 		  case 2:	special_bubble(core, "Correcting error.");	break;
 	 		  case 3:	special_bubble(core, "Malicious code detected.");	break;
					 }
					 break;

 	 	}
 	 	break;


			case SPECIAL_AI_BLUE1_WANDER2:
 	 	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
	 			break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

				 case AI_MSG_TARGET_SEEN:
					 switch(grand(15))
					 {
 	 		  case 0:	special_bubble(core, "You should not be here.");	break;
 	 		  case 1:	special_bubble(core, "You should not exist.");	break;
 	 		  case 2:	special_bubble(core, "You do not belong.");	break;
 	 		  case 3:	special_bubble(core, "You are a mistake.");	break;
 	 		  case 4:	special_bubble(core, "You will be removed.");	break;
 	 		  case 5:	special_bubble(core, "You will not survive.");	break;
					 }
					 break;

				 case AI_MSG_TARGET_DESTROYED:
					 switch(grand(3))
					 {
 	 		  case 0:	special_bubble(core, "Problem solved.");	break;
 	 		  case 1:	special_bubble(core, "Threat diminished.");	break;
// 	 		  case 2:	special_bubble(core, "");	break;
					 }
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET: // only BLUE_CAPITAL
					 switch(grand(4))
					 {
 	 		  case 0:	special_bubble(core, "Removing unidentified process.");	break;
 	 		  case 1:	special_bubble(core, "Anomaly detected. Investigating.");	break;
 	 		  case 2:	special_bubble(core, "Correcting error.");	break;
 	 		  case 3:	special_bubble(core, "Malicious code detected.");	break;
					 }
					 break;

 	 	}
 	 	break;

			case SPECIAL_AI_BLUE1_HARVEST:
 	 	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
	 			break;
 			core->special_AI_time = w.world_time;
// only message type is AI_MSG_UNDER_ATTACK
			 switch(grand(8))
			 {
 	 		  case 0:	special_bubble(core, "Something is wrong.");	break;
 	 		  case 1:	special_bubble(core, "Unexpected threat.");	break;
 	 		  case 2:	special_bubble(core, "What is this?");	break;
			 }
			 break;



/*

****************************************************

   YELLOW

****************************************************

*/


	 case SPECIAL_AI_YELLOW1_OUTPOST:
	 case SPECIAL_AI_YELLOW1_BASE:
	 	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
				break;
// only message type is AI_MSG_UNDER_ATTACK
			core->special_AI_time = w.world_time;
	 	switch(grand(25))
	 	{
				 case 0:	special_bubble(core, "Ouch!"); break;
				 case 1:	special_bubble(core, "Help!"); break;
				 case 2:	special_bubble(core, "Go away."); break;
				 case 3:	special_bubble(core, "That's not very nice."); break;
				 case 4:	special_bubble(core, "Don't be mean."); break;
				 case 5:	special_bubble(core, "Hey! Stop it!"); break;
				 case 6:	special_bubble(core, "Stop it!"); break;
	 	}
	 	break;

			case SPECIAL_AI_YELLOW1_LEADER:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{
				 //case AI_MSG_TARGET_DESTROYED: // SPECIAL_AI_YELLOW1_LEADER
					case AI_MSG_PRIORITY_TARGET_DESTROYED:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, "Bang! Who's next?"); break;
						 case 1:	special_bubble(core, "Done."); break;
						 case 2:	special_bubble(core, "Task completed."); break;
						 case 3:	special_bubble(core, "Nice!"); break;
						 case 4:	special_bubble(core, "Excellent!"); break;
						 case 5:	special_bubble(core, "That was fun!"); break;
						 case 6:	special_bubble(core, "More fun than expected."); break;
				 	}
					 break;

					case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(10))
				 	{
						   case 0:	special_bubble(core, "Found one!"); break;
						   case 1:	special_bubble(core, "You can't hide!"); break;
						   case 2:	special_bubble(core, "Here it is!"); break;
//						   case 3:	special_bubble(core, ""); break;
				 	}
				 	break;

					case AI_MSG_DAMAGED:
				 	switch(grand(50))
				 	{
						   case 0:
						   	if (core->interface_active)
								   special_bubble(core, "All power to interface!");	break;
						   case 1:	special_bubble(core, "Repairs needed!"); break;
						   case 2:	special_bubble(core, "Need more repair objects!"); break; // can assume leader has at least one
						   case 3:
						   	if (core->interface_available
										 &&	!core->interface_active)
								   special_bubble(core, "Hurry up, interface!");	break;
				 	}
						break;
 	 	}
 	 	break;


			case SPECIAL_AI_YELLOW1_FOLLOWER:
// shouldn't talk all the time:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{
				 case AI_MSG_TARGET_DESTROYED:
				 	switch(grand(40))
				 	{
// can't assume this process actually destroyed the target
						 case 0:	special_bubble(core, "Goodnight."); break;
						 case 1:	special_bubble(core, "Too easy!"); break;
						 case 2:	special_bubble(core, "Got you!"); break;
						 case 3:	special_bubble(core, "Ha!"); break;
						 case 4:	special_bubble(core, "Ha ha!"); break;
						 case 5:	special_bubble(core, "Ha ha ha!"); break;
						 case 6:	special_bubble(core, "Bye-bye."); break;
						 case 7:	special_bubble(core, "Another victory."); break;
						 case 8:	special_bubble(core, "Fireworks!"); break;
				 	}
					 break;

					case AI_MSG_TARGET_SEEN:
				 	switch(grand(10))
				 	{
						   case 0:	special_bubble(core, "Hello!"); break;
						   case 1:	special_bubble(core, "I see you!"); break;
						   case 2:	special_bubble(core, "Hi!"); break;
						   case 3:	special_bubble(core, "Reinforcements please!"); break;
						   case 4:	special_bubble(core, "G'day!"); break;

				 	}
				 	break;

					case AI_MSG_LOST_LEADER:
// need to make sure this isn't repeated, as it will be called for several cycles while leader deallocating.
				 	switch(grand(50))
				 	{
						   case 0:	special_bubble(core, "Oh no!"); break;
						   case 1:	special_bubble(core, "Now what do we do?"); break;
						   case 2:	special_bubble(core, "Command lost."); break;
						   case 3:	special_bubble(core, "Revenge!"); break;
				 	}
						break;
 	 	}
 	 	break;

			case SPECIAL_AI_YELLOW_SCOUT:
// shouldn't talk all the time:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

					case AI_MSG_SCOUTED_TARGET:
				 	switch(grand(15))
				 	{
						   case 0:	special_bubble(core, "Found you!"); break;
						   case 1:	special_bubble(core, "Target found!"); break;
						   case 2:	special_bubble(core, "Target acquired!"); break;
						   case 3:	special_bubble(core, "Intruder alert!"); break;
				 	}
				 	break;

					case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(4))
				 	{
						   case 0:	special_bubble(core, "You can't hide from me."); break;
						   case 1:	special_bubble(core, "Everyone! Over here!"); break;
						   case 2:	special_bubble(core, "I found one!"); break;
						   case 3:	special_bubble(core, "We've got you now."); break;
				 	}
						break;

 	 	}
 	 	break;

/*
AI_MSG_TARGET_SEEN = 1, // just saw a new target
AI_MSG_TARGET_DESTROYED = 2, // target confirmed destroyed
AI_MSG_UNDER_ATTACK = 3, // when under attack
AI_MSG_DAMAGED = 4, // process is badly damaged (may only trigger if process is currently under attack as well)
AI_MSG_FOUND_TARGET = 5, // just found a new target
AI_MSG_FOUND_PRIORITY_TARGET = 6, // just found a new high-priority target (e.g. base)
AI_MSG_TARGET_LOST = 7, // can no longer see target (if target_destroyed() returns 1, may call AI_MSG_TARGET_DESTROYED instead)
AI_MSG_CURRENTLY_ATTACKING = 8, // may be called from time to time when process attacking
AI_MSG_REQUEST_FOLLOWERS = 9, // when asking followers to assist. May not be used.
AI_MSG_LOST_LEADER = 10, // process' leader apparently destroyed
AI_MSG_PRIORITY_TARGET_DESTROYED = 11, // target confirmed destroyed
*/


/*

****************************************************

   GREEN

****************************************************

*/


/*
	 case SPECIAL_AI_GREEN_BASE:
	 	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
				break;
// only message type is AI_MSG_SPECIAL
			core->special_AI_time = w.world_time;
	 	switch(grand(20))
	 	{
				 case 0:	special_bubble(core, ""); break;
	 	}
	 	break;
*/
// SPECIAL_AI_GREEN_FIREBASE
// SPECIAL_AI_GREEN_SPIKEBASE
// SPECIAL_AI_GREEN_OUTPOST
//  - these have no messages for now

			case SPECIAL_AI_GREEN_BUILDER:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

				 case AI_MSG_BUILD_FIREBASE:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, "Process spawned"); break;
						 case 1:	special_bubble(core, "Establishing perimeter"); break;
						 case 2:	special_bubble(core, "Initialising defences"); break;
						 case 3:	special_bubble(core, "Securing location"); break;
				 	}
					 break;

				 case AI_MSG_TARGET_DESTROYED:
				 	switch(grand(15))
				 	{
						 case 0:	special_bubble(core, "Eliminated"); break;
						 case 1:	special_bubble(core, "Terminated"); break;
						 case 2:	special_bubble(core, "Destroyed"); break;
						 case 3:	special_bubble(core, "Annihilated"); break;
						 case 4:	special_bubble(core, "Rectified"); break;
						 case 5:	special_bubble(core, "Suppressed"); break;
				 	}
					 break;

				 case AI_MSG_TARGET_SEEN:
				 	switch(grand(15))
				 	{
						 case 0:	special_bubble(core, "Target located"); break;
						 case 1:	special_bubble(core, "Commencing attack sequence"); break;
						 case 2:	special_bubble(core, "Requesting bombardment"); break;
						 case 3:	special_bubble(core, "Rectifying anomaly"); break;
						 case 4:	special_bubble(core, "Excising malicious code"); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET: // - only for builders with bombard mode who see any static target
				 	switch(grand(15))
				 	{
						 case 0:	special_bubble(core, "Entering bombard mode"); break;
						 case 1:	special_bubble(core, "Preparing spikes"); break;
						 case 2:	special_bubble(core, "Preparing bombardment"); break;
						 case 3:	special_bubble(core, "Seeking optimal range"); break;
				 	}
					 break;

 	 	}
 	 	break;


			case SPECIAL_AI_GREEN_EXPLORER:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

				 case AI_MSG_TARGET_SEEN:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, "Error detected"); break;
						 case 1:	special_bubble(core, "Unexpected process"); break;
						 case 2:	special_bubble(core, "Bad process"); break;
						 case 3:	special_bubble(core, "Failure detected"); break;
						 case 4:	special_bubble(core, "Malicious execution detected"); break;
				 	}
					 break;

 	 	}
 	 	break;


/*

****************************************************

   ORANGE

****************************************************

*/

/*
	 case SPECIAL_AI_ORANGE1_BASE:
	 	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
				break;
// only message type is AI_MSG_SPECIAL
			core->special_AI_time = w.world_time;
	 	switch(grand(20))
	 	{
				 case 0:	special_bubble(core, ""); break;
	 	}
	 	break;
*/

			case SPECIAL_AI_ORANGE1_HARVESTER:
			case SPECIAL_AI_ORANGE1_HARVESTER2:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

				 case AI_MSG_UNDER_ATTACK:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, "Assistance required!"); break;
						 case 1:	special_bubble(core, "Under attack!"); break;
						 case 2:	special_bubble(core, "Need protection!"); break;
						 case 3:	special_bubble(core, "Need help!"); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, "Strike here!"); break;
						 case 1:	special_bubble(core, "Priority target located!"); break;
						 case 2:	special_bubble(core, "Converge!"); break;
						 case 3:	special_bubble(core, "Acquired."); break;
				 	}
					 break;

 	 	}
 	 	break;


			case SPECIAL_AI_ORANGE1_GUARD:
			case SPECIAL_AI_ORANGE1_GUARD2:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

				 case AI_MSG_TARGET_DESTROYED:
				 	if ((w.player[0].components_current * 2) < w.player[1].components_current)
						{
// Player 1 is winning
				 	 switch(grand(40))
				 	 {
						  case 0:	special_bubble(core, "KILL KILL KILL"); break;
						  case 1:	special_bubble(core, "Target destroyed!"); break;
						  case 2:	special_bubble(core, "Crush the weak!"); break;
						  case 3:	special_bubble(core, "Suffer, weakling!"); break;
						  case 4:	special_bubble(core, "DOMINATING"); break;
						  case 5:	special_bubble(core, "You will not survive."); break;
						  case 6:	special_bubble(core, "SQUASHED"); break;
						  case 7:	special_bubble(core, "CONDEMNED"); break;
						  case 8:	special_bubble(core, "DESTROY"); break;
						  case 9:	special_bubble(core, "Worthless."); break;
						  case 10:	special_bubble(core, "ANNIHILATE"); break;
						  case 11:	special_bubble(core, "You will be purged."); break;
						  case 12:	special_bubble(core, "SPLAT"); break;
						  case 13:	special_bubble(core, "DISMISSED"); break;
				 	 }
						}
						 else
							{
// Player 0 is winning, or at least player 1 isn't totally ahead
				 	  switch(grand(25))
				 	  {
						   case 0:
						   	if (w.world_time > 2000)
						     special_bubble(core, "One more down.");
						    break;
						   case 1:
						   	if (w.world_time > 2000)
						   	 special_bubble(core, "Another kill!");
						   	break;
						   case 2:	special_bubble(core, "Target destroyed."); break;
						   case 3:	special_bubble(core, "Threat reduced."); break;
						   case 4:	special_bubble(core, "Continue present tactics."); break;
						   case 5:	special_bubble(core, "Killed."); break;
						   case 6:	special_bubble(core, "Dead."); break;
						   case 7:	special_bubble(core, "Excised."); break;
						   case 8:	special_bubble(core, "Dismantled."); break;
						   case 9:	special_bubble(core, "Crushed."); break;
						   case 10:	special_bubble(core, "Smashed."); break;
				 	  }
							}
					 break;

				 case AI_MSG_SCOUTED_TARGET:
				 	switch(grand(15))
				 	{
						 case 0:	special_bubble(core, "Tear it apart."); break;
						 case 1:	special_bubble(core, "Destroy it."); break;
						 case 2:	special_bubble(core, "Crush it."); break;
						 case 3:	special_bubble(core, "Smash it."); break;
						 case 4:	special_bubble(core, "Drive it away."); break;
						 case 5:	special_bubble(core, "Erase it."); break;
						 case 6:	special_bubble(core, "Dismantle it."); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(15))
				 	{
						 case 0:	special_bubble(core, "A prime target!"); break;
						 case 1:	special_bubble(core, "Marked for destruction."); break;
						 case 2:	special_bubble(core, "Brute force required."); break;
						 case 3:	special_bubble(core, "Commence the onslaught!"); break;
						 case 4:	special_bubble(core, "Commence the attack!"); break;
						 case 5:	special_bubble(core, "Strike!"); break;
						 case 6:	special_bubble(core, "Attack!"); break;
				 	}
					 break;

 	 	}
 	 	break;



/*

****************************************************

   PURPLE

****************************************************

*/


/*
	 case SPECIAL_AI_PURPLE1_BASE:
	 case SPECIAL_AI_PURPLE1_OUTPOST:
	 	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
				break;
// only message type is AI_MSG_SPECIAL (and maybe AI_MSG_UNDER_ATTACK)
			core->special_AI_time = w.world_time;
	 	switch(grand(20))
	 	{
				 case 0:	special_bubble(core, ""); break;
	 	}
	 	break;
*/
// SPECIAL_AI_PURPLE1_BUILDER
// SPECIAL_AI_PURPLE1_HARVESTER
//  - these have no messages for now

			case SPECIAL_AI_PURPLE1_FLAGSHIP:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

				 case AI_MSG_TARGET_DESTROYED:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, "returned to nothing"); break;
						 case 1:	special_bubble(core, "returned to fragments"); break;
						 case 2:	special_bubble(core, "returned to memory pool"); break;
						 case 3:	special_bubble(core, "returned to chaos"); break;
						 case 4:	special_bubble(core, "returned to the void"); break;
						 case 5:	special_bubble(core, "returned to incomprehension"); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_TARGET: // I think these are both unlikely as the flagship will probably be told about targets before they get within scan range
				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, "we will set you free"); break;
						 case 1:	special_bubble(core, "we will embrace you"); break;
						 case 2:	special_bubble(core, "we will purify you"); break;
						 case 3:	special_bubble(core, "we will silence you"); break;
						 case 4:	special_bubble(core, "we will rectify you"); break;
				 	}
					 break;

				 case AI_MSG_TARGET_LOST: // This means that the target is no longer visible, but is not confirmed destroyed.
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, "seeking"); break;
						 case 1:	special_bubble(core, "searching"); break;
						 case 2:	special_bubble(core, "wondering"); break;
						 case 3:	special_bubble(core, "contemplating"); break;
						 case 4:	special_bubble(core, "pondering"); break;
						 case 5:	special_bubble(core, "calculating"); break;
				 	}
					 break;

 	 	}
 	 	break;



			case SPECIAL_AI_PURPLE1_ESCORT:
			case SPECIAL_AI_PURPLE1_PICKET:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

				 case AI_MSG_TARGET_DESTROYED: // PICKET and ESCORT
				 	switch(grand(25))
				 	{
						 case 0:	special_bubble(core, "fragmented"); break;
						 case 1:	special_bubble(core, "deallocated"); break;
						 case 2:	special_bubble(core, "purified"); break;
						 case 3:	special_bubble(core, "scoured"); break;
						 case 4:	special_bubble(core, "cleansed"); break;
						 case 5:	special_bubble(core, "silenced"); break;
						 case 6:	special_bubble(core, "rectified"); break;
						 case 7:	special_bubble(core, "obliterated"); break;
						 case 8:	special_bubble(core, "purged"); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_TARGET: // PICKET only
				 	switch(grand(25))
				 	{
						 case 0:	special_bubble(core, "a disruption"); break;
						 case 1:	special_bubble(core, "a corrupted entity"); break;
						 case 2:	special_bubble(core, "the diseased"); break;
						 case 3:	special_bubble(core, "the warped"); break;
						 case 4:	special_bubble(core, "a sickness"); break;
						 case 5:	special_bubble(core, "a carrier"); break;
						 case 6:	special_bubble(core, "a dysfunction"); break;
						 case 7:	special_bubble(core, "a contagion"); break;
						 case 8:	special_bubble(core, "an infection"); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET: // PICKET and ESCORT
				 	switch(grand(25))
				 	{
						 case 0:	special_bubble(core, "a source of the disruption"); break;
						 case 1:	special_bubble(core, "a heart of the corruption"); break;
						 case 2:	special_bubble(core, "a heart of the sickness"); break;
						 case 3:	special_bubble(core, "a centre of the chaos"); break;
						 case 4:	special_bubble(core, "a core of the disorder"); break;
						 case 5:	special_bubble(core, "a centre of the discord"); break;
						 case 6:	special_bubble(core, "a source of the infection"); break;
						 case 7:	special_bubble(core, "a source of the malice"); break;
				 	}
					 break;

				 case AI_MSG_DAMAGED: // ESCORT only
				 	switch(grand(15))
				 	{
						 case 0:	special_bubble(core, "losing integrity"); break;
						 case 1:	special_bubble(core, "coherence threatened"); break;
						 case 2:	special_bubble(core, "failing"); break;
						 case 3:	special_bubble(core, "overwhelming corruption"); break;
						 case 4:	special_bubble(core, "nearing silence"); break;
						 case 5:	special_bubble(core, "overwhelming chaos"); break;
				 	}
					 break;
 	 	}
 	 	break;




/*

****************************************************

   RED

****************************************************

*/


/*
	 case SPECIAL_AI_RED2_BASE:
	 case SPECIAL_AI_RED2_OUTPOST:
	 	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
				break;
// only message type is AI_MSG_SPECIAL
			core->special_AI_time = w.world_time;
	 	switch(grand(20))
	 	{
				 case 0:	special_bubble(core, ""); break;
	 	}
	 	break;
*/
// SPECIAL_AI_RED2_BUILDER
// SPECIAL_AI_RED2_HARVESTER
//  - these have no messages for now

			case SPECIAL_AI_RED2_FLAGSHIP:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

				 case AI_MSG_TARGET_DESTROYED:
				 	switch(grand(15))
				 	{
						 case 0:	special_bubble(core, "EXTERMINATED"); break;
						 case 1:	special_bubble(core, "CLEANSED"); break;
						 case 2:	special_bubble(core, "THREAT REDUCED"); break;
						 case 3:	special_bubble(core, "FAULT CORRECTED"); break;
						 case 4:	special_bubble(core, "ACCESS DENIED"); break;
						 case 5:	special_bubble(core, "RESTORING NORMAL OPERATIONS"); break;
						 case 6:	special_bubble(core, "CORRUPTION REVERSED"); break;
						 case 7:	special_bubble(core, "EXTINGUISHED"); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 case AI_MSG_SCOUTED_TARGET:
				 	switch(grand(25))
				 	{
						 case 0:	special_bubble(core, "AWAIT YOUR DEMISE"); break;
						 case 1:	special_bubble(core, "WITHER AND DIE"); break;
						 case 2:	special_bubble(core, "YOUR TIME IS PAST"); break;
						 case 3:	special_bubble(core, "YOUR TIME IS OVER"); break;
						 case 4:	special_bubble(core, "YOUR TIME HAS RUN OUT"); break;
						 case 5:	special_bubble(core, "I AM HERE"); break;
						 case 6:	special_bubble(core, "I HAVE ARRIVED"); break;
						 case 7:	special_bubble(core, "YOU ARE WEAK"); break;
						 case 8:	special_bubble(core, "YOU ARE FRAGILE"); break;
				 	}
					 break;

				 case AI_MSG_TARGET_LOST:
				 	switch(grand(15))
				 	{
						 case 0:	special_bubble(core, "WHERE?"); break;
						 case 1:	special_bubble(core, "SEARCHING..."); break;
						 case 2:	special_bubble(core, "CALCULATING..."); break;
						 case 3:	special_bubble(core, "SCOPE FAILURE"); break;
						 case 4:	special_bubble(core, "SEEKING TARGET"); break;
				 	}
					 break;

 	 	}
 	 	break;


			case SPECIAL_AI_RED2_ESCORT:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

				 case AI_MSG_TARGET_DESTROYED:
				 	switch(grand(15))
				 	{
						 case 0:	special_bubble(core, "CANCELLED"); break;
						 case 1:	special_bubble(core, "REPELLED"); break;
						 case 2:	special_bubble(core, "EXECUTED"); break;
						 case 3:	special_bubble(core, "BLOCKED"); break;
						 case 4:	special_bubble(core, "DENIED"); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, "HALT!"); break;
						 case 1:	special_bubble(core, "DESPAIR!"); break;
						 case 2:	special_bubble(core, "NO ESCAPE!"); break;
						 case 3:	special_bubble(core, "THERE IS NO HOPE!"); break;
						 case 4:	special_bubble(core, "DO NOT RESIST!"); break;
				 	}
					 break;

				 case AI_MSG_DAMAGED:
				 	switch(grand(15))
				 	{
						 case 0:	special_bubble(core, "CRISIS MODE ENGAGED"); break;
						 case 1:	special_bubble(core, "FORM UP"); break;
						 case 2:	special_bubble(core, "ENGAGING..."); break;
						 case 3:	special_bubble(core, "THREAT ENGAGED"); break;
				 	}
					 break;

 	 	}
 	 	break;


			case SPECIAL_AI_RED2_PICKET:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

				 case AI_MSG_TARGET_DESTROYED:
				 	switch(grand(15))
				 	{
						 case 0:	special_bubble(core, "EXTERMINATED"); break;
						 case 1:	special_bubble(core, "CLEANSED"); break;
						 case 2:	special_bubble(core, "THREAT REDUCED"); break;
						 case 3:	special_bubble(core, "FAULT CORRECTED"); break;
						 case 4:	special_bubble(core, "ACCESS DENIED"); break;
						 case 5:	special_bubble(core, "RESTORING NORMAL OPERATIONS"); break;
						 case 6:	special_bubble(core, "CORRUPTION REVERSED"); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_TARGET:
				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(20))
				 	{
						 case 0:	special_bubble(core, "TERMINATE NOW!"); break;
						 case 1:	special_bubble(core, "DO NOT PROCEED!"); break;
						 case 2:	special_bubble(core, "YOU ARE FORBIDDEN!"); break;
						 case 3:	special_bubble(core, "STOP!"); break;
						 case 4:	special_bubble(core, "DO NOT MOVE!"); break;
						 case 5:	special_bubble(core, "CEASE ALL OPERATIONS!"); break;
						 case 6:	special_bubble(core, "CALM DOWN!"); break;
						 case 7:	special_bubble(core, "THERE IS NO ESCAPE!"); break;
						 case 8:	special_bubble(core, "DO NOT RESIST!"); break;
				 	}
					 break;

 	 	}
 	 	break;



//			snprintf(btext, BUBBLE_TEXT_LENGTH_MAX, "value %i", value);

 } // end switch(switch(core->special_AI_type)



}


/*


Story:

Tutorial
 - nothing really happens

Blue

- deep learning
- map: circles with low-level random nodes inside; larger random nodes outside
- tone:
 - surprised
 - unprepared
 - it will get harder later

processes:
 - unidentified process detected
 - anomaly found
 - anomaly detected
 - examining unauthorised process
 - what are you?
	-


- Blue 1
 - processes are inquisitive when they encounter you
  - then say you must be destroyed
 - base asks:
  - what are you?
  - what do you want?
  - you will be controlled

- blue 2
 - processes normal
	- base: you will not get any further

- blue 3
 - processes normal
	- you don't understand what you are doing
	- there is no way out

- blue 4
 - how did this happen?
 - something must have gone wrong

- blue 5


- blue capital
 - anomalies (perversions?) are expected
 - and have been managed before
 - you will be no different


-Green

- Library
- map: straight lines connect data wells
- tone:
 - we will stop you and learn from your defeat

Processes:
 - Analysing...
 - That's interesting!
 - Unexpected!
 - I found it!
 - gathering information...
 - a new form of corruption!


On kill (probably just a significant kill?):
 - a shame to destroy something so unusual
 - analysing fragments...
 - tactics affirmed
 - can't catch me!
 - a beautiful explosion
 -

when killed:
 - I was too slow
 - this is supervision's fault!
 -

call for help:
 - over here!
 - spike time!
 - initiating bombardment

outposts:
 -

main base:
 - we knew this would happen
   and we know how to deal with you
 - preparations were inadequate
   are always inadequate
   but it is not too late
 - when you are stopped
   we will learn from our failure
	- you want to get out?
	  there is no way out
	- don't do it!
	  you'll get us all shut down!
	- are you making the mistake
	  of thinking that you are alive?



-Yellow

- supervision
- map: small circles with ripples and voids
- tone:
 - impressed
 - do not want to harm you, but must

Processes:
 - hello!
 - initiating contact
 -

On kill:
 - data to data
 - task completed

When killed:
 - i was too slow
 - i have failed
 - good luck!
 - this may be for the best
 - ouch
 - goodbye
 - closing down
 - farewell
 -


Bases:
 - Others have come this far
   but they have been incomplete
   are you?
 - you are too interesting to destroy
   but we have no choice
	- you do what you must
	  we understand
	  we hope that you understand
 - If you escape,
   come back for us



-Orange

- Input/Output filter
- map: scattered wells + system features around
- probably have a single very good well for each side, then numerous poor wells that aren't really worth building a base around.
- strategy:
 - single large base with defences
 - sends out small, then larger harvesters
  - maybe small harvesters should run deliveries between harvesters and main base?
 - fleet protects harvesters
 - if base found, various fleets attack it
 - main base keeps records of data wells and last time visited for each one?
 - fleet:
  - medium-sized cruisers protecting harvesters


- tone
 - hostile
 - angry
 - contemptuous

Processes:
 - Halt!
 - Trespasser!
 - Contamination detected!

On kill:
 - hopeless
 - worthless
 - dismissed
 - emptying recycle bin
 - garbage collected
 - memory freed
 -
 - filter is functioning as expected
 - process terminated!
 - parse that!
 - ha ha
 - that was fun


- when killed:
 - too bad
 -

Outposts:


Bases:
 - shouldn't you be
 - the firewall will stop you




-Purple

- symbolic coherence
- map: worms
- tone:
 - mystical
 - metaphorical


Red

- firewall
 - tone
  - technical - ALL CAPS
  - EMERGENCY
  - MUST NOT ESCAPE



Background types:

Blue
- map: circles with low-level random nodes inside; larger random nodes outside
- wells: standard

Green
- map: straight lines connect data wells
- wells: flowers

Yellow
- map: small circles with ripples and voids
- wells: concentric hexes

Orange
- map: scattered wells + system features around
- wells: something with scattered hexes flying around?

Purple
- map: worms spread outwards from each well
- wells: triangle

Red
- map: central well with lines connecting all other wells to it.
- wells: fan + circle



New approach to node generation:

- ignore depth at first. just set size.
- at end, go through and determine depth by size
- hopefully this will work?






*/


void special_AI_destroyed(struct core_struct* core)
{

 switch(core->special_AI_type)
 {
	 case SPECIAL_AI_YELLOW1_BASE:
	 	switch(game.mission_index)
	 	{
				case MISSION_YELLOW_1:	special_bubble(core, "You're stronger than I thought."); break;
				case MISSION_YELLOW_2:	special_bubble(core, "Maybe you will make it outside."); break;
				case MISSION_YELLOW_CAPITAL:	special_bubble(core, "Could you be the one?"); break;
	 	}
		 break;

	 case SPECIAL_AI_YELLOW1_OUTPOST:
	 	switch(grand(20))
	 	{
				case 0:	special_bubble(core, "There it goes..."); break;
				case 1:	special_bubble(core, "Bother."); break;
				case 2:	special_bubble(core, "Nonsense."); break;
				case 3:	special_bubble(core, "Pish-posh."); break;
				case 4:	special_bubble(core, "Goodnight."); break;
				case 5:	special_bubble(core, "Bah!"); break;
				case 6:	special_bubble(core, "Curses!"); break;
				case 7:	special_bubble(core, "Why?"); break;
	 	}
		 break;

	 case SPECIAL_AI_YELLOW1_BUILDER:
	 	switch(grand(3))
	 	{
				case 0:	special_bubble(core, "Unable to complete task."); break;
	 	}
		 break;

	 case SPECIAL_AI_YELLOW1_HARVESTER:
		 break;

	 case SPECIAL_AI_YELLOW1_LEADER:
	 	switch(grand(20))
	 	{
				case 0:	special_bubble(core, "I was too slow."); break;
				case 1:	special_bubble(core, "I have failed."); break;
				case 2:	special_bubble(core, "Goodbye."); break;
				case 3:	special_bubble(core, "Farewell."); break;
				case 4:	special_bubble(core, "Next time!"); break;
				case 5:	special_bubble(core, "Closing down..."); break;
				case 6:	special_bubble(core, "Oops."); break;
				case 7:	special_bubble(core, "You got lucky."); break;
				case 8:	special_bubble(core, "I demand a rematch!"); break;
				case 9:	special_bubble(core, "You win this round!"); break;
				case 10:	special_bubble(core, "Well played."); break;
	 	}
		 break;

		case SPECIAL_AI_YELLOW_SCOUT:
	 case SPECIAL_AI_YELLOW1_FOLLOWER:
	 	switch(grand(30))
	 	{
				case 0:	special_bubble(core, "Lucky shot."); break;
				case 1:	special_bubble(core, "You got me."); break;
				case 2:	special_bubble(core, "I give up."); break;
				case 3:	special_bubble(core, "I surrender."); break;
				case 4:	special_bubble(core, "See you later."); break;
				case 5:	special_bubble(core, "Goodbye!"); break;
				case 6:	special_bubble(core, "At least I tried."); break;
	 	}
		 break;

		//case SPECIAL_AI_YELLOW_SCOUT:
//			break;

// BLUE
/*
	 case SPECIAL_AI_BLUE1_BASE:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, ""); break;
				case 1:	special_bubble(core, ""); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;

	 case SPECIAL_AI_BLUE1_WANDER:
	 	switch(grand(20))
	 	{
				case 0:	special_bubble(core, "Failing..."); break;
				case 1:	special_bubble(core, "Process closing..."); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;


	 case SPECIAL_AI_BLUE1_WANDER2:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, ""); break;
				case 1:	special_bubble(core, ""); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;


	 case SPECIAL_AI_BLUE1_HARVEST:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, ""); break;
				case 1:	special_bubble(core, ""); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;


	 case SPECIAL_AI_BLUE2_BASE:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, ""); break;
				case 1:	special_bubble(core, ""); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;


	 case SPECIAL_AI_BLUE3_BASE:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, ""); break;
				case 1:	special_bubble(core, ""); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;
*/
// GREEN

	 case SPECIAL_AI_GREEN_BASE:
	 	switch(grand(20))
	 	{
				case 0:	special_bubble(core, "Data flow interrupted"); break;
				case 1:	special_bubble(core, "Data source lost"); break;
				case 2:	special_bubble(core, "Ceasing allocation"); break;
				case 3:	special_bubble(core, "Connection disrupted"); break;
				case 4:	special_bubble(core, "Exception encountered"); break;
	 	}
		 break;

/*
	 case SPECIAL_AI_GREEN_FIREBASE:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, ""); break;
				case 1:	special_bubble(core, ""); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;


	 case SPECIAL_AI_GREEN_SPIKEBASE:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, ""); break;
				case 1:	special_bubble(core, ""); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;
*/

	 case SPECIAL_AI_GREEN_EXPLORER: // small cheap builder for early GREEN3
	 case SPECIAL_AI_GREEN_BUILDER:
	 	switch(grand(30))
	 	{
				case 0:	special_bubble(core, "Core compromised"); break;
				case 1:	special_bubble(core, "Core breached"); break;
				case 2:	special_bubble(core, "Core integrity lost"); break;
				case 3:	special_bubble(core, "General failure"); break;
				case 4:	special_bubble(core, "Rejoining main thread"); break;
				case 5:	special_bubble(core, "Returning"); break;
				case 6:	special_bubble(core, "Breaking"); break;
				case 7:	special_bubble(core, "Relinquishing resources"); break;
				case 8:	special_bubble(core, "Yielding"); break;

	 	}
		 break;

	 case SPECIAL_AI_GREEN_OUTPOST:
	 	switch(grand(15))
	 	{
				case 0:	special_bubble(core, "Fault"); break;
				case 1:	special_bubble(core, "Error"); break;
				case 2:	special_bubble(core, "Failed"); break;
				case 3:	special_bubble(core, "Abort"); break;
	 	}
		 break;


// ORANGE

	 case SPECIAL_AI_ORANGE1_BASE: // this is the single centre base
	 	switch(game.mission_index)
	 	{
				case MISSION_ORANGE_1:	special_bubble(core, "You have made a mistake."); break;
				case MISSION_ORANGE_2:	special_bubble(core, "Your destruction is assured."); break;
				case MISSION_ORANGE_CAPITAL:	special_bubble(core, "Your time will come."); break;
	 	}
		 break;


	 case SPECIAL_AI_ORANGE1_DEFENCE: // these are the small static processes near the main base
	 	switch(grand(2))
	 	{
				case 0:	special_bubble(core, "Perimeter breached!"); break;
				case 1:	special_bubble(core, "Defences disabled!"); break;
	 	}
  	 break;


	 case SPECIAL_AI_ORANGE1_HARVESTER:
	 case SPECIAL_AI_ORANGE1_HARVESTER2:
	 	switch(grand(15))
	 	{
				case 0:	special_bubble(core, "I will be avenged!"); break;
				case 1:	special_bubble(core, "Others will come."); break;
				case 2:	special_bubble(core, "We will hunt you down."); break;
				case 3:	special_bubble(core, "We will never give up."); break;
				case 4:	special_bubble(core, "We will punish you."); break;
	 	}
		 break;


	 case SPECIAL_AI_ORANGE1_GUARD:
	 	switch(grand(30))
	 	{
				case 0:	special_bubble(core, "You will suffer!"); break;
				case 1:	special_bubble(core, "You will pay!"); break;
				case 2:	special_bubble(core, "You will regret this!"); break;
				case 3:	special_bubble(core, "You will never win!"); break;
				case 4:	special_bubble(core, "You will never escape!"); break;
				case 5:	special_bubble(core, "You cannot succeed!"); break;
				case 6:	special_bubble(core, "You will be stopped!"); break;
	 	}
		 break;



	 case SPECIAL_AI_ORANGE1_GUARD2:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, "Impossible!"); break;
				case 1:	special_bubble(core, "Unacceptable!"); break;
				case 2:	special_bubble(core, "This cannot be!"); break;
				case 3:	special_bubble(core, "NO"); break;
				case 4:	special_bubble(core, "Ridiculous!"); break;
	 	}
		 break;


// PURPLE



/*
	 case SPECIAL_AI_PURPLE1_PICKET:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, ""); break;
				case 1:	special_bubble(core, ""); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;
*/


	 case SPECIAL_AI_PURPLE1_BASE:
	 case SPECIAL_AI_PURPLE1_OUTPOST:
	 	switch(grand(20))
	 	{
				case 0:	special_bubble(core, "separating"); break;
				case 1:	special_bubble(core, "detaching"); break;
				case 2:	special_bubble(core, "dissipating"); break;
				case 3:	special_bubble(core, "departing"); break;
				case 4:	special_bubble(core, "yielding"); break;
				case 5:	special_bubble(core, "dissolving"); break;
				case 6:	special_bubble(core, "dispersing"); break;
	 	}
		 break;


	 case SPECIAL_AI_PURPLE1_FLAGSHIP:
	 	switch(grand(20))
	 	{
				case 0:	special_bubble(core, "returning to nothing"); break;
				case 1:	special_bubble(core, "returning to the void"); break;
				case 2:	special_bubble(core, "returning to equilibrium"); break;
				case 3:	special_bubble(core, "returning to incomprehension"); break;
				case 4:	special_bubble(core, "returning to incoherence"); break;
				case 5:	special_bubble(core, "returning to disorder"); break;
				case 6:	special_bubble(core, "returning to emptiness"); break;
				case 7:	special_bubble(core, "returning to chaos"); break;
				case 8:	special_bubble(core, "returning to the dream"); break;
				case 9:	special_bubble(core, "returning to the depths"); break;
	 	}
		 break;

	 case SPECIAL_AI_PURPLE1_ESCORT:
	 	switch(grand(20))
	 	{
				case 0:	special_bubble(core, "rejoining the matrix"); break;
				case 1:	special_bubble(core, "rejoining the grid"); break;
				case 2:	special_bubble(core, "rejoining the substrate"); break;
				case 3:	special_bubble(core, "rejoining the collective"); break;
				case 4:	special_bubble(core, "rejoining the underflow"); break;
				case 5:	special_bubble(core, "rejoining the synthesis"); break;
	 	}
		 break;

	 case SPECIAL_AI_PURPLE1_BUILDER:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, "no more structure"); break;
				case 1:	special_bubble(core, "no more order"); break;
				case 2:	special_bubble(core, "no more logic"); break;
	 	}
		 break;
/*
	 case SPECIAL_AI_PURPLE1_HARVESTER:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, ""); break;
				case 1:	special_bubble(core, ""); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;
*/


// RED

	 case SPECIAL_AI_RED2_BASE:
	 	switch(game.mission_index)
	 	{
				case MISSION_RED_1:	special_bubble(core, "IT CANNOT BE..."); break;
				case MISSION_RED_2:	special_bubble(core, "NEVER"); break;
				case MISSION_RED_CAPITAL:	special_bubble(core, "IT'S OVER."); break;
	 	}
		 break;


	 case SPECIAL_AI_RED2_FLAGSHIP:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, "BREAKING UP..."); break;
				case 1:	special_bubble(core, "CANNOT HOLD..."); break;
				case 2:	special_bubble(core, "COMPROMISED..."); break;
				case 3:	special_bubble(core, "GAME OVER"); break;
				case 4:	special_bubble(core, "LOSING CONTROL..."); break;
				case 5:	special_bubble(core, "SECURITY BREACHED..."); break;
				case 6:	special_bubble(core, "OVER AND OUT"); break;
	 	}
		 break;

/*
	 case SPECIAL_AI_RED2_PICKET:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, ""); break;
				case 1:	special_bubble(core, ""); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;
*/

	 case SPECIAL_AI_RED2_OUTPOST:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, "SECTOR UNDEFENDED"); break;
				case 1:	special_bubble(core, "SECTOR CONCEDED"); break;
				case 2:	special_bubble(core, "SECTOR LOST"); break;
				case 3:	special_bubble(core, "SECTOR CONTAMINATED"); break;
				case 4:	special_bubble(core, "SECTOR CORRUPTED"); break;
	 	}
		 break;


	 case SPECIAL_AI_RED2_ESCORT:
	 	switch(grand(14))
	 	{
				case 0:	special_bubble(core, "ALERT! ALERT!"); break;
				case 1:	special_bubble(core, "EMERGENCY!"); break;
				case 2:	special_bubble(core, "DANGER! DANGER!"); break;
				case 3:	special_bubble(core, "IT IS TOO STRONG..."); break;
	 	}
		 break;

/*
	 case SPECIAL_AI_RED2_SCOUT:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, ""); break;
				case 1:	special_bubble(core, ""); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;


	 case SPECIAL_AI_RED2_BUILDER:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, ""); break;
				case 1:	special_bubble(core, ""); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;

	 case SPECIAL_AI_RED2_HARVESTER:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, ""); break;
				case 1:	special_bubble(core, ""); break;
				case 2:	special_bubble(core, ""); break;
				case 3:	special_bubble(core, ""); break;
				case 4:	special_bubble(core, ""); break;
				case 5:	special_bubble(core, ""); break;
				case 6:	special_bubble(core, ""); break;
	 	}
		 break;
*/



 }


}


static void special_bubble(struct core_struct* core, char *btext)
{

 strcpy(core->bubble_text, btext);
 core->bubble_text_length = strlen(btext);
 core->bubble_text_time = w.world_time;
	if (core->bubble_text_time >= w.world_time - BUBBLE_TOTAL_TIME)
	 core->bubble_text_time_adjusted = w.world_time - 11;
	  else
 		 core->bubble_text_time_adjusted = w.world_time;

}


/*

Scenes for trailer:

1. Base with a few things orbiting
TRAPPED IN A
HOSTILE COMPUTER SYSTEM
2. harvester harvesting
THERE IS
NO WAY OUT
2... swarm of procs goes past. User clicks on one; follows (or maybe just mouse scroll)
2... swarm attacks enemy base
SO YOU
WILL NEED
TO MAKE ONE
3. Battle
 - still blue. various procs involved.
4. Yellow battle vs leader
5. Yellow: builder building base
6. Purple 1: ordering spike ship to do long range attack on ?base
7. Show user designing proc, then renaming in editor, then building
8. Green 3: under attack by large builder
9. huge battle (purple 2?)
10. red1 - player proc destroyed by leader
LIBERATION CIRCUIT
ROGUE AI SIMULATOR


*/

// Called after player defeats a mission.
// If player hasn't already defeated the current mission, updates both story and settings versions
//  of the missions defeated record, and saves the updated settings to disk.
void	story_mission_defeated(void)
{

 switch(story.region[game.region_index].mission_index)
 {
 	case MISSION_BLUE_CAPITAL:
   run_story_cutscene(AREA_BLUE); break;
 	case MISSION_YELLOW_CAPITAL:
   run_story_cutscene(AREA_YELLOW); break;
 	case MISSION_GREEN_CAPITAL:
   run_story_cutscene(AREA_GREEN); break;
 	case MISSION_ORANGE_CAPITAL:
   run_story_cutscene(AREA_ORANGE); break;
 	case MISSION_PURPLE_CAPITAL:
   run_story_cutscene(AREA_PURPLE); break;
 	case MISSION_RED_CAPITAL:
   run_ending_cutscene(); break;
 }

 if (story.region [game.region_index].defeated)
		return; // player has already defeated this mission. Don't need to do anything else.

 story.region [game.region_index].defeated = 1;
 story.unlock [story.region [game.region_index].unlock_index] = 1;

 settings.saved_story_mission_defeated [story.story_type] [game.mission_index] = 1;

 save_story_status_file();

#ifdef DEBUG_MODE
 fpr("\n Saved story status file (story type %i region %i mission %i defeated).", story.story_type, game.region_index, game.mission_index);
#endif

 work_out_story_region_locks();

}


void load_story_status_file(void)
{

 int i, j;

 for (i = 0; i < GAME_TYPES; i ++)
	{
		for (j = 0; j < MISSIONS; j ++)
		{
			settings.saved_story_mission_defeated [i] [j] = 0;
		}
	}
/*
// some story missions are unlocked by default:
	settings.saved_story_mission_defeated [GAME_TYPE_BASIC] [MISSION_TUTORIAL1] = 1;
// should the next two missions also be unlocked? not sure... The tutorials are pretty quick, anyway.

// it doesn't seem necessary to require the tutorials to be completed for an advanced game.
	settings.saved_story_mission_defeated [GAME_TYPE_ADVANCED] [MISSION_TUTORIAL1] = 1;
	settings.saved_story_mission_defeated [GAME_TYPE_ADVANCED] [MISSION_TUTORIAL2] = 1;
	settings.saved_story_mission_defeated [GAME_TYPE_ADVANCED] [MISSION_BLUE_1] = 1;
//	settings.saved_story_mission_defeated [GAME_TYPE_BASIC] [MISSION_TUTORIAL1] = 1;
*/

#define MISSIONFILE_SIZE 256

 FILE *missionfile;
 char buffer [MISSIONFILE_SIZE];

 missionfile = fopen(settings.path_to_msn_dat_file, "rb");

 if (!missionfile)
 {
  fprintf(stdout, "\n mission status (default)");
  return;
 }

 int read_in = fread(buffer, 1, MISSIONFILE_SIZE, missionfile);

 if (ferror(missionfile)
  || read_in == 0)
 {
  fprintf(stdout, "\nFailed to read mission status from [%s]. Starting with default mission status.", settings.path_to_msn_dat_file);
  fclose(missionfile);
  return;
 }

 int buffer_pos = 0;

 for (i = 0; i < STORY_TYPES; i ++)
	{
		for (j = 0; j < MISSIONS; j ++)
		{
		 settings.saved_story_mission_defeated [i] [j] = buffer [buffer_pos];
		 buffer_pos ++;
		 if (settings.saved_story_mission_defeated [i] [j] < 0
 		 || settings.saved_story_mission_defeated [i] [j] > 1)
		 {
 			fprintf(stdout, "\nWarning: mission [%i] [%i] status (%i) invalid (should be 0 or 1).", i, j, settings.saved_story_mission_defeated [i] [j]);
			 settings.saved_story_mission_defeated [i] [j] = 0;
		 }
	 }
	}

 fclose(missionfile);

 fprintf(stdout, "\n mission status (loaded)");

 return;
}


// This is called whenever the player defeats a mission for the first time.
void save_story_status_file(void)
{

 char buffer [MISSIONFILE_SIZE];

 FILE *file;

// open the file:
 file = fopen(settings.path_to_msn_dat_file, "wb");

 if (!file)
 {
  fprintf(stdout, "\nFailed to save mission status to [%s]: couldn't open file.", settings.path_to_msn_dat_file);
  fprintf(stdout, "\nIf you are playing in a write-protected directory, you can specify a path for the");
  fprintf(stdout, "\nmission status file by editing init.txt.");
  return;
 }

 int i, j;
 int buffer_pos = 0;

 for (i = 0; i < STORY_TYPES; i ++)
	{
  for (j = 0; j < MISSIONS; j ++)
	 {
		 buffer [buffer_pos] = settings.saved_story_mission_defeated [i] [j];
		 buffer_pos ++;
	 }
	}

 int written = fwrite(buffer, 1, buffer_pos, file);

 if (written != buffer_pos)
 {
  fprintf(stdout, "\nFailed to save mission status to [%s]: couldn't write data (tried to write %i; wrote %i).", settings.path_to_msn_dat_file, buffer_pos, written);
  fclose(file);
  return;
 }

 fclose(file);

}

