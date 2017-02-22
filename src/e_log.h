
#ifndef H_E_LOG
#define H_E_LOG


enum
{
MLOG_COL_EDITOR, // messages from the editor
MLOG_COL_TEMPLATE, // messages about templates
MLOG_COL_COMPILER, // messages from the compiler/preprocess/etc
MLOG_COL_FILE, // messages from load/save game/gamefile functions (other than errors)
MLOG_COL_ERROR, // error message
MLOG_COL_WARNING, // warning
MLOG_COL_HELP, // help message printed by help.c
MLOG_COLS
};

//void init_log(int w_pixels, int h_pixels);
void init_log(void);
void display_log(void);
void reset_log(void);
void log_resized(void);

void write_to_log(const char* str);
void start_log_line(int mcol);
void set_log_line_source_position(int player_index, int template_index, int src_line);
void finish_log_line(void);

//void write_to_log(const char* str, int source, int source_line);
void write_line_to_log(char* str, int mcol);
//void start_log_line(int source, int source_line);
//void finish_log_line(void);

void write_number_to_log(int num);

void mouse_click_on_log_window(void);

#endif
