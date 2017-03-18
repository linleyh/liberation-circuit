
#ifndef H_I_CONSOLE
#define H_I_CONSOLE

#include "e_slider.h"

#define CLINE_LENGTH 70
#define CLINES 48

//#define LINE_HEIGHT 15
#define CONSOLE_LINE_HEIGHT scaleUI_y(FONT_SQUARE,18)





#define PBOX_HEADER_HEIGHT scaleUI_y(FONT_SQUARE,15)
#define PBOX_W scaleUI_x(FONT_SQUARE,320)
#define PBOX_LINE scaleUI_y(FONT_SQUARE,12)
/*
struct proc_boxstruct
{
 int x1, y1, x2, y2;
 int button_highlight;
 int maximised;
 int button_x1, button_y1, button_x2, button_y2;
};*/


struct consolelinestruct
{
 int used; // whether the line contains anything
 int colour; // index in PRINT_COLS. value in this field should be bounds-checked (as it would have been taken from w.print_colour, which is bounds-checked)
 char text [CLINE_LENGTH];

 int source_index; // index of process (-1 if other source - may not currently be possible)
 timestamp source_core_created_timestamp; // if from a proc, this is its created_timestamp

 timestamp time_written;

};


struct consolestruct
{

// put basic parameters of the console here:

 int x, y;
 int h_lines; // height in lines
 int w_letters; // width in letters
 int h_pixels; // height in pixels
 int w_pixels;
 int line_highlight; // if the mouse is over a line in the console
 timestamp line_highlight_time; // game.total_time when mouse was over line_highlight

 int cpos; // position in the cline array
 int current_line_length;

 struct consolelinestruct cline [CLINES];

 int source_index; // index of player or proc that most recently wrote to console
 timestamp time_written; // time of most recent writing to console



};

enum
{
CONSOLE_GENERAL, // bottom left console - displays messages from user's processes etc
CONSOLE_SYSTEM, // top left console - special
CONSOLES
};

#define SYSTEM_CONSOLE_WIDTH_LETTERS 66

void init_consoles(void);
void setup_consoles(void);
void clear_console(int console_index);

//void program_write_to_console(const char* str, int source_program_type, int source_index, int source_player);
//void write_error_to_console(const char* str, int source_program_type, int source_index, int source_player);

void run_consoles(void);
void display_consoles_and_buttons(void);

void write_text_to_console(int console_index, int print_colour, int text_source, int source_core_created_timestamp, char* write_text);
void console_newline(int console_index, int print_colour);

void write_text_to_bubble(int core_index, timestamp print_timestamp, char* write_text);

void check_mouse_on_consoles_etc(int mouse_x, int mouse_y, int left_button);
void place_build_buttons(void);
void reset_build_queue_buttons_y1(int queue_length);

//void run_score_boxes(void);
//void display_score_boxes(void);

//void draw_proc_box(void);
//void place_proc_box(int x, int y, struct proc_struct* pr);
//void reset_proc_box_height(struct proc_struct* pr);

//void set_console_size_etc(int c, int new_w, int new_h, int font_index, int force_update);

#endif
