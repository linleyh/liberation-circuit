
#ifndef H_C_PREPR
#define H_C_PREPR

int preprocess(struct source_edit_struct* source_edit);
int valid_source_character(char read_char);

int load_source_file(const char* file_path, struct source_struct* target_source);
//int load_source_file(const char* file_path, struct source_struct* target_source);
//int load_binary_file(const char* file_path, struct bcode_struct* bcode, int src_file_index, int preprocessing);

// max length of a preprocessor token
#define PTOKEN_LENGTH 32

#define NUMTOKENS 700

// numtokens are built-in defined numbers
struct numtokenstruct
{
 char name [PTOKEN_LENGTH];
 int value;
};


#endif
