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

#include "v_interp.h"

struct vmstate_struct vmstate;
extern struct instruction_set_struct instruction_set [INSTRUCTIONS]; // in c_compile.c
extern struct scanlist_struct scanlist; // in g_method_std.c. scanlist.current is reset to 0 every cycle.

#define get_next_instr if (vmstate.bcode_pos >= BCODE_POS_MAX) goto bcode_bounds_error; instr = vmstate.bcode->op [++vmstate.bcode_pos];
#define get_next_instr_expect_address if (vmstate.bcode_pos >= BCODE_POS_MAX) goto bcode_bounds_error; instr = vmstate.bcode->op [++vmstate.bcode_pos]; if (instr < 0 || instr >= MEMORY_SIZE) goto memory_address_error;

static void	print_execution_error(const char* error_message, int values, int value1);

void execute_bcode(struct core_struct* core, struct bcode_struct* bc, s16b* memory)
{

	vmstate.core = core;
	vmstate.bcode = bc;
	vmstate.bcode_pos = -1; // will be incremented before use
	vmstate.instructions_left = INSTRUCTION_COUNT; // current 1024
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
		vmstate.vm_register [VM_REGISTERS] = 0;
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

			case OP_call_object:
		  vmstate.instructions_left --;
				get_next_instr; // instr is bounds-checked in call_object()
#ifdef SHOW_BCODE
    fpr(" %i", instr);
#endif
				if (core == NULL) // currently NULL is used for debugging
					goto object_called_by_non_core_error;
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
				if (core == NULL) // currently NULL is used for debugging
					goto member_called_by_non_core_error;
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
				if (core == NULL) // currently NULL is used for debugging
					goto core_called_by_non_core_error;
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
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
//	fpr("\n core %i instructions_left %i (used %i)", core->index, vmstate.instructions_left, INSTRUCTION_COUNT - vmstate.instructions_left);
 return; // success

bcode_bounds_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("execution out of bounds", 0, 0);
 return;

memory_address_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid memory access", 1, instr);
 return;

invalid_instruction_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid instruction", 1, instr);
 return;

out_of_instructions_error:
	core->instructions_used = INSTRUCTION_COUNT;// - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("instructions exhausted", 0, 0);
 return;

jump_target_bounds_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid jump target", 1, vmstate.bcode->op [vmstate.bcode_pos]);
 return;

stack_full_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("stack overflow", 0, 0);
 return;

stack_below_zero_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("stack base reached", 0, 0);
 return;

core_called_by_non_core_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("INVALID ERROR?!", 0, 0); // shouldn't happen - no non-core programs
// fpr("\nError: self core method called by non-core program at bcode %i", vmstate.bcode_pos);
 return;

member_called_by_non_core_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("INVALID ERROR?!", 0, 0); // shouldn't happen - no non-core programs
// fpr("\nError: self member method called by non-core program at bcode %i", vmstate.bcode_pos);
 return;

object_called_by_non_core_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("INVALID ERROR?!", 0, 0); // shouldn't happen - no non-core programs
// fpr("\nError: self object method called by non-core program at bcode %i", vmstate.bcode_pos);
 return;

invalid_derefB_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid B dereference", 1, vmstate.vm_register [VM_REG_B]);
// fpr("\nError: register B dereference is out of bounds (%i) at bcode %i", vmstate.vm_register [VM_REG_B], vmstate.bcode_pos);
 return;

invalid_derefA_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid A dereference", 1, vmstate.vm_register [VM_REG_A]);
// fpr("\nError: register A dereference is out of bounds (%i) at bcode %i", vmstate.vm_register [VM_REG_A], vmstate.bcode_pos);
 return;

return_sub_bounds_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	if (w.debug_mode)
		print_execution_error("invalid return address", 1, instr);
//	fpr("\nError: subroutine return value %i out of bounds", instr);
	return;

switch_jump_table_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	 if (w.debug_mode)
		 print_execution_error("error in switch jump table", 0, 0); // this should be caught by the compiler
		return;

generic_error:
	core->instructions_used = INSTRUCTION_COUNT - vmstate.instructions_left;
	return; // used when e.g. calling an object causes a fatal error, and an error essage has already been written.

}


static void	print_execution_error(const char* error_message, int values, int value1)
{

 static char ex_error_string [120];

	if (values)
	 sprintf(ex_error_string, "Error [%s:%i] at bcode %i.", error_message, value1, vmstate.bcode_pos);
	  else
	   sprintf(ex_error_string, "Error [%s] at bcode %i.", error_message, vmstate.bcode_pos);

	write_text_to_console(CONSOLE_GENERAL, PRINT_COL_LRED, vmstate.core->index, vmstate.core->created_timestamp, ex_error_string);


}


/*

How will power/stress work?

core will have:
 int power_capacity; // total capacity - determined by core type and maybe by objects?
 int power_used; // amount used this cycle. applied to stress just before execution

 int stress;

at start of cycle, power_used is applied to stress (+ or -) and reset to 0
during cycle, power_used is increased by object use.

object use generally uses multiples of 10 but some (e.g. move) may not





*/
