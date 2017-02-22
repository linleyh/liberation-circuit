
#ifndef H_E_EDITOR
#define H_E_EDITOR

#include "c_header.h"
#include "e_header.h"

void init_editor(void);

void run_editor(void);

void open_editor(void);
void close_editor(void);

int add_char(char added_char, int check_completion);

void update_source_lines(struct source_edit_struct* se, int sline, int lines);
void window_find_cursor(struct source_edit_struct* se);
int insert_empty_lines(struct source_edit_struct* se, int before_line, int lines);
void delete_lines(struct source_edit_struct* se, int start_line, int lines);
int is_something_selected(struct source_edit_struct* se);
int delete_selection(void);
struct source_edit_struct* get_current_source_edit(void);
void open_tab(int tab);
void change_tab(int new_tab);
int source_edit_to_source(struct source_struct* src, struct source_edit_struct* se);
int get_current_source_edit_index(void);

void open_overwindow(int ow_type);

void init_source_edit_struct(struct source_edit_struct* se);
void clear_source_edit_struct(struct source_edit_struct* se);
void clear_source_edit_text(struct source_edit_struct* se);
int source_to_editor(struct source_struct* src, int esource);

void flush_game_event_queues(void);

int source_line_highlight_syntax(struct source_edit_struct* se, int src_line, int in_a_comment);
int get_source_char_type(char read_source);
void open_template_in_editor(struct template_struct* tpl);

#endif
