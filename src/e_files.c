
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"

#include "g_misc.h"

#include "c_header.h"
#include "e_slider.h"

#include "e_header.h"
#include "e_inter.h"
#include "e_clip.h"
#include "i_input.h"
#include "i_view.h"
#include "m_input.h"
#include "e_log.h"
#include "e_editor.h"

#include "c_prepr.h"
#include "d_draw.h"
#include "t_template.h"


extern ALLEGRO_DISPLAY* display;
extern struct editorstruct editor;
extern struct design_window_struct dwindow;
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct game_struct game;

int open_file_into_source_or_binary(struct source_struct* src);
int check_file_already_open(const char* file_path);
void save_source_edit_file(struct source_edit_struct* se);
void give_source_edit_name_to_tab(int tab, struct source_edit_struct* se);
int get_file_type_from_name(const char* file_path);
int save_as(void);
int new_empty_source_tab(void);
int find_last_nonzero_bcode_op(struct bcode_struct* bcode);

// WRITE_SIZE is the max length of a file buffer for saving (doesn't include the padding at the end of a line that exists in the se.text array)
//#define WRITE_SIZE 8192
#define WRITE_SIZE 100000

void save_source_buffer_to_file(char buffer [WRITE_SIZE], int buf_length, struct source_edit_struct* se);
void save_binary_buffer_to_file(s16b buffer [WRITE_SIZE], int buf_length, struct source_edit_struct* se);
int source_edit_to_source_buffer(char buffer [WRITE_SIZE], struct source_edit_struct* se);
int source_edit_to_binary_buffer(s16b buffer [WRITE_SIZE], struct source_edit_struct* se);
int save_source_edit_source_to_file(struct source_edit_struct* se);
int save_source_edit_binary_to_file(struct source_edit_struct* se);

void remove_source_tab(int tab, struct source_edit_struct* se, int se_index);
void close_source_tab(int tab, int force_close);

void init_editor_files(void)
{



// This code may be needed if a version of Allegro after 5.0.9 or 5.1.0 is being used. My version doesn't have it

 if (!al_init_native_dialog_addon())
 {
  fprintf(stdout, "\nError: failed to initialise Allegro's native dialog addon.");
  error_call();
 }

}


void open_file_into_current_source_edit(void)
{


 int sei = editor.current_source_edit_index;

// at this point, tab is the tab_index entry and sei is the source_edit entry

 struct source_struct src;

 int opened = open_file_into_source_or_binary(&src);

 switch(opened)
 {
  case OPEN_FAIL:
  case OPENED_ALREADY:
  default: // shouldn't be possible but do default anyway
//   close_source_tab(tab, 1); // 1 is force close (don't ask about saving)
   return;

  case OPENED_SOURCE:
// init_source_edit_struct(&editor.source_edit [sei]); unneeded - source_to_editor() does this
   if (!dwindow.templ->locked)
		  open_new_template(dwindow.templ);
   source_to_editor(&src, sei);
   editor.source_edit[sei].from_a_file = 1;
   editor.source_edit[sei].saved = 1;

//   editor.tab_index [tab] = sei;
//   editor.tab_type [tab] = TAB_TYPE_SOURCE;
// copy the file name to the tab name:
//   give_source_edit_name_to_tab(tab, &editor.source_edit [sei]);

//   open_tab(tab);

//   fprintf(stdout, "\nOpened source to tab %i sei %i", tab, sei);
   break;

  case OPENED_BINARY:
//   binary_to_editor(&editor.source_edit[sei].bcode, sei);

//   editor.tab_index [tab] = sei;
//   editor.tab_type [tab] = TAB_TYPE_BINARY;
// copy the file name to the tab name:
//   give_source_edit_name_to_tab(tab, &editor.source_edit [sei]);

//   open_tab(tab);

//   fprintf(stdout, "\nOpened binary to tab %i sei %i", tab, sei);
   break;


 }


 return;

}

// tries to open a file (by using a native file dialog box) into a free tab
// returns -1 on failure (error, user cancel or file already open)
// returns index in tab_index on success
int open_file_into_free_tab(void)
{
/*
 int tab = new_empty_source_tab();

 if (tab == -1)
  return -1;

 int sei = editor.tab_index [tab];

// at this point, tab is the tab_index entry and sei is the source_edit entry

 struct source_struct src;

 int opened = open_file_into_source_or_binary(&src, &editor.source_edit[sei].bcode, 1);

 switch(opened)
 {
  case OPEN_FAIL:
  case OPENED_ALREADY:
  default: // shouldn't be possible but do default anyway
   close_source_tab(tab, 1); // 1 is force close (don't ask about saving)
   return -1;

  case OPENED_SOURCE:
// init_source_edit_struct(&editor.source_edit [sei]); unneeded - source_to_editor() does this
   source_to_editor(&src, sei);
   editor.source_edit[sei].from_a_file = 1;
   editor.source_edit[sei].saved = 1;

   editor.tab_index [tab] = sei;
   editor.tab_type [tab] = TAB_TYPE_SOURCE;
// copy the file name to the tab name:
   give_source_edit_name_to_tab(tab, &editor.source_edit [sei]);

   open_tab(tab);

//   fprintf(stdout, "\nOpened source to tab %i sei %i", tab, sei);
   break;

  case OPENED_BINARY:
//   binary_to_editor(&editor.source_edit[sei].bcode, sei);

   editor.tab_index [tab] = sei;
   editor.tab_type [tab] = TAB_TYPE_BINARY;
// copy the file name to the tab name:
   give_source_edit_name_to_tab(tab, &editor.source_edit [sei]);

   open_tab(tab);

//   fprintf(stdout, "\nOpened binary to tab %i sei %i", tab, sei);
   break;


 }


 return tab;
*/
return 1;
}

// called when a tab is given a name by being opened or saved-as
// se's src_file_name must be valid
void give_source_edit_name_to_tab(int tab, struct source_edit_struct* se)
{
/*
 strncpy(editor.tab_name [tab], se->src_file_name, TAB_NAME_LENGTH - 1);
 editor.tab_name [tab] [TAB_NAME_LENGTH - 1] = '\0'; // strncpy doesn't null terminate if source is longer than the number of chars copied
 strcpy(editor.tab_name_unsaved [tab], "*");
 if (strlen(editor.tab_name [tab]) == TAB_NAME_LENGTH - 1)
 {
  strncat(editor.tab_name_unsaved [tab], editor.tab_name [tab], TAB_NAME_LENGTH - 2);
  editor.tab_name_unsaved [tab] [TAB_NAME_LENGTH - 1] = '\0';
 }
  else
   strcat(editor.tab_name_unsaved [tab], editor.tab_name [tab]);
*/

}


// uses the Allegro native file chooser functions
// is called by both editor file open (with check_editor_tabs = 1) and template file open (with check_editor_tabs = 0)
//  also called by mission file functions (also with check_editor_tabs = 0)
// returns:
//  OPEN_FAIL (-1) on failure
//  OPENED_SOURCE (-2) if .c file successfully opened
//  OPENED_BINARY if .bc file successfully opened
//  OPENED_ALREADY (-3) if check_editor_tabs == 1 and the file is already open (also switches the editor to the appropriate tab)
int open_file_into_source_or_binary(struct source_struct* src) //, struct bcode_struct* bcode)//, int check_editor_tabs)
{

// ALLEGRO_FILECHOOSER* file_dialog = al_create_native_file_dialog("", "Choose file", "*.c;*.bc",  ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
 ALLEGRO_FILECHOOSER* file_dialog = al_create_native_file_dialog("", "Choose file", "*.c",  ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);

 if (file_dialog == NULL)
 {
  write_line_to_log("Error: couldn't open Allegro file dialog!", MLOG_COL_ERROR);
  return OPEN_FAIL; // this should probably be an error_call()
 }

 al_show_mouse_cursor(display);
 al_show_native_file_dialog(display, file_dialog); // this should block everything else until it finishes.
 al_hide_mouse_cursor(display);

 flush_game_event_queues(); // opening may have taken some time

 int files_to_open = al_get_native_file_dialog_count(file_dialog);

 if (files_to_open == 0)
 {
  al_destroy_native_file_dialog(file_dialog);
  return OPEN_FAIL;
 }

 if (files_to_open > 1)
 {
  write_line_to_log("Can only open one file at a time, sorry.", MLOG_COL_ERROR); // not sure this is necessary if the multiple file flag isn't set for al_create_native_file_dialog()
  al_destroy_native_file_dialog(file_dialog);
  return OPEN_FAIL;
 }

 const char* file_path_ptr = al_get_native_file_dialog_path(file_dialog, 0);

// fprintf(stdout, "\nFile number %i path (%s)", files_to_open, file_path_ptr);

 if (strlen(file_path_ptr) >= FILE_PATH_LENGTH) // not sure this is needed
 {
  write_line_to_log("File path too long, sorry.", MLOG_COL_ERROR);
  al_destroy_native_file_dialog(file_dialog);
  return OPEN_FAIL;
 }

 int file_type = get_file_type_from_name(file_path_ptr);

 switch(file_type)
 {
  case FILE_TYPE_SOURCE:
// load the file into the src struct
   if (!load_source_file(file_path_ptr, src))
   {
    write_line_to_log("Couldn't open source file.", MLOG_COL_ERROR);
    al_destroy_native_file_dialog(file_dialog);
    return OPEN_FAIL;
   }
   return OPENED_SOURCE;
  default:
   write_line_to_log("File must be a .c source code file.", MLOG_COL_ERROR);
   break;
/*  case FILE_TYPE_BINARY:
// load the file into the bcode struct
   if (!load_binary_file(file_path_ptr, bcode, 0, 0))
   {
    write_line_to_log("Couldn't open binary file.", MLOG_COL_ERROR);
    al_destroy_native_file_dialog(file_dialog);
    return OPEN_FAIL;
   }
   al_destroy_native_file_dialog(file_dialog);
   return OPENED_BINARY;*/
 }

 al_destroy_native_file_dialog(file_dialog);
 return OPEN_FAIL;

}


// returns FILE_TYPE_ERROR, FILE_TYPE_SOURCE, FILE_TYPE_BINARY
// assumes file_path is a valid string, but not that it's in any particular format
int get_file_type_from_name(const char* file_path)
{

 int length = strlen(file_path);

 if (length < 3)
  return FILE_TYPE_ERROR;

 if (file_path [length - 1] == 'f'
  && file_path [length - 2] == 't'
  && file_path [length - 3] == '.') // *.tf file
   return FILE_TYPE_TEMPLATE;


 if (file_path [length - 1] != 'c')
  return FILE_TYPE_ERROR;

 if (file_path [length - 2] == '.')
  return FILE_TYPE_SOURCE;

 if (file_path [length - 2] == 'b'
  && file_path [length - 3] == '.')
   return FILE_TYPE_BINARY;

 return FILE_TYPE_ERROR;

}


// checks whether file file_path is already open in a tab in the editor
// returns -1 if not already open
// returns tab index if already open
int check_file_already_open(const char* file_path)
{
/*
 int i;
 struct source_edit_struct* se;

 for (i = 0; i < ESOURCES; i ++)
 {
  if (editor.tab_type [i] == TAB_TYPE_SOURCE
   || editor.tab_type [i] == TAB_TYPE_BINARY)
  {
   se = &editor.source_edit [editor.tab_index [i]];
   if (se->from_a_file == 0)
    continue; // probably a new, unsaved file
   if (strcmp(file_path, se->src_file_path) == 0) // match found!
    return i;
  }
 }*/

 return -1;

}


// creates an empty source file in a new tab.
// the file's source_edit->src_file_path will be empty; it's src_file_name will be "untitled.c"
// fails (with a log message) if there's no space left
// returns -1 on failure, or new tab index on success
int new_empty_source_tab(void)
{
/*
// make sure there's room:
 if (editor.tab_type [ESOURCES - 1] != TAB_TYPE_NONE)
 {
  write_line_to_log("Too many files open.", MLOG_COL_ERROR);
  return -1;
 }

// find first empty space in the tab index
 int tab;

 for (tab = 0; tab < ESOURCES; tab ++)
 {
  if (editor.tab_type [tab] == TAB_TYPE_NONE)
   break;
 }

// find first empty source_edit_struct
 int sei;

 for (sei = 0; sei < ESOURCES; sei ++)
 {
  if (editor.source_edit [sei].active == 0)
   break;
 }

// now can assume that tab and sei are valid

 struct source_edit_struct* se = &editor.source_edit [sei];

 init_source_edit_struct(se); // this initialises the names, so this call must come before they are initialised below

 se->active = 1;
 se->from_a_file = 0;
 se->saved = 0;
 strcpy(se->src_file_name, "untitled");
 strcpy(se->src_file_path, "");

 editor.tab_index [tab] = sei;
 editor.tab_type [tab] = TAB_TYPE_SOURCE;
 strcpy(editor.tab_name [tab], "untitled");
 strcpy(editor.tab_name_unsaved [tab], "*untitled");

// open_tab(tab); - don't do this - the calling function should do this, but only on success

 return tab;
*/
return 0;
}

void save_current_file(void)
{

 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
		|| se->active == 0
  || se->type != SOURCE_EDIT_TYPE_SOURCE)
	{
		write_line_to_log("Couldn't save; no current source file.", MLOG_COL_ERROR);
  return;
	}

#ifndef DEBUG_MODE
 if (game.type == GAME_TYPE_MISSION
	 && se->player_index == 1)
	{
		write_line_to_log("You can't save your opponent's files in story mode!", MLOG_COL_ERROR);
		return;
	}
#endif

 if (se->saved == 1)
	{
		write_line_to_log("File already saved.", MLOG_COL_EDITOR);
  return;
	}

// if it isn't from a file (e.g. it's a newly created source file) call save-as instead:
 if (se->from_a_file == 0)
 {
  save_as(); // handles both binary and source
  return;
 }


 save_source_edit_file(se);

}

// tries to save all files for currently open player
void save_all_files(void)
{

	int saving_player = dwindow.templ->player_index;

	sancheck(saving_player, 0, PLAYERS, "save_all_files: saving_player");

#ifndef DEBUG_MODE
 if (game.type == GAME_TYPE_MISSION
	 && saving_player == 1)
	{
		write_line_to_log("You can't save your opponent's files in story mode!", MLOG_COL_ERROR);
		return;
	}
#endif

	int i;

	for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
	{
		if (!templ[saving_player][i].active
   || !templ[saving_player][i].source_edit->active
   || templ[saving_player][i].source_edit->saved)
		 continue;
		if	(templ[saving_player][i].source_edit->type != SOURCE_EDIT_TYPE_SOURCE)
		{
			start_log_line(MLOG_COL_WARNING);
 		write_to_log("Template ");
 		write_number_to_log(i);
 		write_to_log(" not saved (no source code).");
 		finish_log_line();
		 continue;
		}
		if	(templ[saving_player][i].source_edit->from_a_file == 0)
		{
			start_log_line(MLOG_COL_WARNING);
 		write_to_log("Template ");
 		write_number_to_log(i);
 		write_to_log(" not saved (not from a file).");
 		finish_log_line();
		 continue;
		}

		 save_source_edit_file(templ[saving_player][i].source_edit);

	}



}

// assumes se is a valid source_edit_struct

void save_source_edit_file(struct source_edit_struct* se)
{

// now we know that se is a valid source_edit.
// first make sure it's not saved:

 save_source_edit_source_to_file(se);

/*
 switch(se->type)
 {
  case SOURCE_EDIT_TYPE_SOURCE:
   save_source_edit_source_to_file(se);
   break;
  case SOURCE_EDIT_TYPE_BINARY:
   save_source_edit_binary_to_file(se);
   break;
 }
*/


/*
 switch(se->type)
 {
  case SOURCE_EDIT_TYPE_SOURCE:
   ;
   char buffer [WRITE_SIZE];
// the file is actually opened after the code to build the write buffer (to avoid wiping the file when the write buffer can't be built for some reason)
   int buf_length = source_edit_to_source_buffer(buffer, se);
   if (buf_length == -1)
    return; // failed; source_edit_to_buffer will have provided an error message to the log
// fprintf(stdout, "\nSaving: path (%s) total_src_lines %i buf_pos %i", se->src_file_path, total_src_lines, buf_pos);
// fprintf(stdout, "%s", buffer);
   save_source_buffer_to_file(buffer, buf_length, se);
   break;
  case SOURCE_EDIT_TYPE_BINARY:
   ;
   short sbuffer [WRITE_SIZE];
// the file is actually opened after the code to build the write buffer (to avoid wiping the file when the write buffer can't be built for some reason)
   int sbuf_length = source_edit_to_binary_buffer(sbuffer, se);
   if (sbuf_length == -1)
    return; // failed; source_edit_to_buffer will have provided an error message to the log
// fprintf(stdout, "\nSaving: path (%s) total_src_lines %i buf_pos %i", se->src_file_path, total_src_lines, buf_pos);
// fprintf(stdout, "%s", buffer);
   save_binary_buffer_to_file(sbuffer, sbuf_length, se);
   break;
 }*/

}


// saves contents of buffer to a file
// file's path is derived from se, and some values in se may be changed (e.g. to indicate it's been saved)
// on failure, writes error message to log and returns normally
void save_source_buffer_to_file(char buffer [WRITE_SIZE], int buf_length, struct source_edit_struct* se)
{

 FILE *file;

// open the file:
 file = fopen(se->src_file_path, "wt");

 if (!file)
 {
  write_line_to_log("Error: failed to open target file.", MLOG_COL_ERROR);
  return;
 }

 int written = fwrite(buffer, 1, buf_length, file);

 if (written != buf_length)
 {
//     fprintf(stdout, "\nError: buf_length %i written %i", buf_length, written);
     write_line_to_log("Error: file write failed.", MLOG_COL_ERROR);
     fclose(file);
     return;
 }


 fclose(file);

 if (se->from_a_file == 0)
  se->from_a_file = 1; // if it wasn't already from a file, it is now.

  start_log_line(MLOG_COL_EDITOR);
  write_to_log("File ");
  write_to_log(se->src_file_name);
  write_to_log(" saved.");
  finish_log_line();


// write_line_to_log("File saved.", MLOG_COL_EDITOR);
 se->saved = 1;

}


// puts se->text into buffer so buffer can be written to file
// returns -1 on failure, size of buffer on success
int source_edit_to_source_buffer(char buffer [WRITE_SIZE], struct source_edit_struct* se)
{


 int src_line = 0;
 int src_pos = 0;
 int total_src_lines = 0;

// first find out how long the source is by counting back from the end until we find a non-empty line:
 src_line = SOURCE_TEXT_LINES - 1;

 while (TRUE)
 {
  src_line --;
  if (src_line == 0)
  {
   write_line_to_log("Can't save an empty source file.", MLOG_COL_ERROR);
   return -1;
  }
  if (se->text [se->line_index [src_line]] [0] != '\0')
   break;
 };

 total_src_lines = src_line;


// assemble the write buffer

 int buf_pos = 0;
 char write_char;

 src_line = 0;
 src_pos = 0;

 while(TRUE)
 {
  while(TRUE)
  {
   if (buf_pos >= WRITE_SIZE - 1)
   {
    write_line_to_log("Error: source file too long; couldn't save.", MLOG_COL_ERROR);
    return -1;
   }
   write_char = se->text [se->line_index [src_line]] [src_pos];
   if (write_char == '\0')
   {
    buffer [buf_pos] = '\n';
    buf_pos ++;
    break;
   }
   buffer [buf_pos] = se->text [se->line_index [src_line]] [src_pos];
   src_pos ++;
   buf_pos ++;
  };
  src_line ++;
  src_pos = 0;
  if (src_line > total_src_lines)
  {
//   buf_pos ++;
//   buffer [buf_pos] = '\n'; // not sure this is really needed
//   buf_pos ++;
   break;
  }
 };

  return buf_pos;

}



// saves contents of buffer to a file
// file's path is derived from se, and some values in se may be changed (e.g. to indicate it's been saved)
// on failure, writes error message to log and returns normally
void save_binary_buffer_to_file(s16b buffer [WRITE_SIZE], int buf_length, struct source_edit_struct* se)
{

 FILE *file;

// open the file:
 file = fopen(se->src_file_path, "wb");

 if (!file)
 {
  write_line_to_log("Error: failed to open target file.", MLOG_COL_ERROR);
  return;
 }

 int written = fwrite(buffer, 2, buf_length, file); // note size = 2

 if (written != buf_length)
 {
//     fprintf(stdout, "\nError: buf_length %i written %i", buf_length, written);
     write_line_to_log("Error: file write failed.", MLOG_COL_ERROR);
     fclose(file);
     return;
 }


 fclose(file);

 if (se->from_a_file == 0)
  se->from_a_file = 1;

 se->saved = 1;
 write_line_to_log("File saved.", MLOG_COL_EDITOR);

}


// puts se->bcode into buffer so buffer can be written to file
// returns -1 on failure, size of buffer on success
int source_edit_to_binary_buffer(s16b buffer [WRITE_SIZE], struct source_edit_struct* se)
{

// int i;
/*
 if (se->bcode.static_length <= 0)
 {
  write_line_to_log("Can't save an empty binary file.", MLOG_COL_ERROR);
  return -1;
 }

 for (i = 0; i < se->bcode.static_length; i ++)
 {
  buffer [i] = se->bcode.op[i];
 }

 return se->bcode.static_length;*/
 return 1;


}

// returns position in bcode.op array of last non-zero element
// returns -1 on failure
int find_last_nonzero_bcode_op(struct bcode_struct* bcode)
{
/*
 int i = bcode->bcode_size - 1;

 while(TRUE)
 {
  if (bcode->op[i] != 0)
   break;
  i --;
  if (i == 0)
   return -1;
 };

 return i;
*/
return 1;
}



// call this function when the user closes a source tab.
// if force_close == 0 it creates a confirm dialog if the source is unsaved.
// otherwise it opens an overwindow to ask whether the user wants to close without saving
void close_source_tab(int tab, int force_close)
{
/*
 if (tab == -1
  || (editor.tab_type [tab] != TAB_TYPE_SOURCE
   && editor.tab_type [tab] != TAB_TYPE_BINARY))
  return;

 struct source_edit_struct* se = &editor.source_edit [editor.tab_index [tab]];


 if (se->saved == 1
  || force_close) // easy; just close the tab
 {
  remove_source_tab(tab, se, editor.tab_index [tab]);
  return;
 }

// otherwise open overwindow:
 open_overwindow(OVERWINDOW_TYPE_CLOSE);
*/
}

// for now this assumes that current_tab is being closed
// is called from close_source_tab
void remove_source_tab(int tab, struct source_edit_struct* se, int se_index)
{

 se->active = 0;
/*
 remove_closed_file_from_undo_stack(se_index);

 editor.tab_type [tab] = TAB_TYPE_NONE;

// move all subsequent tabs
 while (tab < ESOURCES - 1)
 {
  editor.tab_type [tab] = editor.tab_type [tab + 1];
  editor.tab_index [tab] = editor.tab_index [tab + 1];
  strcpy(editor.tab_name [tab], editor.tab_name [tab + 1]);
  strcpy(editor.tab_name_unsaved [tab], editor.tab_name_unsaved [tab + 1]);
  tab ++;
 };

// clear the final tab
 editor.tab_type [tab] = TAB_TYPE_NONE;
 editor.tab_index [tab] = -1;

 if (editor.current_tab > 0
  || editor.tab_type [0] == TAB_TYPE_NONE)
  editor.current_tab --; // may result in -2 if only tab open is closed. Need to correct this:

	if (editor.current_tab < -1)
		editor.current_tab = -1; // indicates no tab open

 change_tab(editor.current_tab);
*/

}




// uses the Allegro native file chooser functions to find a path for a file to save as
// then saves it
// return value is probably useless (if it encounters an error it prints it to the log and returns normally)
int save_as(void)
{

 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
		|| se->active == 0
  || se->type != SOURCE_EDIT_TYPE_SOURCE)
	{
		write_line_to_log("Couldn't save; no current source file.", MLOG_COL_ERROR);
  return 0;
	}


 ALLEGRO_FILECHOOSER* file_dialog;

 if (se->type == SOURCE_EDIT_TYPE_SOURCE)
  file_dialog = al_create_native_file_dialog("", "Choose file", "*.c",  ALLEGRO_FILECHOOSER_SAVE);
   else // SOURCE_EDIT_TYPE_BINARY
    file_dialog = al_create_native_file_dialog("", "Choose file", "*.bc",  ALLEGRO_FILECHOOSER_SAVE);

 if (file_dialog == NULL)
 {
  write_line_to_log("Error: couldn't open Allegro file dialog!", MLOG_COL_ERROR);
  return 0;
 }

 al_show_mouse_cursor(display);
 al_show_native_file_dialog(display, file_dialog); // this should block everything else until it finishes.
 al_hide_mouse_cursor(display);

 flush_game_event_queues();

 int files_to_open = al_get_native_file_dialog_count(file_dialog);

 if (files_to_open == 0)
 {
  al_destroy_native_file_dialog(file_dialog);
  return 0;
 }

 if (files_to_open > 1)
 {
  write_line_to_log("Can only open one file at a time, sorry.", MLOG_COL_ERROR); // not sure this is necessary if the multiple file flag isn't set for al_create_native_file_dialog()
  al_destroy_native_file_dialog(file_dialog);
  return 0;
 }

 const char* file_path_ptr = al_get_native_file_dialog_path(file_dialog, 0);

// fprintf(stdout, "\nFile number %i path (%s)", files_to_open, file_path_ptr);

 if (strlen(file_path_ptr) >= FILE_PATH_LENGTH) // not sure this is needed
 {
  write_line_to_log("File path too long, sorry.", MLOG_COL_ERROR);
  al_destroy_native_file_dialog(file_dialog);
  return 0;
 }

// check whether the user is trying to save as a file that's already open:
 int already_open_tab = check_file_already_open(file_path_ptr);

 if (already_open_tab != -1) // check_file_already_open returns -1 if not already open; otherwise it returns the relevant tab
 {
  write_line_to_log("Can't overwrite a file that is itself open.", MLOG_COL_ERROR);
  al_destroy_native_file_dialog(file_dialog);
  return 2;
 }

/*
// check whether the file already exists:
 FILE *file;

// try opening the file with r file access mode (which should fail if the file doesn't already exist):
 file = fopen(file_path_ptr, "r"); // doesn't matter whether it's rb or rt because this file handle is used only for checking whether it exists

 if (file != NULL)
 {
  write_line_to_log("Error: file already exists.", MLOG_COL_ERROR);
  fclose(file);
  al_destroy_native_file_dialog(file_dialog);
  return 0;
 }*/

/*
 FILE *file;

// try opening the file with r file access mode (which should fail if the file doesn't already exist):
 file = fopen(file_path_ptr, "r"); // doesn't matter whether it's rb or rt because this file handle is used only for checking whether it exists

 if (file != NULL)
 {
  write_line_to_log("File already exists - overwriting.", MLOG_COL_FILE);
//  fclose(file);
//  al_destroy_native_file_dialog(file_dialog);
//  return 0;
 }*/

// from this point file is unused (and null) so we shouldn't have to close it or anything before returning (it was just being used to check whether the file still exists)
//   ^^^ no longer true now that overwriting is enabled.

// now need to put the filename and path into se so that it can be used to save the buffer etc.
// first save the current file name and path, and also the tab name, so they can be reset if needed:
/* char save_src_file_name [FILE_NAME_LENGTH];
 char save_src_file_path [FILE_PATH_LENGTH]; // actually I don't think we need to do any of this, since se->src_file_x isn't overwritten until further down
 char save_tab_name [TAB_NAME_LENGTH];
 char save_tab_name_unsaved [TAB_NAME_LENGTH];
 strcpy(save_src_file_name, se->src_file_name);
 strcpy(save_src_file_path, se->src_file_path);
 strcpy(save_tab_name, editor.tab_name [editor.current_tab]);
 strcpy(save_tab_name_unsaved, editor.tab_name_unsaved [editor.current_tab]);*/

// need to extract the file name from the path.
// Fortunately Allegro can help us here:
 ALLEGRO_PATH* path_struct = al_create_path(file_path_ptr);

 if (path_struct == NULL)
 {
  write_line_to_log("Error: Invalid path.", MLOG_COL_ERROR);
  al_destroy_native_file_dialog(file_dialog);
// don't need to destroy path_struct as it wasn't created properly
  return 0;
 }

 const char* file_name_ptr = al_get_path_filename(path_struct);

 if (strlen(file_name_ptr) >= FILE_NAME_LENGTH)
 {
  write_line_to_log("Error: File name too long.", MLOG_COL_ERROR);
  al_destroy_path(path_struct);
  al_destroy_native_file_dialog(file_dialog);
  return 0;
 }

 int file_type = get_file_type_from_name(file_name_ptr);

 if (se->type == SOURCE_EDIT_TYPE_SOURCE
  && file_type != FILE_TYPE_SOURCE)
 {
  write_line_to_log("Error: source file must have .c extension.", MLOG_COL_ERROR);
  al_destroy_path(path_struct);
  al_destroy_native_file_dialog(file_dialog);
  return 0;
 }

 if (se->type == SOURCE_EDIT_TYPE_BINARY
  && file_type != FILE_TYPE_BINARY)
 {
  write_line_to_log("Error: binary file must have .bc extension.", MLOG_COL_ERROR);
  al_destroy_path(path_struct);
  al_destroy_native_file_dialog(file_dialog);
  return 0;
 }

 strcpy(se->src_file_name, file_name_ptr);
 strcpy(se->src_file_path, file_path_ptr);

 int save_success = 0;

 save_success = save_source_edit_source_to_file(se);
//   if (save_success) - source_edit file name has been renamed above so should do this anyway
// give_source_edit_name_to_tab(editor.current_tab, se);

/*
 switch(se->type)
 {
  case SOURCE_EDIT_TYPE_SOURCE:
   save_success = save_source_edit_source_to_file(se);
//   if (save_success) - source_edit file name has been renamed above so should do this anyway
    give_source_edit_name_to_tab(editor.current_tab, se);
   break;
  case SOURCE_EDIT_TYPE_BINARY:
   save_success = save_source_edit_binary_to_file(se);
//   if (save_success)
    give_source_edit_name_to_tab(editor.current_tab, se);
   break;
 }*/

 al_destroy_path(path_struct);
 al_destroy_native_file_dialog(file_dialog);
 return save_success;

}

// saves source code in se to file indicated by se's src_file_path
// assumes se is valid and of the correct type
// assumes se->src_file_path set
// returns 1 success, 0 failure
int save_source_edit_source_to_file(struct source_edit_struct* se)
{

   char buffer [WRITE_SIZE];

// copy se's text into buffer:
   int buf_length = source_edit_to_source_buffer(buffer, se);

   if (buf_length == -1)
    return 0;

// save buffer to file (path information has already been put in se):
   save_source_buffer_to_file(buffer, buf_length, se);

   return 1;

}



// saves bcode in se to file indicated by se's src_file_path
// assumes se is valid and of the correct type
// assumes se->src_file_path set
// returns 1 success, 0 failure
int save_source_edit_binary_to_file(struct source_edit_struct* se)
{

   s16b sbuffer [WRITE_SIZE];

// copy se's text into buffer:
   int sbuf_length = source_edit_to_binary_buffer(sbuffer, se);

   if (sbuf_length == -1)
    return 0;

// save buffer to file (path information has already been put in se):
   save_binary_buffer_to_file(sbuffer, sbuf_length, se);
// the saved file should contain the entire bcode including any zeros at the end that form part of static memory (but not unused trailing zeros)

   return 1;

}





