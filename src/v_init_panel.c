#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "c_header.h"
#include "c_keywords.h"
#include "m_globvars.h"
#include "i_header.h"

#include "g_misc.h"

#include "i_input.h"
#include "i_view.h"
#include "i_display.h"
#include "i_buttons.h"
#include "t_template.h"
#include "e_slider.h"

#include "p_panels.h"
#include "d_draw.h"
#include "v_draw_panel.h"
#include "v_interp.h"

extern struct instruction_set_struct instruction_set [INSTRUCTIONS];
extern struct bcode_panel_state_struct bcp_state;
extern struct slider_struct slider [SLIDERS];
extern struct fontstruct font [FONTS];

/*

How will the line_index work?

- each entry in it is a line on the display
- it stores:
 - the corresponding bcode address (-1 means empty)
 - the source line

*/



extern struct template_debug_struct template_debug [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct identifierstruct identifier [IDENTIFIERS];

static void clear_template_debug(int player_index, int template_index);

// This is called from prepare_template_debug_from_cstate()
static void clear_template_debug(int player_index, int template_index)
{

 int i;
 struct template_debug_struct* tdb = &template_debug[player_index][template_index];

 for (i = 0; i < MEMORY_SIZE; i ++)
	{
		tdb->variable[i].name [0] = 0;
//		tdb->variable[i].address = 0;
	}
 for (i = 0; i < DEBUGGER_LABELS; i ++)
	{
		tdb->label[i].name [0] = 0;
		tdb->label[i].address = -1;
	}

	for (i = 0; i < DEBUGGER_LINES; i ++)
	{
		tdb->debugger_line[i].bcode_address = -1; // means empty
		tdb->debugger_line[i].source_line = SOURCE_LINE_EMPTY; // means empty or unknown
		tdb->debugger_line[i].special_value = -1; // meaning depends on value of source_line
	}

}


// This is called (with use_user_identifiers=1) when the compiler has finished compiling (not test compiling)
// it builds the template_debug_struct for the template
// it is also called from the multi-binary loading function with use_user_identifiers=0,
//  because multi-binaries don't have identifier information.
void prepare_template_debug(int player_index, int template_index, int use_user_identifiers)
{

 int i;
 int label_index = 0;
// int expoint_index = 0;
 struct template_debug_struct* tdb = &template_debug[player_index][template_index];
 struct template_struct* sts = &templ[player_index][template_index]; // source template struct

 clear_template_debug(player_index, template_index);

 int bcode_op_special_type [BCODE_MAX];

 for (i = 0; i < BCODE_MAX; i ++)
	{
		bcode_op_special_type [i] = 0;
	}

 if (use_user_identifiers)
	{
// USER_IDENTIFIERS is the start of user-defined identifiers (after all keywords etc)
 for (i = USER_IDENTIFIERS; i < IDENTIFIERS; i ++)
	{
		switch(identifier[i].type)
		{
		 case CTOKEN_TYPE_IDENTIFIER_USER_VARIABLE:
		 	if (identifier[i].address >= MEMORY_SIZE)
					break;
			 strncpy(tdb->variable[identifier[i].address].name, identifier[i].name, DEBUGGER_NAME_LENGTH);
			 break;
			case CTOKEN_TYPE_IDENTIFIER_LABEL:
				if (label_index >= DEBUGGER_LABELS)
					break;
			 strncpy(tdb->label[label_index].name, identifier[i].name, DEBUGGER_NAME_LENGTH);
			 tdb->label[label_index].address = identifier[i].address;
//			 fpr("\n adding label %i (%s) to address %i", label_index, tdb->label[label_index].name, tdb->label[label_index].address);
			 if (tdb->label[label_index].address > 0)
			  bcode_op_special_type [tdb->label[label_index].address] = SOURCE_LINE_LABEL;
			 label_index ++;
			 break;
//			case CTOKEN_TYPE_IDENTIFIER_CLASS:
//			 if (
		}

	}
	}

// now go through the bcode and index it:
 int debug_line_index = 0;
 int bcode_index = 0;
 int opcode;

// any code after this should not assume that the identifiers struct can be used.

 while(bcode_index <= BCODE_POS_MAX)
	{
//		fpr("\n %i %i", bcode_index, debug_line_index);
		opcode = sts->bcode.op[bcode_index];
		tdb->debugger_line[debug_line_index].bcode_address = bcode_index;
		tdb->debugger_line[debug_line_index].source_line = sts->bcode.src_line[bcode_index];

			switch(bcode_op_special_type [bcode_index])
			{
				case SOURCE_LINE_JUMP_TABLE_DEFAULT:
   		tdb->debugger_line[debug_line_index].source_line = SOURCE_LINE_JUMP_TABLE_DEFAULT;
   		bcode_index ++;
	 	  debug_line_index ++;
   		continue;
   	case SOURCE_LINE_JUMP_TABLE:
   		tdb->debugger_line[debug_line_index].source_line = SOURCE_LINE_JUMP_TABLE;
   		bcode_index ++;
	 	  debug_line_index ++;
   		continue;
   	case SOURCE_LINE_LABEL:
   	 {
//   	 	int incremented_debug_line_index = 0;
   	 	for (i = 0; i < DEBUGGER_LABELS; i ++)
						{
							if (tdb->label[i].address == -1)
								break; // end of list
							if (tdb->label[i].address == bcode_index)
							{
      		tdb->debugger_line[debug_line_index].bcode_address = bcode_index; // probably not used
      		tdb->debugger_line[debug_line_index].source_line = SOURCE_LINE_LABEL;
      		tdb->debugger_line[debug_line_index].special_value = i;
								debug_line_index ++;
     		 tdb->debugger_line[debug_line_index].bcode_address = bcode_index;
		      tdb->debugger_line[debug_line_index].source_line = sts->bcode.src_line[bcode_index];
//      		opcode = sts->bcode.op[bcode_index];
//								incremented_debug_line_index = 1;
							}
						}
//      if (!incremented_debug_line_index)
//							debug_line_index ++;
   	 }
// 		  bcode_index --;
   	 break; // now go and deal with the rest of the line
   	case SOURCE_LINE_STRING:
 		  bcode_index ++;
	 	  debug_line_index ++;
	 	  continue;
		 }

		bcode_index ++;
		debug_line_index ++;

		if (opcode >= 0
			&& opcode < INSTRUCTIONS)
		{

			if (opcode == OP_switchA)
			{
// switchA instruction should be followed by three operands: address of start of jump table, lowest case value, highest case value.
				int jump_table_start = sts->bcode.op[bcode_index];
				int jump_table_first_case = sts->bcode.op[bcode_index + 1];
				int jump_table_last_case = sts->bcode.op[bcode_index + 2];
				int jump_table_span = jump_table_last_case - jump_table_first_case;
				if (jump_table_start >= 1
					&& jump_table_start < BCODE_POS_MAX
					&& jump_table_first_case <= jump_table_last_case // should be true but just make sure
					&& jump_table_start + jump_table_span < BCODE_POS_MAX)
				{
					bcode_op_special_type [jump_table_start - 1] = SOURCE_LINE_JUMP_TABLE_DEFAULT; // default
					for (i = jump_table_start; i < jump_table_start + jump_table_span + 1; i ++)
					{
 					bcode_op_special_type [i] = SOURCE_LINE_JUMP_TABLE; // case
					}
				}
			}

 		bcode_index += instruction_set[opcode].operands;
 		if (opcode == OP_print
				|| opcode == OP_bubble)
			{
				tdb->debugger_line[debug_line_index].bcode_address = bcode_index;
				tdb->debugger_line[debug_line_index].source_line = SOURCE_LINE_STRING;

				int str_pos = 0;
				while(TRUE)
				{
					bcode_index ++;
					str_pos ++;
					if (sts->bcode.op[bcode_index] == 0
						|| str_pos >= STRING_MAX_LENGTH - 1
						|| bcode_index >= BCODE_POS_MAX)
					{
						bcode_index ++;
						break;
					}
				}
				debug_line_index ++;
			}
		}

	}

	tdb->total_lines_bcode_window = debug_line_index;


}


