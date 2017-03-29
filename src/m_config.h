
#ifndef H_M_CONFIG
#define H_M_CONFIG

#include <stdint.h>

// DEBUG_MODE gives access to various special commands, etc and does some other stuff like give player 0 free data
//#define DEBUG_MODE

// SANITY_CHECK runs various checks for things that should never happen, and shuts the game down if they do.
// It doesn't seem to slow things down much so I've left it on, at least for now:
//#define SANITY_CHECK

#ifdef SANITY_CHECK
#define sancheck(value, min, max, text) if(value<min||value>=max){fpr("\nError: [%s]=(%i) out of bounds (should be %i to %i).",text,value,min,max-1);error_call();}
#else
#define sancheck(value, min, max, text)
#endif

// RECORDING_VIDEO sets the resolution to 1280x720 (which is usually not possible, and makes some designer stuff unusable, as the minimum vertical resolution is 768)
//  it also allows unlocking of player 1's templates in story mode
//#define RECORDING_VIDEO

// RECORDING_VIDEO_2 hides some of the display elements
//#define RECORDING_VIDEO_2





#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define ANGLE_MASK 8191
#define ANGLE_1 8192
#define ANGLE_2 4096
#define ANGLE_3 2730
// 3 is not exact
#define ANGLE_4 2048
#define ANGLE_5 1638
// not exact
#define ANGLE_6 1365
#define ANGLE_7 1170
// 6 is not exact
#define ANGLE_8 1024
#define ANGLE_10 819
#define ANGLE_12 683
#define ANGLE_16 512
//#define ANGLE_8_3 384
//#define ANGLE_16_3 192
#define ANGLE_32 256
#define ANGLE_64 128
#define ANGLE_128 64
#define ANGLE_256 32
//#define ANGLE_TO_FIXED 4

#define AFX_MASK 0xffffff

#define AFX_ANGLE_1 al_itofix(256)
#define AFX_ANGLE_2 al_itofix(128)
#define AFX_ANGLE_4 al_itofix(64)
#define AFX_ANGLE_8 al_itofix(32)
#define AFX_ANGLE_16 al_itofix(16)
#define AFX_ANGLE_32 al_itofix(8)
#define AFX_ANGLE_64 al_itofix(4)
#define AFX_ANGLE_128 al_itofix(2)
#define AFX_ANGLE_256 al_itofix(1)

typedef int16_t s16b;
typedef uint16_t u16b;

typedef uint32_t timestamp;

/*

IMPORTANT

This program assumes that all integers wrap on overflow.
Unfortunately, according to the C standard this is undefined behaviour for signed integers.
I think it's possible to force the intended behaviour, at least with GCC, by using either of the following compiler flags:
-fwrapv
-fno-strict-overflow
Some sources claim that fno-strict-overflow is more reliable, so I've used that for binary distribution.

*/

#ifndef PI
#define PI 3.141592
#endif

#define PI_2 (PI/2)
#define PI_4 (PI/4)
#define PI_8 (PI/8)
#define PI_16 (PI/16)
#define PI_32 (PI/32)

#ifndef MAX
	#define MAX( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef MIN
	#define MIN( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

// bcode *should* be using 2's complement:
#define BCODE_VALUE_MAXIMUM 32767
#define BCODE_VALUE_MINIMUM -32768

#define SOURCE_TEXT_LINES 2000
#define SOURCE_TEXT_LINE_LENGTH 160

// TEMPLATES_PER_PLAYER is how many templates each player gets
#define TEMPLATES_PER_PLAYER 10

#define fpr(...) fprintf(stdout, __VA_ARGS__)
#define adtf(x, y, ...) al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_YELLOW] [SHADE_MAX], x, y, ALLEGRO_ALIGN_LEFT, __VA_ARGS__)


#ifdef __GNUC__
#define USE_GCC_EXPECT
// Uses GCC's __builtin_expect() for optimising a few things (I haven't tested to make sure this achieves anything)
#endif


// Directory management (not currently implemented):

enum
{
PATH_TYPE_COMPLETE, // the game makes no changes (used for saving/loading files where the correct path should be available from the filechooser or other sources)
PATH_TYPE_MAIN_DIRECTORY, // where the game looks for init.txt
PATH_TYPE_DATA, // the data/ subdirectory
PATH_TYPE_USER, // where the game saves msn.dat
PATH_TYPE_STORY, // the story/ subdirectory

STANDARD_PATH_TYPES

};



#ifndef DIR_DATA
#define DIR_DATA "data/"
#endif

#ifndef DIR_USER
#define DIR_USER ""
#endif

#ifndef DIR_STORY
#define DIR_STORY "story/"
#endif



#endif


