

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>


#include <stdio.h>
#include <string.h>
#include <math.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"

#include "g_misc.h"

#include "c_header.h"

#include "e_header.h"
#include "e_inter.h"
#include "e_help.h"
#include "i_header.h"
#include "i_input.h"
#include "i_view.h"
#include "m_input.h"
#include "e_log.h"
#include "m_maths.h"

#include "p_panels.h"

#include "d_draw.h"
#include "d_design.h"
#include "d_geo.h"
#include "d_code.h"
#include "d_code_header.h"

#include "g_shapes.h"
#include "t_template.h"
#include "c_keywords.h"
#include "h_story.h"

#include "x_sound.h"


extern struct nshape_struct nshape [NSHAPES];
extern struct dshape_struct dshape [NSHAPES]; // uses same indices as NSHAPES
extern struct object_type_struct otype [OBJECT_TYPES];
extern struct fontstruct font [FONTS];

extern struct object_type_struct otype [OBJECT_TYPES];

extern struct dcode_state_struct dcode_state;
extern struct story_struct story;

// remove c_prepr.h when finished testing source loading
//#include "c_prepr.h"

//extern struct fontstruct font [FONTS];

/*

How do do this:
There will be a main design window at the top.
Then a window with information about what's going on in the design window
Then the message log.

How will a user interact with the design window?
 - The window will start with a core process in the middle.
 - There are h and v scrollbars.
 - User can select a subprocess by clicking on it.
 - User can add a new subprocess by clicking on a vertex and adding a link object
 - How does a user add an object or similar?
  - click on vertex
  - an overwindow pops up with a list of possibilities
  - information about the object etc being selected in the overwindow
  - OR: list of possibilities appears in the subpanel below the design window
   - possibly in multiple columns, scrollable L-R?
   - information can be got by right-clicking anything, so a special information part is probably unnecessary.
		 - When you click on a process or object:
		  - the left part of the panel gives you a list of things you can change.
		  - the right part of the panel gives you list of things you can change them to, or similar.
		  - process:
		   - change shape - right part of panel has possible shapes (probably need a h scrollbar for this)
		   - change size
		   - change angle? Or could use right click for this (right click on link object?)
		   - cut/copy/paste?
		   - flip (flips
		   - delete (if not core)
				- object/vertex
				 - add/change type
				 - remove
				 - optimise?
				 - cut/copy/paste?

- can rotate things by dragging them to other part of window where they can't be dropped



*/

/*
struct template_object_struct
{
	int type;
	al_fixed angle;
	int value;
};
typedef struct template_object_struct template_object_struct;

struct template_connection_struct
{
	int template_member_index; // -1 if no connection
	int object_index; // object (vertex) that is connected.
 int reverse_connection_index; // index of connection in other proc's connection structure.
 int reverse_object_index; // index of other proc's link object.
};
typedef struct template_connection_struct template_connection_struct;

struct template_member_struct
{
 int exists;
 int shape;
 int size;
 al_fixed group_angle_offset; // offset from pointing directly upwards.
 al_fixed connection_angle_offset; // offset from the angle the proc would have if two vertices just connected directly
 template_object_struct object [PROC_MAX_OBJECTS]; // index is the vertex the object is on.
 template_connection_struct connection [GROUP_CONNECTIONS];
 cart position; // x/y position - derived from position in group, which needs to be determined recursively.
};

struct template_struct
{

 struct template_member_struct member [GROUP_MAX_MEMBERS]; // member [0] is core

};
typedef struct template_struct template;



*/

//;extern struct design_window_struct dwindow;
extern struct game_struct game;

void init_templ_group_member(struct template_struct* templ, int gm);
void check_mouse_over_dwindow(int mouse_x, int mouse_y, int* mouse_over_member, int* mouse_over_link, int* mouse_over_rotation, int *link_x, int* link_y);
void reset_design_tools_subpanel(int new_subpanel);
void open_subtools(int subtools);
void set_sub_buttons_range(int range_start, int range_end, int special_highlight_value);
static void set_autocode_sub_buttons(void);

void change_member_shape(int mem, int new_shape);
void set_special_highlight_shape(int new_shape);
void set_member_rotation_icon_position(void);
void set_link_rotation_icon_position(void);

void set_member_object(int mem, int obj, int new_obj);
void set_special_highlight_object(void);
void clear_member_object(int mem, int obj, int reset_panel);

int add_linked_member(int parent_mem, int parent_link, int check_results_of_change);
static void clear_member_object_classes(int mem, int obj);
int change_uplink(int child_mem_index, int new_link_index, int check_results_of_change);
int move_uplink(int child_mem_index, int new_parent_index, int new_downlink_index, int check_results_of_change);
static int move_downlink(int old_parent_index, int old_downlink_index, int new_parent_index, int new_downlink_index);

static void copy_nonlink_object(int target_member_index, int target_object_index, int source_member_index, int source_object_index);
void copy_symmetrical_downlink_recursively(int old_parent_member_index, int new_parent_member_index, int object_index, int new_link_index);
int get_linked_member_index(int parent_member_index, int link_object_index);
static void fix_symmetrical_object_angle(int old_member_index, int old_object, int new_member_index, int new_object);

static void up_to_down_design_symmetry(void);
//static int noncore_mirror_object(int member_index, int object_index);
//static int core_mirror_object(int mirror_axis, int object_index);
static int mirror_object_centreline(int member_index, int mirror_axis, int object_index);
static int mirror_object_noncentre(int member_index, int object_index);
static int mirror_process_on_axis_recursively(int member_index);

static void delete_selected_member(void);
static void delete_member_and_submembers(int member_index);
static void remove_design_members_recursively(int mem);
void delete_downlink_object(int parent_member, int object_index);

static void template_locked_design_message(void);

extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];

static void play_design_sound(int note);

// This is also called by c_fix when a template is being generated from source code
void init_template_for_design(struct template_struct* init_templ)
{

	clear_template_but_not_source(init_templ);

	init_templ_group_member(init_templ, 0);

 init_templ->member[0].approximate_angle_offset = 0;
 init_templ->member[0].approximate_distance = 0;


}


void init_templ_group_member(struct template_struct* group_templ, int gm)
{
	group_templ->member[gm].exists = 1;
	group_templ->member[gm].shape = NSHAPE_CORE_QUAD_A;
	group_templ->member[gm].group_angle_offset = 0;
	group_templ->member[gm].connection_angle_offset = 0;
	group_templ->member[gm].connection_angle_offset_angle = 0;
	group_templ->member[gm].collision = 0;
	group_templ->member[gm].move_obstruction = 0;
	group_templ->member[gm].story_lock_failure = 0;
	int i, j;
/*	for (i = 0; i < PROC_MAX_OBJECTS; i++)
	{
	 group_templ->member[gm].object[i].type = 0;
	}*/
	for (i = 0; i < GROUP_CONNECTIONS; i ++)
	{
		group_templ->member[gm].connection[i].template_member_index = -1;
	}
	for (i = 0; i < MAX_LINKS; i ++)
	{
		group_templ->member[gm].object[i].type = OBJECT_TYPE_NONE;
		group_templ->member[gm].object[i].base_angle_offset = 0;
		group_templ->member[gm].object[i].base_angle_offset_angle = 0;
		group_templ->member[gm].object[i].template_error = TEMPLATE_OBJECT_ERROR_NONE;
	 for (j = 0; j < CLASSES_PER_OBJECT; j ++)
		{
			group_templ->member[gm].object[i].object_class[j] = -1;
		}
	}

}

void open_template_in_designer(struct template_struct* tpl)
{

	dwindow.templ = tpl;//&templ [p] [t];

	reset_design_window();

	if (dwindow.templ->active == 1)
  reset_design_tools_subpanel(FSP_DESIGN_TOOLS_MAIN);
   else
    reset_design_tools_subpanel(FSP_DESIGN_TOOLS_EMPTY);


}



// This function is called when the mouse is in the design window element.
void design_window_input(int mouse_x, int mouse_y)
{

 if (dwindow.templ->active == 0)
		return;

// first, adjust mouse_x/y for window position (they have already been adjusted for the element position):
 mouse_x += dwindow.window_pos_x;
 mouse_y += dwindow.window_pos_y;

 int mouse_over_member = -1;
 int mouse_over_link = -1;
 int mouse_over_rotation = 0;

 check_mouse_over_dwindow(mouse_x, mouse_y, &mouse_over_member, &mouse_over_link, &mouse_over_rotation, &dwindow.highlight_link_x, &dwindow.highlight_link_y);

 dwindow.highlight_member = mouse_over_member;
 dwindow.highlight_link = mouse_over_link;

 if (control.mbutton_press [0] == BUTTON_JUST_PRESSED) // just pressed left mouse button
	{
		if (mouse_over_rotation) // should be able to assume that dwindow.selected_member is valid
		{
			if (dwindow.selected_link == -1)
			{
		  if (dwindow.selected_member == 0)
		   reset_design_tools_subpanel(FSP_DESIGN_TOOLS_CORE);
		    else
		     reset_design_tools_subpanel(FSP_DESIGN_TOOLS_MEMBER);
			 control.mouse_drag = MOUSE_DRAG_DESIGN_MEMBER;
    control.mouse_drag_panel = PANEL_DESIGN;
    control.mouse_drag_element = FPE_DESIGN_WINDOW;
    set_member_rotation_icon_position();
    play_design_sound(TONE_3C);
			}
			 else
				{
			  control.mouse_drag = MOUSE_DRAG_DESIGN_OBJECT;
     control.mouse_drag_panel = PANEL_DESIGN;
     control.mouse_drag_element = FPE_DESIGN_WINDOW;
     set_link_rotation_icon_position();
     play_design_sound(TONE_3C);
				}

		}
		 else
		if (mouse_over_member != -1)
		{
			if (dwindow.selected_member != mouse_over_member)
			 dwindow.select_member_timestamp = inter.running_time;
			dwindow.selected_member = mouse_over_member;
//			control.mouse_drag = MOUSE_DRAG_DESIGN_MEMBER;
   control.mouse_drag_panel = PANEL_DESIGN;
   control.mouse_drag_element = FPE_DESIGN_WINDOW;
   set_member_rotation_icon_position();
   play_design_sound(TONE_3C);
			if (mouse_over_link == -1)
			{
				start_log_line(MLOG_COL_TEMPLATE);
				write_to_log("Selected component ");
				write_number_to_log(mouse_over_member); // - this member index can be misleading because it may be different from the index in the process header
				write_to_log(".");
				finish_log_line();
// component number may not actually be useful as I think the number in the template can differ from the number
//  in the process header (if the component has been added in the designed and not yet compiled from the header)
// oh well.
			 dwindow.selected_link = -1;
			 if (dwindow.selected_member == 0)
			  reset_design_tools_subpanel(FSP_DESIGN_TOOLS_CORE);
			   else
			    reset_design_tools_subpanel(FSP_DESIGN_TOOLS_MEMBER);
			}
			 else
				{
				 start_log_line(MLOG_COL_TEMPLATE);
//				 write_to_log("Selected component ");
//				 write_number_to_log(mouse_over_member); - this member index is misleading because it may be different from the index in the process header
				 write_to_log("Selected component ");
				 write_number_to_log(mouse_over_member); // - this member index can be misleading because it may be different from the index in the process header
				 write_to_log(" link ");
				 write_number_to_log(mouse_over_link);
				 write_to_log(".");
				 finish_log_line();
				 dwindow.selected_link = mouse_over_link;

/*				 int k;

				 int conn_index = -1;

				 for (k = 0; k < GROUP_CONNECTIONS; k ++)
					{
						if (dwindow.templ->member[dwindow.selected_member].connection[k].link_index == mouse_over_link)
						{
							conn_index = k;
							break;
						}
					}*/

 			 control.mouse_drag = MOUSE_DRAG_DESIGN_OBJECT_MOVE;
     control.mouse_drag_panel = PANEL_DESIGN;
     control.mouse_drag_element = FPE_DESIGN_WINDOW;
				 set_link_rotation_icon_position();
				 dwindow.select_link_timestamp = inter.running_time;
//  			control.mouse_drag = MOUSE_DRAG_DESIGN_OBJECT;
 			 reset_design_tools_subpanel(FSP_DESIGN_TOOLS_EMPTY_LINK);
				}
		}
		 else
			{
				dwindow.selected_member = -1;
				dwindow.selected_link = -1;
			 reset_design_tools_subpanel(FSP_DESIGN_TOOLS_MAIN);
    play_design_sound(TONE_2A);
			}
	}


 if (control.mbutton_press [0] == BUTTON_JUST_RELEASED) // just let left mouse button go
	{
// currently all this can do is move or copy objects:
  if (control.mouse_drag == MOUSE_DRAG_DESIGN_OBJECT_MOVE)
		{
   control.mouse_drag = MOUSE_DRAG_NONE;
// We can hopefully assume that selected member etc are still valid
//  Is there any way that this could be false?
//    ! check whether it's possible for the design to be re-generated from source - e.g. if something tries to build it and it auto-compiles
//      - and whether this would be a problem
    if (mouse_over_member != -1
					&& mouse_over_link != -1
					&& dwindow.selected_member != -1 // just to make sure
					&& dwindow.selected_link != -1 // just to make sure
					&& (dwindow.selected_member != mouse_over_member
						|| dwindow.selected_link != mouse_over_link))
				{
					if (dwindow.templ->locked)
					{
					 template_locked_design_message();
					 goto finished_button_just_released;
					}
					if (dwindow.templ->member[mouse_over_member].object[mouse_over_link].type == OBJECT_TYPE_UPLINK
					 || dwindow.templ->member[mouse_over_member].object[mouse_over_link].type == OBJECT_TYPE_DOWNLINK)
					{
  		  write_line_to_log("Can't overwrite a link object.", MLOG_COL_TEMPLATE);
						goto finished_button_just_released;
					}
// but may be able to move a link object:
					if (dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type == OBJECT_TYPE_UPLINK)
					{
						if (dwindow.selected_member == mouse_over_member)
							change_uplink(mouse_over_member, mouse_over_link, 1); // just move the uplink around the process
							 else
         move_uplink(dwindow.selected_member, mouse_over_member, mouse_over_link, 1); // move the uplink to a different process
						goto finished_button_just_released;
					}
					if (dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type == OBJECT_TYPE_DOWNLINK)
					{
      move_downlink(dwindow.selected_member, dwindow.selected_link, mouse_over_member, mouse_over_link);
						goto finished_button_just_released;
					}
// it should be possible to delete an object by dragging an empty link onto it
// ... I really should implement at least a keyboard shortcut for delete to deal with this
					set_member_object(mouse_over_member, mouse_over_link, dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
// delete old object as well, unless user is holding shift:
		  	if (ex_control.special_key_press [SPECIAL_KEY_SHIFT] <= 0)
		  	{
						clear_member_object(dwindow.selected_member, dwindow.selected_link, 1);
						calculate_template_cost_and_power(dwindow.templ);
		  	}



				}
		}
	}

finished_button_just_released:

 if (control.mouse_drag == MOUSE_DRAG_DESIGN_OBJECT_MOVE)
	{
  if (ex_control.special_key_press [SPECIAL_KEY_SHIFT] > 0
			&& (dwindow.selected_member	!= -1
			 && dwindow.selected_link	!= -1 // these two tests probably unnecessary but can't hurt
			 && dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type != OBJECT_TYPE_UPLINK // can't copy a link - only move
			 && dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type != OBJECT_TYPE_DOWNLINK))
   ex_control.mouse_cursor_type = MOUSE_CURSOR_DESIGN_DRAG_OBJECT_COPY;
    else
     ex_control.mouse_cursor_type = MOUSE_CURSOR_DESIGN_DRAG_OBJECT;
	}

	return;

}


// returns 1 if still dragging, 0 otherwise
int	mouse_drag_design_member(int mouse_x, int mouse_y, int ctrl_pressed)
{

 if (dwindow.selected_member == -1 // not sure this can happen but check anyway
		|| dwindow.templ->locked) // this can definitely happen
		return 0;

//	if (ex_control.special_key_press [SPECIAL_KEY_SHIFT] <= 0)
//	 && control.key_press [KEY_RSHIFT] <= 0)
//	 return 1; // doesn't interrupt dragging

	int i;

 mouse_x += dwindow.window_pos_x;
 mouse_y += dwindow.window_pos_y;

 int select_member_x = al_fixtoi(dwindow.templ->member[dwindow.selected_member].position.x) + (DESIGN_WINDOW_W/2);// - dwindow.window_pos_x;
 int select_member_y = al_fixtoi(dwindow.templ->member[dwindow.selected_member].position.y) + (DESIGN_WINDOW_H/2);// - dwindow.window_pos_y;

 int parent_member_index = 0; // not strictly necessary but avoids a compiler warning about being used uninitialised (which I'm pretty sure can't actually happen)
 int parent_x;
 int parent_y;

   struct nshape_struct* parent_nshape = NULL;// = &nshape[dwindow.templ->member[dwindow.templ->member[dwindow.selected_member].connection[0].template_member_index].shape];
   struct nshape_struct* child_nshape = &nshape[dwindow.templ->member[dwindow.selected_member].shape];


	if (dwindow.selected_member == 0) // core
	{
		parent_x = select_member_x;
		parent_y = select_member_y;
	}
	 else
		{

			parent_member_index = dwindow.templ->member[dwindow.selected_member].connection[0].template_member_index;

		 parent_x = select_member_x;
		 parent_y = select_member_y;

   parent_nshape = &nshape[dwindow.templ->member[parent_member_index].shape];


   parent_x = al_fixtoi(dwindow.templ->member[parent_member_index].position.x)
                 + al_fixtoi((symmetrical_cos(dwindow.templ->member[parent_member_index].group_angle_offset
																		+ parent_nshape->link_angle_fixed [dwindow.templ->member[dwindow.selected_member].connection[0].reverse_link_index])
																		* (parent_nshape->link_dist_pixel [dwindow.templ->member[dwindow.selected_member].connection[0].reverse_link_index])))
              + (DESIGN_WINDOW_W/2);
   parent_y = al_fixtoi(dwindow.templ->member[parent_member_index].position.y)
                 + al_fixtoi((symmetrical_sin(dwindow.templ->member[parent_member_index].group_angle_offset
																		+ parent_nshape->link_angle_fixed [dwindow.templ->member[dwindow.selected_member].connection[0].reverse_link_index])
																		* (parent_nshape->link_dist_pixel [dwindow.templ->member[dwindow.selected_member].connection[0].reverse_link_index])))
              + (DESIGN_WINDOW_H/2);

		}

 if (abs(mouse_x - select_member_x) > 10
		|| abs(mouse_y - select_member_y) > 10)
	{

  float mouse_angle = atan2(mouse_y - parent_y, mouse_x - parent_x); // float okay here because it will be converted to fixed before ever being used in the game itself

  al_fixed new_angle = (int_angle_to_fixed(radians_to_angle(mouse_angle))) & AFX_MASK; // need to add radians_to_fixed_angle()

		if (dwindow.selected_member == 0) // core
		{

			if (ctrl_pressed)
			{
				int restricted_angle = fixed_angle_to_int(new_angle);
				int closest_mirror_axis = -1;
				int closest_axis_difference = 10000;
				int current_angle_difference;
				for (i = 0; i < nshape[dwindow.templ->member[dwindow.selected_member].shape].mirror_axes; i ++)
				{
					current_angle_difference = angle_difference_int(restricted_angle, nshape[dwindow.templ->member[dwindow.selected_member].shape].mirror_axis_angle [i]);

					if (current_angle_difference < closest_axis_difference)
					{
						closest_axis_difference = current_angle_difference;
						closest_mirror_axis = i;
					}
				}
				dwindow.templ->member[dwindow.selected_member].connection_angle_offset_angle = nshape[dwindow.templ->member[dwindow.selected_member].shape].mirror_axis_angle [closest_mirror_axis];
			}
			 else
     dwindow.templ->member[dwindow.selected_member].connection_angle_offset_angle = fixed_angle_to_int(new_angle);

   dwindow.templ->member[dwindow.selected_member].group_angle_offset = int_angle_to_fixed(dwindow.templ->member[dwindow.selected_member].connection_angle_offset_angle);
   dwindow.templ->member[dwindow.selected_member].connection_angle_offset = dwindow.templ->member[dwindow.selected_member].group_angle_offset;

   for (i = 1; i < GROUP_CONNECTIONS; i ++) // note i begins at 1
	  {
		  if (dwindow.templ->member[dwindow.selected_member].connection[i].template_member_index != -1)
			  update_design_member_position_recursively(dwindow.templ, dwindow.templ->member[dwindow.selected_member].connection[i].template_member_index);
	  }
		}
   else
//  if (dwindow.selected_member != 0) // i.e. is not the core
		{

// work out base angle (angle of connection from parent if connection_angle_offset is zero
	  al_fixed base_angle = parent_nshape->link_angle_fixed [dwindow.templ->member[dwindow.selected_member].connection[0].reverse_link_index]
	                        + dwindow.templ->member[parent_member_index].group_angle_offset;

   al_fixed new_angle_offset = (new_angle - base_angle);// & AFX_MASK;

   int new_angle_offset_int = angle_difference_signed_int(0, fixed_angle_to_int(new_angle_offset));

 	 if (ctrl_pressed)
	 	{
		 	new_angle_offset_int /= ANGLE_32;
		 	new_angle_offset_int *= ANGLE_32;
//		 	new_angle_offset_int &= ANGLE_MASK;
		 }

   if (new_angle_offset_int < -ANGLE_4)
				new_angle_offset_int = -ANGLE_4;
   if (new_angle_offset_int > ANGLE_4)
				new_angle_offset_int = ANGLE_4;

// Need to convert new angle to integer angle units (so it can be stored in source code form) then back to al_fixed:
   dwindow.templ->member[dwindow.selected_member].connection_angle_offset_angle = new_angle_offset_int;//angle_difference_signed_int(0, new_angle_offset_int);
   dwindow.templ->member[dwindow.selected_member].connection_angle_offset = int_angle_to_fixed(dwindow.templ->member[dwindow.selected_member].connection_angle_offset_angle); //new_angle_offset) & AFX_MASK;

//   dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].angle_offset_angle = angle_difference_signed_int(0, fixed_angle_to_int(new_angle_offset));
//   dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].angle_offset = int_angle_to_fixed(dwindow.templ->member[dwindow.selected_member].object [dwindow.selected_link].angle_offset_angle); //new_angle_offset) & AFX_MASK;

   dwindow.templ->member[parent_member_index].object[dwindow.templ->member[dwindow.selected_member].connection[0].reverse_link_index].base_angle_offset_angle = dwindow.templ->member[dwindow.selected_member].connection_angle_offset_angle;
   dwindow.templ->member[parent_member_index].object[dwindow.templ->member[dwindow.selected_member].connection[0].reverse_link_index].base_angle_offset = int_angle_to_fixed(dwindow.templ->member[parent_member_index].object[dwindow.templ->member[dwindow.selected_member].connection[0].reverse_link_index].base_angle_offset_angle);

	  dwindow.templ->member[dwindow.selected_member].group_angle_offset = dwindow.templ->member[dwindow.selected_member].group_angle_offset
	                                                                           + dwindow.templ->member[dwindow.templ->member[dwindow.selected_member].connection[0].template_member_index].group_angle_offset
	                                                                           + parent_nshape->link_angle_fixed [dwindow.templ->member[dwindow.selected_member].connection[0].reverse_link_index]
	                                                                           + (AFX_ANGLE_2 - child_nshape->link_angle_fixed [dwindow.templ->member[dwindow.selected_member].connection[0].link_index]);

    update_design_member_position_recursively(dwindow.templ, dwindow.selected_member);


		}



	}

 check_template_collisions(dwindow.templ);
 check_move_objects_obstruction(dwindow.templ);
 dwindow.templ->modified = 1;

 set_member_rotation_icon_position();

 return 1;

}




// returns 1 if still dragging, 0 otherwise
int	mouse_drag_design_object(int mouse_x, int mouse_y, int ctrl_pressed)
{

 if (dwindow.selected_member == -1
		|| dwindow.selected_link == -1
//		|| ex_control.special_key_press [SPECIAL_KEY_SHIFT] <= 0
		|| otype[dwindow.templ->member[dwindow.selected_member].object [dwindow.selected_link].type].object_details.only_zero_angle_offset // can't rotate this kind of object
		|| dwindow.templ->locked)
		return 0;

 mouse_x += dwindow.window_pos_x;
 mouse_y += dwindow.window_pos_y;

	struct nshape_struct* nsh = &nshape [dwindow.templ->member[dwindow.selected_member].shape];

 int select_object_x = al_fixtoi(dwindow.templ->member[dwindow.selected_member].position.x + fixed_cos(dwindow.templ->member[dwindow.selected_member].group_angle_offset + nsh->object_angle_fixed [dwindow.selected_link]) * nsh->object_dist_pixel [dwindow.selected_link]) + (DESIGN_WINDOW_W/2);
 int select_object_y = al_fixtoi(dwindow.templ->member[dwindow.selected_member].position.y + fixed_sin(dwindow.templ->member[dwindow.selected_member].group_angle_offset + nsh->object_angle_fixed [dwindow.selected_link]) * nsh->object_dist_pixel [dwindow.selected_link]) + (DESIGN_WINDOW_H/2);

 if (abs(mouse_x - select_object_x) > 15
		|| abs(mouse_y - select_object_y) > 15)
	{

  float mouse_angle = atan2(mouse_y - select_object_y, mouse_x - select_object_x); // float okay here because it will be converted to fixed before ever being used in the game itself

		int new_angle_int = radians_to_angle(mouse_angle);

	 if (ctrl_pressed)
		{
			new_angle_int = (new_angle_int + ANGLE_1) / ANGLE_16;
			new_angle_int *= ANGLE_16;
			new_angle_int &= ANGLE_MASK;
		}

  al_fixed new_angle = (int_angle_to_fixed(new_angle_int)) & AFX_MASK;

// work out base angle (angle of connection from parent if connection_angle_offset is zero
	  al_fixed base_angle = (nsh->object_angle_fixed [dwindow.selected_link] + dwindow.templ->member[dwindow.selected_member].group_angle_offset) & AFX_MASK;

   al_fixed new_angle_offset = angle_difference_signed(base_angle, new_angle); //(new_angle - base_angle) & AFX_MASK;


// Need to convert new angle to integer angle units (so it can be stored in source code form) then back to al_fixed:
   dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].base_angle_offset_angle = angle_difference_signed_int(0, fixed_angle_to_int(new_angle_offset));

   if (dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].base_angle_offset_angle > ANGLE_4)
				dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].base_angle_offset_angle = ANGLE_4;
   if (dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].base_angle_offset_angle < -ANGLE_4)
				dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].base_angle_offset_angle = -ANGLE_4;

   dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].base_angle_offset = int_angle_to_fixed(dwindow.templ->member[dwindow.selected_member].object [dwindow.selected_link].base_angle_offset_angle); //new_angle_offset) & AFX_MASK;

   dwindow.templ->modified = 1;

	}

	if (dwindow.templ->member[dwindow.selected_member].object [dwindow.selected_link].type == OBJECT_TYPE_MOVE)
		check_move_objects_obstruction(dwindow.templ); // unfortunately calling the single move object obstruction function for just this object doesn't reset the member move_obstruction values (because they might be obstructing a different object)

 set_link_rotation_icon_position();
//	check_template_collisions();

 return 1;

}

/*
struct object_details_struct
{
 int packet_speed; // a default value is used for this for non-attacking objects (so that the intercept method won't fail if called with them)
 int attack_type;
 int power_cost;
 int recycle_time;
 int packet_size;
 int damage;
 int rotate_speed; // pulse etc. object turn rate when aiming (this is actually turned into a fixed value)
};

struct object_type_struct
{
	char name [OBJECT_NAME_LENGTH];
	int keyword_index; // index of the identifier for the object's name
	int object_base_type;
	int data_cost;
 struct object_details_struct object_details; // this is a separate struct because it's mostly irrelevant for non-attacking objects
*/

/*

New approach to power:

- core will determine base power capacity
 - and also power capacity per component

Will go from about 40 + 5 to about 80 + 30


- Only core will determine stress capacity
 - values similar to present

Also:
 - excess stress will cause process to stop working, not explode
  - probably for 10 seconds or something


OR

- so that I can just release this game sometime, remove the whole power/stress system.
- instead just have power as a limit on the objects that can be put on a process
 - have capacity determined by core and components as above.
- Things that otherwise rely on power, like interface recharging, will just work automatically.

*/




enum
{
DSBTYPE_SHAPE,
DSBTYPE_CORE_SHAPE,
DSBTYPE_OBJECT,
DSBTYPE_ACTION,
DSBTYPE_AUTOCODE_TYPE
};

#define DSB_NAME_LENGTH 18

char shape_name [31] [DSB_NAME_LENGTH] =
{
	"core_quad_A",
	"core_quad_B",
	"core_pent_A",
	"core_pent_B",
	"core_pent_C",
	"core_hex_A",
	"core_hex_B",
	"core_hex_C",
	"core_static_quad",
	"core_static_pent",
	"core_static_hex_A",
	"core_static_hex_B",
	"core_static_hex_C",
	"component_tri",
	"component_fork",
	"component_box",
	"component_long4",
	"component_cap",
	"component_prong",
	"component_long5",
	"component_peak",
	"component_snub",
	"component_bowl",
	"component_long6",
	"component_drop",
	"component_side",


	"hexagon",
	"hexagon",
	"hexagon",
	"hexagon",
	"hexagon",
// TO DO: put these names into shape_dat struct and use for everything
};

struct design_sub_button_struct design_sub_button [DSB_STRUCT_SIZE] =
{

	{DSBTYPE_CORE_SHAPE, NSHAPE_CORE_QUAD_A, shape_name [0]}, // DSB_SHAPE_CORE_QUAD_A
	{DSBTYPE_CORE_SHAPE, NSHAPE_CORE_QUAD_B, shape_name [1]}, // DSB_SHAPE_CORE_QUAD_B
	{DSBTYPE_CORE_SHAPE, NSHAPE_CORE_PENT_A, shape_name [2]}, // DSB_SHAPE_CORE_PENT_A
	{DSBTYPE_CORE_SHAPE, NSHAPE_CORE_PENT_B, shape_name [3]}, // DSB_SHAPE_CORE_PENT_B
	{DSBTYPE_CORE_SHAPE, NSHAPE_CORE_PENT_C, shape_name [4]}, // DSB_SHAPE_CORE_PENT_C
	{DSBTYPE_CORE_SHAPE, NSHAPE_CORE_HEX_A, shape_name [5]}, // DSB_SHAPE_CORE_HEX_A,
	{DSBTYPE_CORE_SHAPE, NSHAPE_CORE_HEX_B, shape_name [6]}, // DSB_SHAPE_CORE_HEX_B,
	{DSBTYPE_CORE_SHAPE, NSHAPE_CORE_HEX_C, shape_name [7]}, // DSB_SHAPE_CORE_HEX_C,
	{DSBTYPE_CORE_SHAPE, NSHAPE_CORE_STATIC_QUAD, shape_name [8]}, // DSB_SHAPE_CORE_STATIC_QUAD,
	{DSBTYPE_CORE_SHAPE, NSHAPE_CORE_STATIC_PENT, shape_name [9]}, // DSB_SHAPE_CORE_STATIC_PENT,
	{DSBTYPE_CORE_SHAPE, NSHAPE_CORE_STATIC_HEX_A, shape_name [10]}, // DSB_SHAPE_CORE_STATIC_HEX_A,
	{DSBTYPE_CORE_SHAPE, NSHAPE_CORE_STATIC_HEX_B, shape_name [11]}, // DSB_SHAPE_CORE_STATIC_HEX_B,
	{DSBTYPE_CORE_SHAPE, NSHAPE_CORE_STATIC_HEX_C, shape_name [12]}, // DSB_SHAPE_CORE_STATIC_HEX_C,

	{DSBTYPE_SHAPE, NSHAPE_COMPONENT_TRI, shape_name [13]}, // DSB_SHAPE_COMPONENT_TRI,
	{DSBTYPE_SHAPE, NSHAPE_COMPONENT_FORK, shape_name [14]}, // DSB_SHAPE_COMPONENT_FORK,
	{DSBTYPE_SHAPE, NSHAPE_COMPONENT_BOX, shape_name [15]}, // DSB_SHAPE_COMPONENT_BOX,
	{DSBTYPE_SHAPE, NSHAPE_COMPONENT_LONG4, shape_name [16]}, // DSB_SHAPE_COMPONENT_LONG4,
	{DSBTYPE_SHAPE, NSHAPE_COMPONENT_CAP, shape_name [17]}, // DSB_SHAPE_COMPONENT_CAP,
	{DSBTYPE_SHAPE, NSHAPE_COMPONENT_PRONG, shape_name [18]}, // DSB_SHAPE_COMPONENT_PRONG,
	{DSBTYPE_SHAPE, NSHAPE_COMPONENT_LONG5, shape_name [19]}, // DSB_SHAPE_COMPONENT_LONG5,
	{DSBTYPE_SHAPE, NSHAPE_COMPONENT_PEAK, shape_name [20]}, // DSB_SHAPE_COMPONENT_PEAK,
	{DSBTYPE_SHAPE, NSHAPE_COMPONENT_SNUB, shape_name [21]}, // DSB_SHAPE_COMPONENT_SNUB,
	{DSBTYPE_SHAPE, NSHAPE_COMPONENT_BOWL, shape_name [22]}, // DSB_SHAPE_COMPONENT_BOWL,
	{DSBTYPE_SHAPE, NSHAPE_COMPONENT_LONG6, shape_name [23]}, // DSB_SHAPE_COMPONENT_LONG6,
	{DSBTYPE_SHAPE, NSHAPE_COMPONENT_DROP, shape_name [24]}, // DSB_SHAPE_COMPONENT_DROP,
	{DSBTYPE_SHAPE, NSHAPE_COMPONENT_SIDE, shape_name [25]}, // DSB_SHAPE_COMPONENT_SIDE,


//	{DSBTYPE_SHAPE, NSHAPE_WARP, shape_name [1]}, // DSB_SHAPE4_DIAMOND,
/*	{DSBTYPE_SHAPE, SHAPE_4POINTY, shape_name [2]}, // DSB_SHAPE4_POINTY,
	{DSBTYPE_SHAPE, SHAPE_4TRAP, "trapezoid"}, // DSB_SHAPE4_TRAP,
	{DSBTYPE_SHAPE, SHAPE_4IRREG_L, "irreg-l"}, // DSB_SHAPE4_IRREG_L,
	{DSBTYPE_SHAPE, SHAPE_4IRREG_R, "irreg-r"}, // DSB_SHAPE4_IRREG_R,
	{DSBTYPE_SHAPE, SHAPE_4ARROW, "arrow"}, // DSB_SHAPE4_ARROW,* /

	{DSBTYPE_SHAPE, SHAPE_5PENTAGON, "pentagon"}, // DSB_SHAPE5_PENTAGON,
	{DSBTYPE_SHAPE, SHAPE_5POINTY, "pointy"}, // DSB_SHAPE5_POINTY,
	{DSBTYPE_SHAPE, SHAPE_5LONG, "long"}, // DSB_SHAPE5_LONG,
	{DSBTYPE_SHAPE, SHAPE_5WIDE, "wide"}, // DSB_SHAPE5_WIDE,

	{DSBTYPE_SHAPE, SHAPE_6HEXAGON, "hexagon"}, // DSB_SHAPE6_HEXAGON,
	{DSBTYPE_SHAPE, SHAPE_6POINTY, ""}, // DSB_SHAPE6_POINTY,
	{DSBTYPE_SHAPE, SHAPE_6LONG, ""}, // DSB_SHAPE6_LONG,
	{DSBTYPE_SHAPE, SHAPE_6IRREG_L, ""}, // DSB_SHAPE6_IRREG_L,
	{DSBTYPE_SHAPE, SHAPE_6IRREG_R, ""}, // DSB_SHAPE6_IRREG_R,
	{DSBTYPE_SHAPE, SHAPE_6ARROW, ""}, // DSB_SHAPE6_ARROW,
	{DSBTYPE_SHAPE, SHAPE_6STAR, ""}, // DSB_SHAPE6_STAR,*/

	{DSBTYPE_OBJECT, OBJECT_TYPE_UPLINK, "uplink"}, // DSB_OBJECT_LINK,
	{DSBTYPE_OBJECT, OBJECT_TYPE_DOWNLINK, "downlink"}, // DSB_OBJECT_LINK,

	{DSBTYPE_OBJECT, OBJECT_TYPE_MOVE, "move"}, // DSB_OBJECT_MOVE_MOVE,

	{DSBTYPE_OBJECT, OBJECT_TYPE_PULSE, "pulse"}, // DSB_OBJECT_ATTACK_PULSE,
	{DSBTYPE_OBJECT, OBJECT_TYPE_PULSE_L, "pulse_l"}, // DSB_OBJECT_ATTACK_PULSE_L,
	{DSBTYPE_OBJECT, OBJECT_TYPE_PULSE_XL, "pulse_xl"}, // DSB_OBJECT_ATTACK_PULSE_XL,
	{DSBTYPE_OBJECT, OBJECT_TYPE_BURST, "burst"}, // DSB_OBJECT_ATTACK_BURST,
	{DSBTYPE_OBJECT, OBJECT_TYPE_BURST_L, "burst_l"}, // DSB_OBJECT_ATTACK_BURST_L,
	{DSBTYPE_OBJECT, OBJECT_TYPE_BURST_XL, "burst_xl"}, // DSB_OBJECT_ATTACK_BURST_XL,
	{DSBTYPE_OBJECT, OBJECT_TYPE_STREAM, "stream"}, // DSB_OBJECT_ATTACK_STREAM,
	{DSBTYPE_OBJECT, OBJECT_TYPE_STREAM_DIR, "stream_dir"}, // DSB_OBJECT_ATTACK_STREAM_DIR,
	{DSBTYPE_OBJECT, OBJECT_TYPE_SPIKE, "spike"}, // DSB_OBJECT_ATTACK_SPIKE,
	{DSBTYPE_OBJECT, OBJECT_TYPE_ULTRA, "ultra"}, // DSB_OBJECT_ATTACK_ULTRA,
	{DSBTYPE_OBJECT, OBJECT_TYPE_ULTRA_DIR, "ultra_dir"}, // DSB_OBJECT_ATTACK_ULTRA_DIR,
	{DSBTYPE_OBJECT, OBJECT_TYPE_SLICE, "slice"}, // DSB_OBJECT_ATTACK_SLICE,
//	{DSBTYPE_OBJECT, OBJECT_TYPE_SLICE_DIR, "slice_dir"}, // DSB_OBJECT_ATTACK_SLICE_DIR,

	{DSBTYPE_OBJECT, OBJECT_TYPE_BUILD, "build"}, // DSB_OBJECT_STD_BUILD,
	{DSBTYPE_OBJECT, OBJECT_TYPE_HARVEST, "harvest"}, // DSB_OBJECT_STD_HARVEST
	{DSBTYPE_OBJECT, OBJECT_TYPE_STORAGE, "storage"}, // DSB_OBJECT_STD_STORAGE,
	{DSBTYPE_OBJECT, OBJECT_TYPE_ALLOCATE, "allocate"}, // DSB_OBJECT_STD_ALLOCATE,

	{DSBTYPE_OBJECT, OBJECT_TYPE_INTERFACE, "interface"}, // DSB_OBJECT_DEFEND_INTERFACE,
//	{DSBTYPE_OBJECT, OBJECT_TYPE_INTERFACE_DEPTH, "interface_depth"}, // DSB_OBJECT_DEFEND_INTERFACE_DEPTH,
//	{DSBTYPE_OBJECT, OBJECT_TYPE_INTERFACE_STABILITY, "interface_sta"}, // DSB_OBJECT_DEFEND_INTERFACE_STABILITY,
//	{DSBTYPE_OBJECT, OBJECT_TYPE_INTERFACE_RESPONSE, "interface_res"}, // DSB_OBJECT_DEFEND_INTERFACE_RESPONSE,
	{DSBTYPE_OBJECT, OBJECT_TYPE_REPAIR, "repair"}, // DSB_OBJECT_DEFEND_REPAIR,
	{DSBTYPE_OBJECT, OBJECT_TYPE_REPAIR_OTHER, "repair_other"}, // DSB_OBJECT_DEFEND_REPAIR_OTHER,
	{DSBTYPE_OBJECT, OBJECT_TYPE_STABILITY, "stability"}, // DSB_OBJECT_DEFEND_STABILITY,

	{DSBTYPE_AUTOCODE_TYPE, AUTOCODE_NONE, "no attack"}, // DSB_AUTOCODE_NONE,
	{DSBTYPE_AUTOCODE_TYPE, AUTOCODE_STANDARD, "basic attack"}, // DSB_AUTOCODE_STANDARD,
	{DSBTYPE_AUTOCODE_TYPE, AUTOCODE_CHARGE, "charge"}, // DSB_AUTOCODE_CHARGE,
	{DSBTYPE_AUTOCODE_TYPE, AUTOCODE_BOMBARD, "bombard"}, // DSB_AUTOCODE_BOMBARD,
	{DSBTYPE_AUTOCODE_TYPE, AUTOCODE_CIRCLE_CW, "circle cw"}, // DSB_AUTOCODE_CIRCLE_CW,
	{DSBTYPE_AUTOCODE_TYPE, AUTOCODE_CIRCLE_ACW, "circle acw"}, // DSB_AUTOCODE_CIRCLE_ACW,
	{DSBTYPE_AUTOCODE_TYPE, AUTOCODE_ERRATIC, "erratic"}, // DSB_AUTOCODE_ERRATIC,
	{DSBTYPE_AUTOCODE_TYPE, AUTOCODE_CAUTIOUS, "cautious"}, // DSB_AUTOCODE_CAUTIOUS,

	{DSBTYPE_ACTION, 0, "confirm"}, // DSB_ACTION_DELETE,

/*
	{DSBTYPE_SHAPE, SHAPE_4SQUARE, "square"}, // DSB_SHAPE4_SQUARE,
	{DSBTYPE_SHAPE, SHAPE_4DIAMOND, "diamond"}, // DSB_SHAPE4_DIAMOND,
	{DSBTYPE_SHAPE, SHAPE_4POINTY, "pointy"}, // DSB_SHAPE4_POINTY,
	{DSBTYPE_SHAPE, SHAPE_4TRAP, "trapezoid"}, // DSB_SHAPE4_TRAP,
	{DSBTYPE_SHAPE, SHAPE_4IRREG_L, "irreg-l"}, // DSB_SHAPE4_IRREG_L,
	{DSBTYPE_SHAPE, SHAPE_4IRREG_R, "irreg-r"}, // DSB_SHAPE4_IRREG_R,
	{DSBTYPE_SHAPE, SHAPE_4ARROW, "arrow"}, // DSB_SHAPE4_ARROW,

	{DSBTYPE_SHAPE, SHAPE_5PENTAGON, "pentagon"}, // DSB_SHAPE5_PENTAGON,
	{DSBTYPE_SHAPE, SHAPE_5POINTY, "pointy"}, // DSB_SHAPE5_POINTY,
	{DSBTYPE_SHAPE, SHAPE_5LONG, "long"}, // DSB_SHAPE5_LONG,
	{DSBTYPE_SHAPE, SHAPE_5WIDE, "wide"}, // DSB_SHAPE5_WIDE,

	{DSBTYPE_SHAPE, SHAPE_6HEXAGON, "hexagon"}, // DSB_SHAPE6_HEXAGON,
	{DSBTYPE_SHAPE, SHAPE_6POINTY, ""}, // DSB_SHAPE6_POINTY,
	{DSBTYPE_SHAPE, SHAPE_6LONG, ""}, // DSB_SHAPE6_LONG,
	{DSBTYPE_SHAPE, SHAPE_6IRREG_L, ""}, // DSB_SHAPE6_IRREG_L,
	{DSBTYPE_SHAPE, SHAPE_6IRREG_R, ""}, // DSB_SHAPE6_IRREG_R,
	{DSBTYPE_SHAPE, SHAPE_6ARROW, ""}, // DSB_SHAPE6_ARROW,
	{DSBTYPE_SHAPE, SHAPE_6STAR, ""}, // DSB_SHAPE6_STAR,
*/
};


void reset_design_tools_subpanel(int new_subpanel)
{


// first close all:
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_EMPTY].open = 0;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].open = 0;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MEMBER].open = 0;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_CORE].open = 0;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_EMPTY_LINK].open = 0;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_ACTIVE_LINK].open = 0;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_DELETE].open = 0;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_AUTOCODE].open = 0;
// and subtools:
// panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS_MAIN].open = 0;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS].open = 0;
// panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS_VERTEX_OBJECT].open = 0;

 dwindow.tools_open = new_subpanel;

	switch(new_subpanel)
	{
  case FSP_DESIGN_TOOLS_EMPTY:
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_EMPTY].open = 1;
  	break;
	 case FSP_DESIGN_TOOLS_MAIN:
// check for main already being open here
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].open = 1;
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS].open = 1;
   open_subtools(SUBTOOLS_MAIN);
 		break;
 	case FSP_DESIGN_TOOLS_MEMBER:
 		// note that this subpanel could already be open, but it could be for a different member
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MEMBER].open = 1;
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS].open = 1;
   open_subtools(SUBTOOLS_SHAPE);// + (shape_dat [dwindow.templ->member[dwindow.selected_member].shape] [0].vertices - 4));
   set_special_highlight_shape(dwindow.templ->member[dwindow.selected_member].shape);
			break;
 	case FSP_DESIGN_TOOLS_CORE:
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_CORE].open = 1;
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS].open = 1;
   open_subtools(SUBTOOLS_CORE);// + (shape_dat [dwindow.templ->member[dwindow.selected_member].shape] [0].vertices - 4));
   set_special_highlight_shape(dwindow.templ->member[dwindow.selected_member].shape);
			break;
 	case FSP_DESIGN_TOOLS_EMPTY_LINK:
 	case FSP_DESIGN_TOOLS_ACTIVE_LINK:
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_EMPTY_LINK].open = 1;
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS].open = 1;
   open_subtools(SUBTOOLS_EMPTY_LINK);
   switch (otype[dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type].object_base_type)
   {
   	case OBJECT_BASE_TYPE_LINK:
//     open_subtools(SUBTOOLS_OBJECTS_LINK);
     set_special_highlight_object(); // dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
     break;
   	case OBJECT_BASE_TYPE_MOVE:
     open_subtools(SUBTOOLS_OBJECTS_MOVE);
     set_special_highlight_object(); // dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
     break;
   	case OBJECT_BASE_TYPE_STD:
     open_subtools(SUBTOOLS_OBJECTS_STD);
     set_special_highlight_object(); // dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
     break;
   	case OBJECT_BASE_TYPE_ATTACK:
     open_subtools(SUBTOOLS_OBJECTS_ATTACK);
     set_special_highlight_object(); // dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
     break;
   	case OBJECT_BASE_TYPE_DEFEND:
     open_subtools(SUBTOOLS_OBJECTS_DEFEND);
     set_special_highlight_object(); // dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
     break;
   	case OBJECT_BASE_TYPE_MISC:
     open_subtools(SUBTOOLS_OBJECTS_MISC);
     set_special_highlight_object(); // dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
     break;
   	case OBJECT_BASE_TYPE_NONE:
     open_subtools(SUBTOOLS_OBJECTS_CLEAR);
     set_special_highlight_object(); // dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
     break;
   }
			break;
  case FSP_DESIGN_TOOLS_DELETE:
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_DELETE].open = 1;
  	break;
  case FSP_DESIGN_TOOLS_AUTOCODE:
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_AUTOCODE].open = 1;
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS].open = 1;
   open_subtools(SUBTOOLS_AUTOCODE);
  	break;
/* 	case FSP_DESIGN_TOOLS_ACTIVE_LINK:
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_ACTIVE_LINK].open = 1;
   panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS].open = 1;
   open_subtools(SUBTOOLS_ACTIVE_LINK);
   break;*/
	}

}

void open_subtools(int subtools)
{
	dwindow.subtools_open = subtools;

	switch(subtools)
	{
		case SUBTOOLS_MAIN:
   set_sub_buttons_range(0, -1, -1); // closes all sub buttons
   break;
		case SUBTOOLS_SHAPE:
	  set_sub_buttons_range(DSB_SHAPE_FIRST, DSB_SHAPE_LAST, dwindow.templ->member[dwindow.selected_member].shape);
	  break;
		case SUBTOOLS_CORE:
	  set_sub_buttons_range(DSB_CORE_SHAPE_FIRST, DSB_CORE_SHAPE_LAST, dwindow.templ->member[dwindow.selected_member].shape);
	  break;
  case SUBTOOLS_EMPTY_LINK:
   set_sub_buttons_range(0, -1, -1); // closes all sub buttons
			break;
  case SUBTOOLS_ACTIVE_LINK:
   set_sub_buttons_range(0, -1, -1); // closes all sub buttons
			break;

/*		case SUBTOOLS_SHAPE5:
	  set_sub_buttons_range(DSB_SHAPE5_FIRST, DSB_SHAPE5_LAST, dwindow.templ->member[dwindow.selected_member].shape);
	  break;
		case SUBTOOLS_SHAPE6:
	  set_sub_buttons_range(DSB_SHAPE6_FIRST, DSB_SHAPE6_LAST, dwindow.templ->member[dwindow.selected_member].shape);
	  break;*/
//	 case SUBTOOLS_OBJECTS_LINK:
//   set_sub_buttons_range(DSB_OBJECT_LINK_FIRST, DSB_OBJECT_LINK_LAST, dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
//   break;
	 case SUBTOOLS_OBJECTS_STD:
   set_sub_buttons_range(DSB_OBJECT_STD_FIRST, DSB_OBJECT_STD_LAST, dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
   break;
	 case SUBTOOLS_OBJECTS_MOVE:
   set_sub_buttons_range(DSB_OBJECT_MOVE_FIRST, DSB_OBJECT_MOVE_LAST, dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
//   set_sub_buttons_range(0, -1, -1); // closes all sub buttons
   break;
	 case SUBTOOLS_OBJECTS_ATTACK:
   set_sub_buttons_range(DSB_OBJECT_ATTACK_FIRST, DSB_OBJECT_ATTACK_LAST, dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
   break;
	 case SUBTOOLS_OBJECTS_DEFEND:
   set_sub_buttons_range(DSB_OBJECT_DEFEND_FIRST, DSB_OBJECT_DEFEND_LAST, dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
   break;
	 case SUBTOOLS_AUTOCODE:
	 	set_autocode_sub_buttons();
/*	 	if (dwindow.templ->member[0].shape < FIRST_MOBILE_NSHAPE)
    set_sub_buttons_range(DSB_AUTOCODE_FIRST, DSB_AUTOCODE_FIRST, dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);
     else
      set_sub_buttons_range(DSB_AUTOCODE_FIRST + 1, DSB_AUTOCODE_LAST, dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type);*/
   break;
	 case SUBTOOLS_OBJECTS_MISC:
   set_sub_buttons_range(0, -1, -1); // closes all sub buttons
   break;
	 case SUBTOOLS_OBJECTS_CLEAR:
   set_sub_buttons_range(0, -1, -1); // closes all sub buttons
   break;

	}

}

// see also set_autocode_sub_buttons() below
void set_sub_buttons_range(int range_start, int range_end, int special_highlight_value)
{
	int i = 0;

	int button_index = 0;



	while(range_start + i <= range_end)
	{
		if (game.type == GAME_TYPE_MISSION) // story unlocks are only relevant in story mode, not custom games (GAME_TYPE_BASIC)
		{
		 switch(design_sub_button[range_start + i].type)
		 {
		  case DSBTYPE_OBJECT:
		 	 if (!story.unlock[otype[design_sub_button[range_start + i].value].unlock_index])
				 {
   		 i ++;
 					continue;
				 }
 			 break;
		  case DSBTYPE_CORE_SHAPE:
		  case DSBTYPE_SHAPE:
/*		 	fpr("\n i %i button %i nshape %i unlock_index %i unlocked %i", i,
											button_index,
											design_sub_button[range_start + i].value,
											nshape[design_sub_button[range_start + i].value].unlock_index,
											story.unlock[nshape[design_sub_button[range_start + i].value].unlock_index]);*/
		 	 if (!story.unlock[nshape[design_sub_button[range_start + i].value].unlock_index])
				 {
   		 i ++;
 					continue;
				 }
		 	 break;
		 }
		}
		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + button_index].open = 1;
		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + button_index].value [0] = range_start + i;
		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + button_index].value [2] = design_sub_button [range_start + i].value; // value [1] used below
		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + button_index].name = design_sub_button [range_start + i].name;
		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + button_index].value [3] = 0; // no special autocoder highlighting
		if (design_sub_button [range_start + i].value == special_highlight_value)
		{
			panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + button_index].value [1] = 1;
		}
		  else
			  panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + button_index].value [1] = 0;
		i ++;
		button_index ++;
#ifdef SANITY_CHECK
  if (i >= DESIGN_SUB_BUTTONS)
		{
			fpr("\nError: d_design.c: set_sub_buttons_range(): too many sub-buttons (%i) (range %i to %i).", i, range_start, range_end);
			error_call();
		}
#endif
	};

	while(button_index < DESIGN_SUB_BUTTONS)
	{
		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + button_index].open = 0;
		button_index++;
	};


}


// autocoder sub_buttons need some special code
static void set_autocode_sub_buttons(void)
{
	int i = 0;
	int range_start = DSB_AUTOCODE_FIRST;
	int range_end = DSB_AUTOCODE_LAST;

// this fills out some values in dcode_state, which are used below to work out which autocode types are available
 initialise_autocoder(); // in d_code_header.c

	while(range_start + i <= range_end)
	{
		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].open = 1;
		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [0] = range_start + i;
		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [2] = design_sub_button [range_start + i].value; // value [1] used below
		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].name = design_sub_button [range_start + i].name;
  panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [1] = 0; // no special highlight for autocode buttons
		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_YES; // may be set to unavailable state below


// static processes can only use NONE
  if (dwindow.templ->member[0].shape < FIRST_MOBILE_NSHAPE)
		{
			if (i != AUTOCODE_NONE)
			{
  		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_NO_STATIC;
			}
 		goto next_loop;
		}

		if (!dcode_state.unindexed_auto_class_present [AUTO_CLASS_MOVE])
		{
			if (i != AUTOCODE_NONE)
			{
  		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_MAYBE_NO_MOVE;
			}
 		goto next_loop;
		}

		switch(i)
		{
		 case AUTOCODE_NONE:
			 break; // anything can be none
			  // maybe should add a "maybe" mode for processes that would be better with another one?

			case AUTOCODE_STANDARD:
			case AUTOCODE_CHARGE:
			case AUTOCODE_CAUTIOUS:
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_MAIN]
					|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR])
						break; // success
//				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
//				{
//   		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_MAYBE_SPIKE;
//   		break;
//				}
  		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_MAYBE_NO_FWD_ATT;
				break;

// circle is like standard etc but is also happy with right attack, and can't use main attack
   case AUTOCODE_CIRCLE_CW:
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR]
					|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_RIGHT_DIR])
						break; // success
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_MAIN])
				{
   		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_MAYBE_POOR_MAIN;
   		break;
				}
//				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
//				{
//   		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_MAYBE_SPIKE;
//   		break;
//				}
  		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_MAYBE_NO_FWD_OR_R_ATT;
				break;

// circle is like standard etc but is also happy with lef attack
   case AUTOCODE_CIRCLE_ACW:
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR]
					|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_LEFT_DIR])
						break; // success
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_MAIN])
				{
   		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_MAYBE_POOR_MAIN;
   		break;
				}
//				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
//				{
//   		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_MAYBE_SPIKE;
   		//break;
//				}
  		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_MAYBE_NO_FWD_OR_L_ATT;
				break;

// not sure what to do for these. Probably any attack
   case AUTOCODE_ERRATIC:
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_FRONT_DIR]
					|| dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_LEFT_DIR])
						break; // success
				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_ATTACK_MAIN])
				{
   		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_MAYBE_POOR_MAIN;
   		break;
				}
//				if (dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
//				{
//   		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_MAYBE_SPIKE;
//   		break;
//				}
  		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_MAYBE_NO_FWD_OR_L_ATT;
				break;

			case AUTOCODE_BOMBARD:
				if (!dcode_state.unindexed_auto_class_present [AUTO_CLASS_SPIKE_FRONT])
   		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].value [3] = AUTOCODE_AVAILABLE_MAYBE_NO_SPIKE;
   	break;

		}

	next_loop:

		i ++;
#ifdef SANITY_CHECK
  if (i >= DESIGN_SUB_BUTTONS)
		{
			fpr("\nError: d_design.c: set_sub_buttons_range(): too many sub-buttons (%i) (range %i to %i).", i, range_start, range_end);
			error_call();
		}
#endif
	};

	while(i < DESIGN_SUB_BUTTONS)
	{
		panel[PANEL_DESIGN].element[FPE_DESIGN_SUB_BUTTON_0 + i].open = 0;
		i++;
	};

// now work out which ones are available:



}

void set_member_rotation_icon_position(void)
{

 int mem = dwindow.selected_member; // should only be called if it's valid

 int i;
 float icon_angle;
 int uplink_index;

 if (mem == 0)
	{
		uplink_index = 0;
		icon_angle = fixed_to_radians(dwindow.templ->member[mem].group_angle_offset);
//		return fixed_to_radians(dwindow.templ->member[0].group_angle_offset);
	}
		 else
			{

    for (i = 0; i < MAX_LINKS; i ++)
	   {
	    if (dwindow.templ->member[mem].object[i].type == OBJECT_TYPE_UPLINK)
		    break;
	   }

#ifdef SANITY_CHECK
  if (i == MAX_LINKS)
		{
			fpr("\n Error: d_design.c: get_member_rotation_icon_angle(): no uplink found on subprocess?");
			error_call();
		}
#endif

	   uplink_index = i;

	   icon_angle = fixed_to_radians(nshape[dwindow.templ->member[mem].shape].link_angle_fixed [uplink_index]);
	   icon_angle += fixed_to_radians(dwindow.templ->member[mem].group_angle_offset);
   	icon_angle += PI;
			}


	int member_x, member_y;

 member_x = al_fixtoi(dwindow.templ->member[mem].position.x);
	member_y = al_fixtoi(dwindow.templ->member[mem].position.y);

	dwindow.member_rotation_x = member_x + cos(icon_angle) * 80;
	dwindow.member_rotation_y = member_y + sin(icon_angle) * 80;

}



void set_link_rotation_icon_position(void)
{

 int mem = dwindow.selected_member; // should only be called if it's valid
 int link_index = dwindow.selected_link; // same here

 float icon_angle;

 icon_angle = fixed_to_radians(nshape[dwindow.templ->member[mem].shape].link_angle_fixed [link_index]);
 icon_angle += fixed_to_radians(dwindow.templ->member[mem].object[link_index].base_angle_offset);
 icon_angle += fixed_to_radians(dwindow.templ->member[mem].group_angle_offset);
//	icon_angle += PI;

	int link_x, link_y;
	int member_x, member_y;
 struct nshape_struct* nsh = &nshape [dwindow.templ->member[mem].shape];

 member_x = al_fixtoi(dwindow.templ->member[mem].position.x);
	member_y = al_fixtoi(dwindow.templ->member[mem].position.y);

 link_x = member_x + al_fixtoi(fixed_cos(dwindow.templ->member[mem].group_angle_offset + nsh->object_angle_fixed [link_index]) * nsh->object_dist_pixel [link_index]);
 link_y = member_y + al_fixtoi(fixed_sin(dwindow.templ->member[mem].group_angle_offset + nsh->object_angle_fixed [link_index]) * nsh->object_dist_pixel [link_index]);

	dwindow.link_rotation_x = link_x + cos(icon_angle) * 50;
	dwindow.link_rotation_y = link_y + sin(icon_angle) * 50;


}



// vertex_x and y are set if mouse_over_vertex is because it's annoying to recalculate them later
void check_mouse_over_dwindow(int mouse_x, int mouse_y, int* mouse_over_member, int* mouse_over_link, int* mouse_over_rotation, int *link_x, int* link_y)
{
	int i;
	int closest_member = -1;
	int closest_member_dist = 10000;
	int check_member;
	int member_size;
	int dist;
	int member_x, member_y;
	int near_selected_member = 0;



// first check for the mouse over the rotation icon
 if (dwindow.selected_member != -1)
	{
		if (dwindow.selected_link == -1)
	 {
 		int rotate_x = dwindow.member_rotation_x + DESIGN_WINDOW_CENTRE_X;
 		int rotate_y = dwindow.member_rotation_y + DESIGN_WINDOW_CENTRE_Y;
	 	int rotation_icon_size = 12;
		 if (mouse_x < rotate_x + rotation_icon_size
 			&& mouse_x > rotate_x - rotation_icon_size
			 && mouse_y < rotate_y + rotation_icon_size
			 && mouse_y > rotate_y - rotation_icon_size)
		 {
   	dwindow.highlight_rotation_time = inter.running_time;
 			*mouse_over_rotation = 1;
			 return;
		 }
	 }
	  else
			{
    int object_type = dwindow.templ->member[dwindow.selected_member].object [dwindow.selected_link].type;

    if (!otype[object_type].object_details.only_zero_angle_offset // can't rotate this kind of object
						&& object_type != OBJECT_TYPE_UPLINK
						&& object_type != OBJECT_TYPE_DOWNLINK)
				{
 		  int rotate_x = dwindow.link_rotation_x + DESIGN_WINDOW_CENTRE_X;
 		  int rotate_y = dwindow.link_rotation_y + DESIGN_WINDOW_CENTRE_Y;
	 	  int rotation_icon_size = 12;
		   if (mouse_x < rotate_x + rotation_icon_size
 		  	&& mouse_x > rotate_x - rotation_icon_size
			   && mouse_y < rotate_y + rotation_icon_size
			   && mouse_y > rotate_y - rotation_icon_size)
		   {
     	dwindow.highlight_rotation_time = inter.running_time;
 		  	*mouse_over_rotation = 1;
			   return;
		   }
				}
			}
	}

	for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (!dwindow.templ->member[i].exists)
			continue;
		member_size = 60;
		member_x = al_fixtoi(dwindow.templ->member[i].position.x) + DESIGN_WINDOW_CENTRE_X;
		member_y = al_fixtoi(dwindow.templ->member[i].position.y) + DESIGN_WINDOW_CENTRE_Y;
		if (mouse_x < member_x + member_size
			&& mouse_x > member_x - member_size
			&& mouse_y < member_y + member_size
			&& mouse_y > member_y - member_size)
		{
			if (i == dwindow.selected_member)
			{
				near_selected_member = 1; // will prefer this member if mouse is over one of its links
			}
// otherwise choose closest other member
			dist = abs(mouse_x - member_x) + abs(mouse_y - member_y);
			if (dist < closest_member_dist)
			{
				closest_member = i;
				closest_member_dist = dist;
			}
		}
	}

	*mouse_over_link = -1;

	int check;

	for (check = 0; check < 2; check ++)
	{

// If mouse is near the selected member, check whether the mouse is over its links first (before checking the links of the closest member, if different)
		if (check == 0)
		{
			if (!near_selected_member
				|| closest_member == dwindow.selected_member)
				continue;
			check_member = dwindow.selected_member;
		}
		 else
		 {
		 	if (*mouse_over_link != -1)
					break; // mouse must be over link of selected member, which overrides any other
				check_member = closest_member;
		 }

	 if (check_member != -1)
	 {

		 int over_link_x, over_link_y;
		 member_x = al_fixtoi(dwindow.templ->member[check_member].position.x) + DESIGN_WINDOW_CENTRE_X;
		 member_y = al_fixtoi(dwindow.templ->member[check_member].position.y) + DESIGN_WINDOW_CENTRE_Y;
		 struct nshape_struct* nsh = &nshape [dwindow.templ->member[check_member].shape];

#define LINK_SELECT_SIZE 12

#ifdef SANITY_CHECK
			if (nsh->links > MAX_LINKS)
			{
			 fpr("\nnshape has too many links (%i)", nsh->links);
			 error_call();
			}
#endif
		 for (i = 0; i < nsh->links; i ++)
		 {

			 over_link_x = member_x + al_fixtoi(fixed_cos(dwindow.templ->member[check_member].group_angle_offset + nsh->object_angle_fixed [i]) * nsh->object_dist_pixel [i]);
			 over_link_y = member_y + al_fixtoi(fixed_sin(dwindow.templ->member[check_member].group_angle_offset + nsh->object_angle_fixed [i]) * nsh->object_dist_pixel [i]);
			 if (mouse_x < over_link_x + LINK_SELECT_SIZE
				 && mouse_x > over_link_x - LINK_SELECT_SIZE
				 && mouse_y < over_link_y + LINK_SELECT_SIZE
 				&& mouse_y > over_link_y - LINK_SELECT_SIZE)
			 {
				 *mouse_over_link = i;
				 *mouse_over_member = check_member;
				 *link_x = over_link_x;
 				*link_y = over_link_y;
 				return;
			 }
 		}

	 }
	}

	*mouse_over_member = closest_member; // might be -1
	*mouse_over_link = -1;

}


void design_panel_button(int button_element)
{

  play_design_sound(TONE_3D);

 int i;

	switch(button_element)
	{
	 case FPE_DESIGN_TOOLS_EMPTY_NEW:
			dwindow.selected_member = -1;
			dwindow.selected_link = -1;
		 open_new_template(dwindow.templ);
		 reset_design_tools_subpanel(FSP_DESIGN_TOOLS_MAIN);
		 break;
		case FPE_DESIGN_TOOLS_MAIN_AUTO:
			if (dwindow.templ->locked)
			{
   	write_line_to_log("Can't change header of locked template.", MLOG_COL_ERROR);
				break;
			}
			reset_log();
			write_design_structure_to_source_edit(0); // 0 means the source_edit hasn't been verified as empty
			break;
		case FPE_DESIGN_TOOLS_MAIN_AUTOCODE:
/*			if (dwindow.templ->locked)
			{
				template_locked_design_message();
				break;
			}*/
			if (dwindow.templ->locked)
			{
// Note that opening the autocode menu initialises some class information in the template,
//  which could disrupt any active processes from the template. Preventing the autocoder affecting locked templates prevents this.
   	write_line_to_log("Can't autocode locked template.", MLOG_COL_ERROR);
				break;
			}
		 reset_design_tools_subpanel(FSP_DESIGN_TOOLS_AUTOCODE);
//			reset_log();
//			autocode();
			break;
		case FPE_DESIGN_TOOLS_MAIN_SYMM:
			if (!dwindow.templ->locked)
			 up_to_down_design_symmetry();
			  else
					 template_locked_design_message();
			break;
		case FPE_DESIGN_TOOLS_MAIN_LOCK:
// This now calls Write Header first:
   if (dwindow.templ->locked)
			{
   	write_line_to_log("Template already locked.", MLOG_COL_ERROR);
   	break;
			}
			reset_log();
			write_design_structure_to_source_edit(0); // 0 means the source_edit hasn't been verified as empty
			lock_template(dwindow.templ);
			break;
		case FPE_DESIGN_TOOLS_MAIN_UNLOCK:
   reset_log();
			unlock_template(dwindow.templ->player_index, dwindow.templ->template_index);
			break;
		case FPE_DESIGN_TOOLS_MAIN_DELETE:
		 reset_design_tools_subpanel(FSP_DESIGN_TOOLS_DELETE);
			break;
	 case FPE_DESIGN_TOOLS_MEMBER_SHAPE:
   open_subtools(SUBTOOLS_SHAPE);
		 break;
	 case FPE_DESIGN_TOOLS_CORE_CORE_SHAPE:
   open_subtools(SUBTOOLS_CORE);
		 break;
		case FPE_DESIGN_TOOLS_MEMBER_DELETE:
			if (!dwindow.templ->locked)
			 delete_selected_member();
			  else
					 template_locked_design_message();
			break;
/*	 case FPE_DESIGN_TOOLS_MEMBER_SHAPE5:
   open_subtools(SUBTOOLS_SHAPE5);
		 break;
	 case FPE_DESIGN_TOOLS_MEMBER_SHAPE6:
   open_subtools(SUBTOOLS_SHAPE6);
		 break;*/
	 case FPE_DESIGN_TOOLS_ADD_COMPONENT:
//	 	add_linked_member(dwindow.selected_member, dwindow.selected_link);
//   open_subtools(SUBTOOLS_OBJECTS_LINK);
   set_member_object(dwindow.selected_member, dwindow.selected_link, OBJECT_TYPE_DOWNLINK);
		 break;
		case FPE_DESIGN_TOOLS_CHANGE_UPLINK:
   set_member_object(dwindow.selected_member, dwindow.selected_link, OBJECT_TYPE_UPLINK);
			break;
//	 case FPE_DESIGN_TOOLS_NEXT_LINK:
//   open_subtools(SUBTOOLS_OBJECTS_LINK);
//		 break;
	 case FPE_DESIGN_TOOLS_VERTEX_OBJ_STD:
   open_subtools(SUBTOOLS_OBJECTS_STD);
		 break;
	 case FPE_DESIGN_TOOLS_VERTEX_OBJ_MOVE:
   open_subtools(SUBTOOLS_OBJECTS_MOVE);
		 break;
	 case FPE_DESIGN_TOOLS_VERTEX_OBJ_ATTACK:
   open_subtools(SUBTOOLS_OBJECTS_ATTACK);
		 break;
	 case FPE_DESIGN_TOOLS_VERTEX_OBJ_DEFEND:
   open_subtools(SUBTOOLS_OBJECTS_DEFEND);
		 break;
//	 case FPE_DESIGN_TOOLS_VERTEX_OBJ_MISC:
//   open_subtools(SUBTOOLS_OBJECTS_MISC);
//		 break;
	 case FPE_DESIGN_TOOLS_VERTEX_OBJ_CLEAR:
			if (dwindow.templ->locked)
			{
			 template_locked_design_message();
			 break;
			}
	 	if (otype[dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type].object_base_type == OBJECT_BASE_TYPE_LINK)
				break; // can't delete linked member this way. should write failure message here
			clear_member_object(dwindow.selected_member, dwindow.selected_link, 1);
   calculate_template_cost_and_power(dwindow.templ);
   dwindow.templ->modified = 1;
//   open_subtools(SUBTOOLS_OBJECTS_CLEAR);
		 break;
		case FPE_DESIGN_SUB_BUTTON_0:
		case FPE_DESIGN_SUB_BUTTON_1:
		case FPE_DESIGN_SUB_BUTTON_2:
		case FPE_DESIGN_SUB_BUTTON_3:
		case FPE_DESIGN_SUB_BUTTON_4:
		case FPE_DESIGN_SUB_BUTTON_5:
		case FPE_DESIGN_SUB_BUTTON_6:
		case FPE_DESIGN_SUB_BUTTON_7:
		case FPE_DESIGN_SUB_BUTTON_8:
		case FPE_DESIGN_SUB_BUTTON_9:
		case FPE_DESIGN_SUB_BUTTON_10:
		case FPE_DESIGN_SUB_BUTTON_11:
		case FPE_DESIGN_SUB_BUTTON_12:
		case FPE_DESIGN_SUB_BUTTON_13:
		case FPE_DESIGN_SUB_BUTTON_14:
			if (dwindow.templ->locked)
			{
			 template_locked_design_message();
			 break;
			}
			switch(design_sub_button[panel[PANEL_DESIGN].element[button_element].value[0]].type)
			{
			 case DSBTYPE_CORE_SHAPE:
			 	{
			 		int new_shape = design_sub_button[panel[PANEL_DESIGN].element[button_element].value[0]].value;
			 		int old_shape = dwindow.templ->member[dwindow.selected_member].shape;
			 		if (new_shape == old_shape)
							break;
			 		if (nshape[new_shape].links < nshape[old_shape].links)
						{
							for (i = 1; i < nshape[old_shape].links; i++)
							{
								if (i >= nshape[new_shape].links)
								{
									if (dwindow.templ->member[dwindow.selected_member].object [i].type == OBJECT_TYPE_DOWNLINK) // only test for downlinks; no uplinks on core
									{
								  start_log_line(MLOG_COL_ERROR);
					     write_to_log("Downlink at link ");
								  write_number_to_log(i);
								  write_to_log(" prevents change to core with only ");
								  write_number_to_log(nshape[new_shape].links);
								  write_to_log(" links.");
								  finish_log_line();
								  write_line_to_log("(try removing it, or moving it to a different link)", MLOG_COL_ERROR);
								  return;
									}
									dwindow.templ->member[dwindow.selected_member].object[i].type = OBJECT_TYPE_NONE;
								}
							}
						}
			 	 change_member_shape(dwindow.selected_member, design_sub_button[panel[PANEL_DESIGN].element[button_element].value[0]].value);
			 	}
			 	break;
				 case DSBTYPE_SHAPE:
			 	{
			 		int new_shape = design_sub_button[panel[PANEL_DESIGN].element[button_element].value[0]].value;
			 		int old_shape = dwindow.templ->member[dwindow.selected_member].shape;
			 		if (new_shape == old_shape)
							break;
			 		if (nshape[new_shape].links < nshape[old_shape].links)
						{
							for (i = 1; i < nshape[old_shape].links; i++)
							{
								if (i >= nshape[new_shape].links)
								{
									if (dwindow.templ->member[dwindow.selected_member].object [i].type == OBJECT_TYPE_UPLINK
										|| dwindow.templ->member[dwindow.selected_member].object [i].type == OBJECT_TYPE_DOWNLINK)
									{
								  start_log_line(MLOG_COL_ERROR);
										if (dwindow.templ->member[dwindow.selected_member].object [i].type == OBJECT_TYPE_UPLINK)
								   write_to_log("Uplink at link ");
									   else
								     write_to_log("Downlink at link ");
								  write_number_to_log(i);
								  write_to_log(" prevents change to shape with only ");
								  write_number_to_log(nshape[new_shape].links);
								  write_to_log(" links.");
								  finish_log_line();
								  write_line_to_log("(try removing it, or moving it to a different link)", MLOG_COL_ERROR);
								  return;
									}
									dwindow.templ->member[dwindow.selected_member].object[i].type = OBJECT_TYPE_NONE;
								}
							}
						}
			 	 change_member_shape(dwindow.selected_member, design_sub_button[panel[PANEL_DESIGN].element[button_element].value[0]].value);
			 	}
			 	break;
			 case DSBTYPE_OBJECT:
//			 	if (otype[dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type].object_class == OBJECT_CLASS_LINK)
//						break; // can't delete linked member this way. should write failure message here.
					set_member_object(dwindow.selected_member, dwindow.selected_link, design_sub_button[panel[PANEL_DESIGN].element[button_element].value[0]].value);
					break;
				case DSBTYPE_AUTOCODE_TYPE:
					if (panel[PANEL_DESIGN].element[button_element].value[3] == AUTOCODE_AVAILABLE_NO_STATIC)
					{
				  write_line_to_log("Autocode failed - static process can't use this attacking mode.", MLOG_COL_ERROR);
				  break;
					}
  			autocode(design_sub_button[panel[PANEL_DESIGN].element[button_element].value[0]].value);
     reset_design_tools_subpanel(FSP_DESIGN_TOOLS_MAIN);
					break;
/*				case DSBTYPE_ACTION:
					fpr("\n Action %i %i", panel[PANEL_DESIGN].element[button_element].value[0], DSB_ACTION_DELETE);
					switch(panel[PANEL_DESIGN].element[button_element].value[0])
					{
					 case DSB_ACTION_DELETE:
					 	fpr("\nDelete!");
					 	clear_template_including_source(dwindow.templ);
					 	break;

					}
					break;*/

			}
			break;
		case FPE_DESIGN_TOOLS_DELETE_CONFIRM:
			if (dwindow.templ->locked)
			{
			 template_locked_design_message();
			 break;
			}
	 	clear_template_including_source(dwindow.templ);
//   	if (dwindow.templ->active == 1) // failed for some reason
//     reset_design_tools_subpanel(FSP_DESIGN_TOOLS_MAIN);
//      else
						{
   		  write_line_to_log("Template deleted.", MLOG_COL_TEMPLATE);
       reset_design_tools_subpanel(FSP_DESIGN_TOOLS_EMPTY);
						}
	 	break;
	 case FPE_DESIGN_TOOLS_CORE_EXIT:
		case FPE_DESIGN_TOOLS_DELETE_EXIT:
		case FPE_DESIGN_TOOLS_MEMBER_EXIT:
		case FPE_DESIGN_TOOLS_VERTEX_EXIT:
		case FPE_DESIGN_TOOLS_AUTOCODE_EXIT:
   reset_design_tools_subpanel(FSP_DESIGN_TOOLS_MAIN);
   break;
	}

}

void change_member_shape(int mem, int new_shape)
{

	dwindow.templ->member[mem].shape = new_shape;
	dwindow.templ->member[mem].story_lock_failure = 0; // should be able to assume that the new shape is valid (doesn't matter too much if somehow this may not be true)

	if (mem == 0)
	{
		int i;
		for (i = 0; i < GROUP_CONNECTIONS; i++)
		{
			if (dwindow.templ->member[0].connection[i].template_member_index != -1)
				update_design_member_position_recursively(dwindow.templ, dwindow.templ->member[0].connection[i].template_member_index);
		}
	}
	 else
	  update_design_member_position_recursively(dwindow.templ, mem);
 if (mem == 0)
  reset_design_tools_subpanel(FSP_DESIGN_TOOLS_CORE);
   else
    reset_design_tools_subpanel(FSP_DESIGN_TOOLS_MEMBER);

 set_special_highlight_shape(new_shape);

 check_template_collisions(dwindow.templ);
 check_template_member_objects(dwindow.templ, mem);
 check_move_objects_obstruction(dwindow.templ);
 calculate_template_cost_and_power(dwindow.templ);
 dwindow.templ->modified = 1;

}

static void delete_selected_member(void)
{
	if (dwindow.selected_member == -1)
	{
		write_line_to_log("No subprocess selected.", MLOG_COL_ERROR);
		return;
	}
	if (dwindow.selected_member == 0)
	{
		write_line_to_log("Can't delete core process.", MLOG_COL_ERROR);
		return;
	}

 delete_member_and_submembers(dwindow.selected_member);

}

static void delete_member_and_submembers(int member_index)
{

 if (member_index == 0)
	 return;

// first need to remove the downlink from the parent subprocess (we can assume there's a downlink as this function doesn't work on the core):
 int parent_member = dwindow.templ->member[member_index].connection[0].template_member_index;
 int parent_link_object = dwindow.templ->member[member_index].connection[0].reverse_link_index;
 int parent_connection = dwindow.templ->member[member_index].connection[0].reverse_connection_index;

 dwindow.templ->member[parent_member].connection[parent_connection].template_member_index = -1;
	dwindow.templ->member[parent_member].object[parent_link_object].type = OBJECT_TYPE_NONE;
//	dwindow.templ->member[parent_member].object[parent_link_object].angle_offset = 0;
//	dwindow.templ->member[parent_member].object[parent_link_object].angle_offset_angle = 0;

 remove_design_members_recursively(member_index);
 check_template_collisions(dwindow.templ);
 check_move_objects_obstruction(dwindow.templ);
 calculate_template_cost_and_power(dwindow.templ);
 dwindow.templ->modified = 1;

}

static void remove_design_members_recursively(int mem)
{

 int i;

// note for i = 1 (uplink member is not removed)
	for (i = 1; i < GROUP_CONNECTIONS; i++)
	{
		if (dwindow.templ->member[mem].connection[i].template_member_index != -1)
			remove_design_members_recursively(dwindow.templ->member[mem].connection[i].template_member_index);
	}

	dwindow.templ->member[mem].exists = 0;

}

void set_special_highlight_shape(int new_shape)
{

   panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_MEMBER_SHAPE].value [0] = 0;
//   panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_MEMBER_SHAPE5].value [0] = 0;
//   panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_MEMBER_SHAPE6].value [0] = 0;
/*
   switch(shape_dat [dwindow.templ->member[dwindow.selected_member].shape] [0].vertices)
   {
			 case 4:
				 panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_MEMBER_SHAPE4].value [0] = 1; break;
			 case 5:
				 panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_MEMBER_SHAPE5].value [0] = 1; break;
			 case 6:
				 panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_MEMBER_SHAPE6].value [0] = 1; break;
   }
*/

}


void set_member_object(int mem, int obj, int new_obj)
{

// Deal with link objects specially:
 if (new_obj == OBJECT_TYPE_DOWNLINK)
	{
		if (dwindow.templ->member[mem].object[obj].type != OBJECT_TYPE_UPLINK
		&& dwindow.templ->member[mem].object[obj].type != OBJECT_TYPE_DOWNLINK)
		{
   clear_member_object(mem, obj, 0);
 		add_linked_member(dwindow.selected_member, dwindow.selected_link, 1);
   calculate_template_cost_and_power(dwindow.templ);
		}
// put warning here if fails
		return;
	}

	if (new_obj == OBJECT_TYPE_UPLINK
		&& dwindow.templ->member[mem].object[obj].type != OBJECT_TYPE_DOWNLINK)
	{
//  clear_member_object(mem, obj, 0); // change_uplink does this
  change_uplink(mem, obj, 1);
// shouldn't need to recalculate template cost here - only the link the uplink was on has changed
		return;
	}

// Otherwise, can't deal with a link object this way.
 if (otype[dwindow.templ->member[mem].object[obj].type].object_base_type == OBJECT_BASE_TYPE_LINK)
	{
  write_line_to_log("Can't delete link (a component is attached to it).", MLOG_COL_ERROR);
		return; // can't delete linked member this way. should write failure message here.
	}

 int old_type = dwindow.templ->member[mem].object[obj].type;

// call clear member object here?? (probably not as new object may be a similar type which should retain various values...)
	dwindow.templ->member[mem].object[obj].type = new_obj;

// most objects can only have a zero offset
	if (otype[new_obj].object_details.only_zero_angle_offset)
	{
	 dwindow.templ->member[mem].object[obj].base_angle_offset = 0;
	 dwindow.templ->member[mem].object[obj].base_angle_offset_angle = 0;
	}

	if (old_type == OBJECT_TYPE_MOVE)
		check_move_objects_obstruction(dwindow.templ);

 clear_member_object_classes(mem, obj);
 reset_design_tools_subpanel(FSP_DESIGN_TOOLS_ACTIVE_LINK);
 set_special_highlight_object();
 dwindow.templ->modified = 1;
	dwindow.templ->member[mem].object[obj].template_error = TEMPLATE_OBJECT_ERROR_NONE;

 check_template_member_objects(dwindow.templ, mem);
 if (new_obj == OBJECT_TYPE_MOVE)
		check_single_move_object_obstruction(dwindow.templ, mem, obj);

 calculate_template_cost_and_power(dwindow.templ); // this also calculates and sets member costs

}


void clear_member_object(int mem, int obj, int reset_panel)
{

 int old_object_type = dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type;

// Can't deal with a link object this way.
 if (otype[old_object_type].object_base_type == OBJECT_BASE_TYPE_LINK)
		return; // can't delete linked member this way. should write failure message here.

	if (old_object_type == OBJECT_TYPE_NONE)
			return; // already clear


 clear_member_object_classes(mem, obj);

	dwindow.templ->member[mem].object[obj].type = OBJECT_TYPE_NONE;
	dwindow.templ->member[mem].object[obj].base_angle_offset = 0;
	dwindow.templ->member[mem].object[obj].base_angle_offset_angle = 0;
	dwindow.templ->member[mem].object[obj].template_error = TEMPLATE_OBJECT_ERROR_NONE;
 dwindow.templ->modified = 1;

 check_template_member_objects(dwindow.templ, mem);

 if (old_object_type == OBJECT_TYPE_MOVE)
  check_move_objects_obstruction(dwindow.templ);

 calculate_template_member_cost(dwindow.templ, mem);

 if (reset_panel)
	{
  reset_design_tools_subpanel(FSP_DESIGN_TOOLS_ACTIVE_LINK);
  set_special_highlight_object();
	}

// NOTE: does not call calculate_template_cost. Calling function should do this, if appropriate

}

static void clear_member_object_classes(int mem, int obj)
{
	int i;

	for (i = 0; i < CLASSES_PER_OBJECT; i ++)
	{
// don't need to deal with the template's class list (it's generated from object class information in c_fix when the template is fixed)
		dwindow.templ->member[mem].object[obj].object_class [i] = -1;
	}

}

void set_special_highlight_object(void)
{

//   panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_ADD_LINK].value [0] = 0;
   panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_VERTEX_OBJ_STD].value [0] = 0;
   panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_VERTEX_OBJ_MOVE].value [0] = 0;
   panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_VERTEX_OBJ_ATTACK].value [0] = 0;
   panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_VERTEX_OBJ_DEFEND].value [0] = 0;
//   panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_VERTEX_OBJ_MISC].value [0] = 0;
   panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_VERTEX_OBJ_CLEAR].value [0] = 0;

   switch(otype[dwindow.templ->member[dwindow.selected_member].object[dwindow.selected_link].type].object_base_type)
   {
//			 case OBJECT_BASE_TYPE_LINK:
//				 panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_ADD_LINK].value [0] = 1; break;
			 case OBJECT_BASE_TYPE_STD:
				 panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_VERTEX_OBJ_STD].value [0] = 1; break;
			 case OBJECT_BASE_TYPE_MOVE:
				 panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_VERTEX_OBJ_MOVE].value [0] = 1; break;
			 case OBJECT_BASE_TYPE_ATTACK:
				 panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_VERTEX_OBJ_ATTACK].value [0] = 1; break;
			 case OBJECT_BASE_TYPE_DEFEND:
				 panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_VERTEX_OBJ_DEFEND].value [0] = 1; break;
//			 case OBJECT_BASE_TYPE_MISC:
//				 panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_VERTEX_OBJ_MISC].value [0] = 1; break;
			 case OBJECT_BASE_TYPE_NONE:
				 panel[PANEL_DESIGN].element [FPE_DESIGN_TOOLS_VERTEX_OBJ_CLEAR].value [0] = 1; break;
   }


}

// returns -1 on failure
// or member index of new member on success
// Does not call calculate_template_cost. Calling function should do so, if appropriate.
int add_linked_member(int parent_mem_index, int parent_link, int check_results_of_change)
{
	int new_mem;
	struct template_member_struct* parent_mem = &dwindow.templ->member[parent_mem_index];

	for (new_mem = 0; new_mem < GROUP_MAX_MEMBERS; new_mem++)
	{
		if (dwindow.templ->member[new_mem].exists == 0)
			break;
	}
	if (new_mem == GROUP_MAX_MEMBERS)
	{
  write_line_to_log("Too many components.", MLOG_COL_ERROR);
  return -1;
	}

	if (parent_mem->downlinks_from_core >= MAX_DOWNLINKS_FROM_CORE - 1)
	{
  write_line_to_log("Too many downlinks away from core.", MLOG_COL_ERROR);
  return -1;
	}

	int parent_connection;

	for (parent_connection = 1; parent_connection < GROUP_CONNECTIONS; parent_connection ++) // note begins at 1
	{
		if (parent_mem->connection[parent_connection].template_member_index == -1)
			break;
	}
	if (parent_connection == GROUP_CONNECTIONS)
	{
// write to log here...
  write_line_to_log("Parent has too many connections.", MLOG_COL_ERROR); // should never happen
  return -1;
	}

	init_templ_group_member(dwindow.templ, new_mem); // can probably remove this

	struct template_member_struct* mem = &dwindow.templ->member[new_mem];

	mem->exists = 1;
	mem->shape = NSHAPE_COMPONENT_BOX;
	mem->connection_angle_offset = al_itofix(0);
	mem->connection_angle_offset_angle = 0;
	mem->connection[0].template_member_index = parent_mem_index;
	mem->connection[0].reverse_connection_index = parent_connection;
	mem->connection[0].link_index = 0;
	mem->connection[0].reverse_link_index = parent_link;
	mem->object[0].type = OBJECT_TYPE_UPLINK;
	mem->downlinks_from_core = parent_mem->downlinks_from_core + 1;

	parent_mem->connection[parent_connection].template_member_index = new_mem;
	parent_mem->connection[parent_connection].link_index = parent_link;
	parent_mem->connection[parent_connection].reverse_connection_index = 0;
	parent_mem->connection[parent_connection].reverse_link_index = 0;
	parent_mem->object [parent_link].type = OBJECT_TYPE_DOWNLINK;
	parent_mem->object [parent_link].base_angle_offset = 0;
	parent_mem->object [parent_link].base_angle_offset_angle = 0;
 set_special_highlight_object();

 struct nshape_struct* parent_nshape = &nshape[parent_mem->shape];
 struct nshape_struct* child_nshape = &nshape[mem->shape];

	mem->group_angle_offset = parent_mem->group_angle_offset
	                          + parent_nshape->link_angle_fixed [parent_link]
	                          + (AFX_ANGLE_2 - child_nshape->link_angle_fixed [0]);

//	                        + shape_dat [dwindow.templ->member[parent_mem].shape] [dwindow.templ->member[parent_mem].size].vertex_angle [vertex]
//	                        + (AFX_ANGLE_2 - shape_dat [mem->shape] [mem->size].vertex_angle [0]);


// now work out new member's position:
 mem->position.x = parent_mem->position.x
                 + (symmetrical_cos(parent_mem->group_angle_offset + parent_nshape->link_angle_fixed [parent_link])
																		* (parent_nshape->link_dist_pixel [parent_link]))
																	- (symmetrical_cos(mem->group_angle_offset + child_nshape->link_angle_fixed [0])
																		* (child_nshape->link_dist_pixel [0]));
//																		* (get_link_dist_pixel(child_nshape->link_dist_pixel [0], 0)));

 mem->position.y = parent_mem->position.y
                 + (symmetrical_sin(parent_mem->group_angle_offset + parent_nshape->link_angle_fixed [parent_link])
																		* (parent_nshape->link_dist_pixel [parent_link]))
																	- (symmetrical_sin(mem->group_angle_offset + child_nshape->link_angle_fixed [0])
																		* (child_nshape->link_dist_pixel [0]));
//																		* (get_link_dist_pixel(child_nshape->link_dist_pixel [0], 0)));




 if (check_results_of_change)
	{
  check_template_collisions(dwindow.templ);
  check_move_objects_obstruction(dwindow.templ);
  reset_design_tools_subpanel(FSP_DESIGN_TOOLS_ACTIVE_LINK);
	}

 calculate_template_cost_and_power(dwindow.templ);
 dwindow.templ->modified = 1;

	return new_mem;
}


// Use this to move the uplink from one link to another. Doesn't affect parent's downlink (except by changing some internal values)
int change_uplink(int child_mem_index, int new_link_index, int check_results_of_change)
{


 if (child_mem_index == 0)
	{
		write_line_to_log("Cores do not have uplinks.", MLOG_COL_ERROR);
		return 0;
	}

	struct template_member_struct* child_mem = &dwindow.templ->member[child_mem_index];
	struct template_member_struct* parent_mem = &dwindow.templ->member[child_mem->connection[0].template_member_index];
	int parent_connection = child_mem->connection[0].reverse_connection_index;

	child_mem->connection_angle_offset = al_itofix(0);
	child_mem->connection_angle_offset_angle = 0;
	child_mem->object[child_mem->connection[0].link_index].type = OBJECT_TYPE_NONE;
	child_mem->object[new_link_index].type = OBJECT_TYPE_UPLINK;
	child_mem->connection[0].link_index = new_link_index;
//	mem->connection[0].reverse_link_index = parent_link; doesn't change

	parent_mem->connection[parent_connection].reverse_link_index = new_link_index;
	parent_mem->object[parent_mem->connection[parent_connection].link_index].base_angle_offset_angle = 0;
	parent_mem->object[parent_mem->connection[parent_connection].link_index].base_angle_offset = 0;

 update_design_member_position_recursively(dwindow.templ, child_mem_index);


 if (check_results_of_change)
	{
  check_template_collisions(dwindow.templ);
  check_move_objects_obstruction(dwindow.templ);
  reset_design_tools_subpanel(FSP_DESIGN_TOOLS_ACTIVE_LINK);
// don't need to call these functions if called from e.g. symmetric member copying functions.
	}

 dwindow.templ->modified = 1;

	return 1;

}


// Use this to move a link between parent components. Index of uplink on child stays the same.
int move_uplink(int child_mem_index, int new_parent_index, int new_downlink_index, int check_results_of_change)
{

#ifdef SANITY_CHECK
 if (child_mem_index == 0)
	{
		fpr("\nFailure: d_design.c: change_uplink(): tried to change uplink of core.");
		return 0;
	}
#endif

//fpr("\n cmi %i npi %i ndi %i", child_mem_index, new_parent_index, new_downlink_index);
	struct template_member_struct* child_mem = &dwindow.templ->member[child_mem_index];
	struct template_member_struct* old_parent_mem = &dwindow.templ->member[child_mem->connection[0].template_member_index];
	struct template_member_struct* new_parent_mem = &dwindow.templ->member[new_parent_index];
// old_parent_mem and new_parent_mem may be the same.
	int old_parent_connection = child_mem->connection[0].reverse_connection_index;
	int uplink_index = child_mem->connection[0].link_index;
	int old_parent_downlink_index = old_parent_mem->connection[old_parent_connection].link_index;
	int new_parent_connection; // set below
	int i;


 if (old_parent_mem != new_parent_mem) // or if just moving between different links of same parent
	{
// make sure that the new parent is not the child, or a sub-process of the child:
//  if (new_parent_mem == child_mem) - actually this should have been tested already
  i = new_parent_index;
  while(i != 0)
		{
			i = dwindow.templ->member[i].connection[0].template_member_index;
			if (i == child_mem_index)
			{
    write_line_to_log("Can't link process to its own subprocess!", MLOG_COL_ERROR);
    return 0;
			}
		}

// if the uplink is being moved to a different member entirely, need to find an available connection index:
		new_parent_connection = -1;
		for (i = 1; i < GROUP_CONNECTIONS; i ++) // note for i = 1
		{
			if (new_parent_mem->connection[i].template_member_index == -1)
			{
				new_parent_connection = i;
				break;
			}
		}
		if (new_parent_connection == -1)
		{
// I think this is actually impossible, but can't hurt to test for it
   write_line_to_log("Too many connections on new parent component.", MLOG_COL_ERROR);
   return 0;
		}
//fpr(" npi %i", new_parent_connection);
		if (new_parent_mem->object[new_downlink_index].type == OBJECT_TYPE_UPLINK
			|| new_parent_mem->object[new_downlink_index].type == OBJECT_TYPE_DOWNLINK)
		{
// in theory could overwrite downlinks in this way, but it would be complicated
   write_line_to_log("Can't move a link to an existing link object.", MLOG_COL_ERROR);
   return 0;
		}
	}

	child_mem->connection_angle_offset = al_itofix(0);
	child_mem->connection_angle_offset_angle = 0;
//	child_mem->object[old_uplink_index].type = OBJECT_TYPE_NONE;
//	child_mem->object[new_uplink_index].type = OBJECT_TYPE_UPLINK;
//	child_mem->connection[0].link_index = new_uplink_index;
 old_parent_mem->object[old_parent_downlink_index].type = OBJECT_TYPE_NONE;
 old_parent_mem->object[old_parent_downlink_index].base_angle_offset_angle = 0;
 old_parent_mem->object[old_parent_downlink_index].base_angle_offset = 0;



 if (old_parent_mem == new_parent_mem)
	{
  old_parent_mem->object[old_parent_downlink_index].type = OBJECT_TYPE_NONE;
  old_parent_mem->object[new_downlink_index].type = OBJECT_TYPE_DOWNLINK;
	 old_parent_mem->connection[old_parent_connection].link_index = new_downlink_index;
	 old_parent_mem->connection[old_parent_connection].reverse_link_index = uplink_index;
	 child_mem->connection[0].reverse_link_index = new_downlink_index;
	}
	 else
		{

	  old_parent_mem->connection[old_parent_connection].template_member_index = -1;
	  old_parent_mem->object[old_parent_downlink_index].type = OBJECT_TYPE_NONE;

	  new_parent_mem->connection[new_parent_connection].link_index = new_downlink_index;
	  new_parent_mem->connection[new_parent_connection].template_member_index = child_mem_index;
	  new_parent_mem->connection[new_parent_connection].reverse_connection_index = 0;
	  new_parent_mem->connection[new_parent_connection].reverse_link_index = uplink_index;

	  new_parent_mem->object[new_downlink_index].type = OBJECT_TYPE_DOWNLINK;
	  new_parent_mem->object[new_downlink_index].base_angle_offset_angle = 0;
	  new_parent_mem->object[new_downlink_index].base_angle_offset = 0;

  	child_mem->connection[0].template_member_index = new_parent_index;
  	child_mem->connection[0].reverse_connection_index = new_parent_connection;
  	child_mem->connection[0].reverse_link_index = new_downlink_index;
//  	child_mem->connection[0].link_index = uplink_index; above

		}


 update_design_member_position_recursively(dwindow.templ, child_mem_index);


 if (check_results_of_change)
	{
  check_template_collisions(dwindow.templ);
  check_move_objects_obstruction(dwindow.templ);
  reset_design_tools_subpanel(FSP_DESIGN_TOOLS_ACTIVE_LINK);
// don't need to call these functions if called from e.g. symmetric member copying functions.
	}

 dwindow.templ->modified = 1;

	return 1;

}

static int move_downlink(int old_parent_index, int old_downlink_index, int new_parent_index, int new_downlink_index)
{

//		struct template_member_struct* old_parent_mem = &dwindow.templ->member[child_mem->connection[0].template_member_index];

 int i;
 int old_parent_connection_index = -1;

 for (i = 0; i < GROUP_CONNECTIONS; i ++)
	{
		if (dwindow.templ->member[old_parent_index].connection[i].template_member_index != -1
			&& dwindow.templ->member[old_parent_index].connection[i].link_index == old_downlink_index)
		{
			old_parent_connection_index = i;
			break;
		}
	}

 int child_mem_index = dwindow.templ->member[old_parent_index].connection[old_parent_connection_index].template_member_index;

 if (child_mem_index == new_parent_index)
	{
// this means that the user is trying to move a downlink to one of the directly downlinked component's links.
// treat this as changing the uplink:
   change_uplink(child_mem_index, new_downlink_index, 1);
//   write_line_to_log("Can't copy downlink object to downlinked component!", MLOG_COL_ERROR);
   return 1;
	}


 return move_uplink(child_mem_index, new_parent_index, new_downlink_index, 1);

}


/*
Preconditions for up-down symmetry:
 - core process is aligned left-right on one of its symmetry axes.
  - so the shape code will need to record what these axes are for each core shape.
 - no shapes linked from above centre line overlap into below.

	- there aren't too many members (check this after pruning below mirror line)

*/
static void up_to_down_design_symmetry(void)
{

 mirror_process_on_axis_recursively(0);

 update_design_member_positions(dwindow.templ);
 check_template_collisions(dwindow.templ);
 check_template_member_objects(dwindow.templ, 1);
 check_move_objects_obstruction(dwindow.templ);
 calculate_template_cost_and_power(dwindow.templ);
 write_line_to_log("Mirror operation complete.", MLOG_COL_COMPILER);

 dwindow.templ->modified = 1;

}

static void copy_nonlink_object(int target_member_index, int target_object_index, int source_member_index, int source_object_index)
{

	dwindow.templ->member[target_member_index].object[target_object_index].type = dwindow.templ->member[source_member_index].object[source_object_index].type;
	dwindow.templ->member[target_member_index].object[target_object_index].base_angle_offset = dwindow.templ->member[source_member_index].object[source_object_index].base_angle_offset;
	dwindow.templ->member[target_member_index].object[target_object_index].base_angle_offset_angle = dwindow.templ->member[source_member_index].object[source_object_index].base_angle_offset_angle;
	dwindow.templ->member[target_member_index].object[target_object_index].template_error = dwindow.templ->member[source_member_index].object[source_object_index].template_error; // maybe not needed?

	int i;

	for (i = 0; i < CLASSES_PER_OBJECT; i ++)
	{
 	dwindow.templ->member[target_member_index].object[target_object_index].object_class [i] = dwindow.templ->member[source_member_index].object[source_object_index].object_class [i];
	}

}



void delete_downlink_object(int parent_member, int object_index)
{

 int i;

 for (i = 1;	i < GROUP_CONNECTIONS; i ++)
	{
		if (dwindow.templ->member[parent_member].connection[i].link_index == object_index
			&& dwindow.templ->member[parent_member].connection[i].template_member_index != -1)
		{
			delete_member_and_submembers(dwindow.templ->member[parent_member].connection[i].template_member_index);
			return;
		}
	}

#ifdef SANITY_CHECK
 fpr("\n Error: d_design.c: delete_downlink_object(): failed to find downlink at object %i of member %i.", object_index, parent_member);
 error_call();
#endif

}

// use this to mirror a process (including core process) on the horizontal centreline.
// returns 1 on success, 0 on failure. Failure should halt mirroring if process is core, but doesn't need to otherwise.
static int mirror_process_on_axis_recursively(int member_index)
{

 int i;
 int mirror_axis_index = -1;
 struct nshape_struct* axis_nshape = &nshape[dwindow.templ->member[member_index].shape];

// some non-core shapes don't have any mirror axes
 if (axis_nshape->mirror_axes == 0)
		return 0; // this can't happen to cores (all core shapes should have symmetry axes)

// Need to find which mirror axis we're using.
// This is different for cores and non-cores:

#define MIRROR_AXIS_TOLERANCE 32

 if (member_index == 0)
	{
  for (i = 0; i < axis_nshape->mirror_axes; i++)
	 {

 		if (angle_difference_int((ANGLE_1 - dwindow.templ->member[member_index].connection_angle_offset_angle) & ANGLE_MASK, axis_nshape->mirror_axis_angle [i]) < MIRROR_AXIS_TOLERANCE)
		 {
			 mirror_axis_index = i;
			 break;
 		}
	 }
	}
	 else
		{
   for (i = 0; i < axis_nshape->mirror_axes; i++)
	  {

		  if (angle_difference_int((ANGLE_1 - fixed_angle_to_int(dwindow.templ->member[member_index].group_angle_offset)) & ANGLE_MASK, axis_nshape->mirror_axis_angle [i]) < MIRROR_AXIS_TOLERANCE)
		  {
 			 mirror_axis_index = i;
			  break;
 		 }
	  }
		}



 if (mirror_axis_index == -1)
	{
		if (member_index == 0)
		{
		 write_line_to_log("Failed - core must be aligned symmetrically.", MLOG_COL_ERROR);
		 write_line_to_log("(Hold control while rotating core to lock to symmetrical alignments.)", MLOG_COL_ERROR);
		}
		return 0; // if non-core just don't bother mirroring it (should print a warning?)
	}

/*
 fpr("\nshape %i axis %i", dwindow.templ->member[member_index].shape, mirror_axis_index);

 for (i = 0; i < axis_nshape->links; i ++)
	{
		fpr("\n mirror of %i is %i", i, mirror_object_centreline(member_index, mirror_axis_index, i));
	}
*/
// great - now we've confirmed that the shape is aligned correctly.
// next, we go through all links below the centre line and remove anything attached.
// links below centre line have mirrored_object -2

// (remember to ignore anything at angle 0 and ANGLE_2 (don't delete or copy))

 for (i = 0; i < axis_nshape->links; i ++)
	{
/*		if (((dwindow.templ->member[0].connection_angle_offset_angle
				  +	fixed_angle_to_int(axis_nshape->link_angle_fixed [i]))
				   & ANGLE_MASK)	< ANGLE_2)*/
		if (axis_nshape->mirrored_object_centreline [mirror_axis_index] [i] == -2)
		{
			switch(dwindow.templ->member[member_index].object[i].type)
			{
			 case OBJECT_TYPE_NONE:
 				break;
 			case OBJECT_TYPE_DOWNLINK:
 				delete_downlink_object(member_index, i);
 				clear_member_object_classes(member_index, i);
 				break;
 			default:
					clear_member_object(member_index, i, 0);
					break;
			}
		}
 }

// now copy from top to bottom:

 int mirror_object_index;

 for (i = 0; i < axis_nshape->links; i ++)
	{

		mirror_object_index = mirror_object_centreline(member_index, mirror_axis_index, i);

		if (mirror_object_index == -2)
			continue; // object is below axis and doesn't need to be mirrored

// now check for horizontal downlinks, and call this function recursively if they're found
		if (mirror_object_index == -1) // horizontal
		{
			if (dwindow.templ->member[member_index].object[i].type == OBJECT_TYPE_DOWNLINK
				&& dwindow.templ->member[member_index].object[i].base_angle_offset_angle == 0) // this may have already been caught above but check anyway
			{
				mirror_process_on_axis_recursively(get_linked_member_index(member_index, i)); // can ignore return value as it doesn't really matter if it fails
			}
			continue; // non-downlink horizontal links aren't copied
		}

/*		if (((dwindow.templ->member[0].connection_angle_offset_angle
				  +	fixed_angle_to_int(axis_nshape->link_angle_fixed [i]))
				   & ANGLE_MASK)	> ANGLE_2)*/
		{
			switch(dwindow.templ->member[member_index].object[i].type)
			{
			 case OBJECT_TYPE_NONE:
			 case OBJECT_TYPE_UPLINK: // possible if non-core process
 				break;
 			case OBJECT_TYPE_DOWNLINK:
// 				 int new_link_index = mirror_object_centreline(member_index, mirror_axis_index, i);
 			 copy_symmetrical_downlink_recursively(member_index, member_index, i, mirror_object_centreline(member_index, mirror_axis_index, i)); // note old and new member index are the same
 				break;
 			default:
// 				mirror_object_index = core_mirror_object(0,i);
 				copy_nonlink_object(member_index, mirror_object_index, member_index, i);
     fix_symmetrical_object_angle(member_index, i, member_index, mirror_object_index);
//				 dwindow.templ->member[member_index].object[mirror_object_index].base_angle_offset_angle = angle_difference_signed_int(0, 0 - dwindow.templ->member[member_index].object[i].base_angle_offset_angle);
//				 dwindow.templ->member[member_index].object[mirror_object_index].base_angle_offset = int_angle_to_fixed(dwindow.templ->member[member_index].object[mirror_object_index].base_angle_offset_angle);
					break;
			}
		}

	}

 return 1;

}

// call after the object has been copied
static void fix_symmetrical_object_angle(int old_member_index, int old_object, int new_member_index, int new_object)
{

	struct nshape_struct* nsh = &nshape [dwindow.templ->member[old_member_index].shape];

 al_fixed old_angle_absolute = dwindow.templ->member[old_member_index].group_angle_offset + nsh->object_angle_fixed [old_object] + dwindow.templ->member[old_member_index].object[old_object].base_angle_offset;

 al_fixed new_angle_absolute = (AFX_ANGLE_1 - old_angle_absolute) & AFX_MASK;

// work out base angle (angle of connection from parent if connection_angle_offset is zero
 al_fixed base_angle = (dwindow.templ->member[new_member_index].group_angle_offset + nshape [dwindow.templ->member[new_member_index].shape].object_angle_fixed [new_object]) & AFX_MASK;

 al_fixed new_angle_offset = angle_difference_signed(base_angle, new_angle_absolute); //(new_angle - base_angle) & AFX_MASK;




//   dwindow.templ->member[new_member_index].object[new_object].base_angle_offset = new_angle_offset; //new_angle_offset) & AFX_MASK;
//   dwindow.templ->member[new_member_index].object[new_object].base_angle_offset_angle = fixed_angle_to_int(new_angle_offset); //new_angle_offset) & AFX_MASK;


//return;


// Need to convert new angle to integer angle units (so it can be stored in source code form) then back to al_fixed:
   dwindow.templ->member[new_member_index].object[new_object].base_angle_offset_angle = angle_difference_signed_int(0, fixed_angle_to_int(new_angle_offset));

   if (dwindow.templ->member[new_member_index].object[new_object].base_angle_offset_angle > ANGLE_4)
				dwindow.templ->member[new_member_index].object[new_object].base_angle_offset_angle = ANGLE_4;
   if (dwindow.templ->member[new_member_index].object[new_object].base_angle_offset_angle < -ANGLE_4)
				dwindow.templ->member[new_member_index].object[new_object].base_angle_offset_angle = -ANGLE_4;

   dwindow.templ->member[new_member_index].object[new_object].base_angle_offset = int_angle_to_fixed(dwindow.templ->member[new_member_index].object [new_object].base_angle_offset_angle); //new_angle_offset) & AFX_MASK;

   dwindow.templ->modified = 1;

}


void copy_symmetrical_downlink_recursively(int old_parent_member_index, int new_parent_member_index, int object_index, int new_link_index)
{
//	struct template_member_struct* parent_mem = &dwindow.templ->member[parent_member_index];
	int old_child_member_index = get_linked_member_index(old_parent_member_index, object_index);
	struct template_member_struct* old_child_mem = &dwindow.templ->member[old_child_member_index];

#ifdef SANITY_CHECK
 if (new_link_index == -1)
	{
		fpr("\n Error: d_design.c: copy_symmetrical_downlink_recursively(): new_link_index is -1 (parent_member_index %i).", old_parent_member_index);
		error_call();
	}
#endif

 int new_member_index = add_linked_member(new_parent_member_index, new_link_index, 0);

 if (new_member_index == -1)
	{
		return; // failed for some reason (e.g. too many members)
	}

	struct template_member_struct* new_mem = &dwindow.templ->member[new_member_index];

 new_mem->shape = old_child_mem->shape;
// ignore objects and connections for now
// ignore position and approximation values

 int i, j;

 int mirror_object_index;

// before we do anything else, update the uplink object:
 for (i = 0; i < nshape[old_child_mem->shape].links; i++)
	{
		if (old_child_mem->object[i].type == OBJECT_TYPE_UPLINK)
		{
		 	mirror_object_index = mirror_object_noncentre(new_member_index, i); // should be able to assume that this is a noncentre process
    clear_member_object(new_member_index, mirror_object_index, 0);
    change_uplink(new_member_index, mirror_object_index, 0);
    new_mem->group_angle_offset = AFX_ANGLE_1 - old_child_mem->group_angle_offset;
    new_mem->connection_angle_offset = AFX_ANGLE_1 - old_child_mem->connection_angle_offset;
// correct connection_angle_offset_angle to an offset because it's used in the auto process definition generation:
    if (old_child_mem->connection_angle_offset_angle < 0)
					new_mem->connection_angle_offset_angle = 0 - old_child_mem->connection_angle_offset_angle;
				  else
       new_mem->connection_angle_offset_angle = ANGLE_1 - old_child_mem->connection_angle_offset_angle;
		 	break;
		}
	}

 for (i = 0; i < nshape[old_child_mem->shape].links; i++)
	{
		switch(old_child_mem->object[i].type)
		{
		 case OBJECT_TYPE_UPLINK: // already updated above
		 	break;
/*		 	mirror_object_index = mirror_object(new_member_index, 0, i);
    clear_member_object(new_member_index, mirror_object_index, 0);
    change_uplink(new_member_index, mirror_object_index, 0);
    new_mem->group_angle_offset = AFX_ANGLE_1 - old_child_mem->group_angle_offset;
    new_mem->connection_angle_offset = AFX_ANGLE_1 - old_child_mem->connection_angle_offset;
    new_mem->connection_angle_offset_angle = ANGLE_1 - old_child_mem->connection_angle_offset_angle;
		 	break;*/
 	 case OBJECT_TYPE_DOWNLINK:

// 	 	break;
// need to call this function recursively using the parent member's actual child (not a symmetrical version of it) as the source member
	 	 for (j = 0; j < GROUP_CONNECTIONS; j++)
				{
					if (old_child_mem->connection[j].link_index == i)
					{
//	 	   copy_symmetrical_downlink_recursively(old_child_mem->connection[j].template_member_index, noncore_mirror_object(new_member_index,i), -1);
//	 	   copy_symmetrical_downlink_recursively(old_child_member_index, noncore_mirror_object(new_member_index,i), -1);
	 	   copy_symmetrical_downlink_recursively(old_child_member_index, new_member_index, i, mirror_object_noncentre(old_child_member_index, i));
	 	   break;

					}
				}
#ifdef SANITY_CHECK
    if (j == GROUP_CONNECTIONS)
				{
					fpr("\n Failure: failed to find link index (old_parent_member %i new_member_index %i link_index %i", old_parent_member_index, new_member_index, i);
 	 	 for (j = 0; j < GROUP_CONNECTIONS; j++)
	 			{
	 				fpr(" \n (old_child_member connection %i link_index %i)", j, old_child_mem->connection[j].link_index);
	 			}
				}
#endif
				mirror_object_index = mirror_object_noncentre(new_member_index, i);
    fix_symmetrical_object_angle(old_child_member_index, i, new_member_index, mirror_object_index);
//				new_mem->object[mirror_object_index].base_angle_offset_angle = 0 - old_child_mem->object[i].base_angle_offset_angle;
//				new_mem->object[mirror_object_index].base_angle_offset = int_angle_to_fixed(new_mem->object[mirror_object_index].base_angle_offset_angle);
		 	break;
		 case OBJECT_TYPE_NONE:
				break; // do nothing
		 default:
				mirror_object_index = mirror_object_noncentre(new_member_index, i);
				copy_nonlink_object(new_member_index, mirror_object_index, old_child_member_index, i);
    fix_symmetrical_object_angle(old_child_member_index, i, new_member_index, mirror_object_index);
//				new_mem->object[mirror_object_index].base_angle_offset_angle = 0 - old_child_mem->object[i].base_angle_offset_angle;
//				new_mem->object[mirror_object_index].base_angle_offset = int_angle_to_fixed(new_mem->object[mirror_object_index].base_angle_offset_angle);
				break;
		}
	}

 dwindow.templ->member[new_parent_member_index].object[new_link_index].base_angle_offset_angle = new_mem->connection_angle_offset_angle;
 dwindow.templ->member[new_parent_member_index].object[new_link_index].base_angle_offset = int_angle_to_fixed(new_mem->connection_angle_offset_angle);



}

/*
static void fix_symmetrical_connection_angle(int old_member_index, int old_member_group_angle_offset, int new_member_index, int new_uplink_object
{

// need to work out the angle of the new parent's downlink that gives the new child the same group_angle_offset as the old child
 int uplink_to_component_angle_difference = angle_difference(0, nshape[dwindow.templ->member[new_member_index].shape].link_angle_fixed [new_uplink_object]);

}*/


static int mirror_object_centreline(int member_index, int mirror_axis, int object_index)
{

	return nshape[dwindow.templ->member[member_index].shape].mirrored_object_centreline [mirror_axis] [object_index];

}

static int mirror_object_noncentre(int member_index, int object_index)
{

	return nshape[dwindow.templ->member[member_index].shape].mirrored_object_noncentre [object_index];

}

int get_linked_member_index(int parent_member_index, int link_object_index)
{
	int i;

	for (i = 0; i < GROUP_CONNECTIONS; i ++)
	{
		if (dwindow.templ->member[parent_member_index].connection[i].template_member_index != -1
			&&	dwindow.templ->member[parent_member_index].connection[i].link_index == link_object_index)
			return dwindow.templ->member[parent_member_index].connection[i].template_member_index;
	}

#ifdef SANITY_CHECK
 fpr("\n Error: d_design.c: get_linked_member_index(): couldn't find link index (parent_member %i, link_object_index %i.", parent_member_index, link_object_index);

	for (i = 0; i < GROUP_CONNECTIONS; i ++)
	{
	 fpr("\n  connection %i index %i", i, dwindow.templ->member[parent_member_index].connection[i].link_index);
	}


 error_call();
#endif

	return -1; // should never happen

}

/*
#include "g_proc_new.h"
void create_new_from_design(void)
{

	cart core_position;
	core_position.x = w.data_well[0].position.x + al_itofix(800);
	core_position.y = w.data_well[0].position.y;

//	create_new_from_template(dwindow.templ, dwindow.templ->player_index, core_position, al_itofix(0));
//	create_new_from_template(&templ[0][0], 0, core_position, al_itofix(0));

	core_position.x = w.data_well[1].position.x - al_itofix(1600);
	core_position.y = w.data_well[1].position.y;

	create_new_from_template(&templ[1][0], 1, core_position, al_itofix(128));

//	w.core[0].group_speed.x = al_ftofix(0.2);
/ *
	core_position.x = al_itofix(1100);
	core_position.y = al_itofix(1000);

	create_new_from_template(dwindow.templ, 0, core_position, al_itofix(0));

	w.core[1].group_speed.x = al_ftofix(-0.2);* /

}
*/

static void template_locked_design_message(void)
{
	write_line_to_log("Can't modify design of locked template.", MLOG_COL_ERROR);
}

static void play_design_sound(int note)
{

	play_interface_sound(SAMPLE_BLIP2, note);

}

