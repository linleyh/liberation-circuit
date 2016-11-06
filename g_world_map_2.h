
#ifndef H_G_WORLD_MAP_2
#define H_G_WORLD_MAP_2


enum
{
BNC_CENTRE_EMPTY, // clear centre
BNC_CENTRE_LEAVE	// do nothing to centre
};

enum
{
BNC_INNER_SCATTERED_GRADIENT, // delete some (should have empty centre) and give others gradient from small to large
BNC_INNER_GRADIENT // small to large gradient, but don't overwrite nodes that are already larger
};

enum
{
BNC_MIDDLE_RAISED, // raised ring
};

enum
{
BNC_OUTER_GRADIENT,
BNC_OUTER_RANDOM_GRADIENT,
};


void generate_map_from_map_init(void);
void clear_background_circle(int centre_block_x, int centre_block_y, int clear_size, int edge_thickness);

#endif
