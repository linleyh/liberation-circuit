
#ifndef H_F_LOAD
#define H_F_LOAD



#define LOAD_BUFFER_SIZE 8192

struct load_statestruct
{
 ALLEGRO_FILECHOOSER* file_dialog;
 FILE *file;
 char buffer [LOAD_BUFFER_SIZE];
 int bp;
 int error;
 int current_buffer_size; // number of actual loaded values in the buffer
};


int load_game(void);

void load_template(int t, int load_basic_template_details);

void load_int(int* value, int min, int max, const char* name);
void load_unsigned_int(unsigned int* value, int min, int max, const char* name);
void load_int_unchecked(int* value, const char* name);
void load_unsigned_int_unchecked(unsigned int* value, const char* name);
void load_short(s16b* value, int check_min_max, int min, int max, const char* name);
void load_fixed(al_fixed* value, int check_min_max, al_fixed min, al_fixed max, const char* name);
void load_string(char* str, int length, int accept_line_breaks, const char* name);
void load_char(char* value, const char* name);
//int load_8b(char* value, const char* name);
int load_8b(const char* name);
void load_proc_pointer(struct proc_struct** pr, const char* name);
void load_packet_pointer(struct packet_struct** pk, const char* name);
void load_object_coordinates(al_fixed* x, al_fixed* y, const char* name);
int check_object_coordinates(al_fixed x, al_fixed y, const char* name);
void load_error(int value, int min, int max, const char* name);
void load_error_fixed(al_fixed value, al_fixed min, al_fixed max, const char* name);
void simple_load_error(const char* name);

int open_load_file(const char* dialog_name, const char* file_extension);
int read_load_buffer(void);
void close_load_file(void);



#endif
