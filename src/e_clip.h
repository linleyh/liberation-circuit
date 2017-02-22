
#ifndef H_E_CLIP
#define H_E_CLIP

void copy_selection(void);
void paste_clipboard(void);

void init_undo(void);
void call_undo(void);
void call_redo(void);

void add_char_undo(char achar);
void delete_char_undo(char achar);
void backspace_char_undo(char achar);
void add_enter_undo(void);
void add_block_to_undo(int start_line, int start_pos, int end_line, int end_pos, int undo_type);
void add_undo_remove_enter(void);
void remove_closed_file_from_undo_stack(int se_index);

#endif
