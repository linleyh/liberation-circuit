
// header for interpreter, assembler and compiler modules

#ifndef H_C_HEADER
#define H_C_HEADER


#define IDENTIFIER_MAX_LENGTH 32

#define IDENTIFIERS 512

#define RECURSION_LIMIT 1000
// since the compiler uses recursive descent, we need to make sure it doesn't run out of stack space on bad input.


enum
{
COMPILE_MODE_TEST, // just makes sure the source compiles. Doesn't alter the current template.
COMPILE_MODE_BUILD, // full build (updates design and bcode) but doesn't fail if a template design problem (e.g. component collision) is found
COMPILE_MODE_LOCK, // like build, but fails (returns 0) if a template design problem is found
COMPILE_MODE_FIX, // just fixes the template design.
};


/*
enum
{
ID_NONE,

//C_ID_KEYWORD,

ID_USER_UNTYPED, // temporary type that is used during parsing to indicate that a token is a user-defined identifier before we know what type it is exactly. Later turned into e.g. ID_USER_INT.

ID_USER_INT, // user-defined variable - asm or c (auto variables and arrays are c only, though)
ID_USER_LABEL, // user-defined label - asm or c
ID_USER_LABEL_UNDEFINED, // label used but not yet defined
//ID_USER_CFUNCTION, // user-defined function name - compiler only (although probably could use in asm as well)
//ID_BUILTIN_CFUNCTION, // built-in cfunctions - e.g. put. compiler only.
ID_PROCESS, // the name of a process
ID_ENUM, // an enum. Is converted to a number by the parser, so the compiler doesn't really know the difference.
};

* need to use ctoken types for all of these!

*/

#define ARRAY_DIMENSIONS 3
#define ARRAY_DIMENSION_MAX_SIZE 512

struct identifierstruct
{

  char name [IDENTIFIER_MAX_LENGTH]; // name (null-terminated string
  int type; // the type of the identifier (a CTOKEN_TYPE value)
  int value; // purpose of this depends on type

  int address; // either address in memory (if a variable) or in bcode (if a label or similar). Defaults to -1;

// TO DO: possibly put the following values in a separate variable structure as they are irrelevant in many cases
//  int address_bcode; // the address that it points to in the bcode (actual address)
//  int declare_line; // line of source code in which it is declared
//  int declare_pos; // pos on line of source code in which it is declared - note that this is the END of the token (i.e. it's where src_pos is when the token has been read in)

  int array_dims;
  int array_dim_size [ARRAY_DIMENSIONS];
  int array_element_size [ARRAY_DIMENSIONS]; // the size of one element of this dimension (e.g. in a [5] [3] [2] the size of an element of the first dim is 6; the second is 2; the last is always 1)

//  int initial_value; // if it's e.g. an int variable, this is the value it is initialised to. Not used for arrays.

// when adding any values to this struct, remember to add initialisation code to c_init.c!!

};

enum
{
IC_NONE,
IC_OP,
IC_OP_WITH_VARIABLE_OPERAND,
IC_EXIT_POINT_TRUE,
IC_EXIT_POINT_FALSE,
IC_LABEL_DEFINITION,
IC_GOTO_LABEL,
IC_IFFALSE_JUMP_TO_EXIT_POINT,
IC_IFTRUE_JUMP_TO_EXIT_POINT,
IC_JUMP_EXIT_POINT_TRUE,
IC_JUMP_EXIT_POINT_FALSE,
IC_NUMBER, // just a number written directly to bcode
IC_SWITCH,
IC_JUMP_TABLE,
};

enum
{
LINE_WRAP_NO,
LINE_WRAP_YES
}; // what is this for??



enum
{
EXPOINT_TYPE_BASIC, // can't be used with break or continue (used for if)
EXPOINT_TYPE_LOOP, // break jumps to false point; continue jumps to true point
EXPOINT_TYPE_SWITCH // break jumps to false point; can't be used with continue
};

#define EXPOINTS 600

struct expointstruct
{
 int type;
// int true_point_icode; // location in intercode
// int false_point_icode;
 int true_point_bcode; // location in bcode. -1 if not defined
 int false_point_bcode;
 int true_point_used; // 1 if the point is used, 0 if not (this determines whether it will be defined if asm is generated)
 int false_point_used;
// int offset; // is added to the address of the expoint (currently only used by switch statements; the offset is the minimum case * -1)
}; // probably still need this.


struct ctokenstruct
{
 char name [IDENTIFIER_MAX_LENGTH + 3];

 int type; // what basic type?
 int subtype; // if the type is CTOKEN_TYPE_OPERATOR, this holds the type of operator it is

 int number_value; // if the type is CTOKEN_TYPE_NUMBER, holds the value

 int identifier_index; // if ctoken type is CTOKEN_TYPE_IDENTIFIER, this holds the index in the identifier array.

// int src_line; // line in original source code on which the ctoken occurs.
};




#define INTERCODE_VALUES 3
#define INTERCODE_SIZE 3000

struct intercode_struct
{
	int type;
	int value [INTERCODE_VALUES];
	int src_line; // don't worry about src_pos
};







struct source_struct
{
// lines in the text array should be null-terminated, although actually they don't have to be as each time text is used bounds-checking is done
 char text [SOURCE_TEXT_LINES] [SOURCE_TEXT_LINE_LENGTH];
//  *** text array must be the same as in source_struct (as the code that converts bcode to source code assumes that it can treat source.text in the same way as source_edit.text)

// int src_file [SOURCE_TEXT_LINES]; // stores the index of the file that the line came from
 int from_a_file; // is 1 if loaded from a file, 0 otherwise (will be 0 if a new empty source file created in the editor, until it's saved)
 char src_file_name [FILE_NAME_LENGTH]; // should be empty if from_a_file == 0
 char src_file_path [FILE_PATH_LENGTH]; // same
};



enum
{
CTOKEN_TYPE_NONE, // used to terminate identifier list
CTOKEN_TYPE_NUMBER,
CTOKEN_TYPE_OPERATOR_ARITHMETIC,
CTOKEN_TYPE_OPERATOR_LOGICAL,
CTOKEN_TYPE_OPERATOR_COMPARISON,
CTOKEN_TYPE_OPERATOR_ASSIGN, // includes = += ++ etc

CTOKEN_TYPE_IDENTIFIER_NEW, // newly created identifier of unknown type.

//CTOKEN_TYPE_IDENTIFIER_KEYWORD, // built-in keywords only
CTOKEN_TYPE_IDENTIFIER_CLASS,
//CTOKEN_TYPE_IDENTIFIER_USER_CFUNCTION,
CTOKEN_TYPE_IDENTIFIER_USER_VARIABLE,
//CTOKEN_TYPE_IDENTIFIER_USER_VARIABLE_UNDEFINED, // not used
//CTOKEN_TYPE_IDENTIFIER_BUILTIN_CFUNCTION,
//CTOKEN_TYPE_PROCESS, // the name of a process. Default process is called "self"
CTOKEN_TYPE_ENUM, // is converted to a number
CTOKEN_TYPE_IDENTIFIER_LABEL, // a code label (used for goto) that has been defined
CTOKEN_TYPE_IDENTIFIER_LABEL_UNDEFINED, // a code label that has been used (in a goto) but not yet defined

CTOKEN_TYPE_PUNCTUATION,

CTOKEN_TYPE_IDENTIFIER_CORE_SHAPE,
CTOKEN_TYPE_IDENTIFIER_SHAPE,
CTOKEN_TYPE_IDENTIFIER_OBJECT,
CTOKEN_TYPE_IDENTIFIER_C_KEYWORD, // keywords from standard C
CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD, // keywords like "process" that aren't in C
CTOKEN_TYPE_IDENTIFIER_OMETHOD, // object method
CTOKEN_TYPE_IDENTIFIER_MMETHOD, // member method
CTOKEN_TYPE_IDENTIFIER_CMETHOD, // core method
CTOKEN_TYPE_IDENTIFIER_SMETHOD, // standard method
CTOKEN_TYPE_IDENTIFIER_UMETHOD, // universal method

CTOKEN_TYPE_INVALID // error state

};


enum
{
CTOKEN_SUBTYPE_PLUS, // +
CTOKEN_SUBTYPE_PLUSEQ, // +=
CTOKEN_SUBTYPE_INCREMENT, // ++
CTOKEN_SUBTYPE_MINUS, // -
CTOKEN_SUBTYPE_MINUSEQ, // -=
CTOKEN_SUBTYPE_DECREMENT, // --
CTOKEN_SUBTYPE_MULEQ, // *=
CTOKEN_SUBTYPE_MUL, // *
CTOKEN_SUBTYPE_DIVEQ, // /=
CTOKEN_SUBTYPE_DIV, // /
CTOKEN_SUBTYPE_EQ_EQ, // ==
CTOKEN_SUBTYPE_EQ, // =
CTOKEN_SUBTYPE_LESEQ, // <=
CTOKEN_SUBTYPE_BITSHIFT_L, // <<
CTOKEN_SUBTYPE_BITSHIFT_L_EQ, // <<=
CTOKEN_SUBTYPE_LESS, // <
CTOKEN_SUBTYPE_GREQ, // >=
CTOKEN_SUBTYPE_BITSHIFT_R, // >>
CTOKEN_SUBTYPE_BITSHIFT_R_EQ, // >>=
CTOKEN_SUBTYPE_GR, // >
CTOKEN_SUBTYPE_BITWISE_AND_EQ, // &=
CTOKEN_SUBTYPE_LOGICAL_AND, // &&
CTOKEN_SUBTYPE_BITWISE_AND, // &
CTOKEN_SUBTYPE_BITWISE_OR_EQ, // |=
CTOKEN_SUBTYPE_LOGICAL_OR, // ||
CTOKEN_SUBTYPE_BITWISE_OR, // |
CTOKEN_SUBTYPE_BITWISE_XOR_EQ, // ^=
CTOKEN_SUBTYPE_BITWISE_XOR, // ^
CTOKEN_SUBTYPE_BITWISE_NOT_EQ, // ~=
CTOKEN_SUBTYPE_BITWISE_NOT, // ~
CTOKEN_SUBTYPE_MODEQ, // %=
CTOKEN_SUBTYPE_MOD, // %
CTOKEN_SUBTYPE_COMPARE_NOT, // !=
CTOKEN_SUBTYPE_NOT, // !

CTOKEN_SUBTYPE_BRACE_OPEN, // {
CTOKEN_SUBTYPE_BRACE_CLOSE, // }
CTOKEN_SUBTYPE_BRACKET_OPEN, // (
CTOKEN_SUBTYPE_BRACKET_CLOSE, // )
CTOKEN_SUBTYPE_SQUARE_OPEN, // [
CTOKEN_SUBTYPE_SQUARE_CLOSE, // ]
CTOKEN_SUBTYPE_COMMA, // ,
CTOKEN_SUBTYPE_SEMICOLON, // ;
CTOKEN_SUBTYPE_COLON, // :

CTOKEN_SUBTYPE_QUOTE, // '
CTOKEN_SUBTYPE_QUOTES, // "
CTOKEN_SUBTYPE_FULL_STOP, // .
CTOKEN_SUBTYPE_DOLLAR, // $
CTOKEN_SUBTYPE_HASH, // #

};

enum
{
ADDRESS_RESOLVE_LABEL,
ADDRESS_RESOLVE_EX_POINT_TRUE,
ADDRESS_RESOLVE_EX_POINT_FALSE
};

#define ADDRESS_RESOLUTION_ENTRIES 512
struct ic_address_resolution_struct
{
	int type;
	int bcode_pos; // the bcode op entry that needs to be updated
	int value; // the value it is updated to
};


#define REACHED_END_OF_SCODE -1001

#define SCODE_LENGTH 10000

// scode contains information from a sourcestruct that has been through the preprocessor.
struct scodestruct
{
 char text [SCODE_LENGTH]; // this is the source code, in one giant string.
 int text_length; // chars in source code
 s16b src_line [SCODE_LENGTH]; // this is the line number of each character in the text string.
// s16b src_line_pos [SCODE_LENGTH]; // same for position within the line
// s16b src_file [SCODE_LENGTH]; // file it came from
// char src_file_name [SOURCE_FILES] [FILE_NAME_LENGTH]; // name of the source file (SOURCE_FILES is the number of files that an scode can be derived from; it's the original file plus any #included files)

};

struct cstatestruct
{
 int compile_mode; // a COMPILER_MODE enum

 int src_line; // source line of current position - remember that this must be passed through source_edit->line_index to find the entry in the source_edit->text array!
 int src_pos; // position in that line
// int src_file; // which file the line is in
 int expoint_pos;
 int scode_pos;
 int reached_end_of_source;

 int error;
 int recursion_level;
 int just_returned; // is 1 if the last statement was return

 struct template_struct* templ; // target template
 struct source_edit_struct* source_edit; // source code
 struct scodestruct scode; // intermediate code (generated from source_edit by the preprocesser in c_prepr.c)

// memory use:
 int mem_pos; // keeps track of how much process memory has been allocated to variable storage

// intercode:
 int ic_pos;
 struct intercode_struct intercode [INTERCODE_SIZE];
 int resolve_pos;
 struct ic_address_resolution_struct ic_address_resolution [ADDRESS_RESOLUTION_ENTRIES];

// output
 int bc_pos; // used in the code generator
 struct bcode_struct* target_bcode;

 struct expointstruct expoint [EXPOINTS];
};

#define BCODE_POS_MIN 8
#define BCODE_POS_MAX (BCODE_MAX - BCODE_POS_MIN)
// BCODE_POS values allow a bit of a buffer to allow instructions to refer to next and previous instructions without bounds-checking

#define STRING_MAX_LENGTH 64
// This must not be less than BUBBLE_TEXT_LENGTH_MAX



enum
{
OP_nop,
OP_pushA,
OP_popB,
OP_add,
OP_sub_BA, // A = B-A
OP_sub_AB, // A = A-B
OP_mul,
OP_div_BA, // A = B/A
OP_div_AB, // A = A/B
OP_mod_BA, // A = B%A
OP_mod_AB, // A = A%B
OP_not, // ~
OP_and, // &
OP_or, // |
OP_xor, // ^
OP_lsh_BA, // A = B<<A
OP_lsh_AB, // A = A<<B
OP_rsh_BA, // A = B>>A
OP_rsh_AB, // A = A>>B
OP_lnot, // logical not !
OP_setA_num,
OP_setA_mem,
OP_copyA_to_mem,
OP_push_num,
OP_push_mem,
OP_incr_mem,
OP_decr_mem,
OP_jump_num,
OP_jumpA,
OP_comp_eq,
OP_comp_gr,
OP_comp_greq,
OP_comp_ls,
OP_comp_lseq,
OP_comp_neq,
OP_iftrue_jump,
OP_iffalse_jump,

OP_mulA_num,
OP_addA_num,
OP_derefA,
OP_copyA_to_derefB,
OP_incr_derefA,
OP_decr_derefA,
OP_copyAtoB,
OP_deref_stack_toA,

OP_push_return_address,
OP_return_sub,

OP_switchA,

OP_pcomp_eq, // process comparison
OP_pcomp_neq,

OP_print, // should be followed by null-terminated string
OP_printA, // prints contents of register A
OP_bubble, // should be followed by null-terminated string
OP_bubbleA, // prints contents of register A to bubble

OP_call_object,
OP_call_member,
OP_call_core,
OP_call_extern_member,
OP_call_extern_core,
OP_call_std,
OP_call_std_var, // smethod call with variable number of parameters
OP_call_uni,
OP_call_class,

OP_stop,
OP_terminate,

INSTRUCTIONS
};

#define INSTRUCTION_NAME_LENGTH 24
#define OPERANDS 3

enum
{
OPERAND_TYPE_NONE,
OPERAND_TYPE_MEMORY,
OPERAND_TYPE_NUMBER,
OPERAND_TYPE_BCODE_ADDRESS
};

struct instruction_set_struct
{
	char name [INSTRUCTION_NAME_LENGTH];
	int operands;
	int operand_type [OPERANDS];
};



enum
{
CERR_NONE,

CERR_RECURSION_LIMIT_REACHED,
CERR_SYNTAX_AT_STATEMENT_START,
CERR_TOO_MUCH_INTERCODE,
CERR_EXPECTED_SEMICOLON,
CERR_READ_FAIL,
CERR_SYNTAX_PUNCTUATION_IN_EXPRESSION,
CERR_NO_CLOSE_BRACKET,
CERR_SYNTAX_EXPRESSION_VALUE,
CERR_TOO_MANY_EXIT_POINTS,

// errors from the fixer (in c_fix.c):
CERR_FIXER_EXPECTED_PROCESS_HEADER,
CERR_FIXER_EXPECTED_CODE_HEADER,

// errors from the parser (in c_ctoken.c):
CERR_PARSER_SCODE_BOUNDS,
CERR_PARSER_REACHED_END_WHILE_READING,
CERR_PARSER_UNKNOWN_TOKEN_TYPE,
CERR_PARSER_NUMBER_TOO_LARGE,
CERR_PARSER_LETTER_IN_NUMBER,
CERR_PARSER_TOKEN_TOO_LONG,
CERR_PARSER_NUMBER_STARTS_WITH_ZERO,
CERR_PARSER_LETTER_IN_HEX_NUMBER,
CERR_INVALID_NUMBER_IN_BINARY_NUMBER,
CERR_PARSER_LETTER_IN_BINARY_NUMBER,
CERR_PARSER_TOO_MANY_IDENTIFIERS,

// errors from code generation (c_generate.c)
CERR_INTERCODE,

CERR_GENERIC, // used for errors that call comp_error_text with specific text.

CERRS
};


#endif
