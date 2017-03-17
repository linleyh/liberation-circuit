
#ifndef H_M_INPUT
#define H_M_INPUT




enum
{
KEY_CODE_BACKSPACE,
KEY_CODE_DELETE,
KEY_CODE_TAB,
KEY_CODE_ENTER,
KEY_CODE_LEFT,
KEY_CODE_RIGHT,
KEY_CODE_UP,
KEY_CODE_DOWN,
KEY_CODE_PGUP,
KEY_CODE_PGDN,
KEY_CODE_HOME,
KEY_CODE_END,
KEY_CODE_INSERT,
KEY_CODE_PAD_0,
#define KEY_PAD_START KEY_CODE_PAD_0
KEY_CODE_PAD_1,
KEY_CODE_PAD_2,
KEY_CODE_PAD_3,
KEY_CODE_PAD_4,
//KEY_CODE_PAD_5,
KEY_CODE_PAD_6,
KEY_CODE_PAD_7,
KEY_CODE_PAD_8,
KEY_CODE_PAD_9,
KEY_CODE_PAD_DELETE,
#define KEY_PAD_END KEY_CODE_PAD_DELETE
KEY_CODE_PAD_ENTER, // doesn't count as a key_pad code because numlock doesn't affect it
KEY_CODE_SHIFT_L,
KEY_CODE_SHIFT_R,
KEY_CODE_CTRL_L,
KEY_CODE_CTRL_R,
KEY_CODE_ALT,
KEY_CODE_ALT_GR,
KEY_CODE_F1,
KEY_CODE_F2,
KEY_CODE_F3,
KEY_CODE_F4,
KEY_CODE_F5,
KEY_CODE_F6,
KEY_CODE_F7,
KEY_CODE_F8,
KEY_CODE_F9,
KEY_CODE_F10,
KEY_CODE_0,
KEY_CODE_1,
KEY_CODE_2,
KEY_CODE_3,
KEY_CODE_4,
KEY_CODE_5,
KEY_CODE_6,
KEY_CODE_7,
KEY_CODE_8,
KEY_CODE_9,

KEY_CODE_Z,
KEY_CODE_X,
KEY_CODE_C,
KEY_CODE_V,
KEY_CODE_B,
KEY_CODE_N,
KEY_CODE_M,

KEY_CODES
};



enum
{
// More than one actual key may map onto each of these
//  If any are added, or if the order changes, may need to update the text in init.txt
SPECIAL_KEY_BACKSPACE,
SPECIAL_KEY_DELETE,
SPECIAL_KEY_TAB,
SPECIAL_KEY_ENTER,
SPECIAL_KEY_HOME,
SPECIAL_KEY_END,
SPECIAL_KEY_PGUP,
SPECIAL_KEY_PGDN,
SPECIAL_KEY_LEFT,
SPECIAL_KEY_RIGHT,
SPECIAL_KEY_UP,
SPECIAL_KEY_DOWN,
SPECIAL_KEY_INSERT,
SPECIAL_KEY_SHIFT,
SPECIAL_KEY_CTRL,
//SPECIAL_KEY_ALT, not used
//SPECIAL_KEY_ALT_GR,
SPECIAL_KEY_F1,
SPECIAL_KEY_F2,
SPECIAL_KEY_F3,
SPECIAL_KEY_F4,
SPECIAL_KEY_F5,
SPECIAL_KEY_F6,
SPECIAL_KEY_F7,
SPECIAL_KEY_F8,
SPECIAL_KEY_F9,
SPECIAL_KEY_F10,

// numbers are treated as special keys for the purposes of build commands (because they need to be shifted/ctrld)
//  but numbers in text entry uses unicode values and does not use the special key code.
SPECIAL_KEY_0,
SPECIAL_KEY_1,
SPECIAL_KEY_2,
SPECIAL_KEY_3,
SPECIAL_KEY_4,
SPECIAL_KEY_5,
SPECIAL_KEY_6,
SPECIAL_KEY_7,
SPECIAL_KEY_8,
SPECIAL_KEY_9,

// These keys are used for control groups:
//  (they are the bottom row of characters on a QWERTY keyboard but can be remapped)
SPECIAL_KEY_CONTROL_GROUP_0,
SPECIAL_KEY_CONTROL_GROUP_1,
SPECIAL_KEY_CONTROL_GROUP_2,
SPECIAL_KEY_CONTROL_GROUP_3,
SPECIAL_KEY_CONTROL_GROUP_4,
SPECIAL_KEY_CONTROL_GROUP_5,
SPECIAL_KEY_CONTROL_GROUP_6,

SPECIAL_KEYS
};


struct ex_control_struct // this struct holds information taken directly from input, and is updated every frame. It is used to fill in the control_struct and also for editor input. See m_input.c
{
	int old_mouse_x_pixels;
	int old_mouse_y_pixels;

 int mouse_x_pixels;
 int mouse_y_pixels;
 int mb_press [MOUSE_BUTTONS];
 int mousewheel_pos;
 int mousewheel_change;
 int mouse_dragging_panel; // is the mouse dragging the editor panel?
 int panel_drag_ready; // is the mouse in the right position to drag the editor panel? (shows a special line thing)
// int key_press [ALLEGRO_KEY_MAX];
 int special_key_press [SPECIAL_KEYS];
 timestamp special_key_press_time [SPECIAL_KEYS];
 int keys_pressed;
 int sticky_ctrl; // is 1 if control was pressed and the keyboard hasn't been totally released since then (used by the editor to avoid e.g. ctrl-c capturing the c if ctrl is released a moment before the c key)
 int numlock;
 int mouse_on_display;
 int mouse_cursor_type;
 int mouse_grabbed; // should be 1 if the capture_mouse setting is on and the mouse is grabbed.

// timestamp unichar_input_received; // most recent unichar received - based on inter.running_time
 int unichar_input; // used for text input where the character entered is important (e.g. the editor). Hopefully should work correctly for non-QWERTY keyboards.

// int using_slider; // is 1 if mouse is interacting with a slider, 0 otherwise. This is used to prevent simultaneous interaction with multiple sliders.
 // the sl.hold_type value of a slider indicates whether it's the one being used.

 int console_action_type; // console_action values are set when the user clicks on a line in a console that has an action attached to it - see i_console.c
 int console_action_console; // which console was clicked on
 int console_action_val1; // they can be used by the observer console method
 int console_action_val2;
 int console_action_val3;

 int key_code_map [KEY_CODES] [2]; // [0] is ALLEGRO_KEY_ code that maps to this key code. [1] is a SPECIAL_KEY code (more than one ALLEGRO_KEY_ code may map to a special key code)

 int debug_special_keys;

};


void get_ex_control(int close_button_status, int grab_mouse_if_captured);
void init_ex_control(void);
void init_key_type(void);
void init_key_maps(void);

enum
{
 KEY_TYPE_LETTER, // a-z
 KEY_TYPE_NUMBER, // 0-9
 KEY_TYPE_SYMBOL, // any other symbol key e.g. ;
 KEY_TYPE_CURSOR, // moves the cursor e.g. arrow keys or end, or makes some change like deleting something
// KEY_TYPE_NUMPAD, // like cursor but affected by numlock - not presently used
 KEY_TYPE_MOD, // modifier e.g. shift
 KEY_TYPE_OTHER, // ignored keys
};

struct key_typestruct
{
 int type; // basic type e.g. KEY_TYPE_LETTER
 char unshifted;
 char shifted;
};

//extern struct key_typestruct key_type [ALLEGRO_KEY_MAX];


void start_text_input_box(int b, char* input_str, int max_length);
int accept_text_box_input(int b);

enum
{
TEXT_BOX_EDITOR_FIND,
TEXT_BOX_PLAYER_NAME,
TEXT_BOX_MAP_CODE,
TEXT_BOXES
};

#endif
