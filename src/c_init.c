#include <allegro5/allegro.h>
#include "m_config.h"
#include "g_header.h"
#include "c_header.h"

#include "g_misc.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "e_log.h"
#include "e_slider.h"
#include "e_header.h"
#include "e_files.h"

//#include "i_header.h"
#include "i_console.h"
#include "c_prepr.h"
#include "c_keywords.h"

extern struct cstatestruct cstate;
extern struct identifierstruct identifier [IDENTIFIERS];
/*
// call this once
void init_compiler_at_startup(void)
{


}*/

// call this each time the compiler is needed
// this function should not change templ at all (just refer to it)
int init_compiler(struct template_struct* templ, int compiler_mode)
{

 int i;

 cstate.src_line = 0;
 cstate.src_pos = 0;
 cstate.scode_pos = 0;
 cstate.expoint_pos = 0;
 cstate.error = 0;
 cstate.recursion_level = 0;
 cstate.just_returned = 0;
 cstate.reached_end_of_source = 0;
 cstate.recursion_level = 0;
 cstate.target_bcode = &templ->bcode;

 cstate.mem_pos = 0;

 cstate.compile_mode = compiler_mode;

 cstate.templ = templ;


 cstate.scode.text [0] = '\0';
 cstate.scode.text_length = 0;

	identifier[USER_IDENTIFIERS].type = CTOKEN_TYPE_NONE; // terminates the identifier list just after the end of the list of fixed compiler keywords

	for (i = USER_IDENTIFIERS; i < IDENTIFIERS; i ++)
	{
		identifier[i].type = CTOKEN_TYPE_NONE;
		identifier[i].address = -1;
		identifier[i].value = 0;
	}

// intercode:
 cstate.ic_pos = 0;
// + think about initialising intercode array (shouldn't really be needed if ic_pos is used properly)

 for (i = 0; i < EXPOINTS; i ++)
	{
		cstate.expoint[i].true_point_used = 0;
		cstate.expoint[i].false_point_used = 0;
	}


	return 1;
}

