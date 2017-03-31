
#include <allegro5/allegro.h>


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
#include "e_editor.h"
#include "e_clip.h"
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

#include "c_header.h"
#include "c_keywords.h"


#define PROCESS_INDENT 2


extern struct object_type_struct otype [OBJECT_TYPES];
extern struct editorstruct editor;
extern struct identifierstruct identifier [IDENTIFIERS];
extern struct design_window_struct dwindow;
extern struct nshape_struct nshape [NSHAPES];

extern struct dcode_state_struct dcode_state;
extern char auto_class_name [AUTO_CLASSES] [AUTO_CLASS_NAME_LENGTH];


static int dcode_read_next(struct dtoken_struct* dtoken);
static int dcode_expect_directive(const char* directive_name);
static int dcode_skip_spaces(void);

static int write_dcode_members_recursively(int member_index);
static int write_dcode_objects_recursively(int member_index);

static int auto_classify_objects(struct template_struct* templ);

static int add_auto_class_to_object(struct template_struct* templ, int member_index, int object_index, int auto_class);
static int get_auto_class_index(struct template_struct* templ, int auto_class);

static void object_autoclass_failure_message(struct template_struct* templ, int member_index, int object_index);
static int get_object_quadrant(struct template_struct* templ, int member_index, int object_index);
static int get_object_group_angle(struct template_struct* templ, int member_index, int object_index);
static void clear_unused_classes(struct template_struct* templ);
void remove_auto_classes_from_objects(struct template_struct* templ);

// This file needs special dcode writing functions that insert text into an existing source file
//  (unlike the main autocode function, which can assume it's adding text to the end of an empty file)
static int dcode_header_add_string(char* source_str);
static int dcode_header_add_number(int num);
static int dcode_header_newline(void);

static int write_dcode_header_buffer_to_source(int start_line);

// if verified_clear_file == 1, this function assumes that editor.currentsource_edit_index is valid and that the source_edit is empty.
//  - and also that its classes have been cleared.
// Otherwise it makes sure that either the file is empty, or there is a proper process header for it to erase and replace.
// returns 1 success/0 failure
int write_design_structure_to_source_edit(int verified_clear_file)
{

 write_line_to_log("Writing process header.", MLOG_COL_COMPILER);

 struct template_struct* templ = dwindow.templ;

	if (editor.current_source_edit_index == -1)
		return 0;
	dcode_state.ses = &editor.source_edit [editor.current_source_edit_index];
	dcode_state.source_line = 0;
	dcode_state.cursor_pos = 0;
	dcode_state.indent_level = 0;

	int i, j, start_line, end_line;

	int empty_file = 1;

	if (!verified_clear_file)
 {
	 for (i = 0; i < SOURCE_TEXT_LINES; i ++)
	 {
 		if (dcode_state.ses->text [dcode_state.ses->line_index [i]] [0] != '\0')
		 {
 			empty_file = 0;
			 break;
		 }
	 }
 }

 dcode_state.ses->cursor_pos = 0; // prevents the cursor being left stranded past the line end if it's on a line that gets shortened

 if (empty_file)
	{
		start_line = 1;
		end_line = 3;
// #process and #code are added to empty files later later
//		strcpy(dcode_state.ses->text [dcode_state.ses->line_index [0]], "#process");
//		strcpy(dcode_state.ses->text [dcode_state.ses->line_index [end_line]], "#code");
	}
	 else
		{

// first, find the start of the process definition:
   if (!dcode_expect_directive("process")) // looks for #process line
		  return dcode_error("#process directive not found (source file must contain #process directive, or be empty).");

   dcode_state.source_line++;
   start_line = dcode_state.source_line;

   if (!dcode_expect_directive("code")) // looks for #code line
		  return dcode_error("#code directive was not found (source file must contain #code directive, or be empty).");

   end_line = dcode_state.source_line;
		}

 if (end_line > start_line + 2)
	{
//		delete_lines(dcode_state.ses, start_line + 1, end_line - start_line - 3);
		delete_lines(dcode_state.ses, start_line + 1, end_line - start_line - 1);
	}

// let's prune out any classes with no members:
 if (!verified_clear_file)
  clear_unused_classes(templ);

// At this point we're unlikely to fail, so we assemble a class list:
 auto_classify_objects(templ);

// Now need to add stuff.
 dcode_state.dcode_buffer [0] = '\0';
 dcode_state.source_line = start_line + 1;
 dcode_state.cursor_pos = 0;
// If any empty file, start with #process:
 if (empty_file)
	{
		 dcode_header_newline();
		 if (templ->name [0] == 0)
    dcode_header_add_string("#process \"unnamed\"");
     else
					{
      dcode_header_add_string("#process \"");
      dcode_header_add_string(templ->name);
      dcode_header_add_string("\"");
					}
	}

 int written_a_class_declaration = 0;

 dcode_state.process_structure_lines = 1;
 for (i = 0; i < OBJECT_CLASSES; i ++)
	{
		if (dwindow.templ->object_class_active [i] != 0)
		{
			if (!written_a_class_declaration)
			{
		  dcode_header_newline();
		  dcode_header_newline();
    dcode_header_add_string("// This process has objects with the following auto classes:");
				written_a_class_declaration = 1;
			}
		 dcode_header_newline();
   dcode_header_add_string("class ");
   dcode_header_add_string(dwindow.templ->object_class_name [i]);
   dcode_header_add_string(";");
		}
	}

// Now just add class declarations for the remaining auto classes:

	int auto_class_in_use [AUTO_CLASSES];

	for (i = 0; i < AUTO_CLASSES; i ++)
	{
		auto_class_in_use [i] = 0;
	}

 for (i = 0; i < OBJECT_CLASSES; i ++)
	{
		if (dwindow.templ->object_class_active [i] != 0
			&& dwindow.templ->object_class_name [i] [0] == 'a'
			&& dwindow.templ->object_class_name [i] [1] == 'u') // anything starting with "au" is probably an auto class
		{
			for (j = 0; j < AUTO_CLASSES; j ++)
			{
				if (auto_class_in_use [j] == 0
				 && strcmp(dwindow.templ->object_class_name [i], auto_class_name [j]) == 0)
				{
					auto_class_in_use [j] = 1;
					break;
				}
			}
		}
	}

	int added_any_unused_auto_classes = 0;

	for (i = 0; i < AUTO_CLASSES; i ++)
	{

		if (!auto_class_in_use [i])
		{
			if (!added_any_unused_auto_classes)
			{
		  dcode_header_newline();
		  dcode_header_newline();
    dcode_header_add_string("// The following auto classes are not currently used by any objects:");
//		  dcode_header_newline();
	   added_any_unused_auto_classes = 1;
			}
		 dcode_header_newline();
   dcode_header_add_string("class ");
   dcode_header_add_string(auto_class_name [i]);
   dcode_header_add_string(";");
	 }
	}


// Now core:
 dcode_header_newline();
 dcode_header_newline();
 dcode_header_add_string(identifier[nshape[templ->member[0].shape].keyword_index].name);
 dcode_header_add_string(", ");
 dcode_header_add_number(templ->member[0].connection_angle_offset_angle); // this is core angle
 dcode_header_add_string(", ");
 dcode_state.indent_level += PROCESS_INDENT;
 write_dcode_objects_recursively(0); // skip member and go straight to objects
 dcode_state.indent_level = 0;

 if (empty_file)
	{
		 dcode_header_newline();
   dcode_header_add_string("#code");
	}

 write_dcode_header_buffer_to_source(start_line);

 write_line_to_log("Process structure written to source.", MLOG_COL_COMPILER);

// For now, clear the editor's undo stack at this point.
// In future should really run all changes made here through the undo functions...
 init_undo();
// dcode_state.ses->cursor_line = 1; - no - autocode function may want to pick up at end of process header
// dcode_state.ses->cursor_pos = 0;
// dcode_state.ses->cursor_base = 1;

 dwindow.templ->modified = 0; // resets modified value as the process header in the source code should now match the design

 return 1;
}

// called when the 'autocode' button is pressed - initialises dcode_state to help work out which autocode types are available.
int initialise_autocoder(void)
{

 clear_unused_classes(dwindow.templ);

 auto_classify_objects(dwindow.templ);

 return 1;

}

static int write_dcode_members_recursively(int member_index)
{
 dcode_header_newline();
 dcode_state.process_structure_lines++;
 dcode_header_add_string("{");
//	dcode_add_string("process_shape"); // to be replaced by actual shape name
// fpr("kw%i,", nshape[dwindow.templ->member[member_index].shape].keyword_index);
	dcode_header_add_string(identifier[nshape[dwindow.templ->member[member_index].shape].keyword_index].name); // to be replaced by actual shape name
 dcode_header_add_string(", // component ");
 dcode_header_add_number(member_index);
	dcode_state.indent_level+=PROCESS_INDENT;
 write_dcode_objects_recursively(member_index);
	dcode_state.indent_level-=PROCESS_INDENT;
 dcode_header_newline();
 dcode_state.process_structure_lines++;
 dcode_header_add_string("}");
 dcode_state.indent_level-=PROCESS_INDENT;
 dcode_header_newline();
 dcode_state.process_structure_lines++;
 dcode_state.indent_level+=PROCESS_INDENT;

 return 1;
}

static int write_dcode_objects_recursively(int member_index)
{
	int i,j;
	for (i = 0; i < nshape[dwindow.templ->member[member_index].shape].links; i++)
	{
	 dcode_header_newline();
  dcode_state.process_structure_lines++;
  dcode_header_add_string("{");
		switch(dwindow.templ->member[member_index].object[i].type)
		{
			case OBJECT_TYPE_NONE:
    dcode_header_add_string(identifier[KEYWORD_OBJECT_NONE].name); break;
			case OBJECT_TYPE_UPLINK:
    dcode_header_add_string(identifier[KEYWORD_OBJECT_UPLINK].name); break;
			case OBJECT_TYPE_DOWNLINK:
    dcode_header_add_string(identifier[KEYWORD_OBJECT_DOWNLINK].name);
    dcode_header_add_string(", ");
/*
    fpr("\n downlink: parent %i link %i base_angle_offset %i(%i) base_angle_offset_angle %i ads %i",
								member_index,
								i,
								al_fixtoi(dwindow.templ->member[member_index].object[i].base_angle_offset),
								fixed_angle_to_int(dwindow.templ->member[member_index].object[i].base_angle_offset),
								dwindow.templ->member[member_index].object[i].base_angle_offset_angle,
								angle_difference_signed_int(0, dwindow.templ->member[member_index].object[i].base_angle_offset_angle));
*/
    dcode_header_add_number(angle_difference_signed_int(0, dwindow.templ->member[member_index].object[i].base_angle_offset_angle));
    dcode_header_add_string(", ");
    for (j = 0; j < GROUP_CONNECTIONS; j++)
				{
					if (dwindow.templ->member[member_index].connection[j].template_member_index != -1
						&& dwindow.templ->member[member_index].connection[j].link_index == i)
							break;
				}
				dcode_state.indent_level+=PROCESS_INDENT;
				write_dcode_members_recursively(dwindow.templ->member[member_index].connection[j].template_member_index);
				dcode_state.indent_level-=PROCESS_INDENT;
    break;
			default:
    dcode_header_add_string(identifier[otype[dwindow.templ->member[member_index].object[i].type].keyword_index].name); break;
		}

		int class_index;

		for (class_index = 0; class_index < CLASSES_PER_OBJECT; class_index ++)
		{
			if (dwindow.templ->member[member_index].object[i].object_class[class_index] != -1)
			{
				dcode_header_add_string(":");
				dcode_header_add_string(dwindow.templ->object_class_name[dwindow.templ->member[member_index].object[i].object_class[class_index]]);
			}
		}

  if (dwindow.templ->member[member_index].object[i].type != OBJECT_TYPE_DOWNLINK)
		{
   dcode_header_add_string(", ");
   dcode_header_add_number(angle_difference_signed_int(0, dwindow.templ->member[member_index].object[i].base_angle_offset_angle));
    // angle_difference_signed_int makes sure it's an offset in the correct range
// classes go here

  }

   if (i < MAX_OBJECTS - 1)
				dcode_header_add_string("},");
			  else
					{
      dcode_header_add_string("}");
	    }

	}

	return 1;

}


// Call this function to assign objects to the auto classes.
// It won't interfere with the non-auto classes.
// It won't deal with the template's class lists - only with the objects' object_class fields.
// Is called during write header or autocode
//  is also called when the autocode button is pressed, to work out which kinds of autocode are available.
//   - this updates the template, but that shouldn't matter as any updated values should be worked out again
//     before the template does anything (and locked templates can't be autocoded)
// returns 1 on success, 0 on failure (e.g. if the template has too many classes)
static int auto_classify_objects(struct template_struct* templ)
{
 int i, j;
 int quadrant;
 int object_angle;

 int components_protectable_by_interface = 0;
 int this_component_protectable_by_interface;

// remove auto classes, but not user-defined classes
 remove_auto_classes_from_objects(templ);

// clear existing auto classes
 for (i = 0; i < AUTO_CLASSES; i ++)
	{
		dcode_state.auto_class_index [i] = -1;
		dcode_state.unindexed_auto_class_present [i] = 0;
	}

// also clear object_type_present (used later in autocoding):
 for (i = 0; i < OBJECT_TYPES; i ++)
	{
		dcode_state.object_type_present [i] = 0;
	}

 for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (templ->member[i].exists == 0)
			continue;
  this_component_protectable_by_interface = 1;
		for (j = 0; j < nshape[templ->member[i].shape].links; j++)
		{
   dcode_state.object_type_present [templ->member[i].object[j].type] ++; // note that this counts OBJECT_TYPE_NONE
			switch(templ->member[i].object[j].type)
			{
			 case OBJECT_TYPE_MOVE:
			 	add_auto_class_to_object(templ, i, j, AUTO_CLASS_MOVE);
    	quadrant = get_object_quadrant(templ, i, j);
    	if (quadrant == QUADRANT_FORWARD)
 			 	add_auto_class_to_object(templ, i, j, AUTO_CLASS_RETRO);
     this_component_protectable_by_interface = 0;
     break;
    case OBJECT_TYPE_INTERFACE:
					this_component_protectable_by_interface = 0;
					break;
			 case OBJECT_TYPE_PULSE:
			 case OBJECT_TYPE_PULSE_L:
			 case OBJECT_TYPE_PULSE_XL:
			 case OBJECT_TYPE_STREAM_DIR:
			 case OBJECT_TYPE_ULTRA_DIR:
			 case OBJECT_TYPE_SLICE:
    	quadrant = get_object_quadrant(templ, i, j);
			 	add_auto_class_to_object(templ, i, j, AUTO_CLASS_ATTACK_FRONT_DIR + quadrant);
     break;
			 case OBJECT_TYPE_SPIKE:
    	object_angle = get_object_group_angle(templ, i, j); // this is & ANGLE_MASK
    	if (!dcode_state.mobile)
					{
 					 write_line_to_log("Warning: the autocoder does not currently support", MLOG_COL_WARNING);
 					 write_line_to_log("spike objects on static processes.", MLOG_COL_WARNING);
 			 	 add_auto_class_to_object(templ, i, j, AUTO_CLASS_SPIKE_FRONT); // just assign it to the class anyway.
 			 	 break;
					}
    	if (object_angle < ANGLE_4
						||	object_angle > ANGLE_2 + ANGLE_4) // could adjust a little bit more?
 			 	 add_auto_class_to_object(templ, i, j, AUTO_CLASS_SPIKE_FRONT);
							 else
								{
	   					 write_line_to_log("Warning: spike objects should be aimed forwards or sidewards.", MLOG_COL_WARNING);
 			 	 add_auto_class_to_object(templ, i, j, AUTO_CLASS_SPIKE_FRONT); // just assign it to the class anyway.
								}
     break;
    case OBJECT_TYPE_STREAM:
    case OBJECT_TYPE_BURST:
    case OBJECT_TYPE_BURST_L:
    case OBJECT_TYPE_BURST_XL:
				case OBJECT_TYPE_ULTRA:
    	quadrant = get_object_quadrant(templ, i, j);
    	if (quadrant != QUADRANT_FORWARD)
					{
						{
						 object_autoclass_failure_message(templ, i, j);
						 write_line_to_log("Warning: fixed attacking objects should be aimed forwards.", MLOG_COL_WARNING);
// could probably give a more useful warning than that...
						}
						break;
					}
			 	add_auto_class_to_object(templ, i, j, AUTO_CLASS_ATTACK_MAIN);
     break;
    case OBJECT_TYPE_HARVEST:
			 	add_auto_class_to_object(templ, i, j, AUTO_CLASS_HARVEST);
     break;
    case OBJECT_TYPE_ALLOCATE:
			 	add_auto_class_to_object(templ, i, j, AUTO_CLASS_ALLOCATE);
     break;
    case OBJECT_TYPE_STABILITY:
			 	add_auto_class_to_object(templ, i, j, AUTO_CLASS_STABILITY);
     break;
//    case OBJECT_TYPE_BUILD:
//			 	add_auto_class_to_object(templ, i, j, AUTO_CLASS_BUILD);
//     break;

    default:
    case OBJECT_TYPE_NONE:
     break;

			}
		}
  components_protectable_by_interface += this_component_protectable_by_interface;
	}

	if (dcode_state.object_type_present [OBJECT_TYPE_INTERFACE]
  && components_protectable_by_interface == 0)
	{
	 write_line_to_log("Warning: process has interface, but no components that can be protected.", MLOG_COL_WARNING);
	 write_line_to_log(" (a component with a move or interface object can't be protected)", MLOG_COL_WARNING);
	}

	return 1;
}

void remove_auto_classes_from_objects(struct template_struct* templ)
{


 int class_is_auto_class [OBJECT_CLASSES];
 int i, j, k;

 for (i = 0; i < OBJECT_CLASSES; i ++)
	{
		class_is_auto_class [i] = 0;
		if (templ->object_class_name [i] [0] == 'a'
			&& templ->object_class_name [i] [1] == 'u'
			&& templ->object_class_name [i] [2] == 't'
			&& templ->object_class_name [i] [3] == 'o'
			&& templ->object_class_name [i] [4] == '_')
		  class_is_auto_class [i] = 1;
	}

	for (i = 0; i < GROUP_MAX_MEMBERS; i++)
	{
		if (templ->member[i].exists)
		{
		 for (j = 0; j < MAX_OBJECTS; j++)
			{
				for (k = 0; k < CLASSES_PER_OBJECT; k ++)
				{
					if (templ->member[i].object[j].object_class[k] != -1
						&& class_is_auto_class [templ->member[i].object[j].object_class[k]])
							templ->member[i].object[j].object_class[k] = -1;
				}
			}
		}
	}

}


static void object_autoclass_failure_message(struct template_struct* templ, int member_index, int object_index)
{

						start_log_line(MLOG_COL_WARNING);
						write_to_log("Couldn't assign component ");
						write_number_to_log(member_index);
						write_to_log(" object ");
						write_number_to_log(object_index);
						write_to_log(" (type ");
						write_to_log(otype[templ->member[member_index].object[object_index].type].name);
						write_to_log(") to class.");
						finish_log_line();

}

// returns one of the QUADRANT enums based on which direction an object is pointing
// assumes templ/member_index/object_index are valid
static int get_object_quadrant(struct template_struct* templ, int member_index, int object_index)
{
	int object_angle = get_object_group_angle(templ, member_index, object_index);
/*	templ->member[member_index].object[object_index].base_angle_offset_angle;
	object_angle += fixed_angle_to_int(templ->member[member_index].group_angle_offset);
	object_angle += fixed_angle_to_int(nshape[templ->member[member_index].shape].object_angle_fixed [object_index]);
	object_angle &= ANGLE_MASK;*/

	if (object_angle < ANGLE_8
		|| object_angle >= ANGLE_1 - ANGLE_8)
		return QUADRANT_FORWARD;

	if (object_angle < ANGLE_4 + ANGLE_8)
		return QUADRANT_RIGHT;

	if (object_angle < ANGLE_2 + ANGLE_8)
		return QUADRANT_BACK;

	return QUADRANT_LEFT;

}

static int get_object_group_angle(struct template_struct* templ, int member_index, int object_index)
{

	int object_angle = templ->member[member_index].object[object_index].base_angle_offset_angle;
	object_angle += fixed_angle_to_int(templ->member[member_index].group_angle_offset);
	object_angle += fixed_angle_to_int(nshape[templ->member[member_index].shape].object_angle_fixed [object_index]);
	object_angle += fixed_angle_to_int(templ->member[member_index].object [object_index].base_angle_offset_angle); // not sure about this one
	object_angle &= ANGLE_MASK;

	return object_angle;

}

// returns 1 on success, 0 on failure.
// failure just means it couldn't add the class to the object; doesn't need to be treated as fatal.
static int add_auto_class_to_object(struct template_struct* templ, int member_index, int object_index, int auto_class)
{

	int object_class_index;

	if (dcode_state.auto_class_index [auto_class] != -1)
		object_class_index = dcode_state.auto_class_index [auto_class];
		 else
			{
	   object_class_index = get_auto_class_index(templ, auto_class);
	   if (object_class_index == -1)
					return 0;
			}

	int i;

	for (i = 0; i < CLASSES_PER_OBJECT; i ++)
	{
		if (templ->member[member_index].object[object_index].object_class [i] == object_class_index)
		{
   dcode_state.unindexed_auto_class_present [auto_class] ++;
//   dcode_state.unindexed_auto_class_number [auto_class] ++;
			return 1; // object already a member of this class
		}
	}

	for (i = 0; i < CLASSES_PER_OBJECT; i ++)
	{
		if (templ->member[member_index].object[object_index].object_class [i] == -1)
			break;
	}

 if (i == CLASSES_PER_OBJECT)
	{
		dcode_warning("an object has too many classes."); // could be more informative than this.
		return 0;
	}

 templ->member[member_index].object[object_index].object_class [i] = object_class_index;
 dcode_state.unindexed_auto_class_present [auto_class] ++;
// dcode_state.unindexed_auto_class_number [auto_class] ++;

	return 1;

}

static int get_auto_class_index(struct template_struct* templ, int auto_class)
{

	if (dcode_state.auto_class_index [auto_class] != -1)
		return dcode_state.auto_class_index [auto_class];

// auto class not found during the present classification process. Need to compare existing class names to it.
 int i;

 for (i = 0; i < OBJECT_CLASSES; i++)
	{
		if (templ->object_class_active [i])
		{
			if (strcmp(templ->object_class_name [i], auto_class_name [auto_class]) == 0)
			{
    dcode_state.auto_class_index [auto_class] = i;
				return i;
			}
		}
	}

//	auto class hasn't been used at all:
 for (i = 0; i < OBJECT_CLASSES; i ++)
	{
		if (templ->object_class_active [i] == 0)
			break;
	}

	if (i == OBJECT_CLASSES)
	{
		write_line_to_log("Error: too many classes in use (maximum 16).", MLOG_COL_ERROR);
		return -1;
	}

 templ->object_class_active [i] = 1;
 strcpy(templ->object_class_name [i], auto_class_name [auto_class]);
 dcode_state.auto_class_index [auto_class] = i;

 return i;
}

static void clear_unused_classes(struct template_struct* templ)
{
	int class_members [OBJECT_CLASSES];
	int i, j, k;

	for (i = 0; i < OBJECT_CLASSES; i ++)
	{
		class_members [i] = 0;
	}

	for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (templ->member[i].exists == 0)
		 continue;
		for (j = 0; j < nshape[templ->member[i].shape].links; j++)
		{
			if (templ->member[i].object[j].type == OBJECT_TYPE_NONE)
				continue;
			for (k = 0; k < CLASSES_PER_OBJECT; k ++)
			{
				if (templ->member[i].object[j].object_class [k] != -1)
					class_members [templ->member[i].object[j].object_class [k]] ++;
			}
		}
	}

	for (i = 0; i < OBJECT_CLASSES; i ++)
	{
		if (class_members [i] == 0)
			templ->object_class_active [i] = 0;
	}


}


static int dcode_expect_directive(const char* directive_name)
{

 int i;
	struct dtoken_struct dtoken;

 for (i = dcode_state.source_line; i < SOURCE_TEXT_LINES; i ++)
	{
		if (dcode_state.ses->text [dcode_state.ses->line_index [i]] [0] == '#')
		{
			dcode_state.source_line = i;
			dcode_state.cursor_pos = 1;
			if (!dcode_read_next(&dtoken))
				return dcode_error("reached end of source looking for directive.");
			if (strcmp(dtoken.name, directive_name) == 0)
				return 1;
			return 0;
		}
	}

 return 0;
}

// Simple text parser that reads until a space or end of line
//  Doesn't stop at punctuation!
static int dcode_read_next(struct dtoken_struct* dtoken)
{

 if (!dcode_skip_spaces())
		return 0;

 int dtoken_pos = 0;
 dtoken->name [0] = 0;

 while(TRUE)
	{
  if (dcode_state.cursor_pos >= SOURCE_TEXT_LINE_LENGTH - 1
			|| dcode_state.ses->text [dcode_state.ses->line_index [dcode_state.source_line]] [dcode_state.cursor_pos] == ' '
			|| dcode_state.ses->text [dcode_state.ses->line_index [dcode_state.source_line]] [dcode_state.cursor_pos] == '\0')
		{
			return 1;
		}
		if (dtoken_pos >= DTOKEN_LENGTH - 2)
		 return dcode_error("word is too long?");
  dtoken->name [dtoken_pos] = dcode_state.ses->text [dcode_state.ses->line_index [dcode_state.source_line]] [dcode_state.cursor_pos];
  dtoken_pos++;
  dtoken->name [dtoken_pos] = 0;
  dcode_state.cursor_pos ++;
	};


}

// returns 1 if found something other than end of source_edit, 0 otherwise.
// doesn't write error.
static int dcode_skip_spaces(void)
{

	while(TRUE)
	{
		if (dcode_state.ses->text [dcode_state.ses->line_index [dcode_state.source_line]] [dcode_state.cursor_pos] == '\0')
		{
			dcode_state.source_line ++;
			dcode_state.cursor_pos = 0;
			if (dcode_state.source_line >= SOURCE_TEXT_LINES)
				return 0;
		}
		if (dcode_state.ses->text [dcode_state.ses->line_index [dcode_state.source_line]] [dcode_state.cursor_pos] != ' '
			&& dcode_state.ses->text [dcode_state.ses->line_index [dcode_state.source_line]] [dcode_state.cursor_pos] != '\t')
		{
			return 1;
		}
		dcode_state.cursor_pos ++;
	};


 return 0;
}



static int dcode_header_add_string(char* source_str)
{

#ifdef SANITY_CHECK
if (strlen(dcode_state.dcode_buffer) > DCODE_BUFFER_LENGTH - 100)
{
	fpr("\nError: d_code.c: dcode_add_string(): dcode_buffer length limit exceeded.");
	error_call();
}
//fpr("\n sl %i max %i", strlen(dcode_state.dcode_buffer), DCODE_BUFFER_LENGTH - 100);
#endif

strcat(dcode_state.dcode_buffer, source_str);
//fpr(" finished");
// TO DO: optimise by keeping record of length of dcode_buffer and adding to end rather than strcatting the whole thing.

/*
// dcode line lengths shouldn't be too near the max length of source lines, but let's be conservative:
	if (dcode_state.cursor_pos >= SOURCE_TEXT_LINE_LENGTH - 33)
	{
		if (!dcode_newline())
			return 0; // will probably be ignored
  dcode_state.process_structure_lines++;
	}

	while(TRUE)
	{
		dcode_state.ses->text [dcode_state.ses->line_index [dcode_state.source_line]] [dcode_state.cursor_pos] = *source_str;
		if (*source_str == 0)
			break;
		dcode_state.cursor_pos++;
		source_str++;
	}
*/
	return 1;
}

static int dcode_header_add_number(int num)
{

	char num_string [10]; // should just be writing s16b anyway

	snprintf(num_string, 9, "%i", num);

	return dcode_header_add_string(num_string);

}

static int dcode_header_newline(void)
{

 char write_string [120] = "\n"; // 120 should be plenty of space

 int i;

 for (i = 0; i < dcode_state.indent_level; i ++)
	{
		write_string [i+1] = ' ';
	}
	write_string [i+1] = '\0';

 dcode_header_add_string(write_string);
/*
 int dcode_buffer_end = strlen(dcode_state.dcode_buffer);

	if (dcode_state.source_line >= SOURCE_TEXT_LINES - 10)
		return dcode_error("ran out of space in source file"); // unlikely to ever happen
 dcode_state.source_line ++;
	insert_empty_lines(dcode_state.ses, dcode_state.source_line, 1); // need to optimise by calling this in chunks
 dcode_state.cursor_pos = 0;
// insert_empty_lines() makes sure line is at 0
// int i;
 for (dcode_state.cursor_pos = 0; dcode_state.cursor_pos < dcode_state.indent_level; dcode_state.cursor_pos ++)
	{
		dcode_state.ses->text [dcode_state.ses->line_index [dcode_state.source_line]] [dcode_state.cursor_pos] = ' ';
	}
	dcode_state.ses->text [dcode_state.ses->line_index [dcode_state.source_line]] [dcode_state.cursor_pos] = 0;
*/
	return 1;

}



int write_dcode_header_buffer_to_source(int start_line)
{

//fpr("\n buffer: [[[%s]]]", dcode_state.dcode_buffer);


	int i = 0;
	int j;
	int number_of_lines = 1;

	for (i = 0; i < DCODE_BUFFER_LENGTH; i ++)
	{
		if (dcode_state.dcode_buffer [i] == '\n')
			number_of_lines++;
		if (dcode_state.dcode_buffer [i] == '\0')
			break;
	}

 insert_empty_lines(dcode_state.ses, start_line + 1, number_of_lines);
 dcode_state.ses->saved = 0;

	char write_line [SOURCE_TEXT_LINE_LENGTH];
	int write_line_index = start_line;

 i = 0;

	while(TRUE)
	{
		j = 0;
		while(TRUE)
		{
   write_line [j] = dcode_state.dcode_buffer [i];
   if (write_line[j] == '\n'
				|| write_line[j] == '\0')
			{
				write_line[j] = '\0';
				break;
			}
   i++;
			j++;
		};
    strcpy(dcode_state.ses->text [dcode_state.ses->line_index [write_line_index++]], write_line);
		if (dcode_state.dcode_buffer [i] == '\0')
			break;
		i++;
	};


 update_source_lines(dcode_state.ses, start_line, number_of_lines + 3);
 dcode_state.source_line = number_of_lines + 3; // this is only correct if the file was empty (but it's only used for autocoding, which assumes the file was empty)


	return 1;

}

