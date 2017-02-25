
#ifndef H_G_WORLD_MAP
#define H_G_WORLD_MAP



#define MDETAILS 50

enum
{
MDETAIL_NONE,
MDETAIL_RIPPLE, // concentric ripples around centre
MDETAIL_LINE, // just a straight line
MDETAIL_INDIRECT_PATH, // some kind of indirect line
MDETAIL_VOID, // empty space with smaller nodes around edge
MDETAIL_RISE, // patch of higher nodes
MDETAIL_DIP, // patch of lower nodes
MDETAIL_RING, // circle of raised nodes
MDETAIL_RING_EMPTY, // circle of raised nodes with void in centre
MDETAIL_SYSTEM, // single large node surrounded by smaller ones
MDETAIL_SYSTEM_CLEAR, // like SYSTEM but clears around itself first
MDETAIL_WORM_SOURCE

};

enum
{
BACKGROUND_TYPE_UNIFORM, // all nodes the same (size and depth based on base_backgroun values in map_init)
BACKGROUND_TYPE_NOISE, // like uniform but also applies the background random valuds in map_init


};


struct mdetail_struct
{
 int type;
 int dsize;
 int player_col; // pre-coloured with player colours
 block_cart block_position;
 block_cart block_position2;
};



// this struct contains details of how the background is to be assembled.
// it's part of the w_init struct
struct map_init_struct
{

 int map_size_blocks; // taken from w_init value
 int players; // taken from w_init value
 int area_index; // affects the type of data wells generated etc.

 struct mdetail_struct mdetail [MDETAILS];

 int general_background_type;

// int base_background_depth;
// int background_depth_random_freq; // frequency of random addition/subtraction to background depth
// int background_depth_random_add; // size of random addition
// int background_depth_random_sub; // size of random subtraction
// int base_background_size;
// int background_size_random_freq; // frequency of random addition/subtraction to background size
// int background_size_random_add; // size of random addition
// int background_size_random_sub; // size of random subtraction
 int background_size_base;
 int background_size_random;
 int background_size_random_freq;//20;//60; // frequency of random addition/subtraction to background depth


 block_cart spawn_position [PLAYERS]; // may be nonsense for non-existent players
 int spawn_angle [PLAYERS];

 int data_wells;
 block_cart data_well_position [DATA_WELLS];
	float data_well_spin_rate [DATA_WELLS]; // only used for display
	int data_well_reserve_data [DATA_WELLS] [DATA_WELL_RESERVES];
	int data_well_reserve_squares [DATA_WELLS]; // currently the same for both reserves

//	int data_well_style; // e.g. AREA_BLUE

};

void reset_map_init(int map_size_blocks,
																				int map_area,
																				int players);
void generate_random_map(int area_index,
																									int size_blocks,
																					    int players,
																					    unsigned int map_seed);
void generate_scattered_map(int area_index,
																												int size_blocks,
																					       int players,
																					       unsigned int map_seed);

void set_player_spawn_position_by_latest_well(int player_index, int angle_from_well, int distance_from_well);
void set_player_spawn_position(int player_index, int block_x, int block_y, int angle);
void set_player_spawn_position_by_specified_well(int player_index, int well_index, int angle_from_well, int distance_from_well);

int add_data_well_to_map_init(int x, int y, int reserve_A, int reserve_B, int reserve_squares, float spin_rate);
int add_mdetail_ring(int centre_x, int centre_y, int ring_size, int empty_centre);
int add_mdetail_line(int start_x, int start_y, int end_x, int end_y, int line_thickness);
int add_mdetail_system(int centre_x, int centre_y, int system_size);
int add_mdetail_worm_source(int centre_x, int centre_y, int worms);
int	add_line_between_data_wells(int well_1, int well_2, int line_thickness);
int add_data_well_to_mdetail_ring(int mdetail_ring_index, int angle, int reserve_A, int reserve_B, int reserve_squares, float spin_rate);
void add_extra_spawn_by_latest_well(int player_index, int template_index, int angle_from_well);
block_cart get_well_block_position(int well_index);
void add_mdetail_worm_source_to_all_wells(void);

#endif
