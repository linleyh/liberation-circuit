#include <allegro5/allegro.h>
#include "m_config.h"
#include "g_header.h"
#include "c_header.h"

#include "g_misc.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "e_log.h"
//#include "e_slider.h"
#include "e_header.h"
//#include "e_files.h"

//#include "i_header.h"
//#include "i_console.h"
//#include "c_init.h"
//#include "c_prepr.h"
//#include "c_lexer.h"
//#include "c_fix.h"
//#include "c_compile.h"



extern struct instruction_set_struct instruction_set [INSTRUCTIONS];

extern struct cstatestruct cstate;

extern struct identifierstruct identifier [IDENTIFIERS];

static int add_expoint_address_resolve(int resolve_type, int ep_index);
static int intercode_error_text(const char* error_text);
static int resolve_addresses(void);
static void write_bcode(s16b new_value);


int intercode_to_bcode(void)
{

 int i;

 for (i = 0; i < BCODE_MAX; i ++)
	{
		cstate.target_bcode->op [i] = OP_nop;
		cstate.target_bcode->src_line [i] = 0;
	}

// the end of the bcode is filled with stop instructions
	for (i = BCODE_POS_MAX; i < BCODE_MAX; i ++)
	{
		cstate.target_bcode->op [i] = OP_stop;
	}

	int intercode_length = cstate.ic_pos;
	cstate.bc_pos = 0;
	cstate.ic_pos = 0;
	cstate.resolve_pos = 0; // position in cstate.ic_address_resolve struct

 for (cstate.ic_pos = 0; cstate.ic_pos < intercode_length; cstate.ic_pos ++)
	{
		if (cstate.bc_pos >= BCODE_POS_MAX - 8)
		{
			return intercode_error_text("bcode too large");
		}
		switch(cstate.intercode[cstate.ic_pos].type)
		{
		 case IC_OP:
#ifdef SANITY_CHECK
    if (cstate.intercode[cstate.ic_pos].value [0]	< 0
					|| cstate.intercode[cstate.ic_pos].value [0]	>= INSTRUCTIONS)
				{
					fpr("\nError: c_generate.c: intercode_to_bcode(): invalid IC_OP instruction %i at intercode %i (source line %i)", cstate.intercode[cstate.ic_pos].value [0], cstate.ic_pos, cstate.intercode[cstate.ic_pos].src_line);
					error_call();
				}
#endif
    write_bcode(cstate.intercode[cstate.ic_pos].value [0]);
    if (instruction_set[cstate.intercode[cstate.ic_pos].value [0]].operands > 0)
				{
     write_bcode(cstate.intercode[cstate.ic_pos].value [1]);
				}
    if (instruction_set[cstate.intercode[cstate.ic_pos].value [0]].operands > 1)
				{
     write_bcode(cstate.intercode[cstate.ic_pos].value [2]);
				}
    break;
   case IC_OP_WITH_VARIABLE_OPERAND:
// This is like IC_OP but value [1] is an identifier index instead of a value
#ifdef SANITY_CHECK
    if (cstate.intercode[cstate.ic_pos].value [0]	< 0
					|| cstate.intercode[cstate.ic_pos].value [0]	>= INSTRUCTIONS)
				{
					fpr("\nError: c_generate.c: intercode_to_bcode(): invalid IC_OP_WITH_VARIABLE_OPERAND instruction %i at intercode %i (source line %i)", cstate.intercode[cstate.ic_pos].value [0], cstate.ic_pos, cstate.intercode[cstate.ic_pos].src_line);
					error_call();
				}
    if (identifier[cstate.intercode[cstate.ic_pos].value [1]].address < 0
					|| identifier[cstate.intercode[cstate.ic_pos].value [1]].address >= MEMORY_SIZE)
				{
					fpr("\nError: c_generate.c: intercode_to_bcode(): invalid IC_OP_WITH_VARIABLE_OPERAND operand (address %i) at intercode %i (source line %i)", identifier[cstate.intercode[cstate.ic_pos].value [1]].address, cstate.ic_pos, cstate.intercode[cstate.ic_pos].src_line);
					error_call();
				}	// unlikely to be possible as references to undeclared variables should have been caught during compilation stage.
#endif
    write_bcode(cstate.intercode[cstate.ic_pos].value [0]);
    write_bcode(identifier[cstate.intercode[cstate.ic_pos].value [1]].address);
    break;
   case IC_EXIT_POINT_TRUE:
				cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].true_point_bcode = cstate.bc_pos;
				break;
   case IC_EXIT_POINT_FALSE:
				cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].false_point_bcode = cstate.bc_pos;
				break;
			case IC_LABEL_DEFINITION:
			 identifier[cstate.intercode[cstate.ic_pos].value [0]].address = cstate.bc_pos;
			 break;
			case IC_GOTO_LABEL:
				if (identifier[cstate.intercode[cstate.ic_pos].value [0]].type != CTOKEN_TYPE_IDENTIFIER_LABEL)
   		return intercode_error_text("goto label not defined");
    write_bcode(OP_jump_num);
				if (identifier[cstate.intercode[cstate.ic_pos].value [0]].address != -1)
				{
     write_bcode(identifier[cstate.intercode[cstate.ic_pos].value [0]].address);
				}
				 else
					{
						if (!add_expoint_address_resolve(ADDRESS_RESOLVE_LABEL, cstate.intercode[cstate.ic_pos].value [0]))
							return 0;
					}
				break;
			case IC_IFFALSE_JUMP_TO_EXIT_POINT:
    write_bcode(OP_iffalse_jump);
				if (cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].false_point_bcode == -1)
				{
// exit point address not yet known, so must resolve it at the end of code generation:
					if (!add_expoint_address_resolve(ADDRESS_RESOLVE_EX_POINT_FALSE, cstate.intercode[cstate.ic_pos].value [0]))
						return 0;
				}
				 else
						write_bcode(cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].false_point_bcode); // address known
				cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].false_point_used = 1;
				break;
			case IC_IFTRUE_JUMP_TO_EXIT_POINT:
    write_bcode(OP_iftrue_jump);
				if (cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].true_point_bcode == -1)
				{
// exit point address not yet known, so must resolve it at the end of code generation:
					if (!add_expoint_address_resolve(ADDRESS_RESOLVE_EX_POINT_TRUE, cstate.intercode[cstate.ic_pos].value [0]))
						return 0;
				}
				 else
						write_bcode(cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].true_point_bcode); // address known
				cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].true_point_used = 1;
				break;
			case IC_JUMP_EXIT_POINT_TRUE:
    write_bcode(OP_jump_num);
				if (cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].true_point_bcode == -1)
				{
// exit point address not yet known, so must resolve it at the end of code generation:
					if (!add_expoint_address_resolve(ADDRESS_RESOLVE_EX_POINT_TRUE, cstate.intercode[cstate.ic_pos].value [0]))
						return 0;
				}
				 else
						write_bcode(cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].true_point_bcode); // address known
				cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].true_point_used = 1;
				break;
			case IC_JUMP_EXIT_POINT_FALSE:
    write_bcode(OP_jump_num);
				if (cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].false_point_bcode == -1)
				{
// exit point address not yet known, so must resolve it at the end of code generation:
					if (!add_expoint_address_resolve(ADDRESS_RESOLVE_EX_POINT_FALSE, cstate.intercode[cstate.ic_pos].value [0]))
						return 0;
				}
				 else
						write_bcode(cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].false_point_bcode); // address known
				cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].false_point_used = 1;
				break;
			case IC_NUMBER:
    write_bcode(cstate.intercode[cstate.ic_pos].value [0]);
    break;
   case IC_SWITCH:
    write_bcode(OP_switchA);
				if (!add_expoint_address_resolve(ADDRESS_RESOLVE_EX_POINT_TRUE, cstate.intercode[cstate.ic_pos].value [0]))
					return 0;
    write_bcode(cstate.intercode[cstate.ic_pos].value [1]);
    write_bcode(cstate.intercode[cstate.ic_pos].value [2]);
				cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].true_point_used = 1;
    break;
   case IC_JUMP_TABLE:
// this just writes a number (to be used by switch code), no instruction.
//   	cstate.target_bcode->op[cstate.bc_pos] = cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].true_point_bcode;
//    cstate.bc_pos ++;
				if (cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].true_point_bcode == -1)
				{
// exit point address not yet known, so must resolve it at the end of code generation:
					if (!add_expoint_address_resolve(ADDRESS_RESOLVE_EX_POINT_TRUE, cstate.intercode[cstate.ic_pos].value [0]))
						return 0;
				}
				 else
						write_bcode(cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].true_point_bcode); // address known
				cstate.expoint[cstate.intercode[cstate.ic_pos].value [0]].true_point_used = 1;
    break;

			default:
				fpr("\nError: c_generate.c: intercode_to_bcode(): invalid instruction %i at intercode %i (source line %i)", cstate.intercode[cstate.ic_pos].type, cstate.ic_pos, cstate.intercode[cstate.ic_pos].src_line);
				error_call();
				break; // should never happen

		}
	}

 if (!resolve_addresses())
		return 0;

	start_log_line(MLOG_COL_COMPILER);
	write_to_log("Bcode length ");
	write_number_to_log(cstate.bc_pos);
	write_to_log(" (");
	write_number_to_log(BCODE_MAX);
	write_to_log("). Memory used ");
	write_number_to_log(cstate.mem_pos);
	write_to_log(" (");
	write_number_to_log(MEMORY_SIZE);
	write_to_log(").");
	finish_log_line();

//	fpr("\n generation success! bc_pos %i ic_pos %i", cstate.bc_pos, cstate.ic_pos);

 return 1; // success!

}

static void write_bcode(s16b new_value)
{

	cstate.target_bcode->op[cstate.bc_pos] = new_value;
	cstate.target_bcode->src_line[cstate.bc_pos] = cstate.intercode[cstate.ic_pos].src_line;
//fpr("[%i:%i]", cstate.bc_pos, cstate.intercode[cstate.ic_pos].src_line);
	cstate.bc_pos ++;

}


static int add_expoint_address_resolve(int resolve_type, int ep_index)
{

#ifdef SANITY_CHECK
    if (resolve_type != ADDRESS_RESOLVE_LABEL // this type doesn't use exit points; ep_index is the label identifier or something (stupidly)
					&& (ep_index	< 0
					|| ep_index	>= EXPOINTS))
				{
					fpr("\nError: c_generate.c: add_expoint_address_resolve(): invalid exit point index %i at intercode %i (source line %i)", ep_index, cstate.ic_pos, cstate.intercode[cstate.ic_pos].src_line);
					error_call();
				}
#endif

	if (cstate.resolve_pos >= ADDRESS_RESOLUTION_ENTRIES - 1)
		return intercode_error_text("too many addresses to resolve"); // shouldn't realistically happen

 cstate.ic_address_resolution[cstate.resolve_pos].type = resolve_type;
 cstate.ic_address_resolution[cstate.resolve_pos].bcode_pos = cstate.bc_pos;
 cstate.ic_address_resolution[cstate.resolve_pos].value = ep_index;
 cstate.resolve_pos++;
 cstate.target_bcode->src_line[cstate.bc_pos] = cstate.intercode[cstate.ic_pos].src_line;
 cstate.bc_pos ++; // this bcode entry is ignored for now, but will be fixed later by resolve_addresses()
 return 1;

}


static int resolve_addresses(void)
{

 if (cstate.resolve_pos == 0)
		return 1; // this is possible as very simple programs may not have exit points

	int i;

	for (i = 0; i < cstate.resolve_pos; i ++)
	{
  switch(cstate.ic_address_resolution[i].type)
  {
		 case ADDRESS_RESOLVE_EX_POINT_TRUE:
		 	if (cstate.expoint[cstate.ic_address_resolution[i].value].true_point_bcode == -1)
		   return intercode_error_text("exit point (true) not defined?"); // probably shouldn't happen (may be a sanity check rather than a generation error)
			 cstate.target_bcode->op [cstate.ic_address_resolution[i].bcode_pos] = cstate.expoint[cstate.ic_address_resolution[i].value].true_point_bcode;
//			 cstate.expoint[cstate.ic_address_resolution[i].value].true_point_used = 1; - this may not be the right place to put this, as true_point_used probably needs to be set earlier. Not sure.
			 break;
		 case ADDRESS_RESOLVE_EX_POINT_FALSE:
		 	if (cstate.expoint[cstate.ic_address_resolution[i].value].false_point_bcode == -1)
		   return intercode_error_text("exit point (false) not defined?"); // probably shouldn't happen (may be a sanity check rather than a generation error)
			 cstate.target_bcode->op [cstate.ic_address_resolution[i].bcode_pos] = cstate.expoint[cstate.ic_address_resolution[i].value].false_point_bcode;
			 break;
			case ADDRESS_RESOLVE_LABEL:
				if (identifier[cstate.ic_address_resolution[i].value].address == -1)
		   return intercode_error_text("label not defined");
			 cstate.target_bcode->op [cstate.ic_address_resolution[i].bcode_pos] = identifier[cstate.ic_address_resolution[i].value].address;
			 break;
	 } // end switch resolution type
	} // end for i loop

 return 1;

}


static int intercode_error_text(const char* error_text)
{



     start_log_line(MLOG_COL_ERROR);
     write_to_log("Code generation error at line ");
     write_number_to_log(cstate.intercode[cstate.ic_pos].src_line + 1);
     write_to_log(".");
     finish_log_line();

     start_log_line(MLOG_COL_ERROR);
     write_to_log("Error: ");
     write_to_log(error_text);
     write_to_log(".");
     finish_log_line();

     cstate.error = CERR_INTERCODE;
//     error_call();
     return 0;
}




