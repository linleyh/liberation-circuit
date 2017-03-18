
#ifndef H_E_COMPLETE
#define H_E_COMPLETE

void init_code_completion(void);

void check_code_completion(struct source_edit_struct* se, int from_backspace);
void draw_code_completion_box(void);
void complete_code(struct source_edit_struct* se, int select_line);
void completion_box_select_line_down(void);
void completion_box_select_line_up(void);
void completion_box_select_lines_up(int amount);
void completion_box_select_lines_down(int amount);
void scroll_completion_box_up(int amount);
void scroll_completion_box_down(int amount);



#define MIN_COMPLETION_LENGTH 3
// max is IDENTIFIER_MAX_LENGTH

#define COMPLETION_LIST_LENGTH 128

#define COMPLETION_BOX_MAX_LINES 12
#define COMPLETION_BOX_W scaleUI_x(FONT_BASIC,180)
#define COMPLETION_BOX_LINE_H scaleUI_y(FONT_BASIC,11)
#define COMPLETION_BOX_LINE_Y_OFFSET 4
// COMPLETION_BOX_LINE_Y_OFFSET is the number of pixels at the top of the completion box

#define COMPLETION_TABLE_STRING_LENGTH 36


enum
{
COMPLETION_TYPE_NUMTOKEN,
COMPLETION_TYPE_C_KEYWORD,
COMPLETION_TYPE_ASM_KEYWORD,
COMPLETION_TYPE_BUILTIN,
COMPLETION_TYPES
};

struct completionstruct
{
	int table_size; // size of sorted completion token table

	int list_size;
	int list_entry_type [COMPLETION_LIST_LENGTH];
	int list_entry_index [COMPLETION_LIST_LENGTH];

	int word_length; // length of current word

	int box_x, box_y, box_x2, box_y2; // coordinates on screen
	int box_lines;

	int window_pos; // position in list of top of window

	int select_line; // line selected by keyboard
};

extern struct completionstruct completion;




#endif
