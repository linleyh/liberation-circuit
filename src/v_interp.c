#include <allegro5/allegro.h>
#include <stdio.h>
#include <stdlib.h>

#include "m_config.h"
#include "g_header.h"
#include "m_globvars.h"
#include "c_header.h"
#include "g_misc.h"
#include "g_method.h"
#include "g_method_core.h"
#include "g_method_std.h"
#include "g_method_uni.h"
#include "i_console.h"
#include "i_error.h"
#include "e_log.h"

#include "v_interp.h"
#include "v_draw_panel.h"

struct vmstate_struct vmstate;
extern struct instruction_set_struct instruction_set [INSTRUCTIONS]; // in c_compile.c
extern struct scanlist_struct scanlist; // in g_method_std.c. scanlist.current is reset to 0 every cycle.
extern struct bcode_panel_state_struct bcp_state;
extern struct game_struct game;
extern struct slider_struct slider [SLIDERS];
extern struct template_debug_struct template_debug [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct identifierstruct identifier [IDENTIFIERS];

extern struct call_type_struct call_type [CALL_TYPES];
extern struct mmethod_call_type_struct mmethod_call_type [MMETHOD_CALL_TYPES];
extern struct cmethod_call_type_struct cmethod_call_type [CMETHOD_CALL_TYPES];
extern struct smethod_call_type_struct smethod_call_type [SMETHOD_CALL_TYPES];
extern struct umethod_call_type_struct umethod_call_type [UMETHOD_CALL_TYPES];



#define get_next_instr if (vmstate.bcode_pos >= BCODE_POS_MAX) goto bcode_bounds_error; instr = vmstate.bcode->op [++vmstate.bcode_pos];
#define get_next_instr_expect_address if (vmstate.bcode_pos >= BCODE_POS_MAX) goto bcode_bounds_error; instr = vmstate.bcode->op [++vmstate.bcode_pos]; if (instr < 0 || instr >= MEMORY_SIZE) goto memory_address_error;

static void	print_execution_error(const char* error_message, int values, int value1);
int execute_bcode_single_step_for_watch(void);
static void print_method_parameters(int parameters, char* log_line_string, char* first_parameter_text, char* second_parameter_text);

void execute_bcode(struct core_struct* core, struct bcode_struct* bc, s16b* memory)
{

	vmstate.core = core;
	vmstate.bcode = bc;
	vmstate.bcode_pos = -1; // will be incremented before use
	vmstate.instructions_left = core->instructions_per_cycle;
	vmstate.memory = memory;
	vmstate.stack_pos = 1;
	vmstate.error_state = 0;

	vmstate.nearby_well_index = -2; // means this has not yet been calculated
	scanlist.current = 0; // means the scanlist will need to be built if a scanning function is called.

	char print_string [STRING_MAX_LENGTH];

	int i;

	for (i = 0; i < VM_STACK_SIZE; i ++)
	{
		vmstate.vm_stack [i] = 0;
	}

	for (i = 0; i < VM_REGISTERS; i++)
	{
		vmstate.vm_register [i] = 0;
	}

	s16b instr;
	int value [3];

//#define SHOW_BCODE

#ifdef SHOW_BCODE
 fpr("\n\n starting...");
#endif

	while(TRUE)
	{
//		Note that bcode_pos can be -1 at this point! (probably because of a jump to 0) It should be incremented before being used (get_next_instr does this)

		get_next_instr;

		vmstate.instructions_left --; // should be a cost depending on instruction type
		if (vmstate.instructions_left <= 0) // this is only checked here, not during operations. If instructions are exhausted during an operation, the operation will complete before the vm exits.
			goto out_of_instructions_error;

#ifdef SHOW_BCODE
		if (instr != 0)
			fpr("\n%04d [%i] %s", vmstate.bcode_pos, w.core[0].memory[1], instruction_set[instr].name);
#endif

		switch(instr)
		{
		 case OP_nop:
			 break;
			case OP_pushA:
				vmstate.vm_stack [vmstate.stack_pos++] = vmstate.vm_register [VM_REG_A];
				if (vmstate.stack_pos >= VM_STACK_SIZE)
					goto stack_full_error;
				break;
			case OP_popB:
				vmstate.stack_pos --;
				if (vmstate.stack_pos <= 0)
					goto stack_below_zero_error;
				vmstate.vm_register [VM_REG_B] = vmstate.vm_stack [vmstate.stack_pos];
				break;
			case OP_add:
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] + vmstate.vm_register [VM_REG_A];
				break;
			case OP_sub_BA:
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] - vmstate.vm_register [VM_REG_A];
				break;
			case OP_sub_AB:
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_A] - vmstate.vm_register [VM_REG_B];
				break;
			case OP_mul:
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] * vmstate.vm_register [VM_REG_A];
				break;
			case OP_div_BA:
// division costs an extra instruction:
		  vmstate.instructions_left --;
				if (vmstate.vm_register [VM_REG_A] == 0)
					vmstate.vm_register [VM_REG_A] = 0;
				  else
   				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] / vmstate.vm_register [VM_REG_A];
				break;
			case OP_div_AB:
// division costs an extra instruction:
		  vmstate.instructions_left --;
				if (vmstate.vm_register [VM_REG_B] == 0)
					vmstate.vm_register [VM_REG_A] = 0;
				  else
   				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_A] / vmstate.vm_register [VM_REG_B];
				break;
			case OP_mod_BA:
		  vmstate.instructions_left --;
				if (vmstate.vm_register [VM_REG_A] == 0)
				{
//					fpr("\n core %i template %i mod by 0 at bcode %i", core->index, core->template_index, vmstate.bcode_pos);
					vmstate.vm_register [VM_REG_A] = 0;
				}
				  else
   				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] % vmstate.vm_register [VM_REG_A];
				break;
			case OP_mod_AB:
		  vmstate.instructions_left --;
				if (vmstate.vm_register [VM_REG_B] == 0)
				{
//					fpr("\n core %i template %i mod by 0 at bcode %i", core->index, core->template_index, vmstate.bcode_pos);
					vmstate.vm_register [VM_REG_A] = 0;
				}
				  else
   				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_A] % vmstate.vm_register [VM_REG_B];
				break;
			case OP_not:
				vmstate.vm_register [VM_REG_A] = ~vmstate.vm_register [VM_REG_A];
				break;
			case OP_and:
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] & vmstate.vm_register [VM_REG_A];
				break;
			case OP_or:
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] | vmstate.vm_register [VM_REG_A];
				break;
			case OP_xor:
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] ^ vmstate.vm_register [VM_REG_A];
				break;
			case OP_lsh_BA:
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] << vmstate.vm_register [VM_REG_A];
				break;
			case OP_lsh_AB:
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_A] << vmstate.vm_register [VM_REG_B];
				break;
			case OP_rsh_BA:
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] >> vmstate.vm_register [VM_REG_A];
				break;
			case OP_rsh_AB:
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_A] >> vmstate.vm_register [VM_REG_B];
				break;
			case OP_lnot:
				vmstate.vm_register [VM_REG_A] = !vmstate.vm_register [VM_REG_A];
				break;
			case OP_setA_num:
		  vmstate.instructions_left --;
				get_next_instr;
				vmstate.vm_register [VM_REG_A] = instr;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				break;
			case OP_setA_mem:
		  vmstate.instructions_left --;
				get_next_instr_expect_address;
				vmstate.vm_register [VM_REG_A] = memory [instr];
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				break;
			case OP_copyA_to_mem:
		  vmstate.instructions_left --;
				get_next_instr_expect_address;
				memory [instr] = vmstate.vm_register [VM_REG_A];
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				break;
			case OP_push_num:
		  vmstate.instructions_left --;
				get_next_instr;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				vmstate.vm_stack [vmstate.stack_pos++] = instr;
				if (vmstate.stack_pos >= VM_STACK_SIZE)
					goto stack_full_error;
				break;
			case OP_push_mem:
		  vmstate.instructions_left --;
				get_next_instr_expect_address;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				vmstate.vm_stack [vmstate.stack_pos++] = memory [instr];
				if (vmstate.stack_pos >= VM_STACK_SIZE)
					goto stack_full_error;
				break;
			case OP_incr_mem:
		  vmstate.instructions_left --;
				get_next_instr_expect_address;
				memory [instr] ++;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				break;
			case OP_decr_mem:
		  vmstate.instructions_left --;
				get_next_instr_expect_address;
				memory [instr] --;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				break;

			case OP_jump_num:
		  vmstate.instructions_left --;
				get_next_instr;
#ifdef SHOW_BCODE
    fpr(" %i(- 1)", instr);
#endif
				if (instr < 0 || instr >= BCODE_POS_MAX)
					goto jump_target_bounds_error;
				vmstate.bcode_pos = instr - 1;
				break;
			case OP_jumpA:
				if (vmstate.vm_register [VM_REG_A] < 0 || vmstate.vm_register [VM_REG_A] >= BCODE_POS_MAX)
					goto jump_target_bounds_error;
				vmstate.bcode_pos = vmstate.vm_register [VM_REG_A] - 1;
				break;

			case OP_comp_eq:
				if (vmstate.vm_register [VM_REG_B] == vmstate.vm_register [VM_REG_A]) // note B before A
					vmstate.vm_register [VM_REG_A] = 1;
				  else
						 vmstate.vm_register [VM_REG_A] = 0;
				break;
			case OP_comp_gr:
				if (vmstate.vm_register [VM_REG_B] > vmstate.vm_register [VM_REG_A]) // note B before A
					vmstate.vm_register [VM_REG_A] = 1;
				  else
						 vmstate.vm_register [VM_REG_A] = 0;
				break;
			case OP_comp_greq:
				if (vmstate.vm_register [VM_REG_B] >= vmstate.vm_register [VM_REG_A]) // note B before A
					vmstate.vm_register [VM_REG_A] = 1;
				  else
						 vmstate.vm_register [VM_REG_A] = 0;
				break;
			case OP_comp_ls:
				if (vmstate.vm_register [VM_REG_B] < vmstate.vm_register [VM_REG_A]) // note B before A
					vmstate.vm_register [VM_REG_A] = 1;
				  else
						 vmstate.vm_register [VM_REG_A] = 0;
				break;
			case OP_comp_lseq:
				if (vmstate.vm_register [VM_REG_B] <= vmstate.vm_register [VM_REG_A]) // note B before A
					vmstate.vm_register [VM_REG_A] = 1;
				  else
						 vmstate.vm_register [VM_REG_A] = 0;
				break;
			case OP_comp_neq:
				if (vmstate.vm_register [VM_REG_B] != vmstate.vm_register [VM_REG_A]) // note B before A
					vmstate.vm_register [VM_REG_A] = 1;
				  else
						 vmstate.vm_register [VM_REG_A] = 0;
				break;

			case OP_mulA_num:
		  vmstate.instructions_left --;
				get_next_instr;
				vmstate.vm_register [VM_REG_A] *= instr;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				break;
			case OP_addA_num:
		  vmstate.instructions_left --;
				get_next_instr;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				vmstate.vm_register [VM_REG_A] += instr;
				break;
			case OP_derefA:
				if (vmstate.vm_register [VM_REG_A] < 0
					|| vmstate.vm_register [VM_REG_A] >= MEMORY_SIZE)
						goto invalid_derefA_error;
				vmstate.vm_register [VM_REG_A] = memory [vmstate.vm_register [VM_REG_A]];
				break;
			case OP_copyA_to_derefB:
				if (vmstate.vm_register [VM_REG_B] < 0
				 || vmstate.vm_register [VM_REG_B] >= MEMORY_SIZE)
						goto invalid_derefB_error;
				memory [vmstate.vm_register [VM_REG_B]] = vmstate.vm_register [VM_REG_A];
				break;
			case OP_incr_derefA:
				if (vmstate.vm_register [VM_REG_A] < 0
					|| vmstate.vm_register [VM_REG_A] >= MEMORY_SIZE)
						goto invalid_derefA_error;
				memory [vmstate.vm_register [VM_REG_A]] ++;
				break;
			case OP_decr_derefA:
				if (vmstate.vm_register [VM_REG_A] < 0
					|| vmstate.vm_register [VM_REG_A] >= MEMORY_SIZE)
						goto invalid_derefA_error;
				memory [vmstate.vm_register [VM_REG_A]] --;
				break;
			case OP_copyAtoB:
				vmstate.vm_register [VM_REG_B] = vmstate.vm_register [VM_REG_A];
				break;
   case OP_deref_stack_toA:
				if (vmstate.stack_pos <= 0)
					goto stack_below_zero_error;
				if (vmstate.vm_stack [vmstate.stack_pos - 1] < 0
					|| vmstate.vm_stack [vmstate.stack_pos - 1] >= MEMORY_SIZE)
					goto memory_address_error;
				vmstate.vm_register [VM_REG_A] = memory [vmstate.vm_stack [vmstate.stack_pos - 1]];
// note: does not change the stack pointer, so the value stays on the stack
				break;

   case OP_push_return_address:
				vmstate.vm_stack [vmstate.stack_pos++] = vmstate.bcode_pos + 2;
				if (vmstate.stack_pos >= VM_STACK_SIZE)
					goto stack_full_error;
   	break;
   case OP_return_sub:
		  vmstate.instructions_left --;
				vmstate.stack_pos --;
				if (vmstate.stack_pos <= 0)
					goto stack_below_zero_error;
				instr = vmstate.vm_stack [vmstate.stack_pos];
				if (instr < 0
					|| instr >= BCODE_POS_MAX)
					goto return_sub_bounds_error;
				vmstate.bcode_pos = instr;
				break;

			case OP_switchA:
		  vmstate.instructions_left -= 5;
// switchA instruction should be followed by three operands: address of start of jump table, lowest case value, highest case value.
    get_next_instr; // TO DO: optimise these!!
    value [0] = instr;
    get_next_instr;
    value [1] = instr;
    get_next_instr;
    value [2] = instr;
#ifdef SHOW_BCODE
    fpr("\nswitch %i %i %i  A=%i", value [0], value [1], value [2], vmstate.vm_register [VM_REG_A]);
#endif
    int jump_table_entry_address = value [0] - 1; // this address should hold address of default case code
    if (vmstate.vm_register [VM_REG_A] >= value [1]
					&& vmstate.vm_register [VM_REG_A] <= value [2])
				{
					jump_table_entry_address = value [0] + vmstate.vm_register [VM_REG_A] - value [1];
				}
#ifdef SHOW_BCODE
				fpr(" jump_table_entry_address %i ", jump_table_entry_address);
#endif
				if (jump_table_entry_address < 0
					|| jump_table_entry_address >= BCODE_MAX - 8)
					goto switch_jump_table_error; // compiler should prevent this from happening in properly compiled code.
				int target_code_address = vmstate.bcode->op [jump_table_entry_address];
#ifdef SHOW_BCODE
				fpr(" target_code_address %i ", target_code_address);
#endif
				vmstate.bcode_pos = target_code_address - 1;
				if (vmstate.bcode_pos < 0
			  || vmstate.bcode_pos >= BCODE_MAX - 8)
						goto switch_jump_table_error;
				break;

// remove:
			case OP_pcomp_eq:
				break;
			case OP_pcomp_neq:
				break;

			case OP_iftrue_jump:
		  vmstate.instructions_left --;
				get_next_instr;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				if (instr < 0 || instr >= BCODE_POS_MAX)
					goto jump_target_bounds_error;
				if (vmstate.vm_register [VM_REG_A] != 0)
				 vmstate.bcode_pos = instr - 1;
				break;
			case OP_iffalse_jump:
		  vmstate.instructions_left --;
				get_next_instr;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				if (instr < 0 || instr >= BCODE_POS_MAX)
					goto jump_target_bounds_error;
				if (vmstate.vm_register [VM_REG_A] == 0)
				 vmstate.bcode_pos = instr - 1;
				break;

			case OP_print:
				{
//				fpr("\nprint ");
				i = 0;
				int max_length = STRING_MAX_LENGTH - 2;
				vmstate.bcode_pos++; // skip past the print instruction
// let's just bounds-check once:
				if (vmstate.bcode_pos >= BCODE_MAX - STRING_MAX_LENGTH - 1)
					max_length = BCODE_MAX - vmstate.bcode_pos - 2;
				while (i < max_length
								&& vmstate.bcode->op [vmstate.bcode_pos] != 0)
				{
					print_string [i] = vmstate.bcode->op [vmstate.bcode_pos];
					i++;
					vmstate.bcode_pos++;
				}
				print_string [i] = '\0';
				sancheck(i, 0, STRING_MAX_LENGTH, "print_string length");
				int source_index = -1;
				int source_created_timestamp = 0;
				if (core != NULL)
				{
					source_index = core->index;
					source_created_timestamp = core->created_timestamp;
				}
    write_text_to_console(CONSOLE_GENERAL, PRINT_COL_WHITE, source_index, source_created_timestamp, print_string);
//				fpr("[%s]", print_string);
				}
				break;

			case OP_printA:
				{
					sprintf(print_string, "%i", vmstate.vm_register [VM_REG_A]);
//				fpr(" [A=%i] ", vmstate.vm_register [VM_REG_A]);
 				int source_index2 = -1;
 				int source_created_timestamp = 0;
				 if (core != NULL)
					{
					 source_index2 = core->index;
 					source_created_timestamp = core->created_timestamp;
					}
     write_text_to_console(CONSOLE_GENERAL, PRINT_COL_WHITE, source_index2, source_created_timestamp, print_string);
				}
				break;



			case OP_bubble:
				{
//				fpr("\nprint ");
				i = 0;
				int max_length = BUBBLE_TEXT_LENGTH_MAX - 2;
				vmstate.bcode_pos++; // skip past the print instruction
// let's just bounds-check once:
				if (vmstate.bcode_pos >= BCODE_MAX - BUBBLE_TEXT_LENGTH_MAX - 1)
					max_length = BCODE_MAX - vmstate.bcode_pos - 2;
				while (i < max_length
								&& vmstate.bcode->op [vmstate.bcode_pos] != 0)
				{
					print_string [i] = vmstate.bcode->op [vmstate.bcode_pos];
					i++;
					vmstate.bcode_pos++;
				}
				print_string [i] = '\0';
				sancheck(i, 0, BUBBLE_TEXT_LENGTH_MAX, "print_string (bubble) length");
    write_text_to_bubble(core->index, w.world_time, print_string);
//				fpr("[%s]", print_string);
				}
				break;

			case OP_bubbleA:
				{
					sprintf(print_string, "%i", vmstate.vm_register [VM_REG_A]);
//				fpr(" [A=%i] ", vmstate.vm_register [VM_REG_A]);
     write_text_to_bubble(core->index, w.world_time, print_string);
				}
				break;

			case OP_call_object:
		  vmstate.instructions_left --;
				get_next_instr; // instr is bounds-checked in call_object()
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
#ifdef DEBUG_MODE
				if (core == NULL) // currently NULL is used for debugging
					goto object_called_by_non_core_error;
#endif
				vmstate.vm_register [VM_REG_A] = call_object_method(core, instr); // call_object also uses vmstate to read other values from the stack
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_member:
		  vmstate.instructions_left --;
				get_next_instr; // instr is bounds-checked in call_object()
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
#ifdef DEBUG_MODE
				if (core == NULL) // currently NULL is used for debugging
					goto member_called_by_non_core_error;
#endif
				vmstate.vm_register [VM_REG_A] = call_self_member_method(core, instr);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_core:
		  vmstate.instructions_left --;
				get_next_instr; // instr is bounds-checked in call_core_method()
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
#ifdef DEBUG_MODE
				if (core == NULL) // this should never happen
					goto core_called_by_non_core_error;
#endif
				vmstate.vm_register [VM_REG_A] = call_self_core_method(core, instr);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_extern_member:
		  vmstate.instructions_left --;
				get_next_instr; // instr is bounds-checked in call_object()
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
// note that core passed to call_extern_member_method() is the calling core, not the target core. May be the same as the target core, or may be NULL.
				vmstate.vm_register [VM_REG_A] = call_extern_member_method(core, instr);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_extern_core:
		  vmstate.instructions_left --;
				get_next_instr; // instr is bounds-checked in call_object()
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
// note that core passed to call_extern_core_method() is the calling core, not the target core. May be the same as the target core, or may be NULL.
				vmstate.vm_register [VM_REG_A] = call_extern_core_method(core, instr);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_std:
		  vmstate.instructions_left --;
				get_next_instr; // method type (is bounds-checked in call_object())
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				vmstate.vm_register [VM_REG_A] = call_std_method(core, instr, 0);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_std_var:
				{
		   vmstate.instructions_left --;
				 get_next_instr; // method type
				 int method_type = instr;
				 get_next_instr; // number of parameters
#ifdef SHOW_BCODE
    fpr(" %i %i", method_type, instr);
#endif
				 vmstate.vm_register [VM_REG_A] = call_std_method(core, method_type, instr);
				 if (vmstate.error_state)
 					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				}
				break;

			case OP_call_uni:
		  vmstate.instructions_left --;
				get_next_instr; // instr is bounds-checked in call_object()
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				vmstate.vm_register [VM_REG_A] = call_uni_method(core, instr);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_class:
		  vmstate.instructions_left --;
				get_next_instr;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				vmstate.vm_register [VM_REG_A] = call_class_method(core, instr);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_stop:
				goto finished_execution;

			case OP_terminate:
				core->self_destruct = 1; // core will self-destruct when this function returns
				goto finished_execution;

   default:
				goto invalid_instruction_error;

		}

//fpr("\n A %i B %i mem %i,%i,%i,%i,%i stack %i,%i,%i,%i,%i (pointer %i)", vmstate.vm_register[0], vmstate.vm_register[1], memory[0],memory[1],memory[2],memory[3],memory[4], vmstate.vm_stack[0],vmstate.vm_stack[1],vmstate.vm_stack[2],vmstate.vm_stack[3],vmstate.vm_stack[4],vmstate.stack_pos);

	};

finished_execution:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
//	fpr("\n core %i instructions_left %i (used %i)", core->index, vmstate.instructions_left, INSTRUCTION_COUNT - vmstate.instructions_left);
 return; // success

bcode_bounds_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("execution out of bounds", 0, 0);
 return;

memory_address_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid memory access", 1, instr);
 return;

invalid_instruction_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid instruction", 1, instr);
 return;

out_of_instructions_error:
	core->instructions_used = core->instructions_per_cycle;// - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("instructions exhausted", 0, 0);
 return;

jump_target_bounds_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid jump target", 1, vmstate.bcode->op [vmstate.bcode_pos]);
 return;

stack_full_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("stack overflow", 0, 0);
 return;

stack_below_zero_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("stack base reached", 0, 0);
 return;

#ifdef DEBUG_MODE
core_called_by_non_core_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("INVALID ERROR?!", 0, 0); // shouldn't happen - no non-core programs
// fpr("\nError: self core method called by non-core program at bcode %i", vmstate.bcode_pos);
 return;

member_called_by_non_core_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("INVALID ERROR?!", 0, 0); // shouldn't happen - no non-core programs
// fpr("\nError: self member method called by non-core program at bcode %i", vmstate.bcode_pos);
 return;

object_called_by_non_core_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("INVALID ERROR?!", 0, 0); // shouldn't happen - no non-core programs
// fpr("\nError: self object method called by non-core program at bcode %i", vmstate.bcode_pos);
 return;
#endif

invalid_derefB_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid B dereference", 1, vmstate.vm_register [VM_REG_B]);
// fpr("\nError: register B dereference is out of bounds (%i) at bcode %i", vmstate.vm_register [VM_REG_B], vmstate.bcode_pos);
 return;

invalid_derefA_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid A dereference", 1, vmstate.vm_register [VM_REG_A]);
// fpr("\nError: register A dereference is out of bounds (%i) at bcode %i", vmstate.vm_register [VM_REG_A], vmstate.bcode_pos);
 return;

return_sub_bounds_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid return address", 1, instr);
//	fpr("\nError: subroutine return value %i out of bounds", instr);
	return;

switch_jump_table_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	 if (w.debug_mode)
		 print_execution_error("error in switch jump table", 0, 0); // this should be caught by the compiler
		return;

generic_error:
	core->instructions_used = core->instructions_per_cycle - vmstate.instructions_left;
	return; // used when e.g. calling an object causes a fatal error, and an error essage has already been written.

}


static void	print_execution_error(const char* error_message, int values, int value1)
{

 static char ex_error_string [120];

 if (vmstate.bcode_pos < 0
		|| vmstate.bcode_pos >= BCODE_MAX)
	{
// if bcode_pos is out of bounds, don't want to try to display the source line:
	 if (values)
	  sprintf(ex_error_string, "Error [%s:%i] at invalid bcode %i.", error_message, value1, vmstate.bcode_pos);
	   else
	    sprintf(ex_error_string, "Error [%s] at invalid bcode %i.", error_message, vmstate.bcode_pos);
	}
	 else
		{

  	if (values)
  	 sprintf(ex_error_string, "Error [%s:%i] at line %i (bcode %i).", error_message, value1, (int) vmstate.bcode->src_line [vmstate.bcode_pos] + 1, vmstate.bcode_pos);
	    else
	     sprintf(ex_error_string, "Error [%s] at line %i (bcode %i).", error_message, (int) vmstate.bcode->src_line [vmstate.bcode_pos] + 1, vmstate.bcode_pos);
		}

	write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LRED, vmstate.core->index, vmstate.core->created_timestamp, ex_error_string);


}

void init_bcode_execution_for_watch(struct core_struct* core, struct bcode_struct* bc, s16b* memory)
{

	vmstate.core = core;
	vmstate.bcode = bc;
	vmstate.bcode_pos = -1; // will be incremented before use
	vmstate.instructions_left = core->instructions_per_cycle;
	vmstate.memory = memory;
	vmstate.stack_pos = 1;
	vmstate.error_state = 0;

	vmstate.nearby_well_index = -2; // means this has not yet been calculated
	scanlist.current = 0; // means the scanlist will need to be built if a scanning function is called.

	bcp_state.subpanel_bcode_line = 0;
 slider_moved_to_value(&slider[SLIDER_BCODE_BCODE_SCROLLBAR_V], bcp_state.subpanel_bcode_line);

	int i;

	for (i = 0; i < VM_STACK_SIZE; i ++)
	{
		vmstate.vm_stack [i] = 0;
	}

	for (i = 0; i < VM_REGISTERS; i++)
	{
		vmstate.vm_register [i] = 0;
	}

	write_line_to_log("Execution initialised...", MLOG_COL_TEMPLATE);

}


// This function is called from the main game loop each frame, or from v_draw_panel.
void run_bcode_watch(void)
{

	struct template_debug_struct* tdb = &template_debug[bcp_state.player_index][bcp_state.template_index];

 switch(bcp_state.bcp_wait_mode)
 {
	 case BCP_WAIT_PAUSE:
	 	return;
		case BCP_WAIT_PAUSE_ONE_STEP:
		 bcp_state.bcp_wait_mode = BCP_WAIT_PAUSE;
		 break;
		case BCP_WAIT_STEP_SLOW:
	  bcp_state.step_counter --;
	  if (bcp_state.step_counter > 0)
  		return;
		 bcp_state.step_counter = 40;
		 break;
		case BCP_WAIT_STEP_FAST:
	  bcp_state.step_counter -= 3;
	  if (bcp_state.step_counter > 0)
  		return;
		 bcp_state.step_counter = 40;
		 break;

 }

	if (!execute_bcode_single_step_for_watch())
	{
//		game.pause_watch = 0; // will be set to 1 again at next cycle if game.watching is still WATCH_ON
		game.watching = WATCH_FINISHED_BUT_STILL_WATCHING; // may be reset after this function returns
		return;
	}

// not good:
 int i;

 for (i = 0; i < DEBUGGER_LINES; i ++)
	{
		if (tdb->debugger_line[i].bcode_address == vmstate.bcode_pos + 1)
			break;
	}

	if (i == DEBUGGER_LINES)
		return;

	int new_first_debugger_line = i - 10;
	if (new_first_debugger_line < 0)
		new_first_debugger_line = 0;

	bcp_state.subpanel_bcode_line = new_first_debugger_line;
 slider_moved_to_value(&slider[SLIDER_BCODE_BCODE_SCROLLBAR_V], bcp_state.subpanel_bcode_line);

}



// This function is called from the main game loop each frame, or from v_draw_panel.
void finish_executing_bcode_in_watch(void)
{


	while (execute_bcode_single_step_for_watch())
	{
	};

//		game.pause_watch = 0; // will be set to 1 again at next cycle if game.watching is still WATCH_ON

 	bcp_state.subpanel_bcode_line = 0;
  slider_moved_to_value(&slider[SLIDER_BCODE_BCODE_SCROLLBAR_V], bcp_state.subpanel_bcode_line);

		game.watching = WATCH_FINISHED_BUT_STILL_WATCHING; // may be reset after this function returns

}



// returns 0 if execution finished
int execute_bcode_single_step_for_watch(void)
{


	s16b instr;
	int value [3];
	s16b temp_val;
	int i;

	char log_line [150];
	log_line [0] = 0;
	char print_string [STRING_MAX_LENGTH];



		get_next_instr;

		vmstate.instructions_left --; // should be a cost depending on instruction type
		if (vmstate.instructions_left <= 0) // this is only checked here, not during operations. If instructions are exhausted during an operation, the operation will complete before the vm exits.
			goto out_of_instructions_error;

		int instruction_pos = vmstate.bcode_pos;

		switch(instr)
		{
		 case OP_nop:
		 	sprintf(log_line, "[%4d] nop (no operation)", instruction_pos);
			 break;
			case OP_pushA:
				vmstate.vm_stack [vmstate.stack_pos++] = vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] pushA (push %i from A to stack)", instruction_pos, vmstate.vm_register [VM_REG_A]);
				if (vmstate.stack_pos >= VM_STACK_SIZE)
					goto stack_full_error;
				break;
			case OP_popB:
				vmstate.stack_pos --;
				if (vmstate.stack_pos <= 0)
					goto stack_below_zero_error;
				vmstate.vm_register [VM_REG_B] = vmstate.vm_stack [vmstate.stack_pos];
		 	sprintf(log_line, "[%4d] popB (pop %i from stack to B)", instruction_pos, vmstate.vm_register [VM_REG_B]);
				break;
			case OP_add:
				temp_val = vmstate.vm_register [VM_REG_A];
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] + vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] add (A + B -> A: %i + %i = %i)", instruction_pos, vmstate.vm_register [VM_REG_B], temp_val, vmstate.vm_register [VM_REG_A]);
				break;
			case OP_sub_BA:
				temp_val = vmstate.vm_register [VM_REG_A];
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] - vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] sub_BA (B - A -> A: %i - %i = %i)", instruction_pos, vmstate.vm_register [VM_REG_B], temp_val, vmstate.vm_register [VM_REG_A]);
				break;
			case OP_sub_AB:
				temp_val = vmstate.vm_register [VM_REG_A];
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_A] - vmstate.vm_register [VM_REG_B];
		 	sprintf(log_line, "[%4d] sub_AB (A - B -> A: %i - %i = %i)", instruction_pos, temp_val, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
				break;
			case OP_mul:
				temp_val = vmstate.vm_register [VM_REG_A];
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] * vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] mul (A * B -> A: %i * %i = %i)", instruction_pos, temp_val, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
				break;
			case OP_div_BA:
				temp_val = vmstate.vm_register [VM_REG_A];
// division costs an extra instruction:
		  vmstate.instructions_left --;
				if (vmstate.vm_register [VM_REG_A] == 0)
					vmstate.vm_register [VM_REG_A] = 0;
				  else
   				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] / vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] div_BA (B / A -> A: %i / %i = %i)", instruction_pos, vmstate.vm_register [VM_REG_B], temp_val, vmstate.vm_register [VM_REG_A]);
				break;
			case OP_div_AB:
				temp_val = vmstate.vm_register [VM_REG_A];
// division costs an extra instruction:
		  vmstate.instructions_left --;
				if (vmstate.vm_register [VM_REG_B] == 0)
					vmstate.vm_register [VM_REG_A] = 0;
				  else
   				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_A] / vmstate.vm_register [VM_REG_B];
		 	sprintf(log_line, "[%4d] div_AB (A / B -> A: %i / %i = %i)", instruction_pos, temp_val, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
				break;
			case OP_mod_BA:
				temp_val = vmstate.vm_register [VM_REG_A];
		  vmstate.instructions_left --;
				if (vmstate.vm_register [VM_REG_A] == 0)
				{
//					fpr("\n core %i template %i mod by 0 at bcode %i", core->index, core->template_index, vmstate.bcode_pos);
					vmstate.vm_register [VM_REG_A] = 0;
				}
				  else
   				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] % vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] mod_BA (B %% A -> A: %i / %i = %i)", instruction_pos, vmstate.vm_register [VM_REG_B], temp_val, vmstate.vm_register [VM_REG_A]);
				break;
			case OP_mod_AB:
		  vmstate.instructions_left --;
				temp_val = vmstate.vm_register [VM_REG_A];
				if (vmstate.vm_register [VM_REG_B] == 0)
				{
//					fpr("\n core %i template %i mod by 0 at bcode %i", core->index, core->template_index, vmstate.bcode_pos);
					vmstate.vm_register [VM_REG_A] = 0;
				}
				  else
   				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_A] % vmstate.vm_register [VM_REG_B];
		 	sprintf(log_line, "[%4d] mod_AB (A %% B -> A: %i / %i = %i)", instruction_pos, temp_val, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
				break;
			case OP_not:
				temp_val = vmstate.vm_register [VM_REG_A];
				vmstate.vm_register [VM_REG_A] = ~vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] not (bitwise not: ~A -> A: ~%i = %i)", instruction_pos, temp_val, vmstate.vm_register [VM_REG_A]);
				break;
			case OP_and:
				temp_val = vmstate.vm_register [VM_REG_A];
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] & vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] and (A & B -> A: %i & %i = %i)", instruction_pos, temp_val, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
				break;
			case OP_or:
				temp_val = vmstate.vm_register [VM_REG_A];
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] | vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] or (A | B -> A: %i | %i = %i)", instruction_pos, temp_val, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
				break;
			case OP_xor:
				temp_val = vmstate.vm_register [VM_REG_A];
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] ^ vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] xor (A ^ B -> A: %i ^ %i = %i)", instruction_pos, temp_val, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
				break;
			case OP_lsh_BA:
				temp_val = vmstate.vm_register [VM_REG_A];
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] << vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] lsh_BA (B << A -> A: %i << %i = %i)", instruction_pos, vmstate.vm_register [VM_REG_B], temp_val, vmstate.vm_register [VM_REG_A]);
				break;
			case OP_lsh_AB:
				temp_val = vmstate.vm_register [VM_REG_A];
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_A] << vmstate.vm_register [VM_REG_B];
		 	sprintf(log_line, "[%4d] lsh_AB (A << B -> A: %i << %i = %i)", instruction_pos, temp_val, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
				break;
			case OP_rsh_BA:
				temp_val = vmstate.vm_register [VM_REG_A];
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_B] >> vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] rsh_BA (B >> A -> A: %i >> %i = %i)", instruction_pos, vmstate.vm_register [VM_REG_B], temp_val, vmstate.vm_register [VM_REG_A]);
				break;
			case OP_rsh_AB:
				temp_val = vmstate.vm_register [VM_REG_A];
				vmstate.vm_register [VM_REG_A] = vmstate.vm_register [VM_REG_A] >> vmstate.vm_register [VM_REG_B];
		 	sprintf(log_line, "[%4d] rsh_AB (A >> B -> A: %i >> %i = %i)", instruction_pos, temp_val, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
				break;
			case OP_lnot:
				temp_val = vmstate.vm_register [VM_REG_A];
				vmstate.vm_register [VM_REG_A] = !vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] lnot (logical not: !A -> A: !%i = %i)", instruction_pos, temp_val, vmstate.vm_register [VM_REG_A]);
				break;
			case OP_setA_num:
		  vmstate.instructions_left --;
				get_next_instr;
				vmstate.vm_register [VM_REG_A] = instr;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
		 	sprintf(log_line, "[%4d] setA_num %i (set A to %i)", instruction_pos, vmstate.vm_register [VM_REG_A], vmstate.vm_register [VM_REG_A]);
				break;
			case OP_setA_mem:
		  vmstate.instructions_left --;
				get_next_instr_expect_address;
				vmstate.vm_register [VM_REG_A] = vmstate.memory [instr];
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
		 	sprintf(log_line, "[%4d] setA_mem %i (set A to value at memory address %i: A = %i)", instruction_pos, instr, instr, vmstate.vm_register [VM_REG_A]);
				break;
			case OP_copyA_to_mem:
		  vmstate.instructions_left --;
				get_next_instr_expect_address;
				vmstate.memory [instr] = vmstate.vm_register [VM_REG_A];
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
		 	sprintf(log_line, "[%4d] copyA_to_mem %i (copy value %i from A to memory address %i)", instruction_pos, instr, vmstate.vm_register [VM_REG_A], instr);
				break;
			case OP_push_num:
		  vmstate.instructions_left --;
				get_next_instr;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				vmstate.vm_stack [vmstate.stack_pos++] = instr;
		 	sprintf(log_line, "[%4d] push_num %i (push %i to stack)", instruction_pos, instr, instr);
				if (vmstate.stack_pos >= VM_STACK_SIZE)
					goto stack_full_error;
				break;
			case OP_push_mem:
		  vmstate.instructions_left --;
				get_next_instr_expect_address;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				vmstate.vm_stack [vmstate.stack_pos++] = vmstate.memory [instr];
		 	sprintf(log_line, "[%4d] push_mem %i (push value %i at memory address %i to stack)", instruction_pos, instr, vmstate.memory [instr], instr);
				if (vmstate.stack_pos >= VM_STACK_SIZE)
					goto stack_full_error;
				break;
			case OP_incr_mem:
		  vmstate.instructions_left --;
				get_next_instr_expect_address;
				temp_val = vmstate.memory [instr];
				vmstate.memory [instr] ++;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
		 	sprintf(log_line, "[%4d] incr_mem %i (increment value at memory address %i: %i++ = %i)", instruction_pos, instr, instr, temp_val, vmstate.memory [instr]);
				break;
			case OP_decr_mem:
		  vmstate.instructions_left --;
				get_next_instr_expect_address;
				temp_val = vmstate.memory [instr];
				vmstate.memory [instr] --;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
		 	sprintf(log_line, "[%4d] decr_mem %i (decrement value at memory address %i: %i-- = %i)", instruction_pos, instr, instr, temp_val, vmstate.memory [instr]);
				break;

			case OP_jump_num:
		  vmstate.instructions_left --;
				get_next_instr;
#ifdef SHOW_BCODE
    fpr(" %i(- 1)", instr);
#endif
		 	sprintf(log_line, "[%4d] jump_num %i (jump to bcode address %i)", instruction_pos, instr, instr);
				if (instr < 0 || instr >= BCODE_POS_MAX)
					goto jump_target_bounds_error;
				vmstate.bcode_pos = instr - 1;
				break;
			case OP_jumpA:
				if (vmstate.vm_register [VM_REG_A] < 0 || vmstate.vm_register [VM_REG_A] >= BCODE_POS_MAX)
					goto jump_target_bounds_error;
		 	sprintf(log_line, "[%4d] jumpA (jump to bcode address in A (%i))", instruction_pos, vmstate.vm_register [VM_REG_A]); // not - 1 because the address will be incremented before being used
				vmstate.bcode_pos = vmstate.vm_register [VM_REG_A] - 1;
				break;

			case OP_comp_eq:
				if (vmstate.vm_register [VM_REG_B] == vmstate.vm_register [VM_REG_A]) // note B before A
				{
 		 	sprintf(log_line, "[%4d] comp_eq (A %i is equal to B %i, so set A to 1)", instruction_pos, vmstate.vm_register [VM_REG_A], vmstate.vm_register [VM_REG_B]);
					vmstate.vm_register [VM_REG_A] = 1;
				}
				  else
						{
   		 	sprintf(log_line, "[%4d] comp_eq (A %i is not equal to B %i, so set A to 0)", instruction_pos, vmstate.vm_register [VM_REG_A], vmstate.vm_register [VM_REG_B]);
						 vmstate.vm_register [VM_REG_A] = 0;
						}
				break;
			case OP_comp_gr:
				if (vmstate.vm_register [VM_REG_B] > vmstate.vm_register [VM_REG_A]) // note B before A
				{
 		 	sprintf(log_line, "[%4d] comp_gr (B %i is greater than A %i, so set A to 1)", instruction_pos, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
					vmstate.vm_register [VM_REG_A] = 1;
				}
				  else
						{
   		 	sprintf(log_line, "[%4d] comp_gr (B %i is not greater than A %i, so set A to 0)", instruction_pos, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
						 vmstate.vm_register [VM_REG_A] = 0;
						}
				break;
			case OP_comp_greq:
				if (vmstate.vm_register [VM_REG_B] >= vmstate.vm_register [VM_REG_A]) // note B before A
				{
 		 	sprintf(log_line, "[%4d] comp_greq (B %i is greater than or equal to A %i, so set A to 1)", instruction_pos, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
					vmstate.vm_register [VM_REG_A] = 1;
				}
				  else
						{
   		 	sprintf(log_line, "[%4d] comp_greq (B %i is not greater than or equal to A %i, so set A to 0)", instruction_pos, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
						 vmstate.vm_register [VM_REG_A] = 0;
						}
				break;
			case OP_comp_ls:
				if (vmstate.vm_register [VM_REG_B] < vmstate.vm_register [VM_REG_A]) // note B before A
				{
 		 	sprintf(log_line, "[%4d] comp_ls (B %i is less than A %i, so set A to 1)", instruction_pos, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
					vmstate.vm_register [VM_REG_A] = 1;
				}
				  else
						{
   		 	sprintf(log_line, "[%4d] comp_ls (B %i is not less than A %i, so set A to 0)", instruction_pos, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
						 vmstate.vm_register [VM_REG_A] = 0;
						}
				break;
			case OP_comp_lseq:
				if (vmstate.vm_register [VM_REG_B] <= vmstate.vm_register [VM_REG_A]) // note B before A
				{
 		 	sprintf(log_line, "[%4d] comp_lseq (B %i is less than or equal to A %i, so set A to 1)", instruction_pos, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
					vmstate.vm_register [VM_REG_A] = 1;
				}
				  else
						{
   		 	sprintf(log_line, "[%4d] comp_lseq (B %i is not less than or equal to A %i, so set A to 0)", instruction_pos, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
						 vmstate.vm_register [VM_REG_A] = 0;
						}
				break;
			case OP_comp_neq:
				if (vmstate.vm_register [VM_REG_B] != vmstate.vm_register [VM_REG_A]) // note B before A
				{
 		 	sprintf(log_line, "[%4d] comp_neq (B %i is not equal to A %i, so set A to 1)", instruction_pos, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
					vmstate.vm_register [VM_REG_A] = 1;
				}
				  else
						{
   		 	sprintf(log_line, "[%4d] comp_neq (B %i is equal to A %i, so set A to 0)", instruction_pos, vmstate.vm_register [VM_REG_B], vmstate.vm_register [VM_REG_A]);
						 vmstate.vm_register [VM_REG_A] = 0;
						}
				break;

			case OP_mulA_num:
				temp_val = vmstate.vm_register [VM_REG_A];
		  vmstate.instructions_left --;
				get_next_instr;
				vmstate.vm_register [VM_REG_A] *= instr;
		 	sprintf(log_line, "[%4d] mulA_num %i (A * %i -> A: %i * %i = %i)", instruction_pos,
												instr, instr, temp_val, instr, vmstate.vm_register [VM_REG_A]);
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				break;
			case OP_addA_num:
				temp_val = vmstate.vm_register [VM_REG_A];
		  vmstate.instructions_left --;
				get_next_instr;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				vmstate.vm_register [VM_REG_A] += instr;
		 	sprintf(log_line, "[%4d] addA_num %i (A + %i -> A: %i + %i = %i)", instruction_pos,
												instr, instr, temp_val, instr, vmstate.vm_register [VM_REG_A]);
				break;
			case OP_derefA:
				temp_val = vmstate.vm_register [VM_REG_A];
				if (vmstate.vm_register [VM_REG_A] < 0
					|| vmstate.vm_register [VM_REG_A] >= MEMORY_SIZE)
						goto invalid_derefA_error;
				vmstate.vm_register [VM_REG_A] = vmstate.memory [vmstate.vm_register [VM_REG_A]];
		 	sprintf(log_line, "[%4d] derefA (set A to the value (%i) in the memory address (%i) pointed to by A)", instruction_pos,
												vmstate.memory [vmstate.vm_register [VM_REG_A]], temp_val);
				break;
			case OP_copyA_to_derefB:
				if (vmstate.vm_register [VM_REG_B] < 0
				 || vmstate.vm_register [VM_REG_B] >= MEMORY_SIZE)
						goto invalid_derefB_error;
				vmstate.memory [vmstate.vm_register [VM_REG_B]] = vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] copyA_to_derefB (copy A (%i) to the memory address (%i) pointed to by B)", instruction_pos,
												vmstate.vm_register [VM_REG_A], vmstate.vm_register [VM_REG_B]);
				break;
			case OP_incr_derefA:
				if (vmstate.vm_register [VM_REG_A] < 0
					|| vmstate.vm_register [VM_REG_A] >= MEMORY_SIZE)
						goto invalid_derefA_error;
				temp_val = vmstate.memory [vmstate.vm_register [VM_REG_A]];
				vmstate.memory [vmstate.vm_register [VM_REG_A]] ++;
		 	sprintf(log_line, "[%4d] incr_derefA (increment the memory address (%i) pointed to by A: %i++ = %i)", instruction_pos,
												vmstate.vm_register [VM_REG_A], temp_val, vmstate.memory [vmstate.vm_register [VM_REG_A]]);
				break;
			case OP_decr_derefA:
				if (vmstate.vm_register [VM_REG_A] < 0
					|| vmstate.vm_register [VM_REG_A] >= MEMORY_SIZE)
						goto invalid_derefA_error;
				temp_val = vmstate.memory [vmstate.vm_register [VM_REG_A]];
				vmstate.memory [vmstate.vm_register [VM_REG_A]] --;
		 	sprintf(log_line, "[%4d] decr_derefA (decrement the memory address (%i) pointed to by A: %i-- = %i)", instruction_pos,
												vmstate.vm_register [VM_REG_A], temp_val, vmstate.memory [vmstate.vm_register [VM_REG_A]]);
				break;
			case OP_copyAtoB:
				vmstate.vm_register [VM_REG_B] = vmstate.vm_register [VM_REG_A];
		 	sprintf(log_line, "[%4d] copyAtoB (copy A (%i) to B)", instruction_pos,
												vmstate.vm_register [VM_REG_A]);
				break;
   case OP_deref_stack_toA:
				if (vmstate.stack_pos <= 0)
					goto stack_below_zero_error;
				if (vmstate.vm_stack [vmstate.stack_pos - 1] < 0
					|| vmstate.vm_stack [vmstate.stack_pos - 1] >= MEMORY_SIZE)
					goto memory_address_error;
				vmstate.vm_register [VM_REG_A] = vmstate.memory [vmstate.vm_stack [vmstate.stack_pos - 1]];
		 	sprintf(log_line, "[%4d] deref_stack_toA (copy the memory address (%i) pointed to by the front of the stack to A: %i -> A)", instruction_pos,
												vmstate.vm_stack [vmstate.stack_pos - 1], vmstate.vm_register [VM_REG_A]);
// note: does not change the stack pointer, so the value stays on the stack
				break;

   case OP_push_return_address:
				vmstate.vm_stack [vmstate.stack_pos++] = vmstate.bcode_pos + 2;
		 	sprintf(log_line, "[%4d] push_return_address (push the following bcode address (%i) to the stack)", instruction_pos,
												vmstate.bcode_pos + 2);
				if (vmstate.stack_pos >= VM_STACK_SIZE)
					goto stack_full_error;
   	break;
   case OP_return_sub:
		  vmstate.instructions_left --;
				vmstate.stack_pos --;
				if (vmstate.stack_pos <= 0)
					goto stack_below_zero_error;
				instr = vmstate.vm_stack [vmstate.stack_pos];
				if (instr < 0
					|| instr >= BCODE_POS_MAX)
					goto return_sub_bounds_error;
		 	sprintf(log_line, "[%4d] return_sub (jump to the value at the front of the stack (%i))", instruction_pos,
												instr);
				vmstate.bcode_pos = instr;
				break;

			case OP_switchA:
		  vmstate.instructions_left -= 5;
// switchA instruction should be followed by three operands: address of start of jump table, lowest case value, highest case value.
    get_next_instr; // TO DO: optimise these!!
    value [0] = instr;
    get_next_instr;
    value [1] = instr;
    get_next_instr;
    value [2] = instr;
		 	sprintf(log_line, "[%4d] switchA %i %i %i (jump table is at %i; lowest case is %i; highest case is %i)", instruction_pos,
												value [0], value [1], value [2], value [0], value [1], value [2]);
				write_line_to_log(log_line, MLOG_COL_COMPILER);
#ifdef SHOW_BCODE
    fpr("\nswitch %i %i %i  A=%i", value [0], value [1], value [2], vmstate.vm_register [VM_REG_A]);
#endif
    int jump_table_entry_address = value [0] - 1; // this address should hold address of default case code
    if (vmstate.vm_register [VM_REG_A] >= value [1]
					&& vmstate.vm_register [VM_REG_A] <= value [2])
				{
					jump_table_entry_address = value [0] + vmstate.vm_register [VM_REG_A] - value [1];
 		 	sprintf(log_line, "       (switched value A is %i; jump table address is %i)",
	 											vmstate.vm_register [VM_REG_A], jump_table_entry_address);
		 		write_line_to_log(log_line, MLOG_COL_COMPILER);
				}
				 else
					{
 		 	 sprintf(log_line, "       (switched value A is %i (default); jump table address for default is %i)",
	 				 							vmstate.vm_register [VM_REG_A], jump_table_entry_address);
		 		 write_line_to_log(log_line, MLOG_COL_COMPILER);
					}
#ifdef SHOW_BCODE
				fpr(" jump_table_entry_address %i ", jump_table_entry_address);
#endif
				if (jump_table_entry_address < 0
					|| jump_table_entry_address >= BCODE_MAX - 8)
					goto switch_jump_table_error; // compiler should prevent this from happening in properly compiled code.
				int target_code_address = vmstate.bcode->op [jump_table_entry_address];
#ifdef SHOW_BCODE
				fpr(" target_code_address %i ", target_code_address);
#endif
				vmstate.bcode_pos = target_code_address - 1;
		 	sprintf(log_line, "       (jump to %i)", vmstate.bcode_pos);
//				write_line_to_log(log_line, MLOG_COL_COMPILER); // done later
				if (vmstate.bcode_pos < 0
			  || vmstate.bcode_pos >= BCODE_MAX - 8)
						goto switch_jump_table_error;
				break;

// remove:
			case OP_pcomp_eq:
				break;
			case OP_pcomp_neq:
				break;

			case OP_iftrue_jump:
		  vmstate.instructions_left --;
				get_next_instr;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				if (instr < 0 || instr >= BCODE_POS_MAX)
					goto jump_target_bounds_error;
				if (vmstate.vm_register [VM_REG_A] != 0)
				{
 		 	sprintf(log_line, "[%4d] iftrue_jump %i (A %i is non-zero, so jump to %i)", instruction_pos,
	 											instr, vmstate.vm_register [VM_REG_A], instr);
				 vmstate.bcode_pos = instr - 1;
				}
				 else
					{
   		 	sprintf(log_line, "[%4d] iftrue_jump %i (A is zero, so do not jump)", instruction_pos, instr);
					}
				break;
			case OP_iffalse_jump:
		  vmstate.instructions_left --;
				get_next_instr;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				if (instr < 0 || instr >= BCODE_POS_MAX)
					goto jump_target_bounds_error;
				if (vmstate.vm_register [VM_REG_A] == 0)
				{
 		 	sprintf(log_line, "[%4d] iffalse_jump %i (A is zero, so jump to %i)", instruction_pos,
	 											instr, instr);
				 vmstate.bcode_pos = instr - 1;
				}
				 else
					{
   		 	sprintf(log_line, "[%4d] iffalse_jump %i (A is non-zero, so do not jump)", instruction_pos, instr);
					}
				break;

			case OP_print:
				{
//				fpr("\nprint ");
				i = 0;
				int max_length = STRING_MAX_LENGTH - 2;
				vmstate.bcode_pos++; // skip past the print instruction
// let's just bounds-check once:
				if (vmstate.bcode_pos >= BCODE_MAX - STRING_MAX_LENGTH - 1)
					max_length = BCODE_MAX - vmstate.bcode_pos - 2;
				while (i < max_length
								&& vmstate.bcode->op [vmstate.bcode_pos] != 0)
				{
					print_string [i] = vmstate.bcode->op [vmstate.bcode_pos];
					i++;
					vmstate.bcode_pos++;
				}
				print_string [i] = '\0';
				sancheck(i, 0, STRING_MAX_LENGTH, "print_string length");
				int source_index = -1;
				int source_created_timestamp = 0;
				if (vmstate.core != NULL)
				{
					source_index = vmstate.core->index;
					source_created_timestamp = vmstate.core->created_timestamp;
				}
    write_text_to_console(CONSOLE_GENERAL, PRINT_COL_WHITE, source_index, source_created_timestamp, print_string);
		 	sprintf(log_line, "[%4d] print (print following string to console)", instruction_pos);
//				fpr("[%s]", print_string);
				}
				break;

			case OP_printA:
				{
					sprintf(print_string, "%i", vmstate.vm_register [VM_REG_A]);
//				fpr(" [A=%i] ", vmstate.vm_register [VM_REG_A]);
 				int source_index2 = -1;
 				int source_created_timestamp = 0;
				 if (vmstate.core != NULL)
					{
					 source_index2 = vmstate.core->index;
 					source_created_timestamp = vmstate.core->created_timestamp;
					}
     write_text_to_console(CONSOLE_GENERAL, PRINT_COL_WHITE, source_index2, source_created_timestamp, print_string);
 		 	sprintf(log_line, "[%4d] printA (print A %i to console)", instruction_pos, vmstate.vm_register [VM_REG_A]);
				}
				break;



			case OP_bubble:
				{
//				fpr("\nprint ");
				i = 0;
				int max_length = BUBBLE_TEXT_LENGTH_MAX - 2;
				vmstate.bcode_pos++; // skip past the print instruction
// let's just bounds-check once:
				if (vmstate.bcode_pos >= BCODE_MAX - BUBBLE_TEXT_LENGTH_MAX - 1)
					max_length = BCODE_MAX - vmstate.bcode_pos - 2;
				while (i < max_length
								&& vmstate.bcode->op [vmstate.bcode_pos] != 0)
				{
					print_string [i] = vmstate.bcode->op [vmstate.bcode_pos];
					i++;
					vmstate.bcode_pos++;
				}
				print_string [i] = '\0';
				sancheck(i, 0, BUBBLE_TEXT_LENGTH_MAX, "print_string (bubble) length");
    write_text_to_bubble(vmstate.core->index, w.world_time, print_string);
		 	sprintf(log_line, "[%4d] bubble (print following string to bubble)", instruction_pos);
//				fpr("[%s]", print_string);
				}
				break;

			case OP_bubbleA:
				{
					sprintf(print_string, "%i", vmstate.vm_register [VM_REG_A]);
//				fpr(" [A=%i] ", vmstate.vm_register [VM_REG_A]);
     write_text_to_bubble(vmstate.core->index, w.world_time, print_string);
 		 	sprintf(log_line, "[%4d] bubbleA (print A %i to bubble)", instruction_pos, vmstate.vm_register [VM_REG_A]);
				}
				break;

			case OP_call_object:
		  vmstate.instructions_left --;
				get_next_instr; // instr is bounds-checked in call_object()
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
#ifdef DEBUG_MODE
				if (vmstate.core == NULL) // currently NULL is used for debugging
					goto object_called_by_non_core_error;
#endif
				if (instr >= 0 && instr < CALL_TYPES)
				{
		 	 sprintf(log_line, "[%4d] call_object %i (call object method %i (%s))", instruction_pos, instr, instr, identifier[call_type[instr].keyword_index].name);
    	write_line_to_log(log_line, MLOG_COL_COMPILER);
//    	if (call_type[instr].parameters > 0)
      print_method_parameters(call_type[instr].parameters + 2, log_line, "component ", "object "); // + 2 is for component, object
//       else
//								sprintf(log_line, "       (no parameters)");
				}
				 else
	 	 	 sprintf(log_line, "[%4d] call_object %i (call object method %i - invalid method?)", instruction_pos, instr, instr);
				write_line_to_log(log_line, MLOG_COL_COMPILER);
				vmstate.vm_register [VM_REG_A] = call_object_method(vmstate.core, instr); // call_object also uses vmstate to read other values from the stack
				sprintf(log_line, "       (return value %i -> A)", vmstate.vm_register [VM_REG_A]);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_member:
		  vmstate.instructions_left --;
				get_next_instr; // instr is bounds-checked in call_object()
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
#ifdef DEBUG_MODE
				if (vmstate.core == NULL) // currently NULL is used for debugging
					goto member_called_by_non_core_error;
#endif
				if (instr >= 0 && instr < MMETHOD_CALL_TYPES)
				{
		 	 sprintf(log_line, "[%4d] call_member %i (call component method %i (%s))", instruction_pos, instr, instr, identifier[mmethod_call_type[instr].keyword_index].name);
    	write_line_to_log(log_line, MLOG_COL_COMPILER);
//    	if (mmethod_call_type[instr].parameters > 0)
      print_method_parameters(mmethod_call_type[instr].parameters + 1, log_line, "component ", ""); // + 1 is for component index
//       else
//								sprintf(log_line, "       (no parameters)");
				}
				 else
	 	 	 sprintf(log_line, "[%4d] call_member %i (call component method %i - invalid method?)", instruction_pos, instr, instr);
				write_line_to_log(log_line, MLOG_COL_COMPILER);
				vmstate.vm_register [VM_REG_A] = call_self_member_method(vmstate.core, instr);
				sprintf(log_line, "       (return value %i -> A)", vmstate.vm_register [VM_REG_A]);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_core:
		  vmstate.instructions_left --;
				get_next_instr; // instr is bounds-checked in call_core_method()
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
#ifdef DEBUG_MODE
				if (vmstate.core == NULL) // this should never happen
					goto core_called_by_non_core_error;
#endif
				if (instr >= 0 && instr < CMETHOD_CALL_TYPES)
				{
		 	 sprintf(log_line, "[%4d] call_core %i (call process method %i (%s))", instruction_pos, instr, instr, identifier[cmethod_call_type[instr].keyword_index].name);
    	write_line_to_log(log_line, MLOG_COL_COMPILER);
    	if (cmethod_call_type[instr].parameters > 0)
      print_method_parameters(cmethod_call_type[instr].parameters, log_line, "", "");
       else
								sprintf(log_line, "       (no parameters)");
				}
				 else
	 	 	 sprintf(log_line, "[%4d] call_core %i (call process method %i - invalid method?)", instruction_pos, instr, instr);
				write_line_to_log(log_line, MLOG_COL_COMPILER);
				vmstate.vm_register [VM_REG_A] = call_self_core_method(vmstate.core, instr);
				sprintf(log_line, "       (return value %i -> A)", vmstate.vm_register [VM_REG_A]);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_extern_member:
		  vmstate.instructions_left --;
				get_next_instr; // instr is bounds-checked in call_object()
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				if (instr >= 0 && instr < MMETHOD_CALL_TYPES)
				{
		 	 sprintf(log_line, "[%4d] call_extern_member %i (call external component method %i (%s))", instruction_pos, instr, instr, identifier[mmethod_call_type[instr].keyword_index].name);
    	write_line_to_log(log_line, MLOG_COL_COMPILER);
//    	if (mmethod_call_type[instr].parameters > 0)
      print_method_parameters(mmethod_call_type[instr].parameters + 2, log_line, "target ", "component "); // + 2 is for target memory index and component index
//       else
//								sprintf(log_line, "       (no parameters)");
				}
				 else
	 	 	 sprintf(log_line, "[%4d] call_extern_member %i (call external component method %i - invalid method?)", instruction_pos, instr, instr);
				write_line_to_log(log_line, MLOG_COL_COMPILER);
// note that core passed to call_extern_member_method() is the calling core, not the target core (target core is on the stack). May be the same as the target core, or may be NULL.
				vmstate.vm_register [VM_REG_A] = call_extern_member_method(vmstate.core, instr);
				sprintf(log_line, "       (return value %i -> A)", vmstate.vm_register [VM_REG_A]);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_extern_core:
		  vmstate.instructions_left --;
				get_next_instr; // instr is bounds-checked in call_object()
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				if (instr >= 0 && instr < CMETHOD_CALL_TYPES)
				{
		 	 sprintf(log_line, "[%4d] call_extern_core %i (call external process method %i (%s))", instruction_pos, instr, instr, identifier[cmethod_call_type[instr].keyword_index].name);
    	write_line_to_log(log_line, MLOG_COL_COMPILER);
//    	if (cmethod_call_type[instr].parameters > 0)
      print_method_parameters(cmethod_call_type[instr].parameters + 1, log_line, "target ", ""); // + 1 is for core
//       else
//								sprintf(log_line, "       (no parameters)");
				}
				 else
	 	 	 sprintf(log_line, "[%4d] call_extern_core %i (call external process method %i - invalid method?)", instruction_pos, instr, instr);
				write_line_to_log(log_line, MLOG_COL_COMPILER);
// note that core passed to call_extern_core_method() is the calling core, not the target core. May be the same as the target core, or may be NULL.
				vmstate.vm_register [VM_REG_A] = call_extern_core_method(vmstate.core, instr);
				sprintf(log_line, "       (return value %i -> A)", vmstate.vm_register [VM_REG_A]);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_std:
		  vmstate.instructions_left --;
				get_next_instr; // method type (is bounds-checked in call_object())
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				if (instr >= 0 && instr < SMETHOD_CALL_TYPES)
				{
		 	 sprintf(log_line, "[%4d] call_std %i (call standard method %i (%s))", instruction_pos, instr, instr, identifier[smethod_call_type[instr].keyword_index].name);
    	write_line_to_log(log_line, MLOG_COL_COMPILER);
    	if (smethod_call_type[instr].parameters > 0)
      print_method_parameters(smethod_call_type[instr].parameters, log_line, "", "");
       else
								sprintf(log_line, "       (no parameters)");
				}
				 else
	 	 	 sprintf(log_line, "[%4d] call_std %i (call standard method %i - invalid method?)", instruction_pos, instr, instr);
				write_line_to_log(log_line, MLOG_COL_COMPILER);
				vmstate.vm_register [VM_REG_A] = call_std_method(vmstate.core, instr, 0);
				sprintf(log_line, "       (return value %i -> A)", vmstate.vm_register [VM_REG_A]);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_std_var:
				{
		   vmstate.instructions_left --;
				 get_next_instr; // method type
				 int method_type = instr;
				 get_next_instr; // number of parameters
#ifdef SHOW_BCODE
    fpr(" %i %i", method_type, instr);
#endif
				if (method_type >= 0 && method_type < SMETHOD_CALL_TYPES)
				{
		 	 sprintf(log_line, "[%4d] call_std_var %i %i (call standard method %i (%s) with %i parameters)", instruction_pos, method_type, instr, method_type, identifier[smethod_call_type[method_type].keyword_index].name, instr);
    	write_line_to_log(log_line, MLOG_COL_COMPILER);
//    	if (smethod_call_type[method_type].parameters > 0)
      print_method_parameters(instr, log_line, "", "");
//       else
//								sprintf(log_line, "       (no parameters)");
				}
				 else
	 	 	 sprintf(log_line, "[%4d] call_std_var %i %i (call standard variable method %i - invalid method?)", instruction_pos, method_type, instr, method_type);
				write_line_to_log(log_line, MLOG_COL_COMPILER);
 	 	 vmstate.vm_register [VM_REG_A] = call_std_method(vmstate.core, method_type, instr);
				 if (vmstate.error_state)
 					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				}
				break;

			case OP_call_uni:
		  vmstate.instructions_left --;
				get_next_instr; // instr is bounds-checked in call_object()
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				if (instr >= 0 && instr < UMETHOD_CALL_TYPES)
				{
		 	 sprintf(log_line, "[%4d] call_uni %i (call universal method %i (%s))", instruction_pos, instr, instr, identifier[umethod_call_type[instr].keyword_index].name);
    	write_line_to_log(log_line, MLOG_COL_COMPILER);
    	if (umethod_call_type[instr].parameters > 0)
      print_method_parameters(umethod_call_type[instr].parameters, log_line, "", "");
       else
								sprintf(log_line, "       (no parameters)");
				}
				 else
	 	 	 sprintf(log_line, "[%4d] call_uni %i (call universal method %i - invalid method?)", instruction_pos, instr, instr);
				write_line_to_log(log_line, MLOG_COL_COMPILER);
				vmstate.vm_register [VM_REG_A] = call_uni_method(vmstate.core, instr);
				sprintf(log_line, "       (return value %i -> A)", vmstate.vm_register [VM_REG_A]);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_call_class:
		  vmstate.instructions_left --;
				get_next_instr;
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				if (instr >= 0 && instr < CALL_TYPES)
				{
		 	 sprintf(log_line, "[%4d] call_class %i (call object method %i (%s) on class)", instruction_pos, instr, instr, identifier[call_type[instr].keyword_index].name);
    	write_line_to_log(log_line, MLOG_COL_COMPILER);
//    	if (call_type[instr].parameters > 0)
      print_method_parameters(call_type[instr].parameters + 1, log_line, "class ", ""); // + 1 is for class
//       else
//								sprintf(log_line, "       (no parameters)");
				}
				 else
	 	 	 sprintf(log_line, "[%4d] call_class %i (call object method %i - invalid method?)", instruction_pos, instr, instr);
				write_line_to_log(log_line, MLOG_COL_COMPILER);
				vmstate.vm_register [VM_REG_A] = call_class_method(vmstate.core, instr);
				sprintf(log_line, "       (return value %i -> A)", vmstate.vm_register [VM_REG_A]);
				if (vmstate.error_state)
					goto generic_error;
// the call may have cost additional instructions. instructions_left will be checked next time through the loop.
				break;

			case OP_stop:
		 	sprintf(log_line, "[%4d] exit (stop execution until the next cycle)", vmstate.bcode_pos);
   	write_line_to_log(log_line, MLOG_COL_COMPILER);
				goto finished_execution;

			case OP_terminate:
				vmstate.core->self_destruct = 1; // core will self-destruct when this function returns
		 	sprintf(log_line, "[%4d] terminate (self-destruct)", vmstate.bcode_pos);
   	write_line_to_log(log_line, MLOG_COL_COMPILER);
				goto finished_execution;

   default:
		 	sprintf(log_line, "[%4d] invalid instruction!", vmstate.bcode_pos);
   	write_line_to_log(log_line, MLOG_COL_ERROR);
				goto invalid_instruction_error;

		}

if (log_line [0] != 0)
	write_line_to_log(log_line, MLOG_COL_COMPILER);

	return 1;

finished_execution:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
//	fpr("\n core %i instructions_left %i (used %i)", core->index, vmstate.instructions_left, INSTRUCTION_COUNT - vmstate.instructions_left);
 return 0;

bcode_bounds_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("execution out of bounds", 0, 0);
 return 0;

memory_address_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid memory access", 1, instr);
 return 0;

invalid_instruction_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid instruction", 1, instr);
 return 0;

out_of_instructions_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle;// - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("instructions exhausted", 0, 0);
 return 0;

jump_target_bounds_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid jump target", 1, vmstate.bcode->op [vmstate.bcode_pos]);
 return 0;

stack_full_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("stack overflow", 0, 0);
 return 0;

stack_below_zero_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("stack base reached", 0, 0);
 return 0;

#ifdef DEBUG_MODE
core_called_by_non_core_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("INVALID ERROR?!", 0, 0); // shouldn't happen - no non-core programs
// fpr("\nError: self core method called by non-core program at bcode %i", vmstate.bcode_pos);
 return 0;

member_called_by_non_core_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("INVALID ERROR?!", 0, 0); // shouldn't happen - no non-core programs
// fpr("\nError: self member method called by non-core program at bcode %i", vmstate.bcode_pos);
 return 0;

object_called_by_non_core_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("INVALID ERROR?!", 0, 0); // shouldn't happen - no non-core programs
// fpr("\nError: self object method called by non-core program at bcode %i", vmstate.bcode_pos);
 return 0;
#endif

invalid_derefB_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid B dereference", 1, vmstate.vm_register [VM_REG_B]);
// fpr("\nError: register B dereference is out of bounds (%i) at bcode %i", vmstate.vm_register [VM_REG_B], vmstate.bcode_pos);
 return 0;

invalid_derefA_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid A dereference", 1, vmstate.vm_register [VM_REG_A]);
// fpr("\nError: register A dereference is out of bounds (%i) at bcode %i", vmstate.vm_register [VM_REG_A], vmstate.bcode_pos);
 return 0;

return_sub_bounds_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid return address", 1, instr);
//	fpr("\nError: subroutine return value %i out of bounds", instr);
	return 0;

switch_jump_table_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	 if (w.debug_mode)
		 print_execution_error("error in switch jump table", 0, 0); // this should be caught by the compiler
		return 0;

generic_error:
	vmstate.core->instructions_used = vmstate.core->instructions_per_cycle - vmstate.instructions_left;
	return 0; // used when e.g. calling an object causes a fatal error, and an error essage has already been written.

}



static void print_method_parameters(int parameters, char* log_line_string, char* first_parameter_text, char* second_parameter_text)
{

	if (vmstate.stack_pos < parameters)
	{
		write_line_to_log("       (Error: not enough parameters on stack?)", MLOG_COL_ERROR);
		return;
	}

	char parameter_string [15];
	strcpy(log_line_string, "       parameters (");

	int i;
	int param_number = 0;

	for (i = vmstate.stack_pos - parameters; i < vmstate.stack_pos; i ++)
	{
		if (param_number == 0
			&& first_parameter_text [0] != 0)
  		strcat(log_line_string, first_parameter_text);

		if (param_number == 1
			&& second_parameter_text [0] != 0)
  		strcat(log_line_string, second_parameter_text);

		if (i < vmstate.stack_pos - 1)
		 sprintf(parameter_string, "%i, ", vmstate.vm_stack [i]);
		  else
  		 sprintf(parameter_string, "%i)", vmstate.vm_stack [i]);
		strcat(log_line_string, parameter_string);
		param_number++;
	}

//	write_line_to_log(log_line_string, MLOG_COL_COMPILER); - this is done later

}








