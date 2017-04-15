
#ifndef H_V_INTERP
#define H_V_INTERP

#define VM_STACK_SIZE 128

//#define INSTRUCTION_COUNT 1024 - this is now based on core type

enum
{
VM_REG_A,
VM_REG_B,
VM_REGISTERS
};



struct vmstate_struct
{
	struct core_struct* core;
	struct bcode_struct* bcode;
	s16b* memory;
	int bcode_pos;
	int instructions_left;
	s16b vm_stack [VM_STACK_SIZE];
	int stack_pos;
	s16b vm_register [VM_REGISTERS]; // currently just A and B
	int error_state; // is set to 1 if there's been an error in a function called by the interpreter (although probably not if the interpreter itself finds an error)

// some values used to store things use for multiple method calls within a single execution, to avoid recalculating them
 int nearby_well_index; // -2 if not yet calculated, -1 if calculated and no well within scan range
};


void execute_bcode(struct core_struct* core, struct bcode_struct* bc, s16b* memory);
void run_bcode_watch(void);
void init_bcode_execution_for_watch(struct core_struct* core, struct bcode_struct* bc, s16b* memory);
void finish_executing_bcode_in_watch(void);

#endif
