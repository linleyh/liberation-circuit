
#ifndef H_G_MISC
#define H_G_MISC

#include <stdio.h>

void init_random_numbers(int grand_seed);
unsigned int grand(unsigned int max);
unsigned int irand(unsigned int max);
void error_call(void);

void wait_for_space(void);
void print_binary(int num);
void print_binary8(int num);
void print_binary32(int num);
void safe_exit(int exit_value);

FILE* open_file_from_standard_path(const char* basic_file_name, const char* mode);

#endif
