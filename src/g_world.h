
#ifndef H_G_WORLD
#define H_G_WORLD

void start_world(void);

void initialise_world(void);
void new_world_from_world_init(void);
void deallocate_world(void);

void run_world(void);

void disrupt_block_nodes(al_fixed x, al_fixed y, int player_cause, int size);
void disrupt_single_block_node(al_fixed x, al_fixed y, int player_cause, int size);
void explosion_affects_block_nodes(al_fixed explosion_x, al_fixed explosion_y, int explosion_size, int player_index);
void pulse_block_node(al_fixed pulse_x, al_fixed pulse_y);

void change_block_node(struct backblock_struct* bl, int i, int move_x, int move_y, int size_change);
void change_block_node_colour(struct backblock_struct* bl, int i, int player_cause);
void align_block_node(struct backblock_struct* bl, int i);

void run_markers(void);

void load_mission_source(char* filename, int player_index, int template_index);
void load_default_source(char* filename, int player_index, int template_index);
int load_source_file_into_template(char* filename, int player_index, int template_index);
int load_source_file_into_template_without_compiling(char* filename, int player_index, int template_index, int open_the_template);

#endif
