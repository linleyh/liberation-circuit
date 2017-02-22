
#ifndef H_C_COMPILE
#define H_C_COMPILE

int compile(struct template_struct* templ, struct source_edit_struct* source_edit, int compiler_mode);

int comp_error(int error_type, struct ctokenstruct* ctoken);
int comp_error_minus1(int error_type, struct ctokenstruct* ctoken);
int comp_error_text(const char* error_text, struct ctokenstruct* ctoken);
void comp_warning_text(const char* warning_text);

int check_template_objects(struct template_struct* templ, int warning_or_error);

#endif
