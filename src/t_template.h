
#ifndef H_T_TEMPLATE
#define H_T_TEMPLATE


//#define TEMPLATES 8
// this is per player


struct template_state_struct
{
	int template_player_tab;
	int current_template;

};

void init_template(struct template_struct* tpl, int player_index, int templ_index);
void clear_template_including_source(struct template_struct* tpl);
void clear_template_but_not_source(struct template_struct* clear_template);
void clear_remaining_templates(int player_index, int first_template);
void init_all_templates(void);
void reset_template_member(struct template_member_struct* member);
void open_new_template(struct template_struct* tpl);
void open_template(int player_index, int template_index);
void calculate_template_cost_and_power(struct template_struct* costed_templ);

void template_panel_button(int element);

void copy_template(struct template_struct* target_templ, struct template_struct* source_templ, int copy_design);

int lock_template(struct template_struct* lock_templ);
void unlock_template(int player_index, int template_index);
void prepare_templates_for_new_game(void);

void calculate_template_member_cost(struct template_struct* tpl, int member_index);
int check_template_member_objects(struct template_struct* templ, int member_index);

#endif


//#include "e_header.h"
/*
void init_templates(void);
//void setup_templates(int game_type);

void setup_system_template(void);
void setup_templates_for_game_start(void);
void setup_templates_for_mission_menu(void);
void setup_templates_for_advanced_mission_menu(void);
void setup_templates_for_default_template_init(void);
void copy_default_mission_templates_to_actual_templates(int starting_from_template, int advanced);
void copy_mission_templates_to_default(int copy_standard, int copy_advanced);
void finish_template_setup(void);

void reset_templates(void);
void reset_template_x_values(void);

void draw_template_bmp(void);
int find_loaded_gamefile_template(void);
int find_template_access_index(int player_index, int templ_access_index);
int add_template(int type, int player, int fixed, int access_index, int category_heading);
void assign_template_references(void);

void run_templates(void);


void open_templates(void);
void close_templates(void);

void clear_template(int t);
void set_opened_template(int t, int type, int fixed);
*/


//#define TEMPL_BUTTONS 2
//#define TEMPL_NAME_LENGTH 16
/*
enum
{
TEMPL_BUTTON_OPEN_FILE,
TEMPL_BUTTON_IMPORT,
TEMPL_BUTTON_LINK,
TEMPL_BUTTON_CLEAR,

TEMPL_BUTTON_TYPES
};*/

/*
// Put everything in here that should be copied from one template to another when duplicating templates
// Leave out everything that forms part of the template's user interface (button locations)
//  or the basic type of the template (type, player etc)
struct templ_contentstruct
{
 char file_name [FILE_NAME_LENGTH];
 char file_path [FILE_PATH_LENGTH];
 int loaded; // 0 if nothing in this template
 int access_index; // if template is accessible from the game itself, this is its access index (which can be used by new_proc methods). Is -1 if not accessible.
 int origin; // where did the template come from? (e.g. imported from editor)

 struct bcode_struct bcode; // remember to set bcode note to NULL

// This structure is copied with the = operator in copy_template_contents().
// If this structure (or the bcode structure) contains pointers, copy_template_contents() may need
//  to deal with them specially (as it does for the bcode.op pointer)

};*/

//void copy_template_contents(struct templ_contentstruct* tc_target, struct templ_contentstruct* tc_source);

/*
struct templstruct
{
 int type; // this is determined by the game type (e.g. there may or may not be a template position for client programs)
 int player;
 int fixed; // 1 if template is fixed and cannot be altered by user
// char templ_name [TEMPL_NAME_LENGTH]; // contains name of template if specified by system program when copying from bcode
 int highlight;

 int category_heading; // if this template is the first (or only) in its category (e.g. first in list of player 1 procs) this is 1, so a heading will be displayed (heading type is determined by template type)

 int buttons;
 int button_type [TEMPL_BUTTONS];
// int button_x1 [TEMPL_BUTTONS];
 int button_y1 [TEMPL_BUTTONS];
// int button_x2 [TEMPL_BUTTONS];
 int button_y2 [TEMPL_BUTTONS];

 int y1, y2;

 struct templ_contentstruct contents;

};


enum
{
TEMPL_ORIGIN_NONE, // no origin
TEMPL_ORIGIN_FILE, // loaded directly from file
TEMPL_ORIGIN_EDITOR, // imported from editor
TEMPL_ORIGIN_SYSTEM, // copied by system program
TEMPL_ORIGIN_CLIENT, // copied by client program (operator or delegate)
TEMPL_ORIGIN_TURNFILE, // loaded from a turnfile

TEMPL_ORIGINS
};


struct import_liststruct
{
 int open; // is 0 if no tab open in editor at this position
 char name [TAB_NAME_LENGTH];
 struct source_edit_struct* se;
};




struct tstatestruct
{

// the contents of this struct are not saved when the game is saved (whereas the templ template structures are).
// this means that everything in here must be able to be set up by adding the saved templates.

// int h; // total height of template window's contents in pixels
// int window_pos;
// int use_scrollbar; // is 1 if template screen is long enough to need a scrollbar, 0 otherwise
// struct slider_struct tscrollbar; // vertical scrollbar if template list is too long to display

 struct import_liststruct import_list [ESOURCES];
// int import_list_open;
// int list_t;
// int list_highlight;

 int templ_setup_counter; // this is the counter used when setting up templates; it indicates which template is next to be set up. Is reset to 0 by reset_templates
 int templ_setup_y; // this is how long the template list currently is in pixels

};


enum
{
TEMPL_MISSION_DEFAULT_OPERATOR,
TEMPL_MISSION_DEFAULT_OBSERVER,
TEMPL_MISSION_DEFAULT_DELEGATE,
TEMPL_MISSION_DEFAULT_PROC0,
TEMPL_MISSION_DEFAULT_PROC1,
TEMPL_MISSION_DEFAULT_PROC2,
TEMPL_MISSION_DEFAULT_PROC3,
TEMPL_MISSION_DEFAULTS
};
*/
