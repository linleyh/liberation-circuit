
#ifndef H_G_METHOD_MISC
#define H_G_METHOD_MISC

//int run_scan(struct bcode_struct* bcode, struct proc_struct* ignore_proc, al_fixed scan_centre_x, al_fixed scan_centre_y, al_fixed range, s16b* mbase, int results_type);

// maximum number of procs in a process scanlist
#define SCANLIST_MAX_SIZE 64

int run_scan_square_spiral(struct core_struct* scanning_core,
                    int x, int y, int range,
                    al_fixed fix_x, al_fixed fix_y, al_fixed fix_range,
                    int max_number, int results_type);


int run_scan_rectangle(struct bcode_struct* bcode, al_fixed scan_x, al_fixed scan_y, al_fixed scan_x2, al_fixed scan_y2, s16b* mb, int results_type);

int check_point_collision(al_fixed x, al_fixed y, int minimum_collision_level);
int check_point_collision_ignore_team(al_fixed x, al_fixed y, int ignore_team, int minimum_collision_level);
int check_fuzzy_point_collision(al_fixed x, al_fixed y);

int check_start_end_addresses(int start_address, int end_address, int bcode_size);
int check_proc_shape_size(int proc_shape, int proc_size);

int run_examine(struct bcode_struct* bcode, al_fixed x, al_fixed y, al_fixed base_x, al_fixed base_y, s16b* mb);

int get_player_template_index(int player_index, int template_number);
void print_player_template_name(int source_program_type, int source_index, int source_player, int template_number);

void new_proc_fail_cloud(al_fixed x, al_fixed y, al_fixed angle, int shape, int size, int player_owner);

int copy_bcode_to_template(int t, struct bcode_struct* bcode, int source_program_type, int source_player_index, int template_origin, int start_address, int end_address, int name_address);


extern char method_error_string [120];

void	print_method_error(const char* error_message, int values, int value1);
void	print_method_error_string(void);


#endif
