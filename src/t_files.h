
#ifndef H_T_FILES
#define H_T_FILES

#include "c_header.h"

//int expected_program_type(int templ_type);
//void initialise_default_mission_templates(void);
//int load_known_file_into_template(int t, const char* file_path, int file_type);
//int open_file_into_template(int t);
//int finish_loading_source_file_into_template(struct source_struct* load_src, int t, const char* defined);
//void finish_loading_binary_file_into_template(int t);
//int copy_template_to_program(int t, struct programstruct* cl, int expect_program_type, int player);
//void import_template_from_list(int line, int t);

void save_template_file(int player_index);
void load_template_file(int player_index);
void load_default_templates(void);

#endif

