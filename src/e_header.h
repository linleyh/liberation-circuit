
#ifndef H_E_HEADER
#define H_E_HEADER

#include "e_slider.h"
#include "g_header.h"

#define ELINE_LENGTH 80

#define ELINE_HEIGHT 15
#define ELETTER_WIDTH 6
#define ESOURCES (PLAYERS * TEMPLATES_PER_PLAYER)


enum
{
TAB_TYPE_NONE,
TAB_TYPE_SOURCE,
TAB_TYPE_BINARY
};

enum
{
SOURCE_EDIT_TYPE_SOURCE, // should only be stored in TAB_TYPE_SOURCE
SOURCE_EDIT_TYPE_BINARY, // TAB_TYPE_BINARY
};

#define CLIPBOARD_SIZE (SOURCE_TEXT_LINES * SOURCE_TEXT_LINE_LENGTH + 1)
#define UNDO_STACK_SIZE 20
//#define UNDO_BUFFER_SIZE 20
#define UNDO_BUFFER_SIZE (CLIPBOARD_SIZE * 2)
#define CURSOR_FLASH_MAX 40
#define CURSOR_FLASH_OFF 20

enum
{
UNDO_TYPE_NONE,
UNDO_TYPE_INSERT_TEXT, // this means text has been inserted. stores the text on the undo buffer in case of a redo.
UNDO_TYPE_INSERT_BLOCK,
UNDO_TYPE_BACKSPACE,
UNDO_TYPE_DELETE,
UNDO_TYPE_DELETE_BLOCK,
UNDO_TYPE_ENTER,
UNDO_TYPE_DELETE_ENTER

};

// EBAR_HEIGHT is the height of menu/tab bars in pixels
#define EBAR_HEIGHT scaleUI_y(FONT_BASIC,12)
// EMENUS_Y is the y location of the menu bar (file etc); _X is the base x location. WIDTH is the width of each menu name
#define EMENU_BAR_Y scaleUI_y(FONT_SQUARE,15)
#define EMENU_BAR_X 5
#define EMENU_BAR_NAME_WIDTH scaleUI_x(FONT_BASIC,45)
#define EMENU_BAR_H scaleUI_y(FONT_BASIC,15)

enum
{
SUBMENU_FILE,
SUBMENU_EDIT,
SUBMENU_SEARCH,
SUBMENU_COMPILE,
SUBMENUS

};

#define SUBMENU_MAX_LINES 7

#define SUBMENU_NAME_LENGTH 20
#define SUBMENU_SHORTCUT_LENGTH 12

#define SUBMENU_WIDTH scaleUI_x(FONT_BASIC,150)
#define SUBMENU_LINE_HEIGHT scaleUI_y(FONT_BASIC, 15)

/*
#define SOURCE_TAB_X 5
#define SOURCE_TAB_Y (EMENU_BAR_Y+EMENU_BAR_H)
#define SOURCE_TAB_W 75
#define SOURCE_TAB_H 15*/

struct submenu_linestruct
{

 char name [SUBMENU_NAME_LENGTH];
 char shortcut [SUBMENU_SHORTCUT_LENGTH];
// int help_type;

};

struct submenustruct
{

 int lines;
 struct submenu_linestruct line [SUBMENU_MAX_LINES];

};


enum
{
//SUBMENU_FILE_NEW,
SUBMENU_FILE_OPEN,
SUBMENU_FILE_RELOAD,
SUBMENU_FILE_SAVE,
SUBMENU_FILE_SAVE_AS,
SUBMENU_FILE_SAVE_ALL,
//SUBMENU_FILE_CLOSE,
//SUBMENU_FILE_QUIT,
SUBMENU_FILE_END
// if adding a line, may need to increase SUBMENU_MAX_LINES above
};

enum
{
SUBMENU_EDIT_UNDO,
SUBMENU_EDIT_REDO,
SUBMENU_EDIT_CUT,
SUBMENU_EDIT_COPY,
SUBMENU_EDIT_PASTE,
SUBMENU_EDIT_CLEAR,
SUBMENU_EDIT_SELECT_ALL,
SUBMENU_EDIT_END
// if adding a line, may need to increase SUBMENU_MAX_LINES above
};

enum
{
SUBMENU_SEARCH_FIND,
SUBMENU_SEARCH_FIND_NEXT,
SUBMENU_SEARCH_END

};

enum
{
SUBMENU_COMPILE_TEST,
SUBMENU_COMPILE_COMPILE,
SUBMENU_COMPILE_COMPILE_LOCK,
//SUBMENU_COMPILE_TO_BCODE,
//SUBMENU_COMPILE_TO_ASM,
//SUBMENU_COMPILE_TO_ASM_CRUNCH,
//SUBMENU_COMPILE_CONVERT_BCODE,
//SUBMENU_COMPILE_IMPORT_BCODE,
SUBMENU_COMPILE_END
// if adding a line, may need to increase SUBMENU_MAX_LINES above

};

enum
{
OVERWINDOW_TYPE_NONE,
OVERWINDOW_TYPE_CLOSE, // attempting to close current tab without saving.
OVERWINDOW_TYPE_FIND,
OVERWINDOW_TYPES
};
#define OVERWINDOW_BUTTONS 3
#define OVERWINDOW_BUTTON_W scaleUI_x(FONT_BASIC,60)
#define OVERWINDOW_BUTTON_H scaleUI_y(FONT_BASIC,20)
enum
{
OVERWINDOW_BUTTON_TYPE_CLOSE_TAB, // closes tab without saving
OVERWINDOW_BUTTON_TYPE_NO, // closes overwindow without doing anything
OVERWINDOW_BUTTON_TYPE_FIND,
OVERWINDOW_BUTTON_TYPE_CANCEL_FIND
};

// TAB_NAME_LENGTH should leave space (on the screen) for a * to indicate an unsaved source file
// note that TAB_NAME_LENGTH is also used for the names on the import template list box in t_template.h
#define TAB_NAME_LENGTH 12

#define SEARCH_STRING_LENGTH 30

struct editorstruct
{
// int panel_x, panel_y, panel_w, panel_h;
 int text_width; // initialised in e_inter.c
 struct source_edit_struct source_edit [ESOURCES];

// int current_source_edit; // this is the index in the source_edit array, not the tab index

// ALLEGRO_BITMAP* sub_bmp;
// ALLEGRO_BITMAP* edit_sub_bmp;
// ALLEGRO_BITMAP* log_sub_bmp;

 int edit_window_x1;
// int edit_window_y1; - constant (EDIT_WINDOW_Y)
 int edit_window_w;
 int edit_window_h;
 int edit_window_lines;
 int edit_window_chars;
// int mlog_window_y;

// struct slider_struct scrollbar_v;
// struct slider_struct scrollbar_h;

 int key_delay;
 int special_key_being_pressed;
 int selecting;
 int cursor_flash;
 int already_pressed_cursor_key_etc; // is used to make sure cursor keys are only processed once per tick
 int ignore_previous_key_pressed; // if one key (other than ctrl, shift etc) is being pressed and another is pressed, this is set to the first key. The first key will then be ignored if still being pressed when the second key stops.
// int first_press = 0;
 int mouse_cursor_line; // the little line indicating where the cursor will go if mouse is clicked
 int mouse_cursor_pos;

 char clipboard [CLIPBOARD_SIZE];
 int clipboard_lines; // number of lines in clipboard. Is zero if just contains text within a line (so can't be used to tell whether the clipboard is empty - clipboard [0] will be '\0' if so)


 int undo_pos;
 //int undo_pos_base; // this is the bottom of the current undo stack.
 int undo_type [UNDO_STACK_SIZE];
 int undo_se [UNDO_STACK_SIZE]; // source edit
 int undo_start [UNDO_STACK_SIZE];
 int undo_size [UNDO_STACK_SIZE];
 int undo_cursor_line [UNDO_STACK_SIZE];
 int undo_cursor_pos [UNDO_STACK_SIZE];
 int undo_end_line [UNDO_STACK_SIZE]; // end values are just used for block pastes (so that undo can be done by just removing everything from cursor_line/pos to end_line/pos)
 int undo_end_pos [UNDO_STACK_SIZE];
 int undo_block_ends_with_full_line [UNDO_STACK_SIZE]; // these are only used for block deletions/insertions
 int undo_block_ends_at_line_start [UNDO_STACK_SIZE];
 int undone [UNDO_STACK_SIZE];
// when adding undo values, may need to add to delete_undo_pos_base()
 int undo_buffer_pos;
 int undo_buffer_base_pos; // first position in the buffer currently in use by an undo stack entry
 char undo_buffer [UNDO_BUFFER_SIZE];

// submenus (for File, Edit etc.)
 int submenu_open; // is -1 if no submenu is open
 int submenu_name_highlight; // highlighting of name of submenu (file, edit etc). Is -1 if none.
 int submenu_highlight; // highlighting within the submenu. is -1 if no submenu line is highlighted
 int submenu_x, submenu_y, submenu_w, submenu_h; // if submenu open, this describes its location
// struct submenustruct submenu [SUBMENUS];

// tabs
/*
 int tab_index [ESOURCES]; // contains an index to the source_edit_structs. Position in this array indicates position in the tab bar. -1 if not in use.
 int tab_type [ESOURCES]; // see TAB_TYPE enum above
 int tab_highlight; // -1 if no tab highlighted
 int current_tab; // should be the tab containing current_source_edit
 char tab_name [ESOURCES] [TAB_NAME_LENGTH];
 char tab_name_unsaved [ESOURCES] [TAB_NAME_LENGTH]; // same as tab_name but with an asterisk (and one char shorter if needed)
*/

 int current_source_edit_index;
// currently open template should be more global than editorstruct (as it relates to the template and design panels too)

// overwindow - e.g. "close without saving" query, find text
 int overwindow_type; // is 0 if no overwindow
 int overwindow_buttons;
 int overwindow_x, overwindow_y, overwindow_w, overwindow_h;
 int overwindow_button_x [OVERWINDOW_BUTTONS];
 int overwindow_button_y [OVERWINDOW_BUTTONS];
 int overwindow_button_type [OVERWINDOW_BUTTONS];

// text string search
 char search_string [SEARCH_STRING_LENGTH];

#ifdef DEBUG_MODE
 int debug_keys;
#endif

};


enum
{
STOKEN_TYPE_WORD,
STOKEN_TYPE_KEYWORD,
//STOKEN_TYPE_ASM_KEYWORD,
//STOKEN_TYPE_ASM_OPCODE,
STOKEN_TYPE_NUMBER,
STOKEN_TYPE_OPERATOR,
STOKEN_TYPE_STRING,
STOKEN_TYPE_ERROR,
STOKEN_TYPE_PUNCTUATION,
STOKEN_TYPE_COMMENT,
STOKEN_TYPE_PREPROCESSOR, // not actually an stoken type, but used to indicate syntax highlight colour for preprocessor directives (any uncommented line starting in #)
STOKEN_TYPES

};

enum
{
SCHAR_LETTER,
SCHAR_NUMBER,
SCHAR_OPERATOR,
SCHAR_PUNCTUATION,
SCHAR_QUOTE, // "
SCHAR_SPACE,
SCHAR_NULL, // \0
SCHAR_ERROR

};

#define EDIT_WINDOW_X scaleUI_x(FONT_BASIC,25)
#define EDIT_WINDOW_Y scaleUI_y(FONT_BASIC,50)
#define EDIT_LINE_H font[FONT_BASIC].height
#define EDIT_LINE_OFFSET 2
#define LOG_WINDOW_H scaleUI_y(FONT_BASIC,120)

#define LOG_LINE_LENGTH 120
#define LOG_LINES 40

#define LOG_LINE_HEIGHT (font[FONT_BASIC].height + 3)
#define LOG_LETTER_WIDTH font[FONT_BASIC].width

struct loglinestruct
{
 int used; // whether the line contains anything
 char text [LOG_LINE_LENGTH];
 int colour;
// The following are used to go to an error when the line is clicked:
 int source_player_index; // -1 means don't do anything if this line clicked on.
 int source_template_index;
 int source_line;

};


struct log_struct
{

 struct loglinestruct log_line [LOG_LINES];
 int lpos; // position in the log_line array
 int h_lines; // height in lines
 int w_letters; // width in letters
// int h_pixels; // height in pixels
// int w_pixels;
 int minimised; // whether it's minimised

 int window_pos;

 struct slider_struct scrollbar_v;

};

#define SOURCE_WINDOW_MARGIN scaleUI_y(FONT_BASIC, 3)

// types of output when the various commands in the build submenu are used:
enum
{
COMPILE_OUTPUT_TEST, // doesn't produce any output
COMPILE_OUTPUT_BCODE, // not implemented
COMPILE_OUTPUT_BCODE_TAB, // produces a new binary tab with bcode output
COMPILE_OUTPUT_ASM_TAB, // produces a new source tab with asm output
COMPILE_OUTPUT_ASM_TAB_CRUNCH, // produces a new source tab with asm output (with multiple asm instructions on each line)
COMPILE_OUTPUT_TEMPLATE, // produces bcode in target_bcode (which is then used for a template)
};



enum
{
FILE_TYPE_SOURCE,
FILE_TYPE_BINARY,
FILE_TYPE_TEMPLATE,
FILE_TYPE_ERROR
};

enum
{
OPEN_FAIL,
OPENED_SOURCE,
OPENED_BINARY,
OPENED_ALREADY
};


#endif
