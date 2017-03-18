#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>

#include "m_config.h"
#include "m_globvars.h"

#include "g_header.h"
#include "g_misc.h"
#include "i_disp_in.h"
#include "i_display.h"
#include "i_header.h"
#include "i_background.h"

#include "p_panels.h"



extern ALLEGRO_DISPLAY *display;
extern struct fontstruct font [FONTS];

const int base_col_value [BASIC_COLS] [6] =
{
// first 3 are minimum for base colours. 2nd 3 are maximum.
 {1,1,1, 250,250,250}, // COL_GREY
 {50,5,15, 200,40,80, }, // COL_RED
 {5,50,15, 40,200,80}, // COL_GREEN
 {15,5,50, 150,140,210}, // COL_BLUE
 {40,30,5, 180,170,70}, // COL_YELLOW
 {45,20,5, 190,130,60}, // COL_ORANGE
 {40,5,40, 150,60,180}, // COL_PURPLE

 {5,10,45, 70,120,220}, // COL_TURQUOISE
 {1,20,40, 50,160,180}, // COL_AQUA
 {5,30,50, 30,180,200}, // COL_CYAN
 {15,15,35, 120,120,200}, // COL_DULL
};

const int base_fade_col_value [BASIC_COLS] [6] =
{
// first 3 are minimum for base_fade colours. 2nd 3 are maximum.
 {1,1,1, 250,250,250}, // COL_GREY
 {230,10,5, 250,80,50}, // COL_RED (dark red to yellow)
 {5,230,15, 130,250,60}, // COL_GREEN (dark green to yellow)
 {15,5,230, 130,150,240}, // COL_BLUE (dark blue to white)
 {200,40,5, 230,190,70}, // COL_YELLOW (orange to yellowish-white)
 {210,1,1, 220,150,60}, // COL_ORANGE (deep red to orange)
 {180,5,180, 220,60,220}, // COL_PURPLE (red to purple)

 {5,130,220, 70,150,240}, // COL_TURQUOISE
 {1,130,150, 50,160,180}, // COL_AQUA
 {5,180,200, 30,190,210}, // COL_CYAN
 {120,120,150, 150,150,200}, // COL_DULL
};





const int print_col_value [PRINT_COLS] [3] =
{

 {100,100,100}, // PRINT_COL_DGREY,
 {170,170,170}, // PRINT_COL_LGREY,
 {210,210,210}, // PRINT_COL_WHITE,
 {100,120,180}, // PRINT_COL_LBLUE,
 {70,80,180}, // PRINT_COL_DBLUE,
 {200,130,100}, // PRINT_COL_LRED,
 {150,100,70}, // PRINT_COL_DRED,
 {100,180,110}, // PRINT_COL_LGREEN,
 {70,160,80}, // PRINT_COL_DGREEN,
 {150,100,160}, // PRINT_COL_LPURPLE,
 {120,70,130}, // PRINT_COL_DPURPLE,

};


enum
{
BASE_TEAM_COL_MIN = PROC_COL_LEVELS,
BASE_TEAM_COL_MAX,
BASE_TEAM_COL_MAP_PIXEL,

BASE_PROC_COL_ARRAY_SIZE

};


int base_proc_col [TEAM_COLS] [BASE_PROC_COL_ARRAY_SIZE] [3] =
{
	{
		{38, 58, 128}, // 0 underlay
		{74, 142, 220}, // 1 core (mutable - these are base values)
		{50, 115, 190}, // 2 main 1
		{74, 142, 230}, // 3 main 2
		{64, 132, 230}, // 4 main 3
		{38, 98, 148}, // 5 link
		{65, 110, 170}, // 6 object base
		{64, 140, 220}, // 7 object 1
		{52, 120, 220}, // 8 object 2
// The above go into the proc_col_ list
  {30, 50, 100}, // 9 BASE_TEAM_COL_MIN
  {80, 140, 250}, // 10 BASE_TEAM_COL_MAX
  {80, 110, 254}, // 11 BASE_TEAM_COL_MAP_PIXEL
//  {80, 80, 250}, // 11 BASE_TEAM_COL_MAP_PIXEL

	}, // end TEAM_COL_BLUE

	{
		{130, 80, 40}, // 0 underlay
		{230, 142, 74}, // 1 core
		{210, 115, 54}, // 2 main 1
		{220, 132, 64}, // 3 main 2
		{220, 122, 54}, // 4 main 3
		{180, 108, 44}, // 5 link
		{190, 120, 52}, // 6 object base
		{230, 150, 72}, // 7 object 1
		{230, 120, 52}, // 8 object 2
// The above go into the proc_col_ list
  {60, 30, 10}, // 9 BASE_TEAM_COL_MIN
  {210, 150, 60}, // 10 BASE_TEAM_COL_MAX
  {250, 150, 50}, // BASE_TEAM_COL_MAP_PIXEL

	}, // end TEAM_COL_YELLOW
	{
/*
		{20, 80, 50}, // 0 underlay
		{44, 140, 112}, // 1 core
		{34, 130, 102}, // 2 main 1
		{24, 120, 92}, // 3 main 2
		{24, 110, 82}, // 4 main 3
		{20, 100, 38}, // 5 link
		{42, 140, 110}, // 6 object base
		{62, 160, 140}, // 7 object 1
		{62, 160, 150}, // 8 object 2
// The above go into the proc_col_ list
  {20, 80, 60}, // 9 BASE_TEAM_COL_MIN
  {70, 240, 140}, // 10 BASE_TEAM_COL_MAX
  {50, 250, 80}, // BASE_TEAM_COL_MAP_PIXEL
*/
		{20, 80, 50}, // 0 underlay
		{44, 140, 112}, // 1 core
		{14, 140, 12}, // 2 main 1
		{14, 130, 22}, // 3 main 2
		{14, 140, 22}, // 4 main 3
		{20, 100, 38}, // 5 link
		{42, 140, 80}, // 6 object base
		{62, 160, 90}, // 7 object 1
		{62, 160, 80}, // 8 object 2
// The above go into the proc_col_ list
  {20, 80, 60}, // 9 BASE_TEAM_COL_MIN
  {70, 240, 140}, // 10 BASE_TEAM_COL_MAX
  {50, 250, 80}, // BASE_TEAM_COL_MAP_PIXEL

	}, // end TEAM_COL_GREEN
	{
		{100, 50, 50}, // 0 underlay
		{180, 180, 180}, // 1 core
		{160, 160, 160}, // 2 main 1
		{160, 120, 120}, // 3 main 2
		{160, 100, 100}, // 4 main 3
		{160, 100, 100}, // 5 link
		{160, 110, 110}, // 6 object base
		{150, 130, 130}, // 7 object 1
		{180, 140, 140}, // 8 object 2
// The above go into the proc_col_ list
  {60, 20, 20}, // 9 BASE_TEAM_COL_MIN
  {190, 190, 190}, // 10 BASE_TEAM_COL_MAX
  {250, 200, 200}, // BASE_TEAM_COL_MAP_PIXEL


	}, // end TEAM_COL_WHITE
	{
		{58, 28, 78}, // 0 underlay
		{144, 72, 220}, // 1 core (mutable - these are base values)
		{100, 15, 150}, // 2 main 1
		{110, 30, 190}, // 3 main 2
		{110, 24, 190}, // 4 main 3
		{108, 38, 148}, // 5 link
		{120, 40, 170}, // 6 object base
		{170, 40, 220}, // 7 object 1
		{120, 30, 220}, // 8 object 2
// The above go into the proc_col_ list
  {90, 20, 100}, // 9 BASE_TEAM_COL_MIN
  {160, 40, 250}, // 10 BASE_TEAM_COL_MAX
  {150, 30, 250}, // 11 BASE_TEAM_COL_MAP_PIXEL

/*
		{78, 58, 128}, // 0 underlay
		{144, 72, 220}, // 1 core (mutable - these are base values)
		{120, 15, 170}, // 2 main 1
		{130, 30, 210}, // 3 main 2
		{112, 24, 210}, // 4 main 3
		{108, 38, 148}, // 5 link
		{120, 40, 170}, // 6 object base
		{170, 40, 220}, // 7 object 1
		{120, 30, 220}, // 8 object 2
// The above go into the proc_col_ list
  {90, 20, 100}, // 9 BASE_TEAM_COL_MIN
  {160, 40, 250}, // 10 BASE_TEAM_COL_MAX
  {150, 30, 250}, // 11 BASE_TEAM_COL_MAP_PIXEL
*/
	}, // end TEAM_COL_PURPLE
	{
		{130, 60, 40}, // 0 underlay
		{230, 122, 74}, // 1 core
		{220, 85, 54}, // 2 main 1
		{230, 102, 64}, // 3 main 2
		{230, 92, 54}, // 4 main 3
		{180, 88, 44}, // 5 link
		{190, 100, 52}, // 6 object base
		{230, 130, 72}, // 7 object 1
		{230, 100, 52}, // 8 object 2
// The above go into the proc_col_ list
  {60, 10, 20}, // 9 BASE_TEAM_COL_MIN
  {210, 130, 60}, // 10 BASE_TEAM_COL_MAX
  {250, 130, 50}, // BASE_TEAM_COL_MAP_PIXEL

	}, // end TEAM_COL_ORANGE

	{
//		{110, 10, 5}, // 0 underlay
		{110, 5, 5}, // 0 underlay
		{230, 42, 24}, // 1 core
		{150, 30, 28}, // 2 main 1
		{160, 36, 32}, // 3 main 2
		{165, 36, 30}, // 4 main 3
		{125, 38, 14}, // 5 link
		{140, 20, 6}, // 6 object base
		{190, 25, 10}, // 7 object 1
		{190, 20, 18}, // 8 object 2
// The above go into the proc_col_ list
  {70, 10, 20}, // 9 BASE_TEAM_COL_MIN
  {220, 110, 40}, // 10 BASE_TEAM_COL_MAX
  {250, 100, 60}, // BASE_TEAM_COL_MAP_PIXEL

/*
//		{110, 10, 5}, // 0 underlay
		{130, 70, 5}, // 0 underlay
		{230, 42, 24}, // 1 core
		{140, 15, 8}, // 2 main 1
		{150, 4, 12}, // 3 main 2
		{150, 8, 10}, // 4 main 3
		{120, 38, 14}, // 5 link
		{180, 20, 6}, // 6 object base
		{190, 25, 10}, // 7 object 1
		{190, 20, 18}, // 8 object 2
// The above go into the proc_col_ list
  {70, 10, 20}, // 9 BASE_TEAM_COL_MIN
  {220, 110, 40}, // 10 BASE_TEAM_COL_MAX
  {250, 100, 60}, // BASE_TEAM_COL_MAP_PIXEL
*/
	}, // end TEAM_COL_RED



};

const int base_packet_colours [PACKET_COLS] [6] =
{
/* {230, 250, // min and max red values
  10, 180, // green
  5, 60}, // blue - PACKET_COL_YELLOW_ORANGE
*/
 {230, 250, // min and max red values
  50, 180, // green
  25, 90}, // blue - PACKET_COL_YELLOW_ORANGE
 {10, 230, // min and max red values
  10, 240, // green
  230, 250}, // blue - PACKET_COL_WHITE_BLUE
 {230, 230, // min and max red values
  140, 230, // green
  5, 230}, // blue - PACKET_COL_WHITE_YELLOW
 {180, 230, // min and max red values
  10, 230, // green
  140, 230}, // blue - PACKET_COL_WHITE_PURPLE

 {230, 250, // min and max red values
  1, 100, // green
  1, 40}, // blue - PACKET_COL_ORANGE_RED

 {140, 220, // min and max red values
  10, 180, // green
  200, 250}, // blue - PACKET_COL_BLUE_PURPLE
 {10, 230, // min and max red values
  10, 110, // green
  230, 150}, // blue - PACKET_COL_ULTRAVIOLET


};

/*

How should interface colours work?

3 situations:

- fully charged - faint, whitish
- charging - should move from deeper colour towards fully charged colour
- hit - flash

So maybe need 3 values:
 - base - min charge
 - charge - increases with charge
 - hit - increases after being hit


*/


const int base_interface_colours [PACKET_COLS] [9] =
{
// values should leave room for hit modifier (currently up to 48)
// remember that these are base and variable values, not min and max!
//  base, variable(charge), variable(hit)
 {170, 20, 60, // base and variable red values
  30, 100, 80, // green
  20, 50, 10}, // blue - PACKET_COL_YELLOW_ORANGE
 {30, 100, 50, // base and var red values
  30, 100, 50, // green
  130, 10, 100}, // blue - PACKET_COL_WHITE_BLUE
 {160, 10, 70, // base and variable red values
  130, 20, 90, // green
  20, 50, 10}, // blue - PACKET_COL_WHITE_YELLOW
 {110, 50, 50, // min and max red values
  10, 20, 200, // green
  100, 60, 50}, // blue - PACKET_COL_WHITE_PURPLE

 {230, 20, // min and max red values
  10, 100, // green
  5, 40}, // blue - PACKET_COL_ORANGE_RED

 {30, 100, 50, // base and var red values
  30, 100, 50, // green
  130, 10, 100}, // blue - PACKET_COL_BLUE_PURPLE
 {30, 100, 50, // base and var red values
  30, 100, 50, // green
  130, 10, 100}, // blue - PACKET_COL_ULTRAVIOLET


/*
// remember that these are base and variable values, not min and max!
 {230, 20, // base and variable red values
  10, 180, // green
  5, 20}, // blue - PACKET_COL_YELLOW_ORANGE
 {70, 140, // base and var red values
  70, 140, // green
  230, 20}, // blue - PACKET_COL_WHITE_BLUE
 {230, 20, // base and variable red values
  10, 220, // green
  5, 220}, // blue - PACKET_COL_WHITE_YELLOW
 {180, 50, // min and max red values
  10, 200, // green
  150, 50}, // blue - PACKET_COL_WHITE_PURPLE

 {230, 20, // min and max red values
  10, 100, // green
  5, 40}, // blue - PACKET_COL_ORANGE_RED
*/
};


const int back_and_hex_colours [BACK_COLS] [9] =
{
// first 3 are background, next 3 are hex base colours, next three are hex variable colours. These can be set differently
	{15, 15, 40,
	 30, 30, 70,
	 0,  2, 10}, // BACK_COLS_BLUE
	{10, 40, 12,
	 5, 60, 10,
	 0,  7, 6}, // BACK_COLS_GREEN
	{55, 35, 0,
	 75, 37, 10,
	 12,  4, -3}, // BACK_COLS_YELLOW
//	 23,  15, -5}, // BACK_COLS_YELLOW
	{72, 15, 12,
	 80, 16, 14,
	 14,  5, -3}, // BACK_COLS_ORANGE
	{40, 10, 45,
	 50, 10, 55,
	 10,  5, 20}, // BACK_COLS_PURPLE
	{9, 9, 40,
	 20, 20, 70,
	 5,  10, 20}, // BACK_COLS_BLUE_DARK
	{50, 5, 5,
	 50, 5, 5,
	 15,  0, 0}, // BACK_COLS_RED


};



/*
// These colours have PROC_COL_LEVELS but not damage levels
// They also have specific alpha values
const int plan_colours [PLAN_COLS] [7] =
{

 {20, 70,
  40, 120,
  80, 230,
  255}, // PLAN_COL_DESIGN_BASIC - blue
 {60, 210,
  30, 70,
  10, 50,
  255}, // PLAN_COL_DESIGN_ERROR - red
 {20, 70,
  70, 180,
  80, 210,
  255}, // PLAN_COL_DESIGN_LOCKED - blue/green

 {20, 70,
  40, 120,
  80, 230,
  150}, // PLAN_COL_BUILD_OKAY - blue
 {60, 210,
  30, 70,
  10, 50,
  120}, // PLAN_COL_BUILD_ERROR - red


};
*/


int base_plan_col [PLAN_COLS] [PROC_COL_LEVELS] [3] =
{
	{
		{30, 70, 140}, // 0 underlay
		{64, 132, 230}, // 1 core (mutable - these are base values)
		{40, 105, 200}, // 2 main 1
		{64, 132, 240}, // 3 main 2
		{54, 122, 240}, // 4 main 3
		{34, 98, 190}, // 5 link
		{55, 100, 180}, // 6 object base
		{54, 130, 230}, // 7 object 1
		{42, 110, 230}, // 8 object 2
	}, // end PLAN_COL_DESIGN_BASIC

	{
		{130, 40, 40}, // 0 underlay
		{230, 64, 64}, // 1 core
		{230, 64, 64}, // 2 main 1
		{230, 64, 64}, // 3 main 2
		{230, 64, 64}, // 4 main 3
		{180, 48, 44}, // 5 link
		{190, 64, 52}, // 6 object base
		{230, 74, 72}, // 7 object 1
		{230, 64, 52}, // 8 object 2
	}, // end PLAN_COL_DESIGN_ERROR

	{
		{30, 110, 140}, // 0 underlay
		{64, 172, 230}, // 1 core (mutable - these are base values)
		{40, 145, 200}, // 2 main 1
		{64, 182, 240}, // 3 main 2
		{54, 172, 240}, // 4 main 3
		{44, 138, 190}, // 5 link
		{55, 140, 180}, // 6 object base
		{54, 170, 230}, // 7 object 1
		{42, 150, 230}, // 8 object 2
	}, // end PLAN_COL_DESIGN_LOCKED

	{
		{80, 30, 140}, // 0 underlay
		{134, 62, 230}, // 1 core (mutable - these are base values)
		{105, 55, 200}, // 2 main 1
		{134, 62, 240}, // 3 main 2
		{124, 82, 240}, // 4 main 3
		{104, 68, 190}, // 5 link
		{115, 70, 180}, // 6 object base
		{104, 100, 230}, // 7 object 1
		{92, 80, 230}, // 8 object 2
	}, // end PLAN_COL_DESIGN_MODIFIED

	{
		{30, 70, 140}, // 0 underlay
		{64, 132, 230}, // 1 core (mutable - these are base values)
		{40, 105, 200}, // 2 main 1
		{64, 132, 240}, // 3 main 2
		{54, 122, 240}, // 4 main 3
		{34, 98, 190}, // 5 link
		{55, 100, 180}, // 6 object base
		{54, 130, 230}, // 7 object 1
		{42, 110, 230}, // 8 object 2
	}, // end PLAN_COL_BUILD_OKAY

	{
		{130, 40, 40}, // 0 underlay
		{230, 64, 64}, // 1 core
		{230, 64, 64}, // 2 main 1
		{230, 64, 64}, // 3 main 2
		{230, 64, 64}, // 4 main 3
		{180, 48, 44}, // 5 link
		{190, 64, 52}, // 6 object base
		{230, 74, 72}, // 7 object 1
		{230, 64, 52}, // 8 object 2
	}, // end PLAN_COL_BUILD_ERROR

	{
		{30, 90, 120}, // 0 underlay
		{64, 152, 210}, // 1 core (mutable - these are base values)
		{40, 125, 180}, // 2 main 1
		{64, 152, 220}, // 3 main 2
		{54, 142, 220}, // 4 main 3
		{34, 118, 170}, // 5 link
		{55, 80, 160}, // 6 object base
		{54, 110, 210}, // 7 object 1
		{42, 90, 210}, // 8 object 2
	}, // end PLAN_COL_BUILD_QUEUE - these have a slightly lower alpha

	{
		{40, 100, 120}, // 0 underlay
		{74, 162, 210}, // 1 core (mutable - these are base values)
		{50, 135, 180}, // 2 main 1
		{74, 162, 220}, // 3 main 2
		{64, 152, 220}, // 4 main 3
		{44, 128, 170}, // 5 link
		{65, 90, 160}, // 6 object base
		{64, 120, 210}, // 7 object 1
		{52, 100, 210}, // 8 object 2
	}, // end PLAN_COL_BUILD_QUEUE_HIGHLIGHT


};

//void init_cloud_graphics(void);
//void map_cloud_cols(int col, int rmin, int rmax, int gmin, int gmax, int bmin, int bmax);
//void fade_circles(int centre_x, int centre_y, int col, int size, int core_size);
//void fade_circles2(int centre_x, int centre_y, int col, int size);
//void draw_to_clouds_bmp(ALLEGRO_BITMAP* source_bmp, int cloud_index, int* draw_x, int* draw_y, int source_x, int source_y, int width, int height, int* row_height);

void set_default_modifiable_colours(void);
//void init_mouse_cursors(void);

static void map_player_colours(int p, int base_col, int packet_col, int background_col);
static void map_cloud_cols(int p, int packet_or_drive, int min_col [3], int max_col [3]);
static void map_plan_cols(void);

static void map_player_base_colours(int p, int colour_index);
static void map_player_packet_colours(int p);
static void map_player_drive_colours(int p);
static void map_background_colour(void);
static void map_hex_colours(int p, int base_hex_colour);

//static void colour_fraction(int base_cols [3], int out_cols [3], int fraction, int subtract);
//static int check_col(int col_in);

//void init_packet_bmp(void);


struct coloursstruct colours;

extern ALLEGRO_BITMAP* display_window; // in i_display.c
extern ALLEGRO_BITMAP* vision_mask;
extern ALLEGRO_BITMAP* vision_mask_map [MAP_MASKS];

// Much of the display initialisation is done in main.c
void initialise_display(void)
{

 vision_mask = al_create_bitmap(settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
 if (!vision_mask)
	{
  fpr("\nError: vision_mask bitmap creation failed.");
		error_call();
	}

	int i;

 for (i = 0; i < MAP_MASKS; i ++)
	{

  vision_mask_map [i] = al_create_bitmap(MAP_DISPLAY_SIZE, MAP_DISPLAY_SIZE);
  if (!vision_mask_map [i])
	 {
 	 fpr("\nError: vision_mask_map [%i] bitmap creation failed.", i);
		 error_call();
	 }

	}


 int j;
 int k;
 int r_prop, g_prop, b_prop;
 int shade_factor;

 for (i = 0; i < BASIC_COLS; i ++)
 {
  r_prop = (base_col_value [i] [3] - base_col_value [i] [0]) / (BASIC_SHADES-1);
  g_prop = (base_col_value [i] [4] - base_col_value [i] [1]) / (BASIC_SHADES-1);
  b_prop = (base_col_value [i] [5] - base_col_value [i] [2]) / (BASIC_SHADES-1);
  for (j = 0; j < BASIC_SHADES; j ++)
  {
  	shade_factor = j;
//  	if (j == BASIC_SHADES - 2)
//				shade_factor = BASIC_SHADES - 1;
//  	if (j == BASIC_SHADES - 1)
//				shade_factor = BASIC_SHADES + 1;


   colours.base [i] [j] = map_rgb(base_col_value [i] [0] + (r_prop * shade_factor),
                                  base_col_value [i] [1] + (g_prop * shade_factor),
                                  base_col_value [i] [2] + (b_prop * shade_factor));//, 120);
   for (k = 0; k < 3; k ++)
			{
    colours.base_trans [i] [j] [k] = map_rgba(base_col_value [i] [0] + (r_prop * shade_factor),
                                  base_col_value [i] [1] + (g_prop * shade_factor),
                                  base_col_value [i] [2] + (b_prop * shade_factor),
																																		45 + k * 90);
			}
  }
  r_prop = (base_fade_col_value [i] [3] - base_fade_col_value [i] [0]) / CLOUD_SHADES;
  g_prop = (base_fade_col_value [i] [4] - base_fade_col_value [i] [1]) / CLOUD_SHADES;
  b_prop = (base_fade_col_value [i] [5] - base_fade_col_value [i] [2]) / CLOUD_SHADES;
  for (j = 0; j < CLOUD_SHADES; j ++)
  {
   colours.base_fade [i] [j] = map_rgba(base_fade_col_value [i] [0] + (r_prop * j),
                                       base_fade_col_value [i] [1] + (g_prop * j),
                                       base_fade_col_value [i] [2] + (b_prop * j),
																																							10 + ((211 * j) / CLOUD_SHADES));
  }
 }

 colours.black = map_rgb(0,0,0);
 colours.none = map_rgba(0,0,0,0);

 set_default_modifiable_colours();

 map_plan_cols();

 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

// work out where the mode buttons go (based on screen size, which cannot change during execution)
// for (i = 0; i < MODE_BUTTONS; i ++)
// {
//   inter.mode_button_x [i] = settings.option [OPTION_WINDOW_W] - ((MODE_BUTTON_SPACING + MODE_BUTTON_SIZE) * MODE_BUTTONS) + ((MODE_BUTTON_SPACING + MODE_BUTTON_SIZE) * i);
//   inter.mode_button_y [i] = MODE_BUTTON_Y; // Since this never changes, need to remove the variable!
// }

// reset_mode_buttons(); // make sure this is done after panels opened etc.

 for (i = 0; i < PRINT_COLS; i ++)
 {
   colours.print [i] = map_rgb(print_col_value [i] [0],
                               print_col_value [i] [1],
                               print_col_value [i] [2]);
 }

#define CONSOLE_BACKGROUND_R 10
#define CONSOLE_BACKGROUND_G 10
#define CONSOLE_BACKGROUND_B 30

 colours.console_background = map_rgba(CONSOLE_BACKGROUND_R, CONSOLE_BACKGROUND_G, CONSOLE_BACKGROUND_B, 100);

 for (i = 0; i < PRINT_COLS; i ++)
 {
  int r_top = print_col_value [i] [0] / 2;
  int g_top = print_col_value [i] [1] / 2;
  int b_top = print_col_value [i] [2] / 2;
  for (j = 0; j < CONSOLE_LINE_FADE; j ++)
  {
   r_prop = (((r_top - CONSOLE_BACKGROUND_R) * j) / CONSOLE_LINE_FADE) + CONSOLE_BACKGROUND_R;
   g_prop = (((g_top - CONSOLE_BACKGROUND_G) * j) / CONSOLE_LINE_FADE) + CONSOLE_BACKGROUND_G;
   b_prop = (((b_top - CONSOLE_BACKGROUND_B) * j) / CONSOLE_LINE_FADE) + CONSOLE_BACKGROUND_B;
   colours.print_fade [i] [j] = map_rgba(r_prop, g_prop, b_prop, (j+1) * 6);
  }

 }

// init_bshapes();

// init_mouse_cursors();
// set_mouse_cursor(0);
}


// These colours are also updated at the start of each game
//  (they probably don't need to be initialised but it can't hurt in case they're used somewhere)
void set_default_modifiable_colours(void)
{

 int p;

 int player_cols [PLAYERS];

 for (p = 0; p < PLAYERS; p ++)
	{
		player_cols [p] = p;
	}

	player_cols [0] = 0;
	player_cols [1] = 1;

	int test_packet_cols [4] = {0,1,2,3};

 set_game_colours(BACK_COLS_BLUE,
																		BACK_COLS_BLUE,
																		4, // players in game
																		player_cols,
																		test_packet_cols); // player_cols};


}

void set_game_colours(int background_col, // index in back_and_hex_colours array
																						int hex_col, // index in back_and_hex_colours array
																						int players, // players in game
																						int player_base_cols [PLAYERS], // index in base_proc_col array
																						int player_packet_cols [PLAYERS]) // index in base_packet_colours array and similar interface array
{

//fpr("\n sgc pbc [1] = %i", player_base_cols [1]);
 w.background_colour [0] = back_and_hex_colours [background_col] [0];
	w.background_colour [1] = back_and_hex_colours [background_col] [1];
	w.background_colour [2] = back_and_hex_colours [background_col] [2];
 map_background_colour();
	w.hex_base_colour [0] = back_and_hex_colours [hex_col] [3]; // hex_base_colour is mapped in map_player_base_colours
	w.hex_base_colour [1] = back_and_hex_colours [hex_col] [4];
	w.hex_base_colour [2] = back_and_hex_colours [hex_col] [5];

//fpr("\n hex cols %i %i %i", w.hex_base_colour [0], w.hex_base_colour [1], w.hex_base_colour [2]);

	int i;

	for (i = 0; i < players; i ++)
	{
		if (settings.replace_colour [i] != -1)
			player_base_cols [i] = settings.replace_colour [i];

		map_player_colours(i, player_base_cols [i], player_packet_cols [i], background_col);
	}


}

// maps player p's colours to colour player_col
//  - is called at startup and can also be called at other times.
void map_player_colours(int p, int base_col, int packet_col, int background_col)
{
//fpr("\n mapping player %i colours to base %i packet %i background %i", p, base_col, packet_col, background_col);
#ifdef SANITY_CHECK
 if (base_col < 0
		|| base_col >= TEAM_COLS)
	{
		fpr("\n Error: i_disp_in.c: map_player_colours(): base_col out of range (is %i; should be 0 to %i)", base_col, TEAM_COLS - 1);
		error_call();
	}
#endif


	int i;

 for (i = 0; i < 3; i ++)
 {
// I think the proc_colour_min/max values are just used for map points and maybe a few interface things (not actual process colours)
   w.player[p].proc_colour_min [i] = base_proc_col [base_col] [BASE_TEAM_COL_MIN] [i];
   w.player[p].proc_colour_max [i] = base_proc_col [base_col] [BASE_TEAM_COL_MAX] [i];

   w.player[p].packet_colour_min [i] = base_packet_colours [packet_col] [i * 2];
   w.player[p].packet_colour_max [i] = base_packet_colours [packet_col] [(i * 2) + 1];
   w.player[p].drive_colour_min [i] = base_packet_colours [packet_col] [i * 2]; // for now these just use packet colours
   w.player[p].drive_colour_max [i] = base_packet_colours [packet_col] [(i * 2) + 1];
   w.player[p].interface_colour_base [i] = base_interface_colours [packet_col] [i * 3];
   w.player[p].interface_colour_charge [i] = base_interface_colours [packet_col] [(i * 3) + 1];
   w.player[p].interface_colour_hit [i] = base_interface_colours [packet_col] [(i * 3) + 2];
 }

 map_player_base_colours(p, base_col);
 map_player_packet_colours(p);
 map_player_drive_colours(p);
// note that interface colours are not mapped here; they're mapped when used, instead
 map_hex_colours(p, background_col);



}


void map_background_colour(void)
{

	 colours.world_background = map_rgb(w.background_colour [0],
																												         w.background_colour [1],
																												         w.background_colour [2]);

}


// This function sets up player p's proc and team colours (but not packet or drive colours)
// It is called by init functions in this file, which set the colours to default values,
// It assumes that the player[p].proc_colour_min and proc_colour_max values have been set up
static void map_player_base_colours(int p, int colour_index)
{

// int col [3]; // passed to colour_fraction to store results
 int base_cols [3];
// int max_cols [3];
// int a; // alpha
 int i;
 int j;

 base_cols [0] = w.player[p].proc_colour_min [0];
 base_cols [1] = w.player[p].proc_colour_min [1];
 base_cols [2] = w.player[p].proc_colour_min [2];
// max_cols [0] = w.player[p].proc_colour_max [0];
// max_cols [1] = w.player[p].proc_colour_max [1];
// max_cols [2] = w.player[p].proc_colour_max [2];


// colour_fraction(base_cols, col, 100, 10);
/*
 for (j = 0; j < PROC_FILL_SHADES; j ++)
 {
  colours.proc_fill [p] [j] [0] = map_rgba(base_cols [0] + ((max_cols [0] - base_cols [0]) * j) / PROC_FILL_SHADES,
                                           base_cols [1] + ((max_cols [1] - base_cols [1]) * j) / PROC_FILL_SHADES,
                                           base_cols [2] + ((max_cols [2] - base_cols [2]) * j) / PROC_FILL_SHADES,
                                           40 + (j * 10));
  colours.proc_fill [p] [j] [1] = map_rgba((base_cols [0] + ((max_cols [0] - base_cols [0]) * j) / PROC_FILL_SHADES) * 1.4,
                                           (base_cols [1] + ((max_cols [1] - base_cols [1]) * j) / PROC_FILL_SHADES) * 1.4,
                                           (base_cols [2] + ((max_cols [2] - base_cols [2]) * j) / PROC_FILL_SHADES) * 1.4,
                                           50 + (j * 10));

//  colours.proc_fill [p] [j] [0] = map_rgba(col [0], col [1], col [2], 40 + (j * 10));
//  colours.proc_fill [p] [j] [1] = map_rgba(col [0] * 1.3, col [1] * 1.3, col [2] * 1.3, 40 + (j * 10));
 }
*/
/*
 colour_fraction(base_cols, col, 100, 10);
 a = 180;
 colours.team [p] [TCOL_FILL_BASE] = map_rgba(col [0] / 1, col [1] / 1, col [2] / 1, a / 2);
 colours.team [p] [TCOL_MAIN_EDGE] = map_rgba(col [0] / 2, col [1] / 2, col [2] / 2, a);
 colours.team [p] [TCOL_METHOD_EDGE] = map_rgba(col [0] / 4, col [1] / 4, col [2] / 4, a / 2);

// the box colours are colours for score boxes and proc info boxes for each player
 colour_fraction(base_cols, col, 50, 0);
 a = 50;
 colours.team [p] [TCOL_BOX_FILL] = map_rgba(col [0], col [1], col [2], a);

 colour_fraction(base_cols, col, 45, -10);
 a = 100;
 colours.team [p] [TCOL_BOX_HEADER_FILL] = map_rgba(col [0], col [1], col [2], a);
*/

// From this point we set colours that can't be too dark (e.g. because text and map points need to be
//  visible against the backgrounds).
// Need to increase brightness if too dark:

 int base_col_proportion;

// need to avoid divide-by-zero:
 if (base_cols [0] <= 0)
	 base_cols [0] = 1;
 if (base_cols [1] <= 0)
	 base_cols [1] = 1;
 if (base_cols [2] <= 0)
	 base_cols [2] = 1;

 if (base_cols [0] >= base_cols [1]
		&& base_cols [0] >= base_cols [2])
	{
		base_col_proportion = 2000 / base_cols [0];
	}
	 else
		{
			if (base_cols [1] >= base_cols [2])
				base_col_proportion = 2000 / base_cols [1];
			  else
				  base_col_proportion = 2000 / base_cols [2];
		}

 base_cols [0] = (base_cols [0] * base_col_proportion) / 10;
 base_cols [1] = (base_cols [1] * base_col_proportion) / 10;
 base_cols [2] = (base_cols [2] * base_col_proportion) / 10;


// colours.team [p] [TCOL_MAP_POINT] = map_rgba(base_cols [0], base_cols [1], base_cols [2], 220);
// colours.team [p] [TCOL_MAP_POINT_FAINT] = map_rgba(base_cols [0], base_cols [1], base_cols [2], 100);
// colours.team [p] [TCOL_MAP_POINT_THICK] = map_rgba(base_cols [0], base_cols [1], base_cols [2], 150);

 colours.team [p] [TCOL_MAP_POINT_MIN] = map_rgba(base_proc_col [colour_index] [BASE_TEAM_COL_MAP_PIXEL] [0],
																																														base_proc_col [colour_index] [BASE_TEAM_COL_MAP_PIXEL] [1],
																																														base_proc_col [colour_index] [BASE_TEAM_COL_MAP_PIXEL] [2],
																																														140);
 colours.team [p] [TCOL_MAP_POINT_MED] = map_rgba(base_proc_col [colour_index] [BASE_TEAM_COL_MAP_PIXEL] [0],
																																														base_proc_col [colour_index] [BASE_TEAM_COL_MAP_PIXEL] [1],
																																														base_proc_col [colour_index] [BASE_TEAM_COL_MAP_PIXEL] [2],
																																														220);
 colours.team [p] [TCOL_MAP_POINT_MAX] = map_rgba(base_proc_col [colour_index] [BASE_TEAM_COL_MAP_PIXEL] [0] * 1.15,
																																														base_proc_col [colour_index] [BASE_TEAM_COL_MAP_PIXEL] [1] * 1.15,
																																														base_proc_col [colour_index] [BASE_TEAM_COL_MAP_PIXEL] [2] * 1.15,
																																														240);

// set base_cols again:
 base_cols [0] = w.player[p].proc_colour_min [0];
 base_cols [1] = w.player[p].proc_colour_min [1];
 base_cols [2] = w.player[p].proc_colour_min [2];

 colours.base_core_r [p] = base_proc_col [colour_index] [PROC_COL_CORE_MUTABLE] [0];
 colours.base_core_g [p] = base_proc_col [colour_index] [PROC_COL_CORE_MUTABLE] [1];
 colours.base_core_b [p] = base_proc_col [colour_index] [PROC_COL_CORE_MUTABLE] [2];

	for (i = 0; i < PROC_COL_LEVELS; i ++)
	{
 	for (j = 0; j < PROC_DAMAGE_COLS; j ++)
		{

   int redness = (PROC_DAMAGE_COLS - j) - 1;
   int not_red_adjust = redness;
/*
Colours:
0 underlay
1 main 1 (these might have slightly different colours but aren't just increasingly bright)
2 main 2
3 main 3
4 link (also maybe some object stuff)
5 object base
6 object 1
7 object 2

*/



  colours.proc_col [p] [j] [0] [i] = map_rgb(base_proc_col [colour_index] [i] [0] + (redness * 6),
                                         base_proc_col [colour_index] [i] [1] - not_red_adjust * 6,
                                         base_proc_col [colour_index] [i] [2] - not_red_adjust * 6);


  colours.proc_col [p] [j] [1] [i] = map_rgb(base_proc_col [colour_index] [i] [0] + (redness * 6) + 60,
                                         base_proc_col [colour_index] [i] [1] - not_red_adjust * 6 + 60,
                                         base_proc_col [colour_index] [i] [2] - not_red_adjust * 6 + 60);

/*
 colours.proc_col [p] [j] [1] [i] = map_rgb(96 + (base_proc_col [colour_index] [i] [0] + (redness * 6)) / 2,
                                                         96 + (base_proc_col [colour_index] [i] [1] - not_red_adjust * 6) / 2,
                                                         96 + (base_proc_col [colour_index] [i] [2] - not_red_adjust * 6) / 2);
*/
/*
  if (i == PROC_COL_MAIN_1)
		{
   colours.proc_col_main_hit_pulse [p] [j] [0] = colours.proc_col [p] [j] [i];

   colours.proc_col_main_hit_pulse [p] [j] [1] = map_rgb(128 + (base_proc_col [colour_index] [i] [0] + (redness * 6)) / 3,
                                                         128 + (base_proc_col [colour_index] [i] [1] - not_red_adjust * 6) / 3,
                                                         128 + (base_proc_col [colour_index] [i] [2] - not_red_adjust * 6) / 3);

		}
*/

		}
	}
/*
 	for (j = 0; j < PROC_DAMAGE_COLS; j ++)
		{

    colours.base_proc_main_col [p] [j] [0] = base_proc_col [colour_index] [PROC_COL_MAIN_1] [0];
    colours.base_proc_main_col [p] [j] [1] = base_proc_col [colour_index] [PROC_COL_MAIN_1] [1];
    colours.base_proc_main_col [p] [j] [2] = base_proc_col [colour_index] [PROC_COL_MAIN_1] [2];

		}
*/

}

static void map_hex_colours(int p, int base_hex_colour)
{


 int base_r, base_g, base_b;
// int team_r, team_g, team_b;
 int k, j, l;

 int base_cols [2] [3];
/*
 base_cols [0] [0] = w.player[p].packet_colour_min [0];
 base_cols [0] [1] = w.player[p].packet_colour_min [1];
 base_cols [0] [2] = w.player[p].packet_colour_min [2];
 base_cols [1] [0] = w.player[p].packet_colour_max [0];
 base_cols [1] [1] = w.player[p].packet_colour_max [1];
 base_cols [1] [2] = w.player[p].packet_colour_max [2];
*/

 base_cols [0] [0] = w.player[p].proc_colour_min [0];
 base_cols [0] [1] = w.player[p].proc_colour_min [1];
 base_cols [0] [2] = w.player[p].proc_colour_min [2];
 base_cols [1] [0] = w.player[p].proc_colour_max [0];
 base_cols [1] [1] = w.player[p].proc_colour_max [1];
 base_cols [1] [2] = w.player[p].proc_colour_max [2];

 int min_max_gap [3];
 for (k = 0; k < 3; k ++)
	{
		min_max_gap [k] = base_cols [1] [k] - base_cols [0] [k];
	}

 base_r = w.background_colour [0];
 base_g = w.background_colour [1];
 base_b = w.background_colour [2];

 float dwh1 = 1.35;
 float dwh2 = 1.9;
 float dwh3 = 2.4;

// first do data well hexes:
 colours.data_well_hexes [0] = map_rgb(base_r * dwh1, base_g * dwh1, base_b * dwh1);
 colours.data_well_hexes [1] = map_rgb(base_r * dwh2, base_g * dwh2, base_b * dwh2);
 colours.data_well_hexes [2] = map_rgb(base_r * dwh3, base_g * dwh3, base_b * dwh3);
// colours.data_well_hexes [0] = map_rgb(base_r * 0.85, base_g * 0.85, base_b * 0.85);
// colours.data_well_hexes [1] = map_rgb(base_r * 1.25, base_g * 1.25, base_b * 1.25);
// colours.data_well_hexes [2] = map_rgb(base_r * 1.6, base_g * 1.6, base_b * 1.6);

// base_r = w.hex_base_colour [0];
// base_g = w.hex_base_colour [1];
// base_b = w.hex_base_colour [2];


// team_r = base_cols [0] [0];// / 6;
// team_g = base_cols [0] [1];// / 6;
// team_b = base_cols [0] [2];// / 6;

 int red, green, blue;

 int variable_r, variable_g, variable_b;


for (l = 0; l < BACKBLOCK_LAYERS; l ++)
{


 base_r = back_and_hex_colours [base_hex_colour] [3];
 base_g = back_and_hex_colours [base_hex_colour] [4];
 base_b = back_and_hex_colours [base_hex_colour] [5];
 variable_r = back_and_hex_colours [base_hex_colour] [6];
 variable_g = back_and_hex_colours [base_hex_colour] [7];
 variable_b = back_and_hex_colours [base_hex_colour] [8];

 for (j = 0; j < BACK_COL_SATURATIONS; j ++)
 {
/*
  colours.back_line [p] [j] = map_rgba(base_r + (team_r * (j+0)) - (20),
                                           base_g + (team_g * (j+0)) - (20),
                                           base_b + (team_b * (j+0)) - (20),
                                           60);*/
//                                           120);
 	float base_proportion = (float) j / (BACK_COL_SATURATIONS + 6);

// 	base_proportion *= 2;

  for (k = 0; k < BACK_COL_FADE; k ++)
  {
  	float fade_proportion = (float) k / 8;//32;// / 4;//BACK_COL_FADE;

  	red = base_r;
  	green = base_g;
  	blue = base_b;

  	if (j > 0)
			{
				if (k == 0)
			 {
				 red += (base_cols [0] [0] * base_proportion);
				 green += (base_cols [0] [1] * base_proportion);
 				blue += (base_cols [0] [2] * base_proportion);
			 }
			  else
					{
				  red += (base_cols [0] [0]) / 2;
				  green += (base_cols [0] [1]) / 2;
 				 blue += (base_cols [0] [2]) / 2;
					}
			}

  	red += (min_max_gap [0] * (fade_proportion)) / 3;
  	green += (min_max_gap [1] * (fade_proportion)) / 3;
  	blue += (min_max_gap [2] * (fade_proportion)) / 3;

    colours.back_fill [l] [p] [j] [k] = map_rgb(red + (variable_r * (BACKBLOCK_LAYERS - l - 1)),
                                                green + (variable_g * (BACKBLOCK_LAYERS - l - 1)),
                                                blue + (variable_b * (BACKBLOCK_LAYERS - l - 1)));

//    colours.back_fill [l] [p] [j] [k] = map_rgb(red * (1.7 - 0.212 * l),
//                                                green * (1.7 - 0.212 * l),
//                                                blue * (1.7 - 0.12 * l));


  }
 }
}

// colours.back_fill [0] [0] [0] = map_rgba(30,30,55,60);
// colours.back_fill [0] [0] [0] = map_rgb(20,20,55);

}


static void map_player_packet_colours(int p)
{

   map_cloud_cols(p, 0, w.player[p].packet_colour_min, w.player[p].packet_colour_max);
// also maps bloom colours

}

static void map_player_drive_colours(int p)
{

   map_cloud_cols(p, 1, w.player[p].drive_colour_min, w.player[p].drive_colour_max);

}


static void map_plan_cols(void)
{

 int i, j;

 for (i = 0; i < PLAN_COL_DESIGN_MODIFIED + 1; i ++)
 {
  for (j = 0; j < PROC_COL_LEVELS; j ++)
  {

   colours.plan_col [i] [j] = map_rgb(base_plan_col [i] [j] [0],
																																						base_plan_col [i] [j] [1],
																																						base_plan_col [i] [j] [2]);

  }
 }

 i = PLAN_COL_BUILD_OKAY;

 for (j = 0; j < PROC_COL_LEVELS; j ++)
 {

  colours.plan_col [i] [j] = map_rgba(base_plan_col [i] [j] [0],
																																						base_plan_col [i] [j] [1],
																																						base_plan_col [i] [j] [2],
																																						80);

 }

 i = PLAN_COL_BUILD_ERROR;

 for (j = 0; j < PROC_COL_LEVELS; j ++)
 {

  colours.plan_col [i] [j] = map_rgba(base_plan_col [i] [j] [0],
																																						base_plan_col [i] [j] [1],
																																						base_plan_col [i] [j] [2],
																																						80);

 }

 i = PLAN_COL_BUILD_QUEUE;

 for (j = 0; j < PROC_COL_LEVELS; j ++)
 {

  colours.plan_col [i] [j] = map_rgba(base_plan_col [i] [j] [0],
																																						base_plan_col [i] [j] [1],
																																						base_plan_col [i] [j] [2],
																																						60);

 }

 i = PLAN_COL_BUILD_QUEUE_HIGHLIGHT;

 for (j = 0; j < PROC_COL_LEVELS; j ++)
 {

  colours.plan_col [i] [j] = map_rgba(base_plan_col [i] [j] [0],
																																						base_plan_col [i] [j] [1],
																																						base_plan_col [i] [j] [2],
																																						80);

 }


}

static void map_cloud_cols(int p, int packet_or_drive, int min_col [3], int max_col [3])
{

  int i;
  int r, g, b;//, a;

  for (i = 0; i < CLOUD_SHADES; i ++)
  {
   r = min_col [0] + (((max_col [0] - min_col [0]) * i) / CLOUD_SHADES);
   g = min_col [1] + (((max_col [1] - min_col [1]) * i) / CLOUD_SHADES);
   b = min_col [2] + (((max_col [2] - min_col [2]) * i) / CLOUD_SHADES);
//   a = 10 + ((211 * i) / CLOUD_SHADES);

   if (packet_or_drive == 0)
			{
    colours.packet [p] [i] = map_rgba(r, g, b, ((221 * i) / CLOUD_SHADES));
 		 colours.bloom_centre [p] [i] = map_rgba(r, g, b, 80);
 		 colours.bloom_edge [p] [i] = map_rgba(r, g, b, 0);
			}
     else
      colours.drive [p] [i] = map_rgba(r, g, b, 50 + ((171 * i) / CLOUD_SHADES));
 }

}





/*
static void colour_fraction(int base_cols [3], int out_cols [3], int fraction, int subtract)
{

 out_cols [0] = check_col(((base_cols [0] * fraction) / 100) - subtract);
 out_cols [1] = check_col(((base_cols [1] * fraction) / 100) - subtract);
 out_cols [2] = check_col(((base_cols [2] * fraction) / 100) - subtract);

}

static int check_col(int col_in)
{
 if (col_in < 0)
  col_in = 0;
 if (col_in > 255)
  col_in = 255;
 return col_in;
}
*/

#ifdef USE_SYSTEM_MOUSE_CURSOR


ALLEGRO_BITMAP* mcurs_bmp [MOUSE_CURSORS];
ALLEGRO_BITMAP* new_bitmap(int w, int h, char* bitmap_name);

void init_mouse_cursors(void)
{

	int i;

	for (i = 0; i < MOUSE_CURSORS; i ++)
	{
		mcurs_bmp [i] = new_bitmap(30, 30, "mouse cursor");
  make_mouse_cursor(mcurs_bmp [i], i); // in i_display.c (so that it can use various drawing functions)
	}


}

// program exits on failure
ALLEGRO_BITMAP* new_bitmap(int x, int y, char* bitmap_name)
{

 ALLEGRO_BITMAP* bmp = al_create_bitmap(x, y);
 if (!bmp)
	{
			fpr("\nError: new_bitmap() bitmap creation failed [%s] (size %i,%i).", bitmap_name, x, y);
	 	error_call();
	}

	return bmp;

}

#endif
