


#ifndef H_F_SAVE
#define H_F_SAVE

void save_game(void);

#define SAVE_BUFFER_SIZE 8192



struct save_statestruct
{
 ALLEGRO_FILECHOOSER* file_dialog;
 FILE *file;
 char buffer [SAVE_BUFFER_SIZE];
 int bp;
 int error;

};

int open_save_file(const char* dialog_name, const char* file_extension);

void save_bcode(struct bcode_struct* bc);
void save_template(int t, int save_basic_template_details);

void save_proc_pointer(struct proc_struct* pr);
int save_int(int num);
int save_short(s16b num);
int save_fixed(al_fixed num);
int save_char(char num);
void close_save_file(void);
int write_save_buffer(void);



#endif

