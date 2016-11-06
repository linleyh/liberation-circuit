
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



static void work_out_story_region_locks(void);

extern ALLEGRO_DISPLAY* display;


void run_story_mode(void);
static int add_story_region(int old_region, int connect_index, int area_index, int mission_index);
static void remove_story_region(int region_index);

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
		story.region[i].defeated = 0;
		story.region[i].capital = 0;
//		story.region[i].unlocked = 0; // is done by work_out_story_region_locks

		for (j = 0; j < SRC_DIRECTIONS; j ++)
		{
		 story.region[i].connect [j] = -1;
		}

	}
/*
// place regions here
 story.region[0].exists = 1;
 story.region[0].grid_x = 0;
 story.region[0].grid_y = 0;
 story.region[1].exists = 1;
 story.region[1].grid_x = story.region[0].grid_x + region_connect[SRC_UR].x_offset;
 story.region[1].grid_y = story.region[0].grid_y + region_connect[SRC_UR].y_offset;
 story.region[2].exists = 1;
 story.region[2].grid_x = story.region[1].grid_x + region_connect[SRC_UL].x_offset;
 story.region[2].grid_y = story.region[1].grid_y + region_connect[SRC_UL].y_offset;
 story.region[3].exists = 1;
 story.region[3].grid_x = story.region[2].grid_x + region_connect[SRC_UL].x_offset;
 story.region[3].grid_y = story.region[2].grid_y + region_connect[SRC_UL].y_offset;
 story.region[4].exists = 1;
 story.region[4].grid_x = story.region[3].grid_x + region_connect[SRC_UL].x_offset;
 story.region[4].grid_y = story.region[3].grid_y + region_connect[SRC_UL].y_offset;


 story.region[5].exists = 1;
 story.region[5].grid_x = story.region[1].grid_x + region_connect[SRC_UR].x_offset;
 story.region[5].grid_y = story.region[1].grid_y + region_connect[SRC_UR].y_offset;
 story.region[6].exists = 1;
 story.region[6].grid_x = story.region[5].grid_x + region_connect[SRC_UR].x_offset;
 story.region[6].grid_y = story.region[5].grid_y + region_connect[SRC_UR].y_offset;
*/


 int new_region;

 new_region = add_story_region(-1, 0, AREA_TUTORIAL, MISSION_TUTORIAL1);
 new_region = add_story_region(new_region, SRC_R, AREA_TUTORIAL, MISSION_TUTORIAL2);
// blue
 int first_blue_region = add_story_region(new_region, SRC_UR, AREA_BLUE, MISSION_BLUE_1);
 new_region = add_story_region(first_blue_region, SRC_UR, AREA_BLUE, MISSION_BLUE_2);
 new_region = add_story_region(first_blue_region, SRC_UL, AREA_BLUE, MISSION_BLUE_3);
 int blue_capital_region = add_story_region(new_region, SRC_UR, AREA_BLUE, MISSION_BLUE_CAPITAL);
 story.region[blue_capital_region].capital = 1;
 new_region = add_story_region(blue_capital_region, SRC_R, AREA_BLUE, MISSION_BLUE_4);
 new_region = add_story_region(blue_capital_region, SRC_L, AREA_BLUE, MISSION_BLUE_5);
 int blue_to_yellow_region = add_story_region(blue_capital_region, SRC_UR, AREA_BLUE, MISSION_BLUE_6);
 new_region = add_story_region(blue_capital_region, SRC_UL, AREA_BLUE, MISSION_BLUE_7);
// green
 int first_green_region = add_story_region(new_region, SRC_L, AREA_GREEN, MISSION_GREEN_1);
 new_region = add_story_region(first_green_region, SRC_L, AREA_GREEN, MISSION_GREEN_1); // removed
 new_region = add_story_region(new_region, SRC_UR, AREA_GREEN, MISSION_GREEN_3);
 int green_capital_region = add_story_region(new_region, SRC_L, AREA_GREEN, MISSION_GREEN_CAPITAL);
 story.region[green_capital_region].capital = 1;
 new_region = add_story_region(green_capital_region, SRC_UR, AREA_GREEN, MISSION_GREEN_4);
 new_region = add_story_region(first_green_region, SRC_UR, AREA_GREEN, MISSION_GREEN_2);
// yellow
 int first_yellow_region = add_story_region(blue_to_yellow_region, SRC_UL, AREA_YELLOW, MISSION_YELLOW_1);
 new_region = add_story_region(first_yellow_region, SRC_R, AREA_YELLOW, MISSION_YELLOW_2);
 new_region = add_story_region(new_region, SRC_DR, AREA_YELLOW, MISSION_YELLOW_3);
 new_region = add_story_region(new_region, SRC_UR, AREA_YELLOW, MISSION_YELLOW_4);
 int yellow_capital_region = add_story_region(new_region, SRC_UL, AREA_YELLOW, MISSION_YELLOW_CAPITAL);
 story.region[yellow_capital_region].capital = 1;
 new_region = add_story_region(yellow_capital_region, SRC_L, AREA_YELLOW, MISSION_YELLOW_5);
 new_region = add_story_region(yellow_capital_region, SRC_UR, AREA_YELLOW, MISSION_YELLOW_6);
// purple
 int first_purple_region = add_story_region(first_yellow_region, SRC_UL, AREA_PURPLE, MISSION_PURPLE_1);
 new_region = add_story_region(first_purple_region, SRC_UR, AREA_PURPLE, MISSION_PURPLE_2);
 new_region = add_story_region(new_region, SRC_R, AREA_PURPLE, MISSION_PURPLE_3);
 new_region = add_story_region(first_purple_region, SRC_L, AREA_PURPLE, MISSION_PURPLE_4);
 new_region = add_story_region(first_purple_region, SRC_UL, AREA_PURPLE, MISSION_PURPLE_5);
 int purple_capital_region = add_story_region(new_region, SRC_UR, AREA_PURPLE, MISSION_PURPLE_CAPITAL);
 story.region[purple_capital_region].capital = 1;
 new_region = add_story_region(purple_capital_region, SRC_R, AREA_PURPLE, MISSION_PURPLE_6);
// orange
 int first_orange_region = add_story_region(green_capital_region, SRC_UL, AREA_ORANGE, MISSION_ORANGE_1);
 new_region = add_story_region(first_orange_region, SRC_UR, AREA_ORANGE, MISSION_ORANGE_2);
 new_region = add_story_region(new_region, SRC_R, AREA_ORANGE, MISSION_ORANGE_3);
 new_region = add_story_region(new_region, SRC_UL, AREA_ORANGE, MISSION_ORANGE_4);
// new_region = add_story_region(new_region, SRC_L, AREA_ORANGE, 10);
 new_region = add_story_region(first_orange_region, SRC_UL, AREA_ORANGE, MISSION_ORANGE_5);
 int orange_capital_region = add_story_region(new_region, SRC_UR, AREA_ORANGE, MISSION_ORANGE_CAPITAL);
 story.region[orange_capital_region].capital = 1;
// red
 int first_red_region = add_story_region(purple_capital_region, SRC_L, AREA_RED, MISSION_RED_1);
 new_region = add_story_region(first_red_region, SRC_UR, AREA_RED, MISSION_RED_2);
// new_region = add_story_region(new_region, SRC_R, AREA_RED, 10);
 new_region = add_story_region(new_region, SRC_R, AREA_RED, MISSION_RED_3);
 new_region = add_story_region(new_region, SRC_R, AREA_RED, MISSION_RED_4);
 new_region = add_story_region(new_region, SRC_UL, AREA_RED, MISSION_RED_5);
 new_region = add_story_region(new_region, SRC_L, AREA_RED, MISSION_RED_6);
 new_region = add_story_region(new_region, SRC_L, AREA_RED, MISSION_RED_7);
 new_region = add_story_region(new_region, SRC_UR, AREA_RED, MISSION_RED_8);
 new_region = add_story_region(new_region, SRC_R, AREA_RED, MISSION_RED_9);
 int exit_region = add_story_region(new_region, SRC_UL, AREA_RED, MISSION_RED_CAPITAL);
 story.region[exit_region].capital = 1;
// dark blue
 int first_dark_blue_region = add_story_region(yellow_capital_region, SRC_R, AREA_DARK_BLUE, MISSION_DARK_BLUE_1);
 new_region = add_story_region(first_dark_blue_region, SRC_UR, AREA_DARK_BLUE, MISSION_DARK_BLUE_2);
 new_region = add_story_region(new_region, SRC_UL, AREA_DARK_BLUE, MISSION_DARK_BLUE_3);
 new_region = add_story_region(new_region, SRC_L, AREA_DARK_BLUE, MISSION_DARK_BLUE_4);
 new_region = add_story_region(first_dark_blue_region, SRC_R, AREA_DARK_BLUE, MISSION_DARK_BLUE_5);
 int dark_blue_capital_region = add_story_region(new_region, SRC_UR, AREA_DARK_BLUE, MISSION_DARK_BLUE_CAPITAL);
 story.region[dark_blue_capital_region].capital = 1;

 remove_story_region(8);
 remove_story_region(9);
 remove_story_region(11);
 remove_story_region(23);
 remove_story_region(26);
 remove_story_region(19);
 remove_story_region(22);
 remove_story_region(25);
 remove_story_region(33);
 remove_story_region(34);
 remove_story_region(50);


// now work out connections:
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

	work_out_story_region_locks();

}

static void remove_story_region(int region_index)
{

 story.region[region_index].exists = 0;

}

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
	story.region[14].defeated = 1;
	story.region[30].defeated = 1;

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

	run_story_mode();

}

// returns index of new story in story.region array
static int add_story_region(int old_region, int connect_index, int area_index, int mission_index)
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

// fpr("\n old %i new %i connect %i at %i,%i", old_region, i, connect_index, story.region[i].grid_x, story.region[i].grid_y);

	return i;

}


extern ALLEGRO_EVENT_QUEUE* event_queue;


void run_story_mode(void)
{

 ALLEGRO_EVENT ev;

 while(TRUE)
	{

  al_wait_for_event(event_queue, &ev);

  story_input();
  draw_story_interface();

	}

 flush_game_event_queues();
 al_hide_mouse_cursor(display);


}



