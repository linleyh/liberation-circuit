/*

Contains code for generated a template's process design from source code (actually, scode that has been preprocessed)


*/
#include <allegro5/allegro.h>
#include "m_config.h"
#include "g_header.h"
#include "c_header.h"

#include "g_misc.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "e_log.h"

#include "c_lexer.h"
#include "c_prepr.h"
#include "c_compile.h"
#include "d_design.h"
#include "d_geo.h"
#include "g_shapes.h"
#include "m_maths.h"
#include "c_keywords.h"
#include "t_template.h"
#include "c_fix.h"

#define read_ctoken if (!read_next(&ctoken)) return 0

#define EXPECT_COMMA if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_COMMA))	return comp_error(CERR_EXPECTED_COMMA, &ctoken);
//#define EXPECT_BRACE_OPEN if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_BRACE_OPEN))	return comp_error(CERR_EXPECTED_BRACE_OPEN, &ctoken);
//#define EXPECT_BRACE_CLOSE if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_BRACE_CLOSE))	return comp_error(CERR_EXPECTED_BRACE_CLOSE, &ctoken);

extern struct identifierstruct identifier [IDENTIFIERS];
extern struct nshape_struct nshape [NSHAPES];
extern struct object_type_struct otype [OBJECT_TYPES];

extern struct cstatestruct cstate;

int declare_new_class(struct ctokenstruct* ctoken);
//int add_object_to_class(int member_index, int object_index, struct ctokenstruct* ctoken);
int finalise_template_class_lists(void);

struct fstatestruct
{
	struct template_struct* target_templ;

	int procdef_pos;
};

struct fstatestruct fstate;

/*
struct procdef_line_struct
{
	int object_type;
	int object_classes [CLASSES_PER_OBJECT];
	int object_angle;
	int component_type; // if object is a downlink, this is its type
};
#define PROCDEF_LINES (GROUP_MAX_MEMBERS*MAX_LINKS)
*/

struct procdef_struct procdef;

/*
contents of procdef buffer:

component type (+ if member 0, component angle)
 component objects:
  - object type
  - number of classes
   - index of each class (only up to the number of actual classes)
		- object angle
		- if object is a downlink, go back to component type.

don't really need to verify anything at this stage

Plan for rewriting this module:
 copy everything to procdef versions
  - change them so they parse source code and write to procdef instead of template
	then go back and change the old stuff so it parses the procdef in the template, instead of parsing source code


* most of the functions should be static!

*/

static int generate_template_from_procdef(int compile_mode);
int read_member_recursively(int parent_member_index, int parent_object, int parent_connection_index, int downlinks_from_core);
int read_member_objects_recursively(int member_index, int downlinks_from_core);
static int read_procdef(void);
static int procdef_error(const char* error_text);
int finalise_template_details(struct template_struct* finish_templ);

int	parse_process_definition(void);

// Reads the process structure definition from cstate.scode.
// Depending on the compiler mode, may discard process structure (but always processes structure to test it and to declare classes)
// Updates cstate with e.g. new scode_pos.
// returns 1 on success, 0 on failure
int	fix_template_design_from_scode(void)
{

	fstate.target_templ = cstate.templ;


 init_template_for_design(fstate.target_templ);

 if (!parse_process_definition())
		return 0;

	return generate_template_from_procdef(cstate.compile_mode);

}

/*
void test_src_lines(const char* nametext)
{

 int i;

 fpr("\n\n text: %s", nametext);

 for (i = 0; i < 30; i ++)
	{

			char tempstr [2];
			tempstr [0] = cstate.scode.text[i];
			tempstr [1] = 0;
			fpr("(%i:%i[%s]),", i, cstate.scode.src_line[i], tempstr);
	}

}
*/
// generates process structure definition from procdef that's already been filled in
// used when loading template files from disk
// doesn't need to use cstate
int fix_template_design_from_procdef(struct template_struct* target_templ)
{

	fstate.target_templ = target_templ;

 init_template_for_design(fstate.target_templ); // don't think this is necessary as the file loading functions will already have cleared the template

 if (!generate_template_from_procdef(COMPILE_MODE_FIX))
		return 0;

 strcpy(target_templ->source_edit->src_file_name, procdef.template_name);
 strcpy(target_templ->source_edit->src_file_path, procdef.template_name); // not sure about this

	target_templ->active = 1;

 return 1;


}


static int read_procdef(void)
{
	if (fstate.procdef_pos >= procdef.buffer_length)
		return 0; // this will probably cause an error.

	return procdef.buffer [fstate.procdef_pos++];
}


// This function can be called either from the compiler or from the template file loading functions
//  so don't use any compiler-related stuff.
// When loading a file, compile_mode should be COMPILE_MODE_FIX or maybe BUILD
static int generate_template_from_procdef(int compile_mode)
{

// struct ctokenstruct ctoken;
//fpr("\n read A(%i,%i) ", procdef.buffer[15], procdef.buffer[16]);
 strcpy(fstate.target_templ->name, procdef.template_name);

 fstate.procdef_pos = 0;

// now read in core shape:
//  (check for non-core shapes first because this is an obvious mistake to make)
 int core_shape = read_procdef();
#ifdef TEST_PROCDEF
 fpr("\nread core_shape %i (%i)", core_shape, fstate.procdef_pos);
#endif
 if (core_shape < 0
		|| core_shape >= FIRST_NONCORE_SHAPE)
		return procdef_error("invalid core type.");
// the error messages here are not super-helpful, but these errors shouldn't really occur
	fstate.target_templ->member[0].shape = core_shape;
	if (core_shape < FIRST_MOBILE_NSHAPE)
		fstate.target_templ->mobile = 0;
 	 else
  		fstate.target_templ->mobile = 1;
// Core angle offset
 int core_angle = read_procdef() & ANGLE_MASK;
#ifdef TEST_PROCDEF
 fpr("\nread core_angle %i (%i)", core_angle, fstate.procdef_pos);
#endif
	fstate.target_templ->member[0].connection_angle_offset_angle = core_angle;
	fstate.target_templ->member[0].group_angle_offset = int_angle_to_fixed(core_angle);
	fstate.target_templ->member[0].connection_angle_offset = fstate.target_templ->member[0].group_angle_offset;
//	fstate.target_templ->member[0].downlinks_from_core = 0; this is set by read_member_objects_recursively

// read objects:
 if (!read_member_objects_recursively(0, 0))
		return 0;

	if (compile_mode == COMPILE_MODE_TEST) // locked template
		return 1; // successful test.

	if (fstate.target_templ->locked)
	{
  fstate.target_templ->modified = 0; // design version of process should match source code version
		return 1; // if template locked, process header is parsed but ignored (except to get class name identifiers)
	}

 update_design_member_positions(fstate.target_templ);

 if (!finalise_template_class_lists())
		return 0;

 calculate_template_cost_and_power(fstate.target_templ);

 if (!finalise_template_details(fstate.target_templ))
		return 0;

 fstate.target_templ->modified = 0; // design version of process should match source code version

 return 1;

}

// Does some final fix-up stuff on a newly built template.
// For now, can't fail.
int finalise_template_details(struct template_struct* finish_templ)
{

 finish_templ->first_build_object_member = -1;
 finish_templ->first_build_object_link = -1;
 finish_templ->first_repair_object_member = -1;
 finish_templ->first_repair_object_link = -1;
	if (finish_templ->member[0].shape < FIRST_MOBILE_NSHAPE)
	 finish_templ->build_cooldown_cycles = finish_templ->data_cost / 4;//nshape[finish_templ->member[0].shape].build_or_restore_time;
	  else
	   finish_templ->build_cooldown_cycles = finish_templ->data_cost / 2;//nshape[finish_templ->member[0].shape].build_or_restore_time;
//	fpr("\n templ bct %i nsh %i", finish_templ->build_cooldown_cycles, nshape[finish_templ->member[0].shape].build_or_restore_time);

 finish_templ->has_allocator = 0;


 int i, j;
 int last_build_member = -1;
 int last_build_object = -1;
 int last_repair_member = -1;
 int last_repair_object = -1;

// int has_interface = 0; // is set to 1 below if at least 1 interface object is present



//fpr("\nfinishing template");

 for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (finish_templ->member[i].exists == 0)
			continue; // could probably break as they should be in order with no gaps, but nevermind

		finish_templ->member[i].interface_can_protect = 1; // this will be set to zero later if the member has a move object. If 1 doesn't guarantee that process has an interface

		for (j = 0; j < nshape[finish_templ->member[i].shape].links; j ++)
		{
			switch(finish_templ->member[i].object[j].type)
			{
				case OBJECT_TYPE_BUILD:
 				if (last_build_member == -1)
				 {
					 finish_templ->first_build_object_member = i;
 					finish_templ->first_build_object_link = j;
				 }
				  else
					 {
						 finish_templ->member[last_build_member].object[last_build_object].next_similar_object_member = i;
						 finish_templ->member[last_build_member].object[last_build_object].next_similar_object_link = j;
 					}
				 last_build_member = i;
				 last_build_object = j;
				 break; // end build object
				case OBJECT_TYPE_REPAIR:
				case OBJECT_TYPE_REPAIR_OTHER:
				 if (last_repair_member == -1)
				 {
					 finish_templ->first_repair_object_member = i;
 					finish_templ->first_repair_object_link = j;
				 }
				  else
					 {
						 finish_templ->member[last_repair_member].object[last_repair_object].next_similar_object_member = i;
						 finish_templ->member[last_repair_member].object[last_repair_object].next_similar_object_link = j;
 					}
				 last_repair_member = i;
				 last_repair_object = j;
				 break;
//				case OBJECT_TYPE_INTERFACE:
//					has_interface = 1;
//					break;
    case OBJECT_TYPE_INTERFACE:
				case OBJECT_TYPE_MOVE:
					finish_templ->member[i].interface_can_protect = 0;
					break;
				case OBJECT_TYPE_ALLOCATE:
					finish_templ->has_allocator = 1;
					break;

			} // end switch

		} // end for j (object/link loop)
	} // end for i (member loop)

// If necessary, terminate the lists:
	if (last_build_member != -1)
	{
		finish_templ->member[last_build_member].object[last_build_object].next_similar_object_member = -1;
		finish_templ->member[last_build_member].object[last_build_object].next_similar_object_link = -1;
//		fpr("\n finishing %i %i", last_build_member, last_build_object);
	}
	if (last_repair_member != -1)
	{
		finish_templ->member[last_repair_member].object[last_repair_object].next_similar_object_member = -1;
		finish_templ->member[last_repair_member].object[last_repair_object].next_similar_object_link = -1;
	}

// Note that the lists produced here will not be updated when components of a process are destroyed.
// So functions that do things like draw lines from all build objects to built target will need to skip over destroyed (but reserved) procs.

 return 1;

}


int read_member_objects_recursively(int member_index, int downlinks_from_core)
{

 int i, j;
 int links = nshape[fstate.target_templ->member[member_index].shape].links;
 fstate.target_templ->member[member_index].downlinks_from_core = downlinks_from_core;

// struct ctokenstruct ctoken;

// Object looks like this:
// {object_type, angle : class, class2} (plus possible comma)
// link object can look like this:
// {object_downlink, angle, {shape...}}

//* remember to update d_code.c so it generates the process in the right format!

//fpr("\n read_member_objects_recursively %i", member_index);

 for (i = 0; i < links; i ++)
	{
  int object_type = read_procdef();
//fpr("\n   obj %i %i", i, object_type);
#ifdef TEST_PROCDEF
 fpr("\nread object_type %i (%i)", object_type, fstate.procdef_pos);
#endif
  if (object_type < 0
			|| object_type >= OBJECT_TYPES)
				return procdef_error("expected object type");
	 fstate.target_templ->member[member_index].object[i].type = object_type;

	 int number_of_classes = read_procdef();
#ifdef TEST_PROCDEF
 fpr("\nread number of classes %i (%i)", number_of_classes, fstate.procdef_pos);
#endif
	 if (number_of_classes < 0
			|| number_of_classes >= CLASSES_PER_OBJECT)
			return procdef_error("wrong number of classes");
//fpr("\n number of classes %i: ", number_of_classes);
	 j = 0;
	 while (j < number_of_classes)
		{
			int class_index = read_procdef();
//			fpr("(%i:%i), ", j, class_index);
#ifdef TEST_PROCDEF
 fpr("\nread class_index %i (%i)", class_index, fstate.procdef_pos);
#endif
			if (class_index < 0
				|| class_index >= OBJECT_CLASSES)
				return procdef_error("invalid class index");
  	fstate.target_templ->member[member_index].object[i].object_class[j] = class_index;
  	j++;
		}

		int object_angle = read_procdef();
#ifdef TEST_PROCDEF
 fpr("\nread object_angle %i (%i)", object_angle, fstate.procdef_pos);
#endif
		if (object_angle < -ANGLE_4
			|| object_angle > ANGLE_4)
			return procdef_error("invalid object angle");

		if (otype[object_type].object_details.only_zero_angle_offset)
		{
//			fpr("\n   Fixing angle %i to 0 (otype %i)", object_angle, object_type);
			object_angle = 0; // some object types can only have zero offset
		}

	 fstate.target_templ->member[member_index].object[i].base_angle_offset_angle = object_angle;
//	fpr("\n read [%i] template %i member %i object %i angle_offset %i", fstate.procdef_pos, fstate.target_templ->template_index, member_index, i, fstate.target_templ->member[member_index].object[i].base_angle_offset_angle);

	 fstate.target_templ->member[member_index].object[i].base_angle_offset = angle_difference_signed(0, int_angle_to_fixed(object_angle));
//fpr("\n fix oa %i base_angle %i base_angle_f %f", object_angle, fstate.target_templ->member[member_index].object[i].base_angle_offset_angle, al_fixtof(fstate.target_templ->member[member_index].object[i].base_angle_offset));
	 if (object_type == OBJECT_TYPE_DOWNLINK)
		{
		 for (j = 1; j < GROUP_CONNECTIONS; j ++)
		 {
 			if (fstate.target_templ->member[member_index].connection[j].template_member_index == -1)
			 {
 		  if (!read_member_recursively(member_index, i, j, downlinks_from_core + 1))
			   return 0;
			  break;
			 }
		 }
	 if (j >= GROUP_CONNECTIONS)
			return comp_error_text("too many connections", NULL); // not sure this is possible (there should always be enough space in the connections array)
	}

  if (cstate.error != CERR_NONE)
			return 0;
	}

 return 1;

}


// call this for every member except the core
int read_member_recursively(int parent_member_index, int parent_object, int parent_connection_index, int downlinks_from_core)
{
	int child_member_index;
//	struct ctokenstruct ctoken;

	if (downlinks_from_core >= MAX_DOWNLINKS_FROM_CORE - 1)
	 return procdef_error("component too many downlinks away from core");

	for (child_member_index = parent_member_index + 1; child_member_index < GROUP_MAX_MEMBERS; child_member_index ++)
	{
		if (fstate.target_templ->member[child_member_index].exists == 0)
		 break;
	}
	if (child_member_index >= GROUP_MAX_MEMBERS)
	 return procdef_error("too many members");

 init_templ_group_member(fstate.target_templ, child_member_index);

// read member's shape:
 int member_nshape = read_procdef();
#ifdef TEST_PROCDEF
 fpr("\nread member_nshape %i (%i)", member_nshape, fstate.procdef_pos);
#endif
//  (check for core process shapees first because this is an obvious mistake to make)
 if (member_nshape < FIRST_NONCORE_SHAPE)
  return procdef_error("only the process core can be a core shape");
	if (member_nshape >= NSHAPES)
			return procdef_error("invalid process shape");
	fstate.target_templ->member[child_member_index].shape = member_nshape;

	fstate.target_templ->member[child_member_index].exists = 1;
// location etc can wait until later

	fstate.target_templ->member[child_member_index].connection[0].template_member_index = parent_member_index;
	fstate.target_templ->member[child_member_index].connection[0].reverse_link_index = parent_object;
	fstate.target_templ->member[child_member_index].connection[0].reverse_connection_index = parent_connection_index;
// the final part of the connection structure, link_index, will be filled in below after the member's uplink object is found
	fstate.target_templ->member[parent_member_index].connection[parent_connection_index].template_member_index = child_member_index;
	fstate.target_templ->member[parent_member_index].connection[parent_connection_index].link_index = parent_object;
	fstate.target_templ->member[parent_member_index].connection[parent_connection_index].reverse_connection_index = 0;
// reverse_link_index set below

 if (!read_member_objects_recursively(child_member_index, downlinks_from_core))
		return 0;


// now find the uplink:
 int i;
 int uplink_object_index = -1;
 for (i = 0; i < MAX_OBJECTS; i ++)
	{
		if (fstate.target_templ->member[child_member_index].object[i].type == OBJECT_TYPE_UPLINK)
		{
			if (uplink_object_index != -1)
				return procdef_error("member process has more than one uplink object");
			uplink_object_index = i;
		}
	}

	fstate.target_templ->member[child_member_index].connection[0].link_index = uplink_object_index;
	fstate.target_templ->member[parent_member_index].connection[parent_connection_index].reverse_link_index = uplink_object_index;

// The angle of the parent member's downlink object should be sufficient to work out where this member is:
 fstate.target_templ->member[child_member_index].connection_angle_offset_angle = fstate.target_templ->member[parent_member_index].object[parent_object].base_angle_offset_angle;
 fstate.target_templ->member[child_member_index].connection_angle_offset = int_angle_to_fixed(fstate.target_templ->member[child_member_index].connection_angle_offset_angle);


 return 1;

}


// call this function when a template has been finished and is ready to be put in the game.
// it reads the classes that each object has been assigned to (in the objects' object_class arrays)
//  and uses that to build the class lists in the main template struct.
//  - can be called from code and design
int finalise_template_class_lists(void)
{

 int i, j, k, m, class_index;
 char error_text [60];

// may as well start by clearing the lists:
 for (i = 0; i < OBJECT_CLASSES; i ++)
	{
  for (j = 0; j < OBJECT_CLASS_SIZE; j ++)
		{
			fstate.target_templ->object_class_member [i] [j] = -1;
			fstate.target_templ->object_class_object [i] [j] = -1;
		}
	}

// now go through each member, object, and object object_class
 for (i = 0; i < GROUP_MAX_MEMBERS; i ++)
	{
		if (fstate.target_templ->member[i].exists == 0)
			continue;
		for (j = 0; j < MAX_OBJECTS; j ++)
		{
 		if (fstate.target_templ->member[i].object[j].type == OBJECT_TYPE_NONE)
	 		continue;
			for (k = 0; k < CLASSES_PER_OBJECT; k ++)
			{
 		 if (fstate.target_templ->member[i].object[j].object_class [k] == -1)
	 		 continue;
	 		class_index = fstate.target_templ->member[i].object[j].object_class [k];
#ifdef SANITY_CHECK
if (class_index < 0 || class_index >= OBJECT_CLASSES)
{
	fpr("\nError: c_fix.c:finalise_template_class_lists(): invalid class_index %i for member %i object %i object_class %i", class_index, i, j, k);
	error_call();
}
#endif
		  for (m = 0; m < OBJECT_CLASS_SIZE; m ++)
				{
					if (fstate.target_templ->object_class_member [class_index] [m] == -1)
						break;
				}
				if (m == OBJECT_CLASS_SIZE)
				{
					snprintf(error_text, 60, "object class %s has too many objects (maximum is %i)", fstate.target_templ->object_class_name [class_index], OBJECT_CLASS_SIZE);
					return comp_error_text(error_text, NULL);
				}
				fstate.target_templ->object_class_active [class_index] = 1; // shouldn't be needed but can't hurt
				fstate.target_templ->object_class_member [class_index] [m] = i;
				fstate.target_templ->object_class_object [class_index] [m] = j;
			}
		}
	}

 return 1;

}


static int procdef_error(const char* error_text)
{

     start_log_line(MLOG_COL_ERROR);
     if (fstate.procdef_pos > 0)
					{
      write_to_log("Process definition error at line ");
      write_number_to_log(procdef.buffer_source_line [fstate.procdef_pos-1]);
					}
					 else
						{
       write_to_log("Process definition error.");
						}
     write_to_log(".");
     finish_log_line();

     start_log_line(MLOG_COL_ERROR);
     write_to_log("Error: ");
     write_to_log(error_text);
     write_to_log(".");
     finish_log_line();

     cstate.error = CERR_GENERIC;
     return 0;
}






































// PROCDEF stuff!!!!!!!!!!!!!!!!!!!

static int init_procdef(void);
static void write_to_procdef(s16b value);
static int procdef_read_member_objects_recursively(int shape_index);
static int procdef_read_member_recursively(void);//, int parent_connection_index)
static int procdef_declare_new_class(struct ctokenstruct* ctoken);


static int init_procdef(void)
{

 procdef.template_name [0] = '\0';

// int i;

// for (i = 0; i < OBJECT_CLASSES; i ++)
//	{
		//procdef.class_declared [i] = 0; - not used. Class stuff is worked out from object class details.
//		procdef.class_name [i] [0] = '\0'; - probably don't need this
//	}

	procdef.buffer_length = 0;
// could clear the buffer but that's probably not necessary

 return 1;

}


// Call this when compiling process definition part of source code.
// Reads the process structure definition from cstate.scode.
// Depending on the compiler mode, may discard process structure (but always processes structure to test it and to declare classes)
// Updates cstate with e.g. new scode_pos.
// returns 1 on success, 0 on failure
int	parse_process_definition(void)
{

/*fpr("\n A sc_pos %i src_line %i", cstate.scode_pos, cstate.src_line);

int k;

for (k = 0; k < 100; k ++)
{
	fpr("\n scode_pos %i src_line %i", k, cstate.scode.src_line [k]);
}
*/

	fstate.target_templ = cstate.templ;

 init_procdef();
//fpr("\n template %i A(%i,%i) ", cstate.templ->template_index, procdef.buffer [15],procdef.buffer [16]);
 init_template_for_design(fstate.target_templ);
//fpr("B(%i,%i) ", procdef.buffer [15],procdef.buffer [16]);

 struct ctokenstruct ctoken;

// the first thing in the scode should be #process
//  (deal with naming processes later)
	if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_HASH))
	{
//		fpr("\n # token type %i found", ctoken.type);
		return comp_error(CERR_FIXER_EXPECTED_PROCESS_HEADER, &ctoken);
	}

	if (!accept_next(&ctoken, CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD, KEYWORD_C_PROCESS))
	{
//		fpr("\n pr token type %i found", ctoken.type);
		return comp_error(CERR_FIXER_EXPECTED_PROCESS_HEADER, &ctoken);
	}

// accept template name
//  (this should be on the same line as #process, but technically doesn't have to be)
 if (accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_QUOTES))
	{
  char template_name_string [TEMPLATE_NAME_LENGTH];
  template_name_string [0] = '\0';

	 int read_char;
	 int template_name_length = 0;

	 while(TRUE)
	 {
 		read_char = c_get_next_char_from_scode();
		 if (read_char == REACHED_END_OF_SCODE)
  		return comp_error_text("reached end of source inside string", NULL);
		 if (read_char == 0)
  		return comp_error_text("found null character inside string?", NULL);

   if (template_name_length >= TEMPLATE_NAME_LENGTH - 2)
  		return comp_error_text("template name too long", NULL);

 	 if (read_char == '"')
  	 break;
 	 template_name_string [template_name_length] = read_char;
   template_name_length++;
	 };

  template_name_string [template_name_length] = '\0';

  strcpy(procdef.template_name, template_name_string);

	}
//fpr("C(%i,%i) ", procdef.buffer [15],procdef.buffer [16]);

// accept classes
 while(accept_next(&ctoken, CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD, KEYWORD_C_CLASS))
	{
		if (!read_next(&ctoken))
			return 0;
		while (TRUE)
		{
		 if (ctoken.type != CTOKEN_TYPE_IDENTIFIER_NEW)
 			return comp_error_text("expected new class name after class (word already in use?)", &ctoken);
 		if (!procdef_declare_new_class(&ctoken))
				return 0;
		 if (accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_SEMICOLON))
				break;
			if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_COMMA))
				return comp_error_text("expected ; or , after class declaration", &ctoken);
		};
	};
//fpr("D(%i,%i) ", procdef.buffer [15],procdef.buffer [16]);

// expect open brace:
//	if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_BRACE_OPEN))
//		return comp_error_text("expected open brace at start of process structure definition", &ctoken);

// now read in core shape:
//  (check for non-core shapes first because this is an obvious mistake to make)
	if (accept_next(&ctoken, CTOKEN_TYPE_IDENTIFIER_SHAPE, -1))
		return comp_error_text("the process core must be a core shape", &ctoken);
	if (!accept_next(&ctoken, CTOKEN_TYPE_IDENTIFIER_CORE_SHAPE, -1))
		return comp_error_text("expected a core shape", &ctoken);
//	fstate.target_templ->member[0].shape = identifier[ctoken.identifier_index].value;
 int core_shape = identifier[ctoken.identifier_index].value;
 write_to_procdef(core_shape);
#ifdef TEST_PROCDEF
 fpr("\nwrite core_shape %i (%i)", core_shape, procdef.buffer_length);
#endif
	if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_COMMA))
		return comp_error_text("expected comma after core shape", &ctoken);
// Core angle offset
 if (!expect_angle(&ctoken))
		return comp_error_text("core angle not a constant number?", &ctoken);
 write_to_procdef(ctoken.number_value);
#ifdef TEST_PROCDEF
 fpr("\nwrite core_angle %i (%i)", ctoken.number_value, procdef.buffer_length);
#endif

	if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_COMMA))
		return comp_error_text("expected comma after core angle", &ctoken);
// read objects:
 if (!procdef_read_member_objects_recursively(core_shape))
		return 0;

// expect close brace:
//  * actually read_member_objects_recursively should have read this.
	//if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_BRACE_CLOSE))
		//return comp_error_text("expected open brace at start of process structure definition", &ctoken);

// Finish by checking for #code directive
	if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_HASH))
		return comp_error(CERR_FIXER_EXPECTED_CODE_HEADER, &ctoken);
	if (!accept_next(&ctoken, CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD, KEYWORD_C_CODE))
		return comp_error(CERR_FIXER_EXPECTED_CODE_HEADER, &ctoken);
//fpr("E(%i,%i) ", procdef.buffer [15],procdef.buffer [16]);

 return 1;

}

//	if (cstate.compile_mode == COMPILE_MODE_TEST)
//		return 1; // successful test.

// should not proceed past here if target_templ is not dwindow.templ


//* write process definition to template here.


// update_design_member_positions(cstate.templ);  - not needed in procdef
/*
// Now, fix the template!
 int i;
// core should already have been set up. need to work out position of each linked process:
 for (i = 1; i < GROUP_CONNECTIONS; i ++) // note: begin from 1 because core has no parent
 {
 	if (fstate.target_templ->member[0].connection[i].template_member_index != -1)
			update_design_member_position_recursively(fstate.target_templ->member[0].connection[i].template_member_index);
 }
*/
/*
Will look like this:

#process thing

class movement;
class movement2;


core_basic_4, angle
{object_downlink, angle,
 {shape_basic_4,
  {object_move, angle : movement},
  {object_uplink},
  {object_downlink, angle,
   {shape_basic_4,
    {object_none},
    {object_uplink}
   }
  }
 },
 {object_none},
 {object_move, angle : movement, movement2}
}

#code


*/

// if (!finalise_template_class_lists())
//		return 0; - not needed in procdef

// calculate_template_cost(fstate.target_templ);  - not needed in procdef

// return 1;
//}


static void write_to_procdef(s16b value)
{
	if (procdef.buffer_length <= PROCDEF_BUFFER - 1)
	{
		procdef.buffer [procdef.buffer_length] = value;
		procdef.buffer_source_line [procdef.buffer_length] = cstate.src_line;
		procdef.buffer_length ++;
	}
// otherwise just ignore. Deal with excessively long procdef.buffer at the end.
}

static int procdef_read_member_objects_recursively(int shape_index)
{

 int i, j;
// maybe verify shape_index here?
 int links = nshape[shape_index].links;

 struct ctokenstruct ctoken;

// Object looks like this:
// {object_type, angle : class, class2} (plus possible comma)
// link object can look like this:
// {object_downlink, angle, {shape...}}

//* remember to update d_code.c so it generates the process in the right format!



 for (i = 0; i < links; i ++)
	{

// check for end of objects list:
 	if (accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_BRACE_CLOSE))
		{
   while (i < links)
			{
				write_to_procdef(OBJECT_TYPE_NONE);
#ifdef TEST_PROCDEF
 fpr("\nwrite object type %i (%i)", OBJECT_TYPE_NONE, procdef.buffer_length);
#endif
				write_to_procdef(0); // no classes
#ifdef TEST_PROCDEF
 fpr("\nwrite number_of_classes %i (%i)", 0, procdef.buffer_length);
#endif
				write_to_procdef(0); // angle
#ifdef TEST_PROCDEF
 fpr("\nwrite object_angle %i (%i)", 0, procdef.buffer_length);
#endif
				i++;
			}
	 	return 1;
		}

// expect open brace:
	if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_BRACE_OPEN))
		return comp_error_text("expected open brace at start of object", &ctoken);
// expect object type:
	if (!accept_next(&ctoken, CTOKEN_TYPE_IDENTIFIER_OBJECT, -1))
		return comp_error_text("expected object type", &ctoken);
	int object_type = identifier[ctoken.identifier_index].value;
	write_to_procdef(object_type);
#ifdef TEST_PROCDEF
 fpr("\nwrite object_type %i (%i)", object_type, procdef.buffer_length);
#endif
//	fstate.target_templ->member[member_index].object[i].type = identifier[ctoken.identifier_index].value;
	int classes_on_object = 0;
	int object_class [CLASSES_PER_OBJECT];
	while(accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_COLON))
	{
 	if (classes_on_object >= CLASSES_PER_OBJECT)
	 	return comp_error_text("objects can be members of maximum 4 classes", NULL);

		if (!read_next(&ctoken))
			return 0;
		if (ctoken.type != CTOKEN_TYPE_IDENTIFIER_CLASS)
			return comp_error_text("expected class name after colon", &ctoken);
		object_class [classes_on_object] =	identifier[ctoken.identifier_index].value;
		classes_on_object++;
	}
	write_to_procdef(classes_on_object);
#ifdef TEST_PROCDEF
 fpr("\nwrite classes_on_object %i (%i)", classes_on_object, procdef.buffer_length);
#endif
//	fpr("\nclasses on object: %i: ", classes_on_object);
	j = 0;
	while (j < classes_on_object)
	{
 	write_to_procdef(object_class [j]);
#ifdef TEST_PROCDEF
 fpr("\nwrite object_class %i (%i)", object_class [j], procdef.buffer_length);
#endif
		j ++;
	}
	if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_COMMA))
		return comp_error_text("expected , or : after object type or class", &ctoken);
	if (!expect_angle(&ctoken))
		return comp_error_text("expected object angle", &ctoken); // should accept } here for angle 0 (or unspecified, for objects without angles)
	if (ctoken.number_value < -ANGLE_4)
		return comp_error_text("object angle offset too low (minimum is -2048)", &ctoken);
	if (ctoken.number_value > ANGLE_4)
		return comp_error_text("object angle offset too high (maximum is 2048)", &ctoken);
	write_to_procdef(ctoken.number_value);
#ifdef TEST_PROCDEF
 fpr("\nwrite object_angle %i (%i)", ctoken.number_value, procdef.buffer_length);
#endif
//	fstate.target_templ->member[member_index].object[i].base_angle_offset_angle = ctoken.number_value;
//	fstate.target_templ->member[member_index].object[i].base_angle_offset = angle_difference_signed(0, int_angle_to_fixed(ctoken.number_value));

 if (object_type == OBJECT_TYPE_DOWNLINK)
	{
	 if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_COMMA))
		 return comp_error_text("expected comma after downlink object angle", &ctoken);
//		for (j = 1; j < GROUP_CONNECTIONS; j ++)
//		{
//			if (fstate.target_templ->member[member_index].connection[j].template_member_index == -1)
//			{
		  if (!procdef_read_member_recursively())
			  return 0;
//			 break;
//			}
		//}
//		if (j >= GROUP_CONNECTIONS)
//			return comp_error_text("too many connections", NULL); // not sure this is possible (there should always be enough space in the connections array)
	}

	 if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_BRACE_CLOSE))
	 	return comp_error_text("expected closing brace at end of object", &ctoken);

// finally, accept (but don't require) a comma:
 	accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_COMMA);
// could check for error here...
  if (cstate.error != CERR_NONE)
			return 0;
	}

 return 1; // explicit object found for all links, so we've finished here

}


// call this for every member except the core
static int procdef_read_member_recursively(void)//, int parent_connection_index)
{
//	int child_member_index;
	struct ctokenstruct ctoken;

// init_templ_group_member(fstate.target_templ, child_member_index);

// expect open brace:
	if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_BRACE_OPEN))
		return comp_error_text("expected open brace at start of process member", &ctoken);
// read member's shape:
//  (check for core process shapees first because this is an obvious mistake to make)
	if (accept_next(&ctoken, CTOKEN_TYPE_IDENTIFIER_CORE_SHAPE, -1))
		return comp_error_text("only the process core can be a core shape", &ctoken);
	if (!accept_next(&ctoken, CTOKEN_TYPE_IDENTIFIER_SHAPE, -1))
		return comp_error_text("expected process shape", &ctoken);
	int component_shape = identifier[ctoken.identifier_index].value;
	write_to_procdef(component_shape);
#ifdef TEST_PROCDEF
 fpr("\nwrite component_shape %i (%i)", component_shape, procdef.buffer_length);
#endif

//	fstate.target_templ->member[child_member_index].shape = identifier[ctoken.identifier_index].value;

// accept (but don't require) a comma:
	accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_COMMA);
// could check for error here...

//	fstate.target_templ->member[child_member_index].exists = 1;
// location etc can wait until later

//	fstate.target_templ->member[child_member_index].connection[0].template_member_index = parent_member_index;
//	fstate.target_templ->member[child_member_index].connection[0].reverse_link_index = parent_object;
//	fstate.target_templ->member[child_member_index].connection[0].reverse_connection_index = parent_connection_index;
// the final part of the connection structure, link_index, will be filled in below after the member's uplink object is found
//	fstate.target_templ->member[parent_member_index].connection[parent_connection_index].template_member_index = child_member_index;
//	fstate.target_templ->member[parent_member_index].connection[parent_connection_index].link_index = parent_object;
//	fstate.target_templ->member[parent_member_index].connection[parent_connection_index].reverse_connection_index = 0;
// reverse_link_index set below

 if (!procdef_read_member_objects_recursively(component_shape))
		return 0;

/*
// now find the uplink:
 int i;
 int uplink_object_index = -1;
 for (i = 0; i < MAX_OBJECTS; i ++)
	{
		if (fstate.target_templ->member[child_member_index].object[i].type == OBJECT_TYPE_UPLINK)
		{
			if (uplink_object_index != -1)
				return comp_error_text("member process has more than one uplink object", NULL);
			uplink_object_index = i;
		}
	}

	fstate.target_templ->member[child_member_index].connection[0].link_index = uplink_object_index;
	fstate.target_templ->member[parent_member_index].connection[parent_connection_index].reverse_link_index = uplink_object_index;

// The angle of the parent member's downlink object should be sufficient to work out where this member is:
 fstate.target_templ->member[child_member_index].connection_angle_offset_angle = fstate.target_templ->member[parent_member_index].object[parent_object].base_angle_offset_angle;
 fstate.target_templ->member[child_member_index].connection_angle_offset = int_angle_to_fixed(fstate.target_templ->member[child_member_index].connection_angle_offset_angle);
*/
	if (!accept_next(&ctoken, CTOKEN_TYPE_PUNCTUATION, CTOKEN_SUBTYPE_BRACE_CLOSE))
		return comp_error_text("expected closing brace at end of process member (too many objects?)", &ctoken);

//* not sure about these open/close braces

 return 1;

}

// This is a bit of a hack - it declares a class that will be used later when the template is being generated from the procdef.
static int procdef_declare_new_class(struct ctokenstruct* ctoken)
{
//fpr("\n procdef_declare_new_class(%s) scp %i srcl %i", identifier[ctoken->identifier_index].name, cstate.scode_pos, cstate.src_line);
	int i;

	for (i = 0; i < OBJECT_CLASSES; i ++)
	{
		if (fstate.target_templ->object_class_active [i] == 0)
			break;
	}

	if (i == OBJECT_CLASSES)
		return comp_error_text("too many classes declared (maximum 16)", ctoken);

	identifier[ctoken->identifier_index].type = CTOKEN_TYPE_IDENTIFIER_CLASS;
 identifier[ctoken->identifier_index].value = i;

 fstate.target_templ->object_class_active [i] = 1;
 if (strlen(identifier[ctoken->identifier_index].name) >= CLASS_NAME_LENGTH)
		return comp_error_text("class name too long (maximum 16 characters)", ctoken); // maximum length for an identifier is longer than maximum class name length
 strcpy(fstate.target_templ->object_class_name [i], identifier[ctoken->identifier_index].name);

 return 1;

}

/*
// this function designates an object as a member of a class.
// It does not add the object to the class lists in the main template (this must be done later)
//  - it does not guarantee that there is space for the object in the main template's class lists (see finalise_template_class_lists() for this)
int add_object_to_class(int member_index, int object_index, struct ctokenstruct* ctoken)
{

	int class_index = identifier[ctoken->identifier_index].value;
	int i;
	struct object_struct* declared_object = &fstate.target_templ->member[member_index].object[object_index];

	for (i = 0; i < CLASSES_PER_OBJECT; i ++)
	{
		if (declared_object->object_class[i] == -1)
			break;
	}
	if (i >= CLASSES_PER_OBJECT)
		return comp_error_text("objects can be members of maximum 4 classes", ctoken);

	declared_object->object_class[i] = class_index;

	return 1;

}
*/





static int derive_member_objects_recursively(struct template_struct* derive_templ, int member_index);


// Code to derive procdef from template (needed when saving and loading template file in binary form)
// Should probably only be called on a locked template, so we can be sure that the template's values are valid
// Also, the source line fields of procdef will be wrong, so don't use them after deriving.
int derive_procdef_from_template(struct template_struct* derive_templ)
{

	if (!derive_templ->active)
		return 0; // just to make sure

 init_procdef();

 strcpy(procdef.template_name, derive_templ->name);

	write_to_procdef(derive_templ->member[0].shape);
	write_to_procdef(derive_templ->member[0].connection_angle_offset_angle);

 derive_member_objects_recursively(derive_templ, 0);


 return 1;

}


static int derive_member_objects_recursively(struct template_struct* derive_templ, int member_index)
{

 int i, j;
 int links = nshape[derive_templ->member[member_index].shape].links;

 for (i = 0; i < links; i ++)
	{
  write_to_procdef(derive_templ->member[member_index].object[i].type);

  int number_of_classes = 0;
  int object_class [CLASSES_PER_OBJECT];

  for (j = 0; j < CLASSES_PER_OBJECT; j ++)
		{
			if (derive_templ->member[member_index].object[i].object_class[j] != -1)
			{
				object_class [j] = derive_templ->member[member_index].object[i].object_class[j];
				number_of_classes++;
			}
		}

		write_to_procdef(number_of_classes);
//fpr("\n number_of_classes %i: ", number_of_classes);
		if (number_of_classes > 0)
		{
		 for (j = 0; j < number_of_classes; j ++)
		 {
		 	write_to_procdef(object_class [j]);
//		 	fpr("(%i:%i), ", j, object_class [j]);
		 }
		}

		write_to_procdef(derive_templ->member[member_index].object[i].base_angle_offset_angle);
//		fpr("\n [%i] template %i member %i object %i angle_offset %i", procdef.buffer_length, derive_templ->template_index, member_index, i, derive_templ->member[member_index].object[i].base_angle_offset_angle);

		if (derive_templ->member[member_index].object[i].type == OBJECT_TYPE_DOWNLINK)
		{
			for (j = 1; j < GROUP_CONNECTIONS; j ++)
			{
				if (derive_templ->member[member_index].connection[j].link_index == i)
					break; // should be able to assume that this search will find the right link (as long as this is only ever called on valid templates)
			}
			write_to_procdef(derive_templ->member[derive_templ->member[member_index].connection[j].template_member_index].shape);
			derive_member_objects_recursively(derive_templ, derive_templ->member[member_index].connection[j].template_member_index);
		}

	}

 return 1;

}



