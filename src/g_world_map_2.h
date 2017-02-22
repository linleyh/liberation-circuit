
#ifndef H_G_WORLD_MAP_2
#define H_G_WORLD_MAP_2


enum
{
BNC_CENTRE_EMPTY, // clear centre
BNC_CENTRE_LEAVE,	// do nothing to centre
BNC_CENTRE_SMALL,	// make centre hexes small
};

enum
{
BNC_INNER_EMPTY,
BNC_INNER_NONE,
BNC_INNER_SCATTERED_GRADIENT, // delete some (should have empty centre) and give others gradient from small to large
BNC_INNER_GRADIENT, // small to large gradient, but don't overwrite nodes that are already larger
BNC_INNER_LINE, // one line
};

enum
{
BNC_MIDDLE_EMPTY,
BNC_MIDDLE_NONE,
BNC_MIDDLE_RAISED, // raised ring
BNC_MIDDLE_LINE, // narrow raised ring
};

enum
{
BNC_OUTER_EMPTY,
BNC_OUTER_NONE,
BNC_OUTER_GRADIENT,
BNC_OUTER_RANDOM_GRADIENT,
BNC_OUTER_LINE, // line then upwards gradient outwards
BNC_OUTER_ORANGE
};


void generate_map_from_map_init(void);
void clear_background_circle(int centre_block_x, int centre_block_y, int clear_size, int edge_thickness);
void reset_map_vision_masks(void);

#endif
