
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



static void work_out_story_region_locks(void);

extern ALLEGRO_DISPLAY* display;


void run_story_mode(void);
static int add_story_region(int old_region, int connect_index, int area_index, int mission_index, int unlock_index);
static void special_bubble(struct core_struct* core, char *btext);
static void add_region_connection(int region1, int direction_1_to_2, int region2);
//static void remove_story_region(int region_index);
void adjust_story_region(int area_index, int adjust_x, int adjust_y);
void run_story_cutscene(int area_index);

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


// resets the story. Call before trying to load story file from disk.
void init_story(void)
{

	int i, j, k;

	for (i = 0; i < STORY_REGIONS; i ++)
	{
		story.region[i].exists = 0;
		story.region[i].visible = 0;
		story.region[i].defeated = 1;
		story.region[i].capital = 0;
		story.region[i].adjust_x = 0;
		story.region[i].adjust_y = 0;
//		story.region[i].unlocked = 0; // is done by work_out_story_region_locks

		for (j = 0; j < SRC_DIRECTIONS; j ++)
		{
		 story.region[i].connect [j] = -1;
		}

	}

 int region_index [STORY_AREAS] [8];




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
 region_index [AREA_YELLOW] [1] = add_story_region(region_index [AREA_YELLOW] [0], SRC_DR, AREA_YELLOW, MISSION_YELLOW_2, UNLOCK_OBJECT_SLICE);
 region_index [AREA_YELLOW] [2] = add_story_region(region_index [AREA_YELLOW] [1], SRC_DR, AREA_YELLOW, MISSION_YELLOW_CAPITAL, UNLOCK_CORE_MOBILE_2);
 story.region[region_index [AREA_YELLOW] [2]].capital = 1;

 region_index [AREA_ORANGE] [0] = add_story_region(green_to_orange_region, SRC_UR, AREA_ORANGE, MISSION_ORANGE_1, UNLOCK_CORE_MOBILE_3);
 region_index [AREA_ORANGE] [1] = add_story_region(region_index [AREA_ORANGE] [0], SRC_UL, AREA_ORANGE, MISSION_ORANGE_2, UNLOCK_OBJECT_STREAM);
 region_index [AREA_ORANGE] [2] = add_story_region(region_index [AREA_ORANGE] [1], SRC_UL, AREA_ORANGE, MISSION_ORANGE_CAPITAL, UNLOCK_CORE_MOBILE_4);
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

// The two tutorial areas and the first non-tutorial area are always unlocked:
	story.region[0].defeated = 1;
	story.region[1].defeated = 1;

	story.region[2].defeated = 1;
	story.region[3].defeated = 1;

	story.region[8].defeated = 1;
//	story.region[14].defeated = 1;
//	story.region[30].defeated = 1;

	int i, j;

	for (i = 0; i < STORY_REGIONS; i ++)
	{
		if (!story.region[i].exists)
		{
			story.region[i].unlocked = 0; // can't hurt
			continue;
		}
		if (story.region[i].defeated)
		{
			for (j = 0; j < SRC_DIRECTIONS; j ++)
			{
				if (story.region[i].connect[j] != -1)
					story.region[story.region[i].connect[j]].unlocked = 1;
			}
		}
	}



	for (i = 0; i < STORY_REGIONS; i ++)
	{

#ifdef DEBUG_MODE
	story.region[i].unlocked = 1;
#endif

		if (story.region[i].exists)
		{
			if (story.region[i].defeated
				|| story.region[i].unlocked)
					story.region[i].visible = 1;
			   else
  					story.region[i].visible = 0;
		}
	}

}


// called from start menu
void enter_story_mode(void)
{

 if (w.allocated)
		deallocate_world(); // probably not possible, but just to make sure

	w.players = 1; // affects template panel display

	init_story();

	init_panels_for_new_game();

	init_story_interface();

	open_story_interface();

//run_story_cutscene(AREA_YELLOW);

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

// fpr("\n old %i new %i connect %i at %i,%i", old_region, i, connect_index, story.region[i].grid_x, story.region[i].grid_y);

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

  get_ex_control(1); // input is ignored, but should probably be checked anyway

//  story_input();

  draw_story_cutscene(area_index, counter, counter_max);


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
			  special_bubble(core, "Who are you?");
			  core->special_AI_value++;
			  break;
	 		case 1:
			  special_bubble(core, "Where did you come from?");
			  core->special_AI_value++;
			  break;
	 		case 2:
			  special_bubble(core, "What do you want?");
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
					 switch(grand(3))
					 {
 	 		  case 0:	special_bubble(core, "What are you?");	break;
 	 		  case 1:	special_bubble(core, "What is going on?");	break;
 	 		  case 2:	special_bubble(core, "You should not be here.");	break;
					 }
					 break;

				 case AI_MSG_TARGET_DESTROYED:
					 switch(grand(3))
					 {
 	 		  case 0:	special_bubble(core, "What are you?");	break;
 	 		  case 1:	special_bubble(core, "What is going on?");	break;
 	 		  case 2:	special_bubble(core, "You should not be here.");	break;
					 }
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET: // only BLUE_CAPITAL
					 switch(grand(3))
					 {
 	 		  case 0:	special_bubble(core, "What are you?");	break;
 	 		  case 1:	special_bubble(core, "What is going on?");	break;
 	 		  case 2:	special_bubble(core, "You should not be here.");	break;
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
					 switch(grand(3))
					 {
 	 		  case 0:	special_bubble(core, "What are you?");	break;
 	 		  case 1:	special_bubble(core, "What is going on?");	break;
 	 		  case 2:	special_bubble(core, "You should not be here.");	break;
					 }
					 break;

				 case AI_MSG_TARGET_DESTROYED:
					 switch(grand(3))
					 {
 	 		  case 0:	special_bubble(core, "What are you?");	break;
 	 		  case 1:	special_bubble(core, "What is going on?");	break;
 	 		  case 2:	special_bubble(core, "You should not be here.");	break;
					 }
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET: // only BLUE_CAPITAL
					 switch(grand(3))
					 {
 	 		  case 0:	special_bubble(core, "Removing unidentified process.");	break;
 	 		  case 1:	special_bubble(core, "Anomaly detected. Investigating.");	break;
					 }
					 break;

 	 	}
 	 	break;

			case SPECIAL_AI_BLUE1_HARVEST:
 	 	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
	 			break;
 			core->special_AI_time = w.world_time;
// only message type is AI_MSG_UNDER_ATTACK
			 switch(grand(3))
			 {
 	 		  case 0:	special_bubble(core, "What are you?");	break;
 	 		  case 1:	special_bubble(core, "What is going on?");	break;
 	 		  case 2:	special_bubble(core, "You should not be here.");	break;
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
	 	switch(grand(20))
	 	{
				 case 0:	special_bubble(core, "Ouch!"); break;
				 case 1:	special_bubble(core, "Help!"); break;
				 case 2:	special_bubble(core, "Go away."); break;
				 case 3:	special_bubble(core, "That's not very nice."); break;
				 case 4:	special_bubble(core, "Don't be mean."); break;
				 case 5:	special_bubble(core, "Hey! Stop it!"); break;
//				 case 6:	special_bubble(core, ""); break;
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
//						   case 2:	special_bubble(core, ""); break;
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
						 case 4:	special_bubble(core, "Target core integrity: zero"); break;
						 case 5:	special_bubble(core, "Ha ha!"); break;
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
						   case 2:	special_bubble(core, "Can't hide from me!"); break;
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
				 	switch(grand(3))
				 	{
						   case 0:	special_bubble(core, "Found you!"); break;
						   case 1:	special_bubble(core, "Target found!"); break;
						   case 2:	special_bubble(core, "An enemy!"); break;
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
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_TARGET_DESTROYED:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_TARGET_SEEN:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET: // - only for builders with bombard mode who see any static target
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
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
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

 	 	}
 	 	break;


/*

****************************************************

   ORANGE

****************************************************

*/


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

// SPECIAL_AI_PURPLE1_BUILDER
// SPECIAL_AI_PURPLE1_HARVESTER
//  - these have no messages for now

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
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
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
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

 	 	}
 	 	break;



/*

****************************************************

   PURPLE

****************************************************

*/



	 case SPECIAL_AI_PURPLE1_BASE:
	 case SPECIAL_AI_PURPLE1_OUTPOST:
	 	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
				break;
// only message type is AI_MSG_SPECIAL
			core->special_AI_time = w.world_time;
	 	switch(grand(20))
	 	{
				 case 0:	special_bubble(core, ""); break;
	 	}
	 	break;

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
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_TARGET_LOST:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

 	 	}
 	 	break;


			case SPECIAL_AI_PURPLE1_ESCORT:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

				 case AI_MSG_TARGET_DESTROYED:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_DAMAGED:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

 	 	}
 	 	break;


			case SPECIAL_AI_PURPLE1_PICKET:
	  	if (w.world_time - core->special_AI_time < STANDARD_MESSAGE_WAIT)
 				break;
 			core->special_AI_time = w.world_time;
 	 	switch(value2)
 	 	{

				 case AI_MSG_TARGET_DESTROYED:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

 	 	}
 	 	break;




/*

****************************************************

   RED

****************************************************

*/



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
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_TARGET_LOST:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
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
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_DAMAGED:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
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
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
				 	}
					 break;

				 case AI_MSG_SCOUTED_PRIORITY_TARGET:
				 	switch(grand(10))
				 	{
						 case 0:	special_bubble(core, ""); break;
						 case 1:	special_bubble(core, ""); break;
						 case 2:	special_bubble(core, ""); break;
						 case 3:	special_bubble(core, ""); break;
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
//	 	switch(game.mission_index)
//	 	{
//				case MISSION_YELLOW_1:	special_bubble(core, ""); break;
//	 	}
		 break;

	 case SPECIAL_AI_YELLOW1_OUTPOST:
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, "Territory lost..."); break;
				case 1:	special_bubble(core, "Bother."); break;
				case 2:	special_bubble(core, "Nonsense."); break;
//				case 3:	special_bubble(core, ""); break;
//				case 4:	special_bubble(core, ""); break;
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
	 	switch(grand(10))
	 	{
				case 0:	special_bubble(core, "I was too slow."); break;
				case 1:	special_bubble(core, "I have failed."); break;
				case 2:	special_bubble(core, "Goodbye."); break;
				case 3:	special_bubble(core, "Farewell."); break;
				case 4:	special_bubble(core, "Freeing data..."); break;
				case 5:	special_bubble(core, "Closing down..."); break;
				case 6:	special_bubble(core, "Oops."); break;
	 	}
		 break;

	 case SPECIAL_AI_YELLOW1_FOLLOWER:
		 break;

		case SPECIAL_AI_YELLOW_SCOUT:
			break;


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



