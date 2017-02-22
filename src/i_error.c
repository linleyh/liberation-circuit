
#include <allegro5/allegro.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "i_console.h"
#include "m_input.h"

#include "g_misc.h"

// This file contains functions for building an error_string to send to the console

int error_source_program_type;
int error_source_index;
int error_source_player;

void start_error(int source_program_type, int source_index, int source_player)
{

 error_source_program_type = source_program_type;
 error_source_index = source_index;
 error_source_player = source_player;

}

// can assume input is a valid string
void error_string(const char* str)
{

// write_error_to_console(str, error_source_program_type, error_source_index, error_source_player);

}

void error_number(int num)
{

 char num_string [10];
 sprintf(num_string, "%i", num);

// write_error_to_console(num_string, error_source_program_type, error_source_index, error_source_player);

}

