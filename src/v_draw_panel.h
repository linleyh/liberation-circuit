

#ifndef H_V_DRAW_PANEL
#define H_V_DRAW_PANEL

//#define DEBUGGER_VARIABLES 64
#define DEBUGGER_CLASSES 16
#define DEBUGGER_LABELS 64
#define DEBUGGER_EXIT_POINTS 1000
#define DEBUGGER_NAME_LENGTH 32


struct debugger_variable_struct
{
	char name [DEBUGGER_NAME_LENGTH+1];
//	int address;
};

struct debugger_label_struct
{
	char name [DEBUGGER_NAME_LENGTH+1];
	int address;
};

struct debugger_expoint_struct
{
	int type; // -1 = empty, 0 = false, 1 = true
	int address;
};

struct debugger_class_struct
{
	char name [DEBUGGER_NAME_LENGTH+1];
};

// 2100 is probably too many...
#define DEBUGGER_LINES 2100

#define SOURCE_LINE_EMPTY -1
#define SOURCE_LINE_JUMP_TABLE -2
#define SOURCE_LINE_JUMP_TABLE_DEFAULT -3
#define SOURCE_LINE_STRING -4
#define SOURCE_LINE_LABEL -5

struct debugger_line_struct
{
	int bcode_address; // -1 if empty
	int source_line; // source line of op, or SOURCE_LINE_*
	int special_value; // special value for lines with special SOURCE_LINE_* values
//	int label_index; // -1 if no label or empty; otherwise, index of label in label array
};

struct template_debug_struct
{

	struct debugger_variable_struct variable [MEMORY_SIZE];
 struct debugger_label_struct label [DEBUGGER_LABELS];
// struct debugger_expoint_struct expoint [DEBUGGER_EXIT_POINTS];
// struct debugger_class_struct dclass [DEBUGGER_CLASSES];

 struct debugger_line_struct debugger_line [DEBUGGER_LINES];

 int total_lines_bcode_window;


};

enum
{
BCP_MOUSEOVER_NONE,
BCP_MOUSEOVER_SOURCE_LINE,
BCP_MOUSEOVER_BCODE_ADDRESS,
BCP_MOUSEOVER_VARIABLE,
BCP_MOUSEOVER_OPCODE,
BCP_MOUSEOVER_CORE_INDEX,
BCP_MOUSEOVER_LABEL,

};

enum
{
BCP_MODE_EMPTY, // nothing in bc panel
BCP_MODE_TEMPLATE, // just looking at a template (determined by dwindow template)
BCP_MODE_PROCESS,	// watching a specific process

BCP_MODES
};

enum
{
BCP_WAIT_OFF,
BCP_WAIT_PAUSE,
BCP_WAIT_PAUSE_ONE_STEP,
BCP_WAIT_STEP_SLOW,
BCP_WAIT_STEP_FAST,

};

struct bcode_panel_state_struct
{
 int bcp_mode;
 int bcp_wait_mode;
 int watch_core_index;
 timestamp watch_core_timestamp;
 int step_counter; // frame counter for step-through mode

 int player_index;
 int template_index;

	int subpanel_bcode_line;
	int subpanel_bcode_total_lines;
	int subpanel_variable_line;
	int subpanel_stack_line;
	int subpanel_target_line;
	int subpanel_target_total_lines;

	int lines;

//	int mouse_x, mouse_y;

	timestamp mouseover_time;
	int mouseover_type;
	int mouseover_value;

};


void init_bcode_panel(void);
void reset_bcode_panel(void);
void draw_bcode_panel(void);

#endif
