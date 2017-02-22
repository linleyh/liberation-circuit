

#ifndef H_I_BACKGROUND
#define H_I_BACKGROUND


#define BSHAPE_VERTICES 16
enum
{
BSHAPE_THING,

BSHAPES

};


struct bshape_struct
{

 int vertices;
 float vertex_angle [BSHAPE_VERTICES];
 float vertex_dist [BSHAPE_VERTICES];

 int triangles;
	int triangle_index [BSHAPE_VERTICES] [3];


};

enum
{
BACKBLOCK_OUTER, // outside map, not an edge
BACKBLOCK_EDGE_LEFT,
BACKBLOCK_EDGE_RIGHT,
BACKBLOCK_EDGE_UP,
BACKBLOCK_EDGE_DOWN,
BACKBLOCK_EDGE_UP_LEFT, // put this to the right of a vertical line of solid blocks and below a horizontal line (e.g. top left corner of map).
BACKBLOCK_EDGE_UP_RIGHT,
BACKBLOCK_EDGE_DOWN_LEFT,
BACKBLOCK_EDGE_DOWN_RIGHT,

BACKBLOCK_BASIC_HEX,
BACKBLOCK_BASIC_HEX_NO_NODES, // could have nodes added to it, but currently has none.
BACKBLOCK_EMPTY, // no nodes, and nodes cannot be added
BACKBLOCK_DATA_WELL, // block containing the well itself
BACKBLOCK_DATA_WELL_EDGE, // empty blocks near the data well that indicate that the data well should be drawn even though its centre is out of drawing distance (see also data_well[].last_drawn)

BACKBLOCK_TYPES
};

struct background_block_struct
{
// this is a struct of types of background block.
 int block_w; // how many blocks wide is it?
 int block_h; // height?

};

void init_bshapes(void);

#endif
