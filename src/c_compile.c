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
#include "c_init.h"
#include "c_prepr.h"
#include "c_lexer.h"
#include "c_fix.h"
#include "c_compile.h"
#include "c_generate.h"
#include "c_keywords.h"

#include "g_method.h"
#include "g_method_core.h"
#include "g_method_std.h"
#include "g_method_uni.h"
#include "g_shapes.h"

#include "d_geo.h"

#include "t_template.h"

#include "v_interp.h"
#include "v_init_panel.h"

extern struct call_type_struct call_type [CALL_TYPES];
extern struct cmethod_call_type_struct cmethod_call_type [CMETHOD_CALL_TYPES];
extern struct mmethod_call_type_struct mmethod_call_type [MMETHOD_CALL_TYPES];
extern struct smethod_call_type_struct smethod_call_type [SMETHOD_CALL_TYPES];
extern struct umethod_call_type_struct umethod_call_type [UMETHOD_CALL_TYPES];

extern struct nshape_struct nshape [NSHAPES];

// opcode names are used in some debugging code (not currently activated)
// and may be used in an inline asm or something later
struct instruction_set_struct instruction_set [INSTRUCTIONS] =
{
 {"nop", 0}, // OP_nop,
 {"pushA", 0}, // OP_pushA,
 {"popB", 0}, // OP_popB,
 {"add", 0}, // OP_add,
 {"sub_BA", 0}, // OP_sub_BA,
 {"sub_AB", 0}, // OP_sub_AB,
 {"mul", 0}, // OP_mul,
 {"div_BA", 0}, // OP_div_BA,
 {"div_AB", 0}, // OP_div_AB,
 {"mod_BA", 0}, // OP_mod_BA,
 {"mod_AB", 0}, // OP_mod_AB,
 {"not", 0}, // OP_not, // ~
 {"and", 0}, // OP_and, // &
 {"or", 0}, // OP_or, // |
 {"xor", 0}, // OP_xor, // ^
 {"lsh_BA", 0}, // OP_lsh_BA, // <<
 {"lsh_AB", 0}, // OP_lsh_AB, // <<
 {"rsh_BA", 0}, // OP_rsh_BA, // >>
 {"rsh_AB", 0}, // OP_rsh_AB, // >>
 {"lnot", 0}, // OP_lnot, // logical not !
 {"setA_num", 1, {OPERAND_TYPE_NUMBER}}, // OP_setA_number,
 {"setA_mem", 1, {OPERAND_TYPE_MEMORY}}, // OP_setA_memory,
 {"copyA_to_mem", 1, {OPERAND_TYPE_MEMORY}}, // OP_getA_memory,
 {"push_num", 1, {OPERAND_TYPE_NUMBER}}, // OP_push_num,
 {"push_mem", 1, {OPERAND_TYPE_MEMORY}}, // OP_push_mem,
 {"incr_mem", 1, {OPERAND_TYPE_MEMORY}}, // OP_incr_mem,
 {"decr_mem", 1, {OPERAND_TYPE_MEMORY}}, // OP_decr_mem,
 {"jump_num", 1, {OPERAND_TYPE_BCODE_ADDRESS}}, // OP_jump_num,
 {"jumpA", 0}, // OP_jumpA
 {"comp_eq", 0}, // OP_comp_eq,
 {"comp_gr", 0}, // OP_comp_gr,
 {"comp_greq", 0}, // OP_comp_greq,
 {"comp_ls", 0}, // OP_comp_ls,
 {"comp_lseq", 0}, // OP_comp_lseq,
 {"comp_neq", 0}, // OP_comp_neq,
 {"iftrue_jump", 1, {OPERAND_TYPE_BCODE_ADDRESS}}, // OP_iftrue_jump,
 {"iffalse_jump", 1, {OPERAND_TYPE_BCODE_ADDRESS}}, // OP_iftrue_jump,

// special instructions used for array referencing and similar:
 {"mulA_num", 1, {OPERAND_TYPE_NUMBER}}, // OP_mulA_num
 {"addA_num", 1, {OPERAND_TYPE_NUMBER}}, // OP_addA_num
 {"derefA", 0}, // OP_derefA
 {"copyA_to_derefB", 0}, // OP_copyA_to_derefB
 {"incr_derefA", 0}, // OP_incr_derefA,
 {"decr_derefA", 0}, // OP_decr_derefA,
 {"copyAtoB", 0}, // OP_copyAtoB,
 {"deref_stack_toA", 0}, // OP_deref_stack_toA,

// used for gosub
 {"push_return_address", 0}, // OP_push_return_address,

 {"return_sub", 0}, // OP_return_sub,

 {"switchA", 3, {OPERAND_TYPE_BCODE_ADDRESS, OPERAND_TYPE_NUMBER, OPERAND_TYPE_NUMBER}}, // OP_switchA,

// process comparisons: (not sure these are used)
 {"pcomp_eq", 0}, // OP_pcomp_eq
 {"pcomp_neq", 0}, // OP_pcomp_eq

 {"print", 0}, // OP_print (actually kind of has operands),
 {"printA", 0}, // OP_printA
 {"bubble", 0}, // OP_bubble (actually kind of has operands),
 {"bubbleA", 0}, // OP_bubbleA

 {"call_object", 1, {OPERAND_TYPE_NUMBER}}, // OP_call_object,
 {"call_member", 1, {OPERAND_TYPE_NUMBER}}, // OP_call_member,
 {"call_core", 1, {OPERAND_TYPE_NUMBER}}, // OP_call_core,
 {"call_extern_member", 1, {OPERAND_TYPE_NUMBER}}, // OP_call_extern_member,
 {"call_extern_core", 1, {OPERAND_TYPE_NUMBER}}, // OP_call_extern_core,
 {"call_std", 1, {OPERAND_TYPE_NUMBER}}, // OP_call_std
 {"call_std_var", 2, {OPERAND_TYPE_NUMBER, OPERAND_TYPE_NUMBER}}, // OP_call_std_var
 {"call_uni", 1, {OPERAND_TYPE_NUMBER}}, // OP_call_uni
 {"call_class", 1, {OPERAND_TYPE_NUMBER}}, // OP_call_class,

 {"stop", 0}, // OP_stop,
 {"terminate", 0}, // OP_terminate,

};



enum
{
// BIND_NONE, // loosest - not used
BIND_EXPRESSION_START, // expression start binds most loosely
BIND_LOGICAL_AND, // &&
BIND_LOGICAL_OR, // ||
BIND_COMPARE, // < > <= >= == !=
BIND_BITWISE, // all bitwise operators
BIND_ADD, // + -
 BIND_MUL, // * / %
BIND_LOGICAL_NOT, // ! - don't think this is used (binding for ! is dealt with as a special case)

};

enum
{
STATEMENT_END_SEMICOLON, // expects a semicolon at end of statement, unless statement is a block ending in }
STATEMENT_END_BRACKET	// expects a closing bracket ) at end of statement, even if statement is a block also ending in }
};




struct cstatestruct cstate;

extern struct identifierstruct identifier [IDENTIFIERS];

static int next_statement(int exit_point_break, int exit_point_continue, int ending_punctuation);
static void add_intercode(int ic_type, int value1, int value2, int value3);
static int get_operator_binding_level(int ctoken_subtype);
static int is_ctoken_closing_punctuation(struct ctokenstruct* ctoken);

static int variable_assignment(struct ctokenstruct* ctoken);

static int next_expression(int exit_point, int bind_level);
static int next_expression_value(int exit_point, int binding);
static int next_expression_operator(struct ctokenstruct* ctoken_operator, int exit_point, int previous_operator_binding);
static int parse_process_expression(int statement_start);
static int parse_member_expression(int statement_start, int accept_object_method_call, int external_process_call);
static int parse_core_method_call(int cmethod_index, int statement_start, int external_process_call);
static int parse_std_method_call(int smethod_index);
static int parse_uni_method_call(int umethod_index);
static int parse_class_method_call(void);
static int parse_class_name_in_expression(int class_index);
static int parse_class_keyword_in_code(void);

static int parse_if(int exit_point_break, int exit_point_continue);
static int parse_for(void);
static int parse_while(void);
static int parse_do_while(void);
static int parse_goto_or_gosub(void);
static int parse_printf(int print_op, int printA_op);
static int parse_enum(void);
static int parse_switch(int exit_point_continue);
static int add_print_string(char* source_string, int string_length, int print_op);

static int allocate_exit_point(int type);
static int variable_declaration(void);
static int read_array_dimension_declaration(void);
static int get_array_element_address(int id_index);

struct template_struct compiled_template;

// this function runs and initialises the compiler.
// returns 1 on success, 0 on failure.
int compile(struct template_struct* templ, struct source_edit_struct* source_edit, int compiler_mode)
{

 write_line_to_log("Starting compiler.", MLOG_COL_COMPILER);

 compiled_template.source_edit = source_edit;
 compiled_template.active = 1;
 compiled_template.locked = templ->locked;
 cstate.source_edit = source_edit;

	init_compiler(&compiled_template, compiler_mode);

	if (!preprocess(source_edit)) // processes source_edit into cstate.scode
		return 0;

 if (!fix_template_design_from_scode()) // updates the design in templ from scode. If compiler_mode is test, doesn't actually update it.
		return 0;

// if (compiler_mode == COMPILE_MO???
 int retval;

 while(TRUE)
	{
		retval = next_statement(-1, -1, STATEMENT_END_SEMICOLON); // -1s mean break or continue gives an error
		if (retval == 0)
			return 0; // error
	 if (retval == 2)
			break; // finished
	}

	add_intercode(IC_OP, OP_stop, 0, 0);

 if (cstate.ic_pos >= INTERCODE_SIZE - 1)
		return comp_error_text("code generation failed (intermediate code size too large)", NULL); // shouldn't happen

 if (cstate.error != CERR_NONE)
		return 0;

 if (!intercode_to_bcode())
		return 0;

 // at this point can fail only if the mode is COMPILE_MODE_LOCK
 //  and there's a design problem with the template (e.g. component collision)
 int success = 1;
 int completed = 1;

 switch(cstate.compile_mode)
 {
  case COMPILE_MODE_TEST:

// test compile doesn't test the design. Maybe it should?
//  - if so, will need to make sure the previously called template design functions don't stop early in test compile mode (currently they do)

/*
// 		copy_template(templ, &compiled_template);
 		if (check_template_collisions(&compiled_template) != 0)
    write_line_to_log("Warning: process component collision.", MLOG_COL_WARNING);
 		if (check_move_objects_obstruction(&compiled_template) != 0)
    write_line_to_log("Warning: move object obstructed by process component.", MLOG_COL_WARNING);
 	 check_template_objects(&compiled_template, 0);
 	 calculate_template_cost_and_power(&compiled_template);
// - test compile only tests the source code, not the design.
//  - probably should test the design as well.*/
 	 break;
  case COMPILE_MODE_BUILD:
 		copy_template(templ, &compiled_template, (templ->locked == 0));
 		if (check_template_collisions(templ) != 0)
			{
    write_line_to_log("Warning: process component collision.", MLOG_COL_WARNING);
    completed = 0;
			}
 		if (check_move_objects_obstruction(templ) != 0)
			{
    write_line_to_log("Warning: move object obstructed by process component.", MLOG_COL_WARNING);
    completed = 0;
			}
			if (check_template_objects(templ, 1)) // returns 1 on error
				completed = 0;
 	 calculate_template_cost_and_power(templ);
   prepare_template_debug(templ->player_index, templ->template_index, 1); // ,1 means that the debug template will include identifier information
 	 break;
 	case COMPILE_MODE_LOCK:
 		copy_template(templ, &compiled_template, (templ->locked == 0));
 		if (check_template_collisions(templ) != 0)
			{
    write_line_to_log("Error: process component collision.", MLOG_COL_ERROR);
    success = 0;
			}
 		if (check_move_objects_obstruction(templ) != 0)
			{
    write_line_to_log("Error: move object obstructed by process component.", MLOG_COL_ERROR);
    success = 0;
			}
			if (check_template_objects(templ, 1)) // returns 1 on error
				success = 0;
 	 calculate_template_cost_and_power(templ);
   prepare_template_debug(templ->player_index, templ->template_index, 1); // ,1 means that the debug template will include identifier information
			break;
 }

 if (!success)
		completed = 0;

 if (completed)
  write_line_to_log("Compilation complete.", MLOG_COL_COMPILER);


// s16b test_mem [MEMORY_SIZE];

// fpr("\n\n*** testing execution\n");

// execute_bcode(NULL, &templ->bcode, test_mem);

 return success;

}

// Checks template objects for invalid details like an interface object on the core.
// Also checks for mistakes like interface object with no depth objects
// returns 1 if an error (not just a warning) found, 0 otherwise
int check_template_objects(struct template_struct* templ, int warning_or_error)
{
	int i,j,k;
	int process_has_object [OBJECT_TYPES];
//	int member_has_object [OBJECT_TYPES];
 int error_found = 0;

 for (k = 0; k < OBJECT_TYPES; k ++)
 {
	 process_has_object [k] = 0;
 }


	for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (templ->member[i].exists == 0)
			continue;

		if (check_template_member_objects(templ, i) == 1)
			error_found = 1;

//	 for (k = 0; k < OBJECT_TYPES; k ++)
//	 {
//		 member_has_object [k] = 0;
//	 }

		for (j = 0; j < nshape[templ->member[i].shape].links; j ++)
		{
			process_has_object [templ->member[i].object[j].type] ++;
		}

	}

	if (process_has_object [OBJECT_TYPE_HARVEST]
		&& !process_has_object [OBJECT_TYPE_STORAGE])
	{
  write_line_to_log("Warning: process has harvest object, but no storage object to store harvested data.", MLOG_COL_WARNING);
	}

	if (process_has_object [OBJECT_TYPE_ALLOCATE]
		&& !process_has_object [OBJECT_TYPE_STORAGE])
	{
  write_line_to_log("Warning: process has allocate object, but no storage object to store data for allocation.", MLOG_COL_WARNING);
	}
/*
	if (process_has_object [OBJECT_TYPE_INTERFACE]
		&& !process_has_object [OBJECT_TYPE_INTERFACE_DEPTH])
	{
  write_line_to_log("Warning: process has interface object, but no interface depth object.", MLOG_COL_WARNING);
	}*/

	return error_found;

}


// exit_point_break: break statements will jump to false
// exit_point_continue: continue statements will jump to true
// can be the same or different (for e.g. switch inside loop). Can be -1 (break/continue gives an error)
// returns:
//  0 on error
//  1 on success
//  2 on success, and finished file
static int next_statement(int exit_point_break, int exit_point_continue, int ending_punctuation)
{

 cstate.recursion_level ++; // this will be decremented later on (see comp_statement_success label) if the function returns successfully

 if (cstate.recursion_level > RECURSION_LIMIT)
  return comp_error(CERR_RECURSION_LIMIT_REACHED, NULL);

 struct ctokenstruct ctoken;
 struct ctokenstruct operator_ctoken;
// struct ctokenstruct ctoken_operator;
 int retval;
// int save_scode_pos, save_scode_pos2;

   if (!read_next(&ctoken))
			{
				if (cstate.reached_end_of_source)
					return 2; // finished
    return 0;
			}

   switch(ctoken.type)
   {

    case CTOKEN_TYPE_PUNCTUATION:
     if (ctoken.subtype == CTOKEN_SUBTYPE_SEMICOLON)
      goto dont_need_semicolon; // does nothing
     if (ctoken.subtype != CTOKEN_SUBTYPE_BRACE_OPEN) // a brace is the only other punctuation that's accepted at the start of a line
      return comp_error_text("unexpected punctuation or operator at statement start", NULL);
     while(TRUE)
     {
      if (accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_BRACE_CLOSE))
       goto dont_need_semicolon;
      retval = next_statement(exit_point_break, exit_point_continue, STATEMENT_END_SEMICOLON);
      if (retval == 0)
							return 0; // error
      if (retval == 2)
							return comp_error_text("reached end of source inside block of code", NULL);
// if retval == 1, just keep on going
     } // end code block loop
     goto dont_need_semicolon; // end case CTOKEN_TYPE_PUNCTUATION

    case CTOKEN_TYPE_IDENTIFIER_USER_VARIABLE:
// should be an assignment:
					if (!variable_assignment(&ctoken))
						return 0;
     break; // end case CTOKEN_TYPE_IDENTIFIER_USER_VARIABLE

    case CTOKEN_TYPE_IDENTIFIER_NEW:
    case CTOKEN_TYPE_IDENTIFIER_LABEL_UNDEFINED:
// an undefined identifier should be a label
     if (!accept_next(&operator_ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_COLON))
      return comp_error_text("syntax error at start of statement", &ctoken);
     identifier[ctoken.identifier_index].type = CTOKEN_TYPE_IDENTIFIER_LABEL;

     add_intercode(IC_LABEL_DEFINITION, ctoken.identifier_index, 0, 0);

// no semicolon here
					goto dont_need_any_punctuation; // end undefined labels

				case CTOKEN_TYPE_IDENTIFIER_C_KEYWORD:
				case CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD:
					switch(ctoken.identifier_index)
					{
					 case KEYWORD_C_INT:
					 	if (!variable_declaration())
								return 0;
							while (check_next(CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_COMMA))
							{
					 	 if (!variable_declaration())
								 return 0;
							}
						 break;
						case KEYWORD_C_IF:
							if (!parse_if(exit_point_break, exit_point_continue))
								return 0;
							goto dont_need_semicolon;
						case KEYWORD_C_FOR:
							if (!parse_for()) // loop resets break/continue exit points
								return 0;
							goto dont_need_semicolon;
						case KEYWORD_C_WHILE:
							if (!parse_while()) // loop resets break/continue exit points
								return 0;
							goto dont_need_semicolon;
						case KEYWORD_C_DO:
							if (!parse_do_while()) // loop resets break/continue exit points
								return 0;
							break;
						case KEYWORD_C_GOTO:
							if (!parse_goto_or_gosub())
								return 0;
							break;
						case KEYWORD_C_GOSUB:
							add_intercode(IC_OP, OP_push_return_address, 0, 0);
							if (!parse_goto_or_gosub())
								return 0;
							break;
						case KEYWORD_C_RETURN:
							add_intercode(IC_OP, OP_return_sub, 0, 0);
							break;
						case KEYWORD_C_PRINTF:
							if (!parse_printf(OP_print, OP_printA))
								return 0;
							break;
						case KEYWORD_C_BUBBLEF:
							if (!parse_printf(OP_bubble, OP_bubbleA))
								return 0;
							break;
						case KEYWORD_C_PROCESS:
							if (!parse_process_expression(1))
								return 0;
							break;
						case KEYWORD_C_COMPONENT:
							if (!parse_member_expression(1, 1, 0)) // calls a member/object method. can be an expression or a statement.
								return 0;
							break;
						case KEYWORD_C_CLASS:
							if (!parse_class_keyword_in_code()) // e.g. class[2].method_call()
								return 0;
							break;
						case KEYWORD_C_EXIT: // stop execution
							add_intercode(IC_OP, OP_stop, 0, 0);
							break;
						case KEYWORD_C_TERMINATE: // self-destruct
							add_intercode(IC_OP, OP_terminate, 0, 0);
							break;
						case KEYWORD_C_ENUM:
							if (!parse_enum())
								return 0;
							break;
						case KEYWORD_C_SWITCH:
							if (!parse_switch(exit_point_continue)) // switch resets break exit point but retains continue
								return 0;
							goto dont_need_semicolon;
						case KEYWORD_C_CASE:
							return comp_error_text("case outside of switch", &ctoken);
						case KEYWORD_C_DEFAULT:
							return comp_error_text("default outside of switch", &ctoken);
// remember not to return on success; break or goto dont_need_punctuation instead
      case KEYWORD_C_BREAK:
      	if (exit_point_break == -1)
								return comp_error_text("break outside loop or switch", &ctoken);
							add_intercode(IC_JUMP_EXIT_POINT_FALSE, exit_point_break, 0, 0);
							break;
      case KEYWORD_C_CONTINUE:
      	if (exit_point_continue == -1)
								return comp_error_text("continue outside loop", &ctoken);
							add_intercode(IC_JUMP_EXIT_POINT_TRUE, exit_point_continue, 0, 0);
							break;

					}
					break; // end case CTOKEN_TYPE_IDENTIFIER_C_KEYWORD/CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD

			case CTOKEN_TYPE_IDENTIFIER_CMETHOD:
  		if (!parse_core_method_call(identifier[ctoken.identifier_index].value,  // see c_keywords.c for a list of these in the identifier initialiser
																																1, 0))
					return 0;
				break;

			case CTOKEN_TYPE_IDENTIFIER_SMETHOD:
				if (!parse_std_method_call(identifier[ctoken.identifier_index].value))
					return 0;
				break;

			case CTOKEN_TYPE_IDENTIFIER_UMETHOD:
				if (!parse_uni_method_call(identifier[ctoken.identifier_index].value))
					return 0;
				break;

			case CTOKEN_TYPE_IDENTIFIER_CLASS:
// class method calls need to confirm that the class name is followed by a full stop
//  (this is done here because class names can be used in expressions without the full stop)
    if (!expect_punctuation(CTOKEN_SUBTYPE_FULL_STOP))
		   return comp_error_text("expected full stop and object method call after class name in statement", NULL);
// push the class index:
    add_intercode(IC_OP, OP_push_num, identifier[ctoken.identifier_index].value, 0);
				if (!parse_class_method_call())
					return 0;
				break;

   case CTOKEN_TYPE_IDENTIFIER_LABEL: // label for goto target
    return comp_error_text("label already defined", &ctoken);

   case CTOKEN_TYPE_IDENTIFIER_OMETHOD:
    return comp_error_text("object method must be called for an object or class", &ctoken);

   case CTOKEN_TYPE_IDENTIFIER_MMETHOD:
    return comp_error_text("component method must be called for a component", &ctoken);

   default:
    return comp_error_text("error at statement start", &ctoken);


   } // end of ctoken.type switch

 if (ending_punctuation == STATEMENT_END_SEMICOLON
		&& !expect_punctuation(CTOKEN_SUBTYPE_SEMICOLON))
//			return comp_error(CERR_EXPECTED_SEMICOLON, NULL);
			return comp_error_text("expected ; after statement", NULL);

dont_need_semicolon: // used for last statement in for loop header, which ends with ) rather than ;

// require closing bracket here
 if (ending_punctuation == STATEMENT_END_BRACKET
		&& !expect_punctuation(CTOKEN_SUBTYPE_BRACKET_CLOSE))
			return comp_error_text("expected ) after statement", NULL);

dont_need_any_punctuation:

 cstate.recursion_level --;
 return 1;

}


static int parse_if(int exit_point_break, int exit_point_continue)
{

  int exit_point = allocate_exit_point(EXPOINT_TYPE_BASIC);

  if (exit_point == -1)
   return 0;

  if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_OPEN))
			return comp_error_text("expected open bracket after if", NULL);
		if (!next_expression(exit_point, BIND_EXPRESSION_START))
			return 0;
  if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_CLOSE))
			return comp_error_text("expected closing bracket at end of if expression", NULL);

// so now we check whether the expression is true or false
   add_intercode(IC_IFFALSE_JUMP_TO_EXIT_POINT, exit_point, 0, 0);
// here's the "true" exit point for the expression (any exit jumps from || logical operators (that aren't inside sub-expressions) are set to here)
   add_intercode(IC_EXIT_POINT_TRUE, exit_point, 0, 0);
// here's the code that follows from the if statement
   if (!next_statement(exit_point_break, exit_point_continue, STATEMENT_END_SEMICOLON))
    return 0;
// at this point, assume we've reached the end of the block or statement that followed the if. Check for else:
   if (check_next(CTOKEN_TYPE_IDENTIFIER_C_KEYWORD, KEYWORD_C_ELSE))
   {
// first we create a new exit point.
    int avoid_else_exit_point = allocate_exit_point(EXPOINT_TYPE_BASIC);
    if (avoid_else_exit_point == -1)
     return 0;
// if control reaches the end of the conditional code because the if statement was true and the code has finished executing, we jump past the else code.
    add_intercode(IC_JUMP_EXIT_POINT_TRUE, avoid_else_exit_point, 0, 0); // this is an unconditional jump to the avoid_else exit point
 // this is the point where control will jump to if the if statement was false (as conditional_exit_point remains set from before):
    add_intercode(IC_EXIT_POINT_FALSE, exit_point, 0, 0);
    if (!next_statement(exit_point_break, exit_point_continue, STATEMENT_END_SEMICOLON)) // if the first thing comp_statement finds is {, it will call itself until it finds }
     return 0;
    add_intercode(IC_EXIT_POINT_TRUE, avoid_else_exit_point, 0, 0); // shouldn't be necessary to set the "false" exit point
   }
    else
// no else, so we just set the "false" exit point for the if statement and go back to comp_statement
     add_intercode(IC_EXIT_POINT_FALSE, exit_point, 0, 0);

 return 1; // finished!


}

static int parse_while(void)
{

// first we need an exit point:
 int loop_exit_point = allocate_exit_point(EXPOINT_TYPE_LOOP);
 if (loop_exit_point == -1)
		return 0;

// true exit point is before while expression (used when end of loop, or continue within loop, found)
	add_intercode(IC_EXIT_POINT_TRUE, loop_exit_point, 0, 0);

	if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_OPEN))
		return comp_error_text("expected ( after while", NULL);
	if (!next_expression(-1, BIND_EXPRESSION_START))
		return 0;
	if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_CLOSE))
		return comp_error_text("expected ) after while", NULL);

// result of while(expression) should be in A. If false, jump straight to end of loop:
	add_intercode(IC_IFFALSE_JUMP_TO_EXIT_POINT, loop_exit_point, 0, 0);
// otherwise, fall through to the looped statement:
 if (!next_statement(loop_exit_point, loop_exit_point, STATEMENT_END_SEMICOLON))
		return 0; // will treat a block as a single statement and parse the whole thing

// now jump back and evaluate the while condition again:
 add_intercode(IC_JUMP_EXIT_POINT_TRUE, loop_exit_point, 0, 0);

// false exit point is at end of loop statement/block (used for failed while condition, and also break within loop)
	add_intercode(IC_EXIT_POINT_FALSE, loop_exit_point, 0, 0);

	return 1;

}

static int parse_do_while(void)
{

/*

do...while loops need two exit points:
ep 1 - passed to next_statement:
 - true is before while expression/evaluation. Used for continues within loop.
 - false is after loop entirely. Used for breaks within loop.

ep 2: true is at start of loop (before do). Used where while expression evaluates to true.

*/


// first we need an exit point right at the start (only true is used):
 int main_exit_point = allocate_exit_point(EXPOINT_TYPE_BASIC);
 if (main_exit_point == -1)
		return 0;

 int conditional_exit_point = allocate_exit_point(EXPOINT_TYPE_LOOP);
 if (conditional_exit_point == -1)
		return 0;

// main exit point is before do (used when end of loop found and while() expression is true)
	add_intercode(IC_EXIT_POINT_TRUE, main_exit_point, 0, 0);

// now we parse the statement after do
// conditional_exit_point is the exit point used for continue and break within the statement.
 if (!next_statement(conditional_exit_point, conditional_exit_point, STATEMENT_END_SEMICOLON))
		return 0; // will treat a block as a single statement and parse the whole thing

// true conditional exit point is after the loop and before the while is evaluated (used for continue within loop)
	add_intercode(IC_EXIT_POINT_TRUE, conditional_exit_point, 0, 0);

// now check for while:
 if (!check_next(CTOKEN_TYPE_IDENTIFIER_C_KEYWORD, KEYWORD_C_WHILE))
  return comp_error_text("expected while after do loop", NULL);

	if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_OPEN))
		return comp_error_text("expected ( after while", NULL);
	if (!next_expression(-1, BIND_EXPRESSION_START))
		return 0;
	if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_CLOSE))
		return comp_error_text("expected ) after while", NULL);
//	if (!expect_punctuation(CTOKEN_SUBTYPE_SEMICOLON))
//		return comp_error(CERR_EXPECTED_SEMICOLON, NULL);

// result of while(expression) should be in A. If true, jump back to start of loop:
	add_intercode(IC_IFTRUE_JUMP_TO_EXIT_POINT, main_exit_point, 0, 0);
// otherwise, fall through to the next thing after the loop.

// false conditional exit point is after the loop (used for break within loop)
	add_intercode(IC_EXIT_POINT_FALSE, conditional_exit_point, 0, 0);

	return 1;

}


static int parse_for(void)
{

/*

for works like this:

for (statement W; expression X; statement Y)
{
 statement Z
}

compiles to:


W
ep1 true
if X(ep2) goto ep2 true
if !X(ep2) goto ep2 false
ep3 true
Y
goto ep1 true
ep2 true
Z (ep1)
goto ep3 true
ep1 false
ep2 false


*/


// first we need an exit point right at the start (only true is used):
 int main_exit_point = allocate_exit_point(EXPOINT_TYPE_LOOP);
 if (main_exit_point == -1)
		return 0;

 int conditional_exit_point = allocate_exit_point(EXPOINT_TYPE_BASIC);
 if (conditional_exit_point == -1)
		return 0;

 int statement_exit_point = allocate_exit_point(EXPOINT_TYPE_BASIC);
 if (statement_exit_point == -1)
		return 0;

// statement W
	if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_OPEN))
		return comp_error_text("expected ( after for", NULL);
 if (!next_statement(-1, -1, STATEMENT_END_SEMICOLON))
		return 0;

// exit point 1 true
 add_intercode(IC_EXIT_POINT_TRUE, main_exit_point, 0, 0);

// expression X
 if (!next_expression(conditional_exit_point, BIND_EXPRESSION_START))
		return 0;
	if (!expect_punctuation(CTOKEN_SUBTYPE_SEMICOLON))
		return comp_error_text("expected semicolon after expression", NULL);

// evaluate X:
 add_intercode(IC_IFTRUE_JUMP_TO_EXIT_POINT, conditional_exit_point, 0, 0);
 add_intercode(IC_IFFALSE_JUMP_TO_EXIT_POINT, conditional_exit_point, 0, 0);

// exit point 3 true
 add_intercode(IC_EXIT_POINT_TRUE, statement_exit_point, 0, 0);

// statement Y
 if (!next_statement(-1, -1, STATEMENT_END_BRACKET))
		return 0;
// now jump to ep 1 true (before	expression X evaluated)
 add_intercode(IC_JUMP_EXIT_POINT_TRUE, main_exit_point, 0, 0);

// exit point 2 true
 add_intercode(IC_EXIT_POINT_TRUE, conditional_exit_point, 0, 0);
// statement Z
 if (!next_statement(main_exit_point, main_exit_point, STATEMENT_END_SEMICOLON))
		return 0;
 add_intercode(IC_JUMP_EXIT_POINT_TRUE, statement_exit_point, 0, 0);

// final exit points:
 add_intercode(IC_EXIT_POINT_FALSE, main_exit_point, 0, 0); // reached from break within statement Z
 add_intercode(IC_EXIT_POINT_FALSE, conditional_exit_point, 0, 0); // reached from X evaluating to false

	return 1;

}

// for gosub, an OP_push_return_address should be written just before this function is called
static int parse_goto_or_gosub(void)
{

// goto is a little tricky because labels don't have to be declared or defined beforehand.

 struct ctokenstruct ctoken;

 if (!read_next(&ctoken))
		return 0;

	if (ctoken.type == CTOKEN_TYPE_IDENTIFIER_LABEL // previously defined
		|| ctoken.type == CTOKEN_TYPE_IDENTIFIER_LABEL_UNDEFINED) // previously used by a goto but not yet defined
	{
		add_intercode(IC_GOTO_LABEL, ctoken.identifier_index, 0, 0);
		return 1;
	}

	if (ctoken.type == CTOKEN_TYPE_IDENTIFIER_NEW)
	{
		identifier[ctoken.identifier_index].type = CTOKEN_TYPE_IDENTIFIER_LABEL_UNDEFINED;
		add_intercode(IC_GOTO_LABEL, ctoken.identifier_index, 0, 0);
		return 1;
	}

	return comp_error_text("goto/gosub must be followed by a label", &ctoken);

}

static int parse_enum(void)
{

	if (!expect_punctuation(CTOKEN_SUBTYPE_BRACE_OPEN))
	 return comp_error_text("expected { after enum", NULL);

	struct ctokenstruct ctoken;
	int enum_id_index;
	int enum_value = 0;

	while(TRUE)
	{
		if (!read_next(&ctoken))
			return 0;
		if (ctoken.type == CTOKEN_TYPE_PUNCTUATION
			&& ctoken.subtype == CTOKEN_SUBTYPE_BRACE_CLOSE)
			return 1; // finished (also checked for below)
		if (ctoken.type != CTOKEN_TYPE_IDENTIFIER_NEW)
			return comp_error_text("enum must be an unused identifier", &ctoken);
		enum_id_index = ctoken.identifier_index;
		identifier[enum_id_index].type = CTOKEN_TYPE_ENUM;
// now check for what comes right after enum
		if (!read_next(&ctoken))
			return 0;
		if (ctoken.type == CTOKEN_TYPE_OPERATOR_ASSIGN
			&& ctoken.subtype == CTOKEN_SUBTYPE_EQ)
		{
			if (!expect_constant(&ctoken))
				return comp_error_text("value assigned to enum must be a single constant number", &ctoken);
//	 	identifier[enum_id_index].value = ctoken.number_value;
			enum_value = ctoken.number_value;// + 1;
// now read in next thing , which should be , or }
		 if (!read_next(&ctoken))
			 return 0;
		}
		if (ctoken.type == CTOKEN_TYPE_PUNCTUATION)
		{
			switch(ctoken.subtype)
			{
			 case CTOKEN_SUBTYPE_COMMA:
			 	identifier[enum_id_index].value = enum_value;
			 	enum_value++;
			 	continue; // read next one
			 case CTOKEN_SUBTYPE_BRACE_CLOSE:
			 	identifier[enum_id_index].value = enum_value;
					return 1; // finished (also checked for above)
				default:
					return comp_error_text("syntax error in enum list", &ctoken);
			}
		}
		 else
				return comp_error_text("syntax error in enum list", &ctoken);
	} // back through the loop

};

#define SWITCH_CASES 32
#define SWITCH_JUMP_TABLE_MAX 32
// if either of these values is changed, need to change error messages below as well.

static int parse_switch(int exit_point_continue)
{

 struct ctokenstruct ctoken;
 int case_value [SWITCH_CASES];
 int case_epoint [SWITCH_CASES];
 int current_case_entry = 0;
 int lowest_case = 80000;
 int highest_case = -80000;
 int default_case = 0; // set to 1 if there's a default in the switch
 int switch_intercode;

 int i;

 int end_epoint = allocate_exit_point(EXPOINT_TYPE_BASIC); // this is the point at the end of the switch
 if (end_epoint == -1)
		return 0;
 int jump_table_epoint = allocate_exit_point(EXPOINT_TYPE_BASIC); // this is the point at the start of the jump table
 if (jump_table_epoint == -1)
		return 0;
 int default_epoint = allocate_exit_point(EXPOINT_TYPE_SWITCH);
 if (default_epoint == -1)
		return 0;

 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_OPEN))
		return comp_error_text("expected ( after switch", NULL);
	if (!next_expression(-1, BIND_EXPRESSION_START))
		return 0;
 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_CLOSE))
		return comp_error_text("expected ) after switch expression", NULL);

 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACE_OPEN))
		return comp_error_text("expected { after switch()", NULL);

	switch_intercode = cstate.ic_pos;
	if (switch_intercode >= INTERCODE_SIZE - 1)
		return 0; // add_intercode() finds this error and notifies user, but does not break compilation.
	add_intercode(IC_SWITCH, jump_table_epoint, 0, 0); // second and third operands will be corrected later (to lowest and highest values)

 while(TRUE)
	{
		if (!peek_next(&ctoken))
			return comp_error_text("reached end of source within switch?", NULL);
		if (ctoken.type == CTOKEN_TYPE_IDENTIFIER_C_KEYWORD)
		{
			switch(ctoken.identifier_index)
			{
				case KEYWORD_C_CASE:
			  if (!read_next(&ctoken)) // read in the ctoken just peeked at
				  return 0; // shouldn't fail
			  if (!expect_constant(&ctoken)) // read number
			   return comp_error_text("expected constant number after case", &ctoken);
			  if (current_case_entry >= SWITCH_CASES-1)
  					return comp_error_text("too many cases in switch (maximum currently 32)", &ctoken);
			  case_value [current_case_entry] = ctoken.number_value;
			  if (ctoken.number_value < lowest_case)
				  lowest_case = ctoken.number_value;
			  if (ctoken.number_value > highest_case)
				  highest_case = ctoken.number_value;
			  case_epoint [current_case_entry] = allocate_exit_point(EXPOINT_TYPE_SWITCH);
			  if (case_epoint [current_case_entry] == -1)
				  return 0;
			  if (!expect_punctuation(CTOKEN_SUBTYPE_COLON))
				  return comp_error_text("expected : after case value", NULL);
			  add_intercode(IC_EXIT_POINT_TRUE, case_epoint [current_case_entry], 0, 0);
			  current_case_entry++;
			  continue; // end case case
			 case KEYWORD_C_DEFAULT:
			  if (!read_next(&ctoken)) // read in the ctoken just peeked at
				  return 0; // shouldn't fail
			  if (default_case)
				  return comp_error_text("switch has more than one default?", &ctoken);
			  if (!expect_punctuation(CTOKEN_SUBTYPE_COLON))
				  return comp_error_text("expected : after default", NULL);
			  add_intercode(IC_EXIT_POINT_TRUE, default_epoint, 0, 0);
			  default_case = 1;
			  continue;
			 case KEYWORD_C_BREAK:
			  if (!read_next(&ctoken)) // read in the ctoken just peeked at
				  return 0; // shouldn't fail
			  add_intercode(IC_JUMP_EXIT_POINT_FALSE, end_epoint, 0, 0);
				 continue;
		 } // end keywords switch
		} // end keywords
		if (ctoken.type == CTOKEN_TYPE_PUNCTUATION
			&& ctoken.subtype == CTOKEN_SUBTYPE_BRACE_CLOSE)
		{
			if (!read_next(&ctoken)) // read in the ctoken just peeked at
				return 0; // shouldn't fail
			break; // finished!
		}
		if (!next_statement(end_epoint, exit_point_continue, STATEMENT_END_SEMICOLON))
			return 0;
	} // end switch statement loop

// add a break at the end of the switch code:
 add_intercode(IC_JUMP_EXIT_POINT_FALSE, end_epoint, 0, 0);

// now we need to assemble the jump table.

// first check that the switch isn't empty:
 if (highest_case == -80000) // this is impossible if there have been any cases (as -80000 is not a valid s16b value)
		return comp_error_text("switch without cases", NULL); // should this really be an error?

	int jump_table_size = (highest_case - lowest_case) + 1;

	if (jump_table_size >= SWITCH_JUMP_TABLE_MAX)
		return comp_error_text("difference between lowest and highest case is too great (max is 32)", NULL);

// the switch instruction needs to know the lowest and highest cases: (value [0] is the exit point at the start of the jump table)
	cstate.intercode[switch_intercode].value [1] = lowest_case;
	cstate.intercode[switch_intercode].value [2] = highest_case;

// default exit point is just before the start of the rest of the jump table:
	add_intercode(IC_JUMP_TABLE, default_epoint, 0, 0); // this will appear as a number in the bcode

	add_intercode(IC_EXIT_POINT_TRUE, jump_table_epoint, 0, 0);

 int jump_table_intercode_start = cstate.ic_pos;
 if (jump_table_intercode_start >= INTERCODE_SIZE + SWITCH_JUMP_TABLE_MAX + 2)
		return comp_error_text("not enough space in bcode for jump table", NULL);

// fill the jump table with jumps to the default:
	for (i = 0; i < jump_table_size; i ++)
	{
		add_intercode(IC_JUMP_TABLE, default_epoint, 0, 0);
	}

// current_case_entry++;

// fpr("\n cases %i", current_case_entry);

// now set up the jump table with the actual cases:
 for (i = 0; i < current_case_entry; i ++)
	{
//  fpr("\n jt %i jt_start %i setting intercode[%i] to %i", i, jump_table_intercode_start, jump_table_intercode_start + (case_value [i] - lowest_case), case_epoint [i]);
// check for duplicate cases (remember that case_value array isn't sorted at all)
		if (cstate.intercode[jump_table_intercode_start + case_value [i] - lowest_case].value [0] != default_epoint)
		{
			char duplicate_case_error [40];
			snprintf(duplicate_case_error, 40, "duplicate case value %i", case_value [i]);
			return comp_error_text(duplicate_case_error, NULL);
		}
		cstate.intercode[jump_table_intercode_start + (case_value [i] - lowest_case)].value [0] = case_epoint [i];
	}

// add the exit point for breaks within the switch:
 add_intercode(IC_EXIT_POINT_FALSE, end_epoint, 0, 0);
// finally, if no default, add a default exit point:
 if (default_case == 0)
  add_intercode(IC_EXIT_POINT_TRUE, default_epoint, 0, 0);

 return 1;

}


// print_op and printA_op are set to the print or bubble ops as appropriate
static int parse_printf(int print_op, int printA_op)
{
// assume that STRING_MAX_LENGTH >= BUBBLE_TEXT_LENGTH_MAX
 char raw_string [STRING_MAX_LENGTH];
 char target_string [STRING_MAX_LENGTH];
 raw_string [0] = '\0';
 target_string [0] = '\0';

 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_OPEN))
		return comp_error_text("expected open bracket after printf/bubble", NULL);
 if (!expect_punctuation(CTOKEN_SUBTYPE_QUOTES))
		return comp_error_text("expected open quote after printf/bubble", NULL);

	int read_char;
	int raw_string_length = 0;

	while(TRUE)
	{
		read_char = c_get_next_char_from_scode();
		if (read_char == REACHED_END_OF_SCODE)
 		return comp_error_text("reached end of source inside string", NULL);
		if (read_char == 0)
 		return comp_error_text("found null terminator inside string?", NULL);

  if (raw_string_length >= STRING_MAX_LENGTH - 2) // should probably check this against BUBBLE_TEXT_LENGTH_MAX if relevant...
 		return comp_error_text("string too long", NULL);

 	if (read_char == '"')
 	 break;
 	raw_string [raw_string_length] = read_char;
  raw_string_length++;
	};

 raw_string [raw_string_length] = '\0';

// Now we generator intercode, checking for format (currently only %i and %%)
 int i;
 int target_string_pos = 0;

 for (i = 0; i < raw_string_length; i ++)
	{
		if (raw_string [i] == '%')
		{
			if (raw_string [i+1] == '%')
				i++; // only read one of the % chars
				 else
					{
						if (raw_string [i+1] == 'i')
						{
							i += 1; // += 2
// if there is something in target_string_pos that needs to be printed, write intercode to print it:
							if (target_string_pos > 0)
							{
								if (!add_print_string(target_string, target_string_pos, print_op))
									return 0;
								target_string_pos = 0;
							}
							// now read expression after end of string:
       if (check_next(CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_BRACKET_CLOSE))
 		     return comp_error_text("not enough arguments for format", NULL);
 		    if (!expect_punctuation(CTOKEN_SUBTYPE_COMMA))
 		     return comp_error_text("expected comma in format argument list", NULL);
 		    if (!next_expression(-1, BIND_EXPRESSION_START))
								return 0;
						 add_intercode(IC_OP, printA_op, 0, 0); // prints contents of register A (which should have been set as result of parsing expression)
						 target_string_pos = 0;
//						 i--;
						 continue;
						}
						 else
								return comp_error_text("unrecognised format specifier (only %i currently supported)", NULL);
					}
		} // end if raw_string [i] == '%'
		if (raw_string [i] == '\\')
		{
			if (raw_string [i+1] == '\\')
				i++; // only read one of the \ chars
				 else
					{
						if (raw_string [i+1] == 'n') // currently \n and \\ are the only ones accepted
						{
							i += 1; // += 2
		     target_string [target_string_pos] = '\n';
		     target_string_pos++;
						 continue;
						}
						 else
								return comp_error_text("unrecognised escape sequence (only \\\\ and \\n currently supported)", NULL);
					}
		}
		target_string [target_string_pos] = raw_string [i];
		target_string_pos++;
	}

	if (target_string_pos > 0)
	{
		if (!add_print_string(target_string, target_string_pos, print_op))
			return 0;
	}


 if (check_next(CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_COMMA))
  return comp_error_text("too many arguments for format", NULL);

// if (!expect_punctuation(CTOKEN_SUBTYPE_QUOTES))
//		return comp_error_text("expected closing quote at end of printf statement", NULL);
 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_CLOSE))
		return comp_error_text("expected closing bracket after printf/bubble (too many arguments?)", NULL);
// if (!expect_punctuation(CTOKEN_SUBTYPE_SEMICOLON))
//		return comp_error_text("expected semicolon at end of printf", NULL);

 return 1;

}

// source_string shouldn't be null terminated within string_length (parse_printf checks for this)
// formatting specifiers are ignored
static int add_print_string(char* source_string, int string_length, int print_op)
{
//fpr("\n*** add_print_string line %i [%s] length %i", cstate.src_line, source_string, string_length);
	add_intercode(IC_OP, print_op, 0, 0);

	int i;

	for (i = 0; i < string_length; i ++)
	{
		add_intercode(IC_NUMBER, source_string[i], 0, 0);
	}
//	char temp_string [100];
//	snprintf(temp_string, string_length, source_string);
//fpr("\n add_string<%s>", temp_string);
// terminate string with zero:
	add_intercode(IC_NUMBER, 0, 0, 0);

	if (cstate.error != CERR_NONE)
		return 0;

	return 1;

}

/*

Syntax for std, core, member and object method calls:

standard methods:
- just call them directly, e.g.
a = hypot(b, c);

core methods:
- for self, call them directly:
a = get_core_x();
a = get_group_x();
- can also call indirectly with index -1:
a = process[-1].get_x();
- for processes in process memory:
a = process[1].get_x();

also see below for core comparisons:

member methods:
- for self, just refer to member:
a = member[1].get_member_x();
- can also call indirectly with core index -1:
a = process[-1].member[1].get_member_x();
- for processes in process memory:
a = process[1].member[0].get_member_x();

object methods:
- follows the above:
a = member[1].object[2].get_angle();
** a = process[-1].member[1].object[2].get_angle();
** a = process[1].member[1].object[2].get_angle();
 ** although probably don't allow external object calls.

parsing of core methods is slightly complicated by the possible use of process references in comparison expressions, e.g.
if (process[1] == process[3]) {...}
 *** actually no, don't do this - do comparisons etc through std methods (compare_process(a,b) etc)


this compiles to:
setA_num 1
pushA
setA_num 3
popB
pcomp_eq
iffalse_jump <to exit point of if statement>


*/

// call this just after the "process" keyword is encountered in an expression
//  - unlike "component", "process" should not occur at the start of a statement.
static int parse_process_expression(int statement_start)
{

	struct ctokenstruct ctoken;

	if (!expect_punctuation(CTOKEN_SUBTYPE_SQUARE_OPEN))
		return comp_error_text("expected [ after 'process'", NULL);

	if (!next_expression(-1, BIND_EXPRESSION_START))
		return 0;
 add_intercode(IC_OP, OP_pushA, 0, 0);

	if (!expect_punctuation(CTOKEN_SUBTYPE_SQUARE_CLOSE))
		return comp_error_text("expected ] after process index", NULL);
	if (!expect_punctuation(CTOKEN_SUBTYPE_FULL_STOP))
		return comp_error_text("expected full stop after process index", NULL);

	if (!read_next(&ctoken))
		return 0;
// There are the following possibilities here:
//  - process[].core_method()
//  - process[].component[].component_method()

 if (ctoken.type == CTOKEN_TYPE_IDENTIFIER_CMETHOD)
		return parse_core_method_call(identifier[ctoken.identifier_index].value,  // see c_keywords.c for a list of these in the identifier initialiser
																																statement_start, 1);

 if (ctoken.type == CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD
		&& ctoken.identifier_index == KEYWORD_C_OBJECT)
		return parse_member_expression(statement_start, 0, 1); // 0 means an object method call is not accepted.

 return comp_error_text("expected process method or component reference after process", &ctoken);


}

static int parse_core_method_call(int cmethod_index, int statement_start, int external_process_call)
{

		int i;

	 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_OPEN))
 		return comp_error_text("expected opening bracket after process method", NULL);

		if (cmethod_call_type[cmethod_index].parameters > 0)
		{
			for (i = 0; i < cmethod_call_type[cmethod_index].parameters; i++)
			{
 	  if (!next_expression(-1, BIND_EXPRESSION_START))
		   return 0;
		  add_intercode(IC_OP, OP_pushA, 0, 0);
				if (i < cmethod_call_type[cmethod_index].parameters - 1)
				{
					if (!expect_punctuation(CTOKEN_SUBTYPE_COMMA))
					 return comp_error_text("expected comma after process method parameter", NULL);
				}
			}
		}

  if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_CLOSE))
		{
 		if (cmethod_call_type[cmethod_index].parameters == 0)
	   return comp_error_text("expected closing bracket (this process method has no parameters)", NULL);

	  return comp_error_text("expected closing bracket after process method parameters", NULL);
		}
// finally, add the call and the call method type:
  if (external_process_call)
   add_intercode(IC_OP, OP_call_extern_core, cmethod_index, 0); // could test for process[-1] and optimise it to self core method call
    else
     add_intercode(IC_OP, OP_call_core, cmethod_index, 0);

  if (statement_start)
			comp_warning_text("process method call with no effect? (return value unused)"); // core methods are read only and shouldn't be entire statements

  return 1;

}

// call this just after the "component" keyword is encountered in an expression
//  - also when it's found at the start of a statement
static int parse_member_expression(int statement_start, int accept_object_method_call, int external_process_call)
{

	struct ctokenstruct ctoken;

	if (!expect_punctuation(CTOKEN_SUBTYPE_SQUARE_OPEN))
		return comp_error_text("expected [ after 'component'", NULL);

	if (!next_expression(-1, BIND_EXPRESSION_START))
		return 0;
 add_intercode(IC_OP, OP_pushA, 0, 0);

	if (!expect_punctuation(CTOKEN_SUBTYPE_SQUARE_CLOSE))
		return comp_error_text("expected ] after component index", NULL);
	if (!expect_punctuation(CTOKEN_SUBTYPE_FULL_STOP))
		return comp_error_text("expected full stop after component index", NULL);

	if (!read_next(&ctoken))
		return 0;

	if (ctoken.type == CTOKEN_TYPE_IDENTIFIER_MMETHOD)
	{
	 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_OPEN))
 		return comp_error_text("expected opening bracket after component method", NULL);
		int mmethod_index = identifier[ctoken.identifier_index].value; // see c_keywords.c for a list of these in the identifier initialiser
		int i;
		if (mmethod_call_type[mmethod_index].parameters > 0)
		{
			for (i = 0; i < mmethod_call_type[mmethod_index].parameters; i++)
			{
 	  if (!next_expression(-1, BIND_EXPRESSION_START))
		   return 0;
		  add_intercode(IC_OP, OP_pushA, 0, 0);
				if (i < mmethod_call_type[mmethod_index].parameters - 1)
				{
					if (!expect_punctuation(CTOKEN_SUBTYPE_COMMA))
					 return comp_error_text("expected comma after component method parameter", NULL);
				}
			}
		}
	 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_CLOSE))
		{
 		if (mmethod_call_type[mmethod_index].parameters == 0)
	   return comp_error_text("expected closing bracket (this component method has no parameters)", NULL);

 		return comp_error_text("expected closing bracket after component method parameters", NULL);
		}
// finally, add the call and the call method type:
  if (external_process_call)
   add_intercode(IC_OP, OP_call_extern_member, mmethod_index, 0);
    else
     add_intercode(IC_OP, OP_call_member, mmethod_index, 0);
  if (statement_start)
			comp_warning_text("component method call with no effect?");
		return 1;
	}

	if (ctoken.type == CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD
		&& ctoken.identifier_index == KEYWORD_C_OBJECT)
	{
// object methods.
// These are not allowed if the component reference is part of a process expression:
  if (!accept_object_method_call)
			return comp_error_text("can't call an object method here", &ctoken);
// First, read which object:
	 if (!expect_punctuation(CTOKEN_SUBTYPE_SQUARE_OPEN))
 		return comp_error_text("expected [ after 'object'", NULL);
 	if (!next_expression(-1, BIND_EXPRESSION_START))
		 return 0;
  add_intercode(IC_OP, OP_pushA, 0, 0);
	 if (!expect_punctuation(CTOKEN_SUBTYPE_SQUARE_CLOSE))
 		return comp_error_text("expected ] after object index", NULL);
 	if (!expect_punctuation(CTOKEN_SUBTYPE_FULL_STOP))
		 return comp_error_text("expected full stop after object index", NULL);
// (TO DO: if component and object indices are constants, should be able to confirm that this method is valid for the known object type)
// now expect an object method:
  if (!read_next(&ctoken))
			return 0;
		if (ctoken.type != CTOKEN_TYPE_IDENTIFIER_OMETHOD)
			return comp_error_text("expected object method after full stop", &ctoken);
	 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_OPEN))
 		return comp_error_text("expected opening bracket after object method", NULL);
		int omethod_index = identifier[ctoken.identifier_index].value; // see c_keywords.c for a list of these in the identifier initialiser
		int i;
		if (call_type[omethod_index].parameters > 0)
		{
			for (i = 0; i < call_type[omethod_index].parameters; i++)
			{
 	  if (!next_expression(-1, BIND_EXPRESSION_START))
		   return 0;
		  add_intercode(IC_OP, OP_pushA, 0, 0);
				if (i < call_type[omethod_index].parameters - 1)
				{
					if (!expect_punctuation(CTOKEN_SUBTYPE_COMMA))
					 return comp_error_text("expected comma after object method parameter", NULL);
				}
			}
		}
	 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_CLOSE))
		{
			if (call_type[omethod_index].parameters == 0)
 		 return comp_error_text("expected closing bracket (this object method has no parameters)", NULL);

 		return comp_error_text("expected closing bracket after object method parameters", NULL);
		}
// finally, add the call and the call method type:
  add_intercode(IC_OP, OP_call_object, omethod_index, 0);
  return 1;
	}

 return comp_error_text("expected component method or object reference after component", &ctoken);

}


static int parse_std_method_call(int smethod_index)
{
		int i;

	 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_OPEN))
 		return comp_error_text("expected opening bracket after std method", NULL);

		if (smethod_call_type[smethod_index].parameters > 0)
		{
			for (i = 0; i < smethod_call_type[smethod_index].parameters; i++)
			{
 	  if (!next_expression(-1, BIND_EXPRESSION_START))
		   return 0;
		  add_intercode(IC_OP, OP_pushA, 0, 0);
				if (i < smethod_call_type[smethod_index].parameters - 1)
				{
					if (!expect_punctuation(CTOKEN_SUBTYPE_COMMA))
					 return comp_error_text("expected comma after std method parameter", NULL);
				}
			}
		}


		if (smethod_call_type[smethod_index].parameters < 0) // variable number of parameters - number in smethod_call_type.parameters is minimum*-1
		{
			i = 0;
			while(TRUE)
			{
// must have at least one parameter
 	  if (!next_expression(-1, BIND_EXPRESSION_START))
		   return 0;
		  add_intercode(IC_OP, OP_pushA, 0, 0);
		  i ++;
 	  if (check_next(CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_BRACKET_CLOSE))
 	   break;
				if (!expect_punctuation(CTOKEN_SUBTYPE_COMMA))
				 return comp_error_text("expected comma or bracket after parameter in variable-length call", NULL);
 			if (i > SMETHOD_VARIABLE_PARAMS_MAX)
	 			return comp_error_text("too many parameters in variable-length call", NULL);
			}
			if (i < smethod_call_type[smethod_index].parameters * -1)
				return comp_error_text("too few parameters in variable-length call", NULL);

   add_intercode(IC_OP, OP_call_std_var, smethod_index, i);

		}
		 else
			{
  	 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_CLOSE))
				{
    		if (smethod_call_type[smethod_index].parameters == 0)
 	  	  return comp_error_text("expected closing bracket (this std method has no parameters)", NULL);
 	  	   else
 	  	    return comp_error_text("expected closing bracket after std method parameters", NULL);
				}

// finally, add the call and the call method type:
    add_intercode(IC_OP, OP_call_std, smethod_index, 0);

			}


//  if (statement_start)
//			comp_warning_text("std method call with no effect? (return value unused)"); // core methods are read only and shouldn't be entire statements

// could check for std method calls with no effect here... (some of them have side effects, though)

  return 1;

}


static int parse_uni_method_call(int umethod_index)
{
		int i;

	 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_OPEN))
 		return comp_error_text("expected opening bracket after uni method", NULL);

		if (umethod_call_type[umethod_index].parameters > 0)
		{
			for (i = 0; i < umethod_call_type[umethod_index].parameters; i++)
			{
 	  if (!next_expression(-1, BIND_EXPRESSION_START))
		   return 0;
		  add_intercode(IC_OP, OP_pushA, 0, 0);
				if (i < umethod_call_type[umethod_index].parameters - 1)
				{
					if (!expect_punctuation(CTOKEN_SUBTYPE_COMMA))
					 return comp_error_text("expected comma after uni method parameter", NULL);
				}
			}
		}

	 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_CLOSE))
		{
 		if (umethod_call_type[umethod_index].parameters == 0)
 	  return comp_error_text("expected closing bracket (this std method has no parameters)", NULL);
 	   else
 	    return comp_error_text("expected closing bracket after std method parameters", NULL); // currently no different between uni and std methods
		}
// finally, add the call and the call method type:
  add_intercode(IC_OP, OP_call_uni, umethod_index, 0);

//  if (statement_start)
//			comp_warning_text("std method call with no effect? (return value unused)"); // core methods are read only and shouldn't be entire statements

// could check for std method calls with no effect here... (some of them have side effects, though)

  return 1;

}

// call this if the "class" keyword is found in a statement or expression
// it allows a class to be called by index (which can be a variable) rather than by name
// e.g. class[2].set_power(0);
// It isn't called for class declarations in process headers (see c_fix.c for that)
static int parse_class_keyword_in_code(void)
{

 if (!expect_punctuation(CTOKEN_SUBTYPE_SQUARE_OPEN))
		return comp_error_text("expected opening square bracket after class keyword", NULL);
// read the index expression:
	if (!next_expression(-1, BIND_EXPRESSION_START))
		return 0;
// push the index (so a following class method call can find it on the stack)
 add_intercode(IC_OP, OP_pushA, 0, 0);

 if (!expect_punctuation(CTOKEN_SUBTYPE_SQUARE_CLOSE))
		return comp_error_text("expected closing square bracket after class index expression", NULL);
 if (!expect_punctuation(CTOKEN_SUBTYPE_FULL_STOP))
		return comp_error_text("expected full stop after class index expression", NULL);

 return parse_class_method_call();

}


// call this when a class name is found in an expression.
// it will interpret the name as a constant equal to the class's index
//  unless it's followed by a full stop, which means a class method call
static int parse_class_name_in_expression(int class_index)
{

 if (check_next(CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_FULL_STOP))
	{
// class method call code assumes that the class index has just been pushed to the stack:
  add_intercode(IC_OP, OP_push_num, class_index, 0);
		return parse_class_method_call();
	}

// otherwise, set A to the class index
	add_intercode(IC_OP, OP_setA_num, class_index, 0);
	return 1;

}


// call this after:
//  - finding the full stop after the class name in a class method call
//  - and then pushing the class index
static int parse_class_method_call(void)
{

 struct ctokenstruct ctoken;

// fpr("\n pcmc ci %i", class_index);

// next thing after the full stop should be the method name:
 if (!read_next(&ctoken))
		return comp_error(CERR_READ_FAIL, NULL);
	if (ctoken.type != CTOKEN_TYPE_IDENTIFIER_OMETHOD)
		return comp_error_text("expected object method after class reference", &ctoken);

		int omethod_index = identifier[ctoken.identifier_index].value; // see c_keywords.c for a list of these in the identifier initialiser

//  fpr(" omi %i (params %i)", omethod_index, call_type[omethod_index].parameters);

	 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_OPEN))
 		return comp_error_text("expected opening bracket after object method", NULL);

		int i;

		if (call_type[omethod_index].parameters > 0)
		{
			for (i = 0; i < call_type[omethod_index].parameters; i++)
			{
 	  if (!next_expression(-1, BIND_EXPRESSION_START))
		   return 0;
		  add_intercode(IC_OP, OP_pushA, 0, 0);
				if (i < call_type[omethod_index].parameters - 1)
				{
					if (!expect_punctuation(CTOKEN_SUBTYPE_COMMA))
					 return comp_error_text("expected comma after object method parameter", NULL);
				}
			}
		}
	 if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_CLOSE))
		{
   if (call_type[omethod_index].parameters == 0)
 		 return comp_error_text("expected closing bracket (this object method has no parameters)", NULL);

		 return comp_error_text("expected closing bracket after object method parameters", NULL);
		}
// finally, add the call and the call method type:
  add_intercode(IC_OP, OP_call_class, omethod_index, 0);
//  fpr(" finished");

  return 1;


}


static int variable_declaration(void)
{

 int storage_size = 1;

	struct ctokenstruct variable_ctoken;
	struct identifierstruct* variable_id;

	if (!read_next(&variable_ctoken))
		return 0;

	switch(variable_ctoken.type)
	{
	 case CTOKEN_TYPE_IDENTIFIER_NEW:
		 break; // this is the only one accepted
		case CTOKEN_TYPE_IDENTIFIER_C_KEYWORD:
		case CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD:
 		return comp_error_text("variable name already in use as keyword", &variable_ctoken);
	 case CTOKEN_TYPE_IDENTIFIER_USER_VARIABLE:
 		return comp_error_text("variable name already in use", &variable_ctoken);
 	default:
 		return comp_error_text("invalid variable name", &variable_ctoken);
	}

	variable_id = &identifier[variable_ctoken.identifier_index];

 variable_id->type = CTOKEN_TYPE_IDENTIFIER_USER_VARIABLE;
 variable_id->array_dims = 0;
// identifier[variable_ctoken.identifier_index].value =

// check for array dimensions:

 if (check_next(CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_SQUARE_OPEN))
	{
		variable_id->array_dim_size [0] = read_array_dimension_declaration();
		if (variable_id->array_dim_size [0] == 0) // error
			return 0; // read_array_dimension_declaration will have already written an error message
		variable_id->array_dims = 1;
  if (check_next(CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_SQUARE_OPEN))
	 {
		 variable_id->array_dim_size [1] = read_array_dimension_declaration();
		 if (variable_id->array_dim_size [1] == 0) // error
 			return 0; // read_array_dimension_declaration will have already written an error message
		 variable_id->array_dims = 2;
   if (check_next(CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_SQUARE_OPEN))
	  {
 		 variable_id->array_dim_size [2] = read_array_dimension_declaration();
		  if (variable_id->array_dim_size [2] == 0) // error
  			return 0; // read_array_dimension_declaration will have already written an error message
		  variable_id->array_dims = 3;
	  }
	 }
  switch(variable_id->array_dims)
  {
		 case 1:
			 variable_id->array_element_size [0] = 1;
			 storage_size = variable_id->array_dim_size [0];
			 break;
		 case 2:
			 variable_id->array_element_size [0] = variable_id->array_dim_size [1];
			 variable_id->array_element_size [1] = 1;
			 storage_size = variable_id->array_dim_size [0] * variable_id->array_dim_size [1];
			 break;
		 case 3:
			 variable_id->array_element_size [0] = variable_id->array_dim_size [1] * variable_id->array_dim_size [2];
			 variable_id->array_element_size [1] = variable_id->array_dim_size [2];
			 variable_id->array_element_size [2] = 1;
			 storage_size = variable_id->array_dim_size [0] * variable_id->array_dim_size [1] * variable_id->array_dim_size [2];
			 break;
  }
	}

 if (check_next(CTOKEN_TYPE_OPERATOR_ASSIGN, CTOKEN_SUBTYPE_EQ))
	{
// variable initialisation combined with declaration is currently not supported.
// supporting it will require 2 things: constant folding, and treating initial memory contents as part of a process' binary code.
// explain this error because it might come as a surprise:
  comp_error_text("cannot combine variable declaration and initialisation", NULL);
  write_line_to_log("(you may need initialisation code that runs once when the process is created)", MLOG_COL_ERROR);
		return 0;
	}

// Now allocate memory:

 variable_id->address = cstate.mem_pos;
 cstate.mem_pos += storage_size;


 if (cstate.mem_pos >= MEMORY_SIZE)
	{
		comp_error_text("not enough memory for variable", &variable_ctoken);
  start_log_line(MLOG_COL_COMPILER);
  write_to_log("Variable size is ");
  write_number_to_log(storage_size);
  write_to_log("; memory left is ");
  write_number_to_log(MEMORY_SIZE - variable_id->address);
  write_to_log("/");
  write_number_to_log(MEMORY_SIZE);
  write_to_log(".");
  finish_log_line();
	}

	return 1;

}




// call this after the [ is read and before the number is read.
// it will read the number and the following ]
// returns size of array dimension (not accounting for further sub-dimensions) on success, 0 on failure
static int read_array_dimension_declaration(void)
{
	struct ctokenstruct ctoken;

	if (!accept_next(&ctoken, CTOKEN_TYPE_NUMBER, -1))
		return comp_error_text("array dimension must be a number (constant expressions not currently supported)", &ctoken);

	if (ctoken.number_value <= 0)
		return comp_error_text("array dimension must be at least 1", &ctoken);

	if (ctoken.number_value >= ARRAY_DIMENSION_MAX_SIZE)
	{
		char error_str [60];
		snprintf(error_str, 60, "array dimension too large (max %i)", ARRAY_DIMENSION_MAX_SIZE);
		return comp_error_text(error_str, &ctoken);
	}

	if (!expect_punctuation(CTOKEN_SUBTYPE_SQUARE_CLOSE))
		return comp_error_text("expected ] after array dimension (dimension must be a literal number)", &ctoken);

	return ctoken.number_value; // success!

}


// Call this when a user variable has been read into ctoken.
// Should be an assignment
static int variable_assignment(struct ctokenstruct* ctoken)
{

	struct ctokenstruct operator_ctoken;

 if (identifier[ctoken->identifier_index].array_dims > 0)
	{
  if (!get_array_element_address(ctoken->identifier_index))
		 return 0;
// this address is pushed below before the following expression is read	(except in cases of ++/--)
	}

	if (!accept_next(&operator_ctoken, CTOKEN_TYPE_OPERATOR_ASSIGN, -1))
		return comp_error_text("expected assignment operator after variable", NULL);

// check for ++ and --
	if (operator_ctoken.subtype == CTOKEN_SUBTYPE_INCREMENT)
	{
  if (identifier[ctoken->identifier_index].array_dims == 0)
		 add_intercode(IC_OP_WITH_VARIABLE_OPERAND, OP_incr_mem, ctoken->identifier_index, 0);
		  else
		   add_intercode(IC_OP, OP_incr_derefA, 0, 0); // A should hold the address of the target element (after get_array_element_address() call above)
//		if (!expect_punctuation(CTOKEN_SUBTYPE_SEMICOLON))
//			return comp_error(CERR_EXPECTED_SEMICOLON, NULL);
		return 1;
	}
	if (operator_ctoken.subtype == CTOKEN_SUBTYPE_DECREMENT)
	{
  if (identifier[ctoken->identifier_index].array_dims == 0)
		 add_intercode(IC_OP_WITH_VARIABLE_OPERAND, OP_decr_mem, ctoken->identifier_index, 0);
		  else
		   add_intercode(IC_OP, OP_decr_derefA, 0, 0); // A should hold the address of the target element (after get_array_element_address() call above)
//		if (!expect_punctuation(CTOKEN_SUBTYPE_SEMICOLON))
//			return comp_error(CERR_EXPECTED_SEMICOLON, NULL);
		return 1;

	}

 if (identifier[ctoken->identifier_index].array_dims > 0)
	{
// If not ++/--, push the address so it can be retried later:
	 add_intercode(IC_OP, OP_pushA, 0, 0);
	}

 if (!next_expression(-1, BIND_EXPRESSION_START))
		return 0;
// next_expression should have left result in A

 if (operator_ctoken.subtype == CTOKEN_SUBTYPE_EQ)
	{
// now just copy A to correct memory location:
  if (identifier[ctoken->identifier_index].array_dims == 0)
   add_intercode(IC_OP_WITH_VARIABLE_OPERAND, OP_copyA_to_mem, ctoken->identifier_index, 0);
    else
				{
					add_intercode(IC_OP, OP_popB, 0, 0); // this should be address of target, pushed above
					add_intercode(IC_OP, OP_copyA_to_derefB, 0, 0); // copy contents of A to memory address pointed to by B
				}
//		if (!expect_punctuation(CTOKEN_SUBTYPE_SEMICOLON))
//			return comp_error(CERR_EXPECTED_SEMICOLON, NULL);
		return 1;
	}

 if (identifier[ctoken->identifier_index].array_dims == 0)
	{
  add_intercode(IC_OP, OP_pushA, 0, 0);
  add_intercode(IC_OP_WITH_VARIABLE_OPERAND, OP_setA_mem, ctoken->identifier_index, 0);
  add_intercode(IC_OP, OP_popB, 0, 0);
	}
	 else
		{
// if this is an array, the address of the target element should be on the stack.
// we need the result of the expression in B,
//  and the value of the variable in A
//  and the target address at the top of the stack
			add_intercode(IC_OP, OP_copyAtoB, 0, 0);
			add_intercode(IC_OP, OP_deref_stack_toA, 0, 0); // this instruction does not change the stack pointer
		}

// at this point, A holds variable's current value while B holds result of expression


 switch(operator_ctoken.subtype)
 {
	 case CTOKEN_SUBTYPE_PLUSEQ:
   add_intercode(IC_OP, OP_add, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_MINUSEQ:
   add_intercode(IC_OP, OP_sub_AB, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_MULEQ:
   add_intercode(IC_OP, OP_mul, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_DIVEQ:
   add_intercode(IC_OP, OP_div_AB, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_MODEQ:
   add_intercode(IC_OP, OP_mod_AB, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_BITSHIFT_L_EQ:
   add_intercode(IC_OP, OP_lsh_AB, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_BITSHIFT_R_EQ:
   add_intercode(IC_OP, OP_rsh_AB, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_BITWISE_AND_EQ:
   add_intercode(IC_OP, OP_and, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_BITWISE_OR_EQ:
   add_intercode(IC_OP, OP_or, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_BITWISE_XOR_EQ:
   add_intercode(IC_OP, OP_xor, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_BITWISE_NOT_EQ: // pretty sure this is wrong as ~ is not a binary operator
   add_intercode(IC_OP, OP_not, 0, 0); // but it will probably never be used.
   break;

  default:
  	fpr("\nError in c_compile.c: variable_assignment(): unknown assignment operator (subtype %i)", operator_ctoken.subtype);
  	error_call();	// this shouldn't happen
 } // end of operator subtype switch

// Now the result should be in A, so assign it:

// (arrays not yet done)

// only some assignment types will get here (others, such as =, are done above)
  if (identifier[ctoken->identifier_index].array_dims == 0)
   add_intercode(IC_OP_WITH_VARIABLE_OPERAND, OP_copyA_to_mem, ctoken->identifier_index, 0);
    else
				{
					add_intercode(IC_OP, OP_popB, 0, 0); // this should be address of target, pushed above
					add_intercode(IC_OP, OP_copyA_to_derefB, 0, 0);
				}
//		if (!expect_punctuation(CTOKEN_SUBTYPE_SEMICOLON))
//			return comp_error(CERR_EXPECTED_SEMICOLON, NULL);

		return 1;

} // end of variable_assignment



/*
// Call this when a user variable has been read into ctoken.
// Should be an assignment
static int variable_assignment(struct ctokenstruct* ctoken)
{

	struct ctokenstruct operator_ctoken;

 if (identifier[ctoken->identifier_index].array_dims > 0)
	{
  if (!get_array_element_address(ctoken->identifier_index))
		 return 0;
// this address is pushed below before the following expression is read	(except in cases of ++/--)
	}

	if (!accept_next(&operator_ctoken, CTOKEN_TYPE_OPERATOR_ASSIGN, -1))
		return comp_error_text("expected assignment operator after variable", NULL);

// check for ++ and --
	if (operator_ctoken.subtype == CTOKEN_SUBTYPE_INCREMENT)
	{
  if (identifier[ctoken->identifier_index].array_dims == 0)
		 add_intercode(IC_OP_WITH_VARIABLE_OPERAND, OP_incr_mem, ctoken->identifier_index, 0);
		  else
		   add_intercode(IC_OP, OP_incr_derefA, 0, 0); // A should hold the address of the target element (after get_array_element_address() call above)
//		if (!expect_punctuation(CTOKEN_SUBTYPE_SEMICOLON))
//			return comp_error(CERR_EXPECTED_SEMICOLON, NULL);
		return 1;
	}
	if (operator_ctoken.subtype == CTOKEN_SUBTYPE_DECREMENT)
	{
  if (identifier[ctoken->identifier_index].array_dims == 0)
		 add_intercode(IC_OP_WITH_VARIABLE_OPERAND, OP_decr_mem, ctoken->identifier_index, 0);
		  else
		   add_intercode(IC_OP, OP_decr_derefA, 0, 0); // A should hold the address of the target element (after get_array_element_address() call above)
//		if (!expect_punctuation(CTOKEN_SUBTYPE_SEMICOLON))
//			return comp_error(CERR_EXPECTED_SEMICOLON, NULL);
		return 1;

	}

 if (identifier[ctoken->identifier_index].array_dims > 0)
	{
// If not ++/--, push the address so it can be retried later:
	 add_intercode(IC_OP, OP_pushA, 0, 0);
	}

 if (!next_expression(-1, BIND_EXPRESSION_START))
		return 0;
// next_expression should have left result in A

 if (operator_ctoken.subtype == CTOKEN_SUBTYPE_EQ)
	{
// now just copy A to correct memory location:
  if (identifier[ctoken->identifier_index].array_dims == 0)
   add_intercode(IC_OP_WITH_VARIABLE_OPERAND, OP_copyA_to_mem, ctoken->identifier_index, 0);
    else
				{
					add_intercode(IC_OP, OP_popB, 0, 0); // this should be address of target, pushed above
					add_intercode(IC_OP, OP_copyA_to_derefB, 0, 0); // copy contents of A to memory address pointed to by B
				}
//		if (!expect_punctuation(CTOKEN_SUBTYPE_SEMICOLON))
//			return comp_error(CERR_EXPECTED_SEMICOLON, NULL);
		return 1;
	}

 if (identifier[ctoken->identifier_index].array_dims == 0)
	{
  add_intercode(IC_OP, OP_pushA, 0, 0);
  add_intercode(IC_OP_WITH_VARIABLE_OPERAND, OP_setA_mem, ctoken->identifier_index, 0);
  add_intercode(IC_OP, OP_popB, 0, 0);
	}
	 else
		{
// if this is an array, the address of the target element should be on the stack.
// we need the result of the expression in B,
//  and the value of the variable in A
//  and the target address at the top of the stack
			add_intercode(IC_OP, OP_copyAtoB, 0, 0);
			add_intercode(IC_OP, OP_deref_stack_toA, 0, 0); // this instruction does not change the stack pointer
		}

// at this point, A holds variable's current value while B holds result of expression


 switch(operator_ctoken.subtype)
 {
	 case CTOKEN_SUBTYPE_PLUSEQ:
   add_intercode(IC_OP, OP_add, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_MINUSEQ:
   add_intercode(IC_OP, OP_sub, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_MULEQ:
   add_intercode(IC_OP, OP_mul, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_DIVEQ:
   add_intercode(IC_OP, OP_div, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_MODEQ:
   add_intercode(IC_OP, OP_mod, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_BITSHIFT_L_EQ:
   add_intercode(IC_OP, OP_lsh, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_BITSHIFT_R_EQ:
   add_intercode(IC_OP, OP_rsh, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_BITWISE_AND_EQ:
   add_intercode(IC_OP, OP_and, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_BITWISE_OR_EQ:
   add_intercode(IC_OP, OP_or, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_BITWISE_XOR_EQ:
   add_intercode(IC_OP, OP_xor, 0, 0);
   break;
	 case CTOKEN_SUBTYPE_BITWISE_NOT_EQ:
   add_intercode(IC_OP, OP_not, 0, 0);
   break;

  default:
  	fpr("\nError in c_compile.c: variable_assignment(): unknown assignment operator (subtype %i)", operator_ctoken.subtype);
  	error_call();	// this shouldn't happen
 } // end of operator subtype switch

// Now the result should be in A, so assign it:

// (arrays not yet done)

// only some assignment types will get here (others, such as =, are done above)
  if (identifier[ctoken->identifier_index].array_dims == 0)
   add_intercode(IC_OP_WITH_VARIABLE_OPERAND, OP_copyA_to_mem, ctoken->identifier_index, 0);
    else
				{
					add_intercode(IC_OP, OP_popB, 0, 0); // this should be address of target, pushed above
					add_intercode(IC_OP, OP_copyA_to_derefB, 0, 0);
				}
//		if (!expect_punctuation(CTOKEN_SUBTYPE_SEMICOLON))
//			return comp_error(CERR_EXPECTED_SEMICOLON, NULL);

		return 1;

} // end of variable_assignment

*/

// called at the start of an expression and also when an opening bracket is found within an expression.
static int next_expression(int exit_point, int bind_level)
{

 cstate.recursion_level ++;

 if (cstate.recursion_level > RECURSION_LIMIT)
  return comp_error(CERR_RECURSION_LIMIT_REACHED, NULL);

 if (cstate.error != CERR_NONE)
  return 0;

 struct ctokenstruct ctoken;
 strcpy(ctoken.name, "(?)");
// int save_scode_pos;
 int retval;

// first set an exit point in case the expression includes logical operators (unless the calling function has already set an exit point, e.g. if it's from CM_IF)
 int fix_exit_point = 0; // if 1 this variable indicates that exit points need to be added by this function (because the calling function won't add them)

 if (exit_point == -1)
 {
   exit_point = allocate_exit_point(EXPOINT_TYPE_BASIC);
   if (exit_point == -1)
    return 0;
   fix_exit_point = 1;
 }

// now read the initial value into A:
 if (!next_expression_value(exit_point, bind_level))
		return 0;

// now loop through any further operators and values in the expression:
  while(TRUE)
  {
   if (!peek_next(&ctoken))
    comp_error(CERR_READ_FAIL, &ctoken);

			if (is_ctoken_closing_punctuation(&ctoken))
				break; // ??? is this correct?


/*   if (ctoken.type == CTOKEN_TYPE_OPERATOR_ASSIGN
    && accept_assignment_at_end)
    break; // may be finished*/
// parse_expression_operator reads the following operator then reads the next value into second_register (passed as the first parameter)
// if it needs to use first_register for anything, it needs to save it first then pop it afterwards
   retval = next_expression_operator(&ctoken, exit_point, bind_level);
   if (!retval)
    return 0; // error
// retval == 1 means next_expression_operator() was successful and we go to the next value
   if (retval == 2)
    break; // means a looser binding was found so we stop reading this part of the expression
  };

// if an exit point was allocated in this function (rather than being passed to this function), set its true and false points at the end of the expression:
 if (fix_exit_point)
 {
   add_intercode(IC_EXIT_POINT_TRUE, exit_point, 0, 0);
   add_intercode(IC_EXIT_POINT_FALSE, exit_point, 0, 0);
 }


 cstate.recursion_level --;
 return 1; // success!

}


/*

Expressions:
 a = b * (c - d);

setA a
pushA

setA *b
pushA
 setA *c
 pushA
 setA *d
 popB
 sub (A = B - A or A = c - d)
 popB // pops *b
 mul (A = b * (c - d))
popB (B = &a)
copy_A_to_address_in_B

So:
first, assign_variable sets A to &a
 - then pushes
calls next_expression with BIND_EXPRESSION_START
 - calls next_expression_value
  - next_expression_value reads b and writes: setA *b
  ? question: when is this pushed onto the stack? by next_expression_value or by next_expression?
 - next_expression calls next_expression_operator with BIND_EXPRESSION_START
  - next_expression_operator reads * and remembers that it will need to multiply
  - next_expression_operator calls next_expression_value with BIND_MUL
   - next_expression_value reads ( and calls next_expression with BIND_EXPRESSION_START
				- next_expression calls next_expression_value
				 - next_expression_value reads c and writes: setA *c
				- next_expression calls next_expression_operator with BIND_EXPRESSION_START
				 - next_expression_operator reads - and remembers that it will need to subtract
				 - next_expression_operator calls next_expression_value with BIND_ADD
				  - next_expression_value reads d and writes: setA *d
				 - next_expression_operator peeks at ) and realises the expression is over
				 - next_expression_operator recalls that it needs to subtract and writes: popB sub (which sets B to c and then A = B - A)
				- next_expression peeks at ) and returns
			- next_expression_value reads ) and returns
		- next_expression_operator recalls that it needs to multiply and writes: popB mul (which sets B to b and then A = B * A; A still holds c - d)
	- next_expression reads ; and returns
- assign_variable writes popB copy_A_to_address_in_B

done!

Think about really simple a = b;
setA &a
pushA
setA *b
popB
copy_A_to_address_in_B

Question: when does a value just read in need to be pushed? In the a = b example, it's a waste of time to push *b then get it back somehow.
- How about: next_expression_operator pushes if it reads an operator (rather than the end of the expression)
 - pushes just before it calls next_expression_value
  - this should work, I think.




*/




/*
returns:
same as parse_expression
*/
static int next_expression_value(int exit_point, int bind_level)
{

 if (cstate.error != CERR_NONE)
  return 0;

 struct ctokenstruct ctoken;
 strcpy(ctoken.name, "(?)");
// int dereference = 0;
// int retval;
 int apply_not = 0;//, apply_bitwise_not = 0;
// int fix_exit_point = 0; // this should be set to 1 if exit_point is allocated within this function rather than being passed by the passing function (it makes sure the exit point is actually created)

 int subexpression_exit_point = -1; // if the value is an open bracket, may need a new exit point for a subexpression
// int fix_subexpression_exit_point = 0; - not needed - the subexpression exit point is always fixed.
 //   *** TO DO: this should be optimised by combining multiple predictable jumps into a single jump
 //     (could be done easily in intercode, or in bcode generation)

/* if (check_next(CTOKEN_TYPE_OPERATOR_ARITHMETIC, CTOKEN_SUBTYPE_BITWISE_NOT)) // ~
 {
  apply_bitwise_not = 1;
 }*/

 if (check_next(CTOKEN_TYPE_OPERATOR_ARITHMETIC, CTOKEN_SUBTYPE_NOT)) // !
 {
  apply_not = 1;
 }
/*
// now check for ~ again in case it came after the !
 if (check_next(CTOKEN_TYPE_OPERATOR_ARITHMETIC, CTOKEN_SUBTYPE_BITWISE_NOT)) // ~
 {
  apply_bitwise_not = 1;
 }*/
/*
// check for dereference prefixes - *
 while (check_next(CTOKEN_TYPE_OPERATOR_ARITHMETIC, CTOKEN_SUBTYPE_MUL))
 {
  dereference ++;
 };*/

 if (!read_next(&ctoken))
  comp_error(CERR_READ_FAIL, &ctoken);

   switch(ctoken.type)
   {

    case CTOKEN_TYPE_PUNCTUATION: // only ( is accepted
     if (ctoken.subtype != CTOKEN_SUBTYPE_BRACKET_OPEN) // this is the only punctuation accepted at this point.
      return comp_error(CERR_SYNTAX_PUNCTUATION_IN_EXPRESSION, &ctoken);
/*
     if (exit_point == -1)
     {
      exit_point = allocate_exit_point(EXPOINT_TYPE_BASIC);
      if (exit_point == -1)
       return 0;
      fix_exit_point = 1;
     }*/
//     fpr("\n - found open bracket.");

     subexpression_exit_point = allocate_exit_point(EXPOINT_TYPE_BASIC);
     if (subexpression_exit_point == -1)
      return 0;

     if (!next_expression(subexpression_exit_point, BIND_EXPRESSION_START)) // new sub-expression resets binding.
      return 0;

     if (!expect_punctuation(CTOKEN_SUBTYPE_BRACKET_CLOSE))
						return comp_error_text("expected closing bracket after sub-expression", NULL);


      add_intercode(IC_EXIT_POINT_TRUE, subexpression_exit_point, 0, 0);// * should combine these two separate calls to just one IC
      add_intercode(IC_EXIT_POINT_FALSE, subexpression_exit_point, 0, 0);

//     if (fix_exit_point)
     {
//      add_intercode(IC_EXIT_POINT_TRUE, exit_point, 0, 0);// * should combine these two separate calls to just one IC
//      add_intercode(IC_EXIT_POINT_FALSE, exit_point, 0, 0);
     }

//     dereference_loop(REGISTER_WORKING, &dereference);
     if (apply_not)
      add_intercode(IC_OP, OP_lnot, 0, 0);

     break; // end case CTOKEN_TYPE_PUNCTUATION

    case CTOKEN_TYPE_NUMBER:
     add_intercode(IC_OP, OP_setA_num, ctoken.number_value, 0);
//     dereference_loop(target_register, &dereference);

     if (apply_not)
      add_intercode(IC_OP, OP_lnot, 0, 0);
//     if (apply_bitwise_not)
//      add_intercode(IC_OP, OP_not, 0, 0);
     break;

    case CTOKEN_TYPE_OPERATOR_ARITHMETIC:
// check for negative numbers:
    	if (ctoken.subtype == CTOKEN_SUBTYPE_MINUS)
					{
      if (!read_next(&ctoken)) // use read_next rather than expect_constant because expect_constant would accept another - sign
       comp_error(CERR_READ_FAIL, &ctoken);
      if (ctoken.type != CTOKEN_TYPE_NUMBER)
							return comp_error(CERR_SYNTAX_EXPRESSION_VALUE, &ctoken);
      add_intercode(IC_OP, OP_setA_num, ctoken.number_value * -1, 0);
      if (apply_not)
       add_intercode(IC_OP, OP_lnot, 0, 0);
						break;
					}
     return comp_error(CERR_SYNTAX_EXPRESSION_VALUE, &ctoken);

    case CTOKEN_TYPE_IDENTIFIER_USER_VARIABLE:
/*
     if (identifier[ctoken.identifier_index].array_dims > 0)
     {
      retval = parse_array_reference(target_register, secondary_register, save_secondary_register, ctoken.identifier_index, 0); // the ,0 means that we want the value of the element, not the address
      dereference_loop(target_register, &dereference);
      if (apply_not)
       add_intercode(IC_NOT_REG_REG, target_register, target_register, 0);
      if (apply_bitwise_not)
       add_intercode(IC_BITWISE_NOT_REG_REG, target_register, target_register, 0);
      return 1;
     } // end code for arrays
*/
     if (identifier[ctoken.identifier_index].array_dims > 0)
     {
     	if (!get_array_element_address(ctoken.identifier_index))
							return 0;
// now A contains the address of the array element in memory. Need to dereference it to get the value:
      add_intercode(IC_OP, OP_derefA, 0, 0);
     }
      else
       add_intercode(IC_OP_WITH_VARIABLE_OPERAND, OP_setA_mem, ctoken.identifier_index, 0);
//     dereference_loop(target_register, &dereference);
     if (apply_not)
      add_intercode(IC_OP, OP_lnot, 0, 0);
//     if (apply_bitwise_not)
//      add_intercode(IC_OP, OP_not, 0, 0);
     break; // end CTOKEN_TYPE_IDENTIFIER_USER_VARIABLE

    case CTOKEN_TYPE_IDENTIFIER_C_KEYWORD:
			 case CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD:
					switch(ctoken.identifier_index)
					{
					 case KEYWORD_C_PROCESS:
					 	if (!parse_process_expression(0))
								return 0;
							break;
					 case KEYWORD_C_COMPONENT:
					 	if (!parse_member_expression(0, 1, 0))
								return 0;
							break;
						case KEYWORD_C_CLASS: // indexed class reference (e.g. class[2].method())
							if (!parse_class_keyword_in_code())
								return 0;
							break;

						default:
					  return comp_error_text("unexpected keyword", &ctoken);
					}
     if (apply_not)
      add_intercode(IC_OP, OP_lnot, 0, 0);
					break; // end CTOKEN_TYPE_IDENTIFIER_C_KEYWORD/CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD


				case CTOKEN_TYPE_IDENTIFIER_CMETHOD:
  		 if (!parse_core_method_call(identifier[ctoken.identifier_index].value,  // see c_keywords.c for a list of these in the identifier initialiser
																																0, 0))
					return 0;
    if (apply_not)
     add_intercode(IC_OP, OP_lnot, 0, 0);
				break; // end CTOKEN_TYPE_IDENTIFIER_CMETHOD

				case CTOKEN_TYPE_IDENTIFIER_SMETHOD:
					if (!parse_std_method_call(identifier[ctoken.identifier_index].value))
						return 0;
    if (apply_not)
     add_intercode(IC_OP, OP_lnot, 0, 0);
				break; // end CTOKEN_TYPE_IDENTIFIER_SMETHOD

				case CTOKEN_TYPE_IDENTIFIER_UMETHOD:
					if (!parse_uni_method_call(identifier[ctoken.identifier_index].value))
						return 0;
    if (apply_not)
     add_intercode(IC_OP, OP_lnot, 0, 0);
				break; // end CTOKEN_TYPE_IDENTIFIER_UMETHOD

				case CTOKEN_TYPE_IDENTIFIER_CLASS:
					if (!parse_class_name_in_expression(identifier[ctoken.identifier_index].value))
						return 0;
    if (apply_not)
     add_intercode(IC_OP, OP_lnot, 0, 0);
				break; // end CTOKEN_TYPE_IDENTIFIER_CLASS

/*
    case CTOKEN_TYPE_OPERATOR_ARITHMETIC: // only & is accepted
     if (ctoken.subtype == CTOKEN_SUBTYPE_BITWISE_AND)
     {
// can't combine address-of prefix with dereferencing (*) or negation (!)
      if (dereference > 0)
       return comp_error(CERR_DEREFERENCED_ADDRESS_OF, &ctoken);
      if (apply_not > 0 || apply_bitwise_not > 0)
       return comp_error(CERR_NEGATED_ADDRESS_OF, &ctoken);
// get address of variable:
      if (!read_next_expression_value(&ctoken))
       comp_error(CERR_READ_FAIL, &ctoken);
      if (ctoken.type != CTOKEN_TYPE_IDENTIFIER_USER_VARIABLE)
       return comp_error(CERR_ADDRESS_OF_PREFIX_MISUSE, &ctoken);
      if (identifier[ctoken.identifier_index].array_dims == 0)
      {
// simplest case: address of non-array static variable:
       if (identifier[ctoken.identifier_index].storage_class == STORAGE_STATIC)
       {
        add_intercode(IC_ID_ADDRESS_PLUS_OFFSET_TO_REGISTER, target_register, ctoken.identifier_index, 0);
        return 1;
       }
// address of non-array automatic variable:
       if (!auto_var_address_to_register(ctoken.identifier_index, target_register, 0))
        return 0;
       return 1;
      }
// for an array, can use parse_array_reference
      return parse_array_reference(target_register, secondary_register, save_secondary_register, ctoken.identifier_index, 1); // ,1 means the address of the element will be in the register

     } // end address-of operator
     return comp_error(CERR_SYNTAX_EXPRESSION_VALUE, &ctoken); // end CTOKEN_TYPE_OPERATOR_ARITHMETIC
*/

				case CTOKEN_TYPE_IDENTIFIER_NEW:
					return comp_error_text("unrecognised token", &ctoken);

    default:
     return comp_error(CERR_SYNTAX_EXPRESSION_VALUE, &ctoken);
   }


   return 1;
/*
// successful! The value is now in A.
// peek at next operator:
   if (!peek_next(&ctoken))
				return 0;

			if (is_ctoken_closing_punctuation(&ctoken))
				return 1; // ??? is this correct?

			if (ctoken.type != CTOKEN_TYPE_OPERATOR_ARITHMETIC
				&& ctoken.type != CTOKEN_TYPE_OPERATOR_LOGICAL
				&& ctoken.type != CTOKEN_TYPE_OPERATOR_COMPARISON)
					return comp_error_text("expected operator or appropriate punctuation after value in expression", ctoken);

//   add_intercode(IC_OP, OP_pushA, 0, 0);

   return next_expression_operator(ctoken, exit_point, bind_level);
*/
//   return comp_error(CERR_SYNTAX_EXPRESSION_VALUE, &ctoken);

} // end next_expression_value





enum
{
NEXT_EXP_OPERATOR_RESULT_ERROR,
NEXT_EXP_OPERATOR_RESULT_CONTINUE, // found an operator and processed it. Now read next one.
NEXT_EXP_OPERATOR_RESULT_BINDING, // found looser-binding operator. return without advancing scode_pos so that expression up to this point can be resolved.
NEXT_EXP_OPERATOR_RESULT_FINISHED // found punctuation indicating end of expression
};


// note that ctoken_operator has only been peeked and not read properly
static int next_expression_operator(struct ctokenstruct* ctoken_operator, int exit_point, int previous_operator_binding)
{

//   if (!peek_next(&ctoken_operator))
//				return NEXT_EXP_OPERATOR_RESULT_ERROR;

   cstate.recursion_level ++;

   if (cstate.recursion_level > RECURSION_LIMIT)
    return comp_error(CERR_RECURSION_LIMIT_REACHED, NULL);

   if (cstate.error != CERR_NONE)
    return NEXT_EXP_OPERATOR_RESULT_ERROR;

//   int retval = 0;
   int return_value = NEXT_EXP_OPERATOR_RESULT_CONTINUE;
//   struct ctokenstruct ctoken_value;
//   int expression_complete = 0;

   int operator_binding = get_operator_binding_level(ctoken_operator->subtype);

// If the new operator binds more loosely than the previous operator, return until next_expression
   if (operator_binding <= previous_operator_binding)
    return NEXT_EXP_OPERATOR_RESULT_BINDING;

// operator binds as closely as, or closer than, previous operator:

// read the operator properly (before it was just peeked)
   if (!read_next(ctoken_operator))
    return comp_error(CERR_READ_FAIL, ctoken_operator);

// logical operators are dealt with here:
   if (ctoken_operator->type == CTOKEN_TYPE_OPERATOR_LOGICAL)
			{
     switch(ctoken_operator->subtype)
     {
      case CTOKEN_SUBTYPE_LOGICAL_AND:
       add_intercode(IC_IFFALSE_JUMP_TO_EXIT_POINT, exit_point, 0, 0);
       return_value = NEXT_EXP_OPERATOR_RESULT_CONTINUE;//next_expression(exit_point, BIND_LOGICAL_AND);
       break;
      case CTOKEN_SUBTYPE_LOGICAL_OR:
       add_intercode(IC_IFTRUE_JUMP_TO_EXIT_POINT, exit_point, 0, 0);
       return_value = NEXT_EXP_OPERATOR_RESULT_CONTINUE;//next_expression(exit_point, BIND_LOGICAL_OR);
       break;
     }
			}
			 else
				{
			  // if the operator isn't logical, put the existing contents of A onto the stack:
     add_intercode(IC_OP, OP_pushA, 0, 0);
     // logical operators discard the contents of A
				}



//			if (!next_expression_value(exit_point, operator_binding))
			if (!next_expression(exit_point, operator_binding))
			 return NEXT_EXP_OPERATOR_RESULT_ERROR;

// The result of the next expression or value should now be in A.

   switch(ctoken_operator->type)
   {

    case CTOKEN_TYPE_OPERATOR_ARITHMETIC:
    case CTOKEN_TYPE_OPERATOR_COMPARISON:
// First pop the previous value to B (this matches the pushA instruction just before the call to next_expression_value)
       add_intercode(IC_OP, OP_popB, 0, 0);
// A should hold the result of the value or subexpression after the operator.
       switch(ctoken_operator->subtype)
       {
        case CTOKEN_SUBTYPE_PLUS:
         add_intercode(IC_OP, OP_add, 0, 0);
         break;
        case CTOKEN_SUBTYPE_MINUS:
         add_intercode(IC_OP, OP_sub_BA, 0, 0);
         break;
        case CTOKEN_SUBTYPE_MUL:
         add_intercode(IC_OP, OP_mul, 0, 0);
         break;
        case CTOKEN_SUBTYPE_DIV:
         add_intercode(IC_OP, OP_div_BA, 0, 0);
         break;
        case CTOKEN_SUBTYPE_MOD:
         add_intercode(IC_OP, OP_mod_BA, 0, 0);
         break;
        case CTOKEN_SUBTYPE_BITWISE_AND:
         add_intercode(IC_OP, OP_and, 0, 0);
         break;
        case CTOKEN_SUBTYPE_BITWISE_XOR:
         add_intercode(IC_OP, OP_xor, 0, 0);
         break;
        case CTOKEN_SUBTYPE_BITWISE_OR:
         add_intercode(IC_OP, OP_or, 0, 0);
         break;
        case CTOKEN_SUBTYPE_BITWISE_NOT:
         add_intercode(IC_OP, OP_not, 0, 0);
         break;
        case CTOKEN_SUBTYPE_BITSHIFT_L:
         add_intercode(IC_OP, OP_lsh_BA, 0, 0);
         break;
        case CTOKEN_SUBTYPE_BITSHIFT_R:
         add_intercode(IC_OP, OP_rsh_BA, 0, 0);
         break;


        case CTOKEN_SUBTYPE_EQ_EQ: // ==
         add_intercode(IC_OP, OP_comp_eq, 0, 0); // this leaves register A with 1 if true, 0 if false
         break;
        case CTOKEN_SUBTYPE_GR: // >
         add_intercode(IC_OP, OP_comp_gr, 0, 0); // this leaves register A with 1 if true, 0 if false
         break;
        case CTOKEN_SUBTYPE_GREQ: // >=
         add_intercode(IC_OP, OP_comp_greq, 0, 0); // this leaves register A with 1 if true, 0 if false
         break;
        case CTOKEN_SUBTYPE_LESS: // <
         add_intercode(IC_OP, OP_comp_ls, 0, 0); // this leaves register A with 1 if true, 0 if false
         break;
        case CTOKEN_SUBTYPE_LESEQ: // <=
         add_intercode(IC_OP, OP_comp_lseq, 0, 0); // this leaves register A with 1 if true, 0 if false
         break;
        case CTOKEN_SUBTYPE_COMPARE_NOT: // !=
         add_intercode(IC_OP, OP_comp_neq, 0, 0); // this leaves register A with 1 if true, 0 if false
         break;

        default:
         fpr("\nc_compile.c: next_expression_operator(): Unimplemented operator found?");
         error_call();
         break;
       }
     break; // end CTOKEN_TYPE_OPERATOR_ARITHMETIC

// logical operators now dealt with above

/*
    case CTOKEN_TYPE_OPERATOR_LOGICAL:
     switch(ctoken_operator->subtype)
     {
      case CTOKEN_SUBTYPE_LOGICAL_AND:
       add_intercode(IC_IFFALSE_JUMP_TO_EXIT_POINT, exit_point, 0, 0);
       return_value = NEXT_EXP_OPERATOR_RESULT_CONTINUE;//next_expression(exit_point, BIND_LOGICAL_AND);
       goto parse_expression_operator_success;
      case CTOKEN_SUBTYPE_LOGICAL_OR:
       add_intercode(IC_IFTRUE_JUMP_TO_EXIT_POINT, exit_point, 0, 0);
       return_value = NEXT_EXP_OPERATOR_RESULT_CONTINUE;//next_expression(exit_point, BIND_LOGICAL_OR);
       goto parse_expression_operator_success;

/ *
This code for logical operators is poorly optimised, because in many cases it will cause a single t/f value to be tested by
iffalse/iftrue instructions multiple times. This happens especially where subexpressions are enclosed in brackets and separated by
|| (and maybe &&).

I could improve this by making it so that before an iftrue/iffalse instruction was added the compiler looked back at
immediately preceding exit point labels and:
 - if the instruction's truth value matched the exit point's - replacing jumps to the exit point with jumps to the instruction's exit point;
 - if not - putting the label after the instruction.
Should probably do this!


* /


     }
     break; // end case CTOKEN_TYPE_OPERATOR_LOGICAL
*/
   }

//   parse_expression_operator_success:

/*
Here we should probably call next_expression_operator again.

It will go on to read the expression until it finds a looser-binding operator, then return.
After this it will


Basically, the expression parser should push everything to the stack until it reaches an operator of looser precedence than the previous operator
Then, it rolls back through the stacked values, writing the appropriate operation each time, until it finds an operator of precedence equal to or looser than the last operator found.
Then it continues reading, with the new operator's binding level.

next_expression
- calls next_expression_value with current binding level
loop:
 - calls next_expression_operator with current binding level
  - if returns 0, error
  - if returns 1, continues through the loop
  - if returns 2, breaks

next_expression_value
 - reads value to A
  - if open bracket:
   - calls next_expression with new expression binding level, then checks for closing bracket
 - peeks at next ctoken. If ending punctuation, returns.
 - pushes A to stack
 - calls next_expression_operator
  - returns retval of next_expression_operator

next_expression_operator
 - peeks at next operator
  - if next operator has binding tighter than the previous operator
   - push A to stack
   - reads next operator
   - remembers next operator type
** - calls next_expression with next operator binding level
    - next_expression will parse the expression until it reaches an operator with binding looser than or equal to that of next operator, then it will return.
   - writes instruction to perform operation, assuming that next value is in A and that running result is next on stack
    - i.e. popB, A = B [operation] A
   - returns
  - if next operator has binding looser than the last operator found:
   - returns 2 to tell next_expression to return as well. Eventually the calls will unwind to the point where next operator has binding tighter than current (possibly when it has got back to the outermost next_expression call)
  - if next operator has binding EQUAL TO the last operator found:
   - returns 1 to tell next_expression to go through loop again
    * actually no, I think this should just be same as return 2.
    * equal binding is dealt with by the stack unrolling until it reaches the ** mark above,
      at which point it will write the previous instruction then




a = b + c * d + e;

setA b
pushA
setA c
pushA
setA d
 // finds +
popB
mul (A = c * d)



*/

   cstate.recursion_level --;

   return return_value;

} // end parse_expression_operator







// TO DO: this should be data rather than a function
static int get_operator_binding_level(int ctoken_subtype)
{

	switch(ctoken_subtype)
	{
		case CTOKEN_SUBTYPE_LOGICAL_AND: // &&
			return BIND_LOGICAL_AND;

		case CTOKEN_SUBTYPE_LOGICAL_OR: // ||
			return BIND_LOGICAL_OR;

		case CTOKEN_SUBTYPE_EQ_EQ: // ==
		case CTOKEN_SUBTYPE_LESEQ: // <=
		case CTOKEN_SUBTYPE_LESS: // <
		case CTOKEN_SUBTYPE_GREQ: // >=
		case CTOKEN_SUBTYPE_GR: // >
		case CTOKEN_SUBTYPE_COMPARE_NOT: // !=
			return BIND_COMPARE;

		case CTOKEN_SUBTYPE_BITSHIFT_R: // >>
		case CTOKEN_SUBTYPE_BITSHIFT_L: // <<
		case CTOKEN_SUBTYPE_BITWISE_AND: // &
		case CTOKEN_SUBTYPE_BITWISE_OR: // |
		case CTOKEN_SUBTYPE_BITWISE_XOR: // ^
		case CTOKEN_SUBTYPE_BITWISE_NOT: // ~
			return BIND_BITWISE;

		case CTOKEN_SUBTYPE_PLUS: // +
		case CTOKEN_SUBTYPE_MINUS: // -
		return BIND_ADD;

		case CTOKEN_SUBTYPE_MUL: // *
		case CTOKEN_SUBTYPE_DIV: // /
		case CTOKEN_SUBTYPE_MOD: // %
			return BIND_MUL;

		case CTOKEN_SUBTYPE_NOT: // !
			return BIND_LOGICAL_NOT;

	}

			return BIND_EXPRESSION_START; // this should probably be an error_call instead as this should never happen.

}




static int is_ctoken_closing_punctuation(struct ctokenstruct* ctoken)
{

   if (ctoken->type == CTOKEN_TYPE_PUNCTUATION
    && (ctoken->subtype == CTOKEN_SUBTYPE_BRACKET_CLOSE
     || ctoken->subtype == CTOKEN_SUBTYPE_SEMICOLON
     || ctoken->subtype == CTOKEN_SUBTYPE_SQUARE_CLOSE
     || ctoken->subtype == CTOKEN_SUBTYPE_BRACE_CLOSE
     || ctoken->subtype == CTOKEN_SUBTYPE_COMMA))
     return 1;

			return 0;


}

// Call this function just after an array variable is read in an assignment or expression.
// It leaves the address of the element in A.
// TO DO: optimise better (or at all) for constant indices
static int get_array_element_address(int id_index)
{

 struct identifierstruct* variable_id = &identifier[id_index];

 int i;

 for (i = 0; i < variable_id->array_dims; i ++)
	{
		if (!expect_punctuation(CTOKEN_SUBTYPE_SQUARE_OPEN))
			return comp_error_text("expected [ at start of array index", NULL);
		if (!next_expression(-1, BIND_EXPRESSION_START))
			return 0;
		if (!expect_punctuation(CTOKEN_SUBTYPE_SQUARE_CLOSE))
			return comp_error_text("expected ] at end of array index", NULL);

// If this isn't the final dimension, we multiply the index by the size of each element of the dimension:
//   (the final dimension always has elements of size 1)
		if (i < (variable_id->array_dims - 1))
		{
			if (variable_id->array_element_size [i] > 1) // possible if for some reason the dimension has just one element
		  add_intercode(IC_OP, OP_mulA_num, variable_id->array_element_size [i], 0);
		 add_intercode(IC_OP, OP_pushA, 0, 0);
		}
	}


// At this point, A holds the offset of the target element from the first element of the final dimension.

// If the array has >1 dimensions, need to add the indices for the other dimensions:
	if (variable_id->array_dims > 1)
	{
	 add_intercode(IC_OP, OP_popB, 0, 0);
	 add_intercode(IC_OP, OP_add, 0, 0);
	}
	if (variable_id->array_dims == 3)
	{
	 add_intercode(IC_OP, OP_popB, 0, 0);
	 add_intercode(IC_OP, OP_add, 0, 0);
	}

// at this point, A should hold the offset from the first element of the entire array.
// To get the element, just add the address of the first element:
 add_intercode(IC_OP_WITH_VARIABLE_OPERAND, OP_addA_num, id_index, 0); // id_index is converted to the variable's actual address later

 return 1;

}

/*

bcode for array reference:
int a [5] [3];
a [b] [2] = 1;

setA_mem b (produced by next_expression)
pushA
setA_num 3 (sizeof first dimension elements)
popB
mul
pushA

setA_num 2 (produced by next_expression)
popB
add
pushA

// at end:
setA_address a
popB
add

// = 1 part:
pushA
setA_num 1 (produced by next_expression)
popB
copyA_to_derefB

- how to optimise? (without assuming that the first setA is a single instruction)
 - new instruction: mulA_num
 - new instruction: addA_num

setA_mem b
mulA_num 3
pushA
setA_num 2
popB
addA_num (address of a)



*/



/*
Okay, how should variable assignment work?

a = 3;

1.
setA &a
pushA
2.
setA 3
3.
popB
getA &B

optimised to:
1.
push_address a
2.
setA 3
3.
copy_A_to_popped_address

optimised to:
1. push_address a
2. copy_to_popped_address 3

optimised to:
1. set_address a 3

a = b;

1.
setA a
pushA
2.
setA b
3.
popB
copy_derefA_to_addressB

a [3] = b;

1.
setA a
push A
2.
setA 3
popB
add // adds B to A
pushA
3.
setA b
4.
popB
5.
copy_derefA_to_addressB // will need to bounds-check this!

How can we optimise this?
- when the add is added, should look back and work out that all values are constants (&a and 3).

*** OR:

There's no reason to use anything like asm here. Why not:
-if no array, just do it like this:
a = 3;
parse_expression (results in: setA 3)
copy_A_to_address a
// which is then easily optimised to
copy_number_to_address a 3

a = b;
parse_expression (results in: copy_from_address_to_A b)
deref_A_to_address a
// which is then easily optimised to
copy_between_addresses a b

-if an array:
a [2] = 3;
copy_number_to_A &a
pushA
copy_number_to_A 2 // (from parse_expression)
popB
add
pushA
copy_number_to_A 3
popB
copy_A_to_address_in_B

How to optimise this?
 - when the add instructions is added, should look back. If the previous 4 instructions are [copy_number_to_A, pushA, copy_number_to_A, popB], should be very easy to optimise.
 - to this:
copy_number_to_A (&a + 2)
Then, pushA turns this into:
push_number (&a + 2)
 - then, when copy_A_to_address_in_B is found, it looks back. If the previous instructions are (push_number, copy_number_to_A, popB, copy_A_to_address_in_B), should be very easy to optimise.

instructions needed:
NOP
copy_number_to_A <number>
copy_deref_to_A <number/address>
pushA
popB
add sub mult div mod not and or xor lshift rshift logical_not
copy_A_to_address_in_B
push_number_to_A <number>
push_deref_to_A <number/address>
pop_B_to_address <number/address>



*/


static void add_intercode(int ic_type, int value1, int value2, int value3)
{

 if (cstate.ic_pos >= INTERCODE_SIZE - 2)
	{
		if (cstate.error == CERR_NONE)
			comp_error(CERR_TOO_MUCH_INTERCODE, NULL);
		return;
	}

 cstate.intercode[cstate.ic_pos].type = ic_type;
 cstate.intercode[cstate.ic_pos].value [0] = value1;
 cstate.intercode[cstate.ic_pos].value [1] = value2;
 cstate.intercode[cstate.ic_pos].value [2] = value3;
 cstate.intercode[cstate.ic_pos].src_line = cstate.src_line;

//#define PRINT_INTERCODE

#ifdef PRINT_INTERCODE

 fpr("\n[%i:%i] ", cstate.ic_pos, cstate.src_line);


 switch(ic_type)
 {
		case IC_NONE:
			fpr("none");
			break;
	 case IC_OP:
	 case IC_OP_WITH_VARIABLE_OPERAND:
		 fpr("%s ", instruction_set[value1].name);
		 if (instruction_set[value1].operands >= 1)
		 {
		 	if (instruction_set[value1].operand_type [0] == OPERAND_TYPE_NUMBER)
				 fpr("%i ", value2);
		 	if (instruction_set[value1].operand_type [0] == OPERAND_TYPE_MEMORY)
				 fpr("%s ", identifier[value2].name);
		 	if (instruction_set[value1].operand_type [0] == OPERAND_TYPE_BCODE_ADDRESS)
				 fpr("%i ", value2); // not sure this will be useful
		 }
		 break;
		case IC_EXIT_POINT_TRUE:
			fpr(" exit point %i (true)", value1);
			break;
		case IC_EXIT_POINT_FALSE:
			fpr(" exit point %i (false)", value1);
			break;
		case IC_LABEL_DEFINITION:
			fpr("+++label [%s]", identifier[value1].name);
			break;
		case IC_GOTO_LABEL:
			fpr("goto %s", identifier[value1].name);
			break;
		case IC_IFFALSE_JUMP_TO_EXIT_POINT:
			fpr("if_false_jump_to_exit_point %i", value1);
			break;
		case IC_IFTRUE_JUMP_TO_EXIT_POINT:
			fpr("if_true_jump_to_exit_point %i", value1);
			break;
		case IC_JUMP_EXIT_POINT_TRUE:
			fpr("jump_to_true_exit_point %i", value1);
			break;
		case IC_JUMP_EXIT_POINT_FALSE:
			fpr("jump_to_false_exit_point %i", value1);
			break;
		case IC_NUMBER:
			fpr("(%i)", value1);
			break;
		case IC_SWITCH:
			fpr("switchA %i %i %i", value1, value2, value3);
			break;
		case IC_JUMP_TABLE:
			fpr("jt %i", value1);
			break;
//		case IC_NUMBER:
//			fpr("$%i", cstate.inter);

 }


#endif

 cstate.ic_pos ++;

}


static int allocate_exit_point(int type)
{

 cstate.expoint_pos ++;
 if (cstate.expoint_pos >= EXPOINTS)
  return comp_error_minus1(CERR_TOO_MANY_EXIT_POINTS, NULL);

 cstate.expoint[cstate.expoint_pos].type = type;
 cstate.expoint[cstate.expoint_pos].true_point_used = 0;
 cstate.expoint[cstate.expoint_pos].false_point_used = 0;
 cstate.expoint[cstate.expoint_pos].true_point_bcode = -1; // indicates undefined
 cstate.expoint[cstate.expoint_pos].false_point_bcode = -1;

 return cstate.expoint_pos;

}



const char *error_name [CERRS] =
{
"(no error)", // CERR_NONE

// compiler errors
//"expected comma", // CERR_EXPECTED_COMMA_AFTER_
"recursion limit exceeded (code too complex?)", // CERR_RECURSION_LIMIT_REACHED
"syntax error at statement start", // CERR_SYNTAX_AT_STATEMENT_START
"out of space in intercode (code too complex)", // CERR_TOO_MUCH_INTERCODE
"expected semicolon at end of statement", // CERR_EXPECTED_SEMICOLON
"code read failure", // CERR_READ_FAIL,
"unexpected punctuation in expression", // CERR_SYNTAX_PUNCTUATION_IN_EXPRESSION,
"expected closing bracket", // CERR_NO_CLOSE_BRACKET,
"syntax error in expression", // CERR_SYNTAX_EXPRESSION_VALUE,
"too many exit points allocated (code too complex)", // CERR_TOO_MANY_EXIT_POINTS,

// fixer errors
"expected #process header at start of source", // CERR_FIXER_EXPECTED_PROCESS_HEADER
"expected #code header at end of process structure definition", // CERR_FIXER_EXPECTED_CODE_HEADER

// parser errors
"scode range error?", // CERR_PARSER_SCODE_BOUNDS (probably should never occur)
"parser reached end of code while reading token", // CERR_PARSER_REACHED_END_WHILE_READING
"unknown token type", // CERR_PARSER_UNKNOWN_TOKEN_TYPE
"number too large", // CERR_PARSER_NUMBER_TOO_LARGE
"letter or underscore found in decimal number", // CERR_PARSER_LETTER_IN_NUMBER
"token too long", // CERR_PARSER_TOKEN_TOO_LONG
"number starts with 0 but is not hex or binary", // CERR_PARSER_NUMBER_STARTS_WITH_ZERO,
"invalid letter or underscore found in hex number", // CERR_PARSER_LETTER_IN_HEX_NUMBER,
"invalid digit in binary number", // CERR_INVALID_NUMBER_IN_BINARY_NUMBER,
"letter in binary number", // CERR_PARSER_LETTER_IN_BINARY_NUMBER,
"too many identifiers", // CERR_PARSER_TOO_MANY_IDENTIFIERS

"intercode error", // CERR_INTERCODE - used in c_generate.c. This text shouldn't be displayed.
"generic error", // CERR_GENERIC - used by comp_error_text()
};


int comp_error(int error_type, struct ctokenstruct* ctoken)
{

     start_log_line(MLOG_COL_ERROR);
     set_log_line_source_position(cstate.source_edit->player_index, cstate.source_edit->template_index, cstate.src_line);
     write_to_log("Compiler error at line ");
     write_number_to_log(cstate.src_line + 1);
     write_to_log(".");
     finish_log_line();

     start_log_line(MLOG_COL_ERROR);
     set_log_line_source_position(cstate.source_edit->player_index, cstate.source_edit->template_index, cstate.src_line);
     write_to_log("Error: ");
     write_to_log(error_name [error_type]);
     write_to_log(".");
     finish_log_line();

     if (ctoken != NULL)
     {
      start_log_line(MLOG_COL_COMPILER);
      set_log_line_source_position(cstate.source_edit->player_index, cstate.source_edit->template_index, cstate.src_line);
      write_to_log("Last token read: ");
      write_to_log(ctoken->name);
      write_to_log(".");
      finish_log_line();
     }

     cstate.error = error_type;
//     error_call();
     return 0;
}


// call when comp_error is called from a function that returns -1 on failure (and so needs to return -1)
int comp_error_minus1(int error_type, struct ctokenstruct* ctoken)
{

 comp_error(error_type, ctoken);
 return -1;

}

int comp_error_text(const char* error_text, struct ctokenstruct* ctoken)
{



     start_log_line(MLOG_COL_ERROR);
     set_log_line_source_position(cstate.source_edit->player_index, cstate.source_edit->template_index, cstate.src_line);
     write_to_log("Compiler error at line ");
     write_number_to_log(cstate.src_line + 1);
     write_to_log(".");
     finish_log_line();

     start_log_line(MLOG_COL_ERROR);
     set_log_line_source_position(cstate.source_edit->player_index, cstate.source_edit->template_index, cstate.src_line);
     write_to_log("Error: ");
     write_to_log(error_text);
     write_to_log(".");
     finish_log_line();

     if (ctoken != NULL)
     {
      start_log_line(MLOG_COL_COMPILER);
      set_log_line_source_position(cstate.source_edit->player_index, cstate.source_edit->template_index, cstate.src_line);
      write_to_log("Last token read: ");
      write_to_log(ctoken->name);
      write_to_log(".");
      finish_log_line();
     }

     cstate.error = CERR_GENERIC;
//     error_call();
     return 0;
}


void comp_warning_text(const char* warning_text)
{



     start_log_line(MLOG_COL_COMPILER);
     set_log_line_source_position(cstate.source_edit->player_index, cstate.source_edit->template_index, cstate.src_line);
     write_to_log("Compiler warning at line ");
     write_number_to_log(cstate.src_line + 1);
     write_to_log(".");
     finish_log_line();

     start_log_line(MLOG_COL_COMPILER);
     set_log_line_source_position(cstate.source_edit->player_index, cstate.source_edit->template_index, cstate.src_line);
     write_to_log("Warning: ");
     write_to_log(warning_text);
     write_to_log(".");
     finish_log_line();

     return;
}
