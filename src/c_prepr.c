#include <allegro5/allegro.h>
#include "m_config.h"
#include "g_header.h"
#include "c_header.h"

#include "g_misc.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "e_log.h"
#include "e_slider.h"
#include "e_header.h"
#include "e_files.h"

//#include "i_header.h"
#include "i_console.h"
#include "c_prepr.h"


int get_file_name_from_path(char* file_name, const char* file_path);
int remove_file_name_from_path(char* file_path);

int letter_number(char read_char);
int letter(char read_char);
int find_end_of_comment(void);
//int prepr_error(int error_type);
//int prepr_error_minus1(int error_type);
//int prepr_error_no_source(int error_type, const char* file_name);
//int prepr_error_no_source_minus1(int error_type, const char* file_name);
int prepr_error(const char* error_str);


int load_source_file(const char* file_path, struct source_struct* target_source);
//int load_source_file(const char* file_path, struct source_struct* target_source);
//int load_binary_file(const char* file_path, struct bcode_struct* bcode, int src_file_index, int preprocessing);

extern struct cstatestruct cstate;

struct prstate_struct
{
	struct source_edit_struct* source_edit;
	int src_line;
	int src_pos;
	int scode_pos;
};
struct prstate_struct prstate;


int preprocess(struct source_edit_struct* source_edit)
{

	prstate.src_line = 0;
	prstate.src_pos = 0;
	prstate.source_edit = source_edit;
//	cstate.scode.text_length = 0; - compiler should already have been initialised
//	cstate.scode.text [0] = 0;
	char read_char;
	char read_char2;
	int writing_space = 0;

// Need to start scode with a space as the lexer
	prstate.scode_pos = 1;
	cstate.scode.text [0] = ' ';
	cstate.scode.text [1] = '\0';
	cstate.scode.text_length = 1;


	while(TRUE)
	{
		read_char = prstate.source_edit->text[prstate.source_edit->line_index[prstate.src_line]][prstate.src_pos];
		if (read_char == '\0')
		{
			prstate.src_line++;
			prstate.src_pos = 0;
			if (prstate.src_line >= SOURCE_TEXT_LINES)
				break; // finished!
			if (writing_space == 1)
				continue; // don't need to write multiple spaces
			read_char = ' '; // replace end-of-line with a space.
 		prstate.src_pos --; // make sure the first actual character of new line is read.
		}
		if (read_char == '/')
		{
 		read_char2 = prstate.source_edit->text[prstate.source_edit->line_index[prstate.src_line]][prstate.src_pos + 1]; // note + 1
 		if (read_char2 == '/') // single-line comment
			{
			 prstate.src_line ++;
			 prstate.src_pos = 0;
			 if (prstate.src_line >= SOURCE_TEXT_LINES)
			 	break; // finished!
			 continue;
			}
			if (read_char2 == '*') // multi-line comment
			{
				prstate.src_pos++;
				if (!find_end_of_comment())
					break; // finished! (don't really care if the source ends in an unclosed comment)
// find_end_of_comment leaves src_pos just after end of comment
				continue;
			}
		} // end / comment code
	 if (!valid_source_character(read_char))
		{
			char temp_text [2];
			temp_text [0] = read_char;
			temp_text [1] = 0;
			fpr("\nInvalid character in source code: %i [%s]", read_char, temp_text);
		 return prepr_error("invalid character");
		}
		if (read_char == ' ')
		{
			if (writing_space == 1)
			{
  		prstate.src_pos ++; // this shouldn't put src_pos past the end of the line (even though end-of-line is replaced by a space above) because of the continue in the end-of-line check.
  		continue;
			}
			writing_space = 1;
		}
		 else
			 writing_space = 0;
	 cstate.scode.text [prstate.scode_pos] = read_char;
	 cstate.scode.src_line [prstate.scode_pos] = prstate.src_line;
/*
		if (prstate.scode_pos < 100)
		{
			char tempstr [2];
			tempstr [0] = read_char;
			tempstr [1] = 0;
			fpr("\n scode_pos %i src_line %i [%s]", prstate.scode_pos, prstate.src_line, tempstr);
		}*/


	 prstate.scode_pos ++;
		prstate.src_pos ++;

		if (prstate.scode_pos >= SCODE_LENGTH - 3)
			return prepr_error("source code too long");

	} // end while loop

 cstate.scode.text [prstate.scode_pos] = '\0';
	cstate.scode.text_length = prstate.scode_pos;

 return 1;

}


// call this when src_pos is at the * in the start of a multi-line comment.
// leaves src_pos just after the / that ends the comment (which may be at the end of line)
// returns 1 if still reading, 0 if end of source reached
int find_end_of_comment(void)
{
	char read_char;

	while(TRUE)
	{
		read_char = prstate.source_edit->text[prstate.source_edit->line_index[prstate.src_line]][prstate.src_pos];
		if (read_char == '\0')
		{
			prstate.src_line++;
			prstate.src_pos = 0;
			if (prstate.src_line >= SOURCE_TEXT_LINES)
				return 0; // finished!
			continue;
		}
		if (read_char == '*'
			&& prstate.source_edit->text[prstate.source_edit->line_index[prstate.src_line]][prstate.src_pos+1] == '/')
		{
			prstate.src_pos += 2; // assume each line of text is null terminated
			break;
		}
		prstate.src_pos++;
	} // end while loop

	return 1;

} // end find_end_of_comment()


// returns:
//  0 if invalid
//  1 for most valid chars
//  2 for valid chars that the editor input code treats differently (I think just carriage return)
int valid_source_character(char read_char)
{

 if (read_char >= '0' && read_char <= '9')
  return 1;

 if ((read_char >= 'A' && read_char <= 'Z')
  || (read_char >= 'a' && read_char <= 'z'))
   return 1;

 switch(read_char)
 {
  case ' ':
  case '(':
  case ')':
  case '+':
  case '-':
  case '*':
  case '=':
  case '{':
  case '}':
  case '[':
  case ']':
  case '/':
  case '_':
  case ';':
  case ':':
  case ',':
  case '.':
  case '>':
  case '<':
  case '&':
  case '^':
  case '~':
  case '#': // if # appears at start of line, we expect a preprocessor directive
  case '|':
  case '!':
  case '%':
  case '?':
  case '"':
  case '@': // does nothing but could be included in printed text
  case '$': // does nothing but could be included in printed text
  case '\'':
  case '\\':
   return 1;

  case '\n':
  case '\r':
  	return 2;

 }

 return 0;

}

int letter_number(char read_char)
{

    if ((read_char >= '0' && read_char <= '9')
     || (read_char >= 'a' && read_char <= 'z')
     || (read_char >= 'A' && read_char <= 'Z')
     || read_char == '_')
      return 1;

    return 0;

}

int letter(char read_char)
{

    if ((read_char >= 'a' && read_char <= 'z')
     || (read_char >= 'A' && read_char <= 'Z')
     || read_char == '_')
      return 1;

    return 0;

}


/*
This function loads source file file_name into source_struct target_source.
src_file_index is the index of the file to identify it in multi-file bcodenotestructs

 - TO DO: probably should store the names of all included files in the source_struct (to copy to bcodenotestruct)

Assumes file_name is a valid string (of any length; checks length).
Assumes src_file_index is valid (from 0 to INCLUDED_FILES-1).
 - current src_file_index isn't used

Doesn't assume file file_name exists, or can be read in correctly.
Is used by both the preprocessor (in which case preprocessing==1 and errors are treated as preprocessor errors) and the editor's file open command (preprocessing==0)

returns 1 on success, 0 on failure. Will call prepr_error if preprocessing==1; otherwise will write error to log
*/
int load_source_file(const char* file_path, struct source_struct* target_source)
{

#define READ_SIZE 100000

 char buffer [READ_SIZE];
 FILE *file;
 unsigned int read_in = 0;
 int i;

 if (strlen(file_path) >= FILE_PATH_LENGTH)
 {
  write_line_to_log("Error: Path too long.", MLOG_COL_ERROR);
  return 0;
 }


// open the file:
 file = fopen(file_path, "rt");

 if (file)
 {

    read_in = fread(buffer, 1, READ_SIZE, file);


    if (ferror(file) || read_in == 0)
    {
     fclose(file);
     write_line_to_log("Error: Couldn't read file.", MLOG_COL_ERROR);
     return 0;
    }

    if (!feof(file))
    {
     fclose(file);
     write_line_to_log("Error: EOF not found (file too long?).", MLOG_COL_ERROR);
     return 0;
    }

    fclose(file);

 }
  else
  {
   write_line_to_log("Error: File not found.", MLOG_COL_ERROR);
   return 0;
  }

// file_path has been checked for length above
 strcpy(target_source->src_file_path, file_path);

// need to extract the file name from the path:
 if (!get_file_name_from_path(target_source->src_file_name, file_path))
  return 0;

// we know that the source is from a file:
 target_source->from_a_file = 1;

// need to add null terminator
 if (read_in < READ_SIZE)
  buffer [read_in] = 0;
   else
    buffer [READ_SIZE - 1] = 0;

 int src_line = 0;
 int src_pos = 0;
 int line_finished;

 for (i = 0; i < read_in; i ++)
 {
// Peter Hull solved a problem with loading source code on systems that don't use Windows-style line endings:
   /* hack for *nix */
   if (buffer[i] == '\r')
    continue;
   /* end hack */
   if (buffer[i] == '\t')
    buffer[i] = ' '; // I'm not really sure how to handle tabs, sorry
  target_source->text [src_line] [src_pos] = buffer [i];
  line_finished = 0;
  if (buffer[i] == '\n')
  {
   line_finished = 1;
// newline found - fill the rest of the source line with 0s
   if (src_pos < SOURCE_TEXT_LINE_LENGTH)
   {
//    target_source->text [src_line] [src_pos] = ' '; // this will replace the \n with a space
    target_source->text [src_line] [src_pos] = '\0'; // this will replace the \n with a space
    src_pos ++;
   }
   while (src_pos < SOURCE_TEXT_LINE_LENGTH)
   {
    target_source->text [src_line] [src_pos] = 0;
    src_pos ++;
   }
  }
  src_pos ++;

  if (src_pos >= SOURCE_TEXT_LINE_LENGTH - 1)
  {
   target_source->text [src_line] [src_pos] = '\0'; // if line is too long, terminate it and start new line
   if (!line_finished) // line is too long and was not terminated by newline
   {
    start_log_line(MLOG_COL_ERROR);
    write_to_log("Error: line ");
    write_number_to_log(src_line + 1);
    write_to_log(" of file ");
    write_to_log(target_source->src_file_name);
    write_to_log(" too long (maximum is ");
    write_number_to_log(SOURCE_TEXT_LINE_LENGTH);
    write_to_log(" chars).");
    finish_log_line();
    return 0;
   }
   src_line ++;
   src_pos = 0;
  }
  if (src_line >= SOURCE_TEXT_LINES)
  {
   write_line_to_log("Error: too many lines in file.", MLOG_COL_ERROR);
   return 0;
  }
 }

// fill the rest of the source_struct with 0s
 while (src_line < SOURCE_TEXT_LINES)
 {
  target_source->text [src_line] [src_pos] = '\0';
  src_pos ++;
  if (src_pos >= SOURCE_TEXT_LINE_LENGTH)
  {
   src_line ++;
   src_pos = 0;
  }
 };

// make sure every source line is null-terminated:
 src_line = 0;

 while (src_line < SOURCE_TEXT_LINES)
 {
  target_source->text [src_line] [SOURCE_TEXT_LINE_LENGTH - 1] = '\0';
  src_line ++;
 };

 return 1;

}



/*
This function loads binary (.bc) file file_name into bcode_struct bcode.
src_file_index is the index of the file to identify it in multi-file bcodenotestructs (probably not used at all for now)

 - TO DO: probably should store the names of all included files in the source_struct (to copy to bcodenotestruct)

Assumes file_name is a valid string (of any length; checks length).
Assumes src_file_index is valid (from 0 to INCLUDED_FILES-1).
 *** currently src_file_index isn't used (not implemented)

Doesn't assume file file_name exists, or can be read in correctly.
Is used by both the preprocessor (in which case preprocessing==1 and errors are treated as preprocessor errors) and the editor's file open command (preprocessing==0)

leaves length of file in bcode->static_length

returns 1 on success, 0 on failure. Will call prepr_error if preprocessing==1; otherwise will write error to log
* /
int load_binary_file(const char* file_path, struct bcode_struct* bcode, int src_file_index, int preprocessing)
{

#define BINARY_READ_SIZE 20000

 s16b sbuffer [BINARY_READ_SIZE];
 FILE *file;
 unsigned int read_in = 0;
 int i;

 if (strlen(file_path) >= FILE_PATH_LENGTH)
 {
  if (preprocessing)
   return prepr_error_no_source(PRERR_FILE_PATH_TOO_LONG, file_path);
  write_line_to_log("Error: Path too long.", MLOG_COL_ERROR);
  return 0;
 }


// open the file:
 file = fopen(file_path, "rb");


 if (file)
 {

    read_in = fread(sbuffer, sizeof(s16b), BINARY_READ_SIZE, file);


    if (ferror(file) || read_in == 0)
    {
     fclose(file);
     if (preprocessing)
      return prepr_error_no_source(PRERR_FILE_READ_ERROR, file_path);
     write_line_to_log("Error: Couldn't read binary file.", MLOG_COL_ERROR);
     return 0;
    }

    if (!feof(file))
    {
     fclose(file);
     if (preprocessing)
      return prepr_error_no_source(PRERR_FILE_TOO_LONG, file_path);
     write_line_to_log("Error: EOF not found (file too long?).", MLOG_COL_ERROR);
     return 0;
    }

    if (read_in >= bcode->bcode_size)
    {
     fclose(file);
     if (preprocessing)
      return prepr_error_no_source(PRERR_FILE_TOO_LONG, file_path);
     write_line_to_log("Error: binary file too long.", MLOG_COL_ERROR);
     return 0;
    }

    fclose(file);

 }
  else
  {
   if (preprocessing)
    return prepr_error_no_source(PRERR_FILE_OPEN_ERROR, file_path);
   write_line_to_log("Error: failed to open binary file.", MLOG_COL_ERROR);
   return 0;
  }

// file_path has been checked for length above
 strcpy(bcode->src_file_path, file_path);

// need to extract the file name from the path:
 if (!get_file_name_from_path(bcode->src_file_name, file_path, 1))
  return 0;

// we know that the source is from a file:
 bcode->from_a_file = 1;

// clear target bcode
 for (i = 0; i < bcode->bcode_size; i ++)
 {
  bcode->op [i] = 0;
 }

// we've already checked length of read buffer (read_in) and confirmed it's less than BCODE_MAX
 for (i = 0; i < read_in; i ++)
 {
  bcode->op [i] = sbuffer [i];
 }

 bcode->static_length = read_in;

 return 1;

}

*/

// extracts the file name from file_path and copies it to *file_name
// assumes file_name is at least FILE_NAME_LENGTH and file_path is at least FILE_PATH_LENGTH
// on failure, calls prepr_error if preprocessing == 1  - otherwise just writes to mlog
// returns 0 fail/ 1 success
int get_file_name_from_path(char* file_name, const char* file_path)
{

 ALLEGRO_PATH* path_struct = al_create_path(file_path);

 if (path_struct == NULL)
 {
  write_line_to_log("Error: Invalid path.", MLOG_COL_ERROR);
// don't need to call al_destroy_path() before returning as it wasn't created properly
  return 0;
 }

 const char* file_name_ptr = al_get_path_filename(path_struct);

 if (strlen(file_name_ptr) >= FILE_NAME_LENGTH)
 {
  al_destroy_path(path_struct); // remember not to use path_struct after this
  write_line_to_log("Error: File name too long.", MLOG_COL_ERROR);
  return 0;
 }

 strcpy(file_name, file_name_ptr);

 al_destroy_path(path_struct);

 return 1;

}

// alters file_path by removing everything after the final delimiter
// assumes file_path is a null-terminated string
// returns 1 if no delimiter
// returns 0 fail or 1 success
int remove_file_name_from_path(char* file_path)
{

 int i = strlen(file_path) - 1;

 if (i <= 0)
 {
  write_line_to_log("Error: invalid path.", MLOG_COL_ERROR);
// not sure these errors can ever actually happen
  return 0;
 }

 while(i > -1)
 {
  if (file_path [i] == '\\' || file_path [i] == '/')
  {
   file_path [i + 1] = '\0'; // remember that i is initialised to strlen(file_path) - 1
   return 1;
  }
  i --;
 };

 file_path [0] = '\0'; // not sure about this
 return 1;

}


int prepr_error(const char* error_str)
{

     start_log_line(MLOG_COL_ERROR);
     write_to_log("Preprocessor error at line ");
     write_number_to_log(prstate.src_line + 1);
/*     if (prstate.source_file == 0)
      write_to_log(" of file ");
       else
        write_to_log(" of included file ");
     write_to_log(prstate.source [prstate.source_file]->src_file_name);*/
     write_to_log(".");
     finish_log_line();

//     if (error_type != PRERR_EMPTY)
     {
      start_log_line(MLOG_COL_ERROR);
      write_to_log("Error: ");
      write_to_log(error_str);//prepr_error_name [error_type]);
      write_to_log(".");
      finish_log_line();
     }


//     prstate.error = error_type;
     return 0;


}


