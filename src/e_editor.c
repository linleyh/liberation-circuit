
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>


#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"

#include "g_misc.h"

#include "c_header.h"
#include "e_slider.h"

#include "e_header.h"
#include "e_inter.h"
#include "e_tools.h"
#include "e_help.h"
#include "i_header.h"
#include "i_input.h"
#include "i_view.h"
#include "m_input.h"
#include "e_log.h"
#include "e_clip.h"
#include "e_files.h"
#include "e_complete.h"
#include "e_editor.h"
#include "d_draw.h"
#include "c_compile.h"
#include "c_prepr.h"
#include "g_world.h"

#include "p_panels.h"
#include "t_template.h"

extern struct fontstruct font [FONTS];

// these queues are declared in g_game.c. They're externed here so that they can be flushed when the editor does something slow.
extern ALLEGRO_EVENT_QUEUE* event_queue; // these queues are initialised in main.c
extern ALLEGRO_EVENT_QUEUE* fps_queue;
extern ALLEGRO_EVENT_QUEUE* control_queue; // in m_input.c

extern struct view_struct view;
extern struct log_struct mlog;
extern struct slider_struct slider [SLIDERS];

extern struct game_struct game;

/*

This file contains functions for displaying source code and editing text.

Keep in mind that it may be good to let the player read and maybe also edit source code while watching a game,
so try to make the display code work alongside the game code if needed.


How is this going to work??

I think there will be a main row of tabs across the very top of the screen:

 main | world | editor

 - main contains options for starting games, quitting etc
 - world is the game currently being run
  - it will also be how players start off games (by filling in boxes with filenames etc)
  - and will also be used for e.g. setting up PBEM games
 - editor displays source and allows editing (I think)
- MAYBE NOT - maybe don't separate world and editor, which means main could just be an icon somewhere.

- It should also be possible to split the screen between world and editor.
 - so write the editor in a way that allows it to be arbitrarily resized and run alongside world execution.

- should put display functions in a separate file
- should probably set up a special bitmap for the editor as it will only need to be updated every few frames at most.
 - draw cursor separately.

- input should be handled by catching clicks and presses

Need to do:

int source_to_editor(struct source_struct* src)
 - takes a source_struct and loads it into a source_edit_struct

int editor_input(
 - call this function at the start of normal input functions if the editor is visible
 - returns 1 if all input captured by editor, 0 if some input may remain
 - captures:
  - all mouse clicks within editor panel
  - holding mouse IF the mouse started being held when pointer within editor
   - doesn't capture holding mouse if the mouse starting being held outside. In this case, game gets holding value and pointer treated as at edge of game panel.
  - if editor is focussed, all key presses.
 - editor becomes focussed if mouse is clicked on it.
 - editor loses focus if mouse is clicked elsewhere. Click is not ignored if within game window.

void display_editor_panel() (in a separate file)
 - draws editor panel bitmap to screen

- void update_editor_panel_bitmap()
 - refreshes or initialises editor panel bitmap if something has been changed.


editor panel looks like:
 - scrollbars at right and bottom
 - File and Edit menus at top
  - or maybe just buttons for various functions - shouldn't need too many.
 - tabs below menus.

*/

struct editorstruct editor;

//static int source_line_highlight_syntax(struct source_edit_struct* se, int src_line, int in_a_comment);
static void add_src_highlight(struct source_edit_struct* se, int src_line, int stoken_pos, int stoken_end, int stoken_type);
//static void init_source_edit_struct(struct source_edit_struct* se);
//static int source_to_editor(struct source_struct* src, int esource);
static int mark_as_long_comment(struct source_edit_struct* se, int src_line, int comment_start);
static void get_cursor_position_from_mouse(struct source_edit_struct* se, int x, int y, int* mouse_cursor_line, int* mouse_cursor_pos);

static void editor_input(void);
static void click_in_edit_window(int x, int y);
static void	click_in_completion_box(struct source_edit_struct* se, int x, int y);
//static void complete_code(struct source_edit_struct* se, int select_line);
static void editor_input_keys(void);
static void editor_keypress(int key_press);
static void editor_keypress_unichar(int unichar_value);
//static void editor_special_keypress(int key_press);
//static void update_source_lines(struct source_edit_struct* se, int sline, int lines);
static void cursor_etc_key(int key_press);
static void reset_cursor_after_action(void);
//static void window_find_cursor(struct source_edit_struct* se);
//static int insert_empty_lines(struct source_edit_struct* se, int before_line, int lines);
//static void delete_lines(struct source_edit_struct* se, int start_line, int lines);
static void movement_keys(struct source_edit_struct* se);
static void consider_selecting_to_cursor(struct source_edit_struct* se, int old_line, int old_pos, int require_shift);
static int get_skip_word_type(char current_char);
static void select_text(int x, int y);
//static int is_something_selected(struct source_edit_struct* se);
//static int delete_selection(void);
//static int just_pressed_ctrl_key(int key);
static void open_submenu(int sm);
static void close_submenu(void);
static void submenu_operation(int sm, int line);
//static void open_tab(int tab);
//static void change_tab(int new_tab);
//static int get_current_source_edit_index(void);
static void editor_change_source_edit(int new_source_edit_index);
static int move_cursor_to_end_of_source(struct source_edit_struct* se);

static void overwindow_input(void);
//static void open_overwindow(int ow_type);

//static void close_editor(void);
struct source_edit_struct* get_current_source_edit(void);

static int compile_current_source_edit(int compile_mode);
//static void template_locked_editor_message(void);

#define KEY_DELAY1 20
#define KEY_DELAY2 1

//s16b source_edit_bcode [ESOURCES] [BCODE_MAX];


// This #define adds a keyboard shortcut (ctrl-R) for the "reload" function that
//  reloads a file from disk.
// It's an option because some players may prefer to avoid having a single shortcut that potentially overwrites work
#define RELOAD_KEYBOARD_SHORTCUT


struct submenustruct submenu [SUBMENUS] =
{

 { // submenu 0
  SUBMENU_FILE_END,
  { // lines
//   {"New", "", HELP_SUBMENU_NEW},
   {"Open", ""},//"Ctrl-O"}, currently can't use a keyboard shortcut for this as Allegro's keyboard routines cause problems when returning from the native file dialogue. TO DO: use al_clear_keyboard_state() when it's available.
#ifdef RELOAD_KEYBOARD_SHORTCUT
   {"Reload", "Ctrl-R"},
#else
   {"Reload", ""},
#endif
   {"Save", "Ctrl-S"},
   {"Save as", ""},
   {"Save all", "Shift-Ctrl-S"},
//   {"Close", "", HELP_SUBMENU_CLOSE_FILE},
//   {"Quit", ""},
  }
 },
 {
  SUBMENU_EDIT_END,
  {
   {"Undo", "Ctrl-Z"},
   {"Redo", "Ctrl-Y"},
   {"Cut", "Ctrl-X"},
   {"Copy", "Ctrl-C"},
   {"Paste", "Ctrl-V"},
   {"Clear", "del"},
   {"Select all", "Ctrl-A"},
  }
 },
 {
  SUBMENU_SEARCH_END,
  {
   {"Find", "Ctrl-F"},
   {"Find next", "F3"}
  }
 },
 {
  SUBMENU_COMPILE_END,
  {
   {"Test Compile", "Shift-F10"},
   {"Compile", "F10"},
   {"Compile+lock", ""},
//   {"Build asm", "", HELP_SUBMENU_BUILD_ASM},
//   {"Crunched asm", "", HELP_SUBMENU_CRUNCH_ASM},
//   {"Convert bcode", "", HELP_SUBMENU_CONVERT_BCODE},
//   {"Import bcode", "", HELP_SUBMENU_IMPORT_BCODE}
  }
 }

};

// call at startup
// should be called before init_templates as some properties of the template window are derived from the editor window
void init_editor(void)
{

// editor.panel_x = settings.editor_x_split;
// editor.panel_y = 0;
// editor.panel_w = panel[PANEL_EDITOR].w;
// editor.panel_h = panel[PANEL_EDITOR].h;
 editor.key_delay = 0;
 editor.selecting = 0;
 editor.special_key_being_pressed = -1;
 editor.ignore_previous_key_pressed = -1;
 editor.cursor_flash = CURSOR_FLASH_MAX;
// editor.first_press = 1;
 editor.submenu_open = -1;

 int i, j;

 for (i = 0; i < PLAYERS; i ++)
 {
 	for (j = 0; j < TEMPLATES_PER_PLAYER; j ++)
		{
   int se_index = (i * TEMPLATES_PER_PLAYER) + j;
   editor.source_edit[se_index].active = 0;
   editor.source_edit[se_index].player_index = i;
   editor.source_edit[se_index].template_index = j;

		}
//  editor.source_edit[i].bcode.op = source_edit_bcode [i];
//  editor.source_edit[i].bcode.bcode_size = BCODE_MAX;
//  editor.source_edit[i].bcode.from_a_file = 0;
/*  editor.tab_index [i] = -1;
  editor.tab_type [i] = TAB_TYPE_NONE;
  editor.tab_name [i] [0] = '\0';
  editor.tab_name_unsaved [i] [0] = '\0';*/
 }

// editor.tab_highlight = -1;
// editor.current_tab = -1;
 editor.overwindow_type = OVERWINDOW_TYPE_NONE;

 editor.search_string [0] = '\0';

 editor.clipboard [0] = '\0';

 init_editor_display(panel[PANEL_EDITOR].w, panel[PANEL_EDITOR].h);
// init_log(editor.edit_window_w, LOG_WINDOW_H); // now in p_init.c

 init_undo();
 init_editor_files();

 completion.list_size = 0; // initialise code completion box

 init_code_completion(); // in e_complete.c


}

void init_source_edit_struct(struct source_edit_struct* se)
{

 clear_source_edit_struct(se); // currently the source_edit struct is just cleared.

}

void clear_source_edit_struct(struct source_edit_struct* se)
{

 strcpy(se->src_file_name, "unsaved");
 strcpy(se->src_file_path, "unsaved");

 se->active = 0;
 se->type = SOURCE_EDIT_TYPE_SOURCE;
 se->from_a_file = 0;

 clear_source_edit_text(se);

}

// clears source_edit text and resets editor stuff like cursor location without reinitialising the source_edit in any other way.
// used to e.g. prepare for autocoder.
void clear_source_edit_text(struct source_edit_struct* se)
{

 se->cursor_line = 0;
 se->cursor_pos = 0;
 se->cursor_base = 0;
 se->window_line = 0;
 se->window_pos = 0;
 se->selected = 0;
 se->saved = 0;

 int i;

 for (i = 0; i < SOURCE_TEXT_LINES; i ++)
 {
  strcpy(se->text [i], "");
  se->line_index [i] = i; // may change through editing
  se->comment_line [i] = 0;
 }

}

// currently this function cannot fail
int source_to_editor(struct source_struct* src, int esource)
{
 int i;

 struct source_edit_struct* se = &editor.source_edit[esource];

 init_source_edit_struct(se);

 se->active = 1;
 strcpy(se->src_file_name, src->src_file_name);
 strcpy(se->src_file_path, src->src_file_path);
// se->from_a_file = 1;
// se->saved = 1; // indicates that the source_edit matches the file on disk

 int in_a_comment = 0;

 for (i = 0; i < SOURCE_TEXT_LINES; i ++)
 {
  strcpy(se->text [i], src->text [i]);
  se->line_index [i] = i; // may change through editing
//  se->comment_line [i] = 0;
  in_a_comment = source_line_highlight_syntax(se, i, in_a_comment);
// if this code is changed, see corresponding code in convert_bcode_to_source_tab() in e_build.c
 }

// wait_for_space();

 return 1;

}


// converts se to source_struct format
// carries over name and path of file (or "untitled" if no name/file)
// returns 1 on success, 0 on failure
int source_edit_to_source(struct source_struct* src, struct source_edit_struct* se)
{

 int i;

 src->from_a_file = se->from_a_file;

 if (se->from_a_file)
 {
  strcpy(src->src_file_name, se->src_file_name);
  strcpy(src->src_file_path, se->src_file_path);

 }
  else
  {
   strcpy(src->src_file_name, "(unnamed)");
   strcpy(src->src_file_path, "(unnamed)");
  }

 for (i = 0; i < SOURCE_TEXT_LINES; i ++)
 {
  strcpy(src->text [i], se->text [se->line_index [i]]);
 }

 return 1;

}


// currently this function cannot fail
int binary_to_editor(struct bcode_struct* bcode, int esource)
{
	/*
 int i;

 struct source_edit_struct* se = &editor.source_edit[esource];

 init_source_edit_struct(se);

 se->active = 1;
 se->type = SOURCE_EDIT_TYPE_BINARY; // init_source_edit_struct sets type by default to _SOURCE
 strcpy(se->src_file_name, bcode->src_file_name);
 strcpy(se->src_file_path, bcode->src_file_path);
 se->from_a_file = 1;
 se->saved = 1; // indicates that the source_edit matches the file on disk
*/
// fprintf(stdout, "bcode size %i (static %i) se bcode size %i (static %i) name [%s] path [%s]", bcode->bcode_size, bcode->static_length,
//									se->bcode.bcode_size, se->bcode.static_length, se->src_file_name, se->src_file_path);
/*
 for (i = 0; i < bcode->bcode_size; i ++)
 {
  se->bcode.op[i] = bcode->op[i];
 }

 se->bcode.static_length = bcode->static_length;*/

 return 1;

}


// returns 1 if the line ends within a multi-line comment, 0 otherwise
// src_line is position in text array, not line index array
int source_line_highlight_syntax(struct source_edit_struct* se, int src_line, int in_a_comment)
{

// char stoken_string [SOURCE_TEXT_LINE_LENGTH];
 char read_char;
 int char_type;
 int stoken_type;
// int leading_zero = 0; // used to find binary or hex numbers - not currently implemented (anything starting with a number is treated as a number)

 int i = 0;
 int finished_stoken = 0;
 int stoken_start;
// char stoken_word [SOURCE_TEXT_LINE_LENGTH];
// int j;

 int preprocessor_line = 0; // if 1, this forces anything on the line (other than a comment) to be preprocessor colour

 se->comment_line [src_line] = 0; // may be set to 1 later

 if (in_a_comment) // if already in a multi-line comment, mark this line as a comment as well (until */ is found)
 {
  se->comment_line [src_line] = 1;
  i = mark_as_long_comment(se, src_line, 0); // mark_as_long_comment returns the position in the source line in which the comment ends, or -1 if it doesn't end on this line
  if (i == -1)
   return 1;
  if (se->text [src_line] [i] == '\0')
   return 0; // comment must have finished immediately before end of line
  in_a_comment = 0;
 }
  else // no need to check for preprocessor directives if line is in a comment
  {
   if (se->text [src_line] [0] == '#')
    preprocessor_line = 1;
  }


 while(TRUE)
 {
// now read in the next token.
// determine the token type from the first character:
  do
  {
   read_char = se->text [src_line] [i];
   char_type = get_source_char_type(read_char);
   i++;
  } while (char_type == SCHAR_SPACE);

  switch(char_type)
  {
   case SCHAR_NULL:
   case SCHAR_ERROR:
   default:
    return in_a_comment; // finished. Ignore errors.
   case SCHAR_LETTER:
    stoken_type = STOKEN_TYPE_WORD; break;
   case SCHAR_NUMBER:
    stoken_type = STOKEN_TYPE_NUMBER; break;
   case SCHAR_OPERATOR:
    if (read_char == '/')
    {
     if (se->text [src_line] [i] == '/') // should be able to assume that the next character is within bounds (may be null terminator)
     {
// this code matches the way the compiler ignores */ if within a // comment (which I don't think is standard. oh well)
      add_src_highlight(se, src_line, i - 1, strlen(se->text [src_line]), STOKEN_TYPE_COMMENT);
      return in_a_comment; // should this be zero?
     }
     if (se->text [src_line] [i] == '*') // now check for beginning of long comment
     {
      i = mark_as_long_comment(se, src_line, i - 1);
      if (i == -1)
       return 1; // comment continues past this line
      in_a_comment = 0;  // comment must have finished on this line; now read the next stoken (after the end of the comment)
      continue;
     }
    }
    stoken_type = STOKEN_TYPE_OPERATOR; break;
   case SCHAR_QUOTE:
    stoken_type = STOKEN_TYPE_STRING; break;
   case SCHAR_PUNCTUATION:
    stoken_type = STOKEN_TYPE_PUNCTUATION; break;
  }

  stoken_start = i - 1;

// now read from the source line until we find a character that can't form part of this stoken (or that is of a different type)
  while(TRUE)
  {
   read_char = se->text [src_line] [i];
   char_type = get_source_char_type(read_char);
   finished_stoken = 0;
   if (stoken_type != STOKEN_TYPE_STRING)
   {
    switch(char_type)
    {
     case SCHAR_NULL:
     case SCHAR_ERROR: // not sure about this - if an unrecognised character is found, syntax highlighting is broken for the rest of the line
     case SCHAR_SPACE:
      finished_stoken = 1;
      break;
     case SCHAR_LETTER:
      if (stoken_type != STOKEN_TYPE_WORD
							&& stoken_type != STOKEN_TYPE_NUMBER)
       finished_stoken = 1;
      break;
     case SCHAR_NUMBER:
      if (stoken_type != STOKEN_TYPE_WORD
       && stoken_type != STOKEN_TYPE_NUMBER)
        finished_stoken = 1;
      break;
     case SCHAR_OPERATOR:
      if (stoken_type != STOKEN_TYPE_OPERATOR)
       finished_stoken = 1;
      break;
     case SCHAR_PUNCTUATION:
      if (stoken_type != STOKEN_TYPE_PUNCTUATION)
       finished_stoken = 1;
      break;
     case SCHAR_QUOTE:
      finished_stoken = 1;
      break;
     }
   }
    else // in a string, so treat most characters as part of the string
    {
     switch(char_type)
     {
      case SCHAR_NULL:
      case SCHAR_ERROR: // not sure about this - if an unrecognised character is found, syntax highlighting is broken for the rest of the line
       finished_stoken = 1;
       break;
      case SCHAR_QUOTE:
       i++; // to read in closing quote
       finished_stoken = 1;
       break;
      }
    }

   if (finished_stoken)
   {
// now go back to the start of the current stoken and colour it appropriately:
/*
    if (stoken_type == STOKEN_TYPE_WORD)
    {
// if stoken is a word, need to check whether it's a keyword:
     for (j = stoken_start; j < i; j ++)
     {
      stoken_word [j - stoken_start] = se->text [src_line] [j];
     }
     stoken_word [j - stoken_start] = '\0';



//     stoken_type = check_word_type(stoken_word);


// FIX for highlighting different kinds of words!!!

    }*/
    if (preprocessor_line)
     stoken_type = STOKEN_TYPE_PREPROCESSOR;
    add_src_highlight(se, src_line, stoken_start, i, stoken_type);
    break;
   }

   i++;
  };

 };

}

// returns the position in the source line immediately after the end of the comment (i.e. after the closing */)
// returns -1 if the comment doesn't end on this line
// src_line is position in text array, not line_index array
static int mark_as_long_comment(struct source_edit_struct* se, int src_line, int comment_start)
{

 int i = comment_start;

 while(TRUE)
 {
  if (se->text [src_line] [i] == '\0')
   return -1;
  if (se->text [src_line] [i] == '*'
   && se->text [src_line] [i + 1] == '/')
  {
   se->source_colour [src_line] [i] = STOKEN_TYPE_COMMENT;
   se->source_colour [src_line] [i + 1] = STOKEN_TYPE_COMMENT;
   return i + 2; // note that i + 2 may be the null terminator
  }
  se->source_colour [src_line] [i] = STOKEN_TYPE_COMMENT;
  i++;
 };

}


int get_source_char_type(char read_source)
{

 if (read_source == '\0')
  return SCHAR_NULL;

 if ((read_source >= 'a' && read_source <= 'z')
  || (read_source >= 'A' && read_source <= 'Z')
  || read_source == '_')
   return SCHAR_LETTER;

 if (read_source >= '0' && read_source <= '9')
   return SCHAR_NUMBER;

 switch(read_source)
 {
  case '(':
  case ')':
  case '{':
  case '}':
  case '[':
  case ']':
  case '\'':
   return SCHAR_PUNCTUATION;

  case ';':
  case ':':
  case ',':
  case '.':
  case '~':
  case '+':
  case '-':
  case '*':
  case '/':
  case '=':
  case '<':
  case '>':
  case '&':
  case '|':
  case '^':
  case '%':
  case '!':
// the following aren't operators, but for now we'll treat them as operators for this purpose:
  case '\\':
  case '@':
  case '#':
  case '$':
  case '?':
  case '`':
   return SCHAR_OPERATOR;

  case '"':
   return SCHAR_QUOTE;

  case ' ':
   return SCHAR_SPACE;

  default: return SCHAR_ERROR; // invalid char

 }

 return SCHAR_ERROR; // invalid char

}



// src_line is position in text array, not line_index array
static void add_src_highlight(struct source_edit_struct* se, int src_line, int stoken_pos, int stoken_end, int stoken_type)
{


 while(stoken_pos <= stoken_end)
 {
  se->source_colour [src_line] [stoken_pos] = stoken_type;
  stoken_pos ++;
 };


}


// this function is called from game_loop in game.c and also the story mode interface in h_interface.c
// It's called even if the editor panel is closed, for some reason
void run_editor(void)
{

//  if (game.keyboard_capture == INPUT_EDITOR)
  editor_input();

  editor.cursor_flash --;
  if (editor.cursor_flash <= 0)
   editor.cursor_flash = CURSOR_FLASH_MAX;

//  draw_edit_bmp();

/*
 al_flush_event_queue(event_queue);
 ALLEGRO_EVENT ev;

 while(TRUE)
 {

  editor_input();
  draw_edit_bmp();

  al_wait_for_event(event_queue, &ev);
  al_flush_event_queue(fps_queue);

 }*/

}

/*

I think I've been taking the wrong approach with the editor.
It would be better to have the editor available from the game anytime (including during game setup)
Put a little icon up at the top right of the screen (allow client program to turn it off) - clicking on it pulls the editor over the right half of the screen (and re-minimises the editor when finished).
 - this should probably resize the game screen, and use a flag of some kind to tell the client program that it's been resized.
With 120 character limit, the editor will take up 120*6 = 720, which leaves some space for the game
 - maybe allow the client to force the editor to use the full screen and suspend the game?
There will be a variable in game_struct which determines which side captures input.
 - switch between them by clicking on one side or the other.



Also, pausing. Should be two types:
 1. pause - activated by the client program, which will keep running (to allow selection etc)
 2. suspend - activated by the system; client program stops running as well.

*/


static void editor_input(void)
{

 struct source_edit_struct* se = get_current_source_edit();
// NOTE: se may be NULL!

// if (inter.panel_input_capture == PANEL_EDITOR // currently I think this is always true (because I haven't worked out when the editor should be open but not accepting keyboard input)
//		&&	panel[PANEL_EDITOR].open)
		if (panel[PANEL_EDITOR].open)
   editor_input_keys();

// overwindow intercepts all mouse input:
 if (editor.overwindow_type != OVERWINDOW_TYPE_NONE)
 {
  overwindow_input();
  return;
 }

 int lm_button_status = ex_control.mb_press [0]; // this value can be changed if the button status is captured (e.g. because user clicked on a submenu)

 editor.submenu_name_highlight = -1; // is updated below if needed

 int mouse_x = control.mouse_x_screen_pixels - panel[PANEL_EDITOR].x1;
 int mouse_y = control.mouse_y_screen_pixels - panel[PANEL_EDITOR].y1;

 int force_basic_mouse_cursor = 0;

#define USING_EDITOR_INPUT

#ifdef USING_EDITOR_INPUT

/*

Plan for editor:

It's not worth reimplementing the interface using the panel system.

Instead I just need to use panels as a wrapper around the existing editor interface.

source_edits/tabs: Instead of the tab system:
 - each template will have its own source_edit_struct
 - there will be no other tabs
 - opening a file opens it into the next available template
  - if opened from the design panel, automatically loads the interface into the designer
  - if opened from the editor panel, maybe doesn't? Actually there's probably no point in not doing so.


*/

 if (control.mouse_panel == PANEL_LOG
		&& ex_control.mb_press [0] == BUTTON_JUST_PRESSED)
	{
		mouse_click_on_log_window();
		return;
	}

// check for the mouse pointer being in the game window:
 if (control.mouse_panel != PANEL_EDITOR)
 {
  return;
 }

// number of lines scrolled by moving the mousewheel:
#define EDITOR_MOUSEWHEEL_SPEED 8

// check for mousewheel movement:
 if (se != NULL)
 {

  if (completion.list_size > 0
			&& ex_control.mouse_x_pixels >= completion.box_x
			&& ex_control.mouse_x_pixels <= completion.box_x2
			&& ex_control.mouse_y_pixels >= completion.box_y
			&& ex_control.mouse_y_pixels <= completion.box_y2)
		{
    if (ex_control.mousewheel_change == 1)
					scroll_completion_box_down(4);
    if (ex_control.mousewheel_change == -1)
					scroll_completion_box_up(4);
		}
		 else
			{
    if (ex_control.mousewheel_change == 1)
    {
     se->window_line += EDITOR_MOUSEWHEEL_SPEED;
     if (se->window_line >= SOURCE_TEXT_LINES)
      se->window_line = SOURCE_TEXT_LINES - 1;
     slider_moved_to_value(&slider[SLIDER_EDITOR_SCROLLBAR_V], se->window_line);
    }
    if (ex_control.mousewheel_change == -1)
    {
     se->window_line -= EDITOR_MOUSEWHEEL_SPEED;
     if (se->window_line < 0)
      se->window_line = 0;
     slider_moved_to_value(&slider[SLIDER_EDITOR_SCROLLBAR_V], se->window_line);
    }
			}
 }


// now check for the user interacting with the scrollbars
// if (editor.current_tab != -1
//  && editor.tab_type [editor.current_tab] == TAB_TYPE_SOURCE) // no scrollbars if file is binary
 if (editor.current_source_edit_index != -1)
 {
// editor scrollbar
//  run_slider(SLIDER_EDITOR_SCROLLBAR_V);
  //run_slider(SLIDER_EDITOR_SCROLLBAR_H);
//  editor.source_edit[editor.current_source_edit].window_line = editor.scrollbar_v.value;
 }
// message log scrollbar:
// run_slider(&mlog.scrollbar_v, 0, 0);

// int mouse_x = ex_control.mouse_x_pixels - panel[PANEL_EDITOR].x1;
// int mouse_y = ex_control.mouse_y_pixels - panel[PANEL_EDITOR].y1;

// check for user clicking on submenu:
 if (editor.submenu_open != -1)
 {
// check for mouse being inside submenu box:
  if (mouse_x >= editor.submenu_x
   && mouse_x <= editor.submenu_x + editor.submenu_w
   && mouse_y >= editor.submenu_y
   && mouse_y <= editor.submenu_y + editor.submenu_h)
  {
   force_basic_mouse_cursor = 1;
   editor.submenu_highlight = (mouse_y - editor.submenu_y) / SUBMENU_LINE_HEIGHT;
   if (editor.submenu_highlight < 0
    || editor.submenu_highlight >= submenu[editor.submenu_open].lines)
     editor.submenu_highlight = -1;
// choose a submenu item by releasing the mouse button:
   if (lm_button_status == BUTTON_JUST_RELEASED)
   {
    submenu_operation(editor.submenu_open, editor.submenu_highlight);
    close_submenu();
    completion.list_size = 0;
   }
/*   if (ex_control.mb_press [1] == BUTTON_JUST_PRESSED
				&& editor.submenu_highlight != -1)
			{
				print_help(submenu[editor.submenu_open].line[editor.submenu_highlight].help_type);
			 completion.list_size = 0;
			}*/

   lm_button_status = BUTTON_NOT_PRESSED; // captures mouse button press if mouse over submenu
  }
   else
   {
    editor.submenu_highlight = -1;
   }
// remember to add code to close submenu if user clicks elsewhere
 }

 int mouse_in_completion_box = 0;

  if (completion.list_size > 0
			&& ex_control.mouse_x_pixels >= completion.box_x
			&& ex_control.mouse_x_pixels <= completion.box_x2
			&& ex_control.mouse_y_pixels >= completion.box_y
			&& ex_control.mouse_y_pixels <= completion.box_y2)
		{
			mouse_in_completion_box = 1;
   force_basic_mouse_cursor = 1;
  	completion.select_line = completion.window_pos + ((ex_control.mouse_y_pixels - completion.box_y - COMPLETION_BOX_LINE_Y_OFFSET) / COMPLETION_BOX_LINE_H);
  	if (completion.select_line < 0)
				completion.select_line = 0;
  	if (completion.select_line >= completion.list_size)
				completion.select_line = completion.list_size - 1;
		}

 editor.mouse_cursor_line = -1;
// editor.mouse_cursor_pos = -1; just use line

// now check for the user clicking in the editing window:
 if (lm_button_status == BUTTON_JUST_PRESSED)
 {
  if (completion.list_size > 0
			&& mouse_in_completion_box == 1)
		{
   close_submenu();
			click_in_completion_box(se, ex_control.mouse_x_pixels - completion.box_x, ex_control.mouse_y_pixels - completion.box_y);
		}
		 else
			{
				completion.list_size = 0;
    if (mouse_x >= EDIT_WINDOW_X
     && mouse_y >= EDIT_WINDOW_Y
     && mouse_x <= EDIT_WINDOW_X + editor.edit_window_w
     && mouse_y <= EDIT_WINDOW_Y + editor.edit_window_h)
    {
     ex_control.mouse_cursor_type = MOUSE_CURSOR_TEXT;
     close_submenu();
     click_in_edit_window(mouse_x - EDIT_WINDOW_X, mouse_y - EDIT_WINDOW_Y);
    }
			}
 }
  else
		{
    if (mouse_x >= EDIT_WINDOW_X
     && mouse_y >= EDIT_WINDOW_Y
     && mouse_x <= EDIT_WINDOW_X + editor.edit_window_w
     && mouse_y <= EDIT_WINDOW_Y + editor.edit_window_h
     && se != NULL)
    {
    	get_cursor_position_from_mouse(se, mouse_x - EDIT_WINDOW_X, mouse_y - EDIT_WINDOW_Y, &editor.mouse_cursor_line, &editor.mouse_cursor_pos);
     ex_control.mouse_cursor_type = MOUSE_CURSOR_TEXT;
    	if (ex_control.mb_press [1] == BUTTON_JUST_PRESSED)
					{
						editor_help_click(se, editor.mouse_cursor_line, editor.mouse_cursor_pos);
					}
    }
		}

// menu bar (File Edit Compile etc.)
    if (mouse_y >= EMENU_BAR_Y
     && mouse_y <= EMENU_BAR_Y + EMENU_BAR_H)
    {
     int submenu_chosen = (mouse_x - EMENU_BAR_X) / EMENU_BAR_NAME_WIDTH;
     if (lm_button_status == BUTTON_JUST_PRESSED)
     {
      if (submenu_chosen >= 0 && submenu_chosen < SUBMENUS)
      {
       open_submenu(submenu_chosen);
      }
     }
      else
      {
       if (submenu_chosen >= 0 && submenu_chosen < SUBMENUS)
       {
        editor.submenu_name_highlight = submenu_chosen;
        if (editor.submenu_open != -1
         && editor.submenu_open != submenu_chosen)
          open_submenu(submenu_chosen);
       }
      }
    }


//   editor.tab_highlight = -1;
/*
// menu bar (File Edit Compile etc.)
    if (mouse_y >= SOURCE_TAB_Y
     && mouse_y <= SOURCE_TAB_Y + SOURCE_TAB_H)
    {
     int tab_chosen = (mouse_x - SOURCE_TAB_X) / SOURCE_TAB_W;
     if (lm_button_status == BUTTON_JUST_PRESSED)
     {
      if (tab_chosen >= 0
       && tab_chosen < ESOURCES
       && editor.tab_index [tab_chosen] != -1)
      {
       open_tab(tab_chosen);
      }
     }
      else
      {
       if (tab_chosen >= 0 && tab_chosen < ESOURCES)
       {
        if (editor.tab_index [tab_chosen] != -1)
         editor.tab_highlight = tab_chosen;
       }
      }
    }
*/
#endif

 if (editor.selecting == 1
  && lm_button_status == BUTTON_HELD
  && mouse_x >= EDIT_WINDOW_X
  && mouse_y >= EDIT_WINDOW_Y
  && mouse_x <= EDIT_WINDOW_X + editor.edit_window_w
  && mouse_y <= EDIT_WINDOW_Y + editor.edit_window_h)
 {
  select_text(mouse_x - EDIT_WINDOW_X, mouse_y - EDIT_WINDOW_Y);
 }

 if (lm_button_status <= 0)
 {
  // if mouse button not being pressed, stop selecting:
   editor.selecting = 0; // but doesn't remove the selection values from the source_edit_struct
 }

 if (force_basic_mouse_cursor)
		ex_control.mouse_cursor_type = MOUSE_CURSOR_BASIC;

}


static void overwindow_input(void)
{
//#ifdef USING_OVERWINDOW
 int lm_button_status = ex_control.mb_press [0];

 int mouse_x = ex_control.mouse_x_pixels - panel[PANEL_EDITOR].x1;
 int mouse_y = ex_control.mouse_y_pixels;// - editor.panel_y;
 int i;

 struct source_edit_struct* se;

 if (lm_button_status == BUTTON_JUST_PRESSED
  && mouse_x >= editor.overwindow_x
  && mouse_x <= editor.overwindow_x + editor.overwindow_w
  && mouse_y >= editor.overwindow_y
  && mouse_y <= editor.overwindow_y + editor.overwindow_h)
 {
  for (i = 0; i < editor.overwindow_buttons; i ++)
  {
   if (mouse_x >= editor.overwindow_button_x [i]
    && mouse_x <= editor.overwindow_button_x [i] + OVERWINDOW_BUTTON_W
    && mouse_y >= editor.overwindow_button_y [i]
    && mouse_y <= editor.overwindow_button_y [i] + OVERWINDOW_BUTTON_H)
   {
    switch(editor.overwindow_button_type [i])
    {
/*     case OVERWINDOW_BUTTON_TYPE_NO:
      editor.overwindow_type = OVERWINDOW_TYPE_NONE;
      break;
     case OVERWINDOW_BUTTON_TYPE_CLOSE_TAB:
      close_source_tab(editor.current_tab, 1);
      editor.overwindow_type = OVERWINDOW_TYPE_NONE;
      break;*/
     case OVERWINDOW_BUTTON_TYPE_FIND:
      editor.overwindow_type = OVERWINDOW_TYPE_NONE;
      se = get_current_source_edit();
      if (se != NULL
       && se->type == SOURCE_EDIT_TYPE_SOURCE)
      {
       find_next();
       window_find_cursor(se);
      }
// should match code for pressing enter just below

      return;

     case OVERWINDOW_BUTTON_TYPE_CANCEL_FIND:
      editor.overwindow_type = OVERWINDOW_TYPE_NONE;
      break;

    }
   }
  }

 }

 int input_result;

 switch(editor.overwindow_type)
 {
  case OVERWINDOW_TYPE_FIND:
  	if (!control.editor_captures_input)
				break;
   input_result = accept_text_box_input(TEXT_BOX_EDITOR_FIND);
   if (input_result == 1) // enter pressed
   {
    editor.overwindow_type = OVERWINDOW_TYPE_NONE;
    se = get_current_source_edit();
    if (se != NULL
     && se->type == SOURCE_EDIT_TYPE_SOURCE)
    {
     find_next();
     window_find_cursor(se);
    }
    if (ex_control.special_key_press [SPECIAL_KEY_ENTER] == BUTTON_JUST_PRESSED)
    {
     ex_control.special_key_press [SPECIAL_KEY_ENTER] = BUTTON_HELD;
     editor.special_key_being_pressed = SPECIAL_KEY_ENTER;
    }
/*    if (ex_control.key_press [ALLEGRO_KEY_PAD_ENTER] == BUTTON_JUST_PRESSED)
    {
     ex_control.key_press [ALLEGRO_KEY_PAD_ENTER] = BUTTON_HELD;
     editor.key_being_pressed = ALLEGRO_KEY_PAD_ENTER;
    }*/
    editor.key_delay = 30;
// should match code for button press just above
   }
   break;
 }

//#endif

}


void open_overwindow(int ow_type)
{

//#ifdef USING_OVERWINDOW

 editor.overwindow_type = ow_type;
 editor.overwindow_w = scaleUI_x(FONT_BASIC,200);
 editor.overwindow_h = scaleUI_y(FONT_BASIC,100);
 editor.overwindow_x = (panel[PANEL_EDITOR].w / 2) - (editor.overwindow_w / 2);
 editor.overwindow_y = (panel[PANEL_EDITOR].h / 2) - (editor.overwindow_h / 2);

 switch(ow_type)
 {
  case OVERWINDOW_TYPE_CLOSE:
   editor.overwindow_buttons = 2;
   editor.overwindow_button_type [0] = OVERWINDOW_BUTTON_TYPE_CLOSE_TAB;
   editor.overwindow_button_type [1] = OVERWINDOW_BUTTON_TYPE_NO;
   editor.overwindow_button_x [0] = editor.overwindow_x + 20;
   editor.overwindow_button_y [0] = editor.overwindow_y + editor.overwindow_h - 20 - OVERWINDOW_BUTTON_H;
   editor.overwindow_button_x [1] = editor.overwindow_x + editor.overwindow_w - 30 - OVERWINDOW_BUTTON_W;
   editor.overwindow_button_y [1] = editor.overwindow_y + editor.overwindow_h - 20 - OVERWINDOW_BUTTON_H;
   break;
  case OVERWINDOW_TYPE_FIND:
   editor.overwindow_buttons = 2;
   editor.overwindow_button_type [0] = OVERWINDOW_BUTTON_TYPE_FIND;
   editor.overwindow_button_type [1] = OVERWINDOW_BUTTON_TYPE_CANCEL_FIND;
   editor.overwindow_button_x [0] = editor.overwindow_x + 20;
   editor.overwindow_button_y [0] = editor.overwindow_y + editor.overwindow_h - 20 - OVERWINDOW_BUTTON_H;
   editor.overwindow_button_x [1] = editor.overwindow_x + editor.overwindow_w - 30 - OVERWINDOW_BUTTON_W;
   editor.overwindow_button_y [1] = editor.overwindow_y + editor.overwindow_h - 20 - OVERWINDOW_BUTTON_H;
   start_text_input_box(TEXT_BOX_EDITOR_FIND, editor.search_string, SEARCH_STRING_LENGTH);
   break;


 }
//#endif
}

//static void close_overwindow(void)
//{
// editor.overwindow_type = OVERWINDOW_TYPE_NONE;
//}

// assumes sm is valid
static void open_submenu(int sm)
{

  editor.submenu_open = sm;
  editor.submenu_highlight = -1;
  editor.submenu_x = EMENU_BAR_X + (EMENU_BAR_NAME_WIDTH * sm);
  editor.submenu_y = EMENU_BAR_Y + EMENU_BAR_H;
  editor.submenu_w = SUBMENU_WIDTH;
  editor.submenu_h = submenu[sm].lines * SUBMENU_LINE_HEIGHT;
}

// can't assume this will be called any time a submenu is closed (e.g. submenu_open alone is used to switch between submenus)
static void close_submenu(void)
{

 editor.submenu_open = -1;

}

// Note that this can be called by keyboard shortcuts as well as by clicking on the submenu
static void submenu_operation(int sm, int line)
{

 editor.key_delay = KEY_DELAY1;
 editor.cursor_flash = CURSOR_FLASH_MAX;
// these are mostly for keyboard shortcuts, but it can't hurt to do this anyway


 struct source_edit_struct* se;

 switch(sm)
 {
  case SUBMENU_FILE:
   switch(line)
   {
    //case SUBMENU_FILE_NEW:
//     new_empty_source_tab(); // may fail, but just ignore failure (an error will have been written to the log)
//     break;
    case SUBMENU_FILE_OPEN:
#ifndef DEBUG_MODE
    	if (game.type == GAME_TYPE_MISSION
						&& dwindow.templ->player_index == 1)
					{
      	write_line_to_log("Can't load a file into an enemy template during a mission!", MLOG_COL_ERROR);
      	break;
					}
#endif
    	if (dwindow.templ->locked)
					{
     	write_line_to_log("Warning: loading source file into locked template.", MLOG_COL_WARNING);
		 	  write_line_to_log("(See 'Recompiling a locked template' in the manual for more detail)", MLOG_COL_COMPILER);
					}
     open_file_into_current_source_edit(); // may fail, but just ignore failure
//					  else
//       	write_line_to_log("Can't load file into locked template.", MLOG_COL_ERROR);

/*
    	if (!dwindow.templ->locked)
      open_file_into_current_source_edit(); // may fail, but just ignore failure
					  else
       	write_line_to_log("Can't load file into locked template.", MLOG_COL_ERROR);
*/
     break;
    case SUBMENU_FILE_RELOAD:
#ifndef DEBUG_MODE
    	if (game.type == GAME_TYPE_MISSION
						&& dwindow.templ->player_index == 1)
					{
      	write_line_to_log("Can't load a file into an enemy template during a mission!", MLOG_COL_ERROR);
      	break;
					}
#endif
    	if (!dwindow.templ->active
						|| dwindow.templ->source_edit->type != SOURCE_EDIT_TYPE_SOURCE
						|| !dwindow.templ->source_edit->from_a_file
						|| dwindow.templ->source_edit->src_file_path [0] == 0)
					{
      	write_line_to_log("Source reload failed.", MLOG_COL_ERROR);
      	write_line_to_log("(Reload only works if the template contains source code from a file.)", MLOG_COL_ERROR);
      	break;
					}
// the 0 at the end of the load_source_file_into_template_without_compiling call means that the template is already open, so it doesn't need to be opened or reset.
 				if (load_source_file_into_template_without_compiling(dwindow.templ->source_edit->src_file_path, dwindow.templ->player_index, dwindow.templ->template_index, 0) == 1)
					{
     	write_line_to_log("Source reloaded.", MLOG_COL_TEMPLATE);
     	if (dwindow.templ->locked)
	 				{
      	write_line_to_log("Warning: source file loaded into locked template.", MLOG_COL_WARNING);
		  	  write_line_to_log("(See 'Recompiling a locked template' in the manual for more detail)", MLOG_COL_COMPILER);
				 	}
					}
					  else
       	write_line_to_log("Source reload failed.", MLOG_COL_ERROR);
					break;
    case SUBMENU_FILE_SAVE:
     save_current_file();
     break;
    case SUBMENU_FILE_SAVE_ALL:
     save_all_files();
     break;
    case SUBMENU_FILE_SAVE_AS:
     save_as();
     break;
//    case SUBMENU_FILE_CLOSE:
//     close_source_tab(editor.current_tab, 0);
//     break;
   }
   break;
  case SUBMENU_EDIT:
   switch(line)
   {
// if any edit operations are changed, may need to update the code for keyboard shortcuts too (in editor_input_keys())
    case SUBMENU_EDIT_UNDO:
//    	if (!dwindow.templ->locked)
      call_undo();
      reset_cursor_after_action();
//					  else
//						  template_locked_editor_message();
     break;
    case SUBMENU_EDIT_REDO:
//    	if (!dwindow.templ->locked)
      call_redo();
      reset_cursor_after_action();
//					  else
//						  template_locked_editor_message();
     break;
    case SUBMENU_EDIT_COPY:
// fine to copy text from locked template
     copy_selection();
     break;

    case SUBMENU_EDIT_CUT:
//    	if (!dwindow.templ->locked)
					{
      copy_selection();
      delete_selection(); // ignore return value
					}
//					 else
//						 template_locked_editor_message();
     break;
    case SUBMENU_EDIT_PASTE:
//    	if (!dwindow.templ->locked)
      paste_clipboard();
      reset_cursor_after_action();
//					  else
//						  template_locked_editor_message();
     break;
    case SUBMENU_EDIT_SELECT_ALL:
    	{
      se = get_current_source_edit();
      if (se != NULL
					 	&& se->active == 1
       && se->type == SOURCE_EDIT_TYPE_SOURCE)
      {
      	se->cursor_line = 0;
      	se->cursor_pos = 0;
      	se->cursor_base = 0;
//       editor.selecting = 1;
       se->select_fix_line = 0;
       se->select_fix_pos = 0;
       se->select_free_line = 0;
       se->select_free_pos = 0;
       if (move_cursor_to_end_of_source(se))
							{
								consider_selecting_to_cursor(se, 0, 0, 0);
							}
      }


    	}
					break;
   }
   break;
  case SUBMENU_SEARCH:
   switch(line)
   {
    case SUBMENU_SEARCH_FIND:
     open_overwindow(OVERWINDOW_TYPE_FIND);
     break;
    case SUBMENU_SEARCH_FIND_NEXT:
     se = get_current_source_edit();
     if (se != NULL
						&& se->active == 1
      && se->type == SOURCE_EDIT_TYPE_SOURCE)
     {
      find_next();
      window_find_cursor(se);
     }
     break;
   }
   break;
  case SUBMENU_COMPILE:
   switch(line)
   {

    case SUBMENU_COMPILE_TEST:
     reset_log();
    	if (dwindow.templ->locked)
					{
		 	  write_line_to_log("Template locked - process design not updated.", MLOG_COL_COMPILER);
		 	  write_line_to_log("(See 'Recompiling a locked template' in the manual for more detail)", MLOG_COL_COMPILER);
					}
//    	if (!dwindow.templ->locked)
					 compile_current_source_edit(COMPILE_MODE_TEST);
//					  else
//  				  template_locked_editor_message();
    	break;
    case SUBMENU_COMPILE_COMPILE:
      reset_log();
      if (dwindow.templ->player_index == 1
							&& game.type == GAME_TYPE_MISSION)
     	{
#ifndef DEBUG_MODE
		write_line_to_log("You can't recompile your opponent's templates in story mode!", MLOG_COL_ERROR);
		return;
#else
		write_line_to_log("DEBUG MODE: Allowing recompilation of mission opponent template.", MLOG_COL_WARNING);
#endif
     	}
    	 if (dwindow.templ->locked)
					 {
		 	   write_line_to_log("Template locked - process design not updated.", MLOG_COL_COMPILER);
 		 	  write_line_to_log("(See 'Recompiling a locked template' in the manual for more detail)", MLOG_COL_COMPILER);
					 }
//    	if (!dwindow.templ->locked)
					 compile_current_source_edit(COMPILE_MODE_BUILD);
//					  else
//  				  template_locked_editor_message();
    	break;
    case SUBMENU_COMPILE_COMPILE_LOCK:
     reset_log();
     if (dwindow.templ->player_index == 1
						&& game.type == GAME_TYPE_MISSION)
	    {
#ifndef DEBUG_MODE
		write_line_to_log("You can't recompile your opponent's templates in story mode!", MLOG_COL_ERROR);
		return;
#else
		write_line_to_log("DEBUG MODE: Allowing recompilation of mission opponent template.", MLOG_COL_WARNING);
#endif
    	}
    	if (dwindow.templ->locked)
					{
		 	  write_line_to_log("Template already locked - process design not updated.", MLOG_COL_COMPILER);
		 	  write_line_to_log("(See 'Recompiling a locked template' in the manual for more detail)", MLOG_COL_COMPILER);
					}
/*    	if (dwindow.templ->locked)
					{
		 	  write_line_to_log("This template is already locked.", MLOG_COL_ERROR);
				  break;
					}*/
					lock_template(dwindow.templ); // this compiles the source
    	break;

   }
   break;
 }

}

/*
// assumes tab is valid
void open_tab(int tab)
{

 if (editor.current_tab == tab)
  return;

 editor.current_tab = tab;

 change_tab(tab);

}*/

// Currently this function is called every time a different template is selected.
// It could be optimised by only being called if the editor panel is open, and when the editor panel is opened.
void open_template_in_editor(struct template_struct* tpl)
{

// fpr("\nopen template %i, player %i", tpl->template_index, tpl->player_index);

 int new_se_index = tpl->esource_index;//(p * TEMPLATES_PER_PLAYER) + t;

//	if (editor.current_source_edit_index == new_se_index)
//		return;

	editor.current_source_edit_index = new_se_index;

	editor_change_source_edit(new_se_index);


}

static void click_in_edit_window(int x, int y)
{

 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || !se->active)
  return;

// check for click in code completion box here:

 completion.list_size = 0; // remove code completion box

 int old_line = se->cursor_line;
 int old_pos = se->cursor_pos;

 get_cursor_position_from_mouse(se, x, y, &se->cursor_line, &se->cursor_pos);
 se->cursor_base = se->cursor_pos;
 editor.cursor_flash = CURSOR_FLASH_MAX;

 window_find_cursor(se);

// if (editor.selecting)
// if (se->selected)
  consider_selecting_to_cursor(se, old_line, old_pos, 1);

  if (!se->selected)
			{
    editor.selecting = 1;
    se->select_fix_line = se->cursor_line;
    se->select_fix_pos = se->cursor_pos;
    se->select_free_line = se->cursor_line;
    se->select_free_pos = se->cursor_pos;
			}
/*   else
			{
    editor.selecting = 1;
    se->select_fix_line = se->cursor_line;
    se->select_fix_pos = se->cursor_pos;
    se->select_free_line = se->cursor_line;
    se->select_free_pos = se->cursor_pos;
			}*/

}




static void select_text(int x, int y)
{

 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || !se->active)
  return;

 get_cursor_position_from_mouse(se, x, y, &se->cursor_line, &se->cursor_pos);
 se->cursor_base = se->cursor_pos;

// check whether the mouse has been moved since the selecting started)
 if (se->select_fix_line != se->cursor_line
  || se->select_fix_pos != se->cursor_pos
  || se->select_free_line != se->cursor_line
  || se->select_free_pos != se->cursor_pos)
 {
  se->selected = 1;
  se->select_free_line = se->cursor_line;
  se->select_free_pos = se->cursor_pos;
 }
  else
   se->selected = 0;


}

static void get_cursor_position_from_mouse(struct source_edit_struct* se, int x, int y, int* mouse_cursor_line, int* mouse_cursor_pos)
{

// *mouse_cursor_line = se->window_line + ((y + 3 + EDIT_LINE_OFFSET) / EDIT_LINE_H); // 3 is fine-tuning
 *mouse_cursor_line = se->window_line + ((y + 0 + EDIT_LINE_OFFSET) / EDIT_LINE_H); // 3 is fine-tuning

 if (*mouse_cursor_line >= SOURCE_TEXT_LINES)
  *mouse_cursor_line = SOURCE_TEXT_LINES - 1;
 if (*mouse_cursor_line < 0)
  *mouse_cursor_line = 0;

 *mouse_cursor_pos = se->window_pos + ((x - SOURCE_WINDOW_MARGIN) / editor.text_width);
// *mouse_cursor_pos = se->window_pos - 1 + ((x + 8) / editor.text_width);

 if (*mouse_cursor_pos < 0)
		*mouse_cursor_pos = 0;

 if (*mouse_cursor_pos > strlen(se->text [se->line_index [*mouse_cursor_line]]))
  *mouse_cursor_pos = strlen(se->text [se->line_index [*mouse_cursor_line]]);

// se->cursor_base = se->cursor_pos;

}

// user clicked in the code completion box
// x and y are offsets from the top left of the box
static void	click_in_completion_box(struct source_edit_struct* se, int x, int y)
{

	int line_clicked;

	line_clicked = completion.window_pos + ((y - COMPLETION_BOX_LINE_Y_OFFSET) / COMPLETION_BOX_LINE_H);

	complete_code(se, line_clicked); // complete_code will bounds-check line_clicked and check se for NULL

}


// can call this anytime the editor window should be open (even if it's already open)
void open_editor(void)
{

// settings.edit_window = EDIT_WINDOW_EDITOR;
 inter.panel_input_capture = PANEL_EDITOR;

}

void close_editor(void)
{

 editor.submenu_open = -1;

// settings.edit_window = EDIT_WINDOW_CLOSED;
 inter.panel_input_capture = PANEL_EDITOR;

}


enum
{
// skipping left or right skips to the next character that's a different kind of "word":
SKIP_WORD_TYPE_SPACE, // if on space, skips to next non-space character
SKIP_WORD_TYPE_WORD, // letter or numbers
SKIP_WORD_TYPE_OTHER, // anything else - punctuation or operator
SKIP_WORD_TYPE_ERROR // some kind of error found	- probably don't try to skip

};


/*

Need to revise this as it doesn't work very well.

basically:
 - if multiple keys are pressed at the same time, all should be registered.
 - then the last one to be registered counts as the key being held down
 - as long as that key is being held and no new key is pressed, that key keeps being added subject to key delay
  - however, if another key is pressed, it is immediately set as the key being held down and replaces the previous key


*/
static void editor_input_keys(void)
{

 if (!control.editor_captures_input)
		goto no_keys_accepted; // don't accept keypresses while mouse in main game panel

 if (editor.selecting)
  goto no_keys_accepted; // don't accept keypresses while the mouse is being dragged to select text.

 if (editor.overwindow_type != OVERWINDOW_TYPE_NONE)
  goto no_keys_accepted; // don't accept keypresses while overwindow open



//#define DEBUG_MODE
/*
#ifdef DEBUG_MODE
 if (ex_control.special_key_press [SPECIAL_KEY_F6] == BUTTON_JUST_PRESSED)
	{
		if (editor.debug_keys == 0)
			editor.debug_keys = 1;
 		 else
					editor.debug_keys = 0;
	}

	if (editor.debug_keys)
		fpr("\n - ed kd %i ign %i bp %i alr %i:", editor.key_delay, editor.ignore_previous_key_pressed, editor.special_key_being_pressed, editor.already_pressed_cursor_key_etc);


#endif
*/
 int i, j, found_word;
 struct source_edit_struct* se;

 if (ex_control.unichar_input != 0)
	{


#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" unichar %i", ex_control.unichar_input);
#endif
		editor_keypress_unichar(ex_control.unichar_input);

  if (ex_control.unichar_input == 6) // ctrl-f (I hope)
  {
  	submenu_operation(SUBMENU_SEARCH, SUBMENU_SEARCH_FIND);
// if changed, check code for Edit submenu in submenu_operation
//     open_overwindow(OVERWINDOW_TYPE_FIND);
//     editor.key_delay = 15;
     return;
  }

	}

 if (ex_control.keys_pressed == 0)
 {

no_keys_accepted:

#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" nokeys");
#endif

  editor.key_delay = 0;
  editor.special_key_being_pressed = -1;
  editor.ignore_previous_key_pressed = -1;
  return;
 }

  editor.already_pressed_cursor_key_etc = 0;

  close_submenu();

// test for modified keys here:
  if (ex_control.special_key_press [SPECIAL_KEY_CTRL] > 0)
  {

#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" ctrl");
#endif

   if (editor.key_delay > 0)
   {
// we need special rules for key delay here because the control key may be kept down while another key is being pressed multiple times:
					if (ex_control.keys_pressed == 1 // just the control key being pressed - reset key delay
					||	(ex_control.keys_pressed == 2 // just control key and a shift key (for control+shift+cursor key to do word skip while selecting)
						&& ex_control.special_key_press [SPECIAL_KEY_SHIFT] > 0))
     editor.key_delay = 0;
      else
       editor.key_delay --;
    return;
   }


   switch(ex_control.unichar_input)
   {
// edit menu
   	case 3: // ctrl-C
   		submenu_operation(SUBMENU_EDIT, SUBMENU_EDIT_COPY);
   		return;
   	case 22: // ctrl-V
   		submenu_operation(SUBMENU_EDIT, SUBMENU_EDIT_PASTE);
   		return;
   	case 24: // ctrl-X
   		submenu_operation(SUBMENU_EDIT, SUBMENU_EDIT_CUT);
   		return;
   	case 26: // ctrl-Z
   		submenu_operation(SUBMENU_EDIT, SUBMENU_EDIT_UNDO);
   		return;
   	case 25: // ctrl-Y
   		submenu_operation(SUBMENU_EDIT, SUBMENU_EDIT_REDO);
   		return;
// File menu
   	case 1: // ctrl-A
   		submenu_operation(SUBMENU_EDIT, SUBMENU_EDIT_SELECT_ALL);
   		return;
   	case 19: // ctrl-S and ctrl-shift-S
   		if (ex_control.special_key_press [SPECIAL_KEY_SHIFT])
    		submenu_operation(SUBMENU_FILE, SUBMENU_FILE_SAVE_ALL);
    		 else
   		   submenu_operation(SUBMENU_FILE, SUBMENU_FILE_SAVE);
   		return;
#ifdef RELOAD_KEYBOARD_SHORTCUT
   	case 18: // ctrl-R
   		submenu_operation(SUBMENU_FILE, SUBMENU_FILE_RELOAD);
   		return;
#endif


//   	case 15: // ctrl-O // currently not supported because of problems updating Allegro's keyboard state when context changes.
//
// al_clear_keyboard_state(display);

//   		submenu_operation(SUBMENU_FILE, SUBMENU_FILE_OPEN);
// unfortunately the window-switching involved in using native file dialogues
//  seems to confuse the keyboard routines. So we need to reset the control
//  key here:
//     ex_control.special_key_press [SPECIAL_KEY_CTRL] = BUTTON_NOT_PRESSED;
//   		return;

// Need:
//  S save
//  A save all
//  O open
//

   }

/*
   if (ex_control.unichar_input == 3) // ctrl-C
//    || ex_control.special_key_press [SPECIAL_KEY_INSERT] != 0)
//    || ex_control.key_press [ALLEGRO_KEY_PAD_0] != 0)
   {
//    if (just_pressed_ctrl_key(ALLEGRO_KEY_C)
//     || just_pressed_ctrl_key(ALLEGRO_KEY_INSERT)
//     || just_pressed_ctrl_key(ALLEGRO_KEY_PAD_0))
    {
// if changed, check code for Edit submenu in submenu_operation
#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" ctrl-c");
#endif
     copy_selection();
     editor.key_delay = KEY_DELAY1;
     editor.cursor_flash = CURSOR_FLASH_MAX;
    }
    return;
   }
//   if (ex_control.key_press [ALLEGRO_KEY_V] != 0)
   if (ex_control.unichar_input == 22) // ctrl-V
   {
#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" ctrl-v");
#endif

//    if (just_pressed_ctrl_key(ALLEGRO_KEY_V))
    {
// if changed, check code for Edit submenu in submenu_operation
//     if (!dwindow.templ->locked)
      paste_clipboard();
//       else
//								template_locked_editor_message();
     editor.key_delay = KEY_DELAY1;
     editor.cursor_flash = CURSOR_FLASH_MAX;
    }
    return;
   }
//   if (ex_control.key_press [ALLEGRO_KEY_X] != 0)
   if (ex_control.unichar_input == 24) // ctrl-X
   {
#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" ctrl-x");
#endif
//    if (just_pressed_ctrl_key(ALLEGRO_KEY_X))
    {
// if changed, check code for Edit submenu in submenu_operation
//     if (!dwindow.templ->locked)
//					{
      copy_selection();
      delete_selection(); // ignore return value
//					}
//      else
//							template_locked_editor_message();
     editor.key_delay = KEY_DELAY1;
     editor.cursor_flash = CURSOR_FLASH_MAX;
    }
    return;
   }
//   if (ex_control.key_press [ALLEGRO_KEY_Z] != 0)
   if (ex_control.unichar_input == 26) // ctrl-Z
   {
//    if (just_pressed_ctrl_key(ALLEGRO_KEY_Z))
    {
// if changed, check code for Edit submenu in submenu_operation
//     if (!dwindow.templ->locked)
      call_undo();
//       else
//							 template_locked_editor_message();
     editor.key_delay = KEY_DELAY1;
     editor.cursor_flash = CURSOR_FLASH_MAX;
     reset_cursor_after_action();
    }
    return;
   }
//   if (ex_control.key_press [ALLEGRO_KEY_Y] != 0)
   if (ex_control.unichar_input == 25) // ctrl-Y
   {
//    if (just_pressed_ctrl_key(ALLEGRO_KEY_Y))
    {
// if changed, check code for Edit submenu in submenu_operation
//     if (!dwindow.templ->locked)
      call_redo();
//       else
//							 template_locked_editor_message();
     editor.key_delay = KEY_DELAY1;
     editor.cursor_flash = CURSOR_FLASH_MAX;
    }
    return;
   }
*/
   if (ex_control.special_key_press [SPECIAL_KEY_LEFT] > 0
    && ex_control.special_key_press [SPECIAL_KEY_RIGHT] <= 0)
   {
#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" ctrl-left %i (right %i)", ex_control.special_key_press [SPECIAL_KEY_LEFT], ex_control.special_key_press [SPECIAL_KEY_RIGHT]);
#endif
    se = get_current_source_edit();
    if (se == NULL
     || !se->active)
     return;
    int old_line = se->cursor_line;
    int old_pos = se->cursor_pos;
    i = se->cursor_pos;
    j = se->cursor_line;
    int check_pos = i;
    int check_line = j;
    found_word = 0;
    int finished = 0;
    int current_word_type = -1; // indicates that word type to left of cursor not yet determined   // get_skip_word_type(se->text [se->line_index [j]] [i]);
    while(TRUE)
    {
     check_pos --;
     if (check_pos < 0 && found_word == 1) // this deals with a word that starts at the start of the line
					{
      check_pos ++;
      i = check_pos;
      j = check_line;
      finished = 1;
      break;
					}
     while (check_pos < 0) // reached start of line
     {
      if (check_line <= 0) // reached start of file
      {
       check_pos = 0;
       check_line = 0;
       i = check_pos;
       j = check_line;
       break;
      }
      check_line --;
      check_pos = strlen(se->text [se->line_index [check_line]]) - 1; // this may result in check_pos being -1, in which case the while (i <= 0) loop will reiterate
      i = check_pos;
      j = check_line;
      if (i >= 0)
						{
							i ++;
							finished = 1;
							break;
						}
     } // end while (check_pos <= 0)
     if (check_pos == 0 && check_line == 0)
      break; // reached start of file
     if (finished) // reached non-empty line when skipping past start of current line
							break;
     int new_word_type = get_skip_word_type(se->text [se->line_index [check_line]] [check_pos]);
     if (current_word_type == -1)
						current_word_type = new_word_type;
					if (current_word_type != SKIP_WORD_TYPE_SPACE)
					 found_word = 1;
     if (new_word_type != current_word_type)
     {
						if (current_word_type == SKIP_WORD_TYPE_SPACE)
						{
							current_word_type = new_word_type; // skip past spaces first
					  found_word = 1;
						}
						  else
						   break;
     }
     i = check_pos;
     j = check_line;
    } // end while(TRUE)
    se->cursor_pos = i;
    se->cursor_line = j;
    if (ex_control.special_key_press [SPECIAL_KEY_LEFT] == BUTTON_JUST_PRESSED)
      editor.key_delay = KEY_DELAY1;
       else
        editor.key_delay = KEY_DELAY2;
    editor.cursor_flash = CURSOR_FLASH_MAX;
    window_find_cursor(se);
    consider_selecting_to_cursor(se, old_line, old_pos, 1);
    return;
   } // end if left cursor key pressed

   if (ex_control.special_key_press [SPECIAL_KEY_RIGHT] > 0
    && ex_control.special_key_press [SPECIAL_KEY_LEFT] <= 0)
   {
#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" ctrl-right %i (left %i)", ex_control.special_key_press [SPECIAL_KEY_RIGHT], ex_control.special_key_press [SPECIAL_KEY_LEFT]);
#endif

    se = get_current_source_edit();
    if (se == NULL
     || !se->active)
     return;
    int old_line = se->cursor_line;
    int old_pos = se->cursor_pos;
    i = se->cursor_pos;
    j = se->cursor_line;
//    int found_end_of_line = 0;
    int current_word_type = get_skip_word_type(se->text [se->line_index [j]] [i]);
    while(TRUE)
    {
     while (se->text [se->line_index [j]] [i] == 0) //i >= strlen(se->text [se->line_index [j]])) // reached end of line
     {
      if (j >= SOURCE_TEXT_LINES - 1) // reached end of file
      {
       i = 0;
       j = SOURCE_TEXT_LINES - 1;
       break;
      }
//      found_end_of_line = 1;
      j ++;
      i = 0;
      current_word_type = SKIP_WORD_TYPE_SPACE; // end of line counts as space
     } // end while (se->text [se->line_index [j]] [i] == 0)
     if (i == 0 && j == SOURCE_TEXT_LINES - 1)
      break; // reached end of file
//     if (found_end_of_line == 1)
//      break;
//     char_type = get_source_char_type(se->text [se->line_index [j]] [i]);
     int new_word_type = get_skip_word_type(se->text [se->line_index [j]] [i]);
     if (new_word_type != current_word_type)
     {
// skip past spaces found at end of current word:
     	if (new_word_type == SKIP_WORD_TYPE_SPACE)
						 current_word_type = SKIP_WORD_TYPE_SPACE;
						  else
         break;
     }
     i ++;
     if (se->text [se->line_index [j]] [i] == 0)
						break;
    } // end while(TRUE)
    se->cursor_pos = i;
// the following block just makes sure that if the cursor goes below the window, it only scrolls a few lines rather than resetting the window. Still need to call window_find_cursor() below
    if (j > se->cursor_line)
    {
     se->cursor_line = j;
     while (se->cursor_line >= se->window_line + editor.edit_window_lines)
     {
      se->window_line ++;
     };
//     if (se->cursor_line >= se->window_line + editor.edit_window_lines)
//      se->window_line = se->cursor_line - editor.edit_window_lines;
    }
    if (ex_control.special_key_press [SPECIAL_KEY_RIGHT] == BUTTON_JUST_PRESSED)
       editor.key_delay = KEY_DELAY1;
        else
         editor.key_delay = KEY_DELAY2;
     editor.cursor_flash = CURSOR_FLASH_MAX;
     window_find_cursor(se);
     consider_selecting_to_cursor(se, old_line, old_pos, 1);
     return;
    } // end if right cursor key pressed

   if (ex_control.special_key_press [SPECIAL_KEY_END] > 0)
			{
    se = get_current_source_edit();
    if (se == NULL
     || !se->active)
     return;
    int old_line = se->cursor_line;
    int old_pos = se->cursor_pos;
    editor.cursor_flash = CURSOR_FLASH_MAX;
				if (move_cursor_to_end_of_source(se))
     consider_selecting_to_cursor(se, old_line, old_pos, 1);
    return;
			} // end if end pressed

   if (ex_control.special_key_press [SPECIAL_KEY_HOME] > 0)
			{
    se = get_current_source_edit();
    if (se == NULL
     || !se->active)
     return;
    int old_line = se->cursor_line;
    int old_pos = se->cursor_pos;
    se->cursor_line = 0;
    se->cursor_pos = 0;
    se->cursor_base = 0;
    editor.cursor_flash = CURSOR_FLASH_MAX;
    window_find_cursor(se);
    consider_selecting_to_cursor(se, old_line, old_pos, 1);
    return;
			} // end if home pressed


   editor.key_delay = 0;
   return;
  } // end of if CTRL key being pressed


 if (editor.ignore_previous_key_pressed != -1
		&& ex_control.special_key_press [editor.ignore_previous_key_pressed] <= 0)
			editor.ignore_previous_key_pressed = -1;

// first: if a key was being held down, check if it still is:
 if (editor.special_key_being_pressed != -1)
 {
#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" spec %i (status %i time %i) ", editor.special_key_being_pressed, ex_control.special_key_press [editor.special_key_being_pressed], ex_control.special_key_press_time [editor.special_key_being_pressed]);
#endif
  if (ex_control.special_key_press [editor.special_key_being_pressed] <= 0) // no longer being pressed
  {
#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" rel");
#endif
   editor.special_key_being_pressed = -1;
   editor.key_delay = 0;
// so check whether another key is being pressed instead:
   for (i = 0; i < SPECIAL_KEYS; i ++)
   {
    if (ex_control.special_key_press [i] > 0)
    {
     if (i != SPECIAL_KEY_CTRL && i != SPECIAL_KEY_SHIFT)
     {
      editor.special_key_being_pressed = i;
//      if (key_type [i].type == KEY_TYPE_OTHER)
//       editor_special_keypress(i);
//        else
#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" spkp_A %i", i);
#endif
         editor_keypress(i);
      editor.key_delay = KEY_DELAY1;
      editor.cursor_flash = CURSOR_FLASH_MAX;
      break;
 //    return;
     }
    }
   }
  }
   else // key_being_pressed is still being pressed.
   {
    if (editor.key_delay > 0)
    {
     editor.key_delay --;
    }
     else
     {
      editor.key_delay = KEY_DELAY2;
#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" spkp_B %i ", editor.special_key_being_pressed);
#endif
      editor_keypress(editor.special_key_being_pressed);
//   fprintf(stdout, "\nPressed: %i type %i max %i", editor.key_being_pressed, key_type [editor.key_being_pressed], ALLEGRO_KEY_MAX);
//     return;
     }
// now we check for another key just having been pressed:
    for (i = 0; i < SPECIAL_KEYS; i ++)
    {
     if (ex_control.special_key_press [i] == BUTTON_JUST_PRESSED
      && i != editor.special_key_being_pressed)
     {

      if (i != SPECIAL_KEY_CTRL && i != SPECIAL_KEY_SHIFT)
      {
       editor.ignore_previous_key_pressed = editor.special_key_being_pressed; // this key will be ignored if another key is pressed and released while this key is being held
       editor.special_key_being_pressed = i;
//       if (key_type [i].type == KEY_TYPE_OTHER)
//        editor_special_keypress(i);
//         else
#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" spkp_C %i", i);
#endif
          editor_keypress(i);
       editor.key_delay = KEY_DELAY1;
       editor.cursor_flash = CURSOR_FLASH_MAX;
 //    return;
// lack of return or break means that all currently pressed keys will be processed - the last one processed will end up being the new key_being_pressed
      }
     }
    }

   }

 } // end if editor.special_key_being_pressed != -1
  else
  {
    editor.key_delay = 0; // not sure this is strictly necessary
    editor.ignore_previous_key_pressed = -1;
// no key previously being pressed. Check for a new key to be pressed:
    for (i = 0; i < SPECIAL_KEYS; i ++)
    {
     if (ex_control.special_key_press [i] > 0)
     {
      if (i != SPECIAL_KEY_CTRL && i != SPECIAL_KEY_SHIFT)
      {
       editor.special_key_being_pressed = i;
//       if (key_type [i].type == KEY_TYPE_OTHER)
//        editor_special_keypress(i);
//         else
#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" spkp_D %i", i);
#endif
          editor_keypress(i);
       editor.key_delay = KEY_DELAY1;
       editor.cursor_flash = CURSOR_FLASH_MAX;
 //    return;
// lack of return or break means that all currently pressed keys will be processed - the last one processed will end up being the new key_being_pressed
      }
     }
    }
  }


/*
// first: if a key was being held down, check if it still is:
 if (editor.special_key_being_pressed != -1)
 {
  if (ex_control.special_key_press [editor.special_key_being_pressed] <= 0) // no longer being pressed
  {
   editor.special_key_being_pressed = -1;
   editor.key_delay = 0;
// so check whether another key is being pressed instead:
   for (i = 0; i < SPECIAL_KEYS; i ++)
   {
    if (ex_control.key_press [i] > 0)
    {
     if (key_type [i].type != KEY_TYPE_MOD)
     {
      editor.key_being_pressed = i;
      if (key_type [i].type == KEY_TYPE_OTHER)
       editor_special_keypress(i);
        else
         editor_keypress(i);
      editor.key_delay = KEY_DELAY1;
      editor.cursor_flash = CURSOR_FLASH_MAX;
      break;
 //    return;
     }
    }
   }
  }
   else // key_being_pressed is still being pressed.
   {
    if (editor.key_delay > 0)
    {
     editor.key_delay --;
    }
     else
     {
      editor.key_delay = KEY_DELAY2;
      editor_keypress(editor.key_being_pressed);
//   fprintf(stdout, "\nPressed: %i type %i max %i", editor.key_being_pressed, key_type [editor.key_being_pressed], ALLEGRO_KEY_MAX);
//     return;
     }
// now we check for another key just having been pressed:
    for (i = 1; i < ALLEGRO_KEY_MAX; i ++)
    {
     if (ex_control.key_press [i] == BUTTON_JUST_PRESSED
      && i != editor.key_being_pressed)
     {
      if (key_type [i].type != KEY_TYPE_MOD)
      {
       editor.ignore_previous_key_pressed = editor.key_being_pressed; // this key will be ignored if another key is pressed and released while this key is being held
       editor.key_being_pressed = i;
       if (key_type [i].type == KEY_TYPE_OTHER)
        editor_special_keypress(i);
         else
          editor_keypress(i);
       editor.key_delay = KEY_DELAY1;
       editor.cursor_flash = CURSOR_FLASH_MAX;
 //    return;
// lack of return or break means that all currently pressed keys will be processed - the last one processed will end up being the new key_being_pressed
      }
     }
    }

   }

 } // end if editor.key_being_pressed != -1
  else
  {
     editor.key_delay = 0; // not sure this is strictly necessary
// no key previously being pressed. Check for a new key to be pressed:
    for (i = 1; i < ALLEGRO_KEY_MAX; i ++)
    {
     if (ex_control.key_press [i] > 0)
     {
      if (key_type [i].type != KEY_TYPE_MOD)
      {
       editor.key_being_pressed = i;
       if (key_type [i].type == KEY_TYPE_OTHER)
        editor_special_keypress(i);
         else
          editor_keypress(i);
       editor.key_delay = KEY_DELAY1;
       editor.cursor_flash = CURSOR_FLASH_MAX;
 //    return;
// lack of return or break means that all currently pressed keys will be processed - the last one processed will end up being the new key_being_pressed
      }
     }
    }
  }
*/

}


// This function assumes that either lctrl or rctrl is being pressed.
// it checks whether key is being pressed
// then also checks that any of lctrl, rctrl or key has just been pressed
//static int just_pressed_ctrl_key(int key)
//{
/*
 if (ex_control.key_press [key] <= 0)
  return 0;

 if (ex_control.key_press [key] == BUTTON_JUST_PRESSED
  || ex_control.key_press [ALLEGRO_KEY_LCTRL] == BUTTON_JUST_PRESSED
  || ex_control.key_press [ALLEGRO_KEY_RCTRL] == BUTTON_JUST_PRESSED)
   return 1;
*/
// return 0;

//}

// like window_find_cursor() but verifies source edit first
static void reset_cursor_after_action(void)
{
	    struct source_edit_struct* se = get_current_source_edit();

     if (se != NULL
						&& se->active == 1
      && se->type == SOURCE_EDIT_TYPE_SOURCE)
     {
      window_find_cursor(se);
     }


}



static int get_skip_word_type(char current_char)
{



	switch(get_source_char_type(current_char))
	{
	 case SCHAR_SPACE:
	 case SCHAR_NULL: // end of line
		 return SKIP_WORD_TYPE_SPACE;
		case SCHAR_LETTER:
		case SCHAR_NUMBER:
		 return SKIP_WORD_TYPE_WORD;
		case SCHAR_ERROR:
			return SKIP_WORD_TYPE_ERROR;
		default:
			return SKIP_WORD_TYPE_OTHER;

	}

}



static void consider_selecting_to_cursor(struct source_edit_struct* se, int old_line, int old_pos, int require_shift)
{

 if (ex_control.special_key_press [SPECIAL_KEY_SHIFT] > 0
	|| !require_shift)
 {
  if (se->selected == 0)
  {
   se->select_fix_line = old_line;
   se->select_fix_pos = old_pos;
  }
  se->selected = 1;
  se->select_free_line = se->cursor_line;
  se->select_free_pos = se->cursor_pos;
 }
  else
   se->selected = 0;

}

static void editor_keypress_unichar(int unichar_value)
{

	unichar_value &= 0xff;

	if (valid_source_character(unichar_value) == 1) // valid_source_character returns 1 for characters that are entered in source when their key is pressed (carriage-return returns 2)
	{

		 struct source_edit_struct* se = get_current_source_edit();

		 if (se == NULL)
				return;

			if (ex_control.sticky_ctrl)
				return; // don't register a keypress if control was being pressed and the keys haven't been released since
  	completion.list_size = 0; // remove code completion box, if present
   if (!delete_selection())
   {
    window_find_cursor(se);
    return;
   }
/*   if (ex_control.key_press [ALLEGRO_KEY_LSHIFT]
    || ex_control.key_press [ALLEGRO_KEY_RSHIFT])
     add_char(key_type [key_press].shifted, 1);
      else
       add_char(key_type [key_press].unshifted, 1);*/

   add_char(unichar_value, 1);
   window_find_cursor(se);
   se->cursor_base = se->cursor_pos;
   editor.cursor_flash = CURSOR_FLASH_MAX;

	}

}

// TO DO: fold this into cursor_etc_key()
static void editor_keypress(int key_press)
{

#ifdef SANITY_CHECK
 if (key_press < 0 || key_press >= ALLEGRO_KEY_MAX)
 {
  fprintf(stdout, "\nError: key_press out of bounds (%i)", key_press);
  error_call();
 }
#endif

#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" ekp %i", key_press);
#endif

// fpr("\nkey_press %i", key_press);

 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || !se->active)
  return;

#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(".A");
#endif


 if (key_press == editor.ignore_previous_key_pressed)
		return;

//	if (editor.ignore_previous_key_pressed == -1)
//		editor.ignore_previous_key_pressed = key_press;

   if (!editor.already_pressed_cursor_key_etc)
   {
    cursor_etc_key(key_press);
    editor.already_pressed_cursor_key_etc = 1; // set to 0 again at start of editor_key_input
   }

#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr("B");
#endif

/* switch(key_type [key_press].type)
 {
  case KEY_TYPE_LETTER:
  case KEY_TYPE_NUMBER:
  case KEY_TYPE_SYMBOL:

  	break;*/

/*  	if (dwindow.templ->locked)
			{
				template_locked_editor_message();
				break;
			}* /
/ *			if (ex_control.sticky_ctrl)
				break; // don't register a keypress if control was being pressed and the keys haven't been released since
  	completion.list_size = 0; // remove code completion box, if present
   if (!delete_selection())
   {
    window_find_cursor(se);
    break;
   }
   if (ex_control.key_press [ALLEGRO_KEY_LSHIFT]
    || ex_control.key_press [ALLEGRO_KEY_RSHIFT])
     add_char(key_type [key_press].shifted, 1);
      else
       add_char(key_type [key_press].unshifted, 1);
   window_find_cursor(se);
   se->cursor_base = se->cursor_pos;
   return; // end letters* /
  case KEY_TYPE_CURSOR:
   if (!editor.already_pressed_cursor_key_etc)
   {
    cursor_etc_key(key_press);
    editor.already_pressed_cursor_key_etc = 1; // set to 0 again at start of editor_key_input
   }
   break;


 }// end switch key_type
*/

}

/*
static void editor_special_keypress(int key_press)
{

#ifdef SANITY_CHECK
 if (key_press < 0 || key_press >= ALLEGRO_KEY_MAX)
 {
  fprintf(stdout, "\nError: key_press out of bounds (%i)", key_press);
  error_call();
 }
#endif

 struct source_edit_struct* se;

 switch(key_press)
 {
  case SPECIAL_KEY_F3:
   se = get_current_source_edit();
   if (se != NULL
    && se->type == SOURCE_EDIT_TYPE_SOURCE)
   {
    find_next();
    window_find_cursor(se);
   }
   break;

 }// end switch key_press

}
*/
/*
ALLEGRO_KEY_A ... ALLEGRO_KEY_Z,
ALLEGRO_KEY_0 ... ALLEGRO_KEY_9,
ALLEGRO_KEY_PAD_0 ... ALLEGRO_KEY_PAD_9,
ALLEGRO_KEY_F1 ... ALLEGRO_KEY_F12,
ALLEGRO_KEY_ESCAPE,
ALLEGRO_KEY_TILDE,
ALLEGRO_KEY_MINUS,
ALLEGRO_KEY_EQUALS,
ALLEGRO_KEY_BACKSPACE,
ALLEGRO_KEY_TAB,
ALLEGRO_KEY_OPENBRACE, ALLEGRO_KEY_CLOSEBRACE,
ALLEGRO_KEY_ENTER,
ALLEGRO_KEY_SEMICOLON,
ALLEGRO_KEY_QUOTE,
ALLEGRO_KEY_BACKSLASH, ALLEGRO_KEY_BACKSLASH2,
ALLEGRO_KEY_COMMA,
ALLEGRO_KEY_FULLSTOP,
ALLEGRO_KEY_SLASH,
ALLEGRO_KEY_SPACE,
ALLEGRO_KEY_INSERT, ALLEGRO_KEY_DELETE,
ALLEGRO_KEY_HOME, ALLEGRO_KEY_END,
ALLEGRO_KEY_PGUP, ALLEGRO_KEY_PGDN,
ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT,
ALLEGRO_KEY_UP, ALLEGRO_KEY_DOWN,
ALLEGRO_KEY_PAD_SLASH, ALLEGRO_KEY_PAD_ASTERISK,
ALLEGRO_KEY_PAD_MINUS, ALLEGRO_KEY_PAD_PLUS,
ALLEGRO_KEY_PAD_DELETE, ALLEGRO_KEY_PAD_ENTER,
ALLEGRO_KEY_PRINTSCREEN, ALLEGRO_KEY_PAUSE,
ALLEGRO_KEY_ABNT_C1, ALLEGRO_KEY_YEN, ALLEGRO_KEY_KANA,
ALLEGRO_KEY_CONVERT, ALLEGRO_KEY_NOCONVERT,
ALLEGRO_KEY_AT, ALLEGRO_KEY_CIRCUMFLEX,
ALLEGRO_KEY_COLON2, ALLEGRO_KEY_KANJI,
ALLEGRO_KEY_LSHIFT, ALLEGRO_KEY_RSHIFT,
ALLEGRO_KEY_LCTRL, ALLEGRO_KEY_RCTRL,
ALLEGRO_KEY_ALT, ALLEGRO_KEY_ALTGR,
ALLEGRO_KEY_LWIN, ALLEGRO_KEY_RWIN,
ALLEGRO_KEY_MENU,
ALLEGRO_KEY_SCROLLLOCK,
ALLEGRO_KEY_NUMLOCK,
ALLEGRO_KEY_CAPSLOCK
ALLEGRO_KEY_PAD_EQUALS,
ALLEGRO_KEY_BACKQUOTE,
ALLEGRO_KEY_SEMICOLON2,
ALLEGRO_KEY_COMMAND
*/

// assumes that key is a valid character to put in source code (e.g. a letter)
// returns 1 on success, 0 on failure
// Note that this function is used to add letters from code completion
// set check_completion to 0 if doing so
int add_char(char added_char, int check_completion)
{

 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || !se->active)
  return 0;

// fprintf(stdout, "\nAdding %i at line %i pos %i index line %i", added_char, se->cursor_line, se->cursor_pos, se->line_index [se->cursor_line]);
// return;

// make sure the new character will fit:
 if (se->cursor_pos >= SOURCE_TEXT_LINE_LENGTH - 2
  || strlen(se->text [se->line_index [se->cursor_line]]) >= SOURCE_TEXT_LINE_LENGTH - 2)
 {
  write_line_to_log("Source line too long.", MLOG_COL_ERROR);
  return 0; // fail
 }

// are the - 2 bits in the if statement correct? Not 100% sure.

// now shift the rest of the line to the right:
 int i = strlen(se->text [se->line_index [se->cursor_line]]);

 int at_line_end = 0;

 if (i == se->cursor_pos)
 {
  at_line_end = 1;
 }

 i++;

 while (i > se->cursor_pos)
 {
  if (i > 0)
   se->text [se->line_index [se->cursor_line]] [i] = se->text [se->line_index [se->cursor_line]] [i - 1];
  i --;
 };

 if (at_line_end == 1)
  se->text [se->line_index [se->cursor_line]] [se->cursor_pos + 1] = '\0';
 se->text [se->line_index [se->cursor_line]] [se->cursor_pos] = added_char;
 add_char_undo(added_char);

 if (check_completion) // don't want to check it when we're actually adding letters because of code completion
  check_code_completion(se, 0);

 se->cursor_pos ++;
 se->saved = 0; // indicates that source has been modified

 update_source_lines(se, se->cursor_line, 1);

 return 1;

}
/*
void update_source_lines(struct source_edit_struct* se, int sline, int lines)
{

 int next_line_commented = 0;

 if (sline < SOURCE_TEXT_LINES - 1)
 {

  next_line_commented = se->comment_line [se->line_index [sline + 1]];
 }

 int new_comment = source_line_highlight_syntax(se, se->line_index [sline], se->comment_line [se->line_index [sline]]);
// source_line_highlight_syntax returns 1 if the line ends in a multi-line comment

// the update may have resulted in the line now ending in a multi-line comment when it didn't before:
 int i = sline;

 if (new_comment
  && !next_line_commented)
 {
  do
  {
   i++;
  }  while (i < SOURCE_TEXT_LINES - 1
         && source_line_highlight_syntax(se, se->line_index [i], 1));
 }

// the update may have resulted in the line now *not* ending in a multi-line comment when it did before:
 if (!new_comment
  && next_line_commented)
 {
  do
  {
   i++;
  }  while (i < SOURCE_TEXT_LINES - 1
         && !source_line_highlight_syntax(se, se->line_index [i], 0));
 }


}
*/



// updates syntax highlighting on one or more lines of source code
// if these lines now end in a comment when they didn't before, or vice-versa, it continues on and updates further lines.
// sline is position in line_index array (not text array)
void update_source_lines(struct source_edit_struct* se, int sline, int lines)
{

// fprintf(stdout, "\nUpdating: sline %i lines %i", sline, lines);

// first update all lines we've been told to update:
 int i = 0;
 int in_a_comment = se->comment_line [se->line_index [sline]];

 if (sline + lines >= SOURCE_TEXT_LINES - 1)
  lines = SOURCE_TEXT_LINES - sline - 1;

 while(i < lines)
 {
  in_a_comment = source_line_highlight_syntax(se, se->line_index [sline + i], in_a_comment);
  i ++;

 };
// finished updating the specified lines.
// now see if the update has resulted in the immediately following line's comment status changing:


 sline += lines - 1;

 if (sline >= SOURCE_TEXT_LINES - 1)
  return;

 int next_line_commented = se->comment_line [se->line_index [sline + 1]];
// int comment_status = 0;

 i = sline;

// fprintf(stdout, "\nin_a_comment %i next_line_commented %i", in_a_comment, next_line_commented);



// the update may have resulted in the line now ending in a multi-line comment when it didn't before:
 if (in_a_comment
 && !next_line_commented)
 {
  do
  {
   i++;
  }  while (i < SOURCE_TEXT_LINES - 1
         && source_line_highlight_syntax(se, se->line_index [i], 1));
 }

// the update may have resulted in the line now *not* ending in a multi-line comment when it did before:
 if (!in_a_comment
  && next_line_commented)
 {
  do
  {
   i++;
// this doesn't work correctly - it uncomments many lines that it doesn't need to (although the ultimate result is correct). Not sure why.
  }  while (i < SOURCE_TEXT_LINES - 1
         && !source_line_highlight_syntax(se, se->line_index [i], 0));
 }



}




static void cursor_etc_key(int key_press)
{

#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" cek %i", key_press);
#endif


 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || !se->active
  || se->type != SOURCE_EDIT_TYPE_SOURCE)
  return;

 int i;

 editor.cursor_flash = CURSOR_FLASH_MAX;

 switch(key_press)
 {

  case SPECIAL_KEY_F3:
  	 submenu_operation(SUBMENU_SEARCH, SUBMENU_SEARCH_FIND_NEXT);
//    find_next();
//    window_find_cursor(se);
    break;
//  case SPECIAL_KEY_F8:
//  	 submenu_operation(SUBMENU_COMPILE, SUBMENU_COMPILE_TEST);
//    break;
  case SPECIAL_KEY_F10:
  	 if (ex_control.special_key_press [SPECIAL_KEY_SHIFT])
  	  submenu_operation(SUBMENU_COMPILE, SUBMENU_COMPILE_TEST);
  	   else
  	    submenu_operation(SUBMENU_COMPILE, SUBMENU_COMPILE_COMPILE);
    break;

  case SPECIAL_KEY_LEFT:
  case SPECIAL_KEY_RIGHT:
  case SPECIAL_KEY_UP:
  case SPECIAL_KEY_DOWN:
  case SPECIAL_KEY_END:
  case SPECIAL_KEY_HOME:
  case SPECIAL_KEY_PGUP:
  case SPECIAL_KEY_PGDN:
   movement_keys(se);
   break;

// text movement, deletion etc:
  case SPECIAL_KEY_ENTER:
/*  	if (dwindow.templ->locked)
			{
				template_locked_editor_message();
				break;
			}*/
   se->saved = 0; // indicates that source has been modified
   if (completion.list_size != 0)
			{
				complete_code(se, completion.select_line);
				break;
			}
   if (!delete_selection())
   {
    window_find_cursor(se);
    break;
   }
// pressing enter inserts an empty line directly after the current line, then copies the rest of the current line to it, then deletes the rest of the current line.
// first insert an empty line (must check whether possible):
   if (insert_empty_lines(se, se->cursor_line + 1, 1))
   {
// current line is now followed by an empty line.
// now copy remaining text from current line onto next line:
    i = 0;
// check whether we want to indent the next line:
    int next_line_indent = 0; // count the leading spaces on the current line
    while(se->text [se->line_index [se->cursor_line]] [i] == ' ')
				{
					next_line_indent ++;
					i ++;
				}
				if (next_line_indent > se->cursor_pos)
					next_line_indent = se->cursor_pos; // don't indent further than the cursor's position
				i = 0;
// now indent the next line:
    while(i < next_line_indent)
				{
					se->text [se->line_index [se->cursor_line + 1]] [i] = ' ';
					i ++;
				}
				i = 0;
    while (se->text [se->line_index [se->cursor_line]] [se->cursor_pos + i] != '\0')
    {
     se->text [se->line_index [se->cursor_line + 1]] [i + next_line_indent] = se->text [se->line_index [se->cursor_line]] [se->cursor_pos + i];
     i++;
    };
    se->text [se->line_index [se->cursor_line + 1]] [i + next_line_indent] = '\0';
    se->text [se->line_index [se->cursor_line]] [se->cursor_pos] = '\0';
// update syntax highlighting for both lines:
    update_source_lines(se, se->cursor_line, 2);
//    update_source_lines(se, se->cursor_line + 1);
// put cursor on next line
    add_enter_undo();
    se->cursor_line ++;
    se->cursor_pos = next_line_indent;
    if (se->cursor_line >= se->window_line + editor.edit_window_lines)
     se->window_line ++;
    window_find_cursor(se);
   }
    else
    {
      write_line_to_log("Out of space in source file.", MLOG_COL_ERROR);
    }
   se->cursor_base = se->cursor_pos;
   break;

  case SPECIAL_KEY_BACKSPACE:
/*  	if (dwindow.templ->locked)
			{
				template_locked_editor_message();
				break;
			}*/
   se->saved = 0; // indicates that source has been modified
   if (is_something_selected(se))
   {
    delete_selection(); // ignore return value
    return;
   }
   if (se->cursor_pos > 0)
   {
    se->cursor_pos --;
    backspace_char_undo(se->text [se->line_index [se->cursor_line]] [se->cursor_pos]);
    check_code_completion(se, 1);
    i = se->cursor_pos;
    while(i < SOURCE_TEXT_LINE_LENGTH - 1)
    {
// this loop runs to the end of the line, ignoring null terminator, but that doesn't really matter.
     se->text [se->line_index [se->cursor_line]] [i] = se->text [se->line_index [se->cursor_line]] [i + 1];
     i++;
    };
    update_source_lines(se, se->cursor_line, 1);
    window_find_cursor(se);
    se->cursor_base = se->cursor_pos;
    break;
   }
// cursor must be at start of line.
// first make sure it's not at the start of the file:
   if (se->cursor_line == 0)
   {
    window_find_cursor(se);
    se->cursor_base = se->cursor_pos;
    break;
   }
// now make sure the current line can fit on the end of the previous line:
   if (strlen(se->text [se->line_index [se->cursor_line]]) + strlen(se->text [se->line_index [se->cursor_line - 1]]) >= SOURCE_TEXT_LINE_LENGTH)
   {
    write_line_to_log("Lines too long to combine.", MLOG_COL_ERROR);
    se->cursor_base = se->cursor_pos;
    break;
   }
// now set cursor pos to the end of the previous line:
   se->cursor_pos = strlen(se->text [se->line_index [se->cursor_line - 1]]);
   add_undo_remove_enter();
// copy current line to end of previous line, and update it:
   strcat(se->text [se->line_index [se->cursor_line - 1]], se->text [se->line_index [se->cursor_line]]);
// now delete the current line:
   delete_lines(se, se->cursor_line, 1);
// finally reduce cursor_line and run syntax highlighting:
   se->cursor_line --;
   update_source_lines(se, se->cursor_line, 1);
   window_find_cursor(se);
   se->cursor_base = se->cursor_pos;
   break;

  case SPECIAL_KEY_DELETE:
/*  	if (dwindow.templ->locked)
			{
				template_locked_editor_message();
				break;
			}*/
   se->saved = 0; // indicates that source has been modified
   if (is_something_selected(se))
   {
    delete_selection(); // ignore return value
    return;
   }
   if (se->cursor_pos < strlen(se->text [se->line_index [se->cursor_line]]))
   {
    delete_char_undo(se->text [se->line_index [se->cursor_line]] [se->cursor_pos]);
    i = se->cursor_pos;
    while(i < SOURCE_TEXT_LINE_LENGTH - 1)
    {
// this loop runs to the end of the line, ignoring null terminator, but that doesn't really matter.
     se->text [se->line_index [se->cursor_line]] [i] = se->text [se->line_index [se->cursor_line]] [i + 1];
     i++;
    };
    update_source_lines(se, se->cursor_line, 1);
    window_find_cursor(se);
    se->cursor_base = se->cursor_pos;
    break;
   }
// cursor must be at the end of the line.
// first make sure it's not at the end of the file:
   if (se->cursor_line >= SOURCE_TEXT_LINES - 1)
   {
    window_find_cursor(se);
    se->cursor_base = se->cursor_pos;
    break;
   }
// now make sure the next line can fit on the end of the current line:
   if (strlen(se->text [se->line_index [se->cursor_line]]) + strlen(se->text [se->line_index [se->cursor_line + 1]]) >= SOURCE_TEXT_LINE_LENGTH)
   {
    write_line_to_log("Lines too long to combine.", MLOG_COL_ERROR);
    se->cursor_base = se->cursor_pos;
    break;
   }
   add_undo_remove_enter();
// now copy next line to end of current line, and update it:
   strcat(se->text [se->line_index [se->cursor_line]], se->text [se->line_index [se->cursor_line + 1]]);
// now delete the next line:
   delete_lines(se, se->cursor_line + 1, 1);
// finally run syntax highlighting:
   update_source_lines(se, se->cursor_line, 1);
   window_find_cursor(se);
   se->cursor_base = se->cursor_pos;
   break;




 }

}

// if the user presses a key while the window is away from the cursor, bring the window back
// also updates the scrollbar to deal with any changes in window position
void window_find_cursor(struct source_edit_struct* se)
{

 if (se->window_line <= se->cursor_line - editor.edit_window_lines
  || se->window_line >= se->cursor_line)// + editor.edit_window_lines)
 {
  se->window_line = se->cursor_line - 20;
  if (se->window_line < 0)
			se->window_line = 0;
 }

 if (se->window_pos < se->cursor_pos - editor.edit_window_chars
  || se->window_pos > se->cursor_pos)// + editor.edit_window_lines)
 {
// 	fprintf(stdout, "\nwindow_pos %i cursor_pos %i ewc %i", se->window_pos, se->cursor_pos, editor.edit_window_chars);
  se->window_pos = se->cursor_pos - editor.edit_window_chars;
  if (se->cursor_pos + se->window_pos > editor.edit_window_chars - 3)
   se->window_pos += 8;
  if (se->window_pos < 0)
   se->window_pos = 0;
 }


 slider_moved_to_value(&slider[SLIDER_EDITOR_SCROLLBAR_V], se->window_line);
 slider_moved_to_value(&slider[SLIDER_EDITOR_SCROLLBAR_H], se->window_pos);

}


// if any movement key is being pressed, all of them are accepted inputs:
static void movement_keys(struct source_edit_struct* se)
{

#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" mov(ign %i)", editor.ignore_previous_key_pressed);
#endif



 int old_line = se->cursor_line;
 int old_pos = se->cursor_pos;
 editor.cursor_flash = CURSOR_FLASH_MAX;

 int reset_code_completion_box = 1; // completion.list_size will be reset to 0 if this is still 1 by the end of the function. It is set to zero by up and down

// the following is code for left arrow without control (left arrow with control is dealt with in editor_input_keys()):
 if (ex_control.special_key_press [SPECIAL_KEY_LEFT] > 0
		&& editor.ignore_previous_key_pressed != SPECIAL_KEY_LEFT
  && ex_control.special_key_press [SPECIAL_KEY_RIGHT] <= 0)
 {

#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" left %i (ign %i right %i)",
			ex_control.special_key_press [SPECIAL_KEY_LEFT],
		 editor.ignore_previous_key_pressed,
   ex_control.special_key_press [SPECIAL_KEY_RIGHT]);
#endif

    if (se->cursor_pos > 0)
    {
     se->cursor_pos --;
    }
     else
     {
      if (se->cursor_line > 0)
      {
       se->cursor_line --;
       se->cursor_pos = strlen(se->text [se->line_index [se->cursor_line]]);
       if (se->cursor_line < se->window_line)
        se->window_line --;
      }
     }
   window_find_cursor(se);
   se->cursor_base = se->cursor_pos;
 }

 if (ex_control.special_key_press [SPECIAL_KEY_RIGHT] > 0
		&& editor.ignore_previous_key_pressed != SPECIAL_KEY_RIGHT
  && ex_control.special_key_press [SPECIAL_KEY_LEFT] <= 0)
 {

#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" right %i (ign %i left %i)",
			ex_control.special_key_press [SPECIAL_KEY_RIGHT],
		 editor.ignore_previous_key_pressed,
   ex_control.special_key_press [SPECIAL_KEY_LEFT]);
#endif


// the following is code for right arrow without control (right arrow with control is dealt with in editor_input_keys()):
   if (se->cursor_pos < strlen(se->text [se->line_index [se->cursor_line]]))
   {
    se->cursor_pos ++;
   }
    else
    {
     if (se->cursor_line < SOURCE_TEXT_LINES - 1)
     {
      se->cursor_line ++;
      se->cursor_pos = 0;
      if (se->cursor_line >= se->window_line + editor.edit_window_lines)
       se->window_line ++;
     }
    }
   window_find_cursor(se);
   se->cursor_base = se->cursor_pos;
 }


 if (ex_control.special_key_press [SPECIAL_KEY_UP] > 0
		&& editor.ignore_previous_key_pressed != SPECIAL_KEY_UP
  && ex_control.special_key_press [SPECIAL_KEY_DOWN] <= 0)
 {


#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" up %i (ign %i dwn %i)",
			ex_control.special_key_press [SPECIAL_KEY_UP],
		 editor.ignore_previous_key_pressed,
   ex_control.special_key_press [SPECIAL_KEY_DOWN]);
#endif

		if (completion.list_size > 0)
		{
			completion_box_select_line_up();
	  reset_code_completion_box = 0;
		}
		 else
			{
    if (se->cursor_line > 0)
    {
     se->cursor_line --;
     se->cursor_pos = se->cursor_base;
     if (se->cursor_pos > strlen(se->text [se->line_index [se->cursor_line]]))
      se->cursor_pos = strlen(se->text [se->line_index [se->cursor_line]]);
     if (se->cursor_line < se->window_line)
      se->window_line --;
    }
    window_find_cursor(se);
			}
 }


 if (ex_control.special_key_press [SPECIAL_KEY_DOWN] > 0
		&& editor.ignore_previous_key_pressed != SPECIAL_KEY_DOWN
  && ex_control.special_key_press [SPECIAL_KEY_UP] <= 0)
 {

#ifdef DEBUG_MODE
 if (editor.debug_keys)
		fpr(" dwn %i (ign %i up %i)",
			ex_control.special_key_press [SPECIAL_KEY_DOWN],
		 editor.ignore_previous_key_pressed,
   ex_control.special_key_press [SPECIAL_KEY_UP]);
#endif

		if (completion.list_size > 0)
		{
   completion_box_select_line_down();
	  reset_code_completion_box = 0;
		}
		 else
			{
    if (se->cursor_line < SOURCE_TEXT_LINES - 1)
    {
     se->cursor_line ++;
     se->cursor_pos = se->cursor_base;
     if (se->cursor_pos > strlen(se->text [se->line_index [se->cursor_line]]))
      se->cursor_pos = strlen(se->text [se->line_index [se->cursor_line]]);
     if (se->cursor_line >= se->window_line + editor.edit_window_lines)
      se->window_line ++;
    }
    window_find_cursor(se);
			}
 }


 if (ex_control.special_key_press [SPECIAL_KEY_END] > 0
//		&& editor.ignore_previous_key_pressed != SPECIAL_KEY_DOWN
  && ex_control.special_key_press [SPECIAL_KEY_HOME] <= 0)
  {
   se->cursor_pos = strlen(se->text [se->line_index [se->cursor_line]]);
   window_find_cursor(se);
   se->cursor_base = se->cursor_pos;
  }

 if (ex_control.special_key_press [SPECIAL_KEY_HOME] > 0
//		&& editor.ignore_previous_key_pressed != SPECIAL_KEY_DOWN
  && ex_control.special_key_press [SPECIAL_KEY_END] <= 0)
 {
   se->cursor_pos = 0;//strlen(se->text [se->line_index [se->cursor_line]]);
   window_find_cursor(se);
   se->cursor_base = se->cursor_pos;
 }

 if (ex_control.special_key_press [SPECIAL_KEY_PGUP] > 0
  && ex_control.special_key_press [SPECIAL_KEY_PGDN] <= 0)
 {
 		if (completion.list_size > 0)
		 {
 			completion_box_select_lines_up(11);
	   reset_code_completion_box = 0;
		 }
		  else
		  {
     se->cursor_line -= editor.edit_window_lines - 1;
     if (se->cursor_line < 0)
      se->cursor_line = 0;
     if (se->cursor_pos > strlen(se->text [se->line_index [se->cursor_line]]))
      se->cursor_pos = strlen(se->text [se->line_index [se->cursor_line]]);
     se->window_line -= editor.edit_window_lines - 1;
     if (se->window_line < 0)
      se->window_line = 0;
     window_find_cursor(se);
     se->cursor_base = se->cursor_pos;
		  }
  }

 if (ex_control.special_key_press [SPECIAL_KEY_PGDN] > 0
  && ex_control.special_key_press [SPECIAL_KEY_PGUP] <= 0)
 {
		if (completion.list_size > 0)
	 {
			completion_box_select_lines_down(11);
   reset_code_completion_box = 0;
	 }
	  else
			{
    se->cursor_line += editor.edit_window_lines - 1;
    if (se->cursor_line >= SOURCE_TEXT_LINES)
     se->cursor_line = SOURCE_TEXT_LINES - 1;
    if (se->cursor_pos > strlen(se->text [se->line_index [se->cursor_line]]))
     se->cursor_pos = strlen(se->text [se->line_index [se->cursor_line]]);
    se->window_line += editor.edit_window_lines - 1;
    if (se->window_line >= SOURCE_TEXT_LINES)
     se->window_line = SOURCE_TEXT_LINES - 1;
    window_find_cursor(se);
    se->cursor_base = se->cursor_pos;
			}
  }

 consider_selecting_to_cursor(se, old_line, old_pos, 1);
/*

 if (ex_control.key_press [ALLEGRO_KEY_LSHIFT] > 0
  || ex_control.key_press [ALLEGRO_KEY_RSHIFT] > 0)
 {
  if (se->selected == 0)
  {
   se->select_fix_line = old_line;
   se->select_fix_pos = old_pos;
  }
  se->selected = 1;
  se->select_free_line = se->cursor_line;
  se->select_free_pos = se->cursor_pos;
 }
  else
   se->selected = 0;
*/
 if (reset_code_completion_box == 1)
		completion.list_size = 0;

}



// returns 1 if successful, 0 on failure
// before_line is position in line_index
int insert_empty_lines(struct source_edit_struct* se, int before_line, int lines)
{
//fpr("\nInserting %i lines before line %i", lines, before_line);
 if (before_line + lines >= SOURCE_TEXT_LINES - 1)
  return 0;

 if (lines <= 0)
  return 1;

 int in_a_comment = 0;

 if (before_line > 0
  && se->comment_line [se->line_index [before_line - 1]])
   in_a_comment = 1;

// first make sure there are enough empty lines at the end of the source
// also copy the indexes of those empty lines into a storage array so they can be used later
 int i;
 int line_index_save [SOURCE_TEXT_LINES];
 int j = 0;

 for (i = SOURCE_TEXT_LINES - lines; i < SOURCE_TEXT_LINES; i ++)
 {
  if (se->text [se->line_index [i]] [0] != '\0')
   return 0; // failed
  line_index_save [j] = se->line_index [i];
  j++;
 }

// now work back from the end of the end of the source, pushing each line back by lines:
 i = SOURCE_TEXT_LINES - 1;

 while (i >= before_line + lines)
 {
  se->line_index [i] = se->line_index [i - lines];
  i --;
 };

// now set the line_index values of the inserted lines to the lines that were removed from the end of the source:
 for (i = 0; i < lines; i ++)
 {
  se->line_index [before_line + i] = line_index_save [i];
  se->comment_line [se->line_index [before_line + i]] = in_a_comment;
 }

 return 1;

}


// start_line is position in line_index
// note that the content of the deleted line is lost.
void delete_lines(struct source_edit_struct* se, int start_line, int lines)
{

 if (start_line + lines >= SOURCE_TEXT_LINES - 1)
  return; // hopefully this should never happen if this function is called correctly

// first store the indexes of the lines being deleted.
 int i;
 int line_index_save [SOURCE_TEXT_LINES];
 int j = 0;

 for (i = start_line; i < start_line + lines; i ++)
 {
  line_index_save [j] = se->line_index [i];
  j++;
 }

// now reassign all lines after start_line, up to (SOURCE_TEXT_LINES - lines):
 i = start_line;
 while (i + lines < SOURCE_TEXT_LINES)
 {
  se->line_index [i] = se->line_index [i + lines];
  i ++;
 };

 int source_ends_in_a_comment = 0;

 if (se->comment_line [se->line_index [SOURCE_TEXT_LINES - 1]])
  source_ends_in_a_comment = 1; // this may not work correctly if the last line in the file contains an opening comment. Oh well.

// now replace the remaining lines at the end of the text array with the lines that were saved earlier:
 for (i = 0; i < lines; i ++)
 {
  se->text [line_index_save [i]] [0] = '\0';
  se->line_index [SOURCE_TEXT_LINES - lines + i] = line_index_save [i];
  se->comment_line [se->line_index [SOURCE_TEXT_LINES - lines + i]] = source_ends_in_a_comment;
 }


}



// assumes select_line/pos values are in-bounds
// returns 1 on success, 0 on failure
// success means no error (may not actually delete anything)
int delete_selection(void)
{

 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || !se->active)
  return 0;

 if (!is_something_selected(se))
  return 1;

 se->saved = 0; // indicates that source has been modified

 int start_line = 0, start_pos = 0, end_line = 0, end_pos = 0;

 int i;

// if the start and end are on the same line, it's a bit simpler:
 if (se->select_fix_line == se->select_free_line)
 {
  if (se->select_fix_pos == se->select_free_pos)
   return 1; // nothing to delete
  start_pos = se->select_fix_pos;
  end_pos = se->select_free_pos;
  if (se->select_free_pos < se->select_fix_pos)
  {
   start_pos = se->select_free_pos;
   end_pos = se->select_fix_pos;
  }
// add the selected text to the undo buffer:
  se->cursor_line = se->select_fix_line;
  se->cursor_pos = start_pos;
  for (i = start_pos; i < end_pos; i ++)
  {
//   se->cursor_pos ++;
   delete_char_undo(se->text [se->line_index [se->select_fix_line]] [i]);
  }
// now delete the text by copying over it with later text from the same line:
  i = 0;
  while (TRUE)
  {
   se->text [se->line_index [se->select_fix_line]] [start_pos + i] = se->text [se->line_index [se->select_fix_line]] [end_pos + i];
   if (se->text [se->line_index [se->select_fix_line]] [end_pos + i] == '\0')
    break;
   i ++;
  };
  se->selected = 0;
  update_source_lines(se, se->select_fix_line, 1);
  se->cursor_line = se->select_fix_line;
  se->cursor_pos = start_pos;
  window_find_cursor(se);
  return 1;
 }

 if (se->select_fix_line < se->select_free_line)
 {
  start_line = se->select_fix_line;
  start_pos = se->select_fix_pos;
  end_line = se->select_free_line;
  end_pos = se->select_free_pos;
 }
 if (se->select_fix_line > se->select_free_line)
 {
  start_line = se->select_free_line;
  start_pos = se->select_free_pos;
  end_line = se->select_fix_line;
  end_pos = se->select_fix_pos;
 }


// this is broken (it activates when it shouldn't). Need to fix
 if (strlen(se->text [se->line_index [start_line]]) + strlen(se->text [se->line_index [end_line]]) >= SOURCE_TEXT_LINE_LENGTH - 1)
 {
   write_line_to_log("Couldn't delete selection; first and last lines too long to join.", MLOG_COL_ERROR);
   return 0;
 }


 add_block_to_undo(start_line, start_pos, end_line, end_pos, UNDO_TYPE_DELETE_BLOCK);

// at this point we know multiple lines are involved.
// first we remove all lines between the first and last (if any):
 if (end_line - start_line > 1)
 {
  delete_lines(se, start_line + 1, end_line - start_line - 1);
  end_line = start_line + 1;
 }

/*

TO DO:  put this code back in (with undo)

// Now we check that it will be possible to stitch together the first and last lines:
 if (strlen(se->text [se->line_index [start_line]]) + strlen(se->text [se->line_index [end_line]]) >= SOURCE_TEXT_LINE_LENGTH - 1)
 {
// lines are too long. So we just remove selected text from each:
  se->text [se->line_index [start_line]] [start_pos] = '\0';
  i = 0;
  while (TRUE)
  {
   se->text [se->line_index [end_line]] [i] = se->text [se->line_index [end_line]] [end_pos + i];
   if (se->text [se->line_index [end_line]] [end_pos + i] == '\0')
    break;
   i ++;
  };
  se->selected = 0;
  update_source_lines(se, start_line, 2);
//  update_source_lines(se, end_line, 1);
  se->cursor_line = start_line;
  se->cursor_pos = start_pos;
  window_find_cursor(se);
  return;
 }*/

// truncate start line:
 se->text [se->line_index [start_line]] [start_pos] = '\0';
// add end line to start line:
 char* end_line_end_pos = &se->text [se->line_index [end_line]] [end_pos];
 strcat(se->text [se->line_index [start_line]], end_line_end_pos);
// strcat(se->text [se->line_index [start_line]], se->text [se->line_index [end_line]]);
// delete end line:
 delete_lines(se, end_line, 1);
 update_source_lines(se, start_line, 1);
 se->cursor_line = start_line;
 se->cursor_pos = start_pos;
 window_find_cursor(se);
 se->selected = 0;

 return 1;

}

// returns 1 if the cursor found anything before right at the start of the file
// returns 0 if the cursor is right at the start
static int move_cursor_to_end_of_source(struct source_edit_struct* se)
{

	int last_line = SOURCE_TEXT_LINES - 1;

	while(se->text [se->line_index [last_line]] [0] == 0)
	{
		last_line --;
		if (last_line <= 1)
			break;
	};

	se->cursor_line = last_line;
	se->cursor_pos = strlen(se->text [se->line_index [last_line]]);
 window_find_cursor(se);
 se->cursor_base = se->cursor_pos;

 return 1;
}


// checks whether at least one character is selected
int is_something_selected(struct source_edit_struct* se)
{

 if (se->selected == 1
  && (se->select_fix_line != se->select_free_line
   || se->select_fix_pos != se->select_free_pos))
   return 1;

 return 0;

}

// returns pointer to currently open source_edit (based on which tab is open)
// returns NULL if none open (e.g. current tab is not a source file)
struct source_edit_struct* get_current_source_edit(void)
{

// current source_edit should really be a struct pointer in editor struct
 if (editor.current_source_edit_index == -1)
		return NULL;

 return &editor.source_edit [editor.current_source_edit_index];

/*
 if (editor.current_tab == -1
 || (editor.tab_type [editor.current_tab] != TAB_TYPE_SOURCE
  && editor.tab_type [editor.current_tab] != TAB_TYPE_BINARY))
  return NULL;

 return &editor.source_edit [editor.tab_index [editor.current_tab]];
*/
}

// Like get_current_source_edit() but returns index in editor.source_edit []
// Returns -1 if no current source_edit
int get_current_source_edit_index(void)
{

 return editor.current_source_edit_index;
/*
 if (editor.current_tab == -1
 || (editor.tab_type [editor.current_tab] != TAB_TYPE_SOURCE
  && editor.tab_type [editor.current_tab] != TAB_TYPE_BINARY))
  return -1;

 return editor.tab_index [editor.current_tab];
*/

}


// changes the currently open tab
static void editor_change_source_edit(int new_source_edit_index)
{

 if (new_source_edit_index == -1)
 {
   editor.current_source_edit_index = -1;
// should do something about the sliders here
   return;
 }

 editor.current_source_edit_index = new_source_edit_index;

 init_slider(SLIDER_EDITOR_SCROLLBAR_V,
													PANEL_EDITOR, // pan
													FSP_EDITOR_WHOLE, // subpan
													FPE_EDITOR_WINDOW_SCROLLBAR_V, // element
													&editor.source_edit[new_source_edit_index].window_line, // value_pointer
													SLIDEDIR_VERTICAL, // dir
													0, // value_min
													SOURCE_TEXT_LINES, // value_max
													editor.edit_window_h, // total_length
													1, // button_increment
													(editor.edit_window_h / EDIT_LINE_H) - 1, // track_increment // panel[pan].element[FPE_DESIGN_WINDOW].h
													editor.edit_window_h / EDIT_LINE_H, // slider_represents_size
													SLIDER_BUTTON_SIZE, // thickness
													COL_BLUE, // colour
													0); // 1 = hidden if unused

 init_slider(SLIDER_EDITOR_SCROLLBAR_H,
													PANEL_EDITOR, // pan
													FSP_EDITOR_WHOLE, // subpan
													FPE_EDITOR_WINDOW_SCROLLBAR_H, // element
													&editor.source_edit[new_source_edit_index].window_pos, // value_pointer
													SLIDEDIR_HORIZONTAL, // dir
													0, // value_min
													SOURCE_TEXT_LINE_LENGTH, // value_max
													editor.edit_window_w, // total_length
													1, // button_increment
													(editor.edit_window_w / editor.text_width) - 1, // track_increment // panel[pan].element[FPE_DESIGN_WINDOW].h
													editor.edit_window_w / editor.text_width, // slider_represents_size
													SLIDER_BUTTON_SIZE, // thickness
													COL_BLUE, // colour
													0); // 1 = hidden if unused

 reset_editor_slider_locations();


}


static int compile_current_source_edit(int compile_mode)
{

 struct source_edit_struct* se;

 se = get_current_source_edit();

     if (se != NULL
						&& se->active == 1
      && se->type == SOURCE_EDIT_TYPE_SOURCE)
     {
    	 return compile(dwindow.templ, se, compile_mode);
     }


  write_line_to_log("No source code to compile.", MLOG_COL_ERROR);
  return 0;

}



// this function is called after the editor does something (like opening a file or running the compiler) that may take a significant amount of time
// it's also called from other places (e.g. from game startup in g_game.c)
void flush_game_event_queues(void)
{

 al_flush_event_queue(event_queue);
 al_flush_event_queue(fps_queue);
 al_flush_event_queue(control_queue);

}



