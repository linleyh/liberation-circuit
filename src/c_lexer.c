
#include <allegro5/allegro.h>
#include "m_config.h"
#include "g_header.h"
#include "c_header.h"
#include "c_compile.h"

#include "g_misc.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "c_lexer.h"


enum
{
INITIAL_CTOKEN_TYPE_SINGLE_CHAR,
INITIAL_CTOKEN_TYPE_ONE_OR_TWO_CHAR,
// I don't think there's a need for INITIAL_CTOKEN_TYPE_DOUBLE_CHAR as I don't think there are any operators etc of this type.
INITIAL_CTOKEN_TYPE_NUMBER, // any number other than 0
INITIAL_CTOKEN_TYPE_ZERO, // may be 0 or a hex or binary number
INITIAL_CTOKEN_TYPE_IDENTIFIER,

INITIAL_CTOKEN_TYPE_INVALID
};

int c_get_next_char_from_scode(void);
int determine_initial_ctoken_type(struct ctokenstruct* ctoken, char read_source);
int get_ctoken_one_or_two_char(struct ctokenstruct* ctoken, char read_char);
int parse_ctoken(struct ctokenstruct* ctoken, int parse_type);

int get_ctoken_number(struct ctokenstruct* ctoken);
int get_ctoken_number_zero(struct ctokenstruct* ctoken);
int skip_spaces(void);
void print_identifier_list(void);
int read_identifier(struct ctokenstruct* ctoken, char read_char);

//extern struct scodestruct *scode;
extern struct cstatestruct cstate;
extern struct identifierstruct identifier [IDENTIFIERS]; // defined in c_keywords.c


// This function is called by the compiler. It reads the next ctoken from the scode.
// It starts by determining the basic type of the token from the first character.
// Then:
//  - if the first character is an operator or similar that can only be one character long (e.g. a comma), it fills out ctoken and returns.
//  - if the first character indicates that the token could be one or two characters long (e.g. '<', which could be < or <=) it checks the following character, then fills out ctoken and returns.
//  - if the first character is a number, it reads the token in as a number and puts its value in ctoken->value. Returns error if non-number found. Doesn't fill in name.
//  - if the first character is a valid identifier character, it reads the token in as an identifier and tries to find a matching identifier within current scope, or creates a new one).
//  - otherwise - it returns an error
// returns 1 if it finishes reading the ctoken without finding an error.
// returns 0 on error (including if it finds the end of scode - this shouldn't happen while reading a token).
// Don't call this to read the contents of a string
int read_next(struct ctokenstruct* ctoken)
{

 cstate.recursion_level ++;

 if (cstate.recursion_level > RECURSION_LIMIT)
  return comp_error(CERR_RECURSION_LIMIT_REACHED, NULL);

 int return_value = 1;

 strcpy(ctoken->name, "(empty)");

 if (cstate.error != CERR_NONE)
  return 0;

 if (cstate.scode_pos < -1 || cstate.scode_pos >= SCODE_LENGTH)
  return comp_error(CERR_PARSER_SCODE_BOUNDS, ctoken);

// int tp;
 int read_source;

// first we skip any spaces:
 int skipped = skip_spaces();

 if (skipped == 0)
	{
		cstate.reached_end_of_source = 1;
  return 0; // reached end of scode
	}

 cstate.src_line = cstate.scode.src_line [cstate.scode_pos];

// now we read the first character to work out what kind of ctoken we have here:

 read_source = c_get_next_char_from_scode();
 if (read_source == REACHED_END_OF_SCODE)
	{
		cstate.reached_end_of_source = 1;
  return 0; // reached end of source
	}

 char read_char = (char) read_source; // read_source is an int so c_get_next_char_from_scode can return REACHED_END_OF_SCODE

 int initial_ctoken_type = determine_initial_ctoken_type(ctoken, read_char); // passes ctoken because if it's a single-character token the structure is filled in

 switch(initial_ctoken_type)
 {
  case INITIAL_CTOKEN_TYPE_INVALID:
//   fprintf(stdout, "\nCtoken: invalid");
   ctoken->type = CTOKEN_TYPE_INVALID;
   return 0;
  case INITIAL_CTOKEN_TYPE_SINGLE_CHAR: // In this case, determine_initial_ctoken_type() has already filled in the ctokenstruct, so we can just return.
   ctoken->name [0] = read_char;
   ctoken->name [1] = 0;
//   fprintf(stdout, "\nCtoken: %s (single character)", ctoken->name);
   return_value = 1;
   goto parse_ctoken_success;
  case INITIAL_CTOKEN_TYPE_ONE_OR_TWO_CHAR:
   if (!get_ctoken_one_or_two_char(ctoken, read_char)) // This function can fail (as reading the second character could reach the end of scode, which will be an error)
    return comp_error(CERR_PARSER_SCODE_BOUNDS, ctoken);
//   fprintf(stdout, "\nCtoken: single or double character");
   // if it doesn't fail, the ctokenstruct will have been filled in.
   return_value = 1;
   goto parse_ctoken_success;
  case INITIAL_CTOKEN_TYPE_NUMBER:
   cstate.scode_pos --;
   if (!get_ctoken_number(ctoken))
    return 0;
//    fprintf(stdout, "\nCtoken: %i (single number)", ctoken->number_value);
   return_value = 1;
   goto parse_ctoken_success;
  case INITIAL_CTOKEN_TYPE_ZERO:
   cstate.scode_pos --;
// any changes here may need to be reflected in the enum-reading code below in case INITIAL_CTOKEN_TYPE_IDENTIFIER
   if (!get_ctoken_number_zero(ctoken))
    return 0;
//    fprintf(stdout, "\nCtoken: %i (single number)", ctoken->number_value);
   return_value = 1;
   goto parse_ctoken_success;
// if it doesn't fail, the ctokenstruct will have been filled in.
  case INITIAL_CTOKEN_TYPE_IDENTIFIER:
   ctoken->identifier_index = read_identifier(ctoken, read_char);
   if (ctoken->identifier_index == -1) // This function can fail (as reading further characters could reach the end of scode, which will be an error)
    return comp_error(CERR_PARSER_REACHED_END_WHILE_READING, ctoken);
// if it's an enum, it's converted into a number. The code here should correspond to the code for number ctokens above
   if (ctoken->type == CTOKEN_TYPE_ENUM)
   {
    ctoken->number_value = identifier[ctoken->identifier_index].value;
    ctoken->type = CTOKEN_TYPE_NUMBER;
   }
   return_value = 1;
   goto parse_ctoken_success;
// if it doesn't fail, the ctokenstruct will have been filled in.

 } // end switch(initial_ctoken_type)

 return comp_error(CERR_PARSER_UNKNOWN_TOKEN_TYPE, ctoken);

parse_ctoken_success:

 cstate.recursion_level --;
 return return_value;

}


// This function will be changed to verify the range of the angle somehow
int expect_angle(struct ctokenstruct* ctoken)
{
	if (!expect_constant(ctoken))
		return 0;
// Note: can't & ANGLE_MASK because this may be a negative angle offset
	return 1;
}

// reads a constant and leaves its value in ctoken->number_value.
//  does not currently perform constant folding! sorry
// returns 1 on success, 0 on failure (no constant found means failure)
int expect_constant(struct ctokenstruct* ctoken)
{
	int sign = 1;
	if (!read_next(ctoken))
		return 0;
// check for negative number
 if (ctoken->type == CTOKEN_TYPE_OPERATOR_ARITHMETIC
		&& ctoken->subtype == CTOKEN_SUBTYPE_MINUS)
	{
		sign = -1;
		if (!read_next(ctoken))
			return 0;
	}
// now ctoken must be a number
 if (ctoken->type != CTOKEN_TYPE_NUMBER)
		return 0;
	ctoken->number_value *= sign;
	return 1;

}


// returns a value from 0 to 255 on success (I think this may not necessarily work on systems that implement char in a different way. Not sure about this)
// returns REACHED_END_OF_SOURCE if end of source reached
// leaves src_pos with a new value (which may be at end of the scode struct)
// assumes scode text array is null-terminated.
// can be called directly from c_compile.c (e.g. in reading a string)
int c_get_next_char_from_scode(void)
{

 cstate.scode_pos ++;

 if (cstate.scode.text [cstate.scode_pos] == '\0') // reached end
	{
  return REACHED_END_OF_SCODE;
	}
/*
 char tstr[2];
 tstr[0] = cstate.scode.text [cstate.scode_pos];
 tstr[1] = 0;
 fprintf(stdout, "%s", tstr);*/

 return cstate.scode.text [cstate.scode_pos];

}

/*
This function works out what is indicated about a ctoken by its first character.
If the first character is a character that is always an entire token by itself, the function also fills in the ctoken structure.
Otherwise, it does not.
Returns the type of ctoken it is (returns INITIAL_CTOKEN_TYPE_INVALID on invalid token)
*/
int determine_initial_ctoken_type(struct ctokenstruct* ctoken, char read_source)
{

 if ((read_source >= 'a' && read_source <= 'z')
  || (read_source >= 'A' && read_source <= 'Z')
  || read_source == '_')
   return INITIAL_CTOKEN_TYPE_IDENTIFIER;

 if (read_source == '0')
  return INITIAL_CTOKEN_TYPE_ZERO; // may be 0 or a hex/bin number

 if (read_source >= '1' && read_source <= '9')
   return INITIAL_CTOKEN_TYPE_NUMBER;

 switch(read_source)
 {
  case '(': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_BRACKET_OPEN; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
  case ')': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_BRACKET_CLOSE; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
  case '{': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_BRACE_OPEN; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
  case '}': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_BRACE_CLOSE; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
  case '[': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_SQUARE_OPEN; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
  case ']': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_SQUARE_CLOSE; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
//  case '\'': ctoken.type = CTOKEN_TYPE_PUNCTUATION;
//            ctoken.subtype = CTOKEN_SUBTYPE_DIV; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
  case '\'': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_QUOTE; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
  case '"': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_QUOTES; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
  case ';': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_SEMICOLON; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
  case ':': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_COLON; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
  case ',': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_COMMA; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
  case '.': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_FULL_STOP; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
  case '$': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_DOLLAR; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
  case '#': ctoken->type = CTOKEN_TYPE_PUNCTUATION;
            ctoken->subtype = CTOKEN_SUBTYPE_HASH; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;
   // these characters are only ever parsed individually, so we can actually fill out ctokenstruct at this point
  case '~': ctoken->type = CTOKEN_TYPE_OPERATOR_ARITHMETIC;
            ctoken->subtype = CTOKEN_SUBTYPE_BITWISE_NOT; return INITIAL_CTOKEN_TYPE_SINGLE_CHAR;

  case '+': // could be + +=
  case '-': // - -= ->
  case '*': // * *=
  case '/': // / /=
  case '=': // = ==
  case '<': // < <= <<
  case '>': // > >= >>
  case '&': // & &= &&
  case '|': // | |= ||
  case '^': // ^ ^=
  case '%': // % %=
  case '!': // ! !=
   return INITIAL_CTOKEN_TYPE_ONE_OR_TWO_CHAR; // these characters might be individual, but might also be part of a 2-char operator or similar (e.g. +=)
// details of these are filled out in get_ctoken_one_or_two_char

  default: return INITIAL_CTOKEN_TYPE_INVALID; // invalid char

 }

 return INITIAL_CTOKEN_TYPE_INVALID; // invalid char

}


/*
This function is called if a character that could be a one- or two-character token is found.
Returns 1 on success (read a token) or 0 on error (which I guess can only happen if the end of scode is reached)
*/
int get_ctoken_one_or_two_char(struct ctokenstruct* ctoken, char read_char)
{

 int read_source2 = c_get_next_char_from_scode();
 if (read_source2 == REACHED_END_OF_SCODE)
  return 0; // reached end of scode (error)

 char read_char2 = (char) read_source2;

 ctoken->type = CTOKEN_TYPE_OPERATOR_ARITHMETIC; // default

 switch(read_char)
 {
  case '+': // could be + ++ +=
   switch(read_char2)
   {
    case '=': ctoken->type = CTOKEN_TYPE_OPERATOR_ASSIGN;
     ctoken->subtype = CTOKEN_SUBTYPE_PLUSEQ; return 1;
    case '+': ctoken->type = CTOKEN_TYPE_OPERATOR_ASSIGN;
     ctoken->subtype = CTOKEN_SUBTYPE_INCREMENT; return 1;
   }
   ctoken->subtype = CTOKEN_SUBTYPE_PLUS;
   cstate.scode_pos--;
   return 1;
  case '-': // - -= ->
   switch(read_char2)
   {
    case '=': ctoken->type = CTOKEN_TYPE_OPERATOR_ASSIGN;
     ctoken->subtype = CTOKEN_SUBTYPE_MINUSEQ; return 1;
    case '-': ctoken->type = CTOKEN_TYPE_OPERATOR_ASSIGN;
     ctoken->subtype = CTOKEN_SUBTYPE_DECREMENT; return 1;
   }
// need to deal with the possibility that this is a negative number!
   ctoken->subtype = CTOKEN_SUBTYPE_MINUS;
   cstate.scode_pos--;
   return 1;
  case '*': // could be * *= *(pointer)
   switch(read_char2)
   {
    case '=': ctoken->type = CTOKEN_TYPE_OPERATOR_ASSIGN;
     ctoken->subtype = CTOKEN_SUBTYPE_MULEQ; return 1;
   }
   ctoken->subtype = CTOKEN_SUBTYPE_MUL;
   cstate.scode_pos--;
   return 1;
  case '/': // / /=
   switch(read_char2)
   {
    case '=': ctoken->type = CTOKEN_TYPE_OPERATOR_ASSIGN;
     ctoken->subtype = CTOKEN_SUBTYPE_DIVEQ; return 1;
   }
   ctoken->subtype = CTOKEN_SUBTYPE_DIV;
   cstate.scode_pos--;
   return 1;
  case '=': // = ==
   switch(read_char2)
   {
    case '=': ctoken->type = CTOKEN_TYPE_OPERATOR_COMPARISON;
     ctoken->subtype = CTOKEN_SUBTYPE_EQ_EQ; return 1;
   }
   ctoken->type = CTOKEN_TYPE_OPERATOR_ASSIGN;
   ctoken->subtype = CTOKEN_SUBTYPE_EQ;
   cstate.scode_pos--;
   return 1;
  case '<': // < <= <<
   switch(read_char2)
   {
    case '=': ctoken->type = CTOKEN_TYPE_OPERATOR_COMPARISON;
     ctoken->subtype = CTOKEN_SUBTYPE_LESEQ; return 1;
    case '<': ctoken->subtype = CTOKEN_SUBTYPE_BITSHIFT_L; return 1;
   }
   ctoken->type = CTOKEN_TYPE_OPERATOR_COMPARISON;
   ctoken->subtype = CTOKEN_SUBTYPE_LESS;
   cstate.scode_pos--;
   return 1;
  case '>': // > >= >>
   switch(read_char2)
   {
    case '=': ctoken->type = CTOKEN_TYPE_OPERATOR_COMPARISON;
     ctoken->subtype = CTOKEN_SUBTYPE_GREQ; return 1;
    case '>': ctoken->subtype = CTOKEN_SUBTYPE_BITSHIFT_R; return 1;
   }
   ctoken->type = CTOKEN_TYPE_OPERATOR_COMPARISON;
   ctoken->subtype = CTOKEN_SUBTYPE_GR;
   cstate.scode_pos--;
   return 1;
  case '&': // & &= &&
   switch(read_char2)
   {
    case '=': ctoken->type = CTOKEN_TYPE_OPERATOR_ASSIGN;
     ctoken->subtype = CTOKEN_SUBTYPE_BITWISE_AND_EQ; return 1;
    case '&': ctoken->type = CTOKEN_TYPE_OPERATOR_LOGICAL;
     ctoken->subtype = CTOKEN_SUBTYPE_LOGICAL_AND; return 1;
   }
   ctoken->subtype = CTOKEN_SUBTYPE_BITWISE_AND;
   cstate.scode_pos--;
   return 1;
  case '|': // | |= ||
   switch(read_char2)
   {
    case '=': ctoken->type = CTOKEN_TYPE_OPERATOR_ASSIGN;
     ctoken->subtype = CTOKEN_SUBTYPE_BITWISE_OR_EQ; return 1;
    case '|': ctoken->type = CTOKEN_TYPE_OPERATOR_LOGICAL;
     ctoken->subtype = CTOKEN_SUBTYPE_LOGICAL_OR; return 1;
   }
   ctoken->subtype = CTOKEN_SUBTYPE_BITWISE_OR;
   cstate.scode_pos--;
   return 1;
  case '^': // ^ ^=
   switch(read_char2)
   {
    case '=': ctoken->type = CTOKEN_TYPE_OPERATOR_ASSIGN;
     ctoken->subtype = CTOKEN_SUBTYPE_BITWISE_XOR_EQ; return 1;
   }
   ctoken->subtype = CTOKEN_SUBTYPE_BITWISE_XOR;
   cstate.scode_pos--;
   return 1;
  case '~': // ~ ~=
   switch(read_char2)
   {
    case '=': ctoken->type = CTOKEN_TYPE_OPERATOR_ASSIGN;
     ctoken->subtype = CTOKEN_SUBTYPE_BITWISE_NOT_EQ; return 1;
   }
   ctoken->subtype = CTOKEN_SUBTYPE_BITWISE_NOT;
   cstate.scode_pos--;
   return 1;
  case '%': // % %=
   switch(read_char2)
   {
    case '=': ctoken->type = CTOKEN_TYPE_OPERATOR_ASSIGN;
     ctoken->subtype = CTOKEN_SUBTYPE_MODEQ; return 1;
   }
   ctoken->subtype = CTOKEN_SUBTYPE_MOD;
   cstate.scode_pos--;
   return 1;
  case '!': // ! !=
   switch(read_char2)
   {
    case '=': ctoken->type = CTOKEN_TYPE_OPERATOR_COMPARISON;
     ctoken->subtype = CTOKEN_SUBTYPE_COMPARE_NOT; return 1;
   }
   ctoken->subtype = CTOKEN_SUBTYPE_NOT;
   cstate.scode_pos--;
   return 1;


 }

 return 0;

}

// Assumes that scode_pos is set so that the next char read will be the first char of a number (could be the only char of the number).
// assumes that if the number is negative and starts with -, the - has already been read (see read_number_value())
int get_ctoken_number(struct ctokenstruct* ctoken)
{

 int number [10] = {0,0,0,0,0,0,0,0,0,0};
 int i;
 int read_source2;
 char read_char;

 i = 0;

 do
 {
  read_source2 = c_get_next_char_from_scode();
  if (read_source2 == REACHED_END_OF_SCODE)
   return comp_error(CERR_PARSER_REACHED_END_WHILE_READING, ctoken);

  read_char = (char) read_source2;

  if (read_char >= '0' && read_char <= '9')
  {
   if (i > 5) // maximum length of 16-bit int as a decimal string
    return comp_error(CERR_PARSER_NUMBER_TOO_LARGE, ctoken);
   number [i] = read_char - '0';
   i ++;
   continue;
  }

  if ((read_char >= 'a' && read_char <= 'z')
   || (read_char >= 'A' && read_char <= 'Z')
   || read_char == '_')
    return comp_error(CERR_PARSER_LETTER_IN_NUMBER, ctoken);

// if it's not a number or a letter, it's probably a space or an operator. So we stop reading the number, decrement *scode_pos and let whatever's been found be dealt with as the next ctoken:
  cstate.scode_pos --;
  break;

 } while (TRUE);

 int ctoken_value = 0;
// at this point we assume that the number of digits in the number is (i + 1):
 int j = 0;
 int multiplier = 1;

 for (j = i - 1; j >= 0; j --)
 {
  ctoken_value += number [j] * multiplier;
  multiplier *= 10;
 }

 if (ctoken_value > BCODE_VALUE_MAXIMUM
  || ctoken_value < BCODE_VALUE_MINIMUM)
   return comp_error(CERR_PARSER_NUMBER_TOO_LARGE, ctoken);

 ctoken->type = CTOKEN_TYPE_NUMBER;
 ctoken->number_value = ctoken_value;

// fprintf(stdout, "\nread number: %i", ctoken_value);

 return 1;


}

#define NUMBER_STRING_SIZE 20
// NUMBER_STRING_SIZE must be large enough to store a 16-digit binary number

// Like get_ctoken_number, but is called when first digit of a number is 0.
// This can mean it's just a zero, or that it's a hex number in 0x form.
int get_ctoken_number_zero(struct ctokenstruct* ctoken)
{

 int number [NUMBER_STRING_SIZE];
 int i;
 int read_source2;
 char read_char;
 int ctoken_value;
 int j;
 int multiplier;

 for (i = 0; i < NUMBER_STRING_SIZE; i ++)
 {
  number [i] = 0;
 }

 i = 0;

// read the zero again:
 read_source2 = c_get_next_char_from_scode();
 if (read_source2 == REACHED_END_OF_SCODE)
  return comp_error(CERR_PARSER_REACHED_END_WHILE_READING, ctoken);

// fprintf(stdout, "\nget_ctoken_number_zero");

// the zero has been read in already, so there are various possibilites as to what comes next:
//  - the next character is x, which means this is a hex number
//  - the next character is invalid (e.g. out of bounds) so this is an error
//  - the next character is also a digit, so this is an error (decimal numbers shouldn't start with 0, and octal numbers are not supported)
//  - any other case - the number is just zero.

 read_source2 = c_get_next_char_from_scode();
 if (read_source2 == REACHED_END_OF_SCODE)
  return comp_error(CERR_PARSER_REACHED_END_WHILE_READING, ctoken);

 read_char = (char) read_source2;

 if (read_char >= '0' && read_char <= '9')
  return comp_error(CERR_PARSER_NUMBER_STARTS_WITH_ZERO, ctoken);


// May be a number in 0x hex form:
 if (read_char == 'x'
  || read_char == 'X')
 {
  do
  {
   read_source2 = c_get_next_char_from_scode();
   if (read_source2 == REACHED_END_OF_SCODE)
    return comp_error(CERR_PARSER_REACHED_END_WHILE_READING, ctoken);

   read_char = (char) read_source2;

   if (read_char >= '0' && read_char <= '9')
   {
    if (i > 4) // maximum length of 16-bit int as a hex string
     return comp_error(CERR_PARSER_NUMBER_TOO_LARGE, ctoken);
    number [i] = read_char - '0';
    i ++;
    continue;
   }

   if (read_char >= 'a' && read_char <= 'f')
   {
    if (i > 4) // maximum length of 16-bit int as a hex string
     return comp_error(CERR_PARSER_NUMBER_TOO_LARGE, ctoken);
    number [i] = read_char - 'a' + 10;
    i ++;
    continue;
   }

   if (read_char >= 'A' && read_char <= 'F')
   {
    if (i > 4) // maximum length of 16-bit int as a hex string
     return comp_error(CERR_PARSER_NUMBER_TOO_LARGE, ctoken);
    number [i] = read_char - 'A' + 10;
    i ++;
    continue;
   }

   if ((read_char > 'f' && read_char <= 'z')
    || (read_char > 'F' && read_char <= 'Z')
    || read_char == '_')
     return comp_error(CERR_PARSER_LETTER_IN_HEX_NUMBER, ctoken);

// if it's not a number or a letter, it's probably a space or an operator. So we stop reading the number, decrement scode_pos and let whatever's been found be dealt with as the next ctoken:
   cstate.scode_pos --;
   break;

  } while (TRUE);

  ctoken_value = 0;
// at this point we assume that the number of digits in the number is (i + 1):
  j = 0;
  multiplier = 1;

  for (j = i - 1; j >= 0; j --)
  {
   ctoken_value += number [j] * multiplier;
   multiplier *= 16;
  }

// need to convert unsigned hex number to s16b:
//  if (ctoken_value > 0x7FFF)
//			ctoken_value *= -1; // will this work? I hope so!

  s16b ctoken_value_16b = ctoken_value;

//  if (ctoken_value > BCODE_VALUE_MAXIMUM
//   || ctoken_value < BCODE_VALUE_MINIMUM)
//    return comp_error(CERR_PARSER_NUMBER_TOO_LARGE, ctoken);

  ctoken->type = CTOKEN_TYPE_NUMBER;
  ctoken->number_value = ctoken_value_16b;//ctoken_value;

  return 1;
 } // end hex




// may be a binary number (0b prefix)
 if (read_char == 'b'
  || read_char == 'B')
 {

  do
  {
   read_source2 = c_get_next_char_from_scode();
   if (read_source2 == REACHED_END_OF_SCODE)
    return comp_error(CERR_PARSER_REACHED_END_WHILE_READING, ctoken);

   read_char = (char) read_source2;

   if (read_char == '0' || read_char == '1')
   {
    if (i > 15) // maximum length of 16-bit int
     return comp_error(CERR_PARSER_NUMBER_TOO_LARGE, ctoken);
    number [i] = read_char - '0';
    i ++;
    continue;
   }

   if (read_char >= '2' && read_char <= '9')
     return comp_error(CERR_INVALID_NUMBER_IN_BINARY_NUMBER, ctoken);

   if ((read_char >= 'a' && read_char <= 'z')
    || (read_char >= 'A' && read_char <= 'Z')
    || read_char == '_')
     return comp_error(CERR_PARSER_LETTER_IN_BINARY_NUMBER, ctoken);

// if it's not a number or a letter, it's probably a space or an operator. So we stop reading the number, decrement *scode_pos and let whatever's been found be dealt with as the next ctoken:
   cstate.scode_pos --;
   break;

  } while (TRUE);

  ctoken_value = 0;
// at this point we assume that the number of digits in the number is (i + 1):
  j = 0;
  multiplier = 0;

  for (j = i - 1; j >= 0; j --)
  {
   if (number [j] == 1)
    ctoken_value += 1 << multiplier;
   multiplier ++;
  }

  if (ctoken_value > BCODE_VALUE_MAXIMUM
   || ctoken_value < BCODE_VALUE_MINIMUM)
    return comp_error(CERR_PARSER_NUMBER_TOO_LARGE, ctoken);

  ctoken->type = CTOKEN_TYPE_NUMBER;
  ctoken->number_value = ctoken_value;

  return 1;


 }


// number was just a zero followed by something else, so we set up a zero number ctoken and return.
  ctoken->type = CTOKEN_TYPE_NUMBER;
  ctoken->number_value = 0;
  cstate.scode_pos --;
  return 1;

}


/*
Returns index of identifier on success
read_char is the first character of the token (this function starts from the second)
Returns -1 on failure
*/
int read_identifier(struct ctokenstruct* ctoken, char read_char)
{

 int i;


 int read_source2;
 char read_char2;

 ctoken->name [0] = read_char;
 ctoken->name [1] = '\0';
 ctoken->type = CTOKEN_TYPE_IDENTIFIER_NEW; // this value can be replaced below

 i = 0;

 do
 {
  read_source2 = c_get_next_char_from_scode();
  if (read_source2 == REACHED_END_OF_SCODE)
   return -1; // reached end of scode (error)

  read_char2 = (char) read_source2;

  if ((read_char2 >= 'a' && read_char2 <= 'z')
   || (read_char2 >= 'A' && read_char2 <= 'Z')
   || (read_char2 >= '0' && read_char2 <= '9') // an identifier name can contain numbers; it just can't start with a number
   || read_char2 == '_')
  {
   i ++;
   if (i >= IDENTIFIER_MAX_LENGTH)
    return comp_error_minus1(CERR_PARSER_TOKEN_TOO_LONG, ctoken);
   ctoken->name [i] = read_char2;
   ctoken->name [i + 1] = '\0';
   continue;
  }

//  if (read_char2 == '.') // check for out-of-scope references
//  {
//   return out_of_scope_reference(ctoken);
//  }

// if it's not a number or a letter, it's probably a space or an operator. So we stop reading the number and let whatever's been found be dealt with as the next ctoken:
  cstate.scode_pos --;
  break;

 } while (TRUE);

 for (i = 0; i < IDENTIFIERS; i ++)
 {
  if (identifier[i].type == CTOKEN_TYPE_NONE)
   break;

  if (strcmp(ctoken->name, identifier[i].name) == 0) // match found!
  {
			ctoken->type = identifier[i].type;
   return i; // ctoken->identifier_index is set to this return value
  }
 }

 if (i >= IDENTIFIERS - 1)
  return comp_error(CERR_PARSER_TOO_MANY_IDENTIFIERS, ctoken);

// create a new untyped identifier:

 strcpy(identifier[i].name, ctoken->name);
 identifier[i].type = CTOKEN_TYPE_IDENTIFIER_NEW;
 identifier[i].value = 0;
 identifier[i].address = -1; // indicates undefined (for identifiers with addresses)

// terminate the identifier list
 identifier[i+1].type = CTOKEN_TYPE_NONE;

 ctoken->type = CTOKEN_TYPE_IDENTIFIER_NEW;
 ctoken->identifier_index = i;

 return i; // ctoken->identifier_index is set to this return value

// return new_c_identifier(ctoken, ID_USER_UNTYPED); // return value is -1 on failure


}





// this function checks whether the next ctoken is of ctoken_type. For some ctoken types, also checks against check_subtype if it isn't -1.
// returns 1 if yes, zero if no (or if error).
// if yes, advances cstate.scode_pos and fills in ctoken for use by calling function
// if no, returns 0 without advancing cstate.scode_pos and without writing an error message.
// in either case, may create a new identifier (of IDENTIFIER_NEW type)
// if check_subtype is -1, only checks type
int accept_next(struct ctokenstruct* ctoken, int ctoken_type, int check_subtype)
{

 int save_scode_pos = cstate.scode_pos;

 if (!read_next(ctoken))
		goto accept_failed;

	if (ctoken->type != ctoken_type)
		goto accept_failed;


	if (check_subtype == -1)
			return 1;


	switch(ctoken->type)
	{
	 case CTOKEN_TYPE_IDENTIFIER_C_KEYWORD:
		case CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD:
	 	if (ctoken->identifier_index != check_subtype)
				goto accept_failed;
			return 1; // this allows e.g. accept_next(ctoken, CTOKEN_TYPE_IDENTIFIER_C_KEYWORD, KEYWORD_C_INT);
		case CTOKEN_TYPE_OPERATOR_ARITHMETIC:
		case CTOKEN_TYPE_OPERATOR_ASSIGN:
		case CTOKEN_TYPE_OPERATOR_COMPARISON:
		case CTOKEN_TYPE_OPERATOR_LOGICAL:
		case CTOKEN_TYPE_PUNCTUATION:
	 	if (ctoken->subtype != check_subtype)
				goto accept_failed;
			return 1;
// no other types should be called with check_subtype != -1
	}

accept_failed:

 cstate.scode_pos = save_scode_pos;
 return 0;

}


// same as accept_next but doesn't fill in a ctoken from the calling function
int check_next(int ctoken_type, int check_subtype)
{

 int save_scode_pos = cstate.scode_pos;

 struct ctokenstruct ctoken;

 if (!read_next(&ctoken))
		goto accept_failed;

	if (ctoken.type != ctoken_type)
		goto accept_failed;


	if (check_subtype == -1)
			return 1;


	switch(ctoken.type)
	{
	 case CTOKEN_TYPE_IDENTIFIER_C_KEYWORD:
		case CTOKEN_TYPE_IDENTIFIER_NON_C_KEYWORD:
	 	if (ctoken.identifier_index != check_subtype)
				goto accept_failed;
			return 1;
		case CTOKEN_TYPE_OPERATOR_ARITHMETIC:
		case CTOKEN_TYPE_OPERATOR_ASSIGN:
		case CTOKEN_TYPE_OPERATOR_COMPARISON:
		case CTOKEN_TYPE_OPERATOR_LOGICAL:
		case CTOKEN_TYPE_PUNCTUATION:
	 	if (ctoken.subtype != check_subtype)
				goto accept_failed;
			return 1;
// no other types should be called with check_subtype != -1
	}

accept_failed:

 cstate.scode_pos = save_scode_pos;
 return 0; // may not be an error

}



int expect_punctuation(int ctoken_subtype)
{

 struct ctokenstruct ctoken;

 if (!read_next(&ctoken))
		return 0;

	if (ctoken.type != CTOKEN_TYPE_PUNCTUATION)
		return 0;

	if (ctoken_subtype == -1
		|| ctoken.subtype == ctoken_subtype)
			return 1;

		return 0;

}


// peek next looks at the next ctoken and fills in the ctoken struct, but does not advance scode_pos
int peek_next(struct ctokenstruct* ctoken)
{

 int save_scode_pos = cstate.scode_pos;

 int retval = read_next(ctoken);

 cstate.scode_pos = save_scode_pos;

 return retval;

}


int skip_spaces(void)
{

 int read_source;

 while (TRUE)
 {
  read_source = c_get_next_char_from_scode(); // note scode_pos is a pointer, and is incremented by get_next_char_from_scode
  if (read_source == REACHED_END_OF_SCODE)
		{
   return 0;
		}
  if (read_source != ' ')
  {
   (cstate.scode_pos) --;
   return 1;
  }
 };

 return 0; // should never be reached
}
/*
// debugging function - not currently used
void print_identifier_list(void)
{
 int i;

   fprintf(stdout, "\n Identifier list");


 for (i = 0; i < IDENTIFIERS; i ++)
 {
  if (identifier[i].type != ID_NONE)
  {
   fprintf(stdout, "\nid %2d ", i);
   switch(identifier[i].type)
   {
    case C_ID_KEYWORD: fprintf(stdout, "keyword"); break;
    case ID_USER_UNTYPED: fprintf(stdout, "untyped"); break;
    case ID_USER_INT: fprintf(stdout, "int"); break;
    case ID_USER_LABEL: fprintf(stdout, "label"); break;
    case ID_USER_LABEL_UNDEFINED: fprintf(stdout, "undefined label"); break;
    case ID_USER_CFUNCTION: fprintf(stdout, "cfunction"); break;
    case ID_PROCESS: fprintf(stdout, "process"); break;
    case ID_ENUM: fprintf(stdout, "enum"); break;
    case ID_BUILTIN_CFUNCTION: fprintf(stdout, "built-in cfunction"); break;
    case ASM_ID_OPCODE: fprintf(stdout, "asm opcode"); break;
    case ASM_ID_KEYWORD: fprintf(stdout, "asm keyword"); break;
    case ASM_ID_GENERIC_UNDEFINED: fprintf(stdout, "asm generic undefined"); break;
    case ASM_ID_GENERIC_DEFINED: fprintf(stdout, "asm generic defined"); break;
    case ASM_ID_GENERIC_ALIAS: fprintf(stdout, "asm generic alias"); break;
    case ASM_ID_ASPACE: fprintf(stdout, "asm aspace"); break;
    case ASM_ID_NSPACE: fprintf(stdout, "asm nspace"); break;
    default: fprintf(stdout, "unknown (%i)", identifier[i].type);
   }
   fprintf(stdout, " \"%s\" pr_scope %i scope %i as_scope %i ns_scope %i", identifier[i].name, identifier[i].process_scope, identifier[i].scope, identifier[i].aspace_scope, identifier[i].nspace_scope);

  }

 }

// error_call();
}
*/
