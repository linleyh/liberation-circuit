

#ifndef H_P_PANELS
#define H_P_PANELS

//void init_panels(void);



enum
{
SP_TYPE_NONE,
SP_TYPE_WHOLE, // SP that takes up whole panel
SP_TYPE_PANEL_RESIZE,

SP_TYPE_EDITOR_WINDOW,
SP_TYPE_EDITOR_TABS,
SP_TYPE_EDITOR_SUBMENU,
SP_TYPE_EDITOR_CLASSES, // ?
SP_TYPE_DESIGN_WINDOW,
SP_TYPE_TEMPLATES_TABS,
SP_TYPE_TEMPLATES_FILE,
SP_TYPE_TEMPLATES_MAIN, // templates

SP_TYPE_WINDOW, // standard window with elements on it if needed

SP_TYPE_LOG_WHOLE,
//SP_TYPE_LOG_SCROLLBAR,
SP_TYPES
};

enum
{
FSP_SYSMENU_WHOLE,
FSP_SYSMENU_PANEL_RESIZE,
FSPS_SYSMENU
};

enum
{
FPE_SYSMENU_PAUSE,
//FPE_SYSMENU_SAVE,
FPE_SYSMENU_QUIT,
FPE_SYSMENU_CONFIRM_QUIT,
FPE_SYSMENU_PANEL_RESIZE,
FPES_SYSMENU
};

// fixed subpanels:
enum
{
FSP_LOG_WHOLE,

FSPS_LOG
};

enum
{
FPE_LOG_WINDOW,
FPE_LOG_SCROLLBAR,

FPE
};

enum
{
FSP_BCODE_PANEL_RESIZE,
FSP_BCODE_BCODE,
FSP_BCODE_MEMORY,
FSP_BCODE_TARGET_MEMORY,
FSP_BCODE_STACK,
FSP_BCODE_CONTROL,
FSP_BCODE_MESSAGES,


FSPS_BCODE
};

enum
{
FPE_BCODE_PANEL_RESIZE,
FPE_BCODE_BCODE_MAIN,
FPE_BCODE_BCODE_SCROLLBAR,
FPE_BCODE_MEMORY_MAIN,
FPE_BCODE_MEMORY_SCROLLBAR,
FPE_BCODE_TARGET_MAIN,
FPE_BCODE_TARGET_SCROLLBAR,
FPE_BCODE_STACK_MAIN,
FPE_BCODE_STACK_SCROLLBAR,
FPE_BCODE_CONTROL,
FPE_BCODE_MESSAGES_MAIN, // no scrollbar

FPES_BCODE
};

enum
{
FSP_DESIGN_WINDOW, // design window
FSP_DESIGN_DATA,
FSP_DESIGN_PANEL_RESIZE,

FSP_DESIGN_TOOLS_EMPTY, // when template empty
FSP_DESIGN_TOOLS_MAIN, // when nothing selected
FSP_DESIGN_TOOLS_MEMBER, // when member selected
FSP_DESIGN_TOOLS_CORE, // when core selected
FSP_DESIGN_TOOLS_EMPTY_LINK, // when vertex selected
FSP_DESIGN_TOOLS_ACTIVE_LINK,
FSP_DESIGN_TOOLS_DELETE, // just a "confirm delete" button
FSP_DESIGN_TOOLS_AUTOCODE, // when autocode button pressed

FSP_DESIGN_SUBTOOLS, // nothing selected
//FSP_DESIGN_SUBTOOLS_MEMBER_SHAPE, // member selected
//FSP_DESIGN_SUBTOOLS_VERTEX_OBJECT, // object selected


// remember limit of 8 subpanels
FSPS_DESIGN
};

enum
{
FPE_DESIGN_WINDOW,
FPE_DESIGN_WINDOW_SCROLLBAR_V,
FPE_DESIGN_WINDOW_SCROLLBAR_H,
FPE_DESIGN_PANEL_RESIZE,
FPE_DESIGN_DATA,

FPE_DESIGN_TOOLS_EMPTY_NEW,

//FPE_DESIGN_TOOLS_MAIN_LOAD,
FPE_DESIGN_TOOLS_MAIN_AUTO,
FPE_DESIGN_TOOLS_MAIN_AUTOCODE,
FPE_DESIGN_TOOLS_MAIN_SYMM,
FPE_DESIGN_TOOLS_MAIN_LOCK,
FPE_DESIGN_TOOLS_MAIN_UNLOCK,
FPE_DESIGN_TOOLS_MAIN_DELETE,
FPE_DESIGN_TOOLS_MAIN_HELP,
FPE_DESIGN_TOOLS_MAIN_HELP_MORE,
// subtools - currently none

FPE_DESIGN_TOOLS_MEMBER_SHAPE,
//FPE_DESIGN_TOOLS_MEMBER_FLIP,
FPE_DESIGN_TOOLS_MEMBER_DELETE,
FPE_DESIGN_TOOLS_MEMBER_EXIT,
// subtools
//FPE_DESIGN_SUBTOOLS_MEMBER_SHAPE,
//FPE_DESIGN_SUBTOOLS_MEMBER_SCROLLBAR_H,

FPE_DESIGN_TOOLS_CORE_CORE_SHAPE,
FPE_DESIGN_TOOLS_CORE_EXIT,

FPE_DESIGN_TOOLS_ADD_COMPONENT,
FPE_DESIGN_TOOLS_CHANGE_UPLINK,

//FPE_DESIGN_TOOLS_VERTEX_OBJ_LINK,
FPE_DESIGN_TOOLS_VERTEX_OBJ_STD,
FPE_DESIGN_TOOLS_VERTEX_OBJ_MOVE,
FPE_DESIGN_TOOLS_VERTEX_OBJ_ATTACK,
FPE_DESIGN_TOOLS_VERTEX_OBJ_DEFEND,
//FPE_DESIGN_TOOLS_VERTEX_OBJ_MISC,
FPE_DESIGN_TOOLS_VERTEX_OBJ_CLEAR,
FPE_DESIGN_TOOLS_VERTEX_EXIT,
//FPE_DESIGN_TOOLS_NEXT_LINK,

FPE_DESIGN_TOOLS_DELETE_CONFIRM,
FPE_DESIGN_TOOLS_DELETE_EXIT,

FPE_DESIGN_TOOLS_AUTOCODE_EXIT,

// subtools
//FPE_DESIGN_SUBTOOLS_VERTEX_OBJ,
//FPE_DESIGN_SUBTOOLS_VERTEX_OBJ_SCROLLBAR_H,


FPE_DESIGN_SUB_BUTTON_0,
FPE_DESIGN_SUB_BUTTON_1,
FPE_DESIGN_SUB_BUTTON_2,
FPE_DESIGN_SUB_BUTTON_3,
FPE_DESIGN_SUB_BUTTON_4,
FPE_DESIGN_SUB_BUTTON_5,
FPE_DESIGN_SUB_BUTTON_6,
FPE_DESIGN_SUB_BUTTON_7,
FPE_DESIGN_SUB_BUTTON_8,
FPE_DESIGN_SUB_BUTTON_9,
FPE_DESIGN_SUB_BUTTON_10,
FPE_DESIGN_SUB_BUTTON_11,
FPE_DESIGN_SUB_BUTTON_12,
FPE_DESIGN_SUB_BUTTON_13,
FPE_DESIGN_SUB_BUTTON_14,
// if adding more design sub-buttons, search all files for FPE_DESIGN_SUB_BUTTON_14
};
/*
#define DESIGN_SUB_BUTTON_W 64
#define DESIGN_SUB_BUTTON_H 64
#define DESIGN_SUB_BUTTON_X_GAP 8
#define DESIGN_SUB_BUTTON_Y_GAP 8*/

#define DESIGN_SUB_BUTTONS 16

enum
{
FSP_TEMPLATES_TABS, // player tabs
FSP_TEMPLATES_FILE, // save/load buttons
FSP_TEMPLATES_MAIN, // template buttons

// Maybe no resize? Template panel doesn't really need it.
FSPS_TEMPLATES
};

#define ELEMENTS_PER_TEMPLATE 1
// ELEMENTS_PER_TEMPLATE is the number of elements each template has. Increase it if buttons are added to templates.

enum
{
FPE_TEMPLATES_TAB_P0,
FPE_TEMPLATES_TAB_P1,
FPE_TEMPLATES_TAB_P2,
FPE_TEMPLATES_TAB_P3,
FPE_TEMPLATES_FILE_LOAD,
FPE_TEMPLATES_FILE_SAVE,
FPE_TEMPLATES_TEMPL_0,
/*
Templates will work like this:
- design panel always shows currently selected template
- templates have the following buttons:
 - if loaded: clear template
 - otherwise:

- Actually - maybe don't need buttons on templates at all.
- Put them all in the design panel.
 - *Maybe* just have clear and load/import

*/

// FPEs for other templates are calculated from FPR_TEMPLATES_TEMPL_0, so don't put any new elements at the end of this list (put them before FPE_TEMPLATES_TEMPL_0)
};




enum
{
FSP_EDITOR_WHOLE,

FSP_EDITOR_WINDOW, // text window
FSP_EDITOR_PANEL_RESIZE,

FSP_EDITOR_TABS,
FSP_EDITOR_SUBMENUS,
FSP_EDITOR_SUBMENU_FILE,
FSP_EDITOR_SUBMENU_EDIT,
FSP_EDITOR_SUBMENU_SEARCH,
FSP_EDITOR_SUBMENU_BUILD,

// remember limit of 8 subpanels
FSPS_EDITOR
};

enum
{
FPE_EDITOR_WINDOW,
FPE_EDITOR_WINDOW_SCROLLBAR_V, // may not need these? could just draw it clipped and leave it up to the user to make the window wide enough
FPE_EDITOR_WINDOW_SCROLLBAR_H,
FPE_EDITOR_PANEL_RESIZE,

FPE_EDITOR_TAB_0,
FPE_EDITOR_TAB_1,
FPE_EDITOR_TAB_2,
FPE_EDITOR_TAB_3,
FPE_EDITOR_TAB_4,
FPE_EDITOR_TAB_5,
FPE_EDITOR_TAB_6,
FPE_EDITOR_TAB_7,

FPE_EDITOR_SMB_FILE,
FPE_EDITOR_SMB_EDIT,
FPE_EDITOR_SMB_SEARCH,
FPE_EDITOR_SMB_BUILD,

FPE_EDITOR_FILE_NEW,
FPE_EDITOR_FILE_OPEN,
FPE_EDITOR_FILE_SAVE,
FPE_EDITOR_FILE_SAVE_AS,
FPE_EDITOR_FILE_CLOSE,
FPE_EDITOR_EDIT_UNDO,
FPE_EDITOR_EDIT_REDO,
FPE_EDITOR_EDIT_CUT,
FPE_EDITOR_EDIT_COPY,
FPE_EDITOR_EDIT_PASTE,
FPE_EDITOR_EDIT_CLEAR,
FPE_EDITOR_SEARCH_FIND,
FPE_EDITOR_SEARCH_NEXT,
FPE_EDITOR_BUILD_TEST,

};






#define SUBPANELS 16
// SUBPANELS is the max number of subpanels a panel can have
#define ELEMENTS 64
// ELEMENTS is the max number of elements a panel (not a subpanel) can have

enum // panel element types
{
PE_TYPE_NONE,
PE_TYPE_WINDOW,
PE_TYPE_BUTTON,
//PE_TYPE_SCROLLBAR_EL_V, // vertical scrollbar for an element
//PE_TYPE_SCROLLBAR_EL_H, // horizontal scrollbar for an element
PE_TYPE_SCROLLBAR_EL_V_CHAR, // vertical scrollbar for an element (scrolls char-by-char)
PE_TYPE_SCROLLBAR_EL_H_CHAR, // vertical scrollbar for an element
PE_TYPE_SCROLLBAR_EL_V_PIXEL, // vertical scrollbar for an element (scrolls pixel-by-pixel)
PE_TYPE_SCROLLBAR_EL_H_PIXEL, // horizontal scrollbar for an element
PE_TYPE_DESIGN_WINDOW,
PE_TYPE_DESIGN_DATA, // information about cost, number of members etc
PE_TYPE_EDITOR_WINDOW,

PE_TYPE_PANEL_RESIZE,

//PE_TYPE_
};

#define ELEMENT_VALUES 4

enum
{
ELEMENT_LOCATION_LEFT_TOP,
ELEMENT_LOCATION_LEFT_BOTTOM,
ELEMENT_LOCATION_RIGHT_TOP,
ELEMENT_LOCATION_RIGHT_BOTTOM,
}; // determines where element is placed (in relation to subpanel) if panel resized

enum
{
ELEMENT_FIT_FIXED, // fixed size
ELEMENT_FIT_SUBPANEL_W, // element expands to fit subpanel width
ELEMENT_FIT_SUBPANEL_H,
ELEMENT_FIT_FILL, // elemend expands to fit whole subpanel
ELEMENT_FIT_FILL_WITH_V_SCROLLBAR, // element expands to fit subpanel, leaving space for a vertical scrollbar
ELEMENT_FIT_FILL_WITH_H_SCROLLBAR,
ELEMENT_FIT_FILL_WITH_SCROLLBARS,
ELEMENT_FIT_SUBPANEL_W_WITH_SCROLLBAR, // element expands to fit subpanel width, leaving space for scrollbar
ELEMENT_FIT_SUBPANEL_H_WITH_SCROLLBAR
}; // determines whether element is resized (and how) if subpanel is resized.

enum
{
BUTTON_STYLE_MENU_BIG,
BUTTON_STYLE_TAB,
BUTTON_STYLE_TEMPLATE,
BUTTON_STYLE_DESIGN,
BUTTON_STYLE_DESIGN_SUB,
BUTTON_STYLE_SUBMENU,
BUTTON_STYLE_SUBMENU_LINE,
BUTTON_STYLES
};

struct element_struct
{
	int exists; // if 0, element is unused and can be re-used
	int open; // if 0, element is not being displayed but can't be re-used (if 1 element may not necessarily be visible; depends on subpanel and panel)
	int panel; // which panel
	int subpanel; // which subpanel
	int type; // type of element
	int style; // e.g. button style
	int x1, y1, x2, y2; // these are offsets from the subpanel in which the element is located
	int w, h;
	int location; // ELEMENT_LOCATION_* value
	int offset_x, offset_y; // offset from location (x1, y1, x2, y2 are derived from this)
	int fit;
	timestamp highlight; // last time the element started being highlighted. Reset this if the button is highlighted and last_highlight is longer ago than one tick.
	timestamp last_highlight; // last time the element was highlighted. Reset every tick the element is highlighted.

	int value [ELEMENT_VALUES];
	int* ptr_value;
 char* name; // pointer to a null-terminated string giving the name of the element.
};

struct subpanel_struct
{
	int exists; // if 0, subpanel is unused and can be re-used
	int open; // if 0, subpanel is not being displayed but can't be re-used (if 1 subpanel may not necessarily be visible; depends on panel)
	int panel; // which panel the subpanel is in
	int type;
	int x1, y1, x2, y2; // these are offsets from the panel in which the element is located
	int w, h;
	int first_element; // index of first element associated with this subpanel in the panel's element_struct array
	int clip; // if 1, clipping rectangle should be set when this subpanel is drawn. If 0, don't bother.
	timestamp highlight;
	timestamp last_highlight;

};

struct panel_struct
{
	int open;
	int x1, y1, x2, y2;
	int w, h;
	ALLEGRO_COLOR background_colour;
	struct subpanel_struct subpanel [SUBPANELS];
	struct element_struct element [ELEMENTS];

};

extern struct panel_struct panel [PANELS];


void run_panels(void);
void reset_panel_positions(void);
void reset_mode_buttons(void);
void close_all_panels(void);
void close_panel(int pan, int set_panel_restore); // set_panel_restore will be zero if all panels are being closed at once
void open_panel(int pan);

#endif

