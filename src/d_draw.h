

#ifndef H_D_DRAW
#define H_D_DRAW

void init_design_window(void);
void draw_design_window(void);
void draw_design_data(void);
void reset_design_window(void);


enum
{
SUBTOOLS_MAIN,

SUBTOOLS_SHAPE,
SUBTOOLS_CORE,
//SUBTOOLS_SHAPE5,
//SUBTOOLS_SHAPE6,
SUBTOOLS_EMPTY_LINK,
SUBTOOLS_ACTIVE_LINK,

SUBTOOLS_OBJECTS_LINK,
SUBTOOLS_OBJECTS_STD,
SUBTOOLS_OBJECTS_MOVE,
SUBTOOLS_OBJECTS_ATTACK,
SUBTOOLS_OBJECTS_DEFEND,
SUBTOOLS_OBJECTS_MISC,
SUBTOOLS_OBJECTS_CLEAR,

SUBTOOLS_AUTOCODE,



};


struct design_window_struct
{
 int window_pos_x;
 int window_pos_y;

 struct template_struct* templ;

 int selected_member; // -1 if nothing selected
 int selected_link; // -1 if no vertex selected
// int selected_link_x, selected_link_y; // if vertex selected, this is its position in the design window(set in input function to be used in display)
 int member_rotation_x, member_rotation_y; // if member selected, rotation icon appears here
 timestamp highlight_rotation_time;
 int link_rotation_x, link_rotation_y; // if rotatable object selected, rotation icon appears here
 timestamp select_member_timestamp, select_link_timestamp; // game.total_time timestamp of selection

 int highlight_member; // -1 if nothing highlighted
 int highlight_link; // -1 if no vertex highlighted
 int highlight_link_x, highlight_link_y; // if vertex highlighted, this is its position (set in input function to be used in display)

 int tools_open; // determines help displayed in tool panel. should be an FSP_* value. Anything else will just print nothing.
 int subtools_open; // similar

};

extern struct design_window_struct dwindow;


#define DESIGN_WINDOW_W scaleUI_x(FONT_BASIC,1000)
#define DESIGN_WINDOW_CENTRE_X (DESIGN_WINDOW_W/2)
#define DESIGN_WINDOW_H scaleUI_y(FONT_BASIC,800)
#define DESIGN_WINDOW_CENTRE_Y (DESIGN_WINDOW_H/2)

#endif

