#include <allegro5/allegro.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "c_header.h"
#include "m_globvars.h"
#include "e_slider.h"
#include "e_header.h"
#include "e_editor.h"

#include "e_inter.h"
#include "e_log.h"

#include "g_misc.h"

void copy_selection(void);
void paste_clipboard(void);
int get_next_line_from_clipboard(int pos, char* str);
int add_text_at_end_of_line(struct source_edit_struct* se, char* add_str);
int add_text_at_start_of_line(struct source_edit_struct* se, char* add_str);

void delete_undo_pos_base(void);
void check_undo_buffer_overlap(int old_buffer_pos, int new_buffer_pos);
void move_to_next_undo(void);
void increment_undo_pos(void);
void add_char_to_undo_buffer(char achar);
int get_undo_size(void);
void finish_undo_entry(void);
void add_char_undo(char achar);
void init_undo(void);
void decrement_undo_pos(void);
void remove_text_from_source_line(struct source_edit_struct* se, int sline, int text_end, int chars);
void insert_text_into_source_line(struct source_edit_struct* se, int sline, int target_start, int buffer_pos, int chars);
void insert_text_into_source_line_backwards(struct source_edit_struct* se, int sline, int target_start, int buffer_pos, int chars);
//void pu(void);
void add_block_to_undo(int start_line, int start_pos, int end_line, int end_pos, int undo_type);
int bufcpy(char* target, int b);
int bufcat(char* target, int b);
int bufins(char* target, int b);

extern struct editorstruct editor; // defined in e_editor.c
extern struct log_struct mlog; // in e_log.c

// Note that this function assumes the clipboard is at least large enough to contain an entire source file
void copy_selection(void)
{

 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || se->type != SOURCE_EDIT_TYPE_SOURCE)
  return;

 if (!is_something_selected(se)) // confirms that at least one character is selected
  return;

 int start_line, start_pos, end_line, end_pos;

 if (se->select_fix_line < se->select_free_line
  || (se->select_fix_line == se->select_free_line
   && se->select_fix_pos < se->select_free_pos))
  {
   start_line = se->select_fix_line;
   start_pos = se->select_fix_pos;
   end_line = se->select_free_line;
   end_pos = se->select_free_pos;
  }
   else
   {
    start_line = se->select_free_line;
    start_pos = se->select_free_pos;
    end_line = se->select_fix_line;
    end_pos = se->select_fix_pos;
   }

  int copy_line = start_line;
  int copy_pos = start_pos;
  int i = 0;
  editor.clipboard_lines = 0;


  if (start_line == end_line)
  {
// this code assumes that the selection does not extend past the end of the line (which shouldn't be possible)
   while (copy_pos < end_pos)
   {
    editor.clipboard [i] = se->text [se->line_index [copy_line]] [copy_pos];
    copy_pos++;
    i++;
   };
   editor.clipboard [i] = '\0';
   editor.clipboard_lines = 0;
//   fprintf(stdout, "\nCopied: (%s)", editor.clipboard);
   return;
  }

// start and end must be on different lines.
// first copy the selected text on the first line:
   while (se->text [se->line_index [copy_line]] [copy_pos] != '\0')
   {
    editor.clipboard [i] = se->text [se->line_index [copy_line]] [copy_pos];
    copy_pos++;
    i++;
   };

   editor.clipboard [i] = '\n';
   editor.clipboard_lines++;
   i++;
   copy_line ++;
   copy_pos = 0;

// now copy each line until the end_line is reached (if any):

   while (copy_line < end_line)
   {
    while (se->text [se->line_index [copy_line]] [copy_pos] != '\0')
    {
     editor.clipboard [i] = se->text [se->line_index [copy_line]] [copy_pos];
     copy_pos++;
     i++;
    };
    editor.clipboard [i] = '\n';
    editor.clipboard_lines++;
    i++;
    copy_line ++;
    copy_pos = 0;
   };

// now copy the end line:
   while (copy_pos < end_pos)
   {
    editor.clipboard [i] = se->text [se->line_index [copy_line]] [copy_pos];
    copy_pos++;
    i++;
   };

   editor.clipboard [i] = '\0';
   editor.clipboard_lines++;

// fprintf(stdout, "\nCopied %i lines: (%s)", editor.clipboard_lines, editor.clipboard);


}

void paste_clipboard(void)
{

 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || se->type != SOURCE_EDIT_TYPE_SOURCE)
  return;

 if (editor.clipboard [0] == '\0')
  return;

 if (!delete_selection())
  return;

 se->saved = 0; // indicates that source has been modified

 int i, length;
 int old_line = se->cursor_line;
 int old_pos = se->cursor_pos;

 if (editor.clipboard_lines == 0) // just within one line
 {
  if (strlen(editor.clipboard) + strlen(se->text [se->line_index [se->cursor_line]]) >= SOURCE_TEXT_LINE_LENGTH - 1)
  {
   write_line_to_log("Line too long to paste text.", MLOG_COL_ERROR);
   return;
  }
//  i = 0;
  length = strlen(editor.clipboard);
// push the rest of the line back to make space for inserted text:
  i = SOURCE_TEXT_LINE_LENGTH - 1;
  while(i >= se->cursor_pos + length)
  {
   se->text [se->line_index [se->cursor_line]] [i] = se->text [se->line_index [se->cursor_line]] [i - length];
   i --;
  };
/*  while(i < length)
  {
   se->text [se->line_index [se->cursor_line]] [se->cursor_pos + i] = se->text [se->line_index [se->cursor_line]] [se->cursor_pos + i + length];
   i ++;
  };*/
// now insert the text:
  i = 0;
  while(i < length)
  {
   add_char_undo(editor.clipboard [i]);
   se->text [se->line_index [se->cursor_line]] [se->cursor_pos] = editor.clipboard [i];
   i ++;
   se->cursor_pos ++;
  };
//  se->cursor_pos += length;
  se->cursor_base = se->cursor_pos;
  update_source_lines(se, se->cursor_line, 1);
  window_find_cursor(se);
  return;
 } // end code for single-line paste


  int start_line = se->cursor_line;
// at this point we know that the clipboard text contains at least one line break (represented by '\n' char).
// Start by splitting the current line
   if (insert_empty_lines(se, se->cursor_line + 1, 1))
   {
// current line is now followed by an empty line.
// now copy remaining text from current line onto next line:
    i = 0;
    while (se->text [se->line_index [se->cursor_line]] [se->cursor_pos + i] != '\0')
    {
     se->text [se->line_index [se->cursor_line + 1]] [i] = se->text [se->line_index [se->cursor_line]] [se->cursor_pos + i];
     i++;
    };
    se->text [se->line_index [se->cursor_line + 1]] [i] = '\0';
    se->text [se->line_index [se->cursor_line]] [se->cursor_pos] = '\0';
// update syntax highlighting for both lines:
//    update_source_line(se, se->cursor_line);
//    update_source_line(se, se->cursor_line + 1);
// put cursor on next line
//    se->cursor_line ++;
//    se->cursor_pos = 0;
//    window_find_cursor(se);
   }
    else
    {
      write_line_to_log("Out of space in source file.", MLOG_COL_ERROR);
      return;
    }
// at this point we have what was the current line split into two, with the cursor at the end of the first.
// now get the first line from the clipboard:
   int clip_pos = 0;
   char paste_string [SOURCE_TEXT_LINE_LENGTH + 1];
   paste_string [0] = '\0';
   clip_pos = get_next_line_from_clipboard(clip_pos, paste_string);
// should be able to assume that clip_pos is not -1, as we have already dealt with single-line clipboards
   if (!add_text_at_end_of_line(se, paste_string)) // this function leaves cursor_line at the next line
    return;

// now insert enough lines to fit the rest of the clipboard:
    if (!insert_empty_lines(se, se->cursor_line, editor.clipboard_lines - 2))
    {
     write_line_to_log("Out of space in source file.", MLOG_COL_ERROR);
     return;
    }

    se->cursor_line --;

// go to the first such empty line (may actually be the last line, with text already, if the clipboard is just two lines)

// go through the clipboard line by line:
    while (TRUE)
    {
     se->cursor_line ++;
     clip_pos = get_next_line_from_clipboard(clip_pos, paste_string);
     if (clip_pos == -1)
      break; // found end
     strcpy(se->text [se->line_index [se->cursor_line]], paste_string);
    }
// paste_string now contains the last line of the clipboard.

    if (!add_text_at_start_of_line(se, paste_string))
     return;

   update_source_lines(se, start_line, se->cursor_line - start_line + 1);

   se->cursor_pos = strlen(paste_string);

   add_block_to_undo(old_line, old_pos, se->cursor_line, se->cursor_pos, UNDO_TYPE_INSERT_BLOCK);

   se->cursor_base = se->cursor_pos;

   window_find_cursor(se);

}

// this function reads the next line from the clipboard into str, and returns the resulting position in the clipboard buffer.
// returns -1 if the line read ends in \0 rather than \n
// assumes str has enough space for a full source line
int get_next_line_from_clipboard(int pos, char* str)
{

 int i = 0;

 do
 {
  str [i] = editor.clipboard [pos + i];
  if (str [i] == '\n')
  {
   str [i] = '\0';
   return pos + i + 1;
  }
  if (str [i] == '\0')
   return -1;
  i ++;
 } while (TRUE);

 return -1;

}

// tries to add text at end of source line (se->cursor_line). splits line into two if not enough space
// returns 1 on success, 0 on failure.
// leaves se->cursor_line after the last line edited.
int add_text_at_end_of_line(struct source_edit_struct* se, char* add_str)
{
   if (strlen(add_str) + strlen(se->text [se->line_index [se->cursor_line]]) < SOURCE_TEXT_LINE_LENGTH + 1)
   {
    strcat(se->text [se->line_index [se->cursor_line]], add_str);
    se->cursor_line ++;
   }
    else
    {
// if it won't fit, put it on a new line:
     if (insert_empty_lines(se, se->cursor_line + 1, 1))
     {
      se->cursor_line ++;
      strcpy(se->text [se->line_index [se->cursor_line]], add_str);
      se->cursor_line ++;
     }
      else
      {
       write_line_to_log("Out of space in source file.", MLOG_COL_ERROR);
       return 0;
      }
    }


 return 1;
}

// opposite of add_text_at_end_of_line
int add_text_at_start_of_line(struct source_edit_struct* se, char* add_str)
{
   char temp_str [SOURCE_TEXT_LINE_LENGTH + 1];

   if (strlen(add_str) + strlen(se->text [se->line_index [se->cursor_line]]) < SOURCE_TEXT_LINE_LENGTH + 1)
   {
    strcpy(temp_str, add_str);
    strcat(temp_str, se->text [se->line_index [se->cursor_line]]);
    strcpy(se->text [se->line_index [se->cursor_line]], temp_str);
//    se->cursor_line ++;
//       write_line_to_log("Enough space", -1, -1);
   }
    else
    {
// if it won't fit, put the existing text on a new line:
     if (insert_empty_lines(se, se->cursor_line + 1, 1))
     {
      strcpy(se->text [se->line_index [se->cursor_line + 1]], se->text [se->line_index [se->cursor_line]]);
      strcpy(se->text [se->line_index [se->cursor_line]], add_str);
      se->cursor_line ++;
//       write_line_to_log("new line", -1, -1);
     }
      else
      {
       write_line_to_log("Out of space in source file.", MLOG_COL_ERROR);
       return 0;
      }
    }


 return 1;
}



/*

UNDO!!!!

How is this going to work?

#define UNDO_STACK_SIZE 10
#define UNDO_BUFFER_SIZE (CLIPBOARD_SIZE * 2)

Editor will have:
 int undo_pos;
 int undo_pos_base; // this is the bottom of the current undo stack.
 int undo_type [UNDO_STACK_SIZE];
 int undo_se [UNDO_STACK_SIZE]; // source edit
 int undo_start [UNDO_STACK_SIZE];
 int undo_size [UNDO_STACK_SIZE];
 int undo_cursor_line [UNDO_STACK_SIZE];
 int undo_cursor_pos [UNDO_STACK_SIZE];

 int undo_buffer_pos;
 int undo_buffer_base_pos; // first position in the buffer currently in use by an undo stack entry
 char undo_buffer [UNDO_BUFFER_SIZE];

enum
{
UNDO_TYPE_NONE,
UNDO_TYPE_INSERT_TEXT, // this means text has been inserted. stores the text on the undo buffer in case of a redo.
UNDO_TYPE_INSERT_BLOCK,
UNDO_TYPE_BACKSPACE,
UNDO_TYPE_DELETE,
UNDO_TYPE_ENTER,


};


*/


void init_undo(void)
{
 int i;

 for (i = 0; i < UNDO_STACK_SIZE; i ++)
 {
  editor.undo_type [i] = UNDO_TYPE_NONE;
  editor.undone [i] = 0;
 }

 editor.undo_pos = 1;
 editor.undo_buffer_pos = 0;
 editor.undo_buffer_base_pos = 0;

 editor.undo_buffer [0] = '\0';

}


// call this when a single character is added to the source.
void add_char_undo(char achar)
{

//pu();

 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || se->type != SOURCE_EDIT_TYPE_SOURCE)
  return;

 if (editor.undo_pos == 0)
  editor.undo_pos = 1;

// find out if we need to put a new entry on the undo stack:
 if (editor.undo_type [editor.undo_pos] != UNDO_TYPE_INSERT_TEXT
  || editor.undo_se [editor.undo_pos] != editor.current_source_edit_index // if there's no current tab get_current_source_edit will already have failed
  || editor.undo_cursor_line [editor.undo_pos] != se->cursor_line
  || editor.undo_cursor_pos [editor.undo_pos] != se->cursor_pos - 1)
 {
//  if (editor.undo_pos == editor.undo_pos_base)
//   increment_undo_pos();
  if (editor.undo_type [editor.undo_pos] != UNDO_TYPE_NONE)
  {
//   fprintf(stdout, "\nFinishing %i (base %i)", editor.undo_pos, editor.undo_pos_base);
   finish_undo_entry();
   move_to_next_undo(); // increments undo_pos and clears the next stack entry.
//   fprintf(stdout, "\nNow %i (base %i)", editor.undo_pos, editor.undo_pos_base);
  }
  editor.undo_type [editor.undo_pos] = UNDO_TYPE_INSERT_TEXT;
  editor.undo_se [editor.undo_pos] = editor.current_source_edit_index;
  editor.undo_start [editor.undo_pos] = editor.undo_buffer_pos;
  editor.undo_size [editor.undo_pos] = 0;
  editor.undo_cursor_line [editor.undo_pos] = se->cursor_line;
  editor.undone [editor.undo_pos] = 0;
//  if (editor.undo_buffer_base_pos == -1)
//   editor.undo_buffer_base_pos = editor.undo_buffer_pos;
//  editor.undo_cursor_pos [editor.undo_pos] = se->cursor_pos;
 }

 add_char_to_undo_buffer(achar);
 editor.undo_size [editor.undo_pos] ++;
 editor.undo_cursor_pos [editor.undo_pos] = se->cursor_pos;

}


// call this when a line break is added to the source.
// for now these aren't grouped together. Each one gets its own stack entry
void add_enter_undo(void)
{

//pu();
 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || se->type != SOURCE_EDIT_TYPE_SOURCE)
  return;

 if (editor.undo_pos == 0)
  editor.undo_pos = 1;

// find out if we need to put a new entry on the undo stack:
  if (editor.undo_type [editor.undo_pos] != UNDO_TYPE_NONE)
  {
   finish_undo_entry();
   move_to_next_undo(); // increments undo_pos and clears the next stack entry.
  }
  editor.undo_type [editor.undo_pos] = UNDO_TYPE_ENTER;
  editor.undo_se [editor.undo_pos] = editor.current_source_edit_index;
  editor.undo_start [editor.undo_pos] = editor.undo_buffer_pos;
  editor.undo_size [editor.undo_pos] = 0;
  editor.undo_cursor_line [editor.undo_pos] = se->cursor_line;
  editor.undone [editor.undo_pos] = 0;
//  if (editor.undo_buffer_base_pos == -1)
//   editor.undo_buffer_base_pos = editor.undo_buffer_pos;

// add_char_to_undo_buffer('\n'); // not necessary, but let's be consistent for now
// editor.undo_size [editor.undo_pos] ++;
 editor.undo_cursor_pos [editor.undo_pos] = se->cursor_pos;

}


// call this when a single character is deleted from the source.
void delete_char_undo(char achar)
{

//pu();
 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || se->type != SOURCE_EDIT_TYPE_SOURCE)
  return;

 if (editor.undo_pos == 0)
  editor.undo_pos = 1;

// find out if we need to put a new entry on the undo stack:
 if (editor.undo_type [editor.undo_pos] != UNDO_TYPE_DELETE
  || editor.undo_se [editor.undo_pos] != editor.current_source_edit_index
  || editor.undo_cursor_line [editor.undo_pos] != se->cursor_line
  || editor.undo_cursor_pos [editor.undo_pos] != se->cursor_pos)
 {
  if (editor.undo_type [editor.undo_pos] != UNDO_TYPE_NONE)
  {
   finish_undo_entry();
   move_to_next_undo(); // increments undo_pos and clears the next stack entry.
  }
  editor.undo_type [editor.undo_pos] = UNDO_TYPE_DELETE;
  editor.undo_se [editor.undo_pos] = editor.current_source_edit_index;
  editor.undo_start [editor.undo_pos] = editor.undo_buffer_pos;
  editor.undo_size [editor.undo_pos] = 0;
  editor.undo_cursor_line [editor.undo_pos] = se->cursor_line;
  editor.undone [editor.undo_pos] = 0;
//  if (editor.undo_buffer_base_pos == -1)
//   editor.undo_buffer_base_pos = editor.undo_buffer_pos;
//  editor.undo_cursor_pos [editor.undo_pos] = se->cursor_pos;
 }

 add_char_to_undo_buffer(achar);
 editor.undo_size [editor.undo_pos] ++;
 editor.undo_cursor_pos [editor.undo_pos] = se->cursor_pos;

}


// like delete_char_undo but for backspace (I really should merge the two)
void backspace_char_undo(char achar)
{

//pu();
 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || se->type != SOURCE_EDIT_TYPE_SOURCE)
  return;

 if (editor.undo_pos == 0)
  editor.undo_pos = 1;

// find out if we need to put a new entry on the undo stack:
 if (editor.undo_type [editor.undo_pos] != UNDO_TYPE_BACKSPACE
  || editor.undo_se [editor.undo_pos] != editor.current_source_edit_index
  || editor.undo_cursor_line [editor.undo_pos] != se->cursor_line
  || editor.undo_cursor_pos [editor.undo_pos] != se->cursor_pos + 1)
 {
  if (editor.undo_type [editor.undo_pos] != UNDO_TYPE_NONE)
  {
   finish_undo_entry();
   move_to_next_undo(); // increments undo_pos and clears the next stack entry.
  }
  editor.undo_type [editor.undo_pos] = UNDO_TYPE_BACKSPACE;
  editor.undo_se [editor.undo_pos] = editor.current_source_edit_index;
  editor.undo_start [editor.undo_pos] = editor.undo_buffer_pos;
  editor.undo_size [editor.undo_pos] = 0;
  editor.undo_cursor_line [editor.undo_pos] = se->cursor_line;
  editor.undone [editor.undo_pos] = 0;
//  if (editor.undo_buffer_base_pos == -1)
//   editor.undo_buffer_base_pos = editor.undo_buffer_pos;
//  editor.undo_cursor_pos [editor.undo_pos] = se->cursor_pos;
 }

 add_char_to_undo_buffer(achar);
 editor.undo_size [editor.undo_pos] ++;
 editor.undo_cursor_pos [editor.undo_pos] = se->cursor_pos;

}

void add_undo_remove_enter(void)
{

//pu();
 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || se->type != SOURCE_EDIT_TYPE_SOURCE)
  return;

 if (editor.undo_pos == 0)
  editor.undo_pos = 1;

// find out if we need to put a new entry on the undo stack:
  if (editor.undo_type [editor.undo_pos] != UNDO_TYPE_NONE)
  {
   finish_undo_entry();
   move_to_next_undo(); // increments undo_pos and clears the next stack entry.
  }
  editor.undo_type [editor.undo_pos] = UNDO_TYPE_DELETE_ENTER;
  editor.undo_se [editor.undo_pos] = editor.current_source_edit_index;
  editor.undo_start [editor.undo_pos] = editor.undo_buffer_pos;
  editor.undo_size [editor.undo_pos] = 0;
  editor.undo_cursor_line [editor.undo_pos] = se->cursor_line; // this is the line at the end of which the enter is being deleted
  editor.undone [editor.undo_pos] = 0;
//  if (editor.undo_buffer_base_pos == -1)
//   editor.undo_buffer_base_pos = editor.undo_buffer_pos;

// add_char_to_undo_buffer('\n'); // not necessary, but let's be consistent for now
// editor.undo_size [editor.undo_pos] ++;
 editor.undo_cursor_pos [editor.undo_pos] = se->cursor_pos;

}



// call whenever an undo entry is finished (e.g. user is entering text and presses enter - undo entry finishes at end of line)
void finish_undo_entry(void)
{
  if (editor.undo_type [editor.undo_pos] == UNDO_TYPE_NONE)
   return; // nothing to do

//  editor.undo_size [editor.undo_pos] = get_undo_size();

//  add_char_to_undo_buffer('\0');
//  editor.undo_size [editor.undo_pos] ++;

}

// returns the size of current undo in the undo buffer.
int get_undo_size(void)
{


 if (editor.undo_start [editor.undo_pos] < editor.undo_buffer_pos)
  return editor.undo_buffer_pos - editor.undo_start [editor.undo_pos];
   else
    return (UNDO_BUFFER_SIZE - editor.undo_start [editor.undo_pos]) + editor.undo_buffer_pos;

}

void add_char_to_undo_buffer(char achar)
{

// fprintf(stdout, "\nAdding %i (%i)", achar, editor.undo_buffer [0]);

 editor.undo_buffer [editor.undo_buffer_pos] = achar;
 editor.undo_buffer_pos ++;

 if (editor.undo_buffer_pos == UNDO_BUFFER_SIZE)
  editor.undo_buffer_pos = 0;

 while (editor.undo_buffer_pos == editor.undo_buffer_base_pos)
 {
  delete_undo_pos_base();
 }; // This loop should always finish as long as the undo buffer is larger than a single source code file can be (if it's not larger, the start point of the current undo entry could be the buffer base and this loop could be infinite)
// if (editor.undo_buffer_pos == editor.undo_buffer_base)
//  resolve_undo_buffer_overlap();

}

// this is called when adding something to the undo stack, and also when redoing something
/*void increment_undo_pos(void)
{
//fprintf(stdout, "\nincr");
 editor.undo_pos ++;

 if (editor.undo_pos == UNDO_STACK_SIZE - 1)
  delete_undo_pos_base();

}*/

/*
void decrement_undo_pos(void)
{

 editor.undo_pos --;

// if (editor.undo_pos == -1)
//  editor.undo_pos = UNDO_STACK_SIZE - 1;

}*/

// this is called when adding something to the undo stack (but not when redoing)
void move_to_next_undo(void)
{

// fprintf(stdout, "\nMove pos %i base %i", editor.undo_pos, editor.undo_pos_base);

// if the current undo_pos is the stack base, start by incrementing:
// if (editor.undo_pos == editor.undo_pos_base)
// {
//  increment_undo_pos();
// }

/*// if the stack is full, need to delete the undo pos base
 if (editor.undo_pos == editor.undo_pos_base
  || (editor.undo_pos == UNDO_STACK_SIZE - 1 && editor.undo_pos_base == 0))
 {
  delete_undo_pos_base();
 }*/

 editor.undo_pos ++;
 editor.undo_type [editor.undo_pos] = UNDO_TYPE_NONE;
 editor.undo_type [editor.undo_pos + 1] = UNDO_TYPE_NONE;

 if (editor.undo_pos == UNDO_STACK_SIZE - 2)
  delete_undo_pos_base();


// if (editor.undo_pos == editor.undo_pos_base) // likely
//  delete_undo_pos_base();

}


// This function should be called when:
//  - undo stack pos catches up with the undo base; or
//  - undo buffer pos catches up with the buffer base.
// Doesn't guarantee space in the buffer for the next stack pos. Need to check (and may need to call this function multiple times)
void delete_undo_pos_base(void)
{

 if (editor.undo_pos == 0)
  return; // nothing to do

 int i;

 for (i = 0; i < UNDO_STACK_SIZE - 1; i ++)
 {
  editor.undo_type [i] = editor.undo_type [i + 1];
  editor.undo_se [i] = editor.undo_se [i + 1];
  editor.undo_start [i] = editor.undo_start [i + 1];
  editor.undo_size [i] = editor.undo_size [i + 1];
  editor.undo_cursor_line [i] = editor.undo_cursor_line [i + 1];
  editor.undo_cursor_pos [i] = editor.undo_cursor_pos [i + 1];
  editor.undone [i] = editor.undone [i + 1];
 }

 editor.undo_buffer_base_pos = editor.undo_start [0];

 editor.undo_pos --;

}



void call_undo(void)
{

// if (editor.current_source_edit == -1)
//  return;
//pu();

/* fprintf(stdout, "\n(%i)", editor.undo_buffer [0]);


 if (editor.undo_type [editor.undo_pos] == UNDO_TYPE_NONE)
  fprintf(stdout, "\nfail1");
 if (editor.undo_pos == 0)
  fprintf(stdout, "\nfail2");
 if (editor.undone [editor.undo_pos] == 1)
  fprintf(stdout, "\nfail3");*/


 if (editor.undo_type [editor.undo_pos] == UNDO_TYPE_NONE
  || editor.undo_pos == 0
  || editor.undone [editor.undo_pos] == 1
  || editor.undo_se [editor.undo_pos] != editor.current_source_edit_index)
  return;

 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL)
  return;

 int i;
 int b;

 switch(editor.undo_type [editor.undo_pos])
 {
  case UNDO_TYPE_NONE:
   break;
  case UNDO_TYPE_INSERT_TEXT:
   se->saved = 0; // indicates that source has been modified
   se->cursor_line = editor.undo_cursor_line [editor.undo_pos];
   se->cursor_pos = editor.undo_cursor_pos [editor.undo_pos];
   remove_text_from_source_line(se, se->cursor_line, se->cursor_pos, editor.undo_size [editor.undo_pos]);
   update_source_lines(se, se->cursor_line, 1);
   editor.undone [editor.undo_pos] = 1;
   editor.undo_buffer_pos = editor.undo_start [editor.undo_pos];
   editor.undo_pos --;
   break;
  case UNDO_TYPE_DELETE:
   se->saved = 0; // indicates that source has been modified
   se->cursor_line = editor.undo_cursor_line [editor.undo_pos];
   se->cursor_pos = editor.undo_cursor_pos [editor.undo_pos];
   insert_text_into_source_line(se, se->cursor_line, se->cursor_pos, editor.undo_start [editor.undo_pos], editor.undo_size [editor.undo_pos]);
   update_source_lines(se, se->cursor_line, 1);
   editor.undone [editor.undo_pos] = 1;
   editor.undo_buffer_pos = editor.undo_start [editor.undo_pos];
   editor.undo_pos --;
   break;
  case UNDO_TYPE_BACKSPACE:
   se->saved = 0; // indicates that source has been modified
   se->cursor_line = editor.undo_cursor_line [editor.undo_pos];
   se->cursor_pos = editor.undo_cursor_pos [editor.undo_pos];
   insert_text_into_source_line_backwards(se, se->cursor_line, se->cursor_pos, editor.undo_start [editor.undo_pos], editor.undo_size [editor.undo_pos]);
   update_source_lines(se, se->cursor_line, 1);
   editor.undone [editor.undo_pos] = 1;
   editor.undo_buffer_pos = editor.undo_start [editor.undo_pos];
   editor.undo_pos --;
   break;
  case UNDO_TYPE_ENTER:
   se->saved = 0; // indicates that source has been modified
   se->cursor_line = editor.undo_cursor_line [editor.undo_pos];
   se->cursor_pos = editor.undo_cursor_pos [editor.undo_pos]; // ??
   strcat(se->text [se->line_index [se->cursor_line]], se->text [se->line_index [se->cursor_line + 1]]);
   delete_lines(se, se->cursor_line + 1, 1);
   update_source_lines(se, se->cursor_line, 2);
   editor.undone [editor.undo_pos] = 1;
   editor.undo_buffer_pos = editor.undo_start [editor.undo_pos];
   editor.undo_pos --;
   break;
  case UNDO_TYPE_DELETE_ENTER:
   se->saved = 0; // indicates that source has been modified
   se->cursor_line = editor.undo_cursor_line [editor.undo_pos];
   se->cursor_pos = editor.undo_cursor_pos [editor.undo_pos];
   insert_empty_lines(se, se->cursor_line + 1, 1);
   i = 0;
   while (i + se->cursor_pos < SOURCE_TEXT_LINE_LENGTH)
   {
    se->text [se->line_index [se->cursor_line + 1]] [i] = se->text [se->line_index [se->cursor_line]] [i + se->cursor_pos];
    i ++;
   };
   se->text [se->line_index [se->cursor_line]] [se->cursor_pos] = '\0';
   update_source_lines(se, se->cursor_line, 2);
   editor.undone [editor.undo_pos] = 1;
   editor.undo_buffer_pos = editor.undo_start [editor.undo_pos];
   editor.undo_pos --;
   break;
  case UNDO_TYPE_INSERT_BLOCK:
   se->saved = 0; // indicates that source has been modified
// assumes at least 2 lines are involved.
   se->cursor_line = editor.undo_cursor_line [editor.undo_pos];
   se->cursor_pos = editor.undo_cursor_pos [editor.undo_pos]; // ??
// truncate the first line of the block:
   se->text [se->line_index [se->cursor_line]] [se->cursor_pos] = '\0';
// now delete text from the start of the last line of the block:
   i = editor.undo_end_pos [editor.undo_pos];
   while (i < SOURCE_TEXT_LINE_LENGTH - 1)
   {
    se->text [se->line_index [editor.undo_end_line [editor.undo_pos]]] [i - editor.undo_end_pos [editor.undo_pos]] = se->text [se->line_index [editor.undo_end_line [editor.undo_pos]]] [i];
    i ++;
   };
// now add remainder of last line to first, then delete last line:
   strcat(se->text [se->line_index [se->cursor_line]], se->text [se->line_index [editor.undo_end_line [editor.undo_pos]]]);

   delete_lines(se, se->cursor_line + 1, editor.undo_end_line [editor.undo_pos] - se->cursor_line);

/*   delete_lines(se, se->line_index [editor.undo_end_line [editor.undo_pos]], 1);
// now delete lines in between, if any
   if (editor.undo_end_line [editor.undo_pos] > se->cursor_line + 1)
    delete_lines(se, se->cursor_line + 1, editor.undo_end_line [editor.undo_pos] - se->cursor_line);*/

   update_source_lines(se, se->cursor_line, 1);

   editor.undone [editor.undo_pos] = 1;
   editor.undo_buffer_pos = editor.undo_start [editor.undo_pos];
   editor.undo_pos --;
   break;
  case UNDO_TYPE_DELETE_BLOCK:
   se->saved = 0; // indicates that source has been modified
//   fprintf(stdout, "\nUndo buffer (");
//   fprintf(stdout, editor.undo_buffer);
//   fprintf(stdout, ")\n");
// assumes at least 2 lines are involved.
   se->cursor_line = editor.undo_cursor_line [editor.undo_pos];
   se->cursor_pos = editor.undo_cursor_pos [editor.undo_pos]; // ??
// first split the line the cursor is on
// make a new line then put the second half of the current line on it:
// (but only if this is actually necessary)
//   int new_first_line = 1;
//   int new_last_line = 1;
//   if (se->text [se->line_index [se->cursor_line]] [0] == '\0')
//    new_first_line = 0;
//   if (editor.undo_end_pos [editor.undo_pos] == strlen (se->text [se->line_index [se->cursor_line]] [0]))
//    new_last_line = 0;
   insert_empty_lines(se, se->cursor_line + 1, 1);
   i = 0;
   while (TRUE)
   {
    se->text [se->line_index [se->cursor_line + 1]] [i] = se->text [se->line_index [se->cursor_line]] [se->cursor_pos + i];
    if (se->text [se->line_index [se->cursor_line + 1]] [i] == '\0')
     break;
    i ++;
   };
// truncate the first line:
   se->text [se->line_index [se->cursor_line]] [se->cursor_pos] = '\0';
// insert empty lines:
   if (editor.undo_end_line [editor.undo_pos] > se->cursor_line + 1)
    insert_empty_lines(se, se->cursor_line + 1, editor.undo_end_line [editor.undo_pos] - se->cursor_line - 1);
//    insert_empty_lines(se, se->cursor_line + 1, editor.undo_end_line [editor.undo_pos] - se->cursor_line - (editor.undo_block_ends_with_full_line [editor.undo_pos] == 0)); // undo_block_ends_with_full_line is 0 or 1
//    insert_empty_lines(se, se->cursor_line + 1, editor.undo_end_line [editor.undo_pos] - se->cursor_line - (editor.undo_end_pos [editor.undo_pos] == 0)); // don't add a line if end_pos is at zero position
//   fprintf(stdout, "\nInserting lines %i (end_line %i, cursor_line %i)", editor.undo_end_line [editor.undo_pos] - se->cursor_line, editor.undo_end_line [editor.undo_pos], se->cursor_line);
// add the first line of the block to the current source line:
   b = bufcat(se->text [se->line_index [se->cursor_line]], editor.undo_start [editor.undo_pos]); // returns buffer position
// add the rest:
   i = 1;
   while (i < editor.undo_end_line [editor.undo_pos] - se->cursor_line)
   {
    b = bufcpy(se->text [se->line_index [se->cursor_line + i]], b); // returns buffer position
    i ++;
   };
// put the last line of the block at the front of the last source line:
   b = bufins(se->text [se->line_index [se->cursor_line + i]], b);

   update_source_lines(se, se->cursor_line, editor.undo_end_line [editor.undo_pos] - se->cursor_line);

   editor.undone [editor.undo_pos] = 1;
   editor.undo_buffer_pos = editor.undo_start [editor.undo_pos];

   se->cursor_line = editor.undo_end_line [editor.undo_pos];
   se->cursor_pos = editor.undo_end_pos [editor.undo_pos];
   se->cursor_base = se->cursor_pos;
   window_find_cursor(se);

   editor.undo_pos --;
   break;

 }

}

void call_redo(void)
{

 if (editor.undo_type [editor.undo_pos + 1] == UNDO_TYPE_NONE
  || editor.undone [editor.undo_pos + 1] == 0
  || editor.undo_se [editor.undo_pos + 1] != editor.current_source_edit_index)
  return;

 editor.undo_pos ++;

 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL)
  return;
// need to think about how to handle multiple source_edits being open at once.

 int i, b;

 switch(editor.undo_type [editor.undo_pos])
 {
  case UNDO_TYPE_NONE:
   break; // should have been caught above
  case UNDO_TYPE_INSERT_TEXT:
   se->saved = 0; // indicates that source has been modified
   se->cursor_line = editor.undo_cursor_line [editor.undo_pos];
   se->cursor_pos = editor.undo_cursor_pos [editor.undo_pos] - editor.undo_size [editor.undo_pos] + 1;
   insert_text_into_source_line(se, se->cursor_line, se->cursor_pos, editor.undo_start [editor.undo_pos], editor.undo_size [editor.undo_pos]);
   update_source_lines(se, se->cursor_line, 1);
   editor.undone [editor.undo_pos] = 0;
   editor.undo_buffer_pos = (editor.undo_start [editor.undo_pos] + editor.undo_size [editor.undo_pos]) % UNDO_BUFFER_SIZE;
   break;
  case UNDO_TYPE_DELETE:
  case UNDO_TYPE_BACKSPACE:
   se->saved = 0; // indicates that source has been modified
   se->cursor_line = editor.undo_cursor_line [editor.undo_pos];
   se->cursor_pos = editor.undo_cursor_pos [editor.undo_pos];
   remove_text_from_source_line(se, se->cursor_line, se->cursor_pos + editor.undo_size [editor.undo_pos] - 1, editor.undo_size [editor.undo_pos]);
   update_source_lines(se, se->cursor_line, 1);
   editor.undone [editor.undo_pos] = 0;
   editor.undo_buffer_pos = (editor.undo_start [editor.undo_pos] + editor.undo_size [editor.undo_pos]) % UNDO_BUFFER_SIZE;
   break;
  case UNDO_TYPE_ENTER:
   se->saved = 0; // indicates that source has been modified
   se->cursor_line = editor.undo_cursor_line [editor.undo_pos];
   se->cursor_pos = editor.undo_cursor_pos [editor.undo_pos];
   insert_empty_lines(se, se->cursor_line + 1, 1);
// current line is now followed by an empty line.
// now copy remaining text from current line onto next line:
   i = 0;
   while (se->text [se->line_index [se->cursor_line]] [se->cursor_pos + i] != '\0')
   {
    se->text [se->line_index [se->cursor_line + 1]] [i] = se->text [se->line_index [se->cursor_line]] [se->cursor_pos + i];
    i++;
   };
   se->text [se->line_index [se->cursor_line + 1]] [i] = '\0';
   se->text [se->line_index [se->cursor_line]] [se->cursor_pos] = '\0';
// update syntax highlighting for both lines:
   update_source_lines(se, se->cursor_line, 2);
// put cursor on next line
   se->cursor_line ++;
   se->cursor_pos = 0;
   editor.undone [editor.undo_pos] = 0;
   editor.undo_buffer_pos = (editor.undo_start [editor.undo_pos] + editor.undo_size [editor.undo_pos]) % UNDO_BUFFER_SIZE;
   break;
  case UNDO_TYPE_DELETE_ENTER:
   se->saved = 0; // indicates that source has been modified
   se->cursor_line = editor.undo_cursor_line [editor.undo_pos];
   se->cursor_pos = editor.undo_cursor_pos [editor.undo_pos]; // ??
   strcat(se->text [se->line_index [se->cursor_line]], se->text [se->line_index [se->cursor_line + 1]]);
   delete_lines(se, se->cursor_line + 1, 1);
   update_source_lines(se, se->cursor_line, 2);
   editor.undone [editor.undo_pos] = 0;
   editor.undo_buffer_pos = (editor.undo_start [editor.undo_pos] + editor.undo_size [editor.undo_pos]) % UNDO_BUFFER_SIZE;
   break;

  case UNDO_TYPE_DELETE_BLOCK:
   se->saved = 0; // indicates that source has been modified
// assumes at least 2 lines are involved.
   se->cursor_line = editor.undo_cursor_line [editor.undo_pos];
   se->cursor_pos = editor.undo_cursor_pos [editor.undo_pos]; // ??
// truncate the first line of the block:
   se->text [se->line_index [se->cursor_line]] [se->cursor_pos] = '\0';
// now delete text from the start of the last line of the block:
   i = editor.undo_end_pos [editor.undo_pos];
   while (i < SOURCE_TEXT_LINE_LENGTH - 1)
   {
    se->text [se->line_index [editor.undo_end_line [editor.undo_pos]]] [i - editor.undo_end_pos [editor.undo_pos]] = se->text [se->line_index [editor.undo_end_line [editor.undo_pos]]] [i];
    i ++;
   };
// now add remainder of last line to first, then delete last line:
   strcat(se->text [se->line_index [se->cursor_line]], se->text [se->line_index [editor.undo_end_line [editor.undo_pos]]]);

   delete_lines(se, se->cursor_line + 1, editor.undo_end_line [editor.undo_pos] - se->cursor_line);

/*
   delete_lines(se, se->line_index [editor.undo_end_line [editor.undo_pos]], 1);
// now delete lines in between, if any
   if (editor.undo_end_line [editor.undo_pos] > se->cursor_line + 1)
    delete_lines(se, se->cursor_line + 1, editor.undo_end_line [editor.undo_pos] - se->cursor_line - 1);*/

   update_source_lines(se, se->cursor_line, 1);

   editor.undone [editor.undo_pos] = 0;
   editor.undo_buffer_pos = (editor.undo_start [editor.undo_pos] + editor.undo_size [editor.undo_pos]) % UNDO_BUFFER_SIZE;
   break;



  case UNDO_TYPE_INSERT_BLOCK:
   se->saved = 0; // indicates that source has been modified
//   fprintf(stdout, "\nRedo buffer (");
//   fprintf(stdout, editor.undo_buffer);
//   fprintf(stdout, ")\n");
// assumes at least 2 lines are involved.
   se->cursor_line = editor.undo_cursor_line [editor.undo_pos];
   se->cursor_pos = editor.undo_cursor_pos [editor.undo_pos]; // ??
// first split the line the cursor is on
// make a new line then put the second half of the current line on it:
// (but only if this is actually necessary)
//   int new_first_line = 1;
//   int new_last_line = 1;
//   if (se->text [se->line_index [se->cursor_line]] [0] == '\0')
//    new_first_line = 0;
//   if (editor.undo_end_pos [editor.undo_pos] == strlen (se->text [se->line_index [se->cursor_line]] [0]))
//    new_last_line = 0;
   insert_empty_lines(se, se->cursor_line + 1, 1);
   i = 0;
   while (TRUE)
   {
    se->text [se->line_index [se->cursor_line + 1]] [i] = se->text [se->line_index [se->cursor_line]] [se->cursor_pos + i];
    if (se->text [se->line_index [se->cursor_line + 1]] [i] == '\0')
     break;
    i ++;
   };
// truncate the first line:
   se->text [se->line_index [se->cursor_line]] [se->cursor_pos] = '\0';
// insert empty lines:
   if (editor.undo_end_line [editor.undo_pos] > se->cursor_line + 1)
    insert_empty_lines(se, se->cursor_line + 1, editor.undo_end_line [editor.undo_pos] - se->cursor_line - 1);
//   fprintf(stdout, "\nInserting lines %i (end_line %i, cursor_line %i)", editor.undo_end_line [editor.undo_pos] - se->cursor_line, editor.undo_end_line [editor.undo_pos], se->cursor_line);
// add the first line of the block to the current source line:
   b = bufcat(se->text [se->line_index [se->cursor_line]], editor.undo_start [editor.undo_pos]); // returns buffer position
// add the rest:
   i = 1;
   while (i < editor.undo_end_line [editor.undo_pos] - se->cursor_line)
   {
    b = bufcpy(se->text [se->line_index [se->cursor_line + i]], b); // returns buffer position
    i ++;
   };
// put the last line of the block at the front of the last source line:
   b = bufins(se->text [se->line_index [se->cursor_line + i]], b);

   update_source_lines(se, se->cursor_line, editor.undo_end_line [editor.undo_pos] - se->cursor_line);

   editor.undone [editor.undo_pos] = 0;
   editor.undo_buffer_pos = (editor.undo_start [editor.undo_pos] + editor.undo_size [editor.undo_pos]) % UNDO_BUFFER_SIZE;

   se->cursor_line = editor.undo_end_line [editor.undo_pos];
   se->cursor_pos = editor.undo_end_pos [editor.undo_pos];
   se->cursor_base = se->cursor_pos;
   window_find_cursor(se);

   break;


 }

}
/*
void pu(void)
{

 return;

 fprintf(stdout, "\npos %i current size %i (", editor.undo_pos, get_undo_size());
 int i;
 for (i = 0; i < UNDO_STACK_SIZE; i ++)
 {
  fprintf(stdout, "%i(%i:%i), ", editor.undo_type [i], editor.undo_start [i], editor.undo_size [i]);
 }
 fprintf(stdout, ")");
}
*/
/*TO DO:
need to make sure the base undo stack entry is empty
(and the base undo buffer pos is the start of the *next* stack entry)
*/


void remove_text_from_source_line(struct source_edit_struct* se, int sline, int text_end, int chars)
{

 int i = text_end - chars + 1;

 while (i < SOURCE_TEXT_LINE_LENGTH - 1 - chars)
 {
  se->text [se->line_index [sline]] [i] = se->text [se->line_index [sline]] [i + chars];
  i ++;
 };

 se->cursor_pos = text_end - chars + 1;
 se->cursor_base = se->cursor_pos;

/*
 int i = SOURCE_TEXT_LINE_LENGTH - 1 - chars;

 while (i > text_end - chars)
 {
  se->text [se->line_index [sline]] [i] = se->text [se->line_index [sline]] [i + chars];
  i --;
 };*/

}


// note: this function doesn't check bounds in source, so it can only be used for undo/redo (when it can be assumed that the inserted text fits in the line, because it was there before)
// (it does check bounds for the undo buffer, though)
void insert_text_into_source_line(struct source_edit_struct* se, int sline, int target_start, int buffer_pos, int chars)
{

 int i = SOURCE_TEXT_LINE_LENGTH - 1;

 while (i > target_start + chars - 1)
 {
  se->text [se->line_index [sline]] [i] = se->text [se->line_index [sline]] [i - chars];
  i --;
 };

 for (i = 0; i < chars; i ++)
 {
  se->text [se->line_index [sline]] [target_start + i] = editor.undo_buffer [buffer_pos];
  buffer_pos ++;
  if (buffer_pos == UNDO_BUFFER_SIZE)
   buffer_pos = 0;
 }

 se->cursor_pos = target_start + chars;
 se->cursor_base = se->cursor_pos;

/*
 int i = SOURCE_TEXT_LINE_LENGTH - 1 - chars;

 while (i > text_end - chars)
 {
  se->text [se->line_index [sline]] [i] = se->text [se->line_index [sline]] [i + chars];
  i --;
 };*/

}

void insert_text_into_source_line_backwards(struct source_edit_struct* se, int sline, int target_start, int buffer_pos, int chars)
{

 int i = SOURCE_TEXT_LINE_LENGTH - 1;

 buffer_pos += chars - 1;
 if (buffer_pos >= UNDO_BUFFER_SIZE)
  buffer_pos -= UNDO_BUFFER_SIZE;

 while (i > target_start + chars - 1)
 {
  se->text [se->line_index [sline]] [i] = se->text [se->line_index [sline]] [i - chars];
  i --;
 };

 for (i = 0; i < chars; i ++)
 {
//  fprintf(stdout, "\n%i: %i becomes %i: %i", i, se->text [se->line_index [sline]] [target_start + i], buffer_pos, editor.undo_buffer [buffer_pos]);
  se->text [se->line_index [sline]] [target_start + i] = editor.undo_buffer [buffer_pos];
//  if (se->text [se->line_index [sline]] [target_start + i] == 0)
//   se->text [se->line_index [sline]] [target_start + i] = 'x';
  buffer_pos --;
  if (buffer_pos == -1)
   buffer_pos = UNDO_BUFFER_SIZE - 1;
 }

 se->cursor_pos = target_start + chars;
 se->cursor_base = se->cursor_pos;

/*
 int i = SOURCE_TEXT_LINE_LENGTH - 1 - chars;

 while (i > text_end - chars)
 {
  se->text [se->line_index [sline]] [i] = se->text [se->line_index [sline]] [i + chars];
  i --;
 };*/

}


// like strcpy but with the undo buffer as a source (which requires a check for wrapping around to the start)
// note: this function doesn't check bounds in source, so it can only be used for undo/redo (when it can be assumed that the inserted text fits in the line, because it was there before)
// (it does check bounds for the undo buffer, though)
// can't assume that target is any particular length (because of the way bufcat() calls this function)
// returns new value for b (position within undo buffer)
int bufcpy(char* target, int b)
{

 int i = 0;

 while (TRUE)
 {
//  fprintf(stdout, "\nbufcpy i %i b %i %i", i, b, editor.undo_buffer [b]);
/*  if (b >= 50
   || i >= SOURCE_TEXT_LINE_LENGTH)
    error_call();*/

  target [i] = editor.undo_buffer [b];
  if (editor.undo_buffer [b] == '\0')
  {
   b++;
//   fprintf(stdout, "\nend reached");
   return b;
  }
  if (editor.undo_buffer [b] == '\n') // indicates line break within a block in the buffer
  {
   target [i] = '\0'; // replace line break with end of line
   b++;
//   fprintf(stdout, "\nend of line");
   return b;
  }
  i ++;
  b ++;
  if (b == UNDO_BUFFER_SIZE)
   b = 0;
 };


}

int bufcat(char* target, int b)
{

 target += strlen(target);

 return bufcpy(target, b);

}

// inserts text from buffer at start of target
// assumes target is a whole source line
int bufins(char* target, int b)
{

// first get the string from the buffer:
 char temp_str [SOURCE_TEXT_LINE_LENGTH];
 int i = 0;

 while (TRUE)
 {
  temp_str [i] = editor.undo_buffer [b];
  b ++;
  if (b == UNDO_BUFFER_SIZE)
   b = 0;
  if (temp_str [i] == '\0')
   break;
  i ++;
 };

 int chars = strlen(temp_str);

 if (chars == 0)
  return b;

// fprintf(stdout, "\nInsert (%s)", temp_str);

// now shift characters at the start of target:
 i = SOURCE_TEXT_LINE_LENGTH - 1;

 while (i > chars - 1)
 {
  target [i] = target [i - chars];
  i --;
 };

// now put temp_str at start of line:
 i = 0;

 while (i < chars)
 {
  target [i] = temp_str [i];
  i ++;
 };

 return b;

}


/*
// this function is called when more than one character is added to the buffer (if one character is added the check is simpler).
//
void check_undo_buffer_overlap(int old_buffer_pos, int new_buffer_pos)
{

// first check whether the text being added to the buffer causes it to wrap around:
 if (new_buffer_pos < old_buffer_pos) // must have wrapped around
 {
  while((editor.undo_buffer_pos_base < new_buffer_pos
    || editor.undo_buffer_pos_base > old_buffer_pos)
     && editor.undo_pos_base != editor.undo_pos)
  {
   delete_undo_pos_base();
  };
  return;
 }

// at this point we know that new_buffer_pos is larger than old_buffer pos
// so we need to clear stack entries until the buffer base pos is outside the range between old and new buffer pos

 while(editor.undo_buffer_pos_base >= old_buffer_pos
    && editor.undo_buffer_pos_base <= new_buffer_pos
    && editor.undo_pos_base != editor.undo_pos)
 {
  delete_undo_pos_base();
 };


}
*/
/*
void clear_undo_buffer_base_undo(void)
{

 if (editor.undo_buffer_base_undo == -1)
  return; // nothing to do

 int un = editor.undo_buffer_base_undo;

 editor.undo_type [un] = UNDO_TYPE_NONE;



}*/


// call this to put a block of text in the undo buffer.
// can be used for both deletion and insertion
// assumes start_line < end_line (not <=)
void add_block_to_undo(int start_line, int start_pos, int end_line, int end_pos, int undo_type)
{


 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || se->type != SOURCE_EDIT_TYPE_SOURCE)
  return;
// TO DO: deal with multiple se windows

/* int i;
 int total_size = 0;

// first calculate total size
 total_size += strlen(se->text [se->line_index [start_line]]) - start_pos + 1;
 total_size += end_pos; // only one that doesn't need + 1

 i = start_pos + 1;

// loop won't run at all if block is only 2 lines
 while(i < end_pos)
 {
  total_size += strlen
// + 1 for /n
 };*/

 if (editor.undo_pos == 0)
  editor.undo_pos = 1;

// find out if we need to put a new entry on the undo stack:
  if (editor.undo_type [editor.undo_pos] != UNDO_TYPE_NONE)
  {
   finish_undo_entry();
   move_to_next_undo(); // increments undo_pos and clears the next stack entry.
  }

  editor.undo_end_line [editor.undo_pos] = end_line;
  editor.undo_end_pos [editor.undo_pos] = end_pos;

  editor.undo_type [editor.undo_pos] = undo_type;
  editor.undo_se [editor.undo_pos] = editor.current_source_edit_index;
  editor.undo_start [editor.undo_pos] = editor.undo_buffer_pos;
  editor.undo_size [editor.undo_pos] = 0;
  editor.undo_cursor_line [editor.undo_pos] = se->cursor_line;
  editor.undone [editor.undo_pos] = 0;

  if (end_pos == strlen(se->text [se->line_index [end_line]]))
   editor.undo_block_ends_with_full_line [editor.undo_pos] = 1;
    else
     editor.undo_block_ends_with_full_line [editor.undo_pos] = 0;

  int current_line = start_line;
  int current_pos = start_pos;
  char read_char;

  while(current_line < end_line
     || current_pos < end_pos)// + 1)
  {
   read_char = se->text [se->line_index [current_line]] [current_pos];
   if (read_char == '\0')
   {
    read_char = '\n';
    current_line ++;
    current_pos = -1;
   }
   add_char_to_undo_buffer(read_char);
   editor.undo_size [editor.undo_pos] ++;
   current_pos ++;
  };

  add_char_to_undo_buffer('\0');


// fprintf(stdout, "\n**** size1: %i", editor.undo_size [editor.undo_pos]);

// add_char_to_undo_buffer('\n'); // not necessary, but let's be consistent for now
// editor.undo_size [editor.undo_pos] ++;
 if (editor.undo_buffer_pos > editor.undo_start [editor.undo_pos])
  editor.undo_size [editor.undo_pos] = editor.undo_buffer_pos - editor.undo_start [editor.undo_pos];
   else
   {
    editor.undo_size [editor.undo_pos] = editor.undo_buffer_pos + (UNDO_BUFFER_SIZE - editor.undo_start [editor.undo_pos]);
   }

// fprintf(stdout, "\nsize2: %i", editor.undo_size [editor.undo_pos]);


 editor.undo_cursor_pos [editor.undo_pos] = start_pos;
 editor.undo_cursor_line [editor.undo_pos] = start_line;


}

// Call this when a source_edit is closed.
// It will go through the undo stack and remove all undo stuff for the closed source_edit.
void remove_closed_file_from_undo_stack(int se_index)
{

 int i;

 for (i = 0; i < UNDO_STACK_SIZE; i ++)
 {
  if (editor.undo_se [i] == se_index)
   editor.undo_se [i] = -2; // will never match
 }

}

