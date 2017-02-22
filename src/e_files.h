
#ifndef H_E_FILES
#define H_E_FILES

void init_editor_files(void);
int open_file_into_free_tab(void);
int new_empty_source_tab(void);
void give_source_edit_name_to_tab(int tab, struct source_edit_struct* se);
void open_file_into_current_source_edit(void);

//void save_source_edit_file(struct source_edit_struct* se);
void save_current_file(void);
void save_all_files(void);
void close_source_tab(int tab, int force_close);
int save_as(void);

//int open_file_into_source_or_binary(struct source_struct* src, struct bcode_struct* bcode, int check_editor_tabs);
int open_file_into_source_or_binary(struct source_struct* src, struct bcode_struct* bcode);
int get_file_type_from_name(const char* file_path);

int find_last_nonzero_bcode_op(struct bcode_struct* bcode);

#endif
