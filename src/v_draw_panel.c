#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"
#include "c_header.h"
#include "e_header.h"
#include "i_header.h"
#include "e_slider.h"
#include "e_editor.h"
#include "e_log.h"
#include "e_help.h"
#include "m_input.h"

#include "g_misc.h"
#include "g_command.h"

#include "i_input.h"
#include "i_view.h"
#include "i_display.h"
#include "i_buttons.h"
#include "t_template.h"
#include "g_method.h"
#include "g_method_core.h"
#include "g_method_std.h"
#include "g_method_uni.h"

#include "p_panels.h"
#include "d_draw.h"
#include "v_draw_panel.h"
#include "v_interp.h"

extern struct fontstruct font [FONTS];
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct instruction_set_struct instruction_set [INSTRUCTIONS];
extern struct identifierstruct identifier [IDENTIFIERS];
extern struct ex_control_struct ex_control;
extern struct slider_struct slider [SLIDERS];
extern struct game_struct game;
extern struct command_struct command;
extern struct vmstate_struct vmstate;
extern struct view_struct view;

extern struct call_type_struct call_type [CALL_TYPES];
extern struct mmethod_call_type_struct mmethod_call_type [MMETHOD_CALL_TYPES];
extern struct cmethod_call_type_struct cmethod_call_type [CMETHOD_CALL_TYPES];
extern struct smethod_call_type_struct smethod_call_type [SMETHOD_CALL_TYPES];
extern struct umethod_call_type_struct umethod_call_type [UMETHOD_CALL_TYPES];



struct template_debug_struct template_debug [PLAYERS] [TEMPLATES_PER_PLAYER];

static void check_bcp_mouseover(int x_min, int x_max, int line_y, int mouseover_type, int mouseover_value, int box_colour);
static void setup_bcode_panel(void);

/*

Colours:
src code line - blue
bcode addr - greenish
instruction - blue
number - orange
method - green?

*/
#define BCODE_PANEL_COL_BACKGROUND COL_BLUE
#define BCODE_PANEL_COL_HEADING COL_BLUE

#define BCODE_PANEL_COL_SRC COL_BLUE
#define BCODE_PANEL_COL_BCODE_ADDR COL_TURQUOISE
#define BCODE_PANEL_COL_MEMORY COL_ORANGE
#define BCODE_PANEL_COL_VARIABLE COL_ORANGE
#define BCODE_PANEL_COL_INSTRUCTION COL_BLUE
#define BCODE_PANEL_COL_NUMBER COL_YELLOW
#define BCODE_PANEL_COL_METHOD COL_GREEN
#define BCODE_PANEL_COL_STRING COL_GREY
#define BCODE_PANEL_COL_MISC COL_AQUA
#define BCODE_PANEL_COL_MESSAGE COL_AQUA
#define BCODE_PANEL_COL_TARGET COL_TURQUOISE

const char* opcode_help [] =
{

"Do nothing.", // OP_nop,
"Push the value in A to the stack and increment the stack pointer.", // OP_pushA,
"Pop a value from the stack to B and decrement the stack pointer.", // OP_popB,
"Compute A + B and leave the result in A.", // OP_add,
"Compute B - A and leave the result in A.", // OP_sub_BA, // A = B-A
"Compute A - B and leave the result in A.", // OP_sub_AB, // A = A-B
"Compute A * B and leave the result in A.", // OP_mul,
"Compute B / A and leave the result in A.", // OP_div_BA, // A = B/A
"Compute A / B and leave the result in A.", // OP_div_AB, // A = A/B
"Compute B % A and leave the result in A.", // OP_mod_BA, // A = B%A
"Compute A % B and leave the result in A.", // OP_mod_AB, // A = A%B
"Compute ~A and leave the result in A. (Note that the compiler implements this poorly - it requires two operands even though only one is used.)", // OP_not, // ~
"Bitwise and. Compute A & B and leave the result in A.", // OP_and, // &
"Bitwise or. Compute A | B and leave the result in A.", // OP_or, // |
"Bitwise xor. Compute A ^ B and leave the result in A.", // OP_xor, // ^
"Left bitshift. Compute B << A and leave the result in A.", // OP_lsh_BA, // A = B<<A
"Left bitshift. Compute A << B and leave the result in A.", // OP_lsh_AB, // A = A<<B
"Right bitshift. Compute B >> A and leave the result in A.", // OP_rsh_BA, // A = B>>A
"Right bitshift. Compute A >> B and leave the result in A.", // OP_rsh_AB, // A = A>>B
"Logical not. If A is 0, set A to 1. Otherwise, set A to 0.", // OP_lnot, // logical not !
"Set A to a specified number.", // OP_setA_num,
"Set A to the value in a memory address.", // OP_setA_mem,
"Copy A to a memory address.", // OP_copyA_to_mem,
"Push a specified number to the stack.", // OP_push_num,
"Push a value from memory to the stack.", // OP_push_mem,
"Increment a value in memory.", // OP_incr_mem,
"Decrement a value in memory.", // OP_decr_mem,
"Jump to a specified bcode address.", // OP_jump_num,
"Jump to the bcode address in A.", // OP_jumpA,
"If A is equal to B, set A to 1. Otherwise, set A to 0.", // OP_comp_eq,
"If B is greater than A, set A to 1. Otherwise, set A to 0.", // OP_comp_gr,
"If B if greater than or equal to A, set A to 1. Otherwise, set A to 0.", // OP_comp_greq,
"If B is less than A, set A to 1. Otherwise, set A to 0.", // OP_comp_ls,
"If B is less than or equal to A, set A to 1. Otherwise, set A to 0.", // OP_comp_lseq,
"If A is equal to B, set A to 0. Otherwise, set A to 1.", // OP_comp_neq,
"If A is non-zero, jump to the specified bcode address.", // OP_iftrue_jump,
"If A is zero, jump to the specified bcode address.", // OP_iffalse_jump,

"Multiply A by a specified number and leave the result in A.", // OP_mulA_num,
"Add a specified number to A and leave the result in A.", // OP_addA_num,
"Set A to the value in the memory address pointed to by A.", // OP_derefA,
"Copy A to the memory address pointed to by B.", // OP_copyA_to_derefB,
"Increment the memory address pointed to by A.", // OP_incr_derefA,
"Decrement the memory address pointed to by A.", // OP_decr_derefA,
"Copy A to B.", // OP_copyAtoB,
"Copy the contents of the memory address pointed to by the value at the front of the stack to A.", // OP_deref_stack_toA,

"Push the bcode address of the next instruction to the stack.", // OP_push_return_address,
"Jump to the bcode address at the front of the stack.", // OP_return_sub,

"Complex instruction to implement C switch command with a jump table. First operand is the bcode address of the start of the table; second is the lowest case; third is the highest case.", // OP_switchA,

"Not used.", // OP_pcomp_eq, // process comparison
"Not used.", // OP_pcomp_neq,

"Print a null-terminated string to the console. The string must follow this instruction in bcode.", // OP_print, // should be followed by null-terminated string
"Print the value in A as a number to the console.", // OP_printA, // prints contents of register A
"Print a null-terminated string to a bubble. The string must follow this instruction in bcode.", // OP_bubble, // should be followed by null-terminated string
"Print the value in A as a number to a bubble.", // OP_bubbleA, // prints contents of register A to bubble

"Call an object method. The component and object indices, then any parameters for the object call, are taken from the stack.", // OP_call_object,
"Call a member (component) method on a component of this process. The component index is taken from the stack.", // OP_call_member,
"Call a process method on this process.", // OP_call_core,
"Call a component method on another process. The value on the front of the stack is treated as an index in targetting memory for the target; the next value is the component.", // OP_call_extern_member,
"Call a process method on another process. The value on the front of the stack is treated as an index in targetting memory for the target.", // OP_call_extern_core,
"Call a standard method. Parameters (if any) are taken from the stack.", // OP_call_std,
"Call a standard method with a number of parameters specified by the second operand. The parameters (if any) are taken from the stack.", // OP_call_std_var, // smethod call with variable number of parameters
"Call a universal method. (These are basically the same as standard methods.)", // OP_call_uni,
"Call an object method on a class of objects. The operand is the class; any parameters of the call are taken from the stack.", // OP_call_class,

"Stop execution for this cycle.", // OP_stop,
"Self-destruct.", // OP_terminate,


};


enum
{
BCP_HELP_EMPTY, // when panel is empty and no button is selected
BCP_HELP_WATCHING, // when a process is in the panel but its execution isn't being watched
BCP_HELP_WATCHING_EXECUTION, // when a process is in the panel and its execution is being watched

};

static void bcode_panel_general_help_text(int help_type);



/*

Plan for bytecode viewer:

vertical subpanels:
1. bcode
[bcode][src_line] operation
       [src_line] label,label,ep1t

2. memory
[address][value] name

3. process memory
[address][target][player] template name

4. stack
[address] number (meaning??: constant, variable address, calculated, return address)

Below:
registers A B stack execution

buttons:
- start watching selected process
- stop watching process
- pause
- +step
- step through


-wait
-one step
-step through (slow, fast)
- dump to file?

subpanels: (probably each has a single main element + a scrollbar
bcode
memory
process memory
stack
control (bottom)

*/

enum
{
//BCP_BUTTON_HELP,
BCP_BUTTON_WATCH_SELECTED,
BCP_BUTTON_STOP_WATCHING,
BCP_BUTTON_WAIT,
BCP_BUTTON_ADVANCE,
BCP_BUTTON_STEP_THROUGH_SLOW,
BCP_BUTTON_STEP_THROUGH_FAST,
BCP_BUTTON_FINISH,

BCP_BUTTONS
};

char bcp_button_text [BCP_BUTTONS] [25] =
{
//	"? help", // BCP_BUTTON_HELP
	"Watch selected process", // BCP_BUTTON_WATCH_SELECTED
	"Close", // BCP_BUTTON_STOP_WATCHING
	"Watch execution", // BCP_BUTTON_WAIT
	"Advance one step", // BCP_BUTTON_ADVANCE
	"Step through - slow", // BCP_BUTTON_STEP_THROUGH_SLOW
	"Step through - fast", // BCP_BUTTON_STEP_THROUGH_FAST
	"Finish", // BCP_BUTTON_FINISH
};





/*

Buttons:

while not watching anything:
	help
	Watch selected process

while watching a process but not stepping through
 help
 Watch selected process
 Stop watching
 Wait/+step
 Step through - slow
 Step through - fast

while stepping through
 help
 Watch selected process
 Stop watching
 Wait/+step
 Step through -slow
 Step through -fast
 Finish

*/

struct bcode_panel_state_struct bcp_state;

#define BCODE_LINE_HEADER_SIZE (font[FONT_BASIC].height * 2 + 6)
#define BCODE_LINE_HEIGHT (font[FONT_BASIC].height + 1)

//void bcode_panel_input(void);
static void bcode_subpanel_mousewheel(int subpanel_index, int* line_ptr, int maximum_value);


void init_bcode_panel(void)
{

 bcp_state.lines = (panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].h - BCODE_LINE_HEADER_SIZE) / BCODE_LINE_HEIGHT;

 reset_bcode_panel();

 setup_bcode_panel();

}

// should probably reset at the start and end of every game
//  - otherwise watch_core_index may refer to an invalid core
void reset_bcode_panel(void)
{

 bcp_state.bcp_mode = BCP_MODE_EMPTY;
 bcp_state.bcp_wait_mode = BCP_WAIT_OFF;
 bcp_state.player_index = 0;
 bcp_state.template_index = 0;
 bcp_state.watch_core_index = 0;
 bcp_state.watch_core_timestamp = 0;

	bcp_state.player_index = 0;
	bcp_state.template_index = 0;

	bcp_state.subpanel_bcode_total_lines = 0; // this will be reset later
	bcp_state.subpanel_bcode_line = 0;
	bcp_state.subpanel_variable_line = 0;
	bcp_state.subpanel_stack_line = 0;
	bcp_state.subpanel_target_line = 0;

 slider_moved_to_value(&slider[SLIDER_BCODE_BCODE_SCROLLBAR_V], bcp_state.subpanel_bcode_line);
 slider_moved_to_value(&slider[SLIDER_BCODE_MEMORY_SCROLLBAR_V], bcp_state.subpanel_variable_line);
 slider_moved_to_value(&slider[SLIDER_BCODE_STACK_SCROLLBAR_V], bcp_state.subpanel_stack_line);
 slider_moved_to_value(&slider[SLIDER_BCODE_TARGET_SCROLLBAR_V], bcp_state.subpanel_target_line);

//	bcp_state.mouse_x = control.mouse_x_screen_pixels - panel[PANEL_BCODE].x1;
//	bcp_state.mouse_y = control.mouse_y_screen_pixels;// - panel[PANEL_BCODE].y1;

	bcp_state.mouseover_time = 0;
	bcp_state.mouseover_type = BCP_MOUSEOVER_NONE;
	bcp_state.mouseover_value = 0;

}


void draw_bcode_panel(void)
{

// bcode_panel_input();

//	al_set_clipping_rectangle(panel[PANEL_BCODE].x1, panel[PANEL_BCODE].y1, panel[PANEL_BCODE].w, panel[PANEL_BCODE].h);
//	int pan = PANEL_BCODE;
//	int el;
	int line_y;
	int base_text_x;
	int text_x;
//	int j;
	int line_index;
	struct template_debug_struct* tdb = &template_debug[bcp_state.player_index][bcp_state.template_index];
	struct template_struct* sts = &templ[bcp_state.player_index][bcp_state.template_index];

 int mouse_in_subpanel = -1;
	int mouse_on_line = -1;
	int mouse_x = control.mouse_x_screen_pixels - panel[PANEL_BCODE].x1;
	int mouse_y = control.mouse_y_screen_pixels;// - panel[PANEL_BCODE].y1;

	int clip_right_x;
	int clip_width;


 al_clear_to_color(panel[PANEL_BCODE].background_colour);
 int i;
 char operand_string [80];

 al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_GREY] [SHADE_MAX], panel[PANEL_BCODE].x1 + 2, panel[PANEL_BCODE].y1 + 2, ALLEGRO_ALIGN_LEFT, "Bytecode");

	if (panel[PANEL_BCODE].element[FPE_BCODE_PANEL_RESIZE].last_highlight == inter.running_time)
	{
  al_draw_filled_rectangle(panel[PANEL_BCODE].x1, 0, panel[PANEL_BCODE].x1 + 5, settings.option [OPTION_WINDOW_H], colours.base [COL_BLUE] [SHADE_MED]);
	}
	 else
   al_draw_filled_rectangle(panel[PANEL_BCODE].x1, 0, panel[PANEL_BCODE].x1 + 2, settings.option [OPTION_WINDOW_H], colours.base [COL_BLUE] [SHADE_LOW]);



if (bcp_state.bcp_mode == BCP_MODE_EMPTY)
	goto finished_drawing;

	if (game.phase == GAME_PHASE_WORLD
		&& control.mouse_panel == PANEL_BCODE)
	{

	  if (mouse_x >= panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x1
		  && mouse_x <= panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x2
		  && mouse_y >= panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].y1
		  && mouse_y <= panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].y2)
		 {
		  mouse_in_subpanel = FSP_BCODE_BCODE;
  		if (ex_control.mousewheel_change != 0)
  		{
			  bcode_subpanel_mousewheel(FSP_BCODE_BCODE, &bcp_state.subpanel_bcode_line, bcp_state.subpanel_bcode_total_lines);
     slider_moved_to_value(&slider[SLIDER_BCODE_BCODE_SCROLLBAR_V], bcp_state.subpanel_bcode_line);
		  }
		 }
		   else
			{
	  if (mouse_x >= panel[PANEL_BCODE].subpanel[FSP_BCODE_MEMORY].x1
		  && mouse_x <= panel[PANEL_BCODE].subpanel[FSP_BCODE_MEMORY].x2
		  && mouse_y >= panel[PANEL_BCODE].subpanel[FSP_BCODE_MEMORY].y1
		  && mouse_y <= panel[PANEL_BCODE].subpanel[FSP_BCODE_MEMORY].y2)
		 {
		  mouse_in_subpanel = FSP_BCODE_MEMORY;
  		if (ex_control.mousewheel_change != 0)
  		{
			  bcode_subpanel_mousewheel(FSP_BCODE_MEMORY, &bcp_state.subpanel_variable_line, MEMORY_SIZE - 1);
     slider_moved_to_value(&slider[SLIDER_BCODE_MEMORY_SCROLLBAR_V], bcp_state.subpanel_variable_line);
		  }
		 }
		   else
					{
	  if (mouse_x >= panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].x1
		  && mouse_x <= panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].x2
		  && mouse_y >= panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].y1
		  && mouse_y <= panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].y2)
		 {
		  mouse_in_subpanel = FSP_BCODE_STACK;
  		if (ex_control.mousewheel_change != 0)
  		{
			  bcode_subpanel_mousewheel(FSP_BCODE_STACK, &bcp_state.subpanel_stack_line, VM_STACK_SIZE - 1);
     slider_moved_to_value(&slider[SLIDER_BCODE_STACK_SCROLLBAR_V], bcp_state.subpanel_stack_line);
		  }
		 }
		   else
					{
	  if (mouse_x >= panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].x1
		  && mouse_x <= panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].x2
		  && mouse_y >= panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].y1
		  && mouse_y <= panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].y2)
		 {
		  mouse_in_subpanel = FSP_BCODE_TARGET_MEMORY;
  		if (ex_control.mousewheel_change != 0)
  		{
			  bcode_subpanel_mousewheel(FSP_BCODE_TARGET_MEMORY, &bcp_state.subpanel_target_line, PROCESS_MEMORY_SIZE - 1);
     slider_moved_to_value(&slider[SLIDER_BCODE_TARGET_SCROLLBAR_V], bcp_state.subpanel_target_line);
		  }
		 }
		  else
				{
	  if (mouse_x >= panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].x1
		  && mouse_x <= panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].x2
		  && mouse_y >= panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].y1
		  && mouse_y <= panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].y2)
		 {
		  mouse_in_subpanel = FSP_BCODE_MESSAGES;
		 }


				}


					}


					}


			}

	}


// bcode subpanel:

	//al_set_clipping_rectangle(0, 0, 1600, 900);//panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].w, panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].h);

	clip_width = panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].w;

	clip_right_x = panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].w;

	if (clip_right_x > panel[PANEL_BCODE].x2)
	{
		clip_right_x = panel[PANEL_BCODE].x2;
		clip_width = clip_right_x - (panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x1);
		if (clip_width <= 0)
			goto finished_drawing;
	}

	al_set_clipping_rectangle(panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x1, panel[PANEL_BCODE].y1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].y1, clip_width, panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].h);
// al_draw_rectangle(panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x1, panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].y1, panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].w, panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].y1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].h, colours.base [COL_GREY] [SHADE_MAX], 1);
 al_clear_to_color(colours.base [BCODE_PANEL_COL_BACKGROUND] [SHADE_LOW]);



 line_y = panel[PANEL_BCODE].y1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].y1;
 line_index	= bcp_state.subpanel_bcode_line;
 base_text_x = panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x1 + 3;

 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_HEADING] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 1, line_y + 2, ALLEGRO_ALIGN_LEFT, "Bytecode instructions");

 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_SRC] [SHADE_HIGH], base_text_x + font[FONT_BASIC].width * 4, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_RIGHT, "src");
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_BCODE_ADDR] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 9, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_RIGHT, "addr");
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_INSTRUCTION] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 10 + 3, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_LEFT, "instruction");

 line_y += BCODE_LINE_HEADER_SIZE;


	if (mouse_in_subpanel == FSP_BCODE_BCODE)
	{
		mouse_on_line = mouse_y - line_y;
		mouse_on_line /= BCODE_LINE_HEIGHT;
		if (mouse_on_line < 0
			|| mouse_on_line >= bcp_state.lines)
				mouse_on_line = 1;
	}
	 else
			mouse_on_line = -1;

	for (i = 0; i < bcp_state.lines; i ++)
	{

		line_index = bcp_state.subpanel_bcode_line + i;
		if (line_index >= DEBUGGER_LINES - 1)
			break;

		if (tdb->debugger_line[line_index].bcode_address == -1)
			break;

		text_x = base_text_x;


		if (game.watching == WATCH_PAUSED_TO_WATCH
			&& vmstate.bcode_pos + 1 == tdb->debugger_line[line_index].bcode_address)
		{
/*			fpr("\n %i,%i %i,%f", panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x1,
																																	line_y - 2,
																																	panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x2,
																																	line_y + BCODE_LINE_HEIGHT - scaleUI_y(FONT_BASIC, 2));*/
        al_draw_filled_rectangle(panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x1,
																																	line_y - 2,
																																	panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x2,
																																	line_y + BCODE_LINE_HEIGHT - scaleUI_y(FONT_BASIC, 2),
																																	colours.base_trans [COL_CYAN] [SHADE_MAX] [TRANS_FAINT]);
		}


/*
  			  if (mouse_on_line == i)
								check_bcp_mouseover(text_x + font[FONT_BASIC].width * 5 - 2,
																												text_x + font[FONT_BASIC].width * 9 + 2,
																												line_y,
																												BCP_MOUSEOVER_BCODE_ADDRESS,
																												tdb->debugger_line[line_index].bcode_address,
																												COL_AQUA);*/

	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_BCODE_ADDR] [SHADE_MAX], text_x + font[FONT_BASIC].width * 9, line_y, ALLEGRO_ALIGN_RIGHT, "%i", tdb->debugger_line[line_index].bcode_address);

	 if (tdb->debugger_line[line_index].source_line >= 0)
		{
  			  if (mouse_on_line == i)
								check_bcp_mouseover(text_x,
																												text_x + font[FONT_BASIC].width * 4 + 2,
																												line_y,
																												BCP_MOUSEOVER_SOURCE_LINE,
																												tdb->debugger_line[line_index].source_line,
																												COL_AQUA);

 	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_SRC] [SHADE_HIGH], text_x + font[FONT_BASIC].width * 4, line_y, ALLEGRO_ALIGN_RIGHT, "%i", tdb->debugger_line[line_index].source_line + 1);
		}
 	  else
				{
					switch(tdb->debugger_line[line_index].source_line)
					{
						case SOURCE_LINE_JUMP_TABLE:
/*  			  if (mouse_on_line == i)
								check_bcp_mouseover(text_x + font[FONT_BASIC].width * 10 - 2,
																												text_x + font[FONT_BASIC].width * 14 + 5,
																												line_y,
																												BCP_MOUSEOVER_BCODE_ADDRESS,
																												sts->bcode.op[tdb->debugger_line[line_index].bcode_address],
																												COL_ORANGE);*/
  			  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_YELLOW] [SHADE_MED], text_x + font[FONT_BASIC].width * 14 + 3, line_y, ALLEGRO_ALIGN_RIGHT, "%i", sts->bcode.op[tdb->debugger_line[line_index].bcode_address]);
  			  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_YELLOW] [SHADE_MED], text_x + font[FONT_BASIC].width * 14 + 6, line_y, ALLEGRO_ALIGN_LEFT, "jump table case");
  			  break;
						case SOURCE_LINE_JUMP_TABLE_DEFAULT:
/*  			  if (mouse_on_line == i)
								check_bcp_mouseover(text_x + font[FONT_BASIC].width * 10 - 2,
																												text_x + font[FONT_BASIC].width * 14 + 5,
																												line_y,
																												BCP_MOUSEOVER_BCODE_ADDRESS,
																												sts->bcode.op[tdb->debugger_line[line_index].bcode_address],
																												COL_ORANGE);*/
  			  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_YELLOW] [SHADE_MED], text_x + font[FONT_BASIC].width * 14 + 3, line_y, ALLEGRO_ALIGN_RIGHT, "%i", sts->bcode.op[tdb->debugger_line[line_index].bcode_address]);
  			  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_YELLOW] [SHADE_MED], text_x + font[FONT_BASIC].width * 14 + 6, line_y, ALLEGRO_ALIGN_LEFT, "jump table default");
//  			  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_YELLOW] [SHADE_MED], text_x += font[FONT_BASIC].width * 10 + 3, line_y, ALLEGRO_ALIGN_LEFT, " %i jump table default", sts->bcode.op[tdb->debugger_line[line_index].bcode_address]);
  			  break;
  			 case SOURCE_LINE_STRING:
						{
					  char print_str [STRING_MAX_LENGTH + 1];
					  int str_pos = 0;
					  while(TRUE)
					  {
						  print_str [str_pos] = sts->bcode.op[tdb->debugger_line[line_index].bcode_address + str_pos];
						  if (print_str [str_pos] == 0)
									break;
								if (print_str [str_pos] == '\n')
										print_str [str_pos] = 127; // down arrow
						  str_pos ++;
						  if (str_pos >= STRING_MAX_LENGTH - 1
									|| tdb->debugger_line[line_index].bcode_address + str_pos >= BCODE_POS_MAX)
								{
									print_str [str_pos] = 0;
									break;
								}
					  }
//  			  text_x += 10 * //(strlen(instruction_set[opcode].name) + 1) * font[FONT_BASIC].width;
       text_x += font[FONT_BASIC].width * 10 + 3;
   			 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_STRING] [SHADE_HIGH], text_x, line_y, ALLEGRO_ALIGN_LEFT, "\"%s\"", print_str);
				  }
						break;
					case SOURCE_LINE_LABEL:
/*
  			  if (mouse_on_line == i)
								check_bcp_mouseover(text_x + font[FONT_BASIC].width * 14 + 1,
																												text_x + font[FONT_BASIC].width * (14 + strlen(tdb->label[tdb->debugger_line[line_index].special_value].name)) + 3,
																												line_y,
																												BCP_MOUSEOVER_LABEL,
																												tdb->debugger_line[line_index].special_value,
																												BCODE_PANEL_COL_BCODE_ADDR);*/
 			  al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_BCODE_ADDR] [SHADE_MAX], text_x + font[FONT_BASIC].width * 14 + 3, line_y, ALLEGRO_ALIGN_LEFT, "%s:", tdb->label[tdb->debugger_line[line_index].special_value].name);
 			  break;
					}
					goto finished_drawing_line;
				}


		int opcode = sts->bcode.op[tdb->debugger_line[line_index].bcode_address];

  text_x += font[FONT_BASIC].width * 10 + 3;

		if (opcode >= 0
			&& opcode < INSTRUCTIONS)
		{
  			  if (mouse_on_line == i)
								check_bcp_mouseover(text_x - 2,
																												text_x + font[FONT_BASIC].width * (strlen(instruction_set[opcode].name)) + 3,
																												line_y,
																												BCP_MOUSEOVER_OPCODE,
																												opcode,
																												BCODE_PANEL_COL_INSTRUCTION);

			 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_INSTRUCTION] [SHADE_MAX], text_x, line_y, ALLEGRO_ALIGN_LEFT, "%s", instruction_set[opcode].name);
			 int operand_colour = COL_ORANGE;
			 if (instruction_set[opcode].operands > 0)
				{
			  text_x += (strlen(instruction_set[opcode].name) + 1) * font[FONT_BASIC].width;
			  int first_operand = sts->bcode.op[tdb->debugger_line[line_index].bcode_address + 1];

			  switch(instruction_set[opcode].operand_type[0])
			  {
			  	case OPERAND_TYPE_MEMORY:
  			  if (first_operand >= 0
						  && first_operand < MEMORY_SIZE)
 				   sprintf(operand_string, "%i(%s)", first_operand, tdb->variable[first_operand].name);
 				    else
   				   sprintf(operand_string, "%i", first_operand);
   				operand_colour = BCODE_PANEL_COL_MEMORY;
/*
  			  if (mouse_on_line == i)
								check_bcp_mouseover(text_x - 2,
																												text_x + font[FONT_BASIC].width * strlen(operand_string) + 3,
																												line_y,
																												BCP_MOUSEOVER_VARIABLE,
																												opcode,
																												BCODE_PANEL_COL_MEMORY);*/

   				break;
   			case OPERAND_TYPE_BCODE_ADDRESS:
				   sprintf(operand_string, "%i", first_operand);
   				operand_colour = BCODE_PANEL_COL_BCODE_ADDR;
/*
  			  if (mouse_on_line == i)
								check_bcp_mouseover(text_x - 2,
																												text_x + font[FONT_BASIC].width * strlen(operand_string) + 3,
																												line_y,
																												BCP_MOUSEOVER_BCODE_ADDRESS,
																												opcode,
																												BCODE_PANEL_COL_BCODE_ADDR);*/

							break;
						default:
						case OPERAND_TYPE_NUMBER:
							switch(opcode)
							{
							 case OP_call_class:
								case OP_call_object:
     				operand_colour = BCODE_PANEL_COL_METHOD;
									if (first_operand >= 0
										&& first_operand < CALL_TYPES)
									{
   				   sprintf(operand_string, "%i(%s)", first_operand, identifier[call_type[first_operand].keyword_index].name);
									}
									 else
    				   sprintf(operand_string, "%i", first_operand);
									break;
								case OP_call_core:
								case OP_call_extern_core:
     				operand_colour = BCODE_PANEL_COL_METHOD;
									if (first_operand >= 0
										&& first_operand < CMETHOD_CALL_TYPES)
									{
   				   sprintf(operand_string, "%i(%s)", first_operand, identifier[cmethod_call_type[first_operand].keyword_index].name);
									}
									 else
    				   sprintf(operand_string, "%i", first_operand);
									break;
								case OP_call_member:
								case OP_call_extern_member:
     				operand_colour = BCODE_PANEL_COL_METHOD;
									if (first_operand >= 0
										&& first_operand < MMETHOD_CALL_TYPES)
									{
   				   sprintf(operand_string, "%i(%s)", first_operand, identifier[mmethod_call_type[first_operand].keyword_index].name);
									}
									 else
    				   sprintf(operand_string, "%i", first_operand);
									break;
								case OP_call_std:
								case OP_call_std_var:
     				operand_colour = BCODE_PANEL_COL_METHOD;
									if (first_operand >= 0
										&& first_operand < SMETHOD_CALL_TYPES)
									{
   				   sprintf(operand_string, "%i(%s)", first_operand, identifier[smethod_call_type[first_operand].keyword_index].name);
									}
									 else
    				   sprintf(operand_string, "%i", first_operand);
									break;
								case OP_call_uni:
     				operand_colour = BCODE_PANEL_COL_METHOD;
									if (first_operand >= 0
										&& first_operand < UMETHOD_CALL_TYPES)
									{
   				   sprintf(operand_string, "%i(%s)", first_operand, identifier[umethod_call_type[first_operand].keyword_index].name);
									}
									 else
    				   sprintf(operand_string, "%i", first_operand);
									break;



								default:
  				   sprintf(operand_string, "%i", first_operand);
     				operand_colour = BCODE_PANEL_COL_NUMBER;
  				   break;
							}
				   break;
			  }
/*
			  if (sts->bcode.op[tdb->debugger_line[line_index].bcode_address + 1] >= 0
						&& sts->bcode.op[tdb->debugger_line[line_index].bcode_address + 1] < MEMORY_SIZE)
 				 sprintf(operand_string, "%s(%i)", tdb->variable[sts->bcode.op[tdb->debugger_line[line_index].bcode_address + 1]].name, sts->bcode.op[tdb->debugger_line[line_index].bcode_address + 1]);
 				  else
   				 sprintf(operand_string, "%i", sts->bcode.op[tdb->debugger_line[line_index].bcode_address + 1]);
*/
 			 al_draw_textf(font[FONT_BASIC].fnt, colours.base [operand_colour] [SHADE_MAX], text_x, line_y, ALLEGRO_ALIGN_LEFT, "%s", operand_string);

// current opcodes only have OPERAND_TYPE_NUMBER in operands 1 and 2
			 if (instruction_set[opcode].operands > 1)
 			{
			  text_x += (strlen(operand_string) + 1) * font[FONT_BASIC].width;
//			  text_x += 7*font[FONT_BASIC].width;//((sts->bcode.op[tdb->debugger_line[line_index].bcode_address + 1] / 10) + 1) * font[FONT_BASIC].width;
				 sprintf(operand_string, "%i", sts->bcode.op[tdb->debugger_line[line_index].bcode_address + 2]);
 			 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_NUMBER] [SHADE_HIGH], text_x, line_y, ALLEGRO_ALIGN_LEFT, operand_string);
			 if (instruction_set[opcode].operands > 2)
 			{
// 				sprintf(number_string, "%i", sts->bcode.op[tdb->debugger_line[line_index].bcode_address + 2]);
			  text_x += (strlen(operand_string) + 1) * font[FONT_BASIC].width;
		//	  text_x += ((sts->bcode.op[tdb->debugger_line[line_index].bcode_address + 2] / 10) + 1) * font[FONT_BASIC].width;
 			 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_NUMBER] [SHADE_HIGH], text_x, line_y, ALLEGRO_ALIGN_LEFT, "%i", sts->bcode.op[tdb->debugger_line[line_index].bcode_address + 3]);
 			}
				}
				}
/*
				 else
					{
			   if (opcode == OP_print || opcode == OP_bubble)
				  {
					  char print_str [STRING_MAX_LENGTH + 1];
					  int str_pos = 0;
					  while(TRUE)
					  {
						  print_str [str_pos] = sts->bcode.op[tdb->debugger_line[line_index].bcode_address + str_pos + 1];
						  if (print_str [str_pos] == 0)
									break;
								if (print_str [str_pos] == '\n')
										print_str [str_pos] = 127; // down arrow
						  str_pos ++;
						  if (str_pos >= STRING_MAX_LENGTH - 1
									|| tdb->debugger_line[line_index].bcode_address + str_pos + 1 >= BCODE_POS_MAX)
								{
									print_str [str_pos] = 0;
									break;
								}
					  }
  			  text_x += (strlen(instruction_set[opcode].name) + 1) * font[FONT_BASIC].width;
   			 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], text_x, line_y, ALLEGRO_ALIGN_LEFT, "\"%s\"", print_str);
				  }
					}
*/
		}
		 else
			{
			 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_ORANGE] [SHADE_MED], text_x, line_y, ALLEGRO_ALIGN_LEFT, "%i", sts->bcode.op[tdb->debugger_line[line_index].bcode_address]);
			}

finished_drawing_line:
		line_index ++;
		line_y += BCODE_LINE_HEIGHT;

	}
//al_set_clipping_rectangle(0,0,1600,900);
	draw_scrollbar(SLIDER_BCODE_BCODE_SCROLLBAR_V);



// memory subpanel:


	clip_width = panel[PANEL_BCODE].subpanel[FSP_BCODE_MEMORY].w;

	clip_right_x = panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MEMORY].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MEMORY].w;

	if (clip_right_x > panel[PANEL_BCODE].x2)
	{
		clip_right_x = panel[PANEL_BCODE].x2;
		clip_width = clip_right_x - (panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MEMORY].x1);
		if (clip_width <= 0)
			goto finished_drawing;
	}

	al_set_clipping_rectangle(panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MEMORY].x1, panel[PANEL_BCODE].y1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MEMORY].y1, clip_width, panel[PANEL_BCODE].subpanel[FSP_BCODE_MEMORY].h);
 al_clear_to_color(colours.base [BCODE_PANEL_COL_BACKGROUND] [SHADE_LOW]);



 line_y = panel[PANEL_BCODE].y1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MEMORY].y1;
 line_index	= bcp_state.subpanel_variable_line;
 base_text_x = panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MEMORY].x1 + 3;
 text_x = base_text_x;

 int variable_name_x = base_text_x + font[FONT_BASIC].width * 5;

 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_HEADING] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 1, line_y + 2, ALLEGRO_ALIGN_LEFT, "Variable memory");

 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_MEMORY] [SHADE_HIGH], base_text_x + font[FONT_BASIC].width * 4, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_RIGHT, "addr");
 if (bcp_state.bcp_mode == BCP_MODE_PROCESS)
	{
  al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_MEMORY] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 11, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_RIGHT, "val");
  variable_name_x += font[FONT_BASIC].width * 7;
	}
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_VARIABLE] [SHADE_MAX], variable_name_x, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_LEFT, "name");

 line_y += BCODE_LINE_HEADER_SIZE;

	if (mouse_in_subpanel == FSP_BCODE_MEMORY)
	{
		mouse_on_line = mouse_y - line_y;
		mouse_on_line /= BCODE_LINE_HEIGHT;
		if (mouse_on_line < 0
			|| mouse_on_line >= bcp_state.lines)
				mouse_on_line = 1;
	}
	 else
			mouse_on_line = -1;

	for (i = 0; i < bcp_state.lines; i ++)
	{

		line_index = bcp_state.subpanel_variable_line + i;

		int memory_address = line_index;
		if (memory_address < 0
			|| memory_address >= MEMORY_SIZE)
				break;





	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_MEMORY] [SHADE_MAX], text_x + font[FONT_BASIC].width * 4, line_y, ALLEGRO_ALIGN_RIGHT, "%i", memory_address);

  if (bcp_state.bcp_mode == BCP_MODE_PROCESS)
		{
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_VARIABLE] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 11, line_y, ALLEGRO_ALIGN_RIGHT, "%i", w.core[bcp_state.watch_core_index].memory[memory_address]);
		}

	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_VARIABLE] [SHADE_HIGH], variable_name_x, line_y, ALLEGRO_ALIGN_LEFT, "%s", tdb->variable[memory_address].name);

		line_y += BCODE_LINE_HEIGHT;

	}

	draw_scrollbar(SLIDER_BCODE_MEMORY_SCROLLBAR_V);





// stack subpanel:


	clip_width = panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].w;

	clip_right_x = panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].w;

	if (clip_right_x > panel[PANEL_BCODE].x2)
	{
		clip_right_x = panel[PANEL_BCODE].x2;
		clip_width = clip_right_x - (panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].x1);
		if (clip_width <= 0)
			goto finished_drawing;
	}


	al_set_clipping_rectangle(panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].x1,
																											panel[PANEL_BCODE].y1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].y1,
																											clip_width,
																											panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].h);
 al_clear_to_color(colours.base [BCODE_PANEL_COL_BACKGROUND] [SHADE_LOW]);



 line_y = panel[PANEL_BCODE].y1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].y1;
 line_index	= bcp_state.subpanel_stack_line;
 base_text_x = panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].x1 + 3;
 text_x = base_text_x;

 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_HEADING] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 1, line_y + 2, ALLEGRO_ALIGN_LEFT, "Stack");

 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_BCODE_ADDR] [SHADE_HIGH], base_text_x + font[FONT_BASIC].width * 4, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_RIGHT, "pos");


// Stack is only valid while the process is executing:
 if (game.watching == WATCH_PAUSED_TO_WATCH
		&&	bcp_state.bcp_mode == BCP_MODE_PROCESS)
	{
  al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_MISC] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 11, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_RIGHT, "val");
	}

 line_y += BCODE_LINE_HEADER_SIZE;

	if (mouse_in_subpanel == FSP_BCODE_STACK)
	{
		mouse_on_line = mouse_y - line_y;
		mouse_on_line /= BCODE_LINE_HEIGHT;
		if (mouse_on_line < 0
			|| mouse_on_line >= bcp_state.lines)
				mouse_on_line = 1;
	}
	 else
			mouse_on_line = -1;

	for (i = 0; i < bcp_state.lines; i ++)
	{

		line_index = bcp_state.subpanel_stack_line + i;

		int stack_address = line_index;
		if (stack_address < 0
			|| stack_address >= VM_STACK_SIZE)
				break;

		if (game.watching == WATCH_PAUSED_TO_WATCH
			&& stack_address + 1 == vmstate.stack_pos)
		{
/*			fpr("\n %i,%i %i,%f", panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x1,
																																	line_y - 2,
																																	panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x2,
																																	line_y + BCODE_LINE_HEIGHT - scaleUI_y(FONT_BASIC, 2));*/
        al_draw_filled_rectangle(panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].x1,
																																	line_y - 2,
																																	panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].x2,
																																	line_y + BCODE_LINE_HEIGHT - scaleUI_y(FONT_BASIC, 2),
																																	colours.base_trans [COL_CYAN] [SHADE_LOW] [TRANS_MED]);
		}

	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_BCODE_ADDR] [SHADE_MAX], text_x + font[FONT_BASIC].width * 4, line_y, ALLEGRO_ALIGN_RIGHT, "%i", stack_address);

  if (bcp_state.bcp_mode == BCP_MODE_PROCESS
		 && game.watching	== WATCH_PAUSED_TO_WATCH)
		{
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_MISC] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 11, line_y, ALLEGRO_ALIGN_RIGHT, "%i", vmstate.vm_stack [stack_address]);
		}

//	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], variable_name_x, line_y, ALLEGRO_ALIGN_LEFT, "%s", tdb->variable[memory_address].name);

		line_y += BCODE_LINE_HEIGHT;

	}

	draw_scrollbar(SLIDER_BCODE_STACK_SCROLLBAR_V);





// Messages...


	clip_width = panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].w;

	clip_right_x = panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].w;

	if (clip_right_x > panel[PANEL_BCODE].x2)
	{
		clip_right_x = panel[PANEL_BCODE].x2;
		clip_width = clip_right_x - (panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].x1);
		if (clip_width <= 0)
			goto finished_drawing;
	}

	al_set_clipping_rectangle(panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].x1,
																											panel[PANEL_BCODE].y1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].y1,
																											clip_width,
																											panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].h);
 al_clear_to_color(colours.base [BCODE_PANEL_COL_BACKGROUND] [SHADE_LOW]);



 line_y = panel[PANEL_BCODE].y1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].y1;
 line_index	= bcp_state.subpanel_stack_line;
 base_text_x = panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].x1 + 3;
 text_x = base_text_x;

 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_HEADING] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 1, line_y + 2, ALLEGRO_ALIGN_LEFT, "Messages");



 if (bcp_state.bcp_mode == BCP_MODE_PROCESS)
	{
/*
  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_AQUA] [SHADE_HIGH], base_text_x + font[FONT_BASIC].width * 4, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_RIGHT, "msg");
  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 7, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_RIGHT, "ch");
  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 11, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_RIGHT, "src");
  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 15, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_RIGHT, "tar");*/
//  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 16, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_LEFT, "message contents");

  line_y += BCODE_LINE_HEADER_SIZE - BCODE_LINE_HEIGHT - 3;

	 if (mouse_in_subpanel == FSP_BCODE_MESSAGES)
	 {
 		mouse_on_line = mouse_y - line_y;
		 mouse_on_line /= (BCODE_LINE_HEIGHT * 3);
		 if (mouse_on_line < 0
 			|| mouse_on_line >= MESSAGES)
				 mouse_on_line = -1;
	 }
	 else
			mouse_on_line = -1;

	 for (i = 0; i < w.core[bcp_state.watch_core_index].messages_received; i ++)
//	 for (i = 0; i < MESSAGES; i ++)
	 {
	 	int button_shade = SHADE_LOW;
	 	if (game.watching == WATCH_PAUSED_TO_WATCH
		 	&&	w.core[bcp_state.watch_core_index].message_reading == i)
				button_shade = SHADE_MED;
	 	add_menu_button(panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].x1 + 2,
																			line_y + (i * BCODE_LINE_HEIGHT * 3) - 1,
																			panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_MESSAGES].x2 - 2,
																			line_y + ((i+1) * BCODE_LINE_HEIGHT * 3) - 3,
																			colours.base [BCODE_PANEL_COL_MESSAGE] [button_shade],
																			2,3);
	 }

  draw_menu_buttons();

	 for (i = 0; i < w.core[bcp_state.watch_core_index].messages_received; i ++)
//	 for (i = 0; i < MESSAGES; i ++)
	 {

	 	line_index = i;//bcp_state.subpanel_stack_line + i;

/*
 		if (game.watching == WATCH_PAUSED_TO_WATCH
			 && line_index + 1 == w.core[bcp_state.watch_core_index].message_reading)
		 {
         al_draw_filled_rectangle(panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].x1,
																																	 line_y - 2,
																																	 panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_STACK].x2,
																																	 line_y + BCODE_LINE_HEIGHT - scaleUI_y(FONT_BASIC, 2),
																																	 colours.base_trans [COL_CYAN] [SHADE_LOW] [TRANS_MED]);
		 }
*/

  			  if (mouse_on_line == i
								&& mouse_y > line_y
								&& mouse_y < line_y + BCODE_LINE_HEIGHT)
							{
								check_bcp_mouseover(text_x + font[FONT_BASIC].width * 14 - 2,
																												base_text_x + font[FONT_BASIC].width * 22 + 1,
																												line_y,
																												BCP_MOUSEOVER_CORE_INDEX,
																												w.core[bcp_state.watch_core_index].message[i].source_index,
																												BCODE_PANEL_COL_MESSAGE);
								check_bcp_mouseover(text_x + font[FONT_BASIC].width * 23 - 2,
																												base_text_x + font[FONT_BASIC].width * 33 + 1,
																												line_y,
																												BCP_MOUSEOVER_CORE_INDEX,
																												w.core[bcp_state.watch_core_index].message[i].target_core_index,
																												BCODE_PANEL_COL_MESSAGE);
							}

							if (w.core[bcp_state.watch_core_index].message[i].target_core_index == -1)
							{

	  al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_MESSAGE] [SHADE_MAX], text_x + font[FONT_BASIC].width, line_y, ALLEGRO_ALIGN_LEFT,
																		"msg %i chan %i src %3d",
																		line_index,
																		w.core[bcp_state.watch_core_index].message[i].channel,
																		w.core[bcp_state.watch_core_index].message[i].source_index);
							}
							 else
	  al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_MESSAGE] [SHADE_MAX], text_x + font[FONT_BASIC].width, line_y, ALLEGRO_ALIGN_LEFT,
																		"msg %i chan %i src %3d  target %3d",
																		line_index,
																		w.core[bcp_state.watch_core_index].message[i].channel,
																		w.core[bcp_state.watch_core_index].message[i].source_index,
																		w.core[bcp_state.watch_core_index].message[i].target_core_index);



/*
	  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], text_x + font[FONT_BASIC].width * 4, line_y, ALLEGRO_ALIGN_RIGHT, "%i", line_index);

   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 7, line_y, ALLEGRO_ALIGN_RIGHT, "%i", w.core[bcp_state.watch_core_index].message[i].channel);
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 11, line_y, ALLEGRO_ALIGN_RIGHT, "%i", w.core[bcp_state.watch_core_index].message[i].source_index);
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 15, line_y, ALLEGRO_ALIGN_RIGHT, "%i", w.core[bcp_state.watch_core_index].message[i].target_core_index);
*/

		 line_y += BCODE_LINE_HEIGHT;

//   for (j = 0; j < w.core[bcp_state.watch_core_index].message[i].length; j ++)
//   for (j = 0; j < MESSAGE_LENGTH; j ++)
//			{
//    al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * (7 + j * 7), line_y, ALLEGRO_ALIGN_RIGHT, "%i", w.core[bcp_state.watch_core_index].message[i].data [j]);
//			}

	 	if (w.core[bcp_state.watch_core_index].message_reading == i
				&& w.core[bcp_state.watch_core_index].message_position >= 0
				&& w.core[bcp_state.watch_core_index].message_position < MESSAGE_LENGTH)
			{
				int ry1 = line_y - 1;
				int ry2 = line_y + BCODE_LINE_HEIGHT - scaleUI_y(FONT_BASIC, 3);

				int draw_message_pos = w.core[bcp_state.watch_core_index].message_position;
				if (draw_message_pos > 4)
				{
					ry1 += BCODE_LINE_HEIGHT;
					draw_message_pos -= 4;
				}
				int rx1 = base_text_x + font[FONT_BASIC].width * 6 + (font[FONT_BASIC].width * 7 * draw_message_pos);
				int rx2 = rx1 + font[FONT_BASIC].width * 7;

				        al_draw_filled_rectangle(rx1,
																																	    ry1,
																																	    rx2,
																																	    ry2,
																																	    colours.base_trans [COL_YELLOW] [SHADE_LOW] [TRANS_MED]);

			}


   al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_MESSAGE] [SHADE_MAX], base_text_x + font[FONT_BASIC].width, line_y, ALLEGRO_ALIGN_LEFT,
																	"data %7d%7d%7d%7d",
																	w.core[bcp_state.watch_core_index].message[i].data [0],
																	w.core[bcp_state.watch_core_index].message[i].data [1],
																	w.core[bcp_state.watch_core_index].message[i].data [2],
																	w.core[bcp_state.watch_core_index].message[i].data [3]);

		 line_y += BCODE_LINE_HEIGHT;

   al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_MESSAGE] [SHADE_MAX], base_text_x + font[FONT_BASIC].width, line_y, ALLEGRO_ALIGN_LEFT,
																	"     %7d%7d%7d%7d",
																	w.core[bcp_state.watch_core_index].message[i].data [4],
																	w.core[bcp_state.watch_core_index].message[i].data [5],
																	w.core[bcp_state.watch_core_index].message[i].data [6],
																	w.core[bcp_state.watch_core_index].message[i].data [7]);


//   char message_line_text [80] = "data ";

//   for (j = 0; j < w.core[bcp_state.watch_core_index].message[i].length; j ++)
//    sprintf(message_line_text + strlen(message_line_text), "%i,", w.core[bcp_state.watch_core_index].message[i].data [j]);

//   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], base_text_x + font[FONT_BASIC].width, line_y, ALLEGRO_ALIGN_LEFT, "%s",message_line_text);//"contents %i,%i,%i,%i,%i,%i,%i,%i",

//																	w.core[bcp_state.watch_core_index].message[i].data [0]);


//	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], variable_name_x, line_y, ALLEGRO_ALIGN_LEFT, "%s", message_line_text);

		 line_y += BCODE_LINE_HEIGHT;

	 }
	}




// target memory:


	clip_width = panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].w;

	clip_right_x = panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].w;

	if (clip_right_x > panel[PANEL_BCODE].x2)
	{
		clip_right_x = panel[PANEL_BCODE].x2;
		clip_width = clip_right_x - (panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].x1);
		if (clip_width <= 0)
			goto finished_drawing;
	}

	al_set_clipping_rectangle(panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].x1,
																											panel[PANEL_BCODE].y1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].y1,
																											clip_width,
																											panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].h);
 al_clear_to_color(colours.base [BCODE_PANEL_COL_BACKGROUND] [SHADE_LOW]);



 line_y = panel[PANEL_BCODE].y1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].y1;
 line_index	= bcp_state.subpanel_stack_line;
 base_text_x = panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].x1 + 3;
 text_x = base_text_x;

// int target_subpanel_lines = (panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].h - BCODE_LINE_HEADER_SIZE - 1) / BCODE_LINE_HEIGHT;

 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_HEADING] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 1, line_y + 2, ALLEGRO_ALIGN_LEFT, "Target memory");

 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_MISC] [SHADE_HIGH], base_text_x + font[FONT_BASIC].width * 4, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_RIGHT, "addr");


 if (bcp_state.bcp_mode == BCP_MODE_PROCESS)
	{
  al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_TARGET] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 11, line_y + BCODE_LINE_HEIGHT + 2, ALLEGRO_ALIGN_RIGHT, "target");
	}

 line_y += BCODE_LINE_HEADER_SIZE;

	if (mouse_in_subpanel == FSP_BCODE_TARGET_MEMORY)
	{
		mouse_on_line = mouse_y - line_y;
		mouse_on_line /= BCODE_LINE_HEIGHT;
		if (mouse_on_line < 0
			|| mouse_on_line >= bcp_state.subpanel_target_total_lines)
				mouse_on_line = 1;
	}
	 else
			mouse_on_line = -1;

	for (i = 0; i < bcp_state.subpanel_target_total_lines; i ++)
	{

		line_index = bcp_state.subpanel_target_line + i;

		int target_address = line_index;
		if (target_address < 0
			|| target_address >= PROCESS_MEMORY_SIZE)
				break;

	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_TARGET] [SHADE_MAX], text_x + font[FONT_BASIC].width * 4, line_y, ALLEGRO_ALIGN_RIGHT, "%i", target_address);

  if (bcp_state.bcp_mode == BCP_MODE_PROCESS)
		{
			if (w.core[bcp_state.watch_core_index].process_memory[target_address] == -1)
 			al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_TARGET] [SHADE_MED], base_text_x + font[FONT_BASIC].width * 11, line_y, ALLEGRO_ALIGN_RIGHT, "empty");
					else
			  {
   			if (w.core[bcp_state.watch_core_index].process_memory_timestamp[target_address] != w.core[w.core[bcp_state.watch_core_index].process_memory[target_address]].created_timestamp)
 			   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_RED] [SHADE_MED], base_text_x + font[FONT_BASIC].width * 11, line_y, ALLEGRO_ALIGN_RIGHT, "lost");
 			    else
								{

  			  if (mouse_on_line == i)
								check_bcp_mouseover(base_text_x + font[FONT_BASIC].width * 6 - 2,
																												base_text_x + font[FONT_BASIC].width * 30 + 3,
																												line_y,
																												BCP_MOUSEOVER_CORE_INDEX,
																												w.core[bcp_state.watch_core_index].process_memory[target_address],
																												COL_BLUE);



   			   al_draw_textf(font[FONT_BASIC].fnt, colours.base [BCODE_PANEL_COL_TARGET] [SHADE_MAX], base_text_x + font[FONT_BASIC].width * 11, line_y, ALLEGRO_ALIGN_RIGHT, "%i", w.core[bcp_state.watch_core_index].process_memory[target_address]);
   			   al_draw_textf(font[FONT_BASIC].fnt,
																										colours.team [w.core[w.core[bcp_state.watch_core_index].process_memory[target_address]].player_index] [TCOL_MAP_POINT_MAX],
																										base_text_x + font[FONT_BASIC].width * 12,
																										line_y,
																										ALLEGRO_ALIGN_LEFT,
																										"%s",
																										templ[w.core[w.core[bcp_state.watch_core_index].process_memory[target_address]].player_index][w.core[w.core[bcp_state.watch_core_index].process_memory[target_address]].template_index].name);
								}

			  }

		}


//	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], variable_name_x, line_y, ALLEGRO_ALIGN_LEFT, "%s", tdb->variable[memory_address].name);

		line_y += BCODE_LINE_HEIGHT;

	}

	draw_scrollbar(SLIDER_BCODE_TARGET_SCROLLBAR_V);


/*

Also need (maybe):
- targetting memory
 addr core_index template?(colour coded for player_index?)
- messages
 - channel, source, priority, target, contents...
- commands? Not sure

+ remember to prevent commands/selection during execution!!
 (or maybe not? Could just give a warning)
+ remember to set debug mode!

*/













/*

Registers etc:

*/

finished_drawing:

	al_set_clipping_rectangle(panel[PANEL_BCODE].x1, panel[PANEL_BCODE].y1, panel[PANEL_BCODE].w, panel[PANEL_BCODE].h);

 int register_x = panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x1;
 int register_y = panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].y2 + 5;
 int register_h = scaleUI_y(FONT_BASIC, 18);
 int register_text_x = register_x + scaleUI_x(FONT_BASIC, 4);
 int register_text_y = register_y + scaleUI_y(FONT_BASIC, 5);

 if (bcp_state.bcp_mode == BCP_MODE_PROCESS
		&& game.watching == WATCH_PAUSED_TO_WATCH)
	{

 add_menu_button(register_x, register_y, register_x + font[FONT_BASIC].width * 60, register_y + register_h, colours.base [COL_AQUA] [SHADE_LOW], 4, 2);
 draw_menu_buttons();
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_AQUA] [SHADE_MAX], register_text_x, register_text_y, ALLEGRO_ALIGN_LEFT, "REGISTERS");
 register_text_x += font[FONT_BASIC].width * 12;
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_AQUA] [SHADE_HIGH], register_text_x, register_text_y, ALLEGRO_ALIGN_LEFT, "A");
 register_text_x += font[FONT_BASIC].width * 8;
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_AQUA] [SHADE_HIGH], register_text_x, register_text_y, ALLEGRO_ALIGN_RIGHT, "%i", vmstate.vm_register [VM_REG_A]);
 register_text_x += font[FONT_BASIC].width * 3;
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_AQUA] [SHADE_HIGH], register_text_x, register_text_y, ALLEGRO_ALIGN_LEFT, "B");
 register_text_x += font[FONT_BASIC].width * 8;
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_AQUA] [SHADE_HIGH], register_text_x, register_text_y, ALLEGRO_ALIGN_RIGHT, "%i", vmstate.vm_register [VM_REG_B]);
 register_text_x += font[FONT_BASIC].width * 3;
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_AQUA] [SHADE_HIGH], register_text_x, register_text_y, ALLEGRO_ALIGN_LEFT, "Stack");
 register_text_x += font[FONT_BASIC].width * 11;
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_AQUA] [SHADE_HIGH], register_text_x, register_text_y, ALLEGRO_ALIGN_RIGHT, "%i", vmstate.stack_pos);
 register_text_x += font[FONT_BASIC].width * 3;
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_AQUA] [SHADE_HIGH], register_text_x, register_text_y, ALLEGRO_ALIGN_LEFT, "Exec");
 register_text_x += font[FONT_BASIC].width * 10;
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_AQUA] [SHADE_HIGH], register_text_x, register_text_y, ALLEGRO_ALIGN_RIGHT, "%i", vmstate.bcode_pos);

 register_x += font[FONT_BASIC].width * 62;
 add_menu_button(register_x, register_y, register_x + font[FONT_BASIC].width * 42, register_y + register_h, colours.base [COL_TURQUOISE] [SHADE_LOW], 4, 2);
 draw_menu_buttons();
 register_text_x = register_x + scaleUI_x(FONT_BASIC, 4);
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], register_text_x, register_text_y, ALLEGRO_ALIGN_LEFT, "Instr");
 register_text_x += font[FONT_BASIC].width * 16;
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], register_text_x, register_text_y, ALLEGRO_ALIGN_RIGHT, "%i(%i)",
															vmstate.instructions_left,
															w.core[bcp_state.watch_core_index].instructions_per_cycle);
 register_text_x += font[FONT_BASIC].width * 3;
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], register_text_x, register_text_y, ALLEGRO_ALIGN_LEFT, "Power");
 register_text_x += font[FONT_BASIC].width * 16;
 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], register_text_x, register_text_y, ALLEGRO_ALIGN_RIGHT, "%i(%i)",
															w.core[bcp_state.watch_core_index].power_left,
															w.core[bcp_state.watch_core_index].power_capacity);

	}

/*

Buttons etc:

*/


 int button_base_x = panel[PANEL_BCODE].x1 + panel[PANEL_BCODE].subpanel[FSP_BCODE_BCODE].x1;
 int button_base_y = register_y + register_h + 5;
 int button_w = scaleUI_x(FONT_BASIC, 150);
 int button_h = scaleUI_y(FONT_BASIC, 16);
 int button_gap = 3;
 int button_x, button_y;
 int mouse_over_button = -1;

 int buttons_shown = 1;
 int first_button = 0;
 int button_col = COL_BLUE;

 if (bcp_state.bcp_mode == BCP_MODE_PROCESS)
	{
		if (game.watching == WATCH_PAUSED_TO_WATCH)
		{
			buttons_shown = BCP_BUTTONS;
			first_button = 1; // don't show "watch selected process"
		}
		  else
  			buttons_shown = BCP_BUTTONS - 1; // no "finish"
	}

 for (i = first_button; i < buttons_shown; i ++)
	{
		button_x = button_base_x;
		button_y = button_base_y + (i * (button_h + button_gap));
		if (control.mouse_x_screen_pixels >= button_x
		 && control.mouse_x_screen_pixels <= button_x + button_w
		 && control.mouse_y_screen_pixels >= button_y
		 && control.mouse_y_screen_pixels <= button_y + button_h + button_gap)
				mouse_over_button = i;
		button_col = COL_BLUE;

		if (i == BCP_BUTTON_STEP_THROUGH_FAST
 		&& bcp_state.bcp_wait_mode == BCP_WAIT_STEP_FAST)
				button_col = COL_CYAN;
		if (i == BCP_BUTTON_STEP_THROUGH_SLOW
 		&& bcp_state.bcp_wait_mode == BCP_WAIT_STEP_SLOW)
				button_col = COL_CYAN;
		if (i == BCP_BUTTON_WAIT
 		&& bcp_state.bcp_wait_mode != BCP_WAIT_OFF)
				button_col = COL_CYAN;

  add_menu_string(button_x + 4, button_y + scaleUI_y(FONT_BASIC, 4), &colours.base [button_col] [SHADE_MAX], ALLEGRO_ALIGN_LEFT, FONT_BASIC, bcp_button_text [i]);
  add_menu_button(button_x, button_y, button_x + button_w, button_y + button_h, colours.base [button_col] [SHADE_LOW + (mouse_over_button == i)], 2, 4);
	}

 draw_menu_buttons();
 draw_menu_strings();
 draw_vbuf();

 switch(bcp_state.bcp_mode)
 {
	 case BCP_MODE_EMPTY: // should never happen?
    bcode_panel_general_help_text(BCP_HELP_EMPTY);
				break;
	 case BCP_MODE_PROCESS:

	 	    line_y = button_base_y + scaleUI_y(FONT_BASIC, 4);

  			  if (mouse_y >= line_y
								&& mouse_y <= line_y + BCODE_LINE_HEIGHT)
								check_bcp_mouseover(button_base_x + button_w + 8,
																												button_base_x + button_w + 10 + font[FONT_BASIC].width * 40,
																												line_y,
																												BCP_MOUSEOVER_CORE_INDEX,
																												bcp_state.watch_core_index,
																												COL_BLUE);

 	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH],
																		button_base_x + button_w + 10,
																		line_y, ALLEGRO_ALIGN_LEFT,
																		"Watching process %i (%s)",
																		bcp_state.watch_core_index,
																		templ[bcp_state.player_index][bcp_state.template_index].name);

   if (game.watching == WATCH_PAUSED_TO_WATCH
				|| game.watching == WATCH_ON)
    bcode_panel_general_help_text(BCP_HELP_WATCHING_EXECUTION);
			  else
      bcode_panel_general_help_text(BCP_HELP_WATCHING);
	 	break;
	 case BCP_MODE_TEMPLATE:
// Not sure this is implemented.
 	 al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], button_base_x + button_w + 10, button_base_y + scaleUI_y(FONT_BASIC, 4), ALLEGRO_ALIGN_LEFT,
																		"Showing bcode for player %i template %i (%s)",
																		bcp_state.player_index,
																		bcp_state.template_index,
																		templ[bcp_state.player_index][bcp_state.template_index].name);
	 	break;

 }

if (control.mbutton_press[0] == BUTTON_JUST_PRESSED)
{
 if (mouse_over_button != -1)
	{
		switch(mouse_over_button)
		{
		 case BCP_BUTTON_WATCH_SELECTED:

		 	if (game.phase == GAME_PHASE_WORLD)
				{
					if (command.selected_core [0] != SELECT_EMPTY
					 && command.selected_core [0] != SELECT_TERMINATE)
				 {
					 reset_bcode_panel();
					 bcp_state.bcp_mode = BCP_MODE_PROCESS;
					 bcp_state.watch_core_index = command.selected_core [0];
					 bcp_state.watch_core_timestamp = w.core[command.selected_core [0]].created_timestamp;
					 bcp_state.player_index = w.core[command.selected_core [0]].player_index;
					 bcp_state.template_index = w.core[command.selected_core [0]].template_index;
					 setup_bcode_panel();
				 }
				  else
							write_line_to_log("No process selected.", MLOG_COL_ERROR);
				}
			  else
						write_line_to_log("This only works during a game.", MLOG_COL_ERROR);
				break;
			case BCP_BUTTON_WAIT:
		 	if (bcp_state.bcp_mode == BCP_MODE_PROCESS
					&& bcp_state.watch_core_index >= 0
					&& bcp_state.watch_core_timestamp == w.core[bcp_state.watch_core_index].created_timestamp)
				{
					if (game.watching == WATCH_OFF)
					{
 					game.watching = WATCH_ON;
 					bcp_state.bcp_wait_mode = BCP_WAIT_PAUSE;
 					break;
					}
//		   bcp_state.bcp_wait_mode = BCP_WAIT_OFF;
//					if (bcp_state.bcp_wait_mode == BCP_WAIT_PAUSE)
// 					bcp_state.bcp_wait_mode = BCP_WAIT_PAUSE;
//					  else
				 if (game.watching == WATCH_PAUSED_TO_WATCH)
				 {
//					bcp_state.bcp_wait_mode = BCP_WAIT_OFF; - don't need to change this
 			 	finish_executing_bcode_in_watch();
				  game.watching = WATCH_FINISHED_JUST_STOP_PAUSING;
				 }
		   bcp_state.bcp_wait_mode = BCP_WAIT_OFF;
				}
				break;
			case BCP_BUTTON_FINISH:
				if (game.watching == WATCH_PAUSED_TO_WATCH)
				{
//					bcp_state.bcp_wait_mode = BCP_WAIT_OFF; - don't need to change this
 				finish_executing_bcode_in_watch();
				 game.watching = WATCH_FINISHED_BUT_STILL_WATCHING;
				}
				break;

			case BCP_BUTTON_ADVANCE:
		 	if (bcp_state.bcp_mode == BCP_MODE_PROCESS
					&& bcp_state.watch_core_index >= 0
					&& bcp_state.watch_core_timestamp == w.core[bcp_state.watch_core_index].created_timestamp)
				{
					if (game.watching == WATCH_OFF)
 					game.watching = WATCH_ON;
					if (bcp_state.bcp_wait_mode == BCP_WAIT_PAUSE)
 					bcp_state.bcp_wait_mode = BCP_WAIT_PAUSE_ONE_STEP;
					  else
					   bcp_state.bcp_wait_mode = BCP_WAIT_PAUSE;
				}
				break;
			case BCP_BUTTON_STEP_THROUGH_SLOW:
		 	if (bcp_state.bcp_mode == BCP_MODE_PROCESS
					&& bcp_state.watch_core_index >= 0
					&& bcp_state.watch_core_timestamp == w.core[bcp_state.watch_core_index].created_timestamp)
				{
					if (game.watching == WATCH_OFF)
					{
					 game.watching = WATCH_ON;
 					bcp_state.bcp_wait_mode = BCP_WAIT_STEP_SLOW;
					}
					  else
							{
								if (bcp_state.bcp_wait_mode == BCP_WAIT_STEP_SLOW)
								 bcp_state.bcp_wait_mode = BCP_WAIT_PAUSE;
								  else
      					bcp_state.bcp_wait_mode = BCP_WAIT_STEP_SLOW;
							}
				}
				break;
			case BCP_BUTTON_STEP_THROUGH_FAST:
		 	if (bcp_state.bcp_mode == BCP_MODE_PROCESS
					&& bcp_state.watch_core_index >= 0
					&& bcp_state.watch_core_timestamp == w.core[bcp_state.watch_core_index].created_timestamp)
				{
					if (game.watching == WATCH_OFF)
					{
					 game.watching = WATCH_ON;
 					bcp_state.bcp_wait_mode = BCP_WAIT_STEP_FAST;
					}
					  else
							{
								if (bcp_state.bcp_wait_mode == BCP_WAIT_STEP_FAST)
								 bcp_state.bcp_wait_mode = BCP_WAIT_PAUSE;
								  else
      					bcp_state.bcp_wait_mode = BCP_WAIT_STEP_FAST;
							}
				}
				break;
			case BCP_BUTTON_STOP_WATCHING:
				bcp_state.bcp_wait_mode = BCP_WAIT_OFF;
				if (game.phase == GAME_PHASE_WORLD
					&&	game.watching == WATCH_PAUSED_TO_WATCH)
				{
				 finish_executing_bcode_in_watch();
     game.watching = WATCH_FINISHED_AND_STOP_WATCHING;
				}
				 else
					{
						bcp_state.bcp_mode = BCP_MODE_EMPTY;
						game.watching = WATCH_OFF;
					}
				break;
/*			case BCP_BUTTON_FINISH:
				if (game.watching == WATCH_PAUSED_TO_WATCH)
				{
//					bcp_state.bcp_wait_mode = BCP_WAIT_OFF; - don't need to change this
 				finish_executing_bcode_in_watch();
				 game.watching = WATCH_FINISHED_BUT_STILL_WATCHING;
				}
				break;
*/


		}

		return;

	}


			if (bcp_state.bcp_mode != BCP_MODE_EMPTY
				&& game.phase == GAME_PHASE_WORLD
				&& bcp_state.mouseover_time == inter.running_time)
			{
				switch(bcp_state.mouseover_type)
				{
				 case BCP_MOUSEOVER_CORE_INDEX:
					 if (bcp_state.mouseover_value >= 0
				   && bcp_state.mouseover_value < w.max_cores
				   && w.core[bcp_state.mouseover_value].exists)
			   {
							view.camera_x = w.core[bcp_state.mouseover_value].core_position.x;
							view.camera_y = w.core[bcp_state.mouseover_value].core_position.y;
			   }
			   break;
					case BCP_MOUSEOVER_SOURCE_LINE:
					{

      open_template(bcp_state.player_index, bcp_state.template_index);

      struct source_edit_struct* se = get_current_source_edit();

      if (se == NULL
       || se->type != SOURCE_EDIT_TYPE_SOURCE)
       break;

      int new_source_line = bcp_state.mouseover_value;// - 1;
      if (new_source_line < 0) // possible?
		     new_source_line = 0;
      if (new_source_line >= SOURCE_TEXT_LINES - 3) // arbitrary upper limit
		     new_source_line = SOURCE_TEXT_LINES - 3;

      se->cursor_line = new_source_line;
      se->cursor_pos = 0;
      se->cursor_base = se->cursor_pos;
      se->selected = 0;

      window_find_cursor(se);
				 }
					break;
				}

			} // end if (bcp_state.mouseover_time == inter.running_time)

} // end if left mouse button pressed


 if (control.mbutton_press [1] == BUTTON_JUST_PRESSED) // right mouse button
	{


			if (bcp_state.bcp_mode != BCP_MODE_EMPTY
				&& game.phase == GAME_PHASE_WORLD
				&& bcp_state.mouseover_time == inter.running_time)
			{
				switch(bcp_state.mouseover_type)
				{
				 case BCP_MOUSEOVER_OPCODE:
					 if (bcp_state.mouseover_value >= 0
				   && bcp_state.mouseover_value < INSTRUCTIONS)
			   {
			   	reset_log();
							start_log_line(MLOG_COL_HELP);
							write_to_log("Help: bcode instruction (");
							write_number_to_log(bcp_state.mouseover_value);
							write_to_log(")");
							finish_log_line();
							start_log_line(MLOG_COL_COMPILER);
							write_to_log(instruction_set[bcp_state.mouseover_value].name);
							switch(instruction_set[bcp_state.mouseover_value].operand_type [0])
							{
								case OPERAND_TYPE_NUMBER:
  							write_to_log(" <number>"); break;
								case OPERAND_TYPE_BCODE_ADDRESS:
  							write_to_log(" <bcode_address>"); break;
								case OPERAND_TYPE_MEMORY:
  							write_to_log(" <memory_address>"); break;
							}
							if (instruction_set[bcp_state.mouseover_value].operands > 1)
         write_to_log(" <number>");
							if (instruction_set[bcp_state.mouseover_value].operands > 2)
         write_to_log(" <number>");
							finish_log_line();
							print_help_string(opcode_help [bcp_state.mouseover_value]);
			   }
			   break;
				}

			} // end if (bcp_state.mouseover_time == inter.running_time)



	}

// can return in the mouse button input block above.

}

static void bcode_panel_general_help_text(int help_type)
{

	int help_line_y = panel[PANEL_BCODE].y2 - font[FONT_BASIC].width * 19;
	int help_text_x = panel[PANEL_BCODE].x1 + font[FONT_BASIC].width * 40;

// al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_TURQUOISE] [SHADE_MAX], base_text_x + font[FONT_BASIC].width, line_y, ALLEGRO_ALIGN_LEFT,


	switch(help_type)
	{
	 case BCP_HELP_EMPTY:
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], help_text_x - 8, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"Bytecode panel");
			help_line_y += BCODE_LINE_HEIGHT + 3;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"This panel shows the bytecode version of the source code that controls a process.");
			help_line_y += BCODE_LINE_HEIGHT;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"Use it for debugging, or to see how the compiler works.");
			help_line_y += BCODE_LINE_HEIGHT;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"To begin, select a process (in the main game display) and click 'Watch selected process'.");
	 	break;

	 case BCP_HELP_WATCHING:
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], help_text_x - 8, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"Bytecode panel - watching");
			help_line_y += BCODE_LINE_HEIGHT + 3;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"The Bytecode instructions window shows the detailed code produced by the compiler for this process.");
			help_line_y += BCODE_LINE_HEIGHT;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	" - Click on a value in the 'src' column and the editor will go to that line in the source code.");
			help_line_y += BCODE_LINE_HEIGHT;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	" - Right-click on an opcode name (e.g. 'pushA') for an explanation of what it does.");
			help_line_y += BCODE_LINE_HEIGHT + 3;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"Other windows show the process' memory, its stack (only while executing), messages received");
			help_line_y += BCODE_LINE_HEIGHT;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	" and targetting memory (click on a target to go to that target).");
			help_line_y += BCODE_LINE_HEIGHT + 3;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"Click on 'Watch execution' (or the other buttons below it) to watch the process execute.");


/*
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"The Variable memory window shows the contents of the process' memory.");
			help_line_y += BCODE_LINE_HEIGHT;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"The Stack window shows the process' stack (only while it's executing).");
			help_line_y += BCODE_LINE_HEIGHT;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"The Messages window shows broadcasts and transmissions received by the process.");
			help_line_y += BCODE_LINE_HEIGHT;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"The Target memory window shows the process' targetting memory (click on an entry to go to the target).");
*/
	 	break;

  case BCP_HELP_WATCHING_EXECUTION:
   al_draw_textf(font[FONT_SQUARE].fnt, colours.base [COL_BLUE] [SHADE_MAX], help_text_x - 8, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"Bytecode panel - watching execution");
			help_line_y += BCODE_LINE_HEIGHT + 3;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"The Bytecode instructions window shows the process' code executing.");
			help_line_y += BCODE_LINE_HEIGHT;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	" - Use the buttons on the left to stop watching, control execution speed or finish this cycle.");
			help_line_y += BCODE_LINE_HEIGHT;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"   (Execution will also stop while the game is paused.)");
			help_line_y += BCODE_LINE_HEIGHT + 3;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"Other windows are updated as the process executes.");
			help_line_y += BCODE_LINE_HEIGHT + 3;
   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_HIGH], help_text_x, help_line_y, ALLEGRO_ALIGN_LEFT,
																	"Registers and remaining instructions and power are shown below the Bytecode window.");
			break;



	}


}



static void setup_bcode_panel(void)
{

 struct template_debug_struct* tdb = &template_debug[bcp_state.player_index][bcp_state.template_index];

 bcp_state.subpanel_bcode_total_lines = tdb->total_lines_bcode_window;
 if (bcp_state.subpanel_bcode_line > tdb->total_lines_bcode_window)
		bcp_state.subpanel_bcode_line = tdb->total_lines_bcode_window - 1;
 slider[SLIDER_BCODE_BCODE_SCROLLBAR_V].value_max = tdb->total_lines_bcode_window - bcp_state.lines;
 if (slider[SLIDER_BCODE_BCODE_SCROLLBAR_V].value_max < 0)
		slider[SLIDER_BCODE_BCODE_SCROLLBAR_V].value_max = 1;
 slider[SLIDER_BCODE_BCODE_SCROLLBAR_V].value_min = 0;
 slider[SLIDER_BCODE_BCODE_SCROLLBAR_V].track_increment = bcp_state.lines - 1;
 reset_slider_length(SLIDER_BCODE_BCODE_SCROLLBAR_V, slider[SLIDER_BCODE_BCODE_SCROLLBAR_V].total_length, tdb->total_lines_bcode_window / bcp_state.lines);

 slider[SLIDER_BCODE_MEMORY_SCROLLBAR_V].value_max = MEMORY_SIZE - bcp_state.lines;
 slider[SLIDER_BCODE_MEMORY_SCROLLBAR_V].value_min = 0;
 slider[SLIDER_BCODE_MEMORY_SCROLLBAR_V].track_increment = bcp_state.lines - 1;
 reset_slider_length(SLIDER_BCODE_MEMORY_SCROLLBAR_V, slider[SLIDER_BCODE_MEMORY_SCROLLBAR_V].total_length, MEMORY_SIZE / bcp_state.lines);

 slider[SLIDER_BCODE_STACK_SCROLLBAR_V].value_max = VM_STACK_SIZE - bcp_state.lines;
 slider[SLIDER_BCODE_STACK_SCROLLBAR_V].value_min = 0;
 slider[SLIDER_BCODE_STACK_SCROLLBAR_V].track_increment = bcp_state.lines - 1;
 reset_slider_length(SLIDER_BCODE_STACK_SCROLLBAR_V, slider[SLIDER_BCODE_STACK_SCROLLBAR_V].total_length, VM_STACK_SIZE / bcp_state.lines);

 bcp_state.subpanel_target_total_lines = (panel[PANEL_BCODE].subpanel[FSP_BCODE_TARGET_MEMORY].h - (font[FONT_BASIC].height * 2 + 7)) / (font[FONT_BASIC].height + 1);

 slider[SLIDER_BCODE_TARGET_SCROLLBAR_V].value_max = PROCESS_MEMORY_SIZE - bcp_state.subpanel_target_total_lines;
 slider[SLIDER_BCODE_TARGET_SCROLLBAR_V].value_min = 0;
 slider[SLIDER_BCODE_TARGET_SCROLLBAR_V].track_increment = bcp_state.subpanel_target_total_lines - 1;
 reset_slider_length(SLIDER_BCODE_TARGET_SCROLLBAR_V, slider[SLIDER_BCODE_TARGET_SCROLLBAR_V].total_length, PROCESS_MEMORY_SIZE / bcp_state.subpanel_target_total_lines);

//void reset_slider_length(int s, int new_total_length, int slider_represents_size)

}




static void bcode_subpanel_mousewheel(int subpanel_index, int* line_ptr, int maximum_value)
{

	if (ex_control.mousewheel_change < 0)
	{
		*line_ptr -= 8;
		if (*line_ptr < 0)
			*line_ptr = 0;
		return;
	}

	*line_ptr += 8;
	if (*line_ptr > maximum_value)
		*line_ptr = maximum_value;

}




static void check_bcp_mouseover(int x_min, int x_max, int line_y, int mouseover_type, int mouseover_value, int box_colour)
{

							if (control.mouse_x_screen_pixels >= x_min
								&& control.mouse_x_screen_pixels <= x_max)
							{
								bcp_state.mouseover_time = inter.running_time;
								bcp_state.mouseover_type = mouseover_type;
								bcp_state.mouseover_value = mouseover_value;

        al_draw_filled_rectangle(x_min,
																																	line_y - 2,
																																	x_max,
																																	line_y + BCODE_LINE_HEIGHT - scaleUI_y(FONT_BASIC, 2),
																																	colours.base_trans [box_colour] [SHADE_MED] [TRANS_MED]);

							}


}

