
#include <allegro5/allegro.h>
//#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"

#include "g_misc.h"
#include "g_world.h"

#include "c_header.h"
#include "e_slider.h"
#include "e_header.h"
#include "e_editor.h"
#include "e_log.h"
#include "e_files.h"

#include "c_fix.h"
#include "c_prepr.h"
#include "t_template.h"
#include "t_files.h"
#include "v_init_panel.h"



#define TFILE_BUFFER 30000
// is this enough? should probably test. Remember that this is 8 bits while a lot of template stuff is 16 or 32 bits

enum
{
TFILE_ERROR_NONE,
TFILE_ERROR_BUFFER, // out of space in buffer
TFILE_ERROR_TEMPLATE, // something wrong with template
};

extern struct tstatestruct tstate;
extern struct procdef_struct procdef;
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern ALLEGRO_DISPLAY* display;

struct tfilestruct
{

 char template_file_path [FILE_PATH_LENGTH]; // should this be in tstate?
 char template_file_buffer [TFILE_BUFFER];
 int template_file_buffer_pos;
 int error_state;
 struct template_struct* templ;
	int procdef_address [TEMPLATES_PER_PLAYER];
	int procdef_end [TEMPLATES_PER_PLAYER];
	int bcode_address [TEMPLATES_PER_PLAYER];
	int bcode_end [TEMPLATES_PER_PLAYER];

// specific to loading:
 int load_buffer_length;

};

static struct tfilestruct tfile;

/*

Structure of a template file:

for each template:
- the following 4 values are each s16b:
	- address of procdef - 0 means template empty
	- end address of procdef
	- address of bcode
	- end address of bcode



*/

static int build_template_file_buffer(int player_index);
static int write_bcode_to_tfile(void);
static int write_procdef_to_tfile(void);
static void write_tbuffer(char value);
static void write_tbuffer_s16b(s16b value);
static void write_tbuffer_32(int value);

static int read_procdef_from_tfile(int end_address);
static int read_bcode_from_tfile(struct template_struct* target_templ, int end_address);
static int read_tbuffer(void);
static int read_tbuffer_s16b(void);
static int read_tbuffer_32(void);


static int choose_template_file(int filechooser_mode);


void save_template_file(int player_index)
{

	tfile.error_state = TFILE_ERROR_NONE;

	if (!choose_template_file(ALLEGRO_FILECHOOSER_SAVE))
		return;

// now can assume that tfile.template_file_path holds name of target file
// try to open the file:

 FILE *file;

// open the file:
 file = fopen(tfile.template_file_path, "wb");

 if (!file)
 {
  write_line_to_log("Error: failed to open target file.", MLOG_COL_ERROR);
  return;
 }

// opened the file! So now we assemble the buffer:

	if (!build_template_file_buffer(player_index))
		return;

 if (tfile.error_state != TFILE_ERROR_NONE)
		return;

// now save the file:

 int written = fwrite(tfile.template_file_buffer, 1, tfile.template_file_buffer_pos, file);


 fclose(file);

 if (written != tfile.template_file_buffer_pos)
 {
//     fprintf(stdout, "\nError: buf_length %i written %i", buf_length, written);
     write_line_to_log("Error: file write failed.", MLOG_COL_ERROR);
     return;
 }

 write_line_to_log("Multi-binary file saved.", MLOG_COL_TEMPLATE);
 write_line_to_log("* Note: multi-binaries are for multiplayer games.", MLOG_COL_WARNING);
 write_line_to_log("        To save individual processes, use the file", MLOG_COL_WARNING);
 write_line_to_log("        menu in the editor panel.", MLOG_COL_WARNING);

// success!

}


static int build_template_file_buffer(int player_index)
{


	int i;

// player name?
	tfile.template_file_buffer_pos = TEMPLATES_PER_PLAYER*16; // leave space for tfile index (currently 4 ints per template)

	for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
	{
		tfile.templ = &templ[player_index][i];

		if (!tfile.templ->active)
		{
 		tfile.procdef_address [i] = 0;
 		tfile.procdef_end [i] = 0;
 		tfile.bcode_address [i] = 0;
 		tfile.bcode_end [i] = 0;
			continue;
		}
		if (!lock_template(tfile.templ)) // if template already locked this just returns 1
		{
			start_log_line(MLOG_COL_ERROR);
			write_to_log("Failed to lock template ");
			write_number_to_log(i);
			write_to_log(".");
			finish_log_line();
			return 0;
		}
		tfile.procdef_address [i] = tfile.template_file_buffer_pos;
		if (!write_procdef_to_tfile())
		{
			return 0;
		}
		tfile.procdef_end [i] = tfile.template_file_buffer_pos;
		tfile.bcode_address [i] = tfile.template_file_buffer_pos;
		if (!write_bcode_to_tfile())
		{
			return 0;
		}

 	if (tfile.error_state != TFILE_ERROR_NONE)
	 	return 0;
		tfile.bcode_end [i] = tfile.template_file_buffer_pos;
	}

	if (tfile.error_state != TFILE_ERROR_NONE)
		return 0;

// now reset the buffer pos to write to the index at the start of the buffer:
 int save_buffer_pos = tfile.template_file_buffer_pos; // hack

 tfile.template_file_buffer_pos = 0;

	for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
	{
		write_tbuffer_32(tfile.procdef_address [i]);
		write_tbuffer_32(tfile.procdef_end [i]);
		write_tbuffer_32(tfile.bcode_address [i]);
		write_tbuffer_32(tfile.bcode_end [i]);
// if anything changed here, need to change the space reserved for the index above.
	}

 tfile.template_file_buffer_pos = save_buffer_pos;


 return 1;

}

static int write_bcode_to_tfile(void)
{

	int i;
	int last_nonzero_bcode_address = BCODE_POS_MAX - 1; // this avoids the 8 stop instructions at the end

	while(last_nonzero_bcode_address > 0)
	{
		if (tfile.templ->bcode.op[last_nonzero_bcode_address] != 0)
			break;
		last_nonzero_bcode_address --;
	}

	last_nonzero_bcode_address += 4; // I don't really understand why this is needed, but it is
//fpr("\n Write template %i last_nonzero_bcode_address %i ", tfile.templ->template_index, last_nonzero_bcode_address);
	for (i = 0; i < last_nonzero_bcode_address; i ++)
	{
		write_tbuffer_s16b(tfile.templ->bcode.op[i]);
	}
/*
 if (tfile.templ->template_index == 1)
	{
		for (i = 0; i < 1000; i ++)
		{
			fpr("[%i:%i] ", i, tfile.templ->bcode.op[i]);
		}
	}*/

	return 1;

}

int write_procdef_to_tfile(void)
{

 if (!derive_procdef_from_template(tfile.templ))
	{
  write_line_to_log("Failed to derive process definition from template.", MLOG_COL_ERROR);
  return 0; // not sure this is possible (lock_template() should have failed earlier if there's a problem with the template)
	}

	int i = 0;

	while(procdef.template_name [i] != 0)
	{
		write_tbuffer(procdef.template_name [i++]);
// I'm pretty sure we can assume that this will be null-terminated within TEMPLATE_NAME_LENGTH
	}
	write_tbuffer(0); // terminates string

	for (i = 0; i < procdef.buffer_length; i ++)
	{
		write_tbuffer_s16b(procdef.buffer [i]);
	}
//fpr("\n procdef.buffer_length %i tfile.template_file_buffer_pos %i\n", procdef.buffer_length, tfile.template_file_buffer_pos);
	if (tfile.error_state != TFILE_ERROR_NONE)
		return 0;

	return 1;

}




static void write_tbuffer(char value)
{
	if (tfile.template_file_buffer_pos >= TFILE_BUFFER - 3)
	{
		tfile.error_state = TFILE_ERROR_BUFFER;
//		fpr("\n write_tbuffer out of room");
		return;
	}
	tfile.template_file_buffer [tfile.template_file_buffer_pos] = value;
	tfile.template_file_buffer_pos++;
}

static void write_tbuffer_s16b(s16b value)
{

	write_tbuffer(value >> 8);
	write_tbuffer(value & 255);

}

static void write_tbuffer_32(int value)
{

	write_tbuffer((value >> 24) & 255);
	write_tbuffer((value >> 16) & 255);
	write_tbuffer((value >> 8) & 255);
	write_tbuffer(value & 255);


}


/*

void load_int_unchecked(int* value, const char* name)
{
 int loaded [4];
// char cvalues [4];

 loaded[0] = load_8b(name);
 loaded[1] = load_8b(name);
 loaded[2] = load_8b(name);
 loaded[3] = load_8b(name);

 *value = ((int) loaded[0] << 24) | ((int) loaded[1] << 16) | ((int) loaded[2] << 8) | ((int) loaded[3]);

}


*/




void load_template_file(int player_index)
{

// first make sure none of the player's existing templates are locked:
	int i;

	for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
	{
		if (templ[player_index][i].active
			&& templ[player_index][i].locked)
		{
   write_line_to_log("Can't load multi-binary file while any templates are locked.", MLOG_COL_ERROR);
   return;
		}
	}

	tfile.error_state = TFILE_ERROR_NONE;
 tfile.template_file_buffer_pos = 0;

	if (!choose_template_file(ALLEGRO_FILECHOOSER_FILE_MUST_EXIST))
		return;

// now can assume that tfile.template_file_path holds name of target file
// try to open the file:

 FILE *file;

// open the file:
 file = fopen(tfile.template_file_path, "rb");

 if (!file)
 {
  write_line_to_log("Error: failed to open target file.", MLOG_COL_ERROR);
  return;
 }


// So now we read the buffer:

    tfile.load_buffer_length = fread(tfile.template_file_buffer, 1, TFILE_BUFFER, file);

    if (ferror(file) || tfile.load_buffer_length == 0)
    {
     fclose(file);
     write_line_to_log("Error: Couldn't read multi-binary file.", MLOG_COL_ERROR);
     return;
    }

    if (!feof(file))
    {
     fclose(file);
     write_line_to_log("Error: EOF not found (file too long?).", MLOG_COL_ERROR);
     return;
    }

    fclose(file);

// now we read the index, which should be at the start of the file:
    for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
				{
					tfile.procdef_address [i] = read_tbuffer_32();
//					fpr("W(%i)", tfile.procdef_address [i]);
					if (tfile.procdef_address [i] < 0 // 0 indicates empty template
						|| tfile.procdef_address [i] >= tfile.load_buffer_length)
					{
      write_line_to_log("Error: invalid procdef start address.", MLOG_COL_ERROR);
      return;
					}
					tfile.procdef_end [i] = read_tbuffer_32();
					if (tfile.procdef_end [i] < 0 // 0 indicates empty template
						|| tfile.procdef_end [i] >= tfile.load_buffer_length)
					{
      write_line_to_log("Error: invalid procdef end address.", MLOG_COL_ERROR);
      return;
					}
					tfile.bcode_address [i] = read_tbuffer_32();
					if (tfile.bcode_address [i] < 0 // 0 indicates empty template
						|| tfile.bcode_address [i] >= tfile.load_buffer_length)
					{
      write_line_to_log("Error: invalid bcode start address.", MLOG_COL_ERROR);
      return;
					}
					tfile.bcode_end [i] = read_tbuffer_32();
					if (tfile.bcode_end [i] < 0 // 0 indicates empty template
						|| tfile.bcode_end [i] > tfile.load_buffer_length)
					{
      write_line_to_log("Error: invalid bcode end address.", MLOG_COL_ERROR);
      return;
					}
				}
//return;
// We'll probably succeed now, so clear the player's existing templates:

	for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
	{
  clear_template_but_not_source(&templ[player_index][i]);
  clear_source_edit_struct(templ[player_index][i].source_edit);
	}



// now read in the templates:
    for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
				{
					if (tfile.procdef_address [i] == 0)
					{
						templ[player_index][i].active = 0;
						continue; // this doesn't check for bcode address. But if procdef_address is 0 the template lock should have failed anyway.
					}
					if (!read_procdef_from_tfile(tfile.procdef_end [i]))
						return;
					if (!fix_template_design_from_procdef(&templ[player_index][i]))
						return;
					if (!read_bcode_from_tfile(&templ[player_index][i], tfile.bcode_end [i]))
						return;

    	templ[player_index][i].active = 1;
	    templ[player_index][i].locked = 1;
	    templ[player_index][i].modified = 0; // this was probably done in one of the functions above but can't hurt to do it again
     templ[player_index][i].source_edit->active = 1;
     templ[player_index][i].source_edit->saved = 1;
     strcpy(templ[player_index][i].source_edit->text [0], "// this file was loaded from a multi-binary file, ");
     strcpy(templ[player_index][i].source_edit->text [1], "//  and does not have source code. ");
      // clear_source_edit_struct() call above reset the line index, so text[0] is the first line

     prepare_template_debug(player_index, i, 0); // sets up the BC panel information. ,0 means no identifier (variable and label name) information will be included.
				}


 write_line_to_log("Multi-binary file loaded.", MLOG_COL_TEMPLATE);
 write_line_to_log("* Note: multi-binaries are for multiplayer games.", MLOG_COL_WARNING);
 write_line_to_log("        To load individual processes, load their .c files", MLOG_COL_WARNING);
 write_line_to_log("        using the file menu in the editor panel.", MLOG_COL_WARNING);
// success!

}


// not sure if this function needs to use start address.
// should be able to assume that the tfile position is already in the right place.
static int read_procdef_from_tfile(int end_address)
{

	int i;

	for (i = 0; i < TEMPLATE_NAME_LENGTH; i++)
	{
		procdef.template_name [i] = read_tbuffer();
		if (procdef.template_name [i] == 0)
			break;
	}
	if (i == TEMPLATE_NAME_LENGTH)
	{
  write_line_to_log("Template name in procdef not null-terminated?", MLOG_COL_ERROR);
  return 0;
	}

	for (i = 0; i < PROCDEF_BUFFER; i ++)
	{
		procdef.buffer [i] = read_tbuffer_s16b();
		procdef.buffer_source_line [i] = 0;
		if (tfile.template_file_buffer_pos >= end_address)
			break;
	}

	procdef.buffer_length = i+1;
//	fpr("\n template buffer pos %i", procdef.buffer_length);
	return 1;

}



// not sure if this function needs to use start address.
// should be able to assume that the tfile position is already in the right place.
static int read_bcode_from_tfile(struct template_struct* target_templ, int end_address)
{

	int i;

	for (i = 0; i < BCODE_MAX; i ++)
	{
		target_templ->bcode.op [i] = read_tbuffer_s16b();
		if (tfile.template_file_buffer_pos >= end_address)
			break;
	}

// fill bcode out with 0 (nop)
	while(i < BCODE_MAX)
	{
		target_templ->bcode.op [i++] = 0;
	}

	for (i = BCODE_POS_MAX; i < BCODE_MAX; i ++)
	{
		target_templ->bcode.op [i] = OP_stop;
	}

/*
 if (target_templ->template_index == 1)
	{
		for (i = 0; i < 1000; i ++)
		{
			fpr("[%i:%i] ", i, target_templ->bcode.op[i]);
		}
	}
*/

	return 1;

}



static int read_tbuffer(void)
{
	if (tfile.template_file_buffer_pos >= TFILE_BUFFER - 3)
	{
		tfile.error_state = TFILE_ERROR_BUFFER;
		return 0; // will probably cause an error that will be caught eventually
	}

	return tfile.template_file_buffer [tfile.template_file_buffer_pos++] & 0xff;
}

static int read_tbuffer_s16b(void)
{

 s16b read_value [2];
 read_value [0] = read_tbuffer();
 read_value [1] = read_tbuffer();

 return ((read_value [0] & 0xff) << 8) | (read_value [1]);


}

static int read_tbuffer_32(void)
{

 int read_value [4];
 read_value [0] = read_tbuffer();
 read_value [1] = read_tbuffer();
 read_value [2] = read_tbuffer();
 read_value [3] = read_tbuffer();

//fpr("\n rv[%i,%i,%i,%i]", read_value [0], read_value [1], read_value [2], read_value [3]);

//fpr("\n rtb32(%i) ", ((read_value [0] & 0xff) << 24) | ((read_value [1] & 0xff) << 16) | ((read_value [2] & 0xff) << 8) | (read_value [3]));

 return ((int) read_value [0] << 24) | ((int) read_value [1] << 16) | ((int) read_value [2] << 8) | ((int) read_value [3]);


}





// uses the Allegro native file chooser functions
// filechooser_mode should be ALLEGRO_FILECHOOSER_FILE_MUST_EXIST or ALLEGRO_FILECHOOSER_SAVE
// if returns 1 (success), can assume that template_file_path global variable contains name of a tf file (which may or may not exist - filechooser_mode may not be reliable - so needs to be verified later)
static int choose_template_file(int filechooser_mode)
{

 ALLEGRO_FILECHOOSER* file_dialog = al_create_native_file_dialog("", "Choose file", "*.tf",  filechooser_mode);

 if (file_dialog == NULL)
 {
  write_line_to_log("Error: couldn't open Allegro file dialog!", MLOG_COL_ERROR);
  return 0; // this should probably be an error_call()
 }

 al_show_mouse_cursor(display);
 al_show_native_file_dialog(display, file_dialog); // this should block everything else until it finishes.
 al_hide_mouse_cursor(display);

 flush_game_event_queues(); // opening may have taken some time

 int files_to_open = al_get_native_file_dialog_count(file_dialog);

 if (files_to_open == 0)
 {
 	goto choose_fail;
 }

 if (files_to_open > 1)
 {
  write_line_to_log("Error: Can only open one file at a time, sorry.", MLOG_COL_ERROR);
 	goto choose_fail;
 }

 const char* file_path_ptr = al_get_native_file_dialog_path(file_dialog, 0);

 if (strlen(file_path_ptr) >= FILE_PATH_LENGTH) // not sure this is needed
 {
  write_line_to_log("Error: File path too long, sorry.", MLOG_COL_ERROR);
 	goto choose_fail;
 }

 int file_type = get_file_type_from_name(file_path_ptr);

 if (file_type != FILE_TYPE_TEMPLATE)
 {
  write_line_to_log("Error: Must be a multi-binary template file with the .tf extension.", MLOG_COL_ERROR);
  return 0;
 }

 strcpy(tfile.template_file_path, file_path_ptr);
 al_destroy_native_file_dialog(file_dialog);
 return 1;

 choose_fail:
  al_destroy_native_file_dialog(file_dialog);
  return 0;

}


// called at startup. Loads any templates specified as default templates by init.txt.
void load_default_templates(void)
{

	int i, j;

	for (i = 0; i < PLAYERS; i ++)
	{
		for (j = 0; j < TEMPLATES_PER_PLAYER; j ++)
		{
			if (settings.default_template_path [i] [j] [0] == 0)
				continue; // no default for this template
			load_source_file_into_template(settings.default_template_path [i] [j], i, j);
		}
	}


}




