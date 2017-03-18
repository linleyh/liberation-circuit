



#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "m_config.h"

#include "g_header.h"

#include "g_misc.h"

#include "c_header.h"
#include "e_header.h"
#include "e_editor.h"
#include "e_complete.h"
#include "e_log.h"
#include "c_prepr.h"
#include "c_keywords.h"
#include "i_header.h"

#include "p_panels.h"

struct completionstruct completion;



// word lists:
//extern const struct numtokenstruct numtoken [NUMTOKENS];
//extern const struct c_keywordstruct c_keyword [C_KEYWORDS];
//extern const struct c_keywordstruct c_builtin_cfunction [C_BUILTIN_CFUNCTION_NAMES];
//extern const struct c_keywordstruct asm_keyword [ASM_KEYWORDS];



extern struct editorstruct editor; // defined in e_editor.c
extern struct coloursstruct colours;
struct fontstruct font [FONTS];


//#define COMPLETION_TABLE_SIZE 1200

// completion_table is an alphabetically sorted list of all code completion tokens.
// it's also used in e_help.c (as is alphabet_index)
char completion_table [KEYWORDS] [COMPLETION_TABLE_STRING_LENGTH];
int completion_table_keyword_index [KEYWORDS];

int alphabet_index [26]; // index to start point in completion table

static int get_word_from_editor(struct source_edit_struct* se, int source_line, int source_pos, int in_code_completion, char* check_word);
int alphabetical_comparison(const void* str1, const void* str2);

extern struct identifierstruct identifier [IDENTIFIERS]; // defined in c_keywords.c

void init_code_completion(void)
{

	int i = 0;
//	int j = 0;

	for (i = 0; i < KEYWORDS; i ++)
	{
		strcpy(completion_table [i], identifier[i].name);
	}

	completion.table_size = KEYWORDS;

 qsort(completion_table, completion.table_size, COMPLETION_TABLE_STRING_LENGTH, alphabetical_comparison);



// Now built an alphabetical quick look-up thing for the first letter of each word
//  (don't need to find the last word starting with a letter because the list builder stops when it finds a non-matching word after a matching one)
 for (i = 0; i < 26; i ++)
	{
  alphabet_index [i] = -1; // -1 means no keyword starts with this letter
	}

	char alpha_number = 'a';
	int alpha_number_int;

 for (i = 0; i < KEYWORDS; i ++)
	{
		alpha_number = completion_table [i] [0];
// bounds-check	this just in case a keyword that doesn't start with 'a' to 'z' is set
#ifdef SANITY_CHECK
  if (alpha_number < 'a' || alpha_number > 'z')
		{
			fpr("\n Error: e_complete.c: init_code_completion(): keyword %i doesn't start with a letter from a to z (lower case).", i);
			error_call();
		}
#endif

  alpha_number_int = alpha_number - 'a'; // this just avoids a compiler warning about array subscript of char type

		if (alphabet_index [alpha_number_int] == -1)
			alphabet_index [alpha_number_int] = i;
	}
// this will leave some alphabet_index values as -1, for letters not found

/*
	while(numtoken[j].name [0] != '\0')
	{
		strcpy(completion_table [i], numtoken[j].name);
		i++;
		j++;
		if (j >= NUMTOKENS)
		{
			fprintf(stdout, "\nError: e_complete.c: init_code_completion(): passed end of numtoken list?");
			error_call();
		}
		if (i >= COMPLETION_TABLE_SIZE)
		{
			fprintf(stdout, "\nError: e_complete.c: init_code_completion(): ran out of space in completion table.");
			error_call();
		}
	};*/

// j = 0; // don't reset i
/*
	while(c_keyword[j].name [0] != '\0'
				&& j < C_KEYWORDS)
	{
		strcpy(completion_table [i], c_keyword[j].name);
		i++;
		j++;
		if (i >= COMPLETION_TABLE_SIZE)
		{
			fprintf(stdout, "\nError: e_complete.c: init_code_completion(): ran out of space in completion table.");
			error_call();
		}
	};*/
/*
 j = 0; // don't reset i

	while(c_builtin_cfunction[j].name [0] != '\0'
				&& j < C_BUILTIN_CFUNCTION_NAMES)
	{
		strcpy(completion_table [i], c_builtin_cfunction[j].name);
		i++;
		j++;
		if (i >= COMPLETION_TABLE_SIZE) w_init.core_setting = 2;
 w_init.size_setting = 2;

		{
			fprintf(stdout, "\nError: e_complete.c: init_code_completion(): ran out of space in completion table.");
			error_call();
		}
	};*/
/*
 j = 0; // don't reset i

	while(asm_keyword[j].name [0] != '\0'
				&& j < ASM_KEYWORDS)
	{
		strcpy(completion_table [i], asm_keyword[j].name);
		i++;
		j++;

		if (i >= COMPLETION_TABLE_SIZE)
		{
			fprintf(stdout, "\nError: e_complete.c: init_code_completion(): ran out of space in completion table.");
			error_call();
		}
	};
*/
/*
 for (i = 0; i < completion.table_size; i ++)
	{
		fprintf(stdout, "\n%i: %s", i, completion_table [i]);
	}*/


}



int alphabetical_comparison(const void* str1, const void* str2)
{
 return strcmp(str1, str2);
}



// call this when the user types a letter or equivalent symbol
void check_code_completion(struct source_edit_struct* se, int from_backspace)
{

 int i;

 int check_pos = se->cursor_pos;
 int cursor_pos = se->cursor_pos;

 if (from_backspace)
		cursor_pos --;

 completion.list_size = 0;

 char check_word [IDENTIFIER_MAX_LENGTH + 1] = "";


 int check_word_length = get_word_from_editor(se, se->cursor_line, check_pos, 1, check_word);

 if (!check_word_length)
		return;
//* here

// fprintf(stdout, "\nword [%s] length %i", check_word, check_word_length);


// Now check_word contains the current word (ignoring anything that appears after the cursor)
// Use it to build a code completion list.
 completion.list_size = 0;

 int j;
 int found;
 int started_finding = 0;

 int first_letter = check_word [0] - 'a';

 if (check_word [0] < 'a' || check_word [0] > 'z')
		return; // this makes it case-sensitive (currently all keywords start with lower case)

 int starting_pos = alphabet_index [first_letter];

 if (starting_pos == -1)
		return; // no keyword starts with the same letter as check_word

 for (i = starting_pos; i < KEYWORDS; i ++)
	{

		if (completion.list_size >= COMPLETION_LIST_LENGTH)
			break; // shouldn't happen

  j = 0;
  found = 0;

  while(TRUE)
		{
   if (j >= check_word_length)
			{
				found = 1;
				started_finding = 1;
				break;
			}
   if (check_word [j] != completion_table [i] [j]
			 || completion_table [i] [j] == '\0')
		 {
		 	break;
		 }
		 j ++;
		};
		if (found == 1)
		{
//			completion.list_entry_type [completion.list_size] = COMPLETION_TYPE_NUMTOKEN;
			completion.list_entry_index [completion.list_size] = i;
			completion.list_size++;
		}
		 else
				if (started_finding)
				 break; // After finding a match, stop after finding the first non-matching word

	}


/*
 fprintf(stdout, "\nlist size: %i", completion.list_size);

 for (i = 0; i < completion.list_size; i ++)
	{
		fprintf(stdout, "\n%s in list at %i", numtoken[completion.list_entry_index [i]].name, i);
	}
*/

 if (completion.list_size > 0)
	{
		completion.window_pos = 0; // think about how to avoid resetting window_pos and select_line if the user selects something after the first line then keeps typing a word consistent with the selection
		completion.select_line = 0;


		completion.box_x = panel[PANEL_EDITOR].x1 + EDIT_WINDOW_X + (((se->cursor_pos+1) - se->window_pos) * editor.text_width) - SOURCE_WINDOW_MARGIN;
 	if (completion.box_x > panel[PANEL_EDITOR].x1 + panel[PANEL_EDITOR].w - COMPLETION_BOX_W)
	  completion.box_x = panel[PANEL_EDITOR].x1 + panel[PANEL_EDITOR].w - COMPLETION_BOX_W;
		completion.box_y = panel[PANEL_EDITOR].y1 + EDIT_WINDOW_Y + (se->cursor_line - se->window_line) * EDIT_LINE_H + EDIT_LINE_OFFSET + 12;
		completion.box_x2 = completion.box_x + COMPLETION_BOX_W;
		completion.box_lines = completion.list_size;
		if (completion.box_lines > COMPLETION_BOX_MAX_LINES)
			completion.box_lines = COMPLETION_BOX_MAX_LINES;
		completion.box_y2 = completion.box_y + 3 + completion.box_lines * COMPLETION_BOX_LINE_H;
	}


}



// gets a word at a location in a source_edit
// location must be valid (and not past the end of the line)
// returns length of the word (or 0 on failure)
static int get_word_from_editor(struct source_edit_struct* se, int source_line, int source_pos, int in_code_completion, char* check_word)
{

 int check_pos = source_pos;

 if (check_pos < MIN_COMPLETION_LENGTH)
	 return 0;

 char read_char;
 int read_char_type;
 int word_pos = 0;

// we look left from the cursor to find how long the current word is:

 while(TRUE)
	{
		if (check_pos < 0)
		{
			check_pos = 0;
			break;
		}
		read_char = se->text [se->line_index [source_line]] [check_pos];
		read_char_type = get_source_char_type(read_char);

		if (read_char_type != SCHAR_LETTER
			&& read_char_type != SCHAR_NUMBER)
		{
			check_pos++;
	  break;
		}
		check_pos --;
	};


	int check_word_length = source_pos - check_pos + 1;


	if (check_word_length >= IDENTIFIER_MAX_LENGTH - 1)
	{
		if (!in_code_completion)
			write_line_to_log("Word too long.", MLOG_COL_ERROR);
		return 0; // word too long
	}
	if (check_word_length <= MIN_COMPLETION_LENGTH)
	{
		if (!in_code_completion)
			write_line_to_log("Not a keyword.", MLOG_COL_ERROR);
		return 0; // word too short
	}

 if (in_code_completion)
	 completion.word_length = check_word_length;

// now we have a start and end position for a word.
// read the word into check_word:
 while(TRUE)
	{
		if (check_pos > source_pos)
			break;
		check_word [word_pos] = se->text [se->line_index [source_line]] [check_pos];
		check_word [word_pos + 1] = '\0';
		word_pos ++;
		check_pos ++;
	};

 return check_word_length;

}



// assumes that target bitmap is set
//  and clipping rectangle is suitable (probably needs to be able to draw on the entire panel)
void draw_code_completion_box(void)
{
 if	(completion.list_size == 0)
		return;

 int x = completion.box_x;
 int y = completion.box_y;

 al_draw_filled_rectangle(x, y, completion.box_x2, completion.box_y2, colours.base [COL_BLUE] [SHADE_LOW]);
 al_draw_rectangle(x + 0.5, y + 0.5, completion.box_x2 + 0.5, completion.box_y2 + 0.5, colours.base [COL_BLUE] [SHADE_HIGH], 1);
 int i;

 for (i = 0; i < completion.box_lines; i ++)
	{
		if (completion.select_line == i + completion.window_pos)
   al_draw_filled_rectangle(x + 1.5, y + i * COMPLETION_BOX_LINE_H + 2, completion.box_x2 - 0.5, y + i * COMPLETION_BOX_LINE_H + scaleUI_y(FONT_BASIC,14), colours.base [COL_BLUE] [SHADE_MED]);

  al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], x + 3, y + i * COMPLETION_BOX_LINE_H + COMPLETION_BOX_LINE_Y_OFFSET, ALLEGRO_ALIGN_LEFT, "%s", completion_table [completion.list_entry_index [i + completion.window_pos]]);

//		switch(completion.list_entry_type [i])
//		{
//			case COMPLETION_TYPE_NUMTOKEN:
//	   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], x + 3, y + i * COMPLETION_BOX_LINE_H + 4, ALLEGRO_ALIGN_LEFT, "%s", numtoken [completion.list_entry_index [i]].name);
//	   al_draw_textf(font[FONT_BASIC].fnt, colours.base [COL_BLUE] [SHADE_MAX], x + 3, y + i * COMPLETION_BOX_LINE_H + COMPLETION_BOX_LINE_Y_OFFSET, ALLEGRO_ALIGN_LEFT, "%s", numtoken [completion.list_entry_index [i + completion.window_pos]].name);
//	   break;
//		}
	}


}


void completion_box_select_line_up(void)
{

			completion.select_line --;
			if (completion.select_line < 0)
			{
				completion.select_line = completion.list_size - 1;
				if (completion.select_line >= completion.window_pos + completion.box_lines - 1)
					completion.window_pos = completion.select_line - completion.box_lines + 1;
			}
			 else
				{
			  if (completion.select_line == completion.window_pos - 1)
						completion.window_pos --;
				}

}

void completion_box_select_line_down(void)
{

			completion.select_line ++;
			if (completion.select_line >= completion.list_size)
			{
				completion.select_line = 0;
				completion.window_pos = 0;
			}
			 else
				{
				 if (completion.select_line >= completion.window_pos + completion.box_lines)
					 completion.window_pos = completion.select_line - completion.box_lines + 1;
				}

}


// like line_up but doesn't wrap
void completion_box_select_lines_up(int amount)
{

			completion.select_line -= amount;
			if (completion.select_line < 0)
				completion.select_line = 0;
			completion.window_pos = completion.select_line;

}

void completion_box_select_lines_down(int amount)
{

			completion.select_line += amount;
			if (completion.select_line >= completion.list_size)
				completion.select_line = completion.list_size - 1;
			if (completion.select_line >= completion.window_pos + completion.box_lines)
				completion.window_pos = completion.select_line - completion.box_lines + 1;
			if (completion.window_pos < 0)
			 completion.window_pos = 0;

}


void scroll_completion_box_up(int amount)
{

			completion.window_pos -= amount;
			if (completion.window_pos < 0)
					completion.window_pos = 0;

			if (completion.select_line > completion.window_pos + completion.box_lines)
				completion.select_line = completion.window_pos + completion.box_lines;

}

void scroll_completion_box_down(int amount)
{

			completion.window_pos += amount;
			if (completion.window_pos > completion.list_size - completion.box_lines)
				completion.window_pos = completion.list_size - completion.box_lines;
			if (completion.window_pos < 0)
					completion.window_pos = 0;

			if (completion.select_line < completion.window_pos)
				completion.select_line = completion.window_pos;

}




void complete_code(struct source_edit_struct* se, int select_line)
{

	if (se == NULL)
		return;

	if (select_line < 0
	 || select_line >= completion.list_size)
		return;

 char add_word [PTOKEN_LENGTH + 1];

		 strcpy(add_word, completion_table [completion.list_entry_index [select_line]]);


// switch(completion.list_entry_type [select_line])
// {
//  case COMPLETION_TYPE_NUMTOKEN:
//		 strcpy(add_word, numtoken[completion.list_entry_index [select_line]].name);
//		 break;
//	 default:
// 		return;
// }

 int i = completion.word_length;

 while(add_word [i] != '\0')
	{
		if (!add_char(add_word [i], 0))
		{
			return;
		}
		i++;
	};

	completion.list_size = 0;
// se->cursor_base = se->cursor_pos;

}


//#endif

