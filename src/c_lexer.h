
#ifndef H_C_LEXER
#define H_C_LEXER


int read_next(struct ctokenstruct* ctoken);
int accept_next(struct ctokenstruct* ctoken, int ctoken_type, int check_subtype);
int peek_next(struct ctokenstruct* ctoken);
int check_next(int ctoken_type, int check_subtype);

int expect_punctuation(int ctoken_subtype);
int expect_constant(struct ctokenstruct* ctoken);
int expect_angle(struct ctokenstruct* ctoken);

int c_get_next_char_from_scode(void);

#endif

