
#ifndef H_I_HEADER
#define H_I_HEADER

#include <allegro5/allegro_font.h>

#define PROC_FILL_SHADES 16
#define CLOUD_SHADES 32


enum
{
MAP_MASK_BASE, // underlying map
MAP_MASK_DRAWN, // changed each frame to indicate visibility. drawn over the base.
MAP_MASK_OPAQUE, // blitted onto drawn if map not revealed
MAP_MASK_TRANS, // blitted onto drawn if map revealed

MAP_MASKS

};

// can probably use font size to work out how to scale the map:
//#define MAP_DISPLAY_SIZE scaleUI_x(FONT_BASIC,190)
#define MAP_DISPLAY_SIZE 190



enum
{
PROC_COL_UNDERLAY,
PROC_COL_CORE_MUTABLE, // this one is changed to show stress pulses. can be reset to colours.base_core_r/g/b [player_index]
PROC_COL_MAIN_1,
PROC_COL_MAIN_2,
PROC_COL_MAIN_3,
PROC_COL_LINK,
PROC_COL_OBJECT_BASE,
PROC_COL_OBJECT_1,
PROC_COL_OBJECT_2,
// when adding to this list, also add to get_zcolour_string() in z_poly.c

PROC_COL_LEVELS
};

enum
{
PLAN_COL_DESIGN_BASIC,
PLAN_COL_DESIGN_ERROR, // must be next entry after PLAN_COL_DESIGN_BASIC
PLAN_COL_DESIGN_LOCKED,
PLAN_COL_DESIGN_MODIFIED, // colour of design when it may be different from process header in current source code
// next ones are transparent colours used when placing new process in game
PLAN_COL_BUILD_OKAY,
PLAN_COL_BUILD_ERROR,
PLAN_COL_BUILD_QUEUE, // when a builder is selected, queued builds appear on screen in this colour
PLAN_COL_BUILD_QUEUE_HIGHLIGHT, // ... or this colour if the mouse is over the queue button


PLAN_COLS
};

struct coloursstruct
{

 ALLEGRO_COLOR team [PLAYERS] [TCOLS]; // this could be simplified as not much of the TCOL array is currently used.
// ALLEGRO_COLOR proc_fill [PLAYERS] [PROC_FILL_SHADES] [2];
 ALLEGRO_COLOR packet [PLAYERS] [CLOUD_SHADES];
 ALLEGRO_COLOR bloom_centre [PLAYERS] [CLOUD_SHADES];
 ALLEGRO_COLOR bloom_edge [PLAYERS] [CLOUD_SHADES]; // zero alpha but other components same as bloom_centre
 ALLEGRO_COLOR drive [PLAYERS] [CLOUD_SHADES];
// ALLEGRO_COLOR virtual_method [PLAYERS] [CLOUD_SHADES];
 ALLEGRO_COLOR world_background;
 ALLEGRO_COLOR back_fill [BACKBLOCK_LAYERS] [PLAYERS] [BACK_COL_SATURATIONS] [BACK_COL_FADE];
 ALLEGRO_COLOR data_well_hexes [3];
// ALLEGRO_COLOR back_line [PLAYERS] [BACK_COL_SATURATIONS];

#define PROC_DAMAGE_COLS 10
//#define BUILD_FADE_LEVELS 16

 ALLEGRO_COLOR proc_col [PLAYERS] [PROC_DAMAGE_COLS] [2] [PROC_COL_LEVELS];
 ALLEGRO_COLOR proc_outline [PLAYERS] [PROC_DAMAGE_COLS] [PROC_COL_LEVELS]; // not sure this is needed
 int base_core_r [PLAYERS];
 int base_core_g [PLAYERS];
 int base_core_b [PLAYERS];

// ALLEGRO_COLOR proc_col_main_hit_pulse [PLAYERS] [PROC_DAMAGE_COLS] [2];

// int base_proc_main_col [PLAYERS] [PROC_DAMAGE_COLS] [3];


 ALLEGRO_COLOR plan_col [PLAN_COLS] [PROC_COL_LEVELS];


// These interface colours are fixed at startup
 ALLEGRO_COLOR base [BASIC_COLS] [BASIC_SHADES];
 ALLEGRO_COLOR base_trans [BASIC_COLS] [BASIC_SHADES] [BASIC_TRANS];
 ALLEGRO_COLOR base_fade [BASIC_COLS] [CLOUD_SHADES];
 ALLEGRO_COLOR print [PRINT_COLS];
 ALLEGRO_COLOR print_fade [PRINT_COLS] [CONSOLE_LINE_FADE];
 ALLEGRO_COLOR console_background;

 ALLEGRO_COLOR black;
 ALLEGRO_COLOR none;

};

extern struct coloursstruct colours;

struct fontstruct
{
	ALLEGRO_FONT* fnt;
	int width; // fixed width fonts only
	int height; // specified height may be ignored
	float font_scale_x;
	float font_scale_y;
};

#define scaleUI_x(font_type,value) (value * font[font_type].font_scale_x)
#define scaleUI_y(font_type,value) (value * font[font_type].font_scale_y)

enum
{
FONT_BASIC,
//FONT_BASIC_BOLD,
FONT_SQUARE,
//FONT_SQUARE_BOLD,
FONT_SQUARE_LARGE,

FONT_BASIC_UNSCALED, // used for a few interface details that aren't scaled
FONTS
};

// the following are used for the size of display buffers declared in i_display.c and used in one or two other places:
//#define POLY_BUFFER 20000
//#define POLY_TRIGGER 19000
//#define LINE_BUFFER 20000
//#define LINE_TRIGGER 19000


//#define VERTEX_BUFFER_SIZE 20000
//#define VERTEX_BUFFER_TRIGGER 19000

//#define LINE_VERTEX_BUFFER_SIZE 20000
//#define LINE_VERTEX_BUFFER_TRIGGER 19000

#define DISPLAY_LAYERS 5

//#define VERTEX_INDEX_SIZE 14000
//#define VERTEX_INDEX_TRIGGER 12000

#define VERTEX_INDEX_SIZE 34000
#define VERTEX_INDEX_TRIGGER 32000

#define VERTEX_BUFFER_SIZE (VERTEX_INDEX_SIZE * DISPLAY_LAYERS + 1)
#define VERTEX_BUFFER_TRIGGER (VERTEX_BUFFER_SIZE - 2000)


//#define VERTEX_INDEX_SIZE 4000
//#define VERTEX_INDEX_TRIGGER 3000



struct vbuf_struct
{

 int vertex_pos_triangle; // position in buffer
 ALLEGRO_VERTEX buffer_triangle [VERTEX_BUFFER_SIZE];
 int index_triangle [DISPLAY_LAYERS] [VERTEX_INDEX_SIZE];
 int index_pos_triangle [DISPLAY_LAYERS]; // position in triangle index

 int vertex_pos_line; // position in buffer
 ALLEGRO_VERTEX buffer_line [VERTEX_BUFFER_SIZE]; // this could be made much smaller
 int index_line [DISPLAY_LAYERS] [VERTEX_INDEX_SIZE];
 int index_pos_line [DISPLAY_LAYERS]; // position in line index
// the indices don't really need to be as large as the buffer


};


#endif
