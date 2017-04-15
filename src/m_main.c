/*

Liberation Circuit
Copyright 2017 Linley Henzell
Licensed under the GPL v3 (or any later version)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


How to compile

I use Code::Blocks and don't really understand makefiles, so there isn't one.
It shouldn't be too hard to compile the game, though. Basically, compile
everything in the \src directory and link it to Allegro 5.

*** Don't try to compile the ".c" files in the "proc" or "story" subdirectories!
They are process programs for the game's own compiler. (they have the
.c extension to make it easier to load them into an IDE)

The game should run on any platform supported by Allegro 5. It shouldn't have
any other dependencies.

The game assumes that signed integers will wrap on overflow and may run
(slightly) differently if compiled in an environment where this is not true.
In gcc, compile with the
 -fno-strict-overflow
flag to tell the compiler not to optimise on the basis of signed integer
overflow behaviour being undefined (not sure about other compilers, sorry).

I also use
 -ffast-math
which I think only optimises floating-point maths.

Currently the game supports code that right-bitshifts negative values.
I may need to prevent this (by removing the >> operator from the compiler)
if it causes compatibility problems, although I'm not sure how likely it
is to be an issue in practice.

I also compile with -O3


Structure

The source code is organised into categories indicated by the first letter
of each file. Each .c file has a header file, and there are some other special
header files as well.

The code was built on top of the earlier code for Invincible Countermeasure, and
it still contains all kinds of useless stuff that I need to remove sometime.


The categories are:


Main/miscellaneous

m_input.c - contains Allegro calls that read user input (see also i_input.c)
m_main.c - this file. Contains the main function and some initialisation stuff
m_maths.c - special maths functions

m_config.h - header file containing some configuration options
m_globvars.h - a few global variable extern declarations


Game

g_cloud.c - handles clouds (particle effects and similar)
g_command.c - user selection of processes, and giving of commands
g_game.c - contains the main game loop and some initialisation stuff
g_group.c - not currently used
g_method.c - code for executing object methods and class calls
g_method_clob.c - not currently used
g_method_core.c - core and component (member) methods
g_method_misc.c - some miscellaneous method code
g_method_pr.c - not currently used
g_method_std.c - standard methods
g_method_sy.c - not currently used
g_method_uni.c - some other standard methods that are internally treated as "universal" methods (this distinction is currently not relevant to gameplay)
g_misc.c - some miscellaneous game stuff
g_motion.c - handles game physics
g_packet.c - runs packets (bullets)
g_proc.c - deals with certain aspects of process creation and destruction
g_proc_new.c - deals with process creation
g_proc_run.c - runs processes
g_shapes.c - initialises the shape (process/component design and collision information) data structures
g_world.c - initialises and runs aspects of the game world
g_world_back.c - map background
g_world_map.c - map background
g_world_map_2.c - map background

g_header.h - contains a vast amount of game data declarations


Story (starts with h mostly because of its place in the alphabet)

h_interface.c - story mode region selection interface
h_mission.c - sets up story missions
h_story.c - general story code


Interface (runs the display)

i_background.c - some background stuff. May not actually do anything at the moment.
i_buttons.c - contains code for interface buttons
i_console.c - runs consoles (the text boxes that appear on the game display) and a few other things
i_disp_in.c - contains some display initialisation code
i_display.c - runs the display
i_error.c - writes some errors to consoles. I don't think this is currently used.
i_input.c - some input functions that really should be integrated into m_input.c
i_sysmenu.c - mostly unused. sysmenu display is mostly in p_draw.c now, I think
i_view.c - contains some view initialisation stuff

i_header.h - display-related declarations


Panels (the windows that appear on the right of the screen)

- the panel code in general is a horrible mess that needs to be rebuilt completely,
  but it mostly works.
- most of the code for the editor panel is in the e_*.c files.

p_draw.c - draws the panel display
p_init.c - initialises the panels
p_panels.c - general panl code


Sound effects

x_init.c - initialises sound. Creates the separate sound thread.
x_music.c - music code
x_sound.c - sound effect code
x_synth.c - synthesiser


Start (runs the start menu and some other things that actually belong in the g_ files)

s_menu.c - runs the start menu
s_mission.c - loads and starts missions
s_turn.c - not currently used


Templates

t_draw.c - draws the template panel
t_files.c - opens and reads files for templates
t_init.c - initialises templates
t_template.c - handles templates and the Te template menu


Virtual machine

v_interp.c - contains the bcode interpreter


Compiler

c_compile.c - the compiler
c_fix.c - converts the #process header into a process design for a template
c_generate.c - generates bcode from the compiler's output
c_init.c - initialises the compiler
c_keywords.c - giant list of compiler keywords
c_lexer.c - the compiler's lexer
c_prepr.c - the minimal preprocessor

c_header.h - contains a lot of compiler-related stuff


Process designer

d_code.c - the autocoder (generates code for user-designed processes)
d_code_header.c - generates the #process header from the contents of the design window
d_design.c - general design code
d_draw.c - draws the designer panel
d_geo.c - some designer geometry stuff


Code editor

e_clip.c - clipboard (cut/paste, undo/redo etc)
e_complete.c - code completion
e_editor.c - general code for the editor
e_files.c - opening/closing files
e_help.c - help system
e_inter.c - editor interface stuff
e_log.c - the message log (used by other panels as well)
e_slider.c - code to run sliders/scrollbars (used by other panels as well)
e_tools.c - some random editor stuff

e_header.h - general editor declarations


Files
 - saved games are not currently implemented, so none of these files do much, if anything

f_game.c - gamefiles (no longer used)
f_load.c - loads and verifies saved games (not implemented)
f_save.c - saves games (not implemented)
f_turn.c - turnfiles (not implemented)


Shape editor

z_poly.c - an editor for me to use to design components. Generates code
 to be pasted into g_shapes.c. It's not user-friendly and using it at all
	requires recompilation.


*/

#include "m_config.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>
#include <stdio.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <string.h>

#include "g_header.h"

#include "i_header.h"
#include "i_disp_in.h"
#include "i_display.h"
#include "i_view.h"
#include "i_sysmenu.h"
#include "g_game.h"
#include "g_misc.h"

#include "c_header.h"
#include "e_slider.h"
#include "e_header.h"
#include "e_editor.h"
#include "t_template.h"
#include "t_files.h"
#include "m_input.h"
#include "s_menu.h"
#include "s_mission.h"
#include "x_init.h"
#include "x_sound.h"

#include "m_maths.h"

#include "p_init.h"
#include "p_panels.h"

#include "g_shapes.h"

#include "h_interface.h"
#include "h_story.h"

#include "z_poly.h"

// timer interrupt functions and variables:
void framecount(void);

volatile int framecounter;
volatile int frames_per_second;

// startup function declarations:
void init_at_startup(void);
int finish_initfile_line(char* buffer, int buffer_length, int bpos);
int read_initfile_line(char* buffer, int buffer_length, int bpos);
int read_initfile_number(int* read_number, char* buffer, int buffer_length, int bpos);
int read_initfile_word(char* ifword, char* buffer, int buffer_length, int bpos);
char check_initfile_char(char* buffer, int buffer_length, int bpos);
void load_font(int f, const char* font_file_name, int height, float font_scale_x, float font_scale_y);
void read_initfile(void);
void init_inter(void);

extern ALLEGRO_DISPLAY* display;
extern ALLEGRO_BITMAP* display_window; // in i_display.c
extern ALLEGRO_EVENT_QUEUE* event_queue;
extern ALLEGRO_EVENT_QUEUE* fps_queue;
struct fontstruct font [FONTS];
ALLEGRO_TIMER* timer;
ALLEGRO_TIMER* timer_1_second;

// the following are all externed in m_globvars.h:
struct settingsstruct settings;
struct world_struct w;
struct ex_control_struct ex_control;
struct inter_struct inter;

extern ALLEGRO_BITMAP* title_bitmap; // in s_menu.c

int main(int argc, char **argv)
{

   display = NULL;
   timer = NULL;


   if (!al_init())
   {
      fprintf(stdout, "\nError: failed to initialize Allegro.");
      return -1;
   }


   timer = al_create_timer((float) 1.0/60);
   if (!timer)
   {
      fprintf(stdout, "\nError: failed to create timer.");
      return -1;
   }

   timer_1_second = al_create_timer(1);
   if (!timer_1_second)
   {
      fprintf(stdout, "Error: failed to create second timer.");
      return -1;
   }

 al_start_timer(timer);
 al_start_timer(timer_1_second);
/*
 int i;

 for (i = 0; i < 50; i ++)
	{
		fpr("\n i=%i hypot(%i, 33) = %f distance(%i, 33) = %f", i, i, al_fixtof(al_fixhypot(al_itofix(i), al_itofix(33))), i, al_fixtof(distance(al_itofix(i), al_itofix(33))));

	}
	error_call();
*/
// init_random_numbers(0); // this initialises the rand seed (using srand) and also prepares the irand buffer (see g_misc.c)

 init_drand(); // initialises pseudorandom numbers for display

 init_at_startup();

 initialise_display();

fpr("\n display part 2");

// init_shapes(); // for debugging purposes, needs to be after initialise_display()

 init_nshapes_and_dshapes();

fpr("\n geometry");

 init_editor(); // in e_editor.c. Must come after initialise_display

fpr("\n editor");

 init_all_templates(); // in t_template.c. This call must be after init_editor() and init_at_startup()

 load_default_templates(); // in t_template.c. This call must be after init_all_templates() and also after read_initfile().

fpr("\n templates");

 init_sysmenu(); // in i_sysmenu.c. This call must be after init_editor()

#ifdef Z_POLY
 run_zpoly();
#endif

 init_story_interface();

fpr("\nInitialised.\n");

#ifdef DEBUG_MODE
fpr("Debug mode active.");
#endif



/*
al_drop_path_tail(test_path);
fpr("\npath2A [%s]", al_path_cstr(test_path, '/'));
test_path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
fpr("\npath3 [%s]", al_path_cstr(test_path, '/'));
al_drop_path_tail(test_path);
fpr("\npath4 [%s]", al_path_cstr(test_path, '/'));
*/
 start_menus(); // game loop is called from here

 return 0;

}



void init_at_startup(void)
{

   al_install_keyboard();
   al_install_mouse();

   al_init_primitives_addon();

   al_init_image_addon();

   fprintf(stdout, "Liberation Circuit");
   fprintf(stdout, "\nCopyright 2017 Linley Henzell");
   fprintf(stdout, "\nVersion 1.3");

   fprintf(stdout, "\n\nThis is free software and comes with no warranty; see licence.txt.");


//   fprintf(stdout, "\n\nThis program comes with absolutely no warranty!");
/*

       <program>  Copyright (C) <year>  <name of author>
    This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
    This is free software, and you are welcome to redistribute it
    under certain conditions; type `show c' for details.
  */

   fprintf(stdout, "\nFor instructions, read manual.htm.");
   fprintf(stdout, "\nTo configure screen resolution etc, edit init.txt.");
   fprintf(stdout, "\n\nHave fun!\n");


fpr("\nInitialising:");

//   fpr("\n al_ftofix(0.41) = %i", al_ftofix(0.41));  = 26870
//   fpr("\n al_ftofix(0.94124) = %i", al_ftofix(0.94124));  = 61685

//   al_set_new_display_flags(ALLEGRO_FULLSCREEN);

// most of these can be overridden by options in init.txt
   settings.option [OPTION_WINDOW_W] = 1024;
   settings.option [OPTION_WINDOW_H] = 768;
   settings.option [OPTION_FULLSCREEN] = 0;
   settings.option [OPTION_FULLSCREEN_TRUE] = 0;
   settings.option [OPTION_MSAA_OFF] = 0;
   settings.option [OPTION_FAST_BACKGROUND] = 0;
   settings.option [OPTION_NO_BACKGROUND] = 0;
   settings.option [OPTION_VOL_MUSIC] = 80;
   settings.option [OPTION_VOL_EFFECT] = 80;
   settings.option [OPTION_SPECIAL_CURSOR] = 0;
   settings.option [OPTION_CAPTURE_MOUSE] = 0;
   settings.option [OPTION_DOUBLE_FONTS] = 0;
   settings.option [OPTION_LARGE_FONTS] = 0;
   strcpy(settings.path_to_msn_dat_file, "msn.dat");

   init_key_maps(); // must be before read_initfile() as keys may be remapped

// any settings values that could be set in the initfile need to be initialised to default values before read_initfile() is called.
   read_initfile();

// Set up multisampled anti-aliasing
   if (settings.option [OPTION_MSAA_OFF] == 0)
			{
    al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
    al_set_new_display_option(ALLEGRO_SAMPLES, 4, ALLEGRO_SUGGEST); // 4 seems to work okay. Any more doesn't seem to achieve anything.
			}

//settings.option [OPTION_FULLSCREEN] = 1;

//    al_set_new_display_flags(ALLEGRO_OPENGL);

   if (settings.option [OPTION_FULLSCREEN_TRUE] == 1)
   {
// This probably won't work (it may crash if a file dialogue is opened) but support it anyway:
    al_set_new_display_flags(ALLEGRO_FULLSCREEN);

// OPTION_WINDOW_W/H are not used (although I think they are used if for some reason the game swaps out of fullscreen? Not sure)
    display = al_create_display(settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);

    if (!display)
    {
       fprintf(stdout, "\nError: failed to create true fullscreen display.");
       safe_exit(-1);
    }

/*    settings.option [OPTION_WINDOW_W] = al_get_display_width(display);
    settings.option [OPTION_WINDOW_H] = al_get_display_height(display);

    if (settings.option [OPTION_WINDOW_W] < 1024
     || settings.option [OPTION_WINDOW_H] < 768)
    {
       fprintf(stdout, "\nError: display too small (should be at least 1024x768, but is %ix%i).", settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
       safe_exit(-1);
    }
*/
   }
   else
   if (settings.option [OPTION_FULLSCREEN] == 1)
   {
// We use ALLEGRO_FULLSCREEN_WINDOW rather than ALLEGRO_FULLSCREEN here because true fullscreen has problems with native file menus
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);

// OPTION_WINDOW_W/H are not used (although I think they are used if for some reason the game swaps out of fullscreen? Not sure)
    display = al_create_display(settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);

    if (!display)
    {
       fprintf(stdout, "\nError: failed to create fullscreen display.");
       safe_exit(-1);
    }

    settings.option [OPTION_WINDOW_W] = al_get_display_width(display);
    settings.option [OPTION_WINDOW_H] = al_get_display_height(display);

    if (settings.option [OPTION_WINDOW_W] < 1024
     || settings.option [OPTION_WINDOW_H] < 768)
    {
       fprintf(stdout, "\nError: display too small (should be at least 1024x768, but is %ix%i).", settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
       safe_exit(-1);
    }

   }
    else
    {
     display = al_create_display(settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
//    display = al_create_display(100, 100);

     if (!display)
     {
       fprintf(stdout, "\nError: failed to create display window.");
       safe_exit(-1);
     }

    }

//fpr("\n OpenGL %i", al_get_opengl_version());
fpr("\n display");

if (settings.option [OPTION_WINDOW_W] == 1024 && settings.option [OPTION_WINDOW_H] == 768)
{
	fpr(" (Warning: 1024x768 resolution is playable, but higher is recommended.)");
}

   al_init_font_addon(); // initialize the font addon

			load_font(FONT_BASIC_UNSCALED, "data/images/fwss_font.bmp", 12, 1.0, 1.0);

   if (settings.option [OPTION_DOUBLE_FONTS])
			{
    load_font(FONT_BASIC, "data/images/fwss_font_L.bmp", 20, 1.9, 1.6);
    load_font(FONT_SQUARE, "data/images/fwt_font_L.bmp", 20, 1.9, 1.6);
    load_font(FONT_SQUARE_LARGE, "data/images/large_font_L.bmp", 20, 1.9, 1.9);
			}
			 else
				{
     if (settings.option [OPTION_LARGE_FONTS])
			  {
      load_font(FONT_BASIC, "data/images/fwss_font_M.bmp", 14, 1.3, 1.2);
      load_font(FONT_SQUARE, "data/images/fwt_font_M.bmp", 18, 1.5, 1.4);
      load_font(FONT_SQUARE_LARGE, "data/images/large_font_M.bmp", 20, 1.5, 1.5);
			  }
			   else
						{
					  load_font(FONT_BASIC, "data/images/fwss_font.bmp", 12, 1.0, 1.0);
       load_font(FONT_SQUARE, "data/images/fwt_font.bmp", 16, 1.0, 1.0);
       load_font(FONT_SQUARE_LARGE, "data/images/large_font.bmp", 20, 1.0, 1.0);
						}
 			}



fpr("\n fonts");

   init_inter();
fpr("\n interface");

   inter.edit_window_columns = 80;

   init_view_at_startup(settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]); // in i_view.c

   al_set_target_bitmap(al_get_backbuffer(display));

//   if (settings.option [OPTION_SPECIAL_CURSOR] == 0)
//    al_show_mouse_cursor(display);
//     else

      al_hide_mouse_cursor(display); // this can be reset by a few other functions (e.g. when native file dialog or exit game screen is up)

   event_queue = al_create_event_queue();
   if (!event_queue)
   {
      fprintf(stdout, "\nError: failed to create event_queue.");
      al_destroy_display(display);
      al_destroy_timer(timer);
      safe_exit(-1);
   }

   fps_queue = al_create_event_queue();
   if (!fps_queue)
   {
      fprintf(stdout, "\nError: failed to create fps_queue.");
      al_destroy_display(display);
      al_destroy_timer(timer);
      safe_exit(-1);
   }

// abcdefghijklmnopqrstuvwxyz

   al_register_event_source(event_queue, al_get_timer_event_source(timer));
   al_register_event_source(fps_queue, al_get_timer_event_source(timer_1_second));
fpr("\n events");

   init_trig();
fpr("\n maths");

//   init_nearby_distance(); // in maths.c


   init_key_type(); // in m_input.c

   init_ex_control(); // in m_input.c

fpr("\n controls");

   init_vision_area_map(); // in g_game.c

//   init_drag_table(); // in g_motion.c

   w.allocated = 0; // indicates world doesn't need to be deallocated before use

// Read in the mission progress file:
//  load_mission_status_file();
//fpr("\n mission status");

// need a random seed for music initialisation
// doesn't need to be very random, so just use mouse position:

  ALLEGRO_MOUSE_STATE init_mouse_state;
  al_get_mouse_state(&init_mouse_state);
  int music_rand_seed = init_mouse_state.x + (init_mouse_state.y * 1000);

// init_sound must come after read_initfile() (as read_initfile may set volume levels)
  init_sound(music_rand_seed); // calls allegro sound init functions and loads samples. If it fails, it will disable sound (through settings.sound_on)

fpr("\n sound");

 load_story_status_file(); // this prints its own progress report


// load the title bitmap
  title_bitmap = al_load_bitmap("data/images/title.bmp");

  if (!title_bitmap)
		{
			fprintf(stdout, "\nError: failed to load data/images/title.bmp.");
			error_call();
		}

  al_convert_mask_to_alpha(title_bitmap, al_get_pixel(title_bitmap, 0, 0));

}


void load_font(int f, const char* font_file_name, int height, float font_scale_x, float font_scale_y)
{

   ALLEGRO_BITMAP *font_bmp = al_load_bitmap(font_file_name);

   if(!font_bmp)
   {
      fprintf(stdout, "\nError:failed to load font file (%s)", font_file_name);
      safe_exit(-1);
   }

   al_convert_mask_to_alpha(font_bmp, al_get_pixel(font_bmp, 3, 3));

   int ranges[] = {0x0020, 0x007F};
   font[f].fnt = al_grab_font_from_bitmap(font_bmp, 1, ranges);

   if (!font[f].fnt)
   {
      fprintf(stdout, "\nError: failed to grab font from file (%s)", font_file_name);
      error_call();
   }

   al_destroy_bitmap(font_bmp);

// should only use fix-width fonts:
   font[f].width = al_get_text_width(font[f].fnt, "a");

   font[f].height = height;
   font[f].font_scale_x = font_scale_x;
   font[f].font_scale_y = font_scale_y;


}



// Tries to open file init.txt and read game settings from it.
// If it fails for some reason, settings are not updated (they should have already been set to defaults).
void read_initfile(void)
{

// First we need to clear the default template path names, which can be read in from the initfile:
 int i, j;

 for (i = 0; i < PLAYERS; i ++)
	{
		for (j = 0; j < TEMPLATES_PER_PLAYER; j ++)
		{
			settings.default_template_path [i] [j] [0] = 0;
		}

		settings.replace_colour [i] = -1;

	}


#define INITFILE_SIZE 8192
#define INITFILE_WORD_LENGTH FILE_PATH_LENGTH
// INITFILE_WORD_LENGTH needs to be FILE_PATH_LENGTH to accommodate file path names specified for default templates

 FILE *initfile;
 char buffer [INITFILE_SIZE];

 initfile = fopen("init.txt", "rt");

 if (!initfile)
 {
  fprintf(stdout, "\nFailed to open init.txt. Starting with default settings.");
  return;
 }

 int read_in = fread(buffer, 1, INITFILE_SIZE, initfile);

 if (ferror(initfile)
  || read_in == 0)
 {
  fprintf(stdout, "\nFailed to read settings from init.txt. Starting with default settings.");
  fclose(initfile);
  return;
 }

 int bpos = 0;

 while(TRUE)
 {
  bpos = read_initfile_line(buffer, read_in, bpos);
  if (bpos == -1)
   break;
 };

 fclose(initfile);

 if (settings.option [OPTION_VOL_MUSIC] == 0
  && settings.option [OPTION_VOL_EFFECT] == 0)
   settings.sound_on = 0;


#ifdef RECORDING_VIDEO
   settings.option [OPTION_WINDOW_W] = 1280;
   settings.option [OPTION_WINDOW_H] = 720;
   fpr("\n In recording mode (display set to 1280x720)");
#endif

}




// returns -1 if end of file (or error) reached, new position in buffer otherwise
int read_initfile_line(char* buffer, int buffer_length, int bpos)
{

 char initfile_word [INITFILE_WORD_LENGTH];
 int i;

// first check for a # at the start of the line, indicating a comment:
 if (check_initfile_char(buffer, buffer_length, bpos) == '#')
 {
  bpos = finish_initfile_line(buffer, buffer_length, bpos);
  return bpos;
 }

 bpos = read_initfile_word(initfile_word, buffer, buffer_length, bpos);

 if (bpos == -1)
  return -1;

 int read_number = 0;
 int invalid_value_fixed = 0;

 if (strcmp(initfile_word, "display_w") == 0)
 {
  bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
  if (bpos == -1)
   return -1;
  settings.option [OPTION_WINDOW_W] = read_number;
  if (settings.option [OPTION_WINDOW_W] < 1024)
		{
   settings.option [OPTION_WINDOW_W] = 1024;
   invalid_value_fixed = 1;
		}
  if (settings.option [OPTION_WINDOW_W] > 1920)
		{
   settings.option [OPTION_WINDOW_W] = 1920;
   invalid_value_fixed = 1;
		}


		if (invalid_value_fixed != 0)
   fprintf(stdout, "\nDisplay width (%i) fixed to %i.", read_number, settings.option [OPTION_WINDOW_W]);

  return bpos;
 }

 invalid_value_fixed = 0;

 if (strcmp(initfile_word, "display_h") == 0)
 {
  bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
  if (bpos == -1)
   return -1;
  settings.option [OPTION_WINDOW_H] = read_number;
  if (settings.option [OPTION_WINDOW_H] < 768)
		{
   settings.option [OPTION_WINDOW_H] = 768;
   invalid_value_fixed = 1;
		}
  if (settings.option [OPTION_WINDOW_H] > 1200)
		{
   settings.option [OPTION_WINDOW_H] = 1200;
   invalid_value_fixed = 1;
		}
		if (invalid_value_fixed != 0)
   fprintf(stdout, "\nDisplay height (%i) fixed to %i.", read_number, settings.option [OPTION_WINDOW_H]);
  return bpos;
 }

 if (strcmp(initfile_word, "debug") == 0)
 {
  bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
  if (bpos == -1)
   return -1;
  settings.option [OPTION_DEBUG] = read_number;
  return bpos;
 }



 if (strcmp(initfile_word, "true_fullscreen") == 0)
 {
  settings.option [OPTION_FULLSCREEN_TRUE] = 1;
  return bpos;
 }

 if (strcmp(initfile_word, "msaa_off") == 0)
 {
  settings.option [OPTION_MSAA_OFF] = 1;
  return bpos;
 }

 if (strcmp(initfile_word, "capture_mouse") == 0)
 {
  settings.option [OPTION_CAPTURE_MOUSE] = 1;
  return bpos;
 }

 if (strcmp(initfile_word, "fullscreen") == 0)
 {
  settings.option [OPTION_FULLSCREEN] = 1;
  return bpos;
 }

 if (strcmp(initfile_word, "special_cursor") == 0) // not currently implemented
 {
  settings.option [OPTION_SPECIAL_CURSOR] = 1;
  return bpos;
 }

 if (strcmp(initfile_word, "double_fonts") == 0)
 {
  settings.option [OPTION_DOUBLE_FONTS] = 1;
  return bpos;
 }

 if (strcmp(initfile_word, "large_fonts") == 0)
 {
  settings.option [OPTION_LARGE_FONTS] = 1;
  return bpos;
 }

 invalid_value_fixed = 0;

 if (strcmp(initfile_word, "vol_music") == 0)
 {
  bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
  if (bpos == -1)
   return -1;
  settings.option [OPTION_VOL_MUSIC] = read_number;
  if (settings.option [OPTION_VOL_MUSIC] < 0)
		{
   settings.option [OPTION_VOL_MUSIC] = 0;
   invalid_value_fixed = 1;
		}
  if (settings.option [OPTION_VOL_MUSIC] > 100)
		{
   settings.option [OPTION_VOL_MUSIC] = 100;
   invalid_value_fixed = 1;
		}
		if (invalid_value_fixed != 0)
   fprintf(stdout, "\nMusic volume (%i) fixed to %i.", read_number, settings.option [OPTION_VOL_MUSIC]);
  return bpos;
 }

 invalid_value_fixed = 0;

 if (strcmp(initfile_word, "vol_effect") == 0)
 {
  bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
  if (bpos == -1)
   return -1;
  settings.option [OPTION_VOL_EFFECT] = read_number;
  if (settings.option [OPTION_VOL_EFFECT] < 0)
		{
   settings.option [OPTION_VOL_EFFECT] = 0;
   invalid_value_fixed = 1;
		}
  if (settings.option [OPTION_VOL_EFFECT] > 100)
		{
   settings.option [OPTION_VOL_EFFECT] = 100;
   invalid_value_fixed = 1;
		}
		if (invalid_value_fixed != 0)
   fprintf(stdout, "\nEffects volume (%i) fixed to %i.", read_number, settings.option [OPTION_VOL_EFFECT]);
  return bpos;
 }

 invalid_value_fixed = 0;

 if (strcmp(initfile_word, "background_detail") == 0)
 {
  bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
  if (bpos == -1)
   return -1;
		switch(read_number)
		{
			case 0: settings.option [OPTION_NO_BACKGROUND] = 1; break;
			case 1: settings.option [OPTION_FAST_BACKGROUND] = 1; break;
			default: fprintf(stdout, "\nInvalid background_detail setting (%i) - should be 0 or 1.", read_number);

		}

  return bpos;
 }

 if (strcmp(initfile_word, "standard_paths") == 0)
 {
  bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
  if (bpos == -1)
   return -1;
		switch(read_number)
		{
			case 0:
				settings.option [OPTION_STANDARD_PATHS] = STANDARD_PATHS_NONE;
				break;
			case 1:
				settings.option [OPTION_STANDARD_PATHS] = STANDARD_PATHS_EXECUTABLE;
				break;
			case 2:
				settings.option [OPTION_STANDARD_PATHS] = STANDARD_PATHS_VARIOUS;
				break;
			default: fprintf(stdout, "\nInvalid standard_paths setting (%i) - should be 0, 1 or 2.", read_number);

		}

  return bpos;
 }





static int default_templates_loaded [PLAYERS] = {0,0,0,0};

 if (strcmp(initfile_word, "template") == 0)
 {
 	int player_index;
  bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
  if (bpos == -1)
   return -1;
// just floor/cap out-of-bounds numbers:
  player_index = read_number;
  if (player_index < 0)
			player_index = 0;
  if (player_index >= PLAYERS)
			player_index = PLAYERS - 1;
// now read file name:
  char read_default_template_path [INITFILE_WORD_LENGTH]; // currently INITFILE_WORD_LENGTH should be FILE_PATH_LENGTH
  bpos = read_initfile_word(read_default_template_path, buffer, buffer_length, bpos);
  if (bpos == -1)
			return -1;
		if (default_templates_loaded [player_index] < TEMPLATES_PER_PLAYER)
		{
		 strcpy(settings.default_template_path [player_index] [default_templates_loaded [player_index]], read_default_template_path);
		 default_templates_loaded [player_index] ++;
		}
		 else
				fpr("\nFailed to read default template path for player %i: too many templates.\n(path [%s])", player_index, read_default_template_path);
// the file will actually be loaded at a later stage of initialisation.
  return bpos;
 }


 if (strcmp(initfile_word, "savefile") == 0)
 {
// read file name:
  char read_savefile_path [INITFILE_WORD_LENGTH]; // currently INITFILE_WORD_LENGTH should be FILE_PATH_LENGTH
  bpos = read_initfile_word(read_savefile_path, buffer, buffer_length, bpos);
  if (bpos == -1)
			return -1;
		strncpy(settings.path_to_msn_dat_file, read_savefile_path, FILE_PATH_LENGTH - 5);
		fpr("\nSave file set to [%s].", settings.path_to_msn_dat_file);
// the file will actually be loaded at a later stage of initialisation.
  return bpos;
 }


 if (strcmp(initfile_word, "keymap") == 0)
 {
 	fpr("\n found keymap ");
// first read in the key that will be assigned
  bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
  if (bpos == -1)
   return -1;
  if (read_number < 0
			|| read_number >= SPECIAL_KEYS)
		{
   fprintf(stdout, "\nkeymap failed - invalid keymap function %i (should be 0 to %i).", read_number, SPECIAL_KEYS-1);
// read past next number
   bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
   if (bpos == -1)
    return -1;
   return bpos;
		}
		int special_key_to_remap = read_number;
// now read in the key code that it's being remapped to:
  bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
  if (bpos == -1)
   return -1;
  if (read_number < 1
			|| read_number >= ALLEGRO_KEY_MAX)
		{
   fprintf(stdout, "\nkeymap failed - key %i being mapped to invalid key code %i (should be 1 to %i).", special_key_to_remap, read_number, ALLEGRO_KEY_MAX-1);
   return bpos;
		}
// now modify the keymap (which should previously have been initialised, in m_input.c)
//  - find all key codes which are mapped to the function being remapped (there may be more than one)
//  - first found is remapped; any others are set to -1
					fpr("\n remapping function %i to key %i", special_key_to_remap, read_number);
  int found_remap = 0;
  for (i = 0; i < SPECIAL_KEYS; i ++)
		{
//			fpr("[%i:%i]", i, ex_control.key_code_map [i] [1]);
			if (ex_control.key_code_map [i] [1] == special_key_to_remap) // [1] contains the SPECIAL_KEY_ value
			{
				if (!found_remap)
				{
					fpr("\n remapped function %i to key %i", special_key_to_remap, read_number);
				 ex_control.key_code_map [i] [0] = read_number; // [0] contains the ALLEGRO_KEY_ value
				 found_remap = 1;
				}
				 else
				  ex_control.key_code_map [i] [0] = -1; // no key
			}
		}
  return bpos;
 }


 if (strcmp(initfile_word, "replace_col") == 0)
 {
// first read in the player index
  bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
  if (bpos == -1)
   return -1;
  if (read_number < 0
			|| read_number >= PLAYERS)
		{
   fprintf(stdout, "\nreplace colour failed - invalid player %i (should be 0 to %i).", read_number, PLAYERS-1);
// read past next number
   bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
   if (bpos == -1)
    return -1;
   return bpos;
		}
		int player_index = read_number;
// now read in the colour that will be used:
  bpos = read_initfile_number(&read_number, buffer, buffer_length, bpos);
  if (bpos == -1)
   return -1;
  if (read_number < 0
			|| read_number >= TEAM_COLS)
		{
   fprintf(stdout, "\nreplace colour failed - player %i being set to invalid colour %i (should be 0 to %i).", player_index, read_number, TEAM_COLS-1);
   return bpos;
		}
// now modify the keymap (which should previously have been initialised, in m_input.c)
//  - find all key codes which are mapped to the function being remapped (there may be more than one)
//  - first found is remapped; any others are set to -1
					fpr("\n replacing player %i colour with colour %i", player_index, read_number);

					settings.replace_colour [player_index] = read_number;

  return bpos;
 }



// unrecognised word, so just go to the next word:

//  bpos = finish_initfile_line(buffer, buffer_length, bpos);

  return bpos;
}


char check_initfile_char(char* buffer, int buffer_length, int bpos)
{

 if (bpos >= buffer_length)
  return 0;

 return buffer [bpos];

}

// returns new value of bpos on success, or -1 if EOL or EOF found
int read_initfile_word(char* ifword, char* buffer, int buffer_length, int bpos)
{

 int word_pos = 0;

 while(TRUE)
 {
  if (bpos >= buffer_length)
   return -1;
  if (buffer [bpos] == '\0'
   || buffer [bpos] == '\n'
   || buffer [bpos] == '\r'
   || buffer [bpos] == ' '
   || word_pos >= INITFILE_WORD_LENGTH - 1)
  {
   ifword [word_pos] = '\0';
   bpos++;
   return bpos;
  }
  ifword [word_pos] = buffer [bpos];
  word_pos++;
  bpos++;
 };

 return bpos;

}



// returns new value of bpos on success, or -1 if EOL or EOF found
int read_initfile_number(int* read_number, char* buffer, int buffer_length, int bpos)
{
#define INITFILE_NUMBER_LENGTH 6
 int number_pos = 0;
 int number_string [INITFILE_NUMBER_LENGTH];
 *read_number = 0;

 while(TRUE)
 {
  if (bpos >= buffer_length)
   return -1;
  if (buffer [bpos] < '0'
   || buffer [bpos] > '9'
   || number_pos >= INITFILE_NUMBER_LENGTH - 1)
  {
   bpos++;
   break;
  }
  number_string [number_pos] = buffer [bpos] - '0';
  number_pos++;
  bpos++;
 };

 int multiplier = 1;

 while(number_pos > 0)
 {
  number_pos --;
  *read_number += number_string [number_pos] * multiplier;
  multiplier *= 10;
 };

 return bpos;

}

int finish_initfile_line(char* buffer, int buffer_length, int bpos)
{

 while(TRUE)
 {
  if (bpos >= buffer_length + 1)
   return -1;
  if (buffer [bpos] == '\n'
   || buffer [bpos] == '\r')
   return bpos + 1;
  bpos ++;
 };

 return bpos;

}




void init_inter(void)
{

	inter.display_w = settings.option [OPTION_WINDOW_W];
	inter.display_h = settings.option [OPTION_WINDOW_H];

//	fprintf(stdout, "\ninter.display_w %i h %i", inter.display_w, inter.display_h);

//	inter.panel_input_capture = PANEL_MAIN;

	inter.edit_window_columns = 80;

	inter.mlog_x1 = inter.display_w / 2; // this is actually set in panel code
	inter.mlog_y1 = inter.display_h - LOG_WINDOW_H;

	int i;

	for (i = 0; i < PANEL_BCODE+1; i ++)
	{
		inter.panel_restore [i] = 0; // PANEL_LOG value is just ignored
	}

	inter.block_mode_button_area_scrolling = 0;

 init_panels();
}


